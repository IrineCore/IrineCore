#include "StdAfx.h"
#include "DnTradePrivateMarket.h"
#include "DnInterface.h"
#include "DnPrivateMarketDlg.h"
#include "DnItemTask.h"
#include "TradeSendPacket.h"
#include "DnBridgeTask.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#include "DnLocalPlayerActor.h"
#include "DnChatTabDlg.h"
#include "DnChatOption.h"
#include "DnInventory.h"
#include "DnCharInventory.h"
#include "DNIsolate.h"
#include "DnRestraintTask.h"
#include "DnChatRoomDlg.h"
#include "DnAcceptRequestDlg.h"
#ifdef PRE_ADD_SECONDARY_SKILL
#include "DnLifeSkillCookingTask.h"
#endif // PRE_ADD_SECONDARY_SKILL



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnTradePrivateMarket::CDnTradePrivateMarket(void)
	: m_dwCurTradeUserID(0)
	, m_dwReadyTradeUserID(0)
	, m_pPrivateMarketDialog(NULL)
	, m_nChatMode(CHAT_NORMAL)
{
	m_vecTradePlayerItem.resize(10);
}

CDnTradePrivateMarket::~CDnTradePrivateMarket(void)
{
	ClearTradePlayerItem();
}

void CDnTradePrivateMarket::AddTradePlayerItem( int nIndex, TItemInfo &Item )
{
	if( nIndex < 0 ) return;
	if( nIndex >= (int)m_vecTradePlayerItem.size() ) return;
	CDnItem *pItem = GetItemTask().CreateItem( Item );
	if( pItem )
	{
		m_vecTradePlayerItem[nIndex] = pItem;
		CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );
	}
}

void CDnTradePrivateMarket::RemoveTradePlayerItem( int nIndex )
{
	if( nIndex < 0 ) return;
	if( nIndex >= (int)m_vecTradePlayerItem.size() ) return;
	SAFE_DELETE( m_vecTradePlayerItem[nIndex] );
}

void CDnTradePrivateMarket::ClearTradePlayerItem()
{
	SAFE_DELETE_PVEC( m_vecTradePlayerItem );
}

void CDnTradePrivateMarket::RequestPrivateMarket( UINT nSessionID )
{
	if( nSessionID <= 0 )
	{
		CDebugSet::ToLogFile( "CDnTradePrivateMarket::RequestPrivateMarket, Invalid SessionID(%d)!", nSessionID );
		return;
	}

	// Check trade restriction
	if( GetRestraintTask().CheckRestraint( _RESTRAINTTYPE_TRADE, true ) )
		return;
	
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( nSessionID );
	if( !hActor )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3601 ), MB_OK );	// UISTRING : User does not exist.
		return;
	}

	if (GetIsolateTask().IsBlackList(hActor->GetName()))
	{
		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3623 ), MB_OK);	// UISTRING : Cannot request a trade with a blocked character.
		return;
	}

	AddTradeUserInfo( true, nSessionID );

	//wchar_t wszTemp[256] = {0};
	//swprintf_s( wszTemp, _countof(wszTemp), L"%s %s", hActor->GetName(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3602 ) );
	//if( !GetInterface().OpenPrivateMarketReqDialog( wszTemp, 10.0f, PRIVATE_MARKET_REQ_DIALOG, this ) )
	//	return;

	SendExchangeRequest( nSessionID, false );
}

void CDnTradePrivateMarket::RequestPrivateMarketAccept( bool bAccept, UINT nSessionID )
{
#ifdef PRE_FIX_49403
	// #49403 In a specific sequence, cannot board the airship even after spending money. 
	// Block trade acceptance when blind window is open.
	if( GetInterface().IsOpenBlind() )
		bAccept = false;
#endif	

	// Note : Accept trade
	//
	SendExchangeAccept( bAccept, nSessionID );
}

void CDnTradePrivateMarket::RequestPrivateMarketAddItem( int nSlotIndex, int nInvenIndex, int nCount, INT64 biSerial )
{
	// Note : Add item
	//
	ASSERT( nCount>0 );
	SendExchangeAddItem( nSlotIndex, nInvenIndex, nCount, biSerial );
}

