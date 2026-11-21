#include "StdAfx.h"
#include "DnPremiumPcBangNotifyToolTipDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPremiumPcBangNotifyToolTipDlg::CDnPremiumPcBangNotifyToolTipDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pTextBox( NULL )
{
}

CDnPremiumPcBangNotifyToolTipDlg::~CDnPremiumPcBangNotifyToolTipDlg()
{
}

void CDnPremiumPcBangNotifyToolTipDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PcToolTipDlg.ui" ).c_str(), bShow );
}

void CDnPremiumPcBangNotifyToolTipDlg::InitialUpdate()
{
	m_pTextBox = GetControl<CEtUITextBox>( "ID_TEXTBOX" );
}

void CDnPremiumPcBangNotifyToolTipDlg::SetNotifyText( const WCHAR *wszText )
{
	if( m_pTextBox )
	{
		m_pTextBox->ClearText();
		if( wszText )
			m_pTextBox->AddText( wszText );
	}
}
