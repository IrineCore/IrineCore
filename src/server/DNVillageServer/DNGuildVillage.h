#pragma once

#include "DNGuildBase.h"
#include "DNGuildWare.h"
#include "DNUserSession.h"
#include "DNUserSessionManager.h"

class CDNGuildVillage : public CDNGuildBase, public TBoostMemoryPool<CDNGuildVillage>
{
public:
	CDNGuildVillage();
	~CDNGuildVillage();

	void			Reset();
	void			OpenWare(const TAGetGuildWareInfo* pPacket);
	

	// Check if guild member
	bool			IsValiadMember(CDNUserSession *pUserObj, INT64 nCharacterDBID);

	// Broadcast to all logged-in guild members
	void			SendGuildMemberLoginList(SCGuildMemberLoginList* pPacket);

	// Check guild resource.
	bool			CheckGuildResource(TGuildMember* aGuildMemeber);

	void			SendAddGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszCharacterName, TP_JOB nJob, char cLevel, __time64_t pJoinDate, TCommunityLocation* pLocation);
	void			SendDelGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, bool bIsExiled);
	void			SendChangeGuildInfo(UINT nAccountDBID, INT64 nCharacterDBID, BYTE btGuildUpdate, int iInt1, int iInt2, int iInt3, int iInt4, INT64 biInt64, LPCWSTR pText);
	void			SendChangeGuildMemberInfo(UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nChgAccountDBID, INT64 nChgCharacterDBID, BYTE btGuildMemberUpdate, int iInt1, int iInt2, INT64 biInt64, LPCWSTR pText);

	void			SendGuildChat(UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen);
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	void SendDoorsGuildChat(INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen);
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT

#ifdef PRE_ADD_JOINGUILD_SUPPORT
	void			SendRefreshGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo, CDNUserSession* pSession=NULL, bool bNoUpdate = false);
#else		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
	void			SendRefreshGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo, CDNUserSession* pSession=NULL);
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
	void			SendRefreshGuildCoin(INT64 biTotalCoin, CDNUserSession* pSession=NULL);
	void			SendChangeGuildName(const wchar_t* wszGuildName);
	void			SendChangeGuildMark(const MAGuildChangeMark *pPacket);
	void			SendExtendGuildWare(short wTotalSize);

	void			SendGuildLevelUp (int nLevel);
	void			SendUpdateGuildExp(BYTE cPointType, int nPointValue, int nGuildExp, INT64 biCharacterDBID, int nMissionID);
	void			SendGuildRewardItem( TGuildRewardItem* GuildRewardItem );
	void			SendAddGuildRewardItem( TGuildRewardItem GuildRewardItem );
	void			SendExtendGuildSize( short nGuildSize );	

	void			UpdateGuildInfo(CDNUserSession* pSession, const TAChangeGuildInfo *pPacket);
	CDNGuildWare*	GetGuildWare();

	void			UpdateWare(const MAGuildWareInfo *pPacket);

	// RECV
	void			RecvApiOpenGuildWare(CDNUserSession * pSession);
	void			RecvCsCloseGuildWare(CDNUserSession * pSession);
	void			RecvCsGetGuildWareHistory(CDNUserSession * pSession, CSGetGuildWareHistory *pPacket);
	void			RecvMaRefreshGuildItem(MARefreshGuildItem* pPacket);
	void			RecvMaRefreshGuildCoin(MARefreshGuildCoin* pPacket);

	// UPDATE
	void			UpdateMemberInfo(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket);
	virtual void	GuildWarReset();
	void			UpdateWarInfoOnAllMember(short wScheduleID, BYTE cTeamColorCode);
	void			SetGuildWarMyRankingInfo(TAGetGuildWarPointGuildTotal* pGetGuildWarPointGuildTotal );
	inline SMyGuildWarMissionGuildRankingInfo* GetGuildWarMyRankingInfo() { return &m_sMyGuildWarRankingInfo;};
	void			SetGuildWarTotalRankingInfo(TAGetGuildWarPointGuildPartTotal* pGetGuildWarPointGuildPartTotal );	
	inline SMyGuildWarMissionGuildRankingInfo* GetGuildWarTotalRankingInfo() { return m_sMyGuildWarMissionGuildRankingInfo;};
	inline bool		GetGuildWarStats() { return m_bGuildWarStats;};	
	inline void		SetGuildWarPoint(int nGuildWarPoint) { m_nGuildWarPoint=nGuildWarPoint;};
	inline int		GetGuildWarPoint() { return m_nGuildWarPoint;};
	inline void		AddGuildWarPoint(int nAddPoint) { m_nGuildWarPoint += nAddPoint;};
	int				CalcGuildWarTrialPoint();
	void			SendGuildWarPreWinGuild(bool bPreWin);

	void			ReqGetGuildWareInfo(bool bForceReq = false);

	void			UpdateWareSize(CDNUserBase* pUserObject, short wSize);

	void			SendGuildWareList(CDNUserSession * pSession, int nResult);
private:
	void			FindOpenWareHour();
	

	void			ModifyMembIntro(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket);
	void			ModifyMembRole(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket);
	void			ChangeGuildMaster(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket);
	void			ApplyGuildPoint(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket);

	void			CheckResetWareLimit(TGuildMember* pGuildMember);


private:
	CDNGuildWare*	m_GuildWare;
	int				m_nResetWareHour;
	bool			m_bGuildWarStats;		// Loading Guild War preliminary results..
	int				m_nGuildWarPoint;		// Preliminary points
	// Guild rank info (overall rank)
	SMyGuildWarMissionGuildRankingInfo m_sMyGuildWarRankingInfo;
	// Guild rank (rank by category)
	SMyGuildWarMissionGuildRankingInfo m_sMyGuildWarMissionGuildRankingInfo[GUILDWAR_RANKINGTYPE_MAX]; // Our guild rank
};