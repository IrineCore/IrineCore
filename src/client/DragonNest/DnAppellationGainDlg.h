#pragma once
#include "EtUIDialog.h"
#include "DnItem.h"

#include "DNDefine_Work.h"

#ifdef PRE_MOD_LEVELUP_REWARD_SMARTMOVE_APP
#include "DnSmartMoveCursor.h"
#endif


class CDnItem;
class CDnQuickSlotButton;

class CDnAppellationGainDlg : public CEtUIDialog, public CEtUICallback
{
public:
	CDnAppellationGainDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnAppellationGainDlg();

protected:
	CEtUIStatic *m_pStaticText;
	CDnItem *m_pItem;


#ifdef PRE_MOD_LEVELUP_REWARD_SMARTMOVE_APP
	CDnSmartMoveCursor		m_SmartMove;				// # 61437 - Apply smart move when using GM storage, bag, or resurrection scroll - elkain03
#endif

public:
	void SetAppellationGainItem( CDnItem *pItem );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};
