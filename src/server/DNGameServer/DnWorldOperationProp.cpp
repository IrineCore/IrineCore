#include "stdafx.h"
#include "DnWorldOperationProp.h"
#include "DNProtocol.h"
#include "DnWorld.h"
#include "DnPropState.h"
#include "DnPropStateTrigger.h"
#include "DnPropOperatedCondition.h"
#include "DNUserSession.h"


CDnWorldOperationProp::CDnWorldOperationProp( CMultiRoom *pRoom )
: CDnWorldActProp( pRoom ), m_bIsOperated( false )
{
	m_pTriggerActionState = NULL;
}

CDnWorldOperationProp::~CDnWorldOperationProp()
{
}


bool CDnWorldOperationProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	// Create state
	CDnPropState* pNormalState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::NORMAL );
	CDnPropState* pTriggerState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::TRIGGER );
	CDnPropStateTrigger* pPropStateTrigger = static_cast<CDnPropStateTrigger*>(pTriggerState);
	pPropStateTrigger->SetFuncName( "CDnWorldProp::OnClickProp" );
	m_pTriggerActionState = pPropStateTrigger;

	// Create condition
	CDnPropCondition* pOpCondition = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::OPERATED );
	static_cast<CDnPropOperatedCondition*>(pOpCondition)->SetRequireState( ON );		// Is 1...
	CDnPropCondition* pNULLCondition = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::NULL_CONDITION );

	m_pFSM->AddCondition( pOpCondition );
	m_pFSM->AddCondition( pNULLCondition );

	m_pFSM->AddState( pNormalState );
	m_pFSM->AddState( pTriggerState );

	pNormalState->AddTransitState( pTriggerState, pOpCondition );
	pTriggerState->AddTransitState( pNormalState, pNULLCondition );

	m_pFSM->SetEntryState( pNormalState );

	return CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
}

void CDnWorldOperationProp::OnDispatchMessage( CDNUserSession *pSession, DWORD dwProtocol, BYTE *pPacket )
{
	switch( dwProtocol ) {
		case eProp::CS_CMDOPERATION:
			{
				if( false == IsShow() )
					return;

#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
				if (IsVisible() == false)
					return;
#else
	#ifdef PRE_FIX_PROP_RANDOM_VISIBLE
				if( false == m_bRandomResultIsVisible )
					return;
	#endif // #ifdef PRE_FIX_PROP_RANDOM_VISIBLE
#endif

				CmdOperarionProp(pSession);
			}
			break;
	};
	CDnWorldActProp::OnDispatchMessage( pSession, dwProtocol, pPacket );
}

void CDnWorldOperationProp::OnSyncComplete( CDNUserSession* pBreakIntoGameSession )
{
	if( !pBreakIntoGameSession || !pBreakIntoGameSession->GetActorHandle() )
		return;

	DiableActorManage( pBreakIntoGameSession->GetActorHandle(), m_bEnableOperator );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &m_bEnableOperator, sizeof(bool) );

	Send( eProp::SC_CMDENABLEOPERATOR, pBreakIntoGameSession->GetActorHandle(), &Stream );

	CDnWorldActProp::OnSyncComplete( pBreakIntoGameSession );
}

void CDnWorldOperationProp::CmdEnableOperator( DnActorHandle hActor, bool bEnable )
{
	m_bEnableOperator = bEnable;

	DiableActorManage( hActor, bEnable );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &bEnable, sizeof(bool) );

	Send( eProp::SC_CMDENABLEOPERATOR, hActor, &Stream );
}

void CDnWorldOperationProp::CmdOperarionProp(CDNUserSession * pSession)
{
#if !defined( PRE_TRIGGER_TEST )
	if( std::find( m_hVecDisableActor.begin(), m_hVecDisableActor.end(), pSession->GetActorHandle() ) != m_hVecDisableActor.end() )
		return;
#endif // #if defined( PRE_TRIGGER_TEST )

	m_bIsOperated = true;
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	//If the user has never clicked before, register as a clicked user
	if( std::find( m_hVecClickActor.begin(), m_hVecClickActor.end(), pSession->GetActorHandle() ) == m_hVecClickActor.end() )
		m_hVecClickActor.push_back(pSession->GetActorHandle());
#endif
	// Transition immediately. Must process as soon as message is dispatched to handle simultaneous or queued packets properly.
	//				SetLastAccessActor( pSession->GetActorHandle() );
	m_pTriggerActionState->AddFuncParam( "LastOperationActor", ( pSession->GetActorHandle() ) ? pSession->GetActorHandle()->GetUniqueID() : -1 );
	m_pFSM->Process( 0, 0.0f );

	//m_pFSM->ChangeState( m_pTriggerState, 0, 0.0f );
}

void CDnWorldOperationProp::DiableActorManage( DnActorHandle hActor, bool bEnable )
{
	if( !hActor )
		return;

	if( bEnable ) {
		DNVector(DnActorHandle)::iterator it = std::find( m_hVecDisableActor.begin(), m_hVecDisableActor.end(), hActor );
		if( it == m_hVecDisableActor.end() ) return;
		m_hVecDisableActor.erase( it );
	}
	else {
		if( std::find( m_hVecDisableActor.begin(), m_hVecDisableActor.end(), hActor ) != m_hVecDisableActor.end() ) return;
		m_hVecDisableActor.push_back( hActor );
	}
}
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
bool CDnWorldOperationProp::IsClickPropAndCheckUser(DnActorHandle hActor)
{
	//Skip processing if value is null
	if(!hActor)
		return true;
	DNVector(DnActorHandle)::iterator iter = std::find( m_hVecDisableActor.begin(), m_hVecDisableActor.end(), hActor );
	if( iter != m_hVecDisableActor.end() ) 
		return true;
	//User who never clicked requested a reward. Handle as error.
	if( std::find( m_hVecClickActor.begin(), m_hVecClickActor.end(), hActor ) == m_hVecClickActor.end() )
		return true;

	//Check if the user has not received the reward
	m_hVecDisableActor.push_back( hActor );
	return false;
}
#endif		// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)