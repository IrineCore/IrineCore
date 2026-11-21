#pragma once
#include "EtUITabDialog.h"

class CEtUIIMEEditBox;
class CEtUIRadioButton;
class CEtUIButton;
class CDnChatDlg;
class CDnChatHelpDlg;
#ifdef PRE_MOD_CHATBG
class CDnChatBoardDlg;
#endif

#ifdef PRE_ADD_WORLD_MSG_RED
class CDnChatTabDlg : public CEtUITabDialog, public CEtUICallback, public CEtUINameLinkInterface
#else // PRE_ADD_WORLD_MSG_RED
class CDnChatTabDlg : public CEtUITabDialog, public CEtUICallback
#endif // PRE_ADD_WORLD_MSG_RED
{
protected:
	enum CHATDLG_SIZE_OLD
	{
		SIZE_STEP_SMALL,
		SIZE_STEP_MIDDLE,
		SIZE_STEP_BIG,
	};

	enum
	{
		CHAT_HELP_DIALOG,
	};

	enum
	{
		CMD_MSG_HELP,
		CMD_MSG_INVITE,
		CMD_MSG_MAKEPARTY,
		CMD_MSG_LEAVEPARTY,
		CMD_MSG_BAN,
		CMD_MSG_TRADE,
		CMD_MSG_DICE,
		CMD_MSG_LOCK,
		CMD_MSG_UNLOCK,
		CMD_MSG_ESCAPE,
		CMD_MSG_RAIDNOTICE,
#ifdef PRE_PRIVATECHAT_CHANNEL
		CMD_MSG_CHANNEL_CREATE,
		CMD_MSG_CHANNEL_JOIN,
		CMD_MSG_CHANNEL_PASSWORD,
		CMD_MSG_CHANNEL_INVITE,
		CMD_MSG_CHANNEL_LEAVE,
		CMD_MSG_CHANNEL_BAN,
		CMD_MSG_CHANNEL_MASTER,
		CMD_MSG_CHANNEL_HELP,
#endif // PRE_PRIVATECHAT_CHANNEL
	};

	enum CHATDLG_SIZE
	{
		SIZE_BASE,	// Default state
		SIZE_OVER,	// Mouse over
		SIZE_DRAG	// State adjustable by pressing
	};

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
public:
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
#ifdef PRE_PRIVATECHAT_CHANNEL
	enum
	{
		CHANNEL_NAME_MIN = 2,
		CHANNEL_NAME_MAX = 10,
	};
#endif // PRE_PRIVATECHAT_CHANNEL

public:
	CDnChatTabDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChatTabDlg(void);

protected:
	CEtUIRadioButton *m_pNormalTab;
#ifdef PRE_ADD_CHAT_RENEWAL
	CEtUIRadioButton * m_pWhisperTab; // Whisper.
#endif
	CEtUIRadioButton *m_pPartyTab;
	CEtUIRadioButton *m_pGuildTab;
	CEtUIRadioButton *m_pSystemTab;

	CEtUIButton *m_pButtonDummy;
	int m_nSizeMode;
	float m_fBasisY;	// y position when drag started

	CDnChatDlg *m_pChatNormalDlg;
#ifdef PRE_ADD_CHAT_RENEWAL
	CDnChatDlg * m_pChatWhisperDlg; // Whisper.
#endif
	CDnChatDlg *m_pChatPartyDlg;
	CDnChatDlg *m_pChatGuildDlg;
	CDnChatDlg *m_pChatSystemDlg;

	CDnChatHelpDlg *m_pChatHelpDlg;
#ifdef PRE_MOD_CHATBG
	CDnChatBoardDlg* m_pChatBoardDlg;
#endif

#ifdef PRE_PRIVATECHAT_CHANNEL
	CEtUIRadioButton* m_pPrivateChannelTab;
	CDnChatDlg* m_pPrivateChannelDlg;
#endif // PRE_PRIVATECHAT_CHANNEL
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	CEtUIStatic* m_pStaticPrivateChannelNoticeText;
	CEtUIStatic* m_pStaticPrivateChannelNoticeBalloon;
	CEtUIStatic* m_pStaticPrivateChannelTabHighlight;
	bool m_bJoinPrivateChannelNotify;
	float m_fJoinPrivateChannelNotify;
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

	typedef std::vector<CDnChatDlg*>	CHATDLG_VEC;
	typedef CHATDLG_VEC::iterator		CHATDLG_VEC_ITER;
	CHATDLG_VEC m_vecChatDlg;

	int m_nChatMode;
#ifdef PRE_ADD_CHAT_RENEWAL	
	bool m_bChangeChatMode;
#endif
	CEtUIButton *m_pButtonModeUp;
	CEtUIButton *m_pButtonModeDown;
	CEtUIButton *m_pButtonOption;
	CEtUIButton *m_pButtonHelp;
	CEtUIStatic *m_pStaticMode;
	CEtUIIMEEditBox *m_pIMEEditBox;
	CEtUIButton *m_pButtonReport;

#ifdef PRE_ADD_WORLD_MSG_RED
	CEtUIStatic* m_pWorldChatRedBoard;
	CEtUITextBox* m_pWorldChatRed;
	float m_fWorldMsgRedCoolTime;
	float m_fWorldMsgRed1Sec;
	int m_nBlinkCount;
#endif // PRE_ADD_WORLD_MSG_RED

	bool m_bCustomizeChatMode;          // Enable/Disable
	int m_nCustomizeModeType;           // Select mode (party mode, normal mode, guild mode, etc.)
	LPCWSTR m_wszCustomizeModeHeader;   // Can change depending on message context such as [Party], [Normal], [Guild], etc.
	DWORD m_dwCustomizeModeTextColor;   // Color supported - currently set to change only one at a time. Use a Vector when changing multiple simultaneously -

	typedef std::map<std::wstring,int>	COMMANDMODE_MAP;
	typedef COMMANDMODE_MAP::iterator	COMMANDMODE_MAP_ITER;
	COMMANDMODE_MAP m_mapCommandMode;

	// Create separately to distinguish from mode.
	typedef std::map<std::wstring,int>		COMMANDMESSAGE_MAP;
	typedef COMMANDMESSAGE_MAP::iterator	COMMANDMESSAGE_MAP_ITER;
	COMMANDMESSAGE_MAP m_mapCommandMessage;

	bool m_bShowOptDlg;

	// Currently unused variable. Remove once stabilized.
	CHATDLG_SIZE_OLD m_ChatDlgSize;

	// It may look strange that m_strLastPrivateUserName and m_listPrivateName are separate, but
	// It was built this way and both are currently used, so just leave it as is.
	//
	// Anyway, when a message arrives, set m_strLastPrivateUserName in interface and call AddPrivateName.
	// m_strLastPrivateUserName is used to remember the last whisper target, and
	// m_listPrivateName is used when switching whisper targets with Tab.
	std::wstring m_strPrivateUserName;
	std::wstring m_strLastPrivateUserName;

	typedef std::list<std::wstring>		PRIVATENAME_LIST;
	typedef PRIVATENAME_LIST::iterator	PRIVATENAME_LIST_ITER;
	PRIVATENAME_LIST m_listPrivateName;

	typedef std::list<std::wstring>		CHAT_HISTORY_LIST;
	typedef CHAT_HISTORY_LIST::iterator	CHAT_HISTORY_LIST_ITER;
	CHAT_HISTORY_LIST m_listChatHistory;

	bool m_bMouseIn;
	bool m_bSetFocusEditBox;
	bool m_IsPVPGameChat; //Whether it is PvP game chat 
	bool m_bPrivateModeChecker;
	bool m_bPVPLobbyChat;

	// Play sound effect if the tab is not active when a whisper arrives.
	int m_nPrivateAlarmSoundIndex;
	std::wstring m_ChatTextWrapper;

	int m_nChatModeBeforePrivate;

	bool m_bIgnoreShowFunc;
#ifdef PRE_ADD_ANTI_CHAT_SPAM
	bool m_bSpammer;
#endif

protected:
	// Note : Message entered through IME edit box
	//
	bool AddEditBoxString();

public:
	int GetChatMode()							{ return m_nChatMode; }
	bool SetChatMode( int nChatMode, bool bParty = false );
	void ResetChatMode();
	int GetChatTabMode()						{ return GetCurrentTabID(); }
	void SetChatTabMode( int nChatTabMode )		{ SetCheckedTab( nChatTabMode ); }

	void CustomizeChatMode(bool bTrue,int nModeType,LPCWSTR wszHeader,DWORD dwTextColor);
	DWORD GetCustomizeChatColor(){return m_dwCustomizeModeTextColor;}
	int GetCustomizeChatMode(){return m_nCustomizeModeType;}
	bool IsCustomizeChatMode(){return m_bCustomizeChatMode;}

	// Prevent closing when blind opens. Works once per call.
	void SetIgnoreShowFunc( bool bIgnore ) { m_bIgnoreShowFunc = bIgnore; }

protected:
	void SetChatModeText();
	void SetChatPrevMode();
	void SetChatNextMode();

	void CheckCommandMode();
	bool CheckCommandMode( LPCWSTR wszMsg );
	void ResizeChatBox( float fHeightDelta );
	void ResizeIMECCtl( LPCWSTR wszHeader );
	void SetPosBottomCtl( float fResizeRatio );
	void SetPosChatDlg( float fResizeRatio );
	void ShowControl( bool bShow );

	bool CommandMessage( LPCWSTR wszMsg );

	void AddChatDlg( LPCWSTR wszMsg, int nChatType );
	void _AddChatMessage( int nChatType, LPCWSTR wszFromCharName, LPCWSTR wszMessage );
	void AddPrivateName( const std::wstring strPrivateName );
	void ChangePrivateName();

	void AddChatHistotry( LPCWSTR wszChat );
	void SetPrevChatHistory();
	void SetNextChatHistory();

	// Prevent repetition
	std::wstring m_wszPrevString;
	UINT m_nRepeat;
	bool CheckRepeat();
	bool CheckRepeat(const std::wstring& chatMsg);

	// No spamming
	float m_fPaperingRemainTime;
	std::list<float> m_listRecentChatTime;
	bool CheckPapering();
#ifdef PRE_CHAT_FIX_DICE_PAPERING
	bool HandleChatPapering(const std::wstring& chatMsg);
#endif // PRE_CHAT_FIX_DICE_PAPERING

	bool IsGuildWarMaster( LPCWSTR wszFromCharName );
	UINT  GetGuildWarUserState( LPCWSTR wszFromCharName );

public:
	// Note : Normal chat message entered by user
	//
#ifdef PRE_ADD_DOORS
	void AddChatMessage( int nChatType, LPCWSTR wszFromCharName, LPCWSTR wszMessage, bool isAppend = false, bool hasColor = false, DWORD colorValue = 0xffffffff, DWORD bgColorValue = 0, bool bIsDoorsMobile = false);
#else
	void AddChatMessage( int nChatType, LPCWSTR wszFromCharName, LPCWSTR wszMessage, bool isAppend = false, bool hasColor = false, DWORD colorValue = 0xffffffff, DWORD bgColorValue = 0);
#endif

	// System messages should sometimes not appear in the center of the screen.
	void AddSystemMessage( LPCWSTR wszFromCharName, LPCWSTR wszMessage, bool bShowCaption = true );

	// Notice messages are always shown at the center of the screen.
	void AddNoticeMessage( LPCWSTR wszFromCharName, LPCWSTR wszMessage );
	void AppendChatEditBox(LPCWSTR wszString, bool bMoveCaret);

	void AddChatQuestMsg( LPCWSTR wszMsg );
	void AddChatQuestReceipt( LPCWSTR wszQuestName );
	void AddChatQuestCompletion( LPCWSTR wszQuestName );

	void ShowEx( bool bShow );
	void ShowChatDlg();
	CDnChatDlg *GetCurrentChatDlg();

	bool GetLastPrivateName( std::wstring &strLastPrivateName );
	void SetLastPrivateName( LPCWSTR wszLastPrivateName );
	void DelPrivateName( const std::wstring &strPrivateName );
	void SetPrivateName( const std::wstring &strPrivateName );
	std::wstring GetPrivateName() { return m_strPrivateUserName; }

	// IMEEditBox
	bool IsEditBoxFocus() { return m_pIMEEditBox->IsFocus(); }
	void SetEditBoxFocus() { RequestFocus(m_pIMEEditBox); }
	CEtUIIMEEditBox *GetIMEEditBox() { return m_pIMEEditBox; }
	bool GetGameMode() {return m_IsPVPGameChat;}
	void ClearText();
	void SendChat(int chatMode, std::wstring& chatMsg, const std::wstring& userName);

#ifdef PRE_MOD_CHATBG
	bool IsBackGroundMode() const;
#endif
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	void UpdatePrivateChannelNotify( float fElapsedTime );
	void SetPrivateChannelNotify( bool bNotify );
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

public:
	virtual void Process( float fElapsedTime );
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Render( float fElapsedTime );
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void SetChatGameMode( bool isPVP );

	virtual void OnChangeResolution();
#ifdef PRE_ADD_WORLD_MSG_RED
	virtual bool OnParseTextItemInfo( const std::wstring& argString );
#endif // PRE_ADD_WORLD_MSG_RED

#if defined(PRE_FIX_57852)
protected:
	bool m_bPvPLobbyChatOtherAreaInside;	//PvP lobby right area mouse handling
#endif // PRE_FIX_57852

};