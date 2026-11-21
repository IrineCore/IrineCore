#include "StdAfx.h"
#include "EtUIMan.h"
#include "DnInvenDlg.h"
#include "DnInvenSlotDlg.h"
#include "MIInventoryItem.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnInvenDlg::CDnInvenDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUITabDialog( dialogType, pParentDialog, nID, pCallback  )
#ifdef PRE_ADD_INVEN_EXTENSION
	, m_SlotType( ST_ITEM_NONE )
#endif // #ifdef PRE_ADD_INVEN_EXTENSION
{

}

CDnInvenDlg::~CDnInvenDlg(void)
{
#if defined(PRE_ADD_AUTOUNPACK) || defined(PRE_ADD_CASHREMOVE)
	m_pVecInvenSlotDlg.clear();
#endif
}

void CDnInvenDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "InvenDlg.ui" ).c_str(), bShow );
}

void CDnInvenDlg::InitialUpdate()
{
	CDnInvenSlotDlg *pInvenSlotDlg(NULL);
	CEtUIRadioButton *pTabButton(NULL);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC01");
	pInvenSlotDlg = new CDnInvenSlotDlg( UI_TYPE_CHILD, this, PAGE_01 );
	pInvenSlotDlg->Initialize( false, PAGE_01, ITEM_MAX );
#ifdef PRE_ADD_AUTOUNPACK
	m_pVecInvenSlotDlg.push_back( pInvenSlotDlg );
#endif
	AddTabDialog( pTabButton, pInvenSlotDlg );
	m_vecTabButton.push_back(pTabButton);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC02");
	pInvenSlotDlg = new CDnInvenSlotDlg( UI_TYPE_CHILD, this, PAGE_02 );
	pInvenSlotDlg->Initialize( false, PAGE_02, ITEM_MAX );
#ifdef PRE_ADD_AUTOUNPACK
	m_pVecInvenSlotDlg.push_back( pInvenSlotDlg );
#endif
	AddTabDialog( pTabButton, pInvenSlotDlg );
	m_vecTabButton.push_back(pTabButton);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC03");
	pInvenSlotDlg = new CDnInvenSlotDlg( UI_TYPE_CHILD, this, PAGE_03 );
	pInvenSlotDlg->Initialize( false, PAGE_03, ITEM_MAX );
#ifdef PRE_ADD_AUTOUNPACK
	m_pVecInvenSlotDlg.push_back( pInvenSlotDlg );
#endif
	AddTabDialog( pTabButton, pInvenSlotDlg );
	m_vecTabButton.push_back(pTabButton);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC04");
	pInvenSlotDlg = new CDnInvenSlotDlg( UI_TYPE_CHILD, this, PAGE_04 );
	pInvenSlotDlg->Initialize( false, PAGE_04, ITEM_MAX );
#ifdef PRE_ADD_AUTOUNPACK
	m_pVecInvenSlotDlg.push_back( pInvenSlotDlg );
#endif
	AddTabDialog( pTabButton, pInvenSlotDlg );
	m_vecTabButton.push_back(pTabButton);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC05");
	pInvenSlotDlg = new CDnInvenSlotDlg( UI_TYPE_CHILD, this, PAGE_05 );
	pInvenSlotDlg->Initialize( false, PAGE_05, ITEM_MAX );
#ifdef PRE_ADD_AUTOUNPACK
	m_pVecInvenSlotDlg.push_back( pInvenSlotDlg );
#endif
	AddTabDialog( pTabButton, pInvenSlotDlg );
	m_vecTabButton.push_back(pTabButton);

#if defined( PRE_PERIOD_INVENTORY )
	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC06");
	pInvenSlotDlg = new CDnInvenSlotDlg( UI_TYPE_CHILD, this, PAGE_PERIOD );
	pInvenSlotDlg->Initialize( false, PAGE_PERIOD, ITEM_MAX );
	pInvenSlotDlg->SetSlotType( ST_INVENTORY_PERIOD );
#ifdef PRE_ADD_AUTOUNPACK
	m_pVecInvenSlotDlg.push_back( pInvenSlotDlg );
#endif
	AddTabDialog( pTabButton, pInvenSlotDlg );
	m_vecTabButton.push_back(pTabButton);
#endif	// #if defined( PRE_PERIOD_INVENTORY )

	SetCheckedTab( m_vecTabButton[PAGE_01]->GetTabID() );
}

