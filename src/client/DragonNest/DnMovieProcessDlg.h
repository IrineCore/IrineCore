#pragma once
#include "EtUIDialog.h"

class CDnMovieProcessDlg : public CEtUIDialog
{
public:
	CDnMovieProcessDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMovieProcessDlg(void);

protected:
	CEtUIStatic *m_pStatic;
	CEtUIButton *m_pProcessComplete;	// Invisible dummy button.
	CEtUIButton *m_pCancelButton;
	CEtUIProgressBar *m_pProgressBarTime;

	float m_fMaxTimer;
	float m_fTimer;
	char m_cFlag;

public:
	void DisableFlag() { m_cFlag = 0; }	// Call when needing to close Dialog manually without Cancel handling via UI CallBack
	void SetInfo( LPCWSTR wszMessage, float fTimer, int nID, CEtUICallback *pCall, bool bShowButton = true );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};