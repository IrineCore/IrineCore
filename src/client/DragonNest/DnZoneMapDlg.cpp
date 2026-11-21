#include "StdAfx.h"
#include "DnZoneMapDlg.h"
#include "DnMinimap.h"
#include "DnActor.h"
#include "DnWorld.h"
#include "EtWorldEventArea.h"
#include "DnMainMenuDlg.h"
#include "DnNPCActor.h"
#include "DnWorldDataManager.h"
#include "DnWorldData.h"
#include "DnInterface.h"
#include "DnVillageTask.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnPartyTask.h"
#include "DnTableDB.h"
#include "DnLocalPlayerActor.h"
#include "DnCommonUtil.h"
#include "DnUIString.h"
#include "DnLifeSkillPlantTask.h"
#include "DnLifeConditionDlg.h"
#include "DnQuestTask.h"

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
#include "DnWorldZoneSelectListDlg.h"
#include "DnWorldZoneSelectDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnZoneMapDlg::CDnZoneMapDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pTextureMinimap(NULL)
	, m_pStaticPlayer(NULL)
	, m_pButtonToWorld(NULL)
	, m_pButtonCancelCheck(NULL)
	, m_pButtonGuild(NULL)
	, m_pButtonEnemy(NULL)
	, m_pButtonGhost(NULL)
	, m_bCurWorld(false)
	, m_nSetMapIndex(0)
	, m_nCurMapIndex(0)
	, m_pStaticMapName(NULL)
	, m_pStaticDungeonTitle(NULL)
	, m_pShowGateButton(NULL)
	, m_pStaticPlayerUp(NULL)
	, m_nTraceQuest( -1 )
	, m_pButtonCereals(NULL)
	, m_pButtonEmptyArea(NULL)
	, m_pButtonFruit(NULL)
	, m_pButtonFungus(NULL)
	, m_pButtonLeaf(NULL)
	, m_pButtonQuestTrace(NULL)
	, m_pButtonRoot(NULL)
	, m_fElapsedTime( 0.0f )
{
	std::fill( m_pButtonZoneNPC, m_pButtonZoneNPC + CDnNPCActor::typeAmount, (CDnZoneNPCButton*)NULL);
	std::fill( m_pButtonGate, m_pButtonGate + ZONE_GATE_AMOUNT, (CDnZoneGateButton*)NULL);

	int i = 0;
	for (; i < CLASSKINDMAX; ++i)
	{
		m_pButtonParty[i] = NULL;
	}
}

CDnZoneMapDlg::~CDnZoneMapDlg(void)
{
	SAFE_RELEASE_SPTR( m_hMinimap );
}

void CDnZoneMapDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ZoneMapDlg.ui" ).c_str(), bShow );
}

void CDnZoneMapDlg::InitialUpdate()
{
	m_pStaticMapName = GetControl<CEtUIStatic>("ID_STATIC_MAPNAME");
	m_pStaticPlayer = GetControl<CEtUIStatic>("ID_STATIC_PLAYER");
	m_pStaticPlayerUp = (CEtUIStatic*) CreateControl( m_pStaticPlayer->GetProperty() );

	m_pButtonParty[CommonUtil::eWARRIOR]	= GetControl<CEtUIButton>("ID_BUTTON_WARRIOR");
	m_pButtonParty[CommonUtil::eARCHER]		= GetControl<CEtUIButton>("ID_BUTTON_ARCHER");
	m_pButtonParty[CommonUtil::eSORCERESS]	= GetControl<CEtUIButton>("ID_BUTTON_SORCERESS");
	m_pButtonParty[CommonUtil::eCLERIC]		= GetControl<CEtUIButton>("ID_BUTTON_PARTY");
#ifdef PRE_ADD_ACADEMIC
	m_pButtonParty[CommonUtil::eACADEMIC]   = GetControl<CEtUIButton>("ID_BUTTON_ACADEMIC"); // #35786  // Added Academic
#endif
#if defined( PRE_ADD_KALI ) && !defined( PRE_REMOVE_KALI )
	m_pButtonParty[CommonUtil::eKALI]   = GetControl<CEtUIButton>("ID_BUTTON_KALI");
#endif

	m_pButtonGuild = GetControl<CEtUIButton>("ID_BUTTON_GUILD");
	m_pButtonToWorld = GetControl<CEtUIButton>("ID_BUTTON_TOWORLD");
	m_pButtonCancelCheck = GetControl<CEtUIButton>("ID_BUTTON_CANCEL_CHECK");
	m_pButtonCancelCheck->Show( false );
	m_pTextureMinimap = GetControl<CEtUITextureControl>("ID_TEXTUREL_MAP");
	m_pStaticDungeonTitle = GetControl<CEtUIStatic>("ID_DUNGEON_GATE");

	for( int i = 1; i <= 5; i++) {
		SDungeonInfo Info;
		char szID[64];
		sprintf_s(szID, "ID_DUNGEON_%02d", i);
		Info.pStatic = GetControl<CEtUIStatic>( szID );	
		sprintf_s(szID, "ID_DUNGEON_COVER_%02d", i);
		Info.pStaticRecommend = GetControl<CEtUIStatic>( szID );	
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST 
		sprintf_s(szID, "ID_STATIC_QUEST%d", i );
		Info.pStaticQuestMark = GetControl<CEtUIStatic>( szID );
		sprintf_s(szID, "ID_TEXT_QUESTCOUNT%d", i );
		Info.pStaticQuestCount = GetControl<CEtUIStatic>( szID );	
		sprintf_s( szID, "ID_DUNGEON_BASE%d", i);
		Info.pStaticBackground = GetControl<CEtUIStatic>( szID );	
#endif 
		Info.Coord = Info.pStatic->GetUICoord();
		Info.Coord.fX -= m_pStaticDungeonTitle->GetUICoord().fX;
		Info.Coord.fY -= m_pStaticDungeonTitle->GetUICoord().fY;

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
		Info.QuestMarkCoord = Info.pStaticQuestMark->GetUICoord();
		Info.QuestMarkCoord.fX -= m_pStaticDungeonTitle->GetUICoord().fX;
		Info.QuestMarkCoord.fY -= m_pStaticDungeonTitle->GetUICoord().fY;

		Info.QuestCountCoord = Info.pStaticQuestCount->GetUICoord();
		Info.QuestCountCoord.fX -= m_pStaticDungeonTitle->GetUICoord().fX;
		Info.QuestCountCoord.fY -= m_pStaticDungeonTitle->GetUICoord().fY;
#endif

		m_vecDungeonName.push_back( Info );
	}	

	m_pStaticPlayer->Show( false );
	m_pStaticPlayerUp->Show( false );

	int j = 0;
	for (; j < CLASSKINDMAX; ++j)
	{
		if (m_pButtonParty[j])
			m_pButtonParty[j]->Show(false);
	}

	m_pButtonGuild->Show( false );

	m_pButtonGate[ZONE_GATE_VIALLAGE] = GetControl<CDnZoneGateButton>("ID_BUTTON_GATE_VILLAGE");
	m_pButtonGate[ZONE_GATE_DUNGEON] = GetControl<CDnZoneGateButton>("ID_BUTTON_GATE_DUNGEON");
	m_pButtonGate[ZONE_GATE_WORLD] = GetControl<CDnZoneGateButton>("ID_BUTTON_GATE_WORLD");
	m_pButtonGate[ZONE_GATE_CLOSE] = GetControl<CDnZoneGateButton>("ID_BUTTON_GATE_CLOSE");
	m_pButtonGate[ZONE_GATE_UNKNOWN] = GetControl<CDnZoneGateButton>("ID_BUTTON_GATE_UNKNOWN");

	m_pButtonGate[ZONE_GATE_VIALLAGE]->Show( false );
	m_pButtonGate[ZONE_GATE_DUNGEON]->Show( false );
	m_pButtonGate[ZONE_GATE_WORLD]->Show( false );
	m_pButtonGate[ZONE_GATE_CLOSE]->Show( false );
	m_pButtonGate[ZONE_GATE_UNKNOWN]->Show( false );

	m_pButtonEnemy = GetControl<CEtUIButton>("ID_BUTTON_PVP_ENEMY");
	m_pButtonGhost = GetControl<CEtUIButton>("ID_BUTTON_PVP_GHOST");

	m_pButtonRoot = GetControl<CDnFarmAreaButton>("ID_BUTTON_LIFE0");
	m_pButtonLeaf = GetControl<CDnFarmAreaButton>("ID_BUTTON_LIFE2");
	m_pButtonFruit = GetControl<CDnFarmAreaButton>("ID_BUTTON_LIFE4");
	m_pButtonCereals = GetControl<CDnFarmAreaButton>("ID_BUTTON_LIFE1");
	m_pButtonFungus = GetControl<CDnFarmAreaButton>("ID_BUTTON_LIFE3");
	m_pButtonEmptyArea = GetControl<CDnFarmAreaButton>("ID_BUTTON_CHECK");

	m_pButtonRoot->Show( false );
	m_pButtonLeaf->Show( false );
	m_pButtonFruit->Show( false );
	m_pButtonCereals->Show( false );
	m_pButtonFungus->Show( false );
	m_pButtonEmptyArea->Show( false );

	m_pButtonQuestTrace = GetControl<CEtUIButton>( "ID_BUTTON_QUESTPOINT" );
	m_pButtonQuestTrace->Show( false );

	m_pButtonEnemy->Show( false );	// Seems unused right now, so keeping it hidden.
	m_pButtonGhost->Show( false );	

	m_SmartMoveEx.SetControl( m_pButtonToWorld );

	InitializeNpcControl();
}

