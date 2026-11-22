#pragma once

#include "DnActor.h"
#include "DnAIBotBuildData.h"
#include <map>
#include <string>
#include <vector>

class CDnPlayerActor;
class CDnSkill;
class CDnMonsterActor;
class CDnWorldProp;
class CDnProjectile;
class CEtWorldEventArea;
struct ProjectileStruct;

class CDnAIBotController
{
public:
	CDnAIBotController( CDnPlayerActor* pActor );
	~CDnAIBotController();

	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnStageStart();

	// Live combat & learning feedback hooks from CDnPlayerActor / CDnGameTask
	void OnDamaged( DnActorHandle hHitter, INT64 nDamage );
	void OnAttackHitTarget( DnActorHandle hTarget, bool bFromProjectile );
	static void ObserveBossHitOnActor( CDnActor* pHitter, CDnActor* pVictim, INT64 nDamage );
	static void ObserveHumanAttackTarget( CDNGameRoom* pRoom, DnActorHandle hTarget );
	static bool HandlePartyChatCommand( CDNGameRoom* pRoom, CDNUserSession* pSender, const WCHAR* wszMsg );

	// Strategy engine accessors (composable combat & non-combat flags)
	uint32_t GetCombatStrategies() const { return m_uCombatStrategies; }
	uint32_t GetNonCombatStrategies() const { return m_uNonCombatStrategies; }
	void SetCombatStrategies( uint32_t uMask ) { m_uCombatStrategies = uMask; }
	void SetNonCombatStrategies( uint32_t uMask ) { m_uNonCombatStrategies = uMask; }
	void ResetStrategiesToDefaults();
	void RequestImmediateBuff() { m_fBuffCheckTimer = 0.f; }
	void RequestImmediateHeal() { m_fDecisionTimer = 0.f; m_fSkillDelayTimer = 0.f; }
	void RequestAssistEngage( DnActorHandle hTarget );

	DnActorHandle GetTargetActor() const { return m_hTargetActor; }
	const EtVector3& GetAimPosition() const { return m_vAimPos; }

	static void SendBotProjectilePacket( CDnPlayerActor* pBotActor, CDnProjectile* pProjectile, ProjectileStruct* pStruct, int nActionIndex, int nSignalIndex );

private:
	enum MoveCommandState
	{
		MoveState_None = 0,
		MoveState_Moving,
		MoveState_Stopped,
	};

	enum EncounterMode
	{
		Mode_Dungeon = 0,
		Mode_Nest,
		Mode_DragonNest,
		Mode_DarkLair,
	};

	enum ObstacleType
	{
		Obstacle_None = 0,
		Obstacle_SolidProp,
		Obstacle_BreakableProp,
		Obstacle_WorldCollision,
		Obstacle_NavEdge,
		Obstacle_SteepSlope,
		Obstacle_Trap,
	};

	enum BossThreatKind
	{
		Threat_None = 0,
		Threat_StompJump,
		Threat_FrontalCleave,
		Threat_PointBlankAoE,
		Threat_DonutOuter,
		Threat_RoomWipe,
		Threat_Projectile,
	};

	struct PathObstacle
	{
		ObstacleType eType;
		DnPropHandle hProp;
		EtVector3 vHitNormal;
		float fHitDist;

		PathObstacle()
			: eType( Obstacle_None )
			, vHitNormal( 0.f, 0.f, 0.f )
			, fHitDist( FLT_MAX )
		{}
	};

	struct BossThreatInfo
	{
		BossThreatKind eKind;
		DnActorHandle hBoss;
		EtVector3 vBossPos;
		EtVector3 vBossForward;
		EtVector3 vHitOrigin;
		float fTimeToImpact;
		float fSafeMinDist;
		float fSafeMaxDist;
		float fThreatDamageRatio;
		bool bBotCurrentlyInDanger;

		BossThreatInfo()
			: eKind( Threat_None )
			, vBossPos( 0.f, 0.f, 0.f )
			, vBossForward( 0.f, 0.f, 1.f )
			, vHitOrigin( 0.f, 0.f, 0.f )
			, fTimeToImpact( 99.f )
			, fSafeMinDist( 0.f )
			, fSafeMaxDist( 0.f )
			, fThreatDamageRatio( 0.f )
			, bBotCurrentlyInDanger( false )
		{}
	};

