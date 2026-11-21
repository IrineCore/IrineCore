#pragma once

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "EtUIDialog.h"

class CDnTooltipTalismanDlg : public CEtUIDialog
{
public:
	CDnTooltipTalismanDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnTooltipTalismanDlg();

	void SetTalismanSlot( int nTableRowID, bool bCashSlot = false );
	void AdjustPosition();

protected:
	CEtUITextBox *m_pTextBox;

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
};

#endif // PRE_ADD_TALISMAN_SYSTEM
