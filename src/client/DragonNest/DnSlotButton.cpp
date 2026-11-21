#include "StdAfx.h"
#include "DnSlotButton.h"
#include "DnInterface.h"
#include "MIInventoryItem.h"
#include "DnTooltipDlg.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//DECL_DN_SMART_PTR_STATIC( CDnSlotButton, 4000 )
DECL_SMART_PTR_STATIC( CDnSlotButton, 4000 )

std::vector<CDnSlotButton::sOrigIconTextureInfo> CDnSlotButton::s_vecOrigIconTextureHandle;
std::vector<CDnSlotButton::sIconTextureInfo> CDnSlotButton::s_vecIconTextureHandle;

CDnSlotButton::CDnSlotButton( CEtUIDialog *pParent )
	: CEtUIButton( pParent )
//	, CDnUnknownRenderObject( false )
	, m_nSlotIndex(-1)
	, m_nIconIndex(-1)
	, m_pItem(NULL)
	, m_nCountForRender(0)
	, m_nSplitCount(INVALID_SPLIT_COUNT)
	, m_itemType(MIInventoryItem::Item)
	, m_nItemID(0)
	, m_dwIconColor(0xFFFFFFFF)
	, m_bRenderCount(true)
	, m_ButtonState(ITEMSLOT_ENABLE)
	, m_nTooltipIndexBack(0)
	, m_SlotType(ST_ITEM_NONE)
	, m_bRegist(false)
	, m_bCountMoreThanOne(false)
	, m_bShowTooltip(true)
	, m_nExpireCount(0)
	, m_dwFontColor(0xffffffff)
{
	//m_bSnap = false;
	//m_bSnapBack = false;
}

CDnSlotButton::~CDnSlotButton(void)
{
	if( m_pItem ) {
		m_pItem->RemoveOwnerShip( this );		
	}
}

void CDnSlotButton::ProcessMemoryOptimize( float fElapsedTime )
{
	std::vector<sOrigIconTextureInfo>::iterator iter = s_vecOrigIconTextureHandle.begin();
	while( iter != s_vecOrigIconTextureHandle.end() )
	{
		(*iter).fDelta -= fElapsedTime;
		if( (*iter).fDelta <= 0.0f )
		{
			SAFE_RELEASE_SPTR( (*iter).hOrigTexture );
			iter = s_vecOrigIconTextureHandle.erase( iter );
			continue;
		}
		++iter;
	}
}

void CDnSlotButton::ReleaseIconTexture()
{
	for( int i = 0; i < (int)s_vecIconTextureHandle.size(); ++i )
	{
		SAFE_RELEASE_SPTR( s_vecIconTextureHandle[i].hIconTexture );
	}
	s_vecIconTextureHandle.clear();
	for( int i = 0; i < (int)s_vecOrigIconTextureHandle.size(); ++i )
	{
		SAFE_RELEASE_SPTR( s_vecOrigIconTextureHandle[i].hOrigTexture );
	}
	s_vecOrigIconTextureHandle.clear();
}

EtTextureHandle CDnSlotButton::GetItemIcon( int nIconIndex, SUICoord &UVCoord )
{
	EtTextureHandle hIconTexture;
	UVCoord.fX = UVCoord.fY = 0.0f;
	UVCoord.fWidth = UVCoord.fHeight = 1.0f;
	hIconTexture = GetIconTexture( eItemIconTexture, nIconIndex );
	return hIconTexture;
}

bool CDnSlotButton::GetSkillIcon( int nIconIndex, SUICoord &UVCoord, EtTextureHandle &hSkillTexture, EtTextureHandle &hSkillGrayTexture )
{
	hSkillTexture.Identity();
	hSkillGrayTexture.Identity();

	UVCoord.fX = UVCoord.fY = 0.0f;
	UVCoord.fWidth = UVCoord.fHeight = 1.0f;
	hSkillTexture = GetIconTexture( eSkillIconTexture, nIconIndex );
	hSkillGrayTexture = GetIconTexture( eSkillGrayIconTexture, nIconIndex );

	return ( hSkillTexture || hSkillGrayTexture );
}

EtTextureHandle CDnSlotButton::GetIconTexture( eIconTextureType eType, int nIconIndex )
{
	for( int i = 0; i < (int)s_vecIconTextureHandle.size(); ++i )
	{
		if( s_vecIconTextureHandle[i].nType == eType &&
			s_vecIconTextureHandle[i].nIconIndex == nIconIndex )
		{
			return s_vecIconTextureHandle[i].hIconTexture;
		}
	}

	// First search in the OrigTexture pool.
	EtTextureHandle hOrigTexture;
	int nOrigTextureIndex = 0;
	switch( eType )
	{
	case eItemIconTexture: nOrigTextureIndex = nIconIndex / ITEM_TEXTURE_ICON_COUNT; break;
	case eSkillIconTexture:
	case eSkillGrayIconTexture: nOrigTextureIndex = nIconIndex / SKILL_TEXTURE_ICON_COUNT; break;
	}
	for( int i = 0; i < (int)s_vecOrigIconTextureHandle.size(); ++i )
	{
		if( s_vecOrigIconTextureHandle[i].nType == eType &&
			s_vecOrigIconTextureHandle[i].nTextureIndex == nOrigTextureIndex )
		{
			s_vecOrigIconTextureHandle[i].fDelta = 30.0f;
			hOrigTexture = s_vecOrigIconTextureHandle[i].hOrigTexture;
			break;
		}
	}

	if( !hOrigTexture )
	{
		// Load from file and store in pool.
		CFileNameString szFileName;
		bool bExistFile;
		char szTemp[32] = {0,};
		switch( eType )
		{
		case eItemIconTexture:
			{
				if( nOrigTextureIndex+1 < 100 ) sprintf_s( szTemp, 32, "ItemIcon%02d.dds", nOrigTextureIndex+1 );
				else sprintf_s( szTemp, 32, "ItemIcon%03d.dds", nOrigTextureIndex+1 );
			}
			break;
		case eSkillIconTexture: sprintf_s( szTemp, 32, "SkillIcon%02d.dds", nOrigTextureIndex+1 ); break;
		case eSkillGrayIconTexture: sprintf_s( szTemp, 32, "SkillIcon%02d_b.dds", nOrigTextureIndex+1 ); break;
		}
		szFileName = CEtResourceMng::GetInstance().GetFullName( szTemp, &bExistFile );
		if( bExistFile )
		{
			hOrigTexture = LoadResource( szFileName.c_str(), RT_TEXTURE );
			if( hOrigTexture )
			{
				sOrigIconTextureInfo OrigInfo;
				OrigInfo.nType = eType;
				OrigInfo.nTextureIndex = nOrigTextureIndex;
				OrigInfo.hOrigTexture = hOrigTexture;
				OrigInfo.fDelta = 30.0f;
				s_vecOrigIconTextureHandle.push_back( OrigInfo );
			}
		}
	}

	// hOrigTexture should now exist.
	EtTextureHandle hIconTexture;
	if( hOrigTexture )
	{
		// Crop the icon from Orig texture and create a new texture.
		int nSizeX = 50;	// Set default value to 50.
		int nSizeY = 50;
		switch( eType )
		{
		case eItemIconTexture: nSizeX = ITEMSLOT_ICON_XSIZE; nSizeY = ITEMSLOT_ICON_YSIZE; break;
		case eSkillIconTexture:
		case eSkillGrayIconTexture: nSizeX = SKILLSLOT_ICON_XSIZE; nSizeY = SKILLSLOT_ICON_YSIZE; break;
		}
		hIconTexture = CEtTexture::CreateNormalTexture( nSizeX, nSizeY, FMT_DXT5, USAGE_DEFAULT, POOL_MANAGED );	// Icon texture is DXT5
		if( hIconTexture )
		{
			int nIconIndexInTexture = 0;
			switch( eType )
			{
			case eItemIconTexture: nIconIndexInTexture = nIconIndex % ITEM_TEXTURE_ICON_COUNT; break;
			case eSkillIconTexture:
			case eSkillGrayIconTexture: nIconIndexInTexture = nIconIndex % SKILL_TEXTURE_ICON_COUNT; break;
			}

			int nX, nY;
			CalcButtonUV( nIconIndexInTexture, hOrigTexture, nX, nY, nSizeX, nSizeY );

			RECT SrcRect;
			SrcRect.left = nX;
			SrcRect.top = nY;
			SrcRect.right = nX + nSizeX;
			SrcRect.bottom = nY + nSizeY;
			hIconTexture->Copy( hOrigTexture, D3DX_FILTER_NONE, NULL, &SrcRect );

			sIconTextureInfo IconInfo;
			IconInfo.nType = eType;
			IconInfo.nIconIndex = nIconIndex;
			IconInfo.hIconTexture = hIconTexture;
			s_vecIconTextureHandle.push_back( IconInfo );
		}
	}
	return hIconTexture;
}

void CDnSlotButton::Initialize( SUIControlProperty *pProperty )
{
	CEtUIButton::Initialize( pProperty );
	m_nTooltipIndexBack = pProperty->nTooltipStringIndex;
}

void CDnSlotButton::Render( float fElapsedTime )
{
	if( !IsShow() )
		return;

	if( IsDragMode() )
	{
		if( !drag::IsValid() || !CDnMouseCursor::GetInstance().IsShowCursor() )
		{
			//ReleaseDragMode(true);
			DisableSplitMode(true);
			drag::ReleaseControl();
		}
	}
}

bool CDnSlotButton::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
		{
			if( IsInside( fX, fY ) )
			{
				if( !m_bFocus )			
				{
					m_pParent->RequestFocus( this );
				}

				UINT uMsg2 = uMsg;
				if( wParam & MK_SHIFT )
				{
					uMsg2 |= 0x0010;
				}

				m_pParent->ProcessCommand( EVENT_BUTTON_CLICKED, true, this, uMsg2 );

				// Exclude button sound on slot buttons since it seems unnecessary.
				//GetUISound().Play( m_emSoundIndex );
				//if( m_nSoundIndex != -1 ) CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundIndex );

				return true;
			}
		}
		break;
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		{
			if( IsInside( fX, fY ) )
			{
				if( CDnMouseCursor::GetInstance().IsShowCursor() && !GetCapture() && m_bShowTooltip)
				{
					ShowTooltip( fX, fY );
				}

				// Note : On WM_MOUSEMOVE message, call Dialog's OnMouseMove() function
				//		Should not return true for this.
			}
		}
		break;
		// Right-click is completely omitted for EtUIButton, so handle it separately in slotButton.
	case WM_RBUTTONDOWN:
		{
			if( IsInside( fX, fY ) )
			{
				m_bPressed = true;
				SetCapture( m_pParent->GetHWnd() );
				if( !m_bFocus )
				{
					m_pParent->RequestFocus( this );
				}
				return true;
			}
			m_MouseCoord.fX = fX;
			m_MouseCoord.fY = fY;
		}
		break;
		// Override because SHIFT must also work with right-click
	case WM_RBUTTONUP:
		{
			if( m_bPressed )
			{
				m_bPressed = false;
				ReleaseCapture();

				if( IsInside( fX, fY ) )
				{
					if( wParam & MK_SHIFT )
					{
						uMsg |= 0x0010;
					}
					m_pParent->ProcessCommand( EVENT_BUTTON_CLICKED, true, this, uMsg );

					// Exclude button sound on slot buttons since it seems unnecessary.
					//GetUISound().Play( m_emSoundIndex );
					//if( m_nSoundIndex != -1 ) CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundIndex );
				}
				return true;
			}
		}
		break;
	}

	return CEtUIButton::HandleMouse( uMsg, fX, fY, wParam, lParam );
}

void CDnSlotButton::SetItem(MIInventoryItem *pItem, int count)
{
	if (pItem == NULL)
	{
		// etcIcon range is 61~99. Need to pass as index so subtract 60
		m_hItemIcon = GetItemIcon( 12013, m_IconUV );
		ResetSlot();
		return;
	}

	ResetSlot();

	// Note : Tooltip is hidden once the item is set.
	//
	SUIControlProperty *pProperty = GetProperty();
	pProperty->nTooltipStringIndex = 0;
	
	if( m_pItem ) {
		m_pItem->RemoveOwnerShip( this );
	}
	m_pItem = pItem;
	m_pItem->AddOwnerShip( this );
	m_nItemID = m_pItem->GetClassID();
	SetItemType(m_pItem->GetType());
	int nIconIndex = m_pItem->GetIconImageIndex();

	if (m_pItem && m_pItem->GetType() == MIInventoryItem::Item)
	{
		CDnItem* pCurItem = static_cast<CDnItem*>(m_pItem);
		eItemTypeEnum type = pCurItem->GetItemType();
		m_bCountMoreThanOne = (type == ITEMTYPE_NORMAL 
							|| type == ITEMTYPE_COOKING 
							|| type == ITEMTYPE_JEWEL 
							|| type == ITEMTYPE_RANDOM 
							|| type == ITEMTYPE_HAIRDYE 
							|| type == ITEMTYPE_GACHACOIN 
							|| type == ITEMTYPE_CHARM
#if defined( PRE_ADD_EASYGAMECASH )
							|| type == ITEMTYPE_ALLGIVECHARM
#endif
#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
							|| type == ITEMTYPE_CHARMRANDOM
#endif
							|| type == ITEMTYPE_POTENTIAL_JEWEL) ? true : false;
		if( m_bCountMoreThanOne && pCurItem->GetMaxOverlapCount() == 1 ) m_bCountMoreThanOne = false;
	}

	m_nCountForRender = (count != NO_COUNTABLE_RENDER && count != ITEM_ORIGINAL_COUNT) ? count : GetSlotOriginalCount();
	m_nSplitCount = 0;
	
	RefreshIcon(*pItem);
}

void CDnSlotButton::RefreshIcon(const MIInventoryItem& item)
{
	if( m_pItem == NULL )
		return;

	int nIconIndex = item.GetIconImageIndex();
	switch(m_pItem->GetType())
	{
	case MIInventoryItem::Item:
		{
			m_hItemIcon = GetItemIcon( nIconIndex, m_IconUV );
		}
		break;
	case MIInventoryItem::Skill:
		{
			/*
			int nIndex = nIconIndex/SKILL_TEXTURE_ICON_COUNT;
			m_hSkillIcon = m_hSkillIconList[nIndex];
			m_hSkillGrayIcon = m_hSkillGrayIconList[nIndex];
			nIconIndex %= SKILL_TEXTURE_ICON_COUNT;

			CalcButtonUV( nIconIndex, m_hSkillIcon, m_IconUV, SKILLSLOT_ICON_XSIZE, SKILLSLOT_ICON_YSIZE );
			*/
			GetSkillIcon( nIconIndex, m_IconUV, m_hSkillIcon, m_hSkillGrayIcon );
		}
		break;
	default:
		ASSERT(0&&"CDnSlotButton::SetItem");
		break;
	}
}

void CDnSlotButton::ResetSlot()
{
	m_nIconIndex = -1;
	m_nItemID = 0;
	if( m_pItem ) {
		m_pItem->RemoveOwnerShip( this );
	}
	m_pItem = NULL;
	//DisableSplitMode(true);

	SUIControlProperty *pProperty = GetProperty();
	pProperty->nTooltipStringIndex = m_nTooltipIndexBack;
}

void CDnSlotButton::ShowTooltip( float fX, float fY )
{
	if( ST_FARM_CONDITION == m_SlotType )
		return;

	CDnTooltipDlg *pDialog = GetInterface().GetTooltipDialog();
	if( !pDialog ) return;

//#ifdef PRE_ADD_MAINQUEST_UI
//	if(pDialog->IsShowMainQuestTooltip())
//		return;
//#endif

	bool bHide = false;
	if( EtInterface::drag::IsValid() ) bHide = true;
	if( m_itemType == MIInventoryItem::Item || m_itemType == MIInventoryItem::Skill )
		if( IsEmptySlot() ) bHide = true;
	if( m_itemType == MIInventoryItem::Gesture || MIInventoryItem::SecondarySkill )
		if( !GetItemID() ) bHide = true;

	if( bHide )
	{
		pDialog->HideTooltip();
	}
	else
	{
#ifdef PRE_ADD_OVERLAP_SETEFFECT
		if(pDialog->IsShowOverlapToolTip() == false)
			pDialog->ShowTooltip( this, fX, fY );
#else
		pDialog->ShowTooltip( this, fX, fY );
#endif
		
	}
}

bool CDnSlotButton::IsRenderCount()
{
	if( m_itemType != MIInventoryItem::Item ) return false;
	if( !m_bRenderCount )	return false;

	int minCount = (m_bCountMoreThanOne) ? 1 : 2;
	if (m_nCountForRender < minCount)
		return false;

	return true;
}

bool CDnSlotButton::IsRenderDragCount() const
{
	bool bRet = true;
	if (m_itemType != MIInventoryItem::Item)
		bRet = false;
	if (m_bRenderCount == false)
		bRet = false;
	// Show even 1 item while dragging.
	// Instead, do not display when the current OverlapCount is 1.
	if (m_nSplitCount == 1 && GetSlotOriginalCount() == 1)
		bRet = false;
	if (m_nSplitCount < 1)
		bRet = false;

	return bRet;
}

void CDnSlotButton::RenderCount( SUIElement *pElement )
{
	if( !pElement ) return;
	if( !IsRenderCount() ) return;

	// Removed since it is no longer used.
	//m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord );

	WCHAR wszBuffer[32]={0};

	_itow_s( m_nCountForRender, wszBuffer, 32, 10 );

	SUICoord uiTextCoord;
	m_pParent->CalcTextRect( wszBuffer, pElement, uiTextCoord );
	uiTextCoord.fX = m_Property.UICoord.Right()-uiTextCoord.fWidth;
	uiTextCoord.fX -= 2 / ( float )DEFAULT_UI_SCREEN_WIDTH;		// Setting 0.003 for pixel error makes it slightly blurry.
	uiTextCoord.fY = m_Property.UICoord.Bottom()-uiTextCoord.fHeight;
	uiTextCoord.fY -= 2 / ( float )DEFAULT_UI_SCREEN_HEIGHT;
	//m_pParent->DrawDlgText( wszBuffer, pElement, pElement->TextureColor.dwCurrentColor, uiTextCoord );
	//m_pParent->DrawDlgText( wszBuffer, pElement, EtInterface::textcolor::GOLD, uiTextCoord );

	// Render font color using m_dwFontColor instead of Element's FontColor.
	if( GetSlotType() == ST_SET_FONTCOLOR )
		m_pParent->DrawDlgText( wszBuffer, pElement, m_dwFontColor, uiTextCoord );	
	else
		m_pParent->DrawDlgText( wszBuffer, pElement, pElement->FontColor.dwCurrentColor, uiTextCoord );
}

void CDnSlotButton::RenderExpire( SUIElement *pElement )
{
	WCHAR wszBuffer[32]={0};
	_itow_s( m_nExpireCount, wszBuffer, 32, 10 );

	SUICoord uiTextCoord;
	m_pParent->CalcTextRect( wszBuffer, pElement, uiTextCoord );
	uiTextCoord.fX = m_Property.UICoord.fX;
	uiTextCoord.fX -= 2 / ( float )DEFAULT_UI_SCREEN_WIDTH;		// Setting 0.003 for pixel error makes it slightly blurry.
	uiTextCoord.fY = m_Property.UICoord.fY;
	uiTextCoord.fY -= 2 / ( float )DEFAULT_UI_SCREEN_HEIGHT;
	//m_pParent->DrawDlgText( wszBuffer, pElement, pElement->TextureColor.dwCurrentColor, uiTextCoord );
	//m_pParent->DrawDlgText( wszBuffer, pElement, EtInterface::textcolor::GOLD, uiTextCoord );
	m_pParent->DrawDlgText( wszBuffer, pElement, textcolor::RED, uiTextCoord );
}

