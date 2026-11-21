
#include "stdafx.h"
#include "DNBestFriend.h"
#include "DNUserSession.h"
#include "DNDBConnection.h"
#include "TimeSet.h"
#include "DNWorldUserState.h"
#include "DNGameDataManager.h"
#include "DNAppellation.h"
#include "DNCashRepository.h"
#if defined(_VILLAGESERVER)
#include "DNUserSessionManager.h"
#endif

#if defined (PRE_ADD_BESTFRIEND)

CDNBestFriend::CDNBestFriend(CDNUserSession* pSession)
{
	m_pUserSession = pSession;

	Reset();
}

CDNBestFriend::~CDNBestFriend()
{

}

void CDNBestFriend::LoadData(const TAGetBestFriend* pData)
{
	m_Info = pData->Info;

	if (!pData->bSend)
		return;

	CheckAndSendData();
} 

void CDNBestFriend::CheckAndSendData()
{
	if (m_Info.biCharacterDBID == 0)
		return;
	
	// Calculate registered period
	CalcRegistTime();

	// Check reward
	CheckReward();

#if defined (_VILLAGESERVER)
	// Deletion check
	CheckStatus();
#endif

	// Send best friend info
	SendGetBestFriendInfo();
}

void CDNBestFriend::SendGetBestFriendInfo()
{
	sWorldUserState State;
	if (g_pWorldUserState->GetUserState(m_Info.wszName, m_Info.biCharacterDBID, &State))
	{
		m_Info.Location.cServerLocation = State.nLocationState;
		m_Info.Location.nChannelID = State.nLocationState == _LOCATION_VILLAGE ? State.nChannelID : -1;
		m_Info.Location.nMapIdx = State.nMapIdx;
	}
	else
		m_Info.Location.cServerLocation = _LOCATION_NONE;

#if defined(_VILLAGESERVER)
	CDNUserSession* pSession = g_pUserSessionManager->FindUserSessionByName(m_Info.wszName);
	if (pSession)
		m_Info.nSessionID = pSession->GetSessionID();
#endif // #if defined(_VILLAGESERVER)

	m_pUserSession->SendGetBestFriend(ERROR_NONE, m_Info);
}

bool CDNBestFriend::IsRegistered()
{
	if (m_Info.biCharacterDBID != 0)
		return true;

	return false;
}

void CDNBestFriend::CalcRegistTime()
{
	CTimeSet LocalSet;
	CTimeSet StartSet(m_Info.tStartDate, true);
	INT64 biRegistSecond = LocalSet - StartSet;
	if (biRegistSecond <= 0)
		biRegistSecond = 1;

	m_Info.nDay = (int)(biRegistSecond / (60 * 60 * 24)) + 1;
}

void CDNBestFriend::CheckStatus(bool bCheat)
{
	CTimeSet LocalSet;
	CTimeSet EndSet(m_Info.tEndDate, true);
	INT64 biDiffSecond = LocalSet - EndSet;

	if (BESTFRIEND_CLOSETIME > biDiffSecond)
		m_Info.biWaitingTimeForDelete = BESTFRIEND_CLOSETIME - biDiffSecond;

	if (m_Info.bStatus || bCheat)
	{
		if (biDiffSecond > BESTFRIEND_CLOSETIME || bCheat)
		{
			m_pUserSession->GetDBConnection()->QueryCloseBestFriend(m_pUserSession);
			return;
		}
	}
}

void CDNBestFriend::ChangeName(const WCHAR* pwszName)
{
	_wcscpy(m_Info.wszName, _countof(m_Info.wszName), pwszName, (int)wcslen(pwszName));

	SendGetBestFriendInfo();
}

void CDNBestFriend::ChangeLevel(BYTE cLevel)
{
	m_Info.cLevel = cLevel;

	SendGetBestFriendInfo();
}

