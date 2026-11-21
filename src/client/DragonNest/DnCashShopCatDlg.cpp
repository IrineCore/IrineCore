#include "StdAfx.h"
#include "DnCashShopCatDlg.h"
#include "DnCashShopGoodsDlg.h"
#include "DnCashShopCommonDlgMgr.h"
#include "DnItemTask.h"
#include "DnCashShopTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCashShopCatDlg::CDnCashShopCatDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUITabDialog( dialogType, pParentDialog, nID, pCallback  )
, m_pCommonDlgMgr( NULL )
, m_MainTabNum( 0 )
{
}

CDnCashShopCatDlg::~CDnCashShopCatDlg(void)
{
#ifdef PRE_ADD_CASHSHOP_RENEWAL	
	m_pCategoryBtns.clear();
#endif // PRE_ADD_CASHSHOP_RENEWAL
}

void CDnCashShopCatDlg::Initialize(bool bShow, int mainTabNum, CDnCashShopCommonDlgMgr* pCommonDlgMgr)
{
	if (pCommonDlgMgr == NULL)
	{
		_ASSERT(0);
		return;
	}

	m_pCommonDlgMgr = pCommonDlgMgr;
	m_MainTabNum = mainTabNum;

	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName( "CSsubCategory.ui" ).c_str(), bShow);
}

#define MAX_RADIO_BTN_COUNT 9

void CDnCashShopCatDlg::InitialUpdate()
{
	CDnCashShopGoodsDlg*	pDlg(NULL);
	CEtUIRadioButton*		pTabButton(NULL);

	int i = 0;
	std::string subTabID;

	if (CDnCashShopTask::IsActive() == false)
	{
		_ASSERT(0);
		return;
	}

	const std::map<int, SCashShopTabShowInfo>* pShowSubTabList = GetCashShopTask().GetShowSubTabList(m_MainTabNum);


//-----------------------------//
// PRE_ADD_CASHSHOP_RENEWAL 
#ifdef PRE_ADD_CASHSHOP_RENEWAL
	
	// Search category //
	if( pShowSubTabList == NULL && m_MainTabNum == MAINCATEGORY_SERACH )
	{
		CDnCashShopGoodsDlg* pGoodsDlg = new CDnCashShopGoodsDlg( UI_TYPE_CHILD, this, SUBCATEGORY_SERACH );
#ifdef PRE_ADD_SALE_COUPON
		pGoodsDlg->Initialize( false, m_pCommonDlgMgr, false, false, m_MainTabNum );
#else
		pGoodsDlg->Initialize( false, m_pCommonDlgMgr, false, false );
#endif // PRE_ADD_SALE_COUPON
		CEtUIRadioButton * pRBtn = GetControl<CEtUIRadioButton>("ID_BUTTON_DETAILCATEGORY_SEARCH");
		AddTabDialog( pRBtn, pGoodsDlg );
		pRBtn->Show( false );
		m_pCategoryBtns.push_back( pRBtn );
	}
	

	// Category //
	else
	{
		if (pShowSubTabList == NULL)
			return;

		// MainCategory.
		bool bMainTab = ( m_MainTabNum == 0 ? true : false );


#ifdef PRE_ADD_CASHSHOP_SUBCAT_COLOR
		const std::map<int, SCashShopSubTabShowInfo>* pShowSubTabCatList = GetCashShopTask().GetShowSubTabCatList( m_MainTabNum );
		if( pShowSubTabCatList == NULL )
			return;

		std::map<int, SCashShopSubTabShowInfo>::const_iterator SubCatiter = pShowSubTabCatList->begin();

		std::wstring SelectedColor;
		std::wstring DeSelectedColor;	
#endif 

		std::map<int, SCashShopTabShowInfo>::const_iterator iter = pShowSubTabList->begin();
		for (; i < MAX_RADIO_BTN_COUNT; ++i)
		{
			subTabID = FormatA("ID_BUTTON_DETAILCATEGORY%d", i);
			pTabButton = GetControl<CEtUIRadioButton>(subTabID.c_str());
			if (pTabButton == NULL)
			{
				_ASSERT(0);
				continue;
			}

			if (iter == pShowSubTabList->end())
			{
				pTabButton->Show(false);
				continue;
			}

			int curType = (*iter).first;
			const SCashShopTabShowInfo& subTabInfo = (*iter).second;
			++iter;

			if (subTabInfo.showType == eCST_HIDE)
			{
				pTabButton->Show(false);
				continue;
			}
			else if (subTabInfo.showType == eCST_DISABLE)
				pTabButton->Enable(false);

			const std::wstring subTitle = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, subTabInfo.uiStringNum);
			if (subTitle.empty() == false)
			{
				pTabButton->SetText(subTitle.c_str());
#ifdef PRE_ADD_CASHSHOP_SUBCAT_COLOR

				SubCatiter = pShowSubTabCatList->begin();

				while( SubCatiter != pShowSubTabCatList->end() )
				{
					const SCashShopSubTabShowInfo& subTabCatInfo =  (*SubCatiter).second ;
					SelectedColor	= subTabCatInfo.szSelectedColor.IsEmpty() ? SelectedColor : subTabCatInfo.szSelectedColor.GetString();

					if( ! SelectedColor.empty() && i == _wtoi(&SelectedColor[0]) ) {
						pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_FOCUS, 1 );
						pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_NORMAL, 1 );
						pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_MOUSEENTER, 1 );
						pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_PRESSED, 1 );
						break;
					}
					else{
						SelectedColor = L"0:z";
						pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_FOCUS, 1 );
						pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_NORMAL, 1 );
						pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_MOUSEENTER, 1 );
						pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_PRESSED, 1 );
					}

					SubCatiter++;
				}

				SubCatiter = pShowSubTabCatList->begin();

				while( SubCatiter != pShowSubTabCatList->end() ) 
				{
					const SCashShopSubTabShowInfo& subTabCatInfo =  (*SubCatiter).second ;
					DeSelectedColor = subTabCatInfo.szDeSelectedColor.IsEmpty() ? DeSelectedColor : subTabCatInfo.szDeSelectedColor.GetString();

					if( ! DeSelectedColor.empty() && i == _wtoi(&DeSelectedColor[0]) )
					{
						pTabButton->SetTextColor( DeSelectedColor, UI_CONTROL_STATE::UI_STATE_NORMAL  );
						pTabButton->SetTextColor( DeSelectedColor, UI_CONTROL_STATE::UI_STATE_MOUSEENTER  );
						pTabButton->SetTextColor( DeSelectedColor, UI_CONTROL_STATE::UI_STATE_PRESSED  );
					}	 
					SubCatiter++;
				}

#endif
			}
			else
			{
				const SCashShopMainTabShowInfo* pInfo = GetCashShopTask().GetShowMainTabInfo(m_MainTabNum);
				if (pInfo->option & eCSMTOPT_WISHLIST)
				{
					CDnCashShopGoodsDlg* pGoodsDlg = new CDnCashShopGoodsDlg( UI_TYPE_CHILD, this, curType, NULL, bMainTab );
#ifdef PRE_ADD_SALE_COUPON
					pGoodsDlg->Initialize( false, m_pCommonDlgMgr, true, (pInfo->option & eCSMTOPT_JOBSORT) == 0, m_MainTabNum );
#else // PRE_ADD_SALE_COUPON
					pGoodsDlg->Initialize( false, m_pCommonDlgMgr, true, (pInfo->option & eCSMTOPT_JOBSORT) == 0 );
#endif // PRE_ADD_SALE_COUPON
					pTabButton->SetText(L"");
					AddTabDialog(pTabButton, pGoodsDlg);
					pTabButton->Enable(false);
					m_pCategoryBtns.push_back( pTabButton );
					break;
				}
				else
				{
					pTabButton->Show(false);
					continue;
				}
			}

			const SCashShopMainTabShowInfo* pInfo = GetCashShopTask().GetShowMainTabInfo(m_MainTabNum);
			CDnCashShopGoodsDlg* pGoodsDlg = new CDnCashShopGoodsDlg( UI_TYPE_CHILD, this, curType, NULL, bMainTab );

#ifdef PRE_ADD_SALE_COUPON
			pGoodsDlg->Initialize( false, m_pCommonDlgMgr, false, (pInfo->option & eCSMTOPT_JOBSORT) == 0, m_MainTabNum );
#else // PRE_ADD_SALE_COUPON
			pGoodsDlg->Initialize( false, m_pCommonDlgMgr, false, (pInfo->option & eCSMTOPT_JOBSORT) == 0 );
#endif // PRE_ADD_SALE_COUPON
			AddTabDialog(pTabButton, pGoodsDlg);
			m_pCategoryBtns.push_back( pTabButton );
			
			// Hide.
			pTabButton->Show( false );
		}
	}

	GetControl<CEtUIStatic>("ID_STATIC0")->Show( false ); // Hide.

//-----------------------------//
#else // PRE_ADD_CASHSHOP_RENEWAL 

	if (pShowSubTabList == NULL)
		return;
	
#ifdef PRE_ADD_CASHSHOP_SUBCAT_COLOR
	const std::map<int, SCashShopSubTabShowInfo>* pShowSubTabCatList = GetCashShopTask().GetShowSubTabCatList( m_MainTabNum );
	std::map<int, SCashShopSubTabShowInfo>::const_iterator SubCatiter = pShowSubTabCatList->begin();
	
	std::wstring SelectedColor;
	std::wstring DeSelectedColor;	

	if( pShowSubTabCatList == NULL )
		return;
#endif 
	
	std::map<int, SCashShopTabShowInfo>::const_iterator iter = pShowSubTabList->begin();
	for (; i < MAX_RADIO_BTN_COUNT; ++i)
	{
		subTabID = FormatA("ID_BUTTON_DETAILCATEGORY%d", i);
		pTabButton = GetControl<CEtUIRadioButton>(subTabID.c_str());
		if (pTabButton == NULL)
		{
			_ASSERT(0);
			continue;
		}

		if (iter == pShowSubTabList->end())
		{
			pTabButton->Show(false);
			continue;
		}

		int curType = (*iter).first;
		const SCashShopTabShowInfo& subTabInfo = (*iter).second;
		++iter;

		if (subTabInfo.showType == eCST_HIDE)
		{
			pTabButton->Show(false);
			continue;
		}
		else if (subTabInfo.showType == eCST_DISABLE)
			pTabButton->Enable(false);

		const std::wstring subTitle = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, subTabInfo.uiStringNum);
		if (subTitle.empty() == false)
		{
			pTabButton->SetText(subTitle.c_str());
#ifdef PRE_ADD_CASHSHOP_SUBCAT_COLOR
			
			SubCatiter = pShowSubTabCatList->begin();

			while( SubCatiter != pShowSubTabCatList->end() )
			{
				const SCashShopSubTabShowInfo& subTabCatInfo =  (*SubCatiter).second ;
				SelectedColor	= subTabCatInfo.szSelectedColor.IsEmpty() ? SelectedColor : subTabCatInfo.szSelectedColor.GetString();

				if( ! SelectedColor.empty() && i == _wtoi(&SelectedColor[0]) ) {
					pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_FOCUS, 1 );
					pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_NORMAL, 1 );
					pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_MOUSEENTER, 1 );
					pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_PRESSED, 1 );
					break;
				}
				else{
					SelectedColor = L"0:z";
					pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_FOCUS, 1 );
					pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_NORMAL, 1 );
					pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_MOUSEENTER, 1 );
					pTabButton->SetTextColor( SelectedColor, UI_CONTROL_STATE::UI_STATE_PRESSED, 1 );
				}

				SubCatiter++;
			}

			SubCatiter = pShowSubTabCatList->begin();

			while( SubCatiter != pShowSubTabCatList->end() ) 
			{
				const SCashShopSubTabShowInfo& subTabCatInfo =  (*SubCatiter).second ;
				DeSelectedColor = subTabCatInfo.szDeSelectedColor.IsEmpty() ? DeSelectedColor : subTabCatInfo.szDeSelectedColor.GetString();

				if( ! DeSelectedColor.empty() && i == _wtoi(&DeSelectedColor[0]) )
				{
					pTabButton->SetTextColor( DeSelectedColor, UI_CONTROL_STATE::UI_STATE_NORMAL  );
					pTabButton->SetTextColor( DeSelectedColor, UI_CONTROL_STATE::UI_STATE_MOUSEENTER  );
					pTabButton->SetTextColor( DeSelectedColor, UI_CONTROL_STATE::UI_STATE_PRESSED  );
				}	 
				SubCatiter++;
			}
			
