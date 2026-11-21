#pragma once
#include "DnBlowDef.h"
#include "DnDamageBase.h"
#include "DnActor.h"
#ifdef PRE_FIX_MEMOPT_SIGNALH
#include "DnCommonUtil.h"
#endif

#ifndef _GAMESERVER
// Effect output mode data
struct EffectOutputInfo
{
	int iStateEffectIndex;
	int iShowTimingType;
	int iOutputType;
	int iPlayType;
	string strSkinFileName;
	string strAniFileName;
	string strActFileName;
	string strActorActionName;
	int iUseDiffuseVariationMethod;
	float afDiffuseRed[ 2 ];
	float afDiffuseGreen[ 2 ];
	float afDiffuseBlue[ 2 ];
	float fDiffuseChangeSpeed;
	vector<int> vlDummyBoneIndices;
	int iEffectFileDataIndex;			// Effect index read once from data and reused continuously

	// Activation timing
	enum
	{
		ALL_TIME,
		OWNER_HANDLING,
	};

	// Playback type
	enum
	{
		REPEAT,
		ONCE,
		PART_REPEAT,
	};

	// Effect output type
	enum
	{
		ATTACH,
		DO_ACTION,
		DUMMY_BONE_ATTACH,
	};

	// Must match default contents in StateEffectTable
	EffectOutputInfo( void ) : iStateEffectIndex( 0 ), iShowTimingType( ALL_TIME ), iOutputType( ATTACH ), iPlayType( REPEAT ),
		iUseDiffuseVariationMethod( 0 ), fDiffuseChangeSpeed( 1.0f ), iEffectFileDataIndex( -1 )
	{
		memset( afDiffuseRed, 1, sizeof(afDiffuseRed) );
		memset( afDiffuseBlue, 1, sizeof(afDiffuseBlue) );
		memset( afDiffuseGreen, 1, sizeof(afDiffuseGreen) );
	};

	const char* GetSkinFileName( void ) const { return strSkinFileName.empty() ? NULL : strSkinFileName.c_str(); };
	const char* GetAniFileName( void ) const { return strAniFileName.empty() ? NULL : strAniFileName.c_str(); };
	const char* GetActFileName( void ) const { return strActFileName.empty() ? NULL : strActFileName.c_str(); };
	const char* GetActorActFileName( void ) const { return strActorActionName.empty() ? NULL : strSkinFileName.c_str(); };
};
#endif

