#pragma once
#include "DnWorldActProp.h"
#include "MAActorProp.h"
#include "DnDamageBase.h"


// #60784 - PTE_HitStateEffect
// Apply state effects to targets hit by the prop.
class CDnWorldHitStateEffectProp : public CDnWorldActProp,
								   public MAActorProp,
								   public CDnDamageBase							
{

public:

	CDnWorldHitStateEffectProp();
	virtual ~CDnWorldHitStateEffectProp();


	// CDnWorldActProp //
	virtual bool InitializeTable( int nTableID );
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 & vRotate, EtVector3 & vScale );
	//virtual void OnChangeAction( const char *szPrevAction );
	//virtual void _OnLifeTimeEnd( LOCAL_TIME LocalTime, float fDelta );

	void ReleasePostCustomParam(){
		if( m_pData && m_bPostCreateCustomParam ){
			HitStateEffectStruct * pStruct = static_cast< HitStateEffectStruct * >( m_pData );
			SAFE_DELETE( pStruct );
			m_pData = NULL;
		}
	}

	// from CDnDamageBase
	virtual DnActorHandle GetActorHandle( void ) { return m_hMonster; };
};
