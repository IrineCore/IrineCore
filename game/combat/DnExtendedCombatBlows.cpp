#include "stdafx.h"
#include "DnExtendedCombatBlows.h"
#include "DnStateBlow.h"
#ifdef _GAMESERVER
#include "DnMonsterActor.h"
#include "DnSkill.h"
#include "DnProjectile.h"
#endif

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

namespace
{
	void ExtractBrackets( const std::string& str, std::vector<std::string>& outBrackets )
	{
		outBrackets.clear();
		size_t pos = 0;
		while( pos < str.size() )
		{
			size_t lb = str.find( '[', pos );
			if( lb == std::string::npos )
				break;
			size_t rb = str.find( ']', lb + 1 );
			if( rb == std::string::npos )
				break;
			outBrackets.push_back( str.substr( lb + 1, rb - lb - 1 ) );
			pos = rb + 1;
		}
	}

	bool ParseSlashSubBlow( const std::string& token, SExtendedSubBlowSpec& outSpec )
	{
		size_t slash = token.find( '/' );
		if( slash == std::string::npos )
			return false;
		int idx = atoi( token.substr( 0, slash ).c_str() );
		if( idx <= 0 || idx >= STATE_BLOW::BLOW_MAX )
			return false;
		outSpec.nBlowIndex = idx;
		outSpec.strParam = token.substr( slash + 1 );
		return true;
	}

	void ParseIntList( const std::string& str, std::vector<int>& outIDs )
	{
		outIDs.clear();
		if( str.empty() )
			return;
		std::vector<std::string> tokens;
		TokenizeA( str, tokens, ";," );
		for( size_t i = 0; i < tokens.size(); ++i )
		{
			int v = atoi( tokens[i].c_str() );
			if( v > 0 )
				outIDs.push_back( v );
		}
	}

	void ParseLegacySemiSubBlow( const char* szValue, int& nDurationMs, std::vector<SExtendedSubBlowSpec>& outSpecs )
	{
		if( !szValue || szValue[0] == '\0' )
			return;
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 )
		{
			int idx = atoi( vlTokens[0].c_str() );
			if( idx > 0 && idx < STATE_BLOW::BLOW_MAX )
			{
				if( vlTokens.size() >= 2 )
					nDurationMs = atoi( vlTokens[1].c_str() );
				std::string param = ( vlTokens.size() >= 3 ) ? vlTokens[2] : "";
				outSpecs.push_back( SExtendedSubBlowSpec( idx, param ) );
			}
		}
	}

	// Parse "[prob;duration;cooltime][blow1/val1][blow2/val2]_[skill1;skill2]"
	void ParseAppointedSkillBlowFormat(
		const char* szValue,
		float& outProb,
		int& outDurationMs,
		float& outCoolTimeSec,
		std::vector<SExtendedSubBlowSpec>& outSpecs,
		std::vector<int>& outSkillIDs )
	{
		outProb = 1.0f;
		outDurationMs = 0;
		outCoolTimeSec = 0.0f;
		outSpecs.clear();
		outSkillIDs.clear();
		if( !szValue || szValue[0] == '\0' )
			return;

		std::string s( szValue );
		if( s.find( '[' ) != std::string::npos )
		{
			std::string leftPart = s;
			std::string rightPart;
			size_t sep = s.rfind( "_[" );
			if( sep != std::string::npos )
			{
				leftPart = s.substr( 0, sep );
				rightPart = s.substr( sep + 1 );
			}

			std::vector<std::string> leftBrackets;
			ExtractBrackets( leftPart, leftBrackets );
			if( !leftBrackets.empty() )
			{
				std::vector<std::string> hdr;
				TokenizeA( leftBrackets[0], hdr, ";" );
				if( hdr.size() >= 1 )
				{
					float p = (float)atof( hdr[0].c_str() );
					outProb = ( p > 1.0f ) ? ( p * 0.01f ) : p;
				}
				if( hdr.size() >= 2 )
					outDurationMs = atoi( hdr[1].c_str() );
				if( hdr.size() >= 3 )
					outCoolTimeSec = (float)atoi( hdr[2].c_str() ) / 1000.0f;

				for( size_t i = 1; i < leftBrackets.size(); ++i )
				{
					SExtendedSubBlowSpec spec;
					if( ParseSlashSubBlow( leftBrackets[i], spec ) )
						outSpecs.push_back( spec );
				}
			}

			if( !rightPart.empty() )
			{
				std::vector<std::string> rightBrackets;
				ExtractBrackets( rightPart, rightBrackets );
				if( !rightBrackets.empty() )
					ParseIntList( rightBrackets[0], outSkillIDs );
			}
			return;
		}

		// Legacy "skillID;blowIdx;dur;param"
		std::vector<std::string> vlTokens;
		TokenizeA( s, vlTokens, ";" );
		if( vlTokens.size() >= 1 )
		{
			int sid = atoi( vlTokens[0].c_str() );
			if( sid > 0 ) outSkillIDs.push_back( sid );
		}
		if( vlTokens.size() >= 2 )
		{
			int idx = atoi( vlTokens[1].c_str() );
			if( idx > 0 && idx < STATE_BLOW::BLOW_MAX )
			{
				if( vlTokens.size() >= 3 ) outDurationMs = atoi( vlTokens[2].c_str() );
				std::string param = ( vlTokens.size() >= 4 ) ? vlTokens[3] : "";
				outSpecs.push_back( SExtendedSubBlowSpec( idx, param ) );
			}
		}
	}

