#include "StdAfx.h"
#include "DnPVPGameResultDlg.h"
#include "DnPlayerActor.h"
#include "DnBridgeTask.h"
#include "DnTooltipDlg.h"
#include "Dninterface.h"
#include "TaskManager.h"
#include "DnPvPGameTask.h"
#include "DnMainMenuDlg.h"
#include "SystemSendPacket.h"
#include "DnGuildTask.h"
#include "DnRevengeTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

bool Compare_TotalScore( CDnPVPGameResultDlg::SUserInfo a, CDnPVPGameResultDlg::SUserInfo b )
{
	if( a.uiTotalScore < b.uiTotalScore ) return false;
	else if( a.uiTotalScore > b.uiTotalScore ) return true;
	if( a.uiTotalScore < b.uiTotalScore ) return false;
	return false;
}

#if defined( PRE_ADD_REVENGE )
bool Compare_RevengeUser( CDnPVPGameResultDlg::SUserInfo a, CDnPVPGameResultDlg::SUserInfo b )
{
	return a.eRevengeUser > b.eRevengeUser;
}
#endif	// #if defined( PRE_ADD_REVENGE )

CDnPVPGameResultDlg::CDnPVPGameResultDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	m_bAutoCursor = true;

	for(int iPlayerNum = 0 ; iPlayerNum < PvP_TeamUserSlot ; iPlayerNum++)
	{
		m_pMyTeamLevel[iPlayerNum] = NULL;
		m_pMyJobIcon[iPlayerNum] = NULL;
		m_pMySlotRank[iPlayerNum] = NULL;
		m_pMyGuildMark[iPlayerNum] = NULL;
		m_pMyGuildName[iPlayerNum] = NULL;
		m_pMyTeamName[iPlayerNum] = NULL;
		m_pMyKOCount[iPlayerNum] = NULL;
		m_pMyKObyCount[iPlayerNum] = NULL;
		m_pMyAssistPoint[iPlayerNum] = NULL;
		m_pMyTotalPoint[iPlayerNum] = NULL;
		m_pMyXP[iPlayerNum] = NULL;
		m_pMyPCRoom[iPlayerNum] = NULL;
		m_pMyMedalIcon[iPlayerNum] = NULL;
		m_pMyMedal[iPlayerNum] = NULL;
		m_pStaticMy[iPlayerNum] = NULL;

		m_pEnemyTeamLevel[iPlayerNum] = NULL;
		m_pEnemyJobIcon[iPlayerNum] = NULL;	
		m_pEnemySlotRank[iPlayerNum] = NULL;
		m_pEnemyGuildMark[iPlayerNum] = NULL;
		m_pEnemyGuildName[iPlayerNum] = NULL;
		m_pEnemyTeamName[iPlayerNum] = NULL;
		m_pEnemyKOCount[iPlayerNum] = NULL;
		m_pEnemyKObyCount[iPlayerNum] = NULL;
		m_pEnemyAssistPoint[iPlayerNum] = NULL;
		m_pEnemyTotalPoint[iPlayerNum] = NULL;
		m_pEnemyXP[iPlayerNum] = NULL;
		m_pEnemyPCRoom[iPlayerNum] = NULL;
		m_pEnemyMedalIcon[iPlayerNum] = NULL;
		m_pEnemyMedal[iPlayerNum] = NULL;

		m_pMyBossKOCount[iPlayerNum] = NULL;
		m_pEnemyBossKOCount[iPlayerNum] = NULL;

	}

	int Max_TeamNum = 2;
	for(int i=0;i<Max_TeamNum;i++)
	{
		m_pKillBoss[i] = NULL;
		m_pXpCount[i] = NULL;
		m_pPCRoom[i] = NULL;
		m_pMedalReward[i] = NULL;
		m_pKillUser[i] = NULL;
		m_pDeath[i] = NULL;
		m_pAssist[i] = NULL;
	}
	
	m_bIsLadder = false;
	m_bIsOccupation = false;
	m_IsFinalResult = false;
	m_pButtonClose = NULL;
	m_fTotal_ElapsedTime = 0.0f;

#ifdef PRE_MOD_PVPOBSERVER
	m_pStaticFriendly = NULL;
	m_pStaticEnemy = NULL;
	m_bShowResult = false;
#endif // PRE_MOD_PVPOBSERVER

}

CDnPVPGameResultDlg::~CDnPVPGameResultDlg(void)
{
	SAFE_RELEASE_SPTR( m_hPVPMedalIconImage );
}

void CDnPVPGameResultDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpScore.ui" ).c_str(), bShow );
}

void CDnPVPGameResultDlg::InitialUpdate()
{
	char szUILevelId[100],szUIJobIcon[100],szUISlotRank[100],szUISlotMark[100],szUIGuildNameId[100],szUINameId[100],szUIKOCount[100],szUIKObyCount[100];
	char szUIAssistScore[100],szUIScore[100],szUIXP[100],szUIPCRoom[100],szUIMedalIcon[100],szUIMedal[100];
	char szUIStaticMy[100];
	char szUIBossKOCount[100];


	SecureZeroMemory(szUIBossKOCount,sizeof(szUIBossKOCount));
	SecureZeroMemory(szUILevelId,sizeof(szUILevelId));
	SecureZeroMemory(szUIJobIcon,sizeof(szUIJobIcon));
	SecureZeroMemory(szUISlotRank,sizeof(szUISlotRank));
	SecureZeroMemory(szUISlotMark,sizeof(szUISlotMark));
	SecureZeroMemory(szUIGuildNameId,sizeof(szUIGuildNameId));
	SecureZeroMemory(szUINameId,sizeof(szUINameId));
	SecureZeroMemory(szUIKOCount,sizeof(szUIKOCount));
	SecureZeroMemory(szUIKObyCount,sizeof(szUIKObyCount));
	SecureZeroMemory(szUIAssistScore,sizeof(szUIAssistScore));
	SecureZeroMemory(szUIScore,sizeof(szUIScore));
	SecureZeroMemory(szUIXP,sizeof(szUIXP));
	SecureZeroMemory(szUIPCRoom,sizeof(szUIPCRoom));
	SecureZeroMemory(szUIMedalIcon,sizeof(szUIMedalIcon));
	SecureZeroMemory(szUIMedal,sizeof(szUIMedal));
	SecureZeroMemory(szUIStaticMy,sizeof(szUIStaticMy));

	m_pButtonClose = GetControl<CEtUIButton>("ID_BUTTON_CLOSE");
	m_pButtonClose->Show( false );

	SAFE_RELEASE_SPTR( m_hPVPMedalIconImage );
	m_hPVPMedalIconImage = LoadResource( CEtResourceMng::GetInstance().GetFullName( "Pvp_Medal.dds" ).c_str(), RT_TEXTURE );

	for(int iTeamNum = 0 ; iTeamNum < 2 ; iTeamNum++)
	{
		for(int iPlayerNum = 0 ; iPlayerNum < PvP_TeamUserSlot ; iPlayerNum++)
		{
			if( iTeamNum == 0 )
			{
				sprintf(szUILevelId,"ID_SCORE_BLUE_LEVEL%d",iPlayerNum);
				sprintf(szUIJobIcon,"ID_SCORE_BLUE_JOB%d",iPlayerNum);
				sprintf(szUISlotRank,"ID_TEXTURE_BLUE_RANK%d",iPlayerNum);
				sprintf(szUISlotMark,"ID_TEXTURE_BLUE_MARK%d",iPlayerNum);
				sprintf(szUIGuildNameId,"ID_SCORE_BLUE_GUILDNAME%d",iPlayerNum);
				sprintf(szUINameId,"ID_SCORE_BLUE_NAME%d",iPlayerNum);
				sprintf(szUIKOCount,"ID_SCORE_BLUE_KILLCOUND%d",iPlayerNum);
				sprintf(szUIKObyCount,"ID_SCORE_BLUE_DEATH%d",iPlayerNum);
				sprintf(szUIAssistScore,"ID_SCORE_BLUE_ASSIST%d",iPlayerNum);
				sprintf(szUIScore,"ID_SCORE_BLUE_SCORE%d",iPlayerNum);
				sprintf(szUIXP,"ID_SCORE_BLUE_PVPXP%d",iPlayerNum);
				sprintf(szUIPCRoom,"ID_SCORE_BLUE_PC%d",iPlayerNum);
				sprintf(szUIMedalIcon,"ID_SCORE_BLUE_MEDALICON%d",iPlayerNum);
				sprintf(szUIMedal,"ID_SCORE_BLUE_MEDAL%d",iPlayerNum);
				sprintf(szUIStaticMy,"ID_STATIC_ME%d",iPlayerNum);	// Only this is the Static version on our side.
				sprintf(szUIBossKOCount,"ID_SCORE_BLUE_KILLBOSS%d",iPlayerNum);
			}
			else
			{
				sprintf(szUILevelId,"ID_SCORE_RED_LEVEL%d",iPlayerNum);
				sprintf(szUIJobIcon,"ID_SCORE_RED_JOB%d",iPlayerNum);
				sprintf(szUISlotRank,"ID_TEXTURE_RED_RANK%d",iPlayerNum);
				sprintf(szUISlotMark,"ID_TEXTURE_RED_MARK%d",iPlayerNum);
				sprintf(szUIGuildNameId,"ID_SCORE_RED_GUILDNAME%d",iPlayerNum);
				sprintf(szUINameId,"ID_SCORE_RED_NAME%d",iPlayerNum);
				sprintf(szUIKOCount,"ID_SCORE_RED_KILLCOUND%d",iPlayerNum);
				sprintf(szUIKObyCount,"ID_SCORE_RED_DEATH%d",iPlayerNum);
				sprintf(szUIAssistScore,"ID_SCORE_RED_ASSIST%d",iPlayerNum);
				sprintf(szUIScore,"ID_SCORE_RED_SCORE%d",iPlayerNum);
				sprintf(szUIXP,"ID_SCORE_RED_PVPXP%d",iPlayerNum);
				sprintf(szUIPCRoom,"ID_SCORE_RED_PC%d",iPlayerNum);
				sprintf(szUIMedalIcon,"ID_SCORE_RED_MEDALICON%d",iPlayerNum);
				sprintf(szUIMedal,"ID_SCORE_RED_MEDAL%d",iPlayerNum);
				sprintf(szUIBossKOCount,"ID_SCORE_RED_KILLBOSS%d",iPlayerNum);
			}

			if( iTeamNum == 0 )
			{
				m_pMyTeamLevel[iPlayerNum] = GetControl<CEtUIStatic>( szUILevelId );
				m_pMyJobIcon[iPlayerNum] = GetControl<CDnJobIconStatic>( szUIJobIcon );
				m_pMySlotRank[iPlayerNum] = GetControl<CEtUITextureControl>( szUISlotRank );
				m_pMyGuildMark[iPlayerNum] = GetControl<CEtUITextureControl>( szUISlotMark );
				m_pMyGuildName[iPlayerNum] = GetControl<CEtUIStatic>( szUIGuildNameId );
				m_pMyTeamName[iPlayerNum] = GetControl<CEtUIStatic>( szUINameId );
				m_pMyKOCount[iPlayerNum] = GetControl<CEtUIStatic>( szUIKOCount );
				m_pMyKObyCount[iPlayerNum] = GetControl<CEtUIStatic>( szUIKObyCount );
				m_pMyAssistPoint[iPlayerNum] = GetControl<CEtUIStatic>( szUIAssistScore );
				m_pMyTotalPoint[iPlayerNum] = GetControl<CEtUIStatic>( szUIScore );
				m_pMyXP[iPlayerNum] = GetControl<CEtUIStatic>( szUIXP );
				m_pMyPCRoom[iPlayerNum] = GetControl<CEtUIStatic>( szUIPCRoom );
				m_pMyMedalIcon[iPlayerNum] = GetControl<CEtUITextureControl>( szUIMedalIcon );
				m_pMyMedal[iPlayerNum] = GetControl<CEtUIStatic>( szUIMedal );
				m_pStaticMy[iPlayerNum] = GetControl<CEtUIStatic>( szUIStaticMy );
				m_pMyBossKOCount[iPlayerNum] = GetControl<CEtUIStatic>( szUIBossKOCount );
				m_pMyBossKOCount[iPlayerNum]->Show(false); // Hide because it is only used in Captain Mode.
			}
			else
			{
				m_pEnemyTeamLevel[iPlayerNum] = GetControl<CEtUIStatic>( szUILevelId );
				m_pEnemyJobIcon[iPlayerNum] = GetControl<CDnJobIconStatic>( szUIJobIcon );
				m_pEnemySlotRank[iPlayerNum] = GetControl<CEtUITextureControl>( szUISlotRank );
				m_pEnemyGuildMark[iPlayerNum] = GetControl<CEtUITextureControl>( szUISlotMark );
				m_pEnemyGuildName[iPlayerNum] = GetControl<CEtUIStatic>( szUIGuildNameId );
				m_pEnemyTeamName[iPlayerNum] = GetControl<CEtUIStatic>( szUINameId );
				m_pEnemyKOCount[iPlayerNum] = GetControl<CEtUIStatic>( szUIKOCount );
				m_pEnemyKObyCount[iPlayerNum] = GetControl<CEtUIStatic>( szUIKObyCount );
				m_pEnemyAssistPoint[iPlayerNum] = GetControl<CEtUIStatic>( szUIAssistScore );
				m_pEnemyTotalPoint[iPlayerNum] = GetControl<CEtUIStatic>( szUIScore );
				m_pEnemyXP[iPlayerNum] = GetControl<CEtUIStatic>( szUIXP );
				m_pEnemyPCRoom[iPlayerNum] = GetControl<CEtUIStatic>( szUIPCRoom );
				m_pEnemyMedalIcon[iPlayerNum] = GetControl<CEtUITextureControl>( szUIMedalIcon );
				m_pEnemyMedal[iPlayerNum] = GetControl<CEtUIStatic>( szUIMedal );
				m_pEnemyBossKOCount[iPlayerNum] = GetControl<CEtUIStatic>( szUIBossKOCount );
				m_pEnemyBossKOCount[iPlayerNum]->Show(false); // Hide because it is only used in Captain Mode.
			}
		}
	}

	m_pKillBoss[PvPCommon::TeamIndex::A] = GetControl<CEtUIStatic>("ID_BLUE_KILLBOSS");
	m_pKillBoss[PvPCommon::TeamIndex::A]->Show(false);
	m_pKillBoss[PvPCommon::TeamIndex::B] = GetControl<CEtUIStatic>("ID_RED_KILLBOSS");
	m_pKillBoss[PvPCommon::TeamIndex::B]->Show(false);

	m_pKillUser[PvPCommon::TeamIndex::A] = GetControl<CEtUIStatic>("ID_BLUE_KILLCOUND");
	m_pKillUser[PvPCommon::TeamIndex::B] = GetControl<CEtUIStatic>("ID_RED_KILLCOUND");

	m_pDeath[PvPCommon::TeamIndex::A] = GetControl<CEtUIStatic>("ID_BLUE_DEATH");
	m_pDeath[PvPCommon::TeamIndex::B] = GetControl<CEtUIStatic>("ID_RED_DEATH");

	m_pAssist[PvPCommon::TeamIndex::A] = GetControl<CEtUIStatic>("ID_BLUE_ASSIST");
	m_pAssist[PvPCommon::TeamIndex::B] = GetControl<CEtUIStatic>("ID_RED_ASSIST");

	m_pXpCount[PvPCommon::TeamIndex::A] = GetControl<CEtUIStatic>("ID_BLUE_PVPXP");
	m_pXpCount[PvPCommon::TeamIndex::B] = GetControl<CEtUIStatic>("ID_RED__PVPXP");
	m_pPCRoom[PvPCommon::TeamIndex::A] = GetControl<CEtUIStatic>("ID_BLUE_PC");
	m_pPCRoom[PvPCommon::TeamIndex::B] = GetControl<CEtUIStatic>("ID_RED_PC");
	m_pMedalReward[PvPCommon::TeamIndex::A] = GetControl<CEtUIStatic>("ID_BLUE_MEDAL");
	m_pMedalReward[PvPCommon::TeamIndex::B] = GetControl<CEtUIStatic>("ID_RED_MEDAL");

#ifdef PRE_MOD_PVPOBSERVER
	m_pStaticFriendly = GetControl< CEtUIStatic >("ID_STATIC1");
	m_pStaticEnemy = GetControl< CEtUIStatic >("ID_STATIC0");	
	m_bShowResult = true;
#endif // PRE_MOD_PVPOBSERVER

}

void CDnPVPGameResultDlg::DisableAssist(bool bTrue)
{
	for(int iPlayerNum = 0 ; iPlayerNum < PvP_TeamUserSlot ; iPlayerNum++)
	{
		m_pMyAssistPoint[iPlayerNum]->Show(!bTrue); 
		m_pEnemyAssistPoint[iPlayerNum]->Show(!bTrue); 
	}

	for(int iTeamNum = 0 ; iTeamNum < 2 ; iTeamNum++)
		m_pAssist[iTeamNum]->Show(false);
}
void CDnPVPGameResultDlg::EnableCaptainMode(bool bTrue)
{
	if(!bTrue)
		return;

	for(int iTeamNum = 0 ; iTeamNum < 2 ; iTeamNum++)
	{
		for(int iPlayerNum = 0 ; iPlayerNum < PvP_TeamUserSlot ; iPlayerNum++)
		{
			m_pMyBossKOCount[iPlayerNum]->Show(true); 
			m_pEnemyBossKOCount[iPlayerNum]->Show(true); 
		}
		m_pKillBoss[iTeamNum]->Show(true);
		m_pKillBoss[iTeamNum]->Show(true);
	}
}
void CDnPVPGameResultDlg::EnableLadderMode( bool bTrue )
{
	m_bIsLadder = bTrue;
	if( !bTrue )
		return;

	for( int iTeamNum = 0 ; iTeamNum < 2 ; iTeamNum++ )
	{
#ifdef PRE_MOD_PVP_LADDER_XP
		m_pXpCount[iTeamNum]->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126020 ) ); // 126020 : Rating
		m_pPCRoom[iTeamNum]->Show( false );
		m_pMedalReward[iTeamNum]->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120217 ) ); // 120217 : Pvp Exp
#else // PRE_MOD_PVP_LADDER_XP
		m_pXpCount[iTeamNum]->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126020 ) ); // 126020 : Rating
		m_pPCRoom[iTeamNum]->Show( false );
		m_pMedalReward[iTeamNum]->Show( false );
#endif // PRE_MOD_PVP_LADDER_XP
	}
}

void CDnPVPGameResultDlg::EnableOccupationMode( bool bTrue )
{
	m_bIsOccupation = bTrue;
	if( !m_bIsOccupation )
		return;

	for( int itr = 0; itr < 2; ++itr )
	{
		m_pKillUser[itr]->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120139) );	// Capture
		m_pKillUser[itr]->Show( true );

		m_pDeath[itr]->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120140) );	// Conquest
		m_pDeath[itr]->Show( true );

		m_pAssist[itr]->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 121051) );		// Kill
		m_pAssist[itr]->Show( true );
	}
}

void CDnPVPGameResultDlg::SetOccupationScore( int nSessionID, UINT uiOccupation, UINT uiOccupationSteal )
{
	for( UINT i = 0; i< m_vMyUserInfo.size();i++ )
	{
		if( nSessionID == m_vMyUserInfo[i].nSessionID )
		{
			m_vMyUserInfo[i].uiOccupationCount = uiOccupation;
			m_vMyUserInfo[i].uiOccupationCancelCount = uiOccupationSteal;
			return;
		}
	}

	for( UINT i = 0; i< m_vEnemyTeamUserInfo.size();i++ )
	{
		if( nSessionID == m_vEnemyTeamUserInfo[i].nSessionID )
		{
			m_vEnemyTeamUserInfo[i].uiOccupationCount = uiOccupation;
			m_vEnemyTeamUserInfo[i].uiOccupationCancelCount = uiOccupationSteal;
			return;
		}
	}
}

void CDnPVPGameResultDlg::RestartRound()
{
	for( UINT i = 0; i< m_vMyUserInfo.size();i++ )
	{
		if ( m_vMyUserInfo[i].iState != MINE_S )
			m_vMyUserInfo[i].iState = Normal_S;
	}

	for( UINT i = 0; i< m_vEnemyTeamUserInfo.size();i++ )
	{
		m_vEnemyTeamUserInfo[i].iState = Normal_S;
	}
}

void CDnPVPGameResultDlg::AddUser( DnActorHandle hUser , bool IsMyTeam )
{
	if( hUser && hUser->IsPlayerActor() && hUser->GetTeam() != PvPCommon::Team::Observer)
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hUser.GetPointer());
		if( pPlayerActor && !pPlayerActor->IsObserver() )
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

			if( IsMyTeam )
				m_vMyUserInfo.push_back(stUserInfo);
			else
				m_vEnemyTeamUserInfo.push_back(stUserInfo);
		}
	}
}

void CDnPVPGameResultDlg::SetUserState( DnActorHandle hUser, int iState )
{
	if( hUser )
	{
		for( UINT i = 0; i< m_vMyUserInfo.size();i++ )
		{
			if( hUser->GetUniqueID() == m_vMyUserInfo[i].nSessionID && m_vMyUserInfo[i].iState != MINE_S )
			{
				m_vMyUserInfo[i].iState = iState;
				return;
			}
		}

		for( UINT i = 0; i< m_vEnemyTeamUserInfo.size();i++ )
		{
			if( hUser->GetUniqueID() == m_vEnemyTeamUserInfo[i].nSessionID )
			{				
				m_vEnemyTeamUserInfo[i].iState = iState;
				return;
			}
		}				
	}
}

void CDnPVPGameResultDlg::RemoveUser( DnActorHandle hUser )
{
	if( m_IsFinalResult )
		return;

	if( hUser )
	{
		for( UINT i = 0; i< m_vMyUserInfo.size();i++ )
		{
			if( hUser->GetUniqueID() == m_vMyUserInfo[i].nSessionID )
			{
				m_vMyUserInfo.erase(m_vMyUserInfo.begin() + i );
				return;
			}
		}

		for( UINT i = 0; i< m_vEnemyTeamUserInfo.size();i++ )
		{
			if( hUser->GetUniqueID() == m_vEnemyTeamUserInfo[i].nSessionID )
			{
				m_vEnemyTeamUserInfo.erase( m_vEnemyTeamUserInfo.begin() + i );
				return;
			}
		}				
	}
}

void CDnPVPGameResultDlg::FinalResultOpen( )
{
	WCHAR wszTemp[256];
	SecureZeroMemory(wszTemp,sizeof(wszTemp));

	// When FinalResultOpen happens, force-close first,
	// Ignore the bShow argument in subsequent Show calls triggered by user.
	// And control it directly in Process.
	Show(false);
	m_IsFinalResult = true;


	// Switch cursor mode
	CDnMouseCursor::GetInstance().ShowCursor( true, true );
}

void CDnPVPGameResultDlg::Show( bool bShow )
{ 
	if( bShow == m_bShow || m_IsFinalResult )
		return;

#ifdef PRE_MOD_PVPOBSERVER
	if( !bShow )
		m_bShowResult = true;
#endif // PRE_MOD_PVPOBSERVER


	CEtUIDialog::Show( bShow );
}

void CDnPVPGameResultDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
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

void CDnPVPGameResultDlg::ProcessScoring( int nSessionID, int nKOCount, int nKObyCount, int nKOP, int nAssistP )
{// This function seems unused. Monitor and delete later.
	
	for( UINT i = 0; i< m_vMyUserInfo.size();i++ )
	{
		if( nSessionID == m_vMyUserInfo[i].nSessionID )
		{
			m_vMyUserInfo[i].uiKOCount += nKOCount;
			m_vMyUserInfo[i].uiKObyCount += nKObyCount;
			m_vMyUserInfo[i].uiKOP += nKOP;
			m_vMyUserInfo[i].uiAssistP += nAssistP;
			m_vMyUserInfo[i].uiTotalScore += (nKOP + nAssistP);
			m_vMyUserInfo[i].uiBossKOCount += nKOCount;
			return;
		}
	}

	for( UINT i = 0; i< m_vEnemyTeamUserInfo.size();i++ )
	{
		if( nSessionID == m_vEnemyTeamUserInfo[i].nSessionID )
		{
			m_vEnemyTeamUserInfo[i].uiKOCount += nKOCount;
			m_vEnemyTeamUserInfo[i].uiKObyCount += nKObyCount;
			m_vEnemyTeamUserInfo[i].uiKOP += nKOP;
			m_vEnemyTeamUserInfo[i].uiAssistP += nAssistP;
			m_vEnemyTeamUserInfo[i].uiTotalScore += (nKOP + nAssistP);			
			m_vEnemyTeamUserInfo[i].uiBossKOCount += nKOCount;
			return;
		}
	}

}

void CDnPVPGameResultDlg::SetUserScore( int nSessionID, int nKOCount, int nKObyCount,int nCaptainKOCount, UINT uiKOP, UINT uiAssistP, UINT uiTotalP )
{
	for( UINT i = 0; i< m_vMyUserInfo.size();i++ )
	{
		if( nSessionID == m_vMyUserInfo[i].nSessionID )
		{
			m_vMyUserInfo[i].uiKOCount = nKOCount;
			m_vMyUserInfo[i].uiKObyCount = nKObyCount;
			m_vMyUserInfo[i].uiKOP = uiKOP;
			m_vMyUserInfo[i].uiAssistP = uiAssistP;
			m_vMyUserInfo[i].uiTotalScore = uiTotalP;
			m_vMyUserInfo[i].uiBossKOCount = nCaptainKOCount;
			return;
		}
	}

	for( UINT i = 0; i< m_vEnemyTeamUserInfo.size();i++ )
	{
		if( nSessionID == m_vEnemyTeamUserInfo[i].nSessionID )
		{
			m_vEnemyTeamUserInfo[i].uiKOCount = nKOCount;
			m_vEnemyTeamUserInfo[i].uiKObyCount = nKObyCount;
			m_vEnemyTeamUserInfo[i].uiKOP = uiKOP;
			m_vEnemyTeamUserInfo[i].uiAssistP = uiAssistP;
			m_vEnemyTeamUserInfo[i].uiTotalScore = uiTotalP;
			m_vEnemyTeamUserInfo[i].uiBossKOCount = nCaptainKOCount;
			return;
		}
	}
}

void CDnPVPGameResultDlg::InitializeSlot()
{
	for(int iPlayerNum = 0 ; iPlayerNum < PvP_TeamUserSlot ; iPlayerNum++)
	{
		m_pMyTeamLevel[iPlayerNum]->ClearText();
		m_pMyJobIcon[iPlayerNum]->SetIconID( -1 );
		m_pMySlotRank[iPlayerNum]->Show( false );
		m_pMyGuildMark[iPlayerNum]->Show( false );
		m_pMyGuildName[iPlayerNum]->ClearText();
		m_pMyTeamName[iPlayerNum]->ClearText();
		m_pMyKOCount[iPlayerNum]->ClearText();
		m_pMyKObyCount[iPlayerNum]->ClearText();
		m_pMyAssistPoint[iPlayerNum]->ClearText();
		m_pMyTotalPoint[iPlayerNum]->ClearText();
		m_pMyXP[iPlayerNum]->ClearText();
		m_pMyPCRoom[iPlayerNum]->ClearText();
		m_pMyMedalIcon[iPlayerNum]->Show( false );
		m_pMyMedal[iPlayerNum]->ClearText();
		m_pStaticMy[iPlayerNum]->Show( false );

		m_pEnemyTeamLevel[iPlayerNum]->ClearText();
		m_pEnemyJobIcon[iPlayerNum]->SetIconID( -1 );
		m_pEnemySlotRank[iPlayerNum]->Show( false );
		m_pEnemyGuildMark[iPlayerNum]->Show( false );
		m_pEnemyGuildName[iPlayerNum]->ClearText();
		m_pEnemyTeamName[iPlayerNum]->ClearText();
		m_pEnemyKOCount[iPlayerNum]->ClearText();
		m_pEnemyKObyCount[iPlayerNum]->ClearText();
		m_pEnemyAssistPoint[iPlayerNum]->ClearText();
		m_pEnemyTotalPoint[iPlayerNum]->ClearText();
		m_pEnemyXP[iPlayerNum]->ClearText();
		m_pEnemyPCRoom[iPlayerNum]->ClearText();
		m_pEnemyMedalIcon[iPlayerNum]->Show( false );
		m_pEnemyMedal[iPlayerNum]->ClearText();

		m_pMyBossKOCount[iPlayerNum]->ClearText();
		m_pEnemyBossKOCount[iPlayerNum]->ClearText();
	}
}

void CDnPVPGameResultDlg::UpdateUser()
{
	WCHAR wszTemp[256];
	SecureZeroMemory(wszTemp,sizeof(wszTemp));

	if( m_vMyUserInfo.size() > PvP_TeamUserSlot || m_vEnemyTeamUserInfo.size() > PvP_TeamUserSlot )
	{
		ErrorLog("CDnPVPGameResultDlg::UpdateUser() User Num is Wrong");
		return;
	}

	std::sort( m_vMyUserInfo.begin(), m_vMyUserInfo.end(), Compare_TotalScore );
	std::sort( m_vEnemyTeamUserInfo.begin(), m_vEnemyTeamUserInfo.end(), Compare_TotalScore );

#if defined( PRE_ADD_REVENGE )
	std::sort( m_vMyUserInfo.begin(), m_vMyUserInfo.end(), Compare_RevengeUser );
	std::sort( m_vEnemyTeamUserInfo.begin(), m_vEnemyTeamUserInfo.end(), Compare_RevengeUser );
#endif	// #if defined( PRE_ADD_REVENGE )

	InitializeSlot();

	for( UINT i = 0; i< m_vMyUserInfo.size(); i++ )
	{
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
		wsprintf(wszTemp, L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), m_vMyUserInfo[i].nLevel);
#else
		wsprintf(wszTemp, L"LV %d", m_vMyUserInfo[i].nLevel);
#endif 
		m_pMyTeamLevel[i]->SetText(wszTemp);
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		if( m_vMyUserInfo[i].cJobClassID > 0 )
			m_pMyJobIcon[i]->SetIconID( m_vMyUserInfo[i].cJobClassID, true );

		//Icon
		int iIconW,iIconH;
		int iU,iV;
		iIconW = GetInterface().GeticonWidth();
		iIconH = GetInterface().GeticonHeight();
		if( GetInterface().ConvertPVPGradeToUV( m_vMyUserInfo[i].cPVPlevel, iU, iV ))
		{
			m_pMySlotRank[i]->SetTexture(GetInterface().GetPVPIconTex(), iU, iV, iIconW, iIconH);
			m_pMySlotRank[i]->Show(true);
		}

		if( m_vMyUserInfo[i].GuildSelfView.IsSet() ) 
		{	
			{
				// Guild name
				m_pMyGuildName[i]->SetText( m_vMyUserInfo[i].GuildSelfView.wszGuildName );
				m_pMyGuildName[i]->Show( true );

				// Guild mark
				const TGuildView &GuildView = m_vMyUserInfo[i].GuildSelfView;
				if( GetGuildTask().IsShowGuildMark( GuildView ) )
				{
					EtTextureHandle hGuildMark = GetGuildTask().GetGuildMarkTexture( GuildView );
					m_pMyGuildMark[i]->SetTexture( hGuildMark );
					m_pMyGuildMark[i]->Show( true );
				}
			}		
		}


		// Name
		if ( m_vMyUserInfo[i].iState == Normal_S )
			m_pMyTeamName[i]->SetTextColor(EtInterface::textcolor::WHITE);
		else if ( m_vMyUserInfo[i].iState == MINE_S )
			m_pMyTeamName[i]->SetTextColor(EtInterface::textcolor::GOLD);
		else if ( KILLED_S == m_vMyUserInfo[i].iState )
			m_pMyTeamName[i]->SetTextColor(EtInterface::textcolor::RED);

#if defined( PRE_ADD_REVENGE )
		if( MINE_S != m_vMyUserInfo[i].iState )
		{
			CDnRevengeTask * pRevengeTask = (CDnRevengeTask *)CTaskManager::GetInstance().GetTask( "RevengeTask" );
			if( NULL != pRevengeTask )
				pRevengeTask->GetRevengeUserID( m_vMyUserInfo[i].nSessionID, m_vMyUserInfo[i].eRevengeUser );

			if( Revenge::RevengeTarget::eRevengeTarget_Target == m_vMyUserInfo[i].eRevengeUser )
				m_pMyTeamName[i]->SetTextColor( EtInterface::textcolor::PVP_REVENGE_TARGET );
			else if( Revenge::RevengeTarget::eRevengeTarget_Me == m_vMyUserInfo[i].eRevengeUser )
				m_pMyTeamName[i]->SetTextColor( EtInterface::textcolor::PVP_REVENGE_ME );
			else
				m_pMyTeamName[i]->SetTextColor( EtInterface::textcolor::WHITE );
		}
#endif	//	#if defined( PRE_ADD_REVENGE )

		m_pMyTeamName[i]->SetText( m_vMyUserInfo[i].wszUserName.c_str() );
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		// From kills downward..
		m_pMyKOCount[i]->SetIntToText(m_vMyUserInfo[i].uiKOCount);
		m_pMyKObyCount[i]->SetIntToText(m_vMyUserInfo[i].uiKObyCount);

		m_pMyBossKOCount[i]->SetIntToText(m_vMyUserInfo[i].uiBossKOCount);

		wsprintf(wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120315 ),m_vMyUserInfo[i].uiAssistP );
		m_pMyAssistPoint[i]->SetText(wszTemp);
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		if( CDnBridgeTask::GetInstance().IsPvPGameMode(PvPCommon::GameMode::PvP_Occupation) )
		{
			m_pMyKOCount[i]->SetIntToText(m_vMyUserInfo[i].uiOccupationCount);
			m_pMyKObyCount[i]->SetIntToText(m_vMyUserInfo[i].uiOccupationCancelCount);
			m_pMyAssistPoint[i]->SetIntToText(m_vMyUserInfo[i].uiKOCount);
		}

		wsprintf(wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120315 ),m_vMyUserInfo[i].uiTotalScore );
		m_pMyTotalPoint[i]->SetText(wszTemp);
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		// Show only when XP arrives and is set during FinalResultOpen.
		if( m_IsFinalResult || m_vMyUserInfo[i].uiXP )
		{
			m_pMyXP[i]->SetIntToText(m_vMyUserInfo[i].uiXP);
			m_pMyPCRoom[i]->ClearText();
			wsprintf(wszTemp,L"X %d",m_vMyUserInfo[i].uiMedal);

			{
				m_pMyMedal[i]->SetText(wszTemp);

				// Currently there is only one type of medal.
				m_pMyMedalIcon[i]->SetTexture( m_hPVPMedalIconImage, 0, 0, PVP_MEDAL_ICON_XSIZE, PVP_MEDAL_ICON_YSIZE );
				m_pMyMedalIcon[i]->Show(true);
			}

			if(m_bIsLadder)
			{
				if((int)m_vMyUserInfo[i].uiXP >= 0)
					wsprintf(wszTemp,L"%d(+%d)",m_vMyUserInfo[i].uiTotalXP,m_vMyUserInfo[i].uiXP);
				else
					wsprintf(wszTemp,L"%d(%d)",m_vMyUserInfo[i].uiTotalXP,m_vMyUserInfo[i].uiXP);

				m_pMyXP[i]->SetText(wszTemp);
#ifdef PRE_MOD_PVP_LADDER_XP
				m_pMyMedal[i]->SetIntToText(m_vMyUserInfo[i].uiLadderPVPXP);
#else // PRE_MOD_PVP_LADDER_XP
				m_pMyMedal[i]->Show(false);
#endif // PRE_MOD_PVP_LADDER_XP
				m_pMyMedalIcon[i]->Show(false);
				m_pMyPCRoom[i]->Show(false);
			}
		}

		if( m_vMyUserInfo[i].iState == MINE_S )
			m_pStaticMy[i]->Show( true );
	}

	for( UINT i = 0; i< m_vEnemyTeamUserInfo.size();i++ )
	{
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
		wsprintf(wszTemp,L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), m_vEnemyTeamUserInfo[i].nLevel);
#else
		wsprintf(wszTemp,L"LV %d",m_vEnemyTeamUserInfo[i].nLevel);
#endif 
		m_pEnemyTeamLevel[i]->SetText(wszTemp);
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		if( m_vEnemyTeamUserInfo[i].cJobClassID > 0 )
			m_pEnemyJobIcon[i]->SetIconID( m_vEnemyTeamUserInfo[i].cJobClassID, true );

		//Icon
		int iIconW,iIconH;
		int iU,iV;
		iIconW = GetInterface().GeticonWidth();
		iIconH = GetInterface().GeticonHeight();
		if( GetInterface().ConvertPVPGradeToUV( m_vEnemyTeamUserInfo[i].cPVPlevel   ,iU, iV ))
		{
			m_pEnemySlotRank[i]->SetTexture(GetInterface().GetPVPIconTex(),iU, iV ,iIconW,iIconH );
			m_pEnemySlotRank[i]->Show(true);
		}

		if( m_vEnemyTeamUserInfo[i].GuildSelfView.IsSet() ) 
		{
			{
				// Guild name
				m_pEnemyGuildName[i]->SetText( m_vEnemyTeamUserInfo[i].GuildSelfView.wszGuildName );
				m_pEnemyGuildName[i]->Show( true );

				// Guild mark
				const TGuildView &GuildView = m_vEnemyTeamUserInfo[i].GuildSelfView;
				if( GetGuildTask().IsShowGuildMark( GuildView ) )
				{
					EtTextureHandle hGuildMark = GetGuildTask().GetGuildMarkTexture( GuildView );
					m_pEnemyGuildMark[i]->SetTexture( hGuildMark );
					m_pEnemyGuildMark[i]->Show( true );
				}
			}
		}


		// Name
		if( m_vEnemyTeamUserInfo[i].iState == Normal_S )
			m_pEnemyTeamName[i]->SetTextColor(EtInterface::textcolor::WHITE);
		else if ( m_vEnemyTeamUserInfo[i].iState ==  MINE_S )
			m_pEnemyTeamName[i]->SetTextColor(EtInterface::textcolor::GOLD);
		else if ( KILLED_S == m_vEnemyTeamUserInfo[i].iState )
			m_pEnemyTeamName[i]->SetTextColor( EtInterface::textcolor::RED);

#if defined( PRE_ADD_REVENGE )
		if( MINE_S != m_vEnemyTeamUserInfo[i].iState )
		{
			CDnRevengeTask * pRevengeTask = (CDnRevengeTask *)CTaskManager::GetInstance().GetTask( "RevengeTask" );
			if( NULL != pRevengeTask )
				pRevengeTask->GetRevengeUserID( m_vEnemyTeamUserInfo[i].nSessionID, m_vEnemyTeamUserInfo[i].eRevengeUser );

			if( Revenge::RevengeTarget::eRevengeTarget_Target == m_vEnemyTeamUserInfo[i].eRevengeUser )
				m_pEnemyTeamName[i]->SetTextColor( EtInterface::textcolor::PVP_REVENGE_TARGET );
			else if( Revenge::RevengeTarget::eRevengeTarget_Me == m_vEnemyTeamUserInfo[i].eRevengeUser )
				m_pEnemyTeamName[i]->SetTextColor( EtInterface::textcolor::PVP_REVENGE_ME );
			else
				m_pEnemyTeamName[i]->SetTextColor( EtInterface::textcolor::WHITE );
		}
#endif	//	#if defined( PRE_ADD_REVENGE )

		m_pEnemyTeamName[i]->SetText( m_vEnemyTeamUserInfo[i].wszUserName.c_str() );
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		// From kills downward...
		m_pEnemyKOCount[i]->SetIntToText(m_vEnemyTeamUserInfo[i].uiKOCount);
		m_pEnemyKObyCount[i]->SetIntToText(m_vEnemyTeamUserInfo[i].uiKObyCount);

		m_pEnemyBossKOCount[i]->SetIntToText(m_vEnemyTeamUserInfo[i].uiBossKOCount);

		wsprintf(wszTemp,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120315 ),m_vEnemyTeamUserInfo[i].uiAssistP );
		m_pEnemyAssistPoint[i]->SetText(wszTemp);
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		if( CDnBridgeTask::GetInstance().IsPvPGameMode(PvPCommon::GameMode::PvP_Occupation) )
		{
			m_pEnemyKOCount[i]->SetIntToText(m_vEnemyTeamUserInfo[i].uiOccupationCount);
			m_pEnemyKObyCount[i]->SetIntToText(m_vEnemyTeamUserInfo[i].uiOccupationCancelCount);
			m_pEnemyAssistPoint[i]->SetIntToText(m_vEnemyTeamUserInfo[i].uiKOCount);
		}

		wsprintf(wszTemp,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120315 ),m_vEnemyTeamUserInfo[i].uiTotalScore );
		m_pEnemyTotalPoint[i]->SetText(wszTemp);
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		// Show only when XP arrives and is set during FinalResultOpen.
		if( m_IsFinalResult || m_vEnemyTeamUserInfo[i].uiXP )
		{
			m_pEnemyXP[i]->SetIntToText(m_vEnemyTeamUserInfo[i].uiXP);
			m_pEnemyPCRoom[i]->ClearText();
			wsprintf(wszTemp,L"X %d",m_vEnemyTeamUserInfo[i].uiMedal);
			
			{
				m_pEnemyMedal[i]->SetText(wszTemp);

				// Currently there is only one type of medal.
				m_pEnemyMedalIcon[i]->SetTexture( m_hPVPMedalIconImage, 0, 0, PVP_MEDAL_ICON_XSIZE, PVP_MEDAL_ICON_YSIZE );
				m_pEnemyMedalIcon[i]->Show(true);
			}

			if(m_bIsLadder)
			{
				if((int)m_vEnemyTeamUserInfo[i].uiXP >= 0)
					wsprintf(wszTemp,L"%d(+%d)",m_vEnemyTeamUserInfo[i].uiTotalXP,m_vEnemyTeamUserInfo[i].uiXP);
				else
					wsprintf(wszTemp,L"%d(%d)",m_vEnemyTeamUserInfo[i].uiTotalXP,m_vEnemyTeamUserInfo[i].uiXP);

				m_pEnemyXP[i]->SetText(wszTemp);
#ifdef PRE_MOD_PVP_LADDER_XP
				m_pEnemyMedal[i]->SetIntToText(m_vEnemyTeamUserInfo[i].uiLadderPVPXP);
#else // PRE_MOD_PVP_LADDER_XP
				m_pEnemyMedal[i]->Show(false);
#endif // PRE_MOD_PVP_LADDER_XP
				m_pEnemyMedalIcon[i]->Show(false);
				m_pEnemyPCRoom[i]->Show(false);
			}
		}
	}
}

void CDnPVPGameResultDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() )
		UpdateUser();

#ifdef PRE_MOD_PVPOBSERVER		
	if( m_bShowResult && CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer )
	{
		m_bShowResult = false;
		m_pStaticFriendly->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7856 ) ); // "Order Dragon Team"
		m_pStaticEnemy->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7857 ) );
	}
#endif // PRE_MOD_PVPOBSERVER

	if( m_IsFinalResult ) {
		m_fTotal_ElapsedTime += fElapsedTime;

		if( m_fTotal_ElapsedTime > static_cast<float>(PvPCommon::Common::PvPFinishScoreOpenDelay) )
		{
			if( !IsShow() ) {
				m_pButtonClose->Show( true );
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
}

void CDnPVPGameResultDlg::SetPVPXP( UINT nSessionID, UINT uiAddXPScore , UINT uiTotalXpScore ,UINT uiMedalScore)
{
	// Could handle in CDnMutatorGame::EndGame, but since each mode needs separate handling, handle when receiving EXP.
	// The XP packet arrives after the result-window-open packet, and besides, medal Exp is split and calculated from experience anyway.
	int nGainMedalCount = 0;
	int nMedalExp = 0;
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP ) {
		nMedalExp = ((CDnPvPGameTask *)pGameTask)->GetMedalExp();
	}

	for( UINT i = 0; i< m_vMyUserInfo.size();i++ )
	{
		if( nSessionID == m_vMyUserInfo[i].nSessionID )
		{
			m_vMyUserInfo[i].uiXP = uiAddXPScore;
			m_vMyUserInfo[i].uiTotalXP = uiTotalXpScore;
			m_vMyUserInfo[i].uiMedal = uiMedalScore;

			if( m_vMyUserInfo[i].iState == MINE_S )
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

	for( UINT i = 0; i< m_vEnemyTeamUserInfo.size();i++ )
	{
		if( nSessionID == m_vEnemyTeamUserInfo[i].nSessionID )
		{
			m_vEnemyTeamUserInfo[i].uiXP = uiAddXPScore;
			m_vEnemyTeamUserInfo[i].uiTotalXP = uiTotalXpScore;
			m_vEnemyTeamUserInfo[i].uiMedal = uiMedalScore;
			return;
		}
	}
}

#ifdef PRE_MOD_PVP_LADDER_XP
void CDnPVPGameResultDlg::SetPVPXP( UINT nSessionID, UINT uiAddXPScore, UINT uiTotalXpScore, UINT uiMedalScore, UINT uiLadderPVPXP )
{
	for( UINT i=0; i<m_vMyUserInfo.size(); i++ )
	{
		if( nSessionID == m_vMyUserInfo[i].nSessionID )
		{
			m_vMyUserInfo[i].uiLadderPVPXP = uiLadderPVPXP;
		}
	}

	for( UINT i=0; i<m_vEnemyTeamUserInfo.size(); i++ )
	{
		if( nSessionID == m_vEnemyTeamUserInfo[i].nSessionID )
		{
			m_vEnemyTeamUserInfo[i].uiLadderPVPXP = uiLadderPVPXP;
		}
	}

	SetPVPXP( nSessionID, uiAddXPScore, uiTotalXpScore, uiMedalScore );
}
#endif // PRE_MOD_PVP_LADDER_XP

#ifdef PRE_WORLDCOMBINE_PVP
void CDnPVPGameResultDlg::ChangePVPScoreTeam( int nSessionID, int nTeam )
{
	bool bIsLeft = false;
	if( !CDnActor::s_hLocalActor || CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer )
		bIsLeft = (nTeam == PvPCommon::Team::A) ? true : false;
	else	
		bIsLeft = ( CDnActor::s_hLocalActor->GetTeam() == nTeam ) ? true : false;	// Normally, my team is on the left and the opposing team is on the right

	bool bChange = false;
	for( UINT i=0; i<m_vMyUserInfo.size(); i++ )
	{
		if( !bIsLeft && nSessionID == m_vMyUserInfo[i].nSessionID )
		{
			m_vEnemyTeamUserInfo.push_back( m_vMyUserInfo[i] );
			m_vMyUserInfo.erase( m_vMyUserInfo.begin() + i );
			bChange = true;
			break;
		}
	}

	if( !bChange )
	{
		for( UINT i=0; i<m_vEnemyTeamUserInfo.size(); i++ )
		{
			if( bIsLeft && nSessionID == m_vEnemyTeamUserInfo[i].nSessionID )
			{
				m_vMyUserInfo.push_back( m_vEnemyTeamUserInfo[i] );
				m_vEnemyTeamUserInfo.erase( m_vEnemyTeamUserInfo.begin() + i );
				bChange = true;
				break;
			}
		}
	}
}
#endif // PRE_WORLDCOMBINE_PVP
