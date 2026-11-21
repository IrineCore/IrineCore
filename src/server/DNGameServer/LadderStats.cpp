
#include "stdafx.h"
#include "LadderStats.h"
#include "DNPvPGameRoom.h"
#include "DNUserSession.h"
#include "DnPlayerActor.h"
#include "DNDBConnection.h"
#include "PvPGameMode.h"
#include "DnMonsterActor.h"

using namespace LadderSystem;

//###############################################################################################################################
// FactoryClass
//###############################################################################################################################

CStatsRepository* CStatsFactory::CreateRepository( CDNGameRoom* pGameRoom )
{
	_ASSERT( pGameRoom->bIsLadderRoom() );

	CStatsRepository* pStats = new CStatsRepository( pGameRoom );
	if( pStats->bInitialize() == false )
	{
		_ASSERT(0);
		SAFE_DELETE( pStats );
	}

	return pStats;
}


//###############################################################################################################################
// CStats
//###############################################################################################################################

CStats::CStats( CDNUserSession* pSession, MatchType::eCode MatchType )
:m_biCharacterDBID(pSession->GetCharacterDBID()),m_MatchType(MatchType)
{
	m_bInit				= false;
	m_iGradePoint		= 0;					// Rating
	m_iHiddenGradePoint	= 0;					// Hidden grade point
	m_MatchResult		= MatchResult::None;	// Match result
	m_iVSJobCode		= 0;					// Used only when MatchType::_1vs1
}

void CStats::ConvertKillResult( std::vector<LadderKillResult>& vData )
{
	if( m_mKillDeathCount.empty() )
		return;

	vData.reserve( m_mKillDeathCount.size() );

	for( std::map<int,SKillDeathCount>::iterator itor=m_mKillDeathCount.begin() ; itor!=m_mKillDeathCount.end() ; ++itor )
	{
		LadderKillResult Data;
		Data.cJobCode		= (*itor).first;
		Data.nKillCount		= (*itor).second.iKillCount;
		Data.nDeathCount	= (*itor).second.iDeathCount;
		vData.push_back( Data );
	}
}

void CStats::OnRecvLadderScore( const TAGetListPvPLadderScore* pPacket )
{
	if( pPacket->nRetCode != ERROR_NONE )
	{
		_ASSERT(0);
		return;
	}
	
	// If duplicate initialization arrives, there is a problem.. Even if it happens, trust the latest data.
	if( m_bInit == true )
		_ASSERT(0);

	bool bFound = false;

	for( int i=0 ; i<pPacket->Data.cLadderTypeCount ; ++i )
	{
		if( pPacket->Data.LadderScore[i].cPvPLadderCode == m_MatchType )
		{
			m_iGradePoint		= pPacket->Data.LadderScore[i].iPvPLadderGradePoint;		// Rating
			m_iHiddenGradePoint	= pPacket->Data.LadderScore[i].iHiddenPvPLadderGradePoint;	// Hidden grade point
			bFound = true;
			break;
		}
	}

	if( bFound == false )
	{
		m_iGradePoint		= Stats::InitGradePoint;
		m_iHiddenGradePoint	= Stats::InitGradePoint;
	}


	m_bInit = true;
}

void CStats::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	if( m_bInit == false )	
	{
		_ASSERT(0);
		return;
	}

	if( !hHitter )
		return;

	// DeathCount
	std::map<int,SKillDeathCount>::iterator itor = m_mKillDeathCount.find( hHitter->OnGetJobClassID() );
	if( itor == m_mKillDeathCount.end() )
	{
		m_mKillDeathCount.insert( std::make_pair(hHitter->OnGetJobClassID(),SKillDeathCount(0,1)) );
	}
	else
	{
		++(*itor).second.iDeathCount;
	}
}

