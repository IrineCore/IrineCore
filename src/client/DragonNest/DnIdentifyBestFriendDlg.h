#pragma once


// Best friend registration user confirm Dlg. //

#ifdef PRE_ADD_BESTFRIEND

#include "EtUIDialog.h"


class CDnIdentifyBestFriendDlg : public CEtUIDialog
{

private:


	CEtUIStatic * m_pStaticBFID;
	CEtUIStatic * m_pStaticLevel;
	CEtUIStatic * m_pStaticJob;


public:

	CDnIdentifyBestFriendDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnIdentifyBestFriendDlg(){
		ReleaseDlg();
	}

	void ReleaseDlg();


	// Override - CEtUIDialog //		
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();	
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );


public:

	void SetSearchBF( struct BestFriend::SCSearch * pData ); // Best friend confirm info.

	void RequestBF(); // Best friend registration request.

};



#endif