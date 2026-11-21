#pragma once


// Best friend reward Dlg. //

#ifdef PRE_ADD_BESTFRIEND

#include "DnCustomDlg.h"


class CDnBestFriendRewardDlg : public CDnCustomDlg
{

private:

	bool m_bFirst;

	struct SItem
	{
		CEtUIStatic * pStatic;
		CEtUIStatic * pStaticSelect;
		CDnItemSlotButton * pItemSlot;
	};	
	std::vector< SItem * > m_vecItems;
	int m_selectionIndex;

	INT64 m_giftSerial;
	int m_itemID;
	int m_typeParam1;

public:

	CDnBestFriendRewardDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnBestFriendRewardDlg(){
		ReleaseDlg();
	}

	void ReleaseDlg();


	// Override - CEtUIDialog //		
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();	
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );


public:

	int GetItemID(){
		return m_itemID;
	}

	void SetGiftInfo( INT64 giftSerial, int typeParam1 ){
		m_giftSerial = giftSerial;  m_typeParam1 = typeParam1;
	}

	// Reward item grant complete.
	void SendRewardComplet( int nItemID );

};



#endif