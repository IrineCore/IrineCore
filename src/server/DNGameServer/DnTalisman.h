#pragma once

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnItem.h"
#include "DnUnknownRenderObject.h"

class CDnTalisman : public CDnUnknownRenderObject< CDnTalisman, MAX_SESSION_COUNT >, public CDnItem {
public:
	CDnTalisman( CMultiRoom *pRoom );
	virtual ~CDnTalisman();

	enum TalismanTypeEnum {
		Normal = 0,
		Skill = 1,
		Drop = 2,
	};

protected:
	TalismanTypeEnum m_TalismanType;
	int m_nTalismanIndex;
	float m_fExpectation;
	float m_fEfficiency;

public:
	static DnTalismanHandle CreateTalisman( CMultiRoom *pRoom, int nTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false );

public:
	virtual bool Initialize( int nTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false );

	TalismanTypeEnum GetTalismanType() const { return m_TalismanType; }
	int GetTalismanIndex() const { return m_nTalismanIndex; }
	float GetExpectation() const { return m_fExpectation; }
	void SetEfficiency( float fEfficiency ) { m_fEfficiency = fEfficiency; }
	float GetEfficiency() const { return m_fEfficiency; }
};

#endif // PRE_ADD_TALISMAN_SYSTEM
