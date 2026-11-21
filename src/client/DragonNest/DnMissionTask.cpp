#include "StdAfx.h"
#include "DnMissionTask.h"
#include "DnTableDB.h"
#include "DnMissionDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnInCodeResource.h"
#include "DnUIString.h"
#include "GameOptionSendPacket.h"
#include "DnQuestTask.h"
#include "MissionSendPacket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnMissionTask::MissionInfoStruct::MissionInfoStruct()
{
	nArrayIndex = 0;
	nNotifierID = 0;
	MainCategory = CDnMissionTask::Normal;
	nIcon = 0;
	nAchieveIcon = 0;
	memset(nRewardItemID, 0, sizeof(nRewardItemID));
	memset(nRewardCashItemSN, 0, sizeof(nRewardCashItemSN));
	memset(nRewardItemCount, 0, sizeof(nRewardItemCount));
	memset(nRewardCashItemCount, 0, sizeof(nRewardCashItemCount));
	nRewardCoin = 0;
	nRewardAppellationID = 0;
	nRewardPoint = 0;
	nRewardCounter = 0;
	nCounter = 0;
	nAchieveSystemMsgID = 0;
	nType = 0;
	bAchieve = false;
}
void CDnMissionTask::MissionInfoStruct::Clone( MissionInfoStruct * p )
{
	nArrayIndex = p->nArrayIndex;
	nNotifierID = p->nNotifierID;
	MainCategory = p->MainCategory;
	szSubCategory = p->szSubCategory;
	szParentSubCategory = p->szParentSubCategory;
	szTitle = p->szTitle;
	szSubTitle = p->szSubTitle;
	nIcon = p->nIcon;			
	nAchieveIcon = p->nAchieveIcon;	

	memcpy( nRewardItemID, p->nRewardItemID, sizeof(nRewardItemID) );
	memcpy( nRewardCashItemSN, p->nRewardCashItemSN, sizeof(nRewardCashItemSN) );
	memcpy( nRewardItemCount, p->nRewardItemCount, sizeof(nRewardItemCount) );
	memcpy( nRewardCashItemCount, p->nRewardCashItemCount, sizeof(nRewardCashItemCount) );

	nRewardCoin = p->nRewardCoin;
	nRewardAppellationID = p->nRewardAppellationID;
	nRewardPoint = p->nRewardPoint;
	nRewardCounter = p->nRewardCounter;
	nCounter = p->nCounter;
	nAchieveSystemMsgID = p->nAchieveSystemMsgID;
	nType = p->nType;
	bAchieve = p->bAchieve;
}

//////////////////////////////////////////////////////////////////////////

CDnMissionTask::CDnMissionTask()
: CTaskListener( false )
{
	memset( m_nMissionScore, 0, sizeof(m_nMissionScore) );
	m_nQuestScore = 0;

	for( int i=0; i<LASTMISSIONACHIEVEMAX; i++ ) 
		m_nLastUpdateAchieveList[i] = -1;
	
#ifdef PRE_MOD_MISSION_HELPER
	m_NotifierData[0].Register( DNNotifier::Type::NormalMission, -1 );
	m_NotifierData[1].Register( DNNotifier::Type::DailyMission, -1 );
#else
	m_NotifierData[0].Register( DNNotifier::Type::DailyMission, -1);
	m_NotifierData[1].Register( DNNotifier::Type::WeeklyMission, -1);
#endif
}

CDnMissionTask::~CDnMissionTask()
{
	Finalize();
}

void CDnMissionTask::OnDisconnectTcp( bool bValidDisconnect )
{
}

void CDnMissionTask::OnDisconnectUdp()
{
}

void CDnMissionTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	switch( nMainCmd ) {
		case SC_MISSION: OnRecvMissionMessage( nSubCmd, pData, nSize ); break;
	}
}

bool CDnMissionTask::Initialize()
{
#if defined( PRE_ADD_CHAT_MISSION )
	InitChatMission();
#endif	// #if defined( PRE_ADD_CHAT_MISSION )

	return true;
}

void CDnMissionTask::Finalize()
{
	SAFE_DELETE_MAP( m_nMapSearch );
	SAFE_DELETE_PVEC( m_pVecMissionList );

#ifdef PRE_ADD_STAMPSYSTEM
	m_mapMission.clear(); // <int(mission id),>
#endif // PRE_ADD_STAMPSYSTEM

	for( int i=0; i<DailyMissionTypeEnum_Amount; i++ ) {
		SAFE_DELETE_MAP( m_nMapDailySearch[i] );
		SAFE_DELETE_PVEC( m_pVecDailyMissionList[i] );
	}
}

void CDnMissionTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_VecGainAlarmQueueList.empty() ) {
		m_VecGainAlarmQueueList[0].fDelta--;
		if( m_VecGainAlarmQueueList[0].fDelta <= 0.f ) {
			GetInterface().ShowMissionAlarmDialog( m_VecGainAlarmQueueList[0].pInfo, 2.f );
			m_VecGainAlarmQueueList.erase( m_VecGainAlarmQueueList.begin() );
		}
	}

	if( !m_VecAchieveAlarmQueueList.empty() ) {
		fDelta = min( fDelta, 0.5f );	// When entering village for the first time, fDelta can exceed 2.5s due to loading. So forcibly lower fDelta.
		m_VecAchieveAlarmQueueList[0].fDelta -= fDelta;
		if( m_VecAchieveAlarmQueueList[0].fDelta <= 0.f ) {
			GetInterface().ShowMissionAlarmDialog( m_VecAchieveAlarmQueueList[0].pInfo, 3.f );

#ifdef PRE_ADD_ACTIVEMISSION
			// For ActiveMission, since it was forcibly added, memory must be freed here.
			if( m_VecAchieveAlarmQueueList[0].pInfo->nArrayIndex == ACTIVEMISSIONID )
				SAFE_DELETE( m_VecAchieveAlarmQueueList[0].pInfo );
#endif // PRE_ADD_ACTIVEMISSION

			m_VecAchieveAlarmQueueList.erase( m_VecAchieveAlarmQueueList.begin() );
		}
	}
}

