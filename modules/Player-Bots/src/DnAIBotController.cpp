#include "stdafx.h"
#include "DnAIBotController.h"
#include "DnPlayerActor.h"
#include "DnProjectile.h"
#include "EtActionSignal.h"
#include "DnMonsterActor.h"
#include "DnSkill.h"
#include "DnPlayAniProcess.h"
#include "DnPartialPlayProcessor.h"
#include "DnRangeChecker.h"
#include "IDnSkillUsableChecker.h"
#include "IDnSkillProcessor.h"
#include "DnBlow.h"
#include "DnBlowDef.h"
#include "DnStateBlow.h"
#include "DnWorld.h"
#include "DnWorldProp.h"
#include "DnWorldBrokenProp.h"
#include "DnWorldTrapProp.h"
#include "DNGameRoom.h"
#include "DnGameTask.h"
#include "DNUserSession.h"
#include "DnPartyTask.h"
#include "DnWorldGateStruct.h"
#include "EtWorldEventArea.h"
#include "MAMovementBase.h"
#include "navigationcell.h"
#include "navigationmesh.h"
#include "navigationpath.h"

#include <mutex>
#include <set>
#include <unordered_map>
#include <cmath>
#include <algorithm>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

namespace
{
	inline float XZDistSq( const EtVector3& a, const EtVector3& b )
	{
		float dx = a.x - b.x;
		float dz = a.z - b.z;
		return dx * dx + dz * dz;
	}

	inline float XZDist( const EtVector3& a, const EtVector3& b )
	{
		return sqrtf( XZDistSq( a, b ) );
	}

	inline float ClampF( float v, float lo, float hi )
	{
		if( v < lo ) return lo;
		if( v > hi ) return hi;
		return v;
	}

	inline EtVector2 SafeDir2( const EtVector3& vFrom, const EtVector3& vTo )
	{
		EtVector2 v( vTo.x - vFrom.x, vTo.z - vFrom.z );
		float fLenSq = v.x * v.x + v.y * v.y;
		if( fLenSq > 0.0001f )
		{
			float fInv = 1.0f / sqrtf( fLenSq );
			v.x *= fInv;
			v.y *= fInv;
		}
		else
		{
			v.x = 0.0f;
			v.y = 1.0f;
		}
		return v;
	}

	inline EtVector2 Normalize2( const EtVector2& in )
	{
		float fLenSq = in.x * in.x + in.y * in.y;
		if( fLenSq > 0.0001f )
		{
			float fInv = 1.0f / sqrtf( fLenSq );
			return EtVector2( in.x * fInv, in.y * fInv );
		}
		return EtVector2( 0.f, 1.f );
	}

	inline float ComputeAngleDiffDeg( const EtVector2& a, const EtVector2& b )
	{
		float fDot = ClampF( a.x * b.x + a.y * b.y, -1.0f, 1.0f );
		return EtToDegree( acosf( fDot ) );
	}

	inline EtVector2 RotateDir2Deg( const EtVector2& v, float fDeg )
	{
		float fRad = EtToRadian( fDeg );
		float c = cosf( fRad );
		float s = sinf( fRad );
		return EtVector2( v.x * c - v.y * s, v.x * s + v.y * c );
	}

	inline DWORD HashStringFNV1a( const char* sz )
	{
		if( !sz ) return 0;
		DWORD h = 2166136261u;
		while( *sz )
		{
			h ^= (BYTE)(*sz++);
			h *= 16777619u;
		}
		return h;
	}

	bool HasEnemyBlow( CDnActor* pActor, STATE_BLOW::emBLOW_INDEX eBlow )
	{
		if( !pActor ) return false;
		return pActor->IsAppliedThisStateBlow( eBlow );
	}

	bool ActorHasOwnSummonedMonster( CDnPlayerActor* pBot )
	{
		if( !pBot || !pBot->GetRoom() ) return false;
		DNVector(DnActorHandle) vecActors;
		CDnActor::ScanActor( pBot->GetRoom(), *pBot->GetPosition(), 2000.f, vecActors );
		for( size_t i = 0; i < vecActors.size(); ++i )
		{
			DnActorHandle hAct = vecActors[i];
			if( !hAct || hAct->IsDie() || !hAct->IsMonsterActor() ) continue;
			CDnMonsterActor* pMon = static_cast<CDnMonsterActor*>( hAct.GetPointer() );
			if( pMon->GetSummonerPlayerActor() == pBot->GetActorHandle() )
				return true;
		}
		return false;
	}

	// =========================================================================
	// Thread-Safe Shared Adaptive Learning Memory across all AI Bots & Runs
	// =========================================================================
	struct SkillLearnEntry
	{
		float fLearnedMaxRange;     // tightened whenever a cast whiffs against a live enemy
		float fVerifiedHitMaxRange; // highest distance at which a hit was confirmed
		float fHitRateEMA;          // 0..1 smoothed hit reliability
		float fAvgHitsPerCastEMA;   // multi-hit effectiveness
		int nTotalCasts;
		int nConsecutiveWhiffs;

		SkillLearnEntry()
			: fLearnedMaxRange( 0.f )
			, fVerifiedHitMaxRange( 0.f )
			, fHitRateEMA( 0.85f )
			, fAvgHitsPerCastEMA( 1.0f )
			, nTotalCasts( 0 )
			, nConsecutiveWhiffs( 0 )
		{}
	};

	struct BossActionLearnEntry
	{
		float fObservedMaxHitDist;
		float fObservedMinHitDist;
		float fMaxDamageRatio;
		int nHitCount;
		int nFrontalHits;
		bool bObservedJumpable;

		BossActionLearnEntry()
			: fObservedMaxHitDist( 0.f )
			, fObservedMinHitDist( FLT_MAX )
			, fMaxDamageRatio( 0.f )
			, nHitCount( 0 )
			, nFrontalHits( 0 )
			, bObservedJumpable( false )
		{}
	};

	class CTacticalLearningMemory
	{
	public:
		static CTacticalLearningMemory& Get()
		{
			static CTacticalLearningMemory s_Instance;
			return s_Instance;
		}

		bool QuerySkillLearning( int nJobID, int nSkillID, SkillLearnEntry& outEntry ) const
		{
			std::lock_guard<std::mutex> lock( m_Mutex );
			unsigned __int64 key = MakeSkillKey( nJobID, nSkillID );
			std::unordered_map<unsigned __int64, SkillLearnEntry>::const_iterator it = m_SkillMap.find( key );
			if( it == m_SkillMap.end() )
				return false;
			outEntry = it->second;
			return true;
		}

		bool RecordSkillOutcome( int nJobID, int nSkillID, float fCastDist, float fStaticMaxRange, bool bHit, int nHitCount, float& outPrevCap, float& outNewCap )
		{
			std::lock_guard<std::mutex> lock( m_Mutex );
			unsigned __int64 key = MakeSkillKey( nJobID, nSkillID );
			SkillLearnEntry& entry = m_SkillMap[key];
			if( entry.fLearnedMaxRange <= 0.f )
				entry.fLearnedMaxRange = fStaticMaxRange;

			outPrevCap = entry.fLearnedMaxRange;
			bool bTightened = false;

			++entry.nTotalCasts;
			float fHitVal = bHit ? 1.0f : 0.0f;
			entry.fHitRateEMA = entry.fHitRateEMA * 0.78f + fHitVal * 0.22f;

			if( bHit )
			{
				entry.nConsecutiveWhiffs = 0;
				if( nHitCount > 0 )
					entry.fAvgHitsPerCastEMA = entry.fAvgHitsPerCastEMA * 0.75f + (float)nHitCount * 0.25f;

				if( fCastDist > entry.fVerifiedHitMaxRange )
					entry.fVerifiedHitMaxRange = fCastDist;

				// Slowly relax cap slightly toward verified hit distance if previously over-tightened
				if( fCastDist > entry.fLearnedMaxRange * 0.92f && entry.fLearnedMaxRange < fStaticMaxRange )
				{
					entry.fLearnedMaxRange = ClampF( entry.fLearnedMaxRange + 18.f, 80.f, fStaticMaxRange );
				}
			}
			else
			{
				++entry.nConsecutiveWhiffs;
				// If a skill missed at distance D, tighten its learned effective range so bots walk closer!
				float fShrinkFactor = ( entry.nConsecutiveWhiffs >= 2 ) ? 0.74f : 0.82f;
				float fCandidate = ClampF( fCastDist * fShrinkFactor, 80.f, fStaticMaxRange );
				if( fCandidate < entry.fLearnedMaxRange - 8.f )
				{
					entry.fLearnedMaxRange = fCandidate;
					bTightened = true;
				}
			}

			outNewCap = entry.fLearnedMaxRange;
			return bTightened;
		}

		bool QueryBossAction( int nMonsterClassID, const char* szActionName, BossActionLearnEntry& outEntry ) const
		{
			if( nMonsterClassID <= 0 || !szActionName || !szActionName[0] )
				return false;
			std::lock_guard<std::mutex> lock( m_Mutex );
			unsigned __int64 key = MakeBossKey( nMonsterClassID, szActionName );
			std::unordered_map<unsigned __int64, BossActionLearnEntry>::const_iterator it = m_BossMap.find( key );
			if( it == m_BossMap.end() )
				return false;
			outEntry = it->second;
			return true;
		}

		bool RecordBossActionHit( int nMonsterClassID, const char* szActionName, float fHitDist, float fDmgRatio, bool bWasFrontal, bool bWasJumpable )
		{
			if( nMonsterClassID <= 0 || !szActionName || !szActionName[0] )
				return false;
			std::lock_guard<std::mutex> lock( m_Mutex );
			unsigned __int64 key = MakeBossKey( nMonsterClassID, szActionName );
			BossActionLearnEntry& entry = m_BossMap[key];
			bool bFirstTime = ( entry.nHitCount == 0 );

			++entry.nHitCount;
			if( bWasFrontal )
				++entry.nFrontalHits;
			if( bWasJumpable )
				entry.bObservedJumpable = true;

			if( fHitDist > entry.fObservedMaxHitDist )
				entry.fObservedMaxHitDist = ClampF( fHitDist, 60.f, 1600.f );
			if( fHitDist < entry.fObservedMinHitDist )
				entry.fObservedMinHitDist = ClampF( fHitDist, 0.f, 1600.f );
			if( fDmgRatio > entry.fMaxDamageRatio )
				entry.fMaxDamageRatio = ClampF( fDmgRatio, 0.f, 1.0f );

			return bFirstTime;
		}

		bool TryClaimRoomCalloutSlot( unsigned int uiRoomID, DWORD dwNowTick, DWORD dwMinCooldownMs )
		{
			std::lock_guard<std::mutex> lock( m_Mutex );
			DWORD& dwLast = m_RoomLastCalloutTick[uiRoomID];
			if( dwLast != 0 && ( dwNowTick - dwLast ) < dwMinCooldownMs )
				return false;
			dwLast = dwNowTick;
			return true;
		}

	private:
		static unsigned __int64 MakeSkillKey( int nJobID, int nSkillID )
		{
			return ( (unsigned __int64)(DWORD)nJobID << 32 ) | (unsigned __int64)(DWORD)nSkillID;
		}

		static unsigned __int64 MakeBossKey( int nMonsterClassID, const char* szActionName )
		{
			return ( (unsigned __int64)(DWORD)nMonsterClassID << 32 ) | (unsigned __int64)HashStringFNV1a( szActionName );
		}

		mutable std::mutex m_Mutex;
		std::unordered_map<unsigned __int64, SkillLearnEntry> m_SkillMap;
		std::unordered_map<unsigned __int64, BossActionLearnEntry> m_BossMap;
		std::unordered_map<unsigned int, DWORD> m_RoomLastCalloutTick;
	};
}

CDnAIBotController::CDnAIBotController( CDnPlayerActor* pActor )
	: m_pActor( pActor )
	, m_Role( AIBot::RoleType::Dps )
	, m_nBotSlotIndex( 0 )
	, m_bRoleInitialized( false )
	, m_eEncounterMode( Mode_Dungeon )
	, m_vAimPos( 0.f, 0.f, 0.f )
	, m_nCurWaypoint( 0 )
	, m_vLastPathGoal( 0.f, 0.f, 0.f )
	, m_fRepathTimer( 0.f )
	, m_fMoveSyncTimer( 0.f )
	, m_vLastMoveDir( 0.f, 0.f, 1.f )
	, m_vLastLookDir( 0.f, 1.f )
	, m_cLastMoveFlag( 0x04 )
	, m_strLastMoveAction( "Move_Front" )
	, m_eMoveState( MoveState_None )
	, m_vStuckCheckPos( 0.f, 0.f, 0.f )
	, m_fStuckCheckTimer( 0.f )
	, m_fStuckAccumTime( 0.f )
	, m_fScanTimer( 0.f )
	, m_fDecisionTimer( 0.f )
	, m_fSkillDelayTimer( 0.25f )
	, m_fDodgeCooldown( 0.f )
	, m_fJumpCooldown( 0.f )
	, m_fRecoveryCooldown( 0.f )
	, m_fDashGapCooldown( 0.f )
	, m_fComboChainTimer( 0.f )
	, m_fBuffCheckTimer( 0.5f )
	, m_nBasicComboStep( 0 )
	, m_bLoggedStageSummary( false )
	, m_bLoggedFirstCombat( false )
	, m_bLoggedFirstAction( false )
	, m_fStrafeSign( 1.f )
	, m_fStrafeSwitchTimer( 3.2f )
	, m_fReactionJitter( 0.06f )
	, m_fDefensiveBias( 0.f )
	, m_fPostCombatPauseTimer( 0.f )
	, m_bWasInCombatLastTick( false )
	, m_nPendingCastSkillID( 0 )
	, m_fPendingCastDist( 0.f )
	, m_fPendingCastTimer( 0.f )
	, m_bPendingCastHit( false )
	, m_nPendingCastHitCount( 0 )
	, m_bHasValidPath( true )
	, m_bBlockedByObstacle( false )
	, m_fObstacleProbeTimer( 0.f )
	, m_fAvoidHoldTimer( 0.f )
	, m_fAvoidSideSign( 1.f )
	, m_vAvoidDir( 0.f, 1.f )
	, m_fGateWaitTimer( 0.f )
	, m_nSyncedGateIndex( -1 )
{
}

CDnAIBotController::~CDnAIBotController()
{
	m_vecWaypoints.clear();
	m_mapSkillProfiles.clear();
}

void CDnAIBotController::OnStageStart()
{
	FinalizePendingSkillOutcome( true );

	m_hTargetActor.Identity();
	m_hPendingCastTarget.Identity();
	m_vecWaypoints.clear();
	m_nCurWaypoint = 0;
	m_vLastPathGoal = EtVector3( 0.f, 0.f, 0.f );
	m_fRepathTimer = 0.f;
	m_fMoveSyncTimer = 0.f;
	m_cLastMoveFlag = 0x04;
	m_strLastMoveAction = "Move_Front";
	m_eMoveState = MoveState_None;
	m_fStuckCheckTimer = 0.f;
	m_fStuckAccumTime = 0.f;
	m_fScanTimer = 0.f;
	m_fDecisionTimer = 0.f;
	m_fSkillDelayTimer = 0.35f + (float)( m_nBotSlotIndex % 4 ) * 0.09f;
	m_fDodgeCooldown = 0.f;
	m_fJumpCooldown = 0.f;
	m_fRecoveryCooldown = 0.f;
	m_fDashGapCooldown = 0.f;
	m_fComboChainTimer = 0.f;
	m_fBuffCheckTimer = 0.6f + (float)( m_nBotSlotIndex % 3 ) * 0.3f;
	m_fPostCombatPauseTimer = 0.f;
	m_bWasInCombatLastTick = false;
	m_bLoggedStageSummary = false;
	m_bLoggedFirstCombat = false;
	m_bLoggedFirstAction = false;
	m_bHasValidPath = true;
	m_bBlockedByObstacle = false;
	m_fObstacleProbeTimer = 0.f;
	m_fAvoidHoldTimer = 0.f;
	m_fAvoidSideSign = ( ( m_nBotSlotIndex % 2 ) == 0 ) ? 1.f : -1.f;
	m_fStrafeSign = ( ( m_nBotSlotIndex % 2 ) == 0 ) ? 1.f : -1.f;
	m_fStrafeSwitchTimer = 2.6f + (float)( m_nBotSlotIndex % 5 ) * 0.55f;
	m_fGateWaitTimer = 0.f;
	m_nSyncedGateIndex = -1;
	m_hPathBlockingBreakable.Identity();
	m_LastScan = ArenaScanResult();
	m_eEncounterMode = DetectEncounterMode();
	if( m_pActor )
	{
		m_vStuckCheckPos = *m_pActor->GetPosition();
		// Decay defensive bias slightly on clean stage advance
		m_fDefensiveBias *= 0.65f;
	}
}

void CDnAIBotController::EnsureRoleInitialized()
{
	if( m_bRoleInitialized || !m_pActor )
		return;

	m_bRoleInitialized = true;
	m_Role = AIBot::GetJobRole( (BYTE)m_pActor->GetJobClassID() );
	m_nBotSlotIndex = 0;

	CDNGameRoom* pRoom = m_pActor->GetGameRoom();
	CDNUserSession* pMySession = m_pActor->GetUserSession();
	if( pRoom && pMySession )
	{
		int nSlot = 0;
		for( UINT i = 0; i < pRoom->GetUserCount(); ++i )
		{
			CDNUserSession* pSess = pRoom->GetUserData( i );
			if( !pSess || !pSess->bIsAIBot() ) continue;
			if( pSess == pMySession )
			{
				m_nBotSlotIndex = nSlot;
				break;
			}
			++nSlot;
		}
	}

	m_fAvoidSideSign = ( ( m_nBotSlotIndex % 2 ) == 0 ) ? 1.f : -1.f;
	m_fStrafeSign = ( ( m_nBotSlotIndex % 2 ) == 0 ) ? 1.f : -1.f;
	m_fReactionJitter = 0.04f + (float)( m_nBotSlotIndex % 5 ) * 0.024f;
	m_fStrafeSwitchTimer = 2.5f + (float)( m_nBotSlotIndex % 4 ) * 0.65f;
	m_eEncounterMode = DetectEncounterMode();
}

CDnAIBotController::EncounterMode CDnAIBotController::DetectEncounterMode() const
{
	if( !m_pActor || !m_pActor->GetRoom() )
		return Mode_Dungeon;

	CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
	if( !pWorld )
		return Mode_Dungeon;

	if( pWorld->GetDragonNestType() != eDragonNestType::None )
		return Mode_DragonNest;

	EWorldEnum::MapSubTypeEnum eSub = pWorld->GetMapSubType();
	if( eSub == EWorldEnum::MapSubTypeNest || eSub == EWorldEnum::MapSubTypeNestNormal )
		return Mode_Nest;
	if( eSub == EWorldEnum::MapSubTypeDarkLair || eSub == EWorldEnum::MapSubTypeDarkLairWorldZone )
		return Mode_DarkLair;

	return Mode_Dungeon;
}

void CDnAIBotController::OnDamaged( DnActorHandle hHitter, INT64 nDamage )
{
	if( !m_pActor || nDamage <= 0 )
		return;

	INT64 nMaxHP = m_pActor->GetMaxHP();
	float fDmgRatio = ( nMaxHP > 0 ) ? ( (float)nDamage / (float)nMaxHP ) : 0.f;

	// Adaptively raise defensive awareness when taking real damage (especially in Nests)
	if( fDmgRatio >= 0.10f )
		m_fDefensiveBias = ClampF( m_fDefensiveBias + fDmgRatio * 1.4f, 0.f, 1.0f );

	// Cancel pending whiff penalty if our cast got interrupted by taking a hit
	if( m_nPendingCastSkillID > 0 && m_pActor->IsHit() )
		m_nPendingCastSkillID = 0;

	if( hHitter && hHitter->IsMonsterActor() )
		ObserveBossHitOnActor( hHitter.GetPointer(), m_pActor, nDamage );
}

void CDnAIBotController::ObserveBossHitOnActor( CDnActor* pHitter, CDnActor* pVictim, INT64 nDamage )
{
	if( !pHitter || !pVictim || !pHitter->IsMonsterActor() || nDamage <= 0 )
		return;

	CDnMonsterActor* pMon = static_cast<CDnMonsterActor*>( pHitter );
	// Learn from Named / Elite / Boss / NestBoss monsters
	if( pMon->GetGrade() < CDnMonsterState::Elite )
		return;

	const char* szAction = pMon->GetCurrentAction();
	if( !szAction || !szAction[0] )
		return;
	if( strstr( szAction, "Stand" ) || strstr( szAction, "Move" ) || strstr( szAction, "Hit" ) || strstr( szAction, "Down" ) )
		return;

	INT64 nMaxHP = pVictim->GetMaxHP();
	float fDmgRatio = ( nMaxHP > 0 ) ? ( (float)nDamage / (float)nMaxHP ) : 0.1f;

	float fVictimRadius = GetActorEffectiveRadius( pVictim );
	float fHitDist = XZDist( *pMon->GetPosition(), *pVictim->GetPosition() ) - fVictimRadius;
	if( fHitDist < 0.f ) fHitDist = 0.f;

	EtVector2 vBossFwd = Normalize2( EtVector2( pMon->GetMatEx()->m_vZAxis.x, pMon->GetMatEx()->m_vZAxis.z ) );
	EtVector2 vToVictim = SafeDir2( *pMon->GetPosition(), *pVictim->GetPosition() );
	bool bWasFrontal = ( ComputeAngleDiffDeg( vBossFwd, vToVictim ) <= 75.f );

	bool bWasJumpable = false;
	CDnActionBase::ActionElementStruct* pElem = pMon->GetElement( pMon->GetCurrentActionIndex() );
	if( pElem )
	{
		std::map<int, std::vector<CEtActionSignal*>>::iterator itHit = pElem->MapSearch.find( STE_Hit );
		if( itHit != pElem->MapSearch.end() )
		{
			for( size_t i = 0; i < itHit->second.size(); ++i )
			{
				HitStruct* pHit = static_cast<HitStruct*>( itHit->second[i]->GetData() );
				if( pHit && pHit->fHeightMax > 0.f && pHit->fHeightMax <= 95.f && pHit->fHeightMin <= 40.f && pHit->fDistanceMax >= 200.f )
				{
					bWasJumpable = true;
					break;
				}
			}
		}
	}

	bool bFirstLearned = CTacticalLearningMemory::Get().RecordBossActionHit(
		pMon->GetMonsterClassID(), szAction, fHitDist, fDmgRatio, bWasFrontal, bWasJumpable );

	if( bFirstLearned && fDmgRatio >= 0.12f )
	{
		g_Log.Log( LogType::_NORMAL, L"[AIBot] LearnedBossThreat Monster=%d Action=%S Dist=%.1f DmgRatio=%.2f Frontal=%d Jumpable=%d\n",
			pMon->GetMonsterClassID(), szAction, fHitDist, fDmgRatio, bWasFrontal ? 1 : 0, bWasJumpable ? 1 : 0 );
	}
}

