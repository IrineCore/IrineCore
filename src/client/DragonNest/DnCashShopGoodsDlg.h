#pragma once
#include "EtUIDialogGroup.h"
#include "DnCustomDlg.h"
#include "DnCashShopDefine.h"
#include "DnItem.h"

class CDnCashShopCommonDlgMgr;
class CDnCashShopGoodsDlg : public CDnCustomDlg, public CEtUICallback
{
	enum eGoodsIconType
	{
		ICON_NONE,
		ICON_MIN,
		ICON_HOT = ICON_MIN,
		ICON_NEW,
		ICON_EVENT,
		ICON_DISCOUNT,
		ICON_LIMITED,
#ifdef PRE_ADD_SALE_COUPON
		ICON_COUPON,
#endif // PRE_ADD_SALE_COUPON
#ifdef PRE_ADD_LIMITED_CASHITEM
		ICON_COUNTLIMIT,
		ICON_COUNTLIMITCLOSE,
#endif // PRE_ADD_LIMITED_CASHITEM
		MAXICON
	};

	struct SCashShopSlotUnit
	{
		CDnQuickSlotButton*	pQuickSlotBtn;
		CDnItem*			pItem;

		CEtUIStatic*		pBase;
		CEtUIStatic*		pItemName;
		CEtUIStatic*		pItemCount;
		CEtUITextBox*		pItemPrice;
		CEtUIStatic*		pItemDiscountRate;
		CEtUIStatic*		pItemReserve;
		CEtUIStatic*		pItemPeriod;
		CEtUIStatic*		pQuickSlotBG;

#ifdef PRE_ADD_CASHSHOP_RENEWAL
	    // Texture per product status.		
		static std::vector< EtTextureHandle > vStateTexture;
#else
		CEtUIStatic*		pTypeIcon[MAXICON];
#endif // PRE_ADD_CASHSHOP_RENEWAL

#ifdef PRE_ADD_CASHSHOP_RENEWAL
		std::vector< CEtUIStatic * > vItemState;
#endif // PRE_ADD_CASHSHOP_RENEWAL

		CEtUIButton*		pBuyBtn;
		CEtUIButton*		pGiftBtn;
		CEtUIButton*		pCartBtn;
//		CEtUIButton*		pWishBtn;
		CEtUIButton*		pDeleteBtn;
#ifdef PRE_ADD_LIMITED_CASHITEM
		DWORD				dwTextColor;
#endif // PRE_ADD_LIMITED_CASHITEM
		SCashShopSlotUnit()
		{
			pQuickSlotBtn = NULL;
			pBase = pItemName = pItemCount = 
			pItemDiscountRate = pItemReserve =
			pQuickSlotBG = pItemPeriod = NULL;
//			pBuyBtn = pGiftBtn = pCartBtn = pWishBtn = pDeleteBtn = NULL;
			pBuyBtn = pGiftBtn = pCartBtn = pDeleteBtn = NULL;
			pItem = NULL;
			pItemPrice = NULL;
#ifdef PRE_ADD_CASHSHOP_RENEWAL
			// ...
#else
			int i = 0;
			for (; i < MAXICON; ++i)
			{
				pTypeIcon[i] = NULL;
			}
#endif // PRE_ADD_CASHSHOP_RENEWAL

#ifdef PRE_ADD_LIMITED_CASHITEM
			dwTextColor = 0;
#endif // PRE_ADD_LIMITED_CASHITEM
		}

		~SCashShopSlotUnit()
		{
			SAFE_DELETE(pItem);

#ifdef PRE_ADD_CASHSHOP_RENEWAL
			vItemState.clear();
#endif // PRE_ADD_CASHSHOP_RENEWAL
		}

		void Set(const SCashShopItemInfo& data, int userClassId);

		void Clear()
		{
			pQuickSlotBtn->ResetSlot();
			pItemName->ClearText();
			pItemCount->ClearText();
			pItemPrice->ClearText();
			pItemDiscountRate->ClearText();
			pItemReserve->ClearText();
			pItemPeriod->ClearText();

			pItemName->Enable(true);
			pBuyBtn->Enable(true);
			pCartBtn->Enable(true);
			pItemCount->Enable(true);
			pItemPrice->Enable(true);
			pItemReserve->Enable(true);
			pItemPeriod->Enable(true);
#ifdef PRE_MOD_SALE_COUPON_LEVEL_LIMIT
			pBuyBtn->ClearTooltipText();
#endif // PRE_MOD_SALE_COUPON_LEVEL_LIMIT
		}

