#include "StdAfx.h"
#include "DnItemCompoundTab2Dlg.h"
#include "DnTableDB.h"
#include "DnInterfaceString.h"
#include "DnMainFrame.h"
#include "DnItemCompoundMixDlg.h"
#include "DnItemCompoundMix2Dlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "VillageSendPacket.h"
#include "DnLocalPlayerActor.h"
#include "ItemSendPacket.h"
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
#include "DnItemCompoundNotifyDlg.h"
#endif
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#endif

using namespace DN_INTERFACE::STRING;

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnItemCompoundTab2Dlg::CDnItemCompoundTab2Dlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
{
	m_nSelectPartsType = 0;
	m_nSelectJobType = 0;
	m_nSelectSortType = 0;
	m_bCheckHideLowLevel = false;
	m_bCheckShowPossibleOnly = false;

	m_pComboBoxJob = NULL;
	m_pComboBoxSort = NULL;
	m_pCheckBoxLowLevel = NULL;
	m_pCheckBoxPossibleOnly = NULL;

	for( int i=0; i<5; i++ ) {
		m_pNeedItem[i] = NULL;
		m_pNeedItemSlotButton[i] = NULL;
	}

	m_pCompoundMixDlg = NULL;
	m_pCompoundMix2Dlg = NULL;

	m_nCurGroupPage = 0;
	m_nSelectGroup = 0;

	m_nCurOptionPage = 0;
	m_nSelectOption = 0;

	m_pButtonCancel = NULL;
	m_pButtonOK = NULL;
	m_pStaticBronze = NULL;
	m_pStaticGold = NULL;
	m_pStaticGroupPage = NULL;
	m_pStaticOptionDesc = NULL;
	m_pStaticOptionPage = NULL;
	m_pStaticSelectGroup = NULL;
	m_pStaticSelectOption = NULL;
	m_pStaticSilver = NULL;
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	m_pTextNotifyItem = NULL;
	m_pCheckNotifyItem = NULL;
	m_pButtonClose = NULL;
	m_pRadioButtonCreate = NULL;
	m_pRadioButtonEnchant = NULL;
	m_bForceOpenMode = false;
#endif

	m_nRemoteItemID = 0;
}

CDnItemCompoundTab2Dlg::~CDnItemCompoundTab2Dlg()
{
	for( int i=0; i<5; i++ )
		SAFE_DELETE( m_pNeedItem[i] );

	SAFE_DELETE( m_pCompoundMixDlg );
	SAFE_DELETE( m_pCompoundMix2Dlg );
}

void CDnItemCompoundTab2Dlg::InitialUpdate()
{
	m_pComboBoxJob = GetControl<CEtUIComboBox>("ID_COMBOBOX_JOB");
	m_pComboBoxSort = GetControl<CEtUIComboBox>("ID_COMBOBOX_SORT");
	m_pCheckBoxLowLevel = GetControl<CEtUICheckBox>("ID_CHECKBOX_LEVEL");
	m_pCheckBoxPossibleOnly = GetControl<CEtUICheckBox>("ID_CHECKBOX_LIST");

	m_pStaticGroupPage = GetControl<CEtUIStatic>("ID_TEXT_PAGE");
	m_pStaticSelectGroup = GetControl<CEtUIStatic>("ID_LIST_SELECT");

	m_pStaticOptionPage = GetControl<CEtUIStatic>("ID_PAGE_NUM");
	m_pStaticSelectOption = GetControl<CEtUIStatic>("ID_STATIC_SELECT");
	m_pStaticOptionDesc = GetControl<CEtUIStatic>("ID_TEXT_OPTION");

	m_pStaticGold = GetControl<CEtUIStatic>("ID_GOLD"); 
	m_pStaticSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pStaticBronze = GetControl<CEtUIStatic>("ID_BRONZE");
	m_dwColorGold = m_pStaticGold->GetTextColor();
	m_dwColorSilver = m_pStaticSilver->GetTextColor();
	m_dwColorBronze = m_pStaticBronze->GetTextColor();

	m_pButtonOK = GetControl<CEtUIButton>("ID_OK"); 
	m_pButtonCancel = GetControl<CEtUIButton>("ID_CANCEL"); 

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
//[OK]

#if defined( PRE_ADD_LEGEND_ITEM_UI )
	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 79 ), NULL, 1 );			// UISTRING : All
	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2271 ), NULL, 2 );		// UISTRING : Magic
	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2272 ), NULL, 3 );		// UISTRING : Rare
	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2273 ), NULL, 4 );		// UISTRING : Epic

	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2274 ), NULL, 5 );		// UISTRING : Unique

#if defined(_WORK)
	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2212 ), NULL, 6 );		// UISTRING : Legend
#endif
#else // #if defined( PRE_ADD_LEGEND_ITEM_UI )
	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3249 ), NULL, 1, true );
	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3250 ), NULL, 2, true );
	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3251 ), NULL, 3, true );
	m_pComboBoxSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3252 ), NULL, 4, true );
#endif	// #if defined( PRE_ADD_LEGEND_ITEM_UI )

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	m_pButtonClose = GetControl<CEtUIButton>("ID_BT_CLOSE");
	m_pButtonClose->Show(false);
	m_pTextNotifyItem = GetControl<CEtUIStatic>("ID_STATIC13");
	m_pCheckNotifyItem = GetControl<CEtUICheckBox>("ID_CHECKBOX_INFO");
	m_pRadioButtonCreate = GetControl<CEtUIRadioButton>("ID_RBT_BASETAB0");
	m_pRadioButtonCreate->Show(false);
	m_pRadioButtonEnchant = GetControl<CEtUIRadioButton>("ID_RBT_BASETAB1");
	m_pRadioButtonEnchant->Show(false);
