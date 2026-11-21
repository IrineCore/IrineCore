
#include "stdafx.h"
#include "MAAiChecker.h"
#include "DnActor.h"
#include "MAAiScript.h"
#include "DnBlow.h"
#include "DNAggroSystem.h"
#include "DnMonsterActor.h"
#include "DnStateBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

/*#####################################################################################################################
	ActorChecker
#####################################################################################################################*/


// Weapon check
// return true if weapon exists, return false if no weapon;

bool CMAAiActorWeaponChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor )
		return false;

	return hActor->GetWeapon(m_uiWeaponIndex) ? true : false;
}

// Check part destruction
// If the part is destroyed return true; otherwise return false;
bool CMAAiActorPartsDestroyChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor )
		return false;

	for( UINT i=0 ; i<m_vPartsTableID.size() ; ++i )
	{
		if( !hActor->bIsDestroyParts(m_vPartsTableID[i]) )
			return false;

	}

	return true;
}

// Check part existence
// If the part exists return true; otherwise return false;
bool CMAAiActorPartsExistChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor )
		return false;

	for( UINT i=0 ; i<m_vPartsTableID.size() ; ++i )
	{
		if( hActor->bIsDestroyParts(m_vPartsTableID[i]) )
			return false;

	}

	return true;
}

// Check Fly state
bool CMAAiActorFlyStateChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	return (hActor->IsFly()&&!hActor->IsFly2());
}

bool CMAAiActorFly2StateChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	return hActor->IsFly2();
}

// Check Ground state
bool CMAAiActorGroundStateChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	return hActor->IsGround();
}

// Check UnderGround state
bool CMAAiActorUnderGroundStateChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	return hActor->IsUnderGround();
}

// Check HPPercent
bool CMAAiActorHPPercentChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	return m_iHPPercent >= hActor->GetHPPercent();
}

// Check HPPercent range (Min <= val <= Max)
bool CMAAiActorHPPercentCheckerRange::bIsCheck( DnActorHandle hActor, int iParam )
{
	int iHPPercent = hActor->GetHPPercent();
	return (iHPPercent >= m_iHPMinPercent && iHPPercent <= m_iHPMaxPercent);
}

// Check SP
bool CMAAiActorSPChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	return m_iSP <= hActor->GetSP();
}

// Check skills currently in use
bool CMAAiActorUsedSkillChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor->GetStateBlow() ) return false;

	std::vector<UINT> vCheckTableID = m_vSkillTableID;

	for( int i=0 ; i<hActor->GetStateBlow()->GetNumStateBlow() ; ++i )
	{
		DnBlowHandle hBlow = hActor->GetStateBlow()->GetStateBlow(i);
		if( !hBlow ) continue;

		const CDnSkill::SkillInfo* pSkillinfo = hBlow->GetParentSkillInfo();
		if ( pSkillinfo )
		{
			vCheckTableID.erase( remove( vCheckTableID.begin(), vCheckTableID.end(), pSkillinfo->iSkillID), vCheckTableID.end() );
			if( vCheckTableID.empty() )
				return true;
		}
	}

	return false;
}

// Check skills not currently in use
bool CMAAiActorNotUsedSkillChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor->GetStateBlow() ) return true;
	for( int i=0 ; i<hActor->GetStateBlow()->GetNumStateBlow() ; ++i )
	{
		DnBlowHandle hBlow = hActor->GetStateBlow()->GetStateBlow(i);
		if( !hBlow ) continue;

		const CDnSkill::SkillInfo* pSkillinfo = hBlow->GetParentSkillInfo();
		if ( pSkillinfo )
		{
			if( std::find( m_vSkillTableID.begin(), m_vSkillTableID.end(), pSkillinfo->iSkillID ) != m_vSkillTableID.end() )
				return false;
		}
	}

	return true;
}

// rate
bool CMAAiActorSkillRateChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( m_iRate < (_rand(hActor->GetRoom())%100) )
		return false;

	return true;
}

// LimitCount
bool CMAAiActorSkillLimitCountChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( hActor->IsMonsterActor() )
	{
		CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
		if( pMonsterActor->GetUseSkillCount( iParam ) < m_iLimitCount )
			return true;
	}
	return false;
}

bool CMAAiActorEncounterTickChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor || !hActor->GetAggroSystem() )
		return false;

	bool bIsProvocationTarget;
	DnActorHandle hTarget = hActor->GetAggroSystem()->OnGetAggroTarget( bIsProvocationTarget );
	if( !hTarget )
		return false;

	if( static_cast<int>(timeGetTime()-hActor->GetGenTick()) >= m_iCheckTick )
		return true;

	return false;
}

bool CMAAiActorLimitTickChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor )
		return false;

	if( static_cast<int>(timeGetTime()-hActor->GetGenTick()) >= m_iCheckTick )
		return false;

	return true;
}

bool CMAAiActorNotifyAnnounceChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor || !hActor->IsMonsterActor() )
		return false;

	CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());

	bool bNotify = pMonster->GetAIBase()->bIsNotifyDieAnnounce( m_iTickGap );
	if( !bNotify )
		return false;
	
	pMonster->GetAIBase()->ResetNotifyDieAnnounce();

	if( m_iRate > (_rand(hActor->GetRoom())%100) )
		return true;

	return false;
}

bool CMAAiActorGlobalCoolTimeChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor || !hActor->IsMonsterActor() )
		return false;

	CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());

	MAAiScript* pScript = static_cast<MAAiScript*>(pMonster->GetAIBase());
	if( m_iCoolTimeIndex >= static_cast<int>(pScript->GetScriptData().m_AIGlobalCoolTime.size()) )
	{
		_ASSERT(0);
		return false;
	}

	DWORD dwRet = pScript->m_GlobalCoolTime.Update( m_iCoolTimeIndex, timeGetTime() );
	return ( dwRet == 0 );
}

bool CMAAiActorNoAggroChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor || !hActor->IsMonsterActor() )
		return false;

	CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());

	DnActorHandle hTarget = pMonster->GetAIBase()->GetTarget();

	return hTarget ? false : true;
}

bool CMAAiActorWaitOrderChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( !hActor || !hActor->IsMonsterActor() )
		return false;

	CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());

	MAAiScript* pScript = static_cast<MAAiScript*>(pMonster->GetAIBase());
	int iSkillIndex = pScript->GetMonsterSkillAI()->GetSkillIndexFromLuaSkillIndex( iParam );

	return pMonster->GetWaitOrderCount( iSkillIndex ) > 0;
}

// SkillToggle
bool CMAAiActorSkillToggleChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	if( hActor->IsMonsterActor() == false )
		return false;

	CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());
	DnActorHandle hSummoner = pMonster->GetSummonerPlayerActor();
	if( !hSummoner )
		return false;

	MAAiScript* pScript = static_cast<MAAiScript*>(pMonster->GetAIBase());
	int iSkillIndex = pScript->GetMonsterSkillAI()->GetSkillIndexFromLuaSkillIndex( iParam );
	DnSkillHandle hMonsterSkill = pMonster->FindSkill( iSkillIndex );
	if( !hMonsterSkill )
		return false;

	DnSkillHandle hSkill = hSummoner->FindSkill( m_iSkillID );
	if( !hSkill )
		return false;


	if( hSkill->GetDurationType() == CDnSkill::ActiveToggleForSummon && hSkill->IsToggleOn() == true && hSkill->GetLevel() == m_iSkillLevel )
	{
		if( hSummoner->GetSP() >= hSkill->GetDecreaseMP() )
		{
#if defined( PRE_ADD_ACADEMIC )
			hMonsterSkill->SetSummonerDecreaseSP( m_iSkillID, hSkill->GetDecreaseMP() );
#endif // #if defined( PRE_ADD_ACADEMIC )
			return true;
		}
	}

	return false;
}

// PassiveSkill
bool CMAAiActorPassiveSkillChecker::bIsCheck( DnActorHandle hActor, int iParam )
{
	CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());
	DnActorHandle hSummoner = pMonster->GetSummonerPlayerActor();
	if( !hSummoner )
		return false;

	DnSkillHandle hSkill = hSummoner->FindSkill( m_iSkillID );
	if( !hSkill )
		return false;

	if( (hSkill->GetSkillType() == CDnSkill::Passive || hSkill->GetSkillType() == CDnSkill::EnchantPassive) && hSkill->GetLevel() == m_iSkillLevel )
		return true;

	return false;
}

/*#####################################################################################################################
	TargetActorChecker
#####################################################################################################################*/