#ifdef _GAMESERVER
	bool RollChance( CMultiRoom* pRoom, float fProb )
	{
		if( fProb >= 1.0f )
			return true;
		if( fProb <= 0.0f )
			return false;
		int nRoll = pRoom ? ( _rand( pRoom ) % 10000 ) : ( rand() % 10000 );
		return nRoll < (int)( fProb * 10000.0f );
	}

	void ApplySubBlowSpecs( DnActorHandle hTarget, const CDnSkill::SkillInfo* pParentSkillInfo, int nDurationMs, const std::vector<SExtendedSubBlowSpec>& specs )
	{
		if( !hTarget || hTarget->IsDie() )
			return;
		int nDur = ( nDurationMs > 0 ) ? nDurationMs : 5000;
		for( size_t i = 0; i < specs.size(); ++i )
		{
			if( specs[i].nBlowIndex > 0 && specs[i].nBlowIndex < STATE_BLOW::BLOW_MAX )
			{
				hTarget->CmdAddStateEffect( pParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)specs[i].nBlowIndex, nDur, specs[i].strParam.c_str(), false, false );
			}
		}
	}
#endif
}

// =============================================================================
// BLOW_282: CDnSufferForceCirticalDamage
// =============================================================================
CDnSufferForceCirticalDamage::CDnSufferForceCirticalDamage( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_282;
	SetValue( szValue );
	m_fValue = ( szValue && szValue[0] != '\0' ) ? (float)atof( szValue ) : 1.0f;
}
CDnSufferForceCirticalDamage::~CDnSufferForceCirticalDamage( void ) {}
void CDnSufferForceCirticalDamage::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnSufferForceCirticalDamage::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnSufferForceCirticalDamage::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}

// =============================================================================
// BLOW_283: CDnSubSkillOnTargetHitBlow
// =============================================================================
CDnSubSkillOnTargetHitBlow::CDnSubSkillOnTargetHitBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nSkillID( 0 ), m_fProb( 1.0f ), m_fCoolTime( 0.0f ), m_fRemainCoolTime( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_283;
	SetValue( szValue );
#ifdef _GAMESERVER
	AddCallBackType( SB_ONTARGETHIT );
#endif
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_nSkillID = atoi( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 )
		{
			float p = (float)atof( vlTokens[1].c_str() );
			m_fProb = ( p > 1.0f ) ? ( p * 0.01f ) : p;
		}
		if( vlTokens.size() >= 3 ) m_fCoolTime = (float)atoi( vlTokens[2].c_str() ) / 1000.0f;
	}
}
CDnSubSkillOnTargetHitBlow::~CDnSubSkillOnTargetHitBlow( void ) {}
void CDnSubSkillOnTargetHitBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_fRemainCoolTime = 0.0f; }
void CDnSubSkillOnTargetHitBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	if( m_fRemainCoolTime > 0.0f )
	{
		m_fRemainCoolTime -= fDelta;
		if( m_fRemainCoolTime < 0.0f ) m_fRemainCoolTime = 0.0f;
	}
}
void CDnSubSkillOnTargetHitBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnSubSkillOnTargetHitBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	if( !m_hActor || m_hActor->IsDie() || m_nSkillID <= 0 || m_fRemainCoolTime > 0.0f )
		return;
	if( !RollChance( m_hActor->GetRoom(), m_fProb ) )
		return;
	if( m_hActor->FindSkill( m_nSkillID ) )
	{
		m_hActor->UseSkill( m_nSkillID, false, true );
		m_fRemainCoolTime = m_fCoolTime;
	}
}
#endif

// =============================================================================
// BLOW_284: CDnTimeGoesGrowingBlow
// =============================================================================
CDnTimeGoesGrowingBlow::CDnTimeGoesGrowingBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fInterval( 1.0f ), m_fElapsed( 0.0f ), m_nMaxStack( 1 ), m_nCurStack( 0 ), m_nSubDurationMs( 5000 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_284;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::string s( szValue );
		if( s.find( '[' ) != std::string::npos )
		{
			std::vector<std::string> brackets;
			ExtractBrackets( s, brackets );
			if( !brackets.empty() )
			{
				std::vector<std::string> hdr;
				TokenizeA( brackets[0], hdr, ";" );
				if( hdr.size() >= 1 ) m_fInterval = (float)atof( hdr[0].c_str() );
				if( m_fInterval > 100.0f ) m_fInterval /= 1000.0f;
				if( hdr.size() >= 2 ) m_nMaxStack = atoi( hdr[1].c_str() );
				if( hdr.size() >= 3 ) m_nSubDurationMs = atoi( hdr[2].c_str() );
				for( size_t i = 1; i < brackets.size(); ++i )
				{
					SExtendedSubBlowSpec spec;
					if( ParseSlashSubBlow( brackets[i], spec ) )
						m_vecSubBlows.push_back( spec );
				}
			}
		}
		else
		{
			std::vector<std::string> vlTokens;
			TokenizeA( s, vlTokens, ";" );
			if( vlTokens.size() >= 1 ) m_fInterval = (float)atof( vlTokens[0].c_str() );
			if( m_fInterval > 100.0f ) m_fInterval /= 1000.0f;
			if( vlTokens.size() >= 2 ) m_nMaxStack = atoi( vlTokens[1].c_str() );
			if( vlTokens.size() >= 4 )
			{
				int bIdx = atoi( vlTokens[2].c_str() );
				if( bIdx > 0 && bIdx < STATE_BLOW::BLOW_MAX )
					m_vecSubBlows.push_back( SExtendedSubBlowSpec( bIdx, vlTokens[3] ) );
			}
		}
		if( m_fInterval <= 0.0f ) m_fInterval = 1.0f;
		if( m_nMaxStack <= 0 ) m_nMaxStack = 1;
	}
}
CDnTimeGoesGrowingBlow::~CDnTimeGoesGrowingBlow( void ) {}
void CDnTimeGoesGrowingBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	m_fElapsed = 0.0f;
	m_nCurStack = 0;
	m_fValue = 0.0f;
	m_vecAppliedServerBlowIDs.clear();
}
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
			m_fValue = (float)m_nCurStack;