bool CDnInvenDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_MOUSEWHEEL:
		{
			CEtUIDialog *pShowDialog = m_groupTabDialog.GetShowDialog();
			if( !pShowDialog ) return false;
			if( !pShowDialog->IsShow() ) return false;

			if( IsMouseInDlg() || pShowDialog->IsMouseInDlg() )
			{
				UINT uLines;
				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
				int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
				if( nScrollAmount > 0 )
				{
					PrevPage();
				}
				else if( nScrollAmount < 0 )
				{
					NextPage();
				}
			}
		}
		break;

	case WM_RBUTTONUP:
		{
		}
		break;

	case WM_RBUTTONDOWN:
		{
		}
		break;
	}

	return CEtUITabDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnInvenDlg::PrevPage()
{
	const CEtUIRadioButton *pCurrRadioButton = GetCurrentTabControl();
	CEtUIRadioButton *pPrevRadioButton = (CEtUIRadioButton *)pCurrRadioButton;
	for( int i = 0; i < (int)m_vecTabButton.size(); ++i )
	{
		if( m_vecTabButton[i] == pCurrRadioButton )
			break;
		pPrevRadioButton = m_vecTabButton[i];
	}
	if( pPrevRadioButton != pCurrRadioButton )
		SetCheckedTab( pPrevRadioButton->GetTabID() );
}

void CDnInvenDlg::NextPage()
{
	const CEtUIRadioButton *pCurrRadioButton = GetCurrentTabControl();
	CEtUIRadioButton *pNextRadioButton = (CEtUIRadioButton *)pCurrRadioButton;
	bool bNext = false;
	for( int i = 0; i < (int)m_vecTabButton.size(); ++i )
	{
		if( bNext )
		{
			pNextRadioButton = m_vecTabButton[i];
			break;
		}
		if( m_vecTabButton[i] == pCurrRadioButton )
			bNext = true;
	}
	if( pNextRadioButton != pCurrRadioButton )
		SetCheckedTab( pNextRadioButton->GetTabID() );
}

#if defined( PRE_PERIOD_INVENTORY )
void CDnInvenDlg::EnablePeriodInven( const bool bEnable, const __time64_t tTime )
{
	m_vecTabButton[PAGE_PERIOD]->Enable( bEnable );
	SetPeriodInvenTooltip( bEnable, tTime );

	CEtUIDialogGroup::DIALOG_VEC &vecDialog = m_groupTabDialog.GetDialogList();
	for( int i=0; i<(int)vecDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecDialog[i];

		int nDialogID = id_dlg_pair.second->GetDialogID();
		if( PAGE_PERIOD != nDialogID )
			continue;

		((CDnInvenSlotDlg*)id_dlg_pair.second)->SetUseItemCnt( PERIOD_ITEM_MAX );
	}
}

void CDnInvenDlg::SetPeriodInvenTooltip( const bool bEnable, const __time64_t tTime )
{
	if( true == bEnable )
	{
		std::wstring wszString;
#ifdef PRE_ADD_EQUIPLOCK
		DN_INTERFACE::STRING::GetTimeText2( wszString, tTime, 8364 );	// UISTRING : Timed expansion slot.\n#y(%d/%d/%d.........
#endif	// #ifdef PRE_ADD_EQUIPLOCK

		m_vecTabButton[PAGE_PERIOD]->SetTooltipText( wszString.c_str() );
	}
	else
		m_vecTabButton[PAGE_PERIOD]->SetTooltipText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8365 ) );	// UISTRING : Timed expansion slot has expired; moving item........
}

bool CDnInvenDlg::IsPeriodIvenEnable()
{
	return m_vecTabButton[PAGE_PERIOD]->GetTabID() == GetCurrentTabID();
}
#endif	//	#if defined( PRE_PERIOD_INVENTORY )

void CDnInvenDlg::SetSlotType( ITEM_SLOT_TYPE slotType )
{
	CEtUIDialogGroup::DIALOG_VEC &vecDialog = m_groupTabDialog.GetDialogList();
	for( int i=0; i<(int)vecDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecDialog[i];

#if defined( PRE_PERIOD_INVENTORY )
		int nDialogID = id_dlg_pair.second->GetDialogID();
		if( PAGE_PERIOD == nDialogID )
			continue;
#endif	// #if defined( PRE_PERIOD_INVENTORY )

		((CDnInvenSlotDlg*)id_dlg_pair.second)->SetSlotType( slotType );
	}
#ifdef PRE_ADD_INVEN_EXTENSION
	m_SlotType = slotType;
#endif
}

void CDnInvenDlg::SetItem( MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnInventoryDlg::SetItem");
	if( !pItem ) return;

	int nSlotIndex = pItem->GetSlotIndex();
	int nSlotPage = nSlotIndex / ITEM_MAX;

	CDnInvenSlotDlg *pInvenSlotDlg = static_cast<CDnInvenSlotDlg*>(m_groupTabDialog.GetDialog(nSlotPage+1));
	pInvenSlotDlg->SetItem( pItem );
}

void CDnInvenDlg::ResetSlot( int nSlotIndex )
{
	int nSlotPage = nSlotIndex / ITEM_MAX;

	CDnInvenSlotDlg *pInvenSlotDlg = static_cast<CDnInvenSlotDlg*>(m_groupTabDialog.GetDialog(nSlotPage+1));
	pInvenSlotDlg->ResetSlot( nSlotIndex );
}

void CDnInvenDlg::SetUseItemCnt( DWORD dwItemCnt )
{
	CDnInvenSlotDlg *pInvenSlotDlg(NULL);

	DWORD dwPageCnt = dwItemCnt/ITEM_MAX;
	DWORD i = 0;

	for( ; i<dwPageCnt; i++ )
	{
		pInvenSlotDlg = static_cast<CDnInvenSlotDlg*>(m_groupTabDialog.GetDialog(i+1));
		pInvenSlotDlg->SetUseItemCnt(ITEM_MAX);
	}

	for( ; i<(DWORD)m_groupTabDialog.GetDialogCount(); i++ )
	{
#if defined( PRE_PERIOD_INVENTORY )
		if( PAGE_PERIOD == i )
			continue;
#endif	// #if defined( PRE_PERIOD_INVENTORY )

		pInvenSlotDlg = static_cast<CDnInvenSlotDlg*>(m_groupTabDialog.GetDialog(i+1));
		pInvenSlotDlg->SetUseItemCnt(0);
	}

	DWORD dwTemp = dwItemCnt%ITEM_MAX;
	if( dwTemp > 0 )
	{
		pInvenSlotDlg = static_cast<CDnInvenSlotDlg*>(m_groupTabDialog.GetDialog(dwPageCnt+1));
		pInvenSlotDlg->SetUseItemCnt(dwTemp);
	}

#ifdef PRE_ADD_INVEN_EXTENSION
	if( m_SlotType == ST_INVENTORY )
	{
		for( i = 0; i<(int)m_vecTabButton.size(); i++ )
		{
#if defined( PRE_PERIOD_INVENTORY )
			if( PAGE_PERIOD == i )
				continue;
#endif	// #if defined( PRE_PERIOD_INVENTORY )

			SUIElement *pElement = m_vecTabButton[i]->GetElement(0);
			if( !pElement ) continue;

			bool bForceAlpha = false;
			int nOpenCnt = ITEM_MAX;
			if( dwItemCnt <= i*ITEM_MAX )
			{
				bForceAlpha = true;
				nOpenCnt = 0;
			}
			else if( dwItemCnt > i*ITEM_MAX && dwItemCnt < (i+1)*ITEM_MAX )
			{
				nOpenCnt = dwItemCnt%ITEM_MAX;
			}
			else if( dwItemCnt >= (i+1)*ITEM_MAX )
			{
			}

			EtColor Color = pElement->TextureColor.dwColor[UI_STATE_NORMAL];
			Color.a = bForceAlpha ? 0.4f : 1.0f;
			pElement->TextureColor.dwColor[UI_STATE_NORMAL] = Color;
			m_vecTabButton[i]->SetIgnoreAdjustAlpha( bForceAlpha );

			if( nOpenCnt == ITEM_MAX )
				m_vecTabButton[i]->SetTooltipText( L"" );
			else
			{
				WCHAR wszTemp[128] = {0,};
				swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4868 ), nOpenCnt, ITEM_MAX );
				m_vecTabButton[i]->SetTooltipText( wszTemp );
			}
		}
	}
#endif
}

bool CDnInvenDlg::IsEmptySlot()
{
	CEtUIDialogGroup::DIALOG_VEC &vecDialog = m_groupTabDialog.GetDialogList();
	for( int i=0; i<(int)vecDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecDialog[i];
		if( ((CDnInvenSlotDlg*)id_dlg_pair.second)->IsEmptySlot() )
		{
			return true;
		}
	}

	return false;
}

int CDnInvenDlg::GetEmptySlot()
{
	int nSlotIndex(-1);

	CEtUIDialogGroup::DIALOG_VEC &vecDialog = m_groupTabDialog.GetDialogList();
	for( int i=0; i<(int)vecDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecDialog[i];
		nSlotIndex = ((CDnInvenSlotDlg*)id_dlg_pair.second)->GetEmptySlot();
		if( nSlotIndex != -1 )
		{
			return nSlotIndex;
		}
	}

	return nSlotIndex;
}

int CDnInvenDlg::GetEmptySlotCount()
{
	int nSlotCount(0);

	CEtUIDialogGroup::DIALOG_VEC &vecDialog = m_groupTabDialog.GetDialogList();
	for( int i=0; i<(int)vecDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecDialog[i];
		nSlotCount += ((CDnInvenSlotDlg*)id_dlg_pair.second)->GetEmptySlotCount();
	}

	return nSlotCount;
}

void CDnInvenDlg::ShowPage( int nPageNum, bool bShow )
{
	m_vecTabButton[nPageNum]->Show(bShow);
}

void CDnInvenDlg::ReleaseNewGain()
{
	CEtUIDialogGroup::DIALOG_VEC &vecDialog = m_groupTabDialog.GetDialogList();
	for( int i=0; i<(int)vecDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecDialog[i];
		((CDnInvenSlotDlg*)id_dlg_pair.second)->ReleaseNewGain();
	}
}

int CDnInvenDlg::GetRegisteredItemCount()
{
	int nCount = 0;

	CEtUIDialogGroup::DIALOG_VEC &vecDialog = m_groupTabDialog.GetDialogList();
	for( int i=0; i<(int)vecDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecDialog[i];
		nCount += ((CDnInvenSlotDlg*)id_dlg_pair.second)->GetRegisteredSlotCount();
	}

	return nCount;
}

void CDnInvenDlg::DisablePressedButtonSplitMode()
{
	CEtUIDialogGroup::DIALOG_VEC &vecDialog = m_groupTabDialog.GetDialogList();
	for( int i=0; i<(int)vecDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecDialog[i];
		if( ((CDnInvenSlotDlg*)id_dlg_pair.second)->IsShow() )
			((CDnInvenSlotDlg*)id_dlg_pair.second)->DisablePressedButtonSplitMode();
	}
}