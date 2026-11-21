#include "stdafx.h"
#include "DnBubbleEventHandler.h"
#include "DnObserverEventMessage.h"
#include "DnBubbleSystem.h"
#include "DnPlayerActor.h"
#include "DnBlow.h"
#include "DnTableDB.h"

namespace BubbleSystem
{

IDnBubbleEventHandler* IDnBubbleEventHandler::Create( int iEventHandlerType, DnActorHandle hActor, const char* pArgument )
{
	IDnBubbleEventHandler* pEventHandler = NULL;

	switch( iEventHandlerType )
	{
		case GETTING_BUBBLE:
			pEventHandler = new CDnGettingBubbleHandler;
			break;

		case REMOVE_BUBBLE:
			pEventHandler = new CDnRemoveBubbleHandler;
			break;

		case UPDATE_BUBBLE_STATE_EFFECT:
			pEventHandler = new CDnUpdateStateEffectHandler;
			break;

		case ENCHANT_SKILL_ONCE:
			pEventHandler = new CDnEnchantSkillOnce;
			break;

		default:
			return NULL;
	}

	pEventHandler->Initialize( hActor, pArgument );

	return pEventHandler;
}

// CDnGettingBubbleHandler
void CDnGettingBubbleHandler::Initialize( DnActorHandle hActor, const char* pArgument )
{
	if( !hActor || NULL == pArgument )
		return;

	m_hActor = hActor;

	std::vector<string> vlTokens;
	string strArgument( pArgument );
	TokenizeA( strArgument, vlTokens, ";" );

	_ASSERT( 2 == (int)vlTokens.size() );

	m_iBubbleTypeID = atoi( vlTokens.at(0).c_str() );
	m_fDurationTime = (float)(atoi( vlTokens.at(1).c_str() )) / 1000.0f;

	// Do not stack more bubbles if the maximum bubble count is exceeded.
	m_iAllowedMaxBubbleCount = 0;
	DNTableFileFormat* pBubbleDefineTable = GetDNTable( CDnTableDB::TSKILLBUBBLEDEFINE );
	if( pBubbleDefineTable->IsExistItem( m_iBubbleTypeID ) )
	{
		m_iAllowedMaxBubbleCount = pBubbleDefineTable->GetFieldFromLablePtr( m_iBubbleTypeID, "_BubbleLimit" )->GetInteger();
	}
}

void CDnGettingBubbleHandler::ProcessEvent( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent )
{
	// Do not add if the max bubble count is exceeded.
	int iNowBubbleCount = pBubbleSystem->GetBubbleCountByTypeID( m_iBubbleTypeID );
	if( iNowBubbleCount < m_iAllowedMaxBubbleCount )
	{
		CDnBubbleSystem::S_CREATE_BUBBLE Info;
		Info.iBubbleTypeID = m_iBubbleTypeID;
		Info.iIconIndex = m_iIconIndex;
		Info.fDurationTime = m_fDurationTime;
		pBubbleSystem->AddBubble( Info );

		int iAddedBubbleCount = pBubbleSystem->GetBubbleCountByTypeID( m_iBubbleTypeID );

		// Send bubble add packet to client.
		if( m_hActor )
		{
			// Build the packet after the icon index is set.
			char acBuffer[ 32 ] = { 0 };
			CPacketCompressStream Stream( acBuffer, 32 );
			Stream.Write( &m_iBubbleTypeID, sizeof(m_iBubbleTypeID) );
			Stream.Write( &m_iIconIndex, sizeof(m_iIconIndex) );
			Stream.Write( &m_fDurationTime, sizeof(m_fDurationTime) );
			Stream.Write( &iAddedBubbleCount, sizeof(iNowBubbleCount) );

			m_hActor->Send( eActor::SC_ADDBUBBLE, &Stream );
		}
	}
	else
	{
		// Since max bubble count is exceeded, only refresh the duration of existing bubbles.
		if( 0 < pBubbleSystem->GetBubbleCountByTypeID( m_iBubbleTypeID ) )
		{
			// Update duration.
			pBubbleSystem->SetDurationTime( m_iBubbleTypeID, m_fDurationTime );
			
			char acBuffer[ 32 ] = { 0 };
			CPacketCompressStream Stream( acBuffer, 32 );
			Stream.Write( &m_iBubbleTypeID, sizeof(int) );
			Stream.Write( &m_iIconIndex, sizeof(m_iIconIndex) );
			Stream.Write( &m_fDurationTime, sizeof(float) );
			Stream.Write( &iNowBubbleCount, sizeof(int) );

			m_hActor->Send( eActor::SC_REFRESH_BUBBLE_DURATIONTIME, &Stream );
		}
	}
}
//////////////////////////////////////////////////////////////////////////


// CDnRemoveBubbleHandler
void CDnRemoveBubbleHandler::Initialize( DnActorHandle hActor, const char* pArgument )
{
	if( !hActor || NULL == pArgument )
		return;

	m_hActor = hActor;

	std::vector<string> vlTokens;
	string strArgument( pArgument );
	TokenizeA( strArgument, vlTokens, ";" );

	_ASSERT( 2 == (int)vlTokens.size() );

	m_iBubbleTypeID = atoi( vlTokens.at(0).c_str() );
	m_iRemoveCount = atoi( vlTokens.at(1).c_str() );

	SecureZeroMemory( &m_PacketBuffer, sizeof(m_PacketBuffer) );
	m_pPacketStream.reset( new CPacketCompressStream( &m_PacketBuffer, sizeof(m_PacketBuffer) ) );
	m_pPacketStream->Write( &m_iBubbleTypeID, sizeof(m_iBubbleTypeID) );
	m_pPacketStream->Write( &m_iRemoveCount, sizeof(m_iRemoveCount) );
}

void CDnRemoveBubbleHandler::ProcessEvent( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent )
{
	pBubbleSystem->RemoveBubbleByTypeID( m_iBubbleTypeID, m_iRemoveCount );

	// Send bubble remove packet to client.
	if( m_hActor )
	{
		m_hActor->Send( eActor::SC_REMOVEBUBBLE, m_pPacketStream.get() );
	}
}
//////////////////////////////////////////////////////////////////////////

// CDnUpdateStateEffectHandler
void CDnUpdateStateEffectHandler::Initialize( DnActorHandle hActor, const char* pArgument )
{
	if( !hActor || NULL == pArgument )
		return;

	m_hActor = hActor;

	std::vector<string> vlTokens;
	string strArgument( pArgument );
	TokenizeA( strArgument, vlTokens, ";" );

	_ASSERT( 2 == (int)vlTokens.size() );

	m_iBubbleTypeID = atoi( vlTokens.at(0).c_str() );
	m_iSkillID = atoi( vlTokens.at(1).c_str() );
}

void CDnUpdateStateEffectHandler::ProcessEvent( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* /*pEvent*/ )
{
	// Delete any existing bubble state effect
	pBubbleSystem->RemoveBubbleStateBlow( m_iBubbleTypeID );

	// Apply leveling based on bubble count.
	int iLevel = pBubbleSystem->GetBubbleCountByTypeID( m_iBubbleTypeID );
	m_hSkill = CDnSkill::CreateSkill( m_hActor, m_iSkillID, iLevel );

	// Failed to create skill. Missing or invalid data; both pvp/pve data must exist.
	if( !m_hSkill )
		return;

	int iNumStateEffect = m_hSkill->GetStateEffectCount();
	for( int iSE = 0; iSE < iNumStateEffect; ++iSE )
	{
		const CDnSkill::StateEffectStruct* pSE = m_hSkill->GetStateEffectFromIndex( iSE );
		if( CDnSkill::ApplySelf == pSE->ApplyType )
		{
			// State effect add packet. Deletion of state effects added here is handled by CDnBubbleSystem
			// Process it. It doesn't look great, but for now..
			int iBlowID = m_hActor->CmdAddStateEffect( NULL, (STATE_BLOW::emBLOW_INDEX)pSE->nID, -1, pSE->szValue.c_str() );
			_ASSERT( -1 != iBlowID );
			if( -1 != iBlowID )
			{
				pBubbleSystem->AddBubbleStateBlow( m_iBubbleTypeID, iBlowID );
			}
		}
	}

	SAFE_RELEASE_SPTR( m_hSkill );
}
//////////////////////////////////////////////////////////////////////////

// CDnEnchantSkillOnce
void CDnEnchantSkillOnce::Initialize( DnActorHandle hActor, const char* pArgument )
{
	if( !hActor || NULL == pArgument )
		return;

	m_hActor = hActor;
	_ASSERT( m_hActor->IsPlayerActor() );

	std::vector<string> vlTokens;
	string strArgument( pArgument );
	TokenizeA( strArgument, vlTokens, ";" );

	_ASSERT( 2 == (int)vlTokens.size() );

	m_iTargetSkillID = atoi( vlTokens.at(0).c_str() );
	m_iEnchantSkillID = atoi( vlTokens.at(1).c_str() );
}

void CDnEnchantSkillOnce::ProcessEvent( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* /*pEvent*/ )
{
	if( m_hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		pPlayerActor->ApplyEnchantSkillOnceFromBubble( m_iTargetSkillID, m_iEnchantSkillID );

		// Client skill is not enhanced; only server processes enhancement once.
		// Since the goal is to modify the original skill state effects and all state effects are sent via packets from the server
		// Handle swapping once when using a skill only on game server.
		// Currently there is no way on the client to pre-enhance a skill before using it, so do it this way.
	}
} 
//////////////////////////////////////////////////////////////////////////

}