#ifdef _GAMESERVER
			if( m_hActor && !m_hActor->IsDie() && !m_vecSubBlows.empty() )
			{
				int nDur = ( m_StateBlow.fDurationTime > 0.0f ) ? (int)( m_StateBlow.fDurationTime * 1000.0f ) : m_nSubDurationMs;
				for( size_t i = 0; i < m_vecSubBlows.size(); ++i )
				{
					int nBlowID = m_hActor->CmdAddStateEffect( &m_ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)m_vecSubBlows[i].nBlowIndex, nDur, m_vecSubBlows[i].strParam.c_str(), false, false );
					if( nBlowID > 0 )
						m_vecAppliedServerBlowIDs.push_back( nBlowID );
				}
			}
#endif
		}
	}
}
void CDnTimeGoesGrowingBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	if( m_hActor )
	{
		for( size_t i = 0; i < m_vecAppliedServerBlowIDs.size(); ++i )
			m_hActor->CmdRemoveStateEffectFromID( m_vecAppliedServerBlowIDs[i] );
	}
	m_vecAppliedServerBlowIDs.clear();
#endif
}

// =============================================================================
// BLOW_285: CDnDoWhenSkillBeginBlow
// =============================================================================
CDnDoWhenSkillBeginBlow::CDnDoWhenSkillBeginBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nActionType( 0 ), m_nParam( 0 ), m_fProb( 1.0f ), m_nDurationMs( 5000 ), m_fCoolTime( 0.0f ), m_fRemainCoolTime( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_285;
	SetValue( szValue );
	AddCallBackType( SB_ONBEGIN_SKILL );
	if( szValue && szValue[0] != '\0' )
	{
		std::string s( szValue );
		if( s.find( '[' ) != std::string::npos )
		{
			ParseAppointedSkillBlowFormat( szValue, m_fProb, m_nDurationMs, m_fCoolTime, m_vecSubBlows, m_vecTargetSkillIDs );
		}
		else
		{
			std::vector<std::string> vlTokens;
			TokenizeA( s, vlTokens, ";" );
			if( vlTokens.size() >= 1 ) m_nActionType = atoi( vlTokens[0].c_str() );
			if( vlTokens.size() >= 2 ) m_nParam = atoi( vlTokens[1].c_str() );
		}
	}
}
CDnDoWhenSkillBeginBlow::~CDnDoWhenSkillBeginBlow( void ) {}
void CDnDoWhenSkillBeginBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_fRemainCoolTime = 0.0f; }
void CDnDoWhenSkillBeginBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	if( m_fRemainCoolTime > 0.0f )
	{
		m_fRemainCoolTime -= fDelta;
		if( m_fRemainCoolTime < 0.0f ) m_fRemainCoolTime = 0.0f;
	}
}
void CDnDoWhenSkillBeginBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnDoWhenSkillBeginBlow::OnBeginSkill( DnSkillHandle hSkill )
{
	if( !m_hActor || m_hActor->IsDie() || !hSkill || m_fRemainCoolTime > 0.0f )
		return;
	if( !m_vecTargetSkillIDs.empty() &&
		std::find( m_vecTargetSkillIDs.begin(), m_vecTargetSkillIDs.end(), hSkill->GetClassID() ) == m_vecTargetSkillIDs.end() )
		return;
	if( !RollChance( m_hActor->GetRoom(), m_fProb ) )
		return;

	if( !m_vecSubBlows.empty() )
	{
		ApplySubBlowSpecs( m_hActor, &m_ParentSkillInfo, m_nDurationMs, m_vecSubBlows );
		m_fRemainCoolTime = m_fCoolTime;
	}
	else if( m_nActionType > 0 && m_nParam > 0 )
	{
		if( m_nActionType == 1 && m_hActor->FindSkill( m_nParam ) )
			m_hActor->UseSkill( m_nParam, false, true );
		else if( m_nActionType == 2 && m_nParam < STATE_BLOW::BLOW_MAX )
			m_hActor->CmdAddStateEffect( &m_ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)m_nParam, m_nDurationMs, "1", false, false );
		m_fRemainCoolTime = m_fCoolTime;
	}
}
#endif

// =============================================================================
// BLOW_286: CDnCheckImmuneBeDamage
// =============================================================================
CDnCheckImmuneBeDamage::CDnCheckImmuneBeDamage( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fProb( 1.0f ), m_nAddBlowIndex( 0 ), m_nMode( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_286;
	SetValue( szValue );
#ifdef _GAMESERVER
	AddCallBackType( SB_ONDEFENSEATTACK );
#endif
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 )
		{
			m_nMode = atoi( vlTokens[0].c_str() );
			float v = (float)atof( vlTokens[0].c_str() );
			if( v > 0.0f )
				m_fProb = ( v > 1.0f ) ? ( v * 0.01f ) : v;
		}
		if( vlTokens.size() >= 2 ) m_nAddBlowIndex = atoi( vlTokens[1].c_str() );
	}
}
CDnCheckImmuneBeDamage::~CDnCheckImmuneBeDamage( void ) {}
void CDnCheckImmuneBeDamage::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnCheckImmuneBeDamage::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnCheckImmuneBeDamage::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
bool CDnCheckImmuneBeDamage::OnDefenseAttack( DnActorHandle hActor, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	if( !m_hActor || m_hActor->IsDie() )
		return false;

	// Breaking Point (Skill 551) / ImmuneBeDamage check: if hit is resisted/blocked/immune, optionally trigger configured state effect
	if( m_nAddBlowIndex > 0 && m_nAddBlowIndex < STATE_BLOW::BLOW_MAX && RollChance( m_hActor->GetRoom(), m_fProb ) )
	{
		int nDur = ( m_StateBlow.fDurationTime > 0.0f ) ? (int)( m_StateBlow.fDurationTime * 1000.0f ) : 5000;
		m_hActor->CmdAddStateEffect( &m_ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)m_nAddBlowIndex, nDur, "1", false, false );
	}
	return false;
}
#endif

// =============================================================================
// BLOW_288: CDnAddStateOnTargetHitBlow
// DNT format: "[1;-1;0;0;100;10000;0;20000;1][29/0.20]" or "blowIdx;dur;param"
// =============================================================================
CDnAddStateOnTargetHitBlow::CDnAddStateOnTargetHitBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fProb( 1.0f ), m_nDurationTime( 5000 ), m_fCoolTime( 0.0f ), m_fRemainCoolTime( 0.0f ), m_bApplySelf( false )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_288;
	SetValue( szValue );
#ifdef _GAMESERVER
	AddCallBackType( SB_ONTARGETHIT );
#endif
	if( szValue && szValue[0] != '\0' )
	{
		std::string s( szValue );
		if( s.find( '[' ) != std::string::npos )
		{
			std::vector<std::string> brackets;
			ExtractBrackets( s, brackets );
			if( !brackets.empty() )
			{
				std::vector<std::string> hdr;
				TokenizeA( brackets[0], hdr, ";" );
				// Full 9-field header: [targetType;skillFilter;checkCrit;checkBlock;prob;durationMs;delayMs;coolTimeMs;applySelf]
				if( hdr.size() >= 8 )
				{
					float p = (float)atof( hdr[4].c_str() );
					m_fProb = ( p > 1.0f ) ? ( p * 0.01f ) : p;
					m_nDurationTime = atoi( hdr[5].c_str() );
					m_fCoolTime = (float)atoi( hdr[7].c_str() ) / 1000.0f;
					if( hdr.size() >= 9 )
						m_bApplySelf = ( atoi( hdr[8].c_str() ) != 0 );
				}
				else if( hdr.size() >= 2 )
				{
					float p = (float)atof( hdr[0].c_str() );
					m_fProb = ( p > 1.0f ) ? ( p * 0.01f ) : p;
					m_nDurationTime = atoi( hdr[1].c_str() );
					if( hdr.size() >= 3 ) m_fCoolTime = (float)atoi( hdr[2].c_str() ) / 1000.0f;
				}
				for( size_t i = 1; i < brackets.size(); ++i )
				{
					SExtendedSubBlowSpec spec;
					if( ParseSlashSubBlow( brackets[i], spec ) )
						m_vecSubBlows.push_back( spec );
				}
			}
		}
		else
		{
			ParseLegacySemiSubBlow( szValue, m_nDurationTime, m_vecSubBlows );
		}
	}
}
CDnAddStateOnTargetHitBlow::~CDnAddStateOnTargetHitBlow( void ) {}
void CDnAddStateOnTargetHitBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_fRemainCoolTime = 0.0f; }
void CDnAddStateOnTargetHitBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	if( m_fRemainCoolTime > 0.0f )
	{
		m_fRemainCoolTime -= fDelta;
		if( m_fRemainCoolTime < 0.0f ) m_fRemainCoolTime = 0.0f;
	}
}
void CDnAddStateOnTargetHitBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnAddStateOnTargetHitBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	if( !m_hActor || m_hActor->IsDie() || m_vecSubBlows.empty() || m_fRemainCoolTime > 0.0f )
		return;
	if( !RollChance( m_hActor->GetRoom(), m_fProb ) )
		return;

	DnActorHandle hApplyTarget = m_bApplySelf ? m_hActor : hTargetActor;
	if( hApplyTarget && !hApplyTarget->IsDie() )
	{
		ApplySubBlowSpecs( hApplyTarget, &m_ParentSkillInfo, m_nDurationTime, m_vecSubBlows );
		m_fRemainCoolTime = m_fCoolTime;
	}
}
#endif

// =============================================================================
// BLOW_289: CDnAddStateOnSkillBeginBlow
// =============================================================================
CDnAddStateOnSkillBeginBlow::CDnAddStateOnSkillBeginBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fProb( 1.0f ), m_nDurationTime( 5000 ), m_fCoolTime( 0.0f ), m_fRemainCoolTime( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_289;
	SetValue( szValue );
#ifdef _GAMESERVER
	AddCallBackType( SB_ONBEGIN_SKILL );
#endif
	if( szValue && szValue[0] != '\0' )
	{
		std::string s( szValue );
		if( s.find( '[' ) != std::string::npos )
		{
			std::vector<int> unusedSkills;
			ParseAppointedSkillBlowFormat( szValue, m_fProb, m_nDurationTime, m_fCoolTime, m_vecSubBlows, unusedSkills );
		}
		else
		{
			ParseLegacySemiSubBlow( szValue, m_nDurationTime, m_vecSubBlows );
		}
	}
}
CDnAddStateOnSkillBeginBlow::~CDnAddStateOnSkillBeginBlow( void ) {}
void CDnAddStateOnSkillBeginBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_fRemainCoolTime = 0.0f; }
void CDnAddStateOnSkillBeginBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	if( m_fRemainCoolTime > 0.0f )
	{
		m_fRemainCoolTime -= fDelta;
		if( m_fRemainCoolTime < 0.0f ) m_fRemainCoolTime = 0.0f;
	}
}
void CDnAddStateOnSkillBeginBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnAddStateOnSkillBeginBlow::OnBeginSkill( DnSkillHandle hSkill )
{
	if( !m_hActor || m_hActor->IsDie() || m_vecSubBlows.empty() || m_fRemainCoolTime > 0.0f )
		return;
	if( !RollChance( m_hActor->GetRoom(), m_fProb ) )
		return;

	ApplySubBlowSpecs( m_hActor, &m_ParentSkillInfo, m_nDurationTime, m_vecSubBlows );
	m_fRemainCoolTime = m_fCoolTime;
}
#endif

// =============================================================================
// BLOW_290: CDnAddStateOnSummonMonsterBlow
// DNT format: "[32008][1;100;-1;0;0][58/1.00]"
// =============================================================================
CDnAddStateOnSummonMonsterBlow::CDnAddStateOnSummonMonsterBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fProb( 1.0f ), m_nDurationTime( -1 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_290;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::string s( szValue );
		if( s.find( '[' ) != std::string::npos )
		{
			std::vector<std::string> brackets;
			ExtractBrackets( s, brackets );
			if( brackets.size() >= 1 )
				ParseIntList( brackets[0], m_vecMonsterIDs );
			if( brackets.size() >= 2 )
			{
				std::vector<std::string> hdr;
				TokenizeA( brackets[1], hdr, ";" );
				if( hdr.size() >= 2 )
				{
					float p = (float)atof( hdr[1].c_str() );
					m_fProb = ( p > 1.0f ) ? ( p * 0.01f ) : p;
				}
				if( hdr.size() >= 3 )
					m_nDurationTime = atoi( hdr[2].c_str() );
			}
			for( size_t i = 2; i < brackets.size(); ++i )
			{
				SExtendedSubBlowSpec spec;
				if( ParseSlashSubBlow( brackets[i], spec ) )
					m_vecSubBlows.push_back( spec );
			}
		}
		else
		{
			std::vector<std::string> vlTokens;
			TokenizeA( s, vlTokens, ";" );
			if( vlTokens.size() >= 1 )
			{
				int mid = atoi( vlTokens[0].c_str() );
				if( mid > 0 ) m_vecMonsterIDs.push_back( mid );
			}
			if( vlTokens.size() >= 2 )
			{
				int idx = atoi( vlTokens[1].c_str() );
				if( idx > 0 && idx < STATE_BLOW::BLOW_MAX )
				{
					if( vlTokens.size() >= 3 ) m_nDurationTime = atoi( vlTokens[2].c_str() );
					std::string param = ( vlTokens.size() >= 4 ) ? vlTokens[3] : "";
					m_vecSubBlows.push_back( SExtendedSubBlowSpec( idx, param ) );
				}
			}
		}
	}
}
CDnAddStateOnSummonMonsterBlow::~CDnAddStateOnSummonMonsterBlow( void ) {}
void CDnAddStateOnSummonMonsterBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddStateOnSummonMonsterBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddStateOnSummonMonsterBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnAddStateOnSummonMonsterBlow::OnSummonMonster( DnActorHandle hSummonedMonster, int nMonsterID )
{
	if( !hSummonedMonster || hSummonedMonster->IsDie() || m_vecSubBlows.empty() )
		return;
	if( !m_vecMonsterIDs.empty() &&
		std::find( m_vecMonsterIDs.begin(), m_vecMonsterIDs.end(), nMonsterID ) == m_vecMonsterIDs.end() )
		return;
	if( !RollChance( hSummonedMonster->GetRoom(), m_fProb ) )
		return;

	int nDur = ( m_nDurationTime == -1 ) ? -1 : ( ( m_nDurationTime > 0 ) ? m_nDurationTime : 60000 );
	for( size_t i = 0; i < m_vecSubBlows.size(); ++i )
	{
		hSummonedMonster->CmdAddStateEffect( &m_ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)m_vecSubBlows[i].nBlowIndex, nDur, m_vecSubBlows[i].strParam.c_str(), false, false );
	}
	if( hSummonedMonster->GetMaxHP() > hSummonedMonster->GetHP() )
		hSummonedMonster->SetHP( hSummonedMonster->GetMaxHP() );
}
#endif

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
// DNT format: "distRef;addRatio;stepDist;mode" (e.g. "300;0.10;100;0" or "0;0.10;100;0")
// =============================================================================
CDnAddDamageByTargetDistanceBlow::CDnAddDamageByTargetDistanceBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fRefDistance( 0.0f ), m_fAddDamageRatio( 0.0f ), m_fStepDistance( 100.0f ), m_nMode( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_293;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 1 ) m_fRefDistance = (float)atof( vlTokens[0].c_str() );
		if( vlTokens.size() >= 2 ) m_fAddDamageRatio = (float)atof( vlTokens[1].c_str() );
		if( vlTokens.size() >= 3 ) m_fStepDistance = (float)atof( vlTokens[2].c_str() );
		if( vlTokens.size() >= 4 ) m_nMode = atoi( vlTokens[3].c_str() );
		if( m_fStepDistance <= 0.0f ) m_fStepDistance = 100.0f;
		m_fValue = m_fAddDamageRatio;
	}
}
CDnAddDamageByTargetDistanceBlow::~CDnAddDamageByTargetDistanceBlow( void ) {}
void CDnAddDamageByTargetDistanceBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddDamageByTargetDistanceBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddDamageByTargetDistanceBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
float CDnAddDamageByTargetDistanceBlow::CalcDistanceDamageRatio( float fDistance ) const
{
	if( m_fAddDamageRatio <= 0.0f )
		return 0.0f;
	if( m_fRefDistance <= 0.0f )
	{
		// Mode: flat / close-range distance scaling when ref distance == 0
		return ( fDistance <= m_fStepDistance || m_nMode == 0 ) ? m_fAddDamageRatio : 0.0f;
	}
	if( m_nMode == 0 )
	{
		// Closer than refDistance (or within refDistance) grants bonus up to m_fAddDamageRatio
		if( fDistance <= m_fRefDistance )
			return m_fAddDamageRatio;
		float fOver = fDistance - m_fRefDistance;
		float fFactor = 1.0f - ( fOver / ( m_fStepDistance * 10.0f ) );
		return ( fFactor > 0.0f ) ? ( m_fAddDamageRatio * fFactor ) : 0.0f;
	}
	// Farther than refDistance grants bonus
	return ( fDistance >= m_fRefDistance ) ? m_fAddDamageRatio : 0.0f;
}

