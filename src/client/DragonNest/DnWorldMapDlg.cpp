#include "StdAfx.h"
#include "DnWorldMapDlg.h"
#include "DnMainMenuDlg.h"
#include "DnActorState.h"
#include "DnInterface.h"
#include "DnVillageTask.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "DnTableDB.h"
#include "DnPlayerActor.h"
#include "DnQuestTask.h"
#include "DnWorldData.h"
#include "DnZoneMapDlg.h"
#include "DnMainDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnWorldMapDlg::CDnWorldMapDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pCurIcon(NULL)
	, m_nCurMapIndex(0)
	, m_nTraceQuest( -1 )
{
	for(int i=0; i<CLASSKINDMAX; i++ )
		m_pStaticIcon[i] = NULL;
	m_pQuestTraceButton = NULL;
}

CDnWorldMapDlg::~CDnWorldMapDlg(void)
{
}

void CDnWorldMapDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "WorldMapDlg.ui" ).c_str(), bShow );
}

void CDnWorldMapDlg::InitialUpdate()
{
	m_pStaticIcon[CDnActorState::Warrior] = GetControl<CEtUIStatic>("ID_ICON_WARRIOR");
	m_pStaticIcon[CDnActorState::Archer] = GetControl<CEtUIStatic>("ID_ICON_ARCHER");
	m_pStaticIcon[CDnActorState::Soceress] = GetControl<CEtUIStatic>("ID_ICON_SOCERESS");
	m_pStaticIcon[CDnActorState::Cleric] = GetControl<CEtUIStatic>("ID_ICON_CLERIC");
#ifdef PRE_ADD_ACADEMIC
	m_pStaticIcon[CDnActorState::Academic] = GetControl<CEtUIStatic>("ID_ICON_ACADEMIC"); // #35786 Added Academic
#endif
#if defined( PRE_ADD_KALI ) && !defined( PRE_REMOVE_KALI )
	m_pStaticIcon[CDnActorState::Kali] = GetControl<CEtUIStatic>("ID_ICON_KALI");
#endif

	int i;
	for( i=0; i<CLASSKINDMAX; i++ )
	{
		if(m_pStaticIcon[i])
			m_pStaticIcon[i]->Show(false);
	}

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TWORLDMAPUI );

	int nCount = pSox->GetItemCount();
	for( i = 0; i < nCount; i++) {
		int nItemID = pSox->GetItemID( i );
		int nActiveLevel = pSox->GetFieldFromLablePtr( nItemID, "_Alv")->GetInteger();
		int nEnableLevel = pSox->GetFieldFromLablePtr( nItemID, "_Elv")->GetInteger();

		char *szMapButton = pSox->GetFieldFromLablePtr( nItemID, "_MapBtn")->GetString();
		char *szRoadImage = pSox->GetFieldFromLablePtr( nItemID, "_RoadImg")->GetString();
		char *szBridgeImage = pSox->GetFieldFromLablePtr( nItemID, "_BridgeImg")->GetString();
		char *szFogImage = pSox->GetFieldFromLablePtr( nItemID, "_FogImg")->GetString();

		if( szMapButton && szMapButton[0] != '\0') 
			m_vecButton.push_back( boost::make_tuple( GetControl<CDnWorldMapButton>( szMapButton ), nActiveLevel , nEnableLevel) );

		if( szRoadImage && szRoadImage[0] != '\0' ) 
			m_vecRoadImage.push_back( std::make_pair( GetControl<CEtUIStatic>( szRoadImage ), nActiveLevel ) );

		if( szBridgeImage && szBridgeImage[0] != '\0' ) 
			m_vecBridgeImage.push_back( std::make_pair( GetControl<CEtUIStatic>( szBridgeImage ), nActiveLevel ) );

		if( szFogImage && szFogImage[0] != '\0' ) 
			m_vecFogImage.push_back( std::make_pair( GetControl<CEtUIStatic>( szFogImage ), nActiveLevel ) );

	}

	m_pQuestTraceButton = GetControl<CEtUIButton>( "ID_BUTTON_QUESTPOINT" );
	MoveToTail( m_pQuestTraceButton );

	UpdateButton();	//Bug #11098, pressing world map on initial login briefly shows the full map
}

void CDnWorldMapDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( strstr( pControl->GetControlName(), "ID_BUTTON_" ) )
		{
			CDnWorldMapButton *pButton = (CDnWorldMapButton*)pControl;
			if( pButton->IsButtonOpened() )
			{
				CDnMainMenuDlg *pMainMenuDlg = (CDnMainMenuDlg*)GetParentDialog();
				if( pMainMenuDlg )	pMainMenuDlg->ShowZoneMapDialog( true, pButton->GetButtonID() );
			}
			return;
		}
		if( IsCmdControl( "ID_CLOSE" ) ) {
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnWorldMapDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		UpdateButton();
		UpdatePlayerIcon();
		UpdateTraceQuest();
	}
	else
	{
		m_pCurIcon = NULL;
		m_fElapsedTime = 0.0f;
		m_nTraceQuest = -1;

		for each( boost::tuple< CDnWorldMapButton* , int , int> element in m_vecButton ) {
			CDnWorldMapButton *pButton = boost::get<0>(element);
			pButton->SetBlink( false );
		}
	}

	CEtUIDialog::Show( bShow );
//	CDnMouseCursor::GetInstance().ShowCursor( bShow );
}

void CDnWorldMapDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( m_pCurIcon )
	{
		if( m_fElapsedTime < 0.0f )
		{
			bool bShow = m_pCurIcon->IsShow();
			m_pCurIcon->Show( !bShow );

			if( bShow )
			{
				m_fElapsedTime = 0.3f;
			}
			else
			{
				m_fElapsedTime = 0.5f;
			}
		}
		else
		{
			m_fElapsedTime -= fElapsedTime;
		}
	}
}

