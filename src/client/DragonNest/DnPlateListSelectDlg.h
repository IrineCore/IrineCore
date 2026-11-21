#pragma once
#include "DnCustomDlg.h"
#include "EtUIDialogGroup.h"
#include "DnItemTask.h"

class CDnPlateListDlg;

class CDnPlateListSelectDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnPlateListSelectDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPlateListSelectDlg(void);

	struct RuneInfo{
		int m_nID[NUM_MAX_PLATE_COMPOUND_VARI];
		int m_nNum[NUM_MAX_PLATE_COMPOUND_VARI];
	};

	enum{
		SLOT_NUM = 5,
	};

private :

	CDnPlateListDlg *	m_pPlateListDlg;
	CDnItem	*			m_pNeedRune[SLOT_NUM];
	CDnItemSlotButton *	m_pNeedRuneSlotButton[SLOT_NUM];

	CEtUIButton *		m_pButtonNext;
	CEtUIButton *		m_pButtonPrev;
	CEtUIStatic *		m_pStaticPage;
	CEtUIStatic *		m_pStaticGold;
	CEtUIStatic *		m_pStaticSilver;
	CEtUIStatic *		m_pStaticBronze;
	DWORD				m_dwColorGold;
	DWORD				m_dwColorSilver;
	DWORD				m_dwColorBronze;

	int			m_nCurSelect;
	RuneInfo	m_RuneInfo[SLOT_NUM];
	int			m_pPlateInfo[NUM_MAX_PLATE_COMPOUND_VARI];
	std::vector<CDnItemTask::ItemCompoundInfo> m_vNeedItemInfo;

public:
	void RefreshPlateList();
	int GetSelectedCompoundID();

	int GetPlateID( int nIndex )	{ return m_pPlateInfo[nIndex]; }
	void SelectPlate( int nIndex );
	int * GetPlateInfo()			{ return m_pPlateInfo; }
	std::vector<CDnItemTask::ItemCompoundInfo> & GetCompoundInfo()	{ return m_vNeedItemInfo; }

	bool EnoughCoin();
	bool EnoughRune();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

/*
class CDnPlateListDlg;
class CDnPlateMixViewDlg;

class CDnPlateListSelectDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnPlateListSelectDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPlateListSelectDlg(void);

	enum
	{
		COMPOUND_CONFIRM_DIALOG,
COMPOUND_MOVIE_DIALOG, // In-progress movie cutscene dialog
	};

	enum emPlateStep
	{
		stepBase,
		stepPlateRegist,
		stepListSelect,
stepSlotItem, // Step for inserting jewel into slot
	};

	enum emSLOT_DLG_ID
	{
		// Up to 5 slot dialogs only.
		SLOT_DLG_00,
		SLOT_DLG_01,
		SLOT_DLG_02,
		SLOT_DLG_03,
		SLOT_DLG_04
	};

protected:

	emPlateStep m_emStep;

	// Plate item and slot.
	CDnItem *m_pItem;
	CDnItemSlotButton *m_pItemSlotButton;
	CEtUIStatic *m_pStaticFix;

	CEtUIButton *m_pButtonOK;
	CEtUIButton *m_pButtonCancel;

	CEtUIStatic *m_pStaticBack;

	CEtUIStatic *m_pStaticGold;
	CEtUIStatic *m_pStaticSilver;
	CEtUIStatic *m_pStaticBronze;

	// Since this also registers, need a QuickSlot pointer.
	CDnQuickSlotButton *m_pQuickSlotButton;

	// Child dialogs
	CDnPlateListDlg *m_pPlateListDlg;
	CEtUIDialogGroup m_SlotDlgGroup;
	CDnPlateMixViewDlg *m_pPlateMixViewDlg;
	//CDn

	// Dummy plate data
	// 62 000 000 1  No destruction
	// 62 000 000 2  Destroy jewel on failure
	// 62 000 000 3  Destroy plate on failure
	// 62 000 000 4  Destroy both on failure


	void RefreshPlateList();
	void ClosePlateList();
	void ShowPlateMixView();
	void ClosePlateMixView();

	// Check item type and count to put into jewel slot.
	void CheckSlotItems();

	bool IsEnableRegisterPlateItem(CDnSlotButton* pDragButton, CDnItem* pItem) const;

	// Required item struct to send to server
	vector<CDnItemTask::ItemCompoundInfo> m_vNeedItemInfo;
public:

	// Direct registration on right-click in inventory.
	void SetPlateItem( CDnQuickSlotButton *pPressedButton );
	int GetPlateItemID();

	void ChangeStep( emPlateStep emStep );


	void OnRecvEmblemCompoundOpen();
	void OnRecvEmblemCompound();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
};
*/