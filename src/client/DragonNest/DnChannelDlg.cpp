#include "StdAfx.h"
#include "DnChannelDlg.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnInterface.h"
#include "DnInterfaceDlgID.h"
#include "PartySendPacket.h"
#include "DnTradeTask.h"
#include "DnTradePrivateMarket.h"
#include "DnMainMenuDlg.h"
#include "DnChatRoomTask.h"
//blondymarry start
//#include "DnVillageTask.h"
//#include "DnPartyTask.h"
//blondymarry end

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChannelDlg::CDnChannelDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnChannelListDlg( dialogType, pParentDialog, nID, pCallback, true )
{
}

CDnChannelDlg::~CDnChannelDlg(void)
{
}

void CDnChannelDlg::InitialUpdate()
{
	CDnChannelListDlg::InitialUpdate();

	m_pButtonOK->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 95 ) );
	m_pButtonCancel->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 72 ) );
	m_pButtonCancel->SetHotKey( VK_ESCAPE );
}

void CDnChannelDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_OK" ) )
		{
			SendChannel();
		}
	}
	else if( nCommand == EVENT_LISTBOX_ITEM_DBLCLK )
	{
		if( IsCmdControl("ID_LISTBOX_CHANNEL" ) )
		{
			SendChannel();
		}
	}
	else if( nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( IsCmdControl("ID_LISTBOX_CHANNEL" ) )
		{
			SListBoxItem *pItem = m_pChannelList->GetSelectedItem();
			if( pItem )
			{
				SChannelInfo *pInfo = (SChannelInfo*)(pItem->pData);
				if( CGlobalInfo::GetInstance().m_cLocalAccountLevel == 0 && pInfo->m_nCongestion == HIGH_CONGESTION )
				{
					m_pButtonOK->Enable( false );
				}
				else
				{
					m_pButtonOK->Enable( true );
				}
			}
		}
	}
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED ) 
	{
		if( IsCmdControl( "ID_RBT_TAB0" ) ) 
		{
			m_eChannelAttribute = CHANNEL_ATT_NORMAL;
			ResetList();
		}
		else if( IsCmdControl( "ID_RBT_TAB1" ) ) 
		{
			m_eChannelAttribute = CHANNEL_ATT_PVP;
			
			ResetList();
		}
		else if( IsCmdControl( "ID_RBT_TAB2" ) ) 
		{
			m_eChannelAttribute = CHANNEL_ATT_DARKLAIR;
			ResetList();
		}
		else if( IsCmdControl( "ID_RBT_TAB3" ) ) 
		{
			m_eChannelAttribute = CHANNEL_ATT_FARMTOWN;
			ResetList();
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnChannelDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		SendReqMoveChannelList();

		if( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHATROOM_DIALOG ) && GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHATROOM_DIALOG )->IsShow() )
			GetInterface().GetMainMenuDialog()->ShowChatRoomCreateDlg( false );
	}

	CDnChannelListDlg::Show( bShow );
}

void CDnChannelDlg::Process( float fElapsedTime )
{
	CDnChannelListDlg::Process( fElapsedTime );
}

void CDnChannelDlg::SendChannel()
{
	SListBoxItem *pItem = m_pChannelList->GetSelectedItem();
	if( !pItem ) return;

	CDnChannelListDlg::SChannelInfo *pInfo = (CDnChannelListDlg::SChannelInfo*)(pItem->pData);
	if( CGlobalInfo::GetInstance().m_cLocalAccountLevel == 0 && pInfo->m_nCongestion == HIGH_CONGESTION )
	{
		GetInterface().MessageBox( 97, MB_OK );
		return;
	}
/*
	//Start processing if in a party 
	CDnPartyTask* pPartyTask  = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));

	if( pPartyTask != NULL && pPartyTask->GetPartyState() != CDnPartyTask::PartyStateEnum::SINGLE )
	{
		//blondymarry start compare whether target channel is arena town or normal town: no warning between arenas, show message box for normal->arena
		CDnVillageTask *pTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );

		if( pTask != NULL )
		{
if( !pTask->GetPVPType())//Current town is not a PVP town
			{
if( pInfo->m_nChannelType == 1 )//Target town is a PVP town
				{
GetInterface().MessageBox(L"Temp: Moving to a PVP channel will leave your party." , MB_OK ,MESSAGEBOX_36);
				}			
			}
		}

	}	
*/
//blondymarry end

	// If there is a pending trade request, cancel it and request the move packet.
	GetTradeTask().GetTradePrivateMarket().ClearTradeUserInfoList();
	GetInterface().CloseAcceptRequestDialog(true);
	GetInterface().DisableCashShopMenuDlg(true);

	if( CDnChatRoomTask::IsActive() && GetChatRoomTask().IsRequestWait() )
		return;

	SendReqMoveSelectChannel(pInfo->m_nChannelID );
}

void CDnChannelDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	/*
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_YES" ) ) 
		{
			switch( nID )
			{
			    case MESSAGEBOX_37:
					//Leave party
					GetPartyTask().ReqOutParty();
					//Move
					SendReqMoveSelectChannel(m_nChannelID);
					break;
			}
		}
	}
	*/
}