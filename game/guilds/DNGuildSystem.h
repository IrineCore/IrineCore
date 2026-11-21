#pragma once

#include "DNGuildBase.h"
#include "BitSetEx1.hpp"

typedef std::vector<class CDNGuildBase*, boost::pool_allocator<class CDNGuildBase*> > TVecGuilds;

class CDNGuildSystem
{
public:
	CDNGuildSystem ();
	~CDNGuildSystem ();

	DWORD				Open (DWORD dwSize = GUILDMNGRSIZE_DEF);
	void				Close ();
	bool				IsOpen () const {return m_bOpen;}
	DWORD				Release (CDNGuildBase* pGuild);
	CDNGuildBase*		At (const TGuildUID pGuildUID);
	const CDNGuildBase*	At (const TGuildUID pGuildUID) const;
	CDNGuildBase*		Get (const TGuild* pInfo);

	char		GetDefaultSize() const { return  m_DefaultSize; }

	// Sync
	CSyncLock*			GetLock () {return (&m_Lock);}

	// Copy List
	void				GetMainList (TVecGuilds& pList);
	void				GetCurrList (std::vector<TGuildUID, boost::pool_allocator<TGuildUID>>& pList);

	// Dismiss
	bool				AddDismiss (const TGuildUID pGuildUID);
	bool				IsDismissExist (const TGuildUID pGuildUID);

	// UPDATE
	void				DoUpdate (DWORD CurTick);

	// RECV FROM MA
	void				OnRecvMaDismissGuild (const MADismissGuild *pPacket);
	void				OnRecvMaAddGuildMember (const MAAddGuildMember *pPacket);
	void				OnRecvMaDelGuildMember (const MADelGuildMember *pPacket);
	void				OnRecvMaChangeGuildInfo (const MAChangeGuildInfo *pPacket);
	void				OnRecvMaChangeGuildMemberInfo (const MAChangeGuildMemberInfo *pPacket);
	void				OnRecvMaGuildChat (const MAGuildChat *pPacket);
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	void OnRecvMaDoorsGuildChat(const MADoorsGuildChat *pPacket);
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	void				OnRecvMaMemberLevelUp (const MAGuildMemberLevelUp *pPacket);
	void				OnRecvMaGuildChangeName(const MAGuildChangeName *pPacket);
	void				OnRecvMaGuildChangeMark(const MAGuildChangeMark *pPacket);
	void				OnRecvMaGuildUpdateExp( const MAUpdateGuildExp* pPacket );
	void				OnRecvMaEnrollGuildWar(const MAEnrollGuildWar *pPacket);

	// RECV FROM CS
	void				OnRecvCsGetGuildInfo (CDNUserSession * pSession, const CSGetGuildInfo *pPacket);
	void				OnRecvCsGetGuildWareHistory (CDNUserSession * pSession, CSGetGuildWareHistory *pPacket);
	void				OnRecvCsCloseGuildWare (CDNUserSession * pSession);
	
	// FROM LUA API
	bool				OnRecvApiOpenGuildWare (CDNUserSession * pSession);

	// AUTH
	DWORD				GetAuthBase (eGuildRoleType pGuildRoleType) const;
	DWORD				GetAuthPrmt (eGuildRoleType pGuildRoleType) const;
	DWORD				GetAuthRstt (eGuildRoleType pGuildRoleType) const;
	const CBitSetEx1<GUILDAUTH_TYPE_CNT, WORD>& RefAuthBase (eGuildRoleType pGuildRoleType) const;
	const CBitSetEx1<GUILDAUTH_TYPE_CNT, WORD>& RefAuthPrmt (eGuildRoleType pGuildRoleType) const;
	const CBitSetEx1<GUILDAUTH_TYPE_CNT, WORD>& RefAuthRstt (eGuildRoleType pGuildRoleType) const;
	void				SetAuthBase (int pGuildRoleAuth[GUILDROLE_TYPE_CNT]) const;	// Set guild rank (initial)
	void				SetAuthMust (int pGuildRoleAuth[GUILDROLE_TYPE_CNT]) const;	// Set guild rank (allowed/denied)
	void				SetRoleLimitation (TGuild* pInfo);

	// External Interface
	void				UpdateGuildResource (TAGetGuildInfo *pPacket);
	void				UpdateGuildMember(TAGetGuildMember *pPacket);
	void				OpenGuildWare (const TAGetGuildWareInfo *pPacket);

	int					GetResetWareHour () {return m_nResetWareHour;}
	void				MaChangePoint (CDNGuildBase* pGuild, const MAChangeGuildInfo *pPacket);

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
	void				MaModifyMembJob (CDNGuildBase* pGuild, INT64 nCharacterDBID, int nChangeJob);
#endif

	void				UpdateGuildRewardItem( TAGetGuildRewardItem *pGuildRewardItem );	
	int					BuyGuildRewardItem( CDNUserSession * pSession, int nItemID );

private:
	CDNGuildBase*		CreateGuildInstance ();

	// RECV
	bool				PreRecvProcess (int nManagedID, TGuildUID pGuildUID, CDNGuildBase*& pGuild, bool bCheckManagdID = true);

	// RECV FROM MASTER SERVER
	// Change guild info
	void				MaChangeNotification (CDNGuildBase* pGuild, const MAChangeGuildInfo *pPacket);
	void				MaChangeRoleAuth (CDNGuildBase* pGuild, const MAChangeGuildInfo *pPacket);
	void				MaChangeGuildWarFinalInfo(CDNGuildBase* pGuild, const MAChangeGuildInfo *pPacket);
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	void				MaChangeGuildHomePage(CDNGuildBase* pGuild, const MAChangeGuildInfo *pPacket);
#endif
	
	// Update guild member info
	void				MaModifyMembIntro (CDNGuildBase* pGuild, const MAChangeGuildMemberInfo* pPacket);
	void				MaModifyTypeRole (CDNGuildBase* pGuild, const MAChangeGuildMemberInfo* pPacket);
	void				MaDelegateGuildMaster (CDNGuildBase* pGuild, const MAChangeGuildMemberInfo* pPacket);
	void				MaChangeConnectState (CDNGuildBase* pGuild, const MAChangeGuildMemberInfo* pPacket);
	
	// Reset permissions
	void				InitAuth ();

#if defined(_DEBUG)
	bool				IsLock() const;
#endif

private:
	bool				m_bOpen;				// Whether initialized
	BYTE				m_DefaultSize;			// Default guild resource size
	
	// Guild resource pool
	std::queue <class CDNGuildBase*> m_Pool;					
	
	// Full guild resource list
	TVecGuilds			m_MainList;							
	std::map <TGuildUID, class CDNGuildBase*> m_CurrList;	// List of currently active guilds

	std::set <TGuildUID> m_DismList;	// Disbanded guild list - always maintained while server is running

	CSyncLock			m_Lock;					

	CBitSetEx1<GUILDAUTH_TYPE_CNT, WORD>		m_AuthBase[GUILDROLE_TYPE_CNT];		// Default authority by guild rank (initial)
	CBitSetEx1<GUILDAUTH_TYPE_CNT, WORD>		m_AuthPrmt[GUILDROLE_TYPE_CNT];		// Default authority by guild rank (allowed)
	CBitSetEx1<GUILDAUTH_TYPE_CNT, WORD>		m_AuthRstt[GUILDROLE_TYPE_CNT];		// Default authority by guild rank (denied)

	DWORD				m_TickDoUpdate;
	int					m_nResetWareHour;
};

extern CDNGuildSystem * g_pGuildManager;