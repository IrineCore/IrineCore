#pragma once
#include "Task.h"
#include "MessageListener.h"
#include "DNProtocol.h"
#include "DNPacket.h"

#include "DnTableDB.h"

#include "DnBlindDlg.h"

// Enable this define to remove client-side guild permission setting restrictions (only applicable after guild renewal).
//#define PRE_DEBUG_GUILDAUTH

class CDnGuildDlg;

class CDnGuildTask : public CTask, public CTaskListener, public CSingleton<CDnGuildTask>, public CEtUICallback, public CLostDeviceProcess, public CDnBlindCallBack
{
public:
	enum
	{
		AUTO_REQUEST_GUILDINFO_FIRST = 30,				// Make the first guild request 30 seconds after SCEnter (30 seconds after guild task creation).
		AUTO_REQUEST_GUILDINFO_DIALOG_SHOW_TRUE = 120,	// Refresh interval when dialog is shown after first request
		AUTO_REQUEST_GUILDINFO_DIALOG_SHOW_FALSE = 300,	// Refresh interval when dialog is hidden after first request
		REQUEST_GUILDINFO_DELAY = 60,					// Once the dialog is opened, a request can be sent again after 1 minute.
	};

	enum eGuildMemberSortType
	{
		GuildMemberSort_Name,
		GuildMemberSort_Level,
		GuildMemberSort_Job,
		GuildMemberSort_Role,
		GuildMemberSort_LastLogoutDate,

		GuildMemberSort_Amount = 5,
	};


	struct SGuildLevelInfo
	{
		int nLevel;
		int nReqGuildPoint;
		int nDailyGuildPointLimit;
		int nDailyMissionPointLimit;
	};

	CDnGuildTask(void);
	virtual ~CDnGuildTask(void);

	bool Initialize();
	void Finalize() {}

public:
	virtual void OnDisconnectTcp( bool bValidDisconnect ) {}
	virtual void OnDisconnectUdp( bool bValidDisconnect ) {}
	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

#ifdef PRE_ADD_BEGINNERGUILD
	virtual void OnBlindClosed();
#endif

public:
	void RequestCreateGuild( const WCHAR *wszGuildName );			// Note : Create guild
	void RequestGetGuildInfo( bool bNeedMemeberList );				// Note : Request guild info
	void RequestDismissGuild();										// Note : Delete guild
	void RequestInviteGuildMember( const WCHAR *wszPlayerName );	// Note : Guild invite
	void RequestInviteGuildMemberAck( bool bAccept );				// Note : Guild invite response
	void RequestLeaveGuild();										// Note : Leave guild
	void RequestExileGuild( UINT nAccountDBID, INT64 nCharacterDBID );
	void RequestChangeGuildInfo(BYTE btGuildUpdate, int iInt1 = 0, int iInt2 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL);
	void RequestChangeGuildMemberInfo(BYTE btGuildUpdate, int iInt1 = 0, int iInt2 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL,  UINT nChgAccountDBID = 0, INT64 nChgCharacterDBID = 0);
	void RequestGetGuildHistoryList(INT64 biIndex, bool bDirection);
	void RequestGetGuildStorageHistoryList(int nPage);
	void RequestChangeGuildInfoEx(BYTE btGuildUpdate, int iInt1 = 0, int iInt2 = 0, int iInt3 = 0, int iInt4 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL);

	void RequestGetGuildRecruitRegisterInfo();
#ifdef PRE_ADD_GUILD_EASYSYSTEM
	void RequestRegisterGuildRecruit( bool bModify, BYTE *pClassGrade, int nMinLevel, int nMaxLevel, const WCHAR *pGuildIntroduction, BYTE cPurposeCode, bool bCheckHomepage );
	void RequestCancelGuildRecruit();
	void RequestGetGuildRecruitList( int nPage, BYTE cPurposeCode, const WCHAR *pGuildName, BYTE cSortType );
#else
	void RequestRegisterGuildRecruit( bool bModify, BYTE *pClassGrade, int nMinLevel, int nMaxLevel, const WCHAR *pGuildIntroduction );
	void RequestCancelGuildRecruit();
	void RequestGetGuildRecruitList( int nPage );
#endif
	void RequestGetGuildRecruitRequestCount();
	void RequestGetGuildRecruitMyList();
	void RequestGetGuildRecruitCharacter();
	void RequestJoinGuild( TGuildUID GuildUID, bool bCancel = false );
	void RequestAcceptJoinGuild( bool bAccept, INT64 nCharacterDBID, const WCHAR *pToCharacterName );

