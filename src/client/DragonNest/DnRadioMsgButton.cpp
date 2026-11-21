#include "StdAfx.h"
#include "DnRadioMsgButton.h"
#include "DnCustomControlCommon.h"
#include "DnRadioMsgTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnRadioMsgButton::CDnRadioMsgButton( CEtUIDialog *pParent )
: CDnSlotButton( pParent )
{
	m_itemType = MIInventoryItem::RadioMsg;
}

CDnRadioMsgButton::~CDnRadioMsgButton(void)
{
	for( int i=0; i<RADIOMSG_TEXTURE_COUNT; i++ )
	{
		SAFE_RELEASE_SPTR( m_hRadioMsgIconList[i] );
	}
}

void CDnRadioMsgButton::Initialize( SUIControlProperty *pProperty )
{
	CEtUIButton::Initialize( pProperty );

	m_hRadioMsgIconList[0] = LoadResource( CEtResourceMng::GetInstance().GetFullName( "RadioMsgIcon01.dds" ).c_str(), RT_TEXTURE );
}

void CDnRadioMsgButton::SetRadioMsgInfo( int nRadioMsgID )
{
	CDnRadioMsgTask::SRadioMsgInfo *pInfo;
	pInfo = GetRadioMsgTask().GetRadioMsgInfoFromID( nRadioMsgID );

	if( pInfo )
	{
		// Use m_nItemID as the distinguishing ID.
		m_nItemID = pInfo->nID;
		m_hRadioMsgIcon = m_hRadioMsgIconList[pInfo->nIconID/RADIOMSG_TEXTURE_ICON_COUNT];
		CalcButtonUV( pInfo->nIconID % RADIOMSG_TEXTURE_ICON_COUNT, m_hRadioMsgIcon, m_IconUV, RADIOMSGSLOT_ICON_XSIZE, RADIOMSGSLOT_ICON_YSIZE );
	}
}

void CDnRadioMsgButton::Render( float fElapsedTime )
{
	CDnSlotButton::Render(fElapsedTime);

	if( !m_nItemID )
		return;

	DWORD dwColor = m_dwIconColor;

	if( IsDragMode() || !IsEnable() )
	{
		dwColor = 0xff808080;
	}

	m_pParent->DrawSprite( m_hRadioMsgIcon, m_IconUV, dwColor, m_Property.UICoord );

	SUIElement *pElement = GetElement(0);
	if( pElement )
	{
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, dwColor, m_Property.UICoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, dwColor, m_Property.UICoord );
	}
}

void CDnRadioMsgButton::RenderDrag( float fElapsedTime )
{
	if (IsDragMode() == false)
		return;

	SUIElement *pElement;
	pElement = GetElement(0);

	POINT CursorPoint;
	GetCursorPos( &CursorPoint );
	ScreenToClient( GetEtDevice()->GetHWnd(), &CursorPoint );
	m_CursorCoord = m_Property.UICoord;
	m_CursorCoord.fWidth /= m_pParent->GetScreenWidthRatio();
	m_CursorCoord.fHeight /= m_pParent->GetScreenHeightRatio();
	m_CursorCoord.fX = CursorPoint.x / ( float )GetEtDevice()->Width() - m_CursorCoord.fWidth * 0.5f;
	m_CursorCoord.fY = CursorPoint.y / ( float )GetEtDevice()->Height() - m_CursorCoord.fHeight * 0.5f;

	CEtSprite::GetInstance().DrawSprite( ( EtTexture * )m_hRadioMsgIcon->GetTexturePtr(), m_hRadioMsgIcon->Width(), m_hRadioMsgIcon->Height(), 
		m_IconUV, 0xffffffff, m_CursorCoord, 0.0f );

	EtTextureHandle hTexture;
	hTexture = m_pParent->GetUITexture();
	if( m_bExistTemplateTexture )
		hTexture = m_Template.m_hTemplateTexture;
	if( !hTexture ) return;
	if( !pElement ) return;

	SUICoord UVCoord = pElement->UVCoord;
	if( m_bExistTemplateTexture )
		UVCoord = pElement->TemplateUVCoord;

	CEtSprite::GetInstance().DrawSprite( ( EtTexture * )hTexture->GetTexturePtr(), hTexture->Width(), hTexture->Height(), 
		UVCoord, pElement->TextureColor.dwCurrentColor, m_CursorCoord, 0.0f );
}