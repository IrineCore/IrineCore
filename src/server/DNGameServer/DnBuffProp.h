#pragma once

#include "DnWorldActProp.h"
#include "MAActorProp.h"
#include "DnBuffPropComponent.h"


//A. Feature that affects surrounding area
//B. Grant a specific Buff to a specific Area.
//C. Effect target
//i. Player and allies (monsters)
//ii. Monster
//iii. Both
//D. Effect presentation
//i. Create connected Prop (+prop's effect)
//ii. Display on character the same way status abnormal effects are displayed.
//1. UI
//2. Effect attached to body
//2. Decide development direction
//A. This feature is developed as an Action of a specific Prop
//3. Developed as a method to apply effects to a specific event area
class CDnBuffProp : public CDnWorldActProp,
					public MAActorProp,	
					public CDnDamageBase,
					public TBoostMemoryPool< CDnBuffProp >
{
private:
	CDnBuffPropComponent m_BuffPropComponent;


protected:
	virtual bool InitializeTable( int nTableID );
	void ReleasePostCustomParam( void ) 
	{ 
		if( m_pData && m_bPostCreateCustomParam )
		{
			BuffStruct* pBuffStruct = static_cast<BuffStruct*>( m_pData );
			SAFE_DELETE( pBuffStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};

public:
	CDnBuffProp( CMultiRoom* pRoom );
	virtual ~CDnBuffProp(void);

	// from CDnWorldActProp
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual void OnChangeAction( const char *szPrevAction );

	// from CDnDamageBase
	virtual DnActorHandle GetActorHandle( void ) { return m_hMonster; };
	void OnMessage( const boost::shared_ptr<IStateMessage>& pMessage );

	void SetSummoner( DnActorHandle hActor ) 
	{
		m_BuffPropComponent.GetSkillComponent().SetSkillUser( hActor ); 
		m_BuffPropComponent.SetTeam( hActor->GetTeam() );
	};
};