#include "StdAfx.h"
#include "DnTextBoxDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnTextBoxDlg::CDnTextBoxDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCommDlg( dialogType, pParentDialog, nID, pCallback  )
	, m_pTextBoxText(NULL)
{
	m_dwCountDown = 0;
	m_fElapsed_CountDownTime = 0.0f;

	m_fX = 0.0f;
	m_fY = 0.0f;
	m_nFormat = 0;
	m_nPos = 0;
#ifdef PRE_FIX_TEXTDLG_TIMER
	m_bTimer = false;
#endif
}

CDnTextBoxDlg::~CDnTextBoxDlg(void)
{
}

void CDnTextBoxDlg::Initialize( bool bShow )
{
	if( !m_pTextBoxText )
	{
		CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "TextBoxDlg.ui" ).c_str(), bShow );
		SetFadeMode( CEtUIDialog::CancelRender );
	}
}

void CDnTextBoxDlg::InitialUpdate()
{
	m_pTextBoxText = GetControl<CEtUITextBox>("ID_TEXTBOX_TEXT");
}

void CDnTextBoxDlg::Process( float fElapsedTime )
{
	Process_CountDown( fElapsedTime );

	if( m_fDelayTime > 0.0f )
	{
		m_fDelayTime -= fElapsedTime;
	}
#ifdef PRE_FIX_TEXTDLG_TIMER
	else
	{
		if (m_bTimer)
		{
			Show(false);
			return;
		}
	}
#endif

	CEtUIDialog::Process( fElapsedTime );
}

void CDnTextBoxDlg::Show( bool bShow )
{ 
	if( bShow )
	{
	}
	else
	{
		m_fX = 0.0f;
		m_fY = 0.0f;
		m_nPos = 0;
		m_nFormat = 0;
		m_fDelayTime = 0.0f;
#ifdef PRE_FIX_TEXTDLG_TIMER
		m_bTimer = false;
#endif
		m_strText.clear();
		m_pTextBoxText->ClearText();

		SetTextBox_VariableType( UITextBoxVariableType::UI_TEXTBOX_BOTH );
	}

	m_pTextBoxText->Show( bShow );
	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		// #31449 Issue where textbox dialog region was not updated.
		// Checked and found it is due to dialog optimization
		// When new text is set on a textbox dialog whose texture disappeared after showing once and hiding
		// After calculating the bounds including the new dialog, call UpdateRects at the end,
		// Because there is no texture at this time, all uiCoord values are normal, but
		// The issue was that UpdateFrameRectsEx was skipped without being called.
		// Still, not optimizing isn't great, so force calling UpdateRect once more when newly shown.
		// Must be called after CEtUIDialog::Show so the texture is in a loaded state.
		UpdateRects();
	}
}

void CDnTextBoxDlg::SetText( LPCWSTR wszStr, float fX, float fY, float fWidth, float fDelayTime )
{
	ASSERT( wszStr&&"CDnTextBoxDlg::SetText, wszStr is NULL!" );

	m_pTextBoxText->ClearText();

	SUICoord uiCoord;

	GetDlgCoord( uiCoord );
	uiCoord.SetPosition( fX*GetScreenWidthRatio(), fY*GetScreenHeightRatio() );
	uiCoord.SetSize( fWidth*GetScreenWidthRatio(), uiCoord.fHeight*GetScreenHeightRatio() );
	SetDlgCoord( uiCoord );

	uiCoord.SetPosition( 0.03f*GetScreenWidthRatio(), 0.03f*GetScreenHeightRatio() );
	uiCoord.SetSize( (fWidth-0.06f)*GetScreenWidthRatio(), (uiCoord.fHeight-0.06f)*GetScreenHeightRatio() );
	m_pTextBoxText->SetUICoord( uiCoord );

	m_pTextBoxText->SetText( wszStr );

	GetDlgCoord( uiCoord );
	{
		if( m_BaseDlgCoord.fHeight > uiCoord.fHeight )
		{
			uiCoord.fHeight = m_BaseDlgCoord.fHeight;
		}

		if( m_BaseDlgCoord.fWidth > uiCoord.fWidth )
		{
			uiCoord.fWidth = m_BaseDlgCoord.fWidth;

			// Note: Center the text box.
			//
			SUICoord textCoord;
			m_pTextBoxText->GetUICoord( textCoord );
			m_pTextBoxText->SetPosition( ((uiCoord.fWidth-textCoord.fWidth)/2.0f), 0.03f );
		}
	}
	SetDlgCoord( uiCoord );

	m_fDelayTime = fDelayTime;
#ifdef PRE_FIX_TEXTDLG_TIMER
	m_bTimer = (fDelayTime > 0.f);
#endif
}

void CDnTextBoxDlg::SetText( LPCWSTR wszStr, float fX, float fY, int nPos, int nFormat, float fDelayTime )
{
	ASSERT( wszStr&&"CDnTextBoxDlg::SetText, wszStr is NULL!" );

	m_strText = wszStr;
	m_fX = fX;
	m_fY = fY;
	m_nPos = nPos;
	m_nFormat = nFormat;
	m_fDelayTime = fDelayTime;
#ifdef PRE_FIX_TEXTDLG_TIMER
	m_bTimer = (fDelayTime > 0.f);
#endif

	UpdateTextBox();
}

void CDnTextBoxDlg::SetCountDown( int nCountDown, float fX, float fY, int nPos, int nFormat, float fDelayTime )
{
	m_dwCountDown = nCountDown;
	m_fElapsed_CountDownTime = 0.0f;

	m_fX = fX;
	m_fY = fY;
	m_nPos = nPos;
	m_nFormat = nFormat;
	m_fDelayTime = fDelayTime;
#ifdef PRE_FIX_TEXTDLG_TIMER
	m_bTimer = (fDelayTime > 0.f);
#endif

	Process_CountDown( 0.0f );
	UpdateTextBox();
	SetTextBox_VariableType( UITextBoxVariableType::UI_TEXTBOX_NONE );
}

void CDnTextBoxDlg::UpdateTextBox()
{
	if( !m_pTextBoxText )
		return;

	m_pTextBoxText->ClearText();

	SUIControlProperty *pProperty = m_pTextBoxText->GetProperty();
	switch( m_nFormat )
	{
	case 0:		pProperty->TextBoxProperty.AllignHori = AT_HORI_LEFT;		break;
	case 1:		pProperty->TextBoxProperty.AllignHori = AT_HORI_CENTER;	    break;
	default:	pProperty->TextBoxProperty.AllignHori = AT_HORI_LEFT;	    break;
	}

	m_pTextBoxText->SetText( m_strText.c_str() );

	/////////////////////////////////////////////////

	SUICoord uiCoord;
	GetDlgCoord( uiCoord );

	float fRatioX = m_fX*GetScreenWidthRatio();
	float fRatioY = m_fY*GetScreenHeightRatio();

	switch( m_nPos )
	{
	case 0: // Left, top
		uiCoord.SetPosition( fRatioX, fRatioY );
		break;
	case 1: //	Center, Top
		uiCoord.SetPosition( fRatioX-(uiCoord.fWidth/2.0f), fRatioY );
		break;
	case 2: // Right, top
		uiCoord.SetPosition( fRatioX-uiCoord.fWidth, fRatioY );
		break;
	case 3: // Left, center
		uiCoord.SetPosition( fRatioX, fRatioY-(uiCoord.fHeight/2.0f) );
		break;
	case 4: // Middle right
		uiCoord.SetPosition( fRatioX-uiCoord.fWidth, fRatioY-(uiCoord.fHeight/2.0f) );
		break;
	case 5: // Left, bottom
		uiCoord.SetPosition( fRatioX, fRatioY-uiCoord.fHeight );
		break;
	case 6: // Center, Bottom
		uiCoord.SetPosition( fRatioX-(uiCoord.fWidth/2.0f), fRatioY-uiCoord.fHeight );
		break;
	case 7: // Right, bottom
		uiCoord.SetPosition( fRatioX-uiCoord.fWidth, fRatioY-uiCoord.fHeight );
		break;
	case 8: // Center, Center
		uiCoord.SetPosition( fRatioX-(uiCoord.fWidth/2.0f), fRatioY-(uiCoord.fHeight/2.0f) );
		break;
	default:
		ASSERT(0&&"CDnTextBoxDlg::SetText, default case!");
		break;
	}

	SetDlgCoord( uiCoord );
}

void CDnTextBoxDlg::Process_CountDown( float fElapsedTime )
{
	if( m_dwCountDown == 0 )
		return;

	m_fElapsed_CountDownTime += fElapsedTime;

	DWORD dwRemainderTime = m_dwCountDown - (DWORD)m_fElapsed_CountDownTime;
	
	DWORD dwSecond = dwRemainderTime % 60;
	DWORD dwMinute = dwRemainderTime / 60;

	WCHAR wszTime[64] = {0,};
	swprintf_s( wszTime, _countof(wszTime), L"    %02d : %02d    ", dwMinute, dwSecond );
	m_strText = std::wstring( wszTime );

	m_pTextBoxText->ClearText();
	m_pTextBoxText->SetText( wszTime );
}

void CDnTextBoxDlg::SetTextBox_VariableType( int nType )
{
	SUIControlProperty *pProperty = m_pTextBoxText->GetProperty();
	pProperty->TextBoxProperty.VariableType = (UITextBoxVariableType)nType;
}

void CDnTextBoxDlg::OnChangeResolution()
{
	CEtUIDialog::OnChangeResolution();
	UpdateTextBox();
}