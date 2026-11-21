#include "StdAfx.h"
#include "DnPlateListDlg.h"
#include "DnPlateListSelectDlg.h"
#include "DnPlateMainDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnPlateListDlg::CDnPlateListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_nCurSelect(-1)
, m_pStaticSelect(NULL)
{
}

CDnPlateListDlg::~CDnPlateListDlg(void)
{
}

void CDnPlateListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PlateListDlg.ui" ).c_str(), bShow );
}

void CDnPlateListDlg::InitialUpdate()
{
	m_pStaticSelect = GetControl<CEtUIStatic>("ID_SELECT");
	m_pStaticSelect->Show(false);

	char szControlName[32];
	for( int i = 0; i < MAX_PLATE_NUM; ++i )
	{
		sprintf_s(szControlName, _countof(szControlName), "ID_SLOT%d", i);
		m_sPlateItem[i].m_pItemSlotButton = GetControl<CDnItemSlotButton>(szControlName);

		sprintf_s(szControlName, _countof(szControlName), "ID_ITEM%d", i);
		m_sPlateItem[i].m_pStaticName = GetControl<CEtUIStatic>(szControlName);

		sprintf_s(szControlName, _countof(szControlName), "ID_BASE%d", i);
		m_sPlateItem[i].m_pStaticBase = GetControl<CEtUIStatic>(szControlName);

		m_sPlateItem[i].Clear();
	}
}

void CDnPlateListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnPlateListDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

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
				if( m_sPlateItem[nFindSelect].m_pStaticBase->IsShow() )
				{
					m_nCurSelect = nFindSelect;
					UpdateSelectBar();
					((CDnPlateListSelectDlg *)m_pParentDialog)->SelectPlate( m_nCurSelect );

					CDnPlateMainDlg *pParentDialog = (CDnPlateMainDlg *)m_pParentDialog->GetParentDialog();
					pParentDialog->ChangeStep( CDnPlateMainDlg::eStepMake );

					return true;
				}
				else
				{
					((CDnPlateListSelectDlg *)m_pParentDialog)->RefreshPlateList();

					m_pStaticSelect->Show(false);
					m_nCurSelect = -1;
				}
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		if( !IsMouseInDlg() ) break;
		CDnPlateMainDlg *pParentDialog = (CDnPlateMainDlg *)m_pParentDialog->GetParentDialog();
		pParentDialog->ProcessCommand( EVENT_BUTTON_CLICKED, false, pParentDialog->GetControl<CEtUIButton>("ID_BUTTON_OK"), 0 );
		return true;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}


void CDnPlateListDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_nCurSelect = -1;
	}
	else
	{
		m_pStaticSelect->Show(false);
	}

	CEtUIDialog::Show( bShow );
}

void CDnPlateListDlg::RefreshPlateList()
{
	for( int i = 0; i < MAX_PLATE_NUM; ++i )
		m_sPlateItem[i].Clear();

	m_pStaticSelect->Show(false);
	m_nCurSelect = -1;

	CDnPlateListSelectDlg *pParentDialog = (CDnPlateListSelectDlg *)m_pParentDialog;
	int * pPlateItemID = pParentDialog->GetPlateInfo();

	TItemInfo itemInfo;
	for( int itr = 0; itr < MAX_PLATE_NUM; ++itr )
	{
		if( CDnItem::MakeItemInfo( pPlateItemID[itr], 1, itemInfo ) == false ) break;

		m_sPlateItem[itr].m_pItem = GetItemTask().CreateItem( itemInfo );
		m_sPlateItem[itr].m_pItemSlotButton->SetItem(m_sPlateItem[itr].m_pItem, 0);
		m_sPlateItem[itr].m_pStaticBase->Show(true);
		m_sPlateItem[itr].m_pStaticName->Show(true);
		m_sPlateItem[itr].m_pStaticName->SetText( m_sPlateItem[itr].m_pItem->GetName() );
	}

	/*const CDnItemCompounder::S_PLATE_INFO* pPlateInfo = GetItemTask().GetPlateInfoByItemID( nPlateItemID );
ASSERT(pPlateInfo&&"Step passed properly, but plate ID appears to be missing.");
	if( pPlateInfo )
	{
		for( int i = 0; i < NUM_MAX_PLATE_COMPOUND_VARI; ++i )
		{
			int nCompoundTableID = pPlateInfo->aiCompoundTableID[i];
			if( nCompoundTableID > 0 )
			{
				CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
				GetItemTask().GetCompoundInfo( nCompoundTableID, &CompoundInfo );
				
				if( CompoundInfo.aiSuccessTypeOrCount[0] == -1 && CompoundInfo.aiSuccessItemID[0] > 0 )
				{
					// Crest Jewel will only produce the result at index 0.
					m_sPlateItem[i].SetInfo( CompoundInfo.aiSuccessItemID[0] );
				}
				else if(  CompoundInfo.aiSuccessTypeOrCount[0] == 0 )
				{
					// Since it is a random item, register only the plate item without an item slot.
					m_sPlateItem[i].SetInfo( 0 );
				}
			}
		}
	}*/
}

int CDnPlateListDlg::FindInsideItem( float fX, float fY )
{
	for( int i = 0; i < MAX_PLATE_NUM; i++ )
	{
		if( m_sPlateItem[i].IsInsideItem( fX, fY ) )
		{
			return i;
		}
	}

	return -1;
}

void CDnPlateListDlg::UpdateSelectBar()
{
	SUICoord uiCoordBase, uiCoordSelect;
	m_sPlateItem[m_nCurSelect].m_pStaticBase->GetUICoord(uiCoordBase);
	m_pStaticSelect->GetUICoord(uiCoordSelect);
	uiCoordSelect.fX = uiCoordBase.fX;// - 0.003f;
	uiCoordSelect.fY = uiCoordBase.fY;
	m_pStaticSelect->SetUICoord(uiCoordSelect);
	m_pStaticSelect->Show(true);
}

int CDnPlateListDlg::GetSelectedCompoundID()
{
	if( m_nCurSelect == -1 )
		return -1;

	CDnPlateMainDlg *pParentDialog = (CDnPlateMainDlg *)m_pParentDialog->GetParentDialog();
	int nPlateItemID = pParentDialog->GetPlateItemID();

	const CDnItemCompounder::S_PLATE_INFO* pPlateInfo = GetItemTask().GetPlateInfoByItemID( nPlateItemID );
	ASSERT(pPlateInfo&&"CDnPlateListDlg::GetSelectedCompoundID");
	return pPlateInfo->aiCompoundTableID[m_nCurSelect];
}
