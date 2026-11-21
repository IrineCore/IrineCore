#pragma once
#include "DnCustomDlg.h"
#include "DnPVPDetailedInfoDlg.h"
#include "DnUIDefine.h"

#ifdef PRE_ADD_PVP_TOURNAMENT

using namespace PvPTournamentUIDef;

class CDnPVPTournamentDataMgr;
class CDnPVPTournamentFinalResultDlg : public CDnCustomDlg
{
public:
	struct SWinnersItem
	{
		CDnJobIconStatic* pJobIcon;
		CEtUITextureControl* pRankIcon;  // Rank icon 
		CEtUIStatic* pLevel;
		CEtUIStatic* pName;
		CEtUITextureControl* pGuildMark;
		CEtUIStatic* pGuildName;
		CEtUIStatic* pKillCount;
		CEtUIStatic* pDeathCount;
#ifdef PRE_ADD_PVP_TOURNAMENT_WINNERXP
		CEtUIStatic* pPVPXP;
#endif
		CEtUIStatic* pMedalCount;
		CEtUITextureControl* pMedalIcon;
		CDnItemSlotButton* pItemSlot;
		CDnItem* pItem;
#ifdef PRE_PVP_GAMBLEROOM
		CEtUIStatic* pPrize;
#endif // PRE_PVP_GAMBLEROOM

		SWinnersItem()
		{
			pJobIcon = NULL;
			pRankIcon = NULL;
			pLevel = NULL;
			pName = NULL;
			pGuildMark = NULL;
			pGuildName = NULL;
			pKillCount = NULL;
			pDeathCount = NULL;
#ifdef PRE_ADD_PVP_TOURNAMENT_WINNERXP
			pPVPXP = NULL;
#endif
			pMedalCount = NULL;
			pMedalIcon = NULL;
			pItemSlot = NULL;
			pItem = NULL;
#ifdef PRE_PVP_GAMBLEROOM
			pPrize = NULL;
#endif // PRE_PVP_GAMBLEROOM
		}

		~SWinnersItem() { SAFE_DELETE(pItem); }

		void ShowAll(bool bShow);
	};

	CDnPVPTournamentFinalResultDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL);
	virtual ~CDnPVPTournamentFinalResultDlg(void);

	virtual void InitialUpdate();
	virtual void Initialize(bool bShow);
	virtual void Process(float fElapsedTime);
	virtual void ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */);

	CDnPVPTournamentDataMgr* GetDataMgr() const;

	void AddUser(DnActorHandle hUser);
	void RemoveUser(DnActorHandle hUser);
	void SetUserScore(int nSessionID, int nKOCount, int nKObyCount, int nCaptainKOCount, UINT uiKOP, UINT uiAssistP, UINT uiTotalP);
	void SetUserState(DnActorHandle hUser, int nState);
	void SetPVPXP(UINT nSessionID, UINT uiAddXPScore, UINT uiTotalXPScore, UINT uiMedalScore);
	void UpdateUsers();

	void FinalResultOpen();
#ifdef PRE_PVP_GAMBLEROOM
	void SetGamebleRoom( bool bGamebleRoom ) { m_bGamebleRoom = bGamebleRoom; }
#endif // PRE_PVP_GAMBLEROOM

private:
	void UpdateWinnerUsers(const SFinalReportUserInfo& info, int slotNumber);
	void UpdateLoserUsers(const SFinalReportUserInfo& info, int rank);

	SWinnersItem m_WinnersUI[MAX_WINNERS_FOR_FINAL_RESULT_DLG];
	CEtUIListBoxEx* m_pLosersListBox;
	CEtUIButton* m_pCloseButton;
	bool m_IsFinalResult;
	float m_fTotal_ElapsedTime;

	EtTextureHandle m_hPVPMedalIconImage;

#ifdef PRE_PVP_GAMBLEROOM
	bool m_bGamebleRoom;
#endif // PRE_PVP_GAMBLEROOM
};

#endif