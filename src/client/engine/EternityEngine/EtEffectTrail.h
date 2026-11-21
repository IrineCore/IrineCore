#pragma once
#include "etbaseeffect.h"

class CEtEffectTrail;
typedef CSmartPtr< CEtEffectTrail >	EtEffectTrailHandle;

// Rotha Configured upon request to use LineTrailEffect in finer detail
// Can link to Bone, set blend options, etc., and
// Will configure handling per min / max distance, etc.
// Set base structure to follow LineTrailEffect.

#define MAX_EFFECT_TRAIL_VERTEX	300
class CEtEffectTrail : public CEtBaseEffect
{
public:
	CEtEffectTrail(void);
	virtual ~CEtEffectTrail(void);

protected:
	float m_fLineWidth;
	float m_fLifeTime;
	float m_fInverseLifeTime;
	float m_fMinSegment;
	EtColor m_LineColor;

	bool m_bModify;
	float m_fCurrentStartTime;
	std::vector< EtVector3 > m_vecPosition;
	std::vector< EtVector3 > m_vecTangent;
	std::vector< EtVector2 > m_vecTime;

	bool	m_bShow;

	static EtMaterialHandle s_hMaterial;
	static EtIndexBuffer *s_pCommonIndexBuffer;

	float	m_fElapsedTime;

public:
	void Initialize( EtTextureHandle hTexture, float fLifeTime, float fLineWidth, float fMinSegment = 50.0f );
	void SetLineColor( EtColor &LineColor ) { m_LineColor = LineColor; }
	void CreateVertexBuffer();
	void CopyVertexBuffer();
	void AddPoint( EtVector3 &Point );
	void Show( bool bShow ) {m_bShow=bShow;}
	bool IsShow() {return m_bShow;}
	virtual void Render( float fElapsedTime );

	static void InitializeEffect();
	static void FinializeEffect();
};
