#include "StdAfx.h"
#include "DnBuffUIMng.h"
#include "DnCustomControlCommon.h"
#include "DnBlow.h"
#include "DnLocalPlayerActor.h"
#include "DnBubbleSystem.h"
#include "DnTableDB.h"
#include "DnStateBlow.h"
#include "DnSkillTask.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnInterface.h"

using namespace BubbleSystem;

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnBuffUIMng::CDnBuffUIMng(int maxBuffSlotCount, int maxBubbleSlotCount, eBuffUIType type) : m_BuffSlotCount(maxBuffSlotCount), m_BubbleSlotCount(maxBubbleSlotCount), m_Type(type)
{
}

int CDnBuffUIMng::CheckEmptyBubbleSlot() const
{
	int emptySlotSize = m_BubbleSlotCount - (int)m_BubbleSlotList.size();
	if (emptySlotSize <= m_BubbleSlotCount && emptySlotSize > 0)
		return emptySlotSize;
	return 0;
}

CDnBuffUIMng::SBubbleUnit* CDnBuffUIMng::GetBubbleSlot(int bubbleTypeID)
{
	std::list<SBubbleUnit>::iterator iter = m_BubbleSlotList.begin();
	for (; iter != m_BubbleSlotList.end(); ++iter)
	{
		SBubbleUnit& unit = *iter;
		if (unit.bubbleTypeID == bubbleTypeID)
			return &unit;
	}

	return NULL;
}

int CDnBuffUIMng::CheckEmptyBuffSlot() const
{
	int emptySlotSize = m_BuffSlotCount - (int)m_BuffSlotList.size();
	if (emptySlotSize <= m_BuffSlotCount && emptySlotSize > 0)
		return emptySlotSize;
	return 0;
}

CDnBuffUIMng::SBuffUnit* CDnBuffUIMng::GetBuffSlot(int skillId)
{
	std::list<SBuffUnit>::iterator iter = m_BuffSlotList.begin();
	for (; iter != m_BuffSlotList.end(); ++iter)
	{
		SBuffUnit& unit = *iter;
		if (unit.skillID == skillId)
			return &unit;
	}

	return NULL;
}

CDnBuffUIMng::SBuffUnit* CDnBuffUIMng::GetBuffInWaitingList(int skillId)
{
	std::list<SBuffUnit>::iterator iter = m_WaitingBuffList.begin();
	for (; iter != m_WaitingBuffList.end(); ++iter)
	{
		SBuffUnit& unit = *iter;
		if (unit.skillID == skillId)
			return &unit;
	}

	return NULL;
}

bool CDnBuffUIMng::ProcessExistingBuffs(DnBlowHandle hBlow, int skillId, DnActorHandle hActor)
{
	bool bRet = false;
	if (!hBlow)
		return bRet;

	const float& curDuration = hBlow->GetDurationTime();
	const int& curBlowId = hBlow->GetBlowIndex();

	SBuffUnit* pUnit = GetBuffSlot(skillId);
	if (pUnit != NULL)
	{
		const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
		if( pUnit && pSkillInfo && ( (pSkillInfo->hSkillUser == hActor || pSkillInfo->iSkillUserTeam == hActor->GetTeam()) == pUnit->bOwner ) )
		{
			if (pUnit->skillID == 2212)	// temp by kalliste
			{
				GetDuration(pUnit->duration, pUnit->blowIdx, hActor, hBlow);
			}
			else
			{
				if (pUnit->blowIdx == curBlowId)
					pUnit->duration = curDuration;
			}
			bRet = pUnit->bUpdated = true;
		}
		//OutputDebug("[BUFFUI] InBuffSlot ----- skillId:%d, pUnit->blowIdx(%d), curBlowid(%d), pUnit->duration(%f) = curDuration(%f), iconIdx(%f,%f)\n", skillId, pUnit->blowIdx, curBlowId, pUnit->duration, curDuration, pUnit->textureUV.fX, pUnit->textureUV.fY);
	}

	SBuffUnit* pWaitUnit = GetBuffInWaitingList(skillId);
	if (pWaitUnit != NULL)
	{
		if (pWaitUnit->blowIdx == curBlowId)
			pWaitUnit->duration = curDuration;
		bRet = pWaitUnit->bUpdated = true;
		//OutputDebug("[BUFFUI] InWaitingSlot ----- skillId:%d, pWaitUnit->blowIdx(%d), curBlowid(%d), pWaitUnit->duration(%f) = curDuration(%f), iconIdx(%f,%f)\n", skillId, pWaitUnit->blowIdx, curBlowId, pWaitUnit->duration, curDuration, pUnit->textureUV.fX, pUnit->textureUV.fY);
	}

	return bRet;
}

