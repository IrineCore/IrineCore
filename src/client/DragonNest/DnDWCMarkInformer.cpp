#include "StdAfx.h"

#if defined( PRE_ADD_DWC )

#include "DnDWCMarkInformer.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnDWCMarkTooltipDlg
//-----------------------------------------------------------------------------
CDnDWCMarkTooltipDlg::CDnDWCMarkTooltipDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnSimpleTooltipDlg( dialogType, pParentDialog, nID, pCallback )
{
}

CDnDWCMarkTooltipDlg::~CDnDWCMarkTooltipDlg()
{
}

void CDnDWCMarkTooltipDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SimpleTooltipDlg.ui" ).c_str(), bShow );
}

void CDnDWCMarkTooltipDlg::SetDWCMarkInfo( const WCHAR *wszTeamName, int nRank, int nPoint )
{
	if( !m_pTextBox )
		return;

	m_pTextBox->ClearText();
	if( wszTeamName && wcslen( wszTeamName ) > 0 )
		m_pTextBox->AddText( wszTeamName, textcolor::YELLOW );

	WCHAR wszBuf[128] = { 0 };
	swprintf_s( wszBuf, _countof( wszBuf ), L"Rank: %d  Point: %d", nRank, nPoint );
	m_pTextBox->AddText( wszBuf, textcolor::WHITE );
}

//-----------------------------------------------------------------------------
// CDnDWCMarkInformer
//-----------------------------------------------------------------------------
CDnDWCMarkInformer::CDnDWCMarkInformer()
	: m_bActive( false )
	, m_nRank( 0 )
	, m_cGrade( 0 )
{
}

CDnDWCMarkInformer::~CDnDWCMarkInformer()
{
}

void CDnDWCMarkInformer::UpdateMarkInfo( const WCHAR *wszTeamName, int nRank, BYTE cGrade )
{
	m_bActive = true;
	m_nRank = nRank;
	m_cGrade = cGrade;
	m_strTeamName = wszTeamName ? wszTeamName : L"";
}

void CDnDWCMarkInformer::ResetMarkInfo()
{
	m_bActive = false;
	m_nRank = 0;
	m_cGrade = 0;
	m_strTeamName.clear();
}

#endif // PRE_ADD_DWC
