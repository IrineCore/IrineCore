#pragma once
#include "EtUIDialog.h"

class CEtUIStatic;
class CEtUIProgressBar;

// Since only one of the Accept/Decline dialog or Request Cancel dialog can be open at a time, a single dialog is shared like this.
class CDnAcceptRequestDlg : public CEtUIDialog
{
public:
	enum eAcceptRequestType
	{
		eType01,
		eType02,
		eTypeMax
	};
	CDnAcceptRequestDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnAcceptRequestDlg(void);

protected:
	CEtUIHtmlTextBox* m_pMainText[eTypeMax];
	CEtUIProgressBar *m_pProgressBarTime;

	CEtUIButton *m_pButtonOK;		// Accept
	CEtUIButton *m_pButtonReject;	// Reject
	CEtUIButton *m_pButtonCancel;	// (Request) Cancel

	float m_fTotalTime;				// Total time
	float m_fElapsedTime;			// Remaining time (usually set equal to above when first Shown)

	bool m_bAccept;

public:
	// If bAccept is true, shows accept/decline dialog; if false, shows request-cancel dialog.
	void SetInfo( LPCWSTR pwszMessage, float fTotalTime, bool bAccept, eAcceptRequestType type, int nID = -1, CEtUICallback *pCall = NULL );

	// To adjust the progress bar directly, pass a new fElapsedTime value.
	void SetElapsedTime( float fElapsedTime );

	//bool IsEnd();
	void OnTimeIsUp();

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
};
