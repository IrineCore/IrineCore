#include "stdafx.h"
#include "DnPostCap60CombatBlows.h"
#include "DnStateBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

static void ParseStateEffectTokens( const char* szValue, int& nBlowIndex, int& nDuration, std::string& strParam )
{
	nBlowIndex = 0;
	nDuration = 0;
	strParam.clear();
	if( !szValue || szValue[0] == '\0' )
		return;

	std::vector<std::string> vlTokens;
	TokenizeA( std::string( szValue ), vlTokens, ";" );
	if( vlTokens.size() >= 1 ) nBlowIndex = atoi( vlTokens[0].c_str() );
	if( vlTokens.size() >= 2 ) nDuration = atoi( vlTokens[1].c_str() );
	if( vlTokens.size() >= 3 ) strParam = vlTokens[2];
}

// =============================================================================
// BLOW_282: CDnSufferForceCirticalDamage
// =============================================================================
CDnSufferForceCirticalDamage::CDnSufferForceCirticalDamage( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_282;
	SetValue( szValue );
	m_fValue = szValue ? (float)atof( szValue ) : 0.0f;
}
CDnSufferForceCirticalDamage::~CDnSufferForceCirticalDamage( void ) {}
void CDnSufferForceCirticalDamage::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnSufferForceCirticalDamage::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnSufferForceCirticalDamage::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_283: CDnSubSkillOnTargetHitBlow
// =============================================================================
CDnSubSkillOnTargetHitBlow::CDnSubSkillOnTargetHitBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nSkillID( 0 ), m_fProb( 1.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_283;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_nSkillID = atoi( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_fProb = (float)atof( vlTokens[1].c_str() );
	}
}
CDnSubSkillOnTargetHitBlow::~CDnSubSkillOnTargetHitBlow( void ) {}
void CDnSubSkillOnTargetHitBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnSubSkillOnTargetHitBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnSubSkillOnTargetHitBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_284: CDnTimeGoesGrowingBlow
// =============================================================================
CDnTimeGoesGrowingBlow::CDnTimeGoesGrowingBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fInterval( 1.0f ), m_fElapsed( 0.0f ), m_nMaxStack( 1 ), m_nCurStack( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_284;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_fInterval = (float)atof( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_nMaxStack = atoi( vlTokens[1].c_str() );
		if( m_fInterval <= 0.0f ) m_fInterval = 1.0f;
	}
}
CDnTimeGoesGrowingBlow::~CDnTimeGoesGrowingBlow( void ) {}
void CDnTimeGoesGrowingBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_fElapsed = 0.0f; m_nCurStack = 0; }
void CDnTimeGoesGrowingBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	if( m_nCurStack < m_nMaxStack )
	{
		m_fElapsed += fDelta;
		if( m_fElapsed >= m_fInterval )
		{
			m_fElapsed -= m_fInterval;
			++m_nCurStack;
		}
	}
}
void CDnTimeGoesGrowingBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_285: CDnDoWhenSkillBeginBlow
// =============================================================================
CDnDoWhenSkillBeginBlow::CDnDoWhenSkillBeginBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nActionType( 0 ), m_nParam( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_285;
	SetValue( szValue );
	AddCallBackType( SB_ONBEGIN_SKILL );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_nActionType = atoi( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_nParam = atoi( vlTokens[1].c_str() );
	}
}
CDnDoWhenSkillBeginBlow::~CDnDoWhenSkillBeginBlow( void ) {}
void CDnDoWhenSkillBeginBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnDoWhenSkillBeginBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnDoWhenSkillBeginBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_286: CDnCheckImmuneBeDamage
// =============================================================================
CDnCheckImmuneBeDamage::CDnCheckImmuneBeDamage( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fProb( 0.0f ), m_nAddBlowIndex( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_286;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_fProb = (float)atof( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_nAddBlowIndex = atoi( vlTokens[1].c_str() );
	}
}
CDnCheckImmuneBeDamage::~CDnCheckImmuneBeDamage( void ) {}
void CDnCheckImmuneBeDamage::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnCheckImmuneBeDamage::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnCheckImmuneBeDamage::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_288: CDnAddStateOnTargetHitBlow
// =============================================================================
CDnAddStateOnTargetHitBlow::CDnAddStateOnTargetHitBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nStateEffectIndex( 0 ), m_nDurationTime( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_288;
	SetValue( szValue );
	ParseStateEffectTokens( szValue, m_nStateEffectIndex, m_nDurationTime, m_strStateParam );
#ifdef _GAMESERVER
	AddCallBackType( SB_ONTARGETHIT );
#endif
}
CDnAddStateOnTargetHitBlow::~CDnAddStateOnTargetHitBlow( void ) {}
void CDnAddStateOnTargetHitBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddStateOnTargetHitBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddStateOnTargetHitBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnAddStateOnTargetHitBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	if( hTargetActor && m_nStateEffectIndex > 0 && m_nStateEffectIndex < STATE_BLOW::BLOW_MAX )
	{
		hTargetActor->CmdAddStateEffect( &m_ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)m_nStateEffectIndex, m_nDurationTime, m_strStateParam.c_str() );
	}
}
#endif