#ifdef _GAMESERVER
class CDnBlow : public CMultiSmartPtrBase< CDnBlow, MAX_SESSION_COUNT >
#else
class CDnBlow : public CSmartPtrBase< CDnBlow >
#endif
{

public:
	CDnBlow( DnActorHandle hActor );
	virtual ~CDnBlow();

protected:
	
	LOCAL_TIME m_LastLocalTime;
	DnActorHandle m_hActor;

	STATE_BLOW m_StateBlow;
	STATE_BLOW::emBLOW_INDEX m_eOriginalBlowIndex;
	
	bool m_bPermanentBlow;
	bool m_bIgnoreEffectAction;			// Ignore action execution in the state effect table. Used for freeze state effects applied to boss-grade monsters.

	DWORD m_dwCallBackType;

	int m_nBlowID;
	int m_nThreatAggro;
	
	// Refers to the ID of the skill table and skill level table respectively (ClassID)
	bool m_bHasParentSkill;				// Whether the state effect originated from a skill.
	CDnSkill::SkillInfo	m_ParentSkillInfo;

	bool m_bDuplicated;		// Whether this is a stacked state effect
	int m_iDuplicateCount;
	
	float m_fOriginalDurationTime;
	float m_fValue;

	bool m_bEternity;
	
	bool m_bGuildBlow;			//Whether the created state effect was used by a guild skill

	bool m_bFromSourceItem;
#ifdef _GAMESERVER
	bool m_bHPMPFullWhenBegin;
#endif

#ifndef _GAMESERVER
	const EffectOutputInfo* m_pEffectOutputInfo;
	DnEtcHandle m_hEtcObjectEffect;
	DNVector(DnEtcHandle) m_vlhDummyBoneEtcObjects;		// FX effect attached to a dummy bone
	bool m_bUseTableDefinedGraphicEffect;
	float m_fAccumTime;
	bool m_bAllowDiffuseVariation;		// Externally, it is set to output only the one with the highest Variation number.

	// ID of the state effect created on the server. Later, with the ID sent from server via SC_CMDREMOVESTATEEFFECTFROMID packet
	// Store when the state effect is created so it can be deleted later.
	int m_nServerBlowID;

#else
	char m_PacketBuffer[ 128 ];
	CPacketCompressStream* m_pPacketStream;
	// #31067 For monsters, already executed if there is an action to execute directly.
	bool m_bStartedMonsterActionInSETableInfo;
#endif


private:

	// Must be called only from CDnStateBlow::Process().
	friend class CDnStateBlow;
	void SetActorHandle( DnActorHandle hActor ) { m_hActor = hActor; };

public:

#ifndef _GAMESERVER
	DnEtcHandle _AttachGraphicEffect( bool bAllowSameSkillDuplicate = false );
	bool _DetachGraphicEffect( void );
	void _SetDiffuse( float fRed, float fGreen, float fBlue, float fAlpha );

	bool AttachGraphicEffectDefaultType( void );
	bool DetachGraphicEffectDefaultType( void );
	void UseTableDefinedGraphicEffect( bool bUseTableDefined ) { m_bUseTableDefinedGraphicEffect = bUseTableDefined; };
	bool IsUseTableDefinedGraphicEffect( void ) { return m_bUseTableDefinedGraphicEffect; };
	void SetEffectOutputInfo( const EffectOutputInfo* pOutputInfo ) { m_pEffectOutputInfo = pOutputInfo; };
	const EffectOutputInfo* GetEffectOutputInfo( void ) { return m_pEffectOutputInfo; };
	void AllowDiffuseVariation( bool bAllowDiffuseVariation );
	void SetServerBlowID( int nServerBlowID ) { m_nServerBlowID = nServerBlowID; };
	int GetServerBlowID( void ) { return m_nServerBlowID; };
#else
	// #31067 If an actor action is defined in the state effect table, for monsters it must also be executed on the server.
	void CheckAndStartActorActionInEffectInfo( void );
	void CheckAndStopActorActionInEffectInfo( void );
#endif
	
	bool CanAddGraphicsEffect();

	void SetCallBackType( DWORD dwType ) { m_dwCallBackType = dwType; }
	void AddCallBackType( DWORD dwType ) { m_dwCallBackType |= dwType; }
	DWORD GetCallBackType() { return m_dwCallBackType; }

	void SetBlowID( int nID ) { m_nBlowID = nID; }
	int GetBlowID() { return m_nBlowID; }

	void SetParentSkillInfo( const CDnSkill::SkillInfo* pParentSkillInfo );
	const CDnSkill::SkillInfo* GetParentSkillInfo( void ) { return &m_ParentSkillInfo; };

	void SetPermanent( bool bInstant ) { m_bPermanentBlow = bInstant; }
	__inline bool IsPermanent() { return m_bPermanentBlow; }

	DnActorHandle GetActorHandle() { return m_hActor; }

public:
	void SetStateBlow( STATE_BLOW &stateblow )			{ m_StateBlow = stateblow; m_fOriginalDurationTime = stateblow.fDurationTime; }
	void SetBlow( STATE_BLOW::emBLOW_INDEX blow )		{ m_StateBlow.emBlowIndex = blow; }
	void SetState( STATE_BLOW::emBLOW_STATE state )		{ m_StateBlow.emBlowState = state; }

	void ResetDurationTime( void ) { m_StateBlow.fDurationTime = m_fOriginalDurationTime; };

	void SetDurationTime( float fTime );
	void SetDelayTime( float fTime );
	void SetValue( const char *szValue )			{ m_StateBlow.szValue = szValue ? szValue : ""; }

	inline STATE_BLOW& GetStateBlow()				{ return m_StateBlow; }
	inline STATE_BLOW::emBLOW_INDEX GetBlowIndex()	{ return m_StateBlow.emBlowIndex; }
	inline STATE_BLOW::emBLOW_STATE GetBlowState()	{ return m_StateBlow.emBlowState; }

	inline float GetDurationTime()							{ return m_StateBlow.fDurationTime; }
	inline float GetDelayTime()							{ return m_StateBlow.fDelayTime; }
	inline const char* GetValue()							{ return m_StateBlow.szValue.c_str(); }
	
	inline float GetFloatValue()							{ return m_fValue; };
	
	inline bool IsBegin()		{ return (m_StateBlow.emBlowState==STATE_BLOW::STATE_BEGIN); }
	inline bool IsDuration()	{ return (m_StateBlow.emBlowState==STATE_BLOW::STATE_DURATION); }
	inline bool IsEnd()		{ return (m_StateBlow.emBlowState==STATE_BLOW::STATE_END); }

	int GetThreatAggro();
	inline bool IsGuildBlow() { return m_bGuildBlow; }

public:
	virtual bool CanBegin( void ) { return true; };		// Check probability and other pre-activation conditions to determine whether the state effect can trigger.
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
	virtual void OnChangedWeapon() {}

	// Using state from the attacker hActor may reflect normal attack/defense since the skill action might already be finished, so 
	// Use pAttackerState which also accounts for being carried by a projectile. 
	// If pAttackerState is used inside the function, it must be NULL-checked internally.
	virtual bool OnDefenseAttack( DnActorHandle hActor, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess ) { return bHitSuccess; }
	virtual int OnUseMP( int nMPDelta ) { return nMPDelta; };

	// On client, not involved in actual damage calculation; called when OnDamage packet is received.
	virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam ) { return 0.0f; };

	// Time when the target is hit.
	virtual void OnTargetHit( DnActorHandle hTargetActor ) {};

	// Function used only in CDnActorStatIntervalManipulator!
	virtual bool OnCheckIntervalProb( void ) { return true; };
	virtual float GetStatDelta( void ) { return 0.0f; };
	virtual bool OnCustomIntervalProcess( void ) { return false; };

	//bool IsPostStateEffect( void );
	CDnActorState::AddBlowStateType GetAddBlowStateType();

	// Handles effect stacking. Override the function if specific handling is needed per state effect.
	void Duplicate( const STATE_BLOW& StateBlowInfo );
	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );
	virtual bool CalcDuplicateValue( const char* szValue ) { return false; };
	bool IsDuplicated( void ) { return m_bDuplicated; };
	int GetDuplicateCount( void ) { return m_iDuplicateCount; };

	virtual void OnCmdActionFromPacket( const char* pActionName ) {};

