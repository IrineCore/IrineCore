#include "stdafx.h"
#include "DnDissolveDebufBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnDissolveDebufBlow::CDnDissolveDebufBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_069;
	SetValue( szValue );
	//m_nValue = atoi( szValue );
	m_fValue = (float)atof(szValue);
}

CDnDissolveDebufBlow::~CDnDissolveDebufBlow(void)
{
}

bool CDnDissolveDebufBlow::_IsBadStateBlow( DnBlowHandle hBlow )
{
	return hBlow->GetBlowIndex() == STATE_BLOW::BLOW_041 ||
		   hBlow->GetBlowIndex() == STATE_BLOW::BLOW_042 ||
		   hBlow->GetBlowIndex() == STATE_BLOW::BLOW_043 ||
		   hBlow->GetBlowIndex() == STATE_BLOW::BLOW_044 ||
		   hBlow->GetBlowIndex() == STATE_BLOW::BLOW_045 || // Sleep
		   hBlow->GetBlowIndex() == STATE_BLOW::BLOW_078 ||	// Skill disable state effect
		   hBlow->GetBlowIndex() == STATE_BLOW::BLOW_070 ||
		   hBlow->GetBlowIndex() == STATE_BLOW::BLOW_071;
}


void CDnDissolveDebufBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{

#ifdef _GAMESERVER
	// Dispel unit is by debuff skill group, not by state effect count.
	multimap<int, DnBlowHandle> mmapDebuffBlowsBySkill;

	int iDissolveCount = 0;
	int iNumAppliedBlow = m_hActor->GetNumAppliedStateBlow();
	vector<int> vlDebuffSkillID;
	for( int iBlow = 0; iBlow < iNumAppliedBlow; ++iBlow )
	{
		DnBlowHandle hBlow = m_hActor->GetAppliedStateBlow( iBlow );
		if( !hBlow )
			continue;

		CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());

		if( pSkillInfo && pSkillInfo->iSkillID )
		{
			// Negative state effects: electrocution, poison, burn, stun/disable, root, or skills flagged as debuffs
			if( (0 == pSkillInfo->iDissolvable) && 
				(CDnSkill::Debuff == pSkillInfo->eDurationType || _IsBadStateBlow(hBlow)) )
			{
				// If it is a debuff and not a designated harmful state effect, team checks must be considered.
				bool bBadBlow = true;
				if( (pSkillInfo->hSkillUser) && (false == _IsBadStateBlow(hBlow)) )
				{
					bBadBlow = false;
					bool bOtherTeam = (m_hActor->GetTeam() != pSkillInfo->hSkillUser->GetTeam());
					if( (CDnSkill::Enemy == pSkillInfo->eTargetType) && bOtherTeam )
					{
						bBadBlow = true;
					}

					// Case where skill target type is All (like Bubble Bubble) and each state effect has its own target.
					if( (CDnSkill::All == pSkillInfo->eTargetType) && bOtherTeam )
					{
						DnSkillHandle hSkill = pSkillInfo->hSkillUser->FindSkill( pSkillInfo->iSkillID );
						if( hSkill )
						{
							DWORD dwNumSE = hSkill->GetStateEffectCount();
							for( DWORD dwSE = 0; dwSE < dwNumSE; ++dwSE )
							{
								const CDnSkill::StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( dwSE );
								if( pSE && pSE->ApplyType == CDnSkill::StateEffectApplyType::ApplyEnemy )
								{
									if( (STATE_BLOW::emBLOW_INDEX)pSE->nID == hBlow->GetBlowIndex() )
									{
										bBadBlow = true;
										break;
									}
								}
							}
						}
					}
				}

				if( bBadBlow )
				{
					mmapDebuffBlowsBySkill.insert( make_pair(pSkillInfo->iSkillID, hBlow) );

					// A single skill can have multiple state effects. Insert only unique skill IDs.
					if( vlDebuffSkillID.end() == find( vlDebuffSkillID.begin(), vlDebuffSkillID.end(), pSkillInfo->iSkillID ) )
						vlDebuffSkillID.push_back( pSkillInfo->iSkillID );
				}
			}
		}
	}

	DNVector(int) vlAppliedDebuffBlowIds;
	for( int i = 0; i < (int)vlDebuffSkillID.size(); ++i )
	{
		if( iDissolveCount >= (int)m_fValue )
			break;

		pair<multimap<int, DnBlowHandle>::iterator, multimap<int, DnBlowHandle>::iterator> iter_pair = 
			mmapDebuffBlowsBySkill.equal_range( vlDebuffSkillID.at(i) );

		// Collect debuff state effects originating from the same skill.
		multimap<int, DnBlowHandle>::iterator iter = iter_pair.first;
		for( iter; iter != iter_pair.second; ++iter )
			vlAppliedDebuffBlowIds.push_back( iter->second->GetBlowID() );

		++iDissolveCount;
	}


	// Dispel only dispellable harmful skills and state effects
	int iNumBlow = (int)vlAppliedDebuffBlowIds.size();
	for( int iBlow = 0; iBlow < iNumBlow; ++iBlow )
	{
		int iBlowID = vlAppliedDebuffBlowIds.at( iBlow );
		m_hActor->CmdRemoveStateEffectFromID( iBlowID );
	}
#endif
}


void CDnDissolveDebufBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{

}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDissolveDebufBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	fValue[0] = (float)atof( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	fValue[1] = (float)atof( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] + fValue[1];

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}

void CDnDissolveDebufBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	fValue[0] = (float)atof( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	fValue[1] = (float)atof( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Calculate value
	float fResultValue = fValue[0] - fValue[1];

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
