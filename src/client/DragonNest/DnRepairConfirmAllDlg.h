#pragma once
#include "EtUIDialog.h"
#include "DnSmartMoveCursor.h"

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
class CDnRepStoreBenefitTooltipDlg;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

// Feels unnecessary to have made a separate Equip one. I should talk to the designer next time.
class CDnRepairConfirmAllDlg : public CEtUIDialog
{
public:
	CDnRepairConfirmAllDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnRepairConfirmAllDlg(void);

protected:
	CEtUIStatic *m_pGold;
	CEtUIStatic *m_pSilver;
	CEtUIStatic *m_pBronze;

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CEtUIStatic *m_pReputationBenefit;
	CDnRepStoreBenefitTooltipDlg* m_pStoreBenefitTooltip;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

	CEtUIButton *m_pButtonOK;
	CEtUIButton *m_pButtonCancel;

	// Smart move
	CDnSmartMoveCursor m_SmartMove;

protected:

public:
	void SetPrice( int nPrice );
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	void ShowReputationBenefit( bool bShow );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};