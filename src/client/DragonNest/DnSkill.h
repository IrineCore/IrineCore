#pragma once

//#include "DnActor.h"
#include "MIInventoryItem.h"
#include "DnWeapon.h"
#include "DnSkillTreeSystem.h"
#include "SmartPtr.h"


class IDnSkillUsableChecker;
class IDnSkillProcessor;

const int MAX_SKILL_NEED_EQUIP_COUNT = 2;

const int TIME_ACCELERATION_SKILL_ID = 2415;
const int SPRIT_BOOST_SKILL_ID = 1412;


// Skill object
class CDnSkill : public CSmartPtrBase< CDnSkill >, public MIInventoryItem
{
public:
	CDnSkill( DnActorHandle hActor );
	virtual ~CDnSkill(void);


	enum SkillTypeEnum 	{
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
		ActiveToggleForSummon,
		/*
Monster summon skills show as Toggle in UI, while actual skill usage is processed as Instantly.
Enable EnableToggle in OnBegin, and disable EnableToggle when monster summon is released.
Even if the skill is usable at cast time, if a summoned monster exists, only unsummon it and do not use the skill.
Even if skill usage fails, unsummon any existing summoned monster.
		*/
		SummonOnOff,					//Skill for toggling summon monster On/Off (Summon/Unsummon Alfredo)
		StanceChange,
		TotalLevelSkill,				// Unified level skill
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
		int nFromEnchantPassiveSkillID;			// If this state effect was added from an enhancement passive skill ID, store that skill's ID.

		// #40643 Case of modifying parameters of specially implemented state effects like Ping Pong Bomb EX.
		// Have no choice but to merge them.
		// If this flag is true, it was already merged into an existing state effect, so skip applying it.
		string strEnchantSkillSEParam;

		// #52905 Whether stack additional effect is enabled 
		bool bAddtionalStateInfo;

		StateEffectStruct( void ) : nID( 0 ), ApplyType( ApplySelf ), nDurationTime( 0 ), nGlyphID( 0 ), nFromEnchantPassiveSkillID( 0 ), bAddtionalStateInfo(false) {}
	};

	enum GLYPH_EFFECT {
		NONE,
		SKILLMP_PRO,
		SKILLDELAY_PRO,
		SKILLDELAY_RESET_PRO,
		SKILLDURATION_PRO,
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
		int iSkillLevelIDOffset;
#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
		int iAppliedEnchantSkillID;
#endif

		// Only the two variables above are the skill info sent via CS_CMDADDSTATEEFFECT packet.
		DurationTypeEnum	eDurationType;
		SkillTypeEnum		eSkillType;
		int					iDissolvable;
		int					iSkillBufIcon;
		string				effectOutputIDs;
		DnActorHandle		hSkillUser;
		bool				bIsItemSkill;
		int					iBuffIconImageIndex;
#ifdef PRE_ADD_SKILL_ADDTIONAL_BUFF
		int                 iDebuffIconImageIndex;
		string				debuffEffectOutputIDs;
#endif
		TargetTypeEnum		eTargetType;
		int					iSkillUserTeam;
		bool				bFromInputHasPassiveSignal;
		bool				bFromBuffProp;
		
		// Skill used by the item prefix system. Values are set directly on the server and via the SC_CMDADDSTATEEFFECT packet on the client.
		// Prefix skill state effects with this flag on must not be Duplicated. They begin immediately upon packet receipt.
		bool				bItemPrefixSkill;

		// Added to ignore immunity handling for guild war skills (castle gate related) [2011/04/14 semozz]
		bool				bIgnoreImmune;

#if defined(PRE_FIX_NEXTSKILLINFO)
		int					nSkillLevel;	//Skill level info
#endif // PRE_FIX_NEXTSKILLINFO

