#pragma once

#include "DnMonsterActor.h"
#include "MAStaticMovement.h"
#include "MASingleBody.h"
#include "MAMultiDamage.h"


struct BasiliskProperty 
{
	// Monster group ID to summon when climbing up a pillar
	int	nGenerationMonsterGroupID;
	// Monster count to summon when climbing up a pillar
	int nGenerationMonsterCount;
	// When summoning a monster, do not summon any more if current actor count is at or above this number.
	int nMaxActorSize;

	// Durability per armor
	float fArmorDurability;

	// Defense percentage per armor
	float	fArmorDefensePercent;

	// Armor count
	float fArmorCnt;

	// Ranged attack defense percentage
	float fRangeDefensePercent;

	// Armor durability reduction when hit normally
	float fArmorDurabilityDamage;

	// Armor durability loss when falling to the ground 
	float	fFall_to_the_ground_damage;

	// Number of legs
	float	fLegCnt;

	// Damage taken when a leg is hit once
	float	fLegDurabilityDamage;

	// Durability per leg
	float	fLegDurability;

	// Leg durability recovery value per unit time
	float fLegDurabilityRepairValue;

	// Tick count for recovering leg durability
	float fLegDurabilityRepairTime;

	// How much leg durability must drop before losing balance.
	float	fLoseBalanceValue;

	// How much leg durability must drop before falling out of the lost-balance state
	float	fFallDownValue;

};

class CDnBasiliskMonsterActor : public TDnMonsterActor< MASingleBody, MAMultiDamage >, public TBoostMemoryPool< CDnBasiliskMonsterActor >
{
public:
	CDnBasiliskMonsterActor( CMultiRoom *pRoom, int nClassID );
	virtual ~CDnBasiliskMonsterActor();

	virtual MAMovementBase* CreateMovement();
	virtual bool Initialize();
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct );
	virtual void ProcessDie( LOCAL_TIME LocalTime, float fDelta );
	virtual void CmdAction( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, bool bCheckOverlapAction = true, bool bFromStateBlow = false, bool bSkillChain = false );

	//virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	virtual float CalcDamage( DnActorHandle hHitter, SHitParam &HitParam  );

	// Additional Packet Message
	// Damage Packet
	virtual void OnDamageWriteAdditionalPacket( CPacketCompressStream *pStream );
	virtual void OnDamageReadAdditionalPacket( CPacketCompressStream *pStream );

public:
	void SetMeshDiffuseColor(const char* szMeshName, EtColor& clr);
	void ClearMeshDiffuseColor() { m_MeshDiffuseColorMap.clear(); }

	void OnFallToTheGround();

	bool	IsLoseBalance() 
	{
		if ( m_fLegTotalDurablility <= m_Property.fLegDurability*m_Property.fLegCnt-m_Property.fLoseBalanceValue )
			return true; 

		return false;
	}
	bool	IsFallDown() 
	{
		if ( m_fLegTotalDurablility <= m_Property.fLegDurability*m_Property.fLegCnt-m_Property.fLoseBalanceValue-m_Property.fFallDownValue )
			return true; 

		return false;
	}
	inline std::string GetPrevAction() { return m_szPrevAction; }
	inline BasiliskProperty& GetProperty() { return m_Property; }

private:
	void _InitRegionDamage();
	bool _LoadProperty(const char* szFile);
	float _CalcDamage( int nOverTurned, int AttackType, int AttackFoot, float nOriginalDamage, int nArmorCnt );

protected:
	float m_fShakeDelta;
	int m_nCustomParamIndex;
	std::string m_szPrevAction;

	
	std::map<std::string, EtColor> m_MeshDiffuseColorMap;
	std::vector<EtAniObjectHandle>	m_VecShellObject;

	lua_State* m_pLua;
	BasiliskProperty	m_Property;

	struct LEG_INFO
	{
		std::string szBoneName;
		std::string szSubMeshName;
		//float fLegDurability; // Durability of each leg	
	};

	typedef std::map<std::string, LEG_INFO>		LEG_INFO_DAMAGE_MAP;
	typedef LEG_INFO_DAMAGE_MAP::iterator		LEG_INFO_DAMAGE_MAP_IT;

	LEG_INFO_DAMAGE_MAP	m_LegDamageMap;
	
	// Sum of all shell durability (shield energy...)
	float	m_fArmorTotalDurability;

	// Number of living (?) shells
	int		m_nArmorCnt;

	// Leg energy...
	float	m_fLegTotalDurablility;

	// Last recovery time
	LOCAL_TIME	m_nLastLegRepairTime;


	
};