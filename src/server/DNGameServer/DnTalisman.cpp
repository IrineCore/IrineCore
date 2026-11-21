#include "stdafx.h"

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnTalisman.h"
#include "DNTableFile.h"
#include "DnTableDB.h"

DECL_DN_SMART_PTR_STATIC( CDnTalisman, MAX_SESSION_COUNT, 100 )

CDnTalisman::CDnTalisman( CMultiRoom *pRoom )
: CDnUnknownRenderObject( pRoom, false )
, CDnItem( pRoom )
, m_TalismanType( Normal )
, m_nTalismanIndex( 0 )
, m_fExpectation( 0.0f )
, m_fEfficiency( 1.0f )
{
}

CDnTalisman::~CDnTalisman()
{
}

DnTalismanHandle CDnTalisman::CreateTalisman( CMultiRoom *pRoom, int nTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound )
{
	if( nTableID < 1 ) return CDnTalisman::Identity();

	DnTalismanHandle hTalisman = (new CDnTalisman(pRoom))->GetMySmartPtr();
	if( !hTalisman ) return CDnTalisman::Identity();

	hTalisman->Initialize( nTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound );
	return hTalisman;
}

bool CDnTalisman::Initialize( int nTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TTALISMAN );
	if( !pSox )
	{
		g_Log.Log( LogType::_FILELOG, L"talismanTable.dnt failed\r\n" );
		return false;
	}
	if( !pSox->IsExistItem( nTableID ) ) return false;

	m_TalismanType = (TalismanTypeEnum)pSox->GetFieldFromLablePtr( nTableID, "_Type" )->GetInteger();
	m_nTalismanIndex = pSox->GetFieldFromLablePtr( nTableID, "_Index" )->GetInteger();
	m_fExpectation = pSox->GetFieldFromLablePtr( nTableID, "_Expectation" )->GetFloat();
	m_fEfficiency = 1.0f;

	CDnItem::Initialize( nTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound );

	return true;
}

#endif // PRE_ADD_TALISMAN_SYSTEM
