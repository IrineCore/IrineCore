#pragma once
#include "EtUITabDialog.h"
#include "DnCashShopDefine.h"

class CEtUIRadioButton;
class CDnCashShopGoodsDlg;
class CDnCashShopCommonDlgMgr;
class CDnCashShopCatDlg : public CEtUITabDialog
{
	enum eChildID
	{
		GOODS_DLG,
	};
public:
	CDnCashShopCatDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL);
	virtual ~CDnCashShopCatDlg(void);

	virtual void	Initialize(bool bShow, int mainTabNum, CDnCashShopCommonDlgMgr* pCommonDlgMgr);

	void			Show(bool bShow);
	void			UpdatePage();

#ifdef PRE_ADD_SALE_COUPON
	int				GetMainTabNum() { return m_MainTabNum; }
	CEtUIDialogGroup&		GetGroupTabDialog()	{ return m_groupTabDialog; }
#endif // PRE_ADD_SALE_COUPON

protected:
	virtual void	InitialUpdate();
	virtual bool	MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:
	void			MoveCategory(bool bNext);

	typedef std::map<std::string, eCashShopSubCatType> TAB_CATTYPE_MATCHLIST;

	TAB_CATTYPE_MATCHLIST		m_TabStrCatTypeMatchList;
	int							m_MainTabNum;
	CDnCashShopCommonDlgMgr*	m_pCommonDlgMgr;

#ifdef PRE_ADD_CASHSHOP_RENEWAL	
	std::vector<CEtUIRadioButton*>	m_pCategoryBtns;
#endif // PRE_ADD_CASHSHOP_RENEWAL

public:

#ifdef PRE_ADD_CASHSHOP_RENEWAL	
	void ChangeSortJob( int nJob );
	void ChangeSortEtc( int nEtc );
	eCashShopSubCatType SearchItems( int type );

	CEtUIDialog * GetMainSubDlg(); // Returns MainSubDlg.

	void SelectCategory( int tabID );
	//void UpdateSaleTab(); // Update discount coupon tab.
#endif // PRE_ADD_CASHSHOP_RENEWAL


};