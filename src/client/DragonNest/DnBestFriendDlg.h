#pragma once


// Best friend info Dlg - Dlg placed in community tab. //

#ifdef PRE_ADD_BESTFRIEND

#include "EtUIDialog.h"


class CDnBestFriendDlg : public CEtUIDialog, public CEtUICallback
{

private:

	CEtUIStatic * m_pStaticTex0;
	CEtUIStatic * m_pStaticTex1;
	CEtUIStatic * m_pStaticTex2;
	CEtUIStatic * m_pStaticTex3;
	CEtUIStatic * m_pStaticTex4;
	CEtUIStatic * m_pStaticTex5;
	CEtUIStatic * m_pStaticTex6;
	CEtUIStatic * m_pStaticCond;
	CEtUIStatic * m_pStaticTex7;
	CEtUIStatic * m_pStaticTex8;	


	CEtUIStatic * m_pStaticDate; // Date became best friends.
	CEtUIStatic * m_pStaticDay;  // Days elapsed since becoming best friends.
	CEtUIStatic * m_pStaticName; 
	CEtUIStatic * m_pStaticLevel;
	CEtUIStatic * m_pStaticJob; 
	CEtUIStatic * m_pStaticLocation;
	CEtUIStatic * m_pStaticMemo;
	CEtUIStatic * m_pStaticFriendMemo;

	// Statics displayed when breaking best friend.
	CEtUIStatic * m_pStatiBrokeupTime;
	CEtUIStatic * m_pStaticBrokeupNotice;

	CEtUIButton * m_pBtnBrokeup;       // Break best friend.
	CEtUIButton * m_pBtnBrokeupCancel; // Cancel breaking best friend.
	CEtUIButton * m_pBtnBrokeupAgreement; // Agree to break best friend.

	CEtUIButton * m_pBtnParty; // Party request.
	CEtUIButton * m_pBtnMemo; // Input memo. 
	
	// Best friend DBID.
	INT64 m_biCharacterDBID;

	std::vector< CEtUIStatic * > m_vStaticEnable; // Statics to enable when breaking best friend. 


	float m_timeOneMin; // 1 minute 
	float m_CountDown; // Remaining time to break best friend.
	wchar_t * m_pStrCountDown;


	bool m_bBFState; // Best friend (true), break up (false)
	bool m_bBFBrokeUpCancel; // Cancel dissolution.


	bool m_bAllowRequestBFInfo; // Whether to allow best friend info request.
	float m_TimeAllowBFInfo;

	
public:

	CDnBestFriendDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnBestFriendDlg(){
		ReleaseDlg();
	}
	
	void ReleaseDlg();


	// Override - CEtUIDialog //		
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();	
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual void Show( bool bShow );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

private:

	// Handle control display based on best friend status.
	void SetCtrlBFInfo( bool bShow );
	void SetCtrlDestroyedBFInfo( bool bShow );

	
	void EnableCtrByGameTask();

	void RequestBFInfo(); // Request best friend info.

public:

	void SetBFData( struct TBestFriendInfo & bfInfo ); // Best friend info.

	void SetMemo( bool bFromMe, WCHAR * pStrMemo ); // Receive memo.

	void InvitePartyBF(); // Party invite.

	void BrokeupOrCancelBF( bool bCancel, WCHAR * strName ); // Break best friend or cancel break.
};


#endif