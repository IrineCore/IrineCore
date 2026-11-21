#pragma once
#include "EtUIDialog.h"



class CDnCharPetInfoDlg : public CEtUIDialog
{
public:
	CDnCharPetInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharPetInfoDlg();

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );

public:
	void SetPetLevelTableID( int nPetLevelTableID ) { m_nPetLevelTableID = nPetLevelTableID; }
	void SetPetAddAbility();

protected:
	CEtUITextBox*	m_pTextBox;
	int				m_nPetLevelTableID;
};
