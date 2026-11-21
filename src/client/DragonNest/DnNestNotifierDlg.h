#pragma once

#include "DnCustomDlg.h"

class CDnNestNotifierListItemDlg : public CDnCustomDlg
{
public:
	CDnNestNotifierListItemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnNestNotifierListItemDlg();

protected:
	CEtUIStatic *m_pStaticName;
	CEtUIStatic *m_pStaticCount;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void SetNestInfo( const WCHAR *wszNestName, int nRemainCount, int nMaxCount );
};

class CDnNestNotifierDlg : public CDnCustomDlg
{
public:
	CDnNestNotifierDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnNestNotifierDlg();

protected:
	CEtUIListBoxEx *m_pListBoxEx;
	CEtUIStatic *m_pStaticNotice;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void ClearNestList();
	void RefreshNestList();
	void AddNestItem( const WCHAR *wszNestName, int nRemainCount, int nMaxCount );
	void SetNoticeText( const WCHAR *wszNotice );
};
