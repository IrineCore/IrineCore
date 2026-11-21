#include "StdAfx.h"
#include "DnPVPLadderTeamGameDlg.h"
#include "DnInterfaceString.h"
#include "PvPSendPacket.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnPVPLadderTeamGameDlg::CDnPVPLadderTeamGameDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	pStaticCover_SelfMaster = NULL;
	m_pLadderInviteUserListDlg = NULL;
	m_nRoomState = 0;
	m_nCurrentInvitedUserNumber = 0;
	m_nInviteRoomCount = eLadderTeamUI_Common::None;
	m_bIsMasterUser = true;
}

CDnPVPLadderTeamGameDlg::~CDnPVPLadderTeamGameDlg(void)
{	
	SAFE_DELETE(m_pLadderInviteUserListDlg);
}


void CDnPVPLadderTeamGameDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpLadderPartyDlg.ui" ).c_str(), bShow );
}

void CDnPVPLadderTeamGameDlg::InitialUpdate()
{
	m_pLadderInviteUserListDlg = new CDnPVPLadderInviteUserListDlg(UI_TYPE_MODAL, this );
	m_pLadderInviteUserListDlg->Initialize( false );


	InitializeLadderTeamUI();
	pStaticCover_SelfMaster = GetControl<CEtUIStatic>("ID_STATIC_MASTER");

	for(int i=0;i<MaxTeamSlot;i++)
		m_sLadderTeamUI[i].pStaticCover_Master->Show(false);
	pStaticCover_SelfMaster->Show(true);
}

void CDnPVPLadderTeamGameDlg::InitializeLadderTeamUI()
{
	char wszUIName[256];

	for(int i=0;i<MaxTeamSlot;i++)
	{
		sprintf(wszUIName, "ID_TEXT_NAME%d", i);
		m_sLadderTeamUI[i].pStatic_Name = GetControl<CEtUIStatic>(wszUIName);
		sprintf(wszUIName, "ID_TEXT_SCORE%d", i);
		m_sLadderTeamUI[i].pStatic_LadderPoint = GetControl<CEtUIStatic>(wszUIName);
		sprintf(wszUIName, "ID_TEXT_GRADE%d", i);
		m_sLadderTeamUI[i].pStatic_LadderGrade = GetControl<CEtUIStatic>(wszUIName);
		sprintf(wszUIName, "ID_TEXT_CLASS%d", i);
		m_sLadderTeamUI[i].pStatic_JobName = GetControl<CEtUIStatic>(wszUIName);

		sprintf(wszUIName, "ID_TEXT%d_0", i);
		m_sLadderTeamUI[i].pStaticCover_Name =GetControl<CEtUIStatic>(wszUIName);
		sprintf(wszUIName, "ID_TEXT%d_1", i);         
		m_sLadderTeamUI[i].pStaticCover_LadderPoint = GetControl<CEtUIStatic>(wszUIName);
		sprintf(wszUIName, "ID_TEXT%d_2", i);         
		m_sLadderTeamUI[i].pStaticCover_LadderGrade = GetControl<CEtUIStatic>(wszUIName);
		sprintf(wszUIName, "ID_TEXT%d_3", i);         
		m_sLadderTeamUI[i].pStaticCover_JobName = GetControl<CEtUIStatic>(wszUIName);
		
		sprintf(wszUIName, "ID_STATIC_MASTER%d", i);         
		m_sLadderTeamUI[i].pStaticCover_Master = GetControl<CEtUIStatic>(wszUIName);
		sprintf(wszUIName, "ID_STATIC_COVER%d", i);         
		m_sLadderTeamUI[i].pStaticCover_Slot = GetControl<CEtUIStatic>(wszUIName);
		sprintf(wszUIName, "ID_STATIC_INBOARD%d", i);         
		m_sLadderTeamUI[i].pStaticCover_UserInit = GetControl<CEtUIStatic>(wszUIName);

		// button
		sprintf(wszUIName, "ID_BT_JOIN%d", i);         
		m_sLadderTeamUI[i].pStaticButton_Invite = GetControl<CEtUIButton>(wszUIName);
		m_sLadderTeamUI[i].pStaticButton_Invite->Show(false);
		sprintf(wszUIName, "ID_BT_OUT%d", i);         
		m_sLadderTeamUI[i].pStaticButton_Leave = GetControl<CEtUIButton>(wszUIName);
		m_sLadderTeamUI[i].pStaticButton_Leave->Show(false);
	}
}

void CDnPVPLadderTeamGameDlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );
	ResetAllSlot();

	if(bShow && m_pLadderInviteUserListDlg)
		m_pLadderInviteUserListDlg->Show(false);
}

int CDnPVPLadderTeamGameDlg::FindSlotByUserName(WCHAR *wszName)
{

	for(int i=0;i<MaxTeamSlot;i++)
	{
		if(__wcsicmp_l(m_sLadderTeamUI[i].pStatic_Name->GetText(),wszName) == NULL)
			return i;
	}
		
	return eLadderTeamUI_Common::None;
}

int CDnPVPLadderTeamGameDlg::FindSlotByUserDBID(INT64 nDBID)
{
	for(int i=0;i<MaxTeamSlot;i++)
	{
		if(m_sLadderTeamUI[i].nUserDBID == nDBID)
			return i;
	}

	return eLadderTeamUI_Common::None;
}

void CDnPVPLadderTeamGameDlg::ShowMasterMarkByName(WCHAR *wszName)
{
	for(int i=0;i<MaxTeamSlot;i++)
		m_sLadderTeamUI[i].pStaticCover_Master->Show(false);
	pStaticCover_SelfMaster->Show(false);
	eLadderTeamUI_Common eCode = (eLadderTeamUI_Common)FindSlotByUserName(wszName);

	m_bIsMasterUser = false;

	switch(eCode)
	{
	case eLadderTeamUI_Common::None :
		pStaticCover_SelfMaster->Show(true);
		m_bIsMasterUser = true;
		break;
	case eLadderTeamUI_Common::First :
		m_sLadderTeamUI[eCode].pStaticCover_Master->Show(true);
		break;
	case eLadderTeamUI_Common::Second :
		m_sLadderTeamUI[eCode].pStaticCover_Master->Show(true);
		break;
	case eLadderTeamUI_Common::Third :
		m_sLadderTeamUI[eCode].pStaticCover_Master->Show(true);
		break;
	}

	for(int i=0; i<eLadderTeamUI_Common::MaxTeamSlot; i++ )
	{
		m_sLadderTeamUI[i].pStaticButton_Invite->Enable(m_bIsMasterUser);
		m_sLadderTeamUI[i].pStaticButton_Leave->Enable(m_bIsMasterUser);
	}
}

void CDnPVPLadderTeamGameDlg::RefrehsLadderUser(LadderSystem::SC_REFRESH_USERINFO *pData)
{
	m_nCurrentInvitedUserNumber = pData->cCount;

	if(m_nCurrentInvitedUserNumber < eLadderTeamUI_Common::MaxTeamSlot)
	{
		for(int i=0; i<m_nCurrentInvitedUserNumber; i++ )
		{
			m_sLadderTeamUI[i].pStatic_Name->SetText(pData->sUserInfoArr[i].wszCharName);
			m_sLadderTeamUI[i].pStatic_LadderPoint->SetIntToText(pData->sUserInfoArr[i].iGradePoint);
			m_sLadderTeamUI[i].pStatic_JobName->SetText(DN_INTERFACE::STRING::GetJobString(pData->sUserInfoArr[i].cJob)); // Temporary code
			m_sLadderTeamUI[i].nUserDBID = pData->sUserInfoArr[i].biCharDBID;
			m_sLadderTeamUI[i].pStaticCover_UserInit->Show(true);
			
			if(m_bIsMasterUser)
			{
				// button
				m_sLadderTeamUI[i].pStaticButton_Invite->Show(false);
				m_sLadderTeamUI[i].pStaticButton_Leave->Show(true);
				m_sLadderTeamUI[i].pStaticButton_Leave->Enable(true);
			}
			else
			{
				// button
				m_sLadderTeamUI[i].pStaticButton_Invite->Show(false);
				m_sLadderTeamUI[i].pStaticButton_Leave->Show(true);
				m_sLadderTeamUI[i].pStaticButton_Leave->Enable(false);
			}
		}
	}
	m_nInviteRoomCount = eLadderTeamUI_Common::None;
}

