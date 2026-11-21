
#pragma once

#include <boost/tuple/tuple.hpp>

namespace LadderSystem
{
	const int MAX_GRADEPOINT_RANGE = 6;

	// Stats
	class CStats:public TBoostMemoryPool<CStats>
	{
		public:

			CStats( CDNUserSession* pSession, MatchType::eCode MatchType );

			bool	bIsInit(){ return m_bInit; }
			void	SetVSJobCode( int iJob ){ m_iVSJobCode=iJob; };
			BYTE	GetVSJobCode(){ return static_cast<BYTE>(m_iVSJobCode); }
			int		GetGradePoint(){ return m_iGradePoint; }
			int		GetHiddenGradePoint(){ return m_iHiddenGradePoint; }
			
			void	ConvertKillResult( std::vector<LadderKillResult>& vData );

			void	OnRecvLadderScore( const TAGetListPvPLadderScore* pPacket );
			void	OnDie( DnActorHandle hActor, DnActorHandle hHitter );
			void	OnKill( DnActorHandle hActor, DnActorHandle hDieActor );

			struct SKillDeathCount
			{
			public:
				int iKillCount;
				int iDeathCount;
				SKillDeathCount():iKillCount(0),iDeathCount(0){}
				SKillDeathCount( int KillCount, int DeathCount ):iKillCount(KillCount),iDeathCount(DeathCount){}
			};

		private:

			const INT64						m_biCharacterDBID;
			const MatchType::eCode			m_MatchType;
			bool							m_bInit;
			int								m_iGradePoint;			// Rating
			int								m_iHiddenGradePoint;	// Hidden grade point
			MatchResult::eCode				m_MatchResult;			// Match result
			std::map<int,SKillDeathCount>	m_mKillDeathCount;		// <Job/SKillDeathCount>
			int								m_iVSJobCode;			// Used only when MatchType::_1vs1
	};


	// Repository
	class CStatsRepository:public TBoostMemoryPool<CStatsRepository>
	{
		public:
		
			CStatsRepository( CDNGameRoom* pGameRoom );
			~CStatsRepository();
			bool bInitialize();
			bool QueryUpdateResult( CDNUserSession* pSession, UINT uiWinTeam, PvPCommon::QueryUpdatePvPDataType::eCode Type );

			void OnDie( DnActorHandle hActor, DnActorHandle hHitter );
			void OnRecvLadderScore( INT64 biCharacterDBID, int iTeam, const TAGetListPvPLadderScore* pPacket );
			void OnSetPlayState();

			bool	bIsValidLadderTeam( int iTeam );
			int		GetAddGradePoint( INT64 biCharacterDBID );
			int		GetResultGradePoint( INT64 biCharacterDBID );

			struct SResult
			{
				int iAddGradePoint;
				int iResultGradePoint;

				SResult():iAddGradePoint(0),iResultGradePoint(0){}
				SResult( int iAdd, int iResult ):iAddGradePoint(iAdd),iResultGradePoint(iResult){}
			};

		private:

			CDNGameRoom*					m_pGameRoom;
			MatchType::eCode				m_MatchType;
			std::map<INT64,CStats*>			m_mRepository;
			std::map<int,std::vector<int>>	m_mGradePointSum;
			int								m_iTeamGradePointAvg[PvPCommon::TeamIndex::Max];
			std::map<INT64,SResult>			m_mResult;

			//#############################################
			// For rating calculation
			//#############################################

			boost::tuple<float,float,float> S_Repository[MAX_GRADEPOINT_RANGE];
			boost::tuple<float,float,float> HiddenS_Repository[MAX_GRADEPOINT_RANGE];
			boost::tuple<float,float,float,float> K_Repository[MAX_GRADEPOINT_RANGE];
			boost::tuple<float,float,float,float> HiddenK_Repository[MAX_GRADEPOINT_RANGE];

	};


	// Factory
	class CStatsFactory
	{
		public:

			static CStatsRepository* CreateRepository( CDNGameRoom* pGameRoom );
	};
};