// =============================================================================
// BLOW_294: CDnAddDamageByWeightBlow
// DNT format: "weightScale;ratio" (e.g. "5;1.0") or single "ratio"
// =============================================================================
CDnAddDamageByWeightBlow::CDnAddDamageByWeightBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fWeightThreshold( 1.0f ), m_fWeightRatio( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_294;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 2 )
		{
			m_fWeightThreshold = (float)atof( vlTokens[0].c_str() );
			m_fWeightRatio = (float)atof( vlTokens[1].c_str() );
		}
		else
		{
			m_fWeightRatio = (float)atof( szValue );
		}
		m_fValue = m_fWeightRatio;
	}
}
CDnAddDamageByWeightBlow::~CDnAddDamageByWeightBlow( void ) {}
void CDnAddDamageByWeightBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) {}
void CDnAddDamageByWeightBlow::Process( LOCAL_TIME LocalTime, float fDelta ) { CDnBlow::Process( LocalTime, fDelta ); }
void CDnAddDamageByWeightBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
float CDnAddDamageByWeightBlow::CalcWeightDamageRatio( float fTargetWeight ) const
{
	if( m_fWeightRatio <= 0.0f || fTargetWeight <= 0.0f )
		return 0.0f;
	if( m_fWeightThreshold > 0.0f && fTargetWeight >= m_fWeightThreshold )
		return m_fWeightRatio;
	return ( fTargetWeight / ( m_fWeightThreshold > 0.0f ? m_fWeightThreshold : 1.0f ) ) * m_fWeightRatio;
}

// =============================================================================
// BLOW_295: CDnAddStateOnBreakSuperArmorBlow
// =============================================================================
CDnAddStateOnBreakSuperArmorBlow::CDnAddStateOnBreakSuperArmorBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fProb( 1.0f ), m_nDurationTime( 5000 ), m_fCoolTime( 0.0f ), m_fRemainCoolTime( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_295;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::string s( szValue );
		if( s.find( '[' ) != std::string::npos )
		{
			std::vector<int> unusedSkills;
			ParseAppointedSkillBlowFormat( szValue, m_fProb, m_nDurationTime, m_fCoolTime, m_vecSubBlows, unusedSkills );
		}
		else
		{
			ParseLegacySemiSubBlow( szValue, m_nDurationTime, m_vecSubBlows );
		}
	}
}
CDnAddStateOnBreakSuperArmorBlow::~CDnAddStateOnBreakSuperArmorBlow( void ) {}
void CDnAddStateOnBreakSuperArmorBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_fRemainCoolTime = 0.0f; }
void CDnAddStateOnBreakSuperArmorBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	if( m_fRemainCoolTime > 0.0f )
	{
		m_fRemainCoolTime -= fDelta;
		if( m_fRemainCoolTime < 0.0f ) m_fRemainCoolTime = 0.0f;
	}
}
void CDnAddStateOnBreakSuperArmorBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnAddStateOnBreakSuperArmorBlow::OnBreakSuperArmor( DnActorHandle hTargetActor )
{
	if( !m_hActor || m_hActor->IsDie() || m_vecSubBlows.empty() || m_fRemainCoolTime > 0.0f )
		return;
	if( !RollChance( m_hActor->GetRoom(), m_fProb ) )
		return;

	ApplySubBlowSpecs( m_hActor, &m_ParentSkillInfo, m_nDurationTime, m_vecSubBlows );
	m_fRemainCoolTime = m_fCoolTime;
}
#endif

// =============================================================================
// BLOW_296: CDnAddStateOnAppointedSkillBlow
// DNT format: "[100;10000;1000][251/0.20]_[402;412;417]"
// =============================================================================
CDnAddStateOnAppointedSkillBlow::CDnAddStateOnAppointedSkillBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fProb( 1.0f ), m_nDurationTime( 5000 ), m_fCoolTime( 0.0f ), m_fRemainCoolTime( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_296;
	SetValue( szValue );
#ifdef _GAMESERVER
	AddCallBackType( SB_ONBEGIN_SKILL );
#endif
	ParseAppointedSkillBlowFormat( szValue, m_fProb, m_nDurationTime, m_fCoolTime, m_vecSubBlows, m_vecAppointedSkillIDs );
}
CDnAddStateOnAppointedSkillBlow::~CDnAddStateOnAppointedSkillBlow( void ) {}
void CDnAddStateOnAppointedSkillBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_fRemainCoolTime = 0.0f; }
void CDnAddStateOnAppointedSkillBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	if( m_fRemainCoolTime > 0.0f )
	{
		m_fRemainCoolTime -= fDelta;
		if( m_fRemainCoolTime < 0.0f ) m_fRemainCoolTime = 0.0f;
	}
}
void CDnAddStateOnAppointedSkillBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnAddStateOnAppointedSkillBlow::OnBeginSkill( DnSkillHandle hSkill )
{
	if( !m_hActor || m_hActor->IsDie() || !hSkill || m_vecSubBlows.empty() || m_fRemainCoolTime > 0.0f )
		return;
	if( !m_vecAppointedSkillIDs.empty() &&
		std::find( m_vecAppointedSkillIDs.begin(), m_vecAppointedSkillIDs.end(), hSkill->GetClassID() ) == m_vecAppointedSkillIDs.end() )
		return;
	if( !RollChance( m_hActor->GetRoom(), m_fProb ) )
		return;

	ApplySubBlowSpecs( m_hActor, &m_ParentSkillInfo, m_nDurationTime, m_vecSubBlows );
	m_fRemainCoolTime = m_fCoolTime;
}
#endif

