#include "stdafx.h"
#include "MASkillUser.h"
#include "DnSkill.h"
#include "IDnSkillUsableChecker.h"
#include "IDnSkillProcessor.h"
#if defined(_CLIENT)
#include "InputReceiver.h"
#endif
#include "DnProjectile.h"
#include "DnPlayerActor.h"
#include "DnChangeProjectileProcessor.h"
#include "DnChangeActionStrProcessor.h"
#include "DnBlow.h"
#include "DnSkillTask.h"
#include "DnStateBlow.h"
#ifndef _GAMESERVER
#include "DnCantUseActiveSkillBlow.h"
#include "DnWorld.h"
#include "TaskManager.h"
#include "DnItemTask.h"
#include "DnInterface.h"
#else
#include "DNUserSession.h"
#include "DnApplySEWhenTargetNormalHitProcessor.h"
#endif

#if defined(_GAMESERVER)
#include "DnProbabilityChecker.h"
#endif

#include "DnCreateBlow.h"

#include "SmartPtrDef.h"

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
#include "TotalLevelSkillSystem.h"
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

MASkillUser::MASkillUser()
//: m_vSkillTargetPos( 0.f, 0.f, 0.f )
{
	m_LocalTime = 0;

	m_pActor = NULL;
	m_bIsValidActor = true;

	//m_fAdjustCoolTime = 1.0f;
#ifdef _GAMESERVER
	m_bIgnoreCoolTime = false;
#else
	m_bSkillExecutedThisFrame = false;
#endif

	m_fCoolTimeDeltaAdjustValue = 1.0f;

	//ZeroMemory( m_aEternityItemApplyStat, sizeof(m_aEternityItemApplyStat) );
}

MASkillUser::~MASkillUser()
{
#ifndef PRE_FIX_SKILLLIST
	for( DWORD i=0; i<m_vlhSkillList.size(); i++ ) {
		if( m_vbSelfAllocList[i] == true ) SAFE_RELEASE_SPTR( m_vlhSkillList[i] );
	}
	SAFE_DELETE_VEC( m_vlhSkillList );
	SAFE_DELETE_VEC( m_vbSelfAllocList );
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		S_SKILL_OBJECT& SkillObject = m_vlSkillObjects.at( i );
		if( true == SkillObject.bSelfAlloc )
			SAFE_RELEASE_SPTR( SkillObject.hSkill );
	}
	SAFE_DELETE_VEC( m_vlSkillObjects );
#endif // #ifndef PRE_FIX_SKILLLIST

#if defined(_GAMESERVER)
	m_PrefixSkillCoolTimeManager.InitList();

	//Reset defensive skill
	PREFIX_SYSTEM_SKILL_LIST::iterator iter = m_prefixSystemDefenceSkills.begin();
	PREFIX_SYSTEM_SKILL_LIST::iterator endIter = m_prefixSystemDefenceSkills.end();
	for (; iter != endIter; ++iter)
	{
		if (iter->second.hSkill)
			SAFE_RELEASE_SPTR(iter->second.hSkill);
	}
	SAFE_DELETE_MAP(m_prefixSystemDefenceSkills);

	//Reset attack skills
	iter = m_prefixSystemOffenceSkills.begin();
	endIter = m_prefixSystemOffenceSkills.end();
	for (; iter != endIter; ++iter)
	{
		if (iter->second.hSkill)
			SAFE_RELEASE_SPTR(iter->second.hSkill);
	}
	SAFE_DELETE_MAP(m_prefixSystemOffenceSkills);
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
#if defined(_GAMESERVER)
	{
		//Prefix defense skill
		PREFIX_SKILL_INFO::iterator iter = m_PrefixDefenceSkills.begin();
		PREFIX_SKILL_INFO::iterator endIter = m_PrefixDefenceSkills.end();
		for (; iter != endIter; ++iter)
		{
			if(iter->second)
				SAFE_DELETE(iter->second);
		}
		m_PrefixDefenceSkills.clear();

		//Prefix attack skill
		iter = m_PrefixOffenceSkills.begin();
		endIter = m_PrefixOffenceSkills.end();
		for (; iter != endIter; ++iter)
		{
			if(iter->second)
				SAFE_DELETE(iter->second);
		}
		m_PrefixOffenceSkills.clear();

		//List of prefix attack skills currently in use
		m_ProcessPrefixOffenceSkills.clear();
	}
#endif // _GAMESERVER
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
}

bool MASkillUser::IsValidActor()
{
	if( m_pActor ) return true;
	else {
		if( !m_bIsValidActor ) return false;
		m_pActor = dynamic_cast<CDnActor*>(this);
		if( !m_pActor ) m_bIsValidActor = false;
	}
	return true;
}

void MASkillUser::OnAddSkill( DnSkillHandle hSkill, bool isInitialize/* = false*/ )
{
	// Immediately apply skills that are passive, buffs, and have Self state effects.
	if( !IsValidActor() ) return;
	bool bPassiveBuff = false;

	bPassiveBuff = ApplyPassiveSkill( hSkill, isInitialize );
#ifdef _GAMESERVER
	if( CDnSkill::AutoPassive == hSkill->GetSkillType() )
		m_listAutoPassiveSkills.push_back( hSkill );
#else
	const set<string>& setUseAction = hSkill->GetUseActionSet();
	set<string>::const_iterator iter = setUseAction.begin();
	for( iter; iter != setUseAction.end(); ++iter )
		m_setUseActionNames.insert( *iter );
#endif

	if( bPassiveBuff )
		m_vlhSelfPassiveBlowSkill.push_back( hSkill );
}

bool MASkillUser::AddSkill( int nSkillTableID, int nLevel/* = 1*/, int iSkillLevelApplyType/* = CDnSkill::PVE*/ )
{
	if( IsExistSkill( nSkillTableID, nLevel ) )
		return false;

	if( !IsValidActor() ) 
		return false;

#ifdef _GAMESERVER
	if( m_pActor == NULL || m_pActor->GetRoom() == NULL ) 
		return false;
#endif

	//Add if level-up info exists.
	int nLevelUp = GetSkillLevelUpValue(nSkillTableID);
	nLevel += nLevelUp;

	DnSkillHandle hSkill = CDnSkill::CreateSkill( m_pActor->GetMySmartPtr(), nSkillTableID, nLevel );

	//If level-up was attempted but skill creation failed
	if (nLevelUp != 0 && !hSkill)
	{
		OutputDebug("Failed to create skill (%d) level (%d).. Check skill level table!!!!!!\n", nSkillTableID, nLevel);
		_ASSERT( hSkill && "Skill level-up attempt failed.. Check level table" );

		//Retry creating at the original level..
		nLevel -= nLevelUp;
		hSkill = CDnSkill::CreateSkill(m_pActor->GetMySmartPtr(), nSkillTableID, nLevel);

		nLevelUp = 0;
	}

	if( !hSkill ) 
		return false;

	//Set the added level-up information.
	hSkill->SetLevelUpValue(nLevelUp);

	// Set skill level table apply type
	if( m_pActor->IsPlayerActor() )
		hSkill->SelectLevelDataType( iSkillLevelApplyType );

#ifndef PRE_FIX_SKILLLIST
	m_vlhSkillList.push_back( hSkill );
	m_vbSelfAllocList.push_back( true );
#else
	m_vlSkillObjects.push_back( S_SKILL_OBJECT(hSkill, true) );
#endif // #ifndef PRE_FIX_SKILLLIST

	// Store toggle and aura skills in a separate list
	CDnSkill::DurationTypeEnum eDurationType = hSkill->GetDurationType();
	switch(eDurationType)
	{
	case CDnSkill::ActiveToggle:
	case CDnSkill::TimeToggle:
	case CDnSkill::ActiveToggleForSummon:
		m_vlhToggleSkills.push_back(hSkill);
	case CDnSkill::Aura:
		m_vlhAuraSkills.push_back(hSkill);
	}

	if( hSkill->GetActor() )
		hSkill->SetHasActor( m_pActor->GetMySmartPtr() );

	OnAddSkill( hSkill );
	return true;
}

#if defined(PRE_FIX_62052)
void MASkillUser::ApplyGlobalSkillCoolTime(DnSkillHandle hSkill)
{
	int nGlobalSkillGroupID = hSkill ? hSkill->GetGlobalSkillGroupID() : 0;
	//When a skill with GlobalSkillGroupID set is added, also apply the cooldown of an existing skill with the same ID to the newly added skill.

	if (nGlobalSkillGroupID != 0)
	{
		for( DWORD i = 0; i < GetSkillCount(); ++i )
		{
			// Because the values of m_vlGroupedSkillIDs are simply pulled from the table
			// Must verify whether the skill has actually been learned.
			DnSkillHandle hExistSkill = GetSkillFromIndex( i );
			float fElapsedDelayTime = hExistSkill->GetElapsedDelayTime();

			if( hExistSkill && 
				hExistSkill->GetGlobalSkillGroupID() == nGlobalSkillGroupID &&
				fElapsedDelayTime > 0.0f)
			{
				DnSkillHandle hOrigSkill = FindSkill(hExistSkill->GetAnotherGlobalSkillID());

				hSkill->OnAnotherGlobalSkillBeginCoolTime( hOrigSkill );
				hSkill->SetElapsedDelayTime(fElapsedDelayTime);
			}
		}
	}
}
#endif // PRE_FIX_62052

bool MASkillUser::AddSkill( DnSkillHandle hSkill )
{
#ifdef _GAMESERVER
	// On game server, addskill directly as a skill object only for monsters.
	// Handled as a virtual function on the server for player actors due to skill tree management 
	// Do not use this function unless overridden by playeractor.
	// Place an assert as a reminder in case it is used in the future.
	//_ASSERT( IsValidActor() );
	//_ASSERT( m_pActor->IsMonsterActor() );
#endif

	if( IsExistSkill( hSkill->GetClassID(), hSkill->GetLevel() ) ) return false;

#if defined(PRE_FIX_62052)
	ApplyGlobalSkillCoolTime(hSkill);
#endif // PRE_FIX_62052

#ifndef PRE_FIX_SKILLLIST
	m_vlhSkillList.push_back( hSkill );
	m_vbSelfAllocList.push_back( false );
#else
	m_vlSkillObjects.push_back( S_SKILL_OBJECT(hSkill, false) );
#endif // #ifndef PRE_FIX_SKILLLIST

	// Store toggle and aura skills in a separate list
	CDnSkill::DurationTypeEnum eDurationType = hSkill->GetDurationType();
	switch(eDurationType)
	{
	case CDnSkill::ActiveToggle:
	case CDnSkill::TimeToggle:
	case CDnSkill::ActiveToggleForSummon:
		m_vlhToggleSkills.push_back(hSkill);
	case CDnSkill::Aura:
		m_vlhAuraSkills.push_back(hSkill);
	}

	OnAddSkill( hSkill );

	return true;
}

bool MASkillUser::bIsPassiveSkill( DnBlowHandle hBlow )
{
	int iNumSkill = (int)m_vlhSelfPassiveBlowSkill.size();
	for( int iSkill = 0; iSkill < iNumSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhSelfPassiveBlowSkill.at( iSkill );
		_ASSERT( CDnSkill::Passive == hSkill->GetSkillType() );
		const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
		if( pSkillInfo && (pSkillInfo->iSkillID == hSkill->GetClassID()) )
			return true;

		//if( CDnSkill::Passive == hSkill->GetSkillType() )
		//{
		//	int iNumStateEffect = hSkill->GetStateEffectCount();
		//	for( int i = 0; i < iNumStateEffect; ++i )
		//	{
		//		CDnSkill::StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( i );
		//		if( CDnSkill::StateEffectApplyType::ApplySelf == pSE->ApplyType )
		//		{
		//			if( CDnSkill::DurationTypeEnum::Buff == hSkill->GetDurationType() )
		//			{
		//				if( hBlow->GetBlowIndex() == pSE->nID )
		//					return true;
		//			}
		//		}
		//	}
		//}
	}

	return false;
}

bool MASkillUser::ApplyPassiveSkill( DnSkillHandle hSkill, bool isInitialize/* = false*/ )
{
	bool bPassiveBuf = false;

	// #24143 When initializing own skills, call AddSkill() based on the actually acquired skill list from CDnSkillTask 
	// No issue since it is called, but for other players it just uses the skill list pushed by the server (including reset level 0 skills)
	// Because it calls AddSkill() directly, there is a bug where reset passive skills also end up being applied. 
	// Therefore, if not in acquired state here, do not activate passive buff skills (such as max HP/MP increase skills, etc.).
	// Unlike client, on game server each CDnPlayerActor object holds only the skills it actually possesses.
	// On client side, LocalPlayerActor also needs reset skill info, so add all skills in skill list without distinguishing PlayerActor..
#ifndef _GAMESERVER
	if( false == hSkill->IsAcquired() )
		return false;
#endif // #ifndef _GAMESERVER

	if( CDnSkill::Passive == hSkill->GetSkillType() )
	{

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
		bool isTotalLevelSkill = false;
		if (m_pActor && m_pActor->IsPlayerActor())
		{
			CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_pActor);
			CDnTotalLevelSkillSystem* pTotalLevelSystem = pPlayerActor ? pPlayerActor->GetTotalLevelSkillSystem() : NULL;
			
			if (pTotalLevelSystem)
				isTotalLevelSkill = pTotalLevelSystem->IsTotalLevelSkill(hSkill->GetClassID());
		}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

		int iNumStateEffect = hSkill->GetStateEffectCount();
		for( int i = 0; i < iNumStateEffect; ++i )
		{
			CDnSkill::StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( i );
			if( CDnSkill::StateEffectApplyType::ApplySelf == pSE->ApplyType )
			{
				if( CDnSkill::DurationTypeEnum::Buff == hSkill->GetDurationType() )
				{
					bPassiveBuf = true;
					if( hSkill->IsSatisfyWeapon() )
					{
						int iBlowID = -1;
						DnBlowHandle hBlow;

#ifdef _GAMESERVER
						if(m_pActor)
						{
							iBlowID = m_pActor->CDnActor::CmdAddStateEffect( hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pSE->nID, -1, pSE->szValue.c_str(), true ); // If Duration Time is -1, it is applied infinitely
							OnApplyPassiveSkillBlow( iBlowID );

							hBlow = m_pActor->GetStateBlowFromID(iBlowID);
						}
#else
						if(m_pActor)
							hBlow = m_pActor->CDnActor::CmdAddStateEffect( hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pSE->nID, -1, pSE->szValue.c_str(), true ); // If Duration Time is -1, it is applied infinitely
#endif

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
						if (isTotalLevelSkill == true && isInitialize == true && hBlow)
							hBlow->FromSourceItem();
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

						hSkill->SetAppliedPassiveBlows( true );
						
#ifndef _FINAL_BUILD
						char szTemp[256] = { 0, };
						WideCharToMultiByte( CP_ACP, 0, hSkill->GetName(), -1, szTemp, _countof(szTemp), NULL, NULL );
						OutputDebug( "[Passive Buff Skill Applied: %d] \"%s\" StateEffectIndex: %d, Value:%s\n", hSkill->GetClassID(), szTemp, pSE->nID, pSE->szValue.c_str() );
#endif // #ifndef _FINAL_BUILD
					}
#ifndef _FINAL_BUILD
					else
					{
						char szTemp[256] = { 0, };
						WideCharToMultiByte( CP_ACP, 0, hSkill->GetName(), -1, szTemp, _countof(szTemp), NULL, NULL );
						OutputDebug( "[Passive Buff Skill Not Applied: %d] Required weapon type specified for \"%s\" skill does not match.\n", hSkill->GetClassID(), szTemp );
					}
#endif // #ifndef _FINAL_BUILD
				}
#ifndef _FINAL_BUILD
				else
				{
					char szTemp[256] = { 0, };
					WideCharToMultiByte( CP_ACP, 0, hSkill->GetName(), -1, szTemp, _countof(szTemp), NULL, NULL );
					OutputDebug( "[Check Skill Data: %d] \"%s\" seems to be a passive buff skill, but not applied because Buff is not set\n", hSkill->GetClassID(), szTemp );
				}
#endif // #ifndef _FINAL_BUILD
			}
		}
	}

	return bPassiveBuf;
}
 
void MASkillUser::ApplyPassiveSkills( void )
{
	if( !IsValidActor() ) return;

	int iNumSkill = (int)m_vlhSelfPassiveBlowSkill.size();
	for( int iSkill = 0; iSkill < iNumSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhSelfPassiveBlowSkill.at( iSkill );
		ApplyPassiveSkill(hSkill);
	}
}

void MASkillUser::OnRemoveSkill( DnSkillHandle hSkill )
{
#ifndef _GAMESERVER
	const set<string>& setUseAction = hSkill->GetUseActionSet();
	set<string>::const_iterator iter = setUseAction.begin();
	for( iter; iter != setUseAction.end(); ++iter )
		m_setUseActionNames.erase( *iter );
#endif
}

bool MASkillUser::RemoveSkillAll()
{
	if( !IsValidActor() )
		return false;

#ifndef PRE_FIX_SKILLLIST
	int nSkillSize = (int)m_vlhSkillList.size();

	for( int i=0 ; i<nSkillSize ; i++ )
	{
		DnSkillHandle hSkill = m_vlhSkillList.back();
		if( !hSkill )
			continue;

		bool bRet = RemoveSkill( hSkill->GetClassID() );
		_ASSERT( bRet );
	}
#else

	int nSkillSize = (int)m_vlSkillObjects.size();

	for( int i=0 ; i<nSkillSize ; i++ )
	{
		S_SKILL_OBJECT& SkillObject = m_vlSkillObjects.back();
		DnSkillHandle hSkill = SkillObject.hSkill;
		if( !hSkill )
			continue;

		bool bRet = RemoveSkill( hSkill->GetClassID() );
		_ASSERT( bRet );
	}
#endif // #ifndef PRE_FIX_SKILLLIST

	return true;
}

bool MASkillUser::EndStateEffectSkill(int nSkillTableID)
{ 
	DNVector(DnBlowHandle) vlBlows;
	m_pActor->GetAllAppliedStateBlow(vlBlows);

	for (int i = 0; i < (int)vlBlows.size(); ++i)
	{
		DnBlowHandle hBlow = vlBlows.at(i);
		const CDnSkill::SkillInfo * skillInfo = hBlow->GetParentSkillInfo();
		if (skillInfo->iSkillID == nSkillTableID)
		{
			hBlow->OnEnd(0, 0);
			m_pActor->OnEndStateBlow(hBlow);
			m_pActor->RemoveStateBlowFromID(hBlow->GetBlowID()); //? need test.
		}
	}
/*
	DnSkillHandle hSkill = FindSkill(nSkillTableID);
	if (m_pActor && m_pActor->GetStateBlow())
	{
		std::vector<int> vecRemoveBlowID;
		DNVector(DnBlowHandle) vlBlows;
		m_pActor->GetAllAppliedStateBlowBySkillID(nSkillTableID, vlBlows);

		int iStateEffectCount = hSkill->GetStateEffectCount();
		for (int iStateEffect = 0; iStateEffect < iStateEffectCount; ++iStateEffect)
		{
			const CDnSkill::StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex(iStateEffect);
			if (pSE && CDnSkill::StateEffectApplyType::ApplySelf == pSE->ApplyType)
			{
				for (DWORD BLOW_COUNT = 0; BLOW_COUNT < vlBlows.size(); BLOW_COUNT++)
				{
					if (vlBlows[BLOW_COUNT] && (vlBlows[BLOW_COUNT]->GetBlowIndex() == (STATE_BLOW::emBLOW_INDEX)pSE->nID))
					{
							vecRemoveBlowID.push_back(vlBlows[BLOW_COUNT]->GetBlowID());
					}
				}
			}
		}

		for (DWORD Index = 0; Index < vecRemoveBlowID.size(); Index++)
		{
			m_pActor->GetStateBlow()->RemoveImediatlyStateEffectFromID(vecRemoveBlowID[Index]);
		}
	}
*/
	return true;
}

bool MASkillUser::RemoveSkill( int nSkillTableID )
{
	if( !IsValidActor() ) return false;

	int iNumPassiveBlowSkill = (int)m_vlhSelfPassiveBlowSkill.size();
	for( int iPassiveBlowSkill = 0; iPassiveBlowSkill < iNumPassiveBlowSkill; ++iPassiveBlowSkill )
	{
		DnSkillHandle hSkill = m_vlhSelfPassiveBlowSkill.at( iPassiveBlowSkill );
		if( hSkill && hSkill->GetClassID() == nSkillTableID )
		{
			if( m_pActor && m_pActor->GetStateBlow() )
			{
				std::vector<int> vecRemoveBlowID;
				DNVector(DnBlowHandle) vlBlows;
				m_pActor->GetAllAppliedStateBlowBySkillID( nSkillTableID , vlBlows );

				int iStateEffectCount = hSkill->GetStateEffectCount();
				for( int iStateEffect = 0; iStateEffect < iStateEffectCount; ++iStateEffect )
				{
					const CDnSkill::StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( iStateEffect );
					if( pSE && CDnSkill::StateEffectApplyType::ApplySelf == pSE->ApplyType )
					{
						for( DWORD BLOW_COUNT=0; BLOW_COUNT < vlBlows.size(); BLOW_COUNT++ )
						{
							if( vlBlows[BLOW_COUNT] && ( vlBlows[BLOW_COUNT]->GetBlowIndex() == (STATE_BLOW::emBLOW_INDEX)pSE->nID ) )
							{
								vecRemoveBlowID.push_back( vlBlows[BLOW_COUNT]->GetBlowID() );
							}
						}
					}
				}		

				for( DWORD Index=0; Index<vecRemoveBlowID.size(); Index++)
				{
					m_pActor->GetStateBlow()->RemoveImediatlyStateEffectFromID( vecRemoveBlowID[Index] );
				}
			}

			// 2009.03.16 kimbap
			m_vlhSelfPassiveBlowSkill.erase( m_vlhSelfPassiveBlowSkill.begin()+iPassiveBlowSkill );
			break;
		}
	}

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	//If it is a unified level skill, no need to execute the routine below...
	if (m_pActor && m_pActor->IsPlayerActor())
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_pActor);
		CDnTotalLevelSkillSystem *pTotalLevelSystem = pPlayerActor ? pPlayerActor->GetTotalLevelSkillSystem(): NULL;

		if (pTotalLevelSystem && pTotalLevelSystem->IsTotalLevelSkill(nSkillTableID))
			return true;
	}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#ifdef _GAMESERVER
	list<DnSkillHandle>::iterator iter = m_listAutoPassiveSkills.begin();
	for( iter; m_listAutoPassiveSkills.end() != iter; ++iter )
	{
		if( (*iter)->GetClassID() == nSkillTableID )
		{
			m_listAutoPassiveSkills.erase( iter );
			break;
		}
	}
#endif

#ifndef PRE_FIX_SKILLLIST
	for( DWORD i=0; i<m_vlhSkillList.size(); i++ ) {
		if( m_vlhSkillList[i]->GetClassID() == nSkillTableID ) {
			
			// If toggle or aura skill, remove from the separate list
			DnSkillHandle hSkill = m_vlhSkillList.at(i);
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		S_SKILL_OBJECT& SkillObject = m_vlSkillObjects.at( i );
		if( SkillObject.hSkill && SkillObject.hSkill->GetClassID() ==  nSkillTableID )
		{
			// If toggle or aura skill, remove from the separate list
			DnSkillHandle hSkill = SkillObject.hSkill;
#endif // #ifndef PRE_FIX_SKILLLIST
			if( CDnSkill::ActiveToggle == hSkill->GetDurationType() ||
				CDnSkill::TimeToggle == hSkill->GetDurationType() 	|| 
				CDnSkill::ActiveToggleForSummon == hSkill->GetDurationType()
				)
			{
				DNVector(DnSkillHandle)::iterator Localiter = find( m_vlhToggleSkills.begin(), m_vlhToggleSkills.end(), hSkill );
				m_vlhToggleSkills.erase( Localiter );
			}
			else
			if( CDnSkill::Aura == hSkill->GetDurationType() )
			{
				if (m_hAuraSkill && m_hAuraSkill->GetClassID() == nSkillTableID && m_hAuraSkill->IsAuraOn())
				{
					if (m_hAuraSkill == m_hProcessSkill)
						m_hProcessSkill.Identity();

					m_hAuraSkill->EnableAura( false );
					OnSkillAura(m_hAuraSkill, false);
					m_hAuraSkill.Identity();
				}

				DNVector(DnSkillHandle)::iterator Localiter = find( m_vlhAuraSkills.begin(), m_vlhAuraSkills.end(), hSkill );
				m_vlhAuraSkills.erase( Localiter );
			}

#ifndef PRE_FIX_SKILLLIST
			OnRemoveSkill( m_vlhSkillList[i] );
			if( m_vbSelfAllocList[i] == true ) SAFE_RELEASE_SPTR( m_vlhSkillList[i] );

			m_vlhSkillList.erase( m_vlhSkillList.begin() + i );
			m_vbSelfAllocList.erase( m_vbSelfAllocList.begin() + i );
#else
			OnRemoveSkill( SkillObject.hSkill );
			if( SkillObject.bSelfAlloc )
				SAFE_RELEASE_SPTR( SkillObject.hSkill );
			m_vlSkillObjects.erase( m_vlSkillObjects.begin()+i );
#endif // #ifndef PRE_FIX_SKILLLIST
			return true;
		}
	}
	return true;
}

#ifndef _GAMESERVER
bool MASkillUser::ReplacementSkill( int nSkillTableID, int nLevel )
{
	if( !IsValidActor() ) return false;

	DnSkillHandle hSkill = CDnSkill::CreateSkill( m_pActor->GetMySmartPtr(), nSkillTableID, nLevel );
	if( !hSkill ) return false;
	
	return ReplacementSkill( hSkill );
}


bool MASkillUser::ReplacementSkill( DnSkillHandle hNewSkill )
{
#ifndef PRE_FIX_SKILLLIST
	int iSkillID = hNewSkill->GetClassID();
	for( DWORD i=0; i<m_vlhSkillList.size(); i++ ) 
	{
		if( m_vlhSkillList[ i ]->GetClassID() == iSkillID ) 
		{
			// Is it a passive blow skill? If so, remove all existing state effects and apply new ones.
			DNVector(DnSkillHandle)::iterator iter = find( m_vlhSelfPassiveBlowSkill.begin(), m_vlhSelfPassiveBlowSkill.end(), m_vlhSkillList[ i ] );
			if( m_vlhSelfPassiveBlowSkill.end() != iter )
			{
				// Remove currently applied state effects.
				// Find and delete state effects matching the parent skill.
				if( !IsValidActor() ) continue;

				// When a passive skill that increases HP/SP is removed, current HP/SP reverts to base HP/SP without the skill
				// Since it gets clipped, fetch in advance and set it.
				INT64 iHP = m_pActor->GetHP();
				int iSP = m_pActor->GetSP();

				DNVector(DnBlowHandle) vlhAppliedSE;
				DNVector(DnBlowHandle) vlhSEToRemove;
				m_pActor->GetAllAppliedStateBlow( vlhAppliedSE );
				int iNumAppliedSE = (int)vlhAppliedSE.size();
				for( int iSE = 0; iSE < iNumAppliedSE; ++iSE )
				{
					DnBlowHandle hAppliedBlow = vlhAppliedSE.at( iSE );
					const CDnSkill::SkillInfo* pSkillInfo = hAppliedBlow->GetParentSkillInfo();
					if( pSkillInfo->iSkillID == (*iter)->GetClassID() )
						vlhSEToRemove.push_back( hAppliedBlow );
				}

				int iNumSEToRemove = (int)vlhSEToRemove.size();
				for( int iSE = 0; iSE < iNumSEToRemove; ++iSE )
				{
					DnBlowHandle hBlow = vlhSEToRemove.at(iSE);
					int iID = hBlow->GetBlowID();
					OutputDebug( "[Passive Buff Skill Removed: %d] %s StateEffectIndex: %d, Value:%s\n", 
								 (*iter)->GetClassID(), (*iter)->GetName(), hBlow->GetBlowIndex(), hBlow->GetValue() );
					m_pActor->RemoveStateBlowFromID( iID );
				}

				m_vlhSelfPassiveBlowSkill.erase( iter );

				ApplyPassiveSkill( hNewSkill );
				m_vlhSelfPassiveBlowSkill.push_back( hNewSkill );

				m_pActor->SetHP( iHP );
				m_pActor->SetSP( iSP );
			}

			if (m_hAuraSkill && m_hAuraSkill->GetClassID() == iSkillID && m_hAuraSkill->IsAuraOn())
			{
				if (m_hAuraSkill == m_hProcessSkill)
					m_hProcessSkill.Identity();

				m_hAuraSkill->EnableAura( false );
				OnSkillAura(m_hAuraSkill, false);
				m_hAuraSkill.Identity();
			}

			if( m_vbSelfAllocList[ i ] )
			{
				// Because self must delete the skill it created.
				DnSkillHandle hSkillToRelease = m_vlhSkillList[ i ];

				// Search and replace in the aura and toggle skill lists as well
				DNVector(DnSkillHandle)::iterator iterToggle = find( m_vlhToggleSkills.begin(), m_vlhToggleSkills.end(), hSkillToRelease );
				if( m_vlhToggleSkills.end() != iterToggle )
					*iterToggle = hNewSkill;

				DNVector(DnSkillHandle)::iterator iterAura = find( m_vlhAuraSkills.begin(), m_vlhAuraSkills.end(), hSkillToRelease );
				if( m_vlhAuraSkills.end() != iterAura )
					*iterAura = hNewSkill;

				OnRemoveSkill( hSkillToRelease );
				SAFE_RELEASE_SPTR( hSkillToRelease );

			}
			else
			{
				// Do not delete skill objects created by the skill task here.
				DnSkillHandle hPrevLevelSkill = m_vlhSkillList[ i ];

				// Search and replace in the aura and toggle skill lists as well
				DNVector(DnSkillHandle)::iterator iterToggle = find( m_vlhToggleSkills.begin(), m_vlhToggleSkills.end(), hPrevLevelSkill );
				if( m_vlhToggleSkills.end() != iterToggle )
					*iterToggle = hNewSkill;

				DNVector(DnSkillHandle)::iterator iterAura = find( m_vlhAuraSkills.begin(), m_vlhAuraSkills.end(), hPrevLevelSkill );
				if( m_vlhAuraSkills.end() != iterAura )
					*iterAura = hNewSkill;
			}

			OnReplacementSkill( m_vlhSkillList[ i ], hNewSkill );
			m_vlhSkillList[ i ] = hNewSkill;

			return true;
		}
	}

	return false;
#else
	int iSkillID = hNewSkill->GetClassID();
	for( DWORD i = 0; i < m_vlSkillObjects.size(); i++ ) 
	{
		S_SKILL_OBJECT& SkillObject = m_vlSkillObjects.at( i );
		DnSkillHandle hSkill = SkillObject.hSkill;
		if( hSkill->GetClassID() == iSkillID ) 
		{
			// Is it a passive blow skill? If so, remove all existing state effects and apply new ones.
			DNVector(DnSkillHandle)::iterator iter = find( m_vlhSelfPassiveBlowSkill.begin(), m_vlhSelfPassiveBlowSkill.end(), hSkill );
			if( m_vlhSelfPassiveBlowSkill.end() != iter )
			{
				// Remove currently applied state effects.
				// Find and delete state effects matching the parent skill.
				if( !IsValidActor() ) continue;

				// When a passive skill that increases HP/SP is removed, current HP/SP reverts to base HP/SP without the skill
				// Since it gets clipped, fetch in advance and set it.
				INT64 iHP = m_pActor->GetHP();
				int iSP = m_pActor->GetSP();

				DNVector(DnBlowHandle) vlhAppliedSE;
				DNVector(DnBlowHandle) vlhSEToRemove;
				m_pActor->GetAllAppliedStateBlow( vlhAppliedSE );
				int iNumAppliedSE = (int)vlhAppliedSE.size();
				for( int iSE = 0; iSE < iNumAppliedSE; ++iSE )
				{
					DnBlowHandle hAppliedBlow = vlhAppliedSE.at( iSE );
					const CDnSkill::SkillInfo* pSkillInfo = hAppliedBlow->GetParentSkillInfo();
					if( pSkillInfo->iSkillID == (*iter)->GetClassID() )
						vlhSEToRemove.push_back( hAppliedBlow );
				}

				int iNumSEToRemove = (int)vlhSEToRemove.size();
				for( int iSE = 0; iSE < iNumSEToRemove; ++iSE )
				{
					DnBlowHandle hBlow = vlhSEToRemove.at(iSE);
					int iID = hBlow->GetBlowID();
					OutputDebug( "[Passive Buff Skill Removed: %d] %s StateEffectIndex: %d, Value:%s\n", 
								 (*iter)->GetClassID(), (*iter)->GetName(), hBlow->GetBlowIndex(), hBlow->GetValue() );
					m_pActor->RemoveStateBlowFromID( iID );
				}

				m_vlhSelfPassiveBlowSkill.erase( iter );

				ApplyPassiveSkill( hNewSkill );
				m_vlhSelfPassiveBlowSkill.push_back( hNewSkill );

				m_pActor->SetHP( iHP );
				m_pActor->SetSP( iSP );
			}

			if (m_hAuraSkill && m_hAuraSkill->GetClassID() == iSkillID && m_hAuraSkill->IsAuraOn())
			{
				if (m_hAuraSkill == m_hProcessSkill)
					m_hProcessSkill.Identity();

				m_hAuraSkill->EnableAura( false );
				OnSkillAura(m_hAuraSkill, false);
				m_hAuraSkill.Identity();
			}

			if( true == SkillObject.bSelfAlloc )
			{
				// Because self must delete the skill it created.
				DnSkillHandle hSkillToRelease = hSkill;

				// Search and replace in the aura and toggle skill lists as well
				DNVector(DnSkillHandle)::iterator iterToggle = find( m_vlhToggleSkills.begin(), m_vlhToggleSkills.end(), hSkillToRelease );
				if( m_vlhToggleSkills.end() != iterToggle )
					*iterToggle = hNewSkill;

				DNVector(DnSkillHandle)::iterator iterAura = find( m_vlhAuraSkills.begin(), m_vlhAuraSkills.end(), hSkillToRelease );
				if( m_vlhAuraSkills.end() != iterAura )
					*iterAura = hNewSkill;

				OnRemoveSkill( hSkillToRelease );
				SAFE_RELEASE_SPTR( hSkillToRelease );

			}
			else
			{
				// Do not delete skill objects created by the skill task here.
				DnSkillHandle hPrevLevelSkill = hSkill;

				// Search and replace in the aura and toggle skill lists as well
				DNVector(DnSkillHandle)::iterator iterToggle = find( m_vlhToggleSkills.begin(), m_vlhToggleSkills.end(), hPrevLevelSkill );
				if( m_vlhToggleSkills.end() != iterToggle )
					*iterToggle = hNewSkill;

				DNVector(DnSkillHandle)::iterator iterAura = find( m_vlhAuraSkills.begin(), m_vlhAuraSkills.end(), hPrevLevelSkill );
				if( m_vlhAuraSkills.end() != iterAura )
					*iterAura = hNewSkill;
			}

			OnReplacementSkill( hSkill, hNewSkill );
			SkillObject.hSkill = hNewSkill;

			return true;
		}
	}

	return false;
#endif // #ifndef PRE_FIX_SKILLLIST
}
#endif

