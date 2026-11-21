#pragma once

#include "DnCustomDlg.h"

class CDnDungeonEnterNotifierDlg : public CDnCustomDlg
{
public:
	CDnDungeonEnterNotifierDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDungeonEnterNotifierDlg();

protected:
	CEtUIStatic *m_pStaticTitle;
	CEtUIStatic *m_pStaticDesc;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetDungeonNotice( const WCHAR *wszTitle, const WCHAR *wszDesc );
};