// =============================================================================
// BLOW_297: CDnAddStateOnAppointedSkillHitBlow
// DNT format: "[100;0;0][294/5;1.0]_[2401;2405;2553]"
// =============================================================================
CDnAddStateOnAppointedSkillHitBlow::CDnAddStateOnAppointedSkillHitBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fProb( 1.0f ), m_nDurationTime( 5000 ), m_fCoolTime( 0.0f ), m_fRemainCoolTime( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_297;
	SetValue( szValue );
#ifdef _GAMESERVER
	AddCallBackType( SB_ONTARGETHIT );
#endif
	ParseAppointedSkillBlowFormat( szValue, m_fProb, m_nDurationTime, m_fCoolTime, m_vecSubBlows, m_vecAppointedSkillIDs );
}
CDnAddStateOnAppointedSkillHitBlow::~CDnAddStateOnAppointedSkillHitBlow( void ) {}
void CDnAddStateOnAppointedSkillHitBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_fRemainCoolTime = 0.0f; }
void CDnAddStateOnAppointedSkillHitBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	if( m_fRemainCoolTime > 0.0f )
	{
		m_fRemainCoolTime -= fDelta;
		if( m_fRemainCoolTime < 0.0f ) m_fRemainCoolTime = 0.0f;
	}
}
void CDnAddStateOnAppointedSkillHitBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnAddStateOnAppointedSkillHitBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	if( !m_hActor || m_hActor->IsDie() || !hTargetActor || m_vecSubBlows.empty() || m_fRemainCoolTime > 0.0f )
		return;

	int nHitSkillID = 0;
	if( m_hActor->IsProcessSkill() && m_hActor->GetProcessSkill() )
		nHitSkillID = m_hActor->GetProcessSkill()->GetClassID();

	if( !m_vecAppointedSkillIDs.empty() )
	{
		if( nHitSkillID <= 0 ||
			std::find( m_vecAppointedSkillIDs.begin(), m_vecAppointedSkillIDs.end(), nHitSkillID ) == m_vecAppointedSkillIDs.end() )
			return;
	}

	if( !RollChance( m_hActor->GetRoom(), m_fProb ) )
		return;

	ApplySubBlowSpecs( m_hActor, &m_ParentSkillInfo, m_nDurationTime, m_vecSubBlows );
	m_fRemainCoolTime = m_fCoolTime;
}
#endif

// =============================================================================
// BLOW_299: CDnDamageTransitionToSummonMonsterBlow
// =============================================================================
CDnDamageTransitionToSummonMonsterBlow::CDnDamageTransitionToSummonMonsterBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_fTransitionRatio( 0.0f ), m_nSummonMonsterID( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_299;
	SetValue( szValue );
#ifdef _GAMESERVER
	AddCallBackType( SB_ONCALCDAMAGE );
#endif
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
#ifdef _GAMESERVER
float CDnDamageTransitionToSummonMonsterBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	if( !m_hActor || fOriginalDamage <= 0.0f || m_fTransitionRatio <= 0.0f )
		return 0.0f;

	DnMonsterActorHandle hTargetSummon;
	const std::list<DnMonsterActorHandle>& listSummoned = m_hActor->GetSummonedMonsterList();
	for( std::list<DnMonsterActorHandle>::const_iterator it = listSummoned.begin(); it != listSummoned.end(); ++it )
	{
		DnMonsterActorHandle hMon = *it;
		if( hMon && !hMon->IsDie() && ( m_nSummonMonsterID == 0 || hMon->GetMonsterClassID() == m_nSummonMonsterID ) )
		{
			hTargetSummon = hMon;
			break;
		}
	}
	if( !hTargetSummon )
	{
		const std::map<int, std::list<DnMonsterActorHandle> >& mapGrouped = m_hActor->GetGroupingSummonedMonsterList();
		for( std::map<int, std::list<DnMonsterActorHandle> >::const_iterator mit = mapGrouped.begin(); mit != mapGrouped.end() && !hTargetSummon; ++mit )
		{
			for( std::list<DnMonsterActorHandle>::const_iterator it = mit->second.begin(); it != mit->second.end(); ++it )
			{
				DnMonsterActorHandle hMon = *it;
				if( hMon && !hMon->IsDie() && ( m_nSummonMonsterID == 0 || hMon->GetMonsterClassID() == m_nSummonMonsterID ) )
				{
					hTargetSummon = hMon;
					break;
				}
			}
		}
	}
	if( !hTargetSummon )
		return 0.0f;

	int nTransferDmg = (int)( fOriginalDamage * m_fTransitionRatio );
	if( nTransferDmg <= 0 )
		return 0.0f;

	char szBuf[64] = { 0 };
	_itoa_s( nTransferDmg, szBuf, sizeof( szBuf ), 10 );
	hTargetSummon->CmdAddStateEffect( &m_ParentSkillInfo, STATE_BLOW::BLOW_177, 0, szBuf, false, false );
	return -(float)nTransferDmg;
}
#endif

// =============================================================================
// BLOW_300: CDnAddStateOnBlockBlow
// DNT format: "[10000;20000][2/0.20]" -> [durationMs;coolTimeMs][blow/val]
// =============================================================================
CDnAddStateOnBlockBlow::CDnAddStateOnBlockBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nDurationTime( 5000 ), m_fCoolTime( 0.0f ), m_fRemainCoolTime( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_300;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::string s( szValue );
		if( s.find( '[' ) != std::string::npos )
		{
			std::vector<std::string> brackets;
			ExtractBrackets( s, brackets );
			if( !brackets.empty() )
			{
				std::vector<std::string> hdr;
				TokenizeA( brackets[0], hdr, ";" );
				if( hdr.size() >= 1 ) m_nDurationTime = atoi( hdr[0].c_str() );
				if( hdr.size() >= 2 ) m_fCoolTime = (float)atoi( hdr[1].c_str() ) / 1000.0f;
				for( size_t i = 1; i < brackets.size(); ++i )
				{
					SExtendedSubBlowSpec spec;
					if( ParseSlashSubBlow( brackets[i], spec ) )
						m_vecSubBlows.push_back( spec );
				}
			}
		}
		else
		{
			ParseLegacySemiSubBlow( szValue, m_nDurationTime, m_vecSubBlows );
		}
	}
}
CDnAddStateOnBlockBlow::~CDnAddStateOnBlockBlow( void ) {}
void CDnAddStateOnBlockBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta ) { m_fRemainCoolTime = 0.0f; }
void CDnAddStateOnBlockBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	if( m_fRemainCoolTime > 0.0f )
	{
		m_fRemainCoolTime -= fDelta;
		if( m_fRemainCoolTime < 0.0f ) m_fRemainCoolTime = 0.0f;
	}
}
void CDnAddStateOnBlockBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta ) {}
#ifdef _GAMESERVER
void CDnAddStateOnBlockBlow::OnBlockSuccess( void )
{
	if( !m_hActor || m_hActor->IsDie() || m_vecSubBlows.empty() || m_fRemainCoolTime > 0.0f )
		return;

	ApplySubBlowSpecs( m_hActor, &m_ParentSkillInfo, m_nDurationTime, m_vecSubBlows );
	m_fRemainCoolTime = m_fCoolTime;
}
#endif

// =============================================================================
// BLOW_301: CDnGaugeShieldBlow
// DNT format: "mode;maxGauge;breakSkillID" (e.g. "1;7800000;34143") or single "maxGauge"
// =============================================================================
CDnGaugeShieldBlow::CDnGaugeShieldBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor ), m_nMode( 0 ), m_nMaxGauge( 0 ), m_nCurGauge( 0 ), m_nBreakSkillID( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_301;
	SetValue( szValue );
	if( szValue && szValue[0] != '\0' )
	{
		std::vector<std::string> vlTokens;
		TokenizeA( std::string( szValue ), vlTokens, ";" );
		if( vlTokens.size() >= 2 )
		{
			m_nMode = atoi( vlTokens[0].c_str() );
			m_nMaxGauge = _atoi64( vlTokens[1].c_str() );
			if( vlTokens.size() >= 3 )
				m_nBreakSkillID = atoi( vlTokens[2].c_str() );
		}
		else
		{
			m_nMaxGauge = _atoi64( szValue );
		}
	}
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
	if( m_nCurGauge > nDmg )
	{
		m_nCurGauge -= nDmg;
		return -fOriginalDamage;
	}

	float fAbsorbed = (float)m_nCurGauge;
	m_nCurGauge = 0;
	SetState( STATE_BLOW::STATE_END );
	if( m_hActor )
	{
		m_hActor->CmdRemoveStateEffectFromID( GetBlowID() );
		if( m_nBreakSkillID > 0 && !m_hActor->IsDie() && m_hActor->FindSkill( m_nBreakSkillID ) )
			m_hActor->UseSkill( m_nBreakSkillID, false, true );
	}
	return -fAbsorbed;
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
void CDnDuplicateExplosionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	if( !m_hActor || m_hActor->IsDie() || m_nTargetBlowIndex <= 0 || m_nTargetBlowIndex >= STATE_BLOW::BLOW_MAX )
		return;

	STATE_BLOW::emBLOW_INDEX targetIdx = (STATE_BLOW::emBLOW_INDEX)m_nTargetBlowIndex;
	if( !m_hActor->IsAppliedThisStateBlow( targetIdx ) )
		return;

	DNVector(DnBlowHandle) vlTargetBlows;
	m_hActor->GatherAppliedStateBlowByBlowIndex( targetIdx, vlTargetBlows );
	int nStacks = 0;
	for( DWORD i = 0; i < vlTargetBlows.size(); ++i )
	{
		if( vlTargetBlows[i] )
			nStacks += ( vlTargetBlows[i]->GetDuplicateCount() > 0 ) ? vlTargetBlows[i]->GetDuplicateCount() : 1;
	}
	m_hActor->CmdRemoveStateEffect( targetIdx );

	if( nStacks > 0 && m_fDamageRatio > 0.0f && m_ParentSkillInfo.hSkillUser )
	{
		float fBaseAtk = (float)max( m_ParentSkillInfo.hSkillUser->GetAttackPMax(), m_ParentSkillInfo.hSkillUser->GetAttackMMax() );
		int nBurstDmg = (int)( fBaseAtk * m_fDamageRatio * (float)nStacks );
		if( nBurstDmg > 0 )
		{
			char szBuf[64] = { 0 };
			_itoa_s( nBurstDmg, szBuf, sizeof( szBuf ), 10 );
			m_hActor->CmdAddStateEffect( &m_ParentSkillInfo, STATE_BLOW::BLOW_177, 0, szBuf, false, false );
		}
	}
#endif
}
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
