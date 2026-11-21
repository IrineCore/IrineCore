#include "stdafx.h"
#include "DnPvPRacingResultDlg.h"
#include "DnLocalPlayerActor.h"
#include "SystemSendPacket.h"
#include "TaskManager.h"
#include "DnPvPGameTask.h"
#include "DnMainMenuDlg.h"
#include "DnGuildTask.h"
#include "DnRevengeTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

bool CompareLapTime( const CDnPvPRacingResultDlg::SUserInfo & pFirst, const CDnPvPRacingResultDlg::SUserInfo & pSecond )
{
	return pFirst.dwLapTime < pSecond.dwLapTime;
}

bool ZeroCompareDword( const CDnPvPRacingResultDlg::SUserInfo & pFirst, const CDnPvPRacingResultDlg::SUserInfo & pSecond )
{
	return pFirst.dwLapTime != 0 && pSecond.dwLapTime == 0;
}

#if defined( PRE_ADD_REVENGE )
bool RevengeUserCompare( const CDnPvPRacingResultDlg::SUserInfo & pFirst, const CDnPvPRacingResultDlg::SUserInfo & pSecond )
{
	return pFirst.eRevengeUser > pSecond.eRevengeUser;
}
#endif	// #if defined( PRE_ADD_REVENGE )

CDnPvPRacingResultDlg::CDnPvPRacingResultDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pBtClose( NULL )
, m_bIsFinalResult( false )
, m_fTotal_ElapsedTime( 0.f )
{

}

CDnPvPRacingResultDlg::~CDnPvPRacingResultDlg()
{
	SAFE_RELEASE_SPTR( m_hPVPMedalIconImage );
}

void CDnPvPRacingResultDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpScore_RaceDlg.ui" ).c_str(), bShow );
}

void CDnPvPRacingResultDlg::InitialUpdate()
{
	char szUIName[256] = {0,};

	for( int itr = 0; itr < eSlotCount::eMax_SlotCount; ++itr )
	{
		//sprintf( szUIName, "ID_SCORE_LEVEL%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_SCORE_LEVEL%d", itr);
		m_UISlotData[itr].m_pStaticLevel = GetControl<CEtUIStatic>( szUIName );

		//sprintf( szUIName, "ID_SCORE_JOB%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_SCORE_JOB%d", itr);
		m_UISlotData[itr].m_pStaticJob = GetControl<CDnJobIconStatic>( szUIName );

		//sprintf( szUIName, "ID_SCORE_GUILDNAME%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_SCORE_GUILDNAME%d", itr);
		m_UISlotData[itr].m_pStaticGuildName = GetControl<CEtUIStatic>( szUIName );

		//sprintf( szUIName, "ID_SCORE_NAME%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_SCORE_NAME%d", itr);
		m_UISlotData[itr].m_pStaticPlayerName = GetControl<CEtUIStatic>( szUIName );

		//sprintf( szUIName, "ID_SCORE_RACERANK%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_SCORE_RACERANK%d", itr);
		m_UISlotData[itr].m_pStaticFinalRanking = GetControl<CEtUIStatic>( szUIName );

		//sprintf( szUIName, "ID_SCORE_LAPTIME%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_SCORE_LAPTIME%d", itr);
		m_UISlotData[itr].m_pStaticLapTime = GetControl<CEtUIStatic>( szUIName );

		//sprintf( szUIName, "ID_SCORE_PC%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_SCORE_PC%d", itr);
		m_UISlotData[itr].m_pStaticPCBang = GetControl<CEtUIStatic>( szUIName );

		//sprintf( szUIName, "ID_SCORE_MEDAL%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_SCORE_MEDAL%d", itr);
		m_UISlotData[itr].m_pStaticReward = GetControl<CEtUIStatic>( szUIName );

		//sprintf( szUIName, "ID_STATIC_ME%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_STATIC_ME%d", itr);
		m_UISlotData[itr].m_pStaticCover = GetControl<CEtUIStatic>( szUIName );

		//sprintf( szUIName, "ID_TEXTURE_RANK%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_TEXTURE_RANK%d", itr);
		m_UISlotData[itr].m_pTCGrade = GetControl<CEtUITextureControl>( szUIName );

		//sprintf( szUIName, "ID_TEXTURE_MARK%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_TEXTURE_MARK%d", itr);
		m_UISlotData[itr].m_pTCGuildMark = GetControl<CEtUITextureControl>( szUIName );
		
		//sprintf( szUIName, "ID_SCORE_BLUE_MEDALICON%d", itr );
		_snprintf_s(szUIName, _countof(szUIName), _TRUNCATE, "ID_SCORE_BLUE_MEDALICON%d", itr);
		m_UISlotData[itr].m_pTCMedalIcon = GetControl<CEtUITextureControl>( szUIName );
	}

	m_pBtClose = GetControl<CEtUIButton>( "ID_BUTTON_CLOSE" );
	m_pBtClose->Show( false );

	SAFE_RELEASE_SPTR( m_hPVPMedalIconImage );
	m_hPVPMedalIconImage = LoadResource( CEtResourceMng::GetInstance().GetFullName( "Pvp_Medal.dds" ).c_str(), RT_TEXTURE );
}

