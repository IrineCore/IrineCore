#include "StdAfx.h"
#include "DnGachaRouletteDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef PRE_ADD_GACHA_JAPAN

CDnGachaRouletteDlg::CDnGachaRouletteDlg( UI_DIALOG_TYPE dialogType /* = UI_TYPE_FOCUS */, 
										  CEtUIDialog *pParentDialog /* = NULL */, 
										  int nID /* = -1 */, CEtUICallback *pCallback /* = NULL */ ) : CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	SecureZeroMemory( m_apTextBox, sizeof(m_apTextBox) );
}

CDnGachaRouletteDlg::~CDnGachaRouletteDlg(void)
{

}


void CDnGachaRouletteDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "Jp_Roulette.ui" ).c_str(), bShow );
}


void CDnGachaRouletteDlg::InitialUpdate( void )
{
	char acBuffer[ 128 ] = { 0 };
	for( int i = 0; i < NUM_GACHA_ROULETTE_ITEM; ++i )
	{	
		sprintf_s( acBuffer, "ID_TEXTBOX%d", i );
		m_apTextBox[ i ] = GetControl<CEtUITextBox>( acBuffer );

		// Add test string.
		//wchar_t awcBuffer[ 128 ] = { 0 };
		//swprintf_s( awcBuffer, L"%d Test %d", i, i );
		//m_apStatic[ i ]->SetText( awcBuffer );
	}
}

void CDnGachaRouletteDlg::SetItemText( int iIndex, const wchar_t* pText )
{
	_ASSERT( 0 <= iIndex && iIndex < NUM_GACHA_ROULETTE_ITEM && pText );
	if( 0 <= iIndex && iIndex < NUM_GACHA_ROULETTE_ITEM && pText )
	{
		m_apTextBox[ iIndex ]->ClearText();
		m_apTextBox[ iIndex ]->AddText( pText, D3DXCOLOR(1.0f, 0.8f, 0.0f, 1.0f) );
	}
}


void CDnGachaRouletteDlg::SetItemText( int iIndex, const wchar_t* pItemName, const wchar_t* pRankName, DWORD dwRankColor )
{ 
	_ASSERT( 0 <= iIndex && iIndex < NUM_GACHA_ROULETTE_ITEM && pItemName && pRankName );
	if( 0 <= iIndex && iIndex < NUM_GACHA_ROULETTE_ITEM && pItemName && pRankName )
	{
		m_apTextBox[ iIndex ]->ClearText();
		m_apTextBox[ iIndex ]->AddText( pItemName, D3DXCOLOR(1.0f, 0.8f, 0.0f, 1.0f) );
		//m_apTextBox[ iIndex ]->AppendText( _T("  ") );
		m_apTextBox[ iIndex ]->AppendText( _T("["), dwRankColor );
		m_apTextBox[ iIndex ]->AppendText( pRankName, dwRankColor );
		m_apTextBox[ iIndex ]->AppendText( _T("]"), dwRankColor );
	}
}

void CDnGachaRouletteDlg::SetStatText( int iIndex, const wchar_t* pStat, DWORD dwRankColor )
{
	_ASSERT( 0 <= iIndex && iIndex < NUM_GACHA_ROULETTE_ITEM && pStat );
	if( 0 <= iIndex && iIndex < NUM_GACHA_ROULETTE_ITEM && pStat )
	{
		m_apTextBox[ iIndex ]->ClearText();
		m_apTextBox[ iIndex ]->AddText( pStat, dwRankColor );
	}
}

void CDnGachaRouletteDlg::Clear( void )
{
	for( int i = 0; i < NUM_GACHA_ROULETTE_ITEM; ++i )
	{
		m_apTextBox[ i ]->ClearText();
	}
}

#endif //PRE_ADD_GACHA_JAPAN