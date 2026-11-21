#include "Stdafx.h"

#ifdef PRE_ADD_DONATION

#include <algorithm>

#include "VillageSendPacket.h"
#include "DnActor.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnContributionDlg.h"
#include "DnContributionTask.h"


// Sort ranking list. - For tie-breaking.
void CDnContributionTask::RankerListSort()
{	
	int size = (int)m_vListRanker.size();
	for( int i=0; i<size; ++i )
	{
		for( int k=i+1; k<size; ++k )
		{
			// Next item is larger; swap order.
			if( m_vListRanker[ i ]->nGold < m_vListRanker[ k ]->nGold )
			{
				m_vListRanker[ k ]->nRank = m_vListRanker[ i ]->nRank; // Rank changed.
				_Swap( m_vListRanker[ i ], m_vListRanker[ k ] ); // swap.
			}

			// Tie.
			else if( m_vListRanker[ i ]->nGold == m_vListRanker[ k ]->nGold )
			{
				m_vListRanker[ k ]->nRank = m_vListRanker[ i ]->nRank; // Rank changed.
			}
		}
	}
}

// Sort ranker list. Descending.
bool RankerSort( SContributionRanker * lVal, SContributionRanker * rVal ){	
	return rVal->nGold < lVal->nGold;
}

CDnContributionTask::CDnContributionTask()
: CTaskListener( false )
, m_nContributionGold( 0 )
, m_nLPGold( 0 )
, m_nLPRank( 0 )
{
}

bool CDnContributionTask::Initialize()
{
	return true;
}

// Remove ranking list.
void CDnContributionTask::ReleaseRankerList()
{
	// Donation ranker.
	int size = (int)m_vListRanker.size();
	for( int i=0; i<size; ++i )
		SAFE_DELETE( m_vListRanker[ i ] );
	m_vListRanker.clear();  
}

void CDnContributionTask::Finalize()
{
	// Remove ranking list.
	ReleaseRankerList();

	m_vRewardMouth.clear(); // Monthly.
	m_vRewardDay.clear();   // Daily.
}


void CDnContributionTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	if( nMainCmd == eSCMainCmd::SC_DONATION )
	{
		switch( nSubCmd )
		{		
		// Refresh ranker list.
		case eDonation::SC_DONATION_RANKING:
			OnRecvRankListMessage( pData );
			break;

		// Donation result.
		case eDonation::SC_DONATE:
			OnRecvContributionResult( pData );
			break;

		// Open donation window.
		case eDonation::SC_DONATION_OPEN:
			OnRecvShowContribution();
			break;
		}
	}
}

 
// Open donation window.
void CDnContributionTask::OnRecvShowContribution()
{
	GetInterface().OpenContributionDialog( true );
}


// Called when DonationRewardDlg is first opened.
// : No need to preload reward items, so load on first Open.
void CDnContributionTask::OnFirstOpenRewardDlg()
{
	//// EXT.
	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TCONTRIBUTION );
	_ASSERT( pTable && "Failed to read Table [ donationcompensationtable ]." );
	DNTableCell * pCell = NULL;

	if( pTable )
	{
		char strField[32]={0,};
		m_vRewardMouth.resize( 11 );
		m_vRewardDay.resize( 11 );

		int size = pTable->GetItemCount();
		_ASSERT( (size<1) && "Table [ donationcompensationtable ] is empty." );

		for( int i=0; i<size; ++i )
		{
			SContributionRewardItems rowItem;

			int itemID = pTable->GetItemID( i );
			rowItem.nRank = pTable->GetFieldFromLablePtr( itemID, "_Grade" )->GetInteger();			 
			rowItem.nType = pTable->GetFieldFromLablePtr( itemID, "_Type" )->GetInteger();			

			for( int k=0; k<5; ++k )
			{
				sprintf_s( strField, 32, "_Compensation%d", k + 1 );
				pCell = pTable->GetFieldFromLablePtr( itemID, strField );
				if( pCell )
					rowItem.nItemID[ k ] = pCell->GetInteger();

				sprintf_s( strField, 32, "_CompensationValue%d", k + 1 );
				pCell = pTable->GetFieldFromLablePtr( itemID, strField );
				if( pCell )
					rowItem.nItemCnt[ k ] = pCell->GetInteger();				
			}
			// Monthly.
			if( rowItem.nType == 0 )
				m_vRewardMouth[ rowItem.nRank ] = rowItem;
			
			// Daily.
			else if( rowItem.nType == 1 )
				m_vRewardDay[ rowItem.nRank ] = rowItem;
		}

	}

}


// Return LocalPlayer index in ranking list.
int CDnContributionTask::GetRListToLPIndex( std::wstring & strID )
{	
	int size = (int)m_vListRanker.size();
	for( int i=0; i<size; ++i )	
	{
		if( m_vListRanker[i]->strID == strID )
		{					
			return i;
		}		
	}	
	return -1;
}

// Sort ranking list - on donation, if LocalPlayer is in top 10, update my list's gold and sort.
void CDnContributionTask::SortRankerList( INT64 nContributionGold )
{	
	std::wstring strID = CDnActor::s_hLocalActor->GetName();
	int index = GetRListToLPIndex( strID );
	if( index == -1 )
		return;
	
	// Update my donation (my donation is not sent on every donation, so store and calculate on client..
	m_nLPGold += m_nContributionGold; // LocalPlayer.
	m_vListRanker[ index ]->nGold += nContributionGold; // List Ranker.
	
	// Sort.
	std::sort( m_vListRanker.begin(), m_vListRanker.end(), RankerSort );
	// RankerListSort(); Sort for tie-breaking. Commented out since ties (same gold donated) are not handled.
		
	//// LocalPlayer's rank after sorting. For sorting.
	//index = GetRListToLPIndex( strID );
	//if( index != -1 )
	//{	
	//	m_nLPRank = m_vListRanker[ index ]->nRank;
	//}


	// Reset Rank of List after Sort.
	int size = (int)m_vListRanker.size();
	for( int i=0; i<size; ++i )
		m_vListRanker[ i ]->nRank = i + 1;

	// Rank of LocalPlayer.
	index = GetRListToLPIndex( strID );
	if( index != -1 )
		m_nLPRank = m_vListRanker[ index ]->nRank;

}



//-----------------------------------------------
// Packet result.

// Ranking list refresh result.
void CDnContributionTask::OnRecvRankListMessage( char * pData )
{
	// Remove ranking list.
	ReleaseRankerList();


	SCDonationRanking * aRankList = ( SCDonationRanking * )pData;
		
	// LocalPlayer.
	m_nLPRank = aRankList->nMyRanking;
	/*if( m_nLPRank == -1 )
		m_nLPRank = 10000;
	else if( m_nLPRank < -1 )
		m_nLPRank = 0;*/
	if( m_nLPRank < 0 )
		m_nLPRank = -1;


	m_nLPGold = aRankList->nMyCoin;	

	// Ranker List.		
	for( int i=0; i<10; ++i )
	{	
		const Donation::Ranking & aRanker = aRankList->Ranking[ i ];	
		if( aRanker.biCharacterID == 0 || _wcsicmp( aRanker.wszCharacterName, L"" ) == 0 )
			break;

		m_vListRanker.push_back( new SContributionRanker( i+1, aRanker.btJobCode, (wchar_t*)(aRanker.wszGuildName), (wchar_t*)(aRanker.wszCharacterName), aRanker.nCoin ) );
	}

	// Refresh donation ranking list.
	CDnContributionDlg *pDlg = static_cast<CDnContributionDlg *>( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CONTRIBUTION_DIALOG ) );
	if( pDlg ) 		
		pDlg->RefreshList();
}

// Donation result.
void CDnContributionTask::OnRecvContributionResult( char * pData )
{
	SCDonate * pMsg = ( SCDonate * )pData;

	if( pMsg->nResult == 0 )
	{
		if( m_nContributionGold >= 10000 )
		{
			wchar_t buf[1024]={0,};
			swprintf_s( buf, 1024, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7332 ), CDnActor::s_hLocalActor->GetName(), m_nContributionGold/10000 );		
			GetInterface().MessageBox( buf );
		}

		// Sort ranking list.
		SortRankerList( m_nContributionGold );
		m_nContributionGold = 0; // Reset requested donation amount.

		// Refresh donation dialog.
		CDnContributionDlg *pDlg = static_cast<CDnContributionDlg *>( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CONTRIBUTION_DIALOG ) );
		if( pDlg ) 		
		{
			pDlg->RefreshList(); // Refresh donation ranking list.		
			pDlg->RefreshContributionGoldDlg(); // Refresh Donate Dialog - current gold.
		}
	}
	else
	{
		m_nContributionGold = 0; // Reset requested donation amount.

		// Server connection is unstable.
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, pMsg->nResult ) );
	}	
}


//-----------------------------------------------------------------
// Send packet.

// Send donation packet.
void CDnContributionTask::OnSendMakeDonations( INT64 nCoin )
{
	m_nContributionGold = nCoin; // Save requested donation amount.
	SendMakeDonationsMsg( nCoin );
}


// Send ranking list refresh packet.
void CDnContributionTask::OnSendRefreshRankerList()
{
	
	SendRefreshRankerList();
}


#endif

