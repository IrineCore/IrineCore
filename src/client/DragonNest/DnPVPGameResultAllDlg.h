#pragma once
#include "DnCustomDlg.h"
#include "DnPVPDetailedInfoDlg.h"
#include "DnPVPUserToolTipDlg.h"


// Rotha: Redefining solo mode inside DnPVPGAmeResultDlg looked too messy, so create a new one. //
// A new UI displaying Assist info was requested, so configure it to switch using EnableAssistMode. 

class CDnPVPGameResultAllDlg : public CDnCustomDlg
{

public:

	enum
	{
		PVP_MEDAL_ICON_XSIZE = 23,
		PVP_MEDAL_ICON_YSIZE = 23,
	};
	
	enum eUserState{
		Normal_S = 0,
		MINE_S = 1,
		KILLED_S = 2,
	};

	enum eSlotLimit
	{
		PvP_MaxUserSlot = 16,
		PvP_TeamUserSlot = 8,
	};

	struct SUserInfo
	{
		int nSessionID;
		int nLevel;
		byte cJobClassID;
		byte cPVPlevel;
		std::wstring wszUserName;
		TGuildSelfView GuildSelfView;
		UINT uiKOCount;		// Kill
		UINT uiKObyCount;	// Death
		UINT uiKOP;			// Seems to be kill score.
		UINT uiAssist;
		UINT uiTotalScore;	// Total sum
		UINT uiXP;			// EXP
		UINT uiTotalXP;
		UINT uiMedal;		// Medal count
		int iState;
#if defined( PRE_ADD_REVENGE )
		UINT eRevengeUser;
#endif	// #if defined( PRE_ADD_REVENGE )

		SUserInfo()
			: nSessionID(-1)
			, nLevel(0)
			, cJobClassID(255)
			, cPVPlevel(0)
			, uiKOCount(0)
			, uiKObyCount(0)
			, uiKOP(0)
			, uiAssist(0)
			, uiTotalScore(0)
			, uiXP(0)
			, uiTotalXP(0)
			, uiMedal(0)
			, iState(Normal_S)
#if defined( PRE_ADD_REVENGE )
			, eRevengeUser( Revenge::RevengeTarget::eRevengeTarget_None )
#endif	// #if defined( PRE_ADD_REVENGE )
		{
		}
	};


protected:	

	float m_fTotal_ElapsedTime;
	
	bool m_IsFinalResult;
	bool m_bAssistMode;

	UINT m_nBestUserKillCount;
	UINT m_nBestUserDeathCount;
	UINT m_nBestUserSessionID;
	std::vector<SUserInfo> m_vUserInfo;
	
	CEtUIStatic *m_pBoardName;
	CEtUIStatic *m_pTeamLevel[PvP_MaxUserSlot];
	CEtUIStatic *m_pGuildName[PvP_MaxUserSlot];
	CEtUIStatic *m_pTeamName[PvP_MaxUserSlot];
	CEtUIStatic *m_pKOCount[PvP_MaxUserSlot];
	CEtUIStatic *m_pKObyCount[PvP_MaxUserSlot];
	CEtUIStatic *m_pTotalPoint[PvP_MaxUserSlot];
	CEtUIStatic *m_pXP[PvP_MaxUserSlot];
	CEtUIStatic *m_pPCRoom[PvP_MaxUserSlot];
	CEtUIStatic *m_pStaticAssist[PvP_MaxUserSlot];
	CEtUIStatic *m_pMedal[PvP_MaxUserSlot];
	CEtUIStatic *m_pStaticCover[PvP_MaxUserSlot];

	CEtUIButton *m_pButtonClose;
	CDnJobIconStatic *m_pJobIcon[PvP_MaxUserSlot];
	EtTextureHandle m_hPVPMedalIconImage;

	CEtUITextureControl *m_pMedalIcon[PvP_MaxUserSlot];
	CEtUITextureControl *m_pSlotRank[PvP_MaxUserSlot];
	CEtUITextureControl *m_pGuildMark[PvP_MaxUserSlot];


public:
	CDnPVPGameResultAllDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPGameResultAllDlg(void);

	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

protected:
	void UpdateUser( );
	void InitializeSlot();

public:

	void EnableAssistMode(bool bEnable){m_bAssistMode = bEnable;} // Please set this before Initialize.
	void AddUser( DnActorHandle hUser );
	void RemoveUser( DnActorHandle hUser  );
	void FinalResultOpen();
	void SetBoardName(std::wstring wszName){m_pBoardName->SetText(wszName);}
	void SetUserScore( int nSessionID, int nKOCount, int nKObyCount, UINT uiKOP, UINT uiAssistP , UINT uiTotalP );
	void SetUserState( DnActorHandle hUser , int iState );
	void RestartRound();
	void SetPVPXP( UINT nSessionID, UINT uiAddXPScore,UINT uiTotalXPScore,UINT uiMedalScore);

	std::wstring GetBestUserName();
	int GetBestUserScore();
	int GetBestUserDeathScore();

#ifdef PRE_ADD_PVP_COMBOEXERCISE
	void SetComboExerciseMode(); // UI settings in combo practice mode.
#endif // PRE_ADD_PVP_COMBOEXERCISE
};
