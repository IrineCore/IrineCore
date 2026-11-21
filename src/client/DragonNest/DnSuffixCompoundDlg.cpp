#include "Stdafx.h"
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
#include "DnSuffixCompoundDlg.h"
#include "DnLocalPlayerActor.h"
#include "DnItemCompoundRenewalTab.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnSuffixCompoundDlg::CDnSuffixCompoundDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor)
: CDnCompoundBase( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
, m_pCheckBoxRare(NULL)
, m_pCheckBoxEpic(NULL)
, m_pCheckBoxUnique(NULL)
, m_pComboBoxJob(NULL)
, m_pComboBoxGrade( NULL )
, m_pTreeCtrlEquipment(NULL)
, m_nSelectJobType( -1 )
, m_nSelectSuffixGrade( -1 )
, m_bInitialize( true )
, m_bRemote( false )
{
}

CDnSuffixCompoundDlg::~CDnSuffixCompoundDlg()
{
	DeleteAllCheckBox();
	CompoundGroupListClear();
}

void CDnSuffixCompoundDlg::CompoundGroupListClear()
{
	for( DWORD i = 0; i < m_VecSetList.size(); ++i )
		SAFE_DELETE_VEC( m_VecSetList[i]->m_SetItemList );	
}


void CDnSuffixCompoundDlg::InitialUpdate()
{
	m_pCheckBoxRare   =  GetControl<CEtUICheckBox>("ID_CHECKBOX_RARE");
	m_pCheckBoxEpic   =  GetControl<CEtUICheckBox>("ID_CHECKBOX_EPIC");
	m_pCheckBoxUnique =  GetControl<CEtUICheckBox>("ID_CHECKBOX_UNIQUE");

	CEtUICheckBox*  pCheckBoxMagic = GetControl<CEtUICheckBox>("ID_CHECKBOX_MAGIC");
	pCheckBoxMagic->SetChecked(true, false);
	pCheckBoxMagic->Enable( false );
	CEtUIStatic* pTextMagic = GetControl<CEtUIStatic>("ID_TEXT_MAGIC");
	pTextMagic->Enable(false);


	m_pComboBoxJob    =  GetControl<CEtUIComboBox>("ID_COMBOBOX_CLASS");
	m_pComboBoxGrade    =  GetControl<CEtUIComboBox>("ID_COMBOBOX_GRADE");
	
	m_pTreeCtrlEquipment = GetControl<CEtUITreeCtl>("ID_TREE_ITEM");

	char szControlName[48];

	CEtUICheckBox* pCheckBox = NULL;
	CEtUIStatic* pStaticText= NULL;

	m_pCheckBoxRare->SetChecked(true );
	m_pCheckBoxEpic->SetChecked(true );
	m_pCheckBoxUnique->SetChecked(true );

	for( int i = 0; i < MAXLEVEL; ++i )
	{
		sprintf_s( szControlName, _countof( szControlName ), "ID_CHECKBOX_LEVEL%d", i );
		pCheckBox = GetControl<CEtUICheckBox>( szControlName );	
		pCheckBox->SetChecked( true );
		pCheckBox->Show( ( i >= 7 ) ? false : true );

		sprintf_s( szControlName, _countof( szControlName ), "ID_TEXT_LEVEL%d", i );
		pStaticText = GetControl<CEtUIStatic>( szControlName );	
		pStaticText->SetText( FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2164 ), (  i < 3 ? ( i+3 ) * 8 : ( i+6 ) * 5 ) ) );
		pStaticText->Show(( i >= 7 ) ? false : true);

		if( i < 7) 
		{
			m_vecCheckBoxLevel.push_back( pCheckBox );
			m_vecStaticLevel.push_back( pStaticText );
			m_nVecLevelList.push_back( i < 3 ? ( i+3 ) * 8 : ( i+6 ) * 5 );    
		}
	}

	CheckSuffixStruct* pSuffixStruct;
	for( int i =0; i < MAXSUFFIXSIZE; ++i )
	{
		pSuffixStruct = new CheckSuffixStruct();

		sprintf_s( szControlName, _countof( szControlName ), "ID_CHECKBOX_OPTION%d", i );
		pCheckBox = GetControl<CEtUICheckBox>( szControlName );
		pCheckBox->SetChecked( false );
		pCheckBox->Show( false );
	
		sprintf_s( szControlName, _countof( szControlName ), "ID_TEXT_OPTION%d", i );
		pStaticText = GetControl<CEtUIStatic>( szControlName );
		pStaticText->Show( false );

		pSuffixStruct->pCheckBoxSuffix = pCheckBox;
		pSuffixStruct->pStaticName = pStaticText;

		m_VecCheckBoxSuffix.push_back( pSuffixStruct );
	}
	
	int nPvPIndex = MAXSUFFIXSIZE;
	for( int i = 0; i < MAXPVPSUFFIXSIZE; ++i )
	{
		pSuffixStruct = new CheckSuffixStruct();
		sprintf_s( szControlName, _countof( szControlName ), "ID_CHECKBOX_PVPOPTION%d", i );
		pCheckBox = GetControl<CEtUICheckBox>( szControlName );
		pCheckBox->SetChecked( false );
		pCheckBox->Show( false );
	
		sprintf_s( szControlName, _countof( szControlName ), "ID_TEXT_PVPOPTION%d", i );
		pStaticText = GetControl<CEtUIStatic>( szControlName );
		pStaticText->Show( false );

		pSuffixStruct->pCheckBoxSuffix = pCheckBox;
		pSuffixStruct->pStaticName = pStaticText;

		m_VecCheckBoxSuffix.push_back( pSuffixStruct );
	}
	
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

	// Suffix filter 
	m_pComboBoxGrade->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2134), NULL, 1001 );	// 1st tier
	m_pComboBoxGrade->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2135), NULL, 2001 );	// 2nd tier
	m_pComboBoxGrade->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2136), NULL, 3001 );	// 3rd 
	
	if( CDnActor::s_hLocalActor )
		m_pComboBoxJob->SetSelectedByIndex( CDnActor::s_hLocalActor->GetClassID() - 1 );
	
	RefreshCheckBoxLevel( CDnActor::s_hLocalActor->GetLevel(), m_bRemote );

	if( SetUpEquipList() )
	{
		RefreshEquipList();
	}

	// Set normal suffix 
	m_pComboBoxGrade->SetSelectedByIndex( 0 );

	SetCompoundTable( CDnCompoundBase::COMPOUND_SUFFIX_DIALOG );
	
	RefreshSuffixList();

	m_bInitialize = false;

}
void CDnSuffixCompoundDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemCompound_Tab02Dlg.ui" ).c_str(), bShow );
}

bool CDnSuffixCompoundDlg::SetUpEquipList()
{	
	for( DWORD i = 0; i< m_VecPartsItemList.size(); ++i )
		m_VecPartsItemList[i].Reset();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUNDGROUP );
	if( ! pSox )
		return false;	

	std::vector<int> JobVecList;
	for( DWORD i = 0; i < CDnSetItemCompoundDlg::PLAYER_MAX; ++i )
	{
		pSox->GetItemIDListFromField( "_JobClass", i + 1, JobVecList );
		if( JobVecList.empty() )
			continue;

		PartsItemStruct PartsItem( i );

		std::multimap<int, int> mutimapEquipList; 
		int nMainID, nSubID;

		for( DWORD i=0; i<JobVecList.size(); i++ )
		{
			nMainID = pSox->GetFieldFromLablePtr( JobVecList[i], "_PartsNameID" )->GetInteger();
			nSubID = pSox->GetFieldFromLablePtr( JobVecList[i], "_CompoundGroupType" )->GetInteger();

			if( PartsItem.mutimapEquipList.end() == PartsItem.mutimapEquipList.find( nMainID ) )
			{
				PartsItem.mutimapEquipList.insert( make_pair( nMainID, nSubID ) );	
			}			
		}

		m_VecPartsItemList.push_back( PartsItem );
	}

	m_nSelectJobType = ( CDnActor::s_hLocalActor->GetClassID() - 1 );

	return true;
}


bool CDnSuffixCompoundDlg::CheckCompoundItemLevel( const int& nLevel )
{
	for( DWORD i = 0; i < m_nVecLevelList.size(); ++i )
	{
		if( m_nVecLevelList[i] == nLevel )
			return true;
	}

	return false;
}

