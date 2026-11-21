#include "StdAfx.h"
#include "DnPVPObserverDlg.h"
#include "DnPVPLobbyChatTabDlg.h"
#include "DnPVPGameRoomDlg.h"
#include "DnInterface.h"
#include "DnPvPObserverItemDlg.h"
#include "DnPVPLobbyChatTabDlg.h"

CDnPVPObserverDlg::CDnPVPObserverDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pObserverListBox(NULL)
, m_pPVPPopupDlg(NULL)
{
}

CDnPVPObserverDlg::~CDnPVPObserverDlg(void)
{
	SAFE_DELETE(m_pPVPPopupDlg);
}

void CDnPVPObserverDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpObserverDlg.ui" ).c_str(), bShow );
	Show(false);
}

void CDnPVPObserverDlg::InitialUpdate()
{
	m_pObserverListBox = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_OBSERVER");
	
	m_pPVPPopupDlg = new CDnPVPPopupDlg(UI_TYPE_CHILD,this);
	m_pPVPPopupDlg->Initialize(false);

}


void CDnPVPObserverDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	if(nCommand == EVENT_LISTBOX_SELECTION) // On list box click
	{
		CEtUIListBoxEx* pUI = GetInterface().GetPVPLobbyChatTabDlg()->GetObserverListBox();
		pUI->SetRenderSelectBar(true); // Redisplay the cleared select image
		pUI->GetSelectedIndex();
		GetInterface().GetPVPLobbyChatTabDlg()->SetSelectedListBox(true);
		GetInterface().GetGameRoomDlg()->EnableKickButton(true); // Enable the kick button.
		GetInterface().GetGameRoomDlg()->EnableChangeMasterButton(false);
		GetInterface().GetGameRoomDlg()->ResetSelectButton();
	}
}


void CDnPVPObserverDlg::Show( bool bShow )
{
	if(m_pPVPPopupDlg && !bShow)
	{
		m_pPVPPopupDlg->Show(false);
	}

	if(m_pObserverListBox)
		m_pObserverListBox->Show(bShow);

	CEtUIDialog::Show(bShow);
}


bool CDnPVPObserverDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	switch( uMsg )
	{

	case WM_LBUTTONDOWN:
		{

			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fMouseX, fMouseY;
			fMouseX = MousePoint.x / GetScreenWidth();
			fMouseY = MousePoint.y / GetScreenHeight();

			if( IsMouseInDlg() ) {
				if( m_pPVPPopupDlg->IsShow() ) {
					SUICoord uiCoord;
					m_pPVPPopupDlg->GetDlgCoord( uiCoord );
					if( !uiCoord.IsInside( fMouseX, fMouseY ) ) {
						ShowChildDialog( m_pPVPPopupDlg, false );
						return true;
					}
				}
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fMouseX, fMouseY;
			fMouseX = MousePoint.x / GetScreenWidth();
			fMouseY = MousePoint.y / GetScreenHeight();

			bool bIsInsideItem = false;
			std::wstring wszName;

				
			for( int i = m_pObserverListBox->GetScrollBar()->GetTrackPos(); i < m_pObserverListBox->GetSize(); ++i ) {
					CDnPVPObserverItemDlg *pDlg = m_pObserverListBox->GetItem<CDnPVPObserverItemDlg>(i);
					if( !pDlg ) continue;
					SUICoord uiCoord;
					pDlg->GetDlgCoord( uiCoord );
					if( uiCoord.IsInside( fMouseX, fMouseY ) ) {
						bIsInsideItem = true;
						m_pPVPPopupDlg->SetTargetName(GetInterface().GetPVPLobbyChatTabDlg()->GetNameFromList(i));
						break;
					}
			}

			if( bIsInsideItem ) {

				SUICoord sDlgCoord;
				static float fyBGap(16.f /DEFAULT_UI_SCREEN_WIDTH);   // Looks better positioned slightly to the right..

				ShowChildDialog( m_pPVPPopupDlg, false );
				m_pPVPPopupDlg->SetPosition( fMouseX+fyBGap, fMouseY );

				m_pPVPPopupDlg->GetDlgCoord( sDlgCoord );
				if( (sDlgCoord.Right()) > GetScreenWidthRatio() )
					sDlgCoord.fX -= (sDlgCoord.Right() - GetScreenWidthRatio()); //  Push back if overflowing off screen.
				m_pPVPPopupDlg->SetDlgCoord( sDlgCoord );
				ShowChildDialog( m_pPVPPopupDlg, true );
				GetInterface().GetGameRoomDlg()->ShowPVPPopUpDlg(false);     // Duplicates disallowed
			}
		
		}
		break;

	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );

}
