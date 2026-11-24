#include "stdafx.h"
#include "DnProbInvincibleAtBlow.h"
#include "DnProjectile.h"

#ifdef _GAMESERVER
#include "DnMonsterActor.h"
#include "DnProjectile.h"
#else
#include "DnStateBlow.h"
#endif // #ifdef _GAMESERVER

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnProbInvincibleAtBlow::CDnProbInvincibleAtBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_226;
	SetValue( szValue );

#ifdef _GAMESERVER
	AddCallBackType( SB_ONCALCDAMAGE );
	AddInvincibleAt(szValue);
	m_pNowMapSelectedSkillInfos = NULL;
	m_bNowSelectedSkillUsedProjectile = false;
#else
	m_pEffectOutputInfo = CDnStateBlow::GetEffectOutputInfo( 100 );		// Source item state effect in StateEffectTable is designated as 100.
#endif // #ifdef _GAMESERVER

	m_fValue = 0.0f;
}

CDnProbInvincibleAtBlow::~CDnProbInvincibleAtBlow(void)
{
}

void CDnProbInvincibleAtBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}

void CDnProbInvincibleAtBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#ifdef _GAMESERVER
	CheckAllTargetSkillState();
	CheckProbFailedSkillState();
#endif // #ifdef _GAMESERVER
}


void CDnProbInvincibleAtBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}

#ifdef _GAMESERVER
void CDnProbInvincibleAtBlow::AddInvincibleAt(const char* szValue)
{	
	string str = szValue;
	vector<std::string> vlTokens;
	string delimiters = ";";

	// Target monster ID to evade;[Target skill ID1][Target skill prob1][Target skill ID2][Target skill prob2];...
	TokenizeA( str, vlTokens, delimiters );
	_ASSERT( vlTokens.size()%2 == 0 );
	for( int i = 0; i < (int)vlTokens.size(); i += 2 )
	{
		int iMonsterID = atoi( vlTokens.at(i).c_str() );
		S_INVINCIBLE_INFO& Info = m_mapInvincibleAt[ iMonsterID ];		// Per request to allow duplicate monster IDs...
		Info.iMonsterID = iMonsterID;
		vector<string> vlSkillInfos;
		TokenizeA( vlTokens.at(i+1), vlSkillInfos, string("["), string("]"), false );
		for( int k = 0; k < (int)vlSkillInfos.size(); k += 2 )
		{
			int iSkillID = atoi( vlSkillInfos.at( k ).c_str() );
			float fProb = (float)atof( vlSkillInfos.at( k+1 ).c_str() ) * 100.0f;
			Info.mapSkillInfos.insert( make_pair(iSkillID, fProb) );
		}
	}
}

void CDnProbInvincibleAtBlow::BeginInvincibleAt( DnSkillHandle hTargetSkill )
{
	if( !hTargetSkill )
		return;

	int iSkillID = hTargetSkill->GetClassID();
	if( false == IsBeginInvincible(iSkillID) )
	{
		int iSkillID = hTargetSkill->GetClassID();
		if( 0 < m_pNowMapSelectedSkillInfos->count( iSkillID ) )
		{
			// Probability check.
			int iProb = int((*m_pNowMapSelectedSkillInfos)[ iSkillID ] * 100.0f);
			if( _rand(m_hActor->GetRoom()) % 10000 <= iProb )
			{
				// Start ignoring this skill target.
				//m_hTargetSkill = hTargetSkill;
				S_SKILL_INFO TargetSkillInfo;
				TargetSkillInfo.hTargetSkill = hTargetSkill;
				TargetSkillInfo.bTargetSkillUsedProjectile = m_bNowSelectedSkillUsedProjectile;
				m_mapTargetSkillUsedProjectile[ iSkillID ] = m_bNowSelectedSkillUsedProjectile;
				m_listTargetSkills.push_back( TargetSkillInfo );
			}
			else
			{
				// If the probability check fails, do not re-check until the skill ends.
				m_mapTargetSkillUsedProjectile[ iSkillID ] = m_bNowSelectedSkillUsedProjectile;
				m_dqProbInvincibleFailedSkills.push_back( hTargetSkill );
			}
		}
	}
}