void CDnMissionTask::OnRecvMissionMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eMission::SC_MISSION_LIST: OnRecvMissionList( (SCMissionList*)pData ); break;
		case eMission::SC_GAIN_MISSION: OnRecvMissionGain( (SCMissionGain*)pData ); break;
		case eMission::SC_ACHIEVE_MISSION: OnRecvMissionAchieve( (SCMissionAchieve*)pData ); break;

		case eMission::SC_DAILYMISSION_LIST: OnRecvDailyMissionList( (SCDailyMissionList*)pData ); break;
		case eMission::SC_COUNT_DAILYMISSION: OnRecvDailyMissionCount( (SCDailyMissionCount*)pData ); break;
		case eMission::SC_ACHIEVE_DAILYMISSION: OnRecvDailyMissionAchieve( (SCDailyMissionAchieve*)pData ); break;

		case eMission::SC_ALARM_DAILYMISSION: OnRecvDailyMissionAlarm( (SCDailyMissionAlarm*)pData ); break;
		case eMission::SC_EVENT_POPUP: OnRecvEventPopUp( (SCMissionEventPopUp*)pData ); break;
		case eMission::SC_HELP_ALARM: OnRecvHelpAlarm( (SCMissionHelpAlarm*)pData ); break;

#ifdef PRE_ADD_ACTIVEMISSION
		case eMission::SC_GAIN_ACTIVE_MISSION: OnGainActiveMission( pData ); break;
		case eMission::SC_ACHIEVE_ACTIVE_MISSION: OnAchieveActiveMission( pData ); break;
#endif // PRE_ADD_ACTIVEMISSION


	}
}

void CDnMissionTask::OnRecvMissionList( SCMissionList *pPacket )
{
	// Get the accumulated daily mission score,
	m_nQuestScore = pPacket->nMissionScore;

	memset( m_nMissionScore, 0, sizeof(m_nMissionScore) );

	for( DWORD i=0; i<MISSIONMAX; i++ ) {
		if( GetBitFlag( pPacket->cGain, i ) == true ) {
			MissionInfoStruct *pStruct = AddMission( i );
			if( pStruct == NULL ) {
				//_ASSERT(0);
				// Missions that should not be visible on the client may have pStruct set to NULL.
				continue;
			}
			if( GetBitFlag( pPacket->cAchieve, i ) == true ) {
				pStruct->bAchieve = true;
				m_nMissionScore[pStruct->MainCategory] += pStruct->nRewardPoint;
			}
		}
	}
	memcpy( m_nLastUpdateAchieveList, pPacket->wLastUpdate, sizeof(short) * LASTMISSIONACHIEVEMAX );
}

void CDnMissionTask::OnRecvMissionGain( SCMissionGain *pPacket )
{
	MissionInfoStruct *pStruct = AddMission( pPacket->nArrayIndex );
	if( pStruct == NULL ) return;

	OnGainMission( pStruct );
}

void CDnMissionTask::OnRecvMissionAchieve( SCMissionAchieve *pPacket )
{
	std::map<int, MissionInfoStruct *>::iterator it = m_nMapSearch.find( pPacket->nArrayIndex );
	if( it == m_nMapSearch.end() ) return;

	MissionInfoStruct *pStruct = it->second;
	pStruct->bAchieve = true;
	m_nMissionScore[pStruct->MainCategory] += pStruct->nRewardPoint;

	UpdateLastAchieveList( pPacket->nArrayIndex );
	OnAchieveMission( pStruct );
}

void CDnMissionTask::OnRecvDailyMissionList( SCDailyMissionList *pPacket )
{
	SAFE_DELETE_PVEC( m_pVecDailyMissionList[pPacket->cType] );
	SAFE_DELETE_MAP( m_nMapDailySearch[pPacket->cType] );

	for( int i=0; i<pPacket->nCount; i++ ) {
		MissionInfoStruct *pStruct = AddDailyMission( (DailyMissionTypeEnum)pPacket->cType, i, pPacket->Mission[i].nMissionID );
		if( !pStruct ) continue;
		pStruct->nCounter = pPacket->Mission[i].nCounter;
		pStruct->bAchieve = pPacket->Mission[i].bAchieve;
	}

	CDnMissionDlg *pDlg = (CDnMissionDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::MISSION_DIALOG );
	if( pDlg ) pDlg->RefreshInfo();
}

void CDnMissionTask::OnRecvDailyMissionCount( SCDailyMissionCount *pPacket )
{
	std::map<int, MissionInfoStruct *>::iterator it = m_nMapDailySearch[pPacket->cType].find( pPacket->nArrayIndex );
	if( it == m_nMapDailySearch[pPacket->cType].end() ) return;

	MissionInfoStruct *pStruct = it->second;
	pStruct->nCounter = pPacket->nCount;

	OnCounterDailyMission( pStruct );
}

void CDnMissionTask::OnRecvDailyMissionAchieve( SCDailyMissionAchieve *pPacket )
{
	std::map<int, MissionInfoStruct *>::iterator it = m_nMapDailySearch[pPacket->cType].find( pPacket->nArrayIndex );
	if( it == m_nMapDailySearch[pPacket->cType].end() ) return;

	MissionInfoStruct *pStruct = it->second;
	pStruct->bAchieve = true;
	pStruct->nCounter = pStruct->nRewardCounter;

	if( (DailyMissionTypeEnum)pPacket->cType == CDnMissionTask::GuildWar )	// For guild missions, increment nRewardPoint by 1 (actual nRewardPoint is used as guild festival points)
	{
		m_nQuestScore += 1;

		if( pPacket->bSecret )	// If secret, display the mission acquired icon as secret
			pStruct->nAchieveIcon++;
	}
	else
		m_nQuestScore += pStruct->nRewardPoint;
	OnAchieveDailyMission( pStruct );
}

void CDnMissionTask::OnRecvDailyMissionAlarm( SCDailyMissionAlarm *pPacket )
{
	switch( pPacket->cType ) {
		case Daily:
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7127 ), false );
			break;
		case Weekly:
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7128 ), false );
			break;
#ifdef PRE_ADD_MONTHLY_MISSION
		case MonthlyEvent:
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10168 ), false ); 
#endif 
	}
}

void CDnMissionTask::OnRecvEventPopUp( SCMissionEventPopUp* pPacket )
{
	// Find the matching record in the event table by mission table ID and get the URL value.
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TEVENTPOPUP );
	if (!pSox)
		return;

	char *szUrlAddress = pSox->GetFieldFromLablePtr( pPacket->nItemID, "_ImgUrl" )->GetString();

#ifdef PRE_ADD_START_POPUP_QUEUE
	if (szUrlAddress != NULL && szUrlAddress[0] != '\0')
	{
		CDnStartPopupMgr& mgr = GetInterface().GetStartPopupMgr();
		mgr.RegisterLow(CDnStartPopupMgr::eEventGuide, NULL, szUrlAddress);

		mgr.StartShow();
	}
#else
	CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
	if (pMainMenuDlg)
		pMainMenuDlg->OnEventPopUp (szUrlAddress);
#endif
}

void CDnMissionTask::OnRecvHelpAlarm( SCMissionHelpAlarm *pPacket )
{
	GetInterface().ShowHelpAlarmDialog( pPacket->nItemID );
}

void CDnMissionTask::OnGainMission( MissionInfoStruct *pStruct )
{
	CDnMissionDlg *pDlg = (CDnMissionDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::MISSION_DIALOG );
	if( pDlg ) pDlg->RefreshInfo();

	AlarmQueueStruct AlarmStruct( ( m_VecGainAlarmQueueList.empty() ) ? 0.f : 5.f, pStruct ); 
	m_VecGainAlarmQueueList.push_back( AlarmStruct );

	/*
	if( CDnActor::s_hLocalActor ) {
		DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
		if( hHandle ) {
			hHandle->SetPosition( *CDnActor::s_hLocalActor->GetPosition() );
			hHandle->SetActionQueue( "MissionGain" );
		}
	}

	char szTemp[256];
	WideCharToMultiByte(CP_ACP, 0, pStruct->szTitle.c_str(), -1, szTemp, 256, NULL, NULL);

OutputDebug( "Mission acquired : %s\n", szTemp );
	*/
	RefreshNotifier();
}

void CDnMissionTask::OnAchieveMission( MissionInfoStruct *pStruct )
{
	CDnMissionDlg *pDlg = (CDnMissionDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::MISSION_DIALOG );
	if( pDlg ) pDlg->RefreshInfo();

	AlarmQueueStruct AlarmStruct( ( m_VecAchieveAlarmQueueList.empty() ) ? 0.f : 5.f, pStruct ); 
	m_VecAchieveAlarmQueueList.push_back( AlarmStruct );

	if( CDnActor::s_hLocalActor ) {
		if( !m_hAchieveAlarmEffect ) {
			m_hAchieveAlarmEffect = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
			if( m_hAchieveAlarmEffect ) {
				m_hAchieveAlarmEffect->SetPosition( *CDnActor::s_hLocalActor->GetPosition() );
				m_hAchieveAlarmEffect->SetActionQueue( "MissionAchieve" );
			}
		}
	}
	if( pStruct->nAchieveSystemMsgID > 0 ) {
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pStruct->nAchieveSystemMsgID ) );
	}

#ifdef PRE_MOD_MISSION_HELPER
	RemoveNotifier( DNNotifier::Type::NormalMission, pStruct->nNotifierID );
#endif
}


void CDnMissionTask::OnCounterDailyMission( MissionInfoStruct *pStruct )
{
	RefreshNotifier();
}

void CDnMissionTask::OnAchieveDailyMission( MissionInfoStruct *pStruct )
{
	AlarmQueueStruct AlarmStruct( ( m_VecAchieveAlarmQueueList.empty() ) ? 0.f : 5.f, pStruct ); 
	m_VecAchieveAlarmQueueList.push_back( AlarmStruct );

	if( CDnActor::s_hLocalActor ) {
		if( !m_hAchieveAlarmEffect ) {
			m_hAchieveAlarmEffect = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
			if( m_hAchieveAlarmEffect ) {
				m_hAchieveAlarmEffect->SetPosition( *CDnActor::s_hLocalActor->GetPosition() );
				m_hAchieveAlarmEffect->SetActionQueue( "MissionAchieve" );
			}
		}
	}
#ifdef PRE_MOD_MISSION_HELPER
	bool bRemoveNotifier = true;
	if( pStruct->nType-1 == GuildWar ||
		pStruct->nType-1 == GuildCommon ||
		pStruct->nType-1 == WeekendRepeat )
		bRemoveNotifier = false;
	if( bRemoveNotifier ) RemoveNotifier( GetNotifierType( pStruct ), pStruct->nNotifierID );
#else
	RemoveNotifier( pStruct->nNotifierID );
#endif
	RefreshNotifier();
}


