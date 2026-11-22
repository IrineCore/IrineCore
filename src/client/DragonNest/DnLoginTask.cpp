#include "StdAfx.h"
#include "DnLoginTask.h"
#include "DnStaticCamera.h"
#include "DnInterface.h"
#include "DnLoginDlg.h"
#include "DnCharCreateDlg.h"
#include "DnActorClassDefine.h"
#include "MAPartsBody.h"
#include "DnWorld.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "DnMainFrame.h"
#include "DnLoadingTask.h"
#include "TaskManager.h"
#include "DnTableDB.h"
#include "DnWeapon.h"
#include "PerfCheck.h"
#include "DnServiceSetup.h"
#include "DnInterfaceDlgID.h"
#include "DnMessageBox.h"
#include "DnBridgeTask.h"
#include "DnAniCamera.h"
#include "EtTestCollision.h"
#include "DnInCodeResource.h"
#include <MMSystem.h>
#include "DnPlayerActor.h"
#include "DnPartsHair.h"
#include "VillageClientSession.h"
#include "DnInterfaceString.h"
#include "SyncTimer.h"
#include "GameOption.h"
#include "DnCEDof.h"
#include "DnServiceSetup.h"
#include <ctime>
#include "Version.h"
#include "EtActionCoreMng.h"
#include "DnPartsVehicleEffect.h"
#ifndef _FINAL_BUILD
#include "DnFreeCamera.h"
#endif // _FINAL_BUILD
#ifdef PRE_MOD_SELECT_CHAR
#include "DnCharSelectDlg.h"
#endif // PRE_MOD_SELECT_CHAR
#include "TimeSet.h"
#include "DnTitleTask.h"
#ifdef PRE_CHARLIST_SORTING
#include "DnCharSelectListDlg.h"
#endif // PRE_CHARLIST_SORTING



#if defined(PRE_ADD_23829)
#include "DnAuthTask.h"
#endif

#ifdef PRE_FIX_MEMOPT_EXT
#include "DnCommonUtil.h"
#endif


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#ifdef PRE_MOD_SELECT_CHAR


CDnLoginTask::CDnLoginTask()
: CInputReceiver( true )
, m_pWorld( NULL )
, m_nCharSlotCount( 0 )
, m_nCharSlotMaxCount( 0 )
, m_State( Logo )
, m_PrevState( Logo )
, m_nSelectedServerIndex( -1 )
, m_nSelectedChannelIndex( -1 )
, m_bCompleteCharList( false )
, m_bStartCharSelect( false )
, m_bShowBlackDragonRadialBlur( false )
, m_bCharSelectToServerList( false )
, m_nSelectCharIndex( -1 )
, m_bRequestWait( false )
, m_fChangePartsFadeInTime( 0.0f )
, m_nCurrentMapIndex( 0 )
, m_fOldDOFNearBlurSize( 0.0f )
, m_fOldDOFFarBlurSize( 0.0f )
, m_fOriginalFarStart( 0.0f )
, m_fOriginalFarEnd( 0.0f )
, m_fOriginalFocusDist( 0.0f )
, m_fShowOutlineTime( 0.0f )
, m_nSelectClassIndex( -1 )
, m_bStartSelectMove( false )
, m_nLoginTitleMapIndex( 0 )
, m_nLoginTitlePropIndex( 0 )
, m_nLoginCharMapIndex( 0 )
, m_nCharCreateStartSoundIndex( -1 )
, m_nTitleSoundIndex( -1 )
, m_nSelectSoundIndex( -1 )
, m_nAuthPasswordSeed( 0 )
, m_nSlotActorMapID( 0 )
#ifdef PRE_ADD_RELOGIN_PROCESS
, m_nReLoginCount( 0 )
#endif // PRE_ADD_RELOGIN_PROCESS
#ifdef PRE_CHARLIST_SORTING
, m_eCharacterListSortCommand( CHARACTERLIST_SORT_BY_NONE )
#endif // PRE_CHARLIST_SORTING
, m_bLoginInitParts(false)
#if defined( PRE_ADD_DWC )
, m_bDWCCharCreateMode( false )
, m_cDWCJobCode1( 0 )
, m_cDWCJobCode2( 0 )
, m_cDWCLevel( 60 )
, m_nDWCExp( 0 )
, m_nDWCMapID( 0 )
, m_nDWCGold( 0 )
, m_nDWCSkillPoint( 0 )
#endif // PRE_ADD_DWC
{
	memset( m_nAuthPasswordValue, 0, sizeof(m_nAuthPasswordValue) );
	memset( &m_CharList, 0, sizeof(SCCharList) );
	m_CharList.nRet = ERROR_LOGIN_FAIL;
}

CDnLoginTask::~CDnLoginTask()
{
#ifdef PRE_PLAYER_PRELOAD
	while( !CDnMainFrame::GetInstance().IsFinishPreloadPlayer() ) { Sleep(1); }
#endif
	GetInterface().Finalize( CDnInterface::Login );

	SAFE_RELEASE_SPTR( m_hCamera );
#ifndef _FINAL_BUILD
	SAFE_RELEASE_SPTR( m_hDebugCamera );
#endif // _FINAL_BUILD

	m_vecTitleMapIndex.clear();
	m_vecCharListData.clear();
#ifdef PRE_CHARLIST_SORTING
	m_vecOriginCharListData.clear();
#endif // PRE_CHARLIST_SORTING

	if( m_hSlotActorHandle )
		SAFE_RELEASE_SPTR( m_hSlotActorHandle );

	if( m_hCommonEffectHandle )
		SAFE_RELEASE_SPTR( m_hCommonEffectHandle );

	SAFE_RELEASE_SPTRVEC( m_hVecCreateActorList );
	
	for( DWORD i=0; i<m_VecCreateDefaultPartsList.size(); i++ ) 
	{
		for( int j=0; j<CDnParts::PartsTypeEnum_Amount; j++ ) 
		{
			SAFE_RELEASE_SPTRVEC( m_VecCreateDefaultPartsList[i].hVecParts[j] );
			SAFE_DELETE_VEC( m_VecCreateDefaultPartsList[i].nVecList[j] );
		}
		SAFE_RELEASE_SPTR( m_VecCreateDefaultPartsList[i].hWeapon[0] );
		SAFE_RELEASE_SPTR( m_VecCreateDefaultPartsList[i].hWeapon[1] );
	}
	SAFE_DELETE_VEC( m_VecCreateDefaultPartsList );

	ResetShowPartsList();
#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
	ResetPreviewCashCostumeList();
#endif //PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
	CDnActor::ReleaseClass();
	CDnEtcObject::ReleaseClass();
	CDnWeapon::ReleaseClass();

	if( m_pWorld ) 
		m_pWorld->Destroy();

	if( CEtActionCoreMng::IsActive() )
		CEtActionCoreMng::GetInstance().FlushWaitDelete();

	CEtResource::FlushWaitDelete();
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	if( GetEtEngine() ) GetEtEngine()->FlushBuffer();
#endif // PRE_FIX_CLIENT_MEMOPTIMIZE
	if( CEtFontMng::IsActive() )
		CEtFontMng::GetInstance().FlushFontCache();

	SAFE_RELEASE_SPTR( m_hSelectOutline );
	SAFE_RELEASE_SPTRVEC( m_hVecSelectCharOutline );

	if( m_nTitleSoundIndex != -1 ) 
	{
		CEtSoundEngine::GetInstance().RemoveChannel( m_hTitleSoundChannel );
		CEtSoundEngine::GetInstance().RemoveSound( m_nTitleSoundIndex );
		m_hTitleSoundChannel.Identity();
		m_nTitleSoundIndex = -1;
	}
}

void CDnLoginTask::PreInitialize( bool bReload )
{
	DNTableFileFormat* pTitleTable = GetDNTable( CDnTableDB::TTITLE );
	if( pTitleTable == NULL ) return;

	for( int i=0; i<pTitleTable->GetItemCount(); i++ )
	{
		int nTableID = pTitleTable->GetItemID( i );

		if( pTitleTable->GetFieldFromLablePtr( nTableID, "_UseMap" )->GetInteger() == 1 )
			m_vecTitleMapIndex.push_back( nTableID );
	}

	m_nLoginCharMapIndex = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Login_Char_MapIndex );

	// Create camera
	CDnAniCamera *pCamera = new CDnAniCamera;
	m_hCamera = pCamera->GetMySmartPtr();
	
#ifndef _FINAL_BUILD
	m_hDebugCamera = (new CDnFreeCamera)->GetMySmartPtr();
#endif // _FINAL_BUILD

	DNTableFileFormat* pCamTable = GetDNTable( CDnTableDB::TCAMERA );
	if( pCamTable == NULL ) return;

	m_strCharCreateSelectCamera = pCamTable->GetFieldFromLablePtr( 2, "_Camfilename" )->GetString();
	m_strMakingCamera = pCamTable->GetFieldFromLablePtr( 3, "_Camfilename" )->GetString();
	m_strMakingCameraFadeIn = pCamTable->GetFieldFromLablePtr( 4, "_Camfilename" )->GetString();
	m_strSelectCamera = pCamTable->GetFieldFromLablePtr( 29, "_Camfilename" )->GetString();
	m_strCharCreateStartCamera = pCamTable->GetFieldFromLablePtr( 30, "_Camfilename" )->GetString();
	m_nCharCreateStartSoundIndex = CEtSoundEngine::GetInstance().LoadSound( pCamTable->GetFieldFromLablePtr( 30, "_Soundfilename" )->GetString(), false, false );

	string strCamName = m_strCharCreateSelectCamera + ".cam";
	pCamera->AddCameraData( strCamName.c_str() );
	strCamName = m_strMakingCamera + ".cam";
	pCamera->AddCameraData( strCamName.c_str() );
	strCamName = m_strMakingCameraFadeIn + ".cam";
	pCamera->AddCameraData( strCamName.c_str() );
	strCamName = m_strSelectCamera + ".cam";
	pCamera->AddCameraData( strCamName.c_str() );
	strCamName = m_strCharCreateStartCamera + ".cam";
	pCamera->AddCameraData( strCamName.c_str() );

	CDnTitleTask *pTitleTask = (CDnTitleTask *)CTaskManager::GetInstance().GetTask( "TitleTask" );
	if( pTitleTask ) pTitleTask->CheckPointHoundLoading( 77 );
	if( pTitleTask ) pTitleTask->ForceAddHoundLoading( 3, 2000 );

#ifndef _FINAL_BUILD
	if( CGlobalValue::GetInstance().IsPermitLogin() )
	{
	}
	else 
#endif //_FINAL_BUILD
	{
		if( bReload )
			LoadWorld( m_nLoginCharMapIndex );
		else
			LoadTitle();

		if( pTitleTask ) pTitleTask->CheckPointHoundLoading( 82 );

		CDnActor::InitializeClass();
		CDnEtcObject::InitializeClass();
		InitSelectCharacter();

		if( pTitleTask ) pTitleTask->CheckPointHoundLoading( 85 );
		if( pTitleTask ) pTitleTask->ForceAddHoundLoading( 10, 5000 );

		InitCreateCharacter();

		if( pTitleTask ) pTitleTask->CheckPointHoundLoading( 99 );
	}

	// Note : Interface Initialize
	GetInterface().Initialize( CDnInterface::Login, this );
	m_nSelectSoundIndex = CEtSoundEngine::GetInstance().LoadSound( "UI_button_ok.wav", false, false );
}

bool CDnLoginTask::LoadTitle()
{
	CDnAniCamera* pCamera = (CDnAniCamera*)m_hCamera.GetPointer();
	if( pCamera == NULL ) 
		return false;

	DNTableFileFormat* pTitleTable = GetDNTable( CDnTableDB::TTITLE );
	if( pTitleTable == NULL ) 
		return false;

	int nTitleMapCount = static_cast<int>( m_vecTitleMapIndex.size() );
	if( nTitleMapCount <= 0 ) 
		return false;

	for( int i=0; i<static_cast<int>( m_vecTitleMapIndex.size() ); i++ )
	{
		int nMapIndex = pTitleTable->GetFieldFromLablePtr( m_vecTitleMapIndex[i], "_MapID" )->GetInteger();
		if( nMapIndex == m_nCurrentMapIndex )
			return false;
	}

	int nRandomIndex = _rand() % nTitleMapCount;
	m_nLoginTitleMapIndex = pTitleTable->GetFieldFromLablePtr( m_vecTitleMapIndex[nRandomIndex], "_MapID" )->GetInteger();
	m_nLoginTitlePropIndex = pTitleTable->GetFieldFromLablePtr( m_vecTitleMapIndex[nRandomIndex], "_PropID" )->GetInteger();
	m_strTitleIdleCamera = pTitleTable->GetFieldFromLablePtr( m_vecTitleMapIndex[nRandomIndex], "_Camfilename1" )->GetString();
	m_strTitleFadeOutCamera = pTitleTable->GetFieldFromLablePtr( m_vecTitleMapIndex[nRandomIndex], "_Camfilename2" )->GetString();

	string strCamName = m_strTitleIdleCamera + ".cam";
	pCamera->AddCameraData( strCamName.c_str() );
	strCamName = m_strTitleFadeOutCamera + ".cam";
	pCamera->AddCameraData( strCamName.c_str() );

	return LoadWorld( m_nLoginTitleMapIndex );
}

bool CDnLoginTask::LoadWorld( int nMapIndex )
{
	m_pWorld = &CDnWorld::GetInstance();
	if( m_pWorld == NULL || m_nCurrentMapIndex == nMapIndex )
		return false;

	m_nCurrentMapIndex = nMapIndex;
	m_pWorld->Destroy();

	GetCurRenderStack()->EmptyRenderElement();

	DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
	char szLabel[64];
	std::vector<std::string> szVecToolMapName;
	std::string szTempStr;

	for( int i=0; i<10; i++ )
	{
		sprintf_s( szLabel, "_ToolName%d", i + 1 );
		szTempStr = pMapSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetString();
		if( szTempStr.empty() ) continue;

		szVecToolMapName.push_back( szTempStr );
	}

	if( szVecToolMapName.empty() )
		return false;
	
	bool bResult = m_pWorld->Initialize( CEtResourceMng::GetInstance().GetFullPath( "MapData" ).c_str(), szVecToolMapName[_rand()%szVecToolMapName.size()].c_str(), this, true );

	if( !bResult ) return false;

	CDnWorld::WeatherEnum Weather = m_pWorld->GetNextWeather( nMapIndex );
	std::string szEnviName = CDnWorld::GetEnviName( nMapIndex, 0, Weather );
	bResult = m_pWorld->InitializeEnviroment( CEtResourceMng::GetInstance().GetFullName( szEnviName ).c_str(), m_hCamera );
	m_hCamera->DeActivate();
	
	CDnCamera::SetActiveCamera( m_hCamera );
	PlayDefaultCamera();

	return bResult;
}

void CDnLoginTask::PlayDefaultCamera()
{
	if( m_hCamera == NULL )
		return;

	if( m_nCurrentMapIndex == m_nLoginTitleMapIndex )
		((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( m_strTitleIdleCamera.c_str(), NULL );
	else if( m_nCurrentMapIndex == m_nLoginCharMapIndex )
		((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( m_strSelectCamera.c_str(), NULL );
}

void CDnLoginTask::InitSelectCharacter()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDEFAULTCREATE );
	DNTableFileFormat* pCamTable = GetDNTable( CDnTableDB::TCAMERA );

	if( pSox == NULL || pCamTable == NULL ) return;

	int nActorCount = pSox->GetItemCount();
	
	for( int i=1; i<=nActorCount; i++ )
	{
		ClassPropCamInfo stClassPropCamInfo;

		stClassPropCamInfo.nPropIndex = pSox->GetFieldFromLablePtr( i, "_CreatePropID" )->GetInteger();

		int nCamIndex = pSox->GetFieldFromLablePtr( i, "_CamIdleID" )->GetInteger();
		stClassPropCamInfo.vecCamList.push_back( pCamTable->GetFieldFromLablePtr( nCamIndex, "_Camfilename" )->GetString() );
		nCamIndex = pSox->GetFieldFromLablePtr( i, "_CamSelectID" )->GetInteger();
		stClassPropCamInfo.vecCamList.push_back( pCamTable->GetFieldFromLablePtr( nCamIndex, "_Camfilename" )->GetString() );
		int nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( pCamTable->GetFieldFromLablePtr( nCamIndex, "_Soundfilename" )->GetString(), false, false );
		stClassPropCamInfo.vecSoundList.push_back( nSoundIndex );
		nCamIndex = pSox->GetFieldFromLablePtr( i, "_CamDeselectID" )->GetInteger();
		stClassPropCamInfo.vecCamList.push_back( pCamTable->GetFieldFromLablePtr( nCamIndex, "_Camfilename" )->GetString() );
		nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( pCamTable->GetFieldFromLablePtr( nCamIndex, "_Soundfilename" )->GetString(), false, false );
		stClassPropCamInfo.vecSoundList.push_back( nSoundIndex );
		nCamIndex = pSox->GetFieldFromLablePtr( i, "_CamFadeOutID" )->GetInteger();
		stClassPropCamInfo.vecCamList.push_back( pCamTable->GetFieldFromLablePtr( nCamIndex, "_Camfilename" )->GetString() );

		m_mapPropCamListByClass.insert( make_pair( i, stClassPropCamInfo ));

		EtOutlineHandle hSelectOutline = CEtOutlineObject::Create( CEtObject::Identity() );
		static EtColor vColor( 0.1f, 0.25f, 0.5f, 1.0f );
		hSelectOutline->SetColor( vColor );
		hSelectOutline->SetWidth( 0.5f );
		m_hVecSelectCharOutline.push_back( hSelectOutline );
	}

	ShowSelectCharacter( false );

	std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.begin();
	for( ; iter != m_mapPropCamListByClass.end(); iter++ )
	{
		ClassPropCamInfo& stClassPropCamInfo = iter->second;
		std::vector<string>::iterator iter2 = stClassPropCamInfo.vecCamList.begin();
		for( ; iter2 != stClassPropCamInfo.vecCamList.end(); iter2++ )
		{
			if( m_hCamera )
			{
				string strCamName = (*iter2) + ".cam";
				((CDnAniCamera*)m_hCamera.GetPointer())->AddCameraData( strCamName.c_str() );
			}
		}
	}
}

void CDnLoginTask::ShowSelectCharacter( bool bShow )
{
	if( m_pWorld == NULL )
		return;

	std::vector<CEtWorldProp*> pVecList;

	std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.begin();
	for( ; iter != m_mapPropCamListByClass.end(); iter++ )
	{
		ClassPropCamInfo& stClassPropCamInfo = iter->second;

		m_pWorld->FindPropFromCreateUniqueID( stClassPropCamInfo.nPropIndex, &pVecList );
		if( !pVecList.empty() ) 
		{
			CDnWorldProp* pProp = (CDnWorldProp*)pVecList[0];
			pProp->Show( bShow );
			pVecList.clear();
		}
	}

	// Exception (Alfredo)
	m_pWorld->FindPropFromCreateUniqueID( 848, &pVecList );
	if( !pVecList.empty() ) 
	{
		CDnWorldProp* pProp = (CDnWorldProp*)pVecList[0];
		pProp->Show( bShow );
		pVecList.clear();
	}
}

void CDnLoginTask::ShowSelectCharOutline( bool bShow, float fSpeed )
{
	std::vector<EtOutlineHandle>::iterator iter = m_hVecSelectCharOutline.begin();
	for( ; iter != m_hVecSelectCharOutline.end(); iter++ )
	{
		(*iter)->Show( bShow, fSpeed );
	}
}

void CDnLoginTask::InitCreateCharacter()
{
	char szLabel[64];
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDEFAULTCREATE );
	DNTableFileFormat* pActorSOX = GetDNTable( CDnTableDB::TACTOR );
	int nActorCount = pSox->GetItemCount();
	DnActorHandle hActor;
	char *szLabelList[CDnParts::PartsTypeEnum_Amount] = { "_Face", "_Hair", "_Helmet", "_Body", "_Leg", "_Hand", "_Foot", NULL, NULL, NULL, NULL };
	char szTemp[128];

	for( int i=1; i<=nActorCount; i++ )
	{
		int nClassID = pSox->GetFieldFromLablePtr( i, "_ClassID" )->GetInteger();
		hActor = CreateActor( nClassID, false, true );
		if( !hActor ) continue;

		((CDnPlayerActor*)hActor.GetPointer())->SetJobHistory( nClassID );
		hActor->ShowHeadName( false );
		if( hActor->GetObjectHandle() && hActor->GetObjectHandle()->GetSkinInstance() )
			hActor->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );

		MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(hActor.GetPointer());

		char* pStaticName = pActorSOX->GetFieldFromLablePtr( nClassID, "_StaticName" )->GetString();
#ifdef PRE_FIX_MEMOPT_EXT
		char* szDummySkinName = CommonUtil::GetFileNameFromFileEXT( pActorSOX, nClassID, "_SkinName" );
#else
		char* szDummySkinName = pActorSOX->GetFieldFromLablePtr( nClassID, "_SkinName" )->GetString();
#endif
		sprintf_s( szTemp, "New_Select\\new_%s_login.ani", pStaticName, pStaticName );
		if (pPartsBody)
			pPartsBody->LoadSkin( CEtResourceMng::GetInstance().GetFullName( szDummySkinName ).c_str(), CEtResourceMng::GetInstance().GetFullName( szTemp ).c_str() );
		sprintf_s( szTemp, "New_Select\\new_%s_login.act", pStaticName, pStaticName );
		bool bLoadAction = hActor->LoadAction( CEtResourceMng::GetInstance().GetFullName( szTemp ).c_str() );
		if( !bLoadAction )
		{
			SAFE_RELEASE_SPTR( hActor );
			continue;
		}
		hActor->Initialize();

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
		((CDnPlayerActor*)hActor.GetPointer())->RecomposeAction();
#endif
		((CDnPlayerActor*)hActor.GetPointer())->SetBattleMode( false );
		hActor->GetObjectHandle()->SetCalcPositionFlag( CALC_POSITION_X|CALC_POSITION_Y|CALC_POSITION_Z );
		hActor->SetLevel( 1 );
		hActor->SetHP( 1 );

		DefaultPartsStruct Struct;
		for( int k=0; k<CDnParts::PartsTypeEnum_Amount; k++ ) 
		{
			if( szLabelList[k] == NULL ) continue;
			for( int j=0; j<10; j++ ) 
			{
				sprintf_s( szLabel, "%s%d", szLabelList[k], j + 1 );
				int nTableID = pSox->GetFieldFromLablePtr( i, szLabel )->GetInteger();
				if( nTableID <= 0 ) continue;

				Struct.nVecList[k].push_back( nTableID );
				Struct.hVecParts[k].push_back( CDnParts::CreateParts( nTableID, 0 ) );
			}
			Struct.nSelectPartsIndex[k] = ( Struct.nVecList[k].empty() ) ? -1 : 0;
		}
		Struct.dwColor[MAPartsBody::HairColor] = D3DCOLOR_XRGB(255,255,255);
		Struct.dwColor[MAPartsBody::SkinColor] = D3DCOLOR_ARGB(255,128,128,128);
		Struct.dwColor[MAPartsBody::EyeColor] = D3DCOLOR_XRGB(255,255,255);

		hActor->SetActionQueue( "NormalStand" );

		for( int j=0; j<2; j++ ) 
		{
			sprintf_s( szLabel, "_Weapon%d", j + 1 );
			int nWeaponIndex = pSox->GetFieldFromLablePtr( i, szLabel )->GetInteger();
			if( nWeaponIndex != -1 ) 
			{
				Struct.hWeapon[j] = CDnWeapon::CreateWeapon( nWeaponIndex, 0 );

				// On the creation screen, the character is close to the camera and clothing seams are noticeable, so
				// Called CDnPlayerActor::AttachWeapon to avoid combining or hiding glove submesh (to prevent TDn side from being called).
				// However, for Kali characters, LeftWeapon must be handled, so handle it as below.
				// Could use dynamic_cast, but since character class ID won't change anyway, just check ID.
				bool bCallPlayerActorFunction = true;
#ifdef PRE_ADD_KALI
				if( nClassID-1 == CDnActorState::Kali ) bCallPlayerActorFunction = false;
#endif
//[NotSure]

				if (bCallPlayerActorFunction)
				{
					((CDnPlayerActor*)hActor.GetPointer())->CDnPlayerActor::AttachWeapon( Struct.hWeapon[j], j, false );
				}
				else{
					hActor->AttachWeapon(Struct.hWeapon[j], j, false);
				}
			}
			Struct.nWeapon[j] = nWeaponIndex;
		}

		m_VecCreateDefaultPartsList.push_back( Struct );
		m_hVecCreateActorList.push_back( hActor );
	}

	ShowCreateCharacter( false );
}

bool CDnLoginTask::Initialize( LoginStateEnum State )
{
	if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );
	CDnPlayerActor::EnableAnotherPlayers( false );

	CDnMouseCursor::GetInstance().ShowCursor( true );

	m_hSelectOutline = CEtOutlineObject::Create( CEtObject::Identity() );
	static EtColor vColor(0.2f, 0.5f, 1.0f, 1.0f);
	m_hSelectOutline->SetColor( vColor );
	m_hSelectOutline->SetWidth( 0.7f );

	CGlobalInfo::GetInstance().CalcLoginConnectInfo();
	std::vector<std::string> szVecIP;
	std::vector<USHORT> nVecPort;
	for( DWORD i=0; i<CGlobalInfo::GetInstance().GetLoginConnectInfoCount(); i++ )
	{
		szVecIP.push_back( CGlobalInfo::GetInstance().GetLoginConnectInfo(i)->szIP );
		nVecPort.push_back( CGlobalInfo::GetInstance().GetLoginConnectInfo(i)->nPort );
	}

	if( CClientSessionManager::GetInstance().ConnectLoginServer( szVecIP, nVecPort, State == CharSelect ? false : true ) == false ) 
	{
		GetInterface().MessageBox( MESSAGEBOX_9, MB_OK, UICallbackEnum::ConnectLoginFailed, this, true, false, false, true );
	}

	if( m_pWorld ) m_pWorld->PlayBGM();	// Keep for now in case there is ambient sound
	DNTableFileFormat* pCamTable = GetDNTable( CDnTableDB::TCAMERA );
	if( pCamTable == NULL ) return false;
	std::string strTitleSoundFileName = pCamTable->GetFieldFromLablePtr( 1, "_Soundfilename" )->GetString();
	m_nTitleSoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( strTitleSoundFileName ).c_str(), false, false );
	m_hTitleSoundChannel = CEtSoundEngine::GetInstance().PlaySound__( "BGM", m_nTitleSoundIndex, true, true );
	if( m_hTitleSoundChannel ) 
	{
		m_hTitleSoundChannel->SetVolume( 1.0f );
		m_hTitleSoundChannel->Resume();
	}

	DebugLog("Login state  %d",State);

	switch( State )
	{
		case CharSelect:
			ChangeState( State, false );
			m_PrevState = StartGame;
			CClientSessionManager::GetInstance().DisConnectServer(true);
			break;
		default:
			m_State = State;
			m_PrevState = State;

			std::string strCmd;
			strCmd = szVersion;
			ToLowerA(strCmd);

			std::vector<std::string> tokens;
			TokenizeA(strCmd, tokens, ".");

			int nMajorVersion = atoi(tokens[0].c_str());
			int nMinorVersion = atoi(tokens[1].c_str());

			SendCheckVersion( 6, 6, false, nMajorVersion, nMinorVersion );

			DebugLog("SendCheckVersion OK");
#ifndef _FINAL_BUILD
			if( CGlobalValue::GetInstance().IsPermitLogin() ) {
				ChangeState( RequestIDPass );
				return true;
			}
#endif //_FINAL_BUILD
			GetInterface().FadeDialog( 0xFFFFFFFF, 0x00FFFFFF, 2.0f, this, true );
			break;
	}

	return true;
}

void CDnLoginTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	UpdateHang();
	if( m_bDestroyTask ) return;
	CDnAniCamera* pCamera = (CDnAniCamera*)m_hCamera.GetPointer();
	if( pCamera == NULL ) return;

	// Process Input Device
	CInputDevice::GetInstance().Process( LocalTime, fDelta );

	CDnCamera::ProcessClass( LocalTime, fDelta );
	CDnActor::ProcessClass( LocalTime, fDelta );
	CDnEtcObject::ProcessClass( LocalTime, fDelta );

	// Process Costume Aura
#ifdef PRE_ADD_CASH_COSTUME_AURA
	CDnPlayerActor*	pPlayer;
	if(!m_hVecCreateActorList.empty())
	{
		if(m_nSelectClassIndex >= 0)
		{
			pPlayer = dynamic_cast<CDnPlayerActor*>( m_hVecCreateActorList[m_nSelectClassIndex].GetPointer() );
			if( pPlayer )
				pPlayer->ShowCostumeAura();
		}
	}
#endif

	// Process Sound
	if( CEtSoundEngine::IsActive() ) 
	{
		CEtSoundEngine::GetInstance().Process( fDelta );
		DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
		if( hCamera ) 
			CEtSoundEngine::GetInstance().SetListener( hCamera->GetMatEx()->m_vPosition, hCamera->GetMatEx()->m_vZAxis, hCamera->GetMatEx()->m_vYAxis );
	}

	if( m_State == LoginStateEnum::CharSelect && m_CharList.nRet == ERROR_NONE )
	{
		_OnRecvLoginCharList( &m_CharList );
		m_CharList.nRet = ERROR_LOGIN_FAIL;
		GetInterface().FadeDialog( 0xFF000000, 0x00000000, fDelta + 2.5f, NULL );
	}
	else if( m_State == LoginStateEnum::CharCreate_SelectClass )
	{

		// Black Dragon roar Radial Blur
		if( m_pWorld && m_bShowBlackDragonRadialBlur )
		{
			std::vector<CEtWorldProp*> pVecList;
			m_pWorld->FindPropFromCreateUniqueID( 840, &pVecList );
			if( !pVecList.empty() ) 
			{
				CDnWorldActProp* pProp = dynamic_cast<CDnWorldActProp*>(pVecList[0]);
				if( pProp && strcmp( pProp->GetCurrentAction(), "Idle" ) == 0 && pProp->GetCurFrame() >= 148.0f )
				{
					DWORD dwTime = static_cast<DWORD>( ( 82 / 60.0f ) * 800.0f );
					EtVector2 vCenter( 0.5f, 0.5f );
					pCamera->RadialBlur( dwTime, vCenter, 0.2f, 0.2f, 0.4f );
					m_bShowBlackDragonRadialBlur = false;
				}
			}
		}

		if( m_nSelectClassIndex > -1 )
		{
			if( m_bStartSelectMove )
			{
				if( strstr( pCamera->GetCurrentAction(), m_strCharCreateSelectCamera.c_str() ) )
				{
					std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.find( m_nSelectClassIndex + 1 );
					if( iter != m_mapPropCamListByClass.end() )
					{
						if( m_hCamera )
						{
							ClassPropCamInfo& stClassPropCamInfo = iter->second;
							pCamera->PlayCamera( stClassPropCamInfo.vecCamList[CAM_SELECT].c_str(), stClassPropCamInfo.vecCamList[CAM_IDLE].c_str() );
							float fTotalFrame = pCamera->GetTotalFrame( stClassPropCamInfo.vecCamList[CAM_SELECT].c_str() );
							DWORD dwTime = static_cast<DWORD>( ( fTotalFrame / 60.0f ) * 800.0f );	// Use only 80% of time so the trailing part does not look awkward
							EtVector2 vCenter( 0.5f, 0.5f );
							pCamera->RadialBlur( dwTime, vCenter, 0.1f, 0.2f, 0.4f );
							CEtSoundEngine::GetInstance().PlaySound__("3D", stClassPropCamInfo.vecSoundList[0] );
						}
						m_bStartSelectMove = false;
					}
				}
			}
			else
			{
				if( !GetInterface().IsShowCharCreatePlayerInfo() )
				{
					std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.find( m_nSelectClassIndex + 1 );
					if( iter != m_mapPropCamListByClass.end() )
					{
						ClassPropCamInfo& stClassPropCamInfo = iter->second;

						if( strstr( pCamera->GetCurrentAction(), stClassPropCamInfo.vecCamList[CAM_IDLE].c_str() ) )
						{
							GetInterface().ShowCharCreatePlayerInfo( true, m_nSelectClassIndex );
//							GetInterface().ShowCharCreateBackDlg( false );
						}
					}
				}
			}
		}
		else if( m_bStartCharSelect )
		{
			if( strstr( pCamera->GetCurrentAction(), m_strCharCreateSelectCamera.c_str() ) )
			{
				GetInterface().ShowCharCreateServerNameDlg( true );
				GetInterface().ShowCharCreateSelectDlg( true );
				GetInterface().ShowCharCreateBackDlg( true );
				GetInterface().ShowCharLoginTitleDlg( true, 14 );
				m_bStartCharSelect = false;

				ShowSelectCharOutline( true, 1.0f );
				m_fShowOutlineTime = 2.0f;
			}
		}
		
		if( m_fShowOutlineTime > 0.0f )
		{
			m_fShowOutlineTime -= fDelta;
			if( m_fShowOutlineTime <= 0.0f )
			{
				m_fShowOutlineTime = 0.0f;
				ShowSelectCharOutline( false, 1.0f );
			}
		}
	}
	else if( m_State == LoginStateEnum::CharCreate_ChangeParts )
	{
		if( m_fChangePartsFadeInTime > 0.0f )
		{
			m_fChangePartsFadeInTime -= fDelta;

			if( m_fChangePartsFadeInTime <= 0.0f )
			{
				m_fChangePartsFadeInTime = 0.0f;
				pCamera->PlayCamera( m_strMakingCameraFadeIn.c_str(), m_strMakingCamera.c_str() );

				RestoreOriginalDOFInfo();
				GetInterface().FadeDialog( 0xFF000000, 0x00000000, 1.0f, this );
				GetInterface().ShowCharCreateDlg( true );
				GetInterface().EnableCharCreateBackDlgControl( true );

				if( m_nSelectClassIndex > -1 )
				{
					ShowCreateCharacter( true, m_nSelectClassIndex );
#ifdef PRE_MOD_CREATE_CHAR
					SetCharCreateBasePartsName();
#endif // PRE_MOD_CREATE_CHAR
				}
			}
		}
	}

	// Keep setting position because ghosts have an issue where their position gradually drifts.
	// Originally only done in ProcessDeleteWait, but it kept creeping up even during long idle waits on the character creation screen..
	if( m_hSlotActorHandle ) 
		m_hSlotActorHandle->SetPosition( m_CreateOffset.m_vPosition );

#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	// Process Interface
	GetInterface().Process( LocalTime, fDelta );
#endif

#ifndef _FINAL_BUILD
	#ifdef PRE_FIX_SHOWVER
	std::string szVersion = CGlobalInfo::GetInstance().MakeVersionString();
	#else
	std::string szVersion = CGlobalValue::GetInstance().MakeVersionString();
	#endif
	EtVector2 vPos = EtVector2( ( 1.f / CEtDevice::GetInstance().Width() ) * 10.f, ( 1.f / CEtDevice::GetInstance().Height() ) * ( CEtDevice::GetInstance().Height() - 20.f ) );
	EternityEngine::DrawText2D( vPos, szVersion.c_str(), 0xff000000 );
	vPos = EtVector2( ( 1.f / CEtDevice::GetInstance().Width() ) * 9.f, ( 1.f / CEtDevice::GetInstance().Height() ) * ( CEtDevice::GetInstance().Height() - 21.f ) );
	EternityEngine::DrawText2D( vPos, szVersion.c_str(), 0xffffffff );
#endif //_FINAL_BUILD

	// Note : UI Process
	EtInterface::Process( fDelta );

	// Process World
	if( m_pWorld ) 
	{
		m_pWorld->Process( LocalTime, fDelta );
	}

	EternityEngine::ClearScreen( 0xFFFFFFFF, 1.0f, 0, false );
	EternityEngine::RenderFrame( fDelta );
	EtInterface::Render( fDelta );
	EternityEngine::ShowFrame( NULL );
}

void CDnLoginTask::ChangeState( LoginStateEnum State, bool bAutoSendMessage )
{
	WriteLog( 1, ", Info, CDnLoginTask::ChangeState : Prev(%d), Cur(%d)\n", m_State, State );
	m_PrevState = m_State;
	m_State = State;

	if( bAutoSendMessage )
	{
#ifndef _FINAL_BUILD
		if( CGlobalValue::GetInstance().IsPermitLogin() )
		{
			switch( m_State )
			{
			case LoginStateEnum::RequestIDPass:
				{
#ifdef PRE_ADD_RELOGIN_PROCESS
					m_nReLoginCount = 0;
#endif // PRE_ADD_RELOGIN_PROCESS
					RequestLogin( CGlobalValue::GetInstance().m_szID.c_str(), CGlobalValue::GetInstance().m_szPass.c_str() );
				}
				return;
			case LoginStateEnum::ServerList:
				SendServerList();
				return;
			case LoginStateEnum::CharSelect:
				{
					CDnBridgeTask::GetInstance().SetCurrentCharLevel( 1 );
					bool isIgnoreAuthNotify = false;
#if defined(PRE_ADD_23829)
					// User option for showing 2nd password window [2010/12/07 semozz]
					CDnAuthTask *pAuthTask = (CDnAuthTask *)CTaskManager::GetInstance().GetTask("AuthTask");
					isIgnoreAuthNotify = pAuthTask ? pAuthTask->GetIgnore2ndPassNotify() : false;
#endif // PRE_ADD_23829
					int nSeed = 1;
					int nValue[4] = { 0, };
					if( !CGlobalValue::GetInstance().m_szSecondPass.empty() ) 
					{
						CMtRandom Random;

						std::vector<std::string> tokens;
						TokenizeA( std::string(SecondPassword::GetVersion()), tokens, "." );

						int nKey = atoi(tokens[0].c_str()) + atoi(tokens[1].c_str());
						if( nKey % 2 ) nKey = -nKey;

						int nTempSeed = nSeed + nKey;
						Random.srand( nTempSeed );
						std::vector<int> nVecList, nVecList2;
						int nArray[10] = { 7, 8, 9, 4, 5, 6, 1, 2, 3, 0 };
						
						for( int i=0; i<10; i++ ) 
							nVecList2.push_back(nArray[i]);

						while( !nVecList2.empty() ) 
						{
							int nOffset = Random.rand( (int)nVecList2.size() );
							nVecList.push_back( nVecList2[nOffset] );
							nVecList2.erase( nVecList2.begin() + nOffset );
						}
						int nNum[4];
						int nTemp = _wtoi( CGlobalValue::GetInstance().m_szSecondPass.c_str() );
						nNum[0] = nTemp / 1000;
						nNum[1] = (nTemp % 1000) / 100;
						nNum[2] = (nTemp % 100) / 10;
						nNum[3] = nTemp % 10;
						
						for( int i=0; i<4; i++ ) 
						{
							for( int j=0; j<10; j++ ) 
							{
								if( nVecList[j] == nNum[i] ) 
								{
									nValue[i] = j;
									break;
								}
							}
						}
					}

					SendSelectChar( GetSelectCharacterDBID( CGlobalValue::GetInstance().m_nLocalPlayerClassID ), nSeed, nValue, isIgnoreAuthNotify );
					return;
				}
				break;
			case LoginStateEnum::ChannelList:
				SendSelectedChannel( m_nSelectedChannelIndex, false );
				m_State = LoginStateEnum::StartGame;
				break;
			}
		}
		else
#endif // _FINAL_BUILD
		{
			switch( m_State ) 
			{
			case LoginStateEnum::ServerList:
				{
					if( m_PrevState == LoginStateEnum::CharSelect )
					{
						GetInterface().FadeDialog( 0x00000000, 0xFF000000, 1.0f, this, true );						
						m_bCharSelectToServerList = true;
						m_nSelectCharIndex = -1;
#ifdef PRE_CHARLIST_SORTING
						SendCharacterSlotCode( m_eCharacterListSortCommand );
#endif // PRE_CHARLIST_SORTING
					}
					else
					{
						SendServerList();
						LoadTitle();
					}
				}
				break;
			case LoginStateEnum::ChannelList:
				{
					bool isIgnoreAuthNotify = false;
#if defined(PRE_ADD_23829)
					// User option for showing 2nd password window [2010/12/07 semozz]
					CDnAuthTask *pAuthTask = (CDnAuthTask *)CTaskManager::GetInstance().GetTask("AuthTask");
					isIgnoreAuthNotify = pAuthTask ? pAuthTask->GetIgnore2ndPassNotify() : false;
#endif // PRE_ADD_23829
					SendSelectChar( GetSelectCharacterDBID( m_nSelectCharIndex ), m_nAuthPasswordSeed, m_nAuthPasswordValue, isIgnoreAuthNotify );
				}
				break;
			case LoginStateEnum::StartGame:
					g_pServiceSetup->WriteStageLog_( IServiceSetup::JoinChannel , _T("StartGmae"));	
					
					if( m_hSlotActorHandle && m_hSlotActorHandle->GetWeapon( 0, false ) )
					{
						((CDnPlayerActor*)m_hSlotActorHandle.GetPointer())->CmdToggleBattle( true );
					}

					SendSelectedChannel( m_nSelectedChannelIndex, false );
					GetInterface().FadeDialog( 0x00000000, 0x00000000, 0.0f, this, true );
#ifdef PRE_CHARLIST_SORTING
					SendCharacterSlotCode( m_eCharacterListSortCommand );
#endif // PRE_CHARLIST_SORTING
				break;
			case LoginStateEnum::RequestIDPass:
				{
#ifdef PRE_ADD_RELOGIN_PROCESS
					m_nReLoginCount = 0;
#endif // PRE_ADD_RELOGIN_PROCESS
					ShowSelectCharacter( false );
				}
				break;
			case LoginStateEnum::CharCreate_SelectClass:
				{
					m_nSelectClassIndex = -1;
					SetDOFInfo( 250.0f, 500.0f, 250.0f, 0.0f, 3.5f );
					GetInterface().FadeDialog( 0x00000000, 0xFF000000, 1.0f, this );

				}
				break;
			case LoginStateEnum::CharCreate_ChangeParts:
				{
					std::vector<DnActorHandle>::iterator iter = m_hVecCreateActorList.begin();
					for( ; iter != m_hVecCreateActorList.end(); iter++ )
					{
						(*iter)->SetPosition( m_CreateOffset.m_vPosition );
						(*iter)->Look( -EtVec3toVec2( m_CreateOffset.m_vZAxis ) );
					}

					InitShowPartsList();
					DetachAllParts();
					ShowDefaultPartsList( true );
				}
				break;
			}
		}
	}

	if( m_State != CharCreate_SelectClass && m_State != CharCreate_ChangeParts )
		ShowSelectCharacter( false );
#if defined( PRE_ADD_DWC )
	if( m_State != CharCreate_ChangeParts )
		ResetDWCCharCreateMode();
#endif // PRE_ADD_DWC
	
	if( m_State == CharSelect )
	{
		m_nSelectClassIndex = -1;
		m_bStartSelectMove = false;
		m_bStartCharSelect = false;
		m_bCompleteCharList = false;
		LoadWorld( m_nLoginCharMapIndex );
		ShowCreateCharacter( false );
		ShowSelectCharacter( false );

		if( m_pWorld )
		{
			std::vector<CEtWorldProp*> pVecList;
			m_pWorld->FindPropFromCreateUniqueID( 841, &pVecList );		// Character creation position
			if( !pVecList.empty() ) 
			{
				CDnWorldProp* pProp = (CDnWorldProp*)pVecList[0];
				m_CreateOffset = *pProp->GetMatEx();
				pVecList.clear();
			}

			if( m_mapPropCamListByClass.size() == m_hVecSelectCharOutline.size() )
			{
				int i = 0;
				std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.begin();
				for( ; iter != m_mapPropCamListByClass.end(); iter++, i++ )
				{
					ClassPropCamInfo& stClassPropCamInfo = iter->second;
					m_pWorld->FindPropFromCreateUniqueID( stClassPropCamInfo.nPropIndex, &pVecList );
					if( !pVecList.empty() ) 
					{
						CDnWorldProp* pProp = (CDnWorldProp*)pVecList[0];
						if( pProp )
							m_hVecSelectCharOutline[i]->SetObject( pProp->GetObjectHandle() );
						pVecList.clear();
					}
				}
			}
			ShowSelectCharOutline( false, 0.0f );
		}

		if( m_PrevState == LoginStateEnum::ServerList )
			GetOriginalDOFInfo();
		else
			RestoreOriginalDOFInfo();

		CDnAniCamera* pCamera = (CDnAniCamera *)m_hCamera.GetPointer();
		if( pCamera == NULL )return;
		pCamera->PlayCamera( m_strSelectCamera.c_str(), NULL );

		if( m_nCharSlotCount > 0 )
		{
			if( m_PrevState == LoginStateEnum::CharCreate_SelectClass || m_PrevState == LoginStateEnum::CharCreate_ChangeParts || m_PrevState == LoginStateEnum::StartGame 
				)
			{
				CDnCharSelectDlg* pCharSelectDlg = GetInterface().GetCharSelectDlg();
				if( pCharSelectDlg )
				{
					if( m_nSelectCharIndex > -1 && m_nSelectCharIndex < m_nCharSlotCount )
					{
						int nSelectCharIndex = m_nSelectCharIndex;
						m_nSelectCharIndex = -1;
						pCharSelectDlg->SelectCharIndex( nSelectCharIndex );
					}
					else
						pCharSelectDlg->SelectCharIndex( 0 );
				}
			}
		}
	}

	GetInterface().ChangeState( m_State );
}

void CDnLoginTask::ShowCreateCharacter( bool bShow, int nClassID )
{
	for( DWORD i=0; i<m_hVecCreateActorList.size(); i++ ) 
	{
		m_hVecCreateActorList[i]->Show( false );
		m_hVecCreateActorList[i]->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( -1, -1, true );
	}

	if( nClassID >= static_cast<int>( m_hVecCreateActorList.size() ) )
		return;

	if( bShow && nClassID > -1 )
	{
		m_hVecCreateActorList[nClassID]->Show( true );
		m_hVecCreateActorList[nClassID]->SetAction( "CreateSelect_Cancle_Idle", 0.f, 0.f );

		int nSelectHairColorIndex = _rand() % HAIR_COLOR_COUNT;
		SetCharColor( MAPartsBody::HairColor, nSelectHairColorIndex, nClassID );

		int nSelectSkinColorIndex = 0;//_rand() % SKIN_COLOR_COUNT; // Let's use the default skin color...
		SetCharColor( MAPartsBody::SkinColor, nSelectSkinColorIndex, nClassID );

		int nSelectEyeColorIndex = _rand() % EYE_COLOR_COUNT;
		SetCharColor( MAPartsBody::EyeColor, nSelectEyeColorIndex, nClassID );

		m_hVecCreateActorList[nClassID]->SetFaceAction( "Flicker" );

		DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
		if( hHandle ) 
		{
			hHandle->SetActionQueue( "CharSelect_New" );
			hHandle->SetPosition( m_CreateOffset.m_vPosition );
			m_hVecCreateActorList[nClassID]->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );
		}
	}
}


void CDnLoginTask::OnConnectTcp()
{
	CLoginClientSession::OnConnectTcp();
}

void CDnLoginTask::OnDisconnectTcp( bool bValidDisconnect )
{
	if( !bValidDisconnect ) 
	{
		WriteLog( 0, ", Error, Disconnect Login Server\n" );
#if defined(_FILELOG_PACKET) || defined(_FILELOG_SPOS)
		g_bInvalidDisconnect = true;
#endif
		GetInterface().MessageBox( MESSAGEBOX_10, MB_OK, UICallbackEnum::DisconnectTCP, this, true, false, false, true );


	}
}

void CDnLoginTask::OnRecvLoginCheckVersion( SCCheckVersion *pPacket )
{
	FUNC_LOG();
	
	if (pPacket->nRet == ERROR_NONE) 
	{
		g_pServiceSetup->Initialize(NULL);
	}
	else
	{
		CClientSessionManager::GetInstance().DisConnectServer(false);
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::CheckVersionFailed, this, true, false, false, true );
	}
}

void CDnLoginTask::OnRecvLoginCheckLogin( SCCheckLogin *pPacket )
{
	FUNC_LOG();
	CSyncTimer::GetInstance().SetServerTime( pPacket->_tServerTime );
	if (pPacket->nRet == ERROR_NONE) 
	{
		ChangeState( ServerList );
	}
	else if ((pPacket->nRet == ERROR_LOGIN_FCM_PROTECTED) || (pPacket->nRet == ERROR_LOGIN_FCM_LIMITED))
	{
		ChangeState( ServerList );
		CDnMessageBoxTextBox* pDnMessageBoxTextBox = GetInterface().GetMessageBoxTextBox();
		if( pDnMessageBoxTextBox )
		{
			WCHAR sep;
			sep = 0xff00;
			pDnMessageBoxTextBox->AddMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106101 ) );
			pDnMessageBoxTextBox->AddMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106102 ) );
			pDnMessageBoxTextBox->AddMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106103 ) );
			std::wstring message;

			EtInterface::GetNameLinkMgr().AppendTag(message, CEtUINameLinkMgr::eNLTAG_START);
			message += FormatW( _T("%cl%cc%08x%s"), sep, sep, D3DCOLOR_ARGB(0xFF,0xFF,0xFF,0x00), GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106104 ) );
			EtInterface::GetNameLinkMgr().AppendTag(message, CEtUINameLinkMgr::eNLTAG_END);

			pDnMessageBoxTextBox->AddMessage( message.c_str() );
			pDnMessageBoxTextBox->AddMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106105 ) );
		}

		GetInterface().MessageBoxTextBox( L"" );
	}
	else
	{
#ifdef PRE_ADD_RELOGIN_PROCESS
		// Retry 3 times on duplicate login.
		if( pPacket->nRet == ERROR_GENERIC_DUPLICATEUSER && m_nReLoginCount < 3 )
		{
			Sleep( 1000 );
			RequestLogin( m_strConnectID.c_str(), m_strConnectPassword.c_str() );
			m_nReLoginCount++;
			return;
		}
#endif // PRE_ADD_RELOGIN_PROCESS
		// Not actually a MESSAGEBOX_1 message, but set to MESSAGEBOX_2 to share callback handling (focus goes to PW input box when message box closes).
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, MESSAGEBOX_2, GetInterface().GetLoginDlg());
		GetInterface().GetLoginDlg()->EnableLoginButton( true );
	}
}

void CDnLoginTask::OnRecvLoginCharList( SCCharList *pPacket )
{
	SetRequestWait( false );
	FUNC_LOG();
	if( pPacket->nRet != ERROR_NONE )
	{
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::CharListFailed, this);
		return;
	}
	// Block Back on channel list to prevent UI from showing early and getting messed up by clicking Back again or Create Character.
	if( m_PrevState == ChannelList ) GetInterface().ChangeState( CharSelect );

	CGlobalInfo::GetInstance().m_cLocalAccountLevel = pPacket->cAccountLevel;

	// m_nSelectedServerIndex is -1 only when returning to character select from village or in-game.
	if( m_nSelectedServerIndex != -1 ) 
	{
		CDnBridgeTask *pTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
		std::wstring wszName = CGlobalInfo::GetInstance().GetServerName( m_nSelectedServerIndex );
		if( pTask && !wszName.empty() )
			pTask->SetCurrentServerName( wszName.c_str() );
	}

	if( m_State == ServerList ) 
	{
		memset( &m_CharList, 0, sizeof(SCCharList) );
		m_CharList.nRet = pPacket->nRet;
#if defined(PRE_CHARLIST_SORTING)
		m_CharList.cCharacterSortCode = pPacket->cCharacterSortCode;
#endif	// #if defined(PRE_CHARLIST_SORTING)
		m_CharList.cCharCount = pPacket->cCharCount;
		m_CharList.cCharCountMax = pPacket->cCharCountMax;
		for( int i = 0; i < pPacket->cCharCount; i++ ) 
		{
			m_CharList.CharListData[i] = pPacket->CharListData[i];
		}

#ifndef _FINAL_BUILD
		if( CGlobalValue::GetInstance().IsPermitLogin() )
		{
			_OnRecvLoginCharList( &m_CharList );
			ChangeState( CharSelect );
			return;
		}
#endif // _FINAL_BUILD
		GetInterface().ShowServerSelectDlg( false );
		float fTotalFrame = ((CDnAniCamera*)m_hCamera.GetPointer())->GetTotalFrame( m_strTitleFadeOutCamera.c_str() );
		if( fTotalFrame > 0.0f )
			GetInterface().FadeDialog( 0x00000000, 0xFF000000, (fTotalFrame / 60.0f) * 0.9f, this, true );	// Fade out over about 90% of total frame time
		else
			GetInterface().FadeDialog( 0x00000000, 0xFF000000, 1.5f, this, true );

		((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( m_strTitleFadeOutCamera.c_str(), NULL );

		std::vector<CEtWorldProp*> pVecList;
		m_pWorld->FindPropFromCreateUniqueID( m_nLoginTitlePropIndex, &pVecList );
		if( !pVecList.empty() ) 
		{
			CDnWorldActProp* pProp = dynamic_cast<CDnWorldActProp*>(pVecList[0]);
			if( pProp )
				pProp->SetActionQueue( "FadeOut" );
		}
	}
	else 
	{
		_OnRecvLoginCharList( pPacket );
#ifndef _FINAL_BUILD
		if( CGlobalValue::GetInstance().IsPermitLogin() )
		{
			ChangeState( CharSelect );
			return;
		}
#endif
		if( m_PrevState == StartGame )
			GetInterface().FadeDialog( 0xFF000000, 0x00000000, 1.0f, this );

		CEtSoundEngine::GetInstance().FadeVolume( NULL, 1.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );
	}
}

INT64 CDnLoginTask::GetSelectCharacterDBID( int nSelectIndex )
{
	if( nSelectIndex < 0 || nSelectIndex >= static_cast<int>( m_vecCharListData.size() ) )
		return 0;

	return m_vecCharListData[nSelectIndex].biCharacterDBID;
}

bool CDnLoginTask::SortByServerIndex( const TCharListData& a, const TCharListData& b )
{
	return ( a.cWorldID < b.cWorldID ) ? true : false;
}

void CDnLoginTask::_OnRecvLoginCharList( SCCharList *pPacket )
{
	if( m_hSlotActorHandle )
	{
		m_hSlotActorHandle->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( -1, -1, true );
		SAFE_RELEASE_SPTR( m_hSlotActorHandle );
	}

	m_nSlotActorMapID = 0;
	m_vecCharListData.clear();

	m_nCharSlotMaxCount = pPacket->cCharCountMax;	// Max slot count
	m_nCharSlotCount = pPacket->cCharCount;			// Current character count

#ifdef PRE_CHARLIST_SORTING
	m_vecOriginCharListData.clear();
	for( int i=0; i<m_nCharSlotCount; i++ )			// Save character list
	{
		m_vecOriginCharListData.push_back( pPacket->CharListData[i] );
		m_vecCharListData.push_back( pPacket->CharListData[i] );
	}

	if( pPacket->cCharacterSortCode == 0 )
		SortCharacterList( CHARACTERLIST_SORT_BY_SERVER );
	else
		SortCharacterList( static_cast<eCharacterListSortCommand>( pPacket->cCharacterSortCode ) );
#else // PRE_CHARLIST_SORTING
	std::vector<TCharListData> vecSelectedServerCharListData;
	for( int i=0; i<m_nCharSlotCount; i++ )			// Save character list
	{
		if( pPacket->CharListData[i].cWorldID == CGlobalInfo::GetInstance().m_nSelectedServerIndex )
			vecSelectedServerCharListData.push_back( pPacket->CharListData[i] );
		else
			m_vecCharListData.push_back( pPacket->CharListData[i] );
	}
	std::sort( m_vecCharListData.begin(), m_vecCharListData.end(), SortByServerIndex );
	std::vector<TCharListData>::iterator iter = vecSelectedServerCharListData.begin();
	for( ; iter != vecSelectedServerCharListData.end(); iter++ )
	{
		m_vecCharListData.insert( m_vecCharListData.begin(), *iter );
	}
	vecSelectedServerCharListData.clear();
#endif // PRE_CHARLIST_SORTING

#ifndef _FINAL_BUILD
	if( CGlobalValue::GetInstance().IsPermitLogin() )
		return;
#endif

	if( m_wstrCreateCharName.length() > 0 )	// Automatically select upon entering after character creation
	{
		std::vector<TCharListData>::iterator iter = m_vecCharListData.begin();
		int nIndex = 0;
		for( ; iter != m_vecCharListData.end(); iter++, nIndex++ )
		{
			if( wcscmp( (*iter).wszCharacterName, m_wstrCreateCharName.c_str() ) == 0 )
				m_nSelectCharIndex = nIndex;
		}
		m_wstrCreateCharName = L"";
	}

	CDnCharSelectDlg* pCharSelectDlg = GetInterface().GetCharSelectDlg();
	if( pCharSelectDlg )
	{
#ifdef PRE_CHARLIST_SORTING
		CDnCharSelectListDlg* pCharSelectListDlg = pCharSelectDlg->GetCharSelectListDlg();
		if( pCharSelectListDlg )
			pCharSelectListDlg->SetCharacterListSort( m_eCharacterListSortCommand );
#endif // PRE_CHARLIST_SORTING

		pCharSelectDlg->SetCharacterList( m_vecCharListData, m_nCharSlotMaxCount );

		if( m_nCharSlotCount > 0 )
		{
			if( m_nSelectCharIndex > -1 && m_nSelectCharIndex < m_nCharSlotCount )
			{
				int nSelectCharIndex = m_nSelectCharIndex;
				m_nSelectCharIndex = -1;
				pCharSelectDlg->SelectCharIndex( nSelectCharIndex );
			}
			else
				pCharSelectDlg->SelectCharIndex( 0 );
		}
	}

	m_bCompleteCharList = true;
#ifndef _FINAL_BUILD
	CGlobalValue::GetInstance().m_bMessageBoxWithMsgID = false;
#endif
}

void CDnLoginTask::SetCharSelect( int nSelectIndex )
{
	if( m_nSelectCharIndex == nSelectIndex ) 
		return;
		
	if( m_hSlotActorHandle )
	{
		SAFE_RELEASE_SPTR( m_hSlotActorHandle );
	}

	if( m_hCommonEffectHandle == NULL )
	{
		m_hCommonEffectHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
		if( m_hCommonEffectHandle ) 
		{
			m_hCommonEffectHandle->SetPosition( m_CreateOffset.m_vPosition );
		}
	}

	m_nSlotActorMapID = 0;
	m_nSelectCharIndex = -1;

	if( nSelectIndex < 0 || nSelectIndex >= static_cast<int>( m_vecCharListData.size() ) )
	{
		if( m_hCommonEffectHandle )
		{
			m_hCommonEffectHandle->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveAllSignalHandle();
			m_hCommonEffectHandle->SetActionQueue( "Idle" );
		}

		return;
	}

	if( m_hCommonEffectHandle && m_hCommonEffectHandle->GetCurrentActionIndex() != m_hCommonEffectHandle->GetElementIndex( "CharSelect_New" ) )
	{
		m_hCommonEffectHandle->SetActionQueue( "CharSelect_New" );
	}

	m_nSelectCharIndex = nSelectIndex;
	CEtSoundEngine::GetInstance().PlaySound__( "2D", m_nSelectSoundIndex );
	
	DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );
	if( !pActorSox ) return;

	std::vector<TCharListData>::iterator iter = m_vecCharListData.begin();
	iter += nSelectIndex;

	int nBaseClassID = CDnPlayerState::GetJobToBaseClassID( (*iter).cJob );
	if( nBaseClassID == 0 )
		return;

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
	m_nSelectClassIndex = nBaseClassID - 1;
#endif

	DnActorHandle hActor;
	hActor = CreateActor( nBaseClassID, false, true );

	if( !hActor )
		return;

	hActor->ShowHeadName( false );
	if( hActor->GetObjectHandle() && hActor->GetObjectHandle()->GetSkinInstance() )
		hActor->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );

	CDnPlayerActor*	pPlayer;
	pPlayer = dynamic_cast<CDnPlayerActor*>(hActor.GetPointer());
	char* pStaticName = pActorSox->GetFieldFromLablePtr( nBaseClassID, "_StaticName" )->GetString();
