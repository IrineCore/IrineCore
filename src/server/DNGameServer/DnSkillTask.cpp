#include "stdafx.h"
#include "DnSkillTask.h"
#include "DNProtocol.h"
#include "DNUserSession.h"
#include "DnActor.h"
#include "DNLogConnection.h"
#include "DNMissionSystem.h"
#include "DNDBConnectionManager.h"
#include "DNGameDataManager.h"
#include "DnPlayerActor.h"
#include "DNDBConnection.h"

CDnSkillTask::CDnSkillTask( CDNGameRoom* pRoom ) : CTask( pRoom ), 
												   CMultiSingleton<CDnSkillTask, MAX_SESSION_COUNT>( pRoom )
{
	m_listWaitSkillLevelup.clear();
}

CDnSkillTask::~CDnSkillTask(void)
{
	list<S_SKILL_LEVELUP_INFO*>::iterator iter = m_listWaitSkillLevelup.begin();
	for( iter; iter != m_listWaitSkillLevelup.end(); ++iter )
		delete (*iter);
	m_listWaitSkillLevelup.clear();
}

bool CDnSkillTask::Initialize( void )
{
	//m_SkillTreeSystem.InitializeTable();

	return true;
}

#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
int CDnSkillTask::UnLockSkillByMoney( CDNUserSession* pSession, int nSkillID )
#else
bool CDnSkillTask::UnLockSkillByMoney( CDNUserSession* pSession, int nSkillID )
#endif
{
	CDnPlayerActor * pPlayerActor = pSession->GetPlayerActor();

	_ASSERT( pPlayerActor );

	if( nSkillID <= 0 )
	{
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_ACQUIREFAIL;
#else
		pSession->SendAcquireSkill( nSkillID, ERROR_SKILL_ACQUIREFAIL );
		return false;
#endif
	}

	TSkillData * pSkillData = g_pDataManager->GetSkillData( nSkillID );
	if( NULL == pSkillData )
	{
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_ACQUIREFAIL;
#else
		pSession->SendAcquireSkill( nSkillID, ERROR_SKILL_ACQUIREFAIL );
		return false;
#endif
	}

	if( pPlayerActor->IsExclusiveSkill( nSkillID, pSkillData->nExclusiveID ) )
	{
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE;
#else
		pSession->SendAcquireSkill( nSkillID, ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE );
		return false;
#endif
	}

	bool bAlreadyGlobalSkillAcquired = false;
	if( pPlayerActor->HasSameGlobalIDSkill( pSkillData->nSkillID ) )
		bAlreadyGlobalSkillAcquired = true;

	const INT64 iNowMoney = pSession->GetCoin();
	if( iNowMoney < pSkillData->nUnlockPrice )
	{
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_UNLOCK_NOT_ENOUGH_MONEY;
#else
		pSession->SendAcquireSkill( nSkillID, ERROR_SKILL_UNLOCK_NOT_ENOUGH_MONEY );
		return false;
#endif
	}

	DnSkillHandle hSkill = pPlayerActor->FindSkill( nSkillID );
	if( NULL != hSkill.GetPointer() )
	{
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_UNLOCK_ALREADY_OPEN;
#else
		pSession->SendAcquireSkill( nSkillID, ERROR_SKILL_UNLOCK_ALREADY_OPEN );
		return false;
#endif
	}

	int nRetCode = ERROR_NONE;
	CDnSkillTreeSystem * pSkillTreeSystem = g_pDataManager->GetSkillTreeSystem();

#ifdef PRE_ADD_ONLY_SKILLBOOK
	if( pSkillTreeSystem )
	{
		bool bExistSkill = false;
		bool bNeedSkillBook = pSkillTreeSystem->IsNeedSkillBook( nSkillID, bExistSkill );
		if( bExistSkill && bNeedSkillBook )
		{
			return ERROR_SKILL_UNLOCK_FAIL;
		}
	}
#endif // PRE_ADD_ONLY_SKILLBOOK

	CDnSkillTreeSystem::S_OUTPUT Output;
	CDnSkillTreeSystem::S_TRY_ACQUIRE TryAcquire( pPlayerActor->GetPossessedSkillInfo() );
	TryAcquire.iCurrentCharLevel = pSession->GetLevel();
	TryAcquire.iTryAcquireSkillID = nSkillID;
	TryAcquire.iHasSkillPoint = pPlayerActor->GetAvailSkillPointByJob( nSkillID );

	pSkillTreeSystem->TryAcquireSkill( TryAcquire, &Output );

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
	std::vector<int> nNeedSPValues;
	pSkillTreeSystem->GetNeedSPValuesByJob( nSkillID, nNeedSPValues );

	std::vector<int> nJobHistorys;
	pPlayerActor->GetJobHistory( nJobHistorys );

	const bool bAvailableSPByJob = IsAvailableSPByJob( nJobHistorys, nNeedSPValues, pPlayerActor );
	if( false == bAvailableSPByJob )
		Output.eResult = CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE;
#endif	// #if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)

	bool bIgnoreParentSkillCondition = ( (CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL == Output.eResult) ||
										(CDnSkillTreeSystem::R_LOCKED_PARENTSKILL == Output.eResult) ||
										(CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL == Output.eResult) ) &&
										true == bAlreadyGlobalSkillAcquired;

	if( CDnSkillTreeSystem::R_SUCCESS == Output.eResult || true == bIgnoreParentSkillCondition )
	{
		TSkill Skill;
		SecureZeroMemory( &Skill, sizeof(TSkill) );
		Skill.nSkillID = nSkillID;
		Skill.cSkillLevel = 0;
		Skill.bLock = false;

		CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
		SkillInfo.iSkillID = nSkillID;
		SkillInfo.iSkillLevel = 0;
		SkillInfo.bCurrentLock = false;

		// Unlock and acquire.
		pPlayerActor->UnLockSkill( nSkillID, pSkillData->nUnlockPrice );
		pPlayerActor->AcquireSkill( nSkillID );

		// Send result to client.
#if !defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		pSession->SendUnlockSkill( nSkillID, ERROR_NONE );
		pSession->SendAcquireSkill( nSkillID, ERROR_NONE );
#endif // #if !defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)		

#if defined( _GAMESERVER )
		g_Log.Log(LogType::_ERROR, pSession, L"SkillTask152 PrevCoin=%I64d ChangeGold=%d PickUpCoin=%I64d \r\n", pSession->GetCoin(), pSkillData->nUnlockPrice, pSession->GetPickUpCoin());
#endif
		pSession->DelCoin( pSkillData->nUnlockPrice, DBDNWorldDef::CoinChangeCode::DoNotDBSave, 0 );
	}
	else
	{
		switch( Output.eResult )
		{
		case CDnSkillTreeSystem::R_NOT_ENOUGH_CHAR_LEVEL:
			nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_CHAR_LEVEL;
			break;

		case CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL:
			nRetCode = ERROR_SKILL_ACQUIRE_FAIL_DONT_HAVE_PARENT_SKILL;
			break;

		case CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL:
			nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_PARENT_SKILL_LEVEL;
			break;

		case CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE:
			nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_SKILLPOINT;
			break;

		default:
			nRetCode = ERROR_SKILL_UNLOCK_FAIL;
			break;
		}
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return nRetCode;
	}
	return ERROR_NONE;
#else
		pSession->SendAcquireSkill( nSkillID, nRetCode );
	}
	return true;
