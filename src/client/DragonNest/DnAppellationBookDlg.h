#pragma once
#include "DnCustomDlg.h"


#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL

struct stUIAppellationItem
{
	CEtUIStatic* m_StaticIn;			// Title owned
	CEtUIStatic* m_StaticOut;			// Title not owned
	CEtUIStatic* m_StaticNewMark;		// New mark for newly acquired items

	CEtUIStatic* m_StaticInfo[2];		// Previous title description
	CEtUIStatic* m_StaticNewInfo[2];	// New title description
};

#define MAX_APPITEM_PER_PAGE	10

struct stUIAppellationPage
{
	CEtUIStatic* m_StaticTitle;			// Collection heading
	CEtUIStatic* m_StaticDescription;	// Collection description
	CEtUIStatic* m_StaticPercent;		// Collection completion rate
	CEtUIStatic* m_StaticReward;		// Collection reward

	CEtUIStatic*		m_pStaticClearMark;		// Clear stamp mark when all collected
	CDnItemSlotButton*	m_pStaticItemSlot[2];	// Item slot.
	CEtUIStatic*		m_pStaticItemCoin[2];	// Item Coin

	stUIAppellationItem m_AppellationItem[ MAX_APPITEM_PER_PAGE ];
};

class CDnMissionAppellationTooltipDlg;
class CDnAppellationRewardCoinTooltipDlg;
class CDnAppellationBookDlg : public CDnCustomDlg
{
public:
	CDnAppellationBookDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnAppellationBookDlg( void );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	void SetMaxPage( int nMaxPage ) { m_nMaxPage = nMaxPage-1; }
	void SetPageUI( int nPage );							// Fetch and set the page collection info
	void SetCurrentPageUI() { SetPageUI( m_nCurrentPage ); }
	void ClearPageUI();
	void ShowAppellationTooltip( float fMouseX, float fMouseY, int i, int j = -1 );

private:
	enum
	{
		E_IN = 0 ,
		E_OUT = 1,
		E_MAX
	};
	int					m_nCurrentPage;						// Current page
	int					m_nMaxPage;							// Max page count
	CEtUIStatic*		m_pStaticCurrentPage;
	stUIAppellationPage	m_stUIAppellationPage;				// Collection of page display UI controls
	CDnMissionAppellationTooltipDlg* m_pAppellationTooltip;	// Title tooltip

	
	// New control
	CEtUIListBoxEx*		m_pListBoxEx;			// List box
	bool				m_bIsSelectListBox;		// Is list box selected?
	bool				m_bIsHaveCoinReward;	// Is there Coin as a reward?
	int					m_nRewardCoinValue;

	CDnAppellationRewardCoinTooltipDlg* m_pRewardCoinToolTipDlg;

public:
	// New function
	void SetContents();
	void SelectContents(int nIndex);
	void SetAppellationToolTip(std::map<int, int>::iterator it, std::wstring& wszTooltip, bool bIsOpenAppellation);
	CEtUIListBoxEx* GetListBoxPtr()	{ return m_pListBoxEx; }
};


#else


struct stUIAppellationItem
{
	CEtUIStatic* m_StaticIn;			// Title owned
	CEtUIStatic* m_StaticOut;			// Title not owned
	CEtUIStatic* m_StaticInfo;			// Title description
};

#define MAX_APPITEM_PER_PAGE	10

struct stUIAppellationPage
{
	CEtUIStatic* m_StaticTitle;			// Collection heading
	CEtUIStatic* m_StaticDescription;	// Collection description
	CEtUIStatic* m_StaticPercent;		// Collection completion rate
	CEtUIStatic* m_StaticReward;		// Collection reward

	stUIAppellationItem m_AppellationItem[ MAX_APPITEM_PER_PAGE ];
};

class CDnMissionAppellationTooltipDlg;

class CDnAppellationBookDlg : public CDnCustomDlg
{
public:
	CDnAppellationBookDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnAppellationBookDlg( void );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	void SetMaxPage( int nMaxPage ) { m_nMaxPage = nMaxPage; }
	void SetPageUI( int nPage );							// Fetch and set the page collection info
	void SetCurrentPageUI() { SetPageUI( m_nCurrentPage ); }
	void ClearPageUI();
	void ShowAppellationTooltip( float fMouseX, float fMouseY, int i, int j = -1 );

private:
	int					m_nCurrentPage;						// Current page
	int					m_nMaxPage;							// Max page count
	CEtUIStatic*		m_pStaticCurrentPage1;
	CEtUIStatic*		m_pStaticCurrentPage2;
	stUIAppellationPage	m_stUIAppellationPage[2];			// Collection of page display UI controls
	CDnMissionAppellationTooltipDlg* m_pAppellationTooltip;	// Title tooltip
};


#endif //PRE_MOD_APPELLATIONBOOK_RENEWAL