#ifdef PRE_FIX_MEMOPT_EXT
	char* pDummySkinName = CommonUtil::GetFileNameFromFileEXT( pActorSox, nBaseClassID, "_SkinName" );
#else
	char* pDummySkinName = pActorSox->GetFieldFromLablePtr( nBaseClassID, "_SkinName" )->GetString();
#endif
	char szTemp[128];
	sprintf_s( szTemp, "%s\\%s_login.ani", pStaticName, pStaticName );
	pPlayer->LoadSkin( CEtResourceMng::GetInstance().GetFullName( pDummySkinName ).c_str(), CEtResourceMng::GetInstance().GetFullName( szTemp ).c_str() );
	sprintf_s( szTemp, "%s\\%s_login.act", pStaticName, pStaticName );
	if( !hActor->LoadAction( CEtResourceMng::GetInstance().GetFullName( szTemp ).c_str() ) )
		return;
	pPlayer->SetJobHistory( (*iter).cJob );

	int nDefaultPartsInfo[4] = { (*iter).nDefaultBody, (*iter).nDefaultLeg, (*iter).nDefaultHand, (*iter).nDefaultFoot };
	pPlayer->SetDefaultPartsInfo( nDefaultPartsInfo );

	int *pEquipArray = (*iter).nEquipArray;
	for( int j=EQUIP_FACE; j<=EQUIP_RING2; j++ )
	{
		if( pEquipArray[j] < 1 )
		{
			pPlayer->DetachParts( (CDnParts::PartsTypeEnum)j );
			continue;
		}
		DnPartsHandle hParts = CDnParts::CreateParts( pEquipArray[j], 0 );
		pPlayer->AttachParts( hParts, (CDnParts::PartsTypeEnum)j, true, false );

		if( pPlayer->GetObjectHandle() && pPlayer->GetObjectHandle()->GetSkinInstance() )
			pPlayer->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
	}		

	pPlayer->SetPartsColor( MAPartsBody::HairColor, (*iter).dwHairColor );
	pPlayer->SetPartsColor( MAPartsBody::EyeColor, (*iter).dwEyeColor );
	pPlayer->SetPartsColor( MAPartsBody::SkinColor, (*iter).dwSkinColor );

	pPlayer->SetBattleMode( false );

	for( int j=0; j<2; j++ )
	{
		if( pEquipArray[EQUIP_WEAPON1 + j] == 0 ) continue;
		DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( pEquipArray[EQUIP_WEAPON1 + j], 0 );
		hActor->AttachWeapon( hWeapon, j, true );
	}

	if( !hActor->Initialize() )
		return;

	int *pCashEquipArray = (*iter).nCashEquipArray;
	for( int j=CASHEQUIP_HELMET; j<=CASHEQUIP_FAIRY; j++ )
	{
		if( pCashEquipArray[j] < 1 ) continue;

		// Since login does not call CombineParts, remove normal items here.
		if( j >= CASHEQUIP_HELMET && j <= CASHEQUIP_FOOT )
		{
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
			if( pPlayer->GetParts( (CDnParts::PartsTypeEnum)( j + 2 ) ) && !CDnParts::IsTransparentSkin( pCashEquipArray[j], nBaseClassID ) )
#else
			if( pPlayer->GetParts( (CDnParts::PartsTypeEnum)( j + 2 ) ) )
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME
				pPlayer->DetachParts( (CDnParts::PartsTypeEnum)( j + 2 ) );
		}
		DnPartsHandle hParts = CDnParts::CreateParts( pCashEquipArray[j], 0 );
		pPlayer->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)j, true, false );
		if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
			hParts->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
	}

	for( int j=0; j<2; j++ )
	{
		if( pCashEquipArray[CASHEQUIP_WEAPON1 + j] == 0 ) continue;
		DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( pCashEquipArray[CASHEQUIP_WEAPON1 + j], 0 );
		pPlayer->AttachCashWeapon( hWeapon, j, true, false );
	}
#ifdef PRE_MOD_WARRIOR_RIGHTHAND
	// Since login does not call CombineParts, forcibly call hiding right glove mesh when warrior equips gauntlet.
	pPlayer->OnCombineParts();
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND

	m_hSlotActorHandle = hActor;
	m_nSlotActorMapID = (*iter).nMapID;

	hActor->SetPosition( m_CreateOffset.m_vPosition );
	hActor->Look( -EtVec3toVec2( m_CreateOffset.m_vZAxis ) );
	hActor->SetActionQueue( "NormalStand" );
	hActor->SetFaceAction( "Flicker" );
	hActor->SetName( (*iter).wszCharacterName );
	hActor->SetLevel( ( (*iter).cLevel < 1 ) ? 1 : (*iter).cLevel );
	hActor->SetHP( hActor->GetMaxHP() );
	hActor->GetObjectHandle()->SetCalcPositionFlag( CALC_POSITION_X|CALC_POSITION_Y|CALC_POSITION_Z );

	if( (*iter).bDeleteFlag )
	{
		hActor->SetScale( EtVector3( 0.8f, 0.8f, 0.8f ) );
		hActor->SetHP( 0 );
		pPlayer->ToggleGhostMode( true, false );
		if( pPlayer->GetElement( "Move_Front" ) )
			pPlayer->SetActionQueue( "Stand2", 0, 0.f, (float)( rand()%pPlayer->GetElement( "Move_Front" )->dwLength ), false, false );
	}

#ifdef PRE_ADD_CASH_COSTUME_AURA
	if( !pPlayer->ComputeMyEquipCostumeAura() )
		pPlayer->LoadCostumeAuraEffect(false);
#endif

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	// #54540 Call RecompseAction() one more time including unowned characters.
	// RecomposeAction() in PreInitialize() is called without going through LoginTask when returning from the tutorial
	// Because it was called directly without matching _Login.act, related sound or effect signals are not loaded.
	// Therefore, change from calling only for owned characters from the character list to calling for all character types.
	for( int i = 0; i < (int)m_hVecCreateActorList.size(); ++i )
	{
		DnActorHandle hActor = m_hVecCreateActorList.at(i);
		if( hActor && hActor->IsPlayerActor() )
		{
			CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
			pPlayerActor->RecomposeAction();
		}
	}
#endif

	GetInterface().EnableCharSelectDlgControl( true, "ID_CONNECT" );
	GetInterface().EnableCharSelectDlgControl( true, "ID_DELETE_CHAR" );
	CDnBridgeTask::GetInstance().SetCurrentCharLevel( m_hSlotActorHandle->GetLevel() );
}

bool CDnLoginTask::CheckSelectCharDeleteWait()
{
	if( m_nSelectCharIndex < 0 || m_nSelectCharIndex >= static_cast<int>( m_vecCharListData.size() ) )
		return false;

	DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );
	if( !pActorSox ) return false;

	std::vector<TCharListData>::iterator iter = m_vecCharListData.begin();
	iter += m_nSelectCharIndex;

	return (*iter).bDeleteFlag;
}

void CDnLoginTask::CharacterAddRotateYaw( float fAngle )
{
	if( m_State == LoginStateEnum::CharSelect )
	{
		if( m_hSlotActorHandle )
		{
			m_hSlotActorHandle->GetMatEx()->RotateYaw( fAngle );

			EtVector3 vPos = CDnCamera::GetActiveCamera()->GetMatEx()->m_vPosition;
			vPos += ( -1000.f * CDnCamera::GetActiveCamera()->GetMatEx()->m_vZAxis );
			m_hSlotActorHandle->SetHeadLookTarget( vPos );
		}
	}
	else if( m_State == LoginStateEnum::CharCreate_ChangeParts )
	{
		if( m_nSelectClassIndex > -1 && m_nSelectClassIndex < static_cast<int>( m_hVecCreateActorList.size() ) )
		{
			m_hVecCreateActorList[m_nSelectClassIndex]->GetMatEx()->RotateYaw( fAngle );

			EtVector3 vPos = CDnCamera::GetActiveCamera()->GetMatEx()->m_vPosition;
			vPos += ( -1000.f * CDnCamera::GetActiveCamera()->GetMatEx()->m_vZAxis );
			m_hVecCreateActorList[m_nSelectClassIndex]->SetHeadLookTarget( vPos );
		}
	}
}

void CDnLoginTask::CharCreateGestureAction( const char* pGestureActionName, bool bCharCreate )
{
	if( pGestureActionName == NULL )
		return;

	if( bCharCreate && m_State == LoginStateEnum::CharCreate_ChangeParts )
	{
		if( m_nSelectClassIndex > -1 && m_nSelectClassIndex < static_cast<int>( m_hVecCreateActorList.size() ) )
		{
			m_hVecCreateActorList[m_nSelectClassIndex]->CmdAction( pGestureActionName );
		}
	}
	else if( !bCharCreate && m_State == LoginStateEnum::CharSelect )
	{
		if( m_hSlotActorHandle )
		{
			m_hSlotActorHandle->CmdAction( pGestureActionName );
		}
	}
}

#ifdef PRE_MOD_CREATE_CHAR
void CDnLoginTask::SetCharCreateBasePartsName()
{
	DnPartsHandle hParts;
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Hair][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hair]];
	GetInterface().SetCharCreatePartsName( CDnParts::Hair, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Face][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Face]];
	GetInterface().SetCharCreatePartsName( CDnParts::Face, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Body][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Body]];
	GetInterface().SetCharCreatePartsName( CDnParts::Body, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Leg][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Leg]];
	GetInterface().SetCharCreatePartsName( CDnParts::Leg, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Hand][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hand]];
	GetInterface().SetCharCreatePartsName( CDnParts::Hand, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Foot][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Foot]];
	GetInterface().SetCharCreatePartsName( CDnParts::Foot, hParts->GetName() );
}
#endif // PRE_MOD_CREATE_CHAR

void CDnLoginTask::OnRecvLoginWaitUser( SCWaitUser *pPacket )
{
	if( m_State == LoginStateEnum::StartGame )
	{
		std::wstring wszServerName = CGlobalInfo::GetInstance().GetServerName( m_nSelectedServerIndex );
		GetInterface().ShowWaitUserDlg( true );
		GetInterface().SetWaitUserProperty( wszServerName.c_str(),  pPacket->nWaitUserCount, pPacket->nEstimateTime );
	}
}

void CDnLoginTask::OnRecvLoginBlock( SCCHeckBlock *pPacket )
{
	// Penalty
	WCHAR wszTemp[512]={0,};

	std::wstring wszBeginTime, wszEndTime;
	DN_INTERFACE::STRING::GetTimeText( wszBeginTime, pPacket->StartDate );
	DN_INTERFACE::STRING::GetTimeText( wszEndTime, pPacket->EndDate );

	if( (pPacket->cType == _RESTRAINTTYPE_BLOCK) && pPacket->iReasonID == SecondAuth::Common::RestraintReasonID ) 
	{
		swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6417 ) , SecondAuth::Common::LimitCount   , SecondAuth::Common::RestraintTimeMin,  wszBeginTime.c_str(), wszEndTime.c_str() );
		GetInterface().MessageBox( wszTemp, MB_OK, UICallbackEnum::DisconnectTCP, this, true );
		return; 
	}

#if defined(_RDEBUG)
	bool bPermanent = false;
	DBTIMESTAMP DbTime;
	CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( pPacket->EndDate, &DbTime );
	if( DbTime.year == 2050 && DbTime.month == 1 && DbTime.day == 1 && DbTime.hour == 0 && DbTime.minute == 0 && DbTime.second == 0 ) bPermanent = true;
	if( DbTime.year == 2049 && DbTime.month == 12 && DbTime.day == 31 && DbTime.hour == 23 && DbTime.minute == 0 && DbTime.second == 0 ) bPermanent = true;
	if( bPermanent )
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4432 ) );
	else
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4400 ), pPacket->wszBlockReason, wszBeginTime.c_str(), wszEndTime.c_str() );
#else
	swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4400 ), pPacket->wszBlockReason, wszBeginTime.c_str(), wszEndTime.c_str() );
#endif
	GetInterface().MessageBox( wszTemp, MB_OK, UICallbackEnum::DisconnectTCP, this, true, false, false, true );
}

void CDnLoginTask::OnRecvLoginCharCreate( SCCreateChar *pPacket )
{
	FUNC_LOG();
	if( pPacket->nRet == ERROR_NONE )
	{
		ShowCreateCharacter( false );
		ChangeState( CharSelect );
		SelectServer( CGlobalInfo::GetInstance().m_nSelectedServerIndex );
		m_wstrCreateCharName = pPacket->wszCharacterName;
	}
	else
	{
		if( pPacket->nRet == ERROR_LOGIN_CANNOT_CREATE_CHARACTER_TODAY )
		{
			WCHAR wszTemp[128];
			swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100058 ), pPacket->cDailyCreateMaxCount );
			GetInterface().MessageBox( wszTemp, MB_OK, 0, GetInterface().GetCharCreateDlg() );
		}
		else
		{
			GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, 0, GetInterface().GetCharCreateDlg());
		}
	}
}


void CDnLoginTask::SetRequestWait( bool bWait )
{
	m_bRequestWait = bWait;
	if( bWait ) 
	{
		GetInterface().EnableCharSelectDlgControl( false, "ID_CREATE_CHAR" );
		GetInterface().EnableCharSelectDlgControl( false, "ID_DELETE_CHAR" );
		GetInterface().EnableCharSelectDlgControl( false, "ID_CONNECT" );
		GetInterface().EnableCharSelectDlgControl( false, "ID_BACK" );
	}
	else 
	{
		GetInterface().EnableCharSelectDlgControl( true, "ID_BACK" );
		GetInterface().EnableCharSelectDlgControl( ((m_nCharSlotMaxCount==CHARCOUNTMAX )?false:true), "ID_CREATE_CHAR" );
	}
}

void CDnLoginTask::OnRecvLoginCharDelete( SCDeleteChar *pPacket )
{
	FUNC_LOG();
	SetRequestWait( false );

	//// Do not run callback for Fail received on guild master deletion.
	//if( pPacket->nRet == 103101 )
	//{
	//	GetInterface().ServerMessageBox( pPacket->nRet, MB_OK );
	//	return;
	//}

	//if( pPacket->nRet == 103211 )
	//{
	//	GetInterface().ServerMessageBox( pPacket->nRet, MB_OK );
	//	return;
	//}

	// Block Callback handling.
	if( pPacket->nRet == 103101 || // Guild master.
		pPacket->nRet == 103211 || // Master and apprentice...
		pPacket->nRet == 103334 || // Best friend status.
		pPacket->nRet == 103346    // Join guild
		)
	{
		GetInterface().ServerMessageBox( pPacket->nRet, MB_OK );
		return;
	}

	if( pPacket->nRet != ERROR_NONE )
	{
		if( pPacket->nRet == ERROR_SECONDAUTH_CHECK_OLDPWINVALID || 
			pPacket->nRet == ERROR_SECONDAUTH_CHECK_FAILED)
		{
			WCHAR wzStrTmp[1024]= {0,};
			swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6303) , pPacket->cFailCount , SecondAuth::Common::LimitCount );
			GetInterface().MessageBox( wzStrTmp ) ;	
		}
		else 
		{
			GetInterface().ServerMessageBox( pPacket->nRet, MB_OK, UICallbackEnum::CharDeleteFailed, this );
		}
		return;
	}

	SelectServer( CGlobalInfo::GetInstance().m_nSelectedServerIndex );
}

void CDnLoginTask::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
#ifndef _FINAL_BUILD
	if( nReceiverState & IR_KEY_DOWN ) 
	{
		if( IsPushKey( DIK_LCONTROL ) )
		{
			if( CDnCamera::GetActiveCamera() == m_hDebugCamera )
				CDnCamera::SetActiveCamera( m_hCamera );
			else 
			{
				*m_hDebugCamera->GetCameraInfo() = *m_hCamera->GetCameraInfo();
				*m_hDebugCamera->GetMatEx() =  *m_hCamera->GetMatEx();
				m_hDebugCamera->GetMatEx()->m_vPosition.y += 50.0f;
				CDnCamera::SetActiveCamera( m_hDebugCamera );
			}
			ReleasePushKey( DIK_LCONTROL );
			ReleasePushKey( DIK_TAB );
		}
	}
#endif // _FINAL_BUILD

	if( m_State == LoginStateEnum::CharCreate_SelectClass )
	{
		if( nReceiverState & IR_MOUSE_LB_DOWN )
		{
			if( !m_bStartCharSelect && m_nSelectClassIndex == -1 )
			{
				int nIndex = GetCreateSelectClass();
				if( nIndex > -1 )
				{
					SetDOFInfo(30.0f, 500.0f, 30.0f, 0.0f, 6.0f);	
						GetInterface().SetCharCreateSelect(nIndex);
					GetInterface().SetFocusCharIndex( -1 );
				}
			}
		}
		else if( nReceiverState & IR_MOUSE_MOVE )
		{
			CDnAniCamera *pCamera = (CDnAniCamera *)m_hCamera.GetPointer();
			if( strstr( pCamera->GetCurrentAction(), m_strCharCreateSelectCamera.c_str() ) )
			{
				int nIndex = GetCreateSelectClass();
				SetSelectClassOutLine( nIndex );
					GetInterface().SetFocusCharIndex(nIndex);
			}
		}
	}
}

void CDnLoginTask::SetSelectClassOutLine( int nSelectIndex )
{
	if( nSelectIndex > -1 )
	{
		std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.find( nSelectIndex + 1 );
		if( iter != m_mapPropCamListByClass.end() )
		{
			ClassPropCamInfo& stClassPropCamInfo = iter->second;

			std::vector<CEtWorldProp*> pVecList;
			m_pWorld->FindPropFromCreateUniqueID( stClassPropCamInfo.nPropIndex, &pVecList );		// Character creation position
			if( !pVecList.empty() ) 
			{
				EtObjectHandle hObject;
				CDnWorldProp* pProp = (CDnWorldProp*)pVecList[0];
				hObject = pProp->GetObjectHandle();
				if( m_hSelectOutline->GetObject() != hObject )
				{
					//test??
					m_hSelectOutline->SetColor(EtColor(((float)rand() / (float)(RAND_MAX)) * 1.0, ((float)rand() / (float)(RAND_MAX)) * 1.0, ((float)rand() / (float)(RAND_MAX)) * 1.0, 1.0));
					//
					m_hSelectOutline->SetObject( hObject );
					m_hSelectOutline->Show( true, 2.0f );
				}
				pVecList.clear();
			}
		}
	}
	else
	{
		m_hSelectOutline->SetObject( CEtObject::Identity() );
		m_hSelectOutline->Show( false, 2.0f );
	}
}

int CDnLoginTask::GetCreateSelectClass()
{
	SSegment Segment;
	POINT p;
	::GetCursorPos( &p );
	ScreenToClient( CDnMainFrame::GetInstance().GetHWnd(), &p );
	if( m_hCamera->GetCameraHandle() ) m_hCamera->GetCameraHandle()->CalcPositionAndDir( p.x, p.y, Segment.vOrigin, Segment.vDirection );
	Segment.vDirection *= 100000.f;

	SSphere Sphere;

	std::vector<CEtWorldProp*> pVecList;

	int nIndex = 0;
	std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.begin();
	for( ; iter != m_mapPropCamListByClass.end(); iter++, nIndex++ )
	{
		ClassPropCamInfo& stClassPropCamInfo = iter->second;

		m_pWorld->FindPropFromCreateUniqueID( stClassPropCamInfo.nPropIndex, &pVecList );
		if( !pVecList.empty() ) 
		{
			CDnWorldProp* pProp = (CDnWorldProp*)pVecList[0];
			pProp->GetBoundingSphere( Sphere );
			Sphere.fRadius *= 0.35f;

			if( TestLineToSphere( Segment.vOrigin, Segment.vDirection, Sphere ) == true )
				return nIndex;

			pVecList.clear();
		}
	}

	return -1;
}

void CDnLoginTask::CreateCharacter( TCHAR* szName, int nSelectServerIndex )
{
	int Info[EQUIPMAX] = { 0, };
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Face] != -1 )
		Info[EQUIP_FACE] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Face][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Face]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hair] != -1 )
		Info[EQUIP_HAIR] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Hair][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hair]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Body] != -1 )
		Info[EQUIP_BODY] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Body][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Body]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Leg] != -1 )
		Info[EQUIP_LEG] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Leg][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Leg]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hand] != -1 )
		Info[EQUIP_HAND] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Hand][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hand]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Foot] != -1 )
		Info[EQUIP_FOOT] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Foot][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Foot]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Helmet] != -1 )
		Info[EQUIP_HELMET] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Helmet][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Helmet]];

	Info[EQUIP_WEAPON1] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nWeapon[0];
	Info[EQUIP_WEAPON2] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nWeapon[1];
#if defined( PRE_ADD_DWC )
	if( m_bDWCCharCreateMode )
	{
		SendCreateDWCChar( Info, szName,
			m_VecCreateDefaultPartsList[m_nSelectClassIndex].dwColor[MAPartsBody::HairColor],
			m_VecCreateDefaultPartsList[m_nSelectClassIndex].dwColor[MAPartsBody::EyeColor],
			m_VecCreateDefaultPartsList[m_nSelectClassIndex].dwColor[MAPartsBody::SkinColor],
			(BYTE)m_hVecCreateActorList[m_nSelectClassIndex]->GetClassID(),
			m_cDWCJobCode1, m_cDWCJobCode2, m_cDWCLevel, m_nDWCExp, m_nDWCMapID, m_nDWCGold, m_nDWCSkillPoint,
			(BYTE)nSelectServerIndex );
	}
	else
#endif // PRE_ADD_DWC
	{
		SendCreateChar( Info, szName, m_VecCreateDefaultPartsList[m_nSelectClassIndex].dwColor[MAPartsBody::HairColor],
						m_VecCreateDefaultPartsList[m_nSelectClassIndex].dwColor[MAPartsBody::EyeColor], 
						m_VecCreateDefaultPartsList[m_nSelectClassIndex].dwColor[MAPartsBody::SkinColor], 
						(BYTE)m_hVecCreateActorList[m_nSelectClassIndex]->GetClassID(), (BYTE)nSelectServerIndex , 0);
	}
#ifdef PRE_CHARLIST_SORTING
	SendCharacterSlotCode( m_eCharacterListSortCommand );
#endif // PRE_CHARLIST_SORTING
}

#if defined( PRE_ADD_DWC )
void CDnLoginTask::ResetDWCCharCreateMode()
{
	m_bDWCCharCreateMode = false;
	m_cDWCJobCode1 = 0;
	m_cDWCJobCode2 = 0;
	m_cDWCLevel = 60;
	m_nDWCExp = 0;
	m_nDWCMapID = 0;
	m_nDWCGold = 0;
	m_nDWCSkillPoint = 0;
}

void CDnLoginTask::StartDWCCharCreate( int nClassID, int nJobCode1, int nJobCode2, int nLevel, int nExp, int nMapID, int nGold, int nSkillPoint )
{
	int nTargetIndex = -1;
	for( size_t i = 0; i < m_hVecCreateActorList.size(); ++i )
	{
		if( m_hVecCreateActorList[i] && m_hVecCreateActorList[i]->GetClassID() == nClassID )
		{
			nTargetIndex = static_cast<int>( i );
			break;
		}
	}

	if( nTargetIndex < 0 && nClassID > 0 && (nClassID - 1) < static_cast<int>( m_hVecCreateActorList.size() ) )
		nTargetIndex = nClassID - 1;

	if( nTargetIndex < 0 )
		return;

	if( m_hSlotActorHandle )
	{
		m_hSlotActorHandle->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( -1, -1, true );
		SAFE_RELEASE_SPTR( m_hSlotActorHandle );
	}

	m_nSelectClassIndex = nTargetIndex;
	m_bDWCCharCreateMode = true;
	m_cDWCJobCode1 = static_cast<BYTE>( nJobCode1 );
	m_cDWCJobCode2 = static_cast<BYTE>( nJobCode2 );
	m_cDWCLevel = static_cast<BYTE>( nLevel > 0 ? nLevel : 60 );
	m_nDWCExp = nExp;
	m_nDWCMapID = nMapID;
	m_nDWCGold = nGold;
	m_nDWCSkillPoint = nSkillPoint;

	StartCharacterChangeParts();
}
#endif // PRE_ADD_DWC

void CDnLoginTask::DeleteCharacter()
{
	SendDeleteChar( GetSelectCharacterDBID( m_nSelectCharIndex ), m_nAuthPasswordSeed, m_nAuthPasswordValue );
	SetRequestWait( true );
#ifdef PRE_CHARLIST_SORTING
	SendCharacterSlotCode( m_eCharacterListSortCommand );
#endif // PRE_CHARLIST_SORTING
}

void CDnLoginTask::SetCharColor( MAPartsBody::PartsColorEnum colorType, int nColorIndex, int nClassIndex )
{
	DNTableFileFormat* pDefaultSox = GetDNTable( CDnTableDB::TDEFAULTCREATE );
	if( pDefaultSox == NULL ) return;

	int nClass = ( nClassIndex == -1) ? m_nSelectClassIndex : nClassIndex;
	
	switch( colorType ) 
	{
		case MAPartsBody::HairColor:
			{
				D3DCOLOR d3dColor = D3DCOLOR_XRGB( pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_HairColor%dR", nColorIndex+1).c_str() )->GetInteger(),
												pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_HairColor%dG", nColorIndex+1).c_str() )->GetInteger(),
												pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_HairColor%dB", nColorIndex+1).c_str() )->GetInteger() );

				MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[ nClass ].GetPointer());
				float fIntensity = 1.1f;
				DWORD dwColorR10G10B10=0;
				CDnParts::ConvertD3DCOLORToR10G10B10( &dwColorR10G10B10, d3dColor, fIntensity );
				m_VecCreateDefaultPartsList[ nClass ].dwColor[ MAPartsBody::HairColor ] = dwColorR10G10B10;
				pPartsBody->SetPartsColor( MAPartsBody::HairColor, dwColorR10G10B10 );
			}
			break;
		case MAPartsBody::SkinColor:
			{
				int nA = (int)(pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_SkinColor%dA", nColorIndex+1).c_str() )->GetFloat() * 255);
				int nR = (int)(pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_SkinColor%dR", nColorIndex+1).c_str() )->GetFloat() * 255);
				int nG = (int)(pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_SkinColor%dG", nColorIndex+1).c_str() )->GetFloat() * 255);
				int nB = (int)(pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_SkinColor%dB", nColorIndex+1).c_str() )->GetFloat() * 255);
				
				ASSERT( nA >= 0 && nR >= 0 && nG >= 0 && nB >= 0 );
				ASSERT( nA <= 255 && nR <= 255 && nG <= 255 && nB <= 255 );

				DWORD dwSkinColor = D3DCOLOR_ARGB( nA, nR, nG, nB );
				m_VecCreateDefaultPartsList[ nClass ].dwColor[ MAPartsBody::SkinColor ] = dwSkinColor;

				MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[ nClass ].GetPointer());
				pPartsBody->SetPartsColor( MAPartsBody::SkinColor, dwSkinColor );
			}
			break;
		case MAPartsBody::EyeColor:
			{
				D3DCOLOR d3dColor = D3DCOLOR_XRGB( pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_EyeColor%dR", nColorIndex+1).c_str() )->GetInteger(),
												pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_EyeColor%dG", nColorIndex+1).c_str() )->GetInteger(),
												pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_EyeColor%dB", nColorIndex+1).c_str() )->GetInteger() );

				MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[ nClass ].GetPointer());			
				float fIntensity = 1.0f;
				DWORD dwColorR10G10B10=0;
				CDnParts::ConvertD3DCOLORToR10G10B10( &dwColorR10G10B10, d3dColor, fIntensity );
				m_VecCreateDefaultPartsList[ nClass ].dwColor[ MAPartsBody::EyeColor ] = dwColorR10G10B10;
				pPartsBody->SetPartsColor( MAPartsBody::EyeColor, dwColorR10G10B10 );
			}
			break;
	}
}

void CDnLoginTask::RotateCreateParts( CDnParts::PartsTypeEnum PartsIndex, bool bAdd )
{
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[PartsIndex].empty() ) return;

	if( bAdd ) m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex]++;
	else m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex]--;

	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex] >= (int)m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[PartsIndex].size() ) 
		m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex] = 0;
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex] < 0 ) 
		m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex] = (int)m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[PartsIndex].size() - 1;

	MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[m_nSelectClassIndex].GetPointer());

	pPartsBody->DetachParts( PartsIndex );
	DnPartsHandle hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[PartsIndex][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex]];
	pPartsBody->AttachParts( hParts, (CDnParts::PartsTypeEnum)-1, false, false );

	pPartsBody->SetPartsColor( MAPartsBody::HairColor, m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[MAPartsBody::HairColor] );
	pPartsBody->SetPartsColor( MAPartsBody::SkinColor, m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[MAPartsBody::SkinColor] );
	pPartsBody->SetPartsColor( MAPartsBody::EyeColor, m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[MAPartsBody::EyeColor] );

/*
	if( hParts->GetPartsType() == CDnParts::PartsTypeEnum::Face ) {
		hParts->UpdateEyeColorParam( m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[EyeColor] );
	}

	CDnPartsHair *pPartsHair = dynamic_cast<CDnPartsHair*>(hParts.GetPointer());
	if( pPartsHair ) {	
		pPartsHair->SetColor( m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[HairColor] );
		pPartsHair->UpdateColorParam();
	}
*/
#ifdef PRE_MOD_CREATE_CHAR
	GetInterface().SetCharCreatePartsName( PartsIndex, hParts->GetName() );
#endif // PRE_MOD_CREATE_CHAR
}

static bool CompareServerList( TServerListData *s1, TServerListData *s2 )
{
	// Higher character count first
	if( s1->cMyCharCount < s2->cMyCharCount ) return false;
	else if( s1->cMyCharCount > s2->cMyCharCount ) return true;

	return false;
}

void CDnLoginTask::OnRecvLoginServerList( SCServerList *pPacket )
{
	FUNC_LOG();

	CGlobalInfo::GetInstance().m_mapServerList.clear();

	if( pPacket->cServerCount > 0 )
	{
		GetInterface().ClearServerList();
		for( int i=0; i<pPacket->cServerCount; ++i )
		{
			CGlobalInfo::GetInstance().m_mapServerList.insert( make_pair( pPacket->ServerListData[i].cWorldID, pPacket->ServerListData[i].wszServerName ) );
		}
#ifndef _FINAL_BUILD
		for( int i=0; i<pPacket->cServerCount; ++i )
		{
			if( pPacket->ServerListData[i].bOnline )
			{
				GetInterface().AddServerList( pPacket->ServerListData[i].cWorldID, pPacket->ServerListData[i].wszServerName,
											float(pPacket->ServerListData[i].nWorldCurUser) / float(pPacket->ServerListData[i].nWorldMaxUser), pPacket->ServerListData[i].cMyCharCount );
			}
		}
#else
		std::vector<TServerListData*> vecServerInfo;
		for( int i=0; i<pPacket->cServerCount; i++ )
		{
			if( pPacket->ServerListData[i].bOnTop == false )
			{
				vecServerInfo.push_back( &pPacket->ServerListData[i] );
			}
			else		//If bOnTop is enabled, put it on top
			{
				if( pPacket->ServerListData[i].bOnline )
				{
					GetInterface().AddServerList( pPacket->ServerListData[i].cWorldID, pPacket->ServerListData[i].wszServerName,  
												float(pPacket->ServerListData[i].nWorldCurUser) / float(pPacket->ServerListData[i].nWorldMaxUser), pPacket->ServerListData[i].cMyCharCount );
				}
			}
		}

		std::sort( vecServerInfo.begin(), vecServerInfo.end(), CompareServerList );
		int nCharCountZeroIndex = -1;
		for( int i=0; i<(int)vecServerInfo.size(); i++ ) 
		{
			if( vecServerInfo[i]->cMyCharCount == 0 ) 
			{
				nCharCountZeroIndex = i;
				break;
			}
		}

		if( nCharCountZeroIndex != -1 )
			std::random_shuffle( vecServerInfo.begin()+nCharCountZeroIndex, vecServerInfo.end() );

		for( int i=0; i<(int)vecServerInfo.size(); i++ )
		{
			if( pPacket->ServerListData[i].bOnline )
			{
				GetInterface().AddServerList( vecServerInfo[i]->cWorldID, vecServerInfo[i]->wszServerName,  
											float(vecServerInfo[i]->nWorldCurUser) / float(vecServerInfo[i]->nWorldMaxUser), vecServerInfo[i]->cMyCharCount );
			}
		}
#endif
		GetInterface().SetDefaultServerList();

#ifndef _FINAL_BUILD
		if( CGlobalValue::GetInstance().IsPermitLogin() )
		{
			SelectServer( CGlobalInfo::GetInstance().GetServerIndex( CGlobalValue::GetInstance().m_szServerName.c_str() ) );
			SendSelectedServer( (BYTE)CGlobalInfo::GetInstance().GetServerIndex( CGlobalValue::GetInstance().m_szServerName.c_str() ) );
		}
#endif
	}
	else
	{
		CDebugSet::ToLogFile( "CDnLoginTask::OnRecvLoginCheckServer, Server Count is 0!" );
	}

	GetInterface().ShowServerList( true );
}

void CDnLoginTask::OnRecvSystemGameInfo( SCGameInfo *pPacket )
{
	if( pPacket->nRet != ERROR_NONE ) 
	{
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::GameInfoFailed, this);
		return;
	}
}

void CDnLoginTask::OnRecvSystemVillageInfo( SCVillageInfo *pPacket )
{
	if( pPacket->nRet != ERROR_NONE ) 
	{
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::VillageInfoFailed, this);
		return;
	}

//	g_pServiceSetup->WriteStageLog_( IServiceSetup::NHN_StartGmae , _T("StartGmae"));	
}

void CDnLoginTask::OnRecvLoginChannelList( SCChannelList *pPacket )
{
	FUNC_LOG();

	WCHAR wzStrTmp[1024]={0,};

	switch(pPacket->nRet)
	{
		case ERROR_NONE:
			break;
#if defined(PRE_MOD_SELECT_CHAR)
		case ERROR_GENERIC_MASTERCON_NOT_FOUND:
			GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, NULL, this);
			m_State = CDnLoginTask::CharSelect; 
			ChangeState( CDnLoginTask::CharSelect, false );
			return;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
		case ERROR_SECONDAUTH_CHECK_MAXFAILED:
			swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6305) , pPacket->cFailCount );
			GetInterface().MessageBox( wzStrTmp );
			// Temporarily force-switch. On ChangeState, to prevent duplicate Back button when Prev is ChannelList
			// Check code is included; replace it if it looks ugly.
			m_State = CDnLoginTask::CharSelect; 
			ChangeState( CDnLoginTask::CharSelect, false );
			return;
		case ERROR_SECONDAUTH_CHECK_OLDPWINVALID:
		case ERROR_SECONDAUTH_CHECK_FAILED: 
		default:
			swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6303) , pPacket->cFailCount , SecondAuth::Common::LimitCount );
			GetInterface().MessageBox( wzStrTmp ) ;	
			m_State = CDnLoginTask::CharSelect;
			ChangeState( CDnLoginTask::CharSelect, false );
			return;	
	}

	GetInterface().ShowWaitUserDlg( false );
	GetInterface().ShowChannelList( true );

#ifndef _FINAL_BUILD
	if( CGlobalValue::GetInstance().IsPermitLogin() )
	{
		for( int i=0; i<pPacket->cCount; i++ )
		{
			if( (i+1) == (BYTE)CGlobalValue::GetInstance().m_nChannelIndex )
			{
				m_nSelectedChannelIndex = pPacket->Info->nChannelID;
				ChangeState( ChannelList );
				return;
			}
			else
			{
				CDebugSet::ToLogFile( "CDnLoginTask::OnRecvLoginChannelList, Invalid Channel Index(%d)!", (BYTE)CGlobalValue::GetInstance().m_nChannelIndex );
			}
		}

		return;
	}
#endif

	std::vector<sChannelInfo*> vecChannelInfo;

	for( int i=0; i<pPacket->cCount; i++ )
	{
		vecChannelInfo.push_back( &pPacket->Info[i] );
	}

	std::stable_sort( vecChannelInfo.begin(), vecChannelInfo.end(), CVillageClientSession::CompareChannelInfo );		// Sort by best connection/comfort.

	for( int i=0; i<(int)vecChannelInfo.size(); i++ )
	{
		GetInterface().AddChannelList( vecChannelInfo[i] );
	}

	GetInterface().SetDefaultChannelList();
}

void CDnLoginTask::OnRecvLoginBackButton()
{
	CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( pTask )
	{
		pTask->ChangeState( CDnLoginTask::RequestIDPass );
	}

}

void CDnLoginTask::OnRecvLoginReviveChar( SCReviveChar *pPacket )
{
	if( pPacket->nRet != 0 )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1210 ), MB_OK, UICallbackEnum::CharReviveFailed );
		return;
	}
	SelectServer( CGlobalInfo::GetInstance().m_nSelectedServerIndex );
}


void CDnLoginTask::SelectServer( int nServerIndex )
{
	m_nSelectedServerIndex = nServerIndex;
	CGlobalInfo::GetInstance().m_nSelectedServerIndex = nServerIndex;
	SendSelectedServer( (BYTE)m_nSelectedServerIndex );
	SetRequestWait( true );
}

void CDnLoginTask::SelectChannel( int nChannelIndex )
{
	//Initial game start
	m_nSelectedChannelIndex = nChannelIndex;
	ChangeState( StartGame );
}

void CDnLoginTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	switch( nID )
	{
		case UICallbackEnum::ConnectLoginFailed:
		case UICallbackEnum::CheckVersionFailed:
		case UICallbackEnum::DisconnectTCP:
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
				{
					if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );
					CTaskManager::GetInstance().RemoveAllTask( false );
				}
			}
			break;
		case UICallbackEnum::CharListFailed:
		case UICallbackEnum::CharCreateFailed:
			break;
		case UICallbackEnum::CharDeleteFailed:
			{
				SendBackButton();
				ChangeState(LoginStateEnum::ServerList);
			}
			break;
		case UICallbackEnum::CheckLoginFailed:
			break;
		case UICallbackEnum::GameInfoFailed:
		case UICallbackEnum::VillageInfoFailed:
			ChangeState( LoginStateEnum::CharSelect );
			break;
		case UICallbackEnum::CharReviveFailed:
			SelectServer( CGlobalInfo::GetInstance().m_nSelectedServerIndex );
			break;
		case FADE_DIALOG:
			if( nCommand == EVENT_FADE_COMPLETE )
			{
				switch( m_State )
				{
					case LoginStateEnum::Logo:
						GetInterface().ChangeState( LoginStateEnum::RequestIDPass );
						break;
					case LoginStateEnum::ServerList:
						{
							if( m_bCharSelectToServerList )
							{
								SendServerList();
								LoadTitle();
								GetInterface().FadeDialog( 0xFF000000, 0x00000000, 3.0f, NULL );
								m_bCharSelectToServerList = false;
								if( m_hSlotActorHandle )
								{
									m_hSlotActorHandle->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( -1, -1, true );
									SAFE_RELEASE_SPTR( m_hSlotActorHandle );
								}
							}
							else
							{
								if( m_nSelectedServerIndex == -1 ) break;
								ChangeState( CharSelect );
							}
						}
						break;
					case LoginStateEnum::CharSelect:
						GetInterface().GetInstance().ChangeState( CharSelect );
						break;
					case LoginStateEnum::CharCreate_SelectClass:
						{
							CDnAniCamera* pCamera = (CDnAniCamera *)m_hCamera.GetPointer();
							if( pCamera && ( !strstr( pCamera->GetCurrentAction(), m_strCharCreateStartCamera.c_str() ) && !strstr( pCamera->GetCurrentAction(), m_strCharCreateSelectCamera.c_str() ) ) )
							{
								if( m_hSlotActorHandle )
								{
									m_hSlotActorHandle->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( -1, -1, true );
									SAFE_RELEASE_SPTR( m_hSlotActorHandle );
								}

								ShowCreateCharacter( false );
								ShowSelectCharacter( true );
								GetInterface().FadeDialog( 0xFF000000, 0x00000000, 1.5f, NULL );
								if (m_PrevState == LoginStateEnum::CharCreate_ChangeParts)
								{
									pCamera->PlayCamera( m_strCharCreateSelectCamera.c_str(), NULL );
									GetInterface().ShowCharCreateSelectDlg( true );
									GetInterface().ShowCharCreateBackDlg( true );
								}
								else
								{
									if( m_pWorld )
									{
										std::vector<CEtWorldProp*> pVecList;
										m_pWorld->FindPropFromCreateUniqueID( 840, &pVecList );		// Black Dragon action
										if( !pVecList.empty() ) 
										{
											CDnWorldActProp* pProp = dynamic_cast<CDnWorldActProp*>(pVecList[0]);
											if( pProp )
												pProp->SetActionQueue( "Idle" );

											m_bShowBlackDragonRadialBlur = true;
										}
									}
									
									pCamera->PlayCamera( m_strCharCreateStartCamera.c_str(), m_strCharCreateSelectCamera.c_str() );
									CEtSoundEngine::GetInstance().PlaySound__("3D", m_nCharCreateStartSoundIndex );
									m_bStartCharSelect = true;
									GetInterface().ShowCharCreateServerNameDlg( false );
									GetInterface().ShowCharLoginTitleDlg( false );
									}
								}
						}
						break;
					case LoginStateEnum::CharCreate_ChangeParts:
						{
							CDnAniCamera* pCamera = (CDnAniCamera *)m_hCamera.GetPointer();
#if defined( PRE_ADD_DWC )
							bool bDwcPendingFadeIn = m_bDWCCharCreateMode && ( !GetInterface().GetCharCreateDlg() || !GetInterface().GetCharCreateDlg()->IsShow() );
							if( bDwcPendingFadeIn || ( pCamera && strstr( pCamera->GetCurrentAction(), "_FadeOut" ) ) )
#else
							if( pCamera && strstr( pCamera->GetCurrentAction(), "_FadeOut" ) )
#endif // PRE_ADD_DWC
							{
								ShowSelectCharacter( false );
								m_fChangePartsFadeInTime = 0.5f;
							}
						}
						break;
				}
			}
			break;
	}
}

bool CDnLoginTask::IsEmptySelectActorSlot()
{
	return m_nCharSlotCount == 0 ? true : false;
}

int CDnLoginTask::GetCurrentSelectLevel()
{
	if( m_nSelectCharIndex < 0 || m_hSlotActorHandle == NULL )
		return -1;

	return m_hSlotActorHandle->GetLevel();
}

void CDnLoginTask::GetOriginalDOFInfo()
{							
	if( !m_pWorld ) return;
	int nDOFIndex = m_pWorld->GetEnvironment()->GetDOFEffectIndex();
	if( nDOFIndex != -1 ) 
	{
		CDnCEDof* pEffect = (CDnCEDof*)m_hCamera->FindCameraEffect( nDOFIndex );
		if( pEffect ) 
		{
			pEffect->GetBlurSize( &m_fOldDOFNearBlurSize, &m_fOldDOFFarBlurSize );
			CEtDOFFilter* pDOFFilter = pEffect->GetFilter();
			if( pDOFFilter )
			{
				m_fOriginalFarStart = pDOFFilter->GetFarDOFStart();
				m_fOriginalFarEnd = pDOFFilter->GetFarDOFEnd();
				m_fOriginalFocusDist = pDOFFilter->GetFocusDistance();
			}
		}
	}
}

void CDnLoginTask::RestoreOriginalDOFInfo()
{
	if( !m_pWorld ) return;
	int nDOFIndex = m_pWorld->GetEnvironment()->GetDOFEffectIndex();
	if( nDOFIndex != -1 )
	{
		CDnCEDof* pEffect = (CDnCEDof*)m_hCamera->FindCameraEffect( nDOFIndex );
		if( pEffect ) 
		{
			pEffect->SetBlurSize( m_fOldDOFNearBlurSize, m_fOldDOFFarBlurSize );
			CEtDOFFilter* pDOFFilter = pEffect->GetFilter();
			if( pDOFFilter )
			{
				pDOFFilter->SetFarDOFStart( m_fOriginalFarStart );
				pDOFFilter->SetFarDOFEnd( m_fOriginalFarEnd );
				pDOFFilter->SetFocusDistance( m_fOriginalFocusDist );
			}
		}
	}
}

void CDnLoginTask::SetDOFInfo( float fDOFStart, float fDOFEnd, float fDOFFocusDistance, float fNearBlurSize, float fFarBlurSize )
{
	if( !m_pWorld ) return;
	int nDOFIndex = m_pWorld->GetEnvironment()->GetDOFEffectIndex();
	if( nDOFIndex != -1 )
	{
		CDnCEDof* pEffect = (CDnCEDof*)m_hCamera->FindCameraEffect( nDOFIndex );
		if( pEffect ) 
		{
			CEtDOFFilter* pDOFFilter = pEffect->GetFilter();
			if( pDOFFilter )
			{
				pDOFFilter->SetFarDOFStart( fDOFStart );
				pDOFFilter->SetFarDOFEnd( fDOFEnd );
				pDOFFilter->SetFocusDistance( fDOFFocusDistance );
			}

			pEffect->SetBlurSize( fNearBlurSize, fFarBlurSize );
		}
	}
}

void CDnLoginTask::ReviveCharacter( int nSlotIndex )
{
	SendReviveDeleteChar( GetSelectCharacterDBID( nSlotIndex ) );
#ifdef PRE_CHARLIST_SORTING
	SendCharacterSlotCode( m_eCharacterListSortCommand );
#endif // PRE_CHARLIST_SORTING
}

bool CDnLoginTask::OnCloseTask()
{
	if( !CDnInterface::IsActive() ) return false;


// #69613 - Removed message change handling.
//#ifdef PRE_ADD_NEWCOMEBACK
//	CDnBridgeTask *pTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
//	if( pTask->GetComeback() )
//		GetInterface().MessageBox( 4948, MB_YESNO, MESSAGEBOX_21, CDnInterface::GetInstancePtr() ); // "If you exit without logging into the game, you will not receive returnee rewards upon reconnecting. Do you want to exit?"
//	else
//		GetInterface().MessageBox( MESSAGEBOX_21, MB_YESNO, MESSAGEBOX_21, CDnInterface::GetInstancePtr() );
//#else
	GetInterface().MessageBox( MESSAGEBOX_21, MB_YESNO, MESSAGEBOX_21, CDnInterface::GetInstancePtr() );
//#endif // PRE_ADD_NEWCOMEBACK

	return true;
}

void CDnLoginTask::DeSelectCharacter()
{

	if( m_nSelectClassIndex == -1 )
		return;

	std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.find( m_nSelectClassIndex + 1 );
	if( iter != m_mapPropCamListByClass.end() )
	{
		CDnAniCamera* pCamera = (CDnAniCamera*)m_hCamera.GetPointer();
		if( pCamera )
		{
			ClassPropCamInfo& stClassPropCamInfo = iter->second;
			pCamera->PlayCamera( stClassPropCamInfo.vecCamList[CAM_DESELECT].c_str(), m_strCharCreateSelectCamera.c_str() );
			float fTotalFrame = pCamera->GetTotalFrame( stClassPropCamInfo.vecCamList[CAM_SELECT].c_str() );
			DWORD dwTime = static_cast<DWORD>( ( fTotalFrame / 60.0f ) * 800.0f );	// Use only 80% of time so the trailing part does not look awkward
			EtVector2 vCenter( 0.5f, 0.5f );
			pCamera->RadialBlur( dwTime, vCenter, 0.1f, 0.2f, 0.4f );
			CEtSoundEngine::GetInstance().PlaySound__("3D", stClassPropCamInfo.vecSoundList[1] );
		}
		m_nSelectClassIndex = -1;
	}

	m_nSelectClassIndex = -1;
	m_bStartSelectMove = false;
	SetDOFInfo( 250.0f, 500.0f, 250.0f, 0.0f, 3.5f );
	GetInterface().ShowCharCreatePlayerInfo( false );
	GetInterface().ResetCharSelectDlgButton();
	GetInterface().ShowCharCreateBackDlg( true );
}


void CDnLoginTask::SelectMoveCharacter( int nSelectIndex )
{
	if( nSelectIndex == -1 )
		return;

	if( m_nSelectClassIndex == -1 )
	{
		m_nSelectClassIndex = nSelectIndex;
		SetDOFInfo( 30.0f, 500.0f, 30.0f, 0.0f, 6.0f );
		GetInterface().SetCharCreateSelect( m_nSelectClassIndex );
		SetSelectClassOutLine( m_nSelectClassIndex );
		m_bStartSelectMove = true;

	}
	else if( m_nSelectClassIndex != nSelectIndex )
	{
		std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.find( m_nSelectClassIndex + 1 );
		if( iter != m_mapPropCamListByClass.end() )
		{
			CDnAniCamera* pCamera = (CDnAniCamera*)m_hCamera.GetPointer();
			if( pCamera )
			{
				ClassPropCamInfo& stClassPropCamInfo = iter->second;
				pCamera->PlayCamera( stClassPropCamInfo.vecCamList[CAM_DESELECT].c_str(), m_strCharCreateSelectCamera.c_str() );
				float fTotalFrame = pCamera->GetTotalFrame( stClassPropCamInfo.vecCamList[CAM_SELECT].c_str() );
				DWORD dwTime = static_cast<DWORD>( ( fTotalFrame / 60.0f ) * 800.0f );	// Use only 80% of time so the trailing part does not look awkward
				EtVector2 vCenter( 0.5f, 0.5f );
				pCamera->RadialBlur( dwTime, vCenter, 0.1f, 0.2f, 0.4f );
				CEtSoundEngine::GetInstance().PlaySound__("3D", stClassPropCamInfo.vecSoundList[1] );
			}
			m_nSelectClassIndex = nSelectIndex;
			GetInterface().ShowCharCreatePlayerInfo( false );
			SetSelectClassOutLine( m_nSelectClassIndex );
		}
		m_bStartSelectMove = true;
	}
}

void CDnLoginTask::SetSelectedCharacterDark(int JobID)
{
	m_nSelectClassIndex = JobID;
}

void CDnLoginTask::StartCharacterChangeParts()
{
	if( m_nSelectClassIndex == -1 )
		return;

	ChangeState( LoginStateEnum::CharCreate_ChangeParts );

	std::map<int, ClassPropCamInfo>::iterator iter = m_mapPropCamListByClass.find( m_nSelectClassIndex + 1 );
	if( iter != m_mapPropCamListByClass.end() )
	{
		if( m_hCamera )
		{
			ClassPropCamInfo& stClassPropCamInfo = iter->second;
			((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( stClassPropCamInfo.vecCamList[CAM_FADEOUT].c_str(), NULL, false );
		}
		GetInterface().FadeDialog( 0x00000000, 0xFF000000, 0.5f, this );
	}
#if defined( PRE_ADD_DWC )
	else if( m_bDWCCharCreateMode )
	{
		GetInterface().FadeDialog( 0x00000000, 0xFF000000, 0.5f, this );
	}
#endif // PRE_ADD_DWC

	GetInterface().SetClassHairColor( m_nSelectClassIndex );
	GetInterface().ShowCharCreatePlayerInfo( false );
	GetInterface().ShowCharCreateSelectDlg( false );
}

void CDnLoginTask::InitShowPartsList()
{
	ResetShowPartsList();

	if( m_nSelectClassIndex < 0 )
		return;

	//test
	OutputDebug("[%s] test", __FUNCTION__);

	DNTableFileFormat* pDefaultCreateTable = GetDNTable( CDnTableDB::TDEFAULTCREATE );
	if( pDefaultCreateTable == NULL )
		return;

	int nActorCount = pDefaultCreateTable->GetItemCount();
	int nClassID = 0;
	std::vector<int> vecCostumePreviewID;

	for( int i=1; i<=nActorCount; i++ ) 
	{
		nClassID = pDefaultCreateTable->GetFieldFromLablePtr( i, "_ClassID" )->GetInteger();
		if( nClassID == m_nSelectClassIndex + 1  )
		{
			vecCostumePreviewID.push_back( pDefaultCreateTable->GetFieldFromLablePtr( i, "_CostumePreviewID1" )->GetInteger() );
			vecCostumePreviewID.push_back( pDefaultCreateTable->GetFieldFromLablePtr( i, "_CostumePreviewID2" )->GetInteger() );
			vecCostumePreviewID.push_back( pDefaultCreateTable->GetFieldFromLablePtr( i, "_CostumePreviewID3" )->GetInteger() );
			break;
		}
	}

	DNTableFileFormat* pDefaultCreateCostumeTable = GetDNTable( CDnTableDB::TDEFAULTCREATECOSTUME );
	if( pDefaultCreateCostumeTable == NULL )
		return;

	for( int i=0; i<static_cast<int>( vecCostumePreviewID.size() ); i++ )
	{
		DefaultPartsStruct sPreviewCostume;

		int nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Helmet" )->GetInteger();
		sPreviewCostume.nVecList[0].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[0].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );

		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Body" )->GetInteger();
		sPreviewCostume.nVecList[1].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[1].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );

		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Leg" )->GetInteger();
		sPreviewCostume.nVecList[2].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[2].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );

		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Hand" )->GetInteger();
		sPreviewCostume.nVecList[3].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[3].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );

		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Foot" )->GetInteger();
		sPreviewCostume.nVecList[4].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[4].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );


		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Weapon" )->GetInteger();
		sPreviewCostume.hWeapon[0] = CDnWeapon::CreateWeapon( nCostumeIndex, 0 );
		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Weapon1" )->GetInteger();
		sPreviewCostume.hWeapon[1] = CDnWeapon::CreateWeapon( nCostumeIndex, 1 );

		m_VecPreviewCostumePartsList.push_back( sPreviewCostume );
	}
}

void CDnLoginTask::ResetShowPartsList()
{
	for( DWORD i=0; i<m_VecPreviewCostumePartsList.size(); i++ ) 
	{
		for( int j=0; j<CDnParts::PartsTypeEnum_Amount; j++ ) 
		{
			SAFE_RELEASE_SPTRVEC( m_VecPreviewCostumePartsList[i].hVecParts[j] );
			SAFE_DELETE_VEC( m_VecPreviewCostumePartsList[i].nVecList[j] );
		}
		SAFE_RELEASE_SPTR( m_VecPreviewCostumePartsList[i].hWeapon[0] );
		SAFE_RELEASE_SPTR( m_VecPreviewCostumePartsList[i].hWeapon[1] );
	}
	SAFE_DELETE_VEC( m_VecPreviewCostumePartsList );
}

void CDnLoginTask::ShowDefaultPartsList( bool bFirstRandom )
{
	_srand( m_LocalTime );

	if( m_nSelectClassIndex < 0 || m_nSelectClassIndex >= static_cast<int>( m_VecCreateDefaultPartsList.size() ) )
		return;

	MAPartsBody* pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[m_nSelectClassIndex].GetPointer());
	if( pPartsBody == NULL )
		return;

	CDnPlayerActor*	pPlayer;
	pPlayer = dynamic_cast<CDnPlayerActor*>( m_hVecCreateActorList[m_nSelectClassIndex].GetPointer() );
	if( pPlayer == NULL )
		return;

	for( int i=0; i<EQUIPMAX; i++ )
	{
		if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[i].empty() ) continue;
		if( bFirstRandom )
			m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[i] = _rand()%(int)m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[i].size();

		DnPartsHandle hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[i][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[i]];
		if( hParts )
			pPartsBody->AttachParts( hParts, (CDnParts::PartsTypeEnum)-1, false, false );
	}

	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].hWeapon[0] )
	{
		m_VecCreateDefaultPartsList[m_nSelectClassIndex].hWeapon[0]->ShowRenderBase( true );
		pPlayer->CDnPlayerActor::AttachWeapon( m_VecCreateDefaultPartsList[m_nSelectClassIndex].hWeapon[0], 0, false );
	}

	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].hWeapon[1] )
	{
		m_VecCreateDefaultPartsList[m_nSelectClassIndex].hWeapon[1]->ShowRenderBase( true );
		pPlayer->CDnPlayerActor::AttachWeapon( m_VecCreateDefaultPartsList[m_nSelectClassIndex].hWeapon[1], 1, false );
	}
#ifdef PRE_MOD_WARRIOR_RIGHTHAND
	// Since login does not call CombineParts, forcibly call hiding right glove mesh when warrior equips gauntlet.
	pPlayer->OnCombineParts();
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
}

void CDnLoginTask::DetachAllParts()
{
	MAPartsBody* pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[m_nSelectClassIndex].GetPointer());
	if( pPartsBody == NULL )
		return;

	for( int i=0; i<EQUIPMAX; i++ )
		pPartsBody->DetachParts( (CDnParts::PartsTypeEnum)i );

	((CDnPlayerActor*)m_hVecCreateActorList[m_nSelectClassIndex].GetPointer())->CDnPlayerActor::DetachWeapon( 0 );
	((CDnPlayerActor*)m_hVecCreateActorList[m_nSelectClassIndex].GetPointer())->CDnPlayerActor::DetachWeapon( 1 );

	for( int i=CASHEQUIP_HELMET; i<=CASHEQUIP_FAIRY; i++ )
		pPartsBody->DetachCashParts( (CDnParts::PartsTypeEnum)i );

	((CDnPlayerActor*)m_hVecCreateActorList[m_nSelectClassIndex].GetPointer())->CDnPlayerActor::DetachCashWeapon( 0 );
	((CDnPlayerActor*)m_hVecCreateActorList[m_nSelectClassIndex].GetPointer())->CDnPlayerActor::DetachCashWeapon( 1 );
}

void CDnLoginTask::ShowPartsList( int nPartsListIndex )
{
	DetachAllParts();
	ShowDefaultPartsList();
	
	if( nPartsListIndex > 0 && nPartsListIndex <= static_cast<int>( m_VecPreviewCostumePartsList.size() ) )
	{
		int nPartsList = nPartsListIndex - 1;

		MAPartsBody* pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[m_nSelectClassIndex].GetPointer());
		if( pPartsBody == NULL )
			return;

		CDnPlayerActor*	pPlayer;
		pPlayer = dynamic_cast<CDnPlayerActor*>( m_hVecCreateActorList[m_nSelectClassIndex].GetPointer() );
		if( pPlayer == NULL )
			return;

		for( int i=CASHEQUIP_HELMET; i<=CASHEQUIP_FAIRY; i++ )
		{
			if( m_VecPreviewCostumePartsList[nPartsList].hVecParts[i].size() > 0)
			{
				DnPartsHandle hParts = m_VecPreviewCostumePartsList[nPartsList].hVecParts[i][0];
				if( hParts )
				{
					pPartsBody->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)i, false, false );
				}
			}
		}

		if( m_VecPreviewCostumePartsList[nPartsList].hWeapon[0] )
		{
			m_VecPreviewCostumePartsList[nPartsList].hWeapon[0]->ShowRenderBase( true );
			pPlayer->CDnPlayerActor::AttachCashWeapon( m_VecPreviewCostumePartsList[nPartsList].hWeapon[0], 0, false );
		}
		
		if( m_VecPreviewCostumePartsList[nPartsList].hWeapon[1] )
		{
			m_VecPreviewCostumePartsList[nPartsList].hWeapon[1]->ShowRenderBase( true );
			pPlayer->CDnPlayerActor::AttachCashWeapon( m_VecPreviewCostumePartsList[nPartsList].hWeapon[1], 1, false );
		}
