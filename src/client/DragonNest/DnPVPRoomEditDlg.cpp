#include "StdAfx.h"
#include "DnPVPRoomEditDlg.h"
#include "DnInterface.h"
#include "PVPSendPacket.h"
#include "DnTableDB.h"
#include "DnInterfaceString.h"

#include "DnPVPLobbyVillageTask.h"
#include "TaskManager.h"
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#include "DnPVPLadderTabDlg.h"
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

#ifdef PRE_MOD_PVPOBSERVER
#include "DnBaseRoomDlg.h"
#endif // PRE_MOD_PVPOBSERVER


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPVPRoomEditDlg::CDnPVPRoomEditDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnPVPMakeRoomDlg( dialogType, pParentDialog, nID, pCallback )
{
	iMaxPlayerLevel = 100;
	iMinPlayerLevel = 1;
	bIsEditRoom = false;
}

CDnPVPRoomEditDlg::~CDnPVPRoomEditDlg(void)
{

}

void CDnPVPRoomEditDlg::SendMakeRoom()
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

	GetInterface().SetPVPRoomPassword(wszRoomPW);

	if( m_pEditBoxPassWord->GetTextLength() >= 1 &&  m_pEditBoxPassWord->GetTextLength() <= 3 )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPMakeGame::PasswordShort) , MB_OK);	
		return;
	}

	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
	if(!pPVPLobbyTask)
		return;

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	char cRoomType = 0;

	CDnPVPLadderTabDlg* pPVPLadderTabDlg = GetInterface().GetPVPLadderTabDlg();
	if( pPVPLadderTabDlg )
	{
		cRoomType = static_cast<char>(pPVPLadderTabDlg->GetSelectedGradeChannel());
	}
	
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	int nPlayerLevel = pPVPLobbyTask->GetUserInfo().cLevel;

	if(m_pEditBoxMinLevel->GetTextToInt() > nPlayerLevel)
		m_pEditBoxMinLevel->SetIntToText(nPlayerLevel);

	if(m_pEditBoxMaxLevel->GetTextToInt() <  nPlayerLevel)
		m_pEditBoxMaxLevel->SetIntToText(nPlayerLevel);

	if(bIsEditRoom)
	{
		if(m_pEditBoxMinLevel->GetTextToInt() > iMinPlayerLevel)
			m_pEditBoxMinLevel->SetIntToText(iMinPlayerLevel);

		if(m_pEditBoxMaxLevel->GetTextToInt() < iMaxPlayerLevel)
			m_pEditBoxMaxLevel->SetIntToText(iMaxPlayerLevel);

		if(m_pEditBoxMinLevel->GetTextToInt() > m_pEditBoxMaxLevel->GetTextToInt())
			m_pEditBoxMaxLevel->SetIntToText(m_pEditBoxMinLevel->GetTextToInt());

		bIsEditRoom = false;
	}

	SendPvPModifyRoom( 
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
		m_bShowHp , 
		m_bRandomTeamMode , 
		m_bRevision, 
		m_bRandomOrder,
#ifdef PRE_MOD_PVPOBSERVER
		m_bObserverAccess,
		GetInterface().GetGameRoomDlg()->IsEventRoom(),
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		cRoomType,
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
		BYTE(m_pEditBoxMinLevel->GetTextToInt()),BYTE(m_pEditBoxMaxLevel->GetTextToInt()));

	Show(false);
}

void CDnPVPRoomEditDlg::SetRoomName( const WCHAR * RoomName )
{
	if( RoomName && m_pEditBoxRoomName )
	{
		m_pEditBoxRoomName->SetText(RoomName);
	}
}

void CDnPVPRoomEditDlg::SetPassword()
{
	if(GetInterface().GetPVPRoomPassword().c_str() && m_pEditBoxPassWord)
		m_pEditBoxPassWord->SetText(GetInterface().GetPVPRoomPassword().c_str());
}

void CDnPVPRoomEditDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

#ifdef PRE_MOD_PVP_ROOM_CREATE
	if(nCommand == EVENT_COMBOBOX_SELECTION_CHANGED)
	{
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
			SetGameModeButtonState();
			return;
		}
	}
#else
	if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED )
	{
		if( IsCmdControl("ID_PVP_MAKEROOM_INPUTMAP") )
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
	}
#endif // PRE_MOD_PVP_ROOM_CREATE

	CDnPVPMakeRoomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

#ifdef PRE_ADD_PVP_TOURNAMENT
void CDnPVPRoomEditDlg::EnableMapDropDownList(bool bEnable)
{
	m_pComboBoxMapSelection->Enable(bEnable);
}

void CDnPVPRoomEditDlg::EnableGameModeDropDownList(bool bEnable)
{
	m_pComboBoxGameMode->Enable(bEnable);
}

void CDnPVPRoomEditDlg::EnableRandomTeam(bool bEnable)
{
	m_pCheckBoxRandomTeamMode->Enable(bEnable);
}
#endif