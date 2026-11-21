#include "stdafx.h"
#include "DNUserSkill.h"
#include "DNUserSession.h"
#include "DNGameDataManager.h"
#include "DNLogConnection.h"
#include "DNMissionSystem.h"
#include "DNDBConnectionManager.h"
#include "DNDBConnection.h"

extern TVillageConfig g_Config;

CDNUserSkill::CDNUserSkill(CDNUserSession *pUserObj)
: m_pUserSession(pUserObj)
{

	m_pSkillData = m_pUserSession->GetSkillData( true );
	for(DWORD i=0; i<DualSkill::Type::MAX; i++ )
		m_SkillList[i].clear();
}

CDNUserSkill::~CDNUserSkill(void)
{
	for(DWORD i=0; i<DualSkill::Type::MAX; i++ )
		m_SkillList[i].clear();
}

void CDNUserSkill::LoadUserData()
{
	int nUseSkillPoint = 0;

	int nOpendSkillPageCount = m_pUserSession->GetItem()->GetSkillPageCount();

	for(int nSkillCount =0 ; nSkillCount < nOpendSkillPageCount; nSkillCount++)
	{
		for (int i = 0; i < SKILLMAX; i++){
			if (m_pSkillData[nSkillCount].SkillList[i].nSkillID <= 0) continue;

			// If a skill ID not in data is saved in DB, do not load it.
			const TSkillData* pSkillData = g_pDataManager->GetSkillData( m_pSkillData[nSkillCount].SkillList[i].nSkillID );
			if( NULL == pSkillData )
				continue;

			m_SkillList[nSkillCount][m_pSkillData[nSkillCount].SkillList[i].nSkillID] = m_pSkillData[nSkillCount].SkillList[i];

			CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
			SkillInfo.iSkillID = m_pSkillData[nSkillCount].SkillList[i].nSkillID;
			SkillInfo.iSkillLevel = m_pSkillData[nSkillCount].SkillList[i].cSkillLevel;
			SkillInfo.bCurrentLock = m_pSkillData[nSkillCount].SkillList[i].bLock;

			m_vlPossessedSkill[nSkillCount].push_back( SkillInfo );


			if (m_pSkillData[nSkillCount].SkillList[i].nSkillID > 0){
				for (int j = 0; j < m_pSkillData[nSkillCount].SkillList[i].cSkillLevel; j++){
					nUseSkillPoint += g_pDataManager->GetNeedSkillPoint(m_pSkillData[nSkillCount].SkillList[i].nSkillID, j + 1);
				}
			}
		}

		// Check crest skill
		const TItem *pItem = m_pUserSession->GetItem()->GetGlyph( GLYPH_SPECIALSKILL );
		if( pItem && pItem->nItemID ) {
			if( pItem->nItemID > 0 ) {
				TItemData *pItemData = g_pDataManager->GetItemData( pItem->nItemID );
				if( pItemData && (pItemData->nSkillID > 0) ) {
					TSkillData *pSkillData = g_pDataManager->GetSkillData( pItemData->nSkillID );
					TSkill Skill;
					Skill.nSkillID = pSkillData->nSkillID;
					Skill.cSkillLevel = 1;
					Skill.nCoolTime = 0;
					Skill.bLock = false;

					m_SkillList[nSkillCount][pItemData->nSkillID] = Skill;

					CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
					SkillInfo.iSkillID = pItemData->nSkillID;
					SkillInfo.iSkillLevel = Skill.cSkillLevel;
					SkillInfo.bCurrentLock = Skill.bLock;

					m_vlPossessedSkill[nSkillCount].push_back( SkillInfo );

				}
			}
		}
	}

#ifdef PRE_ADD_SP_REVISION

#ifdef _FINAL_BUILD
	// Verify owned skill points. Even if mismatched, adjust without sending a separate packet to the client. 
	// Because info is sent to the client after passing this point.
	CheckAndRevisionSkillPoint( false );
#endif // #ifdef _FINAL_BUILD

#endif // #ifdef PRE_ADD_SP_REVISION
}

void CDNUserSkill::SaveUserData(TUserData *pUserData)
{
	if (m_SkillList[GetCurrentSkillPage()].empty()) return;
	memset(&(pUserData->Skill[DualSkill::Type::Primary].SkillList), 0, sizeof(TSkill) * SKILLMAX);

	TMapSkillList::iterator iter;
	int nCount = 0;
	for (iter = m_SkillList[GetCurrentSkillPage()].begin(); iter != m_SkillList[GetCurrentSkillPage()].end(); ++iter){
		pUserData->Skill[DualSkill::Type::Primary].SkillList[nCount] = iter->second;
		pUserData->Skill[DualSkill::Type::Primary].SkillList[nCount].nCoolTime = 0;
		nCount++;
	}
}

// Function used only by the addskill cheat after switching to the skill tree...
void CDNUserSkill::AddSkill( int nSkillID, int iLevel/* = 0*/ )
{
	// 20080901 Log server skill added [acquired]
	if ( nSkillID <= 0 )
	{
		m_pUserSession->SendAddSkill(nSkillID, ERROR_SKILL_ACQUIREFAIL);
		return;
	}

	// Cannot acquire an already acquired skill again.
	if( FindSkill( nSkillID ) )
	{
		m_pUserSession->SendAddSkill(nSkillID, ERROR_SKILL_ACQUIREFAIL);
		return;
	}

	TSkill Skill;
	memset(&Skill, 0, sizeof(TSkill));
	Skill.nSkillID = nSkillID;
	Skill.cSkillLevel = iLevel;
	//Skill.bLock = true; // Initially unlocked state when skill is acquired. (Level 0 state)
	Skill.bLock = false;

	m_SkillList[GetCurrentSkillPage()][nSkillID] = Skill;
	
	CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
	SkillInfo.iSkillID = nSkillID;
	SkillInfo.iSkillLevel = Skill.cSkillLevel;
	SkillInfo.bCurrentLock = Skill.bLock;

	m_vlPossessedSkill[GetCurrentSkillPage()].push_back( SkillInfo );

	if( m_pUserSession )
	{
		m_pUserSession->GetDBConnection()->QueryAddSkill(m_pUserSession, nSkillID, Skill.cSkillLevel, 0, 0, DBDNWorldDef::SkillChangeCode::GainByAdmin);
		m_pUserSession->SendAddSkill(nSkillID, ERROR_NONE);

		m_pUserSession->GetEventSystem()->OnEvent( EventSystem::OnSkillAdd, 1, EventSystem::SkillID, nSkillID );
	}
}
 
void CDNUserSkill::DelSkill(int nSkillID)
{
	if (nSkillID <= 0){
		m_pUserSession->SendDelSkill(nSkillID, ERROR_SKILL_DELETEFAIL);
		return;
	}

	if (m_SkillList[GetCurrentSkillPage()].empty()){
		m_pUserSession->SendDelSkill(nSkillID, ERROR_SKILL_ISNOT_YOURS);
		return;
	}

	TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].find(nSkillID);
	if (iter != m_SkillList[GetCurrentSkillPage()].end()){
		m_SkillList[GetCurrentSkillPage()].erase(iter);

		vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO>::iterator iter = m_vlPossessedSkill[GetCurrentSkillPage()].begin();
		for( iter; m_vlPossessedSkill[GetCurrentSkillPage()].end() != iter; ++iter )
		{
			if( iter->iSkillID == nSkillID )
			{
				m_vlPossessedSkill[GetCurrentSkillPage()].erase( iter );
				break;
			}
		}

		m_pUserSession->SendDelSkill(nSkillID, ERROR_NONE);
		return;
	}

	if( m_pUserSession )
	{
		m_pUserSession->GetDBConnection()->QueryDelSkill(m_pUserSession, nSkillID, DBDNWorldDef::SkillChangeCode::DelByAdmin, false, GetCurrentSkillPage() );
		m_pUserSession->SendDelSkill(nSkillID, ERROR_SKILL_ISNOT_YOURS);
	}
}

#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
int CDNUserSkill::SkillLevelUp( int nSkillID, int nUsePoint )
#else
void CDNUserSkill::SkillLevelUp( int nSkillID, int nUsePoint )
#endif
{
#ifdef PRE_ADD_SP_REVISION

	// #29463 Fix mismatch between current skill points and owned skill list here.
#ifdef _FINAL_BUILD
	CheckAndRevisionSkillPoint();
#endif // #ifdef _FINAL_BUILD

#endif // #ifdef PRE_ADD_SP_REVISION

	bool bReqSuccess = false;
	int nError = ERROR_NONE;

	bool bFinded = false;

	if (nSkillID <= 0)
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_GENERIC_UNKNOWNERROR;;
#else
		return;
#endif//#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)

	if (m_SkillList[GetCurrentSkillPage()].empty()) 
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_GENERIC_UNKNOWNERROR;;
#else
		return;
#endif//#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)		

	TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].find(nSkillID);
	if (iter != m_SkillList[GetCurrentSkillPage()].end())
		bFinded = true;
	else
	{
		_ASSERT( "Skill not found in skill list." );
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_GENERIC_UNKNOWNERROR;;
#else
		return;
#endif //#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
	}

	TSkill& SkillInfo = iter->second;

	DNTableFileFormat* pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

	vector<int> vlSkillLevelList;
	if( pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", nSkillID, vlSkillLevelList ) == 0 ) 
	{
		_ASSERT( "SkillTable data error" );
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_GENERIC_UNKNOWNERROR;;
#else
		return;
#endif//#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)		
	}

#if defined(PRE_FIX_NEXTSKILLINFO)
	int nLevelUpCount = 1;	// Skill level always increases by 1.
	
	SKILL_LEVEL_TABLE_IDS::iterator findIter;
	SKILL_LEVEL_INFO* pTableInfo = g_pDataManager->GetSkillLevelTableIDList(nSkillID, 0);

	int nNeedSkillPoint = 0;
	bool bValidPacket = false;
	if( NULL != pTableInfo )
	{
		int nSkillLevel = SkillInfo.cSkillLevel + nLevelUpCount;
		findIter = pTableInfo->_SkillLevelTableIDs.find(nSkillLevel);

		int SkillLevelTableID = -1;
		if (findIter != pTableInfo->_SkillLevelTableIDs.end())
			SkillLevelTableID = findIter->second;
	
		if( -1 != SkillLevelTableID )
		{
			nNeedSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( SkillLevelTableID, "_NeedSkillPoint" )->GetInteger();
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
			if( nNeedSkillPoint > 0 )
#else
			if( nNeedSkillPoint == nUsePoint )
#endif
				bValidPacket = true;
		}
	}
	
	if( false == bValidPacket )
	{
		// Packet data differs from the server, so handle as an error and return.
		nError = ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT;
		bFinded = false;
	}
#else
	int iSkillLevelTableID = -1;
	for( int i = 0; i < (int)vlSkillLevelList.size(); ++i )
	{
		int iNowLevel = pSkillLevelTable->GetFieldFromLablePtr( vlSkillLevelList.at(i), "_SkillLevel" )->GetInteger();
		if( iNowLevel == SkillInfo.cSkillLevel )
		{
			iSkillLevelTableID = vlSkillLevelList.at( i );
			break;
		}
	}

	if( -1 == iSkillLevelTableID )
	{
		_ASSERT( "SkillTable data error" );
		return;
	}

	int nNeedSkillPoint = 0;
	int nLevelUpCount = 0;
	while( nUsePoint != nNeedSkillPoint )
	{
		++nLevelUpCount;
		nNeedSkillPoint += pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID+nLevelUpCount, "_NeedSkillPoint" )->GetInteger();

		if( nUsePoint < nNeedSkillPoint )
		{
			nError = ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT;
			bFinded = false;
			break;
		}
	}
