#pragma once

#include "DnCustomDlg.h"

class CDnSubGaugeDlg : public CDnCustomDlg
{
public:
	CDnSubGaugeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_BOTTOM, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSubGaugeDlg();

protected:
	CEtUIProgressBar *m_pProgressGauge;
	CEtUIStatic *m_pStaticText;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void SetGauge( float fPercent, const WCHAR *wszLabel = NULL );
};
