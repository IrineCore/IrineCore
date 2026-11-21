#include "stdafx.h"
#include "DNFieldBlock.h"
#include "DNUserSession.h"
#include "Util.h"
#include "Log.h"

CDNFieldBlock::CDNFieldBlock(void)
{
	m_nMinX = m_nMinZ = m_nMaxX = m_nMaxZ = m_nXPoint = m_nZPoint = 0;
	Final();
}

CDNFieldBlock::~CDNFieldBlock(void)
{
	Final();
}

bool CDNFieldBlock::Init(int nXPoint, int nZPoint, int nMinX, int nMinZ)
{
	Final();

	m_nXPoint = nXPoint;
	m_nZPoint = nZPoint;
	m_nMinX = nMinX;
	m_nMinZ = nMinZ;
	m_nMaxX = m_nMinX + FIELDBLOCKRANGE;
	m_nMaxZ = m_nMinZ + FIELDBLOCKRANGE;

	return true;
}

bool CDNFieldBlock::Final()
{
	m_pObjectList.clear();
	return true;
}

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
void CDNFieldBlock::GetWaitUserList( std::list<CDNUserSession*>& UserList, int iLimitLevel/*=-1*/, BYTE cPvPChannelType/* = static_cast<BYTE>(PvPCommon::RoomType::max)*/ )
#else		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
void CDNFieldBlock::GetWaitUserList( std::list<CDNUserSession*>& UserList, int iLimitLevel/*=-1*/ )
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
{
	ScopeLock<CSyncLock> lock(m_ObjectLock);

	for( TObjectList::iterator iter=m_pObjectList.begin() ; iter != m_pObjectList.end(); ++iter )
	{
		if( g_IDGenerator.IsUser((*iter)->GetObjectID()) == false )
			continue;

		CDNUserSession* pSession = static_cast<CDNUserSession*>(*iter);
		if( pSession->GetPvPIndex() > 0 )
			continue;
		if( pSession->bIsLadderUser() == true )
			continue;
		if( iLimitLevel > 0 )
		{
			if( pSession->GetLevel() < iLimitLevel )
				continue;
		}
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		if (cPvPChannelType != PvPCommon::RoomType::max && pSession->GetPvPChannelType() != cPvPChannelType)
			continue;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		UserList.push_back( pSession );
	}
}

int CDNFieldBlock::SendFieldMessage (CDNBaseObject *pSender, USHORT wMsg, TBaseData *pSenderData, TParamData *pParamData)
{
	if (m_pObjectList.empty()) return -1;

	ScopeLock<CSyncLock> lock(m_ObjectLock);

	CDNBaseObject *pBaseObject = NULL;
	TObjectList::iterator iter;

	for (iter = m_pObjectList.begin(); iter != m_pObjectList.end(); ){
		pBaseObject = *iter;
		if (pBaseObject) {	// Seems more checks are needed
			//Also send this to self
			if( wMsg == FM_GUILDWARWINSKILL || pBaseObject != pSender) {
				pBaseObject->FieldProcess(pSender, wMsg, pSenderData, pParamData);
			}
			++iter;
		}
		else {
			iter = m_pObjectList.erase(iter);
		}
	}

	return 0;
}

int CDNFieldBlock::SendUserFieldMessage (CDNBaseObject *pSender, USHORT wMsg, TBaseData *pSenderData, TParamData *pParamData)
{
	if (m_pObjectList.empty()) return -1;

	ScopeLock<CSyncLock> lock(m_ObjectLock);

	CDNBaseObject *pBaseObject = NULL;
	TObjectList::iterator iter;

	for (iter = m_pObjectList.begin(); iter != m_pObjectList.end(); ){
		if (g_IDGenerator.IsNpc((*iter)->GetObjectID()))
		{
			++iter;
			continue;	// Process user only
		}

		pBaseObject = *iter;

		if (pBaseObject){	// Seems more checks are needed
			if (pBaseObject != pSender){
				pBaseObject->FieldProcess(pSender, wMsg, pSenderData, pParamData);
			}
			++iter;
		}
		else {
			iter = m_pObjectList.erase(iter);
		}
	}

	return 0;
}

