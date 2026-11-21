#include "StdAfx.h"
#include "DnCashShopMenuDlg.h"
#include "DnInterface.h"
#include "DnCashShopTask.h"
#include "DnPartyTask.h"
#include "DnLocalPlayerActor.h"
#include "DnTradeTask.h"
#include "DnTradePrivateMarket.h"
#include "DnLoadingTask.h"
#include "ChatRoomSendPacket.h"
#include "DnChatRoomDlg.h"
#include "DnMainMenuDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
CDnCashShopMenuDlg::CDnCashShopMenuDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, false )
{
	m_pBtn = NULL;
	m_bDisable = false;
}

CDnCashShopMenuDlg::~CDnCashShopMenuDlg()
{
}

void CDnCashShopMenuDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CashShopDlg.ui" ).c_str(), bShow );
}

void CDnCashShopMenuDlg::InitialUpdate()
{
	m_pBtn = GetControl<CEtUIButton>("ID_BUTTON0");
}

void CDnCashShopMenuDlg::Show( bool bShow )
{
	if (bShow)
		m_bDisable = false;

	if (m_bShow == bShow)
		return;

	CEtUIDialog::Show(bShow);
}

void CDnCashShopMenuDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_BUTTON0"))
		{
			//GetInterface().ShowCashShopDlg(true);
			
			// If Cash Shop open conditions change, pressing the coin purchase button in the Japanese Gachapon dialog also opens the Cash Shop 
			// Therefore that part must also be modified together. (ID_BUTTON_BUYCOIN handling in CDnGachaDlg::ProcessCommand)
			// - Cold air
			if (CDnInterface::GetInstance().GetChatRoomDlg() && CDnInterface::GetInstance().GetChatRoomDlg()->IsShow())
			{
				if( CDnInterface::GetInstance().GetChatRoomDlg()->IsMaster() && CDnInterface::GetInstance().GetChatRoomDlg()->IsEmptyRoom() == false )
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8137 ), MB_YESNO, MESSAGEBOX_EXITCHATROOM, this, true, true );
				else
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8134 ), MB_YESNO, MESSAGEBOX_EXITCHATROOM, this );
			}
			else
			{
				if (CDnCashShopTask::IsActive())
					CDnCashShopTask::GetInstance().RequestCashShopOpen();
			}

			return ;
		}
	}
	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnCashShopMenuDlg::Process( float fElapsedTime )
{
	if (CDnActor::s_hLocalActor && CDnPartyTask::IsActive())
	{
		CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());

		bool bEnable = true;
		if (CDnPartyTask::GetInstance().IsLocalActorEnterGateReady() || 
			pPlayer->IsAutoRun() ||
			GetInterface().IsShowPrivateMarketDlg()
			|| GetInterface().IsShowCostumeMixDlg()
			|| GetInterface().IsShowCostumeDesignMixDlg()
#ifdef PRE_ADD_COSRANDMIX
			|| GetInterface().IsShowCostumeRandomMixDlg()
#endif
			|| m_bDisable
			|| (GetInterface().GetMainMenuDialog(CDnMainMenuDlg::CHATROOM_DIALOG) && GetInterface().GetMainMenuDialog(CDnMainMenuDlg::CHATROOM_DIALOG)->IsShow())
			|| pPlayer->IsFollowing()
			|| CDnPartyTask::GetInstance().IsPartyProcessState(CDnPartyTask::PPS_ACCEPT_INVITE)
			|| (GetInterface().IsShowChannelDlg())
			)
		{
			bEnable = false;
		}

		if (CDnTradeTask::IsActive() && GetTradeTask().GetTradePrivateMarket().IsRequesting())
			bEnable = false;

		if (CDnPartyTask::GetInstance().IsPartyAcceptRequesting())
			bEnable = false;

		m_pBtn->Enable(bEnable);
	}

	CEtUIDialog::Process(fElapsedTime);
}

bool CDnCashShopMenuDlg::IsEnableButton()
{
	if( !m_pBtn ) return false;
	return m_pBtn->IsEnable();
}

void CDnCashShopMenuDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	if (pControl)
		SetCmdControlName( pControl->GetControlName() );

	switch( nID )
	{
	case MESSAGEBOX_EXITCHATROOM:
		{
			if (nCommand == EVENT_BUTTON_CLICKED)
			{
				if (IsCmdControl("ID_YES"))
				{
					SendLeaveChatRoom();
					if (CDnCashShopTask::IsActive())
						CDnCashShopTask::GetInstance().RequestCashShopOpen();
				}
			}
		}
		break;
	}
}