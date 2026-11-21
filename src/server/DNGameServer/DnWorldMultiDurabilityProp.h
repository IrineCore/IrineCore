#pragma once
#include "DnWorldBrokenProp.h"
#include "DnSkillPropComponent.h"
#include "MAActorProp.h"

class CDnPropState;

// #24282 Broken prop with multiple durability values
class CDnWorldMultiDurabilityProp : public CDnWorldBrokenProp, 
									public MAActorProp,
									public CDnDamageBase,
									public TBoostMemoryPool<CDnWorldMultiDurabilityProp>
{
private:
	CDnSkillPropComponent m_SkillComponent;
	//string m_strLastActionName;

	struct S_STATE_INFO
	{
		const char* pActionNameWhenSubDurabilityZero;
		int iOriginalDurability;
		int iNowDurability;
		CDnPropState* pState;

		S_STATE_INFO( void ) : pActionNameWhenSubDurabilityZero( NULL ), 
							   iOriginalDurability(0 ),
							   iNowDurability( 0 ),
							   pState( NULL )
		{};

		void RestoreDurability( void )
		{
			iNowDurability = iOriginalDurability;
		}
	};
	DNVector( S_STATE_INFO ) m_vlStateInfos;

	int m_iNowStateIndex;
	CDnPropState* m_pActivateState;			// Enters this State when the Activate action is executed via trigger.

protected:
	void _InitializeFSM( void );

public:
	CDnWorldMultiDurabilityProp( CMultiRoom* pRoom );
	virtual ~CDnWorldMultiDurabilityProp( void );

	bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	bool InitializeTable( int nTableID );
	void ReleasePostCustomParam( void ) 
	{ 
		if( m_pData && m_bPostCreateCustomParam )
		{
			MultiDurabilityBrokenPropStruct* pMultiDurabilityStruct = static_cast<MultiDurabilityBrokenPropStruct*>( m_pData );
			SAFE_DELETE( pMultiDurabilityStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};

	virtual void SetActionQueue( const char *szActionName, int nLoopCount /* = 0 */, float fBlendFrame /* = 3.f */, float fStartFrame /* = 0.f */ );

	void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	void OnSyncComplete( CDNUserSession* pBreakIntoGameSession/*=NULL*/ );
	void OnChangeAction( const char *szPrevAction );
	
	// from CDnDamageBase
	virtual DnActorHandle GetActorHandle( void ) { return m_hMonster; };
	virtual DWORD GetDamageObjectUniqueID() { return GetUniqueID(); };

	// from CDnWorldBrokenProp
	virtual void CalcDamage( CDnDamageBase *pHitter, CDnDamageBase::SHitParam &HitParam );

	virtual int GetSubDurability( int iIndex );

	virtual void OnFSMStateEntry( const TDnFSMState<DnPropHandle>* pState );
};
