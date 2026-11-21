
#include "stdafx.h"
#include "DNMonsterAggroSystem.h"
#include "DnMonsterActor.h"
#include "MAScanner.h"
#include "DnStateBlow.h"
#include "DnPlayerActor.h"
#include "DnAggroResetBlow.h"

CDNMonsterAggroSystem::CDNMonsterAggroSystem( DnActorHandle hActor )
: CDNAggroSystem( hActor ), m_pMonsterActor( reinterpret_cast<CDnMonsterActor*>(m_hActor.GetPointer()) )
{
	m_uiFrameCount	= 0;
	m_bInit			= false;
	_Create();
}

CDNMonsterAggroSystem::~CDNMonsterAggroSystem()
{

}

void CDNMonsterAggroSystem::_Create()
{
	// Parse table
	DNTableFileFormat* pMonsterSox = GetDNTable( CDnTableDB::TMONSTER );
	_ASSERT( pMonsterSox );
	
	int iItemID = m_pMonsterActor->GetMonsterClassID();
	
	_ASSERT( pMonsterSox->IsExistItem( iItemID ) );

	m_AggroRange.Initialize( m_pMonsterActor, pMonsterSox );
	m_iThreatAggro						= pMonsterSox->GetFieldFromLablePtr( iItemID, "_ThreatAggro" )->GetInteger();
	m_fDecreaseAggroPer					= pMonsterSox->GetFieldFromLablePtr( iItemID, "_DecreaseAggro" )->GetFloat();
	m_fRangeAttackAdditionalAggroPer	= pMonsterSox->GetFieldFromLablePtr( iItemID, "_RangeAttackAdditionalAggro" )->GetFloat();
	m_fStunStateAdditionalAggroPer		= pMonsterSox->GetFieldFromLablePtr( iItemID, "_StunStateAdditionalAggro" )->GetFloat();
	m_fDownStateAdditionalAggroPer		= pMonsterSox->GetFieldFromLablePtr( iItemID, "_DownStateAdditionalAggro" )->GetFloat();
	m_fCantMoveStateAdditionalAggroPer	= pMonsterSox->GetFieldFromLablePtr( iItemID, "_CantMoveStateAdditionalAggro" )->GetFloat();
	m_fSleepStateAdditionalAggroPer		= pMonsterSox->GetFieldFromLablePtr( iItemID, "_SleepStateAdditionalAggro" )->GetFloat();
	m_fUnderHPStateAdditionalAggroPer	= pMonsterSox->GetFieldFromLablePtr( iItemID, "_UnderHPStateAdditionalAggro" )->GetFloat();
	
	m_uiLastTopAggro = 0;
}

