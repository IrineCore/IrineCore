#pragma once
#include "EtUiDialog.h"

class CDnGameControlOptDlg;

class CDnGameControlComboDlg : public CEtUIDialog
{
public:
	CDnGameControlComboDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGameControlComboDlg(void);

	enum {
		KEYBOARD_MOUSE = 0,
		XBOX360_PAD,
		CUSTOM1_PAD,
	};

protected:

	CEtUIComboBox * m_pComboBox;
	CDnGameControlOptDlg * m_pControlOptDlg;

	bool m_bInitialized;

public:

	void SetControlOptDlg(CDnGameControlOptDlg * dlg)	{ m_pControlOptDlg = dlg; }
	int  GetComboSelectIndex()	{ return m_pComboBox->GetSelectedIndex(); }

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};