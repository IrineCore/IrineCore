#pragma once

#include "DnActor.h"
#include "DnPlayerState.h"

// include Method
#include "MAPartsBody.h"
#include "MAWalkMovement.h"
#include "MACP.h"
#include "MACP_Renewal.h"
#include "DNGameRoom.h"
#include "DnSkillTreeSystem.h"

#include "DnDropItem.h"
#include "DnActionSpecificInfo.h"
#include "MAPlateUser.h"
#if defined(PRE_ADD_TALISMAN_SYSTEM)
#include "MATalismanUser.h"
#endif

#include "DnVehicleActor.h"
#include "DnActorClassDefine.h"

#include "DnObservable.h"

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
#include "FrameSync.h"
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE
#ifdef PRE_FIX_MEMOPT_SIGNALH
#include "DnCommonUtil.h"
#endif


namespace BubbleSystem
{
class CDnBubbleSystem;
}

// Current weapon count is 2 (main weapon / sub weapon).
const int NUM_WEAPON = 2;


class CDNUserSession;
class IDnPlayerChecker;
class CDnVehicleActor;
class CDnAIBotController;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
class CDnTotalLevelSkillSystem;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

class CDnPlayerActor : public CDnActor, 
#if defined( PRE_ADD_CP_RENEWAL )
					   public MACP_Renewal,
#else	// #if defined( PRE_ADD_CP_RENEWAL )
					   public MACP,
#endif	// #if defined( PRE_ADD_CP_RENEWAL )
					   public CDnPlayerState,
					   virtual public MAPartsBody, 
					   public CDnObservable,
#if defined(PRE_ADD_TALISMAN_SYSTEM)
					   virtual public MATalismanUser,