void CDnAIBotController::OnAttackHitTarget( DnActorHandle hTarget, bool bFromProjectile )
{
	if( !hTarget || !m_pActor )
		return;

	if( m_nPendingCastSkillID > 0 )
	{
		m_bPendingCastHit = true;
		++m_nPendingCastHitCount;
	}
}

void CDnAIBotController::FinalizePendingSkillOutcome( bool bForce )
{
	if( m_nPendingCastSkillID <= 0 || !m_pActor )
		return;

	if( !bForce && m_fPendingCastTimer > 0.f && ( m_pActor->IsProcessSkill() || m_fPendingCastTimer > 0.25f ) )
		return;

	// Only penalize a whiff if the target was actually alive and NOT invulnerable
	bool bValidWhiff = false;
	if( !m_bPendingCastHit && !bForce )
	{
		if( m_hPendingCastTarget && !m_hPendingCastTarget->IsDie() &&
			!HasEnemyBlow( m_hPendingCastTarget.GetPointer(), STATE_BLOW::BLOW_099 ) )
		{
			bValidWhiff = true;
		}
	}

	if( m_bPendingCastHit || bValidWhiff )
	{
		int nJobID = m_pActor->GetJobClassID();
		float fStaticMax = 260.f;
		std::map<int, SkillProfile>::const_iterator itProf = m_mapSkillProfiles.find( m_nPendingCastSkillID );
		if( itProf != m_mapSkillProfiles.end() )
			fStaticMax = itProf->second.fMaxRange;

		float fPrevCap = 0.f, fNewCap = 0.f;
		bool bTightened = CTacticalLearningMemory::Get().RecordSkillOutcome(
			nJobID, m_nPendingCastSkillID, m_fPendingCastDist, fStaticMax,
			m_bPendingCastHit, m_nPendingCastHitCount, fPrevCap, fNewCap );

		if( bTightened && m_pActor->GetUserSession() )
		{
			CDNUserSession* pSess = m_pActor->GetUserSession();
			g_Log.Log( LogType::_NORMAL, pSess->GetWorldSetID(), pSess->GetAccountDBID(), pSess->GetCharacterDBID(), pSess->GetSessionID(),
				L"[AIBot] LearnedRange Bot=%s SkillID=%d WhiffAt=%.1f Cap=%.1f->%.1f\n",
				m_pActor->GetName(), m_nPendingCastSkillID, m_fPendingCastDist, fPrevCap, fNewCap );
		}
	}

	m_nPendingCastSkillID = 0;
	m_fPendingCastDist = 0.f;
	m_fPendingCastTimer = 0.f;
	m_bPendingCastHit = false;
	m_nPendingCastHitCount = 0;
	m_hPendingCastTarget.Identity();
}

void CDnAIBotController::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_pActor || !m_pActor->GetUserSession() || !m_pActor->GetGameRoom() )
		return;
	if( m_pActor->IsDie() )
		return;

	CDNGameRoom* pRoom = m_pActor->GetGameRoom();
	if( pRoom->GetRoomState() != _GAME_STATE_PLAY )
		return;

	EnsureRoleInitialized();

	if( !m_bLoggedStageSummary )
	{
		m_bLoggedStageSummary = true;
		int nOffSkills = 0;
		for( DWORD i = 0; i < m_pActor->GetSkillCount(); ++i )
		{
			DnSkillHandle hSk = m_pActor->GetSkillFromIndex( i );
			if( hSk && IsOffensiveActiveSkill( hSk.GetPointer() ) )
			{
				GetOrAnalyzeSkillProfile( hSk.GetPointer() );
				++nOffSkills;
			}
		}
		const char* szBasic = NULL;
		bool bRangedBasic = false;
		ResolveBasicAttackAction( 0, &szBasic, &bRangedBasic );
		CDNUserSession* pSess = m_pActor->GetUserSession();
		g_Log.Log( LogType::_NORMAL, pSess->GetWorldSetID(), pSess->GetAccountDBID(), pSess->GetCharacterDBID(), pSess->GetSessionID(),
			L"[AIBot] StageInit Bot=%s Role=%d Mode=%d Class=%d Job=%d Skills=%u OffSkills=%d BasicAct=%S Ranged=%d\n",
			m_pActor->GetName(), (int)m_Role, (int)m_eEncounterMode, m_pActor->GetClassID(), m_pActor->GetJobClassID(),
			m_pActor->GetSkillCount(), nOffSkills, szBasic ? szBasic : "(none)", bRangedBasic ? 1 : 0 );
	}

	if( fDelta <= 0.f || fDelta > 0.5f )
		fDelta = 0.016f;

	m_fRepathTimer = ( m_fRepathTimer > fDelta ) ? ( m_fRepathTimer - fDelta ) : 0.f;
	m_fMoveSyncTimer = ( m_fMoveSyncTimer > fDelta ) ? ( m_fMoveSyncTimer - fDelta ) : 0.f;
	m_fScanTimer = ( m_fScanTimer > fDelta ) ? ( m_fScanTimer - fDelta ) : 0.f;
	m_fDecisionTimer = ( m_fDecisionTimer > fDelta ) ? ( m_fDecisionTimer - fDelta ) : 0.f;
	m_fSkillDelayTimer = ( m_fSkillDelayTimer > fDelta ) ? ( m_fSkillDelayTimer - fDelta ) : 0.f;
	m_fDodgeCooldown = ( m_fDodgeCooldown > fDelta ) ? ( m_fDodgeCooldown - fDelta ) : 0.f;
	m_fJumpCooldown = ( m_fJumpCooldown > fDelta ) ? ( m_fJumpCooldown - fDelta ) : 0.f;
	m_fRecoveryCooldown = ( m_fRecoveryCooldown > fDelta ) ? ( m_fRecoveryCooldown - fDelta ) : 0.f;
	m_fDashGapCooldown = ( m_fDashGapCooldown > fDelta ) ? ( m_fDashGapCooldown - fDelta ) : 0.f;
	m_fComboChainTimer = ( m_fComboChainTimer > fDelta ) ? ( m_fComboChainTimer - fDelta ) : 0.f;
	m_fBuffCheckTimer = ( m_fBuffCheckTimer > fDelta ) ? ( m_fBuffCheckTimer - fDelta ) : 0.f;
	m_fObstacleProbeTimer = ( m_fObstacleProbeTimer > fDelta ) ? ( m_fObstacleProbeTimer - fDelta ) : 0.f;
	m_fAvoidHoldTimer = ( m_fAvoidHoldTimer > fDelta ) ? ( m_fAvoidHoldTimer - fDelta ) : 0.f;
	m_fPostCombatPauseTimer = ( m_fPostCombatPauseTimer > fDelta ) ? ( m_fPostCombatPauseTimer - fDelta ) : 0.f;

	// Smoothly decay adaptive defensive bias over long safe stretches
	if( m_fDefensiveBias > 0.f )
		m_fDefensiveBias = ( m_fDefensiveBias > fDelta * 0.012f ) ? ( m_fDefensiveBias - fDelta * 0.012f ) : 0.f;

	// Organic human strafe direction switches every 2.8 - 5.5s
	m_fStrafeSwitchTimer -= fDelta;
	if( m_fStrafeSwitchTimer <= 0.f )
	{
		m_fStrafeSign = -m_fStrafeSign;
		m_fStrafeSwitchTimer = 2.8f + (float)( ( (int)( LocalTime / 97 ) + m_nBotSlotIndex * 13 ) % 27 ) * 0.1f;
	}

	// Track pending skill hit/whiff outcome
	if( m_nPendingCastSkillID > 0 )
	{
		m_fPendingCastTimer = ( m_fPendingCastTimer > fDelta ) ? ( m_fPendingCastTimer - fDelta ) : 0.f;
		FinalizePendingSkillOutcome( false );
	}

	// 1. Human-like Air / Knockdown recovery (Aerial Evasion & Wake-Up) runs immediately even while hit!
	if( m_pActor->IsAir() || m_pActor->IsDown() )
	{
		if( m_fRecoveryCooldown <= 0.f && TryHandleHitRecovery() )
		{
			m_fRecoveryCooldown = 0.65f;
			return;
		}
		return;
	}

	// 2. If currently stunned or flinching (grounded hitstun), wait out hitstun naturally
	if( m_pActor->IsStun() || m_pActor->IsHit() )
		return;

	// 3. Periodic battlefield, boss telegraph, ally & portal perception (faster during Nests/active boss fights!)
	if( m_fScanTimer <= 0.f )
	{
		ScanArenaAndParty( LocalTime );
		bool bHighTempo = ( m_eEncounterMode == Mode_Nest || m_eEncounterMode == Mode_DragonNest || (bool)m_LastScan.hPriorityBoss );
		m_fScanTimer = ( bHighTempo ? 0.12f : 0.18f ) + ( m_fReactionJitter * 0.5f );
	}

	// Sync portal readiness whenever human enters/leaves an active stage portal
	UpdateGateReadiness( fDelta );

	// 4. Maintain Battle / Peace stance cleanly (sheathe weapons out of combat when human is peaceful)
	bool bNeedCombatStance = ( !m_LastScan.bHumanInGate && ( m_LastScan.bHasAnyEnemy || (bool)m_LastScan.hBreakableProp ) )
		|| (bool)m_LastScan.hBlockingBreakableProp
		|| (bool)m_hPathBlockingBreakable;

	if( m_bWasInCombatLastTick && !bNeedCombatStance && !m_LastScan.bHumanInGate )
	{
		// Brief natural human pause after clearing a pack before sprinting on
		m_fPostCombatPauseTimer = 0.22f + m_fReactionJitter * 2.0f;
	}
	m_bWasInCombatLastTick = bNeedCombatStance;

	if( bNeedCombatStance && !m_pActor->IsBattleMode() )
	{
		ToggleBattleState( true );
	}
	else if( !bNeedCombatStance && m_pActor->IsBattleMode() && m_pActor->IsStay() &&
		( m_LastScan.hHumanLeader && !static_cast<CDnPlayerActor*>( m_LastScan.hHumanLeader.GetPointer() )->IsBattleMode() && XZDist( *m_pActor->GetPosition(), *m_LastScan.hHumanLeader->GetPosition() ) < 450.f ) )
	{
		ToggleBattleState( false );
	}

	// 5. While mid-skill animation: check human-like follow-up skill chains (LMB/RMB chain) & allow emergency boss cancel/dodge!
	if( m_pActor->IsProcessSkill() )
	{
		if( m_fComboChainTimer <= 0.f && TrySkillChainFollowUp() )
		{
			m_fComboChainTimer = 0.22f;
			return;
		}
		// Keep aim updated if target moves slightly
		if( m_hTargetActor && !m_hTargetActor->IsDie() )
			m_vAimPos = ComputeTargetAimPoint( m_hTargetActor.GetPointer(), true );
		return;
	}

	if( m_pActor->IsAttack() )
		return;

	// 6. Emergency Boss Telegraph & Trap Reaction (Stomp Jump, I-Frame Skill, Directional Tumble Dodge)
	if( !m_LastScan.bHumanInGate )
	{
		if( m_LastScan.BossThreat.bBotCurrentlyInDanger && TryReactToBossThreat() )
			return;

		if( m_LastScan.bHasTrapDanger && m_fDodgeCooldown <= 0.f )
		{
			if( TryDirectionalTumble( m_LastScan.vDangerAvoidVec ) )
			{
				m_fDodgeCooldown = 2.4f;
				m_fSkillDelayTimer = 0.30f;
				return;
			}
		}
	}

	// 7. Portal catch-up & anti-abandonment leash (never softlock a human waiting in a portal)
	if( m_LastScan.bHumanInGate && m_LastScan.nActiveGateIndex != -1 && m_nSyncedGateIndex != m_LastScan.nActiveGateIndex )
	{
		float fGateDist = XZDist( *m_pActor->GetPosition(), m_LastScan.vGateTargetPos );
		if( fGateDist > 1800.f || m_fGateWaitTimer > 4.5f || ( m_fGateWaitTimer > 2.2f && m_fStuckAccumTime > 1.2f ) )
		{
			PerformPortalTeleport( m_LastScan.vGateTargetPos );
			UpdateGateReadiness( 0.f );
			return;
		}
	}

	if( !m_LastScan.bHumanInGate && m_LastScan.hHumanLeader && !m_LastScan.hHumanLeader->IsDie() )
	{
		float fHumanDist = XZDist( *m_pActor->GetPosition(), *m_LastScan.hHumanLeader->GetPosition() );
		float fLeashDist = ( m_LastScan.bHasAnyEnemy || m_LastScan.bIsNestEncounter ) ? 2800.f : 1500.f;
		if( fHumanDist > fLeashDist || ( !m_LastScan.bHasAnyEnemy && fHumanDist > 750.f && m_fStuckAccumTime > 3.0f ) )
		{
			PerformLeashTeleport( *m_LastScan.hHumanLeader->GetPosition() );
			return;
		}
	}

	// 8. Combat & Support Decision Making (heals, cures, buffs, exact-range skills, basic combos, breakables)
	if( m_fSkillDelayTimer <= 0.f && m_fDecisionTimer <= 0.f )
	{
		m_fDecisionTimer = 0.08f + m_fReactionJitter;
		UpdateCombatDecision( LocalTime, fDelta );
		if( m_pActor->IsProcessSkill() || m_pActor->IsAttack() )
			return;
	}

	// 9. Brief post-combat natural breath before following
	if( m_fPostCombatPauseTimer > 0.f && !m_LastScan.bHumanInGate && !bNeedCombatStance )
	{
		if( m_eMoveState == MoveState_Moving )
			SendStopPacket();
		return;
	}

	// 10. Human-Like Movement, Directional Combat Strafing & Smart NavMesh/Obstacle Steering
	bool bWantsToMove = false;
	UpdateMovementAndPositioning( fDelta, bWantsToMove );

	// 11. Real Stuck Detection & Obstacle-Bypass Recovery
	m_fStuckCheckTimer += fDelta;
	if( m_fStuckCheckTimer >= 0.35f )
	{
		EtVector3 vCurPos = *m_pActor->GetPosition();
		float fMoved = XZDist( vCurPos, m_vStuckCheckPos );
		if( bWantsToMove && m_eMoveState == MoveState_Moving && fMoved < 20.f )
		{
			m_fStuckAccumTime += m_fStuckCheckTimer;
			m_bBlockedByObstacle = true;
			m_fObstacleProbeTimer = 0.f;
			m_fRepathTimer = 0.f;
			m_vecWaypoints.clear();
			if( m_fStuckAccumTime > 1.2f )
			{
				m_fAvoidSideSign = -m_fAvoidSideSign;
				m_fAvoidHoldTimer = 0.f;
			}
		}
		else if( fMoved >= 28.f )
		{
			m_fStuckAccumTime = 0.f;
			m_bBlockedByObstacle = false;
		}
		m_vStuckCheckPos = vCurPos;
		m_fStuckCheckTimer = 0.f;
	}
}

void CDnAIBotController::ScanArenaAndParty( LOCAL_TIME LocalTime )
{
	ArenaScanResult res;
	EtVector3 vMyPos = *m_pActor->GetPosition();
	int nMyTeam = m_pActor->GetTeam();
	CDNGameRoom* pRoom = m_pActor->GetGameRoom();
	CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
	m_eEncounterMode = DetectEncounterMode();
	res.bIsNestEncounter = ( m_eEncounterMode == Mode_Nest || m_eEncounterMode == Mode_DragonNest );

	// 1. Scan Party Members (Human Leader, Heals, Cures, Raid Burst Buff Detection)
	bool bAnyPartyBurstBuff = false;
	if( pRoom )
	{
		for( UINT i = 0; i < pRoom->GetUserCount(); ++i )
		{
			CDNUserSession* pSess = pRoom->GetUserData( i );
			if( !pSess || !pSess->GetActorHandle() ) continue;
			CDnPlayerActor* pMate = static_cast<CDnPlayerActor*>( pSess->GetActorHandle().GetPointer() );
			if( !pMate || pMate->IsDie() ) continue;

			if( !pSess->bIsAIBot() && !res.hHumanLeader )
				res.hHumanLeader = pMate->GetActorHandle();

			INT64 nMaxHP = pMate->GetMaxHP();
			float fHpRatio = ( nMaxHP > 0 ) ? ( (float)pMate->GetHP() / (float)nMaxHP ) : 1.0f;

			// Prioritize Human Player's safety slightly higher than bots
			float fEffectiveRatio = pSess->bIsAIBot() ? fHpRatio : ( fHpRatio - 0.14f );
			if( !pSess->bIsAIBot() && fHpRatio < 0.62f )
				res.bHumanInDanger = true;

			CDnPlayerActor* pCurLowest = res.hLowestHpAlly ? static_cast<CDnPlayerActor*>( res.hLowestHpAlly.GetPointer() ) : NULL;
			float fBestEffective = ( pCurLowest && pCurLowest->GetUserSession() && !pCurLowest->GetUserSession()->bIsAIBot() )
				? ( res.fLowestHpRatio - 0.14f )
				: res.fLowestHpRatio;

			if( !res.hLowestHpAlly || fEffectiveRatio < fBestEffective )
			{
				res.hLowestHpAlly = pMate->GetActorHandle();
				res.fLowestHpRatio = fHpRatio;
			}

			if( !res.hDebuffedAlly && IsHarmfulDebuffPresent( pMate ) )
				res.hDebuffedAlly = pMate->GetActorHandle();

			// Detect active Spirit Boost / Beyond Time / Action Speed / Cooldown reduction raid burst buffs
			if( pMate->IsAppliedThisStateBlow( STATE_BLOW::BLOW_025 ) ||
				pMate->IsAppliedThisStateBlow( STATE_BLOW::BLOW_129 ) ||
				pMate->IsAppliedThisStateBlow( STATE_BLOW::BLOW_076 ) )
			{
				bAnyPartyBurstBuff = true;
			}
		}
	}

	// 2. Scan Enemies (Exact Reachability, Boss Mechanics, Live Telegraph Reading, Cluster Density)
	DNVector(DnActorHandle) vecActors;
	CDnActor::ScanActor( m_pActor->GetRoom(), vMyPos, 2800.f, vecActors );

	int nBestScore = -999999;
	bool bBestHasLOS = true;
	DnPropHandle hBestBlockingBreakable;
	int nNavCheckBudget = 6;
	CDnPlayerActor* pLeaderPtr = res.hHumanLeader ? static_cast<CDnPlayerActor*>( res.hHumanLeader.GetPointer() ) : NULL;
	std::vector<EtVector3> vecEnemyPositions;
	vecEnemyPositions.reserve( 16 );

	for( size_t i = 0; i < vecActors.size(); ++i )
	{
		DnActorHandle hAct = vecActors[i];
		if( !hAct || !IsHittableEnemy( hAct.GetPointer(), m_pActor->GetActorHandle(), nMyTeam, LocalTime ) )
			continue;

		CDnMonsterActor* pMon = static_cast<CDnMonsterActor*>( hAct.GetPointer() );
		res.bHasAnyEnemy = true;
		++res.nAliveEnemyCount;

		EtVector3 vMonPos = *pMon->GetPosition();
		vecEnemyPositions.push_back( vMonPos );
		float fDistSq = XZDistSq( vMyPos, vMonPos );

		// Evaluate direct Line of Sight & whether a breakable barricade sits between bot and enemy
		PathObstacle losObs;
		bool bBlockedByObj = CheckSegmentObstacle( vMyPos, vMonPos, 10.f, true, LocalTime, losObs );
		bool bBlockedByBreakable = ( bBlockedByObj && losObs.eType == Obstacle_BreakableProp && (bool)losObs.hProp );
		bool bHasLOS = !bBlockedByObj;

		// Penalize enemies in locked/separate navmesh rooms so bots clear accessible targets first
		bool bNavReachable = true;
		if( fDistSq > ( 350.f * 350.f ) && ( bBlockedByObj || fDistSq > ( 950.f * 950.f ) ) )
			bNavReachable = HasNavMeshConnectivity( vMyPos, vMonPos, nNavCheckBudget );

		int nScore = ScoreEnemyPriority( pMon, m_Role, fDistSq, pLeaderPtr, m_eEncounterMode );
		if( !bNavReachable )
			nScore -= 1400;
		else if( bBlockedByObj && !bBlockedByBreakable )
			nScore -= 240;

		// Sticky target bonus prevents erratic target ping-ponging like a robot
		if( hAct == m_hTargetActor )
			nScore += 160;

		if( nScore > nBestScore )
		{
			nBestScore = nScore;
			res.hBestEnemy = hAct;
			bBestHasLOS = bHasLOS;
			hBestBlockingBreakable = bBlockedByBreakable ? losObs.hProp : CDnWorldProp::Identity();
		}

		// Track Boss / Nest Boss state & live telegraphs
		if( pMon->GetGrade() >= CDnMonsterState::Boss )
		{
			res.bIsNestEncounter = true;
			if( !res.hPriorityBoss || pMon->GetGrade() > static_cast<CDnMonsterActor*>( res.hPriorityBoss.GetPointer() )->GetGrade() )
			{
				res.hPriorityBoss = hAct;
				INT64 nBossMaxHP = pMon->GetMaxHP();
				res.fBossHpRatio = ( nBossMaxHP > 0 ) ? ( (float)pMon->GetHP() / (float)nBossMaxHP ) : 1.0f;
			}
			if( HasEnemyBlow( pMon, STATE_BLOW::BLOW_099 ) || pMon->IsEnableNoDamage() )
				res.bBossInvulnerable = true;
		}

		// Analyze real-time attack telegraphs for ANY Boss or Elite nearby!
		if( pMon->GetGrade() >= CDnMonsterState::Elite && fDistSq < ( 1500.f * 1500.f ) )
		{
			BossThreatInfo candThreat;
			AnalyzeBossThreat( pMon, vMyPos, candThreat );
			if( candThreat.eKind != Threat_None && candThreat.bBotCurrentlyInDanger )
			{
				// Prioritize the threat hitting soonest (or RoomWipe/Stomp)
				if( !res.BossThreat.bBotCurrentlyInDanger ||
					candThreat.eKind == Threat_RoomWipe ||
					candThreat.fTimeToImpact < res.BossThreat.fTimeToImpact )
				{
					res.BossThreat = candThreat;
				}
			}
		}
	}

	res.bTargetHasLOS = bBestHasLOS;
	res.hBlockingBreakableProp = hBestBlockingBreakable;

	// Count clustered enemies around primary target (drives AoE vs single-target skill selection)
	if( res.hBestEnemy )
	{
		EtVector3 vBestPos = *res.hBestEnemy->GetPosition();
		for( size_t i = 0; i < vecEnemyPositions.size(); ++i )
		{
			if( XZDistSq( vBestPos, vecEnemyPositions[i] ) <= ( 320.f * 320.f ) )
				++res.nEnemiesNearTarget;
		}
	}

	// Determine whether a coordinated Raid Burst Window is active!
	if( res.hPriorityBoss && !res.hPriorityBoss->IsDie() && !res.bBossInvulnerable )
	{
		bool bBossIced = HasEnemyBlow( res.hPriorityBoss.GetPointer(), STATE_BLOW::BLOW_041 ) ||
			HasEnemyBlow( res.hPriorityBoss.GetPointer(), STATE_BLOW::BLOW_144 );
		bool bBossExecute = ( res.fBossHpRatio <= 0.35f );
		if( bBossIced || bAnyPartyBurstBuff || bBossExecute )
			res.bBurstWindowActive = true;
	}
	else if( !res.hPriorityBoss && res.nEnemiesNearTarget >= 4 && m_eEncounterMode == Mode_Dungeon )
	{
		res.bBurstWindowActive = false; // still save 40-Ult for stage boss
	}

	// Fallback generic boss casting danger check if no specific signal threat was found
	if( !res.BossThreat.bBotCurrentlyInDanger && res.hPriorityBoss && !res.hPriorityBoss->IsDie() )
	{
		CDnMonsterActor* pBoss = static_cast<CDnMonsterActor*>( res.hPriorityBoss.GetPointer() );
		float fBossDist = XZDist( vMyPos, *pBoss->GetPosition() );
		float fBossRad = GetActorEffectiveRadius( pBoss );
		if( pBoss->IsProcessSkill() && fBossDist < ( fBossRad + 260.f ) )
		{
			EtVector3 vToMe = vMyPos - *pBoss->GetPosition();
			vToMe.y = 0.f;
			if( EtVec3LengthSq( &vToMe ) > 1.f )
			{
				EtVec3Normalize( &vToMe, &vToMe );
				EtVector3 vBossZ = pBoss->GetMatEx()->m_vZAxis;
				vBossZ.y = 0.f;
				if( EtVec3LengthSq( &vBossZ ) > 0.01f )
					EtVec3Normalize( &vBossZ, &vBossZ );
				if( EtVec3Dot( &vToMe, &vBossZ ) > 0.45f )
				{
					res.bBossCastingNearMe = true;
					res.vDangerAvoidVec = EtVector3( -vToMe.z * m_fStrafeSign, 0.f, vToMe.x * m_fStrafeSign );
				}
			}
		}
	}

	// 3. Detect Active Stage Portals & Human Gate Readiness / Approach
	if( pWorld && res.hHumanLeader && !res.hHumanLeader->IsDie() )
	{
		int nHumanEnteredGate = -1;
		if( CDnPartyTask::IsActive( m_pActor->GetRoom() ) )
			nHumanEnteredGate = CDnPartyTask::GetInstance( m_pActor->GetRoom() ).GetHumanEnteredGateIndex();

		if( nHumanEnteredGate != -1 )
		{
			GateStruct* pGate = pWorld->GetGateStruct( (char)nHumanEnteredGate );
			if( pGate && pGate->pGateArea && pGate->pGateArea->IsActive() )
			{
				res.bHumanInGate = true;
				res.nActiveGateIndex = nHumanEnteredGate;
				res.vGateTargetPos = ComputeGateStandPoint( pGate->pGateArea );
			}
		}
		else if( !res.bHasAnyEnemy )
		{
			EtVector3 vLeaderPos = *res.hHumanLeader->GetPosition();
			float fBestGateDistSq = 900.f * 900.f;
			for( DWORD g = 0; g < pWorld->GetGateCount(); ++g )
			{
				GateStruct* pGate = pWorld->GetGateStructFromIndex( g );
				if( !pGate || !pGate->pGateArea || !pGate->pGateArea->IsActive() )
					continue;
				if( pGate->PermitFlag != EWorldEnum::PermitEnter )
					continue;

				EtVector3 vGateCenter = pGate->pGateArea->GetOBB()->Center;
				float fDistSq = XZDistSq( vLeaderPos, vGateCenter );
				if( pGate->pGateArea->CheckArea( vLeaderPos, true ) )
				{
					res.bHumanInGate = true;
					res.nActiveGateIndex = (int)pGate->cGateIndex;
					res.vGateTargetPos = ComputeGateStandPoint( pGate->pGateArea );
					break;
				}
				if( fDistSq < fBestGateDistSq )
				{
					fBestGateDistSq = fDistSq;
					res.bHumanApproachingGate = true;
					res.nActiveGateIndex = (int)pGate->cGateIndex;
					res.vGateTargetPos = ComputeGateStandPoint( pGate->pGateArea );
				}
			}
		}
	}

	// 4. Scan World Props (Traps, Nest Safe-Zone Buff Props, Destructible Barricades / Crystals)
	if( pWorld )
	{
		DNVector(CEtWorldProp*) vecProps;
		pWorld->ScanProp( vMyPos, 1500.f, vecProps );

		float fBestBuffDistSq = FLT_MAX;
		float fBestBreakDistSq = FLT_MAX;
		for( size_t i = 0; i < vecProps.size(); ++i )
		{
			CDnWorldProp* pProp = static_cast<CDnWorldProp*>( vecProps[i] );
			if( !pProp || !pProp->IsShow() ) continue;

			EtVector3 vPropPos = pProp->GetMatEx()->m_vPosition;
			float fDistSq = XZDistSq( vMyPos, vPropPos );
			PropTypeEnum eType = pProp->GetPropType();

			if( eType == PTE_Trap )
			{
				if( fDistSq < ( 210.f * 210.f ) )
				{
					res.bHasTrapDanger = true;
					EtVector3 vAway = vMyPos - vPropPos;
					vAway.y = 0.f;
					if( EtVec3LengthSq( &vAway ) < 1.f )
						vAway = EtVector3( 1.f, 0.f, 0.f );
					EtVec3Normalize( &vAway, &vAway );
					res.vDangerAvoidVec = vAway;
				}
			}
			else if( eType == PTE_Buff || eType == PTE_BuffBroken )
			{
				if( fDistSq < fBestBuffDistSq )
				{
					fBestBuffDistSq = fDistSq;
					res.hSafeBuffProp = pProp->GetMySmartPtr();
				}
			}

			if( pProp->IsBrokenType() )
			{
				CDnWorldBrokenProp* pBroken = static_cast<CDnWorldBrokenProp*>( pProp );
				if( !pBroken->IsBroken() && pBroken->IsHittable( m_pActor->GetActorHandle(), LocalTime ) )
				{
					if( fDistSq < fBestBreakDistSq )
					{
						fBestBreakDistSq = fDistSq;
						res.hBreakableProp = pProp->GetMySmartPtr();
					}
				}
			}
		}
	}

	m_LastScan = res;
	m_hTargetActor = res.hBestEnemy;
	if( m_hTargetActor )
	{
		m_vAimPos = ComputeTargetAimPoint( m_hTargetActor.GetPointer(), true );
	}
	else if( res.hBlockingBreakableProp )
	{
		SAABox propBox;
		propBox.Reset();
		res.hBlockingBreakableProp->GetBoundingBox( propBox );
		m_vAimPos = res.hBlockingBreakableProp->GetMatEx()->m_vPosition;
		m_vAimPos.y = ( propBox.Max.y > propBox.Min.y ) ? ( propBox.Min.y + ( propBox.Max.y - propBox.Min.y ) * 0.5f ) : ( m_vAimPos.y + 50.f );
	}
	else if( res.hBreakableProp )
	{
		SAABox propBox;
		propBox.Reset();
		res.hBreakableProp->GetBoundingBox( propBox );
		m_vAimPos = res.hBreakableProp->GetMatEx()->m_vPosition;
		m_vAimPos.y = ( propBox.Max.y > propBox.Min.y ) ? ( propBox.Min.y + ( propBox.Max.y - propBox.Min.y ) * 0.5f ) : ( m_vAimPos.y + 50.f );
	}
}