void CDnProbInvincibleAtBlow::EndInvincibleAt( DnSkillHandle hTargetSkill )
{
	if( !hTargetSkill )
		return;

	for( list<S_SKILL_INFO>::iterator iter = m_listTargetSkills.begin(); 
		m_listTargetSkills.end() != iter; )
	{
		S_SKILL_INFO& TargetSkillInfo = (*iter);	
		if( (!TargetSkillInfo.hTargetSkill) || 
			(TargetSkillInfo.hTargetSkill == hTargetSkill) )
		{
			m_mapTargetSkillUsedProjectile.erase( hTargetSkill->GetClassID() );
			iter = m_listTargetSkills.erase( iter );
		}
		else
		{
			++iter;
		}
	}
}

bool CDnProbInvincibleAtBlow::IsBeginInvincible( int iSkillID )
{
	bool bResult = false;

	bResult = (FindInvincibleTargetSkill(iSkillID) != NULL);

	return bResult;
}

CDnProbInvincibleAtBlow::S_SKILL_INFO* CDnProbInvincibleAtBlow::FindInvincibleTargetSkill( int iSkillID )
{
	S_SKILL_INFO* pResult = NULL;

	for( list<S_SKILL_INFO>::iterator iter = m_listTargetSkills.begin(); 
		m_listTargetSkills.end() != iter; )
	{
		S_SKILL_INFO& TargetSkillInfo = (*iter);
		if( TargetSkillInfo.hTargetSkill && 
			TargetSkillInfo.hTargetSkill->GetClassID() == iSkillID )
		{
			pResult = &TargetSkillInfo;
			break;
		}
		else
		if( !TargetSkillInfo.hTargetSkill )
		{
			m_mapTargetSkillUsedProjectile.erase( iSkillID );
			iter = m_listTargetSkills.erase( iter );
			continue;
		}

			++iter;
	}

	return pResult;
}

bool CDnProbInvincibleAtBlow::IsInvincibleAt( DnSkillHandle hTargetSkill )
{
	if( NULL == m_pNowMapSelectedSkillInfos )
		return false;

	if( !hTargetSkill )
		return false;

	bool bResult = false;
	int iSkillID = hTargetSkill->GetClassID();

	if( false == IsProbFailedSkillObject( hTargetSkill ) )
	{
		CheckTargetSkillStateAndDoEnd( hTargetSkill );

		if( false == IsBeginInvincible( iSkillID ) )
			BeginInvincibleAt( hTargetSkill );

		S_SKILL_INFO* pResult = FindInvincibleTargetSkill( iSkillID );
		if( pResult )
		{	
			if( pResult->hTargetSkill == hTargetSkill )
				bResult = true;
		}
	}

	return bResult;
}

bool CDnProbInvincibleAtBlow::IsProbFailedSkillObject( DnSkillHandle hSkill )
{
	bool bResult = false;

	// Once a skill fails the probability check, do not re-check invincibility probability until the skill ends.
	deque<DnSkillHandle>::iterator iter = find( m_dqProbInvincibleFailedSkills.begin(), m_dqProbInvincibleFailedSkills.end(), hSkill );
	if( iter != m_dqProbInvincibleFailedSkills.end() )
	{
		if( IsSkillFinished( hSkill ) )
		{
			m_dqProbInvincibleFailedSkills.erase( iter );
		}
		else
		{
			bResult = true;
		}
	}

	return bResult;
}

void CDnProbInvincibleAtBlow::CheckAllTargetSkillState( void )
{
	vector<DnSkillHandle> vlhEndedTargetSkills;
	GatherEndTargetState( vlhEndedTargetSkills );

	for( int i = 0; i < (int)vlhEndedTargetSkills.size(); ++i )
	{
		EndInvincibleAt( vlhEndedTargetSkills.at( i ) );
	}
}

