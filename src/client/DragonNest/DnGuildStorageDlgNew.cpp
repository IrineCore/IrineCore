#include "StdAfx.h"

#include "DnGuildStorageDlgNew.h"
#include "DnGuildStorageDlg.h"
#include "DnItemTask.h"
#include "DnGuildMoneyInputDlg.h"
#include "DnStoreConfirmExDlg.h"
#include "DnInterface.h"
#include "DnInterfaceString.h"
#include "DnMainMenuDlg.h"
#include "DnGuildTask.h"
#include "DnGuildStorageHistoryDlg.h"
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnGuildStorageDlgNew::CDnGuildStorageDlgNew( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pGuildStorageDlg(NULL)
, m_pStaticGold(NULL)
, m_pStaticSilver(NULL)
, m_pStaticBronze(NULL)
, m_pButtonMoneyIn(NULL)
, m_pButtonMoneyOut(NULL)
, m_pStaticCount(NULL)
, m_pButtonHistory(NULL)
, m_pMoneyInputDlg(NULL)
, m_pGuildMoneyInputDlg(NULL)
, m_pSplitConfirmExDlg(NULL)
, m_pHistoryDlg(NULL)
, m_nItemCnt(0)
{
	m_emMoneyType = MONEY_CONFIRM_IN;
}

CDnGuildStorageDlgNew::~CDnGuildStorageDlgNew(void)
{
	SAFE_DELETE( m_pGuildStorageDlg );
	SAFE_DELETE( m_pMoneyInputDlg );
	SAFE_DELETE( m_pGuildMoneyInputDlg );
	SAFE_DELETE( m_pSplitConfirmExDlg );
	SAFE_DELETE( m_pHistoryDlg );
}

void CDnGuildStorageDlgNew::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "StorageGuildDlg.ui" ).c_str(), bShow );
}

void CDnGuildStorageDlgNew::InitialUpdate()
{
	m_pGuildStorageDlg = new CDnGuildStorageDlg( UI_TYPE_CHILD, this );
	m_pGuildStorageDlg->Initialize( true );

	GetItemTask().GetGuildInventory().SetInventoryDialog( this );

	m_pStaticGold = GetControl<CEtUIStatic>("ID_GOLD");
	m_pStaticSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pStaticBronze = GetControl<CEtUIStatic>("ID_BRONZE");

	m_pButtonMoneyIn = GetControl<CEtUIButton>("ID_MONEY_IN");
	m_pButtonMoneyOut = GetControl<CEtUIButton>("ID_MONEY_OUT");

	m_pStaticCount = GetControl<CEtUIStatic>("ID_TEXT_COUNT");
	m_pButtonHistory = GetControl<CEtUIButton>("ID_BUTTON_LIST");

	m_pMoneyInputDlg = new CDnMoneyInputDlg( UI_TYPE_MODAL, NULL, MONEY_INPUT_DIALOG, this );
	m_pMoneyInputDlg->Initialize( false );

	m_pGuildMoneyInputDlg = new CDnGuildMoneyInputDlg( UI_TYPE_MODAL, NULL, MONEY_INPUT_DIALOG, this );
	m_pGuildMoneyInputDlg->Initialize( false );

	m_pSplitConfirmExDlg = new CDnStoreConfirmExDlg( UI_TYPE_MODAL, NULL, ITEM_ATTACH_CONFIRM_DIALOG, this );
	m_pSplitConfirmExDlg->Initialize( false );

	m_pHistoryDlg = new CDnGuildStorageHistoryDlg( UI_TYPE_MODAL );
	m_pHistoryDlg->Initialize( false );

	// Initialize guild storage beforehand since its state can be dynamic.
	SetUseItemCnt( 0 );
}

void CDnGuildStorageDlgNew::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_MONEY_IN") )
		{
			// Storage
			m_emMoneyType = MONEY_CONFIRM_IN;
			m_pMoneyInputDlg->SetInfo( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1605 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1607 ) );
			m_pMoneyInputDlg->SetMaxMoney( -1 );
			m_pMoneyInputDlg->Show( true );
			return;
		}
		else if( IsCmdControl("ID_MONEY_OUT") )
		{
			// Withdraw
			m_emMoneyType = MONEY_CONFIRM_OUT;
			m_pGuildMoneyInputDlg->SetInfo( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1606 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1608 ) );
			m_pGuildMoneyInputDlg->SetMaxMoney( GetItemTask().GetGuildWareCoin() );
			m_pGuildMoneyInputDlg->Show( true );
			return;
		}
		else if( IsCmdControl("ID_BUTTON_LIST") )
		{
			m_pHistoryDlg->Show( true );
			return;
		}
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_GUILD);
			return;
		}
#endif
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnGuildStorageDlgNew::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		if( drag::IsValid() )
		{
			CDnSlotButton *pDragButton = static_cast<CDnSlotButton*>(drag::GetControl());
			if( pDragButton->GetSlotType() == ST_STORAGE_GUILD )
			{
				drag::Command(UI_DRAG_CMD_CANCEL);
				drag::ReleaseControl();
			}
		}
	}
	else
	{
		// Fetch current guild info and
		if( !CDnGuildTask::IsActive() ) return;
		TGuild *pGuild = GetGuildTask().GetGuildInfo();
		if( !pGuild->IsSet() || !GetGuildTask().GetGuildMemberCount() ) {
			// If the window is opened right after connecting to the server, the player may be in a guild without having received its info yet, so handle it this way.
			if( CDnActor::s_hLocalActor ) {
				CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
				if( pPlayer && pPlayer->IsJoinGuild() ) {
					GetGuildTask().RequestGetGuildInfo( true );
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3963 ), false );

					// Force close open windows when opening guild storage.
					GetInterface().GetMainMenuDialog()->CloseInvenDialog();
					GetInterface().CloseNpcTalkReturnDlg();
				}
			}
			return;
		}
	}

	CEtUIDialog::Show( bShow );
}

void CDnGuildStorageDlgNew::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		switch( nID ) 
		{
		case MONEY_INPUT_DIALOG:
			{
				if( IsCmdControl("ID_BUTTON_OK") )
				{
					if( m_emMoneyType == MONEY_CONFIRM_IN )
					{
						GetItemTask().RequestInvenToGuildWare( m_pMoneyInputDlg->GetMoney() );
						m_pMoneyInputDlg->Show( false );
					}
					else
					{
						GetItemTask().RequestGuildWareToInven( m_pGuildMoneyInputDlg->GetMoney() );
						m_pGuildMoneyInputDlg->Show( false );
					}
				}
				else if( IsCmdControl("ID_BUTTON_CANCEL") )
				{
					m_pMoneyInputDlg->Show( false );
					m_pGuildMoneyInputDlg->Show( false );
				}
			}
			break;

		case ITEM_ATTACH_CONFIRM_DIALOG:
			{
				if (IsCmdControl("ID_OK"))
				{
					CDnItem* pItem = m_pSplitConfirmExDlg->GetItem();
					if (pItem)
					{
						int nCount = m_pSplitConfirmExDlg->GetItemCount();

						if (nCount <= 0)
						{
							m_pSplitConfirmExDlg->Show(false);
							return;
						}

						CDnInterface::emSTORE_CONFIRM_TYPE confirmType = m_pSplitConfirmExDlg->GetConfirmType();
						if (confirmType == CDnInterface::STORAGE_FROM_INVENTORY)
							RequestMoveItem(MoveType_InvenToGuildWare, pItem, nCount);
					}

					m_pSplitConfirmExDlg->Show(false);
				}
				else if (IsCmdControl("ID_CANCEL"))
				{
					m_pSplitConfirmExDlg->Show(false);
				}
			}
			break;
		}
	}
}

void CDnGuildStorageDlgNew::SetUseItemCnt( DWORD dwItemCnt )
{
	ASSERT(!(dwItemCnt%ITEM_X)&&"CDnGuildStorageDlgNew::SetUseItemCnt");
	if( dwItemCnt % ITEM_X )
		return;

	m_nItemCnt = (int)dwItemCnt;

	if( m_pGuildStorageDlg )
	{
		m_pGuildStorageDlg->SetUseItemCnt( dwItemCnt );
	}
}

void CDnGuildStorageDlgNew::SetItem( MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnStorageTabDlg::SetItem, pItem is NULL!");
	m_pGuildStorageDlg->SetItem( pItem );
}

void CDnGuildStorageDlgNew::ResetSlot( MIInventoryItem *pItem )
{
	m_pGuildStorageDlg->ResetSlot( pItem->GetSlotIndex() );
}

bool CDnGuildStorageDlgNew::IsEmptySlot()
{
	if( !m_pGuildStorageDlg )
		return false;
	return m_pGuildStorageDlg->IsEmptySlot();
}

int CDnGuildStorageDlgNew::GetEmptySlot()
{
	if( !m_pGuildStorageDlg )
		return -1;
	return m_pGuildStorageDlg->GetEmptySlot();
}

void CDnGuildStorageDlgNew::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() )
	{
		if( !CDnItemTask::IsActive() ) return;
		if( !CDnGuildTask::IsActive() ) return;
		INT64 nCoin = GetItemTask().GetGuildWareCoin();
		INT64 nGold = nCoin/10000;
		INT64 nSilver = (nCoin%10000)/100;
		INT64 nBronze = nCoin%100;
		std::wstring strString;

		m_pStaticGold->SetInt64ToText( nGold );
		DN_INTERFACE::UTIL::GetValue_2_String( (int)nGold, strString );
		m_pStaticGold->SetTooltipText( strString.c_str() );

		m_pStaticSilver->SetInt64ToText( nSilver );
		DN_INTERFACE::UTIL::GetValue_2_String( (int)nSilver, strString );
		m_pStaticSilver->SetTooltipText( strString.c_str() );

		m_pStaticBronze->SetInt64ToText( nBronze );
		DN_INTERFACE::UTIL::GetValue_2_String( (int)nBronze, strString );
		m_pStaticBronze->SetTooltipText( strString.c_str() );

		// Refresh storage withdrawal count. (Must be handled in Process so changes to rank or per-rank count are reflected immediately.)
		if( CDnGuildTask::IsActive() && GetGuildTask().GetGuildInfo() && GetGuildTask().GetGuildInfo()->IsSet() )
		{
#ifdef PRE_ADD_CHANGEGUILDROLE
			if( static_cast<eGuildRoleType>(GetGuildTask().GetMyRole()) <= GUILDROLE_TYPE_SUBMASTER )
#else
			if( GetGuildTask().IsMaster() )
#endif
			{
				m_pStaticCount->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3599 ) );
			}
			else
			{
				int nMaxTakeItemCount = GetGuildTask().GetMaxTakeItemByRole( static_cast<eGuildRoleType>(GetGuildTask().GetMyRole()) );
				int nRemainCount = nMaxTakeItemCount - GetItemTask().GetTakeGuildWareItemCount();
				if( nRemainCount <= 0 )
				{
					m_pStaticCount->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3816 ) );
				}
				else
				{
					WCHAR wszTemp[64] = {0,};
					swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4123 ), nRemainCount );
					m_pStaticCount->SetText( wszTemp );
				}
			}
		}
	}
}

void CDnGuildStorageDlgNew::MoveItemToStorage(CDnQuickSlotButton* pFromSlot, bool bItemSplit)
{
	if (pFromSlot == NULL)
	{
		_ASSERT(0);
		return;
	}

	MIInventoryItem* pInvenItem = pFromSlot->GetItem();
	if (pInvenItem == NULL || pInvenItem->GetType() != MIInventoryItem::Item)
	{
		_ASSERT(0);
		return;
	}

	CDnItem* pItem = static_cast<CDnItem *>(pInvenItem);
	if (pItem == NULL)
		return;

	if (pItem->GetReversion() == CDnItem::Belong && pItem->IsSoulbBound())
	{
		eItemTypeEnum type = pItem->GetItemType();
		if (type == ITEMTYPE_WEAPON || 
			type == ITEMTYPE_PARTS ||
			type == ITEMTYPE_GLYPH ||
			type == ITEMTYPE_POTENTIAL_JEWEL)
		{
			if (pItem->GetSealCount() > 0)
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3629 ), MB_OK ); // UISTRING : Re-sealable bound items must be sealed with a Sealing Stamp to insert.
				return;
			}
		}

		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3836 ), MB_OK ); // UISTRING : This item cannot be traded
		return;
	}
	else if (pItem->GetReversion() == CDnItem::GuildBelong)
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3836 ), MB_OK ); // UISTRING : This item cannot be traded
		return;
	}

	if( GetGuildTask().GetAuth( static_cast<eGuildRoleType>(GetGuildTask().GetMyRole()), GUILDAUTH_TYPE_STOREITEM ) == false )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3914 ), MB_OK );
		return;
	}

	if (pFromSlot->GetRenderCount() > 1 && bItemSplit)
	{
		m_pSplitConfirmExDlg->SetItem(pFromSlot, CDnInterface::STORAGE_FROM_INVENTORY);
		m_pSplitConfirmExDlg->Show(true);
		return;
	}

	RequestMoveItem(MoveType_InvenToGuildWare, pItem, pItem->GetOverlapCount());
}

void CDnGuildStorageDlgNew::RequestMoveItem(eItemMoveType moveType, CDnItem* pItem, int itemCount)
{
	DWORD emptySlotType = -1;
	if (moveType == MoveType_InvenToGuildWare)	
	{
		// Interacting with this dialog itself means the tab is already selected.
		// Player and guild storage merged.
		emptySlotType = CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG;
	}
	else if (moveType == MoveType_GuildWareToInven)	emptySlotType = CDnMainMenuDlg::INVENTORY_DIALOG;
	else
	{
		_ASSERT(0);
		return;
	}

	int destIdx = -1;

	// When moving from inventory to guild storage, find an appropriate slot for stackable items (MoveType_GuildWareToInven seems unused here).
	if (moveType == MoveType_InvenToGuildWare)
	{
		// Find an appropriate slot in the inventory window.
		// Since it does not check whether the current slot is closed,
		// If it ever becomes UnUsable while containing items in the future, this routine will also need to be modified (currently inventory expansion is not time-limited, so this cannot happen).
		int nInsertableSlotIndex = -1;
		std::vector<CDnItem*> vecItem;
		GetItemTask().GetGuildInventory().ScanItemFromID( pItem->GetClassID(), &vecItem );
		for( int i = 0; i < (int)vecItem.size(); ++i )
		{
			if (vecItem[i]->GetOverlapCount() + itemCount <= pItem->GetMaxOverlapCount())
			{
				nInsertableSlotIndex = vecItem[i]->GetSlotIndex();
				break;
			}
		}

		if (nInsertableSlotIndex == -1)
		{
			if (!GetInterface().IsEmptySlot(CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG))
			{
				GetInterface().MessageBox(1609, MB_OK);
				return;
			}
			nInsertableSlotIndex = GetInterface().GetEmptySlot(emptySlotType);
		}
		destIdx = nInsertableSlotIndex;
	}
	else if (moveType == MoveType_GuildWareToInven)
	{
		destIdx = GetInterface().GetEmptySlot(emptySlotType);
	}

	CDnItemTask::GetInstance().RequestMoveItem(moveType,
											pItem->GetSlotIndex(),
											pItem->GetSerialID(),
											destIdx,
											itemCount);
}

void CDnGuildStorageDlgNew::OnRecvGetGuildStorageHistoryList( SCGetGuildWareHistory *pPacket )
{
	if( !m_pHistoryDlg ) return;
	m_pHistoryDlg->OnRecvGetGuildStorageHistoryList( pPacket );
}
