#pragma once

#ifdef PRE_ADD_PVP_RANKING

#include "DnPvPRankListItemDlg.h"


// Same as CDnPvPRankListItemDlg, but uses a different .ui file.
// As an ItemDlg added to CEtUITreeCtl, TreeControl handles its creation and initialization, so 
// Added an extra class because the .ui file cannot be changed depending on the state.
class CDnPvPRankListItemLadderDlg : public CDnPvPRankListItemDlg
{
public:

	CDnPvPRankListItemLadderDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnPvPRankListItemLadderDlg(){}

	virtual void Initialize( bool bShow );
};


#endif // PRE_ADD_PVP_RANKING