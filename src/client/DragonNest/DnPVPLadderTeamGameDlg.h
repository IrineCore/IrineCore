#pragma once
#include "DnCustomDlg.h"
#include "DnPVPLadderInviteUserListDlg.h"

class CDnPVPLadderTeamGameDlg : public CDnCustomDlg, public CEtUICallback
{
public:					 
	CDnPVPLadderTeamGameDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPLadderTeamGameDlg(void);
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );

	enum eLadderTeamUI_Common
	{
		None = -1,
		First = 0,
		Second = 1,
		Third  = 2,
		MaxTeamSlot = 3,
	};

	struct sLadderTeamUI
	{
		CEtUIStatic *pStaticCover_Slot;
		CEtUIStatic *pStaticCover_Master;
		CEtUIStatic *pStaticCover_UserInit;
		CEtUIButton *pStaticButton_Invite;
		CEtUIButton *pStaticButton_Leave;

		CEtUIStatic *pStatic_Name;
		CEtUIStatic *pStatic_LadderPoint;
		CEtUIStatic *pStatic_LadderGrade;
		CEtUIStatic *pStatic_JobName;

		CEtUIStatic *pStaticCover_Name;
		CEtUIStatic *pStaticCover_LadderPoint;
		CEtUIStatic *pStaticCover_LadderGrade;
		CEtUIStatic *pStaticCover_JobName;

		INT64 nUserDBID;

		sLadderTeamUI()
			: pStaticCover_Slot(NULL)
			, pStatic_Name(NULL)
			, pStatic_LadderPoint(NULL)
			, pStatic_LadderGrade(NULL)
			, pStatic_JobName(NULL)
			, pStaticCover_Name(NULL)
			, pStaticCover_LadderPoint(NULL)
			, pStaticCover_LadderGrade(NULL)
			, pStaticCover_JobName(NULL)
			, pStaticCover_Master(NULL)
			, pStaticCover_UserInit(NULL)
			, pStaticButton_Invite(NULL)
		    , pStaticButton_Leave(NULL)
			, nUserDBID(0)
		{}
	};

public:
	int FindSlotByUserName(WCHAR *wszName);
	int FindSlotByUserDBID(INT64 nDBID);
	bool IsMasterUser(){return m_bIsMasterUser;}
	bool IsReady();

	void ResetAllSlot();
	void RemoveUserByDBID(INT64 nDBID);
	void InsterLadderUser(LadderSystem::SC_NOTIFY_JOINUSER *pData);
	void RefrehsLadderUser(LadderSystem::SC_REFRESH_USERINFO *pData);
	void SetRoomState(LadderSystem::MatchType::eCode eCode);
	void ShowLadderTeamInfo(eLadderTeamUI_Common eCode , bool bShow );
	void ShowMasterMarkByName(WCHAR *wszName);
	void InitializeLadderTeamUI();
	CDnPVPLadderInviteUserListDlg* GetLadderInviteUserListDlg(){return m_pLadderInviteUserListDlg;}
	
protected:
	CDnPVPLadderInviteUserListDlg *m_pLadderInviteUserListDlg;

	sLadderTeamUI m_sLadderTeamUI[MaxTeamSlot]; // There are 3 slots for another player.
	CEtUIStatic *pStaticCover_SelfMaster;
	int m_nRoomState;
	int m_nCurrentInvitedUserNumber;
	int m_nInviteRoomCount;
	bool m_bIsMasterUser;
};