#endif
}

bool CDnSkillTask::UseSkillBook( CDNUserSession* pSession, int nItemID )
{
	_ASSERT( pSession->GetPlayerActor() );
	CDnSkillTreeSystem* pSkillTreeSystem = g_pDataManager->GetSkillTreeSystem();
	if (!pSkillTreeSystem){
		pSession->SendAcquireSkill( 0, ERROR_SKILL_UNLOCK_FAIL );
		return false;
	}

	// Check if skill is already owned
	int iSkillIDToUnLock = pSkillTreeSystem->FindSkillBySkillBook( nItemID );

	// Prevent learning mutually exclusive skills.
	DnActorHandle hActor = pSession->GetActorHandle();
	CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
	TSkillData* pSkillData = g_pDataManager->GetSkillData( iSkillIDToUnLock );
	if( pPlayerActor->IsExclusiveSkill( iSkillIDToUnLock, pSkillData->nExclusiveID ) )
	{
		pSession->SendAcquireSkill( 0, ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE );
		return false;
	}

	DnSkillHandle hAlreadyHasSkill = pSession->GetActorHandle()->FindSkill( iSkillIDToUnLock );
	int nRetCode = ERROR_NONE;
	int nResultSkillID = 0;
	if( !hAlreadyHasSkill )
	{
		CDnSkillTreeSystem::S_OUTPUT Output;

		//CDnSkillTreeSystem::S_CURRENT_USERSKILL_INFO CurrentSkillInfo( pPlayerActor->GetPossessedSkillInfo() );
		CDnSkillTreeSystem::S_TRY_UNLOCK CurrentSkillInfo;
		CurrentSkillInfo.iTryUnlockSkillID = iSkillIDToUnLock;
		CurrentSkillInfo.iCurrentCharLevel = pPlayerActor->GetLevel();
		CurrentSkillInfo.iSkillBookItemID = nItemID;

		DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
		int iNeedJob = pSkillTable->GetFieldFromLablePtr( iSkillIDToUnLock, "_NeedJob" )->GetInteger();
		if( pSession->GetPlayerActor()->IsPassJob( iNeedJob ) )
		{
			//CurrentSkillInfo.iJobID = iNeedJob;

			pSkillTreeSystem->TryUnLockSkill( CurrentSkillInfo, &Output );

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
			CDnPlayerActor * pPlayerActor = pSession->GetPlayerActor();
			if( NULL != pPlayerActor )
			{
				std::vector<int> nNeedSPValues;
				pSkillTreeSystem->GetNeedSPValuesByJob(iSkillIDToUnLock, nNeedSPValues);

				std::vector<int> jobHistory;
				pPlayerActor->GetJobHistory( jobHistory );

				bool bAvailableSPByJob = IsAvailableSPByJob(jobHistory, nNeedSPValues, pPlayerActor);

				if (bAvailableSPByJob == false)
					Output.eResult = CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE;
			}
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP

			// Notify client based on result.
			if( CDnSkillTreeSystem::R_SUCCESS == Output.eResult )
			{
				// Success.
				nResultSkillID = iSkillIDToUnLock;

				// Check if it is in unlocked skills
				_ASSERT( pPlayerActor );

				if( pPlayerActor )
				{
					nRetCode = pPlayerActor->CanAcquireSkillIfUnlock( nResultSkillID );
					if( ERROR_NONE == nRetCode )
					{
						// Unlock and acquire.
						pPlayerActor->UnLockSkill( nResultSkillID );
						pPlayerActor->AcquireSkill( nResultSkillID );

						// Send result to client.
						pSession->SendUnlockSkill( nResultSkillID, ERROR_NONE );
						pSession->SendAcquireSkill( nResultSkillID, nRetCode );

						return true;
					}
				}
			}
			else if ( CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE == Output.eResult )
				nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_SKILLPOINT;
			else
				nRetCode = ERROR_SKILL_UNLOCK_FAIL;
		}
		else
		{
			nRetCode = ERROR_SKILL_UNLOCK_MISMATCH_JOB;
		}
	}
	else
	{
		nRetCode = ERROR_SKILL_UNLOCK_ALREADY_OPEN;
	}

	pSession->SendAcquireSkill( nResultSkillID, nRetCode );
	
	return false;
}


void CDnSkillTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_pBaseRoom && static_cast<CDNGameRoom*>(m_pBaseRoom)->GetRoomState() != _GAME_STATE_PLAY )
		return;

	if( !m_listWaitSkillLevelup.empty() )
	{
		list<S_SKILL_LEVELUP_INFO*>::iterator iter = m_listWaitSkillLevelup.begin();
		for( iter; iter != m_listWaitSkillLevelup.end(); )
		{
			S_SKILL_LEVELUP_INFO* pInfo = *iter;
			if( pInfo->hActor && pInfo->hSkill )
			{
				// When it is an active skill and not currently in use.
				// For auto passive skill, process skill level-up packet and notify client when not currently triggered.
				if( (/*(pInfo->hSkill->GetSkillType() == CDnSkill::Active) &&*/ (pInfo->hActor->GetProcessSkill() != pInfo->hSkill)) ||
					((pInfo->hSkill->GetSkillType() == CDnSkill::AutoPassive) && (false == pInfo->hActor->IsProcessingAutoPassive(pInfo->hSkill->GetClassID()))) )
				{
					SkillLevelUp( pInfo->pSession, pInfo->nSkillID, pInfo->cUseSkillPoint );
					delete (*iter);
					iter = m_listWaitSkillLevelup.erase( iter );
					continue;
				}
			}
			else
			{
				// Remove disappeared actor objects from the list.
				delete (*iter);
				iter = m_listWaitSkillLevelup.erase( iter );
				continue;
			}
			 
			++iter;
		}
	}
}

int CDnSkillTask::OnDispatchMessage( CDNUserSession * pSession, int nMainCmd, int nSubCmd, char* pData, int nLen )
{
	switch( nMainCmd )
	{
		case CS_SKILL: 
			return OnRecvSkillMessage( pSession, nSubCmd, pData, nLen );
	}

	return ERROR_NONE;
}

