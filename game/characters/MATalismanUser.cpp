#include "stdafx.h"

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "MATalismanUser.h"

MATalismanUser::MATalismanUser()
{
	memset( m_bSelfDelete, 0, sizeof(m_bSelfDelete) );
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		m_fTalismanEfficiency[i] = 1.0f;
		m_TalismanScaledState[i].ResetState();
	}
}

MATalismanUser::~MATalismanUser()
{
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		DetachTalisman( i );
	}
}

void MATalismanUser::ScaleStateByEfficiency( CDnState &dst, CDnState *src, float fEfficiency )
{
	dst.ResetState();
	if( !src ) return;

	if( fEfficiency <= 0.0f )
		fEfficiency = 1.0f;

	dst.SetStrength( (int)( src->GetStrength() * fEfficiency ) );
	dst.SetAgility( (int)( src->GetAgility() * fEfficiency ) );
	dst.SetIntelligence( (int)( src->GetIntelligence() * fEfficiency ) );
	dst.SetStamina( (int)( src->GetStamina() * fEfficiency ) );
	dst.SetSpirit( (int)( src->GetSpirit() * fEfficiency ) );
	dst.SetAttackPMin( (int)( src->GetAttackPMin() * fEfficiency ) );
	dst.SetAttackPMax( (int)( src->GetAttackPMax() * fEfficiency ) );
	dst.SetAttackMMin( (int)( src->GetAttackMMin() * fEfficiency ) );
	dst.SetAttackMMax( (int)( src->GetAttackMMax() * fEfficiency ) );
	dst.SetDefenseP( (int)( src->GetDefenseP() * fEfficiency ) );
	dst.SetDefenseM( (int)( src->GetDefenseM() * fEfficiency ) );
	dst.SetMaxHP( (INT64)( src->GetMaxHP() * (double)fEfficiency ) );
	dst.SetMaxSP( (int)( src->GetMaxSP() * fEfficiency ) );
	dst.SetMoveSpeed( (int)( src->GetMoveSpeed() * fEfficiency ) );
	dst.SetSafeZoneMoveSpeed( (int)( src->GetSafeZoneMoveSpeed() * fEfficiency ) );
	dst.SetStiff( (int)( src->GetStiff() * fEfficiency ) );
	dst.SetStiffResistance( (int)( src->GetStiffResistance() * fEfficiency ) );
	dst.SetCritical( (int)( src->GetCritical() * fEfficiency ) );
	dst.SetCriticalResistance( (int)( src->GetCriticalResistance() * fEfficiency ) );
	dst.SetStun( (int)( src->GetStun() * fEfficiency ) );
	dst.SetStunResistance( (int)( src->GetStunResistance() * fEfficiency ) );
	dst.SetRecoverySP( (int)( src->GetRecoverySP() * fEfficiency ) );
	dst.SetSuperAmmor( (int)( src->GetSuperAmmor() * fEfficiency ) );
	dst.SetFinalDamage( (int)( src->GetFinalDamage() * fEfficiency ) );
	dst.SetDownDelayProb( src->GetDownDelayProb() * fEfficiency );
	dst.SetAddExp( src->GetAddExp() * fEfficiency );

	for( int e = 0; e < CDnState::ElementEnum_Amount; ++e )
	{
		CDnState::ElementEnum elem = (CDnState::ElementEnum)e;
		dst.SetElementAttack( elem, src->GetElementAttack( elem ) * fEfficiency );
		dst.SetElementDefense( elem, src->GetElementDefense( elem ) * fEfficiency );
	}

	dst.SetStrengthRatio( src->GetStrengthRatio() * fEfficiency );
	dst.SetAgilityRatio( src->GetAgilityRatio() * fEfficiency );
	dst.SetIntelligenceRatio( src->GetIntelligenceRatio() * fEfficiency );
	dst.SetStaminaRatio( src->GetStaminaRatio() * fEfficiency );
	dst.SetSpiritRatio( src->GetSpiritRatio() * fEfficiency );
	dst.SetAttackPMinRatio( src->GetAttackPMinRatio() * fEfficiency );
	dst.SetAttackPMaxRatio( src->GetAttackPMaxRatio() * fEfficiency );
	dst.SetAttackMMinRatio( src->GetAttackMMinRatio() * fEfficiency );
	dst.SetAttackMMaxRatio( src->GetAttackMMaxRatio() * fEfficiency );
	dst.SetDefensePRatio( src->GetDefensePRatio() * fEfficiency );
	dst.SetDefenseMRatio( src->GetDefenseMRatio() * fEfficiency );
	dst.SetStiffRatio( src->GetStiffRatio() * fEfficiency );
	dst.SetStiffResistanceRatio( src->GetStiffResistanceRatio() * fEfficiency );
	dst.SetCriticalRatio( src->GetCriticalRatio() * fEfficiency );
	dst.SetCriticalResistanceRatio( src->GetCriticalResistanceRatio() * fEfficiency );
	dst.SetStunRatio( src->GetStunRatio() * fEfficiency );
	dst.SetStunResistanceRatio( src->GetStunResistanceRatio() * fEfficiency );
	dst.SetMaxHPRatio( src->GetMaxHPRatio() * fEfficiency );
	dst.SetMaxSPRatio( src->GetMaxSPRatio() * fEfficiency );
	dst.SetMoveSpeedRatio( src->GetMoveSpeedRatio() * fEfficiency );
	dst.SetSafeZoneMoveSpeedRatio( src->GetSafeZoneMoveSpeedRatio() * fEfficiency );
	dst.SetRecoverySPRatio( src->GetRecoverySPRatio() * fEfficiency );
	dst.SetSuperAmmorRatio( src->GetSuperAmmorRatio() * fEfficiency );
	dst.SetFinalDamageRatio( src->GetFinalDamageRatio() * fEfficiency );

	dst.CalcValueType();
}

bool MATalismanUser::AttachTalisman( DnTalismanHandle hTalisman, int nSlotIndex, float fEfficiency, bool bDelete )
{
	if( !hTalisman ) return false;
	if( nSlotIndex < 0 || nSlotIndex >= TALISMAN_MAX ) return false;

	if( m_hTalisman[nSlotIndex] && m_hTalisman[nSlotIndex] != hTalisman )
	{
		MATalismanUser::DetachTalisman( nSlotIndex );
	}

	m_hTalisman[nSlotIndex] = hTalisman;
	m_bSelfDelete[nSlotIndex] = bDelete;
	m_fTalismanEfficiency[nSlotIndex] = ( fEfficiency > 0.0f ) ? fEfficiency : 1.0f;

	ScaleStateByEfficiency( m_TalismanScaledState[nSlotIndex], hTalisman.GetPointer(), m_fTalismanEfficiency[nSlotIndex] );

	if( hTalisman->GetTalismanType() != CDnTalisman::Normal && fEfficiency > 0.0f )
	{
		hTalisman->SetEfficiency( fEfficiency );
	}

	return true;
}

bool MATalismanUser::DetachTalisman( int nSlotIndex )
{
	if( nSlotIndex < 0 || nSlotIndex >= TALISMAN_MAX ) return false;
	if( !m_hTalisman[nSlotIndex] ) return true;

	if( m_bSelfDelete[nSlotIndex] )
	{
		SAFE_RELEASE_SPTR( m_hTalisman[nSlotIndex] );
		m_bSelfDelete[nSlotIndex] = false;
	}
	m_hTalisman[nSlotIndex].Identity();
	m_fTalismanEfficiency[nSlotIndex] = 1.0f;
	m_TalismanScaledState[nSlotIndex].ResetState();

	return true;
}

CDnState *MATalismanUser::GetTalismanScaledState( int nSlotIndex )
{
	if( nSlotIndex < 0 || nSlotIndex >= TALISMAN_MAX ) return NULL;
	if( !m_hTalisman[nSlotIndex] ) return NULL;
	return &m_TalismanScaledState[nSlotIndex];
}

void MATalismanUser::Process( LOCAL_TIME LocalTime, float fDelta )
{
}

#endif // PRE_ADD_TALISMAN_SYSTEM
