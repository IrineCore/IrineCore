#include "stdafx.h"
#include "DnPoisonBlow.h"
#include "DnDamageBase.h"
#include "DnActionBase.h"
#ifndef _GAMESERVER
#include "DnPlayerActor.h"
#else
#include "DnHighLanderBlow.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


const LOCAL_TIME POISON_DAMAGE_INTERVAL = 2000;
const int DEFAULT_POISON_DAMAGE_PROB = 1000;			// Unit is percentage * 10

#if defined(_GAMESERVER)
extern int GetGaussianRandom( int nMin, int nMax, CMultiRoom *pRoom );
#endif // _GAMESERVER

CDnPoisonBlow::CDnPoisonBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ), 
																			m_IntervalChecker( hActor, GetMySmartPtr() ),
																			m_fEffectLength( 0.0f )
																			,m_bEffecting(false)
#ifdef _GAMESERVER
																			, m_fIntervalDamage( 0.0f )
#endif
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_044;
	SetValue( szValue );

	m_fDamageProb = 0.0f;

#if defined(PRE_FIX_53266)
	m_fAddDamageValue = 0.0f;

	std::string str = szValue;//"DamageProb;DamageRatio;ExtraDamage", or "Damage";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);

	int nTokenSize = (int)tokens.size();
	//If setting value is 1, apply attack power as before?
	if (1 == nTokenSize)
	{
		m_fValue = (float)atof( tokens[0].c_str() );
		m_fDamageProb = -1.0f;
	}
	else if (nTokenSize >= 2)
	{
		m_fValue = (float)atof(tokens[0].c_str());
		m_fDamageProb = (float)atof(tokens[1].c_str());

		if (nTokenSize >= 3)
			m_fAddDamageValue = (float)atof(tokens[2].c_str());
	}
	else
	{
		OutputDebug("%s Invalid Value %s\n", __FUNCTION__, szValue);
	}
#else
	string strValue(szValue);
	int iSemiColonIndex = (int)strValue.find_first_of( ';', 0 );
	bool bDamageProbMode = (string::npos != iSemiColonIndex);
	if( bDamageProbMode )
	{
		string strProb = strValue.substr( 0, iSemiColonIndex );
		string strDamageProb = strValue.substr( iSemiColonIndex+1, strValue.length() );

		// Probability
		m_fValue = (float)atof( strProb.c_str() );
		m_fDamageProb = (float)atof( strDamageProb.c_str() );

#ifdef _GAMESERVER
		_ASSERT( 0.0f <= m_fDamageProb/* && m_fDamageProb <= 1.0f*/ );
#if defined(PRE_ADD_SKILLBUF_RENEW)
		//Calculate damage when skill info is set (OnSetParentSkillInfo)
		//m_fIntervalDamage = CalcIntervalDamage(CDnState::Dark);
#else
		m_fIntervalDamage = m_hActor->GetLastDamage() * m_fDamageProb;
#endif // PRE_ADD_SKILLBUF_RENEW
#endif
	}
	else
	{
		m_fValue = (float)atof( szValue );
		m_fDamageProb = -1.0f;
	}
#endif // PRE_FIX_53266

#ifndef _GAMESERVER
	UseTableDefinedGraphicEffect( false );
#endif
}

CDnPoisonBlow::~CDnPoisonBlow(void)
{

}

#ifdef _GAMESERVER
bool CDnPoisonBlow::CanBegin( void )
{
	bool bResult = true;

	// Legacy handling with only 1 argument does not check resistance.
	if( -1.0f == m_fDamageProb )
		return true;

	// Check Poison resistance.
	DNTableFileFormat* pSox = NULL;
	int iClassID = m_hActor->GetClassID();
	int nWeightItemID = 0;
	float fWeight = 0.0f;

	// Apply elemental weight only for players.
	bool bPlayer = false;
	if( m_hActor->GetActorType() <= CDnActor::Reserved6 )
		bPlayer = true;

	if( bPlayer )
	{
		fWeight = CPlayerWeightTable::GetInstance().GetValue( iClassID, CPlayerWeightTable::ElementDefense );
	}

	// <Target's poison evasion rate> = <dark elemental resistance x elemental defense value from PlayerWeightTable>
	// Target's actual shock chance = <skill poison rate> - <target poison evasion rate>
	float fAvoidProb = 1.0f - (m_fValue - (m_hActor->GetElementDefense( CDnState::Dark ) * fWeight));
	if( _rand(GetRoom()) % 10000 <= int(fAvoidProb * 10000.0f) )
	{
		// Changed so the caller invokes CanBegin and immediately deletes the state effect on failure.
		//SetState( STATE_BLOW::STATE_END );
		OutputDebug( "CDnPoisonBlow::CanAdd - Poison Fail\n" );
		bResult = false;
	}

	return bResult;
}
#endif


void CDnPoisonBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	// Calculate poison damage once initially and keep deducting it.
	if( -1.0f == m_fDamageProb )
	{
		if( m_ParentSkillInfo.hSkillUser )
		{
			CDnDamageBase::SHitParam HitParam;
			HitParam.cAttackType = 1;					// Skill damage is magic attack
			HitParam.HasElement = CDnState::Dark;
			m_fIntervalDamage = m_hActor->PreCalcDamage( m_ParentSkillInfo.hSkillUser.GetPointer(), HitParam, 1.0f, m_fValue );
		}
		else
		{
			// Since SkillUser is not set for prop state effects such as poison 
			// Apply the state effect's argument values directly.
			m_fIntervalDamage = m_fValue;
		}
	}
	
	// Guarantee at least 1 DoT damage.
	if( m_fIntervalDamage < 1.0f )
	{
		//SetState( STATE_BLOW::STATE_END );
		//OutputDebug( "CDnPoisonBlow::CanAdd - Damage is too small. ( < 1) Poison Fail\n" );
		m_fIntervalDamage = 1.0f;
	}
#endif

	m_IntervalChecker.OnBegin( LocalTime, POISON_DAMAGE_INTERVAL );

	m_fEffectLength = 0.0f;
	m_bEffecting = false;
	
	OutputDebug( "CDnPoisonBlow::OnBegin, Value:%2.2f (HP : %d ) \n", m_fValue, m_hActor->GetHP());
}

//bool CDnPoisonBlow::OnCheckIntervalProb( void )
//{
//	bool bSuccess = false;
//
//	int iDamageProbability = DEFAULT_POISON_DAMAGE_PROB - m_hActor->GetElementDefense( CDnState::Dark ) * 3;
//
//	if( iDamageProbability < 0 )
//		iDamageProbability = 0;
//	else
//	if( iDamageProbability > 1000 )
//		iDamageProbability = 1000;
//
//	bSuccess = (_rand(GetRoom()) % 1000 < iDamageProbability);
//
//	return bSuccess;
//}


bool CDnPoisonBlow::OnCustomIntervalProcess( void )
{
	//OutputDebug( "Poison Blow : OnCustomInvterval Called id: %d\n", m_hActor->GetUniqueID() );

#ifdef _GAMESERVER
	if( 0.0f < m_fIntervalDamage )
	{
#if defined(PRE_FIX_44884)
#if defined(PRE_FIX_61382)
		DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
		if (hActor &&
			hActor->IsDie() == false)
		{
			m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), (int)m_fIntervalDamage);

			//#48491 Die is being called from ActorStatIntervlaManipulator when applying damage via timer.
			//No need to call Die here.
			//if (m_hActor->IsDie())
			//	m_hActor->Die(m_ParentSkillInfo.hSkillUser);
		}
#else
		if (m_hActor->IsDie() == false)
		{
			m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), (int)m_fIntervalDamage);

			//#48491 Die is being called from ActorStatIntervlaManipulator when applying damage via timer.
			//No need to call Die here.
			//if (m_hActor->IsDie())
			//	m_hActor->Die(m_ParentSkillInfo.hSkillUser);
		}
#endif // PRE_FIX_61382
#else
		// Must not die while the Highlander state effect is active.
		float fDamage = m_fIntervalDamage;
		if( m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_143 ) )
		{
			// IsAppliedThisStateBlow() returns true, but duration has expired
			// State effects are not retrieved from GatherAppliedStateBlowByBlowIndex(), so the list may be empty.
			DNVector( DnBlowHandle ) vlhHighLanderBlow;
			m_hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_143, vlhHighLanderBlow );
			if( 0 < (int)vlhHighLanderBlow.size() )
			{
				CDnHighlanderBlow* pHighlanderBlow = static_cast<CDnHighlanderBlow*>(vlhHighLanderBlow.front().GetPointer());
				fDamage += pHighlanderBlow->CalcDamage( fDamage );
			}
		}

		DWORD dwHitterUniqueID = m_ParentSkillInfo.hSkillUser ? m_ParentSkillInfo.hSkillUser->GetUniqueID() : -1;
		m_hActor->SetHP( m_hActor->GetHP()-(INT64)fDamage );
		m_hActor->RequestHPMPDelta( m_ParentSkillInfo.eSkillElement, -(INT64)fDamage, dwHitterUniqueID );
