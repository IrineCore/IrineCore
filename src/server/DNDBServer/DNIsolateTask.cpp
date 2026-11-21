#include "stdafx.h"
#include "DNIsolateTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "Log.h"

CDNIsolateTask::CDNIsolateTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{

}

CDNIsolateTask::~CDNIsolateTask(void)
{
}

void CDNIsolateTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLMembership *pMembershipDB = NULL;
	CDNSQLWorld *pWorldDB = NULL;
	int nRet = ERROR_DB;

	switch (nSubCmd)
	{
	case QUERY_GETISOLATELIST:
		{
			TQGetIsolateList * pPacket = (TQGetIsolateList*)pData;
			TAGetIsolateList packet;
			memset(&packet, 0, sizeof(TAGetIsolateList));

			packet.nAccountDBID = pPacket->nAccountDBID;
			packet.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pPacket->cWorldSetID);
			if (pWorldDB)
			{
				packet.nRetCode = pWorldDB->QueryGetListBlockedCharacter(pPacket->biCharacterDBID, packet.cCount, packet.Isolate);
			}
			else
			{
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_GETISOLATELIST:%d] pWorldDB not found\r\n", pPacket->nAccountDBID, pPacket->biCharacterDBID, pPacket->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&packet, sizeof(packet) - sizeof(packet.Isolate) + (packet.cCount * sizeof(TIsolateItem)));
		}
		break;

	case QUERY_ADDISOLATE:
		{
			TQAddIsolate * pPacket = (TQAddIsolate*)pData;
			TAAddIsolate packet;
			memset(&packet, 0, sizeof(TAAddIsolate));

			packet.nAccountDBID = pPacket->nAccountDBID;
			_wcscpy(packet.IsolateItem.wszIsolateName, _countof(packet.IsolateItem.wszIsolateName), pPacket->wszIsolateName, (int)wcslen(pPacket->wszIsolateName));
			packet.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pPacket->cWorldSetID);
			if (pWorldDB)
			{
				bool bNeedDelFriend = false;
				packet.nRetCode = pWorldDB->QueryAddBlockedCharacter(pPacket->biCharacterDBID, pPacket->wszIsolateName, packet.IsolateItem.biIsolateCharacterDBID, bNeedDelFriend, ISOLATELISTMAX);
				if (packet.nRetCode == ERROR_NONE)
				{
					packet.cNeedDelFriend = bNeedDelFriend == true ? 1 : 0;
				}
			}
			else
			{
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_ADDISOLATE:%d] pWorldDB not found\r\n", pPacket->nAccountDBID, pPacket->biCharacterDBID, pPacket->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&packet, sizeof(packet));

		}
		break;

	case QUERY_DELISOLATE:
		{
			TQDelIsolate * pPacket = (TQDelIsolate*)pData;

			if (pPacket->cCount < 0 || pPacket->cCount >= ISOLATELISTMAX)
			{
				_DANGER_POINT();
				break;
			}

			TADelIsolate packet;
			memset(&packet, 0, sizeof(TADelIsolate));

			packet.nAccountDBID = pPacket->nAccountDBID;
			packet.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pPacket->cWorldSetID);
			if (pWorldDB)
			{
				packet.nRetCode = pWorldDB->QueryDelBlockedCharacter(pPacket->biCharacterDBID, pPacket->cCount, pPacket->biIsolateCharacterDBIDs);
				if (packet.nRetCode == ERROR_NONE)
				{
					if (pPacket->cCount < ISOLATELISTMAX && pPacket->cCount > 0)
					{
						packet.cCount = pPacket->cCount;
						memcpy(packet.biIsolateCharacterDBIDs, pPacket->biIsolateCharacterDBIDs, sizeof(INT64) * pPacket->cCount);
					}
					else
					{
						_DANGER_POINT();
						break;
					}
				}
			}
			else
			{
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[ADBID:%u CDBID:%lld] [QUERY_ADDISOLATE:%d] pWorldDB not found\r\n", pPacket->nAccountDBID, pPacket->biCharacterDBID, pPacket->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&packet, sizeof(packet) - sizeof(packet.biIsolateCharacterDBIDs) + (sizeof(INT64) * packet.cCount));
		}
		break;
	}
}
