#include "Stdafx.h"
#include "DnTooltipPlateDlg.h"
#include "DnTableDB.h"

CDnTooltipPlateDlg::CDnTooltipPlateDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pTextBox( NULL )
{
}

CDnTooltipPlateDlg::~CDnTooltipPlateDlg()
{
}

void CDnTooltipPlateDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "TooltipPlateDlg.ui" ).c_str(), bShow );
}

void CDnTooltipPlateDlg::InitialUpdate()
{
	m_pTextBox = GetControl<CEtUITextBox>( "ID_TEXTBOX" );
}

void CDnTooltipPlateDlg::Show(bool bShow)
{
	CEtUIDialog::Show( bShow );
}

void CDnTooltipPlateDlg::SetGlyphSlot( int nSlotIndex )
{
	wchar_t szString[256];
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSLOT );

	int nID = pSox->GetItemID( nSlotIndex );

	int nSlotType = pSox->GetFieldFromLablePtr( nID, "_slotType" )->GetInteger();
	int nLevel = pSox->GetFieldFromLablePtr( nID, "_LevelLimit" )->GetInteger();
	int nIsCash = pSox->GetFieldFromLablePtr( nID, "_isCash" )->GetInteger();

	m_pTextBox->ClearText();

	wsprintf( szString, L" %s : ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8038) );	// Crest limit

	m_pTextBox->AddText( szString, textcolor::TOMATO );

	if( eGlyphType::GLYPETYPE_ENCHANT == nSlotType )
		m_pTextBox->AppendText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8039), textcolor::WHITE );	// Enhancement crest
	else if( eGlyphType::GLYPETYPE_SKILL == nSlotType )
		m_pTextBox->AppendText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8040), textcolor::WHITE );	// Skill heraldry
	else if( eGlyphType::GLYPETYPE_SPECIALSKILL == nSlotType )
		m_pTextBox->AppendText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8041), textcolor::WHITE );	// Special Skill Heraldry
	else if( eGlyphType::GLYPHTYPE_ALL == nSlotType )
		m_pTextBox->AppendText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9030), textcolor::WHITE );	// All heraldry crests equippable
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	else if( eGlyphType::GLYPHTYPE_DRAGON == nSlotType )
		m_pTextBox->AppendText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8099), textcolor::WHITE );	// Special Enhancement Heraldry
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)

	wsprintf( szString, L" %s : ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9022) );
	m_pTextBox->AddText( szString, textcolor::TOMATO );

	swprintf_s( szString, _countof(szString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 728 ), nLevel );
	m_pTextBox->AppendText( szString, textcolor::WHITE );

	wsprintf( szString, L" %s : ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 763) );
	m_pTextBox->AddText( szString, textcolor::TOMATO );

	if( nIsCash )
	{
		swprintf_s( szString, _countof(szString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8043 ), 0 );
		m_pTextBox->AppendText( szString, textcolor::WHITE );	
	}
	else
	{
		m_pTextBox->AppendText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8042), textcolor::WHITE );
	}
}