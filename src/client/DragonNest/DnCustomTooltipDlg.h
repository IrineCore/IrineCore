
#pragma once
#include "EtUIDialog.h"
class CEtUITextBox;


// Rotha
// Because creating individual tooltip dialogs for specific situations takes too much time and effort
// Set up a shared tooltip dialog. < Used when simple handling or direct text setting is possible > 

class CDnCustomTooltipDlg : public CEtUIDialog
{
public:
	CDnCustomTooltipDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCustomTooltipDlg(void);
	

protected:
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	CEtUITextBox *GetTooltipTextBox() { return m_pTextBox; }

#ifdef PRE_ADD_WEEKLYEVENT
	void SetWeeklyEventText();
#endif

#ifdef PRE_ADD_ACTIVEMISSION
	void SetActiveMissionText( std::wstring & str );
#endif // PRE_ADD_ACTIVEMISSION

protected:
	CEtUITextBox *m_pTextBox;
};
