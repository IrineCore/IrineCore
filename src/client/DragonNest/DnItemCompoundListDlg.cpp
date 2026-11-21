#include "Stdafx.h"
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
#include "DnItemCompoundListDlg.h"
#include "DnTableDB.h"
#include "DNCommonDef.h"
#include "DnInterfaceString.h"
#include "DnItemCompoundNotifyDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "ItemSendPacket.h"

using namespace DN_INTERFACE::STRING;

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

int CDnItemCompoundListDlg::m_nRemoteItemID = 0;

CDnItemCompoundListDlg::CDnItemCompoundListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
{
	m_pStaticBase = NULL;
	m_pItemSlotButton = NULL;
	m_pItem = NULL;
	m_pStaticName = NULL;
	m_pStaticLevel = NULL;
	m_pStaticTypeText = NULL;
	m_pCheckBoxCompoundCheck = NULL;
	m_pStaticGold = NULL;
	m_pStaticSilver = NULL;
	m_pStaticBronze = NULL;
	for( int i = 0; i < 5; i ++ )
	{
		m_pNeedItem[i] = NULL;
		m_pNeedItemButtonSlot[i] = NULL;
	}
	m_pCreateButton = NULL;
	m_bEnable = false;
	m_pStaticNotifyFlag = NULL;
	m_pTextNotifyItem = NULL;
	m_pStaticBackBoard = NULL;
	m_nCompoundID = 0;
	m_bSelected = true;
	m_bForceOpenMode = false;
	m_dwColorGold = m_dwColorSilver = m_dwColorBronze= 0;
	m_nItemID = 0;
}
CDnItemCompoundListDlg::~CDnItemCompoundListDlg()
{
	Clear();
}

void CDnItemCompoundListDlg::InitialUpdate()
{
	m_pStaticBase = GetControl<CEtUIStatic>("ID_STATIC0");
	m_pItemSlotButton = GetControl<CDnItemSlotButton>("ID_ITEM"); 
	m_pItemSlotButton->SetSlotType( ST_ITEM_COMPOUND );
	m_pStaticName = GetControl<CEtUIStatic>("ID_TEXT_NAME");
	m_pStaticLevel = GetControl<CEtUIStatic>("ID_TEXT_LEVEL");
	m_pStaticTypeText = GetControl<CEtUIStatic>("ID_TEXT_TYPE");

	m_pCheckBoxCompoundCheck = GetControl<CEtUICheckBox>("ID_CHECKBOX_INFO");
	m_pTextNotifyItem = GetControl<CEtUIStatic>("ID_TEXT_INFO");

	m_pStaticGold = GetControl<CEtUIStatic>("ID_GOLD");
	m_pStaticSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pStaticBronze = GetControl<CEtUIStatic>("ID_BRONZE");
	m_pCreateButton = GetControl<CEtUIButton>("ID_BT_ALL"); 

	m_dwColorGold = m_pStaticGold->GetTextColor();
	m_dwColorSilver = m_pStaticSilver->GetTextColor();
	m_dwColorBronze = m_pStaticBronze->GetTextColor();
	
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	m_pStaticNotifyFlag = GetControl<CEtUIStatic>("ID_STATIC_FLAG");
	m_pStaticNotifyFlag->Show( false );
#endif

	m_pStaticBackBoard = GetControl<CEtUIStatic>("ID_STATIC0");
	m_pStaticBackBoard->Show( false );
	
	char szControlName[32];
	for( int i=0; i<5; i++ ) 
	{
		sprintf_s( szControlName, _countof(szControlName), "ID_ITEM_SLOT%d", 4 - i );
		m_pNeedItemButtonSlot[i] = GetControl<CDnItemSlotButton>(szControlName);
		m_pNeedItemButtonSlot[i]->SetSlotType( ST_ITEM_COMPOUND_JEWEL );
		m_pNeedItemButtonSlot[i]->SetBlendRate( 0.f );		
	}
}
void CDnItemCompoundListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemCompound_ListDlg.ui" ).c_str(), bShow );
}
void CDnItemCompoundListDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		ResetForceOpenMode();
	}

	CEtUIDialog::Show( bShow );
}
void CDnItemCompoundListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	if( !IsShow() )
		return;

	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		// Later make the interface control the message box and set the item value here. 
		if( IsCmdControl("ID_BT_ALL") )
		{
			//CTreeItem *pItem  = m_pTreeCtrlEquipment->GetSelectedItem();
			if( m_pParentDialog )
				m_pParentDialog->ProcessCommand( EVENT_BUTTON_CLICKED, false, pControl, 0 );	
		}
	}
	else if( nCommand == EVENT_CHECKBOX_CHANGED ) 
	{
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		if( IsCmdControl( "ID_CHECKBOX_INFO" ) ) 
		{
			if( m_pCheckBoxCompoundCheck->IsChecked() )
			{
				if( m_nCompoundID && m_pItem )
				{
					CSCompoundNotify Info;
					Info.nItemID = m_nItemID;
					Info.nItemOption = m_nCompoundID;
					EnableNotify( true );
					SendItemCompoundNotify(Info);
				}
				if( m_pParentDialog )
					m_pParentDialog->ProcessCommand( EVENT_CHECKBOX_CHANGED, false, pControl, 0 );	
			}
			else
			{
				CSCompoundNotify Info;
				Info.nItemID = 0;
				Info.nItemOption = 0;
				EnableNotify( false );
				SendItemCompoundNotify(Info);
			}
		}
#endif
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnItemCompoundListDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
		{
			POINT MousePoint;
			float fMouseX, fMouseY;

			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );
			PointToFloat( MousePoint, fMouseX, fMouseY );

			if( m_pStaticBase->IsShow() && m_pCreateButton->IsInside( fMouseX, fMouseY ) )
			{
				//ProcessCommand( EVENT_BUTTON_CLICKED, false, NULL, 0 );
			}

			if( m_bSelected && m_pStaticBase->IsShow() ) 
			{
				UpdateSelectOption();
			}
		}break;
	case WM_LBUTTONDBLCLK:
		{
		}break;

	case WM_MOUSEWHEEL:
		{
		}break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}
void CDnItemCompoundListDlg::SetNotifyCheckBox( bool bCheck )
{
	m_pCheckBoxCompoundCheck->SetChecked( bCheck, false );
}
bool CDnItemCompoundListDlg::IsNotifyCheck()
{
	return m_pCheckBoxCompoundCheck->IsChecked();
}

void CDnItemCompoundListDlg::EnableNotify( bool bEnable )
{
	if( m_pStaticNotifyFlag ) m_pStaticNotifyFlag->Show( bEnable );
}

void CDnItemCompoundListDlg::Clear()
{
	SAFE_DELETE( m_pItem );
	if( m_pStaticBase ) m_pStaticBase->Show( false );
	if( m_pItemSlotButton ) m_pItemSlotButton->ResetSlot();
	if( m_pStaticName ) m_pStaticName->SetText( L"" );
	if( m_pStaticLevel ) m_pStaticLevel->SetText( L"" );
	if( m_pStaticTypeText ) m_pStaticTypeText->Show( false );

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	if( m_pStaticNotifyFlag ) m_pStaticNotifyFlag->Show( false );
#endif
	
	for( int i = 0; i < 5; ++i )
	{
		SAFE_DELETE( m_pNeedItem[i] );
		m_pNeedItemButtonSlot[i]->ResetSlot();
	}
}

void CDnItemCompoundListDlg::ForceBlend()
{
	if( m_pStaticBase ) m_pStaticBase->ForceBlend();
	if( m_pStaticName ) m_pStaticName->ForceBlend();
	if( m_pStaticLevel ) m_pStaticLevel->ForceBlend();
	if( m_pStaticTypeText ) m_pStaticTypeText->ForceBlend();
}

void CDnItemCompoundListDlg::Enable( bool bValue )
{
	m_bEnable = bValue;
	if( m_pStaticBase )	m_pStaticBase->Enable( m_bEnable );
	if( m_pItemSlotButton ) m_pItemSlotButton->SetRegist( !m_bEnable );
	if( m_pStaticName ) m_pStaticName->Enable( m_bEnable );
	if( m_pStaticLevel ) m_pStaticLevel->Enable( m_bEnable );
	if( m_pStaticTypeText ) m_pStaticTypeText->Enable( m_bEnable );
	if( m_pStaticBackBoard ) 
	{
		m_pStaticBackBoard->Enable( m_bEnable );
		m_pStaticBackBoard->Show( true );
	}
}

bool CDnItemCompoundListDlg::IsInsideItem( float fX, float fY )
{
	SUICoord uiCoordsBase;
	m_pStaticBase->GetUICoord(uiCoordsBase);
	return uiCoordsBase.IsInside(fX, fY);
}

INT64 CDnItemCompoundListDlg::GetDiscountedCost( INT64 iCost )
{
	INT64 iResult = iCost;

	if( m_nRemoteItemID ) 
	{
		int nTypeParam = CDnItem::GetItemTypeParam( m_nRemoteItemID );
		INT64 nDiscount = iCost * nTypeParam / 100;
		iResult = iCost - nDiscount;
	}

	return iResult;
}

void CDnItemCompoundListDlg::SetInfo( const int& nCompoundID, const int& nItemID,/* CDnCompoundBase::ItemGroupStruct* const pCompoundGroupData,*/ CEtUIDialog* pParent )
{

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	int nOptionIndex = pSox->GetFieldFromLablePtr( nCompoundID, "_CompoundPotentialID" )->GetInteger();

	TItemInfo itemInfo;
	if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) return;
	itemInfo.Item.cOption = nOptionIndex;
	
	SAFE_DELETE( m_pItem );
	m_pItem = GetItemTask().CreateItem( itemInfo );
	
	m_nCompoundID = nCompoundID;
	m_nItemID = nItemID;
	//m_pCompoundGroupData = *pCompoundGroupData;
	m_pParentDialog = pParent;

	WCHAR wszStr[64];
	ZeroMemory( wszStr, sizeof(wszStr) );

	if( m_pItem )
	{
		// Reset durability. Since max value is unknown above, set to max value here like this.
		if( m_pItem->GetItemType() == ITEMTYPE_WEAPON ) 
		{
			CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(m_pItem);
			pWeapon->SetDurability( pWeapon->GetMaxDurability() );
			//swprintf_s( wszStr, L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ITEM::EQUIPTYPE_2_INDEX( (CDnWeapon::EquipTypeEnum)pWeapon->GetItemType() ) ) );
			swprintf_s( wszStr, _countof(wszStr), L"%s : %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7009 ), DN_INTERFACE::STRING::ITEM::GetEquipString( pWeapon->GetEquipType() ) );
		}
		else if( m_pItem->GetItemType() == ITEMTYPE_PARTS ) 
		{
			CDnParts *pParts = dynamic_cast<CDnParts *>(m_pItem);
			pParts->SetDurability( pParts->GetMaxDurability() );
			//swprintf_s( wszStr, L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ITEM::PARTSTYPE_2_INDEX( (CDnParts::PartsTypeEnum)pParts->GetItemType() ) ) );
			swprintf_s( wszStr, _countof(wszStr), L"%s : %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7009 ), DN_INTERFACE::STRING::ITEM::GetPartsString( pParts->GetPartsType() ) );
		}

		//WCHAR wszStr[64];
		//swprintf_s( wszStr, L"%d %s", nLevel, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ITEM::RANK_2_INDEX( (eItemRank)nRank ) ) );
		//pStaticLevel->SetText( wszStr );

		m_pStaticTypeText->SetText( wszStr );
		m_pItemSlotButton->SetItem( m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
		m_pStaticName->SetTextColor( ITEM::RANK_2_COLOR( (eItemRank)m_pItem->GetItemRank() ) );

		m_pStaticName->SetText( CDnItem::GetItemFullName( nItemID, nOptionIndex ).c_str() );
		swprintf_s( wszStr, L"  %s : %d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 64 ), m_pItem->GetLevelLimit() );
		//m_pStaticLevel->SetIntToText( m_pItem->GetLevelLimit() );
		m_pStaticLevel->SetText( wszStr );
	}

	m_pStaticLevel->Show( true );
	m_pStaticTypeText->Show( true );


#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	bool bNotified = CDnItemCompoundNotifyDlg::IsMatchedNotifyItem( nCompoundID );
	EnableNotify( bNotified );
#endif
	
	UpdateSelectOption() ;
}

void CDnItemCompoundListDlg::UpdateSelectOption() 
{
	SUICoord uiCoordBase, uiCoordSelect;

	if( ! m_bSelected  ) 
	{
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		m_pCheckBoxCompoundCheck->Show( false );
		m_pTextNotifyItem->Show( false );
#endif
	}
	else 
	{
		m_pStaticBase->GetUICoord( uiCoordBase );

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		m_pCheckBoxCompoundCheck->Show( true );
		m_pTextNotifyItem->Show( true );

		bool bNotified = CDnItemCompoundNotifyDlg::IsMatchedNotifyItem( m_nCompoundID );
		m_pCheckBoxCompoundCheck->SetChecked( bNotified , false );

#endif

	}

	RefreshCompoundItem();
}

bool CDnItemCompoundListDlg::IsNeedItemList( int nCompoundID )
{
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

	CompoundInfo.iCost = CDnItemCompoundListDlg::GetDiscountedCost( CompoundInfo.iCost );

	if( GetItemTask().GetCoin() >= CompoundInfo.iCost )
	{
		for( int i = 0; i < 5; ++i ) 
		{
			if( CompoundInfo.aiItemID[i] > 0 && CompoundInfo.aiItemCount[i] > 0 )
			{
				CDnItemTask::ItemCompoundInfo Info;
				Info.iItemID = CompoundInfo.aiItemID[ i ];
				Info.iUseCount = CompoundInfo.aiItemCount[ i ];

				// Additionally check inventory for item types and counts.
				int nCurItemCount = 0;
				if( CompoundInfo.abResultItemIsNeedItem[i] ) 
				{
					std::vector<CDnItem *> pVecResultList;
					//When searching for items, only search items matching the option settings..
					nCurItemCount = GetItemTask().GetCharInventory().FindItemList( CompoundInfo.aiItemID[i], CompoundInfo.iCompoundPreliminaryID, pVecResultList );
				}
				else nCurItemCount = GetItemTask().GetCharInventory().GetItemCount( CompoundInfo.aiItemID[i], -1 );

				// Returns false if any one type is missing.
				if( nCurItemCount < CompoundInfo.aiItemCount[i] )
					return false;
			}
		}
	}

	return true;
}

void CDnItemCompoundListDlg::SetNeedItemSlot( int nSlotIndex, int nItemID, int nNeedItemCount, int nCurItemCount, char nOptionIndex/* = -1*/ )
{
	if( nItemID == 0 )
		return;

	SAFE_DELETE( m_pNeedItem[nSlotIndex] );

	TItemInfo itemInfo;
	if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) return;

	//If option values are configured, apply them when creating the item.
	if (nOptionIndex != -1)
		itemInfo.Item.cOption = nOptionIndex;

	m_pNeedItem[nSlotIndex] = GetItemTask().CreateItem( itemInfo );
	m_pNeedItemButtonSlot[nSlotIndex]->SetItem(m_pNeedItem[nSlotIndex], CDnSlotButton::ITEM_ORIGINAL_COUNT);
	m_pNeedItemButtonSlot[nSlotIndex]->SetJewelCount( nNeedItemCount, nCurItemCount );

	if( nCurItemCount < nNeedItemCount )
		m_pNeedItemButtonSlot[nSlotIndex]->SetRegist( true );
	else
		m_pNeedItemButtonSlot[nSlotIndex]->SetRegist( false );
}

void CDnItemCompoundListDlg::RefreshCompoundItem()
{
	// Since CDnItemCompoundListDlg is only shown when clicked in the set item list
	// If not in the set item list, it cannot be clicked so creation itself is impossible. 
	// So the m_bSelected option itself is meaningless. 

	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( m_nCompoundID, &CompoundInfo );

	CompoundInfo.iCost = GetDiscountedCost( CompoundInfo.iCost );

	// Show fee.
	//if( false == m_pCompoundGroupData.bIsPreview )
	//{
		int nGold = (int)(CompoundInfo.iCost/10000);
		int nSilver = (int)((CompoundInfo.iCost%10000)/100);
		int nBronze = (int)(CompoundInfo.iCost%100);
		m_pStaticGold->SetIntToText( nGold );
		m_pStaticSilver->SetIntToText( nSilver );
		m_pStaticBronze->SetIntToText( nBronze );
	//}
	//else
	//{
	//	m_pStaticGold->SetText( L"" );
	//	m_pStaticSilver->SetText( L"" );
	//	m_pStaticBronze->SetText( L"" );
	//}

	// Retrieve and check the fee.
	bool bEnoughCoin = false;
	if( GetItemTask().GetCoin() >= CompoundInfo.iCost )
	{
		bEnoughCoin = true;
		m_pStaticGold->SetTextColor( m_dwColorGold );
		m_pStaticSilver->SetTextColor( m_dwColorSilver );
		m_pStaticBronze->SetTextColor( m_dwColorBronze );
	}
	else
	{
		if( CompoundInfo.iCost >= 10000 ) m_pStaticGold->SetTextColor( 0xFFFF0000, true );
		if( CompoundInfo.iCost >= 100 ) m_pStaticSilver->SetTextColor( 0xFFFF0000, true );
		m_pStaticBronze->SetTextColor( 0xFFFF0000, true );
	}

	bool bNeedItemResult = true;
	bool bPossibleLevel = false;

	//for( int i=0; i<5; i++ )
	//	m_pNeedItemButtonSlot[i]->ResetSlot();
	
	// Set up the required item info struct to send to server.
	m_vNeedItemInfo.clear();
	for( int i = 0; i < 5; ++i ) 
	{
		if( CompoundInfo.aiItemID[i] > 0 && CompoundInfo.aiItemCount[i] > 0 )
		{
			CDnItemTask::ItemCompoundInfo Info;
			Info.iItemID = CompoundInfo.aiItemID[ i ];
			Info.iUseCount = CompoundInfo.aiItemCount[ i ];
			m_vNeedItemInfo.push_back(Info);

			// Additionally check inventory for item types and counts.
			int nCurItemCount = 0;
			if( CompoundInfo.abResultItemIsNeedItem[i] ) 
			{
				std::vector<CDnItem *> pVecResultList;
				//When searching for items, only search items matching the option settings..
				nCurItemCount = GetItemTask().GetCharInventory().FindItemList( CompoundInfo.aiItemID[i], CompoundInfo.iCompoundPreliminaryID, pVecResultList );
			}
			else nCurItemCount = GetItemTask().GetCharInventory().GetItemCount( CompoundInfo.aiItemID[i], -1 );

			// Set material item slot.
			//if( false == m_pCompoundGroupData.bIsPreview )
			SetNeedItemSlot( i, CompoundInfo.aiItemID[i], CompoundInfo.aiItemCount[i], nCurItemCount, CompoundInfo.iCompoundPreliminaryID );

			// Returns false if any one type is missing.
			if( nCurItemCount < CompoundInfo.aiItemCount[i] )
				bNeedItemResult = false;
		}
	}
	//if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetLevel() >= m_pCompoundGroupData->nActivateLevel ) bPossibleLevel = true;
	m_pCreateButton->Enable( ( bNeedItemResult && bEnoughCoin /*&& bPossibleLevel*/ ) );
	
	// Set item struct info needed for the message box
	if( m_vNeedItemInfo.empty() == false && m_pItem != NULL && CompoundInfo.iCost != 0 )
	{
		m_CompoundItemPackage.nCompoundID = m_nCompoundID;
		m_CompoundItemPackage.vNeedItemInfo = m_vNeedItemInfo;
		m_CompoundItemPackage.nCost = CompoundInfo.iCost;
	}

};

void CDnItemCompoundListDlg::OnRecvItemCompound()
{
	UpdateSelectOption();

	//for( int i=0; i<5; i++ )
	//	m_pNeedItemButtonSlot[i]->ResetSlot();

	if( m_nRemoteItemID ) {
		int nCount = 0;
		nCount += GetItemTask().GetCharInventory().GetItemCount( m_nRemoteItemID );
		nCount += GetItemTask().GetCashInventory().GetItemCount( m_nRemoteItemID );
		if( !nCount ) m_pParentDialog->Show( false );
	}
}

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND

void CDnItemCompoundListDlg::SetForceOpenMode( bool bShow )
{	
	// No reason to remove the check button. 
	m_bForceOpenMode = bShow;
	m_pCreateButton->Show( !bShow );
}
void CDnItemCompoundListDlg::ResetForceOpenMode()
{
	m_bForceOpenMode = false;
	m_pCreateButton->Show( true );
}

#endif


#endif 





