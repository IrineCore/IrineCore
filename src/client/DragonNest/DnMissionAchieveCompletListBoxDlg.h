#pragma once
#include "EtUIDialog.h"

#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL

class CDnMissionAchieveCompletListBoxDlg : public CEtUIDialog
{
public:
	CDnMissionAchieveCompletListBoxDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMissionAchieveCompletListBoxDlg(void);

protected:
	CEtUIListBoxEx* m_pListBoxEx;

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void RefreshInfo();
};

#endif
