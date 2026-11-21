#include "stdafx.h"
#include "DnWorldZoneSelectListDlg.h"
#include "DnQuestTask.h"
#include "DnTableDB.h"
#include "DnPartyTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnWorldZoneSelectListDlg::CDnWorldZoneSelectListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
, m_pStaticName( NULL )
, m_pStaticQuestImage( NULL )
, m_pStaticQuestText( NULL )
, m_bIsEnterStage( false )
{
}

CDnWorldZoneSelectListDlg::~CDnWorldZoneSelectListDlg(void)
{
}

void CDnWorldZoneSelectListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "WorldZoneSelectListDlg.ui" ).c_str(), bShow );
}

void CDnWorldZoneSelectListDlg::InitialUpdate()
{
	m_pStaticName = GetControl<CEtUIStatic>( "ID_TEXT_MAPNAME" );
	m_pStaticQuestImage = GetControl<CEtUIStatic>( "ID_STATIC_QUEST" );
	m_pStaticQuestText = GetControl<CEtUIStatic>( "ID_TEXT_QUESTCOUNT" );
}

void CDnWorldZoneSelectListDlg::SetMapName( const std::wstring & szMapName, const bool bEnable )
{
	m_pStaticName->SetText( szMapName );

	if( false == bEnable )
	{
		m_pStaticName->SetTooltipText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1400023 ) );	// UISTRING : Entry requirements not met.
		m_pStaticName->Enable( false );
	}
}

void CDnWorldZoneSelectListDlg::SetQuestInfo( const int nMapIndex )
{
	std::vector<TQuest*> vecProgMainQuest, vecProgSubQuest;
	GetQuestTask().GetPlayMainQuest( vecProgMainQuest );
	GetQuestTask().GetPlaySubQuest( vecProgSubQuest );

	std::vector<int> vecDungeonIndex;
	GetTargetMapDungeonIndex( nMapIndex, vecDungeonIndex );

	int nEnableQuestCount = GetEnableQuestCount( vecProgMainQuest, vecDungeonIndex );
	nEnableQuestCount += GetEnableQuestCount( vecProgSubQuest, vecDungeonIndex );

	if( 0 == nEnableQuestCount )
	{
		m_pStaticQuestImage->Show( false );
		m_pStaticQuestText->Show( false );
		return;
	}

	m_pStaticQuestImage->Show( true );
	m_pStaticQuestText->Show( true );
	m_pStaticQuestText->SetIntToText( nEnableQuestCount );
}

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST

int CDnWorldZoneSelectListDlg::GetEnableQuestCountWithMapIndex( int nMapIndex )
{
	std::vector<TQuest*> vecProgMainQuest, vecProgSubQuest;
	GetQuestTask().GetPlayMainQuest( vecProgMainQuest );
	GetQuestTask().GetPlaySubQuest( vecProgSubQuest );
		
	std::vector<int> vecDungeonIndex;
	vecDungeonIndex.push_back(nMapIndex);

	return GetEnableQuestCount( vecProgMainQuest, vecDungeonIndex ) + GetEnableQuestCount( vecProgSubQuest, vecDungeonIndex );
}
#endif 

int CDnWorldZoneSelectListDlg::GetEnableQuestCount( std::vector<TQuest*> & vecQuestList, std::vector<int> & vecDungeonIndex )
{
	int nRetVal = 0;

#ifdef PRE_FIX_QUEST_COUNT_WORLDZONE
	DWORD i = 0;
	for (; i < (int)vecQuestList.size(); ++i)
	{
		const TQuest* pCurQuest = vecQuestList[i];
		if (pCurQuest)
		{
			if (HasEnableQuestInDungeons(*pCurQuest, vecDungeonIndex))
				++nRetVal;
		}
	}
#else
	for( DWORD itr = 0; itr < vecDungeonIndex.size(); ++itr )
		nRetVal += GetEnableQuestCount( vecDungeonIndex[itr], vecQuestList );
#endif

	return nRetVal;
}

#ifdef PRE_FIX_QUEST_COUNT_WORLDZONE
bool CDnWorldZoneSelectListDlg::HasEnableQuestInDungeons(const TQuest& quest, const std::vector<int> vecDungeonIndexes) const
{
	Journal* pJournal = g_DataManager.GetJournalData(quest.nQuestID);
	if (!pJournal)
		return false;

	JournalPage* pJournalPage = pJournal->FindJournalPage(quest.cQuestJournal);
	if (!pJournalPage)
		return false;

	std::vector<int>::const_iterator iter = vecDungeonIndexes.begin();
	for (; iter != vecDungeonIndexes.end(); ++iter)
	{
		int nMapIndex = (*iter);

		if (pJournalPage->nDestnationMapIndex == nMapIndex)
			return true;

		int i = 0;
		for (; i < (int)pJournalPage->CustomDestnationMark.size(); ++i)
		{
			if (pJournalPage->CustomDestnationMark[i].nMapIdx == nMapIndex)
				return true;
		}
	}

	return false;
}
#endif

