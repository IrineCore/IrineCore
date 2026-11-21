#pragma once

#include "DnParts.h"
class CDnPartsHelmet : public CDnParts
{
public:
	CDnPartsHelmet();
	virtual ~CDnPartsHelmet();

protected:
	int m_nHairShowLevel;
	bool m_bAttachHeadBone;

public:
	virtual bool Initialize( int nPartsTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false, int nLookItemID = ITEMCLSID_NONE );

	int GetHairShowLevel() { return m_nHairShowLevel; }
	bool IsAttachHeadBone() { return m_bAttachHeadBone; }

	virtual bool IsLoadPartsAni();
};
