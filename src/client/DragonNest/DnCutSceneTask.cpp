#include "StdAfx.h"

#include "DnWorld.h"
#include "DnCutSceneTask.h"
#include "DnCutSceneData.h"
#include "TaskManager.h"
#include "DnPartyTask.h"
#include "DnGameTask.h"
#include "DnActor.h"
#include "DnCamera.h"
#include "EternityEngine.h"
#include "DnCutSceneActorProcessor.h"
#include "DnTableDB.h"
#include "DnActorClassDefine.h"
#include "DnPlayerCamera.h"
#include "DnCutSceneData.h"
#include "DnInterface.h"
#include "DnWorldSector.h"
#include "DnPlayerActor.h"
#include "DnCEDof.h"
#include "DnWorldEnvironment.h"
#include "DnQuestTask.h"
#include "DnLocalPlayerActor.h"
#include "GameSendPacket.h"
#include "DnCommonTask.h"
#include "DnFadeInOutDlg.h"
#include "DnMonsterActor.h"
#include "DnCharStatusDlg.h"
#ifdef PRE_ADD_CUTSCENE_PADSKIP
#include "InputWrapper.h"
#endif

//#include "SmartPtr.h"
//#include ""


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 



CDnCutSceneTask::CDnCutSceneTask(void) : m_pCutSceneData( new CDnCutSceneData ), CInputReceiver( true ),
#if defined(TEST_CUTSCENE)
										 m_bFirstLoop( true ),
#endif
										 m_pWorld( NULL ),
										 m_pActorProcessor(	NULL ),
										 m_pDOFFilter( NULL ),
										 m_fNearStart( 0.0f ),
										 m_fNearEnd( 0.0f ),
										 m_fFarStart( 0.0f ),
										 m_fFarEnd( 0.0f ),
										 m_fFocusDist( 0.0f ),
										 m_fNearBlurSize( 0.0f ),
										 m_fFarBlurSize( 0.0f ),
										 m_bSetFadeModeCalledForSkip( false ),
										 m_bCheatCutScene( false ),
										m_pSkipFade( NULL )
{
	m_bRequestSkip = false;
	m_bAutoFadeIn = true;
	m_bLoadingComplete = false;
}

CDnCutSceneTask::~CDnCutSceneTask(void)
{
        // #60295
	m_arrCharacterName.clear();
	m_vecPartsInfo.clear();

	if( CTaskManager::IsActive() ) {
		CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
		if( pTask ) {
			pTask->EndCutScene();
		}
	}
	SAFE_DELETE( m_pSkipFade );
	SAFE_DELETE( m_pCutSceneData );
	SAFE_DELETE( m_pActorProcessor );
}

bool CDnCutSceneTask::Initialize( const char* pFileName, int nQuestIndex /*= -1*/, int nQuestStep /*= -1 */, 
								  bool bAutoFadeIn/* = true*/, DWORD dwQuestPlayerUniqueID/* = UINT_MAX*/ )
{
	// #60295
	m_arrCharacterName.push_back( "WARRIOR" );
	m_arrCharacterName.push_back( "ARCHER" );
	m_arrCharacterName.push_back( "SOCERESS" );
	m_arrCharacterName.push_back( "CLERIC" );
#ifdef PRE_ADD_ACADEMIC
	m_arrCharacterName.push_back( "ACADEMIC" );
#endif
#ifdef PRE_ADD_KALI
	m_arrCharacterName.push_back( "KALI" );
#endif
//[OK]
	bool bResult = false;

	m_bLoadingComplete = false;
	if( CDnLoadingTask::IsActive() ) {
		CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );
		CDnLoadingTask::GetInstance().Initialize( this, 0 );
	}

	if( strlen(pFileName) == 0 )
		return bResult;

	//bool bSuccess = m_pCutSceneData->LoadFromFile( pFileName );
	CStream* pStream = CEtResourceMng::GetInstance().GetStream( pFileName );
	_ASSERT( pStream && "Failed to load cutscene file stream!" );
	if( NULL == pStream )
		return bResult;

	int iSize = pStream->Size();
	char* pBuffer = new char[iSize];
	ZeroMemory( pBuffer, iSize );
	pStream->ReadBuffer( (void*)pBuffer, iSize );

	// After calling ReadBuffer, extra trailing memory was attached, so remove it.
	int i = 0;
	for( i = iSize-1; i > 0; --i )
	{
		if( '}' == pBuffer[ i ] && i < iSize-2 )
		{
			pBuffer[ i+1 ] = NULL;
			break;
		}
	}

	_ASSERT( 0 < i && "Invalid cutscene file." );
	
	bool bSuccess = m_pCutSceneData->LoadFromStringBuffer( pBuffer );
	_ASSERT( bSuccess && "Failed to load cutscene file!" );

	delete [] pBuffer;

	SAFE_DELETE( pStream );

	// Divided into cases with and without a quest number. Check if it came in properly.
	bool bValidQuestInfo = !((-1 == nQuestIndex && -1 != nQuestStep) || (-1 != nQuestIndex && -1 == nQuestStep));
	_ASSERT( bValidQuestInfo && "Quest ID is set without step, or step is set without Quest ID!" );
	if( false == bValidQuestInfo )
		return bResult;

	m_bAutoFadeIn = bAutoFadeIn;
	m_pWorld = CDnWorld::GetInstancePtr();
	bool bVillage = (CDnWorld::MapTypeEnum::MapTypeVillage == m_pWorld->GetMapType());

	if( bSuccess )
	{
		m_CutScenePlayer.SetWorld( m_pWorld );

		// A cutscene may play right after reviving. Restore color values here so it does not appear in monochrome.
		//SGraphicOption Option;
		//GetEtOptionController()->GetGraphicOption( Option );
		
		//Option.bEnableDOF
		CDnWorldEnvironment* pWorldEnv = m_pWorld->GetEnvironment();

		// If it's a village cutscene, unconditionally set hue value to 1. When replaying cutscene directly in village without exiting to game 
		// CDnLocalPlayerActor::GetSaturationColor() returns 0.
		if( bVillage )
			pWorldEnv->ControlColorFilter( CDnWorldEnvironment::Saturation, 1.0f, 500 );
		else
			pWorldEnv->ControlColorFilter( CDnWorldEnvironment::Saturation, CDnLocalPlayerActor::GetSaturationColor(), 500 );

		//if( CDnActor::s_hLocalActor )
		//	CDnActor::s_hLocalActor->SetPlayingCutScene( true );
		CGlobalInfo::GetInstance().SetPlayingCutScene( true );

		SAFE_DELETE( m_pActorProcessor );
		m_pActorProcessor = new CDnCutSceneActorProcessor;

		// Collect actors currently being rendered on screen
		int iNumLiveActor = (int)CDnActor::s_pVecProcessList.size();
		for( int iActor = 0; iActor < iNumLiveActor; ++iActor )
		{
			DnActorHandle hLiveActor = CDnActor::s_pVecProcessList.at( iActor )->GetMySmartPtr();
			if( hLiveActor->IsNpcActor() && !hLiveActor->IsShow() )
				continue;

			m_dqhLiveActors.push_back( hLiveActor );

			// Save original EtMatrixEx
			m_vlOriMatExWorlds.push_back( *hLiveActor->GetMatEx() );

			const char* pCurrentAction = hLiveActor->GetCurrentAction();

			//#43481 Prevent adding skill actions while a skill is in use.
			//Holds the current action and calls the stored action here in Finalize.
			if( NULL == pCurrentAction || 
				strlen(pCurrentAction) == 0 || 
				strcmp(pCurrentAction, "Summon_On") == 0
				|| hLiveActor->IsProcessSkill()
				)
				m_szOriAction.push_back( "Stand" );
			else
				m_szOriAction.push_back( pCurrentAction );
			
			//#43481 When a cutscene starts during a skill, cancel the client skill and send CmdStop packet for sync.
			if (hLiveActor && hLiveActor->IsProcessSkill())
			{
				hLiveActor->CancelUsingSkill();
				hLiveActor->CmdStop("Stand");				
			}
			
			CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hLiveActor.GetPointer());
			if( pPlayer )
				m_bVecPlayerBattleMode.push_back( pPlayer->IsBattleMode() );
			else m_bVecPlayerBattleMode.push_back( false );
		}

		// First add to excluded actor list, then find and remove actors in use.
		m_dqhExceptActors = m_dqhLiveActors;

		DNTableFileFormat*  pActorTable = CDnTableDB::GetInstancePtr()->GetTable( CDnTableDB::TACTOR );

		m_CutScenePlayer.Initialize( m_pCutSceneData );

		// Name of the currently displayed world
		const char* pGridName = NULL;
		if( CDnWorld::IsActive() && CDnWorld::GetInstance().GetGrid() )
			pGridName = CDnWorld::GetInstance().GetGrid()->GetName();

		const char* pCutSceneMapName = NULL;
		int iNumRes = m_pCutSceneData->GetNumRegResource();
		for( int iRes = 0; iRes < iNumRes; ++iRes )
		{
			int iResKind = m_pCutSceneData->GetRegResKindByIndex( iRes );
			if( CDnCutSceneData::RT_RES_MAP == iResKind )
			{
				pCutSceneMapName = m_pCutSceneData->GetRegResNameByIndex( iRes );
				break;
			}
		}

#if defined(_DEBUG)

		string strResMapName( pCutSceneMapName );
		transform( strResMapName.begin(), strResMapName.end(), strResMapName.begin(), tolower );

		bool bSameMap = (strResMapName == pGridName);

		//assert( bSameMap && "Map name saved in cutscene differs from current map." );
