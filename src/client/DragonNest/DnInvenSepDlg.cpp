#include "StdAfx.h"
#include "DnInvenSepDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnInvenSepDlg::CDnInvenSepDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
	, m_pEditBox(NULL)
	, m_nEditValue(0)
	, m_nMaxValue(0)
	, m_pButtonUp( NULL )
	, m_pButtonDown( NULL )
{
}

CDnInvenSepDlg::~CDnInvenSepDlg(void)
{
}

void CDnInvenSepDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "InvenItemSepDlg.ui" ).c_str(), bShow );
}

void CDnInvenSepDlg::InitialUpdate()
{
	m_pEditBox = GetControl<CEtUIEditBox>("ID_EDITBOX_NUM");

	m_pButtonUp = GetControl<CEtUIButton>("ID_BUTTON_UP");
	m_pButtonDown = GetControl<CEtUIButton>("ID_BUTTON_DOWN");
}

void CDnInvenSepDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_UP" ) )
		{
			int nValue = m_pEditBox->GetTextToInt();
			nValue++;

			if( nValue > m_nMaxValue )
				nValue = m_nMaxValue;

			m_pEditBox->SetIntToText( nValue );
			return;
		}

		if( IsCmdControl("ID_BUTTON_DOWN") )
		{
			int nValue = m_pEditBox->GetTextToInt();
			nValue--;

			if( nValue < 1 )
				nValue = 1;

			m_pEditBox->SetIntToText( nValue );
			return;
		}
	}

	if( nCommand == EVENT_EDITBOX_CHANGE )
	{
		if( IsCmdControl("ID_EDITBOX_NUM" ) )
		{
			int nCount = m_pEditBox->GetTextToInt();
			if( nCount > m_nMaxValue )
			{
				m_pEditBox->SetIntToText( m_nMaxValue );
			}
			else if( nCount >= 0 && nCount < 1 )
			{
				m_pEditBox->SetIntToText( 1 );
			}
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnInvenSepDlg::SetEditValue( int nValue, int nMaxValue )
{
	m_nEditValue = 0;
	m_nMaxValue = nMaxValue;

	if( nValue >= 1 && nValue <= m_nMaxValue )
	{
		m_pEditBox->SetIntToText( nValue );
	}
	else
	{
		ASSERT(0&&"CDnInvenSepDlg::SetEditValue");
	}
}

int CDnInvenSepDlg::GetEditValue()
{
	return m_pEditBox->GetTextToInt();
}

void CDnInvenSepDlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		// Smart move feature
		SUICoord uiCoord;
		CEtUIButton *pSmartMoveControl = GetControl<CEtUIButton>("ID_BUTTON_OK");
		pSmartMoveControl->GetUICoord(uiCoord);
		CEtUIDialog *pParentDlg = pSmartMoveControl->GetParent();
		m_uiSmartMovePos.fX = pParentDlg->GetXCoord() + uiCoord.fX + uiCoord.fWidth / 2.0f;
		m_uiSmartMovePos.fY = pParentDlg->GetYCoord() + uiCoord.fY + uiCoord.fHeight / 2.0f;

		m_uiPrePos.fX = pParentDlg->s_fScreenMouseX;
		m_uiPrePos.fY = pParentDlg->s_fScreenMouseY;

		POINT pt;
		pt.x = (LONG)(GetScreenWidth() * m_uiSmartMovePos.fX);
		pt.y = (LONG)(GetScreenHeight() * m_uiSmartMovePos.fY);
		CDnMouseCursor::GetInstance().SetCursorPos( pt.x, pt.y );

		RequestFocus(m_pEditBox);
	}
	else
	{
		if( m_uiPrePos.fX != 0.f && m_uiPrePos.fY != 0.f ) {
			POINT pt;
			pt.x = (LONG)(GetScreenWidth() * m_uiPrePos.fX);
			pt.y = (LONG)(GetScreenHeight() * m_uiPrePos.fY);
			CDnMouseCursor::GetInstance().SetCursorPos( pt.x, pt.y );
		}

		m_pEditBox->ClearText();
		m_nEditValue = m_nMaxValue = 0;
	}
}

bool CDnInvenSepDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	if( uMsg == WM_MOUSEWHEEL )
	{
		POINT MousePoint;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );

		float fMouseX, fMouseY;
		PointToFloat( MousePoint, fMouseX, fMouseY );

		if( IsMouseInDlg() )
		{
			UINT uLines;
			SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
			int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
			if( nScrollAmount > 0 )
			{
				ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonUp, 0 );
				return true;
			}
			else if( nScrollAmount < 0 )
			{
				ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonDown, 0 );
				return true;
			}
		}
	}

	// To make up/down keys work even when focus is on the edit box (quantity input), UI tool hotkeys won't work.
	// So handle it directly in MsgProc.
	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_UP )
		{
			ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonUp, 0 );
			return true;
		}
		else if ( wParam == VK_DOWN )
		{
			ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonDown, 0 );
			return true;
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}