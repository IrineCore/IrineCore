#pragma once
#include "StdAfx.h"
#include "DnSkillTask.h"
#include "DnActor.h"
#include "DnSkill.h"
#include "DnInterface.h"
#include "ItemSendPacket.h"
#include "SkillSendPacket.h"
#include "DnMainDlg.h"
#include "DnStateBlow.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnPartyTask.h"
#include "DnSkillTabDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnLocalPlayerActor.h"
#include "DnSkillTreeContentDlg.h"
#include "DnTableDB.h"
#include "DnIncodeResource.h"
#include "DnItemTask.h"
#include "DnSkillResetDlg.h"
#include "DnSkillTreeDlg.h"
#include "SecondarySkill.h"
#include "DnLifeSkillFishingTask.h"
#include "DnLifeSkillCookingTask.h"
#include "ManufactureSkill.h"
#include "DnTradeTask.h"
#include "DnTradeMail.h"
#include "DnTooltipDlg.h"
#include "DnGuildWarTask.h"
#include "DnGuildWarSkillSlotDlg.h"

//#undef PRE_ADD_SKILL_LEVELUP_RESERVATION

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
#include "TotalLevelSkillSystem.h"
#endif // PRE_ADD_TOTAL_LEVEL_SKILL


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSkillTask::CDnSkillTask() : CTaskListener(true), m_iSkillResetStartJobDegree( 0 ), m_iSkillResetEndJobDegree( 0 ), m_bAddSkillCheatEnabled( false ), m_bRequestWait( false )
{

	m_iSkillTreePage = DualSkill::Type::Primary;
	m_iSkillPageCount = DualSkill::Type::None;
	m_iNeedRefreshSkillPage = DualSkill::Type::None;

	for(int i=0 ;i<DualSkill::Type::MAX; i++)
		m_iSkillPoint[i] = 0;
	m_bTempSkillAdded = false;

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	Clear_SkillLevelUp();
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
}

CDnSkillTask::~CDnSkillTask()
{

	// When changing jobs or similar, not even present in my job's skill tree below
	// Becomes a skill object and is not removed even during skill object release in the skill tree below.
	// Therefore, release ZeroLevelSkills here first. 
	// Many overlap with handles in the tree nodes below.
	// m_vlhUnlockZeroLevelSkills[m_iSkillTreePage] creates and holds skill objects based on info received from the server, and
	// If the user newly acquires it, the handle is removed from m_vlhUnlockZeroLevelSkills[m_iSkillTreePage] and that handle 
	// Structured to be added to m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].

	for(int i=0; i<DualSkill::Type::MAX ; i++)
	{
		SAFE_RELEASE_SPTRVEC( m_vlhLocalPlayerOpenedSkillList[i] );
		SAFE_RELEASE_SPTRVEC( m_vlhUnlockZeroLevelSkills[i] );
	}

	// The statement right above.. it might have been deleted from the local player open skill list, 
	// Since the slot could originally be empty, check valid before removing.
	map<int, S_SKILLTREE_INFO>::iterator iterSkillTreeMap = m_mapSkillTreeInfoByJobID.begin();
	for( iterSkillTreeMap; iterSkillTreeMap != m_mapSkillTreeInfoByJobID.end(); ++iterSkillTreeMap )
	{
		S_SKILLTREE_INFO& SkillTreeInfo = iterSkillTreeMap->second;
		for( int iSlot = 0; iSlot < MAX_SKILL_SLOT_COUNT; ++iSlot )
		{
			if( SkillTreeInfo.aSkillTreeSlotInfo[ iSlot ].hSkill )
				SAFE_RELEASE_SPTR( SkillTreeInfo.aSkillTreeSlotInfo[ iSlot ].hSkill );
		}
	}

	list<S_SKILL_LEVELUP_INFO*>::iterator iterLevelUpInfo = m_listWaitSkillLevelup.begin();
	for( iterLevelUpInfo; iterLevelUpInfo != m_listWaitSkillLevelup.end(); ++iterLevelUpInfo )
		delete (*iterLevelUpInfo);

	SAFE_RELEASE_SPTRVEC( m_vlhGuildWarSkillList );
}

bool CDnSkillTask::InitializeSkillMovieTable()
{
	m_SkillMovieDataList.clear();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSKILLVIDEO );
	if( !pSox ) return false;

	int iItemID = 0;
	for (; iItemID < pSox->GetItemCount(); ++iItemID)
	{
		int nTableID		= pSox->GetItemID(iItemID);
		int skillId			= pSox->GetFieldFromLablePtr(nTableID, "_SkillID")->GetInteger();
		char* szSkillVideo	= pSox->GetFieldFromLablePtr(nTableID, "_SkillVideoFile")->GetString();

		m_SkillMovieDataList.insert(std::make_pair(skillId, szSkillVideo));
	}

	return true;
}

const char* CDnSkillTask::GetSkillMovieName(int skillId) const
{
	std::map<int, std::string>::const_iterator iter = m_SkillMovieDataList.find(skillId);
	if (iter != m_SkillMovieDataList.end())
	{
		const std::string& str = (*iter).second;
		return str.c_str();
	}

	return NULL;
}

bool CDnSkillTask::Initialize()
{
	m_bTempSkillAdded = false;
	// Initialize skill tree system.
	m_SkillTreeSystem.InitializeTable();
	InitializeSkillMovieTable();

	for(int i=0 ;i<DualSkill::Type::MAX; i++)
		m_iSkillPoint[i] = 0;

#if defined(PRE_FIX_NEXTSKILLINFO)
	InitSkillLevelTableIDList();
#endif // PRE_FIX_NEXTSKILLINFO

	return true;
}

void CDnSkillTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	bool bProcessDispatch = false;

	switch( nMainCmd )
	{
		case SC_SKILL:	_OnRecvSkillMessage( nSubCmd, pData, nSize ); bProcessDispatch = true;	break;
		case SC_SECONDARYSKILL:	_OnRecvLifeSkillMessage( nSubCmd, pData, nSize ); bProcessDispatch = true;	break;
		case SC_PVP:	_OnRecvPVPSkillMessage( nSubCmd, pData, nSize ); bProcessDispatch = true;	break;
	}

	if( bProcessDispatch ) 
	{
		WriteLog( 1, ", Info, CDnSkillTask::OnDispatchMessage End : nMainCmd(%d), nSubCmd(%d), nSize(%d)\n", nMainCmd, nSubCmd, nSize );
	}
}

void CDnSkillTask::_OnRecvSkillMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd )
	{
		case eSkill::SC_SKILLLIST:		OnRecvSkillList((SCSkillList*)pData);		break;
		case eSkill::SC_ADDSKILL:		OnRecvSkillAdd((SCAddSkill*)pData);			break;
		case eSkill::SC_DELSKILL:		OnRecvSkillDel((SCDelSkill*)pData);			break;
		case eSkill::SC_SKILLLEVELUP:	OnRecvSkillLevelUp((SCSkillLevelUp*)pData); break;
		case eSkill::SC_OTHERPLAYERSKILLLEVELUP: OnRecvOtherPlayerSkillLevelUp((SCOtherPlayerSkillLevelUp*)pData); break;
		case eSkill::SC_PUSHSKILLPOINT: OnRecvPushSkillPoint( (SCPushSkillPoint*)pData ); break;
		case eSkill::SC_UNLOCKSKILL:	OnRecvUnlockSkill( (SCUnlockSkill*)pData ); break;
		case eSkill::SC_ACQUIRESKILL:	OnRecvAcquireSkill( (SCAcquireSkill*)pData ); break;
		case eSkill::SC_SKILL_RESET:	OnRecvSkillReset( (SCSkillReset*)pData ); break;
		case eSkill::SC_CANUSE_SKILLRESET_CASH_ITEM:	OnRecvCanUseSkillResetCashItem( (SCCanUseSkillResetCashItem*)pData ); break;
		case eSkill::SC_USE_SKILLRESET_CASHITEM_RES: OnRecvUseSkillResetCashItemRes( (SCUseSkillResetCashItemRes*)pData ); break;
		case eSkill::SC_CHANGE_SKILLPAGE_RES: OnRecvChangeSkillPageRes( (SCChangeSkillPage*)pData); break;
		case eSkill::SC_SKILLPAGE_COUNT: OnRecvSkillPageCount( (SCSkillPageCount*)pData ); break;
#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
		case eSkill::SC_RESERVATION_SKILL_LIST_ACK: OnRecvReservationSkillList( (SCReservationSkillListAck *)pData ); break;
#endif	// #if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
#ifdef PRE_ADD_PRESET_SKILLTREE
		case eSkill::SC_LIST_SKILLSET: OnRecvSkillSetList((SCSKillSetList*)pData); break;
		case eSkill::SC_SAVE_SKILLSET: OnRecvSkillSetSaveResult((SCSKillSaveResult*)pData); break;
		case eSkill::SC_DELETE_SKILLSET: OnRecvSkillSetDeleteResult((SCSKillDeleteResult*)pData); break;
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE
	}
}

void CDnSkillTask::_OnRecvLifeSkillMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd )
	{
		case eSecondarySkill::SC_CREATE:			OnRecvLifeSkillAdd( (SecondarySkill::SCCreate*)pData );				break;
		case eSecondarySkill::SC_DELETE:			OnRecvLifeSkillDelete( (SecondarySkill::SCDelete*)pData );			break;
		case eSecondarySkill::SC_UPDATE_EXP:		OnRecvLifeSkillUpdateExp( (SecondarySkill::SCUpdateExp*)pData );	break;
		case eSecondarySkill::SC_LIST:				OnRecvLifeSkillList( (SecondarySkill::SCList*)pData );				break;
	}
}

void CDnSkillTask::OnRecvSkillList( SCSkillList *pPacket )
{
	// Do not repopulate if the skill list arrives again.
	// Equipment item skills are pre-populated upon receiving the equipment list, so they are not included in the skill list sent by the server.
	bool bAllowFillSkillList = true;

	if( pPacket->cSkillPage >= DualSkill::Type::MAX || pPacket->cSkillPage < DualSkill::Type::Primary )
		return;

	if( m_vlhLocalPlayerOpenedSkillList[pPacket->cSkillPage].empty() )
	{
		int iNumSkill = (int)m_vlhLocalPlayerOpenedSkillList[pPacket->cSkillPage].size();
		for( int iSkill = 0; iSkill < iNumSkill; ++iSkill )
		{
			DnSkillHandle hSkill = m_vlhLocalPlayerOpenedSkillList[pPacket->cSkillPage].at( iSkill );
			if( false == hSkill->IsEquipItemSkill() )
				bAllowFillSkillList = false;
		}
	}

	if( bAllowFillSkillList )
	{
		DnSkillHandle hSkill;
		for( int i=0; i<pPacket->cSkillCount; i++ ) 
		{
			// TODO: Also check from server whether it's a Lock/Unlock skill..
			// Skills that are only unlocked are not added to the open skill list and are stored separately. Data sent from the server has level 0.
			
			int nSkillID = pPacket->Skill[i].nSkillID;
			int nSkillLevel = pPacket->Skill[i].cSkillLevel;
			int nLevelUpValue = 0;
			if( CDnActor::s_hLocalActor )
				nLevelUpValue = CDnActor::s_hLocalActor->GetSkillLevelUpValue(nSkillID);

			// #41639 If nSkillLevel is 0, the skill is reset / unlearned.
			if( nSkillLevel < 1 )
				nLevelUpValue = 0;

			nSkillLevel += nLevelUpValue;

			hSkill = CDnSkill::CreateSkill( CDnActor::Identity(), nSkillID, nSkillLevel );

			if (!hSkill && nLevelUpValue != 0)
			{
				OutputDebug("Failed to create skill (%d) level (%d).. Check skill level table!!!!!!\n", nSkillID, nSkillLevel);
				_ASSERT( hSkill && "Skill level-up attempt failed.. Check level table" );

				//Revert level value back down to original
				nSkillLevel -= nLevelUpValue;
				hSkill = CDnSkill::CreateSkill(CDnActor::Identity(), nSkillID, nSkillLevel);

				//Reset level-up value
				nLevelUpValue = 0;
			}

			if ( hSkill )
			{
				// Whether acquired or not acquired
				if( hSkill->IsAcquired() )
				{
					hSkill->SetElapsedDelayTime( pPacket->Skill[i].nCoolTime / 1000.f );
					m_vlhLocalPlayerOpenedSkillList[pPacket->cSkillPage].push_back( hSkill );
				}
				else
					m_vlhUnlockZeroLevelSkills[pPacket->cSkillPage].push_back( hSkill );

				if( hSkill->IsAcquired() )
					hSkill->SetLevelUpValue(nLevelUpValue);
			}
		}

		// Load all skills contained in the skill tree.
		// Set job ID to 1 for now.. check as Warrior.
		if( CDnActor::s_hLocalActor )
		{
			// TODO: Later loop over job history.
			InitializeSkillTree();
		}

		InitializeLocalPlayerSkills();
		GetInterface().RefreshSkillDialog();

		DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
		if( !hLocalActor ) return;
		CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor *)hLocalActor.GetPointer();

		for( int itr = GLYPH_SKILL1; itr <= GLYPH_SKILL4; ++itr )
		{
			CDnItem *pItem = CDnItemTask::GetInstance().GetGlyphItem(itr);
			if( !pItem ) continue;
			if( dynamic_cast<CDnGlyph*>(pItem) && pItem->GetItemType() == ITEMTYPE_GLYPH ) {
				DnGlyphHandle hGlyph = ((CDnGlyph*)pItem)->GetMySmartPtr();

				if( pPacket->cSkillPage > DualSkill::Type::Primary )
					pActor->ApplyGlyphToExpendedSkillPage( hGlyph , pPacket->cSkillPage );
				else
					pActor->AttachGlyph( hGlyph, (CDnGlyph::GlyphSlotEnum)itr );
			}
		}
	}
}

void CDnSkillTask::AddJobSkillTree( int iJobID )
{
	int iNumSkillTreeSlot = m_SkillTreeSystem.GetSlotCount( iJobID );
	_ASSERT( iNumSkillTreeSlot <= MAX_SKILL_SLOT_COUNT );
	for( int iSlot = 0; iSlot < iNumSkillTreeSlot; ++iSlot )
	{
		CDnSkillTreeSystem::S_NODE_RENDER_INFO NodeRenderInfo;
		m_SkillTreeSystem.GetNodeRenderInfo( iSlot, iJobID, &NodeRenderInfo );

		m_mapSkillTreeInfoByJobID[ iJobID ].aSkillTreeSlotInfo[ iSlot ].NodeRenderInfo = NodeRenderInfo;
		S_SKILLTREE_SLOT_INFO& TreeSlotInfo = m_mapSkillTreeInfoByJobID[ iJobID ].aSkillTreeSlotInfo[ iSlot ];

		if( 0 != NodeRenderInfo.iSkillID )
		{
			// If it is an owned skill, set it to that skill object.
			DnSkillHandle hOpenedSkill = FindSkill( NodeRenderInfo.iSkillID );
			if( hOpenedSkill )
			{
				TreeSlotInfo.hSkill = hOpenedSkill;
			}
			else
			{
				// Set unacquired skills that are only unlocked.
				DnSkillHandle hZeroLevelSkill = FindZeroLevelSkill( NodeRenderInfo.iSkillID );
				if( hZeroLevelSkill )
				{
					TreeSlotInfo.hSkill = hZeroLevelSkill;
				}
				else
				{
					// Create at level 1 in locked state. Later must unlock with skill book and invest SP to level up to count as acquired.
					// If a skill already exists in the skill slot, delete it and create a new one. 
					// Created here as a temporary object only for showing tooltips; not used because the user has never acquired this skill.
					// Finally, when the CDnSkillTask object is destroyed, all valid skill handles in the array are reset.
					if( TreeSlotInfo.hSkill )
						SAFE_RELEASE_SPTR( TreeSlotInfo.hSkill );

					TreeSlotInfo.hSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, NodeRenderInfo.iSkillID, 1 );
					TreeSlotInfo.hSkill->Lock();
				}
			}

			TreeSlotInfo.hSkill->SetNodeRenderInfo( TreeSlotInfo.NodeRenderInfo );

			if( CDnActor::s_hLocalActor )
			{
				TreeSlotInfo.hSkill->SetHasActor( CDnActor::s_hLocalActor );
				TreeSlotInfo.hSkill->RefreshDecreaseMP();		// Handled inside SetHasActor.
			}
		}
	}
}

void CDnSkillTask::RemoveJobSkillTree( int iJobID )
{
	map<int, S_SKILLTREE_INFO>::iterator iter = m_mapSkillTreeInfoByJobID.find( iJobID );
	if( m_mapSkillTreeInfoByJobID.end() != iter )
	{
		// Skills never acquired were created and set by AddJobSkillTree(), so delete them here.
		// Since all other skill objects are managed elsewhere, just destroy the directly created skill objects and remove the tree struct itself from the map.
		S_SKILLTREE_INFO& SkillTreeInfo = iter->second;
		for( int i = 0; i < MAX_SKILL_SLOT_COUNT; ++i )
		{
			S_SKILLTREE_SLOT_INFO& SlotInfo = SkillTreeInfo.aSkillTreeSlotInfo[ i ];
			if( SlotInfo.hSkill && SlotInfo.hSkill->IsLocked() )
			{
				SAFE_RELEASE_SPTR( SlotInfo.hSkill );
			}
		}

		m_mapSkillTreeInfoByJobID.erase( iter );
	}
}

void CDnSkillTask::InitializeSkillTree()
{
	// Loop through each job history entry.
	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

	vector<int> vlJobHistory;
	pPlayerActor->GetJobHistory( vlJobHistory );

	for( int i = 0; i < (int)vlJobHistory.size(); ++i )
	{
		int iJobID = vlJobHistory.at( i );

		AddJobSkillTree(iJobID);
	}

	pPlayerActor->InitializedSkillTree();
}

void CDnSkillTask::RefreshSkillTree()
{
	m_mapSkillTreeInfoByJobID.clear();
	InitializeSkillTree();
}

void CDnSkillTask::GetSkillTreeSlotInfo( int iJobID, int iSkillSlotIndex, S_SKILLTREE_SLOT_INFO* pSkillSlotInfo )
{
	// TODO: Handle jobid later when job advancement is added.
	_ASSERT( pSkillSlotInfo );
	_ASSERT( iSkillSlotIndex < MAX_SKILL_SLOT_COUNT );
	if( pSkillSlotInfo && iSkillSlotIndex < MAX_SKILL_SLOT_COUNT )
		*pSkillSlotInfo = m_mapSkillTreeInfoByJobID[ iJobID ].aSkillTreeSlotInfo[ iSkillSlotIndex ];
}

// Now this function is only called when a packet arrives from using the addskill cheat.
void CDnSkillTask::OnRecvSkillAdd( SCAddSkill *pPacket )
{
	// Note : Check return value
	//		pPacket->cRet
	//
	if (pPacket->nRet == ERROR_NONE)
	{
		// Find the skill and change its state to unlocked.
		DnSkillHandle hCheatAcquiredSkill = FindLockedSkill( pPacket->nSkillID );
		if( !hCheatAcquiredSkill )
			return;

		if( hCheatAcquiredSkill )
		{
			SCUnlockSkill UnlockSkill;
			UnlockSkill.nSkillID = pPacket->nSkillID;
			UnlockSkill.nRetCode = ERROR_NONE;
			this->OnRecvUnlockSkill( &UnlockSkill );
		}

		// Skill acquired! Add it to the owned skill list.
		vector<DnSkillHandle>::iterator iter = find( m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].begin(), m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].end(), hCheatAcquiredSkill );
		_ASSERT( m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].end() != iter );
		if( m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].end() != iter )
			m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].erase( iter );

		m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].push_back( hCheatAcquiredSkill );

		
		hCheatAcquiredSkill->SetHasActor( CDnActor::s_hLocalActor );
		hCheatAcquiredSkill->Acquire();

		CDnActor::s_hLocalActor->AddSkill( hCheatAcquiredSkill );

		GetInterface().RefreshSkillDialog();
		m_bAddSkillCheatEnabled = true;

		if( hCheatAcquiredSkill->GetLevel() != 1 )
			return;

		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSKILL );
		if( !pSox ) return;

		for( int itr = GLYPH_SKILL1; itr <= GLYPH_SKILL4; ++itr )
		{
			CDnItem *pItem = CDnItemTask::GetInstance().GetGlyphItem(itr);
			if( !pItem ) continue;
			if( dynamic_cast<CDnGlyph*>(pItem) && pItem->GetItemType() == ITEMTYPE_GLYPH ) {
				DnGlyphHandle hGlyph = ((CDnGlyph*)pItem)->GetMySmartPtr();

				int nSkillID = pSox->GetFieldFromLablePtr( hGlyph->GetClassID(), "_SkillID" )->GetInteger();

				if( hCheatAcquiredSkill->GetClassID() == nSkillID )
					hCheatAcquiredSkill->AddGlyphStateEffect( hGlyph->GetClassID() );
			}
		}
	}
	else
		GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnSkillTask::OnRecvSkillDel( SCDelSkill *pPacket )
{
	// Note : Check return value
	//		pPacket->cRet
	//
	if (pPacket->nRet == ERROR_NONE)
		DelSkill( pPacket->nSkillID );
	else
		GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnSkillTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_listWaitSkillLevelup.empty() )
	{
		list<S_SKILL_LEVELUP_INFO*>::iterator iter = m_listWaitSkillLevelup.begin();
		for( iter; iter != m_listWaitSkillLevelup.end(); )
		{
			S_SKILL_LEVELUP_INFO* pInfo = *iter;
			if( pInfo->hActor )
			{
				if( pInfo->hActor->GetProcessSkill() != pInfo->hSkill )
				{
					OnRecvSkillLevelUp( &(pInfo->Packet) );
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
	if( GetGuildWarTask().GetGuildWarState() == PvPCommon::OccupationSystemState::Play )
	{
		vector<DnSkillHandle>::iterator iter = m_vlhGuildWarSkillList.begin();

		for( ; iter != m_vlhGuildWarSkillList.end(); iter++ )
		{
			DnSkillHandle hSkill = (*iter);
			if( hSkill && hSkill->GetActor() == CDnActor::s_hLocalActor )
				hSkill->Process( LocalTime, fDelta );
		}
	}
}

void CDnSkillTask::OnRecvSkillLevelUp( SCSkillLevelUp *pPacket )
{
	_ASSERT( pPacket );

	if( CDnActor::s_hLocalActor == NULL )
		return;

	m_bRequestWait = false;
	DnSkillHandle hSkill = CDnActor::s_hLocalActor->FindSkill( pPacket->nSkillID );

	// During gameplay it doesn't matter because the game server sends the packet after the skill ends,
	// Handled by this routine when leveling up in village while using a skill with intermediate action like Whirlwind.
	// Level-up occurs immediately on village server, and client only shows the effect later.
	// When leaving the game, other users' skill level-ups start from when the game server first received that user's packet 
	// Queued and fired later, so it is handled automatically.
	if( CDnActor::s_hLocalActor->GetProcessSkill() == hSkill )
	{
		S_SKILL_LEVELUP_INFO* pSkillLevelupInfo = new S_SKILL_LEVELUP_INFO;
		pSkillLevelupInfo->hActor = CDnActor::s_hLocalActor;
		pSkillLevelupInfo->hSkill = hSkill;
		pSkillLevelupInfo->Packet = *pPacket;

		m_listWaitSkillLevelup.push_back( pSkillLevelupInfo );
		return;
	}

	//CDnSkillTabDlg *pSkillDlg = (CDnSkillTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG );
	//CDnSkillTreeContentDlg* pSkillTreeContentDlg = (CDnSkillTreeContentDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_TREE_CONTENT_DIALOG );

	if (pPacket->nRetCode == ERROR_NONE)
	{
		LevelUpSkill( pPacket->nSkillID, pPacket->cLevel );

		/*
		if( pSkillTreeContentDlg )
			pSkillTreeContentDlg->OnSkillLevelUp( true );
		*/
	}
	else
	{
		//GetInterface().ServerMessageBox(pPacket->nRetCode);
		// Note: Currently configured to increase skill level by 1 at a time.

		// Modified to calculate skill level-up points only when skill level-up succeeds on server. (#22216)
		//if( hSkill )
		//	hSkill->ResetLevelUpUsePoint( 1 );

		/*
		if( pSkillTreeContentDlg )
			pSkillTreeContentDlg->OnSkillLevelUp( false );
		*/
	}
}

int CDnSkillTask::QueryPVEOrPVP( void )
{
	int iSkillLevelDataType = CDnSkill::PVE;
	CDnGameTask * pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	bool bIsPVPGame = ((pGameTask) && (pGameTask->GetGameTaskType() == GameTaskType::PvP));
	if( bIsPVPGame )
	{
		iSkillLevelDataType = CDnSkill::PVP;
	}

	return iSkillLevelDataType;
}

void CDnSkillTask::InitializeLocalPlayerSkills()
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor ) return;

	// Default skill level data is always PVE 
	int iSkillLevelDataType = QueryPVEOrPVP();

	DnSkillHandle hSkill;

	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) {
		hSkill = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i];
		// Since some info is updated inside SetHasActor, be sure to update the skill level data type beforehand.
		hSkill->SelectLevelDataType( iSkillLevelDataType , true );
		hSkill->SetHasActor( hLocalActor );

		if( hSkill->IsToggleOn() )
			hSkill->EnableToggle( false );

		if( hSkill->IsAuraOn() )
			hSkill->EnableAura( false );

		if( hLocalActor->CanAddSkill( hSkill->GetClassID() ) == false )
			continue;
		hLocalActor->AddSkill( hSkill );

#if defined(PRE_FIX_CLIENT_MEMOPTIMIZE)
		if( hSkill->IsUseInVillage() ) {
			const set<string> &setActionList = hSkill->GetUseActionSet();
			for( set<string>::const_iterator iter = setActionList.begin(); iter != setActionList.end(); iter++ ) {
				hLocalActor->RefreshUsingElement( (*iter).c_str() );
			}
		}
#endif
	}

	// Iterate through all and update related info if there is an enhanced passive skill.
	InitializeEnchantPassiveSkills();

#if defined(PRE_FIX_51999)
	bool bAddTempSkill = GetSkillTask().IsTempSkillAdded();
#endif // PRE_FIX_51999

	// When moving from game server to game server, the server sends all packets, but when returning to village 
	// Since the connection to the game server is closed, reset any temporarily created skills directly.
	// On entering village, character info is received anew, so temporary class assignment is reset.
	// Just remove the temporary skill here.
	GetSkillTask().RemoveAllTempSkill();

#if defined(PRE_FIX_51999)
	//When moving straight to town after completing class change quest without normal processing
	//Cases occur where SkillTree info is abnormal.
	//If tempSkill is added, refresh skillTree.
	if (bAddTempSkill == true)
		GetSkillTask().InitializeSkillTree();
#endif // PRE_FIX_51999

	// Need to loop once to apply passive buff skills.
	hLocalActor->GetStateBlow()->Process( 0, 0.0f, true );

	if( m_iNeedRefreshSkillPage != DualSkill::Type::None )
	{
		ApplySkillPage(m_iNeedRefreshSkillPage);
		m_iNeedRefreshSkillPage = DualSkill::Type::None;
		// Need to loop once to apply passive buff skills.
		hLocalActor->GetStateBlow()->Process( 0, 0.0f, true );
	}

	hLocalActor->SetHP( hLocalActor->GetMaxHP() );
	hLocalActor->SetSP( hLocalActor->GetMaxSP() );
}

void CDnSkillTask::AddTempSkill( int iSkillID )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor ) return;

	// If the skill being temporarily created is an enhanced passive skill 
	// Check if the base skill is owned; if not, also add the base skill as a temporary skill.
	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	int iNeedBaseSkillID = 0;
	if( pSkillTable->IsExistItem( iSkillID ) )
		iNeedBaseSkillID = pSkillTable->GetFieldFromLablePtr( iSkillID, "_BaseSkillID" )->GetInteger();

	DNVector(int) vlSkillsToAdd;
	if( 0 < iNeedBaseSkillID )
	{
		// If the target skill to enhance is not owned, add it to the vector to create it as well.
		if( false == hLocalActor->IsExistSkill( iNeedBaseSkillID ) )
			vlSkillsToAdd.push_back( iNeedBaseSkillID );
	}
	vlSkillsToAdd.push_back( iSkillID );

	for( int i = 0; i < (int)vlSkillsToAdd.size(); ++i )
	{
		int iSkillIDToAdd = vlSkillsToAdd.at( i );

		for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ )
		{
			if( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->GetClassID() == iSkillIDToAdd ) 
				return;
		}

		DnSkillHandle hSkill = CDnSkill::CreateSkill( hLocalActor, iSkillIDToAdd, 1 );
		if( !hSkill )
			return;
		m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].push_back( hSkill );
		// Default skill level data is always PVE 
		CDnGameTask * pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
		bool bIsPVPGame = ((pGameTask) && (pGameTask->GetGameTaskType() == GameTaskType::PvP));
		int iSkillLevelDataType = CDnSkill::PVE;
		if( bIsPVPGame )
		{
			iSkillLevelDataType = CDnSkill::PVP;
		}
		hSkill->SelectLevelDataType( iSkillLevelDataType );

		hLocalActor->AddSkill( hSkill );

		hSkill->AsTempSkill();

		// If the skill is currently unusable because it is a 2nd job advancement skill,
		// Update object values to match the current character level and job.
		int iLevel = hLocalActor->GetLevel();
		if( iLevel < hSkill->GetLevelLimit() )
			hSkill->SetLevelLimit( iLevel );

		CDnLocalPlayerActor* pLocalPlayer = static_cast<CDnLocalPlayerActor*>(hLocalActor.GetPointer());
		if( false == pLocalPlayer->IsPassJob( hSkill->GetNeedJobClassID() ) )
			hSkill->SetNeedJobClassID( pLocalPlayer->GetJobClassID() );
	}

	m_bTempSkillAdded = true;
}

void CDnSkillTask::RemoveTempSkill( int iSkillID )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor ) return;

	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
	{
		if( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->GetClassID() == iSkillID ) 
		{
			_ASSERT( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i] && m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->IsTempSkill() );
			hLocalActor->RemoveSkill( iSkillID );
			SAFE_RELEASE_SPTR( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i] );
			m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].erase( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].begin() + i );
			break;
		}
	}
}

void CDnSkillTask::RemoveAllTempSkill( void )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor ) return;

	vector<DnSkillHandle>::iterator iter = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].begin();
	for( iter; iter != m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].end(); )
	{
		DnSkillHandle hSkill = *iter;
		if( hSkill->IsTempSkill() )
		{
			hLocalActor->RemoveSkill( hSkill->GetClassID() );
			SAFE_RELEASE_SPTR( hSkill );
			iter = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].erase( iter );
		}
		else
			++iter;
	}

	m_bTempSkillAdded = false;
}

void CDnSkillTask::InitializeEnchantPassiveSkills( void )
{
	// Find skills of the EnchantPassive type and apply their values to the base skill.
	for( DWORD i = 0; i < m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
	{
		if( CDnSkill::EnchantPassive == m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][ i ]->GetSkillType() &&
			0 < m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][ i ]->GetBaseSkillID() )
		{
			DnSkillHandle hEnchantPassiveSkill = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][ i ];
			int iBaseSkillID = hEnchantPassiveSkill->GetBaseSkillID();
			DnSkillHandle hBaseSkill = FindSkill( iBaseSkillID );

			// Exception handling [2011/03/17 semozz]
			if (hBaseSkill)
				hBaseSkill->ApplyEnchantSkill( hEnchantPassiveSkill );
		}
	}
}

// When equipping a Crest skill, etc., the skill is added here.

void CDnSkillTask::AddSkill( int nSkillID, int iLevel /*= 1*/ , int nSkillPage /* = DualSkill::Type::None */ )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor ) return;

	if( nSkillPage == DualSkill::Type::None )
		nSkillPage = m_iSkillTreePage;

	if(nSkillPage < DualSkill::Type::Primary || nSkillPage >= DualSkill::Type::MAX )
		return;

	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[nSkillPage].size(); i++ )
	{
		if( m_vlhLocalPlayerOpenedSkillList[nSkillPage][i]->GetClassID() == nSkillID ) 
			return;
	}

	//Apply skill level-up value.
	int nLevelUpValue = hLocalActor->GetSkillLevelUpValue(nSkillID);
	iLevel += nLevelUpValue;

	DnSkillHandle hSkill = CDnSkill::CreateSkill( hLocalActor, nSkillID, iLevel );

	if (!hSkill && nLevelUpValue != 0)
	{
		OutputDebug("Failed to create skill (%d) level (%d).. Check skill level table!!!!!!\n", nSkillID, iLevel);
		_ASSERT( hSkill && "Skill level-up attempt failed.. Check level table" );

		//Revert level value back down to original
		iLevel -= nLevelUpValue;
		hSkill = CDnSkill::CreateSkill(hLocalActor, nSkillID, iLevel);

		//Reset level-up value
		nLevelUpValue = 0;
	}

	if( !hSkill )
		return;

	hSkill->SetLevelUpValue(nLevelUpValue);

	m_vlhLocalPlayerOpenedSkillList[nSkillPage].push_back( hSkill );

	// Default skill level data is always PVE 
	CDnGameTask * pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	bool bIsPVPGame = ((pGameTask) && (pGameTask->GetGameTaskType() == GameTaskType::PvP));
	int iSkillLevelDataType = CDnSkill::PVE;
	if( bIsPVPGame )
	{
		iSkillLevelDataType = CDnSkill::PVP;
	}
	hSkill->SelectLevelDataType( iSkillLevelDataType );

	hLocalActor->AddSkill( hSkill );

	GetInterface().RefreshSkillDialog();
}

void CDnSkillTask::DelSkill( int nSkillID , int nSkillPage /* = DualSkill::Type::None */ )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor ) return;

	
	if( nSkillPage == DualSkill::Type::None )
		nSkillPage = m_iSkillTreePage;

	if(nSkillPage < DualSkill::Type::Primary || nSkillPage >= DualSkill::Type::MAX )
		return;


	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[nSkillPage].size(); i++ ) 
	{
		if( m_vlhLocalPlayerOpenedSkillList[nSkillPage][i]->GetClassID() == nSkillID ) 
		{
			hLocalActor->RemoveSkill( nSkillID );
			SAFE_RELEASE_SPTR( m_vlhLocalPlayerOpenedSkillList[nSkillPage][i] );
			m_vlhLocalPlayerOpenedSkillList[nSkillPage].erase( m_vlhLocalPlayerOpenedSkillList[nSkillPage].begin() + i );
			break;
		}
	}
	GetInterface().RefreshSkillDialog();
}

void CDnSkillTask::ErrorMessage_ReservationSkill( const int nError )
{
	switch( nError )
	{
		// Related to unlock failure. ////////////////////////////////////////////////////////////////
	case ERROR_SKILL_UNLOCK_FAIL:
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 757) );
		GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 757) );
		break;

	case ERROR_SKILL_UNLOCK_ALREADY_OPEN:
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 722) );
		GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 722) );
		break;

		// When unlocking a skill with gold.
	case ERROR_SKILL_UNLOCK_NOT_ENOUGH_MONEY:
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1744) );
		GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1744) );
		break;

		// Related to acquisition failure. ////////////////////////////////////////////////////////////////
		// Character level requirement not met.
	case ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_CHAR_LEVEL:
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1145) );
		GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1145) );
		break;

		// No prerequisite (parent) skill.
	case ERROR_SKILL_ACQUIRE_FAIL_DONT_HAVE_PARENT_SKILL:
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1817) );
		GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1817) );
		break;

		// Parent skill level requirement not met.
	case ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_PARENT_SKILL_LEVEL:
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1817) );
		GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1817) );
		break;

		// Cannot acquire skill due to insufficient skill points.
	case ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_SKILLPOINT:
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1147) );
		GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1147) );
		break;

		// Cannot learn because another ultimate skill is learned. This response only comes when exclusive id matches in the table.
	case ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE:
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1153) );
		GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1153) );
		break;
	}
}

void CDnSkillTask::Unlock_ReservationSkill( const int nSkillID )
{
	DnSkillHandle hSkillToUnlock = FindLockedSkill( nSkillID );

	if( !hSkillToUnlock )
		return;

	hSkillToUnlock->UnLock();

	// Add to unlocked skill list.
	m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].push_back( hSkillToUnlock );
}

void CDnSkillTask::Acquire_ReservationSkill( const int nSkillID, const bool bRefreshDialog )
{
	// Skill acquired! Add it to the owned skill list.
	DnSkillHandle hAcquiredSkill = FindZeroLevelSkill( nSkillID );
	if( !hAcquiredSkill )
		return;

	hAcquiredSkill->SetHasActor( CDnActor::s_hLocalActor );
	hAcquiredSkill->Acquire();

	vector<DnSkillHandle>::iterator iter = find( m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].begin(), m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].end(), hAcquiredSkill );
	_ASSERT( m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].end() != iter );
	if( m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].end() != iter )
	{
		m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].erase( iter );

		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		pPlayerActor->AddSkill( hAcquiredSkill );
		hAcquiredSkill->SelectLevelDataType( QueryPVEOrPVP() );
		m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].push_back( hAcquiredSkill );
		m_iSkillPoint[m_iSkillTreePage] -= hAcquiredSkill->GetNeedAcquireSkillPoint();

		if( true == bRefreshDialog )
		{
			GetInterface().RefreshSkillDialog();
			GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 723), MB_OK );
			GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 723) );
		}

		// To attach a 'new' mark when drawing the skill tree UI and quickslot buttons.
		hAcquiredSkill->SetNewAcquire();

		// Register skill shortcut
		if( hAcquiredSkill->GetSkillType() == CDnSkill::Active )
		{
			CDnMainDlg *pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
			int nEmptySlotIndex = pMainDlg->GetEmptyQuickSlotIndex();
			if( nEmptySlotIndex != -1 )
			{
				pMainDlg->SetQuickSlot( nEmptySlotIndex, hAcquiredSkill.GetPointer() );
			}

			// Apply cooldown when learning a skill in PvP Colosseum
			// #27498 [2011/01/17 semozz]
			CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
			if(	pTask && pTask->GetGameTaskType() == GameTaskType::PvP ) 
			{
				hAcquiredSkill->OnBeginCoolTime();
			}
		}

		// Show skill unlock effect.
		DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
		if( hHandle ) {
			hHandle->SetPosition( *(CDnActor::s_hLocalActor->GetPosition()) );
			hHandle->SetActionQueue( "BuySkill" );
			EtcObjectSignalStruct *pResult = CDnActor::s_hLocalActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );

			pResult->vOffset = EtVector3( 0.f, 0.f, 0.f );
			pResult->vRotate = EtVector3( 0, 0, 0);
			pResult->bLinkObject = true;
			pResult->bDefendenceParent = true;
		}

		if( hAcquiredSkill->GetLevel() != 1 )
			return;

		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSKILL );
		if( !pSox ) return;

		for( int itr = GLYPH_SKILL1; itr <= GLYPH_SKILL4; ++itr )
		{
			CDnItem *pItem = CDnItemTask::GetInstance().GetGlyphItem(itr);
			if( !pItem ) continue;
			if( dynamic_cast<CDnGlyph*>(pItem) && pItem->GetItemType() == ITEMTYPE_GLYPH ) {
				DnGlyphHandle hGlyph = ((CDnGlyph*)pItem)->GetMySmartPtr();

				int nSkillID = pSox->GetFieldFromLablePtr( hGlyph->GetClassID(), "_SkillID" )->GetInteger();

				if( hAcquiredSkill->GetClassID() == nSkillID )
					hAcquiredSkill->AddGlyphStateEffect( hGlyph->GetClassID() );
			}
		}


		//If a newly acquired skill has level-up info configured, level it up.
		if (pPlayerActor)
		{
			int nLevelUpValue = pPlayerActor->GetSkillLevelUpValue(hAcquiredSkill->GetClassID());
			int nCurLevel = hAcquiredSkill->GetLevel();
			int nSkillID = hAcquiredSkill->GetClassID();

			if (nLevelUpValue != 0)
			{
				ChangeSkillLevelUp(nSkillID, nCurLevel , true );
			}
		}
	}
}

void CDnSkillTask::LevelUpSkill( int nSkillID, int nLevel, const bool bRefreshDialog )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor ) return;

	DnSkillHandle hSkillToLevelUp;
	int iSkillListIndex = -1;

	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
	{
		if( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->GetClassID() == nSkillID ) 
		{
			hSkillToLevelUp = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][ i ];
			iSkillListIndex = i;
			break;
		}
	}

	bool bValidIndex = (-1 != iSkillListIndex);
	_ASSERT( bValidIndex );
	if( bValidIndex )
	{
		int iNowSkillLevel = 0;
		float fElapsedDelay = 0.f;

		hSkillToLevelUp->CalcLevelUpUsePoint( 1 );
		m_iSkillPoint[m_iSkillTreePage] -= hSkillToLevelUp->GetUsePoint();
		fElapsedDelay = hSkillToLevelUp->GetElapsedDelayTime();
		hSkillToLevelUp->SetUsePoint( 0 );

		int nLevelUp = hLocalActor->GetSkillLevelUpValue(nSkillID);
		int nCurSkillLevel = hSkillToLevelUp->GetLevel();
		nLevel += nLevelUp;

		DnSkillHandle hLevelUppedSkill = CDnSkill::CreateSkill( hLocalActor, nSkillID, nLevel );

		//If level-up skill creation failed, cancel level-up...
		if (!hLevelUppedSkill && nLevelUp > 0)
		{
			OutputDebug("Failed to create skill (%d) level (%d).. Check skill level table!!!!!!\n", nSkillID, nLevel);
			_ASSERT( hLevelUppedSkill && "Skill level-up attempt failed.. Check level table" );

			//Revert level value back down to original
			nLevel -= nLevelUp;
			hLevelUppedSkill = CDnSkill::CreateSkill(hLocalActor, nSkillID, nLevel);

			//Reset level-up value
			nLevelUp = 0;
		}

		if (hLevelUppedSkill)
			hLevelUppedSkill->SetLevelUpValue(nLevelUp);

		if( true == hSkillToLevelUp->IsNewAcquire() )
			hLevelUppedSkill->SetNewAcquire();

		for( int i = 0; i < MAX_SKILL_SLOT_COUNT; ++i )
		{
			if( m_mapSkillTreeInfoByJobID[ hLevelUppedSkill->GetNeedJobClassID() ].aSkillTreeSlotInfo[ i ].hSkill == m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].at(iSkillListIndex) )
			{
				m_mapSkillTreeInfoByJobID[ hLevelUppedSkill->GetNeedJobClassID() ].aSkillTreeSlotInfo[ i ].hSkill = hLevelUppedSkill;
				break;
			}
		}
		
		m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].at( iSkillListIndex ) = hLevelUppedSkill;
		hLevelUppedSkill->SetElapsedDelayTime( fElapsedDelay );
		hLevelUppedSkill->SelectLevelDataType( hSkillToLevelUp->GetSelectedLevelDataType() );
		bool bResult = hLocalActor->ReplacementSkill( hLevelUppedSkill );
		_ASSERT( bResult );
		CDnLocalPlayerActor * pLocalPlayerActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));
		pLocalPlayerActor->ReplacementGlyph( hLevelUppedSkill ); 
		
		// Ensure tree info is also handed over.
		CDnSkillTreeSystem::S_NODE_RENDER_INFO sInfo = hSkillToLevelUp->GetNodeRenderInfo();
		hLevelUppedSkill->SetNodeRenderInfo( sInfo );

#ifdef PRE_FIX_SKILL_TOOLTIP_REFRESH
		CDnSkillTreeDlg* pSkillDlg  = (CDnSkillTreeDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG );
		if( pSkillDlg )
		{
			CDnSkillTreeContentDlg* pSkillContentDlg = pSkillDlg->GetContentPanelDlg();
			if( pSkillContentDlg )
			{
				pSkillContentDlg->RefreshToolTip( hSkillToLevelUp );
			}
		}
#endif

#if defined(PRE_FIX_61821)
		DnSkillHandle hNewSkill = hLocalActor->FindSkill(hLevelUppedSkill->GetClassID());

		//If passive skill, update passive skill UI..
		CDnSkill::SkillTypeEnum skillType = hNewSkill->GetSkillType();
		switch(skillType)
		{
		case CDnSkill::Passive:
		case CDnSkill::AutoPassive:
			{
				CDnMainDlg *pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
				if( pMainDlg && hNewSkill && hNewSkill->HasCoolTime() ) 
				{
					pMainDlg->ReplacePassiveSkill( hNewSkill );
				}
			}
			break;
		}
#endif // PRE_FIX_61821

		if( hLevelUppedSkill->GetSkillType() == CDnSkill::Active )
		{
			CDnMainDlg *pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
			int nSkillSlotIndex = pMainDlg->GetSkillQuickSlotIndex( hLevelUppedSkill->GetClassID() );
			if( nSkillSlotIndex != -1 )
				pMainDlg->SetQuickSlot( nSkillSlotIndex, hLevelUppedSkill.GetPointer() );
		}

		SAFE_RELEASE_SPTR( hSkillToLevelUp );

		if( true == bRefreshDialog )
			GetInterface().RefreshSkillDialog();
	}
}


void CDnSkillTask::OnRecvOtherPlayerSkillLevelUp( SCOtherPlayerSkillLevelUp* pPacket )
{
	CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
	DWORD dwNumParty = pPartyTask->GetPartyCount();
	if( pPacket->nPartyIndex < (int)dwNumParty )
	{
		CDnPartyTask::PartyStruct* pParty = pPartyTask->GetPartyData( pPacket->nPartyIndex );
		pParty->hActor->RemoveSkill( pPacket->nSkillID );

		//DnSkillHandle hSkill = CDnSkill::CreateSkill( pParty->hActor, pPacket->nSkillID, pPacket->cLevel );
		//bool bResult = hLocalActor->ReplacementSkill( hSkill );
		//_ASSERT( bResult );
		//pParty->hActor->AddSkill( hSkill );
		pParty->hActor->AddSkill( pPacket->nSkillID, pPacket->cLevel );

	}
}

void CDnSkillTask::OnRecvPushSkillPoint( SCPushSkillPoint* pPacket )
{
	for(int nPage=0; nPage<DualSkill::Type::MAX; nPage++)
		SetSkillPoint( pPacket->usSkillPoint , nPage );
	GetInterface().RefreshSkillDialog();
}

// Results for both skill book unlocks and paid unlocks in town arrive via this packet.
// Gold deduction packet is sent separately by the village server.
void CDnSkillTask::OnRecvUnlockSkill( SCUnlockSkill* pPacket )
{
	m_bRequestWait = false;
	if( ERROR_NONE == pPacket->nRetCode )
	{
		// Skill unlock
		_ASSERT( 0 != pPacket->nSkillID );

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		Unlock_ReservationSkill( pPacket->nSkillID );
#else	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		// Find the skill and change its state to unlocked.
		DnSkillHandle hSkillToUnlock = FindLockedSkill( pPacket->nSkillID );
		
		if( !hSkillToUnlock )
			return;

		hSkillToUnlock->UnLock();

		// Add to unlocked skill list.
		m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].push_back( hSkillToUnlock );

#endif	//	#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	}
}

void CDnSkillTask::OnRecvAcquireSkill( SCAcquireSkill* pPacket )
{
	m_bRequestWait = false;
	if( ERROR_NONE == pPacket->nRetCode )
	{
		Acquire_ReservationSkill( pPacket->nSkillID );
	}
	else
	{
		// Acquisition failed.
		ErrorMessage_ReservationSkill( pPacket->nRetCode );

		CDnTradeMail& tradeMail = GetTradeTask().GetTradeMail();
		if (tradeMail.IsOnMailMode())
			tradeMail.LockMailDlg(false);
	}
}

void CDnSkillTask::OnRecvSkillReset( SCSkillReset* pPacket )
{
	if( m_bAddSkillCheatEnabled )
	{
		m_bAddSkillCheatEnabled = false;
		return;
	}

	DNTableFileFormat*  pDefaultCreateTable = GetDNTable( CDnTableDB::TDEFAULTCREATE );
	CDnLocalPlayerActor* pLocalPlayerActor = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	vector<int> vlJobHistory;
	pLocalPlayerActor->GetJobHistory( vlJobHistory );
	int iFirstJob = (int)vlJobHistory.front();

	char caScratchBuffer[ 64 ];
	SecureZeroMemory( caScratchBuffer, sizeof(caScratchBuffer) );

	// Set only base skills to level 1 and recreate all other owned skills in unacquired state
	vector<DnSkillHandle> vlhDefaultSkills;


	int iResetSkillPage = pPacket->cSkillPage;
	if(iResetSkillPage < DualSkill::Type::Primary || iResetSkillPage >= DualSkill::Type::MAX )
		return;

	int iNumOpenedSkill = (int)m_vlhLocalPlayerOpenedSkillList[iResetSkillPage].size();
	for( int iSkill = 0; iSkill < iNumOpenedSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhLocalPlayerOpenedSkillList[iResetSkillPage].at( iSkill );
		int iSkillID = hSkill->GetClassID();

		
		bool bDefaultSkill = false;
		for( int i = 0; i < DEFAULTSKILLMAX; ++i )
		{
			sprintf_s( caScratchBuffer, "_DefaultSkill%d", i );
			int iDefaultSkilID = pDefaultCreateTable->GetFieldFromLablePtr( iFirstJob, caScratchBuffer )->GetInteger();
			if( iDefaultSkilID == iSkillID )
			{
				bDefaultSkill = true;
				break;
			}
		}

		if( bDefaultSkill )
		{
			int nLevelUpValue = CDnActor::s_hLocalActor->GetSkillLevelUpValue(iSkillID);
			int nSkillLevel = 1;
			nSkillLevel += nLevelUpValue;

			DnSkillHandle hResettedDefaultSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, iSkillID, nSkillLevel );
			if (!hResettedDefaultSkill && nLevelUpValue > 0)
			{
				OutputDebug("Failed to create skill (%d) level (%d).. Check skill level table!!!!!!\n", iSkillID, nSkillLevel);
				_ASSERT( hResettedDefaultSkill && "Skill level-up attempt failed.. Check level table" );

				//Revert level value back down to original
				nSkillLevel -= nLevelUpValue;
				hResettedDefaultSkill = CDnSkill::CreateSkill(CDnActor::s_hLocalActor, iSkillID, nSkillLevel);

				//Reset level-up value
				nLevelUpValue = 0;
			}

			if (hResettedDefaultSkill)
				hResettedDefaultSkill->SetLevelUpValue(nLevelUpValue);
			
			hResettedDefaultSkill->SetHasActor( CDnActor::s_hLocalActor );
			hResettedDefaultSkill->Acquire();
			vlhDefaultSkills.push_back( hResettedDefaultSkill );
			
			// If default skill, replace with level 1 default skill.
			CDnActor::s_hLocalActor->ReplacementSkill( hResettedDefaultSkill );
		}
		else
		{
			DnSkillHandle hResettedZeroLevelSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, iSkillID, 0 );
			m_vlhUnlockZeroLevelSkills[iResetSkillPage].push_back( hResettedZeroLevelSkill );
			pLocalPlayerActor->RemoveSkill( hResettedZeroLevelSkill->GetClassID() );
		}

		// Whether replaced or removed, the existing skill object must be deleted.
		SAFE_RELEASE_SPTR( hSkill );
	}

	m_vlhLocalPlayerOpenedSkillList[iResetSkillPage].swap( vlhDefaultSkills );
	// Rebuild the skill tree info.
	m_mapSkillTreeInfoByJobID.clear();
	InitializeSkillTree();

	// Set skill points to the value received from the server.
	// If no skill level-up was done, the server sends 0, so keep skill points unchanged.

	if( 0 < pPacket->nResultSkillPoint )
	{
		m_iSkillPoint[iResetSkillPage] = pPacket->nResultSkillPoint;
	}

	GetInterface().RefreshSkillDialog();
}

void CDnSkillTask::ResetSkillsByJob( int iStartJobDegree, int iEndJobDegree, int iResultSP , int nSkillPage)
{
	DNTableFileFormat*  pDefaultCreateTable = GetDNTable( CDnTableDB::TDEFAULTCREATE );
	CDnLocalPlayerActor* pLocalPlayerActor = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	vector<int> vlJobHistory;
	pLocalPlayerActor->GetJobHistory( vlJobHistory );
	int iFirstJob = (int)vlJobHistory.front();

	// Job tier from server is as defined in the table, so subtract 1 for proper indexing.
	--iStartJobDegree;
	--iEndJobDegree;

	_ASSERT( 0 <= iStartJobDegree && iStartJobDegree < (int)vlJobHistory.size() );
	_ASSERT( 0 <= iEndJobDegree && iEndJobDegree < (int)vlJobHistory.size() );

	vector<int> vlJobsToReset;
	for( int i = iStartJobDegree; i <= iEndJobDegree; ++i )
	{
		// Job tier does not match the current character. If it matches the item's skill reset job tier range 
		// Return an error code to the client indicating it does not match the current job tier.
		// Allow use if at least one matches the current job tier.
		if( i < (int)vlJobHistory.size() )
			vlJobsToReset.push_back( vlJobHistory.at(i) );
	}

	char caScratchBuffer[ 64 ];
	SecureZeroMemory( caScratchBuffer, sizeof(caScratchBuffer) );

	// Set only base skills to level 1 and recreate all other owned skills in unacquired state
	vector<DnSkillHandle> vlhResultOpenedSkillList;
	if(nSkillPage < DualSkill::Type::Primary || nSkillPage >= DualSkill::Type::MAX )
		return;

	int iNumOpenedSkill = (int)m_vlhLocalPlayerOpenedSkillList[nSkillPage].size();
	for( int iSkill = 0; iSkill < iNumOpenedSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhLocalPlayerOpenedSkillList[nSkillPage].at( iSkill );
		// Reset only the skills of the class requested for reset.
		vector<int>::iterator iter = find( vlJobsToReset.begin(), vlJobsToReset.end(), hSkill->GetNeedJobClassID() );
		if( vlJobsToReset.end() == iter )
		{
			vlhResultOpenedSkillList.push_back( hSkill );
			continue;
		}

		int iSkillID = hSkill->GetClassID();

		bool bDefaultSkill = false;
		for( int i = 0; i < DEFAULTSKILLMAX; ++i )
		{
			sprintf_s( caScratchBuffer, "_DefaultSkill%d", i );
			int iDefaultSkilID = pDefaultCreateTable->GetFieldFromLablePtr( iFirstJob, caScratchBuffer )->GetInteger();
			if( iDefaultSkilID == iSkillID )
			{
				bDefaultSkill = true;
				break;
			}
		}

		if( bDefaultSkill )
		{
			int nLevelUpValue = CDnActor::s_hLocalActor->GetSkillLevelUpValue(iSkillID);
			int nSkillLevel = 1;
			nSkillLevel += nLevelUpValue;

			DnSkillHandle hResettedDefaultSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, iSkillID, nSkillLevel );
			if (!hResettedDefaultSkill && nLevelUpValue > 0)
			{
				OutputDebug("Failed to create skill (%d) level (%d).. Check skill level table!!!!!!\n", iSkillID, nSkillLevel);
				_ASSERT( hResettedDefaultSkill && "Skill level-up attempt failed.. Check level table" );

				//Revert level value back down to original
				nSkillLevel -= nLevelUpValue;
				hResettedDefaultSkill = CDnSkill::CreateSkill(CDnActor::s_hLocalActor, iSkillID, nSkillLevel);

				//Reset level-up value
				nLevelUpValue = 0;
			}

			if (hResettedDefaultSkill)
				hResettedDefaultSkill->SetLevelUpValue(nLevelUpValue);

			hResettedDefaultSkill->SetHasActor( CDnActor::s_hLocalActor );
			hResettedDefaultSkill->Acquire();
			vlhResultOpenedSkillList.push_back( hResettedDefaultSkill );

			// If default skill, replace with level 1 default skill.
			CDnActor::s_hLocalActor->ReplacementSkill( hResettedDefaultSkill );
		}
		else
		{
			DnSkillHandle hResettedZeroLevelSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, iSkillID, 0 );
			m_vlhUnlockZeroLevelSkills[nSkillPage].push_back( hResettedZeroLevelSkill );
			pLocalPlayerActor->RemoveSkill( hResettedZeroLevelSkill->GetClassID() );
		}

		// Whether replaced or removed, the existing skill object must be deleted.
		SAFE_RELEASE_SPTR( hSkill );
	}

	m_vlhLocalPlayerOpenedSkillList[nSkillPage].swap( vlhResultOpenedSkillList );

	// Rebuild the skill tree info.
	m_mapSkillTreeInfoByJobID.clear();
	InitializeSkillTree();

	// Set skill points to the value received from the server.
	// When resetting skills using a job change item,
	// If no skill level-up was done, the server sends 0, so keep skill points unchanged.
	if( 0 < iResultSP )
	{
		m_iSkillPoint[nSkillPage] = iResultSP;
	}

	GetInterface().RefreshSkillDialog();

	// Show effect.
	DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
	if( hHandle )
	{
		hHandle->SetPosition( *(CDnActor::s_hLocalActor->GetPosition()) );
		hHandle->SetActionQueue( "BuySkill" );
		EtcObjectSignalStruct *pResult = CDnActor::s_hLocalActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );

		pResult->vOffset = EtVector3( 0.f, 0.f, 0.f );
		pResult->vRotate = EtVector3( 0, 0, 0);
		pResult->bLinkObject = true;
		pResult->bDefendenceParent = true;
	}
}

void CDnSkillTask::OnRecvCanUseSkillResetCashItem( SCCanUseSkillResetCashItem* pPacket )
{
	if( ERROR_NONE == pPacket->nRet )
	{
#ifdef PRE_MOD_SKILLRESETITEM
		CDnItem* pItem = GetItemTask().GetCashInventory().FindItemFromSerialID( pPacket->biItemSerial );
		if (pItem == NULL)
			pItem = GetItemTask().GetCharInventory().FindItemFromSerialID( pPacket->biItemSerial );
		_ASSERT( pItem );
		if( pItem )
		{
			// Ask whether to actually reset.
			GetInterface().OpenSkillResetConfirmDlg(pItem, pPacket->nRewardSP);
		}
#else
		CDnItem* pItem = GetItemTask().GetCashInventory().FindItemFromSerialID( pPacket->biItemSerial );
		_ASSERT( pItem );
		if( pItem )
		{
			// Ask whether to actually reset.
			GetInterface().OpenSkillResetConfirmDlg( pPacket->biItemSerial, pItem->GetTypeParam( 0 ), pItem->GetTypeParam( 1 ), pPacket->nRewardSP );
		}
#endif
	}
	else
	{
		switch( pPacket->nRet )
		{
			case ERROR_ITEM_DONT_HAVE_SKILL_TO_RESET:
				GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114028) );
				//GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114028) );
				break;

			case ERROR_ITEM_ALREADY_HAVE_HIGHER_JOB_SKILL:
				GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114027) );
				//GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114027) );
				break;

			case ERROR_ITEM_MISMATCH_RESET_SKILL_JOB:
				GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114025) );
				//GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114025) );
				break;
		}
	}
}

void CDnSkillTask::OnRecvUseSkillResetCashItemRes( SCUseSkillResetCashItemRes* pPacket )
{
	m_bRequestWait = false;
	if( ERROR_NONE == pPacket->nRet )
	{
		ResetSkillsByJob( m_iSkillResetStartJobDegree, m_iSkillResetEndJobDegree, pPacket->nResultSP , pPacket->cSkillPage );
		m_iSkillResetStartJobDegree = 0;
		m_iSkillResetEndJobDegree = 0;
	}
};

#ifdef PRE_ADD_PRESET_SKILLTREE
void CDnSkillTask::OnRecvSkillSetList(SCSKillSetList * packet)
{
	m_vecPresetSkillTree.clear();

	int nSkillCount = 0;
	for( int itr = 0; itr < SKILLPRESETMAX; ++itr )
	{
		m_vecPresetSkillTree.push_back( SPresetSkillTree() );

		m_vecPresetSkillTree[itr].m_cIndex = packet->SKillIndex[itr].cIndex;
		m_vecPresetSkillTree[itr].m_wszTreeName = std::wstring( packet->SKillIndex[itr].wszSetName );

		for( BYTE cCount = 0; cCount < packet->SKillIndex[itr].cCount; ++cCount )
		{
			int nSkillID = packet->SKills[nSkillCount].nSkillID;
			BYTE cLevel = packet->SKills[nSkillCount].cLevel;
			m_vecPresetSkillTree[itr].m_Data.push_back( std::pair<int, BYTE>( nSkillID, cLevel ) );
			++nSkillCount;
		}
	}

	for( DWORD itr = 0; itr < m_vecPresetSkillTree.size(); ++itr )
	{
		if( true == m_vecPresetSkillTree[itr].m_Data.empty() )
			continue;

		std::vector< std::pair<int, BYTE> > vecData;
		vecData.clear();

		AdviceSkillTreeSort( m_vecPresetSkillTree[itr].m_Data, vecData );
		m_vecPresetSkillTree[itr].m_Data.clear();

		m_vecPresetSkillTree[itr].m_Data.resize( vecData.size() );
		std::copy( vecData.begin(), vecData.end(), m_vecPresetSkillTree[itr].m_Data.begin() );
	}

	SetPresetSkillTree();
}

void CDnSkillTask::OnRecvSkillSetSaveResult(SCSKillSaveResult * pPacket)
{
	if( ERROR_NONE == pPacket->nRetCode )
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 745 ) );	// UISTRING : Skill tree saved.
	else
		return;

	if( m_SavePresetSkillTree.m_Data.empty() )
		return;

	for( DWORD itr = 0; itr < m_vecPresetSkillTree.size(); ++itr )
	{
		if( m_SavePresetSkillTree.m_cIndex != m_vecPresetSkillTree[itr].m_cIndex )
			continue;

		m_vecPresetSkillTree[itr].m_wszTreeName = m_SavePresetSkillTree.m_wszTreeName;

		m_vecPresetSkillTree[itr].m_Data.clear();
		m_vecPresetSkillTree[itr].m_Data.resize( m_SavePresetSkillTree.m_Data.size() );
		std::copy( m_SavePresetSkillTree.m_Data.begin(), m_SavePresetSkillTree.m_Data.end(), m_vecPresetSkillTree[itr].m_Data.begin() );

		break;
	}
	m_SavePresetSkillTree.Clear();

	CDnSkillTreeDlg* pSkillDlg  = (CDnSkillTreeDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG );
	if( pSkillDlg )
		pSkillDlg->UpdateView();
}

void CDnSkillTask::OnRecvSkillSetDeleteResult(SCSKillDeleteResult * pPacket)
{
	if( ERROR_NONE == pPacket->nRetCode )
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 109 ) );	// UISTRING : Deletion complete
	else
		return;

	for( DWORD itr = 0; itr < m_vecPresetSkillTree.size(); ++itr )
	{
		if( m_vecPresetSkillTree[itr].m_cIndex != m_nPresetSkillDeleteIndex )
			continue;

		m_vecPresetSkillTree[itr].Clear();
		m_vecPresetSkillTree[itr].m_cIndex = m_nPresetSkillDeleteIndex;
		m_vecPresetSkillTree[itr].m_bAdvice = false;
		break;
	}

	CDnSkillTreeDlg* pSkillDlg  = (CDnSkillTreeDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG );
	if( pSkillDlg )
		pSkillDlg->UpdateView();
}
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE

// Request skill acquisition to server
void CDnSkillTask::SendAcquireSkillReq( int iSkillID )
{
	if( !m_bRequestWait )
	{
		::SendAcquireSkillReq( iSkillID );
		m_bRequestWait = true;
	}
}

// Request skill level-up to server
void CDnSkillTask::SendSkillLevelUpReq( int iSkillID, int iUsePoint )
{
	if( !m_bRequestWait )
	{
		if( 0 < m_iSkillPoint[m_iSkillTreePage] && iUsePoint <= m_iSkillPoint[m_iSkillTreePage] )
		{
			::SendSkillLevelUpReq( iSkillID, iUsePoint );
		}
		m_bRequestWait = true;
	}
}

// Request skill unlock with money after talking to the skill trainer
void CDnSkillTask::SendUnlockSkillByMoneyReq( int iSkillID )
{
	if( !m_bRequestWait )
	{
		::SendUnlockSkillByMoneyReq( iSkillID );
		m_bRequestWait = true;
	}
}

// Finally send request to server to use the skill reset cash item
#ifdef PRE_MOD_SKILLRESETITEM
void CDnSkillTask::SendUseSkillResetCashItemReq(const INT64 biItemSerial, const int iStartJobDegree, const int iEndJobDegree, const int iInvenType, const int iInvenIndex)
#else
void CDnSkillTask::SendUseSkillResetCashItemReq( INT64 biItemSerial, int iStartJobDegree, int iEndJobDegree )
#endif
{
	// If the previous request has no response yet, cannot request again until it arrives.
	if ((0 == m_iSkillResetStartJobDegree) && ( 0 == m_iSkillResetEndJobDegree))
	{
#ifdef PRE_MOD_SKILLRESETITEM
		::SendUseSkillResetCashItemReq(biItemSerial, iInvenType, iInvenIndex);
#else
		::SendUseSkillResetCashItemReq( biItemSerial );
#endif

		m_iSkillResetStartJobDegree = iStartJobDegree;
		m_iSkillResetEndJobDegree = iEndJobDegree;

		m_bRequestWait = true;
	}
}


DnSkillHandle CDnSkillTask::FindSkill( int nSkillID )
{
	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) {
		if( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->GetClassID() == nSkillID ) return m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i];
	}
	return CDnSkill::Identity();
}

DnSkillHandle CDnSkillTask::FindZeroLevelSkill( int nSkillID )
{
	for( DWORD i=0; i<m_vlhUnlockZeroLevelSkills[m_iSkillTreePage].size(); i++ ) {
		if( m_vlhUnlockZeroLevelSkills[m_iSkillTreePage][i]->GetClassID() == nSkillID ) return m_vlhUnlockZeroLevelSkills[m_iSkillTreePage][i];
	}

	return CDnSkill::Identity();
}

DnSkillHandle CDnSkillTask::FindLockedSkill( int nSkillID )
{
	map<int, S_SKILLTREE_INFO>::iterator iter = m_mapSkillTreeInfoByJobID.begin();

	for( iter; m_mapSkillTreeInfoByJobID.end() != iter; ++iter )
	{
		for( int iSlot = 0; iSlot < MAX_SKILL_SLOT_COUNT; ++iSlot )
		{
			S_SKILLTREE_SLOT_INFO& SkillTreeSlotInfo = iter->second.aSkillTreeSlotInfo[ iSlot ];
			if( SkillTreeSlotInfo.hSkill && nSkillID == SkillTreeSlotInfo.hSkill->GetClassID() )
				return SkillTreeSlotInfo.hSkill;
		}
	}

	return CDnSkill::Identity();
}

DnSkillHandle CDnSkillTask::FindSkillBySkillBookItemID( int iSkillBookItemID )
{
	int iSkillID = m_SkillTreeSystem.FindSkillBySkillBook( iSkillBookItemID );
	
	return FindSkill( iSkillID );
}

DnSkillHandle CDnSkillTask::FindZeroLevelSkillBySkillBookItemID( int iSkillBookItemID )
{
	int iSkillID = m_SkillTreeSystem.FindSkillBySkillBook( iSkillBookItemID );

	return FindZeroLevelSkill( iSkillID );
}

#ifdef PRE_ADD_LEVELUP_GUIDE
void CDnSkillTask::GetLevelUpSkillInfo(std::vector<int>& newSkillIdList, int newLevel, int jobId)
{
	m_SkillTreeSystem.GetLevelUpSkillInfo(newSkillIdList, newLevel, jobId);
}

void CDnSkillTask::GetLevelUpSkillInfo(std::vector<int>& newSkillIdList, int newLevel, const std::vector<int>& jobHistoryVec)
{
	m_SkillTreeSystem.GetLevelUpSkillInfo(newSkillIdList, newLevel, jobHistoryVec);
}
#endif

DnSkillHandle CDnSkillTask::FindEmblemSkill( void )
{
	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
	{
		if( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->IsEquipItemSkill() )
			return m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i];
	}

	return CDnSkill::Identity();
}

void CDnSkillTask::ResetNewAcquireSkills( void )
{
	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
	{
		if( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->IsNewAcquire() )
			m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->ResetNewAcquire();
	}
}

int CDnSkillTask::_GetAccumlatedNeedSkillPoint( DnSkillHandle hSkill, int iSkillLevel )
{
	int iAccuemlatedUsedSkillPoint = 0;

	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

	vector<int> vlSkillLevelList;
	//pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", hSkill->GetClassID(), vlSkillLevelList );
	GetSkillLevelList( hSkill->GetClassID(), vlSkillLevelList, CDnSkill::PVE );		// Just process based on PvE.
	for( DWORD k = 0; k < vlSkillLevelList.size(); k++ ) 
	{
		int iSkillLevelTableID = vlSkillLevelList.at( k );
		int iSkillLevelInTable = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SkillLevel" )->GetInteger();
		if( iSkillLevel < iSkillLevelInTable )
			break;

		int iNeedSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_NeedSkillPoint" )->GetInteger();
		iAccuemlatedUsedSkillPoint += iNeedSkillPoint;
	}	
	
	return iAccuemlatedUsedSkillPoint;
}

int CDnSkillTask::GetWholeUsedSkillPoint( void )
{
	int iWholeUsedSkillPoint = 0;

	for( DWORD i = 0; i < m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
	{
		DnSkillHandle hSkill = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][ i ];

		if( hSkill->IsAcquired() )
		{
			// Reference value directly from the table.
			int iSkillLevel = hSkill->GetLevel();
			iSkillLevel -= hSkill->GetLevelUpValue();
			iWholeUsedSkillPoint += _GetAccumlatedNeedSkillPoint( hSkill, iSkillLevel );
		}
	}

	return iWholeUsedSkillPoint;
}

int CDnSkillTask::GetUsedSkillPointInThisJob( int iJobID )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor )
		return 0;

	int iWholeUsedSPInThisJob = 0;

	for( DWORD i = 0; i < m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
	{
		DnSkillHandle hSkill = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][ i ];

		int nItemLevel = 0;

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		DnSkillHandle hReservationSkill = GetReservationSkill( hSkill->GetClassID() );
		if( hReservationSkill )
		{
			nItemLevel = hLocalActor->GetSkillLevelUpValue( hReservationSkill->GetClassID() );
			hSkill = hReservationSkill;
		}
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

		if( hSkill->IsAcquired() && 
			hSkill->GetNeedJobClassID() == iJobID )
		{
			// Reference value directly from the table.
			int iSkillLevel = hSkill->GetLevel() - nItemLevel;
			iSkillLevel -= hSkill->GetLevelUpValue();
			iWholeUsedSPInThisJob += _GetAccumlatedNeedSkillPoint( hSkill, iSkillLevel );
		}
	}

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	iWholeUsedSPInThisJob += GetUsedReservationSkillPointInThisJob( iJobID );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

	return iWholeUsedSPInThisJob;
}

CDnSkillTreeSystem::RESULT_CODE CDnSkillTask::CanUnlockThis( DnSkillHandle hSkill )
{
	if( !hSkill )
		return CDnSkillTreeSystem::R_ERROR;

	if( !CDnActor::s_hLocalActor )
		return CDnSkillTreeSystem::R_ERROR;

	CDnSkillTreeSystem::RESULT_CODE eResult = CDnSkillTreeSystem::R_ERROR;

	// Skill book ownership is not checked here.
	CDnSkillTreeSystem::S_OUTPUT Output;

	CDnSkillTreeSystem::S_TRY_UNLOCK TryUnlockInfo;
	TryUnlockInfo.iTryUnlockSkillID = hSkill->GetClassID();
	TryUnlockInfo.iCurrentCharLevel = CDnActor::s_hLocalActor->GetLevel();
	TryUnlockInfo.iSkillBookItemID = hSkill->GetUnlockSkillBookItemID();

	m_SkillTreeSystem.TryUnLockSkill( TryUnlockInfo, &Output );

#if defined( PRE_ADD_ONLY_SKILLBOOK )
	if( CDnSkillTreeSystem::R_ONLY_SKILL_BOOK == Output.eResult )
		return Output.eResult;
#endif	// #if defined( PRE_ADD_ONLY_SKILLBOOK )

	bool bExclusiveSkill = GetSkillTask().IsExclusiveSkill( hSkill->GetClassID(), hSkill->GetExclusiveID() );
	if( bExclusiveSkill ) return CDnSkillTreeSystem::R_EXCLUSIVE_SKILL;

	if( CDnSkillTreeSystem::R_SUCCESS == Output.eResult )
	{
		vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO> vlPossessedSkillInfos;

		for( int i = 0; i < (int)m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); ++i )
		{
			CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
			DnSkillHandle hSkill = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].at( i );

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
			DnSkillHandle hReservationSkill = GetReservationSkill( hSkill->GetClassID() );
			if( hReservationSkill )
				hSkill = hReservationSkill;
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

			SkillInfo.iSkillID = hSkill->GetClassID();
			SkillInfo.iSkillLevel = hSkill->GetLevel();

			//#37528 Ignore bonus levels from skill level-up items when checking prerequisite skill level-up conditions.
			//Deduct by the skill level-up amount.
			SkillInfo.iSkillLevel -= hSkill->GetLevelUpValue();

			SkillInfo.bCurrentLock = false;

			vlPossessedSkillInfos.push_back( SkillInfo );
		}

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		GetPossessReservationSkill( vlPossessedSkillInfos );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

		CDnSkillTreeSystem::S_TRY_ACQUIRE TryAcquire( vlPossessedSkillInfos );
		TryAcquire.iCurrentCharLevel = CDnActor::s_hLocalActor->GetLevel();
		TryAcquire.iTryAcquireSkillID = hSkill->GetClassID();

		int iWholeAvailSPByJob = GetAvailSkillPointForThisJob( hSkill->GetNeedJobClassID() );
		TryAcquire.iHasSkillPoint = iWholeAvailSPByJob;
		
		m_SkillTreeSystem.TryAcquireSkill( TryAcquire, &Output );

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
		std::vector<int> nNeedSPValues;
		GetNeedSPValuesByJob(hSkill->GetClassID(), nNeedSPValues);

		bool bAvailableSPByJob = IsAvailableSPByJob(CDnActor::s_hLocalActor, nNeedSPValues);
		if (bAvailableSPByJob == false)
			Output.eResult = CDnSkillTreeSystem::R_ERROR;
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP
		
		eResult = Output.eResult;
	}

	return eResult;
}

void CDnSkillTask::RefreshAllSkillsDecreaseMP( void )
{
	map<int, S_SKILLTREE_INFO>::iterator iter = m_mapSkillTreeInfoByJobID.begin();
	for( iter; m_mapSkillTreeInfoByJobID.end() != iter; ++iter )
	{
		S_SKILLTREE_INFO& SkillInfo = iter->second;
		for( int i = 0; i < MAX_SKILL_SLOT_COUNT; ++i )
		{
			if( SkillInfo.aSkillTreeSlotInfo[ i ].hSkill )
			{
				SkillInfo.aSkillTreeSlotInfo[ i ].hSkill->RefreshDecreaseMP();
			}
		}
	}
}

int CDnSkillTask::GetAvailSkillPointForThisJob( int iNeedJobID )
{
	// Current job tier
	CDnLocalPlayerActor* pLocalPlayerActor = static_cast<CDnLocalPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
	int iJobDegree = -1;

	vector<int> vlJobHistory;
	pLocalPlayerActor->GetJobHistory( vlJobHistory );
	for( int i = 0; i < (int)vlJobHistory.size(); ++i )
	{
		int iJobID = vlJobHistory.at( i );
		if( iJobID == iNeedJobID )
		{
			iJobDegree = i;
			break;
		}
	}

	_ASSERT( -1 != iJobDegree );

	int iWholeAvailSPByJob = pLocalPlayerActor->GetSPUsingLimitByJobArrayIndex( iJobDegree );
	int iAvailSkillPoint = iWholeAvailSPByJob - GetSkillTask().GetUsedSkillPointInThisJob( iNeedJobID );

	// If remaining job SP exceeds total usable SP, total usable SP is authoritative, so return that amount.
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	const int nSkillPoint = GetSkillPoint() - GetReservationSkillNeedSP();
	if( nSkillPoint < iAvailSkillPoint )
		iAvailSkillPoint = nSkillPoint;
#else	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	if( GetSkillPoint() < iAvailSkillPoint )
		iAvailSkillPoint = GetSkillPoint();
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

	return iAvailSkillPoint;
}

bool CDnSkillTask::IsExclusiveSkill( int iSkillID, int iExclusiveID )
{
	// True if any learned skill has an ID that prevents learning the same skill.

	int iNumSkill = (int)m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size();
	for( int iSkill = 0; iSkill < iNumSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].at( iSkill );
		
		// Excluding own skills.
		if( hSkill->GetClassID() == iSkillID )
			continue;

		if( 0 == hSkill->GetExclusiveID() )
			continue;

		if( iExclusiveID == hSkill->GetExclusiveID() )
		{
			return true;
		}
	}

	return false;
}

void CDnSkillTask::GetSkillLevelList( int iSkillID, vector<int>& vlSkillLevelList, int iLevelDataType )
{
	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

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

CSecondarySkill * CDnSkillTask::GetSecondarySkill( SecondarySkill::SubType::eType eSecondaryType )
{
	std::map<int,CSecondarySkill*>::iterator iter = m_LifeSkillRepository.GetSecondarySkillMap().begin();
	for( ; iter != m_LifeSkillRepository.GetSecondarySkillMap().end(); iter++ )
	{
		if( eSecondaryType == ( (*iter).second )->GetSubType() )
		{
			return (*iter).second;
		}
	}
	return NULL;
}

void CDnSkillTask::SendLifeSkillDelete( int nSkillID )
{
	SecondarySkill::CSDelete LifeSkillDelete;
	LifeSkillDelete.iSkillID = nSkillID;

	CClientSessionManager::GetInstance().SendPacket( CS_SECONDARYSKILL, eSecondarySkill::CS_DELETE, (char*)&LifeSkillDelete, int(sizeof(LifeSkillDelete)) );
}

void CDnSkillTask::OnRecvLifeSkillAdd( SecondarySkill::SCCreate* pPacket )
{
	if( pPacket == NULL )
		return;

	m_LifeSkillRepository.Create( pPacket->iSkillID );

	CDnSkillTreeDlg* pSkillTreeDlg = static_cast<CDnSkillTreeDlg*>( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG ) );
	if( pSkillTreeDlg && pSkillTreeDlg->IsShow() )
		pSkillTreeDlg->LifeSkillAdd( m_LifeSkillRepository.Get( pPacket->iSkillID ) );

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSecondarySkill );
	SecondarySkill::SubType::eType SubType = (SecondarySkill::SubType::eType)pSox->GetFieldFromLablePtr( pPacket->iSkillID, "_SecondarySkillType" )->GetInteger();

	WCHAR wszString[256] = {0,};
	if( SecondarySkill::SubType::CultivationSkill == SubType )
		swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7316 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7301 ) );
	else if( SecondarySkill::SubType::CookingSkill == SubType )
		swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7316 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7300 ) );
	else if( SecondarySkill::SubType::FishingSkill == SubType )
		swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7316 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7302 ) );

	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszString, false );	//Acquired %S
}

void CDnSkillTask::OnRecvLifeSkillDelete( SecondarySkill::SCDelete* pPacket )
{
	if( pPacket == NULL )
		return;

	if( pPacket->iRet == 0 )
	{
		m_LifeSkillRepository.Delete( pPacket->iSkillID );

		CDnSkillTreeDlg* pSkillTreeDlg = static_cast<CDnSkillTreeDlg*>( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG ) );
		if( pSkillTreeDlg && pSkillTreeDlg->IsShow() )
			pSkillTreeDlg->LifeSkillDelete( pPacket->iSkillID );

		GetInterface().GetMainBarDialog()->ResetSecondarySkillQuickSlot( pPacket->iSkillID );

		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSecondarySkill );
		SecondarySkill::SubType::eType SubType = (SecondarySkill::SubType::eType)pSox->GetFieldFromLablePtr( pPacket->iSkillID, "_SecondarySkillType" )->GetInteger();

		WCHAR wszString[256] = {0,};
		if( SecondarySkill::SubType::CultivationSkill == SubType )
			swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7317 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7301 ) );
		else if( SecondarySkill::SubType::CookingSkill == SubType )
			swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7317 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7300 ) );
		else if( SecondarySkill::SubType::FishingSkill == SubType )
			swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7317 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7302 ) );
		
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszString, false );	//	Deleted %S
	}
	else if( pPacket->iRet == ERROR_SECONDARYSKILL_DELETE_FAILED )
	{
		// Failed to delete secondary skill
	}
	else if( pPacket->iRet == ERROR_SECONDARYSKILL_DELETE_FAILED_DONTALLOWMAP )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7514 ) );
	}
}

void CDnSkillTask::OnRecvLifeSkillUpdateExp( SecondarySkill::SCUpdateExp* pPacket )
{
	if( pPacket == NULL )
		return;

	CSecondarySkill* pLifeSkill = m_LifeSkillRepository.Get( pPacket->iSkillID );

	if( pLifeSkill )
	{
		WCHAR wszString[256];
		swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7474 ), pPacket->iExp - pLifeSkill->GetExp() );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszString, false );	// Skill proficiency increased by %d.

		pLifeSkill->SetGrade( pPacket->Grade );
		pLifeSkill->SetLevel( pPacket->iLevel );
		pLifeSkill->SetExp( pPacket->iExp, NULL );
	}

	CDnSkillTreeDlg* pSkillTreeDlg = static_cast<CDnSkillTreeDlg*>( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG ) );
	if( pSkillTreeDlg && pSkillTreeDlg->IsShow() )
		pSkillTreeDlg->LifeSkillUpdateExp( pPacket->iSkillID, pPacket->Grade, pPacket->iLevel, pPacket->iExp );
}

void CDnSkillTask::OnRecvLifeSkillList( SecondarySkill::SCList* pPacket )
{
	if( pPacket == NULL || pPacket->SkillList == NULL )
		return;

	if( pPacket->cCount > SecondarySkill::Max::LearnSecondarySkill )
		return;

	for( int i=0; i<pPacket->cCount; i++ )
	{
		m_LifeSkillRepository.Add( pPacket->SkillList[i].iSkillID, pPacket->SkillList[i].iExp );
		CSecondarySkill* pLifeSkill = m_LifeSkillRepository.Get( pPacket->SkillList[i].iSkillID );

		if( pLifeSkill )
		{
			pLifeSkill->SetGrade( pPacket->SkillList[i].Grade );
			pLifeSkill->SetLevel( pPacket->SkillList[i].iLevel );
		}
	}
}

bool CDnSkillTask::CheckAvailableDeleteLifeSkill( int nSkillID )
{
	DNTableFileFormat* pSoxSkill = GetDNTable( CDnTableDB::TSecondarySkill );
	if( pSoxSkill == NULL )
	{
		DN_ASSERT( 0, "Invalid SecondarySkillType" );
		return false;
	}

	bool bAvailableDelete = false;

	SecondarySkill::SubType::eType SubType;
	SubType = (SecondarySkill::SubType::eType)pSoxSkill->GetFieldFromLablePtr( nSkillID, "_SecondarySkillType" )->GetInteger();

	switch( SubType )
	{
	case SecondarySkill::SubType::CookingSkill:
		{
			CManufactureSkill* pCookingSkill = GetLifeSkillCookingTask().GetCookingSkill();
			if( pCookingSkill )
			{
				if( static_cast<int>( pCookingSkill->GetRecipeCount() ) == 0 )
					bAvailableDelete = true;
				else
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7314 ) );
			}
			else
				bAvailableDelete = true;
		}
		break;
	default:
		{
			bAvailableDelete = true;
		}
		break;
	}

	return bAvailableDelete;
}

void CDnSkillTask::DoLifeSkill( int nSkillID )
{
	DNTableFileFormat* pSoxSkill = GetDNTable( CDnTableDB::TSecondarySkill );
	if( pSoxSkill == NULL )
	{
		DN_ASSERT( 0, "Invalid SecondarySkillType" );
		return;
	}

	SecondarySkill::SubType::eType SubType;
	SubType = (SecondarySkill::SubType::eType)pSoxSkill->GetFieldFromLablePtr( nSkillID, "_SecondarySkillType" )->GetInteger();

	switch( SubType )
	{
	case SecondarySkill::SubType::CookingSkill:
		{
			DoSkillCooking();
		}
		break;
	case SecondarySkill::SubType::FishingSkill:
		{
			DoSkillFishing();
		}
		break;
	default:
		{
		}
		break;
	}
}

void CDnSkillTask::DoSkillFishing()
{
	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
	if( pPlayer )
	{
		CDnLifeSkillFishingTask* pTask = dynamic_cast<CDnLifeSkillFishingTask*>( CTaskManager::GetInstance().GetTask( "LifeSkillFishingTask" ) );
		if( pTask )
			pTask->SendFishingReady();
	}
}

void CDnSkillTask::DoSkillCooking()
{
	GetInterface().OpenCookingDialog( true );
}

void CDnSkillTask::DoSkillNone()
{
}

void CDnSkillTask::LoadGuildWarSkillList( PvPCommon::Team::eTeam eTeam )
{
	SAFE_RELEASE_SPTRVEC( m_vlhGuildWarSkillList );

	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILLTREE );
	if( pSox == NULL )
	{
		DN_ASSERT( 0, "Invalid BattleGroundSkillTree" );
		return;
	}

	for( int i=0; i<=pSox->GetItemCount(); i++ )
	{
		int nItemID = pSox->GetItemID( i );
		int nTeamID = pSox->GetFieldFromLablePtr( nItemID, "_TeamID" )->GetInteger();

		if( nTeamID == 0 || nTeamID == eTeam )
		{
			int nSkillID = pSox->GetFieldFromLablePtr( nItemID, "_SkillTableID" )->GetInteger();
			DnSkillHandle hSkill;
			hSkill = CDnGuildWarSkill::CreateSkill( CDnActor::Identity(), nSkillID, 0 );
			if( hSkill )
				m_vlhGuildWarSkillList.push_back( hSkill );
		}
	}
}

DnSkillHandle CDnSkillTask::FindGuildWarSkill( int nSkillTableID )
{
	DnSkillHandle hRetSkill;
	vector<DnSkillHandle>::iterator iter = m_vlhGuildWarSkillList.begin();

	for( ; iter != m_vlhGuildWarSkillList.end(); iter++ )
	{
		DnSkillHandle hSkill = (*iter); 
		if( hSkill->GetClassID() == nSkillTableID )
		{
			hRetSkill = (*iter);
			break;
		}
	}

	return hRetSkill;
}

bool CDnSkillTask::IsGuildWarSkill( int nSkillTableID )
{
	DnSkillHandle hSkill = FindGuildWarSkill( nSkillTableID );
	if( hSkill )
		return true;

	return false;
}

void CDnSkillTask::_OnRecvPVPSkillMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd )
	{
		case ePvP::SC_PVP_TRYACQUIRESKILL:	OnRecvAcquireGuildWarSkillReq( (SCPvPTryAcquireSkill*)pData );	break;
		case ePvP::SC_PVP_INITSKILL:		OnRecvGuildWarSkillInit( (SCPvPInitSkill*)pData );				break;
		case ePvP::SC_PVP_USESKILL:			OnRecvUseGuildWarSkill( (SCPvPUseSkill*)pData );				break;
		case ePvP::SC_PVP_OCCUPATION_SKILLSTATE:	OnRecvGuildWarSkillState( (SCPvPOccupationSkillState*)pData );	break;
		case ePvP::SC_PVP_SWAPSKILLINDEX:			OnRecvGuildSkillSwapIndex( (SCPvPSwapSkillIndex*)pData ); break;
	}
}

void CDnSkillTask::SendAcquireGuildWarSkillReq( int nSkillID, int nSkillLevel, int nReservationIndex )
{
	CDnMainDlg* pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
	if( pMainDlg == NULL )
		return;

	CDnGuildWarSkillSlotDlg* pGuildWarSkillSlotDlg = pMainDlg->GetGuildWarSkillSlotDialog();
	if( pGuildWarSkillSlotDlg == NULL )
		return;

	int nEmptySlotIndex = pGuildWarSkillSlotDlg->FindEmptyQuickSlotIndex() + nReservationIndex;

	CSPvPTryAcquireSKill AcquireSkillReq;
	SecureZeroMemory( &AcquireSkillReq, sizeof(CSPvPTryAcquireSKill) );
	AcquireSkillReq.nSkillID = nSkillID;
	AcquireSkillReq.nLevel = nSkillLevel;
	AcquireSkillReq.cSlotIndex = nEmptySlotIndex;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_TRYACQUIRE_SKILL, (char*)&AcquireSkillReq, int(sizeof(AcquireSkillReq)) );
}

void CDnSkillTask::SendGuildWarSkillInit()
{
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_INITSKILL, NULL, 0 );
}

void CDnSkillTask::SendUseGuildWarSkill( int nSkillID )
{
	CSPvPUseSkill UseSkill;
	UseSkill.nSkillID = nSkillID;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_USESKILL, (char*)&UseSkill, int(sizeof(UseSkill)) );
}

void CDnSkillTask::SendGuildSkillSwapIndex( int nFromIndex, int nToIndex )
{
	CSPvPSwapSkillIndex SwapIndex;
	SwapIndex.cFromIndex = nFromIndex;
	SwapIndex.cToIndex = nToIndex;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_SWAPSKILLINDEX, (char*)&SwapIndex, int(sizeof(SwapIndex)) );
}

void CDnSkillTask::OnRecvAcquireGuildWarSkillReq( SCPvPTryAcquireSkill* pPacket )
{
	_ASSERT( pPacket );

	if( pPacket->nRetCode == ERROR_NONE )
	{
		DnSkillHandle hSkill;
		vector<DnSkillHandle>::iterator iter = m_vlhGuildWarSkillList.begin();
		for( ; iter != m_vlhGuildWarSkillList.end(); iter++ )
		{
			hSkill = (*iter);
			if( hSkill && hSkill->GetClassID() == pPacket->nSkillID )
			{
				hSkill->SetHasActor( CDnActor::s_hLocalActor );
				hSkill->SelectLevelDataType( CDnSkill::PVP, true );
				hSkill->Acquire();
				break;
			}
		}

		if( hSkill != NULL && hSkill->GetSkillType() == CDnSkill::Active )
		{
			// Register skill shortcut
			CDnMainDlg* pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
			if( pMainDlg )
			{
				CDnGuildWarSkillSlotDlg* pGuildWarSkillSlotDlg = pMainDlg->GetGuildWarSkillSlotDialog();
				int nEmptySlotIndex = pGuildWarSkillSlotDlg->FindEmptyQuickSlotIndex();
				if( nEmptySlotIndex > -1 )
					pGuildWarSkillSlotDlg->SetSlot( nEmptySlotIndex, hSkill.GetPointer() );

				GetInterface().RefreshSkillDialog();

				if( pGuildWarSkillSlotDlg->IsShow() )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 723), MB_OK );
					GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 723) );
				}
			}
		}

		GetGuildWarTask().UseGuildWarSkillPoint();
	}
	else
	{
//		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}

void CDnSkillTask::OnRecvGuildWarSkillInit( SCPvPInitSkill* pPacket )
{
	_ASSERT( pPacket );

	if( pPacket->nRetCode == ERROR_NONE )
	{
	}
	else
	{
	}
}

void CDnSkillTask::OnRecvUseGuildWarSkill( SCPvPUseSkill* pPacket )
{
	_ASSERT( pPacket );

	if( pPacket->nRetCode == ERROR_NONE )
	{
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
		if( CDnLocalPlayerActor::s_hLocalActor == NULL ) return;
		if( hActor == NULL ) return;
		
		DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILLLEVEL );
		if( pSkillLevelTable == NULL ) return;
		int nItemID = pSkillLevelTable->GetItemIDFromField( "_SkillIndex", pPacket->nSkillID );

		if( CDnLocalPlayerActor::s_hLocalActor->GetTeam() == hActor->GetTeam() )	// Set cooldown and alarm if on the same team
		{
			// Run cooldown
			DnSkillHandle hSkill = GetSkillTask().FindGuildWarSkill( pPacket->nSkillID );
			if( !hSkill ) return;
			if( hSkill->GetSkillType() != CDnSkill::Active ) return;
			hSkill->OnBeginCoolTime();

			// Output alarm
			int nAlarmStaringIndex = pSkillLevelTable->GetFieldFromLablePtr( nItemID, "_GuildSkillAlarmID" )->GetInteger();
			if( nAlarmStaringIndex > 0 )
				GetInterface().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nAlarmStaringIndex ), textcolor::TOMATO, 3.0f );
		}

		// Action!
		string strActionName = pSkillLevelTable->GetFieldFromLablePtr( nItemID, "_ProcessParam1" )->GetString();
		if( strActionName.length() > 0 )
		{
			CDnPlayerActor* pActor = dynamic_cast<CDnPlayerActor *>( hActor.GetPointer() );
			if( pActor == NULL ) return;

			if( !pActor->IsBattleMode() )
				pActor->CmdToggleBattle( true );

			hActor->CDnActor::CmdAction( strActionName.c_str(), 0, 3.0f, 0.0f, true, false );
		}
	}
	else
	{
//		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}

void CDnSkillTask::OnRecvGuildWarSkillState( SCPvPOccupationSkillState* pPacket )
{
	_ASSERT( pPacket );
	if( !CDnGuildWarTask::IsActive() )
		return;

	GetGuildWarTask().ResetGuildWarSkillPoint();

	for( int i=0; i<QUICKSLOTMAX; i++ )
	{
		if( pPacket->Info.nSkillID[i] == 0 ) break;

		DnSkillHandle hSkill;
		vector<DnSkillHandle>::iterator iter = m_vlhGuildWarSkillList.begin();
		for( ; iter != m_vlhGuildWarSkillList.end(); iter++ )
		{
			hSkill = (*iter);
			if( hSkill && hSkill->GetClassID() == pPacket->Info.nSkillID[i] )
			{
				hSkill->SetHasActor( CDnActor::s_hLocalActor );
				hSkill->SelectLevelDataType( CDnSkill::PVP, true );
				hSkill->Acquire();

				GetGuildWarTask().UseGuildWarSkillPoint();
				break;
			}
		}

		if( hSkill != NULL && hSkill->GetSkillType() == CDnSkill::Active )
		{
			// Register skill shortcut
			CDnMainDlg* pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
			if( pMainDlg )
			{
				CDnGuildWarSkillSlotDlg* pGuildWarSkillSlotDlg = pMainDlg->GetGuildWarSkillSlotDialog();
				pGuildWarSkillSlotDlg->SetSlot( pPacket->Info.cSlotIndex[i], hSkill.GetPointer() );

				if( pPacket->Info.nSKillUseElapsedTick[i] > 0 )
				{
					float fElapsedDelayTime = hSkill->GetDelayTime() - ( pPacket->Info.nSKillUseElapsedTick[i] / 1000.0f );
					if( fElapsedDelayTime >= 0.0f )
						hSkill->SetElapsedDelayTime( hSkill->GetDelayTime() - ( pPacket->Info.nSKillUseElapsedTick[i] / 1000.0f ) );
				}
			}
		}
	}

	CDnSkillTreeDlg* pSkillTreeDlg = static_cast<CDnSkillTreeDlg*>( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG ) );
	if( pSkillTreeDlg )
	{
		if( pSkillTreeDlg->IsShow() )
		{
			pSkillTreeDlg->UpdateView();
			pSkillTreeDlg->UpdateGuildWarSkillTreeContent();
		}
	}
}

void CDnSkillTask::OnRecvGuildSkillSwapIndex( SCPvPSwapSkillIndex* pPacket )
{
	_ASSERT( pPacket );

	if( pPacket->nRetCode == ERROR_NONE )
	{
		CDnMainDlg* pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
		if( pMainDlg )
		{
			CDnGuildWarSkillSlotDlg* pGuildWarSkillSlotDlg = pMainDlg->GetGuildWarSkillSlotDialog();
			if( pGuildWarSkillSlotDlg->IsShow() )
				pGuildWarSkillSlotDlg->SwapQuickSlot( pPacket->cFrom, pPacket->cTo );
		}
	}
	else
	{
//		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}

void CDnSkillTask::AddSkill( DnSkillHandle hSkill )
{
	if (!hSkill) return;

	m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].push_back( hSkill );

	GetInterface().RefreshSkillDialog();
}

void CDnSkillTask::RemoveSkill( DnSkillHandle hSkill )
{
	if (!hSkill) return;

	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
	{
		if( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->GetClassID() == hSkill->GetClassID() ) 
		{
			SAFE_RELEASE_SPTR( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i] );
			m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].erase( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].begin() + i );
			break;
		}
	}

	GetInterface().RefreshSkillDialog();
}

void CDnSkillTask::ChangeSkillLevelUp( int nSkillID, int nOrigLevel , bool bCurrentPageOnly )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor ) return;

	DnSkillHandle hSkillToLevelUp;
	int iSkillListIndex = -1;

	
	int nChangingValue = 0; // Delta -> adjust skills on other pages by the same delta as well.
	
	for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
	{
		if( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->GetClassID() == nSkillID ) 
		{
			int nLevelUp = hLocalActor->GetSkillLevelUpValue(nSkillID);
			int nLevel = nOrigLevel;
			nLevel += nLevelUp;

			nChangingValue = nLevel - m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][ i ]->GetLevel();
		}
	}

	for(int nSkillPage = DualSkill::Type::Primary; nSkillPage< DualSkill::Type::MAX; nSkillPage++ )
	{
		if( bCurrentPageOnly && m_iSkillTreePage != nSkillPage)
			continue;

		for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[nSkillPage].size(); i++ ) 
		{
			if( m_vlhLocalPlayerOpenedSkillList[nSkillPage][i]->GetClassID() == nSkillID ) 
			{
				hSkillToLevelUp = m_vlhLocalPlayerOpenedSkillList[nSkillPage][ i ];
				iSkillListIndex = i;
				break;
			}
		}

		bool bValidIndex = (-1 != iSkillListIndex);
		_ASSERT( bValidIndex );
		if( bValidIndex && hSkillToLevelUp)
		{
			int iNowSkillLevel = 0;
			float fElapsedDelay = 0.f;

			fElapsedDelay = hSkillToLevelUp->GetElapsedDelayTime();
			
			int nLevelUp = hLocalActor->GetSkillLevelUpValue(nSkillID);
			int nLevel = hSkillToLevelUp->GetLevel() + nChangingValue;


			DnSkillHandle hLevelUppedSkill = CDnSkill::CreateSkill( hLocalActor, nSkillID, nLevel );


			//If level-up skill creation failed, cancel level-up...
			if (!hLevelUppedSkill && nLevelUp > 0)
			{
				OutputDebug("Failed to create skill (%d) level (%d).. Check skill level table!!!!!!\n", nSkillID, nLevel);
				_ASSERT( hLevelUppedSkill && "Skill level-up attempt failed.. Check level table" );

				//Revert level value back down to original
				nLevel -= nLevelUp;
				hLevelUppedSkill = CDnSkill::CreateSkill(hLocalActor, nSkillID, nLevel);

				//Reset level-up value
				nLevelUp = 0;
			}

			if (hLevelUppedSkill)
				hLevelUppedSkill->SetLevelUpValue(nLevelUp);

			for( int i = 0; i < MAX_SKILL_SLOT_COUNT; ++i )
			{
				if(nSkillPage == m_iSkillTreePage ) // Set only the skills on the currently active page.
				{
					if( m_mapSkillTreeInfoByJobID[ hLevelUppedSkill->GetNeedJobClassID() ].aSkillTreeSlotInfo[ i ].hSkill == m_vlhLocalPlayerOpenedSkillList[nSkillPage].at(iSkillListIndex) )
					{
						m_mapSkillTreeInfoByJobID[ hLevelUppedSkill->GetNeedJobClassID() ].aSkillTreeSlotInfo[ i ].hSkill = hLevelUppedSkill;
						break;
					}
				}
			}

			m_vlhLocalPlayerOpenedSkillList[nSkillPage].at( iSkillListIndex ) = hLevelUppedSkill;

			hLevelUppedSkill->SetElapsedDelayTime( fElapsedDelay );
			hLevelUppedSkill->SelectLevelDataType( hSkillToLevelUp->GetSelectedLevelDataType() );


			if(nSkillPage == m_iSkillTreePage ) // Set only the skills on the currently active page.
			{
				bool bResult = hLocalActor->ReplacementSkill( hLevelUppedSkill );
				_ASSERT( bResult );
				CDnLocalPlayerActor * pLocalPlayerActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));
				pLocalPlayerActor->ReplacementGlyph( hLevelUppedSkill ); 
			}

			SAFE_RELEASE_SPTR( hSkillToLevelUp );

			GetInterface().RefreshSkillDialog();
		}
	}
}

bool CDnSkillTask::HasSameGlobalIDSkill( DnSkillHandle hSkill )
{
	bool bResult = false;

	if( 0 < hSkill->GetGlobalSkillGroupID() )
	{
		for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size(); i++ ) 
		{
			if( m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i]->GetGlobalSkillGroupID() == hSkill->GetGlobalSkillGroupID() )
			{
				bResult = true;
				break;
			}
		}

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		std::map< int, DnSkillHandle >::iterator Itor = m_mapReservationSkillHandle.begin();
		for( ; Itor != m_mapReservationSkillHandle.end(); ++Itor )
		{
			DnSkillHandle hReservationSkill = Itor->second;

			if( hReservationSkill->GetGlobalSkillGroupID() == hSkill->GetGlobalSkillGroupID() )
				return true;
		}
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	}

	return bResult;
}



DnSkillHandle CDnSkillTask::FindSkillBySkillPage( int nSkillID , int nSkillPage , bool bEntirePage )
{
	if(nSkillPage < DualSkill::Type::Primary || nSkillPage >= DualSkill::Type::MAX )
		return CDnSkill::Identity();

	for( int nPage = DualSkill::Type::Primary; nPage < DualSkill::Type::MAX; nPage++ )
	{
		if( !bEntirePage && nPage != nSkillPage )
			continue;

		for( DWORD i=0; i<m_vlhLocalPlayerOpenedSkillList[nPage].size(); i++ ) 
		{
			if( m_vlhLocalPlayerOpenedSkillList[nPage][i] && m_vlhLocalPlayerOpenedSkillList[nPage][i]->GetClassID() == nSkillID ) 
			{
				return m_vlhLocalPlayerOpenedSkillList[nPage][i];
			}
		}
	}

	return CDnSkill::Identity();
}

void CDnSkillTask::SendChangeSkillPage(int nSkillPage , bool bForce )
{
	if( (!bForce && m_iSkillTreePage == nSkillPage) || nSkillPage < DualSkill::Type::Primary || nSkillPage >= DualSkill::Type::MAX )
		return;

	m_bRequestWait = true;
	::SendChangeSkillPage(nSkillPage);
}

void CDnSkillTask::OnRecvSkillPageCount( SCSkillPageCount *pPacket )
{
	if(pPacket->cSkillPageCount > DualSkill::Type::MAX || pPacket->cSkillPageCount < DualSkill::Type::Primary )
		return;

	m_iSkillPageCount = pPacket->cSkillPageCount;

	CDnSkillTreeDlg* pSkillTreeDlg = static_cast<CDnSkillTreeDlg*>( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG ) );
	if( pSkillTreeDlg )
	{
		for(int i=0; i<pPacket->cSkillPageCount; i++)
			pSkillTreeDlg->EnableSkillPage(i);
	}
}

void CDnSkillTask::OnRecvChangeSkillPageRes(SCChangeSkillPage* pPacket)
{
	int nSkillPage = (int)pPacket->cSkillPage;
	m_bRequestWait = false;

	std::wstring szCaption;

	if( nSkillPage == DualSkill::Type::Primary )
		szCaption = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 742 ) , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 737 ) ) ;
	else
		szCaption = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 742 ) , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 738 ) ) ;

	GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2 , szCaption.c_str() , textcolor::YELLOW);

	if( CTaskManager::GetInstance().GetTask( "PVPLobbyTask" ) )
	{
		m_iNeedRefreshSkillPage = nSkillPage;
		return;
	}

	if( CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeEnum::MapTypeVillage )
		return;
	
	ApplySkillPage(nSkillPage);
}

void CDnSkillTask::ApplySkillPage(int nSkillPage )
{
	if( m_iSkillTreePage == nSkillPage || nSkillPage < DualSkill::Type::Primary || nSkillPage >= DualSkill::Type::MAX )
		return;

	if(!CDnActor::s_hLocalActor)
		return;

	m_iSkillTreePage = nSkillPage;

	CDnLocalPlayerActor* pLocalPlayerActor = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

	if(pLocalPlayerActor)
		pLocalPlayerActor->RemoveSkillAll(); // Removes all current skills.

	DnSkillHandle hSkill;
	int nSkillSize = (int)m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage].size();
	for(int i=0;i<nSkillSize;i++)
	{
		hSkill = m_vlhLocalPlayerOpenedSkillList[m_iSkillTreePage][i];
		
		if( hSkill )
		{
			int iSkillLevelDataType = QueryPVEOrPVP();
			hSkill->SetHasActor(CDnActor::s_hLocalActor);
			hSkill->SelectLevelDataType( iSkillLevelDataType );

			if ( pLocalPlayerActor && pLocalPlayerActor->CanAddSkill(hSkill->GetClassID()) )
			{
				pLocalPlayerActor->MASkillUser::AddSkill( hSkill );
			}
		}
	}

	if( pLocalPlayerActor && pLocalPlayerActor->IsSummonPet() )
	{
		if( pLocalPlayerActor->GetPetInfo().nSkillID1 > 0 )
			AddSkill( pLocalPlayerActor->GetPetInfo().nSkillID1, 1 );
		if( pLocalPlayerActor->GetPetInfo().nSkillID2 > 0 )
			AddSkill( pLocalPlayerActor->GetPetInfo().nSkillID2, 1 );
	}

	m_mapSkillTreeInfoByJobID.clear();  // Rebuild the skill tree info.
	InitializeSkillTree();              // Skill tree
	GetInterface().ChangeSkillPage(m_iSkillTreePage);
	GetInterface().RefreshSkillDialog();
}


#if defined(PRE_FIX_NEXTSKILLINFO)
SKILL_LEVEL_INFO CDnSkillTask::ms_SkillLevelTableInfo;
SKILL_LEVEL_INFO* CDnSkillTask::GetSkillLevelTableIDList(int nSkillID, int nApplyType)
{
	if (nApplyType >= 2 || nApplyType < 0)
		return &ms_SkillLevelTableInfo;

	SKILL_LEVEL_TABLEID_LIST::iterator findIter = m_SkillLevelTableIDList[nApplyType].find(nSkillID);
	if (findIter != m_SkillLevelTableIDList[nApplyType].end())
		return &findIter->second;

	return &ms_SkillLevelTableInfo;
}

void CDnSkillTask::InitSkillLevelTableIDList()
{
	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

	if (pSkillTable == NULL || pSkillLevelTable == NULL)
		return;

	int nSkillCount = pSkillTable->GetItemCount();
	for (int i = 0; i < nSkillCount; ++i)
	{
		int nSkillID = pSkillTable->GetItemID(i);

		//int nSkillID = pSkillTable->GetFieldFromLablePtr(nItemID, "id")->GetInteger();

		std::vector<int> vlSkillLevelList;
		pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", nSkillID, vlSkillLevelList );

		// Check and filter whether the target is PvE or PvP.
		vector<int>::iterator iterLevelList = vlSkillLevelList.begin();
		for( iterLevelList; iterLevelList != vlSkillLevelList.end(); ++iterLevelList)
		{
			int iSkillLevelTableID = *iterLevelList;
			int iApplyType = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_ApplyType" )->GetInteger();
			int iSkillLevel = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SkillLevel" )->GetInteger();

			AddSkillLevelTableID(nSkillID, iSkillLevel, iSkillLevelTableID, iApplyType);
		}
	}
}

void CDnSkillTask::AddSkillLevelTableID(int nSkillID, int nSkillLevel, int nSkillLevelTableID, int nApplyType)
{
	if (nApplyType >= 2 || nApplyType < 0)
		return;

	SKILL_LEVEL_TABLEID_LIST::iterator findIter = m_SkillLevelTableIDList[nApplyType].find(nSkillID);
	if (findIter != m_SkillLevelTableIDList[nApplyType].end())
	{
		//If an existing list exists, add to that list..
		findIter->second.AddSkillLevelTableID(nSkillLevel, nSkillLevelTableID);
	}
	else
	{
		SKILL_LEVEL_INFO skillLevelInfo;
		skillLevelInfo.AddSkillLevelTableID(nSkillLevel, nSkillLevelTableID);

		m_SkillLevelTableIDList[nApplyType].insert(std::make_pair(nSkillID, skillLevelInfo));
	}
}
#endif // PRE_FIX_NEXTSKILLINFO

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
bool CDnSkillTask::IsAvailableSPByJob(DnActorHandle hActor, vector<int>& needSPValues)
{
	//return m_SkillTreeSystem.IsAvailableSPByJob(hActor, needSPValues);
	if (!hActor)
		return false;

	CDnPlayerActor* pPlayerActor = NULL;

	if (hActor->IsPlayerActor())
		pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());

	if (pPlayerActor == NULL)
		return false;

	bool isAvailableSPByJob = true;

	if (pPlayerActor)
	{
		std::vector<int> jobHistory;
		pPlayerActor->GetJobHistory(jobHistory);
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
	}

	return isAvailableSPByJob;
}

bool CDnSkillTask::IsAvailableSPByJob(std::vector<int>& jobHistory, vector<int>& needSPValues, int nIndex)
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
	int iUsedSkillPointInThisJob = GetUsedSkillPointInThisJob( nJobID );

	//Usable only if the SP spent on the JobID is greater than the check value.
	if (iUsedSkillPointInThisJob >= nNeedSPValue)
		isAvailableSPByJob = true;

	return isAvailableSPByJob;
}

void CDnSkillTask::GetNeedSPValuesByJob(int nSkillID, std::vector<int>& nNeedSPValues)
{
	m_SkillTreeSystem.GetNeedSPValuesByJob(nSkillID, nNeedSPValues);
}
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP


#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
bool CDnSkillTask::IsTotalLevelSkill(int nSkillID)
{
	DNTableFileFormat*  pTotalLevelSkillDB = GetTableDB().GetTable( CDnTableDB::TTOTALLEVELSKILL );

	if (pTotalLevelSkillDB == NULL)
		return false;

	vector<int> vlTotalLevelSkills;
	pTotalLevelSkillDB->GetItemIDListFromField( "_SkillTableID", nSkillID, vlTotalLevelSkills );

	if (vlTotalLevelSkills.empty())
		return false;
	else
		return true;
}

bool CDnSkillTask::IsAvailableTotalSkill(DnActorHandle hActor, CDnSkill* pSkill)
{
	CDnTotalLevelSkillSystem* pTotalLevelSkillSystem = NULL;
	if (CDnActor::s_hLocalActor)
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		pTotalLevelSkillSystem = pPlayerActor ? pPlayerActor->GetTotalLevelSkillSystem() : NULL;
	}

	if (pTotalLevelSkillSystem == NULL || pSkill == NULL)
		return false;

	return pTotalLevelSkillSystem->IsUsableSkill(pSkill->GetMySmartPtr());
}

CDnTotalLevelSkillSystem* CDnSkillTask::GetTotalLevelSkillSystem()
{
	CDnTotalLevelSkillSystem* pTotalLevelSkillSystem = NULL;
	if (CDnActor::s_hLocalActor)
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		pTotalLevelSkillSystem = pPlayerActor ? pPlayerActor->GetTotalLevelSkillSystem() : NULL;
	}

	return pTotalLevelSkillSystem;
}

void CDnSkillTask::AddTotalLevelSkill(DnSkillHandle hSkill)
{
	CDnTotalLevelSkillSystem* pTotalLevelSkillSystem = NULL;
	CDnPlayerActor* pPlayerActor = NULL;
	if (CDnActor::s_hLocalActor)
	{
		pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		pTotalLevelSkillSystem = pPlayerActor ? pPlayerActor->GetTotalLevelSkillSystem() : NULL;
	}

	if (pTotalLevelSkillSystem == NULL || !hSkill)
		return;

	int nSkillID = hSkill->GetClassID();
	CDnTotalLevelSkillSystem::_TotalLevelSkillInfo* pSkillInfo = pTotalLevelSkillSystem->GetTotalLevelSkillInfo(nSkillID);
	
	//Check empty slot position.
	int nSlotIndex = pTotalLevelSkillSystem->FindEmptySlot(nSkillID);
	//int nSlotIndex = pTotalLevelSkillSystem->FindEmptySlot();
	
	if (nSlotIndex == -1)
	{
		//Message that no slot to add was found??
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption1, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1829 ), textcolor::YELLOW, 4.0f );
		return;
	}
	
	//Check if it can be added to an empty slot.
	int nResult = pTotalLevelSkillSystem->CanAddSkill(nSlotIndex, hSkill);
	switch(nResult)
	{
	case CDnTotalLevelSkillSystem::TotalLevelSystem_Error_None:
		{
			if (pPlayerActor)
				pPlayerActor->RequestAddTotalLevelSkill(nSlotIndex, hSkill->GetClassID());
		}
		break;
	case CDnTotalLevelSkillSystem::TotalLevelSystem_Error_SameSkillType:
		{
			//If a unified-level skill of the same type is already equipped...
			GetInterface().ShowCaptionDialog( CDnInterface::typeCaption1, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1830 ), textcolor::YELLOW, 4.0f );
		}
		break;
	}
	
}

void CDnSkillTask::RequestAddTotalLevelSkill(int nSlotIndex, int nSkillID)
{
	CDnPlayerActor* pPlayerActor = NULL;
	if (CDnActor::s_hLocalActor)
		pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	
	if (pPlayerActor)
		pPlayerActor->RequestAddTotalLevelSkill(nSlotIndex, nSkillID);
}

void CDnSkillTask::RequestRemoveTotalLevelSkill(int nSlotIndex)
{
	CDnPlayerActor* pPlayerActor = NULL;
	if (CDnActor::s_hLocalActor)
		pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

	if (pPlayerActor)
		pPlayerActor->RequestRemoveTotalLevelSkill(nSlotIndex);
}

int CDnSkillTask::GetTotalLevel()
{
	int nTotalLevel = 0;

	CDnPlayerActor* pPlayerActor = NULL;
	if (CDnActor::s_hLocalActor)
		pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

	if (pPlayerActor)
	{
		CDnTotalLevelSkillSystem* pTotalLevelSystem = pPlayerActor->GetTotalLevelSkillSystem();

		if (pTotalLevelSystem)
			nTotalLevel = pTotalLevelSystem->GetTotalLevel();
	}

	return nTotalLevel;
}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
void CDnSkillTask::OnRecvReservationSkillList( const SCReservationSkillListAck * pData )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor )
		return;

	for( int itr = 0; itr < pData->nCount; ++itr )
	{
		if( ERROR_NONE != pData->tReservationSkillAck[itr].nResult )
		{
			ErrorMessage_ReservationSkill( pData->tReservationSkillAck[itr].nResult );
			break;
		}

		const TReservationSkillReq & sData = pData->tReservationSkillAck[itr].tReservationSkill;

		if( ReservationSKillList::Type::UnLock == pData->tReservationSkillAck[itr].tReservationSkill.cType )
		{
			CDnSkillTreeSystem::S_OUTPUT Output;
			CDnSkillTreeSystem::S_TRY_UNLOCK TryUnlockInfo;
			TryUnlockInfo.iTryUnlockSkillID = sData.nSkillID;
			TryUnlockInfo.iCurrentCharLevel = sData.nLevel;

			m_SkillTreeSystem.TryUnLockSkill( TryUnlockInfo, &Output );

			if( CDnSkillTreeSystem::R_SUCCESS != Output.eResult )
				continue;

			Unlock_ReservationSkill( sData.nSkillID );
			Acquire_ReservationSkill( sData.nSkillID, false );
		}
		else if( ReservationSKillList::Type::Acquire == pData->tReservationSkillAck[itr].tReservationSkill.cType )
			Acquire_ReservationSkill( sData.nSkillID, false );
		else if( ReservationSKillList::Type::LevelUp == pData->tReservationSkillAck[itr].tReservationSkill.cType )
		{
			const int nLevelUp = hLocalActor->GetSkillLevelUpValue( sData.nSkillID );
			DnSkillHandle hSkill = FindSkill( sData.nSkillID );

			if( CDnSkill::Identity() == hSkill )
			{
				hSkill = GetReservationSkill( sData.nSkillID );
				if( CDnSkill::Identity() == hSkill )
					continue;
			}

			for( int itr = hSkill->GetLevel() + 1 - nLevelUp; itr <= sData.nLevel; ++itr )
				LevelUpSkill( sData.nSkillID, sData.nLevel, false );
		}
	}

	GetInterface().RefreshSkillDialog();
}

void CDnSkillTask::SendReservationSkillList()
{
	::SendReservationSkillList( m_vecReservationSkillData );

	int nNum = 0;
	std::map< int, DnSkillHandle >::iterator Itor = m_mapReservationGuildSkillHandle.begin();
	for( ; Itor != m_mapReservationGuildSkillHandle.end(); ++Itor )
	{
		SendAcquireGuildWarSkillReq( Itor->second->GetClassID(), Itor->second->GetLevel(), nNum );
		++nNum;
	}
}

int CDnSkillTask::GetUsedReservationSkillPointInThisJob( const int iJobID )
{
	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor )
		return 0;

	int iWholeUsedSPInThisJob = 0;

	for( DWORD itr = 0; itr < m_vecReservationSkillData.size(); ++itr )
	{
		if( ReservationSKillList::Type::LevelUp == m_vecReservationSkillData[itr].cType )
			continue;

		DnSkillHandle hSkill = GetReservationSkill( m_vecReservationSkillData[itr].nSkillID );

		if( !hSkill )
			continue;

		if( hSkill->IsAcquired()
			&& hSkill->GetNeedJobClassID() == iJobID )
		{
			int nItemLevel = hLocalActor->GetSkillLevelUpValue( hSkill->GetClassID() );

			int iSkillLevel = hSkill->GetLevel() - nItemLevel;
			iSkillLevel -= hSkill->GetLevelUpValue();
			iWholeUsedSPInThisJob += _GetAccumlatedNeedSkillPoint( hSkill, iSkillLevel );
		}
	}

	return iWholeUsedSPInThisJob;	
}

void CDnSkillTask::GetPossessReservationSkill( vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO> & vlPossessedSkillInfos )
{
	for( DWORD itr = 0; itr < m_vecReservationSkillData.size(); ++itr )
	{
		if( ReservationSKillList::Type::LevelUp == m_vecReservationSkillData[itr].cType )
			continue;

		CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;

		DnSkillHandle hSkill = GetReservationSkill( m_vecReservationSkillData[itr].nSkillID );

		if( !hSkill )
			continue;

		SkillInfo.iSkillID = hSkill->GetClassID();
		SkillInfo.iSkillLevel = hSkill->GetLevel();

		//#37528 Ignore bonus levels from skill level-up items when checking prerequisite skill level-up conditions.
		//Deduct by the skill level-up amount.
		SkillInfo.iSkillLevel -= hSkill->GetLevelUpValue();

		SkillInfo.bCurrentLock = false;

		vlPossessedSkillInfos.push_back( SkillInfo );
	}
}

bool CDnSkillTask::IsJobSP_SkillReservation( const int nJobID )
{
	std::map< int, DnSkillHandle >::iterator Itor = m_mapReservationSkillHandle.begin();
	for( ; Itor != m_mapReservationSkillHandle.end(); ++Itor )
	{
		DnSkillHandle hSkill = Itor->second;
		if( hSkill && hSkill->GetNeedJobClassID() == nJobID )
			return true;
	}
	return false;
}

bool CDnSkillTask::IsClear_SkillLevelUp()
{
	return m_vecReservationSkillData.empty() && m_mapReservationSkillHandle.empty() && m_mapReservationGuildSkillHandle.empty();
}

void CDnSkillTask::Clear_SkillLevelUp()
{
	m_vecReservationSkillData.clear();

	std::map< int, DnSkillHandle >::iterator Itor = m_mapReservationSkillHandle.begin();
	for( ; Itor != m_mapReservationSkillHandle.end(); ++Itor )
		SAFE_RELEASE_SPTR( Itor->second );
	m_mapReservationSkillHandle.clear();

	Itor = m_mapReservationGuildSkillHandle.begin();
	for( ; Itor != m_mapReservationGuildSkillHandle.end(); ++Itor )
		SAFE_RELEASE_SPTR( Itor->second );
	m_mapReservationGuildSkillHandle.clear();

	m_nReservationSkillNeedSP = 0;
	m_nReservationSkillNeedMoney = 0;
}

int CDnSkillTask::GetReservationSkillNeedSP()
{
	return m_nReservationSkillNeedSP;
}

INT64 CDnSkillTask::GetReservationSkillNeedMoney()
{
	return m_nReservationSkillNeedMoney;
}

DnSkillHandle CDnSkillTask::GetReservationSkill( const int nSkillID )
{
	if( m_mapReservationSkillHandle.empty() == false )
	{
		std::map< int, DnSkillHandle >::iterator Itor = m_mapReservationSkillHandle.find( nSkillID );
		if( m_mapReservationSkillHandle.end() != Itor )
			return Itor->second;
	}

	return CDnSkill::Identity();
}

void CDnSkillTask::GetReservationSkillLevelMax( SReservationSkillMax & sData )
{
	if( !CDnActor::s_hLocalActor )
		return;

	CDnLocalPlayerActor * pLocalActor = (CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer());
	if( !pLocalActor )
		return;
	
	DnSkillHandle hSkill = GetReservationSkill( sData.m_nSkillID );
	if( !hSkill )
		hSkill = FindSkill( sData.m_nSkillID );

	bool bNeedAcquire = false;
	bool bNeedUnlock = false;
	if( !hSkill )
	{
		bNeedAcquire = true;
		hSkill = FindZeroLevelSkill( sData.m_nSkillID );

		if( !hSkill )
		{
			bNeedUnlock = true;
			hSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, sData.m_nSkillID, 1 );
		}
	}

	const int nJobID = hSkill->GetNeedJobClassID();
	const int nJobIndex = pLocalActor->GetJobArrayIndex( nJobID );

	const int nJobRemainSkillPoint = pLocalActor->GetSPUsingLimitByJobArrayIndex( nJobIndex ) - GetUsedSkillPointInThisJob( nJobID );
	const int nTotalRemainSkillPoint = GetSkillPoint() - GetReservationSkillNeedSP();
	const int nPlayerLevel = pLocalActor->GetLevel();

	int nTotalNeedSkillPoint = 0;
	if( true == bNeedAcquire )
	{
		const int nNeedAcquireSkillPoint = hSkill->GetNeedAcquireSkillPoint();

		if( nJobRemainSkillPoint < nNeedAcquireSkillPoint 
			|| nTotalRemainSkillPoint < nNeedAcquireSkillPoint 
			|| nPlayerLevel < hSkill->GetLevelLimit() )
		{
			if( bNeedUnlock )
				SAFE_RELEASE_SPTR( hSkill );
			
			return;
		}

		nTotalNeedSkillPoint += nNeedAcquireSkillPoint;
	}

	DNTableFileFormat * pSkillLevelTable = GetTableDB().GetTable( CDnTableDB::TSKILLLEVEL );
	SKILL_LEVEL_INFO * pLevelTableInfo = GetSkillLevelTableIDList( hSkill->GetClassID(), hSkill->GetSelectedLevelDataType());
	SKILL_LEVEL_TABLE_IDS::const_iterator findIter = pLevelTableInfo->_SkillLevelTableIDs.find( hSkill->GetLevel() );

	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	int nSkillMaxLevel = pSkillTable->GetFieldFromLablePtr( hSkill->GetClassID(), "_MaxLevel" )->GetInteger();

	const int nItemLevelUp = pLocalActor->GetSkillLevelUpValue( sData.m_nSkillID );
	int nNowSkillLevel = hSkill->GetLevel();

	if( true == bNeedAcquire )
		nNowSkillLevel += nItemLevelUp;

#if defined( PRE_MOD_68531_NEW )
	nNowSkillLevel -= nItemLevelUp;
#endif	// #if defined( PRE_MOD_68531_NEW )

	const int nNeedLevelUpSkillPoint = hSkill->GetNextLevelSkillPoint();
	int nLevelUpCount = 0, nNextSkillLevelTableID = -1;
	bool bEnable = false;
	while( nSkillMaxLevel >= nNowSkillLevel + nLevelUpCount )
	{
		if( nLevelUpCount > 100 )
			break;

		findIter = pLevelTableInfo->_SkillLevelTableIDs.find( nNowSkillLevel + nLevelUpCount );
		if( findIter != pLevelTableInfo->_SkillLevelTableIDs.end() )
			nNextSkillLevelTableID = findIter->second;

		const int nLevelLimit = pSkillLevelTable->GetFieldFromLablePtr( nNextSkillLevelTableID, "_LevelLimit" )->GetInteger();
		const int nNeedSkillPoint = nTotalNeedSkillPoint + (nLevelUpCount * nNeedLevelUpSkillPoint);

		if( nJobRemainSkillPoint < nNeedSkillPoint 
			|| nTotalRemainSkillPoint < nNeedSkillPoint
			|| nPlayerLevel < nLevelLimit )
			break;

		++nLevelUpCount;
	}

	if( 0 != nLevelUpCount || true == bNeedAcquire )
		--nLevelUpCount;

	if( true == bNeedUnlock )
		sData.m_nNeedSkillMax = eNeed_SkillMax_Unlock;
	else if( true == bNeedAcquire )
		sData.m_nNeedSkillMax = eNeed_SkillMax_Acquire;

	if( 0 != nLevelUpCount )
		sData.m_nNeedSkillMax += eNeed_SkillMax_LevelUp;

	sData.m_nLevelUpCount = nLevelUpCount;

#if defined( PRE_MOD_68531_NEW )
	sData.m_nNowSkillLevel = nNowSkillLevel;
#else
	sData.m_nNowSkillLevel = nNowSkillLevel - nItemLevelUp;
#endif	// #if defined( PRE_MOD_68531_NEW )

	if( bNeedUnlock )
		SAFE_RELEASE_SPTR( hSkill );
}

void CDnSkillTask::Change_SkillLevelUpMax( const int nSkillID )
{
	SReservationSkillMax sData;
	sData.m_nSkillID = nSkillID;

	GetReservationSkillLevelMax( sData );

	if( sData.m_nNeedSkillMax & eNeed_SkillMax_Unlock )
		Change_SkillLevelUp( nSkillID, 1, ReservationSKillList::Type::UnLock );
	else if( sData.m_nNeedSkillMax & eNeed_SkillMax_Acquire )
		Change_SkillLevelUp( nSkillID, 1, ReservationSKillList::Type::Acquire );

	if( sData.m_nNeedSkillMax & eNeed_SkillMax_LevelUp )
	{
		for( int itr = 1; itr < sData.m_nLevelUpCount; ++itr )
			Change_SkillLevelUp( nSkillID, sData.m_nNowSkillLevel + itr, ReservationSKillList::Type::LevelUp );

		Change_SkillLevelUp( nSkillID, sData.m_nNowSkillLevel + sData.m_nLevelUpCount, ReservationSKillList::Type::LevelUp );
	}

	SetReservationSkill_Item( nSkillID );
}

void CDnSkillTask::Change_SkillLevelUp( const int nSkillID, const int nSkillLevel, const char cState )
{
	TReservationSkillReq sReservationSkillData;
	sReservationSkillData.nSkillID = nSkillID;
	sReservationSkillData.cType = cState;

	DnSkillHandle hLevelUpSkill;
	if( ReservationSKillList::Type::UnLock == cState
		|| ReservationSKillList::Type::Acquire == cState )
	{
		sReservationSkillData.nLevel = nSkillLevel;
		hLevelUpSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, nSkillID, sReservationSkillData.nLevel );
		m_nReservationSkillNeedSP += hLevelUpSkill->GetNeedAcquireSkillPoint();

		if( ReservationSKillList::Type::UnLock == cState )
			m_nReservationSkillNeedMoney += hLevelUpSkill->GetUnlockPrice();
	}
	else if( ReservationSKillList::Type::LevelUp == cState )
	{
		sReservationSkillData.nLevel = nSkillLevel;
		hLevelUpSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, nSkillID, sReservationSkillData.nLevel );
		if( CDnSkill::Identity() == hLevelUpSkill )
			return;

		DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );
		const int nLevelTableID = hLevelUpSkill->GetSkillLevelID();
		const int nNextLevelUpSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( nLevelTableID, "_NeedSkillPoint" )->GetInteger();

		if( hLevelUpSkill )
			m_nReservationSkillNeedSP += nNextLevelUpSkillPoint;
	}

	if( m_mapReservationSkillHandle.empty() == false )
	{
		std::map< int, DnSkillHandle>::iterator Handle_Itor = m_mapReservationSkillHandle.find( nSkillID );
		if( m_mapReservationSkillHandle.end() != Handle_Itor )
		{
			SAFE_RELEASE_SPTR( Handle_Itor->second );
			m_mapReservationSkillHandle.erase( Handle_Itor );
		}
	}

	m_vecReservationSkillData.push_back( sReservationSkillData );

	m_mapReservationSkillHandle.insert( std::pair< int, DnSkillHandle >( nSkillID, hLevelUpSkill ) );
}

bool CDnSkillTask::IsGlobalSkillGroupDown( DnSkillHandle hSkill )
{
	bool bParentSkill = false;

	std::vector< std::pair<int, int> > vecParentSkillData;
	m_SkillTreeSystem.GetParentSkillData( hSkill->GetClassID(), vecParentSkillData );

	std::vector< std::pair<int, int> >::iterator parent_Itor = vecParentSkillData.begin();
	for( ; parent_Itor != vecParentSkillData.end(); ++parent_Itor )
	{
		bParentSkill = false;
		std::vector< TReservationSkillReq >::iterator reserve_Itor = m_vecReservationSkillData.begin();
		for( ; reserve_Itor != m_vecReservationSkillData.end(); ++reserve_Itor )
		{
			if( parent_Itor->first != reserve_Itor->nSkillID )
				continue;

			std::map< int, DnSkillHandle >::iterator skill_Itor = m_mapReservationSkillHandle.find( reserve_Itor->nSkillID );
			if( skill_Itor != m_mapReservationSkillHandle.end() && skill_Itor->second->GetLevel() >= parent_Itor->second )
				bParentSkill = true;
		}

		if( false == bParentSkill )
			break;
	}

	return bParentSkill;
}

