#include "StdAfx.h"
#include "DnQuestTask.h"
#include "DnNPCActor.h"
#include "DnInterface.h"
#include "DnChatTabDlg.h"
#include "DnTableDB.h"
#include "DnInvenTabDlg.h"
#include "DnQuestTabDlg.h"
#include "NpcSendPacket.h"
#include "DnNpcDlg.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "EtWorldEventArea.h"
#include "DnInCodeResource.h"
#include "DnMainMenuDlg.h"
#include "DnItemTask.h"
#include "DnWorldData.h"
#include "DnCommonTask.h"
#include "GameOptionSendPacket.h"
#include "DnUIString.h"
#include "DnMainMenuDlg.h"
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#include "DnLocalPlayerActor.h"
#include "DnMainDlg.h"
#ifdef PRE_FIX_71455_REPUTE_RENEW
#include "SyncTimer.h"
#endif // PRE_FIX_71455_REPUTE_RENEW
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
#endif

CDnQuestTask::CDnQuestTask() : CTaskListener(true)
{
	m_nLastUpdateTime = 0;
	m_pQuestDialog = NULL;
	m_pInvenDialog = NULL;
	m_bIsRefreshNpcQuestState = false;
	for( int i = 0; i < DNNotifier::RegisterCount::MainQuest; i++)
		m_NotifierData[ i ].eType = DNNotifier::Type::MainQuest;
	for( int i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++)
		m_NotifierData[ i ].eType = DNNotifier::Type::SubQuest;	

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	m_pReputationRepos = new CReputationSystemRepository;
	m_iReputeUpSoundIndex = -1;
#endif // PRE_ADD_NPC_REPUTATION_SYSTEM

	m_nTraceQuestID = 0;
	memset(&m_Quest, 0, sizeof(m_Quest));
}

CDnQuestTask::~CDnQuestTask()
{
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	SAFE_DELETE( m_pReputationRepos );
	SAFE_RELEASE_SPTR( m_hReputeUpSound );
#endif // PRE_ADD_NPC_REPUTATION_SYSTEM

#ifdef PRE_ADD_REMOTE_QUEST
	m_vecRemoteQuestAskList.clear();
#endif // PRE_ADD_REMOTE_QUEST
}

bool CDnQuestTask::Initialize()
{
	ZeroMemory(&m_Quest, sizeof(TQuestGroup));
	m_QuestPathFinder.Init();
	m_pQuestDialog = NULL;
	m_pInvenDialog = NULL;

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	if( false == m_NpcReaction.IsInitialized() )
	{
		m_NpcReaction.Initialize( m_pReputationRepos );
	}
	//else
	//{
	//	// TODO: Must be called whenever moving to town, dungeon, or other places.
	//	m_NpcReaction.Clear();
	//}
#endif // PRE_ADD_NPC_REPUTATION_SYSTEM

	return true;
}


void CDnQuestTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	ProcessRefreshQuest();
	if ( LocalTime - m_nLastUpdateTime > 100 )
	{
		UpdateQuestNotifierInfo();
		m_nLastUpdateTime = LocalTime; 
	}

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	m_NpcReaction.Process( LocalTime, fDelta );
#ifdef PRE_FIX_71455_REPUTE_RENEW
	RefreshRepUnionMembershipMark();
#endif // PRE_FIX_71455_REPUTE_RENEW
#endif // PRE_ADD_NPC_REPUTATION_SYSTEM
}

void CDnQuestTask::UpdateQuestNotifierInfo(bool bForceUpdate /* = false */)
{
	int nMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;

	const float fDummyPos = 123456.789f;
	for ( int i = 0 ; i < DNNotifier::RegisterCount::TotalQuest ; i++ )	
	{
		if( m_NotifierInfo[i].nQuestIndex < 0 ) continue;
		const TQuest* pQuest = GetQuestIndex(m_NotifierInfo[i].nQuestIndex);
		if ( !pQuest )
			continue;

		Journal* pJournal = g_DataManager.GetJournalData(m_NotifierInfo[i].nQuestIndex);
		if ( !pJournal )
			continue;

		JournalPage* pJournalPage = pJournal->FindJournalPage(pQuest->cQuestJournal);
		if ( !pJournalPage )
			continue;

		// Re-search the path if the journal or map has changed.
		if ( m_NotifierInfo[i].nJournalStep != (int)pQuest->cQuestJournal ||
			 m_NotifierInfo[i].nCurrentMapIndex != nMapIndex || bForceUpdate )
		{
			m_NotifierInfo[i].nJournalStep = (int)pQuest->cQuestJournal; 
			if( pJournalPage->nDestinationNpc == -1 ) {
				m_NotifierInfo[i].vTargetPos = pJournalPage->vDestinationPos;
			}
			else {
				m_NotifierInfo[i].vTargetPos = EtVector2( fDummyPos, fDummyPos);
				CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( pJournalPage->nDestnationMapIndex );
				int nCount = 0;
				if( pWorldData ) nCount = pWorldData->GetNpcCount();
				int j;
				for( j=0; j<nCount; j++ )
				{
					CDnWorldData::NpcData *pNpcData = pWorldData->GetNpcData( j );
					if( pNpcData->nNpcID == pJournalPage->nDestinationNpc ) {
						m_NotifierInfo[i].vTargetPos = pNpcData->vPos;
						break;
					}
				}
				if( j == nCount && nMapIndex == pJournalPage->nDestnationMapIndex ) {
					CDnNPCActor *pNpcActor = CDnNPCActor::FindNpcActorFromID( pJournalPage->nDestinationNpc );
					if( !pNpcActor ) continue;
					m_NotifierInfo[i].vTargetPos = EtVector2( pNpcActor->GetPosition()->x, pNpcActor->GetPosition()->z);
				}
			}
			m_NotifierInfo[ i ].nTargetMapIndex = pJournalPage->nDestnationMapIndex;
			m_NotifierInfo[ i ].nCurrentMapIndex = nMapIndex;
			m_QuestPathFinder.GetPathOneWay(m_NotifierInfo[ i ].nCurrentMapIndex, m_NotifierInfo[ i ].nTargetMapIndex, m_NotifierInfo[ i ].pathResult );

			if( m_pQuestDialog )
			{
				m_pQuestDialog->RefreshSummaryInfo();
			}
		}

		// If current position differs from destination
		if ( m_NotifierInfo[ i ].nCurrentMapIndex != m_NotifierInfo[ i ].nTargetMapIndex  )
		{
			if ( m_NotifierInfo[ i ].pathResult.size() > 0 )
			{
				// After finding out which gate to exit through
				int nGateIndex = m_NotifierInfo[ i ].pathResult[ 0 ].nGateIndex;
				DWORD dwCount = CDnWorld::GetInstance().GetGateCount();
				CDnWorld::GateStruct *pGateStruct( NULL );
				SOBB *pOBB( NULL );
				// When map link info is valid but gate info is invalid
				// Initialize target position with an invalid value for now.
				m_NotifierInfo[ i ].vTargetPos.x = m_NotifierInfo[ i ].vTargetPos.y = fDummyPos;
				if( nGateIndex == -1 ) {
					continue;
				}
				pGateStruct = CDnWorld::GetInstance().GetGateStruct( nGateIndex );

				if( !pGateStruct )
				{
					continue;
				}
				if( !(pGateStruct->pGateArea) )
				{
					continue;
				}
				pOBB = pGateStruct->pGateArea->GetOBB();

				if( !pOBB )
				{
					continue;
				}

				EtVector3 vGatePos = pOBB->Center;
				m_NotifierInfo[i].vTargetPos.x = vGatePos.x;
				m_NotifierInfo[i].vTargetPos.y = vGatePos.z;			
			}
		}
	}
}

void CDnQuestTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	bool bProcessDispatch = false;
	switch( nMainCmd ) {
		case SC_QUEST: OnRecvQuestMessage( nSubCmd, pData, nSize ); bProcessDispatch = true; break;
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
		case SC_REPUTATION: OnRecvReputationMessage( nSubCmd, pData, nSize ); break;
#endif //PRE_ADD_NPC_REPUTATION_SYSTEM
 	}
	if( bProcessDispatch ) {
		WriteLog( 1, ", Info, CDnQuestTask::OnDispatchMessage End : nMainCmd(%d), nSubCmd(%d), nSize(%d)\n", nMainCmd, nSubCmd, nSize );
	}
}

void CDnQuestTask::OnRecvQuestMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eQuest::SC_QUESTINFO: OnRecvQuestInfo( (SCQuestInfo *)pData ); break;
		case eQuest::SC_QUESTCOMPLETEINFO: OnRecvQuestCompleteInfo( (SCQuestCompleteInfo *)pData ); break;
		case eQuest::SC_REFRESH_QUEST:	OnRecvRefreshQuest( (SCRefreshQuest *)pData ); break;
		case eQuest::SC_COMPLETE_QUEST: OnRecvCompleteQuest( (SCCompleteQuest *)pData ); break;
		case eQuest::SC_MARKING_COMPLETE_QUEST: OnRecvMarkingCompleteQuest( (SCMarkingCompleteQuest *)pData ); break;
		case eQuest::SC_REMOVE_QUEST:	OnRecvRemoveQuest( (SCRemoveQuest *)pData );	break;
		case eQuest::SC_QUEST_REWARD: OnRecvQuestReward( (SCQuestReward*)pData ); break;
		case eQuest::SC_QUESTRESULT: OnRecvQuestResult((SCQuestResult*)pData); break;
		case eQuest::SC_ASSIGN_PERIODQUEST: OnRecvPeriodQuest((SCAssignPeriodQuest*)pData); break;
		case eQuest::SC_NOTICE_PERIODQUEST: OnRecvNoticePeriodQuest((SCPeriodQuestNotice*)pData); break;
		case eQuest::SC_SCORE_PERIODQUEST:  OnRecvScorePeriodQuest((SCScorePeriodQuest *)pData); break;