	void RequestEnrollGuildWar();

	TGuild *GetGuildInfo() { return &m_Guild; }
	bool GetAuth( eGuildRoleType pGuildRoleType, eGuildAuthType pGuildAuthType );
	int GetGuildMemberCount() { return (int)m_vecGuildMember.size(); }
	int GetGuildMemberLogonCount();
	TGuildMember *GetGuildMemberFromArrayIndex( int nArrayIndex );
	TGuildMember *GetGuildMemberFromCharacterDBID( INT64 nDBID );
	TGuildMember *GetGuildMemberFromName( LPCWSTR wszName );
	TGuildMember *GetMyGuildMemberInfo();

	// Guild chat
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	void OnRecvGuildChat( LPCWSTR wszMsg, INT64 nDBID, bool bIsDoorsMobile );
#else
	void OnRecvGuildChat( LPCWSTR wszMsg, INT64 nDBID );
#endif

	// Get own guild info through the player object.
	bool IsMaster();
	BYTE GetMyRole();

	// Other guild information
	int GetGuildCreateTex() { return m_nGuildCreateTax; }

	SGuildLevelInfo *GetGuildLevelInfo( int nGuildLevel );
	SGuildLevelInfo *GetGuildLevelInfoFromGuildExp( int nGuildExp );

	int GetMaxTakeItemByRole( eGuildRoleType eRole );	// Item withdrawal count index for the guild rank
	int GetMaxTakeItemData( int nIndex );				// Table data configured by index
	int GetMaxTakeItemIndexCount();						// Total count of table data
	int GetMaxWithDrawByRole( eGuildRoleType eRole );
	int GetMaxWithDrawData( int nIndex );
	int GetMaxWithDrawIndexCount();

	// Sort guild members 
	bool SortGuildMember( eGuildMemberSortType Type, bool bReverse );

	// Guild war related info. Considering grouping into a struct if it grows.
	void SetCurrentGuildWarEventStep( char cCurrentStep ) { m_cCurrentGuildWarEventStep = cCurrentStep; }
	void SetCurrentGuldWarScheduleID( short wCurrentScheduleID ) { m_wCurrentGuldWarScheduleID = wCurrentScheduleID; }	
	char GetCurrentGuildWarEventStep() { return m_cCurrentGuildWarEventStep; }
	short GetCurrentGuldWarScheduleID() { return m_wCurrentGuldWarScheduleID; }	

	// Returns a handle in manager style, so just use it without deleting.
	EtTextureHandle GetGuildMarkTexture( const TGuildView &View );
	bool IsShowGuildMark( const TGuildView &View );


	// Returns a handle in manager style, so just use it without deleting.
	EtTextureHandle GetGuildMarkTexture( const TGuildSelfView &View );
	bool IsShowGuildMark( const TGuildSelfView &View );

	bool IsNotRecruitMember() { return m_bNotRecruitMember; }

	bool IsHaveGuildInfo(){return m_bIsHaveGuildInfo;}
#ifdef PRE_ADD_BEGINNERGUILD
	bool IsBeginnerGuild() const;
	void MessageBoxForGraduateBeginnerGuild();
	bool CheckMessageBoxForGraduateBeginnerGuild() { return m_bMessageBoxForGraduateBeginnerGuild; }
#endif

	tstring GetGuildMasterName();

	virtual void OnLostDevice();
	virtual void OnResetDevice();

protected:
	void OnRecvGuildMessage(int nSubCmd, char *pData, int nSize);
	void OnRecvGuildRecruitMessage(int nSubCmd, char *pData, int nSize);

