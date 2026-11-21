#include "stdafx.h"
#include "DNConnection.h"
#include "DNSQLWorld.h"
#include "DNSQLMembership.h"
#include "DNSQLConnectionManager.h"
#include "Log.h"
#include "DNExtManager.h"
#include "Util.h"

#include "DNAdjustTask.h"
#include "DNAuthTask.h"
#include "DNStatusTask.h"
#include "DNEtcTask.h"
#include "DNQuestTask.h"
#include "DNMissionTask.h"
#include "DNAppellationTask.h"
#include "DNSkillTask.h"
#include "DNFriendTask.h"
#include "DNIsolateTask.h"
#include "DNPvPTask.h"
#include "DNDarkLairTask.h"
#include "DNGuildTask.h"
#include "DNMailTask.h"
#include "DNMarketTask.h"
#include "DNItemTask.h"
#include "DNCashShopTask.h"
#include "DNReputationTask.h"
#include "DNMasterSystemTask.h"
#include "DNSecondarySkillTask.h"
#include "DNFarmTask.h"
#include "DNLogTask.h"
#include "DNJobSystemTask.h"
#include "DNGuildRecruitSystemTask.h"
#if defined (PRE_ADD_DONATION)
#include "DNDonationTask.h"
#endif // #if defined (PRE_ADD_DONATION)
#if defined( PRE_PARTY_DB )
#include "DNPartyTask.h"
#endif // #if defined( PRE_PARTY_DB )
#if defined (PRE_ADD_BESTFRIEND)
#include "DNBestFriendTask.h"
#endif
#if defined( PRE_PRIVATECHAT_CHANNEL )
#include "DNPrivateChatChannelTask.h"
#endif
#if defined( PRE_ALTEIAWORLD_EXPLORE )
#include "DNAlteiaWorldTask.h"
#endif
#if defined( PRE_ADD_STAMPSYSTEM )
#include "DNStampSystemTask.h"
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined( PRE_ADD_DWC )
#include "DNDWCTask.h"
#endif // #if defined( PRE_ADD_DWC )

CDNConnection::CDNConnection(void): CConnection()
{
#if defined(_FINAL_BUILD)
	Init(1024 * 1024, 1024 * 1024);
#else	// #if defined(_FINAL_BUILD)
	Init(1024 * 100, 1024 * 100);
#endif	// #if defined(_FINAL_BUILD)

	m_lDBQueueRemainSize = 0;

	RegisterMessageTask(MAINCMD_MSGADJUST, new CDNAdjustTask(this));
	RegisterMessageTask(MAINCMD_AUTH, new CDNAuthTask(this));
	RegisterMessageTask(MAINCMD_STATUS, new CDNStatusTask(this));
	RegisterMessageTask(MAINCMD_ETC, new CDNEtcTask(this));
	RegisterMessageTask(MAINCMD_QUEST, new CDNQuestTask(this));
	RegisterMessageTask(MAINCMD_MISSION, new CDNMissionTask(this));
	RegisterMessageTask(MAINCMD_APPELLATION, new CDNAppellationTask(this));
	RegisterMessageTask(MAINCMD_SKILL, new CDNSkillTask(this));
	RegisterMessageTask(MAINCMD_FRIEND, new CDNFriendTask(this));
	RegisterMessageTask(MAINCMD_ISOLATE, new CDNIsolateTask(this));
	RegisterMessageTask(MAINCMD_PVP, new CDNPvPTask(this));
	RegisterMessageTask(MAINCMD_DARKLAIR, new CDNDarkLairTask(this));
	RegisterMessageTask(MAINCMD_GUILD, new CDNGuildTask(this));
	RegisterMessageTask(MAINCMD_MAIL, new CDNMailTask(this));
	RegisterMessageTask(MAINCMD_MARKET, new CDNMarketTask(this));
	RegisterMessageTask(MAINCMD_ITEM, new CDNItemTask(this));
	RegisterMessageTask(MAINCMD_CASH, new CDNCashShopTask(this));
	RegisterMessageTask(MAINCMD_LOG, new CDNLogTask(this));
	RegisterMessageTask(MAINCMD_REPUTATION, new CDNReputationTask(this));
	RegisterMessageTask(MAINCMD_MASTERSYSTEM, new CDNMasterSystemTask(this));
	RegisterMessageTask(MAINCMD_SECONDARYSKILL, new CDNSecondarySkillTask(this));
	RegisterMessageTask(MAINCMD_FARM, new CDNFarmTask(this));
	RegisterMessageTask(MAINCMD_JOBSYSTEM, new CDNJobSystemTask(this));
	RegisterMessageTask(MAINCMD_GUILDRECRUIT, new CDNGuildRecruitSystemTask(this));
#if defined (PRE_ADD_DONATION)
	RegisterMessageTask(MAINCMD_DONATION, new CDNDonationTask(this));
#endif // #if defined (PRE_ADD_DONATION)
#if defined( PRE_PARTY_DB )
	RegisterMessageTask(MAINCMD_PARTY, new CDNPartyTask(this) );
#endif // #if defined( PRE_PARTY_DB )
#if defined (PRE_ADD_BESTFRIEND)
	RegisterMessageTask(MAINCMD_BESTFRIEND, new CDNBestFriendTask(this) );
#endif // #if defined (PRE_ADD_BESTFRIEND)
#if defined (PRE_PRIVATECHAT_CHANNEL)
	RegisterMessageTask(MAINCMD_PRIVATECHATCHANNEL, new CDNPrivateChatChannelTask(this) );
#endif // #if defined (PRE_ADD_BESTFRIEND)
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	RegisterMessageTask(MAINCMD_ALTEIAWORLD, new CDNAlteiaWorldTask(this));
#endif
#if defined( PRE_ADD_STAMPSYSTEM )
	RegisterMessageTask(MAINCMD_STAMPSYSTEM, new CDNStampSystemTask(this));
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined( PRE_ADD_DWC )
	RegisterMessageTask(MAINCMD_DWC, new CDNDWCTask(this));
#endif // #if defined( PRE_ADD_DWC )
}

CDNConnection::~CDNConnection(void)
{
	for each (std::map<unsigned char, CDNMessageTask*>::value_type v in m_MessageTasks)
	{
		delete v.second;
	}
}

int CDNConnection::MessageProcess(int iMainCmd, int iSubCmd, char *pData, int iLen)
{
	return 0;
}

void CDNConnection::DBMessageProcess(char *pData, int nThreadID)
{
	DNTPacket *pPacket = (DNTPacket*)pData;

	CDNMessageTask* pTask = GetMessageTask(pPacket->cMainCmd);
	if (!pTask)
		return;

	pTask->OnRecvMessage(nThreadID, pPacket->cMainCmd, pPacket->cSubCmd, pPacket->buf);
}

void CDNConnection::QueryResultError(UINT nAccountDBID, int nRetCode, char cMainCmd, char cSubCmd)
{
	TADBResultError Result;
	Result.nAccountDBID = nAccountDBID;
	Result.nRetCode = nRetCode;
	Result.cMainCmd = cMainCmd;
	Result.cSubCmd = cSubCmd;
	AddSendData(MAINCMD_ETC, QUERY_DBRESULTERROR, (char*)&Result, sizeof(TADBResultError));

	// Register the one that errored here.
	g_pSPErrorCheckManager->Add(nAccountDBID);
	g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryResultError Main:%d Sub:%d nRet:%d]\r\n", cMainCmd, cSubCmd, nRetCode);
}

int CDNConnection::CashItemDBProcess(CDNSQLMembership *pMembershipDB, CDNSQLWorld *pWorldDB, int nWorldSetID, int nAccountDBID, INT64 biCharacterDBID, int nMapID, int nChannelID, bool bPetalPurchase, TCashItemBase &BuyItem, int nPrice, 
									 INT64 biPurchaseOrderID, int nAddMaterializedItemCode, const WCHAR* pwszIP, INT64 biSenderCharacterDBID/* = 0*/, bool bGift/* = false*/, char cPayMethodCode/* = DBDNWorldDef::PayMethodCode::Cash*/)
{
	int nRet = ERROR_DB;

	switch (g_pExtManager->GetItemMainType(BuyItem.CashItem.nItemID))
	{
	case ITEMTYPE_CHARACTER_SLOT:
		{
			char cMaxCount = 0;
			nRet = pMembershipDB->QueryModCharacterSlotCount(biCharacterDBID, 1, cMaxCount);	// 34: Character slot count limit
		}
		break;

	default:
		{
			nRet = pWorldDB->CashItemDBQuery(nAccountDBID, biCharacterDBID, nMapID, nChannelID, bPetalPurchase, BuyItem, nPrice, biPurchaseOrderID, nAddMaterializedItemCode, pwszIP, biSenderCharacterDBID, bGift, cPayMethodCode);
		}
		break;
	}

	return nRet;
}

CDNMessageTask* CDNConnection::GetMessageTask(unsigned char cMainCmd)
{
	std::map<unsigned char, CDNMessageTask*>::iterator it = m_MessageTasks.find(cMainCmd);
	if (it == m_MessageTasks.end())
		return NULL;

	return it->second;
}

void CDNConnection::RegisterMessageTask(eServerMainCmd cMainCmd, CDNMessageTask* pTask)
{
	m_MessageTasks.insert(std::make_pair((unsigned char)cMainCmd, pTask));
}