#ifdef PRE_ADD_REMOTE_QUEST
		case eQuest::SC_GAIN_REMOTE_QUEST:	OnRecvGainRemoteQuest( (SCGainRemoteQuest*)pData ); break;
		case eQuest::SC_COMPLETE_REMOTE_QEUST:	OnRecvCompleteRemoteQuest( (SCCompleteRemoteQuest*)pData ); break;
		case eQuest::SC_REMOVE_REMOTE_QEUST:	OnRecvRemoveRemoteQuest( (SCRemoveRemoteQuest*)pData ); break;
#endif // PRE_ADD_REMOTE_QUEST
	}
}

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
void CDnQuestTask::OnRecvReputationMessage( int nSubCmd, char* pData, int nSize )
{
	switch( nSubCmd )
	{
		case eReputation::SC_REPUTATIONLIST: OnRecvReputationList( (SCReputationList*)pData ); break;
		case eReputation::SC_MODREPUTATION: OnRecvReputationModify( (SCModReputation*)pData ); break;
		case eReputation::SC_OPENGIVENPCPRESENT: OnRecvReputationOpenGiveNpcPresent( (SCOpenGiveNpcPresent*)pData ); break;
		case eReputation::SC_REQUESTSENDSELECTEDPRESENT: OnRecvRequestSendNpcPresentSelect( (SCRequestSendSelectedPresent*)pData ); break;
		case eReputation::SC_SHOWNPCEFFECT: OnRecvShowNpcEffect( (SCShowNpcEffect*)pData ); break;
		case eReputation::SC_UNIONPOINT:				OnRecvUnionPoint((SCUnionPoint*)pData); break;
		case eReputation::SC_UNIONMEMBERSHIPLIST:		OnRecvUnionMembershipList((SCUnionMembershipList*)pData); break;
		case eReputation::SC_BUYUNIONMEMBERSHIPRESULT:	OnRecvUnionBuyMembership((SCBuyUnionMembershipResult*)pData); break;
	}
}
#endif // PRE_ADD_NPC_REPUTATION_SYSTEM


void CDnQuestTask::OnUpdateNotifier( bool bSaveToServer )
{
	if( bSaveToServer ) {
		for( int i = 0 ; i < DNNotifier::RegisterCount::TotalQuest; i++) {
			ASSERT( m_NotifierData[i].iIndex != 0 );
		}
		SendQuestNotifier( m_NotifierData );
	}
	for ( int i = 0; i < DNNotifier::RegisterCount::TotalQuest; i++) {
		m_NotifierInfo[i].nQuestIndex = m_NotifierData[ i ].iIndex;
	}

	m_pQuestDialog->RefreshSummaryInfo();
	UpdateQuestNotifierInfo(true);
}

bool CDnQuestTask::IsRegisterQuestNotifier( DNNotifier::Type::eType QuestType, int nQuestIndex )
{
	if( QuestType == DNNotifier::Type::MainQuest ) {
		if( m_NotifierData[ 0 ].IsEqual( DNNotifier::Type::MainQuest, nQuestIndex ) ) {
			return true;
		}
	}
	else {
		// Check if the same item is already registered
		for( int i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++) {
			if( m_NotifierData[ i ].IsEqual( DNNotifier::Type::SubQuest, nQuestIndex ) ) {	
				return true;
			}
		}
	}	
	return false;
}

bool CDnQuestTask::IsSubQuestNotifierFull()
{
	for( int i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++) {
		if( m_NotifierData[ i ].iIndex < 0 ) {	
			return false;
		}
	}
	return true;
}

int CDnQuestTask::GetRegisteredSubQuestCount()
{
	int nCount = 0;
	for( int i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++) {
		if( m_NotifierData[ i ].iIndex >= 0 ) {	
			ASSERT( m_NotifierData[ i ].eType == DNNotifier::Type::SubQuest );
			nCount++;
		}
	}
	return nCount;
}

bool CDnQuestTask::RemoveQuestNotifier( DNNotifier::Type::eType QuestType, int nQuestIndex )
{
	ASSERT( nQuestIndex != -1 );
	ASSERT( QuestType == DNNotifier::Type::MainQuest || QuestType == DNNotifier::Type::SubQuest );

	if( QuestType == DNNotifier::Type::MainQuest ) 
	{
		m_NotifierData[0].Clear();
		return true;
	}
	else 
	{
		// Check if the same item is already registered
		for( int i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++) 
		{
			if( m_NotifierData[ i ].IsEqual( DNNotifier::Type::SubQuest, nQuestIndex ) ) 
			{
				m_NotifierData[i].Clear();
				return true;
			}
		}
	}
	return false;
}

// Add newly created quests or quests checked by the user.
bool CDnQuestTask::AddQuestNotifier( DNNotifier::Type::eType QuestType, int nQuestIndex )
{
	ASSERT( nQuestIndex != -1 );
	ASSERT( QuestType == DNNotifier::Type::MainQuest || QuestType == DNNotifier::Type::SubQuest );

	if( QuestType == DNNotifier::Type::MainQuest ) {
		if( !m_NotifierData[ 0 ].IsEqual( DNNotifier::Type::MainQuest, nQuestIndex ) ) {
			m_NotifierData[ 0 ].Register( DNNotifier::Type::MainQuest, nQuestIndex );
			return true;
		}
	}
	else {
		// Check if the same item is already registered
		for( int i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++) {
			if( m_NotifierData[ i ].IsEqual( DNNotifier::Type::SubQuest, nQuestIndex ) ) {
				return false;
			}
		}
		// Check if there is an empty slot to insert into
		for( int i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++) {
			ASSERT( m_NotifierData[ i ].iIndex != 0 );
			if( m_NotifierData[ i ].iIndex == -1  ) {
				ASSERT( nQuestIndex != 0);
				m_NotifierData[ i ].Register( DNNotifier::Type::SubQuest, nQuestIndex );
				return true;
			}
		}
	}
	return false;
}

void CDnQuestTask::OnRecvQuestInfo( SCQuestInfo *pPacket )
{
	// Receive quest notifier info from server.
	for( int j = 0; j < DNNotifier::RegisterCount::TotalQuest; j++) {
		ASSERT( pPacket->NotifierData[ j ].iIndex != 0 );
		
		m_NotifierData[ j ] = pPacket->NotifierData[ j ];
		m_NotifierInfo[ j ].nQuestIndex = m_NotifierData[ j ].iIndex;
	}

	if( CDnMissionTask::IsActive() ) {
#ifdef PRE_MOD_MISSION_HELPER
		for( int j = DNNotifier::RegisterCount::TotalQuest; j < DNNotifier::RegisterCount::TotalQuest + DNNotifier::RegisterCount::TotalMission; j++ )
			CDnMissionTask::GetInstance().RegisterNotifier( pPacket->NotifierData[ j ].eType, pPacket->NotifierData[ j ].iIndex, true );
#else
		CDnMissionTask::GetInstance().RegisterNotifier( CDnMissionTask::Daily, pPacket->NotifierData[ DNNotifier::RegisterCount::TotalQuest ].iIndex, true );
		CDnMissionTask::GetInstance().RegisterNotifier( CDnMissionTask::Weekly, pPacket->NotifierData[ DNNotifier::RegisterCount::TotalQuest +DNNotifier::RegisterCount:: DailyMission ].iIndex, true );
#endif
	}

	int nQuestCount = 0;
	for ( nQuestCount = 0; nQuestCount < pPacket->cCount; nQuestCount++){
		if (pPacket->Quest[nQuestCount].nQuestID <= 0) continue;
		TQuest quest = pPacket->Quest[ nQuestCount ];
		m_Quest.Quest[nQuestCount] = quest;
#ifdef PRE_ADD_REMOTE_QUEST
		if( g_DataManager.GetQuestType( pPacket->Quest[nQuestCount].nQuestID ) == QuestType_RemoteQuest
			&& pPacket->Quest[nQuestCount].cQuestState == QuestState_Recompense )
		{
			m_Quest.Quest[nQuestCount].cQuestState = QuestState_Recompense;
			AddRemoteQuestAskList( pPacket->Quest[nQuestCount].nQuestID, CDnQuestTree::REMOTEQUEST_COMPLETE );
			CDnMainDlg* pMainBarDlg = GetInterface().GetMainBarDialog();
			if( pMainBarDlg ) 
			{
				pMainBarDlg->ShowQuestNotify( true, CDnQuestTree::REMOTEQUEST_COMPLETE );
				pMainBarDlg->BlinkMenuButton( CDnMainMenuDlg::QUEST_DIALOG );
			}
		}
#endif // PRE_ADD_REMOTE_QUEST
	}

	for( ; nQuestCount < MAX_PLAY_QUEST; nQuestCount++) {
		ZeroMemory(&(m_Quest.Quest[nQuestCount]), sizeof(TQuest));
	}

	// Do not execute the logic below if the dialog is not created - by robust
	if (!m_pQuestDialog)
		return;

#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
	LoadQuestTalk();
#else
	m_pQuestDialog->RefreshQuest();	
#endif	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)

	OnUpdateNotifier( false );

	if( GetInterface().IsOpenBlind() )
	{
		GetInterface().CloseNpcDialog();
		CDnLocalPlayerActor::LockInput(false);
		CDnCommonTask *pCommonTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
		if( pCommonTask ) pCommonTask->EndNpcTalk();
	}
}

void CDnQuestTask::OnRecvQuestCompleteInfo( SCQuestCompleteInfo *pPacket )
{
	memcpy(m_Quest.CompleteQuest, pPacket->CompleteQuest, sizeof(m_Quest.CompleteQuest));
	
	if( m_pQuestDialog ) {
		m_pQuestDialog->RefreshQuest();
		m_pQuestDialog->RefreshSummaryInfo();
	}
	UpdateQuestNotifierInfo(true);
	SetRefreshNpcQuestState(true);
}