	void OnRecvCreateGuild( SCCreateGuild *pPacket );
	void OnRecvChangeGuildSelfView( SCChangeGuildSelfView *pPacket );
	void OnRecvGetGuildInfo( SCGetGuildInfo *pPacket );
	void OnRecvDismissGuild( SCDismissGuild *pPacket );
	void OnRecvInviteGuildMemberReq( SCInviteGuildMemberReq *pPacket );
	void OnRecvInviteGuildMemberAck( SCInviteGuildMemberAck *pPacket );
	void OnRecvLeaveGuild( SCLeaveGuildMember *pPacket );
	void OnRecvExileGuild( SCExileGuildMember *pPacket );
	void OnRecvChangeGuildInfo( SCChangeGuildInfo *pPacket );
	void OnRecvChangeGuildMemberInfo( SCChangeGuildMemberInfo *pPacket );
	void OnRecvGetGuildHistoryList( SCGetGuildHistoryList *pPacket );
	void OnRecvGuildMemberLoginList( SCGuildMemberLoginList *pPacket );
	void OnRecvOpenGuildWare( SCOpenGuildWare *pPacket );
	void OnRecvGetGuildWareHistoryList( SCGetGuildWareHistory *pPacket );
	void OnRecvExtendGuildWare( SCExtendGuildWare *pPacket );
	void OnRecvChangeGuildName( SCChangeGuildName *pPacket );
	void OnRecvEnrollGuildWar( SCEnrollGuildWar *pPacket );
	void OnRecvChangeGuildWarEvent( SCGuildWarEvent *pPacket );
	void OnRecvGuildWarTournamentWin( SCGuildWarTournamentWin* pPacket );
	void OnRecvUpdateGuildExp( SCUpdateGuildExp *pPacket );
	void OnRecvChangeGuildMark( SCChangeGuildMark *pPacket );

	void OnRecvGuildRecruitRegisterInfo( GuildRecruitSystem::SCGuildRecruitRegisterInfo *pPacket );
	void OnRecvGuildRecruitRegister( GuildRecruitSystem::SCGuildRecruitRegister *pPacket );
	void OnRecvGuildRecruitList( GuildRecruitSystem::SCGuildRecruitList *pPacket );
	void OnRecvGuildRecruitRequestCount( GuildRecruitSystem::SCGuildRecruitRequestCount *pPacket );
	void OnRecvGuildRecruitMyList( GuildRecruitSystem::SCMyGuildRecruitList *pPacket );
	void OnRecvGuildRecruitCharacter( GuildRecruitSystem::SCGuildRecruitCharacterList *pPacket );
	void OnRecvGuildRecruitRequest( GuildRecruitSystem::SCGuildRecruitRequest *pPacket );
	void OnRecvGuildRecruitAcceptResult( GuildRecruitSystem::SCGuildRecruitAccept *pPacket );
	void OnRecvGuildRecruitMemberResult( GuildRecruitSystem::SCGuildRecruitMemberResult *pPacket );

	void OnRecvGuildLevelUp( SCGuildLevelUp *pPacket );

	void OnRecvGuildGetRewardItem(SCGetGuildRewardItem* pPacket);
	void OnRecvGuildBuyRewardItem(SCBuyGuildRewardItem* pPacket);
	void OnRecvGuildAddRewardItem(SCAddGUildRewardItem* pPacket);
	void OnRecvGuildExtendGuildSize(SCExtendGuildSize* pPacket);
	void OnRecvPlayerGuildInfo(SCPlayerGuildInfo* pPacket);

	void RefreshGuildRewardInfo();
	void OnRecvGetGuildMember(SCGetGuildMember* pPacket);

	void SetGuildInfo( const TGuild &Guild );
	void SetGuildMemberInfo( int nMemberCount, int nPage, const TGuildMember *pMemberList );

	void AddGuildMember( TGuildMember *pMember );
	void DelGuildMember( INT64 nDBID );

