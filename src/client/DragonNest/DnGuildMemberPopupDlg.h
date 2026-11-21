#pragma once
#include "EtUIDialog.h"

class CDnGuildRoleChangeDlg;

class CDnGuildMemberPopupDlg : public CEtUIDialog, public CEtUICallback
{
public:
	CDnGuildMemberPopupDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGuildMemberPopupDlg(void);

	enum
	{
		GUILD_EXILE_DIALOG,
		GUILD_MASTER_DIALOG,
	};

protected:
	CEtUIButton *m_pButtonPrivate;
	CEtUIButton *m_pButtonFriend;
	CEtUIButton *m_pButtonCopyName;
	CEtUIButton *m_pButtonChangeRole;
	CEtUIButton *m_pButtonExile;
	CEtUIButton *m_pButtonMaster;
	CEtUIButton *m_pButtonClose;

	INT64 m_nCharacterDBID;

	CDnGuildRoleChangeDlg *m_pGuildRoleChangeDlg;

protected:
	void InitControl();

public:
	void SetGuildMemberDBID( INT64 nDBID ) { m_nCharacterDBID = nDBID; }

public:
	virtual void InitialUpdate();
	virtual void Initialize(bool bShow);
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg );

private:
	bool CheckFriendCondition(const WCHAR* name);
	bool IsFriend( const WCHAR *name );

	// Some guild menu options are said to be usable only in town.
	bool CheckValidCommand();
};
