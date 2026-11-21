#include "StdAfx.h"
#include "DnDamageCount.h"
#include "DnInterface.h"

#include "DnState.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnDamageCount::CDnDamageCount(CEtUIDialog *pParent)
	: CDnCountCtl( pParent )
	, m_fSizeValue(0.0f)
	, m_fVelocity(0.0f)
	, m_fTotalElapsedTime(0.0f)
	, m_bImpactInc(false)
	, m_bImpactDec(false)
	, m_DamageType(Damage_Normal)
{
}

CDnDamageCount::~CDnDamageCount(void)
{
}

void CDnDamageCount::Process( float fElapsedTime )
{
	if( m_nValue == -1 )
	{
		m_fTotalElapsedTime += fElapsedTime;

		if( m_bImpactInc )
		{
			if( m_fSizeValue <= 1.0f )
			{
				if( m_fSizeValue <= 0.8f )
				{
					m_fVelocity += 33.0f * m_fTotalElapsedTime;
				}
				else
				{
					m_fVelocity = (3.0f * m_fTotalElapsedTime);
				}
				
				m_fSizeValue += ( m_fVelocity * fElapsedTime ) * 0.4f;
			}
			else
			{
				m_fTotalElapsedTime = 0.0f;
				m_fVelocity = 0.0f;
				m_fSizeValue = 1.0f;
				m_bImpactInc = false;
				m_bImpactDec = true;
			}
		}
		else if( m_bImpactDec )
		{
			if( m_fSizeValue >= 0.0f )
			{
				if( m_fSizeValue >= 0.8f )
				{
					m_fVelocity = (3.0f * m_fTotalElapsedTime);
				}
				else
				{
					m_fVelocity += 33.0f * m_fTotalElapsedTime;
				}

				m_fSizeValue -= ( m_fVelocity * fElapsedTime ) * 0.4f;
			}
			else
			{
				m_fSizeValue = 0.0f;
				m_bImpactDec = false;
			}
		}
	}
}

void CDnDamageCount::Render( float fElapsedTime )
{
	if( m_nValue == -1 )
	{
		if( m_DamageType == Damage_Critical || m_DamageType == Damage_CriticalRes )
		{
			UI_CONTROL_STATE CurrentState = UI_STATE_NORMAL;
			if( !IsShow() ) CurrentState = UI_STATE_HIDDEN;
			m_Color.Blend( CurrentState, fElapsedTime, 0.8f );

			SUIElement *pElement(NULL);
			SUICoord uiCoord;

			//pElement = GetElement(11);
			////uiCoord = m_Property.UICoord;
			//float fWidth = pElement->fTextureWidth*m_fSizeValue;
			//if( fWidth > pElement->fTextureWidth ) fWidth = pElement->fTextureWidth;
			//float fHeight = pElement->fTextureHeight*m_fSizeValue;
			//if( fHeight > pElement->fTextureHeight ) fHeight = pElement->fTextureHeight;
			//uiCoord.fX = /*m_Property.UICoord.fX+*/(m_Property.UICoord.fWidth-fWidth)/2.0f;
			//uiCoord.fY = /*m_Property.UICoord.fY+*/(m_Property.UICoord.fHeight-fHeight)/2.0f;
			//uiCoord.SetSize( fWidth, fHeight );
			//m_pParent->DrawSprite( pElement->UVCoord, m_Color.dwCurrentColor, uiCoord );

			int nElementIndex = (m_DamageType == Damage_Critical) ? 10 : 23;
			pElement = GetElement(nElementIndex);
			uiCoord.fX = /*m_Property.UICoord.fX+*/(m_Property.UICoord.fWidth-pElement->fTextureWidth)/2.0f;
			uiCoord.fY = /*m_Property.UICoord.fY+*/(m_Property.UICoord.fHeight-pElement->fTextureHeight)/2.0f;
			uiCoord.SetSize( pElement->fTextureWidth, pElement->fTextureHeight );
			if( m_bExistTemplateTexture )
				m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, m_Color.dwCurrentColor, uiCoord );
			else
				m_pParent->DrawSprite( pElement->UVCoord, m_Color.dwCurrentColor, uiCoord );
			return;
		}
		else
		{
			UI_CONTROL_STATE CurrentState = UI_STATE_NORMAL;
			if( !IsShow() ) 
			{
				CurrentState = UI_STATE_HIDDEN;
				m_Color.Blend( CurrentState, fElapsedTime, 0.0f );
			}
			else
				m_Color.Blend( CurrentState, fElapsedTime, 0.8f );

			SUIElement *pElement(NULL);
			SUICoord uiCoord;

			// Unlike critical, resist has no impact image behind it.
#ifdef PRE_ADD_DECREASE_EFFECT
			int nElementIndex = 22;
			if( m_DamageType == Damage_Resist )
				nElementIndex = 22;
			else if( m_DamageType == Damage_DecreaseEffect )
				nElementIndex = 39;
			
			pElement = GetElement( nElementIndex );
#else // PRE_ADD_DECREASE_EFFECT
			pElement = GetElement(22);
#endif // PRE_ADD_DECREASE_EFFECT
			uiCoord.fX = /*m_Property.UICoord.fX*/+(m_Property.UICoord.fWidth-pElement->fTextureWidth)/2.0f;
			uiCoord.fY = /*m_Property.UICoord.fY*/+(m_Property.UICoord.fHeight-pElement->fTextureHeight)/2.0f;
			uiCoord.SetSize( pElement->fTextureWidth, pElement->fTextureHeight );
			if( m_bExistTemplateTexture )
				m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, m_Color.dwCurrentColor, uiCoord );
			else
				m_pParent->DrawSprite( pElement->UVCoord, m_Color.dwCurrentColor, uiCoord );
			return;
		}
	}

	CDnCountCtl::Render( fElapsedTime );
}

float CDnDamageCount::GetControlWidth()
{
	int nElementIndex = -1;
	switch( m_DamageType )
	{
	case Damage_Critical:	nElementIndex = 10;	break;
	case Damage_Resist:		nElementIndex = 22;	break;
	case Damage_CriticalRes:nElementIndex = 23;	break;
#ifdef PRE_ADD_DECREASE_EFFECT
	case Damage_DecreaseEffect: nElementIndex = 39; break;
#endif // PRE_ADD_DECREASE_EFFECT
	}

	if( nElementIndex != -1 )
	{
		SUIElement *pElement = GetElement(nElementIndex);
		if( pElement ) {
			return pElement->fTextureWidth*m_fSizeValue;
		}
	}

	return CDnCountCtl::GetControlWidth();
}

float CDnDamageCount::GetControlHeight()
{
	int nElementIndex = -1;
	switch( m_DamageType )
	{
	case Damage_Critical:	nElementIndex = 10;	break;
	case Damage_Resist:		nElementIndex = 22;	break;
	case Damage_CriticalRes:nElementIndex = 23;	break;
#ifdef PRE_ADD_DECREASE_EFFECT
	case Damage_DecreaseEffect: nElementIndex = 39; break;
#endif // PRE_ADD_DECREASE_EFFECT
	}

	if( nElementIndex != -1 )
	{
		SUIElement *pElement = GetElement(nElementIndex);
		if( pElement ) {
			return pElement->fTextureHeight*m_fSizeValue;
		}
	}

	return CDnCountCtl::GetControlHeight();
}

void CDnDamageCount::SetValue( int nValue )
{
	CDnCountCtl::SetValue( nValue );	
}

void CDnDamageCount::SetCritical()
{
	m_DamageType = Damage_Critical;
	m_nValue = -1;
	m_vecValueIndex.clear();

	m_fSizeValue = 0.0f;
	m_fVelocity = 0.0f;
	m_fTotalElapsedTime = 0.0f;
	m_bImpactInc = true;
	m_bImpactDec = false;

	m_fFontSize = 1.2f;
}

void CDnDamageCount::SetResist()
{
	m_DamageType = Damage_Resist;
	m_nValue = -1;
	m_vecValueIndex.clear();

	m_fSizeValue = 0.0f;
	m_fVelocity = 0.0f;
	m_fTotalElapsedTime = 0.0f;
	m_bImpactInc = true;
	m_bImpactDec = false;
}

void CDnDamageCount::SetCriticalRes()
{
	m_DamageType = Damage_CriticalRes;
	m_nValue = -1;
	m_vecValueIndex.clear();

	m_fSizeValue = 0.0f;
	m_fVelocity = 0.0f;
	m_fTotalElapsedTime = 0.0f;
	m_bImpactInc = true;
	m_bImpactDec = false;

	m_fFontSize = 1.2f;
}

#ifdef PRE_ADD_DECREASE_EFFECT
void CDnDamageCount::SetDecreaseEffect()
{
	m_DamageType = Damage_DecreaseEffect;
	m_nValue = -1;
	m_vecValueIndex.clear();

	m_fSizeValue = 0.0f;
	m_fVelocity = 0.0f;
	m_fTotalElapsedTime = 0.0f;
	m_bImpactInc = true;
	m_bImpactDec = false;
}
#endif // PRE_ADD_DECREASE_EFFECT

void CDnDamageCount::SetHasElement(int hasElement)
{
	int nElementIndex = -1;
	switch( hasElement )
	{
	case CDnState::Fire:	nElementIndex = 27;	break;
	case CDnState::Ice:		nElementIndex = 26;	break;
	case CDnState::Light:	nElementIndex = 25;	break;
	case CDnState::Dark:	nElementIndex = 24;	break;
	}

	if( nElementIndex != -1 )
	{
		SUIElement *pElement = GetElement(nElementIndex);
		if( pElement ) 
			m_vecValueIndex.push_back(nElementIndex);
	}
}