bool CDnQuestTask::UpdateQuestInfo(int nQuestSlot, TQuest quest)
{
	if ( nQuestSlot < 0 || nQuestSlot >= MAX_PLAY_QUEST )
	{
		_ASSERT( false );
		return false;
	}
	m_Quest.Quest[nQuestSlot] = quest;
	return true;
}

bool CDnQuestTask::ClearQuestInfo(int nQuestSlot)
{
	if ( nQuestSlot < 0 || nQuestSlot >= MAX_PLAY_QUEST )
	{
		_ASSERT( false );
		return false;
	}
	
	ZeroMemory(&(m_Quest.Quest[nQuestSlot]), sizeof(TQuest));

	return true;
}

void CDnQuestTask::OnRecvRefreshQuest( SCRefreshQuest *pPacket )
{
	for( int i=0; i<(int)m_VecRefreshQuestList.size(); i++ ) {
		if( m_VecRefreshQuestList[i].nQuestID == pPacket->nQuestID && m_VecRefreshQuestList[i].nRefreshType == pPacket->nRefreshType ) {
			m_VecRefreshQuestList.erase( m_VecRefreshQuestList.begin() + i );
			i--;
			continue;
		}
	}
	m_VecRefreshQuestList.push_back( *pPacket );
}

void CDnQuestTask::ProcessRefreshQuest()
{
	for( DWORD i=0; i<m_VecRefreshQuestList.size(); i++ ) {
		ProcessRefreshQuest( &m_VecRefreshQuestList[i] );
	}
	m_VecRefreshQuestList.clear();
}

void CDnQuestTask::ProcessRefreshQuest( SCRefreshQuest *pPacket )
{
	int nQuestSlot = -1;
	if( pPacket->nRefreshType == RefreshType::ADD_QUEST ) {
		nQuestSlot = FindEmptyIndex();
	}
	else {
		nQuestSlot = FindQuest( pPacket->nQuestID );		
	}
	if( nQuestSlot < 0 ) {
		ASSERT( false );
		return;
	}

	PROFILE_TICK_TEST( UpdateQuestInfo( nQuestSlot, pPacket->Quest ) );

	if( !m_pQuestDialog )
	{
		CDebugSet::ToLogFile( "CDnQuestTask::OnRecvRefreshQuest, m_pQuestDialog is NULL!" );
		return;
	}

	switch( pPacket->nRefreshType )
	{
	case RefreshType::ADD_QUEST:
		{
			m_pQuestDialog->AddQuest(&m_Quest.Quest[ nQuestSlot ]);

			Journal* pJournal = g_DataManager.GetJournalData(m_Quest.Quest[ nQuestSlot ].nQuestID);
			if( pJournal )
			{
				CDnChatTabDlg *pChatDlg = GetInterface().GetChatDialog();
				if( pChatDlg ) pChatDlg->AddChatQuestReceipt( pJournal->wszQuestTitle.c_str() );
			}
#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
			else
				g_DataManager.LoadQuestData( m_Quest.Quest[ nQuestSlot ].nQuestID );
#endif	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)

			m_pQuestDialog->RefreshSummaryInfo();

			// Spawn particles too
			DnActorHandle hActor = CDnActor::s_hLocalActor;
			if( hActor ) 
			{
				DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
				if( hHandle ) {
					hHandle->SetPosition( *hActor->GetPosition() );
					hHandle->SetActionQueue( "QuestAccpet" );
				}
			}

			switch( g_DataManager.GetQuestType(pPacket->Quest.nQuestID) ) {
			case QuestType_MainQuest:
				if( AddQuestNotifier( DNNotifier::Type::MainQuest, pPacket->Quest.nQuestID ) ) {
					OnUpdateNotifier( true );
				}				
				break;
			case QuestType_SubQuest:
				if( AddQuestNotifier( DNNotifier::Type::SubQuest, pPacket->Quest.nQuestID ) ) {
					OnUpdateNotifier( true );
				}
				break;
			case QuestType_PeriodQuest:
				if( AddQuestNotifier( DNNotifier::Type::SubQuest, pPacket->Quest.nQuestID ) ) {
					OnUpdateNotifier( true );
				}
				break;
#ifdef PRE_ADD_REMOTE_QUEST
			case QuestType_RemoteQuest:
				if( AddQuestNotifier( DNNotifier::Type::SubQuest, pPacket->Quest.nQuestID ) ) {
					OnUpdateNotifier( true );
				}
				break;
#endif //PRE_ADD_REMOTE_QUEST
			}
		}
		break;
#ifdef PRE_ADD_REMOTE_QUEST
	case RefreshType::REFRESH_STEPANDJOURNAL:
#endif // PRE_ADD_REMOTE_QUEST
	case RefreshType::REFRESH_JOURNAL:
		{
			m_pQuestDialog->AddJournal( &m_Quest.Quest[ nQuestSlot ] );
			m_pQuestDialog->RefreshSummaryInfo();

			UINT nQuestIdx = m_Quest.Quest[ nQuestSlot ].nQuestID;
			char cQuestJournal = m_Quest.Quest[ nQuestSlot ].cQuestJournal;
			Journal* pJournal = g_DataManager.GetJournalData(nQuestIdx);
			if( pJournal )
			{
				JournalPage* pJournalPage = pJournal->FindJournalPage( cQuestJournal );
				if ( pJournal && pJournalPage )
				{
					wchar_t szTemp[1024] = {0};
#if defined(PRE_ADD_ENGLISH_STRING)
					swprintf_s( szTemp, _countof(szTemp), L"%s: %s", pJournal->wszQuestTitle.c_str(), pJournalPage->szTodoMsg.c_str() );
#else
					swprintf_s( szTemp, _countof(szTemp), L"[%s] : %s, %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 647), pJournal->wszQuestTitle.c_str(), pJournalPage->szTodoMsg.c_str() );
#endif
					
					CDnChatTabDlg *pChatDlg = GetInterface().GetChatDialog();
					if( pChatDlg ) pChatDlg->AddChatQuestMsg( szTemp );
				}

				if ( GetInterface().GetBlindDialog()->GetBlindMode() == CDnBlindDlg::modeOpened ) 
				{
					CDnQuestTask::QuestNotifierInfo* pInfo = GetQuestTask().GetQuestNotifierInfoByQuestID( nQuestIdx );
					if ( pInfo )
					{
						LOCAL_TIME time = 1000 * 12;
						GetQuestTask().UpdateQuestNotifierInfo(true);
						EtVector3 vPos;
						vPos.x = pInfo->vTargetPos.x;
						vPos.y = 0.0f;
						vPos.z = pInfo->vTargetPos.y;

						if(CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeDungeon)
						{
							if(pInfo->nCurrentMapIndex == pInfo->nTargetMapIndex) 
								GetInterface().OpenNotifyArrowDialog( vPos, time, false ); 
						}
						else
						{
							GetInterface().OpenNotifyArrowDialog( vPos, time, false );
						}
					}					
				}
			}
		}
		break;
	case RefreshType::REFRESH_COUNTING:
		PROFILE_TICK_TEST( m_pQuestDialog->RefreshJournal( &m_Quest.Quest[ nQuestSlot ] ) );
		break;
	case RefreshType::REFRESH_COMPLETECOUNTING:
		break;
	case RefreshType::REFRESH_ALLCOMPLETECOUNTING:
		break;
	default:
		break;
	}

	m_pQuestDialog->RefreshQuest();
	PROFILE_TICK_TEST( m_pQuestDialog->RefreshSummaryInfo() );
	PROFILE_TICK_TEST( UpdateQuestNotifierInfo( true ) );
}

void CDnQuestTask::OnRecvMarkingCompleteQuest(SCMarkingCompleteQuest* pPacket )
{
	SetCompleteQuestFlag(pPacket->nQuestID, true);	
}

void CDnQuestTask::OnRecvPeriodQuest(SCAssignPeriodQuest* pPacket )
{
	if (true == pPacket->bFlag)
	{
		int nQuestSlot = FindQuest( pPacket->nQuestID );
		if ( nQuestSlot >= 0 && nQuestSlot < MAX_PLAY_QUEST && m_Quest.Quest[ nQuestSlot ].nQuestID == pPacket->nQuestID )
		{
			if(m_pQuestDialog)
				m_pQuestDialog->DeleteQuest( &(m_Quest.Quest[ nQuestSlot ]) );
			ClearQuestInfo( nQuestSlot );
		}

		if(m_pQuestDialog)
			m_pQuestDialog->RefreshSummaryInfo();

		// When bFlag is true, the event quest is ending due to time expiration.
		// If the quest currently in progress through dialogue matches the event quest to be deleted, stop the dialogue.


		if( CDnInterface::IsActive() && GetInterface().IsOpenBlind()  ) // OpenBlind = currently in NPC Talk = close the window
		{
			GetInterface().CloseNpcDialog();
			CDnLocalPlayerActor::LockInput(false);
			CDnCommonTask *pCommonTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
			if( pCommonTask ) pCommonTask->EndNpcTalk();
		}
		
	}

	SetCompleteQuestFlag(pPacket->nQuestID, pPacket->bFlag);
}

void CDnQuestTask::OnRecvNoticePeriodQuest(SCPeriodQuestNotice* pPacket)
{
	int nItemID = 0;
	int nNoticeCount = 0;
	int nUIString = 0;
	CHAR wszMsg[256] = {0,};

	nNoticeCount = pPacket->nNoticeCount;
	nItemID = pPacket->nItemID;

	DNTableFileFormat* pSoxGlobar = GetDNTable( CDnTableDB::TGLOBALEVENTQUEST );
	if( !pSoxGlobar || !pSoxGlobar->IsExistItem(nItemID)) return;
	
	sprintf_s( wszMsg, "_NoticeText%d", nNoticeCount );

	nUIString = pSoxGlobar->GetFieldFromLablePtr(nItemID , wszMsg )->GetInteger();

	GetInterface().ShowCaptionDialog( CDnInterface::typeCaption1, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nUIString ), textcolor::HOTPINK , 7.5f);
}

void CDnQuestTask::OnRecvScorePeriodQuest(SCScorePeriodQuest* pPacket)
{
	GetInterface().GetMainMenuDialog()->OpenGlobalEventDlg(pPacket);
}

void CDnQuestTask::OnRecvCompleteQuest( SCCompleteQuest *pPacket )
{
	if( !m_pQuestDialog )
	{
		CDebugSet::ToLogFile( "CDnQuestTask::OnRecvCompleteQuest, m_pQuestDialog is NULL!" );
		return;
	}

	int nQuestSlot = FindQuest( pPacket->nQuestID );
	if( nQuestSlot < 0 ) {
		ASSERT( false );
		return;
	}

	m_pQuestDialog->CompleteQuest( &(m_Quest.Quest[ nQuestSlot ]) );

	// If it needs to be removed from the current play list
	// If not marked
	if( pPacket->bDeletePlayList )
	{
		if( nQuestSlot < MAX_PLAY_QUEST && m_Quest.Quest[ nQuestSlot ].nQuestID == pPacket->nQuestID )
		{
			if( IsClearQuest( pPacket->nQuestID ) == false )
			{
				m_pQuestDialog->DeleteQuest( &(m_Quest.Quest[ nQuestSlot ]) );
#ifdef PRE_ADD_REMOTE_QUEST
				if( g_DataManager.GetQuestType( pPacket->nQuestID ) == QuestType_RemoteQuest )
					DeleteRemoteQuestAskList( pPacket->nQuestID );
#endif // PRE_ADD_REMOTE_QUEST
			}
			
			ClearQuestInfo( nQuestSlot );
		}
	}
	
	Journal* pJournal = g_DataManager.GetJournalData( pPacket->nQuestID );
	if( pJournal )
	{
		GetInterface().GetChatDialog()->AddChatQuestCompletion( pJournal->wszQuestTitle.c_str() ); 
	}

	m_pQuestDialog->RefreshSummaryInfo();

	// Spawn particles too
	DnActorHandle hActor = CDnActor::s_hLocalActor;
	if( hActor ) 
	{
		DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
		if( hHandle ) {
			hHandle->SetPosition( *hActor->GetPosition() );
			hHandle->SetActionQueue( "QuestComplete" );
		}

		// Refresh dungeon entry notifier
		CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
		if( pTask ) {
			pTask->CheckDungeonOpenNotice( hActor->GetLevel(), pPacket->nQuestID );
		}
	}
}

void CDnQuestTask::OnRecvRemoveQuest( SCRemoveQuest *pPacket )
{
	if( !m_pQuestDialog )
	{
		CDebugSet::ToLogFile( "CDnQuestTask::OnRecvRemoveQuest, m_pQuestDialog is NULL!" );
		return;
	}

	if (pPacket->nRetCode == ERROR_NONE)
	{

		int nQuestSlot = FindQuest( pPacket->nQuestID );
		if ( nQuestSlot >= 0 && nQuestSlot < MAX_PLAY_QUEST && m_Quest.Quest[ nQuestSlot ].nQuestID == pPacket->nQuestID )
		{
			m_pQuestDialog->DeleteQuest( &(m_Quest.Quest[ nQuestSlot ]) );
			ClearQuestInfo( nQuestSlot );

			m_pQuestDialog->RefreshSummaryInfo();
		}
	}
	else
		GetInterface().ServerMessageBox(pPacket->nRetCode);
}

bool CDnQuestTask::IsClearQuest(int nQuestIndex)
{
	if ( nQuestIndex < 0 || nQuestIndex > MAX_QUEST_INDEX )
		return false;

	int nIndex = nQuestIndex / 8;
	int nMaskType = nQuestIndex % 8;
	char Result = 0;

	if( COMPLETEQUEST_BITSIZE <= nIndex )
		return false;

	switch(nMaskType)
	{
	case 1: Result = m_Quest.CompleteQuest[nIndex] & 0x80; break;
	case 2: Result = m_Quest.CompleteQuest[nIndex] & 0x40; break;
	case 3: Result = m_Quest.CompleteQuest[nIndex] & 0x20; break;
	case 4: Result = m_Quest.CompleteQuest[nIndex] & 0x10; break;
	case 5: Result = m_Quest.CompleteQuest[nIndex] & 0x08; break;
	case 6: Result = m_Quest.CompleteQuest[nIndex] & 0x04; break;
	case 7: Result = m_Quest.CompleteQuest[nIndex] & 0x02; break;
	case 0: Result = m_Quest.CompleteQuest[nIndex] & 0x01; break;		
	}

	if (Result == 0x00) return false;

	return true;
}

void CDnQuestTask::SetCompleteQuestFlag(int nQuestIndex, bool bFlag)
{
	int nIndex = nQuestIndex / 8;
	int nMaskType = nQuestIndex % 8;

	if(nIndex >= COMPLETEQUEST_BITSIZE) return; 	

	if ( bFlag )
	{
		switch(nMaskType)
		{
		case 1: m_Quest.CompleteQuest[nIndex] |= 0x80; break;
		case 2: m_Quest.CompleteQuest[nIndex] |= 0x40; break;
		case 3: m_Quest.CompleteQuest[nIndex] |= 0x20; break;
		case 4: m_Quest.CompleteQuest[nIndex] |= 0x10; break;
		case 5: m_Quest.CompleteQuest[nIndex] |= 0x08; break;
		case 6: m_Quest.CompleteQuest[nIndex] |= 0x04; break;
		case 7: m_Quest.CompleteQuest[nIndex] |= 0x02; break;
		case 0: m_Quest.CompleteQuest[nIndex] |= 0x01; break;		
		}
	}
	else
	{
		switch(nMaskType)
		{
		case 1: m_Quest.CompleteQuest[nIndex] &= ~0x80; break;
		case 2: m_Quest.CompleteQuest[nIndex] &= ~0x40; break;
		case 3: m_Quest.CompleteQuest[nIndex] &= ~0x20; break;
		case 4: m_Quest.CompleteQuest[nIndex] &= ~0x10; break;
		case 5: m_Quest.CompleteQuest[nIndex] &= ~0x08; break;
		case 6: m_Quest.CompleteQuest[nIndex] &= ~0x04; break;
		case 7: m_Quest.CompleteQuest[nIndex] &= ~0x02; break;
		case 0: m_Quest.CompleteQuest[nIndex] &= ~0x01; break;		
		}
	}
}

int CDnQuestTask::FindPlayingQuest(int nQuestID)
{
	for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
#ifdef PRE_ADD_REMOTE_QUEST
		if( m_Quest.Quest[i].nQuestID == nQuestID )
		{
			if( m_Quest.Quest[i].cQuestState == QuestState_Playing || 
				( g_DataManager.GetQuestType( m_Quest.Quest[i].nQuestID ) == QuestType_RemoteQuest && m_Quest.Quest[i].cQuestState == QuestState_Recompense ) )
				return i;
		}
#else // PRE_ADD_REMOTE_QUEST
		if ( m_Quest.Quest[i].nQuestID == nQuestID && m_Quest.Quest[i].cQuestState == QuestState_Playing ) 
			return i;
#endif // PRE_ADD_REMOTE_QUEST
	}

	return -1;
}

int CDnQuestTask::FindQuest(int nQuestID)
{
	for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if ( m_Quest.Quest[i].nQuestID == nQuestID ) 
			return i;
	}

	return -1;
}

int CDnQuestTask::FindEmptyIndex()
{
	for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if ( m_Quest.Quest[i].nQuestID <= 0 ) 
			return i;
	}
	return -1;
}

bool CDnQuestTask::GetCurJournalCountingInfo(IN int nQuestIndex, OUT char& cCurJournalIndex, OUT std::vector<TCount>& Counting)
{
	int nSlot = FindPlayingQuest(nQuestIndex);
	if ( nSlot < 0 )
		return false;

	TCount* pCount = (TCount*)m_Quest.Quest[nSlot].Extra;


	cCurJournalIndex = m_Quest.Quest[nSlot].cQuestJournal;
	Counting.clear();
#if !defined(SW_MODIFY_QUEST_SYSTEM_20091015_b4nfter)
	for ( int i = 0 ; i < QUEST_EXTRACOUNT_MAX ; i++ )
#else	// #if !defined(SW_MODIFY_QUEST_SYSTEM_20091015_b4nfter)
	for ( int i = 0 ; i < COUNT_MAX ; i++ )
#endif	// #if !defined(SW_MODIFY_QUEST_SYSTEM_20091015_b4nfter)
	{
		if ( pCount[i].cType != CountingType_None )
			Counting.push_back( pCount[i] );
	}

	return true;
}

void CDnQuestTask::OnRecvQuestReward( SCQuestReward *pPacket )
{
	LogWnd::Log(1, L"Open quest reward window :%d" , pPacket->nRewardTableIndex );

	TQuestRecompense questRecompense;
	bool bResult = g_DataManager.GetQuestRecompense( pPacket->nRewardTableIndex, questRecompense );
	if(!bResult)
	{
		WriteLog( 1, ", Error, can't find recompense table : %d ", pPacket->nRewardTableIndex );
		return;
	}

	CDnNpcDlg *pNpcDlg = GetInterface().GetNpcDialog();
	if(pNpcDlg) pNpcDlg->SetRecompense( questRecompense, pPacket->bActivate );
}

void CDnQuestTask::OnRecvQuestResult(SCQuestResult * pPacket)
{
	GetInterface().ServerMessageBox(pPacket->nRetCode);
}

const TQuest* CDnQuestTask::GetQuest( int nIndex )	
{ 
	ASSERT((nIndex<MAX_PLAY_QUEST)&&"GetQuest");
	return &(m_Quest.Quest[nIndex]);
}

const TQuest* CDnQuestTask::GetQuestIndex( int nQuestIndex )
{
	for( int i=0; i<MAX_PLAY_QUEST; i++ )
	{
		if( m_Quest.Quest[i].nQuestID == nQuestIndex )
		{
			return &m_Quest.Quest[i];
		}
	}

	return NULL;
}


CDnQuestTask::QuestNotifierInfo* CDnQuestTask::GetQuestNotifierInfoByQuestID(int nQuestID)
{
	for ( int i = 0 ; i < DNNotifier::RegisterCount::TotalQuest ; i++ )
	{
		if (  m_NotifierInfo[i].nQuestIndex == nQuestID )
			return &(m_NotifierInfo[i]);
	}
	return NULL;
}

CDnQuestTask::QuestNotifierInfo * CDnQuestTask::GetQuestNotifierInfoByQuestIDForce(int nQuestID)
{
	m_TraceNotifierInfo.nQuestIndex = nQuestID;

	int nMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;

	const float fDummyPos = 123456.789f;
	const TQuest* pQuest = GetQuestIndex( nQuestID );
	if ( !pQuest )
		return NULL;

	Journal* pJournal = g_DataManager.GetJournalData( nQuestID );
	if ( !pJournal )
		return NULL;

	JournalPage* pJournalPage = pJournal->FindJournalPage( pQuest->cQuestJournal );
	if ( !pJournalPage )
		return NULL;

	m_TraceNotifierInfo.nJournalStep = (int)pQuest->cQuestJournal; 
	if( pJournalPage->nDestinationNpc == -1 ) {
		m_TraceNotifierInfo.vTargetPos = pJournalPage->vDestinationPos;
	}
	else {
		m_TraceNotifierInfo.vTargetPos = EtVector2( fDummyPos, fDummyPos);
		CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( pJournalPage->nDestnationMapIndex );
		int nCount = 0;
		if( pWorldData ) nCount = pWorldData->GetNpcCount();
		int j;
		for( j=0; j<nCount; j++ )
		{
			CDnWorldData::NpcData *pNpcData = pWorldData->GetNpcData( j );
			if( pNpcData->nNpcID == pJournalPage->nDestinationNpc ) {
				m_TraceNotifierInfo.vTargetPos = pNpcData->vPos;
				break;
			}
		}
		if( j == nCount && nMapIndex == pJournalPage->nDestnationMapIndex ) {
			CDnNPCActor *pNpcActor = CDnNPCActor::FindNpcActorFromID( pJournalPage->nDestinationNpc );
			if( !pNpcActor )
				return NULL;
			m_TraceNotifierInfo.vTargetPos = EtVector2( pNpcActor->GetPosition()->x, pNpcActor->GetPosition()->z);
		}
	}
	m_TraceNotifierInfo.nTargetMapIndex = pJournalPage->nDestnationMapIndex;
	m_TraceNotifierInfo.nCurrentMapIndex = nMapIndex;
	m_QuestPathFinder.GetPathOneWay(m_TraceNotifierInfo.nCurrentMapIndex, 
		m_TraceNotifierInfo.nTargetMapIndex,
		m_TraceNotifierInfo.pathResult );

	// If current position differs from destination
	if ( m_TraceNotifierInfo.nCurrentMapIndex != m_TraceNotifierInfo.nTargetMapIndex  )
	{
		if ( m_TraceNotifierInfo.pathResult.size() > 0 )
		{
			// After finding out which gate to exit through
			int nGateIndex = m_TraceNotifierInfo.pathResult[ 0 ].nGateIndex;
			DWORD dwCount = CDnWorld::GetInstance().GetGateCount();
			CDnWorld::GateStruct *pGateStruct( NULL );
			SOBB *pOBB( NULL );
			// When map link info is valid but gate info is invalid
			// Initialize target position with an invalid value for now.
			m_TraceNotifierInfo.vTargetPos.x = m_TraceNotifierInfo.vTargetPos.y = fDummyPos;
			if( nGateIndex == -1 ) {
				return NULL;
			}
			pGateStruct = CDnWorld::GetInstance().GetGateStruct( nGateIndex );

			if( !pGateStruct )
			{
				return NULL;
			}
			if( !(pGateStruct->pGateArea) )
			{
				return NULL;
			}
			pOBB = pGateStruct->pGateArea->GetOBB();

			if( !pOBB )
			{
				return NULL;
			}

			EtVector3 vGatePos = pOBB->Center;
			m_TraceNotifierInfo.vTargetPos.x = vGatePos.x;
			m_TraceNotifierInfo.vTargetPos.y = vGatePos.z;			
		}
	}

	return &m_TraceNotifierInfo;
}

Journal *CDnQuestTask::GetNotifierJournal( int nIndex )
{
	if ( nIndex < 0 || nIndex >= DNNotifier::RegisterCount::TotalQuest )
	{
		return NULL;
	}

	return g_DataManager.GetJournalData(m_NotifierInfo[ nIndex ].nQuestIndex);
}

JournalPage *CDnQuestTask::GetNotifierJournalPage( int nIndex )
{
	if ( nIndex < 0 || nIndex >= DNNotifier::RegisterCount::TotalQuest )
	{
		return NULL;
	}

	const TQuest* pQuest = GetQuestIndex(m_NotifierInfo[ nIndex ].nQuestIndex);
	if ( !pQuest )
		return NULL;

	Journal *pJournal = g_DataManager.GetJournalData( m_NotifierInfo[ nIndex ].nQuestIndex );

	if ( !pJournal )
		return NULL;

	return pJournal->FindJournalPage(pQuest->cQuestJournal);
}

void CDnQuestTask::CancelQuest( int nQuestIndex )
{
#ifdef PRE_ADD_REMOTE_QUEST
	if( g_DataManager.GetQuestType( nQuestIndex ) == QuestType_RemoteQuest )
	{
		if( IsExistRemoteQuestAskList( nQuestIndex, CDnQuestTree::REMOTEQUEST_ASK ) )
			CancelRemoteQuest( nQuestIndex );
		else
			SendCancelQuest( nQuestIndex );

		return;
	}
#endif // PRE_ADD_REMOTE_QUEST

	if( FindPlayingQuest( nQuestIndex ) < 0 )
	{
		return;
	}

	if( g_DataManager.GetQuestType( nQuestIndex ) == QuestType_MainQuest && IsClearQuest( nQuestIndex ) == true )
		return; // For main quests, cleared status is also shown, and an already cleared quest cannot be canceled
	            // Already-completed quests are filtered by FindPlayingQuest above, but add an extra exception check.

	SendCancelQuest( nQuestIndex );
}

void CDnQuestTask::GetCompleteMainQuest( std::vector<Journal*> &vecCompleteMainQuest )
{
	vecCompleteMainQuest.reserve(MAX_QUEST_INDEX);

	std::multimap<int, Journal*> QSort;

	for ( int i = 0 ; i <= MAX_QUEST_INDEX ; i++  )
	{
		if ( IsClearQuest(i) == true )
		{
#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
			QuestInfo * pQuest = g_DataManager.FindQuestInfo(i);

			if( pQuest && QuestType_MainQuest == pQuest->cQuestType )
			{
				g_DataManager.LoadQuestData( i );

				Journal * pJournal = g_DataManager.GetJournalData(i);

				// If it has a chapter, it is a main quest.
				if ( pJournal )
				{
					QuestInfo* pInfo = g_DataManager.FindQuestInfo(i);
					if ( pInfo && pInfo->nSortIndex > 0 )
					{
						QSort.insert( std::make_pair( pInfo->nSortIndex, pJournal ));
					}
				}
			}
#else	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
			Journal* pJournal = g_DataManager.GetJournalData(i);

			// If it has a chapter, it is a main quest.
			if ( pJournal && pJournal->IsMainQuest() )
			{
				QuestInfo* pInfo = g_DataManager.FindQuestInfo(i);
				if ( pInfo && pInfo->nSortIndex > 0 )
				{
					QSort.insert( std::make_pair( pInfo->nSortIndex, pJournal ));
				}
			}
#endif	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
		}
	}

	std::multimap<int, Journal*>::iterator it = QSort.begin();

	for ( ; it != QSort.end() ; ++it )
	{
		vecCompleteMainQuest.push_back(it->second);
	}

}

int CDnQuestTask::GetQuestCount()
{
	int nQuestCount = 0;

	for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if (m_Quest.Quest[i].nQuestID > 0 )
		{
			nQuestCount++;
		}
	}

	return nQuestCount;
}

void CDnQuestTask::GetPlayQuest( std::vector<TQuest*> &vecProgQuest )
{
	vecProgQuest.reserve(MAX_PLAY_QUEST);

	for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if (m_Quest.Quest[i].nQuestID > 0  && 
			m_Quest.Quest[i].cQuestState == QuestState_Playing
			)
		{
			vecProgQuest.push_back( &(m_Quest.Quest[i]) );
		}
	}
}

void CDnQuestTask::GetPlayMainQuest( std::vector<TQuest*> &vecProgMainQuest )
{
	for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if (m_Quest.Quest[i].nQuestID > 0  && 
			m_Quest.Quest[i].cQuestState == QuestState_Playing &&
			g_DataManager.GetQuestType(m_Quest.Quest[i].nQuestID) == QuestType_MainQuest
			)
		{
			vecProgMainQuest.push_back( &(m_Quest.Quest[i]) );
		}
	}
}

void CDnQuestTask::ClearMainQuestFromQuestID(int nQuestID) // When deleting a quest, remove the corresponding quest.
{
	for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if (m_Quest.Quest[i].nQuestID == nQuestID && g_DataManager.GetQuestType(m_Quest.Quest[i].nQuestID) == QuestType_MainQuest )
		{
			ZeroMemory(&(m_Quest.Quest[i]), sizeof(TQuest));
		}
	}
}

void CDnQuestTask::GetCompleteSubQuest( std::vector<Journal*> &vecCompleteSubQuest )
{
	vecCompleteSubQuest.reserve(MAX_QUEST_INDEX);

	for ( int i = 0 ; i <= MAX_QUEST_INDEX ; i++  )
	{
		if ( IsClearQuest(i) == true )
		{
			Journal* pJournal = g_DataManager.GetJournalData(i);

			// If it has a chapter, it is a main quest.
			if ( pJournal && !pJournal->IsMainQuest()  )
				vecCompleteSubQuest.push_back(pJournal);
		}
	}
}

void CDnQuestTask::GetPlaySubQuest( std::vector<TQuest*> &vecProgSubQuest )
{
	vecProgSubQuest.reserve(MAX_PLAY_QUEST);

	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if( m_Quest.Quest[i].nQuestID > 0  && 
			m_Quest.Quest[i].cQuestState == QuestState_Playing &&
			(g_DataManager.GetQuestType(m_Quest.Quest[i].nQuestID) == QuestType_SubQuest 
			|| g_DataManager.GetQuestType(m_Quest.Quest[i].nQuestID) == QuestType_PeriodQuest
#ifdef PRE_ADD_REMOTE_QUEST
			|| g_DataManager.GetQuestType(m_Quest.Quest[i].nQuestID) == QuestType_RemoteQuest
#endif // PRE_ADD_REMOTE_QUEST
			) )
		{
			vecProgSubQuest.push_back( &(m_Quest.Quest[i]) );
		}
	}
}

void CDnQuestTask::GetPlayQuest( const EnumQuestType eQuestType, std::vector<TQuest*> &vecProgQuest )
{
	vecProgQuest.reserve(MAX_PLAY_QUEST);

	for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		EnumQuestType eType = (EnumQuestType)(g_DataManager.GetQuestType(m_Quest.Quest[i].nQuestID));

		if (m_Quest.Quest[i].nQuestID > 0  &&
			m_Quest.Quest[i].cQuestState == QuestState_Playing &&
			eQuestType == eType )
		{
			vecProgQuest.push_back( &(m_Quest.Quest[i]) );
		}
	}
}

void CDnQuestTask::GetCompleteQuest( const EnumQuestType eQuestType, std::vector<Journal*> &vecCompleteQuest )
{
	vecCompleteQuest.reserve(MAX_QUEST_INDEX);

	for ( int i = 0 ; i <= MAX_QUEST_INDEX ; i++  )
	{
		if ( IsClearQuest(i) == true )
		{
			Journal* pJournal = g_DataManager.GetJournalData(i);

			if(pJournal)
			{
				EnumQuestType eType = (EnumQuestType)(g_DataManager.GetQuestType(pJournal->nQuestIndex));
				// If it has a chapter, it is a main quest.
				if ( !pJournal->IsMainQuest() && eQuestType == eType )
					vecCompleteQuest.push_back(pJournal);
			}
		}
	}
}

void CDnQuestTask::GetCurJournalCountingInfoString( int nQuestIndex, int nJournalPageIndex, OUT std::vector< boost::tuple<std::wstring, int, int> > &vecGetList, OUT wstring &szProgressString  )
{
	char cCurJournalIndex(0);
	std::vector<TCount> Counting;
	GetCurJournalCountingInfo( nQuestIndex, cCurJournalIndex, Counting );

	if( cCurJournalIndex != nJournalPageIndex )
		return;

	DNTableFileFormat* pSoxItem = GetDNTable( CDnTableDB::TITEM );
	if( !pSoxItem ) return;

	DNTableFileFormat* pSoxMonster = GetDNTable( CDnTableDB::TMONSTER );
	if( !pSoxMonster ) return;

	DNTableFileFormat* pSoxSymbol = GetDNTable( CDnTableDB::TSYMBOLITEM );
	if( !pSoxSymbol ) return;

	int nSymbolCnt = 0;
	int nSyombolTotalCnt = 0;

	for(int i=0; i<(int)Counting.size(); i++ )
	{
		wchar_t wszName[256] = {0,};
		switch( Counting[i].cType )
		{
		case CountingType_Item:
			{
				DNTableCell* p = pSoxItem->GetFieldFromLablePtr( Counting[i].nIndex, "_NameID" );
				if ( !p ) return;

				_wcscpy( wszName, _countof(wszName), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, p->GetInteger() ), (int)wcslen(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, p->GetInteger() )) );

				int nCnt = GetItemTask().GetCharInventory().GetItemCount(Counting[i].nIndex);
				int nQuestCnt = GetItemTask().GetQuestInventory().GetItemCount(Counting[i].nIndex);
				Counting[i].nCnt = max(nCnt, nQuestCnt);

				if(Counting[i].nCnt > Counting[i].nTargetCnt) // #22983 Exception handling when item count exceeds target count.
					Counting[i].nCnt =  Counting[i].nTargetCnt;
			}
			break;
		case CountingType_MyMonsterKill:
		case CountingType_AllMonsterKill:
			{
				DNTableCell* p = pSoxMonster->GetFieldFromLablePtr( Counting[i].nIndex, "_NameID" );
				if ( !p ) return;
				DNTableCell* pParam = pSoxMonster->GetFieldFromLablePtr( Counting[i].nIndex, "_NameIDParam" );
				if ( !pParam ) return;
				std::wstring wszMonsterName;
				MakeUIStringUseVariableParam( wszMonsterName, p->GetInteger(), pParam->GetString() );
				_wcscpy( wszName, _countof(wszName), wszMonsterName.c_str(), (int)wcslen(wszMonsterName.c_str()) );
			}
			break;
		case CountingType_SymbolItem:
			{
				DNTableCell* p = pSoxSymbol->GetFieldFromLablePtr( Counting[i].nIndex, "_NameID" );
				if ( !p ) return;
				_wcscpy( wszName, _countof(wszName), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, p->GetInteger() ), (int)wcslen(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, p->GetInteger() )) );
			}
			break;
#if !defined(SW_ADD_QUEST_COUNTINGTYPE_20091014_b4nfter)
		case CountingType_StageClear:
			{
				continue;
			}
#endif	// #if !defined(SW_ADD_QUEST_COUNTINGTYPE_20091014_b4nfter)
		}

		std::wstring _wszName;
		_wszName = wszName;

		if ( _wszName.empty() )
#ifdef _FINAL_BUILD
			continue;
#else
			_wszName = L"UnKnown";
#endif
		// Do not display if 30000.
		if ( Counting[i].nTargetCnt == 30000 ) {
			continue;
		}
		// Display name only if 30001.
		else if ( Counting[i].nTargetCnt == 30001 ) {
			swprintf_s( wszName, _countof(wszName), L"- %s ", _wszName.c_str() );
			vecGetList.push_back( boost::make_tuple(std::wstring(wszName), -1, -1) );
		}
		// Display name + current count only if 30002.
		else if ( Counting[i].nTargetCnt == 30002 ) {
			//swprintf_s( wszName, _countof(wszName), L"%s [%d]", _wszName.c_str(), Counting[i].nCnt );
			vecGetList.push_back( boost::make_tuple(std::wstring(_wszName), Counting[i].nCnt, -1) );
		}
		// Show all
		else {
			vecGetList.push_back( boost::make_tuple( std::wstring(_wszName), Counting[i].nCnt, Counting[i].nTargetCnt  ) );
		}
	}
}

void 
CDnQuestTask::RefreshQuestNotifierInfo()
{
	if( m_pQuestDialog ) 
		m_pQuestDialog->RefreshSummaryInfo(); 
}


#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

#define MAX_BENEFIT_TABLE_COUNT 6

bool CDnQuestTask::CheckAndCalcStoreBenefit( int iType, int& iNeedCoin )
{
	bool bResult = false;
	if (m_pReputationRepos == NULL)
		return false;

	DNTableFileFormat*  pBenefitTable = GetDNTable( CDnTableDB::TREPUTEBENEFIT );
	
	const std::vector<CDnInterface::SUnionMembershipData>& membershipList = m_pReputationRepos->GetUnionMembershipData();
	std::vector<CDnInterface::SUnionMembershipData>::const_iterator iter = membershipList.begin();
	for (; iter != membershipList.end(); ++iter)
	{
		const CDnInterface::SUnionMembershipData& curData = (*iter);
		std::string columnName;
		int i = 1;
		for (; i < MAX_BENEFIT_TABLE_COUNT + 1; ++i)
		{
			columnName = FormatA("_Benefit%d", i);
			int tableType = pBenefitTable->GetFieldFromLablePtr( curData.itemId, columnName.c_str() )->GetInteger();
			if (tableType == iType)
			{
				columnName = FormatA("_Benefit%d_Num", i);
				int iAdjustPercent = pBenefitTable->GetFieldFromLablePtr( curData.itemId, columnName.c_str() )->GetInteger();
				if( NpcReputation::StoreBenefit::SellingPriceUp == iType )
				{
					iNeedCoin += int( (float)iNeedCoin * (float(iAdjustPercent) / 100.0f) );
				}
				else
				{
					iNeedCoin -= int( (float)iNeedCoin * (float(iAdjustPercent) / 100.0f) );
				}

				bResult = true;
				return bResult;
			}
		}
	}
	
	return bResult;
}

int CDnQuestTask::GetStoreBenefitValue( int iType )
{
	int iResult = 0;
	if (m_pReputationRepos == NULL)
		return false;

	DNTableFileFormat*  pBenefitTable = GetDNTable( CDnTableDB::TREPUTEBENEFIT );

	const std::vector<CDnInterface::SUnionMembershipData>& membershipList = m_pReputationRepos->GetUnionMembershipData();
	std::vector<CDnInterface::SUnionMembershipData>::const_iterator iter = membershipList.begin();
	for (; iter != membershipList.end(); ++iter)
	{
		const CDnInterface::SUnionMembershipData& curData = (*iter);
		std::string columnName;
		int i = 1;
		for (; i < MAX_BENEFIT_TABLE_COUNT + 1; ++i)
		{
			columnName = FormatA("_Benefit%d", i);
			int tableType = pBenefitTable->GetFieldFromLablePtr( curData.itemId, columnName.c_str() )->GetInteger();
			if (tableType == iType)
			{
				columnName = FormatA("_Benefit%d_Num", i);
				iResult = pBenefitTable->GetFieldFromLablePtr( curData.itemId, columnName.c_str() )->GetInteger();
				return iResult;
			}
		}
	}

	return iResult;
}