int CDnSkillTask::IsReservationSkillDown( const int nSkillID )
{
	if( m_mapReservationSkillHandle.empty() == true )
		return eSkillMinus_Delete;

	std::map< int, DnSkillHandle>::iterator Handle_Itor = m_mapReservationSkillHandle.find( nSkillID );
	if( m_mapReservationSkillHandle.end() == Handle_Itor )
		return eSkillMinus_Delete;

	DnSkillHandle hEraseSkill = Handle_Itor->second;

	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	int nMaxLevel = pSkillTable->GetFieldFromLablePtr( hEraseSkill->GetClassID(), "_MaxLevel" )->GetInteger();

	int nNeedSkillPoint = 0;
	if( hEraseSkill->GetLevel() == 1 )
		nNeedSkillPoint = hEraseSkill->GetNeedAcquireSkillPoint();
	else if( hEraseSkill->GetLevel() == nMaxLevel )
	{
		DNTableFileFormat * pSkillLevelTable = GetTableDB().GetTable( CDnTableDB::TSKILLLEVEL );
		SKILL_LEVEL_INFO * pLevelTableInfo = GetSkillLevelTableIDList( hEraseSkill->GetClassID(), hEraseSkill->GetSelectedLevelDataType());
		SKILL_LEVEL_TABLE_IDS::const_iterator findIter = pLevelTableInfo->_SkillLevelTableIDs.find( hEraseSkill->GetLevel() );

		findIter = pLevelTableInfo->_SkillLevelTableIDs.find( hEraseSkill->GetLevel() - 1 );
		if( findIter != pLevelTableInfo->_SkillLevelTableIDs.end() )
		{
			int nNextSkillLevelTableID = findIter->second;

			nNeedSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( nNextSkillLevelTableID, "_NeedSkillPoint" )->GetInteger();
		}
	}
	else
		nNeedSkillPoint = hEraseSkill->GetNextLevelSkillPoint();

	if( !CDnActor::s_hLocalActor )
		return eSkillMinus_Disable;
	CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	if( NULL == pPlayerActor )
		return eSkillMinus_Disable;

	vector<int> vlJobHistory;
	vector<int> vecJobUsedSkillPoint;
	pPlayerActor->GetJobHistory( vlJobHistory );
	for( DWORD itr = 0; itr < vlJobHistory.size(); ++itr )
	{
		const int nJobID = vlJobHistory[itr];
		int nUsedSkillPoint = GetUsedSkillPointInThisJob( nJobID );

		if( hEraseSkill->GetNeedJobClassID() == nJobID )
			nUsedSkillPoint -= nNeedSkillPoint;

		vecJobUsedSkillPoint.push_back( nUsedSkillPoint );
	}

	std::map< int, DnSkillHandle >::iterator Itor = m_mapReservationSkillHandle.begin();
	for( ; Itor != m_mapReservationSkillHandle.end(); ++Itor )
	{
		DnSkillHandle hSkill = Itor->second;
		if( !hSkill || hSkill->GetClassID() == nSkillID )
			continue;

		if( 0 < hSkill->GetGlobalSkillGroupID() 
			&& hSkill->GetGlobalSkillGroupID() == hEraseSkill->GetGlobalSkillGroupID() 
			&& hSkill->GetClassID() != hEraseSkill->GetClassID() )
		{
			if( true == IsGlobalSkillGroupDown( hEraseSkill ) && false == IsGlobalSkillGroupDown( hSkill ) && hEraseSkill->GetLevel() == 1 )
				return eSkillMinus_Disable;
		}

		std::vector<int> nNeedSPValues;
		m_SkillTreeSystem.GetNeedSPValuesByJob( hSkill->GetClassID(), nNeedSPValues );

		for( DWORD itr = 0 ; itr < nNeedSPValues.size(); ++itr )
		{
			if( vecJobUsedSkillPoint.size() <= itr )
				break;

			if( vecJobUsedSkillPoint[itr] < nNeedSPValues[itr] )
				return eSkillMinus_Disable;
		}
	}

	for( DWORD itr = 0; itr < m_vecReservationSkillData.size(); ++itr )
	{
		if( ReservationSKillList::Type::UnLock == m_vecReservationSkillData[itr].cType 
			|| ReservationSKillList::Type::Acquire == m_vecReservationSkillData[itr].cType )
		{
			const int nPrentLevel = m_SkillTreeSystem.IsParentSkill( nSkillID, m_vecReservationSkillData[itr].nSkillID );
			const int nLevel = hEraseSkill->GetLevel() - pPlayerActor->GetSkillLevelUpValue( nSkillID );

			if( 0 != nPrentLevel && nLevel <= nPrentLevel )
				return eSkillMinus_Disable;
		}
	}

	return eSkillMinus_Enable;
}

void CDnSkillTask::Change_SkillLevelDownMax( const int nSkillID )
{
	for( DWORD itr = 0; itr < 100; ++itr )
	{
		if( eSkillMinus_Enable != IsReservationSkillDown( nSkillID ) )
			return;

		DnSkillHandle hSkill = GetReservationSkill(  nSkillID );
		if( !hSkill )
			break;

		Change_SkillLevelDown( nSkillID, hSkill->GetLevel() );
	}

	SetReservationSkill_Item( nSkillID );
}

void CDnSkillTask::Change_SkillLevelDown( const int nSkillID, const int nSkillLevel )
{
	if( m_mapReservationSkillHandle.empty() == true )
		return;

	std::map< int, DnSkillHandle>::iterator Handle_Itor = m_mapReservationSkillHandle.find( nSkillID );
	if( m_mapReservationSkillHandle.end() == Handle_Itor )
		return;

	DnSkillHandle hReservationSkill = Handle_Itor->second;

	std::vector< TReservationSkillReq >::reverse_iterator Itor = m_vecReservationSkillData.rbegin();
	for( ; Itor != m_vecReservationSkillData.rend(); ++Itor )
	{
		if( nSkillID != Itor->nSkillID )
			continue;
		
		const int nNeedAcquireSkillPoint = hReservationSkill->GetNeedAcquireSkillPoint();
		const int nUnlockPrice = hReservationSkill->GetUnlockPrice();
		const int nReservationSkillLevel = hReservationSkill->GetLevel();

		DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );
		const int nLevelTableID = hReservationSkill->GetSkillLevelID();
		const int nNextLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( nLevelTableID, "_NeedSkillPoint" )->GetInteger();
		
		SAFE_RELEASE_SPTR( Handle_Itor->second );
		m_mapReservationSkillHandle.erase( Handle_Itor );

		if( ReservationSKillList::Type::UnLock == Itor->cType
			|| ReservationSKillList::Type::Acquire == Itor->cType )
		{
			m_nReservationSkillNeedSP -= nNeedAcquireSkillPoint;

			if( ReservationSKillList::Type::UnLock == Itor->cType )
				m_nReservationSkillNeedMoney -= nUnlockPrice;
		}
		else if( ReservationSKillList::Type::LevelUp == Itor->cType )
		{
			m_nReservationSkillNeedSP -= nNextLevelSkillPoint;

			DnSkillHandle hLearnSkill = FindSkill( nSkillID );

			if( !hLearnSkill || hLearnSkill->GetLevel() != nReservationSkillLevel - 1 )
			{
				DnSkillHandle hLevelDownSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, nSkillID, nReservationSkillLevel - 1 );
				m_mapReservationSkillHandle.insert( std::pair< int, DnSkillHandle >( nSkillID, hLevelDownSkill ) );
			}
		}

		m_vecReservationSkillData.erase( (++Itor).base() );

		return;
	}
}

void CDnSkillTask::SetReservationSkill_Item( const int nSkillID )
{
	if( m_mapReservationSkillHandle.empty() == true )
		return;

	std::map< int, DnSkillHandle>::iterator Handle_Itor = m_mapReservationSkillHandle.find( nSkillID );
	if( Handle_Itor == m_mapReservationSkillHandle.end() )
		return;

	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor )
		return;

	const int nLevelUp = hLocalActor->GetSkillLevelUpValue( nSkillID );
	if( 0 == nLevelUp )
		return;

	int nLevel = 0;
	std::vector< TReservationSkillReq >::reverse_iterator Itor = m_vecReservationSkillData.rbegin();
	for( ; Itor != m_vecReservationSkillData.rend(); ++Itor )
	{
		if( Itor->nSkillID == nSkillID )
		{
			nLevel = Itor->nLevel;
			break;
		}
	}

	if( 0 == nLevel )
		return;

	DnSkillHandle hLevelUpSkill = CDnSkill::CreateSkill( CDnActor::s_hLocalActor, nSkillID, nLevel + nLevelUp );
	if( !hLevelUpSkill )
		return;

	SAFE_RELEASE_SPTR( Handle_Itor->second );
	m_mapReservationSkillHandle.erase( Handle_Itor );

	
	m_mapReservationSkillHandle.insert( std::pair< int, DnSkillHandle >( nSkillID, hLevelUpSkill ) );
}

DnSkillHandle CDnSkillTask::GetReservationGuildSkill( const int nSkillID )
{
	std::map< int, DnSkillHandle >::iterator Itor = m_mapReservationGuildSkillHandle.find( nSkillID );
	if( m_mapReservationGuildSkillHandle.end() != Itor )
		return Itor->second;

	return CDnSkill::Identity();
}

void CDnSkillTask::Change_GuildSkillLevelUp( const int nSkillID )
{
	std::map< int, DnSkillHandle >::iterator Itor = m_mapReservationGuildSkillHandle.find( nSkillID );
	if( m_mapReservationGuildSkillHandle.end() != Itor )
	{
		SAFE_RELEASE_SPTR( Itor->second );
		m_mapReservationGuildSkillHandle.erase( Itor );
	}

	DnSkillHandle hSkill = CDnGuildWarSkill::CreateSkill( CDnActor::s_hLocalActor, nSkillID, 1 );
	if( CDnSkill::Identity() == hSkill )
		return;

	m_mapReservationGuildSkillHandle.insert( pair< int, DnSkillHandle >( nSkillID, hSkill ) );
}

void CDnSkillTask::Change_GuildSkillLevelDown( const int nSkillID )
{
	std::map< int, DnSkillHandle >::iterator Itor = m_mapReservationGuildSkillHandle.find( nSkillID );
	if( m_mapReservationGuildSkillHandle.end() != Itor )
	{
		SAFE_RELEASE_SPTR( Itor->second );
		m_mapReservationGuildSkillHandle.erase( Itor );
	}
}

bool CDnSkillTask::IsReservationGuildSkillDown( const int nSkillID )
{
	if( CDnSkill::Identity() == GetReservationGuildSkill( nSkillID ) )
		return false;

	return true;
}

int CDnSkillTask::GetReservationGuildSkillCount()
{
	return (int)m_mapReservationGuildSkillHandle.size();
}
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

#if defined( PRE_ADD_PRESET_SKILLTREE )
void CDnSkillTask::SendPresetSkillTreeList()
{
	::SendSkillSetListReq();
}

void CDnSkillTask::SendPresetSkillTreeSave( std::wstring wszName )
{
	m_SavePresetSkillTree.Clear();

	m_SavePresetSkillTree.m_cIndex = GetEmptyEnablePresetIndex();
	if( -1 == m_SavePresetSkillTree.m_cIndex )
		return;

	m_SavePresetSkillTree.m_wszTreeName = wszName;

	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	if( NULL == pPlayerActor )	return;

	vector<int> vlJobHistory;
	pPlayerActor->GetJobHistory( vlJobHistory );

	for( DWORD itr = 0; itr < vlJobHistory.size(); ++itr )
	{
		int nJobID = vlJobHistory[itr];

		int nSlot = 0;

		// Handle exception when tree structure is inverted.
		// Otherwise we would have to search all skill trees, which is too inefficient.
		if( 41 == nJobID )	// Guardian
		{
			SavePresetSkill( nJobID, 1 );
			SavePresetSkill( nJobID, 0 );
			nSlot = 2;
		}

		for( ; nSlot < MAX_SKILL_SLOT_COUNT; ++nSlot )
			SavePresetSkill( nJobID, nSlot );
	}

	SendSkillSetSave( m_SavePresetSkillTree.m_cIndex, m_SavePresetSkillTree.m_wszTreeName.c_str(), m_SavePresetSkillTree.m_Data );
}

void CDnSkillTask::SendPresetSkillTreeDel( const BYTE cIndex )
{
	m_nPresetSkillDeleteIndex = cIndex;

	SendSKillSetDelete(cIndex);
}

void CDnSkillTask::SavePresetSkill( const int nJobID, const int nSlot )
{
	S_SKILLTREE_SLOT_INFO & sSlotInfo = m_mapSkillTreeInfoByJobID[ nJobID ].aSkillTreeSlotInfo[ nSlot ];

	if( CDnSkill::Identity() == sSlotInfo.hSkill )
		return;

	int nSkillID = sSlotInfo.hSkill->GetClassID();
	BYTE cLevel = sSlotInfo.hSkill->GetLevel();

	DnSkillHandle hReservationSkill = GetReservationSkill( nSkillID );
	if( ( true == sSlotInfo.hSkill->IsLocked() || false == sSlotInfo.hSkill->IsAcquired() )
		&& CDnSkill::Identity() == hReservationSkill )
		return;

	if( hReservationSkill )
		cLevel = hReservationSkill->GetLevel();

	DnActorHandle hLocalActor = CDnActor::s_hLocalActor;
	if( !hLocalActor )
		return;

	const int nLevelUp = hLocalActor->GetSkillLevelUpValue( nSkillID );

	cLevel -= nLevelUp;

	if( 0 >= cLevel )
		return;

	m_SavePresetSkillTree.m_Data.push_back( std::pair<int, BYTE>( nSkillID, cLevel ) );
}

bool CDnSkillTask::IsMySkillTreeSkill( const vector<int> & vlJobHistory, const int nSkillID )
{
	for( DWORD itr = 0; itr < vlJobHistory.size(); ++itr )
	{
		int nJobID = vlJobHistory[itr];

		for( int nSlot = 0; nSlot < MAX_SKILL_SLOT_COUNT; ++nSlot )
		{
			S_SKILLTREE_SLOT_INFO & sSlotInfo = m_mapSkillTreeInfoByJobID[ nJobID ].aSkillTreeSlotInfo[ nSlot ];
			if( CDnSkill::Identity() == sSlotInfo.hSkill )
				continue;

			if( sSlotInfo.hSkill->GetClassID() == nSkillID )
				return true;
		}
	}

	return false;
}

void CDnSkillTask::AdviceSkillTreeSort( std::vector< std::pair<int, BYTE> > & inData, std::vector< std::pair<int, BYTE> > & outData  )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	if( NULL == pPlayerActor )	return;

	vector<int> vlJobHistory;
	pPlayerActor->GetJobHistory( vlJobHistory );

	for( DWORD itr = 0; itr < vlJobHistory.size(); ++itr )
	{
		int nJobID = vlJobHistory[itr];

		int nSlot = 0;

		// Handle exception when tree structure is inverted.
		// Otherwise we would have to search all skill trees, which is too inefficient.
		if( 41 == nJobID )	// Guardian
		{
			AdviceSkillTreePush( inData, outData, nJobID, 1 );
			AdviceSkillTreePush( inData, outData, nJobID, 0 );
			nSlot = 2;
		}

		for( ; nSlot < MAX_SKILL_SLOT_COUNT; ++nSlot )
			AdviceSkillTreePush( inData, outData, nJobID, nSlot );
	}
}

void CDnSkillTask::AdviceSkillTreePush( std::vector< std::pair<int, BYTE> > & inData, std::vector< std::pair<int, BYTE> > & outData, const int nJobID, const int nSlot )
{
	S_SKILLTREE_SLOT_INFO & sSlotInfo = m_mapSkillTreeInfoByJobID[ nJobID ].aSkillTreeSlotInfo[ nSlot ];

	if( CDnSkill::Identity() == sSlotInfo.hSkill )
		return;

	int nSkillID = sSlotInfo.hSkill->GetClassID();

	for( DWORD itr = 0; itr < inData.size(); ++itr )
	{
		if( nSkillID == inData[itr].first )
		{
			outData.push_back( inData[itr] );
			break;
		}
	}
}

void CDnSkillTask::InitAdviceSkillTreeTable()
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	if( NULL == pPlayerActor )	return;

	vector<int> vlJobHistory;
	pPlayerActor->GetJobHistory( vlJobHistory );

	DNTableFileFormat * pSkillTreeTable = GetDNTable( CDnTableDB::TSKILLTREE );
	DNTableFileFormat * pSkillAdviceTable = GetDNTable( CDnTableDB::TSKILLTREE_ADVICE );

	if( NULL == pSkillTreeTable || NULL == pSkillAdviceTable )
		return;

	const int nAdviceTableCount = pSkillAdviceTable->GetItemCount();

	for( int itr = 0; itr < nAdviceTableCount; ++itr )
	{
		int nItemID = pSkillAdviceTable->GetItemID( itr );

		SPresetSkillTree sData;

		int nAdviceIndex = pSkillAdviceTable->GetFieldFromLablePtr(nItemID, "_AdviceSkillIndex")->GetInteger();
		int nStringID = pSkillAdviceTable->GetFieldFromLablePtr(nItemID, "_NameID")->GetInteger();

		sData.m_nLevelLimit = pSkillAdviceTable->GetFieldFromLablePtr(nItemID, "_SkillTreeLevel")->GetInteger();
		sData.m_wszTreeName = std::wstring( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringID) );
		sData.m_bAdvice = true;

		std::vector< std::pair<int, BYTE> > vecData;
		m_SkillTreeSystem.GetAdviceSkillTree( nAdviceIndex - 1, vlJobHistory, vecData );

		AdviceSkillTreeSort( vecData, sData.m_Data );

		m_vecPresetSkillTree.push_back( sData );
	}
}

void CDnSkillTask::SetPresetSkillTree()
{
	InitAdviceSkillTreeTable();

	for( DWORD itr = 0; itr < m_vecPresetSkillTree.size(); ++itr )
	{
		if( true == m_vecPresetSkillTree[itr].m_bAdvice )
			m_vecPresetSkillTree[itr].m_cIndex = (BYTE)(itr + E_ADVICE_INDEX_START);
	}

	CDnSkillTreeDlg* pSkillDlg  = (CDnSkillTreeDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG );
	if( pSkillDlg )
		pSkillDlg->UpdateView();
}

std::vector< CDnSkillTask::SPresetSkillTree > & CDnSkillTask::GetPresetSkillTree()
{
	return m_vecPresetSkillTree;
}

bool CDnSkillTask::IsPresetSkillTreeSave()
{
	const int nUsedSkillPoint = GetWholeUsedSkillPoint() + GetReservationSkillNeedSP();

	if( 0 == nUsedSkillPoint )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 743) );	// UISTRING : No skill points used.
		return false;
	}

	if( -1 == GetEmptyEnablePresetIndex() )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 744) );	// UISTRING : Insufficient storage space.
		return false;
	}

	return true;
}

int CDnSkillTask::GetEmptyEnablePresetIndex()
{
	for( DWORD itr = 0; itr < m_vecPresetSkillTree.size(); ++itr )
	{
		if( m_vecPresetSkillTree[itr].m_bAdvice == false && m_vecPresetSkillTree[itr].m_Data.empty() )
			return m_vecPresetSkillTree[itr].m_cIndex;
	}

	return -1;
}

void CDnSkillTask::PresetSkillLevelUp( const int nSkillID, const BYTE cLevel )
{
	for( int itr = 1; itr <= cLevel; ++itr )
	{
		DnSkillHandle hSkill = FindSkill( nSkillID );

		if( CDnSkill::Identity() != hSkill && hSkill->GetLevel() == cLevel && false == hSkill->IsLocked() )
			break;

		if( 1 == itr )
		{
			if( CDnSkill::Identity() == hSkill )
				hSkill = FindZeroLevelSkill( nSkillID );

			if( CDnSkill::Identity() == hSkill || true == hSkill->IsLocked() )
			{
				CDnSkillTreeSystem::S_OUTPUT Output;
				CDnSkillTreeSystem::S_TRY_UNLOCK TryUnlockInfo;
				TryUnlockInfo.iTryUnlockSkillID = nSkillID;
				TryUnlockInfo.iCurrentCharLevel = cLevel;

				m_SkillTreeSystem.TryUnLockSkill( TryUnlockInfo, &Output );
				if( CDnSkillTreeSystem::R_SUCCESS != Output.eResult )
					continue;

				Change_SkillLevelUp( nSkillID, 1, ReservationSKillList::Type::UnLock );
			}
			else if( hSkill && false == hSkill->IsAcquired() )
				Change_SkillLevelUp( nSkillID, 1, ReservationSKillList::Type::Acquire );
		}
		else
		{
			Change_SkillLevelUp( nSkillID, itr, ReservationSKillList::Type::LevelUp );
		}
	}

	SetReservationSkill_Item( nSkillID );
}
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )