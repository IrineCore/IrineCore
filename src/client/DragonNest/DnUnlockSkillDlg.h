#pragma once
#include "DnCustomDlg.h"
#include "DnSmartMoveCursor.h"

class CDnSkillUnlockDlg : public CDnCustomDlg
{
public:
	CDnSkillUnlockDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallBack = NULL );
	virtual ~CDnSkillUnlockDlg(void);

protected:
	
	CDnSkillSlotButton* m_pSkillSlotButton;
	CEtUIStatic* m_pSkillName;
	CEtUIStatic* m_pNeedSP;

	CEtUIStatic* m_pGold;
	CEtUIStatic* m_pSilver;
	CEtUIStatic* m_pBronze;

	CEtUIButton* m_pButtonOK;
	CEtUIButton* m_pButtonCancel;

	CEtUIStatic* m_pStaticExclusive;

	// Smart move
	CDnSmartMoveCursor m_SmartMove;

public:
	void SetSkill( MIInventoryItem *pSkill );

public:
	virtual void InitialUpdate();
	virtual void Initialize(bool bShow);
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void Show( bool bShow );

	void SetPrice( int iPrice );
};
