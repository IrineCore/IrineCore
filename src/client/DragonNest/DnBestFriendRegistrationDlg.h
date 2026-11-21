#pragma once


// Best friend certificate Dlg - use best friend certificate item. //

#ifdef PRE_ADD_BESTFRIEND

#include "EtUIDialog.h"


class CDnBFRegistrationDlg : public CEtUIDialog
{

private:
	
	CEtUIIMEEditBox * m_IMEBfFriend;


	INT64 m_nSerial;

public:

	CDnBFRegistrationDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnBFRegistrationDlg(){
		ReleaseDlg();
	}

	void ReleaseDlg();


	// Override - CEtUIDialog //		
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();	
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );

private:
	
	void RequestBF(); // Best friend registration request.

public:

	void SetSerial( INT64 serial ); // Best friend certificate serial.
	INT64 GetSerial(){
		return m_nSerial;
	}
	


};



#endif