#include "stdafx.h"
#include "DnSkillTreeSystem.h"
#include "DnTableDB.h"
#if defined(_VILLAGESERVER) || defined(_GAMESERVER)
#include "DNGameDataManager.h"
#else
#include "DNTableFile.h"
#endif
#include "DnCommonUtil.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnSkillTreeSystem::CDnSkillTreeSystem(void)
{
}

CDnSkillTreeSystem::~CDnSkillTreeSystem(void)
{
	SAFE_DELETE_PVEC( m_vlpJobSkillTreeList );
}


bool CDnSkillTreeSystem::InitializeTable( void )
{
	bool bResult = true;

	// Get required job info, lock/unlock info, and required skill book ItemID from the skill table.
	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat* pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );
	DNTableFileFormat* pSkillTreeTable = GetDNTable( CDnTableDB::TSKILLTREE );

	int iNumSkill = pSkillTable->GetItemCount();
	for( int iIndex = 0; iIndex < iNumSkill; ++iIndex )
	{
		int iSkillID = pSkillTable->GetItemID( iIndex );

		// Get related table ID from tree table.
		vector<int> vlSkillTreeTableItemID;
		pSkillTreeTable->GetItemIDListFromField( "_SkillTableID", iSkillID, vlSkillTreeTableItemID );

		if( (int)vlSkillTreeTableItemID.empty() )
			continue;
		
		_ASSERT( (int)vlSkillTreeTableItemID.size() == 1 );
		int iTreeTableID = vlSkillTreeTableItemID.front();

		// SP required for initial skill acquisition is retrieved from the level 1 required SP in the skill level table.
		vector<int> vlSkillLevelList;
		// Just process based on PvE.
		pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", iSkillID, vlSkillLevelList );
		
		// Check and filter whether the target is PvE or PvP.
		vector<int>::iterator iterLevelList = vlSkillLevelList.begin();
		for( iterLevelList; iterLevelList != vlSkillLevelList.end(); )
		{
			int iSkillLevelTableID = *iterLevelList;
			int iApplyType = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_ApplyType" )->GetInteger();
			if( iApplyType != 0 )
				iterLevelList = vlSkillLevelList.erase( iterLevelList );
			else
				++iterLevelList;
		}

		_ASSERT( false == vlSkillLevelList.empty() );
		//
		if (vlSkillLevelList.empty() == true)
			continue;

		int iSkillLevelTableID = vlSkillLevelList.front();
		int iNeedJobID = pSkillTable->GetFieldFromLablePtr( iSkillID, "_NeedJob" )->GetInteger();

		map<int, S_SKILLTREE_INFO*>::iterator iter = m_mapSkillTreeByJobID.find( iNeedJobID );
		if( m_mapSkillTreeByJobID.end() == iter )
		{
			S_SKILLTREE_INFO* pNewTree = new S_SKILLTREE_INFO;
			pNewTree->iJobID = iNeedJobID;
			m_mapSkillTreeByJobID.insert( make_pair(iNeedJobID, pNewTree) );
			m_vlpJobSkillTreeList.push_back( pNewTree );

			pNewTree->vlNodeRenderInfo.assign( MAX_SKILL_SLOT_COUNT, S_NODE_RENDER_INFO() );
		}

		S_SKILLTREE_INFO* pTree = m_mapSkillTreeByJobID[ iNeedJobID ];

		S_NODE_INFO* pNodeInfo = new S_NODE_INFO;
		pNodeInfo->iSkillID = iSkillID;
		pNodeInfo->iJobID = iNeedJobID;
		pNodeInfo->iNeedSkillPointToAcquire = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_NeedSkillPoint" )->GetInteger();
		pNodeInfo->iTreeSlotIndex = pSkillTreeTable->GetFieldFromLablePtr( iTreeTableID, "_TreeSlotIndex" )->GetInteger();
		pNodeInfo->bDefaultLocked = (pSkillTable->GetFieldFromLablePtr( iSkillID, "_Lock" )->GetInteger() == 1) ? true : false;
		
#if defined( PRE_ADD_PRESET_SKILLTREE )
		char szLabel[256] = {0,};
		for( int itr = 0; itr < E_ADVICE_SKILL_LEVEL_COUNT; ++itr )
		{
			sprintf_s( szLabel, _countof(szLabel), "_AdviceSkillLevel%d", itr + 1 );
			int nAdviceSkillLevel = pSkillTreeTable->GetFieldFromLablePtr( iTreeTableID, szLabel )->GetInteger();
			pNodeInfo->vecAdviceSkillLevel.push_back( nAdviceSkillLevel );
		}
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

#if defined( PRE_ADD_ONLY_SKILLBOOK )
		pNodeInfo->bNeedSkillBook = (pSkillTreeTable->GetFieldFromLablePtr( iTreeTableID, "_NeedSkillBook" )->GetInteger() == 1) ? true : false;
#endif	// #if defined( PRE_ADD_ONLY_SKILLBOOK )

		// Acquisition condition info
		char caLabel[ 32 ];
		pNodeInfo->UnLockCondition.iUnlockSkillBookItemID = pSkillTable->GetFieldFromLablePtr( iSkillID, "_UnlockSkillBookItemID" )->GetInteger();
		pNodeInfo->AcquireCondition.iNeedLevel = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_LevelLimit" )->GetInteger();

		for( int i = 0; i < MAX_PARENT_SKILL_COUNT; ++i )
		{
			// Undefined value may be 0.
			sprintf_s( caLabel, "_NeedParentSkillLevel%d", i + 1 );
			int iNeedLevel = pSkillTreeTable->GetFieldFromLablePtr( iTreeTableID, caLabel )->GetInteger();

			if( 0 != iNeedLevel )
				pNodeInfo->AcquireCondition.vlParentSkillNeedLevel.push_back( iNeedLevel );
		}

		// Max slot size exceeded. No choice other than terminating the program!
		_ASSERT( pNodeInfo->iTreeSlotIndex < (int)pTree->vlNodeRenderInfo.size() );

		pTree->vlNodeRenderInfo.at( pNodeInfo->iTreeSlotIndex ) = S_NODE_RENDER_INFO( iSkillID, pNodeInfo->iTreeSlotIndex, iNeedJobID, 
																					  &pNodeInfo->UnLockCondition, &pNodeInfo->AcquireCondition );
		pNodeInfo->pNodeRenderInfo = &(pTree->vlNodeRenderInfo[pNodeInfo->iTreeSlotIndex]);
		pTree->vlpNodeList.push_back( pNodeInfo );
		pTree->mapNodeBySkillID.insert( make_pair(iSkillID, pNodeInfo) );
		m_mapNodeListBySkillBookID.insert( make_pair(pNodeInfo->UnLockCondition.iUnlockSkillBookItemID, pNodeInfo) );
	}

	// Build parent-child hierarchy info and tree structure data.
	_MakeupParentSkillLinks( pSkillTreeTable, pSkillTable );
	
	return bResult;
}


#if defined(_VILLAGESERVER) || defined(_GAMESERVER)
bool CDnSkillTreeSystem::InitializeTableUsingDataManager( CDNGameDataManager* pDataManager )
{
	bool bResult = true;

	// Get required job info, lock/unlock info, and required skill book ItemID from the skill table.
	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );

	int iNumSkill = pSkillTable->GetItemCount();
	for( int iIndex = 0; iIndex < iNumSkill; ++iIndex )
	{
		int iSkillID = pSkillTable->GetItemID( iIndex );

		// Get related table ID from tree table.
		TSkillTreeData* pSkillTreeData = pDataManager->GetSkillTreeData( iSkillID );
		if( NULL == pSkillTreeData )
			continue;

		// SP required for initial skill acquisition is retrieved from the level 1 required SP in the skill level table.
		const TSkillData* pSkillData = pDataManager->GetSkillData( iSkillID );
		int iNeedJobID = pSkillData->nNeedJobID;
		
		map<int, S_SKILLTREE_INFO*>::iterator iter = m_mapSkillTreeByJobID.find( iNeedJobID );
		if( m_mapSkillTreeByJobID.end() == iter )
		{
			S_SKILLTREE_INFO* pNewTree = new S_SKILLTREE_INFO;
			pNewTree->iJobID = iNeedJobID;
			m_mapSkillTreeByJobID.insert( make_pair(iNeedJobID, pNewTree) );
			m_vlpJobSkillTreeList.push_back( pNewTree );

			pNewTree->vlNodeRenderInfo.assign( MAX_SKILL_SLOT_COUNT, S_NODE_RENDER_INFO() );
		}
		
		S_SKILLTREE_INFO* pTree = m_mapSkillTreeByJobID[ iNeedJobID ];

		S_NODE_INFO* pNodeInfo = new S_NODE_INFO;
		pNodeInfo->iSkillID = iSkillID;
		pNodeInfo->iJobID = iNeedJobID;

		// If skill level data does not exist for that skill ID, log it and skip.
		if( pSkillData->vLevelDataList.empty() )
		{
			g_Log.Log( LogType::_ERROR, L"[CSkillTreeSystem::InitializeTableUsingDataManager] Not Exist SkillLevelTable Data!!    SkillID: %d \r\n", iSkillID );
			SAFE_DELETE( pNodeInfo );
			continue;
		}

		pNodeInfo->iNeedSkillPointToAcquire = pSkillData->vLevelDataList.front().nNeedSkillPoint;
		pNodeInfo->iTreeSlotIndex = pSkillTreeData->nTreeSlotIndex;
		pNodeInfo->bDefaultLocked = pSkillData->bDefaultLocked;

#if defined( PRE_ADD_ONLY_SKILLBOOK )
		pNodeInfo->bNeedSkillBook = pSkillTreeData->bNeedSkillBook;
#endif	// #if defined( PRE_ADD_ONLY_SKILLBOOK )

		// Acquisition condition info
		pNodeInfo->UnLockCondition.iUnlockSkillBookItemID = pSkillData->nUnlockSkillBookItemID;
		pNodeInfo->AcquireCondition.iNeedLevel = pSkillData->vLevelDataList.front().cLevelLimit;

		for( int i = 0; i < MAX_PARENT_SKILL_COUNT; ++i )
		{
			// Undefined value may be 0.
			int iNeedLevel = pSkillTreeData->nNeedParentSkillLevel[ i ];

			if( 0 != iNeedLevel )
				pNodeInfo->AcquireCondition.vlParentSkillNeedLevel.push_back( iNeedLevel );
		}

		// Max slot size exceeded. No choice other than terminating the program!
		_ASSERT( pNodeInfo->iTreeSlotIndex < (int)pTree->vlNodeRenderInfo.size() );

		pTree->vlNodeRenderInfo.at( pNodeInfo->iTreeSlotIndex ) = S_NODE_RENDER_INFO( iSkillID, pNodeInfo->iTreeSlotIndex, iNeedJobID, 
			&pNodeInfo->UnLockCondition, &pNodeInfo->AcquireCondition );

		pNodeInfo->pNodeRenderInfo = &(pTree->vlNodeRenderInfo[pNodeInfo->iTreeSlotIndex]);
		pTree->vlpNodeList.push_back( pNodeInfo );
		pTree->mapNodeBySkillID.insert( make_pair(iSkillID, pNodeInfo) );
		m_mapNodeListBySkillBookID.insert( make_pair(pNodeInfo->UnLockCondition.iUnlockSkillBookItemID, pNodeInfo) );
	}

	// Build parent-child hierarchy info and tree structure data.
	int iNumTreeNode = pDataManager->GetNumSkillTreeData();
	for( int iIndex = 0; iIndex < iNumTreeNode; ++iIndex )
	{
		const TSkillTreeData* pSkillTreeData = pDataManager->GetSkillTreeDataByIndex( iIndex );
		//const TSkillData* pSkillData = pDataManager->GetSkillData( pSkillTreeData->nSkillID );

		//int iJobID = pSkillData->nNeedJobID;

		S_NODE_INFO* pNodeInfo = _FindNode( pSkillTreeData->nSkillID );
		if( NULL == pNodeInfo )
		{
			OutputDebug( "[%d] Skill ID exists in Skill Tree Table but not in Skill Table\n", pSkillTreeData->nSkillID );
			_ASSERT( !"Skill ID exists in Skill Tree Table but not in Skill Table" );
		}
		else
		{
#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
			pNodeInfo->pNodeRenderInfo->vlNeedJobSP.push_back(pSkillTreeData->nNeedBasicSP);
			pNodeInfo->pNodeRenderInfo->vlNeedJobSP.push_back(pSkillTreeData->nNeedFirstSP);
			pNodeInfo->pNodeRenderInfo->vlNeedJobSP.push_back(pSkillTreeData->nNeedSecondSP);
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP
		}

		// Find parent skill and register self.
		for( int i = 0; i < MAX_PARENT_SKILL_COUNT; ++i )
		{
			int iParentSkillID = pSkillTreeData->nParentSkillID[ i ];
			if( 0 != iParentSkillID )
			{
				S_NODE_INFO* pParentNodeInfo = _FindNode( iParentSkillID );
				if( NULL == pParentNodeInfo )
				{
					OutputDebug( "[%d, set as parent of %d] Skill ID exists in Skill Tree Table but not in Skill Table\n", pSkillTreeData->nSkillID, iParentSkillID );
					_ASSERT( !"Skill ID exists in Skill Tree Table but not in Skill Table" );
				}

				if( pNodeInfo )
					pNodeInfo->vlpParentNodeInfo.push_back( pParentNodeInfo );
				if( pParentNodeInfo )
					pParentNodeInfo->vlpChildNodeInfo.push_back( pNodeInfo );
			}
		}
	}

	return bResult;
}
#endif

CDnSkillTreeSystem::S_NODE_INFO* CDnSkillTreeSystem::_FindNode( int iSkillID )
{
	S_NODE_INFO* pNodeInfo = NULL;

	vector<S_SKILLTREE_INFO*>::iterator iter = m_vlpJobSkillTreeList.begin();
	for( iter; iter != m_vlpJobSkillTreeList.end(); ++iter )
	{
		map<int, S_NODE_INFO*>& mapNodeBySkillID = (*iter)->mapNodeBySkillID;
		map<int, S_NODE_INFO*>::iterator iterNode = mapNodeBySkillID.find( iSkillID );
		if( mapNodeBySkillID.end() != iterNode )
		{
			pNodeInfo = iterNode->second;
		}
	}

	return pNodeInfo;
}

#ifdef PRE_ADD_ONLY_SKILLBOOK
bool CDnSkillTreeSystem::IsNeedSkillBook( int iSkillID, bool& bExistSkill )
{
	S_NODE_INFO* pNodeInfo = NULL;

	vector<S_SKILLTREE_INFO*>::iterator iter = m_vlpJobSkillTreeList.begin();
	for( iter; iter != m_vlpJobSkillTreeList.end(); ++iter )
	{
		map<int, S_NODE_INFO*>& mapNodeBySkillID = (*iter)->mapNodeBySkillID;
		map<int, S_NODE_INFO*>::iterator iterNode = mapNodeBySkillID.find( iSkillID );
		if( mapNodeBySkillID.end() != iterNode )
		{
			pNodeInfo = iterNode->second;
		}
	}

	if( pNodeInfo )
	{
		bExistSkill = true;
		return pNodeInfo->bNeedSkillBook;
	}

	bExistSkill = false;
	return false;
}
#endif // PRE_ADD_ONLY_SKILLBOOK