void CDnQuestTask::OnRecvReputationList( SCReputationList* pPacket )
{
	// All NPC reputation info pushed whenever connecting to village server.
	// If empty, packet arrives with count 0
	if( 0 == pPacket->cCount )
	{
		m_pReputationRepos->Clear();
	}
	else
	{
		for( int i = 0; i < pPacket->cCount; ++i )
		{
			const TNpcReputation& Reputation = pPacket->ReputationArr[ i ];
			m_pReputationRepos->SetNpcReputation( Reputation.iNpcID, IReputationSystem::NpcFavor, Reputation.iFavorPoint );
			m_pReputationRepos->SetNpcReputation( Reputation.iNpcID, IReputationSystem::NpcMalice, Reputation.iMalicePoint );
		}
	}

	// Refresh if NPC favorability dialog is currently shown.. though that will rarely happen.
	if( GetInterface().GetMainMenuDialog() )
		GetInterface().GetMainMenuDialog()->UpdateNpcReputationDlg();
}

void CDnQuestTask::OnRecvReputationModify( SCModReputation* pPacket )
{
	bool bFavorUp = false;

	std::wstring favorPresentStr, npcStr;
	DNTableFileFormat*  pNpcTable = GetDNTable( CDnTableDB::TNPC );
	int updateNpcId = -1;
	bool bUpdateLastFavorNpc = false;

	// Reputation info of updated NPCs
	for( int i = 0; i < pPacket->cCount; ++i )
	{
		const TNpcReputation& Reputation = pPacket->UpdateArr[ i ];

#ifdef PRE_ADD_REPUTATION_EXPOSURE
		// When favorability of the selected NPC increases, friendly NPCs' favorability may also increase together.
		// In this case, set so that the last selected NPC ID is properly selected.
		// The 0th NPC in the array is the NPC I selected.
		if(i == 0)
			m_pReputationRepos->SetLastUpdateNpcID(Reputation.iNpcID);
#endif

		// Play sound effect if increased compared to before. #20908
		int iLegacyFavor = (int)m_pReputationRepos->GetNpcReputation( Reputation.iNpcID, IReputationSystem::NpcFavor );
		if( iLegacyFavor < Reputation.iFavorPoint )
		{
			bFavorUp = true;
			if (pNpcTable)
			{
				if (npcStr.empty() == false)
					npcStr += L", ";

				int nameStringNum = pNpcTable->GetFieldFromLablePtr(Reputation.iNpcID, "_NameID")->GetInteger();
				npcStr += FormatW(L"%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nameStringNum));
				updateNpcId = Reputation.iNpcID;
				bUpdateLastFavorNpc = true;
			}
		}
		else
		{
			int maxFavor = (int)m_pReputationRepos->GetNpcReputationMax( Reputation.iNpcID, IReputationSystem::NpcFavor );
			if (maxFavor == Reputation.iFavorPoint)
			{
				updateNpcId = Reputation.iNpcID;
				bUpdateLastFavorNpc = true;
			}
		}

		m_pReputationRepos->SetNpcReputation( Reputation.iNpcID, IReputationSystem::NpcFavor, Reputation.iFavorPoint );
		m_pReputationRepos->SetNpcReputation( Reputation.iNpcID, IReputationSystem::NpcMalice, Reputation.iMalicePoint );
	}

	if( bFavorUp )
	{
		if( -1 == m_iReputeUpSoundIndex )
			m_iReputeUpSoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( "UI_winfavor.ogg" ).c_str(), false, false );

		if( !m_hReputeUpSound ||
			false == m_hReputeUpSound->IsPlay() )
		{
			if( -1 != m_iReputeUpSoundIndex )
			{
				m_hReputeUpSound = CEtSoundEngine::GetInstance().PlaySound__(  "2D", m_iReputeUpSoundIndex, false, true  );
				if( m_hReputeUpSound ) {
					m_hReputeUpSound->SetVolume( CEtSoundEngine::GetInstance().GetMasterVolume("2D") );
					m_hReputeUpSound->Resume();
				}
			}
		}

		if (npcStr.empty() == false)
		{
			favorPresentStr = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3283), npcStr.c_str()); // UISTRING : [Affinity increased] with %s.
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", favorPresentStr.c_str(), false);
			GetInterface().ShowCaptionDialog(CDnInterface::typeCaption2, favorPresentStr.c_str(), textcolor::YELLOW);
		}
	}
}

void CDnQuestTask::OnRecvReputationOpenGiveNpcPresent( SCOpenGiveNpcPresent* pPacket )
{
	//GetInterface().OpenGiveNpcPresent( pPacket->nNpcID );

	CDnNpcDlg* pNpcDlg = GetInterface().GetNpcDialog();
	if( pNpcDlg )
		pNpcDlg->SetAcceptPresent( pPacket->nNpcID );
}

void CDnQuestTask::OnRecvRequestSendNpcPresentSelect( SCRequestSendSelectedPresent* pPacket )
{ 
	CDnNpcDlg* pNpcDlg = GetInterface().GetNpcDialog();
	if( pNpcDlg )
	{
		pNpcDlg->SendSelectedNpcPresent( pPacket->nNpcID );
	}
}

void CDnQuestTask::OnRecvShowNpcEffect( SCShowNpcEffect* pPacket )
{
	// Play effect on specific NPC
	m_NpcReaction.AttachNpcEffect( pPacket->nNpcID, pPacket->nEffectIndex );
}

void CDnQuestTask::OnRecvUnionPoint(SCUnionPoint* pPacket)
{
	int i = 0;
	for (; i < NpcReputation::UnionType::Etc; ++i)
	{
		INT64 point = pPacket->biUnionPoint[i];
		UNIONPT_TYPE curUnionPt = m_pReputationRepos->GetUnionPoint(i);
		if (curUnionPt >= 0 && curUnionPt < point)
		{
			DNTableFileFormat*  pUnionTable = GetDNTable(CDnTableDB::TUNION);
			if (pUnionTable != NULL)
			{
				int tableID = pUnionTable->GetItemIDFromField("_UnionID", i);
				int unionNameStringNum = pUnionTable->GetFieldFromLablePtr(tableID, "_UnionName")->GetInteger();

				std::wstring ptUpStr = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3284), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, unionNameStringNum)); // UISTRING : [Earned points] for the %s Union.
				GetInterface().ShowCaptionDialog(CDnInterface::typeCaption2, ptUpStr.c_str(), textcolor::YELLOW);
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", ptUpStr.c_str(), false);
				if( GetInterface().GetMainBarDialog() ) 
					GetInterface().GetMainBarDialog()->BlinkMenuButton( CDnMainMenuDlg::NPC_REPUTATION_TAB_DIALOG );
			}
		}
		m_pReputationRepos->SetUnionPoint(i, point);
	}
}


void CDnQuestTask::OnRecvUnionMembershipList(SCUnionMembershipList* pData)
{
	if (m_pReputationRepos == NULL)
		return;
	__time64_t curTime;
	_time64(&curTime);

	int i = 0;
	for (; i < NpcReputation::UnionType::Etc; ++i)
	{
		m_pReputationRepos->SetUnionMembershipData(i, pData->nItemID[i], curTime, pData->nLeftUseTime[i]);
	}

	const std::vector<CDnInterface::SUnionMembershipData>& membershipData = m_pReputationRepos->GetUnionMembershipData();
	CDnInterface::GetInstance().RefreshRepUnionMembershipMark(membershipData);
}

void CDnQuestTask::OnRecvUnionBuyMembership(SCBuyUnionMembershipResult* pData)
{
	if (m_pReputationRepos == NULL)
		return;
	__time64_t curTime;
	_time64(&curTime);

	m_pReputationRepos->SetUnionMembershipData(pData->cType, pData->nItemID, curTime, pData->nLeftUseTime);

	const std::vector<CDnInterface::SUnionMembershipData>& membershipData = m_pReputationRepos->GetUnionMembershipData();
	CDnInterface::GetInstance().RefreshRepUnionMembershipMark(membershipData);

	DNTableFileFormat*  pBenefitTable = GetDNTable( CDnTableDB::TREPUTEBENEFIT );
	if (pBenefitTable == NULL)
		return;

	int uiStringNum = pBenefitTable->GetFieldFromLablePtr(pData->nItemID, "_NameID")->GetInteger();
	int period = pBenefitTable->GetFieldFromLablePtr(pData->nItemID, "_Period")->GetInteger();
	std::wstring confirmMsg = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3278), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, uiStringNum), period); // UISTRING : Purchased %s membership. \nMembership benefits will apply for %d day(s).
	CDnInterface::GetInstance().MessageBox(confirmMsg.c_str(), MB_OK);
}

#ifdef PRE_FIX_71455_REPUTE_RENEW
void CDnQuestTask::RefreshRepUnionMembershipMark()
{
	const std::vector<CDnInterface::SUnionMembershipData>& membershipData = m_pReputationRepos->GetUnionMembershipData();
	std::vector<CDnInterface::SUnionMembershipData>::const_iterator iter = membershipData.begin();

	bool bExistExpireUnionMembership = false;
	CDnInterface::SUnionMembershipData data;
	for( ; iter != membershipData.end(); iter++ )
	{
		data = *iter;
		const time_t pNowTime = CSyncTimer::GetInstance().GetCurTime();
		if( pNowTime >= data.expireDate )
		{
			bExistExpireUnionMembership = true;
			break;
		}
	}

	if( bExistExpireUnionMembership )
	{
		m_pReputationRepos->DeleteUnionMembershipData( data.unionType, data.itemId );
		CDnInterface::GetInstance().RefreshRepUnionMembershipMark( m_pReputationRepos->GetUnionMembershipData() );
	}
}
#endif // PRE_FIX_71455_REPUTE_RENEW

