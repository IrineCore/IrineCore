
#pragma once

namespace LadderSystem
{
	class CRoom;
	class CMatchingSystem;

	class CRoomRepository
	{
		public:

			CRoomRepository();
			~CRoomRepository();

			void	Process( DWORD dwCurTick );

			//
			int		Create( CDNUserSession* pSession, MatchType::eCode MatchType );
			int		Join( CDNUserSession* pSession, CRoom* pRoom );
			int		Leave( CDNUserSession* pSession, Reason::eCode Reason );
			int		Matching( CDNUserSession* pSession, bool bIsCancel );
			void	DisconnectUser( INT64 biCharDBID, WCHAR* pwszCharName );
			void	SendPlayingRoomList( CDNUserSession* pSession, MatchType::eCode MatchType, UINT uiPage=1 );
			void	InsertPlayingList( CRoom* pLadderRoom );
			void	DeletePlayingList( CRoom* pLadderRoom );

			//
			bool				bIsExist( const WCHAR* pwszCharName );
			CMatchingSystem*	GetMatchingSystemPtr(){ return m_pMatchingSystem; }
			CRoom*				GetRoomPtr( const WCHAR* pwszCharName );
			CRoom*				GetRoomPtr( INT64 biRoomIndex );
			int					GetAvgMatchingTimeSec( MatchType::eCode MatchType );
			const std::map<INT64,CRoom*>& GetRoomList(){ return m_mRoomIndexRoomPtr; }

		private:

			void			_OnDestroyRoom( CRoom* pLadderRoom );
			void			_QueryCUCount( MatchType::eCode MatchType );

		private:

			INT64	_GenerateRoomIndex(){ return ++m_bIIncRoomIndex; }

			DWORD										m_dwCUCountUpdateTick;
			// 
			INT64										m_bIIncRoomIndex;
			CMatchingSystem*							m_pMatchingSystem;
			std::map<std::wstring,INT64>				m_mCharNameRoomIndex;
			std::map<INT64,CRoom*>						m_mRoomIndexRoomPtr;
			std::map<MatchType::eCode,std::list<INT64>>	m_mMatchTypePlayingRoomIndex;
	};
};