bool MASkillUser::IsExistSkill( int nSkillTableID, int nLevel )
{
#ifndef PRE_FIX_SKILLLIST
	for( DWORD i=0; i<m_vlhSkillList.size(); i++ ) {
		if( m_vlhSkillList[i] && ( m_vlhSkillList[i]->GetClassID() == nSkillTableID ) ) 
		{
			if( nLevel == -1 ) return true;
			if( m_vlhSkillList[i]->GetLevel() == nLevel ) return true;
		}
	}
	return false;
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		S_SKILL_OBJECT& SkillObject = m_vlSkillObjects.at( i );
		DnSkillHandle hSkill = SkillObject.hSkill;
		if( hSkill)
		{
			if( hSkill->GetClassID() == nSkillTableID )
			{
				if( -1 == nLevel )
					return true;

				if( hSkill->GetLevel() == nLevel )
					return true;
			}
		}
	}

	return false;
#endif // #ifndef PRE_FIX_SKILLLIST
}

bool MASkillUser::IsSelfAllocSkill( int nSkillTableID )
{
#ifndef PRE_FIX_SKILLLIST
	for( DWORD i=0; i<m_vlhSkillList.size(); i++ ) {
		if( m_vlhSkillList[i]->GetClassID() == nSkillTableID ) 
			return m_vbSelfAllocList[i];
	}
	return false;
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		if( m_vlSkillObjects.at( i ).hSkill->GetClassID() == nSkillTableID )
			return m_vlSkillObjects.at( i ).bSelfAlloc;
	}
	return false;
#endif // #ifndef PRE_FIX_SKILLLIST
}

void MASkillUser::SetSkillLevel( int nSkillTableID, int nValue )
{
	if( !IsSelfAllocSkill( nSkillTableID ) ) return;

	DnSkillHandle hSkill = FindSkill( nSkillTableID );
	if( !hSkill ) return;
	if( hSkill->GetLevel() == nValue ) return;

	float fElapsedDelayTime = hSkill->GetElapsedDelayTime();
	RemoveSkill( nSkillTableID );
	AddSkill( nSkillTableID, nValue );
	hSkill = FindSkill( nSkillTableID );
	hSkill->SetElapsedDelayTime( fElapsedDelayTime );
}

DnSkillHandle MASkillUser::FindSkill( int nSkillTableID )
{
#ifndef PRE_FIX_SKILLLIST
	for( DWORD i=0; i<m_vlhSkillList.size(); i++ ) 
	{
		if( m_vlhSkillList[i]->GetClassID() == nSkillTableID ) 
			return m_vlhSkillList[i];
	}
	return CDnSkill::Identity();
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		if( m_vlSkillObjects.at( i ).hSkill &&
			m_vlSkillObjects.at( i ).hSkill->GetClassID() == nSkillTableID )
			return m_vlSkillObjects.at( i ).hSkill;
	}

	return CDnSkill::Identity();
#endif // #ifndef PRE_FIX_SKILLLIST
}

DnSkillHandle MASkillUser::GetProcessSkill()
{
	return m_hProcessSkill;
}


CDnSkill::UsingResult MASkillUser::CanExecuteSkill( DnSkillHandle hSkill )
{
	_ASSERT( hSkill && "MASkillUser::CanExecuteSkill() -> Invalid hSkill" );

#ifndef _GAMESERVER
	// Cannot use multiple active skills in the same frame.
	if( m_hProcessSkill && 
		m_hProcessSkill->GetSkillType() == CDnSkill::Active && 
		hSkill->GetSkillType() == CDnSkill::Active )
		if( m_bSkillExecutedThisFrame )
			return CDnSkill::UsingResult::Failed;
#endif


// [2011/02/22 semozz]
// Only the client needs to check if animation transition is possible...
#if !defined(_GAMESERVER)
	if (!hSkill->CheckAnimation())
		return CDnSkill::UsingResult::Failed;
#endif // _GAMESERVER

#if !defined(_GAMESERVER)
	if( hSkill->GetGlyphActiveSkillDisable() )
		return CDnSkill::UsingResult::Failed;
#endif	// _GAMESERVER

	return hSkill->CanExecute();
}


#ifdef _GAMESERVER
bool MASkillUser::ExecuteSkill( DnSkillHandle hSkill, LOCAL_TIME LocalTime, float fDelta )
#else
bool MASkillUser::ExecuteSkill( DnSkillHandle hSkill, LOCAL_TIME LocalTime, float fDelta, 
							    bool bCheckValid /*= true*/, bool bAutoUsedFromServer /*= false*/, bool bSendPacketToServer /*= true */ )
#endif
{
	_ASSERT( hSkill && "Invalid hSkill" );

	if( !IsValidActor() ) return false;

	if( m_hProcessSkill )
	{
		// #25154 Aura skills trigger OnEnd when the aura is turned off.
		if( false == m_hProcessSkill->IsAuraOn() )
			m_hProcessSkill->OnEnd( m_LocalTime, 0 );

		m_hProcessSkill.Identity();
	}

	m_hProcessSkill = hSkill;

	switch( m_hProcessSkill->GetDurationType() ) {
		case CDnSkill::Instantly:
		case CDnSkill::Buff:
		case CDnSkill::Debuff:
		case CDnSkill::SummonOnOff:
		case CDnSkill::StanceChange:
			// Prevent SP consumption in town
#ifndef _GAMESERVER
			if( CDnWorld::MapTypeVillage != CDnWorld::GetInstance().GetMapType() )
#endif
				m_pActor->UseMP( -m_hProcessSkill->GetDecreaseMP() );			// Compile Error. Moved by kalliste
				//m_pActor->SetSP( m_pActor->GetSP() - m_hProcessSkill->GetDecreaseMP() );
			
			// The Excute function was renamed to OnBegin.
			//m_hProcessSkill->Execute( LocalTime, fDelta );
			m_hProcessSkill->OnBegin( LocalTime, fDelta );

			// If global cooldown is set, start cooldown for skills in the same group as well..
			if( m_hProcessSkill && 0 < m_hProcessSkill->GetGlobalSkillGroupID() )
			{
				for( DWORD i = 0; i < GetSkillCount(); ++i )
				{
					// Because the values of m_vlGroupedSkillIDs are simply pulled from the table
					// Must verify whether the skill has actually been learned.
					DnSkillHandle hExistSkill = GetSkillFromIndex( i );
					if( hExistSkill && 
						hExistSkill != m_hProcessSkill &&
						hExistSkill->GetGlobalSkillGroupID() == m_hProcessSkill->GetGlobalSkillGroupID() )
					{
						// For skills sharing a global cooldown, replace their cooldown with the used skill's cooldown.
						hExistSkill->ResetCoolTime();
						hExistSkill->OnAnotherGlobalSkillBeginCoolTime( hSkill );
						hExistSkill->OnBeginCoolTime();
					}
				}
			}
			break;

		case CDnSkill::TimeToggle:
		case CDnSkill::ActiveToggle:
		case CDnSkill::ActiveToggleForSummon:
			{
				bool bToggleEnable = !hSkill->IsToggleOn();
				hSkill->EnableToggle(bToggleEnable);
				OnSkillToggle(hSkill, bToggleEnable);

#ifdef _GAMESERVER
				// NOTE: Stores the attack power at the time the toggle is turned on. Other stats besides attack power may be added later.
				if( hSkill && hSkill->IsToggleOn() )
				{
					m_ActorStateSnapshotForToggleProjectile.ResetState();
					hSkill->CheckAndAddSelfStateEffect();
					static_cast<CDnActor*>(this)->GetStateBlow()->Process( 0, 0.0f );
					CDnActor* pActor = static_cast<CDnActor*>(this);

					m_ActorStateSnapshotForToggleProjectile = *(static_cast<CDnState*>(pActor));
					hSkill->CheckAndRemoveInstantApplySelfStateEffect();
				}
#endif
			}
			break;;

		case CDnSkill::Aura:
			{
				// Toggle between aura skills: turns off if another aura skill is enabled, or turns off if it is the currently active aura skill.
				// Must stop existing aura skills first before running the newly enabled aura so items queued in self blow for the new aura... 
				// Not cleared and applied properly.
				bool bProcessingAuraOff = false;		// Executing the same aura skill turns the aura off.
				int iNumAuraSkill = (int)m_vlhAuraSkills.size();
				for( int iAuraSkill = 0; iAuraSkill < iNumAuraSkill; ++iAuraSkill )
				{
					DnSkillHandle hAuraSkill = m_vlhAuraSkills.at( iAuraSkill );
					_ASSERT( hAuraSkill->GetDurationType() == CDnSkill::Aura );
					if( hAuraSkill->IsAuraOn() )
					{
						if( m_hProcessSkill == hAuraSkill )
							bProcessingAuraOff = true;

						hAuraSkill->EnableAura( false );
						OnSkillAura( hAuraSkill, false );
						break;
					}
				}

				// Using another aura skill turned off the existing aura and activated the new one.
				if( false == bProcessingAuraOff )
				{
					for( int iAuraSkill = 0; iAuraSkill < iNumAuraSkill; ++iAuraSkill )
					{
						DnSkillHandle hAuraSkill = m_vlhAuraSkills.at( iAuraSkill );
						if( hAuraSkill == m_hProcessSkill )
						{
							m_hProcessSkill->EnableAura( true );
							OnSkillAura( m_hProcessSkill, true );
							break;

							// Note Hanki: m_hProcessSkill smart pointer must remain valid while the aura skill action is playing
							// Filtered in CDnPlayerActor::CmdStop() on the game server, so on the game server the action signal to the end 
							// Processed. Therefore, set ProcessSkill to Identity in CDnActor::OnChangeAction.
						}
					}
				}
				else
				{
					// Active aura turned off.
					m_hAuraSkill.Identity();
					m_hProcessSkill.Identity();
				}
			}
			break;
	}

#ifndef _GAMESERVER
	m_bSkillExecutedThisFrame = true;
#endif

	return true;
}

#ifdef _GAMESERVER
void MASkillUser::UseAutoPassiveSkill( LOCAL_TIME LocalTime, float fDelta )
{
	list<DnSkillHandle>::iterator iter = m_listAutoPassiveSkills.begin();
	for( iter; m_listAutoPassiveSkills.end() != iter; ++iter )
	{
		DnSkillHandle hSkill = *iter;
		if( CDnSkill::UsingResult::Success == CanExecuteSkill( hSkill ) )
		{
			BYTE pBuffer[128] = {0,};
			CPacketCompressStream Stream( pBuffer, 128 );
			int iSkillID = hSkill->GetClassID();
			int iEnchantSkillID = hSkill->GetEnchantedSkillID();
			char cLevel = hSkill->GetLevel();
			EtVector2 vLook, vZVec;
			bool bUseApplySkillItem = false;
			char cUseSignalSkill = -1;
			bool bAutoUseFromServer = true;
			bool abSignalSkillCheck[ 3 ] = { false };

			vLook = EtVec3toVec2( *m_pActor->GetLookDir() );
			vZVec = EtVec3toVec2( *m_pActor->GetMoveVectorZ() );

			Stream.Write( &iSkillID, sizeof(int) );
			Stream.Write( &cLevel, sizeof(char) );
			Stream.Write( &bUseApplySkillItem, sizeof(bool) );
			Stream.Write( abSignalSkillCheck, sizeof(abSignalSkillCheck) );
			Stream.Write( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
			Stream.Write( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
			Stream.Write( &bAutoUseFromServer, sizeof(bool) );
			Stream.Write( &iEnchantSkillID, sizeof(int) );
#ifdef PRE_ADD_POSITION_SYNC_BY_SKILL_USAGE
			Stream.Write( m_pActor->GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
#endif
			
			m_pActor->Send( eActor::SC_USESKILL, &Stream );

			ExecuteSkill( hSkill, LocalTime, fDelta );

			// Add to the activated list, because process needs to be run on it,
			m_listProcessAutoPassiveSkills.push_back( hSkill );

			// Append used auto-passive skills to the end. Lowers execution priority.
			m_listAutoPassiveSkills.erase( iter );
			m_listAutoPassiveSkills.push_back( hSkill );

			// Initialize since it is not ProcessSkill. If ProcessSkill is present, CmdStop does not work.
			m_hProcessSkill.Identity();
			break;
		}
	}
}


bool MASkillUser::IsProcessingAutoPassive( int iSkillID )
{
	list<DnSkillHandle>::iterator iter = m_listProcessAutoPassiveSkills.begin();
	for( iter; iter != m_listProcessAutoPassiveSkills.end(); ++iter )
	{
		DnSkillHandle hAutoPassive = *iter;
		if( hAutoPassive->GetClassID() == iSkillID )
			return true;
	}

	return false;
}

void MASkillUser::EndAutoPassiveSkill( LOCAL_TIME LocalTime, float fDelta )
{
	list<DnSkillHandle>::iterator iter = m_listProcessAutoPassiveSkills.begin();
	for( iter; iter != m_listProcessAutoPassiveSkills.end(); ++iter )
	{
		DnSkillHandle hAutoPassive = *iter;
		hAutoPassive->OnEnd( LocalTime, fDelta );
	}
}
#endif


void MASkillUser::OnSkillProcess( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_hAuraSkill )
	{
		//Judge only on the game server to terminate server-side / send packet to client...
#if defined(_GAMESERVER)
		if( m_hAuraSkill->IsFinished() )
		{
			if (m_pActor)
				m_pActor->CmdFinishAuraSkill(m_hAuraSkill->GetClassID());
			
			// [2011/03/22 semozz]
			// If the current skill is the same as the aura skill, reset the current skill as well.
			if (m_hProcessSkill == m_hAuraSkill)
				m_hProcessSkill.Identity();

			OnSkillAura( m_hAuraSkill, false );
		}
		else
#endif // _GAMESERVER
			m_hAuraSkill->ProcessExecute( LocalTime, fDelta );

	}

#ifdef _GAMESERVER
	if( m_pActor && m_pActor->IsPlayerActor() )
	{
		list<DnSkillHandle>::iterator iter = m_listProcessAutoPassiveSkills.begin();
		for( iter; m_listProcessAutoPassiveSkills.end() != iter;  )
		{
			DnSkillHandle hSkill = *iter;

			//Add skill handle validity check
			if (!hSkill)
			{
				iter = m_listProcessAutoPassiveSkills.erase( iter );
			}
			else
			{
				if( hSkill->IsFinished() )
				{
					iter = m_listProcessAutoPassiveSkills.erase( iter );
				}
				else
				{
					hSkill->ProcessExecute( LocalTime, fDelta );
					++iter;
				}
			}
		}

		m_PrefixSkillCoolTimeManager.Process(LocalTime, fDelta);

		iter = m_listProcessPreFixDefenceSkills.begin();
		for( iter; m_listProcessPreFixDefenceSkills.end() != iter;  )
		{
			DnSkillHandle hSkill = *iter;

			//Add skill handle validity check
			if (!hSkill)
			{
				iter = m_listProcessPreFixDefenceSkills.erase( iter );
			}
			else
			{
				if( hSkill->IsFinished() )
				{
					hSkill->OnEnd( LocalTime, fDelta );
					iter = m_listProcessPreFixDefenceSkills.erase( iter );
				}
				else
				{
					hSkill->ProcessExecute( LocalTime, fDelta );
					++iter;
				}
			}
		}

		iter = m_listProcessPreFixOffenceSkills.begin();
		for( iter; m_listProcessPreFixOffenceSkills.end() != iter;  )
		{
			DnSkillHandle hSkill = *iter;
			if (!hSkill)
			{
				iter = m_listProcessPreFixOffenceSkills.erase( iter );
			}
			else
			{
				if( hSkill->IsFinished() )
				{
					hSkill->OnEnd( LocalTime, fDelta );
					iter = m_listProcessPreFixOffenceSkills.erase( iter );
				}
				else
				{
					hSkill->ProcessExecute( LocalTime, fDelta );
					++iter;
				}
			}
		}

		if (!m_prefixSystemDefenceSkills.empty())
		{
			PREFIX_SYSTEM_SKILL_LIST::iterator iter = m_prefixSystemDefenceSkills.begin();
			PREFIX_SYSTEM_SKILL_LIST::iterator enditer = m_prefixSystemDefenceSkills.end();

			for (; iter != enditer; ++iter)
			{
				if (iter->second.hSkill)
					iter->second.hSkill->Process(LocalTime, fDelta);
			}
		}

		if (!m_prefixSystemOffenceSkills.empty())
		{
			PREFIX_SYSTEM_SKILL_LIST::iterator iter = m_prefixSystemOffenceSkills.begin();
			PREFIX_SYSTEM_SKILL_LIST::iterator enditer = m_prefixSystemOffenceSkills.end();

			for (; iter != enditer; ++iter)
			{
				if (iter->second.hSkill)
					iter->second.hSkill->Process(LocalTime, fDelta);
			}
		}

	}
#else
	m_bSkillExecutedThisFrame = false;
#endif
	
	if( m_hProcessSkill ) 
	{
		if( m_hProcessSkill->IsFinished() )
		{
			m_hProcessSkill->OnEnd( LocalTime, fDelta );
			m_hProcessSkill.Identity();
		}
		else
			m_hProcessSkill->ProcessExecute( LocalTime, fDelta );
	}

	// If cooldown reduction state effect exists, calculate value affecting delta.
	if( m_pActor && m_pActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_096 ) )
	{
		DNVector(DnBlowHandle) vlhCoolTimeBlows;
		m_pActor->GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_096, vlhCoolTimeBlows );

		// Cooldown reduction state effect is negative-based, so subtract when stacked.
		float fCoolTimeDeltaAdjust = 1.0f;
		int iNumBlow = (int)vlhCoolTimeBlows.size();
		for( int i = 0; i < iNumBlow; ++i )
		{
			fCoolTimeDeltaAdjust -= (1.0f - vlhCoolTimeBlows.at( i )->GetFloatValue());
		}

		// #40670 Spirit Boost now also uses the cooldown reduction state effect.
		// Restricted by WeightTable when stacked.
		float fCoolTimeAccelMax = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CoolTimeAccellSE_Max );
		if( fCoolTimeDeltaAdjust < 1.0f - fCoolTimeAccelMax )
			fCoolTimeDeltaAdjust = 1.0f - fCoolTimeAccelMax;

		// Since 1.0f is the normal baseline and the state effect factor applied directly to total cooldown is the current final cooldown baseline,
		// Multiply the per-frame delta value by the reciprocal of the ratio.
		// The resulting value is fetched from the actor and applied directly when looping in each skill, except skill 2415 Time Acceleration.
		m_fCoolTimeDeltaAdjustValue = 1.0f / fCoolTimeDeltaAdjust;
	}
	else
		m_fCoolTimeDeltaAdjustValue = 1.0f;
	


#ifdef _GAMESERVER
#ifndef PRE_FIX_SKILLLIST

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
	// Cooldown processing. In the future this will handle not only cooldowns but various other continuous processing.
	if( m_pActor && m_pActor->IsPlayerActor() /*&& m_pActor->IsAllowCallSkillProcess( fDelta )*/ )
	{
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

		for( DWORD i = 0; i < m_vlhSkillList.size(); i++ ) 
		{
			if( m_vlhSkillList.at( i ) )
				m_vlhSkillList[i]->Process( LocalTime, fDelta );
		}

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
	}
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

#else

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
	if( m_pActor && m_pActor->IsPlayerActor() /*&& m_pActor->IsAllowCallSkillProcess( fDelta )*/ )
	{
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

		PROFILE_TIME_TEST_BLOCK_START( "OnSkillProcess() - SkillHandleLoop" );
		// Cooldown processing. In the future this will handle not only cooldowns but various other continuous processing.
		for( DWORD i = 0; i < m_vlSkillObjects.size(); i++ ) 
		{
			// On client, SkillTask managing skill objects may be destroyed first, so validity check is required.
			if( m_vlSkillObjects.at( i ).hSkill )
				m_vlSkillObjects.at( i ).hSkill->Process( LocalTime, fDelta );
		}
		PROFILE_TIME_TEST_BLOCK_END();

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
	}
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

#endif // #ifndef PRE_FIX_SKILLLIST
#else
#ifndef PRE_FIX_SKILLLIST
	for( DWORD i = 0; i < m_vlhSkillList.size(); i++ ) 
	{
		if( m_vlhSkillList.at( i ) )
			m_vlhSkillList[i]->Process( LocalTime, fDelta );
	}
#else
	// Cooldown processing. In the future this will handle not only cooldowns but various other continuous processing.
	for( DWORD i = 0; i < m_vlSkillObjects.size(); i++ ) 
	{
		// On client, SkillTask managing skill objects may be destroyed first, so validity check is required.
		if( m_vlSkillObjects.at( i ).hSkill )
			m_vlSkillObjects.at( i ).hSkill->Process( LocalTime, fDelta );
	}
#endif // #ifndef PRE_FIX_SKILLLIST
#endif // #ifdef _GAMESERVER

	// Process if there is a skill attached to the item
	if( m_hItemSkill )
	{
		if( m_hItemSkill->IsFinished() )
		{
			m_hItemSkill->OnEnd( LocalTime, fDelta );

			deque<DnSkillHandle>::iterator iter = find( m_dqhItemSkillList.begin(), m_dqhItemSkillList.end(), m_hItemSkill );
			_ASSERT( m_dqhItemSkillList.end() != iter );
			m_dqhItemSkillList.erase( iter );

			SAFE_RELEASE_SPTR( m_hItemSkill );
		}
		else
			m_hItemSkill->ProcessExecute( LocalTime, fDelta );
	}

	for( DWORD i = 0; i < m_dqhItemSkillList.size(); ++i )
	{
		m_dqhItemSkillList.at( i )->Process( LocalTime, fDelta );
	}

	// Terminate skills scheduled for termination.
	int iNumReservedFinishSkill = (int)m_vlhReservedFinishSkill.size();
	for( int iSkill = 0; iSkill < iNumReservedFinishSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhReservedFinishSkill.at( iSkill );
		if( hSkill == m_hProcessSkill )
		{
			m_hProcessSkill->OnEnd( LocalTime, fDelta );
			m_hProcessSkill.Identity();
		}
		else
		if( hSkill == m_hToggleSkill )
		{
			_ASSERT( m_hToggleSkill->IsToggleOn() );
			if( m_hToggleSkill->IsToggleOn() )
				OnSkillToggle( m_hToggleSkill, false );
		}
		else
		if( hSkill == m_hAuraSkill )
		{
			_ASSERT( m_hAuraSkill->IsAuraOn() );
			if( m_hAuraSkill->IsAuraOn() )
				OnSkillAura( m_hAuraSkill, false );
		}
	}
	
	if( false == m_vlhReservedFinishSkill.empty() )
		m_vlhReservedFinishSkill.clear();
}

// This function is always called whenever Projectile is triggered in OnSignal for monsters, players, or weapons!!
void MASkillUser::OnSkillProjectile( CDnProjectile *pProjectile )
{
	if( !IsValidActor() ) 
		return;

	// Actually changes the visual appearance of the Projectile.
	bool bClientSide = false;

#if defined( _GAMESERVER )
	if( m_pActor->GetClassID() <= CDnActor::Reserved6 && m_pActor->IsPlayerActor() && ((CDnPlayerActor*)m_pActor)->IsLocalActor() )
		bClientSide = true;
#endif // #if defined( _GAMESERVER )
	
	if( m_hToggleSkill && m_hToggleSkill->IsToggleOn() )
	{
		// Handles skills whose toggle is in the state to be enabled.
		_ASSERT( m_hToggleSkill->IsToggleOn() );

		CDnChangeProjectileProcessor* pChangeProj = static_cast<CDnChangeProjectileProcessor*>(m_hToggleSkill->GetProcessor( IDnSkillProcessor::CHANGE_PROJECTILE ));
		if( pChangeProj )
		{
			int iCheckProjectileID = ( bClientSide ) ? pChangeProj->GetSourceWeaponID() : pChangeProj->GetChangeWeaponID();
			if( pProjectile->GetClassID() == iCheckProjectileID )
			{
				if( m_hToggleSkill->GetDurationType() == CDnSkill::ActiveToggle )
				{
					if( !(m_hToggleSkill->GetDecreaseMP() > 0 && m_pActor->GetSP() < m_hToggleSkill->GetDecreaseMP()) )
					{
						// Prevent SP consumption in town
#ifndef _GAMESERVER
						if( CDnWorld::MapTypeVillage != CDnWorld::GetInstance().GetMapType() )
#endif
							m_pActor->UseMP( -m_hToggleSkill->GetDecreaseMP() );
							//m_pActor->SetSP( m_pActor->GetSP() - m_hToggleSkill->GetDecreaseMP() );

						if( bClientSide )
						{
							int iWeaponLength = pProjectile->GetWeaponLength() - pProjectile->GetOriginalWeaponLength();
							*(CDnWeapon*)pProjectile = *pChangeProj->GetProjectile();
							pProjectile->SetWeaponLength( iWeaponLength + pProjectile->GetOriginalWeaponLength() );
						}

						OnSkillToggleProjectile( m_hToggleSkill, pProjectile );

						if( m_pActor->GetSP() < m_hToggleSkill->GetDecreaseMP() ) 
						{	
							OnSkillToggle( m_hToggleSkill, false );
						}
					}
				}
			}
		}
		else
		{
			// There is ChangeAction added by Jiho
			CDnChangeActionStrProcessor* pChangeAction = static_cast<CDnChangeActionStrProcessor*>(m_hToggleSkill->GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR ));
			if( pChangeAction )
			{
				if( m_hToggleSkill->GetDurationType() == CDnSkill::ActiveToggle )
				{
					if( !(m_hToggleSkill->GetDecreaseMP() > 0 && m_pActor->GetSP() < m_hToggleSkill->GetDecreaseMP()) )
					{
						// Prevent SP consumption in town
#ifndef _GAMESERVER
						if( CDnWorld::MapTypeVillage != CDnWorld::GetInstance().GetMapType() )
#endif
							m_pActor->UseMP( -m_hToggleSkill->GetDecreaseMP() );
							//m_pActor->SetSP( m_pActor->GetSP() - m_hToggleSkill->GetDecreaseMP() );

						OnSkillToggleProjectile( m_hToggleSkill, pProjectile );

						if( m_pActor->GetSP() < m_hToggleSkill->GetDecreaseMP() ) 
						{	
							OnSkillToggle( m_hToggleSkill, false );
						}
					}
				}
			}

		}
	}

#ifdef _GAMESERVER
	if( pProjectile && !bClientSide )
	{
		// Note Hangi: Changed to carry ActorState on the Projectile for direct damage calculation.
		// For non-toggle skills that fire projectiles, apply attack power here
		if( !m_hToggleSkill && m_hProcessSkill )
		{
			static_cast<CDnActor*>(this)->GetStateBlow()->Process( 0, 0.0f );
			CDnActor* pActor = static_cast<CDnActor*>(this);

			// Projectiles using chain skill state effect 
			// Use shaared_ptr because CDnState copying can occur very frequently.
			boost::shared_ptr<CDnState> pActorStateSnapshot = boost::shared_ptr<CDnState>(new CDnState);
			*pActorStateSnapshot = *(static_cast<CDnState*>(pActor));

			// Only when it is a skill, handle here 
			pProjectile->FromSkill( true );
			pProjectile->SetShooterStateSnapshot( pActorStateSnapshot );


#if defined(PRE_FIX_65287)
			float fFinalDamageRate = 0.0f;
			if (pActor && pActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_050))
			{
				DNVector(DnBlowHandle) vlhBlows;
				pActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_050, vlhBlows );
				int iNumBlow = (int)vlhBlows.size();
				for( int i = 0; i < iNumBlow; ++i )
				{
					fFinalDamageRate += vlhBlows[i]->GetFloatValue();
				}
			}

			pProjectile->SetShooterFinalDamageRate(fFinalDamageRate);
#endif // PRE_FIX_65287
		}
		else if( m_hToggleSkill && m_hToggleSkill->IsToggleOn() )
		{
			// Toggle skills are not currently used, but keeping the code for future use. 2011.03.15
			// Set the projectile with the stats from when the toggle was turned on.
			boost::shared_ptr<CDnState> pActorStatesnapshot = boost::shared_ptr<CDnState>( new CDnState );
			*pActorStatesnapshot = m_ActorStateSnapshotForToggleProjectile;
			
			pProjectile->SetShooterStateSnapshot( pActorStatesnapshot );
			pProjectile->FromSkill( true );

#if defined(PRE_FIX_65287)
			CDnActor* pActor = static_cast<CDnActor*>(this);

			float fFinalDamageRate = 0.0f;
			if (pActor && pActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_050))
			{
				DNVector(DnBlowHandle) vlhBlows;
				pActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_050, vlhBlows );
				int iNumBlow = (int)vlhBlows.size();
				for( int i = 0; i < iNumBlow; ++i )
				{
					fFinalDamageRate += vlhBlows[i]->GetFloatValue();
				}
			}

			pProjectile->SetShooterFinalDamageRate(fFinalDamageRate);
#endif // PRE_FIX_65287
		}
		else if( m_pActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_121 ) || m_pActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_129 ) )
		{
			// #30362 Projectiles fired during a stand action change state effect are also treated as skill projectiles.
			pProjectile->FromSkill( true );
		}
	}
#endif

}

void MASkillUser::OnSkillToggleProjectile( DnSkillHandle hSkill, CDnProjectile *pProjectile )
{
}

void MASkillUser::OnSkillToggle( DnSkillHandle hSkill, bool bEnable )
{
	if( !IsValidActor() ) return;

	if( bEnable )
	{
		switch( hSkill->GetDurationType() )
		{
			case CDnSkill::ActiveToggle:
			case CDnSkill::ActiveToggleForSummon:
				// Prevent SP consumption in town
#ifndef _GAMESERVER
				if( CDnWorld::MapTypeVillage != CDnWorld::GetInstance().GetMapType() )
#endif
					// Not consumed when toggling; consumed when used
					//m_pActor->SetSP( m_pActor->GetSP() - hSkill->GetDecreaseSP() );
				hSkill->OnBegin( m_LocalTime, 0.0f );
				break;

			case CDnSkill::TimeToggle:
				hSkill->OnBegin( m_LocalTime, 0.0f );
				break;
		}

		SkillToggle(hSkill, true);
	}
	else
	{
		SkillToggle(hSkill, false);

		hSkill->OnEnd( m_LocalTime, 0.0f );
		hSkill->EnableToggle( false );
	}

	//if( bEnable ) {
	//	switch( hSkill->GetDurationType() ) {
	//		case CDnSkillBase::TimeToggle:
	//			pActor->SetSP( pActor->GetSP() - hSkill->GetDecreaseSP() );
	//			hSkill->Execute( m_LocalTime, 0.f );
	//			break;
	//		case CDnSkillBase::ActiveToggle:
	//			hSkill->Execute( m_LocalTime, 0.f );
	//			break;
	//		case CDnSkillBase::Aura:
	//			pActor->SetSP( pActor->GetSP() - hSkill->GetDecreaseSP() );
	//			hSkill->Execute( m_LocalTime, 0.f );
	//			break;
	//	}
	//}
	//else {
	//	hSkill->Finish( m_LocalTime, 0.f );
	//}
}


void MASkillUser::OnSkillAura( DnSkillHandle hSkill, bool bEnable )
{
	if( !IsValidActor() ) return;

	if( bEnable )
	{
		switch( hSkill->GetDurationType() )
		{
			case CDnSkill::Aura:
				// Prevent SP consumption in town
#ifndef _GAMESERVER
				if( CDnWorld::MapTypeVillage != CDnWorld::GetInstance().GetMapType() )
#endif
					m_pActor->UseMP( -hSkill->GetDecreaseMP() );
					//m_pActor->SetSP( m_pActor->GetSP() - hSkill->GetDecreaseMP() );
				hSkill->OnBegin( m_LocalTime, 0.0f );
				break;
		}

		m_hAuraSkill = hSkill;
	}
	else
	{
		hSkill->OnEnd( m_LocalTime, 0.0f );
		hSkill->EnableAura( false );

		if( m_hAuraSkill == hSkill )
			m_hAuraSkill.Identity();
	}
}

void MASkillUser::ResetToggleSkill()
{
	//if( m_hToggleSkill )
	//	m_hToggleSkill.Identity();

	//int iNumToggle = (int)m_vlhToggleSkills.size();
	//for( int i = 0; i < iNumToggle; ++i )
	//{
	//	if( m_vlhToggleSkills.at(i)->IsToggleOn() )
	//		OnSkillToggle( m_vlhToggleSkills.at(i), false );
	//}
}

void MASkillUser::ResetAuraSkill()
{
	//if( m_hAuraSkill )
	//	m_hAuraSkill.Identity();

	//int iNumAura = (int)m_vlhAuraSkills.size();
	//for( int i = 0; i < iNumAura; ++i )
	//{
	//	if( m_vlhAuraSkills.at(i)->IsAuraOn() )
	//		OnSkillAura( m_vlhAuraSkills.at(i), false );
	//}
}


bool MASkillUser::IsProcessSkill()
{
	if( !m_hProcessSkill ) return false;
	return !m_hProcessSkill->IsFinished();

	return true;
}

DWORD MASkillUser::GetSkillCount()
{
#ifndef PRE_FIX_SKILLLIST
	return (DWORD)m_vlhSkillList.size();
#else
	return (DWORD)m_vlSkillObjects.size();
#endif // #ifndef PRE_FIX_SKILLLIST
}

DnSkillHandle MASkillUser::GetSkillFromIndex( DWORD dwIndex )
{
#ifndef PRE_FIX_SKILLLIST
	if( dwIndex < 0 || dwIndex >= m_vlhSkillList.size() ) return CDnSkill::Identity();
	return m_vlhSkillList[dwIndex];
#else
	if( dwIndex < 0 || dwIndex >= m_vlSkillObjects.size() ) 
		return CDnSkill::Identity();
		
	return m_vlSkillObjects.at( dwIndex ).hSkill;
#endif // #ifndef PRE_FIX_SKILLLIST
}

CDnSkill::UsingResult MASkillUser::UseSkill( int nSkillTableID, bool bCheckValid, bool bAutoUseFromServer/* = false*/, int nLuaSkillIndex/*=-1*/ )
{
	CDnSkill::UsingResult eResult = CDnSkill::UsingResult::Failed;

	DnSkillHandle hSkill = FindSkill( nSkillTableID );
	if( hSkill ) 
	{
		if( m_pActor && m_pActor->IsPlayerActor() )
		{
			if( hSkill->GetSkillType() == CDnSkill::SkillTypeEnum::EnchantPassive )
				return CDnSkill::UsingResult::Hack;
		}

#ifdef _GAMESERVER
		// Allows a 0.5s margin to account for server cooldown error, though
		// Because zone transitions or other new cases can cause a drift of 0.5 seconds or more from the client
		// Record the last skill usage timestamp; if interval is greater than the skill cooltime specified in data 
		// Reset the cooldown stored in the server's skill object. (#19737)
		hSkill->UpdateSkillCoolTimeExactly();
#endif

		eResult = CanExecuteSkill( hSkill );
		if( !bCheckValid || CDnSkill::UsingResult::Success == eResult )
		{
#if defined(_CLIENT)
			//Even if skill use succeeds, if a summoned monster exists, only unsummon it without actually using the skill.
			if (hSkill->GetDurationType() == CDnSkill::DurationTypeEnum::SummonOnOff)
			{
				if (hSkill->SummonMonsterOff())
					return eResult;
			}
#endif // _CLIENT
#ifdef _GAMESERVER
			ExecuteSkill( hSkill, m_LocalTime, 0.f );
#else
			ExecuteSkill( hSkill, m_LocalTime, 0.f, bCheckValid, bAutoUseFromServer );
#endif
		}
#if defined(_CLIENT)
		else
		{
			// Unsummon the summoned monster if one exists, even if skill use failed.
			if (hSkill->GetDurationType() == CDnSkill::DurationTypeEnum::SummonOnOff)
				hSkill->SummonMonsterOff();
		}
#endif // _CLIENT
	}

	return eResult;
}

bool MASkillUser::UseItemSkill( int nSkillTableID, int nSkillLevelTableID, CDnItem::ItemSkillApplyType ItemSkillApplyType, int nItemID/* = -1*/ )
{
	if( !IsValidActor() ) return false;
	bool bResult = true;

	switch( ItemSkillApplyType )
	{
		case CDnItem::ApplySkill:
			{
				// Item skills created here are checked for finish in OnProcessSkill and automatically removed from the list.
				DnSkillHandle hSkill = CDnSkill::CreateSkill( m_pActor->GetMySmartPtr(), nSkillTableID, nSkillLevelTableID );

				if( !hSkill ) {
					bResult = false;
					break;
				}
					
				// Set to the skill attached to the item.
				hSkill->AsItemSkill();
				
#if defined(_GAMESERVER)
				hSkill->SetItemID(nItemID);
#endif // _GAMESERVER

				m_dqhItemSkillList.push_back( hSkill );

				ExecuteSkill( hSkill, m_LocalTime, 0.f );
				m_hItemSkill = hSkill;
			}
			break;

			// Scheduled for deletion.
		case CDnItem::ApplyStateBlow:
			{
				//// For Instant items coming here, CDnItemTask creates the item and immediately deletes it.
				//DnSkillHandle hSkill = CDnSkill::CreateSkill( m_pActor->GetMySmartPtr(), nSkillTableID, nSkillLevelTableID );
				//
				//if( !hSkill )
				//	bResult = false;

				//hSkill->AsItemSkill();

				//// Function to forcibly synchronize state effects.
				//hSkill->OnBeginForceSync( m_LocalTime, 0.0f );
				//hSkill->Release();
			}
			break;
	}

	return bResult;
}

void MASkillUser::ResetSkillCoolTime( void )
{
#ifndef PRE_FIX_SKILLLIST
	int iNumSkill = (int)m_vlhSkillList.size();
	for( int iSkill = 0; iSkill < iNumSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( iSkill );
		if( hSkill )
			hSkill->ResetCoolTime();
	}
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlSkillObjects.at( i ).hSkill;
		if (hSkill)
			hSkill->ResetCoolTime();
	}
#endif // #ifndef PRE_FIX_SKILLLIST

#if defined(_GAMESERVER)
	m_listProcessPreFixDefenceSkills.clear();
	m_listProcessPreFixOffenceSkills.clear();
	
	if (!m_prefixSystemDefenceSkills.empty())
	{
		PREFIX_SYSTEM_SKILL_LIST::iterator iter = m_prefixSystemDefenceSkills.begin();
		PREFIX_SYSTEM_SKILL_LIST::iterator enditer = m_prefixSystemDefenceSkills.end();

		for (; iter != enditer; ++iter)
		{
			if (iter->second.hSkill)
				iter->second.hSkill->ResetCoolTime();
		}
	}

	if (!m_prefixSystemOffenceSkills.empty())
	{
		PREFIX_SYSTEM_SKILL_LIST::iterator iter = m_prefixSystemOffenceSkills.begin();
		PREFIX_SYSTEM_SKILL_LIST::iterator enditer = m_prefixSystemOffenceSkills.end();

		for (; iter != enditer; ++iter)
		{
			if (iter->second.hSkill)
				iter->second.hSkill->ResetCoolTime();
		}
	}

	m_PrefixSkillCoolTimeManager.InitList();
#endif // _GAMESERVER
}

void MASkillUser::ResetPvPSkillCoolTime()
{
#ifndef PRE_FIX_SKILLLIST
	int iNumSkill = (int)m_vlhSkillList.size();
	for( int iSkill = 0; iSkill < iNumSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( iSkill );
		if( hSkill->GetSkillType() == CDnSkill::SkillTypeEnum::Active )
		{
			hSkill->OnBeginCoolTime();
		}
	}
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlSkillObjects.at( i ).hSkill;
		if( hSkill->GetSkillType() == CDnSkill::SkillTypeEnum::Active )
		{
			hSkill->OnBeginCoolTime();
		}
	}
#endif // #ifndef PRE_FIX_SKILLLIST
}

void MASkillUser::ResetLadderSkillCoolTime()
{
#ifndef PRE_FIX_SKILLLIST
	ResetSkillCoolTime();
	int iNumSkill = (int)m_vlhSkillList.size();
	for( int iSkill = 0; iSkill < iNumSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( iSkill );
		hSkill->SetElapsedDelayTime( hSkill->GetDelayTime()/2 );
	}
#else
	ResetSkillCoolTime();
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlSkillObjects.at( i ).hSkill;
		hSkill->SetElapsedDelayTime( hSkill->GetDelayTime()/2 );
	}
#endif // #ifndef PRE_FIX_SKILLLIST
}

#ifdef _GAMESERVER
void MASkillUser::IgnoreSkillCoolTime( void )
{
#ifndef PRE_FIX_SKILLLIST
	int iNumSkill = (int)m_vlhSkillList.size();
	for( int iSkill = 0; iSkill < iNumSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( iSkill );
		hSkill->IgnoreCoolTime();
	}

	m_bIgnoreCoolTime = true;
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlSkillObjects.at( i ).hSkill;
		hSkill->IgnoreCoolTime();
	}

	m_bIgnoreCoolTime = true;
#endif // #ifndef PRE_FIX_SKILLLIST
}
#endif


void MASkillUser::ReserveFinishSkill( DnSkillHandle hSkill )
{
	if( hSkill )
	{
		m_vlhReservedFinishSkill.push_back( hSkill );
	}
}


void MASkillUser::CancelUsingSkill( void )
{
	if( m_hProcessSkill )
	{
		m_hProcessSkill->OnEnd( 0, 0.0f );
		m_hProcessSkill.Identity();
	}
};


void MASkillUser::OnAttachWeapon( DnWeaponHandle hWeapon, int iChangedWeaponIndex )
{
	// Check if any passive skills define usable weapons.
	// If passive state effects are not applied because another weapon is equipped, activate them.
	int iNumSelfBlowPassiveSkills = (int)m_vlhSelfPassiveBlowSkill.size();
	for( int i = 0; i < iNumSelfBlowPassiveSkills; ++i )
	{
		DnSkillHandle hSkill = m_vlhSelfPassiveBlowSkill.at( i );
		if( false == hSkill->IsAppliedPassiveBlows() )
		{
			// Checks usable weapons internally.
			ApplyPassiveSkill( hSkill );
		}
	}

	// Skills with rangechecker are updated when a weapon is equipped.
#ifndef _GAMESERVER

#ifndef PRE_FIX_SKILLLIST
	int iNumSkill = (int)m_vlhSkillList.size();
	for( int i = 0; i < iNumSkill; ++i )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( i );
		hSkill->UpdateRangeChecker( hSkill->GetSelectedLevelDataType() );
	}
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlSkillObjects.at( i ).hSkill;
		hSkill->UpdateRangeChecker( hSkill->GetSelectedLevelDataType() );
	}
#endif // #ifndef PRE_FIX_SKILLLIST

#endif
}

void MASkillUser::OnDetachWeapon( DnWeaponHandle hWeapon, int iDetachedWeaponIndex )
{
	// Check if any passive skills define usable weapons.
	// If it does not match the required weapon, remove state effects attached to the passive skill.
	int iNumSelfBlowPassiveSkills = (int)m_vlhSelfPassiveBlowSkill.size();
	for( int i = 0; i < iNumSelfBlowPassiveSkills; ++i )
	{
		DnSkillHandle hSkill = m_vlhSelfPassiveBlowSkill.at( i );
		if( hSkill->IsAppliedPassiveBlows() )
		{
			if( false == hSkill->IsSatisfyWeapon() )
			{
				int iNumStateBlow = hSkill->GetStateEffectCount();
				for( int iBlow = 0; iBlow < iNumStateBlow; ++iBlow )
				{
					const CDnSkill::StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( iBlow );

					DNVector(DnBlowHandle) vlAppliedBlows;
					m_pActor->GatherAppliedStateBlowByBlowIndex( (STATE_BLOW::emBLOW_INDEX)pSE->nID, vlAppliedBlows );

					int iNumAppliedBlow = (int)vlAppliedBlows.size();
					for( int iAppliedBlow = 0; iAppliedBlow < iNumAppliedBlow; ++iAppliedBlow )
					{
						DnBlowHandle hBlow = vlAppliedBlows.at( iAppliedBlow );
						if( hBlow->GetParentSkillInfo()->iSkillID == hSkill->GetClassID() )
						{
							// Delete immediately at this point
							//m_pActor->GetStateBlow()->RemoveImediatlyStateEffectFromID( hBlow->GetBlowID() );
							//Case where RebirthBlow enters via OnBegin and gets deleted.
							//Modified to add to the delete list instead of deleting immediately
							m_pActor->GetStateBlow()->RemoveStateBlowFromID(hBlow->GetBlowID());
						}
					}
				}

				hSkill->SetAppliedPassiveBlows( false );
			}
		}
	}

	// Skills with rangechecker are updated when a weapon is equipped.
#ifndef _GAMESERVER
#ifndef PRE_FIX_SKILLLIST
	int iNumSkill = (int)m_vlhSkillList.size();
	for( int i = 0; i < iNumSkill; ++i )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( i );
		hSkill->UpdateRangeChecker( hSkill->GetSelectedLevelDataType() );
	}
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlSkillObjects.at( i ).hSkill;
		hSkill->UpdateRangeChecker( hSkill->GetSelectedLevelDataType() );
	}
#endif // #ifndef PRE_FIX_SKILLLIST
#endif
}

#ifdef _GAMESERVER
void MASkillUser::SelectSkillLevelDataType( int iLevelDataType, bool bPlayerSummonedMonster/* = false*/ )
{
#ifndef PRE_FIX_SKILLLIST
	for( int i = 0; i < (int)m_vlhSkillList.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( i );
		hSkill->SelectLevelDataType( iLevelDataType, bPlayerSummonedMonster );
	}
#else
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlSkillObjects.at( i ).hSkill;
		hSkill->SelectLevelDataType( iLevelDataType, bPlayerSummonedMonster );
	}
#endif // #ifndef PRE_FIX_SKILLLIST
}

int MASkillUser::GetSelectedSkillLevelDataType( void )
{
#ifndef PRE_FIX_SKILLLIST
	if( m_vlhSkillList.empty() )
		return CDnSkill::NUM_SKILLLEVEL_APPLY_TYPE;
	else
		return m_vlhSkillList.front()->GetSelectedLevelDataType();
#else
	if( m_vlSkillObjects.empty() )
		return CDnSkill::NUM_SKILLLEVEL_APPLY_TYPE;
	else
		return m_vlSkillObjects.front().hSkill->GetSelectedLevelDataType();
#endif // #ifndef PRE_FIX_SKILLLIST
}
#endif

#ifndef _GAMESERVER
bool MASkillUser::IsUsingSkillAction( const char *szAction )
{
	if( m_setUseActionNames.find( szAction ) != m_setUseActionNames.end() ) return true;
	return false;
}
#endif


void MASkillUser::AddStateBlowIDToRemove(int blowID)
{
	m_vlStateBlowIDToRemove.insert(std::map<int, int>::value_type(blowID, blowID));
}

void MASkillUser::InitStateBlowIDToRemove()
{
	m_vlStateBlowIDToRemove.clear();
}

#ifdef _GAMESERVER
void MASkillUser::RemoveApplySEWhenTargetNormalHitProcessor( IDnSkillProcessor* pProcessor )
{
	DNVector( IDnSkillProcessor* )::iterator iter = find( m_vlpApplySEWhenNormalHitProcessor.begin(), m_vlpApplySEWhenNormalHitProcessor.end(), pProcessor );
	if( m_vlpApplySEWhenNormalHitProcessor.end() != iter )
		m_vlpApplySEWhenNormalHitProcessor.erase( iter );
}
#endif


#if defined(_GAMESERVER)
bool MASkillUser::AddPreFixSystemDefenceSkill(int slotIndex, DnSkillHandle hSkill)
{
	if (!hSkill)
	{
		RemovePreFixSystemDefenceSkill(slotIndex);
		return false;
	}

	PREFIX_SYSTEM_SKILLINFO skillInfo;
	skillInfo.slotIndex = slotIndex;
	skillInfo.hSkill = hSkill;
	skillInfo.hSkill->SetItemPrefixSkill();
	
	//Get the probability value of the skill.
	CDnProbabilityChecker* pProbabilityChecker = static_cast<CDnProbabilityChecker*>(hSkill->GetChecker(IDnSkillUsableChecker::PROB_CHECKER));
	skillInfo.fRatio = pProbabilityChecker ? pProbabilityChecker->GetProbability() : 0.0f;
	
	bool bResult = m_prefixSystemDefenceSkills.insert(PREFIX_SYSTEM_SKILL_LIST::value_type(slotIndex, skillInfo)).second;

// 	if (bResult)
// 		RefreshPrefixDefenceSkills();

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
	AddPrefixDefenceSkill(hSkill);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

	return bResult;
}

void MASkillUser::RemovePreFixSystemDefenceSkill(int slotIndex)
{
	PREFIX_SYSTEM_SKILL_LIST::iterator iter = m_prefixSystemDefenceSkills.find(slotIndex);
	if (iter != m_prefixSystemDefenceSkills.end())
	{
		DnSkillHandle hSkill = iter->second.hSkill;
		
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
		RemovePrefixDefenceSkill(hSkill);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

		RemoveProcessPrefixDefenceSkill(hSkill);
				
		SAFE_RELEASE_SPTR(hSkill);
		m_prefixSystemDefenceSkills.erase(iter);

		//RefreshPrefixDefenceSkills();
	}
}

void MASkillUser::RemoveProcessPrefixDefenceSkill(DnSkillHandle hSkill)
{
	list<DnSkillHandle>::iterator iter = m_listProcessPreFixDefenceSkills.begin();
	for( ; iter != m_listProcessPreFixDefenceSkills.end(); )
	{
		DnSkillHandle hProcessPrefixDefenceSkill = *iter;
		if (hProcessPrefixDefenceSkill == hSkill)
		{
			iter = m_listProcessPreFixDefenceSkills.erase(iter);
			continue;
		}

		++iter;
	}
}


void MASkillUser::RefreshPrefixDefenceSkills()
{
	PREFIX_SYSTEM_SKILL_COLLECTION sameSkillList;
	//1. Group skills stored per slot by matching skill.
	GroupingPrefixDefenceSkillsBySameSkillID(m_prefixSystemDefenceSkills, sameSkillList);

	PREFIX_SYSTEM_SKILL_LIST tempCandiateDefenceSkills;
	//2. Sum probabilities of identical skills to determine probability, and select the highest-level skill among duplicates.
	CalculateProbabilitySkill(tempCandiateDefenceSkills, sameSkillList);

	//3. Group by skill priority from the list of highest-level skills per skill ID.
	GroupingSkillByPriority(m_PrefixSystemCandiateDefenceSkills, tempCandiateDefenceSkills);
}

bool MASkillUser::AddPreFixSystemOffenceSkill(int slotIndex, DnSkillHandle hSkill)
{
	if (!hSkill)
	{
		RemovePreFixSystemOffenceSkill(slotIndex);
		return false;
	}

	PREFIX_SYSTEM_SKILLINFO skillInfo;
	skillInfo.slotIndex = slotIndex;
	skillInfo.hSkill = hSkill;
	skillInfo.hSkill->SetItemPrefixSkill();

	//Get the probability value of the skill.
	CDnProbabilityChecker* pProbabilityChecker = static_cast<CDnProbabilityChecker*>(hSkill->GetChecker(IDnSkillUsableChecker::PROB_CHECKER));
	skillInfo.fRatio = pProbabilityChecker ? pProbabilityChecker->GetProbability() : 0.0f;

	bool bResult = m_prefixSystemOffenceSkills.insert(PREFIX_SYSTEM_SKILL_LIST::value_type(slotIndex, skillInfo)).second;

// 	if (bResult)
// 		RefreshPrefixOffenceSkills();

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
	AddPrefixOffenceSkill(hSkill);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

	return bResult;
}

void MASkillUser::RemovePreFixSystemOffenceSkill(int slotIndex)
{
	PREFIX_SYSTEM_SKILL_LIST::iterator iter = m_prefixSystemOffenceSkills.begin(); 
	
	for( ; iter != m_prefixSystemOffenceSkills.end(); )
	{
		if( iter->first == slotIndex )
		{
			DnSkillHandle hSkill = iter->second.hSkill;
			hSkill->OnEnd( 0, 0.f );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
			RemovePrefixOffenceSkill(hSkill);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

			RemoveProcessPrefixOffenceSkill(hSkill);

			SAFE_RELEASE_SPTR(hSkill);
			iter = m_prefixSystemOffenceSkills.erase(iter);
			continue;
		}

		 ++iter;
	}
}

void MASkillUser::RemoveProcessPrefixOffenceSkill(DnSkillHandle hSkill)
{
	list<DnSkillHandle>::iterator iter = m_listProcessPreFixOffenceSkills.begin();
	for( ; iter != m_listProcessPreFixOffenceSkills.end(); )
	{
		DnSkillHandle hProcessPrefixOffenceSkill = *iter;
		if (hProcessPrefixOffenceSkill == hSkill)
		{
			iter = m_listProcessPreFixOffenceSkills.erase(iter);
			continue;
		}

		++iter;
	}
}

void MASkillUser::RefreshPrefixOffenceSkills(float fHitDamageProb)
{
	PREFIX_SYSTEM_SKILL_COLLECTION sameSkillList;
	//1. Group skills stored per slot by matching skill.
	GroupingPrefixDefenceSkillsBySameSkillID(m_prefixSystemOffenceSkills, sameSkillList);

	PREFIX_SYSTEM_SKILL_LIST tempCandiateOffenceSkills;
	//2. Sum probabilities of identical skills to determine probability, and select the highest-level skill among duplicates.
	CalculateProbabilitySkill(tempCandiateOffenceSkills, sameSkillList);

	//3. Group by skill priority from the list of highest-level skills per skill ID.
	GroupingSkillByPriority(m_PrefixSystemCandiateOffenceSkills, tempCandiateOffenceSkills, fHitDamageProb);
}

void MASkillUser::ProcessPrefixDefenceSkill(DnActorHandle hHitter)
{
	//If no prefix system defense skill is registered	
	if (m_prefixSystemDefenceSkills.empty())
		return;

	OutputDebug("Prefix Defense Skill Start-----------------------------!!\n");

	//Select among prefix system defense skills.
	RefreshPrefixDefenceSkills();

	// Among the skills in the final list, use only the skill stored in the highest-priority list.
	if (m_PrefixSystemCandiateDefenceSkills.empty())
	{
		OutputDebug("No Prefix Defense Skill selected.-----------------------------!!\n");
		return;
	}

	PREFIX_SYSTEM_SKILL_COLLECTION::iterator selectIter = m_PrefixSystemCandiateDefenceSkills.begin();
	PRESIX_SYSTEM_SKILLS &skills = selectIter->second;

	PRESIX_SYSTEM_SKILLS::iterator iter = skills.begin();
	PRESIX_SYSTEM_SKILLS::iterator endIter = skills.end();

	for (; iter != endIter; ++iter)
	{
		DnSkillHandle hSkill = (*iter).hSkill;
		if (!hSkill /*|| CDnSkill::UsingResult::Success != hSkill->CanExecute()*/)
			continue;

		/*
		//Skip if cooldown remains.
		if (hSkill->GetCoolTime() != 0.0f && hSkill->GetElapsedDelayTime() > 0.0f)
			continue;
		*/

		OutputDebug("Prefix Defense Skill(%d) Start....\n", hSkill->GetClassID());

		//Start Skill cooldown?
		hSkill->OnBeginCoolTime();
		m_PrefixSkillCoolTimeManager.AddCoolTime(hSkill);

		int nStateEffectCount = hSkill->GetStateEffectCount();
		// Determine the target actor per state effect of the skill, check for duplicate processing,
		// Seems state effects need to be applied per actor...
		for( DWORD k = 0; k < hSkill->GetStateEffectCount(); k++ ) 
		{
			CDnSkill::StateEffectStruct *pLocalStruct = hSkill->GetStateEffectFromIndex(k);

			DnActorHandle hApplyActor;

			// Determines the actor to apply to based on the state effect target type.
			switch( pLocalStruct->ApplyType )
			{
			case CDnSkill::ApplySelf:	// To self
				hApplyActor = m_pActor->GetActorHandle();
				break;

			case CDnSkill::ApplyTarget: // To target
				hApplyActor = hHitter;
				break;

			case CDnSkill::ApplyEnemy:	// To enemy.. (if not an ally, the one who hit me is the enemy..)
				if( m_pActor->GetTeam() == hHitter->GetTeam() )
					continue;
				else
					hApplyActor = hHitter;
				break;

			case CDnSkill::ApplyFriend: // Same side.. (if same side, is the one who hit me on the same side..?)
				if( m_pActor->GetTeam() != hHitter->GetTeam() )
					continue;
				else
					hApplyActor = hHitter;
				break;
			}

			// Among state effects, those with 0 duration and cast on self
			// Store in the skill's m_vlApplyNoPacketStateEffectList and when the skill ends
			// Ensure it gets deleted...[2011/01/13 semozz]
			bool isSelfPermentStateEffect = false;
			if (CDnSkill::ApplySelf == pLocalStruct->ApplyType && 
				(0 == pLocalStruct->nDurationTime ||
				-1 == pLocalStruct->nDurationTime))
			{
				pLocalStruct->nDurationTime = -1;
				isSelfPermentStateEffect = true;
			}

			CDnStateBlow *pStateBlow = hApplyActor->GetStateBlow();
			CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;

			if (pStateBlow)
				eResult = pStateBlow->CanApplySkillStateEffect(hSkill->GetInfo(), *pLocalStruct);
			
			if (CDnSkill::CanApply::Fail != eResult)
			{
				hApplyActor->RemoveResetStateBlow();

				int nBlowID = hApplyActor->CmdAddStateEffect( hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID, 
					pLocalStruct->nDurationTime, pLocalStruct->szValue.c_str(), false, false );

				if (isSelfPermentStateEffect)
					hSkill->AddPrefixBlow(hApplyActor, nBlowID);
			}

			hApplyActor->InitStateBlowIDToRemove();
		}

		//Store the used defense skill in the list.
		m_listProcessPreFixDefenceSkills.push_back(hSkill);
	}

	OutputDebug("Prefix Defense Skill End  -----------------------------!!\n");
}

void MASkillUser::ProcessPrefixOffenceSkill(float fHitDamageProb)
{
	//If no prefix system defense skill is registered	
	if (m_prefixSystemOffenceSkills.empty())
		return;

	//Select among prefix system attack skills.
	RefreshPrefixOffenceSkills(fHitDamageProb);

	// Among the skills in the final list, use only the skill stored in the highest-priority list.
	if (m_PrefixSystemCandiateOffenceSkills.empty())
		return;

	PREFIX_SYSTEM_SKILL_COLLECTION::iterator selectIter = m_PrefixSystemCandiateOffenceSkills.begin();
	PRESIX_SYSTEM_SKILLS &skills = selectIter->second;

	PRESIX_SYSTEM_SKILLS::iterator iter = skills.begin();
	PRESIX_SYSTEM_SKILLS::iterator endIter = skills.end();

	for (; iter != endIter; ++iter)
	{
		DnSkillHandle hSkill = (*iter).hSkill;
		if (!hSkill/* || CDnSkill::UsingResult::Success != hSkill->CanExecute()*/)
			continue;

		//Start Skill cooldown?
		hSkill->OnBeginCoolTime();
		m_PrefixSkillCoolTimeManager.AddCoolTime(hSkill);

		int nStateEffectCount = hSkill->GetStateEffectCount();
		// Determine the target actor per state effect of the skill, check for duplicate processing,
		// Seems state effects need to be applied per actor...
		for( DWORD k = 0; k < hSkill->GetStateEffectCount(); k++ ) 
		{
			CDnSkill::StateEffectStruct *pLocalStruct = hSkill->GetStateEffectFromIndex(k);

			DnActorHandle hApplyActor;

			// Determines the actor to apply to based on the state effect target type.
			switch( pLocalStruct->ApplyType )
			{
			case CDnSkill::ApplySelf:	// To self
				hApplyActor = m_pActor->GetActorHandle();
				break;

			case CDnSkill::ApplyTarget: // To target
				continue;

			case CDnSkill::ApplyEnemy:	// To enemy.. (if not an ally, the one who hit me is the enemy..)
				continue;
				break;

			case CDnSkill::ApplyFriend: // Same side.. (if same side, is the one who hit me on the same side..?)
				continue;
				break;
			}

			// Among state effects, those with 0 duration and cast on self
			// Store in the skill's m_vlApplyNoPacketStateEffectList and when the skill ends
			// Ensure it gets deleted...[2011/01/13 semozz]
			bool isSelfPermentStateEffect = false;
			if (CDnSkill::ApplySelf == pLocalStruct->ApplyType && 
				(0 == pLocalStruct->nDurationTime ||
				-1 == pLocalStruct->nDurationTime))
			{
				pLocalStruct->nDurationTime = -1;
				isSelfPermentStateEffect = true;
			}

			CDnStateBlow *pStateBlow = hApplyActor->GetStateBlow();
			CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;

			if (pStateBlow)
				eResult = pStateBlow->CanApplySkillStateEffect(hSkill->GetInfo(), *pLocalStruct);

			if (CDnSkill::CanApply::Fail != eResult)
			{
				hApplyActor->RemoveResetStateBlow();

				int nBlowID = hApplyActor->CmdAddStateEffect( hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID, 
					pLocalStruct->nDurationTime, pLocalStruct->szValue.c_str(), false, false );

				if (isSelfPermentStateEffect)
					hSkill->AddPrefixBlow(hApplyActor, nBlowID);
			}

			hApplyActor->InitStateBlowIDToRemove();
		}

		//Store the used defense skill in the list.
		m_listProcessPreFixOffenceSkills.push_back(hSkill);
	}
}

void MASkillUser::ApplyPrefixOffenceSkillToTarget(DnActorHandle hTarget)
{
	// Among the skills in the final list, use only the skill stored in the highest-priority list.
	if (m_listProcessPreFixOffenceSkills.empty())
		return;

	list<DnSkillHandle>::iterator iter = m_listProcessPreFixOffenceSkills.begin();
	list<DnSkillHandle>::iterator endIter = m_listProcessPreFixOffenceSkills.end();
	
	for (; iter != endIter; ++iter)
	{
		DnSkillHandle hSkill = (*iter);
		if (!hSkill)
			continue;

		int nStateEffectCount = hSkill->GetStateEffectCount();
		// Determine the target actor per state effect of the skill, check for duplicate processing,
		// Seems state effects need to be applied per actor...
		for( DWORD k = 0; k < hSkill->GetStateEffectCount(); k++ ) 
		{
			CDnSkill::StateEffectStruct *pLocalStruct = hSkill->GetStateEffectFromIndex(k);

			DnActorHandle hApplyActor;

			// Determines the actor to apply to based on the state effect target type.
			switch( pLocalStruct->ApplyType )
			{
			case CDnSkill::ApplySelf:	// To self
				continue;
			case CDnSkill::ApplyTarget: // To target
				hApplyActor = hTarget;
				break;

			case CDnSkill::ApplyEnemy:	// To enemy.. (if not an ally, the one who hit me is the enemy..)
				if( m_pActor->GetTeam() == hTarget->GetTeam() )
					continue;
				else
					hApplyActor = hTarget;
				break;

			case CDnSkill::ApplyFriend: // Same side.. (if same side, is the one who hit me on the same side..?)
				if( m_pActor->GetTeam() != hTarget->GetTeam() )
					continue;
				else
					hApplyActor = hTarget;
				break;
			}

			// Among state effects, those with 0 duration and cast on self
			// Store in the skill's m_vlApplyNoPacketStateEffectList and when the skill ends
			// Ensure it gets deleted...[2011/01/13 semozz]
			bool isSelfPermentStateEffect = false;
			if (CDnSkill::ApplySelf == pLocalStruct->ApplyType && 
				(0 == pLocalStruct->nDurationTime ||
				-1 == pLocalStruct->nDurationTime))
			{
				pLocalStruct->nDurationTime = -1;
				isSelfPermentStateEffect = true;
			}

			CDnStateBlow *pStateBlow = hApplyActor->GetStateBlow();
			CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;
			
			if (pStateBlow)
				eResult = pStateBlow->CanApplySkillStateEffect(hSkill->GetInfo(), *pLocalStruct);

			if (CDnSkill::CanApply::Fail != eResult)
			{
				hApplyActor->RemoveResetStateBlow();

				int nBlowID = hApplyActor->CmdAddStateEffect( hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID, 
					pLocalStruct->nDurationTime, pLocalStruct->szValue.c_str(), false, false );

				if (isSelfPermentStateEffect)
					hSkill->AddPrefixBlow(hApplyActor, nBlowID);
			}

			hApplyActor->InitStateBlowIDToRemove();
		}
	}
}

void MASkillUser::CalculateProbabilitySkill(PREFIX_SYSTEM_SKILL_LIST &candidateSkills,
											PREFIX_SYSTEM_SKILL_COLLECTION &sameSkillList)
{
	//Initialize candidate skill list
	candidateSkills.clear();

	PREFIX_SYSTEM_SKILL_COLLECTION::iterator iter = sameSkillList.begin();
	PREFIX_SYSTEM_SKILL_COLLECTION::iterator endIter = sameSkillList.end();

	for (; iter != endIter; ++iter)
	{
		//Sum probabilities of skills with the same skill ID.
		PRESIX_SYSTEM_SKILLS::iterator skillIter = iter->second.begin();
		PRESIX_SYSTEM_SKILLS::iterator skillEndIter = iter->second.end();

		float fProbability = 0.0f;
		DnSkillHandle hSelectedSkill;
		int nSlotIndex = -1;

		for (; skillIter != skillEndIter; ++skillIter)
		{
			if (skillIter->hSkill)
			{
				fProbability += skillIter->fRatio;
				
				// Target skill
				if (!hSelectedSkill)
				{
					hSelectedSkill = skillIter->hSkill;
					nSlotIndex = skillIter->slotIndex;
				}
				else
				{
					if (hSelectedSkill->GetLevel() < skillIter->hSkill->GetLevel())
					{
						hSelectedSkill = skillIter->hSkill;
						nSlotIndex = skillIter->slotIndex;
					}
				}
			}
		}

		// If a skill is selected, add it to the final skill candidate list.
		if (hSelectedSkill && -1 != nSlotIndex && 0.0f != fProbability)
		{
			int nSkillID = hSelectedSkill->GetClassID();
			
			PREFIX_SYSTEM_SKILLINFO skillInfo;
			skillInfo.fRatio = fProbability;
			skillInfo.hSkill = hSelectedSkill;
			skillInfo.slotIndex = nSlotIndex;

			OutputDebug("Same skill(%d) summed final probability (%f) decided as slot Index(%d)!!!!!\n", skillInfo.hSkill->GetClassID(), skillInfo.fRatio, skillInfo.slotIndex);

			candidateSkills.insert(PREFIX_SYSTEM_SKILL_LIST::value_type(nSkillID, skillInfo));
		}
	}
}

void MASkillUser::GroupingPrefixDefenceSkillsBySameSkillID(PREFIX_SYSTEM_SKILL_LIST &skillList, PREFIX_SYSTEM_SKILL_COLLECTION &sameSkillList)
{
	PREFIX_SYSTEM_SKILL_LIST::iterator iter = skillList.begin();
	PREFIX_SYSTEM_SKILL_LIST::iterator endIter = skillList.end();

	for (; iter != endIter; ++iter)
	{
		int nSkillID = -1;
		int nSlotIndex = iter->first;	//Slot index

		PREFIX_SYSTEM_SKILLINFO skillInfo = iter->second;

		if (!skillInfo.hSkill)
			continue;

		nSkillID = skillInfo.hSkill->GetClassID();

		PREFIX_SYSTEM_SKILL_COLLECTION::iterator findIter = sameSkillList.find(nSkillID);
		
		if (findIter != sameSkillList.end())
		{
			//If a skill with the same skill ID is already in the list
			
			
			skillInfo.slotIndex = nSlotIndex;
			
			findIter->second.push_back(skillInfo);
		}
		else
		{
			//If no skill is registered yet, create a list and register..
			PRESIX_SYSTEM_SKILLS skillList;
			
			skillInfo.slotIndex = nSlotIndex;
			skillList.push_back(skillInfo);

			sameSkillList.insert(PREFIX_SYSTEM_SKILL_COLLECTION::value_type(nSkillID, skillList));
		}
	}
}

void MASkillUser::GroupingSkillByPriority(PREFIX_SYSTEM_SKILL_COLLECTION& prefixSystemSkills, PREFIX_SYSTEM_SKILL_LIST& tempCandiateSkills, float fHitDamageProb/* = 1.0f*/)
{
	prefixSystemSkills.clear();

	PREFIX_SYSTEM_SKILL_LIST::iterator iter = tempCandiateSkills.begin();
	PREFIX_SYSTEM_SKILL_LIST::iterator endIter = tempCandiateSkills.end();

	//Iterate through the representative skill list and group by priority.
	for (; iter != endIter; ++iter)
	{
		int nPriority = 0;

		PREFIX_SYSTEM_SKILLINFO skillInfo = iter->second;

		if (!skillInfo.hSkill)
			continue;

		nPriority = skillInfo.hSkill->GetPrefixSystemPriority();
		
		//Check skill cooldown.
		//if (skillInfo.hSkill->GetCoolTime() > 0.0f && skillInfo.hSkill->GetElapsedDelayTime() > 0.0f)
		if (m_PrefixSkillCoolTimeManager.IsCoolTime(skillInfo.hSkill->GetClassID()))
		{
			OutputDebug("Skill(%d) on cooldown.........!!!!\n", skillInfo.hSkill->GetClassID());
			continue;
		}

		//Only add skills that pass the probability check to the list...
		bool bExecuteable = rand() % 10000 <= (iter->second.fRatio * fHitDamageProb);
		if (!bExecuteable)
		{
			OutputDebug("Skill(%d) filtered out by probability...!!!!\n", skillInfo.hSkill->GetClassID());
			continue;
		}

		OutputDebug("At priority (%d), skill(%d) registered!!!!\n", nPriority, skillInfo.hSkill->GetClassID());

		PREFIX_SYSTEM_SKILL_COLLECTION::iterator findIter = prefixSystemSkills.find(nPriority);
		if (findIter != prefixSystemSkills.end())
		{
			//If priority info is already registered 
			//Add skill info to the priority list.
			findIter->second.push_back(skillInfo);
		}
		else
		{
			//If priority is not registered yet..
			//Create a list to store skills
			//Configure skill info and add to list.
			PRESIX_SYSTEM_SKILLS skillList;
			skillList.push_back(skillInfo);

			prefixSystemSkills.insert(PREFIX_SYSTEM_SKILL_COLLECTION::value_type(nPriority, skillList));
		}
	}

}

void MASkillUser::EndPrefixSystemSkill( LOCAL_TIME LocalTime, float fDelta )
{
	list<DnSkillHandle>::iterator iter = m_listProcessPreFixDefenceSkills.begin();
	for( iter; iter != m_listProcessPreFixDefenceSkills.end(); ++iter )
	{
		DnSkillHandle hAutoPassive = *iter;
		hAutoPassive->OnEnd( LocalTime, fDelta );
	}

	iter = m_listProcessPreFixOffenceSkills.begin();
	for( iter; iter != m_listProcessPreFixOffenceSkills.end(); ++iter )
	{
		DnSkillHandle hAutoPassive = *iter;
		hAutoPassive->OnEnd( LocalTime, fDelta );
	}
}

void MASkillUser::InitPrefixOffenceSkills()
{
	m_PrefixSystemCandiateOffenceSkills.clear();
}

bool MASkillUser::IsPrefixTriggerSkill()
{
	if (!m_hProcessSkill)
		return false;
	
	return m_hProcessSkill->IsPrefixTriggerSkill();
}

#endif // _GAMESERVER


void MASkillUser::UpdateSkillLevelUpInfo()
{
	std::map<int, SkillLevelUpInfo>::iterator iter = m_SkillLevelUpInfoList.begin();
	std::map<int, SkillLevelUpInfo>::iterator endIter = m_SkillLevelUpInfoList.end();

	m_SkillLevelUpInfo.clear();

	for (; iter != endIter; ++iter)
	{
		SkillLevelUpInfo& skillLevelUpInfo = iter->second;

		//If Skill ID / level-up value is valid
		if (skillLevelUpInfo.nSkillID != 0 && skillLevelUpInfo.nLevelUp != 0)
		{
			//Find the matching skill ID in the skill level-up info list.
			std::map<int, int>::iterator findIter = m_SkillLevelUpInfo.find(skillLevelUpInfo.nSkillID);
			if (findIter != m_SkillLevelUpInfo.end())
			{
				// Skip if existing level-up value is larger.
				if (findIter->second > skillLevelUpInfo.nLevelUp)
					continue;
				
				// Remove from list if existing level-up value is not larger..
				m_SkillLevelUpInfo.erase(findIter);
			}

			// Save the new skill level-up info.
			m_SkillLevelUpInfo.insert(std::make_pair(skillLevelUpInfo.nSkillID, skillLevelUpInfo.nLevelUp));
		}
	}
}

void MASkillUser::AddSkillLevelUpInfo(int nSlotIndex, int nSkillID, int nLevelUp)
{
	DnSkillHandle hSkill = FindSkill(nSkillID);
	if( hSkill )
	{
		if (hSkill->GetElapsedDelayTime() > 0.0f || hSkill->IsToggleOn())
			return;
	}

	SkillLevelUpInfo skillLevelUpInfo;
	skillLevelUpInfo.nSkillID = nSkillID;
	skillLevelUpInfo.nLevelUp = nLevelUp;

	m_SkillLevelUpInfoList.insert(std::make_pair(nSlotIndex, skillLevelUpInfo));

	//Update skill level-up info..
	UpdateSkillLevelUpInfo();

	int nSkillLevelUp = GetSkillLevelUpValue(nSkillID);
	SkillLevelUp(nSkillID, nSkillLevelUp);
}

void MASkillUser::RemoveSkillLevelUpInfo(int nSlotIndex)
{
	std::map<int, SkillLevelUpInfo>::iterator findIter = m_SkillLevelUpInfoList.find(nSlotIndex);
	if (findIter != m_SkillLevelUpInfoList.end())
	{
		//Store the skill ID applied to the level-up item to be deleted.
		int nSkillID = findIter->second.nSkillID;

		DnSkillHandle hSkill = FindSkill(nSkillID);
		if( hSkill )
		{
			if (hSkill->GetElapsedDelayTime() > 0.0f || hSkill->IsToggleOn())
				return;
		}

		//Remove level-up info from list and
		m_SkillLevelUpInfoList.erase(findIter);

		//Update level-up info
		UpdateSkillLevelUpInfo();

		//Since level-up info is removed 
		int nLevelUp = GetSkillLevelUpValue(nSkillID);
		SkillLevelUp(nSkillID, nLevelUp);
	}
}

int MASkillUser::GetSkillLevelUpValue(int nSkillID)
{
	int nLevelUpValue = 0;

	int nLevelUpValueByNormalItem = 0;
	std::map<int, int>::iterator findNormalIter = m_SkillLevelUpInfo.find(nSkillID);
	if (findNormalIter != m_SkillLevelUpInfo.end())
		nLevelUpValueByNormalItem = findNormalIter->second;

	int nLevelUpValueByCashItem = 0;
	std::map<int, int>::iterator findCashIter = m_CashSkillLevelUpInfo.find(nSkillID);
	if (findCashIter != m_CashSkillLevelUpInfo.end())
		nLevelUpValueByCashItem = findCashIter->second;

	//Level-up cap value for cash items is 1
	if (nLevelUpValueByCashItem > 1)
		nLevelUpValueByCashItem = 1;

	//Level-up items do not stack; apply the higher value.
	nLevelUpValue = max(nLevelUpValueByNormalItem, nLevelUpValueByCashItem);

	return nLevelUpValue;
}

bool MASkillUser::ExistSkillLevelUpValue()
{
	if( m_SkillLevelUpInfo.empty() && m_CashSkillLevelUpInfo.empty() )
		return false;

	return true;
}

void MASkillUser::SkillLevelUp(int nSkillID, int nLevelUp)
{
	//Find existing skill; if present and not initialized, level it up.
	DnSkillHandle hSkill = FindSkill(nSkillID);
	if (!hSkill)
		return;

	int nCurentSkillLevel = hSkill->GetLevel();
	int nCurLevlUpValue = hSkill->GetLevelUpValue();

	bool bChangeLevel = true;

	if (nCurentSkillLevel == 0)
		bChangeLevel = false;

	//If the new level-up value matches the currently applied level-up value, no need to change the skill.
	if (nCurLevlUpValue == nLevelUp)
		bChangeLevel = false;

	if (bChangeLevel)
	{
		//In AddSkill inside SetSkillLevel, the skill is created by calculating the current increment onto the original level..
		int nOrigLevel = nCurentSkillLevel - nCurLevlUpValue;
		ChangeSkillLevelUp(nSkillID, nOrigLevel);
	}
#if defined(_GAMESERVER)
#else
	else
		hSkill->DisableSkillByItemMove(false);
#endif // _GAMESERVER
}

void MASkillUser::ChangeSkillLevelUp(int nSkillID, int nOrigLevel)
{
	RemoveSkill(nSkillID);
	AddSkill(nSkillID, nOrigLevel);
}

//	std::map<int, SkillLevelUpInfo> m_CashSkillLevelUpInfoList;

void MASkillUser::AddSkillLevelUpInfoByCashItem(int nSlotIndex, int nSkillID, int nLevelUp)
{
	SkillLevelUpInfo skillLevelUpInfo;
	skillLevelUpInfo.nSkillID = nSkillID;
	skillLevelUpInfo.nLevelUp = nLevelUp;

	m_CashSkillLevelUpInfoList.insert(std::make_pair(nSlotIndex, skillLevelUpInfo));

	//Update skill level-up info..
	UpdateSkillLevelUpInfoByCashItem();

	int nSkillLevelUp = GetSkillLevelUpValue(nSkillID);
	SkillLevelUp(nSkillID, nSkillLevelUp);
}

void MASkillUser::UpdateSkillLevelUpInfoByCashItem()
{
	std::map<int, SkillLevelUpInfo>::iterator iter = m_CashSkillLevelUpInfoList.begin();
	std::map<int, SkillLevelUpInfo>::iterator endIter = m_CashSkillLevelUpInfoList.end();

	m_CashSkillLevelUpInfo.clear();

	for (; iter != endIter; ++iter)
	{
		SkillLevelUpInfo& skillLevelUpInfo = iter->second;

		//If Skill ID / level-up value is valid
		if (skillLevelUpInfo.nSkillID != 0 && skillLevelUpInfo.nLevelUp != 0)
		{
			//Find the matching skill ID in the skill level-up info list.
			std::map<int, int>::iterator findIter = m_CashSkillLevelUpInfo.find(skillLevelUpInfo.nSkillID);
			if (findIter != m_CashSkillLevelUpInfo.end())
			{
				// Skip if existing level-up value is larger.
				if (findIter->second > skillLevelUpInfo.nLevelUp)
					continue;

				// Remove from list if existing level-up value is not larger..
				m_CashSkillLevelUpInfo.erase(findIter);
			}

			// Save the new skill level-up info.
			m_CashSkillLevelUpInfo.insert(std::make_pair(skillLevelUpInfo.nSkillID, skillLevelUpInfo.nLevelUp));
		}
	}
}

void MASkillUser::RemoveSkillLevelUpInfoByCashItem(int nSlotIndex)
{
	std::map<int, SkillLevelUpInfo>::iterator findIter = m_CashSkillLevelUpInfoList.find(nSlotIndex);
	if (findIter != m_CashSkillLevelUpInfoList.end())
	{
		//Store the skill ID applied to the level-up item to be deleted.
		int nSkillID = findIter->second.nSkillID;

		//Remove level-up info from list and
		m_CashSkillLevelUpInfoList.erase(findIter);

		//Update level-up info
		UpdateSkillLevelUpInfoByCashItem();

		//Since level-up info is removed 
		int nLevelUp = GetSkillLevelUpValue(nSkillID);
		SkillLevelUp(nSkillID, nLevelUp);
	}
}


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
#if defined(_GAMESERVER)
void MASkillUser::AddPrefixDefenceSkill(DnSkillHandle hSkill)
{
	if (!hSkill)
		return;
	
	//Prefix skill type of this skill (Luck/Destruction...)
	int nPrefixSkillType = hSkill->GetPrefixSkillType();
	PREFIX_SKILL_INFO::iterator findIter = m_PrefixDefenceSkills.find(nPrefixSkillType);

	//If an existing category exists
	if (findIter != m_PrefixDefenceSkills.end())
	{
		//Add skill to existing prefix skill manager object and update info.
		findIter->second->AddSkill(hSkill);
	}
	else
	{
		//If no prefix skill is registered yet, create and register a new one.
		CDnPrefixSkill* pPrefixSkill = new CDnPrefixSkill(nPrefixSkillType);
		pPrefixSkill->AddSkill(hSkill);

		m_PrefixDefenceSkills.insert(make_pair(nPrefixSkillType, pPrefixSkill));
	}
}

void MASkillUser::RemovePrefixDefenceSkill(DnSkillHandle hSkill)
{
	//Prefix skill type of this skill (Luck/Destruction...)
	int nPrefixSkillType = hSkill->GetPrefixSkillType();
	PREFIX_SKILL_INFO::iterator findIter = m_PrefixDefenceSkills.find(nPrefixSkillType);

	//If an existing category exists
	if (findIter != m_PrefixDefenceSkills.end())
	{
		//Remove skill from existing prefix skill manager object.
		findIter->second->RemoveSkill(hSkill);

		//Remove from list if there is no registered skill.
		if (findIter->second->GetSkillCount() == 0)
		{
			SAFE_DELETE(findIter->second);
			m_PrefixDefenceSkills.erase(findIter);
		}
	}
}