int CMAAiTargetActorChecker::GetTargetDegree( DnActorHandle hActor, DnActorHandle hTarget, const EtVector3& AxisVector )
{
	EtVector3 vec = *hActor->GetPosition() - *hTarget->GetPosition();
	vec.y = 0.f;
	EtVec3Normalize( &vec, &vec );

	float fDot = EtVec3Dot( &vec, &AxisVector );

	return static_cast<int>(EtToDegree( acosf( min( fDot, 1.0f) ) ));
}

// LF
bool CMAAi_LF_DirectionChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	int iDegree		= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vXAxis) );
	int iDegree2	= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vZAxis) );
	if( iDegree >= 135 && iDegree2 <= 90 )
		return true;
	return false;
}

// LB
bool CMAAi_LB_DirectionChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	int iDegree		= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vXAxis) );
	int iDegree2	= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vZAxis) );
	if( iDegree >= 135 && iDegree2 >= 90 )
		return true;
	return false;
}

// FL
bool CMAAi_FL_DirectionChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	int iDegree		= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vZAxis) );
	int iDegree2	= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vXAxis) );
	if( iDegree <= 45 && iDegree2 >= 90 )
		return true;
	return false;
}

// FR
bool CMAAi_FR_DirectionChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	int iDegree		= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vZAxis) );
	int iDegree2	= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vXAxis) );
	if( iDegree <= 45 && iDegree2 <= 90 )
		return true;
	return false;
}

// RF
bool CMAAi_RF_DirectionChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	int iDegree		= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vXAxis) );
	int iDegree2	= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vZAxis) );
	if( iDegree <= 45 && iDegree2 <= 90 )
		return true;
	return false;
}

// RB
bool CMAAi_RB_DirectionChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	int iDegree		= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vXAxis) );
	int iDegree2	= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vZAxis) );
	if( iDegree <= 45 && iDegree2 >= 90 )
		return true;
	return false;
}

// BR
bool CMAAi_BR_DirectionChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	int iDegree		= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vZAxis) );
	int iDegree2	= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vXAxis) );
	if( iDegree >= 135 && iDegree2 <= 90 )
		return true;
	return false;
}

// BL
bool CMAAi_BL_DirectionChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	int iDegree		= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vZAxis) );
	int iDegree2	= GetTargetDegree( hActor, hTarget, -(hActor->GetMatEx()->m_vXAxis) );
	if( iDegree >= 135 && iDegree2 >= 90 )
		return true;
	return false;
}

// TargetHPPercent
bool CMAAiTargetActorHPPercentChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	return m_iHPPercent >= hTarget->GetHPPercent();
}

// TargetBuff
bool CMAAiTargetActorBuffChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	for( int i=0 ; i<hTarget->GetNumAppliedStateBlow() ; ++i )
	{
		DnBlowHandle hBlow = hTarget->GetAppliedStateBlow( i );
		if( hBlow )
		{
			const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
			if( !pSkillInfo )
				continue;

			if( pSkillInfo->eDurationType == CDnSkill::DurationTypeEnum::Buff )
				return true;
		}
	}

	return false;
}

// TargetDeBuff
bool CMAAiTargetActorDeBuffChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	for( int i=0 ; i<hTarget->GetNumAppliedStateBlow() ; ++i )
	{
		DnBlowHandle hBlow = hTarget->GetAppliedStateBlow( i );
		if( hBlow )
		{
			const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
			if( !pSkillInfo )
				continue;

			if( pSkillInfo->eDurationType == CDnSkill::DurationTypeEnum::Debuff )
				return true;
		}
	}

	return false;
}

// Blow
bool CMAAiTargetActorBlowChecker::bIsCheck( DnActorHandle hActor, DnActorHandle hTarget )
{
	for( UINT i=0 ; i<m_vTableID.size() ; ++i )
	{
		// If m_bIsNoBlowCheck == true => do not use skill if target has the status effect.
		// If m_bIsNoBlowCheck == false => do not use skill unless target has the status effect.
		if( hTarget->IsAppliedThisStateBlow( static_cast<STATE_BLOW::emBLOW_INDEX>(m_vTableID[i]) ) == m_bIsNoBlowCheck )
			return false;
	}

	return true;
}

/*#####################################################################################################################
	CheckerManager
#####################################################################################################################*/

CMAAiCheckerManager::~CMAAiCheckerManager()
{
	SAFE_DELETE_PVEC( m_vTargetActorChecker );
	SAFE_DELETE_PVEC( m_vTargetActorDirectionChecker );
	SAFE_DELETE_PVEC( m_vActorChecker );
}

