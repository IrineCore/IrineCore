#pragma once

#ifdef PRE_ADD_CHAOSCUBE

#include "DnChaosCubeResultDlg.h"

class CDnChaosGaugeCubeCompleteDlg : public CDnChaosCubeResultDlg
{
public:
	CDnChaosGaugeCubeCompleteDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChaosGaugeCubeCompleteDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};

#endif // PRE_ADD_CHAOSCUBE