#endif // PRE_ADD_NPC_REPUTATION_SYSTEM

int CDnQuestTask::IsAllCompleteCounting(int nQuestID)
{
	int nSlot = FindPlayingQuest(nQuestID);
	if ( nSlot < 0  )
	{
		// Quest not found.
		return -2;
	}

	if ( m_Quest.Quest[nSlot].cQuestState == QuestState_Playing )
	{
		bool CompleteAllCounting = false;

		int bHaveNoCount = true; // If at least one is inside, the count is non-zero.

		TCount* pCount = (TCount*)m_Quest.Quest[nSlot].Extra;

		for ( int j = 0 ; j < QUEST_EXTRACOUNT_MAX ; j++ )
		{
			if ( pCount[j].cType != CountingType_None && pCount[j].cType != CountingType_Max )
			{
	
				// If any goal has not been fully met 
				if ( pCount[j].nCnt < pCount[j].nTargetCnt )
				{
					bHaveNoCount = false;
					return -3;
				}
				else if ( pCount[j].nCnt == pCount[j].nTargetCnt )
				{
					bHaveNoCount = false;
					CompleteAllCounting = true;
				}
			}
		}

		// When all counting conditions of the current quest are met 

		if ( CompleteAllCounting )
		{
			return 1;
		}

		if(bHaveNoCount)
		{
			return 1;
		}
	}

	return -3;
}

void CDnQuestTask::LoadQuestTalk()
{
	for( int itr = 0; itr < MAX_PLAY_QUEST; ++itr )
		g_DataManager.LoadQuestData( m_Quest.Quest[itr].nQuestID );

	if( m_pQuestDialog )
		m_pQuestDialog->RefreshQuest();	
}

#ifdef PRE_ADD_REMOTE_QUEST

void CDnQuestTask::AddRemoteQuestAskList( int nQuestIndex, CDnQuestTree::eRemoteQuestState remoteQuestState )
{
	DNTableFileFormat *pJobSox = GetDNTable( CDnTableDB::TJOB );
	if( pJobSox && CDnActor::s_hLocalActor )
	{
		int nJobID = static_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() )->GetJobClassID();
		if( pJobSox->IsExistItem( nJobID ) )
		{
			DNTableCell *pRemoteCell = pJobSox->GetFieldFromLablePtr( nJobID, "_QuestRemote" );
			if( pRemoteCell && pRemoteCell->GetInteger() < 0 )
				return;
		}
	}

	bool bExsit = false;
	for( int i=0; i<static_cast<int>( m_vecRemoteQuestAskList.size() ); i++ )
	{
		if( m_vecRemoteQuestAskList[i].nQuestIndex == nQuestIndex )
		{
			m_vecRemoteQuestAskList[i].remoteQuestState = remoteQuestState;
			bExsit = true;
			break;
		}
	}

	if( !bExsit )
	{
		stRemoteQuestAsk remoteQuestAsk;
		remoteQuestAsk.nQuestIndex = nQuestIndex;
		remoteQuestAsk.remoteQuestState = remoteQuestState;
		m_vecRemoteQuestAskList.push_back( remoteQuestAsk );
	}
}

void CDnQuestTask::DeleteRemoteQuestAskList( int nQuestIndex )
{
	for( int i=0; i<static_cast<int>( m_vecRemoteQuestAskList.size() ); i++ )
	{
		if( nQuestIndex == m_vecRemoteQuestAskList[i].nQuestIndex )
		{
			m_vecRemoteQuestAskList.erase( m_vecRemoteQuestAskList.begin() + i );
			break;
		}
	}
}

bool CDnQuestTask::IsExistRemoteQuestAskList( int nQuestIndex, CDnQuestTree::eRemoteQuestState remoteQuestState )
{
	bool bExist = false;
	for( int i=0; i<static_cast<int>( m_vecRemoteQuestAskList.size() ); i++ )
	{
		if( nQuestIndex == m_vecRemoteQuestAskList[i].nQuestIndex 
			&& remoteQuestState == m_vecRemoteQuestAskList[i].remoteQuestState )
		{
			bExist = true;
			break;
		}
	}

	return bExist;
}

void CDnQuestTask::OnRecvGainRemoteQuest( SCGainRemoteQuest* pPacket )
{
	if( pPacket == NULL )
		return;

	if( IsExistRemoteQuestAskList( pPacket->nQuestID, CDnQuestTree::REMOTEQUEST_ASK ) )
		return;

	CDnMainDlg *pMainBarDlg = GetInterface().GetMainBarDialog();
	if( pMainBarDlg ) 
	{
		pMainBarDlg->ShowQuestNotify( true, CDnQuestTree::REMOTEQUEST_ASK );
		pMainBarDlg->BlinkMenuButton( CDnMainMenuDlg::QUEST_DIALOG );
	}

	AddRemoteQuestAskList( pPacket->nQuestID, CDnQuestTree::REMOTEQUEST_ASK );

	if( m_pQuestDialog )
		m_pQuestDialog->RecieveRemoteQuest( pPacket->nQuestID, CDnQuestTree::REMOTEQUEST_ASK );
}

void CDnQuestTask::OnRecvCompleteRemoteQuest( SCCompleteRemoteQuest* pPacket )
{
	if( pPacket == NULL )
		return;	

	if( IsExistRemoteQuestAskList( pPacket->nQuestID, CDnQuestTree::REMOTEQUEST_COMPLETE ) )
		return;

	CDnMainDlg* pMainBarDlg = GetInterface().GetMainBarDialog();
	if( pMainBarDlg ) 
	{
		pMainBarDlg->ShowQuestNotify( true, CDnQuestTree::REMOTEQUEST_COMPLETE );
		pMainBarDlg->BlinkMenuButton( CDnMainMenuDlg::QUEST_DIALOG );
	}

	if( m_pQuestDialog )
		m_pQuestDialog->RecieveRemoteQuest( pPacket->nQuestID, CDnQuestTree::REMOTEQUEST_COMPLETE );

	AddRemoteQuestAskList( pPacket->nQuestID, CDnQuestTree::REMOTEQUEST_COMPLETE );
}

void CDnQuestTask::OnRecvRemoveRemoteQuest( SCRemoveRemoteQuest* pPacket )
{
	if( pPacket == NULL )
		return;

	DeleteRemoteQuestAskList( pPacket->nQuestID );

	if( m_pQuestDialog ) 
	{
		TQuest tQuest;
		memset( &tQuest, 0 , sizeof(TQuest) );
		tQuest.nQuestID = pPacket->nQuestID;
		m_pQuestDialog->DeleteQuest( &tQuest );
		m_pQuestDialog->RefreshSummaryInfo();
	}
}

void CDnQuestTask::SendAcceptRemoteQuest( int nQuestIndex )
{
	CSAcceptRemoteQuest acceptRemoteQuest;
	memset( &acceptRemoteQuest, 0 , sizeof(CSAcceptRemoteQuest) );
	acceptRemoteQuest.nQuestID = nQuestIndex;

	CClientSessionManager::GetInstance().SendPacket( CS_QUEST, eQuest::CS_ACCEPT_REMOTE_QEUST, (char*)&acceptRemoteQuest, sizeof(CSAcceptRemoteQuest) );
}

void CDnQuestTask::SendCompleteRemoteQuest( int nQuestIndex, int nRewardIndex, bool bSelectedArray[], bool bSelectedCashArray[] )
{
	CSCompleteRemoteQuest completeRemoteQuest;
	memset( &completeRemoteQuest, 0 , sizeof(CSCompleteRemoteQuest) );

	completeRemoteQuest.nQuestID = nQuestIndex;
	completeRemoteQuest.nIndex = nRewardIndex;
	CopyMemory( &(completeRemoteQuest.SelectArray[0]), bSelectedArray, sizeof(bool)*QUESTREWARD_INVENTORYITEMMAX );
	CopyMemory( &(completeRemoteQuest.SelectCashArray[0]), bSelectedCashArray, sizeof(bool)*MAILATTACHITEMMAX );

	CClientSessionManager::GetInstance().SendPacket( CS_QUEST, eQuest::CS_COMPLETE_REMOTE_QUEST, (char*)&completeRemoteQuest, sizeof(CSCompleteRemoteQuest) );
}

void CDnQuestTask::CancelRemoteQuest( int nQuestIndex )
{
	CSCancelRemoteQuest cancelRemoteQuest;
	memset( &cancelRemoteQuest, 0 , sizeof(CSCancelRemoteQuest) );

	cancelRemoteQuest.nQuestID = nQuestIndex;
	CClientSessionManager::GetInstance().SendPacket( CS_QUEST, eQuest::CS_CANCEL_REMOTE_QUEST, (char*)&cancelRemoteQuest, sizeof(CSCancelRemoteQuest) );
}

void CDnQuestTask::GetRecompenseRemoteQuest( const EnumQuestType eQuestType, std::vector<TQuest*> &vecProgQuest )
{
	vecProgQuest.reserve(MAX_PLAY_QUEST);

	for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		EnumQuestType eType = (EnumQuestType)(g_DataManager.GetQuestType(m_Quest.Quest[i].nQuestID));

		if (m_Quest.Quest[i].nQuestID > 0  &&
			m_Quest.Quest[i].cQuestState == QuestState_Recompense &&
			eQuestType == eType )
		{
			vecProgQuest.push_back( &(m_Quest.Quest[i]) );
		}
	}
}

#endif // PRE_ADD_REMOTE_QUEST
