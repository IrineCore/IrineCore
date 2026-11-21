#pragma once
#include "DnCustomDlg.h"
#include "DnItem.h"

class CDnItem;
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
class CDnRepStoreBenefitTooltipDlg;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

#ifdef PRE_ADD_DIRECT_BUY_UPGRADEITEM
class CDnMarketDirectBuyDlg;
#endif // #ifdef PRE_ADD_DIRECT_BUY_UPGRADEITEM

class CDnItemUpgradeDlg : public CDnCustomDlg, public CEtUICallback
{
	enum
	{
		ITEM_UPGRADE_CONFIRM_DIALOG,
		ITEM_UPGRADE_MOVIE_DIALOG,
		ITEMSLOT_MAX = 5,
		EQUIPPED_ITEM = 7, 
		ENCHANTGUARD_ITEMID = 1107302400,
		ENCHANTSHIELD_ITEMID = 1107386368,	// Default is China.
	};

public:
	CDnItemUpgradeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemUpgradeDlg(void);

protected:
	// Enhancement item slot
	CDnItem *m_pUpgradeItem;
	CDnItemSlotButton *m_pUpgradeItemSlot;
	CDnQuickSlotButton *m_pQuickSlotButton;
#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
	CDnSlotButton*	   m_pControlSlotButton;	// Applies not only to quickslot (inventory) buttons but also to equipment slots, so switch to that control. 
	CDnQuickSlotButton *m_pPreInvenSlotButton;
	CDnQuickSlotButton *m_pCurrentInvenSlotButton;
#endif 
	eItemRank m_emUpgradeItemRank;

	// Equipped enhanced item slot
#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
	std::vector< CDnItemSlotButton* > m_pVecEquippedUpgradeItemSlot;
#endif 

	// Material item slot
	CDnItem *m_pItem[ITEMSLOT_MAX];
	CDnItemSlotButton *m_pItemSlotButton[ITEMSLOT_MAX];
#ifdef PRE_ADD_DIRECT_BUY_UPGRADEITEM 
	CEtUIButton *m_pUpgradeItemButton[ITEMSLOT_MAX];
	bool		 m_bIsMiniMarket;
#endif
	CEtUIButton *m_pSmallHelpButton;

	CEtUIStatic *m_pStaticGold;
	CEtUIStatic *m_pStaticSilver;
	CEtUIStatic *m_pStaticBronze;
	DWORD m_dwColorGold;
	DWORD m_dwColorSilver;
	DWORD m_dwColorBronze;

	CEtUIButton *m_pButtonOK;
	CEtUIButton *m_pButtonCancel;

	CDnItem *m_pGuardItem;
	CDnItemSlotButton *m_pGuardItemSlotButton;
#if defined(PRE_ADD_ENCHANTSHIELD_CASHITEM)
	CEtUIComboBox* m_pComboBoxGuard;	
	int m_nCurShieldCount;
	int m_nCurGuardCount;
	int m_nTotalShieldNees;
	int m_nGuardItemID;			// Protection jelly item ID
	int m_nShiledItemID;		// Advanced protection jelly item ID
	char m_cEnchantGuardType;	// Various protection jelly types eEnchantGuardType
#else
	CEtUICheckBox *m_pCheckGuard;	
	CEtUIStatic *m_pStaticTextBase;
	CEtUIStatic *m_pStaticTextUse;
	CEtUIStatic *m_pStaticTextNotUse; // Unusable
#ifdef PRE_ADD_JELLYCOUNT
	CEtUIStatic *m_pStaticTextNotUse2;// Unusable (Usable)
#endif // PRE_ADD_JELLYCOUNT
#endif	
	CEtUIStatic *m_pStaticTextCount;
	CEtUIButton *m_pButtonDetail;
	CEtUIStatic *m_pStaticGuard;
	int m_nTotalGuardNeeds;

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CEtUIStatic* m_pReputationBenefit;
	CDnRepStoreBenefitTooltipDlg* m_pStoreBenefitTooltip;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CEtUIStatic *m_pStaticNextUpgradeState;

	CDnItem *m_pNextUpgradeItem;
	bool m_bShowNextUpgradeTooltip;
	int m_nSlotIndex;
#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
	bool m_bEnchantEquippedItem;
#endif

	CEtUIStatic *m_pStaticTitle;
	std::wstring m_wszTitle;
	int m_nRemoteItemID;

#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
public:
	void CheckUpgradeItem();
#else
	// Check enhancement slot
	void CheckUpgradeItem();
#endif

	// Enchant table ID to use currently. More specific EnchantTable ItemID than the TableID in DnItem..
	// Value calculated using DnItem's TableID together with its EnchantLevel.
	int GetEnchantTableItemID( CDnItem *pItem ) const;

	// Gold required for enhancement
	int GetEnchantPrice( int nEnchantTableItemID );

	// Set required items. Return true if all counts are met.
	bool CheckNeedItemInfo( int nEnchantTableItemID );
	void SetItemSlot( int nSlotIndex, int nItemID, int nNeedItemCount, int nCurItemCount );

	// Check enhancement destruction prevention item
	void CheckUpgradeGuardItem();

#if defined(PRE_ADD_ENCHANTSHIELD_CASHITEM)
	// Set enhancement support item
	void SetUpgardeGuardItem();
#endif

	void CheckUseItemTooltip();

	// Next enhance level
	void RefreshNextUpgradeItem();
	void ShowNextUpgradeToolTip( bool bShow, float fX = 0.f, float fY = 0.f );

	bool IsUpgradableItem(CDnSlotButton* pSlotButton, CDnItem* pItem) const;

public:

#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
	void SetUpgradeItem( CDnSlotButton *pPressedButton );
	bool IsEquipeedItem() { return m_bEnchantEquippedItem; }
	bool IsUpgradeItem( CDnSlotButton *pPressedButton ); 
#else
	// Direct registration on right-click in inventory.
	void SetUpgradeItem( CDnQuickSlotButton *pPressedButton );
#endif 

#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
	void SetEquippedItem( int nEquipIndex, MIInventoryItem *pItem );
	MIInventoryItem *GetEquippedItem( int nEquipIndex );
	CDnSlotButton   *GetEquipSlotButton( int nEquipIndex );
	void ResetEquippedSlot( int nEquipIndex );
#endif 

	void SetRemoteItemID( int nRemoteItemID );

	void OnRecvEnchant(int nSlotIndex);	// Argument is for validation.
	void OnRecvEnchantComplete( char cEnchantResult, int nItemID, char cLevel = 0, char cOption = 0 );

#ifdef PRE_ADD_JELLYCOUNT
	void CheckUpgradState(); // Handle text according to the item's upgrade state.
#endif

#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
	void				SafeDelForceSlotItem()	{ SAFE_DELETE( m_pUpgradeItem); }
	CDnItemSlotButton* GetCurrentForceSlot() { return m_pUpgradeItemSlot ? m_pUpgradeItemSlot : NULL; }
	void SetPreInvenSlot( CDnQuickSlotButton* pInvenSlot ) { m_pPreInvenSlotButton = pInvenSlot; }		
	CDnQuickSlotButton* GetPreInvenSlot()    { return m_pPreInvenSlotButton ? m_pPreInvenSlotButton : NULL; }
	void SetCurrentInvenSlot( CDnQuickSlotButton* pInvenSlot ) { m_pCurrentInvenSlotButton = pInvenSlot; }	
	CDnQuickSlotButton* GetCurrentInvenSlot() { return m_pCurrentInvenSlotButton; }
#endif 

#ifdef PRE_ADD_DIRECT_BUY_UPGRADEITEM
	void ResetUpgradeItemBtn();
	void ShowDirectBuyMarketDlg(int nSlotIndex);
	void RefreshUpgradeItemButton();
	void SetMiniMarketFlag(bool bFlag) { m_bIsMiniMarket = bFlag; }
#endif

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};