		void Show(bool bShow, bool bWishList)
		{
			pQuickSlotBtn->Show(bShow);

			pBase->Show(bShow);
			pItemName->Show(bShow);
			pItemCount->Show(bShow);
			pItemPrice->Show(bShow);
			pItemDiscountRate->Show(bShow);
			pItemReserve->Show(bShow);
			pQuickSlotBG->Show(bShow);
			pItemPeriod->Show(bShow);

			pBuyBtn->Show(bShow);
			pGiftBtn->Show(bShow);
			pCartBtn->Show(bShow);
			if (bWishList)
			{
				pDeleteBtn->Show(bShow);
//				pWishBtn->Show(false);
			}
			else
			{
//				pWishBtn->Show(bShow);
				pDeleteBtn->Show(false);
			}

#ifdef PRE_ADD_CASHSHOP_RENEWAL
			if (bShow == false)
			{
				int size = (int)vItemState.size();
				for( int i=0; i<size; ++ i )
					vItemState[ i ]->Show( bShow );
			}
#else
			if (bShow == false)
			{
				int i = ICON_MIN;
				for (; i < MAXICON; ++i)
					pTypeIcon[i]->Show(false);
			}
#endif // PRE_ADD_CASHSHOP_RENEWAL

		}

		bool IsShow() const { return pBase->IsShow(); }

		bool IsInside(float fX, float fY)
		{
			SUICoord uiCoords;
			pBase->GetUICoord(uiCoords);
			return uiCoords.IsInside( fX, fY );
		}

#ifdef PRE_ADD_CASHSHOP_RENEWAL
		/*void ShowIcon( std::vector<std::string> & arr )
		{
			std::map< std::string, EtTextureHandle >::iterator it;

			int size = (int)arr.size();
			for( int k=0; k<size; ++k )
			{
				it = mapStateTexture.find( arr[ k ] );
				if( it != mapStateTexture.end() )
				{
vItemState[ k ]->GetTemplate().m_hTemplateTexture = it->second; // Replace texture.
					vItemState[ k ]->Show( true );
				}
			}			
		}*/
#else
		void ShowIcon(eGoodsIconType type)
		{
			if (type == ICON_NONE)
				return;

			pTypeIcon[type]->Show(true);
		}
#endif // 

#ifdef PRE_ADD_LIMITED_CASHITEM
	#ifdef PRE_ADD_CASHSHOP_RENEWAL
		void SetCloseIcon() 
		{
			int size = (int)vItemState.size();
			for( int i=0; i<size; ++i )
			{
				if( i == 0 )
				{
					vItemState[ i ]->GetTemplate().m_hTemplateTexture = vStateTexture[ i ];					
					vItemState[ i ]->Show( true );
				}
				else
				{
					vItemState[ i ]->Show( false );
					//vItemState[ i ]->GetTemplate().m_hTemplateTexture->AddRef();
				}
			}	
			pItemName->SetTextColor( GetNameColor( ICON_COUNTLIMITCLOSE ), true );			
		}
	#else
		void SetCloseIcon() 
		{
			for( int i=ICON_MIN; i<MAXICON; ++i )
				pTypeIcon[i]->Show( false );

			ShowIcon( ICON_COUNTLIMITCLOSE );
			pItemName->SetTextColor( GetNameColor( ICON_COUNTLIMITCLOSE ), true );
		}
	#endif // PRE_ADD_CASHSHOP_RENEWAL
#endif // PRE_ADD_LIMITED_CASHITEM

		DWORD GetNameColor(eGoodsIconType type)
		{
			switch(type)
			{
			case ICON_HOT:		return  D3DCOLOR_ARGB(255,255,53,53);
			case ICON_NEW:		return  D3DCOLOR_ARGB(255,255,219,74);
			case ICON_EVENT:	return  D3DCOLOR_ARGB(255,137,158,250);
			case ICON_DISCOUNT:	return  D3DCOLOR_ARGB(255,207,167,235);
#ifdef PRE_ADD_LIMITED_CASHITEM
			case ICON_COUNTLIMIT:		return	D3DCOLOR_ARGB(255,204,50,155);
			case ICON_COUNTLIMITCLOSE:	return	textcolor::DARKGRAY;
#endif // PRE_ADD_LIMITED_CASHITEM
			}

			return textcolor::WHITE;
		}
	};	

	enum
	{
		PAY_DLG,
		LEVEL_ALERT_BUYITEMNOW_DLG,
		OVERLAP_ALERT_BUYITEMNOW_DLG,
		LEVEL_ALERT_CARTITEMNOW_DLG,
		OVERLAP_ALERT_CARTITEMNOW_DLG,
		BUY_INVEN_STORAGE_BUYITEMNOW_DLG,
		BUY_INVEN_STORAGE_CART_DLG,
	};

public:
	
#ifdef PRE_ADD_CASHSHOP_RENEWAL
	CDnCashShopGoodsDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bMainTab=false );
#else
	CDnCashShopGoodsDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL);
#endif // PRE_ADD_CASHSHOP_RENEWAL

	virtual ~CDnCashShopGoodsDlg(void);
#ifdef PRE_ADD_SALE_COUPON
	virtual void			Initialize( bool bShow, CDnCashShopCommonDlgMgr* pCommonDlgMgr, bool bWishList, bool bDisableJobSort, int nCategory );
#else // PRE_ADD_SALE_COUPON
	virtual void			Initialize( bool bShow, CDnCashShopCommonDlgMgr* pCommonDlgMgr, bool bWishList, bool bDisableJobSort );
