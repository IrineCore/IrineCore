#include "StdAfx.h"

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnTooltipTalismanDlg.h"
#include "DnTableDB.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnTooltipTalismanDlg::CDnTooltipTalismanDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pTextBox( NULL )
{
}

CDnTooltipTalismanDlg::~CDnTooltipTalismanDlg()
{
}

void CDnTooltipTalismanDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "TooltipTalismanDlg.ui" ).c_str(), bShow );
}

void CDnTooltipTalismanDlg::InitialUpdate()
{
	m_pTextBox = GetControl<CEtUITextBox>( "ID_TEXTBOX" );
}

void CDnTooltipTalismanDlg::Show( bool bShow )
{
	CEtUIDialog::Show( bShow );
}

void CDnTooltipTalismanDlg::SetTalismanSlot( int nTableRowID, bool bCashSlot )
{
	if( !m_pTextBox ) return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TTALISMANSLOT );
	if( !pSox || !pSox->IsExistItem( nTableRowID ) )
		return;

	int nDescID = pSox->GetFieldFromLablePtr( nTableRowID, "_Description" )->GetInteger();
	if( nDescID <= 0 )
		return;

	int nLevel = pSox->GetFieldFromLablePtr( nTableRowID, "_Level" )->GetInteger();
	int nAmount = pSox->GetFieldFromLablePtr( nTableRowID, "_Amount" )->GetInteger();
	int nNeedItemID = pSox->GetFieldFromLablePtr( nTableRowID, "_item" )->GetInteger();

	m_pTextBox->ClearText();

	wchar_t wszStr[256] = {0,};
	if( bCashSlot )
	{
		swprintf_s( wszStr, _countof(wszStr), L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nDescID ) );
		m_pTextBox->AddText( wszStr, textcolor::ORANGE );
		return;
	}

	swprintf_s( wszStr, _countof(wszStr), L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nDescID ) );
	m_pTextBox->AddText( wszStr, textcolor::ORANGE );

	// Required Level (UIString 8347)
	swprintf_s( wszStr, _countof(wszStr), L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8347 ) );
	m_pTextBox->AddText( wszStr, textcolor::TOMATO );
	swprintf_s( wszStr, _countof(wszStr), L" %d", nLevel );
	m_pTextBox->AppendText( wszStr, textcolor::WHITE );

	// Required Gold (UIString 8348, format 8370)
	swprintf_s( wszStr, _countof(wszStr), L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8348 ) );
	m_pTextBox->AddText( wszStr, textcolor::TOMATO );
	int nGold = nAmount / 10000;
	int nSilver = ( nAmount % 10000 ) / 100;
	int nCopper = nAmount % 100;
	swprintf_s( wszStr, _countof(wszStr), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8370 ), nGold, nSilver, nCopper );
	m_pTextBox->AppendText( wszStr, textcolor::WHITE );

	// Required Item (UIString 8349; None = UIString 93)
	swprintf_s( wszStr, _countof(wszStr), L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8349 ) );
	m_pTextBox->AddText( wszStr, textcolor::TOMATO );
	if( nNeedItemID < 1 )
	{
		swprintf_s( wszStr, _countof(wszStr), L" %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93 ) );
	}
	else
	{
		swprintf_s( wszStr, _countof(wszStr), L" %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nNeedItemID ) );
	}
	m_pTextBox->AppendText( wszStr, textcolor::WHITE );
}

void CDnTooltipTalismanDlg::AdjustPosition()
{
	SUICoord DlgCoord;
	GetDlgCoord( DlgCoord );
	static float fXRGap = 4.0f / DEFAULT_UI_SCREEN_WIDTH;
	if( ( DlgCoord.Right() + fXRGap ) > GetScreenWidthRatio() )
	{
		DlgCoord.fX -= ( DlgCoord.Right() + fXRGap - GetScreenWidthRatio() );
	}
	SetDlgCoord( DlgCoord );
}

#endif // PRE_ADD_TALISMAN_SYSTEM
