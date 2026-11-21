#pragma once
#include "DnWorldBrokenProp.h"
#include "MAActorProp.h"
#include "DnBuffPropComponent.h"


// Breakable buff prop. Basic properties follow BrokenProp.
class CDnWorldBrokenBuffProp : public CDnWorldBrokenProp,
							   public MAActorProp,
							   public TBoostMemoryPool< CDnWorldBrokenBuffProp >


{
private:
	// Run an additional FSM for buff handling. The original FSM is handled on the broken prop side.
	TDnFiniteStateMachine<DnPropHandle>* m_pBuffFSM;
	CDnBuffPropComponent m_BuffPropComponent;

protected:
	void ReleasePostCustomParam( void )
	{ 
		if( m_pData && m_bPostCreateCustomParam )
		{
			BuffBrokenStruct* pBuffBrokenStruct = static_cast<BuffBrokenStruct*>( m_pData );
			SAFE_DELETE( pBuffBrokenStruct ); 
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};

public:
	CDnWorldBrokenBuffProp( CMultiRoom* pGameRoom );
	virtual ~CDnWorldBrokenBuffProp(void);

	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual bool InitializeTable( int nTableID );

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	void OnMessage( const boost::shared_ptr<IStateMessage>& pMessage );
	void OnChangeAction( const char *szPrevAction );

	// This function is called when summoned by a summoner.
	void SetSummoner( DnActorHandle hActor ) 
	{ 
		m_BuffPropComponent.GetSkillComponent().SetSkillUser( hActor ); 
		m_BuffPropComponent.SetTeam( hActor->GetTeam() );
	};
};