// =============================================================================
// BLOW_289: CDnAddStateOnSkillBeginBlow
// =============================================================================
CDnAddStateOnSkillBeginBlow::CDnAddStateOnSkillBeginBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nStateEffectIndex( 0 ), m_nDurationTime( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_289;
	SetValue( szValue );
	ParseStateEffectTokens( szValue, m_nStateEffectIndex, m_nDurationTime, m_strStateParam );
}
CDnAddStateOnSkillBeginBlow::~CDnAddStateOnSkillBeginBlow( void ) {}
void CDnAddStateOnSkillBeginBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddStateOnSkillBeginBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddStateOnSkillBeginBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_290: CDnAddStateOnSummonMonsterBlow
// =============================================================================
CDnAddStateOnSummonMonsterBlow::CDnAddStateOnSummonMonsterBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nMonsterID( 0 ), m_nStateEffectIndex( 0 ), m_nDurationTime( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_290;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_nMonsterID = atoi( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_nStateEffectIndex = atoi( vlTokens[1].c_str() );
		if( vlTokens.size() >= 3 ) m_nDurationTime = atoi( vlTokens[2].c_str() );
		if( vlTokens.size() >= 4 ) m_strStateParam = vlTokens[3];
	}
}
CDnAddStateOnSummonMonsterBlow::~CDnAddStateOnSummonMonsterBlow( void ) {}
void CDnAddStateOnSummonMonsterBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddStateOnSummonMonsterBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddStateOnSummonMonsterBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_292: CDnAddWeaponBasedDamageBlow
// =============================================================================
CDnAddWeaponBasedDamageBlow::CDnAddWeaponBasedDamageBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fDamageRatio( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_292;
	SetValue( szValue );
	m_fDamageRatio = szValue ? (float)atof( szValue ) : 0.0f;
	m_fValue = m_fDamageRatio;
}
CDnAddWeaponBasedDamageBlow::~CDnAddWeaponBasedDamageBlow( void ) {}
void CDnAddWeaponBasedDamageBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddWeaponBasedDamageBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_293: CDnAddDamageByTargetDistanceBlow
// =============================================================================
CDnAddDamageByTargetDistanceBlow::CDnAddDamageByTargetDistanceBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fMinDistance( 0.0f ), m_fMaxDistance( 0.0f ), m_fAddDamageRatio( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_293;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_fMinDistance = (float)atof( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_fMaxDistance = (float)atof( vlTokens[1].c_str() );
		if( vlTokens.size() >= 3 ) m_fAddDamageRatio = (float)atof( vlTokens[2].c_str() );
	}
}
CDnAddDamageByTargetDistanceBlow::~CDnAddDamageByTargetDistanceBlow( void ) {}
void CDnAddDamageByTargetDistanceBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddDamageByTargetDistanceBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddDamageByTargetDistanceBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
float CDnAddDamageByTargetDistanceBlow::CalcDistanceDamageRatio( float fDistance ) const
{
	if( fDistance >= m_fMinDistance && (m_fMaxDistance <= 0.0f || fDistance <= m_fMaxDistance) )
		return m_fAddDamageRatio;
	return 0.0f;
}

