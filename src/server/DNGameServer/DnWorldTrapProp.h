#pragma once

#include "DnWorldActProp.h"
#include "DnActor.h"
#include "DnDropItem.h"
#include "DnDamageBase.h"
#include "MAActorProp.h"
#include "DnHitPropComponent.h"


class CDnWorldTrapProp : public CDnWorldActProp, public CDnDamageBase, public MAActorProp, public TBoostMemoryPool< CDnWorldTrapProp >
{
public:
	CDnWorldTrapProp( CMultiRoom *pRoom );
	virtual ~CDnWorldTrapProp();

protected:
	virtual bool InitializeTable( int nTableID );
	virtual bool CreateObject();


protected:
	// Related to trap activation conditions.
	enum
	{
		LOOP,					// Keep executing fixed action: no longer used; control active action directly from trigger (same as TRIGGER_HANDLING).
		BOUNDING_BOX_CHECK,		// Activated by checking bounding box
		TRIGGER_HANDLING,		// Fully controlled by trigger.. acts like a normal action prop, except action props do not process hits.
	};

	int m_nIdleElementIndex;
	int m_nActivateElementIndex;
	int m_nNowActionElementIndex;

	CDnHitPropComponent m_HitComponent;

	int m_iActivateType;

	// If it should become idle (if SetActionQueue("Idle") was called)
	bool m_bIdle;

public:
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	// CDnActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	virtual void SetActionQueue( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f , float fStartFrame = 0.f );
	virtual void OnFinishAction( const char* szPrevAction, LOCAL_TIME time );

	virtual DWORD GetDamageObjectUniqueID() { return GetUniqueID(); }
	//DnActorHandle GetMonsterHandle() { return m_hMonster; }

	// from CDnDamageBase
	virtual DnActorHandle GetActorHandle( void ) { return m_hMonster; };

	virtual void OnChangeAction( const char *szPrevAction );
};