#pragma once

#include "DnWorld.h"
#include "EtWorldSector.h"
#include "EtQuadtree.h"

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
#include "FrameSync.h"
#endif

class CEtWorldProp;
class CDnWorldProp;

class CDnWorldSector : public CEtWorldSector, public TBoostMemoryPool< CDnWorldSector >
{
public:
	CDnWorldSector();
	virtual ~CDnWorldSector();

protected:
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	std::vector<CDnWorldProp *>			m_pVecProcessProp[2];
#else
	std::vector<CDnWorldProp *>			m_pVecProcessProp;
#endif
	std::map<DWORD,std::vector<CEtWorldEventArea*>>	m_mPrevEventArea;
	std::map<DWORD,std::vector<CEtWorldEventArea*>>	m_mCurEventArea;


#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	static int s_nProcessAllowCount[2];

	int m_nProcessAllowed[2];
	float *m_fProcessAllowedDelta[2];
#else
	int m_nProcessAllowed;
#endif

protected:
	virtual void InitializeTerrain( DWORD *pAlphaTable, std::vector<int> &nVecBlockType, std::vector< std::vector<std::string> > &szVecLayerTexture, std::vector< std::vector<float> > &fVecLayerTextureDistance, std::vector< std::vector<float> > &fVecLayerTextureRotation, char *szGrassTexture, char *pGrassTable, float *fGrassWidth, float *fGrassHeightMin, float *fGrassHeightMax, float fShakeMin, float fShakeMax );

	virtual int GetPropClassID( const char *szFileName );
	
	void ProcessEventAreaTriggerEventCheck( LOCAL_TIME LocalTime, float fDelta );

public:
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void Free();
	virtual CEtWorldProp *AllocProp( int nClass );
	virtual CEtWorldSound *AllocSound();
	virtual CEtWorldWater *AllocWater();
	virtual CEtWorldDecal *AllocDecal();
	virtual bool LoadSound( const char *szSectorPath );
	virtual bool LoadProp( const char *szSectorPath, int nBlockIndex = -1, bool bThreadLoad = true );
	virtual bool LoadWater( const char *szSectorPath );
	virtual bool LoadDecal( const char *szSectorPath );

	void InsertProcessProp( CDnWorldProp *pProp );
	void RemoveProcessProp( CDnWorldProp *pProp );

	int GetPropTableID( const char *szFileName );

	virtual CEtTrigger *AllocTrigger();

	// There are props whose position changes. Call Update directly on those props.
	CEtOctree<CEtWorldProp*>* GetOctree( void ) { return m_pPropOctree; };
};