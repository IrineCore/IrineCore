#pragma once
#include "EtUIDialog.h"

class CDnLoginDlg : public CEtUIDialog, public CEtUICallback
{
public:
	CDnLoginDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnLoginDlg(void);

protected:
	CEtUIEditBox *m_pEditBoxID;
	CEtUIEditBox *m_pEditBoxPW;

	bool m_bCallbackProcessed;

public:
	void SetId(WCHAR *Id);
	void SetPw(WCHAR *Pw);
	int GetMaxCharID();
	int GetMaxCharPW();

	void EnableLoginButton( bool bEnable );

	
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void Process( float fElapsedTime );

};