#endif
		// Hide if dropped item exists
		CDnDropItem::ShowDropItems( false );

		// Camera setup
		DnCameraHandle hDnCamera = CDnCamera::GetActiveCamera();

		// #38660 Sometimes camera retrieval fails, so fixed to get the player camera directly in that case.
		if( !hDnCamera )
			hDnCamera = CDnCamera::FindCamera( CDnCamera::PlayerCamera );
		m_CutScenePlayer.SetCamera( hDnCamera->GetCameraHandle() );

		SCameraInfo* pCameraInfo = hDnCamera->GetCameraInfo();
		m_OriginalCameraInfo = *pCameraInfo;
		//pCameraInfo->fFogFar += 20000.0f;
		const SceneInfo* pSceneInfo = m_pCutSceneData->GetSceneInfo();
		pCameraInfo->fFogFar += pSceneInfo->fFogFarDelta;
		pCameraInfo->fFar = pCameraInfo->fFogFar + 10000.0f;
		hDnCamera->SetCameraInfo( *pCameraInfo );

		// Pass down DOF filter
		int iDOFEffectIndex = m_pWorld->GetEnvironment()->GetDOFEffectIndex();
		DnCameraHandle hPlayerCamera = CDnCamera::FindCamera( CDnCamera::PlayerCamera );
		// Reset radial blur camera effect
		if( hPlayerCamera ) {
			hPlayerCamera->ResetRadialBlurEffect();
		}
		CDnCEDof* pDofEffect = NULL;
		if( hPlayerCamera ) {
			pDofEffect = static_cast<CDnCEDof*>(hPlayerCamera->FindCameraEffect( iDOFEffectIndex ));
		}
		if( pDofEffect )
		{
			m_pDOFFilter = pDofEffect->GetFilter();

			m_fNearStart =  m_pDOFFilter->GetNearDOFStart();
			m_fNearEnd = m_pDOFFilter->GetNearDOFEnd();
			m_fFarStart = m_pDOFFilter->GetFarDOFStart();
			m_fFarEnd = m_pDOFFilter->GetFarDOFEnd();
			m_fFocusDist = m_pDOFFilter->GetFocusDistance();
			m_fNearBlurSize = m_pDOFFilter->GetNearBlurSize();
			m_fFarBlurSize = m_pDOFFilter->GetFarBlurSize();
			
			m_CutScenePlayer.SetDOFFilter( m_pDOFFilter );
		}

		// Detach camera from actor so cutscene player can control camera.
		if( hDnCamera->GetCameraType() == CDnCamera::PlayerCamera ) {
			CDnPlayerCamera* pPlayerCamera = static_cast<CDnPlayerCamera*>(hDnCamera.GetPointer());
			pPlayerCamera->DetachActor();
		}

		// Handling differs slightly depending on whether the cutscene has quest info and whether players appear.

		// If the player character is in ghost state, revive them as their original character.
		// Store my party info.
		if( false == bVillage )
		{
			CDnPartyTask* pPartyTask  = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
			DWORD dwNumParty = pPartyTask->GetPartyCount();
			for( DWORD dwMember = 0; dwMember < dwNumParty; ++dwMember )
			{
				CDnPartyTask::PartyStruct* pParty = pPartyTask->GetPartyData( dwMember );
				if( !pParty->hActor ) continue;
				CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(pParty->hActor.GetPointer());
				if( pPlayerActor->IsGhost() )
				{
					pPlayerActor->SwapSingleSkin( -1 );

					CDnWorldEnvironment* pWorldEnv = CDnWorld::GetInstance().GetEnvironment();
					pWorldEnv->ControlColorFilter( CDnWorldEnvironment::Saturation, CDnLocalPlayerActor::GetSaturationColor(), 500 );
				}

			}
		}

		// Cutscene without quest info: determine number of players, whether it plays in a town, etc.
		bool bIsQuestCutScene = (-1 != nQuestIndex && -1 != nQuestStep);
		if( !bIsQuestCutScene )
		{
			PrepareNormalCutScene( pActorTable );
		}
		else
		{
			PrepareQuestCutScene( pActorTable, nQuestIndex, nQuestStep, dwQuestPlayerUniqueID );
		}

		// Filter out current action props and save their default actions... 
		m_vlActPropDefault.clear();
		CEtWorldSector* pSector = m_pWorld->GetSector( 0.0f, 0.0f );
		int iNumProp = pSector->GetPropCount();
		for( int iProp = 0; iProp < iNumProp; ++iProp )
		{
			CEtWorldProp* pProp = pSector->GetPropFromIndex( iProp );
			CDnWorldActProp* pActProp = dynamic_cast<CDnWorldActProp*>(pProp);
			if( pActProp )
			{
				S_ACT_PROP_DEFAULT PropDefault;
				PropDefault.pActProp = pActProp;
				PropDefault.hProp = pActProp->GetMySmartPtr();
				if( pActProp->GetCurrentAction() )
					PropDefault.strDefaultAction.assign( pActProp->GetCurrentAction() );
				PropDefault.bShow = PropDefault.hProp->IsShow();

				m_vlActPropDefault.push_back( PropDefault );
			}
		}

		// Configure sequence
		m_CutScenePlayer.MakeUpSequences( m_pActorProcessor );

		// Set all objects included in the sequence to show
		int iNumActor = m_pActorProcessor->GetNumActors();
		for( int i = 0; i < iNumActor; ++i )
		{
			S_CS_ACTOR_INFO* pInfo = m_pActorProcessor->GetActorInfoByIndex( i );
			CDnActor* pActor = static_cast<CDnActor*>( pInfo->pActor );
			pActor->Show( true );
			pActor->SetAlphaBlend( 1.0f, AL_SIGNAL );
			pActor->SetAlphaBlend( 1.0f, AL_NORMAL );
			pActor->SetAlphaBlend( 1.0f, AL_APPEAR );
			pActor->SetAlphaBlend( 1.0f, AL_STATEEFFECT );

			// Cutscene player computes and sets ani distance directly.
			// Since scale values can be present in the world matrix, when the engine calculates animations 
			// Because it is applied to the Y-axis ani distance enabled by default.
			// Currently decided that monster scale does not affect animation key values.
			if( pActor->GetAniObjectHandle() )
				pActor->GetAniObjectHandle()->SetCalcPositionFlag( 0 );
		}

		// Hide all excluded actors
		int iNumActors = (int)m_dqhExceptActors.size();
		for( int iActor = 0; iActor < iNumActors; ++iActor )
		{
			DnActorHandle hActor = m_dqhExceptActors.at( iActor );
			hActor->Show( false );
			hActor->ReleaseSignalImp();
		}

		CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.0f, CGlobalInfo::GetInstance().m_fFadeDelta, false );
		

#if defined(TEST_CUTSCENE)
		m_bFirstLoop = true;
#endif
	}
#if defined(TEST_CUTSCENE)
	else
		m_bFirstLoop = false;
#endif

	// Normally if UI was hidden before cutscene start, it should be shown again, but
	// If g_bEtUIRender is false during Finalize, the transition is not smooth and the character appears standing.
	// Because UI rendering is disabled, the fade-out screen is not visible.
	// So for now, decide not to restore the previous value.
	//
	// Looks like we need a way to show the fade-out dialog even when g_bEtUIRender is true.
	//m_bPrevEtUIRender = EtInterface::g_bEtUIRender;
	EtInterface::g_bEtUIRender = true;
	CEtCustomRender::s_bEnableCustomRender = true;

#ifndef _FINAL_BUILD
#ifdef _RDEBUG
	// In non-final builds, hide UI while holding Shift+Z to capture cutscenes without UI.
	if( GetKeyState( VK_LSHIFT )&0x80 && GetKeyState( 'Z' )&0x80 )
		EtInterface::g_bEtUIRender = false;
#endif
#endif
	m_bLoadingComplete = true;
	if( CDnLoadingTask::IsActive() ) {
		CDnLoadingTask::GetInstance().Initialize( NULL, 0 );
		CDnLoadingTask::GetInstance().EnableBackgroundLoading( true );
	}

	m_pSkipFade = new CDnFadeInOutDlg( UI_TYPE_MOST_TOP, NULL, 4444 );
	m_pSkipFade->Initialize( true );

	CInputDevice::GetInstance().HoldAbsenceCheckTimer(CInputDevice::eABSENCE_PARTY, true);
//	CDnInterface::GetInstance().UseEventMessage( false );

	bResult = true;
	return bResult;
}