// Suffix has a unique ID, so identify only by ID value whether 1st or 2nd 
bool CDnSuffixCompoundDlg::CheckCompoundSuffixID( const int& nSuffixID )
{
	// Filtering needed by suffix grade 
	int nSize = (int)m_VecCheckBoxSuffix.size();
	for( int i = 0; i < nSize; ++i )
	{	
		if( m_VecCheckBoxSuffix[i]->pCheckBoxSuffix->IsChecked() && m_VecCheckBoxSuffix[i]->nID == nSuffixID )
		{
			return true;
		}
	}

	return false;
}
void CDnSuffixCompoundDlg::RefreshCheckBoxLevel( int nLevel, bool bRemote )
{
	for( DWORD i = 0; i < m_vecCheckBoxLevel.size(); ++i )
	{
		m_nVecLevelList[i] = i < 3 ? ( i+3 ) * 8 : ( i+6 ) * 5;
		if( bRemote )
		{
			if( m_nVecLevelList[i] == nLevel )
			{
				m_vecCheckBoxLevel[i]->SetChecked( true, false );
			}
			else
			{
				m_vecCheckBoxLevel[i]->SetChecked( false, false );
				m_nVecLevelList[i] = 0;
			}
		}
		else 
		{
			if( m_nVecLevelList[i] >= nLevel )
			{
				m_vecCheckBoxLevel[i]->SetChecked( true, false );
			}
			else
			{
				m_vecCheckBoxLevel[i]->SetChecked( false, false );
				m_nVecLevelList[i] = 0;
			}
		}
	}
}

void CDnSuffixCompoundDlg::SetRemoteCompoundDlg( CDnCompoundBase::ItemGroupStruct& CompoundGroupData )
{
	m_nSelectJobType = CompoundGroupData.nJob - 1;
	m_pComboBoxJob->SetSelectedByIndex( m_nSelectJobType );
	m_pComboBoxGrade->SetSelectedByValue( CompoundGroupData.nSuffixGrade );

	m_bRemote = true;

	// Job, level, grade, selected options... 
	if( CDnCompoundBase::eRARE == CompoundGroupData.nRank )
	{
		m_pCheckBoxRare->SetChecked( true, false );
		m_Rank.nRare = CDnCompoundBase::eRARE;
	}
	else
	{
		m_pCheckBoxRare->SetChecked( false, false );
		m_Rank.nRare = 0;
	}
	if( CDnCompoundBase::eEPIC == CompoundGroupData.nRank )
	{
		m_pCheckBoxEpic->SetChecked( true, false );
		m_Rank.nEpic = CDnCompoundBase::eEPIC;
	}
	else
	{
		m_pCheckBoxEpic->SetChecked( false, false );
		m_Rank.nEpic = 0;
	}
	if( CDnCompoundBase::eUNIQUE == CompoundGroupData.nRank )
	{
		m_pCheckBoxUnique->SetChecked( true, false );
		m_Rank.nUnique = CDnCompoundBase::eUNIQUE;
	}
	else
	{
		m_pCheckBoxUnique->SetChecked( false, false );
		m_Rank.nUnique = 0;
	}
	

	m_RemoteCompoundItem = CompoundGroupData;

	RefreshCheckBoxLevel( CompoundGroupData.nLevel, m_bRemote );
	RefreshEquipList();
	SetEquipmentPartsControl( m_nSelectJobType , CompoundGroupData.nPartsID  );
	RefreshSuffixList();
	//SetEquipmentSetItemControl( CompoundGroupData.nCompoundSetItemID );

}

void CDnSuffixCompoundDlg::SetEquipmentPartsControl( int nJob, int nPartsID )
{
	if( nPartsID < 0 || nJob < 0 || nJob >= PLAYER_MAX ) return;
	if( m_VecPartsItemList.empty() || m_VecPartsItemList[ nJob ].mutimapEquipList.empty() ) return;

	CTreeItem* pSelectItem = NULL;

	std::multimap<int, int>::iterator itor = m_VecPartsItemList[ nJob ].mutimapEquipList.find( nPartsID );
	if( itor != m_VecPartsItemList[ nJob ].mutimapEquipList.end() )
	{
		if( GetTypeFromPartsID( itor->second ) == ITEMTYPE_WEAPON )
			pSelectItem = m_pTreeCtrlEquipment->FindItemInt( itor->first );
		else if ( GetTypeFromPartsID( itor->second ) == ITEMTYPE_PARTS )
			pSelectItem = m_pTreeCtrlEquipment->FindItemInt( itor->first );

		if( pSelectItem ) m_pTreeCtrlEquipment->SetSelectItem( pSelectItem );
	}
}


// Must be called only when newly updated 
bool CDnSuffixCompoundDlg::RefreshEquipList()
{
	m_pTreeCtrlEquipment->DeleteAllItems();

	if( m_VecPartsItemList.empty() )
		return false;

	if( m_nSelectJobType < 0 || m_nSelectJobType >= CDnSetItemCompoundDlg::PLAYER_MAX )
		return false;

	std::multimap<int, int>::iterator ItorPos;
	std::multimap<int, int>::iterator ItorEnd = m_VecPartsItemList[m_nSelectJobType].mutimapEquipList.end();

	CTreeItem* pWeapon = m_pTreeCtrlEquipment->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1712 ) ); // 1712 : Weapon 
	if( pWeapon ) 
	{
		pWeapon->SetItemValueInt( 0 );
		pWeapon->SetTextColor( textcolor::GOLDENROD );
	}
	CTreeItem* pArmor = m_pTreeCtrlEquipment->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1713 ) ); // 1713 : Armor 
	if( pArmor ) 
	{
		pArmor->SetItemValueInt( 1 );
		pArmor->SetTextColor( textcolor::GOLDENROD );
	}

	CTreeItem* pItem;

	CTreeItem* pDefaultItem;
	bool bFirst = true;

	int nEquipmentType = 0;
	for( ItorPos =  m_VecPartsItemList[m_nSelectJobType].mutimapEquipList.begin(); ItorPos != ItorEnd; ++ItorPos )
	{	
		nEquipmentType = GetTypeFromPartsID( ItorPos->second );
		if( nEquipmentType == ITEMTYPE_WEAPON && pWeapon->FindItemInt( ItorPos->first ) == NULL ) 
		{
			pItem = m_pTreeCtrlEquipment->AddChildItem( pWeapon, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, ItorPos->first ) );
			if( bFirst ) pDefaultItem = pItem;
			bFirst = false;
			pItem->SetItemValueInt( ItorPos->first );
		}
		else if( nEquipmentType == ITEMTYPE_PARTS && pArmor->FindItemInt( ItorPos->first ) == NULL )
		{
			pItem = m_pTreeCtrlEquipment->AddChildItem( pArmor, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, ItorPos->first ) );
			pItem->SetItemValueInt( ItorPos->first );	
		}
	}

	// When item equipment lists are refreshed, the default selection is the weapon tab. 
	if( ! m_bRemote )
	{
		m_pTreeCtrlEquipment->SetSelectItem( pDefaultItem ); 
	}
	m_pTreeCtrlEquipment->ExpandAll();

	return true;
}


void CDnSuffixCompoundDlg::SetCompoundTable( int nType )
{
	CompoundGroupListClear();

	if( !CDnActor::s_hLocalActor ) return;
	if( nType != CDnCompoundBase::COMPOUND_SUFFIX_DIALOG ) return;	

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUNDGROUP );
	DNTableFileFormat* pCompoundSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	if( !pSox || !pCompoundSox ) return;

	std::vector<int> nVecList;
	pSox->GetItemIDListFromField( "_MainCategory", nType, nVecList );

	if( nVecList.empty() )
		return;

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
		pStruct->nSuffixGrade = pSox->GetFieldFromLablePtr( nVecList[i], "_ListID" )->GetInteger();
	
		pCompoundSox->GetItemIDListFromField( "_CompoundGroupID", pStruct->nCompoundGroupID, pStruct->nVecCompoundList );

		if( pStruct->nJob > 0 && ( pStruct->nJob - 1 ) < (int)m_VecSetList.size() )
			m_VecSetList[ pStruct->nJob - 1 ]->Add( pStruct );
	}

}
void CDnSuffixCompoundDlg::ResetCheckBox()
{
	int nSize = (int)m_VecCheckBoxSuffix.size();
	for( int i = 0; i < nSize; ++i )
	{
		if( m_VecCheckBoxSuffix[i] ) 
		{
			m_VecCheckBoxSuffix[i]->Reset();
			m_VecCheckBoxSuffix[i]->Show(false);
			//SAFE_DELETE( pCheckStruct );
		}
	}
}

void CDnSuffixCompoundDlg::DeleteAllCheckBox()
{
	int nSize = (int)m_VecCheckBoxSuffix.size();
	for( int i = 0; i < nSize; ++i )
	{
		if( m_VecCheckBoxSuffix[i] ) 
		{
			SAFE_DELETE( m_VecCheckBoxSuffix[i] );
		}
	}
}

bool CDnSuffixCompoundDlg::IsExistSuffixCheckBox( int nID )
{
	int nSize = (int)m_VecCheckBoxSuffix.size();
	for( int i = 0; i < nSize; ++i )
	{
		if( m_VecCheckBoxSuffix[i]->nID == nID ) 
			return true;
	}

	return false;
}

// Lists filtered by control states... 
void CDnSuffixCompoundDlg::MakeSuffixCheckBox( std::vector< CDnCompoundBase::ItemGroupStruct* >& pVecResultList ) 
{
	ResetCheckBox();

	if( pVecResultList.empty() )
		return;
		
	DNTableFileFormat* pCompoundSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	if(  !pCompoundSox ) return;
	
	int	nVecSize = (int)pVecResultList.size();
	int nCheckBoxSize = (int)m_VecCheckBoxSuffix.size();
	int nPVP = MAXSUFFIXSIZE;
	int nNormal = 0;
	int nSuffixID = 0;
	int nDescriptionID = 0;
	bool bKind = false;

	CDnCompoundBase::ItemGroupStruct* pCompoundData = NULL;
	std::map< int, CheckSuffixStruct* >::iterator ItorPos;
	CheckSuffixStruct* pCheckSuffix = NULL;
	
	for( int i = 0; i < nVecSize; ++i )
	{
		pCompoundData = pVecResultList[i];	
		if( ! pCompoundData )
			continue;
		
		//nNormal = 0;
		//nPVP = MAXSUFFIXSIZE;
		for( int j = 0; j < (int)pCompoundData->nVecCompoundList.size(); ++j )
		{	
			nSuffixID = pCompoundSox->GetFieldFromLablePtr ( pCompoundData->nVecCompoundList[j], "_Explanation" )->GetInteger();
			nDescriptionID = pCompoundSox->GetFieldFromLablePtr ( pCompoundData->nVecCompoundList[j], "_DescriptionID" )->GetInteger();
			bKind = ( pCompoundSox->GetFieldFromLablePtr ( pCompoundData->nVecCompoundList[j], "_Kind" )->GetInteger() == 1000001412 ? true : false );
			// Check if the control exists... 
			if( IsExistSuffixCheckBox( nSuffixID ) )
				continue;
		
			if( bKind ) // 1000001412 : MID : PVP 
			{
				if( nPVP >= MAXSUFFIXSIZE + MAXPVPSUFFIXSIZE ) 
					continue;

				pCheckSuffix = m_VecCheckBoxSuffix[ nPVP++ ];
				pCheckSuffix->bPVP  = true;
			}
			else	// 501 : MID : Normal 
			{		
				if( nNormal >= MAXSUFFIXSIZE )
					continue;

				pCheckSuffix = m_VecCheckBoxSuffix[ nNormal++ ];
				pCheckSuffix->bPVP  = false;
			}

			pCheckSuffix->nID = nSuffixID;		
			pCheckSuffix->nEquipType = GetTypeFromPartsID( pCompoundData->nCompoundGroupType );
			pCheckSuffix->nGroupID = pCompoundData->nVecCompoundList[j];
			pCheckSuffix->pStaticName->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nSuffixID) );
			pCheckSuffix->pCheckBoxSuffix->SetTooltipText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nDescriptionID) );
			pCheckSuffix->Show( true );
				
		}
	}
	
	if( m_bRemote )
	{	
		nSuffixID = pCompoundSox->GetFieldFromLablePtr (m_RemoteCompoundItem.nVecCompoundList[0], "_Explanation" )->GetInteger();
		for( int i = 0; i < (int)m_VecCheckBoxSuffix.size(); ++i )
		{
			pCheckSuffix = m_VecCheckBoxSuffix[i];
			if( pCheckSuffix && pCheckSuffix->nID == nSuffixID )
			{
				pCheckSuffix->pCheckBoxSuffix->SetChecked( true , false );	
				break;
			}
		}
		ProcessCommand( EVENT_CHECKBOX_CHANGED, false, pCheckSuffix->pCheckBoxSuffix , 0 );
	}

}

void CDnSuffixCompoundDlg::RefreshSuffixList()
{
	if( m_nSelectJobType < 0  || 
		m_nSelectJobType >= (int)m_VecSetList.size() || 
		m_VecSetList.empty() )
		return;

	int nCapacity = (int)m_pVecResultCheckBoxList.size() - 1 ;
	for( int i = 0; i < nCapacity; ++i )
		m_pVecResultCheckBoxList[i] = NULL;
	m_pVecResultCheckBoxList.clear();

	std::vector< CDnCompoundBase::ItemGroupStruct* > VecList;
	
	//if( m_bRemote )
	//{
	//	VecList.push_back( &m_RemoteCompoundItem );
	//}
	//else
	//{
		VecList = m_VecSetList[m_nSelectJobType]->m_SetItemList;
	//}
	

	CDnCompoundBase::ItemGroupStruct* pCompoundItem;
	int nSelectControlType = 0;

	CTreeItem *pSelectedPartsItem = m_pTreeCtrlEquipment->GetSelectedItem();	
	if(  pSelectedPartsItem ) nSelectControlType = pSelectedPartsItem->GetItemValueInt();
	
	// Selected equipment list must be Sub, not Title (because the number of suffixes that can be shown in the checkbox is limited). 
	if( m_nSelectSuffixGrade < 0 && pSelectedPartsItem->GetDepth() >= 1 )
		return;

	int nSuffixGrade = 0;
	m_pComboBoxGrade->GetItemValue( m_nSelectSuffixGrade , nSuffixGrade );

	for( DWORD i = 0; i < VecList.size(); ++i )
	{
		pCompoundItem = VecList[i];
		if( ! pCompoundItem ) continue;

		if( pCompoundItem->nRank == eRARE && m_Rank.nRare == 0 ) continue;
		else if( pCompoundItem->nRank == eEPIC && m_Rank.nEpic == 0 ) continue;
		else if( pCompoundItem->nRank == eUNIQUE && m_Rank.nUnique == 0 ) continue;
		else if( pCompoundItem->nRank == eLEGEND /* && m_Rank.nLegend == 0*/ ) continue;		// Skip Legend grade items for now (no Legend in design doc) 
		
		//if( pSelectedPartsItem->GetDepth() > 1 ) 
		if( pSelectedPartsItem && pSelectedPartsItem->GetItemValueInt() != pCompoundItem->nPartsID ) 
			continue;
		if( nSuffixGrade != pCompoundItem->nSuffixGrade ) 
			continue;
		if( ! CheckCompoundItemLevel( pCompoundItem->nLevel ) ) 
			continue;

		m_pVecResultCheckBoxList.push_back( pCompoundItem );
	}

	MakeSuffixCheckBox( m_pVecResultCheckBoxList );

}


int CDnSuffixCompoundDlg::GetTypeFromPartsID( const int& nPartsID )
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

bool CDnSuffixCompoundDlg::IsExistItem( int nSuffixID )
{
	//if(  nSuffixID < 0 || m_VecCheckBoxSuffix.empty() )
	//	return false;

	//static int nCount = 0;
	//
	//std::map< int, CheckSuffixStruct*>::iterator Itor;
	//Itor = m_VecCheckBoxSuffix.find( nSuffixID );
	//
	//if( Itor == m_VecCheckBoxSuffix.end() )
	//{
	//	return false;
	//}
	//
	return true;
}

void CDnSuffixCompoundDlg::Reset()
{	
	m_bRemote = false;
	m_nSelectJobType = ( CDnActor::s_hLocalActor->GetClassID() - 1 );
	m_pComboBoxJob->SetSelectedByIndex( m_nSelectJobType );
	m_pComboBoxGrade->SetSelectedByIndex( 0 );
	
	m_pCheckBoxRare->SetChecked( true, false );
	m_pCheckBoxEpic->SetChecked( true, false );
	m_pCheckBoxUnique->SetChecked( true, false );

	m_Rank.nLegend = eLEGEND;
	m_Rank.nRare = eRARE;
	m_Rank.nEpic = eEPIC;
	m_Rank.nUnique = eUNIQUE;
	
	RefreshCheckBoxLevel( CDnActor::s_hLocalActor->GetLevel(), m_bRemote );	
	RefreshEquipList();
	RefreshSuffixList();
	
	if( m_VecCheckBoxSuffix.empty() == false )
	{
		CheckSuffixStruct* pCheckStruct = m_VecCheckBoxSuffix[0];
		if( pCheckStruct && pCheckStruct->pCheckBoxSuffix ) pCheckStruct->pCheckBoxSuffix->SetChecked(true, false);
	}
}

void CDnSuffixCompoundDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnSuffixCompoundDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if( IsCmdControl("ID_CHECKBOX_RARE") ||
			IsCmdControl("ID_CHECKBOX_EPIC") || 
			IsCmdControl("ID_CHECKBOX_UNIQUE") ||
			IsCmdControl("ID_CHECKBOX_EPIC") )	
		{
			m_Rank.nRare = m_pCheckBoxRare->IsChecked() ? eRARE : 0;
			m_Rank.nEpic = m_pCheckBoxEpic->IsChecked() ? eEPIC : 0;
			m_Rank.nUnique =  m_pCheckBoxUnique->IsChecked() ? eUNIQUE : 0;

			if( m_bRemote ) 
				Reset();
			else 
				RefreshSuffixList();
		}
		if( strstr( pControl->GetControlName(), "ID_CHECKBOX_LEVEL") )
		{
			for( DWORD i = 0; i < m_vecCheckBoxLevel.size(); ++i )
			{
				// Set item level increase : 24, 32, 40, 50, 60	
				if( m_vecCheckBoxLevel[i]->IsChecked() )	
					m_nVecLevelList[i] = i < 3 ? ( i+3 ) * 8 : ( i+6 ) * 5;   
				else 
				{
					m_nVecLevelList[i] = 0;
				}
			}
			if( m_bRemote ) 
				Reset();
			else
			RefreshSuffixList();
		}

		if( strstr( pControl->GetControlName(), "ID_CHECKBOX_OPTION") ||
			strstr( pControl->GetControlName(), "ID_CHECKBOX_PVPOPTION") )
		{
			if( m_bInitialize )
				return;
			
			CDnItemCompoundRenewalTabDlg* pCompoundTabDlg = static_cast<CDnItemCompoundRenewalTabDlg*>( m_pParentDialog );
			if( pCompoundTabDlg ) 
			{	
				//RefreshSuffixList();
				pCompoundTabDlg->UpdateGroupList( CDnCompoundBase::COMPOUND_SUFFIX_DIALOG );
			}
		}

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
				RefreshEquipList();
				RefreshSuffixList();
				m_pParentDialog->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, pControl, 0 );
			}
		}
		else if( IsCmdControl("ID_COMBOBOX_GRADE") )
		{
			if( m_pComboBoxGrade->GetSelectedIndex() != m_nSelectSuffixGrade )
				m_nSelectSuffixGrade = m_pComboBoxGrade->GetSelectedIndex();

			if( m_bRemote ) 
				Reset();
			else
			{
				RefreshSuffixList();
				m_pParentDialog->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, pControl, 0 );
			}
		}

	}

	else if( nCommand == EVENT_TREECONTROL_SELECTION )
	{	
		CTreeItem* pPartsItem;

		if( IsCmdControl("ID_TREE_ITEM") )
		{
			pPartsItem  = m_pTreeCtrlEquipment->GetSelectedItem();
			if( pPartsItem ) 
			{
				RefreshSuffixList();
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnSuffixCompoundDlg::UpdateGroupList( std::vector< CDnCompoundBase::ItemGroupStruct* >& pVecResultList )
{
	if( m_pVecResultCheckBoxList.empty() )
		return;

	if( m_VecCheckBoxSuffix.empty() )
		return;

	DNTableFileFormat* pCompoundSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	if(  !pCompoundSox ) return;
	
	CDnCompoundBase::ItemGroupStruct* pCompoundItem;
	int nSuffixListSize = 0;
	int nSuffixID = 0;
	bool bPass = false;

	for( int i = 0; i < (int)m_pVecResultCheckBoxList.size(); ++i )
	{
		pCompoundItem = m_pVecResultCheckBoxList[i];

		if( ! pCompoundItem ) continue;
		
		nSuffixListSize = (int)pCompoundItem->nVecCompoundList.size();
		pCompoundItem->nVecResultCompoundList.clear();
		
		// If suffix check is unchecked, clear the list value to 0  
		for( int j = 0; j < nSuffixListSize; ++j )
		{
			nSuffixID = pCompoundSox->GetFieldFromLablePtr ( pCompoundItem->nVecCompoundList[j], "_Explanation" )->GetInteger();
			if( CheckCompoundSuffixID( nSuffixID ) )	
			{
				pCompoundItem->nVecResultCompoundList.push_back( pCompoundItem->nVecCompoundList[j] );
			}
		}

		pVecResultList.push_back( pCompoundItem );
	}
}






#endif 
