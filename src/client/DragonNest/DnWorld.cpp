#include "StdAfx.h"
#include "DnWorld.h"
#include "DnWorldGrid.h"
#include "DnCamera.h"
#include "DnWorldSector.h"
#include "DnWorldProp.h"
#include "DnActor.h"
#include "PerfCheck.h"
#include "DnPlayerCamera.h"
#include "DnTableDB.h"
#include "DnWorldSound.h"
#include "DnWorldActProp.h"
#include "EtWorldEventArea.h"
#include "DnTrigger.h"
#include "DnLoadingTask.h"
#include "navigationmesh.h"
#ifdef PRE_FIX_MEMOPT_EXT
#include "DnCommonUtil.h"
#endif
#include "EtWorldEventArea.h"
#include "EtWorldEventControl.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 


CDnWorld::WeatherEnum CDnWorld::s_WeatherDefine[WeatherEnum_Amount][5] = {
	{ CDnWorld::FineDay, CDnWorld::CloudyDay, CDnWorld::FineGlow, CDnWorld::CloudyGlow, (CDnWorld::WeatherEnum)-1 },
	{ CDnWorld::CloudyGlow, CDnWorld::FineNight, CDnWorld::CloudyNight, (CDnWorld::WeatherEnum)-1 },
	{ CDnWorld::CloudyNight, CDnWorld::FineDawn, CDnWorld::CloudyDawn, (CDnWorld::WeatherEnum)-1 },
	{ CDnWorld::FineDay, CDnWorld::CloudyDay, (CDnWorld::WeatherEnum)-1 },
	{ CDnWorld::FineDay, CDnWorld::CloudyDay, CDnWorld::RainDay, CDnWorld::HeavyrainDay, CDnWorld::CloudyGlow },
	{ CDnWorld::FineNight, CDnWorld::CloudyNight, CDnWorld::RainNight, CDnWorld::HeavyrainNight, (CDnWorld::WeatherEnum)-1 },
	{ CDnWorld::RainNight, CDnWorld::HeavyrainNight, CDnWorld::FineDawn, CDnWorld::CloudyDawn, (CDnWorld::WeatherEnum)-1 },
	{ CDnWorld::FineDay, CDnWorld::CloudyDay, CDnWorld::RainDay, (CDnWorld::WeatherEnum)-1 },
	{ CDnWorld::FineDay, CDnWorld::CloudyDay, CDnWorld::RainDay, CDnWorld::HeavyrainDay, CDnWorld::CloudyGlow },
	{ CDnWorld::CloudyNight, CDnWorld::HeavyrainNight, CDnWorld::CloudyDawn, (CDnWorld::WeatherEnum)-1 },
	{ CDnWorld::CloudyDay, CDnWorld::RainDay, CDnWorld::HeavyrainDay, CDnWorld::CloudyGlow, (CDnWorld::WeatherEnum)-1 },
	{ CDnWorld::RainNight, CDnWorld::CloudyDawn, (CDnWorld::WeatherEnum)-1 },
};

CDnWorld g_DnWorld;

CDnWorld::CDnWorld()
{
	m_LandEnvironment = (CDnWorld::LandEnvironment)-1;
	m_CurrentWeather = WeatherEnum::FineDay;
	m_pCurrentTask = NULL;

	m_fPropCollisionDistance = 0.f;
//	m_dwPropUniqueCount = 0;
	m_MapType = MapTypeUnknown;
	m_MapSubType = MapSubTypeNone;
	m_CurrentBattleBGM = BattleBGMEnum::NoneBattle;
	m_DefaultBGM.fCurrentVolume = 1.f;
	m_bApplyWaterEnvironment = true;
	m_bProcessBattleBGM = true;
	m_bShow = true;
	m_iAllowMapType = 0;

	// World default settings [2011/01/12 semozz]
	m_DragonNestType = eDragonNestType::None;


//#ifdef PRE_ADD_FILTEREVENT
	m_bMonochrome = false;
	m_vFilterEventColor = EtVector3(1.0f,1.0f,1.0f);
	m_fFilterEventVolume = 1.0f;
//#endif PRE_ADD_FILTEREVENT

}

CDnWorld::~CDnWorld()
{
}