void CDnCutSceneTask::PrepareNormalCutScene( DNTableFileFormat*  pActorTable )
{

// #61781 Shadow Forest Edge boss (Orc Khromsh) is not holding weapon and shield in cutscene.
//#ifdef _WORK
	enum EWeaponError
	{
		EWeaponError_None = 0,		
		EWeaponError_Other = 1,
		EWeaponError_WeaponIndex,
		EWeaponError_Create,
		EWeaponError_Attach,		

		EWeaponError_Max
	};
	char * arrStrError[ EWeaponError::EWeaponError_Max ] = { "None", "Other", "WeaponIndex", "Create", "Attach" };
	struct STempLog
	{
		EWeaponError errorType;
		bool bCutsceneWeapon[2];
		int MonsterID;
		int weapon[2];		
		STempLog(){
			errorType = EWeaponError::EWeaponError_None; 
			bCutsceneWeapon[0] = bCutsceneWeapon[1] = true;
			MonsterID = weapon[0] = weapon[1] = 0;
		}
	};
	STempLog tempLog;	
//#endif // _WORK


	// Check whether the current location is a game stage or a town
	bool bVillage = false;
	if( CDnWorld::MapTypeEnum::MapTypeVillage == m_pWorld->GetMapType() )
		bVillage = true;

	// Store my party info.
	deque<CDnPartyTask::PartyStruct*> dqParty;
	if( false == bVillage )
	{
		CDnPartyTask* pPartyTask  = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
		DWORD dwNumParty = pPartyTask->GetPartyCount();
		for( DWORD dwMember = 0; dwMember < dwNumParty; ++dwMember )
		{
			CDnPartyTask::PartyStruct* pParty = pPartyTask->GetPartyData( dwMember );
			dqParty.push_back( pParty );
		}
	}

	// Whether there is only one protagonist actor
	vector<string> vlActorResNames;
	bool bSinglePlayerCharacter = false;
	int iNumMainCharacter = 0;
	int iNumActors = m_pCutSceneData->GetNumActors();
	for( int iActor = 0; iActor < iNumActors; ++iActor )
	{
		string strActorResName = m_pCutSceneData->GetActorByIndex( iActor );
		int iOffset = (int)strActorResName.find_last_of( "_", strActorResName.length() );
		strActorResName = strActorResName.substr( 0, iOffset );
		vlActorResNames.push_back( strActorResName );

		int iItemID = pActorTable->GetItemIDFromFieldCaseFree( "_StaticName", strActorResName.c_str() );
		CDnActorState::ActorTypeEnum eActorType = (CDnActorState::ActorTypeEnum)pActorTable->GetFieldFromLablePtr( iItemID, "_Class" )->GetInteger();
		if( eActorType <= CDnActorState::Reserved6 )
		{
			++iNumMainCharacter;
		}
	}

	if( 1 == iNumMainCharacter )
		bSinglePlayerCharacter = true;
	
	bool bMyCharacterIsMain = false;

	// Get the designated main character actor name from cutscene.
	const SceneInfo* pSceneInfo = m_pCutSceneData->GetSceneInfo();
	const string& strMainCharName = pSceneInfo->strMainCharacterName;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TJOB );	


	

	// Among actors used in cutscene, reuse currently rendered ones and newly load the others.		
	for( int iActor = 0; iActor < iNumActors; ++iActor )
	{
		S_CS_ACTOR_INFO ActorToPlay;
		const char* pActorName = m_pCutSceneData->GetActorByIndex( iActor );
		
		ActorToPlay.strActorName.assign( pActorName );

		string& strActorResName = vlActorResNames.at( iActor );

		// Stored in uppercase on the tool side. When searching the table, capitalize only the first letter.
		//transform( strActorResName.begin()+1, strActorResName.end(), strActorResName.begin()+1, tolower );

		int iItemID = pActorTable->GetItemIDFromFieldCaseFree( "_StaticName", strActorResName.c_str() );
		//assert( -1 != iItemID );

		CDnActorState::ActorTypeEnum eActorType = (CDnActorState::ActorTypeEnum)pActorTable->GetFieldFromLablePtr( iItemID, "_Class" ) ->GetInteger();
		bool bPlayerCharacter = eActorType <= CDnActorState::Reserved6;
		ActorToPlay.bPlayer = bPlayerCharacter; // bintitle.
		
		// If there is only one protagonist actor, set my character as the protagonist.
		DnActorHandle hFindedActor;		
		if( bPlayerCharacter )
		{
			// If only one player character appears, always set my character as protagonist regardless of name.
			if( bSinglePlayerCharacter )
			{
				hFindedActor = CDnActor::s_hLocalActor;
				bMyCharacterIsMain = true;
			}
			else
			{
				string strActorResName = m_pCutSceneData->GetActorByIndex( iActor );

				// #60295 Kali opening cutscene Kali not visible. - Kali not visible - Check whether it is a cloned player actor.
				int nItemID = static_cast< CDnActor * >(CDnActor::s_hLocalActor)->GetClassID();						
				if( pSox->IsExistItem( nItemID ) ) {		
					//const wchar_t * pStrLocalActorName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nItemID, "_EnglishName" )->GetInteger() );
					int cntName = (int)m_arrCharacterName.size();
					for( int i=0; i<cntName; ++i )
					{		
						int characterNameIndex = nItemID - 1;
						if (characterNameIndex < 0 || characterNameIndex >= cntName)
						{
							_ASSERT(0);
							break;
						}

						/*if( strstr( m_arrCharacterName[characterNameIndex].c_str(), m_arrCharacterName[ i ].c_str() ) &&
							strstr( strActorResName.c_str(), m_arrCharacterName[ i ].c_str() ) )
						{
							std::string::size_type iOffset = strActorResName.find( "_" );
							if( iOffset != std::string::npos )
							{
								strActorResName = strActorResName.substr( ++iOffset, strActorResName.size() );
								int number = atoi( strActorResName.c_str() );
								if( number > 1 )
									ActorToPlay.bClone = true;
							}
						}*/
						if( strstr( m_arrCharacterName[characterNameIndex].c_str(), m_arrCharacterName[ i ].c_str() ) &&
							strstr( strActorResName.c_str(), m_arrCharacterName[ i ].c_str() ) )
						{
							if( strstr( strActorResName.c_str(), "CLONE" ) )
								ActorToPlay.bClone = true;
						}
							
					}	
				}	

				if( ActorToPlay.bClone == false ) // #60295 Kali opening cutscene - Kali is not visible.
				{
					// If currently in the protagonist slot, make my character the protagonist..
					if( false == bMyCharacterIsMain && strMainCharName == pActorName )
					{
						hFindedActor = CDnActor::s_hLocalActor;
						bMyCharacterIsMain = true;
					}
				}
			}	
			
		}

		// If self is not set as protagonist actor, search in the excluded list
		if( NULL == hFindedActor && ActorToPlay.bClone == false )
		{
			// Find actor with the same ItemID. Find in excluded actor list and remove it.
			// If it has a boss setting, even the monster table ID must match.
			int iNumLiveActor = (int)m_dqhExceptActors.size();
			for( int iActor = 0; iActor < iNumLiveActor; ++iActor )
			{
				DnActorHandle hLiveActor = m_dqhExceptActors.at( iActor );
				if( hLiveActor->GetClassID() == iItemID )
				{
					bool bScaleLock = m_pCutSceneData->GetThisActorsScaleLock( pActorName );
					if( bScaleLock )
					{
						// When scale-locked, actors spawned with a non-default scale are not used in cutscenes.
						if( hLiveActor->GetScale()->x != 1.0f )
							continue;
					}
					else
					{
						// Players are naturally 0, and among monsters, only those set as boss have an actual monster table ID.
						// Select this actor on screen only if the monster table ID also matches.
						int iMonsterTableID = m_pCutSceneData->GetMonsterTableID( pActorName );
						if( 0 != iMonsterTableID && false == bPlayerCharacter )
						{
							// If it is a monster actor and matches the monster table ID set in the tool, pass 
							// Otherwise skip with continue.
							CDnMonsterActor* pMonsterActor = dynamic_cast<CDnMonsterActor*>( hLiveActor.GetPointer() );
							if( false == (pMonsterActor && (iMonsterTableID == pMonsterActor->GetMonsterClassID())) )
							{
								continue;
							}
						}
					}

					hFindedActor = hLiveActor;
					deque<DnActorHandle>::iterator iter = m_dqhExceptActors.begin();
					advance( iter, iActor );
					m_dqhExceptActors.erase( iter );
					break;
				}
			}
		}
		else
		{
			// Find actor with the same ItemID. Now included, so just remove it from the exclusion list.
			int iNumLiveActor = (int)m_dqhExceptActors.size();
			for( int iActor = 0; iActor < iNumLiveActor; ++iActor )
			{
				DnActorHandle hLiveActor = m_dqhExceptActors.at( iActor );
				if( hLiveActor == hFindedActor )
				{
					deque<DnActorHandle>::iterator iter = m_dqhExceptActors.begin();
					advance( iter, iActor );
					m_dqhExceptActors.erase( iter );
					break;
				}
			}
		}
		//else
		//{
		//	int iNumLiveActor = (int)m_dqhExceptActors.size();
		//	for( int iActorInner = 0; iActorInner < iNumLiveActor; ++iActorInner )
		//	{
		//		DnActorHandle hLiveActor = m_dqhExceptActors.at( iActorInner );

		//		if( hLiveActor == hFindedActor )
		//		{
		//			deque<DnActorHandle>::iterator iter = m_dqhExceptActors.begin();
		//			advance( iter, iActorInner );
		//			m_dqhExceptActors.erase( iter );
		//			break;
		//		}
		//	}
		//}

		bool bMonster = (eActorType > CDnActorState::Reserved6 && eActorType < CDnActorState::Npc);

		if( NULL == hFindedActor )
		{
			// Until design team enters proper data 
			if( bPlayerCharacter )
			{
				// // #60295 Kali opening cutscene Kali's appearance is invisible - duplicate if there are two or more player actors.				
				if( ActorToPlay.bClone )
					AddClonePlayer( hFindedActor );

				else
				{
					// If my character is not registered yet, register immediately
					if( false == bMyCharacterIsMain )
					{
						hFindedActor = CDnActor::s_hLocalActor;
						bMyCharacterIsMain = true;
					}
					else
					{
						// If player is not found, map anyone in party order excluding the main character.
						do
						{
							if( dqParty.empty() )
								break;

							hFindedActor = dqParty.front()->hActor;
							dqParty.pop_front();
						}
						while( hFindedActor == CDnActor::s_hLocalActor );

						if( dqParty.empty() )
							continue;
					}
				}
				
			}
			
			else
			{			
				
				//// NOTE: Player characters do not need to be reloaded!
				//// Equip default weapons on protagonists
				//if( (eActorType == CDnActorState::Warrior) ||
				//	(eActorType == CDnActorState::Archer) ||
				//	(eActorType == CDnActorState::Soceress) ||
				//	(eActorType == CDnActorState::Cleric) )
				//{
				//	hFindedActor = CreateActor( iItemID, false, true );
				//	assert( hFindedActor && "Failed to create cutscene actor" ); 

				//	const char* astrDefaultSkinFileName[ 4 ] = { "Warrior.skn", "Archer.skn", "Soceress.skn", "Cleric.skn" };
				//	const char* astrDefaultAniFileName[ 4 ] = { "Warrior.ani", "Archer.ani", "Soceress.ani", "Cleric.ani" };
				//	const char* astrDefaultActFileName[ 4 ] = { "Warrior.act", "Archer.act", "Soceress.act", "Cleric.act" };

				//	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDEFAULTCREATE );

				//	//MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(hActor.GetPointer());

				//	//char *pStaticName = pActorSOX->GetFieldFromLablePtr( nClassID, "_StaticName" )->GetString();
				//	//char *szDummySkinName = pActorSOX->GetFieldFromLablePtr( nClassID, "_SkinName" )->GetString();

				//	MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(hFindedActor.GetPointer());
				//	_ASSERT( pPartsBody );
				//	pPartsBody->LoadSkin( CEtResourceMng::GetInstance().GetFullName( astrDefaultSkinFileName[eActorType] ).c_str(),
				//		CEtResourceMng::GetInstance().GetFullName( astrDefaultAniFileName[eActorType] ).c_str() );
				//	hFindedActor->LoadAction( CEtResourceMng::GetInstance().GetFullName( astrDefaultActFileName[eActorType] ).c_str() );

				//	char szLabel[32];
				//	for( int j=0; j<2; j++ ) {
				//		sprintf_s( szLabel, "_Weapon%d", j + 1 );
				//		int nWeaponIndex = pSox->GetFieldFromLablePtr( iItemID, szLabel )->GetInteger();
				//		if( nWeaponIndex != -1 ) {
				//			DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nWeaponIndex, 0 );
				//			hFindedActor->AttachWeapon( hWeapon, j, true );
				//		}
				//	}

				////	CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(hFindedActor.GetPointer());
				////	_ASSERT( pPlayerActor );
				////	pPlayerActor->SetBattleMode( true );

				//	//hFindedActor->CombineParts();
				//}
				//else


				// If monster or NPC is not found, load it separately.
				DNTableFileFormat* pActorSOX = GetDNTable( CDnTableDB::TACTOR );

				if( eActorType == CDnActorState::Npc )
				{
					hFindedActor = CreateActor( iItemID );
					assert( hFindedActor && "Failed to create cutscene actor" ); 

					// NPC
					DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNPC );

					vector<int> vlNPCIDList;
					pSox->GetItemIDListFromField( "_ActorIndex", iItemID, vlNPCIDList );
					_ASSERT( !vlNPCIDList.empty() && "NPC actor info not found in NPC table" );

					if( false == vlNPCIDList.empty() )
					{
						int iNPCTableID = (int)vlNPCIDList.front();
						char szWeaponLable[32];
						for( int j=0; j<2; j++ ) {
							sprintf_s( szWeaponLable, "_Weapon%dIndex", j + 1 );
							int nWeapon = pSox->GetFieldFromLablePtr( iNPCTableID, szWeaponLable )->GetInteger();
							if( nWeapon < 1 ) continue;
							DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nWeapon, _rand(), 0, 0, 0, 0, false, false, false );
							hFindedActor->AttachWeapon( hWeapon, j, true );
						}
					}
				}
				else
				{
					hFindedActor = CreateActor( iItemID );
					assert( hFindedActor && "Failed to create cutscene actor" ); 

					// Equip monsters with weapons by referencing the monster table
					DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );

					vector<int> vlMonsterIDList;
					pSox->GetItemIDListFromField( "_ActorTableID", iItemID, vlMonsterIDList );
					//_ASSERT( !vlMonsterIDList.empty() && "Monster actor info is missing from the Monster table" );
					if( false == vlMonsterIDList.empty() )
					{
						int iMonsterTableID = (int)vlMonsterIDList.front();

						// Players are naturally 0, and among monsters, only those set as boss have an actual monster table ID.
						// Load by the configured monster ID. If missing, trigger an assert and just load the monster.
						int iToolSelectedMonsterTableID = m_pCutSceneData->GetMonsterTableID( pActorName );
						if( 0 != iToolSelectedMonsterTableID )
						{
							bool bScaleLock = m_pCutSceneData->GetThisActorsScaleLock( pActorName );
							_ASSERT( false == bScaleLock );

							vector<int>::iterator iter = find( vlMonsterIDList.begin(), vlMonsterIDList.end(), iMonsterTableID );
							if( vlMonsterIDList.end() != iter )
							{
								iMonsterTableID = iToolSelectedMonsterTableID;

								// Set scale. Load at max size without randomizing.
								float fScale = pSox->GetFieldFromLablePtr( iMonsterTableID, "_SizeMax" )->GetFloat();
								EtVector3 vScale = EtVector3( fScale, fScale, fScale );
								static_cast<CDnMonsterActor*>(hFindedActor.GetPointer())->SetScale( fScale );
								hFindedActor->SetScale( vScale );
								hFindedActor->SetActionSignalScale( vScale );
							}
						}

						char szWeaponLable[32];
						for( int j=0; j<2; j++ )
						{
							sprintf_s( szWeaponLable, "_Weapon%dIndex", j + 1 );

							/*int nWeapon = pSox->GetFieldFromLablePtr( iMonsterTableID, szWeaponLable )->GetInteger();
							if( nWeapon < 1 ) continue;
							DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nWeapon, _rand(), 0, 0, 0, 0, false, false, false );
							hFindedActor->AttachWeapon( hWeapon, j, true );*/

							int nWeapon = pSox->GetFieldFromLablePtr( iMonsterTableID, szWeaponLable )->GetInteger();
							if( nWeapon < 1 )
							{
#ifdef _WORK
								tempLog.errorType = EWeaponError::EWeaponError_WeaponIndex;
								tempLog.MonsterID = iMonsterTableID;
								tempLog.bCutsceneWeapon[ j ] = false;
								tempLog.weapon[ j ] = nWeapon;
#endif // _WORK
								continue;							
							}

							DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nWeapon, _rand(), 0, 0, 0, 0, false, false, false );
							hFindedActor->AttachWeapon( hWeapon, j, true );
						
#ifdef _WORK
							if( !hWeapon || !hFindedActor->GetWeapon(j) )
							{
								if( !hWeapon )
									tempLog.errorType = EWeaponError::EWeaponError_Create;
								if( !hFindedActor->GetWeapon(j) )
									tempLog.errorType = EWeaponError::EWeaponError_Attach;
								
								tempLog.MonsterID = iMonsterTableID;
								tempLog.bCutsceneWeapon[ j ] = false;
								tempLog.weapon[ j ] = nWeapon;
							}
#endif // _WORK
							
						}
					}
				}

				m_vlhAdditionalLoadedActors.push_back( hFindedActor );
			}
			//else
			//if( false == bVillage )
			//{
			//	// For protagonist characters, authenticate with own party info so other party's characters don't get in.
			//	if( (hFindedActor->GetActorType() == CDnActorState::Warrior) ||
			//		(hFindedActor->GetActorType() == CDnActorState::Archer) ||
			//		(hFindedActor->GetActorType() == CDnActorState::Soceress) ||
			//		(hFindedActor->GetActorType() == CDnActorState::Cleric) )
			//	{
			//		CDnPartyTask::PartyStruct* pSelectedMember = NULL;

			//		// Check if there is an actor of that type in the party.
			//		int iNumParty = (int)vlParty.size();
			//		for( int iMember = 0; iMember < iNumParty; ++iMember )
			//		{
			//			CDnPartyTask::PartyStruct* pMember = vlParty.at( iMember );
			//			if( pMember->hActor == hFindedActor )
			//			{				
			//				pSelectedMember = pMember;
			//				break;
			//			}
			//		}
			//	}
			//}

		}

		//if( false == bVillage )
		//	continue;

		// Filter out if data exists for a cutscene that does not support Academic. /////////////
#if defined (PRE_ADD_ACADEMIC)
		if( pSceneInfo->bAcademicNotSupported )
		{
			if( CDnActorState::Academic == hFindedActor->GetActorType() )
				continue;
		}
