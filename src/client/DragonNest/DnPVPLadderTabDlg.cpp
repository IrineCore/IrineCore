#include "StdAfx.h"
#include "DnPVPLadderTabDlg.h"
#include "DnInterface.h"
#include "EtUIDialog.h"
#include "SystemSendPacket.h"
#include "DnPVPLobbyVillageTask.h"
#include "TaskManager.h"
#include "PVPSendPacket.h"
#include "DnTableDB.h"
#include "DnChatTabDlg.h"
#include "DnPVPRoomListDlg.h"
#include "DnPVPLadderSystemDlg.h"
#include "DnPVPLobbyChatTabDlg.h"

#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
const DWORD PVP_L0BBY_CHANNEL_SELECT_GAP = 1000;
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

CDnPVPLadderTabDlg::CDnPVPLadderTabDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{   
	m_pRButtonRoomList = NULL;

	for(int i=0; i<LadderPage::Max; i++)
		m_pRButtonLadder[i] = NULL;

	m_nSelectedLadderChannel = LadderSystem::MatchType::eCode::None;

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	m_pRButtonBeginnerRoomList = NULL;
	m_eSelectedGradeChannel = PvPCommon::RoomType::eRoomType::max;
	m_eLastSelectdGradeChannel = PvPCommon::RoomType::eRoomType::max;
	m_dwLastSelectChannelTime = 0;
	m_bIsRegular = false;
	m_iLadderState = LadderSystem::RoomState::None;
	m_bChannelTabDelay = false;
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	m_bVillageAccessMode = false;
#endif // #ifdef PRE_ADD_PVP_VILLAGE_ACCESS
}

CDnPVPLadderTabDlg::~CDnPVPLadderTabDlg(void)
{
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	// If entering PVP lobby from village, record the selected channel tab in bridge task.
	// This is to keep that tab when entering from village, quickly entering a room, and coming back out.

	CDnBridgeTask* pBridgeTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
	if( pBridgeTask )
	{
		if( m_bVillageAccessMode && 
			(m_eSelectedGradeChannel != PvPCommon::RoomType::max) )
		{
			pBridgeTask->SetSelectedChannelEnterPVPLobbyFromVillageAccessMode( m_eSelectedGradeChannel );
		}
		else
		{
			pBridgeTask->SetSelectedChannelEnterPVPLobbyFromVillageAccessMode( PvPCommon::RoomType::max );
		}
	}
#endif // #ifdef PRE_ADD_PVP_VILLAGE_ACCESS
#endif //#ifdef PRE_ADD_COLOSSEUM_BEGINNER
}


void CDnPVPLadderTabDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpTabDlg.ui" ).c_str(), bShow );
}

void CDnPVPLadderTabDlg::InitialUpdate()
{
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	m_pRButtonBeginnerRoomList = GetControl<CEtUIRadioButton>( "ID_RBT_COLOBEGINNER" );
	m_pRButtonRoomList = GetControl<CEtUIRadioButton>( "ID_RBT_COLO" );
	m_pRButtonBeginnerRoomList->SetChecked( true );
	m_pRButtonBeginnerRoomList->SetTooltipText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120158 ) );
	m_pRButtonRoomList->SetTooltipText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120159 ) );
#else
	m_pRButtonRoomList = GetControl<CEtUIRadioButton>( "ID_RBT_COLO" );
	m_pRButtonRoomList->SetChecked(true);
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	for(int i=0; i<LadderPage::Max; i++)
	{
		m_pRButtonLadder[i] =GetControl<CEtUIRadioButton>( FormatA("ID_RBT_MATCH%d",i+1).c_str() );
		m_pRButtonLadder[i]->SetChecked(false);
		m_pRButtonLadder[i]->Show(false);
	}

	for(int i=0; i<LadderSystem::MatchType::eCode::MaxCount; i++) 
	{
#if !defined PRE_ADD_PVP_HIDE_LADDERTAB
		m_pRButtonLadder[i]->Show(true);
#endif
#ifdef PRE_ADD_LADDER_TAB_TOOLTIP
		m_pRButtonLadder[i]->SetTooltipText( 
			FormatW( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, (i == LadderPage::_1vs1) ? 8323 : 8324 ) , 
			static_cast<int>(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::PvPLadder_LimitLevel) ) ).c_str() );
#endif
	}

#ifdef PRE_FIX_TEAM_LADDER_3vs3
	m_pRButtonLadder[LadderPage::_2vs2]->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 126028)); // Force-switch to 3:3.
	m_pRButtonLadder[LadderPage::_3vs3]->Show(false);
	m_pRButtonLadder[LadderPage::_4vs4]->Show(false);
#endif
}

void CDnPVPLadderTabDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_PVP);
		}
#endif
	}
	else if(nCommand == EVENT_RADIOBUTTON_CHANGED)
	{
		if(IsCmdControl("ID_RBT_COLO"))
		{
			if(m_nSelectedLadderChannel != LadderSystem::MatchType::eCode::None)
				SendPvPLeaveChannel();

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
			if( bTriggeredByUser )
			{
				SendPVPChangeChannel( PvPCommon::RoomType::eRoomType::regular );
				SetChannelTabSelectTimeStampForInputDelay();
			}
#endif
			return;
		}
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		else 
		if( IsCmdControl("ID_RBT_COLOBEGINNER") )
		{
			if(m_nSelectedLadderChannel != LadderSystem::MatchType::eCode::None)
				SendPvPLeaveChannel();

			if( bTriggeredByUser )
			{
				SendPVPChangeChannel( PvPCommon::RoomType::eRoomType::beginner );
				SetChannelTabSelectTimeStampForInputDelay();
			}
			return;
		}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

		if(CDnActor::s_hLocalActor)
		{
			WCHAR wszString[256];
			int nLevelLimit = static_cast<int>(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::PvPLadder_LimitLevel));
			if( CDnActor::s_hLocalActor->GetLevel() < nLevelLimit )
			{
				swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126174 ) , nLevelLimit  );
				GetInterface().MessageBox(wszString);

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
				if( PvPCommon::RoomType::eRoomType::beginner == m_eLastSelectdGradeChannel )
				{
					m_pRButtonBeginnerRoomList->SetChecked(true);
				}
				else
				if( PvPCommon::RoomType::eRoomType::regular == m_eLastSelectdGradeChannel )
				{
					m_pRButtonRoomList->SetChecked(true);
				}
#else
				m_pRButtonRoomList->SetChecked(true);
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
				
				return;
			}
		}

		for(int i=0; i<LadderPage::Max; i++)
		{
			int nSendLadderType = i+1;

			if(IsCmdControl( FormatA("ID_RBT_MATCH%d", nSendLadderType ).c_str() ))
			{
				if(nSendLadderType > LadderSystem::MatchType::None && nSendLadderType <= LadderSystem::MatchType::_4vs4 )
				{
#ifdef PRE_FIX_TEAM_LADDER_3vs3
					if( i == LadderPage::_2vs2 )
					{
						if(m_nSelectedLadderChannel != LadderSystem::MatchType::eCode::_3vs3)
						{
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
							m_eSelectedGradeChannel = PvPCommon::RoomType::eRoomType::max;

							CDnBridgeTask* pBridgeTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
							if( pBridgeTask )
							{
								if( m_bVillageAccessMode )
								{
									pBridgeTask->SetEnterLadderFromVillageAccessMode();
								}
							}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
							SendPvPEnterChannel(LadderSystem::MatchType::eCode::_3vs3);
						}

						return;
					}
#endif
					if( m_nSelectedLadderChannel != nSendLadderType )
					{
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
						m_eSelectedGradeChannel = PvPCommon::RoomType::eRoomType::max;

						CDnBridgeTask* pBridgeTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
						if( pBridgeTask )
						{
							if( m_bVillageAccessMode )
							{
								pBridgeTask->SetEnterLadderFromVillageAccessMode();
							}
						}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
						SendPvPEnterChannel( (LadderSystem::MatchType::eCode)nSendLadderType );
					}
				}
			}
		}
	}
}

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
void CDnPVPLadderTabDlg::SetSelectedGradeChannelTab( PvPCommon::RoomType::eRoomType eSelectedRoomType )
{
	m_eSelectedGradeChannel = eSelectedRoomType;
	m_eLastSelectdGradeChannel = m_eSelectedGradeChannel;
	
	m_nSelectedLadderChannel = LadderSystem::MatchType::eCode::None;


	switch( m_eSelectedGradeChannel )
	{
		case PvPCommon::RoomType::beginner:
			{
				if( !m_pRButtonBeginnerRoomList->IsChecked() )
					m_pRButtonBeginnerRoomList->SetChecked(true);
			}
			break;

		case PvPCommon::RoomType::regular:
			{
				if( !m_pRButtonRoomList->IsChecked() )
					m_pRButtonRoomList->SetChecked( true );
			}
			break;
	}
}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

void CDnPVPLadderTabDlg::SelectLadderTab(int nType)
{
	m_nSelectedLadderChannel = nType;

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	m_eSelectedGradeChannel = PvPCommon::RoomType::eRoomType::max;
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	GetInterface().GetPVPLobbyChatTabDlg()->GetPVPOptionDlg()->EnalbeFatigue(false);
	switch(nType)
	{
	case LadderSystem::MatchType::eCode::None:
		{
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
			if( PvPCommon::RoomType::eRoomType::beginner == m_eLastSelectdGradeChannel )
			{
				if(!m_pRButtonBeginnerRoomList->IsChecked())
					m_pRButtonBeginnerRoomList->SetChecked(true);
				m_eSelectedGradeChannel = m_eLastSelectdGradeChannel;
			}
			else
				if( PvPCommon::RoomType::eRoomType::regular == m_eLastSelectdGradeChannel )
			{
				if(!m_pRButtonRoomList->IsChecked())
					m_pRButtonRoomList->SetChecked(true);
				m_eSelectedGradeChannel = m_eLastSelectdGradeChannel;
			}
#else
			if(!m_pRButtonRoomList->IsChecked())
				m_pRButtonRoomList->SetChecked(true);
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
			GetInterface().GetPVPLobbyChatTabDlg()->GetPVPOptionDlg()->EnalbeFatigue(true);
		}
		break;

#ifdef PRE_FIX_TEAM_LADDER_3vs3
	case LadderSystem::MatchType::eCode::_3vs3:
		{
			if(!m_pRButtonLadder[LadderPage::_2vs2]->IsChecked())
				m_pRButtonLadder[LadderPage::_2vs2]->SetChecked(true);
		}
		break;
#endif

	default:
		{
			int nArrIndex = nType - 1;
			if(nArrIndex >= 0 && nArrIndex < LadderPage::Max )
			{
				if(!m_pRButtonLadder[nArrIndex]->IsChecked())
					m_pRButtonLadder[nArrIndex]->SetChecked(true);
			}
		}
		break;
	}
}

void CDnPVPLadderTabDlg::EnableTabButton( int nIndex, bool bEnable, bool bAll )
{
	if(bAll)
	{
		m_pRButtonRoomList->Enable(bEnable);
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		if( false == m_bIsRegular )
			m_pRButtonBeginnerRoomList->Enable( bEnable );
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

		for(int i=0; i<LadderPage::Max; i++)
			m_pRButtonLadder[i]->Enable(bEnable);

		return;
	}

	switch(nIndex)
	{
		case LadderSystem::MatchType::eCode::None:
			{
				m_pRButtonRoomList->Enable(bEnable);
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
				if( false == m_bIsRegular )
					m_pRButtonBeginnerRoomList->Enable( bEnable );
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
			}
			break;

		default:
			{
				int nArrIndex = nIndex - 1;
				if(nArrIndex >= 0 && nArrIndex < LadderPage::Max )
					m_pRButtonLadder[nArrIndex]->Enable(bEnable);

				break;
			}
	}
}

void CDnPVPLadderTabDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	if( IsShow() )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
		if( pPartyTask && CDnPartyTask::IsActive() )
		{
			TPvPGroup *pPvPInfo = pPartyTask->GetPvPInfo();
			char cPVPGrade = pPvPInfo->cLevel;
			if( cPVPGrade < CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::Colosseum_Limit_Rank) )
			{
				m_bIsRegular = false;
			}
			else
			{
				m_bIsRegular = true;
			}
		}

		DWORD dwNowTime = GetTickCount();
		if( m_bChannelTabDelay )
		{
			if( PVP_L0BBY_CHANNEL_SELECT_GAP < (dwNowTime - m_dwLastSelectChannelTime) )
			{
				m_dwLastSelectChannelTime = UINT_MAX;
				m_bChannelTabDelay = false;

				if( false == m_bIsRegular )
					m_pRButtonBeginnerRoomList->Enable( true );
				else
					m_pRButtonBeginnerRoomList->Enable( false );

				m_pRButtonRoomList->Enable( true );
			}
		}
		else
		{
			if( LadderSystem::RoomState::None == m_iLadderState ||
				LadderSystem::RoomState::WaitUser == m_iLadderState )
			{
				if( false == m_bIsRegular )
				{
					m_pRButtonBeginnerRoomList->Enable( true );
				}
				else
				{
					m_pRButtonBeginnerRoomList->Enable( false );
					if( m_pRButtonBeginnerRoomList->IsChecked() )
					{
						// Seems packet order differs for certain characters on live server, so get current own pvp rank
						// Supplement the tab button update here.

						if ( m_nSelectedLadderChannel == LadderSystem::MatchType::eCode::None )
						{
							m_pRButtonBeginnerRoomList->SetChecked( false );
							SendPVPChangeChannel( PvPCommon::RoomType::eRoomType::regular );
						}
					}
				}
			}
		}
	}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
}


void CDnPVPLadderTabDlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	if( bShow )
	{
		// Keep beginner channel disabled until userenter packet arrives from server.
		m_pRButtonBeginnerRoomList->Enable( false );
	}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
}

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
void CDnPVPLadderTabDlg::UpdatePvPChannelTab( void )
{
	CDnPartyTask* pPartyTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
	if( pPartyTask )
	{
		TPvPGroup *pPvPInfo = pPartyTask->GetPvPInfo();
		char cPVPGrade = pPvPInfo->cLevel;
		if( cPVPGrade < CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::Colosseum_Limit_Rank) )
		{
			// Default is beginner channel.
			m_pRButtonBeginnerRoomList->Enable( true );
			m_bIsRegular = false;
		}
		else
		{
			// Intermediate soldier or higher cannot enter the novice channel.
			m_pRButtonBeginnerRoomList->Enable( false );
			m_bIsRegular = true;
		}
	}
}

void CDnPVPLadderTabDlg::SelectPvPChannelProperly( void )
{
	CDnPartyTask* pPartyTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
	if( pPartyTask )
	{
		TPvPGroup *pPvPInfo = pPartyTask->GetPvPInfo();
		char cPVPGrade = pPvPInfo->cLevel;
		if( cPVPGrade < CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::Colosseum_Limit_Rank) )
		{
			// Default is beginner channel.
			SendPVPChangeChannel( PvPCommon::RoomType::eRoomType::beginner );
		}
		else
		{
			// Intermediate soldier or higher cannot enter the novice channel.
			SendPVPChangeChannel( PvPCommon::RoomType::eRoomType::regular );
		}
	}
}

void CDnPVPLadderTabDlg::UpdateAndSelectTabProperly()
{
	UpdatePvPChannelTab();
	SelectPvPChannelProperly();
}


void CDnPVPLadderTabDlg::SetChannelTabSelectTimeStampForInputDelay()
{
	m_dwLastSelectChannelTime = GetTickCount();
	m_bChannelTabDelay = true;

	m_pRButtonRoomList->Enable( false );
	m_pRButtonBeginnerRoomList->Enable( false );
}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
void CDnPVPLadderTabDlg::EnableVillageAccessMode(bool bTrue)
{
	const float fOffsetX = 11.0f / DEFAULT_UI_SCREEN_WIDTH;
	const float fOffsetY = 11.0f / DEFAULT_UI_SCREEN_HEIGHT;

	float fMoveX = GetInterface().GetPvPRoomListDlg()->GetXCoord() - GetXCoord();
	float fMoveY = GetInterface().GetPvPRoomListDlg()->GetYCoord() - GetYCoord();

	MoveDialog( fMoveX+fOffsetX , fMoveY+fOffsetY );

	m_bVillageAccessMode = true;
}
#endif
