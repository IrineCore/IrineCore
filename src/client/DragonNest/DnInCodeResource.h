#pragma once

#include "Singleton.h"
class CDnInCodeResource : public CSingleton<CDnInCodeResource> 
{
public:
	CDnInCodeResource();
	virtual ~CDnInCodeResource();

	// Overhead enhance and craft effects
	enum eHeadEffectEtcIndex
	{
		EtcIndex_Enchant = 0,
		EtcIndex_RandomItem,
		EtcIndex_Plate,
		EtcIndex_Compound,

#ifdef PRE_ADD_SECONDARY_SKILL
		EtcIndex_Cooking_Making,
		EtcIndex_Cooking_Finish,
		EtcIndex_Plant,
		EtcIndex_Harvest,
		EtcIndex_Water,
		EtcIndex_Fishing,
#endif // PRE_ADD_SECONDARY_SKILL

#ifdef PRE_ADD_SECONDARY_SKILL
		EtcIndex_Amount = 10,
#else // PRE_ADD_SECONDARY_SKILL
		EtcIndex_Amount = 4,
#endif // PRE_ADD_SECONDARY_SKILL
	};

protected:
	std::string m_szPlayerCommonEffectName;
	DnEtcHandle m_hPlayerCommonEffect;

	DnEtcHandle m_hGhostCommon[10];

	DnEtcHandle m_hHeadEffect[EtcIndex_Amount];
	EtTextureHandle m_hNullWhiteTexture;

public:
	void Initialize();
	void Finalize();

	DnEtcHandle CreatePlayerCommonEffect();
	DnEtcHandle CreateHeadEffect( eHeadEffectEtcIndex eType );

	EtTextureHandle GetNullWhiteTexture() { return m_hNullWhiteTexture; }


#ifdef PRE_ADD_CASH_COSTUME_AURA
	DnEtcHandle CreateCostumeAura(int nItemID);
#endif
};