#pragma once

#include "EtUITabDialog.h"
#include "DnCashShopDefine.h"
#include "DnCashShopCommonDlgMgr.h"

class CDnCashShopCatDlg;
class CDnCashShopPreviewDlg;
class CDnCashShopCartDlg;
class CDnCashShopCommonDlgMgr;
class CDnCashShopInvenTabDlg;
class CDnCashShopCouponDlg;
class CDnCashShopAboutPetal;
class CDnCashShopGiftFailDlg;
class CDnDummyModalDlg;
class CDnCashShopPetPreviewDlg;

#ifdef PRE_ADD_CASHSHOP_RENEWAL
class CDnCashShopDlg : public CEtUITabDialog, public CEtUICallback
{
public:
	enum
	{
		PREVIEW_DLG,
		INVEN_DLG,
		SEND_GIFT_DLG,
		CASHCHARGE_ALERT_DLG,
		LEVEL_ALERT_CART_DLG,
		LEVEL_ALERT_PREVIEWCART_DLG,
		LEVEL_ALERT_BUYITEMNOW_DLG,
	};

	CDnCashShopDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL);
	virtual ~CDnCashShopDlg();

	virtual void Initialize( const char *pFileName, bool bShow );
#ifdef PRE_ADD_CASHSHOP_RENEWAL
	virtual void		Initialize(bool bShow){}
#else
	virtual void		Initialize(bool bShow);
#endif // PRE_ADD_CASHSHOP_RENEWAL
	virtual void		Show(bool bShow);
	void				UpdateGiftReceiverInfo(const SCCashShopCheckReceiver& info);
	void				UpdateRecvGiftList();
	void				UpdateCashReserveAmount();

	void				OpenGiftReply(const WCHAR* pName, int giftDBID);

	void				OpenBuyCart(bool bShow);
	void				OpenBuyPreviewCart(bool bShow);
	void				GiftCart(bool bClear);
	void				GiftPreviewCart(bool bClear);
	void				UpdateCart(eRetCartAction action, const SCartActionParam& param);

#ifdef PRE_ADD_INVEN_EXTENSION
	void				ReserveBuyItemNowOnNextOpened(CASHITEM_SN sn);
#endif

	void				ClearPreviewCart(int classId);
	bool				AttachPartsToPreview(CASHITEM_SN sn, ITEMCLSID itemId, bool bCheckPackageCashRing2 = false);
	void				DetachPartsToPreview(int classId, CASHITEM_SN sn);
	void				HandleCashShopError(eCashShopError code, bool bShowCode, CEtUIDialog* pCallbackDlg = NULL) const;

	bool				IsOpenCashShopInvenDlg() const;
	bool				IsOpenCashShopRecvGiftDlg() const;
	void				DisableAllDlgs(bool bEnable, const std::wstring& text, float delayTime = 0.f);
	eRetPreviewChange	ChangePreviewAvatar(int classId,bool bForce = false);

	void                ChangeVehicleParts(int nClassID = 0 ,int nPartClassID = 0, bool bDefault=false);
	void				ShowGiftPayDlg(bool bShow, eCashUnitType type);
	void				ShowCouponDlg(bool bShow);
	void				ShowRecvGiftDlg(bool bShow);
	void				ShowGiftFailDlg(bool bShow, const std::wstring& msg, const std::vector<CASHITEM_SN>& itemList, UINT uType, int callBackDlgID = -1, CEtUICallback* pCall = NULL) const;
	eCashShopError		IsCartGiftable(eCashUnitType type) const;
	bool				IsCartBuyable() const;
	bool				IsPreviewCartBuyable() const;

	void				EnableDummyModalDlg(bool bSet);
	void				OnCloseCashShopDlg();


	void				ShowCashShopMsgBox(const std::wstring& firstmsg, const std::wstring& secondmsg, int nID, CEtUICallback* pCallBack);
	void				ChangePetPreviewAvatar( int nItemID );
	void				ChangePetPreviewParts( int nPartsType, int nItemID );
	void				ChangePetPreviewColor( eItemTypeEnum eItemType, DWORD dwColor );
	void				OpenBuyPetPreviewCart();
	bool				IsExistPetAvatar();
	void				SwapPreview( bool bShowChar );
	void				DetachPartsToPetPreview( int nPartsSlot );
	CDnCashShopPetPreviewDlg* GetPetPreviewDlg() { return m_pPetPreviewDlg; }
#ifdef PRE_ADD_SALE_COUPON
	virtual void				CheckShowSaleTabButton( bool bSaleCouponGift = false );
	void				SetSaleTabButtonBlink( bool bBlink );
#ifdef PRE_ADD_SALE_COUPON_CATEGORY
	void				SetSaleCouponOpen( bool bSaleCouponOpen, int nSaleCouponSubCategory ) { m_bSaleCouponOpen = bSaleCouponOpen; m_nSaleCouponSubCategory = nSaleCouponSubCategory; }
#else // PRE_ADD_SALE_COUPON_CATEGORY
	void				SetSaleCouponOpen( bool bSaleCouponOpen ) { m_bSaleCouponOpen = bSaleCouponOpen; }
#endif // PRE_ADD_SALE_COUPON_CATEGORY
#endif // PRE_ADD_SALE_COUPON

#ifdef PRE_ADD_LIMITED_CASHITEM	
		void UpdateCurrentGoodsDlg();	
#endif // PRE_ADD_LIMITED_CASHITEM
#ifdef PRE_ADD_CADGE_CASH
	void				CadgeButtonClick() { m_bCadgeButtonClick = true; }
	void				SetReserveCadgeItemList( std::wstring strCadgeReciever, int nMailID, int nPackageSN, int nCount, TWishItemInfo* pItemList );
	int					GetCadgeMailID() { return m_nMailID; }
	void				ResetCadgeMailID() { m_nMailID = 0; }
#endif // PRE_ADD_CADGE_CASH

#ifdef PRE_ADD_CASHSHOP_ACTOZ
	void ShowConfirmRefund( CDnItem * pItem );
#endif // PRE_ADD_CASHSHOP_ACTOZ

	// Product search.
	//void SearchItems( const std::wstring & str );
	
protected:
	virtual void		Render(float fElapsedTime);
	virtual void		InitialUpdate();
	virtual void		ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0);
	virtual void		OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void		Process( float fElapsedTime );
	virtual bool		MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
	const WCHAR*		GetCatTitle(const SCashShopTabShowInfo& info) const;
	void				MoveCategory(bool bNext);

	std::vector<CEtUIRadioButton*>	m_pCategoryBtns;
	std::vector<CDnCashShopCatDlg*>	m_pCategoryDlgs;
	CDnCashShopPreviewDlg*			m_pPreviewDlg;
	CDnCashShopPetPreviewDlg*		m_pPetPreviewDlg;
	int								m_nPetCategory;
	CDnCashShopCartDlg*				m_pCartDlg;
	CDnCashShopInvenTabDlg*			m_pInvenDlg;
	CDnCashShopCouponDlg*			m_pCouponDlg;
	CDnCashShopAboutPetal*			m_pAboutPetalDlg;
	CDnCashShopGiftFailDlg*			m_pGiftFailDlg;
	CDnCashShopMessageBox*			m_pCashShopMsgBox;

#ifdef PRE_ADD_CASHSHOP_ACTOZ
	class CDnCashShopConfirmRefundDlg * m_pConfirmRefundDlg;
#endif // PRE_ADD_CASHSHOP_ACTOZ

#if defined PRE_ADD_PETALSHOP || defined PRE_CHN_OBTCASHSHOP
	CEtUIStatic*					m_pCashUnitBG;
	CEtUIStatic*					m_pCashTitle;
	CEtUIStatic*					m_pCashUnit;
#endif // defined PRE_ADD_PETALSHOP || defined PRE_CHN_OBTCASHSHOP

