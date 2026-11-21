#include "StdAfx.h"
#include "DnGameTipDlg.h"
#include "DNClientScriptAPI.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGameTipDlg::CDnGameTipDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnSlideCaptionDlg( dialogType, pParentDialog, nID, pCallback )
{
}

CDnGameTipDlg::~CDnGameTipDlg(void)
{
}

void CDnGameTipDlg::InitialUpdate()
{
	CDnSlideCaptionDlg::InitialUpdate();

	// No need to move position, because it moves automatically in process to fit the blind.
	//m_pCation->SetPosition();
}

void CDnGameTipDlg::UpdateBlind()
{
	m_uiBlind.fWidth = GetScreenWidthRatio();
	m_uiBlind.fHeight = m_pCation->GetUICoord().fHeight;
	m_uiBlind.fX = -GetScreenWidthBorderSize();
	m_uiBlind.fY = GetScreenHeightRatio() - GetScreenHeightBorderSize() - m_uiBlind.fHeight;
	// (Current resolution / default resolution) * scale - ((left value - 1.0) / 2) = bottommost float value of current screen.
	// Subtracting Height here places it flush on the floor.
}

void CDnGameTipDlg::Render( float fElapsedTime )
{
	if( CGlobalInfo::GetInstance().IsPlayingCutScene() ) return;
	if( IsPlayingTriggerCamera() ) return;

	CDnSlideCaptionDlg::Render( fElapsedTime );
}