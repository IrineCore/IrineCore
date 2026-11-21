#include "StdAfx.h"
#include "DnShooterProp.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif



CDnShooterProp::CDnShooterProp(void) : CDnDamageBase( DamageObjectTypeEnum::Prop )
{
}

CDnShooterProp::~CDnShooterProp(void)
{
}


bool CDnShooterProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	bool bResult = false;
	if( GetData() )
	{
		ProjectileShooterStruct* pStruct = (ProjectileShooterStruct*)GetData();
		int						 nMonsterTableID = pStruct->nMonsterTableID;

//#61146 Structure allowing Trap-type prop attack power to be set differently by difficulty.
#ifdef PER_ADD_PROP_TRAP_DIFFICULT_SET
		bResult = InitializeMonsterActorProp( nMonsterTableID, pStruct->MonsterWeight );
#else
		bResult = InitializeMonsterActorProp( nMonsterTableID );
#endif

		// NOTE: The client only needs to display the graphics. 
		// When hit by this and receiving the packet to add the state effect to the actor, it looks up the skill via FindSkill on this monster object's MASkillUser, so 
		// Add the skill.
		if( bResult )
		{
			//m_hMonster->AddSkill( pStruct->nSkillTableID, pStruct->nSkillLevel );

			// Do not take action yet. Shooter prop needs its start time synced with the client..
			SetAction( "Idle", 0.0f, 0.0f );
		}
	}

	return bResult;
}

bool CDnShooterProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	bool bResult = CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
	if( bResult && m_hMonster ) {
		*m_hMonster->GetMatEx() = *GetMatEx();
		m_hMonster->SetActionSignalScale( *GetActionSignalScale() );
	}
	return bResult;
}


void CDnShooterProp::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	if( m_hMonster )
	{
		*(m_hMonster->GetMatEx()) = m_matExWorld;
		m_hMonster->OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
	}
}