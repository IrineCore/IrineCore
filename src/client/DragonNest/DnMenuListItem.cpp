#include "StdAfx.h"
#include "DnMenuListItem.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnMenuListItem::CDnMenuListItem()
:m_pParentControl(NULL)
,m_pParentItem(NULL)
,m_bIsSelected(false)
,m_bIsOver(false )
,m_bPress(false)
,m_bShow(false)
,m_bGetOutArea(false)
,m_pData(NULL)
,m_nValue(0)
,m_nValue64(0)
,m_fValue(0.0f)
,m_strBaseCoord_X(0.0f)
,m_strOverCoord_X(0.0f)
,m_strSelectCoord_X(0.0f)
,m_TextColor(textcolor::WHITE)
,m_bEllipsis(false)
{

}


CDnMenuListItem::~CDnMenuListItem()
{
	if( !m_vSubItem.empty() )
	{
		int size = (int)m_vSubItem.size();
		for( int i=0; i<size; ++ i )
			SAFE_DELETE( m_vSubItem[ i ] );
		m_vSubItem.clear();
	}
}



void CDnMenuListItem::Initialize( CEtUIControl * pControl, EITEMTYPE type, bool bShow, int nDepth )
{
	Show( bShow );

	SUIControlProperty * pProperty = pControl->GetProperty();
	if( !pProperty )
		return;

	m_pParentControl = pControl;	
	m_fLineSpace = 0;

	m_ItemType = type;

	// Base.
	int elementIdx = (int)type * 3 + 1;
	SUIElement * pElement = NULL;
	pElement = pControl->GetElement( elementIdx );
	if( pElement )
	{		
		m_BaseCoord.fWidth = pElement->fTextureWidth;
		m_BaseCoord.fHeight = pElement->fTextureHeight;

		m_BaseCoord.fX = pProperty->UICoord.fX;// + fIndet;
		m_BaseCoord.fY = pProperty->UICoord.fY;

		m_uiCoord = m_BaseCoord;	

		// Font.
		m_strBaseCoord_X = pElement->fFontHoriOffset;
		m_strBaseCoord = m_uiCoord;
		m_strBaseCoord.fX += pElement->fFontHoriOffset;
		m_strBaseCoord.fWidth -= ( pElement->fFontHoriOffset + pElement->fFontVertOffset );
		
		m_strCoord = m_strBaseCoord;
	}
	
	// Select.
	pElement = pControl->GetElement( elementIdx + 1 );
	if( pElement )
	{
		m_SelectCoord.fWidth = pElement->fTextureWidth;
		m_SelectCoord.fHeight = pElement->fTextureHeight;
		m_SelectCoord.fX = m_uiCoord.fX;
		m_SelectCoord.fY = m_uiCoord.fY;

		// Font.
		m_strOverCoord_X = pElement->fFontHoriOffset;
		m_strSelectCoord = m_SelectCoord;
		m_strSelectCoord.fX += pElement->fFontHoriOffset;
		m_strSelectCoord.fWidth -= ( pElement->fFontHoriOffset + pElement->fFontVertOffset );
	}

	// Over.
	pElement = pControl->GetElement( elementIdx + 2 );
	if( pElement )
	{
		m_OverCoord.fWidth = pElement->fTextureWidth;
		m_OverCoord.fHeight = pElement->fTextureHeight;
		m_OverCoord.fX = m_uiCoord.fX;
		m_OverCoord.fY = m_uiCoord.fY;

		// Font.
		m_strSelectCoord_X = pElement->fFontHoriOffset;
		m_strOverCoord = m_OverCoord;
		m_strOverCoord.fX += pElement->fFontHoriOffset;
		m_strOverCoord.fWidth -= ( pElement->fFontHoriOffset + pElement->fFontVertOffset );
	}

	

	
	
}

void CDnMenuListItem::DeleteSubItem()
{
	int size = (int)m_vSubItem.size();
	for( int i=0; i<size; ++ i )
		delete m_vSubItem[ i ];
	m_vSubItem.clear();
}


#define TEXT_LENGTH_EXCEED	20.0f
void CDnMenuListItem::SetText( LPCWSTR szText, DWORD dwColor )
{
	m_strOrigText = m_strText = szText;
	m_TextColor = dwColor;
	//m_dwColor = dwColor;
	//m_bWithEllipsis = false;

	if( !m_pParentControl ) return;
	if( !m_pParentControl->GetParent() ) return;

	// Reuse similar handling code from buttons.
	SUIElement *pElement = m_pParentControl->GetElement( 1 );
	if( !pElement ) return;

	SUICoord sSymbolCoord;
	m_pParentControl->GetParent()->CalcTextRect( L"...", pElement, sSymbolCoord );

	SUICoord sTextCoord;
	m_pParentControl->GetParent()->CalcTextRect( szText, pElement, sTextCoord );

	if( sTextCoord.fWidth > m_strCoord.fWidth )
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
				//m_bWithEllipsis = true;
				m_bEllipsis = true;
				return;
			}
		}
	}
	else
		m_bEllipsis = false;
}

void CDnMenuListItem::AddItem( CDnMenuListItem * pItem )
{
	m_vSubItem.push_back( pItem );
}

CDnMenuListItem * CDnMenuListItem::GetClickItem( float fX, float fY )
{
	if( !IsShow() )
		return NULL;

	if( m_uiCoord.IsInside( fX, fY ) )
		return this;
	
	if( m_ItemType == EITEMTYPE::EITEMTYPE_MAIN && IsSelected() )
	{
		CDnMenuListItem * pItem = NULL;
		int size = (int)m_vSubItem.size();
		for( int i=0; i<size; ++i )
		{
			pItem = m_vSubItem[ i ]->GetClickItem( fX, fY );
			if( pItem )
				return pItem;
		}
	}

	return NULL;
}

CDnMenuListItem * CDnMenuListItem::GetFrontSubItem()
{
	if( m_vSubItem.empty() )
		return NULL;

	return m_vSubItem.front();
}

CDnMenuListItem * CDnMenuListItem::GetEndSubItem()
{
	if( m_vSubItem.empty() )
		return NULL;

	return m_vSubItem.back();
}

int CDnMenuListItem::GetSubItemSize()
{
	return (int)m_vSubItem.size();
}

void CDnMenuListItem::SetUICoordX( float fX )
{
	m_uiCoord.fX        = fX;
	m_BaseCoord.fX      = fX;
	m_SelectCoord.fX    = fX;	
	m_OverCoord.fX      = fX;

	m_strBaseCoord.fX   = fX + m_strBaseCoord_X;
	m_strOverCoord.fX   = fX + m_strOverCoord_X;
	m_strSelectCoord.fX = fX + m_strSelectCoord_X;

	m_strCoord = CrrStrCoord();
}

void CDnMenuListItem::SetUICoordY( float fY )
{
	m_uiCoord.fY        = fY;
	m_BaseCoord.fY      = fY;
	m_SelectCoord.fY    = fY;
	m_OverCoord.fY      = fY;

	m_strBaseCoord.fY   = fY;
	m_strOverCoord.fY   = fY;
	m_strSelectCoord.fY = fY;

	m_strCoord = CrrStrCoord();
}

void CDnMenuListItem::ShowItems( bool bShow )
{	
	int size = (int)m_vSubItem.size();
	for( int i=0; i<size; ++i )
		m_vSubItem[ i ]->Show( bShow );
}

void CDnMenuListItem::Show( bool bShow, bool bChild )
{
	m_bShow = bShow;

	if( bChild )
	{
		int size = (int)m_vSubItem.size();
		for( int i=0; i<size; ++i )
			m_vSubItem[ i ]->Show( bShow );
	}
}


SUICoord & CDnMenuListItem::CrrStrCoord()
{
	// Normal.
	if( !m_bIsSelected && !m_bIsOver )
		return m_strBaseCoord;

	// Selected.
	if( m_bIsSelected )
		return m_strSelectCoord;

	// Over.
	if( m_bIsOver )
		return m_strOverCoord;
}

//bool CDnMenuListItem::SetTextWithEllipsis( const wchar_t *wszText, const wchar_t *wszSymbol )
//{
//	ASSERT( wszText&&wszSymbol);
//
//	m_szText = wszText;
//	m_strRenderText = m_szText;
//
//	SUIElement *pElement = GetElement(0);
//	if( !pElement ) return false;
//
//	SUICoord sSymbolCoord;
//	m_pParent->CalcTextRect( wszSymbol, pElement, sSymbolCoord );
//
//	SUICoord sTextCoord;
//	m_pParent->CalcTextRect( m_szText.c_str(), pElement, sTextCoord );
//
//	if( sTextCoord.fWidth > m_Property.UICoord.fWidth )
//	{
//		float fRemainWidth = m_Property.UICoord.fWidth - sSymbolCoord.fWidth;
//		int nStrLen = (int)m_szText.length();
//		float fTextCoordHeight = sTextCoord.fHeight;
//		int i(1), nStartPos(0);
//
//		for( ; (nStartPos+i) <= nStrLen; i++)
//		{
//			SUICoord sSubTextCoord;
//			std::wstring subStr = m_szText.substr(nStartPos,i);
//			m_pParent->CalcTextRect( subStr.c_str(), pElement, sSubTextCoord );
//
//			if( sTextCoord.fHeight < (m_Property.UICoord.fHeight - fTextCoordHeight) )
//			{
//				// If the text continues onto the next line...
//				if( sSubTextCoord.fWidth > m_Property.UICoord.fWidth )
//				{
//					nStartPos += i-1;
//					i = 0;
//					fTextCoordHeight += sSubTextCoord.fHeight;
//				}
//			}
//			else
//			{
//				// Check the last line.
//				if( sSubTextCoord.fWidth > fRemainWidth )
//				{
//					i--;
//
//					m_strRenderText = m_szText.substr(0,nStartPos+i);
//					m_strRenderText += wszSymbol;
//					return true;
//				}
//			}
//		}
//	}
//
//	return false;
//}