bool CDnWorldMapDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_RBUTTONDOWN )
	{
		POINT MousePoint;
		float fMouseX, fMouseY;

		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		PointToFloat( MousePoint, fMouseX, fMouseY );

		if( m_BaseDlgCoord.IsInside( fMouseX, fMouseY ) )
		{
			CDnMainMenuDlg *pMainMenuDlg = (CDnMainMenuDlg*)GetParentDialog();
			if( pMainMenuDlg )	pMainMenuDlg->ShowZoneMapDialog( true, CDnZoneMapDlg::PREV_ZONE );
			CONTROL( Button, ID_CLOSE )->PlayButtonSound();
		}
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnWorldMapDlg::UpdateTraceQuest()
{
	m_nTraceQuest = GetQuestTask().GetTraceQuestID();

	m_pQuestTraceButton->Show( false );

	if( -1 != m_nTraceQuest )
	{
		CDnQuestTask::QuestNotifierInfo* pInfo = GetQuestTask().GetQuestNotifierInfoByQuestIDForce( m_nTraceQuest );

		if( !pInfo ) return;
		if( pInfo->pathResult.empty() ) return;

		CDnWorldMapButton *pButton = NULL;

		PathResult::reverse_iterator itor = pInfo->pathResult.rbegin();
		for( ; itor != pInfo->pathResult.rend(); ++itor )
		{
			pButton = FindButton( itor->nMapIndex );

			if( pButton )
				break;
		}

		if( pButton )
		{
			SUICoord buttonCoord, iconCoord;
			pButton->GetUICoord( buttonCoord );
			m_pQuestTraceButton->GetUICoord(iconCoord);

			iconCoord.fX = buttonCoord.fX + buttonCoord.fWidth/2 - iconCoord.fWidth/2;
			iconCoord.fY = buttonCoord.fY - buttonCoord.fHeight;

			m_pQuestTraceButton->SetPosition( iconCoord.fX, iconCoord.fY );
			m_pQuestTraceButton->Show(true);

			m_pQuestTraceButton->GetUICoord( m_sUICoord );

			m_fElapsedTime = 0.5f;

			return;
		}
	}
}

void CDnWorldMapDlg::ProcessTraceQuest()
{
	if( -1 != m_nTraceQuest && m_pQuestTraceButton->IsShow() )	
	{
		SUICoord sUICoord = m_pQuestTraceButton->GetUICoord();

		if( sUICoord.fY >= m_sUICoord.fY )
			sUICoord.fY = m_sUICoord.fY - (m_sUICoord.fHeight*1.5f);
		else
			sUICoord.fY += (GetTickCount() % 60) *0.00005f;

		m_pQuestTraceButton->SetPosition( sUICoord.fX, sUICoord.fY );
	}			
}

void CDnWorldMapDlg::UpdatePlayerIcon()
{
	for( int i=0; i<CLASSKINDMAX; i++ )
	{
		if(m_pStaticIcon[i])
			m_pStaticIcon[i]->Show(false);
	}

	m_nCurMapIndex = 0;
	if( GetInterface().GetInterfaceType() == CDnInterface::Village )
	{
		CDnVillageTask *pTask = (CDnVillageTask*)CTaskManager::GetInstance().GetTask( "VillageTask" );
		if( pTask ) 
		{
			m_nCurMapIndex = pTask->GetVillageMapIndex();
		}
	}
	else if( GetInterface().GetInterfaceType() == CDnInterface::Game || GetInterface().GetInterfaceType() == CDnInterface::DLGame )
	{
		CDnGameTask *pTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
		if( pTask ) 
		{
			m_nCurMapIndex = pTask->GetGameMapIndex();
		}
	}
	else
	{
		ASSERT(0&&"CDnWorldMapDlg::UpdatePlayerIcon");
	}

	if( m_nCurMapIndex == 0 ) return;
	CDnWorldMapButton *pButton = FindButton( m_nCurMapIndex );
	if( pButton )
	{
		if( !CDnActor::s_hLocalActor ) 
			return;

		DnActorHandle hActor = CDnActor::s_hLocalActor;
		int nClassID = hActor->GetClassID()-1;
		if(!m_pStaticIcon[nClassID])
			return;

		m_pCurIcon = m_pStaticIcon[nClassID];

		SUICoord buttonCoord, iconCoord;
		pButton->GetUICoord( buttonCoord );
		m_pCurIcon->GetUICoord(iconCoord);

		iconCoord.fX = ( buttonCoord.fX + buttonCoord.fWidth/4 ) - iconCoord.fWidth + 0.018f;
		iconCoord.fY = ( buttonCoord.fY + buttonCoord.fHeight/4 ) - iconCoord.fHeight + 0.018f;

		m_pCurIcon->SetPosition( iconCoord.fX, iconCoord.fY );
		m_pCurIcon->Show(true);

		m_fElapsedTime = 0.5f;
	}
}

CDnWorldMapButton *CDnWorldMapDlg::FindButton( int nMapIndex )
{
	for each( boost::tuple< CDnWorldMapButton* , int , int> element in m_vecButton ) {
		CDnWorldMapButton *pButton = boost::get<0>(element);
		if( pButton->GetButtonID() == nMapIndex )
		{
			return pButton;
		}
	}
	return NULL;
}

void CDnWorldMapDlg::UpdateButton()
{
	if( !CDnActor::s_hLocalActor ) 
		return;

	int nPlayerLevel = (CDnActor::s_hLocalActor)->GetLevel();

	CDnQuestTask::QuestNotifierInfo MainQuest;
	CDnQuestTask::QuestNotifierInfo DynamicQuest;
	CDnQuestTask::QuestNotifierInfo ActiveQuest;
	if( GetQuestTask().IsMainQuestNotifierExist()) MainQuest = GetQuestTask().GetNotifierInfo( 0 );
	if( GetQuestTask().GetRegisteredSubQuestCount() > 0 ) DynamicQuest = GetQuestTask().GetNotifierInfo( 1 );
	if( GetQuestTask().GetRegisteredSubQuestCount() > 1 ) ActiveQuest = GetQuestTask().GetNotifierInfo( 2 );

	for each( boost::tuple< CDnWorldMapButton* , int , int> element in m_vecButton ) {

		bool bShow = ( boost::get<1>(element) <= nPlayerLevel);
		bool bEnable = ( boost::get<2>(element) <= nPlayerLevel);

		CDnWorldMapButton *pButton = boost::get<0>(element);

		pButton->SetButtonState( bEnable );
		if( pButton->IsShow() != bShow ) {
			pButton->Show( bShow );
		}

		if( !bShow ) continue;

		pButton->SetButtonQuestType( CDnWorldMapButton::typeNoneQuestButton );

		if( !MainQuest.pathResult.empty() ) {
			if( pButton->GetButtonID() == MainQuest.pathResult.back().nMapIndex ) {
				pButton->SetButtonQuestType( CDnWorldMapButton::typeMainQuestButton );
				continue;
			}
		}	
		if( !DynamicQuest.pathResult.empty() ) {
			if( pButton->GetButtonID() == DynamicQuest.pathResult.back().nMapIndex ) {
				pButton->SetButtonQuestType( CDnWorldMapButton::typeDynamicQuestButton );
				continue;
			}
		}
		if( !ActiveQuest.pathResult.empty() ) {
			if( pButton->GetButtonID() == ActiveQuest.pathResult.back().nMapIndex ) {
				pButton->SetButtonQuestType( CDnWorldMapButton::typeActiveQuestButton );
				continue;
			}
		}
		
	}

	for each( std::pair< CEtUIStatic* , int > element in m_vecRoadImage ) {
		element.first->Show( element.second <= nPlayerLevel );
	}
	for each( std::pair< CEtUIStatic* , int > element in m_vecBridgeImage ) {		
		element.first->Show( element.second <= nPlayerLevel );
	}
	for each( std::pair< CEtUIStatic* , int > element in m_vecFogImage ) {		
		element.first->Show( !(element.second <= nPlayerLevel) );
	}
}

int CDnWorldMapDlg::GetLevelLimit( int nMapIndex )
{
	for each( boost::tuple< CDnWorldMapButton* , int , int> element in m_vecButton ) {
		CDnWorldMapButton *pButton = boost::get<0>(element);
		if( pButton->GetButtonID() == nMapIndex ) {
			int nLevelLimit = boost::get<2>(element);
			return nLevelLimit;
		}			
	}	
	return -1;
}

void CDnWorldMapDlg::OnLevelUp( int nPlayerLevel )
{
	bool bBlink = false;
	
	for each( boost::tuple< CDnWorldMapButton* , int , int> element in m_vecButton ) {		
		int nEnableLevel = boost::get<2>(element);
		if( nEnableLevel == nPlayerLevel ) {
			bBlink = true;
			CDnWorldMapButton *pButton = boost::get<0>(element);

			pButton->SetBlink( true );

			CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( pButton->GetButtonID() );
			if( !pWorldData ) continue;

			std::wstring wszMapName;
			pWorldData->GetMapName( wszMapName );

			wchar_t wzStr[255];
			swprintf_s(wzStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2410 ), wszMapName.c_str() );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wzStr, false );
			// 2606
		}		
	}

	/*
	if( bBlink ) {
		if( GetInterface().GetMainBarDialog() ) 
			GetInterface().GetMainBarDialog()->BlinkMenuButton( CDnMainMenuDlg::ZONEMAP_DIALOG );
	}

	if( CDnQuestTask::IsActive() ) {

		DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDUNGEONENTER );

		bool bNewDungeon = false;
		int nLastMapIndex = -1;
		int nCount = pSox->GetItemCount();
		for( int i = 0; i < nCount; i++) {			
			int nItemID = pSox->GetItemID( i );	
			int nDungeonLevel = pSox->GetFieldFromLablePtr( nItemID, "_LvlMin")->GetInteger();		
			if( nDungeonLevel <= nPlayerLevel ) {
				std::vector< int > vecList;
				pMapSox->GetItemIDListFromField( "_EnterConditionTableID",  nItemID, vecList );
				for each( int nMapIndex in vecList ) {
					PathResult pathResult;					
					bool bResult = GetQuestTask().GetPathFinder()->GetPath( CGlobalInfo::GetInstance().m_nCurrentMapIndex, nMapIndex, pathResult);
					if( bResult ) {
						int nMaxLevelLimit = -1;						
						for each( PathInfo path in pathResult ) {
							if( path.nMapIndex == nMapIndex ) continue;
							if( path.nMapIndex >= DUNGEONGATE_OFFSET ) continue;
							nLastMapIndex = path.nMapIndex;
							int nLevelLimit = GetLevelLimit( path.nMapIndex );
							if( nMaxLevelLimit < nLevelLimit ) {
								nMaxLevelLimit = nLevelLimit;
							}					
						}
						if( nMaxLevelLimit <= nPlayerLevel && nLastMapIndex != -1 ) {
							if( nDungeonLevel == nPlayerLevel || nMaxLevelLimit == nPlayerLevel ) {
								bNewDungeon = true;
								break;
							}							
						}
					}					
				}
				if( bNewDungeon) break;
			}
		}
		
		if( bNewDungeon ) {
			int nMapNameID = pMapSox->GetFieldFromLablePtr( nLastMapIndex, "_MapNameID")->GetInteger();
			wchar_t wzStr[255];
			swprintf_s(wzStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2606 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMapNameID ) );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wzStr, false );											
		}
	}
	*/
}
