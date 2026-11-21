#include "StdAfx.h"
#include "DnMinimap.h"
#include "DnWorld.h"
#include "EtWorldSector.h"
#include "DnActor.h"
#include "PerfCheck.h"
#include "DnPartyTask.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "DnWorld.h"
#include "EtWorldEventArea.h"
#include "EtDrawQuad.h"
#include "DnNPCActor.h"
#include "DnMonsterActor.h"
#include "DnBridgeTask.h"
#include "DnWorldData.h"
#include "DnWorldDataManager.h"
#include "DnPlayerActor.h"
#include "DnInterface.h"
#include "DnMainDlg.h"
#include "DnPvPGameTask.h"
#include "DnStateBlow.h"
#include "DnQuestTask.h"
#include "DnGuildWarSituationMng.h"
#include "DnRevengeTask.h"
#ifdef PRE_ADD_PVP_TOURNAMENT
#include "DnPVPTournamentDataMgr.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#define HIDE_DEGREE			25.0f
#define MINIMAP_ICON_COUNT	((float)indexAmount)
#define MINIMAP_ICON_SIZE	32
#define MINIMAP_TEXTURE_WIDTH		256
#define MINIMAP_TEXTURE_HEIGHT		512

const int MINIMAP_ICON_DIVIDE_COUNT = MINIMAP_TEXTURE_WIDTH / MINIMAP_ICON_SIZE; // Row wrap unit: 8 items = 256 / 32
const int MINIMAP_ICON_WIDTH_COUNT = MINIMAP_TEXTURE_WIDTH / MINIMAP_ICON_SIZE;
const int MINIMAP_ICON_HEIGHT_COUNT	= MINIMAP_TEXTURE_HEIGHT / MINIMAP_ICON_SIZE;

using namespace EternityEngine;
extern CSyncLock *g_pEtRenderLock;
extern bool g_bSkipScene;

CDnMinimap::CDnMinimap(void)
: m_bShow(true)
, m_fZoom(MIN_MINIMAP_ZOOM)
, m_bShowFunctionalNPC ( true )
, m_bShowOtherNPC( true )
, m_fTotalTime( 0.f )
, m_LocalTime( 0 )
, m_fElapsedTime( 0.0f )
, m_IsAllPlayerShow( false )
{
}

CDnMinimap::~CDnMinimap(void)
{
	SAFE_RELEASE_SPTR( m_hRenderTarget );
	SAFE_RELEASE_SPTR( m_hMaterial );

	SAFE_RELEASE_SPTR( m_hMinimap );
	SAFE_RELEASE_SPTR( m_hEnemy );
	SAFE_RELEASE_SPTR( m_hOpacity );

	m_mHarvestInfo.clear();
}

void CDnMinimap::Initialize()
{
	if( !m_hMaterial )
	{
		m_hMaterial = LoadResource( "Minimap.fx", RT_SHADER );
	}

	if( !m_hRenderTarget )
	{
		m_hRenderTarget = CreateRenderTargetTexture( MINIMAP_SIZE, MINIMAP_SIZE );
	}

	m_fElapsedTime = 1.0f;
	m_strMinimapFileName.clear();
	m_IsAllPlayerShow = false;
}

CDnMinimap::emICON_INDEX CDnMinimap::ConvertNpcTypeToIconIndex( int nJobType )
{
	emICON_INDEX nIndexIcon = indexNPC;
	switch( nJobType )
	{
	case CDnNPCActor::typeNone:			nIndexIcon = indexNPC;				break;
	case CDnNPCActor::typeWeapon:		nIndexIcon = indexNPC_Weapon;		break;
	case CDnNPCActor::typeParts:		nIndexIcon = indexNPC_Parts;		break;
	case CDnNPCActor::typeNormal:		nIndexIcon = indexNPC_Normal;		break;
	case CDnNPCActor::typeStorage:		nIndexIcon = indexNPC_Storage;		break;
	case CDnNPCActor::typeMarket:		nIndexIcon = indexNPC_Market;		break;
	case CDnNPCActor::typeMail:			nIndexIcon = indexNPC_Mail;			break;
	case CDnNPCActor::typeGuild:		nIndexIcon = indexNPC_Guild;		break;
	case CDnNPCActor::typeWarriorSkill:	nIndexIcon = indexNPC_WarriorSkill;	break;
	case CDnNPCActor::typeArcherSkill:	nIndexIcon = indexNPC_ArcherSkill;	break;
	case CDnNPCActor::typeSoceressSkill:nIndexIcon = indexNPC_SoceressSkill;break;
	case CDnNPCActor::typeClericSkill:	nIndexIcon = indexNPC_ClericSkill;	break;
	case CDnNPCActor::typePlate:		nIndexIcon = indexNPC_Plate;		break;
	case CDnNPCActor::typeCompleteQuest: nIndexIcon =  indexNPC_AdventurerBoard; break;
	case CDnNPCActor::typeAdventureBoard: nIndexIcon = indexNPC_AdventurerBoard; break;
	case CDnNPCActor::typeVehicle: nIndexIcon =        indexNPC_Vehicle; break;
	case CDnNPCActor::typeTicket: nIndexIcon =         indexNPC_Ticket; break;
	case CDnNPCActor::typeAcademicSkill: nIndexIcon =  indexNPC_AcademicSkill; break;
	case CDnNPCActor::typeKaliSkill: nIndexIcon =  indexNPC_KaliSkill; break;
	case CDnNPCActor::typePcBang: nIndexIcon = indexdNPC_PcBang; break;
	case CDnNPCActor::typeRepair: nIndexIcon = indexdNPC_Repair; break;
	case CDnNPCActor::typePresent:
	case CDnNPCActor::typeEvent:		nIndexIcon = indexNPCPresent; break;
	}
	return nIndexIcon;
}

CDnMinimap::emICON_INDEX CDnMinimap::ConvertStateMarkToQuestIconIndex( int nStateMark )
{
	emICON_INDEX emQuestIndex = indexNone;
	if( nStateMark >= CDnNPCActor::QuestAvailable_Main && nStateMark < CDnNPCActor::MaxCount )
	{
		switch( nStateMark )
		{
		case CDnNPCActor::QuestAvailable_Main:
			emQuestIndex = indexQuestAvailable;
			break;
		case CDnNPCActor::QuestAvailable_Sub:
			emQuestIndex = indexSubQuestAvailable;
			break;
		case CDnNPCActor::QuestPlaying_Main:
			emQuestIndex = indexQuestPalying;
			break;
		case CDnNPCActor::QuestPlaying_Sub:
			emQuestIndex = indexSubQuestPalying;
			break;
		case CDnNPCActor::QuestRecompense_Main:
			emQuestIndex = indexQuestRecompense;
			break;
		case CDnNPCActor::QuestRecompense_Sub:
			emQuestIndex = indexSubQuestRecompense;
			break;
		case CDnNPCActor::MailReceiveNew:
			emQuestIndex = indexMail_New;
			break;
		case CDnNPCActor::CrossReQuestComplete:
			emQuestIndex = indexAdventurerBoard_Recompense;
			break;
		case CDnNPCActor::CrossFavorGrant:
			emQuestIndex = indexReputationFavorGrant;
			break;
		case CDnNPCActor::CrossFavorNow:
			emQuestIndex = indexReputationFavorNow;
			break;
		case CDnNPCActor::CrossFavorComplete:
			emQuestIndex = indexReputationFavorComplete;
			break;
		case CDnNPCActor::CrossQuestGlobalGrant:
			emQuestIndex = indexQuestGlobal_Available;
			break;
		case CDnNPCActor::CrossQuestGlobalNow:
			emQuestIndex = indexQuestGlobal_Playing;
			break;
		case CDnNPCActor::CrossQuestGlobalComplete:
			emQuestIndex = indexQuestGlobal_Recompense;
			break;
 		case CDnNPCActor::CrossNoticeWarpNpc:
			break;// Agreed not to show this one on the minimap

#ifdef PRE_ADD_58449
		case CDnNPCActor::CrossNoticePcBang:
			emQuestIndex = indexdNPC_Notice_PcBang;
			break;
#endif
			//KOR UPDATE.
		  case 29:
		  case 30:
			  emQuestIndex = (emICON_INDEX)73;
			break;
		  case 31:
		  case 32:
			  emQuestIndex = (emICON_INDEX)77;
			break;
		  case 33:
			  emQuestIndex = (emICON_INDEX)79;
			break;

		}
	}
	return emQuestIndex;
}

void CDnMinimap::InitializeNPC( int nMapIndex )
{
	// Initialize NPC
	CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( nMapIndex );
	if( !pWorldData ) return;
	int nCount = pWorldData->GetNpcCount();
	for( int i=0; i<nCount; i++ )
	{
		CDnWorldData::NpcData *pNpcData = pWorldData->GetNpcData( i );
		if( !pNpcData ) continue;

		SNPCQuestInfo npcQuestInfo;

		npcQuestInfo.vNpcPos = pNpcData->vPos;
		npcQuestInfo.nNpcID = pNpcData->nNpcID;
		npcQuestInfo.nIndexIcon = ConvertNpcTypeToIconIndex( pNpcData->nJobType );
		npcQuestInfo.emQuestIndex = ConvertStateMarkToQuestIconIndex( CDnNPCActor::CalcStateMark( pNpcData->nNpcID ) );
		bool bNpcShow = true;
		if( CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeVillage ) {
			// When on the game server, check s_pVecProcessList and process only currently visible NPCs.
			bNpcShow = false;
			for( DWORD i=0; i<CDnActor::s_pVecProcessList.size(); i++ ) {
				DnActorHandle hActor = CDnActor::s_pVecProcessList[i]->GetMySmartPtr();
				if( hActor && hActor->GetActorType() == CDnActorState::Npc && hActor->IsShow() ) 
				{
					CDnNPCActor *pNpcActor = dynamic_cast<CDnNPCActor*>(hActor.GetPointer());
					if( pNpcActor && pNpcActor->GetNpcData().nNpcID == pNpcData->nNpcID ) {
						bNpcShow = true;
						break;
					}
				}
			}
		}
		npcQuestInfo.bShow = bNpcShow;

		m_vecNPCQuestInfo.push_back( npcQuestInfo );
	}

	if( GetInterface().GetMainBarDialog() ) {
		GetInterface().GetMainBarDialog()->UpdateMinimapOption(); //UpdateMinimapOption();
	}
}

void CDnMinimap::ShowNpcQuestInfo( int nNpcID, bool bShow )
{
	for( DWORD i=0; i<m_vecNPCQuestInfo.size(); i++ ) {
		if( m_vecNPCQuestInfo[i].nNpcID == nNpcID ) {
			m_vecNPCQuestInfo[i].bShow = bShow;
			break;
		}
	}
}

void CDnMinimap::ReInitializeNPC( int nMapIndex )
{
	m_vecNPCQuestInfo.clear();
	InitializeNPC( nMapIndex );
}

void CDnMinimap::SetTextures( const char *pMinimap )
{
	SAFE_RELEASE_SPTR( m_hMinimap );
	SAFE_RELEASE_SPTR( m_hEnemy );
	SAFE_RELEASE_SPTR( m_hOpacity );

#ifdef PRE_FIX_MATERIAL_DUMP
	if( !m_hMaterial ) return;
#endif

	// Note : Store the minimap filename (*.dds).
	//
	m_strMinimapFileName = pMinimap;

	m_hMinimap = LoadResource( CEtResourceMng::GetInstance().GetFullName( pMinimap ).c_str(), RT_TEXTURE );
	if( !m_hMinimap )
	{
		m_hMinimap = LoadResource( CEtResourceMng::GetInstance().GetFullName( "Minimap.dds" ).c_str(), RT_TEXTURE );
	}

	m_hEnemy = LoadResource( CEtResourceMng::GetInstance().GetFullName( "MiniMapIcon.dds" ).c_str(), RT_TEXTURE );
	m_hOpacity = LoadResource( CEtResourceMng::GetInstance().GetFullName( "Opacity.dds" ).c_str(), RT_TEXTURE );

	int nTexIndex(0);

	nTexIndex = m_hMinimap->GetMyIndex();
	AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_MinimapTex", &nTexIndex );

	nTexIndex = m_hOpacity->GetMyIndex();
	AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_OpacityTex", &nTexIndex );

	nTexIndex = m_hEnemy->GetMyIndex();
	AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_EnemyTex", &nTexIndex );
}

void CDnMinimap::Zoom( float fZoomValue )
{
	m_fZoom += fZoomValue;
	m_fZoom = EtClamp( m_fZoom, MIN_MINIMAP_ZOOM, MAX_MINIMAP_ZOOM);
}