void CDnPvPRacingResultDlg::Show( bool bShow )
{
	if( bShow == m_bShow || m_bIsFinalResult )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnPvPRacingResultDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( m_bIsFinalResult )
	{
		m_fTotal_ElapsedTime += fElapsedTime;

		if( m_fTotal_ElapsedTime > static_cast<float>(PvPCommon::Common::PvPFinishScoreOpenDelay) )
		{
			if( !IsShow() )
			{
				UpdateUser();
				m_pBtClose->Show( true );
				CEtUIDialog::Show( true );	// Force Show
			}

			WCHAR wszCloseMessage[256];
			SecureZeroMemory(wszCloseMessage ,sizeof(wszCloseMessage));
			wsprintf(wszCloseMessage, GetEtUIXML().GetUIString( CEtUIXML::idCategory1,121013), PvPCommon::Common::PvPFinishAutoClose-static_cast<int>(m_fTotal_ElapsedTime) );
			if( m_pBtClose )
				m_pBtClose->SetText(wszCloseMessage );
		}

		if( m_fTotal_ElapsedTime > static_cast<float>(PvPCommon::Common::PvPFinishAutoClose) )
		{
			SendMovePvPGameToPvPLobby();
			m_bIsFinalResult = false;
		}
	}
}

void CDnPvPRacingResultDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CLOSE" ) )
		{
			SendMovePvPGameToPvPLobby();
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPvPRacingResultDlg::AddUser( DnActorHandle hUser )
{
	if( hUser->GetTeam() == PvPCommon::Team::Observer )
		return;

	if( hUser )
	{
		CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(hUser.GetPointer());
		if( pPlayerActor )
		{
			SUserInfo stUserInfo;
			stUserInfo.nSessionID = pPlayerActor->GetUniqueID();
			stUserInfo.nLevel = pPlayerActor->GetLevel();
			stUserInfo.cJobClassID = pPlayerActor->GetJobClassID();
			stUserInfo.cPVPlevel = pPlayerActor->GetPvPLevel();
			stUserInfo.wszUserName = pPlayerActor->GetName();
			if( pPlayerActor->IsJoinGuild() )
				stUserInfo.GuildSelfView.Set( pPlayerActor->GetGuildSelfView() );

#if defined( PRE_ADD_REVENGE )
			CDnRevengeTask * pRevengeTask = (CDnRevengeTask *)CTaskManager::GetInstance().GetTask( "RevengeTask" );
			if( NULL != pRevengeTask && CDnBridgeTask::GetInstance().GetSessionID() != stUserInfo.nSessionID )
				pRevengeTask->GetRevengeUserID( pPlayerActor->GetUniqueID(), stUserInfo.eRevengeUser );
#endif	// #if defined( PRE_ADD_REVENGE )

			m_vUserInfo.push_back(stUserInfo);
		}
	}
}

void CDnPvPRacingResultDlg::RemoveUser( DnActorHandle hUser )
{
	if( true == m_bIsFinalResult )
		return;

	if( hUser )
	{
		for( DWORD itr = 0; itr < m_vUserInfo.size(); ++itr )
		{
			if( hUser->GetUniqueID() == m_vUserInfo[itr].nSessionID )
			{
				m_vUserInfo.erase(m_vUserInfo.begin() + itr );
				return;
			}
		}
	}
}

void CDnPvPRacingResultDlg::SetUserScore( const int nSessionID, const DWORD dwLapTime )
{
	for( DWORD itr = 0; itr < m_vUserInfo.size(); ++itr )
	{
		if( nSessionID == m_vUserInfo[itr].nSessionID )
		{
			m_vUserInfo[itr].dwLapTime = dwLapTime;
		}
	}
}

void CDnPvPRacingResultDlg::SetUserState( DnActorHandle hUser ,int nState )
{
	if( hUser )
	{
		for( UINT i = 0; i< m_vUserInfo.size();i++ )
		{
			if( hUser->GetUniqueID() == m_vUserInfo[i].nSessionID && m_vUserInfo[i].nState != MINE_S )
			{
				m_vUserInfo[i].nState = nState;
				return;
			}
		}
	}
}

void CDnPvPRacingResultDlg::SetPVPXP( UINT nSessionID, UINT uiAddXPScore,UINT uiTotalXPScore,UINT uiMedalScore)
{
	int nGainMedalCount = 0;
	int nMedalExp = 0;
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP ) 
	{
		CDnPvPGameTask *pPVPGameTask = static_cast<CDnPvPGameTask*>(pGameTask);
		nMedalExp = pPVPGameTask->GetMedalExp();
		// #56177 Ghoul mode does not use XP 
		if( pPVPGameTask->GetGameMode() == PvPCommon::GameMode::PvP_Zombie_Survival )
			uiAddXPScore = 0;
	}

	for( UINT i = 0; i< m_vUserInfo.size();i++ )
	{
		if( nSessionID == m_vUserInfo[i].nSessionID )
		{
			m_vUserInfo[i].uiXP = uiAddXPScore;
			m_vUserInfo[i].uiMedal = uiMedalScore;
			m_vUserInfo[i].uiTotalXP = uiTotalXPScore; 

			if( m_vUserInfo[i].nState == MINE_S )
			{
				if( nGainMedalCount && GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG ) ) {
					WCHAR wszTemp[128] = { 0, };
					swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 121069 ), nGainMedalCount );
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp, false );
				}
			}
			return;
		}
	}
}