#endif // PRE_FIX_NEXTSKILLINFO

	// Check if the skill is owned
	// If the skill is not owned, query to find out what the initial data is.
	//bool bFinded = FindSkill( nSkillID );

	if( bFinded )
	{	
		// Owned skill points are less than the points requested to spend
		int iAvailSPByJob = GetAvailSkillPointByJob( nSkillID );
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		if( nNeedSkillPoint <= iAvailSPByJob )
#else
		if( nNeedSkillPoint <= iAvailSPByJob &&	nUsePoint == nNeedSkillPoint )
#endif
		{
			int iLevelFrom = GetSkillLevel( nSkillID );

			// Check skill level limit
			TSkillData *pSkillData = g_pDataManager->GetSkillData(nSkillID);
			if (!pSkillData){
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
				return ERROR_SKILL_LEVELUPFAIL_ISNOT_YOURS;;
#else
				m_pUserSession->SendSkillLevelUp( nSkillID, 0, ERROR_SKILL_LEVELUPFAIL_ISNOT_YOURS );
				return;
#endif//#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)				
			}

			int iNewLevel = iLevelFrom + nLevelUpCount;
			if( iNewLevel <= pSkillData->nMaxLevel )
			{
				TSkillLevelData *pSkillLevel = g_pDataManager->GetSkillLevelData(nSkillID, iNewLevel);
				if (!pSkillLevel){
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
					return ERROR_SKILL_LEVELUPFAIL_ISNOT_YOURS;
#else
					m_pUserSession->SendSkillLevelUp( nSkillID, 0, ERROR_SKILL_LEVELUPFAIL_ISNOT_YOURS );
					return;
#endif
				}

				if( pSkillLevel->cLevelLimit <= (int)m_pUserSession->GetLevel() )
				{
					if( 0 < m_pUserSession->GetSkillPoint() )
					{
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
						SetSkillLevel( nSkillID, iNewLevel, DBDNWorldDef::SkillChangeCode::Use, false );	// Deduct within skill points
#else
						SetSkillLevel( nSkillID, iNewLevel, DBDNWorldDef::SkillChangeCode::Use );	// Deduct within skill points
#endif //#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
						bReqSuccess = true;
					}
					else
					{
						// Insufficient skill points for level-up. Return ERROR_SKILL_LEVELUPFAIL_INSUFFICIENCY_SKILLPOINT to client
						nError = ERROR_SKILL_LEVELUPFAIL_INSUFFICIENCY_SKILLPOINT;
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
	else
	{
		// Does not own the skill. Return ERROR_SKILL_LEVELUPFAIL_ISNOT_YOURS to client
		nError = ERROR_SKILL_LEVELUPFAIL_ISNOT_YOURS;
	}
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
	return nError;
#else
	if (!bReqSuccess)
		m_pUserSession->SendSkillLevelUp( nSkillID, 0, nError );
#endif
}

bool CDNUserSkill::FindSkill(int nSkillID)
{
	if (nSkillID <= 0) return false;

	if (m_SkillList[GetCurrentSkillPage()].empty()) return false;

	TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].find(nSkillID);
	if (iter != m_SkillList[GetCurrentSkillPage()].end()){
		return true;
	}

	return false;
}

int CDNUserSkill::GetSkillLevel( int nSkillID )
{

	TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].find( nSkillID );
	if( iter != m_SkillList[GetCurrentSkillPage()].end() ) {
		return iter->second.cSkillLevel;
	}
	return -1;
}

void CDNUserSkill::SetSkillLevel( int nSkillID, int nLevel, int nLogCode, bool bSendSkillLevelUp /*= true*/ )
{
	TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].find( nSkillID );
	if( iter != m_SkillList[GetCurrentSkillPage()].end() ) {
		int nGap = nLevel - iter->second.cSkillLevel;
		iter->second.cSkillLevel = nLevel;
		
		//////////////////////////////////////////////////////////////////////////
		// Like game server, also account for cases where level changes all at once.
		// Currently only increases by 1 level at a time, but match it with the game server.
		int nNeedSkillPoint = 0;
		for( int i = 0; i < nGap; ++i )
			nNeedSkillPoint += g_pDataManager->GetNeedSkillPoint(nSkillID, nLevel-i);
		//////////////////////////////////////////////////////////////////////////

		if (nNeedSkillPoint > 0) m_pUserSession->ChangeSkillPoint(-nNeedSkillPoint, nSkillID, false, 0, GetCurrentSkillPage() );
		m_pUserSession->GetDBConnection()->QueryModSkillLevel( m_pUserSession, nSkillID, nLevel, iter->second.nCoolTime, -nNeedSkillPoint, nLogCode);	// Save to DB: also update skill points

		if( bSendSkillLevelUp )
			m_pUserSession->SendSkillLevelUp( nSkillID, nLevel, ERROR_NONE );

		vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO>::iterator iter = m_vlPossessedSkill[GetCurrentSkillPage()].begin();
		for( iter; m_vlPossessedSkill[GetCurrentSkillPage()].end() != iter; ++iter )
		{
			if( iter->iSkillID == nSkillID )
			{
				iter->iSkillLevel = nLevel;
				break;
			}
		}

		m_pUserSession->GetEventSystem()->OnEvent( EventSystem::OnSkillLevelUp, 2, EventSystem::SkillID, nSkillID, EventSystem::SkillLevel, nLevel );
	}
}


// Unlock skill. Add to character's owned skills at level 0.
void CDNUserSkill::UnLockSkill( int nSkillID )
{
	TSkill Skill;
	memset(&Skill, 0, sizeof(TSkill));
	Skill.nSkillID = nSkillID;
	Skill.cSkillLevel = 0;
	Skill.bLock = false;		// Lock variable probably not needed

	m_SkillList[GetCurrentSkillPage()][nSkillID] = Skill;

	// Add to a separate vector containing only the needed info.
	CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
	SkillInfo.iSkillID = nSkillID;
	SkillInfo.iSkillLevel = 0;
	SkillInfo.bCurrentLock = false;
	m_vlPossessedSkill[GetCurrentSkillPage()].push_back( SkillInfo );

	m_pUserSession->SendUnlockSkill( nSkillID, ERROR_NONE );
	m_pUserSession->GetDBConnection()->QueryAddSkill(m_pUserSession, nSkillID, SkillInfo.iSkillLevel, 0, DBDNWorldDef::SkillChangeCode::GainByBook, 0);

	// TODO: Ask mission side about skill acquisition and handle it.
	//m_pUserSession->GetEventSystem()->OnEvent( EventSystem::OnSkillAdd, 1, EventSystem::SkillID, nSkillID );

	return;
}

// Unlock with gold via Skill Trainer NPC in village. Currently handles up to acquisition.
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
int CDNUserSkill::UnLockSkillByMoney( int nSkillID )
#else
void CDNUserSkill::UnLockSkillByMoney( int nSkillID )
#endif
{
#ifdef PRE_ADD_SP_REVISION

	// #29463 Fix mismatch between current skill points and owned skill list here.
#ifdef _FINAL_BUILD
	CheckAndRevisionSkillPoint();
#endif // #ifdef _FINAL_BUILD

#endif // #ifdef PRE_ADD_SP_REVISION

	if (nSkillID <= 0){
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_ACQUIREFAIL;
#else
		m_pUserSession->SendAcquireSkill( nSkillID, ERROR_SKILL_ACQUIREFAIL );
		return;
#endif //#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
	}

	// Check if owned money is sufficient.
	TSkillData* pSkillData = g_pDataManager->GetSkillData( nSkillID );
	if (!pSkillData){
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_ACQUIREFAIL;
#else
		m_pUserSession->SendAcquireSkill( nSkillID, ERROR_SKILL_ACQUIREFAIL );
		return;
#endif
	}

	// Cannot learn skills that are mutually exclusive.
	if( IsExclusiveSkill( nSkillID, pSkillData->nExclusiveID ) )
	{
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE;
#else
		m_pUserSession->SendAcquireSkill( nSkillID, ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE );
		return;
#endif
	}

	// #36858 If linked as global skills, learning any one in the group allows learning without parent skill prerequisites.
	bool bAlreadyGlobalSkillAcquired = false;
	if( HasSameGlobalIDSkill( pSkillData->nGlobalSkillGroup ) )
		bAlreadyGlobalSkillAcquired = true;

	INT64 iNowMoney = m_pUserSession->GetCoin();
	if( iNowMoney < pSkillData->nUnlockPrice )
	{
		//m_pUserSession->SendUnlockSkill( nSkillID, ERROR_SKILL_UNLOCK_NOT_ENOUGH_MONEY );
		// Since client handles displaying all message handling upon acquisition...
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_UNLOCK_NOT_ENOUGH_MONEY;
#else
		m_pUserSession->SendAcquireSkill( nSkillID, ERROR_SKILL_UNLOCK_NOT_ENOUGH_MONEY );
		return;
#endif
	}

	bool bAlreadyPossess = this->FindSkill( nSkillID );
	if( bAlreadyPossess )
	{
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_SKILL_UNLOCK_ALREADY_OPEN;
#else
		m_pUserSession->SendAcquireSkill( nSkillID, ERROR_SKILL_UNLOCK_ALREADY_OPEN );
		return;
#endif
	}

	// Then check the acquisition conditions and acquire it.
	int nRetCode = ERROR_NONE;
	CDnSkillTreeSystem* pSkillTreeSystem = g_pDataManager->GetSkillTreeSystem();

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
	CDnSkillTreeSystem::S_TRY_ACQUIRE TryAcquire( GetPossessedSkillInfo() );
	TryAcquire.iCurrentCharLevel = m_pUserSession->GetLevel();
	TryAcquire.iTryAcquireSkillID = nSkillID;
	// Based on the usable SP defined per job.
	TryAcquire.iHasSkillPoint = GetAvailSkillPointByJob( nSkillID );

	pSkillTreeSystem->TryAcquireSkill( TryAcquire, &Output );

	bool bIgnoreParentSkillCondition = ( (CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL == Output.eResult) ||
										 (CDnSkillTreeSystem::R_LOCKED_PARENTSKILL == Output.eResult) ||
										 (CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL == Output.eResult) ) &&
										 true == bAlreadyGlobalSkillAcquired;

	if( CDnSkillTreeSystem::R_SUCCESS == Output.eResult || true == bIgnoreParentSkillCondition )
	{
		// Add skill to the skill list.
		TSkill Skill;
		memset(&Skill, 0, sizeof(TSkill));
		Skill.nSkillID = nSkillID;
		Skill.cSkillLevel = 0;
		Skill.bLock = false;		// Lock variable probably not needed

		m_SkillList[GetCurrentSkillPage()][nSkillID] = Skill;

		// Add to a separate vector containing only the needed info.
		CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
		SkillInfo.iSkillID = nSkillID;
		SkillInfo.iSkillLevel = 0;
		SkillInfo.bCurrentLock = false;

		m_vlPossessedSkill[GetCurrentSkillPage()].push_back( SkillInfo );

		// Deduct money
		m_pUserSession->DelCoin( pSkillData->nUnlockPrice, DBDNWorldDef::CoinChangeCode::DoNotDBSave, 0 );
		
		// Send unlock packet to client.
#if !defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		m_pUserSession->SendUnlockSkill( nSkillID, ERROR_NONE );
#endif // #if !defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		m_pUserSession->GetDBConnection()->QueryAddSkill(m_pUserSession, nSkillID, SkillInfo.iSkillLevel, 0, DBDNWorldDef::SkillChangeCode::GainByBuy, pSkillData->nUnlockPrice);

		// TODO: Ask mission side about skill acquisition and handle it.
		//m_pUserSession->GetEventSystem()->OnEvent( EventSystem::OnSkillAdd, 1, EventSystem::SkillID, nSkillID );

		SetSkillLevel(nSkillID, 1, DBDNWorldDef::SkillChangeCode::Use, false);	// Modify skill points inside the function

		// Send final notification of acquisition success. CDnSkillTreeSystem::S_OUTPUT Output;CDnSkillTreeSystem::S_OUTPUT Output;
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return ERROR_NONE;
#else
		m_pUserSession->SendAcquireSkill( nSkillID, ERROR_NONE );
#endif
	}
	else
	{
		switch( Output.eResult )
		{
				// Character level requirement not met.
			case CDnSkillTreeSystem::R_NOT_ENOUGH_CHAR_LEVEL:
				nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_CHAR_LEVEL;
				break;

				// No prerequisite (parent) skill.
			case CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL:
				nRetCode = ERROR_SKILL_ACQUIRE_FAIL_DONT_HAVE_PARENT_SKILL;
				break;

				// Parent skill level requirement not met.
			case CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL:
				nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_PARENT_SKILL_LEVEL;
				break;

				// Cannot acquire skill due to insufficient skill points.
			case CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE:
				nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_SKILLPOINT;
				break;

			default:
				nRetCode = ERROR_SKILL_UNLOCK_FAIL;
				break;
		}
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		return nRetCode;
#else
		m_pUserSession->SendAcquireSkill( nSkillID, nRetCode );
#endif
	}
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
	return ERROR_NONE;
#endif
}

bool CDNUserSkill::UseSkillBook( int nItemID )
{
#ifdef PRE_ADD_SP_REVISION

	// #29463 Fix mismatch between current skill points and owned skill list here.
#ifdef _FINAL_BUILD
	CheckAndRevisionSkillPoint();
#endif // #ifdef _FINAL_BUILD

#endif // #ifdef PRE_ADD_SP_REVISION

	CDnSkillTreeSystem* pSkillTreeSystem = g_pDataManager->GetSkillTreeSystem();

	// Changed so using a skill book or paying a trainer handles both skill unlock and acquisition in one batch.
	// When all checks pass for insufficient SP, missing prerequisite skill, low prerequisite skill level, and low character level
	// Unlock the skill and level it to 1.

	// Cannot unlock an already unlocked skill again. 
	// Consider whether a level 0 unlocked skill should count as owned.
	int iSkillIDToUnLock = pSkillTreeSystem->FindSkillBySkillBook( nItemID );
	if( 0 == iSkillIDToUnLock )
	{
		g_Log.Log( LogType::_ERROR, L"[UseSkillBook Fail!!!] Can't find SkillID that using %d SkillBook Item ID\n", iSkillIDToUnLock );
		_ASSERT( !"No skill found for the specified skillbook item ID." );
		return false;
	}

	// Cannot learn skills that are mutually exclusive.
	const TSkillData* pSkillData = g_pDataManager->GetSkillData( iSkillIDToUnLock );
	if( IsExclusiveSkill( iSkillIDToUnLock, pSkillData->nExclusiveID ) )
	{
		m_pUserSession->SendAcquireSkill( iSkillIDToUnLock, ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE );
		return false;
	}

	// #36858 If linked as global skills, learning any one in the group allows learning without parent skill prerequisites.
	bool bAlreadyGlobalSkillAcquired = false;
	if( HasSameGlobalIDSkill( pSkillData->nGlobalSkillGroup ) )
		bAlreadyGlobalSkillAcquired = true;

	bool bSuccess = false;
	bool bAlreadyPossess = this->FindSkill( iSkillIDToUnLock );
	int nRetCode = ERROR_NONE;
	int nResultSkillID = 0;
	if( false == bAlreadyPossess )
	{
		CDnSkillTreeSystem::S_OUTPUT Output;

		CDnSkillTreeSystem::S_TRY_UNLOCK CurrentSkillInfo;
		CurrentSkillInfo.iTryUnlockSkillID = iSkillIDToUnLock;
		CurrentSkillInfo.iCurrentCharLevel = m_pUserSession->GetLevel();
		CurrentSkillInfo.iSkillBookItemID = nItemID;

		// Find if the job required by the skill is in history and handle accordingly.
		TSkillData* pSkillData = g_pDataManager->GetSkillData( iSkillIDToUnLock );

		if (pSkillData == NULL) 
		{
			g_Log.Log( LogType::_ERROR, L"[UseSkillBook - GetSkillData Fail!!!] Can't find %d SkillID\n", iSkillIDToUnLock );
			return false;
		}

		CurrentSkillInfo.iJobID = -1;
		for( int i = 0; i < JOBMAX; ++i )
		{
			BYTE cJob = m_pUserSession->GetStatusData()->cJobArray[ i ];
			if( 0 < cJob )
			{
				if( cJob == pSkillData->nNeedJobID )
				{				
					CurrentSkillInfo.iJobID = pSkillData->nNeedJobID;
				}
			}
			else
				break;
		}

		if( -1 != CurrentSkillInfo.iJobID )
		{
			pSkillTreeSystem->TryUnLockSkill( CurrentSkillInfo, &Output );

			// Notify client based on result.
			if( CDnSkillTreeSystem::R_SUCCESS == Output.eResult )
			{
				// Success.
				nResultSkillID = iSkillIDToUnLock;

				// Then check the acquisition conditions.
				CDnSkillTreeSystem* pSkillTreeSystem = g_pDataManager->GetSkillTreeSystem();
				CDnSkillTreeSystem::S_TRY_ACQUIRE TryAcquire( GetPossessedSkillInfo() );
				TryAcquire.iCurrentCharLevel = m_pUserSession->GetLevel();
				TryAcquire.iTryAcquireSkillID = nResultSkillID;
				// Based on the usable SP defined per job.
				TryAcquire.iHasSkillPoint = GetAvailSkillPointByJob( nResultSkillID );

				pSkillTreeSystem->TryAcquireSkill( TryAcquire, &Output );

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
				std::vector<int> nNeedSPValues;
				pSkillTreeSystem->GetNeedSPValuesByJob(nResultSkillID, nNeedSPValues);

				std::vector<int> jobHistory;
				GetJobHistory(m_pUserSession->GetStatusData()->cJobArray, jobHistory);

				bool bAvailableSPByJob = IsAvailableSPByJob(jobHistory, nNeedSPValues);

				if (bAvailableSPByJob == false)
					Output.eResult = CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE;
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP

				bool bIgnoreParentSkillCondition = ( (CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL == Output.eResult) ||
													 (CDnSkillTreeSystem::R_LOCKED_PARENTSKILL == Output.eResult) ||
													 (CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL == Output.eResult) ) &&
													 true == bAlreadyGlobalSkillAcquired;

				if( CDnSkillTreeSystem::R_SUCCESS == Output.eResult ||
					true == bIgnoreParentSkillCondition )
				{
					// After notifying client, retrieve skill info from user object, set level to 0, and add skill.
					UnLockSkill( nResultSkillID );  // Called inside SendUnlockSkill() to send the packet.
					bSuccess = true;

					SetSkillLevel( nResultSkillID, 1, DBDNWorldDef::SkillChangeCode::Use, false );
					
					// Send final notification of acquisition success.
					m_pUserSession->SendAcquireSkill( nResultSkillID, ERROR_NONE );

				}
				else
				{
					switch( Output.eResult )
					{
							// Character level requirement not met.
						case CDnSkillTreeSystem::R_NOT_ENOUGH_CHAR_LEVEL:
							nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_CHAR_LEVEL;
							break;

							// No prerequisite (parent) skill.
						case CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL:
							nRetCode = ERROR_SKILL_ACQUIRE_FAIL_DONT_HAVE_PARENT_SKILL;
							break;

							//// Parent skill is locked.
							//case CDnSkillTreeSystem::R_LOCKED_PARENTSKILL:
							//	break;

							// Parent skill level requirement not met.
						case CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL:
							nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_PARENT_SKILL_LEVEL;
							break;

							// Cannot acquire skill due to insufficient skill points.
						case CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE:
							nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_SKILLPOINT;
							break;
					}

				}
			}
			else
			{
				nRetCode = ERROR_SKILL_UNLOCK_FAIL;
			}
		}
		else
		{
			nRetCode = ERROR_SKILL_UNLOCK_MISMATCH_JOB;
		}
	}
	else
	{
		// Already unlocked.
		nRetCode = ERROR_SKILL_UNLOCK_ALREADY_OPEN;
	}

	if( false == bSuccess )
	{
		//m_pUserSession->SendUnlockSkill( nResultSkillID, nRetCode );
		m_pUserSession->SendAcquireSkill( nResultSkillID, nRetCode );
	}

	return bSuccess;
}


// Acquire skill. (Change level from 0 to 1)
// Called when re-acquiring a skill after a skill reset.
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
int CDNUserSkill::AcquireSkill( int nSkillID )
#else
void CDNUserSkill::AcquireSkill( int nSkillID )
#endif
{
#ifdef PRE_ADD_SP_REVISION

	// #29463 Fix mismatch between current skill points and owned skill list here.
#ifdef _FINAL_BUILD
	CheckAndRevisionSkillPoint();
#endif // #ifdef _FINAL_BUILD

#endif // #ifdef PRE_ADD_SP_REVISION

	TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].find( nSkillID );
	int nRetCode = ERROR_NONE;
	if( iter != m_SkillList[GetCurrentSkillPage()].end() )
	{
		_ASSERT( false == iter->second.bLock );
		_ASSERT( 0 == iter->second.cSkillLevel );

		// Check for mutually exclusive skills when re-acquiring after skill reset
		// Cannot learn skills that are mutually exclusive.
		TSkillData* pSkillData = g_pDataManager->GetSkillData( nSkillID );
		if( m_pUserSession->GetSkill()->IsExclusiveSkill( nSkillID, pSkillData->nExclusiveID ) )
		{
			// Basically blocked on the client, so if a packet is sent to the server, it's a hack.
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)			
			return ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE;
#else
			m_pUserSession->SendAcquireSkill( nSkillID, ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE );
			return;
#endif // #if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		}

		// #36858 If linked as global skills, learning any one in the group allows learning without parent skill prerequisites.
		bool bAlreadyGlobalSkillAcquired = false;
		if( HasSameGlobalIDSkill( pSkillData->nGlobalSkillGroup ) )
			bAlreadyGlobalSkillAcquired = true;

		CDnSkillTreeSystem* pSkillTreeSystem = g_pDataManager->GetSkillTreeSystem();
		CDnSkillTreeSystem::S_TRY_ACQUIRE TryAcquire( GetPossessedSkillInfo() );
		CDnSkillTreeSystem::S_OUTPUT Output;

		TryAcquire.iCurrentCharLevel = m_pUserSession->GetLevel();
		TryAcquire.iTryAcquireSkillID = nSkillID;
		// Based on the usable SP defined per job.
		TryAcquire.iHasSkillPoint = GetAvailSkillPointByJob( nSkillID );
		
		// Find if the job required by the skill is in history and handle accordingly.
		TryAcquire.iJobID = -1;
		if (pSkillData){
			for( int i = 0; i < JOBMAX; ++i )
			{
				BYTE cJob = m_pUserSession->GetStatusData()->cJobArray[ i ];
				if( 0 < cJob )
				{
					if( cJob == pSkillData->nNeedJobID )
					{				
						TryAcquire.iJobID = pSkillData->nNeedJobID;
					}
				}
				else
					break;
			}
		}

		if( -1 != TryAcquire.iJobID )
		{
			pSkillTreeSystem->TryAcquireSkill( TryAcquire, &Output );

			bool bIgnoreParentSkillCondition = ( (CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL == Output.eResult) ||
												 (CDnSkillTreeSystem::R_LOCKED_PARENTSKILL == Output.eResult) ||
												 (CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL == Output.eResult) ) &&
												 true == bAlreadyGlobalSkillAcquired;

			if( CDnSkillTreeSystem::R_SUCCESS == Output.eResult ||
				true == bIgnoreParentSkillCondition )
			{
				SetSkillLevel( nSkillID, 1, DBDNWorldDef::SkillChangeCode::Use, false );

				// TODO: Ask mission side about skill acquisition and handle it.
				//m_pUserSession->GetEventSystem()->OnEvent( EventSystem::OnSkillAdd, 1, EventSystem::SkillID, nSkillID );
			}
			else
			{
				switch( Output.eResult )
				{
					// Character level requirement not met.
				case CDnSkillTreeSystem::R_NOT_ENOUGH_CHAR_LEVEL:
					nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_CHAR_LEVEL;
					break;

					// No prerequisite (parent) skill.
				case CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL:
					nRetCode = ERROR_SKILL_ACQUIRE_FAIL_DONT_HAVE_PARENT_SKILL;
					break;

					//// Parent skill is locked.
					//case CDnSkillTreeSystem::R_LOCKED_PARENTSKILL:
					//	break;

					// Parent skill level requirement not met.
				case CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL:
					nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_PARENT_SKILL_LEVEL;
					break;

					// Cannot acquire skill due to insufficient skill points.
				case CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE:
					nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_SKILLPOINT;
					break;
				}
			}
		}
		else
			nRetCode = ERROR_SKILL_ACQUIRE_FAIL_MISMATCH_JOB;
	}
	else
	{
		// Does not own the skill. Return ERROR_SKILL_LEVELUPFAIL_ISNOT_YOURS to client
		nRetCode = ERROR_SKILL_ACQUIRE_FAIL_ISNOT_YOURS;
	}
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)			
	return nRetCode;