int CDnWorldZoneSelectListDlg::GetEnableQuestCount( const int nMapIndex, std::vector<TQuest*> & vecQuestList )
{
	int nRetVal = 0;

	std::vector<TQuest*>::iterator iter = vecQuestList.begin();
	for( ; iter != vecQuestList.end(); iter++ )
	{
		Journal* pJournal = g_DataManager.GetJournalData( (*iter)->nQuestID );
		if( !pJournal )
			continue;

		JournalPage* pJournalPage = pJournal->FindJournalPage( (*iter)->cQuestJournal );
		if( !pJournalPage )
			continue;

		if( pJournalPage->nDestnationMapIndex == nMapIndex )
			++nRetVal;
		else
		{
			for( int i=0; i<(int)pJournalPage->CustomDestnationMark.size(); i++ )
			{
				if( pJournalPage->CustomDestnationMark[i].nMapIdx == nMapIndex )
					++nRetVal;
			}
		}
	}

	return nRetVal;
}

void CDnWorldZoneSelectListDlg::GetTargetMapDungeonIndex( const int nTargetMapIndex, std::vector<int> & vecDungeonIndex )
{
	vecDungeonIndex.clear();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONMAP );

	const int nCurrentMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
	char szLabel[256] = {0,};

	vecDungeonIndex.push_back( nTargetMapIndex );
	for( int itr = 0; itr < DNWORLD_GATE_COUNT; ++itr )
	{
		std::vector<int> vecMapIndex;
		CDnWorld::GetGateMapIndex( nTargetMapIndex, itr, vecMapIndex );

		for( DWORD jtr = 0; jtr < vecMapIndex.size(); ++jtr )
		{
			int nMapIndex = vecMapIndex[jtr];
			
			if( nCurrentMapIndex == nMapIndex )
				continue;

			if( DUNGEONGATE_OFFSET <= nMapIndex )
			{
				if( pDungeonSox->IsExistItem( nMapIndex ) == false )
					continue;

				for( int ktr = 0; ktr < 5; ++ktr )
				{
					sprintf_s( szLabel, "_MapIndex%d", ktr + 1 );
					int nDungeonMapIndex = pDungeonSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetInteger();
					if( nDungeonMapIndex < 1 )
						continue;

					vecDungeonIndex.push_back( nDungeonMapIndex );
				}
			}
		}
	}
}

bool CDnWorldZoneSelectListDlg::IsEnterStage( const int nMapIndex )
{
	if( !CDnActor::s_hLocalActor ) return false;
	CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));
	if( !localActor ) return false;

	const int nEnterLevel = GetStageEnterLevel( nMapIndex );

	DWORD dwPartyCount = GetPartyTask().GetPartyCount();
	for( DWORD itr = 0; itr < dwPartyCount; ++itr )
	{
		CDnPartyTask::PartyStruct * pData = GetPartyTask().GetPartyData( itr );

		if( NULL == pData )
			continue;

		if( nEnterLevel > pData->cLevel )
			return false;
	}

	return true;
}

int CDnWorldZoneSelectListDlg::GetStageEnterLevel( const int nMapIndex )
{
	int nRetVal = 0;

	std::vector<int> nVecItemList;
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAPGATE );
	pSox->GetItemIDListFromField( "_MapIndex", nMapIndex, nVecItemList );

	for( DWORD itr = 0; itr < nVecItemList.size(); ++itr )
	{
		int nGateIndex = pSox->GetFieldFromLablePtr( nVecItemList[itr], "_GateIndex" )->GetInteger();

		if( 0 != nGateIndex )
			continue;

		nRetVal = pSox->GetFieldFromLablePtr( nVecItemList[itr], "_PermitPlayerLevel" )->GetInteger();
		break;
	}

	return nRetVal;
}

void CDnWorldZoneSelectListDlg::SetGateInfo( CDnWorld::GateStruct * pGateStruct )
{
	m_bIsEnterStage = IsEnterStage( pGateStruct->nMapIndex );

	SetMapName( pGateStruct->szMapName, m_bIsEnterStage );
	SetQuestInfo( pGateStruct->nMapIndex );
}