bool CDnWorld::Initialize( const char *szWorldFolder, const char *szGridName, CTask *pTask, bool bPreLoad )
{
	GetEtDevice()->EvictManagedResources();		// Release MANAGED resources in video memory on map transition.

	m_pCurrentTask = pTask;

	char szPath[_MAX_PATH];

#ifndef _FINAL_BUILD
	if( !CGlobalValue::GetInstance().m_szNation.empty() ) {
		char szNationPath[_MAX_PATH];
		sprintf_s( szNationPath, "%s_%s", szWorldFolder, CGlobalValue::GetInstance().m_szNation.c_str() );

		sprintf_s( szPath, "%s\\Resource\\Tile", szNationPath );
		CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

		sprintf_s( szPath, "%s\\Resource\\Prop", szNationPath );
		CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

		sprintf_s( szPath, "%s\\Resource\\Sound", szNationPath );
		CEtResourceMng::GetInstance().AddResourcePath( szPath, true );
	}
#endif //_FINAL_BUILD
	sprintf_s( szPath, "%s\\Resource\\Tile", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	sprintf_s( szPath, "%s\\Resource\\Prop", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	sprintf_s( szPath, "%s\\Resource\\Sound", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	sprintf_s( szPath, "%s\\Resource\\Envi", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath );

	sprintf_s( szPath, "%s\\Resource\\Trigger", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

//	m_dwPropUniqueCount = 0;	

#ifndef _FINAL_BUILD
	if( !CGlobalValue::GetInstance().m_szNation.empty() ) {
		char szNationPath[_MAX_PATH];
		sprintf_s( szNationPath, "%s_%s\\Grid\\%s", szWorldFolder, CGlobalValue::GetInstance().m_szNation.c_str(), szGridName );
		if( PathFileExistsA( szNationPath ) ) {
			sprintf_s( szNationPath, "%s_%s", szWorldFolder, CGlobalValue::GetInstance().m_szNation.c_str() );
			bool bResult = CEtWorld::Initialize( szNationPath, szGridName );
			if( bResult && bPreLoad ) ForceInitialize();
			return bResult;
		}
	}
#endif //_FINAL_BUILD

	bool bResult = CEtWorld::Initialize( szWorldFolder, szGridName );
	if( bResult && bPreLoad ) bResult = ForceInitialize();
	return bResult;
}

void CDnWorld::Destroy()
{
	if ( CEtResourceMng::IsActive() )
	{
		char szPath[_MAX_PATH];

		sprintf_s( szPath, "%s\\Resource\\Tile", m_szWorldFolder.c_str() );
		CEtResourceMng::GetInstance().RemoveResourcePath( szPath, true );

		sprintf_s( szPath, "%s\\Resource\\Prop", m_szWorldFolder.c_str() );
		CEtResourceMng::GetInstance().RemoveResourcePath( szPath, true );

		sprintf_s( szPath, "%s\\Resource\\Envi", m_szWorldFolder.c_str() );
		CEtResourceMng::GetInstance().RemoveResourcePath( szPath, true );

		sprintf_s( szPath, "%s\\Resource\\Trigger", m_szWorldFolder.c_str() );
		CEtResourceMng::GetInstance().RemoveResourcePath( szPath, true );
	}
	m_pCurrentTask = NULL;

	m_Environment.Finalize();

	SAFE_DELETE_VEC( m_VecVisibleOnCheckList );
	SAFE_DELETE_VEC( m_VecVisibleOffCheckList );
	SAFE_DELETE_PVEC( m_pVecGateList );

	for( int i=0; i<BattleBGMEnum_Amount; i++ ) {
		if( m_BattleBGM[i].nSoundIndex != -1 ) {
			CEtSoundEngine::GetInstance().RemoveChannel( m_BattleBGM[i].hChannel );
			CEtSoundEngine::GetInstance().RemoveSound( m_BattleBGM[i].nSoundIndex );
			m_BattleBGM[i].nSoundIndex = -1;
		}
		m_BattleBGM[i] = BattleBGMStruct();
	}
	m_DefaultBGM = BattleBGMStruct();
	m_DefaultBGM.fCurrentVolume = 1.f;
	m_CurrentBattleBGM = BattleBGMEnum::NoneBattle;


	CEtWorld::Destroy();
	CEtLight::SetInfluenceLight( NULL );

	CDnWorldProp::Reset();

//	m_dwPropUniqueCount = 0;
}

void CDnWorld::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CEtWorld::Process( LocalTime, fDelta );
	m_Environment.Process( LocalTime, fDelta );

	ProcessVisibleProp( LocalTime, fDelta );
	ProcessBattleBGM( LocalTime, fDelta );
}

CEtWorldGrid *CDnWorld::AllocGrid()
{
	return new CDnWorldGrid( this );
}

bool CDnWorld::InitializeEnviroment( const char *szFileName, DnCameraHandle hCamera, int nCameraFar, float fWaterFarRatio )
{
	m_Environment.Finalize();

	if( m_Environment.LoadEnvironment( szFileName ) == false ) return false;

	if( m_Environment.Initialize( hCamera, nCameraFar, fWaterFarRatio ) == false ) return false;

	CDnWorldSector *pSector;
	float fVolume = 1.f;
	for( DWORD i=0; i<m_pGrid->GetActiveSectorCount(); i++ ) {
		pSector = (CDnWorldSector*)m_pGrid->GetActiveSector(i);

		if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().PauseRenderScreen( true );
		pSector->BakeLightmap( m_Environment.GetLightmapBlurSize() );
		if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().PauseRenderScreen( false );

		// Because Envitonment's sound Ratio is not a volume value, but a ratio relative to the Default BGM volume
		// Lower existing volume and raise Envi BGM to existing volume multiplied by Ratio.
		if( m_Environment.GetEnviBGMName() ) {
			if( pSector->GetSoundInfo() ) {
				fVolume = pSector->GetSoundInfo()->GetVolume();
				pSector->GetSoundInfo()->SetVolume( fVolume * ( 1.f - m_Environment.GetEnviBGMRatio() ) );
			}
			m_Environment.SetEnviBGMVolume( fVolume * m_Environment.GetEnviBGMRatio() );
		}
	}

	return true;
}

void CDnWorld::InitializeBattleBGM( int nCurrentMapIndex, int nStageConstructionLevel )
{
	m_CurrentBattleBGM = BattleBGMEnum::NoneBattle;

	// Battle BGM Load
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	std::string szBattleName[3]; 
#ifdef PRE_FIX_MEMOPT_EXT
	DNTableFileFormat* pFileNameSox = GetDNTable( CDnTableDB::TFILE );
	if (pFileNameSox == NULL)
	{
		_ASSERT(0);
		return;
	}

	if( nStageConstructionLevel < 4 ) {
		CommonUtil::GetFileNameFromFileEXT(szBattleName[0], pSox, nCurrentMapIndex, "_BattleBGM", pFileNameSox);
		CommonUtil::GetFileNameFromFileEXT(szBattleName[1], pSox, nCurrentMapIndex, "_BossBGM", pFileNameSox);
		CommonUtil::GetFileNameFromFileEXT(szBattleName[2], pSox, nCurrentMapIndex, "_NestBGM", pFileNameSox);
	}
	else {
		CommonUtil::GetFileNameFromFileEXT(szBattleName[0], pSox, nCurrentMapIndex, "_BattleBGMAbyss", pFileNameSox);
		CommonUtil::GetFileNameFromFileEXT(szBattleName[1], pSox, nCurrentMapIndex, "_BossBGMAbyss", pFileNameSox);
		CommonUtil::GetFileNameFromFileEXT(szBattleName[2], pSox, nCurrentMapIndex, "_NestBGMAbyss", pFileNameSox);
	}
#else
	if( nStageConstructionLevel < 4 ) {
		szBattleName[0] = pSox->GetFieldFromLablePtr( nCurrentMapIndex, "_BattleBGM" )->GetString();
		szBattleName[1] = pSox->GetFieldFromLablePtr( nCurrentMapIndex, "_BossBGM" )->GetString();
		szBattleName[2] = pSox->GetFieldFromLablePtr( nCurrentMapIndex, "_NestBGM" )->GetString();
	}
	else {
		szBattleName[0] = pSox->GetFieldFromLablePtr( nCurrentMapIndex, "_BattleBGMAbyss" )->GetString();
		szBattleName[1] = pSox->GetFieldFromLablePtr( nCurrentMapIndex, "_BossBGMAbyss" )->GetString();
		szBattleName[2] = pSox->GetFieldFromLablePtr( nCurrentMapIndex, "_NestBGMAbyss" )->GetString();
	}
#endif // PRE_FIX_MEMOPT_EXT

	for( int i=0; i<3; i++ ) {
		if( szBattleName[i].empty() ) continue;

		if( m_BattleBGM[i].nSoundIndex != -1 ) {
			CEtSoundEngine::GetInstance().RemoveChannel( m_BattleBGM[i].hChannel );
			CEtSoundEngine::GetInstance().RemoveSound( m_BattleBGM[i].nSoundIndex );
			m_BattleBGM[i].nSoundIndex = -1;
		}
		m_BattleBGM[i].nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( szBattleName[i].c_str(), false, false );
		if( m_BattleBGM[i].nSoundIndex != -1 ) {
			m_BattleBGM[i].hChannel = CEtSoundEngine::GetInstance().PlaySound( "BGM", m_BattleBGM[i].nSoundIndex, true, true );
			if( m_BattleBGM[i].hChannel ) {
				m_BattleBGM[i].hChannel->SetVolume( 0.f );
			}
		}
		m_BattleBGM[i].fTargetVolume = m_BattleBGM[i].fTargetVolume = 0.f;
		m_BattleBGM[i].fDelta = m_BattleBGM[i].fCurrentDelta = 0.f;
	}
	m_DefaultBGM = BattleBGMStruct();
	m_DefaultBGM.fCurrentVolume = 1.f;

	m_bApplyWaterEnvironment = ( pSox->GetFieldFromLablePtr( nCurrentMapIndex, "_ApplyWaterEnvi" )->GetInteger() == TRUE ) ? true : false;
}

TileTypeEnum CDnWorld::GetTileType( EtVector3 &vPos, float *fpHeight )
{
//	return GetTileType( vPos.x, vPos.z, fpHeight );
	if( !m_pCurrentGrid ) return TileTypeEnum::None;

	float fWaterHeight;
	if( m_bApplyWaterEnvironment && CDnWorld::GetInstance().GetWaterHeight( vPos.x, vPos.z, &fWaterHeight ) == true ) {
		if( vPos.y <= fWaterHeight /* && fWaterHeight - fHeight < 80.f */ ) {
//			m_matExWorld.m_vPosition.y = fWaterHeight;
			if( fpHeight ) *fpHeight = fWaterHeight;
			return TileTypeEnum::Water;
		}
	}

	TileTypeEnum eTileType = TileTypeEnum::None;

	eTileType = ((CDnWorldGrid*)m_pCurrentGrid)->GetTileType( vPos.x, vPos.z );

	int nNavType = 0;
	NavigationMesh* pNavMesh = CDnWorld::GetInstance().GetNavMesh( vPos );

	if( pNavMesh )
	{
		NavigationCell* pCurCell = pNavMesh->FindCell( vPos );
		if( pCurCell != NULL )
		{
			nNavType = pCurCell->GetNavType();
		}
	}

	CEtWorldGrid* pGrid = CDnWorld::GetInstance().GetGrid();

	if( pGrid )
	{
		DWORD dwCount = pGrid->GetActiveSectorCount();
		CEtWorldEventControl* pControl = NULL;
		CEtWorldSector* pSector = NULL;
		EtVector2 vStartPos( 0.0f, 0.0f );

		int nFishingAreaID = -1;
		bool bWrongArea = true;

		for( DWORD i=0; i<dwCount; i++ )
		{
			pSector = pGrid->GetActiveSector( i );
			if( !pSector )
				continue;

			pControl = pSector->GetControlFromUniqueID( ETE_EnvironmentEffectAera );
			if( !pControl )
				continue;

			pControl->Process( *CDnActor::s_hLocalActor->GetPosition() );

			if( pControl->GetCheckAreaCount() > 0 )
			{
				for( DWORD j=0; j<pControl->GetCheckAreaCount(); j++ )
				{
					CEtWorldEventArea *pArea = pControl->GetCheckArea( j );
					if( pArea == NULL )
						continue;

					EnvironmentEffectAeraStruct* pStruct = (EnvironmentEffectAeraStruct*)pArea->GetData();
					if( pStruct )
					{
						nNavType = pStruct->EffectType;
						break;
					}
				}
			}
		}
	}

	if( nNavType > 0 && nNavType < TileTypeEnum_Amount )
		eTileType = (TileTypeEnum)nNavType;

	return eTileType;
}


void CDnWorld::ProcessVisibleProp( LOCAL_TIME LocalTime, float fDelta )
{
	m_fPropCollisionDistance = 0.f;

	// Prop Visible Check
	// Instead of running Process on all Props, it scans on demand and runs Process only on those, which makes it a bit messy.
	DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
	if( !hCamera ) return;
	if( hCamera->GetCameraType() != CDnCamera::PlayerCamera &&
		hCamera->GetCameraType() != CDnCamera::NpcTalkCamera ) return;
	// Push entities touching the bounding box that have no Collision Mesh.
	DNVector(CEtWorldProp*) VecResult;
	SOBB Box;
	float fMinimumCollisionDistance = FLT_MAX;
	VisiblePropStruct Struct;
	SSegment Segment;
	EtVector3 vDir;
	EtVector3 vPos = hCamera->GetMatEx()->m_vPosition;
	EtVector3 vCameraPos = vPos;

	if( hCamera->GetCameraType() == CDnCamera::PlayerCamera ) {
		if( ((CDnPlayerCamera*)hCamera.GetPointer())->GetAttachActor() )
			vPos = *((CDnPlayerCamera*)hCamera.GetPointer())->GetAttachActor()->GetPosition();

		Segment.vOrigin = vPos;
//			Segment.vOrigin += hCamera->GetMatEx()->m_vXAxis * CDnPlayerCamera::s_fCenterPushWidth;
		Segment.vOrigin.y += CDnPlayerCamera::s_fCenterPushHeight;

		vDir = Segment.vOrigin - hCamera->GetMatEx()->m_vPosition;
		float fLength = EtVec3Length( &vDir );
		EtVec3Normalize( &vDir, &vDir );

//			Segment.vOrigin += -vDir * (float)CDnActor::s_hLocalActor->GetUnitSize();
		Segment.vDirection = -vDir * fLength;//-vDir * ((CDnPlayerCamera *)hCamera.GetPointer())->GetDistance();
	}
	else {
		vDir = hCamera->GetMatEx()->m_vZAxis;
		Segment.vOrigin = hCamera->GetMatEx()->m_vPosition;
		Segment.vDirection = vDir * 2000.f;
	}
	
	float fDistance = EtVec3Length( &Segment.vDirection );
	float fMinContectDelta = 1.f;

	VecResult.reserve( 400 );
	CEtWorld::ScanProp( hCamera->GetMatEx()->m_vPosition, fDistance, &VecResult );

	for( DWORD i=0; i<VecResult.size(); i++ ) {
		CDnWorldProp *pProp = (CDnWorldProp *)VecResult[i];
		if( pProp->GetIntersectionType() == CDnWorldProp::NoIntersectioin ) continue;

		pProp->GetBoundingBox( Box );
		if( TestLineToOBB( vCameraPos, vDir, Box ) ) {
			if( !pProp->GetObjectHandle() ) continue;
			if( pProp->GetObjectHandle()->GetCollisionPrimitiveCount() == 0 ) {
				switch( pProp->GetIntersectionType() ) {
					case CDnWorldProp::Alpha:
						{
							bool bInvalid = false;
							for( DWORD j=0; j<m_VecVisibleOnCheckList.size(); j++ ) {
								if( pProp == m_VecVisibleOnCheckList[j].pProp ) {
									bInvalid = true;
									break;
								}
							}
							if( bInvalid ) continue;

							for( DWORD j=0; j<m_VecVisibleOffCheckList.size(); j++ ) {
								if( pProp == m_VecVisibleOffCheckList[j].pProp ) {
									m_VecVisibleOffCheckList[j].fDelta = 0.5f - m_VecVisibleOffCheckList[j].fDelta;
									m_VecVisibleOnCheckList.push_back( m_VecVisibleOffCheckList[j] );
									m_VecVisibleOffCheckList.erase( m_VecVisibleOffCheckList.begin() + j );
									bInvalid = true;
									break;
								}
							}
							if( bInvalid ) continue;

							Struct.pProp = pProp;
							Struct.fDelta = 0.5f;
							m_VecVisibleOnCheckList.push_back( Struct );
						}
						break;
				}
			}
			else {
				if( pProp->GetObjectHandle()->TestSegmentCollision( Segment ) == true ) {
					switch( pProp->GetIntersectionType() ) {
						case CDnWorldProp::Alpha:
							{
								bool bInvalid = false;
								for( DWORD j=0; j<m_VecVisibleOnCheckList.size(); j++ ) {
									if( pProp == m_VecVisibleOnCheckList[j].pProp ) {
										bInvalid = true;
										break;
									}
								}
								if( bInvalid ) continue;

								for( DWORD j=0; j<m_VecVisibleOffCheckList.size(); j++ ) {
									if( pProp == m_VecVisibleOffCheckList[j].pProp ) {
										m_VecVisibleOffCheckList[j].fDelta = 0.5f - m_VecVisibleOffCheckList[j].fDelta;
										m_VecVisibleOnCheckList.push_back( m_VecVisibleOffCheckList[j] );
										m_VecVisibleOffCheckList.erase( m_VecVisibleOffCheckList.begin() + j );
										bInvalid = true;
										break;
									}
								}
								if( bInvalid ) continue;

								Struct.pProp = pProp;
								Struct.fDelta = 0.5f;
								m_VecVisibleOnCheckList.push_back( Struct );
							}
							break;
						case CDnWorldProp::Collision:
							{
								SCollisionResponse Response;
								if( pProp->GetObjectHandle()->FindSegmentCollision( Segment, Response ) )
								{
									if( fMinContectDelta < Response.fContactTime ) continue;

									fMinContectDelta = Response.fContactTime;
									bool bIsInside = false;
									switch( Response.pCollisionPrimitive->Type ) {
										case CT_TRIANGLE:
											// When colliding with a floor triangle, using fContactTime as-is makes it look like it clips through..
											// Raise it slightly so it doesn't look clipped through..
											// #13854
											Response.fContactTime *= 0.9f;
											bIsInside = false;
											break;
										default:
											bIsInside = IsInside( *Response.pCollisionPrimitive, Segment.vOrigin );
											break;
									}
									if( bIsInside )
									{
										SCollisionResponse Response2;
										SSegment SegmentTemp = Segment;
										SegmentTemp.vOrigin = vPos;
										SegmentTemp.vDirection = hCamera->GetMatEx()->m_vPosition - SegmentTemp.vOrigin;
										if( ( pProp->GetObjectHandle()->FindSegmentCollision( SegmentTemp, Response2 ) ) && ( Response.pCollisionPrimitive == Response2.pCollisionPrimitive ) )
										{
											m_fPropCollisionDistance = fDistance;
										}
										else
										{
											m_fPropCollisionDistance = 0.0f;
										}
									}
									else
									{
										m_fPropCollisionDistance = fDistance * ( 1.0f - Response.fContactTime );
									}
								}
							}
							break;
					}
				}
			}
		}
	}

	// Subtract alpha by delta while running VisibleOn Process
	// Push actors out of visible check to VisibleOff and remove them from VisibleOn.
	for( DWORD i=0; i<m_VecVisibleOnCheckList.size(); i++ ) {
		CDnWorldProp *pProp = (CDnWorldProp *)m_VecVisibleOnCheckList[i].pProp;
		pProp->GetBoundingBox( Box );
		bool bVisibleOff = false;
		if( pProp->GetObjectHandle()->GetCollisionPrimitiveCount() == 0 ) {
			if( !TestLineToOBB( vCameraPos, vDir, Box ) ) {
				bVisibleOff = true;
			}
		}
		else {
			if( pProp->GetObjectHandle()->TestSegmentCollision( Segment ) == false ) {
				bVisibleOff = true;
			}
		}
		if( bVisibleOff == true ) {
			if( m_VecVisibleOnCheckList[i].fDelta > 0.f )
				m_VecVisibleOnCheckList[i].fDelta = 0.5f - m_VecVisibleOnCheckList[i].fDelta;
			else m_VecVisibleOnCheckList[i].fDelta = 0.5f;
			m_VecVisibleOffCheckList.push_back( m_VecVisibleOnCheckList[i] );
			m_VecVisibleOnCheckList.erase( m_VecVisibleOnCheckList.begin() + i );
			i--;
			continue;
		}

		if( m_VecVisibleOnCheckList[i].fDelta > 0.f ) {
			m_VecVisibleOnCheckList[i].fDelta -= fDelta;
			if( m_VecVisibleOnCheckList[i].fDelta < 0.f ) m_VecVisibleOnCheckList[i].fDelta = 0.f;
			pProp->GetObjectHandle()->SetObjectAlpha( 0.5f + m_VecVisibleOnCheckList[i].fDelta );
		}
	}
	// Run VisibleOff Process.
	for( DWORD i=0; i<m_VecVisibleOffCheckList.size(); i++ ) {
		CDnWorldProp *pProp = (CDnWorldProp *)m_VecVisibleOffCheckList[i].pProp;
		if( pProp->GetIntersectionType() == CDnWorldProp::NoIntersectioin ) {
			m_VecVisibleOffCheckList[i].fDelta = 0.f;
		}
		m_VecVisibleOffCheckList[i].fDelta -= fDelta;
		if( m_VecVisibleOffCheckList[i].fDelta < 0.f ) m_VecVisibleOffCheckList[i].fDelta = 0.f;
		pProp->GetObjectHandle()->SetObjectAlpha( 1.f - m_VecVisibleOffCheckList[i].fDelta );
		if( m_VecVisibleOffCheckList[i].fDelta == 0.f ) {
			m_VecVisibleOffCheckList.erase( m_VecVisibleOffCheckList.begin() + i );
			i--;
		}
	}
}

void CDnWorld::CheckAndRemoveVisibleProp( CDnWorldProp *pProp )
{
	for( DWORD i=0; i<m_VecVisibleOffCheckList.size(); i++ ) {
		CDnWorldProp *pCurProp = (CDnWorldProp *)m_VecVisibleOffCheckList[i].pProp;
		if( pCurProp == pProp ) {
			pCurProp->GetObjectHandle()->SetObjectAlpha( 1.f );
			m_VecVisibleOffCheckList.erase( m_VecVisibleOffCheckList.begin() + i );
			return;
		}
	}

	for( DWORD i=0; i<m_VecVisibleOnCheckList.size(); i++ ) {
		CDnWorldProp *pCurProp = (CDnWorldProp *)m_VecVisibleOnCheckList[i].pProp;
		if( pCurProp == pProp ) {
			pCurProp->GetObjectHandle()->SetObjectAlpha( 1.f );
			m_VecVisibleOnCheckList.erase( m_VecVisibleOnCheckList.begin() + i );
			return;
		}
	}
}

CDnWorld::WeatherEnum CDnWorld::GetNextWeather( int nMapTableID )
{
	std::vector<WeatherEnum> VecWeather;
	DWORD i;
	for( i=0; i<5; i++ ) {
		if( s_WeatherDefine[m_CurrentWeather][i] == (WeatherEnum)-1 ) break;
		VecWeather.push_back( s_WeatherDefine[m_CurrentWeather][i] );
	}
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pWeatherSox = GetDNTable( CDnTableDB::TWEATHER );
	int nWeatherID = pSox->GetFieldFromLablePtr( nMapTableID, "_WeatherID" )->GetInteger();
	int *nWeatherRandomCount = new int[VecWeather.size()];
	int nOffsetMax = 0;
	char szEnviField[64];
	for( i=0; i<VecWeather.size(); i++ ) {
		sprintf_s( szEnviField, "_Envi%d_Prob", VecWeather[i] + 1 );
		float fValue = pWeatherSox->GetFieldFromLablePtr( nWeatherID, szEnviField )->GetFloat();

		nOffsetMax += (int)(fValue * 100.f );
		nWeatherRandomCount[i] = nOffsetMax;
	}
	//blondy
	int nRandom = 0;
	
	if( nOffsetMax == 0 )
	{
		ErrorLog("MapID:%d had not Valid Weather value. Check Map table.",nMapTableID);
		nOffsetMax = 1;
	}

	nRandom = _rand()%nOffsetMax;	
		
	//blondy end

	for( i=0; i<VecWeather.size(); i++ ) {
		if( nRandom < nWeatherRandomCount[i] ) break;
	}

	SAFE_DELETEA( nWeatherRandomCount );
	return s_WeatherDefine[ m_CurrentWeather ][ i ];
}

std::string CDnWorld::GetEnviName( int nMapTableID, int nStageConstructionLevel, WeatherEnum CurrentWeather )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pWeatherSox = GetDNTable( CDnTableDB::TWEATHER );
#ifdef PRE_FIX_MEMOPT_EXT
	DNTableFileFormat*  pFileNameSox = GetDNTable( CDnTableDB::TFILE );
	if (!pFileNameSox)
		return "";
#endif
	int nWeatherID = pSox->GetFieldFromLablePtr( nMapTableID, "_WeatherID" )->GetInteger();
	std::string szTemp;
	char szLabel[32];
	std::vector<std::string> szVecList;
	int nResultIndex = 0;
	if( nStageConstructionLevel < 4 ) {
		for( int i=0; i<5; i++ ) {
			sprintf_s( szLabel, "_Envi%d_%d", CurrentWeather + 1, i + 1 );
#ifdef PRE_FIX_MEMOPT_EXT
			CommonUtil::GetFileNameFromFileEXT(szTemp, pWeatherSox, nWeatherID, szLabel, pFileNameSox);
#else
			szTemp = pWeatherSox->GetFieldFromLablePtr( nWeatherID, szLabel )->GetString();
#endif
			if( szTemp.empty() ) continue;
			szVecList.push_back( szTemp );
		}
		if( szVecList.empty() ) return "";
		nResultIndex = _rand()%(int)szVecList.size();
	}
	else {
		int nOffset[3] = { 0, };
		int nOffsetMax = 0;
		for( int i=0; i<3; i++ ) {
			sprintf_s( szLabel, "_EnviAbyss%d", i + 1 );
#ifdef PRE_FIX_MEMOPT_EXT
			CommonUtil::GetFileNameFromFileEXT(szTemp, pWeatherSox, nWeatherID, szLabel, pFileNameSox);
#else
			szTemp = pWeatherSox->GetFieldFromLablePtr( nWeatherID, szLabel )->GetString();
#endif
			if( szTemp.empty() ) continue;
			szVecList.push_back( szTemp );

			sprintf_s( szLabel, "_EnviAbyss%d_Prob", i + 1 );
			nOffsetMax += (int)( pWeatherSox->GetFieldFromLablePtr( nWeatherID, szLabel )->GetFloat() * 100.f );
			nOffset[i] = nOffsetMax;
		}
		if( szVecList.empty() ) return "";
		nResultIndex = _rand()%nOffsetMax;
		for( int i=0; i<3; i++ ) {
			if( nResultIndex < nOffset[i] ) {
				nResultIndex = i;
				break;
			}
		}
	}
	return szVecList[nResultIndex];
}

int CDnWorld::ScanProp( EtVector3 &vPos, float fRadius, PropTypeEnum PropType, DNVector(DnPropHandle) &hVecList )
{
	int nCount = (int)hVecList.size();
	DNVector(CEtWorldProp *) pVecList;
	CEtWorld::ScanProp( vPos, fRadius, &pVecList );
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		CDnWorldProp *pProp = (CDnWorldProp *)pVecList[i];
		if( pProp && (PropType == -1 || pProp->GetPropType() == PropType) ) 
		{
			if( pProp->GetMySmartPtr() )
				hVecList.push_back( pProp->GetMySmartPtr() );
		}
	}
	return (int)hVecList.size() - nCount;
}

void CDnWorld::InitializeGateInfo( int nCurrentMapIndex, int nArrayIndex )
{
	m_mGateName.clear();
	SAFE_DELETE_PVEC( m_pVecGateList );
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONMAP );
	DNTableFileFormat* pDLMapSox = GetDNTable( CDnTableDB::TDLMAP );
	char szLabel[64];
	std::string szTemp;
	std::vector<CEtWorldEventArea *> VecArea;
	int nTemp;

	for( int i=0; i<DNWORLD_GATE_COUNT; i++ )
	{ 
		std::vector<int> vecMapIndex, vecStartGate, vecPropIndex;
		GetGateMapIndex( nCurrentMapIndex, i, vecMapIndex );
		GetGateStartGate( nCurrentMapIndex, i, vecStartGate );
		GetGatePropIndex( nCurrentMapIndex, i, nArrayIndex, vecPropIndex );

		sprintf_s( szLabel, "_Gate%d_Title", i + 1 );
		int nStringID = pSox->GetFieldFromLablePtr( nCurrentMapIndex, szLabel )->GetInteger();
		std::wstring szGateTitleName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID );
		m_mGateName.insert( std::pair<int, std::wstring>( i + 1, szGateTitleName ) );

		if( vecMapIndex.empty() )
			continue;

		for( DWORD itr = 0; itr < vecMapIndex.size(); ++itr )
		{
			GateStruct *pStruct = NULL;

			int nMapIndex = vecMapIndex[itr];
			if( nMapIndex < DUNGEONGATE_OFFSET ) {
				pStruct = CalcMapInfo( nMapIndex );
				if( pStruct == NULL ) continue;

				if( itr < vecStartGate.size() )
					pStruct->cStartGateIndex = vecStartGate[itr];
				else
					pStruct->cStartGateIndex = 0;
			}
			else {
				if( pDungeonSox->IsExistItem( nMapIndex ) == false ) continue;

				pStruct = new DungeonGateStruct;
				pStruct->nMapIndex = nMapIndex;
				pStruct->MapType = CDnWorld::MapTypeDungeon;
				pStruct->szMapName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pDungeonSox->GetFieldFromLablePtr( nMapIndex, "_Title" )->GetInteger() );
				((DungeonGateStruct*)pStruct)->SetType = (DungeonGateStruct::DungeonSetType)pDungeonSox->GetFieldFromLablePtr( nMapIndex, "_StageSetType" )->GetInteger();
				((DungeonGateStruct*)pStruct)->szGateEnterImage = pDungeonSox->GetFieldFromLablePtr( nMapIndex, "_GateEnterImage" )->GetString();

				bool bIncludeBuild = false;
				for( int j=0; j<5; j++ ) {
					sprintf_s( szLabel, "_MapIndex%d", j + 1 );
					nTemp = pDungeonSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetInteger();
					if( nTemp < 1 ) continue;

					GateStruct *pStructSub = NULL;
					pStructSub = CalcMapInfo( nTemp );
					if( pStructSub == NULL ) continue;
					if( pStructSub->bIncludeBuild == true ) bIncludeBuild = true;

					sprintf_s( szLabel, "_Map%d_StartGate", j + 1 );
					pStructSub->cStartGateIndex = pDungeonSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetInteger();

					((DungeonGateStruct*)pStruct)->pVecMapList.push_back( pStructSub );
				}
				pStruct->bIncludeBuild = bIncludeBuild;
			}
			pStruct->cGateIndex = i + 1;

			if (itr < vecPropIndex.size() )
				nTemp = vecPropIndex[itr];
			else
				nTemp = 0;

			std::vector<CEtWorldProp *> pVecPropList;
			FindPropFromCreateUniqueID( nTemp, &pVecPropList );
			if( !pVecPropList.empty() )
				pStruct->hGateProp = ((CDnWorldProp*)pVecPropList[0])->GetMySmartPtr();
			else
				pStruct->hGateProp = CDnWorldProp::Identity();

			VecArea.clear();
			sprintf_s( szLabel, "Gate %d", i + 1 );
			CDnWorld::GetInstance().FindEventAreaFromName( ETE_EventArea, szLabel, &VecArea );
			if( VecArea.empty() ) pStruct->pGateArea = NULL;
			else pStruct->pGateArea = VecArea[0];

			VecArea.clear();
			sprintf_s( szLabel, "GateSafeZone %d", i + 1 );
			CDnWorld::GetInstance().FindEventAreaFromName( ETE_EventArea, szLabel, &VecArea );
			if( VecArea.empty() ) pStruct->pSafeArea = NULL;
			else pStruct->pSafeArea = VecArea[0];

			m_pVecGateList.push_back( pStruct );
		}
	}
}

