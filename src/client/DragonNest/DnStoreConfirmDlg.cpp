#include "StdAfx.h"
#include "DnStoreConfirmDlg.h"
#include "DnItem.h"
#include "DnInterfaceString.h"
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#include "TaskManager.h"
#include "DnQuestTask.h"
#include "DnRepStoreBenefitTooltipDlg.h"
#include "DnTableDB.h"
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnTradeTask.h"
#include "DnTradeItem.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnStoreConfirmDlg::CDnStoreConfirmDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
	, m_pItemSlotButton(NULL)
	, m_pItemAmount1(NULL)
	, m_pItemAmount2(NULL)
	, m_pItemText(NULL)
	, m_emConfirmType(CDnInterface::STORE_CONFIRM_SELL)
	, m_pItemName(NULL)
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	, m_pReputationBenefit( NULL )
	, m_pStoreBenefitTooltip( NULL )
	, m_iNowStoreBenefit( NpcReputation::StoreBenefit::Max )
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#ifdef PRE_MOD_SELL_SEALEDITEM
	, m_pStaticSellSealedItem(NULL)
#endif
{
}

CDnStoreConfirmDlg::~CDnStoreConfirmDlg(void)
{
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	SAFE_DELETE( m_pStoreBenefitTooltip );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
}

void CDnStoreConfirmDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "StoreConfirmDlg.ui" ).c_str(), bShow );
}

void CDnStoreConfirmDlg::InitialUpdate()
{
	m_pItemName = GetControl<CEtUIStatic>("ID_ITEM_NAME"); 
	m_pItemAmount1 = GetControl<CEtUIStatic>("ID_ITEM_AMOUNT");
	m_pItemAmount2 = GetControl<CEtUIStatic>("ID_ITEM_AMOUNT2");
	m_pItemText = GetControl<CEtUIStatic>("ID_STATIC_TEXT");
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	m_pReputationBenefit = GetControl<CEtUIStatic>("ID_ICON_REPUTE");
	m_pReputationBenefit->Show( false );

	m_pStoreBenefitTooltip = new CDnRepStoreBenefitTooltipDlg( UI_TYPE_CHILD, this );
	m_pStoreBenefitTooltip->Initialize( false );
#endif // PRE_ADD_NPC_REPUTATION_SYSTEM
#ifdef PRE_MOD_SELL_SEALEDITEM
	m_pStaticSellSealedItem = GetControl<CEtUIStatic>("ID_STATIC_TEXT4");
#endif

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_OK") );
}

void CDnStoreConfirmDlg::InitCustomControl( CEtUIControl *pControl )
{
	m_pItemSlotButton = static_cast<CDnItemSlotButton*>(pControl);
}

void CDnStoreConfirmDlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		m_SmartMove.MoveCursor();
	}
	else
	{
		m_SmartMove.ReturnCursor();

		m_pItemSlotButton->ResetSlot();
		m_pItemAmount1->ClearText();
		m_pItemAmount2->ClearText();
		m_pItemText->ClearText();

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
		m_pReputationBenefit->Show( false );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#ifdef PRE_MOD_SELL_SEALEDITEM
		m_pStaticSellSealedItem->Show( false );
#endif
	}
}

void CDnStoreConfirmDlg::SetItem( CDnItem *pItem, CDnInterface::emSTORE_CONFIRM_TYPE type )
{
	if( !pItem )
		return;

	m_emConfirmType = type;
	m_pItemSlotButton->SetItem(pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
	m_pItemName->SetTextColor( DN_INTERFACE::STRING::ITEM::RANK_2_COLOR( pItem->GetItemRank() ) );
	m_pItemName->SetText( pItem->GetName() );

	wchar_t wszTemp[128] = {0};
	int nAmount(0);

	if( m_emConfirmType == CDnInterface::STORE_CONFIRM_SELL )
	{
		m_pItemText->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1726 ) );
		nAmount = pItem->GetItemSellAmount();

		// Whether there is an affinity shop benefit.
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
			bool bAvailBenefit = GetQuestTask().CheckAndCalcStoreBenefit( NpcReputation::StoreBenefit::SellingPriceUp, nAmount );
			if( bAvailBenefit )
			{
				// Show an indicator in the item purchase window that the price is discounted by affinity.
				m_iNowStoreBenefit = NpcReputation::StoreBenefit::SellingPriceUp;
			}
			m_pReputationBenefit->Show( bAvailBenefit );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#ifdef PRE_MOD_SELL_SEALEDITEM
			m_pStaticSellSealedItem->Show( (pItem->GetReversion() > CDnItem::NoReversion && !pItem->IsSoulbBound()) );
#endif

		std::wstring strCoin;
		DN_INTERFACE::UTIL::GetMoneyFormatUseStr( nAmount, strCoin );
		swprintf_s( wszTemp, _countof(wszTemp), L"[%s]", strCoin.c_str() );
		m_pItemAmount1->SetText( wszTemp );
	}
	else if( m_emConfirmType == CDnInterface::STORE_CONFIRM_BUY )
	{
		m_pItemText->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1725 ) );

		CDnStoreTabDlg *pStoreDlg = (CDnStoreTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::STORE_DIALOG );

		if( pStoreDlg && pStoreDlg->IsShow() )
		{
			if (pStoreDlg->IsShowRepurchasableTab())
			{
				SetRepurchaseBuy(*pItem);
				return;
			}

			if (Shop::Type::Combined <= pStoreDlg->GetStoreType())
			{
				SetCombinedBuy( pItem, pStoreDlg->GetCurrentTabID(), 1 );
				return;
			}
		}

		// First check if a medal exists
		if( pItem->GetNeedBuyItem() && pItem->GetNeedBuyItemCount() )
		{
			std::wstring wszNeedItemName = CDnItem::GetItemFullName( pItem->GetNeedBuyItem() );
			swprintf_s( wszTemp, _countof(wszTemp), L"[%s %d%s]", wszNeedItemName.c_str(), pItem->GetNeedBuyItemCount(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1728 ) );
			m_pItemAmount1->SetText( wszTemp );

			nAmount = pItem->GetItemAmount();

			if( nAmount ) {

				// Whether there is an affinity shop benefit.
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
				bool bAvailBenefit = GetQuestTask().CheckAndCalcStoreBenefit( NpcReputation::StoreBenefit::BuyingPriceDiscount, nAmount );
				if( bAvailBenefit )
				{
					// Show an indicator in the item purchase window that the price is discounted by affinity.
					m_iNowStoreBenefit = NpcReputation::StoreBenefit::BuyingPriceDiscount;
				}
				m_pReputationBenefit->Show( bAvailBenefit );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

				std::wstring strCoin;
				DN_INTERFACE::UTIL::GetMoneyFormatUseStr( nAmount, strCoin );
				swprintf_s( wszTemp, _countof(wszTemp), L"[%s]", strCoin.c_str() );
				m_pItemAmount2->SetText( wszTemp );
			}
		}
		else if(pItem->GetNeedBuyLadderPoint())
		{
#ifdef PRE_FIX_LADDERSHOP_SKILL
			int nPoint = GetTradeTask().GetTradeItem().CalcPurchaseLadderPoint(pItem->GetNeedBuyLadderPoint());
			swprintf_s( wszTemp, _countof(wszTemp), L"[%d %s]", nPoint, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126176 ) );
			m_pItemAmount1->SetText( wszTemp );
#else
			swprintf_s( wszTemp, _countof(wszTemp), L"[%d %s]", pItem->GetNeedBuyLadderPoint(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126176 ) );
			m_pItemAmount1->SetText( wszTemp );
#endif
		}
		else if(pItem->GetNeedBuyGuildWarPoint())
		{
			swprintf_s( wszTemp, _countof(wszTemp), L"[%d %s]", pItem->GetNeedBuyGuildWarPoint(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126084 ) );
			m_pItemAmount1->SetText( wszTemp );
		}
		else if (pItem->GetNeedBuyUnionPoint())
		{
			swprintf_s( wszTemp, _countof(wszTemp), L"[%d %s]", pItem->GetNeedBuyUnionPoint(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3273 ) ); // UISTRING : Union Points
			m_pItemAmount1->SetText( wszTemp );

			if (pItem->GetItemType() == ITEMTYPE_UNION_MEMBERSHIP)
			{
				CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));	
				if( !pQuestTask ) 
					return;

				CReputationSystemRepository* pReputationRepos = pQuestTask->GetReputationRepository();
				if( !pReputationRepos )
					return;

				DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
				if( !pItemSox ) return;

				const std::vector<CDnInterface::SUnionMembershipData>& membershipList = pReputationRepos->GetUnionMembershipData();
				std::vector<CDnInterface::SUnionMembershipData>::const_iterator iter = membershipList.begin();

				bool bSameMembership = false;
				bool bSameUnionType = false;
				for (; iter != membershipList.end(); ++iter)
				{
					const CDnInterface::SUnionMembershipData& curData = (*iter);
					if (curData.itemId == pItem->GetClassID())
					{
						m_pItemText->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3274 ));
						bSameMembership = true;
						break;
					}

					if (!pItemSox->IsExistItem(pItem->GetClassID())) return;
					int nTypeParam = pItemSox->GetFieldFromLablePtr( pItem->GetClassID(), "_TypeParam3" )->GetInteger();
					if (curData.unionType == nTypeParam)
					{
						bSameUnionType = true;
						break;
					}
				}

				if (bSameMembership == false)
				{
					if (bSameUnionType)
						m_pItemText->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3272 ));
					else
						m_pItemText->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3270 ));
				}
			}
		}
		else
		{
			nAmount = pItem->GetItemAmount();
			
			// Whether there is an affinity shop benefit.
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
			bool bAvailBenefit = GetQuestTask().CheckAndCalcStoreBenefit( NpcReputation::StoreBenefit::BuyingPriceDiscount, nAmount );
			if( bAvailBenefit )
			{
				// Show an indicator in the item purchase window that the price is discounted by affinity.
				m_iNowStoreBenefit = NpcReputation::StoreBenefit::BuyingPriceDiscount;
			}
			m_pReputationBenefit->Show( bAvailBenefit );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

			std::wstring strCoin;
			DN_INTERFACE::UTIL::GetMoneyFormatUseStr( nAmount, strCoin );
			swprintf_s( wszTemp, _countof(wszTemp), L"[%s]", strCoin.c_str() );
			m_pItemAmount1->SetText( wszTemp );
		}
	}
}


bool CDnStoreConfirmDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	bool bRet = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

	switch( uMsg )
	{
		case WM_MOUSEMOVE:
			{
				CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));	
				if( !pQuestTask ) 
					return false;

				CReputationSystemRepository* pReputationRepos = pQuestTask->GetReputationRepository();
				if( !pReputationRepos )
					return false;

				float fMouseX = 0;
				float fMouseY = 0;
				POINT MousePoint;
				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );
				PointToFloat( MousePoint, fMouseX, fMouseY );

				bool bMouseEnter = false;
				float fReputationValue = 0.0f;
				float fReputationValueMax = 100.0f;

				ShowChildDialog( m_pStoreBenefitTooltip, false );
				if( m_pReputationBenefit->IsShow() )
				{
					if( m_pReputationBenefit->IsInside( fMouseX, fMouseY ) )
						bMouseEnter = true;

					if( !CDnMouseCursor::GetInstance().IsShowCursor() )
						bMouseEnter = false;				

					if( bMouseEnter )
					{
						if( CDnActor::s_hLocalActor )
						{
							int iBenefitValue = pQuestTask->GetStoreBenefitValue( m_iNowStoreBenefit );

							wchar_t awcBuffer[ 256 ] = { 0 };
							if( NpcReputation::StoreBenefit::BuyingPriceDiscount == m_iNowStoreBenefit )
								//swprintf_s( awcBuffer, L"Affinity effect active\nItem %d%% discount (temp text not in UIString)", iBenefitValue );
								swprintf_s( awcBuffer, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3218 ), iBenefitValue );
							else
							if( NpcReputation::StoreBenefit::SellingPriceUp == m_iNowStoreBenefit )
								//swprintf_s( awcBuffer, L"Affinity effect active\nItem sell price %d%% increase (temp text not in UIString)", iBenefitValue );
								swprintf_s( awcBuffer, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3217 ), iBenefitValue );

							m_pStoreBenefitTooltip->SetText( awcBuffer );
							ShowChildDialog( m_pStoreBenefitTooltip, true );

							SUICoord dlgCoord;
							m_pStoreBenefitTooltip->GetDlgCoord( dlgCoord );
							dlgCoord.fX = fMouseX + GetXCoord() - (dlgCoord.fWidth / 2.0f);
							dlgCoord.fY = fMouseY + GetYCoord() - dlgCoord.fHeight - 0.004f;
							m_pStoreBenefitTooltip->SetDlgCoord( dlgCoord );
						}
					}
				}
			}
			break;
	}
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

	return bRet;
}

void CDnStoreConfirmDlg::SetCombinedBuy( CDnItem * pItem, int nTabID, int nCount )
{
	CDnTradeItem::TCombinedShopTableData * pData = GetTradeTask().GetTradeItem().GetCombinedStoreItem( nTabID, pItem->GetSlotIndex() );

	WCHAR wszString[512], wszOrigin[512];
	int nLine = 0;
	for( int itr = 0; itr < Shop::Max::PurchaseType; ++itr )
	{
		wszOrigin[0] = NULL;
		if( Shop::PurchaseType::Gold == pData->PurchaseType[itr].PurchaseType )
		{
			int nMoney = pData->PurchaseType[itr].iPurchaseItemValue;
			int storeBenefitAmount = nMoney;
			std::wstring wszMoney;

			bool bAvailBenefit = GetQuestTask().CheckAndCalcStoreBenefit( NpcReputation::StoreBenefit::BuyingPriceDiscount, storeBenefitAmount );
#ifdef PRE_FIX_REPUTE_DISCOUNT
			if( bAvailBenefit )
			{
				// Show an indicator in the item purchase window that the price is discounted by affinity.
				m_iNowStoreBenefit = NpcReputation::StoreBenefit::BuyingPriceDiscount;
			}
			m_pReputationBenefit->Show( bAvailBenefit );
#endif

			DN_INTERFACE::UTIL::GetMoneyFormatUseStr( (bAvailBenefit ? storeBenefitAmount : nMoney) * nCount, wszMoney );
			swprintf_s( wszOrigin, _countof(wszOrigin), L"%s", wszMoney.c_str() );
		}
		else if( Shop::PurchaseType::ItemID == pData->PurchaseType[itr].PurchaseType )
		{
			std::wstring wszNeedItemName = CDnItem::GetItemFullName( pData->PurchaseType[itr].iPurchaseItemID );
			swprintf_s( wszOrigin, _countof(wszOrigin), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 80 ), wszNeedItemName.c_str(), pData->PurchaseType[itr].iPurchaseItemValue * nCount );	// UISTRING : %s x%d
		}
		else if( Shop::PurchaseType::LadderPoint == pData->PurchaseType[itr].PurchaseType )
		{
#ifdef PRE_FIX_LADDERSHOP_SKILL
			int nPoint = GetTradeTask().GetTradeItem().CalcPurchaseLadderPoint(pData->PurchaseType[itr].iPurchaseItemValue);
			swprintf_s( wszOrigin, _countof(wszOrigin), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1769 ), nPoint * nCount );	// UISTRING : %d Ladder Points
#else
			swprintf_s( wszOrigin, _countof(wszOrigin), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1769 ), pData->PurchaseType[itr].iPurchaseItemValue * nCount );	// UISTRING : %d Ladder Points
#endif
		}
		else if( Shop::PurchaseType::GuildPoint == pData->PurchaseType[itr].PurchaseType )
		{
			swprintf_s( wszOrigin, _countof(wszOrigin), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1771 ), pData->PurchaseType[itr].iPurchaseItemValue * nCount );	// UISTRING : %d Guild Festival Points
		}
		else if( Shop::PurchaseType::UnionPoint== pData->PurchaseType[itr].PurchaseType )
		{
			swprintf_s( wszOrigin, _countof(wszOrigin), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1770 ), pData->PurchaseType[itr].iPurchaseItemValue * nCount );	// UISTRING : %d Union Points

			if (pItem->GetItemType() == ITEMTYPE_UNION_MEMBERSHIP)
			{
				CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));	
				if( !pQuestTask ) 
					return;

				CReputationSystemRepository* pReputationRepos = pQuestTask->GetReputationRepository();
				if( !pReputationRepos )
					return;

				DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
				if( !pItemSox ) return;

				const std::vector<CDnInterface::SUnionMembershipData>& membershipList = pReputationRepos->GetUnionMembershipData();
				std::vector<CDnInterface::SUnionMembershipData>::const_iterator iter = membershipList.begin();

				bool bSameMembership = false;
				bool bSameUnionType = false;
				for (; iter != membershipList.end(); ++iter)
				{
					const CDnInterface::SUnionMembershipData& curData = (*iter);
					if (curData.itemId == pItem->GetClassID())
					{
						m_pItemText->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3274 ));
						bSameMembership = true;
						break;
					}

					if (!pItemSox->IsExistItem(pItem->GetClassID())) return;
					int nTypeParam = pItemSox->GetFieldFromLablePtr( pItem->GetClassID(), "_TypeParam3" )->GetInteger();
					if (curData.unionType == nTypeParam)
					{
						bSameUnionType = true;
						break;
					}
				}

				if (bSameMembership == false)
				{
					if (bSameUnionType)
						m_pItemText->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3272 ));
					else
						m_pItemText->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3270 ));
				}
			}
		}

		if( NULL != wszOrigin[0] )
		{
			swprintf_s( wszString, _countof(wszString), L"[%s]", wszOrigin );
			if( 0 == nLine )
				m_pItemAmount1->SetText( wszString );
			else
				m_pItemAmount2->SetText( wszString );

			++nLine;
		}
	}
}

void CDnStoreConfirmDlg::SetRepurchaseBuy(const CDnItem& item)
{
	const CDnStoreRepurchaseSystem& rs = GetTradeTask().GetTradeItem().GetStoreRepurchaseSystem();
	int nRepurchaseMoney = rs.GetRepurchasePrice(item);
	if (nRepurchaseMoney == CDnStoreRepurchaseSystem::eINVALID_PRICE)
	{
		_ASSERT(0);
		return;
	}

	std::wstring wszMoney;
	DN_INTERFACE::UTIL::GetMoneyFormatUseStr(nRepurchaseMoney, wszMoney);
	m_pItemAmount1->SetText(wszMoney);

	m_pItemText->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1782)); // UISTRING : Repurchasing the item.\nA fee will be added upon repurchase.
}