void CDnPvPRacingResultDlg::UpdateUser()
{
	InitializeSlot();

	std::stable_sort( m_vUserInfo.begin(), m_vUserInfo.end(), CompareLapTime );
	std::stable_sort( m_vUserInfo.begin(), m_vUserInfo.end(), ZeroCompareDword );
#if defined( PRE_ADD_REVENGE )
	std::stable_sort( m_vUserInfo.begin(), m_vUserInfo.end(), RevengeUserCompare );
#endif	// #if defined( PRE_ADD_REVENGE )

	std::wstring wszString;
	WCHAR wszBuffer[256] = {0,};
	int nLank = 0;
	for( DWORD itr = 0; itr < m_vUserInfo.size(); ++itr )
	{
		if( eSlotCount::eMax_SlotCount <= nLank )
			return;

		swprintf_s( wszBuffer, _countof(wszBuffer), L"Lv %d", m_vUserInfo[itr].nLevel );
		m_UISlotData[nLank].m_pStaticLevel->SetText( wszBuffer );

		if( 0 < m_vUserInfo[itr].cJobClassID )
			m_UISlotData[nLank].m_pStaticJob->SetIconID( m_vUserInfo[itr].cJobClassID, true );

		int iIconW,iIconH;
		int iU,iV;
		iIconW = GetInterface().GeticonWidth();
		iIconH = GetInterface().GeticonHeight();
		if( GetInterface().ConvertPVPGradeToUV( m_vUserInfo[itr].cPVPlevel, iU, iV ))
		{
			m_UISlotData[nLank].m_pTCGrade->SetTexture( GetInterface().GetPVPIconTex(), iU, iV, iIconW, iIconH );
			m_UISlotData[nLank].m_pTCGrade->Show(true);
		}

		if( m_vUserInfo[nLank].GuildSelfView.IsSet() ) 
		{
			const TGuildView &GuildView = m_vUserInfo[itr].GuildSelfView;
			if( GetGuildTask().IsShowGuildMark( GuildView ) )
			{
				EtTextureHandle hGuildMark = GetGuildTask().GetGuildMarkTexture( GuildView );
				m_UISlotData[nLank].m_pTCGuildMark->SetTexture( hGuildMark );
				m_UISlotData[nLank].m_pTCGuildMark->Show( true );
			}

			m_UISlotData[nLank].m_pStaticGuildName->SetText( m_vUserInfo[itr].GuildSelfView.wszGuildName );
		}

#if defined( PRE_ADD_REVENGE )
		if( MINE_S != m_vUserInfo[nLank].nState )
		{
			CDnRevengeTask * pRevengeTask = (CDnRevengeTask *)CTaskManager::GetInstance().GetTask( "RevengeTask" );
			if( NULL != pRevengeTask )
				pRevengeTask->GetRevengeUserID( m_vUserInfo[itr].nSessionID, m_vUserInfo[itr].eRevengeUser );

			if( Revenge::RevengeTarget::eRevengeTarget_Target == m_vUserInfo[itr].eRevengeUser )
				m_UISlotData[nLank].m_pStaticPlayerName->SetTextColor( EtInterface::textcolor::PVP_REVENGE_TARGET );
			else if( Revenge::RevengeTarget::eRevengeTarget_Me == m_vUserInfo[itr].eRevengeUser )
				m_UISlotData[nLank].m_pStaticPlayerName->SetTextColor( EtInterface::textcolor::PVP_REVENGE_ME );
			else
				m_UISlotData[nLank].m_pStaticPlayerName->SetTextColor( EtInterface::textcolor::WHITE );
		}
#endif	//	#if defined( PRE_ADD_REVENGE )
		m_UISlotData[nLank].m_pStaticPlayerName->SetText( m_vUserInfo[itr].wszUserName );

		if( 0 == m_vUserInfo[itr].dwLapTime )
			m_UISlotData[nLank].m_pStaticFinalRanking->SetText( L"-" );
		else
			m_UISlotData[nLank].m_pStaticFinalRanking->SetIntToText( nLank + 1 );

		wszString = GetLapTimeString( m_vUserInfo[itr].dwLapTime );
		m_UISlotData[nLank].m_pStaticLapTime->SetText( wszString );
		
		if( m_bIsFinalResult || m_vUserInfo[itr].uiXP )
		{
			m_UISlotData[nLank].m_pStaticPCBang->ClearText();

			swprintf_s( wszBuffer, _countof(wszBuffer), L"X %d", m_vUserInfo[itr].uiMedal );
			m_UISlotData[nLank].m_pStaticReward->SetText( wszBuffer );

			// Currently there is only one type of medal.
			m_UISlotData[nLank].m_pTCMedalIcon->SetTexture( m_hPVPMedalIconImage, 0, 0, PVP_MEDAL_ICON_XSIZE, PVP_MEDAL_ICON_YSIZE );
			m_UISlotData[nLank].m_pTCMedalIcon->Show(true);
		}

		if( m_vUserInfo[nLank].nState == MINE_S )
			m_UISlotData[nLank].m_pStaticCover->Show( true );

		++nLank;
	}
}

void CDnPvPRacingResultDlg::FinalResultOpen()
{
	Show(false);

	m_bIsFinalResult = true;

	UpdateUser();

	CDnMouseCursor::GetInstance().ShowCursor( true, true );
}

std::wstring CDnPvPRacingResultDlg::GetBestUserName()
{
	std::stable_sort( m_vUserInfo.begin(), m_vUserInfo.end(), CompareLapTime );

	if( false == m_vUserInfo.empty() )
	{
		if( 0 != m_vUserInfo[0].dwLapTime )
			return m_vUserInfo[0].wszUserName;
	}

	return std::wstring();
}

void CDnPvPRacingResultDlg::InitializeSlot()
{
	for( DWORD itr = 0; itr < eSlotCount::eMax_SlotCount; ++itr )
	{
		m_UISlotData[itr].m_pStaticLevel->ClearText();
		m_UISlotData[itr].m_pStaticJob->SetIconID( -1 );

		m_UISlotData[itr].m_pTCGrade->Show( false );
		m_UISlotData[itr].m_pTCGuildMark->Show( false );
		m_UISlotData[itr].m_pTCMedalIcon->Show( false );

		m_UISlotData[itr].m_pStaticGuildName->ClearText();
		m_UISlotData[itr].m_pStaticPlayerName->ClearText();
		m_UISlotData[itr].m_pStaticFinalRanking->ClearText();
		m_UISlotData[itr].m_pStaticLapTime->ClearText();
		m_UISlotData[itr].m_pStaticPCBang->ClearText();
		m_UISlotData[itr].m_pStaticReward->ClearText();
		m_UISlotData[itr].m_pStaticCover->Show( false );
	}
}

std::wstring CDnPvPRacingResultDlg::GetLapTimeString( const DWORD dwLapTime )
{
	if( 0 == dwLapTime )
		return std::wstring( L"-- : -- : --" );

	int nMilliSecond = dwLapTime % 1000;
	int nSecond = ( dwLapTime / 1000 ) % 60;
	int nMinute = ( dwLapTime / 1000 ) / 60;

	WCHAR wszBuffer[256] = {0,};
	std::wstring wszString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 121137 );	// UISTRING : %d : %d : %d

	swprintf_s( wszBuffer, _countof(wszBuffer), wszString.c_str(), nMinute, nSecond, nMilliSecond );
	wszString = std::wstring( wszBuffer );

	return wszString;
}