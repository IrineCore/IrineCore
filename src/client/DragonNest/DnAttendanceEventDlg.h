#pragma once
#include "DnCustomDlg.h"

class CDnAttendanceEventDlg : public CDnCustomDlg
{
public:
	CDnAttendanceEventDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnAttendanceEventDlg();

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

#if defined(PRE_ADD_INTEGERATE_EVENTUI)
	DWORD GetEventCount() 
	{ 
		DWORD eventCount = m_pAttendanceEventRewardListBox ? m_pAttendanceEventRewardListBox->GetSize() : 0;
		return eventCount;
	}
#else
private:
#endif // PRE_ADD_INTEGERATE_EVENTUI
	void SetAttendanceEventReward();

protected:
	CEtUIListBoxEx* m_pAttendanceEventRewardListBox;
	CEtUIStatic*	m_pStaticCheckAttendanceDay;
	CEtUIStatic*	m_pStaticEventDate;
};


#define MAX_ATTENDANCEEVENT_REWARD_ITEM 5

class CDnMissionRewardCoinTooltipDlg;
class CDnItem;

class CDnAttendanceEventItemDlg : public CDnCustomDlg
{
public:
	CDnAttendanceEventItemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnAttendanceEventItemDlg();

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );

public:
	void SetCheckAttendanceReward( int nAttendanceDate, bool bCheck, bool bRewardHide, int nMailRewardID, int nCashRewardID );

private:
	int SetRewardItem( int nMailRewardID, int nStartIndex, int nStartRewardSlotIndex, bool bCash );

protected:
	int								m_nRewardCoin;
	CDnMissionRewardCoinTooltipDlg*	m_pRewardCoinTooltipDlg;
	CDnItem*						m_pItem[MAX_ATTENDANCEEVENT_REWARD_ITEM];
	bool							m_bShowQuestion;
};