	struct SkillProfile
	{
		int nSkillID;
		float fMinRange;
		float fMaxRange;
		float fOptimalRange;
		float fMaxVerticalReach;
		float fDashDistance;
		float fTotalDamageProb;
		bool bHasHitSignal;
		bool bHasProjectile;
		bool bHasIFrame;
		bool bIsAoE;
		bool bIsDashForward;
		bool bIsBackstep;
		bool bIsSummon;
		bool bIsUltimate;
		bool bIsBurstBuff;
		bool bIsDebuffOrArmorBreak;
		bool bAnalyzed;

		SkillProfile()
			: nSkillID( 0 )
			, fMinRange( 0.f )
			, fMaxRange( 160.f )
			, fOptimalRange( 110.f )
			, fMaxVerticalReach( 140.f )
			, fDashDistance( 0.f )
			, fTotalDamageProb( 1.0f )
			, bHasHitSignal( false )
			, bHasProjectile( false )
			, bHasIFrame( false )
			, bIsAoE( false )
			, bIsDashForward( false )
			, bIsBackstep( false )
			, bIsSummon( false )
			, bIsUltimate( false )
			, bIsBurstBuff( false )
			, bIsDebuffOrArmorBreak( false )
			, bAnalyzed( false )
		{}
	};

	struct ArenaScanResult
	{
		DnActorHandle hBestEnemy;
		DnActorHandle hPriorityBoss;
		DnActorHandle hEnemyAttackingHuman;
		DnActorHandle hEnemyAttackingHealer;
		DnActorHandle hHumanTarget;
		DnActorHandle hLowestHpAlly;
		float fLowestHpRatio;
		float fMyHpRatio;
		float fMyManaRatio;
		DnActorHandle hDebuffedAlly;
		DnActorHandle hHumanLeader;
		DnPropHandle hSafeBuffProp;
		DnPropHandle hBreakableProp;
		DnPropHandle hBlockingBreakableProp;
		EtVector3 vDangerAvoidVec;
		EtVector3 vAllyRepulsionVec;
		EtVector3 vGateTargetPos;
		int nActiveGateIndex;
		int nEnemiesNearTarget;
		int nAliveEnemyCount;
		float fBossHpRatio;
		bool bHumanInGate;
		bool bHumanApproachingGate;
		bool bTargetHasLOS;
		bool bHasTrapDanger;
		bool bBossCastingNearMe;
		bool bBossInvulnerable;
		bool bHasAnyEnemy;
		bool bIsNestEncounter;
		bool bBurstWindowActive;
		bool bHumanInDanger;
		bool bBotHasBossAggro;
		bool bHasLiveTankAlly;
		bool bSelfInCriticalHp;
		BossThreatInfo BossThreat;

		ArenaScanResult()
			: fLowestHpRatio( 1.0f )
			, fMyHpRatio( 1.0f )
			, fMyManaRatio( 1.0f )
			, vDangerAvoidVec( 0.f, 0.f, 0.f )
			, vAllyRepulsionVec( 0.f, 0.f, 0.f )
			, vGateTargetPos( 0.f, 0.f, 0.f )
			, nActiveGateIndex( -1 )
			, nEnemiesNearTarget( 0 )
			, nAliveEnemyCount( 0 )
			, fBossHpRatio( 1.0f )
			, bHumanInGate( false )
			, bHumanApproachingGate( false )
			, bTargetHasLOS( true )
			, bHasTrapDanger( false )
			, bBossCastingNearMe( false )
			, bBossInvulnerable( false )
			, bHasAnyEnemy( false )
			, bIsNestEncounter( false )
			, bBurstWindowActive( false )
			, bHumanInDanger( false )
			, bBotHasBossAggro( false )
			, bHasLiveTankAlly( false )
			, bSelfInCriticalHp( false )
		{}
	};

	CDnPlayerActor* m_pActor;
	AIBot::RoleType m_Role;
	int m_nBotSlotIndex;
	bool m_bRoleInitialized;
	EncounterMode m_eEncounterMode;
	uint32_t m_uCombatStrategies;
	uint32_t m_uNonCombatStrategies;
	bool m_bStrategiesInitialized;

	DnActorHandle m_hTargetActor;
	DnActorHandle m_hForcedAssistTarget;
	float m_fForcedAssistTimer;
	EtVector3 m_vAimPos;
	EtVector3 m_vStayAnchorPos;
	bool m_bHasStayAnchor;

