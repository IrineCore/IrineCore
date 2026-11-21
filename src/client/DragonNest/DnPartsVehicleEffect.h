#pragma once

// Exceptional case where parts do not need to be constructed separately. < Named Parts for classification, but does not inherit DnParts > Set this way because it relates to character equip.
// Exceptional case where it affects the mount through the character without directly affecting the character, while the object itself is managed by the character. <Different from existing mount slots.>


#include "DnUnknownRenderObject.h"
#include "DnItem.h"

class CDnPartsVehicleEffect : public CDnUnknownRenderObject< CDnPartsVehicleEffect >, 
	public CDnItem
{
public:
	CDnPartsVehicleEffect();
	virtual ~CDnPartsVehicleEffect();

	enum VehicleEffectType {
		None,
		ChangeWeaponAction,
	};

protected:
	VehicleEffectType m_EffectType;
	std::string m_szChangeVehicleActionStr;

public:
	static DnPartsVehicleEffectHandle CreatePartsVehicleEffect( int nTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false, int nLookItemID = ITEMCLSID_NONE );

public:
	virtual bool Initialize( int nTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false, int nLookItemID = ITEMCLSID_NONE );

	VehicleEffectType GetVehicleEffectType() { return m_EffectType; }
	const char *GetChangeVehicleActionStr() { return m_szChangeVehicleActionStr.c_str(); }
};
