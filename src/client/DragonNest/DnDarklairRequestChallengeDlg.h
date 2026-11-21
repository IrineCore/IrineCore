#pragma once
#include "DnMessageBox.h"

class CDnDarklairRequestChallengeDlg : public CDnMessageBox
{
public:
	CDnDarklairRequestChallengeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDarklairRequestChallengeDlg();

public:
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	
public:
	void EnableButton( bool bEnableButton );
};