void CDnProbInvincibleAtBlow::GatherEndTargetState( /*IN OUT*/ vector<DnSkillHandle>& vlhEndedTargetSkill )
{
	for( list<S_SKILL_INFO>::iterator iter = m_listTargetSkills.begin(); m_listTargetSkills.end() != iter; ++iter )
	{
		if( IsSkillFinished(iter->hTargetSkill) )
		{
			vlhEndedTargetSkill.push_back( iter->hTargetSkill );
		}
	}
}

void CDnProbInvincibleAtBlow::CheckTargetSkillStateAndDoEnd( DnSkillHandle hTargetSkill )
{
	if( !hTargetSkill )
		return;

	if( IsSkillFinished( hTargetSkill ) )
	{
		EndInvincibleAt( hTargetSkill );
	}
}

void CDnProbInvincibleAtBlow::CheckProbFailedSkillState( void )
{
	deque<DnSkillHandle>::iterator iter = m_dqProbInvincibleFailedSkills.begin();
	for( iter; iter != m_dqProbInvincibleFailedSkills.end(); )
	{
		if( IsSkillFinished( *iter ) )
		{
			iter = m_dqProbInvincibleFailedSkills.erase( iter );
		}
		else
		{
			++iter;
		}
	}
}

bool CDnProbInvincibleAtBlow::IsSkillFinished( DnSkillHandle hSkill )
{
	bool bResult = false;

	if( hSkill &&
		hSkill->GetActor() &&
		hSkill->GetActor()->IsMonsterActor() )
	{
		bool bSkillObjectFinished = hSkill->IsFinished();
		bResult = bSkillObjectFinished;

		bool bSkillProjectileExist = false;

		bool bTargetSkillUsedProjectile = false;
		map<int, bool>::iterator iter = m_mapTargetSkillUsedProjectile.find( hSkill->GetClassID() );
		if( m_mapTargetSkillUsedProjectile.end() != iter )
		{
			bTargetSkillUsedProjectile = iter->second;
		}

		if( bTargetSkillUsedProjectile)
		{
			// Even if the skill object has ended, check if any projectiles fired by the skill remain... no choice but to check all of them.
			bResult = false;
			for( int i = 0; i < CDnWeapon::GetItemCount( m_hActor->GetRoom() ); ++i )
			{
				DnWeaponHandle hWeapon = CDnWeapon::GetItem( m_hActor->GetRoom(), i );
				if( hWeapon && CDnWeapon::Projectile == hWeapon->GetWeaponType() )
				{
					CDnProjectile* pProjectile = static_cast<CDnProjectile*>(hWeapon.GetPointer());
					DnSkillHandle hSkillFromProjectile = pProjectile->GetParentSkill();
					if( hSkillFromProjectile &&
						hSkillFromProjectile->GetClassID() == hSkill->GetClassID() )
					{
						if( hSkillFromProjectile->GetActor()->IsMonsterActor() )
						{
							CDnMonsterActor* pSkillUserMonsterActor = static_cast<CDnMonsterActor*>(hSkill->GetActor().GetPointer());
							CDnMonsterActor* pNowMonsterActor = static_cast<CDnMonsterActor*>(hSkillFromProjectile->GetActor().GetPointer());
							if( pNowMonsterActor->GetMonsterClassID() == pSkillUserMonsterActor->GetMonsterClassID() )
							{
								bSkillProjectileExist = true;
								break;
							}
						}
					}
				}
			}

			bResult = (bSkillObjectFinished && !bSkillProjectileExist);
		}
	}

	return bResult;
}