void CDnTradePrivateMarket::RequestPrivateMarketDeleteItem( int nIndex )
{
	// Note : Delete item
	//
	ASSERT( nIndex>=0 );
	SendExchangeDeleteItem( nIndex );
}

void CDnTradePrivateMarket::RequestPrivateMarketCoin( INT64 nCoin )
{
	// Note : Enter coins
	//
	ASSERT( nCoin>=0 );
	SendExchangeAddCoin( nCoin );
}

void CDnTradePrivateMarket::RequestPrivateMarketConfirm( char cType )
{
	SendExchangeConfirm( cType );
}

void CDnTradePrivateMarket::RequestPrivateMarketCancel()
{
	SendExchangeCancel();
}

void CDnTradePrivateMarket::OnRecvPrivateMarket( SCExchangeRequest *pPacket )
{
	// Note : Trade request received.
	//
	if( pPacket->nRet == ERROR_NONE )
	{
		bool bAccept(true);

		if( m_dwCurTradeUserID > 0 )
		{
			// Note : Currently trading. Must send a message saying currently in trade.
			//
			bAccept = false;
		}
		else if( m_dwReadyTradeUserID > 0 )
		{
			// Note : If requesting a trade or the trade accept-decline confirmation dialog is open,
			//
			bAccept = false;
		}
		else if( GetInterface().GetChatRoomDlg()->IsShow() )
		{
			// If the chat room is visible,
			// Check whether the user currently belongs to the chat room.
			if( !GetInterface().GetChatRoomDlg()->IsMyChatRoomMember( pPacket->nSenderSessionID ) )
				bAccept = false;
		}
		else if( GetInterface().IsShowMapMoveCashItemDlg() )
		{
			// Cannot trade while village warp item dialog is open.
			bAccept = false;
		}
		else if( GetInterface().IsShowGateReady() )
		{
			bAccept = false;
		}
		else if( CDnLocalPlayerActor::IsLockInput() )
		{
			bAccept = false;
		}
		else if( GetInterface().IsOpenBlind() )
		{
			bAccept = false;
		}
#ifdef PRE_ADD_CHANGEJOB_CASHITEM
		else if (GetInterface().IsShowChangeJobDialog())
		{
			bAccept = false;
		}
#endif
		else if( GetItemTask().IsRequestWait() )
		{
			bAccept = false;
		}
#ifdef PRE_ADD_SECONDARY_SKILL
		else if( CDnLifeSkillCookingTask::IsActive() && ( GetLifeSkillCookingTask().IsNowCooking() || GetLifeSkillCookingTask().IsRequestCooking() ) )
		{
			bAccept = false;
		}
#endif // PRE_ADD_SECONDARY_SKILL
		if( bAccept == false )
		{
			RequestPrivateMarketAccept( false, pPacket->nSenderSessionID );
			return;
		}
#ifdef PRE_ADD_SECONDARY_SKILL
		GetInterface().CloseCookingDialog();
#endif // PRE_ADD_SECONDARY_SKILL
		AddTradeUserInfo( false, pPacket->nSenderSessionID );
	}
	else if( pPacket->nRet == ERROR_EXCHANGE_SENDERCANCEL )
	{
		// As in issue #19668, when A requests B and A clicks cancel right after B accepts before receiving response
		// After a trade succeeds, ERROR_EXCHANGE_SENDERCANCEL is sent to B.
		// If in this state, cancel the trade.
		if( m_dwCurTradeUserID > 0 )
		{
			RequestPrivateMarketCancel();
			return;
		}

		// Note : Trade request canceled.
		//
		std::wstring wszTradeUserName;
		GetTradeUserName( pPacket->nSenderSessionID, wszTradeUserName );
		bool bRet = RemoveTradeUserInfo( false, pPacket->nSenderSessionID );
		m_dwReadyTradeUserID = 0;

		// If accept was clicked and the trade window opened before receiving the partner's cancel packet,
		// Close the window and initialize the variables below.
		m_dwCurTradeUserID = 0;

		// Show message on screen only when successfully deleted from list.
		if( bRet )
		{
			wchar_t wszTemp[256] = {0};
			swprintf_s( wszTemp, _countof(wszTemp), L"%s %s", wszTradeUserName.c_str(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3604 ) );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp );
		}
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet);

		// Enters here if /trade is requested on a ghost.
		RemoveTradeUserInfo( true, pPacket->nSenderSessionID );
	}
}

