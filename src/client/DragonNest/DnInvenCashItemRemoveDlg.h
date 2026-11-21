#include "DnCustomDlg.h"


#ifdef PRE_ADD_CASHREMOVE

class CDnInvenCashItemRemoveDlg : public CDnCustomDlg
{

private:
	
	CDnItemSlotButton * m_pSlotButton;

public:

	// Remove, restore.
	enum DLGTYPE
	{
		DLGTYPE_REMOVE,
		DLGTYPE_RESTORE
	};
	DLGTYPE m_DlgType;

public:


	CDnInvenCashItemRemoveDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnInvenCashItemRemoveDlg(){

	}


	virtual void Show( bool bShow );


	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	//virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	//virtual void Render( float fElapsedTime );
	//virtual void Process( float fElapsedTime );


private:

	void RestoreCashItemAccept(); // Accept item restoration.	

public:	

	// Set Dlg type.
	void SetDlgType( DLGTYPE type );

	void SetItemSlot( MIInventoryItem * pItem );

};


#endif