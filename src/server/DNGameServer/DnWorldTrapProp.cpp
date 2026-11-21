#include "stdafx.h"
#include "DnWorldTrapProp.h"
#include "DnPartyTask.h"
#include "DNUserSession.h"
//#include "DnActorClassDefine.h"
#include "DnMonsterActor.h"

CDnWorldTrapProp::CDnWorldTrapProp( CMultiRoom *pRoom )
: CDnWorldActProp( pRoom )
, CDnDamageBase( DamageObjectTypeEnum::Prop )
, m_iActivateType( LOOP )
, m_nIdleElementIndex( -1 )
, m_nActivateElementIndex( -1 )
, m_nNowActionElementIndex( -1 )
, m_bIdle( false )
{

}

CDnWorldTrapProp::~CDnWorldTrapProp()
{
	
}

bool CDnWorldTrapProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	if( CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale ) == false ) return false;
	if( m_hMonster ) *m_hMonster->GetMatEx() = *GetMatEx();
	return true;
}

bool CDnWorldTrapProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	if( GetData() )
	{
		TrapStruct *pStruct = (TrapStruct *)GetData();
		int			nMonsterTableID = pStruct->nMonsterTableID;
		bool		bResult			= false;

//#61146 Structure allowing Trap-type prop attack power to be set differently by difficulty.
#ifdef PER_ADD_PROP_TRAP_DIFFICULT_SET
		bResult = InitializeMonsterActorProp( nMonsterTableID, pStruct->MonsterWeight );
#else
		bResult = InitializeMonsterActorProp( nMonsterTableID );
#endif	

		if( bResult )
		{
//			m_hMonster->SetUniqueID( STATIC_INSTANCE(CDnActor::s_dwUniqueCount)++ );
			
			m_HitComponent.Initialize( m_hMonster, &m_Cross, GetMySmartPtr(), this );

			m_iActivateType = pStruct->ActivateType;
		}
	}

	return true;
}

bool CDnWorldTrapProp::CreateObject()
{
	bool bResult = CDnWorldActProp::CreateObject();

	if( bResult )
	{
		m_nIdleElementIndex = GetElementIndex( "Idle" );
		m_nActivateElementIndex = GetElementIndex( "Activate" );

		//if( LOOP == m_iActivateType )
		//	GetElement( m_nActivateElementIndex )->szNextActionName.assign( "Activate" );
		//else
		//if( BOUNDING_BOX_CHECK == m_iActivateType )
		//	GetElement( m_nActivateElementIndex )->szNextActionName.assign( "Idle" );
	}

	return bResult;
}

void CDnWorldTrapProp::SetActionQueue( const char *szActionName, int nLoopCount , float fBlendFrame , float fStartFrame )
{
	//if( LOOP == m_iActivateType && strcmp( szActionName, "Idle" ) == 0 )
	//{
	//	//SetActionQueue( "Activate" );
	//	szActionName = "Activate";
	//}

	CDnWorldActProp::SetActionQueue( szActionName, nLoopCount, fBlendFrame, fStartFrame );
	//if( strcmp( szActionName, "Idle" ) == 0 )
	//	m_bIdle = true;
	//else
	//	m_bIdle = false;
}

// Default is idle in the action tool, so change it here for loop types.
void CDnWorldTrapProp::OnFinishAction( const char* szPrevAction, LOCAL_TIME time )
{
	//if( LOOP == m_iActivateType && strcmp( szPrevAction, "Idle" )/*false == m_bIdle*/ )
	//	SetActionQueue( "Activate" );
}

void CDnWorldTrapProp::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnWorldActProp::Process( LocalTime, fDelta );
	if( !m_Handle ) return;
	if( m_nIdleElementIndex == -1 ) return;
	//OutputDebug( "TrapFrame : %.2f\n", CDnActionBase::m_fFrame );

	m_nNowActionElementIndex = GetElementIndex( GetCurrentAction() );

	switch( m_iActivateType )
	{
		case BOUNDING_BOX_CHECK:
			{
				if( m_nIdleElementIndex == m_nNowActionElementIndex )
				{
					SOBB Box;
					GetBoundingBox( Box );

					for( DWORD i=0; i<CDnPartyTask::GetInstance(GetRoom()).GetUserCount(); i++ ) 
					{
						CDNUserSession *pSession = CDnPartyTask::GetInstance(GetRoom()).GetUserData(i);
						DnActorHandle hActor = pSession->GetActorHandle();
						if( !hActor ) continue;
						if( hActor->IsDie() ) continue;

						if( Box.IsInside( *hActor->GetPosition() ) == true ) {
							CmdAction( "Activate" );
							break;
						}
					}
				}
			}
			break;

		//case LOOP:
		//	{
		//		if( m_nActivateElementIndex == m_nNowActionElementIndex )
		//		{
		//			CEtActionBase::ActionElementStruct* pElement = GetElementIndex( m_nActivateElementIndex );
		//			if( (pElement->dwLength / s_fDefaultFps) - m_fFrame - fDelta < 0.0f )
		//				
		//		}
		//	}
		//	break;
	}
}

void CDnWorldTrapProp::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, 
								 LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	m_HitComponent.OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );

	CDnWorldActProp::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
}


// For trap props, projectiles must match the client presentation accurately, so the server sends a packet on every action change.
void CDnWorldTrapProp::OnChangeAction( const char *szPrevAction )
{
	//// For loop-type traps, send only the Activate action to the client.
	//if( LOOP == m_iActivateType )
	//{
	//	if( m_szAction == "Activate" )
	//		ActionSync( "Activate" );
	//}
	//else
	//{
	//	// Since idle repeats anyway unless it changes to another action 
	//	// For idle action, send only once when changing to idle. 
	//	// Fixed issue where packets keep sending when Idle action is short. (#17409)
	//	const char* pCurrentAction = GetCurrentAction();
	//	if( stricmp(pCurrentAction, "Idle") == 0 || stricmp(pCurrentAction, "off") )
	//	{
	//		if( stricmp(szPrevAction, "Idle") != 0 )
	//		{
	//			ActionSync( pCurrentAction );
	//		}
	//	}
	//	else
	//		ActionSync( pCurrentAction );
	//}
	
	// Send packet only when the action changes.
	if( m_nActionIndex != m_nPrevActionIndex )
		ActionSync( GetCurrentAction() );
}