#ifdef PRE_MOD_WARRIOR_RIGHTHAND
		// Since login does not call CombineParts, forcibly call hiding right glove mesh when warrior equips gauntlet.
		pPlayer->OnCombineParts();
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
	}
}

#else // PRE_MOD_SELECT_CHAR


CDnLoginTask::CDnLoginTask()
	: CInputReceiver( true )
{
	m_pWorld = NULL;
	m_nCharSlotCount = 0;

	m_fCameraDelta = 0.f;

	m_State = Logo;
	m_PrevState = Logo;

	m_nSelectedServerIndex = -1;
	m_nSelectedChannelIndex= -1;

	m_bProcessCamera = false;
	m_bShowHideActor = false;

	m_bCompleteCharList = false;
	m_CharList.nRet = ERROR_LOGIN_FAIL;
	m_nSelectIndex = -1;
	m_fOldDOFNearBlurSize = 0.f;
	m_fOldDOFFarBlurSize = 0.f;	

	memset( m_bSlotDeleteWait, 0, sizeof(m_bSlotDeleteWait) );
	memset( m_SlotDelteWaitRemainTime, 0, sizeof(m_SlotDelteWaitRemainTime) );

	m_bRequestWait = false;
	memset( m_bSlotOpen, 0, sizeof(m_bSlotOpen) );
	m_nTotalCharSelectPage = 0;
	m_nCurrentCharSelectPage = 1;
	m_nTotalCharSlotCount = 0;
	m_nEmptyCharSlotCount = 0;
	m_nDeleteCharPage = 0;
#ifdef PRE_ADD_RELOGIN_PROCESS
	m_nReLoginCount = 0;
#endif // PRE_ADD_RELOGIN_PROCESS
}

CDnLoginTask::~CDnLoginTask()
{
#ifdef PRE_PLAYER_PRELOAD
	while( !CDnMainFrame::GetInstance().IsFinishPreloadPlayer() ) { Sleep(1); }
#endif
	GetInterface().Finalize( CDnInterface::Login );

	SAFE_RELEASE_SPTRVEC( m_vecOutlineCreate );
	SAFE_RELEASE_SPTRVEC( m_vecOutlineSlot );

	SAFE_RELEASE_SPTR( m_hCamera );
	m_vecCharListData.clear();
	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ )
	{
		SAFE_RELEASE_SPTR( m_hSlotActorHandle[i] );
	}
	SecureZeroMemory( m_nSlotActorMapID, sizeof(m_nSlotActorMapID) );
	SAFE_RELEASE_SPTRVEC( m_hVecCreateActorList );
	
	for( DWORD i=0; i<m_VecCreateDefaultPartsList.size(); i++ ) {
		for( int j=0; j<CDnParts::PartsTypeEnum_Amount; j++ ) {
			SAFE_RELEASE_SPTRVEC( m_VecCreateDefaultPartsList[i].hVecParts[j] );
			SAFE_DELETE_VEC( m_VecCreateDefaultPartsList[i].nVecList[j] );
		}
		SAFE_RELEASE_SPTR( m_VecCreateDefaultPartsList[i].hWeapon[0] );
		SAFE_RELEASE_SPTR( m_VecCreateDefaultPartsList[i].hWeapon[1] );
	}
	SAFE_DELETE_VEC( m_VecCreateDefaultPartsList );

	CDnActor::ReleaseClass();
	CDnEtcObject::ReleaseClass();
	CDnWeapon::ReleaseClass();
	if( m_pWorld ) m_pWorld->Destroy();

	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
	CEtResource::FlushWaitDelete();
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	if( GetEtEngine() ) GetEtEngine()->FlushBuffer();
#endif
	if( CEtFontMng::IsActive() )
		CEtFontMng::GetInstance().FlushFontCache();
}

void CDnLoginTask::PreInitialize()
{
	// Create camera
	CDnAniCamera *pCamera = new CDnAniCamera;
	m_hCamera = pCamera->GetMySmartPtr();
	pCamera->AddCameraData( "Login_Camera_01.cam" );
	pCamera->AddCameraData( "Select_Stay.cam" );
	pCamera->AddCameraData( "Select_Start.cam" );
	pCamera->AddCameraData( "Select_Start_Stay.cam" );
	pCamera->AddCameraData( "Select_End.cam" );
	pCamera->AddCameraData( "Select_End_Wa.cam" );
	pCamera->AddCameraData( "Select_End_Ar.cam" );
	pCamera->AddCameraData( "Select_End_Cl.cam" );
	pCamera->AddCameraData( "Select_End_So.cam" );
	pCamera->AddCameraData( "Wa_Select.cam" );
	pCamera->AddCameraData( "Wa_Select_Stay.cam" );
	pCamera->AddCameraData( "Wa_UnSelect.cam" );
	pCamera->AddCameraData( "Ar_Select.cam" );
	pCamera->AddCameraData( "Ar_Select_Stay.cam" );
	pCamera->AddCameraData( "Ar_UnSelect.cam" );
	pCamera->AddCameraData( "Cl_Select.cam" );
	pCamera->AddCameraData( "Cl_Select_Stay.cam" );
	pCamera->AddCameraData( "Cl_UnSelect.cam" );
	pCamera->AddCameraData( "So_Select.cam" );
	pCamera->AddCameraData( "So_Select_Stay.cam" );
	pCamera->AddCameraData( "So_UnSelect.cam" );
#ifdef PRE_ADD_ACADEMIC
#ifndef PRE_REMOVE_ACADEMIC
	pCamera->AddCameraData( "Ac_Select.cam" );
	pCamera->AddCameraData( "Ac_Select_Stay.cam" );
	pCamera->AddCameraData( "Ac_UnSelect.cam" );
#endif // #ifndef PRE_REMOVE_ACADEMIC
#endif // #ifdef PRE_ADD_ACADEMIC
#ifdef PRE_ADD_KALI
#ifndef PRE_REMOVE_KALI
	pCamera->AddCameraData( "Ka_Select.cam" );
	pCamera->AddCameraData( "Ka_Select_Stay.cam" );
	pCamera->AddCameraData( "Ka_UnSelect.cam" );
#endif // #ifdef PRE_REMOVE_KALI
#endif // #ifdef PRE_ADD_KALI

	pCamera->AddCameraData( "Wa_Turn.cam" );
	pCamera->AddCameraData( "Ar_Turn.cam" );
	pCamera->AddCameraData( "Cl_Turn.cam" );
	pCamera->AddCameraData( "So_Turn.cam" );
#ifdef PRE_ADD_ACADEMIC
#ifndef PRE_REMOVE_ACADEMIC
	pCamera->AddCameraData( "Ac_Turn.cam" );
#endif // #ifndef PRE_REMOVE_ACADEMIC
#endif // #ifdef PRE_ADD_ACADEMIC
#ifdef PRE_ADD_KALI
#ifndef PRE_REMOVE_KALI
	pCamera->AddCameraData( "Ka_Turn.cam" );
#endif // #ifdndef PRE_REMOVE_KALI
#endif // #ifdef PRE_ADD_KALI

	CDnTitleTask *pTitleTask = (CDnTitleTask *)CTaskManager::GetInstance().GetTask( "TitleTask" );
	if( pTitleTask ) pTitleTask->CheckPointHoundLoading( 77 );
	if( pTitleTask ) pTitleTask->ForceAddHoundLoading( 3, 2000 );

#ifndef _FINAL_BUILD
	if( CGlobalValue::GetInstance().IsPermitLogin() )
	{
	}
	else 
#endif //_FINAL_BUILD
	{
		// Load background
		DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
		int nMapIndex = 50;
		char szLabel[64];
		std::vector<std::string> szVecToolMapName;
		std::string szTempStr;
		//std::vector<std::string> szVecWeatherName[CDnWorld::WeatherEnum_Amount];

		for( int i=0; i<10; i++ ) {
			sprintf_s( szLabel, "_ToolName%d", i + 1 );
			szTempStr = pMapSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetString();
			if( szTempStr.empty() ) continue;

			szVecToolMapName.push_back( szTempStr );
		}

		m_pWorld = &CDnWorld::GetInstance();
		bool bResult = m_pWorld->Initialize( CEtResourceMng::GetInstance().GetFullPath( "MapData" ).c_str(), szVecToolMapName[_rand()%szVecToolMapName.size()].c_str(), this, true );
		if( bResult == false ) return;

		/*for( int j=0; j<CDnWorld::WeatherEnum_Amount; j++ ) {
			for( int k=0; k<5; k++ ) {
				sprintf_s( szLabel, "_Envi%d_%d", j+1, k+1 );
				szTempStr = pMapSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetString();
				if( szTempStr.empty() ) continue;
				szVecWeatherName[j].push_back( szTempStr );
			}
		}

		m_pWorld->SetCurrentWeather( (CDnWorld::WeatherEnum)(_rand()%CDnWorld::WeatherEnum_Amount) );
		CDnWorld::WeatherEnum Weather = m_pWorld->GetNextWeather( nMapIndex );
		std::string szEnviName;
		int nWeatherRandomCount = (int)szVecWeatherName[Weather].size();
		if( nWeatherRandomCount > 0 ) szEnviName = szVecWeatherName[Weather][_rand()%nWeatherRandomCount];*/
		
		m_pWorld->SetCurrentWeather( (CDnWorld::WeatherEnum)(_rand()%CDnWorld::WeatherEnum_Amount) );
		CDnWorld::WeatherEnum Weather = m_pWorld->GetNextWeather( nMapIndex );
		std::string szEnviName = CDnWorld::GetEnviName( nMapIndex, 0, Weather );

		m_pWorld->InitializeEnviroment( CEtResourceMng::GetInstance().GetFullName( szEnviName ).c_str(), m_hCamera );
		m_hCamera->DeActivate();		

		if( pTitleTask ) pTitleTask->CheckPointHoundLoading( 82 );

		CDnActor::InitializeClass();
		CDnEtcObject::InitializeClass();
		CalcCharSlotPosition();

		if( pTitleTask ) pTitleTask->CheckPointHoundLoading( 85 );
		if( pTitleTask ) pTitleTask->ForceAddHoundLoading( 10, 5000 );

		// Preload resources needed for character creation.
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDEFAULTCREATE );
		DNTableFileFormat* pActorSOX = GetDNTable( CDnTableDB::TACTOR );
		int nActorCount = pSox->GetItemCount();
		DnActorHandle hActor;
		char *szLabelList[CDnParts::PartsTypeEnum_Amount] = { "_Face", "_Hair", "_Helmet", "_Body", "_Leg", "_Hand", "_Foot", NULL, NULL, NULL, NULL };
		char szTemp[128];

		for( int i=1; i<=nActorCount; i++ ) {
			int nClassID = pSox->GetFieldFromLablePtr( i, "_ClassID" )->GetInteger();
			hActor = CreateActor( nClassID, false, true );
			if( !hActor )
			{
				_ASSERT( "LoginTask::PreInitialize(), CreateActor() failed!!" );
				continue;
			}

			((CDnPlayerActor*)hActor.GetPointer())->SetJobHistory( nClassID );
			hActor->ShowHeadName( false );
			if( hActor->GetObjectHandle() && hActor->GetObjectHandle()->GetSkinInstance() ) {
				hActor->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
			}

			MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(hActor.GetPointer());

			char *pStaticName = pActorSOX->GetFieldFromLablePtr( nClassID, "_StaticName" )->GetString();
#ifdef PRE_FIX_MEMOPT_EXT
			char* szDummySkinName = CommonUtil::GetFileNameFromFileEXT(pActorSOX, nClassID, "_SkinName");
#else
			char *szDummySkinName = pActorSOX->GetFieldFromLablePtr( nClassID, "_SkinName" )->GetString();
#endif

			sprintf_s( szTemp, "%s\\%s_login.ani", pStaticName, pStaticName );
			pPartsBody->LoadSkin( CEtResourceMng::GetInstance().GetFullName( szDummySkinName ).c_str(), CEtResourceMng::GetInstance().GetFullName( szTemp ).c_str() );
			sprintf_s( szTemp, "%s\\%s_login.act", pStaticName, pStaticName );
			hActor->LoadAction( CEtResourceMng::GetInstance().GetFullName( szTemp ).c_str() );
			hActor->Initialize();

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
			((CDnPlayerActor*)hActor.GetPointer())->RecomposeAction();
#endif

			hActor->GetObjectHandle()->SetCalcPositionFlag( CALC_POSITION_X|CALC_POSITION_Y|CALC_POSITION_Z );
			MatrixEx MatExWorld;
			MatExWorld = m_TreeOffset;

			hActor->SetPosition( MatExWorld.m_vPosition );
			hActor->Look( EtVec3toVec2( MatExWorld.m_vZAxis ) );
			hActor->SetLevel(1);
			hActor->SetHP( hActor->GetMaxHP() );

			DefaultPartsStruct Struct;
			for( int k=0; k<CDnParts::PartsTypeEnum_Amount; k++ ) {
				if( szLabelList[k] == NULL ) continue;
				for( int j=0; j<10; j++ ) {
					sprintf_s( szLabel, "%s%d", szLabelList[k], j + 1 );
					int nTableID = pSox->GetFieldFromLablePtr( i, szLabel )->GetInteger();
					if( nTableID <= 0 ) continue;

					Struct.nVecList[k].push_back( nTableID );
					Struct.hVecParts[k].push_back( CDnParts::CreateParts( nTableID, 0 ) );
				}
				Struct.nSelectPartsIndex[k] = ( Struct.nVecList[k].empty() ) ? -1 : 0;
			}
			Struct.dwColor[MAPartsBody::HairColor] = D3DCOLOR_XRGB(255,255,255);
			Struct.dwColor[MAPartsBody::SkinColor] = D3DCOLOR_ARGB(255,128,128,128);
			Struct.dwColor[MAPartsBody::EyeColor] = D3DCOLOR_XRGB(255,255,255);

			hActor->SetActionQueue( "CreateSelect_Cancle_Idle" );

			for( int j=0; j<2; j++ ) {
				sprintf_s( szLabel, "_Weapon%d", j + 1 );
				int nWeaponIndex = pSox->GetFieldFromLablePtr( i, szLabel )->GetInteger();
				if( nWeaponIndex != -1 ) {
					Struct.hWeapon[j] = CDnWeapon::CreateWeapon( nWeaponIndex, 0 );
					((CDnPlayerActor*)hActor.GetPointer())->CDnPlayerActor::AttachWeapon( Struct.hWeapon[j], j, false );
				}
				Struct.nWeapon[j] = nWeaponIndex;
			}

			m_VecCreateDefaultPartsList.push_back( Struct );
			m_hVecCreateActorList.push_back( hActor );
		}

		if( pTitleTask ) pTitleTask->CheckPointHoundLoading( 99 );

		ShowCreateCharacter( false );
	}

	// Note : Interface Initialize
	GetInterface().Initialize( CDnInterface::Login, this );
}

bool CDnLoginTask::Initialize( LoginStateEnum State )
{
	if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );
	CDnPlayerActor::EnableAnotherPlayers( false );

	CDnMouseCursor::GetInstance().ShowCursor( true );
	
	CDnAniCamera *pCamera = (CDnAniCamera *)m_hCamera.GetPointer();
	if( State == LoginStateEnum::Logo )
		pCamera->PlayCamera( "Login_Camera_01", NULL );
	else {
		pCamera->PlayCamera( "Select_Stay", NULL );

		ReduceFarDOFBlurSize();
	}

	CDnCamera::SetActiveCamera( m_hCamera );
	if( CGlobalInfo::GetInstance().m_szIP.empty() ) {
		CGlobalInfo::GetInstance().m_szIP = "192.168.0.21";
		CGlobalInfo::GetInstance().m_szPort = "14300";
	}
	/*
	CGlobalInfo::GetInstance().CalcLoginConnectInfo();
	int nConnectTrialCount = 1 + ( CGlobalInfo::GetInstance().GetLoginConnectInfoCount() * 2 );
	bool bConnectSuccess = false;
	int nOffset = _rand()%CGlobalInfo::GetInstance().GetLoginConnectInfoCount();
	for( int i=0; i<nConnectTrialCount; i++ ) {
		std::string szIP = CGlobalInfo::GetInstance().GetLoginConnectInfo( nOffset )->szIP;
		short nPort = CGlobalInfo::GetInstance().GetLoginConnectInfo( nOffset )->nPort;

		if( CClientSessionManager::GetInstance().ConnectLoginServer( szIP.c_str(), nPort, State == CharSelect ? false : true ) == true ) {
			bConnectSuccess = true;
			break;
		}
else Sleep(300); // Add a slight delay.
		nOffset++;
		if( nOffset >= CGlobalInfo::GetInstance().GetLoginConnectInfoCount() ) nOffset = 0;
	}
	*/
	CGlobalInfo::GetInstance().CalcLoginConnectInfo();
	std::vector<std::string> szVecIP;
	std::vector<USHORT> nVecPort;
	for( DWORD i=0; i<CGlobalInfo::GetInstance().GetLoginConnectInfoCount(); i++ ) {
		szVecIP.push_back( CGlobalInfo::GetInstance().GetLoginConnectInfo(i)->szIP );
		nVecPort.push_back( CGlobalInfo::GetInstance().GetLoginConnectInfo(i)->nPort );
	}

	if( CClientSessionManager::GetInstance().ConnectLoginServer( szVecIP, nVecPort, State == CharSelect ? false : true ) == false ) {
		GetInterface().MessageBox( MESSAGEBOX_9, MB_OK, UICallbackEnum::ConnectLoginFailed, this, true, false, false, true );
	}

	if( m_pWorld ) m_pWorld->PlayBGM();

	DebugLog("Login state  %d",State);

	switch( State ) {
		case CharSelect:
			ChangeState( State, false );
			m_PrevState = StartGame;
			m_bProcessCamera = false;
			CClientSessionManager::GetInstance().DisConnectServer(true);
			break;
		default:
			m_State = State;
			m_PrevState = State;

			std::string strCmd;
			strCmd = szVersion;
			ToLowerA(strCmd);

			std::vector<std::string> tokens;
			TokenizeA(strCmd, tokens, ".");

			int nMajorVersion = atoi(tokens[0].c_str());
			int nMinorVersion = atoi(tokens[1].c_str());

			SendCheckVersion( 6, 6, false, nMajorVersion, nMinorVersion );

			DebugLog("SendCheckVersion OK");
#ifndef _FINAL_BUILD
			if( CGlobalValue::GetInstance().IsPermitLogin() ) {
				ChangeState( RequestIDPass );
				return true;
			}
#endif //_FINAL_BUILD
			GetInterface().FadeDialog( 0xFFFFFFFF, 0x00FFFFFF, 5.f, this, true );
			break;
	}

	return true;
}

void CDnLoginTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	UpdateHang();
	if( m_bDestroyTask ) return;

	// Mouse Update
//	CDnMouseCursor::GetInstance().UpdateCursor();

	// Process Input Device
	CInputDevice::GetInstance().Process( LocalTime, fDelta );

	CDnCamera::ProcessClass( LocalTime, fDelta );
	CDnActor::ProcessClass( LocalTime, fDelta );
	CDnEtcObject::ProcessClass( LocalTime, fDelta );

	// Process Sound
	if( CEtSoundEngine::IsActive() ) {
		CEtSoundEngine::GetInstance().Process( fDelta );

		DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
		if( hCamera ) 
			CEtSoundEngine::GetInstance().SetListener( hCamera->GetMatEx()->m_vPosition, hCamera->GetMatEx()->m_vZAxis, hCamera->GetMatEx()->m_vYAxis );
	}

	// Process World
	if( m_pWorld ) {
		m_pWorld->Process( LocalTime, fDelta );
	}

	if( m_bProcessCamera ) {
		CDnAniCamera *pCamera = (CDnAniCamera *)m_hCamera.GetPointer();
		if( strstr( pCamera->GetCurrentAction(), "_Stay" ) ) {
			m_bProcessCamera = false;
			GetInterface().ChangeState( m_State );
			switch( m_State ) {
				case LoginStateEnum::CharSelect:
					{
						for( int i=0; i<4; i++ ) {
							ShowSlotInfoDlg( i );
						}
					}
					break;
			}
		}

		if( !m_bShowHideActor ) {
			switch( m_State ) {
				case CharSelect:
					if( pCamera->GetCurFrame() > 60.f ) {
						ShowCreateCharacter( false );
						m_bShowHideActor = true;
					}
					break;
				case CharCreate_SelectClass:
					if( strcmp( pCamera->GetCurrentAction(), "Select_Start" ) == NULL ) {
						if( pCamera->GetCurFrame() > 50.f ) {
							SelectActorSlot( -1 );
							//for( int i=0; i<CHARCOUNTMAX; i++ ) 
							//{
							//	SAFE_RELEASE_SPTR( m_hSlotActorHandle[i] );
							//}
							//m_nSelectClassIndex = -1;
							ShowCreateCharacter( true );

							m_bShowHideActor = true;
						}
					}
					break;
			}
		}
	}

	// Keep setting position because ghosts have an issue where their position gradually drifts.
	// Originally only done in ProcessDeleteWait, but it kept creeping up even during long idle waits on the character creation screen..
	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ ) {
		if( !m_bSlotDeleteWait[i] ) continue;
		if( m_hSlotActorHandle[i] ) 
			m_hSlotActorHandle[i]->SetPosition( m_vSlotActorPos[i] );
	}

	switch( m_State ) {
		case CharSelect:
			{
				if( m_CharList.nRet == ERROR_NONE ) {
					_OnRecvLoginCharList( &m_CharList );
					m_CharList.nRet = ERROR_LOGIN_FAIL;
				}
				ProcessDeleteWait( LocalTime, fDelta );
			}
			break;
		case CharCreate_ChangeParts:
			{
				CDnAniCamera *pCamera = (CDnAniCamera *)m_hCamera.GetPointer();
				if( strstr( pCamera->GetCurrentAction(), "Turn" ) ) {
					GetInterface().EnableCharCreateBackDlgControl( false );
				}
				else GetInterface().EnableCharCreateBackDlgControl( true );
			}
			break;
	}

#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	// Process Interface
	GetInterface().Process( LocalTime, fDelta );
#endif

#ifndef _FINAL_BUILD
	#ifdef PRE_FIX_SHOWVER
	std::string szVersion = CGlobalInfo::GetInstance().MakeVersionString();
	#else
	std::string szVersion = CGlobalValue::GetInstance().MakeVersionString();
	#endif
	EtVector2 vPos = EtVector2( ( 1.f / CEtDevice::GetInstance().Width() ) * 10.f, ( 1.f / CEtDevice::GetInstance().Height() ) * ( CEtDevice::GetInstance().Height() - 20.f ) );
	EternityEngine::DrawText2D( vPos, szVersion.c_str(), 0xff000000 );
	vPos = EtVector2( ( 1.f / CEtDevice::GetInstance().Width() ) * 9.f, ( 1.f / CEtDevice::GetInstance().Height() ) * ( CEtDevice::GetInstance().Height() - 21.f ) );
	EternityEngine::DrawText2D( vPos, szVersion.c_str(), 0xffffffff );
#endif //_FINAL_BUILD

	// Note : UI Process
	EtInterface::Process( fDelta );

	EternityEngine::ClearScreen( 0xFFFFFFFF, 1.0f, 0, false );
	EternityEngine::RenderFrame( fDelta );
	EtInterface::Render( fDelta );
	EternityEngine::ShowFrame( NULL );
}

void CDnLoginTask::ProcessDeleteWait( LOCAL_TIME LocalTime, float fDelta )
{
	__time64_t CurTime;
	time( &CurTime );

	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ ) {
		if( !m_bSlotDeleteWait[i] ) continue;
		/*
		if( m_hSlotActorHandle[i] ) 
			m_hSlotActorHandle[i]->SetPosition( m_vSlotActorPos[i] );
		*/

		tm ExpectTime = *localtime( &m_SlotDelteWaitRemainTime[i] );
		tm CurrentTime = *localtime( &CurTime );

		__int64 nExpectTime = ( GetDateValue( 1900 + ExpectTime.tm_year, ExpectTime.tm_mon + 1, ExpectTime.tm_mday ) * 24 * 60 ) + ( ExpectTime.tm_hour * 60 ) + ExpectTime.tm_min;
		__int64 nCurrentTime = ( GetDateValue( 1900 + CurrentTime.tm_year, CurrentTime.tm_mon + 1, CurrentTime.tm_mday ) * 24 * 60 ) + ( CurrentTime.tm_hour * 60 ) + CurrentTime.tm_min;
		__int64 nRemainTime = nExpectTime - nCurrentTime;
		if( nRemainTime <= 0 ) {
			SAFE_RELEASE_SPTR( m_hSlotActorHandle[i] );
			GetInterface().ShowSlotDeleteWaitDlg( false, i, 0 );
			m_bSlotDeleteWait[i] = false;
		}
	}
}

