#pragma once

#include "EtUIDialog.h"

#ifndef PRE_MOD_SELECT_CHAR

class CDnCharSelectArrowRightDlg : public CEtUIDialog
{
public:
	CDnCharSelectArrowRightDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnCharSelectArrowRightDlg(void);

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};

#endif // PRE_MOD_SELECT_CHAR
