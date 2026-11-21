#pragma once
#include "EtUIDialog.h"
#include "DnSecurityBase.h"


class CDnSecurityNUMPADChangeDlg :	public CEtUIDialog , public CDnSecurityBase
{
public:
	CDnSecurityNUMPADChangeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnSecurityNUMPADChangeDlg(void);

	enum emAuthPwFocus
	{
		OldPw,	// Current password
		NewPw,	// Password
		EnterPw,// Re-enter 
	};


protected:
	std::vector<std::wstring> m_vecNumberStr;
	std::vector<CEtUIButton*> m_pBtnNumPad;

	SecondPassStruct m_SecondPass0;
	SecondPassStruct m_SecondPass1;
	SecondPassStruct m_SecondPass2;

	CEtUIEditBox* m_pEditOldPw;
	CEtUIEditBox* m_pEditNewPw;
	CEtUIEditBox* m_pEditEnterPw;

	emAuthPwFocus  m_eAuthPwFocus;
	
#ifdef PRE_MOD_SECURITYNUMPAD_RENEWAL
	std::vector<std::wstring> m_vecNumberStrCopy;
#endif

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void Show( bool bShow );
	
	
	void InitializeNumber();
	void SetCurrentFocus(emAuthPwFocus AuthPwFocus);
	bool IsCurPwFocus(emAuthPwFocus AuthPwFocus);
	void ShuffleNumber();
	void KeyDelete();

#ifdef PRE_MOD_SECURITYNUMPAD_RENEWAL
	bool CheckAreaSecurityNumber( float fMouseX, float fMouseY );
#endif 
};

