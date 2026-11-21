#pragma once

#include "EtUIDialog.h"
#include "DnSmartMoveCursor.h"

class CDnCashShopAboutPetal : public CEtUIDialog
{
public:
	CDnCashShopAboutPetal( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnCashShopAboutPetal(void) {}
	virtual void Initialize(bool bShow);
	virtual void Show( bool bShow );

protected:
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ );

private:
	CEtUIButton* m_pCloseBtn;
	CDnSmartMoveCursor m_SmartMove;
};