#else
	m_pUserSession->SendAcquireSkill( nSkillID, nRetCode );
#endif
}


// Cheat-key-only skill acquisition function..
void CDNUserSkill::CheatAcquireSkill( int nSkillID )
{
	TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].find( nSkillID );
	int nRetCode = ERROR_NONE;
	if( iter != m_SkillList[GetCurrentSkillPage()].end() )
	{
		SetSkillLevel( nSkillID, 1, DBDNWorldDef::SkillChangeCode::ModSkillLevelByAdmin, false );
		m_pUserSession->SendAcquireSkill( nSkillID, nRetCode );
	}
}

#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
void CDNUserSkill::ReservationSkillList(CSReservationSkillListReq* pUnlockSkillByMoneyReq)
{
	if( pUnlockSkillByMoneyReq->nCount <= 0 )
		return;	

	SCReservationSkillListAck ReservationSkillAck;
	memset(&ReservationSkillAck, 0, sizeof(SCReservationSkillListAck));

	for( int i=0; i<pUnlockSkillByMoneyReq->nCount; ++i)
	{
		memcpy(&ReservationSkillAck.tReservationSkillAck[i].tReservationSkill, &pUnlockSkillByMoneyReq->tReservationSkill[i], sizeof(TReservationSkillReq));
		if(pUnlockSkillByMoneyReq->tReservationSkill[i].cType == ReservationSKillList::Type::UnLock)
		{			
			ReservationSkillAck.tReservationSkillAck[i].nResult = UnLockSkillByMoney(pUnlockSkillByMoneyReq->tReservationSkill[i].nSkillID);
		}
		else if(pUnlockSkillByMoneyReq->tReservationSkill[i].cType == ReservationSKillList::Type::Acquire)
		{		
			ReservationSkillAck.tReservationSkillAck[i].nResult = AcquireSkill(pUnlockSkillByMoneyReq->tReservationSkill[i].nSkillID);
		}
		else if(pUnlockSkillByMoneyReq->tReservationSkill[i].cType == ReservationSKillList::Type::LevelUp)
		{
			int nResult = ERROR_NONE;
			int nCurLevel = GetSkillLevel(pUnlockSkillByMoneyReq->tReservationSkill[i].nSkillID);
			if( nCurLevel >= pUnlockSkillByMoneyReq->tReservationSkill[i].nLevel)
			{
				nResult = ERROR_GENERIC_UNKNOWNERROR;			
			}
			else
			{
				for( int j=nCurLevel;j<pUnlockSkillByMoneyReq->tReservationSkill[i].nLevel;++j)
				{
					nResult = SkillLevelUp(pUnlockSkillByMoneyReq->tReservationSkill[i].nSkillID);
					if( nResult != ERROR_NONE)
						break;
				}
			}
			ReservationSkillAck.tReservationSkillAck[i].nResult = nResult;			
		}
		else
			break;
		++ReservationSkillAck.nCount;
		if(ReservationSkillAck.tReservationSkillAck[i].nResult != ERROR_NONE ) // If even 1 error occurs midway, just finish up..
			break;
	}
	int nLen = sizeof(SCReservationSkillListAck) - sizeof(ReservationSkillAck.tReservationSkillAck) + (sizeof(TReservationSkillAck)*ReservationSkillAck.nCount);
	m_pUserSession->AddSendData(SC_SKILL, eSkill::SC_RESERVATION_SKILL_LIST_ACK, (char*)&ReservationSkillAck, nLen);
}
#endif //#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)


