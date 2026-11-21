
#include "stdafx.h"
#include "DNPrivateChatChannelTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLWorld.h"
#include "Log.h"
#include "Util.h"

#if defined (PRE_PRIVATECHAT_CHANNEL)

CDNPrivateChatChannelTask::CDNPrivateChatChannelTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{
}

CDNPrivateChatChannelTask::~CDNPrivateChatChannelTask()
{
}

void CDNPrivateChatChannelTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData) 
{
	switch (nSubCmd)
	{
	case QUERY_GET_PRIVATECHATCHANNEL:
		{
			TQPrivateChatChannelInfo* pChat = reinterpret_cast<TQPrivateChatChannelInfo*>(pData);

			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pChat->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_GET_PRIVATECHATCHANNEL:%d] pWorldDB not found\r\n", pChat->cWorldSetID);
				break;
			}

			TAPrivateChatChannelInfo Chat;
			memset(&Chat, 0, sizeof(Chat));		

			Chat.cWorldSetID = pChat->cWorldSetID;

			std::vector<TPrivateChatChannelInfo> vChannelList;
			Chat.nRetCode = pWorldDB->QueyrGetPrivateChatChannelInfo(pChat, vChannelList);
			if (Chat.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_GET_PRIVATECHATCHANNEL:%d] Query Error Ret:%d\r\n", pChat->cWorldSetID, Chat.nRetCode);
			
			for( std::vector<TPrivateChatChannelInfo>::iterator itor = vChannelList.begin(); itor != vChannelList.end();itor++)
			{
				Chat.tPrivateChatChannel[Chat.nCount] = *itor;
				Chat.nCount++;
				if(Chat.nCount >= PrivateChatChannel::Common::GetDBMaxChannel)
				{
					m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), sizeof(Chat));
					memset(&Chat, 0, sizeof(Chat));
					Chat.nCount = 0;
				}
			}
			if(Chat.nCount > 0)
			{
				int nSize = (sizeof(TAPrivateChatChannelInfo) - sizeof(Chat.tPrivateChatChannel)) + (Chat.nCount *sizeof(Chat.tPrivateChatChannel[0]));
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), nSize);
			}
			break;			
		}
		break;
	case QUERY_GET_PRIVATECHATCHANNELMEMBER:
		{
			TQPrivateChatChannelMember* pChat = reinterpret_cast<TQPrivateChatChannelMember*>(pData);

			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pChat->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_GET_PRIVATECHATCHANNELMEMBER:%d] pWorldDB not found\r\n", pChat->cWorldSetID);
				break;
			}

			TAPrivateChatChannelMember Chat;
			memset(&Chat, 0, sizeof(Chat));
			Chat.cWorldSetID = pChat->cWorldSetID;
			Chat.nPrivateChatChannelID = pChat->nPrivateChatChannelID;
			Chat.nAccountDBID = pChat->nAccountDBID;

			Chat.nRetCode = pWorldDB->QueyrGetPrivateChatChannelMember(pChat, &Chat);
			if (Chat.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_GET_PRIVATECHATCHANNELMEMBER:%d] Query Error Ret:%d\r\n", pChat->cWorldSetID, Chat.nRetCode);
						
			int nSize = (sizeof(TAPrivateChatChannelMember) - sizeof(Chat.Member)) + (Chat.nCount *sizeof(Chat.Member[0]));
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), nSize);
			
			break;			
		}
		break;
	case QUERY_CREATE_PRIVATECHATCHANNEL:
		{
			TQAddPrivateChatChannel* pChat = reinterpret_cast<TQAddPrivateChatChannel*>(pData);

			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pChat->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_CREATE_PRIVATECHATCHANNEL:%d] pWorldDB not found\r\n", pChat->cWorldSetID);
				break;
			}
			
			TAAddPrivateChatChannel Chat;
			memset(&Chat, 0, sizeof(Chat));

			Chat.cWorldSetID = pChat->cWorldSetID;
			Chat.nAccountDBID = pChat->nAccountDBID;
			Chat.tJoinDate = pChat->tJoinDate;
			Chat.tPrivateChatChannel = pChat->tPrivateChatChannel;
			_wcscpy(Chat.wszCharacterName, _countof(Chat.wszCharacterName), pChat->wszCharacterName, (int)wcslen(pChat->wszCharacterName) );	

			Chat.nRetCode = pWorldDB->QueyrAddPrivateChatChannel(pChat, &Chat);
			if (Chat.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_CREATE_PRIVATECHATCHANNEL:%d] QueyrAddPrivateChatChannel Query Error Ret:%d\r\n", pChat->cWorldSetID, Chat.nRetCode);
			else
			{
				TQAddPrivateChatMember pQData;
				memset(&pQData, 0, sizeof(pQData));
				pQData.Member.nAccountDBID = pChat->nAccountDBID;
				pQData.Member.biCharacterDBID = pChat->tPrivateChatChannel.biMasterCharacterDBID;
				pQData.nPrivateChatChannelID = Chat.tPrivateChatChannel.nPrivateChatChannelID;
				pQData.Member.bMaster = true;
				pQData.Member.tJoinDate = Chat.tJoinDate;
				_wcscpy(pQData.Member.wszCharacterName, _countof(pQData.Member.wszCharacterName), pChat->wszCharacterName, (int)wcslen(pChat->wszCharacterName));

				TAAddPrivateChatMember pAData;
				memset(&pAData, 0, sizeof(pAData));
				Chat.nRetCode = pWorldDB->QueyrAddPrivateChatChannelMember(&pQData);
				if(Chat.nRetCode != ERROR_NONE)
				{
					g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_CREATE_PRIVATECHATCHANNEL:%d] QueyrAddPrivateChatChannelMember Query Error Ret:%d\r\n", pChat->cWorldSetID, Chat.nRetCode);
				}
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), sizeof(Chat));			
		}
		break;
	case QUERY_ADD_PRIVATECHATMEMBER:
		{
			TQAddPrivateChatMember* pChat = reinterpret_cast<TQAddPrivateChatMember*>(pData);

			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pChat->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_ADD_PRIVATECHATMEMBER:%d] pWorldDB not found\r\n", pChat->cWorldSetID);
				break;
			}

			TAAddPrivateChatMember Chat;
			memset(&Chat, 0, sizeof(Chat));

			Chat.cWorldSetID = pChat->cWorldSetID;
			Chat.nAccountDBID = pChat->nAccountDBID;
			Chat.nPrivateChatChannelID = pChat->nPrivateChatChannelID;
			Chat.Member = pChat->Member;			

			if(pChat->eType == PrivateChatChannel::Common::JoinMember)
				Chat.nRetCode = pWorldDB->QueyrAddPrivateChatChannelMember(pChat);
			else if(pChat->eType == PrivateChatChannel::Common::EnterMember)
				Chat.nRetCode = pWorldDB->QueyrEnterPrivateChatChannelMember(pChat);

			switch(Chat.nRetCode)
			{
			case ERROR_NONE:
			case 103345: // User has already entered a private channel.
				break;

			default:
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_ADD_PRIVATECHATMEMBER:%d] Query Error Ret:%d\r\n", pChat->cWorldSetID, Chat.nRetCode);
				break;
			}
			
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), sizeof(Chat));			
		}
		break;
	case QUERY_DEL_PRIVATECHATMEMBER:
	case QUERY_KICK_PRIVATECHATMEMBER:
		{
			TQDelPrivateChatMember* pChat = reinterpret_cast<TQDelPrivateChatMember*>(pData);

			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pChat->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, pChat->biCharacterDBID, 0, L"[CDBID:0] [QUERY_DEL_PRIVATECHATMEMBER:%d] Query pWorldDB not found\r\n", pChat->cWorldSetID);
				break;
			}

			TADelPrivateChatMember Chat;
			memset(&Chat, 0, sizeof(Chat));

			Chat.cWorldSetID = pChat->cWorldSetID;
			Chat.nAccountDBID = pChat->nAccountDBID;
			Chat.biCharacterDBID = pChat->biCharacterDBID;
			Chat.nPrivateChatChannelID = pChat->nPrivateChatChannelID;
			_wcscpy(Chat.wszName, _countof(Chat.wszName), pChat->wszName, (int)wcslen(pChat->wszName) );	

			if( pChat->eType == PrivateChatChannel::Common::ExitMember )
				Chat.nRetCode = pWorldDB->QueyrExitPrivateChatChannelMember(pChat);
			else
				Chat.nRetCode = pWorldDB->QueyrDelPrivateChatChannelMember(pChat);
			if (Chat.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, pChat->biCharacterDBID, 0, L"[CDBID:0] [QUERY_DEL_PRIVATECHATMEMBER:%d] Query Error Ret:%d\r\n", pChat->cWorldSetID, Chat.nRetCode);

			if(nSubCmd == QUERY_KICK_PRIVATECHATMEMBER)
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), sizeof(Chat));			
		}
		break;	
	case QUERY_MOD_PRIVATECHATCHANNELINFO:
		{
			TQModPrivateChatChannelInfo* pChat = reinterpret_cast<TQModPrivateChatChannelInfo*>(pData);

			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pChat->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, pChat->biCharacterDBID, 0, L"[CDBID:0] [QUERY_MOD_PRIVATECHATCHANNELINFO:%d] Query pWorldDB not found\r\n", pChat->cWorldSetID);
				break;
			}

			TAModPrivateChatChannelInfo Chat;
			memset(&Chat, 0, sizeof(Chat));

			Chat.cWorldSetID = pChat->cWorldSetID;
			Chat.eType = pChat->eType;
			Chat.nAccountDBID = pChat->nAccountDBID;
			Chat.biCharacterDBID = pChat->biCharacterDBID;
			Chat.nPassWord = pChat->nPassWord;
			Chat.nPrivateChatChannelID = pChat->nPrivateChatChannelID;


			Chat.nRetCode = pWorldDB->QueyrModPrivateChatChannelInfo(pChat);
			if (Chat.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, pChat->biCharacterDBID, 0, L"[CDBID:0] [QUERY_MOD_PRIVATECHATCHANNELINFO:%d] Query Error Ret:%d\r\n", pChat->cWorldSetID, Chat.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), sizeof(Chat));			
		}
		break;
	case QUERY_INVITE_PRIVATECHATMEMBER:
		{
			TQInvitePrivateChatMember* pChat = reinterpret_cast<TQInvitePrivateChatMember*>(pData);

			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pChat->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_INVITE_PRIVATECHATMEMBER:%d] pWorldDB not found\r\n", pChat->cWorldSetID);
				break;
			}

			TAInvitePrivateChatMember Chat;
			memset(&Chat, 0, sizeof(Chat));

			Chat.cWorldSetID = pChat->cWorldSetID;
			Chat.nAccountDBID = pChat->nAccountDBID;
			Chat.nPrivateChatChannelID = pChat->nPrivateChatChannelID;
			Chat.Member = pChat->Member;
			Chat.nMasterAccountDBID = pChat->nMasterAccountDBID;

			Chat.nRetCode = pWorldDB->QueyrInvitePrivateChatChannelMember(pChat);
			
			if (Chat.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_INVITE_PRIVATECHATMEMBER:%d] Query Error Ret:%d\r\n", pChat->cWorldSetID, Chat.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), sizeof(Chat));			
		}
		break;
	case QUERY_MOD_PRIVATEMEMBERSERVERID:
		{
			TQModPrivateChatMemberServerID* pChat = reinterpret_cast<TQModPrivateChatMemberServerID*>(pData);

			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pChat->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, pChat->biCharacterDBID, 0, L"[CDBID:0] [QUERY_MOD_PRIVATEMEMBERSERVERID:%d] pWorldDB not found\r\n", pChat->cWorldSetID);
				break;
			}			

			int nRetCode = pWorldDB->QueyrModPrivateMemberServerID(pChat);

			if (nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, pChat->biCharacterDBID, 0, L"[CDBID:0] [QUERY_MOD_PRIVATEMEMBERSERVERID:%d] Query Error Ret:%d\r\n", pChat->cWorldSetID, nRetCode);
			
		}
		break;
	case QUERY_DEL_PRIVATEMEMBERSERVERID:
		{
			TQDelPrivateChatMemberServerID* pChat = reinterpret_cast<TQDelPrivateChatMemberServerID*>(pData);

			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pChat->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_DEL_PRIVATEMEMBERSERVERID:%d] pWorldDB not found\r\n", pChat->cWorldSetID);
				break;
			}	

			TADelPrivateChatMemberServerID Chat;			
			memset(&Chat, 0, sizeof(Chat));
			Chat.cWorldSetID = pChat->cWorldSetID;
			std::list<TPrivateMemberDelServer> MemberList;
			MemberList.clear();
			int nRetCode = pWorldDB->QueyrDelPrivateMemberServerID(pChat, MemberList);

			if (nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pChat->cWorldSetID, pChat->nAccountDBID, 0, 0, L"[CDBID:0] [QUERY_DEL_PRIVATEMEMBERSERVERID:%d] Query Error Ret:%d\r\n", pChat->cWorldSetID, nRetCode);

			
			for( std::list<TPrivateMemberDelServer>::iterator itor = MemberList.begin();itor != MemberList.end();itor++ )
			{
				Chat.Member[Chat.nCount] = *itor;
				Chat.nCount++;				
				if( Chat.nCount >= PrivateChatChannel::Common::MaxMemberCount )
				{
					m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), sizeof(Chat));
					memset(&Chat, 0, sizeof(Chat));
					Chat.nCount = 0;
				}
			}

			if( Chat.nCount > 0 )
			{
				int nSize = (sizeof(TADelPrivateChatMemberServerID) - sizeof(Chat.Member)) + (Chat.nCount *sizeof(Chat.Member[0]));
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Chat), sizeof(Chat));
			}
		}
		break;
	
	default:
		break;
	}
}
#endif // #if defined (PRE_PRIVATECHAT_CHANNEL)