		SkillInfo( void ) : iSkillID( 0 ), iSkillLevelID( 0 ), iSkillLevelIDOffset( 0 ),
#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
							iAppliedEnchantSkillID(0),
#endif
							eDurationType( Instantly ), iDissolvable( 0 ),
							iSkillBufIcon( 0 ), 
							eSkillType( Active ), bIsItemSkill(false),
							iBuffIconImageIndex( -1 ), eTargetType(Self), iSkillUserTeam(-1), bFromInputHasPassiveSignal(false), bFromBuffProp( false ), bIgnoreImmune(false)
#ifdef PRE_ADD_SKILL_ADDTIONAL_BUFF
							, iDebuffIconImageIndex( -1 )
#endif
#if defined(PRE_FIX_NEXTSKILLINFO)
							, nSkillLevel(0)
#endif // PRE_FIX_NEXTSKILLINFO
							, bItemPrefixSkill( false ) {}

		bool operator == ( const SkillInfo& Rhs ) const
		{
			return (iSkillID == Rhs.iSkillID) &&
				   (iSkillLevelID == Rhs.iSkillLevelID);
		}

		bool operator < ( const SkillInfo& Rhs ) const
		{
			return (iSkillID < Rhs.iSkillID);
		}
	};


protected:

	DnActorHandle m_hActor;

	bool m_bLock;			// Whether currently locked because the skill book has not been used.
	bool m_bAcquired;		// Whether the skill is acquired. 


	// Whether current data is pvp/pve
	int m_iSelectedSkillLevelDataApplyType;

	SkillTypeEnum				m_eSkillType;

	int							m_iSkillID;
	int							m_iSkillLevelID;
	int							m_iNextLevelSkillPoint;		// Skill points required for next skill level-up. 
	int							m_iFirstLevelSkillPoint;	// Skill points for the first level
	tstring						m_strName;
	string						m_strStaticName;
	tstring						m_strSkillExplainString[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iSkillExplainStringID;		// Skill description string ID. ID used when retrieving via CEtUIXML::GetUIString()
	int							m_iNextSkillExplainStringID;	// Next skill description string ID. 
	tstring						m_strNextSkillExplainString[ NUM_SKILLLEVEL_APPLY_TYPE ];

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
	int							m_iNeedHP[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iNeedMP[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iOriginalNeedMP[ NUM_SKILLLEVEL_APPLY_TYPE ];	//Record MP before modified by Crest skill effect
	int							m_iCPScore;

	int							m_iLevel;
	int							m_iMaxLevel;
	int							m_iUsePoint;
	int							m_iLevelLimit;
	int							m_iFirstLevelLimit;		// Minimum level required to learn the skill (required level for level 1 skill)
	int							m_iIncreaseRange[ NUM_SKILLLEVEL_APPLY_TYPE ];		// Additional range
	int							m_iDecreaseHP[ NUM_SKILLLEVEL_APPLY_TYPE ];
	int							m_iDecreaseMP[ NUM_SKILLLEVEL_APPLY_TYPE ]; 

	float						m_fOriginalDelayTime[ NUM_SKILLLEVEL_APPLY_TYPE ];	//Record cooldown before modified by Crest skill effect
	float						m_fDelayTime[ NUM_SKILLLEVEL_APPLY_TYPE ];
	float						m_fLeftDelayTime;
	float						m_fCoolTime;

	bool						m_bUseCoolTimeCharging;
	int							m_nMaxCoolTimeChargeCount;
	int							m_nCurCoolTimeChargeCount;

	//--------------------------------------------------------------
	//[debug_skill]
	int                         m_nMaxUseNum;
	int                         m_nNowNum;
	//--------------------------------------------------------------


	int							m_iBuffIconImageIndex;
	string						m_EffectOutputIDs;
#ifdef PRE_ADD_SKILL_ADDTIONAL_BUFF
	int                         m_iDebuffIconImageIndx;
	string						m_DebuffEffectOutputIDs;
#endif
	
	int							m_iUnlockSkillBookItemID;
	int							m_iUnlockPrice;

	int							m_iBaseSkillID;	// Valid only for passive enhancement skills. Target skill ID to enhance.
	int							m_iAppliedEnchantPassiveSkillID;
	vector<IDnSkillProcessor*> m_vlpProcessorBackup[ NUM_SKILLLEVEL_APPLY_TYPE ];

	LOCAL_TIME					m_LastTimeToggleMPDecreaseTime;
	bool						m_bFinished;

	// If toggle or aura skill, check whether it is toggled on.
	bool						m_bNowToggled;
	bool						m_bNowAura;

	bool						m_bCantUseInVillage;

	bool                        m_bEnableActiveToggleCoolTime;
	
	vector<StateEffectStruct>		m_vlStateEffectList[ NUM_SKILLLEVEL_APPLY_TYPE ];
	vector< pair<int, int> >		m_vlStateDurationList[ NUM_SKILLLEVEL_APPLY_TYPE ];

	// Skill activation condition list
	vector<IDnSkillUsableChecker*>	m_vlpUsableCheckers[ NUM_SKILLLEVEL_APPLY_TYPE ];

	// Skill process list... Abstraction of situations to handle such as abnormal states, etc.
	vector<IDnSkillProcessor*>		m_vlpProcessors[ NUM_SKILLLEVEL_APPLY_TYPE ];

	bool							m_bItemSkill;							// Whether this skill is an item skill.

	// Not yet used on the client. Core skill info to be passed externally.
	// Reduce the hassle of fetching from TableDB every time.
	SkillInfo						m_SkillInfo[ NUM_SKILLLEVEL_APPLY_TYPE ];

	// Duration set for passive action skill
	float							m_fPassiveActionSkillLength;
	vector<DnBlowHandle>			m_vlAppliedNoPacketInstantBlow;			// For Instant state effects with no duration that are judged directly without a packet from the server
	CDnState::ElementEnum			m_eElement;

	// The corresponding action populated if this skill uses Play Ani
	string							m_strAction;

	set<string>						m_setUseActionNames;

	// Uses the original cooldown after running once. Because of equipment item skills, inherits the cooldown of the previous equipment item in the same slot.
	float m_fOnceDelayTime;
	float m_fOnceElapsedDelayTime;

	// Whether it is an equipment item skill
	bool m_bEquipItemSkill;

	// Skill tree node rendering info
	CDnSkillTreeSystem::S_NODE_RENDER_INFO m_NodeRenderInfo;

	// Fetch and store the cooldown adjustment state effect value at the time the skill is used. 
	float m_fCoolTimeAdjustBlowValue;

	// True if newly acquired
	bool m_bNewAcquired;

	bool m_bAppliedPassiveSelfBlows;

	bool m_bResetSkill;
	std::vector<GlyphEffectStruct> m_vGlyphEffectList;

	int m_iExclusiveID;				// ID in the skill table to distinguish mutually exclusive skills

	bool m_bTempSkill;				// Temporarily created skill #26902

	bool m_bEnableTooltip;

	bool m_bGlyphActiveSkillDisbale;

	// Check if skill was created by an item (different purpose from m_bItemSkill) [2011/03/16 semozz]
	bool m_isCreatedByItem;

	int m_iSkillGroupID;
	int m_iGlobalSkillGroupID;
	float m_afGlobalCoolTime[ NUM_SKILLLEVEL_APPLY_TYPE ];	// In ms in the table; convert to seconds when storing.
	float m_fAnotherGlobalSkillCoolTime;					// When another global skill is used, apply that skill's cooldown.
	int m_iAnotherGlobalSkillLevelDataType;
	int m_nAnotherGlobakSkillID;

protected:
	// Check StateEffects for which the server does not send a separate packet. Determine which effect to display on screen.
	//void _CheckNoPacketSelfStateEffect( void );	
	void _OnBeginProcessException( void );

	bool _LoadSkillLevelData( int iSkillTableID, int iLevel, int iSkillLevelDataApplyType );
	void _RefreshDecreaseMP( int iSkillLevelDataApplyType );

	// Added all actions related to input signal.
	void _AddUsedActions( const char* pActionName );

public:

	bool bIsExtremitySkill(){ return m_iExclusiveID>0; }	// Is it an ultimate skill?

	// Validate data for skill creation
	static bool ValidateSkillData( int iSkillTableID, int iLevel );

	// Skill creation factory function. Unacquired skills are created at level 0.
	static DnSkillHandle CreateSkill( DnActorHandle hActor, int iSkillTableID, int iLevel );

	// Unlock manipulation is not possible after the user actually owns this skill.
	void Lock( void ) { m_bLock = true; m_bAcquired = false; };
	void UnLock( void ) { _ASSERT( 1 == m_iLevel); m_bLock = false; };		// On the client, locked state corresponds to initial level 1 state.
	bool IsLocked( void ) { return m_bLock; };

	// Acquired an unlocked skill for the first time. Must be in unlocked state when acquiring.
	void Acquire( void );
	bool IsAcquired( void ) { return m_bAcquired; };


	DnActorHandle GetActor( void ) { return m_hActor; };
	void SetHasActor( DnActorHandle hActor );
	void UpdateRangeChecker( int iSkillLevelDataApplyType );

	int GetClassID( void ) const { return m_iSkillID; };
	int GetSkillLevelID( void ) const { return m_iSkillLevelID; }
	
	void SetClassID(int nNewSkill)  { m_iSkillID = nNewSkill; };
	int GetApplyType() { return m_iSelectedSkillLevelDataApplyType; };

	//--------------------------------------------------------------
	//[debug_skill]
	int GetSkillMaxUseNum( void ) { return m_nMaxUseNum; }
	int GetSkillNowNum( void ) { return m_nNowNum; }

	int SetSkillMaxUseNum( int nMaxUseNum ) { return m_nMaxUseNum = nMaxUseNum; }
	int SetSkillNowNum( int nNowNum ) { return m_nNowNum = nNowNum; }

	//--------------------------------------------------------------

	bool AddUsableCheckers( IDnSkillUsableChecker* pUsableChecker, int iSkillLevelDataType );
	bool AddProcessor( IDnSkillProcessor* pProcessor, int iSkillLevelDataType );
	
	// Various query functions
	SkillTypeEnum GetSkillType( void ) { return m_eSkillType; };
	DurationTypeEnum GetDurationType( void ) { return m_eDurationType; };
	TargetTypeEnum GetTargetType( void ) { return m_eTargetType; };


	void SetDurationType(DurationTypeEnum eType) { m_eDurationType = eType; };
	void SetSkillType(SkillTypeEnum eType) { m_eSkillType = eType; };


	DWORD GetStateEffectCount( void );
	StateEffectStruct *GetStateEffectFromIndex( DWORD dwIndex );
	
	int GetIncreaseRange( void ) { return m_iIncreaseRange[ m_iSelectedSkillLevelDataApplyType ]; };
	int GetDecreaseHP( void ) { return m_iNeedHP[ m_iSelectedSkillLevelDataApplyType ]; };
	int GetDecreaseMP( void ) { return m_iNeedMP[ m_iSelectedSkillLevelDataApplyType ]; };
	
	// These functions are used only in the UI...
	TCHAR* GetName( void ) const { return const_cast<TCHAR*>(m_strName.c_str()); };
	int GetLevel( void ) const { return m_iLevel; };
	int GetMaxLevel( void ) { return m_iMaxLevel; };
	int GetNextLevelSkillPoint( void ) { return m_iNextLevelSkillPoint; };
	//int GetBuySkillPoint( void ) { return m_iFirstLevelSkillPoint; };
	void SetUsePoint( int nPoint ) { m_iUsePoint = nPoint; }
	int GetUsePoint() { return m_iUsePoint; }
	void CalcLevelUpUsePoint( int iLevelCount );
	void ResetLevelUpUsePoint( int iLevelCount );
	int GetLevelLimit( void ) { return m_iLevelLimit; }
	int GetFirstLevelLimit( void ) { return m_iFirstLevelLimit; }

	int GetExplainStringID( void ) { return m_iSkillExplainStringID; };
	int GetNextExplainStringID( void ) { return m_iNextSkillExplainStringID; };
	const TCHAR *GetNextExplainString( void ) { return m_strNextSkillExplainString[ m_iSelectedSkillLevelDataApplyType ].c_str(); }
	// from MIInventoryItem
	const TCHAR* GetDescription( void ) { return m_strSkillExplainString[ m_iSelectedSkillLevelDataApplyType ].c_str(); };
	
	int GetNeedItemID( void ) { return m_iNeedItemID[ m_iSelectedSkillLevelDataApplyType ]; };
	int GetNeedItemDecreaseCount( void ) { return m_iNeedItemDecreaseCount[ m_iSelectedSkillLevelDataApplyType ]; };
	int GetNeedJobClassID( void ) { return m_iNeedJobClassID; };
	int GetNeedEquipType( int iIndex ) 
	{ 
		_ASSERT( 0 <= iIndex && iIndex < 2 );
		if( 0 <= iIndex && iIndex < 2 )
			return m_aeNeedEquipType[ iIndex ]; 
		else
			return CDnWeapon::EquipTypeEnum_Amount;
	};

	int GetCPScore( void ) { return m_iCPScore; }
	
	int GetBuffIconImageIndex( void ) { return m_iBuffIconImageIndex; };
#ifdef PRE_ADD_SKILL_ADDTIONAL_BUFF
	int GetAdditionalIconImageIndex( void ) { return m_iDebuffIconImageIndx; };
#endif
	
	bool CanDissolve( void ) { return (m_iDissolvable == 0) ? true : false; }; 
	
	int GetMaxDuplicateCount( void ) { return m_iDuplicateCount; };
	int GetDuplicateMethod( void ) { return m_iSkillDuplicateMethod; };
	int	GetEffectDuplicateMethod( void ) { return m_iEffectDuplicateMethod; };

	float GetHPConsumeType( void ) { return m_fHPConsumeType[ m_iSelectedSkillLevelDataApplyType ]; };
	float GetMPConsumeType( void ) { return m_fMPConsumeType[ m_iSelectedSkillLevelDataApplyType ]; };

	CDnState::ElementEnum GetElement( void ) { return m_eElement; };

	virtual bool Initialize( int iSkillTableID, int iLevel );

	bool IsSatisfyWeapon( void );
	UsingResult CanExecute( void );
	bool IsFinished( void );

	void OnBeginCoolTime();
	void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	void OnBeginForceSync( LOCAL_TIME LocalTime, float fDelta );
	void ProcessExecute( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	// Handle cooldown and etc.
	void Process( LOCAL_TIME LocalTime, float fDelta );

	void EnableToggle( bool bEnable ) { m_bNowToggled = bEnable; };
	bool IsToggleOn( void ) { return m_bNowToggled; };
	bool IsEanbleActiveToggleCoolTime(){ return m_bEnableActiveToggleCoolTime; }
	void EnableActiveToggleCoolTime(bool bEnable){ m_bEnableActiveToggleCoolTime = bEnable; }

	void EnableAura( bool bEnable ) { m_bNowAura = bEnable; };
	bool IsAuraOn( void ) { return m_bNowAura; };

	IDnSkillUsableChecker* GetChecker( int iType );
	IDnSkillProcessor* GetProcessor( int iType );

	bool IsUseCheckerType( int iCheckerType );
	bool IsUseProcessorType( int iProcessorType );

	int GetNumChecker( void ) { return (int)m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].size(); };
	int GetNumProcessor( void ) { return (int)m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].size(); };

	// from MIInventoryItem
	virtual float GetCoolTime() { return m_fCoolTime; };
	virtual void SetForceCoolTime( float fCoomTime ) { m_fCoolTime = fCoomTime; }
	void SetAdditionalCoolTimeChargeCount( int nAddCount );
	bool IsUseCoolTimeCharging( void ) const { return m_bUseCoolTimeCharging; }
	int GetMaxCoolTimeChargeCount( void ) const { return m_nMaxCoolTimeChargeCount; }
	int GetCurCoolTimeChargeCount( void ) const { return m_nCurCoolTimeChargeCount; }
	virtual ItemConditionEnum GetItemCondition();
	///////////////////////////////////////////////////////////////////
	void ResetCoolTime( void ) 
	{ 
		m_fCoolTime = 0.0f; m_fLeftDelayTime = 0.0f;  

		// Remove the cooldown value set by another global skill sharing the same global ID when this skill is actually used.
		m_fAnotherGlobalSkillCoolTime = 0.0f;

		m_nAnotherGlobakSkillID = 0;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
		m_fDeltaGlobalCoolTime = 0.0f;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
	}
	void AsItemSkill( void ) { m_bItemSkill = true; };
	bool IsItemSkill( void ) { return m_bItemSkill; };

	const SkillInfo* GetInfo() const { return &m_SkillInfo[ m_iSelectedSkillLevelDataApplyType ]; };

	float GetDelayTime( void );
	void SetElapsedDelayTime( float fValue ) { m_fLeftDelayTime = fValue; }
	float GetElapsedDelayTime() { return m_fLeftDelayTime; }

	bool HasCoolTime() {return (m_fDelayTime[ m_iSelectedSkillLevelDataApplyType ] != 0.f); }

	void SetSkillLevelIDOffset( int iSkillLevelIDOffset, int iSelectedSkillLevelDataType ) { m_SkillInfo[ iSelectedSkillLevelDataType ].iSkillLevelIDOffset = iSkillLevelIDOffset; };

	float GetPassiveSkillLength(){ return m_fPassiveActionSkillLength; }
	void SetPassiveSkillLength( float fTimeLength ) { m_fPassiveActionSkillLength = fTimeLength; };
	void SetPassiveSkillActionName( const char* pActionName );

	void RefreshDecreaseMP( int iSkillLevelDataApplyType = NUM_SKILLLEVEL_APPLY_TYPE );
	bool IsUseActionNames( const set<string>& setUseActionNames );

	// Hanki 2009.7.27 
	// Related to effect application duration used on self (#1911)
	void AddSignalApplyNoPacketStateEffectHandle( DnBlowHandle hBlow ) { m_vlAppliedNoPacketInstantBlow.push_back(hBlow); };
	//

	void SetOnceCoolTime( float fDelayTime, float fElapsedDelayTime );
	void AsEquipItemSkill( void ) { m_bEquipItemSkill = true; };
	bool IsEquipItemSkill( void ) { return m_bEquipItemSkill; };

	int GetUnlockSkillBookItemID( void ) { return m_iUnlockSkillBookItemID; };

	// Request state effect inheritance for an active skill that must have an action playback processor! (Called when handling STE_Input signal)
	void OnChainInput( const char* pActionName );

	void SetNodeRenderInfo( CDnSkillTreeSystem::S_NODE_RENDER_INFO& NodeRenderInfo ) { m_NodeRenderInfo = NodeRenderInfo; };
	const CDnSkillTreeSystem::S_NODE_RENDER_INFO& GetNodeRenderInfo( void ) { return m_NodeRenderInfo; };

	int GetNeedAcquireSkillPoint( void ) { return m_iFirstLevelSkillPoint; };
	int GetUnlockPrice( void ) { return m_iUnlockPrice; };

	bool IsEnableTooltip() { return m_bEnableTooltip; }
	// Whether it is newly acquired.
	bool IsNewAcquire( void ) { return m_bNewAcquired; };
	void SetNewAcquire( void ) { m_bNewAcquired = true; };
	void ResetNewAcquire( void ) { m_bNewAcquired = false; };

	// Whether a passive buff skill is currently applied. 
	// Current criterion is whether the target weapon is equipped.
	void SetAppliedPassiveBlows( bool bAppliedPassiveBlows ) { m_bAppliedPassiveSelfBlows = bAppliedPassiveBlows; };
	bool IsAppliedPassiveBlows( void ) { return m_bAppliedPassiveSelfBlows; };

	void AddGlyphStateEffect( int nGlyphID );
	void DelGlyphStateEffect( int nGlyphID );

	void SetGlyphActiveSkillDisable( bool bActiveSkillDisable );
	bool GetGlyphActiveSkillDisable();

	void SetResetSkill( bool bReset = true )	{ m_bResetSkill = bReset; }

	int GetExclusiveID( void ) { return m_iExclusiveID; };

	void CheckProcessorOnBegin( void );
	void CheckChangeActionStrByBubbleProcessor();
	void CheckProcessorOnEnd( void );

	// Functions for temporarily acquired skills..
	void AsTempSkill( void ) { m_bTempSkill = true; };
	bool IsTempSkill( void ) { return m_bTempSkill; };
	void SetLevelLimit( int iLevelLimit ) { m_iLevelLimit = iLevelLimit; };
	void SetNeedJobClassID( int iJobID ) { m_iNeedJobClassID = iJobID; };
	//////////////////////////////////////////////////////////////////////////

	// Set skill level data values. Not set if conditions are not met.
	bool SelectLevelDataType( int iSkillLevelDataType, bool bForce = false );
	int GetSelectedLevelDataType( void ) { return m_iSelectedSkillLevelDataApplyType; };

	const set<string>& GetUseActionSet( void ) { return m_setUseActionNames; };

	bool operator < ( const DnSkillHandle hSkill )
	{
		if( m_iFirstLevelLimit < hSkill->GetFirstLevelLimit() )
			return true;
		else
		if( m_iFirstLevelLimit == hSkill->GetFirstLevelLimit() )
			return (m_strName < hSkill->GetName());
		else
			return false;
	};

	// If the skill type is EnchantPassive, get the base skill ID.
	int GetBaseSkillID( void );

	// Apply enhancement passive skill values.
	// Store enhancement passive skill ID, data, etc. separately in a struct.
	// Once applied, prevent the same enhancement passive skill ID from being applied again.
	bool ApplyEnchantSkill( DnSkillHandle hEnchantPassiveSkill );
	bool CheckAndUnifyStateEffectArgument( int iSkillLevelApplyType, StateEffectStruct* pEnchantSkillSE );
	void ReleaseEnchantSkill( void );
	bool CheckAndDivideStateEffectArgument( StateEffectStruct* pDestSkillSE );
	bool IsEnchantedSkill( void ) { return (0 != m_iAppliedEnchantPassiveSkillID); };
	int GetEnchantedSkillID( void ) { return m_iAppliedEnchantPassiveSkillID; };

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

public:
	bool CheckAnimation();

protected:
	bool IsFinishedAuraSkill();

public:
	void SetCreatedByItem(bool bFlag) { m_isCreatedByItem = bFlag; }
	bool IsCreatedByItem() { return m_isCreatedByItem; }

	void SetItemPrefixSkill() { m_SkillInfo[ PVP ].bItemPrefixSkill = true; m_SkillInfo[ PVE ].bItemPrefixSkill = true; };
	bool IsItemPrefixSkill() { return m_SkillInfo[ m_iSelectedSkillLevelDataApplyType ].bItemPrefixSkill; };


protected:
	int m_nLevelUpValue;
public:
	void SetLevelUpValue(int nLevelUp) { m_nLevelUpValue = nLevelUp; }
	int GetLevelUpValue() { return m_nLevelUpValue; }

	int GetSkillGroupID( void ) const { return m_iSkillGroupID; }
	int GetGlobalSkillGroupID( void ) { return m_iGlobalSkillGroupID; };
	void OnAnotherGlobalSkillBeginCoolTime( DnSkillHandle hSkill );

protected:
	int m_SummonMonsterID;

	void OnInitializeSummonMonsterInfo();				//Find summon monster ID info in skill action signal.
	DnActorHandle FindSummonMonster(int nMonsterID);	//Check if the configured monster is summoned.

public:
	int GetSummonMonsterID() { return m_SummonMonsterID; }
	bool SummonMonsterOff();				//Unsummon the summoned monster.

	bool IsUseInVillage() { return !m_bCantUseInVillage; }


#if defined(PRE_FIX_45899)
protected:
	bool m_isDisableSkillByItemMove;		//Pause skill usage after requesting item move until the completion packet arrives.
public:
	void DisableSkillByItemMove(bool bDisable) { m_isDisableSkillByItemMove = bDisable; }
#endif // PRE_FIX_45899

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
	string m_BackupDebuffEffectOutputIDs[NUM_SKILLLEVEL_APPLY_TYPE];
public:
	string& GetEffectOutputIDs(int iSkillLevelDataApplyType) { return m_SkillInfo[iSkillLevelDataApplyType].effectOutputIDs; }
	string& GetDebuffEffectOutputIDs(int iSkillLevelDataApplyType) { return m_SkillInfo[iSkillLevelDataApplyType].debuffEffectOutputIDs; }

	string& GetBackupEffectOutputIDs(int iSkillLevelDataApplyType) { return m_BackupEffectOutputIDs[iSkillLevelDataApplyType]; }
	string& GetBackupDebuffEffectOutputIDs(int iSkillLevelDataApplyType) { return m_BackupDebuffEffectOutputIDs[iSkillLevelDataApplyType]; }

	int GetAnotherGlobalSkillID() { return m_nAnotherGlobakSkillID; }
};