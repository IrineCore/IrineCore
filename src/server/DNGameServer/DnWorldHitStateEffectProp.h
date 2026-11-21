#pragma once
#include "DnWorldActProp.h"
#include "MAActorProp.h"
#include "DnDamageBase.h"
#include "DnSkillPropComponent.h"
#include "DnHitPropComponent.h"

// #60784 - PTE_HitStateEffect
// Apply state effects to targets hit by the prop.
class CDnWorldHitStateEffectProp : public CDnWorldActProp,
								public MAActorProp,
								public CDnDamageBase							
{

private:

	// FSM.
	//TDnFiniteStateMachine< DnPropHandle > * m_pFSM;
		
	CDnHitPropComponent m_HitComponent;
	CDnSkillPropComponent m_SkillComponent;

	DnSkillHandle m_hSkill;


public:

	CDnWorldHitStateEffectProp( CMultiRoom * pRoom );
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

	// from CDnActionBase
	void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );	
	//virtual void OnFinishAction( const char* szPrevAction, LOCAL_TIME time );
};

class IBoostPoolDnWorldHitStateEffectProp : public CDnWorldHitStateEffectProp, public TBoostMemoryPool< IBoostPoolDnWorldHitStateEffectProp >
{
public:
	IBoostPoolDnWorldHitStateEffectProp( CMultiRoom *pRoom ):CDnWorldHitStateEffectProp(pRoom){}
	virtual ~IBoostPoolDnWorldHitStateEffectProp(){}
};
