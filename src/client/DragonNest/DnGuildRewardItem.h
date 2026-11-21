#pragma once

#include "DnCustomDlg.h"
#include "DnTableDB.h"

class CDnItem;
class CDnGuildRewardItem : public CDnCustomDlg
{
public:
	CDnGuildRewardItem( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGuildRewardItem(void);

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );
	virtual void Process( float fElapsedTime );

protected:
	GuildReward::GuildRewardInfo m_GuildRewardInfo;	//Guild reward info
	bool m_IsAvailableReward;		//Whether activated
	bool m_IsPurchased;		//Purchased status
	bool m_isAppliedRewardBetterThan;	//

	CDnItemSlotButton* m_pItemSlot;	//Guild reward item slot.
	CDnItem* m_pTempItem;			//Guild reward item

	CEtUIStatic *m_pMasterLimit;	//Display guild master only icon
	CEtUIStatic *m_pLevelLimit;		//Guild level restriction info
	CEtUIStatic *m_pRewardInfo;		//Guild reward name?
	CEtUIStatic *m_pPeriodInfo;		//Duration info
	CEtUIStatic *m_pGold;			//Purchase price gold
	CEtUIStatic *m_pSilver;			//Purchase price silver
	CEtUIStatic *m_pBronze;			//Purchase price copper

	void UpdateInfo();
	void SetRewardItemInfo( const GuildReward::GuildRewardInfo &info );

public:
	void SetGuildRewardInfo(const GuildReward::GuildRewardInfo &info);

	void SetNeedGoldInfo(DWORD dwColor);

	bool IsActivated() { return m_IsAvailableReward; }
	bool IsPurchased() { return m_IsPurchased; }

	const GuildReward::GuildRewardInfo& GetGuildRewardInfo() { return m_GuildRewardInfo; }
};