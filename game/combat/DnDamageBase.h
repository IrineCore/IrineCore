#pragma once

#include "DnWeapon.h"
class CDnDamageBase
{
public:
	enum DamageObjectTypeEnum {
		Unknown = 0,
		Actor,
		Prop,
	};

	enum DistanceTypeEnum
	{
		Melee = 0,
		Range,
	};

#ifdef _GAMESERVER
	struct SHitParam {
		std::string szActionName;
		LOCAL_TIME RemainTime;
		float fDamage;
		float fDurability;
		float fStiffProb;
		EtVector3 vVelocity;
		EtVector3 vResistance;
		DnActorHandle hHitter;
		DnWeaponHandle hWeapon;
		bool bFirstHit;
		EtVector3 vPosition;
		EtVector3 vViewVec;
		std::vector<int>	vBoneIndex;
		int nDamageType;
		CDnWeapon::HitTypeEnum HitType;
		int nSkillSuperAmmorIndex;
		int nSkillSuperAmmorDamage;
		int nSuperAmmorDelay;
		CDnState::ElementEnum HasElement;
		int iElementAttack;
		char cAttackType;
		bool bSuccessNormalDamage;
		DistanceTypeEnum	DistanceType;
		bool bIgnoreCanHit;
		bool bIgnoreParring;
		bool bFromProjectileSkill;		// #23818 Distinguish whether hitparam comes from a hit signal of a projectile fired by a skill. Since attack items with prefix skills have an "on normal attack" condition..

		// For fired projectiles, copy and store the shooter's state at the time of firing in the projectile object 
		// Use that value when calculating damage.
		bool bFromProjectile;
		
		// For normal hit, -1 is the default value and needs no change, but
		// Created and set in Projectile. EndSignalTime in IsHittable function
		// Before distinguishing, check if UniqueID matches first and ignore if different.
		int iUniqueID;

		INT64 nCalcDamage; //Stored to know the hit actor's damage during OnTargetHit.
		int nHitLimitCount;	//Limit hit count
#if defined( PRE_ADD_LOTUSGOLEM )
		bool bIgnoreShowDamage;	
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
#ifdef PRE_ADD_MONSTER_CATCH
		bool bReleaseCatchActor;
#endif // #ifdef PRE_ADD_MONSTER_CATCH
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
		bool bStageLimit;
#endif

		int nWeightValueLimit;	// For Academic Force Out [2011/07/04 semozz]
		bool bBreakSuperAmmor;	// Activated when critical hits via breaking point (super armor break) so critical resistance is not calculated

		SHitParam() 
		: vVelocity( 0.f, 0.f, 0.f ) 
		, vResistance( 0.f, 0.f, 0.f )
		, vPosition( 0.f, 0.f, 0.f )
		, vViewVec( 0.f, 0.f, 0.f )
		, iUniqueID( -1 )
		{
			fDamage = 0.f;
			fDurability = 0.f;
			fStiffProb = 0.f;
			RemainTime = 0;
			hWeapon.Identity();
			bFirstHit = true;
			vBoneIndex.clear();
			HitType = CDnWeapon::HitTypeEnum::Normal;
			nDamageType = 0;
			nSkillSuperAmmorIndex = -1;
			nSkillSuperAmmorDamage = 0;
			HasElement = CDnState::ElementEnum_Amount;
			iElementAttack = 0;
			cAttackType = 0;
			bSuccessNormalDamage = true;
			nSuperAmmorDelay = 0;
			DistanceType	= DistanceTypeEnum::Melee;
			//nProjectileAttackMin = 0;
			//nProjectileAttackMax = 0;
			bFromProjectile = false;
			bIgnoreParring = false;
			bIgnoreCanHit = false;
			bFromProjectileSkill = false;

			nCalcDamage = 0;
			nHitLimitCount = 0;
#if defined( PRE_ADD_LOTUSGOLEM )
			bIgnoreShowDamage = false;
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
#ifdef PRE_ADD_MONSTER_CATCH
			bReleaseCatchActor = false;
#endif // #ifdef PRE_ADD_MONSTER_CATCH
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
			bStageLimit = false;
#endif

			nWeightValueLimit = 0;
			bBreakSuperAmmor = false;
		}
	};
#else
	struct SHitParam {
		std::string szActionName;
		DnWeaponHandle hWeapon;
		INT64 nDamage;
		CDnWeapon::HitTypeEnum HitType;
		EtVector3 vPosition;
		EtVector3 vVelocity;
		EtVector3 vResistance;
		EtVector3 vViewVec;
		float fStiffDelta;
		float fDownDelta;
		bool bFirstHit;
		bool bSuccessNormalDamage;
		bool bRecvOnDamage;
		INT64 nCurrentHP;
		DistanceTypeEnum	DistanceType;
		int nHitLimitCount;

		CDnState::ElementEnum HasElement;
#if defined( PRE_ADD_LOTUSGOLEM )
		bool bIgnoreShowDamage;
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
		bool bStageLimit;
#endif
#ifdef PRE_FIX_77172
		bool bKeepJumpMovement;
#endif // PRE_FIX_77172

		// #32722 Fix issue where only the client cancels the skill action if hit on the server before the client's skill-use packet arrives.
		int iCanceledSkillIDInServer;

		SHitParam()
			: vVelocity( 0.f, 0.f, 0.f )
			, vResistance( 0.f, 0.f, 0.f )
			, vPosition( 0.f, 0.f, 0.f )
			, vViewVec( 0.f, 0.f, 0.f )
		{
			nDamage					= 0;
			HitType					= CDnWeapon::Normal;
			fStiffDelta				= 0.f;
			fDownDelta				= 0.f;
			bFirstHit				= false;
			bSuccessNormalDamage	= true;
			bRecvOnDamage			= true;
			DistanceType = Melee;
			nCurrentHP = 0;
			nHitLimitCount = 0;

			HasElement = CDnState::ElementEnum_Amount;
#if defined( PRE_ADD_LOTUSGOLEM )
			bIgnoreShowDamage = false;
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
			bStageLimit = false;
#endif
#ifdef PRE_FIX_77172
			bKeepJumpMovement = false;
#endif // PRE_FIX_77172
			iCanceledSkillIDInServer = 0;
		}
	};
#endif


	CDnDamageBase() { m_DamageObjectType = Unknown; }
	CDnDamageBase( DamageObjectTypeEnum Type ) { m_DamageObjectType = Type; }
	virtual ~CDnDamageBase() {}

protected:
	DamageObjectTypeEnum m_DamageObjectType;

public:
	DamageObjectTypeEnum GetDamageObjectType() { return m_DamageObjectType; }
	void SetDamageObjectType( DamageObjectTypeEnum Type ) { m_DamageObjectType = Type; }

	virtual void			OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam ) {}
	virtual DWORD			GetDamageObjectUniqueID() { return -1; }
	virtual DnActorHandle	GetActorHandle( void ) = 0;
	virtual void			OnSetPartsHP( DnActorHandle hActor, const int iPartsTableID, const int iCurHP , const int iHitterID){}

};