#endif
		//////////////////////////////////////////////////////////////////////////

		hFindedActor->Show( true );

		// Set HP because it should not die immediately.
		if( 0 == hFindedActor->GetHP() )
		{
			if( bMonster )
				m_vlhDeadMonster.push_back( hFindedActor );

			hFindedActor->SetHP( 10 );
		}

		// Rotha: Player should not play a cutscene while mounted, so dismount from the horse.
		CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(hFindedActor.GetPointer());
		if(pPlayer && pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor())
			pPlayer->UnRideVehicle(true);			

		ActorToPlay.pActor = (void*)hFindedActor.GetPointer();
		ActorToPlay.hObject = hFindedActor->GetObjectHandle();
		if( hFindedActor->IsPlayerActor() )
			ActorToPlay.hMergedObject = static_cast<CDnPlayerActor*>(hFindedActor.GetPointer())->GetCombineObject();
		ActorToPlay.fRotYDegree = m_pCutSceneData->GetRegResRot( ActorToPlay.strActorName.c_str() );
		EtMatrixRotationY( &ActorToPlay.matRotY, ActorToPlay.fRotYDegree*ET_PI / 180.0f );
		ActorToPlay.vPos = m_pCutSceneData->GetRegResPos( ActorToPlay.strActorName.c_str() );
		hFindedActor->GetCachedAniIndexList( ActorToPlay.vlAniIndex );
		ActorToPlay.iNowAniIndex = 0;

		// Change action to stand, so signals from skills etc. do not trigger camera effects.
		// In cutscenes, AniFrame is adjusted directly instead of changing actions, so switch to stand to prevent existing action signals from firing.
		hFindedActor->CmdAction( "Stand" );

		// Disable actor culling while video is playing.
		if( ActorToPlay.hObject ) 
			ActorToPlay.hObject->EnableCull( false );

		if( ActorToPlay.hMergedObject )
			ActorToPlay.hMergedObject->EnableCull( false );

		
		// #71465 - [Other] Halloween Pumpkin Spirit graphics are not displayed in event movie.
		// - 
		if( pPlayer )
		{
			DnPartsHandle hParts = pPlayer->GetCashParts( CDnParts::PartsTypeEnum::CashFairy );
			if( hParts )
				hParts->ShowRenderBase( false );
		}


		// Do not process actor STE_Input/STE_InputHasPassiveSkill during cutscene.
		//hFindedActor->SetPlayingCutScene( true );

		// Do not play sound signals embedded in actor during cutscene.
		hFindedActor->SetPlaySignalSound( m_pCutSceneData->GetSceneInfo()->bUseSignalSound );

		m_pActorProcessor->AddObjectToPlay( &ActorToPlay );		



//#ifdef _WORK		
		if( tempLog.errorType != EWeaponError::EWeaponError_None ) 
		{
			DNTableFileFormat* pSoxWeapon = GetDNTable( CDnTableDB::TWEAPON );
			DNTableFileFormat*  pFileNameSox = GetDNTable( CDnTableDB::TFILE );

			if( pSoxWeapon && pFileNameSox )
			{
				std::string szSkinName;
				char buff[512] = {0,};	
				for( int i=0; i<2; ++i )
				{
					int weaponIndex = tempLog.weapon[i];
					if( tempLog.bCutsceneWeapon[ i ] == false &&
						pSoxWeapon->IsExistItem( weaponIndex ))
					{						
						CommonUtil::GetFileNameFromFileEXT(szSkinName, pSoxWeapon, weaponIndex, "_SkinName", pFileNameSox);			

						sprintf_s( buff, 512, "$$$$$ CutSceneLog >> ErrorType:%s, Monster:%d %s, Weapon:%s %d %s \n",
							arrStrError[ tempLog.errorType ], //tempLog.errorType  strErrorType,
							tempLog.MonsterID, ActorToPlay.strActorName.c_str(), 
							i==0?"Right":"Left", weaponIndex, szSkinName.c_str() );
						g_Log.Log( LogType::_FILELOG, buff );
					}

					memset( buff, 0, 512 );
				}
			}
		}

//#endif // _WORK



	}

}


// Quest cutscenes are never played in town.
void CDnCutSceneTask::PrepareQuestCutScene( DNTableFileFormat*  pActorTable, int nQuestIndex, int nQuestStep, DWORD dwQuestPlayerUniqueID )
{
	// Store my party info.
	deque<CDnPartyTask::PartyStruct*> dqParty;
	CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
	DWORD dwNumParty = pPartyTask->GetPartyCount();
	for( DWORD dwMember = 0; dwMember < dwNumParty; ++dwMember )
	{
		CDnPartyTask::PartyStruct* pParty = pPartyTask->GetPartyData( dwMember );
		dqParty.push_back( pParty );
	}

	// Am I the quest performer? In quest cutscenes, the quest performer becomes the protagonist.
	bool bMyQuestCutScene = false;
	CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));
	const TQuest* pQuest = pQuestTask->GetQuestIndex( nQuestIndex );
	if( pQuest )
	{
		if( pQuest->nQuestStep == nQuestStep )
			bMyQuestCutScene = true;
	}

	//bool bMyCharacterIsMain = false;

	bool bQuestPlayerSelected = false;

	// Is there only one appearing actor
	int iNumPlayerActor = 0;
	vector<string> vlActorResNames;
	int iNumActors = m_pCutSceneData->GetNumActors();
	for( int iActor = 0; iActor < iNumActors; ++iActor )
	{
		string strActorResName = m_pCutSceneData->GetActorByIndex( iActor );
		int iOffset = (int)strActorResName.find_last_of( "_", strActorResName.length() );
		strActorResName = strActorResName.substr( 0, iOffset );
		vlActorResNames.push_back( strActorResName );

		int iItemID = pActorTable->GetItemIDFromFieldCaseFree( "_StaticName", strActorResName.c_str() );
		CDnActorState::ActorTypeEnum eActorType = (CDnActorState::ActorTypeEnum)pActorTable->GetFieldFromLablePtr( iItemID, "_Class" )->GetInteger();
		bool bPlayerCharacter = (eActorType <= CDnActorState::Reserved6);
		if( bPlayerCharacter )
		{
			++iNumPlayerActor;
		}
	}

	// Get the designated main character actor name from cutscene.
	const SceneInfo* pSceneInfo = m_pCutSceneData->GetSceneInfo();
	const string& strMainCharName = pSceneInfo->strMainCharacterName;
	// Among actors used in cutscene, reuse currently rendered ones and newly load the others.	
	for( int iActor = 0; iActor < iNumActors; ++iActor )
	{
		S_CS_ACTOR_INFO ActorToPlay;
		const char* pActorName = m_pCutSceneData->GetActorByIndex( iActor );

		ActorToPlay.strActorName.assign( pActorName );

		string& strActorResName = vlActorResNames.at( iActor );

		// Stored in uppercase on the tool side. When searching the table, capitalize only the first letter.
		//transform( strActorResName.begin()+1, strActorResName.end(), strActorResName.begin()+1, tolower );

		int iItemID = pActorTable->GetItemIDFromFieldCaseFree( "_StaticName", strActorResName.c_str() );
		//assert( -1 != iItemID );

		CDnActorState::ActorTypeEnum eActorType = (CDnActorState::ActorTypeEnum)pActorTable->GetFieldFromLablePtr( iItemID, "_Class" )->GetInteger();
		bool bPlayerCharacter = eActorType <= CDnActorState::Reserved6;

		ActorToPlay.bPlayer = bPlayerCharacter; // bintitle.

		DnActorHandle hFindedActor;

		// When there is one appearing actor or at the protagonist slot, if I am the quest performer, make me the protagonist..
		// If I am not the quest performer, set a party member as the quest performer.
		if( bPlayerCharacter &&
			((1 == iNumPlayerActor) || (strMainCharName == pActorName)) &&
			false == bQuestPlayerSelected )
		{
			if( true == bMyQuestCutScene )
			{
				deque<CDnPartyTask::PartyStruct*>::iterator iter = dqParty.begin();
				for( iter; iter != dqParty.end(); ++iter )
				{
					if( (*iter)->hActor == CDnActor::s_hLocalActor )
					{
						hFindedActor = CDnActor::s_hLocalActor;
						dqParty.erase( iter );
						bQuestPlayerSelected = true;
						break;
					}
				}
			}
			else
			{
				deque<CDnPartyTask::PartyStruct*>::iterator iter = dqParty.begin();
				for( iter; iter != dqParty.end(); ++iter )
				{
					if( (*iter)->hActor->GetUniqueID() == dwQuestPlayerUniqueID )
					{
						hFindedActor = (*iter)->hActor;
						dqParty.erase( iter );
						bQuestPlayerSelected = true;
						break;
					}
				}
			}
		}

		// If there is only one protagonist actor, set my character as the protagonist.
		//DnActorHandle hFindedActor;
		//if( false == bMyCharacterIsMain && bPlayerCharacter )
		//{
		//	// If it's the protagonist spot or only one player appears and I am the quest performer, make my character the protagonist..
		//	if( strMainCharName == pActorName ||
		//		1 == iNumPlayerActor )
		//	{
		//		if( bMyQuestCutScene )
		//		{
		//			hFindedActor = CDnActor::s_hLocalActor;
		//			deque<CDnPartyTask::PartyStruct*>::iterator iter = dqParty.begin();
		//			for( iter; iter != dqParty.end(); ++iter )
		//			{
		//				if( (*iter)->hActor == hFindedActor )
		//				{
		//					dqParty.erase( iter );
		//					break;
		//				}
		//			}
		//			bMyCharacterIsMain = true;
		//		}
		//	}
		//}

		if( NULL == hFindedActor )
		{
			if( bPlayerCharacter )
			{
				// If player is not found, map anyone in party order excluding the main character.
				// Do not add players who became ghosts.
				if( false == dqParty.empty() )
				{
					while( false == dqParty.empty() )
					{
						DnActorHandle hActor = dqParty.front()->hActor;
						dqParty.pop_front();
				
						// Filter out if data exists for a cutscene that does not support Academic. /////////////
#if defined (PRE_ADD_ACADEMIC)
						if( pSceneInfo->bAcademicNotSupported )
						{
							if( CDnActorState::Academic == hActor->GetActorType() )
								continue;
						}
#endif
						//////////////////////////////////////////////////////////////////////////
						hFindedActor = hActor;
						break;
					}
				}
			}
		}

		if( NULL == hFindedActor )
		{
			// Find actor with the same ItemID. Find in excluded actor list and remove it.
			int iNumLiveActor = (int)m_dqhExceptActors.size();
			for( int iActorInner = 0; iActorInner < iNumLiveActor; ++iActorInner )
			{
				DnActorHandle hLiveActor = m_dqhExceptActors.at( iActorInner );

				if( hLiveActor->GetClassID() == iItemID )
				{
					bool bScaleLock = m_pCutSceneData->GetThisActorsScaleLock( pActorName );
					if( bScaleLock )
					{
						// When scale-locked, actors spawned with a non-default scale are not used in cutscenes.
						if( hLiveActor->GetScale()->x != 1.0f )
							continue;
					}
					else
					{
						// Players are naturally 0, and among monsters, only those set as boss have an actual monster table ID.
						// Select this actor on screen only if the monster table ID also matches.
						int iMonsterTableID = m_pCutSceneData->GetMonsterTableID( pActorName );
						if( 0 != iMonsterTableID && false == bPlayerCharacter )
						{
							// If it is a monster actor and matches the monster table ID set in the tool, pass 
							// Otherwise skip with continue.
							CDnMonsterActor* pMonsterActor = dynamic_cast<CDnMonsterActor*>( hLiveActor.GetPointer() );
							if( false == (pMonsterActor && (iMonsterTableID == pMonsterActor->GetMonsterClassID())) )
							{
								continue;
							}
						}
					}

					hFindedActor = hLiveActor;
					deque<DnActorHandle>::iterator iter = m_dqhExceptActors.begin();
					advance( iter, iActorInner );
					m_dqhExceptActors.erase( iter );
					break;
				}
			}
		}
		else
		{
			int iNumLiveActor = (int)m_dqhExceptActors.size();
			for( int iActorInner = 0; iActorInner < iNumLiveActor; ++iActorInner )
			{
				DnActorHandle hLiveActor = m_dqhExceptActors.at( iActorInner );

				if( hLiveActor == hFindedActor )
				{
					deque<DnActorHandle>::iterator iter = m_dqhExceptActors.begin();
					advance( iter, iActorInner );
					m_dqhExceptActors.erase( iter );
					break;
				}
			}
		}

		bool bMonster = (eActorType > CDnActorState::Reserved6 && eActorType < CDnActorState::Npc);
		if( NULL == hFindedActor )
		{
			// Do not add the actor if the player actor is not found in the exclude list.
			if( bPlayerCharacter )
			{
				//m_pActorProcessor->AddObjectToPlay( &ActorToPlay );
				m_CutScenePlayer.HideActor( ActorToPlay.strActorName.c_str() );
			}
			
			{
				// If monster or NPC is not found, load it separately.
				DNTableFileFormat* pActorSOX = GetDNTable( CDnTableDB::TACTOR );

				// NOTE: Player characters do not need to be reloaded!
				// Equip default weapons on protagonists
#ifdef PRE_ADD_ACADEMIC
				if( CDnActorState::Warrior <= eActorType &&
					eActorType <= CDnActorState::Academic )
#else
				if( CDnActorState::Warrior <= eActorType &&
					eActorType <= CDnActorState::Cleric )
#endif // #ifdef PRE_ADD_ACADEMIC
				{
					hFindedActor = CreateActor( iItemID, false, true );
					assert( hFindedActor && "Failed to create cutscene actor" ); 

#ifdef PRE_ADD_ACADEMIC
					const char* astrDefaultSkinFileName[ CDnActorState::Academic+1 ] = { "Warrior.skn", "Archer.skn", "Soceress.skn", "Cleric.skn", "Academic.skn" };
					const char* astrDefaultAniFileName[ CDnActorState::Academic+1 ] = { "Warrior.ani", "Archer.ani", "Soceress.ani", "Cleric.ani", "Academic.ani" };
					const char* astrDefaultActFileName[ CDnActorState::Academic+1 ] = { "Warrior.act", "Archer.act", "Soceress.act", "Cleric.act", "Academic.act" };
#else
					const char* astrDefaultSkinFileName[ CDnActorState::Cleric+1 ] = { "Warrior.skn", "Archer.skn", "Soceress.skn", "Cleric.skn" };
					const char* astrDefaultAniFileName[ CDnActorState::Cleric+1 ] = { "Warrior.ani", "Archer.ani", "Soceress.ani", "Cleric.ani" };
					const char* astrDefaultActFileName[ CDnActorState::Cleric+1 ] = { "Warrior.act", "Archer.act", "Soceress.act", "Cleric.act" };
#endif // #ifdef PRE_ADD_ACADEMIC

					DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDEFAULTCREATE );

					MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(hFindedActor.GetPointer());
					_ASSERT( pPartsBody );
					pPartsBody->LoadSkin( CEtResourceMng::GetInstance().GetFullName( astrDefaultSkinFileName[eActorType] ).c_str(),
						CEtResourceMng::GetInstance().GetFullName( astrDefaultAniFileName[eActorType] ).c_str() );
					hFindedActor->LoadAction( CEtResourceMng::GetInstance().GetFullName( astrDefaultActFileName[eActorType] ).c_str() );

					char szLabel[32];
					for( int j=0; j<2; j++ ) {
						sprintf_s( szLabel, "_Weapon%d", j + 1 );
						int nWeaponIndex = pSox->GetFieldFromLablePtr( iItemID, szLabel )->GetInteger();
						if( nWeaponIndex != -1 ) {
							DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nWeaponIndex, _rand(), 0, 0, 0, 0, false, false, false );
							hFindedActor->AttachWeapon( hWeapon, j, true );
						}
					}
				}
				else
				if( eActorType == CDnActorState::Npc )
				{
					hFindedActor = CreateActor( iItemID );
					assert( hFindedActor && "Failed to create cutscene actor" ); 

					// NPC
					DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNPC );

					vector<int> vlNPCIDList;
					pSox->GetItemIDListFromField( "_ActorIndex", iItemID, vlNPCIDList );
					_ASSERT( !vlNPCIDList.empty() && "NPC actor info not found in NPC table" );

					if( false == vlNPCIDList.empty() )
					{
						int iNPCTableID = (int)vlNPCIDList.front();
						char szWeaponLable[32];
						for( int j=0; j<2; j++ ) {
							sprintf_s( szWeaponLable, "_Weapon%dIndex", j + 1 );
							int nWeapon = pSox->GetFieldFromLablePtr( iNPCTableID, szWeaponLable )->GetInteger();
							if( nWeapon < 1 ) continue;
							DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nWeapon, _rand(), 0, 0, 0, 0, false, false, false );
							hFindedActor->AttachWeapon( hWeapon, j, true );
						}
					}
				}
				else
				{
					bMonster = true;
					hFindedActor = CreateActor( iItemID );
					assert( hFindedActor && "Failed to create cutscene actor" ); 

					// Equip monsters with weapons by referencing the monster table
					DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );

					vector<int> vlMonsterIDList;
					pSox->GetItemIDListFromField( "_ActorTableID", iItemID, vlMonsterIDList );
					//_ASSERT( !vlMonsterIDList.empty() && "Monster actor info is missing from the Monster table" );
					if( false == vlMonsterIDList.empty() )
					{
						int iMonsterTableID = (int)vlMonsterIDList.front();

						// Players are naturally 0, and among monsters, only those set as boss have an actual monster table ID.
						// Load by the configured monster ID. If missing, trigger an assert and just load the monster.
						int iToolSelectedMonsterTableID = m_pCutSceneData->GetMonsterTableID( pActorName );
						if( 0 != iToolSelectedMonsterTableID )
						{
							vector<int>::iterator iter = find( vlMonsterIDList.begin(), vlMonsterIDList.end(), iMonsterTableID );
							if( vlMonsterIDList.end() != iter )
							{
								iMonsterTableID = iToolSelectedMonsterTableID;

								// Set scale. Load at max size without randomizing.
								float fScale = pSox->GetFieldFromLablePtr( iMonsterTableID, "_SizeMax" )->GetFloat();
								EtVector3 vScale = EtVector3( fScale, fScale, fScale );
								static_cast<CDnMonsterActor*>(hFindedActor.GetPointer())->SetScale( fScale );
								hFindedActor->SetScale( vScale );
								hFindedActor->SetActionSignalScale( vScale );
							}
						}

						char szWeaponLable[32];
						for( int j=0; j<2; j++ )
						{
							sprintf_s( szWeaponLable, "_Weapon%dIndex", j + 1 );

							int nWeapon = pSox->GetFieldFromLablePtr( iMonsterTableID, szWeaponLable )->GetInteger();
							if( nWeapon < 1 ) continue;
							DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nWeapon, _rand(), 0, 0, 0, 0, false, false, false );
							hFindedActor->AttachWeapon( hWeapon, j, true );
						}
					}
				}

				m_vlhAdditionalLoadedActors.push_back( hFindedActor );
			}
			//else
			//if( false == bVillage )
			//{
			//	// For protagonist characters, authenticate with own party info so other party's characters don't get in.
			//	if( (hFindedActor->GetActorType() == CDnActorState::Warrior) ||
			//		(hFindedActor->GetActorType() == CDnActorState::Archer) ||
			//		(hFindedActor->GetActorType() == CDnActorState::Soceress) ||
			//		(hFindedActor->GetActorType() == CDnActorState::Cleric) )
			//	{
			//		CDnPartyTask::PartyStruct* pSelectedMember = NULL;

			//		// Check if there is an actor of that type in the party.
			//		int iNumParty = (int)vlParty.size();
			//		for( int iMember = 0; iMember < iNumParty; ++iMember )
			//		{
			//			CDnPartyTask::PartyStruct* pMember = vlParty.at( iMember );
			//			if( pMember->hActor == hFindedActor )
			//			{				
			//				pSelectedMember = pMember;
			//				break;
			//			}
			//		}
			//	}
			//}

		}

		hFindedActor->Show( true );

		// Set HP because it should not die immediately.
		if( 0 == hFindedActor->GetHP() )
		{
			if( bMonster )
				m_vlhDeadMonster.push_back( hFindedActor );

			hFindedActor->SetHP( 10 );
		}

		ActorToPlay.pActor = (void*)hFindedActor.GetPointer();
	
		ActorToPlay.hObject = hFindedActor->GetObjectHandle();
		ActorToPlay.fRotYDegree = m_pCutSceneData->GetRegResRot( ActorToPlay.strActorName.c_str() );
		EtMatrixRotationY( &ActorToPlay.matRotY, ActorToPlay.fRotYDegree*ET_PI / 180.0f );
		ActorToPlay.vPos = m_pCutSceneData->GetRegResPos( ActorToPlay.strActorName.c_str() );
		hFindedActor->GetCachedAniIndexList( ActorToPlay.vlAniIndex );
		ActorToPlay.iNowAniIndex = 0;

		// Disable actor culling while video is playing.
		if( ActorToPlay.hObject ) 
			ActorToPlay.hObject->EnableCull( false );

		if( ActorToPlay.hMergedObject ) 
			ActorToPlay.hMergedObject->EnableCull( false );

		// Change action to stand, so signals from skills etc. do not trigger camera effects.
		// In cutscenes, AniFrame is adjusted directly instead of changing actions, so switch to stand to prevent existing action signals from firing.
		hFindedActor->CmdAction( "Stand" );

		// Do not process actor STE_Input/STE_InputHasPassiveSkill during cutscene.
		//hFindedActor->SetPlayingCutScene( true );

		// Do not play sound signals embedded in actor during cutscene.
		hFindedActor->SetPlaySignalSound( m_pCutSceneData->GetSceneInfo()->bUseSignalSound );

		m_pActorProcessor->AddObjectToPlay( &ActorToPlay );
	}
}


void CDnCutSceneTask::StartPlay( void )
{
	if( NULL == m_pActorProcessor )
		return;

	m_CutScenePlayer.StartPlay( m_LocalTime );

	GetInterface().ShowCutSceneBlindDialog( true );

	// Hide nickname rendering
	int iNumActors = (int)m_dqhLiveActors.size();
	for( int iActor = 0; iActor < iNumActors; ++iActor )
	{
		DnActorHandle hActor = m_dqhLiveActors.at( iActor );
		hActor->ShowHeadName( false );
	}

	// At cutscene start, we don't know whether the player actor has the weapon drawn or sheathed.
	// Match weapon state of player actors in cutscene based on their action at cutscene start.
	// Restore original state when cutscene ends.
	int iNumActor = m_pActorProcessor->GetNumActors();
	m_vlIsPlayerBattleMode.assign( iNumActor, 0 );
	for( int iActor = 0; iActor < iNumActor; ++iActor )
	{
		S_CS_ACTOR_INFO* pActorInfo = m_pActorProcessor->GetActorInfoByIndex( iActor );
		
		CDnActor* pDnActor = static_cast<CDnActor*>(pActorInfo->pActor);
		CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(pDnActor);
		if( pPlayerActor && false == pPlayerActor->IsDie() )
		{
			// Any first action without "Normal" attached is an attack action
			// Regardless of the user's current state, unequip weapon if it is an action with Normal 
			// Otherwise switch to attack state.
			const ActionInfo* pActionInfo = m_pCutSceneData->GetThisActorsActionInfoByIndex( pActorInfo->strActorName.c_str(), 0 );

			size_t iFound = pActionInfo->strAnimationName.find( "Normal" );
			if( iFound == string::npos )
			{
				pPlayerActor->SetBattleMode( true );
				m_vlIsPlayerBattleMode.at( iActor ) = 1;
			}
			else
			{
				pPlayerActor->SetBattleMode( false );
			}

			// Change face back to normal.
			switch( pPlayerActor->GetActorType() )
			{
				case CDnActorState::Warrior:
					pPlayerActor->SetFaceAction( "Warrior" );
					break;

				case CDnActor::Archer:
					pPlayerActor->SetFaceAction( "Archer" );
					break;

				case CDnActor::Soceress:
					pPlayerActor->SetFaceAction( "Soceress" );
					break;

				case CDnActor::Cleric:
					pPlayerActor->SetFaceAction( "Cleric" );
					break;

#ifdef PRE_ADD_ACADEMIC
				case CDnActor::Academic:
					pPlayerActor->SetFaceAction( "Academic" );
					break;
#endif //#ifdef PRE_ADD_ACADEMIC

#ifdef PRE_ADD_KALI
				case CDnActor::Kali:
					pPlayerActor->SetFaceAction( "Kali" );
					break;
#endif // #ifdef PRE_ADD_KALI
//[OK]
					// #ifdef PRE_ADD_ASSASSIN
			}

			// Note: If a cutscene plays while under a state effect, the player character's color may appear altered.
			//pPlayerActor->RemoveAllBlow();

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
			// For players, reconstruct resources such as effect signals attached to actions.
			pPlayerActor->RecomposeAction();
#endif // #ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
		}
	}

	CEtSoundEngine::GetInstance().FadeVolume( "NULL", 1.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );
}



void CDnCutSceneTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
//#if defined(TEST_CUTSCENE)
//	if( m_bFirstLoop )
//	{
//		StartPlay();
//		m_bFirstLoop = false;
//	}
//#endif

	//if( m_CutScenePlayer.IsEndScene() )
	//{
	//	delete this;
	//	return;
	//}

	// Process Input Device - chat window handling
	CInputDevice::GetInstance().Process( LocalTime, fDelta );

	MatrixEx matEx;
	EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
	if( hCamera )
		matEx = *hCamera->GetInvViewMat();
	else {
		DnCameraHandle hGameCam = CDnCamera::GetActiveCamera();
		if( hGameCam ) matEx = *hGameCam->GetMatEx();
	}

	if( hCamera )
	{
		m_pWorld->Update( matEx.m_vPosition, matEx.m_vZAxis, hCamera->GetCameraFar() * 1000.f );

		// #38660 Sometimes camera retrieval fails, so fixed to get the player camera directly in that case.
		if( !m_CutScenePlayer.GetCamera() )
			m_CutScenePlayer.SetCamera( hCamera );
	}

	// Etc Object Process
	CDnEtcObject::ProcessClass( LocalTime, fDelta );

	m_pWorld->Process( LocalTime, fDelta );

	m_CutScenePlayer.Process( LocalTime );

	// Set camera position as Listner

	// Process Sound
	if( CEtSoundEngine::IsActive() ) 
	{
		CEtSoundEngine::GetInstance().Process( fDelta );
		CEtSoundEngine::GetInstance().SetListener( matEx.m_vPosition, matEx.m_vZAxis, matEx.m_vYAxis );
	}

	// Note : UI Process
	EtInterface::Process( fDelta );

	// Do not consider projectiles firing during the current cutscene.
	// Weapon(Projectile) Process
	//CDnWeapon::ProcessClass( LocalTime, fDelta );

	EternityEngine::ClearScreen( 0xFFFFFFFF, 1.0f, 0, false );
	EternityEngine::RenderFrame( fDelta );
	EtInterface::Render( fDelta );
	EternityEngine::ShowFrame( NULL );
}

void CDnCutSceneTask::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime /* = -1  */ )
{
	if( !m_bRequestSkip ) {
		if( (nReceiverState & IR_KEY_DOWN) 
#ifdef PRE_ADD_CUTSCENE_PADSKIP
			|| (nReceiverState & IR_JOYPAD_DOWN)
#endif
			)
		{
			if( IsPushKey( DIK_ESCAPE ) 
#ifdef PRE_ADD_CUTSCENE_PADSKIP
				|| IsPushJoypadButton( IW_UI(IW_SYSTEM) )
#endif
				) {
				CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
//				if( CDnWorld::IsActive() && CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage ) {
				if( pTask ) {
					if( pTask->IsControlQuestScriptCutScene() )
						pTask->SkipCutScene();
					else {
						// GMs cannot skip.
						if( CDnPartyTask::IsActive() ) {
							CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyDataFromSessionID( CDnBridgeTask::GetInstance().GetSessionID() );
							if( pStruct && pStruct->bGMTrace )
								return;
						}
						m_bRequestSkip = true;
						SendSkipCutScene( CDnBridgeTask::GetInstance().GetSessionID() );
					}
				}
			}
		}
	}
}

void CDnCutSceneTask::FadeOutSkip()
{
	if( m_bSetFadeModeCalledForSkip )
		return;

	CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
	m_pSkipFade->SetCallback( pTask );
	m_pSkipFade->SetFadeColor( 0x00000000, 0xff000000, true );
	m_pSkipFade->SetFadeTime( 1.f );
	m_pSkipFade->SetFadeMode( CDnFadeInOutDlg::modeBegin );

	m_bSetFadeModeCalledForSkip = true;
}

void CDnCutSceneTask::Finalize( void )
{
	if( NULL == m_pActorProcessor )
		return;

	if( !m_CutScenePlayer.IsEndScene() ) m_CutScenePlayer.StopPlay();
//	CDnInterface::GetInstance().UseEventMessage( true );
	// Show previously hidden item again
	CDnDropItem::ShowDropItems( true );

	// Remove all effects created during cutscene
	int iNumActor = m_pActorProcessor->GetNumActors();
	for( int iActor = 0; iActor < iNumActor; ++iActor )
	{
		CDnActor* pActor = static_cast<CDnActor*>(m_pActorProcessor->GetActorInfoByIndex( iActor )->pActor);
		pActor->ReleaseSignalImp();

		// Restore player actor weapon state
		CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(pActor);
		if( pPlayerActor && m_vlIsPlayerBattleMode.at(iActor) == 1 )
		{
			//pPlayerActor->SetActionQueue( "PullOut_Weapon" );
			pPlayerActor->SetBattleMode( true );
			for( int j=0; j<2; j++ ) {
				pPlayerActor->ShowWeapon( j, true );
				pPlayerActor->ShowCashWeapon( j, true );
			}
		}

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
		// For players, reconstruct resources such as effect signals attached to actions.
		if( pPlayerActor )
		{
			static_cast<CDnPlayerActor*>(pActor)->RecomposeAction();
		}
#endif // #ifdef PRE_FIX_CLIENT_MEMOPTIMIZE

		if( pPlayerActor ) pPlayerActor->ResetHideWeaponBySignalInCutScene();
	}

	int iNumMonster = (int)m_vlhDeadMonster.size();
	for( int iMonster = 0; iMonster < iNumMonster; ++iMonster )
	{
		DnActorHandle hDeadMonster = m_vlhDeadMonster.at( iMonster );
		hDeadMonster->SetHP( 0 );
		if( !hDeadMonster->IsShow() ) hDeadMonster->Show( true );
		hDeadMonster->Show( false );
		hDeadMonster->ShowWeapon( 0, false );
		hDeadMonster->ShowWeapon( 1, false );
	}
	 
	// Among excluded and included actors, hide dead monster actors even if used in the scene! 
	// Otherwise it remains after the cutscene ends

	// Show previously excluded actors again
	int iNumExceptActor = (int)m_dqhExceptActors.size();
	for( int iActor = 0; iActor < iNumExceptActor; ++iActor )
	{
		DnActorHandle hActor = m_dqhExceptActors.at( iActor );
		if( !hActor ) continue;
		if( !hActor->IsPlayerActor() && hActor->IsDie() ) continue;

		if( hActor->IsShow() ) hActor->Show( false );
		hActor->Show( true );

		// Since hidden actors are not touched...
		//bool bPlayerCharacter = hActor->GetActorType() <= CDnActorState::Reserved6;
		//if( bPlayerCharacter )
		//{
		//	// For player actors, if in battle mode the weapon attach bone may have changed, so set battle mode
		//	CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(hActor.GetPointer());
		//	if( pPlayerActor->IsBattleMode() )
		//		pPlayerActor->SetBattleMode( true );
		//}
	}

	// Restore culling and position
	int iNumLiveActors = (int)m_dqhLiveActors.size();
	for( int iActor = 0; iActor < iNumLiveActors; ++iActor )
	{
		DnActorHandle hActor = m_dqhLiveActors.at( iActor );
		if( !hActor ) continue;
		if( !hActor->IsPlayerActor() && hActor->IsDie() ) 
		{
			// A dead monster might have been used in a cutscene, and post-death handling 
			// Since some cases remain unremoved, hide dead ones immediately.
			if( !hActor->IsShow() ) hActor->Show( true );
			hActor->Show( false );
			continue;
		}

		if( hActor->GetObjectHandle() )
			hActor->GetObjectHandle()->EnableCull( true );

		if( hActor->IsPlayerActor() )
		{
			EtAniObjectHandle hMergedObject = static_cast<CDnPlayerActor*>(hActor.GetPointer())->GetCombineObject();
			if( hMergedObject )
				hMergedObject->EnableCull( true );
		}
		hActor->ShowHeadName( true );
		//hActor->SetPlayingCutScene( false );
		hActor->SetPlaySignalSound( true );
		if( hActor->IsShow() )  hActor->Show( false );
		hActor->Show( true );
		*(hActor->GetMatEx()) = m_vlOriMatExWorlds.at( iActor );
		hActor->ResetMove();
		hActor->ResetLook();
		hActor->SetStaticPosition( *hActor->GetPosition() );

		// Cutscene player computes and sets ani distance directly.
		// Since scale values can be present in the world matrix, when the engine calculates animations 
		// Because it is applied to the Y-axis ani distance enabled by default.
		// Currently decided that monster scale does not affect animation key values.
		if( hActor->GetAniObjectHandle() )
			hActor->GetAniObjectHandle()->SetCalcPositionFlag( CALC_POSITION_Y );

		CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
		if( pPlayer )
		{
			pPlayer->SetBattleMode( m_bVecPlayerBattleMode.at( iActor ) );

			// Rotha: Dismount during cutscene playback since riding isn't allowed, and remount when the cutscene ends; mounts based on VehicleInfo so unmounted state is safe too.
			if(pPlayer && !pPlayer->IsDestroy() && !pPlayer->IsDie() && pPlayer->IsCanVehicleMode() && (pPlayer->GetVehicleInfo().Vehicle[Vehicle::Slot::Body].nItemID != 0))
				pPlayer->RideVehicle(pPlayer->GetVehicleInfo(),true);
		}

#ifdef PRE_ADD_CASH_COSTUME_AURA 
		if( pPlayer )
		{
			if( pPlayer->IsEnableCostumeAura() )
			{
				pPlayer->LoadCostumeAuraEffect(true);
			}
		}
#endif

		// #71465 - [Other] Halloween Pumpkin Spirit graphics are not displayed in event movie.
		if( pPlayer )
		{
			DnPartsHandle hParts = pPlayer->GetCashParts( CDnParts::PartsTypeEnum::CashFairy );
			if( hParts )
				hParts->ShowRenderBase( true );
		}


		if( !hActor->IsDie() ) 
		{
			if( hActor->IsNpcActor() )
			{
				int iActionIndex = hActor->GetElementIndex( m_szOriAction.at( iActor ).c_str() );
				hActor->SetActionIndex( iActionIndex );
			}
				//hActor->SetActionQueue( m_szOriAction.at( iActor ).c_str() );
			else
			{
				hActor->CmdStop( m_szOriAction.at( iActor ).c_str() );
				// Since cutscene player sets action index directly, actions queued in ActionQueue get ignored.
				// So set the current action immediately. (#14014)
				hActor->SetAction( m_szOriAction.at( iActor ).c_str(), 0.0f, 0.0f );
			}

			hActor->ResetStateEffects();
		}
	}

	// Release additionally loaded actors
	int iAdditionalLoadActors = (int)m_vlhAdditionalLoadedActors.size();
	for( int iActor = 0; iActor < iAdditionalLoadActors; ++iActor ) {
		DnActorHandle hActor = m_vlhAdditionalLoadedActors.at( iActor );
		//		hActor->Show( false );
		hActor->ShowWeapon( 0, false );
		hActor->ShowWeapon( 1, false );
		if( !hActor ) continue;
		SAFE_RELEASE_SPTR( hActor );
	}

	// Restore the action the action prop was performing
	int iNumActProp = (int)m_vlActPropDefault.size();
	for( int iActProp = 0; iActProp < iNumActProp; ++iActProp )
	{
		S_ACT_PROP_DEFAULT& PropDefault = m_vlActPropDefault.at( iActProp );
		if( PropDefault.hProp ) 
		{
			PropDefault.pActProp->SetAction( PropDefault.strDefaultAction.empty() ? "" : PropDefault.strDefaultAction.c_str(), 0.0f, 0.0f );
			PropDefault.hProp->Show( PropDefault.bShow );

			// Added defensive code because hObject appears to be invalid in some cases during FinalTest.
			EtAniObjectHandle hObject = PropDefault.hProp->GetObjectHandle();
			if( hObject )
				hObject->ShowObject( PropDefault.bShow );
		}
	}

	// Restore camera
	DnCameraHandle hDnCamera = CDnCamera::GetActiveCamera();
	hDnCamera->SetCameraInfo( m_OriginalCameraInfo);

	// Restore DOF value
	if( m_pDOFFilter )
	{
		m_pDOFFilter->SetNearDOFStart( m_fNearStart );
		m_pDOFFilter->SetNearDOFEnd( m_fNearEnd );
		m_pDOFFilter->SetFarDOFStart( m_fFarStart );
		m_pDOFFilter->SetFarDOFEnd( m_fFarEnd );
		m_pDOFFilter->SetFocusDistance( m_fFocusDist );
		m_pDOFFilter->SetNearBlurSize( m_fNearBlurSize );
		m_pDOFFilter->SetFarBlurSize( m_fFarBlurSize );
	}

	if( hDnCamera ) {
		// Attach to the player.
		if( hDnCamera->GetCameraType() == CDnCamera::PlayerCamera ) {
			CDnPlayerCamera* pPlayerCamera = static_cast<CDnPlayerCamera*>(hDnCamera.GetPointer());
			
			if(CDnActor::s_hLocalActor)
			{
				CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

				if(pPlayer && pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor())
				{
					pPlayerCamera->AttachActor(pPlayer->GetMyVehicleActor()->GetActorHandle());
				}
				else
				{
					pPlayerCamera->AttachActor( CDnActor::s_hLocalActor );
				}
			}
		}
	}

	GetInterface().ShowCutSceneBlindDialog( false );

	bool bVillage = (CDnWorld::MapTypeEnum::MapTypeVillage == m_pWorld->GetMapType());
	if( false == bVillage )
	{
		CDnPartyTask* pPartyTask  = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
		DWORD dwNumParty = pPartyTask->GetPartyCount();
		for( DWORD dwMember = 0; dwMember < dwNumParty; ++dwMember )
		{
			CDnPartyTask::PartyStruct* pParty = pPartyTask->GetPartyData( dwMember );
			CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(pParty->hActor.GetPointer());
			if( pPlayerActor->IsGhost() )
			{
				pPlayerActor->SwapSingleSkin( 499 + pPlayerActor->GetClassID() );
				pPlayerActor->ResetCustomAction();
				pPlayerActor->ResetMixedAnimation( false );

				pPlayerActor->SetHP( 0 );

				if( CDnActor::s_hLocalActor == pParty->hActor )
				{
					CDnWorldEnvironment* pWorldEnv = CDnWorld::GetInstance().GetEnvironment();
					pWorldEnv->ControlColorFilter( CDnWorldEnvironment::Saturation, 0.0f, 500 );
				}

				CDnActor::s_hLocalActor->CmdAction( "Stand" );
			}
		}
	}
	else
	// Issue "Stand" action command to local actor
	if( CDnActor::s_hLocalActor ) 
	{
		CDnActor::s_hLocalActor->CmdAction( "Stand" );
	}

	//if( CDnActor::s_hLocalActor )
	//	CDnActor::s_hLocalActor->SetPlayingCutScene( false );
	CGlobalInfo::GetInstance().SetPlayingCutScene( false );

	SAFE_DELETE( m_pActorProcessor );
	CDnWorld::GetInstance().EnableTriggerEventCallback( true );

	if( m_bAutoFadeIn )
		CEtSoundEngine::GetInstance().FadeVolume( NULL, 1.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );

	CInputDevice::GetInstance().HoldAbsenceCheckTimer(CInputDevice::eABSENCE_PARTY, false);

	if( m_bCheatCutScene )
	{
		FadeOutSkip();
	}
	
#ifdef PRE_FIX_SUMMON_ACTOR_HPBAR 
	if(!m_dqhLiveActors.empty())
	{
		for(int i = 0 ; i < (int)m_dqhLiveActors.size() ; ++i)
		{
			DnActorHandle hActor = m_dqhLiveActors.at(i);
			if(!hActor) continue;
			if(hActor->IsDie()) continue;
			GetInterface().ShowEnemyGauge( hActor, true );
		}
	}
#endif // PRE_FIX_SUMMON_ACTOR_HPBAR
}

