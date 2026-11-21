#pragma once

#include "DnCustomDlg.h"

class CDnPremiumPcBangNotifyToolTipDlg : public CDnCustomDlg
{
public:
	CDnPremiumPcBangNotifyToolTipDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_MOST_TOP, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPremiumPcBangNotifyToolTipDlg();

protected:
	CEtUITextBox *m_pTextBox;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void SetNotifyText( const WCHAR *wszText );
};