#ifdef _GAMESERVER
	const CPacketCompressStream* GetPacketStream( const char* pszOrgParam=NULL, const bool bOnInit=false );
	virtual void WriteAdditionalPacket( void ) {};
#else
	virtual void OnReceiveAddPacket( CPacketCompressStream& PacketStream ) {};
#endif

	void FromSourceItem( void ) { m_bFromSourceItem = true; };
	bool IsFromSourceItem( void ) { return m_bFromSourceItem; };

#ifdef _GAMESERVER
	void SetHPMPFullWhenBegin( void ) { m_bHPMPFullWhenBegin = true; };
#endif

#if !defined(_GAMESERVER)
	// To activate the state effect's graphic effect via packet (eActor::SC_SHOW_STATE_EFFECT). [2011/02/18 semozz]
	virtual void ShowGraphicEffect(bool bShow) {};
#endif // _GAMESERVER

	// Moved the OnSignal call functionality from Process to here..
	virtual void SignalProcess( LOCAL_TIME LocalTime, float fDelta ) {};

	void SetIgnoreEffectAction( bool bIgnore ) { m_bIgnoreEffectAction = bIgnore; }

	void SetOriginalBlowIndex( STATE_BLOW::emBLOW_INDEX eOriginalBlowIndex ) { m_eOriginalBlowIndex = eOriginalBlowIndex; };
	void RestoreOriginalBlowIndex( void ) { m_StateBlow.emBlowIndex = m_eOriginalBlowIndex; };
	bool IsChangedBlowIndex( void ) { return (m_eOriginalBlowIndex != STATE_BLOW::BLOW_NONE); };

	bool IsEternity() { return m_bEternity; }
	void SetEternity( bool bEternity ){ m_bEternity = bEternity; }

#if defined(_GAMESERVER)
#if defined(PRE_FIX_BLOCK_CONDITION)
protected:
	bool IsCanBlock(DnActorHandle hHitter, DnActorHandle hHitted, const CDnDamageBase::SHitParam &HitParam);
#endif // PRE_FIX_BLOCK_CONDITION

#if defined(PRE_ADD_49166)
	bool IsInVaildBlockCondition(DnActorHandle hActor);
#endif // PRE_ADD_49166
#endif // _GAMESERVER

protected:
	virtual void OnSetParentSkillInfo() {};

#if defined(PRE_FIX_51048)
public:
	virtual void RemoveDebufAction(LOCAL_TIME LocalTime, float fDelta) {};
#endif // PRE_FIX_51048

public:
	bool IsMatchStateEffectIndex(int nStateEffectIndex);

#if defined(PRE_ADD_DARKLAIR_HEAL_REGULATION) && defined(_GAMESERVER) 
	float CalcModifiedHealValue( float fModifyValue );
#endif

#if defined( _GAMESERVER )
	void CalcHealValueLimit( const STATE_BLOW::emBLOW_INDEX emBlowIndex, float & fModifyValue );
#endif
#if defined(PRE_ADD_DRAGON_FELLOWSHIP)
#if defined( _GAMESERVER )
	static bool CheckEffectIgnoreMapType(GlobalEnum::eMapTypeEnum eMapType, GlobalEnum::eMapSubTypeEnum eSubMapType);
#else
	static bool CheckEffectIgnoreMapType();
#endif
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
};