void CDNMonsterAggroSystem::OnProcessAggro( const LOCAL_TIME LocalTime, const float fDelta )
{
	++m_uiFrameCount;
	if( m_uiFrameCount >= eCommon::AIProcessFrame )
		m_uiFrameCount = 0;

	// If there is no aggro, scan to find a target.
	if( m_AggroList.empty() )
	{
		if( m_hActor->GetUniqueID()%eCommon::AIProcessFrame != m_uiFrameCount )
			return;
		m_bInit = true;

		DNVector(DnActorHandle) hEnemyList;
		GetMAScanner().Scan( MAScanner::eType::OpponentTeamScan, m_hActor, 0.f, m_AggroRange.GetThreatRange(), hEnemyList );
		if( hEnemyList.empty() )
			return;

		for( UINT i=0 ; i<hEnemyList.size() ; ++i ) 
		{
			// Randomize +/- 30%. Too crowded at the start..
			float fRandomRatio = 0.3f;

			int iValue = static_cast<int>( m_iThreatAggro * ( fRandomRatio * 2.f ) );
			if( iValue < 1 ) 
				iValue = 1;
			int iRandom = _roomrand(m_hActor->GetRoom())%iValue;
			iValue = m_iThreatAggro - (int)( m_iThreatAggro * fRandomRatio ) + iRandom;

			AddAggro( hEnemyList[i], iValue );
		}
	}
	// If there is aggro, check whether the target is dead or out of range
	else
	{
		for( std::list<AggroStruct>::iterator itor=m_AggroList.begin() ; itor!=m_AggroList.end() ; )
		{
			// Delete if actor is missing or dead
			bool bFinishAggro = false;
			if( (*itor).hActor )
			{
				switch( (*itor).hActor->GetActorType() ) 
				{
					case CDnActorState::PropActor:
					{
						if( (*itor).hActor->IsDie() ) 
							bFinishAggro = true;
						break;
					}
					default:
					{
						if( (*itor).hActor->IsDie() || (*itor).hActor->GetTeam() == m_hActor->GetTeam() ) 
							bFinishAggro = true;
						break;
					}
				}
			}
			else
				bFinishAggro = true;

			if( bFinishAggro ) 
			{
				itor = m_AggroList.erase( itor );
				continue;
			}

#ifdef PRE_FIX_PARTSMONSTER_AI_TARGETTING
			float fDistSq = EtVec3LengthSq( &( *m_hActor->GetPosition() - (*itor).hActor->FindAutoTargetPos() ) );
#else
			float fDistSq = EtVec3LengthSq( &( *m_hActor->GetPosition() - *(*itor).hActor->GetPosition() ) );
#endif

			// Remove if outside combat aggro range
			if( fDistSq >= m_AggroRange.GetCognizanceThreatRangeSq() )
			{
				itor = m_AggroList.erase( itor );
				continue;
			}

			// Adjust accumulated aggro data
			itor->fDecreaseDelta -= fDelta;
			if( itor->fDecreaseDelta <= 0.f )
			{
				itor->fDecreaseDelta = static_cast<float>(CDNAggroSystem::eDecreaseDeltaTick/1000.f);
				// Decrease aggro data
				if( m_fDecreaseAggroPer > 0.f && !m_bIgnore )
				{
					itor->iAggro -= static_cast<int>((itor->iAggro)*m_fDecreaseAggroPer);
				}
			}

			if( itor->iAggro <= 0 )
			{
				itor = m_AggroList.erase( itor );
				continue;
			}

			++itor;
		}
	}
}

DnActorHandle CDNMonsterAggroSystem::OnGetAggroTarget( bool& bIsProvocationTarget, DnActorHandle hExceptActor/*=CDnActor::Identity()*/, DNVector(DnActorHandle)* vTarget/*=NULL*/ )
{
	bIsProvocationTarget = false;
	CDnStateBlow* pStateBlow = m_hActor->GetStateBlow();
	if( m_bIgnore == false && m_hActor->GetStateBlow() )
	{
		// Provoke
		if( pStateBlow->IsApplied( STATE_BLOW::BLOW_132) )
		{
			DNVector(DnBlowHandle) vResult;
			pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_132, vResult );

			float fStateValue = 0.0f;
			DnActorHandle hSkillUser;

			for( UINT i=0 ; i<vResult.size() ; ++i )
			{
				DnBlowHandle hBlow = vResult[i];
				if( hBlow )
				{
					if( fStateValue <= hBlow->GetFloatValue() )
					{
						fStateValue = hBlow->GetFloatValue();
						const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
						if( pSkillInfo )
						{
							hSkillUser = pSkillInfo->hSkillUser;
							if( hSkillUser )
							{
#if defined (PRE_MOD_AIMULTITARGET)
								if( vTarget )
								{
									for( UINT j=0; j<vTarget->size(); ++j )
									{
										if( (*vTarget)[j] == hSkillUser )
										{
											bIsProvocationTarget = true;
										}
									}
								}
								else
								{
									bIsProvocationTarget = true;
								}
#else // PRE_MOD_AIMULTITARGET
								bIsProvocationTarget = true; 
#endif // PRE_MOD_AIMULTITARGET
							}
						}
					}
				}
			}

			if( bIsProvocationTarget && hSkillUser )
				return hSkillUser;
		}
	}

	if( m_AggroList.size() == 1 )
	{
		DnActorHandle hActor = (*m_AggroList.begin()).hActor;

		// 148) Reset aggro when this state effect is applied. Valid only if target is a monster actor. (#21673)
		if( pStateBlow->IsApplied( STATE_BLOW::BLOW_148) )
		{
			DNVector(DnBlowHandle) vBlows;
			pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_148, vBlows );
			for( int i = 0; i < (int)vBlows.size(); ++i )
			{
				if( vBlows[i] )
				{
					if( const_cast<CDnSkill::SkillInfo*>(static_cast<CDnAggroResetBlow*>( vBlows.at( i ).GetPointer() )->GetParentSkillInfo())->hSkillUser == hActor )
						return CDnActor::Identity();
				}
			}
		}

#if defined (PRE_MOD_AIMULTITARGET)
		if (vTarget && hActor)
		{
			for (UINT i=0; i<vTarget->size(); ++i)
			{
				if ((*vTarget)[i] == hActor)
					return hActor;
			}
		}
		else
			return hActor ? hActor : CDnActor::Identity();
#else
		return hActor ? hActor : CDnActor::Identity();
#endif
	}

	int			 iMaxAggro	= 0;
	AggroStruct* pFind		= NULL;

	for( std::list<AggroStruct>::iterator itor=m_AggroList.begin() ; itor!=m_AggroList.end() ; )
	{
		if( !(*itor).hActor )
		{
			itor = m_AggroList.erase( itor );
			continue;
		}

		// 148) Reset aggro when this state effect is applied. Valid only if target is a monster actor. (#21673)
		if( pStateBlow->IsApplied( STATE_BLOW::BLOW_148) )
		{
			bool bSkip = false;
			DNVector(DnBlowHandle) vBlows;
			pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_148, vBlows );
			for( int i = 0; i < (int)vBlows.size(); ++i )
			{
				if( vBlows[i] )
				{
					if( const_cast<CDnSkill::SkillInfo*>(static_cast<CDnAggroResetBlow*>( vBlows.at( i ).GetPointer() )->GetParentSkillInfo())->hSkillUser == (*itor).hActor )
					{
						bSkip =true;
						break;
					}
				}
			}

			if( bSkip )
			{
				++itor;
				continue;
			}
		}

		// Calculate Modifier
		CDnStateBlow* pStateBlow	= (*itor).hActor->GetStateBlow();
		(*itor).iModifierAggro		= (*itor).iAggro;

		// Stun target
		if( (*itor).hActor->IsStun() )
		{
			(*itor).iModifierAggro	+= _CalcAdditionalAggro( (*itor).iAggro, m_fStunStateAdditionalAggroPer );
			//g_Log.LogA( "Additional aggro generated due to Stun state!!!\r\n" );
		}

		// Down target
		if( (*itor).hActor->IsDown() )
		{
			(*itor).iModifierAggro	+= _CalcAdditionalAggro( (*itor).iAggro, m_fDownStateAdditionalAggroPer );
			//g_Log.LogA( "Additional aggro generated due to Down state!!!\r\n" );
		}

		// Check Blow
		if( pStateBlow )
		{
			if( (*itor).hActor->GetCantMoveSEReferenceCount() > 0 )
			{
				(*itor).iModifierAggro	+= _CalcAdditionalAggro( (*itor).iAggro, m_fCantMoveStateAdditionalAggroPer );
				//g_Log.LogA( "Additional aggro generated due to Move-restricted state!!!\r\n" );
			}

			// Sleep state (skill effect 45)
			if( pStateBlow->IsApplied( STATE_BLOW::BLOW_045 ) )
			{
				(*itor).iModifierAggro	+= _CalcAdditionalAggro( (*itor).iAggro, m_fSleepStateAdditionalAggroPer );
				//g_Log.LogA( "Additional aggro generated due to Sleep state!!!\r\n" );
			}
		}

		// Check HP
		if( (*itor).hActor->GetHPPercent() <= 30 )
		{
			(*itor).iModifierAggro	+= _CalcAdditionalAggro( (*itor).iAggro, m_fUnderHPStateAdditionalAggroPer );
			//g_Log.LogA( "Additional aggro generated due to UnderHP state!!!\r\n" );
		}

		if( (*itor).iModifierAggro <= 0 )
			(*itor).iModifierAggro = 1;

		if( (*itor).iModifierAggro > iMaxAggro )
		{
			bool bUpdate = true;
			if( hExceptActor )
			{
				if( hExceptActor == (*itor).hActor )
					bUpdate = false;
			}

#if defined (PRE_MOD_AIMULTITARGET)
			if (vTarget)
			{
				bUpdate= false;
				for (UINT i=0; i<vTarget->size(); ++i)
				{
					if ((*vTarget)[i] == (*itor).hActor)
					{
						bUpdate= true;
						break;
					}
				}
			}
#endif
			
			if( bUpdate )
			{
				pFind		= &(*itor);
				iMaxAggro	= pFind->iModifierAggro;
			}
		}

		++itor;
	}

	if( pFind )
	{
		m_uiLastTopAggro = pFind->iModifierAggro;
		return pFind->hActor;
	}

	m_uiLastTopAggro = 0;
	return CDnActor::Identity();
}