void CDNUserSkill::ResetSkill( int nSkillPage )
{
	// Reset all skills on job change.
	// Must handle the rest using cash items.

	ResetAllSkill( nSkillPage );
	m_pUserSession->GetDBConnection()->QueryResetSkill(m_pUserSession, nSkillPage);
}


void CDNUserSkill::WaitForUseSkillResetCashItemFromDBServer( vector<int>& vlSkillIDsToReset )
{
	m_vlWaitingSkillIDsToReset = vlSkillIDsToReset;
}

void CDNUserSkill::OnResponseSkillResetCashItemFromDBServer( bool bSuccess )
{
	if( bSuccess )
	{
		// Reset stored skill IDs. Skill points are handled immediately upon receiving from DB, so no need to worry about them here.
		int aiDefaultSkills[ DEFAULTSKILLMAX ] = { 0 };
		g_pDataManager->GetCreateDefaultSkill( m_pUserSession->GetClassID(), aiDefaultSkills );

		int nCount = 0;
		for( int i = 0; i != (int)m_vlWaitingSkillIDsToReset.size(); ++i )
		{

			TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].find( m_vlWaitingSkillIDsToReset.at(i) );
			_ASSERT( m_SkillList[GetCurrentSkillPage()].end() != iter );
			if( m_SkillList[GetCurrentSkillPage()].end() == iter )
				continue;

			bool bDefaultSkill = false;
			for( int k = 0; k < DEFAULTSKILLMAX; ++k )
			{
				if( aiDefaultSkills[ k ] == iter->second.nSkillID )
				{
					bDefaultSkill = true;
					break;
				}
			}

			if( bDefaultSkill )
			{
				iter->second.cSkillLevel = 1;
			}
			else
				iter->second.cSkillLevel = 0;

			for( UINT k = 0; k < m_vlPossessedSkill[GetCurrentSkillPage()].size(); ++k )
			{
				if( iter->second.nSkillID == m_vlPossessedSkill[GetCurrentSkillPage()].at( k ).iSkillID )
					m_vlPossessedSkill[GetCurrentSkillPage()].at( k ).iSkillLevel = iter->second.cSkillLevel;
			}
			nCount++;
		}
	}
	
	m_vlWaitingSkillIDsToReset.clear();
}


void CDNUserSkill::GatherThisJobSkill( BYTE cJob, IN OUT vector<int>& vlResult )
{
	// If performance becomes an issue later, build and use a separate list.

	vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO>::iterator iter = m_vlPossessedSkill[GetCurrentSkillPage()].begin();
	for ( iter; iter != m_vlPossessedSkill[GetCurrentSkillPage()].end(); ++iter )
	{
		TSkillData* pSkillData = g_pDataManager->GetSkillData( iter->iSkillID );
		if (!pSkillData) continue;
		if( pSkillData->nNeedJobID == cJob )
		{
			// Must be acquired to actually possess the skill.
			if( 0 < iter->iSkillLevel )
				vlResult.push_back( iter->iSkillID );
		}
	}
}

int CDNUserSkill::GetLevelUpSkillPoint( int nPrevLevel, int nCurLevel )
{
	if( nPrevLevel == nCurLevel ) return 0;

	int nSkillPoint = 0;
	for( int i=nPrevLevel+1; i<=nCurLevel; i++ ) {
		nSkillPoint += CPlayerLevelTable::GetInstance().GetValue( m_pUserSession->GetClassID(), i, CPlayerLevelTable::SkillPoint );
	}
	return nSkillPoint;
}


bool CDNUserSkill::IsExclusiveSkill( int nSkillID, int nExclusiveID )
{
	if( 0 != nExclusiveID )
	{
		TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].begin();
		for(iter; iter != m_SkillList[GetCurrentSkillPage()].end(); ++iter)
		{
			const TSkill& ExistSkillData = iter->second;
			if( ExistSkillData.nSkillID <= 0 )
				continue;

			if( nSkillID == ExistSkillData.nSkillID )
				continue;

			// Skills reset to level 0 were not acquired, so skip comparison.
			if( 0 == ExistSkillData.cSkillLevel )
				continue;

			const TSkillData* pSkillData = g_pDataManager->GetSkillData( ExistSkillData.nSkillID );
			if( 0 != pSkillData->nExclusiveID )
			{
				if( nExclusiveID == pSkillData->nExclusiveID )
					return true;
			}
		}
	}

	return false;
}