int CDnSkillTask::OnRecvSkillMessage( CDNUserSession* pSession, int nSubCmd, char *pData, int nLen )
{
	switch( nSubCmd )
	{
		case eSkill::CS_UNLOCKSKILL_BY_MONEY_REQ:
			return OnRecvUnlockSkillByMoneyReq( pSession, (CSUnlockSkillByMoneyReq*)pData );
		case eSkill::CS_SKILLLEVELUP_REQ:
			return OnRecvSkillLevelUpReq( pSession, (CSSkillLevelUpReq*)pData );

		case eSkill::CS_ACQUIRESKILL_REQ:
			return OnRecvAcquireSkillReq( pSession, (CSAcquireSkillReq*)pData );			
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		case eSkill::CS_RESERVATION_SKILL_LIST_REQ:
			return OnRecvReservationSkillListReq( pSession, (CSReservationSkillListReq*)pData );
#endif
#ifdef PRE_ADD_PRESET_SKILLTREE
		case eSkill::CS_LIST_SKILLSET:
			{
				if (nLen != 0)
					return ERROR_INVALIDPACKET;

				return OnRecvSkillSetList( pSession );
			}
			break;

		case eSkill::CS_SAVE_SKILLSET:
			{
				CSSaveSkillSet * pPacket = (CSSaveSkillSet*)pData;

				if (sizeof(CSSaveSkillSet) - sizeof(pPacket->SKills) + (sizeof(TSkillSetPartialData) * pPacket->cCount) != nLen)
					return ERROR_INVALIDPACKET;

				return OnRecvSaveSkillSet( pSession, pPacket );
			}
			break;

		case eSkill::CS_DELETE_SKILLSET:
			{
				if (sizeof(CSDeleteSkillSet) != nLen)
					return ERROR_INVALIDPACKET;

				return OnRecvDeleteSkillSet( pSession, (CSDeleteSkillSet*)pData );
			}
			break;
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE
		default:
			return ERROR_UNKNOWN_HEADER;
	}
}

int CDnSkillTask::OnRecvSkillLevelUpReq( CDNUserSession* pSession, CSSkillLevelUpReq* pPacket )
{
	return SkillLevelUp(pSession, pPacket->nSkillID, pPacket->cUseSkillPoint);	
}

int CDnSkillTask::OnRecvAcquireSkillReq( CDNUserSession* pSession, CSAcquireSkillReq* pPacket )
{
	return AcquireSkill(pSession, pPacket->nSkillID);	
}

int CDnSkillTask::OnRecvUnlockSkillByMoneyReq( CDNUserSession* pSession, CSUnlockSkillByMoneyReq* pPacket )
{
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
	if( ERROR_NONE != UnLockSkillByMoney( pSession, pPacket->nSkillID ) )
#else
	if( false == UnLockSkillByMoney( pSession, pPacket->nSkillID ) )
#endif
		return ERROR_SKILL_UNLOCK_FAIL;

	return ERROR_NONE;
}

int CDnSkillTask::AcquireSkill(CDNUserSession* pSession, int nSkillID)
{
	// Check if it is in unlocked skills
	CDnPlayerActor* pPlayerActor = pSession->GetPlayerActor();
	_ASSERT( pPlayerActor );

	if( pPlayerActor )
	{
		// #26902 Temporary skill added state. UI is blocked on client, but also handle incoming packets.
		if( pPlayerActor->IsTempSkillAdded() )
			return ERROR_GENERIC_INVALIDREQUEST;

		int nRetCode = pPlayerActor->AcquireSkill( nSkillID );
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return nRetCode;
#else
		pSession->SendAcquireSkill( nSkillID, nRetCode );
#endif
	}
	return ERROR_GENERIC_INVALIDREQUEST;
}

