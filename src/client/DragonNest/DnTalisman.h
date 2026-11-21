#pragma once

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnUnknownRenderObject.h"
#include "DnItem.h"

class CDnTalisman : public CDnUnknownRenderObject< CDnTalisman >,
                    public CDnItem
{
public:
	CDnTalisman();
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
	std::wstring m_wszExplainString;

public:
	static DnTalismanHandle CreateTalisman( int nTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false, int nLookItemID = ITEMCLSID_NONE );

public:
	virtual bool Initialize( int nTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false, int nLookItemID = ITEMCLSID_NONE );

	TalismanTypeEnum GetTalismanType() const { return m_TalismanType; }
	int GetTalismanIndex() const { return m_nTalismanIndex; }
	float GetExpectation() const { return m_fExpectation; }
	void SetEfficiency( float fEfficiency ) { m_fEfficiency = fEfficiency; }
	float GetEfficiency() const { return m_fEfficiency; }

	bool IsExpireItem() const;
	void GetTalismanExplainString( std::wstring &wszOut, float fEfficiency = 1.0f );
};

#endif // PRE_ADD_TALISMAN_SYSTEM
