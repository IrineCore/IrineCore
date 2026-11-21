#include "stdafx.h"
#include "DnDissolveBufBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnDissolveBufBlow::CDnDissolveBufBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_083;
	SetValue( szValue );
	//m_nValue = atoi( szValue );
	m_fValue = (float)atof(szValue);
}

CDnDissolveBufBlow::~CDnDissolveBufBlow(void)
{
}


void CDnDissolveBufBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	// Dispel unit is by debuff skill group, not by state effect count.
	multimap<int, DnBlowHandle> mmapBuffBlowsBySkill;

	int iDissolveCount = 0;
	int iNumAppliedBlow = m_hActor->GetNumAppliedStateBlow();
	vector<int> vlBuffSkillID;
	for( int iBlow = 0; iBlow < iNumAppliedBlow; ++iBlow )
	{
		DnBlowHandle hBlow = m_hActor->GetAppliedStateBlow( iBlow );
		if( !hBlow )
			continue;

		const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();

		// Arbitrarily applied infinite state effects are not targets for removal.
		if( hBlow->IsPermanent() )
			continue;

		if( pSkillInfo && pSkillInfo->iSkillID )
		{
			// Beneficial state effects. Remove skills designated as buffs.
			// Except for those marked as passive buffs.
			if( (0 == pSkillInfo->iDissolvable) && 
				(CDnSkill::Passive != pSkillInfo->eSkillType ) &&
				(CDnSkill::Buff == pSkillInfo->eDurationType) )
			{
				// Specific skill evasion state blows used by origin items are not removed by buff-removal state blows.
				// Must be handled separately if used generally in the future.
				if( hBlow->IsFromSourceItem() )
					continue;

				if( hBlow->IsEternity() )
					continue;

				mmapBuffBlowsBySkill.insert( make_pair(pSkillInfo->iSkillID, hBlow) );

				// A single skill can have multiple state effects. Insert only unique skill IDs.
				if( vlBuffSkillID.end() == find( vlBuffSkillID.begin(), vlBuffSkillID.end(), pSkillInfo->iSkillID ) )
					vlBuffSkillID.push_back( pSkillInfo->iSkillID );
			}
		}
	}

	DNVector(int) vlAppliedBuffBlowIDs;
	for( int i = 0; i < (int)vlBuffSkillID.size(); ++i )
	{
		if( iDissolveCount >= (int)m_fValue )
			break;

		pair<multimap<int, DnBlowHandle>::iterator, multimap<int, DnBlowHandle>::iterator> iter_pair = 
			mmapBuffBlowsBySkill.equal_range( vlBuffSkillID.at(i) );

		// Collect debuff state effects originating from the same skill.
		multimap<int, DnBlowHandle>::iterator iter = iter_pair.first;
		for( iter; iter != iter_pair.second; ++iter )
			vlAppliedBuffBlowIDs.push_back( iter->second->GetBlowID() );

		++iDissolveCount;
	}


	// Dispel only dispellable non-passive buff state effects
	int iNumBlow = (int)vlAppliedBuffBlowIDs.size();
	for( int iBlow = 0; iBlow < iNumBlow; ++iBlow )
	{
		int iBlowID = vlAppliedBuffBlowIDs.at( iBlow );
		m_hActor->RemoveStateBlowFromID( iBlowID );

		//// Note: Removed by defined state effect index, so there can be more than one of the same type.
		//// Therefore, validate hBlow as it may have already been deleted in a previous loop.
		//if( hBlow )
		//{
		//	m_hActor->RemoveStateBlowByBlowDefineIndex( hBlow->GetBlowIndex() );
		//}
	}

	OutputDebug( "CDnDissolveBufBlow::OnBegin, Value:%d \n", (int)m_fValue );
}


void CDnDissolveBufBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnDissolveBufBlow::OnEnd\n");
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDissolveBufBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnDissolveBufBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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
