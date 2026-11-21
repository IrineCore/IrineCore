#pragma once
#include "EtUITabDialog.h"

#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL

class CDnMissionAchieveCompletListBoxDlg;
class CDnMissionAppellationListBoxDlg;

class CDnMissionAchieveDlg : public CEtUIDialog
{
public:
	CDnMissionAchieveDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMissionAchieveDlg(void);

	enum
	{
		NUM_ONOFFMISSION = 5,
	};

protected:
	CEtUIStatic *m_pTotalPoint;
	CEtUIProgressBar *m_pTotalPointProgress;
	CEtUIStatic *m_pTotalAchieveCount;

	CEtUIStatic *m_pOnOffAchieveCount[NUM_ONOFFMISSION];
	CEtUIStatic *m_pOnOffAchievePoint[NUM_ONOFFMISSION];
	CEtUIProgressBar *m_pOnOffAchievePointProgress[NUM_ONOFFMISSION];

	CEtUIStatic *m_pDailyQuestCount;
	CEtUIStatic *m_pDailyQuestPoint;
	CEtUIProgressBar *m_pDailyQuestPointProgress;

	// Radio button
	CEtUIRadioButton* m_pCompleteTitleBtn;
	CEtUIRadioButton* m_pAppellationBtn;
	CEtUIRadioButton* m_pEventBtn;

	// List boxes
	CDnMissionAchieveCompletListBoxDlg* m_pCompleteListBox;
	CDnMissionAppellationListBoxDlg*	m_pAppellationListBox;


public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void ResetListBoxs(bool bShow);
	void RefreshInfo();

};


#else



class CDnMissionAchieveDlg : public CEtUIDialog
{
public:
	CDnMissionAchieveDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMissionAchieveDlg(void);

	enum
	{
		NUM_ONOFFMISSION = 5,
	};

protected:
	CEtUIStatic *m_pTotalPoint;
	CEtUIProgressBar *m_pTotalPointProgress;
	CEtUIStatic *m_pTotalAchieveCount;

	CEtUIStatic *m_pOnOffAchieveCount[NUM_ONOFFMISSION];
	CEtUIStatic *m_pOnOffAchievePoint[NUM_ONOFFMISSION];
	CEtUIProgressBar *m_pOnOffAchievePointProgress[NUM_ONOFFMISSION];

	CEtUIStatic *m_pDailyQuestCount;
	CEtUIStatic *m_pDailyQuestPoint;
	CEtUIProgressBar *m_pDailyQuestPointProgress;

	CEtUIListBoxEx *m_pListBoxEx;

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void RefreshInfo();

};
#endif
