#include "StdAfx.h"
#include "DnNpcReaction.h"
#include "DnTableDB.h"
#include "DnWorldProp.h"
#include "DnNpcActor.h"
#include "ReputationSystemRepository.h"
#include "DnChatOption.h"

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

const float NPC_REACTION_CHECK_RANGE = 300.0f;


CDnNpcReaction::CDnNpcReaction(void) : m_bInitialized( false ), m_pReputationRepos(NULL)
{

}

CDnNpcReaction::~CDnNpcReaction(void)
{

}

// Call only once when first connecting to town.
void CDnNpcReaction::Initialize( CReputationSystemRepository* pReputationRepos )
{
	_ASSERT( pReputationRepos );
	m_pReputationRepos = pReputationRepos;

	// Read table
	DNTableFileFormat*  pNpcReactionTable = GetDNTable( CDnTableDB::TNPCREACTION );

	char acBuffer[ 256 ] = { 0 };
	int iItemCount = pNpcReactionTable->GetItemCount();
	for( int iIndex = 0; iIndex < iItemCount; ++iIndex )
	{
		int iItemID = pNpcReactionTable->GetItemID( iIndex );

		S_NPC_REACTION_INFO Info;
		Info.iNpcID = pNpcReactionTable->GetFieldFromLablePtr( iItemID, "_NpcID" )->GetInteger();

		if(Info.iNpcID <= 0 )
			continue;

		for( int i = 0; i < NPC_REACTION_COUNT_MAX; ++i )
		{
			sprintf_s( acBuffer, "_Favor%d", i+1 );
			Info.aiFavorThresholdPercent[ i ] = pNpcReactionTable->GetFieldFromLablePtr( iItemID, acBuffer )->GetInteger();

			// If the affinity ratio threshold is 0, stop there.
			if( 0 < Info.aiFavorThresholdPercent[ i ] )
			{
				sprintf_s( acBuffer, "_NpcTalk%d", i+1 );
				Info.aiSpeechUIStringID[ i ] = pNpcReactionTable->GetFieldFromLablePtr( iItemID, acBuffer )->GetInteger();

				sprintf_s( acBuffer, "_ActFile%d", i+1 );
				const char* pReactionActionName = pNpcReactionTable->GetFieldFromLablePtr( iItemID, acBuffer )->GetString();
				Info.astrEffectActionName[ i ].assign( pReactionActionName );
			}
		}

		m_mapNpcReaction.insert( make_pair(Info.iNpcID, Info) );
	}

	// Anger, disappointment
	m_vlSCNpcEffectActionName.push_back( "Expression_Angry" );
	m_vlSCNpcEffectActionName.push_back( "Expression_Disappoint" );

	m_bInitialized = true;
}


// Called whenever moving to village or other maps.
void CDnNpcReaction::Clear( void )
{
	m_setNpcsInRange.clear();
}


// Attach server-requested effect to specific NPC.
void CDnNpcReaction::AttachNpcEffect( UINT uiNpcID, int iEffectdIndex )
{
	_ASSERT( iEffectdIndex < (int)m_vlSCNpcEffectActionName.size() );
	if( (int)m_vlSCNpcEffectActionName.size() <= iEffectdIndex )
		return;

	CDnNPCActor* pNpcActor = CDnNPCActor::FindNpcActorFromID( uiNpcID );
	if( pNpcActor )
	{
		// Set effect.
		DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
		if( hHandle )
		{
			_ASSERT( false == m_vlSCNpcEffectActionName.at(iEffectdIndex).empty() );
			hHandle->SetActionQueue( m_vlSCNpcEffectActionName.at(iEffectdIndex).c_str() );
			hHandle->SetPosition( *pNpcActor->GetPosition() );
			pNpcActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );
		}
	}
}


void CDnNpcReaction::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !CDnActor::s_hLocalActor )
		return;

	// Search nearby for NPCs. Set range arbitrarily for now.
	// Show speech bubbles only to newly entered NPCs.
	// Remove speech bubble when out of range.
	DNVector( DnActorHandle ) vlhResult;
	CDnActor::ScanActor( *CDnActor::s_hLocalActor->GetPosition(), NPC_REACTION_CHECK_RANGE, vlhResult );

	set<DWORD> setPrevNpcsInRange = m_setNpcsInRange;
	m_setNpcsInRange.clear();

	int iNumActor = (int)vlhResult.size();
	for( int iActor = 0; iActor < iNumActor; ++iActor )
	{
		DnActorHandle hActor = vlhResult.at( iActor );
		if( CDnActorState::Npc == hActor->GetActorType() )		// NPCs only.
			m_setNpcsInRange.insert( hActor->GetUniqueID() );
	}

	// Filter out newly entered NPCs.
	DNVector( DWORD ) vlNewNpcs;
	set_difference( m_setNpcsInRange.begin(), m_setNpcsInRange.end(),
					setPrevNpcsInRange.begin(), setPrevNpcsInRange.end(),
					back_inserter(vlNewNpcs) );

	// Filter out NPCs that were in range and have left.
	DNVector( DWORD ) vlOutNpcs;
	set_difference( setPrevNpcsInRange.begin(), setPrevNpcsInRange.end(),
					m_setNpcsInRange.begin(), m_setNpcsInRange.end(),
					back_inserter(vlOutNpcs) );
	
	// Make newly entered NPCs perform a reaction.
	iNumActor = (int)vlNewNpcs.size();
	for( int iActor = 0; iActor < iNumActor; ++iActor )
	{
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( vlNewNpcs.at( iActor ) );
		_ASSERT( hActor );
		if( !hActor )
			continue;

		_ASSERT( hActor->IsNpcActor() );
		CDnNPCActor* pNpcActor = static_cast<CDnNPCActor*>( hActor.GetPointer() );
		int iNpcID = pNpcActor->GetNpcData().nNpcID;

		map<int, S_NPC_REACTION_INFO>::iterator iter = m_mapNpcReaction.find( iNpcID );
		if( m_mapNpcReaction.end() != iter )
		{
			const S_NPC_REACTION_INFO& Info = iter->second;
			_ASSERT( Info.iNpcID == iNpcID );

			int iReactionIndex = -1;
			int iCurrentPercent = m_pReputationRepos->GetNpcReputationPercent( Info.iNpcID, IReputationSystem::NpcFavor );
			for( int k = 0; k < NPC_REACTION_COUNT_MAX; ++k )
			{
				int iThresholdPercent = Info.aiFavorThresholdPercent[ k ];
				if( 0 != iThresholdPercent )
				{
					if( iThresholdPercent <= iCurrentPercent )
					{
						iReactionIndex = k;
					}
				}
			}

			// Process reactions only when affinity is at or above a certain value.
			if( -1 < iReactionIndex )
			{
				// Show speech bubble
				hActor->SetChatBalloonText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, Info.aiSpeechUIStringID[iReactionIndex] ), GetTickCount(), CHAT_REPUTE );
				hActor->GetChatBalloon().SetAdditionalYOffset( (pNpcActor->GetNPCStateMark() > 0) ? 0.1f : 0.0f );

				// Set effect.
				DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
				if( hHandle )
				{
					_ASSERT( false == Info.astrEffectActionName[ iReactionIndex ].empty() );
					hHandle->SetActionQueue( Info.astrEffectActionName[ iReactionIndex ].c_str() );
					hHandle->SetPosition( *hActor->GetPosition() );
					hActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );
				}

				// TODO: Later could make it perform a specific action.
			}
		}
		
	}

	// Stop reacting to NPCs that are out of range.
	// Make newly entered NPCs perform a reaction.
	iNumActor = (int)vlOutNpcs.size();
	for( int iActor = 0; iActor < iNumActor; ++iActor )
	{
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( vlOutNpcs.at( iActor ) );
		_ASSERT( hActor );
		if( !hActor )
			continue;

		_ASSERT( hActor->IsNpcActor() );
		CDnNPCActor* pNpcActor = static_cast<CDnNPCActor*>( hActor.GetPointer() );
		int iNpcID = pNpcActor->GetNpcData().nNpcID;

		map<int, S_NPC_REACTION_INFO>::iterator iter = m_mapNpcReaction.find( iNpcID );
		if( m_mapNpcReaction.end() != iter )
		{
			// End ongoing reactions.
			hActor->StopHeadEffect( CDnHeadEffectRender::HeadEffect_ChatBalloon );

			// Remove effect.
			// Leave as is because this effect plays once and destroys itself via a destroy signal at the end of the action.

			// TODO: Later could make it perform a specific action.
		}

	}

}

#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM