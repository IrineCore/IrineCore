#pragma once

#include "DnCustomDlg.h"

class CDnItem;

class CDnSecretCashShopAlarmButtonDlg : public CDnCustomDlg
{
public:
	CDnSecretCashShopAlarmButtonDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSecretCashShopAlarmButtonDlg();

protected:
	CEtUIButton *m_pBtnSecret;
	CEtUIStatic *m_pStaticHidden;
	CEtUITextureControl *m_pTextureFace;
	CEtUIStatic *m_pStaticTextHidden;
	EtTextureHandle m_hFaceTexture;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetAlarmInfo( const WCHAR *wszText, const char *szFaceTexture );
};

class CDnSecretCashShopMessageDlg : public CDnCustomDlg
{
public:
	CDnSecretCashShopMessageDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_MODAL, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSecretCashShopMessageDlg();

protected:
	CEtUITextBox *m_pTextBoxAsk;
	CEtUITextureControl *m_pTextureNpc;
	EtTextureHandle m_hNpcTexture;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetMessage( const WCHAR *wszMessage, const char *szNpcTexture = NULL );
};

class CDnSecretCashShopBuyDlg : public CDnCustomDlg
{
public:
	enum {
		SLOT_PER_PAGE = 2,
	};

	struct SSecretShopItem {
		int nSN;
		int nItemID;
		int nCount;
		int nPeriod;
		int nPrice;
		int nDiscountRate;
		SSecretShopItem() : nSN( 0 ), nItemID( 0 ), nCount( 1 ), nPeriod( 0 ), nPrice( 0 ), nDiscountRate( 0 ) {}
	};

public:
	CDnSecretCashShopBuyDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_MODAL, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSecretCashShopBuyDlg();

protected:
	CEtUIButton *m_pBtnPrev;
	CEtUIButton *m_pBtnNext;
	CEtUICheckBox *m_pCheckCash;
	CEtUICheckBox *m_pCheckCredit;
	CEtUICheckBox *m_pCheckReserve;

	CDnItemSlotButton *m_pSlotItems[SLOT_PER_PAGE];
	CEtUIStatic *m_pStaticSelect[SLOT_PER_PAGE];
	CEtUITextBox *m_pTextBoxPrice[SLOT_PER_PAGE];
	CEtUIStatic *m_pStaticDscRate[SLOT_PER_PAGE];
	CEtUIStatic *m_pStaticItemCount[SLOT_PER_PAGE];
	CEtUIStatic *m_pStaticItemName[SLOT_PER_PAGE];
	CEtUIStatic *m_pStaticItemPeriod[SLOT_PER_PAGE];

	CEtUITextBox *m_pTextBoxAsk;
	CEtUITextureControl *m_pTextureNpc;
	CEtUIStatic *m_pStaticCash;
	CEtUIStatic *m_pStaticReserve;
	CEtUIStatic *m_pStaticPage;
	CEtUIStatic *m_pStaticInfo;
	CEtUIStatic *m_pStaticSelectedName;
	CEtUIStatic *m_pStaticSelectedCount;
	CEtUIStatic *m_pStaticSelectedPeriod;

	CDnItem *m_pPreviewItems[SLOT_PER_PAGE];
	std::vector<SSecretShopItem> m_vecItems;
	int m_nCurPage;
	int m_nMaxPage;
	int m_nSelectedIdx;

	void ClearPreviewItems();
	void RefreshPage();
	void SelectSlot( int nSlotIdx );
	void UpdateSelectedInfo();
	void UpdateBalanceDisplay();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetSecretItems( const std::vector<SSecretShopItem> &vecItems );
	size_t GetSecretItemCount() const { return m_vecItems.size(); }
};
