#include "StdAfx.h"
#include "DnSkillUpButton.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSkillUpButton::CDnSkillUpButton( CEtUIDialog *pParent )
	: CEtUIButton( pParent )
	, m_bBlink(false)
	, m_fBlinkTime(0.0f)
	, m_ctlState(UI_STATE_NORMAL)
	, m_nTimeBack(0)
	//, m_iTooltipUIStringBackup( -1 )
{
}

CDnSkillUpButton::~CDnSkillUpButton(void)
{
}

void CDnSkillUpButton::Initialize( SUIControlProperty *pProperty )
{
	CEtUIButton::Initialize( pProperty );

	//m_iTooltipUIStringBackup = pProperty->nTooltipStringIndex;

	// Since the skill system was revamped into a skill tree format
	// The skill-up button does not use the tooltip index configured in the tool.
	m_Property.nTooltipStringIndex = 0;
}

void CDnSkillUpButton::Render( float fElapsedTime )
{
	m_CurrentState = UI_STATE_NORMAL;

	m_MoveButtonCoord = m_Property.UICoord;
	m_MoveBaseCoordLeft = m_BaseCoordLeft;
	m_MoveBaseCoordMid = m_BaseCoordMid;
	m_MoveBaseCoordRight = m_BaseCoordRight;

	if( !IsShow() )
	{
		m_CurrentState = UI_STATE_HIDDEN;
	}
	else if( !IsEnable() )
	{
		m_CurrentState = UI_STATE_DISABLED;
		//m_Property.nTooltipStringIndex = 0;
	}
	else if( m_bPressed )
	{
		m_CurrentState = UI_STATE_PRESSED;

		m_MoveButtonCoord.fX += m_Property.ButtonProperty.fMoveOffsetHori;
		m_MoveButtonCoord.fY += m_Property.ButtonProperty.fMoveOffsetVert;

		m_MoveBaseCoordLeft.fX += m_Property.ButtonProperty.fMoveOffsetHori;
		m_MoveBaseCoordLeft.fY += m_Property.ButtonProperty.fMoveOffsetVert;

		m_MoveBaseCoordMid.fX += m_Property.ButtonProperty.fMoveOffsetHori;
		m_MoveBaseCoordMid.fY += m_Property.ButtonProperty.fMoveOffsetVert;

		m_MoveBaseCoordRight.fX += m_Property.ButtonProperty.fMoveOffsetHori;
		m_MoveBaseCoordRight.fY += m_Property.ButtonProperty.fMoveOffsetVert;
	}
	else if( m_bMouseEnter )
	{
		m_CurrentState = UI_STATE_MOUSEENTER;
	}
	else if( m_bFocus )
	{
		m_CurrentState = UI_STATE_FOCUS;
	}
	//else
	//if( 0 == m_Property.nTooltipStringIndex )
	//	m_Property.nTooltipStringIndex = m_iTooltipUIStringBackup;

	UpdateBlendRate();

	SUIElement *pElement = GetElement(0);

	if( !pElement )
		return;

	if( m_CurrentState == UI_STATE_MOUSEENTER )
	{
		pElement = GetElement(1);
	}
	else if( m_CurrentState == UI_STATE_DISABLED )
	{
		pElement = GetElement(2);
	}

	if( !pElement )
		return;

	if( m_bBlink )
	{
		LOCAL_TIME time = CGlobalInfo::GetInstance().m_LocalTime;

		int nAlpha = 255;
		int BLEND_TIME = 500;
		float BLEND_RANGE  = 1.0f;
		int nTemp = (int)time%(BLEND_TIME+BLEND_TIME);
		if( nTemp < BLEND_TIME ) 
		{
			nAlpha = int(( BLEND_RANGE / (float)BLEND_TIME ) * nTemp * 255.0f);
		}
		else 
		{
			nAlpha = int(( BLEND_RANGE / (float)BLEND_TIME ) * ( (BLEND_TIME+BLEND_TIME) - nTemp ) *  255.0f);
		}

		nAlpha = min(nAlpha, 255);

		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, D3DCOLOR_ARGB(nAlpha,255,255,255), m_MoveButtonCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, D3DCOLOR_ARGB(nAlpha,255,255,255), m_MoveButtonCoord );
	}
	else
	{
		pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );
	}
}

void CDnSkillUpButton::SetBlink(  bool bBlink )
{
	m_bBlink = bBlink;
	m_fBlinkTime = 0.0f;
	m_nTimeBack = 0;
	m_ctlState = UI_STATE_DISABLED;
}