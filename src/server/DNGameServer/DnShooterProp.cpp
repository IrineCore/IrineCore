#include "stdafx.h"
#include "DnShooterProp.h"
#include "DnStateBlow.h"
#include "DnTableDB.h"
#include "DnProjectile.h"


CDnShooterProp::CDnShooterProp( CMultiRoom* pRoom ) : CDnWorldActProp( pRoom ),
													  CDnDamageBase( DamageObjectTypeEnum::Prop )
{

}

CDnShooterProp::~CDnShooterProp(void)
{

}

bool CDnShooterProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	if( CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale ) == false ) return false;
	if( m_hMonster ) *m_hMonster->GetMatEx() = *GetMatEx();
	return true;
}

bool CDnShooterProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable(nTableID) == false ) return false;

	bool bResult = false;

	if( GetData() )
	{
		ProjectileShooterStruct* pStruct = (ProjectileShooterStruct*)GetData();
		int  nMonsterTableID = pStruct->nMonsterTableID;
		int  nSkillTableID	 = pStruct->nSkillTableID;
		int  nSkillLevel	 = pStruct->nSkillLevel;

//#61146 Structure allowing Trap-type prop attack power to be set differently by difficulty.
#ifdef PER_ADD_PROP_TRAP_DIFFICULT_SET
		bResult = InitializeMonsterActorProp( nMonsterTableID, pStruct->MonsterWeight );
#else
		bResult = InitializeMonsterActorProp( nMonsterTableID );
#endif	

		if( bResult )
		{
			//m_hMonster->SetUniqueID( STATIC_INSTANCE(CDnActor::s_dwUniqueCount)++ );
			
			m_SkillComponent.Initialize( nSkillTableID, nSkillLevel );

			// Do not take action yet. Shooter prop needs its start time synced with the client..
			SetAction( "Idle", 0.0f, 0.0f );
		}
	}

	return bResult;
}


// Let's test a skill-using prop as a Shooter prop for now..

void CDnShooterProp::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	if( m_hMonster ) 
	{
		*(m_hMonster->GetMatEx()) = m_Cross;
		m_SkillComponent.OnSignal( m_hMonster, GetMySmartPtr(), Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
	}
}


void CDnShooterProp::OnSyncComplete( CDNUserSession* pBreakIntoGameSession/*=NULL*/ )
{
	if( pBreakIntoGameSession )
		CmdAction( GetCurrentAction(), pBreakIntoGameSession );
	else
		CmdAction( m_szDefaultActionName.c_str(), pBreakIntoGameSession );
}


// For shooter props, the server sends a packet on every action change so projectiles match client visuals accurately.
void CDnShooterProp::OnChangeAction( const char *szPrevAction )
{
	//// Because idle repeats anyway unless it changes to another action 
	//// For idle action, send only once when changed to idle. 
	//// Fixed issue where packets kept firing when Idle action is short. (#17409)
	//const char* pCurrentAction = GetCurrentAction();
	//if( strcmp(pCurrentAction, "Idle") == 0 )
	//{
	//	if( strcmp(szPrevAction, "Idle") != 0 )
	//	{
	//		ActionSync( pCurrentAction );
	//	}
	//}
	//else
	//	ActionSync( pCurrentAction );

	// Send packet only when the action changes.
	if( m_nActionIndex != m_nPrevActionIndex )
		ActionSync( GetCurrentAction() );
}