void CDnMinimap::RenderCustom( float fElapsedTime )
{
	if( !CDnActor::s_hLocalActor )
		return;

#ifdef PRE_FIX_MATERIAL_DUMP
	if( !m_hMaterial ) return;
#endif

	if( m_fElapsedTime > 0.016666f )
	{
		const EtVector3 *pPlayerPos = CDnActor::s_hLocalActor->GetPosition();

		bool bEnableZ = CEtDevice::GetInstance().EnableZ( false );
		CEtDevice::GetInstance().SetDepthStencilSurface( NULL );
		CEtDevice::GetInstance().SetRenderTarget( m_hRenderTarget->GetSurfaceLevel() );
			
		float fMapWidth = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 100.0f;
		float fMapHeight = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 100.0f;
		CEtWorldSector *pSector = CDnWorld::GetInstance().GetSector( pPlayerPos->x, pPlayerPos->z );
		if( !pSector ) return;

		const EtVector3 *pOffset = pSector->GetOffset();
		EtVector3 CalcPos;
		CalcPos.x = ( pPlayerPos->x + pOffset->x ) / fMapWidth + 0.5f;
		CalcPos.z = 0.5f - ( pPlayerPos->z + pOffset->z ) / fMapHeight;

		float fStartU = CalcPos.x - ( 0.5f / m_fZoom );
		float fStartV = CalcPos.z - ( 0.5f / m_fZoom );

		bool bAlphaBlend = GetEtDevice()->EnableAlphaBlend( false );

		// Do not use alpha test via render state. Handle with clip in shader
		DWORD dwAlphaTest=0;
		GetEtDevice()->GetRenderState( D3DRS_ALPHATESTENABLE, &dwAlphaTest);
		GetEtDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE);
		DrawQuadIm( m_hMaterial, m_vecCustomParam, 0, EtVector2( 0.0f, 0.0f ), EtVector2( 1.0f, 1.0f ), 
			EtVector2( fStartU, fStartV ), EtVector2( fStartU + 1.0f / m_fZoom, fStartV + 1.0f / m_fZoom ) );

		m_vecOutInfo.clear();

		ScanActor( pPlayerPos );
		GetEtDevice()->EnableAlphaBlend( true );		
		RenderGate( pPlayerPos );
		RenderEmptyArea( pPlayerPos );
		RenderHarvest( pPlayerPos );

		RenderPlayer( false);

		RenderActor( m_vechEnemy, pPlayerPos, indexEnemy );
		RenderActor( m_vechBoss, pPlayerPos, indexBoss );
		RenderActor( m_vechFriend, pPlayerPos, indexParty );
		RenderNPC( m_vechNpc, pPlayerPos );
		RenderTraceQuest( pPlayerPos );
#if defined( PRE_ADD_REVENGE )
		RenderRevengeUser( pPlayerPos );
#endif	// #if defined( PRE_ADD_REVENGE )

		RenderPartyMember( pPlayerPos );
		RenderPlayer( true);

		GetEtDevice()->EnableAlphaBlend( bAlphaBlend );
		GetEtDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, dwAlphaTest);

		CEtDevice::GetInstance().RestoreRenderTarget();
		CEtDevice::GetInstance().RestoreDepthStencil();
		CEtDevice::GetInstance().EnableZ( bEnableZ );

		m_fElapsedTime = 0.0f;
	}
}

void CDnMinimap::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_fElapsedTime += fDelta;
	m_fTotalTime += fDelta;
	m_LocalTime = LocalTime;
}

void CDnMinimap::ScanActor( const EtVector3 *pPlayerPos )
{
	if( !CDnActor::s_hLocalActor )
		return;

	float fMapWidthHalf = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeightHalf = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
	float fMaxSizeHalf = max( fMapWidthHalf, fMapHeightHalf );
	
	m_vechNpc.clear();
	m_vechBoss.clear();
	m_vechEnemy.clear();
	m_vechFriend.clear();
#if defined( PRE_ADD_REVENGE )
	m_vecRevengeUser.clear();
#endif	//	#if defined( PRE_ADD_REVENGE )

	std::vector<DnActorHandle> vecActorList;
	CDnActor::ScanActor( *(EtVector3*)pPlayerPos, fMaxSizeHalf / m_fZoom * 0.9f, vecActorList );

	for( int i = 0; i < (int)vecActorList.size(); i++ )
	{
		if( !vecActorList[i] )
			continue;

		CDnActor *pActor = vecActorList[i].GetPointer();
		if( pActor->IsPlayerActor() )
		{
			if( IsPVPGame() )
			{
#if defined( PRE_ADD_REVENGE )
				CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>( pActor );
				CDnRevengeTask * pRevengeTask = (CDnRevengeTask *)CTaskManager::GetInstance().GetTask( "RevengeTask" );
				if( pRevengeTask )
				{
					UINT eRevengeState = Revenge::RevengeTarget::eRevengeTarget_None;
					pRevengeTask->GetRevengeUserID( pPlayer->GetUniqueID(), eRevengeState );

					if( Revenge::RevengeTarget::eRevengeTarget_Target == eRevengeState 
						|| Revenge::RevengeTarget::eRevengeTarget_Me == eRevengeState )
						m_vecRevengeUser.push_back( vecActorList[i] );
				}
#endif
			}
		}
		else if( pActor->IsMonsterActor() ) 
		{			
			if( pActor->IsDie() ) continue;
			if( !pActor->IsShow() ) continue;
			if( !pActor->IsShowExposureInfo() ) continue;

			if( pActor->GetTeam() != CDnActor::s_hLocalActor->GetTeam() ) 
			{
				if( ((CDnMonsterActor*)(pActor))->GetGrade() >= CDnMonsterState::Boss )
				{
					m_vechBoss.push_back( vecActorList[i] );
				}
				else
				{
					m_vechEnemy.push_back( vecActorList[i] );
				}
			}
			else
			{
				m_vechFriend.push_back( vecActorList[i] );
			}
		}
		else if( pActor->IsNpcActor() )
		{
			m_vechNpc.push_back( vecActorList[i] );
		}
	}
}

void CDnMinimap::RenderActor( std::vector<DnActorHandle> &vechActor, const EtVector3 *pPlayerPos, emICON_INDEX iconIndex )
{
	if( !CDnActor::s_hLocalActor )
		return;

	float fMapWidthHalf = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeightHalf = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
	float fMaxSizeHalf = max( fMapWidthHalf, fMapHeightHalf );

	int nSize = (int)vechActor.size();
	if( nSize == 0 ) return;
	InitVertexIndex( nSize );

	int nCnt = 0;
	for( int i = 0; i < nSize; i++ )
	{
		if( CDnBridgeTask::GetInstance().IsPvPGameMode( PvPCommon::GameMode::PvP_GuildWar ) )
		{
			if( PvPCommon::Team::A == vechActor[i]->GetTeam() )
				iconIndex = indexParty;
			else if( PvPCommon::Team::B == vechActor[i]->GetTeam() )
				iconIndex = indexEnemy;
		}
		EtVector3 pActorPos = vechActor[ i ]->FindAutoTargetPos();
		EtVector3 ScreenPos = ( pActorPos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );

		BYTE btAlpha = 255;
		EtAniObjectHandle hObject = vechActor[ i ]->GetObjectHandle();
		if( hObject ) {
			btAlpha = (BYTE)(255 * hObject->GetObjectAlpha());
		}

		SetVertexIndex( nCnt, &ScreenPos, iconIndex, false, btAlpha );
		nCnt++;
	}

	CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], nSize * 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], nSize * 2 );
}

