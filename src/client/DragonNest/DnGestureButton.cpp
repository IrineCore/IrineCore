#include "StdAfx.h"
#include "DnGestureButton.h"
#include "DnCustomControlCommon.h"
#include "DNGestureTask.h"
#include "DnActor.h"
#include "DnPlayerActor.h"
#include "DnTableDB.h"
#include "DnWorld.h"

#if defined(PRE_ADD_GUILD_GESTURE)
#include "DnGuildTask.h"
#endif // PRE_ADD_GUILD_GESTURE

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnLifeSkillButton::CDnLifeSkillButton( CEtUIDialog *pParent )
: CDnSlotButton( pParent )
{
	m_itemType = MIInventoryItem::Gesture;
	m_bCash = false;
	m_nItemCondition = -1;
#ifdef PRE_MOD_SELECT_CHAR
	m_bForceRender = false;
#endif // PRE_MOD_SELECT_CHAR
}

CDnLifeSkillButton::~CDnLifeSkillButton(void)
{
	SAFE_RELEASE_SPTR( m_hShortCutIcon );
	for( int i=0; i<GESTURE_TEXTURE_COUNT; i++ )
	{
		SAFE_RELEASE_SPTR( m_hGestureIconList[i] );
	}
	for( int i=0; i<SECONDARYSKILL_TEXTURE_COUNT; i++ )
	{
		SAFE_RELEASE_SPTR( m_hSecondarySkillIconList[i] );
	}
}

void CDnLifeSkillButton::Initialize( SUIControlProperty *pProperty )
{
	CEtUIButton::Initialize( pProperty );

	m_hShortCutIcon = LoadResource( CEtResourceMng::GetInstance().GetFullName( "MainBarShortCut.dds" ).c_str(), RT_TEXTURE );
	m_hGestureIconList[0] = LoadResource( CEtResourceMng::GetInstance().GetFullName( "GestureIcon01.dds" ).c_str(), RT_TEXTURE );
	m_hSecondarySkillIconList[0] = LoadResource( CEtResourceMng::GetInstance().GetFullName( "LifeSkillIcon.dds" ).c_str(), RT_TEXTURE );

	m_nItemCondition = -1;

	m_ShortCutCoord = m_Property.UICoord;
	m_ShortCutCoord.fWidth = (float)QUICKSLOT_SHORCUT_ICON_XSIZE / DEFAULT_UI_SCREEN_WIDTH;
	m_ShortCutCoord.fHeight = (float)QUICKSLOT_SHORCUT_ICON_YSIZE / DEFAULT_UI_SCREEN_HEIGHT;
	m_ShortCutCoord.fY = m_Property.UICoord.Bottom() - m_ShortCutCoord.fHeight;
}

void CDnLifeSkillButton::SetHotKey( int nKey )
{
	CEtUIControl::SetHotKey( nKey );
	CalcHotKeyUV( nKey, m_hShortCutIcon, m_ShortCutUV, QUICKSLOT_SHORCUT_ICON_XSIZE, QUICKSLOT_SHORCUT_ICON_YSIZE );
}

void CDnLifeSkillButton::SetGestureInfo( int nGestureID )
{
	CDnGestureTask::SGestureInfo *pInfo;
	pInfo = GetGestureTask().GetGestureInfoFromID( nGestureID );

	if( pInfo )
	{
#if defined(PRE_ADD_GUILD_GESTURE)
		//Do not register to quick slot if guild is not set.
		if (pInfo->_Type == CDnGestureTask::eGestureType::GestureType_GuildReward)
		{
			TGuild *pGuild = GetGuildTask().GetGuildInfo();
			if (pGuild == NULL || pGuild->IsSet() == false)
				return;
		}
#endif // PRE_ADD_GUILD_GESTURE

		// Use m_nItemID as the distinguishing ID.
		m_nItemID = pInfo->nID;
		m_hGestureIcon = m_hGestureIconList[pInfo->nIconID/GESTURE_TEXTURE_ICON_COUNT];
		CalcButtonUV( pInfo->nIconID % GESTURE_TEXTURE_ICON_COUNT, m_hGestureIcon, m_IconUV, GESTURESLOT_ICON_XSIZE, GESTURESLOT_ICON_YSIZE );
		m_bCash = (pInfo->nUnlockLevel == -1) ? true : false;
		m_itemType = MIInventoryItem::Gesture;
	}
}

#ifdef PRE_MOD_SELECT_CHAR

