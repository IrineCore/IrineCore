#pragma once
#include "EtUIDialog.h"


#ifdef PRE_PRIVATECHAT_CHANNEL

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
class CDnPrivateChannelCreatePWDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelCreatePWDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelCreatePWDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

protected:
	CEtUIIMEEditBox*	m_pIMEEditBox;
	CEtUIEditBox*		m_pEditBox;
};

class CDnPrivateChannelJoinDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelJoinDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelJoinDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

protected:
	CEtUIIMEEditBox*	m_pIMEEditBox;
};

// Channel password input window
class CDnPrivateChannelPasswordDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelPasswordDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelPasswordDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

public:
	void SetChannelName( std::wstring strChannelName ) { m_strChannelName = strChannelName; }

protected:
	CEtUIEditBox*			m_pEditBox;
	std::wstring			m_strChannelName;
};

// Channel password setup window
class CDnPrivateChannelPasswordChangeDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelPasswordChangeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelPasswordChangeDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

protected:
	CEtUIEditBox*			m_pEditBox;
};

#else // PRE_ADD_PRIVATECHAT_CHANNEL

// Channel creation / join message box
class CDnPrivateChannelMessageBoxDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelMessageBoxDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelMessageBoxDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

public:
	enum eChannelJoinType
	{
		TYPE_CHANNEL_NONE = -1,
		TYPE_CHANNEL_CREATE = 0,
		TYPE_CHANNEL_JOIN,
	};

	void SetChannelJoinType( eChannelJoinType channelJoinType ) { m_eChannelJoinType = channelJoinType; }

protected:
	CEtUIIMEEditBox*	m_pIMEEditBox;
	eChannelJoinType	m_eChannelJoinType;
};

// Channel password setup window
class CDnPrivateChannelPasswordDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelPasswordDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelPasswordDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

public:
	enum eChannelPasswordType
	{
		TYPE_PASSWORD_NONE = -1,
		TYPE_PASSWORD_JOIN = 0,
		TYPE_PASSWORD_CHANGE,
	};

	void SetChannelName( std::wstring strChannelName ) { m_strChannelName = strChannelName; }
	void SetChannelPasswordType( eChannelPasswordType channelPasswordType ) { m_eChannelPasswordType = channelPasswordType; }

protected:
	CEtUIEditBox*			m_pEditBox;
	eChannelPasswordType	m_eChannelPasswordType;
	std::wstring			m_strChannelName;
};
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

// Channel tab creation window
class CDnPrivateChannelCreateDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelCreateDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelCreateDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	bool IsShowChannelCreateJoinDlg();
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

protected:
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	CDnPrivateChannelCreatePWDlg*	m_pDnPrivateChannelCreatePWDlg;
	CDnPrivateChannelJoinDlg*		m_pDnPrivateChannelJoinDlg;
#else // PRE_ADD_PRIVATECHAT_CHANNEL
	CDnPrivateChannelMessageBoxDlg*	m_pDnPrivateChannelMessageBoxDlg;
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
};

// Channel participant popup
class CDnChannelEntryPopupDlg : public CEtUIDialog, public CEtUICallback
{
public:
	CDnChannelEntryPopupDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChannelEntryPopupDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg );

public:
	void SetChannelMemberDBID( INT64 nDBID ) { m_nCharacterDBID = nDBID; }

protected:
	INT64 m_nCharacterDBID;
};

// Channel participant info
class CDnChannelEntryInfoDlg : public CEtUIDialog
{
public:
	CDnChannelEntryInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChannelEntryInfoDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

public:
	void SetChannelEntryInfo( PrivateChatChannel::TMemberInfo& channelMemeberInfo );
	INT64 GetMemberDBID() { return m_biMemberDBID; }
	std::wstring& GetMemberName() { return m_strMemberName; }

private:
	INT64			m_biMemberDBID;
	std::wstring	m_strMemberName;
	bool			m_bMaster;
};

// Channel tab info window
class CDnPrivateChannelInfoDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelInfoDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	void RefreshChannelEntryList();

protected:
	CDnChannelEntryPopupDlg*	m_pDnChannelEntryPopupDlg;
	CEtUIListBoxEx*				m_pChannelEntryListBox;
};

// Channel invite window
class CDnPrivateChannelInviteDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelInviteDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelInviteDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

public:
	void SetFriendComboBox();
	void OnAddEditBoxString( const std::wstring& strName );

protected:
	CEtUIIMEEditBox*	m_pEditBoxName;
	CEtUIComboBox*		m_pFriendComboBox;
	std::multimap<wchar_t, std::wstring>	m_FriendNameList;
	std::wstring	m_AutoCompleteTextCache;
};

// Channel window
class CDnPrivateChannelDlg : public CEtUIDialog
{
public:
	CDnPrivateChannelDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPrivateChannelDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

public:
	void RefreshChannelInfo();
	void ShowChannelCreateMode();
	void ShowChannelJoinMode();
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	bool IsShowChannelInputDlg();
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

protected:
	CDnPrivateChannelCreateDlg*	m_pDnPrivateChannelCreateDlg;
	CDnPrivateChannelInfoDlg*	m_pDnPrivateChannelInfoDlg;
	
	CEtUIButton*	m_pButtonChannelInvite;
	CEtUIButton*	m_pButtonChannelPassword;
	CEtUIButton*	m_pButtonChannelLeave;
};

#endif // PRE_PRIVATECHAT_CHANNEL