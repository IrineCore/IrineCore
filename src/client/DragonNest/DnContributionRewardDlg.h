#pragma once

#ifdef PRE_ADD_DONATION


#include "EtUIDialog.h"


//------------------------------------------------
// Donation system - Donation reward confirmation window.

class CDnContributionRewardDlg : public CEtUIDialog
{

private:

	bool m_bFirstOpen; // First Dlg open. First time: true, subsequent: false

	enum ETypeReward
	{
		NONE = 0,
		MONTH,
		DAY
	};

	CEtUIRadioButton * m_pRadioMonth;
	CEtUIRadioButton * m_pRadioDay;	
	CEtUIListBoxEx * m_pListRewardMonth; // Month.
	CEtUIListBoxEx * m_pListRewardDay;   // Day.

	ETypeReward m_crrType;

public:

	CDnContributionRewardDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnContributionRewardDlg(){}


	// Override - CEtUIDIalog.	
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );


private:

	void SetRewardList();
	void RefreshList( ETypeReward type );

};

#endif