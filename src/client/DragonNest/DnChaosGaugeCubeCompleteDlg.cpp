#include "StdAfx.h"

#ifdef PRE_ADD_CHAOSCUBE

#include "DnChaosGaugeCubeCompleteDlg.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChaosGaugeCubeCompleteDlg::CDnChaosGaugeCubeCompleteDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnChaosCubeResultDlg( dialogType, pParentDialog, nID, pCallback )
{
}

CDnChaosGaugeCubeCompleteDlg::~CDnChaosGaugeCubeCompleteDlg()
{
}

void CDnChaosGaugeCubeCompleteDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "InvenBoxResultDlg.ui" ).c_str(), bShow );
}

void CDnChaosGaugeCubeCompleteDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) || IsCmdControl( "ID_BUTTON_CLOSE" ) || IsCmdControl( "ID_BT_OK" ) )
		{
			Show( false );
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

#endif // PRE_ADD_CHAOSCUBE