void CDnSkillTreeSystem::_MakeupParentSkillLinks( DNTableFileFormat* pSkillTreeTable, DNTableFileFormat* pSkillTable )
{
	// Link parent/child skill node info.
	int iNumTreeNode = pSkillTreeTable->GetItemCount();
	for( int iIndex = 0; iIndex < iNumTreeNode; ++iIndex )
	{
		int iTreeTableID = pSkillTreeTable->GetItemID( iIndex );

		int iSkillID = pSkillTreeTable->GetFieldFromLablePtr( iTreeTableID, "_SkillTableID" )->GetInteger();
		
		if( 0 == iSkillID )
			continue;

		int iJobID = pSkillTable->GetFieldFromLablePtr( iSkillID, "_NeedJob" )->GetInteger();

		S_NODE_INFO* pNodeInfo = _FindNode( iSkillID );
		_ASSERT( pNodeInfo && "Skill ID does not exist in Skill Tree Table" );
		if( pNodeInfo ) 
		{
			// Find parent skill and register self.
			char caLabel[ 32 ];
			for( int i = 0; i < MAX_PARENT_SKILL_COUNT; ++i )
			{
				sprintf_s( caLabel, "_ParentSkillID%d", i + 1 );
				int iParentSkillID = pSkillTreeTable->GetFieldFromLablePtr( iTreeTableID, caLabel )->GetInteger();
				if( 0 != iParentSkillID )
				{
					S_NODE_INFO* pParentNodeInfo = _FindNode( iParentSkillID );
					if (pParentNodeInfo == NULL)
						continue;

					pNodeInfo->vlpParentNodeInfo.push_back( pParentNodeInfo );
					pNodeInfo->pNodeRenderInfo->vlParentSkillJobIDs.push_back( pParentNodeInfo->iJobID );
					pNodeInfo->pNodeRenderInfo->vlParentSkillIDs.push_back( pParentNodeInfo->iSkillID );

					_ASSERT( pNodeInfo->iSkillID == pNodeInfo->pNodeRenderInfo->iSkillID && "Skill slots of the same class group may be duplicated in the Skill Table." );
					if( pNodeInfo->iSkillID != pNodeInfo->pNodeRenderInfo->iSkillID )
					{
						OutputDebug( "[SkillID: %d] Skill slot(%d) of the same class group may be duplicated in the Skill Table.\n", 
									  pNodeInfo->iSkillID, pNodeInfo->pNodeRenderInfo->iSlotIndex );
					}

					// Child slot index within the same job tree.
					if( pNodeInfo->iJobID == pParentNodeInfo->iJobID )
						pParentNodeInfo->pNodeRenderInfo->vlChildSlotIndexInJob.push_back( pNodeInfo->iTreeSlotIndex );

					pParentNodeInfo->vlpChildNodeInfo.push_back( pNodeInfo );
				}
			}

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
			int nLimitSPValue = pSkillTreeTable->GetFieldFromLablePtr( iTreeTableID, "_NeedBasicSP1" )->GetInteger();
			pNodeInfo->pNodeRenderInfo->vlNeedJobSP.push_back(nLimitSPValue);
			nLimitSPValue = pSkillTreeTable->GetFieldFromLablePtr( iTreeTableID, "_NeedFirstSP1" )->GetInteger();
			pNodeInfo->pNodeRenderInfo->vlNeedJobSP.push_back(nLimitSPValue);
			nLimitSPValue = pSkillTreeTable->GetFieldFromLablePtr( iTreeTableID, "_NeedSecondSP1" )->GetInteger();
			pNodeInfo->pNodeRenderInfo->vlNeedJobSP.push_back(nLimitSPValue);
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP
		}
	}
}


void CDnSkillTreeSystem::TryUnLockSkill( const S_TRY_UNLOCK& PresentInfo, S_OUTPUT* pResult )
{
	const S_NODE_INFO* pSkillNodeInfo = _FindNode( PresentInfo.iTryUnlockSkillID );

	// Check if a skill node matching the given skill info exists.
	if( NULL == pSkillNodeInfo )
	{
		pResult->eResult = R_ERROR;
		pResult->eErrorCode = E_CANT_FIND_SKILLNODE;
		return;
	}

	//// Does the class match? Allowed if it is a sub-class when searching job history.
	//if( pSkillNodeInfo->iJobID != PresentInfo.iJobID )
	//{
	//	pResult->eResult = R_ERROR;
	//	pResult->eErrorCode = E_MISMATCH_JOB;
	//}

	//Skill book no longer used?
#if defined(PRE_FIX_51727)
#else
	// Check if it is a valid skill book.
	if( PresentInfo.iSkillBookItemID != pSkillNodeInfo->UnLockCondition.iUnlockSkillBookItemID )
	{
		pResult->eResult = R_INVALID_SKILLBOOK_ITEMID;
		return;
	}
#endif // PRE_FIX_51727
	
#if defined(_CLIENT) && defined( PRE_ADD_ONLY_SKILLBOOK )
	if( true == pSkillNodeInfo->bNeedSkillBook )
	{
		pResult->eResult = R_ONLY_SKILL_BOOK;
		return;
	}
#endif	// #if defined( PRE_ADD_ONLY_SKILLBOOK )

	pResult->eResult = R_SUCCESS;
}


void CDnSkillTreeSystem::TryAcquireSkill( const S_TRY_ACQUIRE& TryAcquire, S_OUTPUT* pResult )
{
	// Check if skill is already learned, class matches, and required level is met.
	const S_NODE_INFO* pSkillNodeInfo = _FindNode( TryAcquire.iTryAcquireSkillID );

	// Prerequisite (parent) skill acquisition status.

	// Check if a skill node matching the given skill info exists.
	if( NULL == pSkillNodeInfo )
	{
		pResult->eResult = R_ERROR;
		pResult->eErrorCode = E_CANT_FIND_SKILLNODE; 
		return;
	}

	// If a prerequisite skill exists, whether it is acquired.
	// Function object matching SkillInfo struct.
	struct FindSkill : unary_function<const S_POSSESSED_SKILL_INFO, bool>
	{
		int m_iSkillIDToFind;
		FindSkill( int iSkillID ) : m_iSkillIDToFind( iSkillID ) {};

		bool operator () ( const S_POSSESSED_SKILL_INFO& SkillInfo )
		{
			return SkillInfo.iSkillID == m_iSkillIDToFind;
		}
	};

	//// Does the class match? Allowed if it is a sub-class when searching job history.
	//if( pSkillNodeInfo->iJobID != TryAcquire.iJobID )
	//{
	//	pResult->eResult = R_ERROR;
	//	pResult->eErrorCode = E_MISMATCH_JOB;
	//}

	// Check if required level is met... need to ask if level check is skipped on unlock.
	if( TryAcquire.iCurrentCharLevel < pSkillNodeInfo->AcquireCondition.iNeedLevel )
	{
		pResult->eResult = R_NOT_ENOUGH_CHAR_LEVEL;
		return;
	}

	// Check if owned skill points exceed required points.
	if( TryAcquire.iHasSkillPoint < pSkillNodeInfo->iNeedSkillPointToAcquire )
	{
		pResult->eResult = R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE;
		return;
	}

	// Check if parent skill requirement is met, including parent skill level.
	if( false == pSkillNodeInfo->vlpParentNodeInfo.empty() )
	{
		size_t NumParentSkill = pSkillNodeInfo->vlpParentNodeInfo.size();
		for( size_t i = 0; i < NumParentSkill; ++i )
		{
			const S_NODE_INFO* pParentNodeInfo = pSkillNodeInfo->vlpParentNodeInfo.at( i );

			vector<S_POSSESSED_SKILL_INFO>::const_iterator iter = find_if( TryAcquire.vlPossessedSkillInfo.begin(), TryAcquire.vlPossessedSkillInfo.end(), 
																		   FindSkill(pParentNodeInfo->iSkillID) );
			if( TryAcquire.vlPossessedSkillInfo.end() == iter )
			{
				pResult->eResult = R_DONT_HAVE_PARENT_SKILL;
				return;
			}

			// Parent skill must be unlocked.
			if( true == iter->bCurrentLock )
			{
				pResult->eResult = R_LOCKED_PARENTSKILL;
				return;
			}

			// Parent skill level must be raised to or above the requirement.
			int iParentSkillNeedLevel = pSkillNodeInfo->AcquireCondition.vlParentSkillNeedLevel.at( i );
			if( iter->iSkillLevel < iParentSkillNeedLevel )
			{
				pResult->eResult = R_NOT_ENOUGH_PARENT_SKILL_LEVEL;
				return;
			}
		}
	}

	pResult->eResult = R_SUCCESS;
}


int CDnSkillTreeSystem::GetNodeCount( int iJobID ) const
{
	int iResult = 0;

	map<int, S_SKILLTREE_INFO*>::const_iterator iter = m_mapSkillTreeByJobID.find( iJobID );
	if( m_mapSkillTreeByJobID.end() != iter )
		iResult = (int)iter->second->vlpNodeList.size();

	return iResult;
}

int CDnSkillTreeSystem::GetSlotCount( int iJobID ) const
{
	int iResult = 0;

	map<int, S_SKILLTREE_INFO*>::const_iterator iter = m_mapSkillTreeByJobID.find( iJobID );
	if( m_mapSkillTreeByJobID.end() != iter )
	{
		// Slot index increases from left to right, so the rightmost slot index is the largest.
		//iResult = (int)iter->second->vlNodeRenderInfo.back().iSlotIndex + 1;
		iResult = MAX_SKILL_SLOT_COUNT;
	}

	return iResult;
}

void CDnSkillTreeSystem::GetNodeRenderInfo( int iSlotIndex, int iJobID, /*OUT*/S_NODE_RENDER_INFO* pNodeRenderInfo ) const
{
	_ASSERT( pNodeRenderInfo );
	if( NULL == pNodeRenderInfo )
		return;

	map<int, S_SKILLTREE_INFO*>::const_iterator iter = m_mapSkillTreeByJobID.find( iJobID );
	if( m_mapSkillTreeByJobID.end() != iter )
	{
		const S_SKILLTREE_INFO* pSkillTree = iter->second;

		if( NULL == pSkillTree )
			return;

		_ASSERT( iSlotIndex < (int)pSkillTree->vlNodeRenderInfo.size() );
		if( iSlotIndex < (int)pSkillTree->vlNodeRenderInfo.size() )
			*pNodeRenderInfo = pSkillTree->vlNodeRenderInfo.at( iSlotIndex );
	}
}

int CDnSkillTreeSystem::FindSkillBySkillBook( int iSkillBookItemID )
{
	int iResult = 0;

	map<int, S_NODE_INFO*>::iterator iter = m_mapNodeListBySkillBookID.find( iSkillBookItemID );
	if( m_mapNodeListBySkillBookID.end() != iter )
	{
		iResult = iter->second->iSkillID;
	}

	return iResult;
}

#ifdef PRE_ADD_LEVELUP_GUIDE
void CDnSkillTreeSystem::GetLevelUpSkillInfo(std::vector<int>& newSkillIdList, int newLevel, int jobId)
{
	std::map<int, S_SKILLTREE_INFO*>::const_iterator iter = m_mapSkillTreeByJobID.find(jobId);
	if (m_mapSkillTreeByJobID.end() != iter)
	{
		const S_SKILLTREE_INFO* pInfo = (*iter).second;
		if (pInfo)
		{
			const std::vector<S_NODE_INFO*>& nodeInfoList = pInfo->vlpNodeList;
			std::vector<S_NODE_INFO*>::const_iterator nodeInfoIter = nodeInfoList.begin();
			for (; nodeInfoIter != nodeInfoList.end(); ++nodeInfoIter)
			{
				const S_NODE_INFO* pNodeInfo = (*nodeInfoIter);
				if (pNodeInfo && (pNodeInfo->AcquireCondition.iNeedLevel == newLevel))
					newSkillIdList.push_back(pNodeInfo->iSkillID);
			}
		}
	}
}

void CDnSkillTreeSystem::GetLevelUpSkillInfo(std::vector<int>& newSkillIdList, int newLevel, const std::vector<int>& jobHistoryVec)
{
	std::vector<int>::const_iterator& iter = jobHistoryVec.begin();
	for (; iter != jobHistoryVec.end(); ++iter)
	{
		const int& jobId = *iter;
		if (CommonUtil::IsValidCharacterJobId(jobId) == false)
			continue;

		GetLevelUpSkillInfo(newSkillIdList, newLevel, jobId);
	}
}
#endif

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
void CDnSkillTreeSystem::GetNeedSPValuesByJob(int nSkillID, std::vector<int>& nNeedSPValues)
{
	CDnSkillTreeSystem::S_NODE_INFO* pInfo = _FindNode(nSkillID);
	if (pInfo && pInfo->pNodeRenderInfo)
		nNeedSPValues = pInfo->pNodeRenderInfo->vlNeedJobSP;
}
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP

int CDnSkillTreeSystem::IsParentSkill( const int nParentSkillID, const int nChildSkillID )
{
	S_NODE_INFO * pData = _FindNode( nChildSkillID );

	if( NULL == pData )
		return 0;

	for( DWORD itr = 0; itr < pData->vlpParentNodeInfo.size(); ++itr )
	{
		if( nParentSkillID == pData->vlpParentNodeInfo[itr]->iSkillID )
			return pData->AcquireCondition.vlParentSkillNeedLevel[itr];
	}

	return 0;
}

void CDnSkillTreeSystem::GetParentSkillData( const int nChildSkillID, std::vector< std::pair<int, int> > & vecParentSkillData )
{
	S_NODE_INFO * pData = _FindNode( nChildSkillID );

	if( NULL == pData )
		return;

	for( DWORD itr = 0; itr < pData->vlpParentNodeInfo.size(); ++itr )
		vecParentSkillData.push_back( std::pair<int, int>( pData->vlpParentNodeInfo[itr]->iSkillID, pData->AcquireCondition.vlParentSkillNeedLevel[itr] ) );
}

#if defined( PRE_ADD_PRESET_SKILLTREE )
void CDnSkillTreeSystem::GetAdviceSkillTree( const int nAdviceSkill_Index, const std::vector<int> & vecJobList, std::vector< std::pair<int, BYTE> > & vecData )
{
	if( E_ADVICE_SKILL_LEVEL_COUNT <= nAdviceSkill_Index )
		return;

	vecData.clear();
	for( DWORD itr = 0; itr < vecJobList.size(); ++itr )
	{
		if( NULL == m_mapSkillTreeByJobID[ vecJobList[itr] ] )
			continue;

		vector<S_NODE_INFO*> & vecNodeList = m_mapSkillTreeByJobID[ vecJobList[itr] ]->vlpNodeList;
		vector<S_NODE_INFO*>::iterator Itor = vecNodeList.begin();
		for( ; Itor != vecNodeList.end(); ++Itor )
		{
			BYTE cAdviceSkillLevel = (*Itor)->vecAdviceSkillLevel[ nAdviceSkill_Index ];
			if( 0 != cAdviceSkillLevel )
				vecData.push_back( std::pair<int, BYTE>( (*Itor)->iSkillID, cAdviceSkillLevel ) );
		}
	}
}
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )