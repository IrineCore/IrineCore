#include "stdafx.h"
#include "DnTransformMonsterBlow.h"
#include "DnMonsterActor.h"


CDnTransformMonsterBlow::CDnTransformMonsterBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_277;

	m_nTransformActorID = 0;
	m_nOriginalActorID = 0;

	SetValue( szValue );

	std::string str = szValue;
	std::vector<std::string> tokens;

	TokenizeA( str, tokens,  ";" );
	
	if( tokens.size() > 0 )
		m_nTransformActorID = atoi( tokens[0].c_str() );

#if defined(_GAMESERVER)
	m_bOrderUseSkill = false;
	m_nTransformSkillID = 0;

	if( tokens.size() > 1 )
		m_nTransformSkillID = atoi( tokens[1].c_str() );
#endif

}

CDnTransformMonsterBlow::~CDnTransformMonsterBlow(void)
{

}

void CDnTransformMonsterBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{

#ifdef PRE_ADD_TRANSFORM_MONSTER_ACTOR
	if(m_hActor && m_hActor->IsMonsterActor() )
	{
		CDnMonsterActor *pMonster = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
		m_nOriginalActorID = pMonster->GetActorTableID();
		pMonster->SwapActor( m_nTransformActorID ); 
#if defined(_GAMESERVER)
		m_bOrderUseSkill = true;
#endif
	}
#endif

}

void CDnTransformMonsterBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef PRE_ADD_TRANSFORM_MONSTER_ACTOR
#if defined(_GAMESERVER)
	if( m_bOrderUseSkill == true )
	{
		if( m_nTransformSkillID > 0 )
		{
			if( m_hActor )
				m_hActor->UseSkill( m_nTransformSkillID, false );
		}
		m_bOrderUseSkill = false;
	}
#endif
#endif
}

void CDnTransformMonsterBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	// Because monsters have AI and various other conditions 
	// Have the AI manage transformation situations.
	// Set not to return upon termination.
	// This part may change in the future.
}
