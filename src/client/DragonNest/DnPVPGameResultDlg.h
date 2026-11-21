#pragma once
#include "DnCustomDlg.h"
#include "DnPVPDetailedInfoDlg.h"
#include "DnPVPUserToolTipDlg.h"

class CDnPVPGameResultDlg : public CDnCustomDlg
{
public:
	CDnPVPGameResultDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPGameResultDlg(void);

public:
	enum eUserState{
		Normal_S = 0,
		MINE_S = 1,
		KILLED_S = 2,
	};

	enum eSlotLimit // Rotha Solo mode: As PVP Team member count type changed (increased), calculate using slot limit.
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
		UINT uiAssistP;		// Assist score
		UINT uiTotalScore;	// Total sum
		UINT uiXP;			// EXP
		UINT uiTotalXP;
		UINT uiMedal;		// Medal count
		UINT uiBossKOCount;	// Death
		int iState;
		UINT uiOccupationCount;
		UINT uiOccupationCancelCount;
#if defined( PRE_ADD_REVENGE )
		UINT eRevengeUser;
#endif	// #if defined( PRE_ADD_REVENGE )
#ifdef PRE_MOD_PVP_LADDER_XP
		int uiLadderPVPXP;	// Colosseum points earned in ladder
#endif // PRE_MOD_PVP_LADDER_XP
		SUserInfo()
			: nSessionID(-1)
			, nLevel(0)
			, cJobClassID(255)
			, cPVPlevel(0)
			, uiKOCount(0)
			, uiKObyCount(0)
			, uiKOP(0)
			, uiAssistP(0)
			, uiTotalScore(0)
			, uiXP(0)
			, uiTotalXP(0)
			, uiMedal(0)
			, uiBossKOCount(0)
			, iState(Normal_S)
			, uiOccupationCount( 0 )
			, uiOccupationCancelCount( 0 )
#if defined( PRE_ADD_REVENGE )
			, eRevengeUser( Revenge::RevengeTarget::eRevengeTarget_None )
#endif	// #if defined( PRE_ADD_REVENGE )
#ifdef PRE_MOD_PVP_LADDER_XP
			, uiLadderPVPXP(0)
#endif // PRE_MOD_PVP_LADDER_XP
		{

		}
	};

protected:	
	std::vector<SUserInfo> m_vMyUserInfo;
	std::vector<SUserInfo> m_vEnemyTeamUserInfo;

	CEtUIStatic *m_pMyTeamLevel[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyTeamLevel[PvP_TeamUserSlot];

	CDnJobIconStatic *m_pMyJobIcon[PvP_TeamUserSlot];
	CDnJobIconStatic *m_pEnemyJobIcon[PvP_TeamUserSlot];

	CEtUITextureControl *m_pMySlotRank[PvP_TeamUserSlot];
	CEtUITextureControl *m_pEnemySlotRank[PvP_TeamUserSlot];

	CEtUITextureControl *m_pMyGuildMark[PvP_TeamUserSlot];
	CEtUITextureControl *m_pEnemyGuildMark[PvP_TeamUserSlot];

	CEtUIStatic *m_pMyGuildName[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyGuildName[PvP_TeamUserSlot];

	CEtUIStatic *m_pMyTeamName[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyTeamName[PvP_TeamUserSlot];

	CEtUIStatic *m_pMyKOCount[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyKOCount[PvP_TeamUserSlot];

	CEtUIStatic *m_pMyKObyCount[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyKObyCount[PvP_TeamUserSlot];

	CEtUIStatic *m_pMyAssistPoint[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyAssistPoint[PvP_TeamUserSlot];

	CEtUIStatic *m_pMyTotalPoint[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyTotalPoint[PvP_TeamUserSlot];

	CEtUIStatic *m_pMyXP[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyXP[PvP_TeamUserSlot];

	CEtUIStatic *m_pMyPCRoom[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyPCRoom[PvP_TeamUserSlot];

	CEtUITextureControl *m_pMyMedalIcon[PvP_TeamUserSlot];
	CEtUITextureControl *m_pEnemyMedalIcon[PvP_TeamUserSlot];

	CEtUIStatic *m_pMyMedal[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyMedal[PvP_TeamUserSlot];

	CEtUIStatic *m_pStaticMy[PvP_TeamUserSlot];

	CEtUIStatic *m_pKillBoss[2]; // Used to hide the Kill (Captain) part; no need to show it in other modes.
	CEtUIStatic *m_pMyBossKOCount[PvP_TeamUserSlot];
	CEtUIStatic *m_pEnemyBossKOCount[PvP_TeamUserSlot];

	CEtUIStatic *m_pXpCount[2];     // Colosseum XP title static; in ladder mode, converted to rating delta, etc.
	CEtUIStatic *m_pPCRoom[2];      // Hide when unused
	CEtUIStatic *m_pMedalReward[2]; // 

	CEtUIStatic *m_pKillUser[2];	// In Occupation War 
	CEtUIStatic *m_pDeath[2];		// Used as another value
	CEtUIStatic *m_pAssist[2];		// Do it.

	enum
	{
		PVP_MEDAL_ICON_XSIZE = 23,
		PVP_MEDAL_ICON_YSIZE = 23,
	};
	EtTextureHandle m_hPVPMedalIconImage;

	bool m_IsFinalResult;
	bool m_bIsLadder;
	bool m_bIsOccupation;

	CEtUIButton *m_pButtonClose;
	float m_fTotal_ElapsedTime;


#ifdef PRE_MOD_PVPOBSERVER
	CEtUIStatic * m_pStaticFriendly;
	CEtUIStatic * m_pStaticEnemy;
	bool m_bShowResult;
#endif // PRE_MOD_PVPOBSERVER


protected:
	void UpdateUser( );
	void InitializeSlot();

public:
	void AddUser( DnActorHandle hUser , bool IsMyTeam );
	void RemoveUser( DnActorHandle hUser  );
	void ProcessScoring( int nSessionID, int nKOCount = 0, int nKObyCount = 0, int nKOP = 0, int nAssistP = 0 );
	void FinalResultOpen();

	void SetUserScore( int nSessionID, int nKOCount, int nKObyCount,int nCaptainKOCount, UINT uiKOP, UINT uiAssistP, UINT uiTotalP );
	void SetUserState( DnActorHandle hUser , int iState );
	void RestartRound();
	void SetPVPXP( UINT nSessionID, UINT uiAddXPScore, UINT uiTotalXpScore, UINT uiMedalScore );
#ifdef PRE_MOD_PVP_LADDER_XP
	void SetPVPXP( UINT nSessionID, UINT uiAddXPScore, UINT uiTotalXpScore, UINT uiMedalScore, UINT uiLadderPVPXP );
#endif // PRE_MOD_PVP_LADDER_XP
	void EnableCaptainMode(bool bTrue);

	void EnableLadderMode(bool bTrue);
	void EnableOccupationMode( bool bTrue );
	void SetOccupationScore( int nSessionID, UINT uiOccupation, UINT uiOccupationSteal );
	void DisableAssist(bool bTrue);

	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

#ifdef PRE_WORLDCOMBINE_PVP
	void ChangePVPScoreTeam( int nSessionID, int nTeam );
#endif // PRE_WORLDCOMBINE_PVP
};