#endif
					   virtual public MAPlateUser 
{
public:

	CDnPlayerActor( CMultiRoom *pRoom, int nClassID );
	virtual ~CDnPlayerActor();

	virtual MAMovementBase* CreateMovement();

private:

	DWORD m_dwSyncDatumTick;
	DWORD m_dwSyncDatumSendTick;
	UINT m_nVoiceChannelID;

	IDnPlayerChecker* m_pPlayerSpeedHackChecker;
	IDnPlayerChecker* m_pPlayerDoNotEnterChecker;
	IDnPlayerChecker* m_pPlayerPickupChecker;
	IDnPlayerChecker* m_pPlayerSkillChecker;
	IDnPlayerChecker* m_pPlayerActionChecker;

#ifdef _USE_VOICECHAT
	int m_nVoiceRotate;
#endif
	bool _bIsMasterSystemDurabilityReward();

protected:
	std::string m_szMaintenanceBoneName;
	std::string m_szActionBoneName;
	char m_cMovePushKeyFlag;
	bool m_bBattleMode;
	int m_nWorldLevel;

	int m_nComboDelay;
	int m_nComboCount;
	int m_nTotalComboCount;

	float m_fRecoverySPDelta;
	static float s_fRecoverySPTime;

	LOCAL_TIME m_nFatigueTick;

	CDNUserSession * m_pSession;
	CDNGameRoom::PartyStruct *m_pPartyData;

	bool m_bCompleteCutScene;
	bool m_bCheckCompleteCutScene;
	bool m_bSkipCutScene;
	bool m_bGhost;
	UINT m_uiStateBlowProcessAfterBit;

	DnWeaponHandle m_hCashWeapon[2];
	bool m_bCashSelfDeleteWeapon[2];
	bool m_bWeaponViewOrder[2];

	// Total weapon skill count is parts type count + weapon count.
	// Cash items require more server DB storage, so it will be requested later if needed.
	// Currently only crests use equipment skills.
	float			m_afLastEquipItemSkillDelayTime;
	float			m_afLastEquipItemSkillRemainTime;
	DnSkillHandle	m_ahEquipSkill;

	// Skills used by the bubble system for enhancement must be created and added directly by the player.
	map<int, DnSkillHandle> m_mapEnchantSkillFromBubble;

	// Skill tree related. Add only skills actually acquired at level 1 or higher to MASkillUser.
	// (On the client, SkillTask handles all individual skill tree/list processing.)
	vector<int> m_vlUnlockZeroLevelSkills;		// A skill in level 0 unlocked-only state.
	vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO> m_vlPossessedSkill;

	// OnChangeAction() updates projectile/action info for hack check, but if packets or server frames lag
	// When CS_CMDACTION and projectile packets are processed simultaneously, before OnChangeAction() is called and info is updated
	// Firing a projectile would be flagged as a hack on the server, so update immediately upon receiving the CS_CMDACTION packet and set this flag 
	// Turn on. OnChangeAction() performs updates only when this flag is off. 
	// As a rule, this should only update once per same action. (#21773)
	bool m_bUpdatedProjectileInfoFromCmdAction;

	int m_iNowMaxProjectileCount;				// Maximum number of projectiles that can be fired in the current action. Resets to the new action's value when the action changes.
	int m_iReservedProjectileCount;				// Number of projectiles secured regardless of action change (when a projectile fires another projectile).
	multiset<int> m_setWeaponIDUsingProjectileSignal;	// Collection of weapon indices used in projectile signals, including projectiles fired from weapons.
	deque<int> m_dqProjectileSignalOffset;		// Get the frame where the projectile signal is placed in the action currently performed by the player actor.
	DWORD m_dwLastBasicShootActionTime;			// Time when the last normal attack shoot action was performed.
	DWORD m_dwLastBasicShootCoolTime;			// Cooldown of the last updated normal attack shoot action.
	float m_fFrameSpeed;						// Current animation frame playback speed. Default is 1.0. Used when factoring in action cooldown.

	// Check projectile firing interval. Normally true, but if the player's frames briefly freeze a few times due to super armor when hit
	// Server frame sync can drift beyond a threshold and fail the Term check, so skip the check in this case. (#21133)
	bool m_bCheckProjectileSignalTerm;

	// To trigger passive skills prefixed with Tumble_, the character must be performing a Move_-type action
	// On game server, CmdStop() runs and transitions to Stand afterward so it fails. Chain the action index directly here instead.
	// If Tumble_Front arrived, the character should be doing Move_Front action, so collect action indices.
	map<int, int> m_mapTumbleHelper;
	DWORD m_dwLastChargeShootTime;				// Charge Shot works differently from the existing action system. Measure Charge Shot action cooldown separately.

	bool m_bPlayerCannonMode;
	DnActorHandle m_hCannonMonsterActor;

	int m_nSwapSingleSkinActorID;
	int m_nMonsterMutationTableID;
	EtAniObjectHandle m_hSwapOriginalHandle;
	CDnActionBase *m_pSwapOriginalAction;

	LOCAL_TIME m_LastEscapeTime;
	int m_nDeathCount;		// Death count per stage.. reset on move

	BubbleSystem::CDnBubbleSystem* m_pBubbleSystem;

	map<DWORD, bool> m_mapIcyFractionHitted;

	// #26902 State of temporarily added skill and temporarily added job ID.
	bool m_bTempSkillAdded;
	int m_iTempChangedJob;	
	//////////////////////////////////////////////////////////////////////////

	bool m_bForceEnableRideByTrigger;
	UINT m_nVehicleEffectIndex;
	bool m_bVehicleMode;
	DnActorHandle m_hVehicleActor;

	std::vector<int> m_vecEventEffectList;
	bool m_bDeletedPetSkill[Pet::Skill::Max];

	int m_nAllowedSkill;    
	bool m_bRefreshTransformMode; 
	bool m_bTransformMode;
	bool m_bShootMode; 
	std::vector<int> m_vecTransformSkillList; // Added skill list for monster transformation
	std::string m_strTransformEndAction;

#ifdef PRE_ADD_MONSTER_CATCH
	DnActorHandle m_hCatcherMonster;		// Monster grabbing me.
#endif // #ifdef PRE_ADD_MONSTER_CATCH
	int m_nInvalidPlayerCheckCounter;

	// #32426 Summon control feature - summons that need to follow when moving stage or zone (CmdWarp)
	// Monster objects summoned by this player.
	struct S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO
	{
		// Since monster objects are deleted on stage transition, copy and hold the data separately 
		// When moving to a new stage, recreate the object, copy its State, and then handle the object handle
		// Pass it to the newly created monster object to preserve data validity.
		DnMonsterActorHandle hMonster;
		DWORD dwMonsterClassID;
		SummonMonsterStruct SummonMonsterSignalData;
		LOCAL_TIME iRemainDestroyTime;

		// Set a flag on monsters that finished respawning on stage transition so even if a respawn request for the same MonsterClassID arrives
		// Compare with this flag to distinguish processed monster objects.
		bool bReCreatedFollowStageMonster;

		S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO( void ) : iRemainDestroyTime( 0 ), dwMonsterClassID( 0 ), bReCreatedFollowStageMonster( false ) {};
#ifdef PRE_FIX_MEMOPT_SIGNALH
		S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO( const S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO& rhs )
		{
			hMonster = rhs.hMonster;
			dwMonsterClassID = rhs.dwMonsterClassID;
			CopyShallow_SummonMonsterStruct(SummonMonsterSignalData, &rhs.SummonMonsterSignalData);
			iRemainDestroyTime = rhs.iRemainDestroyTime;
		}
#endif
	};
	std::list<S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO> m_listSummonedMonstersFollowStageInfos;

	LOCAL_TIME m_MixedActionTimeStamp;

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
	// For game server performance, limit CDnSkill::Process() calls per player skill list so it is not called every 20 frames.
	CFrameSkip	m_FrameSkipCallSkillProcess;
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

#ifdef PRE_ADD_COSTUME_SKILL
	int m_nCostumeSkillID;
#endif

protected:
	// Movement Message
	virtual void OnDrop( float fCurVelocity );
	virtual void OnStop( EtVector3 &vPosition );
	virtual void OnFall( float fCurVelocity );

	// Actor Message
	virtual void OnStateBlowProcessAfter();
	virtual void OnAddStateBlowProcessAfterType( eStateBlowAfterProcessType Type );
	virtual void OnDie( DnActorHandle hHitter );
	virtual void ProcessDie( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnHitSuccess( LOCAL_TIME LocalTime, DnActorHandle hActor, HitStruct *pStruct );
	virtual void OnHitFinish( LOCAL_TIME LocalTime, HitStruct *pStruct );
	virtual void OnComboFinish( int nCombo );
	virtual void OnHitProjectile( LOCAL_TIME LocalTime, DnActorHandle hHittedTarget, const CDnDamageBase::SHitParam& HitParam );

	virtual void	OnBreakSkillSuperAmmor( int nIndex, int nOriginalSupperAmmor, int nDescreaseSupperAmmor );
	virtual void	OnAirCombo( int nComboCount );
	virtual int		OnGetJobClassID(){ return CDnPlayerState::GetJobClassID(); }

	virtual void PushSummonMonster( DnMonsterActorHandle hMonster, const SummonMonsterStruct* pSummonMonsterStruct, bool bReCreateFollowStageMonster = false );

	// CDnActorState
	virtual void OnLevelUp( int nLevel, int nLevelUpAmount );
	virtual void OnAddExperience( int nAddExperience, int nLogCode, INT64 biFKey );	// -> Also set biFKey according to nLogCode (see eCharacterExpChangeCode)

	// MASkillUser
	virtual bool ExecuteSkill( DnSkillHandle hSkill, LOCAL_TIME LocalTime, float fDelta );

	void ProcessCombo( LOCAL_TIME LocalTime, float fDelta );
	void ProcessRecoverySP( LOCAL_TIME LocalTime, float fDelta );

	void OnFinishProcessDie();

	void OnChangeAction( const char* szPrevAction );

	void RefreshWeaponViewOrder( int nEquipIndex );

	void LinkCashWeapon( int nEquipIndex );

	virtual float PreCalcDamage( CDnDamageBase *pHitter, SHitParam &HitParam, const float fDefenseRate=1.f, float fStateEffectAttackM = -1.0f );

	// CP
	virtual void OnEventCP( CPTypeEnum Type, int nResult );
	void UpdateAttackedCPPoint( CDnDamageBase *pHitter , CDnWeapon::HitTypeEnum eHitType );

	void _UpdateMaxProjectileCount( int nActionIndex, bool bUpdateReservedCount = false );
	void _CheckActionWithProcessPassiveActionSkill( const char* szPrevAction );
	void _CheckProcessSkillActioncChange( const char* pAction );

public:
	virtual bool Initialize();

	// Actor Message
	virtual bool SetActionQueue( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f, bool bCheck = true, bool bCheckStateEffect = true );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );

	void OrderUseSkillToMySummonedMonster( OrderMySummonedMonsterStruct* pStruct );

	virtual void OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct );
	virtual void SyncClassTime( LOCAL_TIME LocalTime );
	virtual void ResetActor();
	virtual void GetBoundingSphere( SSphere &Sphere, bool bActorSize = false );
	virtual void OnBeginStateBlow( DnBlowHandle hBlow );

	// Quest Callback Message
	virtual void OnKillMonster(DnActorHandle hMonster);

	//MAPlateUser
	virtual bool AttachGlyph( DnGlyphHandle hGlyph, CDnGlyph::GlyphSlotEnum Index, bool bDelete = false );
	virtual bool DetachGlyph( CDnGlyph::GlyphSlotEnum Index );

	// Actor Command
	virtual void CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f );
	virtual void CmdStop( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f );
	virtual void CmdPassiveSkillAction( int nSkillID, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.0f, bool bChargeKey = false, bool bCheckOverlapAction = true, bool bOnlyCheck = false );
	virtual int CmdAddStateEffect( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam, bool bOnPlayerInit = false, bool bCheckCanBegin = true, bool bEternity = false );
	virtual void CmdRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex, bool bRemoveFromServerToo = true );
	virtual void SendRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex );
	virtual void SendRemoveStateEffectGraphic( STATE_BLOW::emBLOW_INDEX emBlowIndex );
	virtual void CmdToggleBattle( bool bBattleMode );

	bool CheckSkillAction( const char *szActionName );

	virtual void CmdAddExperience( TExpData &ExpData, int nLogCode, INT64 biFKey );	// -> Also set biFKey according to nLogCode (see eCharacterExpChangeCode)
	virtual void CmdAddCoin( INT64 nCoin, int nLogCode, int nFKey, bool bSync = true );

	virtual void CmdToggleWeaponViewOrder( int nEquipIndex, bool bShowCash );
	virtual void CmdTogglePartsViewOrder( int nEquipIndex, bool bShowCash );
	virtual void CmdToggleHideHelmet( bool bHideHelmet );
	virtual void CmdChangeJob( int nJobID );
	virtual void CmdPickupItem( PickupItemStruct* pStruct, DnDropItemHandle hDropItem=CDnDropItem::Identity() );
	virtual void CmdEscape( EtVector3 &vPos );
	virtual void CmdWarp( const EtVector3 &vPos, const EtVector2 &vLook, CDNUserSession* pGameSession=NULL, bool bCheckPlayerFollowSummonedMonster=false );

	int GetLevelUpSkillPoint( int nPrevLevel, int nCurLevel );

	// TransAction Message
	virtual void OnDispatchMessage( CDNUserSession *pSession, DWORD dwActorProtocol, BYTE *pPacket );

	// MASkillUser Message
	// Skills that are only unlocked and not acquired are held midway instead of being added to MASkillUser.
	// Declared up to iSkillLevelApplyType to preserve virtual inheritance of MASkillUser::AddSkill, though the actual value is determined by
	// The overridden AddSkill in this class determines pvp/pve and calls MASkillUser::AddSkill().
	virtual bool CanAddSkill( int nSkillTableID, int nLevel = 1 );
	virtual bool AddSkill( int nSkillTableID, int nLevel = 1, int iSkillLevelApplyType = CDnSkill::PVE );
	virtual void OnAddSkill( DnSkillHandle hSkill, bool isInitialize = false );
	virtual void OnRemoveSkill( DnSkillHandle hSkill );

	virtual void OnApplyPassiveSkillBlow( int iBlowID );
	void CheckAndRegisterObserverStateBlow( DnBlowHandle hBlow );

	void SaveUserData(TUserData &UserData);

	void SetUserSession(CDNUserSession * pSession);
	CDNUserSession * GetUserSession() { return m_pSession; }
	CDNGameRoom::PartyStruct *GetPartyData() { return m_pPartyData; }

	bool IsLocalActor() { return false; }
	virtual bool IsDie();

	void SetBattleMode( bool bEnable );
	bool IsBattleMode() { return m_bBattleMode; }
	bool IsCanBattleMode();
	void OnBattleToggle( bool bBattle );

	virtual bool IsPlayerActor() { return true; }
	virtual bool IsGMTrace() const;
	virtual UINT GetSessionID();
	virtual int GetMoveSpeed();

	INT64	GetCharacterDBID();
	int GetComboCount() { return m_nComboCount; }
	int GetComboDelay() { return m_nComboDelay; }
	int GetTotalComboCount() { return m_nTotalComboCount; }
	void ResetTotalComboCount() { m_nTotalComboCount = 0; }

	void OnRepairEquipDurability( bool bDBSave, INT64 nPriceCoin );
	void OnDecreaseEquipDurability( int nValue, bool bDBSave );
	void OnDecreaseEquipDurability( float fValue, bool bDBSave );
	void OnDecreaseInvenDurability( int nValue, bool bDBSave );
	void OnDecreaseInvenDurability( float fValue, bool bDBSave );
	bool OnStageGiveUp();
	bool IsPenaltyStageGiveUp();

	bool IsCompleteCutScene() { return m_bCompleteCutScene; }
	void SetCompleteCutScene( bool bValue ) { m_bCompleteCutScene = bValue; }
	void SetCheckCompleteCutScene( bool bValue ){ m_bCheckCompleteCutScene = bValue; }
	bool IsCheckCompleteCutScene(){ return m_bCheckCompleteCutScene; }

	bool IsSkipCutScene() { return m_bSkipCutScene; }
	void SetSkipCutScene( bool bValue ) { m_bSkipCutScene = bValue; }

	void ToggleGhostMode( bool bGhost );
	bool IsGhost() { return m_bGhost; }
	void SwapSingleSkin( int nChangeActorTableID);
	bool IsSwapSingleSkin(){ return ( m_nSwapSingleSkinActorID == -1 ) ? false : true; }
	int GetSwapSingleSkinActorID() { return m_nSwapSingleSkinActorID; }
	int GetMonsterMutationTableID() { return m_nMonsterMutationTableID; }

	void RecvPartyRefreshGateInfo( const EtVector3& Pos );


	virtual DnWeaponHandle GetActiveWeapon( int nEquipIndex );
	// Add skill if weapon/accessory has a skill. Remove skill when unequipped.
	virtual bool AttachParts( DnPartsHandle hParts, CDnParts::PartsTypeEnum Index, bool bDelete = false );
	virtual bool DetachParts( CDnParts::PartsTypeEnum Index );
	virtual bool AttachCashParts( DnPartsHandle hParts, CDnParts::PartsTypeEnum Index, bool bDelete = false );
	virtual bool DetachCashParts( CDnParts::PartsTypeEnum Index );

	virtual void AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false );
	virtual void DetachWeapon( int nEquipIndex = 0 );

	// This is the local one.
	virtual void AttachCashWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false );
	virtual void DetachCashWeapon( int nEquipIndex = 0 );
	virtual DnWeaponHandle GetCashWeapon( int nEquipIndex = 0 ) { return m_hCashWeapon[nEquipIndex]; }
	void ShowCashWeapon( int nEquipIndex, bool bShow );
	void SetWeaponViewOrder( int nEquipIndex, bool bShowCash );

	// Related to speed hack
	void	SetSyncDatumTick(){ m_dwSyncDatumTick = GetTickCount(); }
	void	SetSyncDatumSendTick(){ m_dwSyncDatumSendTick = GetTickCount(); }
	DWORD	GetSyncDatumGap() const { return GetTickCount()-m_dwSyncDatumTick; }
	DWORD	GetSyncDatumSendTick() const { return m_dwSyncDatumSendTick; }

	// Skill tree related
	vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO>& GetPossessedSkillInfo( void ) { return m_vlPossessedSkill; };
	void UnLockSkill( int nSkillID, INT64 nUnlockPrice=0 );
	// Since unlocking and acquiring via skill book is now handled together, if unlocked
	// Check if conditions to acquire the skill are met. Called before actual unlock to verify if acquisition is also possible upon unlock.
	int CanAcquireSkillIfUnlock( int nSkillID );
	int AcquireSkill( int nSkillID );

	void AddNowMaxProjectileCount( int nAddCount ) { m_iNowMaxProjectileCount += nAddCount; };

	IDnPlayerChecker* GetPlayerSpeedHackChecker( void ) { return m_pPlayerSpeedHackChecker; };