void CDnMinimap::RenderNPC( std::vector<DnActorHandle> &vechActor, const EtVector3 *pPlayerPos )
{
	float fMapWidthHalf = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeightHalf = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
	float fMaxSizeHalf = max( fMapWidthHalf, fMapHeightHalf );

	// Update Quest StateMark

	int nSize = (int)vechActor.size();
	for( int i = 0; i < nSize; i++ )
	{
		CDnNPCActor *pNpcActor = dynamic_cast<CDnNPCActor*>(vechActor[i].GetPointer());
		if( !pNpcActor ) continue;
		int nNPCStateMark = pNpcActor->GetNPCStateMark();

		emICON_INDEX emQuestIndex = ConvertStateMarkToQuestIconIndex( nNPCStateMark);

		int nNpcID = pNpcActor->GetNpcData().nNpcID;
		for( int j = 0; j < (int)m_vecNPCQuestInfo.size(); j++) 
		{
			if( m_vecNPCQuestInfo[j].nNpcID == nNpcID ) {
				m_vecNPCQuestInfo[j].emQuestIndex = emQuestIndex;
			}
		}
	}

	nSize = (int)m_vecNPCQuestInfo.size();
	InitVertexIndex( nSize );
	int nCnt = 0;
	std::vector<SNPCQuestInfo *> pVecQuestIconList;
	for( int i = 0; i < nSize; i++ )
	{
		if( !m_vecNPCQuestInfo[i].bShow ) continue;
		EtVector3 ActorPos( m_vecNPCQuestInfo[i].vNpcPos.x, 0, m_vecNPCQuestInfo[i].vNpcPos.y);
		EtVector3 ScreenPos = ( ActorPos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );

		if( !m_bShowFunctionalNPC && m_vecNPCQuestInfo[i].nIndexIcon != indexNPC ) continue;
		if( !m_bShowOtherNPC && m_vecNPCQuestInfo[i].nIndexIcon == indexNPC ) continue;

		SetVertexIndex( nCnt++, &ScreenPos, m_vecNPCQuestInfo[i].nIndexIcon, false , 255, (m_vecNPCQuestInfo[i].nIndexIcon == indexNPC ) ? 0.8f : 1.0f );

		if( m_vecNPCQuestInfo[i].emQuestIndex != indexNone ) {
			pVecQuestIconList.push_back( &m_vecNPCQuestInfo[i] );
		}
	}
	if( nSize != 0 ) {
		CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], nSize * 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], nSize * 2 );
	}

	// Note: Display quest info icon.
	//
	if( !pVecQuestIconList.empty() ) {
		DWORD dwCount = (DWORD)pVecQuestIconList.size();
		InitVertexIndex( (int)dwCount );
		for( DWORD i=0; i<dwCount; i++ ) {
			EtVector3 ActorPos( pVecQuestIconList[i]->vNpcPos.x, 0, pVecQuestIconList[i]->vNpcPos.y );
			EtVector3 ScreenPos = ( ActorPos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );
			ScreenPos.z += 0.2f;		

			SetVertexIndex( i, &ScreenPos, pVecQuestIconList[i]->emQuestIndex, false );
		}
		CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], dwCount * 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], dwCount * 2 );
	}
} 

void CDnMinimap::InsertNpc( DnActorHandle hActor )
{
	CDnNPCActor *pNpcActor = dynamic_cast<CDnNPCActor*>(hActor.GetPointer());
	if(!pNpcActor) return;

	std::vector< SNPCQuestInfo >::iterator it = m_vecNPCQuestInfo.begin();
	while( it != m_vecNPCQuestInfo.end() ) {
		if( it->nNpcID == pNpcActor->GetNpcData().nNpcID ) { 
			EtVector2 vDist = (it->vNpcPos - pNpcActor->GetMatEx()->GetPositionXZ());
			if( EtVec2LengthSq(&vDist) < 0.001f ) {	// Do not add if an identical one at the same position already exists.
				return ;
			}
		}
		++it;
	}

	SNPCQuestInfo NPCQuestInfo;
	NPCQuestInfo.nNpcID = pNpcActor->GetNpcData().nNpcID;
	NPCQuestInfo.vNpcPos = pNpcActor->GetMatEx()->GetPositionXZ();
	NPCQuestInfo.nIndexIcon = ConvertNpcTypeToIconIndex( pNpcActor->GetNPCJobType() );
	NPCQuestInfo.emQuestIndex = ConvertStateMarkToQuestIconIndex( pNpcActor->GetNPCStateMark() );
	m_vecNPCQuestInfo.push_back( NPCQuestInfo );
}

void CDnMinimap::RemoveNpc( DnActorHandle hActor )
{
	CDnNPCActor *pNpcActor = dynamic_cast<CDnNPCActor*>(hActor.GetPointer());
	if(!pNpcActor) return;

	std::vector< SNPCQuestInfo >::iterator it = m_vecNPCQuestInfo.begin();
	while( it != m_vecNPCQuestInfo.end() ) {
		if( it->nNpcID == pNpcActor->GetNpcData().nNpcID ) {
			it = m_vecNPCQuestInfo.erase( it );
		}
		else {
			++it;
		}
	}
}

void CDnMinimap::RenderPlayer( bool bAnimateAlpha )
{
	if( !CDnActor::s_hLocalActor )
		return;

	InitVertexIndex( 1 );

	BYTE cAlpha = 255;
	if( bAnimateAlpha ) {
		const float fAnimateTerm = 2.f;
		cAlpha = BYTE(255 * sinf( ET_PI *  fmodf( m_fTotalTime ,fAnimateTerm ) / fAnimateTerm ));
	}

	SetVertexIndex( 0, &EtVector3(0.0f,0.0f,0.0f), indexPlayer, true, cAlpha, 0.8f);
	CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], 2 );
}


void CDnMinimap::RenderPartyMember( const EtVector3 *pPlayerPos )
{
	if( !CDnActor::s_hLocalActor )
		return;

	CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm ) 
		return;

	float fMaxSizeHalf = max( CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f, CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f );

	static std::vector<SPartyMemberPosInfo> vVecPos;
	static std::vector< DnActorHandle > vecActorList;
	vVecPos.clear();
	vecActorList.clear();
	
	GetAvailablePartyMember( vecActorList , vVecPos );

	if( !vVecPos.empty() ) 
	{
		int nSize = (int)vVecPos.size();
		InitVertexIndex(nSize);
		int nParty = 0;

		for( int i = 0; i < ( int )vVecPos.size(); i++ )
		{
			EtVector3 *pEnemyPos = &(vVecPos[i].pos);
			EtVector3 ScreenPos = ( *pEnemyPos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );

			if( IsMinimapOut( *pPlayerPos, *pEnemyPos, fMaxSizeHalf, m_fZoom ) )
			{
				InsertOutInfo( pPlayerPos , &vVecPos[i] );
				continue;
			}

			SetVertexIndex( nParty, &ScreenPos, vVecPos[i].eIconIndex, false );
			nParty++;
		}

		CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], nSize * 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], nSize * 2 );
	}

	static std::vector< boost::tuple<DnActorHandle, int, DWORD> > vecRadioRenders;
	vecRadioRenders.clear();
	for( std::vector< boost::tuple< DnActorHandle, int, LOCAL_TIME> >::iterator it = m_RadioActors.begin(); it != m_RadioActors.end();  ) {

		DnActorHandle hActor = it->get<0>();
		int nIconIndex = it->get<1>();
		LOCAL_TIME StartTime = it->get<2>();

		DWORD dwAlpha = CDnHeadIcon::GetHeadIconAlpha( (DWORD)(m_LocalTime - StartTime) );

		if( dwAlpha == 0 ) {
			it = m_RadioActors.erase( it );
			continue;
		}

		int j;
		for( j = 0; j < (int)vecActorList.size(); j++) {
			if( vecActorList[j] == hActor ) {
				vecRadioRenders.push_back( boost::make_tuple(hActor, nIconIndex, dwAlpha) );
				break;
			}
		}
		if( j == (int) vecActorList.size() ) {
			it = m_RadioActors.erase( it );
		}
		else {
			++it;
		}
	}
	if( !vecRadioRenders.empty() ) 
	{
		InitVertexIndex( (int)vecRadioRenders.size() );
		for( int i = 0; i < (int)vecRadioRenders.size(); i++ ) {
			EtVector3 ActorPos( vecRadioRenders[i].get<0>()->GetPosition()->x, 0, vecRadioRenders[i].get<0>()->GetPosition()->z);
			EtVector3 ScreenPos = ( ActorPos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );
			ScreenPos.z += 0.2f;		

			int nIconIndex = vecRadioRenders[i].get<1>();

			emICON_INDEX Icons[] = { indexRadioMsgHeal, indexRadioMsgAttack, indexRadioMsgFollow, 
				indexRadioMsgBack, indexRadioMsgStop, indexRadioMsgHelp };

			SetVertexIndex( i, &ScreenPos, Icons[ nIconIndex ], false, (BYTE)vecRadioRenders[i].get<2>()  );
		}
		CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], (int)vecRadioRenders.size() * 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], (int)vecRadioRenders.size() * 2);
	}
}

CDnMinimap::emICON_INDEX CDnMinimap::ConvertPartyMarkToPartyIconIndex( int classId )
{
	emICON_INDEX nIndexIcon = indexParty;
	switch( classId - 1 )
	{
	case CDnActorState::Warrior:	nIndexIcon = indexPartyMemberWarrior;	break;
	case CDnActorState::Archer:		nIndexIcon = indexPartyMemberArcher;	break;
	case CDnActorState::Soceress:	nIndexIcon = indexPartyMemberSoceress;	break;
	case CDnActorState::Cleric:		nIndexIcon = indexParty;				break;
#ifdef PRE_ADD_ACADEMIC
	case CDnActorState::Academic:   nIndexIcon = indexPartyMemberAcademic;  break;
#endif
#ifdef PRE_ADD_KALI
	case CDnActorState::Kali: nIndexIcon = indexPartyMemberKali; break;
#endif
	}

	return nIndexIcon;
}

void CDnMinimap::GetAvailablePartyMember( std::vector<DnActorHandle> &ResultActorList , std::vector<SPartyMemberPosInfo> &ResultPosList )
{
	CDnPartyTask *pPartyTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
	if( !pPartyTask || pPartyTask->GetPartyRole() == CDnPartyTask::SINGLE )	
		return;

	for( DWORD i=0; i<pPartyTask->GetPartyCount(); i++ )
	{
		CDnPartyTask::PartyStruct *pPartyStruct = pPartyTask->GetPartyData(i);		
		if( pPartyStruct )
		{
			if( pPartyStruct->bGMTrace || pPartyStruct->usTeam == PvPCommon::Team::Observer )
				continue;

			if( pPartyStruct->nSessionID == CDnBridgeTask::GetInstance().GetSessionID() ) 
				continue;

			if( CheckPvPCondition( pPartyStruct->hActor) == false )
				continue;
		
			SPartyMemberPosInfo info;

			if( pPartyStruct->hActor )
			{
				info.pos = *pPartyStruct->hActor->GetPosition();
				info.classId = pPartyStruct->hActor->GetClassID();
				info.team = pPartyStruct->hActor->GetTeam();
				info.eIconIndex = ConvertPartyMarkToPartyIconIndex( pPartyStruct->hActor->GetClassID() );
				info.eIconIndex = ConvertPvPMark( info.eIconIndex, pPartyStruct->hActor );

				ResultPosList.push_back( info );
				ResultActorList.push_back( pPartyStruct->hActor );
			}
			else
			{
				info.pos = pPartyStruct->vPos;
				info.classId = pPartyStruct->cClassID;
				info.team = pPartyStruct->usTeam;
				info.eIconIndex = ConvertPartyMarkToPartyIconIndex( pPartyStruct->cClassID );

				ResultPosList.push_back( info );
				ResultActorList.push_back( CDnActor::Identity() );
			}
		}
	}
}

bool CDnMinimap::CheckPvPCondition( DnActorHandle hActor )
{
	if( !hActor )
		return false;

	bool bIsObserver = CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer;
	bool bIsSameTeam = false;
	
	if( CDnActor::s_hLocalActor->GetTeam() == hActor->GetTeam() 
		|| CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer && hActor->GetTeam() == PvPCommon::Team::A )
	{
		bIsSameTeam = true;
	}

	CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP )
	{
		CDnPvPGameTask *pPvPGameTask = static_cast<CDnPvPGameTask*>(pGameTask);
		if( pPvPGameTask )
		{
			switch( pPvPGameTask->GetGameMode() )
			{
				case PvPCommon::GameMode::PvP_Captain:
					{
						CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
						if(pPlayer && pPlayer->IsCaptainMode())
							return true;
					}
					break;
				case PvPCommon::GameMode::PvP_Zombie_Survival:
					{
						CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
						CDnPlayerActor *pLocalActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
						if( pLocalActor && pPlayer && pLocalActor->IsTransformMode() != pPlayer->IsTransformMode() )
							bIsSameTeam = false;
					}
					break;
				case PvPCommon::GameMode::PvP_GuildWar:
					{
						if( bIsSameTeam == false )
						{
							if( CDnActor::s_hLocalActor->GetStateBlow()->IsExistStateBlowFromBlowIndex( STATE_BLOW::BLOW_210 ) )
								return true;
						}
					}
					break;
#ifdef PRE_ADD_PVP_TOURNAMENT
				case PvPCommon::GameMode::PvP_Tournament:
					{
						const CDnPVPTournamentDataMgr* pTournamentMgr = pPvPGameTask->GetTournamentDataMgr();
						if( pTournamentMgr )
						{
							if( pTournamentMgr->IsInBattlePlayer( hActor->GetUniqueID() ) )
								return true;
						}
					}
					break;
#endif
			}

			if( bIsSameTeam == false )
			{
				CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
				if( pPlayer )
					return ( bIsObserver || pPlayer->GetExposure() || m_IsAllPlayerShow );
			}
		}
	}

	return bIsSameTeam;
}