void CDnLifeSkillButton::SetGestureIcon( int nIconID )
{
	if( nIconID < 0 )
		return;

	m_bForceRender = true;
	m_nItemCondition = MIInventoryItem::Usable;
	
	m_hGestureIcon = m_hGestureIconList[nIconID/GESTURE_TEXTURE_ICON_COUNT];
	CalcButtonUV( nIconID % GESTURE_TEXTURE_ICON_COUNT, m_hGestureIcon, m_IconUV, GESTURESLOT_ICON_XSIZE, GESTURESLOT_ICON_YSIZE );
}

#endif // PRE_MOD_SELECT_CHAR

void CDnLifeSkillButton::SetSecondarySkillInfo( int nSecondarySkillID )
{
#if defined( PRE_ADD_SECONDARY_SKILL )
	DNTableFileFormat* pSoxSkill = GetDNTable( CDnTableDB::TSecondarySkill );
	if( pSoxSkill == NULL ) return;
	if( !pSoxSkill->IsExistItem( nSecondarySkillID ) ) return;

	m_nItemID = nSecondarySkillID;
	int nIconID = pSoxSkill->GetFieldFromLablePtr( m_nItemID, "_SecondarySkillIcon" )->GetInteger();
	m_hGestureIcon = m_hSecondarySkillIconList[nIconID/GESTURE_TEXTURE_ICON_COUNT];
	CalcButtonUV( nIconID % GESTURE_TEXTURE_ICON_COUNT, m_hGestureIcon, m_IconUV, GESTURESLOT_ICON_XSIZE, GESTURESLOT_ICON_YSIZE );
	m_bCash = false;
	m_itemType = MIInventoryItem::SecondarySkill;
#endif	// #if defined( PRE_ADD_SECONDARY_SKILL )
}

bool CDnLifeSkillButton::IsUsable()
{
	if( m_nItemCondition == MIInventoryItem::Usable ) return true;
	return false;
}

void CDnLifeSkillButton::Process( float fElapsedTime )
{
	if( !CDnActor::s_hLocalActor ) return;

	// Item conditions in gestures are as follows.
	// Usable - Can be used. Purchased or given by default. (Only then can it be picked up. Can be registered to quickslot)
	// Unusable - Player is dead or hasn't purchased it. Shaded including border.
	// NotEnoughCondition - Temporarily unusable state when jumping etc. while Usable. Only the inner icon is shaded.
	if( m_nItemID )
	{
		if( m_itemType == MIInventoryItem::Gesture )
		{
			if( CDnGestureTask::IsActive() )
			{
				CDnGestureTask::SGestureInfo *pInfo;
				pInfo = GetGestureTask().GetGestureInfoFromID( m_nItemID );
				if( pInfo )
				{
					if( pInfo->bUsable ) m_nItemCondition = MIInventoryItem::Usable;
					else m_nItemCondition = MIInventoryItem::Unusable;

#if defined(PRE_ADD_GUILD_GESTURE)
					if( pInfo->_Type == CDnGestureTask::eGestureType::GestureType_GuildWarWin && pInfo->bUsable )
#else
					if( pInfo->nUnlockLevel == -2 && pInfo->bUsable )
#endif // PRE_ADD_GUILD_GESTURE
					{
						if( CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeVillage )
							m_nItemCondition = MIInventoryItem::Unusable;
					}
				}
			}
		}
		else if( m_itemType == MIInventoryItem::SecondarySkill )
		{
			// Nothing special to check for SecondarySkill.
			m_nItemCondition = MIInventoryItem::Usable;
		}

		if( CDnActor::s_hLocalActor->IsDie() )
			m_nItemCondition = MIInventoryItem::Unusable;

		if( m_nItemCondition == MIInventoryItem::Usable )
		{
			// Check jump or movement restriction. CDnGroundMovableChecker check.
			bool bResult = false;
			if( !CDnActor::s_hLocalActor->IsAir() )
			{
				if( (CDnActor::s_hLocalActor->GetStateEffect() & CDnActorState::Cant_Move) == CDnActorState::Cant_Move )
					bResult = true;
				else if( CDnActor::s_hLocalActor->IsMovable() )
					bResult = true;
			}

			if( !bResult )
				m_nItemCondition = MIInventoryItem::NotEnoughCondition;
		}

		CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		if(pPlayer)
		{
			if( pPlayer->IsSwapSingleSkin() )
				m_nItemCondition = MIInventoryItem::Unusable;

			if(pPlayer->IsObserver())
				m_nItemCondition = MIInventoryItem::Unusable;

			if(pPlayer->IsVehicleMode())
				m_nItemCondition = MIInventoryItem::Unusable;
			if(pPlayer->IsTransformMode())
				m_nItemCondition = MIInventoryItem::Unusable;
		}
	}
}

void CDnLifeSkillButton::Render( float fElapsedTime )
{
	CDnSlotButton::Render(fElapsedTime);

#ifdef PRE_MOD_SELECT_CHAR
	if( m_nItemID || m_bForceRender )
#else // PRE_MOD_SELECT_CHAR
	if( m_nItemID )
#endif // PRE_MOD_SELECT_CHAR
	{
		DWORD dwColor = m_dwIconColor;
		if( m_nItemCondition != MIInventoryItem::Usable )
			dwColor = 0xff808080;
		m_pParent->DrawSprite( m_hGestureIcon, m_IconUV, dwColor, m_Property.UICoord );

		SUIElement *pElement = NULL;
		if( m_itemType == MIInventoryItem::Gesture ) pElement = GetElement(0);
		else if( m_itemType == MIInventoryItem::SecondarySkill ) pElement = GetElement(3);
		dwColor = m_dwIconColor;
		if( m_nItemCondition == MIInventoryItem::Unusable )
			dwColor = 0xff808080;
		if( pElement )
		{
			if( m_bExistTemplateTexture )
				m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, dwColor, m_Property.UICoord );
			else
				m_pParent->DrawSprite( pElement->UVCoord, dwColor, m_Property.UICoord );
		}

		if( m_itemType == MIInventoryItem::Gesture ) pElement = GetElement(1);
		else if( m_itemType == MIInventoryItem::SecondarySkill ) pElement = GetElement(4);
		if( pElement && IsDragMode() )
		{
			if( m_bExistTemplateTexture )
				m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord );
			else
				m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord );
		}

		pElement = GetElement(2);
		if( pElement && m_bCash ) {
			if( m_bExistTemplateTexture )
				m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord );
			else
				m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord );
		}
	}

	if( GetSlotType() == ST_LIFESKILL_QUICKSLOT )
	{
		if( 0 < m_Property.nHotKey )
			m_pParent->DrawSprite( m_hShortCutIcon, m_ShortCutUV, m_dwIconColor, m_ShortCutCoord );
	}
}

void CDnLifeSkillButton::RenderDrag( float fElapsedTime )
{
	if( IsDragMode() == false )
		return;

	SUIElement *pElement = NULL;
	if( m_itemType == MIInventoryItem::Gesture ) pElement = GetElement(0);
	else if( m_itemType == MIInventoryItem::SecondarySkill ) pElement = GetElement(3);

	POINT CursorPoint;
	GetCursorPos( &CursorPoint );
	ScreenToClient( GetEtDevice()->GetHWnd(), &CursorPoint );
	m_CursorCoord = m_Property.UICoord;
	m_CursorCoord.fWidth /= m_pParent->GetScreenWidthRatio();
	m_CursorCoord.fHeight /= m_pParent->GetScreenHeightRatio();
	m_CursorCoord.fX = CursorPoint.x / ( float )GetEtDevice()->Width() - m_CursorCoord.fWidth * 0.5f;
	m_CursorCoord.fY = CursorPoint.y / ( float )GetEtDevice()->Height() - m_CursorCoord.fHeight * 0.5f;

	CEtSprite::GetInstance().DrawSprite( ( EtTexture * )m_hGestureIcon->GetTexturePtr(), m_hGestureIcon->Width(), m_hGestureIcon->Height(), 
		m_IconUV, 0xffffffff, m_CursorCoord, 0.0f );

	EtTextureHandle hTexture;
	hTexture = m_pParent->GetUITexture();
	if( m_bExistTemplateTexture )
		hTexture = m_Template.m_hTemplateTexture;
	if( !hTexture ) return;

	if( pElement )
	{
		SUICoord UVCoord = pElement->UVCoord;
		if( m_bExistTemplateTexture )
			UVCoord = pElement->TemplateUVCoord;
		CEtSprite::GetInstance().DrawSprite( ( EtTexture * )hTexture->GetTexturePtr(), hTexture->Width(), hTexture->Height(), 
			UVCoord, pElement->TextureColor.dwCurrentColor, m_CursorCoord, 0.0f );
	}
}