void CDnTradePrivateMarket::OnRecvPrivateMarketStart( SCExchangeStart *pPacket )
{
	std::wstring wszTradeUserName;
	GetTradeUserName( pPacket->nTargetSessionID, wszTradeUserName );
	RemoveTradeUserInfo( true, pPacket->nTargetSessionID );

	if( wszTradeUserName.empty() )
		wszTradeUserName = m_wszTradeUserName;

	// Note : Trade has started. Open the window.
	//
	// If the chat room is currently visible, there should be no need to do this.
	bool bStopAll = true;
	if( GetInterface().GetChatRoomDlg() && GetInterface().GetChatRoomDlg()->IsShow() )
		bStopAll = false;
	if( bStopAll )
		CDnLocalPlayerActor::StopAllPartyPlayer();

	if( pPacket->nRet == ERROR_NONE )
	{
		m_dwCurTradeUserID = pPacket->nTargetSessionID;

		// Trade is complete, so reset the list.
		ClearTradeUserInfoList();

		// Open the trade window,
		CDnMainMenuDlg *pMenuDlg = (CDnMainMenuDlg*)GetInterface().GetMainMenuDialog();
		if( pMenuDlg ) pMenuDlg->ShowPrivateMarketDialog( true );

		CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
		if( pInvenDlg )
			pInvenDlg->SetPrivateMarketName( CDnActor::s_hLocalActor->GetName(), wszTradeUserName.c_str() );

		// When trade starts, remember the current trade mode (do not remember whispers)
		m_nChatMode = GetInterface().GetChatDialog()->GetChatMode();
		
		// Switch default chat to whisper to the target.
		GetInterface().GetChatDialog()->SetPrivateName( wszTradeUserName );
	}
	else
	{
#ifdef PRE_FIX_CANCELTRADE
		ClearTradeUserInfoList(true);
#endif
		GetInterface().ServerMessageBox(pPacket->nRet);
		m_dwCurTradeUserID = 0;
	}

	//GetInterface().ClosePrivateMarketReqDialog();
	m_dwReadyTradeUserID = 0;
}