CDnMinimap::emICON_INDEX CDnMinimap::ConvertPvPMark( emICON_INDEX original, DnActorHandle hActor )
{
	CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP )
	{
		CDnPvPGameTask *pPvPGameTask = static_cast<CDnPvPGameTask*>(pGameTask);
		if( pPvPGameTask )
		{
			bool bIsSameTeam = false;

			if( hActor->GetTeam() == CDnActor::s_hLocalActor->GetTeam() 
				|| ( CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer && hActor->GetTeam() == PvPCommon::Team::A ) )
			{
				bIsSameTeam = true;
			}
			CDnGuildWarSituationMng * pGuildWarMng = GetInterface().GetGuildWarSituation();
			if( pGuildWarMng )
			{
				if( pGuildWarMng->IsPartyMember( hActor ) )
					return indexNPC;
				else if( hActor->GetTeam() == PvPCommon::Team::A )
					return indexParty;
				else
					return indexEnemy;
			}
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>( hActor.GetPointer() );
			if( pPlayer && pPlayer->IsCaptainMode() )
			{
				return bIsSameTeam ? indexAllyCaptain : indexEnemyCaptain;
			}

			if( pPvPGameTask->GetGameMode() == PvPCommon::GameMode::PvP_IndividualRespawn )
				bIsSameTeam = false;

			return bIsSameTeam ? indexParty : indexEnemy;
		}
	}

	return original;
}


void CDnMinimap::InsertOutInfo( const EtVector3 *pPlayerPos, SPartyMemberPosInfo *pMemberInfo )
{
	if( !CDnActor::s_hLocalActor )
		return;

	switch( pMemberInfo->eIconIndex )
	{
	case indexAllyCaptain:
		{
			m_vecOutInfo.push_back( SOutInfo( pMemberInfo->eIconIndex , GetDistanceDegree( *pPlayerPos, pMemberInfo->pos ) ) );
		}
		break;
	case indexEnemyCaptain:
		{
			if( CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer )
				m_vecOutInfo.push_back( SOutInfo( pMemberInfo->eIconIndex , GetDistanceDegree( *pPlayerPos, pMemberInfo->pos ) ) );
		}
		break;
	}
}

void CDnMinimap::RenderGate( const EtVector3 *pPlayerPos )
{
	DWORD dwCount = CDnWorld::GetInstance().GetGateCount();
	if( dwCount == 0 ) return;

	float fMapWidthHalf = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeightHalf = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
	float fMaxSizeHalf = max( fMapWidthHalf, fMapHeightHalf );

	InitVertexIndex(dwCount);

	CDnWorld::GateStruct *pGateStruct(NULL);
	int nGateIndex(0);

	for( DWORD i=0; i<dwCount; i++ )
	{
		pGateStruct = CDnWorld::GetInstance().GetGateStructFromIndex( i );
		if( !pGateStruct ) continue;

		emICON_INDEX indexIcon;
		if( pGateStruct->PermitFlag == CDnWorld::PermitEnter )
		{
			switch( pGateStruct->MapType )
			{
			case CDnWorld::MapTypeUnknown:	indexIcon = indexGateUnknown;	break;
			case CDnWorld::MapTypeVillage:	indexIcon = indexGateVillage;	break;
			case CDnWorld::MapTypeWorldMap:	indexIcon = indexGateWorldMap;	break;
			case CDnWorld::MapTypeDungeon:	indexIcon = indexGateDungeon;	break;
			default:						indexIcon = indexGateX;			break;
			}
		}
		else
		{
			indexIcon = indexGateX;
		}

		if( !pGateStruct->pGateArea ) continue;
		SOBB *pOBB = pGateStruct->pGateArea->GetOBB();
		if( !pOBB ) continue;
		EtVector3 *pGatePos = &pOBB->Center;
		if( !pGatePos ) continue;

		if( IsMinimapOut( *pPlayerPos, *pGatePos, fMaxSizeHalf, m_fZoom ) )
			continue;

		EtVector3 ScreenPos = ( *pGatePos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );
		SetVertexIndex( nGateIndex, &ScreenPos, indexIcon, false );
		nGateIndex++;
	}

	CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], dwCount * 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], dwCount * 2 );
}

void CDnMinimap::RenderTraceQuest( const EtVector3 *pPlayerPos )
{
	int nTraceQuestID = 0;
	
	if (CDnQuestTask::IsActive())
		nTraceQuestID = GetQuestTask().GetTraceQuestID();
	
	if( 0 != nTraceQuestID )
	{
		CDnQuestTask::QuestNotifierInfo* pInfo = GetQuestTask().GetQuestNotifierInfoByQuestIDForce( nTraceQuestID );

		if( !pInfo ) return;
		else if( pInfo->pathResult.empty() ) return;

		float fMapWidthHalf = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
		float fMapHeightHalf = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
		float fMaxSizeHalf = max( fMapWidthHalf, fMapHeightHalf );
		InitVertexIndex( 1 );

		EtVector3 vPos( pInfo->vTargetPos.x , 0 , pInfo->vTargetPos.y );
		if( IsMinimapOut( *pPlayerPos, vPos, fMaxSizeHalf, m_fZoom ) )
		{
			m_vecOutInfo.push_back( SOutInfo( indexTraceQuest , GetDistanceDegree( *pPlayerPos , vPos ) ) );
			return;
		}

		EtVector3 ScreenPos = ( vPos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );
		SetVertexIndex( 0, &ScreenPos, indexTraceQuest, false );
		CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], 2 );
	}
}

#if defined( PRE_ADD_REVENGE )
void CDnMinimap::RenderRevengeUser( const EtVector3 *pPlayerPos )
{
	if( !CDnActor::s_hLocalActor || m_vecRevengeUser.empty() )
		return;

	int nCount = (int)m_vecRevengeUser.size();

	InitVertexIndex( nCount );

	float fMapWidthHalf = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeightHalf = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
	float fMaxSizeHalf = max( fMapWidthHalf, fMapHeightHalf );

	for( int itr = 0; itr < nCount; ++itr )
	{
		EtVector3 vPos = *(m_vecRevengeUser[itr]->GetPosition());
		if( IsMinimapOut( *pPlayerPos, vPos, fMaxSizeHalf, m_fZoom ) )
		{
			m_vecOutInfo.push_back( SOutInfo( indexRevengeUser , GetDistanceDegree( *pPlayerPos , vPos ) ) );
			continue;
		}
		
		EtVector3 ScreenPos = ( vPos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );
		SetVertexIndex( 0, &ScreenPos, indexRevengeUser, false );
		CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], 2 );
	}
}
#endif	// #if defined( PRE_ADD_REVENGE )