int CDnSkillTask::SkillLevelUp(CDNUserSession* pSession, int nSkillID, BYTE cUseSkillPoint)
{
	// TODO: If the way game server writes user data to DB changes later, this must also be updated.
	// Currently the game server writes the skill list to the DB automatically when the client disconnects.
	int nError = -1;
	char cLevel = 0;

	// Check if the skill is owned
	// If the skill is not owned, query to find out what the initial data is.
	DnActorHandle hActor = pSession->GetActorHandle();
	if( !hActor )
		return ERROR_GENERIC_INVALIDREQUEST;

	// Temporary skill has been added. Handle even if packet arrives despite UI being blocked on client.
	if( pSession->GetPlayerActor() &&
		pSession->GetPlayerActor()->IsTempSkillAdded() )
		return ERROR_GENERIC_INVALIDREQUEST;

	DnSkillHandle hFindedSkill = hActor->FindSkill( nSkillID );

	if( hFindedSkill )
	{
		// Village server has no skills added by equipment. Therefore level-up is impossible.
		if( hFindedSkill->IsEquipItemSkill() )
			nError = ERROR_SKILL_LEVELUPFAIL_EQUIPITEMSKILL;
		else
		// Cannot level up during aura/toggling.
		if( hFindedSkill->IsToggleOn() || hFindedSkill->IsAuraOn() )
			nError = ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT;
		else
		{
			// If an instant skill or auto passive skill is being cast, wait until it finishes before processing level-up..
			CDnSkill::SkillTypeEnum eSkillType = hFindedSkill->GetSkillType();
			if( (/*(eSkillType == CDnSkill::Active) &&*/ (hFindedSkill == hActor->GetProcessSkill())) ||
				((eSkillType == CDnSkill::AutoPassive) && (hActor->IsProcessingAutoPassive(nSkillID))) )
			{
				// Ignore consecutive level-up requests for the same skill.
				list<S_SKILL_LEVELUP_INFO*>::iterator iter = m_listWaitSkillLevelup.begin();
				for( iter; m_listWaitSkillLevelup.end() != iter; ++iter )
				{
					if( (*iter)->nSkillID == nSkillID )
						return ERROR_NONE;
				}

				S_SKILL_LEVELUP_INFO* pSkillLevelupInfo = new S_SKILL_LEVELUP_INFO;
				pSkillLevelupInfo->hActor = hActor;
				pSkillLevelupInfo->hSkill = hFindedSkill;
				pSkillLevelupInfo->nSkillID = nSkillID;
				pSkillLevelupInfo->cUseSkillPoint = cUseSkillPoint;
				pSkillLevelupInfo->pSession = pSession;

				m_listWaitSkillLevelup.push_back( pSkillLevelupInfo );

				return ERROR_NONE;
			}

			int iLevelFrom = hFindedSkill->GetLevel();
			
			//Calculate actual level.
			int nLevelUpValue = hFindedSkill->GetLevelUpValue();
			iLevelFrom -= nLevelUpValue;

			DNTableFileFormat* pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

			vector<int> vlSkillLevelList;
			//if( pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", pPacket->nSkillID, vlSkillLevelList ) == 0 ) 
			GetSkillLevelList( nSkillID, vlSkillLevelList, CDnSkill::PVE );
			if( vlSkillLevelList.empty() )
			{
				_ASSERT( "SkillTable data error" );
				nError = ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT;
			}

#if defined(PRE_FIX_NEXTSKILLINFO)
			int nLevelUpCount = 1;	// Skill level always increases by 1.

			SKILL_LEVEL_TABLE_IDS::iterator findIter;
			SKILL_LEVEL_INFO* pTableInfo = g_pDataManager->GetSkillLevelTableIDList(hFindedSkill->GetClassID(), hFindedSkill->GetSelectedLevelDataType());

			int nNeedSkillPoint = 0;
			bool bValidPacket = false;
			if( NULL != pTableInfo )
			{
				int nSkillLevel = iLevelFrom + nLevelUpCount;
				findIter = pTableInfo->_SkillLevelTableIDs.find(nSkillLevel);

				int SkillLevelTableID = -1;
				if (findIter != pTableInfo->_SkillLevelTableIDs.end())
					SkillLevelTableID = findIter->second;

				if( -1 != SkillLevelTableID )
				{
					nNeedSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( SkillLevelTableID, "_NeedSkillPoint" )->GetInteger();
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
					if( nNeedSkillPoint > 0)
#else
					if( nNeedSkillPoint == cUseSkillPoint )
#endif // #if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
						bValidPacket = true;
				}
			}

			if( false == bValidPacket )
			{
				// Packet data differs from the server, so handle as an error and return.
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
				return ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT;
#else
				pSession->SendSkillLevelUp(nSkillID, cLevel, ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT);	// Replaced with this since a send function already exists.
				return ERROR_NONE;
#endif //#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
			}
#else
			int iSkillLevelTableID = -1;
			for( int i = 0; i < (int)vlSkillLevelList.size(); ++i )
			{
				int iNowLevel = pSkillLevelTable->GetFieldFromLablePtr( vlSkillLevelList.at(i), "_SkillLevel" )->GetInteger();
				if( iNowLevel == iLevelFrom )
				{
					iSkillLevelTableID = vlSkillLevelList.at( i );
					break;
				}
			}

			int nNeedSkillPoint = 0;
			int nLevelUpCount = 0;
			while( cUseSkillPoint != nNeedSkillPoint && 0 < iSkillLevelTableID )
			{
				++nLevelUpCount;
				nNeedSkillPoint += pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID+nLevelUpCount, "_NeedSkillPoint" )->GetInteger();

				if( cUseSkillPoint < nNeedSkillPoint )
				{
					// Note: Server and client table values differ. Possible hacking.
					nError = ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT;
					break;
				}
			}

#endif // PRE_FIX_NEXTSKILLINFO

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
			bool bAvailableSPByJob = false;

			std::vector<int> nNeedSPValues;
			GetNeedSPValuesByJob(nSkillID, nNeedSPValues);
			
			std::vector<int> jobHistory;
			CDnPlayerActor* pPlayerActor = pSession->GetPlayerActor();
			if (pPlayerActor)
				pPlayerActor->GetJobHistory(jobHistory);

			bAvailableSPByJob = IsAvailableSPByJob(jobHistory, nNeedSPValues, pPlayerActor);
			
			if (bAvailableSPByJob == false)
				nError = ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT;
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP
			


			int iAvailSPByJob = pSession->GetPlayerActor()->GetAvailSkillPointByJob( nSkillID );
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
			if( nNeedSkillPoint <= iAvailSPByJob )
#else
			if( nNeedSkillPoint <= iAvailSPByJob && nNeedSkillPoint == cUseSkillPoint )
#endif
			{
				// Check skill level limit and remaining SP
#if defined(PRE_FIX_NEXTSKILLINFO)
				
				int iLevelTableIDToUp = -1;
				int iMaxLevelTableID = -1;
				int nSkillLevelUp = iLevelFrom + nLevelUpCount;

				//Skill table ID to level up
				findIter = pTableInfo->_SkillLevelTableIDs.find(nSkillLevelUp);
				if (findIter != pTableInfo->_SkillLevelTableIDs.end())
					iLevelTableIDToUp = findIter->second;

				//MaxLevel table
				findIter = pTableInfo->_SkillLevelTableIDs.find(pTableInfo->_MaxLevel);
				if (findIter != pTableInfo->_SkillLevelTableIDs.end())
					iMaxLevelTableID = findIter->second;

				if (nSkillLevelUp <= pTableInfo->_MaxLevel)
#else
				int iLevelTableIDToUp = hFindedSkill->GetSkillLevelID() + nLevelUpCount;
				int iMaxLevelTableID = hFindedSkill->GetSkillLevelID() - (hFindedSkill->GetLevel()-1) + hFindedSkill->GetMaxLevel()-1;
				if( iLevelTableIDToUp <= iMaxLevelTableID )
#endif // PRE_FIX_NEXTSKILLINFO
				{
					int iLevelLimit = pSkillLevelTable->GetFieldFromLablePtr( iLevelTableIDToUp, "_LevelLimit" )->GetInteger();

					if( iLevelLimit <= hActor->GetLevel() )
					{
						// When a passive skill that increases HP/SP is removed, current HP/SP reverts to base HP/SP without the skill
						// Since it gets clipped, fetch in advance and set it.
						INT64 iHP = hActor->GetHP();
						int iSP = hActor->GetSP();

						float fElapsedDelayTime = hFindedSkill->GetElapsedDelayTime();
						bool bRemoveSuccess = hActor->RemoveSkill( nSkillID );
						//_ASSERT( bRemoveSuccess && "CDnSkillTask::OnRecvSkillLevelUpReq() -> Attempting to level up a skill not owned by the actor." );

						if( bRemoveSuccess )
						{
							if( 0 < pSession->GetSkillPoint() /*&&
								pPacket->cUseSkillPoint <= pUserData->Attribute.wSkillPoint*/ )
							{
								int iNewLevel = iLevelFrom + nLevelUpCount;
								hActor->AddSkill( nSkillID, iNewLevel );
								DnSkillHandle hNewSkill = hActor->FindSkill( nSkillID );
								hNewSkill->SetElapsedDelayTime( fElapsedDelayTime );

								hActor->SetHP( iHP );
								hActor->SetSP( iSP );

								pSession->ChangeSkillPoint(-nNeedSkillPoint, nSkillID, false, 0 );
								pSession->GetDBConnection()->QueryModSkillLevel( pSession, nSkillID, iNewLevel, (int)( fElapsedDelayTime * 1000.f ), -nNeedSkillPoint, DBDNWorldDef::SkillChangeCode::Use);	// Save to DB: also update skill points
								
								cLevel = (char)iNewLevel;

								CDnPlayerActor * pPlayerActor = (CDnPlayerActor *)hActor.GetPointer();
								pPlayerActor->ReplacementGlyph( hNewSkill );

								pSession->GetEventSystem()->OnEvent( EventSystem::OnSkillLevelUp, 2, EventSystem::SkillID, nSkillID, EventSystem::SkillLevel, cLevel );
								nError = ERROR_NONE;
							}
							else
							{
								// Insufficient skill points for level-up. Return ERROR_SK_LEVELUP_NOT_ENOUGH_POINT to client
								nError = ERROR_SKILL_LEVELUPFAIL_INSUFFICIENCY_SKILLPOINT;
							}
						}
					}
					else
					{
						//TODO: Character level is lower than the skill's required level..
						nError = ERROR_SKILL_LEVELUPFAIL_INSUFFICIENCY_CHARACTERLEVEL;
					}
				}
				else
				{
					//TODO: Trying to raise skill level above the configured maximum..
					nError = ERROR_SKILL_LEVELUPFAIL_ALREADY_MAX;
				}
			}
			else
			{
				// TODO: Owned points are less than actually used skill points or differ from table data.
				nError = ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT;
			}
		}
	}
	else
	{
		// Does not own the skill. Return ERROR_SK_LEVELUP_NOT_EXIST_FAIL to client
		nError = ERROR_SKILL_ISNOT_YOURS;
	}
	
#if !defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
	pSession->SendSkillLevelUp(nSkillID, cLevel, nError);	// Replaced with this since a send function already exists.
#endif
	
	//hActor->ReplacementSkill( pPacket->nSkillID, cLevel );

	if (nError != ERROR_NONE)
		return ERROR_NONE;

	// Note: Must notify other players for stat changes caused by passive buff skills.
	DWORD dwNumPartyMember = pSession->GetGameRoom()->GetUserCount();
	if( 0 < nSkillID && 1 < dwNumPartyMember )
	{
		int iPartyMemberIndex = -1;
		for (DWORD i = 0; i < dwNumPartyMember; i++)
		{
			CDNGameRoom::PartyStruct *pStruct = pSession->GetGameRoom()->GetPartyData(i);
			if( pStruct == NULL ) continue;
			if( pStruct->pSession == pSession )
				iPartyMemberIndex = i;
		}
		
		_ASSERT( -1 != iPartyMemberIndex );
		if( -1 != iPartyMemberIndex )
		{
			for (DWORD i = 0; i < dwNumPartyMember; i++)
			{
				CDNGameRoom::PartyStruct *pStruct = pSession->GetGameRoom()->GetPartyData(i);
				if( pStruct == NULL ) continue;
				if( pStruct->pSession != pSession )
					pStruct->pSession->SendOtherPlayerSkillLevelUp( iPartyMemberIndex, nSkillID, cLevel );
			}
		}
	}
	return ERROR_NONE;
}

