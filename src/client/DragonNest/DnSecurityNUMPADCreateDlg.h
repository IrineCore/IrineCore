#pragma once
#include "EtUIDialog.h"
#include "DnSecurityBase.h"


struct sSecurityNumPad
{
	CEtUIButton *BtnNumPad; 
	int Number; 
	sSecurityNumPad(CEtUIButton *_BtnNumPad , int _Number)
	{
		BtnNumPad = _BtnNumPad ; 
		Number = _Number; 
	}
};


class CDnSecurityNUMPADCreateDlg :	public CEtUIDialog , public CDnSecurityBase
{
public:
	CDnSecurityNUMPADCreateDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnSecurityNUMPADCreateDlg(void);
	
	enum emAuthPwFocus
	{
		NewPw,	// Password
		EnterPw,// Re-enter 
	};
	
protected:
	std::vector<std::wstring> m_vecNumberStr;

	SecondPassStruct m_SecondPass1;
	SecondPassStruct m_SecondPass2;

	std::vector<CEtUIButton*> m_pBtnNumPad;

	CEtUIEditBox* m_pEditCurPw;
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
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void InitializeNumber();
	void SetCurrentFocus(emAuthPwFocus AuthPwFocus);
	bool IsCurPwFocus(emAuthPwFocus AuthPwFocus);
	void ShuffleNumber();
	void KeyDelete();

#ifdef PRE_MOD_SECURITYNUMPAD_RENEWAL
	bool CheckAreaSecurityNumber( float fMouseX, float fMouseY );
#endif 
};