#endif // PRE_FIX_44884
	}
#else
	//DnEtcHandle hEffectHandle = m_hActor->AttachSEEffect( m_ParentSkillInfo, m_StateBlow.emBlowIndex, GetBlowID(), NULL, NULL, "Poisoning.act" );
	_AttachGraphicEffect();
	if( m_hEtcObjectEffect )
	{	
		//OutputDebug( "Poison Effect created. id: %d\n", m_hActor->GetUniqueID() );
		CEtActionBase::ActionElementStruct* pStruct = m_hEtcObjectEffect->GetElement( "Idle" );
		if( pStruct )
		{
			m_fEffectLength = (float)pStruct->dwLength / 1000.0f;
		}

		_SetDiffuse( 0.0f, 1.0f, 0.0f, 1.0f );
		m_bEffecting = true;
	}
#endif

	return true;	
}


//#ifndef _GAMESERVER
//void CDnPoisonBlow::_SetDiffuse( fl00000000oat fRed, float fGreen, float fBlue, float fAlpha )
//{
//	EtAniObjectHandle hAniObject = m_hActor->GetObjectHandle();
//	int iCustomParamIndex = hAniObject->AddCustomParam( "g_MaterialDiffuse", EPT_VECTOR );
//	EtColor Color( fRed, fGreen, fBlue, fAlpha );
//
//	if( -1 != iCustomParamIndex )
//	{
//		hAniObject->SetCustomParam( iCustomParamIndex, EPT_VECTOR, &Color );
//	}
//	else
//	{
//		// TODO: For multi-part actors, may need to set color per part..
//		// If player
//		CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(m_hActor.GetPointer());
//		if( pPlayerActor )
//		{
//			for( int iPart = (int)CDnParts::Face; iPart < CDnParts::PartsTypeEnum_Amount; ++iPart )
//			{
//				DnPartsHandle hBodyParts = pPlayerActor->GetParts( (CDnParts::PartsTypeEnum)iPart );
//				if( hBodyParts )
//				{
//					EtAniObjectHandle hBodyObject = hBodyParts->GetObjectHandle();
//					int iCustomParam = hBodyObject->AddCustomParam( "g_MaterialDiffuse", EPT_VECTOR );
//
//					hBodyObject->SetCustomParam( iCustomParam, EPT_VECTOR, &Color );
//				}
//			}
//		}
//	}
//
//}
//#endif


void CDnPoisonBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	m_IntervalChecker.Process( LocalTime, fDelta );

#ifndef _GAMESERVER
	if( m_bEffecting )
	{
		m_fEffectLength -= fDelta;
		if( m_fEffectLength < 0.0f )
		{
			m_fEffectLength = 0.0f;
			_SetDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );

			m_bEffecting = false;
		}
	}
#endif

	//LOCAL_TIME ElapsedTime = LocalTime - m_StartTime;
	//int iDamageCount = int(ElapsedTime / POISON_DAMAGE_INTERVAL);
	//while( m_iDamageCount < iDamageCount )
	//{
	//	if(  )
	//	{
	//		CDnDamageBase::SHitParam HitParam;
	//		HitParam.vPosition = *m_hActor->GetPosition();
	//		HitParam.vViewVec = *m_hActor->GetLookDir();
	//		HitParam.fDamage = m_fValue;
	//		m_hActor->OnDamage( m_hActor.GetPointer(), HitParam );

	//		//m_hActor->SetHP( m_hActor->GetHP() - (INT64)m_fValue );
	//	}

	//	m_LastDamageCheckTime = LocalTime;

	//	++m_iDamageCount;
	//}

	// TODO: Takes dark-attribute damage every 2 seconds
	// Dark resistance can evade damage. Default takes 100% poison damage (+0.3% evasion chance per 1 Dark resistance)
	// Visually indicate when evaded.
	//if( LocalTime - m_LastDamageCheckTime > POISON_DAMAGE_INTERVAL )
	//{
		//int iDamageProbability = DEFAULT_POISON_DAMAGE_PROB - m_hActor->GetElementDefense( CDnState::Dark ) * 3;
		//
		//if( iDamageProbability < 0 )
		//	iDamageProbability = 0;
		//else
		//if( iDamageProbability > 1000 )
		//	iDamageProbability = 1000;

		//if( _rand(GetRoom()) % 1000 < iDamageProbability )
		//{
		//	// Does it die automatically when HP <= 0?
		//	m_hActor->SetHP( m_hActor->GetHP() - (INT64)m_fValue );
		//}

		//m_LastDamageCheckTime = LocalTime;
	//}

}

void CDnPoisonBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
//#ifdef _GAMESERVER
	// If called with LocalTime == 0, force-terminates for deletion, stacking handling, etc.
	if( LocalTime != 0 )
		m_IntervalChecker.OnEnd( LocalTime, fDelta );

#ifndef _GAMESERVER
	_SetDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
	
	// Delete effect. Okay even if it already disappeared.
	//m_hActor->DetachSEEffect( m_ParentSkillInfo, m_StateBlow.emBlowIndex, GetBlowID() );
	_DetachGraphicEffect();
#endif
	OutputDebug( "CDnPoisonBlow::OnEnd, Value:%2.2f (HP : %d ) \n", m_fValue, m_hActor->GetHP());
//#endif
}

#if defined(_GAMESERVER)
void CDnPoisonBlow::OnDuplicate( const STATE_BLOW& StateBlowInfo )
{
	// Calculate poison damage once initially and keep deducting it.
	if( m_ParentSkillInfo.hSkillUser )
	{
		if( -1.0f == m_fDamageProb )
		{
			CDnDamageBase::SHitParam HitParam;
			HitParam.cAttackType = 1;					// Skill damage is magic attack
			HitParam.HasElement = CDnState::Dark;
			m_fIntervalDamage = m_hActor->PreCalcDamage( m_ParentSkillInfo.hSkillUser.GetPointer(), HitParam, 1.0f, m_fValue );
		}
	}
}

bool CDnPoisonBlow::CalcDuplicateValue( const char* szValue )
{
	float fValue = 0.0f;
	float fDamageProb = 0.0f;

#if defined(PRE_FIX_53266)
	float fAddDamageValue = 0.0f;

	std::string str = szValue;//"DamageProb;DamageRatio;ExtraDamage", or "Damage";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);

	int nTokenSize = (int)tokens.size();
	//If setting value is 1, apply attack power as before?
	if (1 == nTokenSize)
	{
		fValue = (float)atof( tokens[0].c_str() );
		fDamageProb = -1.0f;

		m_fValue += fValue;
	}
	else if (nTokenSize >= 2)
	{
		fValue = (float)atof(tokens[0].c_str());
		fDamageProb = (float)atof(tokens[1].c_str());

		if (nTokenSize >= 3)
			fAddDamageValue = (float)atof(tokens[2].c_str());

		m_fValue += fValue;
		m_fDamageProb += fDamageProb;
		m_fDamageProb = min(1.0f, m_fDamageProb);

		m_fIntervalDamage = CalcIntervalDamage(CDnState::Dark);
	}
	else
	{
		OutputDebug("%s Invalid Value %s\n", __FUNCTION__, szValue);
	}
#else
	string strValue(szValue);
	int iSemiColonIndex = (int)strValue.find_first_of( ';', 0 );
	bool bDamageProbMode = (string::npos != iSemiColonIndex);
	if( bDamageProbMode )
	{
		string strProb = strValue.substr( 0, iSemiColonIndex );
		string strDamageProb = strValue.substr( iSemiColonIndex+1, strValue.length() );

		// Probability
		fValue = (float)atof( strProb.c_str() );
		fDamageProb = (float)atof( strDamageProb.c_str() );

		m_fValue += fValue;
		m_fDamageProb += fDamageProb;
		if( 1.0f < m_fDamageProb )
			m_fDamageProb = 1.0f;

		_ASSERT( 0.0f <= m_fDamageProb );
#if defined(PRE_ADD_SKILLBUF_RENEW)
		m_fIntervalDamage = CalcIntervalDamage(CDnState::Dark);
#else
		m_fIntervalDamage = m_hActor->GetLastDamage() * m_fDamageProb;
#endif // PRE_ADD_SKILLBUF_RENEW
	}
	else
	{
		fValue = (float)atof( szValue );
		fDamageProb = -1.0f;

		m_fValue += fValue;
	}
#endif // PRE_FIX_53266

	return true;
}

