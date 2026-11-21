#pragma once


#ifdef PRE_ADD_AUTOUNPACK

//------------------------------------------------------
//					CDnAutoUnPack
//
// : Automatically open n registered items (treasure chests or random items, currently charm items only).
//
#include "DnCustomDlg.h"

#ifdef PRE_ADD_INSTANT_CASH_BUY
class CDnAutoUnPackDlg : public CDnCustomDlg, public CEtUICallback
#else // PRE_ADD_INSTANT_CASH_BUY
class CDnAutoUnPackDlg : public CDnCustomDlg
#endif // PRE_ADD_INSTANT_CASH_BUY
{

private:

	CDnItemSlotButton * m_pCtrItemSlot;
	CEtUIStatic * m_pStaticKeyCount;

	class CDnInvenTabDlg * m_pInvenTabDlg;

	INT64 m_ItemSerial; // Auto-open item serial.

	CDnItem * m_pResigtedItem; // Item to auto-open.
	int m_ItemClassID;
	int m_invenType;
	bool m_bUnPackIng; // Whether auto-unwrap is in progress.

	float m_fUnPackTimePerOwn; // Open delay time per item.

protected:


public:
	CDnAutoUnPackDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnAutoUnPackDlg(){
		ReleaseDlg();
	}
	
	void ReleaseDlg();

	// Override - CEtUIDialog //
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();		
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
#ifdef PRE_ADD_INSTANT_CASH_BUY
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
#endif // PRE_ADD_INSTANT_CASH_BUY

public:

	// Stop auto-open.
	void EngUnpack();

	// Auto-open item sn.
	void SetItemSerial( INT64 sn ){
		 m_ItemSerial = sn;
	}

	// Register item to auto-open.
	bool RegisterItem( class CDnItem * pItem );

	// Start auto-open.
	void UnPacking();

	// Restore previous state - re-register the auto-open item if "Stop Opening" was clicked in the auto-open result window.
	void PreStateRestore();

	// Item open start result.
	void OnRecvRequestCharmItem( char cInvenType, short sInvenIndex, INT64 biInvenSerial );
	
	// Prepare UnPacking.
	void ReadyUnPack();
	
	// Set KeyItem count.
	void SetKeyItemCount( CDnItem * pItem );

	// Set open delay time % per item.
	void SetUnPackTime( int nTime );

	// Open delay time per item.
	float GetUnPackTime(){
		return m_fUnPackTimePerOwn; 
	}

};


#endif