bool CMAAiCheckerManager::bIsActorChecker( DnActorHandle hActor, int iParam/*=0*/ ) const
{
	for( UINT i=0 ; i<m_vActorChecker.size() ; ++i )
	{
		if( m_bIgnoreRateChecker )
		{
			if( m_vActorChecker[i]->bIsRateChecker() )
				continue;
		}

		if( !m_vActorChecker[i]->bIsCheck( hActor, iParam ) )
			return false;
	}
	return true;
}

bool CMAAiCheckerManager::bIsTargetActorChecker( DnActorHandle hActor, DnActorHandle hTarget ) const
{
	if( !hTarget )
		return false;
	
	// Check TargetDirection
	if( !m_vTargetActorDirectionChecker.empty() )
	{
		bool bOK = false;
		for( UINT i=0 ; i<m_vTargetActorDirectionChecker.size() ; ++i )
		{
			if( m_vTargetActorDirectionChecker[i]->bIsCheck( hActor, hTarget ) )
			{
				bOK = true;
				break;
			}
		}

		if( !bOK )
			return false;
	}

	// Check Target
	for( UINT i=0 ; i<m_vTargetActorChecker.size() ; ++i )
	{
		if( !m_vTargetActorChecker[i]->bIsCheck( hActor, hTarget ) )
			return false;
	}

	return true;
}

bool CMAAiCheckerManager::bLoadChecker( lua_tinker::table& t )
{
	// Weapon check
	switch( t.get<int>("checkweapon") )
	{
		case 1:
		{
			m_vActorChecker.push_back( new CMAAiActorWeaponChecker( 0 ) );
			break;
		}
		case 2:
		{
			m_vActorChecker.push_back( new CMAAiActorWeaponChecker( 1 ) );
			break;
		}
	}

	// Part destruction
	const char* pszDestroyParts = t.get<const char*>("destroyparts" );
	if( pszDestroyParts )
	{
		std::string					strString(pszDestroyParts);
		std::vector<std::string>	vSplit;
		std::vector<UINT>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( !vTableID.empty() )
			m_vActorChecker.push_back( new CMAAiActorPartsDestroyChecker(vTableID) );
	}

	// Part exists
	const char* pszExistParts = t.get<const char*>("existparts" );
	if( pszExistParts )
	{
		std::string					strString(pszExistParts);
		std::vector<std::string>	vSplit;
		std::vector<UINT>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( !vTableID.empty() )
			m_vActorChecker.push_back( new CMAAiActorPartsExistChecker(vTableID) );
	}

	// Target direction
	const char* pszTD = t.get<const char*>("td");
	if( pszTD )
	{
		std::string					strString(pszTD);
		std::vector<std::string>	vSplit;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
		{
			if( strcmp( vSplit[i].c_str(), "LF" ) == 0 )
				m_vTargetActorDirectionChecker.push_back( new CMAAi_LF_DirectionChecker() );
			else if( strcmp( vSplit[i].c_str(), "FL" ) == 0 )
				m_vTargetActorDirectionChecker.push_back( new CMAAi_FL_DirectionChecker() );
			else if( strcmp( vSplit[i].c_str(), "FR" ) == 0 )
				m_vTargetActorDirectionChecker.push_back( new CMAAi_FR_DirectionChecker() );
			else if( strcmp( vSplit[i].c_str(), "RF" ) == 0 )
				m_vTargetActorDirectionChecker.push_back( new CMAAi_RF_DirectionChecker() );
			else if( strcmp( vSplit[i].c_str(), "RB" ) == 0 )
				m_vTargetActorDirectionChecker.push_back( new CMAAi_RB_DirectionChecker() );
			else if( strcmp( vSplit[i].c_str(), "BR" ) == 0 )
				m_vTargetActorDirectionChecker.push_back( new CMAAi_BR_DirectionChecker() );
			else if( strcmp( vSplit[i].c_str(), "BL" ) == 0 )
				m_vTargetActorDirectionChecker.push_back( new CMAAi_BL_DirectionChecker() );
			else if( strcmp( vSplit[i].c_str(), "LB" ) == 0 )
				m_vTargetActorDirectionChecker.push_back( new CMAAi_LB_DirectionChecker() );
		}
	}

	// State
	for( int i=1 ; i<=3 ; ++i )
	{
		char szBuf[MAX_PATH];
		sprintf_s( szBuf, "custom_state%d", i );

		const char* pszState = t.get<const char*>(szBuf);
		if( pszState )
		{
			if( stricmp( pszState, "custom_fly" ) == 0 )
				m_vActorChecker.push_back( new CMAAiActorFlyStateChecker() );
			else if( stricmp( pszState, "custom_ground" ) == 0 )
				m_vActorChecker.push_back( new CMAAiActorGroundStateChecker() );
			else if( stricmp( pszState, "custom_underground" ) == 0 )
				m_vActorChecker.push_back( new CMAAiActorUnderGroundStateChecker() );
			else if( stricmp( pszState, "custom_fly2" ) == 0 )
				m_vActorChecker.push_back( new CMAAiActorFly2StateChecker() );
		}
	}

	// selfhppercent
	int iHPPercent = t.get<int>("selfhppercent");
	if( iHPPercent > 0 )	// Create Checker if entered
		m_vActorChecker.push_back( new CMAAiActorHPPercentChecker(iHPPercent) );
	
	// selfhppercentrange
	const char* pszHPPercentRange = t.get<const char*>("selfhppercentrange");
	if( pszHPPercentRange )
	{
		std::string					strString(pszHPPercentRange);
		std::vector<std::string>	vSplit;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		if( vSplit.size() != 2 )
			return false;
		
		boost::trim( vSplit[0]);
		boost::trim( vSplit[1]);

		int iMin = boost::lexical_cast<int>( vSplit[0] );
		int iMax = boost::lexical_cast<int>( vSplit[1] );

		if( iMin<0 || iMax<0 || iMin > iMax )
			return false;

		m_vActorChecker.push_back( new CMAAiActorHPPercentCheckerRange(iMin,iMax) );
	}

	// targethppercent
	int iTargetHPPercent = t.get<int>("hppercent");
	if( iTargetHPPercent > 0 )
		m_vTargetActorChecker.push_back( new CMAAiTargetActorHPPercentChecker( iTargetHPPercent ) );
	
	// SP
	int iSP = t.get<int>("sp");
	if( iSP > 0 )
		m_vActorChecker.push_back( new CMAAiActorSPChecker(iSP) );

	// Buff
	int iBuffCheck = t.get<int>("buffcheck");
	if( iBuffCheck > 0 )
		m_vTargetActorChecker.push_back( new CMAAiTargetActorBuffChecker() );

	// DeBuff
	int iDeBuffCheck = t.get<int>("debuffcheck");
	if( iDeBuffCheck > 0 )
		m_vTargetActorChecker.push_back( new CMAAiTargetActorDeBuffChecker() );

	// BlowCheck
	const char* pszBlowCheck = t.get<const char*>("blowcheck");
	if( pszBlowCheck )
	{
		std::string					strString(pszBlowCheck);
		std::vector<std::string>	vSplit;
		std::vector<UINT>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( !vTableID.empty() )
			m_vTargetActorChecker.push_back( new CMAAiTargetActorBlowChecker( vTableID ) );
	}

	// NoBlowCheck
	const char* pszNoBlowCheck = t.get<const char*>("noblowcheck");
	if( pszNoBlowCheck )
	{
		std::string					strString(pszNoBlowCheck);
		std::vector<std::string>	vSplit;
		std::vector<UINT>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( !vTableID.empty() )
			m_vTargetActorChecker.push_back( new CMAAiTargetActorBlowChecker( vTableID,true ) );
	}

	// usedskill
	const char* pszUsedSkill = t.get<const char*>("usedskill");
	if( pszUsedSkill )
	{
		std::string					strString(pszUsedSkill);
		std::vector<std::string>	vSplit;
		std::vector<UINT>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( !vTableID.empty() )
			m_vActorChecker.push_back( new CMAAiActorUsedSkillChecker( vTableID ) );
	}

	// notusedskill
	const char* pszNotUsedSkill = t.get<const char*>("notusedskill");
	if( pszNotUsedSkill )
	{
		std::string					strString(pszNotUsedSkill);
		std::vector<std::string>	vSplit;
		std::vector<UINT>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( !vTableID.empty() )
			m_vActorChecker.push_back( new CMAAiActorNotUsedSkillChecker( vTableID ) );
	}

	// encountertime
	int iEnCounterTime = t.get<int>("encountertime");
	if( iEnCounterTime > 0 )
		m_vActorChecker.push_back( new CMAAiActorEncounterTickChecker( iEnCounterTime ) );

	// limittime
	int iLimitTime = t.get<int>("limittime");
	if( iLimitTime > 0 )
		m_vActorChecker.push_back( new CMAAiActorLimitTickChecker( iLimitTime ) );

	// DieAnnounce
	const char* pszAnnounce = t.get<const char*>("announce");
	if( pszAnnounce )
	{
		std::string					strString(pszAnnounce);
		std::vector<std::string>	vSplit;
		std::vector<int>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( vTableID.size() == 2 )
			m_vActorChecker.push_back( new CMAAiActorNotifyAnnounceChecker( vTableID[0], vTableID[1] ) );
	}

#if defined( PRE_FIX_68096 )	
	int iGlobalCoolTimeIndex = t.get<int>("globalcooltime");
	if( iGlobalCoolTimeIndex > 0 )
		m_vActorChecker.push_back( new CMAAiActorGlobalCoolTimeChecker(iGlobalCoolTimeIndex-1) );

	for( int i=1 ; i<ActionTableCommon::eCommon::MaxGlobalCoolTime ; ++i )
	{
		char szBuf[MAX_PATH];
		sprintf_s( szBuf, "globalcooltime%d", i+1 );
		int nGlobalCoolTimeIndex = t.get<int>(szBuf);		
		if( nGlobalCoolTimeIndex > 0 )
			m_vActorChecker.push_back( new CMAAiActorGlobalCoolTimeChecker(nGlobalCoolTimeIndex-1) );
		else
			break;
	}
#else
	int iGlobalCoolTimeIndex = t.get<int>("globalcooltime");
	if( iGlobalCoolTimeIndex > 0 )
		m_vActorChecker.push_back( new CMAAiActorGlobalCoolTimeChecker(iGlobalCoolTimeIndex-1) );
#endif

	// noaggro
	int iNoAggro = t.get<int>("noaggro");
	if( iEnCounterTime > 0 )
		m_vActorChecker.push_back( new CMAAiActorEncounterTickChecker( iEnCounterTime ) );

	// checkpassiveskill
	const char* pszCheckPassiveSkill = t.get<const char*>("checkpassiveskill");
	if( pszCheckPassiveSkill )
	{
		std::string					strString(pszCheckPassiveSkill);
		std::vector<std::string>	vSplit;
		std::vector<int>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( vTableID.size() == 2 )
			m_vActorChecker.push_back( new CMAAiActorPassiveSkillChecker( vTableID[0], vTableID[1] ) );
	}

	return bOnAddLoadChecker( t );
}