void CStats::OnKill( DnActorHandle hActor, DnActorHandle hDieActor )
{
	if( m_bInit == false )	
	{
		_ASSERT(0);
		return;
	}

	if( !hDieActor )
		return;

	// KillCount
	std::map<int,SKillDeathCount>::iterator itor = m_mKillDeathCount.find( hDieActor->OnGetJobClassID() );
	if( itor == m_mKillDeathCount.end() )
	{
		m_mKillDeathCount.insert( std::make_pair(hDieActor->OnGetJobClassID(),SKillDeathCount(1,0)) );
	}
	else
	{
		++(*itor).second.iKillCount;
	}
}

//###############################################################################################################################
// CStatsRepository
//###############################################################################################################################

CStatsRepository::CStatsRepository( CDNGameRoom* pGameRoom ):m_pGameRoom(pGameRoom),m_MatchType(MatchType::None)
{
	for( int i=0 ; i<_countof(m_iTeamGradePointAvg) ; ++i )
		m_iTeamGradePointAvg[i] = 0;

	//#############################################
	// For rating calculation
	//#############################################

	S_Repository[0] = boost::tuple<float,float,float>(1.f,0.4f,0.5f);
	S_Repository[1] = boost::tuple<float,float,float>(1.f,0.f,0.5f);
	S_Repository[2] = boost::tuple<float,float,float>(1.f,0.f,0.5f);
	S_Repository[3] = boost::tuple<float,float,float>(1.f,0.f,0.4f);
	S_Repository[4] = boost::tuple<float,float,float>(1.f,0.f,0.3f);
	S_Repository[5] = boost::tuple<float,float,float>(1.f,0.f,-0.5f);

	HiddenS_Repository[0] = boost::tuple<float,float,float>(1.f,0.f,0.5f);
	HiddenS_Repository[1] = boost::tuple<float,float,float>(1.f,0.f,0.5f);
	HiddenS_Repository[2] = boost::tuple<float,float,float>(1.f,0.f,0.5f);
	HiddenS_Repository[3] = boost::tuple<float,float,float>(1.f,0.f,0.5f);
	HiddenS_Repository[4] = boost::tuple<float,float,float>(1.f,-0.2f,0.5f);
	HiddenS_Repository[5] = boost::tuple<float,float,float>(1.f,-0.3f,0.5f);

	K_Repository[0] = boost::tuple<float,float,float,float>(60.f,60.f,60.f,60.f);
	K_Repository[1] = boost::tuple<float,float,float,float>(50.f,50.f,50.f,50.f);
	K_Repository[2] = boost::tuple<float,float,float,float>(40.f,40.f,40.f,40.f);
	K_Repository[3] = boost::tuple<float,float,float,float>(30.f,30.f,30.f,30.f);
	K_Repository[4] = boost::tuple<float,float,float,float>(20.f,20.f,20.f,20.f);
	K_Repository[5] = boost::tuple<float,float,float,float>(10.f,10.f,10.f,10.f);

	HiddenK_Repository[0] = boost::tuple<float,float,float,float>(120.f,120.f,120.f,120.f);
	HiddenK_Repository[1] = boost::tuple<float,float,float,float>(100.f,100.f,100.f,100.f);
	HiddenK_Repository[2] = boost::tuple<float,float,float,float>(80.f,80.f,80.f,80.f);
	HiddenK_Repository[3] = boost::tuple<float,float,float,float>(60.f,60.f,60.f,60.f);
	HiddenK_Repository[4] = boost::tuple<float,float,float,float>(40.f,40.f,40.f,40.f);
	HiddenK_Repository[5] = boost::tuple<float,float,float,float>(20.f,20.f,20.f,20.f);
}

CStatsRepository::~CStatsRepository()
{
	for( std::map<INT64,CStats*>::iterator itor=m_mRepository.begin() ; itor!=m_mRepository.end() ; ++itor )
		SAFE_DELETE( (*itor).second );
}

bool CStatsRepository::bInitialize()
{
	CDNPvPGameRoom* pPvPGameRoom = static_cast<CDNPvPGameRoom*>(m_pGameRoom);
	m_MatchType = pPvPGameRoom->GetPvPLadderMatchType();

	int iNeedTeamPlayer = static_cast<int>(m_MatchType);

	// Check Team A member count
	if( pPvPGameRoom->GetUserCount( PvPCommon::Team::A ) != iNeedTeamPlayer )
	{
		_ASSERT(0);
		return false;
	}
	// Check Team B member count
	if( pPvPGameRoom->GetUserCount( PvPCommon::Team::B ) != iNeedTeamPlayer )
	{
		_ASSERT(0);
		return false;
	}

	for( UINT i=0 ; i<m_pGameRoom->GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = m_pGameRoom->GetUserData(i);
		if( (pSession->GetTeam() == PvPCommon::Team::A || pSession->GetTeam() == PvPCommon::Team::B) == false )
			continue;

		CStats* pStats = new CStats( pSession, m_MatchType );
		if( pStats == NULL )
		{
			_ASSERT(0);
			return false;
		}
		m_mRepository.insert( std::make_pair(pSession->GetCharacterDBID(),pStats) );
	}

	if( m_mRepository.size() != iNeedTeamPlayer*2 )
		return false;

	return true;
}

