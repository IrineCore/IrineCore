#pragma once

#include "DnState.h"


class CDnActor;
class CDnActorState : public CDnState
{
public:
	CDnActorState();
	virtual ~CDnActorState();

	static float s_fMaxStiffTime;
	static float s_fCriticalDamageProb;

	enum ActorStateEnum {
		None = 0,
		Stay = 0x0001,
		Move = 0x0002,
		Attack = 0x0004,
		Hit = 0x0008,
		Air = 0x0010,
		Down = 0x0020,
		Stun = 0x0040,
		Stiff = 0x0080,
		IgnoreCantAction = 0x0100,
		IgnoreBackMoveSpeed = 0x0200, // #31110 Prevent halving move speed when moving backward in Move State (e.g., Top Spinning).

		ActorStateEnum_Amount = 11,
	};

	enum ActorCustomStateEnum {
		Custom_None = 0,
		Custom_Ground = 0x0001,
		Custom_Fly = 0x0002,
		Custom_UnderGround = 0x0004,
		Custom_LifeSkill = 0x0008,
		Custom_Fly2 = 0x0010,
		ActorCustomStateEnum_Amount = 5,
	};


	// Actor's special state triggered by a state effect (buff/debuff with duration)
	enum ActorStateEffectEnum
	{
		StateEffect_None = 0,
		Cant_Move = 0x00000001,
		Cant_Use_Skill = 0x00000002,
		Cant_Use_Item = 0x00000004,
		Cant_AttackAction = 0x00000008,
		
		ActorStateEffectEnum_Amount = 5,
	};

	enum ActorMaterialEnum {
		Flesh,
		Metal,
		Solid,
		Wood,
		ActorMaterialEnum_Amount = 4,
	};

	enum ActorTypeEnum {
		Warrior =  0,
		Archer,
		Soceress,
		Cleric,
#if defined(PRE_ADD_ACADEMIC)
		Academic,
#else
		Reserved1,
#endif	// #if defined(PRE_ADD_ACADEMIC)
#ifdef PRE_ADD_KALI
		Kali,
#else
		Reserved2,
#endif // #ifdef PRE_ADD_KALI
		Reserved3,
		Reserved4,
		Reserved5,
		Reserved6,
		SimpleRush,
		Normal,
		Defense,
		Disturbance,
		Range,
		Rush,
		Giant,
		Magic,
		Trap,
		Cannon,				// Actor type of cannon monster fired by player.
		NoAggro,
		NoAggroTrap,
		GoldMonster,		// Actor type 22 (0x16): CDnGoldMonsterActor
		// Boss starts here
		NESTBOSS = 69,
		Basilisk = 70,
		Beholder,

		// Dragon
		BlackDragon, 
		GoldDragon,
		Manticore,
		Cerberos,
		SeaDragon,
		LotusGolem,
		GreenDragon,
		EndOfMonster,
//
		Vehicle = 80, // Rotha Mount system
		Pet,

		// NPC starts here
		Npc = 100,
		
		// Prop actors from here on
		PropActor = 150,
		ActorTypeEnum_Amount,

	};

	enum RefreshStateExtentEnum {
		RefreshBase = 0x01,
		RefreshEquip = 0x02,
		RefreshSkill = 0x04,
		RefreshAll = 0xFF,
	};

	enum StateTypeEnum {
		ST_Strength				= 0x00000001,
		ST_Agility				= 0x00000002,
		ST_Intelligence			= 0x00000004,
		ST_Stamina				= 0x00000008,
		ST_HP					= 0x00000010,
		ST_SP					= 0x00000020,
		ST_AttackP				= 0x00000040,
		ST_AttackM				= 0x00000080,
		ST_ElementAttack		= 0x00000100,
		ST_DefenseP				= 0x00000200,
		ST_DefenseM				= 0x00000400,
		ST_ElementDefense		= 0x00000800,
		ST_MoveSpeed			= 0x00001000,
		ST_DownDelay			= 0x00002000,
		ST_Stiff				= 0x00004000,
		ST_StiffResistance		= 0x00008000,
		ST_Critical				= 0x00010000,
		ST_CriticalResistance	= 0x00020000,
		ST_Stun					= 0x00040000,
		ST_StunResistance		= 0x00080000,
		ST_SuperAmmor			= 0x00100000,
		ST_RecoverySP			= 0x00200000,
		ST_FinalDamage			= 0x00400000,
		ST_Spirit				= 0x00800000,
		ST_SafeZoneMoveSpeed	= 0x01000000,
		ST_AddExp				= 0x02000000,
		ST_All					= 0xFFFFFFFF,
	};

