#pragma once
#include "DnCustomDlg.h"

class CDnPVPLadderInviteUserListDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	
	CDnPVPLadderInviteUserListDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPLadderInviteUserListDlg(void);
	
	virtual void InitialUpdate();
	virtual void Initialize(bool bShow);
 	virtual void Show( bool bShow );
 	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	enum eWaitUser
	{
		TypeLobby = 0,
		TypeFriend = 1,
		TypeGuild = 2,
		RequestTimeLimit_Friend = 60,
		RequestTimeLimit_Guild = 60, // Allowed only once per minute.
	};

	enum Page
	{
		MaxPage = 256,
	};

	enum eLadderInviteUserCommon
	{
		Page_WaitUser = 0,
		Page_Friend = 1,
		Page_Guild = 2,
		MaxSlot = 12,
	};

	struct sUI_LadderUserList
	{
		CDnJobIconStatic *pStatic_JobIcon;
		CEtUIStatic *pStatic_Name;
		CEtUIStatic *pStatic_Level;
		CEtUIStatic *pStaticDummy;
		sUI_LadderUserList()
		{
			pStatic_JobIcon = NULL;
			pStatic_Name = NULL;
			pStatic_Level = NULL;
			pStaticDummy = NULL;
		}
	};

public:

	void InitializeCommonUI();
	void InitializeUserListUI();

	void SendRefreshWaitUserList(UINT Page);
	void ClearWaitUserList(bool bShow = false);
	void SetWaitUserList(PvPCommon::WaitUserList::Repository sUserList , UINT uiMaxPage);
	void SetFriendList(int nPageNumber);
	void SetGuildList(int nPageNumber);
	void SetRefreshUserCount(int nCount){ if(nCount>=0 && nCount < PvPCommon::WaitUserList::Common::MaxPerPage)m_nRefreshUserCount = nCount; }
	int GetCurrentRefreshUserType(){return m_nUserType;}


protected:

	int nSelectedPage;

	int m_nUserType; // Guild, friend, standby list 
	int m_nUserPageIndex;
	int m_nUserMaxPage;
	int m_nRefreshUserCount;
	float m_fRequestGuildTimeLimit;
	float m_fRequestFriendTimeLimit;    // Add delay when sending request.
	float m_fRefreshTime;

	CEtUIStatic *m_pStatic_Select;
	CEtUIStatic *m_pStatic_Page;
	CEtUIButton *m_pButton_Refresh;
	CEtUIButton *m_pButton_Prev;
	CEtUIButton *m_pButton_Next;
	CEtUIRadioButton *m_pButton_WaitUser;
	CEtUIRadioButton *m_pButton_Friend;
	CEtUIRadioButton *m_pButton_Guild;
	CEtUIButton *m_pButton_Invite;
	CEtUIIMEEditBox* m_pEditBoxUserName;

	std::vector<sUI_LadderUserList> vWaitUserSlot;
	
};