void CDnAIBotController::AnalyzeBossThreat( CDnMonsterActor* pBoss, const EtVector3& vMyPos, BossThreatInfo& outThreat ) const
{
	outThreat = BossThreatInfo();
	if( !pBoss || pBoss->IsDie() || !m_pActor )
		return;

	int nActIdx = pBoss->GetCurrentActionIndex();
	if( nActIdx < 0 )
		return;
	CDnActionBase::ActionElementStruct* pElem = pBoss->GetElement( nActIdx );
	if( !pElem )
		return;

	const char* szActName = pElem->szName.c_str();
	if( !szActName || strstr( szActName, "Stand" ) || strstr( szActName, "Move" ) || strstr( szActName, "Hit" ) || strstr( szActName, "Down" ) || strstr( szActName, "Die" ) )
		return;

	float fCurFrame = pBoss->GetCurFrame();
	float fFPS = pBoss->CDnActionBase::GetFPS();
	if( fFPS < 1.f ) fFPS = 60.f;

	EtVector3 vBossPos = *pBoss->GetPosition();
	EtVector3 vBossFwd3 = pBoss->GetMatEx()->m_vZAxis;
	vBossFwd3.y = 0.f;
	if( EtVec3LengthSq( &vBossFwd3 ) < 0.001f ) vBossFwd3 = EtVector3( 0.f, 0.f, 1.f );
	EtVec3Normalize( &vBossFwd3, &vBossFwd3 );
	EtVector3 vBossRight3 = pBoss->GetMatEx()->m_vXAxis;
	vBossRight3.y = 0.f;
	if( EtVec3LengthSq( &vBossRight3 ) < 0.001f ) vBossRight3 = EtVector3( 1.f, 0.f, 0.f );
	EtVec3Normalize( &vBossRight3, &vBossRight3 );

	float fMyRadius = GetActorEffectiveRadius( m_pActor );
	float fDefensivePad = 35.f + m_fDefensiveBias * 45.f;

	// 1. Live Engine Action Signal Inspection (reads upcoming STE_Hit frames directly!)
	std::map<int, std::vector<CEtActionSignal*>>::iterator itHit = pElem->MapSearch.find( STE_Hit );
	if( itHit != pElem->MapSearch.end() )
	{
		for( size_t i = 0; i < itHit->second.size(); ++i )
		{
			CEtActionSignal* pSig = itHit->second[i];
			if( !pSig || !pSig->GetData() ) continue;
			if( (float)pSig->GetEndFrame() < fCurFrame ) continue;

			float fSecUntilHit = ( (float)pSig->GetStartFrame() - fCurFrame ) / fFPS;
			if( fSecUntilHit > 1.25f ) continue;
			if( fSecUntilHit < 0.f ) fSecUntilHit = 0.f;

			HitStruct* pHit = static_cast<HitStruct*>( pSig->GetData() );
			EtVector3 vHitOrigin = vBossPos;
			if( pHit->vOffset )
			{
				vHitOrigin += vBossRight3 * pHit->vOffset->x;
				vHitOrigin += vBossFwd3 * pHit->vOffset->z;
			}

			float fDistFromHit = XZDist( vHitOrigin, vMyPos ) - fMyRadius;
			if( fDistFromHit < 0.f ) fDistFromHit = 0.f;

			float fMaxR = pHit->fDistanceMax + ( pHit->bIncludeWeaponLength ? 110.f : 0.f ) + fDefensivePad;
			float fMinR = ( pHit->fDistanceMin > 30.f ) ? ( pHit->fDistanceMin - 25.f ) : 0.f;

			bool bAngleMatch = true;
			bool bIsFrontalCone = ( pHit->fAngle > 1.f && pHit->fAngle < 320.f );
			if( bIsFrontalCone )
			{
				EtVector2 vConeCenter = RotateDir2Deg( EtVector2( vBossFwd3.x, vBossFwd3.z ), pHit->fCenterAngle );
				EtVector2 vToMe = SafeDir2( vHitOrigin, vMyPos );
				float fAngDiff = ComputeAngleDiffDeg( vConeCenter, vToMe );
				bAngleMatch = ( fAngDiff <= ( pHit->fAngle * 0.5f + 22.f ) );
			}

			// Height check (ground stomp vs full-height cleave)
			bool bJumpableStomp = ( pHit->fHeightMax > 0.f && pHit->fHeightMax <= 95.f && pHit->fHeightMin <= 40.f && fMaxR >= 210.f );

			bool bInDanger = ( fDistFromHit >= fMinR && fDistFromHit <= fMaxR && bAngleMatch );
			if( !bInDanger ) continue;

			BossThreatKind eKind = Threat_PointBlankAoE;
			if( bJumpableStomp )
				eKind = Threat_StompJump;
			else if( pHit->fDistanceMin >= 145.f )
				eKind = Threat_DonutOuter;
			else if( fMaxR >= 650.f && !bIsFrontalCone )
				eKind = Threat_RoomWipe;
			else if( bIsFrontalCone && pHit->fAngle <= 165.f )
				eKind = Threat_FrontalCleave;

			if( !outThreat.bBotCurrentlyInDanger || fSecUntilHit < outThreat.fTimeToImpact || eKind == Threat_RoomWipe )
			{
				outThreat.eKind = eKind;
				outThreat.hBoss = pBoss->GetActorHandle();
				outThreat.vBossPos = vBossPos;
				outThreat.vBossForward = vBossFwd3;
				outThreat.vHitOrigin = vHitOrigin;
				outThreat.fTimeToImpact = fSecUntilHit;
				outThreat.fSafeMinDist = pHit->fDistanceMin;
				outThreat.fSafeMaxDist = fMaxR + 60.f;
				outThreat.fThreatDamageRatio = ClampF( pHit->fDamageProb * 0.15f, 0.12f, 0.9f );
				outThreat.bBotCurrentlyInDanger = true;
			}
		}
	}

	// 2. Upcoming Boss Projectile Telegraph Check
	if( !outThreat.bBotCurrentlyInDanger )
	{
		std::map<int, std::vector<CEtActionSignal*>>::iterator itProj = pElem->MapSearch.find( STE_Projectile );
		if( itProj != pElem->MapSearch.end() )
		{
			float fCenterDist = XZDist( vBossPos, vMyPos );
			EtVector2 vToMe = SafeDir2( vBossPos, vMyPos );
			EtVector2 vBossFwd2( vBossFwd3.x, vBossFwd3.z );
			if( fCenterDist < 950.f && ComputeAngleDiffDeg( vBossFwd2, vToMe ) <= 38.f )
			{
				for( size_t i = 0; i < itProj->second.size(); ++i )
				{
					CEtActionSignal* pSig = itProj->second[i];
					if( !pSig ) continue;
					float fSecUntil = ( (float)pSig->GetStartFrame() - fCurFrame ) / fFPS;
					if( fSecUntil >= -0.05f && fSecUntil <= 0.85f )
					{
						outThreat.eKind = Threat_Projectile;
						outThreat.hBoss = pBoss->GetActorHandle();
						outThreat.vBossPos = vBossPos;
						outThreat.vBossForward = vBossFwd3;
						outThreat.vHitOrigin = vBossPos;
						outThreat.fTimeToImpact = ( fSecUntil > 0.f ) ? fSecUntil : 0.f;
						outThreat.fSafeMaxDist = 260.f;
						outThreat.fThreatDamageRatio = 0.22f;
						outThreat.bBotCurrentlyInDanger = true;
						break;
					}
				}
			}
		}
	}

	// 3. Adaptive Learned Boss Memory Overlay (catches delayed/projectile/trigger mechanics learned from past hits!)
	BossActionLearnEntry learned;
	if( CTacticalLearningMemory::Get().QueryBossAction( pBoss->GetMonsterClassID(), szActName, learned ) )
	{
		if( learned.nHitCount >= 1 && learned.fMaxDamageRatio >= 0.10f )
		{
			float fDistToBoss = XZDist( vBossPos, vMyPos ) - fMyRadius;
			if( fDistToBoss < 0.f ) fDistToBoss = 0.f;
			float fLearnedReach = learned.fObservedMaxHitDist + 65.f + fDefensivePad;
			bool bMostlyFrontal = ( learned.nHitCount >= 2 && ( (float)learned.nFrontalHits / (float)learned.nHitCount ) >= 0.75f );

			bool bAngleThreat = true;
			if( bMostlyFrontal )
			{
				EtVector2 vToMe = SafeDir2( vBossPos, vMyPos );
				bAngleThreat = ( ComputeAngleDiffDeg( EtVector2( vBossFwd3.x, vBossFwd3.z ), vToMe ) <= 85.f );
			}

			if( fDistToBoss <= fLearnedReach && bAngleThreat )
			{
				if( !outThreat.bBotCurrentlyInDanger || learned.fMaxDamageRatio > outThreat.fThreatDamageRatio )
				{
					float fRemainSec = ( (float)pElem->dwLength - fCurFrame ) / fFPS;
					outThreat.hBoss = pBoss->GetActorHandle();
					outThreat.vBossPos = vBossPos;
					outThreat.vBossForward = vBossFwd3;
					outThreat.vHitOrigin = vBossPos;
					outThreat.fTimeToImpact = ClampF( fRemainSec * 0.45f, 0.05f, 0.75f );
					outThreat.fSafeMaxDist = fLearnedReach + 55.f;
					outThreat.fThreatDamageRatio = learned.fMaxDamageRatio;
					outThreat.bBotCurrentlyInDanger = true;

					if( learned.bObservedJumpable )
						outThreat.eKind = Threat_StompJump;
					else if( learned.fObservedMaxHitDist >= 620.f || learned.fMaxDamageRatio >= 0.45f )
						outThreat.eKind = Threat_RoomWipe;
					else if( bMostlyFrontal )
						outThreat.eKind = Threat_FrontalCleave;
					else
						outThreat.eKind = Threat_PointBlankAoE;
				}
			}
		}
	}
}

const CDnAIBotController::SkillProfile& CDnAIBotController::GetOrAnalyzeSkillProfile( CDnSkill* pSkill )
{
	static SkillProfile s_Fallback;
	if( !pSkill || !m_pActor )
		return s_Fallback;

	int nSkillID = pSkill->GetClassID();
	std::map<int, SkillProfile>::iterator it = m_mapSkillProfiles.find( nSkillID );
	if( it != m_mapSkillProfiles.end() && it->second.bAnalyzed )
		return it->second;

	SkillProfile& prof = m_mapSkillProfiles[nSkillID];
	prof = SkillProfile();
	prof.nSkillID = nSkillID;
	prof.bAnalyzed = true;
	prof.bIsUltimate = pSkill->bIsExtremitySkill();
	prof.bIsBurstBuff = IsRaidBurstBuffSkill( pSkill );
	prof.bIsDebuffOrArmorBreak =
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_029 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_030 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_041 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_055 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_060 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_144 );

	// Collect all root action names belonging to this skill
	std::vector<std::string> vecRootActions;
	IDnSkillProcessor* pPlayAni = pSkill->GetProcessor( IDnSkillProcessor::PLAY_ANI );
	if( pPlayAni )
	{
		CDnPlayAniProcess* pAni = static_cast<CDnPlayAniProcess*>( pPlayAni );
		if( pAni->GetActionName() && pAni->GetActionName()[0] )
			vecRootActions.push_back( pAni->GetActionName() );
		if( pAni->GetChainActionName() && pAni->GetChainActionName()[0] )
			vecRootActions.push_back( pAni->GetChainActionName() );
	}
	IDnSkillProcessor* pPartial = pSkill->GetProcessor( IDnSkillProcessor::PARTIAL_PLAY_ANI );
	if( pPartial )
	{
		CDnPartialPlayProcessor* pPart = static_cast<CDnPartialPlayProcessor*>( pPartial );
		if( pPart->GetStartActionName() && pPart->GetStartActionName()[0] )
			vecRootActions.push_back( pPart->GetStartActionName() );
		if( pPart->GetLoopActionName() && pPart->GetLoopActionName()[0] )
			vecRootActions.push_back( pPart->GetLoopActionName() );
		if( pPart->GetEndActionName() && pPart->GetEndActionName()[0] )
			vecRootActions.push_back( pPart->GetEndActionName() );
	}
	const std::set<std::string>& setUse = pSkill->GetUseActionNames();
	for( std::set<std::string>::const_iterator sit = setUse.begin(); sit != setUse.end(); ++sit )
	{
		if( !sit->empty() )
			vecRootActions.push_back( *sit );
	}

	float fWeaponLen = 0.f;
	if( m_pActor->GetWeapon( 0 ) && m_pActor->GetWeapon( 0 )->GetWeaponLength() > 0 )
		fWeaponLen = (float)m_pActor->GetWeapon( 0 )->GetWeaponLength();

	float fBestHitMax = 0.f;
	float fBestHitMin = FLT_MAX;
	float fBestProjMax = 0.f;
	float fBestSummonMax = 0.f;
	float fMaxVert = 130.f;
	float fDashAccum = 0.f;
	float fTotalDmgProb = 0.f;

	std::set<std::string> visited;
	std::vector<std::string> queue = vecRootActions;

	for( size_t q = 0; q < queue.size() && q < 12; ++q )
	{
		const std::string& strAct = queue[q];
		if( strAct.empty() || visited.count( strAct ) )
			continue;
		visited.insert( strAct );

		CDnActionBase::ActionElementStruct* pElem = m_pActor->GetElement( strAct.c_str() );
		if( !pElem )
			continue;

		float fFPS = m_pActor->CDnActionBase::GetFPS();
		if( fFPS < 1.f ) fFPS = 60.f;

		// 1. Velocity/Dash signals (detect gap-closers & backsteps accurately)
		float fLocalForwardDash = 0.f;
		std::map<int, std::vector<CEtActionSignal*>>::iterator itVel = pElem->MapSearch.find( STE_VelocityAccel );
		if( itVel != pElem->MapSearch.end() )
		{
			for( size_t i = 0; i < itVel->second.size(); ++i )
			{
				CEtActionSignal* pSig = itVel->second[i];
				VelocityAccelStruct* pVel = pSig ? static_cast<VelocityAccelStruct*>( pSig->GetData() ) : NULL;
				if( !pVel || !pVel->vVelocity ) continue;

				float fDur = (float)( pSig->GetEndFrame() - pSig->GetStartFrame() ) / fFPS;
				if( fDur < 0.05f ) fDur = 0.25f;
				if( fDur > 0.85f ) fDur = 0.85f;
				float fDispZ = pVel->vVelocity->z * fDur;
				if( fDispZ > 60.f )
				{
					fLocalForwardDash += ClampF( fDispZ * 0.65f, 0.f, 340.f );
					prof.bIsDashForward = true;
				}
				else if( fDispZ < -90.f )
				{
					prof.bIsBackstep = true;
				}
			}
		}
		fDashAccum += fLocalForwardDash;

		// 2. I-Frame signals (STE_CanHit with bHittable == FALSE)
		std::map<int, std::vector<CEtActionSignal*>>::iterator itCanHit = pElem->MapSearch.find( STE_CanHit );
		if( itCanHit != pElem->MapSearch.end() )
		{
			for( size_t i = 0; i < itCanHit->second.size(); ++i )
			{
				CanHitStruct* pCH = static_cast<CanHitStruct*>( itCanHit->second[i]->GetData() );
				if( pCH && !pCH->bHittable )
				{
					int nFrames = itCanHit->second[i]->GetEndFrame() - itCanHit->second[i]->GetStartFrame();
					if( nFrames >= 8 )
						prof.bHasIFrame = true;
				}
			}
		}

		// 3. Melee / AoE Hit signals (EXACT engine hit reach!)
		std::map<int, std::vector<CEtActionSignal*>>::iterator itHit = pElem->MapSearch.find( STE_Hit );
		if( itHit != pElem->MapSearch.end() )
		{
			for( size_t i = 0; i < itHit->second.size(); ++i )
			{
				HitStruct* pHit = static_cast<HitStruct*>( itHit->second[i]->GetData() );
				if( !pHit ) continue;

				prof.bHasHitSignal = true;
				float fOffsetZ = ( pHit->vOffset ) ? pHit->vOffset->z : 0.f;
				float fReach = fOffsetZ + pHit->fDistanceMax + ( pHit->bIncludeWeaponLength ? fWeaponLen : 0.f ) + fLocalForwardDash;
				if( fReach > fBestHitMax )
					fBestHitMax = fReach;

				float fMinReach = pHit->fDistanceMin;
				if( fOffsetZ > pHit->fDistanceMax + 35.f )
					fMinReach = ( fOffsetZ - pHit->fDistanceMax > fMinReach ) ? ( fOffsetZ - pHit->fDistanceMax ) : fMinReach;
				if( fMinReach < fBestHitMin )
					fBestHitMin = fMinReach;

				if( pHit->fHeightMax > fMaxVert )
					fMaxVert = ClampF( pHit->fHeightMax, 130.f, 450.f );

				if( pHit->fAngle >= 210.f || pHit->fAngle <= 0.1f || pHit->fDistanceMax >= 240.f )
					prof.bIsAoE = true;

				if( pHit->fDamageProb > 0.f )
					fTotalDmgProb += pHit->fDamageProb;
				else
					fTotalDmgProb += 1.0f;
			}
		}

		// 4. Projectile signals (EXACT travel range based on OrbitType, Speed, ValidTime, Offset & DestPosition!)
		std::map<int, std::vector<CEtActionSignal*>>::iterator itProj = pElem->MapSearch.find( STE_Projectile );
		if( itProj != pElem->MapSearch.end() )
		{
			for( size_t i = 0; i < itProj->second.size(); ++i )
			{
				ProjectileStruct* pProj = static_cast<ProjectileStruct*>( itProj->second[i]->GetData() );
				if( !pProj ) continue;

				prof.bHasProjectile = true;
				fTotalDmgProb += 1.25f;

				float fOffZ = ( pProj->vOffset ) ? fabsf( pProj->vOffset->z ) : 0.f;
				float fProjRange = 0.f;

				if( pProj->nTargetType == CDnProjectile::DestPosition && pProj->vDestPosition )
				{
					float fDestXZ = sqrtf( pProj->vDestPosition->x * pProj->vDestPosition->x + pProj->vDestPosition->z * pProj->vDestPosition->z );
					fProjRange = fDestXZ + 110.f;
				}
				else if( pProj->fSpeed > 15.f && pProj->nValidTime > 0 )
				{
					float fTimeSec = (float)pProj->nValidTime / 1000.f;
					if( fTimeSec > 2.5f ) fTimeSec = 2.5f;
					float fTravel = pProj->fSpeed * fTimeSec;
					if( pProj->nOrbitType == CDnProjectile::Acceleration && pProj->fResistance < -1.f )
					{
						float fStopT = -pProj->fSpeed / pProj->fResistance;
						float fEffT = ( fStopT < fTimeSec ) ? fStopT : fTimeSec;
						fTravel = pProj->fSpeed * fEffT + 0.5f * pProj->fResistance * fEffT * fEffT;
					}
					fProjRange = fOffZ + fTravel;
				}
				else if( pProj->nTargetType == CDnProjectile::TargetPosition || pProj->nTargetType == CDnProjectile::Target )
				{
					// Ground-targeted spell (Gravity Ascension, Blizzard, Meteor, Lightning, etc.)
					fProjRange = ( fOffZ > 40.f ) ? ( fOffZ + 170.f ) : 500.f;
				}
				else
				{
					// Stationary / point-blank spawned field/wave (Freezing Field, Ice Nova, Flame Wall, Poison Pool)
					fProjRange = fOffZ + 185.f;
				}

				if( pProj->nWeaponTableID > 0 )
					fProjRange += 35.f;

				fProjRange = ClampF( fProjRange, 130.f, 820.f );
				if( fProjRange > fBestProjMax )
					fBestProjMax = fProjRange;
			}
		}

		// 5. Summon signals (Relics, Towers, Ducks, Mirrors)
		std::map<int, std::vector<CEtActionSignal*>>::iterator itSum = pElem->MapSearch.find( STE_SummonMonster );
		if( itSum != pElem->MapSearch.end() && !itSum->second.empty() )
		{
			prof.bIsSummon = true;
			prof.bIsAoE = true;
			fTotalDmgProb += 2.0f;
			SummonMonsterStruct* pSum = static_cast<SummonMonsterStruct*>( itSum->second[0]->GetData() );
			float fSumOff = ( pSum && pSum->vPosition ) ? fabsf( pSum->vPosition->z ) : 120.f;
			float fSumReach = ClampF( fSumOff + 340.f, 240.f, 520.f );
			if( fSumReach > fBestSummonMax )
				fBestSummonMax = fSumReach;
		}

		// Follow next action in chain (skip Stand / Landing loops)
		if( !pElem->szNextActionName.empty() &&
			pElem->szNextActionName != "Stand" &&
			pElem->szNextActionName != "Stand_Battle" &&
			!visited.count( pElem->szNextActionName ) )
		{
			queue.push_back( pElem->szNextActionName );
		}
	}

	prof.fDashDistance = ClampF( fDashAccum, 0.f, 360.f );
	prof.fMaxVerticalReach = fMaxVert;
	prof.fTotalDamageProb = ( fTotalDmgProb > 0.1f ) ? fTotalDmgProb : 1.0f;

	// Combine computed engine reaches cleanly
	float fComputedMax = 0.f;
	if( fBestProjMax > fComputedMax ) fComputedMax = fBestProjMax;
	if( fBestHitMax > fComputedMax ) fComputedMax = fBestHitMax;
	if( fBestSummonMax > fComputedMax ) fComputedMax = fBestSummonMax;

	// Respect explicit RANGE_CHECKER & IncreaseRange from SkillTable
	IDnSkillUsableChecker* pRangeChk = pSkill->GetChecker( IDnSkillUsableChecker::RANGE_CHECKER );
	if( pRangeChk )
	{
		int nChkRange = static_cast<CDnRangeChecker*>( pRangeChk )->GetRange();
		if( nChkRange > 0 )
		{
			if( fComputedMax <= 0.f || (float)nChkRange < fComputedMax )
				fComputedMax = (float)nChkRange;
		}
	}
	if( pSkill->GetIncreaseRange() > 0 && fComputedMax > 0.f )
		fComputedMax += (float)pSkill->GetIncreaseRange();

	// Tight conservative fallback if a skill's hit comes purely from a delayed sub-entity/state effect
	if( fComputedMax <= 10.f )
	{
		if( prof.bIsDebuffOrArmorBreak || IsThreatSkill( pSkill ) )
			fComputedMax = 260.f;
		else
			fComputedMax = HasRangedBasicAttack() ? 380.f : 175.f;
	}

	prof.fMaxRange = ClampF( fComputedMax, 95.f, 820.f );
	prof.fMinRange = ( fBestHitMin < FLT_MAX && !prof.bHasProjectile ) ? ClampF( fBestHitMin, 0.f, prof.fMaxRange * 0.55f ) : 0.f;
	prof.fOptimalRange = ClampF( prof.fMaxRange * 0.72f, 70.f, 560.f );
	if( prof.fOptimalRange < prof.fMinRange + 35.f )
		prof.fOptimalRange = prof.fMinRange + 35.f;

	return prof;
}

float CDnAIBotController::GetEffectiveSkillMaxRange( const SkillProfile& prof ) const
{
	// Apply 0.88x safety buffer so bots never fire at the ragged outer tip where small enemy movement whiffs
	float fMax = prof.fMaxRange * 0.88f;

	if( m_pActor )
	{
		SkillLearnEntry learned;
		if( CTacticalLearningMemory::Get().QuerySkillLearning( m_pActor->GetJobClassID(), prof.nSkillID, learned ) )
		{
			if( learned.fLearnedMaxRange > 60.f && learned.fLearnedMaxRange < fMax )
				fMax = learned.fLearnedMaxRange;
		}
	}
	return ClampF( fMax, 75.f, 750.f );
}

float CDnAIBotController::GetDesiredCombatRange( float fTargetRadius, float fMyRadius ) const
{
	// Dynamically derive the bot's desired spacing from its READY offensive skills' real reach!
	// That way a Sorceress/Cleric/Kali/Academic with short-range skills walks close enough to land them,
	// while a Sniper/Artillery/GearMaster holds an authentic ranged spacing!
	bool bRangedBasic = HasRangedBasicAttack();
	float fBestReadyOpt = -1.f;
	float fMinReadyMax = FLT_MAX;

	if( m_pActor )
	{
		for( DWORD i = 0; i < m_pActor->GetSkillCount(); ++i )
		{
			DnSkillHandle hSkill = m_pActor->GetSkillFromIndex( i );
			if( !hSkill ) continue;
			CDnSkill* pSkill = hSkill.GetPointer();
			if( !IsOffensiveActiveSkill( pSkill ) ) continue;
			if( pSkill->CanExecute() != CDnSkill::UsingResult::Success ) continue;
			if( pSkill->bIsExtremitySkill() && !m_LastScan.bBurstWindowActive ) continue;

			const SkillProfile& prof = const_cast<CDnAIBotController*>( this )->GetOrAnalyzeSkillProfile( pSkill );
			float fEffMax = GetEffectiveSkillMaxRange( prof );
			if( fEffMax < fMinReadyMax )
				fMinReadyMax = fEffMax;
			if( prof.fOptimalRange > fBestReadyOpt && fEffMax <= ( bRangedBasic ? 540.f : 230.f ) )
				fBestReadyOpt = prof.fOptimalRange;
		}
	}

	if( m_Role == AIBot::RoleType::Tank )
		return 65.f;

	if( !bRangedBasic )
	{
		if( fMinReadyMax < FLT_MAX )
			return ClampF( fMinReadyMax * 0.72f, 60.f, 155.f );
		return 85.f;
	}

	// Ranged/Hybrid classes: stand comfortably inside their shortest ready skill's effective range (capped naturally)
	if( fMinReadyMax < FLT_MAX )
	{
		float fStand = ClampF( fMinReadyMax * 0.78f, 120.f, 430.f );
		// In Nests with high defensive bias, ranged classes stay slightly further back when possible
		if( m_eEncounterMode == Mode_Nest || m_eEncounterMode == Mode_DragonNest )
			fStand = ClampF( fStand + m_fDefensiveBias * 70.f, 140.f, 480.f );
		return fStand;
	}

	if( m_Role == AIBot::RoleType::Healer )
		return 340.f;
	return 310.f;
}

bool CDnAIBotController::TryHandleHitRecovery()
{
	if( !m_pActor ) return false;

	int nCurIdx = m_pActor->GetCurrentActionIndex();
	CDnActionBase::ActionElementStruct* pElem = ( nCurIdx >= 0 ) ? m_pActor->GetElement( nCurIdx ) : NULL;
	float fCurFrame = m_pActor->GetCurFrame();

	// 1. Scan real STE_InputHasPassiveSkill signals on current Hit_Air / Down action (Aerial Evasion, WakeUp Attack!)
	if( pElem )
	{
		std::map<int, std::vector<CEtActionSignal*>>::iterator itPass = pElem->MapSearch.find( STE_InputHasPassiveSkill );
		if( itPass != pElem->MapSearch.end() )
		{
			for( size_t i = 0; i < itPass->second.size(); ++i )
			{
				CEtActionSignal* pSig = itPass->second[i];
				if( !pSig || !pSig->GetData() ) continue;
				if( fCurFrame < (float)pSig->GetStartFrame() || fCurFrame > (float)pSig->GetEndFrame() ) continue;

				InputHasPassiveSkillStruct* pData = static_cast<InputHasPassiveSkillStruct*>( pSig->GetData() );
				if( pData->nSkillIndex <= 0 ) continue;
				DnSkillHandle hSkill = m_pActor->FindSkill( pData->nSkillIndex );
				if( !hSkill || hSkill->CanExecute() != CDnSkill::UsingResult::Success ) continue;

				const char* szAct = ( hSkill->IsEnchantedSkill() && pData->szEXSkillChangeAction && pData->szEXSkillChangeAction[0] )
					? pData->szEXSkillChangeAction
					: pData->szChangeAction;
				if( !szAct || !szAct[0] || !m_pActor->GetElement( szAct ) ) continue;

				m_pActor->CmdPassiveSkillAction( pData->nSkillIndex, szAct, 0, (float)pData->nBlendFrame, 0.f, false, false, false );
				SendActionPacket( szAct, 0, (float)pData->nBlendFrame, false );
				m_eMoveState = MoveState_Stopped;
				return true;
			}
		}

		// 2. On Knockdown (Down_*), also check regular STE_Input recovery (Stand_Up / Roll)
		if( m_pActor->IsDown() )
		{
			std::map<int, std::vector<CEtActionSignal*>>::iterator itIn = pElem->MapSearch.find( STE_Input );
			if( itIn != pElem->MapSearch.end() )
			{
				for( size_t i = 0; i < itIn->second.size(); ++i )
				{
					CEtActionSignal* pSig = itIn->second[i];
					if( !pSig || !pSig->GetData() ) continue;
					if( fCurFrame < (float)pSig->GetStartFrame() || fCurFrame > (float)pSig->GetEndFrame() ) continue;

					InputStruct* pData = static_cast<InputStruct*>( pSig->GetData() );
					if( !pData->szChangeAction || !pData->szChangeAction[0] ) continue;
					if( !m_pActor->GetElement( pData->szChangeAction ) ) continue;

					m_pActor->CmdAction( pData->szChangeAction, 0, (float)pData->nBlendFrame, false, false, false );
					SendActionPacket( pData->szChangeAction, 0, (float)pData->nBlendFrame, false );
					m_eMoveState = MoveState_Stopped;
					return true;
				}
			}
		}
	}

	return false;
}

bool CDnAIBotController::TrySkillChainFollowUp()
{
	if( !m_pActor || !m_pActor->IsProcessSkill() )
		return false;
	if( !m_hTargetActor || m_hTargetActor->IsDie() )
		return false;

	int nCurIdx = m_pActor->GetCurrentActionIndex();
	CDnActionBase::ActionElementStruct* pElem = ( nCurIdx >= 0 ) ? m_pActor->GetElement( nCurIdx ) : NULL;
	if( !pElem )
		return false;

	float fCurFrame = m_pActor->GetCurFrame();
	std::map<int, std::vector<CEtActionSignal*>>::iterator itIn = pElem->MapSearch.find( STE_Input );
	if( itIn == pElem->MapSearch.end() )
		return false;

	float fDistToBox = XZDist( *m_pActor->GetPosition(), *m_hTargetActor->GetPosition() ) - GetActorEffectiveRadius( m_hTargetActor.GetPointer() );
	if( fDistToBox < 0.f ) fDistToBox = 0.f;
	float fMaxChainDist = HasRangedBasicAttack() ? 520.f : 220.f;
	if( fDistToBox > fMaxChainDist )
		return false;

	for( size_t i = 0; i < itIn->second.size(); ++i )
	{
		CEtActionSignal* pSig = itIn->second[i];
		if( !pSig || !pSig->GetData() ) continue;
		if( fCurFrame < (float)pSig->GetStartFrame() || fCurFrame > (float)pSig->GetEndFrame() ) continue;

		InputStruct* pData = static_cast<InputStruct*>( pSig->GetData() );
		if( !pData->bSkillChain || !pData->szChangeAction || !pData->szChangeAction[0] ) continue;

		int nNextIdx = m_pActor->GetElementIndex( pData->szChangeAction );
		if( nNextIdx == -1 ) continue;
		if( !m_pActor->IsValidSkillChain( nCurIdx, nNextIdx ) ) continue;

		// Only chain once we're past the first third of the input window so early hits finish cleanly
		float fWindowSpan = (float)( pSig->GetEndFrame() - pSig->GetStartFrame() );
		if( fWindowSpan > 4.f && fCurFrame < ( (float)pSig->GetStartFrame() + fWindowSpan * 0.35f ) )
			continue;

		m_vAimPos = ComputeTargetAimPoint( m_hTargetActor.GetPointer(), true );
		FacePosition( m_vAimPos );
		m_pActor->CmdAction( pData->szChangeAction, 0, (float)pData->nBlendFrame, false, false, true );
		SendActionPacket( pData->szChangeAction, 0, (float)pData->nBlendFrame, true );
		return true;
	}

	return false;
}

bool CDnAIBotController::TryReactToBossThreat()
{
	BossThreatInfo& T = m_LastScan.BossThreat;
	if( !T.bBotCurrentlyInDanger || !T.hBoss )
		return false;

	EtVector3 vMyPos = *m_pActor->GetPosition();
	EtVector3 vAway = vMyPos - T.vHitOrigin;
	vAway.y = 0.f;
	if( EtVec3LengthSq( &vAway ) < 1.f )
		vAway = -T.vBossForward;
	EtVec3Normalize( &vAway, &vAway );

	EtVector3 vSide( -T.vBossForward.z * m_fStrafeSign, 0.f, T.vBossForward.x * m_fStrafeSign );

	switch( T.eKind )
	{
		case Threat_StompJump:
		{
			// Wait until right before stomp impact (0.04s - 0.32s) then JUMP over the shockwave like a pro!
			if( T.fTimeToImpact <= 0.34f && m_fJumpCooldown <= 0.f )
			{
				if( m_eEncounterMode == Mode_Nest || m_eEncounterMode == Mode_DragonNest )
					MaybeSendTacticalCallout( L"Stomp! Jump!", 26.f );
				if( TryJumpStomp() )
				{
					m_fJumpCooldown = 1.6f;
					m_fSkillDelayTimer = 0.25f;
					return true;
				}
			}
			// Fallback i-frame tumble if jump is on cooldown and impact is imminent
			if( T.fTimeToImpact <= 0.22f && m_fDodgeCooldown <= 0.f )
			{
				if( TryDirectionalTumble( vAway, &T.vBossPos ) )
				{
					m_fDodgeCooldown = 2.2f;
					return true;
				}
			}
			break;
		}
		case Threat_RoomWipe:
		{
			// Priority 1: If a Safe-Zone buff prop exists nearby, let movement sprint into it!
			if( m_LastScan.hSafeBuffProp && m_LastScan.hSafeBuffProp->IsShow() )
			{
				float fSafeDist = XZDist( vMyPos, m_LastScan.hSafeBuffProp->GetMatEx()->m_vPosition );
				if( fSafeDist <= 105.f )
					return false; // Already safely inside the buff prop!
				MaybeSendTacticalCallout( L"Safe zone!", 26.f );
				if( fSafeDist > 260.f && m_fDodgeCooldown <= 0.f )
				{
					EtVector3 vToSafe = m_LastScan.hSafeBuffProp->GetMatEx()->m_vPosition - vMyPos;
					vToSafe.y = 0.f;
					if( EtVec3LengthSq( &vToSafe ) > 1.f )
					{
						EtVec3Normalize( &vToSafe, &vToSafe );
						if( TryDirectionalTumble( vToSafe ) )
						{
							m_fDodgeCooldown = 2.5f;
							return true;
						}
					}
				}
				return false; // Movement Priority 0a sprints straight into safe zone!
			}
			// Priority 2: Precision i-frame skill or timed Tumble right before wipe impact
			if( T.fTimeToImpact <= 0.26f )
			{
				if( TryCastIFrameSkill( vAway ) )
					return true;
				if( m_fDodgeCooldown <= 0.f && TryDirectionalTumble( vAway, &T.vBossPos ) )
				{
					m_fDodgeCooldown = 2.4f;
					return true;
				}
			}
			break;
		}
		case Threat_DonutOuter:
		{
			// Safe zone is INSIDE near the boss -> dash INWARD!
			EtVector3 vInward = T.vBossPos - vMyPos;
			vInward.y = 0.f;
			if( EtVec3LengthSq( &vInward ) > 1.f )
			{
				EtVec3Normalize( &vInward, &vInward );
				if( T.fTimeToImpact <= 0.45f && m_fDodgeCooldown <= 0.f && TryDirectionalTumble( vInward, &T.vBossPos ) )
				{
					m_fDodgeCooldown = 2.2f;
					return true;
				}
			}
			break;
		}
		case Threat_FrontalCleave:
		case Threat_Projectile:
		{
			// Sidestep / tumble laterally behind or beside the boss's line of fire
			if( T.fTimeToImpact <= 0.42f && m_fDodgeCooldown <= 0.f )
			{
				if( TryDirectionalTumble( vSide, &T.vBossPos ) )
				{
					m_fDodgeCooldown = 2.2f;
					m_fSkillDelayTimer = 0.22f;
					return true;
				}
			}
			else if( T.fTimeToImpact <= 0.22f && TryCastIFrameSkill( vSide ) )
			{
				return true;
			}
			break;
		}
		case Threat_PointBlankAoE:
		{
			if( T.fTimeToImpact <= 0.36f )
			{
				if( m_fDodgeCooldown <= 0.f && TryDirectionalTumble( vAway, &T.vBossPos ) )
				{
					m_fDodgeCooldown = 2.2f;
					m_fSkillDelayTimer = 0.22f;
					return true;
				}
				if( T.fTimeToImpact <= 0.20f && TryCastIFrameSkill( vAway ) )
					return true;
			}
			break;
		}
		default:
			break;
	}

	return false;
}

bool CDnAIBotController::TryCastIFrameSkill( const EtVector3& vEscapeDir )
{
	if( !m_pActor ) return false;

	for( DWORD i = 0; i < m_pActor->GetSkillCount(); ++i )
	{
		DnSkillHandle hSkill = m_pActor->GetSkillFromIndex( i );
		if( !hSkill ) continue;
		CDnSkill* pSkill = hSkill.GetPointer();
		if( pSkill->GetSkillType() != CDnSkill::Active ) continue;
		if( pSkill->CanExecute() != CDnSkill::UsingResult::Success ) continue;

		const SkillProfile& prof = GetOrAnalyzeSkillProfile( pSkill );
		if( !prof.bHasIFrame ) continue;

		if( m_eMoveState == MoveState_Moving )
			SendStopPacket();

		if( EtVec3LengthSq( &vEscapeDir ) > 0.01f )
		{
			EtVector3 vLookPt = *m_pActor->GetPosition() + vEscapeDir * 200.f;
			FacePosition( vLookPt );
		}

		if( m_pActor->UseSkill( pSkill->GetClassID(), true ) == CDnSkill::UsingResult::Success )
		{
			SendUseSkillPacket( pSkill );
			m_fSkillDelayTimer = 0.32f;
			return true;
		}
	}
	return false;
}

bool CDnAIBotController::TryJumpStomp()
{
	if( !m_pActor ) return false;
	const char* szJump = ( m_pActor->GetElementIndex( "Jump" ) != -1 ) ? "Jump" : NULL;
	if( !szJump ) return false;

	if( m_eMoveState == MoveState_Moving )
		SendStopPacket();

	m_pActor->CmdAction( szJump, 0, 2.f );
	SendActionPacket( szJump, 0, 2.f, false );
	return true;
}

