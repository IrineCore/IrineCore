#include "stdafx.h"
#include "DNBaseObject.h"
#include "DNFieldManager.h"
#include "DNFieldDataManager.h"
#include "Util.h"
#include "Log.h"
#include "DNUserSession.h"
#include "SecondarySkillRepository.h"

extern TVillageConfig g_Config;

CDNBaseObject::CDNBaseObject(void): m_pField(NULL)
{
	memset(&m_BaseData, 0, sizeof(TBaseData));
	memset(&m_ParamData, 0, sizeof(TParamData));

	m_boAllowDelete = false;
	m_cLeaveType = 0;

	m_UserViewObjects.clear();
	m_NpcViewObjects.clear();
}

CDNBaseObject::~CDNBaseObject(void)
{
	FinalObject();
}

bool CDNBaseObject::AddViewObject(CDNBaseObject *pObj)
{
	if (g_IDGenerator.IsUser(pObj->GetObjectID())){
		ScopeLock<CSyncLock> lock(m_UserLock);

		if (m_UserViewObjects.find(pObj->GetObjectID()) != m_UserViewObjects.end()){
			return false;
		}

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		m_UserViewObjects[pObj->GetObjectID()] = std::make_pair(pObj, static_cast<CDNUserSession*>(pObj)->GetSocketContext());
#else
		m_UserViewObjects[pObj->GetObjectID()] = pObj;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	}
	else if (g_IDGenerator.IsNpc(pObj->GetObjectID())){
		ScopeLock<CSyncLock> lock(m_NpcLock);

		if (m_NpcViewObjects.find(pObj->GetObjectID()) != m_NpcViewObjects.end()){
			return false;
		}
		m_NpcViewObjects[pObj->GetObjectID()] = pObj;
	}
	return true;
}

bool CDNBaseObject::DelViewObject(CDNBaseObject *pObj)
{
	TMapObjects::iterator iter;
	if (g_IDGenerator.IsUser(pObj->GetObjectID())){
		if (m_UserViewObjects.empty()) return false;

		ScopeLock<CSyncLock> lock(m_UserLock);

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		std::map<UINT,std::pair<CDNBaseObject*,CSocketContext*>>::iterator iter = m_UserViewObjects.find(pObj->GetObjectID());
#else
		iter = m_UserViewObjects.find(pObj->GetObjectID());
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		if (iter != m_UserViewObjects.end()){
			m_UserViewObjects.erase(iter);
			return true;
		}
	}
	else if (g_IDGenerator.IsNpc(pObj->GetObjectID())){
		ScopeLock<CSyncLock> lock(m_NpcLock);

		iter = m_NpcViewObjects.find(pObj->GetObjectID());
		if (iter != m_NpcViewObjects.end()){
			m_NpcViewObjects.erase(iter);
			return true;
		}
	}

	return false;
}

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
CSocketContext* CDNBaseObject::GetViewObjectContext(UINT nUID)
{
	if( g_IDGenerator.IsUser(nUID) == false )
	{
		_ASSERT(0);
		return NULL;
	}

	ScopeLock<CSyncLock> lock(m_UserLock);
	std::map<UINT,std::pair<CDNBaseObject*,CSocketContext*>>::iterator iter = m_UserViewObjects.find(nUID);
	if (iter == m_UserViewObjects.end())
		return NULL;

	return (*iter).second.second;
}
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

CDNBaseObject* CDNBaseObject::GetViewObject(UINT nObjUID, bool bIgnoreDangling/*=false*/ )
{
	CDNBaseObject *pObj = NULL;
	TMapObjects::iterator iter;
	if (g_IDGenerator.IsUser(nObjUID)){
		if (m_UserViewObjects.empty()) return NULL;

		ScopeLock<CSyncLock> lock(m_UserLock);
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		std::map<UINT,std::pair<CDNBaseObject*,CSocketContext*>>::iterator iter = m_UserViewObjects.find(nObjUID);
#else
		iter = m_UserViewObjects.find(nObjUID);
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		if (iter != m_UserViewObjects.end())
		{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			CScopeInterlocked Scope( &(*iter).second.second->m_lActiveCount );
			if( Scope.bIsDelete() && bIgnoreDangling == false )
				return NULL;

			pObj = static_cast<CDNUserSession*>(iter->second.second->GetParam());
#else
			pObj = iter->second;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			return pObj;
		}
	}
	else if (g_IDGenerator.IsNpc(nObjUID)){
		ScopeLock<CSyncLock> lock(m_NpcLock);

		iter = m_NpcViewObjects.find(nObjUID);
		if (iter != m_NpcViewObjects.end()){
			pObj = iter->second;
			return pObj;
		}
	}

	return NULL;
}

bool CDNBaseObject::EnterWorld()
{
	LeaveWorld();

	m_pField = g_pFieldManager->GetField(m_BaseData.nChannelID);
	if (!m_pField){
		g_Log.Log(LogType::_ERROR, L"[ObjID:%u] EnterWorld Failed (MapID:%d)\r\n", m_BaseData.nObjectID, m_BaseData.nChannelID);
		return false;
	}

	if( m_pField->SendFieldMessage(this, FM_CREATE, &m_BaseData, NULL) == -1 ) {
		m_pField->GetFieldData()->GetStartPosition( 0, m_BaseData.CurPos );
		m_BaseData.nChannelID = m_pField->GetChnnelID();
		m_BaseData.TargetPos = m_BaseData.CurPos;
		m_pField->SendFieldMessage(this, FM_CREATE, &m_BaseData, NULL);
	}
	return true;
}

bool CDNBaseObject::LeaveWorld()
{
	if (!m_pField) return false;

	m_cLeaveType = LEAVE_LOGOUT;
	if (m_pField) m_pField->SendFieldMessage(this, FM_DESTROY, &m_BaseData, NULL);

	CDNBaseObject *pObj = NULL;
	TMapObjects::iterator iter;

	if (!m_UserViewObjects.empty()){
		ScopeLock<CSyncLock> lock(m_UserLock);

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		for ( std::map<UINT,std::pair<CDNBaseObject*,CSocketContext*>>::iterator iter = m_UserViewObjects.begin(); iter != m_UserViewObjects.end(); ++iter)
		{
			CScopeInterlocked Scope( &(*iter).second.second->m_lActiveCount );
			if( Scope.bIsDelete() )
			{
				g_Log.Log(LogType::_ERROR, L"[%d] UserViewObject Occur DanglingPointer!!!\r\n", g_Config.nManagedID );
				continue;
			}
			pObj = static_cast<CDNUserSession*>((*iter).second.second->GetParam());
			pObj->DelViewObject(this);
		}
#else
		for (iter = m_UserViewObjects.begin(); iter != m_UserViewObjects.end(); ++iter)
		{
			pObj = iter->second;
			pObj->DelViewObject(this);
		}
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		m_UserViewObjects.clear();
	}

	if (!m_NpcViewObjects.empty()){
		ScopeLock<CSyncLock> lock(m_NpcLock);

		for (iter = m_NpcViewObjects.begin(); iter != m_NpcViewObjects.end(); ++iter){
			pObj = iter->second;
			// g_Log.Log(L"[ObjID:%u] [CDNBaseObject::LeaveWorld(Npc)] BaseUID:%u, DelViewObject:%u\r\n", m_BaseData.nObjectID, pObj->GetObjectID());
			pObj->DelViewObject(this);
		}
		m_NpcViewObjects.clear();
	}

	m_pField = NULL;

	return true;
}

bool CDNBaseObject::InitObject(WCHAR *pName, UINT nObjUID, int nChannelID, int nMapIndex, TPosition *pCurPos)
{
	m_BaseData.nObjectID = nObjUID;	
	m_BaseData.nChannelID = nChannelID;
	m_BaseData.CurPos = *pCurPos;
	m_BaseData.TargetPos = *pCurPos;
	m_BaseData.bBattleMode = false;

	SetCharacterName(pName);

	m_UserViewObjects.clear();
	m_NpcViewObjects.clear();

	return true;
}

