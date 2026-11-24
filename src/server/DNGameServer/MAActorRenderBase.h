#pragma once

#include "DnRenderBase.h"

class MAActorRenderBase : public CDnRenderBase, virtual public CMultiElement {
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

	float m_fFPS;
public:
	virtual EtVector3 *GetAniDistance();

	// In MixedAnimation, fBlendFrame specifies the blending frame at start and the frame when returning to default animation after Mixed ends!
	void AddMixedAnimation( const char *szActionBoneName, const char *szMaintenanceBoneName, int nAniIndex, float fFrame, float fBlendFrame = 3.f, float fEndBlendFrame = 3.f );
	void AddMixedAnimation( int nBoneIndex, int nMaintenanceBoneIndex, int nAniIndex, float fFrame, float fBlendFrame = 3.f, float fEndBlendFrame = 3.f );
	const char *GetBoneName( int nBoneIndex );

	EtAniObjectHandle GetObjectHandle() { return m_hObject; }

	bool GetBoundingSphere( SSphere &Sphere );
	bool GetBoundingBox( SAABox &Box );
	bool GetOriginalBoundingBox( SAABox &Box );

	void SetScale( const EtVector3 &vScale ) { m_vScale = vScale; }
	EtVector3 *GetScale() { return &m_vScale; }

	virtual void AddAniDistance( EtVector3 &vVec );
	virtual void CalcAniDistance( int nAniIndex, float fFrame, float fPrevFrame, EtVector3 &vDist );

	// DnRenderBase 
	virtual void ChangeAnimation( int nAniIndex, float fFrame = 0.f, float fBlendFrame = 0.f );
	virtual void PreProcess( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( EtMatrix *pmatWorld, LOCAL_TIME LocalTime, float fDelta );
	virtual int GetAniIndex( const char *szAniName );
	virtual EtMatrix GetBoneMatrix( const char *szBoneName );
	virtual int GetBoneIndex( const char *szBoneName );
	
	virtual void SetFPS( float fValue );
	virtual float GetFPS();

	virtual void SetFrame( float fValue );
	virtual void SetPrevFrame( float fValue );


	virtual bool bIsSingleBody(){ return false; }
	virtual bool bIsPartsBody(){ return false; }
	virtual void LoadSkin( const char *szSkinName, const char *szAniName ){};
	void ResetActorRenderBase();
};