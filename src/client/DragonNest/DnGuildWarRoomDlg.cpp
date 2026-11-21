#include "stdafx.h"
#include "DnGuildWarRoomDlg.h"
#include "DnPVPRoomEditDlg.h"
#include "DnGuildWarPopupDlg.h"
#include "DnInterface.h"
#include "DnPVPLobbyChatTabDlg.h"
#include "PvPSendPacket.h"
#include "DnBridgeTask.h"
#include "DnInterface.h"
#include "DnTableDB.h"
#include "DnBridgeTask.h"
#include "DnRevengeTask.h"
#include "TaskManager.h"
#ifdef PRE_MOD_PVPOBSERVER
#include "DnLocalPlayerActor.h"
#endif // PRE_MOD_PVPOBSERVER

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

const float fKickTime = 60;
const float fMasterKickTime = 90; // The room master cannot be kicked too easily

void CDnGuildWarRoomDlg::sUserSlot::SetState()
{
	Show( false );

	if( CDnBaseRoomDlg::InPlayer == m_eSlotState )
	{
		m_pNameText->SetText( m_wszPlayerName );
		m_pNameText->Show( true );

		if( CDnBridgeTask::GetInstance().GetSessionID() ==  m_uiSessionID )//When it is self
			m_pNameText->SetTextColor( EtInterface::textcolor::PVP_MY_SLOTCOLOR );
		else
			m_pNameText->SetTextColor( EtInterface::textcolor::WHITE );

#if defined( PRE_ADD_REVENGE )
		CDnRevengeTask * pRevengeTask = (CDnRevengeTask *)CTaskManager::GetInstance().GetTask( "RevengeTask" );
		if( NULL != pRevengeTask && CDnBridgeTask::GetInstance().GetSessionID() == m_uiSessionID )
		{
			UINT eRevengeTargetType = Revenge::RevengeTarget::eRevengeTarget_None;
			pRevengeTask->GetRevengeUserID( m_uiSessionID, eRevengeTargetType );
			if( Revenge::RevengeTarget::eRevengeTarget_Target == eRevengeTargetType )
				m_pNameText->SetTextColor( textcolor::PVP_REVENGE_TARGET );
			else if( Revenge::RevengeTarget::eRevengeTarget_Me == eRevengeTargetType )
				m_pNameText->SetTextColor( textcolor::PVP_REVENGE_ME );
			else
				m_pNameText->SetTextColor( textcolor::WHITE );
		}
#endif	// #if defined( PRE_ADD_REVENGE )

		if( CDnBaseRoomDlg::IsMaster( m_uiUserState ) )
			m_pBossIcon->Show( true );
		if( CDnBaseRoomDlg::IsReady( m_uiUserState ) )
			m_pReadyIcon->Show( true );
		if( CDnBaseRoomDlg::IsInGame( m_uiUserState ) )
		{
			m_pGaimingIcon->Show( true );
			m_pReadyIcon->Show( false );
		}
		if( CDnBaseRoomDlg::IsGuildMaster( m_uiUserState ) )
			m_pMasterIcon->Show( true );

		m_pClassIcon->SetIconID( m_nJobID, true );
		m_pClassIcon->Show( true );

		int iIconW,iIconH;
		int iU,iV;
		iIconW = GetInterface().GeticonWidth();
		iIconH = GetInterface().GeticonHeight();
		if( GetInterface().ConvertPVPGradeToUV( m_cPVPLevel, iU, iV ))
		{
			m_pRankTexture->SetTexture(GetInterface().GetPVPIconTex(), iU, iV, iIconW, iIconH);
			m_pRankTexture->Show(true);
		}
	}
	else if( CDnBaseRoomDlg::Open == m_eSlotState )
	{
		m_pOpenText->Show( true );
		m_pOpenImage->Show( true );
		m_pSelectBar->Show( false );

		m_uiUserState = 0;
		m_uiSessionID = 0;
		memset(m_wszPlayerName, NULL, sizeof(WCHAR)*NAMELENMAX);
	}
	else if( CDnBaseRoomDlg::Closed == m_eSlotState )
	{
		m_pCloseText->Show( true );
		m_pCloseImage->Show( true );
		m_pSelectBar->Show( false );

		m_uiUserState = 0;
		m_uiSessionID = 0;
		memset(m_wszPlayerName, NULL, sizeof(WCHAR)*NAMELENMAX);
	}
}

CDnGuildWarRoomDlg::CDnGuildWarRoomDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pPVPMakeRoomDlg( NULL )
, m_pGuildWarPopupDlg( NULL )
, m_cMaxPlayerNum( 0 )
, m_cCurrentTeamNum_A( 0 )
, m_cCurrentTeamNum_B( 0 )
, m_nSelectUserCount( 0 )
, m_bMoveParty( false )
#ifdef PRE_ADD_PVP_HELP_MESSAGE
,m_nRadioButtonIndex(E_PVP_RADIOBUTTON_ROOM_INFO)
#endif
{
	m_nBeepSound = -1;
}

CDnGuildWarRoomDlg::~CDnGuildWarRoomDlg()
{
	SAFE_RELEASE_SPTR( m_hMiniMapImage );	
	SAFE_DELETE( m_pPVPMakeRoomDlg );
	SAFE_DELETE( m_pGuildWarPopupDlg );

	if( m_nBeepSound != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nBeepSound );
}

void CDnGuildWarRoomDlg::Reset()
{
	m_uiMapIndex = 0;					// Map index
	m_nGameModeID = 0;					// Game mode
	m_IsBreakIntoFlag = false;        // Whether drop-in		
	SecureZeroMemory(m_wszRoomName,sizeof(m_wszRoomName));
	m_cMaxPlayerNum = 0;
	m_cMyTeam = 255;
	m_uiMyUserState = 0;
	m_uiRoomState = 0;
	m_cCurrentPlayerNum = 0;
	m_nRoomIndex = 0;
	m_fElapsedKickTime = 0.0f;
	m_nWaringNum =0;
	m_bDropItem = false;
	m_bShowHp = false;
	m_bMoveParty = false;
	m_bRevision = false;
#ifdef PRE_MOD_PVPOBSERVER
	m_bEnterObserver = false;
	m_bAllowObserver = false;
	m_bEventRoom = false;
#endif // PRE_MOD_PVPOBSERVER

	m_cMaxLevel = 100;
	m_cMinLevel = 1;

	int nMax = MAXTeamNUM * GuildWar_TeamPerPartySlot * GuildWar_PartyPerUserSlot;
	for( int itr = 0; itr < nMax; ++itr )
	{
		SUserIndex sIndex;
		sIndex.Translate( itr );

		m_pUserSlot[sIndex.nTeam][sIndex.nParty][sIndex.nUser].m_bIsNeedUpdate = true;
		m_pUserSlot[sIndex.nTeam][sIndex.nParty][sIndex.nUser].m_eSlotState = Closed;
		m_pUserSlot[sIndex.nTeam][sIndex.nParty][sIndex.nUser].SetState();
	}

	/*for( int team_itr = 0; team_itr < MAXTeamNUM; ++team_itr )
	{
		for( int party_itr = 0; party_itr < GuildWar_TeamPerPartySlot; ++party_itr )
		{
			for( int user_itr = 0; user_itr < GuildWar_PartyPerUserSlot; ++user_itr )
			{
				m_pUserSlot[team_itr][party_itr][user_itr].m_bIsNeedUpdate = true;
				m_pUserSlot[team_itr][party_itr][user_itr].m_eSlotState = Closed;
				m_pUserSlot[team_itr][party_itr][user_itr].SetState();
			}
		}
	}*/

#ifdef PRE_ADD_PVP_HELP_MESSAGE
	m_nRadioButtonIndex = E_PVP_RADIOBUTTON_ROOM_INFO;
#endif

	CDnBridgeTask::GetInstance().ResetPVPGameStatus();
}

void CDnGuildWarRoomDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildWarReadyDlg.ui" ).c_str(), bShow );
}

void CDnGuildWarRoomDlg::InitialUpdate()
{
	m_pPVPMakeRoomDlg = new CDnPVPRoomEditDlg( UI_TYPE_MODAL );
	m_pPVPMakeRoomDlg->Initialize( false );

	m_pGuildWarPopupDlg = new CDnGuildWarPopupDlg(UI_TYPE_CHILD,this);
	m_pGuildWarPopupDlg->Initialize(false);

	m_pRoomStateStatic = GetControl<CEtUIStatic>( "ID_PVP_MAP_STATE" );
	m_pUIGameMode = GetControl<CEtUIStatic>( "ID_PVP_MAP_MODE" );
	m_pUIPlayerNum = GetControl<CEtUIStatic>( "ID_PVP_MAP_REGULAR" );
	m_pMapName = GetControl<CEtUIStatic>( "ID_PVP_MAPNAME" );
	m_pRoomName = GetControl<CEtUIStatic>( "ID_PVP_ROOMNAME" );
	m_pRoomIndexNum = GetControl<CEtUIStatic>( "ID_PVP_ROOMNUMBER" );
	m_pWinCon = GetControl<CEtUIStatic>( "ID_PVP_MAP_WINCONDITION" );
	m_pInGameJoin = GetControl<CEtUIStatic>( "ID_PVP_MAP_INTRUCTION" );
	m_pNotInGameJoin = GetControl<CEtUIStatic>( "ID_PVP_MAP_NOINTRUCTION" );
	m_pPasswordRoom = GetControl<CEtUIStatic>( "ID_PVP_SECRET" );
	m_pDropItem = GetControl<CEtUIStatic>( "ID_PVP_ITEM" );
	m_pNoDropItem = GetControl<CEtUIStatic>( "ID_PVP_NOITEM" );
	m_pShowHp = GetControl<CEtUIStatic>( "ID_PVP_HP" );
	m_pHideHp = GetControl<CEtUIStatic>( "ID_PVP_NOHP" );
	m_pRevision = GetControl<CEtUIStatic>( "ID_PVP_FIT" );
	m_pNoRevision = GetControl<CEtUIStatic>( "ID_STATIC1" );

	m_pMiniMap = GetControl<CEtUITextureControl>( "ID_PVP_MAPIMG" );

	m_pBlueOption = GetControl<CEtUIStatic>( "ID_BLUE_OPTIONMODE" );
	m_pRedOption = GetControl<CEtUIStatic>( "ID_RED_OPTIONMODE" );

	m_pTeamChangeButton = GetControl<CEtUIButton>( "ID_PVP_CHANGETEAM" );
	m_pTeamChangeButton ->SetDisableTime(2.0f);
	
	m_pSatrtButton = GetControl<CEtUIButton>( "ID_PVP_START" );
	m_pReadyButton = GetControl<CEtUIButton>( "ID_PVP_READY" );
	m_pReadyButton ->SetDisableTime(2.0f);
	m_pCancelButton = GetControl<CEtUIButton>( "ID_PVP_CANCEL" );
	m_pReadyCancelButton = GetControl<CEtUIButton>( "ID_PVP_READY_END" );

	m_pSatrtButton->Show(false);
	m_pReadyButton->Show(false);
	m_pReadyCancelButton->Show(false);

	m_pUserMoveOK = GetControl<CEtUIButton>( "ID_BT_MOVEOK" );
	m_pUserMove = GetControl<CEtUIButton>( "ID_BT_MOVE" );
	
	m_pRoomEditButton = GetControl<CEtUIButton>( "ID_EDIT" );
	m_pRoomEditButton->Show(false);

	m_pKick = GetControl<CEtUIButton>( "ID_NORMAL_KICK" );
	m_pKick->Enable(false);
	m_pPermanenceKick = GetControl<CEtUIButton>( "ID_PERMAN_KICK" );
	m_pPermanenceKick->Enable(false);

	m_pMoveToObserver = GetControl<CEtUIButton>( "ID_PVP_OBSERVER" );
	m_pMoveToPlay = GetControl<CEtUIButton>( "ID_PVP_INGAME" );
	const char *szFileName = CDnTableDB::GetInstance().GetFileName( 10007 );
	if( strlen( szFileName ) > 0 )
	{
		if( m_nBeepSound== -1 )
			m_nBeepSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );			
	}