CDnWorld::GateStruct *CDnWorld::CalcMapInfo( int nMapIndex )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	char szLabel[64];
	std::string szTemp;

	if( pSox->IsExistItem( nMapIndex ) == false ) return NULL;

	GateStruct *pStruct = NULL;
	MapTypeEnum MapType = (CDnWorld::MapTypeEnum)pSox->GetFieldFromLablePtr( nMapIndex, "_MapType" )->GetInteger();

	switch( MapType ) {
		case MapTypeEnum::MapTypeDungeon:
			pStruct = new DungeonGateStruct;
			break;
		default: 
			pStruct = new GateStruct; 
			break;
	}

	pStruct->nMapIndex = nMapIndex;
	pStruct->MapType = MapType;
	pStruct->Environment = (CDnWorld::LandEnvironment)pSox->GetFieldFromLablePtr( nMapIndex, "_Environment" )->GetInteger();

	for( int j=0; j<10; j++ ) {
		sprintf_s( szLabel, "_ToolName%d", j + 1 );
		szTemp = pSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetString();
		if( szTemp.empty() ) continue;

		pStruct->szVecToolMapName.push_back( szTemp );
	}

	/*
	for( int j=0; j<CDnWorld::WeatherEnum_Amount;	j++ ) {
		for( int k=0; k<5; k++ ) {
			sprintf_s( szLabel, "_Envi%d_%d", j+1, k+1 );
			szTemp = pSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetString();
			if( szTemp.empty() ) continue;
			pStruct->szVecWeatherName[j].push_back( szTemp );
		}
	}
	*/
	pStruct->szMapName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nMapIndex, "_MapNameID" )->GetInteger() );
	pStruct->szDesc = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nMapIndex, "_SummaryStringID" )->GetInteger() );
	pStruct->PermitFlag = PermitEnter;
	pStruct->bIncludeBuild = ( pSox->GetFieldFromLablePtr( nMapIndex, "_IncludeBuild" )->GetInteger() ) ? true : false;

	if( MapType == CDnWorld::MapTypeDungeon ) {
		int nTemp = pSox->GetFieldFromLablePtr( nMapIndex, "_EnterConditionTableID" )->GetInteger();
		if( nTemp > 0 ) {
			DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
			if( pDungeonSox->IsExistItem( nTemp ) ) {
				DungeonGateStruct *pDungeonStruct = (DungeonGateStruct *)pStruct;
				pDungeonStruct->nMinLevel = pDungeonSox->GetFieldFromLablePtr( nTemp, "_LvlMin" )->GetInteger();
				pDungeonStruct->nMaxLevel = pDungeonSox->GetFieldFromLablePtr( nTemp, "_LvlMax" )->GetInteger();
				pDungeonStruct->nMinPartyCount = pDungeonSox->GetFieldFromLablePtr( nTemp, "_PartyOneNumMin" )->GetInteger();
				pDungeonStruct->nMaxPartyCount = pDungeonSox->GetFieldFromLablePtr( nTemp, "_PartyOneNumMax" )->GetInteger();
				pDungeonStruct->nMaxUsableCoin = pDungeonSox->GetFieldFromLablePtr( nTemp, "_MaxUsableCoin" )->GetInteger();
				pDungeonStruct->nAbyssMinLevel = pDungeonSox->GetFieldFromLablePtr( nTemp, "_AbyssLvlMin" )->GetInteger();
				pDungeonStruct->nAbyssMaxLevel = pDungeonSox->GetFieldFromLablePtr( nTemp, "_AbyssLvlMax" )->GetInteger();
				pDungeonStruct->nAbyssNeedQuestID = pDungeonSox->GetFieldFromLablePtr( nTemp, "_AbyssQuestID" )->GetInteger();

				static char *szDifficultStr[] = { "Easy", "Normal", "Hard", "VeryHard", "Nightmare" };
				for( int i=0; i<5; i++ ) {
					sprintf_s( szLabel, "_Recommend%sLevel", szDifficultStr[i] );
					DNTableCell *pRecLvlCell = pDungeonSox->GetFieldFromLablePtr( nTemp, szLabel );
					if( pRecLvlCell ) pDungeonStruct->nRecommendLevel[i] = pRecLvlCell->GetInteger();
					sprintf_s( szLabel, "_Recommend%sPartyCount", szDifficultStr[i] );
					DNTableCell *pRecPartyCell = pDungeonSox->GetFieldFromLablePtr( nTemp, szLabel );
					if( pRecPartyCell ) pDungeonStruct->nRecommendPartyCount[i] = pRecPartyCell->GetInteger();
				}
				DNTableCell *pCanEasyCell = pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanEasyLevel" );
				if( pCanEasyCell && pCanEasyCell->GetInteger() == 0 ) {
					pDungeonStruct->cCanDifficult[0] = 0;
				}
				DNTableCell *pRecHardCell = pDungeonSox->GetFieldFromLablePtr( nTemp, "_RecommendHardLevel" );
				if( pRecHardCell && pRecHardCell->GetInteger() > 0 && pDungeonStruct->nRecommendLevel[2] == 0 ) {
					pDungeonStruct->nRecommendLevel[2] = pRecHardCell->GetInteger();
				}
				pDungeonStruct->nNeedItemID = pDungeonSox->GetFieldFromLablePtr( nTemp, "_NeedItemID" )->GetInteger();
				pDungeonStruct->nNeedItemCount = pDungeonSox->GetFieldFromLablePtr( nTemp, "_NeedItemCount" )->GetInteger();
				pDungeonStruct->bNeedVehicle = (pDungeonSox->GetFieldFromLablePtr( nTemp, "_Vehicle" )->GetInteger()) ? true : false;
				
				if( pDungeonStruct->nNeedItemID != 0 && pDungeonStruct->nNeedItemCount == 0 ) {
					_ASSERT(0&&"Why is the required dungeon entry item ID set when the quantity is 0?");
				}
			}
		}
	}

	return pStruct;
}