bool CDnAIBotController::TryDirectionalTumble( const EtVector3& vEscapeDir, const EtVector3* pFaceTarget )
{
	if( !m_pActor ) return false;

	EtVector2 vEsc2 = Normalize2( EtVector2( vEscapeDir.x, vEscapeDir.z ) );
	EtVector2 vFace2 = pFaceTarget ? SafeDir2( *m_pActor->GetPosition(), *pFaceTarget ) : vEsc2;
	EtVector2 vRight2( vFace2.y, -vFace2.x );

	float fFwd = vEsc2.x * vFace2.x + vEsc2.y * vFace2.y;
	float fSide = vEsc2.x * vRight2.x + vEsc2.y * vRight2.y;

	const char* szTumble = "Tumble_Front";
	EtVector2 vZVec = vEsc2;
	EtVector2 vLook = vFace2;

	if( pFaceTarget )
	{
		if( fFwd < -0.55f && m_pActor->GetElementIndex( "Tumble_Back" ) != -1 )
		{
			szTumble = "Tumble_Back";
			vZVec = EtVector2( -vEsc2.x, -vEsc2.y );
		}
		else if( fFwd > 0.55f && m_pActor->GetElementIndex( "Tumble_Front" ) != -1 )
		{
			szTumble = "Tumble_Front";
			vZVec = vEsc2;
		}
		else if( fSide >= 0.f && m_pActor->GetElementIndex( "Tumble_Right" ) != -1 )
		{
			szTumble = "Tumble_Right";
			vZVec = EtVector2( -vEsc2.y, vEsc2.x );
		}
		else if( m_pActor->GetElementIndex( "Tumble_Left" ) != -1 )
		{
			szTumble = "Tumble_Left";
			vZVec = EtVector2( vEsc2.y, -vEsc2.x );
		}
		else
		{
			szTumble = "Tumble_Back";
			vZVec = EtVector2( -vEsc2.x, -vEsc2.y );
		}
	}
	else
	{
		vZVec = vEsc2;
		vLook = vEsc2;
		if( m_pActor->GetElementIndex( "Tumble_Front" ) == -1 )
			szTumble = "Tumble_Back";
	}

	if( m_pActor->GetElementIndex( szTumble ) == -1 )
		return false;

	if( m_eMoveState == MoveState_Moving )
		SendStopPacket();

	EtVector3 vXVec;
	EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );
	m_pActor->SetMoveVectorX( vXVec );
	m_pActor->SetMoveVectorZ( EtVec2toVec3( vZVec ) );
	m_pActor->Look( vLook, true );

	m_pActor->CmdAction( szTumble, 0, 3.f );
	SendActionPacket( szTumble, 0, 3.f, false );
	return true;
}

void CDnAIBotController::MaybeSendTacticalCallout( const WCHAR* wszMsg, float fRoomCooldownSec )
{
	if( !wszMsg || !wszMsg[0] || !m_pActor || !m_pActor->GetGameRoom() )
		return;

	CDNGameRoom* pRoom = m_pActor->GetGameRoom();
	DWORD dwNow = timeGetTime();
	DWORD dwCooldownMs = (DWORD)( fRoomCooldownSec * 1000.f );
	if( !CTacticalLearningMemory::Get().TryClaimRoomCalloutSlot( pRoom->GetRoomID(), dwNow, dwCooldownMs ) )
		return;

	int nLen = (int)wcslen( wszMsg );
	const WCHAR* pBotName = m_pActor->GetName();
	for( UINT i = 0; i < pRoom->GetUserCount(); ++i )
	{
		CDNUserSession* pSess = pRoom->GetUserData( i );
		if( pSess && !pSess->bIsAIBot() )
			pSess->SendChat( CHATTYPE_PARTY, nLen, pBotName, wszMsg );
	}
}

void CDnAIBotController::UpdateMovementAndPositioning( float fDelta, bool& bOutWantsToMove )
{
	EtVector3 vMyPos = *m_pActor->GetPosition();
	EtVector3 vDesiredPos = vMyPos;
	float fStopDist = 110.f;
	bool bHasGoal = false;
	bool bAllowCombatStrafe = false;
	EtVector3 vFaceTarget = vMyPos;
	const EtVector3* pCombatFace = NULL;

	// Priority 0: Human leader entered a stage portal - sprint directly inside the portal box to flag ready!
	if( m_LastScan.bHumanInGate && m_LastScan.nActiveGateIndex != -1 )
	{
		if( m_nSyncedGateIndex == m_LastScan.nActiveGateIndex )
		{
			bOutWantsToMove = false;
			if( m_eMoveState == MoveState_Moving )
				SendStopPacket();
			return;
		}
		vDesiredPos = m_LastScan.vGateTargetPos;
		fStopDist = 35.f;
		bHasGoal = true;
	}

	// Priority 0a: Active Boss Mechanic / Room-Wipe Threat positioning!
	if( !bHasGoal && m_LastScan.BossThreat.bBotCurrentlyInDanger )
	{
		BossThreatInfo& T = m_LastScan.BossThreat;
		if( T.eKind == Threat_RoomWipe && m_LastScan.hSafeBuffProp && m_LastScan.hSafeBuffProp->IsShow() )
		{
			vDesiredPos = m_LastScan.hSafeBuffProp->GetMatEx()->m_vPosition;
			fStopDist = 55.f;
			bHasGoal = true;
		}
		else if( T.eKind == Threat_DonutOuter )
		{
			// Rush inside the safe inner ring next to boss!
			EtVector3 vDirIn = vMyPos - T.vBossPos;
			vDirIn.y = 0.f;
			if( EtVec3LengthSq( &vDirIn ) < 1.f ) vDirIn = EtVector3( 1.f, 0.f, 0.f );
			EtVec3Normalize( &vDirIn, &vDirIn );
			float fInnerTarget = ( T.fSafeMinDist > 90.f ) ? ( T.fSafeMinDist * 0.55f ) : 75.f;
			vDesiredPos = T.vBossPos + vDirIn * fInnerTarget;
			fStopDist = 40.f;
			bHasGoal = true;
		}
		else if( T.eKind == Threat_FrontalCleave || T.eKind == Threat_Projectile )
		{
			// Circle quickly to the boss's flank / rear!
			EtVector3 vFlank = ( -T.vBossForward * 0.65f ) + EtVector3( -T.vBossForward.z * m_fStrafeSign * 0.75f, 0.f, T.vBossForward.x * m_fStrafeSign * 0.75f );
			EtVec3Normalize( &vFlank, &vFlank );
			float fRad = T.hBoss ? GetActorEffectiveRadius( T.hBoss.GetPointer() ) : 80.f;
			vDesiredPos = T.vBossPos + vFlank * ( fRad + 160.f );
			fStopDist = 45.f;
			bHasGoal = true;
			bAllowCombatStrafe = true;
			vFaceTarget = T.vBossPos;
			pCombatFace = &vFaceTarget;
		}
		else if( T.eKind == Threat_PointBlankAoE )
		{
			EtVector3 vAway = vMyPos - T.vHitOrigin;
			vAway.y = 0.f;
			if( EtVec3LengthSq( &vAway ) < 1.f ) vAway = -T.vBossForward;
			EtVec3Normalize( &vAway, &vAway );
			vDesiredPos = T.vHitOrigin + vAway * ( T.fSafeMaxDist + 40.f );
			fStopDist = 45.f;
			bHasGoal = true;
		}
	}

	// Priority 0b: A destructible prop is directly blocking our path - step right up to smash it open
	if( !bHasGoal && ( m_hPathBlockingBreakable || m_LastScan.hBlockingBreakableProp ) )
	{
		DnPropHandle hBlocker = m_hPathBlockingBreakable ? m_hPathBlockingBreakable : m_LastScan.hBlockingBreakableProp;
		if( hBlocker && hBlocker->IsShow() && hBlocker->IsBrokenType() )
		{
			CDnWorldBrokenProp* pBroken = static_cast<CDnWorldBrokenProp*>( hBlocker.GetPointer() );
			if( !pBroken->IsBroken() )
			{
				vDesiredPos = hBlocker->GetMatEx()->m_vPosition;
				fStopDist = HasRangedBasicAttack() ? 290.f : 120.f;
				bHasGoal = true;
			}
			else if( hBlocker == m_hPathBlockingBreakable )
			{
				m_hPathBlockingBreakable.Identity();
			}
		}
		else if( hBlocker == m_hPathBlockingBreakable )
		{
			m_hPathBlockingBreakable.Identity();
		}
	}

	// Priority 1: Healer moves toward a wounded ally out of heal range
	float fHealMoveThresh = ( m_eEncounterMode == Mode_Nest || m_eEncounterMode == Mode_DragonNest ) ? 0.80f : 0.72f;
	if( !bHasGoal && m_Role == AIBot::RoleType::Healer && m_LastScan.hLowestHpAlly && !m_LastScan.hLowestHpAlly->IsDie() && m_LastScan.fLowestHpRatio < fHealMoveThresh )
	{
		float fAllyDist = XZDist( vMyPos, *m_LastScan.hLowestHpAlly->GetPosition() );
		if( fAllyDist > 480.f )
		{
			vDesiredPos = *m_LastScan.hLowestHpAlly->GetPosition();
			fStopDist = 340.f;
			bHasGoal = true;
		}
	}

	// Priority 2: Safe-zone buff prop during Nest/Boss invulnerability or cast phases
	if( !bHasGoal && m_LastScan.hSafeBuffProp && m_LastScan.hSafeBuffProp->IsShow() && m_LastScan.hPriorityBoss && !m_LastScan.hPriorityBoss->IsDie() )
	{
		CDnMonsterActor* pBoss = static_cast<CDnMonsterActor*>( m_LastScan.hPriorityBoss.GetPointer() );
		if( pBoss->IsProcessSkill() || m_LastScan.bBossInvulnerable )
		{
			vDesiredPos = m_LastScan.hSafeBuffProp->GetMatEx()->m_vPosition;
			fStopDist = 60.f;
			bHasGoal = true;
		}
	}

	// Priority 2b: Invulnerable boss + active breakable mechanic prop (totem/crystal) -> go break it!
	if( !bHasGoal && m_LastScan.bBossInvulnerable && m_LastScan.hBreakableProp && m_LastScan.hBreakableProp->IsShow() )
	{
		vDesiredPos = m_LastScan.hBreakableProp->GetMatEx()->m_vPosition;
		fStopDist = HasRangedBasicAttack() ? 310.f : 125.f;
		bHasGoal = true;
	}

	// Priority 3: Tactical Human-Like Combat Positioning (matched to actual READY skill distances + rear flanking + strafe!)
	if( !bHasGoal && m_hTargetActor && !m_hTargetActor->IsDie() )
	{
		EtVector3 vTargetPos = *m_hTargetActor->GetPosition();
		bool bRangedBasic = HasRangedBasicAttack();
		float fMyRadius = GetActorEffectiveRadius( m_pActor );
		float fTargetRadius = GetActorEffectiveRadius( m_hTargetActor.GetPointer() );
		float fEngageRange = GetDesiredCombatRange( fTargetRadius, fMyRadius );
		float fCurCenterDist = XZDist( vMyPos, vTargetPos );
		float fCurDistToBox = ( fCurCenterDist > fTargetRadius ) ? ( fCurCenterDist - fTargetRadius ) : 0.f;

		vFaceTarget = vTargetPos;
		pCombatFace = &vFaceTarget;
		bAllowCombatStrafe = ( fCurDistToBox < 620.f );

		// If ranged bot's line of sight to the enemy is blocked by a pillar/wall/prop, sidestep / close in around it!
		if( bRangedBasic && !m_LastScan.bTargetHasLOS )
		{
			vDesiredPos = vTargetPos;
			fStopDist = fTargetRadius + fMyRadius + 135.f;
			bAllowCombatStrafe = false;
		}
		else if( m_Role == AIBot::RoleType::Tank )
		{
			// Tank stands on the opposite side of the boss from the Human Player so cleaves & breaths face AWAY from party!
			float fStandDist = fTargetRadius + fMyRadius + fEngageRange;
			EtVector3 vOffset( 0.f, 0.f, fStandDist );
			if( m_LastScan.hHumanLeader && !m_LastScan.hHumanLeader->IsDie() )
			{
				EtVector3 vFromHuman = vTargetPos - *m_LastScan.hHumanLeader->GetPosition();
				vFromHuman.y = 0.f;
				if( EtVec3LengthSq( &vFromHuman ) > 10.f )
				{
					EtVec3Normalize( &vFromHuman, &vFromHuman );
					vOffset = vFromHuman * fStandDist;
				}
			}
			vDesiredPos = vTargetPos + vOffset;
			fStopDist = 52.f;
		}
		else
		{
			// DPS / Healer / IceStacker / Support: flank behind boss's rear arc (110..250 deg) with organic slow orbit!
			float fBaseAngleDeg = 125.f + (float)( ( m_nBotSlotIndex * 37 ) % 110 );
			float fOrbitOffsetDeg = m_fStrafeSign * 18.f;
			float fAngle = EtToRadian( fBaseAngleDeg + fOrbitOffsetDeg );

			EtVector3 vTargetForward = m_hTargetActor->GetMatEx()->m_vZAxis;
			vTargetForward.y = 0.f;
			if( EtVec3LengthSq( &vTargetForward ) < 0.01f )
				vTargetForward = EtVector3( 0.f, 0.f, 1.f );
			EtVec3Normalize( &vTargetForward, &vTargetForward );

			float c = cosf( fAngle );
			float s = sinf( fAngle );
			EtVector3 vFlankDir( vTargetForward.x * c - vTargetForward.z * s, 0.f, vTargetForward.x * s + vTargetForward.z * c );

			float fDesiredCenterDist = fTargetRadius + fEngageRange;
			vDesiredPos = vTargetPos + vFlankDir * fDesiredCenterDist;

			// If we are already comfortably inside effective combat range AND not standing right in front of a Boss,
			// don't needlessly twitch-walk back and forth; hold crisp human attack stance!
			bool bInFrontOfBoss = false;
			if( m_hTargetActor == m_LastScan.hPriorityBoss )
			{
				EtVector2 vToMe = SafeDir2( vTargetPos, vMyPos );
				bInFrontOfBoss = ( ComputeAngleDiffDeg( EtVector2( vTargetForward.x, vTargetForward.z ), vToMe ) < 62.f );
			}

			if( !bInFrontOfBoss && fCurDistToBox <= fEngageRange + 45.f && ( !bRangedBasic || fCurDistToBox >= fEngageRange * 0.42f ) )
			{
				fStopDist = fDesiredCenterDist + 80.f; // stay planted & cast smoothly
			}
			else
			{
				fStopDist = bRangedBasic ? 85.f : 48.f;
			}
		}

		// Human-like Gap-Close Tumble when closing a large distance (> 520) to an enemy in open ground
		if( fCurDistToBox > 540.f && m_fDashGapCooldown <= 0.f && m_fDodgeCooldown <= 0.f && !m_bBlockedByObstacle )
		{
			EtVector3 vToTarget = vTargetPos - vMyPos;
			vToTarget.y = 0.f;
			if( EtVec3LengthSq( &vToTarget ) > 1.f )
			{
				EtVec3Normalize( &vToTarget, &vToTarget );
				PathObstacle gapObs;
				if( !ProbePathObstacle( vMyPos, EtVector2( vToTarget.x, vToTarget.z ), 230.f, false, m_pActor->GetLocalTime(), gapObs ) )
				{
					if( TryDirectionalTumble( vToTarget ) )
					{
						m_fDashGapCooldown = 5.5f;
						m_fDodgeCooldown = 2.0f;
						bOutWantsToMove = false;
						return;
					}
				}
			}
		}

		bHasGoal = true;
	}

	// Priority 4: Proactively walk into an active portal when human leader is approaching it out of combat
	if( !bHasGoal && m_LastScan.bHumanApproachingGate && m_LastScan.nActiveGateIndex != -1 )
	{
		vDesiredPos = m_LastScan.vGateTargetPos;
		fStopDist = 45.f;
		bHasGoal = true;
	}

	// Priority 5: Destructible dungeon prop when no active monsters
	if( !bHasGoal && m_LastScan.hBreakableProp && m_LastScan.hBreakableProp->IsShow() )
	{
		vDesiredPos = m_LastScan.hBreakableProp->GetMatEx()->m_vPosition;
		fStopDist = HasRangedBasicAttack() ? 300.f : 120.f;
		bHasGoal = true;
	}

	// Priority 6: Natural out-of-combat formation around Human Party Leader (plus occasional sprint-tumble if far behind!)
	if( !bHasGoal && m_LastScan.hHumanLeader && !m_LastScan.hHumanLeader->IsDie() )
	{
		EtVector3 vLeaderPos = *m_LastScan.hHumanLeader->GetPosition();
		float fLeaderDist = XZDist( vMyPos, vLeaderPos );
		if( fLeaderDist > 560.f && m_fDashGapCooldown <= 0.f && m_fDodgeCooldown <= 0.f && !m_bBlockedByObstacle )
		{
			EtVector3 vToLead = vLeaderPos - vMyPos;
			vToLead.y = 0.f;
			if( EtVec3LengthSq( &vToLead ) > 1.f )
			{
				EtVec3Normalize( &vToLead, &vToLead );
				PathObstacle gapObs;
				if( !ProbePathObstacle( vMyPos, EtVector2( vToLead.x, vToLead.z ), 230.f, false, m_pActor->GetLocalTime(), gapObs ) )
				{
					if( TryDirectionalTumble( vToLead ) )
					{
						m_fDashGapCooldown = 4.5f;
						m_fDodgeCooldown = 2.0f;
						bOutWantsToMove = false;
						return;
					}
				}
			}
		}

		EtVector3 vBack = -m_LastScan.hHumanLeader->GetMatEx()->m_vZAxis;
		vBack.y = 0.f;
		if( EtVec3LengthSq( &vBack ) < 0.01f )
			vBack = EtVector3( 0.f, 0.f, -1.f );
		EtVec3Normalize( &vBack, &vBack );

		float fSpreadDeg = -60.f + (float)( ( m_nBotSlotIndex % 7 ) * 20.f );
		float fRad = EtToRadian( fSpreadDeg );
		float c = cosf( fRad );
		float s = sinf( fRad );
		EtVector3 vOffsetDir( vBack.x * c - vBack.z * s, 0.f, vBack.x * s + vBack.z * c );

		float fRing = 90.f + (float)( ( m_nBotSlotIndex % 2 ) * 40.f );
		vDesiredPos = vLeaderPos + vOffsetDir * fRing;
		fStopDist = 60.f;
		bHasGoal = true;
	}

	if( bHasGoal )
		SteerToward( vDesiredPos, fStopDist, fDelta, bOutWantsToMove, bAllowCombatStrafe, pCombatFace );
	else if( m_eMoveState == MoveState_Moving )
		SendStopPacket();
}

void CDnAIBotController::SteerToward( const EtVector3& vDest, float fStopDist, float fDelta, bool& bOutWantsToMove, bool bAllowCombatStrafe, const EtVector3* pCombatFaceTarget )
{
	EtVector3 vMyPos = *m_pActor->GetPosition();
	float fDirectDist = XZDist( vMyPos, vDest );

	if( fDirectDist <= fStopDist )
	{
		bOutWantsToMove = false;
		m_vecWaypoints.clear();
		m_hPathBlockingBreakable.Identity();
		m_fAvoidHoldTimer = 0.f;
		if( m_eMoveState == MoveState_Moving )
			SendStopPacket();
		// When stopped in combat, naturally keep looking toward our enemy!
		if( pCombatFaceTarget )
		{
			EtVector2 vLook = SafeDir2( vMyPos, *pCombatFaceTarget );
			if( ComputeAngleDiffDeg( m_vLastLookDir, vLook ) > 18.f )
			{
				FacePosition( *pCombatFaceTarget );
				m_vLastLookDir = vLook;
			}
		}
		return;
	}

	bOutWantsToMove = true;

	if( m_pActor->GetCantMoveSEReferenceCount() > 0 )
	{
		if( m_eMoveState == MoveState_Moving )
			SendStopPacket();
		return;
	}

	// Build or refresh NavigationMesh path
	float fGoalDrift = XZDist( vDest, m_vLastPathGoal );
	if( m_vecWaypoints.empty() || m_fRepathTimer <= 0.f || fGoalDrift > 95.f )
	{
		m_bHasValidPath = BuildPath( vMyPos, vDest );
		m_vLastPathGoal = vDest;
		m_fRepathTimer = m_bBlockedByObstacle ? 0.24f : 0.40f;
	}

	EtVector3 vStepTarget = vDest;
	while( m_nCurWaypoint < m_vecWaypoints.size() )
	{
		float fWpDist = XZDist( vMyPos, m_vecWaypoints[m_nCurWaypoint] );
		if( fWpDist < 65.f && ( m_nCurWaypoint + 1 ) < m_vecWaypoints.size() )
			++m_nCurWaypoint;
		else
		{
			vStepTarget = m_vecWaypoints[m_nCurWaypoint];
			break;
		}
	}

	EtVector2 vDesiredDir = SafeDir2( vMyPos, vStepTarget );
	EtVector2 vDir2 = vDesiredDir;
	float fStepDist = XZDist( vMyPos, vStepTarget );
	float fProbeDist = ( fStepDist < 140.f ) ? fStepDist : 140.f;
	if( fProbeDist < 65.f ) fProbeDist = 65.f;

	// Proactive + reactive object/wall/nav-edge awareness
	if( m_bBlockedByObstacle || m_fObstacleProbeTimer <= 0.f )
	{
		m_fObstacleProbeTimer = 0.14f;
		m_hPathBlockingBreakable.Identity();

		PathObstacle frontObs;
		EtVector2 vClearDir = vDesiredDir;
		bool bFoundClear = ChooseClearSteeringDir( vMyPos, vDesiredDir, fProbeDist, m_bBlockedByObstacle, m_pActor->GetLocalTime(), vClearDir, frontObs );

		if( frontObs.eType == Obstacle_BreakableProp && frontObs.hProp )
		{
			m_hPathBlockingBreakable = frontObs.hProp;
			if( frontObs.fHitDist <= ( HasRangedBasicAttack() ? 300.f : 140.f ) )
			{
				bOutWantsToMove = false;
				if( m_eMoveState == MoveState_Moving )
					SendStopPacket();
				return;
			}
		}
		else if( frontObs.eType != Obstacle_None )
		{
			if( bFoundClear )
			{
				vDir2 = vClearDir;
				m_vAvoidDir = vClearDir;
				m_fAvoidHoldTimer = m_bBlockedByObstacle ? 0.55f : 0.35f;
			}
			else
			{
				m_fAvoidSideSign = -m_fAvoidSideSign;
				vDir2 = EtVector2( -vDesiredDir.y * m_fAvoidSideSign, vDesiredDir.x * m_fAvoidSideSign );
				m_vAvoidDir = vDir2;
				m_fAvoidHoldTimer = 0.55f;
			}
		}
		else if( m_fAvoidHoldTimer <= 0.f )
		{
			m_bBlockedByObstacle = false;
		}
	}
	else if( m_fAvoidHoldTimer > 0.f )
	{
		vDir2 = m_vAvoidDir;
	}

	// Human-like Directional Combat Movement:
	// In BattleMode near an enemy, strafe (Move_Left/Move_Right) or kite (Move_Back) while keeping gaze on the enemy!
	const char* szMoveAction = "Move_Front";
	char cMoveFlag = 0x04;
	EtVector2 vZVec = vDir2;
	EtVector2 vLook = vDir2;

	if( bAllowCombatStrafe && pCombatFaceTarget && m_pActor->IsBattleMode() && fDirectDist < 420.f )
	{
		EtVector2 vFace2 = SafeDir2( vMyPos, *pCombatFaceTarget );
		EtVector2 vRight2( vFace2.y, -vFace2.x );
		float fFwd = vDir2.x * vFace2.x + vDir2.y * vFace2.y;
		float fSide = vDir2.x * vRight2.x + vDir2.y * vRight2.y;

		if( fFwd < -0.58f && m_pActor->GetElementIndex( "Move_Back" ) != -1 )
		{
			szMoveAction = "Move_Back";
			cMoveFlag = 0x08;
			vZVec = EtVector2( -vDir2.x, -vDir2.y );
			vLook = vFace2;
		}
		else if( fFwd >= -0.58f && fFwd <= 0.58f )
		{
			if( fSide >= 0.f && m_pActor->GetElementIndex( "Move_Right" ) != -1 )
			{
				szMoveAction = "Move_Right";
				cMoveFlag = 0x02;
				vZVec = EtVector2( -vDir2.y, vDir2.x );
				vLook = vFace2;
			}
			else if( fSide < 0.f && m_pActor->GetElementIndex( "Move_Left" ) != -1 )
			{
				szMoveAction = "Move_Left";
				cMoveFlag = 0x01;
				vZVec = EtVector2( vDir2.y, -vDir2.x );
				vLook = vFace2;
			}
		}
		else
		{
			szMoveAction = "Move_Front";
			cMoveFlag = 0x04;
			vZVec = vDir2;
			vLook = vFace2;
		}
	}

	EtVector3 vXVec;
	EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );
	m_pActor->SetMoveVectorX( vXVec );
	m_pActor->SetMoveVectorZ( EtVec2toVec3( vZVec ) );
	m_pActor->Look( vLook, false );

	EtVector3 vDir3 = EtVec2toVec3( vDir2 );
	EtVector3 vFarTarget = vMyPos + ( vDir3 * 100000.f );
	bool bNeedMovePacket = ( m_eMoveState != MoveState_Moving ) ||
		( m_fMoveSyncTimer <= 0.f ) ||
		( cMoveFlag != m_cLastMoveFlag ) ||
		( m_strLastMoveAction != szMoveAction );

	if( !bNeedMovePacket )
	{
		float fDot = EtVec3Dot( &m_vLastMoveDir, &vDir3 );
		if( fDot < 0.965f || ComputeAngleDiffDeg( m_vLastLookDir, vLook ) > 18.f )
			bNeedMovePacket = true;
	}

	if( bNeedMovePacket || !m_pActor->IsMove() )
	{
		m_pActor->CmdMove( vFarTarget, szMoveAction, -1, 8.f );
		SendDirectionalMovePacket( szMoveAction, cMoveFlag, vZVec, vLook );
		m_vLastMoveDir = vDir3;
		m_vLastLookDir = vLook;
		m_cLastMoveFlag = cMoveFlag;
		m_strLastMoveAction = szMoveAction;
		m_fMoveSyncTimer = 0.20f;
		m_eMoveState = MoveState_Moving;
	}
}