	std::vector<EtVector3> m_vecWaypoints;
	size_t m_nCurWaypoint;
	EtVector3 m_vLastPathGoal;
	float m_fRepathTimer;
	float m_fMoveSyncTimer;
	EtVector3 m_vLastMoveDir;
	EtVector2 m_vLastLookDir;
	char m_cLastMoveFlag;
	std::string m_strLastMoveAction;
	MoveCommandState m_eMoveState;

	EtVector3 m_vStuckCheckPos;
	float m_fStuckCheckTimer;
	float m_fStuckAccumTime;

	float m_fScanTimer;
	float m_fDecisionTimer;
	float m_fSkillDelayTimer;
	float m_fDodgeCooldown;
	float m_fJumpCooldown;
	float m_fRecoveryCooldown;
	float m_fDashGapCooldown;
	float m_fComboChainTimer;
	float m_fBuffCheckTimer;
	int m_nBasicComboStep;
	bool m_bLoggedStageSummary;
	bool m_bLoggedFirstCombat;
	bool m_bLoggedFirstAction;

	// Humanized movement, orbit & adaptive defense state
	float m_fStrafeSign;
	float m_fStrafeSwitchTimer;
	float m_fReactionJitter;
	float m_fDefensiveBias;
	float m_fPostCombatPauseTimer;
	float m_fOutOfCombatTimer;
	bool m_bWasInCombatLastTick;
	bool m_bIsSprintDashing;
	int m_nSprintSpeedBlowID;

	// Exact skill profile cache & live hit/whiff learning tracker
	std::map<int, SkillProfile> m_mapSkillProfiles;
	int m_nPendingCastSkillID;
	float m_fPendingCastDist;
	float m_fPendingCastTimer;
	bool m_bPendingCastHit;
	int m_nPendingCastHitCount;
	DnActorHandle m_hPendingCastTarget;

	// Smart navigation, obstacle avoidance & portal tracking state
	bool m_bHasValidPath;
	bool m_bBlockedByObstacle;
	float m_fObstacleProbeTimer;
	float m_fAvoidHoldTimer;
	float m_fAvoidSideSign;
	EtVector2 m_vAvoidDir;
	float m_fGateWaitTimer;
	int m_nSyncedGateIndex;
	DnPropHandle m_hPathBlockingBreakable;

	ArenaScanResult m_LastScan;

	void EnsureRoleInitialized();
	void InitializeDefaultStrategies();
	bool HasCombatStrategy( uint32_t uFlag ) const { return ( m_uCombatStrategies & uFlag ) != 0; }
	bool HasNonCombatStrategy( uint32_t uFlag ) const { return ( m_uNonCombatStrategies & uFlag ) != 0; }
	bool IsBuffAlreadyActiveOnTarget( CDnSkill* pSkill, CDnPlayerActor* pTarget ) const;
	bool IsDebuffAlreadyActiveOnTarget( CDnSkill* pSkill, CDnActor* pTarget ) const;
	EncounterMode DetectEncounterMode() const;
	void ScanArenaAndParty( LOCAL_TIME LocalTime );
	void UpdateMovementAndPositioning( float fDelta, bool& bOutWantsToMove );
	void UpdateCombatDecision( LOCAL_TIME LocalTime, float fDelta );

	void SteerToward( const EtVector3& vDest, float fStopDist, float fDelta, bool& bOutWantsToMove, bool bAllowCombatStrafe = false, const EtVector3* pCombatFaceTarget = NULL );
	bool BuildPath( const EtVector3& vStart, const EtVector3& vGoal );
	bool HasNavMeshConnectivity( const EtVector3& vStart, const EtVector3& vEnd, int& nAStarBudget ) const;
	bool CheckSegmentObstacle( const EtVector3& vFrom, const EtVector3& vTo, float fExpandRadius, bool bForProjectileLOS, LOCAL_TIME LocalTime, PathObstacle& outObs ) const;
	bool ProbePathObstacle( const EtVector3& vFrom, const EtVector2& vDir2, float fProbeDist, bool bTestWorldCollision, LOCAL_TIME LocalTime, PathObstacle& outObs ) const;
	bool ChooseClearSteeringDir( const EtVector3& vFrom, const EtVector2& vDesiredDir, float fProbeDist, bool bTestWorldCollision, LOCAL_TIME LocalTime, EtVector2& outDir, PathObstacle& outFrontObs );
	void UpdateGateReadiness( float fDelta );
	void PerformPortalTeleport( const EtVector3& vGatePos );
	EtVector3 ComputeGateStandPoint( CEtWorldEventArea* pGateArea ) const;
	EtVector3 ComputeTargetAimPoint( CDnActor* pTarget, bool bLeadVelocity ) const;
	static float GetActorEffectiveRadius( CDnActor* pActor );

