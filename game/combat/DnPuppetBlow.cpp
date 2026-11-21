#include "stdafx.h"
#include "DnPuppetBlow.h"
#if defined(_GAMESERVER)
#include "DnGameTask.h"
#include "TaskManager.h"
#include "DnMonsterActor.h"
#ifdef PRE_ADD_DECREASE_EFFECT
#include "DnStateBlow.h"
#endif // PRE_ADD_DECREASE_EFFECT
#endif // _GAMESERVER

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnPuppetBlow::CDnPuppetBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_247;

	SetValue( szValue );
	//m_fValue = (float)atof(szValue);
	m_fLimitRange = -1.0f;

	std::string str = szValue;
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	TokenizeA(str, tokens, delimiters);
	if (tokens.size() >= 2)
	{
		m_fValue = (float)atof(tokens[0].c_str());	//Monster ID
		m_DamageChangeRateArg = tokens[1];	//Damage ratio

		if (tokens.size() >= 3)
			m_fLimitRange = (float)atof(tokens[2].c_str());
	}
	else
	{
// 		m_fValue = 33501.0f;
// 		m_DamageChangeRateArg = "1.2";
		OutputDebug("%s Invalidvalue.... %s\n", __FUNCTION__, szValue);
	}

#if defined(_GAMESERVER)
	m_vDeltaPos = EtVector3(1.0f, 0.0f, 1.0f);
#endif // _GAMESERVER
}

CDnPuppetBlow::~CDnPuppetBlow(void)
{
}

#ifdef _GAMESERVER
bool CDnPuppetBlow::CanBegin( void )
{
	//Puppet state effect can only be applied when Brand state effect is present.
	if (m_hActor && m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_246) == true)
		return true;
	else
		return false;
}
#endif

void CDnPuppetBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);

#if defined(_GAMESERVER)
	SummonMonsterStruct Struct;
	memset(&Struct, 0, sizeof(Struct));
	Struct.MonsterID = (int)m_fValue;
	Struct.vPosition = &m_vDeltaPos;

	//Set puppet summon duration to the Puppet state effect duration
	Struct.nLifeTime = (int)(m_StateBlow.fDurationTime*1000.0f);

	//Team must be set to 0 here so RequestSummonMonster sets it to the Owner's team.
	//Struct.nTeam = m_hActor->GetTeam();

	Struct.bCopySummonerState = true;
	Struct.fHPAdjustValue = 1.0f;

	Struct.bSuicideWhenSummonerDie = TRUE;	//Set to disappear together when the summoning actor dies.
	Struct.bFollowSummonerStage = FALSE;	//Set to follow the summoned actor

	CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));
	
	if( pGameTask )
		m_hSummonMonster = pGameTask->RequestSummonMonsterBySkill( m_hActor, m_ParentSkillInfo.hSkillUser, &Struct );

	//Add physical/magic damage modification state effect..
	if (m_hSummonMonster)
	{
		int nDurationTime = (int)(m_StateBlow.fDurationTime * 1000.0f);
		const char* szParam = m_DamageChangeRateArg.c_str();

		m_hSummonMonster->CmdAddStateEffect(&m_ParentSkillInfo, STATE_BLOW::BLOW_134, nDurationTime, szParam);
		m_hSummonMonster->CmdAddStateEffect(&m_ParentSkillInfo, STATE_BLOW::BLOW_135, nDurationTime, szParam);


		//Called from pGameTask->RequestSummonMonsterBySkill above, but keep here for now.
		m_hSummonMonster->SetPuppetSummonMonster(true);
	}
#ifdef PRE_ADD_DECREASE_EFFECT
		if( m_hSummonMonster && m_bShowReduce )
		{
			m_hSummonMonster->SendAddSEFail( CDnStateBlow::ADD_DECREASE_EFFECT_BY_IMMUNE, m_StateBlow.emBlowIndex );
			m_bShowReduce = false;
		}
#endif // PRE_ADD_DECREASE_EFFECT
#endif // _GAMESERVER

	OutputDebug( "%s\n", __FUNCTION__ );
}

void CDnPuppetBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process( LocalTime, fDelta );

#if defined(_GAMESERVER)
	if (m_fLimitRange >= 0.0f)
	{
		if (m_hActor && m_hSummonMonster)
		{
#if defined(PRE_FIX_61438)
			SSphere actorSphere;
			SSphere summonActorSphere;

			//For dragons, the bounding info can be set huge.
			//Change to the value from the actual actor table.
			m_hActor->GetBoundingSphere(actorSphere, true);
			m_hSummonMonster->GetBoundingSphere(summonActorSphere, true);
			
			EtVector2 vVec;
			vVec.x = summonActorSphere.Center.x - actorSphere.Center.x;
			vVec.y = summonActorSphere.Center.z - actorSphere.Center.z;

			float fLength = EtVec2Length(&vVec);
			if (summonActorSphere.fRadius + actorSphere.fRadius + m_fLimitRange < fLength)
			{
				SetState(STATE_BLOW::STATE_END);

				//Need to send delete packet to client..
				m_hActor->CmdRemoveStateEffectFromID(GetBlowID());
			}

#else
			EtVector3 ownerActorPos = *(m_hActor->GetPosition());
			EtVector3 puppetActorPos = *(m_hSummonMonster->GetPosition());

			EtVector2 vVec;
			vVec.x = puppetActorPos.x - ownerActorPos.x;
			vVec.y = puppetActorPos.z - ownerActorPos.z;

			float fLength = EtVec2Length(&vVec);

			//End state effect if out of configured range..
			if( m_fLimitRange < fLength )
			{
				SetState( STATE_BLOW::STATE_END );
				
				//Need to send delete packet to client..
				m_hActor->CmdRemoveStateEffectFromID(GetBlowID());
			}
#endif // PRE_FIX_61438
		}
	}
#endif // _GAMESERVER

}

void CDnPuppetBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);

#if defined(_GAMESERVER)
	if (m_hSummonMonster)
		m_hSummonMonster->CmdSuicide(false, false);
#endif // _GAMESERVER

	OutputDebug( "%s\n", __FUNCTION__);
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnPuppetBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnPuppetBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW