#include "StdAfx.h"
#include "DnPVPMakeRoomDlg.h"
#include "DnInterface.h"
#include "PVPSendPacket.h"
#include "DnTableDB.h"
#include "DnInterfaceString.h"
#include "DnPVPLobbyVillageTask.h"
#include "TaskManager.h"
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#include "DnPVPLadderTabDlg.h"
#endif
#ifdef PRE_ADD_PVP_HELP_MESSAGE
#include "DnSimpleTooltipDlg.h"
#endif
#ifdef PRE_WORLDCOMBINE_PVP
#include "DnLocalPlayerActor.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef PRE_MOD_PVP_ROOM_CREATE

CDnPVPMakeRoomDlg::CDnPVPMakeRoomDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pEditBoxRoomName(NULL)
, m_pEditBoxMinLevel(NULL)
, m_pEditBoxMaxLevel(NULL)
, m_pComboBoxMapSelection(NULL)
, m_pComboBoxPlayerNum(NULL)
, m_pCheckBoxInGameJoin(NULL)
, m_pComboBoxGameMode(NULL)
, m_pComboBoxVictoryCondition(NULL)
, m_pExitButton(NULL)
, m_pMiniMap(NULL)
, m_pGameModeImage(NULL)
, m_pCheckBoxDropItem(NULL)
, m_pCheckBoxRevision(NULL)
, m_pCheckBoxRandomTeamMode(NULL)
, m_pCheckBoxRandomOrder(NULL)
, m_pStaticRandomOrder(NULL)
, m_nCurrSelectModeVectorIndex(0)
#ifdef PRE_MOD_PVPOBSERVER
, m_pCheckBoxObserver(NULL)
#endif // PRE_MOD_PVPOBSERVER
, m_pCheckBoxShowHP(NULL)
, m_pEditBoxPassWord(NULL)
#ifdef PRE_WORLDCOMBINE_PVP
, m_pStaticWorldCombineRoom( NULL )
, m_pCheckBoxWorldCombineRoom( NULL )
#endif // PRE_WORLDCOMBINE_PVP
{
	m_SelectedIndex = -1;
	m_SelectedPlayerNum = -1;
	m_SelectedMapIndex = -1;
	m_nGameModeTableID = 0;
	m_nWinCondition = 0;
	m_nPlayTimeSec = 0;

	m_IsInGameJoin = false;
	m_IsFirstInputRoomName = true;
	m_bDropItem = false;
	m_bShowHp = false;
	m_bRandomTeamMode = false;
	m_bRevision = false;
	m_bRandomOrder = false;
#ifdef PRE_MOD_PVPOBSERVER
	m_bObserverAccess = true;
#endif // PRE_MOD_PVPOBSERVER
}

CDnPVPMakeRoomDlg::~CDnPVPMakeRoomDlg(void)
{
	SAFE_RELEASE_SPTR( m_hGameModeImage );
	SAFE_RELEASE_SPTR( m_hMiniMapImage );
}

void CDnPVPMakeRoomDlg::Initialize( bool bShow )
{
	MakePvPGameModeData();
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpMakeRoom.ui" ).c_str(), bShow );
}

void CDnPVPMakeRoomDlg::MakePvPGameModeData()
{
	DNTableFileFormat* pGameModeSox = GetDNTable( CDnTableDB::TPVPGAMEMODE);
	DNTableFileFormat* pPVPMapSox   = GetDNTable( CDnTableDB::TPVPMAP);
	if( !pGameModeSox || !pPVPMapSox ) return;
	
	m_vGameModeList.clear();
	std::vector<sPvPModeData>().swap(m_vGameModeList);

	for(int i = 0 ; i < pGameModeSox->GetItemCount() ; ++i)
	{
		sPvPModeData GameModeData;
		int nCellID		= pGameModeSox->GetItemID(i);
		int nUserCreate = pGameModeSox->GetFieldFromLablePtr(nCellID , "_UserCreate")->GetInteger();
		if( nUserCreate == 0)
			continue;

		int nGameModeID = pGameModeSox->GetFieldFromLablePtr(nCellID , "GamemodeID")->GetInteger();
		if( nGameModeID == -1)
			continue;

#ifdef _FINAL_BUILD 
		if( pGameModeSox->GetFieldFromLablePtr( nCellID, "ReleaseShow" )->GetBool() == false )
			continue;
#endif // #ifdef _FINAL_BUILD

		GameModeData.nTableID			= nCellID;
		GameModeData.nGameMode			= nGameModeID;
		GameModeData.nLadderType	    = pGameModeSox->GetFieldFromLablePtr(nCellID , "LadderType")->GetInteger();
		GameModeData.nGameModeStringID  = pGameModeSox->GetFieldFromLablePtr(nCellID , "GameModeUIString")->GetInteger();
		GameModeData.nNumOfPlayers_Min	= pGameModeSox->GetFieldFromLablePtr(nCellID , "NumOfPlayers_Min")->GetInteger();
		GameModeData.nNumOfPlayers_Max	= pGameModeSox->GetFieldFromLablePtr(nCellID , "NumOfPlayers_Max")->GetInteger();
		GameModeData.strModeImageName	= pGameModeSox->GetFieldFromLablePtr(nCellID , "_ModeImage")->GetString();
		GameModeData.bIsReleaseShow		= pGameModeSox->GetFieldFromLablePtr(nCellID , "ReleaseShow")->GetBool();

		// From here on, read from PVPMapTable.
		for(int j = 0 ; j < pPVPMapSox->GetItemCount() ; ++j)
		{
			sPvPMapData mapData;
			bool	bIsExist = false;
			int		nCellID = pPVPMapSox->GetItemID(j);

			for(int k = 0 ; k < MapData_ModeMAX ; ++k) 
			{				
				int nGameModeTableID = pPVPMapSox->GetFieldFromLablePtr(nCellID, FormatA("GameModeTableID_%d",k+1).c_str())->GetInteger();
				if( nGameModeTableID == GameModeData.nTableID)
				{
					int nMapTablD = pPVPMapSox->GetFieldFromLablePtr( nCellID, "MapTableID")->GetInteger();
					if( nMapTablD > 0)
					{
						mapData.nMapTableID		 = nMapTablD;
						mapData.nMapNameStringID = pPVPMapSox->GetFieldFromLablePtr( nCellID, "MapNameUIstring")->GetInteger();
						mapData.strMapImageName  = pPVPMapSox->GetFieldFromLablePtr( nCellID, "MapImage")->GetString();
						mapData.bIsAllowBreak	 = (pPVPMapSox->GetFieldFromLablePtr( nCellID, "Allow_Breakin_PlayingGame" )->GetInteger() == TRUE) ? true : false;
						bIsExist = true;
					}
				}
			}

			if(!bIsExist) // If map info is missing, do not proceed further and bail out here.
				continue;

			for(int j = 0 ; j < MapData_PlayerMAX ; ++j)
				mapData.nNumOfPlayerOpt[j] = pPVPMapSox->GetFieldFromLablePtr( nCellID , FormatA("NumOfPlayersOption%d",j+1).c_str())->GetInteger();

			mapData.nDefaultNumOfPlayerOpt = pPVPMapSox->GetFieldFromLablePtr( nCellID, "DefaultNumOfPlayers")->GetInteger();
			GameModeData.vAssignMapDataList.push_back(mapData);
		}
		m_vGameModeList.push_back(GameModeData);
	}

	if(m_vGameModeList.empty())
		DebugLog("Failed to create m_vGameModeList in CDnPVPMakeRoomDlg::MakePvPGameModeData()");
}

void CDnPVPMakeRoomDlg::InitialUpdate()
{
	m_pEditBoxMinLevel = GetControl<CEtUIEditBox>( "ID_EDITBOX_LV_MIN" );	
	m_pEditBoxMaxLevel = GetControl<CEtUIEditBox>( "ID_EDITBOX_LV_MAX" );
	m_pEditBoxRoomName = GetControl<CEtUIEditBox>( "ID_PVP_MAKEROOM_INPUTNAME" );
	m_pEditBoxPassWord = GetControl<CEtUIEditBox>( "ID_PVP_MAKEROOM_PASSWORD" );

	m_pComboBoxGameMode = GetControl<CEtUIComboBox>( "ID_PVP_MAKEROOM_INPUTMODE" );	
	m_pComboBoxPlayerNum = GetControl<CEtUIComboBox>( "ID_PVP_MAKEROOM_INPUTNUM" );
	m_pComboBoxMapSelection = GetControl<CEtUIComboBox>( "ID_PVP_MAKEROOM_INPUTMAP" );	
	m_pComboBoxVictoryCondition = GetControl<CEtUIComboBox>( "ID_PVP_MAKEROOM_WINCONDITION" );

	m_pCheckBoxShowHP = GetControl<CEtUICheckBox>("ID_CHECKBOX_HP"); 
	m_pCheckBoxRevision = GetControl<CEtUICheckBox>("ID_CHECKBOX_FIT");
	m_pCheckBoxDropItem = GetControl<CEtUICheckBox>("ID_CHECKBOX_ITEM");
	m_pCheckBoxInGameJoin = GetControl<CEtUICheckBox>("ID_PVP_MAKEROOM_INPUTINTRUTION");
	m_pCheckBoxRandomTeamMode = GetControl<CEtUICheckBox>("ID_CHECKBOX_RANDOM");

	m_pMiniMap = GetControl<CEtUITextureControl>( "ID_PVP_MAPIMG" );
	m_pGameModeImage = GetControl<CEtUITextureControl>( "ID_PVP_MODEIMG" );
	m_pExitButton = GetControl<CEtUIButton>( "ID_BUTTON_CLOSE" );
	
	m_bRandomTeamMode = false;
	m_bRevision = false;

	m_pCheckBoxRandomOrder = GetControl<CEtUICheckBox>("ID_CHECKBOX_RANDOMORDER");
	m_pStaticRandomOrder = GetControl<CEtUIStatic>("ID_STATIC19");
	m_bRandomOrder = false;

#ifdef PRE_MOD_PVPOBSERVER
	m_pCheckBoxObserver = GetControl<CEtUICheckBox>("ID_CHECKBOX_OBSERVER");
	m_pCheckBoxObserver->SetChecked( m_bObserverAccess );
#endif // PRE_MOD_PVPOBSERVER

#ifdef PRE_WORLDCOMBINE_PVP
	m_pStaticWorldCombineRoom = GetControl<CEtUIStatic>( "ID_STATIC21" );
	m_pStaticWorldCombineRoom->Show( false );
	m_pCheckBoxWorldCombineRoom = GetControl<CEtUICheckBox>( "ID_CHECKBOX_WORLDCOMBINEROOM" );
	m_pCheckBoxWorldCombineRoom->Show( false );
#endif // PRE_WORLDCOMBINE_PVP

	MakeGameModeDropDownList();		// Set game mode combo box.
	MakeMapDropDownList(0);			// Set up map combo box.
	MakeDropPlayDownList(0);		// Set player count combo box.
	MakeVicConDropDownList();		// Condition combo box setup.

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_PVP_MAKEROOM_CREATE") );
}

void CDnPVPMakeRoomDlg::Show( bool bShow )
{
	if( bShow )
	{
		m_pEditBoxRoomName->ClearText();
		m_pEditBoxPassWord->ClearText();
		m_pEditBoxMinLevel->SetText(L"1");
		m_pEditBoxMaxLevel->SetText(L"100");
		if( !IsEditRoom() )
			m_pEditBoxRoomName->SetText(MakeDefaultRoomName());

		m_IsFirstInputRoomName = true;
		m_SmartMove.MoveCursor();

#ifdef PRE_WORLDCOMBINE_PVP
		bool bShowWorldCombineRoom = false;
		CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
		if( pPVPLobbyTask )
		{
			//if( AccountLevel_New <= pPVPLobbyTask->GetUserInfo().cAccountLevel)
			if( AccountLevel_New <= pPVPLobbyTask->GetUserInfo().cAccountLevel && AccountLevel_DWC > pPVPLobbyTask->GetUserInfo().cAccountLevel )
				bShowWorldCombineRoom = true;
		}
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
		else if( CDnActor::s_hLocalActor )
		{
			CDnLocalPlayerActor* pLocalPlayer = static_cast<CDnLocalPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
			if( pLocalPlayer && pLocalPlayer->IsDeveloperAccountLevel() )
				bShowWorldCombineRoom = true;
		}
#endif // PRE_ADD_PVP_VILLAGE_ACCESS
		if( m_pStaticWorldCombineRoom )
			m_pStaticWorldCombineRoom->Show( bShowWorldCombineRoom );

		if( m_pCheckBoxWorldCombineRoom )
			m_pCheckBoxWorldCombineRoom->Show( bShowWorldCombineRoom );
#endif // PRE_WORLDCOMBINE_PVP
	}
	else
	{
		m_SmartMove.ReturnCursor();
	}
	CEtUIDialog::Show( bShow );
}

void CDnPVPMakeRoomDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_PVP_MAKEROOM_CREATE" ) )
		{
			SendMakeRoom();
			return;
		}

		if( IsCmdControl("ID_PVP_MAKEROOM_CANCEL" ) )
		{
			Show(false);
			return;
		}

		if( IsCmdControl( "ID_BUTTON_CLOSE") )
		{
			Show(false);
			return;
		}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED )
	{
		if(IsCmdControl("ID_PVP_MAKEROOM_INPUTMODE"))
		{
			m_pComboBoxGameMode->GetSelectedValue(m_nGameModeTableID);
			m_SelectedModeIndex = m_pComboBoxGameMode->GetSelectedIndex();

			int nCurSelectedGameModeID = 0;
			if( m_pComboBoxGameMode->GetSelectedValue(nCurSelectedGameModeID) )
			{
				for(int i = 0 ; i < (int)m_vGameModeList.size() ; ++i)
				{
					if(m_vGameModeList[i].nTableID == nCurSelectedGameModeID)
					{
						m_nCurrSelectModeVectorIndex = i;
						MakeGameModeImage(i);
						MakeMapDropDownList(i);
						break;
					}
				}
			}
			MakeVicConDropDownList();
			SetGameModeButtonState();
			InitCheckBox();
			return;
		}

		if(IsCmdControl("ID_PVP_MAKEROOM_INPUTMAP"))
		{
			int nCurMapIndex = 0;
			if( m_pComboBoxMapSelection->GetSelectedValue( nCurMapIndex ) ) 
			{
				int index = m_nCurrSelectModeVectorIndex;
				if( index >= (int)m_vGameModeList.size() || index < 0 )
					return;

				for( int i = 0; i < (int)m_vGameModeList[index].vAssignMapDataList.size() ; ++i ) 
				{
					if( m_vGameModeList[index].vAssignMapDataList[i].nMapTableID == nCurMapIndex )
					{
						MakeMapImage(i);
						MakeDropPlayDownList(i);
						m_pComboBoxMapSelection->GetSelectedValue(m_SelectedMapIndex);
						break;
					}
				}
			}
			MakeVicConDropDownList();
			SetGameModeButtonState();
			return;
		}

		if( IsCmdControl("ID_PVP_MAKEROOM_INPUTNUM" ) )
		{
			m_pComboBoxPlayerNum->GetSelectedValue(m_SelectedPlayerNum);				
			return;
		}

		if( IsCmdControl("ID_PVP_MAKEROOM_WINCONDITION" ) )
		{
			int nIndex;
			m_pComboBoxVictoryCondition->GetSelectedValue(nIndex);
			GetGamemodeInfo( nIndex );
			return;
		}

	}
	else if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if( IsCmdControl("ID_PVP_MAKEROOM_INPUTINTRUTION" ) ) 
		{
			if( m_pCheckBoxInGameJoin )
				m_IsInGameJoin = m_pCheckBoxInGameJoin->IsChecked();
		}

		if( IsCmdControl( "ID_CHECKBOX_ITEM" ) )
		{
			if( m_pCheckBoxDropItem )
				m_bDropItem = m_pCheckBoxDropItem->IsChecked();
		}

		if( IsCmdControl("ID_CHECKBOX_HP") )   
		{
			if(m_pCheckBoxShowHP)
				m_bShowHp = m_pCheckBoxShowHP->IsChecked();
		}

		if( IsCmdControl("ID_CHECKBOX_RANDOM") )   
		{
			if(m_pCheckBoxRandomTeamMode)
				m_bRandomTeamMode = m_pCheckBoxRandomTeamMode->IsChecked();

			m_pCheckBoxRandomOrder->SetChecked(m_bRandomTeamMode);
			m_pCheckBoxRandomOrder->Enable(!m_bRandomTeamMode);
		}

		if( IsCmdControl("ID_CHECKBOX_FIT") )   
		{
			if( m_pCheckBoxRevision )
				m_bRevision = !m_pCheckBoxRevision->IsChecked();
		}

		if( IsCmdControl("ID_CHECKBOX_RANDOMORDER") )   
		{
			if( m_pCheckBoxRandomOrder )
				m_bRandomOrder = m_pCheckBoxRandomOrder->IsChecked();
		}
#ifdef PRE_MOD_PVPOBSERVER
		if( IsCmdControl( "ID_CHECKBOX_OBSERVER" ) )
		{
			if( m_pCheckBoxObserver )
				m_bObserverAccess = m_pCheckBoxObserver->IsChecked();
		}		
#endif // PRE_MOD_PVPOBSERVER
	}
	else if( nCommand == EVENT_EDITBOX_FOCUS )
	{
		if( IsCmdControl("ID_PVP_MAKEROOM_INPUTNAME" ) ) 
		{
			if(m_IsFirstInputRoomName)
			{
				m_pEditBoxRoomName->ClearText();
				m_IsFirstInputRoomName = false;
			}
		}
	}
	else if( nCommand == EVENT_EDITBOX_RELEASEFOCUS)
	{
		if(IsCmdControl("ID_EDITBOX_LV_MIN"))
		{
			if(m_pEditBoxMinLevel->GetTextToInt() > 100 || m_pEditBoxMinLevel->GetTextToInt() < 1)
			{
				m_pEditBoxMinLevel->SetIntToText(1);
				m_pEditBoxMaxLevel->SetIntToText(100);
			}
		}

		if(IsCmdControl("ID_EDITBOX_LV_MAX"))
		{
			if(m_pEditBoxMinLevel->GetTextToInt() >  m_pEditBoxMaxLevel->GetTextToInt())
				m_pEditBoxMaxLevel->SetIntToText(m_pEditBoxMinLevel->GetTextToInt());

			if(m_pEditBoxMaxLevel->GetTextToInt() > 100 || m_pEditBoxMaxLevel->GetTextToInt() < 1)
				m_pEditBoxMaxLevel->SetIntToText(100);
		}
	}
}

#ifdef PRE_ADD_PVP_HELP_MESSAGE
bool CDnPVPMakeRoomDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	bool bResult = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );

	switch( uMsg )
	{
	case WM_LBUTTONDBLCLK:
		{
		}
		break;

	case WM_LBUTTONDOWN:
		{
		}
		break;

	case WM_RBUTTONDOWN:
		{
		}
		break;

	case WM_MOUSEWHEEL:
		{
		}
		break;

	case WM_MOUSEMOVE:
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fMouseX, fMouseY;
			PointToFloat( MousePoint, fMouseX, fMouseY );

			if(m_pComboBoxGameMode == NULL || m_nGameModeTableID <= 0)
				return false;

			if(m_pComboBoxGameMode->IsInside(fMouseX, fMouseY) && m_pComboBoxGameMode->IsShow() )
			{
				m_pComboBoxGameMode->GetSelectedValue(m_nGameModeTableID);

				DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
				if (!pSox)
					return false;

				int nToolTipStringID	= pSox->GetFieldFromLablePtr(m_nGameModeTableID , "_TooltipUIString")->GetInteger();
				std::wstring wszTooltip = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nToolTipStringID );
				if(!wszTooltip.empty())
					m_pComboBoxGameMode->GetParent()->ShowTooltipDlg(m_pComboBoxGameMode, true, wszTooltip, 0xffffffff, true);
			}
		}
		break;
	}

	return bResult;
}
#endif

void CDnPVPMakeRoomDlg::SendMakeRoom()
{
	std::wstring wszRoomName;

	wszRoomName = m_pEditBoxRoomName->GetText();

	std::wstring strTemp = boost::algorithm::trim_copy(wszRoomName);
	if (strTemp.empty())
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPMakeGame::RoomNameIsNULL) , MB_OK);	
		return;
	}

	if( DN_INTERFACE::UTIL::CheckChat( wszRoomName ) )//Handle forbidden word usage
	{
		if( m_pEditBoxRoomName )
			m_pEditBoxRoomName->ClearText();

		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPMakeGame::RoomNameIsWrong) , MB_OK);	
		return;
	}

	std::wstring  wszRoomPW;
	wszRoomPW = m_pEditBoxPassWord->GetText();

#ifdef PRE_WORLDCOMBINE_PVP
	bool bWorldCombineRoom = m_pCheckBoxWorldCombineRoom->IsChecked();
	if( bWorldCombineRoom && wszRoomPW.length() == 0 )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3507 ) , MB_OK );
		return;
	}
#endif // PRE_WORLDCOMBINE_PVP

	GetInterface().SetPVPRoomPassword(wszRoomPW);

	if( m_pEditBoxPassWord->GetTextLength() >= 1 &&  m_pEditBoxPassWord->GetTextLength() <= 3 )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPMakeGame::PasswordShort ) , MB_OK );
		return;
	}
	int nPlayerLevel = 0;

	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
	if(pPVPLobbyTask)
		nPlayerLevel = pPVPLobbyTask->GetUserInfo().cLevel;
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	else if(CDnActor::s_hLocalActor)
		nPlayerLevel = CDnActor::s_hLocalActor->GetLevel();
#endif
	else
		return;

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	char cRoomType = 0;
	CDnPVPLadderTabDlg* pPVPLadderTabDlg = GetInterface().GetPVPLadderTabDlg();
	if( pPVPLadderTabDlg )
	{
		cRoomType = static_cast<char>(pPVPLadderTabDlg->GetSelectedGradeChannel());
	}

	
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	if(m_pEditBoxMinLevel->GetTextToInt() > nPlayerLevel)
		m_pEditBoxMinLevel->SetIntToText(nPlayerLevel);

	if(m_pEditBoxMaxLevel->GetTextToInt() < nPlayerLevel)
		m_pEditBoxMaxLevel->SetIntToText(nPlayerLevel);

	if(m_pEditBoxMinLevel->GetTextToInt() > m_pEditBoxMaxLevel->GetTextToInt())
		m_pEditBoxMaxLevel->SetIntToText(m_pEditBoxMinLevel->GetTextToInt());

	SendPvPCreateRoom( 
		m_SelectedMapIndex , 
		m_SelectedPlayerNum , 
		m_IsInGameJoin , 
		m_nGameModeTableID, 
		m_nWinCondition, 
		m_nPlayTimeSec , 
		BYTE( wszRoomName.length() ) , 
		BYTE( wszRoomPW.length()),
		wszRoomName.c_str() , 
		wszRoomPW.c_str(), 
		m_bDropItem ,
		m_bShowHp,
		m_bRandomTeamMode,
		m_bRevision, 
		m_bRandomOrder,
#ifdef PRE_MOD_PVPOBSERVER
		m_bObserverAccess,
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		cRoomType,
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_WORLDCOMBINE_PVP
		bWorldCombineRoom,
#endif // PRE_WORLDCOMBINE_PVP
		BYTE(m_pEditBoxMinLevel->GetTextToInt()), 
		BYTE(m_pEditBoxMaxLevel->GetTextToInt())
		);

	Show(false);
}

void CDnPVPMakeRoomDlg::MakeMapDropDownList(UINT nVecIndex) // Create Map ComboBox
{
	if( m_vGameModeList.empty() ) 
		return;

	if( nVecIndex < 0 || nVecIndex >= (int)m_vGameModeList.size() ) 
		return;

	if( m_vGameModeList[nVecIndex].nLadderType > 0 )
		return;

	int nMapSize = (int)m_vGameModeList[nVecIndex].vAssignMapDataList.size();
	m_pComboBoxMapSelection->RemoveAllItems();
	
	if(IsEditRoom()) // Edit mode
	{
		int nCurrentSelectedPlayerNum = 0;
		m_pComboBoxPlayerNum->GetSelectedValue(nCurrentSelectedPlayerNum);

		for(int i = 0 ; i < nMapSize ; ++i)
		{
			for(int j = 0 ; j < MapData_PlayerMAX ; ++j)
			{
				if(m_vGameModeList[nVecIndex].vAssignMapDataList[i].nNumOfPlayerOpt[j] == nCurrentSelectedPlayerNum)
				{
					int nMapTableID		 = m_vGameModeList[nVecIndex].vAssignMapDataList[i].nMapTableID;
					int nMapNameStringID = m_vGameModeList[nVecIndex].vAssignMapDataList[i].nMapNameStringID;
					m_pComboBoxMapSelection->AddItem(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nMapNameStringID), NULL, nMapTableID);
				}
			}
		}
		m_pComboBoxMapSelection->SetSelectedByIndex(0);
		return;
	}

	// Normal map settings
	for(int i = 0 ; i < nMapSize ; ++i)
	{
		int nMapTableID		 = m_vGameModeList[nVecIndex].vAssignMapDataList[i].nMapTableID;
		int nMapNameStringID = m_vGameModeList[nVecIndex].vAssignMapDataList[i].nMapNameStringID;
		m_pComboBoxMapSelection->AddItem(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nMapNameStringID), NULL, nMapTableID);
	}
	m_pComboBoxMapSelection->SetSelectedByIndex(0);
}

void CDnPVPMakeRoomDlg::MakeGameModeDropDownList() // Create GameMode ComboBox
{
	if( m_vGameModeList.empty() )
		return;
	
	std::wstring wszModeName;
	m_pComboBoxGameMode->RemoveAllItems();	
	for(int i = 0 ; i < (int)m_vGameModeList.size() ; ++i)
	{
		if(m_vGameModeList[i].nLadderType > 0)
			continue;

		if(m_vGameModeList[i].vAssignMapDataList.empty())
			continue;


#ifndef _FINAL_BUILD // When "NOT" a final build
		if(m_vGameModeList[i].bIsReleaseShow == false)
		{
			wszModeName = L"[ReleaseShow:No] ";
			wszModeName.append( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, m_vGameModeList[i].nGameModeStringID) );
		}
		else
			wszModeName = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, m_vGameModeList[i].nGameModeStringID);
#else
		wszModeName = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, m_vGameModeList[i].nGameModeStringID);
#endif // _FINAL_BUILD 

		m_pComboBoxGameMode->AddItem(wszModeName.c_str(), NULL, m_vGameModeList[i].nTableID);
	}

	m_nGameModeTableID = m_vGameModeList[0].nTableID;
	m_pComboBoxGameMode->SetSelectedByIndex(0);
}

void CDnPVPMakeRoomDlg::MakeGameModeDropDownList( UINT index ) // Create GameMode ComboBox
{
	if(index < 0 || index >= (int)m_vGameModeList.size())
		return;

	if(m_pComboBoxGameMode && IsEditRoom())
	{
		m_pComboBoxGameMode->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_vGameModeList[index].nGameModeStringID ) , NULL, m_vGameModeList[index].nTableID );
	}
}

void CDnPVPMakeRoomDlg::MakeVicConDropDownList() // Create victory condition ComboBox
{
	m_pComboBoxVictoryCondition->RemoveAllItems();

	if( m_nGameModeTableID == 0 )
		return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if ( !pSox ) 
	{
		ErrorLog("CDnPVPMakeRoomDlg::MakeVicConDropDownList:: gamemode table Not found!! ");
		return;
	}

	int nVicCon = 0;
#ifdef PRE_ADD_PVP_TOURNAMENT
	std::wstring wszWinCon;
	DNTableCell* pWinConditionCell = pSox->GetFieldFromLablePtr(m_nGameModeTableID , "WinCondition_UIString");
	if (pWinConditionCell)
	{
		int winConditionStringIndex = pWinConditionCell->GetInteger();
		wszWinCon = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, winConditionStringIndex);
	}

	int nGameMode = pSox->GetFieldFromLablePtr( m_nGameModeTableID , "GamemodeID" )->GetInteger();
	if (nGameMode == PvPCommon::GameMode::PvP_Tournament)
	{
		m_pComboBoxVictoryCondition->AddItem(wszWinCon.c_str(), NULL, 1);
		m_pComboBoxVictoryCondition->SetSelectedByValue(1);
		return;
	}
#else
	WCHAR wszWinCon[256];
	SecureZeroMemory(wszWinCon,sizeof(wszWinCon));
	wsprintf(wszWinCon ,L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( m_nGameModeTableID , "WinCondition_UIString" )->GetInteger()) );
#endif

	WCHAR wszResult[256];
#ifdef PRE_ADD_PVP_TOURNAMENT
	ZeroMemory(wszResult, sizeof(wszResult));
#else
	SecureZeroMemory(wszResult,sizeof(wszResult));
#endif

	for(int i=0; i<5; i++)
	{
		nVicCon = pSox->GetFieldFromLablePtr( m_nGameModeTableID , FormatA("WinCondition_%d",i+1).c_str() )->GetInteger(); 
		if( nVicCon != 0 )
		{
	#ifdef PRE_ADD_PVP_TOURNAMENT
			if (PvPCommon::GameMode::PvP_AllKill == nGameMode)
			{
				m_pComboBoxVictoryCondition->AddItem(wszWinCon.c_str(), NULL,  i+1);
				continue;
			}
	#else
			int nGameMode = pSox->GetFieldFromLablePtr( m_nGameModeTableID , "GamemodeID" )->GetInteger();
			if( PvPCommon::GameMode::PvP_AllKill == nGameMode )
			{
				m_pComboBoxVictoryCondition->AddItem( wszWinCon , NULL,  i+1  );
				continue;
			}
	#endif // PRE_ADD_PVP_TOURNAMENT
#ifdef PRE_ADD_PVP_TOURNAMENT
			wsprintf(wszResult ,L"%d%s",nVicCon , wszWinCon.c_str());
#else
			wsprintf(wszResult ,L"%d%s",nVicCon , wszWinCon );
#endif

#ifdef PRE_ADD_PVP_COMBOEXERCISE
			if( PvPCommon::GameMode::PvP_ComboExercise == nGameMode )
				m_pComboBoxVictoryCondition->AddItem( wszWinCon.c_str(), NULL,  i+1  );
			else
				m_pComboBoxVictoryCondition->AddItem( wszResult , NULL,  i+1  );
#else
			m_pComboBoxVictoryCondition->AddItem( wszResult , NULL,  i+1  );
#endif // PRE_ADD_PVP_COMBOEXERCISE
		}
	}
	
	int DefaultIndex = 0;
	DefaultIndex = pSox->GetFieldFromLablePtr( m_nGameModeTableID , "DefaultGameWinType" )->GetInteger();
	m_pComboBoxVictoryCondition->SetSelectedByValue(DefaultIndex);
}

void CDnPVPMakeRoomDlg::MakeDropPlayDownList( UINT index ) // Create player count ComboBox
{
	if( m_vGameModeList.empty() )
		return;

	if(!m_pComboBoxPlayerNum->IsEnable())
		return;

	if( m_nCurrSelectModeVectorIndex < 0 || m_nCurrSelectModeVectorIndex >= (int)m_vGameModeList.size() )
		return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if ( !pSox ) 
	{
		ErrorLog("CDnPVPMakeRoomDlg::MakeGameModeDropDownList():: gamemode table Not found!! ");
		return;
	}

	int nMinPlayerNumber = 0;
	int nMaxPlayerNumber = 0;
	
	int DefaultIndex	 = 0;
	WCHAR PlayNum[5][10];

	int nListIdx	  = m_nCurrSelectModeVectorIndex;
	int nModeTableNum = m_vGameModeList[nListIdx].nTableID;
	
	nMinPlayerNumber = pSox->GetFieldFromLablePtr( nModeTableNum , "NumOfPlayers_Min" )->GetInteger();
	nMaxPlayerNumber = pSox->GetFieldFromLablePtr( nModeTableNum , "NumOfPlayers_Max" )->GetInteger();

	if(nMaxPlayerNumber == 0)
		nMaxPlayerNumber = 100;

	m_pComboBoxPlayerNum->RemoveAllItems();

	for(int j=0;j < MapData_PlayerMAX ;j++)
	{
		if(	m_vGameModeList[nListIdx].vAssignMapDataList[index].nNumOfPlayerOpt[j] != 0					&&
			m_vGameModeList[nListIdx].vAssignMapDataList[index].nNumOfPlayerOpt[j] >= nMinPlayerNumber  &&
			m_vGameModeList[nListIdx].vAssignMapDataList[index].nNumOfPlayerOpt[j] <= nMaxPlayerNumber )
		{
			int nPlayerNum = m_vGameModeList[nListIdx].vAssignMapDataList[index].nNumOfPlayerOpt[j];
			wsprintf( PlayNum[j] , L"%d", nPlayerNum );
			m_pComboBoxPlayerNum->AddItem( PlayNum[j] , NULL,  nPlayerNum );

			if( nPlayerNum == m_vGameModeList[nListIdx].vAssignMapDataList[index].nDefaultNumOfPlayerOpt )
				DefaultIndex = j;
		}
	}

	m_pComboBoxPlayerNum->SetSelectedByIndex(DefaultIndex);
	m_pComboBoxPlayerNum->GetSelectedValue(m_SelectedPlayerNum);
}

void CDnPVPMakeRoomDlg::SetMapAndModeDropDownList( UINT nCurMapIndex, UINT nCurModeIndex, LPCWSTR wszItemName ) // Set edit mode map & mode
{
	if(nCurMapIndex <= 0)
		return;

	if(m_vGameModeList.empty())
		return;

	//---------------------------------------
	// 1. Set Mode.
	m_pComboBoxGameMode->RemoveAllItems();
	for( int i = 0; i < (int)m_vGameModeList.size(); ++i ) 
	{
		if(m_vGameModeList[i].nLadderType > 0)
			continue;

		if(m_vGameModeList[i].vAssignMapDataList.empty())
			continue;


		int nSelectedPlayerNum = 0;
		m_pComboBoxPlayerNum->GetSelectedValue(nSelectedPlayerNum);
		
		if(m_vGameModeList[i].nNumOfPlayers_Min == 1 && m_vGameModeList[i].nNumOfPlayers_Max == 100)
		{
		}
		else
		{
			if(m_vGameModeList[i].nTableID != nCurModeIndex) // During editing, the current game mode must not be skipped.
			{
				if(m_vGameModeList[i].nNumOfPlayers_Min < nSelectedPlayerNum)
					continue;

				if(m_vGameModeList[i].nNumOfPlayers_Max > nSelectedPlayerNum)
					continue;
			}
		}

		bool bIsEnd = false;
		for(int j = 0 ; j < (int)m_vGameModeList[i].vAssignMapDataList.size() ; ++j)
		{
			for(int k = 0 ; k < MapData_PlayerMAX ; ++k)
			{
				if(m_vGameModeList[i].vAssignMapDataList[j].nNumOfPlayerOpt[k] == nSelectedPlayerNum)
				{
					MakeGameModeDropDownList(i);
					bIsEnd = true;
					break;
				}
			}
			if(bIsEnd) break;
		}
	}
	int nIndex = m_pComboBoxGameMode->FindItemByValue(nCurModeIndex);
	if( nIndex)
		m_pComboBoxGameMode->SetSelectedByIndex(nIndex);

	//---------------------------------------
	// 2. Set Map.
	int nVecIdx = -1;
	for(int i = 0 ; i < (int)m_vGameModeList.size() ; ++i)
	{
		if(m_vGameModeList[i].nTableID == nCurModeIndex)
		{
			nVecIdx = i;
			break;
		}
	}
	if(nVecIdx != -1)
	{
		MakeMapDropDownList(nVecIdx);
		int nIndex = m_pComboBoxMapSelection->FindItemByValue(nCurMapIndex);
		if( nIndex)
			m_pComboBoxMapSelection->SetSelectedByIndex(nIndex);
	}
}

void CDnPVPMakeRoomDlg::SetInGameJoin( bool IsJoin  )
{
	if( m_pCheckBoxInGameJoin )
		m_pCheckBoxInGameJoin->SetChecked( IsJoin );
}

void CDnPVPMakeRoomDlg::SetDropItem( bool bDropItem )
{
	if( m_pCheckBoxDropItem )
		m_pCheckBoxDropItem->SetChecked( bDropItem );
}

void CDnPVPMakeRoomDlg::SetShowHp( bool bDropItem ) 
{
	if( m_pCheckBoxShowHP )
		m_pCheckBoxShowHP->SetChecked( bDropItem );
}

void CDnPVPMakeRoomDlg::SetRevision( bool bRevision )
{
	if( m_pCheckBoxRevision )
		m_pCheckBoxRevision->SetChecked( !bRevision );
}

void CDnPVPMakeRoomDlg::SetRandomOrder( bool bRandomOrder )
{
	if( m_pCheckBoxRandomOrder )
		m_pCheckBoxRandomOrder->SetChecked(bRandomOrder);
}

void CDnPVPMakeRoomDlg::SetRandomTeamMode( bool bRandomTeamMode ) 
{
	if( m_pCheckBoxRandomTeamMode )
		m_pCheckBoxRandomTeamMode->SetChecked( bRandomTeamMode );
}

void CDnPVPMakeRoomDlg::SetGameWinObjectNum( LPCWSTR wszItemName  )
{
	if( m_pComboBoxVictoryCondition )
	{
		int iIndex = m_pComboBoxVictoryCondition->FindItem( wszItemName );

		if( -1 != iIndex )
			m_pComboBoxVictoryCondition->SetSelectedByIndex(iIndex);
	}
}

void CDnPVPMakeRoomDlg::SetPlayerLevel(int nLevelMin, int nLevelMax)
{	
	m_pEditBoxMinLevel->SetIntToText(nLevelMin);
	m_pEditBoxMaxLevel->SetIntToText(nLevelMax);
}

#ifdef PRE_MOD_PVPOBSERVER
void CDnPVPMakeRoomDlg::SetObserver( bool b )
{
	if( m_pCheckBoxObserver )
		m_pCheckBoxObserver->SetChecked( b );
}
#endif // PRE_MOD_PVPOBSERVER

void CDnPVPMakeRoomDlg::SetMaxPlayerDownList( byte cMaxUserNum )
{
	if( cMaxUserNum <= 0 )
		return;

	int iIndex = 0;
	WCHAR wszMaxPlayerNum[10];
#ifdef PRE_ADD_PVP_TOURNAMENT
	ZeroMemory(wszMaxPlayerNum, sizeof(wszMaxPlayerNum));
#else
	SecureZeroMemory(wszMaxPlayerNum,sizeof(wszMaxPlayerNum));
#endif
	wsprintf(wszMaxPlayerNum,L"%d",cMaxUserNum);

	if( m_pComboBoxPlayerNum )
	{
		m_pComboBoxPlayerNum->RemoveAllItems();
		m_pComboBoxPlayerNum->AddItem( wszMaxPlayerNum, NULL, cMaxUserNum );
		m_pComboBoxPlayerNum->Enable(false);
	}
}

void CDnPVPMakeRoomDlg::GetGamemodeInfo( int nIndex )
{
	if( nIndex <= 0 && m_nGameModeTableID > 0)
		return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if ( !pSox ) 
	{
		ErrorLog("CDnPVPMakeRoomDlg::GetGamemodeInfo:: gamemode table Not found!! ");
		return;
	}

	char szConField[20];
	char szTimeField[20];

	SecureZeroMemory(szConField,sizeof(szConField));
	SecureZeroMemory(szTimeField,sizeof(szTimeField));	

	sprintf(szConField ,"WinCondition_%d",nIndex);
	sprintf(szTimeField,"PlayTime_%d",nIndex);

	m_nWinCondition = pSox->GetFieldFromLablePtr( m_nGameModeTableID ,szConField )->GetInteger();
	m_nPlayTimeSec = pSox->GetFieldFromLablePtr( m_nGameModeTableID ,szTimeField )->GetInteger();
}

#ifdef PRE_ADD_PVP_TOURNAMENT
int CDnPVPMakeRoomDlg::GetGameModeData(int nGameModeTableID, const char* pLableString) const
{
	if (nGameModeTableID <= 0 || pLableString == NULL || pLableString == '\0')
		return -1;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if (!pSox)
	{
		ErrorLog("CDnPVPMakeRoomDlg::GetGamemodeInfo:: gamemode table Not found!! ");
		return -1;
	}

	DNTableCell* pCell = pSox->GetFieldFromLablePtr(nGameModeTableID, pLableString);
	if (pCell == NULL)
		return -1;

	return pCell->GetInteger();
}
#endif

void CDnPVPMakeRoomDlg::MakeGameModeImage( UINT index )
{
	if(index < 0 || index >= (int)m_vGameModeList.size() ) 
		return;

	SAFE_RELEASE_SPTR(m_hGameModeImage);

	if( !m_vGameModeList[index].strModeImageName.empty() )
	{
		m_hGameModeImage = LoadResource(CEtResourceMng::GetInstance().GetFullName(m_vGameModeList[index].strModeImageName.c_str()).c_str(), RT_TEXTURE);
		if(m_hGameModeImage)
			m_pGameModeImage->SetTexture( m_hGameModeImage, 0, 0, m_hGameModeImage->Width(), m_hGameModeImage->Height() );
	}
}

void CDnPVPMakeRoomDlg::MakeMapImage( UINT index )
{
	int nCurModeIdx = m_nCurrSelectModeVectorIndex;
	if( nCurModeIdx >= (int)m_vGameModeList.size() || nCurModeIdx < 0)
		return;

	SAFE_RELEASE_SPTR( m_hMiniMapImage );

	if( m_vGameModeList[nCurModeIdx].vAssignMapDataList[index].strMapImageName.c_str() )
		m_hMiniMapImage = LoadResource( CEtResourceMng::GetInstance().GetFullName(m_vGameModeList[nCurModeIdx].vAssignMapDataList[index].strMapImageName.c_str()).c_str(), 
										RT_TEXTURE );
	if(m_hMiniMapImage)
		m_pMiniMap->SetTexture( m_hMiniMapImage, 0, 0, m_hMiniMapImage->Width() , m_hMiniMapImage->Height() );	
}

const wchar_t* CDnPVPMakeRoomDlg::MakeDefaultRoomName()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGAMEROOMNAME );
	if ( pSox ) 
	{
		std::vector<int> vUISting;
		for( int i= 1; i< pSox->GetItemCount()+1; i++ ) 
		{
			int nUISting = 0;
			nUISting = pSox->GetFieldFromLablePtr( i, "RoomNameUIstring" )->GetInteger();
			if( nUISting )
				vUISting.push_back( nUISting );
			else
				break;
		}

		if( vUISting.size() > 0 )
			return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, vUISting[_rand()%vUISting.size()] );
	}
	return L"";
}

// Shuffle PvP map and PvP game mode.
void CDnPVPMakeRoomDlg::Shuffle()
{

	if( m_pComboBoxGameMode )
	{
		m_pComboBoxGameMode->SetSelectedByIndex( _rand()%m_pComboBoxGameMode->GetItemCount() );
		ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, m_pComboBoxGameMode, 0 );
	}

	if( m_pComboBoxMapSelection )
	{
		m_pComboBoxMapSelection->SetSelectedByIndex( _rand()%m_pComboBoxMapSelection->GetItemCount() );
		ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, m_pComboBoxMapSelection, 0 );
	}
}

void CDnPVPMakeRoomDlg::InitCheckBox()
{
	bool bShowHp = false;
	bool bDropItem = m_pCheckBoxDropItem->IsEnable() ? true : false;
	bool bIngameJoin = m_pCheckBoxInGameJoin->IsEnable() ? true : false;
	bool bRandomTeamMode = false;
	bool bRevision = false;
	bool bRandomOrder = false;

	DNTableFileFormat* pSoxMode = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if(pSoxMode)
	{
		int nItemValue = 0;
		int nGameMode = 0;
		m_pComboBoxGameMode->GetSelectedValue(nItemValue);

		nGameMode = pSoxMode->GetFieldFromLablePtr( nItemValue, "GamemodeID" )->GetInteger();
		if( nGameMode == PvPCommon::GameMode::PvP_GuildWar)
			bIngameJoin = true;
#ifdef PRE_ADD_PVP_TOURNAMENT
		if (nGameMode == PvPCommon::GameMode::PvP_Tournament)
			bIngameJoin = false;
#endif
#ifdef PRE_ADD_PVP_COMBOEXERCISE
		if (nGameMode == PvPCommon::GameMode::PvP_ComboExercise)
			bDropItem = true;
#endif // PRE_ADD_PVP_COMBOEXERCISE

		bRevision = pSoxMode->GetFieldFromLablePtr( nItemValue, "_DefaultRevision" )->GetInteger() ? true : false;
	}

	m_pCheckBoxShowHP->SetChecked(bShowHp);
	m_pCheckBoxDropItem->SetChecked(bDropItem);
	m_pCheckBoxInGameJoin->SetChecked(bIngameJoin);
	m_pCheckBoxRandomTeamMode->SetChecked(bRandomTeamMode);
	m_pCheckBoxRevision->SetChecked(bRevision);
	m_pCheckBoxRandomOrder->SetChecked(bRandomOrder);
}

void CDnPVPMakeRoomDlg::SetGameModeButtonState()
{
	int  nItemValue = 0;
	bool bDisableShowHpMode = false;
	bool bDisalbeRandomTeamMode = false;
	bool bDisableRevisionMode = false;
	bool bCheckRevisionMode = true;
	bool bDisableUseItem = false;
	bool bDisableRandomOrder = true;
	bool bDisableVictoryCondition = false;
	bool bDisableBreakInto = false;
#ifdef PRE_ADD_PVP_TOURNAMENT
	bool bDisableSetPlayerNumber = IsEditRoom();
#endif

	if(!m_vGameModeList.empty() && m_nCurrSelectModeVectorIndex < (int)m_vGameModeList.size() && m_SelectedMapIndex >= 0)
	{
		for(int i = 0 ; i < (int)m_vGameModeList[m_nCurrSelectModeVectorIndex].vAssignMapDataList.size() ; ++i)
		{
			if(m_vGameModeList[m_nCurrSelectModeVectorIndex].vAssignMapDataList[i].nMapTableID == m_SelectedMapIndex )
			{
				bool IsInGameJoin = m_vGameModeList[m_nCurrSelectModeVectorIndex].vAssignMapDataList[i].bIsAllowBreak;
				m_pCheckBoxInGameJoin->SetChecked(IsInGameJoin);
				bDisableBreakInto = !IsInGameJoin;
			}
		}
	}

	m_pComboBoxGameMode->GetSelectedValue(nItemValue);
	DNTableFileFormat* pSoxMode = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if(pSoxMode)
	{
		int nGameMode = pSoxMode->GetFieldFromLablePtr( nItemValue, "GamemodeID" )->GetInteger() ;

		switch(nGameMode)
		{
		case PvPCommon::GameMode::PvP_Captain:
			{
				bDisableShowHpMode = true;
			}
			break;
		case PvPCommon::GameMode::PvP_IndividualRespawn:
			{
				bDisalbeRandomTeamMode = true;
			}
			break;
		case PvPCommon::GameMode::PvP_Zombie_Survival:
			{
				bDisalbeRandomTeamMode = true;
			}
			break;
		case PvPCommon::GameMode::PvP_GuildWar:
			{
				bDisableBreakInto = true;
				bDisableShowHpMode = true;
				bDisalbeRandomTeamMode = true;
				bDisableVictoryCondition = true;
			}
			break;
		case PvPCommon::GameMode::PvP_AllKill:
			{
				bDisableBreakInto = true;
				bDisableUseItem = true;
				bDisableRandomOrder = false;
				bDisableVictoryCondition = true;
			}
			break;

#if defined( PRE_ADD_RACING_MODE )
		case PvPCommon::GameMode::PvP_Racing:
			{
				bDisableBreakInto = true;
				bDisableUseItem = true;
				bDisableShowHpMode = true;
				bDisalbeRandomTeamMode = true;
				bDisableVictoryCondition = true;
			}
			break;
#endif	// #if defined( PRE_ADD_RACING_MODE )

#ifdef PRE_ADD_PVP_TOURNAMENT
		case PvPCommon::GameMode::PvP_Tournament:
			{
				bDisableBreakInto = true;
				bDisableUseItem = true;
				int maxPlayerNum = GetGameModeData(nItemValue, "NumOfPlayers_Max");
				if (maxPlayerNum > 0)
					SetMaxPlayerDownList(maxPlayerNum);
				bDisableSetPlayerNumber = true;
				bDisableVictoryCondition = true;
			}
			break;
#endif

#ifdef PRE_ADD_PVP_COMBOEXERCISE
		case PvPCommon::GameMode::PvP_ComboExercise:
			{
				bDisableShowHpMode = bDisableRandomOrder = bDisalbeRandomTeamMode = bDisableUseItem = bDisableRevisionMode = bDisableVictoryCondition = true;
			}
			break;
#endif // PRE_ADD_PVP_COMBOEXERCISE
		}

		bDisableRevisionMode = pSoxMode->GetFieldFromLablePtr( nItemValue, "_SelectableRevision" )->GetInteger() ? false : true;
		bCheckRevisionMode = pSoxMode->GetFieldFromLablePtr( nItemValue, "_DefaultRevision" )->GetInteger() ? true : false;
	}

	m_pCheckBoxDropItem->Enable( !bDisableUseItem );
	m_pCheckBoxInGameJoin->Enable( !bDisableBreakInto );
	m_pCheckBoxShowHP->Enable(!bDisableShowHpMode);
	m_pCheckBoxRandomTeamMode->Enable(!bDisalbeRandomTeamMode);
	m_pComboBoxVictoryCondition->Enable( !bDisableVictoryCondition );
	m_pCheckBoxRevision->Enable( !bDisableRevisionMode );
	m_pCheckBoxRevision->SetChecked( bCheckRevisionMode );
#ifdef PRE_ADD_PVP_TOURNAMENT
	m_pComboBoxPlayerNum->Enable(!bDisableSetPlayerNumber);
#endif
	m_pCheckBoxRandomOrder->Enable( !bDisableRandomOrder && !m_bRandomTeamMode );
	m_pCheckBoxRandomOrder->Show( !bDisableRandomOrder );
	m_pStaticRandomOrder->Show( !bDisableRandomOrder );
}




#else // PRE_MOD_PVP_ROOM_CREATE



CDnPVPMakeRoomDlg::CDnPVPMakeRoomDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pEditBoxRoomName(NULL)
, m_pEditBoxMinLevel(NULL)
, m_pEditBoxMaxLevel(NULL)
, m_pComboBoxMapSelection(NULL)
, m_pComboBoxPlayerNum(NULL)
, m_pCheckBoxInGameJoin(NULL)
, m_pComboBoxGameMode(NULL)
, m_pComboBoxVictoryCondition(NULL)
, m_pExitButton(NULL)
, m_pMiniMap(NULL)
, m_pCheckBoxDropItem(NULL)
, m_pCheckBoxRevision(NULL)
, m_pCheckBoxRandomTeamMode(NULL)
, m_pCheckBoxRandomOrder(NULL)
, m_pStaticRandomOrder(NULL)
#ifdef PRE_MOD_PVPOBSERVER
, m_pCheckBoxObserver(NULL)
#endif // PRE_MOD_PVPOBSERVER
, m_pCheckBoxShowHP(NULL)
, m_pEditBoxPassWord(NULL)
#ifdef PRE_WORLDCOMBINE_PVP
, m_pStaticWorldCombineRoom( NULL )
, m_pCheckBoxWorldCombineRoom( NULL )
#endif // PRE_WORLDCOMBINE_PVP
{
	m_SelectedIndex = -1;
	m_SelectedPlayerNum = -1;
	m_SelectedMapIndex = -1;
	m_nGameModeTableID = 0;
	m_nWinCondition = 0;
	m_nPlayTimeSec = 0;

	m_IsInGameJoin = false;
	m_IsFirstInputRoomName = true;
	m_bDropItem = false;
	m_bShowHp = false;
	m_bRandomTeamMode = false;
	m_bRevision = false;
	m_bRandomOrder = false;

#ifdef PRE_MOD_PVPOBSERVER
	m_bObserverAccess = true;
#endif // PRE_MOD_PVPOBSERVER

}

CDnPVPMakeRoomDlg::~CDnPVPMakeRoomDlg(void)
{
	SAFE_RELEASE_SPTR( m_hMiniMapImage );
}

void CDnPVPMakeRoomDlg::Initialize( bool bShow )
{
	MakePvPMapData();
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpMakeRoom.ui" ).c_str(), bShow );
}

void CDnPVPMakeRoomDlg::InitialUpdate()
{
	m_pEditBoxMinLevel = GetControl<CEtUIEditBox>( "ID_EDITBOX_LV_MIN" );	
	m_pEditBoxMaxLevel = GetControl<CEtUIEditBox>( "ID_EDITBOX_LV_MAX" );
	m_pEditBoxRoomName = GetControl<CEtUIEditBox>( "ID_PVP_MAKEROOM_INPUTNAME" );
	m_pEditBoxPassWord = GetControl<CEtUIEditBox>( "ID_PVP_MAKEROOM_PASSWORD" );

	m_pComboBoxGameMode = GetControl<CEtUIComboBox>( "ID_PVP_MAKEROOM_INPUTMODE" );	
	m_pComboBoxPlayerNum = GetControl<CEtUIComboBox>( "ID_PVP_MAKEROOM_INPUTNUM" );
	m_pComboBoxMapSelection = GetControl<CEtUIComboBox>( "ID_PVP_MAKEROOM_INPUTMAP" );	
	m_pComboBoxVictoryCondition = GetControl<CEtUIComboBox>( "ID_PVP_MAKEROOM_WINCONDITION" );

	m_pCheckBoxShowHP = GetControl<CEtUICheckBox>("ID_CHECKBOX_HP"); 
	m_pCheckBoxRevision = GetControl<CEtUICheckBox>("ID_CHECKBOX_FIT");
	m_pCheckBoxDropItem = GetControl<CEtUICheckBox>("ID_CHECKBOX_ITEM");
	m_pCheckBoxInGameJoin = GetControl<CEtUICheckBox>("ID_PVP_MAKEROOM_INPUTINTRUTION");
	m_pCheckBoxRandomTeamMode = GetControl<CEtUICheckBox>("ID_CHECKBOX_RANDOM");

	m_pMiniMap = GetControl<CEtUITextureControl>( "ID_PVP_MAPIMG" );
	m_pExitButton = GetControl<CEtUIButton>( "ID_BUTTON_CLOSE" );

	m_bRandomTeamMode = false;
	m_bRevision = false;

	m_pCheckBoxRandomOrder = GetControl<CEtUICheckBox>("ID_CHECKBOX_RANDOMORDER");
	m_pStaticRandomOrder = GetControl<CEtUIStatic>("ID_STATIC19");
	m_bRandomOrder = false;

#ifdef PRE_MOD_PVPOBSERVER
	m_pCheckBoxObserver = GetControl<CEtUICheckBox>("ID_CHECKBOX_OBSERVER");
	m_pCheckBoxObserver->SetChecked( m_bObserverAccess );
#endif // PRE_MOD_PVPOBSERVER

#ifdef PRE_WORLDCOMBINE_PVP
	m_pStaticWorldCombineRoom = GetControl<CEtUIStatic>( "ID_STATIC21" );
	m_pStaticWorldCombineRoom->Show( false );
	m_pCheckBoxWorldCombineRoom = GetControl<CEtUICheckBox>( "ID_CHECKBOX_WORLDCOMBINEROOM" );
	m_pCheckBoxWorldCombineRoom->Show( false );
#endif // PRE_WORLDCOMBINE_PVP

	MakeDropDownList();
	MakeDropPlayDownList(0);
	MakeGameModeDropDownList(0);
	MakeVicConDropDownList();

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_PVP_MAKEROOM_CREATE") );
}

void CDnPVPMakeRoomDlg::Show( bool bShow )
{
	if( bShow )
	{
		m_pEditBoxRoomName->ClearText();
		m_pEditBoxPassWord->ClearText();
		m_pEditBoxMinLevel->SetText(L"1");
		m_pEditBoxMaxLevel->SetText(L"100");
		if( !IsEditRoom() )
			m_pEditBoxRoomName->SetText(MakeDefaultRoomName());

		m_IsFirstInputRoomName = true;
		m_SmartMove.MoveCursor();

#ifdef PRE_WORLDCOMBINE_PVP
		bool bShowWorldCombineRoom = false;
		CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
		if( pPVPLobbyTask )
		{
			//if( AccountLevel_New <= pPVPLobbyTask->GetUserInfo().cAccountLevel )
			if( AccountLevel_New <= pPVPLobbyTask->GetUserInfo().cAccountLevel && AccountLevel_DWC > pPVPLobbyTask->GetUserInfo().cAccountLevel )
				bShowWorldCombineRoom = true;
		}
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
		else if( CDnActor::s_hLocalActor )
		{
			CDnLocalPlayerActor* pLocalPlayer = static_cast<CDnLocalPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
			if( pLocalPlayer && pLocalPlayer->IsDeveloperAccountLevel() )
				bShowWorldCombineRoom = true;
		}
#endif // PRE_ADD_PVP_VILLAGE_ACCESS
		if( m_pStaticWorldCombineRoom )
			m_pStaticWorldCombineRoom->Show( bShowWorldCombineRoom );

		if( m_pCheckBoxWorldCombineRoom )
			m_pCheckBoxWorldCombineRoom->Show( bShowWorldCombineRoom );
#endif // PRE_WORLDCOMBINE_PVP
	}
	else
	{
		m_SmartMove.ReturnCursor();
	}
	CEtUIDialog::Show( bShow );
}

void CDnPVPMakeRoomDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_PVP_MAKEROOM_CREATE" ) )
		{
			SendMakeRoom();
			return;
		}

		if( IsCmdControl("ID_PVP_MAKEROOM_CANCEL" ) )
		{
			Show(false);
			return;
		}

		if( IsCmdControl( "ID_BUTTON_CLOSE") )
		{
			Show(false);
			return;
		}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED )
	{
		if( IsCmdControl("ID_PVP_MAKEROOM_INPUTMAP" ) )
		{
			int nCurMapIndex = 0;
			if( m_pComboBoxMapSelection->GetSelectedValue( nCurMapIndex ) ) {
				for( int i = 0; i < (int)vMapData.size(); ++i ) {
					if( vMapData[i].nMapIndex == nCurMapIndex ) {
						MakeGameModeDropDownList(i);
						MakeMapImage(i);
						m_pComboBoxMapSelection->GetSelectedValue(m_SelectedMapIndex);
						break;
					}
				}
			}
			SetGameModeButtonState();
			return;
		}

		if( IsCmdControl("ID_PVP_MAKEROOM_INPUTNUM" ) )
		{
			m_pComboBoxPlayerNum->GetSelectedValue(m_SelectedPlayerNum);				
			return;
		}

		if( IsCmdControl("ID_PVP_MAKEROOM_WINCONDITION" ) )
		{
			int nIndex;
			m_pComboBoxVictoryCondition->GetSelectedValue(nIndex);
			GetGamemodeInfo( nIndex );
			return;
		}

		if( IsCmdControl("ID_PVP_MAKEROOM_INPUTMODE" ) )
		{
			m_pComboBoxGameMode->GetSelectedValue(m_nGameModeTableID);
			m_SelectedModeIndex = m_pComboBoxGameMode->GetSelectedIndex();

			// DropDownList
			int nCurMapIndex = 0;
			if( m_pComboBoxMapSelection->GetSelectedValue( nCurMapIndex ) ) {
				for( int i = 0; i < (int)vMapData.size(); ++i ) {
					if( vMapData[i].nMapIndex == nCurMapIndex ) {
						MakeDropPlayDownList(i);
					}
				}
			}

			MakeVicConDropDownList();
			SetGameModeButtonState();
			InitCheckBox();
			return;

		}
	}else if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if( IsCmdControl("ID_PVP_MAKEROOM_INPUTINTRUTION" ) ) 
		{
			if( m_pCheckBoxInGameJoin )
				m_IsInGameJoin = m_pCheckBoxInGameJoin->IsChecked();
		}

		if( IsCmdControl( "ID_CHECKBOX_ITEM" ) )
		{
			if( m_pCheckBoxDropItem )
				m_bDropItem = m_pCheckBoxDropItem->IsChecked();
		}

		if( IsCmdControl("ID_CHECKBOX_HP") )   
		{
			if(m_pCheckBoxShowHP)
				m_bShowHp = m_pCheckBoxShowHP->IsChecked();
		}

		if( IsCmdControl("ID_CHECKBOX_RANDOM") )   
		{
			if(m_pCheckBoxRandomTeamMode)
				m_bRandomTeamMode = m_pCheckBoxRandomTeamMode->IsChecked();

			m_pCheckBoxRandomOrder->SetChecked(m_bRandomTeamMode);
			m_pCheckBoxRandomOrder->Enable(!m_bRandomTeamMode);
		}

		if( IsCmdControl("ID_CHECKBOX_FIT") )   
		{
			if( m_pCheckBoxRevision )
				m_bRevision = !m_pCheckBoxRevision->IsChecked();
		}

		if( IsCmdControl("ID_CHECKBOX_RANDOMORDER") )   
		{
			if( m_pCheckBoxRandomOrder )
				m_bRandomOrder = m_pCheckBoxRandomOrder->IsChecked();
		}
#ifdef PRE_MOD_PVPOBSERVER
		if( IsCmdControl( "ID_CHECKBOX_OBSERVER" ) )
		{
			if( m_pCheckBoxObserver )
				m_bObserverAccess = m_pCheckBoxObserver->IsChecked();
		}		
#endif // PRE_MOD_PVPOBSERVER


	}
	else if( nCommand == EVENT_EDITBOX_FOCUS )
	{
		if( IsCmdControl("ID_PVP_MAKEROOM_INPUTNAME" ) ) 
		{
			if(m_IsFirstInputRoomName)
			{
				m_pEditBoxRoomName->ClearText();
				m_IsFirstInputRoomName = false;
			}
		}
	}
	else if( nCommand == EVENT_EDITBOX_RELEASEFOCUS)
	{
		if(IsCmdControl("ID_EDITBOX_LV_MIN"))
		{
			if(m_pEditBoxMinLevel->GetTextToInt() > 100 || m_pEditBoxMinLevel->GetTextToInt() < 1)
			{
				m_pEditBoxMinLevel->SetIntToText(1);
				m_pEditBoxMaxLevel->SetIntToText(100);
			}
		}

		if(IsCmdControl("ID_EDITBOX_LV_MAX"))
		{
			if(m_pEditBoxMinLevel->GetTextToInt() >  m_pEditBoxMaxLevel->GetTextToInt())
				m_pEditBoxMaxLevel->SetIntToText(m_pEditBoxMinLevel->GetTextToInt());

			if(m_pEditBoxMaxLevel->GetTextToInt() > 100 || m_pEditBoxMaxLevel->GetTextToInt() < 1)
				m_pEditBoxMaxLevel->SetIntToText(100);
		}
	}
}

void CDnPVPMakeRoomDlg::SendMakeRoom()
{
	std::wstring wszRoomName;

	wszRoomName = m_pEditBoxRoomName->GetText();

	std::wstring strTemp = boost::algorithm::trim_copy(wszRoomName);
	if (strTemp.empty())
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPMakeGame::RoomNameIsNULL) , MB_OK);	
		return;
	}

	if( DN_INTERFACE::UTIL::CheckChat( wszRoomName ) )//Handle forbidden word usage
	{
		if( m_pEditBoxRoomName )
			m_pEditBoxRoomName->ClearText();

		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPMakeGame::RoomNameIsWrong) , MB_OK);	
		return;
	}

	std::wstring  wszRoomPW;
	wszRoomPW = m_pEditBoxPassWord->GetText();

#ifdef PRE_WORLDCOMBINE_PVP
	bool bWorldCombineRoom = m_pCheckBoxWorldCombineRoom->IsChecked();
	if( bWorldCombineRoom && wszRoomPW.length() == 0 )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3507 ) , MB_OK );
		return;
	}
#endif // PRE_WORLDCOMBINE_PVP

	GetInterface().SetPVPRoomPassword(wszRoomPW);

	if( m_pEditBoxPassWord->GetTextLength() >= 1 &&  m_pEditBoxPassWord->GetTextLength() <= 3 )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPMakeGame::PasswordShort ) , MB_OK );
		return;
	}
	int nPlayerLevel = 0;

	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
	if(pPVPLobbyTask)
		nPlayerLevel = pPVPLobbyTask->GetUserInfo().cLevel;
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	else if(CDnActor::s_hLocalActor)
		nPlayerLevel = CDnActor::s_hLocalActor->GetLevel();
#endif
	else
		return;

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	char cRoomType = 0;
	CDnPVPLadderTabDlg* pPVPLadderTabDlg = GetInterface().GetPVPLadderTabDlg();
	if( pPVPLadderTabDlg )
	{
		cRoomType = static_cast<char>(pPVPLadderTabDlg->GetSelectedGradeChannel());
	}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER


	if(m_pEditBoxMinLevel->GetTextToInt() > nPlayerLevel)
		m_pEditBoxMinLevel->SetIntToText(nPlayerLevel);

	if(m_pEditBoxMaxLevel->GetTextToInt() < nPlayerLevel)
		m_pEditBoxMaxLevel->SetIntToText(nPlayerLevel);

	if(m_pEditBoxMinLevel->GetTextToInt() > m_pEditBoxMaxLevel->GetTextToInt())
		m_pEditBoxMaxLevel->SetIntToText(m_pEditBoxMinLevel->GetTextToInt());

	SendPvPCreateRoom( 
		m_SelectedMapIndex , 
		m_SelectedPlayerNum , 
		m_IsInGameJoin , 
		m_nGameModeTableID, 
		m_nWinCondition, 
		m_nPlayTimeSec , 
		BYTE( wszRoomName.length() ) , 
		BYTE( wszRoomPW.length()),
		wszRoomName.c_str() , 
		wszRoomPW.c_str(), 
		m_bDropItem ,
		m_bShowHp,
		m_bRandomTeamMode,
		m_bRevision, 
		m_bRandomOrder,
#ifdef PRE_MOD_PVPOBSERVER
		m_bObserverAccess,
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		cRoomType,
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_WORLDCOMBINE_PVP
		bWorldCombineRoom,
#endif // PRE_WORLDCOMBINE_PVP
		BYTE(m_pEditBoxMinLevel->GetTextToInt()), 
		BYTE(m_pEditBoxMaxLevel->GetTextToInt())
		);

	Show(false);
}

void CDnPVPMakeRoomDlg::MakePvPMapData()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPMAP );
	if ( !pSox ) 
	{
		ErrorLog("void CDnPVPMakeRoomDlg::MakePvPMapData():: PVP Map table Not found!! ");
		return;
	}

	vMapData.clear();

	int nItemIndex= 0;

	for( int i=0; i<pSox->GetItemCount() ; i++ ) 
	{
		sPvpMapData TempData;

		nItemIndex = pSox->GetItemID(i);

		if ( pSox->GetFieldFromLablePtr( nItemIndex, "MapTableID" ) == NULL )
			continue;

		if( pSox->GetFieldFromLablePtr( nItemIndex, "ReleaseShow" )->GetInteger() == 0 ) continue;
		TempData.nMapIndex = pSox->GetFieldFromLablePtr( nItemIndex, "MapTableID" )->GetInteger();

		if( TempData.nMapIndex == 0 )
			continue;

		TempData.nUIStringIndex = pSox->GetFieldFromLablePtr( nItemIndex, "MapNameUIstring" )->GetInteger();


		for(int x=0; x<MapData_PlayerMAX; x++)
			TempData.bPlayerNum[x] = pSox->GetFieldFromLablePtr( nItemIndex, FormatA("NumOfPlayersOption%d",x+1).c_str() )->GetInteger();

		for(int y=0; y<MapData_ModeMAX; y++)
			TempData.nGameMode[y] = pSox->GetFieldFromLablePtr( nItemIndex,  FormatA("GameModeTableID_%d",y+1).c_str() )->GetInteger();


		TempData.bRecommandedPlayerNum = pSox->GetFieldFromLablePtr( nItemIndex, "DefaultNumOfPlayers")->GetInteger();
		TempData.isInGameJoin = ( pSox->GetFieldFromLablePtr( nItemIndex, "Allow_Breakin_PlayingGame" )->GetInteger() == TRUE )? true : false;
		TempData.szMapImage = pSox->GetFieldFromLablePtr( nItemIndex , "MapImage" )->GetString();

#ifdef _FINAL_BUILD
		if( pSox->GetFieldFromLablePtr( nItemIndex, "ReleaseShow" )->GetInteger() == 0 )
			continue;
#endif // #ifdef _FINAL_BUILD

		vMapData.push_back( TempData );
	}
	if( vMapData.size() > 0 )
		m_SelectedMapIndex = vMapData[0].nMapIndex;
}

void CDnPVPMakeRoomDlg::MakeDropDownList( )
{
	m_pComboBoxMapSelection->RemoveAllItems();	

	if( vMapData.size() > 0 )
	{
		for(UINT i=0; i < vMapData.size() ; i++)
		{
			bool bEnableMap = true;
			bool bIsLadderMode = false;
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
			if(pSox)
			{
				bIsLadderMode = pSox->GetFieldFromLablePtr( vMapData[i].nGameMode[0], "LadderType" )->GetInteger() ? true : false; 
				// Using vMapData[i].nGameMode[0] index 0 feels a bit awkward, but currently the way to check whether it is ladder mode is by that index
				// Get the value from the mode table; for Ladder, only 1 mode is used, so handle it like this.
				// The surest way is to add a Ladder parameter in the map table to distinguish it. Will do so later if needed ..
			}
			if(bIsLadderMode)
				bEnableMap = false;

			if(bEnableMap)
			{
				m_pComboBoxMapSelection->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, vMapData[i].nUIStringIndex ),
					NULL, vMapData[i].nMapIndex, true );
			}
		}
		m_pComboBoxMapSelection->SetSelectedByIndex(0);
	}	
}

void CDnPVPMakeRoomDlg::MakeDropPlayDownList( UINT index )
{
	if(!m_pComboBoxPlayerNum->IsEnable())
		return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if ( !pSox ) 
	{
		ErrorLog("CDnPVPMakeRoomDlg::MakeGameModeDropDownList():: gamemode table Not found!! ");
		return;
	}

	int ModeIndex = m_pComboBoxGameMode->GetSelectedIndex();
	int nMinPlayerNumber =0;
	int nMaxPlayerNumber =0;
	int nModeTableNum;                 
	int DefaultIndex = 0;
	WCHAR PlayNum[5][10];

	nModeTableNum = vMapData[index].nGameMode[ModeIndex];

	nMinPlayerNumber = pSox->GetFieldFromLablePtr( nModeTableNum , "NumOfPlayers_Min" )->GetInteger();
	nMaxPlayerNumber = pSox->GetFieldFromLablePtr( nModeTableNum , "NumOfPlayers_Max" )->GetInteger();

	if(nMaxPlayerNumber==0)
		nMaxPlayerNumber = 100;

	m_pComboBoxPlayerNum->RemoveAllItems();

	if( index < 0  ||  index >= vMapData.size() )
		return;

	for(int i=0;i < MapData_PlayerMAX ;i++)
	{
		if( vMapData[index].bPlayerNum[i] != 0  && (vMapData[index].bPlayerNum[i]>= nMinPlayerNumber) && (vMapData[index].bPlayerNum[i]<= nMaxPlayerNumber))
		{
			wsprintf(PlayNum[i],L"%d", vMapData[index].bPlayerNum[i]);
			m_pComboBoxPlayerNum->AddItem( PlayNum[i] , NULL,  vMapData[index].bPlayerNum[i] );

			if( vMapData[index].bPlayerNum[i] == vMapData[index].bRecommandedPlayerNum )
				DefaultIndex = i;
		}

	}

	m_pComboBoxPlayerNum->SetSelectedByIndex(DefaultIndex);
	m_pComboBoxPlayerNum->GetSelectedValue(m_SelectedPlayerNum);
}

void CDnPVPMakeRoomDlg::SetMapDropDownList( UINT nCurMapIndex, LPCWSTR wszItemName,  byte cCurMaxUserNum )
{
	if( nCurMapIndex <= 0 )
		return;
	m_pComboBoxMapSelection->RemoveAllItems();

	if( vMapData.size() > 0 )
	{
		for(UINT i=0; i < vMapData.size() ; i++)
		{
			bool bEnableMap = false;
			bool bIsLadderMode = false;

			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
			if(pSox)
				bIsLadderMode = pSox->GetFieldFromLablePtr( vMapData[i].nGameMode[0], "LadderType" )->GetInteger() ? true : false; 

			for(int j=0;j < MapData_PlayerMAX ;j++)
			{
				if( vMapData[i].bPlayerNum[j] == cCurMaxUserNum )
				{
					bEnableMap = true;
					break;
				}
			}

			if( bEnableMap && !bIsLadderMode )
			{
				m_pComboBoxMapSelection->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, vMapData[i].nUIStringIndex ), NULL, vMapData[i].nMapIndex, true );

				if( vMapData[i].nMapIndex == nCurMapIndex && m_pComboBoxMapSelection )
				{
					int iIndex = m_pComboBoxMapSelection->FindItem( wszItemName );
					m_pComboBoxMapSelection->SetSelectedByIndex(iIndex);
				}
			}
		}
	}

}


void CDnPVPMakeRoomDlg::SetGameModeDownList( UINT nCurMapIndex, LPCWSTR wszItemName )
{
	if(m_pComboBoxMapSelection)
	{
		for( int i = 0; i < (int)vMapData.size(); ++i ) 
		{
			if( vMapData[i].nMapIndex == nCurMapIndex ) 
			{
				MakeGameModeDropDownList(i); // Must search map data to check whether the current map index matches 
			}
		}
	}

	if( m_pComboBoxGameMode )
	{
		int iIndex = m_pComboBoxGameMode->FindItem( wszItemName );

		if( -1 != iIndex )
			m_pComboBoxGameMode->SetSelectedByIndex(iIndex);
	}

}

void CDnPVPMakeRoomDlg::SetInGameJoin( bool IsJoin  )
{
	if( m_pCheckBoxInGameJoin )
		m_pCheckBoxInGameJoin->SetChecked( IsJoin );

}

void CDnPVPMakeRoomDlg::SetDropItem( bool bDropItem )
{
	if( m_pCheckBoxDropItem )
		m_pCheckBoxDropItem->SetChecked( bDropItem );
}

void CDnPVPMakeRoomDlg::SetShowHp( bool bDropItem ) 
{
	if( m_pCheckBoxShowHP )
		m_pCheckBoxShowHP->SetChecked( bDropItem );
}

void CDnPVPMakeRoomDlg::SetRevision( bool bRevision )
{
	if( m_pCheckBoxRevision )
		m_pCheckBoxRevision->SetChecked( !bRevision );
}

void CDnPVPMakeRoomDlg::SetRandomOrder( bool bRandomOrder )
{
	if( m_pCheckBoxRandomOrder )
		m_pCheckBoxRandomOrder->SetChecked(bRandomOrder);
}

void CDnPVPMakeRoomDlg::SetRandomTeamMode( bool bRandomTeamMode ) 
{
	if( m_pCheckBoxRandomTeamMode )
		m_pCheckBoxRandomTeamMode->SetChecked( bRandomTeamMode );
}


void CDnPVPMakeRoomDlg::SetGameWinObjectNum( LPCWSTR wszItemName  )
{
	if( m_pComboBoxVictoryCondition )
	{
		int iIndex = m_pComboBoxVictoryCondition->FindItem( wszItemName );

		if( -1 != iIndex )
			m_pComboBoxVictoryCondition->SetSelectedByIndex(iIndex);
	}
}

void CDnPVPMakeRoomDlg::SetPlayerLevel(int nLevelMin, int nLevelMax)
{	
	m_pEditBoxMinLevel->SetIntToText(nLevelMin);
	m_pEditBoxMaxLevel->SetIntToText(nLevelMax);
}

#ifdef PRE_MOD_PVPOBSERVER
void CDnPVPMakeRoomDlg::SetObserver( bool b )
{
	if( m_pCheckBoxObserver )
		m_pCheckBoxObserver->SetChecked( b );
}
#endif // PRE_MOD_PVPOBSERVER

void CDnPVPMakeRoomDlg::SetMaxPlayerDownList( byte cMaxUserNum )
{
	if( cMaxUserNum <= 0 )
		return;

	int iIndex = 0;
	WCHAR wszMaxPlayerNum[10];
#ifdef PRE_ADD_PVP_TOURNAMENT
	ZeroMemory(wszMaxPlayerNum, sizeof(wszMaxPlayerNum));
#else
	SecureZeroMemory(wszMaxPlayerNum,sizeof(wszMaxPlayerNum));
#endif
	wsprintf(wszMaxPlayerNum,L"%d",cMaxUserNum);

	if( m_pComboBoxPlayerNum )
	{
		m_pComboBoxPlayerNum->RemoveAllItems();
		m_pComboBoxPlayerNum->AddItem( wszMaxPlayerNum, NULL, cMaxUserNum );
		m_pComboBoxPlayerNum->Enable(false);
	}
}

void CDnPVPMakeRoomDlg::MakeGameModeDropDownList( UINT index )
{
	m_pComboBoxGameMode->RemoveAllItems();

	if( index < 0  ||  index >= vMapData.size() )
		return;

	int nGameModeUIString = 0;
	int nLastSelectedIndex = m_SelectedModeIndex;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if ( !pSox ) 
	{
		ErrorLog("CDnPVPMakeRoomDlg::MakeGameModeDropDownList():: gamemode table Not found!! ");
		return;
	}

	if(m_pComboBoxPlayerNum && IsEditRoom() )
	{
#ifdef PRE_ADD_PVP_TOURNAMENT
#else
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
		if ( !pSox ) 
		{
			ErrorLog("CDnPVPMakeRoomDlg::MakeGameModeDropDownList():: gamemode table Not found!! ");
			return;
		}
#endif

		int nSelectedPlayerNumber = 0;
		int nMinPlayerNumber =0;
		int nMaxPlayerNumber =0;

		m_pComboBoxPlayerNum->GetSelectedValue(nSelectedPlayerNumber);

		for(int i=0;i < MapData_ModeMAX ;i++)
		{
			if( vMapData[index].nGameMode[i] != 0 )
			{
				nMinPlayerNumber = pSox->GetFieldFromLablePtr(  vMapData[index].nGameMode[i] , "NumOfPlayers_Min" )->GetInteger(); // Get player count from the mode table
				nMaxPlayerNumber = pSox->GetFieldFromLablePtr(  vMapData[index].nGameMode[i] , "NumOfPlayers_Max" )->GetInteger(); // Get player count from the mode table.

				int nGameMode = pSox->GetFieldFromLablePtr( vMapData[index].nGameMode[i] , "GamemodeID" )->GetInteger();


				bool bMatchPlayerNum = false;
				for(int j=0; j<5 ; j++)
				{
					if(vMapData[index].bPlayerNum[j] == nSelectedPlayerNumber)
					{
						bMatchPlayerNum = true; // Check if the map's required player count matches the current player count
					}
				}

				if(nSelectedPlayerNumber < nMinPlayerNumber || nMinPlayerNumber > nMaxPlayerNumber) // Check if it matches the min/max player count in the mode table
				{
					bMatchPlayerNum = false;
				}

				if(!bMatchPlayerNum) continue; // Current mode does not match the configured player count -> skip


				nGameModeUIString = pSox->GetFieldFromLablePtr( vMapData[index].nGameMode[i] , "GameModeUIString" )->GetInteger();
				m_pComboBoxGameMode->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nGameModeUIString ) , NULL,  vMapData[index].nGameMode[i] );
			}
		}

		m_nGameModeTableID = vMapData[index].nGameMode[0];
		m_pComboBoxGameMode->SetSelectedByIndex(0);
		m_SelectedModeIndex = nLastSelectedIndex; // Whenever AddItem is called, it enters process command setting and gets initialized.
		m_pComboBoxGameMode->SetSelectedByIndex(nLastSelectedIndex);

		return; // Make it operate independently from the calculation below while editing. 
	}


	for(int i=0;i < MapData_ModeMAX ;i++)
	{
		if( vMapData[index].nGameMode[i] != 0 )
		{
			int nGameMode = pSox->GetFieldFromLablePtr( vMapData[index].nGameMode[i] , "GamemodeID" )->GetInteger();

			nGameModeUIString = pSox->GetFieldFromLablePtr( vMapData[index].nGameMode[i] , "GameModeUIString" )->GetInteger();
			if(nGameModeUIString != 0)
				m_pComboBoxGameMode->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nGameModeUIString ) , NULL,  vMapData[index].nGameMode[i] );
		}
	}

	m_nGameModeTableID = vMapData[index].nGameMode[0];
	m_pComboBoxGameMode->SetSelectedByIndex(0);
	m_SelectedModeIndex = nLastSelectedIndex; //  Whenever AddItem is called, it enters process command setting and gets initialized.
	m_pComboBoxGameMode->SetSelectedByIndex(nLastSelectedIndex);
}

void CDnPVPMakeRoomDlg::MakeVicConDropDownList( )
{
	m_pComboBoxVictoryCondition->RemoveAllItems();

	if( m_nGameModeTableID == 0 )
		return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if ( !pSox ) 
	{
		ErrorLog("CDnPVPMakeRoomDlg::MakeVicConDropDownList:: gamemode table Not found!! ");
		return;
	}

	int nVicCon = 0;
#ifdef PRE_ADD_PVP_TOURNAMENT
	std::wstring wszWinCon;
	DNTableCell* pWinConditionCell = pSox->GetFieldFromLablePtr(m_nGameModeTableID , "WinCondition_UIString");
	if (pWinConditionCell)
	{
		int winConditionStringIndex = pWinConditionCell->GetInteger();
		wszWinCon = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, winConditionStringIndex);
	}

	int nGameMode = pSox->GetFieldFromLablePtr( m_nGameModeTableID , "GamemodeID" )->GetInteger();
	if (nGameMode == PvPCommon::GameMode::PvP_Tournament)
	{
		m_pComboBoxVictoryCondition->AddItem(wszWinCon.c_str(), NULL, 1);
		m_pComboBoxVictoryCondition->SetSelectedByValue(1);
		return;
	}
#else
	WCHAR wszWinCon[256];
	SecureZeroMemory(wszWinCon,sizeof(wszWinCon));
	wsprintf(wszWinCon ,L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( m_nGameModeTableID , "WinCondition_UIString" )->GetInteger()) );
#endif

	WCHAR wszResult[256];
#ifdef PRE_ADD_PVP_TOURNAMENT
	ZeroMemory(wszResult, sizeof(wszResult));
#else
	SecureZeroMemory(wszResult,sizeof(wszResult));
#endif

	for(int i=0; i<5; i++)
	{
		nVicCon = pSox->GetFieldFromLablePtr( m_nGameModeTableID , FormatA("WinCondition_%d",i+1).c_str() )->GetInteger(); 
		if( nVicCon != 0 )
		{
#ifdef PRE_ADD_PVP_TOURNAMENT
			if (PvPCommon::GameMode::PvP_AllKill == nGameMode)
			{
				m_pComboBoxVictoryCondition->AddItem(wszWinCon.c_str(), NULL,  i+1);
				continue;
			}
#else
			int nGameMode = pSox->GetFieldFromLablePtr( m_nGameModeTableID , "GamemodeID" )->GetInteger();
			if( PvPCommon::GameMode::PvP_AllKill == nGameMode )
			{
				m_pComboBoxVictoryCondition->AddItem( wszWinCon , NULL,  i+1  );
				continue;
			}
#endif // PRE_ADD_PVP_TOURNAMENT
#ifdef PRE_ADD_PVP_TOURNAMENT
			wsprintf(wszResult ,L"%d%s",nVicCon , wszWinCon.c_str());
#else
			wsprintf(wszResult ,L"%d%s",nVicCon , wszWinCon );
#endif

#ifdef PRE_ADD_PVP_COMBOEXERCISE
			if( PvPCommon::GameMode::PvP_ComboExercise == nGameMode )
				m_pComboBoxVictoryCondition->AddItem( wszWinCon.c_str(), NULL,  i+1  );
			else
				m_pComboBoxVictoryCondition->AddItem( wszResult , NULL,  i+1  );
#else
			m_pComboBoxVictoryCondition->AddItem( wszResult , NULL,  i+1  );
#endif // PRE_ADD_PVP_COMBOEXERCISE

		}
	}

	int DefaultIndex = 0;
	DefaultIndex = pSox->GetFieldFromLablePtr( m_nGameModeTableID , "DefaultGameWinType" )->GetInteger();
	m_pComboBoxVictoryCondition->SetSelectedByValue(DefaultIndex);

}

void CDnPVPMakeRoomDlg::GetGamemodeInfo( int nIndex )
{
	if( nIndex <= 0 && m_nGameModeTableID > 0)
		return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if ( !pSox ) 
	{
		ErrorLog("CDnPVPMakeRoomDlg::GetGamemodeInfo:: gamemode table Not found!! ");
		return;
	}

	char szConField[20];
	char szTimeField[20];

	SecureZeroMemory(szConField,sizeof(szConField));
	SecureZeroMemory(szTimeField,sizeof(szTimeField));	

	sprintf(szConField ,"WinCondition_%d",nIndex);
	sprintf(szTimeField,"PlayTime_%d",nIndex);

	m_nWinCondition = pSox->GetFieldFromLablePtr( m_nGameModeTableID ,szConField )->GetInteger();
	m_nPlayTimeSec = pSox->GetFieldFromLablePtr( m_nGameModeTableID ,szTimeField )->GetInteger();

}

#ifdef PRE_ADD_PVP_TOURNAMENT
int CDnPVPMakeRoomDlg::GetGameModeData(int nGameModeTableID, const char* pLableString) const
{
	if (nGameModeTableID <= 0 || pLableString == NULL || pLableString == '\0')
		return -1;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if (!pSox)
	{
		ErrorLog("CDnPVPMakeRoomDlg::GetGamemodeInfo:: gamemode table Not found!! ");
		return -1;
	}

	DNTableCell* pCell = pSox->GetFieldFromLablePtr(nGameModeTableID, pLableString);
	if (pCell == NULL)
		return -1;

	return pCell->GetInteger();
}
#endif

void CDnPVPMakeRoomDlg::MakeMapImage( UINT index )
{
	if( index < 0  ||  index >= vMapData.size() )
		return;

	SAFE_RELEASE_SPTR( m_hMiniMapImage );

	if( vMapData[index].szMapImage.c_str() ) {
		m_hMiniMapImage = LoadResource( CEtResourceMng::GetInstance().GetFullName(vMapData[index].szMapImage.c_str()).c_str(), RT_TEXTURE );

		if( m_hMiniMapImage )
			m_pMiniMap->SetTexture( m_hMiniMapImage, 0, 0, m_hMiniMapImage->Width() , m_hMiniMapImage->Height() );	
	}
}

const wchar_t* CDnPVPMakeRoomDlg::MakeDefaultRoomName()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGAMEROOMNAME );
	if ( pSox ) 
	{
		std::vector<int> vUISting;
		for( int i= 1; i< pSox->GetItemCount()+1; i++ ) 
		{
			int nUISting = 0;
			nUISting = pSox->GetFieldFromLablePtr( i, "RoomNameUIstring" )->GetInteger();
			if( nUISting )
				vUISting.push_back( nUISting );
			else
				break;
		}

		if( vUISting.size() > 0 )
			return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, vUISting[_rand()%vUISting.size()] );
	}
	return L"";

}

// Shuffle PvP map and PvP game mode.
void CDnPVPMakeRoomDlg::Shuffle()
{
	if( m_pComboBoxMapSelection )
	{
		m_pComboBoxMapSelection->SetSelectedByIndex( _rand()%m_pComboBoxMapSelection->GetItemCount() );
		ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, m_pComboBoxMapSelection, 0 );
	}

	if( m_pComboBoxGameMode )
	{
		m_pComboBoxGameMode->SetSelectedByIndex( _rand()%m_pComboBoxGameMode->GetItemCount() );
		ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, m_pComboBoxGameMode, 0 );
	}
}

void CDnPVPMakeRoomDlg::InitCheckBox()
{
	bool bShowHp = false;
	bool bDropItem = m_pCheckBoxDropItem->IsEnable() ? true : false;
	bool bIngameJoin = m_pCheckBoxInGameJoin->IsEnable() ? true : false;
	bool bRandomTeamMode = false;
	bool bRevision = false;
	bool bRandomOrder = false;

	DNTableFileFormat* pSoxMode = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if(pSoxMode)
	{
		int nItemValue = 0;
		int nGameMode = 0;
		m_pComboBoxGameMode->GetSelectedValue(nItemValue);

		nGameMode = pSoxMode->GetFieldFromLablePtr( nItemValue, "GamemodeID" )->GetInteger();
		if( nGameMode == PvPCommon::GameMode::PvP_GuildWar)
			bIngameJoin = true;
#ifdef PRE_ADD_PVP_TOURNAMENT
		if (nGameMode == PvPCommon::GameMode::PvP_Tournament)
			bIngameJoin = false;
#endif
#ifdef PRE_ADD_PVP_COMBOEXERCISE
		if (nGameMode == PvPCommon::GameMode::PvP_ComboExercise)
			bDropItem = true;
#endif // PRE_ADD_PVP_COMBOEXERCISE

		bRevision = pSoxMode->GetFieldFromLablePtr( nItemValue, "_DefaultRevision" )->GetInteger() ? true : false;
	}

	m_pCheckBoxShowHP->SetChecked(bShowHp);
	m_pCheckBoxDropItem->SetChecked(bDropItem);
	m_pCheckBoxInGameJoin->SetChecked(bIngameJoin);
	m_pCheckBoxRandomTeamMode->SetChecked(bRandomTeamMode);
	m_pCheckBoxRevision->SetChecked(bRevision);
	m_pCheckBoxRandomOrder->SetChecked(bRandomOrder);
}

void CDnPVPMakeRoomDlg::SetGameModeButtonState()
{
	int nItemValue = 0;
	bool bDisableShowHpMode = false;
	bool bDisalbeRandomTeamMode = false;
	bool bDisableRevisionMode = false;
	bool bCheckRevisionMode = true;
	bool bDisableUseItem = false;
	bool bDisableRandomOrder = true;
	bool bDisableVictoryCondition = false;
	bool bDisableBreakInto = false;
#ifdef PRE_ADD_PVP_TOURNAMENT
	bool bDisableSetPlayerNumber = IsEditRoom();
#endif

	if( m_SelectedMapIndex >= 0  &&  m_SelectedMapIndex < (int)vMapData.size() )
	{
		m_pCheckBoxInGameJoin->SetChecked( vMapData[m_SelectedMapIndex].isInGameJoin );
		bDisableBreakInto = !vMapData[m_SelectedMapIndex].isInGameJoin;
	}


	m_pComboBoxGameMode->GetSelectedValue(nItemValue);
	DNTableFileFormat* pSoxMode = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if(pSoxMode)
	{
		int nGameMode = pSoxMode->GetFieldFromLablePtr( nItemValue, "GamemodeID" )->GetInteger() ;

		switch(nGameMode)
		{
		case PvPCommon::GameMode::PvP_Captain:
			{
				bDisableShowHpMode = true;
			}
			break;
		case PvPCommon::GameMode::PvP_IndividualRespawn:
			{
				bDisalbeRandomTeamMode = true;
			}
			break;
		case PvPCommon::GameMode::PvP_Zombie_Survival:
			{
				bDisalbeRandomTeamMode = true;
			}
			break;
		case PvPCommon::GameMode::PvP_GuildWar:
			{
				bDisableBreakInto = true;
				bDisableShowHpMode = true;
				bDisalbeRandomTeamMode = true;
				bDisableVictoryCondition = true;
			}
			break;
		case PvPCommon::GameMode::PvP_AllKill:
			{
				bDisableBreakInto = true;
				bDisableUseItem = true;
				bDisableRandomOrder = false;
				bDisableVictoryCondition = true;
			}
			break;
#if defined( PRE_ADD_RACING_MODE )
		case PvPCommon::GameMode::PvP_Racing:
			{
				bDisableBreakInto = true;
				bDisableUseItem = true;
				bDisableShowHpMode = true;
				bDisalbeRandomTeamMode = true;
				bDisableVictoryCondition = true;
			}
			break;
#endif	// #if defined( PRE_ADD_RACING_MODE )
#ifdef PRE_ADD_PVP_TOURNAMENT
		case PvPCommon::GameMode::PvP_Tournament:
			{
				bDisableBreakInto = true;
				bDisableUseItem = false;
				int maxPlayerNum = GetGameModeData(nItemValue, "NumOfPlayers_Max");
				if (maxPlayerNum > 0)
					SetMaxPlayerDownList(maxPlayerNum);
				bDisableSetPlayerNumber = true;
				bDisableVictoryCondition = true;
			}
			break;
#endif

#ifdef PRE_ADD_PVP_COMBOEXERCISE
		case PvPCommon::GameMode::PvP_ComboExercise:
			{
				bDisableShowHpMode = bDisableRandomOrder = bDisalbeRandomTeamMode = bDisableUseItem = bDisableRevisionMode = bDisableVictoryCondition = true;				
			}
			break;
#endif // PRE_ADD_PVP_COMBOEXERCISE


		}

		bDisableRevisionMode = pSoxMode->GetFieldFromLablePtr( nItemValue, "_SelectableRevision" )->GetInteger() ? false : true;
		bCheckRevisionMode = pSoxMode->GetFieldFromLablePtr( nItemValue, "_DefaultRevision" )->GetInteger() ? true : false;
	}

	m_pCheckBoxDropItem->Enable( !bDisableUseItem );
	m_pCheckBoxInGameJoin->Enable( !bDisableBreakInto );
	m_pCheckBoxShowHP->Enable(!bDisableShowHpMode);
	m_pCheckBoxRandomTeamMode->Enable(!bDisalbeRandomTeamMode);
	m_pComboBoxVictoryCondition->Enable( !bDisableVictoryCondition );
	m_pCheckBoxRevision->Enable( !bDisableRevisionMode );
	m_pCheckBoxRevision->SetChecked( bCheckRevisionMode );
#ifdef PRE_ADD_PVP_TOURNAMENT
	m_pComboBoxPlayerNum->Enable(!bDisableSetPlayerNumber);
#endif
	m_pCheckBoxRandomOrder->Enable( !bDisableRandomOrder && !m_bRandomTeamMode );
	m_pCheckBoxRandomOrder->Show( !bDisableRandomOrder );
	m_pStaticRandomOrder->Show( !bDisableRandomOrder );
}

#ifdef PRE_ADD_PVP_HELP_MESSAGE
bool CDnPVPMakeRoomDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	bool bResult = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );

	switch( uMsg )
	{
	case WM_LBUTTONDBLCLK:
		{
		}
		break;

	case WM_LBUTTONDOWN:
		{
		}
		break;

	case WM_RBUTTONDOWN:
		{
		}
		break;

	case WM_MOUSEWHEEL:
		{
		}
		break;

	case WM_MOUSEMOVE:
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fMouseX, fMouseY;
			PointToFloat( MousePoint, fMouseX, fMouseY );

			if(m_pComboBoxGameMode == NULL || m_nGameModeTableID <= 0)
				return false;

			if(m_pComboBoxGameMode->IsInside(fMouseX, fMouseY) && m_pComboBoxGameMode->IsShow() )
			{
				m_pComboBoxGameMode->GetSelectedValue(m_nGameModeTableID);

				DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
				if (!pSox)
					return false;

				int nToolTipStringID	= pSox->GetFieldFromLablePtr(m_nGameModeTableID , "_TooltipUIString")->GetInteger();
				std::wstring wszTooltip = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nToolTipStringID );
				if(!wszTooltip.empty())
					m_pComboBoxGameMode->GetParent()->ShowTooltipDlg(m_pComboBoxGameMode, true, wszTooltip, 0xffffffff, true);
			}
		}
		break;
	}

	return bResult;
}
#endif



#endif // PRE_MOD_PVP_ROOM_CREATE