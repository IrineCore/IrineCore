#pragma once


#ifdef PRE_ADD_DONATION


#include "EtUIDialog.h"

//------------------------------------------------
// Donation system - Gold donation window.

class CDnContributionGoldDlg : public CEtUIDialog
{

private:

	CEtUIStatic * m_pStaticGold;
	CEtUIStatic * m_pStaticSilver;
	CEtUIStatic * m_pStaticCopper;
	CEtUIButton * m_pBtnContribute;

	class CDnMoneyControl * m_pMoneyControl;

public:

	CDnContributionGoldDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnContributionGoldDlg(){
		ReleaseDlg();
	}

	void ReleaseDlg();

	// Overrid - CEtUIDialog.
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void Show( bool bShow );
	
	// Override - CEtUICallback.
	//virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );


private:

	
	
	void MakeDonations(); // Donate.

public:

	void SetData();

};

#endif