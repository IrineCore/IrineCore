#pragma once
#include "DnOptionDlg.h"
#include "GameOption.h"

class CDnGameMacroMsgOptDlg : public CDnOptionDlg, public CEtUICallback
{
public:
	CDnGameMacroMsgOptDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL);
	virtual ~CDnGameMacroMsgOptDlg();

private:
	typedef CDnOptionDlg BaseClass;

protected:
	CEtUIIMEEditBox *m_pEditBox[ CGameOption::NUM_COMM_MACROS ];
	int m_nDefaultMacroUIStringID[ CGameOption::NUM_COMM_MACROS ];

	// Temporary variable for return
	WCHAR m_wszTemp[MAX_PATH];

public:
	// Because character limits in the macro UI file may differ by region,
	// And because default macros cannot be loaded in GameOption, load them here, among other reasons
	// When getting actual macro data, retrieve it from the macro option dialog.
	LPCWSTR GetMacroString( int nIndex );	// Range 0~3

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	virtual void ExportSetting();
	virtual void ImportSetting();
	virtual bool IsChanged();
	void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 ) override;

	void ResetDefault();	// Restore default value
};
