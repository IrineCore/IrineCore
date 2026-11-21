#include "Stdafx.h"
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
#include "DnChandleryCompoundDlg.h"
#include "DnLocalPlayerActor.h"
#include "DnItemCompoundRenewalTab.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnChandleryCompoundDlg::CDnChandleryCompoundDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCompoundBase( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
, m_pTreeCtrlChandlery(NULL)
, m_bRemote( false )
{

}

CDnChandleryCompoundDlg::~CDnChandleryCompoundDlg()
{
	CompoundGroupListClear();
}


void CDnChandleryCompoundDlg::InitialUpdate()
{
	m_pTreeCtrlChandlery = GetControl<CEtUITreeCtl>("ID_TREE_LIST");

	SetCompoundTable( CDnCompoundBase::COMPOUND_CHANDLERY_DIALOG );	
	SetChandleryList();
}
void CDnChandleryCompoundDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemCompound_Tab03Dlg.ui" ).c_str(), bShow );
}

void CDnChandleryCompoundDlg::SetCompoundTable( int nType )
{
	CompoundGroupListClear();

	if( !CDnActor::s_hLocalActor ) return;
	if( nType != CDnCompoundBase::COMPOUND_CHANDLERY_DIALOG ) return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUNDGROUP );
	DNTableFileFormat* pCompoundSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	if( !pSox || !pCompoundSox ) return;

	std::vector<int> nVecList;
	pSox->GetItemIDListFromField( "_MainCategory", nType, nVecList );

	if( nVecList.empty() )
		return;

	SetItemStruct* pSetListStruct = NULL; 
	for( int i = eGEM; i < eMaxChandleryList; ++i )
	{	
		pSetListStruct = new SetItemStruct( i );
		if( pSetListStruct ) m_MapChandleryList.insert( make_pair( i, pSetListStruct ) );	

	}

	std::map< int, SetItemStruct* >::iterator mItor;
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

		pCompoundSox->GetItemIDListFromField( "_CompoundGroupID", pStruct->nCompoundGroupID, pStruct->nVecCompoundList );

		if( ! IsExistItem( pStruct->nCompoundGroupType, pStruct->nCompoundGroupID ) )
		{
			m_MapCategoryList[pStruct->nCompoundGroupType].push_back( pStruct );
		}
	
	}


}

bool CDnChandleryCompoundDlg::IsExistItem( int nKey, int nExistitem )
{
	std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> >::iterator mItor =  m_MapCategoryList.find( nKey );
	
	if( mItor != m_MapCategoryList.end() )
	{
		std::vector<CDnCompoundBase::ItemGroupStruct*> VecCompoundGruopList = mItor->second;
		for( int i = 0; i < (int)VecCompoundGruopList.size(); ++i )
		{
			if( VecCompoundGruopList[i]->nCompoundGroupID == nExistitem )
				return true;
		}
	
	}

	return false;	
}

void CDnChandleryCompoundDlg::CompoundGroupListClear()
{
	std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> >::iterator ItorPos = m_MapCategoryList.begin();
	std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> >::iterator Itorend = m_MapCategoryList.end();
	
	std::vector< CDnCompoundBase::ItemGroupStruct* > VecList;
	for( ItorPos; ItorPos != Itorend ; ++ItorPos )
	{
		VecList = ItorPos->second;
		if( ! VecList.empty() )
			SAFE_DELETE_VEC( VecList );		
	}
}

void CDnChandleryCompoundDlg::SetRemoteCompoundDlg( CDnCompoundBase::ItemGroupStruct& CompoundGroupData )
{
	m_bRemote = true;
	m_RemoteCompoundItem = CompoundGroupData;
	
	SetChandleryList();
}

bool CDnChandleryCompoundDlg::SetChandleryList()
{	
	m_pTreeCtrlChandlery->DeleteAllItems();
	CTreeItem* pItem = NULL;

	std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> >::iterator ItorPos = m_MapCategoryList.begin();
	std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> >::iterator Itorend = m_MapCategoryList.end();

	std::vector< CDnCompoundBase::ItemGroupStruct* > VecList;

	CTreeItem* pGem = m_pTreeCtrlChandlery->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2147 ) ); // Jewel
	if( pGem ) 
	{
		pGem->SetItemValueInt( eGEM );
		pGem->SetTextColor( textcolor::GOLDENROD );
	}
	else return false;

	CTreeItem* pMix = m_pTreeCtrlChandlery->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2148 ) ); // Combination
	if( pMix ) 
	{
		pMix->SetItemValueInt( eMIX );
		pMix->SetTextColor( textcolor::GOLDENROD );
	}
	else return false;

	//CTreeItem* pExchangeEpic = m_pTreeCtrlChandlery->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2149 ) ); // Exchange (Epic)
	//if( pExchangeEpic ) pExchangeEpic->SetItemValueInt( eExchangeEpic );
	//else return false;

	//CTreeItem* pExchangeUnique = m_pTreeCtrlChandlery->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2156 ) ); // Exchange (Unique)
	//if( pExchangeUnique ) pExchangeUnique->SetItemValueInt( eExchangeUnique );
	//else return false;

	CTreeItem* pGift = m_pTreeCtrlChandlery->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2157 ) ); // Gift
	if( pGift ) 
	{
		pGift->SetItemValueInt( eGift );
		pGift->SetTextColor( textcolor::GOLDENROD );
	}
	else return false;

	CTreeItem* pSkillBook = m_pTreeCtrlChandlery->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2158 ) ); // Ex skill book
	if( pSkillBook ) 
	{
		pSkillBook->SetItemValueInt( eExSkillbook );
		pSkillBook->SetTextColor( textcolor::GOLDENROD );
	}
	else return false;


	for( ItorPos; ItorPos != Itorend ; ++ItorPos )
	{
		VecList = ItorPos->second;

		for( int i = 0; i < (int)VecList.size(); ++i )
		{
			switch( ItorPos->first )
			{
			case eGEM:
				pItem = m_pTreeCtrlChandlery->AddChildItem( pGem, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, VecList[i]->nPartsID ) );
				break;
			case eMIX:
				pItem = m_pTreeCtrlChandlery->AddChildItem( pMix, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, VecList[i]->nPartsID ) );
				break;
			//case eExchangeEpic:
			//	pItem = m_pTreeCtrlChandlery->AddChildItem( pExchangeEpic, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, VecList[i]->nPartsID ) );
			//	break;
			//case eExchangeUnique:
			//	pItem = m_pTreeCtrlChandlery->AddChildItem( pExchangeUnique, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, VecList[i]->nPartsID ) );
			//	break;
			case eGift:
				pItem = m_pTreeCtrlChandlery->AddChildItem( pGift, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, VecList[i]->nPartsID ) );
				break;
			case eExSkillbook:
				pItem = m_pTreeCtrlChandlery->AddChildItem( pSkillBook, CTreeItem::typeOpen, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, VecList[i]->nPartsID ) );
				break;
			}

			if( pItem )
			{
				pItem->SetItemValueInt( VecList[i]->nPartsID );
				
				if( m_bRemote 
					&& VecList[i]->nCompoundGroupType == m_RemoteCompoundItem.nCompoundGroupType
					&& VecList[i]->nCompoundGroupID == m_RemoteCompoundItem.nCompoundGroupID
					&& m_RemoteCompoundItem.nPartsID == VecList[i]->nPartsID )
				{
					m_pTreeCtrlChandlery->SetSelectItem( pItem );
				}
			}
		}
	}

	m_pTreeCtrlChandlery->ExpandAll();

	if( m_bRemote ) ProcessCommand( EVENT_TREECONTROL_SELECTION, false, m_pTreeCtrlChandlery, 0 );

	return true;
}

void CDnChandleryCompoundDlg::Reset()
{
	m_bRemote = false;
	SetChandleryList();
	if( m_pTreeCtrlChandlery )
	{
		CTreeItem* pGemItem = m_pTreeCtrlChandlery->FindItemInt( eGEM );
		if( pGemItem ) 
		{
			TREEITEM_LIST TreeList = pGemItem->GetTreeItemList();
			CTreeItem* pSelectItem = m_pTreeCtrlChandlery->FindItemInt( (*TreeList.begin())->GetItemValueInt() );
			if( pSelectItem ) m_pTreeCtrlChandlery->SetSelectItem( pSelectItem );
		}
	}
	
}

void CDnChandleryCompoundDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;
	
	CEtUIDialog::Show( bShow );
}

void CDnChandleryCompoundDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_TREECONTROL_SELECTION )
	{	
		CDnItemCompoundRenewalTabDlg* pCompoundTabDlg = static_cast<CDnItemCompoundRenewalTabDlg*>( m_pParentDialog );
		CTreeItem* pPartsItem;

		if( IsCmdControl("ID_TREE_LIST") )
		{
			pPartsItem  = m_pTreeCtrlChandlery->GetSelectedItem();
			if( pPartsItem && pCompoundTabDlg  ) 
			{
				pCompoundTabDlg->UpdateGroupList( CDnCompoundBase::COMPOUND_CHANDLERY_DIALOG );
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnChandleryCompoundDlg::UpdateGroupList( std::vector< CDnCompoundBase::ItemGroupStruct* >& pVecResultList  )
{

	CDnCompoundBase::ItemGroupStruct* pCompoundItem;
	CTreeItem *pSelectedPartsItem = m_pTreeCtrlChandlery->GetSelectedItem();	
	if( ! pSelectedPartsItem )	return;
	int nSelectControlType = pSelectedPartsItem->GetParentItem() != NULL ? pSelectedPartsItem->GetParentItem()->GetItemValueInt() : 0;

	int nEquipmentType = 0;
	std::map< int, std::vector< CDnCompoundBase::ItemGroupStruct* > >::iterator Itor = m_MapCategoryList.find( nSelectControlType );
    std::vector< CDnCompoundBase::ItemGroupStruct* > pVecGroupList;

	if( Itor != m_MapCategoryList.end() )
	{
		pVecGroupList = Itor->second;
			
		for( DWORD i = 0; i < pVecGroupList.size(); ++i )
		{
			pCompoundItem = pVecGroupList[i];

			if( ! pCompoundItem )
				continue;

			// Check misc item type only! 
			if( pSelectedPartsItem && pSelectedPartsItem->GetItemValueInt() != pCompoundItem->nPartsID )
				continue;
							
			pCompoundItem->nVecResultCompoundList = pCompoundItem->nVecCompoundList;
			pVecResultList.push_back( pCompoundItem );
		}
	}
}
#endif 