void CDnPVPLadderTeamGameDlg::InsterLadderUser(LadderSystem::SC_NOTIFY_JOINUSER *pData)
{
	if(m_nCurrentInvitedUserNumber < eLadderTeamUI_Common::MaxTeamSlot)
	{
		int nEmptySlotIndex = 0;

		for(int i=0; i<m_nRoomState-1 ;i++)
		{
			if(m_nInviteRoomCount > eLadderTeamUI_Common::None && m_nInviteRoomCount < eLadderTeamUI_Common::MaxTeamSlot && m_nInviteRoomCount != i  )
				continue;
			
			if(m_sLadderTeamUI[i].nUserDBID == 0)
			{
				nEmptySlotIndex = i;
				break;
			}
		}

		WCHAR wszStr[256];
		wsprintf( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126250 ), pData->sUserInfo.wszCharName );
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  wszStr , textcolor::YELLOW ,4.0f );

		m_sLadderTeamUI[nEmptySlotIndex].pStatic_Name->SetText(pData->sUserInfo.wszCharName);
		
		m_sLadderTeamUI[nEmptySlotIndex].pStatic_LadderPoint->SetIntToText(pData->sUserInfo.iGradePoint);
		m_sLadderTeamUI[nEmptySlotIndex].pStatic_JobName->SetText(DN_INTERFACE::STRING::GetJobString(pData->sUserInfo.cJob)); // Temporary code
		m_sLadderTeamUI[nEmptySlotIndex].nUserDBID = pData->sUserInfo.biCharDBID;
		m_sLadderTeamUI[nEmptySlotIndex].pStaticCover_UserInit->Show(true);

		if(m_bIsMasterUser)
		{
			m_pLadderInviteUserListDlg->Show(false); // Close the window when a user enters.
			// button
			m_sLadderTeamUI[nEmptySlotIndex].pStaticButton_Invite->Show(false);
			m_sLadderTeamUI[nEmptySlotIndex].pStaticButton_Leave->Show(true);
			m_sLadderTeamUI[nEmptySlotIndex].pStaticButton_Leave->Enable(true);
		}
		else
		{
			// button
			m_sLadderTeamUI[nEmptySlotIndex].pStaticButton_Invite->Show(false);
			m_sLadderTeamUI[nEmptySlotIndex].pStaticButton_Leave->Show(true);
			m_sLadderTeamUI[nEmptySlotIndex].pStaticButton_Leave->Enable(false);
		}

		m_nCurrentInvitedUserNumber++;
	}

	m_nInviteRoomCount = eLadderTeamUI_Common::None;
}


void CDnPVPLadderTeamGameDlg::RemoveUserByDBID(INT64 nDBID)
{
	eLadderTeamUI_Common eCode = (eLadderTeamUI_Common)FindSlotByUserDBID(nDBID);
	if(eCode == eLadderTeamUI_Common::None)
		return;

	if(eCode < eLadderTeamUI_Common::MaxTeamSlot)
	{
		WCHAR wszStr[256];
		wsprintf( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126251 ), m_sLadderTeamUI[eCode].pStatic_Name->GetText() );
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  wszStr , textcolor::YELLOW ,4.0f );

		m_sLadderTeamUI[eCode].pStatic_Name->ClearText();
		m_sLadderTeamUI[eCode].pStatic_LadderPoint->ClearText();
		m_sLadderTeamUI[eCode].pStatic_JobName->ClearText();
		m_sLadderTeamUI[eCode].nUserDBID = 0;
		m_sLadderTeamUI[eCode].pStaticCover_UserInit->Show(false);

		if(m_bIsMasterUser)
		{
			// button
			m_sLadderTeamUI[eCode].pStaticButton_Invite->Show(true);
			m_sLadderTeamUI[eCode].pStaticButton_Invite->Enable(true);
			m_sLadderTeamUI[eCode].pStaticButton_Leave->Show(false);
		}
		else
		{
			// button
			m_sLadderTeamUI[eCode].pStaticButton_Invite->Show(true);
			m_sLadderTeamUI[eCode].pStaticButton_Invite->Enable(false);
			m_sLadderTeamUI[eCode].pStaticButton_Leave->Show(false);
		}
		m_nCurrentInvitedUserNumber--;
	}
	m_nInviteRoomCount = eLadderTeamUI_Common::None;
}

void CDnPVPLadderTeamGameDlg::ResetAllSlot()
{
	for(int i=0 ;i < eLadderTeamUI_Common::MaxTeamSlot ; i++)
	{
		m_sLadderTeamUI[i].pStatic_Name->ClearText();
		m_sLadderTeamUI[i].pStatic_LadderPoint->ClearText();
		m_sLadderTeamUI[i].pStatic_JobName->ClearText();
		m_sLadderTeamUI[i].nUserDBID = 0;
		m_sLadderTeamUI[i].pStaticCover_UserInit->Show(false);
		
		// button
		m_sLadderTeamUI[i].pStaticButton_Invite->Show(false);
		m_sLadderTeamUI[i].pStaticButton_Leave->Show(false);
		m_nCurrentInvitedUserNumber = 0;
	}
	m_nInviteRoomCount = eLadderTeamUI_Common::None;
	m_bIsMasterUser = true;
}

