#include "stdafx.h"
#include "DNMailTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "Log.h"

CDNMailTask::CDNMailTask(CDNConnection *pConnection)
: CDNMessageTask(pConnection)
{

}

CDNMailTask::~CDNMailTask(void)
{
}

void CDNMailTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLMembership *pMembershipDB = NULL;
	CDNSQLWorld *pWorldDB = NULL;
	int nRet = ERROR_DB;
	TQHeader *pHeader = (TQHeader*)pData;
	if( pHeader->nAccountDBID > 0)
	{
		if( g_pSPErrorCheckManager->bIsError(pHeader->nAccountDBID) == true )
		{
			//g_Log.Log(LogType::_ERROR, pHeader->cWorldSetID, pHeader->nAccountDBID, 0, 0, L"[SP_ERRORCHECK] Main:%d, Sub:%d\r\n", nMainCmd, nSubCmd);
			return;
		}
	}

	switch (nSubCmd)
	{
	case QUERY_GETCOUNTRECEIVEMAIL:
		{
			TQGetCountReceiveMail *pMail = (TQGetCountReceiveMail*)pData;

			TAGetCountReceiveMail Mail;
			memset(&Mail, 0, sizeof(TAGetCountReceiveMail));

			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				Mail.nRetCode = pWorldDB->QueryGetCountReceiveMail(pMail->biReceiverCharacterDBID, Mail.iTotalMailCount, Mail.iNotReadMailCount, Mail.i7DaysLeftMailCount);

				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETCOUNTRECEIVEMAIL:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETCOUNTRECEIVEMAIL] pWorldDB(%d) not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, sizeof(TAGetCountReceiveMail));
		}
		break;

	case QUERY_GETPAGERECEIVEMAIL:		// Received mail list paging
		{
			TQGetPageReceiveMail *pMail = (TQGetPageReceiveMail*)pData;

			TAGetPageReceiveMail Mail;
			memset(&Mail, 0, sizeof(TAGetPageReceiveMail));

			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				Mail.nRetCode = pWorldDB->QueryGetPageReceiveMail(pMail, &Mail);

				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETPAGERECEIVEMAIL:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETPAGERECEIVEMAIL:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			int nLen = sizeof(TAGetPageReceiveMail) - sizeof(Mail.sMail) + (sizeof(TMailBoxInfo) * Mail.cMailCount);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, nLen);
		}
		break;

	case QUERY_SENDMAIL:
		{
			TQSendMail *pMail = (TQSendMail*)pData;

			TASendMail Mail;
			memset(&Mail, 0, sizeof(TASendMail));

			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.Code = pMail->Code;
			Mail.biAttachCoin = pMail->biAttachCoin;
			Mail.nTax = pMail->nTax;
			Mail.cAttachItemCount = pMail->cAttachItemCount;
			memcpy(Mail.AttachItemArr, pMail->AttachItemArr, sizeof(Mail.AttachItemArr));

			Mail.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				Mail.nRetCode = pWorldDB->QuerySendMail(pMail, &Mail);

				switch (Mail.nRetCode)
				{
				case ERROR_NONE:
				case 103163: // Recipient character does not exist.
				case 103164: // Cannot send mail to a GM.
					break;

				default:
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_SENDMAIL:%d] Result:%d Receiver:%s\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode, pMail->wszReceiverCharacterName);
					break;
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_SENDMAIL:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, sizeof(TASendMail));
		}
		break;

	case QUERY_READMAIL:
		{
			TQReadMail *pMail = (TQReadMail*)pData;

			TAReadMail Mail;
			memset(&Mail, 0, sizeof(TAReadMail));

			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nMailDBID = pMail->iMailID;
			Mail.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				Mail.nRetCode = pWorldDB->QueryReadMail(pMail, &Mail);

				switch (Mail.nRetCode)
				{
				case ERROR_NONE:
				case 103166: // Mail does not exist.  
					break;

				default:
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_READMAIL:%d] Result:%d MailID:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode, pMail->iMailID);
					break;
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_READMAIL:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, sizeof(TAReadMail));
		}
		break;

	case QUERY_DELMAIL:
		{
			TQDelMail *pMail = (TQDelMail*)pData;

			TADelMail Mail;
			memset(&Mail, 0, sizeof(TADelMail));

			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				Mail.nRetCode = pWorldDB->QueryDelMail(pMail, &Mail);

				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_DELMAIL:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_DELMAIL:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, sizeof(TADelMail));
		}
		break;

	case QUERY_GETLISTMAILATTACHMENT:
		{
			TQGetListMailAttachment *pMail = (TQGetListMailAttachment*)pData;

			TAGetListMailAttachment Mail;
			memset(&Mail, 0, sizeof(TAGetListMailAttachment));

			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				Mail.nRetCode = pWorldDB->QueryGetListMailAttachment(pMail, &Mail);
				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETLISTMAILATTACHMENT:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);

				for (int i = 0; i < Mail.cCount; i++){
					if (Mail.MailAttachArray[i].cItemAttachCount <= 0) continue;
					pWorldDB->QueryGetListMailAttachmentByMail(Mail.MailAttachArray[i].nMailDBID, Mail.MailAttachArray[i]);
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETLISTMAILATTACHMENT:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			int nLen = sizeof(TAGetListMailAttachment) - sizeof(Mail.MailAttachArray) + (sizeof(DBPacket::TAttachMail) * Mail.cCount);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, nLen);
		}
		break;

	case QUERY_TAKEATTACHMAILLIST:
		{
			TQTakeAttachMailList *pMail = (TQTakeAttachMailList*)pData;

			TATakeAttachMailList Mail;
			memset(&Mail, 0, sizeof(TATakeAttachMailList));
			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nRetCode = ERROR_DB;
			Mail.cPageCount = pMail->cPageCount;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				for (int i = 0; i < pMail->cPageCount; i++){
					Mail.TakeMailList[i].nMailDBID = pMail->Info[i].nMailDBID;	// MailDBID

					if (pMail->Info[i].bAttachCoin)
					{
#ifdef PRE_ADD_JOINGUILD_SUPPORT
						Mail.nRetCode = pWorldDB->QueryTakeMailAttachCoin(pMail->Info[i].nMailDBID, pMail->nChannelID, pMail->nMapID, Mail.TakeMailList[i].biAttachCoin, pMail->nGuildDBID, Mail.TakeMailList[i].bAddGuildWare);
#else		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
						bool bAddGuildWare = false;
						Mail.nRetCode = pWorldDB->QueryTakeMailAttachCoin(pMail->Info[i].nMailDBID, pMail->nChannelID, pMail->nMapID, Mail.TakeMailList[i].biAttachCoin, 0, bAddGuildWare);
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
					}

					for (int j = 0; j < MAILATTACHITEMMAX; j++){
						if (pMail->Info[i].biSerial[j] == 0) continue;
#ifdef PRE_ADD_JOINGUILD_SUPPORT
						Mail.nRetCode = pWorldDB->QueryTakeMailAttachItem(pMail->Info[i].nMailDBID, pMail->Info[i].biSerial[j], pMail->nChannelID, pMail->nMapID, pMail->wszIP, Mail.TakeMailList[i].TakeItem[j], pMail->nGuildDBID, Mail.TakeMailList[i].bAddGuildWare);
#else		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
						bool bAddGuildWare = false;
						Mail.nRetCode = pWorldDB->QueryTakeMailAttachItem(pMail->Info[i].nMailDBID, pMail->Info[i].biSerial[j], pMail->nChannelID, pMail->nMapID, pMail->wszIP, Mail.TakeMailList[i].TakeItem[j], 0, bAddGuildWare);
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
						Mail.TakeMailList[i].TakeItem[j].cSlotIndex = pMail->Info[i].cSlotIndex[j];
					}
				}

				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_TAKEATTACHMAILLIST:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_TAKEATTACHMAILLIST:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			int nLen = sizeof(TATakeAttachMailList) - sizeof(Mail.TakeMailList) + (sizeof(DBPacket::TTakeAttachInfo) * Mail.cPageCount);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, nLen);
		}
		break;

	case QUERY_TAKEATTACHMAIL:
		{
			TQTakeAttachMail *pMail = (TQTakeAttachMail*)pData;

			TATakeAttachMail Mail;
			memset(&Mail, 0, sizeof(TATakeAttachMail));
			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nRetCode = ERROR_DB;
			Mail.TakeMail.nMailDBID = pMail->Info.nMailDBID;
			Mail.cAttachSlotIndex = pMail->Info.cAttachSlotIndex;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				switch (pMail->Info.cAttachSlotIndex)
				{
				case -1:
					{
						if (pMail->Info.bAttachCoin)
						{
#ifdef PRE_ADD_JOINGUILD_SUPPORT
							Mail.nRetCode = pWorldDB->QueryTakeMailAttachCoin(pMail->Info.nMailDBID, pMail->nChannelID, pMail->nMapID, Mail.TakeMail.biAttachCoin, pMail->nGuildDBID, Mail.TakeMail.bAddGuildWare);
#else		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
							bool bAddGuildWare = false;
							Mail.nRetCode = pWorldDB->QueryTakeMailAttachCoin(pMail->Info.nMailDBID, pMail->nChannelID, pMail->nMapID, Mail.TakeMail.biAttachCoin, 0, bAddGuildWare);
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
						}
						for (int j = 0; j < MAILATTACHITEMMAX; j++){
							if (pMail->Info.biSerial[j] == 0) continue;
#ifdef PRE_ADD_JOINGUILD_SUPPORT
							Mail.nRetCode = pWorldDB->QueryTakeMailAttachItem(pMail->Info.nMailDBID, pMail->Info.biSerial[j], pMail->nChannelID, pMail->nMapID, pMail->wszIP, Mail.TakeMail.TakeItem[j], pMail->nGuildDBID, Mail.TakeMail.bAddGuildWare);
#else		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
							bool bAddGuildWare = false;
							Mail.nRetCode = pWorldDB->QueryTakeMailAttachItem(pMail->Info.nMailDBID, pMail->Info.biSerial[j], pMail->nChannelID, pMail->nMapID, pMail->wszIP, Mail.TakeMail.TakeItem[j], 0, bAddGuildWare);
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
							Mail.TakeMail.TakeItem[j].cSlotIndex = pMail->Info.cSlotIndex[j];
						}
					}
					break;

				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
					{
#ifdef PRE_ADD_JOINGUILD_SUPPORT
						Mail.nRetCode = pWorldDB->QueryTakeMailAttachItem(pMail->Info.nMailDBID, pMail->Info.biSerial[pMail->Info.cAttachSlotIndex],  
							pMail->nChannelID, pMail->nMapID, pMail->wszIP, Mail.TakeMail.TakeItem[pMail->Info.cAttachSlotIndex], pMail->nGuildDBID, Mail.TakeMail.bAddGuildWare);
#else		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
						bool bAddGuildWare = false;
						Mail.nRetCode = pWorldDB->QueryTakeMailAttachItem(pMail->Info.nMailDBID, pMail->Info.biSerial[pMail->Info.cAttachSlotIndex],  
							pMail->nChannelID, pMail->nMapID, pMail->wszIP, Mail.TakeMail.TakeItem[pMail->Info.cAttachSlotIndex], 0, bAddGuildWare);
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
						Mail.TakeMail.TakeItem[pMail->Info.cAttachSlotIndex].cSlotIndex = pMail->Info.cSlotIndex[pMail->Info.cAttachSlotIndex];
					}
					break;
				}

				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_TAKEATTACHMAIL:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_TAKEATTACHMAIL:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, sizeof(TATakeAttachMail));
		}
		break;

	case QUERY_SENDSYSTEMMAIL:
		{
			TQSendSystemMail *pMail = (TQSendSystemMail*)pData;

			TASendSystemMail Mail;
			memset(&Mail, 0, sizeof(TASendSystemMail));
			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nRetCode = ERROR_DB;
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
			Mail.biReceiverCharacterDBID = pMail->biReceiverCharacterDBID;
			Mail.cWorldSetID = pMail->cWorldSetID;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				Mail.nRetCode = pWorldDB->QuerySendSystemMail(pMail, &Mail);

				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_SENDSYSTEMMAIL:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_SENDSYSTEMMAIL:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, sizeof(TASendSystemMail));
		}
		break;

#ifdef PRE_ADD_BEGINNERGUILD
	case QUERY_GETWILLSENDMAIL:
		{
			TQGetWillMails * pPacket = (TQGetWillMails*)pData;

			TAGetWillMails packet;
			memset(&packet, 0, sizeof(TAGetWillMails));

			packet.nAccountDBID = pPacket->nAccountDBID;
			packet.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pPacket->cWorldSetID);
			if (pWorldDB)
			{
				packet.nRetCode = pWorldDB->QueryGetWillSendMail(pPacket, &packet);

				if (packet.nRetCode != ERROR_NONE)
				{
					if (packet.nRetCode == WillSendMail::Common::WillSendMail_NoRecord_RetCode)
						packet.nRetCode = ERROR_NONE;
					else
						g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_GETWILLSENDMAIL:%d] Result:%d\r\n", pPacket->nAccountDBID, pPacket->cWorldSetID, packet.nRetCode);
				}
			}
			else
			{
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_GETWILLSENDMAIL:%d] pWorldDB not found\r\n", pPacket->nAccountDBID, pPacket->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&packet, sizeof(TAGetWillMails) - sizeof(packet.Mails) + (sizeof(WillSendMail::TWillSendInfo) * packet.cCount));
		}
		break;

	case QUERY_DELWILLSENDMAIL:
		{
			TQDelWillMail * pPacket = (TQDelWillMail*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pPacket->cWorldSetID);
			if (pWorldDB)
			{
				int nRetCode = pWorldDB->QueryDelWillSendMail(pPacket->biCharacterDBID, pPacket->nMailID);

				if (nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_DELWILLSENDMAIL:%d] Result:%d\r\n", pPacket->nAccountDBID, pPacket->cWorldSetID, nRetCode);
			}
			else
			{
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_DELWILLSENDMAIL:%d] pWorldDB not found\r\n", pPacket->nAccountDBID, pPacket->cWorldSetID);
			}
		}
		break;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

#if defined(PRE_ADD_CADGE_CASH)
	case QUERY_SENDWISHMAIL:
		{
			TQSendWishMail *pMail = (TQSendWishMail*)pData;

			TASendWishMail Mail;
			memset(&Mail, 0, sizeof(TASendWishMail));

			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				Mail.nRetCode = pWorldDB->QuerySendWishMail(pMail, &Mail);

				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_SENDWISHMAIL:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_SENDWISHMAIL:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, sizeof(Mail));
		}
		break;

	case QUERY_READWISHMAIL:
		{
			TQReadWishMail *pMail = (TQReadWishMail*)pData;

			TAReadWishMail Mail;
			memset(&Mail, 0, sizeof(TAReadWishMail));

			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nMailDBID = pMail->nMailID;
			Mail.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB){
				Mail.nRetCode = pWorldDB->QueryReadWishMail(pMail, &Mail);

				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_READWISHMAIL:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_READWISHMAIL:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, sizeof(Mail));
		}
		break;
#endif	// #if defined(PRE_ADD_CADGE_CASH)

#if defined(PRE_SPECIALBOX)
		// SpecialBox
	case QUERY_ADDEVENTREWARD:
		{
			TQAddEventReward *pSpecialBox = (TQAddEventReward*)pData;

			TAAddEventReward SpecialBox;
			memset(&SpecialBox, 0, sizeof(TAAddEventReward));

			SpecialBox.nAccountDBID = pSpecialBox->nAccountDBID;
			SpecialBox.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				SpecialBox.nRetCode = pMembershipDB->QueryAddEventReward(pSpecialBox, &SpecialBox);

				if (SpecialBox.nRetCode == ERROR_NONE)
					pMembershipDB->QueryGetCountEventReward(pSpecialBox->nAccountDBID, pSpecialBox->biCharacterDBID, SpecialBox.nEventTotalCount);
				else
					g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_ADDEVENTREWARD:%d] Result:%d\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID, SpecialBox.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_ADDEVENTREWARD] pMembershipDB not found\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&SpecialBox, sizeof(SpecialBox));
		}
		break;

	case QUERY_GETCOUNTEVENTREWARD:
		{
			TQGetCountEventReward *pSpecialBox = (TQGetCountEventReward*)pData;

			TAGetCountEventReward SpecialBox;
			memset(&SpecialBox, 0, sizeof(TAGetCountEventReward));

			SpecialBox.nAccountDBID = pSpecialBox->nAccountDBID;
			SpecialBox.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				SpecialBox.nRetCode = pMembershipDB->QueryGetCountEventReward(pSpecialBox->nAccountDBID, pSpecialBox->biCharacterDBID, SpecialBox.nEventTotalCount);

				if (SpecialBox.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETCOUNTEVENTREWARD:%d] Result:%d\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID, SpecialBox.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETCOUNTEVENTREWARD] pMembershipDB not found\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&SpecialBox, sizeof(SpecialBox));
		}
		break;

	case QUERY_GETLISTEVENTREWARD:
		{
			TQGetListEventReward *pSpecialBox = (TQGetListEventReward*)pData;

			TAGetListEventReward SpecialBox;
			memset(&SpecialBox, 0, sizeof(TAGetListEventReward));

			SpecialBox.nAccountDBID = pSpecialBox->nAccountDBID;
			SpecialBox.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				SpecialBox.nRetCode = pMembershipDB->QueryGetListEventReward(pSpecialBox, &SpecialBox);

				if (SpecialBox.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETLISTEVENTREWARD:%d] Result:%d\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID, SpecialBox.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETLISTEVENTREWARD] pMembershipDB not found\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&SpecialBox, sizeof(SpecialBox));
		}
		break;

	case QUERY_GETLISTEVENTREWARDITEM:
		{
			TQGetListEventRewardItem *pSpecialBox = (TQGetListEventRewardItem*)pData;

			TAGetListEventRewardItem SpecialBox;
			memset(&SpecialBox, 0, sizeof(TAGetListEventRewardItem));

			SpecialBox.nAccountDBID = pSpecialBox->nAccountDBID;
			SpecialBox.nEventRewardID = pSpecialBox->nEventRewardID;
			SpecialBox.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				SpecialBox.nRetCode = pMembershipDB->QueryGetListEventRewardItem(pSpecialBox, &SpecialBox);

				if (SpecialBox.nRetCode == ERROR_NONE){
#if defined(PRE_MOD_SELECT_CHAR)
					pMembershipDB->QueryGetCharacterSlotCount(pSpecialBox->nAccountDBID, 0, 0, SpecialBox.cCharacterCount);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
					pMembershipDB->QueryGetCharacterSlotCount(0, 0, pSpecialBox->biCharacterDBID, SpecialBox.cCharacterCount);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
				}
				else
					g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETLISTEVENTREWARDITEM:%d] Result:%d\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID, SpecialBox.nRetCode);
			}
			else{
				g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GETLISTEVENTREWARDITEM] pMembershipDB not found\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&SpecialBox, sizeof(SpecialBox));
		}
		break;

	case QUERY_ADDEVENTREWARDRECEIVER:
		{
			TQAddEventRewardReceiver *pSpecialBox = (TQAddEventRewardReceiver*)pData;

			TAAddEventRewardReceiver SpecialBox;
			memset(&SpecialBox, 0, sizeof(TAAddEventRewardReceiver));

			SpecialBox.nAccountDBID = pSpecialBox->nAccountDBID;
			SpecialBox.nEventRewardID = pSpecialBox->nEventRewardID;
			SpecialBox.nItemID = pSpecialBox->nItemID;
			SpecialBox.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				SpecialBox.nRetCode = pMembershipDB->QueryAddEventRewardReceiver(pSpecialBox->nAccountDBID, pSpecialBox->biCharacterDBID, pSpecialBox->nEventRewardID);

				switch (SpecialBox.nRetCode)
				{
				case ERROR_NONE:
				case 101203:	// Character has already received event reward
					break;

				default:
					g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_ADDEVENTREWARDRECEIVER:%d] Result:%d\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID, SpecialBox.nRetCode);
					break;
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pSpecialBox->cWorldSetID, pSpecialBox->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_ADDEVENTREWARDRECEIVER] pMembershipDB not found\r\n", pSpecialBox->nAccountDBID, pSpecialBox->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&SpecialBox, sizeof(SpecialBox));
		}
		break;