void CDnTradePrivateMarket::OnRecvPrivateMarketAddItem( SCExchangeAddItem *pPacket )
{
	// Note : Opponent added an item.
	//
	if( pPacket->nRet == ERROR_NONE )
	{
		if( pPacket->nSessionID == m_dwCurTradeUserID )
		{
			RemoveTradePlayerItem( pPacket->cExchangeIndex );
			AddTradePlayerItem( pPacket->cExchangeIndex, pPacket->ItemInfo );
			m_pPrivateMarketDialog->OnAddItem_Other( pPacket->cExchangeIndex, m_vecTradePlayerItem[pPacket->cExchangeIndex] );
		}
	}
	else
		GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnTradePrivateMarket::OnRecvPrivateMarketDeleteItem( SCExchangeDeleteItem *pPacket )
{
	// Note : Opponent removed an item.
	//
	if( pPacket->nRet == ERROR_NONE )
	{
		if( pPacket->nSessionID == m_dwCurTradeUserID )
		{
			RemoveTradePlayerItem( pPacket->cExchangeIndex );
			m_pPrivateMarketDialog->OnDelItem_Other( pPacket->cExchangeIndex );
		}
	}
	else
		GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnTradePrivateMarket::OnRecvPrivateMarketCoin( SCExchangeAddCoin *pPacket )
{
	// Note : Opponent changed coins.
	//
	if( pPacket->nRet == ERROR_NONE )
	{
		if( pPacket->nSessionID == m_dwCurTradeUserID )
		{
			m_pPrivateMarketDialog->OnUpdateMoney_Other( pPacket->nCoin );
		}
	}
	else
		GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnTradePrivateMarket::OnRecvPrivateMarketConfirm( SCExchangeConfirm *pPacket )
{
	// Note : Opponent's trade confirm/cancel
	//
	if( pPacket->nSessionID == m_dwCurTradeUserID )
	{
		switch( pPacket->cType )
		{
		case EXCHANGE_REGIST:
			m_pPrivateMarketDialog->OnClickButtonRegist_Other();
			break;
		case EXCHANGE_CONFIRM:
			m_pPrivateMarketDialog->OnClickButtonConfirm_Other();
			break;
		default:
			CDebugSet::ToLogFile( "CDnTradePrivateMarket::OnRecvPrivateMarketConfirm, default case!" );
			break;
		}
	}
}

void CDnTradePrivateMarket::OnRecvPrivateMarketComplete( SCExchangeComplete *pPacket )
{
	if( pPacket->nRet != ERROR_NONE )
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
	else
	{
		wchar_t wszTemp[256] = {0};
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4111 ) );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp );
	}

	// Note : Trade has completely ended, so close the window.
	//
	CDnMainMenuDlg *pMenuDlg = (CDnMainMenuDlg*)GetInterface().GetMainMenuDialog();
	if( pMenuDlg )
	{
		pMenuDlg->ShowPrivateMarketDialog( false, pPacket->nRet != ERROR_NONE );
	}

	// Mark items placed in the trade window as new after a successful trade.
	// Items in PrivateMarketDlg are only items present in the trade window.
	//CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
	//if( pInvenDlg ) pInvenDlg->SetPrivateMarketNewGain();
	//SetNewGainTradeList();

	// When trade ends, revert to the previous chat mode, but leave it alone if changed during the trade.
	DnActorHandle hTarget = CDnActor::FindActorFromUniqueID( m_dwCurTradeUserID );
	if( hTarget )
	{
		if( GetInterface().GetChatDialog()->GetChatMode() == CHAT_PRIVATE &&
			GetInterface().GetChatDialog()->GetPrivateName() == hTarget->GetName() )
		{
			GetInterface().GetChatDialog()->SetChatMode( m_nChatMode );
		}
	}

	m_dwCurTradeUserID = 0;
	m_dwReadyTradeUserID = 0;
}

void CDnTradePrivateMarket::OnRecvPrivateMarketCancel()
{
	// Note : Trade was canceled.
	//

	// There is one check that needs to be done.
	// If both trading users press Cancel at the same time, whichever cancel request is processed first will succeed, but
	// For the next processed request, a message box saying user not found will appear in the get-actor section below.
	// To prevent this, check first whether there is a user currently trading.
	if( !m_dwCurTradeUserID )
		return;
	
	// Close the window and show a message box.
	//
	CDnMainMenuDlg *pMenuDlg = (CDnMainMenuDlg*)GetInterface().GetMainMenuDialog();
	if( pMenuDlg )
	{
		pMenuDlg->ShowPrivateMarketDialog( false );
		pMenuDlg->CloseInvenDialog();
	}

	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( m_dwCurTradeUserID );
	if( !hActor )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3601 ), MB_OK );
	}
	else {
		wchar_t wszTemp[256] = {0};
		swprintf_s( wszTemp, _countof(wszTemp), L"%s %s", hActor->GetName(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3605 ) );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp );

		// When trade ends, revert to the previous chat mode, but leave it alone if changed during the trade.
		DnActorHandle hTarget = CDnActor::FindActorFromUniqueID( m_dwCurTradeUserID );
		if( hTarget )
		{
			if( GetInterface().GetChatDialog()->GetChatMode() == CHAT_PRIVATE &&
				GetInterface().GetChatDialog()->GetPrivateName() == hTarget->GetName() )
			{
				GetInterface().GetChatDialog()->SetChatMode( m_nChatMode );
			}
		}
	}

	m_dwCurTradeUserID = 0;
	m_dwReadyTradeUserID = 0;
}