bool CDnProbInvincibleAtBlow::CanAddThisSkillsStateBlow( int iSkillID )
{
	bool bResult = true;

	//if( IsBeginInvincible() )
	S_SKILL_INFO* pResult = FindInvincibleTargetSkill( iSkillID );
	if( pResult )
	{
		if( iSkillID == pResult->hTargetSkill->GetClassID() )
			bResult = false;
	}

	return bResult;
}

float CDnProbInvincibleAtBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	float fResult = 0.0f;

	if( HitParam.hHitter || HitParam.hWeapon )
	{
		DnSkillHandle hHitterSkill;
		bool bSkillUseProjectile = false;
		if( HitParam.hWeapon &&  
			CDnWeapon::Projectile == HitParam.hWeapon->GetWeaponType() )
		{
			CDnProjectile* pProjectile = static_cast<CDnProjectile*>( HitParam.hWeapon.GetPointer() );
			hHitterSkill = pProjectile->GetParentSkill();
			bSkillUseProjectile = true;
		}
		else
		if( HitParam.hHitter )
		{
			hHitterSkill = HitParam.hHitter->GetProcessSkill();
		}

		if( hHitterSkill )
		{
			// Requested to distinguish monster ID, so apply only when it is a monster...
			DnActorHandle hSkillUser = hHitterSkill->GetActor();
			if( hSkillUser && hSkillUser->IsMonsterActor() )
			{
				int iMonsterID = static_cast<CDnMonsterActor*>(hSkillUser.GetPointer())->GetMonsterClassID();
				map<int, S_INVINCIBLE_INFO>::iterator iter = m_mapInvincibleAt.find( iMonsterID );
				if( m_mapInvincibleAt.end() != iter )
				{
					if( bSkillUseProjectile )
					{
						m_bNowSelectedSkillUsedProjectile = true;

						// #46112 Since it may hit directly via hit signal initially and later spawn projectiles, update based on whether the current hit is from a projectile.
						m_mapTargetSkillUsedProjectile[ hHitterSkill->GetClassID() ] = true;
					}

					m_pNowMapSelectedSkillInfos = &((iter->second).mapSkillInfos);

					bool bInvincible = IsInvincibleAt( hHitterSkill );
					if( bInvincible )
					{
						fResult = -fOriginalDamage;
						m_hActor->SendProbInvincibleSuccess();		// Send packet to client indicating damage nullification succeeded.
					}

					m_pNowMapSelectedSkillInfos = NULL;
					m_bNowSelectedSkillUsedProjectile = false;
				}
			}
		}
	}

	return fResult;
}
#else
void CDnProbInvincibleAtBlow::OnSuccess( void )
{
	if( m_hEtcObjectEffect )
	{
		m_hEtcObjectEffect->SetActionQueue( "Activate" );
	}
}
#endif // #ifdef _GAMESERVER


//#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
//void CDnProbInvincibleAtBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
//{
//	char szBuff[128] = {0, };
//
//	szNewValue = szOrigValue;
//	// If szOrigValue does not end with ";", append ";" before adding szAddValue.
//	int nLength = (int)strlen(szOrigValue);
//
//	if (nLength == 0 || szOrigValue[nLength - 1] == ';')
//		szNewValue += szAddValue;
//	else
//	{
//		szNewValue += ";";
//		szNewValue += szAddValue;
//	}
//}
//
//void CDnProbInvincibleAtBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
//{
//	char szBuff[128] = {0, };
//
//	//"###;###;###;###;....;##;##;##"
//	//|<--prev string----->|<-AddValue->|
//
//	// Trim szAddValue's length from the end of szOrigValue.
//	int nOrigLength = (int)strlen(szOrigValue);
//	int nAddLength = (int)strlen(szAddValue);
//	int nCount = nOrigLength - nAddLength;
//
//	sprintf_s(szBuff, "%s", szOrigValue);
//	szBuff[nCount] = 0;
//
//	szNewValue = szBuff;
//}
//#endif // PRE_ADD_PREFIX_SYSTE_RENEW