void CDnLoginTask::ChangeState( LoginStateEnum State, bool bAutoSendMessage )
{
	WriteLog( 1, ", Info, CDnLoginTask::ChangeState : Prev(%d), Cur(%d)\n", m_State, State );
	m_PrevState = m_State;
	m_State = State;

	if( bAutoSendMessage ) {
#ifndef _FINAL_BUILD
		if( CGlobalValue::GetInstance().IsPermitLogin() )
		{
			switch( m_State ) 
			{
			case LoginStateEnum::RequestIDPass:
				{
#ifdef PRE_ADD_RELOGIN_PROCESS
					m_nReLoginCount = 0;
#endif // PRE_ADD_RELOGIN_PROCESS
					RequestLogin( CGlobalValue::GetInstance().m_szID.c_str(), CGlobalValue::GetInstance().m_szPass.c_str() );
				}
				return;
			case LoginStateEnum::ServerList:
				SendServerList();
				return;
			case LoginStateEnum::CharSelect:
				{
					CDnBridgeTask::GetInstance().SetCurrentCharLevel( 1 );
					bool isIgnoreAuthNotify = false;
#if defined(PRE_ADD_23829)
					// User option for showing 2nd password window [2010/12/07 semozz]
					CDnAuthTask *pAuthTask = (CDnAuthTask *)CTaskManager::GetInstance().GetTask("AuthTask");
					isIgnoreAuthNotify = pAuthTask ? pAuthTask->GetIgnore2ndPassNotify() : false;
#endif // PRE_ADD_23829
					int nSeed = 1;
					int nValue[4] = { 0, };
					if( !CGlobalValue::GetInstance().m_szSecondPass.empty() ) {
						CMtRandom Random;

						std::vector<std::string> tokens;
						TokenizeA( std::string(SecondPassword::GetVersion()), tokens, "." );

						int nKey = atoi(tokens[0].c_str()) + atoi(tokens[1].c_str());
						if( nKey % 2 ) nKey = -nKey;

						int nTempSeed = nSeed + nKey;
						Random.srand( nTempSeed );
						std::vector<int> nVecList, nVecList2;
						int nArray[10] = { 7, 8, 9, 4, 5, 6, 1, 2, 3, 0 };
						for( int i=0; i<10; i++ ) nVecList2.push_back(nArray[i]);

						while( !nVecList2.empty() ) {
							int nOffset = Random.rand( (int)nVecList2.size() );
							nVecList.push_back( nVecList2[nOffset] );
							nVecList2.erase( nVecList2.begin() + nOffset );
						}
						int nNum[4];
						int nTemp = _wtoi( CGlobalValue::GetInstance().m_szSecondPass.c_str() );
						nNum[0] = nTemp / 1000;
						nNum[1] = (nTemp%1000) / 100;
						nNum[2] = (nTemp%100) / 10;
						nNum[3] = nTemp%10;
						for( int i=0; i<4; i++ ) {
							for( int j=0; j<10; j++ ) {
								if( nVecList[j] == nNum[i] ) {
									nValue[i] = j;
									break;
								}
							}
						}

					}
					SendSelectChar( (BYTE)CGlobalValue::GetInstance().m_nLocalPlayerClassID, nSeed, nValue, isIgnoreAuthNotify );
				}
				return;
			case LoginStateEnum::ChannelList:
				SendSelectedChannel( m_nSelectedChannelIndex, false );
				m_State = LoginStateEnum::StartGame;
				break;
			}
		}
		else
	#endif
		{
			switch( m_State ) 
			{
			case LoginStateEnum::ServerList:
				{					
					SendServerList();
					for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ )
					{
						if( m_hSlotActorHandle[i] )
							m_hSlotActorHandle[i]->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( -1, -1, true );
						SAFE_RELEASE_SPTR( m_hSlotActorHandle[i] );
					}
					memset( m_bSlotDeleteWait, 0, sizeof(m_bSlotDeleteWait) );
					memset( m_SlotDelteWaitRemainTime, 0, sizeof(m_SlotDelteWaitRemainTime) );
					memset( m_bSlotOpen, 0, sizeof(m_bSlotOpen) );
				}
				break;
			case LoginStateEnum::CharSelect:
				{
					ReduceFarDOFBlurSize();
					m_bCompleteCharList = false;

					if( m_PrevState == LoginStateEnum::CharCreate_SelectClass ) ((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( "Select_End", "Select_Stay" );
					else if( m_PrevState == LoginStateEnum::CharCreate_ChangeParts ) {

						// Since Academic also exists if Kali exists.. define it like this.
#if defined(PRE_ADD_KALI) && !defined(PRE_REMOVE_KALI)
						const char* szCameraName[] = { "Select_End_Wa", "Select_End_Ar", "Select_End_So", "Select_End_Cl", "Select_End_Cl", "Select_End_Cl" }; 
#elif defined(PRE_ADD_ACADEMIC) && !defined(PRE_REMOVE_ACADEMIC) // Academic is currently using Select_End_CL as well. 
						const char *szCameraName[] = { "Select_End_Wa", "Select_End_Ar", "Select_End_So", "Select_End_Cl", "Select_End_Cl" };
#else
						const char *szCameraName[] = { "Select_End_Wa", "Select_End_Ar", "Select_End_So", "Select_End_Cl" };
#endif // #if defined(PRE_ADD_ACADEMIC) && !defined(PRE_REMOVE_ACADEMIC)
						((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( szCameraName[m_nSelectClassIndex], "Select_Stay" );
					}
					else ((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( "Select_Stay", NULL );

					//SendSelectedServer( (BYTE)m_nSelectedServerIndex );
				}
				break;
			case LoginStateEnum::ChannelList:
				{
					int nSelectCharIndex = 0;
					nSelectCharIndex = GetSelectExtendActorSlot();
					bool isIgnoreAuthNotify = false;
#if defined(PRE_ADD_23829)
					// User option for showing 2nd password window [2010/12/07 semozz]
					CDnAuthTask *pAuthTask = (CDnAuthTask *)CTaskManager::GetInstance().GetTask("AuthTask");
					isIgnoreAuthNotify = pAuthTask ? pAuthTask->GetIgnore2ndPassNotify() : false;
#endif // PRE_ADD_23829
					SendSelectChar( (BYTE)nSelectCharIndex, m_nAuthPasswordSeed, m_nAuthPasswordValue, isIgnoreAuthNotify );
				}
				break;
			case LoginStateEnum::StartGame:
					g_pServiceSetup->WriteStageLog_( IServiceSetup::JoinChannel , _T("StartGmae"));	
					SendSelectedChannel( m_nSelectedChannelIndex, false );
					GetInterface().FadeDialog( 0x00000000, 0x00000000, 0.f, this, true );
				break;
			case LoginStateEnum::RequestIDPass:
				{
					RestoreFarDOFBlurSize();
#ifdef PRE_ADD_RELOGIN_PROCESS
					m_nReLoginCount = 0;
#endif // PRE_ADD_RELOGIN_PROCESS
					if( strcmp( ((CDnAniCamera*)m_hCamera.GetPointer())->GetCurrentAction(), "Login_Camera_01" ) ) 
						((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( "Login_Camera_01", NULL );
					for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ )
					{
						SAFE_RELEASE_SPTR( m_hSlotActorHandle[i] );
					}
					memset( m_bSlotDeleteWait, 0, sizeof(m_bSlotDeleteWait) );
					memset( m_SlotDelteWaitRemainTime, 0, sizeof(m_SlotDelteWaitRemainTime) );
					memset( m_bSlotOpen, 0, sizeof(m_bSlotOpen) );
				}
				break;
			case LoginStateEnum::CharCreate_SelectClass:
				{
					if( m_PrevState == LoginStateEnum::CharCreate_ChangeParts ) {
						// Since Academic also exists if Kali exists.. define it like this.
#if defined(PRE_ADD_KALI) && !defined(PRE_REMOVE_KALI)
						const char *szCameraName[] = { "Select_End", "Wa_UnSelect", "Ar_UnSelect", "So_UnSelect", "Cl_UnSelect", "Ac_UnSelect", "Ka_UnSelect" };
#elif defined(PRE_ADD_ACADEMIC) && !defined(PRE_REMOVE_ACADEMIC)
						const char *szCameraName[] = { "Select_End", "Wa_UnSelect", "Ar_UnSelect", "So_UnSelect", "Cl_UnSelect", "Ac_UnSelect" };
#else
						const char *szCameraName[] = { "Select_End", "Wa_UnSelect", "Ar_UnSelect", "So_UnSelect", "Cl_UnSelect" };
#endif // #if defined(PRE_ADD_ACADEMIC) && !defined(PRE_REMOVE_ACADEMIC)
						((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( szCameraName[m_nSelectClassIndex+1], "Select_Start_Stay" );
					}
					else ((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( "Select_Start", "Select_Start_Stay" );
					/*
					for( int i=0; i<CHARCOUNTMAX; i++ ) 
					{
						SAFE_RELEASE_SPTR( m_hSlotActorHandle[i] );
					}
					*/
				}
				break;
			case LoginStateEnum::CharCreate_ChangeParts:
				break;
			}
		}
	}

	switch( m_State ) {
		case LoginStateEnum::CharSelect:
			{
				// Block Back on channel list because if the UI shows early, double-clicking becomes possible.
				if( m_PrevState == ChannelList ) {
					for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ )
					{
						SAFE_RELEASE_SPTR( m_hSlotActorHandle[i] );
					}
				}
				SelectActorSlot( -1 );
			}
			break;
		case LoginStateEnum::CharCreate_SelectClass:
			{
				/*
				if( PrevState == LoginStateEnum::CharSelect ) {
					SelectActorSlot( -1 );
					m_nSelectClassIndex = -1;
					ShowCreateCharacter( true );
				}
				*/
				SAFE_RELEASE_SPTRVEC( m_vecOutlineCreate );			
				CEtOutlineFilter *pOutlineFilter = GetOutlineFilter();
				if( pOutlineFilter ) {
					int i, nSize;
					nSize = (int)m_hVecCreateActorList.size();
					for( i = 0; i < nSize; i++) {
						EtOutlineHandle hOutline = CEtOutlineObject::Create( m_hVecCreateActorList[i]->GetObjectHandle() );
						static EtColor vColor(0.8f, 1.0f, 0.3f, 1.0f);
						hOutline->SetColor( vColor );
						hOutline->SetWidth( 0.8f );
						m_vecOutlineCreate.push_back( hOutline );
					}
				}

				ChangeCreateClass( -1 );
			}
			break;
		case LoginStateEnum::CharCreate_ChangeParts:
			break;
		case LoginStateEnum::StartGame:
			{
			}
			break;
	}

	switch( m_State ) {
		case RequestIDPass:
		case ServerList:
		case ChannelList:
			{
				GetInterface().ChangeState( m_State );
			}
			break;
		case CharSelect:
			GetInterface().ChangeState( m_State, true );
			// Block setting on Back from channel list because Process immediately calls ChangeState on Interface, showing UI before the packet arrives.
			if( m_PrevState != ChannelList ) m_bProcessCamera = true;
			break;
		default:
			GetInterface().ChangeState( m_State, true );
			m_bProcessCamera = true;
			break;
	}
	m_bShowHideActor = false;
}

void CDnLoginTask::ShowCreateCharacter( bool bShow )
{
	for( DWORD i=0; i<m_hVecCreateActorList.size(); i++ ) {
		m_hVecCreateActorList[i]->Show( bShow );
	}
	if( bShow ) {
		for( DWORD i=0; i<m_VecCreateDefaultPartsList.size(); i++ ) {
			m_hVecCreateActorList[i]->SetAction( "CreateSelect_Cancle_Idle", 0.f, 0.f );

			MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[i].GetPointer());
			for( DWORD j=0; j<EQUIPMAX; j++ ) {
				if( m_VecCreateDefaultPartsList[i].nVecList[j].empty() ) continue;
				m_VecCreateDefaultPartsList[i].nSelectPartsIndex[j] = _rand()%(int)m_VecCreateDefaultPartsList[i].nVecList[j].size();

				int nTableID = m_VecCreateDefaultPartsList[i].nVecList[j][m_VecCreateDefaultPartsList[i].nSelectPartsIndex[j]];

				pPartsBody->DetachParts( (CDnParts::PartsTypeEnum)j );
				DnPartsHandle hParts = m_VecCreateDefaultPartsList[i].hVecParts[j][m_VecCreateDefaultPartsList[i].nSelectPartsIndex[j]];
				pPartsBody->AttachParts( hParts, (CDnParts::PartsTypeEnum)-1, false, false );
			}
			
			int nSelectHairColorIndex = _rand() % HAIR_COLOR_COUNT;
			SetCharColor( MAPartsBody::HairColor, nSelectHairColorIndex, i );

			int nSelectSkinColorIndex = 0;//_rand() % SKIN_COLOR_COUNT; // Let's use the default skin color...
			SetCharColor( MAPartsBody::SkinColor, nSelectSkinColorIndex, i );

			int nSelectEyeColorIndex = _rand() % EYE_COLOR_COUNT;
			SetCharColor( MAPartsBody::EyeColor, nSelectEyeColorIndex, i );

			m_hVecCreateActorList[i]->SetFaceAction( "Flicker" );

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
			if( pPartsBody && pPartsBody->GetCombineObject() && pPartsBody->GetCombineObject()->IsShow() ) pPartsBody->GetCombineObject()->ShowObject( false );
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
		}
	}
}

void CDnLoginTask::OnConnectTcp()
{
//	if( m_bDestroyTask ) return;
	CLoginClientSession::OnConnectTcp();
}

void CDnLoginTask::OnDisconnectTcp( bool bValidDisconnect )
{
	if( !bValidDisconnect ) {
		WriteLog( 0, ", Error, Disconnect Login Server\n" );
#if defined(_FILELOG_PACKET) || defined(_FILELOG_SPOS)
		g_bInvalidDisconnect = true;
#endif
		GetInterface().MessageBox( MESSAGEBOX_10, MB_OK, UICallbackEnum::DisconnectTCP, this, true, false, false, true );


	}
}

void CDnLoginTask::OnRecvLoginCheckVersion( SCCheckVersion *pPacket )
{
	FUNC_LOG();
	
	if (pPacket->nRet == ERROR_NONE) {
		g_pServiceSetup->Initialize(NULL);
	}
	else
	{
		//CClientSessionManager::GetInstance().DisconnectLoginServer( false );
		CClientSessionManager::GetInstance().DisConnectServer(false);
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::CheckVersionFailed, this, true, false, false, true );
	}
}

void CDnLoginTask::OnRecvLoginCheckLogin( SCCheckLogin *pPacket )
{
	FUNC_LOG();
	CSyncTimer::GetInstance().SetServerTime( pPacket->_tServerTime );
	if (pPacket->nRet == ERROR_NONE) {
		ChangeState( ServerList );
	}
	else if ((pPacket->nRet == ERROR_LOGIN_FCM_PROTECTED) || (pPacket->nRet == ERROR_LOGIN_FCM_LIMITED))
	{
		ChangeState( ServerList );
		CDnMessageBoxTextBox* pDnMessageBoxTextBox = GetInterface().GetMessageBoxTextBox();
		if( pDnMessageBoxTextBox )
		{
			WCHAR sep;
			sep = 0xff00;
			pDnMessageBoxTextBox->AddMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106101 ) );
			pDnMessageBoxTextBox->AddMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106102 ) );
			pDnMessageBoxTextBox->AddMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106103 ) );
			std::wstring message;

			EtInterface::GetNameLinkMgr().AppendTag(message, CEtUINameLinkMgr::eNLTAG_START);
			message += FormatW( _T("%cl%cc%08x%s"), sep, sep, D3DCOLOR_ARGB(0xFF,0xFF,0xFF,0x00), GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106104 ) );
			EtInterface::GetNameLinkMgr().AppendTag(message, CEtUINameLinkMgr::eNLTAG_END);

			pDnMessageBoxTextBox->AddMessage( message.c_str() );
			pDnMessageBoxTextBox->AddMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 106105 ) );
		}

		GetInterface().MessageBoxTextBox( L"" );
//		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::CheckLoginFailed, this);
	}
	else
	{
#ifdef PRE_ADD_RELOGIN_PROCESS
		// Retry 3 times on duplicate login.
		if( pPacket->nRet == ERROR_GENERIC_DUPLICATEUSER && m_nReLoginCount < 3 )
		{
			Sleep( 1000 );
			RequestLogin( m_strConnectID.c_str(), m_strConnectPassword.c_str() );
			m_nReLoginCount++;
			return;
		}
#endif // PRE_ADD_RELOGIN_PROCESS
		// Not actually a MESSAGEBOX_1 message, but set to MESSAGEBOX_2 to share callback handling (focus goes to PW input box when message box closes).
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, MESSAGEBOX_2, GetInterface().GetLoginDlg());
		GetInterface().GetLoginDlg()->EnableLoginButton( true );
	}
}

void CDnLoginTask::OnRecvLoginCharList( SCCharList *pPacket )
{
	SetRequestWait( false );
	FUNC_LOG();
	if( pPacket->nRet != ERROR_NONE ) {
		SelectActorSlot( -1 );
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::CharListFailed, this);
		return;
	}
	// Block Back on channel list to prevent UI from showing early and getting messed up by clicking Back again or Create Character.
	if( m_PrevState == ChannelList ) GetInterface().ChangeState( CharSelect );

#ifndef _FINAL_BUILD
	if( CGlobalValue::GetInstance().IsPermitLogin() )
	{
		ChangeState( CharSelect );
		return;
	}
#endif

	CGlobalInfo::GetInstance().m_cLocalAccountLevel = pPacket->cAccountLevel;

	// m_nSelectedServerIndex is -1 only when returning to character select from village or in-game.
	if( m_nSelectedServerIndex != -1 ) {
		CDnBridgeTask *pTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
		std::wstring wszName = GetInterface().GetServerName( m_nSelectedServerIndex );
		if( pTask && !wszName.empty() )
			pTask->SetCurrentServerName( wszName.c_str() );
	}

	if( m_State == ServerList ) {
		memset(&m_CharList, 0, sizeof(SCCharList));
		m_CharList.nRet = pPacket->nRet;
		m_CharList.cCharCount = pPacket->cCharCount;
		m_CharList.cCharCountMax = pPacket->cCharCountMax;
		for( int i = 0; i < pPacket->cCharCount; i++) {
			m_CharList.CharListData[i] = pPacket->CharListData[i];
		}

		GetInterface().ShowWaitUserDlg( false );	
		GetInterface().ShowServerSelectDlg( false );
		GetInterface().FadeDialog( 0x00ffffff, 0xFFFFFFFF, 1.f, this, true );
	}
	else {
		_OnRecvLoginCharList( pPacket );
		if( m_PrevState == StartGame )
			GetInterface().FadeDialog( 0xff000000, 0x00000000, 1.f, this );
		CEtSoundEngine::GetInstance().FadeVolume( NULL, 1.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );
	}
}

void CDnLoginTask::_OnRecvLoginCharList( SCCharList *pPacket )
{
//	CGlobalValue::GetInstance().m_CharListData =  *pPacket;
	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ )
	{
		SAFE_RELEASE_SPTR( m_hSlotActorHandle[i] );
	}

	SecureZeroMemory( m_nSlotActorMapID, sizeof(m_nSlotActorMapID) );
	memset( m_bSlotDeleteWait, 0, sizeof(m_bSlotDeleteWait) );
	memset( m_SlotDelteWaitRemainTime, 0, sizeof(m_SlotDelteWaitRemainTime) );
	GetInterface().ShowSlotInfoDlg( false );
	GetInterface().ShowSlotDeleteWaitDlg( false );

	memset( m_bSlotOpen, 0, sizeof(m_bSlotOpen) );
	GetInterface().ShowSlotBlockDlg( false );

	m_vecCharListData.clear();
	m_nTotalCharSelectPage = 0;
	m_nEmptyCharSlotCount = 0;

	m_nCharSlotCount = pPacket->cCharCountMax;	// Max slot count
	int nCharListCount = pPacket->cCharCount;	// Current character count
	TCharListData stEmptyCharListDat;
	memset( &stEmptyCharListDat, 0, sizeof( TCharListData ) );
	int nBeforeCharIndex = -1;

	for( int i=0; i<nCharListCount; i++ )	// Save character list
	{
		int nIndexGap = 0;
		if( nBeforeCharIndex == -1 )	// When the very first character's index is not 0
			nIndexGap = pPacket->CharListData[i].cCharIndex;
		else
			nIndexGap = pPacket->CharListData[i].cCharIndex - nBeforeCharIndex - 1;

		if( nIndexGap > 0 )	// Fill with empty slots if the index skips over the previous index.
		{
			for( int j=0; j<nIndexGap; j++ )
			{
				m_vecCharListData.push_back( stEmptyCharListDat );
				m_nEmptyCharSlotCount++;
			}
		}
		m_vecCharListData.push_back( pPacket->CharListData[i] );
		nBeforeCharIndex = pPacket->CharListData[i].cCharIndex;
	}

	int nVecCharListCount = static_cast<int>( m_vecCharListData.size() );
	if( m_nCharSlotCount > nVecCharListCount )
	{
		for( int i=0; i<m_nCharSlotCount - nVecCharListCount; i++ )
		{
			m_vecCharListData.push_back( stEmptyCharListDat );
			m_nEmptyCharSlotCount++;
		}
	}

	m_nTotalCharSlotCount = static_cast<int>( m_vecCharListData.size() );

	int nRemainder = m_nTotalCharSlotCount % SHOWCHARNUMPERONEPAGE;
	if( nRemainder > 0 )
		m_nTotalCharSelectPage = ( m_nTotalCharSlotCount / SHOWCHARNUMPERONEPAGE ) + 1;
	else
		m_nTotalCharSelectPage = m_nTotalCharSlotCount / SHOWCHARNUMPERONEPAGE;

	int nCreateCharPage = 0;
	if( m_wstrCreateCharName.length() > 0 )	// If list was received after character creation, move to the page containing the created character
	{
		std::vector<TCharListData>::iterator iter = m_vecCharListData.begin();
		for( ; iter != m_vecCharListData.end(); iter++ )
		{
			if( wcscmp( (*iter).wszCharacterName, m_wstrCreateCharName.c_str() ) == 0 )
				nCreateCharPage = ( (*iter).cCharIndex / SHOWCHARNUMPERONEPAGE ) + 1;
		}
	}

	if( nCreateCharPage > 0 )					// Move to created character's page after character creation
	{
		SetCharSelectPage( nCreateCharPage );
		m_wstrCreateCharName = L"";
	}
	else if( m_nDeleteCharPage > 0 )			// Move to deleted character's page after character deletion
	{
		SetCharSelectPage( m_nDeleteCharPage );
		m_nDeleteCharPage = 0;
	}
	else
	{
		SetCharSelectPage( m_nCurrentCharSelectPage );
	}

#ifndef _FINAL_BUILD
	CGlobalValue::GetInstance().m_bMessageBoxWithMsgID = false;
#endif
}

void CDnLoginTask::SetCharSelectPage( int nSelectPage )
{
	if( nSelectPage < 1 || nSelectPage > m_nTotalCharSelectPage )
		return;

	SelectActorSlot( -1 );
	m_nCurrentCharSelectPage = nSelectPage;

	DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );
	if( !pActorSox ) return;

	DnActorHandle hActor;
	CDnPlayerActor *pPlayer;
	char szTemp[128];

	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ )
		SAFE_RELEASE_SPTR( m_hSlotActorHandle[i] );
	SAFE_RELEASE_SPTRVEC( m_vecOutlineSlot );

	memset( m_bSlotDeleteWait, 0, sizeof(m_bSlotDeleteWait) );
	memset( m_SlotDelteWaitRemainTime, 0, sizeof(m_SlotDelteWaitRemainTime) );
	memset( m_nSlotActorMapID, 0, sizeof(m_nSlotActorMapID) );
	memset( m_bSlotOpen, 0, sizeof(m_bSlotOpen) );

	std::vector<TCharListData>::iterator iter = m_vecCharListData.begin();
	iter += ( nSelectPage - 1) * SHOWCHARNUMPERONEPAGE;

	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++, iter++ )
	{
		if( iter == m_vecCharListData.end() )
			break;

		m_bSlotOpen[i] = true;

		int nBaseClassID = CDnPlayerState::GetJobToBaseClassID( (*iter).cJob );
		
		if( nBaseClassID == 0 )	// Skip if empty slot
			continue;

		hActor = CreateActor( nBaseClassID, false, true );

		if( !hActor )
			continue;

		hActor->ShowHeadName( false );
		if( hActor->GetObjectHandle() && hActor->GetObjectHandle()->GetSkinInstance() )
			hActor->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
		
		pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
		char *pStaticName = pActorSox->GetFieldFromLablePtr( nBaseClassID, "_StaticName" )->GetString();
#ifdef PRE_FIX_MEMOPT_EXT
		char *pDummySkinName = CommonUtil::GetFileNameFromFileEXT(pActorSox, nBaseClassID, "_SkinName");
#else
		char *pDummySkinName = pActorSox->GetFieldFromLablePtr( nBaseClassID, "_SkinName" )->GetString();
#endif
		sprintf_s( szTemp, "%s\\%s_login.ani", pStaticName, pStaticName );
		pPlayer->LoadSkin( CEtResourceMng::GetInstance().GetFullName( pDummySkinName ).c_str(), CEtResourceMng::GetInstance().GetFullName( szTemp ).c_str() );
		sprintf_s( szTemp, "%s\\%s_login.act", pStaticName, pStaticName );
		hActor->LoadAction( CEtResourceMng::GetInstance().GetFullName( szTemp ).c_str() );
		pPlayer->SetJobHistory( (*iter).cJob );

		int nDefaultPartsInfo[4] = { (*iter).nDefaultBody, (*iter).nDefaultLeg, (*iter).nDefaultHand, (*iter).nDefaultFoot };
		pPlayer->SetDefaultPartsInfo( nDefaultPartsInfo );

		int *pEquipArray = (*iter).nEquipArray;
		for( int j=EQUIP_FACE; j<=EQUIP_RING2; j++ )
		{
			if( pEquipArray[j] < 1 )
			{
				pPlayer->DetachParts( (CDnParts::PartsTypeEnum)j );
				continue;
			}
			DnPartsHandle hParts = CDnParts::CreateParts( pEquipArray[j], 0 );
			pPlayer->AttachParts( hParts, (CDnParts::PartsTypeEnum)j, true, false );

			if( pPlayer->GetObjectHandle() && pPlayer->GetObjectHandle()->GetSkinInstance() )
				pPlayer->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
		}		

		pPlayer->SetPartsColor( MAPartsBody::HairColor, (*iter).dwHairColor );
		pPlayer->SetPartsColor( MAPartsBody::EyeColor, (*iter).dwEyeColor );
		pPlayer->SetPartsColor( MAPartsBody::SkinColor, (*iter).dwSkinColor );

		for( int j=0; j<2; j++ )
		{
			if( pEquipArray[EQUIP_WEAPON1 + j] == 0 ) continue;
			DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( pEquipArray[EQUIP_WEAPON1 + j], 0 );
			hActor->AttachWeapon( hWeapon, j, true );
		}

		hActor->Initialize();

		int *pCashEquipArray = (*iter).nCashEquipArray;
		for( int j=CASHEQUIP_HELMET; j<=CASHEQUIP_FAIRY; j++ )
		{
			if( pCashEquipArray[j] < 1 ) continue;

			// Since login does not call CombineParts, remove normal items here.
			if( j >= CASHEQUIP_HELMET && j <= CASHEQUIP_FOOT )
			{
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
				if( pPlayer->GetParts( (CDnParts::PartsTypeEnum)( j + 2 ) ) && !CDnParts::IsTransparentSkin( pCashEquipArray[j], nBaseClassID ) )
#else
				if( pPlayer->GetParts( (CDnParts::PartsTypeEnum)( j + 2 ) ) )
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME
					pPlayer->DetachParts( (CDnParts::PartsTypeEnum)( j + 2 ) );
			}
			DnPartsHandle hParts = CDnParts::CreateParts( pCashEquipArray[j], 0 );
			pPlayer->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)j, true, false );
			if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
				hParts->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
		}

		for( int j=0; j<2; j++ )
		{
			if( pCashEquipArray[CASHEQUIP_WEAPON1 + j] == 0 ) continue;
			DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( pCashEquipArray[CASHEQUIP_WEAPON1 + j], 0 );
			pPlayer->AttachCashWeapon( hWeapon, j, true, false );
		}
#ifdef PRE_MOD_WARRIOR_RIGHTHAND
		// Since login does not call CombineParts, forcibly call hiding right glove mesh when warrior equips gauntlet.
		pPlayer->OnCombineParts();
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND

		CEtOutlineFilter *pOutlineFilter = GetOutlineFilter();
		if( pOutlineFilter )
		{
			EtOutlineHandle hOutline = CEtOutlineObject::Create( hActor->GetObjectHandle() );
			static EtColor vColor(0.2f, 0.5f, 1.0f, 1.0f);
			hOutline->SetColor( vColor );
			hOutline->SetWidth( 0.7f );
			m_vecOutlineSlot.push_back( hOutline );
		}

		m_hSlotActorHandle[i] = hActor;
		m_nSlotActorMapID[i] = (*iter).nMapID;
		m_bSlotDeleteWait[i] = (*iter).bDeleteFlag;
		m_SlotDelteWaitRemainTime[i] = (*iter).DeleteDate;
		hActor->SetPosition( m_vSlotActorPos[i] );
		std::string szAction;
		if( hActor->GetWeapon(0, false) ) szAction = "Stand";
		else szAction = "NormalStand";
		hActor->SetActionQueue( szAction.c_str() );
		hActor->SetFaceAction( "Flicker" );

		hActor->SetName( (*iter).wszCharacterName );
		hActor->SetLevel( ( (*iter).cLevel < 1 ) ? 1 : (*iter).cLevel );
		hActor->SetHP( hActor->GetMaxHP() );
		hActor->GetObjectHandle()->SetCalcPositionFlag(CALC_POSITION_X|CALC_POSITION_Y|CALC_POSITION_Z);
		
		if( m_bSlotDeleteWait[i] )
		{
			hActor->SetScale( EtVector3( 0.8f, 0.8f, 0.8f ) );
			hActor->SetHP( 0 );
			pPlayer->ToggleGhostMode( true, false );
			if( pPlayer->GetElement( "Move_Front" ) )
				pPlayer->SetActionQueue( "Stand2", 0, 0.f, (float)( rand()%pPlayer->GetElement( "Move_Front" )->dwLength ), false, false );
		}

//		EtVector3 vDir = m_hCamera->GetMatEx()->m_vPosition - *hActor->GetPosition();
//		EtVec3Normalize( &vDir, &vDir );
		hActor->Look( EtVec3toVec2( m_vSlotActorLook[i] ) );
	}

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	// #54540 Call RecompseAction() one more time including unowned characters.
	// RecomposeAction() in PreInitialize() is called without going through LoginTask when returning from the tutorial
	// Because it was called directly without matching _Login.act, related sound or effect signals are not loaded.
	// Therefore, change from calling only for owned characters from the character list to calling for all character types.
	for( int i = 0; i < (int)m_hVecCreateActorList.size(); ++i )
	{
		DnActorHandle hActor = m_hVecCreateActorList.at(i);
		if( hActor && hActor->IsPlayerActor() )
		{
			CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
			pPlayerActor->RecomposeAction();
		}
	}
#endif

	m_bCompleteCharList = true;

	// Enable/disable character selection page buttons
	if( m_nCurrentCharSelectPage < m_nTotalCharSelectPage )
		GetInterface().ShowCharSelectArrowRight( true );
	else
		GetInterface().ShowCharSelectArrowRight( false );

	if( m_nCurrentCharSelectPage > 1 )
		GetInterface().ShowCharSelectArrowLeft( true );
	else
		GetInterface().ShowCharSelectArrowLeft( false );

	ShowSlotInfoDlg();	
}

void CDnLoginTask::NextCharListPage()
{
	if( m_nCurrentCharSelectPage < m_nTotalCharSelectPage )
		SetCharSelectPage( m_nCurrentCharSelectPage + 1 );
}

void CDnLoginTask::PrevCharListPage()
{
	if( m_nCurrentCharSelectPage > 1 )
		SetCharSelectPage( m_nCurrentCharSelectPage - 1 );
}

int CDnLoginTask::GetSelectExtendActorSlot()
{
	if( m_nSelectIndex < 0 || m_nCurrentCharSelectPage < 1 )
		return 0;

	int nSelectCharIndex = 0;
	nSelectCharIndex = m_nSelectIndex + ( ( m_nCurrentCharSelectPage - 1 ) * SHOWCHARNUMPERONEPAGE );

	return nSelectCharIndex;
}

#ifdef PRE_MOD_CREATE_CHAR
void CDnLoginTask::SetCharCreateBasePartsName()
{
	DnPartsHandle hParts;
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Hair][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hair]];
	GetInterface().SetCharCreatePartsName( CDnParts::Hair, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Face][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Face]];
	GetInterface().SetCharCreatePartsName( CDnParts::Face, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Body][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Body]];
	GetInterface().SetCharCreatePartsName( CDnParts::Body, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Leg][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Leg]];
	GetInterface().SetCharCreatePartsName( CDnParts::Leg, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Hand][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hand]];
	GetInterface().SetCharCreatePartsName( CDnParts::Hand, hParts->GetName() );
	hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[CDnParts::Foot][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Foot]];
	GetInterface().SetCharCreatePartsName( CDnParts::Foot, hParts->GetName() );
}
#endif // PRE_MOD_CREATE_CHAR

void CDnLoginTask::OnRecvLoginWaitUser( SCWaitUser *pPacket )
{

	std::wstring wszServerName = GetInterface().GetServerName( m_nSelectedServerIndex );
	GetInterface().ShowWaitUserDlg( true );
	GetInterface().ShowServerSelectDlg( false );
	GetInterface().SetWaitUserProperty( wszServerName.c_str(),  pPacket->nWaitUserCount, pPacket->nEstimateTime);	
}

void CDnLoginTask::OnRecvLoginBlock( SCCHeckBlock *pPacket )
{
	// Penalty
	WCHAR wszTemp[512]={0,};

	std::wstring wszBeginTime, wszEndTime;
	DN_INTERFACE::STRING::GetTimeText( wszBeginTime, pPacket->StartDate );
	DN_INTERFACE::STRING::GetTimeText( wszEndTime, pPacket->EndDate );

	if( (pPacket->cType == _RESTRAINTTYPE_BLOCK)  && 
		pPacket->iReasonID == SecondAuth::Common::RestraintReasonID ) 
	{
		swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6417 ) , SecondAuth::Common::LimitCount   , SecondAuth::Common::RestraintTimeMin,  wszBeginTime.c_str(), wszEndTime.c_str() );
		GetInterface().MessageBox( wszTemp, MB_OK, UICallbackEnum::DisconnectTCP, this, true );
		return; 
	}

#if defined(_RDEBUG)
	bool bPermanent = false;
	DBTIMESTAMP DbTime;
	CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( pPacket->EndDate, &DbTime );
	if( DbTime.year == 2050 && DbTime.month == 1 && DbTime.day == 1 && DbTime.hour == 0 && DbTime.minute == 0 && DbTime.second == 0 ) bPermanent = true;
	if( DbTime.year == 2049 && DbTime.month == 12 && DbTime.day == 31 && DbTime.hour == 23 && DbTime.minute == 0 && DbTime.second == 0 ) bPermanent = true;
	if( bPermanent )
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4432 ) );
	else
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4400 ), pPacket->wszBlockReason, wszBeginTime.c_str(), wszEndTime.c_str() );
#else
	swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4400 ), pPacket->wszBlockReason, wszBeginTime.c_str(), wszEndTime.c_str() );
#endif
	GetInterface().MessageBox( wszTemp, MB_OK, UICallbackEnum::DisconnectTCP, this, true, false, false, true );
}

void CDnLoginTask::OnRecvLoginCharCreate( SCCreateChar *pPacket )
{
	FUNC_LOG();
	if (pPacket->nRet == ERROR_NONE) {
		ChangeState( CharSelect );
		SelectServer( m_nSelectedServerIndex );
		m_wstrCreateCharName = pPacket->wszCharacterName;
	}
	else
	{
		if( pPacket->nRet == ERROR_LOGIN_CANNOT_CREATE_CHARACTER_TODAY ) {
			WCHAR wszTemp[128];
			swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100058 ), pPacket->cDailyCreateMaxCount );
			GetInterface().MessageBox( wszTemp, MB_OK, 0, GetInterface().GetCharCreateDlg() );
		}
		else {
			GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, 0, GetInterface().GetCharCreateDlg());
		}
	}
}

void CDnLoginTask::SetRequestWait( bool bWait )
{
	m_bRequestWait = bWait;
	if( bWait ) {
		GetInterface().EnableCharSelectDlgControl( false, "ID_CREATE_CHAR" );
		GetInterface().EnableCharSelectDlgControl( false, "ID_DELETE_CHAR" );
		GetInterface().EnableCharSelectDlgControl( false, "ID_CONNECT" );
		GetInterface().EnableCharSelectDlgControl( false, "ID_BACK" );
	}
	else {
		GetInterface().EnableCharSelectDlgControl( true, "ID_BACK" );
		GetInterface().EnableCharSelectDlgControl( ((m_nCharSlotCount==CHARCOUNTMAX )?false:true), "ID_CREATE_CHAR" );
	}
}

void CDnLoginTask::OnRecvLoginCharDelete( SCDeleteChar *pPacket )
{
	FUNC_LOG();
	SetRequestWait( false );

	SelectActorSlot( -1 );
	/*
	// Do not run callback for Fail received on guild master deletion.
	if( pPacket->nRet == 103101 )
	{
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK);
		return;
	}

	if( pPacket->nRet == 103211 )
	{
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK);
		return;
	}
	*/

	// Block Callback handling.
	if( pPacket->nRet == 103101 || // Guild master.
		pPacket->nRet == 103211 || // Master and apprentice...
		pPacket->nRet == 103334 || // Best friend status.
		pPacket->nRet == 103346)  // Join guild
	{
		GetInterface().ServerMessageBox( pPacket->nRet, MB_OK );
		return;
	}

	if( pPacket->nRet != ERROR_NONE )
	{
		if( pPacket->nRet == ERROR_SECONDAUTH_CHECK_OLDPWINVALID || 
			pPacket->nRet == ERROR_SECONDAUTH_CHECK_FAILED)
		{
			WCHAR wzStrTmp[1024]= {0,};
			swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6303) , pPacket->cFailCount , SecondAuth::Common::LimitCount );
			GetInterface().MessageBox( wzStrTmp ) ;	
		}
		else 
		{
			GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::CharDeleteFailed, this);
		}
		return;
	}

	SelectServer( m_nSelectedServerIndex );
}

bool CDnLoginTask::SelectActorSlot( int nIndex )
{
	if( nIndex != -1 && m_nSelectIndex == nIndex ) 
		return false;

	m_nSelectIndex = nIndex;
	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ ) {
		if( m_hSlotActorHandle[i] )
			m_hSlotActorHandle[i]->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( -1, -1, true );
	}

	if( nIndex == -1 ) 
	{
		GetInterface().EnableCharSelectDlgControl( false, "ID_CONNECT" );
		GetInterface().EnableCharSelectDlgControl( false, "ID_DELETE_CHAR" );
	}
	else {
		std::string szAction;
		if( m_hSlotActorHandle[nIndex]->GetWeapon(0, false) ) szAction = "Select";
		else szAction = "Select_NoWeapon";
		m_hSlotActorHandle[nIndex]->SetActionQueue( szAction.c_str() );

		DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
		if( hHandle ) {
			hHandle->SetActionQueue( "CharSelect" );
			hHandle->SetPosition( m_vSlotActorPos[m_nSelectIndex] );
			m_hSlotActorHandle[m_nSelectIndex]->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );
		}

		GetInterface().EnableCharSelectDlgControl( true, "ID_CONNECT" );
		GetInterface().EnableCharSelectDlgControl( true, "ID_DELETE_CHAR" );

		CDnBridgeTask::GetInstance().SetCurrentCharLevel( m_hSlotActorHandle[m_nSelectIndex]->GetLevel() );

	}
	int nOutlineIndex = -1;
	int nOutlineCount = 0;
	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ ) {
		if( !m_hSlotActorHandle[i] ) continue;
		if( nIndex == i ) {
			nOutlineIndex = nOutlineCount;
			break;
		}
		nOutlineCount++;
	}

	CEtOutlineFilter *pOutlineFilter = GetOutlineFilter();
	if( pOutlineFilter ) {
		for( int i = 0; i < (int)m_vecOutlineSlot.size(); i++) {
			m_vecOutlineSlot[i]->Show( ( nOutlineIndex == i ) ? true : false );
		}
	}

	return true;
}

void CDnLoginTask::CalcCharSlotPosition()
{
	m_TreeOffset.Identity();

	std::vector<CEtWorldProp*> pVecList;
	m_pWorld->FindPropFromCreateUniqueID( 25, &pVecList );
	if( !pVecList.empty() ) {
		CDnWorldProp *pProp = (CDnWorldProp *)pVecList[0];
		m_TreeOffset = *pProp->GetMatEx();
	}

	DWORD dwCount = m_pWorld->GetGrid()->GetActiveSectorCount();
	CEtWorldEventControl *pControl = NULL;
	CEtWorldSector *pSector = NULL;
	CEtWorldEventArea *pArea = NULL;
	char szSlotName[32];

	EtVector2 vPos;
	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ ) {
		m_vSlotActorPos[i] = EtVector3( 0.f, 0.f, 0.f );
		m_vSlotActorLook[i] = EtVector3( 0.f, 0.f, 1.f );
	}

	for( DWORD i=0; i<dwCount; i++ ) {
		pSector = m_pWorld->GetGrid()->GetActiveSector(i);
		pControl = pSector->GetControlFromUniqueID( ETE_EventArea );
		if( !pControl ) continue;

		// Get Select Slot
		for( int j=0; j<SHOWCHARNUMPERONEPAGE; j++ ) {
			sprintf_s( szSlotName, "Slot %d", j + 1 );

			pArea = pControl->GetAreaFromName( szSlotName );
			if( !pArea ) continue;

			vPos.x = ( pArea->GetMin()->x + ( ( pArea->GetMax()->x - pArea->GetMin()->x ) / 2.f ) );
			vPos.y = ( pArea->GetMin()->z + ( ( pArea->GetMax()->z - pArea->GetMin()->z ) / 2.f ) );
			m_vSlotActorPos[j] = EtVec2toVec3( vPos );
			m_vSlotActorPos[j].y = m_pWorld->GetHeight( m_vSlotActorPos[j] );
			m_vSlotActorLook[j] = pArea->GetOBB()->Axis[2];
		}
	}
}

void CDnLoginTask::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( nReceiverState & IR_MOUSE_LB_DOWN ) 
	{
		switch( m_State ) {
			case LoginStateEnum::CharSelect:
				{
					if( GetInterface().IsShowSecurityChangeDlg() ||
						GetInterface().IsShowSecurityCheckDlg() ) break;
					if( m_bRequestWait ) break;
					SSegment Segment = { EtVector3(0,0,0), EtVector3(0,0,1) };
					POINT p;
					::GetCursorPos( &p );
					ScreenToClient( CDnMainFrame::GetInstance().GetHWnd(), &p );
					if( m_hCamera && m_hCamera->GetCameraHandle() ) {
						m_hCamera->GetCameraHandle()->CalcPositionAndDir( p.x, p.y, Segment.vOrigin, Segment.vDirection );
					}
					Segment.vDirection *= 100000.f;

					int nIndex = -1;
					for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ ) {
						if( !m_hSlotActorHandle[i] ) continue;
						if( m_bSlotDeleteWait[i] ) continue;
						if( m_hSlotActorHandle[i]->GetObjectHandle()->TestSegmentCollision( Segment ) == true ) {
							nIndex = i;
							break;
						}
					}
					if( nIndex == -1 ) {
						if( m_nSelectIndex == -1 ) {
							SelectActorSlot( -1 );
						}						
					}
					else 
					{
						if( !SelectActorSlot( nIndex ) )
						{
							if( nReceiverState & IR_MOUSE_LB_DBCLK )
							{
#ifdef PRE_ADD_COMEBACK
								CDnBridgeTask *pTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
								if( pTask && pTask->GetComeback() )
									GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7740 ), MB_OKCANCEL, UICallbackEnum::ComeBack, this ); // "Are you sure you want to receive it on this character? (Cannot be changed once selected. Please choose carefully.)"
								else
									GetInterface().OpenSecurityCheckDlg(CDnInterface::InterfaceTypeEnum::Login , 0 );								
#else
								GetInterface().OpenSecurityCheckDlg(CDnInterface::InterfaceTypeEnum::Login , 0 );
#endif

							//GetInterface().OpenSecurityCheckDlg(CDnInterface::InterfaceTypeEnum::Login , 0 );
							}
						}
					}
				}
				break;
			case LoginStateEnum::CharCreate_SelectClass:
				{
					if( m_bProcessCamera ) break;
					int nIndex = GetCreateSelectClass();
					if( nIndex != -1 && m_nSelectClassIndex != nIndex ) {
						ChangeCreateClass( nIndex );
						if( m_pWorld && m_pWorld->GetEnvironment() ) {
							CEtOutlineFilter *pOutlineFilter = m_pWorld->GetEnvironment()->GetOutlineFilter();
							if( pOutlineFilter ) {
								for( int i = 0; i < (int)m_vecOutlineCreate.size(); i++) {
									m_vecOutlineCreate[i]->Show( false );
								}
							}
						}
					}
				}
				break;
		}
	}
	if( nReceiverState & IR_MOUSE_MOVE ) {
		switch( m_State ) {
			case LoginStateEnum::CharCreate_SelectClass:
				{ 
					if( m_bProcessCamera ) break;
					int nIndex = GetCreateSelectClass();
					if( m_pWorld && m_pWorld->GetEnvironment() ) {
						CEtOutlineFilter *pOutlineFilter = m_pWorld->GetEnvironment()->GetOutlineFilter();
						if( pOutlineFilter ) {
							for( int i = 0; i < (int)m_vecOutlineCreate.size(); i++) {
								m_vecOutlineCreate[i]->Show( ( i == nIndex) ? true : false );
							}
						}
					}
					for( DWORD i=0; i<m_hVecCreateActorList.size(); i++ ) {
						DnActorHandle hActor = m_hVecCreateActorList[i];
						if( nIndex == i ) {
							EtVector3 vPos = CDnCamera::GetActiveCamera()->GetMatEx()->m_vPosition;
							vPos += ( -1000.f * CDnCamera::GetActiveCamera()->GetMatEx()->m_vZAxis );
							hActor->SetHeadLookTarget( vPos );
						}
						else hActor->SetHeadLookTarget( EtVector3( 0.f, 0.f, 0.f ) );
					}
				}
				break;
		}
	}
}

int CDnLoginTask::GetCreateSelectClass()
{
	SSegment Segment;
	POINT p;
	::GetCursorPos( &p );
	ScreenToClient( CDnMainFrame::GetInstance().GetHWnd(), &p );
	m_hCamera->GetCameraHandle()->CalcPositionAndDir( p.x, p.y, Segment.vOrigin, Segment.vDirection );
	Segment.vDirection *= 100000.f;

	SAABox Box;
	float fDist;
	for( DWORD i=0; i<m_hVecCreateActorList.size(); i++ ) {
		DnActorHandle hActor = m_hVecCreateActorList[i];
		hActor->GetBoundingBox( Box );
		EtMatrix matTemp = hActor->GetBoneMatrix( "Bip01" );
		EtVector3 vTemp;
		EtMatrix matTrans = m_TreeOffset;
		memset( &matTrans._41, 0, sizeof(EtVector3) );
		memcpy( &vTemp, &matTemp._41, sizeof(EtVector3) );
		vTemp -= *hActor->GetPosition();
		vTemp.y = 0.f;

		Box.Max += vTemp;
		Box.Min += vTemp;
		if( TestLineToBox( Segment.vOrigin, Segment.vDirection, Box, fDist ) == true ) {
			return i;
		}
	}
	return -1;
}

void CDnLoginTask::CreateCharacter( TCHAR *szName )
{
	int Info[EQUIPMAX] = { 0, };
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Face] != -1 )
		Info[EQUIP_FACE] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Face][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Face]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hair] != -1 )
		Info[EQUIP_HAIR] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Hair][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hair]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Body] != -1 )
		Info[EQUIP_BODY] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Body][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Body]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Leg] != -1 )
		Info[EQUIP_LEG] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Leg][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Leg]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hand] != -1 )
		Info[EQUIP_HAND] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Hand][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Hand]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Foot] != -1 )
		Info[EQUIP_FOOT] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Foot][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Foot]];
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Helmet] != -1 )
		Info[EQUIP_HELMET] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[CDnParts::Helmet][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[CDnParts::Helmet]];

	Info[EQUIP_WEAPON1] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nWeapon[0];
	Info[EQUIP_WEAPON2] = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nWeapon[1];

	SendCreateChar( Info, szName, m_VecCreateDefaultPartsList[m_nSelectClassIndex].dwColor[MAPartsBody::HairColor],
												m_VecCreateDefaultPartsList[m_nSelectClassIndex].dwColor[MAPartsBody::EyeColor], 
												m_VecCreateDefaultPartsList[m_nSelectClassIndex].dwColor[MAPartsBody::SkinColor], 
								(BYTE)m_hVecCreateActorList[m_nSelectClassIndex]->GetClassID() );
	//GetInterface().ShowCharCreateDlg( false );
}

void CDnLoginTask::DeleteCharacter()
{
	int nSelectCharIndex = 0;
	m_nDeleteCharPage = m_nCurrentCharSelectPage;
	nSelectCharIndex = GetSelectExtendActorSlot();


	SendDeleteChar( nSelectCharIndex, m_nAuthPasswordSeed, m_nAuthPasswordValue );
	SetRequestWait( true );
}

void CDnLoginTask::ChangeCreateClass( int nClassID )
{
	for( DWORD i=0; i<m_hVecCreateActorList.size(); i++ ) 
		GetInterface().ShowCreateInfoList( false, i );

	int nPrevIndex = m_nSelectClassIndex;
	m_nSelectClassIndex = nClassID;
	if( m_nSelectClassIndex != -1 )
		GetInterface().ShowCreateInfoList( true, m_nSelectClassIndex );

	DnActorHandle hActor;
	for( DWORD i=0; i<m_hVecCreateActorList.size(); i++ ) {
		hActor = m_hVecCreateActorList[i];
		if( nPrevIndex == i ) {
			hActor->SetActionQueue( "CreateSelect_Cancle", 0, 30.f );
		}
		if( nClassID == i ) {
			hActor->SetActionQueue( "CreateSelect" );
		}
	}
	if( nClassID == -1 ) return;
#if defined(PRE_ADD_KALI) && !defined(PRE_REMOVE_KALI)
	const char *szCameraName[] = { "Wa_Select", "Ar_Select", "So_Select", "Cl_Select", "Ac_Select", "Ka_Select" };
	const char *szCameraNextName[] = { "Wa_Select_Stay", "Ar_Select_Stay", "So_Select_Stay", "Cl_Select_Stay", "Ac_Select_Stay", "Ka_Select_Stay" };
#elif defined(PRE_ADD_ACADEMIC) && !defined(PRE_REMOVE_ACADEMIC)
	const char *szCameraName[] = { "Wa_Select", "Ar_Select", "So_Select", "Cl_Select", "Ac_Select" };
	const char *szCameraNextName[] = { "Wa_Select_Stay", "Ar_Select_Stay", "So_Select_Stay", "Cl_Select_Stay", "Ac_Select_Stay" };
#else
	const char *szCameraName[] = { "Wa_Select", "Ar_Select", "So_Select", "Cl_Select" };
	const char *szCameraNextName[] = { "Wa_Select_Stay", "Ar_Select_Stay", "So_Select_Stay", "Cl_Select_Stay" };
#endif // #if defined(PRE_ADD_ACADEMIC) && !defined(PRE_REMOVE_ACADEMIC)
	((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( szCameraName[m_nSelectClassIndex], szCameraNextName[m_nSelectClassIndex] );
	ChangeState( LoginStateEnum::CharCreate_ChangeParts );
#ifdef PRE_MOD_CREATE_CHAR
	SetCharCreateBasePartsName();
#endif // PRE_MOD_CREATE_CHAR
}

void CDnLoginTask::SetCharColor( MAPartsBody::PartsColorEnum colorType, int nColorIndex, int nClassIndex )
{
	DNTableFileFormat* pDefaultSox = GetDNTable( CDnTableDB::TDEFAULTCREATE );

	int nClass = ( nClassIndex == -1) ? m_nSelectClassIndex : nClassIndex;
	
	switch( colorType ) {
		case MAPartsBody::HairColor:
		{
			D3DCOLOR d3dColor = D3DCOLOR_XRGB(
						pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_HairColor%dR", nColorIndex+1).c_str() )->GetInteger(),
						pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_HairColor%dG", nColorIndex+1).c_str() )->GetInteger(),
						pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_HairColor%dB", nColorIndex+1).c_str() )->GetInteger() );

			MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[ nClass ].GetPointer());
			float fIntensity = 1.1f;
			DWORD dwColorR10G10B10=0;
			CDnParts::ConvertD3DCOLORToR10G10B10( &dwColorR10G10B10, d3dColor, fIntensity );
			m_VecCreateDefaultPartsList[ nClass ].dwColor[ MAPartsBody::HairColor ] = dwColorR10G10B10;
			pPartsBody->SetPartsColor( MAPartsBody::HairColor, dwColorR10G10B10 );
		}
		break;
		case MAPartsBody::SkinColor:
		{
			int nA = (int)(pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_SkinColor%dA", nColorIndex+1).c_str() )->GetFloat() * 255);
			int nR = (int)(pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_SkinColor%dR", nColorIndex+1).c_str() )->GetFloat() * 255);
			int nG = (int)(pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_SkinColor%dG", nColorIndex+1).c_str() )->GetFloat() * 255);
			int nB = (int)(pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_SkinColor%dB", nColorIndex+1).c_str() )->GetFloat() * 255);
			
			ASSERT( nA >= 0 && nR >= 0 && nG >= 0 && nB >= 0 );
			ASSERT( nA <= 255 && nR <= 255 && nG <= 255 && nB <= 255 );

			DWORD dwSkinColor = D3DCOLOR_ARGB( nA, nR, nG, nB );
			m_VecCreateDefaultPartsList[ nClass ].dwColor[ MAPartsBody::SkinColor ] = dwSkinColor;

			MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[ nClass ].GetPointer());
			pPartsBody->SetPartsColor( MAPartsBody::SkinColor, dwSkinColor );
		}
		break;
		case MAPartsBody::EyeColor:
		{
			D3DCOLOR d3dColor = D3DCOLOR_XRGB(
				pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_EyeColor%dR", nColorIndex+1).c_str() )->GetInteger(),
				pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_EyeColor%dG", nColorIndex+1).c_str() )->GetInteger(),
				pDefaultSox->GetFieldFromLablePtr( nClass+1, FormatA("_EyeColor%dB", nColorIndex+1).c_str() )->GetInteger() );

			MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[ nClass ].GetPointer());			
			float fIntensity = 1.0f;
			DWORD dwColorR10G10B10=0;
			CDnParts::ConvertD3DCOLORToR10G10B10( &dwColorR10G10B10, d3dColor, fIntensity );
			m_VecCreateDefaultPartsList[ nClass ].dwColor[ MAPartsBody::EyeColor ] = dwColorR10G10B10;
			pPartsBody->SetPartsColor( MAPartsBody::EyeColor, dwColorR10G10B10 );
		}
		break;
	}
}

void CDnLoginTask::RotateCreateParts( CDnParts::PartsTypeEnum PartsIndex, bool bAdd )
{
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[PartsIndex].empty() ) return;

	if( bAdd ) m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex]++;
	else m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex]--;

	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex] >= (int)m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[PartsIndex].size() ) 
		m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex] = 0;
	if( m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex] < 0 ) 
		m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex] = (int)m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[PartsIndex].size() - 1;

	MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(m_hVecCreateActorList[m_nSelectClassIndex].GetPointer());
	int nTableID = m_VecCreateDefaultPartsList[m_nSelectClassIndex].nVecList[PartsIndex][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex]];

	pPartsBody->DetachParts( PartsIndex );
	DnPartsHandle hParts = m_VecCreateDefaultPartsList[m_nSelectClassIndex].hVecParts[PartsIndex][m_VecCreateDefaultPartsList[m_nSelectClassIndex].nSelectPartsIndex[PartsIndex]];
	pPartsBody->AttachParts( hParts, (CDnParts::PartsTypeEnum)-1, false, false );

	pPartsBody->SetPartsColor( MAPartsBody::HairColor, m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[MAPartsBody::HairColor] );
	pPartsBody->SetPartsColor( MAPartsBody::SkinColor, m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[MAPartsBody::SkinColor] );
	pPartsBody->SetPartsColor( MAPartsBody::EyeColor, m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[MAPartsBody::EyeColor] );

	/*
	if( hParts->GetPartsType() == CDnParts::PartsTypeEnum::Face ) {
		hParts->UpdateEyeColorParam( m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[EyeColor] );
	}

	CDnPartsHair *pPartsHair = dynamic_cast<CDnPartsHair*>(hParts.GetPointer());
	if( pPartsHair ) {	
		pPartsHair->SetColor( m_VecCreateDefaultPartsList[ m_nSelectClassIndex ].dwColor[HairColor] );
		pPartsHair->UpdateColorParam();
	}
	*/
#ifdef PRE_MOD_CREATE_CHAR
	GetInterface().SetCharCreatePartsName( PartsIndex, hParts->GetName() );
#endif // PRE_MOD_CREATE_CHAR
}

void CDnLoginTask::TurnCharacter()
{
	m_hVecCreateActorList[m_nSelectClassIndex]->SetActionQueue( "CreateSelect_Turn" );

#if defined(PRE_ADD_KALI) && !defined(PRE_REMOVE_KALI)
	const char *szCameraName[] = { "Wa_Turn", "Ar_Turn", "So_Turn", "Cl_Turn", "Ac_Turn", "Ka_Turn" };
	const char *szCameraNextName[] = { "Wa_Select_Stay", "Ar_Select_Stay", "So_Select_Stay", "Cl_Select_Stay", "Ac_Select_Stay", "Ka_Select_Stay" };
#elif defined(PRE_ADD_ACADEMIC) && !defined(PRE_REMOVE_ACADEMIC)
	const char *szCameraName[] = { "Wa_Turn", "Ar_Turn", "So_Turn", "Cl_Turn", "Ac_Turn" };
	const char *szCameraNextName[] = { "Wa_Select_Stay", "Ar_Select_Stay", "So_Select_Stay", "Cl_Select_Stay", "Ac_Select_Stay" };
#else
	const char *szCameraName[] = { "Wa_Turn", "Ar_Turn", "So_Turn", "Cl_Turn" };
	const char *szCameraNextName[] = { "Wa_Select_Stay", "Ar_Select_Stay", "So_Select_Stay", "Cl_Select_Stay" };
#endif // #if defined(PRE_ADD_ACADEMIC) && !defined(PRE_REMOVE_ACADEMIC)
	((CDnAniCamera*)m_hCamera.GetPointer())->PlayCamera( szCameraName[m_nSelectClassIndex], szCameraNextName[m_nSelectClassIndex] );
}

void CDnLoginTask::ShowSlotInfoDlg()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if( !pSox ) return;

	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ )
	{
		GetInterface().ShowSlotBlockDlg( !m_bSlotOpen[i], i );

		if( m_bSlotDeleteWait[i] ) {
			GetInterface().ShowSlotDeleteWaitDlg( !m_bProcessCamera, i, m_SlotDelteWaitRemainTime[i] );
		}
		int nMapNameID = 0;

		if( m_nSlotActorMapID[i] > 0 )
			nMapNameID = pSox->GetFieldFromLablePtr( m_nSlotActorMapID[i], "_MapNameID" )->GetInteger();
		else nMapNameID = 1305004;

		if( !m_hSlotActorHandle[i] )
			GetInterface().ShowSlotInfoDlg( false, i, m_hSlotActorHandle[i], GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMapNameID ) );
		else
			GetInterface().ShowSlotInfoDlg( !m_bProcessCamera, i, m_hSlotActorHandle[i], GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMapNameID ) );
	}

	GetInterface().EnableCharSelectDlgControl( m_nEmptyCharSlotCount > 0 ? true : false, "ID_CREATE_CHAR" );
}

void CDnLoginTask::GetCharSelectMousePos( std::vector<EtVector2> &vecPos, float fScreenWidthRatio, float fScreenHeightRatio )
{
	// Uses float screen coordinates by default, so calculate and pass those values.
	std::vector<EtVector3> vecHeadPos;
	for( int i=0; i<SHOWCHARNUMPERONEPAGE; i++ )
	{
		if( m_hSlotActorHandle[i] )
			vecHeadPos.push_back( m_hSlotActorHandle[i]->GetHeadPosition() );
	}

	if( vecHeadPos.empty() )
		return;

	// If there is no camera, just cancel.
	EtMatrix matViewProj;
	DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
	if( hCamera )
	{
		matViewProj = *hCamera->GetCameraHandle()->GetViewProjMat();
	}
	else 
	{
		EtCameraHandle hCam = CEtCamera::GetActiveCamera();
		if( !hCam ) return;
		matViewProj = *CEtCamera::GetActiveCamera()->GetViewProjMat();
	}

	for( int i=0; i<(int)vecHeadPos.size(); i++ )
	{
		EtVector3 vTemp, vPos;
		vTemp = vecHeadPos[i];
		EtVec3TransformCoord( &vPos, &vTemp, &matViewProj );

		if( vPos.z >= 1.0f )
			continue;

		vPos.x = (vPos.x*0.5f) + 0.5f;
		vPos.y = (vPos.y*0.5f) + 0.5f;
		vPos.y = 1.0f - vPos.y;

		// Multiply additionally here so it behaves as if obtained at the dialog level and can all be processed together later.
		vPos.x = vPos.x * fScreenWidthRatio;
		vPos.y = vPos.y * fScreenHeightRatio;

		vecPos.push_back( EtVector2(vPos.x, vPos.y) );
	}
}

static bool CompareServerList( TServerListData *s1, TServerListData *s2 )
{
	// Higher character count first
	if( s1->cMyCharCount < s2->cMyCharCount ) return false;
	else if( s1->cMyCharCount > s2->cMyCharCount ) return true;

	return false;
}

void CDnLoginTask::OnRecvLoginServerList( SCServerList *pPacket )
{
	FUNC_LOG();

	if( pPacket->cServerCount > 0 )
	{
		GetInterface().ClearServerList();
#ifndef _FINAL_BUILD
		for( int i=0; i<pPacket->cServerCount; ++i )
		{
			GetInterface().AddServerList( pPacket->ServerListData[i].cWorldID, pPacket->ServerListData[i].wszServerName,
				float(pPacket->ServerListData[i].nWorldCurUser) / float(pPacket->ServerListData[i].nWorldMaxUser), pPacket->ServerListData[i].cMyCharCount );
		}
#else

		std::vector<TServerListData*> vecServerInfo;
		for( int i=0; i<pPacket->cServerCount; i++ )
		{
			if (pPacket->ServerListData[i].bOnTop == false)
				vecServerInfo.push_back( &pPacket->ServerListData[i] );
			else		//If bOnTop is enabled, put it on top
				GetInterface().AddServerList( pPacket->ServerListData[i].cWorldID, pPacket->ServerListData[i].wszServerName,  
				float(pPacket->ServerListData[i].nWorldCurUser) / float(pPacket->ServerListData[i].nWorldMaxUser), pPacket->ServerListData[i].cMyCharCount );				
		}

		std::sort( vecServerInfo.begin(), vecServerInfo.end(), CompareServerList );
		int nCharCountZeroIndex = -1;
		for( int i=0; i<(int)vecServerInfo.size(); i++ ) {
			if( vecServerInfo[i]->cMyCharCount == 0 ) {
				nCharCountZeroIndex = i;
				break;
			}
		}
		if( nCharCountZeroIndex != -1 )
			std::random_shuffle( vecServerInfo.begin()+nCharCountZeroIndex, vecServerInfo.end() );

		for( int i=0; i<(int)vecServerInfo.size(); i++ )
		{
			GetInterface().AddServerList( vecServerInfo[i]->cWorldID, vecServerInfo[i]->wszServerName,  
				float(vecServerInfo[i]->nWorldCurUser) / float(vecServerInfo[i]->nWorldMaxUser), vecServerInfo[i]->cMyCharCount );
		}
#endif
		GetInterface().SetDefaultServerList();

#ifndef _FINAL_BUILD
		if( CGlobalValue::GetInstance().IsPermitLogin() )
		{
			SendSelectedServer( (BYTE)GetInterface().GetServerIndex( CGlobalValue::GetInstance().m_szServerName.c_str() ) );
		}
#endif
	}
	else
	{
		CDebugSet::ToLogFile( "CDnLoginTask::OnRecvLoginCheckServer, Server Count is 0!" );
	}
	GetInterface().ShowServerList( true );
}

