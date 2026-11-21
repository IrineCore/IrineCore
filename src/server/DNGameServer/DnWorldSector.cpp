#include "stdafx.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"
// Prop Include
#include "DnWorldProp.h"
#include "DnWorldActProp.h"
#include "DnWorldBrokenProp.h"
#include "DnWorldTrapProp.h"
#include "DnWorldOperationProp.h"
#include "DnWorldChestProp.h"
#include "DnShooterProp.h"
#include "DnWorldBrokenDamageProp.h"
#include "DnWorldHitMoveDamageBrokenProp.h"
#include "DnBuffProp.h"
#include "DnWorldBrokenBuffProp.h"
#include "DnWorldNpcProp.h"
#include "DnWorldShooterBrokenProp.h"
#include "DnWorldEnvironmentProp.h"
#include "DnWorldMultiDurabilityProp.h"
#include "DnWorldKeepOperationProp.h"
#include "DnWorldOperationDamageProp.h"
#include "DnWorldHitStateEffectProp.h" // #60784

#include "EtWorldEventControl.h"
#include "DnTableDB.h"

#include "PerfCheck.h"
#include "DnTrigger.h"

#include "EtCollisionMng.h"

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
int CDnWorldSector::s_nProcessAllowCount[2] = { 20, 3 };
#endif
CDnWorldSector::CDnWorldSector()
{

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	memset( m_nProcessAllowed, 0, sizeof(m_nProcessAllowed) );
	m_fProcessAllowedDelta[0] = new float[s_nProcessAllowCount[0]];
	m_fProcessAllowedDelta[1] = new float[s_nProcessAllowCount[1]];
	memset( m_fProcessAllowedDelta[0], 0, sizeof(float) * s_nProcessAllowCount[0] );
	memset( m_fProcessAllowedDelta[1], 0, sizeof(float) * s_nProcessAllowCount[1] );
#else
	m_nProcessAllowed = 0;
#endif
} 

CDnWorldSector::~CDnWorldSector()
{
	Free();

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	SAFE_DELETEA( m_fProcessAllowedDelta[0] );
	SAFE_DELETEA( m_fProcessAllowedDelta[1] );
#endif
}

void CDnWorldSector::Free()
{
	CEtWorldSector::Free();
	SAFE_RELEASE_SPTR( m_Handle );

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	for( int i=0; i<2; i++ ) 
		SAFE_DELETE_VEC( m_pVecProcessProp[i] );
#else 
	SAFE_DELETE_VEC( m_pVecProcessProp );
#endif
}

#include "DnPartyTask.h"
#include "TaskManager.h"
#include "DNUserSession.h"
#include "EtWorldEventArea.h"
void CDnWorldSector::Process( LOCAL_TIME LocalTime, float fDelta )
{
	// Prop Process
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	for( int i=0; i<s_nProcessAllowCount[0]; i++ ) m_fProcessAllowedDelta[0][i] += fDelta;
	for( int i=0; i<s_nProcessAllowCount[1]; i++ ) m_fProcessAllowedDelta[1][i] += fDelta;

	// Action Prop Process
	for( int a=0; a<2; a++ ) {
		for( int i=m_nProcessAllowed[a]; i<(int)m_pVecProcessProp[a].size(); i+=s_nProcessAllowCount[a] ) {
			m_pVecProcessProp[a][i]->Process( LocalTime, m_fProcessAllowedDelta[a][m_nProcessAllowed[a]] );
			if( ((CDnWorldActProp*)m_pVecProcessProp[a][i])->IsDestroy() ) {
				CDnWorldProp *pProp = m_pVecProcessProp[a][i];
				if( DeleteProp( pProp ) == false )
					g_Log.Log( LogType::_ERROR, L"Error! DeleteProp() PropID:%d %S", pProp->GetClassID(), pProp->GetPropName() );
				SAFE_DELETE( pProp );
				m_pVecProcessProp[a].erase( m_pVecProcessProp[a].begin() + i );
				i -= ( s_nProcessAllowCount[a] - 1 );
			}
		}
		m_fProcessAllowedDelta[a][m_nProcessAllowed[a]] = 0.f;
		m_nProcessAllowed[a]++;
		if( m_nProcessAllowed[a] == s_nProcessAllowCount[a] ) m_nProcessAllowed[a] = 0;
	}
#else
	for( int i=m_nProcessAllowed; i<(int)m_pVecProcessProp.size(); i+=3 ) {
		m_pVecProcessProp[i]->Process( LocalTime, fDelta );
		if( ((CDnWorldActProp*)m_pVecProcessProp[i])->IsDestroy() ) {
			CDnWorldProp *pProp = m_pVecProcessProp[i];
			if( DeleteProp( pProp ) == false )
				g_Log.Log( LogType::_ERROR, L"Error! DeleteProp() PropID:%d %S", pProp->GetClassID(), pProp->GetPropName() );
			SAFE_DELETE( pProp );
			m_pVecProcessProp.erase( m_pVecProcessProp.begin() + i );
			i-=2;
		}
	}
	m_nProcessAllowed++;
	if( m_nProcessAllowed == 3 ) m_nProcessAllowed = 0;
#endif

	if( m_pTrigger ) {
		m_pTrigger->Process( LocalTime, fDelta );
	}

	// Event Area Process
	ProcessEventAreaTriggerEventCheck( LocalTime, fDelta );

	/*
	CDNGameRoom *pRoom = ((CDnWorld*)m_pParentGrid->GetWorld())->GetCurrentTask();
	
	for( DWORD j=0; j<CDnPartyTask::GetInstance(pRoom->GetTaskMng()->GetRoom()).GetUserCount(); j++ ) {
		DnActorHandle hActor = CDnPartyTask::GetInstance(pRoom->GetTaskMng()->GetRoom()).GetUserData(j)->GetActorHandle();
		for( DWORD i=0; i<GetControlCount(); i++ ) {
			CEtWorldEventControl *pControl = GetControlFromIndex(i);
			pControl->Process( *hActor->GetPosition() );
		}
	}
	*/
}

void CDnWorldSector::ProcessEventAreaTriggerEventCheck( LOCAL_TIME LocalTime, float fDelta )
{
	CDnWorld *pWorld = (CDnWorld*)GetParentGrid()->GetWorld();
	CMultiRoom *pRoom = pWorld->GetRoom();

	if( !CDnPartyTask::IsActive(pRoom) ) return;
	DWORD dwUserCount = CDnPartyTask::GetInstance(pRoom).GetUserCount();
	bool bCallEvent = false;
	m_mCurEventArea.clear();

	for( DWORD i=0; i<dwUserCount; i++ ) {
		std::vector<CEtWorldEventArea*> vCurEventArea;
		CDNUserSession *pSession = CDnPartyTask::GetInstance(pRoom).GetUserData(i);
		if( !pSession ) continue;
		DnActorHandle hActor = pSession->GetActorHandle();
		if( !hActor ) continue;

		EtVector3 *vCurPos = hActor->GetPosition();
		ScanEventArea( EtVector2( vCurPos->x, vCurPos->z ), (float)hActor->GetUnitSize(), &vCurEventArea );
		for( DWORD j=0; j<vCurEventArea.size(); j++ ) {
			if( !vCurEventArea[j]->CheckArea( *vCurPos, true ) ) {
				vCurEventArea.erase( vCurEventArea.begin() + j );
				j--;
				continue;
			}
		}

		std::map<DWORD,std::vector<CEtWorldEventArea*>>::iterator itor = m_mPrevEventArea.find( hActor->GetUniqueID() );
		if( !bCallEvent )
		{
			if( itor != m_mPrevEventArea.end() )
			{
				if( (*itor).second != vCurEventArea )
					bCallEvent = true;
			}
			else
			{
				if( !vCurEventArea.empty() )
					bCallEvent = true;
			}
		}
		if( !vCurEventArea.empty() )
			m_mCurEventArea.insert( std::make_pair(hActor->GetUniqueID(),vCurEventArea) );
	}

	m_mPrevEventArea = m_mCurEventArea;

#if defined( PRE_TRIGGER_TEST )
	bCallEvent = true;
#endif // #if defined( PRE_TRIGGER_TEST )

	if( bCallEvent ) {
		pWorld->OnTriggerEventCallback( "CDnWorld::OnEventArea", LocalTime, fDelta );
	}
}

CEtWorldProp *CDnWorldSector::AllocProp( int nClass )
{
	CDnWorldProp *pProp = NULL;
//	DWORD dwUniqueID = ((CDnWorld*)GetParentGrid()->GetWorld())->AddPropUniqueID();
	CMultiRoom *pRoom = ((CDnWorld*)GetParentGrid()->GetWorld())->GetRoom();
	switch( (PropTypeEnum)nClass ) 
	{
		case PTE_Static: 
		{
			pProp = new IBoostPoolDnWorldProp(pRoom); 
			break;
		}
		case PTE_Action: 
		{
			pProp = new IBoostPoolDnWorldActProp(pRoom); 
			break;
		}
		//case PTE_Broken: pProp = new CDnWorldBrokenProp(pRoom); break;
		case PTE_Broken: pProp = new IBoostPoolDnWorldBrokenProp(pRoom); break;
		case PTE_Trap: pProp = new CDnWorldTrapProp(pRoom); break;
		case PTE_Operation: 
		{
			pProp = new IBoostPoolDnWorldOperationProp(pRoom); 
			break;
		}
		case PTE_Chest: pProp = new CDnWorldChestProp(pRoom); break;
		case PTE_Light: return NULL;
		case PTE_ProjectileShooter: pProp = new CDnShooterProp(pRoom); break;
		//case PTE_BrokenDamage: pProp = new CDnWorldBrokenDamageProp( pRoom ); break;
		case PTE_BrokenDamage: pProp = new IBoostPoolDnWorldBrokenDamageProp( pRoom ); break;
		case PTE_HitMoveDamageBroken: pProp = new CDnWorldHitMoveDamageBrokenProp( pRoom ); break;
		case PTE_Buff: pProp = new CDnBuffProp( pRoom ); break;
		case PTE_BuffBroken: pProp = new CDnWorldBrokenBuffProp( pRoom ); break;
		case PTE_Npc: pProp = new CDnWorldNpcProp( pRoom ); break;
		case PTE_Camera: return NULL;
		case PTE_ShooterBroken: pProp = new CDnWorldShooterBrokenProp( pRoom ); break;
		case PTE_EnvironmentProp: pProp = new CDnWorldEnvironmentProp( pRoom ); break;
		case PTE_MultiDurabilityBrokenProp: pProp = new CDnWorldMultiDurabilityProp( pRoom ); break;
		// Note: When a new Broken-type prop is added, make IsBrokenType() virtual function return true. 
		// Currently returning true from brokenprop.

		case PTE_KeepOperation: pProp = new IBoostPoolDnWorldKeepOperationProp(pRoom); break;
		case PTE_OperationDamage: pProp = new CDnWorldOperationDamageProp(pRoom); break;

		case PTE_HitStateEffect: pProp = new CDnWorldHitStateEffectProp(pRoom); break; // #60784
		
		default: 
		{
			pProp = new IBoostPoolDnWorldProp(pRoom); 
			break;
		}
	}
	if( pProp ) {
		pProp->InitializeRoom( (CDNGameRoom*)pRoom );
	}
	return pProp;
}

int CDnWorldSector::GetPropTableID( const char *szFileName )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPROP );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"PropTable.ext failed\r\n");
		return -1;
	}
	return pSox->GetItemIDFromField( "_Name", szFileName );
}

int CDnWorldSector::GetPropClassID( const char *szFileName )
{
	if( strcmp( szFileName, "Light.skn" ) == NULL ) return PTE_Light;
	if( strcmp( szFileName, "Camera.skn" ) == NULL ) return PTE_Camera;
	int nIndex = GetPropTableID( szFileName );
	if( nIndex == -1 ) return 0;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPROP );
	return pSox->GetFieldFromLablePtr( nIndex, "_ClassID" )->GetInteger();
}

void CDnWorldSector::InitializeTerrain( DWORD *pAlphaTable, std::vector<int> &nVecBlockType, std::vector< std::vector<std::string> > &szVecLayerTexture, std::vector< std::vector<float> > &fVecLayerTextureDistance, std::vector< std::vector<float> > &fVecLayerTextureRotation, char *szGrassTexture, char *pGrassTable, float *fGrassWidth, float *fGrassHeightMin, float *fGrassHeightMax, float fShakeMin, float fShakeMax )
{
	// Create Terrain
	STerrainInfo Info;
	Info.nSizeX = GetTileWidthCount() - 1;
	Info.nSizeY = GetTileHeightCount() - 1;
	Info.pHeight = m_pHeight;
	Info.pLayerDensity = pAlphaTable;
	Info.fTileSize = m_fTileSize;
	Info.fHeightMultiply = m_fHeightMultiply;
	Info.fTextureDistance = 5000.f;
	Info.Type = TT_NORMAL;
	memcpy( Info.fGrassWidth, fGrassWidth, sizeof(Info.fGrassWidth) );
	memcpy( Info.fMinGrassHeight, fGrassHeightMin, sizeof(Info.fMinGrassHeight) );
	memcpy( Info.fMaxGrassHeight, fGrassHeightMax, sizeof(Info.fMaxGrassHeight) );
	Info.fMinShake = fShakeMin;
	Info.fMaxShake = fShakeMax;
	Info.pGrassBuffer = pGrassTable;

	EtVector3 vPos = m_Offset;
	vPos.x -= ( m_pParentGrid->GetGridWidth() * 100.f ) / 2.f;
	vPos.z -= ( m_pParentGrid->GetGridHeight() * 100.f ) / 2.f;
	Info.TerrainOffset = vPos;

	m_Handle = EternityEngine::CreateTerrain( ((CDnWorld*)GetParentGrid()->GetWorld())->GetRoom(), &Info );
	m_Handle->InitializeBlock( -1 );
}

bool CDnWorldSector::LoadSound( const char *szSectorPath )
{
	return true;
}

bool CDnWorldSector::LoadWater( const char *szSectorPath )
{
	return true;
}

bool CDnWorldSector::LoadDecal( const char *szSectorPath )
{
	return true;
}

CEtWorldWater *CDnWorldSector::AllocWater()
{
	return NULL;
}

CEtWorldSound *CDnWorldSector::AllocSound()
{
	return NULL;
}

CEtWorldDecal *CDnWorldSector::AllocDecal()
{
	return NULL;
}

void CDnWorldSector::InsertProcessProp( CDnWorldProp *pProp )
{
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	if( pProp->GetPropType() == PTE_Action )
		m_pVecProcessProp[0].push_back( pProp );
	else m_pVecProcessProp[1].push_back( pProp );
#else
	m_pVecProcessProp.push_back( pProp );
#endif
}

void CDnWorldSector::RemoveProcessProp( CDnWorldProp *pProp )
{
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	for( int j=0; j<2; j++ ) {
		for( DWORD i=0; i<m_pVecProcessProp[j].size(); i++ ) {
			if( m_pVecProcessProp[j][i] == pProp ) {
				m_pVecProcessProp[j].erase( m_pVecProcessProp[j].begin() + i );
				break;
			}
		}
	}
#else
	for( DWORD i=0; i<m_pVecProcessProp.size(); i++ ) {
		if( m_pVecProcessProp[i] == pProp ) {
			m_pVecProcessProp.erase( m_pVecProcessProp.begin() + i );
			break;
		}
	}
#endif
}

CEtTrigger *CDnWorldSector::AllocTrigger()
{
#if defined( PRE_TRIGGER_TEST )
	return new CDnTestTrigger(this);
#else // #if defined( PRE_TRIGGER_TEST )
	return new CDnTrigger(this);
#endif // #if defined( PRE_TRIGGER_TEST )
}

bool CDnWorldSector::LoadProp( const char *szSectorPath, int nBlockIndex, bool bThreadLoad )
{
	bool bResult = CEtWorldSector::LoadProp( szSectorPath, nBlockIndex, bThreadLoad );
	if( bResult ) {
		char szTemp[_MAX_PATH] = { 0, };

		sprintf_s( szTemp, "%s\\ColBuild.ini", szSectorPath );

		CResMngStream Stream( szTemp );
		if( Stream.IsValid() ) {
			CMultiRoom *pRoom = ((CDnWorld*)GetParentGrid()->GetWorld())->GetRoom();
			CEtCollisionMng::GetInstance(pRoom).Load( &Stream );
		}
		/*
		for( DWORD i=0; i<m_pVecProcessProp.size(); i++ ) {
			CDnWorldActProp *pProp = (CDnWorldActProp *)m_pVecProcessProp[i];
			if( pProp->GetPropType() == PTE_Action && pProp->IsStaticCollision() ) {
				DeleteProp( pProp );
				SAFE_DELETE( pProp );
				m_pVecProcessProp.erase( m_pVecProcessProp.begin() + i );
				i--;
			}
		}
		*/
	}
	return bResult;
}