#endif


	char szControlName[32];
	for( int i=0; i<6; i++ ) {
		sprintf_s( szControlName, _countof(szControlName), "ID_LIST_SLOT%d", i );
		m_CompoundGroupList[i].pItemSlotButton = GetControl<CDnItemSlotButton>(szControlName);
		m_CompoundGroupList[i].pItemSlotButton->SetSlotType( ST_ITEM_COMPOUND );
		m_CompoundGroupList[i].pItemSlotButton->SetBlendRate( 0.f );

		sprintf_s(szControlName, _countof(szControlName), "ID_LIST_BAR%d", i);
		m_CompoundGroupList[i].pStaticBase = GetControl<CEtUIStatic>(szControlName);
		m_CompoundGroupList[i].pStaticBase->SetBlendRate( 0.f );

		sprintf_s(szControlName, _countof(szControlName), "ID_LIST_NAME%d", i);
		m_CompoundGroupList[i].pStaticName = GetControl<CEtUIStatic>(szControlName);
		m_CompoundGroupList[i].pStaticName->SetBlendRate( 0.f );

		sprintf_s(szControlName, _countof(szControlName), "ID_LIST_LEVEL%d", i);
		m_CompoundGroupList[i].pStaticLevel = GetControl<CEtUIStatic>(szControlName);
		m_CompoundGroupList[i].pStaticLevel->SetBlendRate( 0.f );

		sprintf_s(szControlName, _countof(szControlName), "ID_TEXT_LEVEL%d", i);
		m_CompoundGroupList[i].pStaticLevelText = GetControl<CEtUIStatic>(szControlName);
		m_CompoundGroupList[i].pStaticLevelText->SetBlendRate( 0.f );

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		sprintf_s(szControlName, _countof(szControlName), "ID_STATIC_LIST_FLAG%d", i);
		m_CompoundGroupList[i].pStaticNotifyFlag = GetControl<CEtUIStatic>(szControlName);
		m_CompoundGroupList[i].pStaticNotifyFlag->Show( false );
#endif

		m_CompoundGroupList[i].Clear();
	}


	for( int i=0; i<6; i++ ) {
		sprintf_s( szControlName, _countof(szControlName), "ID_SLOT%d", i );
		m_CompoundOptionList[i].pItemSlotButton = GetControl<CDnItemSlotButton>(szControlName);
		m_CompoundOptionList[i].pItemSlotButton->SetSlotType( ST_ITEM_COMPOUND );
		m_CompoundOptionList[i].pItemSlotButton->SetBlendRate( 0.f );

		sprintf_s(szControlName, _countof(szControlName), "ID_BASE%d", i);
		m_CompoundOptionList[i].pStaticBase = GetControl<CEtUIStatic>(szControlName);
		m_CompoundOptionList[i].pStaticBase->SetBlendRate( 0.f );

		sprintf_s(szControlName, _countof(szControlName), "ID_NAME%d", i);
		m_CompoundOptionList[i].pStaticName = GetControl<CEtUIStatic>(szControlName);
		m_CompoundOptionList[i].pStaticName->SetBlendRate( 0.f );

		sprintf_s(szControlName, _countof(szControlName), "ID_LEVEL%d", i);
		m_CompoundOptionList[i].pStaticLevel = GetControl<CEtUIStatic>(szControlName);
		m_CompoundOptionList[i].pStaticLevel->SetBlendRate( 0.f );

		sprintf_s(szControlName, _countof(szControlName), "ID_STATIC_LEVEL%d", i);
		m_CompoundOptionList[i].pStaticLevelText = GetControl<CEtUIStatic>(szControlName);
		m_CompoundOptionList[i].pStaticLevelText->SetBlendRate( 0.f );

		sprintf_s(szControlName, _countof(szControlName), "ID_TYPE%d", i);
		m_CompoundOptionList[i].pStaticType = GetControl<CEtUIStatic>(szControlName);
		m_CompoundOptionList[i].pStaticType->SetBlendRate( 0.f );

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		sprintf_s(szControlName, _countof(szControlName), "ID_STATIC_TYPE_FLAG%d", i);
		m_CompoundOptionList[i].pStaticNotifyFlag = GetControl<CEtUIStatic>(szControlName);
		m_CompoundOptionList[i].pStaticNotifyFlag->Show( false );
#endif

		m_CompoundOptionList[i].Clear();
	}

	for( int i=0; i<5; i++ ) {
		sprintf_s(szControlName, _countof(szControlName), "%s%d", "ID_ITEM_SLOT", i);
		m_pNeedItemSlotButton[i] = GetControl<CDnItemSlotButton>(szControlName);
		m_pNeedItemSlotButton[i]->SetSlotType( ST_ITEM_COMPOUND_JEWEL );
	}

	m_pCompoundMixDlg = new CDnItemCompoundMixDlg( UI_TYPE_MODAL, NULL, COMPOUND_CONFIRM_DIALOG, this );
	m_pCompoundMixDlg->Initialize( false );

	m_pCompoundMix2Dlg = new CDnItemCompoundMix2Dlg( UI_TYPE_MODAL, NULL, COMPOUND_CONFIRM_DIALOG, this );
	m_pCompoundMix2Dlg->Initialize( false );
}

void CDnItemCompoundTab2Dlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemCompoundHighLvTabDlg.ui" ).c_str(), bShow );
}

void CDnItemCompoundTab2Dlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow ) {
		char szStr[32];
		sprintf_s( szStr, "ID_RBT_TAB%d", m_nSelectPartsType );
		CEtUIRadioButton *pButton = GetControl<CEtUIRadioButton>(szStr);
		if( pButton ) pButton->SetChecked( true );
		if( CDnActor::s_hLocalActor )
			m_pComboBoxJob->SetSelectedByIndex( CDnActor::s_hLocalActor->GetClassID() - 1 );


#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		if( m_bForceOpenMode == false )
		{
			m_pCheckBoxLowLevel->SetChecked( true );
			m_pCheckBoxPossibleOnly->SetChecked( true );
		}
#else
		m_pCheckBoxLowLevel->SetChecked( true );
		m_pCheckBoxPossibleOnly->SetChecked( true );
#endif

		UpdateGroupList();
	}
	else 
	{
		ResetList();
		GetInterface().CloseBlind();

		if( m_nRemoteItemID )
		{
			m_nRemoteItemID = 0;
			GetInterface().OpenBaseDialog();
			CDnLocalPlayerActor::LockInput( false );
		}
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		ResetForceOpenMode();
#endif
	}

	CDnCustomDlg::Show( bShow );
}

void CDnItemCompoundTab2Dlg::SetRemoteItemID( int nRemoteItemID )
{
	m_nRemoteItemID = nRemoteItemID;

	if( nRemoteItemID ) {
		GetInterface().GetMainMenuDialog()->CloseInvenDialog();
		GetInterface().GetMainMenuDialog()->SetIgnoreShowFunc( true );
		GetInterface().OpenBlind();
		CDnLocalPlayerActor::StopAllPartyPlayer();
		CDnLocalPlayerActor::LockInput( true );
	}
}

void CDnItemCompoundTab2Dlg::ResetList()
{
	SAFE_DELETE_PVEC( m_pVecCompoundGroupList );
	SAFE_DELETE_VEC( m_pVecResultGroupList );

	for( int i=0; i<5; i++ ) {
		m_pNeedItemSlotButton[i]->ResetSlot();
		SAFE_DELETE( m_pNeedItem[i] );
	}
	for( int i=0; i<6; i++ ) {
		m_CompoundGroupList[i].Clear();
		m_CompoundOptionList[i].Clear();
	}
}

