
#include "stdafx.h"
#include "DNPartyTask.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "DNSQLConnectionManager.h"
#include "Log.h"
#include "DNConnection.h"

#if defined( PRE_PARTY_DB )
extern TDBConfig g_Config;
#endif

#if defined( PRE_PARTY_DB )

CDNPartyTask::CDNPartyTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{

}

CDNPartyTask::~CDNPartyTask(void)
{
}

void CDNPartyTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLMembership *pMembershipDB = NULL;
	CDNSQLWorld *pWorldDB = NULL;
	int nRet = ERROR_DB;

	switch (nSubCmd)
	{
		case QUERY_DELPARTY_FORSERVER:
		{
			TQDelPartyForServer* pParty = reinterpret_cast<TQDelPartyForServer*>(pData);
			TADelPartyForServer Party;
			memset( &Party, 0, sizeof(Party) );

			Party.nRetCode = ERROR_DB;
			Party.iServerID = pParty->iServerID;
			
			if( pParty->cWorldSetID <= 0)
				break;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
			if( pWorldDB )
			{
				Party.nRetCode = pWorldDB->QueryDelPartyForServer( pParty );

				if( Party.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QUERY_DELPARTY_FORSERVER:%d] (Ret:%d) SID:%d\r\n", pParty->cWorldSetID, Party.nRetCode, pParty->iServerID );
			}
			else
				g_Log.Log(LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QUERY_DELPARTY_FORSERVER:%d] pWorldDB Error\r\n", pParty->cWorldSetID);

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
			break;
		}
		case QUERY_DELPARTY_FORGAMESERVER:
			{
				TQDelPartyForServer* pParty = reinterpret_cast<TQDelPartyForServer*>(pData);
				TADelPartyForServer Party;
				memset( &Party, 0, sizeof(Party) );

				Party.nRetCode = ERROR_DB;
				for (int i = 0; i < g_Config.nWorldDBCount; i++)
				{
					pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, g_Config.WorldDB[i].nWorldSetID );
					if( pWorldDB )
					{
						Party.nRetCode = pWorldDB->QueryDelPartyForServer( pParty );

						if( Party.nRetCode != ERROR_NONE )
							g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QUERY_DELPARTY_FORSERVER:%d] (Ret:%d) SID:%d\r\n", pParty->cWorldSetID, Party.nRetCode, pParty->iServerID );
					}
					else
					{
						continue;
					}

					if(	Party.nRetCode != ERROR_NONE )
					{
						Party.iServerID = i;
						m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
						break;
					}
				}

				m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
				break;
			}
		case QUERY_ADDPARTY:
#if defined( PRE_WORLDCOMBINE_PARTY )
		case QUERY_ADDWORLDPARTY:
#endif
		{
			TQAddParty* pParty = reinterpret_cast<TQAddParty*>(pData);
			TAAddParty Party;
			memset( &Party, 0, sizeof(Party) );

			Party.nAccountDBID = pParty->nAccountDBID;
			Party.Data = pParty->Data;
			Party.nRetCode		= ERROR_DB;
			Party.bCheat = pParty->bCheat;

			char cWorldID = pParty->cWorldSetID;
#if defined( PRE_WORLDCOMBINE_PARTY )
			if( Party::bIsWorldCombineParty(pParty->Data.PartyData.Type) )
			{
				if( g_Config.nCombineWorldDBID <= 0 )
				{
					m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
					break;
				}
				cWorldID = g_Config.nCombineWorldDBID;
				pParty->cWorldSetID = cWorldID;
			}
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, cWorldID );
			if( pWorldDB )
			{
				Party.nRetCode = pWorldDB->QueryAddParty( pParty, &Party );

				if( Party.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, cWorldID, pParty->nAccountDBID, pParty->Data.PartyData.biLeaderCharacterDBID, 0, L"[QueryAddParty:%d] (Ret:%d)\r\n", cWorldID, Party.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, cWorldID, pParty->nAccountDBID, pParty->Data.PartyData.biLeaderCharacterDBID, 0, L"[QueryAddParty:%d] pWorldDB Error\r\n", cWorldID );

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
			break;
		}
		case QUERY_ADDPARTYANDMEMBERGAME:
			{
				TQAddPartyAndMemberGame* pParty = reinterpret_cast<TQAddPartyAndMemberGame*>(pData);
				TAAddPartyAndMemberGame Party;
				memset( &Party, 0, sizeof(Party) );

				Party.nAccountDBID = pParty->nAccountDBID;
				Party.Data = pParty->Data;
				memcpy( Party.nKickedMemberList, pParty->nKickedMemberList, sizeof(Party.nKickedMemberList) );				
				Party.nRetCode		= ERROR_DB;

				char cWorldID = pParty->cWorldSetID;
#if defined( PRE_WORLDCOMBINE_PARTY )
				if( Party::bIsWorldCombineParty(pParty->Data.PartyData.Type) )
				{
					if( g_Config.nCombineWorldDBID <= 0 )
					{
						m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
						break;
					}
					cWorldID = g_Config.nCombineWorldDBID;
					pParty->cWorldSetID = cWorldID;
				}
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )

				pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, cWorldID );
				if( pWorldDB )
				{
					Party.nRetCode = pWorldDB->QueryAddPartyAndMemberGame( pParty, &Party );

					if( Party.nRetCode != ERROR_NONE )
						g_Log.Log( LogType::_ERROR, cWorldID, pParty->nAccountDBID, pParty->Data.PartyData.biLeaderCharacterDBID, 0, L"[QueryAddPartyAndMemberGame:%d] (Ret:%d)\r\n", cWorldID, Party.nRetCode);
				}
				else
					g_Log.Log(LogType::_ERROR, cWorldID, pParty->nAccountDBID, pParty->Data.PartyData.biLeaderCharacterDBID, 0, L"[QueryAddPartyAndMemberGame:%d] pWorldDB Error\r\n", cWorldID );

#if defined( PRE_WORLDCOMBINE_PARTY )
				if( Party.nRetCode == ERROR_NONE && Party::bIsWorldCombineParty(pParty->Data.PartyData.Type) == false )
#else
				if( Party.nRetCode == ERROR_NONE )
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
				{
					// Add party member
					TQJoinParty Member;
					memset( &Member, 0, sizeof(TQJoinParty) );
					Member.PartyID = Party.Data.PartyData.PartyID;
					Member.iMaxUserCount = pParty->Data.PartyData.nPartyMaxCount;
					for(int i=0;i<pParty->Data.PartyData.iCurMemberCount;i++)
					{
						Member.biCharacterDBID = pParty->Member[i].biCharacterDBID;
#if defined( PRE_ADD_NEWCOMEBACK )
						Member.bCheckComeBackParty = pParty->Member[i].bCheckComebackAppellation;
#endif
						pWorldDB->QueryJoinParty( &Member );
					}
				}				
				m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
				break;
			}
		case QUERY_ADDPARTYANDMEMBERVILLAGE:
			{
				TQAddPartyAndMemberVillage* pParty = reinterpret_cast<TQAddPartyAndMemberVillage*>(pData);
				TAAddPartyAndMemberVillage Party;
				memset( &Party, 0, sizeof(Party) );

				Party.nAccountDBID = pParty->nAccountDBID;
				Party.Data = pParty->Data;				
				memcpy( Party.nKickedMemberList, pParty->nKickedMemberList, sizeof(Party.nKickedMemberList) );
				for(int i=0;i<pParty->Data.PartyData.iCurMemberCount;i++)
					Party.MemberInfo[i] = pParty->MemberInfo[i];
				Party.nVoiceChannelID = pParty->nVoiceChannelID;
				Party.nRetCode		= ERROR_DB;

				pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
				if( pWorldDB )
				{
					Party.nRetCode = pWorldDB->QueryAddPartyAndMemberVillage( pParty, &Party );

					if( Party.nRetCode != ERROR_NONE )
						g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->Data.PartyData.biLeaderCharacterDBID, 0, L"[QueryAddPartyAndMemberVillage:%d] (Ret:%d)\r\n", pParty->cWorldSetID, Party.nRetCode);
				}
				else
					g_Log.Log(LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->Data.PartyData.biLeaderCharacterDBID, 0, L"[QueryAddPartyAndMemberVillage:%d] pWorldDB Error\r\n", pParty->cWorldSetID);

				if( Party.nRetCode == ERROR_NONE )
				{
					// Add party member
					TQJoinParty Member;
					memset( &Member, 0, sizeof(TQJoinParty) );
					Member.PartyID = Party.Data.PartyData.PartyID;
					Member.iMaxUserCount = pParty->Data.PartyData.nPartyMaxCount;
					for(int i=0;i<pParty->Data.PartyData.iCurMemberCount;i++)
					{
						Member.biCharacterDBID = pParty->MemberInfo[i].biCharacterDBID;
#if defined( PRE_ADD_NEWCOMEBACK )
						Member.bCheckComeBackParty = pParty->MemberInfo[i].bCheckComebackAppellation;
#endif
						pWorldDB->QueryJoinParty( &Member );
					}
				}
				
				m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
				break;
			}
		case QUERY_DELPARTY:
		{
			TQDelParty* pParty = reinterpret_cast<TQDelParty*>(pData);
			TADelParty Party;
			memset( &Party, 0, sizeof(Party) );

			Party.nAccountDBID = pParty->nAccountDBID;
			Party.nRetCode = ERROR_DB;
			Party.PartyID = pParty->PartyID;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
			if( pWorldDB )
			{
				Party.nRetCode = pWorldDB->QueryDelParty( pParty );
				if( Party.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QueryDelParty:%d] (Ret:%d) PartyID=%I64d\r\n", pParty->cWorldSetID, Party.nRetCode, pParty->PartyID );
			}
			else
				g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QueryDelParty:%d] pWorldDB Error\r\n", pParty->cWorldSetID);

			if( Party.nRetCode == ERROR_NONE )
			{
				m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
			}
