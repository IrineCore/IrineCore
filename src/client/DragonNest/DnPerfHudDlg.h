#pragma once

#include "EtUIDialog.h"

class CDnPerfHudDlg : public CEtUIDialog
{
public:
	CDnPerfHudDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_TOP_MSG, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPerfHudDlg( void );

protected:
	CEtUIStatic *m_pStaticFps;
	CEtUIStatic *m_pStaticDivider;
	CEtUIStatic *m_pStaticPing;

	DWORD m_dwFpsLastTick;
	int m_nFpsFrameCount;
	int m_nDisplayFps;
	int m_nLastDisplayedFps;
	int m_nLastDisplayedPing;

	bool m_bDragging;
	POINT m_ptLastMouse;

	void RefreshPerfText( bool bForce );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );
	virtual void Render( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};