int CDNUserSkill::GetUsedSPByJob( int nJobID )
{
	int iResult = 0;

	for( int i = 0; i < (int)m_vlPossessedSkill[GetCurrentSkillPage()].size(); ++i )
	{
		int iSkillID = m_vlPossessedSkill[GetCurrentSkillPage()].at( i ).iSkillID;
		const TSkillData* pSkillData = g_pDataManager->GetSkillData( iSkillID );

		// NULL check only in final build. Let it crash during testing..
#if defined(_FINAL_BUILD)
		if( NULL == pSkillData )
			continue;
#endif // #if defined(_FINAL_BUILD)

		if( pSkillData->nNeedJobID == nJobID )
		{
			int iLevel = m_vlPossessedSkill[GetCurrentSkillPage()].at( i ).iSkillLevel;
			int iSize = (int)pSkillData->vLevelDataList.size();
			for( int k = 0; k < min(iLevel, iSize); ++k )
				iResult += pSkillData->vLevelDataList.at( k ).nNeedSkillPoint;

			if (iLevel > iSize)
				g_Log.Log(LogType::_ERROR, L"GetUsedSPByJob - Wrong Skill Size(WORSETID:%d, CHARID:%d, iLevel:%d, iSize:%d)\r\n", m_pUserSession->GetWorldSetID(), m_pUserSession->GetCharacterDBID(), iLevel, iSize);
		}
	}
	return iResult;
}

int CDNUserSkill::GetAvailSkillPointByJob( int nSkillID )
{
	int iWholeSP = GetLevelUpSkillPoint( 1, m_pUserSession->GetLevel() );
	float fAvailSPRatioByJob = m_pUserSession->GetAvailSkillPointRatioByJob( nSkillID );
	const TSkillData* pSkillDataToLevelUp = g_pDataManager->GetSkillData( nSkillID );
	int iUsedSPThisJob = 0;
	if (pSkillDataToLevelUp)
		iUsedSPThisJob = GetUsedSPByJob( pSkillDataToLevelUp->nNeedJobID );

	// If remaining job SP exceeds total usable SP, total usable SP is authoritative, so return that amount.
	int iAvailPoint = int(fAvailSPRatioByJob*iWholeSP)-iUsedSPThisJob;
	if( m_pUserSession->GetSkillPoint() < iAvailPoint )
		iAvailPoint = m_pUserSession->GetSkillPoint();

	return iAvailPoint;
}

void CDNUserSkill::OnAttachEquip( TItem *pItem )
{
	if( pItem->nItemID > 0 ) {
		TItemData *pItemData = g_pDataManager->GetItemData( pItem->nItemID );
		if( pItemData && (pItemData->nSkillID > 0) ) {
			if( !FindSkill( pItemData->nSkillID ) ) {
				TSkill Skill;
				memset(&Skill, 0, sizeof(TSkill));
				Skill.nSkillID = pItemData->nSkillID;
				Skill.cSkillLevel = 1;
				Skill.bLock = false;
				m_SkillList[GetCurrentSkillPage()][pItemData->nSkillID] = Skill;

				CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
				SkillInfo.iSkillID = pItemData->nSkillID;
				SkillInfo.iSkillLevel = Skill.cSkillLevel;
				SkillInfo.bCurrentLock = Skill.bLock;

				m_vlPossessedSkill[GetCurrentSkillPage()].push_back( SkillInfo );
			}
		}
	}
}

void CDNUserSkill::OnDetachEquip( TItem *pItem )
{
	if( pItem->nItemID > 0 ) {
		TItemData *pItemData = g_pDataManager->GetItemData( pItem->nItemID );
		if( pItemData && (pItemData->nSkillID > 0) ) {
			TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].find( pItemData->nSkillID );
			if (iter != m_SkillList[GetCurrentSkillPage()].end()){
				m_SkillList[GetCurrentSkillPage()].erase(iter);

				vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO>::iterator iter = m_vlPossessedSkill[GetCurrentSkillPage()].begin();
				for( iter; m_vlPossessedSkill[GetCurrentSkillPage()].end() != iter; ++iter )
				{
					if( iter->iSkillID == pItemData->nSkillID )
					{
						m_vlPossessedSkill[GetCurrentSkillPage()].erase( iter );
						break;
					}
				}
			}
		}
	}
}


#ifdef PRE_ADD_SP_REVISION

#ifdef _FINAL_BUILD
void CDNUserSkill::CheckAndRevisionSkillPoint( bool bSendRevisionSPToClient/* = true*/ )
{
	if( NULL == m_pUserSession )
		return;

	// #29463 Fix mismatch between current skill points and owned skill list here.
	int nUseSkillPoint = 0;
	int nWholeSP = GetLevelUpSkillPoint( 1, m_pUserSession->GetLevel() );

	// Fetch default skills; if only default skills exist at level 1, no skills have been learned so reset is not allowed.
	int aiDefaultSkills[ DEFAULTSKILLMAX ] = { 0 };
	g_pDataManager->GetCreateDefaultSkill( m_pUserSession->GetClassID(), aiDefaultSkills );

	int iNumSkill = (int)m_vlPossessedSkill[GetCurrentSkillPage()].size();
	for( int i = 0; i < iNumSkill; ++i )
	{
		CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO& PossessedSkill = m_vlPossessedSkill[GetCurrentSkillPage()].at( i );

		// Exclude default skill at level 1 from skill point calculation.
		bool bDefaultSkill = false;
		bool bDefaultSkillOneLevel = false;
		for( int k = 0; k < DEFAULTSKILLMAX; ++k )
		{
			if( aiDefaultSkills[ k ] == 0 )
				break;

			if( PossessedSkill.iSkillID == aiDefaultSkills[ k ] )
			{
				bDefaultSkill = true;
				if( 1 == PossessedSkill.iSkillLevel )
				{
					bDefaultSkillOneLevel = true;
				}
				break;
			}
		}

		if( bDefaultSkill && bDefaultSkillOneLevel )
			continue;

		for( int k = 0; k < PossessedSkill.iSkillLevel; ++k )
		{
			// Do not count skill points for level 1 default skills.
			// Required SP is set to 0 in the table, but block it in code just in case it is entered incorrectly.
			if( bDefaultSkill && 0 == k )
				continue;

			nUseSkillPoint += g_pDataManager->GetNeedSkillPoint(PossessedSkill.iSkillID, k + 1);
		}
	}

	int nLeftSkillPoint = nWholeSP - nUseSkillPoint;

	// If the value differs from the currently saved skill points, adjust it.
	int nNowSkillPoint = m_pUserSession->GetSkillPoint();
	
	// If the skill was already learned beyond the limit, skip it here so it gets corrected on the next skill reset and re-acquisition.
	if( nLeftSkillPoint < 0 )
		return;

	if( nNowSkillPoint != nLeftSkillPoint )
	{
		m_pUserSession->SetSkillPoint( nLeftSkillPoint, GetCurrentSkillPage() );

#ifdef PRE_FIX_SP_REVISION_CHANGE_SP
		m_pUserSession->GetDBConnection()->QuerySetSkillPoint( m_pUserSession, nLeftSkillPoint, GetCurrentSkillPage() );
#else
		int nSkillPointDelta = nLeftSkillPoint - nNowSkillPoint;
		if( 0 < nSkillPointDelta )
		{
			m_pUserSession->GetDBConnection()->QueryIncreaseSkillPoint( m_pUserSession, nSkillPointDelta, DBDNWorldDef::SkillPointCode::Repair, GetCurrentSkillPage() );
		}
		else
		{
			// For the decrease query, pass the skill points to deduct as a positive number and the DB will subtract them.
			m_pUserSession->GetDBConnection()->QueryDecreaseSkillPoint( m_pUserSession,	-nSkillPointDelta, GetCurrentSkillPage() );
		}
#endif // #ifdef PRE_FIX_SP_REVISION_CHANGE_SP

		// Push skill point packet to client as well.
		if( bSendRevisionSPToClient )
			m_pUserSession->SendPushSkillPoint( nLeftSkillPoint );
	}
}
#endif // #ifdef _FINAL_BUILD

#endif // #ifdef PRE_ADD_SP_REVISION

void CDNUserSkill::ResetAllSkill(int nSkillPage )
{
	// Reset all owned skills to 0 except default skills. Restore spent skill points.
	// Set default skill to level 1.
	int aiDefaultSkills[ DEFAULTSKILLMAX ] = { 0 };
	g_pDataManager->GetCreateDefaultSkill( m_pUserSession->GetClassID(), aiDefaultSkills );

	TMapSkillList::iterator iter;
	int nCount = 0;

	if( nSkillPage < DualSkill::Type::Primary || nSkillPage >= DualSkill::Type::MAX )
		return;

	for (iter = m_SkillList[nSkillPage].begin(); iter != m_SkillList[nSkillPage].end(); ++iter)
	{
		bool bDefaultSkill = false;
		for( int i = 0; i < DEFAULTSKILLMAX; ++i )
		{
			if( aiDefaultSkills[ i ] == iter->second.nSkillID )
			{
				bDefaultSkill = true;
				break;
			}
		}

		if( bDefaultSkill )
		{
			iter->second.cSkillLevel = 1;
		}
		else
			iter->second.cSkillLevel = 0;

		for( int i = 0; i < (int)m_vlPossessedSkill[nSkillPage].size(); ++i )
		{
			if( iter->second.nSkillID == m_vlPossessedSkill[nSkillPage].at( i ).iSkillID )
				m_vlPossessedSkill[nSkillPage].at( i ).iSkillLevel = iter->second.cSkillLevel;
		}

		nCount++;
	}
}


#ifdef PRE_ADD_CHANGEJOB_CASHITEM
void CDNUserSkill::OnResponseChangeJobCode( USHORT wTotalSkillPoint )
{	
	for(int nSkillPage = DualSkill::Type::Primary; nSkillPage < DualSkill::Type::MAX ; nSkillPage++ )
	{
		ResetAllSkill( nSkillPage );
		m_pUserSession->SetSkillPoint( wTotalSkillPoint, nSkillPage );
	}
}
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

bool CDNUserSkill::HasSameGlobalIDSkill( int iGlobalSkillGroupID )
{
	bool bResult = false;

	if( 0 < iGlobalSkillGroupID )
	{
		TMapSkillList::iterator iter = m_SkillList[GetCurrentSkillPage()].begin();
		for(iter; iter != m_SkillList[GetCurrentSkillPage()].end(); ++iter)
		{
			const TSkillData* pExistSkillData = g_pDataManager->GetSkillData( iter->second.nSkillID );
			if( pExistSkillData )
			{
				if( pExistSkillData->nGlobalSkillGroup == iGlobalSkillGroupID )
				{
					bResult = true;
					break;
				}
			}
		}
	}

	return bResult;
}


void CDNUserSkill::ApplyExpendedSkillPage( int nSkillPage )
{
	if( nSkillPage == GetCurrentSkillPage() || nSkillPage <= DualSkill::Type::Primary || nSkillPage >= DualSkill::Type::MAX)
		return;

	int aiDefaultSkills[ DEFAULTSKILLMAX ] = { 0 };
	int addCount = 0;
	int nUseSkillPoint = 0;

	g_pDataManager->GetCreateDefaultSkill( m_pUserSession->GetClassID(), aiDefaultSkills );

	for(int i=0; i<DEFAULTSKILLMAX; i++ )
	{
		if( aiDefaultSkills[i] > 0 )
		{
			m_pSkillData[nSkillPage].SkillList[addCount].cSkillLevel = 1;
			m_pSkillData[nSkillPage].SkillList[addCount].nSkillID = aiDefaultSkills[i];
			m_pSkillData[nSkillPage].SkillList[addCount].nCoolTime = 0;
			addCount++;
		}
	}

	for (int i = 0; i < SKILLMAX; i++){
		if (m_pSkillData[nSkillPage].SkillList[i].nSkillID <= 0) continue;

		// If a skill ID not in data is saved in DB, do not load it.
		const TSkillData* pSkillData = g_pDataManager->GetSkillData( m_pSkillData[nSkillPage].SkillList[i].nSkillID );
		if( NULL == pSkillData )
			continue;

		m_SkillList[nSkillPage][m_pSkillData[nSkillPage].SkillList[i].nSkillID] = m_pSkillData[nSkillPage].SkillList[i];

		CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
		SkillInfo.iSkillID = m_pSkillData[nSkillPage].SkillList[i].nSkillID;
		SkillInfo.iSkillLevel = m_pSkillData[nSkillPage].SkillList[i].cSkillLevel;
		SkillInfo.bCurrentLock = m_pSkillData[nSkillPage].SkillList[i].bLock;

		m_vlPossessedSkill[nSkillPage].push_back( SkillInfo );


		if (m_pSkillData[nSkillPage].SkillList[i].nSkillID > 0){
			for (int j = 0; j < m_pSkillData[nSkillPage].SkillList[i].cSkillLevel; j++){
				nUseSkillPoint += g_pDataManager->GetNeedSkillPoint(m_pSkillData[nSkillPage].SkillList[i].nSkillID, j + 1);
			}
		}
	}

}


int CDNUserSkill::GetCurrentSkillPage()
{
	return m_pUserSession->GetSkillPage();
}

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
void CDNUserSkill::GetJobHistory(BYTE *cJobArray, vector<int>& jobHistory)
{
	for (int j = 0; j < JOBMAX; j++){
		if (cJobArray[j] <= 0) continue;
		
		jobHistory.push_back((int)cJobArray[j]);
	}
}

bool CDNUserSkill::IsAvailableSPByJob(vector<int>& jobHistory, vector<int>& needSPValues)
{
	bool isAvailableSPByJob = true;

	int nJobCount = (int)jobHistory.size();
	int nJobID = -1;

	for (int i = 0; i < nJobCount; ++i)
	{
		if (IsAvailableSPByJob(jobHistory, needSPValues, i) == false)
		{
			isAvailableSPByJob = false;
			break;
		}
	}

	return isAvailableSPByJob;
}

bool CDNUserSkill::IsAvailableSPByJob(std::vector<int>& jobHistory, vector<int>& needSPValues, int nIndex)
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
	int iUsedSkillPointInThisJob = GetUsedSPByJob( nJobID );

	//Usable only if the SP spent on the JobID is greater than the check value.
	if (iUsedSkillPointInThisJob >= nNeedSPValue)
		isAvailableSPByJob = true;

	return isAvailableSPByJob;
}
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP