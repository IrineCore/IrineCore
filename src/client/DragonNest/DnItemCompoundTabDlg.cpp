#include "StdAfx.h"
#include "DnItemCompoundTabDlg.h"
#include "DnInterface.h"
#include "DnTableDB.h"
#include "DnItemCompoundMixDlg.h"
#include "DnMainMenuDlg.h"
#include "DnMainFrame.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnItemCompoundTabDlg::CDnItemCompoundTabDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pTabButtonWeapon(NULL)
, m_pTabButtonParts(NULL)
, m_pTabButtonAccessory(NULL)
, m_pTabButtonNormal(NULL)
, m_nCurTab(0)
, m_pComboBoxJob(NULL)
, m_pComboBoxType(NULL)
, m_pButtonOK(NULL)
, m_pButtonCancel(NULL)
, m_pStaticSelect(NULL)
, m_nCurSelect(-1)
, m_pButtonPagePrev(NULL)
, m_pButtonPageNext(NULL)
, m_pStaticPage(NULL)
, m_nCurPage(0)
, m_nMaxPage(0)
, m_nCurCompoundListID(-1)
, m_pCompoundMixDlg(NULL)
, m_bOK(false)
, m_pGold(NULL)
, m_pSilver(NULL)
, m_pBronze(NULL)
{
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		m_pNeedItem[i] = NULL;
		m_pNeedItemSlotButton[i] = NULL;
	}
}

CDnItemCompoundTabDlg::~CDnItemCompoundTabDlg(void)
{
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
		SAFE_DELETE(m_pNeedItem[i]);
	SAFE_DELETE(m_pCompoundMixDlg);
}

void CDnItemCompoundTabDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemCompoundTabDlg.ui" ).c_str(), bShow );
}

void CDnItemCompoundTabDlg::InitialUpdate()
{
	m_pTabButtonWeapon = GetControl<CEtUIRadioButton>("ID_TAB_WEAPON");
	m_pTabButtonParts = GetControl<CEtUIRadioButton>("ID_TAB_PARTS");
	m_pTabButtonAccessory = GetControl<CEtUIRadioButton>("ID_TAB_ACCESSORY");
	m_pTabButtonNormal = GetControl<CEtUIRadioButton>("ID_TAB_NORMAL");

	m_pComboBoxJob = GetControl<CEtUIComboBox>("ID_COMBOBOX_JOB");

	// Contents for type differ per tab, so do not populate them here.
	m_pComboBoxType = GetControl<CEtUIComboBox>("ID_COMBOBOX_TYPE");

	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_CANCEL");

	char szControlName[32];
	for( int i = 0; i < NUM_MAX_COMPOUND_ITEM; ++i )
	{
		sprintf_s(szControlName, _countof(szControlName), "ID_SLOT%d", i);
		m_sCompoundItem[i].m_pItemSlotButton = GetControl<CDnItemSlotButton>(szControlName);
		m_sCompoundItem[i].m_pItemSlotButton->SetSlotType( ST_ITEM_COMPOUND );

		sprintf_s(szControlName, _countof(szControlName), "ID_BASE%d", i);
		m_sCompoundItem[i].m_pStaticBase = GetControl<CEtUIStatic>(szControlName);

		sprintf_s(szControlName, _countof(szControlName), "ID_NAME%d", i);
		m_sCompoundItem[i].m_pStaticName = GetControl<CEtUIStatic>(szControlName);

		sprintf_s(szControlName, _countof(szControlName), "ID_STATIC_LEVEL%d", i);
		m_sCompoundItem[i].m_pStaticLevelText = GetControl<CEtUIStatic>(szControlName);

		sprintf_s(szControlName, _countof(szControlName), "ID_LEVEL%d", i);
		m_sCompoundItem[i].m_pStaticLevel = GetControl<CEtUIStatic>(szControlName);

		sprintf_s(szControlName, _countof(szControlName), "ID_TYPE%d", i);
		m_sCompoundItem[i].m_pStaticType = GetControl<CEtUIStatic>(szControlName);
	}

	for( int i = 0; i < NUM_MAX_COMPOUND_ITEM; ++i )
		m_sCompoundItem[i].Clear();

	m_pStaticSelect = GetControl<CEtUIStatic>("ID_STATIC_SELECT");
	m_pStaticSelect->Show(false);

	m_pButtonPagePrev = GetControl<CEtUIButton>("ID_PAGE_PREV");
	m_pButtonPageNext = GetControl<CEtUIButton>("ID_PAGE_NEXT");
	m_pStaticPage = GetControl<CEtUIStatic>("ID_PAGE_NUM");
	m_nCurPage = 0;

	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		sprintf_s(szControlName, _countof(szControlName), "%s%d", "ID_ITEM_SLOT", i);
		m_pNeedItemSlotButton[i] = GetControl<CDnItemSlotButton>(szControlName);
		m_pNeedItemSlotButton[i]->SetSlotType( ST_ITEM_COMPOUND_JEWEL );
	}
	m_pGold = GetControl<CEtUIStatic>("ID_GOLD"); 
	m_pSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pBronze = GetControl<CEtUIStatic>("ID_BRONZE");
	m_dwColorGold = m_pGold->GetTextColor();
	m_dwColorSilver = m_pSilver->GetTextColor();
	m_dwColorBronze = m_pBronze->GetTextColor();

	m_pTabButtonWeapon->SetChecked(true);
	m_nCurTab = 0;

	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 46 ), NULL, 1 );
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 47 ), NULL, 2 );
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 49 ), NULL, 3 );
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 48 ), NULL, 4 );
#ifdef PRE_ADD_ACADEMIC
#ifndef PRE_REMOVE_ACADEMIC
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 305 ), NULL, 5 );
#endif
#endif
#if defined( PRE_ADD_KALI ) && !defined( PRE_REMOVE_KALI )
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 306 ), NULL, 6 );
#endif
//[OK]

	// Child_modal is bad because it cannot block inventory input and renders before the inventory.
	// So it seems the shop sell confirmation window is also shown as Modal.
	m_pCompoundMixDlg = new CDnItemCompoundMixDlg( UI_TYPE_MODAL, NULL, COMPOUND_CONFIRM_DIALOG, this );
	m_pCompoundMixDlg->Initialize( false );
}

void CDnItemCompoundTabDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_nCurSelect = -1;
		CheckUsableTab();
		m_nCurPage = 0;
		m_pComboBoxJob->SetSelectedByValue( CDnActor::s_hLocalActor->GetClassID() );
		UpdateList();
	}
	else
	{
		ResetList();
		GetInterface().CloseBlind();
		m_pButtonOK->Enable( false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnItemCompoundTabDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_PAGE_PREV" ) )
		{
			PrevPage();
			UpdateCompoundSlot();
			return;
		}
		else if( IsCmdControl("ID_PAGE_NEXT" ) )
		{
			NextPage();
			UpdateCompoundSlot();
			return;
		}
		else if( IsCmdControl("ID_CANCEL") )
		{
			ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl("ID_CLOSE_DIALOG"), 0 );
			return;
		}
		else if( IsCmdControl("ID_OK") )
		{
			if( m_pButtonOK->IsEnable() )
				ShowConfirmDlg( m_sCompoundItem[m_nCurSelect].m_nCompoundID, m_bOK );
			return;
		}
	}
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( IsCmdControl("ID_TAB_WEAPON") )
		{
			m_nCurTab = 0;
			UpdateList();
			return;
		}
		else if( IsCmdControl("ID_TAB_PARTS") )
		{
			m_nCurTab = 1;
			UpdateList();
			return;
		}
		else if( IsCmdControl("ID_TAB_ACCESSORY") )
		{
			m_nCurTab = 2;
			UpdateList();
			return;
		}
		else if( IsCmdControl("ID_TAB_NORMAL") )
		{
			m_nCurTab = 3;
			UpdateList();
			return;
		}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED ) 
	{
		if( IsCmdControl("ID_COMBOBOX_JOB") )
		{
			UpdateList();
			
		}
		else if( IsCmdControl("ID_COMBOBOX_TYPE") )
		{
			UpdateCompoundList();
			UpdatePage();
			UpdateCompoundSlot();
		}
		// Clear focus from combo box so wheel messages do not go to focus::HandleMouse after changing combo box.
		focus::ReleaseControl();
		return;
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnItemCompoundTabDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	if( m_pComboBoxJob && m_pComboBoxType &&
		!m_pComboBoxJob->IsOpenedDropDownBox() && !m_pComboBoxType->IsOpenedDropDownBox() )
	{
		switch( uMsg )
		{
		case WM_LBUTTONDOWN:
			{
				POINT MousePoint;
				float fMouseX, fMouseY;

				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );
				PointToFloat( MousePoint, fMouseX, fMouseY );

				int nFindSelect = FindInsideItem( fMouseX, fMouseY );
				if( nFindSelect != -1 )
				{
					if( m_sCompoundItem[nFindSelect].m_pStaticBase->IsShow() )
					{
						m_nCurSelect = nFindSelect;
						UpdateSelectBar();
						return true;
					}
					else
					{
						for( int i = 0; i < NUM_NEED_ITEM; ++i )
							m_pNeedItemSlotButton[i]->ResetSlot();

						m_pGold->SetIntToText( 0 );
						m_pSilver->SetIntToText( 0 );
						m_pBronze->SetIntToText( 0 );
						m_pGold->SetTextColor( m_dwColorGold );
						m_pSilver->SetTextColor( m_dwColorSilver );
						m_pBronze->SetTextColor( m_dwColorBronze );

						m_pStaticSelect->Show(false);
						m_nCurSelect = -1;
						m_pButtonOK->Enable( false );
					}
				}
			}
			break;
		case WM_LBUTTONDBLCLK:
			if( !IsMouseInDlg() ) break;
			ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_OK"), 0 );
			return true;
		case WM_MOUSEWHEEL:
			{
				POINT MousePoint;
				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );

				ScreenToClient( hWnd, &MousePoint );
				lParam = MAKELPARAM( MousePoint.x, MousePoint.y );

				if( IsMouseInDlg() )
				{
					UINT uLines;
					SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
					int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
					if( nScrollAmount > 0 )
					{
						if( m_nCurPage > 0 )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_PAGE_PREV"), 0 );
					}
					else if( nScrollAmount < 0 )
					{
						if( m_nCurPage < m_nMaxPage )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_PAGE_NEXT"), 0 );
					}

					// Update tooltip on mouse move message.
					CEtUIDialogBase::StaticMsgProc( CDnMainFrame::GetInstance().GetHWnd(), WM_MOUSEMOVE, 0, (LPARAM)lParam );
					return true;
				}
			}
			break;
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnItemCompoundTabDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		switch( nID ) 
		{
		case COMPOUND_CONFIRM_DIALOG:
			{
				if( IsCmdControl("ID_OK") )
				{
					if( !GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG ) )
					{
						GetInterface().MessageBox( 7012, MB_OK );
						return;
					}

					// After sending registration request to server, show cutscene movie upon receiving the corresponding packet.
					OutputDebug( "Send item compound request packet\n" );
					GetItemTask().RequestItemCompoundOpen( m_sCompoundItem[m_nCurSelect].m_nCompoundID, m_vNeedItemInfo );
					m_pCompoundMixDlg->EnableButton( false );
//					m_pCompoundMixDlg->Show( false );
				}
				else if( IsCmdControl("ID_CANCEL") || IsCmdControl("ID_CLOSE_DIALOG") )
				{
					m_pCompoundMixDlg->Show( false );
				}
			}
			break;
		case COMPOUND_MOVIE_DIALOG:
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				// Invisible button. Indicates that processing has finished.
				if( IsCmdControl("ID_PROCESS_COMPLETE") )
				{
					// Send completion packet.
					// Send...
					GetItemTask().RequestItemCompound( m_sCompoundItem[m_nCurSelect].m_nCompoundID, m_vNeedItemInfo );
					OutputDebug( "Send actual item compound request packet\n" );
				}
				else if( IsCmdControl("ID_BUTTON_CANCEL") )
				{
					// Send cancel packet.
					// Send...
					OutputDebug( "Send cancel packet during heraldry jewel compound\n" );
					GetItemTask().RequestItemCompoundCancel();
				}
			}
			break;
		}
	}
}

int CDnItemCompoundTabDlg::FindInsideItem( float fX, float fY )
{
	for( int i = 0; i < NUM_MAX_COMPOUND_ITEM; i++ )
	{
		if( m_sCompoundItem[i].IsInsideItem( fX, fY ) )
		{
			return i;
		}
	}

	return -1;
}

void CDnItemCompoundTabDlg::UpdateSelectBar()
{
	SUICoord uiCoordBase, uiCoordSelect;
	m_sCompoundItem[m_nCurSelect].m_pStaticBase->GetUICoord(uiCoordBase);
	m_pStaticSelect->GetUICoord(uiCoordSelect);
	uiCoordSelect.fX = uiCoordBase.fX - 0.0039f;
	uiCoordSelect.fY = uiCoordBase.fY- 0.005f;
	m_pStaticSelect->SetUICoord(uiCoordSelect);
	m_pStaticSelect->Show(true);

	CheckCompound();
}

void CDnItemCompoundTabDlg::SetCompoundListID( int nListID )
{
	m_nCurCompoundListID = nListID;
	m_vecCompoundID.clear();

	// Read from the table.
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUNDSHOP );
	if( !pSox ) return;

	vector<int> vCompoundShotItemIDList;
	int nNum = pSox->GetItemIDListFromField( "_ListID", nListID, vCompoundShotItemIDList );
	if( nNum <= 0 )
	{
		ASSERT(0&&"Total count of compound IDs obtained by compound NPC index is 0 or less. Invalid data.");
	}

	for( int i = 0; i < (int)vCompoundShotItemIDList.size(); ++i )
		m_vecCompoundID.push_back( pSox->GetFieldFromLablePtr( vCompoundShotItemIDList[i], "_ItemCompoundID" )->GetInteger() );
}

void CDnItemCompoundTabDlg::ResetList()
{
	m_nCurCompoundListID = -1;
	m_vecCompoundID.clear();
	for( int i = 0; i < NUM_MAX_COMPOUND_ITEM; ++i )
		m_sCompoundItem[i].Clear();

	for( int i = 0; i < NUM_NEED_ITEM; ++i )
		m_pNeedItemSlotButton[i]->ResetSlot();

	m_pGold->SetIntToText( 0 );
	m_pSilver->SetIntToText( 0 );
	m_pBronze->SetIntToText( 0 );
	m_pGold->SetTextColor( m_dwColorGold );
	m_pSilver->SetTextColor( m_dwColorSilver );
	m_pBronze->SetTextColor( m_dwColorBronze );

	m_nCurSelect = -1;
	m_pStaticSelect->Show( false );
}

void CDnItemCompoundTabDlg::CheckUsableTab()
{
	// Related to issue #20161.
	// Originally, check items in each tab and hide the tab if it has no items,
	// Afterwards, if only one tab remains, select that tab and hide all tabs.
	bool bTab[4] = {0,};
	for( int i = 0; i < (int)m_vecCompoundID.size(); ++i )
	{
		CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
		GetItemTask().GetCompoundInfo( m_vecCompoundID[i], &CompoundInfo );
		int nItemID = CompoundInfo.aiSuccessItemID[0];

		// CreateItem has too much overhead just to create an item from this item ID, so fetch the data directly.
		// First filter by tab.
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
		if( !pSox ) return;
		if( !pSox->IsExistItem( nItemID ) ) continue;
		eItemTypeEnum ItemType = (eItemTypeEnum)pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger();

		if( ItemType == ITEMTYPE_WEAPON ) bTab[0] = true;
		else if( ItemType == ITEMTYPE_PARTS )
		{
			DNTableFileFormat* pSoxParts = GetDNTable( CDnTableDB::TPARTS );
			if( !pSoxParts ) return;
			if( !pSoxParts->IsExistItem( nItemID ) ) continue;
			CDnParts::PartsTypeEnum PartsType = (CDnParts::PartsTypeEnum)pSoxParts->GetFieldFromLablePtr( nItemID, "_Parts" )->GetInteger();
			if( PartsType >= CDnParts::Helmet && PartsType <= CDnParts::Foot ) bTab[1] = true;
			if( PartsType >= CDnParts::Necklace && PartsType <= CDnParts::Ring ) bTab[2] = true;
		}
		else if( 1 )
		{
			// If it reaches here, there is an item in the normal tab. Do not check type, because items other than CDnItem::Normal can also be crafted.
			bTab[3] = true;
		}
	}

	int nSum = 0;
	int nLastIndex = -1;
	for( int i = 0; i < sizeof(bTab); ++i )
	{
		nSum += (int)bTab[i];
		if( bTab[i] ) nLastIndex = i;
	}

	if( nSum == 1 )
	{
		m_pTabButtonWeapon->Show( false );
		m_pTabButtonParts->Show( false );
		m_pTabButtonAccessory->Show( false );
		m_pTabButtonNormal->Show( false );

		switch( nLastIndex )
		{
		case 0:	m_pTabButtonWeapon->SetChecked( true );		break;
		case 1:	m_pTabButtonParts->SetChecked( true );		break;
		case 2:	m_pTabButtonAccessory->SetChecked( true );	break;
		case 3:	m_pTabButtonNormal->SetChecked( true );		break;
		}
		m_nCurTab = nLastIndex;
	}
	else
	{
		// When craft lists exist in two or more tabs, use default behavior (also do this if no craft lists exist).
		m_pTabButtonWeapon->Show( true );
		m_pTabButtonParts->Show( true );
		m_pTabButtonAccessory->Show( true );
		m_pTabButtonNormal->Show( true );

		m_pTabButtonWeapon->SetChecked( true );
		m_nCurTab = 0;
	}
}

void CDnItemCompoundTabDlg::UpdateList()
{
	UpdateComboBoxType();
	UpdateCompoundList();
	UpdatePage();
	UpdateCompoundSlot();
}

void CDnItemCompoundTabDlg::UpdateComboBoxType()
{
	if( !m_pComboBoxType ) return;

	m_pComboBoxType->RemoveAllItems();

	// Content changes depending on the currently selected tab and combo box job.
	std::vector<int> vecType;
	if( m_nCurTab == 0 )
	{
		SComboBoxItem *pItem = m_pComboBoxJob->GetSelectedItem();
		if( !pItem ) return;

		switch( pItem->nValue )
		{
		case 1:
			vecType.push_back(CDnWeapon::Sword);
			vecType.push_back(CDnWeapon::Axe);
			vecType.push_back(CDnWeapon::Hammer);
			vecType.push_back(CDnWeapon::Gauntlet);
			break;
		case 2:
			vecType.push_back(CDnWeapon::SmallBow);
			vecType.push_back(CDnWeapon::BigBow);
			vecType.push_back(CDnWeapon::CrossBow);
			vecType.push_back(CDnWeapon::Arrow);
			break;
		case 3:
			vecType.push_back(CDnWeapon::Staff);
			vecType.push_back(CDnWeapon::Orb);
			vecType.push_back(CDnWeapon::Puppet);
			vecType.push_back(CDnWeapon::Book);
			break;
		case 4:
			vecType.push_back(CDnWeapon::Mace);
			vecType.push_back(CDnWeapon::Flail);
			vecType.push_back(CDnWeapon::Wand);
			vecType.push_back(CDnWeapon::Shield);
			break;
		case 5:
			vecType.push_back(CDnWeapon::Cannon);
			vecType.push_back(CDnWeapon::BubbleGun);
			vecType.push_back(CDnWeapon::Glove);
			break;
		case 6:
			vecType.push_back(CDnWeapon::Fan);
			vecType.push_back(CDnWeapon::Chakram);
			vecType.push_back(CDnWeapon::Charm);
			break;
		case 7:
			vecType.push_back(CDnWeapon::Scimiter);
			vecType.push_back(CDnWeapon::Dagger);
			vecType.push_back(CDnWeapon::Crook);
			break;
		case 8:
			vecType.push_back(CDnWeapon::Spear);
			vecType.push_back(CDnWeapon::Bracelet);
			break;
		case 9:
			vecType.push_back(CDnWeapon::KnuckleGear);
			vecType.push_back(CDnWeapon::Claw);
			break;
		}
#pragma message(" ------> DNItemCompoundTabDLG 589 Add other items !")
		m_pComboBoxType->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 79 ), NULL, 99 );
		for( int i = 0; i < (int)vecType.size(); ++i )
			m_pComboBoxType->AddItem( DN_INTERFACE::STRING::ITEM::GetEquipString( (CDnWeapon::EquipTypeEnum)vecType[i]), NULL, vecType[i] );
	}
	else if( m_nCurTab == 1 )
	{
		SComboBoxItem *pItem = m_pComboBoxJob->GetSelectedItem();
		if( !pItem ) return;

		// If Show All is selected, should all Types be shown too?
		switch( pItem->nValue )
		{
		case 1:
		case 2:
		case 3:
		case 4:
			vecType.push_back(CDnParts::Helmet);
			vecType.push_back(CDnParts::Body);
			vecType.push_back(CDnParts::Leg);
			vecType.push_back(CDnParts::Hand);
			vecType.push_back(CDnParts::Foot);
			break;
		}

		m_pComboBoxType->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 79 ), NULL, 99 );
		for( int i = 0; i < (int)vecType.size(); ++i )
			m_pComboBoxType->AddItem( DN_INTERFACE::STRING::ITEM::GetPartsString( (CDnParts::PartsTypeEnum)vecType[i]), NULL, vecType[i] );
	}
	else if( m_nCurTab == 2 )
	{
		SComboBoxItem *pItem = m_pComboBoxJob->GetSelectedItem();
		if( !pItem ) return;

		// If Show All is selected, should all Types be shown too?
		switch( pItem->nValue )
		{
		case 1:
		case 2:
		case 3:
		case 4:
			vecType.push_back(CDnParts::Necklace);
			vecType.push_back(CDnParts::Earring);
			vecType.push_back(CDnParts::Ring);
			break;
		}

		m_pComboBoxType->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 79 ), NULL, 99 );
		for( int i = 0; i < (int)vecType.size(); ++i )
			m_pComboBoxType->AddItem( DN_INTERFACE::STRING::ITEM::GetPartsString( (CDnParts::PartsTypeEnum)vecType[i]), NULL, vecType[i] );
	}
	else if( m_nCurTab == 3 )
	{
		SComboBoxItem *pItem = m_pComboBoxJob->GetSelectedItem();
		if( !pItem ) return;

		// If Show All is selected, should all Types be shown too?
		switch( pItem->nValue )
		{
		case 1:
		case 2:
		case 3:
		case 4:
			vecType.push_back(ITEMRANK_D);
			vecType.push_back(ITEMRANK_C);
			vecType.push_back(ITEMRANK_B);
			vecType.push_back(ITEMRANK_A);
			vecType.push_back(ITEMRANK_S);
			break;
		}

		m_pComboBoxType->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 79 ), NULL, 99 );
		for( int i = 0; i < (int)vecType.size(); ++i )
			m_pComboBoxType->AddItem( DN_INTERFACE::STRING::ITEM::GetRankString( (eItemRank)vecType[i]), NULL, vecType[i] );
	}

	m_pComboBoxType->SetSelectedByIndex(0);
}

static bool CompareCompoundInfo( int nCompoundID1, int nCompoundID2 )
{
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo1, CompoundInfo2;
	GetItemTask().GetCompoundInfo( nCompoundID1, &CompoundInfo1 );
	GetItemTask().GetCompoundInfo( nCompoundID2, &CompoundInfo2 );
	int nResultItemID1 = CompoundInfo1.aiSuccessItemID[0];
	int nResultItemID2 = CompoundInfo2.aiSuccessItemID[0];

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
	if( !pSox ) return false;
	if( !pSox->IsExistItem( nResultItemID1 ) ) return false;
	if( !pSox->IsExistItem( nResultItemID2 ) ) return false;

	int nLevelLimit1 = pSox->GetFieldFromLablePtr( nResultItemID1, "_LevelLimit" )->GetInteger();
	int nLevelLimit2 = pSox->GetFieldFromLablePtr( nResultItemID2, "_LevelLimit" )->GetInteger();

	if( nLevelLimit1 < nLevelLimit2 ) return true;
	else if( nLevelLimit1 > nLevelLimit2 ) return false;

	// If level limit is the same, keep as original 
	if( CompoundInfo1.iCompoundTableID < CompoundInfo2.iCompoundTableID ) return true;
	else if( CompoundInfo1.iCompoundTableID > CompoundInfo2.iCompoundTableID ) return false;

	return false;
}

void CDnItemCompoundTabDlg::UpdateCompoundList()
{
	m_vecCompoundIDinList.clear();

	// Select the combination list to display according to the currently selected tab, job, and type.
	for( int i = 0; i < (int)m_vecCompoundID.size(); ++i )
	{
		CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
		GetItemTask().GetCompoundInfo( m_vecCompoundID[i], &CompoundInfo );
		int nItemID = CompoundInfo.aiSuccessItemID[0];

		// CreateItem has too much overhead just to create an item from this item ID, so fetch the data directly.
		// First filter by tab.
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
		if( !pSox ) return;
		if( !pSox->IsExistItem( nItemID ) ) continue;
		eItemTypeEnum ItemType = (eItemTypeEnum)pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger();

		if( m_nCurTab == 0 && ItemType != ITEMTYPE_WEAPON ) continue;
		else if( m_nCurTab == 1 && ItemType != ITEMTYPE_PARTS ) continue;
		else if( m_nCurTab == 2 && ItemType != ITEMTYPE_PARTS ) continue;	// Accessories also belong to Parts.
		else if( m_nCurTab == 3 && (ItemType == ITEMTYPE_WEAPON || ItemType == ITEMTYPE_PARTS) ) continue;

		// Select the job.
		{
			std::vector<int> vecPermitJobList;
			char *szPermitStr = pSox->GetFieldFromLablePtr( nItemID, "_NeedJobClass" )->GetString();
			int nJobIndex;
			for( int i=0;; i++ ) {
				//char *pStr = _GetSubStrByCount( i, szPermitStr, ';' );
				std::string strValue = _GetSubStrByCountSafe( i, szPermitStr, ';' );
				if( strValue.size() == 0 ) break;
				nJobIndex = atoi(strValue.c_str());
				if( nJobIndex < 1 ) continue;
				vecPermitJobList.push_back( nJobIndex );
			}
			int nCurJob;
			if( m_pComboBoxJob->GetSelectedValue(nCurJob) )
			{
				if( (int)vecPermitJobList.size() == 1 && nCurJob != vecPermitJobList[0] )
					continue;
			}
		}
		// Filter out the type.
		int nCurType;
		if( m_nCurTab == 0 )
		{
			// If weapon, check on weapon side
			DNTableFileFormat* pSoxWeapon = GetDNTable( CDnTableDB::TWEAPON );
			if( !pSoxWeapon ) return;
			if( !pSoxWeapon->IsExistItem( nItemID ) ) continue;
			CDnWeapon::EquipTypeEnum EquipType = (CDnWeapon::EquipTypeEnum)pSoxWeapon->GetFieldFromLablePtr( nItemID, "_EquipType" )->GetInteger();
			if( m_pComboBoxType->GetSelectedValue(nCurType) )
			{
				if( nCurType != 99 && nCurType != EquipType )
					continue;
			}
		}
		else if( m_nCurTab == 1 )
		{
			DNTableFileFormat* pSoxParts = GetDNTable( CDnTableDB::TPARTS );
			if( !pSoxParts ) return;
			if( !pSoxParts->IsExistItem( nItemID ) ) continue;
			CDnParts::PartsTypeEnum PartsType = (CDnParts::PartsTypeEnum)pSoxParts->GetFieldFromLablePtr( nItemID, "_Parts" )->GetInteger();
			if( m_pComboBoxType->GetSelectedValue(nCurType) )
			{
				if( PartsType < CDnParts::Helmet || PartsType > CDnParts::Foot )
					continue;
				if( nCurType != 99 && nCurType != PartsType )
					continue;
			}
		}
		else if( m_nCurTab == 2 )
		{
			DNTableFileFormat* pSoxParts = GetDNTable( CDnTableDB::TPARTS );
			if( !pSoxParts ) return;
			if( !pSoxParts->IsExistItem( nItemID ) ) continue;
			CDnParts::PartsTypeEnum PartsType = (CDnParts::PartsTypeEnum)pSoxParts->GetFieldFromLablePtr( nItemID, "_Parts" )->GetInteger();
			if( m_pComboBoxType->GetSelectedValue(nCurType) )
			{
				if( PartsType < CDnParts::Necklace || PartsType > CDnParts::Ring )
					continue;
				if( nCurType != 99 && nCurType != PartsType )
					continue;
			}
		}
		else if( m_nCurTab == 3 )
		{
			DNTableFileFormat* pSoxParts = GetDNTable( CDnTableDB::TITEM );
			if( !pSoxParts ) return;
			if( !pSoxParts->IsExistItem( nItemID ) ) continue;
			eItemRank Rank = (eItemRank)pSox->GetFieldFromLablePtr( nItemID, "_Rank" )->GetInteger();
			if( m_pComboBoxType->GetSelectedValue(nCurType) )
			{
				if( nCurType != 99 && nCurType != Rank )
					continue;
			}
		}
		else
		{
			// Simply skip other invalid data.
			continue;
		}

		// Items that passed all checks..
		m_vecCompoundIDinList.push_back(m_vecCompoundID[i]);
	}

	// Sort list. Ascending by level.
	std::sort( m_vecCompoundIDinList.begin(), m_vecCompoundIDinList.end(), CompareCompoundInfo );

	// Reset page since the list was updated.
	m_nCurPage = 0;

	// Set max page (stores the highest index: 0 for one page, 1 for two pages).
	m_nMaxPage = ((int)m_vecCompoundIDinList.size()-1) / NUM_MAX_COMPOUND_ITEM;
}

void CDnItemCompoundTabDlg::UpdatePage()
{
	wchar_t wszPage[10]={0};
	swprintf_s( wszPage, 10, L"%d/%d", m_nCurPage+1, m_nMaxPage+1 );
	m_pStaticPage->SetText( wszPage );

	if( m_nCurPage == m_nMaxPage )
		m_pButtonPageNext->Enable(false);
	else
		m_pButtonPageNext->Enable(true);

	if( m_nCurPage == 0 )
		m_pButtonPagePrev->Enable(false);
	else
		m_pButtonPagePrev->Enable(true);
}

void CDnItemCompoundTabDlg::UpdateCompoundSlot()
{
	for( int i = 0; i < NUM_MAX_COMPOUND_ITEM; ++i )
		m_sCompoundItem[i].Clear();

	for( int i = 0; i < NUM_NEED_ITEM; ++i )
		m_pNeedItemSlotButton[i]->ResetSlot();

	m_pGold->SetIntToText( 0 );
	m_pSilver->SetIntToText( 0 );
	m_pBronze->SetIntToText( 0 );
	m_pGold->SetTextColor( m_dwColorGold );
	m_pSilver->SetTextColor( m_dwColorSilver );
	m_pBronze->SetTextColor( m_dwColorBronze );

	int nOffset = m_nCurPage * NUM_MAX_COMPOUND_ITEM;

	// Show only necessary items.
	for( int i = nOffset; i < (int)m_vecCompoundIDinList.size(); ++i )
	{
		if( i >= nOffset + NUM_MAX_COMPOUND_ITEM )
			break;

		CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
		GetItemTask().GetCompoundInfo( m_vecCompoundIDinList[i], &CompoundInfo );
		if( CompoundInfo.aiSuccessItemID[0] > 0 )
			m_sCompoundItem[i-nOffset].SetInfo( CompoundInfo.iCompoundTableID, CompoundInfo.aiSuccessItemID[0] );
	}

	UpdateCompoundableSlot();

	// When the slot is updated, reset the selection as well.
	m_nCurSelect = -1;
	m_pStaticSelect->Show( false );
	m_pButtonOK->Enable( false );
}

void CDnItemCompoundTabDlg::UpdateCompoundableSlot()
{
	// Check if craftable.
	for( int i = 0; i < NUM_MAX_COMPOUND_ITEM; ++i )
	{
		if( m_sCompoundItem[i].m_nCompoundID && !m_sCompoundItem[i].m_pItemSlotButton->IsEmptySlot() )
		{
			if( CheckCompound( m_sCompoundItem[i].m_nCompoundID ) )
			{
				m_sCompoundItem[i].m_pItemSlotButton->SetRegist( false );
				m_sCompoundItem[i].m_pStaticLevelText->Enable( true );
				m_sCompoundItem[i].m_pStaticLevel->Enable( true );
			}
			else
			{
				m_sCompoundItem[i].m_pItemSlotButton->SetRegist( true );
				m_sCompoundItem[i].m_pStaticLevelText->Enable( false );
				m_sCompoundItem[i].m_pStaticLevel->Enable( false );
			}
		}
	}
}

void CDnItemCompoundTabDlg::PrevPage()
{
	--m_nCurPage;
	if( m_nCurPage < 0 )
		m_nCurPage = 0;

	UpdatePage();
}

void CDnItemCompoundTabDlg::NextPage()
{
	++m_nCurPage;
	if( m_nCurPage > m_nMaxPage )
		m_nCurPage = m_nMaxPage;

	UpdatePage();
}

void CDnItemCompoundTabDlg::CheckCompound()
{
	m_bOK = false;
	m_pButtonOK->Enable( true );
	if( 0 <= m_nCurSelect && m_nCurSelect < NUM_MAX_COMPOUND_ITEM )
	{
		int nCompoundID = m_sCompoundItem[m_nCurSelect].m_nCompoundID;
		CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
		GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

		// Show fee.
		int nGold = (int)(CompoundInfo.iCost/10000);
		int nSilver = (int)((CompoundInfo.iCost%10000)/100);
		int nBronze = (int)(CompoundInfo.iCost%100);
		m_pGold->SetIntToText( nGold );
		m_pSilver->SetIntToText( nSilver );
		m_pBronze->SetIntToText( nBronze );

		// Retrieve and check the fee.
		bool bEnoughCoin = false;
		if( GetItemTask().GetCoin() >= CompoundInfo.iCost )
		{
			bEnoughCoin = true;
			m_pGold->SetTextColor( m_dwColorGold );
			m_pSilver->SetTextColor( m_dwColorSilver );
			m_pBronze->SetTextColor( m_dwColorBronze );
		}
		else
		{
			if( CompoundInfo.iCost >= 10000 ) m_pGold->SetTextColor( 0xFFFF0000, true );
			if( CompoundInfo.iCost >= 100 ) m_pSilver->SetTextColor( 0xFFFF0000, true );
			m_pBronze->SetTextColor( 0xFFFF0000, true );
		}

		bool bNeedItemResult = true;

		// First initialize the configured required item slots.
		for( int i = 0; i < NUM_NEED_ITEM; ++i )
			m_pNeedItemSlotButton[i]->ResetSlot();

		// Set up the required item info struct to send to server.
		m_vNeedItemInfo.clear();
		for( int i = 0; i < NUM_NEED_ITEM; ++i )
		{
			if( CompoundInfo.aiItemID[i] > 0 && CompoundInfo.aiItemCount[i] > 0 )
			{
				CDnItemTask::ItemCompoundInfo Info;
				Info.iItemID = CompoundInfo.aiItemID[ i ];
				Info.iUseCount = CompoundInfo.aiItemCount[ i ];
				m_vNeedItemInfo.push_back(Info);

				// Additionally check inventory for item types and counts.
				int nCurItemCount = GetItemTask().GetCharInventory().GetItemCount( CompoundInfo.aiItemID[i] );

				// Set material item slot.
				SetNeedItemSlot( i, CompoundInfo.aiItemID[i], CompoundInfo.aiItemCount[i], nCurItemCount );

				// Returns false if any one type is missing.
				if( nCurItemCount < CompoundInfo.aiItemCount[i] )
					bNeedItemResult = false;
			}
		}

		if( bEnoughCoin && bNeedItemResult )
			m_bOK = true;
	}
}

bool CDnItemCompoundTabDlg::CheckCompound( int nCompoundID )
{
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

	bool bEnoughCoin = false;
	if( GetItemTask().GetCoin() >= CompoundInfo.iCost )
		bEnoughCoin = true;

	bool bNeedItemResult = true;
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		if( CompoundInfo.aiItemID[i] > 0 && CompoundInfo.aiItemCount[i] > 0 )
		{
			int nCurItemCount = GetItemTask().GetCharInventory().GetItemCount( CompoundInfo.aiItemID[i] );
			if( nCurItemCount < CompoundInfo.aiItemCount[i] )
				bNeedItemResult = false;
		}
	}

	if( bEnoughCoin && bNeedItemResult )
		return true;

	return false;
}

void CDnItemCompoundTabDlg::SetNeedItemSlot( int nSlotIndex, int nItemID, int nNeedItemCount, int nCurItemCount )
{
	if( nItemID == 0 )
		return;

	SAFE_DELETE( m_pNeedItem[nSlotIndex] );

	TItemInfo itemInfo;
	if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) return;
	m_pNeedItem[nSlotIndex] = GetItemTask().CreateItem( itemInfo );
	m_pNeedItemSlotButton[nSlotIndex]->SetItem(m_pNeedItem[nSlotIndex], CDnSlotButton::ITEM_ORIGINAL_COUNT);
	m_pNeedItemSlotButton[nSlotIndex]->SetJewelCount( nNeedItemCount, nCurItemCount );

	if( nCurItemCount < nNeedItemCount )
		m_pNeedItemSlotButton[nSlotIndex]->SetRegist( true );
	else
		m_pNeedItemSlotButton[nSlotIndex]->SetRegist( false );
}

void CDnItemCompoundTabDlg::ShowConfirmDlg( int nCompoundID, bool bOK )
{
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );
	m_pCompoundMixDlg->SetCompoundInfo( CompoundInfo.aiItemID, CompoundInfo.aiItemCount );
	m_pCompoundMixDlg->Show( true );

	m_pCompoundMixDlg->SetPrice( (int)CompoundInfo.iCost );

	CDnSlotButton *pDragButton = (CDnSlotButton *)drag::GetControl();
	if( pDragButton ) {
		pDragButton->DisableSplitMode(true);
		drag::ReleaseControl();
	}

	int nProb = (int)(CompoundInfo.fSuccessPossibility * 100.0f);
	if( CompoundInfo.fSuccessPossibility * 100.0f - (float)nProb > 0.999 ) nProb += 1;
	m_pCompoundMixDlg->SetProb( nProb );
	m_pCompoundMixDlg->SetOK( bOK );
}

void CDnItemCompoundTabDlg::OnRecvItemCompoundOpen()
{
	// After obtaining the appropriate duration according to the item to combine
	int nCompoundID = m_sCompoundItem[m_nCurSelect].m_nCompoundID;
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

	GetInterface().OpenMovieProcessDlg(
		"ItemCombine.avi", CompoundInfo.fTimeRequired / 1000.0f, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8019 ), COMPOUND_MOVIE_DIALOG, this );

	m_pCompoundMixDlg->EnableButton( true );
	m_pCompoundMixDlg->Show( false );
}

void CDnItemCompoundTabDlg::OnRecvItemCompound()
{
	// Return to default step.
	m_nCurSelect = -1;
	m_pStaticSelect->Show( false );
	m_pButtonOK->Enable( false );

	for( int i = 0; i < NUM_NEED_ITEM; ++i )
		m_pNeedItemSlotButton[i]->ResetSlot();

	m_pGold->SetIntToText( 0 );
	m_pSilver->SetIntToText( 0 );
	m_pBronze->SetIntToText( 0 );

	// Recheck if craftable since items and gold decreased after crafting.
	UpdateCompoundableSlot();

	m_bOK = false;
}