// =============================================================================
// BLOW_294: CDnAddDamageByWeightBlow
// =============================================================================
CDnAddDamageByWeightBlow::CDnAddDamageByWeightBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fWeightRatio( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_294;
	SetValue( szValue );
	m_fWeightRatio = szValue ? (float)atof( szValue ) : 0.0f;
	m_fValue = m_fWeightRatio;
}
CDnAddDamageByWeightBlow::~CDnAddDamageByWeightBlow( void ) {}
void CDnAddDamageByWeightBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddDamageByWeightBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddDamageByWeightBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_295: CDnAddStateOnBreakSuperArmorBlow
// =============================================================================
CDnAddStateOnBreakSuperArmorBlow::CDnAddStateOnBreakSuperArmorBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nStateEffectIndex( 0 ), m_nDurationTime( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_295;
	SetValue( szValue );
	ParseStateEffectTokens( szValue, m_nStateEffectIndex, m_nDurationTime, m_strStateParam );
}
CDnAddStateOnBreakSuperArmorBlow::~CDnAddStateOnBreakSuperArmorBlow( void ) {}
void CDnAddStateOnBreakSuperArmorBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddStateOnBreakSuperArmorBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddStateOnBreakSuperArmorBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_296: CDnAddStateOnAppointedSkillBlow
// =============================================================================
CDnAddStateOnAppointedSkillBlow::CDnAddStateOnAppointedSkillBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nSkillID( 0 ), m_nStateEffectIndex( 0 ), m_nDurationTime( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_296;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_nSkillID = atoi( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_nStateEffectIndex = atoi( vlTokens[1].c_str() );
		if( vlTokens.size() >= 3 ) m_nDurationTime = atoi( vlTokens[2].c_str() );
		if( vlTokens.size() >= 4 ) m_strStateParam = vlTokens[3];
	}
}
CDnAddStateOnAppointedSkillBlow::~CDnAddStateOnAppointedSkillBlow( void ) {}
void CDnAddStateOnAppointedSkillBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddStateOnAppointedSkillBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddStateOnAppointedSkillBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_297: CDnAddStateOnAppointedSkillHitBlow
// =============================================================================
CDnAddStateOnAppointedSkillHitBlow::CDnAddStateOnAppointedSkillHitBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nSkillID( 0 ), m_nStateEffectIndex( 0 ), m_nDurationTime( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_297;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_nSkillID = atoi( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_nStateEffectIndex = atoi( vlTokens[1].c_str() );
		if( vlTokens.size() >= 3 ) m_nDurationTime = atoi( vlTokens[2].c_str() );
		if( vlTokens.size() >= 4 ) m_strStateParam = vlTokens[3];
	}
}
CDnAddStateOnAppointedSkillHitBlow::~CDnAddStateOnAppointedSkillHitBlow( void ) {}
void CDnAddStateOnAppointedSkillHitBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddStateOnAppointedSkillHitBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddStateOnAppointedSkillHitBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_299: CDnDamageTransitionToSummonMonsterBlow
// =============================================================================
CDnDamageTransitionToSummonMonsterBlow::CDnDamageTransitionToSummonMonsterBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fTransitionRatio( 0.0f ), m_nSummonMonsterID( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_299;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_fTransitionRatio = (float)atof( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_nSummonMonsterID = atoi( vlTokens[1].c_str() );
		m_fValue = m_fTransitionRatio;
	}
}
CDnDamageTransitionToSummonMonsterBlow::~CDnDamageTransitionToSummonMonsterBlow( void ) {}
void CDnDamageTransitionToSummonMonsterBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnDamageTransitionToSummonMonsterBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnDamageTransitionToSummonMonsterBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_300: CDnAddStateOnBlockBlow
// =============================================================================
CDnAddStateOnBlockBlow::CDnAddStateOnBlockBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nStateEffectIndex( 0 ), m_nDurationTime( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_300;
	SetValue( szValue );
	ParseStateEffectTokens( szValue, m_nStateEffectIndex, m_nDurationTime, m_strStateParam );
}
CDnAddStateOnBlockBlow::~CDnAddStateOnBlockBlow( void ) {}
void CDnAddStateOnBlockBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddStateOnBlockBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddStateOnBlockBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_301: CDnGaugeShieldBlow
// =============================================================================
CDnGaugeShieldBlow::CDnGaugeShieldBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nMaxGauge( 0 ), m_nCurGauge( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_301;
	SetValue( szValue );
	m_nMaxGauge = szValue ? _atoi64( szValue ) : 0;
	m_nCurGauge = m_nMaxGauge;
	m_fValue = (float)m_nMaxGauge;
#ifdef _GAMESERVER
	AddCallBackType( SB_ONCALCDAMAGE );
#endif
}
CDnGaugeShieldBlow::~CDnGaugeShieldBlow( void ) {}
void CDnGaugeShieldBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_nCurGauge = m_nMaxGauge; }
void CDnGaugeShieldBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnGaugeShieldBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
float CDnGaugeShieldBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	if( m_nCurGauge <= 0 || fOriginalDamage <= 0.0f )
		return 0.0f;

	INT64 nDmg = (INT64)fOriginalDamage;
	if( m_nCurGauge >= nDmg )
	{
		m_nCurGauge -= nDmg;
		return -fOriginalDamage;
	}
	else
	{
		float fAbsorbed = (float)m_nCurGauge;
		m_nCurGauge = 0;
		SetState( STATE_BLOW::STATE_END );
		return -fAbsorbed;
	}
}
#endif

// =============================================================================
// BLOW_303: CDnDuplicateExplosionBlow
// =============================================================================
CDnDuplicateExplosionBlow::CDnDuplicateExplosionBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nTargetBlowIndex( 0 ), m_fDamageRatio( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_303;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_nTargetBlowIndex = atoi( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_fDamageRatio = (float)atof( vlTokens[1].c_str() );
	}
}
CDnDuplicateExplosionBlow::~CDnDuplicateExplosionBlow( void ) {}
void CDnDuplicateExplosionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnDuplicateExplosionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_314: CDnDecreaseSuperArmorBlow
// =============================================================================
CDnDecreaseSuperArmorBlow::CDnDecreaseSuperArmorBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fDecreaseRatio( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_314;
	SetValue( szValue );
	m_fDecreaseRatio = szValue ? (float)atof( szValue ) : 0.0f;
	m_fValue = m_fDecreaseRatio;
}
CDnDecreaseSuperArmorBlow::~CDnDecreaseSuperArmorBlow( void ) {}
void CDnDecreaseSuperArmorBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnDecreaseSuperArmorBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnDecreaseSuperArmorBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