void CDnLoginTask::OnRecvSystemGameInfo( SCGameInfo *pPacket )
{
	if( pPacket->nRet != ERROR_NONE ) {
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::GameInfoFailed, this);
		return;
	}
}

void CDnLoginTask::OnRecvSystemVillageInfo( SCVillageInfo *pPacket )
{
	if( pPacket->nRet != ERROR_NONE ) 
	{
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK, UICallbackEnum::VillageInfoFailed, this);
		return;
	}

	// g_pServiceSetup->WriteStageLog_( IServiceSetup::NHN_StartGmae , _T("StartGmae"));	
}

void CDnLoginTask::OnRecvLoginChannelList( SCChannelList *pPacket )
{
	FUNC_LOG();

	WCHAR wzStrTmp[1024]={0,};

	switch(pPacket->nRet)
	{
	case ERROR_NONE:
		break;
	case ERROR_SECONDAUTH_CHECK_MAXFAILED:
		swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6305) , pPacket->cFailCount );
		GetInterface().MessageBox( wzStrTmp );	
		// Temporarily force-switch. On ChangeState, to prevent duplicate Back button when Prev is ChannelList
		// Check code is included; replace it if it looks ugly.
		m_State = CDnLoginTask::CharSelect; 
		ChangeState( CDnLoginTask::CharSelect, false );
		return;	
	case ERROR_SECONDAUTH_CHECK_OLDPWINVALID:
	case ERROR_SECONDAUTH_CHECK_FAILED: 
	default:
		
		swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6303) , pPacket->cFailCount , SecondAuth::Common::LimitCount );
		GetInterface().MessageBox( wzStrTmp ) ;	
		m_State = CDnLoginTask::CharSelect;
		ChangeState( CDnLoginTask::CharSelect, false );
		return;	
	}

	GetInterface().ShowChannelList(true);

#ifndef _FINAL_BUILD
	if( CGlobalValue::GetInstance().IsPermitLogin() )
	{
		for( int i=0; i<pPacket->cCount; i++ )
		{
			if( (i+1) == (BYTE)CGlobalValue::GetInstance().m_nChannelIndex )
			{
				m_nSelectedChannelIndex = pPacket->Info->nChannelID;
				ChangeState( ChannelList );
				return;
			}
			else
			{
				CDebugSet::ToLogFile( "CDnLoginTask::OnRecvLoginChannelList, Invalid Channel Index(%d)!", (BYTE)CGlobalValue::GetInstance().m_nChannelIndex );
			}
		}

		return;
	}
#endif

	std::vector<sChannelInfo*> vecChannelInfo;

	for( int i=0; i<pPacket->cCount; i++ )
	{
		vecChannelInfo.push_back( &pPacket->Info[i] );
	}

	std::stable_sort( vecChannelInfo.begin(), vecChannelInfo.end(), CVillageClientSession::CompareChannelInfo );		// Sort by best connection/comfort.

	for( int i=0; i<(int)vecChannelInfo.size(); i++ )
	{
		GetInterface().AddChannelList( vecChannelInfo[i] );
	}

	GetInterface().SetDefaultChannelList();
}

void CDnLoginTask::OnRecvLoginBackButton()
{
	CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( pTask )
	{
		pTask->ChangeState( CDnLoginTask::RequestIDPass );
	}

}

void CDnLoginTask::OnRecvLoginReviveChar( SCReviveChar *pPacket )
{
	if( pPacket->nRet != 0 ) {
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1210 ), MB_OK, UICallbackEnum::CharReviveFailed );
		return;
	}
	SelectServer( m_nSelectedServerIndex );
}


void CDnLoginTask::SelectServer( int nServerIndex )
{
	m_nSelectedServerIndex = nServerIndex;
	SendSelectedServer( (BYTE)m_nSelectedServerIndex );
	SetRequestWait( true );
}

void CDnLoginTask::SelectChannel( int nChannelIndex )
{
	m_nSelectedChannelIndex = nChannelIndex;
	ChangeState( StartGame );
	//Initial game start
}

void CDnLoginTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	switch( nID )  {
		case UICallbackEnum::ConnectLoginFailed:
		case UICallbackEnum::CheckVersionFailed:
		case UICallbackEnum::DisconnectTCP:
			if( nCommand == EVENT_BUTTON_CLICKED ) {
				if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
				{
					if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );
					CTaskManager::GetInstance().RemoveAllTask( false );
				}
			}
			break;
		case UICallbackEnum::CharListFailed:
		case UICallbackEnum::CharCreateFailed:
			break;
		case UICallbackEnum::CharDeleteFailed:
			{
				SendBackButton();
				ChangeState(LoginStateEnum::ServerList);
			}
			break;
		case UICallbackEnum::CheckLoginFailed:
			break;
		case UICallbackEnum::GameInfoFailed:
		case UICallbackEnum::VillageInfoFailed:
			ChangeState( LoginStateEnum::CharSelect );
			break;
		case UICallbackEnum::CharReviveFailed:
			SelectServer( m_nSelectedServerIndex );
			break;

		case FADE_DIALOG:
			if( nCommand == EVENT_FADE_COMPLETE )
			{
				switch( m_State ) {
					case LoginStateEnum::Logo:
						GetInterface().ChangeState( LoginStateEnum::RequestIDPass );
						break;
					case LoginStateEnum::ServerList:
						if( m_nSelectedServerIndex == -1 ) break;
						GetInterface().FadeDialog( 0, 0x00FFFFFF, 1.f, this );
						ChangeState( CharSelect );
						m_bProcessCamera = false;
						break;
					case LoginStateEnum::CharSelect:
						GetInterface().GetInstance().ChangeState( CharSelect );
						for( int i=0; i<4; i++ ) {
							ShowSlotInfoDlg(i);
						}
						break;
				}
			}
			break;

#ifdef PRE_ADD_COMEBACK
		case UICallbackEnum::ComeBack:
			{
				if( nCommand == EVENT_BUTTON_CLICKED ) {
					if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
					{
						GetInterface().OpenSecurityCheckDlg(CDnInterface::InterfaceTypeEnum::Login , 0 );
					}
				}
			}
			break;
#endif
	}
}

bool CDnLoginTask::IsEmptySelectActorSlot()
{
	return m_nCharSlotCount == m_nEmptyCharSlotCount ? true : false;
}

int CDnLoginTask::GetCurrentSelectLevel()
{
	if( m_nSelectIndex < 0 || m_nSelectIndex >= SHOWCHARNUMPERONEPAGE ) {
		return -1;
	}

	if( m_hSlotActorHandle[m_nSelectIndex] ) {
		return m_hSlotActorHandle[m_nSelectIndex]->GetLevel();
	}

	return -1;
}

void CDnLoginTask::ReduceFarDOFBlurSize()		// Hardcode fix for character selection screen only (login map itself wants a large Far Blur Size)  
{							
	if( !m_pWorld ) return;
	// To fix excessive blur on the head during character selection)
	int nDOFIndex = m_pWorld->GetEnvironment()->GetDOFEffectIndex();
	if( nDOFIndex != -1 ) {
		CDnCEDof *pEffect = (CDnCEDof *)m_hCamera->FindCameraEffect( nDOFIndex );
		if( pEffect ) {
			if( m_fOldDOFNearBlurSize == 0.f && m_fOldDOFFarBlurSize == 0.f ) {
				pEffect->GetBlurSize( &m_fOldDOFNearBlurSize, &m_fOldDOFFarBlurSize );
			}
			pEffect->SetBlurSize( EtMin(2.0f, m_fOldDOFNearBlurSize), EtMax(1.75f, m_fOldDOFFarBlurSize * 0.4f) );
		}
	}
}

void CDnLoginTask::RestoreFarDOFBlurSize()
{
	if( !m_pWorld ) return;
	int nDOFIndex = m_pWorld->GetEnvironment()->GetDOFEffectIndex();
	if( nDOFIndex != -1 && m_fOldDOFNearBlurSize != 0.f && m_fOldDOFNearBlurSize != 0.f ) {
		CDnCEDof *pEffect = (CDnCEDof *)m_hCamera->FindCameraEffect( nDOFIndex );
		if( pEffect ) {
			pEffect->SetBlurSize( m_fOldDOFNearBlurSize, m_fOldDOFFarBlurSize);	// Restore previous value.
		}
	}
}

CEtOutlineFilter * CDnLoginTask::GetOutlineFilter()
{
	if( m_pWorld && m_pWorld->GetEnvironment() ) { 
		CEtOutlineFilter *pOutlineFilter = m_pWorld->GetEnvironment()->GetOutlineFilter();
		return pOutlineFilter;
	}
	return NULL;
}

void CDnLoginTask::ShowSlotInfoDlg( int nSlotIndex )
{
	if( !m_hSlotActorHandle[nSlotIndex] ) {
		GetInterface().ShowSlotInfoDlg( false, nSlotIndex, CDnActor::Identity(), L"" );
		GetInterface().ShowSlotDeleteWaitDlg( false, nSlotIndex, 0 );
	}
	else {
		if( !m_bSlotDeleteWait[nSlotIndex] ) {
			GetInterface().ShowSlotDeleteWaitDlg( false, nSlotIndex, 0 );
		}
	}

	if( !m_bSlotOpen[nSlotIndex] )
		GetInterface().ShowSlotBlockDlg( true, nSlotIndex );
}

void CDnLoginTask::ReviveCharacter( int nSlotIndex )
{
	m_nDeleteCharPage = m_nCurrentCharSelectPage;
	nSlotIndex = nSlotIndex + ( ( m_nCurrentCharSelectPage - 1 ) * SHOWCHARNUMPERONEPAGE );

	SendReviveDeleteChar( nSlotIndex );
}

bool CDnLoginTask::OnCloseTask()
{
	if( !CDnInterface::IsActive() ) return false;

	GetInterface().MessageBox( MESSAGEBOX_21, MB_YESNO, MESSAGEBOX_21, CDnInterface::GetInstancePtr() );
	return true;
}
#endif // PRE_MOD_SELECT_CHAR

#ifdef PRE_ADD_DOORS
void CDnLoginTask::OnRecvDoorsAuthKey(SCDoorsGetAuthKey* pPacket)
{
	if (pPacket == NULL)
	{
		_ASSERT(0);
		return;
	}

	GetInterface().OnSetDoorsAuthMobileMsgBox(*pPacket);
}

void CDnLoginTask::OnRecvDoorsCancelAuth(SCDoorsCancelAuth* pPacket)
{
	if (pPacket == NULL)
	{
		_ASSERT(0);
		return;
	}

	GetInterface().OnSetDoorsCancelAuthMobilMsgBox(*pPacket);
}

void CDnLoginTask::OnRecvDoorsAuthFlag(SCDoorsGetAuthFlag* pPacket)
{
	if (pPacket == NULL)
	{
		_ASSERT(0);
		return;
	}

	GetInterface().OnSetDoorsAuthFlag(*pPacket);
}
#endif // PRE_ADD_DOORS

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
void CDnLoginTask::ShowPreviewCashCostume(int nRadioBtnIndex)
{
	CDnPlayerActor*	pPlayer = dynamic_cast<CDnPlayerActor*>( m_hSlotActorHandle.GetPointer() );
	if( pPlayer == NULL )
		return;

	if( pPlayer->IsGhost() )
		return;

	switch(nRadioBtnIndex)
	{
	case PREVIEW_CASH_COSTUME_RESET:
		{
			ResetPreviewCashCostume();
		}
		break;
	case PREVIEW_CASH_COSTUME_ATTACH:
		{
			AttachPreviewCashCostume(nRadioBtnIndex);
		}
		break;
	}

#ifdef PRE_ADD_CASH_COSTUME_AURA
	if( !pPlayer->ComputeMyEquipCostumeAura() )
		pPlayer->LoadCostumeAuraEffect(false);
#endif
}

void CDnLoginTask::ResetPreviewCashCostume()
{
	if(m_vecCharSelectDlgPreviewCostumePartsList.empty() || m_vecCharListData.empty())
		InitPreviewCashCostumeList();

	DetachPreviewCashCostume(); // Unequip HOT costume.

	if( m_nSelectCharIndex < 0 || m_nSelectCharIndex >= static_cast<int>( m_vecCharListData.size() ) )
		return;

	std::vector<TCharListData>::iterator iter = m_vecCharListData.begin();
	iter += m_nSelectCharIndex;

	MAPartsBody*	pPartsBody	= dynamic_cast<MAPartsBody *>(m_hSlotActorHandle.GetPointer());
	CDnPlayerActor*	pPlayer		= dynamic_cast<CDnPlayerActor*>(m_hSlotActorHandle.GetPointer());
	
	if( !pPartsBody || !pPlayer ) return;

	int nDefaultPartsInfo[4] = { (*iter).nDefaultBody, (*iter).nDefaultLeg, (*iter).nDefaultHand, (*iter).nDefaultFoot };
	pPlayer->SetDefaultPartsInfo( nDefaultPartsInfo );

	//------------------------------------------------------------
	// Equip Default gear
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDEFAULTCREATE );
	static char *szLabel[CDnParts::DefaultPartsTypeEnum_Amount] = { "_DefaultBody", "_DefaultLeg", "_DefaultHand", "_DefaultFoot" };
	if( CTaskManager::GetInstance().GetTask( "LoginTask" ) ) 
	{
		for( int i=0; i<CDnParts::DefaultPartsTypeEnum_Amount; i++ ) 
		{
			if( pPlayer->GetParts( (CDnParts::PartsTypeEnum)(CDnParts::Body + i )) ) 
				continue;

			if( pSox->IsExistItem( pPlayer->GetClassID() ) == false ) 
				continue;

			int nItemID = pSox->GetFieldFromLablePtr( pPlayer->GetClassID() , szLabel[i] )->GetInteger();
			if( nItemID < 1 ) 
				continue;

			DnPartsHandle hParts = CDnParts::CreateParts( nItemID, -1 );
			pPartsBody->AttachParts( hParts, (CDnParts::PartsTypeEnum)(CDnParts::Body + i ), true, false );
		}
	}

	//------------------------------------------------------------
	// Equip normal parts.
	int *pEquipArray = (*iter).nEquipArray;
	for( int j=EQUIP_FACE; j<=EQUIP_RING2; j++ )
	{
		if( pEquipArray[j] < 1 )
		{
			//pPlayer->DetachParts( (CDnParts::PartsTypeEnum)j );
			continue;
		}
		DnPartsHandle hParts = CDnParts::CreateParts( pEquipArray[j], 0 );
		pPlayer->AttachParts( hParts, (CDnParts::PartsTypeEnum)j, true, false );

		if( pPlayer->GetObjectHandle() && pPlayer->GetObjectHandle()->GetSkinInstance() )
			pPlayer->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
	}
	pPlayer->SetPartsColor( MAPartsBody::HairColor, (*iter).dwHairColor );
	pPlayer->SetPartsColor( MAPartsBody::EyeColor, (*iter).dwEyeColor );
	pPlayer->SetPartsColor( MAPartsBody::SkinColor, (*iter).dwSkinColor );
	pPlayer->SetBattleMode( false );


	//------------------------------------------------------------
	// Equip normal weapon
	for( int j=0; j<2; j++ )
	{
		if( pEquipArray[EQUIP_WEAPON1 + j] == 0 ) continue;
		DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( pEquipArray[EQUIP_WEAPON1 + j], 0 );
		pPlayer->AttachWeapon( hWeapon, j, true );
	}

	
	//------------------------------------------------------------
	// Equip cash Parts
	int *pCashEquipArray = (*iter).nCashEquipArray;
	for( int j=CASHEQUIP_HELMET; j<=CASHEQUIP_FAIRY; j++ )
	{
		if( pCashEquipArray[j] < 1 ) 
			continue;

		// Since login does not call CombineParts, remove normal items here.
		if( j >= CASHEQUIP_HELMET && j <= CASHEQUIP_FOOT )
		{
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
			if( pPlayer->GetParts( (CDnParts::PartsTypeEnum)( j + 2 ) ) && !CDnParts::IsTransparentSkin( pCashEquipArray[j], pPlayer->GetClassID() ) )
#else
			if( pPlayer->GetParts( (CDnParts::PartsTypeEnum)( j + 2 ) ) )
#endif
				pPlayer->DetachParts( (CDnParts::PartsTypeEnum)( j + 2 ) );
		}
		DnPartsHandle hParts = CDnParts::CreateParts( pCashEquipArray[j], 0 );

		pPlayer->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)j, true, false );
		if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
			hParts->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
	}

	//------------------------------------------------------------
	// Equip cash weapon
	for( int j=0; j<2; j++ )
	{
		if( pCashEquipArray[CASHEQUIP_WEAPON1 + j] == 0 ) continue;
		DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( pCashEquipArray[CASHEQUIP_WEAPON1 + j], 0 );
		pPlayer->AttachCashWeapon( hWeapon, j, true, false );
	}

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
	// Since login does not call CombineParts, forcibly call hiding right glove mesh when warrior equips gauntlet.
	pPlayer->OnCombineParts();
#endif
}

void CDnLoginTask::AttachPreviewCashCostume(int nRadioBtnIndex)
{
	InitPreviewCashCostumeList();
	DetachPreviewCashCostume();

	if( nRadioBtnIndex > 0 && nRadioBtnIndex <= static_cast<int>( m_vecCharSelectDlgPreviewCostumePartsList.size() ) )
	{
		int nPartsList = nRadioBtnIndex - 1;

		MAPartsBody* pPartsBody = dynamic_cast<MAPartsBody *>(m_hSlotActorHandle.GetPointer());
		if( pPartsBody == NULL )
			return;

		CDnPlayerActor*	pPlayer;
		pPlayer = dynamic_cast<CDnPlayerActor*>( m_hSlotActorHandle.GetPointer() );
		if( pPlayer == NULL )
			return;

		for( int i=CASHEQUIP_HELMET; i<=CASHEQUIP_FAIRY; i++ )
		{
			if( m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hVecParts[i].size() > 0)
			{
				DnPartsHandle hParts = m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hVecParts[i][0];
				if( hParts )
				{
					pPartsBody->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)i, false, false );
				}
			}
		}

#ifdef PRE_FIX_WEAPON_DUPLICATE_RENDER
		if( m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hDeafaultWeapon[0] )
		{
			m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hDeafaultWeapon[0]->ShowRenderBase( true );
			pPlayer->CDnPlayerActor::AttachWeapon( m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hDeafaultWeapon[0], 0, false );
		}

		if( m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hDeafaultWeapon[1] )
		{
			m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hDeafaultWeapon[1]->ShowRenderBase( true );
			pPlayer->CDnPlayerActor::AttachWeapon( m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hDeafaultWeapon[1], 1, false );
		}
#else // PRE_FIX_WEAPON_DUPLICATE_RENDER
		if( m_VecCreateDefaultPartsList[nPartsList].hWeapon[0] )
		{
			m_VecCreateDefaultPartsList[nPartsList].hWeapon[0]->ShowRenderBase( true );
			pPlayer->CDnPlayerActor::AttachWeapon( m_VecCreateDefaultPartsList[nPartsList].hWeapon[0], 0, false );
		}

		if( m_VecCreateDefaultPartsList[nPartsList].hWeapon[1] )
		{
			m_VecCreateDefaultPartsList[nPartsList].hWeapon[1]->ShowRenderBase( true );
			pPlayer->CDnPlayerActor::AttachWeapon( m_VecCreateDefaultPartsList[nPartsList].hWeapon[1], 1, false );
		}
#endif // PRE_FIX_WEAPON_DUPLICATE_RENDER
		if( m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hWeapon[0] )
		{
			m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hWeapon[0]->ShowRenderBase( true );
			pPlayer->CDnPlayerActor::AttachCashWeapon( m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hWeapon[0], 0, false );
		}

		if( m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hWeapon[1] )
		{
			m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hWeapon[1]->ShowRenderBase( true );
			pPlayer->CDnPlayerActor::AttachCashWeapon( m_vecCharSelectDlgPreviewCostumePartsList[nPartsList].hWeapon[1], 1, false );
		}

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
		// Since login does not call CombineParts, forcibly call hiding right glove mesh when warrior equips gauntlet.
		pPlayer->OnCombineParts();
#endif
	}
}

void CDnLoginTask::DetachPreviewCashCostume()
{
	MAPartsBody* pPartsBody = dynamic_cast<MAPartsBody*>(m_hSlotActorHandle.GetPointer());
	if(!pPartsBody)
		return;
	
	for( int i=EQUIP_BODY; i<EQUIP_NECKLACE; i++ )
	{
		pPartsBody->DetachParts( (CDnParts::PartsTypeEnum)i );
	}

	for( int i=CASHEQUIP_HELMET; i<=CASHEQUIP_FAIRY; i++ )
	{
		if( m_vecCharSelectDlgPreviewCostumePartsList[0].hVecParts[i].size() > 0)
		{
			DnPartsHandle hParts = m_vecCharSelectDlgPreviewCostumePartsList[0].hVecParts[i][0];
			if( hParts )
			{
				pPartsBody->DetachCashParts((CDnParts::PartsTypeEnum)i);
			}
		}
	}

	((CDnPlayerActor*)m_hSlotActorHandle.GetPointer())->CDnPlayerActor::DetachWeapon( 0 );
	((CDnPlayerActor*)m_hSlotActorHandle.GetPointer())->CDnPlayerActor::DetachWeapon( 1 );

	((CDnPlayerActor*)m_hSlotActorHandle.GetPointer())->CDnPlayerActor::DetachCashWeapon( 0 );
	((CDnPlayerActor*)m_hSlotActorHandle.GetPointer())->CDnPlayerActor::DetachCashWeapon( 1 );
}

void CDnLoginTask::InitPreviewCashCostumeList()
{
	ResetPreviewCashCostumeList();

	if( m_nSelectClassIndex < 0 )
		return;

	DNTableFileFormat* pDefaultCreateTable = GetDNTable( CDnTableDB::TDEFAULTCREATE );
	if( pDefaultCreateTable == NULL )
		return;

	int nActorCount = pDefaultCreateTable->GetItemCount();
	int nClassID = 0;
	std::vector<int> vecCostumePreviewID;
#ifdef PRE_FIX_WEAPON_DUPLICATE_RENDER
	int nDefaultWeaponIndex[2] = {0,};
#endif // PRE_FIX_WEAPON_DUPLICATE_RENDER

	for( int i=1; i<=nActorCount; i++ ) 
	{
		nClassID = pDefaultCreateTable->GetFieldFromLablePtr( i, "_ClassID" )->GetInteger();
		if( nClassID == m_nSelectClassIndex + 1  )
		{
			vecCostumePreviewID.push_back( pDefaultCreateTable->GetFieldFromLablePtr( i, "_CostumePreviewID1" )->GetInteger() );
#ifdef PRE_FIX_WEAPON_DUPLICATE_RENDER
			char szLabel[64];
			for( int j=0; j<2; j++ ) 
			{
				sprintf_s( szLabel, "_Weapon%d", j + 1 );
				nDefaultWeaponIndex[j] = pDefaultCreateTable->GetFieldFromLablePtr( i, szLabel )->GetInteger();
			}
#endif // PRE_FIX_WEAPON_DUPLICATE_RENDER
			break;
		}
	}	

	DNTableFileFormat* pDefaultCreateCostumeTable = GetDNTable( CDnTableDB::TDEFAULTCREATECOSTUME );
	if( pDefaultCreateCostumeTable == NULL )
		return;

	for( int i=0; i<static_cast<int>( vecCostumePreviewID.size() ); i++ )
	{
		DefaultPartsStruct sPreviewCostume;

		int nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Helmet" )->GetInteger();
		sPreviewCostume.nVecList[0].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[0].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );

		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Body" )->GetInteger();
		sPreviewCostume.nVecList[1].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[1].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );

		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Leg" )->GetInteger();
		sPreviewCostume.nVecList[2].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[2].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );

		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Hand" )->GetInteger();
		sPreviewCostume.nVecList[3].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[3].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );

		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Foot" )->GetInteger();
		sPreviewCostume.nVecList[4].push_back( nCostumeIndex );
		sPreviewCostume.hVecParts[4].push_back( CDnParts::CreateParts( nCostumeIndex, 0 ) );


		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Weapon" )->GetInteger();
		sPreviewCostume.hWeapon[0] = CDnWeapon::CreateWeapon( nCostumeIndex, 0 );
		nCostumeIndex = pDefaultCreateCostumeTable->GetFieldFromLablePtr( vecCostumePreviewID[i], "_Weapon1" )->GetInteger();
		sPreviewCostume.hWeapon[1] = CDnWeapon::CreateWeapon( nCostumeIndex, 1 );
#ifdef PRE_FIX_WEAPON_DUPLICATE_RENDER
		sPreviewCostume.hDeafaultWeapon[0] = CDnWeapon::CreateWeapon( nDefaultWeaponIndex[0], 0 );
		sPreviewCostume.hDeafaultWeapon[1] = CDnWeapon::CreateWeapon( nDefaultWeaponIndex[1], 0 );
#endif // PRE_FIX_WEAPON_DUPLICATE_RENDER

		m_vecCharSelectDlgPreviewCostumePartsList.push_back( sPreviewCostume );
	}
}

void CDnLoginTask::ResetPreviewCashCostumeList()
{
	for( DWORD i=0; i<m_vecCharSelectDlgPreviewCostumePartsList.size(); i++ ) 
	{
		for( int j=0; j < EQUIPMAX; j++ ) 
		{
			SAFE_RELEASE_SPTRVEC( m_vecCharSelectDlgPreviewCostumePartsList[i].hVecParts[j] );
			SAFE_DELETE_VEC( m_vecCharSelectDlgPreviewCostumePartsList[i].nVecList[j] );
		}
		SAFE_RELEASE_SPTR( m_vecCharSelectDlgPreviewCostumePartsList[i].hWeapon[0] );
		SAFE_RELEASE_SPTR( m_vecCharSelectDlgPreviewCostumePartsList[i].hWeapon[1] );
#ifdef PRE_FIX_WEAPON_DUPLICATE_RENDER
		SAFE_RELEASE_SPTR( m_vecCharSelectDlgPreviewCostumePartsList[i].hDeafaultWeapon[0] );
		SAFE_RELEASE_SPTR( m_vecCharSelectDlgPreviewCostumePartsList[i].hDeafaultWeapon[1] );
#endif // PRE_FIX_WEAPON_DUPLICATE_RENDER
	}
	SAFE_DELETE_VEC( m_vecCharSelectDlgPreviewCostumePartsList );
}

#endif

#ifdef PRE_CHARLIST_SORTING

bool CDnLoginTask::SortByCreate( const TCharListData& a, const TCharListData& b )
{
	return ( a.CreateDate < b.CreateDate ) ? true : false;
}

bool CDnLoginTask::SortByName( const TCharListData& a, const TCharListData& b )
{
	return (0 > wcscmp( a.wszCharacterName, b.wszCharacterName ) );
}

bool CDnLoginTask::SortByLevel( const TCharListData& a, const TCharListData& b )
{
	return ( a.cLevel > b.cLevel ) ? true : false;
}

bool CDnLoginTask::SortByJob( const TCharListData& a, const TCharListData& b )
{
	return ( a.cJob < b.cJob ) ? true : false;
}

bool CDnLoginTask::SortByLatelyLogin( const TCharListData& a, const TCharListData& b )
{
	return ( a.LastLoginDate > b.LastLoginDate ) ? true : false;
}

void CDnLoginTask::SortCharacterList( eCharacterListSortCommand sortCommand )
{
	m_vecCharListData.clear();
	for( int i=0; i<static_cast<int>( m_vecOriginCharListData.size() ); i++ )
	{
		m_vecCharListData.push_back( m_vecOriginCharListData[i] );
	}

	switch( sortCommand )
	{
		case CHARACTERLIST_SORT_BY_SERVER:
			{
				std::vector<TCharListData> vecSelectedServerCharListData;
				for( int i=0; i<static_cast<int>( m_vecCharListData.size() ); i++ )
				{
					if( m_vecCharListData[i].cWorldID == CGlobalInfo::GetInstance().m_nSelectedServerIndex )
					{		
						vecSelectedServerCharListData.push_back( m_vecCharListData[i] );
						m_vecCharListData.erase( m_vecCharListData.begin() + i );
						i--;
					}
				}
				std::sort( m_vecCharListData.begin(), m_vecCharListData.end(), SortByServerIndex );
				std::vector<TCharListData>::iterator iter = vecSelectedServerCharListData.begin();
				for( ; iter != vecSelectedServerCharListData.end(); iter++ )
				{
					m_vecCharListData.insert( m_vecCharListData.begin(), *iter );
				}
				vecSelectedServerCharListData.clear();
			}
			break;
		case CHARACTERLIST_SORT_BY_CREATE:
			std::sort( m_vecCharListData.begin(), m_vecCharListData.end(), SortByCreate );
			break;
		case CHARACTERLIST_SORT_BY_NAME:
			std::sort( m_vecCharListData.begin(), m_vecCharListData.end(), SortByName );
			break;
		case CHARACTERLIST_SORT_BY_LEVEL:
			std::sort( m_vecCharListData.begin(), m_vecCharListData.end(), SortByLevel );
			break;
		case CHARACTERLIST_SORT_BY_JOB:
			std::sort( m_vecCharListData.begin(), m_vecCharListData.end(), SortByJob );
			break;
		case CHARACTERLIST_SORT_BY_LATELYLOGIN:
			std::sort( m_vecCharListData.begin(), m_vecCharListData.end(), SortByLatelyLogin );
			break;
	}

	CDnCharSelectDlg* pCharSelectDlg = GetInterface().GetCharSelectDlg();
	if( pCharSelectDlg )
	{
		pCharSelectDlg->SetCharacterList( m_vecCharListData, m_nCharSlotMaxCount );
		if( m_nCharSlotCount > 0 && m_eCharacterListSortCommand != sortCommand )
		{
			m_nSelectCharIndex = -1;
			pCharSelectDlg->SelectCharIndex( 0 );
		}
	}

	m_eCharacterListSortCommand = sortCommand;
}
#endif // PRE_CHARLIST_SORTING