void CDnTradePrivateMarket::OnRecvPrivateMarketReject( SCExchangeReject *pPacket )
{
	GetInterface().ClosePrivateMarketReqDialog();

	if( pPacket->nRetCode != ERROR_NONE )
	{
		// If return code is due to a sanction, the return code will contain an error code.

		// After removing since the request window needs to be dismissed
		std::wstring wszTradeUserName;
		GetTradeUserName( pPacket->nSessionID, wszTradeUserName );
		RemoveTradeUserInfo( true, pPacket->nSessionID );

		if( ERROR_GENERIC_LEVELLIMIT == pPacket->nRetCode )
		{
			WCHAR wszMessage[512];
			int iWeight = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::LimitLevel_Exchange);
			swprintf_s( wszMessage, _countof(wszMessage), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4168 ), iWeight );	// Trade request feature is available from level %d.

			GetInterface().MessageBox( wszMessage );

			return;
		}
		// Server error code message box
		GetInterface().ServerMessageBox(pPacket->nRetCode);
	}
	else
	{
		// Otherwise, it is a normal rejection.

		// Trade requests initiated by self are deleted directly in Process when timed out.
		// So there is no routine to check the RemoveTradeUserInfo return value and print a message.
		// However, the server has no specific way to know whether it is an actual trade decline or a decline on logout, so it ends up sending it twice.
		// Check first and then display a message.
		std::wstring wszTradeUserName;
		GetTradeUserName( pPacket->nSessionID, wszTradeUserName );
		bool bRet = RemoveTradeUserInfo( true, pPacket->nSessionID );

		if( bRet )
		{
			wchar_t wszTemp[256] = {0};
			swprintf_s( wszTemp, _countof(wszTemp), L"%s %s", wszTradeUserName.c_str(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3606 ) );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp );
		}
	}
}

void CDnTradePrivateMarket::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	switch( nID ) 
	{
	case ACCEPT_DIALOG:
		{
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
				{
					ClearTradeUserInfoList();
					RequestPrivateMarketAccept( true, m_dwReadyTradeUserID );

					m_dwCurTradeUserID = m_dwReadyTradeUserID;
				}
				else if( (strcmp( pControl->GetControlName(), "ID_CANCEL" ) == 0) )
				{
					RequestPrivateMarketAccept( false, m_dwReadyTradeUserID );
				}

				m_dwReadyTradeUserID = 0;
				GetInterface().CloseAcceptDialog();
			}
		}
		break;
	case PRIVATE_MARKET_REQ_DIALOG:
		{
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( (strcmp( pControl->GetControlName(), "ID_CANCEL" ) == 0) )
				{
					// Note : Must send request cancellation to server here.
					//
					SendExchangeRequest( m_dwReadyTradeUserID, true );
					m_dwReadyTradeUserID = 0;
				}

				GetInterface().ClosePrivateMarketReqDialog();
			}
		}
		break;
	case ACCEPT_REQUEST_DIALOG:
		{
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
				{
					// First remove this request from the list, then
					GetTradeUserName( m_dwReadyTradeUserID, m_wszTradeUserName );	// Temporary storage
					RemoveTradeUserInfo( false, m_dwReadyTradeUserID );

					// Check if the user is nearby.
					DnActorHandle hTarget = CDnActor::FindActorFromUniqueID( m_dwReadyTradeUserID );
					if( hTarget )
					{
						// Cancel all other requests and then send accept packet
						ClearTradeUserInfoList();
						RequestPrivateMarketAccept( true, m_dwReadyTradeUserID );
						m_dwCurTradeUserID = m_dwReadyTradeUserID;
					}
					else
					{
						// Must send a reject packet so the server can clear the state of both clients together.
						RequestPrivateMarketAccept( false, m_dwReadyTradeUserID );
						m_dwReadyTradeUserID = 0;
						GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3601 ), MB_OK );
					}
				}
				else if( (strcmp( pControl->GetControlName(), "ID_REJECT" ) == 0) )
				{
					// Send rejection packet regardless of whether the target is nearby.
					RemoveTradeUserInfo( false, m_dwReadyTradeUserID );
					RequestPrivateMarketAccept( false, m_dwReadyTradeUserID );
					m_dwReadyTradeUserID = 0;
				}
				else if( (strcmp( pControl->GetControlName(), "ID_CANCEL" ) == 0) )
				{
					// Send request cancellation to the server regardless of whether the target is nearby.
					RemoveTradeUserInfo( true, m_dwReadyTradeUserID );
					SendExchangeRequest( m_dwReadyTradeUserID, true );
					m_dwReadyTradeUserID = 0;
				}

				GetInterface().CloseAcceptRequestDialog();
			}
		}
		break;
}
}

#ifdef PRE_FIX_CANCELTRADE
void CDnTradePrivateMarket::ClearTradeUserInfoList(bool bClearOnly)
#else
void CDnTradePrivateMarket::ClearTradeUserInfoList()
#endif
{
#ifdef PRE_FIX_CANCELTRADE
	if (bClearOnly)
{
		m_listTradeUserInfo.clear();
		return;
	}
#endif
	LIST_TRADEUSERINFO_ITER iter = m_listTradeUserInfo.begin();
	for( ; iter != m_listTradeUserInfo.end(); ++iter )
	{
		//RequestPrivateMarketAccept( false, iter->m_dwSessionID );

		if( iter->m_bSelfToTarget == true )
		{
			// In principle, send when forcibly removed.
			SendExchangeRequest( iter->m_dwSessionID, true );
		}
		else
		{
			RequestPrivateMarketAccept( false, iter->m_dwSessionID );
		}
	}

	m_listTradeUserInfo.clear();
}

void CDnTradePrivateMarket::AddTradeUserInfo( bool bSelfToTarget, DWORD nSessionID )
{
	STradeUserInfo sTradeUserInfo;
	sTradeUserInfo.m_bSelfToTarget = bSelfToTarget;
	sTradeUserInfo.m_dwSessionID = nSessionID;

	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( nSessionID );
	if( !hActor ) return;
	sTradeUserInfo.m_strName = hActor->GetName();
	sTradeUserInfo.m_fAcceptTime = 10.0f;

	m_listTradeUserInfo.push_back( sTradeUserInfo );
}

bool CDnTradePrivateMarket::RemoveTradeUserInfo( bool bSelfToTarget, DWORD nSessionID )
{
	LIST_TRADEUSERINFO_ITER iter = m_listTradeUserInfo.begin();
	for( ; iter != m_listTradeUserInfo.end(); ++iter )
	{
		if( iter->m_bSelfToTarget == bSelfToTarget && iter->m_dwSessionID == nSessionID )
		{
			//m_listTradeUserInfo.pop_front();
			m_listTradeUserInfo.erase( iter );
			return true;
		}
	}

	CDebugSet::ToLogFile( "CDnTradePrivateMarket::RemoveTradeUserInfo, Invalid Session ID(%d)", nSessionID );
	return false;
}

bool CDnTradePrivateMarket::GetTradeUserName( DWORD nSessionID, std::wstring &wszName )
{
	LIST_TRADEUSERINFO_ITER iter = m_listTradeUserInfo.begin();
	for( ; iter != m_listTradeUserInfo.end(); ++iter )
	{
		if( iter->m_dwSessionID == nSessionID )
		{
			wszName = iter->m_strName;
			return true;
		}
	}
	return false;
}

bool CDnTradePrivateMarket::IsRequestUser( DWORD nSessionID )
{
	LIST_TRADEUSERINFO_ITER iter = m_listTradeUserInfo.begin();
	for( ; iter != m_listTradeUserInfo.end(); ++iter )
	{
		// Does not matter whether I initiated it or a request came to me.
		if( iter->m_dwSessionID == nSessionID )
		{
			return true;
		}
	}

	return false;
}

bool CDnTradePrivateMarket::IsRequesting()
{
	LIST_TRADEUSERINFO_ITER iter = m_listTradeUserInfo.begin();
	for( ; iter != m_listTradeUserInfo.end(); ++iter )
	{
		// Does not care whom it was cast on.
		if( iter->m_bSelfToTarget == true )
		{
			return true;
		}
	}

	return false;
}