#ifdef PRE_CHN_OBTCASHSHOP
	CEtUIStatic*					m_pReserveUnitBG;
	CEtUIStatic*					m_pReserveTitle;
	CEtUIStatic*					m_pReserveUnit;
#endif // PRE_CHN_OBTCASHSHOP

#ifdef PRE_ADD_CASHSHOP_CREDIT
	CEtUIStatic*					m_pCreditAmount;
#endif // PRE_ADD_CASHSHOP_CREDIT
	CEtUIStatic*					m_pCashAmount;
	CEtUIStatic*					m_pReserveAmount;
	CEtUIButton*					m_pChargeBtn;
	CEtUIButton*					m_pExitBtn;

	CEtUIButton*					m_pCashInvenBtn;
	CEtUIButton*					m_pCouponBtn;
	CEtUIButton*					m_pGiftBtn;

	CEtUIButton*					m_pAboutPetalBtn;
#ifdef PRE_ADD_LIMITED_CASHITEM
	CEtUIButton*					m_pRefreshBtn;
	float							m_fRefreshCoolTime;
	bool							m_bRefreshCoolTimer;
#endif // PRE_ADD_LIMITED_CASHITEM
	EtTextureHandle					m_hBgTexture;
	SUICoord						m_BgCoord;
	SUICoord						m_ScreenCoord;

	CDnCashShopCommonDlgMgr			m_CommonDlgMgr;
	CDnDummyModalDlg*				m_pDummyModalDlg;

	std::wstring					m_DisableDlgStringCache;
	float							m_DisableDlgDelayTime;
	bool							m_bOpenGiftWindow;

#ifdef PRE_ADD_INVEN_EXTENSION
	CASHITEM_SN						m_ReserveBuyItemNowSN;
#endif // PRE_ADD_INVEN_EXTENSION

#ifdef PRE_ADD_CADGE_CASH
	CART_ITEM_LIST					m_ReserveCadgeItemList;
	std::wstring					m_ReserveCadgeReciever;
	int								m_nMailID;
	int								m_nPackageSN;
	bool							m_bCadgeButtonClick;
#endif // PRE_ADD_CADGE_CASH
#ifdef PRE_ADD_SALE_COUPON
	bool							m_bSaleCouponOpen;
#ifdef PRE_ADD_SALE_COUPON_CATEGORY
	int								m_nSaleCouponSubCategory;
#endif // PRE_ADD_SALE_COUPON_CATEGORY
#endif // PRE_ADD_SALE_COUPON
};

#else ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDnCashShopDlg : public CEtUITabDialog, public CEtUICallback
{
public:
	enum
	{
		PREVIEW_DLG,
		INVEN_DLG,
		SEND_GIFT_DLG,
		CASHCHARGE_ALERT_DLG,
		LEVEL_ALERT_CART_DLG,
		LEVEL_ALERT_PREVIEWCART_DLG,
		LEVEL_ALERT_BUYITEMNOW_DLG,
	};

	CDnCashShopDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL);
	virtual ~CDnCashShopDlg();

	virtual void		Initialize(bool bShow);
	virtual void		Show(bool bShow);
	void				UpdateGiftReceiverInfo(const SCCashShopCheckReceiver& info);
	void				UpdateRecvGiftList();
	void				UpdateCashReserveAmount();

	void				OpenGiftReply(const WCHAR* pName, int giftDBID);

	void				OpenBuyCart(bool bShow);
	void				OpenBuyPreviewCart(bool bShow);
	void				GiftCart(bool bClear);
	void				GiftPreviewCart(bool bClear);
	void				UpdateCart(eRetCartAction action, const SCartActionParam& param);

#ifdef PRE_ADD_INVEN_EXTENSION
	void				ReserveBuyItemNowOnNextOpened(CASHITEM_SN sn);
#endif

	void				ClearPreviewCart(int classId);
	bool				AttachPartsToPreview(CASHITEM_SN sn, ITEMCLSID itemId, bool bCheckPackageCashRing2 = false);
	void				DetachPartsToPreview(int classId, CASHITEM_SN sn);
	void				HandleCashShopError(eCashShopError code, bool bShowCode, CEtUIDialog* pCallbackDlg = NULL) const;

	bool				IsOpenCashShopInvenDlg() const;
	bool				IsOpenCashShopRecvGiftDlg() const;
	void				DisableAllDlgs(bool bEnable, const std::wstring& text, float delayTime = 0.f);
	eRetPreviewChange	ChangePreviewAvatar(int classId,bool bForce = false);

	void                ChangeVehicleParts(int nClassID = 0 ,int nPartClassID = 0, bool bDefault=false);
	void				ShowGiftPayDlg(bool bShow, eCashUnitType type);
	void				ShowCouponDlg(bool bShow);
	void				ShowRecvGiftDlg(bool bShow);
	void				ShowGiftFailDlg(bool bShow, const std::wstring& msg, const std::vector<CASHITEM_SN>& itemList, UINT uType, int callBackDlgID = -1, CEtUICallback* pCall = NULL) const;
	eCashShopError		IsCartGiftable(eCashUnitType type) const;
	bool				IsCartBuyable() const;
	bool				IsPreviewCartBuyable() const;

	void				EnableDummyModalDlg(bool bSet);
	void				OnCloseCashShopDlg();


	void				ShowCashShopMsgBox(const std::wstring& firstmsg, const std::wstring& secondmsg, int nID, CEtUICallback* pCallBack);
	void				ChangePetPreviewAvatar( int nItemID );
	void				ChangePetPreviewParts( int nPartsType, int nItemID );
	void				ChangePetPreviewColor( eItemTypeEnum eItemType, DWORD dwColor );
	void				OpenBuyPetPreviewCart();
	bool				IsExistPetAvatar();
	void				SwapPreview( bool bShowChar );
	void				DetachPartsToPetPreview( int nPartsSlot );
	CDnCashShopPetPreviewDlg* GetPetPreviewDlg() { return m_pPetPreviewDlg; }
#ifdef PRE_ADD_SALE_COUPON
	void				CheckShowSaleTabButton( bool bSaleCouponGift = false );
	void				SetSaleTabButtonBlink( bool bBlink );	
#ifdef PRE_ADD_SALE_COUPON_CATEGORY
	void				SetSaleCouponOpen( bool bSaleCouponOpen, int nSaleCouponSubCategory ) { m_bSaleCouponOpen = bSaleCouponOpen; m_nSaleCouponSubCategory = nSaleCouponSubCategory; }
#else // PRE_ADD_SALE_COUPON_CATEGORY
	void				SetSaleCouponOpen( bool bSaleCouponOpen ) { m_bSaleCouponOpen = bSaleCouponOpen; }
#endif // PRE_ADD_SALE_COUPON_CATEGORY
#endif // PRE_ADD_SALE_COUPON

#ifdef PRE_ADD_LIMITED_CASHITEM
	void				UpdateCurrentGoodsDlg();
#endif // PRE_ADD_LIMITED_CASHITEM
#ifdef PRE_ADD_CADGE_CASH
	void				CadgeButtonClick() { m_bCadgeButtonClick = true; }
	void				SetReserveCadgeItemList( std::wstring strCadgeReciever, int nMailID, int nPackageSN, int nCount, TWishItemInfo* pItemList );
	int					GetCadgeMailID() { return m_nMailID; }
	void				ResetCadgeMailID() { m_nMailID = 0; }
#endif // PRE_ADD_CADGE_CASH

#ifdef PRE_ADD_CASHSHOP_ACTOZ
	void ShowConfirmRefund( CDnItem * pItem );
#endif // PRE_ADD_CASHSHOP_ACTOZ

