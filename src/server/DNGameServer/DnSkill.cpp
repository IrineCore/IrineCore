#include "stdafx.h"
#include "DnSkill.h"
#include "DnTableDB.h"
#include "DnItemTask.h"
#include "IDnSkillUsableChecker.h"
#include "IDnSkillProcessor.h"
#include "DnDivideSEArgumentByTargets.h"
#include "DnPlayerActor.h"
#include "DnBlow.h"
#include "TaskManager.h"
#include "DnPartyTask.h"
#include "DNUserSession.h"
#include "DnPlayAniProcess.h"
#include "DnCoolTimeChargingProcess.h"
#include "DnPartialPlayProcessor.h"
#include "DnApplySEWhenActionSetBlowEnabledProcessor.h"
#include "DnChangeActionStrProcessor.h"
#include "DnChangeActionStrByBubbleProcessor.h"
#include "DnStateEffectApplyOnOffByBubbleProcessor.h"
#include "DnStateBlow.h"
#include "DnBubbleSystem.h"
#include "DnObserverEventMessage.h"
#include "DnPingpongBlow.h"
#include "DnBasicBlow.h"
#include "DnHealingBlow.h"
#include "DnHPIncBlow.h"
#include "DnInvincibleAtBlow.h"
#include "DnProbabilityChecker.h"
#include "DnCreateBlow.h"
#include "DnAllowedSkillsBlow.h"
#include "DnMonsterActor.h"
#include "DnProjectile.h"
#include "DnAdditionalStateInfoBlow.h"
#include "DnAddStateBySkillGroupBlow.h"
#include "DnTransformBlow.h"
#include "DnBloodSuckingBlow.h"
#include "DnOrderMySummonedMonsterBlow.h"

#if defined(PRE_FIX_NEXTSKILLINFO)
#include "DNGameDataManager.h"
#endif // PRE_FIX_NEXTSKILLINFO

#if defined(PRE_FIX_46381)
#include "DnContinueBaseMPIncBlow.h"
#endif // PRE_FIX_46381

using namespace BubbleSystem;
using namespace boost;

DECL_MULTISMART_PTR_STATIC( CDnSkill, MAX_SESSION_COUNT, 100 )


CDnSkill::CDnSkill( DnActorHandle hActor ) : CMultiSmartPtrBase< CDnSkill, MAX_SESSION_COUNT >(hActor->GetRoom()),
											 m_hActor( hActor ),
											 m_LastTimeToggleMPDecreaseTime( 0 ),
											 m_bToggle( false ),
											 m_bAura( false ),
											 m_bItemSkill( false ),
											 m_fPassiveActionSkillLength( 0.0f ),
											 m_eElement( CDnState::ElementEnum_Amount ),
											 m_iNextLevelSkillPoint( -1 ),
											 m_bChainingPassiveSkill( false ),
											 m_bAppliedPassiveSelfBlows( false ),
											 m_bTempSkill( false )
{
	SecureZeroMemory( m_iNeedItemID, sizeof(m_iNeedItemID) );
	SecureZeroMemory( m_iNeedItemDecreaseCount, sizeof(m_iNeedItemDecreaseCount) );
	SecureZeroMemory( m_fHPConsumeType, sizeof(m_fHPConsumeType) );
	SecureZeroMemory( m_fMPConsumeType, sizeof(m_fMPConsumeType) );
	SecureZeroMemory( m_iNeedHP, sizeof(m_iNeedHP) );
	SecureZeroMemory( m_iNeedMP, sizeof(m_iNeedMP) );
	SecureZeroMemory( m_iIncreaseRange, sizeof(m_iIncreaseRange) );
	SecureZeroMemory( m_iDecreaseHP, sizeof(m_iDecreaseHP) );
	SecureZeroMemory( m_iDecreaseMP, sizeof(m_iDecreaseMP) );
	SecureZeroMemory( m_fOriginalDelayTime, sizeof(m_fOriginalDelayTime) );
	SecureZeroMemory( m_fDelayTime, sizeof(m_fDelayTime) );

	m_iSkillID = 0;
	m_iSkillLevelID = 0;
	m_iDissolvable = 0;
	m_iDuplicateCount = 0;
	m_iSkillDuplicateMethod = 0;
	m_iEffectDuplicateMethod = 0;

	m_iLevel = 0;
	m_iMaxLevel = 0;

	m_eSkillType = SkillTypeEnum::Active;
	m_eDurationType = DurationTypeEnum::Instantly;
	m_eTargetType = TargetTypeEnum::Self;
	m_iLevelLimit = 0;

	m_fLeftDelayTime = 0.f;
	m_fCoolTime = 0.f;
	m_bUseCoolTimeCharging = false;
	m_nMaxCoolTimeChargeCount = 0;
	m_nCurCoolTimeChargeCount = 0;
	m_iAdditionalThreat = 0;

	m_iCPScore = 0;

	m_pEffectAction = NULL;
	
	m_fOnceDelayTime = 0.0f;
	m_fOnceElapsedDelayTime = 0.0f;
	m_bEquipItemSkill = false;
	m_iEquipIndex = -1;

	m_fCoolTimeAdjustBlowValue = 1.0f;

	m_fStartSuperArmor = 0.0f;
	m_bStartCanHit = true;

	m_fResetCooltime = 0.0f;

	m_dwLastUseSkillTimeStamp = 0;

	m_iExclusiveID = 0;

	m_iSelectedSkillLevelDataApplyType = PVE;		// Default is PvE.

	m_iBaseSkillID = 0;
	m_iAppliedEnchantPassiveSkillID = 0;

	m_nPriority = 0;

	m_nItemID = -1;

	m_fCoolTimeMultipier = 1.0f;

	m_isIgnoreImmuneBackup = false;

	m_nLevelUpValue = 0;

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
	m_nPrefixSkillType = -1;
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

	m_iSkillGroupID = 0;
	m_iGlobalSkillGroupID = 0;
	SecureZeroMemory( m_afGlobalCoolTime, sizeof(m_afGlobalCoolTime) );
	m_fAnotherGlobalSkillCoolTime = 0.0f;

	m_nAnotherGlobakSkillID = 0;

#if defined( PRE_ADD_ACADEMIC )
	m_iSummonerDecreaseSP = 0;
	m_iSummonerDecreaseSPSkillID = 0;
#endif // #if defined( PRE_ADD_ACADEMIC )

	m_SummonMonsterID = -1;

	m_bAddStateEffectQueue = false;

	m_bEnchantedFromBubble = false;

	m_SkillStartTime = 0;
	m_bFinished = false;
	m_bIsPrefixTriggerSkill = false;
	m_iNeedJobClassID = 0;
	m_iNowLevelSkillPoint = 0;

	memset (m_iOriginalNeedMP, 0x00, sizeof(m_iOriginalNeedMP));
	memset (m_aeNeedEquipType, 0x00, sizeof(m_aeNeedEquipType));

#if defined(PRE_FIX_64312)
	m_isAppliedSummonMonsterEnchantSkill = false;
	m_bIsSummonMonsterSkill = false;
#endif // PRE_FIX_64312

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	m_fDeltaGlobalCoolTime = 0.0f;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
}

CDnSkill::~CDnSkill(void)
{
	for( int iSelectedLevelData = PVE; iSelectedLevelData < NUM_SKILLLEVEL_APPLY_TYPE; ++iSelectedLevelData )
	{
		SAFE_DELETE_PVEC( m_vlpUsableCheckers[ iSelectedLevelData ] );
	}

	for( int iSelectedLevelData = PVE; iSelectedLevelData < NUM_SKILLLEVEL_APPLY_TYPE; ++iSelectedLevelData )
	{
		SAFE_DELETE_PVEC( m_vlpProcessors[ iSelectedLevelData ] );
	}

	for( int iSelectedLevelData = PVE; iSelectedLevelData < NUM_SKILLLEVEL_APPLY_TYPE; ++iSelectedLevelData )
	{
		SAFE_DELETE_PVEC( m_vlpProcessorBackup[ iSelectedLevelData ] );
	}

#if defined(PRE_FIX_66175)
	for( int iSelectedLevelData = PVE; iSelectedLevelData < NUM_SKILLLEVEL_APPLY_TYPE; ++iSelectedLevelData )
	{
		SAFE_DELETE_PVEC( m_vlUsableCheckersBackup[ iSelectedLevelData ] );
	}
#endif // PRE_FIX_66175
}

void CDnSkill::SetHasActor( DnActorHandle hActor )
{
	_ASSERT( hActor && "CDnSkill::SetHasActor() actor handle is NULL" );

	m_hActor = hActor;

	for( int iSelectedLevelData = PVE; iSelectedLevelData < NUM_SKILLLEVEL_APPLY_TYPE; ++iSelectedLevelData )
	{
		int iNumChecker = (int)m_vlpUsableCheckers[ iSelectedLevelData ].size();
		for( int iChecker = 0; iChecker < iNumChecker; ++iChecker )
		{
			IDnSkillUsableChecker* pChecker = m_vlpUsableCheckers[ iSelectedLevelData ].at( iChecker );
			pChecker->SetHasActor( hActor );
		}
	}

	for( int iSelectedLevelData = PVE; iSelectedLevelData < NUM_SKILLLEVEL_APPLY_TYPE; ++iSelectedLevelData )
	{
		int iNumProcessor = (int)m_vlpProcessors[ iSelectedLevelData ].size();
		for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
		{
			IDnSkillProcessor* pProcessor = m_vlpProcessors[ iSelectedLevelData ].at( iProcessor );
			pProcessor->SetHasActor( hActor );
			pProcessor->SetParentSkill( GetMySmartPtr() );
		}
	}
}


DnSkillHandle CDnSkill::CreateSkill( DnActorHandle hActor, int iSkillTableID, int iLevel )
{
	CDnSkill* pNewSkill = NULL;

	// Attach skill activation conditions and activation processors.
	// 5 each, 10 parameters in total. Subject to change.
	pNewSkill = new CDnSkill( hActor );
	
	bool bResult = pNewSkill->Initialize( iSkillTableID, iLevel );
	if( false == bResult )
	{
		delete pNewSkill;
		return CDnSkill::Identity();
	}

	if( hActor && hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		pNewSkill->RegisterObserver( pPlayerActor->GetBubbleSystem() );
	}
	
	return pNewSkill->GetMySmartPtr();
}

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
bool CDnSkill::_LoadMonsterSkillLevelData( int iSkillTableID, int iLevel, int iSkillLevelDataApplyType )
{
	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat* pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

	vector<int> vlSkillLevelList;
	if( pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", iSkillTableID, vlSkillLevelList ) <= 0 ) 
		return false;

	if( !pSkillTable->IsExistItem( iSkillTableID) )
		return false;

	// Check and filter whether the target is PvE or PvP.
	vector<int>::iterator iterLevelList = vlSkillLevelList.begin();
	for( iterLevelList; iterLevelList != vlSkillLevelList.end(); )
	{
		int iSkillLevelTableID = *iterLevelList;
		int iApplyType = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_ApplyType" )->GetInteger();
		if( iApplyType != iSkillLevelDataApplyType )
			iterLevelList = vlSkillLevelList.erase( iterLevelList );
		else
			++iterLevelList;
	}

	// If there is no data for the target, simply do nothing and return.
	if( vlSkillLevelList.empty() )
		return true;

	int iSkillLevelTableID = -1;
	for( int i = 0; i < (int)vlSkillLevelList.size(); ++i )
	{
		int iNowLevel = pSkillLevelTable->GetFieldFromLablePtr( vlSkillLevelList.at(i), "_SkillLevel" )->GetInteger();
		if( iNowLevel == iLevel )
		{
			iSkillLevelTableID = vlSkillLevelList.at( i );
			break;
		}
	}

	if( -1 == iSkillLevelTableID )
		return false;

	char caLabel[ 32 ];
	int iCheckerParamOffset = 0;
	int iProcessorParamOffset = 0;
	for( int i = 0; i < MAX_PROCESSOR_COUNT; ++i )
	{
		// Find the activation condition object name. If the parameter field is empty, creation functions return NULL
		sprintf_s( caLabel, "_UsableChecker%d", i + 1 );
		int iUsableChecker = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLabel )->GetInteger();

		sprintf_s( caLabel, "_Processor%d", i + 1 );
		int iProcessor = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLabel )->GetInteger();

		int iOffsetCheck = iCheckerParamOffset;
		IDnSkillUsableChecker* pUsableChecker = IDnSkillUsableChecker::Create( m_hActor, iUsableChecker, iSkillLevelTableID, &iCheckerParamOffset );

		if( NULL != pUsableChecker )
		{
			if( (iCheckerParamOffset - iOffsetCheck) != pUsableChecker->GetNumArgument() )
			{
				OutputDebug( "[SkillLevelTable Error!] Invalid parameter count for skill level row %d.\n", iSkillLevelTableID );
				_ASSERT( !"Invalid SkillLevelTable parameters. Check debug output!" );
			}

			this->AddUsableCheckers( pUsableChecker, iSkillLevelDataApplyType );
		}

		iOffsetCheck = iProcessorParamOffset;
		IDnSkillProcessor* pSkillProcessor = IDnSkillProcessor::Create( m_hActor, iProcessor, iSkillLevelTableID, &iProcessorParamOffset, this->GetUseActionNames() );

		if( NULL != pSkillProcessor )
		{
			if( (iProcessorParamOffset - iOffsetCheck) != pSkillProcessor->GetNumArgument() )
			{
				OutputDebug( "[SkillLevelTable Error!] Invalid parameter count for skill level row %d.\n", iSkillLevelTableID );
				_ASSERT( !"Invalid SkillLevelTable parameters. Check debug output!" );
			}

			this->AddProcessor( pSkillProcessor, iSkillLevelDataApplyType );
			if( pSkillProcessor->GetType() == IDnSkillProcessor::COOLTIME_CHARGING )
			{
				CDnCoolTimeChargingProcess* pChargeProc = static_cast<CDnCoolTimeChargingProcess*>( pSkillProcessor );
				if( pChargeProc && pChargeProc->GetMaxChargeCount() > 1 )
				{
					m_bUseCoolTimeCharging = true;
					m_nMaxCoolTimeChargeCount = pChargeProc->GetMaxChargeCount();
					m_nCurCoolTimeChargeCount = m_nMaxCoolTimeChargeCount;
				}
			}
		}
	}

	// skill table
	m_strStaticName = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_StaticName" )->GetString();

	char caLable[ 64 ];
	ZeroMemory( caLable, sizeof(caLable) );
	for( int i = 0; i < 2; ++i )
	{
		sprintf_s( caLable,  "_NeedWeaponType%d", i+1 );
		int iEquipType = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLable )->GetInteger();
		if( -1 != iEquipType )
			m_aeNeedEquipType[ i ] = (CDnWeapon::EquipTypeEnum)iEquipType;
		else
			m_aeNeedEquipType[ i ] = CDnWeapon::EquipTypeEnum_Amount;
	}

	// Max level in the skill table cannot be trusted. -_-
	// Update with actual count.
	if( 0 == m_iMaxLevel )
	{
		m_iMaxLevel = (int)vlSkillLevelList.size();
	}
	else
	{
		_ASSERT( m_iMaxLevel == (int)vlSkillLevelList.size() );
	}

	m_eSkillType = (SkillTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_SkillType" )->GetInteger();
	m_eDurationType = (DurationTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_DurationType" )->GetInteger();
	m_eTargetType = (TargetTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_TargetType" )->GetInteger();

	m_iDissolvable = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_Dissolvable" )->GetInteger();
	m_iDuplicateCount = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_EffectAmassCount" )->GetInteger();
	m_iSkillDuplicateMethod = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_SkillDuplicate" )->GetInteger();
	m_iEffectDuplicateMethod = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_EffectDuplicate" )->GetInteger();
	m_eElement = (CDnState::ElementEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_Element" )->GetInteger();
	m_iNeedJobClassID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_NeedJob" )->GetInteger();
	if( (CDnState::ElementEnum)-1 == m_eElement )
		m_eElement = CDnState::ElementEnum_Amount;

	// skill level table
	m_iSkillLevelID = iSkillLevelTableID;
	m_iLevel = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SkillLevel" )->GetInteger();
	m_iNowLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_NeedSkillPoint" )->GetInteger();

#if defined(PRE_FIX_NEXTSKILLINFO)
	SKILL_LEVEL_INFO* pTabelInfo = g_pDataManager->GetSkillLevelTableIDList(iSkillTableID, iSkillLevelDataApplyType);
	int nextSkillLevel = m_iLevel + 1;

	if( m_iLevel < m_iMaxLevel )
	{
		int nextLevelTableID = -1;
		SKILL_LEVEL_TABLE_IDS::iterator findIter = pTabelInfo->_SkillLevelTableIDs.find(nextSkillLevel);
		if (findIter != pTabelInfo->_SkillLevelTableIDs.end())
			nextLevelTableID = findIter->second;

		m_iNextLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( nextLevelTableID, "_NeedSkillPoint" )->GetInteger();
	}
	else
		m_iNextLevelSkillPoint = 0;

#else
	if( m_iLevel < m_iMaxLevel )
		m_iNextLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID+1, "_NeedSkillPoint" )->GetInteger();
	else
		m_iNextLevelSkillPoint = 0;
#endif // PRE_FIX_NEXTSKILLINFO

	m_iIncreaseRange[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_AddRange" )->GetInteger();

	m_iAdditionalThreat = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_AddThreat" )->GetInteger();

	m_iCPScore = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_CPScore" )->GetInteger();
	m_fStartSuperArmor = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_StartSuperArmor" )->GetFloat();
	m_bStartCanHit = (pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_StartCanHit" )->GetInteger() == 1) ? true : false;

	// Load state effect info
	StateEffectStruct StateEffect;
	for( int i = 0; i < MAX_STATE_EFFECT_COUNT; ++i )
	{
		sprintf_s( caLable, "_EffectClass%d", i + 1 );
		StateEffect.nID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLable )->GetInteger();

		if( StateEffect.nID < 1 ) 
			continue;

		sprintf_s( caLable, "_EffectClass%dApplyType", i + 1 );
		int iApplyType = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLable )->GetInteger();
		// Apply-to-all. Add 2 identical state effects with different targets.
		// However, skills used as enhancement passives are created singly rather than as 2, since they only serve as data to enhance other skills.
		bool bApplyAll = (StateEffectApplyType::ApplyAll == iApplyType) && (SkillTypeEnum::EnchantPassive != m_eSkillType);

		if( bApplyAll )
			StateEffect.ApplyType = StateEffectApplyType::ApplySelf;
		else
			StateEffect.ApplyType = (StateEffectApplyType)iApplyType;

		sprintf_s( caLable, "_EffectClassValue%d", i + 1 );
		StateEffect.szValue = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetString();

		sprintf_s( caLable, "_EffectClassValue%dDuration", i + 1 );
		StateEffect.nDurationTime = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetInteger();

		m_vlStateEffectList[ iSkillLevelDataApplyType ].push_back( StateEffect );

		// If apply-to-all, switch to target and add the same one once more.
		if( bApplyAll )
		{
			StateEffect.ApplyType = StateEffectApplyType::ApplyTarget;
			m_vlStateEffectList[ iSkillLevelDataApplyType ].push_back( StateEffect );
		}
	}

	SkillInfo& MySkillInfo = m_SkillInfo[ iSkillLevelDataApplyType ];
	MySkillInfo.iSkillID = m_iSkillID;
	MySkillInfo.iSkillLevelID = m_iSkillLevelID;
	MySkillInfo.iLevel = m_iLevel;
	MySkillInfo.iSkillDuplicateMethod = m_iSkillDuplicateMethod;
	MySkillInfo.iDuplicateCount = m_iDuplicateCount;

	MySkillInfo.eSkillType = m_eSkillType;
	MySkillInfo.eDurationType = m_eDurationType;
	MySkillInfo.eTargetType = m_eTargetType;
	MySkillInfo.eApplyType = StateEffect.ApplyType;
	MySkillInfo.iDissolvable = m_iDissolvable;
	MySkillInfo.eSkillElement = m_eElement;
	MySkillInfo.hSkillUser = m_hActor;				// Note: Actor is not always valid
	if( m_hActor )
		MySkillInfo.iSkillUserTeam = m_hActor->GetTeam();

	if( m_hActor && _tcslen(m_hActor->GetName()) > 0 )
		MySkillInfo.strUserName = m_hActor->GetName();

	MySkillInfo.szEffectOutputIDs = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_StateEffectTableID" )->GetString();

	return true;
}

bool CDnSkill::InitializeMonsterSkill( int iSkillTableID, int iLevel )
{
	m_iSkillID = iSkillTableID;

	// Load data for each mode. Read PVP first.
	// Since PVE is the default value, values not separated by mode are filled with the default value.

	// Since this is a monster skill creation function, only call in PVE.
	if( false == _LoadSkillLevelData( iSkillTableID, iLevel, PVP ) )
		return false;

	if( false == _LoadSkillLevelData( iSkillTableID, iLevel, PVE ) )
		return false;

	int iNumProcessor = (int)m_vlpProcessors[ PVE ].size();
	for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
	{
		IDnSkillProcessor* pProcessor = m_vlpProcessors[ PVE ].at( iProcessor );

		// If an action is in use, populate data for external lookup
		if( pProcessor->GetType() == IDnSkillProcessor::PLAY_ANI )
			m_setUseActionNames.insert( static_cast<CDnPlayAniProcess*>(pProcessor)->GetActionName() );
		else
			if( pProcessor->GetType() == IDnSkillProcessor::PARTIAL_PLAY_ANI )
			{
				CDnPartialPlayProcessor* pPartialPlayAni = static_cast<CDnPartialPlayProcessor*>(pProcessor);
				m_setUseActionNames.insert( pPartialPlayAni->GetStartActionName() );
				m_setUseActionNames.insert( pPartialPlayAni->GetLoopActionName() );
				m_setUseActionNames.insert( pPartialPlayAni->GetEndActionName() );
			}
	}

	// When initialized, PvE mode is the default.
	m_iSelectedSkillLevelDataApplyType = PVE;

	return true;
}

DnSkillHandle CDnSkill::CreateMonsterSkill( DnActorHandle hActor, int iSkillTableID, int iLevel )
{
	if( hActor )
	{
		CDnSkill* pNewSkill = pNewSkill = new CDnSkill( hActor );
		if( pNewSkill )
		{
			bool bResult = pNewSkill->InitializeMonsterSkill( iSkillTableID, iLevel );
			if( false == bResult )
			{
				SAFE_DELETE( pNewSkill );
				return CDnSkill::Identity();
			}

			if( hActor->IsPlayerActor() )
			{
				CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
				pNewSkill->RegisterObserver( pPlayerActor->GetBubbleSystem() );
			}

			return pNewSkill->GetMySmartPtr();
		}
		else
		{
			SAFE_DELETE( pNewSkill );
			return CDnSkill::Identity();
		}
	}

	return CDnSkill::Identity();
}
#endif // #ifdef PRE_FIX_GAMESERVER_OPTIMIZE

void CDnSkill::CreateSkillInfo( int nSkillID, int nSkillLevel, CDnSkill::SkillInfo & sSkillInfo, std::vector<CDnSkill::StateEffectStruct> & vecSkillEffect, bool bUseBattleGround/* = false*/ )
{
	if( nSkillID == 0 || nSkillLevel == 0 )
		return;

	DNTableFileFormat* pSkillTable = NULL;
	DNTableFileFormat* pSkillLevelTable = NULL;
	if (bUseBattleGround)
	{
		pSkillTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILL );
		pSkillLevelTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILLLEVEL );
	}
	else
	{
		pSkillTable = GetDNTable( CDnTableDB::TSKILL );
		pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );
	}

	if (pSkillLevelTable == NULL || pSkillTable == NULL)
	{
		_DANGER_POINT();
		return;
	}
	std::vector<int> vlSkillLevelList;
	if( pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", nSkillID, vlSkillLevelList ) <= 0 ) 
		return;

	int iSkillLevelTableID = -1;
	for( int i = 0; i < (int)vlSkillLevelList.size(); ++i )
	{
		int iNowLevel = pSkillLevelTable->GetFieldFromLablePtr( vlSkillLevelList.at(i), "_SkillLevel" )->GetInteger();
		if( iNowLevel == nSkillLevel )
		{
			iSkillLevelTableID = vlSkillLevelList.at( i );
			break;
		}
	}

	if( -1 == iSkillLevelTableID )
		return;

	sSkillInfo.iSkillID = nSkillID;
	sSkillInfo.iSkillLevelID = iSkillLevelTableID;
#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
	sSkillInfo.iAppliedEnchantSkillID = 0;
#endif
	sSkillInfo.iLevel = nSkillLevel;
	sSkillInfo.eDurationType = (CDnSkill::DurationTypeEnum)pSkillTable->GetFieldFromLablePtr( nSkillID, "_DurationType" )->GetInteger();
	sSkillInfo.eTargetType = (CDnSkill::TargetTypeEnum)pSkillTable->GetFieldFromLablePtr( nSkillID, "_TargetType" )->GetInteger();
	sSkillInfo.iSkillDuplicateMethod = pSkillTable->GetFieldFromLablePtr( nSkillID, "_SkillDuplicate" )->GetInteger();
	sSkillInfo.iDuplicateCount = pSkillTable->GetFieldFromLablePtr( nSkillID, "_EffectAmassCount" )->GetInteger();
	sSkillInfo.iDissolvable = pSkillTable->GetFieldFromLablePtr( nSkillID, "_Dissolvable" )->GetInteger();
	sSkillInfo.szEffectOutputIDToClient = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_StateEffectTableID" )->GetString();

	sSkillInfo.bFromBuffProp = true;

	// Load state effect info
	CDnSkill::StateEffectStruct StateEffect;
	char caLable[ 64 ];
	ZeroMemory( caLable, sizeof(caLable) );
	for( int i = 0; i < MAX_STATE_EFFECT_COUNT; ++i )
	{
		sprintf_s( caLable, "_EffectClass%d", i + 1 );
		StateEffect.nID = pSkillTable->GetFieldFromLablePtr( nSkillID, caLable )->GetInteger();

		if( StateEffect.nID < 1 ) 
			continue;

		sprintf_s( caLable, "_EffectClass%dApplyType", i + 1 );
		StateEffect.ApplyType = (CDnSkill::StateEffectApplyType)pSkillTable->GetFieldFromLablePtr( nSkillID, caLable )->GetInteger();

		sprintf_s( caLable, "_EffectClassValue%d", i + 1 );
		StateEffect.szValue = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetString();

		sprintf_s( caLable, "_EffectClassValue%dDuration", i + 1 );
		StateEffect.nDurationTime = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetInteger();

		vecSkillEffect.push_back( StateEffect );
	}
}

void CDnSkill::CreateBattleGroundSkillInfo( int nSkillID, int nSkillLevel, CDnSkill::SkillInfo & sSkillInfo, std::vector<CDnSkill::StateEffectStruct> & vecSkillEffect )
{
	CreateSkillInfo(nSkillID, nSkillLevel, sSkillInfo, vecSkillEffect, true);
}

bool CDnSkill::_LoadSkillLevelData( int iSkillTableID, int iLevel, int iSkillLevelDataApplyType )
{
	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat* pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

	vector<int> vlSkillLevelList;
	if( pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", iSkillTableID, vlSkillLevelList ) <= 0 ) 
		return false;

	if( !pSkillTable->IsExistItem( iSkillTableID) )
		return false;

	// Check and filter whether the target is PvE or PvP.
	vector<int>::iterator iterLevelList = vlSkillLevelList.begin();
	for( iterLevelList; iterLevelList != vlSkillLevelList.end(); )
	{
		int iSkillLevelTableID = *iterLevelList;
		int iApplyType = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_ApplyType" )->GetInteger();
		if( iApplyType != iSkillLevelDataApplyType )
			iterLevelList = vlSkillLevelList.erase( iterLevelList );
		else
			++iterLevelList;
	}

	// If there is no data for the target, simply do nothing and return.
	if( vlSkillLevelList.empty() )
		return true;

	int iSkillLevelTableID = -1;
	for( int i = 0; i < (int)vlSkillLevelList.size(); ++i )
	{
		int iNowLevel = pSkillLevelTable->GetFieldFromLablePtr( vlSkillLevelList.at(i), "_SkillLevel" )->GetInteger();
		if( iNowLevel == iLevel )
		{
			iSkillLevelTableID = vlSkillLevelList.at( i );
			break;
		}
	}

	if( -1 == iSkillLevelTableID )
		return false;

#if defined(PRE_FIX_NEXTSKILLINFO)
	//NextLevel might not be contiguous.. So find and store the next level data table ID here.
	int iMinSkillLevelTableID = -1;
	int iNextSkillLevelTableID = -1;

	SKILL_LEVEL_INFO* pTableInfo = g_pDataManager->GetSkillLevelTableIDList(iSkillLevelTableID, iSkillLevelDataApplyType);
	SKILL_LEVEL_TABLE_IDS::iterator findIter = pTableInfo->_SkillLevelTableIDs.find(pTableInfo->_MinLevel);
	if (findIter != pTableInfo->_SkillLevelTableIDs.end())
		iMinSkillLevelTableID = findIter->second;

	findIter = pTableInfo->_SkillLevelTableIDs.find(iLevel + 1);
	if (findIter != pTableInfo->_SkillLevelTableIDs.end())
		iNextSkillLevelTableID = findIter->second;

#endif // PRE_FIX_NEXTSKILLINFO

	char caLabel[ 32 ];
	int iCheckerParamOffset = 0;
	int iProcessorParamOffset = 0;
	for( int i = 0; i < MAX_PROCESSOR_COUNT; ++i )
	{
		// Find the activation condition object name. If the parameter field is empty, creation functions return NULL
		sprintf_s( caLabel, "_UsableChecker%d", i + 1 );
		int iUsableChecker = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLabel )->GetInteger();

		sprintf_s( caLabel, "_Processor%d", i + 1 );
		int iProcessor = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLabel )->GetInteger();
		
		int iOffsetCheck = iCheckerParamOffset;
		IDnSkillUsableChecker* pUsableChecker = IDnSkillUsableChecker::Create( m_hActor, iUsableChecker, iSkillLevelTableID, &iCheckerParamOffset );

		if( NULL != pUsableChecker )
		{
			if( (iCheckerParamOffset - iOffsetCheck) != pUsableChecker->GetNumArgument() )
			{
				OutputDebug( "[SkillLevelTable Error!] Invalid parameter count for skill level row %d.\n", iSkillLevelTableID );
				_ASSERT( !"Invalid SkillLevelTable parameters. Check debug output!" );
			}

			this->AddUsableCheckers( pUsableChecker, iSkillLevelDataApplyType );
		}

		iOffsetCheck = iProcessorParamOffset;
		IDnSkillProcessor* pSkillProcessor = IDnSkillProcessor::Create( m_hActor, iProcessor, iSkillLevelTableID, &iProcessorParamOffset, this->GetUseActionNames() );

		if( NULL != pSkillProcessor )
		{
			if( (iProcessorParamOffset - iOffsetCheck) != pSkillProcessor->GetNumArgument() )
			{
				OutputDebug( "[SkillLevelTable Error!] Invalid parameter count for skill level row %d.\n", iSkillLevelTableID );
				_ASSERT( !"Invalid SkillLevelTable parameters. Check debug output!" );
			}

			this->AddProcessor( pSkillProcessor, iSkillLevelDataApplyType );
			if( pSkillProcessor->GetType() == IDnSkillProcessor::COOLTIME_CHARGING )
			{
				CDnCoolTimeChargingProcess* pChargeProc = static_cast<CDnCoolTimeChargingProcess*>( pSkillProcessor );
				if( pChargeProc && pChargeProc->GetMaxChargeCount() > 1 )
				{
					m_bUseCoolTimeCharging = true;
					m_nMaxCoolTimeChargeCount = pChargeProc->GetMaxChargeCount();
					m_nCurCoolTimeChargeCount = m_nMaxCoolTimeChargeCount;
				}
			}
		}
	}

	// skill table
	m_strStaticName = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_StaticName" )->GetString();

	char caLable[ 64 ];
	ZeroMemory( caLable, sizeof(caLable) );
	for( int i = 0; i < 2; ++i )
	{
		sprintf_s( caLable,  "_NeedWeaponType%d", i+1 );
		int iEquipType = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLable )->GetInteger();
		if( -1 != iEquipType )
			m_aeNeedEquipType[ i ] = (CDnWeapon::EquipTypeEnum)iEquipType;
		else
			m_aeNeedEquipType[ i ] = CDnWeapon::EquipTypeEnum_Amount;
	}

	// Max level in the skill table cannot be trusted. -_-
	// Update with actual count.
	if( 0 == m_iMaxLevel )
	{
		m_iMaxLevel = (int)vlSkillLevelList.size();
	}
	else
	{
		_ASSERT( m_iMaxLevel == (int)vlSkillLevelList.size() );
	}

	m_eSkillType = (SkillTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_SkillType" )->GetInteger();
	m_eDurationType = (DurationTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_DurationType" )->GetInteger();
	m_eTargetType = (TargetTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_TargetType" )->GetInteger();

	m_iDissolvable = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_Dissolvable" )->GetInteger();
	m_iDuplicateCount = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_EffectAmassCount" )->GetInteger();
	m_iSkillDuplicateMethod = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_SkillDuplicate" )->GetInteger();
	m_iEffectDuplicateMethod = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_EffectDuplicate" )->GetInteger();
	m_eElement = (CDnState::ElementEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_Element" )->GetInteger();
	m_iNeedJobClassID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_NeedJob" )->GetInteger();
	if( (CDnState::ElementEnum)-1 == m_eElement )
		m_eElement = CDnState::ElementEnum_Amount;

	m_iExclusiveID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_DuplicatedSkillType" )->GetInteger();
	m_iBaseSkillID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_BaseSkillID" )->GetInteger();
	DNTableCell* pSkillGroupCell = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_SkillGroup" );
	if( pSkillGroupCell )
		m_iSkillGroupID = pSkillGroupCell->GetInteger();

	m_iGlobalSkillGroupID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_GlobalSkillGroup" )->GetInteger();

	float fGlobalCoolTime = 0.0f;
	if( PVE == iSkillLevelDataApplyType )
		fGlobalCoolTime = (float)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_GlobalCoolTimePvE" )->GetInteger() / 1000.0f;
	else
	if( PVP == iSkillLevelDataApplyType )
		fGlobalCoolTime = (float)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_GlobalCoolTimePvP" )->GetInteger() / 1000.0f;

	m_afGlobalCoolTime[ iSkillLevelDataApplyType ] = fGlobalCoolTime;

	// skill level table
	m_iSkillLevelID = iSkillLevelTableID;
	m_iLevel = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SkillLevel" )->GetInteger();
	m_iNowLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_NeedSkillPoint" )->GetInteger();
	if( m_iLevel < m_iMaxLevel )
#if defined(PRE_FIX_NEXTSKILLINFO)
		m_iNextLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iNextSkillLevelTableID, "_NeedSkillPoint" )->GetInteger();
#else
		m_iNextLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID+1, "_NeedSkillPoint" )->GetInteger();
#endif // PRE_FIX_NEXTSKILLINFO
	else
		m_iNextLevelSkillPoint = 0;

	m_iNeedItemID[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_NeedItem" )->GetInteger();
	m_iNeedItemDecreaseCount[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_NeedItemDecreaseCount" )->GetInteger();
	m_iIncreaseRange[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_AddRange" )->GetInteger();
	m_iDecreaseHP[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_DecreaseHP" )->GetInteger();
	m_iDecreaseMP[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_DecreaseSP" )->GetInteger();
	m_iLevelLimit = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_LevelLimit" )->GetInteger();
	m_fDelayTime[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_DelayTime" )->GetInteger() / 1000.f;

	// If a global skill group is set, replace the skill cooldown with the global cooldown.
	if( 0 < m_iGlobalSkillGroupID )
		m_fDelayTime[ iSkillLevelDataApplyType ] = m_afGlobalCoolTime[ iSkillLevelDataApplyType ];

	m_fOriginalDelayTime[ iSkillLevelDataApplyType ] = m_fDelayTime[ iSkillLevelDataApplyType ];
	m_iAdditionalThreat = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_AddThreat" )->GetInteger();

	m_fHPConsumeType[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_HPConsumeType" )->GetFloat();
	m_fMPConsumeType[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SPConsumeType" )->GetFloat();
	m_iCPScore = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_CPScore" )->GetInteger();
	m_fStartSuperArmor = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_StartSuperArmor" )->GetFloat();
	m_bStartCanHit = (pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_StartCanHit" )->GetInteger() == 1) ? true : false;

	// Load state effect info
	StateEffectStruct StateEffect;
	for( int i = 0; i < MAX_STATE_EFFECT_COUNT; ++i )
	{
		sprintf_s( caLable, "_EffectClass%d", i + 1 );
		StateEffect.nID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLable )->GetInteger();

		if( StateEffect.nID < 1 ) 
			continue;

		sprintf_s( caLable, "_EffectClass%dApplyType", i + 1 );
		int iApplyType = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLable )->GetInteger();
		// Apply-to-all. Add 2 identical state effects with different targets.
		// However, skills used as enhancement passives are created singly rather than as 2, since they only serve as data to enhance other skills.
		bool bApplyAll = (StateEffectApplyType::ApplyAll == iApplyType) && (SkillTypeEnum::EnchantPassive != m_eSkillType);		

		if( bApplyAll )
		{
			StateEffect.ApplyType = StateEffectApplyType::ApplySelf;
			StateEffect.bApplyAllPair = true;
		}
		else
			StateEffect.ApplyType = (StateEffectApplyType)iApplyType;

		sprintf_s( caLable, "_EffectClassValue%d", i + 1 );
		StateEffect.szValue = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetString();

		sprintf_s( caLable, "_EffectClassValue%dDuration", i + 1 );
		StateEffect.nDurationTime = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetInteger();

		m_vlStateEffectList[ iSkillLevelDataApplyType ].push_back( StateEffect );

		// If apply-to-all, switch to target and add the same one once more.
		if( bApplyAll )
		{
			StateEffect.ApplyType = StateEffectApplyType::ApplyTarget;
			m_vlStateEffectList[ iSkillLevelDataApplyType ].push_back( StateEffect );
		}
	}

	SkillInfo& MySkillInfo = m_SkillInfo[ iSkillLevelDataApplyType ];
	MySkillInfo.iSkillID = m_iSkillID;
	MySkillInfo.iSkillLevelID = m_iSkillLevelID;
	MySkillInfo.iLevel = m_iLevel;
	MySkillInfo.iSkillDuplicateMethod = m_iSkillDuplicateMethod;
	MySkillInfo.iDuplicateCount = m_iDuplicateCount;

	MySkillInfo.eSkillType = m_eSkillType;
	MySkillInfo.eDurationType = m_eDurationType;
	MySkillInfo.eTargetType = m_eTargetType;
	MySkillInfo.eApplyType = StateEffect.ApplyType;
	MySkillInfo.iDissolvable = m_iDissolvable;
	MySkillInfo.eSkillElement = m_eElement;
	MySkillInfo.hSkillUser = m_hActor;				// Note: Actor is not always valid
	if( m_hActor )
		MySkillInfo.iSkillUserTeam = m_hActor->GetTeam();

	if( m_hActor && _tcslen(m_hActor->GetName()) > 0 )
		MySkillInfo.strUserName = m_hActor->GetName();

	MySkillInfo.szEffectOutputIDs = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_StateEffectTableID" )->GetString();

	if( 0.0f == m_fHPConsumeType[ iSkillLevelDataApplyType ] )
	{
		m_iNeedHP[ iSkillLevelDataApplyType ] = m_iDecreaseHP[ iSkillLevelDataApplyType ];
	}
	else
	{
		m_iNeedHP[ iSkillLevelDataApplyType ] = int((float)m_hActor->GetMaxHP() * m_fHPConsumeType[ iSkillLevelDataApplyType ]);
	}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
	m_nPrefixSkillType = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_Group" )->GetInteger();

	//Set suffix skill group ID in skill info... (Client does not need to know?...)
	SetPrefixSkillType(m_nPrefixSkillType);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

	return true;
}


void CDnSkill::_OnInitialize( void )
{
	// If state effect #129 ChangeActionSet and DnApplySEWhenActionSetBlowEnabledProcessor trigger type exist, apply all state effects except ChangeActionSet
	// Move into the manifestation-type object and delete. Later, the manifestation type controls so the state effect is valid only in the changed action.
	// Should we just link the pointer first..
	for( int k = PVE; k < NUM_SKILLLEVEL_APPLY_TYPE; ++k )
	{
		CDnApplySEWhenActionSetBlowEnabledProcessor* pApplySEWhenActionSetBlowEnableProcessor = 
			static_cast<CDnApplySEWhenActionSetBlowEnabledProcessor*>(GetProcessor( IDnSkillProcessor::APPLY_SE_WHEN_ACTIONSET_ENABLED, k ));
		if( pApplySEWhenActionSetBlowEnableProcessor )
		{
			// Error if this trigger type exists without ChangeActionSet state effect and ChangeActionStr trigger type..
			bool bValid = false;
			for( int i = 0; i < (int)m_vlStateEffectList[ k ].size(); ++i )
			{
				StateEffectStruct& StateEffect = m_vlStateEffectList[ k ].at( i );
				if( STATE_BLOW::BLOW_129 == StateEffect.nID )
				{
					bValid = true;
					break;
				}
			}

			if( bValid )
			{
				CDnChangeActionStrProcessor* pChangeActionStrProcessor = static_cast<CDnChangeActionStrProcessor*>(GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR, k ));
				if( pChangeActionStrProcessor )
					bValid = true;
				else
					bValid = false;
			}

			if( bValid )
			{
				for( int i = 0; i < (int)m_vlStateEffectList[ k ].size(); ++i )
				{
					StateEffectStruct& StateEffect = m_vlStateEffectList[ k ].at( i );
					if( STATE_BLOW::BLOW_129 != StateEffect.nID )
					{
						pApplySEWhenActionSetBlowEnableProcessor->AddStateEffect( &StateEffect );
						StateEffect.bApplyInProcessor = true;
					}
				}
			}
		}
	}
}


bool CDnSkill::Initialize( int iSkillTableID, int iLevel )
{
	m_iSkillID = iSkillTableID;

	// Load data for each mode. Read PVP first.
	// Since PVE is the default value, values not separated by mode are filled with the default value.

	// For monsters, since there is no pvp data, nothing happens inside the function and it returns.
	// If it is a player and nothing is there, something is wrong.
	if( false == _LoadSkillLevelData( iSkillTableID, iLevel, PVP ) )
		return false;

	if( false == _LoadSkillLevelData( iSkillTableID, iLevel, PVE ) )
		return false;

	// Info to collect separately after initialization.
	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );

	RefreshDecreaseMP();

	int iNumProcessor = (int)m_vlpProcessors[ PVE ].size();
	for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
	{
		IDnSkillProcessor* pProcessor = m_vlpProcessors[ PVE ].at( iProcessor );

		// If an action is in use, populate data for external lookup
		if( pProcessor->GetType() == IDnSkillProcessor::PLAY_ANI )
			m_setUseActionNames.insert( static_cast<CDnPlayAniProcess*>(pProcessor)->GetActionName() );
		else
		if( pProcessor->GetType() == IDnSkillProcessor::PARTIAL_PLAY_ANI )
		{
			CDnPartialPlayProcessor* pPartialPlayAni = static_cast<CDnPartialPlayProcessor*>(pProcessor);
			m_setUseActionNames.insert( pPartialPlayAni->GetStartActionName() );
			m_setUseActionNames.insert( pPartialPlayAni->GetLoopActionName() );
			m_setUseActionNames.insert( pPartialPlayAni->GetEndActionName() );
		}
	}

	// When initialized, PvE mode is the default.
	m_iSelectedSkillLevelDataApplyType = PVE;

	_OnInitialize();

	if (m_eDurationType == SummonOnOff)
		OnInitializeSummonMonsterInfo();

	return true;
}


bool CDnSkill::AddUsableCheckers( IDnSkillUsableChecker* pUsableChecker, int iSelectedLevelData )
{
	bool bResult = false;

	if( pUsableChecker )
	{
		m_vlpUsableCheckers[ iSelectedLevelData ].push_back( pUsableChecker );
		bResult = true;
	}

	return bResult;
}


bool CDnSkill::AddProcessor( IDnSkillProcessor* pProcessor, int iSelectedLevelData )
{
	bool bResult = false;

	if( pProcessor )
	{
		m_vlpProcessors[ iSelectedLevelData ].push_back( pProcessor );
		bResult = true;
	}

	return bResult;
}


bool CDnSkill::IsSatisfyWeapon( void )
{
	bool bSatisfy = true;

	if( (CDnWeapon::EquipTypeEnum_Amount != m_aeNeedEquipType[ 0 ]) ||
		(CDnWeapon::EquipTypeEnum_Amount != m_aeNeedEquipType[ 1 ]) )
	{
		bSatisfy = false;

		if( CDnWeapon::EquipTypeEnum_Amount != m_aeNeedEquipType[ 0 ] && CDnWeapon::EquipTypeEnum_Amount != m_aeNeedEquipType[ 1 ] )
		{
			if( CDnWeapon::IsSubWeapon( m_aeNeedEquipType[ 0 ] ) == CDnWeapon::IsSubWeapon( m_aeNeedEquipType[ 1 ] ) )	// 1. When both are main weapons or both are sub-weapons, or
			{
				int nWeapon = CDnWeapon::IsSubWeapon( m_aeNeedEquipType[ 0 ] ) ? 1 : 0;
#ifdef _GAMESERVER
				if( m_hActor->GetWeapon( nWeapon ) && m_aeNeedEquipType[ 0 ] == m_hActor->GetWeapon( nWeapon )->GetEquipType()
					|| m_hActor->GetWeapon( nWeapon ) && m_aeNeedEquipType[ 1 ] == m_hActor->GetWeapon( nWeapon )->GetEquipType() )
#else // _GAMESERVER
				if( m_hActor->GetWeapon( nWeapon, false ) && m_aeNeedEquipType[ 0 ] == m_hActor->GetWeapon( nWeapon, false )->GetEquipType()
					|| m_hActor->GetWeapon( nWeapon, false ) && m_aeNeedEquipType[ 1 ] == m_hActor->GetWeapon( nWeapon, false )->GetEquipType() )
#endif // _GAMESERVER
					bSatisfy = true;
			}
			else	// 2. If main weapon or sub weapon and
			{
				if( CDnWeapon::IsSubWeapon( m_aeNeedEquipType[ 0 ] ) )
				{
#ifdef _GAMESERVER
					if( m_hActor->GetWeapon( 1 ) && m_aeNeedEquipType[ 0 ] == m_hActor->GetWeapon( 1 )->GetEquipType()
						&& m_hActor->GetWeapon( 0 ) && m_aeNeedEquipType[ 1 ] == m_hActor->GetWeapon( 0 )->GetEquipType() )
#else // _GAMESERVER
					if( m_hActor->GetWeapon( 1, false ) && m_aeNeedEquipType[ 0 ] == m_hActor->GetWeapon( 1, false )->GetEquipType()
						&& m_hActor->GetWeapon( 0, false ) && m_aeNeedEquipType[ 1 ] == m_hActor->GetWeapon( 0, false )->GetEquipType() )
#endif // _GAMESERVER
						bSatisfy = true;
				}
				else
				{
#ifdef _GAMESERVER
					if( m_hActor->GetWeapon( 0 ) && m_aeNeedEquipType[ 0 ] == m_hActor->GetWeapon( 0 )->GetEquipType()
						&& m_hActor->GetWeapon( 1 ) && m_aeNeedEquipType[ 1 ] == m_hActor->GetWeapon( 1 )->GetEquipType() )
#else // _GAMESERVER
					if( m_hActor->GetWeapon( 0, false ) && m_aeNeedEquipType[ 0 ] == m_hActor->GetWeapon( 0, false )->GetEquipType()
						&& m_hActor->GetWeapon( 1, false ) && m_aeNeedEquipType[ 1 ] == m_hActor->GetWeapon( 1, false )->GetEquipType() )
#endif // _GAMESERVER
						bSatisfy = true;
				}
			}
		}
		else
		{
			for( int i=0; i<MAX_SKILL_NEED_EQUIP_COUNT; ++i )
			{
				if( CDnWeapon::EquipTypeEnum_Amount != m_aeNeedEquipType[ i ] )
				{
					for( int k=0; k<2; ++k )
					{
						// Calling bActionMatchWeapon with default true seems to return different weapons depending on action, so always check based on the currently equipped main weapon.
#ifdef _GAMESERVER
						if( m_hActor->GetWeapon( k ) && m_aeNeedEquipType[ i ] == m_hActor->GetWeapon( k )->GetEquipType() )
#else // _GAMESERVER
						if( m_hActor->GetWeapon( k, false ) && m_aeNeedEquipType[ i ] == m_hActor->GetWeapon( k, false )->GetEquipType() )
#endif // _GAMESERVER
						{
							bSatisfy = true;
							break;
						}
					}
					if( bSatisfy )
						break;
				}
			}
		}
	}

	return bSatisfy;
}

CDnSkill::UsingResult CDnSkill::CanExecute( void )
{
	UsingResult eResult = UsingResult::Failed;

	if( !m_hActor )
		return UsingResult::Failed;

	if( m_hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		if( pPlayerActor && pPlayerActor->IsSwapSingleSkin() )
		{
			if( pPlayerActor->IsTransformSkill(GetClassID()) == false )
				return UsingResult::Failed;
		}
	}
	if (m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_176))
	{
		DNVector(DnBlowHandle) vlBlows;
		CDnAllowedSkillsBlow* pAllowedSkillBlow = NULL;
		m_hActor->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_176, vlBlows);
		for (int i = 0; i < (int)vlBlows.size(); ++i)
		{
			pAllowedSkillBlow = static_cast<CDnAllowedSkillsBlow*>(vlBlows[i].GetPointer());
			if (!pAllowedSkillBlow) continue;

			if (!pAllowedSkillBlow->IsAllowSkill(m_iSkillID))
				return UsingResult::Failed;
		}
	}

	// Check remaining SP, HP, level, etc.
	// Aura or toggle skills can be turned off regardless of MP if currently active.
	if( !IsToggleOn() && !IsAuraOn() )
		if( m_hActor->GetSP() < m_iNeedMP[ m_iSelectedSkillLevelDataApplyType ] )
			return UsingResult::Failed;

	// Skills like Warrior's Relieve must be usable even while asleep or stunned.
	// Considered adding an ignore-unattackable trigger type to the table, but it is currently only meaningful for Relieve, so...
	// Observe how it is used elsewhere before standardizing.
	// First, skip incapacitation check for skills with Dissolve state effect.
	// IgnorectCantAction must be enabled on the State signal of the skill's action for the skill action to trigger.
	bool bPassCheckCantAction = false;
	for( DWORD i = 0; i < GetStateEffectCount(); ++i )
	{
		//#40480 To enable quickslots while frozen.
		CDnSkill::StateEffectStruct* pSE = GetStateEffectFromIndex( i );

		if( STATE_BLOW::BLOW_069 == pSE->nID )
		{
			// 76643 (Requested to simply disable if Dissolvable is 2)
			for( int i=0 ; i<m_hActor->GetNumAppliedStateBlow() ; ++i )
			{
				DnBlowHandle hBlow = m_hActor->GetAppliedStateBlow( i );
				if( hBlow )
				{
					const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
					if( pSkillInfo && pSkillInfo->iDissolvable == 2 )
						return UsingResult::Failed;
				}
			}

			bPassCheckCantAction = true;
			break;
		}

		if( STATE_BLOW::BLOW_155 == pSE->nID )
		{
			bPassCheckCantAction = true;
			break;
		}
	}

	// Skills that consume MP cannot be used if there is a skill-disable state effect.
	if( 0 < m_hActor->GetCantUseSkillSEReferenceCount() )
		if( 0 < m_iNeedMP[ m_iSelectedSkillLevelDataApplyType ] )
			return UsingResult::Failed;

	// Active skills cannot be used while state effect 67 (skill restriction) is active. 
	if( m_eSkillType == CDnSkill::Active )
	{
		int iAppliedStateBlow = m_hActor->GetNumAppliedStateBlow();
		for( int iBlow = 0; iBlow < iAppliedStateBlow; ++iBlow )
		{
			DnBlowHandle hBlow = m_hActor->GetAppliedStateBlow( iBlow );
			if( hBlow && hBlow->GetBlowIndex() == STATE_BLOW::BLOW_067 )
				return UsingResult::Failed;
		}
	}

	// HP 
	if( m_hActor->GetHP() < m_iNeedHP[ m_iSelectedSkillLevelDataApplyType ] )
		return UsingResult::Failed;

	bool isCharacterLevelCheck = true;
	//Skip character level check for skills leveled up by a skill level-up item.
	isCharacterLevelCheck = (GetLevelUpValue() == 0);
	
	if (isCharacterLevelCheck)
	{
		if( m_hActor->GetLevel() < m_iLevelLimit )
			return UsingResult::Failed;
	}

	// If a required weapon exists, check if equipped. Max 2. Skill can be used if either one is met.
	if( (CDnWeapon::EquipTypeEnum_Amount != m_aeNeedEquipType[ 0 ]) ||
		(CDnWeapon::EquipTypeEnum_Amount != m_aeNeedEquipType[ 1 ]) )
	{
		bool bSatisfy = false;
		for( int i = 0; i < MAX_SKILL_NEED_EQUIP_COUNT; ++i )
		{
			if( CDnWeapon::EquipTypeEnum_Amount != m_aeNeedEquipType[ i ] )
			{
				for( int k = 0; k < 2; ++k )
				{
					// #11120. Cannot retrieve equipped weapon during kick state, so explicitly get the equipped weapon via the original function.
					if( m_hActor->CDnActor::GetWeapon( k ) && m_aeNeedEquipType[ i ] == m_hActor->CDnActor::GetWeapon( k )->GetEquipType() )
					{
						bSatisfy = true;
						break;
					}
				}
				if( bSatisfy )
					break;
			}
		}

		if( false == bSatisfy )
			return UsingResult::Failed;
	}

	// Check count of consumable items like arrows
	bool bCheckNeedItem = true;
	if( GetRoom() && bIsExtremitySkill() && static_cast<CDNGameRoom*>(GetRoom())->bIsLadderRoom() )	// Ultimate skill in ladder
		bCheckNeedItem = false;

	if( bCheckNeedItem && m_iNeedItemID[ m_iSelectedSkillLevelDataApplyType ] > 0 )
	{
		int iNumNeedItem = CDnItemTask::GetInstance( m_hActor->GetRoom() ).ScanItemFromID( m_hActor, m_iNeedItemID[ m_iSelectedSkillLevelDataApplyType ], NULL );
		if( iNumNeedItem < m_iNeedItemDecreaseCount[ m_iSelectedSkillLevelDataApplyType ] )
			return UsingResult::Failed;
	}
	// Check job
	//if( m_hActor->GetClassID() <= CDnActor::Reserved6  )
	if( m_hActor->IsPlayerActor() )
	{
		//CDnPlayerActor* pActor = dynamic_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		CDnPlayerActor* pActor = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		if( NULL == pActor )
			return UsingResult::Failed;
		if( 0 != m_iNeedJobClassID )
		{
			if( pActor->IsPassJob( m_iNeedJobClassID ) == false ) 
				return UsingResult::Failed;
		}

		// Cannot activate skill unless in battle mode!
		if( !pActor->IsBattleMode() )
			return UsingResult::Failed;
	}

	switch( m_eDurationType )
	{
	    case CDnSkill::Instantly:
		case CDnSkill::Buff:
		case CDnSkill::Debuff:
		case CDnSkill::SummonOnOff:
		case CDnSkill::StanceChange:
			{
				// Cannot cast skill if cooldown has not ended.. Client checks and sends first, so the server allows a small grace period.
				// Check strictly if it is a monster or an auto-passive skill.
				if( m_bUseCoolTimeCharging )
				{
					if( m_nCurCoolTimeChargeCount <= 0 )
						return UsingResult::FailedByCooltime;
				}
				else if( m_hActor->IsMonsterActor() ||
					SkillTypeEnum::AutoPassive == m_eSkillType )
				{
					if( m_fCoolTime > 0.0f )
						return UsingResult::FailedByCooltime;
				}
				else
				{
					if( m_fLeftDelayTime > 0.5f )
						return UsingResult::FailedByCooltime;
				}
			}
			break;

		case CDnSkill::TimeToggle: 
			break;

		case CDnSkill::ActiveToggle:
		case CDnSkill::ActiveToggleForSummon:
			break;

		case CDnSkill::Aura:
			break;
	}

	if( m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].empty() )
		eResult = UsingResult::Success;
	else
	{
		int iNumChecker = (int)m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].size();

		for( int iChecker = 0; iChecker < iNumChecker; ++iChecker )
		{
			IDnSkillUsableChecker* pChecker = m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].at( iChecker );
			if( false == pChecker->CanUse() )
			{
				eResult = UsingResult::FailedByUsableChecker;
				break;
			}
			else
				eResult = UsingResult::Success;
		}
	}

	return eResult;
}

float CDnSkill::GetDelayTime( void ) 
{ 

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	float fDelayTime = 0.0f;

	if( 0.0f < m_fAnotherGlobalSkillCoolTime )
	{
		fDelayTime = m_fAnotherGlobalSkillCoolTime;

		DnSkillHandle hAnotherSkill;
		if (m_hActor)
			hAnotherSkill = m_hActor->FindSkill(m_nAnotherGlobakSkillID);

		if (hAnotherSkill)
		{
			if (hAnotherSkill->GetAnotherGlobalSkillID() != GetClassID())
				fDelayTime = hAnotherSkill->GetDelayTime();
		}
	}
	else
	//Do not apply if m_fAnotherGlobalSkillCoolTime above is already applied..
	{
		fDelayTime = m_fDelayTime[ m_iSelectedSkillLevelDataApplyType ]; 

		float fGlobalCoolTimeRate = 0.0f;
		if (m_hActor &&
			m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_269) && 
			m_iGlobalSkillGroupID > 0)
		{
			DNVector(DnBlowHandle) vlBlows;
			m_hActor->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_269, vlBlows);
			{
				int nCount = (int)vlBlows.size();
				for (int i = 0; i < nCount; ++i)
				{
					DnBlowHandle hBlow = vlBlows[i];
					if (hBlow && hBlow->IsEnd() == false)
					{
						fGlobalCoolTimeRate += hBlow->GetFloatValue();
					}
				}
			}
		}

		fDelayTime -= fDelayTime * fGlobalCoolTimeRate;
	}	

	return fDelayTime;
#else
	if( 0.0f < m_fAnotherGlobalSkillCoolTime )
		return m_fAnotherGlobalSkillCoolTime;

	return  m_fDelayTime[ m_iSelectedSkillLevelDataApplyType ]; 
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
}


void CDnSkill::OnBeginCoolTime()
{
	m_dwLastUseSkillTimeStamp = timeGetTime();

	if( m_bUseCoolTimeCharging && m_nMaxCoolTimeChargeCount > 1 )
	{
		if( m_nCurCoolTimeChargeCount > 0 )
			--m_nCurCoolTimeChargeCount;
		if( m_fLeftDelayTime > 0.0f )
			return;
	}

	m_fCoolTime = (GetDelayTime() == 0.0f) ? 0.0f : 1.0f;

	m_fCoolTimeAdjustBlowValue = 1.0f;

	m_fLeftDelayTime = GetDelayTime()*m_fCoolTimeAdjustBlowValue;

	m_fLeftDelayTime *= m_fCoolTimeMultipier;

	// [2011/03/09 semozz]
	// When LeftDelayTime becomes 0.0 at start, m_fCoolTime must be changed to 0.0f.
	// Otherwise cooldown stays at 1.0 (in Process function, if m_fLeftDelayTime is 0, CoolTime is not updated.
	if (m_fLeftDelayTime == 0.0f)
	{
		m_fCoolTime = 0.0f;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
		m_fDeltaGlobalCoolTime = 0.0f;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
	}
}

void CDnSkill::_OnBeginProcessException( void )
{
	// Release frame freeze immediately if Warrior uses Relieve skill while frozen. #12438
	// When a skill that applies a debuff-removal state effect to self is used while frozen 
	// Generalized to the used case.
	if( m_hActor && 
		(
		m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_041 ) ||
		m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_146 ) ||
		m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_218 ) //#53900 Added Escape skill
		))
	{
		int iNumStateBlow = (int)m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].size();
		for( int i = 0; i < iNumStateBlow; ++i )
		{
			const StateEffectStruct& SE = m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].at( i );
			if( STATE_BLOW::BLOW_069 == SE.nID &&
				StateEffectApplyType::ApplySelf == SE.ApplyType )
			{
				// Release all frame-freeze state effect reference indices. Frames must advance for actions to run
				// The Relieve state effect is applied only when the action runs.
				while( 0 < m_hActor->GetFrameStopRefCount() )
					m_hActor->RemovedFrameStop();
				break;
			}
		}
	}

	//Remove brand state effect
	ClearStigmaStateEffect();
}

void CDnSkill::ClearStigmaStateEffect()
{
	//////////////////////////////////////////////////////////////////////////
	//Check if skill has brand state effect...
	bool hasStigmaState = false;
	int nListCount = (int)m_vlStateEffectList[m_iSelectedSkillLevelDataApplyType].size();
	for (int i = 0; i < nListCount; ++i)
	{
		const StateEffectStruct& SE = m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].at( i );
		if (SE.nID == STATE_BLOW::BLOW_246)
		{
			hasStigmaState = true;
			break;
		}
	}

	//Find and remove nearby stigma effects...
	if (hasStigmaState == true)
	{
		DNVector(DnActorHandle) hVecList;
		m_hActor->ScanActor(m_hActor->GetRoom(), *m_hActor->GetPosition(), FLT_MAX, hVecList);

		int nActorCount = (int)hVecList.size();
		for (int i = 0; i < nActorCount; ++i)
		{
			DnActorHandle hActor = hVecList[i];
			if (hActor && hActor->IsDie() == false && hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_246))
			{
				DNVector(DnBlowHandle) vlhBlows;
				hActor->GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_246, vlhBlows );
				int iNumBlow = (int)vlhBlows.size();
				for( int j = 0; j < iNumBlow; ++j )
				{
					DnBlowHandle hBlow = vlhBlows[j];
					if (hBlow && hBlow->IsEnd() == false)
					{
						CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
						if (pSkillInfo && pSkillInfo->hSkillUser == m_hActor)
						{
							int nBlowID = hBlow->GetBlowID();
							hActor->SendRemoveStateEffectFromID(nBlowID);	//Send remove packet and
							hActor->GetStateBlow()->RemoveImediatlyStateEffectFromID(nBlowID);	//Remove immediately.
						}
					}
				}
			}

		}

	}
	//////////////////////////////////////////////////////////////////////////
}

// Process processors attached to the skill that require pre-processing first.
void CDnSkill::CheckProcessorOnBegin( void )
{
	CheckChangeActionStrByBubbleProcessor();
	CheckStateEffectApplyOnOffByBubbleProcessor();
}

void CDnSkill::CheckChangeActionStrByBubbleProcessor( void )
{
	CDnChangeActionStrByBubbleProcessor* pProcessor = static_cast<CDnChangeActionStrByBubbleProcessor*>(GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR_BY_BUBBLE ));
	if( pProcessor )
	{
		bool bChanged = false;
		const char* pChangedActionName = pProcessor->GetChangeActionNameAndRemoveNeedBubble( &bChanged );
		if( bChanged )
		{
			// Change the action.
			CDnPlayAniProcess* pPlayAniProcessor = static_cast<CDnPlayAniProcess*>(GetProcessor( IDnSkillProcessor::PLAY_ANI ));
			if (pPlayAniProcessor)
				pPlayAniProcessor->ChangeActionNameOnce( pChangedActionName );
		}
	}
}


void CDnSkill::CheckStateEffectApplyOnOffByBubbleProcessor( void )
{
	// Processor that changes the number of state effects based on bubble count
	CDnStateEffectApplyOnOffByBubbleProcessor* pProcessor = static_cast<CDnStateEffectApplyOnOffByBubbleProcessor*>(GetProcessor(IDnSkillProcessor::STATE_EFFECT_APPLY_ONOFF_BY_BUBBLE));
	if( pProcessor )
	{
		pProcessor->SelectAvailableSE( m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ] );
	}
}


void CDnSkill::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	ApplyAddtionalStateInfo();

	// Send skill use message to registered observers.
	boost::shared_ptr<IDnObserverNotifyEvent> pNotifyEvent( new CDnUseSkillMessage );
	boost::shared_ptr<CDnUseSkillMessage> pSkillUseEvent = boost::shared_polymorphic_downcast<CDnUseSkillMessage>( pNotifyEvent );
	pSkillUseEvent->SetSkillID( m_iSkillID );
	CDnObservable::Notify( pNotifyEvent );

	CheckProcessorOnBegin();

	// Remove the cooldown value set by another global skill sharing the same global ID when this skill is actually used.
	m_fAnotherGlobalSkillCoolTime = 0.0f;

	m_nAnotherGlobakSkillID = 0;

	m_SkillStartTime = LocalTime;
	//m_LastAuraCheckTime = 0;

#if defined( PRE_ADD_ACADEMIC ) // Summon-type monster active toggle skills do not need cooldown.
	if(m_eDurationType != DurationTypeEnum::ActiveToggleForSummon)
		//OnBeginCoolTime();

	{
		//--------------------------------------------------
		//[debug_skill]  server
		//
		//

		if ( 6001 == GetClassID() || 6003 == GetClassID() || 6204 == GetClassID() || 6208 == GetClassID() )
		{
			//int nSkillID = GetClassID();
		} 
		else
		{
			OnBeginCoolTime();
		}
		//--------------------------------------------------
	}
#else
		//--------------------------------------------------
		//[debug_skill]  server
		//
	    //

	    if ( 6001 == GetClassID() || 6003 == GetClassID() || 6204 == GetClassID() || 6208 == GetClassID() )
		{
			int nSkillID = GetClassID();
		} 
		else
		{
			OnBeginCoolTime();
		}
		//--------------------------------------------------
