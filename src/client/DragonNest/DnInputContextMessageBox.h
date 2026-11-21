#pragma once

#include "DnCustomDlg.h"

class CDnInputContextMessageBox : public CDnCustomDlg
{
public:
	CDnInputContextMessageBox( UI_DIALOG_TYPE dialogType = UI_TYPE_MODAL, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnInputContextMessageBox();

protected:
	CEtUIButton *m_pBtnOK;
	CEtUIButton *m_pBtnCancel;
	CEtUIIMEEditBox *m_pEditBox;
	CEtUIStatic *m_pStaticMain;
	CEtUIStatic *m_pStaticSub;

	std::wstring m_strRequiredText;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetPopupTableID( int nPopupTableID );
	void SetContextMessage( const WCHAR *wszMain, const WCHAR *wszSub, const WCHAR *wszRequiredInput );
	bool IsInputMatched() const;
};
