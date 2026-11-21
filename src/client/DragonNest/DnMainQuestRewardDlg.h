#pragma once

#include "DnCustomDlg.h"

class CDnItem;

class CDnMainQuestRewardDlg : public CDnCustomDlg
{
public:
	enum {
		REWARD_SLOT_MAX = 6,
		BONUS_SLOT_MAX = 4,
	};

public:
	CDnMainQuestRewardDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMainQuestRewardDlg();

protected:
	CEtUIStatic *m_pStaticGold;
	CEtUIStatic *m_pStaticSilver;
	CEtUIStatic *m_pStaticBronze;
	CEtUIStatic *m_pStaticExp;
	CEtUIStatic *m_pStaticComment;
	CEtUIStatic *m_pStaticFTG;
	CEtUIStatic *m_pStaticLeftBonus;
	CEtUIStatic *m_pStaticRightBonus;

	CDnItemSlotButton *m_pSlotItems[REWARD_SLOT_MAX];
	CEtUIStatic *m_pSlotFrames[REWARD_SLOT_MAX];
	CDnItem *m_pRewardItems[REWARD_SLOT_MAX];

	CDnItemSlotButton *m_pLeftBonusSlots[BONUS_SLOT_MAX];
	CEtUIStatic *m_pLeftBonusSelects[BONUS_SLOT_MAX];
	CDnItemSlotButton *m_pRightBonusSlots[BONUS_SLOT_MAX];
	CEtUIStatic *m_pRightBonusSelects[BONUS_SLOT_MAX];

	void ClearRewardItems();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );

	void SetRewardInfo( INT64 nCoin, int nExp, int nFTG, const std::vector<std::pair<int, int> > &vecItems );
};