	// Exact skill signal profiling & distance/learning engine
	const SkillProfile& GetOrAnalyzeSkillProfile( CDnSkill* pSkill );
	float GetEffectiveSkillMaxRange( const SkillProfile& prof ) const;
	float GetDesiredCombatRange( float fTargetRadius, float fMyRadius ) const;
	void FinalizePendingSkillOutcome( bool bForce );
	void AnalyzeBossThreat( CDnMonsterActor* pBoss, const EtVector3& vMyPos, BossThreatInfo& outThreat ) const;

	// Human reflexes: hit recovery, combo chaining, stomp jump, directional dodge, callouts
	bool TryHandleHitRecovery();
	bool TrySkillChainFollowUp();
	bool TryReactToBossThreat();
	bool TryCastIFrameSkill( const EtVector3& vEscapeDir );
	bool TryJumpStomp();
	bool TryDirectionalTumble( const EtVector3& vEscapeDir, const EtVector3* pFaceTarget = NULL );
	void MaybeSendTacticalCallout( const WCHAR* wszMsg, float fRoomCooldownSec = 24.f );

	void SendMovePacket( const EtVector2& vDir2 );
	void SendDirectionalMovePacket( const char* szMoveAction, char cFlag, const EtVector2& vZVec, const EtVector2& vLook );
	void SendStopPacket( bool bResetStop = false );
	void SendLookPacket( const EtVector2& vDir2 );
	void SendActionPacket( const char* szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, bool bSkillChain = false );
	void SendPassiveSkillActionPacket( int nSkillID, BYTE cLevel, const char* szActionName, int nLoopCount, float fBlendFrame, float fStartFrame, const EtVector2& vZVec, const EtVector2& vLook, char cFlag, bool bChargeKey, bool bOnlyCheck = false );
	void SendUseSkillPacket( CDnSkill* pSkill );

	bool IsHumanLeaderDashing() const;
	bool IsActiveWarriorDash() const;
	bool CanUseWarriorDash() const;
	bool CanUseLevel10SprintPassive() const;
	void CancelSprintPassiveIfActive( bool bResetActionToStand = false );

	bool TryCastSupportOrHeal( bool bNeedHeal, bool bNeedCure, bool bWantBuff, float fAllyDist );
	bool TryCastCombatSkill( float fDistToTargetBox, float fVerticalDelta, bool bBossInvuln );
	bool TryBasicComboAttack( float fDistToTargetBox, float fVerticalDelta );
	bool TryBreakPropSkillOrAttack( const EtVector3& vPropPos );
	bool TryEvadeDanger();
	bool ResolveBasicAttackAction( int nComboStep, const char** pOutAction, bool* pbOutIsRanged ) const;
	bool HasRangedBasicAttack() const;
	void PerformLeashTeleport( const EtVector3& vAnchor );
	void FacePosition( const EtVector3& vTargetPos );
	void ToggleBattleState( bool bBattle );

	static bool IsHarmfulDebuffPresent( CDnPlayerActor* pAlly );
	static bool HasSkillBlow( CDnSkill* pSkill, int nBlowIndex );
	static bool IsHealSkill( CDnSkill* pSkill );
	static bool IsCureSkill( CDnSkill* pSkill );
	static bool IsPartyOrSelfBuffSkill( CDnSkill* pSkill );
	static bool IsRaidBurstBuffSkill( CDnSkill* pSkill );
	static bool IsIceStackSkill( CDnSkill* pSkill );
	static bool IsThreatSkill( CDnSkill* pSkill );
	static bool IsOffensiveActiveSkill( CDnSkill* pSkill );
	static bool IsHittableEnemy( CDnActor* pActor, DnActorHandle hHitter, int nBotTeam, LOCAL_TIME LocalTime );
	static int ScoreEnemyPriority( CDnMonsterActor* pMon, AIBot::RoleType role, float fDistSq, CDnPlayerActor* pHumanLeader, EncounterMode eMode, uint32_t uCombatStrategies, DnActorHandle hHumanTarget, CDnPlayerActor* pSelfBot );
};
