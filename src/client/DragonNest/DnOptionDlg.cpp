#include "StdAfx.h"
#include "DnOptionDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnOptionDlg::CDnOptionDlg( UI_DIALOG_TYPE dialogType , CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  ) :
CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{

}

CDnOptionDlg::~CDnOptionDlg()
{

}

void CDnOptionDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_APPLY" ) )
		{
			ExportSetting();
		}
		if( IsCmdControl("ID_BUTTON_CANCEL" ) )
		{
			CancelOption();
			ImportSetting();
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnOptionDlg::Show( bool bShow )
{
	if( m_bShow == bShow ) return;

	if( bShow )
	{
	}
	else
	{
		CancelOption();
		ImportSetting();
	}
	CEtUIDialog::Show( bShow );
}