#endif

	m_LastTimeToggleMPDecreaseTime = LocalTime;
	m_bFinished = false;

	// If set to consume item, process item consumption.
	bool bCheckNeedItem = true;
	if( GetRoom() && bIsExtremitySkill() && static_cast<CDNGameRoom*>(GetRoom())->bIsLadderRoom() )	// Ultimate skill in ladder
		bCheckNeedItem = false;

	if( bCheckNeedItem && m_iNeedItemID[ m_iSelectedSkillLevelDataApplyType ] > 0 )
	{
		if( m_hActor && m_hActor->IsPlayerActor() )
		{
			CDNUserItem* pUserItem = static_cast<CDnPlayerActor*>(m_hActor.GetPointer())->GetUserSession()->GetItem();
			//int iNeedItemSlotIndex = pUserItem->FindInventorySlot( m_iNeedItemID, m_iNeedItemDecreaseCount );
			if (!pUserItem->DeleteInventoryByItemID( m_iNeedItemID[ m_iSelectedSkillLevelDataApplyType ], m_iNeedItemDecreaseCount[ m_iSelectedSkillLevelDataApplyType ], DBDNWorldDef::UseItem::Use )) return;
		}
	}

	if( ActiveToggle != m_eDurationType && TimeToggle != m_eDurationType && ActiveToggleForSummon != m_eDurationType )
	{
		CheckAndAddSelfStateEffect();
	}

	int iNumProcessor = (int)m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].size();
	for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
	{
		IDnSkillProcessor* pProcessor = m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].at( iProcessor );
		pProcessor->OnBegin( LocalTime, fDelta, GetMySmartPtr() );
	}

	if( false == m_bItemSkill )
	{
		m_hActor->SetSkillSuperAmmor( int(m_hActor->GetSuperAmmor() * m_fStartSuperArmor) );
		m_hActor->SetHittable( m_bStartCanHit );
	}

	// Handle exception cases related to skill usage.
	_OnBeginProcessException();

	if( m_hActor->IsPlayerActor() && 0.0f != m_fResetCooltime )
	{
		int nRand = _rand(m_hActor->GetRoom())%100;

		if( nRand < (int)(m_fResetCooltime * 100) )
		{
			static_cast<CDnPlayerActor*>(m_hActor.GetPointer())->RequestCooltimeReset( m_iSkillID );
			m_fCoolTime = 0.0f; 
			m_fLeftDelayTime = 0.0f;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
			m_fDeltaGlobalCoolTime = 0.0f;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
		}
	}

#if defined(PRE_ADD_50903)
	//Reset when the skill starts...
	SetHitCountForVarianceDamage(0);
#endif // PRE_ADD_50903
}


// Handle when the skill is actually activated
void CDnSkill::ProcessExecute( LOCAL_TIME LocalTime, float fDelta )
{
	switch( m_eDurationType )
	{
		case DurationTypeEnum::TimeToggle:
			break;

		case DurationTypeEnum::Aura:
			break;
	}

	// Process attached processors
	int iNumProcessor = (int)m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].size();
	for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
	{
		IDnSkillProcessor* pProcessor = m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].at( iProcessor );
		pProcessor->Process( LocalTime, fDelta );
	}

	// -1 indicates a skill that stays active while holding a direction key, such as Dash.
	if( -1.0f != m_fPassiveActionSkillLength )
	{
		m_fPassiveActionSkillLength -= fDelta;
		if( m_fPassiveActionSkillLength < 0.0f )
		{
			m_fPassiveActionSkillLength = 0.0f;
		}
	}
}


void CDnSkill::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CheckAndRemoveInstantApplySelfStateEffect();

	//Remove state effect added by prefix
	RemovePrefixBlow();

	int iNumProcessor = (int)m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].size();
	for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
	{
		IDnSkillProcessor* pProcessor = m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].at( iProcessor );
		pProcessor->OnEnd( LocalTime, fDelta );
	}

	m_SkillInfo->bIgnoreImmune = m_isIgnoreImmuneBackup;

	CheckProcessorOnEnd();

	// If enhanced by a bubble, restore to original state.
	if( m_bEnchantedFromBubble )
	{
		ReleaseEnchantSkill();
		m_bEnchantedFromBubble = false;
	}

	RemoveAddtionalStateInfo();

#if defined(PRE_FIX_64312)
	if (m_isAppliedSummonMonsterEnchantSkill == true)
	{
		ReleaseEnchantSkill();
		m_isAppliedSummonMonsterEnchantSkill = false;
	}
#endif // PRE_FIX_64312

}


void CDnSkill::CheckProcessorOnEnd( void )
{
	// Processor that changes the number of state effects based on bubble count
	CDnStateEffectApplyOnOffByBubbleProcessor* pProcessor = static_cast<CDnStateEffectApplyOnOffByBubbleProcessor*>(GetProcessor(IDnSkillProcessor::STATE_EFFECT_APPLY_ONOFF_BY_BUBBLE));
	if( pProcessor )
	{
		pProcessor->RestoreSEList( m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ] );
	}
}


// Handling of cooldown and etc.
void CDnSkill::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( TIME_ACCELERATION_SKILL_ID != m_iSkillID && SPRIT_BOOST_SKILL_ID != m_iSkillID )
		fDelta *= m_hActor->GetCoolTimeDeltaAdjustValue();

	float fTempAdjustValue = m_fCoolTimeAdjustBlowValue;

	fTempAdjustValue *= m_fCoolTimeMultipier;

	switch( m_eDurationType )
	{
		case DurationTypeEnum::Instantly:
		case DurationTypeEnum::Buff:
		case DurationTypeEnum::Debuff:
		case DurationTypeEnum::SummonOnOff:
		case DurationTypeEnum::StanceChange:
			{
				if( 0.0f == m_fOnceElapsedDelayTime )
				{
					if( m_fLeftDelayTime == 0.0f ) return;

					m_fLeftDelayTime -= fDelta;
					if( m_fLeftDelayTime < 0.f ) 
					{
						m_fLeftDelayTime = 0.f;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
						m_fDeltaGlobalCoolTime = 0.0f;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
						if( m_bUseCoolTimeCharging && m_nCurCoolTimeChargeCount < m_nMaxCoolTimeChargeCount )
						{
							++m_nCurCoolTimeChargeCount;
							if( m_nCurCoolTimeChargeCount < m_nMaxCoolTimeChargeCount )
								m_fLeftDelayTime = GetDelayTime() * fTempAdjustValue;
						}
					}

					// [2011/03/09 semozz]
					// Case occurs where fTempAdjustValue becomes 0 due to Windwalker's Showtime skill..
					// Add exception handling
					float fTempDelayTime = (GetDelayTime()*fTempAdjustValue);
					if (fTempDelayTime == 0.0f)
						m_fCoolTime = 0.0f;
					else
						m_fCoolTime = ( 1.0f / fTempDelayTime ) * m_fLeftDelayTime;

				}
				else
				{
					if( m_fOnceElapsedDelayTime == 0.0f ) 
					{
						m_fOnceDelayTime = 0.0f;
						return;
					}

					m_fOnceElapsedDelayTime -= fDelta;
					if( m_fOnceElapsedDelayTime < 0.0f )
						m_fOnceElapsedDelayTime = 0.0f;

					m_fCoolTime = ( 1.0f / (m_fOnceDelayTime*m_fCoolTimeAdjustBlowValue) ) * m_fOnceElapsedDelayTime;

				}
			}
			break;

		case DurationTypeEnum::TimeToggle:

			break;

		case DurationTypeEnum::ActiveToggle:
		case DurationTypeEnum::ActiveToggleForSummon:

			break;


		case DurationTypeEnum::Aura:
		
			break;
	}

	// If the cooldown has ended, clear the set global skill cooldown.
	if( 0.0f < m_fAnotherGlobalSkillCoolTime && 0.0f == m_fCoolTime )
	{
		m_fAnotherGlobalSkillCoolTime = 0.0f;

		m_nAnotherGlobakSkillID = 0;
	}
}

bool CDnSkill::IsFinished( void )
{
	bool bResult = false;

	// If it is an aura skill, do not end it before map transition.
	if( Aura == m_eDurationType )
		return IsFinishedAuraSkill();
	
	if( m_bFinished )
		return m_bFinished;

	if( m_bChainingPassiveSkill )
		return false;

	// For skills that continuously consume MP, end the skill when MP is completely depleted.
	if( 0.0f < m_fPassiveActionSkillLength || -1.0f == m_fPassiveActionSkillLength )
	{
		float fDecreaseMPRatio = 0.f;
		DNVector( DnBlowHandle ) vlhBlows;
		m_hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_014, vlhBlows );

		for( DWORD i=0; i<vlhBlows.size(); i++ )
		{
			if( vlhBlows[i] && vlhBlows[i]->GetParentSkillInfo()->iSkillID == GetClassID() )
			{
				fDecreaseMPRatio = vlhBlows[i]->GetFloatValue();
				break;
			}
		}

		if( m_hActor->GetSP() < m_iNeedMP[ m_iSelectedSkillLevelDataApplyType ] && fDecreaseMPRatio < 0.f )
			return true;

		return false;
	}

#ifdef PRE_FIX_76603
	if( m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_232 ) )
	{
		DNVector(DnBlowHandle) vlBlows;
		m_hActor->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_232, vlBlows);

		for (DWORD i = 0; i < vlBlows.size(); i++)
		{
			if( vlBlows[i] )
			{
				CDnTransformBlow *pTransformBlow = static_cast<CDnTransformBlow*>( vlBlows[i].GetPointer() );
				if( pTransformBlow && pTransformBlow->GetParentSkillInfo()->iSkillID == GetClassID() )
				{
					if( pTransformBlow->CheckSkillDefendency() == true )
						return false;
				}
			}
		}
	}
#endif

	if( m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].empty() )
		bResult = true;
	else
	{
		int iNumProcessor = (int)m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].size();
		for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
		{
			IDnSkillProcessor* pProcessor = m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].at( iProcessor );

			if( true == pProcessor->IsFinished() )
				bResult = true;
			else
			{
				bResult = false;
				break;
			}
		}
	}

	// lazy evaluation
	m_bFinished = bResult;

	return bResult;
}

IDnSkillProcessor* CDnSkill::GetProcessor( int iType )
{
	IDnSkillProcessor* pResult = NULL;

	int iNumProcessor = (int)m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].size();
	for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
	{
		IDnSkillProcessor* pSkillProcessor = m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].at( iProcessor );
		if( iType == pSkillProcessor->GetType() )
		{
			pResult = pSkillProcessor;
			break;
		}
	}

	return pResult;
}

IDnSkillProcessor* CDnSkill::GetProcessor( int iType, int iLevelDataApplyType )
{
	IDnSkillProcessor* pResult = NULL;

	int iNumProcessor = (int)m_vlpProcessors[ iLevelDataApplyType ].size();
	for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
	{
		IDnSkillProcessor* pSkillProcessor = m_vlpProcessors[ iLevelDataApplyType ].at( iProcessor );
		if( iType == pSkillProcessor->GetType() )
		{
			pResult = pSkillProcessor;
			break;
		}
	}

	return pResult;
}


void CDnSkill::CheckTargetCount( int iTargetActorCount )
{
	// Will only be used in healing area for now..
	IDnSkillProcessor* pProcessor = GetProcessor( IDnSkillProcessor::DIVIDE_STATE_EFFECT_ARG );
	
	if( pProcessor )
	{
		// In the Breath of Goddess skill table, only target is set, so add self's state effect here.
		_ASSERT( "Currently only used by Breath of Goddess..." && m_iSkillID == 3004 );

		CDnDivideSEArgumentByTargets* pDivideSEProcessor = static_cast<CDnDivideSEArgumentByTargets*>(pProcessor);
		int iSEArgument = pDivideSEProcessor->GetStateEffectArgument();

		// Add one more including self
		int iDividedValue = iSEArgument / (iTargetActorCount+1);

		int iNumStateEffect = (int)m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].size();
		char acBuf[ 256 ];
		for( int iStateEffect = 0; iStateEffect < iNumStateEffect; ++iStateEffect )
		{
			ZeroMemory( acBuf, sizeof(acBuf) );
			StateEffectStruct& StateEffect = m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].at( iStateEffect );
			itoa( iDividedValue, acBuf, 10 );
			StateEffect.szValue.assign( acBuf );

			// Also add to self
			int iID = m_hActor->CmdAddStateEffect( /*m_hActor,*/ GetInfo(), (STATE_BLOW::emBLOW_INDEX)StateEffect.nID, 
												   StateEffect.nDurationTime, StateEffect.szValue.c_str() );
		}
	}

#if defined(PRE_ADD_50903)
	SetHitCountForVarianceDamage(iTargetActorCount);
#endif // PRE_ADD_50903
}

void CDnSkill::CheckAndAddSelfStateEffect( void )
{
	map<int, bool> mapDuplicateResult;		// State effects cast on self do not undergo separate probability stacking.
	CanApply eResult = CDnSkill::CanApplySkillStateEffect( m_hActor, GetMySmartPtr(), mapDuplicateResult );

	if( CanApply::Fail != eResult )
	{
		int nID(-1);
		StateEffectStruct *pStruct(NULL);

		// Note Hanki: 2009.12.21
		// Related to effect application duration used on self (#1911)
		// Find state effect application signals used on self and set the count.
		// Currently passive skills do not set playaniprocessor, so they are filtered out here. 
		// Cannot use state effect timing signal applied to self.
		// Therefore, if passive action skills also need state effect timing signals in the future
		// Additional work is needed.
		bool bUseApplySelfStateBlowSignal = false;
		CDnPlayAniProcess* pPlayAniProcessor = static_cast<CDnPlayAniProcess*>(GetProcessor( IDnSkillProcessor::PLAY_ANI ));
		if( pPlayAniProcessor )
		{
			const char* pActionName = pPlayAniProcessor->GetActionName();

			CEtActionBase::ActionElementStruct* pActionElement = m_hActor->GetElement( pActionName );
			_ASSERT( pActionElement );
			if( pActionElement )
			{
				int iApplyStateEffectSignalCount = 0;
				int iNumSignal = (int)pActionElement->pVecSignalList.size();
				for( int iSignal = 0; iSignal < iNumSignal; ++iSignal )
				{
					CEtActionSignal* pSignal = pActionElement->pVecSignalList.at( iSignal );
					if( STE_ApplyStateEffect == pSignal->GetSignalIndex() )
					{
						bUseApplySelfStateBlowSignal = true;
						++iApplyStateEffectSignalCount;
					}
				}
			}
		}

		for( DWORD i=0; i<m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].size(); i++ ) 
		{
			pStruct = &m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ][ i ];
			if( !pStruct ) 
				continue;
			
			// There are cases where the manifestation type directly controls adding state effects. (Sacred Hammering)
			if( pStruct->bApplyInProcessor )
				continue;

			if( pStruct->ApplyType == ApplySelf )
			{
				// A self-targeted state effect that ends when the action finishes.
				if( 0 == pStruct->nDurationTime || -1 == pStruct->nDurationTime  )
				{
					// Whether 0 or -1 in the table, standardize to -1 on the program side
					pStruct->nDurationTime = -1;

					// Hanki 2009.7.27 
					// Related to effect application duration used on self (#1911)
					// Since state effects added by skill enhancement crests were not in the original state effects, 
					// Ignore the self-applied state effect timing signal and add immediately.
					if( bUseApplySelfStateBlowSignal && (0 == pStruct->nGlyphID) )
					{
						StateEffectStruct InstantSelfStateEffect = *pStruct;
						InstantSelfStateEffect.nDurationTime = pStruct->nDurationTime;
						m_hActor->AddStateEffectQueue( *GetInfo(), InstantSelfStateEffect );

						SetAddStateEffectQueue(true);
					}
					else
					{
						// [2010/12/13 semozz]
						// Create directly and store in m_vlApplyNoPacketStateEffectList to process state effects added without existing packets,
						// Create as a packet and store in m_vlApplyNoPacketStateEffectList.

						// #32160 Apply the updated skill stacking routine to item skills as well.
						// In addition to items, skills without the ApplyStateEffect signal add state effects directly here, so remove any state effects that should be excluded.
						m_hActor->RemoveResetStateBlow();

						nID = m_hActor->CmdAddStateEffect( GetInfo(), (STATE_BLOW::emBLOW_INDEX)pStruct->nID, pStruct->nDurationTime, pStruct->szValue.c_str() );
						
						if( -1 == nID ) 
							continue;

						// Note Hanki: After taking a potion, if dying from damage when the state effect runs 1 frame and applies, on client 
						// Consumed item may be applied while server clears all state effects upon death so it doesn't apply.. HP sync could desync.
						if( m_bItemSkill )
						{
							DnBlowHandle hBlow = m_hActor->GetStateBlowFromID( nID );
							hBlow->OnBegin( 0, 0.0f );

#ifndef _GAMESERVER
							if( hBlow->IsUseTableDefinedGraphicEffect() )
								hBlow->AttachGraphicEffectDefaultType();
#endif
							m_hActor->OnBeginStateBlow( hBlow );

							hBlow->SetState( STATE_BLOW::STATE_DURATION );
						}

						m_vlApplySelfNoDurationStateEffectList.push_back( nID );
					}
				}
				else
				{
					// Self state effect with duration
					if( bUseApplySelfStateBlowSignal && (0 == pStruct->nGlyphID) )
					{
						m_hActor->AddStateEffectQueue( *GetInfo(), *pStruct );
						SetAddStateEffectQueue(true);
					}
					else
					{
						// #32160 Apply the updated skill stacking routine to item skills as well.
						// In addition to items, skills without the ApplyStateEffect signal add state effects directly here, so remove any state effects that should be excluded.
						m_hActor->RemoveResetStateBlow();

						nID = m_hActor->CmdAddStateEffect( /*m_hActor,*/ GetInfo(), (STATE_BLOW::emBLOW_INDEX)pStruct->nID, 
															   pStruct->nDurationTime, pStruct->szValue.c_str() );
						if( -1 == nID )
							continue;
					}
				}
			}
		}
	}
}

void CDnSkill::CheckAndRemoveInstantApplySelfStateEffect( void )
{
	for( DWORD i=0; i<m_vlApplySelfNoDurationStateEffectList.size(); i++ ) 
	{
		if( m_hActor )
			m_hActor->CmdRemoveStateEffectFromID( m_vlApplySelfNoDurationStateEffectList[i] );
	}

	if( m_hActor )
	{
		// #40768 Item skill uses MASkillUser::m_hItemSkill separately from MASkillUser::m_hProcessSkill, so
		// Since m_hProcessSkill can start even before m_hItemSkill ends, in that state 
		// if m_hItemSkill ends and calls this function, it would wipe out the BlowQueue registered in m_hProcessSkill.
		// Therefore, handle item skills separately.

		//#40760
		//When a suffix skill activates upon skill use, state effects registered from the existing skill
		//Queue may get cleared when calling the function below at the end of a suffix skill.
		//Set flag via SetAddStateEffectQueue(true) when calling AddStateEffectQueue
		//Reset the state effect queue only for skills with the flag set..

		if ( IsAddStateEffectQueue() )
			m_hActor->ClearSelfStateSignalBlowQueue( IsItemSkill() );
	}

	SAFE_DELETE_VEC( m_vlApplySelfNoDurationStateEffectList );
}


DWORD CDnSkill::GetStateEffectCount( void )
{
	return (DWORD)m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].size();
}


CDnSkill::StateEffectStruct* CDnSkill::GetStateEffectFromIndex( DWORD dwIndex )
{
	if( dwIndex >= m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].size() ) return NULL;
	return &m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ][ dwIndex ];
}

bool CDnSkill::IsUseCheckerType( int iCheckerType )
{
	bool bResult = false;

	int iNumChecker = (int)m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].size();
	for( int iChecker = 0; iChecker < iNumChecker; ++iChecker )
	{ 
		if( m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].at( iChecker )->GetType() == iCheckerType )
		{
			bResult = true;
			break;
		}
	}

	return bResult;
}


bool CDnSkill::IsUseProcessorType( int iProcessorType )
{
	bool bResult = false;

	int iNumProcessor = (int)m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].size();
	for( int iProcessor = 0; iProcessor < iNumProcessor; ++iProcessor )
	{
		if( m_vlpProcessors[ m_iSelectedSkillLevelDataApplyType ].at( iProcessor )->GetType() == iProcessorType )
		{
			bResult = true;
			break;
		}
	}

	return bResult;
}

CDnSkill::SkillTypeEnum CDnSkill::GetSkillType( int nSkillID )
{
	if (nSkillID <= 0) return SkillTypeEnum::Active;

	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	if( pSkillTable->IsExistItem( nSkillID ) == false ) return SkillTypeEnum::Active;
	return (SkillTypeEnum)pSkillTable->GetFieldFromLablePtr( nSkillID, "_SkillType" )->GetInteger();
}


bool CDnSkill::IsUseActionNames( const set<string>& setUseActionNames )
{
	list<string> listIntersect;
	set_intersection( setUseActionNames.begin(), setUseActionNames.end(), 
					  m_setUseActionNames.begin(), m_setUseActionNames.end(), 
					  back_inserter(listIntersect) );

	return !listIntersect.empty();
}

void CDnSkill::_RefreshDecreaseMP( int iSkillLevelDataApplyType )
{
	if( 0.0f == m_fMPConsumeType[ iSkillLevelDataApplyType ] )
	{
		if( !m_hActor || m_hActor->IsMonsterActor() ) {
			m_iNeedMP[ iSkillLevelDataApplyType ] = m_iDecreaseMP[ iSkillLevelDataApplyType ];
			return;
		}

		if( m_hActor->IsPlayerActor() )
		{
#ifndef _ADD_NEW_MPCONSUME
			float fRatio = 1.f;
			if( m_hActor->GetLevel() > 0 )
			{
				int nJobClassID = ((CDnPlayerActor*)m_hActor.GetPointer())->GetJobClassID();
				float fDecreaseRatio = CPlayerLevelTable::GetInstance().GetValueFloat( nJobClassID , m_hActor->GetLevel(), CPlayerLevelTable::SPDecreaseRatio );
				fRatio =  fDecreaseRatio * m_hActor->GetLevelWeightValue();
			}

			m_iNeedMP[ iSkillLevelDataApplyType ] = (int)( m_iDecreaseMP[ iSkillLevelDataApplyType ] * fRatio );
#else
			m_iNeedMP[iSkillLevelDataApplyType] = (int)(m_iDecreaseMP[iSkillLevelDataApplyType]);
#endif
		}
	}
	else
	{
		m_iNeedMP[ iSkillLevelDataApplyType ] = int((float)m_hActor->GetMaxSP() * m_fMPConsumeType[ iSkillLevelDataApplyType ]);
	}

	m_iOriginalNeedMP[ iSkillLevelDataApplyType ] = m_iNeedMP[ iSkillLevelDataApplyType ];
}

void CDnSkill::RefreshDecreaseMP( int iSkillLevelDataApplyType /*= NUM_SKILLLEVEL_APPLY_TYPE*/ )
{
	if( iSkillLevelDataApplyType == NUM_SKILLLEVEL_APPLY_TYPE )
	{
		for( int iApplyType = PVE; iApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iApplyType )
			_RefreshDecreaseMP( iApplyType );
	}
	else
	{
		_RefreshDecreaseMP( iSkillLevelDataApplyType );
	}
}


void CDnSkill::SetOnceCoolTime( float fDelayTime, float fElapsedDelayTime )
{
	_ASSERT( fElapsedDelayTime <= fDelayTime );

	m_fOnceDelayTime = fDelayTime;
	m_fOnceElapsedDelayTime = fElapsedDelayTime;
}


void CDnSkill::SetPassiveSkillActionName( const char* pActionName )
{
	if( pActionName )
	{
		// Not automatically added if a duplicate arrives.
		m_setUseActionNames.insert( string(pActionName) );
	}
}


// Related to issue #6190.
CDnSkill::CanApply CDnSkill::CanApplySkillStateEffect( const SkillInfo* pUsingSkillInfo, DnActorHandle hTargetActor, int iSkillID, int iLevel, 
													   int iSkillDuplicateMethod, int iDuplicateCount, map<int, bool>& mapDuplicateResult, bool isHitProcess )
{
	// Initialize reset list when checking state effects [2010/12/09 semozz]
	if (hTargetActor)
		hTargetActor->InitStateBlowIDToRemove();

	CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;
	bool bExistingSameSkill = false;
	int iExistingSameSkillCount = 0;

	// Store and remove state effects of the same lower-level skill when removed due to duplication.
	// Since the same state effect from another skill may be active, always remove using the ID assigned at creation.

	vector<int> vlStateBlowIDToRemove;
	DNVector( DnBlowHandle ) vlhSameSkillBlows;

	int iNumAppliedStateEffect = (int)hTargetActor->GetNumAppliedStateBlow();
	for( int iAppliedStateEffect = 0; iAppliedStateEffect < iNumAppliedStateEffect; ++iAppliedStateEffect )
	{
		DnBlowHandle hExistingBlow = hTargetActor->GetAppliedStateBlow( iAppliedStateEffect );

		const CDnSkill::SkillInfo* pExistingParentSkillInfo = NULL;
		if( hExistingBlow )
			pExistingParentSkillInfo = hExistingBlow->GetParentSkillInfo();

		// State effects without parent skill info were not applied by a skill, so they are not handled here
		if( NULL == pExistingParentSkillInfo )
			continue;

		// Check skill duplicate count
		// If the same state effect appears again for the same skill ID, it is a duplicate skill.
		bool bIsSameSkillID = false;
#if defined(PRE_FIX_58505)
		//If suffix skill type is not set, compare by skill ID,
		//If suffix skill type is set, compare whether suffix skill types match.
		if ( pUsingSkillInfo->bItemPrefixSkill == true && pUsingSkillInfo->nPrefixSkillType != CDnPrefixSkill::Prefix_Non )
			bIsSameSkillID = pUsingSkillInfo->nPrefixSkillType == pExistingParentSkillInfo->nPrefixSkillType;
		else
			bIsSameSkillID = iSkillID == pExistingParentSkillInfo->iSkillID;
#else
		if( iSkillID == pExistingParentSkillInfo->iSkillID )
			bIsSameSkillID = true;
#endif // PRE_FIX_58505

		if (bIsSameSkillID == true)
		{
			// If the same skill ID appears for the first time, increment the active skill count by 1.

			// #27335 - Ankle Shot case [2011/01/11 semozz]
			// When target shoots Ankle Shot at me and CantMove state effect is applied
			// When I fire the same Ankle Shot, the CantMove state effect on myself has the same skill ID
			// Caught by duplicate skill handling..
			// Check the ApplyType of the current state effect
			// Compare the Actor that applied the current state effect with TargetActor
			// If not used by a valid Actor, mark as not the same skill
			
			DnActorHandle hSkillUserActor = pUsingSkillInfo->hSkillUser;
			DnSkillHandle hSkill;
			if (hSkillUserActor)
				hSkill = hSkillUserActor->FindSkill( iSkillID );
			
			//State effects caused by Props may not have skill info.
			if (hSkill)
			{
				// Target type of one of the currently active state effects from the skill about to be used.
				StateEffectApplyType eExistingSEApplyType = ApplySelf;

				bExistingSameSkill = true;
				for( int i = 0; i < (int)hSkill->GetStateEffectCount(); ++i )
				{
					const StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( i );
					if( pSE->nID == hExistingBlow->GetBlowIndex()
						&& pSE->ApplyType == pExistingParentSkillInfo->eApplyType )
					{
						eExistingSEApplyType = pSE->ApplyType;
						break;
					}
				}

				//Depending on the applyType of the currently applied state effect
				switch(eExistingSEApplyType)
				{
				case StateEffectApplyType::ApplySelf:
					if (isHitProcess == false)
						bExistingSameSkill = (hTargetActor == hSkillUserActor);
					else
						bExistingSameSkill = false;
					break;
				case StateEffectApplyType::ApplyEnemy:
					bExistingSameSkill = (hTargetActor->GetTeam() != hSkillUserActor->GetTeam());
					break;
				case StateEffectApplyType::ApplyTarget:
					bExistingSameSkill = (hTargetActor != hSkillUserActor);
					break;
				case StateEffectApplyType::ApplyFriend:
					bExistingSameSkill = (hTargetActor->GetTeam() == hSkillUserActor->GetTeam());
					break;
				case StateEffectApplyType::ApplyAll:
					bExistingSameSkill = true;
					break;
				}
			}
			else
				bExistingSameSkill = true;
			
			if( bExistingSameSkill )
			{
				iExistingSameSkillCount += hExistingBlow->GetDuplicateCount();
				vlhSameSkillBlows.push_back( hExistingBlow );
			}
		}

		if (pUsingSkillInfo->eTargetType == All  //TargetType of the currently used skill is All, and 
			&& (0 != iSkillDuplicateMethod && (iSkillDuplicateMethod == pExistingParentSkillInfo->iSkillDuplicateMethod)) //When overlap process ID is set and it matches the existing skill ID,
			&& (hTargetActor == pUsingSkillInfo->hSkillUser) //Self
			)
			continue;

		// If the persistent effect category index is not 0, duplicate skill effect handling is required.
		// If the persistent effect category index is 0, it simply stacks.
		// Or if it's the same skill and max stack count is 1 or less, replace the skill.
#if defined(PRE_FIX_58505)
		if( 0 != iSkillDuplicateMethod ||
			(bIsSameSkillID == true && iDuplicateCount <= 1) )
#else
		if( 0 != iSkillDuplicateMethod ||
			(iSkillID == pExistingParentSkillInfo->iSkillID && iDuplicateCount <= 1) )
#endif // PRE_FIX_58505
		{
			// Check if the state effect index matches the currently running skill; if so, check whether stacking is allowed.
			// Also stack only when the skill caster's team matches.
			// When an enemy team uses the same skill, my debuff state effect gets removed. (#19812)
			if( (iSkillDuplicateMethod == pExistingParentSkillInfo->iSkillDuplicateMethod) &&
				(pUsingSkillInfo->iSkillUserTeam == pExistingParentSkillInfo->iSkillUserTeam) )
			{
				if( iLevel >= pExistingParentSkillInfo->iLevel )
				{
					// State effect 242 must not be reset once added.

					// If it passes the probability check, add to the state effects to reset.
					// The object that actually checks probability uses the already-applied state effect object, since it only checks probability
					// Use as is.
					if( hExistingBlow->CanBegin() )
					{
						bool isSelfStateEffect = false;

						//#60966 Case where both self-applied and other-applied state effects exist together
						//Does not matter at skill start, but self state effects are not re-applied during Hit processing.
						//So if self-applied state effects are added to the removal list here during hit processing
						//No way to add again.
						//When entering during hit processing, skip self-applied state effects.
						if (isHitProcess == true)
						{
							StateEffectApplyType eExistingSEApplyType = ApplySelf;

							bool bCheck = false;

							DnActorHandle hExistingSkillUserActor = pExistingParentSkillInfo->hSkillUser;
							DnSkillHandle hSkill;
							if (hExistingSkillUserActor)
								hSkill = hExistingSkillUserActor->FindSkill( pExistingParentSkillInfo->iSkillID );

							if (hSkill)
							{
								for( int i = 0; i < (int)hSkill->GetStateEffectCount(); ++i )
								{
									const StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( i );
									if( pSE->nID == hExistingBlow->GetBlowIndex() 
										&& pSE->ApplyType == pExistingParentSkillInfo->eApplyType )
									{
										bCheck = true;
										eExistingSEApplyType = pSE->ApplyType;
										break;
									}
								}

								//Allow state effects cast on self to be applied?..
								if ( StateEffectApplyType::ApplySelf == eExistingSEApplyType && bCheck )
								{
									isSelfStateEffect = true;
								}
							}

						}

						if (isSelfStateEffect == false &&
							hExistingBlow->GetBlowIndex() != STATE_BLOW::BLOW_242 )
						{
							vlStateBlowIDToRemove.push_back( hExistingBlow->GetBlowID() );
							mapDuplicateResult[ hExistingBlow->GetBlowIndex() ] = true;

							// Store states to be reset. [2010/12/08 semozz]
							// When applying states in this list at OnSignal's STE_ApplyStateEffect time
							// Remove existing state.
							hTargetActor->AddStateBlowIDToRemove( hExistingBlow->GetBlowID() );
						}
					}
					else
					{
						eResult = CDnSkill::CanApply::Fail;		// Do not add because the probability check failed during effect refresh.
						mapDuplicateResult[ hExistingBlow->GetBlowIndex() ] = false;
					}
				}
				else
				if( iLevel < pExistingParentSkillInfo->iLevel )
				{
					// Effect not applied.
					eResult = CDnSkill::CanApply::Fail;

					// Note Hanki: If something needs to be displayed graphically later, handle it here.

					// [2011/01/20 semozz]
					// Issue when using Heaven's Judgment before Chain Lightning cast finishes.
					// When skill A has a higher level and skill B used afterwards has a lower level, if DuplicationMethod is the same
					// Since the skill is not added here, Self state effects are not applied. Target should not matter.

					// Select the actor that used the current state effect
					DnActorHandle hExistingSkillUserActor = pExistingParentSkillInfo->hSkillUser;
					DnSkillHandle hSkill;
					if (hExistingSkillUserActor)
						hSkill = hExistingSkillUserActor->FindSkill( pExistingParentSkillInfo->iSkillID );

					if (hSkill)
					{
						// Target type of one of the currently active state effects from the skill about to be used.
						StateEffectApplyType eExistingSEApplyType = ApplySelf;

						for( int i = 0; i < (int)hSkill->GetStateEffectCount(); ++i )
						{
							const StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( i );
							if( pSE->nID == hExistingBlow->GetBlowIndex() 
								&& pSE->ApplyType == pExistingParentSkillInfo->eApplyType )
							{
								eExistingSEApplyType = pSE->ApplyType;
								break;
							}
						}

						//Allow state effects cast on self to be applied?..
						if (StateEffectApplyType::ApplySelf == eExistingSEApplyType)
						{
							eResult = CDnSkill::CanApply::Apply;

							vlStateBlowIDToRemove.push_back( hExistingBlow->GetBlowID() );
							mapDuplicateResult[ hExistingBlow->GetBlowIndex() ] = true;

							hTargetActor->AddStateBlowIDToRemove( hExistingBlow->GetBlowID() );
						}
					}
				}
			}
		}
	}

	if( false == vlStateBlowIDToRemove.empty() )
	{
		// Replace skill effect. No need to handle stack count tracking.
		bExistingSameSkill = false;
		eResult = CDnSkill::CanApply::ApplyDuplicateSameSkill;
	}

	// When stacking state effects, must not exceed the maximum stack count.
	// Do not stack if a higher-level state effect is already applied.
	if( CDnSkill::CanApply::Fail != eResult )
	{
		if( bExistingSameSkill )
		{
			if( iDuplicateCount <= iExistingSameSkillCount )
			{
				// If exceeding max stack count, only reset the duration of the existing state effect.
				int iNumSameSkillBlowsToResetDurationTime = (int)vlhSameSkillBlows.size();
				for( int iBlow = 0; iBlow < iNumSameSkillBlowsToResetDurationTime; ++iBlow )
				{
					DnBlowHandle hBlow = vlhSameSkillBlows.at( iBlow );

					//////////////////////////////////////////////////////////////////////////
					//#53448
					//State effect 249 cannot be added beyond the max stack count.
					//If max stack count is exceeded, no need to loop below to remove existing state effect and re-add.
					if (hBlow && 
						(hBlow->GetBlowIndex() == STATE_BLOW::BLOW_249 || hBlow->GetBlowIndex() == STATE_BLOW::BLOW_242)
						)
						continue;
					//////////////////////////////////////////////////////////////////////////

					// Deleting completely and re-adding is clearer.
					int nDurationTime = int(hBlow->GetDurationTime() * 1000.0f);
					hBlow->ResetDurationTime();

					CDnSkill::SkillInfo SkillInfo = *(hBlow->GetParentSkillInfo());
					STATE_BLOW::emBLOW_INDEX BlowIndex = hBlow->GetBlowIndex();
					string strValue( hBlow->GetValue() );

					// Must be deleted immediately at this point. [2011/01/18 semozz]
					int nBlowID = hBlow->GetBlowID();
					// Send packets to be sent
					hTargetActor->SendRemoveStateEffectFromID(nBlowID);
					// Delete immediately.
					if (hTargetActor->GetStateBlow())
						hTargetActor->GetStateBlow()->RemoveImediatlyStateEffectFromID(nBlowID);

					// Since probabilistic state effects like freeze must succeed 100% here, 
					// Call with bCheckCanBegin flag off so CanBegin is not called.
					hTargetActor->CmdAddStateEffect( &SkillInfo, BlowIndex, nDurationTime, strValue.c_str(), false, false );
				}

				eResult = CDnSkill::CanApply::Fail;
			}
		}
	}

	return eResult;
}