	enum ActorPressEnum {
		Press_Circle,	// Default type
		Press_Capsule,	// Cerberus
		//Press_Ellipse, // Currently not supported.
		Press_NoneSameCircle,	// Push out only when different type
		ActorPressEnum_Amount = 3,
	};

	// Type used in AddBlowState function. Added to separate lists depending on the specified type.
	enum AddBlowStateType
	{
		Equip_Buff_Level,	 // State[0] : Level where equipment and buffs are calculated
		Equip_Skill_Level,   // State[0] : Level where special state effects are calculated after equipment and buffs
		Skill_Level,		 // state[1] : last level to calculate the skill
	};

	static int s_nActorStateIndex[ActorStateEnum_Amount];
	static char *s_szActorStateString[ActorStateEnum_Amount];
	static ActorStateEnum String2ActorStateEnum( const char *szStr );

	static int s_nActorCustomStateIndex[ActorCustomStateEnum_Amount];
	static char *s_szActorCustomStateString[ActorCustomStateEnum_Amount];
	static ActorStateEnum String2ActorCustomStateEnum( const char *szStr );

	CDnActor *m_pActor;
protected:
	tstring m_szName;

#ifdef PRE_ADD_BESTFRIEND
	INT64 m_BFserial;
	tstring m_szBestfriendName;
#endif

	CDnState m_BaseState;
	CDnState m_StateStep[2];

	// Fixed state
	int m_nActorTableID;
	MSDT_DECL(int) m_nLevel;		// Level
	ActorTypeEnum m_ActorType;	// Actor type
	ActorMaterialEnum m_Material;// Actor material
	MSDT_DECL(float) m_fWeight;	// Weight
	MSDT_DECL(int) m_nHeight;	// Key
	MSDT_DECL(int) m_nUnitSize;	// Unit size
	ActorPressEnum m_Press;
	int m_nUnitSizeParam1;	// If capsule, horizontal radius on the xz-plane
	int m_nUnitSizeParam2;	// If capsule, vertical radius on the xz-plane

#ifndef _GAMESERVER
#define RENDER_PRESS_hayannal2009
#ifdef RENDER_PRESS_hayannal2009
public:
	// Render buffer. Uses only 1 for circle, and uses all for capsule.
	SPrimitiveDraw3D m_PressVertices1[10];
	SPrimitiveDraw3D m_PressVertices2[10];
	SPrimitiveDraw3D m_PressVertices3[4];
protected:
#endif
#endif

	MSDT_DECL(float) m_fRotateAngleSpeed; // Rotation speed
	MSDT_DECL(int) m_nPressLevel;		// Push level

	MSDT_DECL(INT64) m_nHP;		// HP: Do not modify directly; use SetHP() instead
	MSDT_DECL(int) m_nSP;		// SP Do not modify directly; use SetSP()

	ActorStateEnum m_State;		// Actor state
	ActorCustomStateEnum	m_CustomState;

	ActorStateEffectEnum m_StateEffect;				// Current status from actor state effect.

	MSDT_DECL(bool) m_bHittable;
	MSDT_DECL(bool) m_bMovable;
	MSDT_DECL(bool) m_bRotatable;

	typedef std::list<CDnState*>		BLOW_STATE_LIST;
	typedef BLOW_STATE_LIST::iterator	BLOW_STATE_LIST_ITER;
	BLOW_STATE_LIST m_listBlowState;
	BLOW_STATE_LIST m_listBeforePostBlowState;
	BLOW_STATE_LIST m_listPostBlowState;

#ifdef PRE_ADD_SKILLBUF_RENEW
	bool m_bCopiedFromSummoner;
	BLOW_STATE_LIST m_listBuffBlowState;
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW

	enum StateListTypeEnum {
		BaseList = 0x0001,
		WeaponList = 0x0002,
		DefenseList = 0x0004,
		AccessoryList = 0x0008,
		StateEffectList = 0x0010,
		PostStateEffectList = 0x0020,
		SetItemList = 0x0040,
		AppellationList = 0x0080,
		GlyphList = 0x0100,
		// #32220 Special state effects used by items are calculated before post state effect calculation.
		// Currently used only for physical attack and magic attack.
		BeforePostStateEffectList = 0x0200,
		PetAbility = 0x0400,
		TalismanList = 0x0800,

		AllList = 0xFFFF,
	};

public:
	void AddBlowState( CDnState *pState, AddBlowStateType eAddBlowStateType = Equip_Buff_Level , bool bIsBuff = false );
	void DelBlowState( CDnState *pState );

protected:
	virtual void OnLevelUp( int nNewLevel, int iLevelUpAmount ) {}
	virtual void OnAddExperience( int nAddExperience, int nLogCode, INT64 biFKey ) {}	// -> Also set biFKey according to nLogCode (see eCharacterExpChangeCode)
	virtual void OnRefreshState() {}

	virtual void CalcBaseState( StateTypeEnum Type = ST_All ) {}
	virtual void CalcState( RefreshStateExtentEnum Extent, StateTypeEnum Type ) {}
	virtual void GetStateList( int ListType, CDnState::ValueType Type, std::vector<CDnState *> &VecList );

#if defined (PRE_ADD_BESTFRIEND)
	bool IsApplyState(CDnActor *pActor, CDnWeapon* pWeapon);
#endif

#ifdef PRE_ADD_SKILLBUF_RENEW
	virtual void GetBuffStateList( CDnState::ValueType Type, std::vector<CDnState *> &VecList );
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW

	virtual void RefreshAdditionalState( RefreshStateExtentEnum &Extent, StateTypeEnum &Type ) {}

	void Initialize( CDnActor *pActor ) { m_pActor = pActor; }

	virtual void OnCalcEquipStep( StateTypeEnum Type, CDnState *pState ) {}

public:
	virtual void Initialize( int nClassID );
	void RefreshState( RefreshStateExtentEnum Extent = RefreshAll, StateTypeEnum Type = ST_All );
	virtual void ProcessState( LOCAL_TIME LocalTime, float fDelta ) {}

	void SetName( const TCHAR *szName ) { m_szName = szName; }
	TCHAR *GetName() const { return (TCHAR*)m_szName.c_str(); }
#ifdef PRE_ADD_BESTFRIEND
	void SetBFserial( INT64 serial ) { m_BFserial = serial; }
	INT64 GetBFserial() { return m_BFserial; }
	void SetBestfriendName( const TCHAR * szBFname ) { m_szBestfriendName.assign( szBFname ); }
	TCHAR *GetBestfriendName() const { return (TCHAR*)m_szBestfriendName.c_str(); }
#endif


	void SetLevel( int nValue );
	__forceinline int GetLevel() { return m_nLevel; }
	virtual int GetCalcLevel() { return m_nLevel; }

	__forceinline INT64 GetHP() { return m_nHP; }
	__forceinline int GetHPPercent() 
	{ 
		float fMaxHP = static_cast<float>(GetMaxHP());
		float fCurHP = static_cast<float>(GetHP());

		if( fMaxHP == 0.f )
			return 0;

		return static_cast<int>((fCurHP/fMaxHP)*100.0f);
	}

	__forceinline float GetHPPercentFloat()
	{ 
		float fMaxHP = static_cast<float>(GetMaxHP());
		float fCurHP = static_cast<float>(GetHP());

		if( fMaxHP == 0.f )
			return 0.f;

		return static_cast<float>(fCurHP/fMaxHP);
	}

#if defined(PRE_ADD_MISSION_COUPON) && (_GAMESERVER)
	void SetHP( INT64 nValue );
#else
	__forceinline void SetHP( INT64 nValue ) 
	{
		//---------------------------------------
		//[_debug]

		if (nValue > 0)
		{
		   m_nHP = nValue;
		}
		//---------------------------------------


		m_nHP = nValue;
		m_nHP = (m_nHP < 0) ? 0 : m_nHP;
	}	
#endif
		
