#include "StdAfx.h"
#include "DnSlideCaptionDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSlideCaptionDlg::CDnSlideCaptionDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_nLastSlideIndex(-1)
, m_bShowSlideText(false)
, m_bRender(false)
, m_srcColor((DWORD)0x00000000)
, m_destColor((DWORD)0x00000000)
, m_renderColor((DWORD)0x00000000)
, m_pCation(NULL)
{
}

CDnSlideCaptionDlg::~CDnSlideCaptionDlg(void)
{
}

void CDnSlideCaptionDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SlideCaptionDlg.ui" ).c_str(), bShow );
}

void CDnSlideCaptionDlg::InitialUpdate()
{
	m_pCation = GetControl<CEtUIStatic>("ID_CAPTION");

	UpdateBlind();
}

void CDnSlideCaptionDlg::Show( bool bShow )
{ 
	if( bShow )
	{
	}
	else
	{
		// Because EtDialog::CloseAllDialog() is called when something like blind opens,
		// If DelCallCaption is called here, it will also be deleted when talking to an NPC.
		//DelAllCaption();
	}

	CEtUIDialog::Show( bShow );
}

void CDnSlideCaptionDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	// Alpha processing for the shaded area must run even when text is invisible (when closing), so process it at the very top.
	if( m_fFadeTime > 0.0f )
	{
		EtColorLerp( &m_renderColor, &m_srcColor, &m_destColor, 1.0f-(1/powf(m_fConst,2)*powf(m_fFadeTime,2) ) );
		m_fFadeTime -= fElapsedTime;
	}
	else
	{
		m_renderColor = m_destColor;
	}

	// While the text is sliding,
	if( m_bShowSlideText )
	{
		m_pCation->SetPosition( m_pCation->GetUICoord().fX - fElapsedTime / 10.0f, m_uiBlind.fY );

		// When exiting off the left side of the screen,
		if( m_pCation->GetUICoord().fX + m_fTextWidth < m_uiBlind.fX )
		{
			m_bShowSlideText = false;

			// Since there is no need to draw the Static control
			Show( false );

			if( m_listSlideInfo.empty() )
			{
				m_fConst = 1.0f;
				m_fFadeTime = 1.0f;
				m_srcColor = m_renderColor;
				m_destColor = (DWORD)0x00000000;
			}
		}
	}

	// Do not process if there is no list.
	if( m_listSlideInfo.empty() )
		return;

	std::list<SSlideInfo>::iterator iter = m_listSlideInfo.begin();
	for( ; iter != m_listSlideInfo.end(); )
	{
		iter->m_fRemainShowTime -= fElapsedTime;
		if( iter->m_fRemainShowTime <= 0.0f )
		{
			iter = m_listSlideInfo.erase( iter );
			continue;
		}
		++iter;
	}

	// If all are deleted, reset the index.
	if( m_listSlideInfo.empty() )
	{
		m_nLastSlideIndex = -1;
		return;
	}

	// If no text is currently rendering, set one text.
	if( !m_bShowSlideText )
	{
		// Refer to m_nLastSlideIndex to get the iter to render this time.
		int i = 0;
		std::list<SSlideInfo>::iterator iterNext = m_listSlideInfo.begin();
		for( ; iterNext != m_listSlideInfo.end(); )
		{
			++iterNext;
			if( i == m_nLastSlideIndex )
				break;
			++i;
		}

		// Check validity and update Last
		if( iterNext == m_listSlideInfo.end() )
		{
			// If exceeding the notice list size, wrap around to get the first item (circular).
			iterNext = m_listSlideInfo.begin();
			m_nLastSlideIndex = 0;

			// Even so, if it is end(), it means there is no list at all, which cannot happen under normal circumstances.
			if( iterNext == m_listSlideInfo.end() )
			{
				ASSERT(0&&"Process is running even though slide notice list is empty.");
				return;
			}
		}
		else
		{
			++m_nLastSlideIndex;
		}

		// Send info to text to be rendered.
		m_RenderSlideInfo = *iterNext;

		// After setting static controls,
		SetCaption( m_RenderSlideInfo.m_wszMsg.c_str(), m_RenderSlideInfo.m_dwColor );

		// Calculate region and position.
		SUICoord textCoord;
		CalcTextRect( m_RenderSlideInfo.m_wszMsg.c_str(), m_pCation->GetElement(0), textCoord );
		m_fTextWidth = textCoord.fWidth;
		m_pCation->SetPosition( m_uiBlind.fX+m_uiBlind.fWidth, m_uiBlind.fY );

		// Show static control so it can be drawn.
		Show( true );
		m_bShowSlideText = true;

		// Blend processing for shaded area.
		m_fConst = 1.0f;
		m_fFadeTime = 1.0f;
		if( m_renderColor.a != 0.0f )
			m_srcColor = m_renderColor;
		else
			m_srcColor = (DWORD)0x00000000;
		m_destColor = (DWORD)0x80000000;
	}
}

void CDnSlideCaptionDlg::Render( float fElapsedTime )
{
	if( m_renderColor.r > 0.0f || m_renderColor.g > 0.0f || m_renderColor.b > 0.0f )
	{
		ASSERT(0&&"Who on earth is putting a color value into slide background color?");
		return;
	}

	// Checking only by alpha value seems to cause issues in some cases.
	if( !m_bRender )
		return;

	if( m_renderColor.a == 0.0f )
		return;

	DrawRect( m_uiBlind, m_renderColor );

	// If there is text that must be drawn regardless of Show state, draw it.
	// This is needed so that even when CEtUIDialog::CloseAllDialog etc. is called due to blind etc.,
	// In-progress blind text may continue rendering.
	CEtUIControl *pControl(NULL);
	int nVecCtlSize = ( int )m_vecControl.size();
	for( int i = 0; i < nVecCtlSize; i++ )
	{
		pControl = m_vecControl[i];
		if( !pControl ) continue;
		pControl->Render( fElapsedTime );
	}
}

void CDnSlideCaptionDlg::AddCaption( LPCWSTR wszMsg, DWORD dwColor, float fShowTime )
{
	SSlideInfo Info;
	Info.SetInfo( wszMsg, dwColor, fShowTime );
	m_listSlideInfo.push_back( Info );
	m_bRender = true;
}

void CDnSlideCaptionDlg::CloseCaption( bool bForce )
{
	m_listSlideInfo.clear();

	if( bForce )
	{
		m_fConst = 1.0f;
		m_fFadeTime = 0.0f;
		m_srcColor = (DWORD)0x00000000;
		m_destColor = (DWORD)0x00000000;
		m_renderColor = (DWORD)0x00000000;

		m_bShowSlideText = false;
		Show( false );
		m_bRender = false;
	}
}

void CDnSlideCaptionDlg::DelCaption( int nIndex )
{
	if( m_listSlideInfo.empty() ) return;

	int i = 0;
	std::list<SSlideInfo>::iterator iter = m_listSlideInfo.begin();
	for( ; iter != m_listSlideInfo.end(); )
	{
		if( i == nIndex )
		{
			iter = m_listSlideInfo.erase( iter );
			return;
		}
		++iter;
		++i;
	}
}

void CDnSlideCaptionDlg::DelCaption( LPCWSTR wszMsg )
{
	if( m_listSlideInfo.empty() ) return;

	std::list<SSlideInfo>::iterator iter = m_listSlideInfo.begin();
	for( ; iter != m_listSlideInfo.end(); )
	{
		if( iter->m_wszMsg == wszMsg )
		{
			iter = m_listSlideInfo.erase( iter );
			return;
		}
		++iter;
	}
}

void CDnSlideCaptionDlg::SetCaption( LPCWSTR wszMsg, DWORD dwColor )
{
	m_pCation->SetText( wszMsg );
	m_pCation->SetTextColor( dwColor );
	m_pCation->Show(true);
}

void CDnSlideCaptionDlg::UpdateBlind()
{
	m_uiBlind.fX = -GetScreenWidthBorderSize();
	m_uiBlind.fY = -GetScreenHeightBorderSize();
	m_uiBlind.fWidth = GetScreenWidthRatio();
	m_uiBlind.fHeight = m_pCation->GetUICoord().fHeight;// * GetScreenHeightRatio(); // Why did BlindDlg multiply by this Ratio?
}

void CDnSlideCaptionDlg::OnChangeResolution()
{
	CEtUIDialog::OnChangeResolution();
	UpdateBlind();

	// Recalculate TextWidth since it may change depending on resolution.
	SUICoord textCoord;
	CalcTextRect( m_RenderSlideInfo.m_wszMsg.c_str(), m_pCation->GetElement(0), textCoord );
	m_fTextWidth = textCoord.fWidth;
}