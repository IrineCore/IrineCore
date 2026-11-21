#include "stdafx.h"
#include "DNGuildTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "DNExtManager.h"
#include "Log.h"

CDNGuildTask::CDNGuildTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{

}

CDNGuildTask::~CDNGuildTask(void)
{
}

void CDNGuildTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLMembership *pMembershipDB = NULL;
	CDNSQLWorld *pWorldDB = NULL;
	int nRet = ERROR_DB;

	switch (nSubCmd)
	{
	case QUERY_DISMISSGUILD:			// Guild disband request
		{
			TQDismissGuild *pGuild = reinterpret_cast<TQDismissGuild *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TADismissGuild Guild;
				memset(&Guild, 0, sizeof(Guild));

				Guild.nRetCode = ERROR_DB;
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.cWorldSetID = pGuild->cWorldSetID;
				Guild.nCharacterDBID = pGuild->nCharacterDBID;
				Guild.nGuildDBID = pGuild->nGuildDBID;

				Guild.nRetCode = pWorldDB->QueryDismissGuild(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(Guild));

				if (ERROR_NONE != Guild.nRetCode) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_DISMISSGUILD:%d] Query Error (Ret:%d)\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID, Guild.nRetCode);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_DISMISSGUILD:%d] pWorldDB not found\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID);
			}
		}
		break;

	case QUERY_ADDGUILDMEMBER:			// Request to add guild member
		{
			TQAddGuildMember *pGuild = reinterpret_cast<TQAddGuildMember *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TAAddGuildMember Guild;
				memset(&Guild, 0, sizeof(Guild));

				Guild.nRetCode = ERROR_DB;
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.cWorldSetID = pGuild->cWorldSetID;
				Guild.nGuildDBID = pGuild->nGuildDBID;

				Guild.nRetCode = pWorldDB->QueryAddGuildMember(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(Guild));

				if (ERROR_NONE != Guild.nRetCode) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_ADDGUILDMEMBER:%d] Query Error (Ret:%d)\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID, Guild.nRetCode);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_ADDGUILDMEMBER:%d] pWorldDB not found\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID);
			}
		}
		break;

	case QUERY_DELGUILDMEMBER:			// Guild member removal (leave/kick) request
		{
			TQDelGuildMember *pGuild = reinterpret_cast<TQDelGuildMember *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TADelGuildMember Guild;
				memset(&Guild, 0, sizeof(Guild));

				Guild.nRetCode = ERROR_DB;
				Guild.cWorldSetID = pGuild->cWorldSetID;
				Guild.nAccountDBID = pGuild->nReqAccountDBID;
				Guild.nReqAccountDBID = pGuild->nReqAccountDBID;
				Guild.nReqCharacterDBID = pGuild->nReqCharacterDBID;
				Guild.nDelAccountDBID = pGuild->nDelAccountDBID;
				Guild.nDelCharacterDBID = pGuild->nDelCharacterDBID;
				Guild.nGuildDBID = pGuild->nGuildDBID;
				Guild.bIsExiled = pGuild->bIsExiled;
#ifdef PRE_ADD_BEGINNERGUILD
				Guild.bIsGraduateBeginnerGuild = pGuild->bIsGraduateBeginnerGuild;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

				Guild.nRetCode = pWorldDB->QueryDelGuildMember(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(Guild));

				switch (Guild.nRetCode)
				{
				case ERROR_NONE:
				case 103241: // Not a guild member or is the guild master.
					break;

				default:
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nReqAccountDBID, pGuild->nReqCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_DELGUILDMEMBER:%d] Query Error (Ret:%d)\r\n", pGuild->nReqAccountDBID, pGuild->nReqCharacterDBID, pGuild->cWorldSetID, Guild.nRetCode);
					break;
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nReqAccountDBID, pGuild->nReqCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_DELGUILDMEMBER:%d] pWorldDB not found\r\n", pGuild->nReqAccountDBID, pGuild->nReqCharacterDBID, pGuild->cWorldSetID);
			}
		}
		break;	

	case QUERY_CREATEGUILD:				// Request guild creation
		{
			TQCreateGuild *pGuild = reinterpret_cast<TQCreateGuild *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TACreateGuild Guild;
				memset(&Guild, 0, sizeof(Guild));

				Guild.nRetCode = ERROR_DB;
				Guild.cWorldSetID = pGuild->cWorldSetID;
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nCharacterDBID = pGuild->nCharacterDBID;
				_wcscpy(Guild.wszGuildName, _countof(Guild.wszGuildName), pGuild->wszGuildName, (int)wcslen(pGuild->wszGuildName));
				Guild.nEstablishExpense = pGuild->nEstablishExpense;

				Guild.nRetCode = pWorldDB->QueryCreateGuild(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(Guild));

				switch (Guild.nRetCode)
				{
				case ERROR_NONE:
				case 103234: // Guild name already exists.
					break;

				default:
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_CREATEGUILD:%d] Query Error (Ret:%d)\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID, Guild.nRetCode);
					break;
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_CREATEGUILD:%d] pWorldDB not found\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID);
			}
		}
		break;

	case QUERY_CHANGEGUILDINFO:			// Request guild info change
		{
			TQChangeGuildInfo *pGuild = reinterpret_cast<TQChangeGuildInfo *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAChangeGuildInfo Guild;
				memset(&Guild, 0, sizeof(Guild));

				Guild.nRetCode = ERROR_DB;
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nCharacterDBID = pGuild->nCharacterDBID;
				Guild.nGuildDBID = pGuild->nGuildDBID;
				Guild.cWorldSetID = pGuild->cWorldSetID;
				Guild.btGuildUpdate = pGuild->btGuildUpdate;
				Guild.nInt1 = pGuild->nInt1;
				Guild.nInt2 = pGuild->nInt2;
				Guild.nInt3 = pGuild->nInt3;
				Guild.nInt4 = pGuild->nInt4;
				Guild.biInt64 = pGuild->biInt64;
				Guild.nCurCommonPoint = pGuild->nAddCommonPoint;
				_wcscpy(Guild.Text, _countof(Guild.Text), pGuild->Text, (int)wcslen(pGuild->Text));

				switch(pGuild->btGuildUpdate) 
				{
				case GUILDUPDATE_TYPE_NOTICE:		// Change notice
					{
						Guild.nRetCode = pWorldDB->QueryChangeGuildInfoNotice(pGuild, &Guild);
					}
					break;
				case GUILDUPDATE_TYPE_ROLEAUTH:		// Change rank permissions
					{
						Guild.nRetCode = pWorldDB->QueryChangeGuildInfoRoleAuth(pGuild, &Guild);
					}
					break;

#if defined( PRE_ADD_GUILD_EASYSYSTEM )
				case GUILDUPDATE_TYPE_HOMEPAGE:
					{
						Guild.nRetCode = pWorldDB->QueryChangeGuildInfoHomePage(pGuild, &Guild);
					}
					break;
#endif

				default:
					break;
				}

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(Guild));

				if (ERROR_NONE != Guild.nRetCode) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_CHANGEGUILDINFO:%d/%d] Query Error (Ret:%d)\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID, pGuild->btGuildUpdate, Guild.nRetCode);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_CHANGEGUILDINFO:%d/%d] pWorldDB not found\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID, pGuild->btGuildUpdate);
			}
		}
		break;

	case QUERY_CHANGEGUILDMEMBERINFO:		// Guild member info update request
		{
			TQChangeGuildMemberInfo *pGuild = reinterpret_cast<TQChangeGuildMemberInfo *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TAChangeGuildMemberInfo Guild;
				memset(&Guild, 0, sizeof(Guild));

				Guild.nRetCode = ERROR_DB;
				Guild.nAccountDBID = pGuild->nReqAccountDBID;
				Guild.nReqAccountDBID = pGuild->nReqAccountDBID;
				Guild.nReqCharacterDBID = pGuild->nReqCharacterDBID;
				Guild.nChgAccountDBID = pGuild->nChgAccountDBID;
				Guild.nChgCharacterDBID = pGuild->nChgCharacterDBID;
				Guild.nGuildDBID = pGuild->nGuildDBID;
				Guild.cWorldSetID = pGuild->cWorldSetID;
				Guild.btGuildMemberUpdate = pGuild->btGuildMemberUpdate;
				Guild.nInt1 = pGuild->nInt1;
				Guild.nInt2 = pGuild->nInt2;
				Guild.biInt64 = pGuild->biInt64;
				_wcscpy(Guild.Text, _countof(Guild.Text), pGuild->Text, (int)wcslen(pGuild->Text));

				switch(pGuild->btGuildMemberUpdate)
				{
				case GUILDMEMBUPDATE_TYPE_INTRODUCE:		// Change self-introduction (guild member self)
					{
						Guild.nRetCode = pWorldDB->QueryChangeGuildMemberInfoIntroduce(pGuild, &Guild);
					}
					break;
				case GUILDMEMBUPDATE_TYPE_ROLE:				// Change rank (Guild Master -> Guild Member ?)
					{
						Guild.nRetCode = pWorldDB->QueryChangeGuildMemberInfoRole(pGuild, &Guild);
					}
					break;
				case GUILDMEMBUPDATE_TYPE_GUILDMASTER:		// Delegate guild master (guild master -> member ?)
					{
						Guild.nRetCode = pWorldDB->QueryChangeGuildMemberInfoGuildMaster(pGuild, &Guild);
					}
					break;
				default:
					break;
				}

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(Guild));

				if (ERROR_NONE != Guild.nRetCode)
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nReqAccountDBID, pGuild->nReqCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_CHANGEGUILDMEMBERINFO:%d/%d] Query Error (Ret:%d)\r\n", pGuild->nReqAccountDBID, pGuild->nReqCharacterDBID, pGuild->cWorldSetID, pGuild->btGuildMemberUpdate, Guild.nRetCode);
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nReqAccountDBID, pGuild->nReqCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_CHANGEGUILDMEMBERINFO:%d/%d] pWorldDB not found\r\n", pGuild->nReqAccountDBID, pGuild->nReqCharacterDBID, pGuild->cWorldSetID, pGuild->btGuildMemberUpdate);
			}
		}
		break;

	case QUERY_ADDGUILDHISTORY:			// Add guild history request
		{
			TQAddGuildHistory *pGuild = reinterpret_cast<TQAddGuildHistory *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QueryAddGuildHistory(pGuild);

				if (ERROR_NONE != nRet) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ADDGUILDHISTORY:%d] Query Error (Ret:%d)\r\n", pGuild->cWorldSetID, nRet);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ADDGUILDHISTORY:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}
		break;

	case QUERY_GETGUILDHISTORYLIST:		// Guild history list request
		{
			TQGetGuildHistoryList *pGuild = reinterpret_cast<TQGetGuildHistoryList *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TAGetGuildHistoryList Guild;
				memset(&Guild, 0, sizeof(Guild));

				Guild.nRetCode = ERROR_DB;
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nCharacterDBID = pGuild->nCharacterDBID;
				Guild.nGuildDBID = pGuild->nGuildDBID;
				Guild.cWorldSetID = pGuild->cWorldSetID;
				Guild.biIndex = pGuild->biIndex;
				Guild.bDirection = pGuild->bDirection;

				Guild.nRetCode = pWorldDB->QueryGetGuildHistoryList(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(Guild));

				if (ERROR_NONE != Guild.nRetCode) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_GETGUILDHISTORYLIST:%d] Query Error (Ret:%d)\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID, Guild.nRetCode);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_GETGUILDHISTORYLIST:%d] pWorldDB not found\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, pGuild->cWorldSetID);
			}
		}
		break;	

	case QUERY_INVITEGUILDMEMBER:			// Guild member invite request
		{
			TQInviteGuildMember *pPacket = reinterpret_cast<TQInviteGuildMember *>(pData);

			TQGetGuildInfo *pGuild = reinterpret_cast<TQGetGuildInfo *>(&pPacket->GuildInfo);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				std::vector<TGuildMember> vGuildMember;
				vGuildMember.clear();
				TAInviteGuildMember Guild;
				memset(&Guild, 0, sizeof(Guild));

				Guild.nRetCode = ERROR_DB;
				Guild.nAccountDBID = pPacket->nAccountDBID;
				Guild.nToSessionID = pPacket->nToSessionID;

				Guild.GuildInfo.nRetCode = ERROR_DB;
				Guild.GuildInfo.nAccountDBID = pPacket->nAccountDBID;
				Guild.GuildInfo.cWorldSetID = pPacket->cWorldSetID;
				Guild.GuildInfo.nGuildDBID = pGuild->nGuildDBID;

				Guild.GuildInfo.nRetCode = pWorldDB->QueryGetGuildInfo(pGuild->nGuildDBID, pGuild->cWorldSetID, &Guild.GuildInfo);
				if (ERROR_NONE == Guild.GuildInfo.nRetCode && pGuild->bNeedMembList)
				{
					Guild.GuildInfo.nRetCode = pWorldDB->QueryGetGuildMemberList(pGuild->nGuildDBID, pGuild->cWorldSetID, vGuildMember);
				}

				Guild.nRetCode = Guild.GuildInfo.nRetCode;
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(Guild));

				if( Guild.GuildInfo.nRetCode == ERROR_NONE )
				{
					TAGetGuildMember GuildMember;
					memset(&GuildMember, 0, sizeof(GuildMember));
					GuildMember.nRetCode = Guild.GuildInfo.nRetCode;
					GuildMember.nGuildDBID = pGuild->nGuildDBID;
					GuildMember.cWorldSetID = pGuild->cWorldSetID;
					GuildMember.bFirstPage = true;

					int nPacketSize = (sizeof(TAGetGuildMember) - sizeof(GuildMember.MemberList));
					int nMaxSendMember = ((SERVERDETACHPACKETSIZE)-nPacketSize) / sizeof(GuildMember.MemberList[0]);
					if(nMaxSendMember > SENDGUILDMEMBER_MAX)
						nMaxSendMember = SENDGUILDMEMBER_MAX;

					for(std::vector<TGuildMember>::iterator itor = vGuildMember.begin(); itor != vGuildMember.end(); itor++)
					{
						GuildMember.MemberList[GuildMember.nCount++] = *itor;
						if(GuildMember.nCount >= nMaxSendMember)
						{
							if( itor + 1 == vGuildMember.end() )
								GuildMember.bEndPage = true;
							int nSize = (sizeof(TAGetGuildMember) - sizeof(GuildMember.MemberList)) + (sizeof(GuildMember.MemberList[0]) * GuildMember.nCount);
							m_pConnection->AddSendData(nMainCmd, QUERY_GET_GUILDMEMBER, reinterpret_cast<char*>(&GuildMember), nSize);							
							GuildMember.nCount = 0;
							memset( GuildMember.MemberList, 0, sizeof(GuildMember.MemberList));
							GuildMember.bFirstPage = false;
						}
					}
					if( GuildMember.nCount > 0 )
					{
						GuildMember.bEndPage = true;
						int nSize = (sizeof(TAGetGuildMember) - sizeof(GuildMember.MemberList)) + (sizeof(GuildMember.MemberList[0]) * GuildMember.nCount);
						m_pConnection->AddSendData(nMainCmd, QUERY_GET_GUILDMEMBER, reinterpret_cast<char*>(&GuildMember), nSize);
					}				
				}				

				if (ERROR_NONE != Guild.nRetCode) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GETGUILDINFO:%d/%d] Query Error (Ret:%d)\r\n", pGuild->cWorldSetID, pGuild->bNeedMembList, Guild.nRetCode);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GETGUILDINFO:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}
		break;	
	case QUERY_MOVE_ITEM_IN_GUILDWARE:		// Move item between guild warehouses
		{
			TQMoveItemInGuildWare *pGuild = reinterpret_cast<TQMoveItemInGuildWare *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TAMoveItemInGuildWare Guild;
				memset(&Guild, 0, sizeof(TAMoveItemInGuildWare));

				Guild.nRetCode = pWorldDB->QueryMoveItemInGuildWare(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAMoveItemInGuildWare));

				switch (Guild.nRetCode)
				{
				case ERROR_NONE:
				case 103269: // Item to split from guild storage is not mapped in guild storage or is locked.
				case 103270: // Item to split from guild storage does not exist.
				case 103271: // Item count to split from guild storage does not match.
				case 103272: // One or more of the two items involved in Merge are not mapped to the guild warehouse or are locked.
				case 103273: // Item for @inbItemSerial does not exist.
				case 103274: // Item quantity for @inbItemSerial does not match.
				case 103275: // Item for @inbMergeTargetItemSerial does not exist.
				case 103276: // Item quantity for @inbMergeTargetItemSerial does not match.
				case 103255: // Guild storage is full
					break;

				default:
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_MOVE_ITEM_IN_GUILDWARE:%d] Query Error\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, Guild.nRetCode);
					break;
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_MOVEITEMINGUILDWARE:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}		
		break;

	case QUERY_MOVE_INVEN_TO_GUILDWARE:		// Move item from Inven -> Guild warehouse
		{
			TQMoveInvenToGuildWare *pGuild = reinterpret_cast<TQMoveInvenToGuildWare *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TAMoveInvenToGuildWare Guild;

				nRet = pWorldDB->QueryMoveInvenToGuildWare(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAMoveInvenToGuildWare));

				switch (nRet)
				{
				case ERROR_NONE:
				case 103170: // Item does not exist or quantity is insufficient. (Split failed)  
				case 103253: // Item to move to guild storage does not exist.  
				case 103254: // Item count to move to guild storage does not match.  
				case 103255: // Guild storage is full.  
				case 103256: // Target item to merge in guild storage does not exist.  
				case 103257: // Target item count to merge in guild storage does not match.  
				case 103258: // Target item to merge in guild storage does not exist or is locked by another user.  
				case 103259: // Item to move to guild storage is not mapped in inventory.  
				case 103292: // Item is already registered on the market.  
					break;

				default:
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_MOVE_INVEN_TO_GUILDWARE:%d] Query Error\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, nRet);
					break;
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_MOVEITEMINGUILDWARE:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}		
		break;

	case QUERY_MOVE_GUILDWARE_TO_INVEN:		// Move item from guild storage -> inventory
		{
			TQMoveGuildWareToInven *pGuild = reinterpret_cast<TQMoveGuildWareToInven *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TAMoveGuildWareToInven Guild;
				nRet = pWorldDB->QueryMoveGuildWareToInven(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAMoveGuildWareToInven));

				switch (nRet)
				{
				case ERROR_NONE:
				case 103170:
				case 103260:
				case 103261:
				case 103262:
				case 103263:
				case 103264:
				case 103265:
				case 103266:
				case 103267:
					break;

				default:
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_MOVE_GUILDWARE_TO_INVEN:%d] Query Error\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, nRet);
					break;
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_MOVEITEMINGUILDWARE:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}		
		break;

	case QUERY_GUILDWARE_COIN:		// Move coin between Inven <-> Warehouse
		{
			TQGuildWareHouseCoin *pGuild = reinterpret_cast<TQGuildWareHouseCoin *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				TAGuildWareHouseCoin Guild;
				nRet = pWorldDB->QueryModGuildCoin(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGuildWareHouseCoin));

				if (ERROR_NONE != nRet) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_GUILDWARE_COIN:%d] Query Error\r\n", pGuild->nAccountDBID, pGuild->nCharacterDBID, nRet);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_GUILDWARE_COIN:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}		
		break;

	case QUERY_GET_GUILDWAREINFO:
		{
			TQGetGuildWareInfo *pGuild = reinterpret_cast<TQGetGuildWareInfo *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAGetGuildWareInfo Guild;

				nRet = pWorldDB->QueryGetGuildWareInfo(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWareInfo));

				if (ERROR_NONE != nRet) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWAREINFO] Query Error Guild:%d Ret:%d\r\n", pGuild->nGuildDBID, nRet);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWAREINFO:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}
		break;

	case QUERY_SET_GUILDWAREINFO:
		{
			TQSetGuildWareInfo *pGuild = reinterpret_cast<TQSetGuildWareInfo *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TASetGuildWareInfo Guild;

				nRet = pWorldDB->QuerySetGuildWareInfo(pGuild, &Guild);

				if (ERROR_NONE != nRet) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_SET_GUILDWAREINFO:%d/%d] Query Error\r\n");
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_SET_GUILDWAREINFO:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}
		break;


	case QUERY_GET_GUILDWAREHISTORY:
		{
			TQGetGuildWareHistory *pGuild = reinterpret_cast<TQGetGuildWareHistory *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAGetGuildWareHistory Guild;

				nRet = pWorldDB->QueryGetGuildWareHistoryList(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWareHistory));

				if (ERROR_NONE != nRet) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_GET_GUILDWAREHISTORY:%d/%d] Query Error\r\n");
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_GET_GUILDWAREHISTORY:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}
		break;

	case QUERY_CHANGEGUILDWARESIZE:
		{
			TQChangeGuildWareSize  *pGuild = reinterpret_cast<TQChangeGuildWareSize  *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAChangeGuildWareSize Guild;

				Guild.nRetCode = ERROR_DB;
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nCharacterDBID = pGuild->nCharacterDBID;
				Guild.nGuildDBID = pGuild->nGuildDBID;
				Guild.cWorldSetID = pGuild->cWorldSetID;
				Guild.wStorageSize = pGuild->wStorageSize;

				Guild.nRetCode = pWorldDB->QueryChangeGuildWareSize(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAChangeGuildWareSize));

				if (ERROR_NONE != Guild.nRetCode) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_CHANGEGUILDWARESIZE:%d/%d] Query Error (Ret:%d)\r\n", Guild.nRetCode);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_CHANGEGUILDWARESIZE:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}
		break;

	case QUERY_CHANGEGUILDMARK:
		{
			TQChangeGuildMark   *pGuild = reinterpret_cast<TQChangeGuildMark   *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAChangeGuildMark Guild;
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nCharacterDBID = pGuild->nCharacterDBID;
				Guild.nGuildDBID = pGuild->nGuildDBID;
				Guild.cWorldSetID = pGuild->cWorldSetID;

				Guild.wGuildMark = pGuild->wGuildMark;
				Guild.wGuildMarkBG = pGuild->wGuildMarkBG;
				Guild.wGuildMarkBorder = pGuild->wGuildMarkBorder;

				Guild.nRetCode = pWorldDB->QueryChangeGuildMark(pGuild, &Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAChangeGuildMark));

				if (ERROR_NONE != Guild.nRetCode) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_CHANGEGUILDMARK:%d/%d] Query Error\r\n");
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->nCharacterDBID, 0, L"[QUERY_CHANGEGUILDMARK:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}
		break;

	case QUERY_GETGUILDINFO:			// Request guild info
		{
			TQGetGuildInfo *pGuild = reinterpret_cast<TQGetGuildInfo *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB){
				std::vector<TGuildMember> vGuildMember;
				vGuildMember.clear();
				TAGetGuildInfo Guild;				
				memset(&Guild, 0, sizeof(Guild));				

				Guild.nRetCode = ERROR_DB;
				Guild.nGuildDBID = pGuild->nGuildDBID;
				Guild.cWorldSetID = pGuild->cWorldSetID;
				Guild.nRetCode = pWorldDB->QueryGetGuildInfo(pGuild->nGuildDBID, pGuild->cWorldSetID, &Guild);
				
				if (ERROR_NONE == Guild.nRetCode && pGuild->bNeedMembList) {
					Guild.nRetCode = pWorldDB->QueryGetGuildMemberList(pGuild->nGuildDBID, pGuild->cWorldSetID, vGuildMember);					
				}
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildInfo));

				if( Guild.nRetCode == ERROR_NONE )
				{
					TAGetGuildMember GuildMember;
					memset(&GuildMember, 0, sizeof(GuildMember));
					GuildMember.nRetCode = Guild.nRetCode;
					GuildMember.nGuildDBID = pGuild->nGuildDBID;
					GuildMember.cWorldSetID = pGuild->cWorldSetID;
					GuildMember.bFirstPage = true;
					int nPacketSize = (sizeof(TAGetGuildMember) - sizeof(GuildMember.MemberList));
					int nMaxSendMember = ((SERVERDETACHPACKETSIZE)-nPacketSize) / sizeof(GuildMember.MemberList[0]);
					if(nMaxSendMember > SENDGUILDMEMBER_MAX)
						nMaxSendMember = SENDGUILDMEMBER_MAX;					

					for(std::vector<TGuildMember>::iterator itor = vGuildMember.begin(); itor != vGuildMember.end(); itor++)
					{
						GuildMember.MemberList[GuildMember.nCount++] = *itor;
						if(GuildMember.nCount >= nMaxSendMember)
						{
							if( itor + 1 == vGuildMember.end() )
								GuildMember.bEndPage = true;
							int nSize = (sizeof(TAGetGuildMember) - sizeof(GuildMember.MemberList)) + (sizeof(GuildMember.MemberList[0]) * GuildMember.nCount);
							m_pConnection->AddSendData(nMainCmd, QUERY_GET_GUILDMEMBER, reinterpret_cast<char*>(&GuildMember), nSize);
							GuildMember.nCount = 0;
							memset( GuildMember.MemberList, 0, sizeof(GuildMember.MemberList));
							GuildMember.bFirstPage = false;
						}
					}
					if( GuildMember.nCount > 0 )
					{
						GuildMember.bEndPage = true;
						int nSize = (sizeof(TAGetGuildMember) - sizeof(GuildMember.MemberList)) + (sizeof(GuildMember.MemberList[0]) * GuildMember.nCount);
						m_pConnection->AddSendData(nMainCmd, QUERY_GET_GUILDMEMBER, reinterpret_cast<char*>(&GuildMember), nSize);
					}
				}			

				if (ERROR_NONE != Guild.nRetCode) {
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GETGUILDINFO:%d/%d] Query Error (Ret:%d)\r\n", pGuild->cWorldSetID, pGuild->bNeedMembList, Guild.nRetCode);
				}
			}
			else {
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GETGUILDINFO:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}
		break;
	case QUERY_CHANGEGUILDNAME :		// Change guild name
		{
			TQChangeGuildName *pGuild = reinterpret_cast<TQChangeGuildName *>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if( pWorldDB )
			{
				TAChangeGuildName Guild;				
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.cWorldSetID = pGuild->cWorldSetID;				
				Guild.nGuildDBID = pGuild->nGuildDBID;
				Guild.biItemSerial = pGuild->biItemSerial;
				_wcscpy(Guild.wszGuildName, _countof(Guild.wszGuildName), pGuild->wszGuildName, (int)wcslen(pGuild->wszGuildName));
				Guild.nRetCode = pWorldDB->QueryModGuildName(pGuild);
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAChangeGuildName));
			}
			else
			{
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_CHANGEGUILDNAME:%d] pWorldDB not found\r\n", pGuild->cWorldSetID);
			}
		}
		break;
	case QUERY_ENROLL_GUILDWAR:			// Apply for Guild War
		{
			TQEnrollGuildWar *pGuild = reinterpret_cast<TQEnrollGuildWar *>(pData);
			TAEnrollGuildWar Guild;
			memset (&Guild, 0, sizeof(Guild));

			Guild.cWorldSetID	= pGuild->cWorldSetID;
			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nGuildDBID	= pGuild->nGuildDBID;
			Guild.wScheduleID	= pGuild->wScheduleID;
			Guild.nRetCode		= ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryEnrollGuildWar(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ENROLL_GUILDWAR:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAEnrollGuildWar));
		}
		break;

	case QUERY_GET_GUILDWARSCHEDULE:	// Get Guild War schedule info
		{
			TQGetGuildWarSchedule *pGuild = reinterpret_cast<TQGetGuildWarSchedule *>(pData);
			TAGetGuildWarSchedule Guild;
			memset (&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID	= pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarSchedule(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARSCHEDULE:%d] DB Error\r\n", pGuild->cWorldSetID);

			if (Guild.wScheduleID == 0)
				Guild.wScheduleID = GUILDWARSCHEDULE_DEF;

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarSchedule));

		}
		break;
	case QUERY_GET_GUILDWARFINALSCHEDULE :	// Fetch finals schedule
		{
			TQGetGuildWarFinalSchedule *pGuild = reinterpret_cast<TQGetGuildWarFinalSchedule*>(pData);
			TAGetGuildWarFinalSchedule Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarFinalSchedule(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARFINALSCHEDULE:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarFinalSchedule));
		}
		break;
	case QUERY_ADD_GUILDWARPOINT :	// Earn points when clearing preliminary mission
		{
			TQAddGuildWarPointRecodrd *pGuild = reinterpret_cast<TQAddGuildWarPointRecodrd*>(pData);
			TAAddGuildWarPointRecodrd Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;
			Guild.nOpeningPoint = pGuild->nOpeningPoint;
			Guild.nGuildDBID = pGuild->nGuildDBID;
			Guild.cTeamColorCode = pGuild->cTeamColorCode;
			Guild.cWorldSetID = pGuild->cWorldSetID;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryAddGuildWarPoint(pGuild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_ADD_GUILDWARPOINT:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TQAddGuildWarPointRecodrd));
		}
		break;
	case QUERY_GET_GUILDWARPOINT :	// Query preliminary points
		{
			TQGetGuildWarPoint *pGuild = reinterpret_cast<TQGetGuildWarPoint*>(pData);
			TAGetGuildWarPoint Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.cQueryType = pGuild->cQueryType;
			Guild.nRetCode = ERROR_DB;
			Guild.biDBID = pGuild->biDBID;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarPoint(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARPOINT:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarPoint));
		}
		break;		
	case QUERY_GET_GUILDWARPOINT_PARTTOTAL :			// Query 1st place by category (individual)
		{
			TQGetGuildWarPointPartTotal *pGuild = reinterpret_cast<TQGetGuildWarPointPartTotal*>(pData);
			TAGetGuildWarPointPartTotal Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.cQueryType = pGuild->cQueryType;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarPointPartTotal(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_GET_GUILDWARPOINT_PARTTOTAL:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarPointPartTotal));
		}
		break;
	case QUERY_GET_GUILDWARPOINT_GUILD_PARTTOTAL :	// Guild ranking by category
		{
			TQGetGuildWarPointGuildPartTotal *pGuild = reinterpret_cast<TQGetGuildWarPointGuildPartTotal*>(pData);
			TAGetGuildWarPointGuildPartTotal Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.cQueryType = pGuild->cQueryType;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarPointGuildPartTotal(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARPOINT_GUILD_PARTTOTAL:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarPointGuildPartTotal));
		}
		break;
	case QUERY_GET_GUILDWARPOINT_DAILY :				// Daily 1st place in preliminaries		
		{
			TQGetGuildWarPointDaily *pGuild = reinterpret_cast<TQGetGuildWarPointDaily*>(pData);
			TAGetGuildWarPointDaily Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarPointDaily(pGuild->cWorldSetID, pGuild->nAccountDBID, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARPOINT_DAILY:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarPointDaily));
		}
		break;
	case QUERY_GET_GUILDWARPOINT_GUILD_TOTAL :		// Aggregated point rankings of each guild (main tournament qualifiers can also be queried here)
		{
			TQGetGuildWarPointGuildTotal *pGuild = reinterpret_cast<TQGetGuildWarPointGuildTotal*>(pData);
			TAGetGuildWarPointGuildTotal Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.cQueryType = pGuild->cQueryType;
			Guild.bMasterServer = pGuild->bMasterServer;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarPointGuildTotal(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARPOINT_GUILD_TOTAL:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarPointGuildTotal)-(sizeof(SGuildWarPointGuildTotal)*(GUILDWAR_FINALS_TEAM_MAX-Guild.nTotalCount)));
		}
		break;
	case QUERY_GET_GUILDWARPOINT_RUNNING_TOTAL : // Guild War preliminary score standings (top 24)
		{
			TQGetGuildWarPointRunningTotal *pGuild = reinterpret_cast<TQGetGuildWarPointRunningTotal*>(pData);
			TAGetGuildWarPointRunningTotal Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarPointRunningTotal(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARPOINT_RUNNING_TOTAL:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarPointRunningTotal)-(sizeof(Guild.sGuildWarPointGuildTotal[0])*(GUILDWAR_TRIAL_POINT_TEAM_MAX-Guild.nTotalCount)) );
		}
		break;
	case QUERY_GET_GUILDWARPOINT_FINAL_REWARDS :
		{
			TQHeader* pGuild = reinterpret_cast<TQHeader*>(pData);
			TAGetGuildWarPointFinalRewards Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarFinalRewardPoint(&Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARPOINT_FINAL_REWARDS:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarPointFinalRewards));
		}
		break;
	case QUERY_ADD_GUILDWAR_FINAL_MATCHLIST :			// Save finals bracket
		{
			TQAddGuildWarFinalMatchList *pGuild = reinterpret_cast<TQAddGuildWarFinalMatchList*>(pData);
			TAAddGuildWarFinalMatchList Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryAddGuildWarFinalMatchList(pGuild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ADD_GUILDWAR_FINAL_MATCHLIST:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAAddGuildWarFinalMatchList));
		}
		break;
	case QUERY_ADD_GUILDWARPOPULARITYVOTE :	// Save popularity vote
		{
			TQAddGuildWarPopularityVote *pGuild = reinterpret_cast<TQAddGuildWarPopularityVote*>(pData);
			TAAddGuildWarPopularityVote Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryAddGuildWarPopularityVote(pGuild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_ADD_GUILDWARPOPULARITYVOTE:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAAddGuildWarPopularityVote));
		}
		break;
	case QUERY_GET_GUILDWARPOPULARITYVOTE :	// Popularity vote result
		{
			TQGetGuildWarPopularityVote *pGuild = reinterpret_cast<TQGetGuildWarPopularityVote*>(pData);
			TAGetGuildWarPopularityVote Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.cQueryType = pGuild->cQueryType;
			Guild.nRetCode = ERROR_DB;			

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarPopularityVote(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_GET_GUILDWARPOPULARITYVOTE:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarPopularityVote));
		}
		break;
	case QUERY_ADD_GUILDWARFINALRESULTS :		// Save Guild War finals result
		{
			TQAddGuildWarFinalResults *pGuild = reinterpret_cast<TQAddGuildWarFinalResults*>(pData);
			TAAddGuildWarFinalResults Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryAddGuildWarFinalResults(pGuild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ADD_GUILDWARFINALRESULTS:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAAddGuildWarFinalResults));
		}
		break;
	case QUERY_GET_GUILDWARFINALRESULTS :		// Get Guild War finals result
		{
			TQGetGuildWarFinalResults *pGuild = reinterpret_cast<TQGetGuildWarFinalResults*>(pData);
			TAGetGuildWarFinalResults Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarFinalResults(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARFINALRESULTS:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarFinalResults));
		}
		break;
	case QUERY_GET_GUILDWAR_PRE_WIN_GUILD :		// Get previous Guild War season winning team
		{
			TQGetGuildWarPreWinGuild *pGuild = reinterpret_cast<TQGetGuildWarPreWinGuild*>(pData);
			TAGetGuildWarPreWinGuild Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarPreWinGuild(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWARFINALRESULTS:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarPreWinGuild));
		}
		break;
	case QUERY_ADD_GUILDWAR_REWARD_CHARACTER :	// Save Guild War preliminary reward result
		{
			TQAddGuildWarRewardCharacter *pGuild = reinterpret_cast<TQAddGuildWarRewardCharacter*>(pData);
			TAAddGuildWarRewardCharacter Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.cRewardType = pGuild->cRewardType;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryAddGuildWarRewarForCharacter(pGuild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_ADD_GUILDWAR_REWARD_CHARACTER:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAAddGuildWarRewardCharacter));
		}
		break;
	case QUERY_GET_GUILDWAR_REWARD_CHARACTER :	// Get Guild War preliminary reward result
		{
			TQGetGuildWarRewardCharacter *pGuild = reinterpret_cast<TQGetGuildWarRewardCharacter*>(pData);
			TAGetGuildWarRewardCharacter Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarRewarForCharacter(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_GET_GUILDWAR_REWARD_CHARACTER:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarRewardCharacter));
		}
		break;
	case QUERY_ADD_GUILDWAR_REWARD_GUILD :
		{
			TQAddGuildWarRewardGuild *pGuild = reinterpret_cast<TQAddGuildWarRewardGuild*>(pData);
			TAAddGuildWarRewardGuild Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nGuildDBID = pGuild->nGuildDBID;
			Guild.cRewardType = pGuild->cRewardType;
			Guild.cWorldSetID = pGuild->cWorldSetID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryAddGuildWarRewarForGuild(pGuild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ADD_GUILDWAR_REWARD_GUILD:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAAddGuildWarRewardGuild));
		}
		break;
	case QUERY_GET_GUILDWAR_REWARD_GUILD :
		{
			TQGetGuildWarRewardGuild *pGuild = reinterpret_cast<TQGetGuildWarRewardGuild*>(pData);
			TAGetGuildWarRewardGuild Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nGuildDBID = pGuild->nGuildDBID;
			Guild.cWorldSetID = pGuild->cWorldSetID;
			Guild.nRetCode = ERROR_DB;
			Guild.cQueryType = pGuild->cQueryType;
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryGetGuildWarRewarForGuild(pGuild, &Guild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDWAR_REWARD_GUILD:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildWarRewardGuild));
		}
		break;
	case QUERY_ADD_GUILDWAR_POINT_QUEUE : // Grant Guild War finals reward points (personal festival points)
		{
			TQAddGuildWarPointQueue *pGuild = reinterpret_cast<TQAddGuildWarPointQueue*>(pData);
			TAHeader Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;			
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryAddGuildWarPointQueue(pGuild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ADD_GUILDWAR_POINT_QUEUE:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAHeader));
		}
		break;
	case QUERY_ADD_GUILDWAR_SYSTEM_MAIL_QUEUE : // Send Guild War finals reward mail
		{
			TQSendSystemMail *pGuild = reinterpret_cast<TQSendSystemMail*>(pData);
			TAHeader Guild;
			memset(&Guild, 0, sizeof(Guild));

			Guild.nAccountDBID = pGuild->nAccountDBID;
			Guild.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				Guild.nRetCode = pWorldDB->QueryAddGuildWarSystemMailQueue(pGuild);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ADD_GUILDWAR_SYSTEM_MAIL_QUEUE:%d] DB Error\r\n", pGuild->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAHeader));
		}
		break;	
	case QUERY_ADD_GUILDWAR_MAKE_GIFT_QUEUE : // Send Guild War finals reward gift (received as character list)
		{
			TQAddGuilWarMakeGiftQueue *pCash = (TQAddGuilWarMakeGiftQueue*)pData;

			TAHeader Cash;
			memset(&Cash, 0, sizeof(Cash));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB)
			{
				// Pain starts from here...				
				// First extract character ID per class.
				string CharacterDBIDStr[CLASS_MAX];
				char cClass = 0;

				for( int k=0; k< GUILDSIZE_MAX; ++k)
				{
					if( pCash->biCharacterDBIDs[k] <= 0 || pCash->cClass[k] <=0 || pCash->cClass[k] >= CLASS_MAX)
						break;

					cClass = pCash->cClass[k];

					if (!CharacterDBIDStr[cClass].empty()){
						CharacterDBIDStr[cClass].append("|");
					}
					CharacterDBIDStr[cClass].append(boost::lexical_cast<std::string>(pCash->biCharacterDBIDs[k]));
				}

				for (int k=CLASS_WARRIER; k< CLASS_MAX; ++k)
				{	
					if( CharacterDBIDStr[k].empty() )
						continue;

					std::wstring wstrText;				
					int ItemSN[MAILATTACHITEMMAX] = { 0, };		
					int nLifeSpan = 7 * 24 * 60;	// The design doc says 7 days for now

					TGuildWarRewardData* pRewardData = g_pExtManager->GetGuildWarRewardData(pCash->cRewardType, k);
					if( pRewardData == NULL)
						continue;
					TMailTableData* pData = g_pExtManager->GetMailTableData(pRewardData->nPresentID);
					if( pData == NULL )
						continue;

					wstrText = boost::io::str( boost::wformat(L"%d") % pData->nTextUIStringIndex );
					for( int i=0; i<MAILATTACHITEMMAX; i++ )
					{
						if( pData->ItemSNArr[i] <= 0 ) continue;
						ItemSN[i] = pData->ItemSNArr[i];
					}

					string ItemIDStr;
					TCashCommodityData CashData;
					TCashPackageData PackageData;
					for (int i = 0; i < MAILATTACHITEMMAX; i++)
					{
						if (ItemSN[i] <= 0) continue;

						memset(&PackageData, 0, sizeof(TCashPackageData));
						bool bPackage = g_pExtManager->GetCashPackageData(ItemSN[i], PackageData);
						if (bPackage)
						{
							for (int j = 0; j < (int)PackageData.nVecCommoditySN.size(); j++)
							{
								memset(&CashData, 0, sizeof(TCashCommodityData));
								bool bRet = g_pExtManager->GetCashCommodityData(PackageData.nVecCommoditySN[j], CashData);
								if (!bRet) continue;

								if (!ItemIDStr.empty()){
									ItemIDStr.append("|");
								}
								ItemIDStr.append(boost::lexical_cast<std::string>(CashData.nItemID[0]));
							}
							Cash.nRetCode = pMembershipDB->QueryAddGiftSendSchedule(CharacterDBIDStr[k].c_str(), ItemSN[i], ItemIDStr.c_str(), const_cast<WCHAR*>(wstrText.c_str()), nLifeSpan,
								DBDNWorldDef::PayMethodCode::GuildWar_Reward, "127.0.0.1", true);
						}
						else
						{
							memset(&CashData, 0, sizeof(TCashCommodityData));
							bool bRet = g_pExtManager->GetCashCommodityData(ItemSN[i], CashData);
							if (!bRet) continue;
							ItemIDStr = FormatA("%d", CashData.nItemID[0]);
							Cash.nRetCode = pMembershipDB->QueryAddGiftSendSchedule(CharacterDBIDStr[k].c_str(), ItemSN[i], ItemIDStr.c_str(), const_cast<WCHAR*>(wstrText.c_str()), nLifeSpan,
								DBDNWorldDef::PayMethodCode::GuildWar_Reward, "127.0.0.1", true);							
						}
					}
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADD_GUILDWAR_MAKE_GIFT_QUEUE:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADD_GUILDWAR_MAKE_GIFT_QUEUE:%d] DB Error\r\n", pCash->nAccountDBID, pCash->cWorldSetID);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TAHeader));
		}
		break;

	case QUERY_ADD_GUILDWAR_MAKE_GIFT_NOTGUILDINFO :
		{
			TQAddGuildWarMakeGiftNotGuildInfo* pGuild =  (TQAddGuildWarMakeGiftNotGuildInfo*)pData;
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			//First, fetch guild member info.
			if (pWorldDB)
			{
				std::vector<TGuildMember> vGuildMember;
				vGuildMember.clear();
				int nRetCode = pWorldDB->QueryGetGuildMemberList(pGuild->nGuildDBID, pGuild->cWorldSetID, vGuildMember);
				if( nRetCode == ERROR_NONE )
				{
					// Insert gift here.
					pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
					if (pMembershipDB)
					{
						// Pain starts from here...
						// First extract character ID per class.
						string CharacterDBIDStr[CLASS_MAX];
						char cClass = 0;

						for( std::vector<TGuildMember>::iterator itor = vGuildMember.begin() ; itor != vGuildMember.end();itor++)
						{
							if( itor->nCharacterDBID <= 0 )
								break;

							//Retrieve base class..
							TJobTableData* pJobData = g_pExtManager->GetJobTableData(itor->nJob);
							if( pJobData == NULL )
								continue;

							cClass = pJobData->cClass;

							if (!CharacterDBIDStr[cClass].empty()){
								CharacterDBIDStr[cClass].append("|");
							}
							CharacterDBIDStr[cClass].append(boost::lexical_cast<std::string>(itor->nCharacterDBID));
						}	

						for (int k=CLASS_WARRIER; k< CLASS_MAX; ++k)
						{
							std::wstring wstrText;				
							int ItemSN[MAILATTACHITEMMAX] = { 0, };		
							int nLifeSpan = 7 * 24 * 60;	// The design doc says 7 days for now

							if( CharacterDBIDStr[k].empty() )
								continue;

							TGuildWarRewardData* pRewardData = g_pExtManager->GetGuildWarRewardData(pGuild->cRewardType, k);
							if( pRewardData == NULL)
								continue;

							TMailTableData* pData = g_pExtManager->GetMailTableData(pRewardData->nPresentID);
							if( pData == NULL )
								continue;

							wstrText = boost::io::str( boost::wformat(L"%d") % pData->nTextUIStringIndex );
							for( int i=0; i<MAILATTACHITEMMAX; i++ )
							{
								if( pData->ItemSNArr[i] <= 0 ) continue;
								ItemSN[i] = pData->ItemSNArr[i];
							}

							string ItemIDStr;
							TCashCommodityData CashData;
							TCashPackageData PackageData;
							for (int i = 0; i < MAILATTACHITEMMAX; i++)
							{
								if (ItemSN[i] <= 0) continue;

								memset(&PackageData, 0, sizeof(TCashPackageData));
								bool bPackage = g_pExtManager->GetCashPackageData(ItemSN[i], PackageData);
								if (bPackage)
								{
									for (int j = 0; j < (int)PackageData.nVecCommoditySN.size(); j++)
									{
										memset(&CashData, 0, sizeof(TCashCommodityData));
										bool bRet = g_pExtManager->GetCashCommodityData(PackageData.nVecCommoditySN[j], CashData);
										if (!bRet) continue;

										if (!ItemIDStr.empty()){
											ItemIDStr.append("|");
										}
										ItemIDStr.append(boost::lexical_cast<std::string>(CashData.nItemID[0]));
									}
									nRetCode = pMembershipDB->QueryAddGiftSendSchedule(CharacterDBIDStr[k].c_str(), ItemSN[i], ItemIDStr.c_str(), const_cast<WCHAR*>(wstrText.c_str()), nLifeSpan,
										DBDNWorldDef::PayMethodCode::GuildWar_Reward, "127.0.0.1", true);
								}
								else
								{
									memset(&CashData, 0, sizeof(TCashCommodityData));
									bool bRet = g_pExtManager->GetCashCommodityData(ItemSN[i], CashData);
									if (!bRet) continue;
									ItemIDStr = FormatA("%d", CashData.nItemID[0]);
									nRetCode = pMembershipDB->QueryAddGiftSendSchedule(CharacterDBIDStr[k].c_str(), ItemSN[i], ItemIDStr.c_str(), const_cast<WCHAR*>(wstrText.c_str()), nLifeSpan,
										DBDNWorldDef::PayMethodCode::GuildWar_Reward, "127.0.0.1", true);
								}
							}
						}
					}
					else{
						g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADD_GUILDWAR_MAKE_GIFT_NOTGUILDINFO:%d] MembershipDB not found\r\n", 0, pGuild->cWorldSetID);
					}
				}
				else
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ADD_GUILDWAR_MAKE_GIFT_NOTGUILDINFO:%d] QueryGetGuildMemberList DB Error\r\n", nRetCode);				
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_ADD_GUILDWAR_MAKE_GIFT_NOTGUILDINFO:%d] DB Error\r\n", pGuild->cWorldSetID);

		}
		break;

	case QUERY_GETLIST_GUILDWAR_ITEM_TRADE_RECORD:
		{
			TQGetListGuildWarItemTradeRecord* pGuild = reinterpret_cast<TQGetListGuildWarItemTradeRecord*>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAGetListGuildWarItemTradeRecord record;
				memset(&record, 0, sizeof(record));
				record.nAccountDBID = pGuild->nAccountDBID;

				record.nRetCode = pWorldDB->QueryGetListGuildWarItemTradeRecord(pGuild->characterDBID, record);
				if (ERROR_NONE != record.nRetCode)
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->characterDBID, 0, L"[QUERY_GETLIST_GUILDWAR_ITEM_TRADE_RECORD] query error (ret:%d)\r\n", record.nRetCode);

				int len = sizeof(TAGetListGuildWarItemTradeRecord) - sizeof(record.itemCount) + (sizeof(ItemCount) * record.count);
				if (len > 0)
					m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&record), len);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->characterDBID, 0, L"[QUERY_GETLIST_GUILDWAR_ITEM_TRADE_RECORD] worldDB not found\r\n");
		}
		break;

	case QUERY_ADD_GUILDWAR_ITEM_TRADE_RECORD:
		{
			TQAddGuildWarItemTradeRecord* pGuild = reinterpret_cast<TQAddGuildWarItemTradeRecord*>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				int retCode = pWorldDB->QueryAddGuildWarItemTradeRecord(pGuild->characterDBID, pGuild->itemCount);
				if (ERROR_NONE != retCode)
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->characterDBID, 0, L"[QUERY_ADD_GUILDWAR_ITEM_TRADE_RECORD] query error (ret:%d)\r\n", retCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->characterDBID, 0, L"[QUERY_ADD_GUILDWAR_ITEM_TRADE_RECORD] worldDB not found\r\n");
		}
		break;
	case QUERY_MOD_GUILDEXP:
		{
			TQModGuildExp *pGuild = reinterpret_cast<TQModGuildExp *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAModGuildExp Guild;
				memset(&Guild, 0, sizeof(Guild));
				Guild.nGuildDBID	= pGuild->nGuildDBID;
				Guild.cWorldSetID	= pGuild->cWorldSetID;
				Guild.cPointType	= pGuild->cPointType;
				Guild.nPointValue	= pGuild->nPointValue;
				Guild.nLevel		= pGuild->nLevel;
				Guild.biCharacterDBID = pGuild->biCharacterDBID;
				Guild.nMissionID = pGuild->nMissionID;

				Guild.nRetCode		= pWorldDB->QueryModGuildExp(pGuild, Guild);
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAModGuildExp));

				if (ERROR_NONE != Guild.nRetCode)
					g_Log.Log(LogType::_ERROR, Guild.cWorldSetID, Guild.nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_MOD_GUILDEXP] query error (ret:%d)\r\n", Guild.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_MOD_GUILDEXP] worldDB not found\r\n");
		}
		break;
	case QUERY_MOD_GUILDCHEAT:
		{
			TQModGuildCheat *pGuild = reinterpret_cast<TQModGuildCheat *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAModGuildCheat Guild;
				memset(&Guild, 0, sizeof(Guild));
				Guild.nGuildDBID	= pGuild->nGuildDBID;
				Guild.cWorldSetID	= pGuild->cWorldSetID;
				Guild.cCheatType	= pGuild->cCheatType;
				Guild.nPoint		= pGuild->nPoint;
				Guild.cLevel		= pGuild->cLevel;
				
				Guild.nRetCode		= pWorldDB->QueryModGuildCheat(pGuild, Guild);
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAModGuildExp));

				if (ERROR_NONE != Guild.nRetCode)
					g_Log.Log(LogType::_ERROR, Guild.cWorldSetID, Guild.nAccountDBID, 0, 0, L"[QUERY_MOD_GUILDCHEAT] query error (ret:%d)\r\n", Guild.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_MOD_GUILDCHEAT] worldDB not found\r\n");
		}
		break;
	case QUERY_GET_GUILDREWARDITEM:
		{
			TQGetGuildRewardItem *pGuild = reinterpret_cast<TQGetGuildRewardItem *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAGetGuildRewardItem Guild;
				memset(&Guild, 0, sizeof(Guild));
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nGuildDBID	= pGuild->nGuildDBID;
				Guild.cWorldSetID	= pGuild->cWorldSetID;				

				Guild.nRetCode		= pWorldDB->QueryGetGuildRewardItem(pGuild, &Guild);
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildRewardItem));

				if (ERROR_NONE != Guild.nRetCode)
					g_Log.Log(LogType::_ERROR, Guild.cWorldSetID, Guild.nAccountDBID, 0, 0, L"[QUERY_GET_GUILDREWARDITEM] query error (ret:%d)\r\n", Guild.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDREWARDITEM] worldDB not found\r\n");
		}
		break;
	case QUERY_ADD_GUILDREWARDITEM:
		{
			TQAddGuildRewardItem *pGuild = reinterpret_cast<TQAddGuildRewardItem *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAAddGuildRewardItem Guild;
				memset(&Guild, 0, sizeof(Guild));
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nGuildDBID	= pGuild->nGuildDBID;
				Guild.cWorldSetID	= pGuild->cWorldSetID;
				Guild.nItemID		= pGuild->nItemID;
				Guild.cItemBuyType = pGuild->cItemBuyType;

				Guild.nRetCode		= pWorldDB->QueryAddGuildRewardItem(pGuild, &Guild);
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildRewardItem));

				if (ERROR_NONE != Guild.nRetCode)
					g_Log.Log(LogType::_ERROR, Guild.cWorldSetID, Guild.nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_GET_GUILDREWARDITEM] query error (ret:%d)\r\n", Guild.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_GET_GUILDREWARDITEM] worldDB not found\r\n");
		}
		break;
	case QUERY_CHANGE_GUILDSIZE:
		{
			TQChangeGuildSize *pGuild = reinterpret_cast<TQChangeGuildSize *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAChangeGuildSize Guild;
				memset(&Guild, 0, sizeof(Guild));
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nGuildDBID	= pGuild->nGuildDBID;
				Guild.cWorldSetID	= pGuild->cWorldSetID;
				Guild.nGuildSize	= pGuild->nGuildSize;

				Guild.nRetCode		= pWorldDB->QueryChangeGuildSize(pGuild);
				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAChangeGuildSize));

				if (ERROR_NONE != Guild.nRetCode)
					g_Log.Log(LogType::_ERROR, Guild.cWorldSetID, Guild.nAccountDBID, 0, 0, L"[QUERY_CHANGE_GUILDSIZE] query error (ret:%d)\r\n", Guild.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_CHANGE_GUILDSIZE] worldDB not found\r\n");
		}
		break;
	case QUERY_DEL_GUILDREWARDITEM:
		{
			TQDelGuildEffectItem *pGuild = reinterpret_cast<TQDelGuildEffectItem *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				int nRetCode = 0;
				nRetCode = pWorldDB->QueryDelGuildRewardItem(pGuild);				

				if (ERROR_NONE != nRetCode)
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_DEL_GUILDREWARDITEM] query error (ret:%d)\r\n", nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_DEL_GUILDREWARDITEM] worldDB not found\r\n");
		}
		break;	
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
	case QUERY_GET_GUILDCONTRIBUTION_POINT:
		{
			TQGetGuildContributionPoint *pGuild = reinterpret_cast<TQGetGuildContributionPoint *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{
				TAGetGuildContributionPoint Guild;
				memset(&Guild, 0, sizeof(Guild));
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nRetCode		= pWorldDB->QueryGetGuildContributionPoint(pGuild, Guild);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildContributionPoint));

				if (ERROR_NONE != Guild.nRetCode)
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_GET_GUILDCONTRIBUTION_POINT] query error (ret:%d)\r\n", Guild.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, pGuild->biCharacterDBID, 0, L"[QUERY_GET_GUILDCONTRIBUTION_POINT] worldDB not found\r\n");
		}
		break;
	case QUERY_GET_GUILDCONTRIBUTION_WEEKLYRANKING:
		{
			TQGetGuildContributionRanking *pGuild = reinterpret_cast<TQGetGuildContributionRanking *>(pData);
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pGuild->cWorldSetID);
			if (pWorldDB)
			{	
				TAGetGuildContributionRanking Guild;
				memset(&Guild, 0, sizeof(Guild));
				Guild.nAccountDBID = pGuild->nAccountDBID;
				Guild.nRetCode		= pWorldDB->QueryGetGuildContributionWeeklyRanking(pGuild, &Guild);
				
				if (ERROR_NONE != Guild.nRetCode)
					g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDCONTRIBUTION_WEEKLYRANKING] query error (ret:%d)\r\n", Guild.nRetCode);

				m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Guild), sizeof(TAGetGuildContributionRanking));
			}
			else
				g_Log.Log(LogType::_ERROR, pGuild->cWorldSetID, pGuild->nAccountDBID, 0, 0, L"[QUERY_GET_GUILDCONTRIBUTION_WEEKLYRANKING] worldDB not found\r\n");
		}
		break;
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
	}
}
