#pragma once

#if defined( PRE_ADD_DWC )

#include "DnCustomDlg.h"
#include "DNPacket.h"

class CDnDwcMemberListDlg : public CEtUIDialog
{
public:
	CDnDwcMemberListDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDwcMemberListDlg();

protected:
	CEtUIStatic *m_pStaticTitle;
	CEtUIStatic *m_pStaticName;
	CEtUIStatic *m_pStaticClass;
	CEtUIStatic *m_pStaticVillage;
	CEtUIStatic *m_pStaticLv;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void SetMemberInfo( const TDWCTeamMemberInfo &info );
};

class CDnDwcCreateTeamDlg : public CEtUIDialog
{
public:
	CDnDwcCreateTeamDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDwcCreateTeamDlg();

protected:
	CEtUIIMEEditBox *m_pEditTeamName;
	CEtUIButton *m_pBtnOK;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};

class CDnDwcAddMemberDlg : public CEtUIDialog
{
public:
	CDnDwcAddMemberDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDwcAddMemberDlg();

protected:
	CEtUIIMEEditBox *m_pEditName;
	CEtUIComboBox *m_pComboName;
	CEtUIButton *m_pBtnOK;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};

class CDnDwcInviteReqDlg : public CEtUIDialog
{
public:
	CDnDwcInviteReqDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDwcInviteReqDlg();

protected:
	CEtUIProgressBar *m_pProgressTime;
	CEtUIStatic *m_pStaticTitleName;
	CEtUIStatic *m_pStaticName;
	CEtUIButton *m_pBtnOK;
	CEtUIButton *m_pBtnCancel;

	UINT m_nTeamID;
	UINT m_nFromSessionID;
	float m_fTotalTime;
	float m_fRemainTime;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetInviteInfo( UINT nTeamID, UINT nFromSessionID, const WCHAR *wszTeamName, const WCHAR *wszFromCharName, float fTimeoutSec );
};

class CDnDwcDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	enum {
		MESSAGEBOX_LEAVE_TEAM = 1,
	};

public:
	CDnDwcDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDwcDlg();

protected:
	CEtUIButton *m_pBtnHelp;
	CEtUIListBoxEx *m_pListBoxMembers;
	CEtUIButton *m_pBtnInvite;
	CEtUIButton *m_pBtnQuit;
	CEtUIStatic *m_pStaticTeamName;
	CEtUIStatic *m_pStaticRank;
	CEtUIStatic *m_pStaticResult;
	CEtUIStatic *m_pStaticCount;

	CDnDwcAddMemberDlg *m_pAddMemberDlg;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	void RefreshTeamInfo();
};

#endif // PRE_ADD_DWC