#ifdef PRE_ADD_ACTIVEMISSION
void CDnMissionTask::OnGainActiveMission( char * pData )
{
	int acMissionID = reinterpret_cast< SCActiveMissionGain * >( pData )->nIndex;
	
	// AlarmDlg.
	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TACTIVEMISSION );
	if( !pTable || ( pTable && !pTable->IsExistItem(acMissionID) ) )
		return;

	DNTableCell * pCell = pTable->GetFieldFromLablePtr( acMissionID, "_TitleNameID" );
	if( !pCell )	
		return;

	std::wstring str;
	int destID = pCell->GetInteger();

	pCell = pTable->GetFieldFromLablePtr( acMissionID, "_TitleNameIDParam" );
	if( pCell )
	{
		str = ParseUIParamString( destID, pCell->GetString() );
		GetInterface().ShowActiveAlarmDlg( true, str, 3.0f );
	}

	// Add icon.
	GetInterface().RefreshActiveMissionEventMark( acMissionID, str, true, true );
}

void CDnMissionTask::OnAchieveActiveMission( char * pData )
{
	int acMissionID = reinterpret_cast< SCActiveMissionAchieve * >( pData )->nIndex;

	// Create mission info as an active mission and insert it into the existing mission completion alarm process.
	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TACTIVEMISSION );
	if( !pTable || ( pTable && !pTable->IsExistItem(acMissionID) ) )
		return;
	
	MissionInfoStruct * pStruct = new MissionInfoStruct;
	pStruct->nArrayIndex = ACTIVEMISSIONID;

	DNTableCell * pCell = pTable->GetFieldFromLablePtr( acMissionID, "_AchieveIcon" );
	if( !pCell )
	{
		SAFE_DELETE( pStruct );
		return;
	}
	pStruct->nAchieveIcon = pCell->GetInteger();

	pCell = pTable->GetFieldFromLablePtr( acMissionID, "_TitleNameID" );
	if( !pCell )
	{
		SAFE_DELETE( pStruct );
		return;
	}
	
	std::wstring str;
	int destID = pCell->GetInteger();

	pCell = pTable->GetFieldFromLablePtr( acMissionID, "_TitleNameIDParam" );
	if( pCell )
		str = ParseUIParamString( destID, pCell->GetString() );
	else
		str = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, destID );

	pStruct->bAchieve = true;
	pStruct->szTitle = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9184 ); // "Active Mission Accomplished!";
	pStruct->szSubTitle = str;


	AlarmQueueStruct AlarmStruct( ( m_VecAchieveAlarmQueueList.empty() ) ? 0.f : 5.f, pStruct ); 
	m_VecAchieveAlarmQueueList.push_back( AlarmStruct );

	if( CDnActor::s_hLocalActor )
	{
		if( !m_hAchieveAlarmEffect )
		{
			m_hAchieveAlarmEffect = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
			if( m_hAchieveAlarmEffect )
			{
				m_hAchieveAlarmEffect->SetPosition( *CDnActor::s_hLocalActor->GetPosition() );
				m_hAchieveAlarmEffect->SetActionQueue( "MissionAchieve" );
			}
		}
	}
}
#endif // PRE_ADD_ACTIVEMISSION


DWORD CDnMissionTask::GetMissionCount()
{
	return (DWORD)m_pVecMissionList.size();
}

CDnMissionTask::MissionInfoStruct *CDnMissionTask::GetMissionInfo( int nIndex )
{
	if( nIndex < 0 || nIndex >= (int)m_pVecMissionList.size() ) return NULL;
	return m_pVecMissionList[nIndex];
}

#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL
CDnMissionTask::MissionInfoStruct *CDnMissionTask::GetAllMission( int nIndex )
{
	std::map<int, MissionInfoStruct *>::iterator it = m_nMapSearch.find(nIndex);
	if( it == m_nMapSearch.end() ) return NULL;
	return it->second;
}

DWORD CDnMissionTask::GetAchieveMissionCount()
{
	return (DWORD)m_nMapSearch.size();
}
#endif

CDnMissionTask::MissionInfoStruct *CDnMissionTask::GetMissionFromArrayIndex( int nArrayIndex )
{
	std::map<int, MissionInfoStruct *>::iterator it = m_nMapSearch.find(nArrayIndex);
	if( it == m_nMapSearch.end() ) return NULL;
	return it->second;
}

