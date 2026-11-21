#include "StdAfx.h"
#include "DnCashShopInvenTabDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#include "DnInvenCashDlg.h"
#include "DnInvenSlotDlg.h"
/////////////////////////////////////////

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
CDnCashShopInvenTabDlg::CDnCashShopInvenTabDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CEtUIDialog(dialogType, pParentDialog, nID, pCallback, true)
, m_OrigHoriType( AT_HORI_NONE )
, m_OrigVertType( AT_VERT_NONE )
{
}

CDnCashShopInvenTabDlg::~CDnCashShopInvenTabDlg(void)
{
}

void CDnCashShopInvenTabDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CSInvenTab.ui" ).c_str(), bShow );
}

void CDnCashShopInvenTabDlg::InitialUpdate()
{
	GetControl<CEtUIRadioButton>("ID_TAB_BASIC01")->Show( false );
	GetControl<CEtUIRadioButton>("ID_TAB_BASIC02")->Show( false );
	GetControl<CEtUIRadioButton>("ID_TAB_BASIC03")->Show( false );
	GetControl<CEtUIRadioButton>("ID_TAB_BASIC04")->Show( false );
	GetControl<CEtUIRadioButton>("ID_TAB_BASIC05")->Show( false );

	CEtUIRadioButton * pRad = GetControl<CEtUIRadioButton>("ID_TAB_BASIC06");
	if( pRad ) pRad->Show( false );


}

void CDnCashShopInvenTabDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_BUTTON0"))
			Show(false);
	}
	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnCashShopInvenTabDlg::Show( bool bShow )
{
	if (bShow == m_bShow)
		return;

	if (bShow)
	{
		SetPositionCashShopInven(true);
	}
	else
	{
		SetPositionCashShopInven(false);
	}

	CEtUIDialog::Show(bShow);
}

void CDnCashShopInvenTabDlg::Render(float fElapsedTime)
{
	if( !IsAllowRender() ) return;

	// #42600 Same handling as chat room inventory.
	if( m_listChildDialog.size() > 0 && m_listChildDialog.front() != NULL && m_listChildDialog.front()->IsShow() )
		m_listChildDialog.front()->LoadDialogTexture();

	CEtUIDialog::Render(fElapsedTime);
}

void CDnCashShopInvenTabDlg::SetPositionCashShopInven( bool bCashShop )
{
	// This constant value must be modified directly.
	const float fOffsetX = -119.0f / DEFAULT_UI_SCREEN_WIDTH;
	const float fOffsetY = 60.0f / DEFAULT_UI_SCREEN_HEIGHT;

	if( bCashShop )
	{
		// Calling Render forcibly complicates MsgProc handling, so we chose to temporarily register it as a Child.
		// When shown, just change AllignType and DlgCoord, attach as child, and detach again.
		CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
		if( pInvenDlg )
		{
			CDnInvenCashDlg *pCashInvenDlg = pInvenDlg->GetCashInvenDlg();
			if( pCashInvenDlg )
			{
				std::vector<CDnInvenSlotDlg *> &pVecInvenSlotDlg = pCashInvenDlg->GetInvenSlotDlgList();

				// Get the alignment attribute of the current dialog,
				UIAllignHoriType HoriType;
				UIAllignVertType VertType;
				GetAllignType( HoriType, VertType );

				// Save the original,
				pCashInvenDlg->GetAllignType( m_OrigHoriType, m_OrigVertType );

				// Move
				pCashInvenDlg->SetAllignType( HoriType, VertType );

#ifdef PRE_ADD_CASHINVENTAB
				pCashInvenDlg->SetAllignTypeCashTab( HoriType, VertType );
#endif // PRE_ADD_CASHINVENTAB		

				pCashInvenDlg->MoveDialog( fOffsetX, fOffsetY );
				for( int i = 0; i < (int)pVecInvenSlotDlg.size(); ++i )
				{
					pVecInvenSlotDlg[i]->SetAllignType( HoriType, VertType );
					//pVecInvenSlotDlg[i]->MoveDialog( fOffsetX , fOffsetY ); // Move propagates to all child slot pages, so no separate call is needed.
				}

				// Set temporary child.
				AddChildDialog( pCashInvenDlg );
				ShowChildDialog( pCashInvenDlg, true );
			}
		}
	}
	else
	{
		CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
		if( pInvenDlg )
		{
			CDnInvenCashDlg *pCashInvenDlg = pInvenDlg->GetCashInvenDlg();
			if( pCashInvenDlg && pCashInvenDlg->IsShow() )
			{
				std::vector<CDnInvenSlotDlg *> &pVecInvenSlotDlg = pCashInvenDlg->GetInvenSlotDlgList();

				// Move
				pCashInvenDlg->SetAllignType( m_OrigHoriType, m_OrigVertType );

#ifdef PRE_ADD_CASHINVENTAB
				pCashInvenDlg->SetAllignTypeCashTab( m_OrigHoriType, m_OrigVertType );
#endif // PRE_ADD_CASHINVENTAB	

				pCashInvenDlg->MoveDialog( -fOffsetX, -fOffsetY );
				for( int i = 0; i < (int)pVecInvenSlotDlg.size(); ++i )
				{
					pVecInvenSlotDlg[i]->SetAllignType( m_OrigHoriType, m_OrigVertType );
					//pVecInvenSlotDlg[i]->MoveDialog( -fOffsetX , -fOffsetY ); // Move propagates to all child slot pages, so no separate call is needed.
				}

				// Detach child
				pCashInvenDlg->ReleaseNewGain();
				pCashInvenDlg->Show( false );
				m_listChildDialog.clear();
			}
		}
	}
}