
#pragma once

#include "DNUserSession.h"

namespace LadderSystem
{
	// Since nothing differs by MatchType other than player count, do not create separate LadderRoom classes per MatchType.
	class CRoom:public TBoostMemoryPool<CRoom>
	{
		public:

			struct SUserInfo
			{
				UINT				uiAccountDBID;
				INT64				biCharDBID;
				WCHAR				wszCharName[NAMELENMAX];
				MatchType::eCode	MatchType;
				BYTE				cJob;
				int					iGradePoint;
				SUserInfo( CDNUserSession* pSession, MatchType::eCode Type )
				{
					uiAccountDBID	= pSession->GetAccountDBID();
					biCharDBID		= pSession->GetCharacterDBID();
					cJob			= pSession->GetUserJob();
					MatchType		= Type;
					iGradePoint		= pSession->GetPvPLadderScoreInfoPtr()->GetGradePoint( MatchType );
					_wcscpy( wszCharName, _countof(wszCharName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()) );
				}
			};

			CRoom( const INT64 biIndex, const MatchType::eCode Type, CDNUserSession* pSession );

			void Process( DWORD dwCurTick );
			bool JoinUser( CDNUserSession* pJoinSession );
			void RefreshUserInfo( CDNUserSession* pSession );
			void UpdateUserInfo( CDNUserSession* pSession );
			bool OutUser( INT64 biCharDBID, const WCHAR* pwszCharName, Reason::eCode Type );
			void AdjustNewLeader();
			void ChangeRoomState( RoomState::eCode State, RoomStateReason::eCode Reason=RoomStateReason::ERROR_NONE );
			void SendPvPGameModeTableID( int iTableID, bool bSet=true );
			void SendAllRoomID( USHORT wGameID, int nRoomID );
			void SendAllReadyToGame( ULONG nIP, USHORT nPort, USHORT nTcpPort );
			void SendStartMsgCount();
			void SendMatchingAvgSec();
			void BroadCast( int iMainCmd, int iSubCmd, char* pData, int iLen );
			void ClearGameServerInfo();
			void SendChat( eChatType eType, int cLen, const WCHAR *pwszCharacterName, const WCHAR *pwszChatMsg, int nRet = ERROR_NONE );

			//
			INT64				GetRoomIndex(){ return m_biRoomIndex; }
			INT64				GetOpponentRoomIndex(){ return m_biOpponentRoomIndex; }
			bool				bIsEmpty(){ return m_vUserInfo.empty(); }
			bool				bIsValidOpponentRoom( CRoom* pLadderRoom, int CheckState=RoomState::None );
			bool				bIsValidUser( CDNUserSession* pSession );
			INT64				GetLeaderCharDBID(){ return m_biLeaderCharDBID; }
			MatchType::eCode	GetMatchType(){ return m_MatchType; }
			RoomState::eCode	GetRoomState(){ return m_RoomState; }
			DWORD				GetRoomStateElapsedTick(){ return timeGetTime()-m_dwRoomStateTick; }
			void				ClearOpponentRoomIndex(){ m_biOpponentRoomIndex=0; }
			void				SetOpponentRoomIndex( INT64 biIndex ){ m_biOpponentRoomIndex = biIndex; }
			int					GetStartMsgCount(){ return m_iStartMsgCount; }
			void				SetStartMsgCount( int iCount );
			bool				bIsValidUserInfo(){ return m_vUserInfo.size() == static_cast<size_t>(m_MatchType); }
			int					GetGameModeTableID(){ return m_iGameModeTableID; }
			int					GetAvgHiddenGradePoint(){ return m_iAvgHiddenGradePoint; }
			int					GetAvgGradePoint();
			bool				bIsMatchingReady(){ return (GetAvgHiddenGradePoint()>0); }
			void				GetMatchingSection( std::vector<int>& vData );
			const std::vector<SUserInfo>& GetUserInfo(){ return m_vUserInfo; }
			void				ToggleForceMatching(){ m_bForceMatching ^= 1; }
			bool				bIsForceMatching(){ return m_bForceMatching; }
			bool				bIsFullUser();
			bool				bIsAllConnectUser();
			const WCHAR*		GetCharName( INT64 biCharDBID );

			void				CopyGameServerInfo( int& iServerID, int& iRoomID ){ iServerID=m_iGameServerID; iRoomID=m_iGameRoomID; }

			void				AddInviteUser( const WCHAR* pwszCharName );
			void				DelInviteUser( const WCHAR* pwszCharName );
			bool				bIsInviteUser( const WCHAR* pwszCharName );
			bool				bIsInviting();

		private:

			const INT64				m_biRoomIndex;
			INT64					m_biOpponentRoomIndex;	// Matched other ladder room index
			const MatchType::eCode	m_MatchType;
			RoomState::eCode		m_RoomState;
			DWORD					m_dwRoomStateTick;
			DWORD					m_dwUpdateProcessTick;
			std::vector<SUserInfo>	m_vUserInfo;
			INT64					m_biLeaderCharDBID;
			int						m_iStartMsgCount;
			// GameServer info
			int						m_iGameServerID;
			int						m_iGameRoomID;
			int						m_iGameModeTableID;
			// Match info
			int						m_iAvgHiddenGradePoint;
			// ForCheat
			bool					m_bForceMatching;
			// Invite-related
			std::map<std::wstring,DWORD>	m_mInviteUser;
	};
};