void CDNMonsterAggroSystem::OnDamageAggro( DnActorHandle hActor, CDnDamageBase::SHitParam& HitParam, int iDamage )
{
	if( !hActor )
		return;

	int iAggroValue = iDamage * 2;
	if( hActor->IsProcessSkill() ) 
		iAggroValue += hActor->GetProcessSkill()->GetAdditionalThreat();

	// Additional aggro for ranged attacks
	if( HitParam.DistanceType == CDnDamageBase::DistanceTypeEnum::Range )
		iAggroValue += _CalcAdditionalAggro( iAggroValue, m_fRangeAttackAdditionalAggroPer );

	AddAggro( hActor, iAggroValue );
}

void CDNMonsterAggroSystem::OnStateBlowAggro( DnBlowHandle hBlow )
{
	if( !hBlow || !hBlow->GetActorHandle() )
		return;

	if( hBlow->GetThreatAggro() <= 0 )
		return;

	AddAggro( hBlow->GetActorHandle(), hBlow->GetThreatAggro() );
}

bool CDNMonsterAggroSystem::bOnCheckPlayerBeginStateBlow( CDnPlayerActor* pPlayer )
{
	EtVector3	vTemp	= *pPlayer->GetPosition() - *m_hActor->GetPosition();
	bool bIsProvocationTarget;
	float		fTempSq = OnGetAggroTarget( bIsProvocationTarget ) ? m_AggroRange.GetCognizanceGentleRangeSq() : m_AggroRange.GetCognizanceThreatRangeSq();

	if( EtVec3LengthSq( &vTemp ) > fTempSq )
		return false;

	return true;
}

void CDNMonsterAggroSystem::OnAggroRegulation( DnActorHandle hActor, int& iAggroValue )
{
	if( hActor && hActor->IsPlayerActor() )
	{
		int iAggroPer = GetPlayerLevelTable().GetValue( hActor->OnGetJobClassID(), hActor->GetLevel(), CPlayerLevelTable::AggroPerPvE );
		iAggroValue = iAggroValue * iAggroPer / 100;
	}
}

int CDNMonsterAggroSystem::_CalcAdditionalAggro( const int iAggro, const float fValue )
{
	if( fValue <= 0.f )
		return -iAggro;
	else if( fValue >= 1.f )
	{
		return static_cast<int>(iAggro*(fValue-1.f));
	}

	return static_cast<int>(-iAggro*(1.f-fValue));
}