#ifdef PRE_ADD_PVP_HELP_MESSAGE
	for(int i = 0 ; i < E_PVP_RADIOBUTTON_MAX ; ++i)
		m_pRoomInfoButton[i] = GetControl<CEtUIRadioButton>( FormatA("ID_RBT_INFO%d",i).c_str() ); // Radio button

	m_pStaticModeInfo = GetControl<CEtUITextBox>("ID_TEXTBOX__MODEINFO"); // Mode description background panel
	m_pStaticModeInfo->Show(false);

	for(int i = 0 ; i < 2 ; ++i)
	{
		m_pStaticInfoBoard[i] = GetControl<CEtUIStatic>( FormatA("ID_STATIC_INFOBOARD%d",i).c_str() );
		m_pStaticInfoBoard[i]->Show(false);
	}

	for(int i = 0 ; i < 3 ; ++i)
		m_pStaticMapBoard[i] = GetControl<CEtUIStatic>( FormatA("ID_STATIC_MAPBOARD%d",i).c_str() );

	for(int i = 0 ; i < 8 ; ++i)
		m_pStaticTextMapInfo[i] = GetControl<CEtUIStatic>( FormatA("ID_TEXT_MAPINFO%d",i).c_str() ); // Player count, mode, status ~ win condition

	for(int i = 0 ; i < 5 ; ++i)
		m_pStaticMapLine[i] = GetControl<CEtUIStatic>( FormatA("ID_STATIC_MAPLINE%d",i).c_str() ); // Backdrop line

	m_pRoomInfoButton[E_PVP_RADIOBUTTON_ROOM_INFO]->SetChecked(true);
#endif


	InitUserSlot();

	Reset();
}

void CDnGuildWarRoomDlg::InitUserSlot()
{
	int total_itr = 0;

	char szSelectBar[128]
		,szOpenText[128]
		,szOpenImage[128]
		,szCloseText[128]
		,szCloseImage[128]
		,szClassIcon[128]
		,szNameText[128]
		,szBossIcon[128]
		,szMasterIcon[128]
		,szReadyIcon[128]
		,szGamingIcon[128]
		,szRankTexture[128]
		,szTeam[128];

	for( int team_itr = 0; team_itr < MAXTeamNUM; ++team_itr)
	{
		if( TeamA == team_itr )
			sprintf(szTeam,"BLUE");
		else
			sprintf(szTeam,"RED");

		for( int party_itr = 0; party_itr < GuildWar_TeamPerPartySlot; ++party_itr )
		{
			for( int user_itr = 0; user_itr < GuildWar_PartyPerUserSlot; ++user_itr )
			{
				sprintf(szSelectBar,"ID_%s%d_SELECT%d", szTeam, party_itr, user_itr);
				sprintf(szOpenText,"ID_%s%d_OPENTEXT%d", szTeam, party_itr, user_itr);
				sprintf(szOpenImage,"ID_%s%d_OPEN%d", szTeam, party_itr, user_itr);
				sprintf(szCloseText,"ID_%s%d_CLOSETEXT%d", szTeam, party_itr, user_itr);
				sprintf(szCloseImage,"ID_%s%d_CLOSE%d", szTeam, party_itr, user_itr);
				sprintf(szClassIcon,"ID_%s%d_CLASS%d", szTeam, party_itr, user_itr);
				sprintf(szNameText,"ID_%s%d_NAME%d", szTeam, party_itr, user_itr);
				sprintf(szBossIcon,"ID_%s%d_BOSS%d", szTeam, party_itr, user_itr);
				sprintf(szMasterIcon,"ID_%s%d_MASTER%d", szTeam, party_itr, user_itr);
				sprintf(szReadyIcon,"ID_%s%d_READY%d", szTeam, party_itr, user_itr);
				sprintf(szGamingIcon,"ID_%s%d_INGAME%d", szTeam, party_itr, user_itr);
				sprintf(szRankTexture,"ID_%s%d_RANK%d", szTeam, party_itr, user_itr);

				m_pUserSlot[team_itr][party_itr][user_itr].m_pSelectBar = GetControl<CEtUIStatic>( szSelectBar );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pOpenText = GetControl<CEtUIStatic>( szOpenText );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pOpenImage = GetControl<CEtUIStatic>( szOpenImage );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pCloseText = GetControl<CEtUIStatic>( szCloseText );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pCloseImage = GetControl<CEtUIStatic>( szCloseImage );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pClassIcon = GetControl<CDnJobIconStatic>( szClassIcon );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pNameText = GetControl<CEtUIStatic>( szNameText );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pBossIcon = GetControl<CEtUIStatic>( szBossIcon );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pMasterIcon = GetControl<CEtUIStatic>( szMasterIcon );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pReadyIcon = GetControl<CEtUIStatic>( szReadyIcon );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pGaimingIcon = GetControl<CEtUIStatic>( szGamingIcon );
				m_pUserSlot[team_itr][party_itr][user_itr].m_pRankTexture = GetControl<CEtUITextureControl>( szRankTexture );

				m_pUserSlot[team_itr][party_itr][user_itr].SetState();

				++total_itr;
			}
		}
	}
}

void CDnGuildWarRoomDlg::Show(bool bShow)
{
	if( bShow == m_bShow )
		return;

#ifdef PRE_ADD_PVP_HELP_MESSAGE
	for(int i = 0 ; i < E_PVP_RADIOBUTTON_MAX ; ++i)
	{
		if(m_pRoomInfoButton[i])
		{
			m_pRoomInfoButton[i]->SetChecked(false);
			if(i == m_nRadioButtonIndex)
				m_pRoomInfoButton[i]->SetChecked(true);
		}
	}
#endif // PRE_ADD_PVP_HELP_MESSAGE

	CEtUIDialog::Show( bShow );
}

void CDnGuildWarRoomDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_PVP_CANCEL" ) )
		{
			GetInterface().GetPVPLobbyChatTabDlg()->RemoveObserver(0,true); // 
			GetInterface().GetPVPLobbyChatTabDlg()->GetPVPOptionDlg()->EnalbeFatigue(true);
			SendPvPLeaveRoom();			
			return;
		}
		else if( IsCmdControl("ID_EDIT" ) )
		{
#ifdef PRE_MOD_PVP_ROOM_CREATE
			m_pPVPMakeRoomDlg->Show( true );
			m_pPVPMakeRoomDlg->SetMaxPlayerDownList( m_cMaxPlayerNum );
			m_pPVPMakeRoomDlg->SetMapAndModeDropDownList(m_uiMapIndex, m_nGameModeID, m_pMapName->GetText());
			m_pPVPMakeRoomDlg->SetRoomName( m_wszRoomName );
			m_pPVPMakeRoomDlg->SetPassword();
			m_pPVPMakeRoomDlg->SetInGameJoin(m_IsBreakIntoFlag );
			m_pPVPMakeRoomDlg->SetDropItem( m_bDropItem );
			m_pPVPMakeRoomDlg->SetGameWinObjectNum( m_pWinCon->GetText() );
			m_pPVPMakeRoomDlg->SetShowHp(m_bShowHp);
			m_pPVPMakeRoomDlg->SetRevision(m_bRevision);
			m_pPVPMakeRoomDlg->SetEditState(true);
			//m_pPVPMakeRoomDlg->SetMinMaxPlayerLevel(GetMinPlayerLevel(),GetMaxPlayerLevel());
			m_pPVPMakeRoomDlg->SetPlayerLevel(m_cMinLevel,m_cMaxLevel);
#ifdef PRE_MOD_PVPOBSERVER
			m_pPVPMakeRoomDlg->SetObserver( m_bAllowObserver );
#endif // PRE_MOD_PVPOBSERVER

#else // #ifdef PRE_MOD_PVP_ROOM_CREATE

			m_pPVPMakeRoomDlg->Show( true );
			m_pPVPMakeRoomDlg->SetMapDropDownList(m_uiMapIndex, m_pMapName->GetText() , m_cMaxPlayerNum);
			m_pPVPMakeRoomDlg->SetMaxPlayerDownList( m_cMaxPlayerNum );
			m_pPVPMakeRoomDlg->SetGameModeDownList( m_uiMapIndex , m_pUIGameMode->GetText() );
			m_pPVPMakeRoomDlg->SetRoomName( m_wszRoomName );
			m_pPVPMakeRoomDlg->SetPassword();
			m_pPVPMakeRoomDlg->SetInGameJoin(m_IsBreakIntoFlag );
			m_pPVPMakeRoomDlg->SetDropItem( m_bDropItem );
			m_pPVPMakeRoomDlg->SetGameWinObjectNum( m_pWinCon->GetText() );

			m_pPVPMakeRoomDlg->SetShowHp(m_bShowHp);
			m_pPVPMakeRoomDlg->SetRevision(m_bRevision);

			m_pPVPMakeRoomDlg->SetEditState(true);
			//m_pPVPMakeRoomDlg->SetMinMaxPlayerLevel(GetMinPlayerLevel(),GetMaxPlayerLevel());
			m_pPVPMakeRoomDlg->SetPlayerLevel(m_cMinLevel,m_cMaxLevel);

#ifdef PRE_MOD_PVPOBSERVER
			m_pPVPMakeRoomDlg->SetObserver( m_bAllowObserver );
#endif // PRE_MOD_PVPOBSERVER

#endif // #ifdef PRE_MOD_PVP_ROOM_CREATE

			return;
		}
		else if( IsCmdControl("ID_BT_MOVE" ) )
		{
			m_bMoveParty = true;
			SavePrevSlot();
		}
		else if( IsCmdControl("ID_BT_MOVEOK" ) )
		{
			m_bMoveParty = false;
			//SwapAllSlot();
		}
		if( IsCmdControl("ID_PVP_READY" ) )
		{
			SendPvPReady( IsReady(m_uiMyUserState)?0:1 );		
			return;
		}
		if( IsCmdControl("ID_PVP_READY_END" ) )
		{
			SendPvPReady( IsReady(m_uiMyUserState)?0:1 );		
			return;
		}
		if( IsCmdControl("ID_PVP_START" ) )
		{
#ifdef _FINAL_BUILD
			SendPvPStart( PvPCommon::Check::AllCheck );
#else
			SendPvPStart( CGlobalValue::GetInstance().m_bPVPGameIgnoreCondition?PvPCommon::Check::AllCheck:PvPCommon::Check::CheckTimeOver|PvPCommon::Check::CheckScore );
#endif
			return;
		}
		if( IsCmdControl("ID_PVP_CHANGETEAM" ) )
		{
			m_bMoveParty = false;
			SendPvPChangeTeam( m_cMyTeam == TeamA?PvPCommon::Team::B:PvPCommon::Team::A );
			return;
		}
		if( IsCmdControl("ID_PVP_OBSERVER") ) // Switch to observer!
		{
			SendPvPChangeTeam(PvPCommon::Team::Observer); // Observer = 1003
			return;
		}

		if( IsCmdControl("ID_PVP_INGAME") ) // Move to player!
		{
			// Check which side has more remaining slots and decide between A or B
			if(m_cCurrentTeamNum_A <= m_cCurrentTeamNum_B ) // When tied, Team A has priority. 
			{
				SendPvPChangeTeam(PvPCommon::Team::A);
			}
			else if(m_cCurrentTeamNum_A > m_cCurrentTeamNum_B )
			{
				SendPvPChangeTeam(PvPCommon::Team::B);
			}

			return;
		}
		if( IsCmdControl("ID_NORMAL_KICK") )
		{
			sUserSlot * pSlot = FindUserSlot( m_FirstSelectUser );

			if( 1 == m_nSelectUserCount && pSlot->m_uiSessionID != CDnBridgeTask::GetInstance().GetSessionID() )
				SendPvPBan( FindUserSlot( m_FirstSelectUser )->m_uiSessionID, PvPCommon::BanType::Normal );

			CEtUIListBoxEx* pUI = GetInterface().GetPVPLobbyChatTabDlg()->GetObserverListBox();
			if(GetInterface().GetPVPLobbyChatTabDlg()->GetObserverNumber() > 0)
			{
				if(GetInterface().GetPVPLobbyChatTabDlg()->IsSelectedListBox()) // Cannot kick when there are no observers. 
				{
					int SelectedSessionID = GetInterface().GetPVPLobbyChatTabDlg()->GetSessionIDFromList(pUI->GetSelectedIndex());
					SendPvPBan( SelectedSessionID, PvPCommon::BanType::Normal );
					GetInterface().GetPVPLobbyChatTabDlg()->RemoveObserver(SelectedSessionID);
				}
				return;
			}
			// Kick the observer.
			return;
		}
		else if( IsCmdControl("ID_PERMAN_KICK") )
		{
			sUserSlot * pSlot = FindUserSlot( m_FirstSelectUser );

			if( 1 == m_nSelectUserCount && pSlot->m_uiSessionID != CDnBridgeTask::GetInstance().GetSessionID() )
				SendPvPBan( FindUserSlot( m_FirstSelectUser )->m_uiSessionID, PvPCommon::BanType::Permanence );

			// Permanently kick (ban) the observer.
			CEtUIListBoxEx* pUI = GetInterface().GetPVPLobbyChatTabDlg()->GetObserverListBox();

			if(GetInterface().GetPVPLobbyChatTabDlg()->GetObserverNumber() > 0) // Cannot kick when there are no observers. 
			{
				if(GetInterface().GetPVPLobbyChatTabDlg()->IsSelectedListBox())
				{
					int SelectedSessionID = GetInterface().GetPVPLobbyChatTabDlg()->GetSessionIDFromList(pUI->GetSelectedIndex());
					SendPvPBan( SelectedSessionID, PvPCommon::BanType::Permanence );
					GetInterface().GetPVPLobbyChatTabDlg()->RemoveObserver(SelectedSessionID);
					return;
				}
			}
			return;
		}
	}
	else if( nCommand == EVENT_EDITBOX_CHANGE )
	{
		if( !IsMaster( m_uiMyUserState ) )
		{
			m_fElapsedKickTime = 0.0f;
			m_nWaringNum = 0;
		}
	}
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
#ifdef PRE_ADD_PVP_HELP_MESSAGE
		if( strstr(pControl->GetControlName() , "ID_RBT_INFO") )
		{
			int nTabID = static_cast<CEtUIRadioButton*>(pControl)->GetTabID();
			ChangeRoomInformation(nTabID);
		}
#endif
	}
}

#ifdef PRE_ADD_PVP_HELP_MESSAGE
void CDnGuildWarRoomDlg::ChangeRoomInformation(int nTabID)
{
	m_nRadioButtonIndex = nTabID;
	

	if(nTabID == E_PVP_RADIOBUTTON_ROOM_INFO)
	{
		bool bShow = true;

		//--------------------------
		// Map-related controls
		if(m_pUIGameMode)
			m_pUIGameMode->Show(bShow);
		if(m_pUIPlayerNum)
			m_pUIPlayerNum->Show(bShow);
		if(m_pMiniMap)
			m_pMiniMap->Show(bShow);
		if(m_pMapName)
			m_pMapName->Show(bShow);
		if(m_pRoomStateStatic)
			m_pRoomStateStatic->Show(bShow);
		if(m_pInGameJoin)
			m_pInGameJoin->Show(m_IsBreakIntoFlag);
		if(m_pNotInGameJoin)
			m_pNotInGameJoin->Show(!m_IsBreakIntoFlag);
		if(m_pRevision)
			m_pRevision->Show(!m_bRevision);
		if(m_pNoRevision)
			m_pNoRevision->Show(m_bRevision);
		if(m_pDropItem)
			m_pDropItem->Show(m_bDropItem);
		if(m_pNoDropItem)
			m_pNoDropItem->Show(!m_bDropItem);
		if(m_pShowHp)
			m_pShowHp->Show(m_bShowHp);
		if(m_pHideHp)
			m_pHideHp->Show(!m_bShowHp);
		if(m_pWinCon)
			m_pWinCon->Show(bShow);

		for(int i = 0 ; i < 3 ; ++i)
		{
			if(m_pStaticMapBoard[i])
				m_pStaticMapBoard[i]->Show(bShow);
		}

		for(int i = 0 ; i < 8 ; ++i)
		{
			if(m_pStaticTextMapInfo[i])
				m_pStaticTextMapInfo[i]->Show(bShow);
		}

		for(int i = 0 ; i < 5 ; ++i)
		{
			if(m_pStaticMapLine[i])
				m_pStaticMapLine[i]->Show(bShow);
		}


		//--------------------------
		// Mode-related controls
		for(int i = 0 ; i < 2 ; ++i)
		{
			// Mode description background panel
			if(m_pStaticInfoBoard[i])
				m_pStaticInfoBoard[i]->Show(!bShow);
		}

		m_pStaticModeInfo->ClearText();
		m_pStaticModeInfo->Show(!bShow);
	}
	else if(nTabID == E_PVP_RADIOBUTTON_MODE_INFO)
	{
		bool bShow = false;

		//--------------------------
		// Map-related controls
		if(m_pUIGameMode)
			m_pUIGameMode->Show(bShow);
		if(m_pUIPlayerNum)
			m_pUIPlayerNum->Show(bShow);
		if(m_pMiniMap)
			m_pMiniMap->Show(bShow);
		if(m_pMapName)
			m_pMapName->Show(bShow);
		if(m_pRoomStateStatic)
			m_pRoomStateStatic->Show(bShow);
		if(m_pInGameJoin)
			m_pInGameJoin->Show(bShow);
		if(m_pNotInGameJoin)
			m_pNotInGameJoin->Show(bShow);
		if(m_pRevision)
			m_pRevision->Show(bShow);
		if(m_pNoRevision)
			m_pNoRevision->Show(bShow);
		if(m_pDropItem)
			m_pDropItem->Show(bShow);
		if(m_pNoDropItem)
			m_pNoDropItem->Show(bShow);
		if(m_pShowHp)
			m_pShowHp->Show(bShow);
		if(m_pHideHp)
			m_pHideHp->Show(bShow);
		if(m_pWinCon)
			m_pWinCon->Show(bShow);

		for(int i = 0 ; i < 3 ; ++i)
		{
			if(m_pStaticMapBoard[i])
				m_pStaticMapBoard[i]->Show(bShow);
		}

		for(int i = 0 ; i < 8 ; ++i)
		{
			if(m_pStaticTextMapInfo[i])
				m_pStaticTextMapInfo[i]->Show(bShow);
		}

		for(int i = 0 ; i < 5 ; ++i)
		{
			if(m_pStaticMapLine[i])
				m_pStaticMapLine[i]->Show(bShow);
		}


		//--------------------------
		// Mode-related controls
		for(int i = 0 ; i < 2 ; ++i)
		{
			// Mode description background panel
			if(m_pStaticInfoBoard[i])
				m_pStaticInfoBoard[i]->Show(!bShow);
		}

		m_pStaticModeInfo->ClearText();
		if(nTabID == E_PVP_RADIOBUTTON_MODE_INFO)
		{
			// Tooltip settings
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
			if (!pSox) return;
			int nToolTipStringID = pSox->GetFieldFromLablePtr(m_nGameModeID , "_TooltipUIString")->GetInteger();

			std::vector<std::wstring> vecString;
			TokenizeW_NewLine(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nToolTipStringID ), vecString, L"\\n" );

			for(int i = 0 ; i < (int)vecString.size() ; ++i)
			{
				m_pStaticModeInfo->AddText(vecString[i].c_str());
			}
		}

		// Mode description Text Static
		m_pStaticModeInfo->Show(!bShow);
	}
}
#endif