#ifdef IRINE_MOD_MAP_WARP
	void ResetWarpAntiCheatCheckers();
#endif
	bool IsInvalidPlayerChecker();
	virtual void OnInvalidPlayerChecker( int nValue );

	// Projectile anti-hack count check series.
	void AddReservedProjectileCount( void ) { ++m_iReservedProjectileCount; };
	bool UseAndCheckAvailProjectileCount( void );
	//////////////////////////////////////////////////////////////////////////

	// Check weapon table ID used in projectile signal.
	bool CheckAndEraseWeaponIDUsingProjectileSignal( int iWeaponTableID )
	{
		bool bResult = false;
		if( false == m_setWeaponIDUsingProjectileSignal.empty() )
		{
			multiset<int>::iterator iter = m_setWeaponIDUsingProjectileSignal.find( iWeaponTableID );
			if( m_setWeaponIDUsingProjectileSignal.end() != iter )
			{
				// Calling erase by value removes all matching values, so erase only one.
				m_setWeaponIDUsingProjectileSignal.erase( iter );
				bResult = true;
			}
		}

		return bResult;
	}

	void OnChangePlaySpeed( DWORD dwFrame, float fSpeed );

	bool CheckProjectileSignalFrameTerm( void )
	{
		// The frame of the projectile fired in the character's fire action at the time the current projectile packet arrived and
		// Compare the frame of the action the character is currently performing on the game server.
		// If there is no projectile signal frame smaller than the character's frame, treat as hack.
		// Only normal attack actions excluding skill actions need to be checked...
		if( m_dqProjectileSignalOffset.empty() )
			return false;

		// If currently in split upper/lower body action
		float fCurrentFrame = 0.0f;
		if( IsCustomAction() )
		{
			fCurrentFrame = GetCustomActionCurrentFrame();
		}
		else
		{
			fCurrentFrame = CDnActionBase::m_fFrame;
		}

		// If there is no frame less than or equal to this, this projectile packet cannot arrive at the current frame.
		// Since it is sorted, only need to compare the first element.
		// Since the server runs at 20 FPS, allow a tolerance of +6 frames.
		int iSignalOffset = int((float)m_dqProjectileSignalOffset.front()/* / m_fFrameSpeed*/);		// Since it is frame-based, if it slows down the frame advances slower too, so compare directly.

		if( m_bCheckProjectileSignalTerm )
		{
			// Account for potentially larger error when frame speed increases.
			if( fCurrentFrame+(8.0f*m_fFrameSpeed) < iSignalOffset )
			{
				// TODO: It's a hack! Wish we could disconnect them..
				return false;
			}
		}
		
		// Subtract one shot since firing was processed.
		m_dqProjectileSignalOffset.pop_front();

		return true;
	}

	bool CheckSkipAirCondition(int iSkill); // When currently in Air, a perception gap between server and client can cause GroundMovable check to fail.
	void ReplacementGlyph( DnSkillHandle hNewSkill );
	bool IsExclusiveSkill( int iSkillID, int iExclusiveID );
	int GetAvailSkillPointByJob( int iSkillID );
	int GetUsedSkillPointInThisJob( const int nJobID );

	// Notify client that cooldown parrying succeeded.
	void RequestCooltimeParrySuccess( int iSkillID );

	// Process any item prefix skill effects that should trigger when hit by a normal attack.
	void CheckNormalHitSEProcessor( DnActorHandle hHittedTarget, const CDnDamageBase::SHitParam& HitParam );

	void AddStageDeathCount() { ++m_nDeathCount; };
	void ClearStageDeathCount() { m_nDeathCount = 0;};
	int GetStageDeathCount() { return m_nDeathCount;};

	void InitializeEnchantPassiveSkills( void );
	// from MASkillUser
	virtual void OnReplacementSkill( DnSkillHandle hLegacySkill, DnSkillHandle hNewSkill );
	// Check if an enhancement skill matching the skill ID is owned, and apply it if so.
	void CheckAndApplyEnchantPassiveSkill( DnSkillHandle hBaseSkill );

	// Currently only Elemental Lord's Icy Fraction exceptionally filters 2 state effects
	// Want only one of the two to be hit, so prevent the first checked one from being caught in the next filtering. #28747
	// Inherited from CDnPlayerActor.
	virtual void OnHitSignalStateEffectFilterException( DWORD dwTargetActorUniqueID, int iBlowIndex );
	virtual bool CheckHitSignalStateEffectFilterException( DWORD dwTargetActorUniqueID, int iBlowIndex );

	virtual bool IsMyRelicMonster( DnActorHandle hActor );

	BubbleSystem::CDnBubbleSystem* GetBubbleSystem( void ) { return m_pBubbleSystem; };
	void RemoveAllBubbles( bool bRemoveEvent );

	// #26902
	// Function called by quest API to make 2nd job advancement skills temporarily usable for trial.
	// Set usable class and usable level to match the current character's level.
	// Temporarily change job only on client side to show skill tree.
	bool IsTempSkillAdded( void ) { return m_bTempSkillAdded; };
	bool CanChangeJob( int iJobID );
	void SendTempJobChange( int iJobID );
	void EndAddTempSkillAndSendRestoreTempJobChange( void );
	void AddTempSkill( int iSkillID );
	void RemoveAllTempSkill( void );

	bool IsCannonMode( void ) { return (m_hCannonMonsterActor && m_bPlayerCannonMode); };
	void OnCannonMonsterDie(void );
	void EndCannonMode();

	// If the mount effect itself ever has stats or affects gameplay, we will need to add it, but since that is not the case right now, only store the item index for now. 
	void SetVehicleEffectIndex(UINT nIndex){m_nVehicleEffectIndex = nIndex;}
	bool IsCanVehicleMode();
	bool IsCanPetMode();
	void RideVehicle(TVehicle *pInfo);
	void UnRideVehicle(bool bForce=true);
	void ForceUnRideVehicle();

	bool IsVehicleMode(){return  m_bVehicleMode;}
	void SetVehicleMode(bool bVehicle){m_bVehicleMode = bVehicle;}
	CDnVehicleActor *GetMyVehicleActor();
	void SetMyVehicleActor(DnActorHandle hVehicleActor){m_hVehicleActor = hVehicleActor;}
	void RemoveVehicleStateEffectImmediately(int nBlowIndex );

	void SetForceEnableRide( const bool bForceEnableRide );

	// If Process needs to be run on the server for Pet / Vehicle, add it inside this function.
	// Client is managed as a Task.

	void ProcessCompanion( LOCAL_TIME LocalTime, float fDelta ); 

	void ApplyEventStateBlow();
	void RemoveEventStateBlow();

	void CheckPetSatietyPercent();
	bool IsTransformMode(){return m_bTransformMode;}
	bool IsTransformSkill( int nSkillID );

	virtual void RefreshTransformMode();
	virtual void ToggleTransformMode(bool bTrue, int nMonsterMutationTableID=-1, bool bForce = false, const char* strEndAction = "Stand" );
	virtual void CmdShootMode(bool bTrue);
	virtual const char *GetChangeShootActionName(const char *szActionName);
	virtual void ProcessNonLocalShootModeAction();

	bool IsSpectatorMode();
	virtual bool OnApplySpectator(bool bEnable);

#ifdef PRE_ADD_MONSTER_CATCH
	void SetCatcherMonsterActor( DnActorHandle hActor ) { m_hCatcherMonster = hActor; };
	void ReleaseCatcherMonsterActor( void ) { m_hCatcherMonster.Identity(); };
	bool IsCatchedByMonster( void ) { return m_hCatcherMonster ? true : false; };
#endif // #ifdef PRE_ADD_MONSTER_CATCH

	void OnInitializeNextStageFinished( void );
	void OnDieSummonedMonster( DnMonsterActorHandle hSummonedMonster );
	void OnBeforeDestroyStageMonsters( void );

	bool HasSameGlobalIDSkill( int iSkillID );

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
	virtual bool IsAllowCallSkillProcess( float fDelta );
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

	void UpdateMissionByMonsterKillCount(int nCount);

	void ApplyEnchantSkillOnceFromBubble( int iTargetSkillID, int iEnchantSkillID );

	//#45680
	//Level-up in the skill tree window is handled in DnSkillTask, however
	//Missing crest effect application on the server when equipping/unequipping skill level-up items.
	virtual void ChangeSkillLevelUp(int nSkillID, int nOrigLevel);

	void MakeEquipAndPassiveState( CDnState &State );

#ifdef PRE_ADD_COSTUME_SKILL
	void RefreshCostumeSkill( int nSkillIndex , int nSkilllevel);