// Create additional Checker on Action side
bool CMAAiActionCheckerManager::bOnAddLoadChecker( lua_tinker::table& t )
{
	return true;
}

// Create additional Checker for Skill
bool CMAAiSkillCheckerManager::bOnAddLoadChecker( lua_tinker::table& t )
{
	int iRate = t.get<int>("rate");
	// If no rate value is entered on the skill side, DefaultValue is 100%.
	if( iRate == 0 )
		iRate = 100;

	m_vActorChecker.push_back( new CMAAiActorSkillRateChecker( iRate ) );

	int iLimitCount = t.get<int>("limitcount");
	if( iLimitCount > 0 )
		m_vActorChecker.push_back( new CMAAiActorSkillLimitCountChecker( iLimitCount ) );

	int iWaitOrder = t.get<int>("waitorder");
	if( iWaitOrder > 0 )
		m_vActorChecker.push_back( new CMAAiActorWaitOrderChecker(iWaitOrder) );	

	// checkskilltoggle
	const char* pszCheckSkillToggle = t.get<const char*>("checkskilltoggle");
	if( pszCheckSkillToggle )
	{
		std::string					strString(pszCheckSkillToggle);
		std::vector<std::string>	vSplit;
		std::vector<int>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( vTableID.size() == 2 )
			m_vActorChecker.push_back( new CMAAiActorSkillToggleChecker( vTableID[0], vTableID[1] ) );
	}

	return true;
}

bool CMAAiCheckerManager::bIsAnnounceSkill()
{
	for( UINT i=0 ; i<m_vActorChecker.size() ; ++i )
	{
		if( m_vActorChecker[i]->bIsAnnounceSkill() )
			return true;
	}

	return false;
}