void CDnItemCompoundTab2Dlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_RADIOBUTTON_CHANGED ) 
	{
		if( strstr( pControl->GetControlName(), "ID_RBT_TAB" ) ) {
			sscanf_s( pControl->GetControlName(), "ID_RBT_TAB %d", &m_nSelectPartsType, sizeof(int) );
			UpdateGroupList();
		}
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		if( IsCmdControl( "ID_RBT_BASETAB0" ) ) 
		{
			if( m_bForceOpenMode && bTriggeredByUser == true )
			{
				static int COMPOUND_CREATE_SHOP_ID = 1001; 
				SetCompoundListID( COMPOUND_CREATE_SHOP_ID );

				char szStr[32];
				sprintf_s( szStr, "ID_RBT_TAB%d", m_nSelectPartsType );
				CEtUIRadioButton *pButton = GetControl<CEtUIRadioButton>(szStr);
				if( pButton ) pButton->SetChecked( true );
				if( CDnActor::s_hLocalActor )
					m_pComboBoxJob->SetSelectedByIndex( CDnActor::s_hLocalActor->GetClassID() - 1 );

				UpdateGroupList();
			}
		}

		if( IsCmdControl( "ID_RBT_BASETAB1") )
		{
			if( m_bForceOpenMode && bTriggeredByUser == true )
			{
				static int COMPOUND_ENCHANT_SHOP_ID = 2001; 
				SetCompoundListID( COMPOUND_ENCHANT_SHOP_ID );

				char szStr[32];
				sprintf_s( szStr, "ID_RBT_TAB%d", m_nSelectPartsType );
				CEtUIRadioButton *pButton = GetControl<CEtUIRadioButton>(szStr);
				if( pButton ) pButton->SetChecked( true );
				if( CDnActor::s_hLocalActor )
					m_pComboBoxJob->SetSelectedByIndex( CDnActor::s_hLocalActor->GetClassID() - 1 );

				UpdateGroupList();
			}
		}
#endif
	}
	else if( nCommand == EVENT_CHECKBOX_CHANGED ) 
	{
		if( IsCmdControl( "ID_CHECKBOX_LEVEL" ) ) 
		{
			m_bCheckHideLowLevel = m_pCheckBoxLowLevel->IsChecked();

			if( COMPOUNT_SUNDRIES_ID == m_nSelectPartsType )
				return;

			UpdateGroupList();
		}
		if( IsCmdControl( "ID_CHECKBOX_LIST" ) ) 
		{
			m_bCheckShowPossibleOnly = m_pCheckBoxPossibleOnly->IsChecked();
			UpdateGroupList();
		}
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		if( IsCmdControl( "ID_CHECKBOX_INFO" ) ) 
		{
			if( m_pCheckNotifyItem->IsChecked() )
			{
				int nIndex = m_nSelectOption % 6;
				if( nIndex >= 0 && nIndex < 6) 
				{
					const GroupList* pSelectedGroup = GetSelectedResultGroupList();
					if( pSelectedGroup )
					{
						CSCompoundNotify Info;
						Info.nItemID =  pSelectedGroup->nItemID;
						Info.nItemOption = GetSelectResultCompoundID();
						SendItemCompoundNotify(Info);
					}
				}
			}
			else
			{
				CSCompoundNotify Info;
				Info.nItemID = 0;
				Info.nItemOption = 0;
				SendItemCompoundNotify(Info);
			}
		}
#endif
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED ) {
		if( IsCmdControl( "ID_COMBOBOX_JOB" ) ) {
			if( m_pComboBoxJob->GetSelectedIndex() != m_nSelectJobType ) {
				m_nSelectJobType = m_pComboBoxJob->GetSelectedIndex();

				UpdateGroupList();
			}
		}
		else if( IsCmdControl( "ID_COMBOBOX_SORT" ) ) {
			if( m_pComboBoxSort->GetSelectedIndex() != m_nSelectSortType ) {
				m_nSelectSortType = m_pComboBoxSort->GetSelectedIndex();

				UpdateGroupList();
			}
		}
	}
	else if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		if ( m_bForceOpenMode )
		{
			if( IsCmdControl( "ID_CLOSE_DIALOG" ) || IsCmdControl( "ID_BT_CLOSE" ) )
			{
				Show(false);
				SetForceOpenMode( false );
				return;
			}
		}
#endif

		if( IsCmdControl( "ID_BT_PREV" ) ) {
			if( m_nCurGroupPage > 0 ) {
				m_nCurGroupPage--;
				if( m_nSelectGroup != -1 ) m_nSelectGroup -= 6;
				RefreshGroupList();
			}
		}
		else if( IsCmdControl( "ID_BT_NEXT" ) ) {
			if( m_nCurGroupPage < GetMaxGroupPage() ) {
				m_nCurGroupPage++;
				if( m_nSelectGroup != -1 ) m_nSelectGroup += 6;
				if( m_nSelectGroup >= (int)m_pVecResultGroupList.size() ) {
					m_nSelectGroup = (int)m_pVecResultGroupList.size() - 1;
				}
				RefreshGroupList();
			}
		}
		if( IsCmdControl( "ID_PAGE_PREV" ) ) {
			if( m_nCurOptionPage > 0 ) {
				m_nCurOptionPage--;
				if( m_nSelectOption != -1 ) m_nSelectOption -= 6;
				RefreshOptionList();
			}
		}
		else if( IsCmdControl( "ID_PAGE_NEXT" ) ) {
			if( m_nCurOptionPage < GetMaxOptionPage() ) {
				m_nCurOptionPage++;
				GroupList *pList = m_pVecResultGroupList[m_nSelectGroup];
				if( m_nSelectOption != -1 ) m_nSelectOption += 6;
				if( m_nSelectOption >= (int)pList->nVecResultCompoundList.size() ) {
					m_nSelectOption = (int)pList->nVecResultCompoundList.size() - 1;
				}
				RefreshOptionList();
			}
		}
		else if( IsCmdControl("ID_CANCEL") ) {
			ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl("ID_CLOSE_DIALOG"), 0 );
			return;
		}
		else if( IsCmdControl("ID_OK") ) {
			if( m_pButtonOK->IsEnable() ) {
				if( m_nRemoteItemID ) {
					int nCount = 0;
					nCount += GetItemTask().GetCharInventory().GetItemCount( m_nRemoteItemID );
					nCount += GetItemTask().GetCashInventory().GetItemCount( m_nRemoteItemID );
					if( !nCount ) {
						GetInterface().MessageBox( 7014, MB_OK );
						return;
					}
				}
				if( GetSelectResultCompoundID() ) 
					ShowConfirmDlg( GetSelectResultCompoundID() );
			}
			return;
		}
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		else if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_ITEM_CREATE);
		}