	void RefreshGuildDlg( bool bRefreshShowing = false );	// true to call refresh only when window is open. Default is force refresh.

	void OnChangeGuildWareSize();

	int GetCurrentGuildStorageSlotCount();

	EtTextureHandle GenerateGuildMarkTexture( int nIndex1, int nIndex2, int nIndex3 );

	TGuild m_Guild;
	std::vector<TGuildMember> m_vecGuildMember;

	float m_fAutoRequestDelayTime;	// Automatically requesting
	float m_fDialogShowDelayTime;	// Remember timestamp so requests have a fixed delay even if the dialog is repeatedly opened and closed.

	TGuildUID m_InviteGuildUID;		// Guild that sent a guild invite request to self. Temporary variable.
	UINT m_InviteSESID;			// Session ID of the user who sent a guild invite request to self.
	UINT m_InviteACCID;
	int m_nGuildCreateTax;

	bool m_bIsHaveGuildInfo;

	std::vector<SGuildLevelInfo> m_vecGuildLevelInfo;

	std::map<int, int> m_mapMaxTakeItemInfo;
	std::map<int, int> m_mapMaxWithDrawInfo;

	char	m_cCurrentGuildWarEventStep;
	short	m_wCurrentGuldWarScheduleID;

	bool m_bNotRecruitMember;

#ifdef PRE_ADD_BEGINNERGUILD
	bool m_bMessageBoxForGraduateBeginnerGuild;
	bool m_bChangedGuildChat;
#endif

	int m_nMarkGenerateDelayFrameCount;
	struct SGuildMarkInfo
	{
		int nIconIndex[3];
		EtTextureHandle hTexture;
	};
	std::vector< SGuildMarkInfo > m_vecGuildMarkInfo;


protected:
	TGuildRewardItem m_GuildRewardItem[GUILDREWARDEFFECT_TYPE_CNT];	//Reward info received from server..

public:
	typedef std::list<GuildReward::GuildRewardInfo> GUILDREWARDINFO_LIST;
	typedef std::map<int, GUILDREWARDINFO_LIST> GUILDREWARD_MAP;
	GUILDREWARD_MAP m_GuildRewardList;

	bool IsAvailableRepurchase(const GuildReward::GuildRewardInfo &info);
	bool IsPrerequisite(const GuildReward::GuildRewardInfo &info);
	bool IsAppliedRewardByBetterThan(const GuildReward::GuildRewardInfo &info);	//Whether a better one is already applied...
	bool IsPurchased(const GuildReward::GuildRewardInfo &info);
	bool IsAvailableReward(const GuildReward::GuildRewardInfo &info);
	__time64_t GetRemainPeriod(const GuildReward::GuildRewardInfo &info);
	int GetPurchasedItemID(const GuildReward::GuildRewardInfo &info);

// 	bool IsAppliedGuildRewardType(int rewardType);
// 	bool IsAppliedGuildRewardType(int rewardType, int rewardID);

	int CollectGuildRewardInfo(GUILDREWARDINFO_LIST & guildRewardList, TGuildRewardItem *pGuildReward = NULL);

	void RequestBuyGuildRewardItem(int nItemID);

	//Check guild reward value.
	int GetGuildRewardValue(GuildReward::GuildRewardType _type);
	//Guild reward restriction period
	__time64_t GetGuildRewardExpireDate(GuildReward::GuildRewardType _type);

	void RequestPlayerGuildInfo(int nSessionID);

	int GetGuildRewardEffectValue(int nType)
	{
		if (nType < 0 || nType >= GUILDREWARDEFFECT_TYPE_CNT )
			return 0;

		return m_GuildRewardItem[nType].nEffectValue;
	}

#ifdef PRE_ADD_GUILD_CONTRIBUTION
	void OnRecvGuildContributionPoint( GuildContribution::SCGuildContributionPoint* pPacket );
	void OnRecvGuildContributionRankList( GuildContribution::SCGuildContributionRank* pPacket );

	void RequsetGuildRankList();
#endif 
};

#define GetGuildTask()	CDnGuildTask::GetInstance()