#endif	// #if defined(PRE_SPECIALBOX)
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	case QUERY_GUILDSUPPORT_REWARDINFO:
		{
			TQGuildSupportRewardInfo * pPacket = (TQGuildSupportRewardInfo*)pData;
			
			TAGuildSupportRewardInfo packet;
			memset(&packet, 0, sizeof(TAGuildSupportRewardInfo));
			
			packet.nRetCode = ERROR_DB;
			packet.bWasGuildSupportRewardFlag = true;
			packet.cJoinGuildLevel = 0;
			packet.nAccountDBID = pPacket->nAccountDBID;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pPacket->cWorldSetID);
			if (pWorldDB)
			{
				packet.nRetCode = pWorldDB->QueryGetGuildSupportRewardInfo(pPacket, &packet);

				if (packet.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GUILDSUPPORT_REWARDINFO:%d] Result:%d\r\n", pPacket->nAccountDBID, pPacket->cWorldSetID, packet.nRetCode);
			}
			else
			{
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_GUILDSUPPORT_REWARDINFO:%d] pWorldDB not found\r\n", pPacket->nAccountDBID, pPacket->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&packet, sizeof(TAGuildSupportRewardInfo));
		}
		break;

	case QUERY_SENDGUILDMAIL:
		{
			TQSendGuildMail *pMail = (TQSendGuildMail*)pData;

			TASendGuildMail Mail;
			memset(&Mail, 0, sizeof(TASendGuildMail));
			Mail.nAccountDBID = pMail->nAccountDBID;
			Mail.nRetCode = ERROR_DB;
			Mail.nReceiverAccountDBID = pMail->nReceiverAccountDBID;
			Mail.biReceiverCharacterDBID = pMail->biReceiverCharacterDBID;
			Mail.cWorldSetID = pMail->cWorldSetID;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pMail->cWorldSetID);
			if (pWorldDB)
			{
				Mail.nRetCode = pWorldDB->QuerySendGuildMail(pMail, &Mail);

				if (Mail.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_SENDGUILDMAIL:%d] Result:%d\r\n", pMail->nAccountDBID, pMail->cWorldSetID, Mail.nRetCode);
			}
			else
			{
				g_Log.Log(LogType::_ERROR, pMail->cWorldSetID, pMail->nAccountDBID, 0, 0, L"[ADBID:%u] [QUERY_SENDGUILDMAIL:%d] pWorldDB not found\r\n", pMail->nAccountDBID, pMail->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Mail, sizeof(TASendGuildMail));
		}
		break;
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
	}
}
