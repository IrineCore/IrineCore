
#pragma once

namespace MasterSystem
{
	class CRewardSystem:public TBoostMemoryPool<CRewardSystem>
	{
		public:

			struct TCountInfo
			{
				TCountInfo()
				{
					Clear();
				}

				TCountInfo( int iMaster, int iPupil, int iClassmate )
				{
					iMasterCount	= iMaster;
					iPupilCount		= iPupil;
					iClassmateCount	= iClassmate;
				}

				bool bIsAllZero()
				{
					return (iMasterCount == 0 && iPupilCount == 0 && iClassmateCount == 0);
				}

				bool operator == ( const TCountInfo& rhs )
				{
					return (iMasterCount == rhs.iMasterCount && iPupilCount == rhs.iPupilCount && iClassmateCount == rhs.iClassmateCount);
				}

				bool operator != ( const TCountInfo& rhs )
				{
					return (iMasterCount != rhs.iMasterCount || iPupilCount != rhs.iPupilCount || iClassmateCount != rhs.iClassmateCount);
				}

				void Clear()
				{
					iMasterCount	= 0;
					iPupilCount		= 0;
					iClassmateCount	= 0;
				}

				int iMasterCount;
				int iPupilCount;
				int iClassmateCount;
			};

			CRewardSystem( CDNGameRoom* pGameRoom );

			void				Update( CDNUserSession* pSession, TAGetMasterSystemCountInfo* pPacket );
			void				RequestRefresh();
			void				RequestDungeonClear( CDNUserSession* pSession );
			void				GetCountInfo( CDNUserSession* pSession, TCountInfo& CountInfo );
			const TCountInfo*	GetCountInfo( CDNUserSession* pSession );
			int					GetMasterSystemAddExp( CDNUserSession* pSession, float fExp, bool bStageClear);
			void				UpdateFavor( CDNUserSession* pSession, INT64 biPupilCharacterDBID, int nAddFavor);						

			bool	bIsDurabilityReward( CDNUserSession* pSession );			// Receive durability compensation?
			bool	bIsFatigueReward( CDNUserSession* pSession );				// Receive fatigue reward?
			float	GetExpRewardRate( CDNUserSession* pSession );				// How much EXP reward?
			float	GetTreasureBoxRewardRate( CDNUserSession* pSession );		// How much reward from the reward box?

		private:

			CDNGameRoom* m_pGameRoom;
			std::map<INT64,TCountInfo> m_mCountRepository;
	};
}