bool CStatsRepository::QueryUpdateResult( CDNUserSession* pSession, UINT uiWinTeam, PvPCommon::QueryUpdatePvPDataType::eCode Type )
{
	if( static_cast<CDNPvPGameRoom*>(m_pGameRoom)->GetPvPGameMode()->GetRemainStartTick() > 0 )
	{
		return false;
	}

	if( bIsValidLadderTeam( pSession->GetTeam() ) == false )
	{
		_ASSERT(0);
		return false;
	}

	std::map<INT64,CStats*>::iterator itor = m_mRepository.find( pSession->GetCharacterDBID() );
	if( itor == m_mRepository.end() )
		return false;

	CStats* pStats = (*itor).second;

	//##############################
	// Average rating
	//##############################

	float Ra = static_cast<float>((pSession->GetTeam() == PvPCommon::Team::A) ? m_iTeamGradePointAvg[PvPCommon::TeamIndex::A] : m_iTeamGradePointAvg[PvPCommon::TeamIndex::B]);
	float Rb = static_cast<float>((pSession->GetTeam() == PvPCommon::Team::A) ? m_iTeamGradePointAvg[PvPCommon::TeamIndex::B] : m_iTeamGradePointAvg[PvPCommon::TeamIndex::A]);
	
	//##############################
	// Adjust ladder rating
	//##############################

	int iAvgGradePoint	= static_cast<int>(Ra);
	int iTupleIndex		= 0;
	
	// Lowest-rank user
	if( iAvgGradePoint < 1400 )
		iTupleIndex = 0;
	// Default user
	else if( iAvgGradePoint < 1700 )
		iTupleIndex = 1;
	// Normal user
	else if( iAvgGradePoint < 2000 )
		iTupleIndex = 2;
	// Top-ranking user
	else if( iAvgGradePoint < 2300 )
		iTupleIndex = 3;
	// Core user
	else if( iAvgGradePoint < 2500 )
		iTupleIndex = 4;
	// Android user
	else
		iTupleIndex = 5;

	if( iTupleIndex >= MAX_GRADEPOINT_RANGE )
	{
		_ASSERT(0);
		iTupleIndex = 0;
	}

	//##############################
	// Match result
	//##############################

	float S = 0.f;
	float HiddenS = 0.f;
	float K = 0.f;
	float HiddenK = 0.f;

	MatchResult::eCode Result = MatchResult::Lose;

	switch( Type )
	{
		case PvPCommon::QueryUpdatePvPDataType::FinishGameMode:
		{
			if( uiWinTeam == PvPCommon::Team::Max )			// Draw
			{
				Result	= MatchResult::Draw;
				S		= S_Repository[iTupleIndex].get<2>();
				HiddenS	= HiddenS_Repository[iTupleIndex].get<2>();
			}
			else if( uiWinTeam == pSession->GetTeam() )		// Win
			{
				Result	= MatchResult::Win;
				S		= S_Repository[iTupleIndex].get<0>();
				HiddenS	= HiddenS_Repository[iTupleIndex].get<0>();
			}
			else											// Loss
			{
				Result	= MatchResult::Lose;
				S		= S_Repository[iTupleIndex].get<1>();
				HiddenS	= HiddenS_Repository[iTupleIndex].get<1>();
			}
			break;
		}
	}

	switch( m_MatchType )
	{
		case MatchType::_1vs1:
		{
			K		= K_Repository[iTupleIndex].get<0>();
			HiddenK = HiddenK_Repository[iTupleIndex].get<0>();
			break;
		}
		case MatchType::_2vs2:
		{
			K		= K_Repository[iTupleIndex].get<1>();
			HiddenK = HiddenK_Repository[iTupleIndex].get<1>();
			break;
		}
		case MatchType::_3vs3:
		{
			K		= K_Repository[iTupleIndex].get<2>();
			HiddenK = HiddenK_Repository[iTupleIndex].get<2>();
			break;
		}
		case MatchType::_4vs4:
		{
			K		= K_Repository[iTupleIndex].get<3>();
			HiddenK = HiddenK_Repository[iTupleIndex].get<3>();
			break;
		}
	}

	//##############################
	// Expected win rate
	//##############################

	float Ea = 1/(1.f+pow(10,(Rb-Ra)/400.f));

	//##############################
	// Rating delta
	//##############################

	float ChangeRa			= K*(S-Ea);
	float ChangeHiddenRa	= HiddenK*(HiddenS-Ea);

	//##############################
	// float rounding
	//##############################

	ChangeRa = static_cast<float>(static_cast<int>(ChangeRa + (ChangeRa > 0.f ? .5f : -.5f) )) / 1.f;
	ChangeHiddenRa = static_cast<float>(static_cast<int>(ChangeHiddenRa + (ChangeHiddenRa > 0.f ? .5f : -.5f) )) / 1.f;

	//##############################
	// Send query
	//##############################

	CDNDBConnection* pDBCon = pSession->GetDBConnection();
	if( pDBCon == NULL )
		return false;

	int		iGradePoint			= pStats->GetGradePoint()+static_cast<int>(ChangeRa);
	int		iHiddenGradePoint	= pStats->GetHiddenGradePoint()+static_cast<int>(ChangeHiddenRa);
	BYTE	cVSJobCode			= pStats->GetVSJobCode();
	std::vector<LadderKillResult> vData;
	pStats->ConvertKillResult( vData );

	if( iGradePoint < Stats::MinGradePoint )
		iGradePoint = Stats::MinGradePoint;
	if( iHiddenGradePoint < Stats::MinGradePoint )
		iHiddenGradePoint = Stats::MinGradePoint;


	g_Log.Log( LogType::_LADDER, pSession, L"result gradepoint:%d\r\n", iGradePoint );

	m_mResult[pSession->GetCharacterDBID()] = SResult(static_cast<int>(ChangeRa),iGradePoint );

	return pDBCon->QueryAddPvPLadderResult( pSession, Type, m_MatchType, iGradePoint, iHiddenGradePoint, Result, cVSJobCode, vData );
}

void CStatsRepository::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	//When a summon scored the kill
	if( hHitter && hHitter->IsMonsterActor() )
	{		
		DnActorHandle hMaster = static_cast<CDnMonsterActor*>(hHitter.GetPointer())->GetSummonerPlayerActor();
		if( hMaster && hMaster->IsPlayerActor() )
			hHitter = hMaster;
	}
	// Validity check
	if( !(hActor && hHitter && hActor->IsPlayerActor() && hHitter->IsPlayerActor()) )
		return;

	CDnPlayerActor* pPlayer = reinterpret_cast<CDnPlayerActor*>(hActor.GetPointer());
	std::map<INT64,CStats*>::iterator itor = m_mRepository.find( pPlayer->GetCharacterDBID() );
	if( itor != m_mRepository.end() )
		(*itor).second->OnDie( hActor, hHitter );

	CDnPlayerActor* pHitter = reinterpret_cast<CDnPlayerActor*>(hHitter.GetPointer());
	itor = m_mRepository.find( pHitter->GetCharacterDBID() );
	if( itor != m_mRepository.end() )
		(*itor).second->OnKill( hHitter, hActor );
}

void CStatsRepository::OnRecvLadderScore( INT64 biCharacterDBID, int iTeam, const TAGetListPvPLadderScore* pPacket )
{
	std::map<INT64,CStats*>::iterator itor = m_mRepository.find( biCharacterDBID );
	if( itor == m_mRepository.end() )
	{
		_ASSERT(0);
		return;
	}

	if( bIsValidLadderTeam( iTeam ) == false )
		_ASSERT(0);

	(*itor).second->OnRecvLadderScore( pPacket );

	// Calculate average rating per team
	int iGradePoint = (*itor).second->GetGradePoint();

	std::map<int,std::vector<int>>::iterator Sumitor = m_mGradePointSum.find( iTeam );
	if( Sumitor == m_mGradePointSum.end() )
	{
		std::vector<int> vTemp;
		vTemp.push_back( iGradePoint );
		m_mGradePointSum.insert( std::make_pair(iTeam,vTemp) );
	}
	else
	{
		(*Sumitor).second.push_back( iGradePoint );
	}

	// Calculate average
	int iSum = 0;
	Sumitor = m_mGradePointSum.find( iTeam );
	for( UINT i=0 ; i<(*Sumitor).second.size() ; ++i )
		iSum += (*Sumitor).second[i];

	if ((*Sumitor).second.size() <= 0)
	{
		_ASSERT(0);
		return;
	}

	m_iTeamGradePointAvg[iTeam-PvPCommon::Team::A] = static_cast<int>(iSum/(*Sumitor).second.size());
}

void CStatsRepository::OnSetPlayState()
{
	// Verify Stats initialization
	for( std::map<INT64,CStats*>::iterator itor=m_mRepository.begin() ; itor!=m_mRepository.end() ; ++itor )
	{
		CStats* pStats = (*itor).second;

		if( pStats->bIsInit() == false )
		{
			_ASSERT(0);
			CDNUserSession* pSession = m_pGameRoom->GetUserSessionByCharDBID( (*itor).first );
			if( pSession )
				pSession->DetachConnection( L"LadderStats isn't Init" );
		}
	}

	// Set VSJobCode when in 1vs1
	if( m_MatchType == MatchType::_1vs1 )
	{
		if( m_pGameRoom->GetUserCount() != 2 )
			return;

		CDNUserSession* pSession	= m_pGameRoom->GetUserData(0);
		CDNUserSession* pSession2	= m_pGameRoom->GetUserData(1);

		if( pSession && pSession2 )
		{
			std::map<INT64,CStats*>::iterator itor = m_mRepository.find( pSession->GetCharacterDBID() );
			if( itor != m_mRepository.end() )
				(*itor).second->SetVSJobCode( pSession2->GetUserJob() );

			itor = m_mRepository.find( pSession2->GetCharacterDBID() );
			if( itor != m_mRepository.end() )
				(*itor).second->SetVSJobCode( pSession->GetUserJob() );
		}
	}
}

bool CStatsRepository::bIsValidLadderTeam( int iTeam )
{
	switch( iTeam )
	{
		case PvPCommon::Team::A:
		case PvPCommon::Team::B:
			return true;
	}

	return false;
}

int	CStatsRepository::GetAddGradePoint( INT64 biCharacterDBID )
{
	std::map<INT64,SResult>::iterator itor = m_mResult.find( biCharacterDBID );
	if( itor != m_mResult.end() )
		return (*itor).second.iAddGradePoint;
	return 0;
}
int CStatsRepository::GetResultGradePoint( INT64 biCharacterDBID )
{
	std::map<INT64,SResult>::iterator itor = m_mResult.find( biCharacterDBID );
	if( itor != m_mResult.end() )
		return (*itor).second.iResultGradePoint;
	return 0;
}
