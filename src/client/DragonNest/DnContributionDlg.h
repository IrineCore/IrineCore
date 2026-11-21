#pragma once

#ifdef PRE_ADD_DONATION


#include "DnCustomDlg.h"


//------------------------------------------------
// Donation system main window.

class CDnContributionDlg : public CDnCustomDlg
{

private:

	// Donation-related Dialog Group.
	enum EDlgGroup
	{
		EDlgGroup_GOLD = 0,  // Gold donation.
		EDlgGroup_REWARD = 1,// View reward.
		EDlgGroup_MAX
	};

	// Diglaog Group.
	class CEtUIDialogGroup * m_pDlgGroup;	

	// LocalPlayer controls.
	CEtUIStatic * m_pStaticNameJobLv;
	CEtUIStatic * m_pStaticRank;
	CEtUIStatic * m_pStaticGold;
	/*CEtUIStatic * m_pStaticSivler;
	CEtUIStatic * m_pStaticCopper;*/

	//// Controls for top rankers 1st, 2nd, 3rd. Commented out due to UI change.
	//struct SHighRankerControls
	//{
	//	CDnJobIconStatic * pJobIcon;
	//	CEtUIStatic * pStaticGuild;
	//	CEtUIStatic * pStaticID;
	//	CEtUIStatic * pStaticGold;
	//	/*CEtUIStatic * pStaticSilver;
	//	CEtUIStatic * pStaticCopper;*/
	//	CDnItemSlotButton * pItemSlot;
	//};
	//vector< SHighRankerControls > m_vecHighRanker;

	// Rank 7~10 ListBox.
	CEtUIListBoxEx * m_pListRank;

	
	// Control - Donate.
	CEtUIButton * m_pBtnContribute;

	bool m_bFirstOpen; // First Dlg open. First time: true, subsequent: false


public:

	CDnContributionDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnContributionDlg(){
		ReleaseDlg();
	}

	void ReleaseDlg();


	// Override - CEtUIDialog //	
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );



	// Refresh donation ranking list.
	void RefreshList();	

	// Refresh Donate Dialog - current gold.
	void RefreshContributionGoldDlg();

private:

	//// Update top rankers.
	//void SetDataHighRanker( const struct SContributionRanker * aRanker, SHighRankerControls & aControls, int nItemID );
	
	
	// Send ranking list refresh packet.	
	void OnSendRefreshRankerList();
};


#endif