#include "stdafx.h"
#include "DnState.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

char *CDnState::s_szElementStr[ElementEnum_Amount] = { "_Fire_", "_Ice_", "_Light_", "_Dark_" };


CDnState::CDnState()
{
	m_ValueType = ValueTypeNone;

	ResetState();
}

CDnState::~CDnState()
{
}

void CDnState::ResetState()
{
	m_nStrength = 0;
	m_fStrengthRatio = 0.f;
	m_nAgility = 0;
	m_fAgilityRatio = 0.f;
	m_nIntelligence = 0;
	m_fIntelligenceRatio = 0.f;
	m_nStamina = 0;
	m_fStaminaRatio = 0.f;

	m_nMaxHP = 0;
	m_fMaxHPRatio = 0.f;
	m_nMaxSP = 0;
	m_fMaxSPRatio = 0.f;

	m_nMoveSpeed = 0;
	m_fMoveSpeedRatio = 0.f;
	for( int i=0; i<2; i++ ) {
		m_nAttackP[i] = 0;
		m_nAttackM[i] = 0;
		m_fAttackPRatio[i] = 0.f;
		m_fAttackMRatio[i] = 0.f;
	}
	m_nDefenseP = 0;
	m_fDefensePRatio = 0.f;
	m_nDefenseM = 0;
	m_fDefenseMRatio = 0.f;
	m_nStiff = 0;
	m_fStiffRatio = 0.f;
	m_nStiffResistance = 0;
	m_fStiffResistanceRatio = 0.f;
	m_nCritical = 0;
	m_fCriticalRatio = 0.f;
	m_nCriticalResistance = 0;
	m_fCriticalResistanceRatio = 0.f;
	m_nStun = 0;
	m_fStunRatio = 0.f;
	m_nStunResistance = 0;
	m_fStunResistanceRatio = 0.f;
	m_fDownDelayProb = 0.f;
	m_nRecoverySP = 0;
	m_fRecoverySPRatio = 0.f;
	m_nSuperAmmor = 0;
	m_fSuperAmmorRatio = 0.f;
	m_nFinalDamage = 0;
	m_fFinalDamageRatio = 0.f;
	m_nSpirit = 0;
	m_fSpiritRatio = 0.f;

	for( int i=0; i<ElementEnum_Amount; i++ ) {
		m_fElementAttack[i] = 0.f;
		m_fElementDefense[i] = 0.f;
	}

	m_nSafeZoneMoveSpeed = 0;
	m_fSafeZoneMoveSpeedRatio = 0.f;

	m_fAddExp = 0.0f;
}

void CDnState::CalcValueType()
{
	m_ValueType = ValueTypeNone;
	if( m_nStrength || m_nAgility || m_nIntelligence || m_nStamina || m_nMaxHP || m_nMaxSP || m_nMoveSpeed || m_nAttackP[0] || m_nAttackP[1] || m_nAttackM[0] || m_nAttackM[1] 
	 || m_nDefenseP || m_nDefenseM || m_nStiff || m_nStiffResistance || m_nCritical || m_nCriticalResistance || m_nStun || m_nStunResistance
	 || m_fDownDelayProb || m_nRecoverySP || m_nSuperAmmor || m_nFinalDamage || m_nSpirit
	 || m_fElementAttack[0] || m_fElementAttack[1] || m_fElementAttack[2] || m_fElementAttack[3]
	 || m_fElementDefense[0] || m_fElementDefense[1] || m_fElementDefense[2] || m_fElementDefense[3] || m_nSafeZoneMoveSpeed || m_fAddExp ) 
	 {
		 m_ValueType = (ValueType)( m_ValueType | ValueTypeAbsolute );
	 }

	if( m_fStrengthRatio || m_fAgilityRatio || m_fIntelligenceRatio || m_fStaminaRatio || m_fMaxHPRatio || m_fMaxSPRatio || m_fMoveSpeedRatio || m_fAttackPRatio[0] || m_fAttackPRatio[1] || m_fAttackMRatio[0] || m_fAttackMRatio[1]
	 || m_fDefensePRatio || m_fDefenseMRatio || m_fStiffRatio || m_fStiffResistanceRatio || m_fCriticalRatio || m_fCriticalResistanceRatio || m_fStunRatio || m_fStunResistanceRatio
	 || m_fRecoverySPRatio || m_fSuperAmmorRatio || m_fFinalDamageRatio || m_fSpiritRatio || m_fSafeZoneMoveSpeedRatio ) 
	 {
		 m_ValueType = (ValueType)( m_ValueType | ValueTypeRatio );
	 }
}

CDnState &CDnState::operator += ( CDnState & e )
{
	if( e.m_ValueType & ValueTypeAbsolute ) {
		m_nStrength += e.m_nStrength;
		m_nAgility += e.m_nAgility;
		m_nIntelligence += e.m_nIntelligence;
		m_nStamina += e.m_nStamina;

		m_nMaxHP += e.m_nMaxHP;
		m_nMaxSP += e.m_nMaxSP;

		m_nMoveSpeed += e.m_nMoveSpeed;

		for( int i=0; i<2; i++ ) {
			m_nAttackP[i] += e.m_nAttackP[i];
			m_nAttackM[i] += e.m_nAttackM[i];
		}
		m_nDefenseP += e.m_nDefenseP;
		m_nDefenseM += e.m_nDefenseM;
		m_nStiff += e.m_nStiff;
		m_nStiffResistance += e.m_nStiffResistance;
		m_nCritical += e.m_nCritical;
		m_nCriticalResistance += e.m_nCriticalResistance;
		m_nStun += e.m_nStun;
		m_nStunResistance += e.m_nStunResistance;
		m_fDownDelayProb += e.m_fDownDelayProb;
		m_nRecoverySP += e.m_nRecoverySP;
		m_nSuperAmmor += e.m_nSuperAmmor;
		m_nFinalDamage += e.m_nFinalDamage;
		m_nSpirit += e.m_nSpirit;
		m_nSafeZoneMoveSpeed += e.m_nSafeZoneMoveSpeed;
		m_fAddExp += e.m_fAddExp;

		for( int i=0; i<ElementEnum_Amount; i++ ) {
			m_fElementAttack[i] += e.m_fElementAttack[i];
			m_fElementDefense[i] += e.m_fElementDefense[i];
		}
	}

	if( e.m_ValueType & ValueTypeRatio ) {
		m_fStrengthRatio += e.m_fStrengthRatio;
		m_fAgilityRatio += e.m_fAgilityRatio;
		m_fIntelligenceRatio += e.m_fIntelligenceRatio;
		m_fStaminaRatio += e.m_fStaminaRatio;
		m_fMaxHPRatio += e.m_fMaxHPRatio;
		m_fMaxSPRatio += e.m_fMaxSPRatio;
		m_fMoveSpeedRatio += e.m_fMoveSpeedRatio;

		for( int i=0; i<2; i++ ) {
			m_fAttackPRatio[i] += e.m_fAttackPRatio[i];
			m_fAttackMRatio[i] += e.m_fAttackMRatio[i];
		}

		m_fDefensePRatio += e.m_fDefensePRatio;
		m_fDefenseMRatio += e.m_fDefenseMRatio;
		m_fStiffRatio += e.m_fStiffRatio;
		m_fStiffResistanceRatio += e.m_fStiffResistanceRatio;
		m_fCriticalRatio += e.m_fCriticalRatio;
		m_fCriticalResistanceRatio += e.m_fCriticalResistanceRatio;
		m_fStunRatio += e.m_fStunRatio;
		m_fStunResistanceRatio += e.m_fStunResistanceRatio;
		m_fRecoverySPRatio += e.m_fRecoverySPRatio;
		m_fSuperAmmorRatio += e.m_fSuperAmmorRatio;
		m_fFinalDamageRatio += e.m_fFinalDamageRatio;
		m_fSpiritRatio += e.m_fSpiritRatio;
		m_fSafeZoneMoveSpeedRatio += e.m_fSafeZoneMoveSpeedRatio;
	}

	m_ValueType = (ValueType)( m_ValueType | e.m_ValueType );

	return *this;
}

CDnState &CDnState::operator -= ( CDnState & e )
{
	if( e.m_ValueType & ValueTypeAbsolute ) {
		m_nStrength -= e.m_nStrength;
		m_nAgility -= e.m_nAgility;
		m_nIntelligence -= e.m_nIntelligence;
		m_nStamina -= e.m_nStamina;

		m_nMaxHP -= e.m_nMaxHP;
		m_nMaxSP -= e.m_nMaxSP;

		m_nMoveSpeed -= e.m_nMoveSpeed;

		for( int i=0; i<2; i++ ) {
			m_nAttackP[i] -= e.m_nAttackP[i];
			m_nAttackM[i] -= e.m_nAttackM[i];
		}
		m_nDefenseP -= e.m_nDefenseP;
		m_nDefenseM -= e.m_nDefenseM;
		m_nStiff -= e.m_nStiff;
		m_nStiffResistance -= e.m_nStiffResistance;
		m_nCritical -= e.m_nCritical;
		m_nCriticalResistance -= e.m_nCriticalResistance;
		m_nStun -= e.m_nStun;
		m_nStunResistance -= e.m_nStunResistance;
		m_fDownDelayProb -= e.m_fDownDelayProb;
		m_nRecoverySP -= e.m_nRecoverySP;
		m_nSuperAmmor -= e.m_nSuperAmmor;
		m_nFinalDamage -= e.m_nFinalDamage;
		m_nSpirit -= e.m_nSpirit;
		m_nSafeZoneMoveSpeed -= e.m_nSafeZoneMoveSpeed;
		m_fAddExp -= e.m_fAddExp;

		for( int i=0; i<ElementEnum_Amount; i++ ) {
			m_fElementAttack[i] -= e.m_fElementAttack[i];
			m_fElementDefense[i] -= e.m_fElementDefense[i];
		}
	}

	if( e.m_ValueType & ValueTypeRatio ) {
		m_fStrengthRatio -= e.m_fStrengthRatio;
		m_fAgilityRatio -= e.m_fAgilityRatio;
		m_fIntelligenceRatio -= e.m_fIntelligenceRatio;
		m_fStaminaRatio -= e.m_fStaminaRatio;
		m_fMaxHPRatio -= e.m_fMaxHPRatio;
		m_fMaxSPRatio -= e.m_fMaxSPRatio;
		m_fMoveSpeedRatio -= e.m_fMoveSpeedRatio;

		for( int i=0; i<2; i++ ) {
			m_fAttackPRatio[i] -= e.m_fAttackPRatio[i];
			m_fAttackMRatio[i] -= e.m_fAttackMRatio[i];
		}

		m_fDefensePRatio -= e.m_fDefensePRatio;
		m_fDefenseMRatio -= e.m_fDefenseMRatio;
		m_fStiffRatio -= e.m_fStiffRatio;
		m_fStiffResistanceRatio -= e.m_fStiffResistanceRatio;
		m_fCriticalRatio -= e.m_fCriticalRatio;
		m_fCriticalResistanceRatio -= e.m_fCriticalResistanceRatio;
		m_fStunRatio -= e.m_fStunRatio;
		m_fStunResistanceRatio -= e.m_fStunResistanceRatio;
		m_fRecoverySPRatio -= e.m_fRecoverySPRatio;
		m_fSuperAmmorRatio -= e.m_fSuperAmmorRatio;
		m_fFinalDamageRatio -= e.m_fFinalDamageRatio;
		m_fSpiritRatio -= e.m_fSpiritRatio;
		m_fSafeZoneMoveSpeedRatio -= e.m_fSafeZoneMoveSpeedRatio;
	}

	m_ValueType = (ValueType)( m_ValueType | e.m_ValueType );

	return *this;
}

void CDnState::MergeState( CDnState &State, ValueType eType )
{
	if( eType & ValueTypeAbsolute ) 
	{
		m_nStrength += State.m_nStrength;
		m_nAgility += State.m_nAgility;
		m_nIntelligence += State.m_nIntelligence;
		m_nStamina += State.m_nStamina;
		m_nMaxHP += State.m_nMaxHP;
		m_nMaxSP += State.m_nMaxSP;
		m_nMoveSpeed += State.m_nMoveSpeed;
		m_nDefenseP += State.m_nDefenseP;
		m_nDefenseM += State.m_nDefenseM;
		m_nStiff += State.m_nStiff;
		m_nStiffResistance += State.m_nStiffResistance;
		m_nCritical += State.m_nCritical;
		m_nCriticalResistance += State.m_nCriticalResistance;
		m_nStun += State.m_nStun;
		m_nStunResistance += State.m_nStunResistance;
		m_fDownDelayProb += State.m_fDownDelayProb;
		m_nRecoverySP += State.m_nRecoverySP;
		m_nSuperAmmor += State.m_nSuperAmmor;
		m_nFinalDamage += State.m_nFinalDamage;
		m_nSpirit += State.m_nSpirit;
		m_nSafeZoneMoveSpeed += State.m_nSafeZoneMoveSpeed;
		m_fAddExp += State.m_fAddExp;

		for( int i=0; i<2; i++ ) 
		{
			m_nAttackP[i] += State.m_nAttackP[i];
			m_nAttackM[i] += State.m_nAttackM[i];
		}

		for( int i=0; i<ElementEnum_Amount; i++ ) 
		{
			m_fElementAttack[i] += State.m_fElementAttack[i];
			m_fElementDefense[i] += State.m_fElementDefense[i];
		}
	}

	if( eType & ValueTypeRatio ) 
	{
		m_fStrengthRatio += State.m_fStrengthRatio;
		m_fAgilityRatio += State.m_fAgilityRatio;
		m_fIntelligenceRatio += State.m_fIntelligenceRatio;
		m_fStaminaRatio += State.m_fStaminaRatio;
		m_fMaxHPRatio += State.m_fMaxHPRatio;
		m_fMaxSPRatio += State.m_fMaxSPRatio;
		m_fMoveSpeedRatio += State.m_fMoveSpeedRatio;
		m_fDefensePRatio += State.m_fDefensePRatio;
		m_fDefenseMRatio += State.m_fDefenseMRatio;
		m_fStiffRatio += State.m_fStiffRatio;
		m_fStiffResistanceRatio += State.m_fStiffResistanceRatio;
		m_fCriticalRatio += State.m_fCriticalRatio;
		m_fCriticalResistanceRatio += State.m_fCriticalResistanceRatio;
		m_fStunRatio += State.m_fStunRatio;
		m_fStunResistanceRatio += State.m_fStunResistanceRatio;
		m_fRecoverySPRatio += State.m_fRecoverySPRatio;
		m_fSuperAmmorRatio += State.m_fSuperAmmorRatio;
		m_fFinalDamageRatio += State.m_fFinalDamageRatio;
		m_fSpiritRatio += State.m_fSpiritRatio;
		m_fSafeZoneMoveSpeedRatio += State.m_fSafeZoneMoveSpeedRatio;

		for( int i=0; i<2; i++ ) 
		{
			m_fAttackPRatio[i] += State.m_fAttackPRatio[i];
			m_fAttackMRatio[i] += State.m_fAttackMRatio[i];
		}
	}
}

void CDnState::CalculateRatioValue( CDnState &State )
{
	m_nStrength += (int)( m_nStrength * State.m_fStrengthRatio );
	m_nAgility += (int)( m_nAgility * State.m_fAgilityRatio );
	m_nIntelligence += (int)( m_nIntelligence * State.m_fIntelligenceRatio );
	m_nStamina += (int)( m_nStamina * State.m_fStaminaRatio );

	m_nMaxHP += (INT64)( m_nMaxHP * State.m_fMaxHPRatio );
	m_nMaxSP += (int)( m_nMaxSP * State.m_fMaxSPRatio );

	m_nMoveSpeed += (int)( m_nMoveSpeed * State.m_fMoveSpeedRatio );

	for( int i=0; i<2; i++ ) 
	{
		m_nAttackP[i] += (int)( m_nAttackP[i] * State.m_fAttackPRatio[i] );
		m_nAttackM[i] += (int)( m_nAttackM[i] * State.m_fAttackMRatio[i] );
	}

	m_nDefenseP += (int)( m_nDefenseP * State.m_fDefensePRatio );
	m_nDefenseM += (int)( m_nDefenseM * State.m_fDefenseMRatio );

	m_nStiff += (int)( m_nStiff * State.m_fStiffRatio );
	m_nStiffResistance += (int)( m_nStiffResistance * State.m_fStiffResistanceRatio );
	m_nCritical += (int)( m_nCritical * State.m_fCriticalRatio );
	m_nCriticalResistance += (int)( m_nCriticalResistance * State.m_fCriticalResistanceRatio );
	m_nStun += (int)( m_nStun * State.m_fStunRatio );
	m_nStunResistance += (int)( m_nStunResistance * State.m_fStunResistanceRatio );
	m_nRecoverySP += (int)( m_nRecoverySP * State.m_fRecoverySPRatio );
	m_nSuperAmmor += (int)( m_nSuperAmmor * State.m_fSuperAmmorRatio );
	m_nFinalDamage += (int)( m_nFinalDamage * State.m_fFinalDamageRatio );
	m_nSpirit += (int)( m_nSpirit * State.m_fSpiritRatio );
	m_nSafeZoneMoveSpeed += (int)( m_nSafeZoneMoveSpeed * State.m_fSafeZoneMoveSpeedRatio );
}
