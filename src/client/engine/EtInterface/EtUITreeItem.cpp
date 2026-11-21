#include "StdAfx.h"
#include "EtUITreeItem.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

int CTreeItem::m_nGoalID = 0;

#define TEXT_LENGTH_EXCEED	20.0f

CTreeItem::CTreeItem()
	: m_dwColor(textcolor::WHITE)
	, m_dwBackColor(textcolor::FONT_GREEN)
	, m_dwItemType(typeNone)
	, m_fTreeHeight(0.0f)
	, m_nTreeDepth(0)
	, m_fIndentSize(0.0f)
	, m_pData(NULL)
	, m_nValue(0)
	, m_nValue64(0)
	, m_fValue(0.0f)
	, m_ClassIconType(eNONE)
	, m_pParentControl(NULL)
	, m_nSubCount( 0 )
	, m_fLineSpace(0.0f)
{
	m_pParentItem = NULL;
	m_bWithEllipsis = false;
}

CTreeItem::~CTreeItem()
{
	DeleteAll();
}

void CTreeItem::Initialize( CEtUIControl *pControl, int nDepth )
{
	ASSERT( pControl&&"CTreeItem::Initialize, pControl is NULL!" );

	SUIControlProperty *pProperty = pControl->GetProperty();
	if( !pProperty ) return;

	m_pParentControl = pControl;
	m_nTreeDepth = nDepth;
	m_fLineSpace = pProperty->TreeControlProperty.fLineSpace;
	m_fIndentSize = pProperty->TreeControlProperty.fIndentSize*(nDepth-1);

	SUIElement *pElement(NULL);
	pElement = pControl->GetElement(0);
	m_ButtonCoord.fWidth = pElement->fTextureWidth;
	m_ButtonCoord.fHeight = pElement->fTextureHeight;

	// Note : Base Coord
	//
	pElement = pControl->GetElement(2);
	if( pElement )
	{
		m_BaseCoord.fWidth = pElement->fTextureWidth;
		m_BaseCoord.fHeight = pElement->fTextureHeight;
	}

	// Note : Select Coord
	//
	pElement = pControl->GetElement(3);
	if( pElement )
	{
		m_SelectCoord.fWidth = pElement->fTextureWidth;
		m_SelectCoord.fHeight = pElement->fTextureHeight;
	}

	// Note : Class Icon Coord
	//
	for( int i=0; i<4; i++ )
	{
		pElement = pControl->GetElement( 4+i );
		if( pElement )
		{
			m_ClassIconCoord[i].fWidth = pElement->fTextureWidth;
			m_ClassIconCoord[i].fHeight = pElement->fTextureHeight;
		}
	}

	if( GetDepth() == 1 )
	{
		DWORD dwType = GetItemType();
		dwType |= typeMain;
		SetItemType( dwType );
	}
}

void CTreeItem::SetText( LPCWSTR szText, DWORD dwColor )
{
	m_strOrigText = m_strText = szText;
	m_dwColor = dwColor;
	m_bWithEllipsis = false;

	if( !m_pParentControl ) return;
	if( !m_pParentControl->GetParent() ) return;

	// Reuse similar handling code from buttons.
	SUIElement *pElement = m_pParentControl->GetElement(0);
	if( !pElement ) return;

	SUICoord sSymbolCoord;
	m_pParentControl->GetParent()->CalcTextRect( L"...", pElement, sSymbolCoord );

	SUICoord sTextCoord;
	m_pParentControl->GetParent()->CalcTextRect( szText, pElement, sTextCoord );

	if( sTextCoord.fWidth + m_fIndentSize > m_strCoord.fWidth )
	{
		float fRemainWidth = m_strCoord.fWidth - sSymbolCoord.fWidth - TEXT_LENGTH_EXCEED/DEFAULT_UI_SCREEN_WIDTH;
		int nStrLen = (int)m_strOrigText.length();
		float fTextCoordHeight = sTextCoord.fHeight;
		int i(0), nStartPos(0);

		for( ; (nStartPos+i)<=nStrLen; i++)
		{
			SUICoord sSubTextCoord;
			m_pParentControl->GetParent()->CalcTextRect( m_strOrigText.substr(nStartPos,i).c_str(), pElement, sSubTextCoord );

			if( sSubTextCoord.fWidth > fRemainWidth )
			{
				i--;
				m_strText = m_strOrigText.substr(0,nStartPos+i);
				m_strText += L"...";
				m_bWithEllipsis = true;
				return;
			}
		}
	}
}

void CTreeItem::SetBackText( LPCWSTR szText, DWORD dwColor )
{
	m_strBackText = szText;
	m_dwBackColor = dwColor;
}

void CTreeItem::SetUICoord( SUICoord &uiCoord )
{
	// Note : Set the control size.
	//
	m_uiCoord = uiCoord;

	if( GetDepth() == 1 )
	{
		m_uiCoord.fHeight += m_fLineSpace;
	}

	m_strCoord.SetSize( m_uiCoord.fWidth - m_ButtonCoord.fWidth, m_BaseCoord.fHeight );
}

void CTreeItem::UpdatePos( float fX, float fY )
{
	// Note : Set the control position.
	//
	m_uiCoord.SetPosition( fX + m_fIndentSize, fY );
	m_ButtonCoord.SetPosition( m_uiCoord.fX, m_uiCoord.fY + m_fLineSpace );
	m_strCoord.SetPosition( m_uiCoord.fX + m_ButtonCoord.fWidth + 0.006f, fY + m_fLineSpace );
	m_BaseCoord.SetPosition( m_uiCoord.fX, m_uiCoord.fY + m_fLineSpace );
	m_SelectCoord.SetPosition( m_uiCoord.fX - 0.004f, m_uiCoord.fY + m_fLineSpace - 0.004f );

	for( int i=0; i<4; i++ )
	{
		m_ClassIconCoord[i].SetPosition( m_uiCoord.fX + 0.003f, m_uiCoord.fY + m_fLineSpace ); //- 0.003f );
	}
}

bool CTreeItem::GetVisibleCount( float fControlHeight, float &fTreeHeight, int nCurLine, int &nCount )
{
	nCount++;

	if( nCount > nCurLine )
	{
		fTreeHeight += m_uiCoord.fHeight;
	}

	if( fControlHeight < fTreeHeight )
	{
		nCount--;
		return true;
	}

	if( IsExpanded() )
	{
		TREEITEM_LIST_ITER iter = m_listItem.begin();
		TREEITEM_LIST_ITER iter_end = m_listItem.end();
		for( ; iter != iter_end; ++iter )
		{
			if( (*iter)->GetVisibleCount( fControlHeight, fTreeHeight, nCurLine, nCount ) )
				return true;
		}
	}

	return false;
}

float CTreeItem::GetTreeHeight()
{
	m_fTreeHeight = m_uiCoord.fHeight;

	if( IsExpanded() )
	{
		TREEITEM_LIST_ITER iter = m_listItem.begin();
		TREEITEM_LIST_ITER iter_end = m_listItem.end();
		for( ; iter != iter_end; ++iter )
		{
			m_fTreeHeight += (*iter)->GetTreeHeight();
		}
	}

	return m_fTreeHeight;
}

void CTreeItem::GetChildCount( int &nCount )
{
	nCount++;

	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		(*iter)->GetChildCount( nCount );
	}
}

void CTreeItem::GetExpandedItemCount( int &nCount )
{
	nCount++;

	if( IsExpanded() )
	{
		TREEITEM_LIST_ITER iter = m_listItem.begin();
		TREEITEM_LIST_ITER iter_end = m_listItem.end();
		for( ; iter != iter_end; ++iter )
		{
			(*iter)->GetExpandedItemCount( nCount );
		}
	}
}

bool CTreeItem::Delete( CTreeItem *pItem )
{
	if( !pItem ) 
		return false;

	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( pItem == (*iter) ) 
		{
			SAFE_DELETE( *iter );
			m_listItem.erase( iter );
			return true;
		}

		if( (*iter)->Delete( pItem ) )
		{
			return true;
		}
	}

	return false;
}

bool CTreeItem::Delete( void *pData )
{
	if( !pData ) 
		return NULL;

	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( pData == (*iter)->GetItemData() ) 
		{
			SAFE_DELETE( *iter );
			m_listItem.erase( iter );
			return true;
		}

		if( (*iter)->Delete( pData ) )
		{
			return true;
		}
	}

	return false;
}

CTreeItem* CTreeItem::FindItemData( void *pData, bool bRecursive )
{
	if( !pData ) 
	{
		return NULL;
	}

	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( pData == (*iter)->GetItemData() )
			return (*iter);

		if( bRecursive )
		{
			CTreeItem *pItem = (*iter)->FindItemData( pData );
			if( pItem  ) return pItem;
		}
	}

	return NULL;
}

CTreeItem* CTreeItem::FindItemInt( int nValue, bool bRecursive )
{
	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( nValue == (*iter)->GetItemValueInt() )
			return (*iter);

		if( bRecursive )
		{
			CTreeItem *pItem = (*iter)->FindItemInt( nValue );
			if( pItem  ) return pItem;
		}
	}

	return NULL;
}

CTreeItem* CTreeItem::FindItemInt64( INT64 nValue, bool bRecursive )
{
	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( nValue == (*iter)->GetItemValueInt64() )
			return (*iter);

		if( bRecursive )
		{
			CTreeItem *pItem = (*iter)->FindItemInt64( nValue );
			if( pItem  ) return pItem;
		}
	}

	return NULL;
}

CTreeItem* CTreeItem::FindItemFloat( float fValue, bool bRecursive )
{
	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( fValue == (*iter)->GetItemValueFloat() )
			return (*iter);

		if( bRecursive )
		{
			CTreeItem *pItem = (*iter)->FindItemFloat( fValue );
			if( pItem  ) return pItem;
		}
	}

	return NULL;
}

CTreeItem* CTreeItem::Add( CTreeItem *pItem )
{
	m_listItem.push_back( pItem );
	return pItem;
}

void CTreeItem::DeleteAll()
{
	std::for_each ( m_listItem.begin(), m_listItem.end(), DeleteItem() );
	m_listItem.clear();
}

CTreeItem* CTreeItem::GetButtonClickItem( float fX, float fY )
{
	if( m_ButtonCoord.IsInside( fX, fY ) )
		return this;

	return NULL;
}

CTreeItem* CTreeItem::GetClickItem( float fX, float fY )
{
	if( m_uiCoord.IsInside( fX, fY ) )
		return this;

	return NULL;
}

void CTreeItem::TravelTree( TREEITEM_LIST &vecTreeTravelItem, int &nCount, int nCurLine, int nVisibleCount )
{
	if( GetDepth() > 0 )
	{
		nCount++;
	}

	if( nCount >= (nCurLine+nVisibleCount) )
		return;

	if( nCount >= nCurLine )
	{
		vecTreeTravelItem.push_back( this );
	}

	if( IsExpanded() )
	{
		TREEITEM_LIST_ITER iter = m_listItem.begin();
		TREEITEM_LIST_ITER iter_end = m_listItem.end();
		for( ; iter != iter_end; ++iter )
		{
			(*iter)->TravelTree( vecTreeTravelItem, nCount, nCurLine, nVisibleCount );
		}
	}
}

void CTreeItem::ExpandAll()
{
	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( !(*iter)->IsOpened() )
			continue;

		if( (*iter)->HasChild() )
			(*iter)->ExpandAll();

		(*iter)->Expand();
	}
}