#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
int CDnSkillTask::OnRecvReservationSkillListReq( CDNUserSession* pSession, CSReservationSkillListReq* pUnlockSkillByMoneyReq )
{
	if( pUnlockSkillByMoneyReq->nCount <= 0 )
		return ERROR_GENERIC_UNKNOWNERROR;	

	SCReservationSkillListAck ReservationSkillAck;
	memset(&ReservationSkillAck, 0, sizeof(SCReservationSkillListAck));

	for( int i=0; i<pUnlockSkillByMoneyReq->nCount; ++i)
	{
		memcpy(&ReservationSkillAck.tReservationSkillAck[i].tReservationSkill, &pUnlockSkillByMoneyReq->tReservationSkill[i], sizeof(TReservationSkillReq));
		if(pUnlockSkillByMoneyReq->tReservationSkill[i].cType == ReservationSKillList::Type::UnLock)
		{			
			ReservationSkillAck.tReservationSkillAck[i].nResult = UnLockSkillByMoney( pSession, pUnlockSkillByMoneyReq->tReservationSkill[i].nSkillID);
		}
		else if(pUnlockSkillByMoneyReq->tReservationSkill[i].cType == ReservationSKillList::Type::Acquire)
		{		
			ReservationSkillAck.tReservationSkillAck[i].nResult = AcquireSkill(pSession, pUnlockSkillByMoneyReq->tReservationSkill[i].nSkillID);
		}
		else if(pUnlockSkillByMoneyReq->tReservationSkill[i].cType == ReservationSKillList::Type::LevelUp)
		{
			int nResult = ERROR_NONE;
			DnActorHandle hActor = pSession->GetActorHandle();
			if( hActor )
			{
				DnSkillHandle hFindedSkill = hActor->FindSkill( pUnlockSkillByMoneyReq->tReservationSkill[i].nSkillID );
				if( hFindedSkill )
				{
					int nCurLevel = hFindedSkill->GetLevel();

					//Calculate actual level.
					int nLevelUpValue = hFindedSkill->GetLevelUpValue();
					nCurLevel -= nLevelUpValue;				
					if( nCurLevel >= pUnlockSkillByMoneyReq->tReservationSkill[i].nLevel)
					{
						nResult = ERROR_GENERIC_UNKNOWNERROR;
					}
					else
					{
						for( int j=nCurLevel;j<pUnlockSkillByMoneyReq->tReservationSkill[i].nLevel;++j)
						{
							nResult = SkillLevelUp(pSession, pUnlockSkillByMoneyReq->tReservationSkill[i].nSkillID, 0);
							if( nResult != ERROR_NONE)
								break;
						}
					}
				}
				else
					nResult = ERROR_GENERIC_UNKNOWNERROR;			
			}
			else
				nResult = ERROR_GENERIC_UNKNOWNERROR;
			ReservationSkillAck.tReservationSkillAck[i].nResult = nResult;			
		}
		else
			break;
		++ReservationSkillAck.nCount;
		if(ReservationSkillAck.tReservationSkillAck[i].nResult != ERROR_NONE ) // If even 1 error occurs midway, just finish up..
			break;
	}
	int nLen = sizeof(SCReservationSkillListAck) - sizeof(ReservationSkillAck.tReservationSkillAck) + (sizeof(TReservationSkillAck)*ReservationSkillAck.nCount);
	pSession->AddSendData(SC_SKILL, eSkill::SC_RESERVATION_SKILL_LIST_ACK, (char*)&ReservationSkillAck, nLen);
	return ERROR_NONE;
}
#endif