bool CDNBaseObject::FinalObject()
{
	m_pField = NULL;
	memset(&m_BaseData, 0, sizeof(TBaseData));

	if (!m_UserViewObjects.empty())
		g_Log.Log(LogType::_ERROR, L"[ObjID:%u] [CDNBaseObject::FinalObject] UserView Error!! This:%x, Count:%d\r\n", m_BaseData.nObjectID, this, m_UserViewObjects.size());
	if (!m_NpcViewObjects.empty())
		g_Log.Log(LogType::_ERROR, L"[ObjID:%u] [CDNBaseObject::FinalObject] NpcView Error!! This:%x, Count:%d\r\n", m_BaseData.nObjectID, this, m_NpcViewObjects.size());

	m_UserViewObjects.clear ();
	m_NpcViewObjects.clear ();

	return true;
}

int CDNBaseObject::FieldProcess(CDNBaseObject *pSender, USHORT wMsg, TBaseData *pSenderData, TParamData *pParamData)
{
	switch(wMsg)
	{
	case FM_CREATE:
		if (IsView(&pSender->GetCurrentPos())){
			pSender->FieldProcess(this, FM_SHOW, &m_BaseData, &m_ParamData);
			FieldProcess(pSender, FM_SHOW, pSenderData, pParamData);
		}
		break;

	case FM_DESTROY:
		if (GetViewObject(pSender->GetObjectID(),true)){
			pSender->FieldProcess(this, FM_HIDE, &m_BaseData, &m_ParamData);
			FieldProcess(pSender, FM_HIDE, pSenderData, pParamData);
		}
		break;

	case FM_SHOW:
		AddViewObject(pSender);
		break;

	case FM_HIDE:
		DelViewObject(pSender);
		break;

	case FM_MOVE:
		{
			int Flag1;
			if (GetViewObject(pSender->GetObjectID()))
				Flag1 = 1;
			else 
				Flag1 = 0;

			int Flag2 = IsView(&pSender->GetTargetPos());

			if (Flag1 == 0 && Flag2 == 1){
				pSender->FieldProcess(this, FM_SHOW, &m_BaseData, &m_ParamData);
				FieldProcess(pSender, FM_SHOW, pSenderData, pParamData);
			}
			else if (Flag1 == 1 && Flag2 == 0){
				m_cLeaveType = LEAVE_MOVE;
				pSender->FieldProcess(this, FM_HIDE, &m_BaseData, &m_ParamData);
				FieldProcess(pSender, FM_HIDE, pSenderData, pParamData);
			}
		}
		break;
	}

	return 1;
}

void CDNBaseObject::DoUpdate(DWORD CurTick)
{

}

