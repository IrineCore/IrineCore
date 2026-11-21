#include "StdAfx.h"
#include "DnPartsHair.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPartsHair::CDnPartsHair()
{
	m_dwColor = 0;
}

CDnPartsHair::~CDnPartsHair()
{
}

bool CDnPartsHair::Initialize( int nPartsTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound, int nLookItemID)
{
	if( CDnParts::Initialize( nPartsTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound, nLookItemID ) == false ) return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) return false;
	m_dwColor = (DWORD)pSox->GetFieldFromLablePtr( nPartsTableID, "_DefaultColor" )->GetInteger();

	return true;
}

bool CDnPartsHair::CreateObject( DnActorHandle hActor, const char *szAniName )
{
	bool bResult = CDnParts::CreateObject( hActor, szAniName);
	UpdateColorParam( m_dwColor );
	return bResult;
}
/*
CDnItem& CDnPartsHair::operator = ( TItem &e )
{
	DWORD dwColorBackup = m_dwColor;
	CDnParts::operator = (e);
if( m_dwColor == 0 ) { // Ignore if 0.
		m_dwColor = dwColorBackup;
	}
	return *this;
}
*/

void CDnPartsHair::UpdateColorParam( DWORD dwColor )
{
	if( !m_hObject ) {
		return;
	}
	int nCustomColor = m_hObject->AddCustomParam( "g_CustomColor" );
	if( nCustomColor != -1 )  {
		float fHairColor[3]={0,};
		CDnParts::ConvertR10G10B10ToFloat(fHairColor, dwColor);
		EtColor Color( fHairColor[0], fHairColor[1], fHairColor[2], 1.0f);
		m_hObject->SetCustomParam(nCustomColor, &Color);
	}
}
