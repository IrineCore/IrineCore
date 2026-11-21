#pragma once
#include "EtUIDialog.h"


class CDnCharStatusPVPInfoItemDlg : public CEtUIDialog
{
public:
	CDnCharStatusPVPInfoItemDlg ( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharStatusPVPInfoItemDlg (void);

protected:
	CEtUIStatic *m_pStaticItem;
	CEtUIStatic *m_pStaticState;
public:
	void SetInfo( const wchar_t * wszItemText , INT64 iData );
	void SetInfo( const wchar_t * wszItemText , const wchar_t *wszDataText );	// Sometimes receives text directly.

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	//virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};
