#pragma once

#include "DnMonsterActor.h"
#include "MAWalkMovementNav.h"
#include "MASingleBody.h"
#include "MAMultiDamage.h"

class CDnPartsMonsterActor : public TDnMonsterActor< MASingleBody, MAMultiDamage >
{
public:
	CDnPartsMonsterActor( int nClassID, bool bProcess = true );
	virtual ~CDnPartsMonsterActor();

	virtual MAMovementBase* CreateMovement()
	{
		MAMovementBase* pMovement = new MAWalkMovementNav();
		return pMovement;
	}

public:
	virtual bool IsPartsMonsterActor(){ return true; }
#ifdef PRE_ADD_CHECK_PART_MONSTER	
	virtual bool IsEnablePartsHPDisplay() { return m_bIsEnablePartsHPDisplay; }
#endif

	// Actor Message
	virtual void OnDestroyParts(const MonsterParts& partsInfo, int nPartsIndex);
	virtual void OnRefreshParts(const MonsterParts& partsInfo, int nPartsIndex);

	// ActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );

	virtual void OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam );
	virtual void OnAddMonsterParts(const MonsterParts::_Info& partsInfo);

	virtual bool Initialize();
	virtual void OnMultiDamage(MonsterParts& Parts);

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	void		 CmdMixDestroyPartsAction();

	void SetUseDamageColor(bool b) { m_bUseDamagePartsColor = b; }
	void ClearMeshAmbientColor() { m_MeshAmbientColorMap.clear(); }
	void SetMeshAmbientColor(const char* szMeshName, EtColor& clr);
	void SetUseDamageShake(bool b) { m_bUseDamagePartsShake = b; }
	
#ifdef PRE_ADD_DAMAGEDPARTS
	// 60073.
	void SetUseDamagedPartsEmissiveColor( bool b ){ 
		m_bUseDamagePartsEmissiveColor = b; 
		if( b )
			m_bUseDamagePartsColor = false; // Enabling hit Emissive forcibly disables existing Ambient Color application.
	}
	void SetUseHPPercentEmissiveColor( bool b ){ m_bUseHPPercentEmissiveColor = b; }
#endif
	// TransAction Message
	virtual void OnDispatchMessage( DWORD dwActorProtocol, BYTE *pPacket );

protected:
#ifdef PRE_ADD_CHECK_PART_MONSTER	
	void CheckEnablePartsHpDisplay();
#endif
	virtual void RefreshStateColor();

	float	m_fDamageBlinkColorDelta;
	float	m_fDamageShakeDelta;

	float	m_fDamageBlinkColorTime;
	float	m_fDamageShakeTime;
	float	m_fDamageScaleFactor;
	EtColor m_DamageBlinkColor;

	bool	m_bUseDamagePartsShake;
	bool	m_bUseDamagePartsColor;
	int		m_nAmbientParamIndex;
	int		m_nDiffuseParamIndex;

#ifdef PRE_ADD_DAMAGEDPARTS
	// 60073.
	int     m_nDamagedEmissiveParamIndex;
	bool	m_bUseDamagePartsEmissiveColor; 
	int     m_nHPPercentEmissiveParamIndex;
	bool    m_bUseHPPercentEmissiveColor;
#endif

#ifdef PRE_ADD_CHECK_PART_MONSTER
	std::vector<int> m_vEnablePartsGaugeList;
	bool	m_bIsEnablePartsHPDisplay;
#endif

#define PRE_FIX_CHANGE_EMISSIVE
#ifdef PRE_FIX_CHANGE_EMISSIVE
	enum ChangeEmissiveStep
	{
		None = -1,
		EmissiveDecrease = 0,
		EmissiveIncrease = 1,
		EmissiveStandBy = 2,
	};
	int		m_nChangeEmissiveStep;
	float	m_fEmissivePower;
	float	m_fEmissivePowerRange;
	float	m_fEmissiveAniSpeed;
	float	m_fCurrentEmissive;
	bool	m_bExistEmissive;
	float	m_fDeltaTime;
	bool	m_bImmediatelyChangeEmissive;
	std::string m_szChangeEmissiveTexture;
#else
	bool	m_bChangeEmissive;
	float	m_fEmissivePower;
	float	m_fEmissivePowerRange;
	float	m_fEmissiveAniSpeed;
	bool	m_bImmediatelyChangeEmissive;
	std::string m_szChangeEmissiveTexture;
#endif


	std::map<std::string, EtColor> m_MeshAmbientColorMap;


#ifdef PRE_ADD_DAMAGEDPARTS 
	// #60073	
	std::map<std::string, float> m_mapDamagedEmissivePower;
#endif
	

	enum DestroyActionDefType
	{
		ActionName,
		ActionBoneName,
		FixBoneName,
		MaxDestroyActionDef
	};

	enum DestroyPartsState
	{
		DPS_None,
		DPS_Destroy,
		DPS_Regeneration
	};

	struct SDestroyActorActionInfo
	{
		std::string			names[MaxDestroyActionDef];
		std::string			meshName;
		DestroyPartsState	curState;
		EtColor				destroyAmbient;
		EtColor				destroyDiffuse;

		SDestroyActorActionInfo()
		{
			curState = DPS_None;
		}

		void Set(const std::string& act, const std::string& actBone, const std::string& fixBone)
		{
			names[ActionName]		= act;
			names[ActionBoneName]	= actBone;
			names[FixBoneName]		= fixBone;
		}
	};

	std::map<int, SDestroyActorActionInfo>		m_DestroyActionInfoList;	// key : _ItemID
};