CDnSkill::CanApply CDnSkill::CanApplySkillStateEffect( DnActorHandle hTargetActor, DnSkillHandle hUsingSkill, map<int, bool>& mapDuplicateResult, bool isHitProcess )
{
	const SkillInfo* pUsingSkillInfo = hUsingSkill->GetInfo();
	int iSkillID = hUsingSkill->GetClassID();
	int iLevel = hUsingSkill->GetLevel();
	int iSkillDuplicateMethod = hUsingSkill->GetDuplicateMethod();
	int iDuplicateCount = hUsingSkill->GetMaxDuplicateCount();

	// Consider the skill as having a duration if any of its state effects has a duration
	// Allow the skill to be added directly if it has no duration. [2010/11/12 semozz]
	bool hasDuration = false;
	for( DWORD k = 0; k < hUsingSkill->GetStateEffectCount(); k++ ) 
	{
		CDnSkill::StateEffectStruct *pLocalStruct = hUsingSkill->GetStateEffectFromIndex(k);
		if( pLocalStruct && (IsNeedCheckApplyStateBlow((STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID) || pLocalStruct->nDurationTime > 0) )
		{
			hasDuration = true;
			break;
		}
#if defined(PRE_FIX_52267)
		//#52267
		//Even if state effect ApplyType is not Self, CanApplySkillStateEffect is not called when duration is 0, so
		//Unintended stacking occurs in some cases.
		//Set hasDuration to true so it can also be checked when ApplyType is not self and durationTime is 0.
		else if (pLocalStruct->nDurationTime == 0 && pLocalStruct->ApplyType != StateEffectApplyType::ApplySelf)
		{
			hasDuration = true;
			break;
		}
#endif // PRE_FIX_52267
	}

	bool bResist = false;
	if( !hTargetActor->IsHittableSkill( iSkillID , bResist ) )
	{
		if( bResist )
		{
			hTargetActor->SendAddSEFail( CDnStateBlow::ADD_FAIL_BY_IMMUNE , STATE_BLOW::BLOW_154 );
		}

		return  CDnSkill::CanApply::Fail;
	}

	if ( false == hasDuration )
	{
		return CDnSkill::CanApply::Apply;
	}

	return CanApplySkillStateEffect( pUsingSkillInfo, hTargetActor, iSkillID, iLevel, iSkillDuplicateMethod, iDuplicateCount, mapDuplicateResult, isHitProcess );
}


// Request action succession for an active skill that must have an action play processor!
void CDnSkill::OnChainInput( const char* pActionName )
{
	if( GetSkillType() == CDnSkill::Active &&
		( GetDurationType() == CDnSkill::Instantly ||
		  GetDurationType() == CDnSkill::Buff ||
		  GetDurationType() == CDnSkill::Debuff || 
		  GetDurationType() == CDnSkill::StanceChange ) )
	{
		IDnSkillProcessor* pProcessor = GetProcessor( IDnSkillProcessor::PLAY_ANI );
		_ASSERT( pProcessor );
		if( pProcessor )
		{
			CDnPlayAniProcess* pPlayAniProcessor = static_cast<CDnPlayAniProcess*>( pProcessor );
			pPlayAniProcessor->OnChainInput( pActionName );
		}
	}
	else
	if( GetSkillType() == CDnSkill::Passive )
	{
		CEtActionBase::ActionElementStruct* pStruct = m_hActor->GetElement( pActionName );
		if( pStruct )
		{
			m_fPassiveActionSkillLength += (float)pStruct->dwLength / s_fDefaultFps;
			m_bChainingPassiveSkill = true;
		}
	}
}

bool CDnSkill::IsChainInputAction( const char* pActionName )
{
	IDnSkillProcessor* pProcessor = GetProcessor( IDnSkillProcessor::PLAY_ANI );
	if( pProcessor )
	{
		CDnPlayAniProcess* pPlayAniProcessor = static_cast<CDnPlayAniProcess*>( pProcessor );
		if( strcmp( pActionName , pPlayAniProcessor->GetChainActionName() ) == 0 )
		{
			return true;
		}
	}

	return false;
}

bool CDnSkill::CheckChainingPassiveSkill( void )
{
	bool bResult = m_bChainingPassiveSkill;
	m_bChainingPassiveSkill = false;

	return bResult;
}

void CDnSkill::AddGlyphStateEffect( int nGlyphID )
{
	DNTableFileFormat* pGlyphTable = GetDNTable( CDnTableDB::TGLYPHSKILL );
	if( !pGlyphTable ) return;

#if defined(PRE_ADD_65808)
	int monsterID = -1;
	if (IsSummonMonsterRecall(monsterID) == true)
	{
		m_hActor->AddSummonMonsterGlyphInfo(monsterID, nGlyphID);
		return;
	}
#endif // PRE_ADD_65808

	char caLable[64];

	// For skill enhancement crests, set both PvP and PvE.
	for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
	{
		// Original count of state effects held
		int iNumOriginalStateEffect = (int)m_vlStateEffectList[ iSkillLevelApplyType ].size();

		// Add state effect
		StateEffectStruct StateEffect;
		StateEffect.nGlyphID = nGlyphID;
		for( int i = 0; i < MAX_GLYPH_STATE_EFFECT_COUNT; ++i )
		{
			sprintf_s( caLable, "_EffectClass%d", i + 1 );
			StateEffect.nID = pGlyphTable->GetFieldFromLablePtr( nGlyphID, caLable )->GetInteger();

			if( StateEffect.nID < 1 ) 
				continue;

			sprintf_s( caLable, "_EffectClass%dApplyType", i + 1 );
			int iApplyType = pGlyphTable->GetFieldFromLablePtr( nGlyphID, caLable )->GetInteger();
			bool bApplyAll = (StateEffectApplyType::ApplyAll == iApplyType);		// Apply-to-all. Add 2 identical state effects with different targets.

			if( bApplyAll )
			{
				StateEffect.ApplyType = StateEffectApplyType::ApplySelf;
				StateEffect.bApplyAllPair = true;
			}
			else
				StateEffect.ApplyType = (StateEffectApplyType)iApplyType;

			sprintf_s( caLable, "_EffectClassValue%d", i + 1 );
			StateEffect.szValue = pGlyphTable->GetFieldFromLablePtr( nGlyphID, caLable )->GetString();

			// Replace duration directly from the existing state effect.
			bool bFound = false;
			for( int k = 0; k < iNumOriginalStateEffect; ++k )
			{
				const StateEffectStruct& OriginalSE = m_vlStateEffectList[ iSkillLevelApplyType ].at( k );
				if( OriginalSE.nID == StateEffect.nID )
				{
					StateEffect.nDurationTime = OriginalSE.nDurationTime;
					bFound = true;
				}
			}
			
			// If there is no same state effect, just set duration to 0. 
			// Then the state effect lasts for the same interval as the action.
			if( false == bFound )
				StateEffect.nDurationTime = 0;

			m_vlStateEffectList[ iSkillLevelApplyType ].push_back( StateEffect );

			// If apply-to-all, switch to target and add the same one once more.
			if( bApplyAll )
			{
				StateEffect.ApplyType = StateEffectApplyType::ApplyTarget;
				m_vlStateEffectList[ iSkillLevelApplyType ].push_back( StateEffect );
			}
		}
	}

	// Add skill attribute
	GlyphEffectStruct GlyphEffect;
	
	GlyphEffect.eEffectIndex = pGlyphTable->GetFieldFromLablePtr( nGlyphID, "_GlyphSkillEffect" )->GetInteger();
	GlyphEffect.fEffectValue = pGlyphTable->GetFieldFromLablePtr( nGlyphID, "_GlyphSkillEffectValue" )->GetFloat();
	GlyphEffect.nGlyphID = nGlyphID;

	switch( GlyphEffect.eEffectIndex )
	{
	case SKILLMP_PRO :
		{
			for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
				m_iNeedMP[ iSkillLevelApplyType ] += (int)( (float)m_iOriginalNeedMP[ iSkillLevelApplyType ] * GlyphEffect.fEffectValue );
			break;
		}

	case SKILLDELAY_PRO :
		{
			for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
				m_fDelayTime[ iSkillLevelApplyType ] += m_fOriginalDelayTime[ iSkillLevelApplyType ] * GlyphEffect.fEffectValue;
			break;
		}

	case SKILLDELAY_RESET_PRO :
		{
			m_fResetCooltime += GlyphEffect.fEffectValue;
			break;
		}

	case SKILLDURATION_PRO :
		{
			// pair<int, int> <status effect, duration>
			for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
			{
				bool bEmpty = m_vlStateDurationList[iSkillLevelApplyType].empty();
				for( int itr = 0; itr < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++itr ) {
					if( bEmpty )
						m_vlStateDurationList[iSkillLevelApplyType].push_back( pair<int, int>(m_vlStateEffectList[ iSkillLevelApplyType ][itr].nID, m_vlStateEffectList[ iSkillLevelApplyType ][itr].nDurationTime) );
					else m_vlStateEffectList[ iSkillLevelApplyType ][itr].nDurationTime = m_vlStateDurationList[iSkillLevelApplyType][itr].second;
				}
			}

			float fRatio = GlyphEffect.fEffectValue;
			for( int itr = 0; itr < (int)m_vGlyphEffectList.size(); ++itr )
			{
				if( SKILLDURATION_PRO == m_vGlyphEffectList[itr].eEffectIndex )
					fRatio += m_vGlyphEffectList[itr].fEffectValue;
			}

			for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
			{
				for( int itr = 0; itr < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++itr )
					m_vlStateEffectList[ iSkillLevelApplyType ][itr].nDurationTime += (int)(m_vlStateEffectList[ iSkillLevelApplyType ][itr].nDurationTime * fRatio);
			}

			break;
		}
	case SKILLATTACKHEAL_ABSOLUTE:
		{
			float fValue = GlyphEffect.fEffectValue;

			for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
			{
				for( int itr = 0; itr < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++itr )
				{
					if( m_vlStateEffectList[ iSkillLevelApplyType ][itr].nID == STATE_BLOW::BLOW_248 )
					{
						m_vlStateEffectList[ iSkillLevelApplyType ][itr].szValue = FormatA( "%f", fValue + atof(m_vlStateEffectList[ iSkillLevelApplyType ][itr].szValue.c_str()) ).c_str();
					}
				}
			}
		}
		break;
	}

	m_vGlyphEffectList.push_back( GlyphEffect );
}

void CDnSkill::DelGlyphStateEffect( int nGlyphID )
{
#if defined(PRE_ADD_65808)
	int monsterID = -1;
	if (IsSummonMonsterRecall(monsterID) == true)
	{
		m_hActor->RemoveSummonMonsterGlyphInfo(monsterID, nGlyphID);
		return;
	}
#endif // PRE_ADD_65808

	// For skill enhancement crests, set both PvP and PvE.
	for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
	{
		for( DNVector(StateEffectStruct)::iterator Itor = m_vlStateEffectList[ iSkillLevelApplyType ].begin(); 
			Itor < m_vlStateEffectList[ iSkillLevelApplyType ].end(); )
		{
			if( nGlyphID == Itor->nGlyphID )
				Itor = m_vlStateEffectList[ iSkillLevelApplyType ].erase(Itor);
			else
				++Itor;
		}
	}

	for( std::vector<GlyphEffectStruct>::iterator Itor = m_vGlyphEffectList.begin();
		Itor < m_vGlyphEffectList.end(); )
	{
		if( nGlyphID == Itor->nGlyphID )
		{
			switch( Itor->eEffectIndex )
			{
			case SKILLMP_PRO :
				{
					for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
						m_iNeedMP[ iSkillLevelApplyType ] -= (int)( (float)m_iOriginalNeedMP[ iSkillLevelApplyType ] * Itor->fEffectValue );
					break;
				}

			case SKILLDELAY_PRO :
				{
					for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
						m_fDelayTime[ iSkillLevelApplyType ] -= m_fOriginalDelayTime[ iSkillLevelApplyType ] * Itor->fEffectValue;
					break;
				}

			case SKILLDELAY_RESET_PRO :
				{
					m_fResetCooltime -= Itor->fEffectValue;
					break;
				}

			case SKILLDURATION_PRO :
				{
					// pair<int, int> <status effect, duration>
					bool bExist = false, bOnce = true;
					float fRatio = 0.0f;
					for( int itr = 0; itr < (int)m_vGlyphEffectList.size(); ++itr )
					{
						if( Itor->nGlyphID == m_vGlyphEffectList[itr].nGlyphID && bOnce) 
						{
							bOnce = false;
							continue;
						}
						if( SKILLDURATION_PRO == m_vGlyphEffectList[itr].eEffectIndex )
						{
							bExist = true;
							fRatio += m_vGlyphEffectList[itr].fEffectValue;
						}
					}

					if( !bExist )
					{
						for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
						{
							for( int itr = 0; itr < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++itr )
								m_vlStateEffectList[ iSkillLevelApplyType ][itr].nDurationTime = m_vlStateDurationList[iSkillLevelApplyType][itr].second;
							m_vlStateDurationList[iSkillLevelApplyType].clear();
						}
					}
					else
					{
						for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
						{
							for( int itr = 0; itr < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++itr )
								m_vlStateEffectList[ iSkillLevelApplyType ][itr].nDurationTime = m_vlStateDurationList[iSkillLevelApplyType][itr].second + (int)(m_vlStateDurationList[iSkillLevelApplyType][itr].second * fRatio);
						}
					}

					break;
				}
			case SKILLATTACKHEAL_ABSOLUTE:
				{
					float fValue = Itor->fEffectValue;
					for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
					{
						for( int itr = 0; itr < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++itr )
						{
							if( m_vlStateEffectList[ iSkillLevelApplyType ][itr].nID == STATE_BLOW::BLOW_248 )
								m_vlStateEffectList[ iSkillLevelApplyType ][itr].szValue = FormatA( "%f", atof(m_vlStateEffectList[ iSkillLevelApplyType ][itr].szValue.c_str()) - fValue).c_str();
						}
					}
				}
				break;
			}

			Itor = m_vGlyphEffectList.erase( Itor );
		}
		else
			++Itor;
	}
}

void CDnSkill::UpdateSkillCoolTimeExactly( void )
{
	// Skip server-side check when cooldown-modifying state effects 96 or 171 are active.
	// Cooldown update on game server was reduced to 10 frames, so relaxed this for control feel. 2011.07.25
	DWORD dwNowTime = timeGetTime();
	if(	DWORD(GetDelayTime()*1000.0f) < dwNowTime - m_dwLastUseSkillTimeStamp )
	{
		ResetCoolTime();
	}
}


bool CDnSkill::SelectLevelDataType( int iSkillLevelDataType, bool bPlayerSummonedMonster/* = false*/ )
{ 
	_ASSERT( (m_hActor->IsPlayerActor() || bPlayerSummonedMonster) && (PVE <= iSkillLevelDataType) && (iSkillLevelDataType < NUM_SKILLLEVEL_APPLY_TYPE) );

	if( (m_hActor->IsPlayerActor() || bPlayerSummonedMonster) &&
		(PVE <= iSkillLevelDataType) &&
		(iSkillLevelDataType < NUM_SKILLLEVEL_APPLY_TYPE) ) 
	{
		m_iSelectedSkillLevelDataApplyType = iSkillLevelDataType;
		return true;
	}

	return false;
};


bool CDnSkill::IsNeedCheckApplyStateBlow( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	bool bResult = false;

	if( STATE_BLOW::BLOW_030 == emBlowIndex )
		bResult = true;

	return bResult;
}

int CDnSkill::GetBaseSkillID( void )
{
	int iResult = 0;

	if( EnchantPassive == m_eSkillType )
	{
		iResult = m_iBaseSkillID;
	}

	return iResult;
}

// Keep client in sync as well; also update CheckAndDivideStateEffectArgument. 
bool CDnSkill::CheckAndUnifyStateEffectArgument( int iSkillLevelApplyType, StateEffectStruct* pEnchantSkillSE )
{
	bool bResult = false;

	if( CDnCreateBlow::IsBasicBlow( (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID ) )
	{
		for( int i = 0; i < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++i )
		{
			StateEffectStruct* pDestSkillSE = &(m_vlStateEffectList[ iSkillLevelApplyType ].at( i ));
			if( pDestSkillSE->nID == pEnchantSkillSE->nID )
			{
				CDnBasicBlow::AddStateEffectValue( pDestSkillSE->szValue.c_str(), pEnchantSkillSE->szValue.c_str(), pDestSkillSE->szValue );
				pDestSkillSE->strEnchantSkillSEParam = pEnchantSkillSE->szValue;
				bResult = true;
			}
		}
	}
	else if( STATE_BLOW::BLOW_016 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID )
	{
		for( int i = 0; i < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++i )
		{
			StateEffectStruct* pDestSkillSE = &(m_vlStateEffectList[ iSkillLevelApplyType ].at( i ));
			if( pDestSkillSE->nID == pEnchantSkillSE->nID )
			{
				CDnHPIncBlow::AddStateEffectValue( pDestSkillSE->szValue.c_str(), pEnchantSkillSE->szValue.c_str(), pDestSkillSE->szValue );
				pDestSkillSE->strEnchantSkillSEParam = pEnchantSkillSE->szValue;
				bResult = true;
			}
		}
	}
	else if( STATE_BLOW::BLOW_208 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID )
	{
		for( int i = 0; i < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++i )
		{
			StateEffectStruct* pDestSkillSE = &(m_vlStateEffectList[ iSkillLevelApplyType ].at( i ));
			if( pDestSkillSE->nID == pEnchantSkillSE->nID )
			{
				CDnPingpongBlow::AddStateEffectValue( pDestSkillSE->szValue.c_str(), pEnchantSkillSE->szValue.c_str(), pDestSkillSE->szValue );
				pDestSkillSE->strEnchantSkillSEParam = pEnchantSkillSE->szValue;
				bResult = true;
			}
		}
	}
	else if( STATE_BLOW::BLOW_140 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID )
	{
		for( int i = 0; i < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++i )
		{
			StateEffectStruct* pDestSkillSE = &(m_vlStateEffectList[ iSkillLevelApplyType ].at( i ));
			if( pDestSkillSE->nID == pEnchantSkillSE->nID )
			{
				CDnHealingBlow::AddStateEffectValue( pDestSkillSE->szValue.c_str(), pEnchantSkillSE->szValue.c_str(), pDestSkillSE->szValue );
				pDestSkillSE->strEnchantSkillSEParam = pEnchantSkillSE->szValue;
				bResult = true;
			}
		}
	}
	else if( STATE_BLOW::BLOW_227 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID )
	{
		for( int i = 0; i < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++i )
		{
			StateEffectStruct* pDestSkillSE = &(m_vlStateEffectList[ iSkillLevelApplyType ].at( i ));
			if( pDestSkillSE->nID == pEnchantSkillSE->nID )
			{
				CDnBloodSuckingBlow::AddStateEffectValue( pDestSkillSE->szValue.c_str(), pEnchantSkillSE->szValue.c_str(), pDestSkillSE->szValue );
				pDestSkillSE->strEnchantSkillSEParam = pEnchantSkillSE->szValue;
				bResult = true;
			}
		}
	}
	else if( STATE_BLOW::BLOW_215 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID )
	{
		for( int i = 0; i < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++i )
		{
			StateEffectStruct* pDestSkillSE = &(m_vlStateEffectList[ iSkillLevelApplyType ].at( i ));
			if( pDestSkillSE->nID == pEnchantSkillSE->nID )
			{
				CDnOrderMySummonedMonsterBlow::AddStateEffectValue( pDestSkillSE->szValue.c_str(), pEnchantSkillSE->szValue.c_str(), pDestSkillSE->szValue );
				pDestSkillSE->strEnchantSkillSEParam = pEnchantSkillSE->szValue;
				bResult = true;
			}
		}
	}
	else if( STATE_BLOW::BLOW_179 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID
			|| STATE_BLOW::BLOW_121 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID
			|| STATE_BLOW::BLOW_129 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID
			|| STATE_BLOW::BLOW_062 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID 
			|| STATE_BLOW::BLOW_242 == (STATE_BLOW::emBLOW_INDEX)pEnchantSkillSE->nID )
	{
		for( int i = 0; i < (int)m_vlStateEffectList[ iSkillLevelApplyType ].size(); ++i )
		{
			StateEffectStruct* pDestSkillSE = &(m_vlStateEffectList[ iSkillLevelApplyType ].at( i ));
			if( pDestSkillSE->nID == pEnchantSkillSE->nID )
			{
				pDestSkillSE->strEnchantSkillSEParam = pDestSkillSE->szValue;
				pDestSkillSE->szValue = pEnchantSkillSE->szValue;
				bResult = true;
				break;
			}
		}
	}

	return bResult;
}

bool CDnSkill::ApplyEnchantSkill( DnSkillHandle hEnchantPassiveSkill )
{
	bool bSuccess = false;

	if( 0 < m_iAppliedEnchantPassiveSkillID )
		return false;

	_ASSERT( m_iSkillID == hEnchantPassiveSkill->GetBaseSkillID() );
	_ASSERT( EnchantPassive != m_eSkillType );
	if( EnchantPassive != m_eSkillType )
	{
		int iNowLevelDataType = m_iSelectedSkillLevelDataApplyType;
		for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
		{
			// Add argument values to existing state effects and append new state effects to the list.
			// Overwrite the cooldown with the enhanced passive skill's cooldown.
			SelectLevelDataType( iSkillLevelApplyType );
			hEnchantPassiveSkill->SelectLevelDataType( iSkillLevelApplyType );
			int iNumStateEffect = (int)hEnchantPassiveSkill->GetStateEffectCount();
			for( int i = 0; i < iNumStateEffect; ++i )
			{
				StateEffectStruct* pEnchantPassiveSE = hEnchantPassiveSkill->GetStateEffectFromIndex( i );

				// #40643 Case of modifying parameters of specially implemented state effects like Ping Pong Bomb EX.
				// Have no choice but to merge them. Not actually added to m_vlStateEffectList.
				if( false == CheckAndUnifyStateEffectArgument( iSkillLevelApplyType, pEnchantPassiveSE ) )
				{
					// Whether attack power buff state effect or not, adding it automatically refreshes stats on skill use.
					m_vlStateEffectList[ iSkillLevelApplyType ].push_back( *pEnchantPassiveSE );
					m_vlStateEffectList[ iSkillLevelApplyType ].back().nFromEnchantPassiveSkillID = hEnchantPassiveSkill->GetClassID();
				}
			}

			// Change the action processor to that of the enhanced skill.
			IDnSkillProcessor* pMyPlayAniProcessor = GetProcessor( IDnSkillProcessor::PLAY_ANI );
			if( pMyPlayAniProcessor )
			{
				IDnSkillProcessor* pEnchantPassiveSkillPlayAniProcessor =  hEnchantPassiveSkill->GetProcessor( IDnSkillProcessor::PLAY_ANI );
				if( pEnchantPassiveSkillPlayAniProcessor )
				{
					// Back up existing one.
					IDnSkillProcessor* pBackup = new CDnPlayAniProcess;
					pBackup->CopyFrom( pMyPlayAniProcessor );
					m_vlpProcessorBackup[ iSkillLevelApplyType ].push_back( pBackup );

					// Apply data from the enhancement passive skill.
					pMyPlayAniProcessor->CopyFrom( pEnchantPassiveSkillPlayAniProcessor );
				}
			}

			IDnSkillProcessor* pMyPartialPlayAniProcessor = GetProcessor( IDnSkillProcessor::PARTIAL_PLAY_ANI );
			if( pMyPartialPlayAniProcessor )
			{
				IDnSkillProcessor* pEnchantPassivePartialSkillPlayAniProcessor =  hEnchantPassiveSkill->GetProcessor( IDnSkillProcessor::PARTIAL_PLAY_ANI );
				if( pEnchantPassivePartialSkillPlayAniProcessor )
				{
					// Back up existing one.
					IDnSkillProcessor* pBackup = new CDnPartialPlayProcessor;
					pBackup->CopyFrom( pMyPartialPlayAniProcessor );
					m_vlpProcessorBackup[ iSkillLevelApplyType ].push_back( pBackup );

					pMyPartialPlayAniProcessor->CopyFrom( pEnchantPassivePartialSkillPlayAniProcessor );
				}
			}

			IDnSkillProcessor* pMyChangeActionStrProcessor = GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR );
			if( pMyChangeActionStrProcessor )
			{
				IDnSkillProcessor* pEnchantPassiveChangeActionStrProcessor =  hEnchantPassiveSkill->GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR );
				if( pEnchantPassiveChangeActionStrProcessor )
				{
					// Back up existing one.
					IDnSkillProcessor* pBackup = new CDnChangeActionStrProcessor;
					pBackup->CopyFrom( pMyChangeActionStrProcessor );
					m_vlpProcessorBackup[ iSkillLevelApplyType ].push_back( pBackup );

					pMyChangeActionStrProcessor->CopyFrom( pEnchantPassiveChangeActionStrProcessor );
				}
			}

#ifdef PRE_ADD_ENCHANTSKILL_BUBBLE_ACTION
			IDnSkillProcessor* pMyChangeActionStrByBubbleProcessor = GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR_BY_BUBBLE );
			if( pMyChangeActionStrByBubbleProcessor )
			{
				IDnSkillProcessor* pEnchantPassiveChangeActionStrProcessor =  hEnchantPassiveSkill->GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR_BY_BUBBLE );
				if( pEnchantPassiveChangeActionStrProcessor )
				{
					IDnSkillProcessor* pBackup = new CDnChangeActionStrByBubbleProcessor( );
					pBackup->CopyFrom( pMyChangeActionStrByBubbleProcessor );
					m_vlpProcessorBackup[ iSkillLevelApplyType ].push_back( pBackup );

					pMyChangeActionStrByBubbleProcessor->CopyFrom( pEnchantPassiveChangeActionStrProcessor );
				}
			}
#endif

			m_eElement = hEnchantPassiveSkill->GetElement();
			m_SkillInfo[iSkillLevelApplyType].eSkillElement = hEnchantPassiveSkill->GetElement();

#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
			bool bSummonMonsterToggleSkill = ( m_isAppliedSummonMonsterEnchantSkill && hEnchantPassiveSkill->GetDurationType() == CDnSkill::ActiveToggleForSummon );
			if( bSummonMonsterToggleSkill == false )
			{
				m_eDurationType = hEnchantPassiveSkill->GetDurationType();
				m_SkillInfo[iSkillLevelApplyType].eDurationType = hEnchantPassiveSkill->GetDurationType();
			}
#endif

#if defined(PRE_FIX_66175)
			//#66175
			//Swap UsableChecker between base skill and EX skill...
			vector<IDnSkillUsableChecker*>& enchangeSkillUsableChecker = hEnchantPassiveSkill->GetUsableChecker(iSkillLevelApplyType);

			//Replace only when UsableChecker is set on the EX skill...
			if (enchangeSkillUsableChecker.size() > 0)
			{
				vector<IDnSkillUsableChecker*>& baseSkillUsableChecker = GetUsableChecker(iSkillLevelApplyType);

				{
					//Store own Checkers in the backup list.
					vector<IDnSkillUsableChecker*>::iterator iter = baseSkillUsableChecker.begin();
					vector<IDnSkillUsableChecker*>::iterator endIter = baseSkillUsableChecker.begin();

					//Save into the backup list...
					m_vlUsableCheckersBackup[iSkillLevelApplyType].clear();

					for (; iter != endIter; ++iter)
					{
						m_vlUsableCheckersBackup[iSkillLevelApplyType].push_back((*iter));
					}

				}				

				//Apply the EX skill's usableChecker to the base skill.
				baseSkillUsableChecker.clear();
				vector<IDnSkillUsableChecker*>::iterator iter = enchangeSkillUsableChecker.begin();
				vector<IDnSkillUsableChecker*>::iterator endIter = enchangeSkillUsableChecker.end();
				for (; iter != endIter; ++iter)
				{
					IDnSkillUsableChecker* pChecker = (*iter);
					baseSkillUsableChecker.push_back(pChecker->Clone());
				}
			}
#endif // PRE_FIX_66175

			//Change EffectIDs as well..
			string enchantEffectIDs = hEnchantPassiveSkill->GetEffectOutputIDs(iSkillLevelApplyType);
			string enchantEffectIDToClient = hEnchantPassiveSkill->GetEffectOutputIDToClient(iSkillLevelApplyType);

			m_BackupEffectOutputIDs[iSkillLevelApplyType] = m_SkillInfo[iSkillLevelApplyType].szEffectOutputIDs;
			m_BackupEffectOutputIDToClient[iSkillLevelApplyType] = m_SkillInfo[iSkillLevelApplyType].szEffectOutputIDToClient;

			//When sending SC_CMDADDSTATEEFFECT packet to client, if szEffectOutputIDToClient is empty, the client reads and uses the table value.
			//When using an EX skill, the base skill ID is passed as the skill ID, so values modified by the EX skill cannot be used.
			//When applying EX skill, change szEffectOutputIDToClient to the EX skill value.
			m_SkillInfo[iSkillLevelApplyType].szEffectOutputIDs = enchantEffectIDs;
			m_SkillInfo[iSkillLevelApplyType].szEffectOutputIDToClient = enchantEffectIDs;

#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
			m_SkillInfo[iSkillLevelApplyType].iAppliedEnchantSkillID = hEnchantPassiveSkill->GetClassID();
#endif
		}

		SelectLevelDataType( iNowLevelDataType );

		m_iAppliedEnchantPassiveSkillID = hEnchantPassiveSkill->GetClassID();
		bSuccess = true;
	}

	return bSuccess;
}


void CDnSkill::ApplyEnchantSkillOnceFromBubble( DnSkillHandle hEnchantPassiveSkill )
{
	m_bEnchantedFromBubble = true;
	ApplyEnchantSkill( hEnchantPassiveSkill );
}


bool CDnSkill::CheckAndDivideStateEffectArgument( StateEffectStruct* pDestSkillSE )
{
	if( pDestSkillSE->strEnchantSkillSEParam.empty() == true )
		return false;

	bool bResult = false;

	if( CDnCreateBlow::IsBasicBlow( (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID ) )
	{
		CDnBasicBlow::RemoveStateEffectValue( pDestSkillSE->szValue.c_str(), pDestSkillSE->strEnchantSkillSEParam.c_str(), pDestSkillSE->szValue );
		pDestSkillSE->strEnchantSkillSEParam.clear();
		bResult = true;
	}
	else if( STATE_BLOW::BLOW_016 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID )
	{
		CDnHPIncBlow::RemoveStateEffectValue( pDestSkillSE->szValue.c_str(), pDestSkillSE->strEnchantSkillSEParam.c_str(), pDestSkillSE->szValue );
		pDestSkillSE->strEnchantSkillSEParam.clear();
		bResult = true;
	}
	else if( STATE_BLOW::BLOW_208 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID )
	{
		CDnPingpongBlow::RemoveStateEffectValue( pDestSkillSE->szValue.c_str(), pDestSkillSE->strEnchantSkillSEParam.c_str(), pDestSkillSE->szValue );
		pDestSkillSE->strEnchantSkillSEParam.clear();
		bResult = true;
	}
	else if( STATE_BLOW::BLOW_140 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID )
	{
		CDnHealingBlow::RemoveStateEffectValue( pDestSkillSE->szValue.c_str(), pDestSkillSE->strEnchantSkillSEParam.c_str(), pDestSkillSE->szValue );
		pDestSkillSE->strEnchantSkillSEParam.clear();
		bResult = true;
	}
	else if( STATE_BLOW::BLOW_227 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID )
	{
		CDnBloodSuckingBlow::RemoveStateEffectValue( pDestSkillSE->szValue.c_str(), pDestSkillSE->strEnchantSkillSEParam.c_str(), pDestSkillSE->szValue );
		pDestSkillSE->strEnchantSkillSEParam.clear();
		bResult = true;
	}
	else if( STATE_BLOW::BLOW_215 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID )
	{
		CDnOrderMySummonedMonsterBlow::RemoveStateEffectValue( pDestSkillSE->szValue.c_str(), pDestSkillSE->strEnchantSkillSEParam.c_str(), pDestSkillSE->szValue );
		pDestSkillSE->strEnchantSkillSEParam.clear();
		bResult = true;
	}
	else if( STATE_BLOW::BLOW_179 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID
		|| STATE_BLOW::BLOW_121 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID
		|| STATE_BLOW::BLOW_129 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID
		|| STATE_BLOW::BLOW_062 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID 
		|| STATE_BLOW::BLOW_242 == (STATE_BLOW::emBLOW_INDEX)pDestSkillSE->nID )
	{
		pDestSkillSE->szValue = pDestSkillSE->strEnchantSkillSEParam;
		pDestSkillSE->strEnchantSkillSEParam.clear();
		bResult = true;
	}
	
	return bResult;
}

void CDnSkill::ReleaseEnchantSkill( void )
{
	if( 0 == m_iAppliedEnchantPassiveSkillID )
		return;

	_ASSERT( EnchantPassive != m_eSkillType );
	if( EnchantPassive != m_eSkillType )
	{
		int iNowLevelDataType = m_iSelectedSkillLevelDataApplyType;
		for( int iSkillLevelApplyType = PVE; iSkillLevelApplyType < NUM_SKILLLEVEL_APPLY_TYPE; ++iSkillLevelApplyType )
		{
			// Add argument values to existing state effects and append new state effects to the list.
			// Overwrite the cooldown with the enhanced passive skill's cooldown.
			SelectLevelDataType( iSkillLevelApplyType );
			DNVector( StateEffectStruct )::iterator iter = m_vlStateEffectList[ iSkillLevelApplyType ].begin();
			for( iter; iter != m_vlStateEffectList[ iSkillLevelApplyType ].end(); )
			{
				if( false == CheckAndDivideStateEffectArgument( &(*iter) ) )
				{
					if( 0 < iter->nFromEnchantPassiveSkillID )
					{
						iter = m_vlStateEffectList[ iSkillLevelApplyType ].erase( iter );
						continue;
					}
				}

				++iter;
			}

			for( int i = 0; i < (int)m_vlpProcessorBackup[ iSkillLevelApplyType ].size(); ++i )
			{
				IDnSkillProcessor* pProcessor = m_vlpProcessorBackup[ iSkillLevelApplyType ].at( i );
				switch( pProcessor->GetType() )
				{
					case IDnSkillProcessor::PLAY_ANI:
						{
							IDnSkillProcessor* pMyPlayAniProcessor = GetProcessor( IDnSkillProcessor::PLAY_ANI );
							if( pMyPlayAniProcessor )
								pMyPlayAniProcessor->CopyFrom( pProcessor );
						}
						break;

					case IDnSkillProcessor::PARTIAL_PLAY_ANI:
						{
							IDnSkillProcessor* pMyPartialPlayAniProcessor = GetProcessor( IDnSkillProcessor::PARTIAL_PLAY_ANI );
							if( pMyPartialPlayAniProcessor )
								pMyPartialPlayAniProcessor->CopyFrom( pProcessor );
						}
						break;
					case IDnSkillProcessor::CHANGE_ACTIONSTR:
						{
							IDnSkillProcessor* pMyChangeActionStrProcessor = GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR );
							if( pMyChangeActionStrProcessor )
								pMyChangeActionStrProcessor->CopyFrom( pProcessor );
						}
						break;
#ifdef PRE_ADD_ENCHANTSKILL_BUBBLE_ACTION
					case IDnSkillProcessor::CHANGE_ACTIONSTR_BY_BUBBLE:
						{
							IDnSkillProcessor* pMyChangeActionStrByBubbleProcessor = GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR_BY_BUBBLE );
							if( pMyChangeActionStrByBubbleProcessor )
								pMyChangeActionStrByBubbleProcessor->CopyFrom( pProcessor );
						}
						break;
#endif
				}
			}

			SAFE_DELETE_PVEC( m_vlpProcessorBackup[ iSkillLevelApplyType ] );

#if defined(PRE_FIX_66175)
			if (m_vlUsableCheckersBackup[iSkillLevelApplyType].size() > 0)
			{
				std::swap(m_vlpUsableCheckers[iSkillLevelApplyType], m_vlUsableCheckersBackup[iSkillLevelApplyType]);
			}
#endif // PRE_FIX_66175

			//Change EffectIDs as well..
			m_SkillInfo[iSkillLevelApplyType].szEffectOutputIDs = m_BackupEffectOutputIDs[iSkillLevelApplyType];
			m_SkillInfo[iSkillLevelApplyType].szEffectOutputIDToClient = m_BackupEffectOutputIDToClient[iSkillLevelApplyType];

			m_BackupEffectOutputIDs[iSkillLevelApplyType].clear();
			m_BackupEffectOutputIDToClient[iSkillLevelApplyType].clear();
		}

		SelectLevelDataType( iNowLevelDataType );
		m_iAppliedEnchantPassiveSkillID = 0;
	}
}


IDnSkillUsableChecker* CDnSkill::GetChecker( int iType )
{
	IDnSkillUsableChecker* pResult = NULL;

	int iNumChecker = (int)m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].size();
	for( int iChecker = 0; iChecker < iNumChecker; ++iChecker )
	{
		IDnSkillUsableChecker* pSkillChecker = m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].at( iChecker );
		if( iType == pSkillChecker->GetType() )
		{
			pResult = pSkillChecker;
			break;
		}
	}

	return pResult;
}

bool CDnSkill::IsExistUsableChecker( int iType ) // Does it have a defined UsableChecker?
{
	int iNumChecker = (int)m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].size();
	for( int iChecker = 0; iChecker < iNumChecker; ++iChecker )
	{
		IDnSkillUsableChecker* pSkillChecker = m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].at( iChecker );
		if( iType == pSkillChecker->GetType() )
			return true;
	}
	return false;
}

bool CDnSkill::IsFinishedAuraSkill()
{
	//For aura skill with continuous flat/percent mana change, sum total mana delta and compare with remaining mana
	//Determine whether the skill can continue.
	if (m_hActor)
	{
		float fIncManaDelta = 0.0f;

		//Absolute increment
		DNVector( DnBlowHandle ) vlManaAbIncBlows;
		m_hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_013, vlManaAbIncBlows );
		if (!vlManaAbIncBlows.empty())
		{
			for( int i = 0; i < (int)vlManaAbIncBlows.size(); ++i )
			{
				DnBlowHandle hBlow = vlManaAbIncBlows.at( i );
				fIncManaDelta += hBlow->GetFloatValue();							
			}
		}

		//Ratio increment
		float fMaxMP = static_cast<float>(m_hActor->GetMaxSP());
		DNVector( DnBlowHandle ) vlManaRateIncBlows;
		m_hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_014, vlManaRateIncBlows );
		if (!vlManaRateIncBlows.empty())
		{
			for( int i = 0; i < (int)vlManaRateIncBlows.size(); ++i )
			{
				DnBlowHandle hBlow = vlManaRateIncBlows.at( i );
				fIncManaDelta += fMaxMP * hBlow->GetFloatValue();							
			}
		}

#if defined(PRE_FIX_46381)
		//Apply state effect 231.
		DNVector(DnBlowHandle) vlContinueBaseMPIncBlows;
		m_hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_231, vlContinueBaseMPIncBlows);
		if (!vlContinueBaseMPIncBlows.empty())
		{
			for (int i = 0; i < (int)vlContinueBaseMPIncBlows.size(); ++i)
			{
				DnBlowHandle hBlow = vlContinueBaseMPIncBlows.at(i);
				fIncManaDelta += static_cast<CDnContinueBaseMPIncBlow*>(hBlow.GetPointer())->GetMPIncValue();
			}
		}
#endif // PRE_FIX_46381

		//Final: if decrement exceeds current remaining mana, insufficient mana..
		if (m_hActor->GetSP() < -(int)fIncManaDelta)
			return true;		
	}
	
	return false;
}

void CDnSkill::AddPrefixBlow(DnActorHandle hActor, int nBlowID)
{
	PREFIX_SKILL_BLOWLIST::iterator findIter = m_PrefixBlowList.find(hActor);
	if (findIter != m_PrefixBlowList.end())
	{
		findIter->second.push_back(nBlowID);
	}
	else
	{
		BLOWLIST blowList;
		blowList.push_back(nBlowID);

		m_PrefixBlowList.insert(PREFIX_SKILL_BLOWLIST::value_type(hActor, blowList));
	}
}

void CDnSkill::RemovePrefixBlow()
{
	PREFIX_SKILL_BLOWLIST::iterator iter = m_PrefixBlowList.begin();
	PREFIX_SKILL_BLOWLIST::iterator endIter = m_PrefixBlowList.end();

	for (; iter != endIter; ++iter)
	{
		DnActorHandle hActor = iter->first;
		if (!hActor)
			continue;

		BLOWLIST& blowList = iter->second;
		for (int i = 0; i < (int)blowList.size(); ++i)
		{
			// [2010/12/13 semozz]
			// Since all state effects are now created via packets, remove state effects via packet.
			hActor->CmdRemoveStateEffectFromID( blowList[i] );
		}

		blowList.clear();
	}

	m_PrefixBlowList.clear();
}

void CDnSkill::OnInitializeSummonMonsterInfo()
{
	if (!m_hActor)
		return;

	set<string>::iterator iter = m_setUseActionNames.begin();
	set<string>::iterator endIter = m_setUseActionNames.end();

	string actionName;
	CEtActionBase::ActionElementStruct *pActionElement = NULL;

	bool bFindMonsterID = false;

	for (; iter != endIter; ++iter)
	{
		actionName = *iter;
		pActionElement = m_hActor->GetElement(actionName.c_str());
		if (pActionElement)
		{
			CEtActionSignal *pSignal = NULL;
			for( DWORD i=0; i<pActionElement->pVecSignalList.size(); i++ ) 
			{
				pSignal = pActionElement->pVecSignalList[i];

				if (pSignal->GetSignalIndex() == STE_SummonMonster)
				{
					SummonMonsterStruct* pStruct = (SummonMonsterStruct *)pSignal->GetData();

					m_SummonMonsterID = pStruct ? pStruct->MonsterID : -1;

					if (m_SummonMonsterID != -1) 
					{
						bFindMonsterID = true;
						break;
					}
				}
			}
		}

		if (bFindMonsterID)
			break;
	}
	
}
DnActorHandle CDnSkill::FindSummonMonster(int nMonsterID)
{
	DnActorHandle hSummonMonster;

	if (m_hActor && m_hActor->IsPlayerActor())
	{
		const list<DnMonsterActorHandle>& listSummonMonster = m_hActor->GetSummonedMonsterList();

		if( false == listSummonMonster.empty() )
		{
			list<DnMonsterActorHandle>::const_iterator iter = listSummonMonster.begin();
			for( iter; iter != listSummonMonster.end(); ++iter )
			{
				DnMonsterActorHandle hMonster = (*iter);
				if( hMonster && hMonster->GetMonsterClassID() == nMonsterID )
				{
					hSummonMonster = hMonster;
					break;
				}
			}
		}	
	}

	return hSummonMonster;
}

bool CDnSkill::SummonMonsterOff()
{
	DnActorHandle hSummonMonster;

	if (m_SummonMonsterID != -1)
		hSummonMonster = FindSummonMonster(m_SummonMonsterID);

	if (hSummonMonster)
	{
		hSummonMonster->CmdSuicide(false, false);
		return true;
	}
	else
		return false;
}

bool CDnSkill::IsSkipStateBlow(const char* szSkipStateBlows, STATE_BLOW::emBLOW_INDEX blowIndex)
{
	if (szSkipStateBlows == NULL)
		return false;

	std::string stringValue = szSkipStateBlows;
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	TokenizeA(stringValue, tokens, delimiters);

	for (UINT i = 0; i < tokens.size(); ++i)
	{
		STATE_BLOW::emBLOW_INDEX skipStateBlowIndex = (STATE_BLOW::emBLOW_INDEX)(atoi(tokens[i].c_str()));
		if (skipStateBlowIndex == blowIndex)
			return true;
	}

	return false;
}

void CDnSkill::AddProjectile(CDnProjectile* pProjectile)
{
	if (pProjectile)
	{
		LOCAL_TIME startTime = pProjectile->GetSkillStartTime();

		SKILL_PROJECTILE_LIST::iterator findIter = m_ProjectileList.find(startTime);
		
		if (findIter == m_ProjectileList.end())
		{
			PROJECTILE_LIST projectileList;
			projectileList.push_back(pProjectile);

			m_ProjectileList.insert(std::make_pair(startTime, projectileList));
		}
		else
		{
			findIter->second.push_back(pProjectile);
		}
	}
}

void CDnSkill::RemoveProjectile(CDnProjectile* pProjectile)
{
	if (pProjectile == NULL)
		return;

	LOCAL_TIME startTime = pProjectile->GetSkillStartTime();

	SKILL_PROJECTILE_LIST::iterator findIter = m_ProjectileList.find(startTime);
	if (findIter != m_ProjectileList.end())
	{
		PROJECTILE_LIST::iterator iter = findIter->second.begin();
		for ( ; iter != findIter->second.end(); ++iter)
		{
			CDnProjectile* pTemp = *iter;
			if (pTemp == pProjectile)
			{
				iter = findIter->second.erase(iter);

				//If list is empty, remove from main list.
				if (findIter->second.empty() == true)
					m_ProjectileList.erase(findIter);
				
				break;
			}
		}
	}
	
}

int CDnSkill::GetProjectileCount(LOCAL_TIME startTime)
{
	int nListCount = 0;

	SKILL_PROJECTILE_LIST::iterator findIter = m_ProjectileList.find(startTime);
	if (findIter != m_ProjectileList.end())
		nListCount = (int)findIter->second.size();
	
	return nListCount;
}

void CDnSkill::ApplyAddtionalStateInfo()
{
	if( !m_hActor ) return;

	//#52905 [Kali] "Stack Additional Effect"
	//When a state effect is applied, if a matching conditional state effect is active, add it to the skill's state effects.
	if( m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_253) )
	{
		DNVector(DnBlowHandle) vlBlows;
		m_hActor->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_253, vlBlows);
		int nListCount = (int)vlBlows.size();

		for (int iIndex = 0; iIndex < nListCount; ++iIndex)
		{
			DnBlowHandle hBlow = vlBlows[iIndex];
			if (hBlow && hBlow->IsEnd() == false)
			{
				CDnAdditionalStateInfoBlow* pAddtionalStateInfoBlow = static_cast<CDnAdditionalStateInfoBlow*>(hBlow.GetPointer());
				if (pAddtionalStateInfoBlow)
				{
					STATE_BLOW::emBLOW_INDEX DestBlowIndex = pAddtionalStateInfoBlow->GetDestStateIndex();
					CDnSkill::StateEffectStruct addStateInfo = pAddtionalStateInfoBlow->GetTargetStateInfo();

					int iNumStateBlow = (int)m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].size();
					for( int i = 0; i < iNumStateBlow; ++i )
					{
						const StateEffectStruct& SE = m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].at( i );
						if( DestBlowIndex == SE.nID)
						{
							addStateInfo.nDurationTime = SE.nDurationTime;	//Set duration to the condition state effect duration.

							//Create the list of state effects to add.
							m_AddtionalStateInfoList.push_back(addStateInfo);

							OutputDebug("Additional state effect applied: %d, %s\n", addStateInfo.nID, addStateInfo.szValue.c_str());
						}
					}
				}
			}
		}

		//Iterate through the state effect list to add and add them to the actual skill state effect list.
		int nAddStateCount = (int)m_AddtionalStateInfoList.size();
		for (int i = 0; i < nAddStateCount; ++i)
		{
			CDnSkill::StateEffectStruct addStateEffect = m_AddtionalStateInfoList[i];
			 m_vlStateEffectList[m_iSelectedSkillLevelDataApplyType].push_back(addStateEffect);
		}
	}

	if( m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_276) )
	{
		DNVector(DnBlowHandle) vlBlows;
		m_hActor->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_276, vlBlows);
		
		for( DWORD i=0; i<vlBlows.size(); i++ )
		{
			if( vlBlows[i] )
			{
				CDnAddStateBySkillGroupBlow* pAddStateBySkillGroupBlow = static_cast<CDnAddStateBySkillGroupBlow*>(vlBlows[i].GetPointer());
				if( pAddStateBySkillGroupBlow ) pAddStateBySkillGroupBlow->ApplyAddtionalStateBlowFromSkill( GetClassID() );
			}
		}
	}

	if( m_hActor && m_hActor->GetStateBlow() )
		m_hActor->GetStateBlow()->OnBeginSkill( GetMySmartPtr() );
}

void CDnSkill::RemoveAddtionalStateInfo()
{
	//Remove extra state effects added upon skill use...
	int nListCount = (int)m_AddtionalStateInfoList.size();
	for (int iIndex = 0; iIndex < nListCount; ++iIndex)
	{
		StateEffectStruct addStateInfo = m_AddtionalStateInfoList[iIndex];

		std::vector<StateEffectStruct>::iterator iter = m_vlStateEffectList[ m_iSelectedSkillLevelDataApplyType ].begin();
		for( ; iter != m_vlStateEffectList[m_iSelectedSkillLevelDataApplyType].end(); ++iter)
		{
			StateEffectStruct SE = (*iter);

			if (SE.nID == addStateInfo.nID &&
				SE.ApplyType == addStateInfo.ApplyType &&
				/*SE.nDurationTime == addStateInfo.nDurationTime &&*/	//Duration is variable...
				SE.bAddtionalStateInfo == addStateInfo.bAddtionalStateInfo)
			{
				
				iter = m_vlStateEffectList[m_iSelectedSkillLevelDataApplyType].erase(iter);
				break;
			}			
		}
	}

	m_AddtionalStateInfoList.clear();


	if( m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_276) )
	{
		DNVector(DnBlowHandle) vlBlows;
		m_hActor->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_276, vlBlows);

		for( DWORD i=0; i<vlBlows.size(); i++ )
		{
			if( vlBlows[i] )
			{
				CDnAddStateBySkillGroupBlow* pAddStateBySkillGroupBlow = static_cast<CDnAddStateBySkillGroupBlow*>(vlBlows[i].GetPointer());
				if( pAddStateBySkillGroupBlow )	pAddStateBySkillGroupBlow->RemoveAdditionalStateBlow();
			}
		}
	}
}

#if defined(PRE_FIX_64312)
bool CDnSkill::IsSummonMonsterSkill()
{
	m_bIsSummonMonsterSkill = false;

	//If skill Checker has summon monster check, is it a skill used on summon monsters??? (designer note)
	int iNumChecker = (int)m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].size();

	for( int iChecker = 0; iChecker < iNumChecker; ++iChecker )
	{
		IDnSkillUsableChecker* pChecker = m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].at( iChecker );
		if( pChecker && pChecker->GetType() == IDnSkillUsableChecker::SUMMON_CHECKER )
		{
			m_bIsSummonMonsterSkill = true;
			break;
		}
	}

	return m_bIsSummonMonsterSkill;
}

void CDnSkill::AddSummonMonsterEnchantSkill(DnSkillHandle hSkill)
{
	m_SummonMonsterEnchantSkill = hSkill;
}

void CDnSkill::RemoveSummonMonsterEnchantSkill()
{
	m_SummonMonsterEnchantSkill.Identity();
}

bool CDnSkill::ApplySummonMonsterEnchantSkill(DnSkillHandle hSkill)
{
	m_isAppliedSummonMonsterEnchantSkill = true;
	return ApplyEnchantSkill(hSkill);
}
#endif // PRE_FIX_64312

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
float CDnSkill::GetOrigDelayTime()
{
	float fDelayTime = 0.0f;

	if( 0.0f < m_fAnotherGlobalSkillCoolTime )
		fDelayTime = m_fAnotherGlobalSkillCoolTime;
	else
		fDelayTime = m_fDelayTime[ m_iSelectedSkillLevelDataApplyType ]; 

	return fDelayTime;
}

void CDnSkill::UpdateGlobalCoolTime(float fRate)
{
	//When global cooldown is set and cooldown has already started
	if (m_iGlobalSkillGroupID > 0)
	{
		if (m_fLeftDelayTime > 0)
		{
			m_fDeltaGlobalCoolTime += fRate;

			float fOrigDelayTime = GetOrigDelayTime();

			m_fLeftDelayTime -= fOrigDelayTime * fRate;		
		}
	}
}

void CDnSkill::ResetGlobalCoolTime(float fRate)
{
	//When global cooldown is set and cooldown has already started
	if (m_iGlobalSkillGroupID > 0 )
	{
		//When global cooldown modification value is set
		if (m_fDeltaGlobalCoolTime > 0.0)
		{
			float fOrigDelayTime = GetOrigDelayTime();

			//If the applied value is smaller than the value to reset now
			if (m_fDeltaGlobalCoolTime < fRate)
				fRate = m_fDeltaGlobalCoolTime;

			//Decrease the applied value..
			m_fDeltaGlobalCoolTime -= fRate;
			if (m_fDeltaGlobalCoolTime < 0.0f)
				m_fDeltaGlobalCoolTime = 0.0f;

			//If cooldown is currently remaining
			if (m_fLeftDelayTime > 0)
				m_fLeftDelayTime += fOrigDelayTime * fRate;		
		}
	}
}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined(PRE_ADD_65808)
bool CDnSkill::IsSummonMonsterRecall(int& monsterID)
{
	//Uses PlayAniProcess and returns true if SummonMonster signal exists in that action..
	CDnPlayAniProcess* pAniProcess = static_cast<CDnPlayAniProcess*>(GetProcessor( IDnSkillProcessor::PLAY_ANI ));
	if (pAniProcess && m_hActor)
	{
		const char* szActionName = pAniProcess->GetActionName();
		if (szActionName == NULL)
			return false;

		CEtActionBase::ActionElementStruct *pStruct = m_hActor->GetElement(szActionName);
		if (pStruct)
		{
			int selectedMonsterID = -1;

			CEtActionSignal *pSignal = NULL;
			for (int i = 0; i < (int)pStruct->pVecSignalList.size(); ++i)
			{
				pSignal = pStruct->pVecSignalList[i];
				if (pSignal == NULL)
					continue;

				int signalIndex = pSignal->GetSignalIndex();
				switch(signalIndex)
				{
				case STE_SummonMonster:
					{
						SummonMonsterStruct* pSummonMonsterInfo = (SummonMonsterStruct *)pSignal->GetData();
						if (pSummonMonsterInfo)
							selectedMonsterID = pSummonMonsterInfo->MonsterID;
					}
					break;
				case STE_SummonMonsterInfo:
					{
						SummonMonsterInfoStruct* pInfo = (SummonMonsterInfoStruct*)pSignal->GetData();
						if (pInfo)
							selectedMonsterID = pInfo->MonsterID;
					}
					break;
				}

				if (selectedMonsterID != -1)
					break;
			}

			if (selectedMonsterID != -1)
			{
				monsterID = selectedMonsterID;
				return true;
			}
		}
	}

	return false;
}
#endif // PRE_ADD_65808


