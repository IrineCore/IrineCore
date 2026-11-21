#pragma once
#include "DnOptionDlg.h"

class CDnGameControlComboDlg;

class CDnGameControlMouseDlg : public CDnOptionDlg
{
public:
	CDnGameControlMouseDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGameControlMouseDlg();

private:
	typedef CDnOptionDlg BaseClass;

	CEtUICheckBox *m_pCheckSmartMove;
#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	CEtUICheckBox *m_pCheckSmartMoveEx;
#endif

	CEtUIButton *m_pButtonDown;
	CEtUIButton *m_pButtonUp;
	CEtUISlider *m_pSliderMouse;
	CEtUIStatic *m_pStaticMouse;

	CEtUIButton *m_pButtonDownCamera;
	CEtUIButton *m_pButtonUpCamera;
	CEtUISlider *m_pSliderCamera;
	CEtUIStatic *m_pStaticCamera;

	CEtUIButton *m_pButtonDownCursor;
	CEtUIButton *m_pButtonUpCursor;
	CEtUISlider *m_pSliderCursor;
	CEtUIStatic *m_pStaticCursor;


	CDnGameControlComboDlg * m_pGameControlComboDlg;

public:
	void Initialize( bool bShow ) override;

private:
	void InitialUpdate() override;
	void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg ) override;
	void Show( bool bShow ) override;

public:
	void ImportSetting() override;
	void ExportSetting() override;
	bool IsChanged() override;

	void ResetDefault();

	virtual void ProcessCombo(const int index);
	void GetComboDialog(CDnGameControlComboDlg * pDialog);
};