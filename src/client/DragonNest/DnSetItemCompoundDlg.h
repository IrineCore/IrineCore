#pragma once

#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL

#include "EtUITabDialog.h"
#include "DnCompoundBase.h"

class CDnSetItemCompoundDlg : public CDnCompoundBase
{
public:
	CDnSetItemCompoundDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnSetItemCompoundDlg();
	
protected:
	
	//CEtUICheckBox*  m_pCheckBoxRare;
	//CEtUICheckBox*  m_pCheckBoxEpic;
	//CEtUICheckBox*  m_pCheckBoxUnique;
	//CEtUICheckBox*  m_pCheckBoxLegend;

	CEtUIComboBox*	m_pComboBoxJob;
	//CEtUIStatic*	m_pStaticTextEquipList;
	
	//std::vector<CEtUICheckBox*>	 m_vecCheckBoxLevel;
	//std::vector<CEtUIStatic*>	 m_vecStaticLevel;

	//CEtUITreeCtl*	m_pTreeCtrlEquipment;
	CEtUITreeCtl*   m_pTreeCtrlSetItemList;
	
	//std::vector<PartsItemStruct> m_VecPartsItemList;
	std::vector<FilterTypeStruct> m_VecSetItemNameList;
	//std::vector< int > m_nVecLevelList;
	
	// Set item list loaded from the table 
	std::vector< SetItemStruct* > m_VecSetList;


	// Options for filtering (0 or -1 if unchecked)
	//int m_nSelectSortType; 
	int m_nSelectJobType;

	//RankStruct m_Rank;
	bool	m_bInitialize;
	bool	m_bRemote;

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	virtual void SetCompoundTable( int nType );
	virtual void UpdateGroupList( std::vector< CDnCompoundBase::ItemGroupStruct* >& pVecResultList );
	virtual void SetRemoteCompoundDlg( CDnCompoundBase::ItemGroupStruct& CompoundGroupData );
	virtual void Reset();


	//bool SetUpEquipList();

	void CompoundGroupListClear();
	void SetSelectInterval();
	//bool RefreshEquipList();
	bool RefreshSetEquipList();
	//void RefreshCheckBoxLevel( int nLevel, bool bRemote );

	bool IsExistItem( int nExistitem );
	int  GetTypeFromPartsID( const int& nPartsID );
	bool CheckCompoundItemLevel( const int& nLevel );

	//void SetEquipmentPartsControl( int nJob, int nPartsID );
	void SetEquipmentSetItemControl( int nEquippedItemID );	


};

#endif 