CDnMissionTask::MissionInfoStruct *CDnMissionTask::AddMission( int nArrayIndex )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMISSION );
	DNTableFileFormat* pCashCommodity = GetDNTable( CDnTableDB::TCASHCOMMODITY );
	int nItemID = pSox->GetItemID( nArrayIndex );
	if( nItemID == -1 ) return NULL;

	// Check and skip types other than normal missions.
	int nType = pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger();
	switch( nType ) {
		case MissionType_MetaHide:
		case MissionType_Quest:
		case MissionType_EventPopUp:
		case MissionType_HelpAlarm:
			return NULL;
			break;
	}

	MissionInfoStruct *pStruct = new MissionInfoStruct;
	pStruct->nArrayIndex = nArrayIndex;
	pStruct->nNotifierID = nItemID;
	pStruct->MainCategory = (MainCategoryEnum)pSox->GetFieldFromLablePtr( nItemID, "_MainCategory" )->GetInteger();
	pStruct->szSubCategory = pSox->GetFieldFromLablePtr( nItemID, "_SubCategoryID" )->GetString();
	int nNameID = pSox->GetFieldFromLablePtr( nItemID, "_TitleNameID" )->GetInteger();
	char *szParam = pSox->GetFieldFromLablePtr( nItemID, "_TitleNameIDParam" )->GetString();
	MakeUIStringUseVariableParam( pStruct->szTitle, nNameID, szParam );
	nNameID = pSox->GetFieldFromLablePtr( nItemID, "_SubTitleNameID" )->GetInteger();
	szParam = pSox->GetFieldFromLablePtr( nItemID, "_SubTitleNameIDParam" )->GetString();
	MakeUIStringUseVariableParam( pStruct->szSubTitle, nNameID, szParam );
	pStruct->nIcon = pSox->GetFieldFromLablePtr( nItemID, "_Icon" )->GetInteger();
	pStruct->nAchieveIcon = pSox->GetFieldFromLablePtr( nItemID, "_AchieveIcon" )->GetInteger();
	int nMailID = pSox->GetFieldFromLablePtr( nItemID, "_MailID" )->GetInteger();
	for( int i=0; i<RewardItem_Amount; i++ ) {
		pStruct->nRewardItemID[i] = CDnTableDB::GetInstance().GetMailInfo( nMailID, CDnTableDB::MailInfoRewardItemID, false, i );
		pStruct->nRewardItemCount[i] = CDnTableDB::GetInstance().GetMailInfo( nMailID, CDnTableDB::MailInfoRewardItemCount, false, i );
	}
	for( int i=0; i<RewardItem_Amount; i++ ) {
		pStruct->nRewardCashItemSN[i] = CDnTableDB::GetInstance().GetMailInfo( nMailID, CDnTableDB::MailInfoRewardItemID, true, i );
		
		// Cash item count is specified in the cashcommodity table.
		if( 0 < pStruct->nRewardCashItemSN[i] )
			pStruct->nRewardCashItemCount[i] = pCashCommodity->GetFieldFromLablePtr( pStruct->nRewardCashItemSN[i], "_Count" )->GetInteger();
		else
			pStruct->nRewardCashItemCount[i] = 0;
	}
	pStruct->nRewardCoin = CDnTableDB::GetInstance().GetMailInfo( nMailID, CDnTableDB::MailInfoRewardCoin );
	pStruct->nRewardAppellationID = pSox->GetFieldFromLablePtr( nItemID, "_RewardAppellation" )->GetInteger();
	pStruct->nRewardPoint = pSox->GetFieldFromLablePtr( nItemID, "_RewardPoint" )->GetInteger();
	pStruct->nCounter = 0;
	pStruct->nAchieveSystemMsgID = pSox->GetFieldFromLablePtr( nItemID, "_SystemMsg" )->GetInteger();
	pStruct->bAchieve = false;
	pStruct->nType = pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger();

	// Because they must be inserted in order..
	bool bInsert = false;
	for( DWORD i=0; i<m_pVecMissionList.size(); i++ ) {
		if( m_pVecMissionList[i]->nArrayIndex > nArrayIndex ) {
			bInsert = true;
			m_pVecMissionList.insert( m_pVecMissionList.begin() + i, pStruct );
			break;
		}
	}
	if( !bInsert ) m_pVecMissionList.push_back( pStruct );

	m_nMapSearch.insert( make_pair( nArrayIndex, pStruct ) );

#ifdef PRE_ADD_STAMPSYSTEM
	m_mapMission.insert( make_pair( nItemID, pStruct) );
#endif // PRE_ADD_STAMPSYSTEM

	return pStruct;
}

CDnMissionTask::MissionInfoStruct *CDnMissionTask::AddDailyMission( DailyMissionTypeEnum Type, int nArrayIndex, int nItemID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDAILYMISSION );
	DNTableFileFormat* pCashCommodity = GetDNTable( CDnTableDB::TCASHCOMMODITY );
	if( !pSox->IsExistItem( nItemID ) ) return NULL;

	MissionInfoStruct *pStruct = new MissionInfoStruct;	
	pStruct->nArrayIndex = -1;
	pStruct->nNotifierID = nItemID;
	pStruct->MainCategory = (MainCategoryEnum)-1;
	int nNameID = pSox->GetFieldFromLablePtr( nItemID, "_TitleNameID" )->GetInteger();
	char *szParam = pSox->GetFieldFromLablePtr( nItemID, "_TitleNameIDParam" )->GetString();
	MakeUIStringUseVariableParam( pStruct->szTitle, nNameID, szParam );
	nNameID = pSox->GetFieldFromLablePtr( nItemID, "_SubTitleNameID" )->GetInteger();
	szParam = pSox->GetFieldFromLablePtr( nItemID, "_SubTitleNameIDParam" )->GetString();
	MakeUIStringUseVariableParam( pStruct->szSubTitle, nNameID, szParam );
	pStruct->nIcon = pSox->GetFieldFromLablePtr( nItemID, "_Icon" )->GetInteger();
	pStruct->nAchieveIcon = pSox->GetFieldFromLablePtr( nItemID, "_AchieveIcon" )->GetInteger();
	int nMailID = pSox->GetFieldFromLablePtr( nItemID, "_MailID" )->GetInteger();
	for( int i=0; i<RewardItem_Amount; i++ ) {
		pStruct->nRewardItemID[i] = CDnTableDB::GetInstance().GetMailInfo( nMailID, CDnTableDB::MailInfoRewardItemID, false, i );
		pStruct->nRewardItemCount[i] = CDnTableDB::GetInstance().GetMailInfo( nMailID, CDnTableDB::MailInfoRewardItemCount, false, i );
	}
	for( int i=0; i<RewardItem_Amount; i++ ) {
		pStruct->nRewardCashItemSN[i] = CDnTableDB::GetInstance().GetMailInfo( nMailID, CDnTableDB::MailInfoRewardItemID, true, i );
		
		// Cash item count is specified in the cashcommodity table.
		if( 0 < pStruct->nRewardCashItemSN[i] )
			pStruct->nRewardCashItemCount[i] = pCashCommodity->GetFieldFromLablePtr( pStruct->nRewardCashItemSN[i], "_Count" )->GetInteger();
		else
			pStruct->nRewardCashItemCount[i] = 0;
	}
	pStruct->nRewardCoin = CDnTableDB::GetInstance().GetMailInfo( nMailID, CDnTableDB::MailInfoRewardCoin );
	pStruct->nRewardAppellationID = pSox->GetFieldFromLablePtr( nItemID, "_RewardAppellation" )->GetInteger();
	pStruct->nRewardPoint = pSox->GetFieldFromLablePtr( nItemID, "_RewardPoint" )->GetInteger();
	pStruct->nRewardCounter = pSox->GetFieldFromLablePtr( nItemID, "_CounterParam" )->GetInteger();
	pStruct->nType = pSox->GetFieldFromLablePtr( nItemID, "_type" )->GetInteger();
	pStruct->nCounter = 0;
	pStruct->nAchieveSystemMsgID = 0;
	pStruct->bAchieve = false;

	m_pVecDailyMissionList[Type].push_back( pStruct );

	m_nMapDailySearch[Type].insert( make_pair( nArrayIndex, pStruct ) );

	return pStruct;
}

