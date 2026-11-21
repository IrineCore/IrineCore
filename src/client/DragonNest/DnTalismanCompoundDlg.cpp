#include "StdAfx.h"

#if defined(PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL) && defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnTalismanCompoundDlg.h"
#include "DnItemCompoundRenewalTab.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnTalismanCompoundDlg::CDnTalismanCompoundDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCompoundBase( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
, m_pTreeCtrlTalisman( NULL )
, m_bRemote( false )
{
}

CDnTalismanCompoundDlg::~CDnTalismanCompoundDlg()
{
	CompoundGroupListClear();
}

void CDnTalismanCompoundDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemCompound_Tab03Dlg.ui" ).c_str(), bShow );
}

void CDnTalismanCompoundDlg::InitialUpdate()
{
	m_pTreeCtrlTalisman = GetControl<CEtUITreeCtl>( "ID_TREE_LIST" );

	SetCompoundTable( CDnCompoundBase::COMPOUND_TALISMAN_DIALOG );
	SetTalismanList();
}

void CDnTalismanCompoundDlg::SetCompoundTable( int nType )
{
	CompoundGroupListClear();

	if( nType != CDnCompoundBase::COMPOUND_TALISMAN_DIALOG )
		return;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TITEMCOMPOUNDGROUP );
	DNTableFileFormat *pCompoundSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	if( !pSox || !pCompoundSox ) return;

	std::vector<int> nVecList;
	pSox->GetItemIDListFromField( "_MainCategory", nType, nVecList );
	if( nVecList.empty() )
		return;

	for( size_t i = 0; i < nVecList.size(); ++i )
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

		if( !IsExistItem( pStruct->nCompoundGroupType, pStruct->nCompoundGroupID ) )
		{
			m_MapCategoryList[pStruct->nCompoundGroupType].push_back( pStruct );
		}
		else
		{
			SAFE_DELETE( pStruct );
		}
	}
}

bool CDnTalismanCompoundDlg::IsExistItem( int nKey, int nExistitem )
{
	std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> >::iterator it = m_MapCategoryList.find( nKey );
	if( it != m_MapCategoryList.end() )
	{
		for( size_t i = 0; i < it->second.size(); ++i )
		{
			if( it->second[i]->nCompoundGroupID == nExistitem )
				return true;
		}
	}
	return false;
}

void CDnTalismanCompoundDlg::CompoundGroupListClear()
{
	for( std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> >::iterator it = m_MapCategoryList.begin(); it != m_MapCategoryList.end(); ++it )
	{
		SAFE_DELETE_VEC( it->second );
	}
	m_MapCategoryList.clear();
}

void CDnTalismanCompoundDlg::SetRemoteCompoundDlg( CDnCompoundBase::ItemGroupStruct& CompoundGroupData )
{
	m_bRemote = true;
	m_RemoteCompoundItem = CompoundGroupData;
	SetTalismanList();
}

bool CDnTalismanCompoundDlg::SetTalismanList()
{
	if( !m_pTreeCtrlTalisman ) return false;

	m_pTreeCtrlTalisman->DeleteAllItems();

	CTreeItem *pNormal = m_pTreeCtrlTalisman->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000054614 ) );
	if( pNormal )
	{
		pNormal->SetItemValueInt( eTalismanNormal );
		pNormal->SetTextColor( textcolor::GOLDENROD );
	}
	else return false;

	CTreeItem *pSpecial = m_pTreeCtrlTalisman->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000056921 ) );
	if( pSpecial )
	{
		pSpecial->SetItemValueInt( eTalismanSpecial );
		pSpecial->SetTextColor( textcolor::GOLDENROD );
	}
	else return false;

	for( std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> >::iterator it = m_MapCategoryList.begin(); it != m_MapCategoryList.end(); ++it )
	{
		std::vector<CDnCompoundBase::ItemGroupStruct*> &vecList = it->second;
		for( size_t i = 0; i < vecList.size(); ++i )
		{
			CTreeItem *pItem = NULL;
			if( it->first == eTalismanNormal )
				pItem = m_pTreeCtrlTalisman->AddChildItem( pNormal, CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, vecList[i]->nPartsID ) );
			else if( it->first == eTalismanSpecial )
				pItem = m_pTreeCtrlTalisman->AddChildItem( pSpecial, CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, vecList[i]->nPartsID ) );

			if( pItem )
			{
				pItem->SetItemValueInt( vecList[i]->nPartsID );
				if( m_bRemote &&
					vecList[i]->nCompoundGroupType == m_RemoteCompoundItem.nCompoundGroupType &&
					vecList[i]->nCompoundGroupID == m_RemoteCompoundItem.nCompoundGroupID &&
					vecList[i]->nPartsID == m_RemoteCompoundItem.nPartsID )
				{
					m_pTreeCtrlTalisman->SetSelectItem( pItem );
				}
			}
		}
	}

	m_pTreeCtrlTalisman->ExpandAll();

	if( m_bRemote )
		ProcessCommand( EVENT_TREECONTROL_SELECTION, false, m_pTreeCtrlTalisman, 0 );

	return true;
}

void CDnTalismanCompoundDlg::Reset()
{
	m_bRemote = false;
	SetTalismanList();
	if( m_pTreeCtrlTalisman )
	{
		CTreeItem *pNormal = m_pTreeCtrlTalisman->FindItemInt( eTalismanNormal );
		if( pNormal )
		{
			TREEITEM_LIST TreeList = pNormal->GetTreeItemList();
			if( !TreeList.empty() )
			{
				CTreeItem *pSelectItem = m_pTreeCtrlTalisman->FindItemInt( (*TreeList.begin())->GetItemValueInt() );
				if( pSelectItem ) m_pTreeCtrlTalisman->SetSelectItem( pSelectItem );
			}
		}
	}
}

void CDnTalismanCompoundDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnTalismanCompoundDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_TREECONTROL_SELECTION && IsCmdControl( "ID_TREE_LIST" ) )
	{
		CDnItemCompoundRenewalTabDlg *pCompoundTabDlg = static_cast<CDnItemCompoundRenewalTabDlg*>( m_pParentDialog );
		if( m_pTreeCtrlTalisman && m_pTreeCtrlTalisman->GetSelectedItem() && pCompoundTabDlg )
		{
			pCompoundTabDlg->UpdateGroupList( CDnCompoundBase::COMPOUND_TALISMAN_DIALOG );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnTalismanCompoundDlg::UpdateGroupList( std::vector< CDnCompoundBase::ItemGroupStruct* >& pVecResultList )
{
	if( !m_pTreeCtrlTalisman ) return;

	CTreeItem *pSelected = m_pTreeCtrlTalisman->GetSelectedItem();
	if( !pSelected ) return;

	int nParentType = pSelected->GetParentItem() ? pSelected->GetParentItem()->GetItemValueInt() : 0;
	std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> >::iterator it = m_MapCategoryList.find( nParentType );
	if( it != m_MapCategoryList.end() )
	{
		for( size_t i = 0; i < it->second.size(); ++i )
		{
			CDnCompoundBase::ItemGroupStruct *pItem = it->second[i];
			if( pItem && pItem->nPartsID == pSelected->GetItemValueInt() )
			{
				pItem->nVecResultCompoundList = pItem->nVecCompoundList;
				pVecResultList.push_back( pItem );
			}
		}
	}
}

#endif // PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL && PRE_ADD_TALISMAN_SYSTEM
