#pragma once

#include "DnCustomDlg.h"
#include "DnMissionTask.h"

class CDnItem;
class CDnMissionAppellationTooltipDlg;
class CDnMissionRewardCoinTooltipDlg;
class CDnMissionDailyQuestDlg;

class CDnMissionListItem : public CDnCustomDlg
{
public:
	CDnMissionListItem( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMissionListItem(void);

protected:
	CEtUIStatic *m_pName;
	CEtUIStatic *m_pDescription;
	CEtUIStatic *m_pRewardPoint;
	CEtUIStatic *m_pRewardAppellation;
	CDnMissionIconStatic *m_pMissionIcon;
	CEtUIStatic *m_pStaticFlagIcon[9];
	CEtUIStatic *m_pStaticGrayFlagIcon[9];
	CEtUIStatic *m_pCompleteMark;
	CEtUIStatic *m_pCompleteText;

	CDnItem *m_pItem[CDnMissionTask::RewardItem_Amount];
	CDnItemSlotButton *m_pRewardItem[CDnMissionTask::RewardItem_Amount];
	CEtUIStatic *m_pRewardMoney[CDnMissionTask::RewardItem_Amount];

	CEtUICheckBox *m_pCheckBox;
	CEtUIProgressBar *m_pDailyProgress;
	CEtUIStatic *m_pDailyCount;
	CEtUIStatic *m_pDailyCountBack;

#ifdef PRE_MOD_MISSION_HELPER
	CEtUIStatic *m_pStaticFlag;
#endif

	CDnMissionTask::MissionInfoStruct *m_pInfo;

	// ListBoxEx Item dialog having another child seems like a poor structure.
	// Will probably need to overhaul this during the next mission UI renewal.
	CDnMissionAppellationTooltipDlg *m_pAppellationTooltip;
	CDnMissionRewardCoinTooltipDlg *m_pRewardCoinTooltipDlg;

#ifdef PRE_MOD_MISSION_HELPER
	CEtUIListBoxEx *m_pParentListBoxEx;
#else
	CDnMissionDailyQuestDlg *m_pParentDailyDialog;
#endif

public:
	void SetInfo( CDnMissionTask::MissionInfoStruct *pInfo );
	void SetCheckBox( bool bCheck );
	void ShowCheckBox( bool bShow );
#ifdef PRE_MOD_MISSION_HELPER
	void SetParentListBoxExControl( CEtUIListBoxEx *pControl )  { m_pParentListBoxEx = pControl; }
	bool IsChecked();
	void EnableCheckBox( bool bEnable );
#else
	void SetParentDailyDialog( CDnMissionDailyQuestDlg *pDialog )  { m_pParentDailyDialog = pDialog; }
#endif
	int GetItemID();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );

	virtual void SetElementDialogShowState( bool bShow );
};
