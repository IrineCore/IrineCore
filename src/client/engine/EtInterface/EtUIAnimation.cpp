#include "StdAfx.h"
#include "EtUIAnimation.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtUIAnimation::CEtUIAnimation( CEtUIDialog *pParent )
	: CEtUIControl( pParent )
	, m_fDelayTime(0.0f)
	, m_nFrameCount(0)
	, m_nCurrentFrame(0)
	, m_bPlay(false)
#ifdef PRE_MOD_ANIMATIONUI_PLAY_CONTROL
	, m_PlayTimeType(UI_ANIMATION_PLAYTIME_PERFRAME)
	, m_fPerActionTime(0.f)
#endif
{
	m_Property.UIType = UI_CONTROL_ANIMATION;
}

CEtUIAnimation::~CEtUIAnimation(void)
{
}

void CEtUIAnimation::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	m_nFrameCount = GetElementCount();
#ifdef PRE_MOD_ANIMATIONUI_PLAY_CONTROL
	m_PlayTimeType		= m_Property.AnimationProperty.PlayTimeType;
	m_fPerActionTime	= (float(m_Property.AnimationProperty.timePerAction) / 1000.f) / float((m_nFrameCount > 0) ? m_nFrameCount : 1);
#endif
}

void CEtUIAnimation::Render( float fElapsedTime )
{
	if( !IsPlaying() )
		return;

	SUIElement *pElement(NULL);
	pElement = GetElement(m_nCurrentFrame);

#ifdef PRE_MOD_ANIMATIONUI_PLAY_CONTROL
	m_fDelayTime += fElapsedTime;

	float frameTime = pElement->fDelayTime;
	if (m_PlayTimeType == UI_ANIMATION_PLAYTIME_PERACTION)
		frameTime = m_fPerActionTime;

	if( frameTime < m_fDelayTime )
	{
		m_nCurrentFrame++;
		m_fDelayTime = 0.0f;
	}

	if( m_nCurrentFrame >= m_nFrameCount )
	{
		if( m_Property.AnimationProperty.bLoop )
		{
			Play();
		}
		else
		{
			Stop();
		}
	}
#else
	m_fDelayTime += fElapsedTime;

	if( pElement->fDelayTime < m_fDelayTime )
	{
		m_nCurrentFrame++;
		m_fDelayTime = 0.0f;
	}

	if( m_nCurrentFrame >= m_nFrameCount )
	{
		if( m_Property.AnimationProperty.bLoop )
		{
			Play();
		}
		else
		{
			Stop();
		}
	}
#endif

	// To make it fade out gradually, handle it like ItemAlarmDlg.
	if( m_bExistTemplateTexture )
		m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord );
	else
		m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord );
}

void CEtUIAnimation::Play()
{
	Stop();
	m_bPlay = true;
}

void CEtUIAnimation::Stop()
{
	m_fDelayTime = 0.0f;
	m_nCurrentFrame = 0;
	m_bPlay = false;
}

bool CEtUIAnimation::IsPlaying()
{
	return m_bPlay;
}

void CEtUIAnimation::SetLoop( bool bLoop )
{
	m_Property.AnimationProperty.bLoop = bLoop;
}