void CDnSlotButton::RenderDragCount( SUIElement *pElement, SUICoord &Coord )
{
	if( !pElement ) return;
	if( !IsRenderDragCount() ) return;

	WCHAR wszBuffer[32]={0};
	_itow_s( m_nSplitCount, wszBuffer, 32, 10 );

	SUICoord uiTextCoord;
	m_pParent->CalcTextRect( wszBuffer, pElement, uiTextCoord );
	uiTextCoord.fX = Coord.Right()-uiTextCoord.fWidth;
	uiTextCoord.fX -= 2 / ( float )DEFAULT_UI_SCREEN_WIDTH;		// Setting 0.003 for pixel error makes it slightly blurry.
	uiTextCoord.fY = Coord.Bottom()-uiTextCoord.fHeight;
	uiTextCoord.fY -= 2 / ( float )DEFAULT_UI_SCREEN_HEIGHT;


	DWORD dwFontColor = pElement->FontColor.dwCurrentColor;
	DWORD dwFontFormat = pElement->dwFontFormat;
	dwFontFormat |= DT_WORDBREAK;

	SFontDrawEffectInfo Info;
	Info.nDrawType = pElement->nDrawType;
	Info.nWeight = pElement->nWeight;
	Info.fAlphaWeight = pElement->fAlphaWeight;
	Info.dwFontColor = dwFontColor;
	Info.dwEffectColor = pElement->ShadowFontColor.dwCurrentColor;
	CEtFontMng::GetInstance().DrawTextW( pElement->nFontIndex, pElement->nFontHeight, wszBuffer, pElement->dwFontFormat, uiTextCoord, -1, Info, true, 0.0f, 0);
}

void CDnSlotButton::OnRefreshTooltip()
{
	CDnTooltipDlg *pTooltipDlg = GetInterface().GetTooltipDialog();
	if( !pTooltipDlg ) return;
	if( !pTooltipDlg->IsShow() ) return;

	if( EtInterface::drag::IsValid() || IsEmptySlot() )
	{
		pTooltipDlg->HideTooltip();
	}
	else
	{
		pTooltipDlg->RefreshTooltip( this );
	}
}

void CDnSlotButton::ReleaseOwnerShip()
{
	m_pItem = NULL;
}

int CDnSlotButton::GetSlotOriginalCount() const
{
	int ret = 0;
	if (m_pItem)
	{
		switch(m_pItem->GetType())
		{
		case MIInventoryItem::Item:
			{
				if( CDnItem::IsCashItem( m_pItem->GetClassID() ) )
					ret = (m_SlotType == ST_QUICKSLOT) ? GetItemTask().GetCashInventory().GetItemCount(m_pItem->GetClassID()) : m_pItem->GetOverlapCount();
				else
					ret = (m_SlotType == ST_QUICKSLOT) ? GetItemTask().GetCharInventory().GetItemCount(m_pItem->GetClassID()) : m_pItem->GetOverlapCount();
			}
			break;
		default:
			{
				ret = 0;
			}
			break;
		}
	}

	return ret;
}

void CDnSlotButton::EnableSplitMode(int splitCount)
{
	if (splitCount == ITEM_ORIGINAL_COUNT)
		splitCount = GetSlotOriginalCount();

	m_nSplitCount = splitCount;

	if( m_nCountForRender > 0 )
		m_nCountForRender -= m_nSplitCount;
}

bool CDnSlotButton::IsSplitting() const
{
	return (m_nSplitCount > 0);
}

void CDnSlotButton::DisableSplitMode(bool bCancel)
{
	if (bCancel)
		m_nCountForRender = GetSlotOriginalCount();

	m_nSplitCount = INVALID_SPLIT_COUNT;
}

bool CDnSlotButton::IsDragMode() const
{
	CDnSlotButton* pDragButton = (CDnSlotButton*)drag::GetControl();
	if (pDragButton && pDragButton == this)
		return true;

	return false;
}

void CDnSlotButton::OnCmdDrag(UIDragCmdParam param)
{
	if (param == UI_DRAG_CMD_CANCEL)
		DisableSplitMode(true);
}

// void CDnSlotButton::HoldDragMode(int dragCount)
// {
// 	drag::SetControl(this);
// 
// 	EnableDragMode(dragCount);
// }
// 
// void CDnSlotButton::ReleaseDragMode(bool bCancel)
// {
// 	CDnSlotButton *pDragButton;
// 	pDragButton = (CDnSlotButton*)drag::GetControl();
// 
// 	if (pDragButton)
// 	{
// 		pDragButton->DisableDragMode(bCancel);
// 		if (drag::GetControl() == this)
// 			drag::ReleaseControl();
// 	}
// }