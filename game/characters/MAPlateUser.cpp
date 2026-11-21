#include "stdafx.h"
#include "MAPlateUser.h"

MAPlateUser::MAPlateUser()
{
	memset( m_bSelfDelete, 0, sizeof(m_bSelfDelete) );
}

MAPlateUser::~MAPlateUser()
{
	for( DWORD i=0; i<CDnGlyph::GlyphSlotEnum_Amount; i++ ) {
		DetachGlyph( (CDnGlyph::GlyphSlotEnum)i );
	}
}

bool MAPlateUser::AttachGlyph( DnGlyphHandle hGlyph, CDnGlyph::GlyphSlotEnum Index, bool bDelete )
{
	if( !hGlyph ) return false;

	CDnGlyph::GlyphSlotEnum SlotIndex = Index;

	if( m_hGlyph[SlotIndex] && m_hGlyph[SlotIndex] != hGlyph ) {
		MAPlateUser::DetachGlyph( SlotIndex );
	}
	m_hGlyph[SlotIndex] = hGlyph;
	m_bSelfDelete[SlotIndex] = bDelete;

	return true;
}

bool MAPlateUser::DetachGlyph( CDnGlyph::GlyphSlotEnum Index )
{
	if( !m_hGlyph[Index] ) return true;

	if( m_bSelfDelete[Index] ) {
		SAFE_RELEASE_SPTR( m_hGlyph[Index] );
		m_bSelfDelete[Index] = false;
	}
	m_hGlyph[Index].Identity();
	return true;
}

void MAPlateUser::Process( LOCAL_TIME LocalTime, float fDelta )
{
}