protected:
	virtual void		Render(float fElapsedTime);
	virtual void		InitialUpdate();
	virtual void		ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0);
	virtual void		OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void		Process( float fElapsedTime );
	virtual bool		MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:
	const WCHAR*		GetCatTitle(const SCashShopTabShowInfo& info) const;
	void				MoveCategory(bool bNext);

	std::vector<CEtUIRadioButton*>	m_pCategoryBtns;
	std::vector<CDnCashShopCatDlg*>	m_pCategoryDlgs;
	CDnCashShopPreviewDlg*			m_pPreviewDlg;
	CDnCashShopPetPreviewDlg*		m_pPetPreviewDlg;
	int								m_nPetCategory;
	CDnCashShopCartDlg*				m_pCartDlg;
	CDnCashShopInvenTabDlg*			m_pInvenDlg;
	CDnCashShopCouponDlg*			m_pCouponDlg;
	CDnCashShopAboutPetal*			m_pAboutPetalDlg;
	CDnCashShopGiftFailDlg*			m_pGiftFailDlg;
	CDnCashShopMessageBox*			m_pCashShopMsgBox;

#ifdef PRE_ADD_CASHSHOP_ACTOZ
	class CDnCashShopConfirmRefundDlg * m_pConfirmRefundDlg;
#endif // PRE_ADD_CASHSHOP_ACTOZ

#if defined PRE_ADD_PETALSHOP || defined PRE_CHN_OBTCASHSHOP
	CEtUIStatic*					m_pCashUnitBG;
	CEtUIStatic*					m_pCashTitle;
	CEtUIStatic*					m_pCashUnit;
#endif // defined PRE_ADD_PETALSHOP || defined PRE_CHN_OBTCASHSHOP

#ifdef PRE_CHN_OBTCASHSHOP
	CEtUIStatic*					m_pReserveUnitBG;
	CEtUIStatic*					m_pReserveTitle;
	CEtUIStatic*					m_pReserveUnit;
#endif // PRE_CHN_OBTCASHSHOP

#ifdef PRE_ADD_CASHSHOP_CREDIT
	CEtUIStatic*					m_pCreditAmount;
#endif // PRE_ADD_CASHSHOP_CREDIT
	CEtUIStatic*					m_pCashAmount;
	CEtUIStatic*					m_pReserveAmount;
	CEtUIButton*					m_pChargeBtn;
	CEtUIButton*					m_pExitBtn;

	CEtUIButton*					m_pCashInvenBtn;
	CEtUIButton*					m_pCouponBtn;
	CEtUIButton*					m_pGiftBtn;

	CEtUIButton*					m_pAboutPetalBtn;
#ifdef PRE_ADD_LIMITED_CASHITEM
	CEtUIButton*					m_pRefreshBtn;
	float							m_fRefreshCoolTime;
	bool							m_bRefreshCoolTimer;
#endif // PRE_ADD_LIMITED_CASHITEM
	EtTextureHandle					m_hBgTexture;
	SUICoord						m_BgCoord;
	SUICoord						m_ScreenCoord;

	CDnCashShopCommonDlgMgr			m_CommonDlgMgr;
	CDnDummyModalDlg*				m_pDummyModalDlg;

	std::wstring					m_DisableDlgStringCache;
	float							m_DisableDlgDelayTime;
	bool							m_bOpenGiftWindow;

#ifdef PRE_ADD_INVEN_EXTENSION
	CASHITEM_SN						m_ReserveBuyItemNowSN;
#endif // PRE_ADD_INVEN_EXTENSION

#ifdef PRE_ADD_CADGE_CASH
	CART_ITEM_LIST					m_ReserveCadgeItemList;
	std::wstring					m_ReserveCadgeReciever;
	int								m_nMailID;
	int								m_nPackageSN;
	bool							m_bCadgeButtonClick;
#endif // PRE_ADD_CADGE_CASH
#ifdef PRE_ADD_SALE_COUPON
	bool							m_bSaleCouponOpen;
#ifdef PRE_ADD_SALE_COUPON_CATEGORY
	int								m_nSaleCouponSubCategory;
#endif // PRE_ADD_SALE_COUPON_CATEGORY
#endif // PRE_ADD_SALE_COUPON
};
#endif // PRE_ADD_CASHSHOP_RENEWAL