#endif
#ifdef PRE_ADD_VEHICLE_SPECIAL_ACTION
	void ReportInvalidAction();
#endif

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
protected:
	CDnTotalLevelSkillSystem* m_pTotalLevelSkillSystem;

public:
	CDnTotalLevelSkillSystem* GetTotalLevelSkillSystem() { return m_pTotalLevelSkillSystem; }

	virtual void UpdateTotalLevel(int nLevel);
	virtual void UpdateTotalLevelByCharLevel();
	virtual void AddTotalLevelSkill(int nSlotIndex, int nSkillID, bool isInitialize = false);
	virtual void RemoveTotalLevelSkill(int nSlotIndex);
	virtual void ActivateTotalLevelSkillSlot(int nSlotIndex, bool bActivate);
	virtual void ActivateTotalLevelSkillCashSlot(int nSlotIndex, bool bActivate, __time64_t tExpireDate);

	virtual void RequestAddTotalLevelSkill(int nSlotIndex, int nSkillID) {};
	virtual void RequestRemoveTotalLevelSkill(int nSlotIndex) {};

	virtual void OnLevelChange();
#endif // PRE_ADD_TOTAL_LEVEL_SKILL


protected:
	CDnAIBotController* m_pAIBotController;

public:
#ifdef IRINE_MOD_PLAYER_BOTS
	bool IsAIBotActor() const;
	CDnAIBotController* GetAIBotController() const { return m_pAIBotController; }
	void OnAIBotStageStart();
#else
	bool IsAIBotActor() const { return false; }
	CDnAIBotController* GetAIBotController() const { return NULL; }
	void OnAIBotStageStart() {}
#endif
};
