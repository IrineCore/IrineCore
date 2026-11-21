#pragma once
#include "DnBlowDef.h"
#include "DnDamageBase.h"
#include "DnBlow.h"

class RemoveStateBlowInfo
{
public:
	enum enmRSBI
	{
		RSBI_BLOW_INDEX = 0,
		RSBI_BLOW_ID,
		RSBI_SKILL_DURATION_TYPE,
		RSBI_BLOW_SERVER_ID
	};

	RemoveStateBlowInfo(enmRSBI type, int nValue)
	{
		m_Type = type;
		m_Value = nValue;
	}

	bool Compare(DnBlowHandle hBlow);
public:
	enmRSBI m_Type;
	int m_Value;
};

// State effect management class shared by server and client
// Effect-related processing code is included on client via #define.
class CDnStateBlow: public TBoostMemoryPool< CDnStateBlow >
{
public:
	enum
	{
		ADD_SUCCESS,
		ADD_FAIL_BY_IMMUNE,
		ADD_FAIL_BY_DIED,
		ADD_FAIL_BY_INVINCIBLE,
		ADD_FAIL_BY_REVIVAL,		// Ignore debuff effects during the 5-second resurrection period
		ADD_FAIL_BY_PROBABILITY,	// Probabilistic state effect failed..
		ADD_FAIL_BY_GUILDBLOW_PRIORITY,		//Ignore effect due to guild skill (guild war skill) priority
		ADD_FAIL_BY_PROB_SKILL_INVINCIBLE,	// Ignored by triggering state blow #226 (chance to ignore specific skill).
		ADD_FAIL_BY_COMBOLIMITBLOW,			// Ignore state effect activation due to state effect 242
#ifdef PRE_ADD_DECREASE_EFFECT
		ADD_DECREASE_EFFECT_BY_IMMUNE,
#endif // PRE_ADD_DECREASE_EFFECT
	};

	CDnStateBlow( DnActorHandle hActor );
	virtual ~CDnStateBlow(void);

	static bool InitializeClass();
	static void ReleaseClass();

protected:
#ifdef PRE_ADD_MONSTER_CHEAT
public:
#endif 
	typedef std::list<DnBlowHandle>		BLOW_HANDLE_LIST;
	typedef BLOW_HANDLE_LIST::iterator	BLOW_HANDLE_LIST_ITER;
#ifndef _GAMESERVER
	typedef std::map<int, EffectOutputInfo*> TMapEffectOutputInfo;

#if !defined(SW_MODIFY_SE_EFFECTOUTPUT_20091119_jhk8211)
	// Alternate effects overlapping on the same bone every 1 second.
	struct S_BONE_EFFECT
	{
		DnEtcHandle hEffect;
		float fOutputElapsedTime;
		bool bShow;

		S_BONE_EFFECT( void ) : fOutputElapsedTime( 0.0f ), bShow( false ) {};
		bool operator == ( const DnEtcHandle& rhs )
		{
			return (hEffect == rhs);
		}

		void Show( bool _bShow )
		{  
			bShow = _bShow;
			//hEffect->Show( true );
			if( hEffect )
			{
				hEffect->Show( bShow );
				
				if( bShow )
					hEffect->SetActionQueue( "Idle", 0, 0.f );
			}
		}
	};
	list<S_BONE_EFFECT> m_aListBoneEffectStatus[ CDnActor::Max_FX_Dummy_Bone ];
#endif

#endif
	BLOW_HANDLE_LIST m_listBlowHandle;
	DnActorHandle m_hActor;
	int m_aiApplied[ STATE_BLOW::BLOW_MAX ];
	//map<string, int> m_mapEffectData;

	// In case the list is cleared during process while looping over state effect objects; if not final release, crash cleanly.
	bool m_bLockStateBlowList;
	bool m_bLockRemoveReservedList;

	// Do not process while loading.
#ifdef _GAMESERVER
	STATIC_DECL( bool s_bStopProcess );
#else
	static bool s_bStopProcess;
#endif

#ifdef _GAMESERVER
	STATIC_DECL( int s_iStateBlowIDCount );
#else
	static int s_iStateBlowIDCount;
#endif


#ifndef _GAMESERVER
	// State effect output data info from StateEffectTagle
	// Matches the state effect index in the design doc; this vector list holds effect output info per level.
	static TMapEffectOutputInfo s_nMapEffectOutputInfo;
	bool m_bAllowDiffuseVariation;
#endif

protected:
	int GenerateStateBlowID();

#ifndef _GAMESERVER
	static void _LoadStateEffectOutputInfo( void );
	static void _ReleaseStateEffectOutputInfo( void );

#if !defined(SW_MODIFY_SE_EFFECTOUTPUT_20091119_jhk8211)
	// Effects on overlapping effect bones are cycled and displayed at 1-second intervals.
	void _ProcessDuplicateEffectBoneRotateShow( LOCAL_TIME LocalTime, float fDelta );
#endif

#endif

	// Certain state effects that can be sensitive between server and client on game frame update (ex) frame change) 
	// Process so that it enters duration state immediately upon calling AddStateBlow.
	void _CheckImmediatelyBegin( DnBlowHandle hBlow );

public:
	DnBlowHandle CreateStateBlow( DnActorHandle hActor, const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam );
	int AddStateBlow( DnBlowHandle hBlow );
	DnBlowHandle GetStateBlowFromID( int nStateBlowID );
	void RemoveStateBlowFromID( int nStateBlowID );
	void GetStateBlowFromBlowIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex, /*IN OUT*/ DNVector(DnBlowHandle)& vlhResult );
	bool IsExistStateBlowFromBlowIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex );
	bool IsExistStateBlowFromBlowID( int nBlowID );
	int RemoveStateBlowByBlowDefineIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex );
	void DelAllStateBlow();
	int GetNumStateBlow( void ) { return (int)m_listBlowHandle.size(); }
	DnBlowHandle GetStateBlow( int iIndex );
	