void CDnZoneMapDlg::InitializeNpcControl()
{
	std::string strNpcUIName[CDnNPCActor::typeAmount];
	for( int i=0; i< CDnNPCActor::typeAmount; i++ )
	{
		strNpcUIName[i] = "ID_BUTTON_NPC";
	}
	
	// Since the structure requires code changes whenever an NPC is added, it would be better to move this part to a table or refactor it.
	
	// strNpcUIName[CDnNPCActor::typeNone] : Default Control , Do Nothing
	strNpcUIName[CDnNPCActor::typeWeapon]  += "_WEAPON";
	strNpcUIName[CDnNPCActor::typeParts]   += "_PARTS";
	strNpcUIName[CDnNPCActor::typeNormal]  += "_NORMAL";
	strNpcUIName[CDnNPCActor::typeStorage] += "_STORAGE";
	strNpcUIName[CDnNPCActor::typeMarket]  += "_MARKET";
	strNpcUIName[CDnNPCActor::typeMail]    += "_MAIL";
	strNpcUIName[CDnNPCActor::typeGuild]   += "_GUILD";

	strNpcUIName[CDnNPCActor::typeWarriorSkill]  += "_WARRIOR";
	strNpcUIName[CDnNPCActor::typeArcherSkill]   += "_ARCHER";
	strNpcUIName[CDnNPCActor::typeSoceressSkill] += "_SOCERESS";
	strNpcUIName[CDnNPCActor::typeClericSkill]   += "_CLERIC";

	strNpcUIName[CDnNPCActor::typePlate]   += "_PLATE";

	strNpcUIName[CDnNPCActor::typeCompleteQuest]  += "_REQUESTGRANT";
	strNpcUIName[CDnNPCActor::typeAdventureBoard] += "_REQUESTGRANT";

	strNpcUIName[CDnNPCActor::typeVehicle] += "_TRAINER";
	strNpcUIName[CDnNPCActor::typeTicket]  += "_TICKET";

	strNpcUIName[CDnNPCActor::typeAcademicSkill] += "_ACADEMIC";
	strNpcUIName[CDnNPCActor::typeKaliSkill]     += "_KALI";
	strNpcUIName[CDnNPCActor::typePcBang]        += "_PC";
	strNpcUIName[CDnNPCActor::typePresent] += "_PRESENT";
	strNpcUIName[CDnNPCActor::typeEvent] += "_EVENT";

	for( int i = 0; i < CDnNPCActor::typeAmount; i++)
	{
		bool bExistControl = false;
		m_pButtonZoneNPC[ i ] = GetControl<CDnZoneNPCButton>( strNpcUIName[i].c_str() , &bExistControl );

		if( !bExistControl )
		{
			m_pButtonZoneNPC[ i ] = GetControl<CDnZoneNPCButton>( "ID_BUTTON_NPC" );
		}
		m_pButtonZoneNPC[ i ]->Show( false );
	}
}

void CDnZoneMapDlg::InitCustomControl( CEtUIControl *pControl )
{
}

void CDnZoneMapDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_CLOSE" ) )	{
			Show( false );
		}

		if( strstr( pControl->GetControlName(), "ID_BUTTON_CHECK" ) || strstr( pControl->GetControlName(), "ID_BUTTON_LIFE" )  )
		{
			CDnFarmAreaButton *pFarmButton = (CDnFarmAreaButton*)pControl;

			if( uMsg == WM_RBUTTONUP )
			{
				bool bPushPin = pFarmButton->IsPushPin();
				if( bPushPin )
				{
					pFarmButton->SetPushPin( false );
					if( CDnActor::s_hLocalActor ) {
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->IsFollowing();
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->ResetAutoRun();
					}
				}
				else
				{
					ResetButtonPin();
					pFarmButton->SetPushPin( true );
					if( CDnActor::s_hLocalActor ) {
#ifdef PRE_MOD_NAVIGATION_PATH
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->AutoMovingToPosition( *pFarmButton->GetWorldPos() );
#else // PRE_MOD_NAVIGATION_PATH
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->FollowPosition( *pFarmButton->GetWorldPos() );
#endif // PRE_MOD_NAVIGATION_PATH
					}
				}

				return;
			}
		}

		if( strstr( pControl->GetControlName(), "ID_BUTTON_GATE_" ) )
		{
			CDnZoneGateButton *pGateButton = (CDnZoneGateButton*)pControl;

			if( uMsg == WM_RBUTTONUP )
			{
				bool bPushPin = pGateButton->IsPushPin();
				if( bPushPin )
				{
					pGateButton->SetPushPin( false );
					if( CDnActor::s_hLocalActor ) {
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->IsFollowing();
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->ResetAutoRun();
					}
				}
				else
				{
					ResetButtonPin();
					pGateButton->SetPushPin( true );
					if( CDnActor::s_hLocalActor ) {
#ifdef PRE_MOD_NAVIGATION_PATH
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->AutoMovingToPosition( *pGateButton->GetWorldPos() );
#else // PRE_MOD_NAVIGATION_PATH
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->FollowPosition( *pGateButton->GetWorldPos() );
#endif // PRE_MOD_NAVIGATION_PATH
					}
				}

				return;
			}
			
			return;
		}

		if( strstr( pControl->GetControlName(), "ID_BUTTON_NPC" ) )
		{

			if (uMsg == WM_LBUTTONUP)
			{
				CDnZoneNPCButton *pNPCButton = (CDnZoneNPCButton*)pControl;
				EtVector3 vPos = EtVec2toVec3(*pNPCButton->GetWorldPos());
				vPos.y = CDnWorld::GetInstance().GetHeight(vPos);
				EtVector2 vDir = *pNPCButton->GetDirection() * (float)(pNPCButton->GetUnitSize() + CDnActor::s_hLocalActor->GetUnitSize());
				vPos += EtVec2toVec3(vDir);
				vDir.x = fmod(vDir.x + 1.80f, 3.60f);
				this->Show(false);
			}

			if( uMsg == WM_RBUTTONUP )
			{
				CDnZoneNPCButton *pNPCButton = (CDnZoneNPCButton*)pControl;
				bool bPushPin = pNPCButton->IsPushPin();

				if( bPushPin )
				{
					pNPCButton->SetPushPin( false );
					if( CDnActor::s_hLocalActor ) {
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->IsFollowing();
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->ResetAutoRun();
					}
				}
				else
				{
					ResetButtonPin();
					pNPCButton->SetPushPin( true );
					if( CDnActor::s_hLocalActor ) {
						EtVector3 vPos = EtVec2toVec3( *pNPCButton->GetWorldPos() );
						vPos.y = CDnWorld::GetInstance().GetHeight( vPos );
						EtVector2 vDir = *pNPCButton->GetDirection() * (float)( pNPCButton->GetUnitSize() + CDnActor::s_hLocalActor->GetUnitSize() );
						vPos += EtVec2toVec3( vDir );
#ifdef PRE_MOD_NAVIGATION_PATH
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->AutoMovingToPosition( vPos );
#else // PRE_MOD_NAVIGATION_PATH
						((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->FollowPosition( vPos );
#endif // PRE_MOD_NAVIGATION_PATH
					}
				}

				return;
			}

			return;
		}

		if( IsCmdControl("ID_BUTTON_TOWORLD") )
		{
			CDnMainMenuDlg *pMainMenuDlg = (CDnMainMenuDlg*)GetParentDialog();
			if( pMainMenuDlg )	pMainMenuDlg->ShowWorldMapDialog( true );
			return;
		}

		if( IsCmdControl("ID_BUTTON_CANCEL_CHECK") )
		{
			ResetButtonPin();
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnZoneMapDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		//if( GetInterface().GetInterfaceType() == CDnInterface::Village )
		//{
		//	CDnVillageTask *pTask = (CDnVillageTask*)CTaskManager::GetInstance().GetTask( "VillageTask" );
		//	if( pTask ) 
		//	{
		//		m_nCurMapIndex = pTask->GetVillageMapIndex();
		//	}
		//}
		//else if( GetInterface().GetInterfaceType() == CDnInterface::Game )
		//{
		//	CDnGameTask *pTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
		//	if( pTask ) 
		//	{
		//		m_nCurMapIndex = pTask->GetGameMapIndex();
		//	}
		//}
		//else
		//{
		//	ASSERT(0&&"CDnZoneMapDlg::SetMapIndex");
		//}

		m_nCurMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;

		if( m_nSetMapIndex == CURRENT_ZONE )
		{
			m_bCurWorld = true;	
		}
		else
		{
			if( m_nCurMapIndex == m_nSetMapIndex )
			{
				m_bCurWorld = true;
			}
			else
			{
				m_nCurMapIndex = m_nSetMapIndex;
				m_bCurWorld = false;
			}
		}

		CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( m_nCurMapIndex );
		if( pWorldData )
		{
			std::string strMiniMapName;
			pWorldData->GetMiniMapName( strMiniMapName );

			char szTemp[_MAX_PATH] = {0};
			sprintf_s( szTemp, _MAX_PATH, "%s\\Grid\\%s\\%s_Map.dds", CEtResourceMng::GetInstance().GetFullPath( "MapData" ).c_str(), strMiniMapName.c_str(), strMiniMapName.c_str() );
			SAFE_RELEASE_SPTR( m_hMinimap );
			m_hMinimap = LoadResource( szTemp, RT_TEXTURE );
			if( !m_hMinimap ) {
				sprintf_s( szTemp, _MAX_PATH, "%s\\Grid\\%s\\%s.dds", CEtResourceMng::GetInstance().GetFullPath( "MapData" ).c_str(), strMiniMapName.c_str(), strMiniMapName.c_str() );
				m_hMinimap = LoadResource( szTemp, RT_TEXTURE );
			}

			int nX = 0, nY = 0, nWidth = 1024, nHeight = 1024;			
			if( m_hMinimap ) {
				m_hMinimap->CalcFittingArea( 10, &nX, &nY, &nWidth, &nHeight);
				
				if( nWidth > nHeight ) {
					nY -= (nWidth-nHeight)/2;
					nHeight = nWidth;
				}
				if( nHeight > nWidth ) {
					nX -= (nHeight-nWidth)/2;
					nWidth = nHeight;
				}

				const int GabSize = 60;

				nX -= GabSize;
				nWidth += GabSize*2;
				nY -= GabSize;
				nHeight += GabSize*2;
			}
			
			m_pTextureMinimap->SetTexture( m_hMinimap, nX, nY, nWidth, nHeight );

			std::wstring strMapName;
			pWorldData->GetMapName( strMapName );
			m_pStaticMapName->SetText( strMapName );
		}

		if( m_bCurWorld )
		{
			CreateNpcControl( m_nCurMapIndex );
			CreateNpcControl();
			CreateGateControl();
			CreatePartyControl();
			CreateFarmControl();
			CreateQuestTrace();
			
			MoveToHead( m_pStaticPlayer );
			MoveToTail( m_pStaticPlayerUp );

			m_pStaticPlayer->Show( true );
			m_pStaticPlayerUp->Show( true );
		}
		else
		{
			CreateNpcControl( m_nCurMapIndex );
			CreateGateControl( m_nCurMapIndex );
		}

		MoveToTail( m_pButtonQuestTrace );
		MoveToTail( m_pStaticDungeonTitle );
		MoveToTail( m_vecDungeonName[0].pStatic );
		MoveToTail( m_vecDungeonName[1].pStatic );
		MoveToTail( m_vecDungeonName[2].pStatic );
		MoveToTail( m_vecDungeonName[3].pStatic );
		MoveToTail( m_vecDungeonName[4].pStatic );
		MoveToTail( m_vecDungeonName[0].pStaticRecommend );
		MoveToTail( m_vecDungeonName[1].pStaticRecommend );
		MoveToTail( m_vecDungeonName[2].pStaticRecommend );
		MoveToTail( m_vecDungeonName[3].pStaticRecommend );
		MoveToTail( m_vecDungeonName[4].pStaticRecommend );

		m_SmartMoveEx.MoveCursor();
	}
	else
	{
		EtInterface::focus::ReleaseControl();
		s_pMouseEnterControl = NULL;

		DeletePartyControl();
		DeleteGateControl();
		DeleteNpcControl();
		DeleteFarmControl();

		m_fElapsedTime = 0.0f;
		m_pStaticPlayer->Show( false );
		m_pStaticPlayerUp->Show( false );
		m_bCurWorld = false;
		m_nSetMapIndex = 0;
		m_nTraceQuest = -1;

		HideStageList();

		if (CDnInterface::IsActive())
			ShowTooltipDlg(NULL, false);
	}

	CEtUIDialog::Show( bShow );

	if( m_pCallback )
	{
		m_pCallback->OnUICallbackProc( GetDialogID(), bShow?EVENT_DIALOG_SHOW:EVENT_DIALOG_HIDE, NULL );
	}
}

void CDnZoneMapDlg::Process( float fElapsedTime )
{
	m_SmartMoveEx.Process();
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	if( m_bCurWorld )
	{
		ProcessPlayer( fElapsedTime );
		ProcessNPC( fElapsedTime );
		ProcessParty( fElapsedTime );
	}

	if( m_pStaticDungeonTitle->IsShow() ) {
		ShowTooltipDlg( NULL , false);
	}
	
}

bool CDnZoneMapDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_MOUSEMOVE ) 
	{
		POINT MousePoint;
		float fMouseX, fMouseY;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		PointToFloat( MousePoint, fMouseX, fMouseY );
	
		bool bInsideButton = false;
		CDnZoneGateButton *pDungeonGateButton = NULL;

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
		std::wstring wszQuestCount;
		std::wstring wszDungeonName;
		int nQuestCount = 0;
#endif

		for( int i = 0; i < ZONE_GATE_AMOUNT; i++) {
			for each( CDnZoneGateButton *pGateButton in  m_vecButtonGate[i] ) {
				if( pGateButton->IsInside( fMouseX, fMouseY ) ) {
					bInsideButton = true;
					pDungeonGateButton = pGateButton;
					break;
				}
			}
			if( bInsideButton && pDungeonGateButton)
			{
				if( pDungeonGateButton->m_bDungeonGate )
				{
					m_pStaticDungeonTitle->SetText( pDungeonGateButton->m_strGateName );
					for( int i = 0; i < 5; i++) {
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
						wszDungeonName = FormatW(L"    %s", pDungeonGateButton->m_strDungeonName[i].c_str() );
						m_vecDungeonName[i].pStatic->SetText( wszDungeonName );
						m_vecDungeonName[i].pStaticRecommend->SetText( wszDungeonName );
#else
						m_vecDungeonName[i].pStatic->SetText( pDungeonGateButton->m_strDungeonName[i] );
						m_vecDungeonName[i].pStaticRecommend->SetText( pDungeonGateButton->m_strDungeonName[i] );
#endif 
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
						nQuestCount = GetEnbleQuestCountWithMapIndex( pDungeonGateButton->m_GateInfo.m_nMapIndex[i] );
						wszQuestCount = FormatW(L"%d", nQuestCount) ; 
						m_vecDungeonName[i].pStaticQuestCount->SetText(wszQuestCount.c_str());
#endif 

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
						switch( pDungeonGateButton->m_GateInfo.m_DungeonType[i] ) {
#else
						switch( pDungeonGateButton->m_DungeonType[i] ) {
#endif 
						case CDnZoneGateButton::DT_RECOMMEND:
							m_vecDungeonName[i].pStaticRecommend->SetTextColor( D3DCOLOR_XRGB(255, 255, 255) );
							m_vecDungeonName[i].pStatic->Show( true );
							m_vecDungeonName[i].pStaticRecommend->Show( true );
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
							if( pDungeonGateButton->m_strDungeonName[i].empty() == false )
							{
								m_vecDungeonName[i].pStaticBackground->Show( true );
								if( nQuestCount > 0 )
								{
									m_vecDungeonName[i].pStaticQuestMark->Show( true );
									m_vecDungeonName[i].pStaticQuestCount->Show( true );
								}
							}
#endif
							break;
						case CDnZoneGateButton::DT_NOTRECOMMEND:
							m_vecDungeonName[i].pStatic->SetTextColor( D3DCOLOR_XRGB(255, 255, 255) );
							m_vecDungeonName[i].pStatic->Show( true );
							m_vecDungeonName[i].pStaticRecommend->Show( false );
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
							if( pDungeonGateButton->m_strDungeonName[i].empty() == false )
							{
								m_vecDungeonName[i].pStaticBackground->Show( true );
								if( nQuestCount > 0 )
								{
									m_vecDungeonName[i].pStaticQuestMark->Show( true );
									m_vecDungeonName[i].pStaticQuestCount->Show( true );
								}
							}

#endif
							break;
						case CDnZoneGateButton::DT_NOTENTER:
							m_vecDungeonName[i].pStatic->SetTextColor( D3DCOLOR_XRGB(255, 64, 64) );
							m_vecDungeonName[i].pStatic->Show( true );
							m_vecDungeonName[i].pStaticRecommend->Show( false );
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
							if( pDungeonGateButton->m_strDungeonName[i].empty() == false )
							{
								m_vecDungeonName[i].pStaticBackground->Show( true );
								if( nQuestCount > 0 )
								{
									m_vecDungeonName[i].pStaticQuestMark->Show( true );
									m_vecDungeonName[i].pStaticQuestCount->Show( true );
								}
							}
#endif
							break;
						}
					}

					float fGateX = pDungeonGateButton->GetUICoord().fX + 0.015f;
					float fGateY = pDungeonGateButton->GetUICoord().fY + 0.02f;
					m_pStaticDungeonTitle->SetPosition( fGateX, fGateY );
					for( int i = 0; i < 5; i++) {
						m_vecDungeonName[i].pStatic->SetPosition( fGateX + m_vecDungeonName[i].Coord.fX,
							fGateY + m_vecDungeonName[i].Coord.fY );

						m_vecDungeonName[i].pStaticRecommend->SetPosition( fGateX + m_vecDungeonName[i].Coord.fX,
							fGateY + m_vecDungeonName[i].Coord.fY );
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
						m_vecDungeonName[i].pStaticBackground->SetPosition( fGateX + m_vecDungeonName[i].Coord.fX,
							fGateY + m_vecDungeonName[i].Coord.fY );	

						m_vecDungeonName[i].pStaticQuestMark->SetPosition( fGateX + m_vecDungeonName[i].QuestMarkCoord.fX,
							fGateY + m_vecDungeonName[i].Coord.fY );
				
						m_vecDungeonName[i].pStaticQuestCount->SetPosition( fGateX + m_vecDungeonName[i].QuestCountCoord.fX,
							fGateY + m_vecDungeonName[i].Coord.fY );
#endif
					}
					m_pShowGateButton = pDungeonGateButton;
					m_pStaticDungeonTitle->Show( true );
				}
			}
			else
			{
				HideStageList();
			}
		}
	}

	if( uMsg == WM_RBUTTONDOWN )
	{

		POINT MousePoint;
		float fMouseX, fMouseY;

		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		PointToFloat( MousePoint, fMouseX, fMouseY );

		bool bInsideButton = false;

		for each( SZoneNPCButtonInfo NpcInfo in m_vecButtonNPC ) {
			if( NpcInfo.pButton->IsInside( fMouseX, fMouseY ) ) {
				bInsideButton = true;
				break;
			}
		}

		for( int i = 0; i < ZONE_GATE_AMOUNT; i++) {
			for each( CDnZoneGateButton *pGateButton in  m_vecButtonGate[i] ) {
				if( pGateButton->IsInside( fMouseX, fMouseY ) ) {
					bInsideButton = true;
					break;
				}
			}
			if( bInsideButton ) break;
		}

		for( int itr = 0; itr < (int)m_vecButtonFarm.size(); ++itr )
		{
			if( m_vecButtonFarm[itr]->IsInside( fMouseX, fMouseY ) )
			{
				bInsideButton = true;
				break;
			}
		}
		
		if( !bInsideButton && m_BaseDlgCoord.IsInside( fMouseX, fMouseY ) )
		{
			CDnMainMenuDlg *pMainMenuDlg = (CDnMainMenuDlg*)GetParentDialog();
			if( pMainMenuDlg )	pMainMenuDlg->ShowWorldMapDialog( true );
			CONTROL( Button, ID_BUTTON_TOWORLD )->PlayButtonSound();
		}
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnZoneMapDlg::ProcessPlayer( float fElapsedTime )
{
	if( !CDnWorld::IsActive() ) 
		return;
	if( !CDnWorld::GetInstance().GetGrid() )
		return;
		
	const EtVector3 *pPlayerPos(NULL);
	DnActorHandle hPlayer = CDnActor::s_hLocalActor;

	if( hPlayer )
		pPlayerPos = hPlayer->GetPosition();
	else 
		pPlayerPos = CDnWorld::GetInstance().GetUpdatePosition();

	if( !pPlayerPos )
		return;

	float fMapWidth = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeight = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;

	float fPlayerX = pPlayerPos->x + fMapWidth;
	float fPlayerY = fabs(pPlayerPos->z - fMapHeight);

	SUICoord UVCoord;
	m_pTextureMinimap->GetUVCoord(UVCoord);

	SUICoord textureCoord;
	m_pTextureMinimap->GetUICoord(textureCoord);

	fPlayerX = (((fPlayerX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
	fPlayerY = (((fPlayerY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;	

	SUICoord playerCoord;
	m_pStaticPlayer->GetUICoord(playerCoord);
	m_pStaticPlayer->SetPosition( textureCoord.fX + fPlayerX - (playerCoord.fWidth/2.0f), textureCoord.fY + fPlayerY - (playerCoord.fHeight/2.0f) );
	m_pStaticPlayerUp->SetPosition( textureCoord.fX + fPlayerX - (playerCoord.fWidth/2.0f), textureCoord.fY + fPlayerY - (playerCoord.fHeight/2.0f) );

	EtVector3 vZDir;
	vZDir = CDnCamera::GetActiveCamera()->GetMatEx()->m_vZAxis;
	float fDegree = EtToDegree( atan2f( vZDir.x, vZDir.z ) );	
	m_pStaticPlayer->SetRotate( fDegree );
	m_pStaticPlayerUp->SetRotate( fDegree );

	m_fElapsedTime += fElapsedTime;

	BYTE cAlpha = 255;
	const float fAnimateTerm = 2.f;
	cAlpha = EtClamp( (int)(255 * sinf( ET_PI *  fmodf( m_fElapsedTime ,fAnimateTerm ) / fAnimateTerm )), 0, 255);

	m_pStaticPlayerUp->SetTextureColor( D3DCOLOR_ARGB(cAlpha, 255, 255, 255)  );

}

static bool CompareNpcControl( CDnZoneMapDlg::SZoneNPCButtonInfo s1, CDnZoneMapDlg::SZoneNPCButtonInfo s2 )
{
	if( s1.pButton->GetUICoord().fY < s2.pButton->GetUICoord().fY ) return false;
	else if( s1.pButton->GetUICoord().fY > s2.pButton->GetUICoord().fY ) return true;

	return false;
}

void CDnZoneMapDlg::CreateNpcControl()
{
	if( !CDnWorld::IsActive() || !CDnWorld::GetInstance().GetGrid() ) 
		return;

	SUICoord textureCoord, buttonCoord;
	m_pTextureMinimap->GetUICoord(textureCoord);
	m_pButtonZoneNPC[ CDnNPCActor::typeNone ]->GetUICoord(buttonCoord);

	float fMapWidth = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeight = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;

	SUIControlProperty sUIProperty;
	CDnZoneNPCButton *pButton(NULL);

	for( DWORD i=0; i<CDnActor::s_pVecProcessList.size(); i++ ) 
	{
		DnActorHandle hActor = CDnActor::s_pVecProcessList[i]->GetMySmartPtr();

		if( hActor && hActor->GetActorType() == CDnActorState::Npc )
		{
			CDnNPCActor *pNpcActor = dynamic_cast<CDnNPCActor*>(hActor.GetPointer());
			if( !pNpcActor || IsCreateNPC( pNpcActor->GetNpcData().nNpcID ) || !pNpcActor->IsShow() )
				continue;

			const EtVector3 *pNpcPos = pNpcActor->GetPosition();
			if( !pNpcPos ) continue;

			float fNpcX = pNpcPos->x + fMapWidth;
			float fNpcY = fabs(pNpcPos->z - fMapHeight);

			SUICoord UVCoord;
			m_pTextureMinimap->GetUVCoord(UVCoord);

			fNpcX = (((fNpcX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
			fNpcY = (((fNpcY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;

			int nJobType = pNpcActor->GetNPCJobType();
			if( !(nJobType >= 0 && nJobType < CDnNPCActor::typeAmount) )
				continue;

			m_pButtonZoneNPC[ nJobType ]->GetProperty( sUIProperty );

			pButton = (CDnZoneNPCButton*)CDnCustomDlg::CreateControl( &sUIProperty );
			pButton->SetPosition( textureCoord.fX + fNpcX - (buttonCoord.fWidth/2.0f), textureCoord.fY + fNpcY - (buttonCoord.fHeight/2.0f) );

			std::wstring wszToolTip;
			wszToolTip += L"#j";
			wszToolTip += pNpcActor->GetName();
			wszToolTip += L"#d";

			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNPC );
			if(pSox && pSox->IsExistItem(pNpcActor->GetNpcData().nNpcID))
			{
				int nDescription = 0;
				nDescription = pSox->GetFieldFromLablePtr(pNpcActor->GetNpcData().nNpcID , "_FunctionDescriptionID")->GetInteger();
				if(nDescription > 0)
					wszToolTip += FormatW(L"\n#y[%s]#d", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nDescription) ); // NPC additional description
			}

			std::vector<std::wstring> vecAvailableQuestName;
			vecAvailableQuestName.clear();

			CDnNPCActor::GetAvailableQuestName(vecAvailableQuestName , pNpcActor->GetNpcData().nNpcID);

			for(int i=0;i<(int)vecAvailableQuestName.size();i++)
				wszToolTip += FormatW(L"\n%s",vecAvailableQuestName[i].c_str()); // List all quests sequentially

			pButton->SetTooltipText(wszToolTip.c_str());

			pButton->Show( true );

			SZoneNPCButtonInfo zoneNPCButtonInfo;
			zoneNPCButtonInfo.pButton = pButton;
			zoneNPCButtonInfo.nNpcID = pNpcActor->GetNpcData().nNpcID;

			m_vecButtonNPC.push_back( zoneNPCButtonInfo );
		}
	}

	// #12208. Added to fix NPC quest mark being hidden by other NPC marks.
	// It is possible to render all other zone map sprites at depth 0.5 and only the exclamation mark at 0.0, but
	// Since doing this requires unintuitive steps like keeping future elements at 0.5,
	// Just sort by Y height to change rendering order.
	// Finally, minimap texture must be drawn first, so call MoveToHead.
	std::sort( m_vecButtonNPC.begin(), m_vecButtonNPC.end(), CompareNpcControl );
	for( int i=0; i<(int)m_vecButtonNPC.size(); i++ )
		MoveToHead( m_vecButtonNPC[i].pButton );
	MoveToHead( m_pTextureMinimap );
}

void CDnZoneMapDlg::RefreshNPCActor()
{
	if( !IsShow() )
		return;

	if( m_bCurWorld )
	{
		CreateNpcControl();
	}
}

void CDnZoneMapDlg::RefreshPartyActor()
{
	if( !IsShow() )
		return;

	if( m_bCurWorld )
	{
		CreatePartyControl();
	}
}

void CDnZoneMapDlg::DeleteNpcControl()
{
	for( int i=0; i<(int)m_vecButtonNPC.size(); i++ )
	{
		DeleteControl( m_vecButtonNPC[i].pButton );
		m_vecButtonNPC[i].nNpcID = 0;
	}

	m_vecButtonNPC.clear();
}

void CDnZoneMapDlg::CreateGateControl()
{
	if( !CDnWorld::IsActive() || !CDnWorld::GetInstance().GetGrid() ) 
		return;

	int nPlayerLevel = -1;
	if( CDnActor::s_hLocalActor ) {
		nPlayerLevel = CDnActor::s_hLocalActor->GetLevel();
	}
	
	SUICoord UVCoord;
	m_pTextureMinimap->GetUVCoord(UVCoord);

	SUICoord UICoord;
	m_pTextureMinimap->GetUICoord(UICoord);

	CDnWorld::GateStruct *pGateStruct(NULL);

	DWORD dwCount = CDnWorld::GetInstance().GetGateCount();

	float fMapWidth = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeight = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;

	for( DWORD i=0; i<dwCount; i++ )
	{
		pGateStruct = CDnWorld::GetInstance().GetGateStructFromIndex( i );
		if( !pGateStruct ) continue;

		if( !pGateStruct->pGateArea ) continue;
		SOBB *pOBB = pGateStruct->pGateArea->GetOBB();
		if( !pOBB ) continue;
		if( !pGateStruct->bIncludeBuild ) continue;
		EtVector3 vGatePos = pOBB->Center;	
		
		DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
		DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );

		float fGateX = vGatePos.x + fMapWidth;
		float fGateY = fabs(vGatePos.z - fMapHeight);

		fGateX = (((fGateX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * UICoord.fWidth;
		fGateY = (((fGateY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * UICoord.fHeight;

		emZoneGateIndex emGateIndex(ZONE_GATE_UNKNOWN);

		if( pGateStruct->PermitFlag == CDnWorld::PermitEnter )
		{
			switch( pGateStruct->MapType )
			{
			case CDnWorld::MapTypeUnknown:		emGateIndex = ZONE_GATE_UNKNOWN;	break;
			case CDnWorld::MapTypeVillage:		emGateIndex = ZONE_GATE_VIALLAGE;	break;
			case CDnWorld::MapTypeWorldMap:		emGateIndex = ZONE_GATE_WORLD;		break;
			case CDnWorld::MapTypeDungeon:		emGateIndex = ZONE_GATE_DUNGEON;	break;
			default:							emGateIndex = ZONE_GATE_CLOSE;		break;
			}
		}
		else
		{
			emGateIndex = ZONE_GATE_CLOSE;
		}
		CDnZoneGateButton *pButton(NULL);	

		SUIControlProperty sUIProperty;
		m_pButtonGate[emGateIndex]->GetProperty( sUIProperty );

		SUICoord buttonCoord;
		m_pButtonGate[emGateIndex]->GetUICoord(buttonCoord);

		SUICoord textureCoord;
		m_pTextureMinimap->GetUICoord(textureCoord);

		pButton = (CDnZoneGateButton*)CDnCustomDlg::CreateControl( &sUIProperty );
		pButton->SetPosition( textureCoord.fX + fGateX - (buttonCoord.fWidth/2.0f), textureCoord.fY + fGateY - (buttonCoord.fHeight/2.0f) );
		pButton->SetWorldPos( vGatePos );

		// Note : Dungeon gate
		//
		if( emGateIndex == ZONE_GATE_DUNGEON )
		{
			pButton->m_bDungeonGate = true;
			pButton->m_strGateName = pGateStruct->szMapName;

			CDnWorld::DungeonGateStruct *pDungeonGateStruct = (CDnWorld::DungeonGateStruct*)pGateStruct;

			for( int j = 0;j < 5; j++) {
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
				pButton->m_GateInfo.m_DungeonType[j] = CDnZoneGateButton::DT_NOTENTER;
#else
				pButton->m_DungeonType[j] = CDnZoneGateButton::DT_NOTENTER;
#endif
			}
			for( int j=0; j<(int)pDungeonGateStruct->pVecMapList.size(); j++ )
			{
				pButton->m_strDungeonName[j] = pDungeonGateStruct->pVecMapList[j]->szMapName;				
				int nDungeonEnterTableID = pMapSox->GetFieldFromLablePtr( pDungeonGateStruct->pVecMapList[j]->nMapIndex, "_EnterConditionTableID" )->GetInteger();
				if( nDungeonEnterTableID > 0 ) {
					if( pDungeonSox->IsExistItem( nDungeonEnterTableID ) ) {
						int nRecommendLevel = pDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendNormalLevel" )->GetInteger();
						int nLimitLevel = pDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_LvlMin" )->GetInteger();

						if( nLimitLevel > nPlayerLevel ) {
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
							pButton->m_GateInfo.m_DungeonType[j] = CDnZoneGateButton::DT_NOTENTER;
							pButton->m_GateInfo.m_nMapIndex[j] = 0;	// In case of level restriction, it does not hold map index info. 
#else
							pButton->m_DungeonType[j] = CDnZoneGateButton::DT_NOTENTER;
#endif
						}
						else if( abs(nRecommendLevel - nPlayerLevel) <= 2 ) {
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
							pButton->m_GateInfo.m_DungeonType[j] = CDnZoneGateButton::DT_RECOMMEND;
							pButton->m_GateInfo.m_nMapIndex[j] = pDungeonGateStruct->pVecMapList[j]->nMapIndex;
#else
							pButton->m_DungeonType[j] = CDnZoneGateButton::DT_RECOMMEND;
#endif 
						}
						else {
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
							pButton->m_GateInfo.m_DungeonType[j] = CDnZoneGateButton::DT_NOTRECOMMEND;
							pButton->m_GateInfo.m_nMapIndex[j] = pDungeonGateStruct->pVecMapList[j]->nMapIndex;
#else
							pButton->m_DungeonType[j] = CDnZoneGateButton::DT_NOTRECOMMEND;
#endif
						}
					}
				}	

				int len = ( int )pButton->m_strDungeonName[j].length();
				if( len > 40 ) {
					pButton->m_strDungeonName[j].erase( pButton->m_strDungeonName[j].begin() + 38 , pButton->m_strDungeonName[j].end() );
					pButton->m_strDungeonName[j] += L"\u2026";	// It's a special character, will it still show as ... in other languages???
				}

				CDnWorld::DungeonGateStruct *pSubGate = dynamic_cast<CDnWorld::DungeonGateStruct *>( pDungeonGateStruct->pVecMapList[j] );
				if( pSubGate ) {
					wchar_t szBuf[64]={0,};
					_itow_s(pSubGate->nMinLevel, szBuf, 10);

#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
					pButton->m_strDungeonName[j] += FormatW(L"(%s%s)", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), szBuf );
#else
					pButton->m_strDungeonName[j] += L" (Lv.";					
					pButton->m_strDungeonName[j] += szBuf;
					pButton->m_strDungeonName[j] += L")";
#endif
				}

			}
		}

		std::wstring wszTitleName = CDnWorld::GetInstance().GetGateTitleName( pGateStruct->cGateIndex );
		if( wszTitleName.empty() )
			wszTitleName = pGateStruct->szMapName.c_str();

		pButton->SetTooltipText( wszTitleName.c_str() );
		pButton->Show( true );

		m_vecButtonGate[emGateIndex].push_back( pButton );
	}
}

void CDnZoneMapDlg::CreateGateControl( int nMapIndex )
{
	CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( nMapIndex );
	if( !pWorldData ) return;

	SUICoord textureCoord;
	m_pTextureMinimap->GetUICoord(textureCoord);

	float fMapWidth = pWorldData->GetGridWidth() * 50.0f;
	float fMapHeight = pWorldData->GetGridHeight() * 50.0f;

	CDnWorld::GateStruct *pGateStruct(NULL);
	SOBB *pOBB(NULL);
	CDnZoneGateButton *pButton(NULL);

	int nPlayerLevel = -1;
	if( CDnActor::s_hLocalActor ) {
		nPlayerLevel = CDnActor::s_hLocalActor->GetLevel();
	}

	DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );

	int nCount = pWorldData->GetGateCount();
	for( int i=0; i<nCount; i++ )
	{
		CDnWorldData::GateData *pGateData = pWorldData->GetGateData( i );
		if( pGateData )
		{
			float fGateX = pGateData->vPos.x + fMapWidth;
			float fGateY = fabs(pGateData->vPos.y - fMapHeight);

			SUICoord UVCoord;
			m_pTextureMinimap->GetUVCoord(UVCoord);

			fGateX = (((fGateX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
			fGateY = (((fGateY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;

			emZoneGateIndex emGateIndex(ZONE_GATE_UNKNOWN);

			switch( pGateData->nMapType )
			{
			case CDnWorld::MapTypeUnknown:		emGateIndex = ZONE_GATE_UNKNOWN;	break;
			case CDnWorld::MapTypeVillage:		emGateIndex = ZONE_GATE_VIALLAGE;	break;
			case CDnWorld::MapTypeWorldMap:		emGateIndex = ZONE_GATE_WORLD;		break;
			case CDnWorld::MapTypeDungeon:		emGateIndex = ZONE_GATE_DUNGEON;	break;
			default:							emGateIndex = ZONE_GATE_CLOSE;		break;
			}

			if( emGateIndex == ZONE_GATE_DUNGEON ) {
				bool bPermitEnter = false;
				// If blocked by level limit, handle as Close.
				for( int j=0; j<(int)pGateData->vecGateData.size(); j++ )
				{
					int nDungeonEnterTableID = pMapSox->GetFieldFromLablePtr( pGateData->vecGateData[j].nMapIndex, "_EnterConditionTableID" )->GetInteger();
					if( nDungeonEnterTableID > 0 ) {
						if( pDungeonSox->IsExistItem( nDungeonEnterTableID ) ) {
							int nLimitLevel = pDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_LvlMin" )->GetInteger();
							if( nLimitLevel <= nPlayerLevel ) {
								bPermitEnter = true;
								break;
							}						
						}
					}
				}
				if( !bPermitEnter ) {
					emGateIndex = ZONE_GATE_CLOSE;
				}
			}

			SUIControlProperty sUIProperty;
			m_pButtonGate[emGateIndex]->GetProperty( sUIProperty );

			SUICoord buttonCoord;
			m_pButtonGate[emGateIndex]->GetUICoord(buttonCoord);

			pButton = (CDnZoneGateButton*)CDnCustomDlg::CreateControl( &sUIProperty );
			pButton->SetPosition( textureCoord.fX + fGateX - (buttonCoord.fWidth/2.0f), textureCoord.fY + fGateY - (buttonCoord.fHeight/2.0f) );

			// Note : Dungeon gate
			//
			if( emGateIndex == ZONE_GATE_DUNGEON )
			{
				pButton->m_bDungeonGate = true;
				pButton->m_strGateName = pGateData->strName;

				for( int j=0; j<5; j++) {
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
					pButton->m_GateInfo.m_DungeonType[j] = CDnZoneGateButton::DT_NOTENTER;
#else
					pButton->m_DungeonType[j] = CDnZoneGateButton::DT_NOTENTER;
#endif
				}
				for( int j=0; j<(int)pGateData->vecGateData.size(); j++ )
				{
					pButton->m_strDungeonName[j] = pGateData->vecGateData[j].strName;					
					int nDungeonEnterTableID = pMapSox->GetFieldFromLablePtr( pGateData->vecGateData[j].nMapIndex, "_EnterConditionTableID" )->GetInteger();
					if( nDungeonEnterTableID > 0 ) {
						if( pDungeonSox->IsExistItem( nDungeonEnterTableID ) ) {
							int nRecommendLevel = pDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendNormalLevel" )->GetInteger();
							int nLimitLevel = pDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_LvlMin" )->GetInteger();
							if( nLimitLevel > nPlayerLevel ) {
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
								pButton->m_GateInfo.m_DungeonType[j] = CDnZoneGateButton::DT_NOTENTER;
								pButton->m_GateInfo.m_nMapIndex[j] = 0;	// In case of level restriction, it does not hold map index info. 
#else
								pButton->m_DungeonType[j] = CDnZoneGateButton::DT_NOTENTER;
#endif
							}
							else if( abs(nRecommendLevel - nPlayerLevel) <= 2 ) {
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
								pButton->m_GateInfo.m_DungeonType[j] = CDnZoneGateButton::DT_RECOMMEND;
								pButton->m_GateInfo.m_nMapIndex[j] = pGateData->vecGateData[j].nMapIndex;
#else
								pButton->m_DungeonType[j] = CDnZoneGateButton::DT_RECOMMEND;
#endif 
							}
							else {
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
								pButton->m_GateInfo.m_DungeonType[j] = CDnZoneGateButton::DT_NOTRECOMMEND;
								pButton->m_GateInfo.m_nMapIndex[j] = pGateData->vecGateData[j].nMapIndex;
#else
								pButton->m_DungeonType[j] = CDnZoneGateButton::DT_NOTRECOMMEND;
#endif
							}
						}				

						int len = ( int )pButton->m_strDungeonName[j].length();
						if( len > 16 ) {
							pButton->m_strDungeonName[j].erase( pButton->m_strDungeonName[j].begin() + 14 , pButton->m_strDungeonName[j].end() );
							pButton->m_strDungeonName[j] += L"\u2026";	// It's a special character, will it still show as ... in other languages???
						}

						wchar_t szBuf[64]={0,};
						_itow_s(pGateData->vecGateData[j].nLevel, szBuf, 10);
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
						pButton->m_strDungeonName[j] += FormatW(L"(%s%s)", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), szBuf );
#else
						pButton->m_strDungeonName[j] += L" (Lv.";
						pButton->m_strDungeonName[j] += szBuf;
						pButton->m_strDungeonName[j] += L")";
#endif 
					}
				}

				pButton->SetTooltipText( pGateData->strName.c_str() );
				pButton->Show( true );

				m_vecButtonGate[emGateIndex].push_back( pButton );
			}
		}
	}

}

void CDnZoneMapDlg::ProcessNPC( float fElapsedTime )
{
	std::vector<SZoneNPCButtonInfo*> vecNPCButton;

	for( int i=0; i<(int)m_vecButtonNPC.size(); i++ )
	{
		vecNPCButton.push_back( &m_vecButtonNPC[i] );
	}

	for( int i=0; i<(int)vecNPCButton.size(); i++ )
	{
		if( !(vecNPCButton[i]->pButton) ) 
			continue;

		CDnZoneNPCButton *pButton = vecNPCButton[i]->pButton;
		CDnNPCActor *pNPCActor = CDnNPCActor::FindNpcActorFromID( vecNPCButton[i]->nNpcID );
		if( !pNPCActor )
			continue;

		int nNPCStateMark = pNPCActor->GetNPCStateMark();
		if( nNPCStateMark >= CDnNPCActor::QuestAvailable_Main && nNPCStateMark < CDnNPCActor::MaxCount )
		{
			switch( nNPCStateMark )
			{
			case CDnNPCActor::QuestAvailable_Main:
				pButton->SetQuestState( CDnZoneNPCButton::buttonQuestAvailable );
				break;
			case CDnNPCActor::QuestAvailable_Sub:
				pButton->SetQuestState( CDnZoneNPCButton::buttonSubQuestAvailable );
				break;
			case CDnNPCActor::QuestPlaying_Main:
				pButton->SetQuestState( CDnZoneNPCButton::buttonQuestPlaying );
				break;
			case CDnNPCActor::QuestPlaying_Sub:
				pButton->SetQuestState( CDnZoneNPCButton::buttonSubQuestPlaying );
				break;
			case CDnNPCActor::QuestRecompense_Main:
				pButton->SetQuestState( CDnZoneNPCButton::buttonQuestRecompense );
				break;
			case CDnNPCActor::QuestRecompense_Sub:
				pButton->SetQuestState( CDnZoneNPCButton::buttonSubQuestRecompense );
				break;
			case CDnNPCActor::CrossFavorGrant:
				pButton->SetQuestState( CDnZoneNPCButton::buttonReputationQuestAvailable );
				break;
			case CDnNPCActor::CrossFavorNow:
				pButton->SetQuestState( CDnZoneNPCButton::buttonReputationQuestPlaying );
				break;
			case CDnNPCActor::CrossFavorComplete:
				pButton->SetQuestState( CDnZoneNPCButton::buttonReputationQuestRecompense );
				break;
			case CDnNPCActor::CrossReQuestComplete:
				pButton->SetQuestState( CDnZoneNPCButton::buttonAdventureQuestRecompense );
				break;
			case CDnNPCActor::CrossQuestGlobalGrant :
				pButton->SetQuestState( CDnZoneNPCButton::buttonQuestGlobalAvailable );
				break;
			case CDnNPCActor::CrossQuestGlobalNow :
				pButton->SetQuestState( CDnZoneNPCButton::buttonQuestGlobalPalying );
				break;
			case CDnNPCActor::CrossQuestGlobalComplete :
				pButton->SetQuestState( CDnZoneNPCButton::buttonQuestGlobalRecompense );
				break;

#ifdef PRE_ADD_58449
			case CDnNPCActor::CrossNoticePcBang:
				pButton->SetQuestState( CDnZoneNPCButton::buttonNoticePcBang );
				break;
#endif

			case CDnNPCActor::MailReceiveNew:
				pButton->SetMailState(CDnZoneNPCButton::buttonMailNew);
			default:
				pButton->SetQuestState( CDnZoneNPCButton::buttonQuestNone );
				break;

			}
		}
	}
}

void CDnZoneMapDlg::ProcessParty( float fElapsedTime )
{
	if( !CDnWorld::IsActive()  || !CDnWorld::GetInstance().GetGrid() ) 
		return;

	if ( !CDnWorld::GetInstance().GetGrid() )
		return;

	CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
	if( !pPartyTask ) return;

	CDnPartyTask::PartyStruct* pParty(NULL);

	SUICoord textureCoord;
	m_pTextureMinimap->GetUICoord(textureCoord);

	float fMapWidth = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeight = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;

	CEtUIButton *pButton(NULL);
	CDnActor *pActor(NULL);

	std::list<SZonePartyButtonInfo>::iterator iter = m_listButtonParty.begin();
	for( ; iter != m_listButtonParty.end(); )
	{
		pActor = (CDnActor*)CDnActor::FindActorFromUniqueID( iter->nSessionID );
		pParty = pPartyTask->GetPartyDataFromSessionID( iter->nSessionID );

		if( !pParty )
		{
			DeleteControl( iter->pButton );
			iter = m_listButtonParty.erase( iter );
			continue;
		}

		EtVector3 *pNpcPos = ( pActor ) ? pActor->GetPosition() : &pParty->vPos;
		if( !pNpcPos ) continue;

		float fNpcX = pNpcPos->x + fMapWidth;
		float fNpcY = fabs(pNpcPos->z - fMapHeight);

		SUICoord UVCoord;
		m_pTextureMinimap->GetUVCoord(UVCoord);

		fNpcX = (((fNpcX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
		fNpcY = (((fNpcY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;

		SUICoord uiCoord;
		iter->pButton->GetUICoord(uiCoord);
		iter->pButton->SetPosition( textureCoord.fX + fNpcX - (uiCoord.fWidth/2.0f), textureCoord.fY + fNpcY - (uiCoord.fHeight/2.0f) );
		iter->pButton->SetTooltipText( pParty->wszCharacterName );

		++iter;
	}
}

void CDnZoneMapDlg::DeleteGateControl()
{
	for( int i=0; i<ZONE_GATE_AMOUNT; i++ )
	{
		for( int j=0; j<(int)m_vecButtonGate[i].size(); j++ )
		{
			DeleteControl( m_vecButtonGate[i][j] );
		}

		m_vecButtonGate[i].clear();
	}
}

void CDnZoneMapDlg::ResetEmptyAreaButtonPin()
{
	for( int itr = 0; itr < (int)m_vecButtonFarm.size(); ++itr )
		m_vecButtonFarm[itr]->SetPushPin( false );
}

void CDnZoneMapDlg::ResetGateButtonPin()
{
	for( int i=0; i<ZONE_GATE_AMOUNT; i++ )
	{
		for( int j=0; j<(int)m_vecButtonGate[i].size(); j++ )
		{
			m_vecButtonGate[i][j]->SetPushPin( false );
		}
	}
}

void CDnZoneMapDlg::ResetNPCButtonPin()
{
	for( int i=0; i<(int)m_vecButtonNPC.size(); i++ )
	{
		if( m_vecButtonNPC[i].pButton )
		{
			m_vecButtonNPC[i].pButton->SetPushPin( false );
		}
	}
}

void CDnZoneMapDlg::ProcessQuestTrace()
{
	if( -1 != m_nTraceQuest && m_pButtonQuestTrace->IsShow() )	
	{
		SUICoord sUICoord = m_pButtonQuestTrace->GetUICoord();

		if( sUICoord.fY >= m_sUICoord.fY )
			sUICoord.fY = m_sUICoord.fY - (m_sUICoord.fHeight*1.5f);
		else
			sUICoord.fY += (GetTickCount() % 60) *0.00005f;

		m_pButtonQuestTrace->SetPosition( sUICoord.fX, sUICoord.fY );
	}
}

void CDnZoneMapDlg::CreateQuestTrace()
{
	m_nTraceQuest = GetQuestTask().GetTraceQuestID();

	m_pButtonQuestTrace->Show( false );

	if( -1 != m_nTraceQuest )	
	{
		CDnQuestTask::QuestNotifierInfo* pInfo = GetQuestTask().GetQuestNotifierInfoByQuestIDForce( m_nTraceQuest );

		if( !pInfo ) return;
		else if( pInfo->pathResult.empty() ) return;

		float fMapWidth = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
		float fMapHeight = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
		SUICoord textureCoord;
		m_pTextureMinimap->GetUICoord(textureCoord);

		m_pButtonQuestTrace->Show( true );

		float fNpcX = pInfo->vTargetPos.x + fMapWidth;
		float fNpcY = fabs(pInfo->vTargetPos.y - fMapHeight);

		SUICoord UVCoord;
		m_pTextureMinimap->GetUVCoord(UVCoord);

		fNpcX = (((fNpcX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
		fNpcY = (((fNpcY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;

		SUICoord uiCoord;
		m_pButtonQuestTrace->GetUICoord(uiCoord);
		m_pButtonQuestTrace->SetPosition( textureCoord.fX + fNpcX - (uiCoord.fWidth/2.0f), textureCoord.fY + fNpcY - uiCoord.fHeight );
		m_pButtonQuestTrace->GetUICoord( m_sUICoord );
	}
}

void CDnZoneMapDlg::CreateFarmControl()
{
	CDnGameTask * pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( !pGameTask || pGameTask->GetGameTaskType() != GameTaskType::Farm )
		return;

	DNTableFileFormat* pFarmListSox = GetDNTable( CDnTableDB::TFARMSKIN );
	DNTableFileFormat* pFarmSox = GetDNTable( CDnTableDB::TFARMCULTIVATE );
	DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );

	float fMapWidth = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeight = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
	SUICoord textureCoord;
	m_pTextureMinimap->GetUICoord(textureCoord);

	CDnFarmAreaButton * pButton(NULL);
	EtVector3 vPos;

	CDnLifeConditionDlg * pLifeConditionDlg = GetInterface().GetLifeConditionDlg();

	if( NULL == pLifeConditionDlg )	return;

	std::vector< std::pair<int, EtVector3> > vecEmptyAreaInfo;
	pLifeConditionDlg->GetEmptyAreaInfo( vecEmptyAreaInfo );

	for( int itr = 0; itr < (int)vecEmptyAreaInfo.size(); ++itr )
	{
		vPos = vecEmptyAreaInfo[itr].second;

		SUIControlProperty sUIProperty;
		m_pButtonEmptyArea->GetProperty( sUIProperty );

		pButton = (CDnFarmAreaButton*)CDnCustomDlg::CreateControl( &sUIProperty );
		pButton->SetWorldPos( vPos );
		//pButton = (CEtUIButton*)CEtUIDialog::CreateControl( &sUIProperty );
		pButton->Show( true );

		float fNpcX = vPos.x + fMapWidth;
		float fNpcY = fabs(vPos.z - fMapHeight);

		SUICoord UVCoord;
		m_pTextureMinimap->GetUVCoord(UVCoord);

		fNpcX = (((fNpcX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
		fNpcY = (((fNpcY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;

		SUICoord uiCoord;
		pButton->GetUICoord(uiCoord);
		pButton->SetPosition( textureCoord.fX + fNpcX - (uiCoord.fWidth/2.0f), textureCoord.fY + fNpcY - (uiCoord.fHeight/2.0f) );

		m_vecButtonFarm.push_back( pButton );
	}

	std::vector< std::pair<int,int> > vecMyFarmInfo;
	pLifeConditionDlg->GetMyFarmInfo( vecMyFarmInfo );
	
	for( int itr = 0; itr < (int)vecMyFarmInfo.size(); ++itr )
	{
		vPos = GetLifeSkillPlantTask().GetFarmAreaPos( vecMyFarmInfo[itr].first );

		int nSkinID = pFarmSox->GetFieldFromLablePtr( vecMyFarmInfo[itr].second, "_SkinID" )->GetInteger();
		char * szHarvestIconName = pFarmListSox->GetFieldFromLablePtr( nSkinID, "_HarvestIcon" )->GetString();

		wstring wszItemName;
		int nNameID = pItemSox->GetFieldFromLablePtr( vecMyFarmInfo[itr].second, "_NameID" )->GetInteger();
		char *szParam = pItemSox->GetFieldFromLablePtr( vecMyFarmInfo[itr].second, "_NameIDParam" )->GetString();
		MakeUIStringUseVariableParam( wszItemName, nNameID, szParam );

		SUIControlProperty sUIProperty;

		if( stricmp( "LifeCrossHair07.dds", szHarvestIconName ) == 0 )
			m_pButtonRoot->GetProperty( sUIProperty );
		else if( stricmp( "LifeCrossHair06.dds", szHarvestIconName ) == 0 )
			m_pButtonCereals->GetProperty( sUIProperty );
		else if( stricmp( "LifeCrossHair05.dds", szHarvestIconName ) == 0 )
			m_pButtonLeaf->GetProperty( sUIProperty );
		else if( stricmp( "LifeCrossHair04.dds", szHarvestIconName ) == 0 )
			m_pButtonFungus->GetProperty( sUIProperty );
		else if( stricmp( "LifeCrossHair03.dds", szHarvestIconName ) == 0 )
			m_pButtonFruit->GetProperty( sUIProperty );

		pButton = (CDnFarmAreaButton*)CDnCustomDlg::CreateControl( &sUIProperty );
		pButton->SetWorldPos( vPos );
		pButton->Show( true );

		float fNpcX = vPos.x + fMapWidth;
		float fNpcY = fabs(vPos.z - fMapHeight);

		SUICoord UVCoord;
		m_pTextureMinimap->GetUVCoord(UVCoord);

		fNpcX = (((fNpcX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
		fNpcY = (((fNpcY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;

		SUICoord uiCoord;
		pButton->GetUICoord(uiCoord);
		pButton->SetPosition( textureCoord.fX + fNpcX - (uiCoord.fWidth/2.0f), textureCoord.fY + fNpcY - (uiCoord.fHeight/2.0f) );
		pButton->SetTooltipText( wszItemName.c_str() );

		m_vecButtonFarm.push_back( pButton );
	}
}

void CDnZoneMapDlg::RefreshFarmControl()
{
	if( !IsShow() )
		return;

	if( m_bCurWorld )
	{
		//DeleteFarmControl();
		//CreateFarmControl();
	}
}

void CDnZoneMapDlg::DeleteFarmControl()
{
	for( std::vector<CDnFarmAreaButton *>::iterator itor = m_vecButtonFarm.begin(); itor != m_vecButtonFarm.end(); ++itor )
		DeleteControl( (*itor) );

	m_vecButtonFarm.clear();
}

void CDnZoneMapDlg::CreatePartyControl()
{
	CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
	if( !pPartyTask ) return;

	CDnGameTask * pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm ) // modified by kalliste
		return;

	CDnPartyTask::PartyStruct* pParty(NULL);
	int nCount = pPartyTask->GetPartyCount();

	CEtUIButton *pButton(NULL);

	for( int i = 0; i<nCount; i++ )
	{
		pParty = pPartyTask->GetPartyData( i );
		if( !pParty ) continue;

		if( pParty->nSessionID == CDnBridgeTask::GetInstance().GetSessionID() )
			continue;
		if( pParty->bGMTrace ) continue;

		if (CommonUtil::IsValidCharacterClassId(pParty->cClassID) == false)
			continue;

		SUIControlProperty sUIProperty;
		if(m_pButtonParty[pParty->cClassID - 1])
			m_pButtonParty[pParty->cClassID - 1]->GetProperty( sUIProperty );

		pButton = (CEtUIButton*)CEtUIDialog::CreateControl( &sUIProperty );
		pButton->Show( true );

		SZonePartyButtonInfo zonePartyButtonInfo;
		zonePartyButtonInfo.pButton = pButton;
		zonePartyButtonInfo.nSessionID = pParty->nSessionID;

		m_listButtonParty.push_back( zonePartyButtonInfo );
	}
}

void CDnZoneMapDlg::DeletePartyControl()
{
	std::list<SZonePartyButtonInfo>::iterator iter = m_listButtonParty.begin();
	std::list<SZonePartyButtonInfo>::iterator iter_end = m_listButtonParty.end();

	for( ; iter != iter_end; ++iter )
	{
		DeleteControl( iter->pButton );
	}

	m_listButtonParty.clear();
}

void CDnZoneMapDlg::ResetButtonPin()
{
	ResetGateButtonPin();
	ResetNPCButtonPin();
	ResetEmptyAreaButtonPin();
}

void CDnZoneMapDlg::CreateNpcControl( int nMapIndex )
{
	CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( nMapIndex );
	if( !pWorldData ) return;

	SUICoord textureCoord, buttonCoord;
	m_pTextureMinimap->GetUICoord(textureCoord);
	m_pButtonZoneNPC[ CDnNPCActor::typeNone ]->GetUICoord(buttonCoord);

	float fMapWidth = pWorldData->GetGridWidth() * 50.0f;
	float fMapHeight = pWorldData->GetGridHeight() * 50.0f;

	SUIControlProperty sUIProperty;
	CDnZoneNPCButton *pButton(NULL);

	int nCount = pWorldData->GetNpcCount();
	for( int i=0; i<nCount; i++ )
	{
		CDnWorldData::NpcData *pNpcData = pWorldData->GetNpcData( i );
		if( pNpcData ) {
			if( pNpcData->nJobType < 0 ) continue;
			// In the current world zone, create only those present in the actor list.
			bool bCreate = false;
			if( nMapIndex == CGlobalInfo::GetInstance().m_nCurrentMapIndex && CTaskManager::GetInstance().GetTask( "GameTask" ) ) {
				for( DWORD i=0; i<CDnActor::s_pVecProcessList.size(); i++ )  {
					DnActorHandle hActor = CDnActor::s_pVecProcessList[i]->GetMySmartPtr();
					if( hActor && hActor->GetActorType() == CDnActorState::Npc ) {
						CDnNPCActor *pNpcActor = dynamic_cast<CDnNPCActor*>(hActor.GetPointer());
						if( pNpcActor && pNpcActor->GetNpcData().nNpcID == pNpcData->nNpcID && pNpcActor->IsShow() ) {
							bCreate = true;
							break;
						}
					}
				}
			}
			else {
				bCreate = true;
			}

			if( !bCreate ) continue;

			float fNpcX = pNpcData->vPos.x + fMapWidth;
			float fNpcY = fabs(pNpcData->vPos.y - fMapHeight);

			SUICoord UVCoord;
			m_pTextureMinimap->GetUVCoord(UVCoord);

			fNpcX = (((fNpcX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
			fNpcY = (((fNpcY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;

			if( !(pNpcData->nJobType >= 0 &&  pNpcData->nJobType < CDnNPCActor::typeAmount))
				continue;

			m_pButtonZoneNPC[ pNpcData->nJobType ]->GetProperty( sUIProperty );

			pButton = (CDnZoneNPCButton*)CDnCustomDlg::CreateControl( &sUIProperty );
			pButton->SetPosition( textureCoord.fX + fNpcX - (buttonCoord.fWidth/2.0f), textureCoord.fY + fNpcY - (buttonCoord.fHeight/2.0f) );

			std::wstring wszToolTip;
			wszToolTip +=  L"#j";
			wszToolTip +=  pNpcData->strName;
			wszToolTip +=  L"#d";

			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNPC );
			if( pSox && pSox->IsExistItem(pNpcData->nNpcID) )
			{
				int nDescription = 0;
				nDescription = pSox->GetFieldFromLablePtr(pNpcData->nNpcID , "_FunctionDescriptionID")->GetInteger();
				if(nDescription > 0)
					wszToolTip += FormatW(L"\n#y[%s]#d", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nDescription) ); // NPC additional description
			}

			std::vector<std::wstring> vecAvailableQuestName;
			vecAvailableQuestName.clear();

			CDnNPCActor::GetAvailableQuestName(vecAvailableQuestName , pNpcData->nNpcID);

			for(int i=0;i<(int)vecAvailableQuestName.size();i++)
				wszToolTip += FormatW(L"\n%s",vecAvailableQuestName[i].c_str()); // List all quests sequentially

			pButton->SetTooltipText(wszToolTip.c_str());
			
			pButton->SetQuestState( CDnZoneNPCButton::buttonQuestNone );
			pButton->SetWorldPos( pNpcData->vPos );
			pButton->SetUnitSize( pNpcData->nUnitSize );
			pButton->SetDirection( EtVector2( sin( EtToRadian( pNpcData->fRotate ) ), cos( EtToRadian( pNpcData->fRotate ) ) ) );

			int nStateMark = CDnNPCActor::CalcStateMark( pNpcData->nNpcID );
			if( nStateMark >= CDnNPCActor::QuestAvailable_Main && nStateMark < CDnNPCActor::MaxCount )
			{
				switch( nStateMark )
				{
				case CDnNPCActor::QuestAvailable_Main:
					pButton->SetQuestState( CDnZoneNPCButton::buttonQuestAvailable );
					break;
				case CDnNPCActor::QuestAvailable_Sub:
					pButton->SetQuestState( CDnZoneNPCButton::buttonSubQuestAvailable );
					break;
				case CDnNPCActor::QuestPlaying_Main:
					pButton->SetQuestState( CDnZoneNPCButton::buttonQuestPlaying );
					break;
				case CDnNPCActor::QuestPlaying_Sub:
					pButton->SetQuestState( CDnZoneNPCButton::buttonSubQuestPlaying );
					break;
				case CDnNPCActor::QuestRecompense_Main:
					pButton->SetQuestState( CDnZoneNPCButton::buttonQuestRecompense );
					break;
				case CDnNPCActor::QuestRecompense_Sub:
					pButton->SetQuestState( CDnZoneNPCButton::buttonSubQuestRecompense );
					break;
				case CDnNPCActor::CrossFavorGrant:
					pButton->SetQuestState( CDnZoneNPCButton::buttonReputationQuestAvailable );
					break;
				case CDnNPCActor::CrossFavorNow:
					pButton->SetQuestState( CDnZoneNPCButton::buttonReputationQuestPlaying );
					break;
				case CDnNPCActor::CrossFavorComplete:
					pButton->SetQuestState( CDnZoneNPCButton::buttonReputationQuestRecompense );
					break;
				case CDnNPCActor::CrossReQuestComplete:
					pButton->SetQuestState( CDnZoneNPCButton::buttonAdventureQuestRecompense );
					break;
				case CDnNPCActor::CrossQuestGlobalGrant :
					pButton->SetQuestState( CDnZoneNPCButton::buttonQuestGlobalAvailable );
					break;
				case CDnNPCActor::CrossQuestGlobalNow :
					pButton->SetQuestState( CDnZoneNPCButton::buttonQuestGlobalPalying );
					break;
				case CDnNPCActor::CrossQuestGlobalComplete :
					pButton->SetQuestState( CDnZoneNPCButton::buttonQuestGlobalRecompense );
					break;

#ifdef PRE_ADD_58449
				case CDnNPCActor::CrossNoticePcBang:
					pButton->SetQuestState( CDnZoneNPCButton::buttonNoticePcBang );
					break;
#endif

				case CDnNPCActor::MailReceiveNew:
					pButton->SetMailState(CDnZoneNPCButton::buttonMailNew);
				default:
					pButton->SetQuestState( CDnZoneNPCButton::buttonQuestNone );
					break;
				}
			}
			pButton->Show( true );

			SZoneNPCButtonInfo zoneNPCButtonInfo;
			zoneNPCButtonInfo.pButton = pButton;
			zoneNPCButtonInfo.nNpcID = pNpcData->nNpcID;

			m_vecButtonNPC.push_back( zoneNPCButtonInfo );
		}
	}

	std::sort( m_vecButtonNPC.begin(), m_vecButtonNPC.end(), CompareNpcControl );
	for( int i=0; i<(int)m_vecButtonNPC.size(); i++ )
		MoveToHead( m_vecButtonNPC[i].pButton );
	MoveToHead( m_pTextureMinimap );
}

void CDnZoneMapDlg::SetMapIndex( int nMapIndex )
{
	if( nMapIndex == PREV_ZONE ) {
		m_nSetMapIndex = m_nCurMapIndex;
	}
	else{
		m_nSetMapIndex = nMapIndex;
	}
}

bool CDnZoneMapDlg::IsCreateNPC( int nID )
{
	for( int i=0; i<(int)m_vecButtonNPC.size(); i++ )
	{
		if( m_vecButtonNPC[i].nNpcID == nID )
			return true;
	}

	return false;
}

void CDnZoneMapDlg::HideStageList()
{
	m_pStaticDungeonTitle->Show( false );
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
	for( int i = 0; i < (int)m_vecDungeonName.size(); ++i )
	{
		m_vecDungeonName[i].pStatic->Show( false );
		m_vecDungeonName[i].pStaticRecommend->Show( false );
		m_vecDungeonName[i].pStaticQuestMark->Show( false );
		m_vecDungeonName[i].pStaticQuestCount->Show( false );
		m_vecDungeonName[i].pStaticBackground->Show( false );
	}
#else
	m_vecDungeonName[0].pStatic->Show( false );
	m_vecDungeonName[1].pStatic->Show( false );
	m_vecDungeonName[2].pStatic->Show( false );
	m_vecDungeonName[3].pStatic->Show( false );
	m_vecDungeonName[4].pStatic->Show( false );
	m_vecDungeonName[0].pStaticRecommend->Show( false );
	m_vecDungeonName[1].pStaticRecommend->Show( false );
	m_vecDungeonName[2].pStaticRecommend->Show( false );
	m_vecDungeonName[3].pStaticRecommend->Show( false );
	m_vecDungeonName[4].pStaticRecommend->Show( false );
#endif 

}

bool CDnZoneMapDlg::IsChanged( int nMapIndex )
{
	if( nMapIndex == CURRENT_ZONE )
		return true;
	else
	{
		if( m_nCurMapIndex == nMapIndex )
			return true;
		else
			return false;
	}
}

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
int	CDnZoneMapDlg::GetEnbleQuestCountWithMapIndex( int nStageIndex )
{
	CDnWorldZoneSelectDlg* pWorldZoneSelectDlg = GetInterface().GetWorldZoneSelectDlg();

	if( ! pWorldZoneSelectDlg )
		return -1;
	
	return pWorldZoneSelectDlg->GetStageQuestCount( nStageIndex );
}
#endif 