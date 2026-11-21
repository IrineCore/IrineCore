#pragma once

//#include "DnActor.h"
#include "DnWeapon.h"
#include "SmartPtr.h"
#include "DnBlowDef.h"
#include "DnObservable.h"


class IDnSkillUsableChecker;
class IDnSkillProcessor;
class CDnProjectile;

const int MAX_SKILL_NEED_EQUIP_COUNT = 2;

const int TIME_ACCELERATION_SKILL_ID = 2415;
const int SPRIT_BOOST_SKILL_ID = 1412;


// Skill object
class CDnSkill : public CMultiSmartPtrBase< CDnSkill, MAX_SESSION_COUNT >, public TBoostMemoryPool< CDnSkill >,
				 public CDnObservable
{
public:
	CDnSkill( DnActorHandle hActor );
	virtual ~CDnSkill(void);

	enum SkillTypeEnum 
	{
		Active,
		Passive,
		AutoPassive,
		EnchantPassive,					// Skill type that enhances an existing owned skill, starting from 2nd job advancement skills.
		AutoActive,						// Skill automatically used every cooldown
	};

	enum DurationTypeEnum 
	{
		Instantly,
		Buff,
		Debuff,
		TimeToggle,
		ActiveToggle,
		Aura,
		ActiveToggleForSummon,		//Toggle skill to be used on summon monster
		SummonOnOff,					//Skill for toggling summon monster On/Off (Summon/Unsummon Alfredo)
		StanceChange,              // Skill type such as Parrying Stance, Hacking Stance, Siege Stance, Demolition, etc.
	};

	enum TargetTypeEnum 
	{
		Self,
		Enemy,
		Friend,
		Party,
		All,
	};

	enum StateEffectApplyType 
	{
		ApplySelf,
		ApplyTarget,
		ApplyAll,

		// Targets below are only valid when TargetType is All.
		ApplyEnemy,
		ApplyFriend,
	};

	// Add skill use failure reasons as needed.
	enum UsingResult
	{
		Failed = 0,
		Success,
		FailedByCooltime,
		FailedByUsableChecker,
		FailedByInvailedAction,
		NoExistSkill,
		Hack,
	};

	// Distinguish current pvp/pve mode. Same value as level table. PVE:0, PVP:1
	enum Mode
	{
		PVE = 0,
		PVP,
		NUM_SKILLLEVEL_APPLY_TYPE,
	};

	// #31013 Max number of state effect definition slots per table record.
	enum MAX_DATA_COUNT
	{	
		MAX_PROCESSOR_COUNT = 5,
		MAX_STATE_EFFECT_COUNT = 7,
		MAX_GLYPH_STATE_EFFECT_COUNT = 5,
	};

	// Data related to status ailment activation
	struct StateEffectStruct
	{
		int nID;
		StateEffectApplyType ApplyType;
		int nDurationTime;
		string szValue;
		int nGlyphID;
		int nFromEnchantPassiveSkillID;
		
		// Valid only for targets with CDnApplySEWhenActionSetBlowEnabledProcessor; unnecessary flag in other cases.
		bool bApplyInProcessor;

		// #40643 Case of modifying parameters of specially implemented state effects like Ping Pong Bomb EX.
		// Have no choice but to merge them.
		// If this flag is true, it was already merged into an existing state effect, so skip applying it.
		string strEnchantSkillSEParam;

		// State effect added as a pair via ApplyAll. Means it pairs with the immediately following state effect.
		bool bApplyAllPair;

		// #52905 Whether stack additional effect is enabled 
		bool bAddtionalStateInfo;

		StateEffectStruct( void ) : nID( 0 ), ApplyType( ApplySelf ), nDurationTime( 0 ), nGlyphID( 0 ), 
									bApplyInProcessor( false ), nFromEnchantPassiveSkillID( 0 ), bApplyAllPair( false ), bAddtionalStateInfo(false) {}
	};

	enum GLYPH_EFFECT {
		NONE,
		SKILLMP_PRO,
		SKILLDELAY_PRO,
		SKILLDELAY_RESET_PRO,
		SKILLDURATION_PRO,
		SKILLATTACKHEAL_ABSOLUTE,
	};

	struct GlyphEffectStruct
	{
		int		nGlyphID;
		int		eEffectIndex;
		float	fEffectValue;
	};

	struct SkillInfo
	{
		int iSkillID;
		int iSkillLevelID;

#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
		int iAppliedEnchantSkillID;
#endif

		int iLevel;
		int iSkillDuplicateMethod;
		int iDuplicateCount;

		int iDissolvable;
		
		SkillTypeEnum eSkillType;
		DurationTypeEnum eDurationType;
		TargetTypeEnum eTargetType;
		StateEffectApplyType eApplyType;
		CDnState::ElementEnum eSkillElement;
		DnActorHandle hSkillUser;
		tstring strUserName;
		bool bIsItemSkill;

		int nItemID;

		// State effect 60. Used only in chain skills. Holds a valid value only for skills that fire projectiles.
		int iProjectileShootActionIndex;
		int iProjectileSignalArrayIndex;
		DnActorHandle hPrevAttacker;
		int iLeaveCount;			// Remaining chain attack count. Stops propagating when it reaches 0.
		//

		// Set when attaching an effect by server-pushed effect table ID like Fire/Ice Shield. Default is -1
		string szEffectOutputIDToClient;

		bool bFromInputHasPassiveSignal;
		bool bFromBuffProp;

		string szEffectOutputIDs;

		int iSkillUserTeam;			// Team index of the actor that used this skill.

		// Skill used by the item prefix system. Values are set directly on the server and via the SC_CMDADDSTATEEFFECT packet on the client.
		// Prefix skill state effects with this flag on must not be Duplicated. They begin immediately upon packet receipt.
		bool bItemPrefixSkill;

		// [2011/04/14 semozz]
		// Added to apply guild skill to castle gate ignoring immunity (value set only by trigger..)
		bool bIgnoreImmune;
		bool bIsGuildSkill;

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
		int nPrefixSkillType;		//Suffix skill group ID
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

		LOCAL_TIME projectileSkillStartTime;

		SkillInfo( void ) : iSkillID( 0 ), iSkillLevelID( 0 ),
#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
							iAppliedEnchantSkillID( 0 ),
#endif
							iLevel( 0 ), iSkillDuplicateMethod( 0 ), iDuplicateCount( 0 ),
							eDurationType( Instantly ),
							eTargetType( TargetTypeEnum::Enemy ),
							iDissolvable( 0 ),
							eSkillType( Active ),
							eApplyType(ApplySelf),
							eSkillElement( CDnState::ElementEnum_Amount ),
							bIsItemSkill( false ),
							nItemID(-1),
							iProjectileShootActionIndex( -1 ),
							iProjectileSignalArrayIndex( 0 ),
							iLeaveCount( -1 ),
							bFromInputHasPassiveSignal( false ),
							bFromBuffProp( false ),
							iSkillUserTeam( -1 ),
							bItemPrefixSkill( false ),
							bIgnoreImmune(false)
							, bIsGuildSkill(false)
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
							, nPrefixSkillType(-1)
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
							, projectileSkillStartTime(0)
							{}
	};

	enum CanApply
	{
		Fail,
		Apply,
		ApplyDuplicateSameSkill,
	};

protected:

	DnActorHandle m_hActor;

	SkillTypeEnum				m_eSkillType;

	// Whether current data is pvp/pve
	int m_iSelectedSkillLevelDataApplyType;

	int							m_iSkillID;
	int							m_iSkillLevelID;
	int							m_iNowLevelSkillPoint;
	int							m_iNextLevelSkillPoint;
	tstring						m_strName;
	string						m_strStaticName;
	tstring						m_strDescription;

	DurationTypeEnum			m_eDurationType;
	TargetTypeEnum				m_eTargetType;

	int							m_iNeedJobClassID;
	CDnWeapon::EquipTypeEnum	m_aeNeedEquipType[ MAX_SKILL_NEED_EQUIP_COUNT ];
	int							m_iNeedItemID[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iNeedItemDecreaseCount[ NUM_SKILLLEVEL_APPLY_TYPE ];

	int							m_iDissolvable;					// Removable flag. 0: removable, 1: not removable
	int							m_iDuplicateCount;				// Skill duplicate count 
	int							m_iSkillDuplicateMethod;		// Persistent effect category
	int							m_iEffectDuplicateMethod;		// Effect output type

	float						m_fHPConsumeType[ NUM_SKILLLEVEL_APPLY_TYPE ];
	float						m_fMPConsumeType[ NUM_SKILLLEVEL_APPLY_TYPE ];
	float						m_fResetCooltime;
	int							m_iNeedHP[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iNeedMP[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iOriginalNeedMP[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iCPScore;

	int							m_iLevel;
	int							m_iMaxLevel;
	int							m_iLevelLimit;
	int							m_iIncreaseRange[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iDecreaseHP[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iDecreaseMP[ NUM_SKILLLEVEL_APPLY_TYPE ];
	float						m_fOriginalDelayTime[ NUM_SKILLLEVEL_APPLY_TYPE ];
	float						m_fCoolTime;

	bool						m_bUseCoolTimeCharging;
	int							m_nMaxCoolTimeChargeCount;
	int							m_nCurCoolTimeChargeCount;


	//--------------------------------------------------------------
	//[debug_skill]
	int                         m_nMaxUseNum;
	int                         m_nNowNum;
	//--------------------------------------------------------------


	float						m_fDelayTime[ NUM_SKILLLEVEL_APPLY_TYPE ];
	float						m_fLeftDelayTime;
	int							m_iAdditionalThreat;

	int							m_iBaseSkillID;	// Valid only for passive enhancement skills. Target skill ID to enhance.
	int							m_iAppliedEnchantPassiveSkillID;
	vector<IDnSkillProcessor*> m_vlpProcessorBackup[ NUM_SKILLLEVEL_APPLY_TYPE ];

	LOCAL_TIME					m_LastTimeToggleMPDecreaseTime;
	bool						m_bFinished;

	float						m_fStartSuperArmor;
	bool						m_bStartCanHit;

	// If toggle or aura skill, check whether it is toggled on.
	bool						m_bToggle;
	bool						m_bAura;

	// Preload effect.
	string						m_strEffectSkinName;
	string						m_strEffectAniName;
	string						m_strEffectActionName;
	EtAniObjectHandle			m_hEffect;
	CDnActionBase*				m_pEffectAction;
	
	DNVector(StateEffectStruct)		m_vlStateEffectList[ NUM_SKILLLEVEL_APPLY_TYPE ];
	vector<int>						m_vlApplySelfNoDurationStateEffectList;			// Collects creation IDs of state effects without durationtime (e.g., 2x attack power) executed once on self.
	vector< pair<int, int> >		m_vlStateDurationList[ NUM_SKILLLEVEL_APPLY_TYPE ];

	// Skill activation condition list
	vector<IDnSkillUsableChecker*>	m_vlpUsableCheckers[ NUM_SKILLLEVEL_APPLY_TYPE ];

	// Skill process list... Abstraction of situations to handle such as abnormal states, etc.
	vector<IDnSkillProcessor*>		m_vlpProcessors[ NUM_SKILLLEVEL_APPLY_TYPE ];

	bool							m_bItemSkill;							// Whether this skill is an item skill.
	
	SkillInfo						m_SkillInfo[ NUM_SKILLLEVEL_APPLY_TYPE ];

	// Update nearby player info for aura skill (actor's unique ID)
	LOCAL_TIME						m_SkillStartTime;
	
	// Duration set for passive action skill
	float							m_fPassiveActionSkillLength;

	CDnState::ElementEnum			m_eElement;

	// If a skill uses actions, collect those actions for use in monster AI.
	set<string>					m_setUseActionNames;

	// Uses the original cooldown after running once. Because of equipment item skills, inherits the cooldown of the previous equipment item in the same slot.
	float m_fOnceDelayTime;
	float m_fOnceElapsedDelayTime;
	bool m_bEquipItemSkill;
	int m_iEquipIndex;

	// Fetch and store the cooldown adjustment state effect value at the time the skill is used. 
	float m_fCoolTimeAdjustBlowValue;

	bool m_bChainingPassiveSkill;
	
	bool m_bAppliedPassiveSelfBlows;

	// Last time the skill was used (timeGetTime())
	DWORD m_dwLastUseSkillTimeStamp;

	std::vector<GlyphEffectStruct> m_vGlyphEffectList;

	int m_iExclusiveID;				// ID in the skill table to distinguish mutually exclusive skills

	bool m_bTempSkill;				// Temporarily created skill #26902

	int m_iGlobalSkillGroupID;
	float m_afGlobalCoolTime[ NUM_SKILLLEVEL_APPLY_TYPE ];	// In ms in the table; convert to seconds when storing.
	float m_fAnotherGlobalSkillCoolTime;					// When another global skill is used, apply that skill's cooldown.
	int m_nAnotherGlobakSkillID;

#if defined( PRE_ADD_ACADEMIC )
	int m_iSummonerDecreaseSP;
	int m_iSummonerDecreaseSPSkillID;
#endif // #if defined( PRE_ADD_ACADEMIC )

	bool m_bEnchantedFromBubble;

protected:
	void _OnInitialize( void );
	void _OnBeginProcessException( void );
	bool _LoadSkillLevelData( int iSkillTableID, int iLevel, int iSkillLevelDataApplyType );
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	bool _LoadMonsterSkillLevelData( int iSkillTableID, int iLevel, int iSkillLevelDataApplyType );
#endif // #ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	void _RefreshDecreaseMP( int iSkillLevelDataApplyType );

public:
	DWORD GetStateEffectCount( void );
	StateEffectStruct *GetStateEffectFromIndex( DWORD dwIndex );
	
	bool bIsExtremitySkill(){ return m_iExclusiveID>0; }	// Is it an ultimate skill?

	static CDnSkill::CanApply CanApplySkillStateEffect( DnActorHandle hTargetActor, DnSkillHandle hUsingSkill, map<int, bool>& mapDuplicateResult, bool isHitProcess = false );
	static CDnSkill::CanApply CanApplySkillStateEffect( const SkillInfo* pUsingSkillInfo, DnActorHandle hTargetActor, int iSkillID, int iLevel, int iSkillDuplicateMethod, int iDuplicateCount, map<int, bool>& mapDuplicateResult, bool isHitProcess = false );

	void CheckTargetCount( int iTargetActorCount );

	void SetHasActor( DnActorHandle hActor );

	void CheckAndAddSelfStateEffect( void );
	void CheckAndRemoveInstantApplySelfStateEffect( void );

	void RefreshDecreaseMP( int iSkillLevelDataApplyType = NUM_SKILLLEVEL_APPLY_TYPE );
	
public:
	// Skill creation factory function
	static DnSkillHandle CreateSkill( DnActorHandle hActor, int iSkillTableID, int iLevel );
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	static DnSkillHandle CreateMonsterSkill( DnActorHandle hActor, int iSkillTableID, int iLevel );
#endif // #ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	static SkillTypeEnum GetSkillType( int nSkillID );	
	static void CreateSkillInfo( int nSkillID, int nSkillLevel, CDnSkill::SkillInfo & sSkillInfo, std::vector<CDnSkill::StateEffectStruct> & vecSkillEffect, bool bUseBattleGround = false );
	static void CreateBattleGroundSkillInfo( int nSkillID, int nSkillLevel, CDnSkill::SkillInfo & sSkillInfo, std::vector<CDnSkill::StateEffectStruct> & vecSkillEffect );

	DnActorHandle GetActor( void ) { return m_hActor; };
	int GetClassID( void ) { return m_iSkillID; };
	int GetSkillLevelID( void ) { return m_iSkillLevelID; } 

	//--------------------------------------------------------------
	//[debug_skill]
	int GetSkillMaxUseNum( void ) { return m_nMaxUseNum; }
	int GetSkillNowNum( void ) { return m_nNowNum; }

	int SetSkillMaxUseNum( int nMaxUseNum ) { return m_nMaxUseNum = nMaxUseNum; }
	int SetSkillNowNum( int nNowNum ) { return m_nNowNum = nNowNum; }

	//--------------------------------------------------------------

	bool AddUsableCheckers( IDnSkillUsableChecker* pUsableChecker, int iSelectedLevelData );
	bool AddProcessor( IDnSkillProcessor* pProcessor, int iSelectedLevelData );
	
	// Various query functions
	SkillTypeEnum GetSkillType( void ) { return m_eSkillType; };
	DurationTypeEnum GetDurationType( void ) { return m_eDurationType; };
	TargetTypeEnum GetTargetType( void ) { return m_eTargetType; };
	
	int GetIncreaseRange( void ) { return m_iIncreaseRange[ m_iSelectedSkillLevelDataApplyType ]; };
	int GetDecreaseHP( void ) { return m_iNeedHP[ m_iSelectedSkillLevelDataApplyType ]; };
	int GetDecreaseMP( void ) { return m_iNeedMP[ m_iSelectedSkillLevelDataApplyType ]; };
	
	TCHAR* GetName( void ) { return const_cast<TCHAR*>(m_strName.c_str()); };
	int GetLevel( void ) { return m_iLevel; };
	int GetNowLevelSkillPoint( void ) { return m_iNowLevelSkillPoint; };
	int GetNextLevelSkillPoint( void ) { return m_iNextLevelSkillPoint; };
	int GetMaxLevel( void ) { return m_iMaxLevel; };
	int GetLevelLimit( void ) { return m_iLevelLimit; };
	
	int GetNeedEquipType( int iIndex ) 
	{ 
		_ASSERT( 0 <= iIndex && iIndex < 2 );
		if( iIndex >= 0 && iIndex < 2 )
		{
			return m_aeNeedEquipType[ iIndex ]; 
		}
		return -1;
	};

	int GetNeedItemID( void ) { return m_iNeedItemID[ m_iSelectedSkillLevelDataApplyType ]; };
	int GetNeedItemDecreaseCount( void ) { return m_iNeedItemDecreaseCount[ m_iSelectedSkillLevelDataApplyType ]; };
	int GetNeedJobClassID( void ) { return m_iNeedJobClassID; };
	int GetAdditionalThreat() { return m_iAdditionalThreat; }

	const char* GetEffectSkinName( void ) { return m_strEffectSkinName.c_str(); };
	const char* GetEffectAniName( void ) { return m_strEffectAniName.c_str(); };
	const char* GetEffectActName( void ) { return m_strEffectActionName.c_str(); };

	bool CanDissolve( void ) { return (m_iDissolvable == 0) ? true : false; };

	int GetMaxDuplicateCount( void ) { return m_iDuplicateCount; };
	int GetDuplicateMethod( void ) { return m_iSkillDuplicateMethod; };
	int	GetEffectDuplicateMethod( void ) { return m_iEffectDuplicateMethod; };

	float GetHPConsumeType( void ) { return m_fHPConsumeType[ m_iSelectedSkillLevelDataApplyType ]; };
	float GetMPConsumeType( void ) { return m_fMPConsumeType[ m_iSelectedSkillLevelDataApplyType ]; };

	int GetCPScore( void ) { return m_iCPScore; }

	CDnState::ElementEnum GetElement( void ) { return m_eElement; };

	bool Initialize( int iSkillTableID, int iLevel );
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	bool InitializeMonsterSkill( int iSkillTableID, int iLevel );
#endif // #ifdef PRE_FIX_GAMESERVER_OPTIMIZE

	bool IsSatisfyWeapon( void );
	UsingResult CanExecute( void );
	bool IsFinished( void );

	void OnBeginCoolTime();
	void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	void ProcessExecute( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	// Handle cooldown and etc.
	void Process( LOCAL_TIME LocalTime, float fDelta );

	void EnableToggle( bool bEnable ) { m_bToggle = bEnable; };
	bool IsToggleOn( void ) { return m_bToggle; };

	void EnableAura( bool bEnable ) { m_bAura = bEnable; };
	bool IsAuraOn( void ) { return m_bAura; };

	IDnSkillProcessor* GetProcessor( int iType );
	IDnSkillProcessor* GetProcessor( int iType, int iLevelDataApplyType );

	float GetCoolTime( void ) { return m_fCoolTime; };
	void SetForceCoolTime( float fCoolTime ) { m_fCoolTime = fCoolTime; }
	void SetAdditionalCoolTimeChargeCount( int nAddCount );
	bool IsUseCoolTimeCharging( void ) const { return m_bUseCoolTimeCharging; }
	int GetMaxCoolTimeChargeCount( void ) const { return m_nMaxCoolTimeChargeCount; }
	int GetCurCoolTimeChargeCount( void ) const { return m_nCurCoolTimeChargeCount; }
	void ResetCoolTime( void ) 
	{ 
		m_fCoolTime = 0.0f; m_fLeftDelayTime = 0.0f;  

		// Remove the cooldown value set by another global skill sharing the same global ID when this skill is actually used.
		m_fAnotherGlobalSkillCoolTime = 0.0f;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
		m_fDeltaGlobalCoolTime = 0.0f;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
	}
	void IgnoreCoolTime( void ) 
	{ 
		ResetCoolTime();
		for( int i = PVE; i < NUM_SKILLLEVEL_APPLY_TYPE; ++i )
			m_fDelayTime[i] = 0;
	}

	bool IsUseCheckerType( int iCheckerType );
	bool IsUseProcessorType( int iProcessorType );

	int GetNumChecker( void ) { return (int)m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].size(); };
	int GetNumProcessor( void ) { return (int)m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].size(); };

	void AsItemSkill( void ) 
	{ 
		m_bItemSkill = true; 

		for( int i = PVE; i < NUM_SKILLLEVEL_APPLY_TYPE; ++i )
			m_SkillInfo[ i ].bIsItemSkill = true;
	};
	bool IsItemSkill( void ) { return m_bItemSkill; };

	// Info to copy to CDnStateBlow. 
	const SkillInfo* GetInfo() { return &m_SkillInfo[ m_iSelectedSkillLevelDataApplyType ]; };

	float GetDelayTime( void );
	void SetElapsedDelayTime( float fValue ) { m_fLeftDelayTime = fValue; }
	float GetElapsedDelayTime() { return m_fLeftDelayTime; }
	
	float GetPassiveSkillLength(){ return m_fPassiveActionSkillLength; }
	void SetPassiveSkillLength( float fTimeLength ) { m_fPassiveActionSkillLength = fTimeLength; };
	void SetPassiveSkillActionName( const char* pActionName );
	
	set<string>& GetUseActionNames( void ) { return m_setUseActionNames; }
	void AddUseActionName( const char* pActionName ) { m_setUseActionNames.insert( pActionName ); };
	bool IsUseActionNames( const set<string>& setUseActionNames );

	void AddSignalApplyNoPacketStateEffectHandle( int iBlowID ) { m_vlApplySelfNoDurationStateEffectList.push_back( iBlowID ); };

	void SetOnceCoolTime( float fDelayTime, float fElapsedDelayTime );
	void AsEquipItemSkill( void ) { m_bEquipItemSkill = true; };
	bool IsEquipItemSkill( void ) { return m_bEquipItemSkill; };
	void SetEquipIndex( int iIndex ) { m_iEquipIndex = iIndex; };
	int GetEquipIndex ( void ) { return m_iEquipIndex; };

	// Request action succession for an active skill that must have an action play processor! (Called when handling STE_Input signal)
	void OnChainInput( const char* pActionName );
	bool IsChainInputAction( const char* pActionName );

	// Check when chain input arrives for a passive skill. The chain input flag is reset once checked.
	// When chain input occurs, passive skill duration extends by the action length.
	// Both flag and time must be used together like this to allow continuous passive skill chaining.
	bool CheckChainingPassiveSkill( void );

	void FromInputHasPassive( void ) 
	{ 
		for( int i = PVE; i < NUM_SKILLLEVEL_APPLY_TYPE; ++i )
			m_SkillInfo[ i ].bFromInputHasPassiveSignal = true; 
	};
	bool IsFromInputHasPassive( void ) { m_SkillInfo[ m_iSelectedSkillLevelDataApplyType ].bFromInputHasPassiveSignal; };

	void SetAppliedPassiveBlows( bool bAppliedPassiveBlows ) { m_bAppliedPassiveSelfBlows = bAppliedPassiveBlows; };
	bool IsAppliedPassiveBlows( void ) { return m_bAppliedPassiveSelfBlows; };

	void AddGlyphStateEffect( int nGlyphID );
	void DelGlyphStateEffect( int nGlyphID );

	// Allows a 0.5s margin to account for server cooldown error, though
	// Because zone transitions or other new cases can cause a drift of 0.5 seconds or more from the client
	// Record the last skill usage timestamp; if interval is greater than the skill cooltime specified in data 
	// Reset the cooldown stored in the server's skill object. (#19737)
	// And we should also consider skills that speed up cooldown like Time Acceleration, though this function 
	// Keep it for now as a safety measure.
	void UpdateSkillCoolTimeExactly( void );

	// Set skill level data values. Not set if conditions are not met.
	bool SelectLevelDataType( int iSkillLevelDataType, bool bPlayerSummonedMonster = false );
	int GetSelectedLevelDataType( void ) { return m_iSelectedSkillLevelDataApplyType; };

	int GetExclusiveID( void ) { return m_iExclusiveID; };

	// Functions for temporarily acquired skills..
	void AsTempSkill( void ) { m_bTempSkill = true; };
	bool IsTempSkill( void ) { return m_bTempSkill; };
	void SetLevelLimit( int iLevelLimit ) { m_iLevelLimit = iLevelLimit; };
	void SetNeedJobClassID( int iJobID ) { m_iNeedJobClassID = iJobID; };
	//////////////////////////////////////////////////////////////////////////


	// Whether this is a state effect that must be checked if skill effect can be applied unconditionally regardless of duration.
	static bool IsNeedCheckApplyStateBlow( STATE_BLOW::emBLOW_INDEX emBlowIndex );

	// If the skill type is EnchantPassive, get the base skill ID.
	int GetBaseSkillID( void );

	// Apply enhancement passive skill values.
	// Store enhancement passive skill ID, data, etc. separately in a struct.
	// Once applied, prevent the same enhancement passive skill ID from being applied again.
	bool ApplyEnchantSkill( DnSkillHandle hEnchantPassiveSkill );
	void ApplyEnchantSkillOnceFromBubble( DnSkillHandle hEnchantPassiveSkill );
	bool CheckAndUnifyStateEffectArgument( int iSkillLevelApplyType, StateEffectStruct* pEnchantSkillSE );
	void ReleaseEnchantSkill( void );
	bool CheckAndDivideStateEffectArgument( StateEffectStruct* pDestSkillSE );
	bool IsEnchantedSkill( void ) { return (0 != m_iAppliedEnchantPassiveSkillID); };
	int GetEnchantedSkillID( void ) { return m_iAppliedEnchantPassiveSkillID; };

	//To recalculate prefix skill probability values
	IDnSkillUsableChecker* GetChecker( int iType );
	//Priority of prefix skill

	bool IsExistUsableChecker( int iType ); // Does it have a defined UsableChecker?

	void CheckProcessorOnBegin( void );
	void CheckChangeActionStrByBubbleProcessor();
	void CheckStateEffectApplyOnOffByBubbleProcessor();
	void CheckProcessorOnEnd( void );

	int GetGlobalSkillGroupID( void ) { return m_iGlobalSkillGroupID; };
	void OnAnotherGlobalSkillBeginCoolTime( DnSkillHandle hSkill );

protected:
	int m_nPriority;
public:
	void SetPrefixSystemPriority(int nPriority) { m_nPriority = nPriority; }
	int GetPrefixSystemPriority() { return m_nPriority; }

protected:
	int m_nItemID;
public:
	int GetItemID() { return m_nItemID;}
	void SetItemID(int nItemID) 
	{ 
		m_nItemID = nItemID;
		for( int i = PVE; i < NUM_SKILLLEVEL_APPLY_TYPE; ++i )
			m_SkillInfo[ i ].nItemID = m_nItemID;
	}

protected:
	bool m_bIsPrefixTriggerSkill;
public:
	void SetPrefixTriggerSkill(bool bValue) { m_bIsPrefixTriggerSkill = bValue; }
	bool IsPrefixTriggerSkill() { return m_bIsPrefixTriggerSkill; }
	void SetItemPrefixSkill() { m_SkillInfo[ PVP ].bItemPrefixSkill = true; m_SkillInfo[ PVE ].bItemPrefixSkill = true; };
	bool IsItemPrefixSkill() { return m_SkillInfo[ m_iSelectedSkillLevelDataApplyType ].bItemPrefixSkill; };


protected:
	float m_fCoolTimeMultipier;	//For cooldown adjustment by skill Showtime
public:
	float GetCoolTimeAdjustValue() { return m_fCoolTimeMultipier; }
	void SetCoolTimeAdjustValue(float fValue) 
	{ 
		m_fCoolTimeMultipier = fValue; 

		m_fCoolTime *= m_fCoolTimeMultipier;
		m_fLeftDelayTime *= m_fCoolTimeMultipier;
	}

protected:
	bool IsFinishedAuraSkill();

protected:
	bool m_isIgnoreImmuneBackup;
public:
	//There is a case called from trigger via hActor->UseSkill(nSkillID).
	//Find the skill and modify its skill info; restore the original value in OnEnd.
	void SetIgnoreImmnune(bool bIgnore)
	{
		m_isIgnoreImmuneBackup = m_SkillInfo->bIgnoreImmune;
		m_SkillInfo->bIgnoreImmune = bIgnore;
	}

//////////////////////////////////////////////////////////////////////////
// One prefix skill applies state effects (without duration) per actor.
// Create a list to remove state effects applied per actor when the skill ends, and
// Need to remove each one..
protected:
	struct ActorHandleComp
	{
		bool operator()(const DnActorHandle& h1, const DnActorHandle& h2) const
		{
			return (h1.GetHandle() < h2.GetHandle());
		}
	};

	typedef vector<int> BLOWLIST;
	typedef std::map<DnActorHandle, BLOWLIST, ActorHandleComp> PREFIX_SKILL_BLOWLIST;

	PREFIX_SKILL_BLOWLIST m_PrefixBlowList;

public:
	void AddPrefixBlow(DnActorHandle hActor, int nBlowID);
	void RemovePrefixBlow();
//////////////////////////////////////////////////////////////////////////

#if defined( PRE_ADD_ACADEMIC )
	void SetSummonerDecreaseSP( int iSkillID, int iSP ){ m_iSummonerDecreaseSPSkillID=iSkillID; m_iSummonerDecreaseSP = iSP; }
	int GetSummonerDecreaseSP(){ return m_iSummonerDecreaseSP; }
	int GetSummonerDecreaseSPSkillID(){ return m_iSummonerDecreaseSPSkillID; }
#endif // #if defined( PRE_ADD_ACADEMIC )

protected:
	int m_nLevelUpValue;
public:
	void SetLevelUpValue(int nLevelUp) { m_nLevelUpValue = nLevelUp; }
	int GetLevelUpValue() { return m_nLevelUpValue; }

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
protected:
	int m_nPrefixSkillType;
public:
	void SetPrefixSkillType(int nSkillType) { m_SkillInfo[PVP].nPrefixSkillType = nSkillType; m_SkillInfo[PVE].nPrefixSkillType = nSkillType; }
	int GetPrefixSkillType() { return m_SkillInfo[m_iSelectedSkillLevelDataApplyType].nPrefixSkillType; }
#endif // PRE_ADD_PREFIX_SYSTE_RENEW


protected:
	int m_SummonMonsterID;

	void OnInitializeSummonMonsterInfo();				//Find summon monster ID info in skill action signal.
	DnActorHandle FindSummonMonster(int nMonsterID);	//Check if the configured monster is summoned.
	
public:
	int GetSummonMonsterID() { return m_SummonMonsterID; }
	bool SummonMonsterOff();				//Unsummon the summoned monster.


	//If any value in string index;index;index......;index; matches blowIndex, this state effect is not applied.
	static bool IsSkipStateBlow(const char* szSkipStateBlows, STATE_BLOW::emBLOW_INDEX blowIndex);


protected:
	bool m_bAddStateEffectQueue;
public:
	bool IsAddStateEffectQueue() { return m_bAddStateEffectQueue; }
	void SetAddStateEffectQueue(bool bAdded) { m_bAddStateEffectQueue = bAdded; }

	void SetChaningPassiveSkill( const bool bChaningPassiveSkill )	{ m_bChainingPassiveSkill = bChaningPassiveSkill; }

#if defined(PRE_ADD_50903)
	//Number of actors hit in HitSignal to be used for damage distribution state effect
protected:
	int m_nHitCountForVarianceDamage;
public:
	void SetHitCountForVarianceDamage(int nCount) { m_nHitCountForVarianceDamage = nCount; }
	int GetHitCountForVarianceDamage() { return m_nHitCountForVarianceDamage; }
#endif // PRE_ADD_50903


protected:
	typedef std::list<CDnProjectile*> PROJECTILE_LIST;	//List of projectiles used in the skill (for state effect 242)
	typedef std::map<LOCAL_TIME, PROJECTILE_LIST> SKILL_PROJECTILE_LIST;
	SKILL_PROJECTILE_LIST m_ProjectileList;
public:
	int GetProjectileCount(LOCAL_TIME startTime);// { return (int)m_ProjectileList.size(); }
	void AddProjectile(CDnProjectile* pProjectile);		//Called when created via STE_Projectile from an actor while a skill with state effect 242 is being cast.
	void RemoveProjectile(CDnProjectile* pProjectile);	//In the projectile destructor, find and call the skill using the skill info set on the projectile to remove it.

	LOCAL_TIME GetSkillStartTime() { return m_SkillStartTime; }

protected:
	void ClearStigmaStateEffect();	//Function for brand state effect (removes any previously applied brand effect before using skill)

protected:
	DNVector(StateEffectStruct) m_AddtionalStateInfoList;

	//Function added to implement Kali stack additional effect..
	void ApplyAddtionalStateInfo();		//Add state effect to be added
	void RemoveAddtionalStateInfo();	//Remove added state effect

#if defined(PRE_FIX_64312)
protected:
	bool m_isAppliedSummonMonsterEnchantSkill;
	bool m_bIsSummonMonsterSkill;

	DnSkillHandle m_SummonMonsterEnchantSkill;

public:
	bool IsSummonMonsterSkill();

	void AddSummonMonsterEnchantSkill(DnSkillHandle hSkill);
	void RemoveSummonMonsterEnchantSkill();

	DnSkillHandle GetSummonMonsterEnchangeSkill() { return m_SummonMonsterEnchantSkill; }

	bool ApplySummonMonsterEnchantSkill(DnSkillHandle hSkill);
#endif // PRE_FIX_64312

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
protected:
	float m_fDeltaGlobalCoolTime;

public:
	float GetOrigDelayTime();
	void UpdateGlobalCoolTime(float fRate);
	void ResetGlobalCoolTime(float fRate);
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined(PRE_FIX_66175)
protected:
	vector<IDnSkillUsableChecker*> m_vlUsableCheckersBackup[NUM_SKILLLEVEL_APPLY_TYPE];
public:
	vector<IDnSkillUsableChecker*>& GetUsableChecker(int nType) { return m_vlpUsableCheckers[nType]; }
#endif // PRE_FIX_66175

#if defined(PRE_ADD_65808)
public:
	bool IsSummonMonsterRecall(int& monsterID);
#endif // PRE_ADD_65808

protected:
	string m_BackupEffectOutputIDs[NUM_SKILLLEVEL_APPLY_TYPE];
	string m_BackupEffectOutputIDToClient[NUM_SKILLLEVEL_APPLY_TYPE];
public:
	string& GetEffectOutputIDs(int iSkillLevelDataApplyType) { return m_SkillInfo[iSkillLevelDataApplyType].szEffectOutputIDs; }
	string& GetEffectOutputIDToClient(int iSkillLevelDataApplyType) { return m_SkillInfo[iSkillLevelDataApplyType].szEffectOutputIDToClient; }

	string& GetBackupEffectOutputIDs(int iSkillLevelDataApplyType) { return m_BackupEffectOutputIDs[iSkillLevelDataApplyType]; }
	string& GetBackupEffectOutputIDToClient(int iSkillLevelDataApplyType) { return m_BackupEffectOutputIDToClient[iSkillLevelDataApplyType]; }

	int GetAnotherGlobalSkillID() { return m_nAnotherGlobakSkillID; }
};


//////////////////////////////////////////////////////////////////////////

#if defined(_GAMESERVER)
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
class CDnPrefixSkill;
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
class CoolTimeInfo
{
protected:
	float m_fDelayTime;

	float m_fCoolTime;
	float m_fCoolTimeAdjustBlowValue;
	float m_fLeftDelayTime;

	CDnSkill::DurationTypeEnum m_eDurationType;

	int m_nSkillID;

	DnActorHandle m_hActor;

public:
	CoolTimeInfo()
	{
		m_fDelayTime = 0.0f;
		m_fCoolTime = 0.0f;
		m_fCoolTimeAdjustBlowValue = 1.0f;
		m_fLeftDelayTime = 0.0f;
		m_eDurationType = CDnSkill::DurationTypeEnum::Instantly;
		m_nSkillID = 0;
	};
	~CoolTimeInfo() {};

	void OnBeginCoolTime();
	bool Process(LOCAL_TIME localTime, float fDelta);	
	void SetInfo(DnSkillHandle &hSkill);
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
	void SetInfo(CDnPrefixSkill* pPrefixSkill);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};


class CoolTimeManager
{
protected:
	typedef std::map<int, CoolTimeInfo*> COOLTIME_LIST;
	COOLTIME_LIST m_CoolTimeList;

public:
	CoolTimeManager() {};
	~CoolTimeManager() {};
	
	bool IsCoolTime(int nSkillID);

	void AddCoolTime(DnSkillHandle hSkill);
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
	void AddCoolTime(CDnPrefixSkill* pPrefixSkill);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

	void RemoveCoolTime(int nSkillID);
	void Process(LOCAL_TIME localTime, float fDelta);

	void InitList();
};
#endif // _GAMESERVER

//////////////////////////////////////////////////////////////////////////


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
class CDnPrefixSkill
{
public:
	CDnPrefixSkill(int nPrefixType);
	~CDnPrefixSkill();

	enum PrefixType
	{
		Prefix_Non = -1,//Not a prefix.
		Prefix_000 = 0,	//of Destruction
		Prefix_001,	//Magical
		Prefix_002,	//Bear weapon
		Prefix_003,	//Weapon of Wind
		Prefix_004,	//Weapon of Wisdom
		Prefix_005,	//Harsh
		Prefix_006,	//Weapon of Darkness
		Prefix_007,	//Weapon of Life
		Prefix_008,	//Weapon of Mana
		Prefix_009,	//Of vitality
		Prefix_010,	//Critical
		Prefix_011,	//Of Restraint
		Prefix_012,	//Of shock
		Prefix_013,	//Hero's
		Prefix_014,	//Knight's
		Prefix_015,	//Iron Wall weapon
		Prefix_016,	//Veil weapon
		Prefix_017,	//Fire weapon
		Prefix_018,	//Weapon of Water
		Prefix_019,	//Light weapon
		Prefix_020,	//Iron Wall armor
		Prefix_021,	//Veil armor
		Prefix_022,	//Bear armor
		Prefix_023,	//Armor of Wind
		Prefix_024,	//Armor of Wisdom
		Prefix_025,	//Healthy
		Prefix_026,	//Lucky
		Prefix_027,	//Armor of Life
		Prefix_028,	//Armor of Mana
		Prefix_029,	//Sturdy
		Prefix_030, //Flexible
		Prefix_031, //Of will
		Prefix_032, //Sturdy
		Prefix_033, //Indomitable
		Prefix_034, //Fire armor
		Prefix_035, //Armor of Water
		Prefix_036, //Light armor
		Prefix_037, //Armor of Darkness
	};
protected:
	int m_nPrefixType;				//Prefix type?
	float m_fProbability;			//Prefix skill activation probability (adds probability when skill is added)
	DnSkillHandle m_hSkill;	//Representative skill..

	std::list<DnSkillHandle> m_SkillList;

	DNVector(CDnSkill::StateEffectStruct)		m_vlStateEffectList;
	
public:
	int GetSkillType() { return m_nPrefixType; }		//Prefix skill type (Lucky/Destructive...)
	float GetProbability() { return m_fProbability; }	//Prefix skill activation probability

	DnSkillHandle GetSkillHandle() { return m_hSkill; }
	void SetSkillHandle(DnSkillHandle hSkill);		//Set representative skill.

	int GetSkillCount() { return (int)m_SkillList.size(); }	//Registered skill count

	void AddSkill(DnSkillHandle hSkill);			//Add prefix skill
	void RemoveSkill(DnSkillHandle hSkill);			//Remove prefix skill

	//Set the skill with the shortest cooldown among registered skills as the representative skill
	void UpdateCandidateSkill();

	//Issue occurs when subtracting state effect value (existing value - single skill state effect)
	//So after removing one skill, iterate through the entire skill list and apply state effect values.
	void UpdateStateEffects();

	void AddStateEffectInfo(DnSkillHandle hSkill);		//Merge skill state effects...

	int GetStateEffectCount() { return (int)m_vlStateEffectList.size(); }
	CDnSkill::StateEffectStruct* GetStateEffectFromIndex(int nIndex) 
	{ 
		if( nIndex < 0 || nIndex >= (int)m_vlStateEffectList.size() ) return NULL;

		return &m_vlStateEffectList[nIndex]; 
	}

protected:
	void AddStateEffectInfo( STATE_BLOW::emBLOW_INDEX blowList[], int nCount, DnSkillHandle hSkill );

	void AddStateEffectInfo_Prefix_000(DnSkillHandle hSkill);//of Destruction
	void AddStateEffectInfo_Prefix_001(DnSkillHandle hSkill);//Magical
	void AddStateEffectInfo_Prefix_002(DnSkillHandle hSkill);//Bear weapon
	void AddStateEffectInfo_Prefix_003(DnSkillHandle hSkill);//Weapon of Wind
	void AddStateEffectInfo_Prefix_004(DnSkillHandle hSkill);//Weapon of Wisdom
	void AddStateEffectInfo_Prefix_005(DnSkillHandle hSkill);//Harsh
	void AddStateEffectInfo_Prefix_006(DnSkillHandle hSkill);//Weapon of Darkness
	void AddStateEffectInfo_Prefix_007(DnSkillHandle hSkill);//Weapon of Life
	void AddStateEffectInfo_Prefix_008(DnSkillHandle hSkill);//Weapon of Mana
	void AddStateEffectInfo_Prefix_009(DnSkillHandle hSkill);//Of vitality
	void AddStateEffectInfo_Prefix_010(DnSkillHandle hSkill);//Critical
	void AddStateEffectInfo_Prefix_011(DnSkillHandle hSkill);//Of Restraint
	void AddStateEffectInfo_Prefix_012(DnSkillHandle hSkill);//Of shock
	void AddStateEffectInfo_Prefix_013(DnSkillHandle hSkill);//Hero's
	void AddStateEffectInfo_Prefix_014(DnSkillHandle hSkill);//Knight's
	void AddStateEffectInfo_Prefix_015(DnSkillHandle hSkill);//Iron Wall weapon
	void AddStateEffectInfo_Prefix_016(DnSkillHandle hSkill);//Veil weapon
	void AddStateEffectInfo_Prefix_017(DnSkillHandle hSkill);//Fire weapon
	void AddStateEffectInfo_Prefix_018(DnSkillHandle hSkill);//Weapon of Water
	void AddStateEffectInfo_Prefix_019(DnSkillHandle hSkill);//Light weapon
	void AddStateEffectInfo_Prefix_020(DnSkillHandle hSkill);//Iron Wall armor
	void AddStateEffectInfo_Prefix_021(DnSkillHandle hSkill);//Veil armor
	void AddStateEffectInfo_Prefix_022(DnSkillHandle hSkill);//Bear armor
	void AddStateEffectInfo_Prefix_023(DnSkillHandle hSkill);//Armor of Wind
	void AddStateEffectInfo_Prefix_024(DnSkillHandle hSkill);//Armor of Wisdom
	void AddStateEffectInfo_Prefix_025(DnSkillHandle hSkill);//Healthy
	void AddStateEffectInfo_Prefix_026(DnSkillHandle hSkill);//Lucky
	void AddStateEffectInfo_Prefix_027(DnSkillHandle hSkill);//Armor of Life
	void AddStateEffectInfo_Prefix_028(DnSkillHandle hSkill);//Armor of Mana
	void AddStateEffectInfo_Prefix_029(DnSkillHandle hSkill);//Sturdy
	void AddStateEffectInfo_Prefix_030(DnSkillHandle hSkill);//Flexible
	void AddStateEffectInfo_Prefix_031(DnSkillHandle hSkill);//Of will
	void AddStateEffectInfo_Prefix_032(DnSkillHandle hSkill);//Sturdy
	void AddStateEffectInfo_Prefix_033(DnSkillHandle hSkill);//Indomitable
	void AddStateEffectInfo_Prefix_034(DnSkillHandle hSkill);//Fire armor
	void AddStateEffectInfo_Prefix_035(DnSkillHandle hSkill);//Armor of Water
	void AddStateEffectInfo_Prefix_036(DnSkillHandle hSkill);//Light armor
	void AddStateEffectInfo_Prefix_037(DnSkillHandle hSkill);//Armor of Darkness
};
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
