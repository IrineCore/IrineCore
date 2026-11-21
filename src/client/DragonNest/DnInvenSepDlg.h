#pragma once

#include "EtUIDialog.h"

class CEtUIEditBox;

class CDnInvenSepDlg : public CEtUIDialog
{
public:
	CDnInvenSepDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnInvenSepDlg(void);

protected:
	CEtUIEditBox *m_pEditBox;
	int m_nEditValue;
	int m_nMaxValue;

	CEtUIButton *m_pButtonUp;
	CEtUIButton *m_pButtonDown;

	// Smart move test. Uses only fX and fY.
	SUICoord m_uiSmartMovePos;
	SUICoord m_uiPrePos;

public:
	void SetEditValue( int nValue, int nMaxValue );
	int GetEditValue();

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};
