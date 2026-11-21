#pragma once

#include "DnRenderBase.h"
#include "MAFaceAniBase.h"

enum ALPHA_LAYER  
{
	AL_NORMAL = 0,
	AL_SIGNAL,
	AL_STATEEFFECT,
	AL_APPEAR,
	AL_MAX,
};

class MAActorRenderBase : public CDnRenderBase , public MAFaceAniBase {
public:
	MAActorRenderBase();
	virtual ~MAActorRenderBase();

	
protected:
	EtAniObjectHandle m_hObject;
	std::string m_szAniFileName;
	LOCAL_TIME m_LocalTime;
	LOCAL_TIME m_AniTime;
	EtVector3 m_vAniDistance;
	EtVector3 m_vAddAniDistance;
	EtVector3 m_vScale;
	int m_nAniIndex;
	float m_fFrame;
	float m_fPrevFrame;

	int m_nBlendAniIndex;
	float m_fBlendAniFrame;
	float m_fBlendFrame;
	float m_fBlendStartFrame;
	float m_fBlendCurFrame;

	int m_nMixedBlendAniIndex;
	float m_fMixedBlendAniFrame;
	int m_nMixedBlendBoneIndex;
	float m_fMixedBlendFrame;
	LOCAL_TIME m_MixedBlendAniTime;
	float m_fMixedFrame;

	struct MixedAniStruct {
		int nAniIndex;
		int nBoneIndex;
		int nMaintenanceBoneIndex;
		float fFrame;
		float fLastFrame;
		float fBlendFrame;
		float fEndBlendFrame;
		int nBlendAniIndex;
		float fBlendAniFrame;
		float fBlendStartFrame;
		int nLoopCount;
		LOCAL_TIME AniTime;
	};
	std::vector<MixedAniStruct> m_VecMixedAni;

	MSDT_DECL(float) m_fFps;
	float	m_fAlphaLayers[ AL_MAX ];

public:
	virtual EtVector3 *GetAniDistance();

	// In MixedAnimation, fBlendFrame specifies the blending frame at start and the frame when returning to default animation after Mixed ends!
	void AddMixedAnimation( const char *szActionBoneName, const char *szMaintenanceBoneName, int nAniIndex, float fFrame, float fBlendFrame = 3.f, float fEndBlendFrame = 3.f, int nLoopCount = 0 );
	void AddMixedAnimation( int nBoneIndex, int nMaintenanceBoneIndex, int nAniIndex, float fFrame, float fBlendFrame = 3.f, float fEndBlendFrame = 3.f, int nLoopCount = 0 );
	const char *GetBoneName( int nBoneIndex );

	EtAniObjectHandle GetObjectHandle() { return m_hObject; }

	bool GetBoundingSphere( SSphere &Sphere );
	bool GetBoundingBox( SAABox &Box );

	float GetAlphaLayer( ALPHA_LAYER AL );
	void SetAlphaBlend( float fAlpha, ALPHA_LAYER Layer = AL_NORMAL );
	float GetAlpha();

	bool IsEnableCastShadow();
	void EnableCastShadow( bool bEnable );

	void SetScale( EtVector3 &vScale ) { m_vScale = vScale; }
	EtVector3 *GetScale() { return &m_vScale; }

	void ResetActorRenderBase();
	void ResetMixedAnimation( bool bBlending = true );
	bool IsExistMixedAnimation( int nBoneIndex, int nAniIndex );

	// DnRenderBase 
	virtual void ChangeAnimation( int nAniIndex, float fFrame = 0.f, float fBlendFrame = 0.f );
	virtual void PreProcess( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( EtMatrix *pmatWorld, LOCAL_TIME LocalTime, float fDelta );
	virtual int GetAniIndex( const char *szAniName );
	virtual EtMatrix GetBoneMatrix( const char *szBoneName, bool *bExistBone = NULL );
	virtual int GetBoneIndex( const char *szBoneName );
	virtual int GetDummyBoneIndex( const char *szBoneName );
	virtual int GetSubMeshIndex( const char *szSubMeshName );
	virtual void SetFPS( float fValue );
	virtual float GetFPS();
	virtual void ShowRenderBase( bool bShow );
	virtual void SetFrame( float fValue );
	virtual void SetPrevFrame( float fValue );
	virtual void AddAniDistance( EtVector3 &vVec );
	virtual void CalcAniDistance( int nAniIndex, float fFrame, float fPrevFrame, EtVector3 &vDist );
};