void CDnBuffUIMng::GetTextureUV(SUICoord& uvCoord, int iconImageIdx)
{
	iconImageIdx %= BUFF_TEXTURE_ICON_COUNT;

	int iconSizeX = (m_Type == ePlayer) ? BUFFSLOT_ICON_XSIZE : BUFFSLOT_ICON_PARTYMEMBER_XSIZE;
	int iconSizeY = (m_Type == ePlayer) ? BUFFSLOT_ICON_YSIZE : BUFFSLOT_ICON_PARTYMEMBER_YSIZE;

	uvCoord.fX = float((iconImageIdx % BUFFSLOT_ICON_XCOUNT) * iconSizeX);
	uvCoord.fY = float((iconImageIdx / BUFFSLOT_ICON_XCOUNT) * iconSizeY);
	uvCoord.fWidth	= (float)iconSizeX;
	uvCoord.fHeight = (float)iconSizeY;
}

void CDnBuffUIMng::GetDuration(float& resDuration, int& resBlowID, DnActorHandle hActor, DnBlowHandle hCurBlow)
{
	const CDnSkill::SkillInfo* curSkillInfo = hCurBlow->GetParentSkillInfo();
	if (curSkillInfo == NULL)
	{
		resDuration = -1;
		resBlowID = -1;
	}

	float curSkillDuration = hCurBlow->GetDurationTime();
	int iNumAppliedStateBlow = hActor->GetNumAppliedStateBlow();
	int iBlow = 0;
	int longesetBlow = hCurBlow->GetBlowIndex();
	for (; iBlow < iNumAppliedStateBlow; ++iBlow)
	{
		DnBlowHandle hBlow = hActor->GetAppliedStateBlow(iBlow);
		if (!hBlow)
			continue;

		const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
		if (pSkillInfo && pSkillInfo->iSkillID == curSkillInfo->iSkillID)
		{
			if (hBlow->GetDurationTime() > curSkillDuration)
			{
				curSkillDuration = hBlow->GetDurationTime();
				longesetBlow = hBlow->GetBlowIndex();
			}
		}
	}

	resDuration = curSkillDuration;
	resBlowID	= longesetBlow;
}