void CDnPVPLadderTeamGameDlg::ShowLadderTeamInfo(eLadderTeamUI_Common eCode , bool bShow)
{
	if(eCode == eLadderTeamUI_Common::None)
		return;

	m_sLadderTeamUI[eCode].pStatic_Name->Show(bShow);
	m_sLadderTeamUI[eCode].pStatic_LadderPoint->Show(bShow);
	m_sLadderTeamUI[eCode].pStatic_LadderGrade->Show(false);
	m_sLadderTeamUI[eCode].pStatic_JobName->Show(bShow);

	m_sLadderTeamUI[eCode].pStaticCover_Name->Show(bShow);
	m_sLadderTeamUI[eCode].pStaticCover_LadderPoint->Show(bShow);
	m_sLadderTeamUI[eCode].pStaticCover_LadderGrade->Show(false);
	m_sLadderTeamUI[eCode].pStaticCover_JobName->Show(bShow);
	m_sLadderTeamUI[eCode].pStaticCover_Slot->Show(!bShow);
	m_sLadderTeamUI[eCode].pStaticCover_UserInit->Show(false);

	// button
	m_sLadderTeamUI[eCode].pStaticButton_Invite->Show(bShow);
	m_sLadderTeamUI[eCode].pStaticButton_Invite->Enable(m_bIsMasterUser);
	m_sLadderTeamUI[eCode].pStaticButton_Leave->Show(false);
}

bool CDnPVPLadderTeamGameDlg::IsReady()
{
	if(m_nCurrentInvitedUserNumber+1 == m_nRoomState)
		return true;

	return false;
}

void CDnPVPLadderTeamGameDlg::SetRoomState(LadderSystem::MatchType::eCode eCode)
{
	for(int i=0;i<MaxTeamSlot;i++)
		ShowLadderTeamInfo((eLadderTeamUI_Common)i,true);
	
	switch(eCode)
	{
	case LadderSystem::MatchType::_2vs2 :
		for(int i=eLadderTeamUI_Common::Second;i<MaxTeamSlot;i++)
			ShowLadderTeamInfo((eLadderTeamUI_Common)i,false);
		break;
	case LadderSystem::MatchType::_3vs3 :
		ShowLadderTeamInfo(eLadderTeamUI_Common::Third,false);
		break;
	case LadderSystem::MatchType::_4vs4 :
		break;
	}

	for(int i=0;i<MaxTeamSlot;i++)
		m_sLadderTeamUI[i].pStaticCover_Master->Show(false);
	pStaticCover_SelfMaster->Show(true);

	m_nRoomState = eCode;
}

void CDnPVPLadderTeamGameDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		// Kick Out
		if( strcmp( pControl->GetControlName(), "ID_BT_OUT0" ) == 0 ) 
			SendLadderKickUser(m_sLadderTeamUI[eLadderTeamUI_Common::First].nUserDBID);

		if( strcmp( pControl->GetControlName(), "ID_BT_OUT1" ) == 0 ) 
			SendLadderKickUser(m_sLadderTeamUI[eLadderTeamUI_Common::Second].nUserDBID);

		if( strcmp( pControl->GetControlName(), "ID_BT_OUT2" ) == 0 ) 
			SendLadderKickUser(m_sLadderTeamUI[eLadderTeamUI_Common::Third].nUserDBID);

		// Invite : Need to remove test code
		if( strcmp( pControl->GetControlName(), "ID_BT_JOIN0" ) == 0 )
		{
			m_pLadderInviteUserListDlg->Show(true);
			m_nInviteRoomCount = eLadderTeamUI_Common::First; // Put into the slot I invited.
		}
		if( strcmp( pControl->GetControlName(), "ID_BT_JOIN1" ) == 0 )
		{
			m_pLadderInviteUserListDlg->Show(true);
			m_nInviteRoomCount = eLadderTeamUI_Common::Second; // Put into the slot I invited.
		}
		if( strcmp( pControl->GetControlName(), "ID_BT_JOIN2" ) == 0 )
		{
			m_pLadderInviteUserListDlg->Show(true);
			m_nInviteRoomCount = eLadderTeamUI_Common::Third; // Put into the slot I invited.
		}
	}
}
