#pragma once
#include "EtUIDialog.h"
#include "EtUIDialogGroup.h"
#include "DnGuildTask.h"

class CDnGuildAuthDlg;
class CDnGuildNoticeDlg;
class CDnGuildMemberAddDlg;
class CDnGuildYesNoDlg;
class CDnGuildInfoDlg;
class CDnGuildMemberPopupDlg;
class CDnGuildMemberInfoDlg;
class CDnGuildRecruitTabDlg;
class CDnGuildDlg : public CEtUIDialog
{
public:
	CDnGuildDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGuildDlg(void);

protected:
	CEtUIStatic *m_pStaticName;
	CEtUITextureControl *m_pTextureGuildMark;
	CEtUIStatic *m_pStaticNotice;
	CEtUIButton *m_pButtonAuth;
	CEtUIButton *m_pButtonNotice;
	CEtUIButton *m_pButtonInvite;
	CEtUIButton *m_pButtonQuit;
	CEtUIButton *m_pButtonInfo;
	CEtUIStatic *m_pStaticGuildMemberCount;

	CEtUIButton *m_pButtonOption;

	// 3-column
	CEtUIListBoxEx *m_pListBoxEx_1;
	CEtUIStatic *m_pBase1_1;
	CEtUIStatic *m_pBase2_1;
	CEtUIStatic *m_pBase3_1;
	CEtUIButton *m_pButton1_1;
	CEtUIButton *m_pButton2_1;
	CEtUIButton *m_pButton3_1;

	// 2-column
	CEtUIListBoxEx *m_pListBoxEx_2;
	CEtUIStatic *m_pBase1_2;
	CEtUIStatic *m_pBase2_2;
	CEtUIButton *m_pButton1_2;
	CEtUIButton *m_pButton2_2;

	CEtUICheckBox *m_pCheckBoxHide;

	CEtUIButton *m_pButtonRecruit;

#ifdef PRE_ADD_GUILD_EASYSYSTEM
#ifdef PRE_MOD_GUILD_EASYSYSTEM_NO_HOMEPAGE
#else
	CEtUIButton *m_pButtonHomepage;
#endif
#endif

	CDnGuildAuthDlg *m_pGuildAuthDlg;
	CDnGuildNoticeDlg *m_pGuildNoticeDlg;
	CDnGuildMemberAddDlg *m_pGuildMemberAddDlg;
	CDnGuildYesNoDlg *m_pGuildYesNoDlg;
	CDnGuildInfoDlg *m_pGuildInfoDlg;
	CDnGuildMemberPopupDlg *m_pGuildMemberPopupDlg;
	CDnGuildMemberInfoDlg *m_pGuildMemberInfoDlg;
	CDnGuildRecruitTabDlg *m_pGuildRecruitTabDlg;
	enum
	{
		GUILD_AUTH_DIALOG,
		GUILD_NOTICE_DIALOG,
		GUILD_MEMBERADD_DIALOG,
		GUILD_YESNO_DIALOG,
		GUILD_INFO_DIALOG,
		GUILD_MEMBERPOPUP_DIALOG,
		GUILD_MEMBERINFO_DIALOG,
		GUILD_RECRUIT_DIALOG,
	};

	enum eListBoxExInfoType
	{
		Level_Job,
		Location,
		Role_LastTime,
		Introduce,

		ListBoxExInfoType_Amount,
	};

	enum eListBoxExType
	{
		SHOW_FALSE,		// Default state has nothing visible.
		THREE_COLUMN,	// Name|Level|Class etc.
		TWO_COLUMN,		// Name|Intro etc.
	};

	void InitControl();
	void InitListBoxExControl( eListBoxExType Type = SHOW_FALSE );
	eListBoxExType m_eCurListBoxExType;
	eListBoxExInfoType m_eCurListBoxInfoType;
	void RefreshGuildMemberList();
	CEtUIListBoxEx *GetListBox( eListBoxExType Type );

	// Store the current value for each sort to allow reverse sorting.
	bool m_bCurrentReverse[CDnGuildTask::GuildMemberSort_Amount];

public:
	void RefreshGuildDlg();
	void OnRecvGetGuildHistoryList( SCGetGuildHistoryList *pPacket );
	void OnRecvGuildRecruitRegisterInfo( GuildRecruitSystem::SCGuildRecruitRegisterInfo *pPacket );
	void OnRecvGuildRecruitCharacter( GuildRecruitSystem::SCGuildRecruitCharacterList *pPacket );
	void OnRecvGuildRecruitAcceptResult( GuildRecruitSystem::SCGuildRecruitAccept *pPacket );

	void GetLogoutText(std::wstring &wszStr, __time64_t LogoutTime);
	bool CheckValidCommand();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );


	CDnGuildInfoDlg* GetGuildInfoDlg() { return m_pGuildInfoDlg; }
};