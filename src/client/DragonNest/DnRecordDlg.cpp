#include "StdAfx.h"
#include "DnRecordDlg.h"
#include "EtBCLEngine.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 


CDnRecordDlg::CDnRecordDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, false )
, m_pRecordTime(NULL)
, m_pRecordSize(NULL) 
{
	memset( m_pRecordIcon, 0, sizeof(m_pRecordIcon) );
	m_dwPrevTime = 0;

	m_ResWidth = 0;
	m_ResHeight = 0;
}

CDnRecordDlg::~CDnRecordDlg()
{
}

void CDnRecordDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "RecordDlg.ui" ).c_str(), bShow );
}

void CDnRecordDlg::InitialUpdate()
{
	m_pRecordTime = GetControl<CEtUIStatic>("ID_TEXT_TIME");
	m_pRecordSize = GetControl<CEtUIStatic>("ID_TEXT_SIZE");

	m_pRecordIcon[0] = GetControl<CEtUIStatic>("ID_REC_ICON0");
	m_pRecordIcon[1] = GetControl<CEtUIStatic>("ID_REC_ICON1");
	m_pRecordIcon[2] = GetControl<CEtUIStatic>("ID_REC_ICON2");
}

void CDnRecordDlg::Show( bool bShow )
{
	if( bShow == m_bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnRecordDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !CEtBCLEngine::IsActive() ) {
		Show( false );
		return;
	}
	if( !CEtBCLEngine::GetInstance().IsCapturing() ) {
		Show( false );
		return;
	}

	int curResWidth = GetEtDevice()->Width();
	int curResHeight = GetEtDevice()->Height();
	if (curResWidth != m_ResWidth || curResHeight != m_ResHeight)
	{
		OnChangeResolution();
		m_ResWidth = curResWidth;
		m_ResHeight = curResHeight;
	}

	Show( true );
	WCHAR wszStr[128];

	int nRecordTime = CEtBCLEngine::GetInstance().GetCaptureTime() / 1000;
	INT64 nRecordSize = CEtBCLEngine::GetInstance().GetCaptureFileSize();

	// Capacity
	double fRecordSize;
	bool bGigaUnit = false;
	if( nRecordSize >= 1073741824 ) { // If over a gigabyte, display in GB.
		fRecordSize = nRecordSize / 1073741824.f;
		bGigaUnit = true;
	}
	else {
		fRecordSize = nRecordSize / 1048576.f;
	}

	swprintf_s( wszStr, L"%.1f %s", fRecordSize, ( bGigaUnit ) ? L"GB" : L"MB" );
	m_pRecordSize->SetText( wszStr );

	// Time
	int nHour = nRecordTime / 3600;
	int nMin = ( nRecordTime % 3600 ) / 60;
	int nSec = nRecordTime % 60;

	swprintf_s( wszStr, L"%02d:%02d:%02d", nHour, nMin, nSec );
	m_pRecordTime->SetText( wszStr );

	// Icon
	if( m_dwPrevTime == 0 ) m_dwPrevTime = timeGetTime();

	int nValue = ( timeGetTime() - m_dwPrevTime ) % 300;
	for( int i=0; i<3; i++ ) {
		if( nValue / 100 == i )
			m_pRecordIcon[i]->Show( true );
		else m_pRecordIcon[i]->Show( false );
	}
}