int CDNFieldBlock::SendPvPLobbyMessage( CDNBaseObject *pSender, USHORT wMsg, TBaseData *pSenderData, TParamData *pParamData )
{
	if (m_pObjectList.empty()) return -1;

	ScopeLock<CSyncLock> lock(m_ObjectLock);

	for( TObjectList::iterator iter = m_pObjectList.begin(); iter != m_pObjectList.end() ; )
	{
		if (g_IDGenerator.IsNpc((*iter)->GetObjectID())) 
		{
			++iter;
			continue;	// Process user only
		}

		CDNUserSession* pUserObject = static_cast<CDNUserSession*>(*iter);
		if(pUserObject)
		{
			if( pUserObject->GetPvPIndex() == 0 && pUserObject->bIsLadderUser() == false )
			{
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
				CDNUserSession * pUserSenderObject = (CDNUserSession*)pSender;
				if (pUserObject != pUserSenderObject)
				{
					if (pUserObject->GetPvPChannelType() != pUserSenderObject->GetPvPChannelType())
					{
						++iter;
						continue;
					}
				}
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
				pUserObject->FieldProcess( pSender, wMsg, pSenderData, pParamData );
			}
			++iter;
		}
		else 
		{
			iter = m_pObjectList.erase(iter);
		}
	}

	return 0;
}

bool CDNFieldBlock::AddBaseObject(CDNBaseObject *pObj)
{
	ScopeLock<CSyncLock> lock(m_ObjectLock);
	m_pObjectList.push_back(pObj);

	return true;
}

bool CDNFieldBlock::DelBaseObject(CDNBaseObject *pObj)
{
	if (m_pObjectList.empty()) return false;

	ScopeLock<CSyncLock> lock(m_ObjectLock);

	m_pObjectList.remove(pObj);

	return true;
}

void CDNFieldBlock::SendFieldNotice(int cLen, const WCHAR *pwszChatMsg, int nShowSec)
{
	if (m_pObjectList.empty()) return;
	ScopeLock<CSyncLock> lock(m_ObjectLock);

	TObjectList::iterator iter;
	CDNUserSession *pUserObj = NULL;

	for (iter = m_pObjectList.begin(); iter != m_pObjectList.end(); iter++){
		if (g_IDGenerator.IsNpc((*iter)->GetObjectID())) continue;	// Process user only

		pUserObj = (CDNUserSession*)*iter;
		if (pUserObj){	// Seems more checks are needed
			pUserObj->SendNotice(pwszChatMsg, cLen, nShowSec);
		}
	}
}

void CDNFieldBlock::SendChannelChat(int cLen, const WCHAR *pwszCharacterName, const WCHAR *pwszChatMsg, int nRet)
{
	if (m_pObjectList.empty()) return;
	ScopeLock<CSyncLock> lock(m_ObjectLock);

	TObjectList::iterator iter;
	CDNUserSession *pUserObj = NULL;

	for (iter = m_pObjectList.begin(); iter != m_pObjectList.end(); iter++){
		if (g_IDGenerator.IsNpc((*iter)->GetObjectID())) continue;	// Process user only

		pUserObj = (CDNUserSession*)*iter;
		if (pUserObj){	// Seems more checks are needed
			pUserObj->SendChat(CHATTYPE_CHANNEL, cLen, pwszCharacterName, pwszChatMsg);
		}
	}
}

void CDNFieldBlock::SendDebugChat(const WCHAR* szMsg)
{
	if (m_pObjectList.empty()) return;
	ScopeLock<CSyncLock> lock(m_ObjectLock);

	TObjectList::iterator iter;
	CDNUserSession *pUserObj = NULL;

	for (iter = m_pObjectList.begin(); iter != m_pObjectList.end(); iter++){
		if (g_IDGenerator.IsNpc((*iter)->GetObjectID())) continue;	// Process user only

		pUserObj = (CDNUserSession*)*iter;
		if (pUserObj){	// Seems more checks are needed
			pUserObj->SendDebugChat(szMsg);
		}
	}
}
