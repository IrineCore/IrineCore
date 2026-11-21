#pragma once
#include "EtUIDialog.h"

class CEtUIStatic;
class CEtUIEditBox;

class CDnPassWordInputDlg : public CEtUIDialog
{
public:
	CDnPassWordInputDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnPassWordInputDlg(void);

protected:
	CEtUIStatic *m_pStaticPassword;
	CEtUIEditBox *m_pEditBoxPassWord;
	CEtUIButton *m_pButtonJoin;

public:
	LPCWSTR GetPassword();

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
};




#ifdef PRE_MOD_PVPOBSERVER
//---------------------------------------------------------------------------------------
// CDnPassWordInputEventDlg
// : Dlg almost identical to CDnPassWordInputDlg above - slightly wider.
class CDnPassWordInputEventDlg : public CDnPassWordInputDlg
{
public:
	CDnPassWordInputEventDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnPassWordInputEventDlg();

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
};
#endif // PRE_MOD_PVPOBSERVER