#ifdef _CLIENT
#ifdef PRE_ADD_MONSTER_CHEAT
	BLOW_HANDLE_LIST GetStateBlowList() { return m_listBlowHandle; }
#endif 
#endif 
#ifdef _GAMESERVER
#ifdef PRE_ADD_PROJECTILE_SE_INFO
	BLOW_HANDLE_LIST GetStateBlowList() { return m_listBlowHandle; }
	void MakeCloneStateBlowList( BLOW_HANDLE_LIST BlowList );
#endif
#endif

	// Query whether the state effect is currently applied
	// Called quite frequently from MAAiBase, so made inline for optimization.. Note: no index check in release!
	inline bool IsApplied( STATE_BLOW::emBLOW_INDEX emBlowIndex ) 
	{ 
		_ASSERT( emBlowIndex < STATE_BLOW::BLOW_MAX );
		if( emBlowIndex < STATE_BLOW::BLOW_NONE || emBlowIndex >= STATE_BLOW::BLOW_MAX ) return false;

		return (0 == m_aiApplied[ emBlowIndex ] ? false : true);
	};
	
#ifndef _GAMESERVER
	void AllowDiffuseVariation( bool bAllowDiffuseVariation ) { m_bAllowDiffuseVariation = bAllowDiffuseVariation; };
	void RestoreAllBlowGraphicEffect();
	static EffectOutputInfo *GetEffectOutputInfo( int nItemID );
	void RemoveStateBlowFromServerID( int nServerBlowID );
	DnBlowHandle GetStateBlowFromServerID( int nServerID );

#if !defined(SW_MODIFY_SE_EFFECTOUTPUT_20091119_jhk8211)
	void AddEffectAttachedBone( int iBone, DnEtcHandle hEffectHandle );
	void DelEffectAttachedBone( int iBone, DnEtcHandle hEffectHandle );
#endif

#endif


public:
	void Process( LOCAL_TIME LocalTime, float fDelta, bool bForceInitialize = false );

public:
	void OnChangedWeapon();
	bool OnDefenseAttack( DnActorHandle hHitter,CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
	int OnUseMP( int iMPDelta );
	float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
	bool OnDie( DnActorHandle hHitter );
	void OnTargetHit( DnActorHandle hTargetActor );
	void OnBeginSkill( DnSkillHandle hSkill );
	int CanAddThisBlow( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX BlowIndex );
	void ResetStateBlowBySkillType( int nSkillDurationType );
	void OnCmdActionFromPacket( const char* pActionName );

#ifdef _GAMESERVER
	// Check whether a new StateBlow can be added at OnSignal->STE_ApplyStateEffect [2010/11/12 semozz]
	CDnSkill::CanApply CanApplySkillStateEffect(const CDnSkill::SkillInfo *pUsingSkillInfo, const CDnSkill::StateEffectStruct &newStateEffect);
#endif


#ifdef _GAMESERVER
	static void StopProcess( CMultiRoom* pRoom ) {  STATIC_INSTANCE_(CDnStateBlow::s_bStopProcess) = true; }
	static void ResumeProcess( CMultiRoom* pRoom ) {  STATIC_INSTANCE_(CDnStateBlow::s_bStopProcess) = false; };
#else
	static void StopProcess( void ) { s_bStopProcess = true; }
	static void ResumeProcess( void ) { s_bStopProcess = false; };
#endif

protected:
	// Changed from vector to list to keep iterators valid when adding to the list [2011/03/21 semozz]
	//typedef std::vector<RemoveStateBlowInfo> REMOVE_STATEBLOW_LIST;
	typedef std::list<RemoveStateBlowInfo> REMOVE_STATEBLOW_LIST;
	REMOVE_STATEBLOW_LIST m_RemoveStateBlowList;
	void RemoveStateBlowReservedList();

	void AddRemoveStateBlowInfo(RemoveStateBlowInfo info);
public:
	//Function for immediate removal of a state effect
	void RemoveImediatlyStateEffectFromID(int nStateBlowID);
	void RemoveImediatlyStateEffectByBlowIndex(STATE_BLOW::emBLOW_INDEX emBlowIndex);
#if !defined(_GAMESERVER)
	void RemoveImediatlyStateEffectByServerID(int nServerBlowID);
#endif // _GAMESERVER

	bool IsImmuned(STATE_BLOW::emBLOW_INDEX blowIndex);

#ifndef _FINAL_BUILD
	void _CrashIfProcessListLocked( void );
	void _CrashIfRemoveListLocked( void );
#endif // #ifdef _FINAL_BUILD

protected:
	void Process_StateBlow_Old(LOCAL_TIME LocalTime, float fDelta, bool bForceInitialize);
	void Process_StateBlow_New(LOCAL_TIME LocalTime, float fDelta, bool bForceInitialize);
	void Process_BlowHandle(DnBlowHandle hBlow, LOCAL_TIME LocalTime, float fDelta);
#ifdef PRE_MOD_MAGICALBREEZE_CHANGE_BLOW
	STATE_BLOW::emBLOW_INDEX CheckMagicalBreezeChangeBlow( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, DnActorHandle hActor );
#endif // PRE_MOD_MAGICALBREEZE_CHANGE_BLOW

protected:
	float GetImmuneReduceTimeValue(STATE_BLOW::emBLOW_INDEX blowIndex);

};