
#pragma once
#include <tuple>

class CDNUserSession;

namespace MasterSystem
{
	const DWORD CACHETICK		= (1000*30);	// 30 seconds
	const DWORD CACHECLEARTICK	= (1000*60*10);	// 10 minutes

	class CCacheRepository:public CSingleton<CCacheRepository>
	{
		public:

			CCacheRepository():m_dwCacheTick(0),m_uiCacheClearCount(0)
			{
			}

			void GetMasterList( CDNUserSession* pSession, UINT uiPage, BYTE cJob, BYTE cGender );
			void SetMasterList( CDNUserSession* pSession, TAGetPageMasterCharacter* pData );

			void GetMasterCharacter( CDNUserSession* pSession, INT64 biCharacterDBID );
			void SetMasterCharacter( CDNUserSession* pSession, TAGetMasterCharacterType1* pData );

			void GetMyMasterInfo( CDNUserSession* pSession, INT64 biPupilCharacterDBID, INT64 biMasterCharacterDBID, bool bRefresh );
			void SetMyMasterInto( CDNUserSession* pSession, TAGetMasterCharacterType2* pData );

			const TPupilInfo*	GetPupilInfo( INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID );
			void				GetPupilList( CDNUserSession* pSession, INT64 biCharacterDBID, bool bRefresh );
			void				SetPupilList( CDNUserSession* pSession, TAGetListPupil* pData );

			void GetMasterClassmateList( CDNUserSession* pSession, INT64 biCharacterDBID, bool bRefresh );
			void SetMasterClassmateList( CDNUserSession* pSession, TAGetListMyMasterAndClassmate* pData );

			void GetClassmateInfo( CDNUserSession* pSession, INT64 biClassmateCharacterDBID, bool bRefresh );
			void SetClassmateInfo( CDNUserSession* pSession, TAGetMyClassmate* pData );

			TCommunityLocation	GetLocationInfo( INT64 biCharacterDBID );

			void SetRegisterMaster( CDNUserSession* pSession, TAAddMasterCharacter* pData );
			void SetRegisterMaster( CDNUserSession* pSession, TADelMasterCharacter* pData );

			void ClearCache();

		private:

			CSyncLock m_Sync;

			DWORD	m_dwCacheTick;
			UINT	m_uiCacheClearCount;

			//#####################################################################################
			// [0]MasterList
			//#####################################################################################
			// Key:<Page,Job,Gender> : Value:<Tick,Data>
			//#####################################################################################

			typedef std::map< std::tuple<UINT,BYTE,BYTE>, std::pair<DWORD,std::vector<TMasterInfo>> >	MMasterList;
			typedef MMasterList::iterator																	MMasterListItor;
			
			MMasterList m_mMasterList;

			//#####################################################################################
			// [1]PupilList
			//#####################################################################################
			// Key:MasterCharacterDBID : value:<Tick,Data>
			//#####################################################################################

			typedef std::map< INT64, std::pair<DWORD,std::vector<TPupilInfo>> >	MPupilList;
			typedef MPupilList::iterator										MPupilListItor;

			MPupilList m_mPupilList;

			//#####################################################################################
			// [2]Master&Classmate List
			//#####################################################################################
			// Key:PupilCharacterDBID : value:<Tick,Data>
			//#####################################################################################

			typedef std::map< INT64, std::pair<DWORD,std::vector<TMasterAndClassmateInfo>> >	MMasterClassmateList;
			typedef MMasterClassmateList::iterator												MMasterClassmateListItor;

			MMasterClassmateList m_mMasterClassmateList;

			//#####################################################################################
			// [3]ClassmateInfo
			//#####################################################################################
			// Key:CharacterDBID : value:<Tick,Data>
			//#####################################################################################

			typedef std::map< INT64, std::pair<DWORD,TClassmateInfo> >	MClassmateInfoList;
			typedef MClassmateInfoList::iterator						MClassmateInfoListItor;

			MClassmateInfoList m_mClassmateList;

			//#####################################################################################
			// [4]MyMasterInfo
			//#####################################################################################
			// Key:<PupilCharacterDBID,MasterCharacterDBID> : value:<Tick,Data>
			//#####################################################################################

			typedef std::map< std::pair<INT64,INT64>, std::pair<DWORD,TMyMasterInfo> >	MMyMasterInfoList;
			typedef MMyMasterInfoList::iterator											MMyMasterInfoListItor;

			MMyMasterInfoList m_mMyMasterList;
	};
}