#endif
		}
		else
		{
			const SCashShopMainTabShowInfo* pInfo = GetCashShopTask().GetShowMainTabInfo(m_MainTabNum);
			if (pInfo->option & eCSMTOPT_WISHLIST)
			{
				CDnCashShopGoodsDlg* pGoodsDlg = new CDnCashShopGoodsDlg( UI_TYPE_CHILD, this, curType);
#ifdef PRE_ADD_SALE_COUPON
				pGoodsDlg->Initialize( false, m_pCommonDlgMgr, true, (pInfo->option & eCSMTOPT_JOBSORT) == 0, m_MainTabNum );
#else // PRE_ADD_SALE_COUPON
				pGoodsDlg->Initialize( false, m_pCommonDlgMgr, true, (pInfo->option & eCSMTOPT_JOBSORT) == 0 );
#endif // PRE_ADD_SALE_COUPON
				pTabButton->SetText(L"");
				AddTabDialog(pTabButton, pGoodsDlg);

				pTabButton->Enable(false);
				break;
			}
			else
			{
				pTabButton->Show(false);
				continue;
			}
		}

		const SCashShopMainTabShowInfo* pInfo = GetCashShopTask().GetShowMainTabInfo(m_MainTabNum);
		CDnCashShopGoodsDlg* pGoodsDlg = new CDnCashShopGoodsDlg( UI_TYPE_CHILD, this, curType);

#ifdef PRE_ADD_SALE_COUPON
		pGoodsDlg->Initialize( false, m_pCommonDlgMgr, false, (pInfo->option & eCSMTOPT_JOBSORT) == 0, m_MainTabNum );
#else // PRE_ADD_SALE_COUPON
		pGoodsDlg->Initialize( false, m_pCommonDlgMgr, false, (pInfo->option & eCSMTOPT_JOBSORT) == 0 );
#endif // PRE_ADD_SALE_COUPON
		AddTabDialog(pTabButton, pGoodsDlg);
	}

#endif // PRE_ADD_CASHSHOP_RENEWAL
//-----------------------------//


}

void CDnCashShopCatDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	CEtUITabDialog::Show(bShow);

	if (m_pCallback)
		m_pCallback->OnUICallbackProc(GetDialogID(), bShow?EVENT_DIALOG_SHOW:EVENT_DIALOG_HIDE, NULL);
}

void CDnCashShopCatDlg::UpdatePage()
{
}

void CDnCashShopCatDlg::MoveCategory(bool bNext)
{
	int i = 0;
	DWORD currentTabID = GetCurrentTabID();
	if (currentTabID < 0 || currentTabID >= UINT_MAX)
	{
		_ASSERT(0);
		return;
	}

	if (bNext)
	{
		for (i = (currentTabID + 1) % MAX_RADIO_BTN_COUNT; i != currentTabID; i = (i + 1) % MAX_RADIO_BTN_COUNT)
		{
			CEtUIRadioButton* pBtn = GetTabControl(i);
			if (pBtn && pBtn->IsShow() && pBtn->IsEnable())
			{
				pBtn->SetChecked(true);
				break;
			}
		}
	}
	else
	{
		i = currentTabID;
		for (i = ((--i) < 0) ? MAX_RADIO_BTN_COUNT - 1 : i; i != currentTabID;)
		{
			CEtUIRadioButton* pBtn = GetTabControl(i);
			if (pBtn && pBtn->IsShow() && pBtn->IsEnable())
			{
				pBtn->SetChecked(true);
				break;
			}
			i = ((--i) < 0) ? MAX_RADIO_BTN_COUNT - 1 : i;
		}
	}
}

bool CDnCashShopCatDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_KEYDOWN:
		{
			if (wParam == VK_TAB)
				MoveCategory(GetKeyState(VK_SHIFT) >= 0);
		}
		break;
	}

	return CEtUITabDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}


#ifdef PRE_ADD_CASHSHOP_RENEWAL
void CDnCashShopCatDlg::ChangeSortJob( int nJob )
{
	CEtUIDialog * pDlg = m_groupTabDialog.GetDialog( GetCurrentTabID() );
	if( pDlg == NULL )
		return;

	CDnCashShopGoodsDlg * pGDlg = static_cast<CDnCashShopGoodsDlg*>( pDlg );
	pGDlg->SetSortJob( nJob );
	pGDlg->Update();
}

void CDnCashShopCatDlg::ChangeSortEtc( int nEtc )
{
	CEtUIDialog * pDlg = m_groupTabDialog.GetDialog( GetCurrentTabID() );
	if( pDlg == NULL )
		return;

	CDnCashShopGoodsDlg * pGDlg = static_cast<CDnCashShopGoodsDlg*>( pDlg );
	if( pGDlg->CashshopSort( nEtc ) )
		pGDlg->Update();	
}

// Returns MainSubDlg.
CEtUIDialog * CDnCashShopCatDlg::GetMainSubDlg()
{
	CEtUIRadioButton * pBtn = GetControl<CEtUIRadioButton>("ID_BUTTON_DETAILCATEGORY0");
	if( !pBtn )
		return NULL;

	CEtUIDialog * pTabDlg = m_groupTabDialog.GetDialog( pBtn->GetTabID() );
	return pTabDlg;
}
	

//void CDnCashShopCatDlg::ChangeSortOrder( int nJob, int nEtc )
//{
//	CEtUIDialog * pDlg = m_groupTabDialog.GetDialog( GetCurrentTabID() );
//	if( pDlg == NULL )
//		return;
//
//	CDnCashShopGoodsDlg * pGDlg = static_cast<CDnCashShopGoodsDlg*>( pDlg );
//	if( nValue != -1 )
//	{
//		if( pGDlg->CashshopSort( nValue ) )
//			pGDlg->Update();
//	}
//	else
//		pGDlg->Update();
//
//}

eCashShopSubCatType CDnCashShopCatDlg::SearchItems( int type )
{
	CEtUIDialog * pDlg = m_groupTabDialog.GetDialog( type );
	if( pDlg == NULL )
		return -1;

	CDnCashShopGoodsDlg * pGDlg = static_cast<CDnCashShopGoodsDlg*>( pDlg );
	//eCashShopSubCatType orgType = pGDlg->GetCurrentCategoryType();

	//pGDlg->SetCurrentCategoryType( type ); // 1. Set as search Type.
	pGDlg->Update();						  // 2. Update the product list.
	//pGDlg->SetCurrentCategoryType( orgType ); // 3. Restore Type - restore after search since each SubCategory has its own type.

	return pGDlg->GetCurrentCategoryType();
}

void CDnCashShopCatDlg::SelectCategory( int tabID )
{		
	SetCheckedTab( tabID );
}

//void CDnCashShopCatDlg::UpdateSaleTab()
//{
//	CEtUIDialogGroup::DIALOG_VEC & vecDlg = m_groupTabDialog.GetDialogList();
//	int size = vecDlg.size();
//	for( int i=0; i<size; ++i )
//	{
//		CEtUIDialogGroup::ID_DIALOG_PAIR & r = vecDlg[ i ];
//		CEtUIDialog * pDlg = m_groupTabDialog.GetDialog( r.first );
//		if( pDlg )
//		{
//			CDnCashShopGoodsDlg * pGDlg = static_cast<CDnCashShopGoodsDlg*>( pDlg );
//			pGDlg->Update();
//		}
//	}
//}
#endif // PRE_ADD_CASHSHOP_RENEWAL