void CDNBestFriend::ChangeMemo(bool bMine, const WCHAR* pwszName)
{
	if(bMine)
		_wcscpy(m_Info.wszMyMemo, _countof(m_Info.wszMyMemo), pwszName, (int)wcslen(pwszName));	
	else
		_wcscpy(m_Info.wszFriendMemo, _countof(m_Info.wszFriendMemo), pwszName, (int)wcslen(pwszName));	
}

void CDNBestFriend::ExecuteReward(int nItemID)
{
	m_pUserSession->ModCommonVariableData(CommonVariable::Type::BESTFRIEND_GIFTBOXID, 0);

	TItemData* pItemData = g_pDataManager->GetItemData(nItemID);
	if (!pItemData)
	{
		_DANGER_POINT();
		g_Log.Log(LogType::_BESTFRIEND, m_pUserSession, L"CDNBestFriend::ExecuteReward Invalid Item [%d] \r\n", nItemID);
		return;
	}

	// Item
	CDNUserItem* pItem = m_pUserSession->GetItem();
	pItem->CreateCashInvenItem(pItemData->nTypeParam[0], 1, DBDNWorldDef::AddMaterializedItem::Trigger, -1, 0, 0, 0,  DBDNWorldDef::PayMethodCode::BestFriend);

	// Title
	int nAppellationID = pItemData->nTypeParam[1] - 1;	
	if( m_pUserSession->GetAppellation()->IsExistAppellation( nAppellationID ) ) 
	{
		// Bug if best friend title already exists
		g_Log.Log(LogType::_BESTFRIEND, m_pUserSession, L"CDNBestFriend::ExecuteReward Exist Appellation [%d] \r\n", nAppellationID);
	}
	else
		m_pUserSession->GetAppellation()->RequestAddAppellation( nAppellationID );

	// Buff skill
	int nBuffItemID = pItemData->nTypeParam[2];
	TItemData* pBuffItemData = g_pDataManager->GetItemData(nBuffItemID);
	if (!pBuffItemData)
	{
		_DANGER_POINT();
		g_Log.Log(LogType::_BESTFRIEND, m_pUserSession, L"CDNBestFriend::ExecuteReward Invalid BuffItem [%d] \r\n", nBuffItemID);
		return;
	}
	CSUseItem pPacket;
	memset(&pPacket, 0, sizeof(pPacket));
	pPacket.biInvenSerial = m_pUserSession->GetItem()->MakeItemSerial();
	m_pUserSession->GetItem()->ApplyPartyEffectSkillItemData(&pPacket, pBuffItemData, m_pUserSession->GetSessionID(), EffectSkillNameSpace::ShowEffectType::NONEEFFECT, true, false);
}

void CDNBestFriend::CheckReward()
{
	INT64 biItemID = 0;
	m_pUserSession->GetCommonVariableDataValue(CommonVariable::Type::BESTFRIEND_GIFTBOXID, biItemID);
	if (biItemID == 0)
		return;

	ExecuteReward((int)biItemID);
}

void CDNBestFriend::RemoveReward()
{
	if (m_Info.biCharacterDBID != 0)
		return;

	// Delete best friend gift box
	m_pUserSession->GetItem()->UseCashItemByType(ITEMTYPE_GIFTBOX, 1, true);

	// Delete best friend title		
	m_pUserSession->GetAppellation()->DelAppellation( AppellationType::Type::BestFriend, true );	//Delete title	

	// Delete best friend buff
	m_pUserSession->GetEffectRepository()->DelEffectItemType( ITEMTYPE_BESTFRIENDBUFFITEM );
	std::vector<TEffectSkillData> vEffectSkill;
	vEffectSkill.clear();
	m_pUserSession->GetItem()->GetEffectSkillItem( vEffectSkill );
	m_pUserSession->SendEffectSkillItemData(m_pUserSession->GetSessionID(), vEffectSkill, false);
}

void CDNBestFriend::Reset()
{
	memset (&m_Info, 0, sizeof(m_Info));
	m_biRegistSerial = 0;
}

void CDNBestFriend::Close()
{
	Reset();
	RemoveReward();
}

#endif // #if defined (PRE_ADD_BESTFRIEND)