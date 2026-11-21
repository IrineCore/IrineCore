#pragma once

#include "DnCustomDlg.h"

class CDnPackageBoxResultAutoDlg : public CDnCustomDlg
{
public:
	CDnPackageBoxResultAutoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPackageBoxResultAutoDlg();

protected:
	CEtUIStatic *m_pStaticTitle;
	CEtUIProgressBar *m_pProgressBar;
	float m_fDuration;
	float m_fElapsed;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void StartAutoOpen( const WCHAR *wszTitle, float fDuration = 1.5f );
};