int CDnMissionTask::GetLastUpdateAchieveIndex( int nIndex )
{
	if( nIndex < 0 || nIndex >= LASTMISSIONACHIEVEMAX ) return -1;
	return m_nLastUpdateAchieveList[nIndex];
}

void CDnMissionTask::UpdateLastAchieveList( int nArrayIndex )
{
	for( int i=LASTMISSIONACHIEVEMAX-1; i>0; i-- ) {
		m_nLastUpdateAchieveList[i] = m_nLastUpdateAchieveList[i-1];
	}
	m_nLastUpdateAchieveList[0] = nArrayIndex;
}

DWORD CDnMissionTask::GetDailyMissionCount( DailyMissionTypeEnum Type )
{
	return (DWORD)m_pVecDailyMissionList[Type].size();
}

CDnMissionTask::MissionInfoStruct *CDnMissionTask::GetDailyMissionInfo( DailyMissionTypeEnum Type, int nIndex )
{
	if( nIndex < 0 || nIndex >= (int)m_pVecDailyMissionList[Type].size() ) return NULL;
	return m_pVecDailyMissionList[Type][nIndex];
}

#ifdef PRE_MOD_MISSION_HELPER

#ifdef PRE_ADD_STAMPSYSTEM
bool CDnMissionTask::GetMissionInfoByID( MissionInfoStruct *& rMission, int itemID )
{
	std::map<int, MissionInfoStruct *>::iterator it = m_mapMission.find( itemID );
	if( it == m_mapMission.end() )
		return false;

	rMission = it->second;
	
	return true;
}

// When all missions are registered -
// : Add immediately if there is an empty slot; if full, shift registered missions up by one and add the new mission in the last slot.
bool CDnMissionTask::PushBack( DNNotifier::Type::eType NotifierType, int nIndex, bool bInit )
{	
	// Full.
	if( IsMissionNotifierFull() )
	{		
		// Before removing missions, copy [2nd mission] ~ [last mission].
		std::vector<CDnMissionTask::MissionInfoStruct *> arrTemp;
		for( int i=1; i<DNNotifier::RegisterCount::TotalMission; ++i )
		{
			CDnMissionTask::MissionInfoStruct * pMissionStruct = GetMissionTask().GetMissionInfoForNotifier( i );
			CDnMissionTask::MissionInfoStruct * pTemp = new CDnMissionTask::MissionInfoStruct();
			pTemp->Clone( pMissionStruct );
			arrTemp.push_back( pTemp );
		}

		// Remove all missions.
		for( int i=0; i<DNNotifier::RegisterCount::TotalMission; ++i )
		{			
			CDnMissionTask::MissionInfoStruct * pMissionStruct = GetMissionTask().GetMissionInfoForNotifier( i );
			DNNotifier::Type::eType _type = GetNotifierType( pMissionStruct );
			if( !RemoveNotifier( _type, pMissionStruct->nNotifierID ) )
			{
				// Error! - Remove copied mission.
				int size = (int)arrTemp.size();
				for( int k=i; k<size; ++k )
					delete arrTemp[ k ];
				arrTemp.clear();

				return false;
			}
		}

		// Register copied mission - moves up one slot.
		int size = (int)arrTemp.size();
		for( int i=0; i<size; ++i )
		{
			CDnMissionTask::MissionInfoStruct * pMissionStruct = arrTemp[ i ];
			if( !RegisterNotifier( GetNotifierType( pMissionStruct ), pMissionStruct->nNotifierID ) )
			{
				// Error! - Remove copied mission.
				int size = (int)arrTemp.size();
				for( int k=i; k<size; ++k )
					delete arrTemp[ k ];
				arrTemp.clear();
			}
		}

		// Register new mission.
		if( !RegisterNotifier( NotifierType, nIndex, bInit ) )
		{
			// Error! - Remove mission array.
			arrTemp.clear();
			return false;
		}

		arrTemp.clear();
	
	}

	// Empty slot available, just add.
	else
	{
		if( !RegisterNotifier( NotifierType, nIndex, bInit ) )
			return false;
	}

	return true;
}
#endif // PRE_ADD_STAMPSYSTEM

