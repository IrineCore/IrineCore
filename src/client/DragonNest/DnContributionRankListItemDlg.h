#pragma once


#ifdef PRE_ADD_DONATION


#include "DnCustomDlg.h"


//------------------------------------------------
// Donation system - Ranker list ItemDlg.

class CDnContributionRankListItemDlg : public CDnCustomDlg // public CEtUIDialog
{

public:
						        
	CDnContributionRankListItemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false  );
	virtual ~CDnContributionRankListItemDlg(){
		ReleaseDlg();
	}

	void ReleaseDlg();

	// Override - CEtUIDialog //	
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	
	//void SetData()

public:
	void SetData( const struct SContributionRanker * aRanker, int nItemID );

private:
	CEtUIStatic * m_pStaticRank;
	CEtUIStatic * m_pStaticGuild;
	CEtUIStatic * m_pStaticID;
	CEtUIStatic * m_pStaticGold;
	class CDnItemSlotButton * m_pItemSlotBtn;
	class CDnJobIconStatic * m_pStaticJob;	
};

#endif