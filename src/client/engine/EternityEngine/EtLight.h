#pragma once

#include "EtDecal.h"

class CEtLight;
typedef CSmartPtr< CEtLight >	EtLightHandle;

class CEtLight : public CSmartPtrBase< CEtLight >
{
public:
	CEtLight();
	virtual ~CEtLight();

protected:
	bool m_bEnable;
	SLightInfo m_LightInfo;
	EtDecalHandle m_hPointLightDecal;
	static EtColor s_GlobalAmbient;
	EtObjectHandle m_hLightCastObject;

	static std::vector< CEtLight * > s_vecDirectionalLight;
	static std::vector< CEtLight * > s_vecPointLight;
	static std::vector< CEtLight * > s_vecSpotLight;

	static std::vector< CEtLight * > s_vecInfluencePointLight;
	static std::vector< CEtLight * > s_vecInfluenceSpotLight;

	static float s_fDirLightAttenuation;

	CEtOctreeNode< CEtLight * > *m_pCurOctreeNode;
	static CEtOctree< CEtLight * > *s_pLightOctree;
public:
	static void InitializeClass();
	static void FinalizeClass();

	void Initialize( SLightInfo *pSLightInfo, EtObjectHandle hCastLightObject = CEtObject::Identity() );
	bool IsEnable() { return m_bEnable; }
	void Enable( bool bEnable ) { m_bEnable = bEnable; }

	SLightInfo *GetLightInfo() { return &m_LightInfo; }
	void SetLightInfo( SLightInfo *pSLightInfo );

	void GetBoundingSphere( SSphere &Sphere, bool bActorSize = false );

	void AddDecal();

	static SLightInfo *GetShadowCastDirLightInfo();

	static void SetGlobalAmbient( EtColor *pAmbient ) { s_GlobalAmbient = *pAmbient; }
	static EtColor *GetGlobalAmbient() { return &s_GlobalAmbient; }

	static int GetDirLightCount() { return ( int )s_vecDirectionalLight.size(); }
	static SLightInfo *GetDirLightInfo( int nIndex ) { return &s_vecDirectionalLight[ nIndex ]->m_LightInfo; }
	static int GetPointLightCount() { return ( int )s_vecPointLight.size(); }
	static SLightInfo *GetPointLightInfo( int nIndex ) { return &s_vecPointLight[ nIndex ]->m_LightInfo; }
	static int GetSpotLightCount() { return ( int )s_vecSpotLight.size(); }
	static SLightInfo *GetSpotLightInfo( int nIndex ) { return &s_vecSpotLight[ nIndex ]->m_LightInfo; }

	static void SetWorldSize( EtVector3 &WorldCenter, float fRadius );
	static void CalcLightInfluence( SSphere &Sphere, DNVector(CEtLight *) &vecInfluenceLight, EtObjectHandle CalcLightObject );
	static void SetInfluenceLight( CEtLight **ppLight );
	static int GetInfluencePointLightCount() { return ( int )s_vecInfluencePointLight.size(); }
	static SLightInfo *GetInfluencePointLightInfo( int nIndex ) { return s_vecInfluencePointLight[ nIndex ]->GetLightInfo(); }
	static int GetInfluenceSpotLightCount() { return ( int )s_vecInfluenceSpotLight.size(); }
	static SLightInfo *GetInfluenceSpotLightInfo( int nIndex ) { return s_vecInfluenceSpotLight[ nIndex ]->GetLightInfo(); }

	static void SetDirLightAttenuation( float fAttenuation ) { s_fDirLightAttenuation = fAttenuation; }
	static float GetDirLightAttenuation() { return s_fDirLightAttenuation; }

	// Provide a force-set function because in special cases (RenderAvatar) settings are needed even when no global light exists.
	static void ForcePushDirLightInfo( CEtLight *pLight ) { s_vecDirectionalLight.push_back( pLight ); }
	static void ForceClearDirLightInfo() { s_vecDirectionalLight.clear(); }
};
