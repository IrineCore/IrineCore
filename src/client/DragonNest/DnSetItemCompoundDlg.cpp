#include "Stdafx.h"

#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
#include "DnSetItemCompoundDlg.h"
#include "DnLocalPlayerActor.h"
#include "DnItemCompoundRenewalTab.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnSetItemCompoundDlg::CDnSetItemCompoundDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCompoundBase( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
//, m_pCheckBoxRare(NULL)
//, m_pCheckBoxEpic(NULL)
//, m_pCheckBoxUnique(NULL)
//, m_pCheckBoxLegend(NULL)
, m_pComboBoxJob(NULL)
//, m_pTreeCtrlEquipment(NULL)
, m_pTreeCtrlSetItemList(NULL)
, m_nSelectJobType( -1 )
, m_bInitialize( true )
, m_bRemote( false )
{

}

CDnSetItemCompoundDlg::~CDnSetItemCompoundDlg()
{
	CompoundGroupListClear();
}

void CDnSetItemCompoundDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemCompound_Tab04Dlg.ui" ).c_str(), bShow );
}


void CDnSetItemCompoundDlg::InitialUpdate()
{
	//m_pCheckBoxRare   =  GetControl<CEtUICheckBox>("ID_CHECKBOX_RARE");
	//m_pCheckBoxEpic   =  GetControl<CEtUICheckBox>("ID_CHECKBOX_EPIC");
	//m_pCheckBoxUnique =  GetControl<CEtUICheckBox>("ID_CHECKBOX_UNIQUE");
	//m_pCheckBoxLegend  =  GetControl<CEtUICheckBox>("ID_CHECKBOX_MAGIC");
	
	m_pComboBoxJob    =  GetControl<CEtUIComboBox>("ID_COMBOBOX_CLASS");
		
	//m_pTreeCtrlEquipment = GetControl<CEtUITreeCtl>("ID_TREE_ITEM");
	m_pTreeCtrlSetItemList = GetControl<CEtUITreeCtl>("ID_TREE_LIST");

	//m_pStaticTextEquipList = GetControl<CEtUIStatic>("ID_TEXT_SETITEM");

	CEtUICheckBox* pCheckBox = NULL;
	CEtUIStatic* pStaticTextLevel= NULL;

	//m_pCheckBoxRare->SetChecked(true );
	//m_pCheckBoxEpic->SetChecked(true );
	//m_pCheckBoxUnique->SetChecked(true );
	//m_pCheckBoxLegend->SetChecked(true );

	//m_pCheckBoxLegend->Enable( false );
	//m_pCheckBoxRare->Enable( false );
	
	//CEtUIStatic* pTextMagic = GetControl<CEtUIStatic>("ID_TEXT_MAGIC");
	//pTextMagic->Enable( false );
	//CEtUIStatic* pTextRare = GetControl<CEtUIStatic>("ID_TEXT_RARE");
	//pTextRare->Enable( false );

	//for( int i = 0; i < MAXLEVEL; ++i )
	//{
	//	sprintf_s( szControlName, _countof( szControlName ), "ID_CHECKBOX_LEVEL%d", i );
	//	pCheckBox = GetControl<CEtUICheckBox>( szControlName );	
	//	pCheckBox->SetChecked( true );
	//	pCheckBox->Show( ( i >= 5 ) ? false : true );
	//	
	//	sprintf_s( szControlName, _countof( szControlName ), "ID_TEXT_LEVEL%d", i );
	//	pStaticTextLevel = GetControl<CEtUIStatic>( szControlName );	
	//	pStaticTextLevel->SetText( FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2164 ), ( ( i+2 ) * 10 ) ) );
	//	pStaticTextLevel->Show( ( i >= 5 ) ? false : true);

	//	// Set checkbox comparison data 
	//	if( i < 5 ) 
	//	{
	//		m_vecStaticLevel.push_back( pStaticTextLevel );
	//		m_vecCheckBoxLevel.push_back( pCheckBox );
	//		m_nVecLevelList.push_back( i < 3 ? ( i+3 ) * 8 : ( i+2 ) * 10 );    
	//	}
	//}

	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 46 ), NULL, 1 );
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 47 ), NULL, 2 );
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 49 ), NULL, 3 );
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 48 ), NULL, 4 );
#ifdef PRE_ADD_ACADEMIC
#ifndef PRE_REMOVE_ACADEMIC
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 305 ), NULL, 5 );
#endif
#endif
#if defined( PRE_ADD_KALI ) && !defined( PRE_REMOVE_KALI )
	m_pComboBoxJob->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 306 ), NULL, 6 );
#endif
//[OK]


	if( CDnActor::s_hLocalActor )
		m_pComboBoxJob->SetSelectedByIndex( CDnActor::s_hLocalActor->GetClassID() - 1 );
	
	//RefreshCheckBoxLevel( CDnActor::s_hLocalActor->GetLevel(), m_bRemote );

	//if( SetUpEquipList() )
		//RefreshEquipList();

	SetCompoundTable( CDnCompoundBase::COMPOUND_SETITEM_DIALOG );
	RefreshSetEquipList();
	SetSelectInterval();


	m_bInitialize = false;
}

bool CDnSetItemCompoundDlg::IsExistItem( int nExistitem )
{
	std::vector<FilterTypeStruct>::iterator itor = m_VecSetItemNameList.begin();
	std::vector<FilterTypeStruct>::iterator itorEnd =  m_VecSetItemNameList.end();
	FilterTypeStruct SetItem;
	for( ; itor != itorEnd; ++itor )
	{
		SetItem = (*itor);
		if( SetItem.nUniqueID == nExistitem )
			return true;
	}

	return false;
}

void CDnSetItemCompoundDlg::CompoundGroupListClear()
{
	for( DWORD i = 0; i < m_VecSetList.size(); ++i )
		SAFE_DELETE_VEC( m_VecSetList[i]->m_SetItemList );	
}

void CDnSetItemCompoundDlg::SetCompoundTable( int nType )
{
	CompoundGroupListClear();

	if( !CDnActor::s_hLocalActor ) return;
	if( nType != CDnCompoundBase::COMPOUND_SETITEM_DIALOG ) return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUNDGROUP );
	DNTableFileFormat* pCompoundSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	if( !pSox || !pCompoundSox ) return;

	std::vector<int> nVecList;
	pSox->GetItemIDListFromField( "_MainCategory", nType, nVecList );

	if( nVecList.empty() )
		return;

	//std::set<int> nSetList;
	//for( DWORD i = 0; i < nVecList.size(); ++i )
	//{
	//	nSetList.insert( nVecList[i] );
	//}
	//SetItemStruct* pSetListStruct ;
	//for( DWORD i=0; i < nSetList.size(); ++i )
	//{
	//	pSetListStruct = new SetItemStruct( i );
	//	if( pSetListStruct ) m_VecSetList.push_back( pSetListStruct );		
	//}
	
	int nSetItemListIndex = 0;
	
	// Pre-set the first set item ID from the table 
	int PreCompoundSetItemID = pSox->GetFieldFromLablePtr( nVecList[0], "_SetID" )->GetInteger();	

	SetItemStruct* pSetListStruct = NULL;
	
	// Preload for the number of jobs
	for( DWORD i = 0; i < PLAYER_MAX; ++i )
	{
		pSetListStruct =  new SetItemStruct( i + 1 );
		if( pSetListStruct ) m_VecSetList.push_back( pSetListStruct );
	}

	for( DWORD i=0; i<nVecList.size(); i++ )
	{
		CDnCompoundBase::ItemGroupStruct *pStruct = new CDnCompoundBase::ItemGroupStruct;
		pStruct->nGroupTableID = nVecList[i];
		pStruct->nCompoundMainCategory = pSox->GetFieldFromLablePtr( nVecList[i], "_MainCategory" )->GetInteger();
		pStruct->nCompoundGroupID = pSox->GetFieldFromLablePtr( nVecList[i], "_CompoundGroupID" )->GetInteger();			
		pStruct->nCompoundGroupType = pSox->GetFieldFromLablePtr( nVecList[i], "_CompoundGroupType" )->GetInteger();		
		pStruct->nPartsID = pSox->GetFieldFromLablePtr( nVecList[i], "_PartsNameID" )->GetInteger();							
		pStruct->nItemID = pSox->GetFieldFromLablePtr( nVecList[i], "_TooltipItemID" )->GetInteger();
		pStruct->nLevel = pSox->GetFieldFromLablePtr( nVecList[i], "_Level" )->GetInteger();
		pStruct->nRank = pSox->GetFieldFromLablePtr( nVecList[i], "_Rank" )->GetInteger();
		pStruct->nJob = pSox->GetFieldFromLablePtr( nVecList[i], "_JobClass" )->GetInteger();
		pStruct->nActivateLevel = pSox->GetFieldFromLablePtr( nVecList[i], "_ActivateLevel" )->GetInteger();
		pStruct->bIsPreview = pSox->GetFieldFromLablePtr( nVecList[i], "_PreviewState" )->GetBool();
		pStruct->nPreviewDescUIStringID = pSox->GetFieldFromLablePtr( nVecList[i], "_PreviewDescription" )->GetInteger();
		pStruct->nCompoundSetItemID = pSox->GetFieldFromLablePtr( nVecList[i], "_SetID" )->GetInteger();
	
		pCompoundSox->GetItemIDListFromField( "_CompoundGroupID", pStruct->nCompoundGroupID, pStruct->nVecCompoundList );
		
		if( ! IsExistItem( pStruct->nCompoundSetItemID ) )
		{
			FilterTypeStruct SetItem;			
			SetItem.nID = nVecList[i];
			SetItem.nLevel = pStruct->nLevel;
			SetItem.nRank = pStruct->nRank;
			SetItem.nUniqueID = pStruct->nCompoundSetItemID;
			m_VecSetItemNameList.push_back( SetItem );
		}			
	
		if( pStruct->nJob > 0 && ( pStruct->nJob - 1 ) < (int)m_VecSetList.size() )
			m_VecSetList[ pStruct->nJob - 1 ]->Add( pStruct );

	}	
}
void CDnSetItemCompoundDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnSetItemCompoundDlg::Reset()
{
	m_bRemote = false;
	m_nSelectJobType = ( CDnActor::s_hLocalActor->GetClassID() - 1 );
	m_pComboBoxJob->SetSelectedByIndex( m_nSelectJobType );

	//m_pCheckBoxRare->SetChecked( true, false );
	//m_pCheckBoxEpic->SetChecked( true, false );
	//m_pCheckBoxUnique->SetChecked( true, false );
	//m_pCheckBoxLegend->SetChecked( true, false );

	//m_Rank.nLegend = eLEGEND;
	//m_Rank.nRare = eRARE;
	//m_Rank.nEpic = eEPIC;
	//m_Rank.nUnique = eUNIQUE;
	
	//RefreshCheckBoxLevel( CDnActor::s_hLocalActor->GetLevel(), m_bRemote );	
	//RefreshEquipList();
	RefreshSetEquipList();
	SetSelectInterval();
}

//void CDnSetItemCompoundDlg::RefreshCheckBoxLevel( int nLevel, bool bRemote )
//{
//	for( DWORD i = 0; i < m_vecCheckBoxLevel.size(); ++i )
//	{
//		m_nVecLevelList[i] = i < 3 ? ( i+3 ) * 8 : ( i+2 ) * 10; 
//		if( bRemote )
//		{
//			if( m_nVecLevelList[i] == nLevel )
//			{
//				m_vecCheckBoxLevel[i]->SetChecked( true, false );
//			}
//			else
//			{
//				m_vecCheckBoxLevel[i]->SetChecked( false, false );
//				m_nVecLevelList[i] = 0;
//			}
//		}
//		else 
//		{
//			if( m_nVecLevelList[i] >= nLevel )
//			{
//				m_vecCheckBoxLevel[i]->SetChecked( true, false );
//			}
//			else
//			{
//				m_vecCheckBoxLevel[i]->SetChecked( false, false );
//				m_nVecLevelList[i] = 0;
//			}
//		}
//	}	
//}

void CDnSetItemCompoundDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		//if( IsCmdControl("ID_CHECKBOX_MAGIC") ||
		//	IsCmdControl("ID_CHECKBOX_RARE")  || 
		//	IsCmdControl("ID_CHECKBOX_UNIQUE") ||
		//	IsCmdControl("ID_CHECKBOX_EPIC") )
		//{
		//		m_Rank.nLegend = m_pCheckBoxLegend->IsChecked() ? eLEGEND : 0;
		//		m_Rank.nRare = m_pCheckBoxRare->IsChecked() ? eRARE : 0;
		//		m_Rank.nEpic = m_pCheckBoxEpic->IsChecked() ? eEPIC : 0;
		//		m_Rank.nUnique =  m_pCheckBoxUnique->IsChecked() ? eUNIQUE : 0;
		//	
		//	if( m_bRemote )
		//		Reset();
		//	else
		//		RefreshSetEquipList();
		//}

		//if( strstr( pControl->GetControlName(), "ID_CHECKBOX_LEVEL") )
		//{
		//	//for( DWORD i = 0; i < m_vecCheckBoxLevel.size(); ++i )
		//	//{
		//	// // Set item level increase: 24, 32, 40, 50, 60	
		//	//	if( m_vecCheckBoxLevel[i]->IsChecked() )	
		//	//		m_nVecLevelList[i] = i < 3 ? ( i+3 ) * 8 : ( i+2 ) * 10;   
		//	//	else 
		//	//	{
		//	//		m_nVecLevelList[i] = 0;
		//	//	}
		//	//}
		//	if( m_bRemote ) 
		//		Reset();
		//	else
		//		RefreshSetEquipList();
		//}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED ) 
	{
		if( IsCmdControl( "ID_COMBOBOX_CLASS" ) ) 
		{
			if( m_pComboBoxJob->GetSelectedIndex() != m_nSelectJobType )  
				m_nSelectJobType = m_pComboBoxJob->GetSelectedIndex(); 
			
			if( m_bRemote ) 
				Reset();
			else
			{
				//RefreshEquipList();
				RefreshSetEquipList();

				CDnItemCompoundRenewalTabDlg* pCompoundTabDlg = static_cast<CDnItemCompoundRenewalTabDlg*>( m_pParentDialog );
				CTreeItem* pSetItem;

				//pPartsItem  = m_pTreeCtrlEquipment->GetSelectedItem();
				pSetItem  = m_pTreeCtrlSetItemList->GetSelectedItem();
				if( /*pPartsItem &&*/ pCompoundTabDlg || pSetItem && pCompoundTabDlg ) 
				{
					pCompoundTabDlg->UpdateGroupList( CDnCompoundBase::COMPOUND_SETITEM_DIALOG );
				}
			}
		}
	}
	else if( nCommand == EVENT_TREECONTROL_SELECTION )
	{	
		//if( IsCmdControl("ID_TREE_ITEM") )
		//{
		//	RefreshSetEquipList();
		//}
		if( IsCmdControl("ID_TREE_LIST") )
		{		
			CDnItemCompoundRenewalTabDlg* pCompoundTabDlg = static_cast<CDnItemCompoundRenewalTabDlg*>( m_pParentDialog );
			CTreeItem* pSetItem;

			//pPartsItem  = m_pTreeCtrlEquipment->GetSelectedItem();
			pSetItem  = m_pTreeCtrlSetItemList->GetSelectedItem();
			if( /*pPartsItem &&*/ pCompoundTabDlg || pSetItem && pCompoundTabDlg ) 
			{
				pCompoundTabDlg->UpdateGroupList( CDnCompoundBase::COMPOUND_SETITEM_DIALOG );
			}	
		}
	}


	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//bool CDnSetItemCompoundDlg::SetUpEquipList()
//{	
//	for( DWORD i = 0; i< m_VecPartsItemList.size(); ++i )
//		m_VecPartsItemList[i].Reset();
//
//	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUNDGROUP );
//	if( ! pSox )
//		return false;	
//
//	std::vector<int> JobVecList;
//	for( DWORD i = 0; i < CDnSetItemCompoundDlg::PLAYER_MAX; ++i )
//	{
//		pSox->GetItemIDListFromField( "_JobClass", i + 1, JobVecList );
//		if( JobVecList.empty() )
//			continue;
//
//		PartsItemStruct PartsItem( i );
//
//		std::multimap<int, int> mutimapEquipList; 
//		int nMainID, nSubID;
//
//		for( DWORD i=0; i<JobVecList.size(); i++ )
//		{
//			nMainID = pSox->GetFieldFromLablePtr( JobVecList[i], "_PartsNameID" )->GetInteger();
//			nSubID = pSox->GetFieldFromLablePtr( JobVecList[i], "_CompoundGroupType" )->GetInteger();
//
//			if( PartsItem.mutimapEquipList.end() == PartsItem.mutimapEquipList.find( nMainID ) )
//			{
//				PartsItem.mutimapEquipList.insert( make_pair( nMainID, nSubID ) );	
//			}			
//		}
//
//		m_VecPartsItemList.push_back( PartsItem );
//	}
//
//	m_nSelectJobType = ( CDnActor::s_hLocalActor->GetClassID() - 1 );
//
//	return true;
//}
// Need a function for default settings... 

void CDnSetItemCompoundDlg::SetRemoteCompoundDlg( CDnCompoundBase::ItemGroupStruct& CompoundGroupData )
{
	m_nSelectJobType = ( CompoundGroupData.nJob - 1 ) > 0 ?  ( CompoundGroupData.nJob - 1 ) : ( CDnActor::s_hLocalActor->GetClassID() - 1 );
	m_pComboBoxJob->SetSelectedByIndex( m_nSelectJobType );
	
	m_bRemote = true;
	//m_Rank.nRare == CompoundGroupData.nRank ? m_pCheckBoxRare->SetChecked( true, false ) : m_pCheckBoxRare->SetChecked( false, false );
	//m_Rank.nEpic == CompoundGroupData.nRank ? m_pCheckBoxEpic->SetChecked( true, false ) : m_pCheckBoxEpic->SetChecked( false, false );
	//m_Rank.nUnique == CompoundGroupData.nRank ? m_pCheckBoxUnique->SetChecked( true, false ) : m_pCheckBoxUnique->SetChecked( false, false );
	//m_Rank.nLegend == CompoundGroupData.nRank ? m_pCheckBoxLegend->SetChecked( true, false ) : m_pCheckBoxLegend->SetChecked( false, false );
	
	//RefreshCheckBoxLevel( CompoundGroupData.nLevel, m_bRemote );
	//RefreshEquipList();
	RefreshSetEquipList();
	//SetEquipmentPartsControl( CompoundGroupData.nJob - 1 , CompoundGroupData.nPartsID  );
	SetEquipmentSetItemControl( CompoundGroupData.nCompoundSetItemID );
}

//void CDnSetItemCompoundDlg::SetEquipmentPartsControl( int nJob, int nPartsID )
//{
//	if( nPartsID < 0 || nJob < 0 || nJob >= PLAYER_MAX ) return;
//	if( m_VecPartsItemList.empty() || m_VecPartsItemList[ nJob ].mutimapEquipList.empty() ) return;
//	
//	CTreeItem* pSelectItem = NULL;
//
//	std::multimap<int, int>::iterator itor = m_VecPartsItemList[ nJob ].mutimapEquipList.find( nPartsID );
//	if( itor != m_VecPartsItemList[ nJob ].mutimapEquipList.end() )
//	{
//		if( GetTypeFromPartsID( itor->second ) == ITEMTYPE_WEAPON )
//			pSelectItem = m_pTreeCtrlEquipment->FindItemInt( itor->first );
//		else if ( GetTypeFromPartsID( itor->second ) == ITEMTYPE_PARTS )
//			pSelectItem = m_pTreeCtrlEquipment->FindItemInt( itor->first );
//
//		if( pSelectItem ) m_pTreeCtrlEquipment->SetSelectItem( pSelectItem );
//	}
//	
//}

void CDnSetItemCompoundDlg::SetEquipmentSetItemControl( int nEquippedItemID )
{
	if( nEquippedItemID < 0 ) return;
	
	CTreeItem* pSelectItem = NULL;

	for( int i = 0; i < (int)m_VecSetItemNameList.size(); ++i )
	{
		 if( m_VecSetItemNameList[i].nUniqueID == nEquippedItemID )
		 {
			pSelectItem = m_pTreeCtrlSetItemList->FindItemInt( m_VecSetItemNameList[i].nUniqueID );
			if( pSelectItem ) m_pTreeCtrlSetItemList->SetSelectItem( pSelectItem );
			break;
		 }
	}
	
	ProcessCommand( EVENT_TREECONTROL_SELECTION, false, m_pTreeCtrlSetItemList, 0 );
}


int CDnSetItemCompoundDlg::GetTypeFromPartsID( const int& nPartsID )
{
	switch( nPartsID )
	{
	case 5 : case 6 : case 9 : case 10 : case 11: 
		return ITEMTYPE_WEAPON;
	case 0 : case 1 : case 2 :  case 3 : case 4 :
		return ITEMTYPE_PARTS;
	default:
		return ITEMTYPE_NORMAL;
	}
}

//bool CDnSetItemCompoundDlg::CheckCompoundItemLevel( const int& nLevel )
//{
//	for( DWORD i = 0; i < m_nVecLevelList.size(); ++i )
//	{
//		if( m_nVecLevelList[i] == nLevel )
//			return true;
//	}
//	
//	return false;
//}

bool CDnSetItemCompoundDlg::RefreshSetEquipList()
{
	int nPreSelectedItemID = 0;
	if( m_pTreeCtrlSetItemList->GetSelectedItem() )
		nPreSelectedItemID = m_pTreeCtrlSetItemList->GetSelectedItem()->GetItemValueInt();

	m_pTreeCtrlSetItemList->DeleteAllItems();

	if( m_VecSetItemNameList.empty() )
		return false;

	//if( m_nVecLevelList.empty() )
	//	return false;

	//if( m_nSetItemLevel < 0 && m_nSetItemLevel > MAXLEVEL )
	//	return false;

	// int == job, SetItemStruct (item info) 

	//std::pair< std::multimap<int, SetItemListStruct>::iterator, std::multimap<int, SetItemListStruct>::iterator > range;
	//range = m_VecSetItemNameList.equal_range( m_nSelectJobType );

	//std::multimap<int, SetItemListStruct>::iterator ItorPos = range.first;


	//CTreeItem* pRare = m_pTreeCtrlSetItemList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2123 ) ); // 2123 : Rare crafted set
	//if( pRare )
	//{
	//	pRare->SetItemValueInt( 0 );
	//	pRare->SetTextColor( textcolor::GOLDENROD );
	//}
	//else return false;

	CTreeItem* pEpic = m_pTreeCtrlSetItemList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2124 ) ); // 2124 : Epic crafted set
	if( pEpic ) 
	{
		pEpic->SetItemValueInt( 1 );
		pEpic->SetTextColor( textcolor::GOLDENROD );
	}
	else return false;

	CTreeItem* pUnique = m_pTreeCtrlSetItemList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2125 ) ); // 2125 : Unique crafted set
	if( pUnique )
	{
		pUnique->SetItemValueInt( 2 );
		pUnique->SetTextColor( textcolor::GOLDENROD );
	}
	else return false;

	CTreeItem* pLegend = m_pTreeCtrlSetItemList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2126 ) ); // 2126 : Legend crafted set
	if( pLegend ) 
	{ 
		pLegend->SetItemValueInt( 3 );
		pLegend->SetTextColor( textcolor::GOLDENROD );
	}
	else return false;

	CTreeItem* pItem;
	FilterTypeStruct  SetItemData;
	
	std::vector<FilterTypeStruct>::iterator ItorPos = m_VecSetItemNameList.begin();
	std::vector<FilterTypeStruct>::iterator ItorEnd = m_VecSetItemNameList.end();
	for( ; ItorPos != ItorEnd ; ++ItorPos )
	{	
		SetItemData = (*ItorPos);

		//if( ( m_bInitialize == true ? m_bInitialize : CheckCompoundItemLevel( SetItemData.nLevel ) ) == false )
		//	continue;

		//if( SetItemData.nRank == m_Rank.nRare )
		//{
		//	pItem = m_pTreeCtrlSetItemList->AddChildItem( pRare, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, SetItemData.nUniqueID ) );
		//	pItem->SetItemValueInt( SetItemData.nUniqueID );	
		//}
		if( SetItemData.nRank == CDnCompoundBase::eEPIC )
		{
			pItem = m_pTreeCtrlSetItemList->AddChildItem( pEpic, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, SetItemData.nUniqueID ) );
			pItem->SetItemValueInt( SetItemData.nUniqueID );	
		}
		if( SetItemData.nRank == CDnCompoundBase::eUNIQUE )
		{
			pItem = m_pTreeCtrlSetItemList->AddChildItem( pUnique, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, SetItemData.nUniqueID ) );
			pItem->SetItemValueInt( SetItemData.nUniqueID );	
		}
		if( SetItemData.nRank == CDnCompoundBase::eLEGEND )
		{
			pItem = m_pTreeCtrlSetItemList->AddChildItem( pLegend, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, SetItemData.nUniqueID ) );
			pItem->SetItemValueInt( SetItemData.nUniqueID );	
		}
	}
	m_pTreeCtrlSetItemList->ExpandAll();

	if( nPreSelectedItemID > 0 )
	{
		CTreeItem* pPreSelectedItem = m_pTreeCtrlSetItemList->FindItemInt( nPreSelectedItemID );
		if( pPreSelectedItem ) m_pTreeCtrlSetItemList->SetSelectItem( pPreSelectedItem );
	}

	return true;
};

//bool CDnSetItemCompoundDlg::RefreshEquipList()
//{
//	m_pTreeCtrlEquipment->DeleteAllItems();
//
//	if( m_VecPartsItemList.empty() )
//		return false;
//
//	if( m_nSelectJobType < 0 || m_nSelectJobType >= CDnSetItemCompoundDlg::PLAYER_MAX )
//		return false;
//
//	std::multimap<int, int>::iterator ItorPos;
//	std::multimap<int, int>::iterator ItorEnd = m_VecPartsItemList[m_nSelectJobType].mutimapEquipList.end();
//
//	CTreeItem* pWeapon = m_pTreeCtrlEquipment->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1712 ) ); // 1712 : Weapon 
//	if( pWeapon ) 
//	{
//		pWeapon->SetItemValueInt( 0 );
//		pWeapon->SetTextColor( textcolor::GOLDENROD );
//	}
//	CTreeItem* pArmor = m_pTreeCtrlEquipment->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1713 ) ); // 1713 : Armor 
//	if( pArmor ) 
//	{
//		pArmor->SetItemValueInt( 1 );
//		pArmor->SetTextColor( textcolor::GOLDENROD );
//	}
//
//	CTreeItem* pItem;
//	int nEquipmentType = 0;
//	for( ItorPos =  m_VecPartsItemList[m_nSelectJobType].mutimapEquipList.begin(); ItorPos != ItorEnd; ++ItorPos )
//	{	
//		nEquipmentType = GetTypeFromPartsID( ItorPos->second );
//		if( nEquipmentType == ITEMTYPE_WEAPON && pWeapon->FindItemInt( ItorPos->first ) == NULL ) 
//		{
//			pItem = m_pTreeCtrlEquipment->AddChildItem( pWeapon, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, ItorPos->first ) );
//			pItem->SetItemValueInt( ItorPos->first );
//		}
//		else if( nEquipmentType == ITEMTYPE_PARTS && pArmor->FindItemInt( ItorPos->first ) == NULL )
//		{
//			pItem = m_pTreeCtrlEquipment->AddChildItem( pArmor, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, ItorPos->first ) );
//			pItem->SetItemValueInt( ItorPos->first );	
//		}
//	}
//	
//	// When item equipment lists are refreshed, the default setting is the weapon tab. 
//	if( ! m_bRemote ) 
//	{
//		m_pTreeCtrlEquipment->SetSelectItem( pWeapon ); 
//	}
//	m_pTreeCtrlEquipment->ExpandAll();
//	
//	return true;
//}

void CDnSetItemCompoundDlg::SetSelectInterval()
{
	bool bFind = false;
	int nLevel = CDnActor::s_hLocalActor->GetLevel();
	CTreeItem* pSelectItem = NULL;
	FilterTypeStruct SetItemName;

	for( int i = 0; i < (int)m_VecSetItemNameList.size(); ++i )
	{
		SetItemName = m_VecSetItemNameList[i];	

		if( nLevel < 32 && SetItemName.nLevel == 24 )  bFind = true;
		else if( nLevel >= 32 && nLevel < 40 && SetItemName.nLevel == 32 )  bFind = true;
		else if( nLevel >= 40 && nLevel < 50 && SetItemName.nLevel == 40 )  bFind = true;
		else if( nLevel >= 50 && nLevel < 60 && SetItemName.nLevel == 50 )  bFind = true;
		else if( nLevel >= 60 && SetItemName.nLevel == 60 )  bFind = true;
		
		if( bFind )
		{
			pSelectItem = m_pTreeCtrlSetItemList->FindItemInt( SetItemName.nUniqueID );
			if( pSelectItem ) m_pTreeCtrlSetItemList->SetSelectItem( pSelectItem );	break;
		}
	}
}

// The function below is triggered when clicking a tab in the craftable set equipment list. 
void CDnSetItemCompoundDlg::UpdateGroupList( std::vector< CDnCompoundBase::ItemGroupStruct* >& pVecResultList )
{
	if( m_nSelectJobType < 0  || 
		m_nSelectJobType >= (int)m_VecSetList.size() || 
		m_VecSetList.empty() )
		return;

	std::vector< CDnCompoundBase::ItemGroupStruct* > VecList = m_VecSetList[m_nSelectJobType]->m_SetItemList;

	CDnCompoundBase::ItemGroupStruct* pCompoundItem;
	//CTreeItem *pSelectedPartsItem = m_pTreeCtrlEquipment->GetSelectedItem();	
	CTreeItem* pSelectedSetItem = m_pTreeCtrlSetItemList->GetSelectedItem();
	
	//int nSelectControlType = pSelectedPartsItem->GetItemValueInt();

	if( /*pSelectedPartsItem == NULL ||*/ pSelectedSetItem == NULL )
		return;

	// No need to check grade here, because the tree control showing the set item list already
	// Set item lists that do not match the grade are filtered out. 

	// VecList is a list for one job, so skip job check 
	int nEquipmentType = 0;
	for( DWORD i = 0; i < VecList.size(); ++i )
	{
		pCompoundItem = VecList[i];
	
		if( ! pCompoundItem )
			continue;

		//if( pSelectedPartsItem->GetDepth() > 1 ) 
		//{
		//	if( pSelectedPartsItem && pSelectedPartsItem->GetItemValueInt() != pCompoundItem->nPartsID )
		//		continue;
		//}
		//else 
		//{
		//	nEquipmentType = GetTypeFromPartsID(  pCompoundItem->nCompoundGroupType );
		//	if( nSelectControlType != nEquipmentType ) continue;
		//}

		// Check set type 
		if( pSelectedSetItem && pSelectedSetItem->GetItemValueInt() != pCompoundItem->nCompoundSetItemID )
			continue;

		//// Check level at the very end.. 
		//if( ! CheckCompoundItemLevel( pCompoundItem->nLevel ) )
		//	continue;
	
		pCompoundItem->nVecResultCompoundList = pCompoundItem->nVecCompoundList;

		pVecResultList.push_back( pCompoundItem );
	}
}

#endif 