#endif // PRE_ADD_SALE_COUPON
	virtual void			Show(bool bShow);
	virtual void			OnChangeResolution();
	void					Update();

#if defined(PRE_ADD_LIMITED_CASHITEM) || defined(PRE_ADD_CASHSHOP_RENEWAL)
	eCashShopSubCatType		GetCurrentCategoryType() { return m_Type; }
#endif // PRE_ADD_LIMITED_CASHITEM

protected:
	virtual void			InitialUpdate();
	virtual void			InitCustomControl(CEtUIControl *pControl);
	virtual void			ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0);
	virtual bool			MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void			Process( float fElapsedTime );
	virtual void			OnUICallbackProc(int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0);

private:
	void					UpdatePages(const CS_INFO_LIST* pDataList, bool bShowPage);
	void					UpdatePageControllers();
	void					UpdateSlots(const CS_INFO_LIST* pDataList, bool bShowPage);
	void					MakeItemListShowable(CS_INFO_LIST& dataList, const CS_INFO_LIST& wholeItemList);
	//void					UpdatWishList(const std::set<CASHITEM_SN>& wishList);

	const CS_INFO_LIST*			GetDataList(eCashShopSubCatType type) const;
	const SCashShopItemInfo*	GetData(eCashShopSubCatType type, int idx) const;
	int							GetOnSalePageCount(const CS_INFO_LIST& dataList) const;
	void						ClearSlots(bool bShowOff);

	void						PrevPage(bool bCircle = false);
	void						NextPage(bool bCircle = false);
	void						PrevGroupPage();
	void						NextGroupPage();

	const SCashShopItemInfo*	GetMatchListData(int index) const;
	int							GetSortStringNum(eCashShopEtcSortType type) const;

	bool						IsShowEnableItem(const SCashShopItemInfo* pInfo) const;
#ifdef PRE_ADD_SALE_COUPON
	bool						IsExistItemList( CS_INFO_LIST& dataList, int nItemdID );
#endif // PRE_ADD_SALE_COUPON

	std::vector<SCashShopSlotUnit>	m_SlotUnits;
	std::map<int, int>				m_DataSlotIdxMatchList;	// key : slot index
	std::map<int, CASHITEM_SN>		m_WishListMatchList;

#ifdef PRE_ADD_SALE_COUPON
	int							m_nCategory;
#endif // PRE_ADD_SALE_COUPON
	
#ifdef PRE_ADD_CASHSHOP_RENEWAL
	bool m_bMainTab; // First page of the main category (in this case, items are listed starting from the 3rd row)
	eCashShopSubCatType	m_Type;
	int m_SortJob; // Sort.
	

	std::vector< CEtUIRadioButton * > m_bannerRadioBtn;
	int m_BannerRollCnt;

	// Banner.
	CEtUITextureControl * m_pTextureCtr;
	std::vector<EtTextureHandle> m_vTexture;
	float m_time;
	int m_crrIdx;
	bool m_bForceRolling;

#else
	const eCashShopSubCatType	m_Type;
#endif // PRE_ADD_CASHSHOP_RENEWAL

	CEtUIDialogGroup			m_GoodsDlgGroup;
	CDnCashShopCommonDlgMgr*	m_pCommonDlgMgr;

	CEtUIButton*				m_pPageGroupPrev;
	CEtUIButton*				m_pPageGroupNext;
	CEtUIButton*				m_pPagePrev;
	CEtUIButton*				m_pPageNext;
	CEtUIButton*				m_pPageBtn[_MAX_PAGE_COUNT_PER_GROUP];

	//CEtUIStatic*				m_pJobSortBG;
	CEtUIComboBox*				m_pJobSortComboBox;
	//CEtUIStatic*				m_pEtcSortBG;
	CEtUIComboBox*				m_pEtcSortComboBox;
#ifdef PRE_ADD_CASHSHOP_JPN_REVISEDLAW
	CEtUIButton* m_pNoticeBrowserBtn;
#endif

	int							m_MaxPage;
	int							m_CurrentPage;
	int							m_SelectedIndex;

	bool						m_bWishList;
	bool						m_bDisableJobSort;
	
	// test
public:
	void OpenBrowser(const std::string& url, const SUICoord& coord);
	void OnExceptionalCloseChargeBrowser(bool onInitOLE);

#ifdef PRE_ADD_CASHSHOP_RENEWAL
	void SetHideControl( char * strName );
	void ReleaseHideTexture( char * strName );
	bool CashshopSort( int nValue );
	void SetSortJob( int nJob ){
		m_SortJob = nJob;
	}
	void SetCurrentCategoryType( eCashShopSubCatType type ){ 
		m_Type = type;
	}	

	// Do not register items with identical id and sn.
	bool CompareItem( CS_INFO_LIST & dataList, SCashShopItemInfo * pInfo );

	void RefreshPageControllers( int size );
#endif // PRE_ADD_CASHSHOP_RENEWAL


};
