#pragma once
#include "EtUIDialog.h"
#include "DnPVPPopupDlg.h"

class CDnPVPObserverDlg : public CEtUIDialog
{
public:
	CDnPVPObserverDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPObserverDlg(void);

protected:
	
	CEtUIListBoxEx *m_pObserverListBox; 
	CDnPVPPopupDlg *m_pPVPPopupDlg;

public:

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void Show( bool bShow );
	
	void ShowPVPPopUpDlg(bool bShow){ShowChildDialog( m_pPVPPopupDlg, bShow );}

	CEtUIListBoxEx *GetObserverListBox(){return m_pObserverListBox;};// Improved during renewal so this class holds list info. Ultimately this is passed along.

};