bool CDnAIBotController::BuildPath( const EtVector3& vStart, const EtVector3& vGoal )
{
	m_vecWaypoints.clear();
	m_nCurWaypoint = 0;

	CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
	if( !pWorld || !pWorld->GetGrid() )
	{
		m_vecWaypoints.push_back( vGoal );
		return false;
	}

	EtVector3 vS = vStart;
	EtVector3 vG = vGoal;
	NavigationMesh* pNavMesh = pWorld->GetGrid()->GetNavMesh( vS );
	if( !pNavMesh || pNavMesh->TotalCells() <= 0 )
	{
		m_vecWaypoints.push_back( vGoal );
		return false;
	}

	NavigationCell* pStartCell = pNavMesh->FindCell( vS );
	if( !pStartCell ) pStartCell = pNavMesh->FindClosestCell( vS );
	NavigationCell* pEndCell = pNavMesh->FindCell( vG );
	if( !pEndCell ) pEndCell = pNavMesh->FindClosestCell( vG );
	if( !pStartCell || !pEndCell )
	{
		m_vecWaypoints.push_back( vGoal );
		return false;
	}

	vS = pNavMesh->FittingPoint( pStartCell, vS );
	vG = pNavMesh->FittingPoint( pEndCell, vG );

	if( pStartCell == pEndCell || pNavMesh->LineOfSightTest( pStartCell, vS, pEndCell, vG ) )
	{
		PathObstacle directObs;
		if( !CheckSegmentObstacle( vS, vG, 24.f, false, m_pActor->GetLocalTime(), directObs ) )
		{
			m_vecWaypoints.push_back( vGoal );
			return true;
		}
	}

	NavigationPath naviPath;
	bool bFound = pNavMesh->BuildNavigationPath( naviPath, pStartCell, vS, pEndCell, vG );
	if( !bFound )
	{
		m_vecWaypoints.push_back( vGoal );
		return false;
	}

	WAYPOINT_LIST& wpList = naviPath.LineOfSightWayPointList();
	if( wpList.empty() )
	{
		m_vecWaypoints.push_back( vGoal );
		return true;
	}

	WAYPOINT_LIST::iterator it = wpList.begin();
	if( it != wpList.end() )
		++it;
	for( ; it != wpList.end(); ++it )
		m_vecWaypoints.push_back( it->Position );

	if( m_vecWaypoints.empty() )
		m_vecWaypoints.push_back( vGoal );
	return true;
}

float CDnAIBotController::GetActorEffectiveRadius( CDnActor* pActor )
{
	if( !pActor ) return 35.f;
	float fRadius = (float)pActor->GetUnitSize();
	SAABox Box;
	Box.Reset();
	pActor->GetBoundingBox( Box );
	float fHalfX = ( Box.Max.x - Box.Min.x ) * 0.5f;
	float fHalfZ = ( Box.Max.z - Box.Min.z ) * 0.5f;
	float fBoxRadius = ( fHalfX > fHalfZ ) ? fHalfX : fHalfZ;
	if( fBoxRadius > fRadius && fBoxRadius < 420.f )
		fRadius = ( fRadius > 0.f ) ? ( fRadius * 0.5f + fBoxRadius * 0.5f ) : fBoxRadius;
	if( fRadius < 25.f ) fRadius = 25.f;
	if( fRadius > 380.f ) fRadius = 380.f;
	return fRadius;
}

EtVector3 CDnAIBotController::ComputeTargetAimPoint( CDnActor* pTarget, bool bLeadVelocity ) const
{
	if( !pTarget )
		return m_vAimPos;

	EtVector3 vPos = *pTarget->GetPosition();
	SAABox Box;
	Box.Reset();
	pTarget->GetBoundingBox( Box );
	float fHeight = Box.Max.y - Box.Min.y;
	if( fHeight > 15.f && fHeight < 900.f )
		vPos.y = Box.Min.y + fHeight * 0.55f;
	else
		vPos.y += 70.f;

	if( bLeadVelocity && m_pActor && pTarget->GetMovement() )
	{
		EtVector3* pVel = pTarget->GetMovement()->GetVelocity();
		if( pVel )
		{
			float fDist = XZDist( *m_pActor->GetPosition(), vPos );
			float fLeadSec = fDist / 1300.f;
			if( fLeadSec > 0.22f ) fLeadSec = 0.22f;
			if( fLeadSec > 0.02f )
			{
				vPos.x += pVel->x * fLeadSec;
				vPos.z += pVel->z * fLeadSec;
			}
		}
	}
	return vPos;
}

EtVector3 CDnAIBotController::ComputeGateStandPoint( CEtWorldEventArea* pGateArea ) const
{
	if( !pGateArea )
		return *m_pActor->GetPosition();

	SOBB* pObb = pGateArea->GetOBB();
	EtVector3 vCenter = pObb->Center;

	float fExtX = pObb->Extent[0] * 0.45f;
	float fExtZ = pObb->Extent[2] * 0.45f;
	if( fExtX > 65.f ) fExtX = 65.f;
	if( fExtZ > 65.f ) fExtZ = 65.f;

	float fAngle = EtToRadian( (float)( ( m_nBotSlotIndex * 73 ) % 360 ) );
	EtVector3 vPt = vCenter
		+ pObb->Axis[0] * ( cosf( fAngle ) * fExtX )
		+ pObb->Axis[2] * ( sinf( fAngle ) * fExtZ );

	if( !pGateArea->CheckArea( vPt, true ) )
		vPt = vCenter;

	CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
	if( pWorld )
		vPt.y = pWorld->GetHeightWithProp( vPt );
	return vPt;
}

void CDnAIBotController::UpdateGateReadiness( float fDelta )
{
	if( !m_pActor || !m_pActor->GetUserSession() || !CDnPartyTask::IsActive( m_pActor->GetRoom() ) )
		return;

	CDnPartyTask& PartyTask = CDnPartyTask::GetInstance( m_pActor->GetRoom() );
	CDNUserSession* pSess = m_pActor->GetUserSession();

	if( m_LastScan.bHumanInGate && m_LastScan.nActiveGateIndex != -1 )
	{
		m_fGateWaitTimer += fDelta;
		CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
		GateStruct* pGate = pWorld ? pWorld->GetGateStruct( (char)m_LastScan.nActiveGateIndex ) : NULL;
		if( pGate && pGate->pGateArea && pGate->pGateArea->IsActive() )
		{
			EtVector3 vCurPos = *m_pActor->GetPosition();
			if( pGate->pGateArea->CheckArea( vCurPos, true ) )
			{
				if( m_nSyncedGateIndex != m_LastScan.nActiveGateIndex )
				{
					if( PartyTask.SyncAIBotGateReady( pSess, m_LastScan.nActiveGateIndex ) )
						m_nSyncedGateIndex = m_LastScan.nActiveGateIndex;
				}
			}
		}
	}
	else
	{
		m_fGateWaitTimer = 0.f;
		if( m_nSyncedGateIndex != -1 )
		{
			PartyTask.SyncAIBotGateReady( pSess, -1 );
			m_nSyncedGateIndex = -1;
		}
	}
}

void CDnAIBotController::PerformPortalTeleport( const EtVector3& vGatePos )
{
	if( m_eMoveState == MoveState_Moving )
		SendStopPacket();

	EtVector3 vDest = vGatePos;
	CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
	if( pWorld )
		vDest.y = pWorld->GetHeightWithProp( vDest );

	m_pActor->SetPosition( vDest );
	m_pActor->SetStaticPosition( vDest );
	m_vecWaypoints.clear();
	m_fStuckAccumTime = 0.f;
	m_bBlockedByObstacle = false;
	m_hPathBlockingBreakable.Identity();
	m_vStuckCheckPos = vDest;
	SendStopPacket( true );
}

bool CDnAIBotController::HasNavMeshConnectivity( const EtVector3& vStart, const EtVector3& vEnd, int& nAStarBudget ) const
{
	CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
	if( !pWorld || !pWorld->GetGrid() )
		return true;

	EtVector3 vStartCopy = vStart;
	EtVector3 vEndCopy = vEnd;
	NavigationMesh* pStartNav = pWorld->GetGrid()->GetNavMesh( vStartCopy );
	NavigationMesh* pEndNav = pWorld->GetGrid()->GetNavMesh( vEndCopy );
	if( !pStartNav || pStartNav->TotalCells() <= 0 )
		return true;
	if( pEndNav && pEndNav != pStartNav )
		return true;

	NavigationCell* pStartCell = pStartNav->FindCell( vStart );
	if( !pStartCell ) pStartCell = pStartNav->FindClosestCell( vStart );
	NavigationCell* pEndCell = pStartNav->FindCell( vEnd );
	if( !pEndCell ) pEndCell = pStartNav->FindClosestCell( vEnd );
	if( !pStartCell || !pEndCell )
		return false;

	EtVector3 vS = pStartNav->FittingPoint( pStartCell, vStart );
	EtVector3 vE = pStartNav->FittingPoint( pEndCell, vEnd );
	if( pStartCell == pEndCell || pStartNav->LineOfSightTest( pStartCell, vS, pEndCell, vE ) )
		return true;

	if( nAStarBudget <= 0 )
		return true;
	--nAStarBudget;

	NavigationPath tempPath;
	return pStartNav->BuildNavigationPath( tempPath, pStartCell, vS, pEndCell, vE );
}

bool CDnAIBotController::CheckSegmentObstacle( const EtVector3& vFrom, const EtVector3& vTo, float fExpandRadius, bool bForProjectileLOS, LOCAL_TIME LocalTime, PathObstacle& outObs ) const
{
	outObs = PathObstacle();
	CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
	if( !pWorld )
		return false;

	EtVector3 vDelta = vTo - vFrom;
	float fLen = XZDist( vFrom, vTo );
	if( fLen < 12.f )
		return false;

	EtVector3 vMid = ( vFrom + vTo ) * 0.5f;
	float fScanRadius = fLen * 0.5f + 160.f;
	if( fScanRadius > 1400.f ) fScanRadius = 1400.f;

	DNVector(CEtWorldProp*) vecProps;
	pWorld->ScanProp( vMid, fScanRadius, vecProps );
	if( vecProps.empty() )
		return false;

	SSegment seg;
	seg.vOrigin = vFrom;
	seg.vOrigin.y += bForProjectileLOS ? 75.f : 62.f;
	seg.vDirection = EtVector3( vTo.x - vFrom.x, ( vTo.y + ( bForProjectileLOS ? 65.f : 62.f ) ) - seg.vOrigin.y, vTo.z - vFrom.z );

	SCollisionCapsule capsule;
	capsule.Segment = seg;
	capsule.fRadius = fExpandRadius;

	bool bHitAny = false;
	for( size_t i = 0; i < vecProps.size(); ++i )
	{
		CDnWorldProp* pProp = static_cast<CDnWorldProp*>( vecProps[i] );
		if( !pProp || !pProp->IsShow() ) continue;

		PropTypeEnum eType = pProp->GetPropType();
		if( eType == PTE_Buff || eType == PTE_BuffBroken || eType == PTE_Light || eType == PTE_Camera )
			continue;

		if( !bForProjectileLOS && eType == PTE_Trap )
		{
			EtVector3 vTrapPos = pProp->GetMatEx()->m_vPosition;
			EtVector2 vSegXZ( vDelta.x, vDelta.z );
			float fSegLenSq = vSegXZ.x * vSegXZ.x + vSegXZ.y * vSegXZ.y;
			if( fSegLenSq > 1.f )
			{
				float t = ( ( vTrapPos.x - vFrom.x ) * vSegXZ.x + ( vTrapPos.z - vFrom.z ) * vSegXZ.y ) / fSegLenSq;
				if( t >= 0.f && t <= 1.f )
				{
					EtVector3 vClosest( vFrom.x + vSegXZ.x * t, vFrom.y, vFrom.z + vSegXZ.y * t );
					float fTrapDist = XZDist( vClosest, vTrapPos );
					if( fTrapDist < 175.f )
					{
						float fAlong = t * fLen;
						if( fAlong < outObs.fHitDist )
						{
							outObs.eType = Obstacle_Trap;
							outObs.hProp = pProp->GetMySmartPtr();
							outObs.fHitDist = fAlong;
							EtVector3 vN = vFrom - vTrapPos;
							vN.y = 0.f;
							if( EtVec3LengthSq( &vN ) > 0.01f ) EtVec3Normalize( &vN, &vN );
							outObs.vHitNormal = vN;
							bHitAny = true;
						}
					}
				}
			}
			continue;
		}

		if( bForProjectileLOS && pProp->IsProjectileSkip() )
			continue;

		bool bBreakableHittable = false;
		if( pProp->IsBrokenType() )
		{
			CDnWorldBrokenProp* pBroken = static_cast<CDnWorldBrokenProp*>( pProp );
			if( pBroken->IsBroken() )
				continue;
			bBreakableHittable = pBroken->IsHittable( m_pActor->GetActorHandle(), LocalTime );
		}

		EtAniObjectHandle hObj = pProp->GetObjectHandle();
		if( !hObj || !hObj->IsEnableCollision() || hObj->GetCollisionPrimitiveCount() <= 0 )
			continue;

		SAABox propBox;
		propBox.Reset();
		pProp->GetBoundingBox( propBox );
		if( propBox.Max.y < vFrom.y + 30.f || propBox.Min.y > vFrom.y + 155.f )
			continue;

		SCollisionResponse resp;
		bool bHit = ( fExpandRadius > 1.f )
			? hObj->FindCapsuleCollision( capsule, resp )
			: hObj->FindSegmentCollision( seg, resp );
		if( !bHit )
			continue;

		if( !bForProjectileLOS && resp.vNormal.y >= 0.65f )
			continue;

		float fContactDist = resp.fContactTime * fLen;
		if( fContactDist < 0.f ) fContactDist = 0.f;
		if( fContactDist < outObs.fHitDist )
		{
			outObs.eType = bBreakableHittable ? Obstacle_BreakableProp : Obstacle_SolidProp;
			outObs.hProp = pProp->GetMySmartPtr();
			outObs.fHitDist = fContactDist;
			outObs.vHitNormal = resp.vNormal;
			if( EtVec3LengthSq( &outObs.vHitNormal ) < 0.01f )
			{
				EtVector3 vAway = vFrom - pProp->GetMatEx()->m_vPosition;
				vAway.y = 0.f;
				if( EtVec3LengthSq( &vAway ) > 0.01f ) EtVec3Normalize( &vAway, &vAway );
				outObs.vHitNormal = vAway;
			}
			bHitAny = true;
		}
	}

	return bHitAny;
}

bool CDnAIBotController::ProbePathObstacle( const EtVector3& vFrom, const EtVector2& vDir2, float fProbeDist, bool bTestWorldCollision, LOCAL_TIME LocalTime, PathObstacle& outObs ) const
{
	outObs = PathObstacle();
	EtVector3 vTo( vFrom.x + vDir2.x * fProbeDist, vFrom.y, vFrom.z + vDir2.y * fProbeDist );

	if( CheckSegmentObstacle( vFrom, vTo, 24.f, false, LocalTime, outObs ) )
		return true;

	CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
	if( pWorld && pWorld->GetGrid() )
	{
		EtVector3 vFromCopy = vFrom;
		NavigationMesh* pNavMesh = pWorld->GetGrid()->GetNavMesh( vFromCopy );
		float fCurY = vFrom.y;
		static const float kSteps[2] = { 0.5f, 1.0f };
		for( int s = 0; s < 2; ++s )
		{
			float d = fProbeDist * kSteps[s];
			EtVector3 vSample( vFrom.x + vDir2.x * d, fCurY, vFrom.z + vDir2.y * d );
			float fGroundY = pWorld->GetHeightWithProp( vSample );
			float fDeltaY = fGroundY - fCurY;
			if( fDeltaY > 90.f || fDeltaY < -190.f )
			{
				outObs.eType = Obstacle_SteepSlope;
				outObs.fHitDist = d;
				outObs.vHitNormal = EtVector3( -vDir2.x, 0.f, -vDir2.y );
				return true;
			}
			vSample.y = fGroundY;

			if( pNavMesh && pNavMesh->TotalCells() > 0 )
			{
				NavigationCell* pCell = pNavMesh->FindCell( vSample );
				if( !pCell )
				{
					NavigationMesh* pSampleNav = pWorld->GetGrid()->GetNavMesh( vSample );
					if( !pSampleNav || !pSampleNav->FindCell( vSample ) )
					{
						outObs.eType = Obstacle_NavEdge;
						outObs.fHitDist = d;
						outObs.vHitNormal = EtVector3( -vDir2.x, 0.f, -vDir2.y );
						return true;
					}
				}
			}
			fCurY = fGroundY;
		}
	}

	if( bTestWorldCollision && m_pActor && m_pActor->GetObjectHandle() )
	{
		EtAniObjectHandle hObj = m_pActor->GetObjectHandle();
		if( hObj->IsEnableCollision() && hObj->GetCollisionPrimitiveCount() > 0 )
		{
			EtMatrix matOrig = *m_pActor->GetMatEx();
			EtMatrix matProbe = matOrig;
			float fColStep = ( fProbeDist < 75.f ) ? fProbeDist : 75.f;
			matProbe._41 += vDir2.x * fColStep;
			matProbe._43 += vDir2.y * fColStep;

			DNVector(SCollisionResponse) vecResp;
			bool bCol = hObj->FindCollision( matProbe, vecResp, true, true );
			hObj->UpdateCollisionPrimitive( matOrig );

			if( bCol )
			{
				for( size_t i = 0; i < vecResp.size(); ++i )
				{
					if( vecResp[i].vNormal.y >= 0.65f || vecResp[i].vNormal.y <= -0.65f )
						continue;
					outObs.eType = Obstacle_WorldCollision;
					outObs.fHitDist = fColStep * 0.5f;
					outObs.vHitNormal = vecResp[i].vNormal;
					return true;
				}
			}
		}
	}

	return false;
}

bool CDnAIBotController::ChooseClearSteeringDir( const EtVector3& vFrom, const EtVector2& vDesiredDir, float fProbeDist, bool bTestWorldCollision, LOCAL_TIME LocalTime, EtVector2& outDir, PathObstacle& outFrontObs )
{
	outDir = vDesiredDir;
	if( !ProbePathObstacle( vFrom, vDesiredDir, fProbeDist, bTestWorldCollision, LocalTime, outFrontObs ) )
		return true;

	if( outFrontObs.eType == Obstacle_BreakableProp && outFrontObs.hProp )
		return false;

	static const float kAnglesDeg[5] = { 30.f, 55.f, 82.f, 112.f, 145.f };
	float fBestScore = -9999.f;
	EtVector2 vBestDir = vDesiredDir;
	float fBestSide = m_fAvoidSideSign;
	bool bFoundClear = false;

	for( int i = 0; i < 5; ++i )
	{
		float fRad = EtToRadian( kAnglesDeg[i] );
		float c = cosf( fRad );
		float s = sinf( fRad );

		for( int side = 0; side < 2; ++side )
		{
			float fSign = ( side == 0 ) ? m_fAvoidSideSign : -m_fAvoidSideSign;
			float sn = s * fSign;
			EtVector2 vCand( vDesiredDir.x * c - vDesiredDir.y * sn, vDesiredDir.x * sn + vDesiredDir.y * c );
			PathObstacle candObs;
			if( ProbePathObstacle( vFrom, vCand, fProbeDist, bTestWorldCollision && i < 2, LocalTime, candObs ) )
				continue;

			float fScore = c * 100.f;
			if( side == 0 ) fScore += 14.f;
			if( EtVec3LengthSq( &outFrontObs.vHitNormal ) > 0.01f )
			{
				float fSlideDot = vCand.x * outFrontObs.vHitNormal.x + vCand.y * outFrontObs.vHitNormal.z;
				fScore += fSlideDot * 22.f;
			}
			if( fScore > fBestScore )
			{
				fBestScore = fScore;
				vBestDir = vCand;
				fBestSide = fSign;
				bFoundClear = true;
			}
		}
		if( bFoundClear && i <= 1 )
			break;
	}

	if( bFoundClear )
	{
		outDir = vBestDir;
		m_fAvoidSideSign = fBestSide;
	}
	return bFoundClear;
}

