#pragma once

#include "DnCustomDlg.h"

class CDnMetaMissionTooltipDlg : public CDnCustomDlg
{
public:
	CDnMetaMissionTooltipDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_MOST_TOP, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMetaMissionTooltipDlg();

protected:
	CEtUIStatic *m_pStaticName;
	CEtUIStatic *m_pStaticBase;
	CEtUIStatic *m_pStaticEquip;
	CEtUIStatic *m_pStaticLine;
	CEtUIStatic *m_pStaticLineV2;
	CEtUIStatic *m_pStaticLineV3;
	CEtUIStatic *m_pStaticPointBox;
	CEtUITextBox *m_pTextBox;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void SetMetaMissionInfo( const WCHAR *wszTitle, const WCHAR *wszDesc );
};
