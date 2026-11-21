#include "StdAfx.h"
#include "DnMetaMissionTooltipDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMetaMissionTooltipDlg::CDnMetaMissionTooltipDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticName( NULL )
	, m_pStaticBase( NULL )
	, m_pStaticEquip( NULL )
	, m_pStaticLine( NULL )
	, m_pStaticLineV2( NULL )
	, m_pStaticLineV3( NULL )
	, m_pStaticPointBox( NULL )
	, m_pTextBox( NULL )
{
}

CDnMetaMissionTooltipDlg::~CDnMetaMissionTooltipDlg()
{
}

void CDnMetaMissionTooltipDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "TooltipDlg.ui" ).c_str(), bShow );
}

void CDnMetaMissionTooltipDlg::InitialUpdate()
{
	m_pStaticName = GetControl<CEtUIStatic>( "ID_STATIC_NAME" );
	m_pStaticBase = GetControl<CEtUIStatic>( "ID_STATIC_BASE" );
	m_pStaticEquip = GetControl<CEtUIStatic>( "ID_STATIC_EQUIP" );
	m_pStaticLine = GetControl<CEtUIStatic>( "ID_LINE" );
	m_pStaticLineV2 = GetControl<CEtUIStatic>( "ID_LINE_VERTICAL2" );
	m_pStaticLineV3 = GetControl<CEtUIStatic>( "ID_LINE_VERTICAL3" );
	m_pStaticPointBox = GetControl<CEtUIStatic>( "ID_STATIC_POINTBOX" );
	m_pTextBox = GetControl<CEtUITextBox>( "ID_TEXTBOX" );
}

void CDnMetaMissionTooltipDlg::SetMetaMissionInfo( const WCHAR *wszTitle, const WCHAR *wszDesc )
{
	if( m_pStaticName ) m_pStaticName->SetText( wszTitle ? wszTitle : L"" );
	if( m_pTextBox )
	{
		m_pTextBox->ClearText();
		if( wszDesc ) m_pTextBox->AddText( wszDesc );
	}
}
