#pragma once
#include "DnCustomDlg.h"


class CDnDungeonSynchroDlg : public CDnCustomDlg
{
public:
	CDnDungeonSynchroDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDungeonSynchroDlg();

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );

	void SetSynchro( bool bSynchro );

protected:
	CEtUIStatic* m_pStaticSyncroSuccess;
	CEtUIStatic* m_pStaticSyncroSuccessBg;
	CEtUIStatic* m_pStaticSyncroFail;
	CEtUIStatic* m_pStaticSyncroFailBg;
};
