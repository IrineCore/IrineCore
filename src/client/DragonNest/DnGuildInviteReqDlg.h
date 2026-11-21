#pragma once
#include "EtUIDialog.h"

class CDnGuildInviteReqDlg : public CEtUIDialog
{
public:
	CDnGuildInviteReqDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGuildInviteReqDlg(void);

protected:
	CEtUIProgressBar *m_pProgressBarTime;

	CEtUIStatic *m_pGuildName;
	CEtUIStatic *m_pCharacterName;
	CEtUIButton *m_pButtonOK;		// Accept join
	CEtUIButton *m_pButtonCancel;	// Reject

	float m_fTotalTime;				// Total time
	float m_fElapsedTime;			// Remaining time (usually set equal to above when first Shown)

public:
	void SetInfo( LPCWSTR pwszGuildName, LPCWSTR wszCharacterName, float fTotalTime, int nID = -1, CEtUICallback *pCall = NULL );

	// To adjust the progress bar directly, pass a new fElapsedTime value.
	void SetElapsedTime( float fElapsedTime );

	// Send
	void SendReject();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
};