bool CDnCutSceneTask::RenderScreen( LOCAL_TIME LocalTime, float fDelta )
{
	return !m_bLoadingComplete;
}


bool CDnCutSceneTask::AttachItem( int nItemID, int nEnchantLevel, int nSubArg1, int nLookItemID )
{
	if( !m_hActor ) return true;

	
	CDnPlayerActor *pActor = (CDnPlayerActor *)m_hActor.GetPointer();
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
	if( !pSox ) return false;
	if( !pSox->IsExistItem( nItemID ) ) return false;

	eItemTypeEnum ItemType = CDnItem::GetItemType( nItemID );
	if( ItemType == ITEMTYPE_HAIRDYE )
	{
		DWORD dwColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();
		pActor->SetPartsColor( MAPartsBody::HairColor, dwColor );
	}
	else if( ItemType == ITEMTYPE_EYEDYE )
	{
		DWORD dwColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();
		pActor->SetPartsColor( MAPartsBody::EyeColor, dwColor );
	}
	else if( ItemType == ITEMTYPE_SKINDYE )
	{
		DWORD dwColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();
		pActor->SetPartsColor( MAPartsBody::SkinColor, dwColor );
	}
	else if( ItemType == ITEMTYPE_PARTS )
	{
		DnPartsHandle hParts = CDnParts::CreateParts( nItemID, 0, 0, 0, 0, 0, false, nLookItemID);
		if( hParts )
		{
			//hParts->SetRTTRenderMode( true, this );
			bool bCash = CDnItem::IsCashItem( nItemID );

			// Thought no special handling was needed since the actor is used as-is, but
			// Had to process without Combine for performance, which caused an issue with one-piece outfits.
			// One-piece outfit handling is all done when combining, but missing when not combining.
			// This is the same issue even on the login character selection screen where Combine is not performed.
			// Instead, on the login screen, after trying on normal cash outfits then equipping a one-piece outfit (or vice versa: equipping normal cash parts after a one-piece outfit)
			// Because it wasn't done, there was no issue.
			//
			// Currently since we do not combine, we clear old info without remembering it when equipping new parts, but
			// Because of this, when attaching another cash upper part while a one-piece outfit is equipped, info for previously worn lower, gloves, and pants is missing.
			//
			// To prevent this, we need to remember normal parts and cash parts separately just like character equipment info,
			// This ends up pulling out all the equipment functionality from ItemTask.
			// Since this render avatar code was meant to touch only the rendering routine as much as possible (that is why AttachItem is supported while DetachItem was intentionally omitted).
			// Decided not to support this feature.
			//
			// Still, to make it look as natural as possible, handle one-piece outfits as an exception and keep their info.

			if( !bCash )
			{
				// Since RenderAvatar does not call CombineParts, when a normal item comes in, check matching cash item
				// If there is a cash item, skip equipping and move on.
				bool bEquip = true;
				if( hParts->GetPartsType() >= CDnParts::Helmet && hParts->GetPartsType() <= CDnParts::Foot )
				{
					if( pActor->GetCashParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() - 2 ) ) )
					{
						bEquip = false;
					}
				}

				//// Check if it is an equipped one-piece outfit part (looks like one-piece sub-parts come through here?).
				//for( int i = 0; i < (int)m_vecPartsInfo.size(); ++i )
				//{
				//	if( m_vecPartsInfo[i].nType == hParts->GetPartsType() )
				//	{
				//		bEquip = false;
				//		break;
				//	}
				//}

				if( bEquip )
				{
					pActor->DetachParts( hParts->GetPartsType() );
					pActor->AttachParts( hParts, hParts->GetPartsType(), true, false );
					if( hParts->GetObjectHandle() ) hParts->GetObjectHandle()->SetRenderType( RT_SCREEN );

					if( hParts->GetPartsType() == CDnParts::Hair )
					{
						// For hair, alpha blending is sometimes not set in the tool. Just force-enable it.
						if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
							hParts->GetObjectHandle()->GetSkinInstance()->EnableAlphaBlend( true );
					}
				}
				else
				{
					SAFE_RELEASE_SPTR( hParts );
				}
			}
			else
			{
				// If currently wearing a one-piece outfit,
				//if( m_vecPartsInfo.size() )
				//{
				//	// Check if the currently equipped part belongs to a one-piece outfit slot.
				//	// Currently all one-piece outfits have the same subparts, but if this changes,
				//	// Not only check hParts->GetPartsType(), probably need to fetch and check sub-parts too.
				//	bool bDetachOnepiece = false;
				//	for( int i = 0; i < (int)m_vecPartsInfo.size(); ++i )
				//	{
				//		if( (m_vecPartsInfo[i].nType - 2) == hParts->GetPartsType() )
				//		{
				//			bDetachOnepiece = true;
				//			break;
				//		}
				//	}

				//	if( bDetachOnepiece )
				//	{
				//		std::vector<SPartsInfo> vecPartsInfo;
				//		vecPartsInfo = m_vecPartsInfo;
				//		m_vecPartsInfo.clear();

				//		// Unequip one-piece outfit first, (the first index is the one-piece parts slot.)
				//		pActor->DetachCashParts( (CDnParts::PartsTypeEnum)( vecPartsInfo[0].nType - 2 ) );

				//		// After running everything with old info
				//		// Since all types were added first, just fetch the matching itemID for the type and re-save.
				//		for( int i = 0; i < (int)vecPartsInfo.size(); ++i )
				//			AttachItem( vecPartsInfo[i].nItemID );

				//		// Perform the routine below just like equipping a normal cash item.
				//	}
				//}

#ifdef PRE_ADD_TRANSPARENCY_COSTUME
				bool bTransparentSkin = CDnParts::IsTransparentSkin( hParts->GetClassID(), pActor->GetClassID() );
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME

				// If the outfit to equip is a one-piece, separately store the parts in the slots covered by it.
				// If only sub parts are processed, the self slot is omitted, so add self separately.
				// (Must remember this so that when wearing cash gloves while wearing a one-piece outfit, it can revert to the previously worn top.)
				if( hParts->IsExistSubParts() )
				{
					SPartsInfo Info;
					Info.nType = (int)hParts->GetPartsType() + 2;	// Since one-piece outfits are cash items, add 2 to match the normal item type.
					Info.nItemID = 0;
					m_vecPartsInfo.push_back( Info );
					for( int i = 0; i < hParts->GetSubPartsCount(); ++i )
					{
						Info.nType = (int)hParts->GetSubPartsIndex(i) + 2;	// The written subtype is also a cash item type, so add +2 to match the normal item type.
						m_vecPartsInfo.push_back( Info );
					}

					// Since all types are already populated, just fetch the item IDs matching the type and re-save.
					for( int i = 0; i < (int)m_vecPartsInfo.size(); ++i )
					{
						if( pActor->GetCashParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType - 2 ) ) )
						{
							m_vecPartsInfo[i].nItemID = pActor->GetCashParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType - 2 ) )->GetClassID();
							pActor->DetachCashParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType - 2 ) );
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
							// Show normal parts appearance when wearing a transparent one-piece costume.
							if( bTransparentSkin )
							{
								DnPartsHandle hNormalParts = pActor->GetParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) );
								if( hNormalParts && !hNormalParts->GetObjectHandle() )
								{
									DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );
#ifdef PRE_FIX_MEMOPT_EXT
									std::string szAniName;
									CommonUtil::GetFileNameFromFileEXT(szAniName, pActorSox, pActor->GetClassID(), "_AniName");
#else
									std::string szAniName = pActorSox->GetFieldFromLablePtr( pActor->GetClassID(), "_AniName" )->GetString();
#endif
									hNormalParts->CreateObject( m_hActor, szAniName.c_str() );
								}
							}
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME
						}
						else if( pActor->GetParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) ) )
						{
							m_vecPartsInfo[i].nItemID = pActor->GetParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) )->GetClassID();
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
							if( !bTransparentSkin ) pActor->GetParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) )->FreeObject();
#else
							pActor->DetachParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) );
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME
						}
						// If detached properly on the actor side, all default parts would be present even in empty slots.
						// Just in case, add the handling below.
						else if( m_vecPartsInfo[i].nType >= CDnParts::DefaultPartsType_Min && m_vecPartsInfo[i].nType <= CDnParts::DefaultPartsType_Max )
						{
							m_vecPartsInfo[i].nItemID = pActor->GetDefaultPartsInfo( (CDnParts::PartsTypeEnum)m_vecPartsInfo[i].nType );
						}
					}
				}

				// Since RenderAvatar does not call CombineParts, remove normal items here.
				if( hParts->GetPartsType() >= CDnParts::CashHelmet && hParts->GetPartsType() <= CDnParts::CashFoot )
				{
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
					// Change to FreeObject instead of Detach due to transparent one-piece costume, so CreateObject can be called again later when needed.
					DnPartsHandle hNormalParts = pActor->GetParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() + 2 ) );
					if( hNormalParts && !bTransparentSkin ) hNormalParts->FreeObject();
#else
					if( pActor->GetParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() + 2 ) ) )
						pActor->DetachParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() + 2 ) );
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME
				}

				if( nSubArg1 == 0 )
				{
					pActor->DetachCashParts( hParts->GetPartsType() );
					pActor->AttachCashParts( hParts, hParts->GetPartsType(), true, false );
				}
				else if( nSubArg1 == 1 )
				{
					if( hParts->GetPartsType() == CDnParts::CashRing )
					{
						pActor->DetachCashParts( CDnParts::CashRing2 );
						pActor->AttachCashParts( hParts, CDnParts::CashRing2, true, false );
					}
				}
				//if( hParts->GetObjectHandle() ) hParts->GetObjectHandle()->SetRenderType( RT_TARGET );
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
				else if( bTransparentSkin )
				{
					DnPartsHandle hNormalParts = pActor->GetParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() + 2 ) );
					if( hNormalParts && !hNormalParts->GetObjectHandle() )
					{
						DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );
#ifdef PRE_FIX_MEMOPT_EXT
						std::string szAniName;
						CommonUtil::GetFileNameFromFileEXT(szAniName, pActorSox, pActor->GetClassID(), "_AniName");
#else
						std::string szAniName = pActorSox->GetFieldFromLablePtr( pActor->GetClassID(), "_AniName" )->GetString();
#endif
						hNormalParts->CreateObject( m_hActor, szAniName.c_str() );
					}
				}
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME

				// Do this only for parts for now. Weapons rarely seem to have this case.
				if( hParts->GetPartsType() == CDnParts::CashHelmet )
				{
					// Some headwear parts use alpha textures at mesh edges even though alpha blending is not enabled in the tool.
					// Force-enable it so it displays properly even like this.
					// However, after making PostProcess preserve the alpha value as-is,
					// The issue where border alpha looked weird seems mostly resolved.
					// So force disabling alpha blend.
					//if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
					//	hParts->GetObjectHandle()->GetSkinInstance()->EnableAlphaBlend( true );
				}
				else if( hParts->GetPartsType() == CDnParts::CashWing )
				{
					// For wings, force alpha blend and two-pass on for clean rendering.
					if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
					{
						hParts->GetObjectHandle()->GetSkinInstance()->EnableAlphaBlend( true );
						hParts->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
					}
				}
			}
		}

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
		// Like login character select screen, CombineParts is not called, so forcibly call hiding right glove mesh when warrior equips gauntlet.
		pActor->OnCombineParts();
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
	}
	else if( ItemType == ITEMTYPE_WEAPON )
	{
		DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nItemID, 0, 0, nEnchantLevel, 0, 0, false, false, true, ITEMCLSID_NONE );
		if( hWeapon )
		{			
			bool bCash = CDnItem::IsCashItem( nItemID );

			int nEquipIndex = CDnCharStatusDlg::EQUIPTYPE_2_EQUIPINDEX( hWeapon->GetEquipType() ) - EQUIP_WEAPON1;
			ASSERT( nEquipIndex == 0 || nEquipIndex == 1 );

			if( !bCash )
			{
				pActor->DetachWeapon( nEquipIndex );
				pActor->AttachWeapon( hWeapon, nEquipIndex, true );
			}
			else
			{
				pActor->DetachCashWeapon( nEquipIndex );
				pActor->AttachCashWeapon( hWeapon, nEquipIndex, true, false );
			}

			if( hWeapon->GetObjectHandle() )
			{
				// Combining a normal mace with a cash flail is problematic.
				// Too much resource work would be required (new mace animations, mace act files, etc.), so do not force disabling simulation.
				//hWeapon->GetObjectHandle()->DisableSimulation();
				//hWeapon->GetObjectHandle()->SetRenderType( RT_TARGET );
			}
		}
	}
	// Face product and hairstyle product.
	else if( ItemType == ITEMTYPE_FACIAL || ItemType == ITEMTYPE_HAIRDRESS )
	{
		int nParam = pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();
		AttachItem( nParam );

		// Since alpha objects are not sorted by distance, re-equip the cash hat when changing Hair so it renders behind Hair.
		if( ItemType == ITEMTYPE_HAIRDRESS )
		{
			if( pActor->GetCashParts( CDnParts::CashHelmet ) )
			{
				DnPartsHandle hHelmet = pActor->GetCashParts( CDnParts::CashHelmet );
				if (!hHelmet)
				{
#ifdef _WORK
					std::wstring str = FormatW(L"DnRenderAvatar::AttachItem - Character CashHelmet does not exist : (%d)", nItemID);
					GetInterface().AddChatMessage(CHATTYPE_NOTICE, L"", str.c_str(), true);
#endif
					return false;
				}

				int nAttachHelmetId = (hHelmet->GetLookItemID() != ITEMCLSID_NONE) ? hHelmet->GetLookItemID() : hHelmet->GetClassID();
				pActor->DetachCashParts( CDnParts::CashHelmet );
				AttachItem(nAttachHelmetId);
			}
		}
	}
	

	return true;
}



void CDnCutSceneTask::AddClonePlayer( DnActorHandle & hFindedActor )
{	
	CDnPlayerActor * pLocalPlayer = static_cast< CDnPlayerActor * >( CDnActor::s_hLocalActor.GetPointer() );
	DnActorHandle hClonePlayer = CreateActor( CDnActor::s_hLocalActor->GetClassID(), false, false, false );
	if( hClonePlayer )
	{
		m_hActor = hClonePlayer;
		hClonePlayer->Show( false );
		hClonePlayer->Initialize();

		CDnPlayerActor * pClonePlayerActor = static_cast< CDnPlayerActor * >( hClonePlayer.GetPointer() );
		pClonePlayerActor->SetWeaponViewOrder( 0, true );
		pClonePlayerActor->SetWeaponViewOrder( 1, true );						
		for( int i=0; i<CDnParts::SwapParts_Amount; ++i )
		{
			//pClonePlayerActor->SetPartsViewOrder( i, true );
			//pClonePlayerActor->SetHideHelmet( false );
			pClonePlayerActor->SetPartsViewOrder( i, pLocalPlayer->IsViewPartsOrder( i ) );

			int nDefaultPartsInfo[ 4 ];
			for( int i=0; i<4; ++i )
				nDefaultPartsInfo[ i ] = pLocalPlayer->GetDefaultPartsInfo( (CDnParts::PartsTypeEnum)(CDnParts::Body + i) );

			pClonePlayerActor->SetDefaultPartsInfo( nDefaultPartsInfo );
			pClonePlayerActor->SetPartsColor( MAPartsBody::HairColor, pLocalPlayer->GetPartsColor( MAPartsBody::HairColor) );
			pClonePlayerActor->SetPartsColor( MAPartsBody::EyeColor, pLocalPlayer->GetPartsColor( MAPartsBody::EyeColor) );
			pClonePlayerActor->SetPartsColor( MAPartsBody::SkinColor, pLocalPlayer->GetPartsColor( MAPartsBody::SkinColor) );
			pClonePlayerActor->SetBattleMode( false );
	
			for( int i=0; i<CDnParts::PartsTypeEnum_Amount; ++i )
			{
				DnPartsHandle hParts = pLocalPlayer->GetParts( (CDnParts::PartsTypeEnum)i );
				//DnPartsHandle hParts = pClonePlayerActor->GetParts( (CDnParts::PartsTypeEnum)i );
				if( !hParts )
					continue;
				if( (CDnParts::PartsTypeEnum)i == CDnParts::Helmet && pLocalPlayer->IsHideHelmet() )
					continue;
				AttachItem( hParts->GetClassID(), 0, 0, hParts->GetLookItemID() );
			}

			
			for( int i = 0; i < CDnWeapon::EquipSlot_Amount; i++ )
			{
				DnWeaponHandle hWeapon = pLocalPlayer->GetWeapon( i );
				//DnWeaponHandle hWeapon = pClonePlayerActor->GetWeapon( i );
				if( !hWeapon ) continue;
				AttachItem( hWeapon->GetClassID(), hWeapon->GetEnchantLevel() );
			}
			

			for( int i = 0; i < CDnParts::CashPartsTypeEnum_Amount; i++ )
			{
				DnPartsHandle hParts = pLocalPlayer->GetCashParts( (CDnParts::PartsTypeEnum)i );
				//DnPartsHandle hParts = pClonePlayerActor->GetCashParts( (CDnParts::PartsTypeEnum)i );
				if( !hParts ) continue;
				if( i < CDnParts::SwapParts_Amount && pLocalPlayer->IsViewPartsOrder(i) == false )
					continue;
				if( (CDnParts::PartsTypeEnum)i == CDnParts::CashHelmet && pLocalPlayer->IsHideHelmet() )
					continue;
				AttachItem( hParts->GetClassID(), 0, ( (i == CDnParts::CashRing2) ? 1 : 0 ), hParts->GetLookItemID() );
			}

			
			for( int i = 0; i < CDnWeapon::EquipSlot_Amount; i++ )
			{
				DnWeaponHandle hWeapon = pLocalPlayer->GetCashWeapon( i );
				//DnWeaponHandle hWeapon = pClonePlayerActor->GetCashWeapon( i );
				if( !hWeapon ) continue;
				if( pClonePlayerActor->IsViewWeaponOrder(i) == false )
					continue;
				AttachItem( hWeapon->GetClassID(), hWeapon->GetEnchantLevel(), 0, hWeapon->GetLookItemID() );
			}
			

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
			// Like login character select screen, CombineParts is not called, so forcibly call hiding right glove mesh when warrior equips gauntlet.
			pClonePlayerActor->OnCombineParts();
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
						
			/*EtVector2 vDir( 0.0f, 1.0f );
			pClonePlayerActor->Look( vDir );

			if( CDnActor::s_hLocalActor )
			{
				LOCAL_TIME LocalTime = CDnActor::s_hLocalActor->GetLocalTime();
				EtMatrix WorldMat = *pClonePlayerActor->GetMatEx();
				pClonePlayerActor->MAActorRenderBase::Process( &WorldMat, LocalTime, 0.0f );
			}*/

			//pClonePlayerActor->SetActionQueue( "Normal_Stand" );
			//pClonePlayerActor->SetGuildSelfView( pLocalPlayer->GetGuildSelfView() );

			//pActor->CombineParts(); // Do not combine in render avatar.
			//pClonePlayerActor->SetHP( 1 ); // Force HP to 1 so it isn't skipped by code that checks HP <= 0 directly.
			//hClonePlayer->Show( true ); // Because it still needs to draw to the render target,
			

			//MAPartsBody *pActorPartsBody = dynamic_cast<MAPartsBody*>(hClonePlayer.GetPointer());
			//pActorPartsBody->SetIgnoreDeleteWaitOnepiece( true );
			//AddObject( pActorPartsBody->GetObjectHandle() );
			//AddObject( pActorPartsBody->GetCombineObject() ); // Alpha (head) and weapon are excluded from combine object.
			//SetRenderFrameCount( -1 ); // Setting to -1 keeps rendering until the object is released.
		}
	

		//// In each part's link bone processing, get the actor's position to calculate matrices of linked children,
		//// Since Process is not run, the initial position must be set separately like this.
		//if( hClonePlayer->GetObjectHandle() )
		//{
		//	EtMatrix Mat;
		//	EtMatrixIdentity( &Mat );
		//	hClonePlayer->GetObjectHandle()->SetWorldMat( Mat );
		//}		

		hFindedActor = hClonePlayer;
	}
	
	m_hActor.Identity();
	m_vecPartsInfo.clear();	
}