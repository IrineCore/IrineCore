#include "stdafx.h"
#include "DnPartyFinishAttackBlow.h"
#include "DnSkill.h"
#include "DnProjectile.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnPartyFinishAttackBlow::CDnPartyFinishAttackBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_278;
	SetValue( szValue );

#if defined(_GAMESERVER)
	m_fEnemyHpRatio = 0.f;
	m_fIncreaseDamagdRatio = 0.f;
	m_fDamageRatio = 0.f;
	m_fDamageAbsolute = 0.f;
	m_fDamageRatioMax = 0.f;

	std::string str = szValue;
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	TokenizeA(str, tokens, delimiters);
	if( 5 == tokens.size())
	{
		m_fEnemyHpRatio = (float)atof( tokens[0].c_str() );
		m_fIncreaseDamagdRatio = (float)atof( tokens[1].c_str() );
		m_fDamageRatio = (float)atof( tokens[2].c_str() );
		m_fDamageAbsolute = (float)atof( tokens[3].c_str() );
		m_fDamageRatioMax = (float)atof( tokens[4].c_str() );
	}

	AddCallBackType( SB_ONTARGETHIT );
#endif
}

CDnPartyFinishAttackBlow::~CDnPartyFinishAttackBlow(void)
{

}

void CDnPartyFinishAttackBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);
}


void CDnPartyFinishAttackBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process(LocalTime, fDelta);
}


void CDnPartyFinishAttackBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);
}

#if defined(_GAMESERVER)

void CDnPartyFinishAttackBlow::OnSetParentSkillInfo()
{
	if( m_hActor )
		m_ParentSkillInfo.hSkillUser = m_hActor;
}

void CDnPartyFinishAttackBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	if (!hTargetActor || hTargetActor->IsDie())
		return;

	CDnDamageBase::SHitParam* pHitParam = hTargetActor->GetHitParam();
	if( pHitParam )
	{
		DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
		if( hActor && hActor->IsDie() == false )
		{
			if( hTargetActor->GetHPPercentFloat() < m_fEnemyHpRatio )
			{
				int nIncreaseRatio = 1 + (int)( ( m_fEnemyHpRatio - hTargetActor->GetHPPercentFloat() ) / m_fIncreaseDamagdRatio );
				if( nIncreaseRatio > m_fDamageRatioMax )
					nIncreaseRatio = (int)m_fDamageRatioMax;

				int nResultDamage = (int)(nIncreaseRatio * ( ( pHitParam->nCalcDamage * m_fDamageRatio ) + m_fDamageAbsolute ));

				if( nResultDamage > 0 )
				{
					char buffer[65];
					_itoa_s(nResultDamage, buffer, 65, 10 );
					hTargetActor->CmdAddStateEffect( &m_ParentSkillInfo, STATE_BLOW::BLOW_177, 0, buffer );
				}
			}
		}
	}
}
#endif // _GAMESERVER