CDnWorld::GateStruct *CDnWorld::GetGateStruct( char cGateIndex )
{
	for( DWORD i=0; i<m_pVecGateList.size(); i++ ) {
		if( m_pVecGateList[i]->cGateIndex == cGateIndex ) return m_pVecGateList[i];
	}
	return NULL;
}

void CDnWorld::GetGateStructList( const char cGateIndex, std::vector<GateStruct *> & vecGateList )
{
	for( DWORD i=0; i<m_pVecGateList.size(); i++ )
	{
		if( m_pVecGateList[i]->cGateIndex == cGateIndex )
			vecGateList.push_back( m_pVecGateList[i] );
	}
}

std::wstring CDnWorld::GetGateTitleName( const char cGateIndex )
{
	std::map<int, std::wstring>::iterator Itor = m_mGateName.find( cGateIndex );

	if( m_mGateName.end() == Itor )
		return std::wstring();

	return Itor->second;
}

CDnWorld::PermitGateEnum CDnWorld::GetPermitGate( int nGateIndex )
{
	GateStruct *pStruct = GetGateStruct( (char)nGateIndex );
	if( pStruct == NULL ) return PermitClose;
	return pStruct->PermitFlag;
}

void CDnWorld::SetPermitGate( int nGateIndex, PermitGateEnum Flag )
{
	GateStruct *pStruct = GetGateStruct( (char)nGateIndex );
	if( pStruct == NULL ) return;

	std::string szAction;
	PermitGateEnum PrevFlag = pStruct->PermitFlag;

	if( ( PrevFlag == PermitEnter ) && ( Flag != PermitEnter ) ) szAction = "Open_Close";
	else if( ( PrevFlag != PermitEnter ) && ( Flag == PermitEnter ) ) szAction = "Close_Open";
	else if( ( PrevFlag != PermitEnter ) && ( Flag != PermitEnter ) ) szAction = "Close";
	else if( ( PrevFlag == PermitEnter ) && ( Flag == PermitEnter ) ) szAction = "Open";

	pStruct->PermitFlag = Flag;

	/*
	int nArrayIndex = -1;
	for( DWORD i=0; i<pStruct->dwVecGatePropIDList.size(); i++ ) {
		if( _stricmp( pStruct->szVecToolMapName[i].c_str(), GetGrid()->GetName() ) != NULL ) continue;
		nArrayIndex = i;
		break;
	}
	if( nArrayIndex == -1 ) return;

	std::vector<CEtWorldProp *> VecProp;
	CDnWorld::GetInstance().FindPropFromCreateUniqueID( pStruct->dwVecGatePropIDList[nArrayIndex], &VecProp );

	for( DWORD i=0; i<VecProp.size(); i++ ) {
		CDnWorldActProp *pProp = dynamic_cast<CDnWorldActProp *>(VecProp[i]);
		if( !pProp ) continue;
		pProp->SetActionQueue( szAction.c_str() );
	}
	*/
	if( pStruct->hGateProp ) {
		pStruct->hGateProp->CmdAction( szAction.c_str() );
	}
}

bool CDnWorld::GateStruct::CanEnterGate()
{
	if( !pSafeArea ) return true;
	SOBB *pOBB = pSafeArea->GetOBB();
	SSphere Sphere;
	float fRadius = max( pOBB->Extent[0], pOBB->Extent[2] );

	DNVector(DnActorHandle) hVecList;
	CDnActor::ScanActor( pOBB->Center, fRadius, hVecList );

	for( DWORD i=0; i<hVecList.size(); i++ ) {
		if( hVecList[i]->GetClassID() <= CDnActor::Reserved6 ) continue;
		if( hVecList[i]->GetClassID() == CDnActor::Npc ) continue;
		if( hVecList[i]->IsDie() ) continue;
		if( CDnActor::s_hLocalActor->GetTeam() == hVecList[i]->GetTeam() ) continue;

		hVecList[i]->GetBoundingSphere( Sphere );
		if( TestOBBToSphere( *pOBB, Sphere ) == true ) return false;
	}

	return true;
}

bool CDnWorld::CanEnterGate( char cGateIndex )
{
	for( DWORD i=0; i<m_pVecGateList.size(); i++ ) {
		if( m_pVecGateList[i]->cGateIndex == cGateIndex ) {
			return m_pVecGateList[i]->CanEnterGate();
		}
	}
	return false;
}

void CDnWorld::CallActionTrigger( SectorIndex Index, int nObjectIndex, int nRandomSeed )
{
	if( !m_pGrid ) return;
	CDnWorldSector *pSector;
	for( DWORD i=0; i<m_pGrid->GetActiveSectorCount(); i++ ) {
		pSector = (CDnWorldSector*)m_pGrid->GetActiveSector(i);
		if( pSector->GetIndex() != Index ) continue;
		((CDnTrigger *)pSector->GetTrigger())->CallActionTrigger( nObjectIndex, nRandomSeed );
		break;
	}
}

void CDnWorld::ChangeBattleBGM( BattleBGMEnum Type, float fPrevVolume, float fPrevFadeDelta, float fChangeVolume, float fChangeFadeDelta )
{
	if( Type == m_CurrentBattleBGM ) return;

	BattleBGMEnum TypeList[2] = { m_CurrentBattleBGM, Type };
	float fVolume[2] = { fPrevVolume, fChangeVolume };
	float fDelta[2] = { fPrevFadeDelta, fChangeFadeDelta };

	for( int i=0; i<2; i++ ) {
		if( TypeList[i] != BattleBGMEnum::NoneBattle ) {
			m_BattleBGM[TypeList[i]].fTargetVolume = fVolume[i];
			m_BattleBGM[TypeList[i]].fDelta = fDelta[i];
			m_BattleBGM[TypeList[i]].fCurrentDelta = fDelta[i];
			if( fDelta[i] == 0.f && m_BattleBGM[TypeList[i]].hChannel ) {
				m_BattleBGM[TypeList[i]].hChannel->SetVolume( fVolume[i] );
				if( fVolume[i] == 0.f ) m_BattleBGM[TypeList[i]].hChannel->Pause();
				else m_BattleBGM[TypeList[i]].hChannel->Resume();
			}
		}
		else {
			/*
			CDnWorldSector *pSector;
			float fTemp = 1.f;
			for( DWORD j=0; j<m_pGrid->GetActiveSectorCount(); j++ ) {
				pSector = (CDnWorldSector*)m_pGrid->GetActiveSector(j);

				if( pSector->GetSoundInfo() ) {
					fTemp = pSector->GetSoundInfo()->GetVolume();
					if( m_Environment.GetEnviBGMName() ) fTemp *= ( 1.f - m_Environment.GetEnviBGMRatio() );
				}
				else {
					fTemp = 0.f;
				}
			}
			*/
			m_DefaultBGM.fTargetVolume = fVolume[i];
			m_DefaultBGM.fDelta = fDelta[i];
			m_DefaultBGM.fCurrentDelta = fDelta[i];
		}
	}
	m_CurrentBattleBGM = Type;
}

