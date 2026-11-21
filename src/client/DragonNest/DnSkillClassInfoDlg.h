#pragma once

#include "DnCustomDlg.h"

class CDnSkillClassInfoDlg : public CDnCustomDlg
{
public:
	CDnSkillClassInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSkillClassInfoDlg();

protected:
	CEtUIButton *m_pBtnInfo;
	CEtUITextBox *m_pTextBox;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetClassInfoText( const WCHAR *wszText );
};
