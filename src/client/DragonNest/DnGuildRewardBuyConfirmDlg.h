#pragma once

#include "DnCustomDlg.h"
#include "DnTableDB.h"

class CDnItem;
class CDnItemSlotButton;
class CDnGuildRewardBuyConfirmDlg : public CDnCustomDlg
{
public:
	CDnGuildRewardBuyConfirmDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGuildRewardBuyConfirmDlg(void);

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );
	virtual void Process( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
	GuildReward::GuildRewardInfo m_GuildRewardInfo;	//Guild reward info
	
	CEtUIStatic *m_pWarringMsg;		//Warning message?
	
	CDnItemSlotButton* m_pItemSlot;	//Guild reward item slot.
	CDnItem* m_pTempItem;			//Guild reward item

	CEtUIStatic *m_pRewardInfo;		//Guild reward name?
	CEtUIStatic *m_pPeriodInfo;		//Duration info
	CEtUIStatic *m_pGold;			//Purchase price gold
	CEtUIStatic *m_pSilver;			//Purchase price silver
	CEtUIStatic *m_pBronze;			//Purchase price copper
	
	void SetRewardItemInfo( const GuildReward::GuildRewardInfo &info );

public:
	void SetGuildRewardInfo(const GuildReward::GuildRewardInfo &info);
	void SetNeedGoldInfo();

};