void CDnAIBotController::UpdateCombatDecision( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_pActor->IsStay() && !m_pActor->IsMove() )
		return;

	bool bIsNest = ( m_eEncounterMode == Mode_Nest || m_eEncounterMode == Mode_DragonNest );
	float fHealThreshold = bIsNest ? ( 0.80f + m_fDefensiveBias * 0.08f ) : 0.75f;
	if( fHealThreshold > 0.88f ) fHealThreshold = 0.88f;

	bool bWantBuff = ( m_fBuffCheckTimer <= 0.f );
	bool bNeedHeal = ( m_LastScan.hLowestHpAlly && !m_LastScan.hLowestHpAlly->IsDie() && m_LastScan.fLowestHpRatio < fHealThreshold );
	bool bNeedCure = ( m_LastScan.hDebuffedAlly && !m_LastScan.hDebuffedAlly->IsDie() );

	float fAllyDist = ( m_LastScan.hLowestHpAlly && !m_LastScan.hLowestHpAlly->IsDie() )
		? XZDist( *m_pActor->GetPosition(), *m_LastScan.hLowestHpAlly->GetPosition() )
		: 0.f;

	// 1. Heal / Cure / Party Buff triage comes first (especially for Healers/Supports)
	if( bNeedHeal || bNeedCure || ( bWantBuff && ( m_LastScan.bHasAnyEnemy || m_LastScan.bIsNestEncounter ) ) )
	{
		if( bWantBuff )
			m_fBuffCheckTimer = 3.8f;
		if( TryCastSupportOrHeal( bNeedHeal, bNeedCure, bWantBuff, fAllyDist ) )
			return;
	}

	// 1b. If a destructible object is directly blocking our path (even while heading to portal/enemy), smash it open!
	DnPropHandle hBlockerProp = m_hPathBlockingBreakable ? m_hPathBlockingBreakable : m_LastScan.hBlockingBreakableProp;
	if( hBlockerProp && hBlockerProp->IsShow() && hBlockerProp->IsBrokenType() )
	{
		CDnWorldBrokenProp* pBroken = static_cast<CDnWorldBrokenProp*>( hBlockerProp.GetPointer() );
		if( !pBroken->IsBroken() && pBroken->IsHittable( m_pActor->GetActorHandle(), LocalTime ) )
		{
			if( TryBreakPropSkillOrAttack( hBlockerProp->GetMatEx()->m_vPosition ) )
				return;
		}
		else if( pBroken->IsBroken() && hBlockerProp == m_hPathBlockingBreakable )
		{
			m_hPathBlockingBreakable.Identity();
		}
	}
	else if( hBlockerProp == m_hPathBlockingBreakable )
	{
		m_hPathBlockingBreakable.Identity();
	}

	// Do not get distracted by combat when human leader is waiting inside an active stage portal
	if( m_LastScan.bHumanInGate )
		return;

	// 2. Combat against active Enemy Target - STRICT distance, vertical reach & LOS enforcement!
	if( m_hTargetActor && !m_hTargetActor->IsDie() )
	{
		EtVector3 vMyPos = *m_pActor->GetPosition();
		EtVector3 vTargetPos = *m_hTargetActor->GetPosition();
		float fCenterDist = XZDist( vMyPos, vTargetPos );
		float fTargetRadius = GetActorEffectiveRadius( m_hTargetActor.GetPointer() );
		// Exact engine hit-check distance: caster center to target bounding box (matches DnActor::SquaredDistance)
		float fDistToTargetBox = ( fCenterDist > fTargetRadius ) ? ( fCenterDist - fTargetRadius ) : 0.f;
		float fVerticalDelta = fabsf( vTargetPos.y - vMyPos.y );
		bool bTargetInvuln = HasEnemyBlow( m_hTargetActor.GetPointer(), STATE_BLOW::BLOW_099 );
		if( !bTargetInvuln && m_hTargetActor->IsMonsterActor() )
			bTargetInvuln = static_cast<CDnMonsterActor*>( m_hTargetActor.GetPointer() )->IsEnableNoDamage();

		if( !m_bLoggedFirstCombat )
		{
			m_bLoggedFirstCombat = true;
			CDNUserSession* pSess = m_pActor->GetUserSession();
			g_Log.Log( LogType::_NORMAL, pSess->GetWorldSetID(), pSess->GetAccountDBID(), pSess->GetCharacterDBID(), pSess->GetSessionID(),
				L"[AIBot] TargetAcquired Bot=%s TargetClass=%d CenterDist=%.1f BoxDist=%.1f Vert=%.1f LOS=%d\n",
				m_pActor->GetName(), m_hTargetActor->GetClassID(), fCenterDist, fDistToTargetBox, fVerticalDelta,
				m_LastScan.bTargetHasLOS ? 1 : 0 );
		}

		// If primary enemy is invulnerable and there is a hittable destructible prop (crystal/totem), break it!
		if( bTargetInvuln && m_LastScan.hBreakableProp && m_LastScan.hBreakableProp->IsShow() )
		{
			if( TryBreakPropSkillOrAttack( m_LastScan.hBreakableProp->GetMatEx()->m_vPosition ) )
				return;
		}

		// If ranged attacker's line of sight is blocked by a solid pillar/wall, let positioning step around it instead of firing into the wall
		if( HasRangedBasicAttack() && !m_LastScan.bTargetHasLOS && fDistToTargetBox > 150.f )
			return;

		if( !bTargetInvuln )
		{
			if( TryCastCombatSkill( fDistToTargetBox, fVerticalDelta, m_LastScan.bBossInvulnerable ) )
				return;
			if( TryBasicComboAttack( fDistToTargetBox, fVerticalDelta ) )
				return;
		}
		else
		{
			// Boss is immune: save all skill cooldowns, only light tap if right next to it
			if( fDistToTargetBox < 160.f && fVerticalDelta < 120.f && TryBasicComboAttack( fDistToTargetBox, fVerticalDelta ) )
				return;
		}
		return;
	}

	// 3. Destructible Dungeon Prop (gates/barricades/chests/crystals)
	if( m_LastScan.hBreakableProp && m_LastScan.hBreakableProp->IsShow() )
		TryBreakPropSkillOrAttack( m_LastScan.hBreakableProp->GetMatEx()->m_vPosition );
}

bool CDnAIBotController::TryCastSupportOrHeal( bool bNeedHeal, bool bNeedCure, bool bWantBuff, float fAllyDist )
{
	CDnSkill* pChosen = NULL;
	int nBestPriority = -1;
	bool bChosenIsBurstBuff = false;
	bool bChosenIsHeal = false;

	bool bReadyForRaidBurst = ( m_LastScan.hPriorityBoss && !m_LastScan.hPriorityBoss->IsDie() && !m_LastScan.bBossInvulnerable && !m_LastScan.BossThreat.bBotCurrentlyInDanger );

	for( DWORD i = 0; i < m_pActor->GetSkillCount(); ++i )
	{
		DnSkillHandle hSkill = m_pActor->GetSkillFromIndex( i );
		if( !hSkill ) continue;
		CDnSkill* pSkill = hSkill.GetPointer();
		if( pSkill->GetSkillType() != CDnSkill::Active ) continue;
		if( pSkill->CanExecute() != CDnSkill::UsingResult::Success ) continue;

		CDnSkill::DurationTypeEnum eDur = pSkill->GetDurationType();
		if( eDur == CDnSkill::ActiveToggle || eDur == CDnSkill::TimeToggle || eDur == CDnSkill::Aura )
		{
			if( pSkill->IsToggleOn() || pSkill->IsAuraOn() ) continue;
		}
		if( eDur == CDnSkill::SummonOnOff && ActorHasOwnSummonedMonster( m_pActor ) )
			continue;

		bool bIsHeal = IsHealSkill( pSkill );
		bool bIsCure = IsCureSkill( pSkill );
		bool bIsRaidBurst = IsRaidBurstBuffSkill( pSkill );
		bool bIsBuff = IsPartyOrSelfBuffSkill( pSkill );

		// Don't blow major raid burst cooldowns (Spirit Boost / Beyond Time / Genie) on trash mobs or invuln bosses!
		if( bIsRaidBurst && ( m_eEncounterMode == Mode_Nest || m_eEncounterMode == Mode_DragonNest ) && !bReadyForRaidBurst )
			continue;

		int nPriority = -1;
		if( bNeedHeal && bIsHeal && fAllyDist <= 820.f )
			nPriority = m_LastScan.bHumanInDanger ? 120 : 100;
		else if( bNeedCure && bIsCure && fAllyDist <= 850.f )
			nPriority = 92;
		else if( bWantBuff && bIsRaidBurst && ( bReadyForRaidBurst || m_LastScan.nAliveEnemyCount >= 3 ) )
			nPriority = 85;
		else if( bWantBuff && bIsBuff )
			nPriority = ( pSkill->GetTargetType() == CDnSkill::Party || pSkill->GetTargetType() == CDnSkill::Friend ) ? 75 : 65;

		if( nPriority > nBestPriority )
		{
			nBestPriority = nPriority;
			pChosen = pSkill;
			bChosenIsBurstBuff = bIsRaidBurst;
			bChosenIsHeal = bIsHeal;
		}
	}

	if( !pChosen )
		return false;

	if( m_eMoveState == MoveState_Moving )
		SendStopPacket();

	if( bNeedHeal && m_LastScan.hLowestHpAlly && !m_LastScan.hLowestHpAlly->IsDie() )
		FacePosition( *m_LastScan.hLowestHpAlly->GetPosition() );
	else if( m_hTargetActor && !m_hTargetActor->IsDie() )
		FacePosition( *m_hTargetActor->GetPosition() );

	CDnSkill::UsingResult eRes = m_pActor->UseSkill( pChosen->GetClassID(), true );
	if( !m_bLoggedFirstAction )
	{
		m_bLoggedFirstAction = true;
		CDNUserSession* pSess = m_pActor->GetUserSession();
		g_Log.Log( LogType::_NORMAL, pSess->GetWorldSetID(), pSess->GetAccountDBID(), pSess->GetCharacterDBID(), pSess->GetSessionID(),
			L"[AIBot] FirstSupportSkill Bot=%s SkillID=%d Lv=%d Res=%d\n",
			m_pActor->GetName(), pChosen->GetClassID(), pChosen->GetLevel(), (int)eRes );
	}
	if( eRes == CDnSkill::UsingResult::Success )
	{
		SendUseSkillPacket( pChosen );
		m_fSkillDelayTimer = 0.42f + m_fReactionJitter;
		if( bChosenIsBurstBuff && ( m_eEncounterMode == Mode_Nest || m_eEncounterMode == Mode_DragonNest ) )
			MaybeSendTacticalCallout( L"Buffs up, burst now!", 26.f );
		else if( bChosenIsHeal && m_LastScan.bHumanInDanger )
			MaybeSendTacticalCallout( L"Healing!", 26.f );
		return true;
	}
	return false;
}

bool CDnAIBotController::TryCastCombatSkill( float fDistToTargetBox, float fVerticalDelta, bool bBossInvuln )
{
	CDnSkill* pBestSkill = NULL;
	float fBestScore = -1.f;
	float fBestEffMaxRange = 0.f;

	bool bTargetIsBoss = ( m_hTargetActor && m_hTargetActor == m_LastScan.hPriorityBoss );
	bool bIsNest = ( m_eEncounterMode == Mode_Nest || m_eEncounterMode == Mode_DragonNest );
	int nJobID = m_pActor->GetJobClassID();

	for( DWORD i = 0; i < m_pActor->GetSkillCount(); ++i )
	{
		DnSkillHandle hSkill = m_pActor->GetSkillFromIndex( i );
		if( !hSkill ) continue;
		CDnSkill* pSkill = hSkill.GetPointer();
		if( !IsOffensiveActiveSkill( pSkill ) ) continue;
		if( pSkill->CanExecute() != CDnSkill::UsingResult::Success ) continue;

		const SkillProfile& prof = GetOrAnalyzeSkillProfile( pSkill );

		// Never waste non-damaging pure utility/stance skills when trying to attack
		if( !prof.bHasHitSignal && !prof.bHasProjectile && !prof.bIsSummon && !prof.bIsDebuffOrArmorBreak && !IsThreatSkill( pSkill ) )
			continue;

		// STRICT ENGINE DISTANCE & VERTICAL REACH CHECK (never casts when enemy is too far or out of reach!)
		float fEffMaxRange = GetEffectiveSkillMaxRange( prof );
		if( fDistToTargetBox > fEffMaxRange )
			continue;
		if( prof.fMinRange > 30.f && fDistToTargetBox < prof.fMinRange )
			continue;
		if( fVerticalDelta > prof.fMaxVerticalReach )
			continue;

		// Don't fire projectile skills through a solid wall/pillar
		if( prof.bHasProjectile && !m_LastScan.bTargetHasLOS && fDistToTargetBox > 140.f )
			continue;

		// Don't dash straight into an imminent boss frontal/PBAoE telegraph!
		if( prof.bIsDashForward && m_LastScan.BossThreat.bBotCurrentlyInDanger )
			continue;

		// Strategy: Save 40-Ultimates for Boss fights & Burst Windows (don't waste Ult on 1 trash mob or invuln boss!)
		float fCD = pSkill->GetDelayTime();
		if( prof.bIsUltimate )
		{
			if( bBossInvuln ) continue;
			if( !bTargetIsBoss ) continue;
			if( bIsNest && !m_LastScan.bBurstWindowActive && m_LastScan.fBossHpRatio > 0.60f )
				continue;
		}

		// Save long cooldown burst skills when boss is currently invulnerable
		if( bBossInvuln && fCD >= 12.f ) continue;

		// Score skill tactically by role, encounter mode, cluster size, real damage prob, distance fit & learned reliability!
		float fScore = 15.f + (float)pSkill->GetLevel() * 0.8f + ClampF( prof.fTotalDamageProb * 6.f, 0.f, 40.f );

		// Favor skills whose optimal range matches our current distance (human-like spacing awareness)
		float fRangeFit = 1.0f - ClampF( fabsf( fDistToTargetBox - prof.fOptimalRange ) / ( fEffMaxRange + 100.f ), 0.f, 1.0f );
		fScore += fRangeFit * 18.f;

		// Weave strong medium-CD skills before quick fillers
		if( fCD > 3.f && fCD < 45.f )
			fScore += fCD * 0.40f;

		// AoE bonus when fighting packed dungeon waves!
		if( prof.bIsAoE && m_LastScan.nEnemiesNearTarget >= 2 )
			fScore += (float)( m_LastScan.nEnemiesNearTarget * 11 );

		// Burst Window & Ultimate priority on Nest / Stage Bosses!
		if( prof.bIsUltimate && bTargetIsBoss )
			fScore += m_LastScan.bBurstWindowActive ? 95.f : 55.f;
		else if( m_LastScan.bBurstWindowActive && fCD >= 12.f )
			fScore += 28.f;

		// Role-specific Nest & Dungeon meta priorities
		if( m_Role == AIBot::RoleType::Tank )
		{
			if( IsThreatSkill( pSkill ) || pSkill->GetAdditionalThreat() > 0 )
				fScore += 50.f;
			if( prof.bIsDebuffOrArmorBreak )
				fScore += 32.f;
		}
		else if( m_Role == AIBot::RoleType::IceStacker )
		{
			if( IsIceStackSkill( pSkill ) )
				fScore += bTargetIsBoss ? 65.f : 35.f;
		}
		else if( m_Role == AIBot::RoleType::Support )
		{
			if( prof.bIsDebuffOrArmorBreak )
				fScore += 35.f;
		}
		else if( m_Role == AIBot::RoleType::Dps )
		{
			if( prof.bIsSummon && bTargetIsBoss )
				fScore += 30.f;
		}

		// Adaptive Learning weighting: prefer skills that reliably land hits and multi-hit!
		SkillLearnEntry learned;
		if( CTacticalLearningMemory::Get().QuerySkillLearning( nJobID, pSkill->GetClassID(), learned ) && learned.nTotalCasts >= 2 )
		{
			fScore += ( learned.fHitRateEMA - 0.5f ) * 26.f;
			fScore += ClampF( learned.fAvgHitsPerCastEMA * 3.f, 0.f, 15.f );
		}

		if( fScore > fBestScore )
		{
			fBestScore = fScore;
			pBestSkill = pSkill;
			fBestEffMaxRange = fEffMaxRange;
		}
	}

	if( !pBestSkill )
		return false;

	if( m_eMoveState == MoveState_Moving )
		SendStopPacket();

	if( m_hTargetActor )
	{
		m_vAimPos = ComputeTargetAimPoint( m_hTargetActor.GetPointer(), true );
		FacePosition( m_vAimPos );
	}

	CDnSkill::UsingResult eRes = m_pActor->UseSkill( pBestSkill->GetClassID(), true );
	if( !m_bLoggedFirstAction )
	{
		m_bLoggedFirstAction = true;
		CDNUserSession* pSess = m_pActor->GetUserSession();
		g_Log.Log( LogType::_NORMAL, pSess->GetWorldSetID(), pSess->GetAccountDBID(), pSess->GetCharacterDBID(), pSess->GetSessionID(),
			L"[AIBot] FirstCombatSkill Bot=%s SkillID=%d Lv=%d BoxDist=%.1f EffMax=%.1f Res=%d\n",
			m_pActor->GetName(), pBestSkill->GetClassID(), pBestSkill->GetLevel(), fDistToTargetBox, fBestEffMaxRange, (int)eRes );
	}
	if( eRes == CDnSkill::UsingResult::Success )
	{
		SendUseSkillPacket( pBestSkill );
		m_fSkillDelayTimer = 0.28f + m_fReactionJitter;

		// Arm the live hit/whiff learner for this skill cast!
		FinalizePendingSkillOutcome( true );
		m_nPendingCastSkillID = pBestSkill->GetClassID();
		m_fPendingCastDist = fDistToTargetBox;
		m_fPendingCastTimer = 1.65f;
		m_bPendingCastHit = false;
		m_nPendingCastHitCount = 0;
		m_hPendingCastTarget = m_hTargetActor;

		if( bTargetIsBoss && m_Role == AIBot::RoleType::Tank && IsThreatSkill( pBestSkill ) && bIsNest )
			MaybeSendTacticalCallout( L"Got aggro, hit from behind!", 30.f );

		return true;
	}
	return false;
}

bool CDnAIBotController::HasRangedBasicAttack() const
{
	const char* szAction = NULL;
	bool bRanged = false;
	ResolveBasicAttackAction( 0, &szAction, &bRanged );
	return bRanged;
}