bool CDnMissionTask::RegisterNotifier( DNNotifier::Type::eType NotifierType, int nIndex, bool bInit )
{
	ASSERT( nIndex != -1 );

#ifdef PRE_ADD_MONTHLY_MISSION
	ASSERT( NotifierType >= DNNotifier::Type::NormalMission && NotifierType <= DNNotifier::Type::LongTermMission );
#else
	ASSERT( NotifierType >= DNNotifier::Type::NormalMission && NotifierType <= DNNotifier::Type::WeekendRepeatMission );
#endif

	if( bInit )
	{
#ifdef PRE_ADD_MONTHLY_MISSION
		if( NotifierType >= DNNotifier::Type::DailyMission && NotifierType <= DNNotifier::Type::LongTermMission )
#else
		if( NotifierType >= DNNotifier::Type::DailyMission && NotifierType <= DNNotifier::Type::WeekendRepeatMission )
#endif 
		{
			// All daily missions except normal missions may disappear on reconnect after being pinned in the notifier.
			// Since the daily mission list arrives first, check and register using it.
			int nCount = GetDailyMissionCount( ToDailyMissionType( NotifierType ) );
			bool bExistMission = false;
			for( int i = 0; i < nCount; i++ ) {
				MissionInfoStruct *pStruct = GetDailyMissionInfo( ToDailyMissionType( NotifierType ), i );
				if( !pStruct ) continue;
				if( pStruct->nNotifierID == nIndex )
				{
					bExistMission = true;
					break;
				}
			}
			if( !bExistMission )
				return false;
		}
	}

	// Check if the same item is already registered
	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
		if( m_NotifierData[ i ].IsEqual( NotifierType, nIndex ) ) {
			return false;
		}
	}
	// Check if there is an empty slot to insert into
	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
		ASSERT( m_NotifierData[ i ].iIndex != 0 );
		if( m_NotifierData[ i ].iIndex == -1 ) {
			ASSERT( nIndex != 0 );
			m_NotifierData[ i ].Register( NotifierType, nIndex );
			if( !bInit )
			{
				SendMissionNotifier( m_NotifierData );
				GetQuestTask().RefreshQuestNotifierInfo();
			}
			return true;
		}
	}
	return false;
}

bool CDnMissionTask::RemoveNotifier( DNNotifier::Type::eType NotifierType, int nIndex )
{
	ASSERT( nIndex != -1 );
#ifdef PRE_ADD_MONTHLY_MISSION
	ASSERT( NotifierType >= DNNotifier::Type::NormalMission && NotifierType <= DNNotifier::Type::LongTermMission );
#else
	ASSERT( NotifierType >= DNNotifier::Type::NormalMission && NotifierType <= DNNotifier::Type::WeekendRepeatMission );
#endif 

	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++) 
	{
		if( m_NotifierData[ i ].IsEqual( NotifierType, nIndex ) ) 
		{
			m_NotifierData[i].Clear();
			SendMissionNotifier( m_NotifierData );
			GetQuestTask().RefreshQuestNotifierInfo();
			return true;
		}
	}
	return false;
}

bool CDnMissionTask::IsRegisterNotifier( DNNotifier::Type::eType NotifierType, int nIndex )
{
	// Check if the same item is already registered
	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
		if( m_NotifierData[ i ].IsEqual( NotifierType, nIndex ) ) {	
			return true;
		}
	}
	return false;
}

CDnMissionTask::MissionInfoStruct * CDnMissionTask::GetMissionInfoForNotifier( int nIndex )
{
	if ( nIndex < 0 || nIndex >= DNNotifier::RegisterCount::TotalMission )
	{
		return NULL;
	}

	if( m_NotifierData[ nIndex ].IsEmpty() || m_NotifierData[ nIndex ].iIndex == -1 )
	{
		return NULL;
	}

	if( m_NotifierData[ nIndex ].eType == DNNotifier::Type::NormalMission )
	{
		for( DWORD i=0; i<GetMissionCount(); i++ ) {
			MissionInfoStruct *pStruct = GetMissionInfo(i);
			if( !pStruct ) continue;
			if( pStruct->nNotifierID == m_NotifierData[ nIndex ].iIndex )
				return pStruct;
		}
	}
#ifdef PRE_ADD_MONTHLY_MISSION
	else if( m_NotifierData[ nIndex ].eType >= DNNotifier::Type::DailyMission && m_NotifierData[ nIndex ].eType <= DNNotifier::Type::LongTermMission )
#else
	else if( m_NotifierData[ nIndex ].eType >= DNNotifier::Type::DailyMission && m_NotifierData[ nIndex ].eType <= DNNotifier::Type::WeekendRepeatMission )
#endif 
	{
		int nCount = GetDailyMissionCount( ToDailyMissionType( m_NotifierData[ nIndex ].eType ) );
		for( int i = 0; i < nCount; i++ ) {
			MissionInfoStruct *pStruct = GetDailyMissionInfo( ToDailyMissionType( m_NotifierData[ nIndex ].eType ), i );
			if( !pStruct ) continue;
			if( pStruct->nNotifierID == m_NotifierData[ nIndex ].iIndex )
				return pStruct;
		}
	}
	return NULL;
}

DNNotifier::Type::eType CDnMissionTask::ToNotifierType( DailyMissionTypeEnum DailyMissionType )
{
	return (DNNotifier::Type::eType)(DNNotifier::Type::DailyMission + DailyMissionType);
}

CDnMissionTask::DailyMissionTypeEnum CDnMissionTask::ToDailyMissionType( DNNotifier::Type::eType Type )
{
	return (CDnMissionTask::DailyMissionTypeEnum)(Type - DNNotifier::Type::DailyMission);
}

DNNotifier::Type::eType CDnMissionTask::GetNotifierType( MissionInfoStruct *pStruct )
{
	if( !pStruct ) return DNNotifier::Type::DailyMission;
	if( pStruct->nArrayIndex != -1 ) return DNNotifier::Type::NormalMission;
	return ToNotifierType( (DailyMissionTypeEnum)(pStruct->nType-1) );
}

bool CDnMissionTask::IsMissionNotifierFull()
{
	bool bFull = true;
	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ )
	{
		CDnMissionTask::MissionInfoStruct *pMissionStruct = GetMissionTask().GetMissionInfoForNotifier( i );
		if( !pMissionStruct ) bFull = false;
	}
	return bFull;
}