void CDnMinimap::RenderEmptyArea( const EtVector3 *pPlayerPos )
{
	DWORD dwCount = (DWORD)m_mEmptyInfo.size();
	if( dwCount == 0 ) return;

	float fMapWidthHalf = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeightHalf = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
	float fMaxSizeHalf = max( fMapWidthHalf, fMapHeightHalf );

	InitVertexIndex(dwCount);

	int nHarvestIndex(0);
	for( std::map<int, std::pair<EtVector3,int>>::iterator itor = m_mEmptyInfo.begin(); itor != m_mEmptyInfo.end(); ++itor )
	{
		EtVector3 vPos = (*itor).second.first;
		if( IsMinimapOut( *pPlayerPos, vPos, fMaxSizeHalf, m_fZoom ) )
			continue;

		EtVector3 ScreenPos = ( vPos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );
		SetVertexIndex( nHarvestIndex, &ScreenPos, (emICON_INDEX)((*itor).second.second), false );
		nHarvestIndex++;
	}

	CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], dwCount * 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], dwCount * 2 );
}

void CDnMinimap::RenderHarvest( const EtVector3 *pPlayerPos )
{
	DWORD dwCount = (DWORD)m_mHarvestInfo.size();
	if( dwCount == 0 ) return;

	float fMapWidthHalf = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeightHalf = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;
	float fMaxSizeHalf = max( fMapWidthHalf, fMapHeightHalf );

	InitVertexIndex(dwCount);

	int nHarvestIndex(0);
	for( std::map<int, std::pair<EtVector3,int>>::iterator itor = m_mHarvestInfo.begin(); itor != m_mHarvestInfo.end(); ++itor )
	{
		EtVector3 vPos = (*itor).second.first;

		if( IsMinimapOut( *pPlayerPos, vPos, fMaxSizeHalf, m_fZoom ) )
			continue;

		EtVector3 ScreenPos = ( vPos - *pPlayerPos ) / ( fMaxSizeHalf / m_fZoom );
		SetVertexIndex( nHarvestIndex, &ScreenPos, (emICON_INDEX)((*itor).second.second), false );
		nHarvestIndex++;
	}

	CustomDrawIm( m_hMaterial, m_vecCustomParam, 1, &m_vecVertex[ 0 ], dwCount * 4, sizeof( STextureDiffuseVertex ), &m_vecIndex[ 0 ], dwCount * 2 );
}

bool CDnMinimap::PointInLine( const EtVector2 *pTest, const EtVector2 *pLineBegin, const EtVector2 *pLineEnd )
{
	ASSERT(pTest&&pLineBegin&&pLineEnd&&"CDnMinimap::PointInLine");

	//if( ((pLineBegin->x < pTest->x) == (pTest->x <= pLineEnd->x)) &&
	if( (((pTest->y - pLineEnd->y) * (pLineBegin->x - pLineEnd->x)) < (pLineBegin->y - pLineEnd->y) * (pTest->x - pLineEnd->x)) )
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CDnMinimap::GetLinePoint( const EtVector3 *pPlayerPos, EtVector2 *pLinePoint, float fDegree, float fRatio )
{
	ASSERT(pPlayerPos&&pLinePoint&&"CDnMinimap::GetLinePoint");

	DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
	if(!hCamera)
		return;

	EtVector3 vZDir = hCamera->GetMatEx()->m_vZAxis;
	vZDir.x = -vZDir.x;
	vZDir.y = 0.0f;
	vZDir.z = -vZDir.z;

	float fAngle = EtToRadian(fDegree);

	EtVector3 vTemp;
	vTemp.x = vZDir.x*cosf(fAngle) - vZDir.z*sinf(fAngle);
	vTemp.z = vZDir.z*cosf(fAngle) + vZDir.x*sinf(fAngle);

	vTemp *= fRatio;
	vTemp += (*pPlayerPos);

	pLinePoint->x = vTemp.x;
	pLinePoint->y = vTemp.z;
}

void CDnMinimap::InitVertexIndex( int nCount )
{
	m_vecVertex.resize( nCount * 4 );
	m_vecIndex.resize( nCount * 6 );
}

void CDnMinimap::SetVertexIndex( int nIndex, const EtVector3 *pScreenPos, emICON_INDEX indexIcon, bool bRotate, BYTE btAlpha, float fScale )
{
	ASSERT(pScreenPos&&"CDnMinimap::SetVertexIndex");
	static float fRenderRate = 0.5f; //0.609375f; // Note: Ratio between main bar display size (156) and target render size (256)
	static float fEnemySize = MINIMAP_ICON_SIZE / fRenderRate / MINIMAP_SIZE;

	m_vecVertex[nIndex*4].Position = EtVector3( (*pScreenPos).x - fEnemySize*fScale, (*pScreenPos).z + fEnemySize*fScale, 0.0f );
	m_vecVertex[nIndex*4+1].Position = EtVector3( (*pScreenPos).x + fEnemySize*fScale, (*pScreenPos).z + fEnemySize*fScale, 0.0f );
	m_vecVertex[nIndex*4+2].Position = EtVector3( (*pScreenPos).x + fEnemySize*fScale, (*pScreenPos).z - fEnemySize*fScale, 0.0f );
	m_vecVertex[nIndex*4+3].Position = EtVector3( (*pScreenPos).x - fEnemySize*fScale, (*pScreenPos).z - fEnemySize*fScale, 0.0f );

	m_vecVertex[nIndex*4].Color = D3DCOLOR_ARGB(btAlpha, 255, 255, 255);
	m_vecVertex[nIndex*4+1].Color = D3DCOLOR_ARGB(btAlpha, 255, 255, 255);
	m_vecVertex[nIndex*4+2].Color = D3DCOLOR_ARGB(btAlpha, 255, 255, 255);
	m_vecVertex[nIndex*4+3].Color = D3DCOLOR_ARGB(btAlpha, 255, 255, 255);

	if( bRotate )
	{
		RotateIcon( &(m_vecVertex[nIndex*4].Position), pScreenPos );
		RotateIcon( &(m_vecVertex[nIndex*4+1].Position), pScreenPos );
		RotateIcon( &(m_vecVertex[nIndex*4+2].Position), pScreenPos );
		RotateIcon( &(m_vecVertex[nIndex*4+3].Position), pScreenPos );
	}

	static float fIconSizeW = 1.0f / MINIMAP_ICON_WIDTH_COUNT;
	static float fIconSizeH = 1.0f / MINIMAP_ICON_HEIGHT_COUNT;

	int nWidthIndex = indexIcon % MINIMAP_ICON_DIVIDE_COUNT;
	int nHeightIndex = indexIcon / MINIMAP_ICON_DIVIDE_COUNT;

	m_vecVertex[nIndex*4].TexCoordinate =   EtVector2( fIconSizeW * nWidthIndex,			fIconSizeH * nHeightIndex );
	m_vecVertex[nIndex*4+1].TexCoordinate = EtVector2( fIconSizeW * ( nWidthIndex + 1 ),	fIconSizeH * nHeightIndex );
	m_vecVertex[nIndex*4+2].TexCoordinate = EtVector2( fIconSizeW * ( nWidthIndex + 1 ),	fIconSizeH * ( nHeightIndex + 1 ) );
	m_vecVertex[nIndex*4+3].TexCoordinate = EtVector2( fIconSizeW * nWidthIndex,			fIconSizeH * ( nHeightIndex + 1 ) );

	m_vecIndex[nIndex*6] = nIndex * 4;
	m_vecIndex[nIndex*6+1] = nIndex * 4 + 1;
	m_vecIndex[nIndex*6+2] = nIndex * 4 + 2;
	m_vecIndex[nIndex*6+3] = nIndex * 4 ;
	m_vecIndex[nIndex*6+4] = nIndex * 4 + 2;
	m_vecIndex[nIndex*6+5] = nIndex * 4 + 3;
}

void CDnMinimap::RotateIcon( EtVector3 *pPos, const EtVector3 *pScreenPos )
{
	DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
	if(!hCamera)
		return;

	EtVector3 vZDir, vCross, vTemp;
	vZDir = hCamera->GetMatEx()->m_vZAxis;
	vZDir.y = 0.0f;
	EtVec3Normalize( &vZDir, &vZDir );
	float fDegree = EtAcos(EtVec3Dot(&EtVector3(0.0f, 0.0f, 1.0f), &vZDir));
	EtVec3Cross( &vCross, &EtVector3(0.0f,0.0f,1.0f), &vZDir );
	if( vCross.y > 0.0f ) fDegree = -fDegree;

	D3DXMATRIX rMatrix;
	D3DXMatrixRotationZ( &rMatrix, fDegree );
	EtVector2 vOut, vIn;

	vIn.x = (*pPos).x - (*pScreenPos).x;
	vIn.y = (*pPos).y - (*pScreenPos).z;
	D3DXVec2TransformCoord( &vOut, &vIn, &rMatrix );
	(*pPos).x = vOut.x + (*pScreenPos).x;
	(*pPos).y = vOut.y + (*pScreenPos).z;
}

bool CDnMinimap::IsPVPGame()
{
	CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );

	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP ) return true;

	return false;
}

void CDnMinimap::SetRadioMark( DnActorHandle hActor, int nIconIndex )
{ 
	int i;
	for( i = 0; i < (int)m_RadioActors.size(); i++) {
		if( m_RadioActors[i].get<0>() == hActor && m_RadioActors[i].get<1>() == nIconIndex ) {
			m_RadioActors[i].get<2>() = hActor->GetLocalTime();
			break;
		}
	}

	if( i == (int)m_RadioActors.size() ) {
		m_RadioActors.push_back( boost::make_tuple( hActor, nIconIndex, hActor->GetLocalTime() ) ); 
	}
}

void CDnMinimap::InsertHarvest( int nAreaIndex, EtVector3 etVector3, char * szHarvestIconName )
{
	int nIndex = 0;

	if( stricmp( "LifeCrossHair07.dds", szHarvestIconName ) == 0 )
		nIndex = indexRoot;
	else if( stricmp( "LifeCrossHair06.dds", szHarvestIconName ) == 0 )
		nIndex = indexCereals;
	else if( stricmp( "LifeCrossHair05.dds", szHarvestIconName ) == 0 )
		nIndex = indexLeaf;
	else if( stricmp( "LifeCrossHair04.dds", szHarvestIconName ) == 0 )
		nIndex = indexFungus;
	else if( stricmp( "LifeCrossHair03.dds", szHarvestIconName ) == 0 )
		nIndex = indexFruit;

	m_mHarvestInfo.insert( std::make_pair( nAreaIndex, std::make_pair(etVector3,nIndex) ) );
}

void CDnMinimap::DeleteHarvest( int nAreaIndex )
{
	std::map<int, std::pair<EtVector3,int>>::iterator itor = m_mHarvestInfo.find(nAreaIndex);
	if( itor == m_mHarvestInfo.end() )
		return;

	m_mHarvestInfo.erase( itor );
}

void CDnMinimap::InsertEmptyArea( int nAreaIndex, EtVector3 etVector3 )
{
	m_mEmptyInfo.insert( std::make_pair( nAreaIndex, std::make_pair(etVector3, indexEmptyArea) ) );
}

void CDnMinimap::DeleteEmptyArea( int nAreaIndex )
{
	std::map<int, std::pair<EtVector3,int>>::iterator itor = m_mEmptyInfo.find(nAreaIndex);
	if( itor == m_mEmptyInfo.end() )
		return;

	m_mEmptyInfo.erase( itor );
}

void CDnMinimap::AllDeleteEmptyArea()
{
	m_mEmptyInfo.clear();
}

float CDnMinimap::GetDistanceDegree(EtVector3 vAxis,EtVector3 vTarget)
{
	EtVector3 vZDir, vCross, vTemp;
	vTemp = vTarget - vAxis;
	vTemp.y = 0.0f;
	EtVec3Normalize( &vTemp, &vTemp );
	vZDir.x = 0.0f;
	vZDir.z = 1.0f;
	vZDir.y = 0.0f;
	EtVec3Normalize( &vZDir, &vZDir );
	float fDegree = EtAcos( EtVec3Dot( &vTemp, &vZDir ) );
	EtVec3Cross( &vCross, &vTemp, &vZDir );
	if( vCross.y > 0.0f ) fDegree = -fDegree;

	return fDegree;
}

bool CDnMinimap::IsMinimapOut(EtVector3 vAxisPos,EtVector3 vTartGetPos,float fMaxSizeHalf,float fZoom)
{
	EtVector3 ScreenPos = ( vTartGetPos - vAxisPos ) / ( fMaxSizeHalf / fZoom );

	EtVector2 vLineBegin, vLineEnd;
	GetLinePoint( &vAxisPos, &vLineBegin, -HIDE_DEGREE, ( fMaxSizeHalf / fZoom ));
	GetLinePoint( &vAxisPos, &vLineEnd, HIDE_DEGREE, ( fMaxSizeHalf / fZoom ));

	bool bTargetOut(false);

	if( PointInLine( &EtVector2(vTartGetPos.x,vTartGetPos.z), &vLineBegin, &vLineEnd )
		!= PointInLine( &EtVector2(vAxisPos.x,vAxisPos.z), &vLineBegin, &vLineEnd ) )
	{
		bTargetOut = true;
	}
	else
	{
		float fTemp = sqrtf( powf(ScreenPos.x,2)+powf(ScreenPos.z,2) );
		if( 0.9f < fTemp ) 
		{
			bTargetOut = true;
		}
	}

	return bTargetOut;
}