#if defined(PRE_ADD_SKILLBUF_RENEW)
float CDnPoisonBlow::CalcIntervalDamage(CDnState::ElementEnum haElement)
{
	float fCalcDamage = 1.0f;

	//Magic attack power..
	int HitterAttackValue[2] = {0, };
	CDnState *pState = NULL;
	if (m_ParentSkillInfo.hSkillUser)
		pState = static_cast<CDnActorState *>(m_ParentSkillInfo.hSkillUser.GetPointer());

	if (pState)
	{
		//Get the skill user's magic attack power (ignore skill-based increase?, include buffs?).
		if (m_ParentSkillInfo.hSkillUser)
		{
			HitterAttackValue[0] = m_ParentSkillInfo.hSkillUser->GetAttackMMinWithoutSkill();//pState->GetAttackMMin();
			HitterAttackValue[1] = m_ParentSkillInfo.hSkillUser->GetAttackMMaxWithoutSkill();//pState->GetAttackMMax();
		}

		//////////////////////////////////////////////////////////////////////////
		// Elemental weight
		float fElementWeight = 1.f;
		fElementWeight = ( ( 1.f + pState->GetElementAttack( haElement ) ) * ( 1.f - m_hActor->GetElementDefense( haElement ) ) );
#ifndef PRE_ADD_BUFF_STATE_LIMIT
		fElementWeight = max( fElementWeight, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::ElementMinRevision ) );
#endif
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		//Defense power.
		float fDefensePower = 1.f;
		float fDefenseWeight = 0.0f;
		int nDefense = 0;

		nDefense = m_hActor->GetDefenseM();
		fDefenseWeight = m_ParentSkillInfo.hSkillUser->GetDefenseConstant();

		fDefensePower = nDefense / fDefenseWeight;
		if( fDefensePower < 0.f ) fDefensePower = 0.f;
		else if( fDefensePower > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DefenseMax ) ) 
			fDefensePower = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DefenseMax );

		fDefensePower = 1.f - fDefensePower;
		//////////////////////////////////////////////////////////////////////////

		fCalcDamage = (float)GetGaussianRandom( HitterAttackValue[0], HitterAttackValue[1], m_hActor->GetRoom() ) * fElementWeight;

		fCalcDamage = fCalcDamage * m_fDamageProb * fDefensePower;
#if defined(PRE_FIX_53266)
		fCalcDamage += m_fAddDamageValue;
#endif // PRE_FIX_53266
	}
	else
		fCalcDamage = 1.0f;

	return fCalcDamage;
}

void CDnPoisonBlow::OnSetParentSkillInfo()
{
	m_fIntervalDamage = CalcIntervalDamage(CDnState::Dark);
}
#endif // PRE_ADD_SKILLBUF_RENEW

#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnPoisonBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	int nValueCount = 0;

	//Required value variable
	float fValue[2];
	float fDamageProb[2];


	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() > 1 ) {
		fValue[0] = (float)atof( vlTokens[0][0].c_str() );
		fDamageProb[0] = (float)atof( vlTokens[0][1].c_str() );

		nValueCount = 2;
	}
	else 
	{
		fValue[0] = (float)atof( szOrigValue );
		fDamageProb[0] = -1.0f;

		nValueCount = 1;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() > 1 ) {
		fValue[1] = (float)atof( vlTokens[1][0].c_str() );
		fDamageProb[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	else 
	{
		fValue[1] = (float)atof( szOrigValue );
		fDamageProb[1] = -1.0f;
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] + fValue[1];
	float fResultDamageProb = fDamageProb[0] + fDamageProb[1];

	if (nValueCount == 2)
	{
		sprintf_s(szBuff, "%f;%f", fResultValue, fResultDamageProb);
	}
	else
	{
		sprintf_s(szBuff, "%f", fResultValue);
	}

	szNewValue = szBuff;
}

void CDnPoisonBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	int nValueCount = 0;

	//Required value variable
	float fValue[2];
	float fDamageProb[2];


	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() > 1 ) {
		fValue[0] = (float)atof( vlTokens[0][0].c_str() );
		fDamageProb[0] = (float)atof( vlTokens[0][1].c_str() );

		nValueCount = 2;
	}
	else 
	{
		fValue[0] = (float)atof( szOrigValue );
		fDamageProb[0] = -1.0f;

		nValueCount = 1;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() > 1 ) {
		fValue[1] = (float)atof( vlTokens[1][0].c_str() );
		fDamageProb[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	else 
	{
		fValue[1] = (float)atof( szOrigValue );
		fDamageProb[1] = -1.0f;
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] - fValue[1];
	float fResultDamageProb = fDamageProb[0] - fDamageProb[1];

	if (nValueCount == 2)
	{
		sprintf_s(szBuff, "%f;%f", fResultValue, fResultDamageProb);
	}
	else
	{
		sprintf_s(szBuff, "%f", fResultValue);
	}

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW