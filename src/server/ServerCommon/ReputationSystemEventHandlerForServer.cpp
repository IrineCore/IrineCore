
#include "stdafx.h"
#include "ReputationSystemEventHandlerForServer.h"
#include "ReputationSystemRepository.h"
#include "DNUserSession.h"
#include "DNDBConnection.h"
#include "NpcReputationSystem.h"
#include "DNGameDataManager.h"
#include "TimeSet.h"

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

CServerReputationSystemEventHandler::CServerReputationSystemEventHandler( CDNUserSession* pSession )
:m_pSession(pSession)
{

}

void CServerReputationSystemEventHandler::OnComplete( IReputationSystem* pReputation, IReputationSystem::eType Type )
{
	switch( Type )
	{
		case IReputationSystem::NpcMalice:
		{
#if defined( PRE_REMOVE_NPCREPUTATION_MALICE )
#else
			CReputationSystemRepository* pRepository = m_pSession->GetReputationSystem();
			_ASSERT( pRepository );

			_ASSERT( dynamic_cast<CNpcReputationSystem*>(pReputation) );
			CNpcReputationSystem* pNpcReputaion = static_cast<CNpcReputationSystem*>(pReputation);

			// Person involved
			pRepository->SetNpcReputation( pNpcReputaion->GetNpcID(), IReputationSystem::NpcFavor, 0 );	// Initialize favor
			pRepository->SetNpcReputation( pNpcReputaion->GetNpcID(), Type, 0 );							// Reset malice

			TReputeTableData* pReputationTable = g_pDataManager->GetReputeTableData( pNpcReputaion->GetNpcID() );
			if (!pReputationTable) break;

			// Close friend
			for( UINT i=0 ; i<pReputationTable->vFavorNpcID.size() ; ++i )
			{
				REPUTATION_TYPE value = pRepository->GetNpcReputation( pReputationTable->vFavorNpcID[i], IReputationSystem::NpcFavor );
				value = value*pReputationTable->iFavorGroupBomb/100;

				pRepository->AddNpcReputation( pReputationTable->vFavorNpcID[i], IReputationSystem::NpcFavor, -value );	// Decrease favorability
			}

			// Disliked person
			for( UINT i=0 ; i<pReputationTable->vMaliceNpcID.size() ; ++i )
			{
				REPUTATION_TYPE value = pRepository->GetNpcReputation( pReputationTable->vMaliceNpcID[i], IReputationSystem::NpcFavor );
				value = value*pReputationTable->iMaliceGroupBomb/100;

				pRepository->AddNpcReputation( pReputationTable->vMaliceNpcID[i], IReputationSystem::NpcFavor, value );	// Increase favorability
			}
#endif // #if defined( PRE_REMOVE_NPCREPUTATION_MALICE )

			break;
		}
	}
}

void CServerReputationSystemEventHandler::OnChange( IReputationSystem::eType Type )
{
}

#if defined( PRE_ADD_REPUTATION_EXPOSURE )
void CServerReputationSystemEventHandler::OnCommit( std::map<IReputationSystem*,size_t>& Reputation, bool bClientSend )
{
	if( m_pSession == NULL || m_pSession->GetDBConnection() == NULL || m_pSession->GetReputationSystem() == NULL )
		return;

	std::vector<TNpcReputation> vData;
	m_pSession->GetReputationSystem()->GetCommitData( vData );

	if( vData.empty() )
		return;

	// Send DB M/W
	CDNDBConnection* pDBCon = m_pSession->GetDBConnection();
	pDBCon->QueryModNpcReputation( m_pSession, vData );

	// Send user
	if( bClientSend )
		m_pSession->SendModReputation( vData );
}
#else
void CServerReputationSystemEventHandler::OnCommit( std::map<IReputationSystem*,int>& Reputation, bool bClientSend )
{
	if( m_pSession == NULL || m_pSession->GetDBConnection() == NULL || m_pSession->GetReputationSystem() == NULL )
		return;

	std::vector<TNpcReputation> vData;
	m_pSession->GetReputationSystem()->GetCommitData( vData );

	if( vData.empty() )
		return;

	// Send DB M/W
	CDNDBConnection* pDBCon = m_pSession->GetDBConnection();
	pDBCon->QueryModNpcReputation( m_pSession, vData );

	// Send user
	if( bClientSend )
		m_pSession->SendModReputation( vData );
}
#endif // #if defined( PRE_ADD_REPUTATION_EXPOSURE )

void CServerReputationSystemEventHandler::OnConnect( __time64_t tTime, bool bSend )
{
	std::vector<int>			vNpcData;
	std::vector<TNpcReputation>	vNpcReputation;

	CReputationSystemRepository* pReputationSystem = m_pSession->GetReputationSystem();
	pReputationSystem->GetAllNpcData( vNpcData );

	for( UINT i=0 ; i<vNpcData.size() ; ++i )
	{
		vNpcReputation.push_back( TNpcReputation( vNpcData[i],
												  static_cast<int>(pReputationSystem->GetNpcReputation(vNpcData[i],IReputationSystem::NpcFavor)),
												  static_cast<int>(pReputationSystem->GetNpcReputation(vNpcData[i],IReputationSystem::NpcMalice)) ) );
	}		

#if defined( _VILLAGESERVER )
	// Amount decreased per day 
	if( bSend == false && m_pSession->GetLastServerType() == SERVERTYPE_LOGIN )
	{
		CTimeSet timeSet;
		__time64_t tLocalTime = timeSet.GetTimeT64_LC();

		INT64	GapSec	= tLocalTime-tTime;
		int		GapDay	= static_cast<int>(GapSec/(60*60*24));

		//if( GapDay > 0 ) // Feature temporarily disabled
		if(0)
		{
			for( UINT i=0 ; i<vNpcData.size() ; ++i )
			{
				TReputeTableData* pReputationTable = g_pDataManager->GetReputeTableData( vNpcData[i] );
				if( pReputationTable == NULL )
					continue;


				pReputationSystem->AddNpcReputation( vNpcData[i], IReputationSystem::NpcFavor, pReputationTable->iTakeFavorPerDay*GapDay*-1, false );
				pReputationSystem->AddNpcReputation( vNpcData[i], IReputationSystem::NpcMalice, pReputationTable->iAddMalicePerDay*GapDay, false );

				for( UINT j=0 ; j<vNpcReputation.size() ; ++j )
				{
					if( vNpcReputation[j].iNpcID == vNpcData[i] )
					{
						vNpcReputation[j].iFavorPoint	= static_cast<int>(pReputationSystem->GetNpcReputation(vNpcData[i],IReputationSystem::NpcFavor));
						vNpcReputation[j].iMalicePoint	= static_cast<int>(pReputationSystem->GetNpcReputation(vNpcData[i],IReputationSystem::NpcMalice));
					}
				}

			}

			pReputationSystem->Commit( false );
		}
	}
#endif // #if defined( _VILLAGESERVER )

	if( bSend )
	{
		m_pSession->SendReputationList( vNpcReputation );
		m_pSession->SendUnionMembershipList(m_pSession->GetItem());
#if defined (_VILLAGESERVER)
		m_pSession->SendUnionPoint();
#endif // #if defined (_VILLAGESERVER)
	}
}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
