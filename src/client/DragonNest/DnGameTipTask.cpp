#include "Stdafx.h"
#include "DnGameTipTask.h"
#include "DnTableDB.h"
#include "GameOption.h"
#include "DnWorld.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnGameTipTask::CDnGameTipTask(void)
{
	m_fDelayTime = 0.0f;
}

CDnGameTipTask::~CDnGameTipTask(void)
{
	Finalize();
}

bool CDnGameTipTask::Initialize()
{
	// Load game tips, and 
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TTODAYTIP );

	for( int itr = 0; itr < pSox->GetItemCount(); ++itr )
	{
		STipData sTipData;

		int nTableID = pSox->GetItemID( itr );
		
		int nGrade = pSox->GetFieldFromLablePtr( nTableID, "_Grade" )->GetInteger();

		bool bCustom = pSox->GetFieldFromLablePtr( nTableID, "_OutputType" )->GetInteger() == OutputType_Custom ? true : false;
		sTipData.nStringID = pSox->GetFieldFromLablePtr( nTableID, "_ToolTip" )->GetInteger();


#if defined(PRE_FIX_TODAYTOOLTIP_READ_TABLE) // [#60978] Today tooltip content update_client (target version M19)
		char* pMapID = NULL;
		pMapID = pSox->GetFieldFromLablePtr( nTableID, "_MapID" )->GetString();
		
		std::vector<string> vlTokens;
		TokenizeA( pMapID, vlTokens, ";" );
		
		if(!vlTokens.empty())
		{
			std::vector<string>::iterator itString = vlTokens.begin();

			for( ; itString != vlTokens.end() ; ++itString )
			{
				sTipData.vMapIDs.push_back( atoi( (*itString).c_str() ) );
			}
		}
#else
	for( int jtr = 0; jtr < MAP_COUNT; ++jtr )
	{
		char szName[256];
		sprintf_s( szName, _countof(szName), "_Custum%d", jtr );
		const DNTableCell* pField = pSox->GetFieldFromLablePtr( nTableID, szName );
		if (pField == NULL)
			continue;

		int nMapID = pField->GetInteger();

		if( 0 != nMapID )
			sTipData.vMapIDs.push_back( nMapID );
	}
#endif // PRE_FIX_TODAYTOOLTIP_READ_TABLE


		if( bCustom )
			m_vecCustom.push_back( sTipData );
		else if( GRADE_ALL == nGrade )
			m_vecAll.push_back( sTipData );
		else if( GRADE_NEW == nGrade )
			m_vecNew.push_back( sTipData );
		else if( GRADE_BEGINNER == nGrade )
			m_vecBeginner.push_back( sTipData );
		else if( GRADE_INTERMEDIATE == nGrade )
			m_vecIntermediate.push_back( sTipData );
	}

	return true;
}

void CDnGameTipTask::Finalize()
{
	m_vecNoBattleStringID.clear();
	m_vecBattleStringID.clear();

	m_vecAll.clear();
	m_vecNew.clear();
	m_vecBeginner.clear();
	m_vecIntermediate.clear();
}

void CDnGameTipTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !CDnActor::s_hLocalActor ) return;
	if( !CGameOption::IsActive() ) return;
	if( !CGameOption::GetInstance().bGameTip ) return;
	if( !CDnWorld::IsActive() ) return;
	if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeUnknown ) return;

	if( m_fDelayTime < 0.0f )
	{
		int nCurMap = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
		int nLevel = CDnActor::s_hLocalActor->GetLevel();

		int nStringID = GetTodayTipStringID( nCurMap, nLevel );

		if( -1 == nStringID )
		{
			m_fDelayTime += EXCEPTION_DELAY_TIME;
			return;
		}

		LPCWSTR wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID );
		GetInterface().AddGameTipMessage( wszStr, textcolor::WHITE, 5.0f );
		m_fDelayTime += DELAY_TIME;
	}
	else
	{
		m_fDelayTime -= fDelta;
	}
}

void CDnGameTipTask::ResetGameTip( float fCurDelayTime )
{
	GetInterface().CloseGameTipMessage();
	m_fDelayTime = fCurDelayTime;
}

int CDnGameTipTask::GetTodayTipStringID( int nMapID, int nLevel )
{
	int nStringID = GetCustomStringID( nMapID );

	if( -1 != nStringID )
		return nStringID;

	bool bAll = (_rand()%100) % 2 == 0 ? true : false;

	if( !bAll )
	{
		if( LEVEL_NEW >= nLevel && !m_vecNew.empty() )
		{
			return m_vecNew[ _rand()%m_vecNew.size() ].nStringID;
		}
		else if( LEVEL_BEGINNER > nLevel && !m_vecNew.empty() )
		{
			return m_vecBeginner[ _rand()%m_vecBeginner.size() ].nStringID;
		}
		else if( LEVEL_BEGINNER <= nLevel && !m_vecIntermediate.empty() )
		{
			return m_vecIntermediate[ _rand()%m_vecIntermediate.size() ].nStringID;
		}
	}

	if( !m_vecAll.empty() )
	{
		return m_vecAll[ _rand()%m_vecAll.size() ].nStringID;
	}

	return -1;
}

int CDnGameTipTask::GetCustomStringID( int nMapID )
{
	std::vector<int> vStringID;

	// Custom data exists only in the full level.
	for( int itr = 0; itr < (int)m_vecCustom.size(); ++itr )
	{
		for( int jtr = 0; jtr < (int)m_vecCustom[itr].vMapIDs.size(); ++jtr )
		{
			if( nMapID == m_vecCustom[itr].vMapIDs[jtr] )
			{
				vStringID.push_back( m_vecCustom[itr].nStringID );
				break;
			}
		}
	}

	if( !vStringID.empty() )
		return vStringID[ _rand()%vStringID.size() ];

	return -1;
}