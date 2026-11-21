#pragma once

#include "DnCustomDlg.h"

class CDnItem;

class CDnItemDisjointResultListItemDlg : public CDnCustomDlg
{
public:
	CDnItemDisjointResultListItemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemDisjointResultListItemDlg();

protected:
	CDnItem *m_pItem;
	CDnItemSlotButton *m_pSlotButton;
	CEtUIStatic *m_pStaticName;
	CEtUIStatic *m_pStaticCount;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void SetResultItem( int nItemID, int nCount );
	void SetResultItem( const TItem &item );
};

class CDnItemDisjointResultDlg : public CDnCustomDlg
{
public:
	CDnItemDisjointResultDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemDisjointResultDlg();

protected:
	CEtUIListBoxEx *m_pListBoxEx;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void ClearResultList();
	void AddResultItem( int nItemID, int nCount );
	void SetResultItems( const std::vector<TItem> &vecItems );
};
