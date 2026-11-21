#include "stdafx.h"
#include "DnGlyph.h"
#include "DNTableFile.h"
#include "DnTableDB.h"

DECL_DN_SMART_PTR_STATIC( CDnGlyph, MAX_SESSION_COUNT, 100 )


CDnGlyph::CDnGlyph( CMultiRoom *pRoom )
: CDnUnknownRenderObject( pRoom, false )
, CDnItem( pRoom )
{
	m_GlyphType = GlyphTypeEnum::None;
}

CDnGlyph::~CDnGlyph()
{
}

DnGlyphHandle CDnGlyph::CreateGlyph( CMultiRoom *pRoom, int nTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound )
{
	if( nTableID < 1 ) return CDnGlyph::Identity();

	DnGlyphHandle hGlyph = (new CDnGlyph(pRoom))->GetMySmartPtr();
	if( !hGlyph ) return CDnGlyph::Identity();

	hGlyph->Initialize( nTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount );
	return hGlyph;
}

bool CDnGlyph::Initialize( int nTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TGLYPHSKILL );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"GlyphSkillTable.ext failed\r\n");
		return false;
	}
	if( !pSox->IsExistItem( nTableID ) ) return false;

	CDnItem::Initialize( nTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound );

	return true;
}
