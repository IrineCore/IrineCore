#ifdef PRE_ADD_CASHSHOP_ACTOZ

#include "DnCustomDlg.h"


// #72888 Actoz migration - Cash Shop UI change.
class CDnCashShopConfirmRefundDlg : public CDnCustomDlg
{

private:

	CDnItemSlotButton * m_pSlotBtn;
	CEtUIStatic * m_pStaticName;


public:


	CDnCashShopConfirmRefundDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnCashShopConfirmRefundDlg();


	// Override - CEtUIDialog //
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();		
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );


private:

	// Request move to cash inventory.
	void MoveCashInventory();

public:

	// Confirmation dialog before moving to cash inventory.
	void SetItem( CDnItem * pItem );

};



#endif // PRE_ADD_CASHSHOP_ACTOZ