#endif
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnItemCompoundTab2Dlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	if( m_pComboBoxJob && m_pComboBoxSort && !m_pComboBoxJob->IsOpenedDropDownBox() && !m_pComboBoxSort->IsOpenedDropDownBox() ) {
		switch( uMsg )
		{
		case WM_LBUTTONDOWN:
			{
				POINT MousePoint;
				float fMouseX, fMouseY;

				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );
				PointToFloat( MousePoint, fMouseX, fMouseY );

				int nSelectGroup = -1;
				int nSelectOption = -1;
				for( int i=0; i<6; i++ ) {
					if( m_CompoundGroupList[i].IsInsideItem( fMouseX, fMouseY ) ) {
						nSelectGroup = i;
						break;
					}
				}
				for( int i=0; i<6; i++ ) {
					if( m_CompoundOptionList[i].IsInsideItem( fMouseX, fMouseY ) ) {
						nSelectOption = i;
						break;
					}
				}

				if( nSelectGroup != -1 ) {
					if( m_CompoundGroupList[nSelectGroup].pStaticBase->IsShow() )
					{
						m_nSelectGroup = ( m_nCurGroupPage * 6 ) + nSelectGroup;
						UpdateSelectGroup();
						return true;
					}
					else
					{
					}
				}
				else if( nSelectOption != -1 ) {
					if( m_CompoundOptionList[nSelectOption].pStaticBase->IsShow() )
					{
						m_nSelectOption = ( m_nCurOptionPage * 6 ) + nSelectOption;
						UpdateSelectOption();
						return true;
					}
					else
					{
					}
				}
			}
			break;
		case WM_LBUTTONDBLCLK:
			{
				if( !IsMouseInDlg() ) break;
				POINT MousePoint;
				SUICoord uiCoordsBase[2];

				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );

				float fMouseX, fMouseY;
				PointToFloat( MousePoint, fMouseX, fMouseY );

				bool bSelect = false;
				for( int i=0; i<6; i++ ) {
					if( m_CompoundOptionList[i].pItem && m_CompoundOptionList[i].IsInsideItem( fMouseX, fMouseY ) ) {
						bSelect = true;
						break;
					}
				}

				if( bSelect ) 
					ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_OK"), 0 );
			}
			return true;
		case WM_MOUSEWHEEL:
			{
				POINT MousePoint;
				SUICoord uiCoordsBase[2], uiCoordsTemp[2];


				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );

				float fMouseX, fMouseY;
				ScreenToClient( hWnd, &MousePoint );
				PointToFloat( MousePoint, fMouseX, fMouseY );
				lParam = MAKELPARAM( MousePoint.x, MousePoint.y );

				GetControl<CEtUIStatic>("ID_STATIC16")->GetUICoord(uiCoordsTemp[0]);
				GetControl<CEtUIStatic>("ID_STATIC2")->GetUICoord(uiCoordsTemp[1]);

				for( int i=0; i<2; i++ ) {
					uiCoordsBase[i] = DlgCoordToScreenCoord( uiCoordsTemp[i] );
				}

				bool bGroup = uiCoordsBase[0].IsInside( fMouseX, fMouseY );
				bool bOption = uiCoordsBase[1].IsInside( fMouseX, fMouseY );

				if( bGroup || bOption )
				{
					UINT uLines;
					SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
					int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
					if( nScrollAmount > 0 )
					{
						if( bGroup ) {
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_PREV"), 0 );
						}
						if( bOption ) {
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_PAGE_PREV"), 0 );
						}
					}
					else if( nScrollAmount < 0 )
					{
						if( bGroup ) {
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_NEXT"), 0 );
						}
						if( bOption ) {
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_PAGE_NEXT"), 0 );
						}
					}

					// Update tooltip on mouse move message.
					CEtUIDialogBase::StaticMsgProc( CDnMainFrame::GetInstance().GetHWnd(), WM_MOUSEMOVE, 0, (LPARAM)lParam );
					return true;
				}
			}
			break;
		}
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnItemCompoundTab2Dlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
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
					if( !GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG ) ) {
						GetInterface().MessageBox( 7012, MB_OK );
						return;
					}
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
					if( m_bForceOpenMode )
						return;
#endif
					if( GetSelectResultCompoundID() ) {
						m_biLastNeedItemSerialID = 0;
						if( m_pCompoundMix2Dlg->IsShow() )
							m_biLastNeedItemSerialID = m_pCompoundMix2Dlg->GetSelectItemSerialID();

						int nCompoundID = GetSelectResultCompoundID();
						CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
						GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );
						GetItemTask().RequestItemCompoundOpen( nCompoundID, m_vNeedItemInfo, m_biLastNeedItemSerialID, GetDiscountedCost(CompoundInfo.iCost) );

						m_pCompoundMixDlg->EnableButton( false );
						m_pCompoundMix2Dlg->EnableButton( false );
					}
				}
				else if( IsCmdControl("ID_CANCEL") || IsCmdControl("ID_CLOSE_DIALOG") ) {
					m_pCompoundMixDlg->Show( false );
					m_pCompoundMix2Dlg->Show( false );
				}
			}
			break;
		case COMPOUND_MOVIE_DIALOG:
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( IsCmdControl("ID_PROCESS_COMPLETE") ) 
				{
					if( GetSelectResultCompoundID() ) 
					{
						int nCompoundID = GetSelectResultCompoundID();
						CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
						GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

						GetItemTask().RequestItemCompound( GetSelectResultCompoundID(), m_vNeedItemInfo, m_biLastNeedItemSerialID, GetDiscountedCost(CompoundInfo.iCost) );
					}
				}
				else if( IsCmdControl("ID_BUTTON_CANCEL") ) {
					GetItemTask().RequestItemCompoundCancel();
				}
			}
			break;
		}
	}
}

void CDnItemCompoundTab2Dlg::RefreshCompoundGroupByLevel( int nLevel )
{
	bool bEnable = true;
	if( CDnActor::s_hLocalActor )
	{
		if( CDnActor::s_hLocalActor->GetLevel() > nLevel )
		{
			bEnable = false;
		}
	}
	m_bCheckHideLowLevel = bEnable;
	m_pCheckBoxLowLevel->SetChecked( bEnable , false );
	UpdateGroupList();
}

void CDnItemCompoundTab2Dlg::SetCompoundListID( int nListID )
{
	m_nCurCompoundListID = nListID;
	SAFE_DELETE_PVEC( m_pVecCompoundGroupList );

	// Read from the table.
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUNDGROUP );
	DNTableFileFormat* pCompoundSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	if( !pSox || !pCompoundSox ) return;

	std::vector<int> nVecList;
	pSox->GetItemIDListFromField( "_ListID", nListID, nVecList );

	for( DWORD i=0; i<nVecList.size(); i++ ) {
		GroupList *pStruct = new GroupList;
		pStruct->nGroupTableID = nVecList[i];
		pStruct->nCompoundGroupID = pSox->GetFieldFromLablePtr( nVecList[i], "_CompoundGroupID" )->GetInteger();
		pStruct->nCompoundGroupType = pSox->GetFieldFromLablePtr( nVecList[i], "_CompoundGroupType" )->GetInteger();
		pStruct->nItemID = pSox->GetFieldFromLablePtr( nVecList[i], "_TooltipItemID" )->GetInteger();
		pStruct->nLevel = pSox->GetFieldFromLablePtr( nVecList[i], "_Level" )->GetInteger();
		pStruct->nRank = pSox->GetFieldFromLablePtr( nVecList[i], "_Rank" )->GetInteger();
		pStruct->nJob = pSox->GetFieldFromLablePtr( nVecList[i], "_JobClass" )->GetInteger();
		pStruct->nActivateLevel = pSox->GetFieldFromLablePtr( nVecList[i], "_ActivateLevel" )->GetInteger();
		pStruct->bIsPreview = pSox->GetFieldFromLablePtr( nVecList[i], "_PreviewState" )->GetBool();
		pStruct->nPreviewDescUIStringID = pSox->GetFieldFromLablePtr( nVecList[i], "_PreviewDescription" )->GetInteger();

		pCompoundSox->GetItemIDListFromField( "_CompoundGroupID", pStruct->nCompoundGroupID, pStruct->nVecCompoundList );

		m_pVecCompoundGroupList.push_back( pStruct );
	}
}

