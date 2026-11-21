#pragma once

#ifdef PRE_ADD_CHAOSCUBE

#include "DnCustomDlg.h"

class CDnItem;

class CDnChaosGaugeCubeListItemDlg : public CDnCustomDlg
{
public:
	CDnChaosGaugeCubeListItemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChaosGaugeCubeListItemDlg();

protected:
	CDnItem *m_pItem;
	CDnItemSlotButton *m_pItemSlotButton;
	CEtUIStatic *m_pStaticName;
	CEtUIStatic *m_pStaticCount;

	int m_nItemID;
	int m_nOwnedCount;
	int m_nGaugeValue;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void InitCustomControl( CEtUIControl *pControl );

	void SetInfo( int nItemID, int nOwnedCount, int nGaugeValue );
	int GetItemID() const { return m_nItemID; }
	int GetOwnedCount() const { return m_nOwnedCount; }
	int GetGaugeValue() const { return m_nGaugeValue; }
	CDnItem *GetItem() const { return m_pItem; }
};

#endif // PRE_ADD_CHAOSCUBE