void MASkillUser::ProcessPrefixDefenceSkill_New(DnActorHandle hHitter)
{
	//If no prefix system defense skill is registered	
	if (m_PrefixDefenceSkills.empty())
		return;

	OutputDebug("Prefix Defense Skill Start-----------------------------!!\n");

	//Calculate prefix skill activation probability.
	PREFIX_SKILL_INFO::iterator iter = m_PrefixDefenceSkills.begin();
	PREFIX_SKILL_INFO::iterator endIter = m_PrefixDefenceSkills.end();
	for (; iter != endIter; ++iter)
	{
		CDnPrefixSkill* pPrefixSkill = iter->second;
		if (!pPrefixSkill)
			continue;

		//Check whether prefix skill is on cooldown.
		if (m_PrefixSkillCoolTimeManager.IsCoolTime(pPrefixSkill->GetSkillType()))
		{
			OutputDebug("Prefix Defense Skill(%d) on cooldown!!!!!!!!!!\n", pPrefixSkill->GetSkillType());
			continue;
		}

		//Check prefix skill activation probability.
		bool bExecuteable = rand() % 10000 <= pPrefixSkill->GetProbability();
		if (!bExecuteable)
		{
			OutputDebug("Prefix Defense Skill(%d) filtered out by probability...!!!!\n", pPrefixSkill->GetSkillType());
			continue;
		}

		//Representative skill
		DnSkillHandle hSkill = pPrefixSkill->GetSkillHandle();
		if (!hSkill)
			continue;

		hSkill->OnBeginCoolTime();
		//Start Skill cooldown?
		m_PrefixSkillCoolTimeManager.AddCoolTime(pPrefixSkill);
		
		int nStateEffectCount = pPrefixSkill->GetStateEffectCount();
		// Determine the target actor per state effect of the skill, check for duplicate processing,
		// Seems state effects need to be applied per actor...
		for( int k = 0; k < pPrefixSkill->GetStateEffectCount(); k++ ) 
		{
			CDnSkill::StateEffectStruct *pLocalStruct = pPrefixSkill->GetStateEffectFromIndex(k);

			DnActorHandle hApplyActor;

			// Determines the actor to apply to based on the state effect target type.
			switch( pLocalStruct->ApplyType )
			{
			case CDnSkill::ApplySelf:	// To self
				hApplyActor = m_pActor->GetActorHandle();
				break;

			case CDnSkill::ApplyTarget: // To target
				hApplyActor = hHitter;
				break;

			case CDnSkill::ApplyEnemy:	// To enemy.. (if not an ally, the one who hit me is the enemy..)
				if( m_pActor->GetTeam() == hHitter->GetTeam() )
					continue;
				else
					hApplyActor = hHitter;
				break;

			case CDnSkill::ApplyFriend: // Same side.. (if same side, is the one who hit me on the same side..?)
				if( m_pActor->GetTeam() != hHitter->GetTeam() )
					continue;
				else
					hApplyActor = hHitter;
				break;
			}

			// Among state effects, those with 0 duration and cast on self
			// Store in the skill's m_vlApplyNoPacketStateEffectList and when the skill ends
			// Ensure it gets deleted...[2011/01/13 semozz]
			bool isSelfPermentStateEffect = false;
			if (CDnSkill::ApplySelf == pLocalStruct->ApplyType && 
				(0 == pLocalStruct->nDurationTime ||
				-1 == pLocalStruct->nDurationTime))
			{
				pLocalStruct->nDurationTime = -1;
				isSelfPermentStateEffect = true;
			}

			CDnStateBlow *pStateBlow = hApplyActor->GetStateBlow();
			CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;

			if (pStateBlow)
				eResult = pStateBlow->CanApplySkillStateEffect(hSkill->GetInfo(), *pLocalStruct);
			
			if (CDnSkill::CanApply::Fail != eResult)
			{
				hApplyActor->RemoveResetStateBlow();

				int nBlowID = hApplyActor->CmdAddStateEffect( hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID, 
					pLocalStruct->nDurationTime, pLocalStruct->szValue.c_str(), false, false );

				OutputDebug("Prefix Defense Skill(%d) State Effect (%d)-(%s)\n", pPrefixSkill->GetSkillType(), pLocalStruct->nID, pLocalStruct->szValue.c_str());

				if (isSelfPermentStateEffect)
					hSkill->AddPrefixBlow(hApplyActor, nBlowID);
			}

			hApplyActor->InitStateBlowIDToRemove();
		}

		//Store the used defense skill in the list.
		m_listProcessPreFixDefenceSkills.push_back(hSkill);
	}

	OutputDebug("Prefix Defense Skill End  -----------------------------!!\n");
}


void MASkillUser::AddPrefixOffenceSkill(DnSkillHandle hSkill)
{
	if (!hSkill)
		return;

	//Prefix skill type of this skill (Luck/Destruction...)
	int nPrefixSkillType = hSkill->GetPrefixSkillType();
	PREFIX_SKILL_INFO::iterator findIter = m_PrefixOffenceSkills.find(nPrefixSkillType);

	//If an existing category exists
	if (findIter != m_PrefixOffenceSkills.end())
	{
		//Add skill to existing prefix skill manager object and update info.
		findIter->second->AddSkill(hSkill);
	}
	else
	{
		//If no prefix skill is registered yet, create and register a new one.
		CDnPrefixSkill* pPrefixSkill = new CDnPrefixSkill(nPrefixSkillType);
		pPrefixSkill->AddSkill(hSkill);

		m_PrefixOffenceSkills.insert(make_pair(nPrefixSkillType, pPrefixSkill));
	}
}

void MASkillUser::RemovePrefixOffenceSkill(DnSkillHandle hSkill)
{
	//Prefix skill type of this skill (Luck/Destruction...)
	int nPrefixSkillType = hSkill->GetPrefixSkillType();
	PREFIX_SKILL_INFO::iterator findIter = m_PrefixOffenceSkills.find(nPrefixSkillType);

	//If an existing category exists
	if (findIter != m_PrefixOffenceSkills.end())
	{
		//Remove skill from existing prefix skill manager object.
		findIter->second->RemoveSkill(hSkill);

		//Remove from list if there is no registered skill.
		if (findIter->second->GetSkillCount() == 0)
		{
			RemoveProcessPrefixOffenceSkill(findIter->second);

			SAFE_DELETE(findIter->second);
			m_PrefixOffenceSkills.erase(findIter);
		}
	}
}

//Regarding dump (runs in one frame so this shouldn't happen, but...) when an attack suffix skill is removed
//Remove if the same skill exists in the running suffix skill list.
void MASkillUser::RemoveProcessPrefixOffenceSkill(CDnPrefixSkill* pRemovePrefixSkill)
{
	if (NULL == pRemovePrefixSkill)
		return;

	list<CDnPrefixSkill*>::iterator iter = m_ProcessPrefixOffenceSkills.begin();
	
	for (; iter != m_ProcessPrefixOffenceSkills.end(); )
	{
		CDnPrefixSkill* pPrefixSkill = (*iter);
		if (pPrefixSkill && pPrefixSkill == pRemovePrefixSkill)
		{
			iter = m_ProcessPrefixOffenceSkills.erase(iter);
			continue;
		}

		++iter;
	}
}

void MASkillUser::ProcessPrefixOffenceSkill_New()
{
	//If no prefix system defense skill is registered	
	if (m_PrefixOffenceSkills.empty())
		return;

	//Clear list before activating prefix skill.
	m_ProcessPrefixOffenceSkills.clear();

	//Calculate prefix skill activation probability.
	PREFIX_SKILL_INFO::iterator iter = m_PrefixOffenceSkills.begin();
	PREFIX_SKILL_INFO::iterator endIter = m_PrefixOffenceSkills.end();
	for (; iter != endIter; ++iter)
	{
		CDnPrefixSkill* pPrefixSkill = iter->second;
		if (!pPrefixSkill)
			continue;

		//Check whether prefix skill is on cooldown (weapon/armor distinguished??).
		if (m_PrefixSkillCoolTimeManager.IsCoolTime(pPrefixSkill->GetSkillType()))
		{
			OutputDebug("Prefix Attack Skill(%d) on cooldown!!!!!!!!!!\n", pPrefixSkill->GetSkillType());
			continue;
		}

		//Check prefix skill activation probability.
		bool bExecuteable = rand() % 10000 <= pPrefixSkill->GetProbability();
		if (!bExecuteable)
		{
			OutputDebug("Prefix Attack Skill(%d) filtered out by probability...!!!!\n", pPrefixSkill->GetSkillType());
			continue;
		}

		//Representative skill
		DnSkillHandle hSkill = pPrefixSkill->GetSkillHandle();
		if (!hSkill)
			continue;

		//Start Skill cooldown?
		hSkill->OnBeginCoolTime();
		m_PrefixSkillCoolTimeManager.AddCoolTime(pPrefixSkill);

		int nStateEffectCount = pPrefixSkill->GetStateEffectCount();
		// Determine the target actor per state effect of the skill, check for duplicate processing,
		// Seems state effects need to be applied per actor...
		for( int k = 0; k < pPrefixSkill->GetStateEffectCount(); k++ ) 
		{
			CDnSkill::StateEffectStruct *pLocalStruct = pPrefixSkill->GetStateEffectFromIndex(k);

			DnActorHandle hApplyActor;

			// Determines the actor to apply to based on the state effect target type.
			switch( pLocalStruct->ApplyType )
			{
			case CDnSkill::ApplySelf:	// To self
				hApplyActor = m_pActor->GetActorHandle();
				break;

			case CDnSkill::ApplyTarget: // To target
				continue;

			case CDnSkill::ApplyEnemy:	// To enemy.. (if not an ally, the one who hit me is the enemy..)
				continue;
				break;

			case CDnSkill::ApplyFriend: // Same side.. (if same side, is the one who hit me on the same side..?)
				continue;
				break;
			}

			// Among state effects, those with 0 duration and cast on self
			// Store in the skill's m_vlApplyNoPacketStateEffectList and when the skill ends
			// Ensure it gets deleted...[2011/01/13 semozz]
			bool isSelfPermentStateEffect = false;
			if (CDnSkill::ApplySelf == pLocalStruct->ApplyType && 
				(0 == pLocalStruct->nDurationTime ||
				-1 == pLocalStruct->nDurationTime))
			{
				pLocalStruct->nDurationTime = -1;
				isSelfPermentStateEffect = true;
			}

			CDnStateBlow *pStateBlow = hApplyActor->GetStateBlow();
			CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;

			if (pStateBlow)
				eResult = pStateBlow->CanApplySkillStateEffect(hSkill->GetInfo(), *pLocalStruct);

			if (CDnSkill::CanApply::Fail != eResult)
			{
				hApplyActor->RemoveResetStateBlow();

				int nBlowID = hApplyActor->CmdAddStateEffect( hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID, 
					pLocalStruct->nDurationTime, pLocalStruct->szValue.c_str(), false, false );

				//
				OutputDebug("Prefix Attack Skill(%d) State Effect applied to self (%d)-(%s)\n", pPrefixSkill->GetSkillType(), pLocalStruct->nID, pLocalStruct->szValue.c_str());

				if (isSelfPermentStateEffect)
					hSkill->AddPrefixBlow(hApplyActor, nBlowID);
			}

			hApplyActor->InitStateBlowIDToRemove();
		}

		//Store the used defense skill in the list.
		m_listProcessPreFixOffenceSkills.push_back(hSkill);

		//Stores the currently triggered offensive prefix skill.
		m_ProcessPrefixOffenceSkills.push_back(pPrefixSkill);
	}
}

void MASkillUser::ApplyPrefixOffenceSkillToTarget_New(DnActorHandle hTarget)
{
	// Among the skills in the final list, use only the skill stored in the highest-priority list.
	if (m_ProcessPrefixOffenceSkills.empty())
		return;

	list<CDnPrefixSkill*>::iterator iter = m_ProcessPrefixOffenceSkills.begin();
	list<CDnPrefixSkill*>::iterator endIter = m_ProcessPrefixOffenceSkills.end();

	for (; iter != endIter; ++iter)
	{
		CDnPrefixSkill* pPrefixSkill = (*iter);
		if (!pPrefixSkill)
			continue;

		DnSkillHandle hSkill = pPrefixSkill->GetSkillHandle();
		if (!hSkill)
			continue;

		int nStateEffectCount = pPrefixSkill->GetStateEffectCount();
		// Determine the target actor per state effect of the skill, check for duplicate processing,
		// Seems state effects need to be applied per actor...
		for( int k = 0; k < pPrefixSkill->GetStateEffectCount(); k++ ) 
		{
			CDnSkill::StateEffectStruct *pLocalStruct = pPrefixSkill->GetStateEffectFromIndex(k);

			DnActorHandle hApplyActor;

			// Determines the actor to apply to based on the state effect target type.
			switch( pLocalStruct->ApplyType )
			{
			case CDnSkill::ApplySelf:	// To self
				continue;
			case CDnSkill::ApplyTarget: // To target
				hApplyActor = hTarget;
				break;

			case CDnSkill::ApplyEnemy:	// To enemy.. (if not an ally, the one who hit me is the enemy..)
				if( m_pActor->GetTeam() == hTarget->GetTeam() )
					continue;
				else
					hApplyActor = hTarget;
				break;

			case CDnSkill::ApplyFriend: // Same side.. (if same side, is the one who hit me on the same side..?)
				if( m_pActor->GetTeam() != hTarget->GetTeam() )
					continue;
				else
					hApplyActor = hTarget;
				break;
			}

			// Among state effects, those with 0 duration and cast on self
			// Store in the skill's m_vlApplyNoPacketStateEffectList and when the skill ends
			// Ensure it gets deleted...[2011/01/13 semozz]
			bool isSelfPermentStateEffect = false;
			if (CDnSkill::ApplySelf == pLocalStruct->ApplyType && 
				(0 == pLocalStruct->nDurationTime ||
				-1 == pLocalStruct->nDurationTime))
			{
				pLocalStruct->nDurationTime = -1;
				isSelfPermentStateEffect = true;
			}

			CDnStateBlow *pStateBlow = hApplyActor->GetStateBlow();
			CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;

			if (pStateBlow)
				eResult = pStateBlow->CanApplySkillStateEffect(hSkill->GetInfo(), *pLocalStruct);

			if (CDnSkill::CanApply::Fail != eResult)
			{
				hApplyActor->RemoveResetStateBlow();

				int nBlowID = hApplyActor->CmdAddStateEffect( hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID, 
					pLocalStruct->nDurationTime, pLocalStruct->szValue.c_str(), false, false );

				OutputDebug("Prefix Attack Skill(%d) State Effect applied to target (%d)-(%s)\n", pPrefixSkill->GetSkillType(), pLocalStruct->nID, pLocalStruct->szValue.c_str());

				if (isSelfPermentStateEffect)
					hSkill->AddPrefixBlow(hApplyActor, nBlowID);
			}

			hApplyActor->InitStateBlowIDToRemove();
		}
	}

	//Clean up suspicious dump-related code
	//Apply and clear the list..
	m_ProcessPrefixOffenceSkills.clear();
}

#endif // _GAMESERVER
#endif // PRE_ADD_PREFIX_SYSTE_RENEW


void MASkillUser::SkillToggle(DnSkillHandle hSkill, bool isOn)
{
	if (!hSkill)	return;

#if defined(_CLIENT)
	CDnSkill::DurationTypeEnum eDurationType = hSkill->GetDurationType();
	if (eDurationType == CDnSkill::DurationTypeEnum::SummonOnOff)
	{
		hSkill->SummonMonsterOff();
	}
#endif // _CLIENT

	int skillID = hSkill->GetClassID();
	TOGGLESKILLLIST::iterator findIter = m_ToggleOnSkillList.find(skillID);
	
	if (isOn)
	{
		//Skill already registered..
		if (findIter != m_ToggleOnSkillList.end())
			return;

		m_ToggleOnSkillList.insert(std::make_pair(skillID, hSkill));
	}
	else
	{
		//If there is no registered skill
		if (findIter == m_ToggleOnSkillList.end())
			return;

		m_ToggleOnSkillList.erase(findIter);
	}
}


void MASkillUser::SummonOnOffSkillInit(DWORD dwSummonMonsterID)
{
	DNVector(DnSkillHandle) summonOnOffSkills;
#if defined(PRE_FIX_SKILLLIST)
	for( DWORD i=0; i<m_vlSkillObjects.size(); i++ ) 
	{
		DnSkillHandle hSkill = m_vlSkillObjects[i].hSkill;
		if (hSkill && hSkill->GetDurationType() == CDnSkill::DurationTypeEnum::SummonOnOff)
			summonOnOffSkills.push_back(hSkill);
	}
#endif // PRE_FIX_SKILLLIST

	for (DWORD i =0; i < summonOnOffSkills.size(); ++i)
	{
		DnSkillHandle hSkill = summonOnOffSkills[i];
		if (hSkill && hSkill->GetSummonMonsterID() == dwSummonMonsterID)
			hSkill->EnableToggle(false);
	}
}


#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
void MASkillUser::UpdateGlobalCoolTime(float fRate)
{
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		if(m_vlSkillObjects.at( i ).hSkill)
			m_vlSkillObjects.at( i ).hSkill->UpdateGlobalCoolTime(fRate);
	}
}

void MASkillUser::ResetGlobalCoolTime(float fRate)
{
	for( DWORD i = 0; i < m_vlSkillObjects.size(); ++i )
	{
		if(m_vlSkillObjects.at( i ).hSkill)
			m_vlSkillObjects.at( i ).hSkill->ResetGlobalCoolTime(fRate);
	}
}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL