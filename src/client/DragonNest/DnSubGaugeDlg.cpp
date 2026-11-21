#include "StdAfx.h"
#include "DnSubGaugeDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSubGaugeDlg::CDnSubGaugeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pProgressGauge( NULL )
	, m_pStaticText( NULL )
{
}

CDnSubGaugeDlg::~CDnSubGaugeDlg()
{
}

void CDnSubGaugeDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SubGaugeDlg.ui" ).c_str(), bShow );
}

void CDnSubGaugeDlg::InitialUpdate()
{
	m_pProgressGauge = GetControl<CEtUIProgressBar>( "ID_PROGRESS1" );
	m_pStaticText = GetControl<CEtUIStatic>( "ID_TEXT1" );
}

void CDnSubGaugeDlg::SetGauge( float fPercent, const WCHAR *wszLabel )
{
	if( m_pProgressGauge )
		m_pProgressGauge->SetProgress( fPercent );
	if( m_pStaticText && wszLabel )
		m_pStaticText->SetText( wszLabel );
}