#if defined( PRE_WORLDCOMBINE_PARTY )
			else
			{
				// On failure, check the unified DB once more.
				pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, g_Config.nCombineWorldDBID );
				if( pWorldDB )
				{
					Party.nRetCode = pWorldDB->QueryDelParty( pParty );
					if( Party.nRetCode != ERROR_NONE )
						g_Log.Log( LogType::_ERROR, g_Config.nCombineWorldDBID, pParty->nAccountDBID, 0, 0, L"[QueryDelParty:%d] (Ret:%d) PartyID=%I64d\r\n", g_Config.nCombineWorldDBID, Party.nRetCode, pParty->PartyID );
				}
				else
					g_Log.Log( LogType::_ERROR, g_Config.nCombineWorldDBID, pParty->nAccountDBID, 0, 0, L"[QueryDelParty:%d] pWorldDB Error\r\n", g_Config.nCombineWorldDBID );

				if( Party.nRetCode == ERROR_NONE )
				{
					m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
				}
			}
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
			break;
		}
		case QUERY_JOINPARTY:
		{
			TQJoinParty* pParty = reinterpret_cast<TQJoinParty*>(pData);
			TAJoinParty Party;
			memset( &Party, 0, sizeof(Party) );

			Party.nAccountDBID = pParty->nAccountDBID;
			Party.nRetCode = ERROR_DB;
			Party.PartyID = pParty->PartyID;
			Party.biCharacterDBID = pParty->biCharacterDBID;
			Party.nSessionID = pParty->nSessionID;
			Party.cThreadID = pParty->cThreadID;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
			if( pWorldDB )
			{
				Party.nRetCode = pWorldDB->QueryJoinParty( pParty );
				switch( Party.nRetCode )
				{
					case ERROR_NONE:
					case 103322: // The party does not exist.
					case 103323: // Already in a party.
					case 103330: // Exceeded maximum number of party members.
						break;

					default:
						g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->biCharacterDBID, 0, L"[QueryJoinParty:%d] (Ret:%d) PartyID=%I64d\r\n", pParty->cWorldSetID, Party.nRetCode, pParty->PartyID );
						break;
				}
			}
			else
				g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->biCharacterDBID, 0, L"[QueryJoinParty:%d] pWorldDB Error\r\n", pParty->cWorldSetID);

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
			break;
		}
		case QUERY_OUTPARTY:
		{
			TQOutParty* pParty = reinterpret_cast<TQOutParty*>(pData);
			TAOutParty Party;
			memset( &Party, 0, sizeof(Party) );

			Party.nRetCode = ERROR_DB;
			Party.nAccountDBID	= pParty->nAccountDBID;
			Party.PartyID = pParty->PartyID;
			Party.biCharacterDBID =pParty->biCharacterDBID;
			Party.nSessionID = pParty->nSessionID;
			Party.Type = pParty->Type;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
			if( pWorldDB )
			{
				Party.nRetCode = pWorldDB->QueryOutParty( pParty, &Party );
				if( Party.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->biCharacterDBID, 0, L"[QueryOutParty:%d] (Ret:%d) PartyID=%I64d\r\n", pParty->cWorldSetID, Party.nRetCode, pParty->PartyID );
			}
			else
				g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->biCharacterDBID, 0, L"[QueryOutParty:%d] pWorldDB Error\r\n", pParty->cWorldSetID);

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
			break;
		}
		case QUERY_GETLISTPARTY:
		{
			TQGetListParty* pParty = reinterpret_cast<TQGetListParty*>(pData);
			TAGetListParty Party;
			memset( &Party, 0, sizeof(Party) );

			std::vector<Party::Data> vData;

			Party.nRetCode = ERROR_DB;
			Party.bIsStart = true;
			
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
			if( pWorldDB )
			{
				Party.nRetCode = pWorldDB->QueryGetListParty( pParty, vData );
				if( Party.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QueryGetListParty:%d] (Ret:%d)\r\n", pParty->cWorldSetID, Party.nRetCode );

#if defined( PRE_WORLDCOMBINE_PARTY )
				if( g_Config.nCombineWorldDBID>0 && pParty->cWorldSetID != g_Config.nCombineWorldDBID )
				{
					pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, g_Config.nCombineWorldDBID );
					if( pWorldDB )
					{
						pWorldDB->QueryGetListParty( g_Config.nCombineWorldDBID, _WORLDCOMBINE_PARTY, vData );
					}
					else
					{
						g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QueryGetListParty:%d] WorldCombineWorldDB Error\r\n", g_Config.nCombineWorldDBID );
					}
				}
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
			}
			else
				g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QueryGetListParty:%d] pWorldDB Error\r\n", pParty->cWorldSetID);

			if( Party.nRetCode == ERROR_NONE && vData.size() )
			{
				size_t TotalCount = 0;
				do
				{
					Party.DataArr[Party.nCount++] = vData[TotalCount++];
					if( Party.nCount == _countof(Party.DataArr) )
					{
						m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );

						Party.nCount = 0;
						Party.bIsStart = false;
					}
				}while( TotalCount < vData.size() );

				// Remaining
				if( Party.nCount > 0 )
				{
					int iSize = static_cast<int>(sizeof(Party)-sizeof(Party.DataArr)+(Party.nCount*sizeof(Party.DataArr[0])));
					m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), iSize );
				}
			}
			else
			{
				int iSize = static_cast<int>(sizeof(Party)-sizeof(Party.DataArr));
				m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), iSize );
			}
			
			break;
		}
		case QUERY_MODPARTY:
		{
			TQModParty* pParty = reinterpret_cast<TQModParty*>(pData);
			TAModParty Party;
			memset( &Party, 0, sizeof(Party) );
			Party.PartyData = pParty->PartyData;		
			Party.iRoomID = pParty->iRoomID;
			Party.nRetCode = ERROR_DB;		

			char cWorldID = pParty->cWorldSetID;
#if defined( PRE_WORLDCOMBINE_PARTY )
			if( Party::bIsWorldCombineParty(pParty->PartyData.Type) )
			{
				if( g_Config.nCombineWorldDBID <= 0 )
				{
					m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
					break;
				}
				cWorldID = g_Config.nCombineWorldDBID;
			}
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, cWorldID );
			if( pWorldDB )
			{
				Party.nRetCode = pWorldDB->QueryModParty( pParty );
				if( Party.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, cWorldID, pParty->nAccountDBID, 0, 0, L"[QueryModParty:%d] (Ret:%d)\r\n", cWorldID, Party.nRetCode );
			}
			else
				g_Log.Log( LogType::_ERROR, cWorldID, pParty->nAccountDBID, 0, 0, L"[QueryModParty:%d] pWorldDB Error\r\n", cWorldID );

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
			break;
		}
		case QUERY_MODPARTYLEADER:
		{
			TQModPartyLeader* pParty = reinterpret_cast<TQModPartyLeader*>(pData);
			
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
			if( pWorldDB )
			{
				int iRet = pWorldDB->QueryModPartyLeader( pParty );
				if( iRet != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->biCharacterDBID, 0, L"[QueryModPartyLeader:%d] (Ret:%d) PartyID:%I64d\r\n", pParty->cWorldSetID, iRet, pParty->PartyID );
			}
			else
				g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->biCharacterDBID, 0, L"[QueryModPartyLeader:%d] pWorldDB Error\r\n", pParty->cWorldSetID);
			break;
		}
		case QUERY_GETPARTY_JOINMEMBERS:
		{
			TQGetListPartyMember* pParty = reinterpret_cast<TQGetListPartyMember*>(pData);
			
			TAGetListPartyMember Party;
			memset( &Party, 0, sizeof(Party) );

			Party.nRetCode = ERROR_DB;
			Party.nAccountDBID = pParty->nAccountDBID;
			Party.PartyID = pParty->PartyID;

			std::vector<Party::MemberData> vData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
			if( pWorldDB )
			{
				Party.nRetCode = pWorldDB->QueryGetListPartyMember( pParty, &Party, vData );
				if( Party.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QueryGetListPartyMember:%d] (Ret:%d) PartyID:%I64d\r\n", pParty->cWorldSetID, Party.nRetCode, pParty->PartyID );
			}
			else
				g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QueryGetListPartyMember:%d] pWorldDB Error\r\n", pParty->cWorldSetID);

			if( Party.nRetCode == ERROR_NONE )
			{
				for( UINT i=0 ; i<vData.size() ; ++i )
				{
					Party.MemberData[Party.nCount++] = vData[i];
					
					if( Party.nCount == _countof(Party.MemberData) )
						break;
				}
			}

			int iSize = sizeof(Party)-sizeof(Party.MemberData)+(Party.nCount*sizeof(Party.MemberData[0]));

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );
			break;
		}
#if defined( PRE_WORLDCOMBINE_PARTY )
		case QUERY_GETLISTWORLDPARTY:
			{
				// This is called only on the reference DB.
				TQGetListParty* pParty = reinterpret_cast<TQGetListParty*>(pData);
				TAGetListParty Party;
				memset( &Party, 0, sizeof(Party) );

				std::vector<Party::Data> vData;

				Party.nRetCode = ERROR_DB;
				Party.bIsStart = true;

				pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
				if( pWorldDB )
				{				
					if( g_Config.nCombineWorldDBID>0 && pParty->cWorldSetID == g_Config.nCombineWorldDBID )
					{
						pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, g_Config.nCombineWorldDBID );
						if( pWorldDB )
						{
							Party.nRetCode = pWorldDB->QueryGetListParty( g_Config.nCombineWorldDBID, _WORLDCOMBINE_PARTY, vData );
						}
						else
						{
							g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QueryGetListParty:%d] WorldCombineWorldDB Error\r\n", g_Config.nCombineWorldDBID );
						}
					}
				}
				else
					g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, 0, 0, L"[QueryGetListParty:%d] pWorldDB Error\r\n", pParty->cWorldSetID);

				if( Party.nRetCode == ERROR_NONE && vData.size() )
				{
					size_t TotalCount = 0;
					do
					{
						Party.DataArr[Party.nCount++] = vData[TotalCount++];
						if( Party.nCount == _countof(Party.DataArr) )
						{
							m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), sizeof(Party) );

							Party.nCount = 0;
							Party.bIsStart = false;
						}
					}while( TotalCount < vData.size() );

					// Remaining
					if( Party.nCount > 0 )
					{
						int iSize = static_cast<int>(sizeof(Party)-sizeof(Party.DataArr)+(Party.nCount*sizeof(Party.DataArr[0])));
						m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), iSize );
					}
				}
				else
				{
					int iSize = static_cast<int>(sizeof(Party)-sizeof(Party.DataArr));
					m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Party), iSize );
				}
			}
			break;
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
		case QUERY_MOD_COMEBACKINFO:
			{
				TQModPartyMemberComeback* pParty = reinterpret_cast<TQModPartyMemberComeback*>(pData);

				pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pParty->cWorldSetID );
				if( pWorldDB )
				{
					int iRet = pWorldDB->QueryModPartyMemberComeback( pParty );
					if( iRet != ERROR_NONE )
						g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->biCharacterDBID, 0, L"[QUERY_MOD_COMEBACKINFO:%d] (Ret:%d) PartyID:%I64d\r\n", pParty->cWorldSetID, iRet, pParty->PartyID );
				}
				else
					g_Log.Log( LogType::_ERROR, pParty->cWorldSetID, pParty->nAccountDBID, pParty->biCharacterDBID, 0, L"[QUERY_MOD_COMEBACKINFO:%d] pWorldDB Error\r\n", pParty->cWorldSetID);
				break;
			}
			break;
#endif
	}
}

#endif // #if defined( PRE_PARTY_DB )