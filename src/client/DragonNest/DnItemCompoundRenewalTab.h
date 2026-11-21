#pragma once
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL

#include "DnCustomDlg.h"
#include "DnSetItemCompoundDlg.h"
#include "DnSuffixCompoundDlg.h"
#include "DnChandleryCompoundDlg.h"
#include "DnArtficeCompoundDlg.h"
#if defined(PRE_ADD_TALISMAN_SYSTEM)
#include "DnTalismanCompoundDlg.h"
#endif
#include "DnItemCompoundMessageDlg.h"
#include "DnItemCompoundListDlg.h"
#include "DnCompoundSelectMaterial.h"


class CDnItemCompoundRenewalTabDlg : public CEtUITabDialog, public CEtUICallback 
{
public:
	CDnItemCompoundRenewalTabDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemCompoundRenewalTabDlg();	

	enum 
	{
		MAX_COMPOUND_LIST = 7,
	};
	enum
	{
		COMPOUND_CONFIRM_DIALOG,
		COMPOUND_MOVIE_DIALOG,
		COMPOUND_SELECT_MATERIAL_DIALOG,
	};

protected:

	// Crafting message box 
	CDnItemCompoundMessageBox*	m_pItemCompoundMsgBox;
	CDnCompoundSelectMaterial*	m_pSelectMaterialDlg;
	INT64 m_nSelectedMaterialSerial;
	// Final compound group list filtered by tab option 
	std::vector<CDnCompoundBase::ItemGroupStruct*> m_pVecResultGroupList;
	
	
	// Options for filtering (0 or -1 if unchecked)
	int m_nSelectEquipType;
	int m_nSelectSortType; 
	int m_nSelectJobType;
	int m_nSelectSetItemType;
	int	m_nCompoundMainCategory;
	int m_nRemoteItemID;

	CEtUIRadioButton* m_pTabButton[CDnCompoundBase::MAX_COMPOUND_DIALOG_COUNT];
	
	
	// Adjust Max size. 
	std::vector< CDnItemCompoundListDlg* > m_VecItemList;
	
	CEtUIListBoxEx*	m_pListBoxEx;
	CDnItemCompoundListDlg* m_pCurrentCompoundListDlg;

	// Crafting window alert helper
	bool m_bForceOpenMode;

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	
	void UpdateGroupList( int nTabID = 0 );
	void RefreshList();
	
	void SetRemoteCompoundDlg( CDnCompoundBase::ItemGroupStruct& CompoundGroupData );
	void RefreshGroupList();
	void OnRecvItemCompoundOpen();
	void SetRemoteItemID( int nRemoteItemID );
	static bool SortByEnableList( int nLsh, int nRsh );
	
	void RefreshItemList(); 

	CDnItemCompoundListDlg* GetCurrentItemCompoundList()		{ return m_pCurrentCompoundListDlg; }
	CDnItemCompoundMessageBox* GetCompoundMsgBox()				{ return m_pItemCompoundMsgBox; }

	std::vector<CDnCompoundBase::ItemGroupStruct*>& GetCompoundList() { return m_pVecResultGroupList; }

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	void SetForceOpenMode()		{ m_bForceOpenMode = true; }
	void ResetForceOpenMode()	{ m_bForceOpenMode = false; }
#endif 

	int GetRemoteItemID() { return m_nRemoteItemID; }
	
	void Reset();
	void Clear();
	// Sort 
	void SortRank();
	void SortLevel();

};

#endif 