#pragma once
#include "EtUIDialog.h"

#ifdef _USE_VOICECHAT
class CDnVoiceChatPremiumOptDlg : public CEtUIDialog
{
public:
	CDnVoiceChatPremiumOptDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnVoiceChatPremiumOptDlg(void);

protected:
	CEtUICheckBox *m_pCheckSurround;
	CEtUIComboBox *m_pComboVoiceFont;

	CEtUIButton *m_pButtonTest;
	CEtUIStatic *m_pStaticTest;
	float m_fTestTime;
	int m_nTestMode;

	CEtUIButton *m_pButtonDown;
	CEtUIButton *m_pButtonUp;
	CEtUISlider *m_pSliderPitch;
	CEtUIStatic *m_pStaticPitch;

	CEtUIButton *m_pButtonDown2;
	CEtUIButton *m_pButtonUp2;
	CEtUISlider *m_pSliderTimbre;
	CEtUIStatic *m_pStaticTimbre;

	void CheckEnableControl();

	// Voice chat voice modulation custom values Pitch and Timbre have a valid range of 0.5~2.0.
	// Convert and set this as a slider value.
	int ToSliderValue( float fCustomValue );
	float ToCustomValue( int nSliderValue );

public:
	void DisableApplyButton();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );

	static void test_finished_callback(void *rock);
};
#endif