void CDnMissionTask::AddDungeonExpectMission( MissionInfoStruct *pStruct )
{
	m_pVecDungeonExpectMissionList.push_back( pStruct );
}

void CDnMissionTask::ClearDungeonExpectMission()
{
	m_pVecDungeonExpectMissionList.clear();
}

void CDnMissionTask::AutoRegisterMissionNotifier()
{
	if( m_pVecDungeonExpectMissionList.empty() )
		return;

#ifdef PRE_ADD_MISSION_HELPER_AUTO_REGISTER_LEVEL
	int nLimitLevel = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MissionHelper_AutoRegisterLevel );
	if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetLevel() < nLimitLevel )
		return;
#endif

	MissionInfoStruct *pStruct = NULL;

	bool bAutoRegister = false;
	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
		if( m_NotifierData[ i ].iIndex == -1 ) {
			for( int j = 0; j < (int)m_pVecDungeonExpectMissionList.size(); ++j ) {
				pStruct = m_pVecDungeonExpectMissionList[j];
				if( IsRegisterNotifier( GetNotifierType( pStruct ), pStruct->nNotifierID ) )
					continue;
				m_NotifierData[ i ].Register( GetNotifierType( pStruct ), pStruct->nNotifierID );
				bAutoRegister = true;
				break;
			}
		}
	}

	if( bAutoRegister ) {
		SendMissionNotifier( m_NotifierData );
		GetQuestTask().RefreshQuestNotifierInfo();
	}
	m_pVecDungeonExpectMissionList.clear();
}
#else
void CDnMissionTask::RegisterNotifier( DailyMissionTypeEnum Type, int nIndex, bool bInit/*=false*/ )
{
	StaticAssert( DNNotifier::RegisterCount::TotalMission ==  _countof(m_NotifierData) );
	switch( Type ) {
		case Daily:
			if( !m_NotifierData[ Daily ].IsEqual( DNNotifier::Type::DailyMission, nIndex ) ) {
				m_NotifierData[ Daily ].Register( DNNotifier::Type::DailyMission, nIndex );
				if( !bInit )
				{
					SendMissionNotifier( m_NotifierData );
					GetQuestTask().RefreshQuestNotifierInfo();
				}
			}
			break;
		case Weekly:
			if( !m_NotifierData[ Weekly ].IsEqual( DNNotifier::Type::WeeklyMission, nIndex ) ) {
				m_NotifierData[ Weekly ].Register( DNNotifier::Type::WeeklyMission, nIndex );
				if( !bInit )
				{
					SendMissionNotifier( m_NotifierData );
					GetQuestTask().RefreshQuestNotifierInfo();
				}
			}
			break;
	}	
}

void CDnMissionTask::RemoveNotifier( int nIndex )
{
	if( m_NotifierData[ Daily ].IsEqual( DNNotifier::Type::DailyMission, nIndex ) ) 
	{
		m_NotifierData[ Daily ].Clear();
		SendMissionNotifier( m_NotifierData );
		GetQuestTask().RefreshQuestNotifierInfo();
	}
	else if( m_NotifierData[ Weekly ].IsEqual( DNNotifier::Type::WeeklyMission, nIndex ) ) 
	{
		m_NotifierData[ Weekly ].Clear();
		SendMissionNotifier( m_NotifierData );
		GetQuestTask().RefreshQuestNotifierInfo();
	}
}

bool CDnMissionTask::IsRegisterNotifier( DailyMissionTypeEnum Type, int nIndex )
{
	switch( Type ) {
		case Daily:
			if(m_NotifierData[ Daily ].IsEqual( DNNotifier::Type::DailyMission, nIndex ) ) {
				return true;
			}
			break;
		case Weekly:
			if(m_NotifierData[ Weekly ].IsEqual( DNNotifier::Type::WeeklyMission, nIndex ) ) {
				return true;
			}
			break;
	}	
	return false;
}

CDnMissionTask::MissionInfoStruct * CDnMissionTask::GetMissionInfoForNotifier( DailyMissionTypeEnum Type )
{
	int nCount = GetDailyMissionCount( Type );
	for( int i = 0; i < nCount; i++) {
		CDnMissionTask::MissionInfoStruct *pMissionStruct = GetDailyMissionInfo( Type, i );
		if( NULL != pMissionStruct && pMissionStruct->nNotifierID == m_NotifierData[ Type ].iIndex ) {
			return pMissionStruct;			
		}
	}	
	return NULL;
}
#endif

void CDnMissionTask::RefreshNotifier()
{
	CDnQuestSummaryInfoDlg *pDlg = (CDnQuestSummaryInfoDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::NOTIFIER_DIALOG );
	if( pDlg ) pDlg->RefreshNotifier();
}

#if defined( PRE_ADD_CHAT_MISSION )
void CDnMissionTask::InitChatMission()
{
	m_vChatMission.clear();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMISSIONTYPING );
	if( NULL == pSox )
		return;

	int nTableCount = pSox->GetItemCount();
	for( int itr = 0; itr < nTableCount; ++itr )
	{
		int nItemID = pSox->GetItemID( itr );

		int nStringID = pSox->GetFieldFromLablePtr( nItemID, "_UIString" )->GetInteger();
		std::wstring wszString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID );

		m_vChatMission.push_back( std::pair< int, std::wstring >(nStringID, wszString) );
	}
}

void CDnMissionTask::RequestChatMission( std::wstring & wszChat )
{
	int nStringID = 0;
	for( DWORD itr = 0; itr < m_vChatMission.size(); ++itr )
	{
		if( wszChat == m_vChatMission[itr].second )
		{
			nStringID = m_vChatMission[itr].first;
			break;
		}
	}

	if( 0 == nStringID )
		return;

	SendChatMission( nStringID );
}
#endif	// #if defined( PRE_ADD_CHAT_MISSION )