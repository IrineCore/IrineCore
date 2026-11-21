#pragma once
#include "DnCustomDlg.h"
#include "DnDungeonEnterDlg.h"


class CDnDungeonExpectMissionRewardDlg;

class CDnDungeonExpectMissionDlg : public CDnCustomDlg
{
public:
	CDnDungeonExpectMissionDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDungeonExpectMissionDlg();

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );

public:
	int LoadMissionInCurrentMap( int nMapIndex, int nDifficult );

private:
	void SetDailyMission( DNTableFileFormat*  pSox, CDnMissionTask::DailyMissionTypeEnum Type, int nMapIndex, int nDifficult );
	void SetAvailableMission( DNTableFileFormat*  pSox, CDnMissionTask::MissionInfoStruct* pMissionInfo, int nMapIndex, int nDifficult );
	bool CheckAvailableMission( DNTableFileFormat*  pSox, CDnMissionTask::MissionInfoStruct* pMissionInfo, int nMapIndex, int nDifficult ); 

protected:
	enum EventValueTypeEnum		// Partial excerpt brought from DnMissionSystem.h
	{
		// Global Variable
		None,
		PlayerLevel,		// Player level
		PartyCount,			// Number of party members
		MapID,				// Map ID
		MapLevel,			// Map Level
	};

	CEtUIListBoxEx* m_pMissionListBox;
	CDnDungeonExpectMissionRewardDlg*	m_pDnDungeonExpectMissionRewardDlg;
};

class CDnDungeonExpectMissionItemDlg : public CDnCustomDlg
{
public:
	CDnDungeonExpectMissionItemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDungeonExpectMissionItemDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

public:
	void SetMissionInfo( CDnMissionTask::MissionInfoStruct* pInfo );
	CDnMissionTask::MissionInfoStruct* GetMissionInfo() { return m_pInfo; }

protected:
	CEtUIStatic* m_pStaticMissionTitle;
	CEtUIStatic* m_pStaticMainIcon;
	CEtUIStatic* m_pStaticSubIcon;
	CEtUIStatic* m_pStaticMissinoIcon;
	CEtUIStatic* m_pStaticSelect;
	CEtUITextBox* m_pTextBoxDescription;

	CDnMissionTask::MissionInfoStruct*	m_pInfo;
};

class CDnItem;
class CDnMissionAppellationTooltipDlg;
class CDnMissionRewardCoinTooltipDlg;

class CDnDungeonExpectMissionRewardDlg : public CDnCustomDlg
{
public:
	CDnDungeonExpectMissionRewardDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDungeonExpectMissionRewardDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual void Show( bool bShow );

public:
	bool SetMissionInfo( CDnMissionTask::MissionInfoStruct* pInfo );

protected:
	CEtUIStatic*		m_pStaticBoard1;
	CEtUIStatic*		m_pStaticBottom1;
	CEtUIStatic*		m_pStaticBoard2;
	CEtUIStatic*		m_pStaticBottom2;
	CEtUIStatic*		m_pRewardAppellation;
	CDnItem*			m_pItem[CDnMissionTask::RewardItem_Amount];
	CDnItemSlotButton*	m_pRewardItem[CDnMissionTask::RewardItem_Amount];
	CEtUIStatic*		m_pRewardMoney[CDnMissionTask::RewardItem_Amount];

	CDnMissionTask::MissionInfoStruct*	m_pInfo;

	CDnMissionAppellationTooltipDlg*	m_pAppellationTooltip;
	CDnMissionRewardCoinTooltipDlg*		m_pRewardCoinTooltipDlg;
};
