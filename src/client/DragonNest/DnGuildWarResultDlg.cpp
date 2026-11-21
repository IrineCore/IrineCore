#include "stdafx.h"
#include "DnGuildWarResultDlg.h"
#include "DnGuildWarResultListDlg.h"
#include "DnPlayerActor.h"
#include "TaskManager.h"
#include "DnPvPGameTask.h"
#include "DnBridgeTask.h"
#include "DnMainMenuDlg.h"
#include "DnInterface.h"
#include "SystemSendPacket.h"
#include "DnRevengeTask.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

bool Compare_GuildWar_TotalScore( CDnGuildWarResultDlg::SUserInfo a, CDnGuildWarResultDlg::SUserInfo b )
{
	if( a.uiTotalScore < b.uiTotalScore ) 	return false;
	else if( a.uiTotalScore > b.uiTotalScore )	return true;
	else if( a.uiTotalScore == b.uiTotalScore && a.uiKObyCount < b.uiKObyCount)	return true;
	else return false;

	return false;
}

CDnGuildWarResultDlg::CDnGuildWarResultDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pButtonAll( NULL )
, m_pButtonRed( NULL )
, m_pButtonBlue( NULL )
, m_pButtonClose( NULL )
, m_pListBoxUser( NULL )
, m_IsFinalResult( false )
, m_nBestUserKillCount(0)
, m_nBestUserDeathCount(0)
, m_nBestUserSessionID(0)
, m_fTotal_ElapsedTime( 0.f )
, m_eTeamState( ALL_TEAM )
{

}

CDnGuildWarResultDlg::~CDnGuildWarResultDlg()
{
	m_pListBoxUser->RemoveAllItems();
}

void CDnGuildWarResultDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildWarScoreDlg.ui" ).c_str(), bShow );
}

void CDnGuildWarResultDlg::InitialUpdate()
{
	m_pStaticTeam[RED_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_TEAM0" );
	m_pStaticTeamName[RED_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_GUILDNAME0" );
	m_pStaticBossKill[RED_TEAM] = GetControl<CEtUIStatic>( "ID_RED_BOSSKILL0" );
	m_pStaticBossNotKill[RED_TEAM] = GetControl<CEtUIStatic>( "ID_RED_BOSSKILL1" );
	m_pStaticResource[RED_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_POUNT0" );
	m_pStaticTotalKill[RED_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_KILLCOUNT0" );
	m_pStaticOccupation[RED_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_PINPOINT0" );
	m_pStaticScore[RED_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_COUNT0" );

	m_pStaticTeam[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_TEAM1" );
	m_pStaticTeamName[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_GUILDNAME1" );
	m_pStaticBossKill[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_BLUE_BOSSKILL0" );
	m_pStaticBossNotKill[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_BLUE_BOSSKILL1" );
	m_pStaticResource[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_POUNT1" );
	m_pStaticTotalKill[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_KILLCOUNT1" );
	m_pStaticOccupation[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_PINPOINT1" );
	m_pStaticScore[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_COUNT1" );

	m_pStaticMainName[RED_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_REDNAME" );
	m_pStaticMainBar[RED_TEAM] = GetControl<CEtUIStatic>( "ID_STATIC_REDBAR" );

	m_pStaticMainName[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_BLUENAME" );
	m_pStaticMainBar[BLUE_TEAM] = GetControl<CEtUIStatic>( "ID_STATIC_BLUEBAR" );

	m_pStaticMainName[ALL_TEAM] = GetControl<CEtUIStatic>( "ID_TEXT_ALL" );
	m_pStaticMainBar[ALL_TEAM] = GetControl<CEtUIStatic>( "ID_STATIC_ALLBAR" );

	m_pButtonAll = GetControl<CEtUIRadioButton>( "ID_RBT_ALL" );
	m_pButtonRed = GetControl<CEtUIRadioButton>( "ID_RBT_RED" );
	m_pButtonBlue = GetControl<CEtUIRadioButton>( "ID_RBT_BLUE" );
	m_pButtonClose = GetControl<CEtUIButton>( "ID_BUTTON_CLOSE" );
	m_pButtonClose->Show( false );

	m_pListBoxUser = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIST" );

	m_pButtonAll->SetChecked( true );

	m_pStaticBossKill[RED_TEAM]->Show( false );
	m_pStaticBossKill[BLUE_TEAM]->Show( false );

	m_pStaticMainName[ALL_TEAM]->Show( true );
	m_pStaticMainBar[ALL_TEAM]->Show( true );

	m_pStaticMainName[RED_TEAM]->Show( false );
	m_pStaticMainBar[RED_TEAM]->Show( false );
	m_pStaticMainName[BLUE_TEAM]->Show( false );
	m_pStaticMainBar[BLUE_TEAM]->Show( false );

	CONTROL( Static, ID_TEXT_TEAM0 )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1,120015) );
	CONTROL( Static, ID_TEXT_TEAM1 )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1,120014) );
}

void CDnGuildWarResultDlg::Show( bool bShow )
{
	if( bShow == m_bShow || m_IsFinalResult )
		return;

	if( bShow )
		UpdateUser( m_eTeamState );

	CDnCustomDlg::Show( bShow );
}

void CDnGuildWarResultDlg::Process( float fElapsedTime )
{
	if( m_IsFinalResult ) {
		m_fTotal_ElapsedTime += fElapsedTime;

		if( m_fTotal_ElapsedTime > static_cast<float>(PvPCommon::Common::PvPFinishScoreOpenDelay) )
		{
			if( !IsShow() ) {
				m_pButtonClose->Show( true );
				UpdateUser( m_eTeamState );
				CEtUIDialog::Show( true );	// Force Show
			}

			WCHAR wszCloseMessage[256];
			SecureZeroMemory(wszCloseMessage ,sizeof(wszCloseMessage));
			wsprintf(wszCloseMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1,121013), PvPCommon::Common::PvPFinishAutoClose-static_cast<int>(m_fTotal_ElapsedTime) );
			if( m_pButtonClose )
				m_pButtonClose->SetText(wszCloseMessage );
		}

		if( m_fTotal_ElapsedTime > static_cast<float>(PvPCommon::Common::PvPFinishAutoClose) )
		{
			SendMovePvPGameToPvPLobby();
			m_IsFinalResult = false;
		}
	}

//	if( m_bShow && focus::GetControl() == m_pListBoxUser )
//		focus::ReleaseControl();

	CDnCustomDlg::Process(fElapsedTime);
}

void CDnGuildWarResultDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( IsCmdControl("ID_RBT_ALL" ) )
		{
			m_eTeamState = ALL_TEAM;

			m_pStaticMainName[ALL_TEAM]->Show( true );
			m_pStaticMainBar[ALL_TEAM]->Show( true );

			m_pStaticMainName[BLUE_TEAM]->Show( false );
			m_pStaticMainBar[BLUE_TEAM]->Show( false );
			m_pStaticMainName[RED_TEAM]->Show( false );
			m_pStaticMainBar[RED_TEAM]->Show( false );

			UpdateUser( m_eTeamState );
		}
		else if( IsCmdControl("ID_RBT_BLUE" ) )
		{
			m_eTeamState = BLUE_TEAM;

			m_pStaticMainName[BLUE_TEAM]->Show( true );
			m_pStaticMainBar[BLUE_TEAM]->Show( true );

			m_pStaticMainName[ALL_TEAM]->Show( false );
			m_pStaticMainBar[ALL_TEAM]->Show( false );
			m_pStaticMainName[RED_TEAM]->Show( false );
			m_pStaticMainBar[RED_TEAM]->Show( false );

			UpdateUser( m_eTeamState );
		}
		else if( IsCmdControl("ID_RBT_RED" ) )
		{
			m_eTeamState = RED_TEAM;

			m_pStaticMainName[RED_TEAM]->Show( true );
			m_pStaticMainBar[RED_TEAM]->Show( true );

			m_pStaticMainName[BLUE_TEAM]->Show( false );
			m_pStaticMainBar[BLUE_TEAM]->Show( false );
			m_pStaticMainName[ALL_TEAM]->Show( false );
			m_pStaticMainBar[ALL_TEAM]->Show( false );

			UpdateUser( m_eTeamState );
		}
	}
	else if ( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CLOSE" ) )
		{
			SendMovePvPGameToPvPLobby();
		}
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	focus::ReleaseControl();
}

void CDnGuildWarResultDlg::UpdateUser(  int eTeamState  )
{
	WCHAR wszTemp[256];
	SecureZeroMemory(wszTemp,sizeof(wszTemp));

	if( m_vUserInfo.size() > GuildWar_MaxUserSlot)
	{
		ErrorLog("CDnPVPGameResultAllDlg::UpdateUser() User Num is Wrong");
		return;
	}

	std::sort( m_vUserInfo.begin(), m_vUserInfo.end(), Compare_GuildWar_TotalScore );

	m_pListBoxUser->RemoveAllItems();

	CDnGuildWarResultListDlg * pItem;
	for( UINT itr = 0; itr < m_vUserInfo.size(); itr++ )
	{
		if( eTeamState == m_vUserInfo[itr].cTeam || eTeamState == ALL_TEAM )
		{
			pItem = m_pListBoxUser->InsertItem<CDnGuildWarResultListDlg>(0);
			pItem->SetList( m_vUserInfo[itr] );
		}
	}
}

void CDnGuildWarResultDlg::AddUser( DnActorHandle hUser )
{
	if( hUser->GetTeam() == PvPCommon::Team::Observer )
		return;

	if( hUser )
	{
		CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(hUser.GetPointer());
		if( pPlayerActor && ( ALL_TEAM == m_eTeamState || pPlayerActor->GetTeam() == m_eTeamState ) )
		{
			SUserInfo stUserInfo;
			stUserInfo.nSessionID = pPlayerActor->GetUniqueID();
			stUserInfo.nLevel = pPlayerActor->GetLevel();
			stUserInfo.cJobClassID = pPlayerActor->GetJobClassID();
			stUserInfo.cPVPlevel = pPlayerActor->GetPvPLevel();
			stUserInfo.wszUserName = pPlayerActor->GetName();
			if( pPlayerActor->IsJoinGuild() )
				stUserInfo.GuildSelfView.Set( pPlayerActor->GetGuildSelfView() );

			if( PvPCommon::Team::A == pPlayerActor->GetTeam() )
				stUserInfo.cTeam = BLUE_TEAM;
			else
				stUserInfo.cTeam = RED_TEAM;

#if defined( PRE_ADD_REVENGE )
			CDnRevengeTask * pRevengeTask = (CDnRevengeTask *)CTaskManager::GetInstance().GetTask( "RevengeTask" );
			if( NULL != pRevengeTask && CDnBridgeTask::GetInstance().GetSessionID() != stUserInfo.nSessionID )
				pRevengeTask->GetRevengeUserID( pPlayerActor->GetUniqueID(), stUserInfo.eRevengeUser );
#endif	// #if defined( PRE_ADD_REVENGE )

			m_vUserInfo.push_back(stUserInfo);

			if( IsShow() )
			{
				CDnGuildWarResultListDlg * pItem = m_pListBoxUser->InsertItem<CDnGuildWarResultListDlg>(0);
				pItem->SetList( stUserInfo );
			}
		}
	}
}

void CDnGuildWarResultDlg::RemoveUser( DnActorHandle hUser )
{
	if(m_IsFinalResult)
		return;

	if( hUser )
	{
		for( UINT i = 0; i< m_vUserInfo.size();i++ )
		{
			if( hUser->GetUniqueID() == m_vUserInfo[i].nSessionID )
			{
				CDnGuildWarResultListDlg * pItem;
				for( int itr = 0; itr < m_pListBoxUser->GetSize(); ++itr )
				{
					pItem = m_pListBoxUser->GetItem<CDnGuildWarResultListDlg>( itr );
					if( pItem->GetSessionID() == m_vUserInfo[i].nSessionID )
					{
						if( IsShow() )
							m_pListBoxUser->RemoveItem( itr );
						break;
					}
				}

				m_vUserInfo.erase(m_vUserInfo.begin() + i );
				return;
			}
		}
	}
}

void CDnGuildWarResultDlg::ProcessScoring( int nSessionID, int nKOCount, int nKObyCount, int nKOP )
{
	for( UINT i = 0; i< m_vUserInfo.size();i++ )
	{
		if( nSessionID == m_vUserInfo[i].nSessionID )
		{
			m_vUserInfo[i].uiKOCount += nKOCount;
			m_vUserInfo[i].uiKObyCount += nKObyCount;
			m_vUserInfo[i].uiKOP += nKOP;
			m_vUserInfo[i].uiTotalScore += (nKOP );
			return;
		}
	}
}

void CDnGuildWarResultDlg::FinalResultOpen()
{
	WCHAR wszTemp[256];
	SecureZeroMemory(wszTemp,sizeof(wszTemp));

	Show(false);
	m_IsFinalResult = true;

	CDnMouseCursor::GetInstance().ShowCursor( true, true );
}

void CDnGuildWarResultDlg::SetBoardName(std::wstring wszName)
{

}

void CDnGuildWarResultDlg::SetUserScore( int nSessionID, int nKOCount, int nKObyCount, UINT uiKOP, UINT uiAssistP, UINT uiTotalP, int nOccupation )
{
	for( UINT i = 0; i< m_vUserInfo.size();i++ )
	{
		if( nSessionID == m_vUserInfo[i].nSessionID )
		{
			m_vUserInfo[i].uiKOCount = nKOCount;
			m_vUserInfo[i].uiKObyCount = nKObyCount;
			m_vUserInfo[i].uiKOP = uiKOP;
			m_vUserInfo[i].uiAssistP = uiAssistP;
			m_vUserInfo[i].uiTotalScore = uiTotalP;
			m_vUserInfo[i].uiOccupation = nOccupation;
			return;
		}
	}
}

void CDnGuildWarResultDlg::SetUserState( DnActorHandle hUser , int iState )
{
	if( hUser )
	{
		for( UINT i = 0; i< m_vUserInfo.size();i++ )
		{
			if( hUser->GetUniqueID() == m_vUserInfo[i].nSessionID && m_vUserInfo[i].iState != MINE_S )
			{
				m_vUserInfo[i].iState = iState;
				return;
			}
		}
	}
}

void CDnGuildWarResultDlg::RestartRound()
{
	for( UINT i = 0; i< m_vUserInfo.size();i++ )
	{
		if ( m_vUserInfo[i].iState != MINE_S )
			m_vUserInfo[i].iState = Normal_S;

	}
}

void CDnGuildWarResultDlg::SetPVPXP( UINT nSessionID, UINT uiAddXPScore,UINT uiTotalXPScore,UINT uiMedalScore)
{
	// Could handle in CDnMutatorGame::EndGame, but since each mode needs separate handling, handle when receiving EXP.
	// The XP packet arrives after the result-window-open packet, and besides, medal Exp is split and calculated from experience anyway.
	int nGainMedalCount = 0;
	int nMedalExp = 0;
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP ) {
		nMedalExp = ((CDnPvPGameTask *)pGameTask)->GetMedalExp();
	}

	for( UINT i = 0; i< m_vUserInfo.size();i++ )
	{
		if( nSessionID == m_vUserInfo[i].nSessionID )
		{
			m_vUserInfo[i].uiXP = uiAddXPScore;
			m_vUserInfo[i].uiMedal = uiMedalScore;
			m_vUserInfo[i].uiTotalXP = uiTotalXPScore; 

			if( m_vUserInfo[i].iState == MINE_S )
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

std::wstring CDnGuildWarResultDlg::GetBestUserName()
{
	int UserIndex = 0;

	if(!m_vUserInfo.empty())
	{
		for(int i=0;i<(int)m_vUserInfo.size();i++)
		{
			if(m_vUserInfo[i].uiKOCount >= m_nBestUserKillCount)
			{

				if(m_vUserInfo[i].uiKOCount == m_nBestUserKillCount) // If kill count equals the previous best user's,
				{
					if(m_nBestUserSessionID != m_vUserInfo[i].nSessionID) // Session ID is different < it's another person >
					{
						if((int)m_vUserInfo[i].uiKObyCount >= GetBestUserDeathScore()) // If kill count is equal or higher, not the best user
							continue;
					}
				}

				m_nBestUserKillCount = m_vUserInfo[i].uiKOCount;
				m_nBestUserDeathCount = m_vUserInfo[i].uiKObyCount;
				m_nBestUserSessionID = m_vUserInfo[i].nSessionID;
				UserIndex = i;
			}
		}

		return m_vUserInfo[UserIndex].wszUserName;
	}


	m_nBestUserKillCount = 0;
	return L"";
}

int CDnGuildWarResultDlg::GetBestUserScore()
{
	int nCurrentBestUser = 0;

	if(!m_vUserInfo.empty())
	{
		for(int i=0;i<(int)m_vUserInfo.size();i++)
		{
			if((int)m_vUserInfo[i].uiKOCount >= nCurrentBestUser)
			{
				nCurrentBestUser = m_vUserInfo[i].uiKOCount;
			}
		}
	}
	m_nBestUserKillCount = nCurrentBestUser;

	return m_nBestUserKillCount;
}

int CDnGuildWarResultDlg::GetBestUserDeathScore()
{
	int nCurrentBestUserDeathCount = 1000;

	if(!m_vUserInfo.empty())
	{
		for(int i=0;i<(int)m_vUserInfo.size();i++)
		{
			if((int)m_vUserInfo[i].uiKOCount == GetBestUserScore())
			{
				if((int)m_vUserInfo[i].uiKObyCount < nCurrentBestUserDeathCount)
				{
					nCurrentBestUserDeathCount = (int)m_vUserInfo[i].uiKObyCount;
					m_nBestUserSessionID = (int)m_vUserInfo[i].nSessionID;
				}
			}
		}
	}

	m_nBestUserDeathCount = nCurrentBestUserDeathCount;
	return m_nBestUserDeathCount;
}

bool CDnGuildWarResultDlg::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( !IsShow() )
		return false;

	switch( uMsg )
	{
	case WM_LBUTTONUP:
		if( m_bShow && focus::GetControl() == m_pListBoxUser )
			focus::ReleaseControl();
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnGuildWarResultDlg::SetData( STeamData & sData )
{
	m_pStaticTeamName[RED_TEAM]->SetText( sData.wszRedGuildName );
	m_pStaticResource[RED_TEAM]->SetIntToText( sData.nRedTeamResource );
	m_pStaticTotalKill[RED_TEAM]->SetIntToText( sData.nRedTeamKill );
	m_pStaticOccupation[RED_TEAM]->SetIntToText( sData.nRedOccupation );
	m_pStaticScore[RED_TEAM]->SetIntToText( sData.nRedTeamScore );

	if( sData.bRedBossKill )
	{
		m_pStaticBossKill[RED_TEAM]->Show( true );
		m_pStaticBossNotKill[RED_TEAM]->Show( false );
	}
	else
	{
		m_pStaticBossKill[RED_TEAM]->Show( false );
		m_pStaticBossNotKill[RED_TEAM]->Show( true );
	}

	m_pStaticTeamName[BLUE_TEAM]->SetText( sData.wszBlueGuildName );
	m_pStaticResource[BLUE_TEAM]->SetIntToText( sData.nBlueTeamResource );
	m_pStaticTotalKill[BLUE_TEAM]->SetIntToText( sData.nBlueTeamKill );
	m_pStaticOccupation[BLUE_TEAM]->SetIntToText( sData.nBlueOccupation );
	m_pStaticScore[BLUE_TEAM]->SetIntToText( sData.nBlueTeamScore );

	if( sData.bBlueBossKill )
	{
		m_pStaticBossKill[BLUE_TEAM]->Show( true );
		m_pStaticBossNotKill[BLUE_TEAM]->Show( false );
	}
	else
	{
		m_pStaticBossKill[BLUE_TEAM]->Show( false );
		m_pStaticBossNotKill[BLUE_TEAM]->Show( true );
	}
}

CDnGuildWarResultDlg::SUserInfo CDnGuildWarResultDlg::GetUserInfo( const int nSessionID )
{
	CDnGuildWarResultDlg::SUserInfo sUserInfo;
	for( DWORD itr = 0; itr < m_vUserInfo.size(); ++itr )
	{
		if( nSessionID == m_vUserInfo[itr].nSessionID )
			sUserInfo = m_vUserInfo[itr];
	}

	return sUserInfo;
}
