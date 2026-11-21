#pragma once
#include "EtUIControl.h"

//#define PRE_MOD_ANIMATIONUI_PLAY_CONTROL

class CEtUIAnimation : public CEtUIControl
{
public:
	CEtUIAnimation( CEtUIDialog *pParent );
	virtual ~CEtUIAnimation(void);

protected:
	int m_nFrameCount;
	int m_nCurrentFrame;
	float m_fDelayTime;
	bool m_bPlay;
#ifdef PRE_MOD_ANIMATIONUI_PLAY_CONTROL
	UIAnimationPlayTimeType m_PlayTimeType;
	float					m_fPerActionTime;
#endif

	// Whether to alpha-fade out when animation disappears. On hold.
	//bool m_bAlphaFadeOut;

public:
	void SetLoop( bool bLoop );

	void Play();
	void Stop();

	bool IsPlaying();

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Render( float fElapsedTime );
};