void CTreeItem::CollapseAll()
{
	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( !(*iter)->IsOpened() )
			continue;

		if( (*iter)->HasChild() )
			(*iter)->CollapseAll();

		(*iter)->Collapse();
	}
}

bool CTreeItem::ExpandAllChildren( const CTreeItem *pItem )
{
	ASSERT(pItem&&"CTreeItem::ExpandAllChildren");

	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( pItem == (*iter) )
		{
			if( !(*iter)->IsOpened() )
				return true;

			if( (*iter)->HasChild() )
				(*iter)->ExpandAll();

			(*iter)->Expand();
			return true;
		}
		else
		{
			if( (*iter)->ExpandAllChildren( pItem ) )
			{
				return true;
			}
		}
	}

	return false;
}

void CTreeItem::AddItemTypeAllChildren( DWORD dwType )
{
	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( (*iter)->HasChild() )
			(*iter)->AddItemTypeAllChildren( dwType );

		(*iter)->AddItemType( dwType );
	}
}

CTreeItem* CTreeItem::GetBeginItem()
{
	if( m_listItem.empty() )
		return NULL;

	return (*m_listItem.begin());
}

CTreeItem* CTreeItem::GetEndItem()
{
	if( m_listItem.empty() )
		return NULL;

	return (*m_listItem.rbegin());
}

bool CTreeItem::EqualQuestID( CTreeItem * pFirst, CTreeItem * pSecond )
{
	if( m_nGoalID == pSecond->GetItemValueInt() )
		return true;

	return false;
}

//#ifdef PRE_ADD_MISSION_NEST_TAB
int CTreeItem::GetSplitItemStringValue( std::string szStrValue, char ch, bool bfront )
{	
	std::string::size_type nSize = szStrValue.length();
	if (nSize <= 0)
		return -1;

	std::string::size_type nPos = szStrValue.find(ch);
	int nStart = 1;

	char szValue[128] = {0, };

	if( bfront )
	{
		strncpy( szValue, szStrValue.c_str(), (nPos == szStrValue.npos || nPos < 0) ? nSize : nPos );
	}
	else 
	{	
		if( nPos == szStrValue.npos || nPos < 0 )  nPos = 0, nStart = -1;
		strncpy( szValue, szStrValue.c_str() + ( nPos + nStart ), ( nSize - nPos ) );
	}

	int nResult = atoi(szValue);

	return  nResult > 0 ? nResult : -1 ;  
}

bool CTreeItem::Assending( CTreeItem* pLitem, CTreeItem* pRIitem )
{
	int nlValue = GetSplitItemStringValue( pLitem->GetItemValueStringA().c_str(), '/', pLitem->HasChild() ? true : false );
	int nRValue = GetSplitItemStringValue( pRIitem->GetItemValueStringA().c_str(), '/', pRIitem->HasChild() ? true : false );

	if( nlValue < nRValue )
		return true;

	return false;
}

bool CTreeItem::Desending( CTreeItem* pLitem, CTreeItem* pRIitem )
{
	int nlValue = GetSplitItemStringValue( pLitem->GetItemValueStringA().c_str(), '/', pLitem->HasChild() ? true : false );
	int nRValue = GetSplitItemStringValue( pRIitem->GetItemValueStringA().c_str(), '/', pRIitem->HasChild() ? true : false );

	if( nlValue > nRValue )
		return true;

	return false;
}
void CTreeItem::SortBySubCategory( bool bSortStanard )
{
	TREEITEM_LIST_ITER iter = m_listItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( (*iter)->HasChild() )
			(*iter)->SortBySubCategory( bSortStanard );	
	}
	if( bSortStanard )
		m_listItem.sort( Assending );
	else
		m_listItem.sort( Desending );
}
//#endif 
void CTreeItem::SetExceptLastSort( int nGoalID )
{
	m_nGoalID = nGoalID;

	m_listItem.sort( EqualQuestID );
}
