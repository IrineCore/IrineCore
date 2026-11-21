#pragma once
#include "EtUIDialog.h"


class CDnGuildWarPopupDlg : public CEtUIDialog, public CEtUICallback
{
public:
	CDnGuildWarPopupDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGuildWarPopupDlg(void);

protected:

	CEtUIButton *m_pButtonMaster;
	CEtUIButton *m_pButtonPrivate;
	CEtUIButton *m_pButtonFriend;
	std::wstring m_wstTargetName; // Target name
	bool m_bMasterDelegate;
	UINT m_uiSessionID;

protected:
	void InitControl();


public:
	virtual void InitialUpdate();
	virtual void Initialize(bool bShow);
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg );
	void SetTargetName(std::wstring wstName){m_wstTargetName = wstName;}
	void SetMasterDelegate( bool bEnable, UINT uiSessionID )	{ m_bMasterDelegate = bEnable; m_uiSessionID = uiSessionID; }

private:
	bool CheckFriendCondition(const WCHAR* name);
	bool IsFriend( const WCHAR *name );
	// Some guild menu options are said to be usable only in town.
	bool CheckValidCommand();
};
