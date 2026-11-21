#pragma once
#include "DnCustomDlg.h"
#include "DnPVPDetailedInfoDlg.h"
#include "DnPVPUserToolTipDlg.h"

class CDnGuildWarResultDlg : public CDnCustomDlg
{
public:
	CDnGuildWarResultDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGuildWarResultDlg(void);

public:

	enum eUserState{
		Normal_S = 0,
		MINE_S = 1,
		KILLED_S = 2,
	};

	enum eTeamState{
		RED_TEAM = 0,
		BLUE_TEAM,
		ALL_TEAM,
		MAX_TEAM,
	};

	enum eSlotLimit
	{
		GuildWar_MaxUserSlot = 32,
		GuildWar_TeamUserSlot = 16,
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
		int iState;
		UINT uiOccupation;	//Capture count
		BYTE cTeam;			//Team
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
			, uiAssistP(0)
			, uiTotalScore(0)
			, uiXP(0)
			, uiTotalXP(0)
			, uiMedal(0)
			, iState(Normal_S)
			, uiOccupation(0)
			, cTeam(ALL_TEAM)
#if defined( PRE_ADD_REVENGE )
			, eRevengeUser( Revenge::RevengeTarget::eRevengeTarget_None )
#endif	// #if defined( PRE_ADD_REVENGE )
		{

		}
	};

	struct STeamData
	{
		int nRedTeamScore;
		int nRedTeamKill;
		int nRedTeamResource;
		int nRedOccupation;
		WCHAR wszRedGuildName[512];
		bool bRedBossKill;

		int nBlueTeamScore;
		int nBlueTeamKill;
		int nBlueTeamResource;
		int nBlueOccupation;
		WCHAR wszBlueGuildName[512];
		bool bBlueBossKill;

		STeamData()
			: nRedTeamScore( 0 )
			, nRedTeamKill( 0 )
			, nRedTeamResource( 0 )
			, nRedOccupation( 0 )
			, bRedBossKill( false )
			, nBlueTeamScore( 0 )
			, nBlueTeamKill( 0 )
			, nBlueTeamResource( 0 )
			, nBlueOccupation( 0 )
			, bBlueBossKill( false )
		{
			memset( wszBlueGuildName, NULL, sizeof(WCHAR)*512 );
			memset( wszRedGuildName, NULL, sizeof(WCHAR)*512 );
		}
	};

protected:
	CEtUIStatic * m_pStaticTeam[ALL_TEAM];
	CEtUIStatic * m_pStaticTeamName[ALL_TEAM];
	CEtUIStatic * m_pStaticBossKill[ALL_TEAM];
	CEtUIStatic * m_pStaticBossNotKill[ALL_TEAM];
	CEtUIStatic * m_pStaticResource[ALL_TEAM];
	CEtUIStatic * m_pStaticTotalKill[ALL_TEAM];
	CEtUIStatic * m_pStaticOccupation[ALL_TEAM];
	CEtUIStatic * m_pStaticScore[ALL_TEAM];

	CEtUIStatic * m_pStaticMainName[MAX_TEAM];
	CEtUIStatic * m_pStaticMainBar[MAX_TEAM];

	CEtUIRadioButton * m_pButtonAll;
	CEtUIRadioButton * m_pButtonRed;
	CEtUIRadioButton * m_pButtonBlue;
	CEtUIButton * m_pButtonClose;

	CEtUIListBoxEx * m_pListBoxUser;

	std::vector<SUserInfo> m_vUserInfo;

	bool m_IsFinalResult;
	float m_fTotal_ElapsedTime;
	int m_eTeamState;

	UINT m_nBestUserKillCount;
	UINT m_nBestUserDeathCount;
	UINT m_nBestUserSessionID;

	void UpdateUser( int eTeamState );

public:
	void AddUser( DnActorHandle hUser );
	void RemoveUser( DnActorHandle hUser  );
	void ProcessScoring( int nSessionID, int nKOCount, int nKObyCount, int nKOP );
	void FinalResultOpen();

	void SetBoardName(std::wstring wszName);
	void SetUserScore( int nSessionID, int nKOCount, int nKObyCount, UINT uiKOP, UINT uiAssistP, UINT uiTotalP, int nOccupation );
	void SetUserState( DnActorHandle hUser , int iState );
	void RestartRound();
	void SetPVPXP( UINT nSessionID, UINT uiAddXPScore,UINT uiTotalXPScore,UINT uiMedalScore);
	void SetData( STeamData & sData );
	SUserInfo GetUserInfo( const int nSessionID );

	std::wstring GetBestUserName();
	int GetBestUserScore();
	int GetBestUserDeathScore();

	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};