int CDnItemCompoundTab2Dlg::GetSelectResultCompoundID()
{
	const GroupList* pGroupList = GetSelectedResultGroupList();

	if( pGroupList )
		return pGroupList->nVecResultCompoundList[m_nSelectOption];
	else
		return 0;
}

const CDnItemCompoundTab2Dlg::GroupList* CDnItemCompoundTab2Dlg::GetSelectedResultGroupList( void )
{
	if( m_nSelectGroup < 0 || m_nSelectGroup >= (int)m_pVecResultGroupList.size() ) 
		return NULL;

	GroupList *pList = m_pVecResultGroupList[m_nSelectGroup];

	if( m_nSelectOption < 0 || m_nSelectOption >= (int)pList->nVecResultCompoundList.size() ) 
		return NULL;

	return pList;
}

bool CDnItemCompoundTab2Dlg::Sort_UpRank( CDnItemCompoundTab2Dlg::GroupList *a, CDnItemCompoundTab2Dlg::GroupList *b )
{
	return ( a->nRank < b->nRank ) ? true : false;
}

bool CDnItemCompoundTab2Dlg::Sort_DownRank( CDnItemCompoundTab2Dlg::GroupList *a, CDnItemCompoundTab2Dlg::GroupList *b )
{
	return ( a->nRank > b->nRank ) ? true : false;
}

bool CDnItemCompoundTab2Dlg::Sort_UpLevel( CDnItemCompoundTab2Dlg::GroupList *a, CDnItemCompoundTab2Dlg::GroupList *b )
{
	return ( a->nLevel < b->nLevel ) ? true : false;
}

bool CDnItemCompoundTab2Dlg::Sort_DownLevel( CDnItemCompoundTab2Dlg::GroupList *a, CDnItemCompoundTab2Dlg::GroupList *b )
{
	return ( a->nLevel > b->nLevel ) ? true : false;
}

void CDnItemCompoundTab2Dlg::UpdateGroupList()
{
	SAFE_DELETE_VEC( m_pVecResultGroupList );
	if( !CDnActor::s_hLocalActor ) return;

	for( DWORD i=0; i<m_pVecCompoundGroupList.size(); i++ ) {
		GroupList *pList = m_pVecCompoundGroupList[i];

		if( pList->nJob != m_nSelectJobType + 1 ) continue;
		if( m_nSelectPartsType != pList->nCompoundGroupType ) continue;
		
		if( COMPOUNT_SUNDRIES_ID != m_nSelectPartsType )
		{
			if( m_bCheckHideLowLevel && CDnActor::s_hLocalActor->GetLevel() > pList->nLevel ) 
				continue;
		}

		if( m_bCheckShowPossibleOnly && CDnActor::s_hLocalActor->GetLevel() < pList->nActivateLevel ) 
			continue;

		pList->nVecResultCompoundList = pList->nVecCompoundList;

		m_pVecResultGroupList.push_back( pList );

	}
#if defined( PRE_ADD_LEGEND_ITEM_UI )
		Sort_Rank();
#else	// #if defined( PRE_ADD_LEGEND_ITEM_UI )
	switch( m_nSelectSortType ) {
		case 0: std::sort( m_pVecResultGroupList.begin(), m_pVecResultGroupList.end(), Sort_UpRank ); break;
		case 1: std::sort( m_pVecResultGroupList.begin(), m_pVecResultGroupList.end(), Sort_DownRank ); break;
		case 2: std::sort( m_pVecResultGroupList.begin(), m_pVecResultGroupList.end(), Sort_UpLevel ); break;
		case 3: std::sort( m_pVecResultGroupList.begin(), m_pVecResultGroupList.end(), Sort_DownLevel ); break;
	}
#endif	// #if defined( PRE_ADD_LEGEND_ITEM_UI )

	m_nSelectGroup = -1;
	m_nCurGroupPage = 0;

	m_nSelectOption = -1;
	m_nCurOptionPage = 0;

	RefreshGroupList();
	SelectCompoundItemFromIndex( GetSelectResultCompoundID() );
}

void CDnItemCompoundTab2Dlg::RefreshGroupList( bool bRefreshSelect )
{
	GetControl<CEtUIButton>("ID_BT_PREV")->Enable( false );
	GetControl<CEtUIButton>("ID_BT_NEXT")->Enable( false );

	if( m_pVecResultGroupList.empty() ) {
		m_pStaticGroupPage->SetText( L"" );
		for( int i=0; i<6; i++ ) {
			m_CompoundGroupList[i].Clear();
		}
		UpdateSelectGroup();
		return;
	}

	WCHAR wszStr[32];
	swprintf_s( wszStr, L"%d/%d", m_nCurGroupPage + 1, GetMaxGroupPage() + 1 );
	m_pStaticGroupPage->SetText( wszStr );

	if( m_nCurGroupPage > 0 ) GetControl<CEtUIButton>("ID_BT_PREV")->Enable( true );
	if( m_nCurGroupPage < GetMaxGroupPage() ) GetControl<CEtUIButton>("ID_BT_NEXT")->Enable( true );

	int nUserLevel = ( CDnActor::s_hLocalActor ) ? CDnActor::s_hLocalActor->GetLevel() : 1;
	for( int i=0; i<6; i++ ) {
		m_CompoundGroupList[i].Clear();

		int nIndex = ( m_nCurGroupPage * 6 ) + i;
		if( nIndex >= (int)m_pVecResultGroupList.size() ) continue;
		m_CompoundGroupList[i].SetInfo( m_pVecResultGroupList[nIndex]->nGroupTableID );
		bool bEnable = ( m_pVecResultGroupList[nIndex]->nActivateLevel > nUserLevel ) ? false : true;
		
		// #50328 Preview data is unconditionally disabled.
		if( m_pVecResultGroupList[nIndex]->bIsPreview )
		{
			bEnable = false;
		}

		m_CompoundGroupList[i].Enable( bEnable );
		m_CompoundGroupList[i].ForceBlend();

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		bool bFindCompoundItem = false;
		for( DWORD j=0; j<m_pVecResultGroupList[nIndex]->nVecCompoundList.size(); j++ )
		{
			if( CDnItemCompoundNotifyDlg::IsMatchedNotifyItem( m_pVecResultGroupList[nIndex]->nVecCompoundList[j] ) )
			{
				bFindCompoundItem = true;
				break;
			}
		}
		m_CompoundGroupList[i].EnableNotify( bFindCompoundItem );
#endif
	}

	if( bRefreshSelect )
		UpdateSelectGroup();
}

