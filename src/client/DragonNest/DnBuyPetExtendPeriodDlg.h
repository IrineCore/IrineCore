#pragma once

#include "DnCustomDlg.h"


#ifdef PRE_ADD_PET_EXTEND_PERIOD

class CDnBuyConfirmPetExtendPeriodDlg : public CDnCustomDlg
{
public:
	CDnBuyConfirmPetExtendPeriodDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnBuyConfirmPetExtendPeriodDlg();

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );

public:
	void SetBuyInfo( int nPetLevel, std::wstring strPetName, std::wstring strPetExtendPeriodItemName );
};

class CDnBuyPetExtendPeriodDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnBuyPetExtendPeriodDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnBuyPetExtendPeriodDlg();

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	void SetPetSerialID( INT64 nPetSerialID ) { m_nPetSerialID = nPetSerialID; }

protected:
	void RefreshPetExtendPeriodItemList();
	void PrevPage();
	void NextPage();
	void SetItemPage( int nCurrentPage );

protected:
	enum{ BUYCONFIRM_PETEXTENDPERIODITEM = 0 };
	CDnBuyConfirmPetExtendPeriodDlg* m_pDnBuyConfirmPetExtendPeriodDlg;
	INT64					m_nPetSerialID;
	CDnItemSlotButton*		m_pPetExtendPeriodItemSlot[5];
	std::vector<CDnItem*>	m_vecpPetExtendPeriodItem;
	CEtUIStatic*			m_pSelectStatic[5];

	CEtUIButton*	m_pPrevButton;
	CEtUIButton*	m_pNextButton;
	CEtUIStatic*	m_pStaticPage;

	int				m_nTotalPage;
	int				m_nCurrentPage;
	int				m_nSelectedItemIndex;
};

#endif // PRE_ADD_PET_EXTEND_PERIOD