bool CDnTradePrivateMarket::IsAccepting()
{
	LIST_TRADEUSERINFO_ITER iter = m_listTradeUserInfo.begin();
	for( ; iter != m_listTradeUserInfo.end(); ++iter )
	{
		// Does not care who cast it on me.
		if( iter->m_bSelfToTarget == false )
		{
			return true;
		}
	}

	return false;
}

void CDnTradePrivateMarket::Process( float fElapsedTime )
{
	// Send rejection for all actions during event and shop usage.
	if( GetInterface().IsOpenBlind() )
		ClearTradeUserInfoList();

	// Should also include channel move.

	// All timing handling for each request is done here; the dialog only displays the processed values (such as time).
	LIST_TRADEUSERINFO_ITER iter = m_listTradeUserInfo.begin();
	for( ; iter != m_listTradeUserInfo.end(); )
	{
		iter->m_fAcceptTime -= fElapsedTime;

		if( iter->m_fAcceptTime <= 0.0f )
		{
			if( iter->m_bSelfToTarget == true )
			{
				// Do not send a request cancel when my request time has already expired.
				//SendExchangeRequest( iter->m_dwSessionID, true );

				// This already contains a different value from when multiple requests were sent.
				//m_dwRequestTradeUserID = 0;

				// Closes automatically when time expires.
				//GetInterface().ClosePrivateMarketReqDialog();

				// Since it will come back as a rejection, do not delete it directly.
				//iter = m_listTradeUserInfo.erase( iter );
				//continue;
			}
			else
			{
				RequestPrivateMarketAccept( false, iter->m_dwSessionID );
				iter = m_listTradeUserInfo.erase( iter );
				continue;
			}
		}

		++iter;
	}

	// For verification
	if( m_listTradeUserInfo.empty() )
		m_dwReadyTradeUserID = 0;
}

float CDnTradePrivateMarket::GetShortestAcceptTime()
{
	if( !m_listTradeUserInfo.empty() )
		return m_listTradeUserInfo.begin()->m_fAcceptTime;
	else
		return _INVALID_TIME_VALUE;
}

void CDnTradePrivateMarket::OpenAcceptRequestDialog()
{
	LIST_TRADEUSERINFO_ITER iterInner = m_listTradeUserInfo.begin();

	wchar_t wszTemp[256] = {0};
	if( iterInner->m_bSelfToTarget == true )
	{
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3602), iterInner->m_strName.c_str());	// UISTRING : <html><br><div align=\\\"center\\\">Requesting trade with [%s].</div></html>
		GetInterface().OpenAcceptRequestDialog( wszTemp, _REQUEST_ACCEPT_TOTALTIME, iterInner->m_fAcceptTime, false, CDnAcceptRequestDlg::eType01, ACCEPT_REQUEST_DIALOG, this );
		m_dwReadyTradeUserID = iterInner->m_dwSessionID;
	}
	else
	{
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3607), iterInner->m_strName.c_str());	// UISTRING : <html><br><div align=\\\"center\\\">[%s] has requested a trade.</div></html>
		GetInterface().OpenAcceptRequestDialog( wszTemp, _REQUEST_ACCEPT_TOTALTIME, iterInner->m_fAcceptTime, true, CDnAcceptRequestDlg::eType01, ACCEPT_REQUEST_DIALOG, this );
		m_dwReadyTradeUserID = iterInner->m_dwSessionID;
	}
}

//void CDnTradePrivateMarket::SetNewGainTradeList()
//{
//	DWORD dwVecSize = (DWORD)m_vecTradePlayerItem.size();
//	for( DWORD i=0; i<dwVecSize; i++ )
//	{
//		if( m_vecTradePlayerItem[i] )
//		{
//			// Mark New on matching items inside the inventory.
//			CDnItem *pItem = GetItemTask().GetCharInventory().GetItemFromSerialID( m_vecTradePlayerItem[i]->GetSerialID() );
//			if( pItem )
//				pItem->SetNewGain( true );
//		}
//	}
//}


#ifdef PRE_FIX_49403
void CDnTradePrivateMarket::SendExchangeRequestCancel( DWORD nSessionID )
{
	SendExchangeRequest( nSessionID, true );
}
#endif