#if defined(_GAMESERVER)
bool CoolTimeInfo::Process(LOCAL_TIME localTime, float fDelta)
{
	switch( m_eDurationType )
	{
	case CDnSkill::DurationTypeEnum::Instantly:
	case CDnSkill::DurationTypeEnum::Buff:
	case CDnSkill::DurationTypeEnum::Debuff:
	case CDnSkill::DurationTypeEnum::SummonOnOff:
	case CDnSkill::DurationTypeEnum::StanceChange:
		{
			if( m_fLeftDelayTime == 0.0f ) return false;

			m_fLeftDelayTime -= fDelta;
			if( m_fLeftDelayTime < 0.f ) 
				m_fLeftDelayTime = 0.f;

			m_fCoolTime = ( 1.0f / (m_fDelayTime*m_fCoolTimeAdjustBlowValue) ) * m_fLeftDelayTime;
		}
		break;

	case CDnSkill::DurationTypeEnum::TimeToggle:
		break;
	case CDnSkill::DurationTypeEnum::ActiveToggle:
	case CDnSkill::DurationTypeEnum::ActiveToggleForSummon:
		break;
	case CDnSkill::DurationTypeEnum::Aura:
		break;
	}

	return (m_fCoolTime != 0.0f && m_fLeftDelayTime > 0.0f);
}

void CoolTimeInfo::OnBeginCoolTime()
{
	m_fCoolTime = (m_fDelayTime == 0.f) ? 0.0f : 1.0f;
	m_fCoolTimeAdjustBlowValue = 1.0f;

	m_fLeftDelayTime = m_fDelayTime * m_fCoolTimeAdjustBlowValue;
}

void CoolTimeInfo::SetInfo(DnSkillHandle &hSkill)
{
	m_nSkillID = hSkill->GetClassID();
	m_hActor = hSkill->GetActor();
	m_eDurationType = hSkill->GetDurationType();

	m_fCoolTimeAdjustBlowValue = 1.0f;

	m_fDelayTime = hSkill->GetDelayTime();
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CoolTimeInfo::SetInfo(CDnPrefixSkill* pPrefixSkill)
{
	m_nSkillID = pPrefixSkill->GetSkillType();

	DnSkillHandle hSkill = pPrefixSkill->GetSkillHandle();
	if (hSkill)
	{
		m_hActor = hSkill->GetActor();
		m_eDurationType = hSkill->GetDurationType();

		m_fCoolTimeAdjustBlowValue = 1.0f;

		m_fDelayTime = hSkill->GetDelayTime();
	}
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW


bool CoolTimeManager::IsCoolTime(int nSkillID)
{
	COOLTIME_LIST::iterator findIter = m_CoolTimeList.find(nSkillID);

	//If still registered, cooldown has not ended yet??
	return (findIter != m_CoolTimeList.end());
}

void CoolTimeManager::AddCoolTime(DnSkillHandle hSkill)
{
	if (!hSkill)
		return;

	int nSkillID = hSkill->GetClassID();
	COOLTIME_LIST::iterator findIter = m_CoolTimeList.find(nSkillID);
	//Register only when the same skill ID does not exist.
	if (findIter == m_CoolTimeList.end())
	{
		CoolTimeInfo *pCoolTimeInfo = new CoolTimeInfo();
		pCoolTimeInfo->SetInfo(hSkill);
		pCoolTimeInfo->OnBeginCoolTime();

		m_CoolTimeList.insert(COOLTIME_LIST::value_type(nSkillID, pCoolTimeInfo));
	}
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CoolTimeManager::AddCoolTime(CDnPrefixSkill* pPrefixSkill)
{
	if (!pPrefixSkill)
		return;

	int nSkillType = pPrefixSkill->GetSkillType();
	COOLTIME_LIST::iterator findIter = m_CoolTimeList.find(nSkillType);
	//Register only when the same skill ID does not exist.
	if (findIter == m_CoolTimeList.end())
	{
		CoolTimeInfo *pCoolTimeInfo = new CoolTimeInfo();
		pCoolTimeInfo->SetInfo(pPrefixSkill);
		pCoolTimeInfo->OnBeginCoolTime();

		m_CoolTimeList.insert(COOLTIME_LIST::value_type(nSkillType, pCoolTimeInfo));
	}
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

void CoolTimeManager::RemoveCoolTime(int nSkillID)
{
	COOLTIME_LIST::iterator findIter = m_CoolTimeList.find(nSkillID);
	if (findIter != m_CoolTimeList.end())
	{
		CoolTimeInfo* pCoolTimeInfo = findIter->second;
		if (pCoolTimeInfo)
			delete pCoolTimeInfo;

		m_CoolTimeList.erase(findIter);
	}
}

void CoolTimeManager::Process(LOCAL_TIME localTime, float fDelta)
{
	COOLTIME_LIST::iterator iter = m_CoolTimeList.begin();
	COOLTIME_LIST::iterator endIter = m_CoolTimeList.end();

	std::list<int> removeCoolTimeList;

	for (; iter != endIter; ++iter)
	{
		if (!iter->second->Process(localTime, fDelta))
			removeCoolTimeList.push_back(iter->first);			
	}

	if (!removeCoolTimeList.empty())
	{
		std::list<int>::iterator iter = removeCoolTimeList.begin();
		std::list<int>::iterator endIter = removeCoolTimeList.end();

		for (; iter != endIter; ++iter)
		{
			RemoveCoolTime((*iter));
		}

		removeCoolTimeList.clear();
	}
}

void CoolTimeManager::InitList()
{
	COOLTIME_LIST::iterator iter = m_CoolTimeList.begin();
	COOLTIME_LIST::iterator endIter = m_CoolTimeList.end();

	for (; iter != endIter; ++iter)
	{
		CoolTimeInfo* pCoolTimeInfo = iter->second;
		if (pCoolTimeInfo)
			delete pCoolTimeInfo;
	}

	m_CoolTimeList.clear();
}
#endif // _GAMESERVER


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)

CDnPrefixSkill::CDnPrefixSkill(int nPrefixType)
{
	m_nPrefixType = nPrefixType;

	m_fProbability = 0.0f;
}

CDnPrefixSkill::~CDnPrefixSkill()
{

}

void CDnPrefixSkill::SetSkillHandle(DnSkillHandle hSkill)
{
	m_hSkill = hSkill;
}

void CDnPrefixSkill::UpdateCandidateSkill()
{
	std::list<DnSkillHandle>::iterator iter = m_SkillList.begin();
	std::list<DnSkillHandle>::iterator endIter = m_SkillList.end();

	float fDelayTime = FLT_MAX;

	DnSkillHandle hSelectedSkill;
	for (; iter != endIter; ++iter)
	{
		DnSkillHandle hSkill = *iter;
		if (!hSkill)
			continue;

		float fSkillDelayTime = hSkill->GetDelayTime();
		if (fSkillDelayTime < fDelayTime)
		{
			hSelectedSkill = hSkill;
			fDelayTime = fSkillDelayTime;
		}
	}

	if (hSelectedSkill)
		SetSkillHandle(hSelectedSkill);
}

void CDnPrefixSkill::AddSkill(DnSkillHandle hSkill)
{
	if (!hSkill) return;

	m_SkillList.push_back(hSkill);

	//Get the probability value of the skill.
	CDnProbabilityChecker* pProbabilityChecker = static_cast<CDnProbabilityChecker*>(hSkill->GetChecker(IDnSkillUsableChecker::PROB_CHECKER));
	m_fProbability += pProbabilityChecker ? pProbabilityChecker->GetProbability() : 0.0f;

	UpdateCandidateSkill();

	AddStateEffectInfo(hSkill);
}

void CDnPrefixSkill::RemoveSkill(DnSkillHandle hSkill)
{
	std::list<DnSkillHandle>::iterator iter = m_SkillList.begin();
	std::list<DnSkillHandle>::iterator endIter = m_SkillList.end();

	for (; iter != endIter; ++iter)
	{
		if ((*iter) == hSkill)
		{
			//Get the probability value of the skill.
			CDnProbabilityChecker* pProbabilityChecker = static_cast<CDnProbabilityChecker*>(hSkill->GetChecker(IDnSkillUsableChecker::PROB_CHECKER));
			m_fProbability -= pProbabilityChecker ? pProbabilityChecker->GetProbability() : 0.0f;
			m_SkillList.erase(iter);
			break;
		}
	}

	UpdateCandidateSkill();

	//When a skill is removed, iterate through the skill list to reapply state effects...
	UpdateStateEffects();
}

void CDnPrefixSkill::UpdateStateEffects()
{
	//1. Remove existing state effect setting values.
	m_vlStateEffectList.clear();

	//Iterate through the skill list and apply state effect values.
	std::list<DnSkillHandle>::iterator iter = m_SkillList.begin();
	std::list<DnSkillHandle>::iterator endIter = m_SkillList.end();

	for (; iter != endIter; ++iter)
	{
		DnSkillHandle hSkill = (*iter);
		if (hSkill)
		{
			AddStateEffectInfo(hSkill);
		}
	}
}

void CDnPrefixSkill::AddStateEffectInfo(DnSkillHandle hSkill)
{
	if (m_vlStateEffectList.empty())
	{
		//Add hSkill's state effect settings directly to the list.
		int nStateEffectCount = hSkill->GetStateEffectCount();
		for (int i = 0; i < nStateEffectCount; ++i)
		{
			CDnSkill::StateEffectStruct* pAddStateEffect = hSkill->GetStateEffectFromIndex(i);

			if (pAddStateEffect)
				m_vlStateEffectList.push_back(*pAddStateEffect);
		}
	}
	else
	{
		//Cannot add if the state effect count does not match.
		if (m_vlStateEffectList.size() != hSkill->GetStateEffectCount())
			return;

		switch(m_nPrefixType)
		{
		case Prefix_000: //of Destruction
			AddStateEffectInfo_Prefix_000(hSkill);
			break;
		case Prefix_001: //Magical
			AddStateEffectInfo_Prefix_001(hSkill);
			break;
		case Prefix_002: //Bear weapon
			AddStateEffectInfo_Prefix_002(hSkill);
			break;
		case Prefix_003: //Weapon of Wind
			AddStateEffectInfo_Prefix_003(hSkill);
			break;
		case Prefix_004: //Weapon of Wisdom
			AddStateEffectInfo_Prefix_004(hSkill);
			break;
		case Prefix_005: //Harsh
			AddStateEffectInfo_Prefix_005(hSkill);
			break;
		case Prefix_006: //Weapon of Darkness
			AddStateEffectInfo_Prefix_006(hSkill);
			break;
		case Prefix_007: //Weapon of Life
			AddStateEffectInfo_Prefix_007(hSkill);
			break;
		case Prefix_008: //Weapon of Mana
			AddStateEffectInfo_Prefix_008(hSkill);
			break;
		case Prefix_009: //Of vitality
			AddStateEffectInfo_Prefix_009(hSkill);
			break;
		case Prefix_010: //Critical
			AddStateEffectInfo_Prefix_010(hSkill);
			break;
		case Prefix_011: //Of Restraint
			AddStateEffectInfo_Prefix_011(hSkill);
			break;
		case Prefix_012: //Of shock
			AddStateEffectInfo_Prefix_012(hSkill);
			break;
		case Prefix_013: //Hero's
			AddStateEffectInfo_Prefix_013(hSkill);
			break;
		case Prefix_014: //Knight's
			AddStateEffectInfo_Prefix_014(hSkill);
			break;
		case Prefix_015: //Iron Wall weapon
			AddStateEffectInfo_Prefix_015(hSkill);
			break;
		case Prefix_016: //Veil weapon
			AddStateEffectInfo_Prefix_016(hSkill);
			break;
		case Prefix_017: //Fire weapon
			AddStateEffectInfo_Prefix_017(hSkill);
			break;
		case Prefix_018: //Weapon of Water
			AddStateEffectInfo_Prefix_018(hSkill);
			break;
		case Prefix_019: //Light weapon
			AddStateEffectInfo_Prefix_019(hSkill);
			break;
		case Prefix_020: //Iron Wall armor
			AddStateEffectInfo_Prefix_020(hSkill);
			break;
		case Prefix_021: //Veil armor
			AddStateEffectInfo_Prefix_021(hSkill);
			break;
		case Prefix_022: //Bear armor
			AddStateEffectInfo_Prefix_022(hSkill);
			break;
		case Prefix_023: //Armor of Wind
			AddStateEffectInfo_Prefix_023(hSkill);
			break;
		case Prefix_024: //Armor of Wisdom
			AddStateEffectInfo_Prefix_024(hSkill);
			break;
		case Prefix_025: //Healthy
			AddStateEffectInfo_Prefix_025(hSkill);
			break;
		case Prefix_026: //Lucky
			AddStateEffectInfo_Prefix_026(hSkill);
			break;
		case Prefix_027: //Armor of Life
			AddStateEffectInfo_Prefix_027(hSkill);
			break;
		case Prefix_028: //Armor of Mana
			AddStateEffectInfo_Prefix_028(hSkill);
			break;
// 		case Prefix_029: // Sturdy
// 			AddStateEffectInfo_Prefix_029(hSkill);
// 			break;
		case Prefix_030: //Flexible
			AddStateEffectInfo_Prefix_030(hSkill);
			break;
		case Prefix_031: //Of will
			AddStateEffectInfo_Prefix_031(hSkill);
			break;
// 		case Prefix_032: // Tough
// 			AddStateEffectInfo_Prefix_032(hSkill);
// 			break;
// 		case Prefix_033: // Indomitable
// 			AddStateEffectInfo_Prefix_033(hSkill);
// 			break;
		case Prefix_034: //Fire armor
			AddStateEffectInfo_Prefix_034(hSkill);
			break;
		case Prefix_035: //Armor of Water
			AddStateEffectInfo_Prefix_035(hSkill);
			break;
		case Prefix_036: //Light armor
			AddStateEffectInfo_Prefix_036(hSkill);
			break;
		case Prefix_037: //Armor of Darkness
			AddStateEffectInfo_Prefix_037(hSkill);
			break;

		default:
			break;

		}		
	}
}

void CDnPrefixSkill::AddStateEffectInfo( STATE_BLOW::emBLOW_INDEX blowList[], int nCount, DnSkillHandle hSkill )
{
	if (!hSkill)
		return;

	int nAddStateEffectCount = hSkill->GetStateEffectCount();

	//Number of state effects is 4
	if (nAddStateEffectCount != nCount)
	{
		OutputDebug("Prefix skill Type[%d] state effect count mismatch!\n", m_nPrefixType);
		return;
	}

	for (int i = 0; i < nCount; ++i)
	{
		CDnSkill::StateEffectStruct* pAddStateEffect = hSkill->GetStateEffectFromIndex(i);
		CDnSkill::StateEffectStruct* pOrigStateEffect = &m_vlStateEffectList[i];

		//It is invalid if the state effect Index is not BLOW_213.
		if ((STATE_BLOW::emBLOW_INDEX)pOrigStateEffect->nID != blowList[i])
		{
			OutputDebug("State effect Index mismatch: expected %d, got %d!\n", blowList[i], pOrigStateEffect->nID);
			continue;
		}

		//If the two state effect indices differ..
		if (pAddStateEffect->nID != pOrigStateEffect->nID)
		{
			OutputDebug("State effect Index mismatch: Orig[%d], Add[%d]!\n", pOrigStateEffect->nID, pAddStateEffect->nID);
			continue;
		}

		std::string szNewValue = "";
		CDnCreateBlow::AddStateEffectValue((STATE_BLOW::emBLOW_INDEX)pOrigStateEffect->nID, pOrigStateEffect->szValue.c_str(), pAddStateEffect->szValue.c_str(), szNewValue);
		
		OutputDebug("Original state effect value(%s), added value(%s), final result(%s)\n", pOrigStateEffect->szValue.c_str(), pAddStateEffect->szValue.c_str(), szNewValue.c_str());

		pOrigStateEffect->szValue = szNewValue;
		//Change duration to a large value.
		pOrigStateEffect->nDurationTime = max(pOrigStateEffect->nDurationTime, pAddStateEffect->nDurationTime);
	}
}


void CDnPrefixSkill::AddStateEffectInfo_Prefix_000(DnSkillHandle hSkill) //of Destruction
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_213,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}

void CDnPrefixSkill::AddStateEffectInfo_Prefix_001(DnSkillHandle hSkill) //Magical
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_214,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}

void CDnPrefixSkill::AddStateEffectInfo_Prefix_002(DnSkillHandle hSkill) //Bear weapon
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_221,
		STATE_BLOW::BLOW_124,
		STATE_BLOW::BLOW_128,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}

void CDnPrefixSkill::AddStateEffectInfo_Prefix_003(DnSkillHandle hSkill) //Weapon of Wind
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_221,
		STATE_BLOW::BLOW_126,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_004(DnSkillHandle hSkill) //Weapon of Wisdom
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_214,
		STATE_BLOW::BLOW_036,
		STATE_BLOW::BLOW_037,
		STATE_BLOW::BLOW_038,
		STATE_BLOW::BLOW_039,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_005(DnSkillHandle hSkill) //Harsh
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_157,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_006(DnSkillHandle hSkill) //Weapon of Darkness
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_035,
		STATE_BLOW::BLOW_182,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_007(DnSkillHandle hSkill) //Weapon of Life
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_016,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_008(DnSkillHandle hSkill) //Weapon of Mana
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_018,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_009(DnSkillHandle hSkill) //Of vitality
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_014,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_010(DnSkillHandle hSkill) //Critical
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_158,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_011(DnSkillHandle hSkill) //Of Restraint
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_070,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_012(DnSkillHandle hSkill) //Of shock
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_221,
		STATE_BLOW::BLOW_175,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_013(DnSkillHandle hSkill) //Hero's
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_079,
	};
	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_014(DnSkillHandle hSkill) //Knight's
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_076,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_015(DnSkillHandle hSkill) //Iron Wall weapon
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_004,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_016(DnSkillHandle hSkill) //Veil weapon
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_094,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_017(DnSkillHandle hSkill) //Fire weapon
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_032,
		STATE_BLOW::BLOW_182,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_018(DnSkillHandle hSkill) //Weapon of Water
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_033,
		STATE_BLOW::BLOW_182,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_019(DnSkillHandle hSkill) //Light weapon
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_034,
		STATE_BLOW::BLOW_182,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_020(DnSkillHandle hSkill) //Iron Wall armor
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_134,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);;
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_021(DnSkillHandle hSkill) //Veil armor
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_135,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_022(DnSkillHandle hSkill) //Bear armor
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_124,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_023(DnSkillHandle hSkill) //Armor of Wind
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_126,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_024(DnSkillHandle hSkill) //Armor of Wisdom
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_036,
		STATE_BLOW::BLOW_037,
		STATE_BLOW::BLOW_038,
		STATE_BLOW::BLOW_039,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_025(DnSkillHandle hSkill) //Healthy
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_124,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_026(DnSkillHandle hSkill) //Lucky
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_111,
		STATE_BLOW::BLOW_112,
		STATE_BLOW::BLOW_113,
		STATE_BLOW::BLOW_114,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_027(DnSkillHandle hSkill) //Armor of Life
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_016,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_028(DnSkillHandle hSkill) //Armor of Mana
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_018,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_029(DnSkillHandle hSkill) //Sturdy
{
// 	STATE_BLOW::emBLOW_INDEX blowList[] = 
// 	{
// 	};
// 
// 	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);
// 
// 	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_030(DnSkillHandle hSkill) //Flexible
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_076,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_031(DnSkillHandle hSkill) //Of will
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_156,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_032(DnSkillHandle hSkill) //Sturdy
{
// 	STATE_BLOW::emBLOW_INDEX blowList[] = 
// 	{
// 	};
// 
// 	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);
// 
// 	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_033(DnSkillHandle hSkill) //Indomitable
{
// 	STATE_BLOW::emBLOW_INDEX blowList[] = 
// 	{
// 	};
// 
// 	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);
// 
// 	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_034(DnSkillHandle hSkill) //Fire armor
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_036,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_035(DnSkillHandle hSkill) //Armor of Water
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_037,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_036(DnSkillHandle hSkill) //Light armor
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_038,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
void CDnPrefixSkill::AddStateEffectInfo_Prefix_037(DnSkillHandle hSkill) //Armor of Darkness
{
	STATE_BLOW::emBLOW_INDEX blowList[] = 
	{
		STATE_BLOW::BLOW_039,
	};

	int nCount = sizeof(blowList) / sizeof(STATE_BLOW::emBLOW_INDEX);

	AddStateEffectInfo(blowList, nCount, hSkill);
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

void CDnSkill::OnAnotherGlobalSkillBeginCoolTime( DnSkillHandle hSkill )
{
	if( hSkill )
	{
		m_fAnotherGlobalSkillCoolTime = hSkill->GetDelayTime();

		m_nAnotherGlobakSkillID = hSkill->GetClassID();
	}
}

#include "DnCoolTimeChargingProcess.h"
void CDnSkill::SetAdditionalCoolTimeChargeCount( int nAddCount )
{
	IDnSkillProcessor* pProc = GetProcessor( IDnSkillProcessor::COOLTIME_CHARGING );
	if( pProc )
	{
		CDnCoolTimeChargingProcess* pChargeProc = static_cast<CDnCoolTimeChargingProcess*>( pProc );
		int nNewMax = pChargeProc->GetMaxChargeCount() + nAddCount;
		m_nMaxCoolTimeChargeCount = nNewMax;
		if( m_nCurCoolTimeChargeCount > m_nMaxCoolTimeChargeCount )
			m_nCurCoolTimeChargeCount = m_nMaxCoolTimeChargeCount;
		if( m_nMaxCoolTimeChargeCount > 1 )
			m_bUseCoolTimeCharging = true;
	}
}