void CDnItemCompoundTab2Dlg::RefreshOptionList( bool bRefreshSelect )
{
	GetControl<CEtUIButton>("ID_PAGE_PREV")->Enable( false );
	GetControl<CEtUIButton>("ID_PAGE_NEXT")->Enable( false );
	if( m_nSelectGroup == -1 ) {
		m_pStaticOptionPage->SetText( L"" );
		m_pStaticOptionDesc->SetText( L"" );
		for( int i=0; i<6; i++ ) m_CompoundOptionList[i].Clear();

		UpdateSelectOption();
		return;
	}

	GroupList *pList = m_pVecResultGroupList[m_nSelectGroup];

	WCHAR wszStr[32];
	swprintf_s( wszStr, L"%d/%d", m_nCurOptionPage + 1, GetMaxOptionPage() + 1 );
	m_pStaticOptionPage->SetText( wszStr );

	if( m_nCurOptionPage > 0 ) GetControl<CEtUIButton>("ID_PAGE_PREV")->Enable( true );
	if( m_nCurOptionPage < GetMaxOptionPage() ) GetControl<CEtUIButton>("ID_PAGE_NEXT")->Enable( true );


	int nUserLevel = ( CDnActor::s_hLocalActor ) ? CDnActor::s_hLocalActor->GetLevel() : 1;
	for( int i=0; i<6; i++ ) {
		m_CompoundOptionList[i].Clear();

		int nIndex = ( m_nCurOptionPage * 6 ) + i;
		if( nIndex >= (int)pList->nVecResultCompoundList.size() ) continue;

		CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
		GetItemTask().GetCompoundInfo( pList->nVecResultCompoundList[nIndex], &CompoundInfo );
		if( CompoundInfo.aiSuccessItemID[0] > 0 ) {
			m_CompoundOptionList[i].SetInfo( CompoundInfo.iCompoundTableID, CompoundInfo.aiSuccessItemID[0] );
			bool bEnable = ( pList->nActivateLevel > nUserLevel ) ? false : true;
			
			// #50328 Preview data is unconditionally disabled.
			if( pList->bIsPreview )
			{
				bEnable = false;
			}

			m_CompoundOptionList[i].Enable( bEnable );
			m_CompoundOptionList[i].ForceBlend();

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
			bool bNotified = CDnItemCompoundNotifyDlg::IsMatchedNotifyItem( pList->nVecResultCompoundList[nIndex] );
			m_CompoundOptionList[i].EnableNotify( bNotified );
#endif
		}
	}

	if( bRefreshSelect )
		UpdateSelectOption();
}

void CDnItemCompoundTab2Dlg::RefreshCompoundItem()
{
	if( m_nSelectGroup == -1 || m_nSelectOption == -1 ) {
		m_pStaticGold->SetText( L"" );
		m_pStaticSilver->SetText( L"" );
		m_pStaticBronze->SetText( L"" );
		m_pStaticOptionDesc->SetText( L"" );
		for( int i=0; i<5; i++ ) {
			SAFE_DELETE( m_pNeedItem[i] );
			m_pNeedItemSlotButton[i]->ResetSlot();
		}
		m_pButtonOK->Enable( false );
		return;
	}

	const GroupList* pSelectedGroup = GetSelectedResultGroupList();
	if( NULL == pSelectedGroup )
		return;

	int nCompoundID = GetSelectResultCompoundID();
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

	CompoundInfo.iCost = GetDiscountedCost( CompoundInfo.iCost );

	// Show fee.
	if( false == pSelectedGroup->bIsPreview )
	{
		int nGold = (int)(CompoundInfo.iCost/10000);
		int nSilver = (int)((CompoundInfo.iCost%10000)/100);
		int nBronze = (int)(CompoundInfo.iCost%100);
		m_pStaticGold->SetIntToText( nGold );
		m_pStaticSilver->SetIntToText( nSilver );
		m_pStaticBronze->SetIntToText( nBronze );
	}
	else
	{
		m_pStaticGold->SetText( L"" );
		m_pStaticSilver->SetText( L"" );
		m_pStaticBronze->SetText( L"" );
	}

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

	for( int i=0; i<5; i++ )
		m_pNeedItemSlotButton[i]->ResetSlot();

	// Set up the required item info struct to send to server.
	m_vNeedItemInfo.clear();
	for( int i=0; i<5; i++ ) {
		if( CompoundInfo.aiItemID[i] > 0 && CompoundInfo.aiItemCount[i] > 0 )
		{
			CDnItemTask::ItemCompoundInfo Info;
			Info.iItemID = CompoundInfo.aiItemID[ i ];
			Info.iUseCount = CompoundInfo.aiItemCount[ i ];
			m_vNeedItemInfo.push_back(Info);

			// Additionally check inventory for item types and counts.
			int nCurItemCount = 0;
			if( CompoundInfo.abResultItemIsNeedItem[i] ) {
				std::vector<CDnItem *> pVecResultList;
				//When searching for items, only search items matching the option settings..
				nCurItemCount = GetItemTask().GetCharInventory().FindItemList( CompoundInfo.aiItemID[i], CompoundInfo.iCompoundPreliminaryID, pVecResultList );

			}
			else nCurItemCount = GetItemTask().GetCharInventory().GetItemCount( CompoundInfo.aiItemID[i], -1 );

			// Set material item slot.
			if( false == pSelectedGroup->bIsPreview )
				SetNeedItemSlot( i, CompoundInfo.aiItemID[i], CompoundInfo.aiItemCount[i], nCurItemCount, CompoundInfo.iCompoundPreliminaryID );

			// Returns false if any one type is missing.
			if( nCurItemCount < CompoundInfo.aiItemCount[i] )
				bNeedItemResult = false;
		}
	}
	if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetLevel() >= m_pVecResultGroupList[m_nSelectGroup]->nActivateLevel ) bPossibleLevel = true;
	m_pButtonOK->Enable( ( bNeedItemResult && bEnoughCoin && bPossibleLevel ) );
}

void CDnItemCompoundTab2Dlg::UpdateSelectGroup()
{
	SUICoord uiCoordBase, uiCoordSelect;

	if( m_nSelectGroup < 0 ) 
	{
		m_pStaticSelectGroup->Show( false );
	}
	else {
		int nIndex = m_nSelectGroup % 6;
		m_CompoundGroupList[nIndex].pStaticBase->GetUICoord( uiCoordBase );
		m_pStaticSelectGroup->GetUICoord(uiCoordSelect);
		uiCoordSelect.fX = uiCoordBase.fX - 0.0039f;
		uiCoordSelect.fY = uiCoordBase.fY- 0.005f;
		m_pStaticSelectGroup->SetUICoord(uiCoordSelect);
		m_pStaticSelectGroup->Show( true );
	}

	m_nSelectOption = -1;
	m_nCurOptionPage = 0;
	RefreshOptionList();
}

void CDnItemCompoundTab2Dlg::UpdateSelectOption()
{
	SUICoord uiCoordBase, uiCoordSelect;

	if( m_nSelectOption < 0 ) 
	{
		m_pStaticSelectOption->Show( false );
		m_pStaticOptionDesc->SetText( L"" );
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		m_pCheckNotifyItem->Show( false );
		m_pTextNotifyItem->Show( false );
#endif
	}
	else 
	{
		int nIndex = m_nSelectOption % 6;
		m_CompoundOptionList[nIndex].pStaticBase->GetUICoord( uiCoordBase );
		m_pStaticSelectOption->GetUICoord(uiCoordSelect);
		uiCoordSelect.fX = uiCoordBase.fX - 0.0039f;
		uiCoordSelect.fY = uiCoordBase.fY- 0.005f;
		m_pStaticSelectOption->SetUICoord(uiCoordSelect);
		m_pStaticSelectOption->Show( true );

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		m_pCheckNotifyItem->Show( true );
		m_pTextNotifyItem->Show( true );

		if( m_CompoundOptionList[nIndex].pItem )
		{
			bool bNotified = CDnItemCompoundNotifyDlg::IsMatchedNotifyItem( GetSelectResultCompoundID() );
			m_pCheckNotifyItem->SetChecked( bNotified , false );
		}
#endif
		if( GetSelectResultCompoundID() ) {
			const GroupList* pSelectedGroup = GetSelectedResultGroupList();
			if( pSelectedGroup )
			{
				if( false == pSelectedGroup->bIsPreview )
				{
					DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
					m_pStaticOptionDesc->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( GetSelectResultCompoundID(), "_DescriptionID" )->GetInteger() ) );
				}
				else
				{
					m_pStaticOptionDesc->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSelectedGroup->nPreviewDescUIStringID ) );
				}
			}
		}
	}

	RefreshCompoundItem();
}


void CDnItemCompoundTab2Dlg::CompoundGroupStruct::SetInfo( int nGroupTableID )
{
	pStaticBase->Show( true );
	pStaticLevelText->Show( true );

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUNDGROUP );
	int nItemID = pSox->GetFieldFromLablePtr( nGroupTableID, "_TooltipItemID" )->GetInteger();
	int nLevel = pSox->GetFieldFromLablePtr( nGroupTableID, "_Level" )->GetInteger();
	int nRank = pSox->GetFieldFromLablePtr( nGroupTableID, "_Rank" )->GetInteger();

	TItemInfo itemInfo;
	if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) return;
	pItem = GetItemTask().CreateItem( itemInfo );

	if( pItem )
	{
		pItem->SetTypeParam( 0, 0 ); // Items in the group tab do not need to show random options, so force-set to 0.

		// Reset durability. Since max value is unknown above, set to max value here like this.
		if( pItem->GetItemType() == ITEMTYPE_WEAPON ) {
			CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(pItem);
			pWeapon->SetDurability( pWeapon->GetMaxDurability() );
		}
		else if( pItem->GetItemType() == ITEMTYPE_PARTS ) {
			CDnParts *pParts = dynamic_cast<CDnParts *>(pItem);
			pParts->SetDurability( pParts->GetMaxDurability() );
		}
		pItemSlotButton->SetItem( pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );

		pStaticName->SetTextColor( ITEM::RANK_2_COLOR( (eItemRank)nRank ) );
		pStaticName->SetText( CDnItem::GetItemFullName( nItemID ).c_str() );
	}

	WCHAR wszStr[64];
	swprintf_s( wszStr, L"%d %s", nLevel, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ITEM::RANK_2_INDEX( (eItemRank)nRank ) ) );
	pStaticLevel->SetText( wszStr );

}

void CDnItemCompoundTab2Dlg::CompoundOptionStruct::SetInfo( int nCompoundID, int nItemID )
{
	pStaticBase->Show( true );
	pStaticLevelText->Show( true );

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	int nOptionIndex = pSox->GetFieldFromLablePtr( nCompoundID, "_CompoundPotentialID" )->GetInteger();

	WCHAR szTemp[64];
	TItemInfo itemInfo;
	if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) return;
	itemInfo.Item.cOption = nOptionIndex;
	pItem = GetItemTask().CreateItem( itemInfo );
	
	if( pItem )
	{
		// Reset durability. Since max value is unknown above, set to max value here like this.
		if( pItem->GetItemType() == ITEMTYPE_WEAPON ) {
			CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(pItem);
			if( pWeapon ) {
				pWeapon->SetDurability( pWeapon->GetMaxDurability() );
				swprintf_s( szTemp, _countof(szTemp), L"%s : %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7009 ), DN_INTERFACE::STRING::ITEM::GetEquipString( pWeapon->GetEquipType() ) );
				pStaticType->SetText( szTemp );
			}
		}
		else if( pItem->GetItemType() == ITEMTYPE_PARTS ) {
			CDnParts *pParts = dynamic_cast<CDnParts *>(pItem);
			if( pParts ) {
				pParts->SetDurability( pParts->GetMaxDurability() );
				swprintf_s( szTemp, _countof(szTemp), L"%s : %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7009 ), DN_INTERFACE::STRING::ITEM::GetPartsString( pParts->GetPartsType() ) );
				pStaticType->SetText( szTemp );
			}
		}
		pItemSlotButton->SetItem( pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );

		pStaticName->SetTextColor( ITEM::RANK_2_COLOR( pItem->GetItemRank() ) );
		pStaticName->SetText( CDnItem::GetItemFullName( nItemID, nOptionIndex ).c_str() );
		pStaticLevel->SetIntToText( pItem->GetLevelLimit() );


#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		bool bNotified = CDnItemCompoundNotifyDlg::IsMatchedNotifyItem( nCompoundID );
		EnableNotify( bNotified );
#endif

	}
}

void CDnItemCompoundTab2Dlg::SetNeedItemSlot( int nSlotIndex, int nItemID, int nNeedItemCount, int nCurItemCount, char nOptionIndex/* = -1*/ )
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
	m_pNeedItemSlotButton[nSlotIndex]->SetItem(m_pNeedItem[nSlotIndex], CDnSlotButton::ITEM_ORIGINAL_COUNT);
	m_pNeedItemSlotButton[nSlotIndex]->SetJewelCount( nNeedItemCount, nCurItemCount );

	if( nCurItemCount < nNeedItemCount )
		m_pNeedItemSlotButton[nSlotIndex]->SetRegist( true );
	else
		m_pNeedItemSlotButton[nSlotIndex]->SetRegist( false );
}

void CDnItemCompoundTab2Dlg::ShowConfirmDlg( int nCompoundID )
{
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	if( m_bForceOpenMode )
		return;
#endif

	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );
	
	CompoundInfo.iCost = GetDiscountedCost( CompoundInfo.iCost );

	bool bUseMixOptionDlg = false;
	if( m_nSelectPartsType < 7 ) {
		if( CompoundInfo.bResultItemIsNeedItem )
			bUseMixOptionDlg = true;
	}

	if( bUseMixOptionDlg ) {
		for( int j=0; j<5; j++ ) {
			if( CompoundInfo.abResultItemIsNeedItem[j] ) {
				std::vector<CDnItem *> pVecResultList;
				//When searching for items, only search items matching the option settings..
				GetItemTask().GetCharInventory().FindItemList( CompoundInfo.aiItemID[j], CompoundInfo.iCompoundPreliminaryID, pVecResultList );

				m_pCompoundMix2Dlg->SetCompoundInfo( pVecResultList, CompoundInfo.iCompoundPreliminaryID );
				break;
			}
		}
		m_pCompoundMix2Dlg->Show( true );
	}
	else {
		m_pCompoundMixDlg->SetCompoundInfo( CompoundInfo.aiItemID, CompoundInfo.aiItemCount );
		m_pCompoundMixDlg->SetPrice( (int)CompoundInfo.iCost );
		m_pCompoundMixDlg->Show( true );
	}

	CDnSlotButton *pDragButton = (CDnSlotButton *)drag::GetControl();
	if( pDragButton ) {
		pDragButton->DisableSplitMode(true);
		drag::ReleaseControl();
	}

	if( bUseMixOptionDlg ) {
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
		int nDescription = pSox->GetFieldFromLablePtr( nCompoundID, "_DescriptionID" )->GetInteger();
		m_pCompoundMix2Dlg->SetDescription( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nDescription ) );
	}

	int nProb = (int)(CompoundInfo.fSuccessPossibility * 100.0f);
	if( CompoundInfo.fSuccessPossibility * 100.0f - (float)nProb > 0.999 ) nProb += 1;
	if( bUseMixOptionDlg ) {
		m_pCompoundMix2Dlg->SetProb( nProb );
		m_pCompoundMix2Dlg->SetOK( true );
	}
	else {
		m_pCompoundMixDlg->SetProb( nProb );
		m_pCompoundMixDlg->SetOK( true );
	}
}

INT64 CDnItemCompoundTab2Dlg::GetDiscountedCost( INT64 iCost )
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

int CDnItemCompoundTab2Dlg::GetMaxGroupPage()
{
	return ( (int)m_pVecResultGroupList.size() - 1 ) / 6;
}

int CDnItemCompoundTab2Dlg::GetMaxOptionPage()
{
	if( m_nSelectGroup < 0 || m_nSelectGroup >= (int)m_pVecResultGroupList.size() ) return 0;

	GroupList *pList = m_pVecResultGroupList[m_nSelectGroup];
	return ( (int)pList->nVecResultCompoundList.size() - 1 ) / 6;
}


void CDnItemCompoundTab2Dlg::OnRecvItemCompoundOpen()
{
	// After obtaining the appropriate duration according to the item to combine

	int nCompoundID = GetSelectResultCompoundID();
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

	GetInterface().OpenMovieProcessDlg(
		"ItemCombine.avi", CompoundInfo.fTimeRequired / 1000.0f, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8019 ), COMPOUND_MOVIE_DIALOG, this );

	m_pCompoundMixDlg->EnableButton( true );
	m_pCompoundMixDlg->Show( false );
	m_pCompoundMix2Dlg->EnableButton( true );
	m_pCompoundMix2Dlg->Show( false );
}

void CDnItemCompoundTab2Dlg::OnRecvItemCompound()
{
	m_nSelectOption = -1;
	UpdateSelectOption();

	for( int i=0; i<5; i++ )
		m_pNeedItemSlotButton[i]->ResetSlot();

	if( m_nRemoteItemID ) {
		int nCount = 0;
		nCount += GetItemTask().GetCharInventory().GetItemCount( m_nRemoteItemID );
		nCount += GetItemTask().GetCashInventory().GetItemCount( m_nRemoteItemID );
		if( !nCount ) Show( false );
	}
}

void CDnItemCompoundTab2Dlg::Sort_Rank()
{
	if( 0 == m_nSelectSortType )
		return;

	std::vector< GroupList * >::iterator Itor = m_pVecResultGroupList.begin();

	while( Itor != m_pVecResultGroupList.end() )
	{
		if( m_nSelectSortType != (*Itor)->nRank )
			Itor = m_pVecResultGroupList.erase( Itor );
		else
			++Itor;
	}
}

void CDnItemCompoundTab2Dlg::SelectCompoundItemFromIndex( int nCompoundIndex )
{
	bool bFind = false;

	for( DWORD i=0; i<m_pVecResultGroupList.size(); i++ ) 
	{
		for( DWORD j=0; j<m_pVecResultGroupList[i]->nVecResultCompoundList.size(); j++ ) 
		{
			if( m_pVecResultGroupList[i]->nVecResultCompoundList[j] == nCompoundIndex ) 
			{
				bFind = true;
				m_nSelectGroup = i;
				m_nCurGroupPage = m_nSelectGroup / 6;
				RefreshGroupList();
				m_nSelectOption = j;
				m_nCurOptionPage = m_nSelectOption / 6;
				RefreshOptionList();
				break;
			}
		}
		if( bFind ) 
			break;
	}
}

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND

void CDnItemCompoundTab2Dlg::SetForceOpenMode( bool bShow )
{
	static int COMPOUND_CREATE_SHOP_ID = 1001; 
	static int COMPOUND_ENCHANT_SHOP_ID = 2001; 

	m_bForceOpenMode = bShow;
	m_pButtonClose->Show( bShow );
	m_pRadioButtonCreate->Show( bShow );
	m_pRadioButtonEnchant->Show( bShow );

	if( m_nCurCompoundListID == COMPOUND_CREATE_SHOP_ID )
	{
		m_pRadioButtonCreate->SetChecked( true );
	}
	else if( m_nCurCompoundListID == COMPOUND_ENCHANT_SHOP_ID )
	{
		m_pRadioButtonEnchant->SetChecked( true );
	}
	
	m_pButtonOK->Show( !bShow );
	m_pButtonCancel->Show( !bShow );
}
void CDnItemCompoundTab2Dlg::ResetForceOpenMode()
{
	m_bForceOpenMode = false;
	m_pButtonClose->Show( false );
	m_pRadioButtonCreate->Show( false );
	m_pRadioButtonEnchant->Show( false );
	m_pButtonOK->Show( true );
	m_pButtonCancel->Show( true );
}

#endif