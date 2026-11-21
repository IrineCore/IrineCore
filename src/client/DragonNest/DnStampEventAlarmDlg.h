#pragma once

#ifdef PRE_ADD_STAMPSYSTEM

#include "EtUIDialog.h"


class CDnStampEventAlarmDlg : public CEtUIDialog
{

private:

	CEtUIStatic * m_pStaticText;


public:
	CDnStampEventAlarmDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnStampEventAlarmDlg(){}

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );


private:

public:

	// Set alarm state.
	void SetAlarmState( bool bOne=false, bool bForce=false );


};


#endif // PRE_ADD_STAMPSYSTEM