#pragma once

#include "DnCustomDlg.h"

class CDnItem;

class CDnItemDisjointExpectItemInfoDlg : public CDnCustomDlg
{
public:
	enum {
		EXPECT_SLOT_PER_ROW = 5,
	};

public:
	CDnItemDisjointExpectItemInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemDisjointExpectItemInfoDlg();

protected:
	CDnItem *m_pExpectItems[EXPECT_SLOT_PER_ROW];
	CDnItemSlotButton *m_pSlotButtons[EXPECT_SLOT_PER_ROW];

	void ClearItems();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void SetExpectItems( const std::vector<int> &vecItemIDs, int nStartIndex );
};
