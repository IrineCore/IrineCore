#pragma once
#include "EtUIDialog.h"

/*
#if defined(PRE_ADD_CH_SNDAAUTH_02)
#else	// #if defined(PRE_ADD_CH_SNDAAUTH_02)
#ifdef PRE_ADD_CH_SNDAAUTH_01
class CDnSecurityECardBDlg : public CEtUIDialog
{
public:
	CDnSecurityECardBDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnSecurityECardBDlg(void);

protected:

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	void SetECardCood(WCHAR *szCood);
	void ResetArray();
	int ExtractNumber(WCHAR *szStr);

std::wstring m_szECardCood; // Stores e-card coordinate position
	std::vector<std::wstring> m_szPosStr; 
	
};
#endif 
#endif	// #if defined(PRE_ADD_CH_SNDAAUTH_02)

*/