void CDnBuffUIMng::Process(DnActorHandle hActor)
{
	if (CDnActor::s_hLocalActor && hActor == CDnActor::s_hLocalActor)
	{
		CDnLocalPlayerActor* pLocalActor = static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		if (pLocalActor)
		{
			CDnBubbleSystem* pBubbleSystem = pLocalActor->GetBubbleSystem();
			if (pBubbleSystem)
			{
				std::vector<CDnBubbleSystem::S_BUBBLE_INFO> bubbleInfoList;
				pBubbleSystem->GetAllAppliedBubbles(bubbleInfoList);

				std::vector<CDnBubbleSystem::S_BUBBLE_INFO>::const_iterator iter = bubbleInfoList.begin();
				for (; iter != bubbleInfoList.end(); ++iter)
				{
					const CDnBubbleSystem::S_BUBBLE_INFO& bubbleInfo = (*iter);




					SBubbleUnit* pUnit = GetBubbleSlot(bubbleInfo.iBubbleTypeID);
					if (pUnit == NULL)
					{
						if (CheckEmptyBubbleSlot() > 0)
						{
							SBubbleUnit curUnitForAdd;
							curUnitForAdd.bubbleTypeID	= bubbleInfo.iBubbleTypeID;
							curUnitForAdd.bubbleCount	= bubbleInfo.iCount;
							curUnitForAdd.remainTime	= bubbleInfo.fRemainTime;
							curUnitForAdd.duration		= bubbleInfo.fDurationTime;
							GetTextureUV(curUnitForAdd.textureUV, bubbleInfo.iIconIndex);
							curUnitForAdd.texturePageIdx = bubbleInfo.iIconIndex / BUFF_TEXTURE_ICON_COUNT;
							curUnitForAdd.bUpdated		= true;


							m_BubbleSlotList.push_back(curUnitForAdd);
						}
					}
					else
					{
						pUnit->bubbleCount	= bubbleInfo.iCount;
						pUnit->remainTime	= bubbleInfo.fRemainTime;
						pUnit->bUpdated		= true;

					}
				}
			}
		}
	}
	
	int iNumAppliedStateBlow = hActor->GetNumAppliedStateBlow();
	int iBlow = 0;
	for (; iBlow < iNumAppliedStateBlow; ++iBlow)
	{
		DnBlowHandle hBlow = hActor->GetAppliedStateBlow(iBlow);
		if (!hBlow)
			continue;

		if( hBlow->IsFromSourceItem() )
			continue;

		const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
		if( pSkillInfo &&
			( (pSkillInfo->eDurationType == CDnSkill::Buff ) || 
			( pSkillInfo->eDurationType == CDnSkill::Debuff ) || 
			( pSkillInfo->eDurationType == CDnSkill::Aura ) || 
			( pSkillInfo->eDurationType == CDnSkill::StanceChange ) || 
			( pSkillInfo->eSkillType == CDnSkill::Passive &&  pSkillInfo->eDurationType == CDnSkill::Instantly ) ) 
			&& ( pSkillInfo->iBuffIconImageIndex >= 0
#ifdef PRE_ADD_SKILL_ADDTIONAL_BUFF			
			||  (pSkillInfo->iDebuffIconImageIndex >= 0 && hActor->GetTeam() != pSkillInfo->iSkillUserTeam )
#endif
			)
		  )
		{
#ifdef PRE_ADD_SKILL_ADDTIONAL_BUFF
			//Do not display if it has no duration (when duration is 0, it becomes negative during Process), is a state effect applied to an opponent (different team), is a debuff, and has no debuff icon applied..
			if (hBlow->GetDurationTime() <= 0.0f && 
				pSkillInfo->eDurationType == CDnSkill::Debuff &&
				hActor->GetTeam() != pSkillInfo->iSkillUserTeam &&
				pSkillInfo->iDebuffIconImageIndex < 0
				)
				continue;
#endif // PRE_ADD_SKILL_ADDTIONAL_BUFF

#ifdef PRE_FIX_STAND_CHANGE_SE_REFRESH
			if( pSkillInfo->eDurationType == CDnSkill::StanceChange )
			{
				if( hBlow->GetBlowIndex() != STATE_BLOW::BLOW_121 )
					continue;
			}
#endif

			if( !hBlow->IsPermanent() || pSkillInfo->bFromBuffProp ||
				(pSkillInfo->eDurationType == CDnSkill::Aura) ||	// Do not display buff icon for 0-second state effects applied to self (except auras) - Hanki
				(hBlow->GetBlowIndex() == STATE_BLOW::BLOW_030) )	// Exception requested to treat block state effect as having no time limit.. 2010.09.20
			{
				if (ProcessExistingBuffs(hBlow, pSkillInfo->iSkillID, hActor) == false)
				{
					SBuffUnit unit;
					unit.skillID	= pSkillInfo->iSkillID;
#ifdef PRE_ADD_BUFF_ADD_INFORMATION
					unit.skillLevelID = pSkillInfo->iSkillLevelID;
#endif
					GetDuration(unit.duration, unit.blowIdx, hActor, hBlow);
					unit.buffType = pSkillInfo->eDurationType;
					unit.texturePageIdx = pSkillInfo->iBuffIconImageIndex / BUFF_TEXTURE_ICON_COUNT;
					GetTextureUV(unit.textureUV, pSkillInfo->iBuffIconImageIndex);
					unit.bUpdated = true;

#if defined(PRE_FIX_48494)
					if( const_cast<CDnSkill::SkillInfo*>(pSkillInfo)->hSkillUser )
					{
						//#49409 Same team? (party) should also display infinity..
						int nActorTeam = hActor->GetTeam();
						int nSkillUserTeam = pSkillInfo->hSkillUser->GetTeam();
						if (hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))	//In case of ice prison..
							hActor->GetOriginalTeam();

						if (pSkillInfo->hSkillUser->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149)) //In case of ice prison..
							pSkillInfo->hSkillUser->GetOriginalTeam();

						unit.bOwner = (hActor == pSkillInfo->hSkillUser || nActorTeam == nSkillUserTeam);	//Whether the skill user is the same.
					}
#endif // PRE_FIX_48494

					// Here we can check if the skill buff icon is harm type in pSkillInfo to classify and insert it.
#ifdef PRE_ADD_SKILL_ADDTIONAL_BUFF
					if( hActor->GetTeam() != pSkillInfo->iSkillUserTeam && pSkillInfo->iDebuffIconImageIndex >= 0)
					{
						unit.texturePageIdx = pSkillInfo->iDebuffIconImageIndex / BUFF_TEXTURE_ICON_COUNT;
						GetTextureUV(unit.textureUV, pSkillInfo->iDebuffIconImageIndex );
					}
#endif
					m_WaitingBuffList.push_back(unit);
				}
			}
		}
	}

	int emptySlotCount = CheckEmptyBuffSlot();
	if (emptySlotCount > 0)
	{
		if (m_WaitingBuffList.empty() == false)
		{
			SBuffUnit frontBuf = m_WaitingBuffList.front();
			if (frontBuf.buffType == CDnSkill::Buff ||
				frontBuf.buffType == CDnSkill::Aura || 
				frontBuf.buffType == CDnSkill::StanceChange || 
				frontBuf.buffType == CDnSkill::Instantly )
			{
				std::list<SBuffUnit>::iterator iter = m_BuffSlotList.begin();

				for (; iter != m_BuffSlotList.end(); ++iter)
				{
					SBuffUnit& unit = *iter;
					if (unit.buffType == CDnSkill::Debuff)
					{
						m_BuffSlotList.insert(iter, frontBuf);
						break;
					}
				}

				if (iter == m_BuffSlotList.end())
					m_BuffSlotList.push_back(frontBuf);

				if (int(m_BuffSlotList.size()) > m_BuffSlotCount)
				{
					_ASSERT(0);
				}
			}
			else if (frontBuf.buffType == CDnSkill::Debuff)
			{
				m_BuffSlotList.push_back(frontBuf);
				if (int(m_BuffSlotList.size()) > m_BuffSlotCount)
				{
					_ASSERT(0);
				}
			}

			m_WaitingBuffList.pop_front();
		}
	}

#ifdef PRE_ADD_BUFF_ADD_INFORMATION

	if ( hActor && hActor->IsPlayerActor() )
	{
#ifdef PRE_ADD_NAMEDITEM_SYSTEM
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		for( int i=0; i< pPlayerActor->GetNumEffectSkill(); i++ )
		{
			const CDnPlayerActor::S_EFFECT_SKILL* EffectItem = pPlayerActor->GetEffectSkillFromIndex( i );
			if(EffectItem)
			{
				DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
				DNTableFileFormat* pSkillSox = GetDNTable( CDnTableDB::TSKILL );
				DNTableFileFormat* pSkillLevelSox = GetDNTable( CDnTableDB::TSKILLLEVEL );

				if( (pItemSox && pItemSox->IsExistItem(EffectItem->iItemID)) ) 
				{
					int nSkillTableID = pItemSox->GetFieldFromLablePtr(EffectItem->iItemID , "_SkillID")->GetInteger();

					eItemTypeEnum eType = (eItemTypeEnum)pItemSox->GetFieldFromLablePtr( EffectItem->iItemID, "_Type" )->GetInteger();
					
					if( ( CDnItem::IsEffectSkillItem( eType , true ) == true ) )
					{
						CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
						if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP ) 
							continue; // Do not display in PVP.
					}
					else
					{
						continue; // Display only named best-friend buffs.
					}

					if( pSkillSox && pSkillSox->IsExistItem(nSkillTableID) && pSkillLevelSox )
					{
						int nSkillLevel = pItemSox->GetFieldFromLablePtr(EffectItem->iItemID , "_SkillLevel")->GetInteger();
						int nBuffIconIndex = pSkillSox->GetFieldFromLablePtr( nSkillTableID, "_BuffIconImageIndex" )->GetInteger();
						int iSkillLevelTableID = -1;

						if( nBuffIconIndex < 0 )
							continue;
						
						vector<int> vlSkillLevelList;
						GetSkillTask().GetSkillLevelList( nSkillTableID, vlSkillLevelList, CDnSkill::PVE );
						if( false == vlSkillLevelList.empty() )
						{
							for( int i = 0; i < (int)vlSkillLevelList.size(); ++i )
							{
								int iNowLevel = pSkillLevelSox->GetFieldFromLablePtr( vlSkillLevelList.at(i), "_SkillLevel" )->GetInteger();
								if( iNowLevel == nSkillLevel )
								{
									iSkillLevelTableID = vlSkillLevelList.at( i );
									break;
								}
							}
						}

						SBuffUnit sBuff;
						sBuff.Clear();

						int nIconTextureIndex = nBuffIconIndex;
						sBuff.texturePageIdx = nIconTextureIndex / BUFF_TEXTURE_ICON_COUNT;
						GetTextureUV(sBuff.textureUV, nIconTextureIndex );

						sBuff.bUpdated = true;
						sBuff.bOwner = true;
						sBuff.duration = float(EffectItem->nEffectSkillLeftTime/1000);

						if( EffectItem->bEternity == true || sBuff.duration > 6000.f || sBuff.duration < 0 )
						{
							sBuff.buffType = CDnSkill::DurationTypeEnum::Aura;
						}
						else
						{
							sBuff.buffType = CDnSkill::DurationTypeEnum::Buff;
						}
			
						sBuff.skillID = nSkillTableID;
						sBuff.skillLevelID = iSkillLevelTableID;
						m_LowPriorityList.push_back(sBuff);
					}
				}
			}
		}
#endif
	}
#endif

}

void CDnBuffUIMng::ClearLists()
{
	m_BuffSlotList.clear();
	m_WaitingBuffList.clear();
	m_BubbleSlotList.clear();
	m_LowPriorityList.clear();
}