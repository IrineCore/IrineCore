#include "StdAfx.h"
#include "DnImageBlindDlg.h"
#include "DnImageLeftBlindNpcDlg.h"
#include "DnImageRightBlindNpcDlg.h"
#include "TaskManager.h"
#include "DnCommonTask.h"

#ifdef PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER 
#include "DnBlindDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#define NO_SLIDE_ANIMATION



CDnImageBlindDlg::CDnImageBlindDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_fCaptionDelta(FLT_MAX)
, m_pBlindStatic(NULL)
, m_pBlindCationTextBox(NULL)
, m_BlindState(E_BLIND_CLOSE)
, m_pLeftImageBlindCaptionDlg( NULL )
, m_pRightImageBlindCaptionDlg( NULL )
{
}

CDnImageBlindDlg::~CDnImageBlindDlg()
{
	SAFE_DELETE(m_pRightImageBlindCaptionDlg);
	SAFE_DELETE(m_pLeftImageBlindCaptionDlg);
}

void CDnImageBlindDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "NpcBlindDlg.ui" ).c_str(), bShow );
}

void CDnImageBlindDlg::InitialUpdate()
{
	m_pBlindStatic		  = GetControl<CEtUIStatic>("ID_STATIC_BLIND");
	m_pBlindCationTextBox = GetControl<CEtUITextBox>("ID_TEXTBOX_NPC");

	m_pLeftImageBlindCaptionDlg = new CDnImageLeftBlindNpcDlg( UI_TYPE_CHILD, this );
	m_pLeftImageBlindCaptionDlg->Initialize( false );

	m_pRightImageBlindCaptionDlg = new CDnImageRightBlindNpcDlg( UI_TYPE_CHILD, this );
	m_pRightImageBlindCaptionDlg->Initialize( false );

	UpdateBlind();
}

void CDnImageBlindDlg::Show( bool bShow )
{
	if(bShow == m_bShow) return;

	if(bShow) {
#ifndef NO_SLIDE_ANIMATION		
		m_BlindState = E_BLIND_OPEN;

		SUICoord BlindCoord;
		m_pBlindStatic->GetUICoord(BlindCoord);
		BlindCoord.fY = BlindCoord.fHeight;
		m_pBlindStatic->SetUICoord(BlindCoord);
#endif
	}
	m_pBlindStatic->Show(bShow);
	m_pBlindCationTextBox->Show(bShow);

	CEtUIDialog::Show(bShow);
}

void CDnImageBlindDlg::Process( float fElapsedTime )
{
	if(!IsShow()) return;
	if(!m_pLeftImageBlindCaptionDlg || !m_pRightImageBlindCaptionDlg) return;

	CEtUIDialog::Process( fElapsedTime );

	// Caption handling
	bool bShowCaption = false;
	if( m_fCaptionDelta == FLT_MAX ) 
		bShowCaption = true;
	else 
	{
		if( m_fCaptionDelta > 0.f ) 
		{
			m_fCaptionDelta -= fElapsedTime;
			
			if( m_fCaptionDelta <= 0.f ) 
				m_fCaptionDelta = 0.f;
			else bShowCaption = true;
		}
	}
	m_pBlindCationTextBox->Show(bShowCaption);

#ifndef NO_SLIDE_ANIMATION
	// Handle blind window
	SUICoord BlindCoord;
	if( m_pBlindStatic->IsShow() && m_BlindState == E_BLIND_OPEN)
	{
		m_pBlindStatic->GetUICoord(BlindCoord);

		if(BlindCoord.fY > 0.0f)
			BlindCoord.fY -= fElapsedTime;
		else
			BlindCoord.fY = 0.0f;

		m_pBlindStatic->SetUICoord(BlindCoord);
	}
	else
	{
		m_pRightImageBlindCaptionDlg->Show(false);
		m_pLeftImageBlindCaptionDlg->Show(false);

		m_pBlindStatic->GetUICoord(BlindCoord);

		if(BlindCoord.fY < BlindCoord.fHeight)
		{
			BlindCoord.fY += fElapsedTime;
			m_pBlindStatic->SetUICoord(BlindCoord);
		}
		else 
		{
			BlindCoord.fY = BlindCoord.fHeight;
			m_pBlindStatic->SetUICoord(BlindCoord);
			Show(false);
		}
	}
#endif // NO_SLIDE_ANIMATION
}

void CDnImageBlindDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );
}

void CDnImageBlindDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /*= 0 */ )
{
	SetCmdControlName( pControl->GetControlName() );
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnImageBlindDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	switch( uMsg )
	{
	case WM_KEYDOWN:
		{
			if(wParam == VK_ESCAPE)
			{
				GetInterface().CloseImageBlind(true);
				return true;
			}
		}
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnImageBlindDlg::OnChangeResolution()
{
	CEtUIDialog::OnChangeResolution();
	UpdateBlind();
}

void CDnImageBlindDlg::SetCaption( const wchar_t *wszCaption, int nDelay )
{
	if(!m_pBlindCationTextBox) return;

	if(nDelay <= 0) m_fCaptionDelta = FLT_MAX;
	else m_fCaptionDelta = nDelay / 1000.f;

	ClearCaptionBox();

	std::vector<std::wstring> vWStringList;
	TokenizeW(wszCaption, vWStringList, L"\\n");
	
	for(int i = 0 ; i < (int)vWStringList.size() ; ++i)
	{
		m_pBlindCationTextBox->AddText(vWStringList[i].c_str());
	}
}

void CDnImageBlindDlg::SetNPCImage( bool bShowNPCName, WCHAR* wszName, const char* szTextureFileName, int nImageType )
{
	if(!m_pRightImageBlindCaptionDlg || !m_pLeftImageBlindCaptionDlg)
		return;

	m_pRightImageBlindCaptionDlg->Show(false);
	m_pLeftImageBlindCaptionDlg->Show(false);

	if(nImageType == E_NPC_IMAGE_LEFT)
	{
		//std::string aa = "DwcEngineer.dds";
		//std::string aa = "WorldMap.dds";
		//std::string aa = "DwcElementallord.dds";
		
		m_pLeftImageBlindCaptionDlg->SetNpcTexture(wszName, szTextureFileName);
		//m_pLeftImageBlindCaptionDlg->SetNpcTexture(wszName, aa.c_str());
		m_pLeftImageBlindCaptionDlg->Show(true);
	}
	else if(nImageType == E_NPC_IMAGE_RIGHT)
	{
		m_pRightImageBlindCaptionDlg->SetNpcTexture(wszName, szTextureFileName);
		m_pRightImageBlindCaptionDlg->Show(true);
	}
}

void CDnImageBlindDlg::UpdateBlind()
{
	m_uiBlind.fX = -GetScreenWidthBorderSize();
	m_uiBlind.fY = -GetScreenHeightBorderSize();
	m_uiBlind.fWidth = GetScreenWidthRatio();
	m_uiBlind.fHeight = GetScreenHeightRatio();
	
	if(m_pBlindStatic) 
		m_pBlindStatic->SetUICoord(m_uiBlind);
}

void CDnImageBlindDlg::CloseBlind()
{
#ifdef NO_SLIDE_ANIMATION
	this->Show(false);
#else
	m_BlindState = E_BLIND_CLOSE;
#endif // NO_SLIDE_ANIMATION
}

#endif