void CDnWorld::ProcessBattleBGM( LOCAL_TIME LocalTime, float fDelta )
{
	if( false == m_bProcessBattleBGM )
		return;

	for( int i=0; i<BattleBGMEnum_Amount; i++ ) {
		BattleBGMStruct *pStruct = &m_BattleBGM[i];
		if( pStruct->nSoundIndex == -1 ) continue;
		if( !pStruct->hChannel ) continue;

		if( pStruct->fCurrentDelta == 0.f ) continue;
		pStruct->fCurrentDelta -= fDelta;
		if( pStruct->fCurrentDelta <= 0.f ) pStruct->fCurrentDelta = 0.f;

		float fRatio = 1.f - ( 1.f / pStruct->fDelta * pStruct->fCurrentDelta );
		float fVolume = pStruct->fCurrentVolume + ( ( pStruct->fTargetVolume - pStruct->fCurrentVolume ) * fRatio );
		if( pStruct->fCurrentDelta == 0.f ) {
			pStruct->fCurrentVolume = pStruct->fTargetVolume;
			fVolume = pStruct->fTargetVolume;
		}
		pStruct->hChannel->SetVolume( fVolume );
		if( fVolume == 0.f ) {
			if( !pStruct->hChannel->IsPause() )
				pStruct->hChannel->Pause();
		}
		else {
			if( pStruct->hChannel->IsPause() ) 
				pStruct->hChannel->Resume();
		}
	}

	BattleBGMStruct *pStruct = &m_DefaultBGM;
	if( pStruct->fCurrentDelta > 0.f ) {
		CDnWorldSector *pSector;
		float fMaxVolumeRatio = 1.f;
		for( DWORD j=0; j<m_pGrid->GetActiveSectorCount(); j++ ) {
			pSector = (CDnWorldSector*)m_pGrid->GetActiveSector(j);
			if( pSector->GetSoundInfo() ) {
				fMaxVolumeRatio = pSector->GetSoundInfo()->GetVolume();
				if( m_Environment.GetEnviBGMName() ) fMaxVolumeRatio *= ( 1.f - m_Environment.GetEnviBGMRatio() );
			}
			else {
				fMaxVolumeRatio = 0.f;
			}

			pStruct->fCurrentDelta -= fDelta;
			if( pStruct->fCurrentDelta <= 0.f ) pStruct->fCurrentDelta = 0.f;

			float fRatio = 1.f - ( 1.f / pStruct->fDelta * pStruct->fCurrentDelta );
			float fVolume = pStruct->fCurrentVolume + ( ( pStruct->fTargetVolume - pStruct->fCurrentVolume ) * fRatio );
			if( pStruct->fCurrentDelta == 0.f ) {
				pStruct->fCurrentVolume = pStruct->fTargetVolume;
			}
			if( pSector->GetSoundInfo() ) pSector->GetSoundInfo()->SetVolume( fVolume * fMaxVolumeRatio );
		}
	}
}


bool CDnWorld::PickFromScreenCoord(float x, float y, OUT EtVector3& vPickPos)
{
	LPDIRECT3DDEVICE9 pDevice= NULL;

	pDevice = (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr();
	D3DVIEWPORT9 vp;
	pDevice->GetViewport ( &vp );

	D3DXMATRIXA16 matProj;
	pDevice->GetTransform( D3DTS_PROJECTION, &matProj );

	EtVector3 v;
	EtVector3 vRayDir;
	EtVector3 vRayPos;

	v.x = ((  (((x-vp.X)*2.0f/vp.Width ) - 1.0f)) - matProj._31 ) / matProj._11;
	v.y = ((- (((y-vp.Y)*2.0f/vp.Height) - 1.0f)) - matProj._32 ) / matProj._22;
	v.z =  1.0f;

	// Get the inverse view matrix
	D3DXMATRIXA16 matView, m;
	pDevice->GetTransform( D3DTS_VIEW, &matView );

	D3DXMatrixInverse( &m, NULL, &matView );

	vRayDir.x  = v.x*m._11 + v.y*m._21 + v.z*m._31;
	vRayDir.y  = v.x*m._12 + v.y*m._22 + v.z*m._32;
	vRayDir.z  = v.x*m._13 + v.y*m._23 + v.z*m._33;
	vRayPos.x = m._41;
	vRayPos.y = m._42;
	vRayPos.z = m._43;

	return Pick(vRayPos, vRayDir, vPickPos);
}

void CDnWorld::RefreshQualityLevel( int nLevel )
{
	if( !m_pCurrentGrid ) return;
	((CDnWorldGrid*)m_pCurrentGrid)->RefreshQualityLevel( nLevel );
}

CDnWorld::GateStruct *CDnWorld::GetGateStructFromIndex( DWORD dwIndex ) 
{ 
	if( dwIndex < 0 || dwIndex >= m_pVecGateList.size() ) return NULL;
	return m_pVecGateList[dwIndex]; 
}

void CDnWorld::PlayBGM()
{
	if( !m_pCurrentGrid ) return;
	for( DWORD i=0; i<m_pCurrentGrid->GetActiveSectorCount(); i++ ) {
		if( !m_pCurrentGrid->GetActiveSector(i)->GetSoundInfo() ) continue;
		m_pCurrentGrid->GetActiveSector(i)->GetSoundInfo()->Play();
	}
}

void CDnWorld::StopBGM()
{
	if( !m_pCurrentGrid ) return;
	for( DWORD i=0; i<m_pCurrentGrid->GetActiveSectorCount(); i++ ) {
		if( !m_pCurrentGrid->GetActiveSector(i)->GetSoundInfo() ) continue;
		m_pCurrentGrid->GetActiveSector(i)->GetSoundInfo()->Stop();
	}
}


// 2009.7.30 hanki
// Return y position accounting for prop height
float CDnWorld::GetHeightWithProp( EtVector3 &vPos, NavigationCell *pCurCell, EtVector3 *pNormal/* = NULL */, int nTileScale/* = 1 */)
{
	float fPropHeight = 0.f;

	NavigationMesh *pNavMesh = INSTANCE(CDnWorld).GetNavMesh( vPos );
	bool bFindCollision = false;
	if( pNavMesh ) 
	{
		if( !pCurCell ) pCurCell = pNavMesh->FindCell( vPos );
		if( pCurCell )
		{
			if( pCurCell->GetType() == NavigationCell::CT_PROP ) {
				fPropHeight = pCurCell->GetPlane()->SolveForY( vPos.x, vPos.z );
				bFindCollision = true;
			}
		}
	}

	float fTerrainHeight = CEtWorld::GetHeight( vPos, NULL, nTileScale );
	float fResult;
	if( bFindCollision ) {
		float fValue1 = abs(vPos.y - fPropHeight);
		float fValue2 = abs(vPos.y - fTerrainHeight);
		fResult = ( fValue1 >= fValue2 ) ? fTerrainHeight : fPropHeight;
	}
	else fResult = fTerrainHeight;

	return fResult;
}
//

#if defined(PRE_FIX_55855)
float CDnWorld::GetMaxHeightWithProp( EtVector3 &vPos, NavigationCell *pCurCell, EtVector3 *pNormal/* = NULL */, int nTileScale/* = 1 */)
{
	float fPropHeight = 0.f;

	NavigationMesh *pNavMesh = INSTANCE(CDnWorld).GetNavMesh( vPos );
	bool bFindCollision = false;
	if( pNavMesh ) 
	{
		if( !pCurCell ) pCurCell = pNavMesh->FindCell( vPos );
		if( pCurCell )
		{
			if( pCurCell->GetType() == NavigationCell::CT_PROP ) {
				fPropHeight = pCurCell->GetPlane()->SolveForY( vPos.x, vPos.z );
				bFindCollision = true;
			}
		}
	}

	float fTerrainHeight = CEtWorld::GetHeight( vPos, NULL, nTileScale );
	
	float fResult = vPos.y;

	//If navmesh height is found, compare with heightmap height and use the higher value.
	if( bFindCollision )
		fResult = ( fTerrainHeight >= fPropHeight ) ? fTerrainHeight : fPropHeight;
	else fResult = fTerrainHeight;

	return fResult;
}
#endif // PRE_FIX_55855

/*
void CDnWorld::CheckAndRemovePermitGateInfo( int nPartyIndex )
{
	for( DWORD i=0; i<m_pVecGateList.size(); i++ ) {
		GateStruct *pGate = m_pVecGateList[i];
		for( DWORD j=0; j<pGate->cVecLessLevelActorIndex.size(); j++ ) {
			if( pGate->cVecLessLevelActorIndex[j] == nPartyIndex ) {
				pGate->cVecLessLevelActorIndex.erase( pGate->cVecLessLevelActorIndex.begin() + j );
			}
		}
	}
}
*/

void CDnWorld::Show( bool bShow )
{
	if( m_bShow == bShow ) return;
	if( !m_pGrid ) return;
	m_bShow = bShow;

	CDnWorldSector *pSector;
	for( DWORD i=0; i<m_pGrid->GetActiveSectorCount(); i++ ) {
		pSector = (CDnWorldSector*)m_pGrid->GetActiveSector(i);
		pSector->Show( bShow );
	}

}

bool CDnWorld::IsUsingTileType( GlobalEnum::TileTypeEnum Type )
{
	if( !m_pGrid ) return false;
	CDnWorldSector *pSector;
	for( DWORD i=0; i<m_pGrid->GetActiveSectorCount(); i++ ) {
		pSector = (CDnWorldSector*)m_pGrid->GetActiveSector(i);
		if( pSector->IsUsingTileType( Type ) ) return true;
	}
	return false;
}

void CDnWorld::SetProcessBattleBGM( bool bProcessBattleBGM )
{
	 m_bProcessBattleBGM = bProcessBattleBGM;

	 if( m_bProcessBattleBGM )
	 {
		 if(m_pGrid)
		 {
			 for( DWORD i=0; i<m_pGrid->GetActiveSectorCount(); i++ ) {
				 CDnWorldSector *pSector = (CDnWorldSector *)m_pGrid->GetActiveSector(i);
				 if( !pSector ) continue;
				 CDnWorldSound *pSound = (CDnWorldSound *)pSector->GetSoundInfo();
				 if( !pSound ) continue;
				 pSound->Stop();
				 pSound->FadeVolumeBGM( 1.f, CGlobalInfo::GetInstance().m_fFadeDelta );
			 }
		 }
	 }
}

void CDnWorld::GetGateMapIndex( const int nCurrentMapIndex, const int nIndex, std::vector<int> & vecMapIndex )
{
	vecMapIndex.clear();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	char szMapIndexLabel[256] = {0, };

	sprintf_s( szMapIndexLabel, "_Gate%d_MapIndex_txt", nIndex + 1 );
	std::string szMapIndex = pSox->GetFieldFromLablePtr( nCurrentMapIndex, szMapIndexLabel )->GetString();
	if( szMapIndex.empty() == true )
		ASSERT( 0 && "MapIndex Empty" );

	std::vector<std::string> tokensMapIndex;
	TokenizeA(szMapIndex, tokensMapIndex, ";");

	std::vector<std::string>::const_iterator iter = tokensMapIndex.begin();
	for (; iter != tokensMapIndex.end(); ++iter)
		vecMapIndex.push_back( atoi( (*iter).c_str() ) );
}

void CDnWorld::GetGateStartGate( const int nCurrentMapIndex, const int nIndex, std::vector<int> & vecStartGate )
{
	//vecStartGate.empty();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	char szStartGateLabel[256] = {0, };

	sprintf_s( szStartGateLabel, "_Gate%d_StartGate_txt", nIndex + 1 );
	std::string szStartGate = pSox->GetFieldFromLablePtr( nCurrentMapIndex, szStartGateLabel )->GetString();
	if( szStartGate.empty() == true )
		ASSERT( 0 && "StartGate Empty" );

	std::vector<std::string> tokensStartGate;
	TokenizeA(szStartGate, tokensStartGate, ";");

	std::vector<std::string>::const_iterator iter = tokensStartGate.begin();
	for (; iter != tokensStartGate.end(); ++iter)
		vecStartGate.push_back( atoi( (*iter).c_str() ) );
}

void CDnWorld::GetGatePropIndex( const int nCurrentMapIndex, const int nIndex, const int nArrayIndex, std::vector<int> & vecPropIndex )
{
	vecPropIndex.empty();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	char szPropIndexLabel[256] = {0, };

	sprintf_s( szPropIndexLabel, "_Gate%d_PropIndex%d_txt", nIndex + 1, nArrayIndex + 1 );
	std::string szPropIndex = pSox->GetFieldFromLablePtr( nCurrentMapIndex, szPropIndexLabel )->GetString();
	if( szPropIndex.empty() == true )
		ASSERT( 0 && "PropIndex Empty" );

	std::vector<std::string> tokensPropIndex;
	TokenizeA(szPropIndex, tokensPropIndex, ";");

	std::vector<std::string>::const_iterator iter = tokensPropIndex.begin();
	for (; iter != tokensPropIndex.end(); ++iter)
		vecPropIndex.push_back( atoi( (*iter).c_str() ) );
}


//#ifdef PRE_ADD_FILTEREVENT
void CDnWorld::SetSceneAbsoluteColor( bool bMonochrome, EtVector3 & vColor, float fVolume )
{	
	bool bExe = false;
	if( m_bMonochrome != bMonochrome || m_vFilterEventColor != vColor || m_fFilterEventVolume != fVolume )
		bExe = true;

	m_bMonochrome = bMonochrome;
	m_vFilterEventColor = vColor;
	m_fFilterEventVolume = fVolume;

	if( bExe )
		m_Environment.SetSceneAbsoluteColor( m_bMonochrome, m_vFilterEventColor, m_fFilterEventVolume );			
}
//#endif PRE_ADD_FILTEREVENT