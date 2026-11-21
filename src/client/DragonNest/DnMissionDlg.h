#pragma once

#include "EtUITabDialog.h"
#include "DnSmartMoveCursor.h"

class CEtUIRadioButton;
class CDnMissionAchieveDlg;
class CDnMissionOnOffDlg;
class CDnMissionDailyQuestDlg;
class CDnMissionGuildCommonDlg;

#ifdef PRE_MOD_MISSION_HELPER
struct MissionInfoStruct;
#endif

class CDnMissionDlg : public CEtUITabDialog 
{
public:
	CDnMissionDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMissionDlg();

	enum TabEnum {		
		Achieve = 1,
		Normal = 2,
		Dungeon = 3,
#ifdef PRE_ADD_MISSION_NEST_TAB
		Nest = 4,
#endif 
		Battle = 5,
		DailyQuest = 6,
		Guild = 7,
		Special = 8,
	};

protected:
	CDnMissionAchieveDlg *m_pAchieveDlg;
#ifdef PRE_ADD_MISSION_NEST_TAB
	CDnMissionOnOffDlg *m_pOnOfflDlg[5];
#else
	CDnMissionOnOffDlg *m_pOnOfflDlg[3];
#endif
	CDnMissionDailyQuestDlg *m_pDailyQuestDlg;
	CDnMissionGuildCommonDlg *m_pGuildMissionDlg;
	CDnSmartMoveCursorEx m_SmartMoveEx;

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );

	void RefreshInfo();

	CDnMissionDailyQuestDlg* GetDailyDlg() { return m_pDailyQuestDlg; }
	CDnMissionGuildCommonDlg *GetGuildMissionDlg() { return m_pGuildMissionDlg; }

#ifdef PRE_MOD_MISSION_HELPER
	void SelectMissionItemFromNotifier( MissionInfoStruct *pStruct );
#endif
};