#if defined(PRE_ADD_PRESET_SKILLTREE)
int CDnSkillTask::OnRecvSkillSetList( CDNUserSession* pSession )
{
	if (pSession->GetDBConnection())
		pSession->GetDBConnection()->QueryGetSkillPresetList(pSession);

	return ERROR_NONE;
}

int CDnSkillTask::OnRecvSaveSkillSet( CDNUserSession* pSession, CSSaveSkillSet *pPacket )
{
	if (wcslen(pPacket->wszSetName) <= 0)
		return ERROR_GENERIC_UNKNOWNERROR;

	if (pPacket->cIndex < 0 || pPacket->cIndex >= SKILLPRESETMAX)
		return ERROR_GENERIC_UNKNOWNERROR;

	if (pSession->GetDBConnection())
		pSession->GetDBConnection()->QueryAddSkillPreset(pSession, pPacket);

	return ERROR_NONE;
}

int CDnSkillTask::OnRecvDeleteSkillSet( CDNUserSession* pSession, CSDeleteSkillSet *pPacket )
{
	if (pSession->GetDBConnection())
		pSession->GetDBConnection()->QueryDelSkillPreset(pSession, pPacket->cIndex);

	return ERROR_NONE;
}
#endif	// #if defined(PRE_ADD_PRESET_SKILLTREE)

void CDnSkillTask::GetSkillLevelList( int iSkillID, vector<int>& vlSkillLevelList, int iLevelDataType )
{
	DNTableFileFormat* pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

	pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", iSkillID, vlSkillLevelList );

	// Check and filter whether the target is PvE or PvP.
	vector<int>::iterator iterLevelList = vlSkillLevelList.begin();
	for( iterLevelList; iterLevelList != vlSkillLevelList.end(); )
	{
		int iSkillLevelTableID = *iterLevelList;
		int iApplyType = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_ApplyType" )->GetInteger();
		if( iApplyType != iLevelDataType )
			iterLevelList = vlSkillLevelList.erase( iterLevelList );
		else
			++iterLevelList;
	}
}

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
bool CDnSkillTask::IsAvailableSPByJob(std::vector<int>& jobHistory, vector<int>& needSPValues, CDnPlayerActor * pPlayerActor)
{
	bool isAvailableSPByJob = true;

	int nJobCount = (int)jobHistory.size();
	int nJobID = -1;

	for (int i = 0; i < nJobCount; ++i)
	{
		if (IsAvailableSPByJob(jobHistory, needSPValues, i, pPlayerActor) == false)
		{
			isAvailableSPByJob = false;
			break;
		}
	}

	return isAvailableSPByJob;
}

bool CDnSkillTask::IsAvailableSPByJob(std::vector<int>& jobHistory, vector<int>& needSPValues, int nIndex, CDnPlayerActor * pPlayerActor)
{
	bool isAvailableSPByJob = false;

	int nJobCount = (int)jobHistory.size();

	int nNeedSPValue = 0;
	int nSPValueCount = (int)needSPValues.size();
	if (nIndex >= 0 && nIndex < nSPValueCount)
		nNeedSPValue = needSPValues[nIndex];

	//When job change has not occurred??
	if (nIndex >= nJobCount)
	{
		if (nNeedSPValue <= 0)
			return true;
		else
			return false;
	}

	int nJobID = -1;

	nJobID = jobHistory[nIndex];
	int iUsedSkillPointInThisJob = pPlayerActor->GetUsedSkillPointInThisJob( nJobID );

	//Usable only if the SP spent on the JobID is greater than the check value.
	if (iUsedSkillPointInThisJob >= nNeedSPValue)
		isAvailableSPByJob = true;

	return isAvailableSPByJob;
}

void CDnSkillTask::GetNeedSPValuesByJob(int nSkillID, std::vector<int>& nNeedSPValues)
{
	CDnSkillTreeSystem* pSkillTreeSystem = g_pDataManager->GetSkillTreeSystem();
	if (pSkillTreeSystem)
		pSkillTreeSystem->GetNeedSPValuesByJob(nSkillID, nNeedSPValues);
}
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP