#pragma once
#include "DnCustomDlg.h"
#define PRE_FIX_11027

class CDnMarketAboutDlg;

class CDnMarketAccountDlg : public CDnCustomDlg, public CEtUICallback
{

public:
	CDnMarketAccountDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMarketAccountDlg(void);

protected:
	
	struct MyAccountItem
	{
		std::wstring wszItemName;
		TMarketCalculationInfo Market;
	};
	enum eSortType {
		ST_NONE = -1,
		ST_NAME,
		ST_PRICE,
		ST_SELLTIME,		
	};

	enum eCount {		
		MAX_ITEM_COUNT_FOR_PAGE = 5,
		PAGE_COUNT_FOR_SCREEN = 10,	// Expected to remain unchanged
	};

	std::vector< MyAccountItem >					m_vecAccountList;

	eSortType	m_lastSortType;
	bool			m_lastAscending;

	struct SellListSorter {
		eSortType m_sortType;
		bool	m_bAscending;

		enum {
			MAGIC_LARGE_NUMBER = 1000000,
		};

		SellListSorter( eSortType sortType, bool bAscending ) : m_sortType( sortType ), m_bAscending( bAscending )  { ; }

		bool operator () ( MyAccountItem &lhs, MyAccountItem &rhs )		// implementation of Less Operator
		{
			bool bLess = false;
			switch( m_sortType )
			{
			case ST_NAME:
				bLess =  ( lhs.wszItemName < rhs.wszItemName ) ? m_bAscending : !m_bAscending;
				break;
			case ST_PRICE:
				bLess = ( lhs.Market.nPrice < rhs.Market.nPrice ) ? m_bAscending : !m_bAscending;
				break;
			case ST_SELLTIME:
				bLess = ( lhs.Market.tBuyDate < rhs.Market.tBuyDate ) ? m_bAscending : !m_bAscending;
				break;
			}
			return bLess;
		}
	};

	int				m_nPage;
	bool			m_bRequestListNeeded;
	std::set<int> m_IsNewPage;

	CDnMarketAboutDlg * m_pMarketAboutDlg;

	typedef CDnCustomDlg BaseClass;
	
private:
	void ProcessAccount();
	void RefreshPageControls();
	void UpdateListItems();
	int GetTotalPage();
	int GetStartPage();
	void SetMoneyControl( MONEY_TYPE nTotalMoney, TAX_TYPE nAccountTax, MONEY_TYPE nAccountMoney, MONEY_TYPE nTotalPetal );
	
public:
	void Initialize( bool bShow ) override;
	void InitialUpdate() override;
	void InitCustomControl( CEtUIControl *pControl ) override;
	void Show( bool bShow ) override;
	void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 ) override;	
#ifdef PRE_FIX_11027
	void RefreshAccount();
#endif
	void Process( float fElapsedTime ) override;

	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) override;
	void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 ) override;

public:
	void OnRecvUpdateList( std::vector< TMarketCalculationInfo > &vecCalculationList );
	void OnRecvRemoveItem( int nMarketDBID );
	void RemoveAllItem();	
	void RefreshRequestNeed(); 
};
