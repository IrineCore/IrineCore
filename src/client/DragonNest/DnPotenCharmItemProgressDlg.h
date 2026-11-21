#pragma once

#include "DnCustomDlg.h"

class CDnPotenCharmItemProgressDlg : public CDnCustomDlg
{
public:
	CDnPotenCharmItemProgressDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPotenCharmItemProgressDlg();

protected:
	CEtUIButton *m_pBtnNext;
	CEtUIButton *m_pBtnStop;
	CEtUIProgressBar *m_pProgressPoten;
	CEtUIStatic *m_pStatic5;
	CEtUIStatic *m_pStaticDouble;
	CEtUIStatic *m_pStaticLight;
	CEtUIStatic *m_pStaticPotenBase;
	CEtUIStatic *m_pStaticCashBase;
	CEtUIStatic *m_pStaticText;
	CEtUITextBox *m_pTextBoxPotenCont;
	CEtUITextBox *m_pTextBoxPoten;

	int m_nSoundFeverStart;
	int m_nSoundFeverLoop;
	int m_nSoundDoubleStart;
	int m_nSoundDoubleLoop;

	bool m_bDoubleFever;
	float m_fGaugePercent;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetPotenGauge( float fPercent, bool bDoubleFever, const WCHAR *wszDesc = NULL );
};