int CDNBaseObject::SendLocalMessage(UINT nObjUID, USHORT wMsg)
{
	CDNBaseObject *pObj = NULL;

	if (nObjUID == 0){
		TMapObjects::iterator iter;

		if (!m_UserViewObjects.empty()){
			ScopeLock<CSyncLock> lock(m_UserLock);

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			for ( std::map<UINT,std::pair<CDNBaseObject*,CSocketContext*>>::iterator iter = m_UserViewObjects.begin(); iter != m_UserViewObjects.end(); )
			{
				CScopeInterlocked Scope( &(*iter).second.second->m_lActiveCount );
				if( Scope.bIsDelete() )
				{
					g_Log.Log(LogType::_ERROR, L"[%d] UserViewObject Occur DanglingPointer!!!\r\n", g_Config.nManagedID );
					m_UserViewObjects.erase(iter++);
					continue;
				}
				pObj = static_cast<CDNUserSession*>((*iter).second.second->GetParam());
				if (pObj && (pObj->GetObjectID() > 0))
				{
					pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
					++iter;
				}
				else {
					m_UserViewObjects.erase(iter++);
				}
			}
		}
#else
			for (iter = m_UserViewObjects.begin(); iter != m_UserViewObjects.end(); )
			{
				pObj = iter->second;
				if (pObj && (pObj->GetObjectID() > 0))
				{
					pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
					++iter;
				}
				else {
					m_UserViewObjects.erase(iter++);
				}
			}
		}
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

		if (!m_NpcViewObjects.empty()){
			ScopeLock<CSyncLock> lock(m_NpcLock);

			for (iter = m_NpcViewObjects.begin(); iter != m_NpcViewObjects.end(); ){
				pObj = iter->second;
				if (pObj && (pObj->GetObjectID() > 0)){
					pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
					++iter;
				}
				else {
					m_NpcViewObjects.erase(iter++);
				}
			}
		}
	}
	else if (nObjUID == m_BaseData.nObjectID){
		FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
		nObjUID = 0;
	}
	else {
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		if (g_IDGenerator.IsUser(nObjUID))
		{
			CSocketContext* pSocketContext = GetViewObjectContext(nObjUID);
			if( pSocketContext )
			{
				CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
				if( Scope.bIsDelete() == false )
				{
					pObj = static_cast<CDNUserSession*>(pSocketContext->GetParam());
					if( pObj )
					{
						pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
						nObjUID = 0;
					}
				}	
			}
		}
		else
		{
			pObj = GetViewObject(nObjUID);
			if (pObj){
				pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
				nObjUID = 0;
			}
		}
#else
		pObj = GetViewObject(nObjUID);
		if (pObj){
			pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
			nObjUID = 0;
		}
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	}

	if (nObjUID == 0) return 0;
	return -1;
}

int CDNBaseObject::SendUserLocalMessage(UINT nObjUID, USHORT wMsg)
{
#if !defined( _FINAL_BUILD )
	if( wMsg == FM_CHAT )
	{
		if( g_IDGenerator.IsUser( GetObjectID() ) == false )
			return 0;

		CDNUserSession* pUserSession = static_cast<CDNUserSession*>(this);

		// Debug code
		std::vector<std::wstring> tokens;
		if( wcslen( m_ParamData.wszChatMsg ) > 0 )
			TokenizeW( m_ParamData.wszChatMsg, tokens, L" " );

		if( !tokens.empty() )
		{
			if( wcsicmp( tokens[0].c_str(), L"/getmasterlist") == 0 )
			{
				MasterSystem::CSMasterList TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				if( tokens.size() >= 2 )
					TxPacket.uiPage = _wtoi( tokens[1].c_str() );
				if( tokens.size() >= 3 )
					TxPacket.cJob = _wtoi( tokens[2].c_str() );
				if( tokens.size() >= 4 )
					TxPacket.cGender = _wtoi( tokens[3].c_str() );

				pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_MASTERLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/getmasterpage") == 0 )
			{
				pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_MASTERCHARACTERINFO, NULL, 0 );
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/registermaster") == 0 )
			{
				MasterSystem::CSIntroduction TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				int iSize = sizeof(TxPacket)-sizeof(TxPacket.wszSelfIntroduction);

				if( tokens.size() >= 2 )
				{
					TxPacket.bRegister = true;
					_wcscpy( TxPacket.wszSelfIntroduction, MasterSystem::Max::SelfIntrotuctionLen, tokens[1].c_str(), static_cast<int>(wcslen(tokens[1].c_str())) );

					iSize = sizeof(TxPacket);
				}

				pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_INTRODUCTION_ONOFF, reinterpret_cast<char*>(&TxPacket), iSize );
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/masterapplication") == 0 )
			{
				MasterSystem::CSMasterApplication TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				if( tokens.size() >= 2 )
				{
					TxPacket.biCharacterDBID = _wtoi64(tokens[1].c_str());

					pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_MASTER_APPLICATION, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/joinmastersystem") == 0 )
			{
				MasterSystem::CSJoin TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				if( tokens.size() >= 2 )
				{
					TxPacket.biMasterCharacterDBID = _wtoi64(tokens[1].c_str());

					pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_JOIN, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/getmasterandmate") == 0 )
			{
				pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_MASTERANDCLASSMATE, NULL, 0 );
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/getmateinfo") == 0 )
			{
				MasterSystem::CSClassmateInfo TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				if( tokens.size() >= 2 )
				{
					TxPacket.biClassmateCharacterDBID = _wtoi64(tokens[1].c_str());

					pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_CLASSMATEINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/getmymasterinfo") == 0 )
			{
				MasterSystem::CSMyMasterInfo TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				if( tokens.size() >= 2 )
				{
					TxPacket.biMasterCharacterDBID = _wtoi64(tokens[1].c_str());

					pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_MYMASTERINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/leavemastersystem") == 0 )
			{
				MasterSystem::CSLeave TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				if( tokens.size() >= 3 )
				{
					TxPacket.biDestCharacterDBID	= _wtoi64(tokens[1].c_str());
					TxPacket.bIsMaster				= _wtoi(tokens[2].c_str()) ? true : false;

					pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_LEAVE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/invitepupilconfirm") == 0 )
			{
				MasterSystem::CSInvitePupilConfirm TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				if( tokens.size() >= 2 )
				{
					_wcscpy( TxPacket.wszMasterCharName, _countof(TxPacket.wszMasterCharName), tokens[1].c_str(), (int)wcslen(tokens[1].c_str()) );

					pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_INVITE_PUPIL_CONFIRM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/joindirectconfirm") == 0 )
			{
				MasterSystem::CSJoinDirectConfirm TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				if( tokens.size() >= 2 )
				{
					_wcscpy( TxPacket.wszPupilCharName, _countof(TxPacket.wszPupilCharName), tokens[1].c_str(), (int)wcslen(tokens[1].c_str()) );

					pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_JOIN_DIRECT_CONFIRM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/recallmaster") == 0 )
			{
				MasterSystem::CSRecallMaster TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				if( tokens.size() >= 2 )
				{
					_wcscpy( TxPacket.wszMasterCharName, _countof(TxPacket.wszMasterCharName), tokens[1].c_str(), (int)wcslen(tokens[1].c_str()) );

					pUserSession->OnRecvMasterSystemMessage( eMasterSystem::CS_RECALL_MASTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
#if defined( PRE_ADD_SECONDARY_SKILL )
			if( wcsicmp( tokens[0].c_str(), L"/addsubskill") == 0 )
			{
				if( tokens.size() >= 2 )
				{
					int iSkillID = _wtoi(tokens[1].c_str());
					pUserSession->GetSecondarySkillRepository()->Create( iSkillID );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/delsubskill" ) == 0 )
			{
				if( tokens.size() >= 2 )
				{
					SecondarySkill::CSDelete TxPacket;
					memset( &TxPacket, 0, sizeof(TxPacket) );

					TxPacket.iSkillID = _wtoi(tokens[1].c_str());
					pUserSession->OnRecvSecondarySkillMessage( eSecondarySkill::CS_DELETE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/subskillexp" ) == 0 )
			{
				if( tokens.size() >= 3 )
				{
					int iSkillID	= _wtoi(tokens[1].c_str());
					int iAddExp		= _wtoi(tokens[2].c_str());

					pUserSession->GetSecondarySkillRepository()->AddExp( iSkillID, iAddExp );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/addrecipe" ) == 0 )
			{
				if( tokens.size() >= 3 )
				{
					SecondarySkill::CSAddRecipe TxPacket;
					memset( &TxPacket, 0, sizeof(TxPacket) );

					TxPacket.iSkillID		= _wtoi(tokens[1].c_str());
					TxPacket.cInvenType		= ITEMPOSITION_INVEN;
					TxPacket.cInvenIndex	= _wtoi(tokens[2].c_str());

					const TItem* pItem = pUserSession->GetItem()->GetInventory( TxPacket.cInvenIndex );
					if( pItem )
					{
						TxPacket.biInvenSerial	= pItem->nSerial;

						pUserSession->OnRecvSecondarySkillMessage( eSecondarySkill::CS_ADD_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
					}
					return 0;
				}
			}
			else if( wcsicmp( tokens[0].c_str(), L"/recipeexp" ) == 0 )
			{
				if( tokens.size() >= 4 )
				{
					int iSkillID	= _wtoi(tokens[1].c_str());
					int iItemID		= _wtoi(tokens[2].c_str());
					int iAddExp		= _wtoi(tokens[3].c_str());

					pUserSession->GetSecondarySkillRepository()->AddRecipeExp( iSkillID, iItemID, iAddExp );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/delrecipe" ) == 0 )
			{
				if( tokens.size() >= 3 )
				{
					SecondarySkill::CSDeleteRecipe TxPacket;
					memset( &TxPacket, 0, sizeof(TxPacket) );

					TxPacket.iSkillID	= _wtoi(tokens[1].c_str());
					TxPacket.iItemID	= _wtoi(tokens[2].c_str());

					pUserSession->OnRecvSecondarySkillMessage( eSecondarySkill::CS_DELETE_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/extractrecipe" ) == 0 )
			{
				if( tokens.size() >= 3 )
				{
					SecondarySkill::CSExtractRecipe TxPacket;
					memset( &TxPacket, 0, sizeof(TxPacket) );

					TxPacket.iSkillID	= _wtoi(tokens[1].c_str());
					TxPacket.iItemID	= _wtoi(tokens[2].c_str());

					pUserSession->OnRecvSecondarySkillMessage( eSecondarySkill::CS_EXTRACT_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
			else if( wcsicmp( tokens[0].c_str(), L"/craftsubskill" ) == 0 )
			{
				if( tokens.size() >= 4 )
				{
					SecondarySkill::CSManufacture TxPacket;
					memset( &TxPacket, 0, sizeof(TxPacket) );

					TxPacket.bIsStart	= _wtoi(tokens[1].c_str()) ? true : false;
					TxPacket.iSkillID	= _wtoi(tokens[2].c_str());
					TxPacket.iItemID	= _wtoi(tokens[3].c_str());

					pUserSession->OnRecvSecondarySkillMessage( eSecondarySkill::CS_MANUFACTURE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				return 0;
			}
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
		}
	}
#endif // #if !defined( _FINAL_BUILD )

	if (m_UserViewObjects.empty()) return -1;

	CDNBaseObject *pObj = NULL;

	if (nObjUID == 0){
		TMapObjects::iterator iter;

		if (!m_UserViewObjects.empty()){
			ScopeLock<CSyncLock> lock(m_UserLock);

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			for ( std::map<UINT,std::pair<CDNBaseObject*,CSocketContext*>>::iterator iter = m_UserViewObjects.begin(); iter != m_UserViewObjects.end(); )
			{
				CScopeInterlocked Scope( &(*iter).second.second->m_lActiveCount );
				if( Scope.bIsDelete() )
				{
					g_Log.Log(LogType::_ERROR, L"[%d] UserViewObject Occur DanglingPointer!!!\r\n", g_Config.nManagedID );
					m_UserViewObjects.erase(iter++);
					continue;
				}

				pObj = static_cast<CDNUserSession*>((*iter).second.second->GetParam());

				if (pObj && (pObj->GetObjectID() > 0))
				{
					pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
					++iter;
				}
				else 
				{
					m_UserViewObjects.erase(iter++);
				}
			}
#else
			for (iter = m_UserViewObjects.begin(); iter != m_UserViewObjects.end(); )
			{
				pObj = iter->second;


				if (pObj && (pObj->GetObjectID() > 0)){
					pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
					++iter;
				}
				else {
					m_UserViewObjects.erase(iter++);
				}
			}
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		}
	}
	else if (nObjUID == m_BaseData.nObjectID){
		FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
		nObjUID = 0;
	}
	else 
	{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		if (g_IDGenerator.IsUser(nObjUID))
		{
			CSocketContext* pSocketContext = GetViewObjectContext(nObjUID);
			if( pSocketContext )
			{
				CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
				if( Scope.bIsDelete() == false )
				{
					pObj = static_cast<CDNUserSession*>(pSocketContext->GetParam());
					if( pObj )
					{
						pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
						nObjUID = 0;
					}
				}	
			}
		}
		else
		{
			pObj = GetViewObject(nObjUID);
			if (pObj){
				pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
				nObjUID = 0;
			}
		}
#else
		pObj = GetViewObject(nObjUID);
		if (pObj){
			pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
			nObjUID = 0;
		}
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	}

	if (nObjUID == 0) return 0;
	return -1;
}

int CDNBaseObject::SendNpcLocalMessage(UINT nObjUID, USHORT wMsg)
{
	if (m_NpcViewObjects.empty()) return -1;

	CDNBaseObject *pObj = NULL;

	if (nObjUID == 0){
		TMapObjects::iterator iter;

		if (!m_NpcViewObjects.empty()){
			ScopeLock<CSyncLock> lock(m_NpcLock);

			for (iter = m_NpcViewObjects.begin(); iter != m_NpcViewObjects.end(); ){
				pObj = iter->second;
				if (pObj && (pObj->GetObjectID() > 0)){
					pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
					++iter;
				}
				else {
					m_NpcViewObjects.erase(iter++);
				}
			}
		}
	}
	else if (nObjUID == m_BaseData.nObjectID){
		FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
		nObjUID = 0;
	}
	else {
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		if (g_IDGenerator.IsUser(nObjUID))
		{
			CSocketContext* pSocketContext = GetViewObjectContext(nObjUID);
			if( pSocketContext )
			{
				CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
				if( Scope.bIsDelete() == false )
				{
					pObj = static_cast<CDNUserSession*>(pSocketContext->GetParam());
					if( pObj )
					{
						pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
						nObjUID = 0;
					}
				}	
			}
		}
		else
		{
			pObj = GetViewObject(nObjUID);
			if (pObj){
				pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
				nObjUID = 0;
			}
		}
#else
		pObj = GetViewObject(nObjUID);
		if (pObj){
			pObj->FieldProcess(this, wMsg, &m_BaseData, &m_ParamData);
			nObjUID = 0;
		}
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	}

	if (nObjUID == 0) return 0;
	return -1;
}

int CDNBaseObject::SendFieldMessage(USHORT wMsg)
{
	if (!m_pField) return -1;
	return m_pField->SendFieldMessage(this, wMsg, &m_BaseData, &m_ParamData);
}

int CDNBaseObject::IsView (TPosition *pPos)
{
	if ((abs(m_BaseData.CurPos.nX - pPos->nX) < VIEWRANGE * 1000) && (abs(m_BaseData.CurPos.nZ - pPos->nZ) < VIEWRANGE * 1000)) return 1;
	return 0;
}

int CDNBaseObject::GetDistance (TPosition *pPos)
{
	return abs((m_BaseData.CurPos.nX / 1000) - (pPos->nX / 1000)) + abs((m_BaseData.CurPos.nY / 1000) - (pPos->nY / 1000)) + abs((m_BaseData.CurPos.nZ / 1000) - (pPos->nZ / 1000));
}

int CDNBaseObject::GetChannelAttribute()
{
	int nAttribute = 0;
	if (m_pField) nAttribute = m_pField->GetChannelAtt();
	return nAttribute;
}

void CDNBaseObject::SetCharacterName(const WCHAR* pwszName)
{
	if (!pwszName) return;
	_wcscpy(m_BaseData.wszName, _countof(m_BaseData.wszName), pwszName, (int)wcslen(pwszName));
}