bool CDnAIBotController::ResolveBasicAttackAction( int nComboStep, const char** pOutAction, bool* pbOutIsRanged ) const
{
	if( pOutAction ) *pOutAction = NULL;
	if( pbOutIsRanged ) *pbOutIsRanged = false;
	if( !m_pActor ) return false;

	int nBaseClass = m_pActor->GetClassID();
	int nEquipType = m_pActor->GetWeapon( 0 ) ? (int)m_pActor->GetWeapon( 0 )->GetEquipType() : -1;
	int nStep = nComboStep % 3;

	const char* candidates[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
	int nCandCount = 0;
	bool bRanged = false;

	auto addCand = [&]( const char* szName ) {
		if( szName && nCandCount < 6 )
			candidates[nCandCount++] = szName;
	};

	if( nBaseClass == 2 )
	{
		bRanged = true;
		if( nEquipType == CDnWeapon::BigBow ) addCand( "Shoot_Stand_BigBow" );
		else if( nEquipType == CDnWeapon::CrossBow ) addCand( "Shoot_Stand_CrossBow" );
		else if( nEquipType == CDnWeapon::SmallBow ) addCand( "Shoot_Stand_SmallBow" );
		addCand( "Shoot_Stand_SmallBow" );
		addCand( "Shoot_Stand_BigBow" );
		addCand( "Shoot_Stand_CrossBow" );
		addCand( "Shoot_Stand" );
	}
	else if( nBaseClass == 3 )
	{
		bRanged = true;
		addCand( "Shoot_Stand" );
	}
	else if( nBaseClass == 5 )
	{
		bRanged = true;
		if( nEquipType == CDnWeapon::Cannon ) addCand( "Shoot_Stand_Cannon" );
		else if( nEquipType == CDnWeapon::BubbleGun ) addCand( "Shoot_Stand_Bubble" );
		addCand( "Shoot_Stand_Cannon" );
		addCand( "Shoot_Stand_Bubble" );
	}
	else if( nBaseClass == 6 )
	{
		if( nEquipType == CDnWeapon::Fan )
		{
			bRanged = true;
			static const char* kFan[3] = { "Attack1_Fan", "Attack2_Fan", "Attack3_Fan" };
			addCand( kFan[nStep] );
			addCand( "Attack1_Fan" );
			addCand( "Attack1" );
		}
		else
		{
			static const char* kChakram[3] = { "Attack1_Chakram", "Attack2_Chakram", "Attack3_Chakram" };
			addCand( kChakram[nStep] );
			addCand( "Attack1_Chakram" );
		}
	}
	else if( nBaseClass == 1 )
	{
		static const char* kSword[3]  = { "Attack1_Sword", "Attack2_Sword", "Attack3_Sword" };
		static const char* kAxe[3]    = { "Attack1_Axe", "Attack2_Axe", "Attack3_Axe" };
		static const char* kHammer[3] = { "Attack1_Hammer", "Attack2_Hammer", "Attack3_Hammer" };
		static const char* kBase[3]   = { "Attack1", "Attack2", "Attack3" };
		if( nEquipType == CDnWeapon::Sword ) addCand( kSword[nStep] );
		else if( nEquipType == CDnWeapon::Axe ) addCand( kAxe[nStep] );
		else if( nEquipType == CDnWeapon::Hammer ) addCand( kHammer[nStep] );
		addCand( kBase[nStep] );
		addCand( "Attack1" );
	}
	else if( nBaseClass == 4 )
	{
		static const char* kMace[3]  = { "Attack1_Mace", "Attack2_Mace", "Attack3_Mace" };
		static const char* kFlail[3] = { "Attack1_Flail", "Attack2_Flail", "Attack3_Flail" };
		static const char* kWand[3]  = { "Attack1_Wand", "Attack2_Wand", "Attack3_Wand" };
		static const char* kBase[3]  = { "Attack1", "Attack2", "Attack3" };
		if( nEquipType == CDnWeapon::Mace ) addCand( kMace[nStep] );
		else if( nEquipType == CDnWeapon::Flail ) addCand( kFlail[nStep] );
		else if( nEquipType == CDnWeapon::Wand ) addCand( kWand[nStep] );
		addCand( kBase[nStep] );
		addCand( "Attack1" );
	}
	else
	{
		addCand( "Attack1" );
		addCand( "Shoot_Stand" );
	}

	if( pbOutIsRanged )
		*pbOutIsRanged = bRanged;

	for( int i = 0; i < nCandCount; ++i )
	{
		if( m_pActor->GetElementIndex( candidates[i] ) != -1 )
		{
			if( pOutAction ) *pOutAction = candidates[i];
			return true;
		}
	}
	return false;
}

bool CDnAIBotController::TryBasicComboAttack( float fDistToTargetBox, float fVerticalDelta )
{
	const char* szAction = NULL;
	bool bRangedBasic = false;
	if( !ResolveBasicAttackAction( m_nBasicComboStep, &szAction, &bRangedBasic ) || !szAction )
		return false;

	// Compute real basic attack reach directly from the basic attack action's STE_Hit / STE_Projectile signals!
	float fMaxBasicRange = bRangedBasic ? 540.f : 155.f;
	CDnActionBase::ActionElementStruct* pElem = m_pActor->GetElement( szAction );
	if( pElem )
	{
		float fWeaponLen = ( m_pActor->GetWeapon( 0 ) && m_pActor->GetWeapon( 0 )->GetWeaponLength() > 0 )
			? (float)m_pActor->GetWeapon( 0 )->GetWeaponLength() : 0.f;
		if( !bRangedBasic )
		{
			std::map<int, std::vector<CEtActionSignal*>>::iterator itHit = pElem->MapSearch.find( STE_Hit );
			if( itHit != pElem->MapSearch.end() )
			{
				float fActMax = 0.f;
				for( size_t i = 0; i < itHit->second.size(); ++i )
				{
					HitStruct* pHit = static_cast<HitStruct*>( itHit->second[i]->GetData() );
					if( !pHit ) continue;
					float fOffZ = pHit->vOffset ? pHit->vOffset->z : 0.f;
					float fR = fOffZ + pHit->fDistanceMax + ( pHit->bIncludeWeaponLength ? fWeaponLen : 0.f );
					if( fR > fActMax ) fActMax = fR;
				}
				if( fActMax > 40.f )
					fMaxBasicRange = ClampF( fActMax * 0.90f, 85.f, 240.f );
			}
		}
		else
		{
			std::map<int, std::vector<CEtActionSignal*>>::iterator itProj = pElem->MapSearch.find( STE_Projectile );
			if( itProj != pElem->MapSearch.end() && !itProj->second.empty() )
			{
				ProjectileStruct* pProj = static_cast<ProjectileStruct*>( itProj->second[0]->GetData() );
				if( pProj && pProj->fSpeed > 15.f && pProj->nValidTime > 0 )
				{
					float fR = pProj->fSpeed * ( (float)pProj->nValidTime / 1000.f );
					fMaxBasicRange = ClampF( fR * 0.85f, 240.f, 620.f );
				}
			}
		}
	}

	if( fDistToTargetBox > fMaxBasicRange || fVerticalDelta > ( bRangedBasic ? 260.f : 135.f ) )
		return false;

	if( m_eMoveState == MoveState_Moving )
		SendStopPacket();

	if( m_hTargetActor )
	{
		m_vAimPos = ComputeTargetAimPoint( m_hTargetActor.GetPointer(), true );
		FacePosition( m_vAimPos );
	}

	if( !m_bLoggedFirstAction )
	{
		m_bLoggedFirstAction = true;
		CDNUserSession* pSess = m_pActor->GetUserSession();
		g_Log.Log( LogType::_NORMAL, pSess->GetWorldSetID(), pSess->GetAccountDBID(), pSess->GetCharacterDBID(), pSess->GetSessionID(),
			L"[AIBot] FirstBasicAttack Bot=%s Action=%S BoxDist=%.1f MaxRange=%.1f\n",
			m_pActor->GetName(), szAction, fDistToTargetBox, fMaxBasicRange );
	}

	m_pActor->CmdAction( szAction, 0, 3.f );
	SendActionPacket( szAction, 0, 3.f, false );
	++m_nBasicComboStep;
	m_fSkillDelayTimer = ( bRangedBasic ? 0.44f : 0.36f ) + m_fReactionJitter;
	return true;
}

bool CDnAIBotController::TryBreakPropSkillOrAttack( const EtVector3& vPropPos )
{
	const char* szAction = NULL;
	bool bRangedBasic = false;
	if( !ResolveBasicAttackAction( 0, &szAction, &bRangedBasic ) || !szAction )
		return false;

	float fDist = XZDist( *m_pActor->GetPosition(), vPropPos );
	float fMaxPropDist = bRangedBasic ? 540.f : 190.f;
	if( fDist > fMaxPropDist )
		return false;

	if( m_eMoveState == MoveState_Moving )
		SendStopPacket();

	m_vAimPos = vPropPos;
	m_vAimPos.y += 50.f;
	FacePosition( vPropPos );

	// Use a quick low-cooldown offensive skill in exact range to smash barricades/crystals faster
	for( DWORD i = 0; i < m_pActor->GetSkillCount(); ++i )
	{
		DnSkillHandle hSkill = m_pActor->GetSkillFromIndex( i );
		if( !hSkill ) continue;
		CDnSkill* pSkill = hSkill.GetPointer();
		if( !IsOffensiveActiveSkill( pSkill ) ) continue;
		if( pSkill->bIsExtremitySkill() || pSkill->GetDelayTime() > 11.f ) continue;
		if( pSkill->CanExecute() != CDnSkill::UsingResult::Success ) continue;

		const SkillProfile& prof = GetOrAnalyzeSkillProfile( pSkill );
		if( !prof.bHasHitSignal && !prof.bHasProjectile ) continue;
		if( fDist > GetEffectiveSkillMaxRange( prof ) ) continue;

		if( m_pActor->UseSkill( pSkill->GetClassID(), true ) == CDnSkill::UsingResult::Success )
		{
			SendUseSkillPacket( pSkill );
			m_fSkillDelayTimer = 0.36f;
			return true;
		}
		break;
	}

	m_pActor->CmdAction( szAction, 0, 3.f );
	SendActionPacket( szAction, 0, 3.f, false );
	m_fSkillDelayTimer = bRangedBasic ? 0.45f : 0.40f;
	return true;
}

bool CDnAIBotController::TryEvadeDanger()
{
	return TryDirectionalTumble( m_LastScan.vDangerAvoidVec );
}

void CDnAIBotController::PerformLeashTeleport( const EtVector3& vAnchor )
{
	if( m_eMoveState == MoveState_Moving )
		SendStopPacket();

	float fAngle = EtToRadian( (float)( ( m_nBotSlotIndex * 51 ) % 360 ) );
	EtVector3 vDest( vAnchor.x + cosf( fAngle ) * 90.f, vAnchor.y, vAnchor.z + sinf( fAngle ) * 90.f );

	CDnWorld* pWorld = CDnWorld::GetInstancePtr( m_pActor->GetRoom() );
	if( pWorld )
	{
		if( pWorld->GetGrid() )
		{
			EtVector3 vAnchorCopy = vAnchor;
			NavigationMesh* pNavMesh = pWorld->GetGrid()->GetNavMesh( vAnchorCopy );
			if( pNavMesh && pNavMesh->TotalCells() > 0 )
			{
				NavigationCell* pCell = pNavMesh->FindCell( vDest );
				if( !pCell )
				{
					vDest = vAnchor;
					pCell = pNavMesh->FindClosestCell( vDest );
				}
				if( pCell )
					vDest = pNavMesh->FittingPoint( pCell, vDest );
			}
		}
		vDest.y = pWorld->GetHeightWithProp( vDest );
	}

	m_pActor->SetPosition( vDest );
	m_pActor->SetStaticPosition( vDest );
	m_vecWaypoints.clear();
	m_fStuckAccumTime = 0.f;
	m_bBlockedByObstacle = false;
	m_hPathBlockingBreakable.Identity();
	m_vStuckCheckPos = vDest;
	SendStopPacket( true );
}

void CDnAIBotController::FacePosition( const EtVector3& vTargetPos )
{
	EtVector2 vDir2 = SafeDir2( *m_pActor->GetPosition(), vTargetPos );
	EtVector3 vXVec;
	EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vDir2 ) );
	m_pActor->SetMoveVectorX( vXVec );
	m_pActor->SetMoveVectorZ( EtVec2toVec3( vDir2 ) );
	m_pActor->Look( vDir2, true );
	m_vLastLookDir = vDir2;
	SendLookPacket( vDir2 );
}

void CDnAIBotController::ToggleBattleState( bool bBattle )
{
	m_pActor->CmdToggleBattle( bBattle );
	m_pActor->OnBattleToggle( bBattle );
}

void CDnAIBotController::SendMovePacket( const EtVector2& vDir2 )
{
	SendDirectionalMovePacket( "Move_Front", 0x04, vDir2, vDir2 );
}

void CDnAIBotController::SendDirectionalMovePacket( const char* szMoveAction, char cFlag, const EtVector2& vZVec, const EtVector2& vLook )
{
	if( !szMoveAction ) szMoveAction = "Move_Front";
	int nActionIndex = m_pActor->GetElementIndex( szMoveAction );
	if( nActionIndex == -1 )
	{
		nActionIndex = m_pActor->GetElementIndex( "Move_Front" );
		cFlag = 0x04;
	}
	if( nActionIndex == -1 ) return;

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	DWORD dwGap = 0;
	EtVector3 vPos = *m_pActor->GetPosition();
	EtVector2 vZ = vZVec;
	EtVector2 vL = vLook;
	int nMoveSpeed = m_pActor->GetMoveSpeed();

	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &vZ, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &vL, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &cFlag, sizeof(char) );
	Stream.Write( &nMoveSpeed, sizeof(int) );

	m_pActor->Send( eActor::SC_CMDMOVE, &Stream );
}

void CDnAIBotController::SendStopPacket( bool bResetStop )
{
	m_pActor->CmdStop( "Stand" );
	m_eMoveState = MoveState_Stopped;

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	DWORD dwGap = 0;
	EtVector3 vPos = *m_pActor->GetPosition();

	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &bResetStop, sizeof(bool) );

	m_pActor->Send( eActor::SC_CMDSTOP, &Stream );
}

void CDnAIBotController::SendLookPacket( const EtVector2& vDir2 )
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	EtVector2 vZVec = vDir2;
	EtVector2 vLook = vDir2;

	Stream.Write( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

	m_pActor->Send( eActor::SC_CMDLOOK, &Stream );
}

void CDnAIBotController::SendActionPacket( const char* szActionName, int nLoopCount, float fBlendFrame, bool bSkillChain )
{
	int nActionIndex = m_pActor->GetElementIndex( szActionName );
	if( nActionIndex == -1 ) return;

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	DWORD dwGap = 0;
	EtVector3 vPos = *m_pActor->GetPosition();
	EtVector2 vZVec = EtVec3toVec2( *m_pActor->GetMoveVectorZ() );
	EtVector2 vLook = EtVec3toVec2( *m_pActor->GetLookDir() );
	char cMovePushKeyFlag = 0;
	bool bFromStateBlow = false;

	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
	Stream.Write( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &cMovePushKeyFlag, sizeof(char) );
	Stream.Write( &bFromStateBlow, sizeof(bool) );
	Stream.Write( &bSkillChain, sizeof(bool) );

	m_pActor->Send( eActor::SC_CMDACTION, &Stream );
}

void CDnAIBotController::SendUseSkillPacket( CDnSkill* pSkill )
{
	if( !pSkill ) return;

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	int nSkillTableID = pSkill->GetClassID();
	BYTE cLevel = (BYTE)pSkill->GetLevel();
	bool bUseApplySkillItem = false;
	bool abSignalSkillCheck[3] = { false, false, false };
	EtVector2 vZVec = EtVec3toVec2( *m_pActor->GetMoveVectorZ() );
	EtVector2 vLook = EtVec3toVec2( *m_pActor->GetLookDir() );
	bool bAutoUseFromServer = false;
	int nEnchantSkillID = pSkill->GetEnchantedSkillID();
	EtVector3 vPos = *m_pActor->GetPosition();

	Stream.Write( &nSkillTableID, sizeof(int) );
	Stream.Write( &cLevel, sizeof(char) );
	Stream.Write( &bUseApplySkillItem, sizeof(bool) );
	Stream.Write( abSignalSkillCheck, sizeof(abSignalSkillCheck) );
	Stream.Write( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &bAutoUseFromServer, sizeof(bool) );
	Stream.Write( &nEnchantSkillID, sizeof(int) );
#ifdef PRE_ADD_POSITION_SYNC_BY_SKILL_USAGE
	Stream.Write( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
#endif

	m_pActor->Send( eActor::SC_USESKILL, &Stream );
}

bool CDnAIBotController::IsHarmfulDebuffPresent( CDnPlayerActor* pAlly )
{
	if( !pAlly ) return false;
	static const STATE_BLOW::emBLOW_INDEX s_Debuffs[] = {
		STATE_BLOW::BLOW_041,
		STATE_BLOW::BLOW_042,
		STATE_BLOW::BLOW_043,
		STATE_BLOW::BLOW_044,
		STATE_BLOW::BLOW_045,
		STATE_BLOW::BLOW_063,
		STATE_BLOW::BLOW_070,
		STATE_BLOW::BLOW_071,
		STATE_BLOW::BLOW_144,
	};
	for( size_t i = 0; i < _countof( s_Debuffs ); ++i )
	{
		if( pAlly->IsAppliedThisStateBlow( s_Debuffs[i] ) )
			return true;
	}
	return false;
}

bool CDnAIBotController::HasSkillBlow( CDnSkill* pSkill, int nBlowIndex )
{
	if( !pSkill ) return false;
	DWORD dwCnt = pSkill->GetStateEffectCount();
	for( DWORD i = 0; i < dwCnt; ++i )
	{
		CDnSkill::StateEffectStruct* pSE = pSkill->GetStateEffectFromIndex( i );
		if( pSE && pSE->nID == nBlowIndex )
			return true;
	}
	return false;
}

bool CDnAIBotController::IsHealSkill( CDnSkill* pSkill )
{
	if( !pSkill || pSkill->GetTargetType() == CDnSkill::Enemy ) return false;
	return HasSkillBlow( pSkill, STATE_BLOW::BLOW_011 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_012 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_015 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_016 );
}

bool CDnAIBotController::IsCureSkill( CDnSkill* pSkill )
{
	if( !pSkill || pSkill->GetTargetType() == CDnSkill::Enemy ) return false;
	return HasSkillBlow( pSkill, STATE_BLOW::BLOW_069 );
}

bool CDnAIBotController::IsRaidBurstBuffSkill( CDnSkill* pSkill )
{
	if( !pSkill ) return false;
	int nID = pSkill->GetClassID();
	if( nID == TIME_ACCELERATION_SKILL_ID || nID == SPRIT_BOOST_SKILL_ID )
		return true;
	// Long-cooldown party raid offensive buffs (e.g. Genie, Battle Howl, Cocktail, Blessing)
	if( pSkill->GetDurationType() == CDnSkill::Buff &&
		( pSkill->GetTargetType() == CDnSkill::Party || pSkill->GetTargetType() == CDnSkill::Friend ) &&
		pSkill->GetDelayTime() >= 45.f )
	{
		return true;
	}
	return false;
}

bool CDnAIBotController::IsPartyOrSelfBuffSkill( CDnSkill* pSkill )
{
	if( !pSkill ) return false;
	CDnSkill::DurationTypeEnum eDur = pSkill->GetDurationType();
	if( eDur == CDnSkill::Buff || eDur == CDnSkill::SummonOnOff || eDur == CDnSkill::Aura )
	{
		CDnSkill::TargetTypeEnum eTgt = pSkill->GetTargetType();
		if( eTgt == CDnSkill::Self || eTgt == CDnSkill::Friend || eTgt == CDnSkill::Party || eTgt == CDnSkill::All )
			return true;
	}
	return false;
}

bool CDnAIBotController::IsIceStackSkill( CDnSkill* pSkill )
{
	return HasSkillBlow( pSkill, STATE_BLOW::BLOW_041 ) || HasSkillBlow( pSkill, STATE_BLOW::BLOW_144 );
}

bool CDnAIBotController::IsThreatSkill( CDnSkill* pSkill )
{
	if( !pSkill ) return false;
	return ( pSkill->GetAdditionalThreat() > 0 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_030 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_055 ) ||
		HasSkillBlow( pSkill, STATE_BLOW::BLOW_066 );
}

bool CDnAIBotController::IsOffensiveActiveSkill( CDnSkill* pSkill )
{
	if( !pSkill ) return false;
	if( pSkill->GetSkillType() != CDnSkill::Active ) return false;
	if( IsHealSkill( pSkill ) || IsCureSkill( pSkill ) ) return false;
	CDnSkill::DurationTypeEnum eDur = pSkill->GetDurationType();
	if( eDur == CDnSkill::SummonOnOff ) return false;
	if( eDur == CDnSkill::Buff && pSkill->GetTargetType() == CDnSkill::Self ) return false;
	if( pSkill->GetProcessor( IDnSkillProcessor::PLAY_ANI ) == NULL &&
		pSkill->GetProcessor( IDnSkillProcessor::PARTIAL_PLAY_ANI ) == NULL )
		return false;
	return true;
}

bool CDnAIBotController::IsHittableEnemy( CDnActor* pActor, DnActorHandle hHitter, int nBotTeam, LOCAL_TIME LocalTime )
{
	if( !pActor || pActor->IsDie() || !pActor->IsShow() )
		return false;
	if( !pActor->IsHittable( hHitter, LocalTime ) )
		return false;
	if( pActor->GetTeam() == nBotTeam || pActor->GetTeam() == 0 )
		return false;
	if( !pActor->IsMonsterActor() )
		return false;
	if( pActor->GetActorType() == CDnActor::NoAggroTrap )
		return false;
	return true;
}

int CDnAIBotController::ScoreEnemyPriority( CDnMonsterActor* pMon, AIBot::RoleType role, float fDistSq, CDnPlayerActor* pHumanLeader, EncounterMode eMode )
{
	float fDist = sqrtf( fDistSq );
	int nScore = 1000 - (int)( fDist * 0.45f );

	bool bInvuln = HasEnemyBlow( pMon, STATE_BLOW::BLOW_099 ) || pMon->IsEnableNoDamage();
	if( bInvuln )
		nScore -= 2800;

	CDnMonsterState::GradeEnum eGrade = pMon->GetGrade();
	bool bIsBoss = ( eGrade >= CDnMonsterState::Boss );
	bool bIsElite = ( eGrade >= CDnMonsterState::Elite && eGrade < CDnMonsterState::Boss );
	bool bIsNest = ( eMode == Mode_Nest || eMode == Mode_DragonNest );

	if( role == AIBot::RoleType::Tank || role == AIBot::RoleType::IceStacker )
	{
		if( bIsBoss && !bInvuln ) nScore += bIsNest ? 1100 : 900;
		else if( bIsElite && !bInvuln ) nScore += 450;
	}
	else
	{
		// In Nests, summoned adds & mechanic mobs must be cleared rapidly while protecting Human Player!
		if( !bInvuln && pMon->IsSummonedMonster() )
			nScore += bIsNest ? 750 : 450;
		if( pHumanLeader && XZDistSq( *pMon->GetPosition(), *pHumanLeader->GetPosition() ) < ( 360.f * 360.f ) )
			nScore += 380;
		if( bIsBoss && !bInvuln )
			nScore += bIsNest ? 620 : 480;
		else if( bIsElite && !bInvuln )
			nScore += 280;
	}

	return nScore;
}

void CDnAIBotController::SendBotProjectilePacket( CDnPlayerActor* pBotActor, CDnProjectile* pProjectile, ProjectileStruct* pStruct, int nActionIndex, int nSignalIndex )
{
	if( !pBotActor || !pProjectile || !pStruct ) return;

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	DWORD dwGap = 0;
	bool bActorAttachWeapon = ( pStruct->nWeaponTableID == 0 );
	DWORD dwUniqueID = pProjectile->GetUniqueID();
	int nWeaponTableID = pStruct->nWeaponTableID;
	MatrixEx* pCross = pProjectile->GetMatEx();

	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( &bActorAttachWeapon, sizeof(bool) );
	Stream.Write( &dwUniqueID, sizeof(DWORD) );
	if( !bActorAttachWeapon )
		Stream.Write( &nWeaponTableID, sizeof(int) );

	Stream.Write( &pCross->m_vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &pCross->m_vXAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
	Stream.Write( &pCross->m_vYAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
	Stream.Write( &pCross->m_vZAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );

#ifdef PRE_MOD_PROJECTILE_HACK
	char cShooterType = 0;
	DWORD dwParentShooterUniqueID = pBotActor->GetUniqueID();
	Stream.Write( &cShooterType, sizeof(char) );
	Stream.Write( &dwParentShooterUniqueID, sizeof(DWORD) );
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nSignalIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
#else
	int nOrbitType = (int)pProjectile->GetOrbitType();
	int nDestroyType = (int)pProjectile->GetDestroyOrbitType();
	int nTargetType = (int)pProjectile->GetTargetType();
	int nValidTime = pStruct->nValidTime;
	int nWeaponLength = pProjectile->GetWeaponLength();
	float fOrbitRotZ = pStruct->fProjectileOrbitRotateZ;
	Stream.Write( &nOrbitType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &nDestroyType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &nTargetType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &nValidTime, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nWeaponLength, sizeof(int) );
	Stream.Write( &fOrbitRotZ, sizeof(float) );
#endif

	int nTargetPartsIndex = -1;
	int nTargetBoneIndex = -1;
	Stream.Write( &nTargetPartsIndex, sizeof(int) );
	Stream.Write( &nTargetBoneIndex, sizeof(int) );

	CDnProjectile::TargetTypeEnum eTargetType = pProjectile->GetTargetType();
	if( eTargetType == CDnProjectile::TargetPosition ||
		eTargetType == CDnProjectile::Target ||
		eTargetType == CDnProjectile::DestPosition ||
		eTargetType == CDnProjectile::Shooter )
	{
		EtVector3 vTargetPos = *pProjectile->GetTargetPosition();
		DWORD dwTargetUniqueID = pProjectile->GetTargetActor() ? pProjectile->GetTargetActor()->GetUniqueID() : (DWORD)-1;
		Stream.Write( &vTargetPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
		Stream.Write( &dwTargetUniqueID, sizeof(DWORD) );
	}

	float fSpeed = pProjectile->GetSpeed();
	Stream.Write( &fSpeed, sizeof(float) );

#ifdef PRE_MOD_PROJECTILE_HACK
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
#endif

#if defined(PRE_FIX_52329)
	int nIgnoreHitType = pProjectile->GetIgnoreHitType();
	Stream.Write( &nIgnoreHitType, sizeof(nIgnoreHitType) );
#endif

	pBotActor->Send( eActor::SC_PROJECTILE, &Stream );
}

bool CDnPlayerActor::IsAIBotActor() const
{
	return ( m_pSession && m_pSession->bIsAIBot() );
}

void CDnPlayerActor::OnAIBotStageStart()
{
	if( !IsAIBotActor() ) return;
	if( !m_pAIBotController )
		m_pAIBotController = new CDnAIBotController( this );
	m_pAIBotController->OnStageStart();
}
