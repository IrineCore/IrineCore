#pragma once

#include "DnParts.h"
class CDnPartsHair : public CDnParts
{
public:
	CDnPartsHair();
	virtual ~CDnPartsHair();

protected:
	DWORD m_dwColor;

public:
	virtual bool Initialize( int nPartsTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false, int nLookItemID = ITEMCLSID_NONE );
	virtual bool CreateObject( DnActorHandle hActor, const char *szAniName );
//	virtual CDnItem &operator = ( TItem &e );

	void UpdateColorParam( DWORD dwColor );
};