	__forceinline int GetSP() { return m_nSP; }
	__forceinline int GetSPPercent() 
	{ 
		float fMaxSP = static_cast<float>(GetMaxSP());
		float fCurSP = static_cast<float>(GetSP());

		if( fMaxSP == 0.f )
			return 0;

		return static_cast<int>((fCurSP/fMaxSP)*100.0f);
	}

#if defined(PRE_ADD_MISSION_COUPON) && (_GAMESERVER)
	void SetSP( int nValue );
#else
	
	__forceinline void SetSP( int nValue ) 
	{ 
		//--------------------------------------
		//[_debug]
		//WCHAR wszBuf[MAX_PATH];
		//wsprintf( wszBuf, L"SetSP SP:%d", nValue );
		//CDnChatTabDlg::AddChatMessage( CHATTYPE_NORMAL, L"", wszBuf  );
		//GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", wszBuf  );
		//CDnInterface::GetInstance().AddChatMessage(CHATTYPE_NORMAL, L"", wszBuf  );   //CHATTYPE_SYSTEM

		//--------------------------------------

		//---------------------------------------
		//[_debug]
		if (nValue <= 0)
		{
			m_nSP = nValue;
		}

        if (nValue > 0)
        {
			m_nSP = nValue;
        }
		//---------------------------------------

		m_nSP = nValue; 
	}

#endif

	virtual float GetWeight() { return m_fWeight; }
	virtual void SetWeight( float fWeight ) { m_fWeight = fWeight; };
	__forceinline int GetHeight() { return m_nHeight; }
	__forceinline void SetHeight(int height) { m_nHeight = height; }

	__inline ActorPressEnum GetPress() { return m_Press; }
	__inline void SetPress(ActorPressEnum _pressType) { m_Press = _pressType; }
	__inline int GetPressLevel() { return m_nPressLevel; }
	__inline void SetPressLevel( int nPressLevel ) { m_nPressLevel = nPressLevel; };		// Used only to back up and restore stats separately when copying stats to a summon monster.

	__forceinline virtual void SetState( ActorStateEnum State ) { m_State = State; SetStateChangeTime(); }
	__forceinline int GetState() { return m_State; }

	__forceinline void SetCustomState( ActorCustomStateEnum State ) { m_CustomState = State; }
	__forceinline int GetCustomState() { return m_CustomState; }

	__forceinline void SetStateEffect( int State ) { m_StateEffect = (ActorStateEffectEnum)State; }
	__forceinline int GetStateEffect() { return m_StateEffect; }

	__forceinline ActorMaterialEnum GetMaterial() { return m_Material; }
	__forceinline void SetMaterial(ActorMaterialEnum _material) { m_Material = _material; }

	__forceinline ActorTypeEnum GetActorType() { return m_ActorType; }
	__forceinline void SetActorType( ActorTypeEnum Type ) { m_ActorType = Type; }

	__forceinline void SetHittable( bool bFlag ) { m_bHittable = bFlag; }
	__forceinline bool IsHittable() { return m_bHittable; }

	__forceinline void SetMovable( bool bFlag ) { m_bMovable = bFlag; }
	__forceinline bool IsMovable() 
	{ 
		if( (m_StateEffect & Cant_Move) )
			return false;
		else
			return m_bMovable;
	}

	__forceinline void SetRotatable( bool bFlag ) { m_bRotatable = bFlag; }
	__forceinline bool IsRotatable() { return m_bRotatable; }

	__forceinline int GetUnitSize() { return m_nUnitSize; }
	__forceinline int GetUnitSizeParam1() { return m_nUnitSizeParam1; }
	__forceinline int GetUnitSizeParam2() { return m_nUnitSizeParam2; }
	
	__forceinline void SetUnitSize(int nSize) { m_nUnitSize = nSize; }
	__forceinline void SetUnitSizeParam1(int nSize) { m_nUnitSizeParam1 = nSize; }
	__forceinline void SetUnitSizeParam2(int nSize) { m_nUnitSizeParam2 = nSize; }

	virtual void SetRotateAngleSpeed( float fRotateAngleSpeed ) { m_fRotateAngleSpeed = fRotateAngleSpeed; };
	virtual float GetRotateAngleSpeed() { return m_fRotateAngleSpeed; }

	virtual int GetMoveSpeed() { return CDnState::GetMoveSpeed(); }


	float GetLevelWeightValue();
	
	bool IsNeedPvPLevelWeight();
	float GetDefenseConstant();
	float GetCriticalConstant();
	float GetFinalDamageConstant();

	virtual bool IsDie();

	const CDnState *GetBaseState() { return &m_BaseState; }
	CDnState *GetStateStep( int nStep ) { return &m_StateStep[nStep]; }

	void CopyStateFromThis( DnActorHandle hActor, bool bDontCopySkillStateEffect = false );

	int GetActorTableID() const { return m_nActorTableID; }
	void SetActorTableID(int nTableID) { m_nActorTableID = nTableID; }
	int GetAttackPMaxWithoutSkill() { return m_StateStep[0].GetAttackPMax(); }
	int GetAttackMMaxWithoutSkill() { return m_StateStep[0].GetAttackMMax(); }
	int GetAttackMMinWithoutSkill() { return m_StateStep[0].GetAttackMMin(); }


protected:
	LOCAL_TIME m_StateChangeTime;
public:
	void SetStateChangeTime();// { if (m_pActor) m_StateChangeTime = m_pActor->GetLocalTime(); }
	LOCAL_TIME GetStateChangeTime() { return m_StateChangeTime;}

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	virtual void OnLevelChange() {};
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
};

#define CALC_STATE_VALUE_ABSOLUTE( itemvector, value, func ) \
	value = 0; \
	for( DWORD ADD_STATE_VALUE_I = 0; ADD_STATE_VALUE_I<itemvector.size(); ADD_STATE_VALUE_I++ ) { \
	value += itemvector[ADD_STATE_VALUE_I]->func;	\
	}

#define CALC_STATE_VALUE_RATIO( itemvector, standardvalue, value, func ) { \
	float FLOAT_STATE_RATIO = 0.f; \
	value = 0; \
	for( DWORD ADD_STATE_VALUE_I = 0; ADD_STATE_VALUE_I<itemvector.size(); ADD_STATE_VALUE_I++ ) { \
	FLOAT_STATE_RATIO += itemvector[ADD_STATE_VALUE_I]->func; \
	} \
	value = ( standardvalue * FLOAT_STATE_RATIO ); \
}

#define MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( itemvector, standardvalue, value, func , minlimit, maxlimit ) { \
	float FLOAT_STATE_RATIO = 0.f; \
	for( DWORD ADD_STATE_VALUE_I = 0; ADD_STATE_VALUE_I<itemvector.size(); ADD_STATE_VALUE_I++ ) { \
	FLOAT_STATE_RATIO += itemvector[ADD_STATE_VALUE_I]->func; \
	} \
	if(FLOAT_STATE_RATIO < minlimit) { \
	value += (int)( -(standardvalue * FLOAT_STATE_RATIO) + (standardvalue * minlimit) ); \
	} \
	if(FLOAT_STATE_RATIO > maxlimit ) { \
	value += (int)( -(standardvalue * FLOAT_STATE_RATIO) + (standardvalue * maxlimit) ); \
	} \
}

#define MODIFY_STATE_VALUE_LIMIT_ABSOLUTE( itemvector, value, func , minlimit, maxlimit ) { \
	float FLOAT_STATE_VALUE = 0.f; \
	for( DWORD ADD_STATE_VALUE_I = 0; ADD_STATE_VALUE_I<itemvector.size(); ADD_STATE_VALUE_I++ ) { \
	FLOAT_STATE_VALUE += itemvector[ADD_STATE_VALUE_I]->func; \
	} \
	if( FLOAT_STATE_VALUE < minlimit ) { \
	value += ( -FLOAT_STATE_VALUE + minlimit ); \
	} \
	if( FLOAT_STATE_VALUE > maxlimit ) { \
	value += ( -FLOAT_STATE_VALUE + maxlimit ); \
	} \
}

#define GATHER_RATIO( itemvector, value, func ) \
	value = 0.f; \
	for( DWORD ADD_STATE_VALUE_I = 0; ADD_STATE_VALUE_I<itemvector.size(); ADD_STATE_VALUE_I++ ) { \
	value += itemvector[ADD_STATE_VALUE_I]->func; \
	} \

template < class T, class T2 >
void CALC_STATE_LIMIT( T &Value, T2 Min, T2 Max )
{
	if( Value < Min ) Value = Min;
	else if( Value > Max ) Value = Max;
}
