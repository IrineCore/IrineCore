
#pragma once
#include <tuple>

class CDNUserSession;

namespace GuildRecruitSystem
{
	const DWORD CACHETICK		= (1000*30);	// 30 seconds
	const DWORD CACHECLEARTICK	= (1000*60*10);	// 10 minutes
		 
	class CCacheRepository:public CSingleton<CCacheRepository>
	{
	public:

		CCacheRepository():m_dwCacheTick(0),m_uiCacheClearCount(0)
		{
		}

#if defined( PRE_ADD_GUILD_EASYSYSTEM )
		void GetGuildRecruitList( CDNUserSession* pSession, UINT uiPage, BYTE cJob, BYTE cLevel, BYTE cPurposeCode, WCHAR* wszGuildName, BYTE cSortType );
#else
		void GetGuildRecruitList( CDNUserSession* pSession, UINT uiPage, BYTE cJob, BYTE cLevel );
#endif
		void SetGuildRecruitList( CDNUserSession* pSession, TAGetGuildRecruit* pData );

		void GetGuildRecruitCharacter( CDNUserSession* pSession, TGuildUID GuildUID );
		void SetGuildRecruitCharacter( CDNUserSession* pSession, TAGetGuildRecruitCharacter* pData );
		void DelGuildRecruitCharacter( TGuildUID GuildUID );

		void GetMyGuildRecruit( CDNUserSession* pSession );
		void SetMyGuildRecruit( CDNUserSession* pSession, TAGetMyGuildRecruit* pData );		
		void DelMyGuildRecruit( CDNUserSession* pSession );
		
		void ClearCache();

	private:

		CSyncLock m_Sync;

		DWORD	m_dwCacheTick;
		UINT	m_uiCacheClearCount;

		//#####################################################################################
		// [0]MGuildRecruitList Registered guild info on guild recruitment board
		//#####################################################################################
		// Key:<Page,Job,Level> : Value:<Tick,Data>
		//#####################################################################################

#if defined( PRE_ADD_GUILD_EASYSYSTEM )
		typedef std::map< std::tuple<UINT,BYTE,int,BYTE,BYTE,WCHAR>, std::pair<DWORD,std::vector<TGuildRecruitInfo>> >	MGuildRecruitList;
#else
		typedef std::map< std::tuple<UINT,BYTE,int>, std::pair<DWORD,std::vector<TGuildRecruitInfo>> >	MGuildRecruitList;
#endif
		typedef MGuildRecruitList::iterator																MMGuildRecruitListItor;

		MGuildRecruitList m_mGuildRecruitList;		

		//#####################################################################################
		// [1]MGuildRecruitCharacterList List of users who applied via guild recruitment board (guild-centric)
		//#####################################################################################
		// Key: Guild UID, value: <Tick, Data>
		//#####################################################################################

		typedef std::map< TGuildUID, std::pair<DWORD,std::vector<TGuildRecruitCharacter>> >	MGuildRecruitCharacterList;
		typedef MGuildRecruitCharacterList::iterator										MGuildRecruitCharacterListItor;

		MGuildRecruitCharacterList m_mGuildRecruitCharacterList;		

		//#####################################################################################
		// [2]MMyGuildRecruitList List of guilds applied to via guild recruitment board (user-centric)
		//#####################################################################################
		// Key:CharacterDBID : value:<Tick,Data>
		//#####################################################################################

		typedef std::map< INT64, std::pair<DWORD,std::vector<TGuildRecruitInfo>> >		MMyGuildRecruitList;
		typedef MMyGuildRecruitList::iterator											MMyGuildRecruitListItor;

		MMyGuildRecruitList m_mMyGuildRecruitList;		
	};
}