bool CDnGuildWarRoomDlg::IsAllReady()
{
	BYTE    cReadyPlayer = 0;

	for( int team_itr = 0; team_itr < MAXTeamNUM; ++team_itr )
	{
		for( int party_itr = 0; party_itr < GuildWar_TeamPerPartySlot; ++party_itr )
		{
			for( int user_itr = 0; user_itr < GuildWar_PartyPerUserSlot; ++user_itr )
			{
				if( m_pUserSlot[team_itr][party_itr][user_itr].m_uiUserState )
				{
					if( IsReady(m_pUserSlot[team_itr][party_itr][user_itr].m_uiUserState) )
						++cReadyPlayer;
				}
			}
		}
	}

	if( cReadyPlayer == (GetRoomPlayerNumber()-1/*Exclude room master*/) && (cReadyPlayer!=0) && cReadyPlayer == m_cMaxPlayerNum-1) 
		return true;

	return false;
}

int CDnGuildWarRoomDlg::GetRoomPlayerNumber()
{
	int userCount = 0;

	for( int team_itr = 0; team_itr < MAXTeamNUM; ++team_itr )
	{
		for( int party_itr = 0; party_itr < GuildWar_TeamPerPartySlot; ++party_itr )
		{
			for( int user_itr = 0; user_itr < GuildWar_PartyPerUserSlot; ++user_itr )
			{
				if( m_pUserSlot[team_itr][party_itr][user_itr].m_eSlotState == SlotState::InPlayer )
					++userCount;
			}
		}
	}

	return userCount;
}

void CDnGuildWarRoomDlg::Process(float fElapsedTime)
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	SetButtonState();

	if( CDnBridgeTask::GetInstance().IsGuildWarSystem() )
		return;

#ifdef _WORK // Do not set kick in dev environment.
	return;
#endif

	if( IsMaster( m_uiMyUserState ) ) //When room master
	{
		if( IsAllReady() || GetRoomPlayerNumber() >= 1) 
			m_fElapsedKickTime += fElapsedTime;
		else
			m_fElapsedKickTime = 0.0f;		


		if( m_fElapsedKickTime > fMasterKickTime-30 && (m_nWaringNum == 0) ) // 30 sec remaining
		{
			WCHAR wsWarnningMSG[100];
			SecureZeroMemory(wsWarnningMSG,sizeof(wsWarnningMSG));

			if(!IsAllReady())
				wsprintf(wsWarnningMSG,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120060),30); // In this case, there is no key input.
			else
				wsprintf(wsWarnningMSG,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120062),30); // Everyone is ready, but the game is not starting.

			GetInterface().AddChatMessage(CHATTYPE_SYSTEM ,L""  , wsWarnningMSG);
			m_nWaringNum = 1;

		}

		if( m_fElapsedKickTime > fMasterKickTime - 10 && (m_nWaringNum == 1) ) // 10 seconds before
		{
			WCHAR wsWarnningMSG[100];
			SecureZeroMemory(wsWarnningMSG,sizeof(wsWarnningMSG));

			if(!IsAllReady())
				wsprintf(wsWarnningMSG,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120060),10); // No key input from room master.
			else
				wsprintf(wsWarnningMSG,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120062),10);     // Case where everyone is ready, but the game does not start.

			GetInterface().AddChatMessage(CHATTYPE_SYSTEM ,L""  , wsWarnningMSG);
			m_nWaringNum = 2;
		}

		if( m_fElapsedKickTime > fMasterKickTime && (m_nWaringNum == 2) )
		{
			GetInterface().GetPVPLobbyChatTabDlg()->RemoveObserver(0,true); 

			m_pPVPMakeRoomDlg->SetEditState(false);
			m_pPVPMakeRoomDlg->Show(false);

			SendPvPLeaveRoom(); // Add window closing logic here
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120064) , MB_OK);//Kicked due to no input
			m_nWaringNum = 0;
			m_fElapsedKickTime =0.0f;
		}

	}else
	{
		if(!IsReady( m_uiMyUserState ) && !(m_cMyTeam > 1) && !(m_cMyTeam < 0)) // m_cMyTeam  0 =A , 1 = B 
			m_fElapsedKickTime += fElapsedTime;
		else
			m_fElapsedKickTime = 0.0f;		

		if( m_fElapsedKickTime > fKickTime/2 && (m_nWaringNum == 0) )
		{
			WCHAR wsWarnningMSG[100];
			SecureZeroMemory(wsWarnningMSG,sizeof(wsWarnningMSG));
			wsprintf(wsWarnningMSG,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120060),30);
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM ,L""  , wsWarnningMSG);
			m_nWaringNum = 1;

		}

		if( m_fElapsedKickTime > fKickTime - (fKickTime/6) && (m_nWaringNum == 1) )
		{
			WCHAR wsWarnningMSG[100];
			SecureZeroMemory(wsWarnningMSG,sizeof(wsWarnningMSG));
			wsprintf(wsWarnningMSG,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120060),10);
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM ,L""  , wsWarnningMSG);
			m_nWaringNum = 2;
		}

		if( m_fElapsedKickTime > fKickTime && (m_nWaringNum == 2) )
		{
			GetInterface().GetPVPLobbyChatTabDlg()->RemoveObserver(0,true); // 
			SendPvPLeaveRoom();
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120064) , MB_OK);//Kicked due to no input
			m_nWaringNum = 0;
			m_fElapsedKickTime =0.0f;
		}

	}
}

bool CDnGuildWarRoomDlg::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( !IsShow() )
		return false;

	switch( uMsg )
	{
	case WM_KEYDOWN:
	case WM_MOUSEMOVE :
		{

			if(IsMaster( m_uiMyUserState ))
			{
				if(!IsAllReady())
				{
					m_fElapsedKickTime = 0.0f;
					m_nWaringNum = 0;
				}
			}
			else if( !IsMaster( m_uiMyUserState ) )
			{
				m_fElapsedKickTime = 0.0f;
				m_nWaringNum = 0;
				//return true;
			}

		}
		break;

	case WM_LBUTTONDOWN:
		{
			if( m_pPVPMakeRoomDlg->IsShow() )
				break;

			POINT MousePoint;
			float fMouseX, fMouseY;

			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );
			PointToFloat( MousePoint, fMouseX, fMouseY );

			if( m_bMoveParty )
				UpdateSelectBarMoveMode( fMouseX, fMouseY );
			else
				UpdateSelectBar( fMouseX, fMouseY );

			if( IsMouseInDlg() ) {
				if( m_pGuildWarPopupDlg->IsShow() ) {
					SUICoord uiCoord;
					m_pGuildWarPopupDlg->GetDlgCoord( uiCoord );
					fMouseX = MousePoint.x / GetScreenWidth();
					fMouseY = MousePoint.y / GetScreenHeight();
					if( !uiCoord.IsInside( fMouseX, fMouseY ) ) {
						ShowChildDialog( m_pGuildWarPopupDlg, false );
						return true;
					}
				}
			}
		}
		break;

	case WM_RBUTTONDOWN:
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fMouseX, fMouseY;
			PointToFloat( MousePoint, fMouseX, fMouseY );

			bool bIsInsideItem = false;
			SUserIndex sIndex;
			int nMax = MAXTeamNUM * GuildWar_TeamPerPartySlot * GuildWar_PartyPerUserSlot;

			for( int itr = 0; itr < nMax; ++itr )
			{
				sIndex.Translate( itr );

				if( m_pUserSlot[sIndex.nTeam][sIndex.nParty][sIndex.nUser].m_eSlotState != InPlayer )
					continue;

				SUICoord uiCoord;
				m_pUserSlot[sIndex.nTeam][sIndex.nParty][sIndex.nUser].m_pSelectBar->GetUICoord( uiCoord );

				if( uiCoord.IsInside( fMouseX, fMouseY ) ) 
				{
					m_pGuildWarPopupDlg->SetTargetName(m_pUserSlot[sIndex.nTeam][sIndex.nParty][sIndex.nUser].m_wszPlayerName);
					bIsInsideItem = true;
					break;
				}
			}

			if( bIsInsideItem ) {

				fMouseX = MousePoint.x / GetScreenWidth();
				fMouseY = MousePoint.y / GetScreenHeight();

				SUICoord sDlgCoord;
				static float fyBGap(16.f /DEFAULT_UI_SCREEN_WIDTH);   // Looks better positioned slightly to the right..

				ShowChildDialog( m_pGuildWarPopupDlg, false );
				m_pGuildWarPopupDlg->SetPosition( fMouseX+fyBGap, fMouseY );

				m_pGuildWarPopupDlg->GetDlgCoord( sDlgCoord );
				if( (sDlgCoord.Right()) > GetScreenWidthRatio() )
					sDlgCoord.fX -= (sDlgCoord.Right() - GetScreenWidthRatio()); //  Push back if overflowing off screen.
				m_pGuildWarPopupDlg->SetDlgCoord( sDlgCoord );
				m_pGuildWarPopupDlg->SetMasterDelegate( (sIndex.nTeam == m_cMyTeam && IsGuildMaster( m_uiMyUserState ) && !m_bMoveParty), m_pUserSlot[sIndex.nTeam][sIndex.nParty][sIndex.nUser].m_uiSessionID );

				ShowChildDialog( m_pGuildWarPopupDlg, true );
				GetInterface().GetPVPLobbyChatTabDlg()->GetPVPObserverDlg()->ShowPVPPopUpDlg(false);
			}
		}
		break;
	}
	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnGuildWarRoomDlg::UpdateSelectBarMoveMode( float fX, float fY )
{
	int nSelectNum = 0;
	int nFirst = -1, nSecond = -1;

	m_nSelectUserCount = 0;

	if( MAXTeamNUM <= m_cMyTeam)
		return;

	for( int party_itr = 0; party_itr < GuildWar_TeamPerPartySlot; ++party_itr ) 
	{
		for( int user_itr = 0; user_itr < GuildWar_PartyPerUserSlot; ++user_itr ) 
		{
			SUICoord uiBaseCoord;
			m_pUserSlot[m_cMyTeam][party_itr][user_itr].m_pSelectBar->GetUICoord(uiBaseCoord);

			if( uiBaseCoord.IsInside(fX, fY) )
			{
				if( m_pUserSlot[m_cMyTeam][party_itr][user_itr].m_pSelectBar->IsShow() )
				{
					m_pUserSlot[m_cMyTeam][party_itr][user_itr].m_pSelectBar->Show( false );
				}
				else
				{
					if( (SlotState::InPlayer == m_pUserSlot[m_cMyTeam][party_itr][user_itr].m_eSlotState ||
						SlotState::Open == m_pUserSlot[m_cMyTeam][party_itr][user_itr].m_eSlotState ) 
						&& m_nSelectUserCount < 2 )
					{
						m_pUserSlot[m_cMyTeam][party_itr][user_itr].m_pSelectBar->Show( true );
						++m_nSelectUserCount;

						if( -1 == nFirst )
							nFirst = nSelectNum;
						else
							nSecond = nSelectNum;
					}
				}
			}
			else
			{
				if( m_pUserSlot[m_cMyTeam][party_itr][user_itr].m_pSelectBar->IsShow() )
				{
					if( -1 == nFirst )
						nFirst = nSelectNum;
					else
						nSecond = nSelectNum;

					++m_nSelectUserCount;
				}
			}
			++nSelectNum;
		}
	}

	if( 2 == m_nSelectUserCount && -1 != nFirst && -1 != nSecond)
		SwapSlot( SUserIndex( nFirst ), SUserIndex( nSecond ) );
}

void CDnGuildWarRoomDlg::UpdateSelectBar( float fX, float fY )
{
	//if( !m_pKick || !m_pPermanenceKick || !m_pChangeMasterButton) return;
	if( !m_pKick || !m_pPermanenceKick ) return;
	if( IsMouseInButton( fX, fY ) ) return;

	int nSelectNum = 0, nCountNum = 0;

	for( int team_itr = 0; team_itr < MAXTeamNUM; ++team_itr ) 
	{
		for( int party_itr = 0; party_itr < GuildWar_TeamPerPartySlot; ++party_itr ) 
		{
			for( int user_itr = 0; user_itr < GuildWar_PartyPerUserSlot; ++user_itr ) 
			{
				SUICoord uiBaseCoord;
				m_pUserSlot[team_itr][party_itr][user_itr].m_pSelectBar->GetUICoord(uiBaseCoord);

				if( uiBaseCoord.IsInside(fX, fY) )
				{
					if( m_pUserSlot[team_itr][party_itr][user_itr].m_pSelectBar->IsShow() )
					{
						m_pUserSlot[team_itr][party_itr][user_itr].m_pSelectBar->Show( false );
						--m_nSelectUserCount;
					}
					else
					{
						if( SlotState::InPlayer == m_pUserSlot[team_itr][party_itr][user_itr].m_eSlotState )
						{
							m_pUserSlot[team_itr][party_itr][user_itr].m_pSelectBar->Show( true );
							++m_nSelectUserCount;
							nSelectNum = nCountNum;
						}
					}
				}
				else
				{
					if( m_pUserSlot[team_itr][party_itr][user_itr].m_pSelectBar->IsShow() )
						--m_nSelectUserCount;

					m_pUserSlot[team_itr][party_itr][user_itr].m_pSelectBar->Show( false );
				}
				++nCountNum;
			}
		}
	}

	bool bKick = false;

	if( 1 == m_nSelectUserCount )
	{
		m_FirstSelectUser.Translate( nSelectNum );

		if( !m_pUserSlot[m_FirstSelectUser.nTeam][m_FirstSelectUser.nParty][m_FirstSelectUser.nUser].m_pBossIcon->IsShow() )
			bKick = true;
	}

#ifdef PRE_MOD_PVPOBSERVER
	if( bKick && ( CGlobalInfo::GetInstance().m_cLocalAccountLevel >= eAccountLevel::AccountLevel_New || (m_uiMyUserState & PvPCommon::UserState::Captain) ) )
#else
	if( bKick && m_uiMyUserState & PvPCommon::UserState::Captain )
#endif // PRE_MOD_PVPOBSERVER
	{
		m_pKick->Enable(true);
		m_pPermanenceKick->Enable(true);
		//m_pChangeMasterButton->Enable(true);
	}
	else
	{
#ifdef PRE_MOD_PVPOBSERVER
		if( CGlobalInfo::GetInstance().m_cLocalAccountLevel >= eAccountLevel::AccountLevel_New )
		{
			m_pKick->Enable(true);
			m_pPermanenceKick->Enable(true);
		}
		else
		{
			m_pKick->Enable(false);
			m_pPermanenceKick->Enable(false);
		}
#else
		m_pKick->Enable(false);
		m_pPermanenceKick->Enable(false);
#endif
		///m_pChangeMasterButton->Enable(false);
	}
	CEtUIListBoxEx* pUI = GetInterface().GetPVPLobbyChatTabDlg()->GetObserverListBox();
	GetInterface().GetPVPLobbyChatTabDlg()->SetSelectedListBox(false);
	pUI->SetRenderSelectBar(false);
}

bool CDnGuildWarRoomDlg::IsMouseInButton( float fX, float fY )
{
	SUICoord uiCoord;
	if( m_pKick ) {
		m_pKick->GetUICoord(uiCoord);
		if( uiCoord.IsInside( fX, fY ) ) return true;
	}

	if( m_pPermanenceKick ) {
		m_pPermanenceKick->GetUICoord(uiCoord);
		if( uiCoord.IsInside( fX, fY ) ) return true;
	}

	/*if( m_pChangeMasterButton ) {
		m_pChangeMasterButton->GetUICoord(uiCoord);
		if( uiCoord.IsInside( fX, fY) ) return true;
	}*/

	if( m_pReadyCancelButton ) {
		m_pReadyCancelButton->GetUICoord(uiCoord);
		if( uiCoord.IsInside( fX, fY ) ) return true;
	}

	if( m_pReadyCancelButton ) {
		m_pReadyCancelButton->GetUICoord(uiCoord);
		if( uiCoord.IsInside( fX, fY ) ) return true;
	}

	if( m_pCancelButton ) {
		m_pCancelButton->GetUICoord(uiCoord);
		if( uiCoord.IsInside( fX, fY ) ) return true;
	}

	if( m_pRoomEditButton ) {
		m_pRoomEditButton->GetUICoord(uiCoord);
		if( uiCoord.IsInside( fX, fY ) ) return true;
	}

	return false;
}

void CDnGuildWarRoomDlg::SavePrevSlot()
{
	m_vPrevSlot.clear();

	for( int itr = 0; itr < m_cMaxPlayerNum / 2; ++itr )
	{
		SUserIndex sIndex;
		sIndex.Translate( itr );

		m_vPrevSlot.push_back( pair<SUserIndex, sUserSlot>( SUserIndex(itr), m_pUserSlot[m_cMyTeam][sIndex.nParty][sIndex.nUser] ) );
	}
}

void CDnGuildWarRoomDlg::SwapSlot( SUserIndex & sFirstIndex, SUserIndex & sSecondIndex )
{
	SavePrevSlot();

	sUserSlot sSlot;

	sSlot = m_pUserSlot[m_cMyTeam][sFirstIndex.nParty][sFirstIndex.nUser];
	m_pUserSlot[m_cMyTeam][sFirstIndex.nParty][sFirstIndex.nUser] = m_pUserSlot[m_cMyTeam][sSecondIndex.nParty][sSecondIndex.nUser];
	m_pUserSlot[m_cMyTeam][sSecondIndex.nParty][sSecondIndex.nUser] = sSlot;

	m_pUserSlot[m_cMyTeam][sFirstIndex.nParty][sFirstIndex.nUser].SetState();
	m_pUserSlot[m_cMyTeam][sSecondIndex.nParty][sSecondIndex.nUser].SetState();

	SwapAllSlot();

	m_nSelectUserCount = 0;
}

void CDnGuildWarRoomDlg::SwapAllSlot()
{
	std::vector< std::pair<int, UINT> > vChangeUser;

	for( int itr = 0; itr < (int)m_vPrevSlot.size(); ++itr )
	{
		SUserIndex sIndex = m_vPrevSlot[itr].first;

		if( m_pUserSlot[m_cMyTeam][sIndex.nParty][sIndex.nUser].m_uiSessionID != m_vPrevSlot[itr].second.m_uiSessionID )
			vChangeUser.push_back( pair<int, UINT>(sIndex.nIndex, m_pUserSlot[m_cMyTeam][sIndex.nParty][sIndex.nUser].m_uiSessionID ) );
	}

	if( !vChangeUser.empty() )
	{
		int nPosition = m_cMyTeam == 1 ? m_cMaxPlayerNum / 2 : 0;

		BYTE cCount = (BYTE)vChangeUser.size();
		BYTE * pIndex = new BYTE[cCount];
		UINT * pSessionID = new UINT[cCount];
		SecureZeroMemory( pIndex, sizeof(BYTE)*cCount );
		SecureZeroMemory( pSessionID, sizeof(UINT)*cCount );

		for( BYTE itr = 0; itr < cCount; ++itr )
		{
			pIndex[itr] = vChangeUser[itr].first + nPosition;
			pSessionID[itr] = vChangeUser[itr].second;
		}
		SendSwapMemberIndex( cCount, pIndex, pSessionID );

		delete[] pIndex;
		delete[] pSessionID;
	}
}

void CDnGuildWarRoomDlg::RevertSlot()
{
	if( m_vPrevSlot.empty() )
		return;

	for( int itr = 0; itr < (int)m_vPrevSlot.size(); ++itr )
	{
		SUserIndex sIndex = m_vPrevSlot[itr].first;

		if( m_pUserSlot[m_cMyTeam][sIndex.nParty][sIndex.nUser].m_uiSessionID != m_vPrevSlot[itr].second.m_uiSessionID )
		{
			m_pUserSlot[m_cMyTeam][sIndex.nParty][sIndex.nUser] = m_vPrevSlot[itr].second;
			m_pUserSlot[m_cMyTeam][sIndex.nParty][sIndex.nUser].SetState();
		}
	}
}

void CDnGuildWarRoomDlg::RefreshSlot( SCPvPMemberIndex * pPacket )
{
	BYTE cTeam = 0;
	if( PvPCommon::Team::B == pPacket->nTeam )
		cTeam = 1;

	if( m_cMyTeam == cTeam && IsGuildMaster(m_uiMyUserState) )
		return;

	sUserSlot pPrevSlot[ GuildWar_TeamUserSlot ];

	for( int itr = 0; itr < m_cMaxPlayerNum/2; ++itr )
	{
		SUserIndex sIndex;
		sIndex.Translate( itr );

		pPrevSlot[itr] = m_pUserSlot[cTeam][sIndex.nParty][sIndex.nUser];
	}

	for( int itr = 0; itr < pPacket->cCount; ++itr )
	{
		SUserIndex sIndex;
		sIndex.Translate( pPacket->Index[itr].cIndex );

		if( 0 == pPacket->Index[itr].nSessionID )
		{
			m_pUserSlot[cTeam][sIndex.nParty][sIndex.nUser].m_uiSessionID = 0;
			m_pUserSlot[cTeam][sIndex.nParty][sIndex.nUser].m_eSlotState = Open;
			m_pUserSlot[cTeam][sIndex.nParty][sIndex.nUser].SetState();
			continue;
		}

		for( int jtr = 0; jtr < m_cMaxPlayerNum/2; ++jtr )
		{
			if( (pPacket->Index[itr].nSessionID == pPrevSlot[jtr].m_uiSessionID) )
			{
				m_pUserSlot[cTeam][sIndex.nParty][sIndex.nUser] = pPrevSlot[jtr];
				m_pUserSlot[cTeam][sIndex.nParty][sIndex.nUser].SetState();
				break;
			}
		}
	}
}


void CDnGuildWarRoomDlg::RoomSetting( PvPCommon::RoomInfo* pInfo ,bool IsModified)
{
	m_nSelectUserCount = 0;
	m_uiMapIndex = pInfo->uiMapIndex;
	m_nGameModeID = pInfo->uiGameModeTableID;
	m_IsBreakIntoFlag = pInfo->unRoomOptionBit & PvPCommon::RoomOption::BreakInto ? true:false;	
	m_bDropItem = pInfo->unRoomOptionBit & PvPCommon::RoomOption::DropItem ? true:false;	
	m_bShowHp = pInfo->unRoomOptionBit & PvPCommon::RoomOption::ShowHP ? true:false;	
	m_bRevision = pInfo->unRoomOptionBit & PvPCommon::RoomOption::NoRegulation ? true:false;
	m_uiRoomState = pInfo->cRoomState;

#ifdef PRE_MOD_PVPOBSERVER
	m_bAllowObserver = pInfo->unRoomOptionBit & PvPCommon::RoomOption::AllowObserver ? true : false;
	m_bEventRoom = pInfo->bExtendObserver;
	m_pMoveToPlay->Enable( !m_bEventRoom );
#endif // PRE_MOD_PVPOBSERVER

	m_cMaxLevel = pInfo->cMaxLevel;
	m_cMinLevel = pInfo->cMinLevel;

	m_bIsGuildWarSystem = pInfo->bIsGuildWarSystem;
	CDnBridgeTask::GetInstance().SetGuildWarSystem( m_bIsGuildWarSystem );

	m_pPasswordRoom->Show(false);
	m_pInGameJoin->Show(false);
	m_pNotInGameJoin->Show(false);
	m_pDropItem->Show(false);
	m_pNoDropItem->Show(false);
	m_pShowHp->Show(false);
	m_pHideHp->Show(false);
	m_pBlueOption->Show(false);
	m_pRedOption->Show(false);
	m_pRevision->Show(false);
	m_pNoRevision->Show(false);

	if( m_IsBreakIntoFlag )
		m_pInGameJoin->Show(true);
	else
		m_pNotInGameJoin->Show(true);

	if( m_bDropItem )
		m_pDropItem->Show(true);
	else
		m_pNoDropItem->Show(true); 

	if(m_bShowHp)
		m_pShowHp->Show(true);
	else
		m_pHideHp->Show(true);

	if(!m_bRevision)
		m_pRevision->Show(true);
	else
		m_pNoRevision->Show(true);

	SecureZeroMemory(m_wszRoomName,sizeof(m_wszRoomName));
	memcpy(m_wszRoomName,pInfo->wszBuffer,pInfo->cRoomNameLen * sizeof(WCHAR));
	m_pRoomName->SetText(m_wszRoomName);

	m_cMaxPlayerNum = pInfo->cMaxUserCount;
	int iTeamplayNum = 0;

	iTeamplayNum = int(m_cMaxPlayerNum/2);
	iTeamplayNum = min(iTeamplayNum, GuildWar_TeamUserSlot);

	if( !IsModified )
	{
		for( int team_itr = 0; team_itr < MAXTeamNUM; ++team_itr )
		{
			int iPartyNum = 0;
			int iPartyPerUserNum = 0;

			for( int user_itr = 0; user_itr < iTeamplayNum; ++user_itr )
			{
				m_pUserSlot[team_itr][iPartyNum][iPartyPerUserNum].m_bIsNeedUpdate = true;
				m_pUserSlot[team_itr][iPartyNum][iPartyPerUserNum].m_eSlotState = Open;
				m_pUserSlot[team_itr][iPartyNum][iPartyPerUserNum].SetState();

				++iPartyPerUserNum;

				if( 0 != iPartyPerUserNum && 0 == iPartyPerUserNum % GuildWar_PartyPerUserSlot )
				{
					++iPartyNum;
					iPartyPerUserNum = 0;
				}
			}
		}

		m_nRoomIndex = pInfo->uiIndex;
	}
	
	WCHAR wszIndex[10];
	wsprintf(wszIndex,L"%d. ", m_nRoomIndex);
	m_pRoomIndexNum->SetText( wszIndex );

	//Part to retrieve the game mode
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if ( !pSox ) 
	{
		ErrorLog("CDnGuildWarGameRoomDlg::RoomSetting:: gamemode table Not found!! ");
		return;
	}

	WCHAR wszWinCon[10];
	SecureZeroMemory(wszWinCon,sizeof(wszWinCon));

	wsprintf(wszWinCon,L"%d%s",pInfo->uiWinCondition,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( m_nGameModeID , "WinCondition_UIString" )->GetInteger()));

	m_pWinCon->SetText(wszWinCon);

	int nGameModeIndex = 0;
	nGameModeIndex = pSox->GetFieldFromLablePtr( m_nGameModeID , "GamemodeID" )->GetInteger();

	int nGameModeUIString = 0;
	nGameModeUIString = pSox->GetFieldFromLablePtr( m_nGameModeID , "GameModeUIString" )->GetInteger();

	const wchar_t * wszModeName = NULL; 
	wszModeName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nGameModeUIString );

	if( wszModeName )
		m_pUIGameMode->SetText(wszModeName);

	//Section to get the map name
	pSox = GetDNTable( CDnTableDB::TPVPMAP );
	if ( !pSox ) 
	{
		ErrorLog("CDnGuildWarGameRoomDlg::RoomSetting:: PVP Map table Not found!! ");
		return;
	}

	int nMapNameUIString = 0;
	nMapNameUIString = pSox->GetFieldFromLablePtr( m_uiMapIndex , "MapNameUIstring" )->GetInteger();

	const wchar_t * wszMapName = NULL; 
	wszMapName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMapNameUIString );

	if( wszMapName )
		m_pMapName->SetText(wszMapName);

	//Minimap rendering part	

	SAFE_RELEASE_SPTR( m_hMiniMapImage );

	std::string szMiniTextureName = pSox->GetFieldFromLablePtr( m_uiMapIndex , "MapImage" )->GetString();

	if( szMiniTextureName.c_str())
		m_hMiniMapImage = LoadResource( CEtResourceMng::GetInstance().GetFullName(szMiniTextureName.c_str()).c_str(), RT_TEXTURE );

	if( m_hMiniMapImage )
		m_pMiniMap->SetTexture( m_hMiniMapImage, 0, 0, m_hMiniMapImage->Width() , m_hMiniMapImage->Height() );

	UpdateSlot( false );

	std::wstring wszFullRoomName = wszIndex;
	wszFullRoomName.append( m_wszRoomName , wcslen(m_wszRoomName));

	SetRoomState(pInfo->cRoomState);
	CDnBridgeTask::GetInstance().SetPVPGameStatus(wszFullRoomName, pInfo->uiWinCondition, m_nGameModeID, pInfo->uiPlayTimeSec, m_cMaxPlayerNum
		, m_bShowHp, m_bRevision);

#ifdef PRE_ADD_PVP_HELP_MESSAGE
	if(IsModified)
	{
		ChangeRoomInformation(E_PVP_RADIOBUTTON_ROOM_INFO);
		if(m_pRoomInfoButton[m_nRadioButtonIndex])
			m_pRoomInfoButton[m_nRadioButtonIndex]->SetChecked(true);
	}
#endif

	GetInterface().GetPVPLobbyChatTabDlg()->GetPVPOptionDlg()->EnalbeFatigue(false);
}

void CDnGuildWarRoomDlg::InsertPlayer(int team, UINT uiUserState, UINT uiSessionID, int iJobID, BYTE  cLevel, BYTE  cPVPLevel, const WCHAR * szPlayerName, char cPosition )
{
	int nPlayerTeamIndex = -1;

	switch(team) // 
	{
	case (PvPCommon::Team::A):
		nPlayerTeamIndex = 0;

		break;
	case (PvPCommon::Team::B):
		nPlayerTeamIndex = 1;
		cPosition -= m_cMaxPlayerNum / 2;
		break;
	case (PvPCommon::Team::Observer):
		break;
	}

	if( nPlayerTeamIndex >= MAXTeamNUM || -1 == nPlayerTeamIndex )
	{
		ErrorLog("CDnGuildWarGameRoomDlg::InsertPlayer:: Not insert player because invalid teamIndex.");
		return;
	}

	if( CDnBridgeTask::GetInstance().GetSessionID() ==  uiSessionID )//When it is self 
		m_cMyTeam = nPlayerTeamIndex;

	int nPartyIndex = cPosition / GuildWar_PartyPerUserSlot;// - (nPlayerTeamIndex == 1 ? 4 : 0);
	int nUserIndex = cPosition % GuildWar_PartyPerUserSlot;

	if( m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_eSlotState == Open )
	{
		m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_nJobID = iJobID;
		m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_cLevel = cLevel;
		m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_uiUserState = uiUserState;
		m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_cPVPLevel = cPVPLevel;
		m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_uiSessionID = uiSessionID;
		m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_eSlotState= InPlayer;
		m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_bIsNeedUpdate = true;;
		m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_cPosition = cPosition;
		m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_cTeam = nPlayerTeamIndex;
		swprintf_s(m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_wszPlayerName, _countof(m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_wszPlayerName), szPlayerName);
		m_cCurrentPlayerNum++;
		SetUserState( uiSessionID, uiUserState );

		if(nPlayerTeamIndex == 0)
		{
			m_cCurrentTeamNum_A++;
		}
		else if(nPlayerTeamIndex == 1)
		{
			m_cCurrentTeamNum_B++;
		}
	}
	else if( m_pUserSlot[nPlayerTeamIndex][nPartyIndex][nUserIndex].m_eSlotState == Closed )
	{
		ErrorLog("CDnGuildWarRoomDlg::InsertPlayer:: Closed");
	}
	else
	{
		ErrorLog("CDnGuildWarRoomDlg::InsertPlayer:: InPlayer");	
	}
}


#ifdef PRE_MOD_PVPOBSERVER
void CDnGuildWarRoomDlg::InsertObserver( UINT uiUserState , UINT uiSessionID )
{
	if( CDnBridgeTask::GetInstance().GetSessionID() ==  uiSessionID )
		SetUserState( uiSessionID , uiUserState );
}
#endif // PRE_MOD_PVPOBSERVER


void CDnGuildWarRoomDlg::SetRoomState( UINT uiRoomState )
{
	m_uiRoomState = uiRoomState;

	int nRoomStateUIString = 120028;	

	if(IsSyncingRoom())
		nRoomStateUIString = 120029;

	if(IsStartingRoom())
		nRoomStateUIString = 120029;

	if(IsPlayingRoom())
		nRoomStateUIString = 120030;

	const wchar_t * wszRoomState = NULL; 
	wszRoomState = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nRoomStateUIString );

	m_pPasswordRoom->Show(IsPasssWordRoom());

	if( wszRoomState )
		m_pRoomStateStatic->SetText(wszRoomState);
}

void CDnGuildWarRoomDlg::RemovePlayer( UINT uiSessionID, PvPCommon::LeaveType::eLeaveType eType )
{
	WCHAR wszMessage[100] = {0,};

	for( int team_itr = 0; team_itr < MAXTeamNUM; ++team_itr )
	{
		for( int party_itr = 0; party_itr < GuildWar_TeamPerPartySlot; ++party_itr )
		{
			for( int user_itr = 0; user_itr < GuildWar_PartyPerUserSlot; ++user_itr )
			{
				if( m_pUserSlot[team_itr][party_itr][user_itr].m_uiSessionID == uiSessionID )
				{
					int nUIString = 0;
					if( eType == PvPCommon::LeaveType::Ban ) nUIString = 121061;
					else if( eType == PvPCommon::LeaveType::PermanenceBan ) nUIString = 121062;
					if( nUIString ) {
						wsprintf( wszMessage, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nUIString ), m_pUserSlot[team_itr][party_itr][user_itr].m_wszPlayerName );
						GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszMessage, false );
					}

					if( m_pUserSlot[team_itr][party_itr][user_itr].m_pSelectBar->IsShow() )
						--m_nSelectUserCount;

					m_pUserSlot[team_itr][party_itr][user_itr].m_eSlotState = SlotState::Open;
					m_pUserSlot[team_itr][party_itr][user_itr].SetState();

					--m_cCurrentPlayerNum;

					if( team_itr == 0 )
					{
						m_cCurrentTeamNum_A--;
					}
					else if( team_itr == 1 )
					{
						m_cCurrentTeamNum_B--;
					}
					return;
				}	
			}
		}
	}
}

void CDnGuildWarRoomDlg::UpdateSlot( bool ForceAll )
{
	WCHAR wszPlayerNum[64] = {0,};
	SecureZeroMemory( wszPlayerNum, sizeof(wszPlayerNum) );

	m_nSelectUserCount = 0;
#ifdef PRE_MOD_PVPOBSERVER
	bool bEnable = ( CGlobalInfo::GetInstance().m_cLocalAccountLevel >= eAccountLevel::AccountLevel_New );
	m_pKick->Enable( bEnable );
	m_pPermanenceKick->Enable( bEnable );
#else
	m_pKick->Enable( false );
	m_pPermanenceKick->Enable( false );
#endif // PRE_MOD_PVPOBSERVER	

	for( int team_itr = 0; team_itr < MAXTeamNUM; ++team_itr )
	{
		for( int party_itr = 0; party_itr < GuildWar_TeamPerPartySlot; ++party_itr )
		{
			for( int user_itr = 0; user_itr < GuildWar_PartyPerUserSlot; ++user_itr )
			{
				if( m_pUserSlot[team_itr][party_itr][user_itr].m_bIsNeedUpdate )
					m_pUserSlot[team_itr][party_itr][user_itr].SetState();
			}
		}
	}

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPWINCONDITION );
	if ( !pSox ) 
	{
		ErrorLog(" CDnGuildWarGameRoomDlg::UpdateSlot: PVPWINCONDITION table Not found!! ");
		return;
	}

	int nItemID = 1;

	for( int i=0 ; i<pSox->GetItemCount() ; ++i )
	{
		nItemID = pSox->GetItemID(i);

		if( m_cMaxPlayerNum == pSox->GetFieldFromLablePtr( nItemID , "PlayerNumber" )->GetInteger() )
			break;
	}

	wsprintf( wszPlayerNum, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120051 ), m_cCurrentPlayerNum,m_cMaxPlayerNum, pSox->GetFieldFromLablePtr( nItemID , "Min_TeamPlayerNum_Needed" )->GetInteger() );

	m_pUIPlayerNum->SetText( wszPlayerNum );
}

void CDnGuildWarRoomDlg::SetUserState( UINT uiSessionID , UINT  uiUserState , bool IsOuterCall )
{
	sUserSlot * pSlot = NULL;
	bool IsMy =  false;
	bool IsChangeMaster =  false;
	WCHAR * wszMasterName = NULL;

	pSlot = (sUserSlot *)FindUserSlot( uiSessionID );

	if( pSlot )
	{
		if( !IsMaster(pSlot->m_uiUserState) && IsMaster(uiUserState) )
		{
			IsChangeMaster= true;
			wszMasterName = pSlot->m_wszPlayerName;
			if( CDnBridgeTask::GetInstance().GetSessionID() ==  uiSessionID )//When it is self
				IsMy = true;
		}
		pSlot->m_uiUserState =  uiUserState;
		pSlot->m_bIsNeedUpdate =  true;
	}

	if( CDnBridgeTask::GetInstance().GetSessionID() ==  uiSessionID )//When it is self
	{
		m_uiMyUserState = uiUserState;		
	}

	if( IsOuterCall )
	{
		//became the room master.
		if( IsMy && IsChangeMaster)
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120132 ) , MB_OK );
		else if( IsChangeMaster )
		{
			WCHAR wszMessage[100];
			SecureZeroMemory(wszMessage,sizeof(wszMessage));
			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120131 ), wszMasterName);

			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"",wszMessage);
		}
	}

	UpdateSlot( false );
}

int CDnGuildWarRoomDlg::GetCurMapIndex()
{
	return 0;
}


void CDnGuildWarRoomDlg::UpdatePvPLevel( SCPVP_LEVEL* pPacket )
{

}

void CDnGuildWarRoomDlg::ShowPVPPopUpDlg(bool bShow)
{

}

void CDnGuildWarRoomDlg::PlaySound( byte cType )
{

}

void CDnGuildWarRoomDlg::ChangeTeam( UINT uiSessionID , int cTeam, char cTeamSlotIndex )
{
	int nPlayerTeamIndex = -1;

	switch(cTeam)
	{
	case (PvPCommon::Team::A):
		nPlayerTeamIndex = 0;
		break;
	case (PvPCommon::Team::B):
		nPlayerTeamIndex = 1;
		break;
	case (PvPCommon::Team::Observer):
		break;
	}

	//Search first
	int iSlotIndex = -1;
	int iPlayerIndex= -1;

	if( CDnBridgeTask::GetInstance().GetSessionID() ==  uiSessionID )//When it is self 
		m_cMyTeam = nPlayerTeamIndex;

	bool IsObserver = false;

	for(int i=0;i<GetInterface().GetPVPLobbyChatTabDlg()->GetObserverNumber();i++)
	{
		if(GetInterface().GetPVPLobbyChatTabDlg()->GetSessionIDFromList(i) == uiSessionID) 
		{
			IsObserver = true; // When session ID is one of the observer list = switch observer -> player
		}
	}

	if(!IsObserver) // Normal movement / Normal -> Observer movement
	{
		sUserSlot * sUser = (sUserSlot *)FindUserSlot( uiSessionID );

		if( sUser ) // Found in default slots = I am a slot player
		{
			if( nPlayerTeamIndex == sUser->m_cTeam)  // SlotIndex = Team A, B 
			{
				DebugLog("CDnGuildWarGameRoomDlg::ChangeTeam  %d is request change same team( %d).",uiSessionID, nPlayerTeamIndex );
				return;
			}

			sUserSlot MovePlayer = (*sUser);

			RemovePlayer( uiSessionID );

			if( cTeam != PvPCommon::Team::Observer )
				InsertPlayer( cTeam, MovePlayer.m_uiUserState, uiSessionID, MovePlayer.m_nJobID, MovePlayer.m_cLevel, MovePlayer.m_cPVPLevel, 
				MovePlayer.m_wszPlayerName, cTeamSlotIndex );
			else if( cTeam == PvPCommon::Team::Observer )
				GetInterface().GetPVPLobbyChatTabDlg()->InsertObserver( cTeam
																		, MovePlayer.m_uiUserState
																		, uiSessionID
																		, MovePlayer.m_nJobID
																		, MovePlayer.m_cLevel
																		, MovePlayer.m_cPVPLevel
																		, MovePlayer.m_wszPlayerName
																		);
		}
	}
	else // Observer -> Player transition
	{

		CDnPVPLobbyChatTabDlg::sObserverSlot sSlot = GetInterface().GetPVPLobbyChatTabDlg()->GetObserverSlotFromSessionID(uiSessionID); // Information stored in the observer
		GetInterface().GetPVPLobbyChatTabDlg()->RemoveObserver(uiSessionID,false);  // Remove from observer slot.

		InsertPlayer(cTeam ,
			sSlot.uiUserState,	
			sSlot.uiSessionID , 
			sSlot.iJobID ,
			sSlot.cLevel ,
			sSlot.cPVPLevel ,
			sSlot.wszPlayerName,
			cTeamSlotIndex 
			); // Pass to player

		CEtUIListBoxEx* pUI = GetInterface().GetPVPLobbyChatTabDlg()->GetObserverListBox();
#ifdef PRE_MOD_PVPOBSERVER
		if(pUI->GetSelectedIndex() == -1 &&
			( CGlobalInfo::GetInstance().m_cLocalAccountLevel < eAccountLevel::AccountLevel_New ) )
#else
		if(pUI->GetSelectedIndex() == -1)
#endif // PRE_MOD_PVPOBSERVER
		{
			m_pKick->Enable(false);
			m_pPermanenceKick->Enable(false);
		}
	}
}

void CDnGuildWarRoomDlg::EnableKickButton(bool bTrue)
{

}

void CDnGuildWarRoomDlg::ResetSelectButton()
{

}

void CDnGuildWarRoomDlg::BaseProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnGuildWarRoomDlg::SetButtonState()
{
	bool IsEnableCancel =  false;
	bool IsEnableReady  =  false;
	bool IsEnableTeamChange =  false;
	bool IsEnableStartGame =  false;
	bool IsEnableEdit = false;
	bool IsEnableReadyCancel =  false;
	bool IsEnablePosition = false;
	bool IsEnableMoveParty = false;

	bool IsEnableMoveToObserver = false;
	bool IsEnableMoveToPlay = false;

	byte cflag = 0;

#ifdef PRE_MOD_PVPOBSERVER	
	bool bGameMaster = ( CGlobalInfo::GetInstance().m_cLocalAccountLevel >= eAccountLevel::AccountLevel_New );	
#endif // PRE_MOD_PVPOBSERVER

	if( IsGuildMaster( m_uiMyUserState ) )
	{
		m_pUserMove->Show( !m_bMoveParty );
		m_pUserMoveOK->Show( m_bMoveParty );
	}
	else
	{
		m_pUserMove->Show( false );
		m_pUserMoveOK->Show( false );
	}

	//Room is in waiting state
	if(IsReadyRoom())
	{
		m_pSatrtButton->Show( IsMaster( m_uiMyUserState ) );
		m_pReadyButton->Show( !IsMaster( m_uiMyUserState ) );
		m_pReadyCancelButton->Show( !IsMaster( m_uiMyUserState ) );

		m_pMoveToObserver->Enable( !IsMaster( m_uiMyUserState ) );
		m_pMoveToPlay->Enable( !IsMaster( m_uiMyUserState ) );

		if( IsGuildMaster(m_uiMyUserState ) )
			IsEnableMoveParty = true;

		if( m_uiMyUserState & PvPCommon::UserState::Captain  ) //Room master in idle/no state 
		{			
			IsEnableTeamChange = true;
			IsEnableCancel = true;
			IsEnableStartGame = true;
			IsEnableEdit = true;

			m_pReadyCancelButton->Show(false);
			m_pReadyButton->Show(false);
		}
		else if( m_uiMyUserState & PvPCommon::UserState::Ready ) //Ready-state user 
		{
			IsEnableReady = true;
			m_pReadyCancelButton->Show(true);
			m_pReadyButton->Show(false);
			IsEnableMoveToObserver = false;
			IsEnableMoveParty = false;
		}
		else
		{
			IsEnableTeamChange = true;
			IsEnableCancel = true;
			IsEnableReady = true;

			m_pReadyCancelButton->Show(false);
			m_pReadyButton->Show(true);

			IsEnableMoveToObserver = true;
			IsEnableMoveToPlay = true;
		}

		if( m_bIsGuildWarSystem )
		{
			IsEnableReady = false;
			IsEnableStartGame = false;
		}
	}
	//Starting 5.4.3.2.1 
	if( IsStartingRoom())
	{
		IsEnableMoveParty = false;

		m_pSatrtButton->Show( IsMaster( m_uiMyUserState ) );
		m_pReadyButton->Show( !IsMaster( m_uiMyUserState ) );

		IsEnableMoveToObserver = false;
		IsEnableMoveToPlay = false;

		if( m_uiMyUserState == PvPCommon::UserState::None ) //User in no state 
		{
			IsEnableTeamChange = true;
			IsEnableCancel = true;
		}
	}

	//Game in progress
	if( IsSyncingRoom() || IsPlayingRoom() )
	{
		IsEnableMoveParty = false;

		m_pSatrtButton->Show( IsMaster( m_uiMyUserState ) );
		m_pReadyButton->Show( !IsMaster( m_uiMyUserState ) );

		m_pMoveToObserver->Enable( !IsMaster( m_uiMyUserState ) );
		m_pMoveToPlay->Enable( !IsMaster( m_uiMyUserState ) );

		if(  m_IsBreakIntoFlag )//Join-in-progress allowed 
		{
			if( m_uiMyUserState == PvPCommon::UserState::None 
				|| ( (m_uiMyUserState & PvPCommon::UserState::Captain) && !(m_uiMyUserState & PvPCommon::UserState::Playing) && !(m_uiMyUserState & PvPCommon::UserState::Syncing) )
				|| m_uiMyUserState == PvPCommon::UserState::GuildWarCaptain
				) //User, room master, or guild master in no state
			{
				m_pSatrtButton->Show( true );
				m_pReadyButton->Show( false );
				m_pReadyCancelButton->Show(false);

				IsEnableTeamChange = true;
				IsEnableCancel = true;
				IsEnableStartGame =true;
				IsEnableMoveToObserver = true;
				IsEnableMoveToPlay = true;
			}
		}else
		{
			if( m_uiMyUserState == PvPCommon::UserState::None ) //User in no state 
			{
				IsEnableTeamChange = true;
				IsEnableCancel = true;

				IsEnableMoveToObserver = true;
				IsEnableMoveToPlay = true;
				if(m_cMyTeam != TeamA && m_cMyTeam != TeamB)  // Allow drop-in when I am an observer
				{
					IsEnableStartGame = true;
					m_pReadyButton->Show(false);
					m_pSatrtButton->Show( true );
					m_pSatrtButton->Enable(true);
				}
				else
				{
					m_pReadyButton->Show(true);
					m_pSatrtButton->Show( false );
					m_pSatrtButton->Enable(false);
				}
			}
		}
	}


	bool IsPlayerObserver = false;

	int nSessionID = CDnBridgeTask::GetInstance().GetSessionID();

	for(int i=0;i<GetInterface().GetPVPLobbyChatTabDlg()->GetObserverNumber();i++)
	{
		if(GetInterface().GetPVPLobbyChatTabDlg()->GetSessionIDFromList(i) == nSessionID)  // I am an observer
		{
			IsPlayerObserver = true;
		}
	}

	if(IsPlayerObserver)
	{
		m_pMoveToObserver->Show(false); // Hide if I am an observer.
		m_pMoveToPlay->Show(true);      // Show join game button if I am an observer.

		if(m_cCurrentPlayerNum == m_cMaxPlayerNum) // If the room is full, cannot switch to player.
		{
			IsEnableMoveToPlay = false;

		}


		IsEnableReady = false;          // Also disable ready.
		IsEnableTeamChange = false;     // Disable team change as well.
	}
	else
	{
		m_pMoveToObserver->Show(true); // Otherwise display
		m_pMoveToPlay->Show(false); // Hide join game

#ifdef PRE_MOD_PVPOBSERVER
		int nPlayerCnt = m_bEventRoom ? PvPCommon::Common::ExtendMaxObserverPlayer : PvPCommon::Common::MaxObserverPlayer;		
		if(GetInterface().GetPVPLobbyChatTabDlg()->GetObserverNumber() >= nPlayerCnt ) // 16 observers for GMs.
#else
		if(GetInterface().GetPVPLobbyChatTabDlg()->GetObserverNumber() >= PvPCommon::Common::MaxObserverPlayer) // Currently up to 4 players only
#endif // PRE_MOD_PVPOBSERVER
		{
			IsEnableMoveToObserver = false; // Disable the spectate button when observer slots are full
		}

	}

	// Rotha - PRE_FIX_18699
	if(m_cMyTeam == TeamA) 
	{
		if(m_cMaxPlayerNum/2 == m_cCurrentTeamNum_B) // Since team change moves to the opposing team, check if the opposing team is full.
		{
			IsEnableTeamChange = false; // Disable if full.  
		}

	}
	else if(m_cMyTeam == TeamB)
	{
		if(m_cMaxPlayerNum/2 == m_cCurrentTeamNum_A) // Since team change moves to the opposing team, check if the opposing team is full.
		{
			IsEnableTeamChange = false; // Disable if full.  
		}
	}       
	// PRE_FIX_18699 End

	if(IsSyncingRoom()) // Disable the Start button if the game is preparing. 
	{
		IsEnableStartGame = false; 
	}

	if( m_bIsGuildWarSystem )	//If the room was created by the guild war system
	{
#ifdef PRE_MOD_PVPOBSERVER
		if( bGameMaster && IsMaster( m_uiMyUserState ) )
		{
			m_pRoomEditButton->Show( true );				
			IsEnableEdit = true;		
		}
		else
		{
			m_pRoomEditButton->Show( false );
			IsEnableEdit = false;
		}

		IsEnableMoveToObserver = false;
		IsEnableMoveToPlay = false;
		IsEnableTeamChange = false;
#else
		m_pRoomEditButton->Show( false );
		IsEnableMoveToObserver = false;
		IsEnableMoveToPlay = false;
		IsEnableEdit = false;
		IsEnableTeamChange = false;
#endif // PRE_MOD_PVPOBSERVER
	}

	m_pReadyButton->Enable(IsEnableReady);
	m_pReadyCancelButton->Enable(IsEnableReady);

	m_pSatrtButton->Enable( IsEnableStartGame );
	m_pCancelButton->Enable(IsEnableCancel);
	m_pTeamChangeButton->Enable(IsEnableTeamChange);
	m_pRoomEditButton->Show(IsEnableEdit);
	m_pKick->Show(IsEnableEdit);
	m_pPermanenceKick->Show(IsEnableEdit);
	m_pUserMove->Enable( IsEnableMoveParty );
	m_pUserMoveOK->Enable( IsEnableMoveParty );

	if( AccountLevel_New <= CGlobalInfo::GetInstance().m_cLocalAccountLevel )
	{
#ifdef PRE_MOD_PVPOBSERVER 
		if( m_bEventRoom )
			IsEnableMoveToObserver = IsEnableMoveToPlay = false;		
#endif // PRE_MOD_PVPOBSERVER
		m_pMoveToObserver->Enable(IsEnableMoveToObserver);
		m_pMoveToPlay->Enable(IsEnableMoveToPlay);
	}
	else
	{
		m_pMoveToObserver->Enable(false);
		m_pMoveToPlay->Enable(false);
	}
}

void * CDnGuildWarRoomDlg::FindUserSlot( UINT uiSessionID )
{
	for( int team_itr = 0; team_itr < MAXTeamNUM; ++team_itr )
	{
		for( int party_itr = 0; party_itr < GuildWar_TeamPerPartySlot; ++party_itr )
		{
			for( int user_itr = 0; user_itr < GuildWar_PartyPerUserSlot; ++user_itr )
			{
				if( m_pUserSlot[team_itr][party_itr][user_itr].m_uiSessionID == uiSessionID )
					return &(m_pUserSlot[team_itr][party_itr][user_itr]);
			}
		}
	}

	ErrorLog("CDnGuildWarRoomDlg::FindUserSlot:: User Not Found%d",uiSessionID);

	return NULL;
}
