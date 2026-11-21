#include "stdafx.h"
#include "DnElectricShockBlow.h"
#include "DnMonsterActor.h"
#include "DnTableDB.h"
#include "DnCantMoveBlow.h"
#include "DnCantActionBlow.h"
#include "DnBasicBlow.h"
#include "DnMonsterActor.h"
#include "DnWorld.h"
#ifdef _GAMESERVER
#include "DNGameRoom.h"
#include "DNGameDataManager.h"
#include "DNUserSession.h"
#else
#include "DnCamera.h"
#endif

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

const LOCAL_TIME STIFF_CHECK_INTERVAL = 5000;
const int DEFAULT_STIFF_PROBABILITY = 150;					// 15% chance of stagger every 5 seconds (in units of percentage * 10)
const int ELECTRIC_ACTION_FRAME = 18;						// Electrocution action frame

#define SHOCK_ACTION_NAME "Shock"
#define AIR_SHOCK_ACTION_NAME "Hit_AirBounce"
#define DOWN_SHOCK_ACTION_NAME "Down_SmallBounce"
#define TRANSFER_FACTOR_NUMBER	3

#if defined(_GAMESERVER)
extern int GetGaussianRandom( int nMin, int nMax, CMultiRoom *pRoom );
#endif // _GAMESERVER

CDnElectricShockBlow::CDnElectricShockBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																						  m_StartTime( 0 ),
																						  m_LastCheckTime( 0 ),
																						  m_LastStiffStartTime( 0 ),
																						  m_fDamageProb( 0.0f ),
																						  m_fIntervalDamage( 0.0f ),
																						  m_bStiff( false ),
																						  m_bAirStiff( false  ),
																						  m_pCantMove( new CDnCantMoveBlow(m_hActor, NULL) ),
																						  m_pCantAction( new CDnCantActionBlow(m_hActor, NULL) ),
																						  m_bMyLocalPlayer( false ),
																						  m_fAddSuccessProb( 0.0f ),
																						  m_fLightResistDelta( 0.0f ),
																						  m_pLightDefenseBlow( NULL ),
																						  m_bBossMonster( false ),
																						  m_bNestMap( false )

{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_043;
	SetValue( szValue );


	// First argument is shock probability. Second argument is light element change ratio (0.0~1.0)
	string strValue(szValue);

	int iSemiColonIndex = (int)strValue.find_first_of( ';', 0 );
	bool bExistingLightResistArg = (string::npos != iSemiColonIndex);
	if( bExistingLightResistArg )
	{
		std::vector< std::string > tokens;
		TokenizeA( strValue, tokens, std::string(";") );
		int nTokenSize = (int)tokens.size();
		if( nTokenSize >= 2  )
		{
			m_fAddSuccessProb = (float)atof( tokens[0].c_str() );
			m_fLightResistDelta = (float)atof( tokens[1].c_str() );

			if (nTokenSize == TRANSFER_FACTOR_NUMBER)
				m_fDamageProb = (float)atof( tokens[2].c_str() );
		}
	}
	else
	{
		m_fAddSuccessProb = (float)atof( strValue.c_str() );
	}

#ifndef _GAMESERVER
	//UseDefaultGraphicEffect( false );
	m_bMyLocalPlayer = (CDnActor::s_hLocalActor == m_hActor);
#endif

	// #28385 Distinguish whether it is a nest map or not, except for pvp.
#ifdef _GAMESERVER
	CDNGameRoom* pGameRoom = static_cast<CDNGameRoom*>(m_hActor->GetRoom());
	if( false == pGameRoom->bIsPvPRoom() )
	{
		UINT uiSessionID = 0;
		pGameRoom->GetLeaderSessionID( uiSessionID );
		CDNUserSession *pUserSession = pGameRoom ? pGameRoom->GetUserSession(uiSessionID) : NULL;

		if( pUserSession )
		{
			const TMapInfo* pMapData = g_pDataManager->GetMapInfo( pUserSession->GetMapIndex() );
			if( pMapData )
#if defined(PRE_ADD_DRAGON_FELLOWSHIP)
				m_bNestMap = CDnBlow::CheckEffectIgnoreMapType(pMapData->MapType, pMapData->MapSubType);
#else	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
				m_bNestMap = (GlobalEnum::MAP_DUNGEON == pMapData->MapType) && 
							 ((GlobalEnum::MAPSUB_NEST == pMapData->MapSubType) ||(GlobalEnum::MAPSUB_NESTNORMAL ==  pMapData->MapSubType));
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
		}
	}
#else
#if defined(PRE_ADD_DRAGON_FELLOWSHIP)
	m_bNestMap = CDnBlow::CheckEffectIgnoreMapType();
#else	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
	CDnWorld* pWorld = CDnWorld::GetInstancePtr();
	m_bNestMap = (CDnWorld::MapTypeEnum::MapTypeDungeon == pWorld->GetMapType()) && 
				 ((CDnWorld::MapSubTypeEnum::MapSubTypeNest == pWorld->GetMapSubType() || CDnWorld::MapSubTypeEnum::MapSubTypeNestNormal == pWorld->GetMapSubType()));
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
#endif

}

CDnElectricShockBlow::~CDnElectricShockBlow(void)
{
	SAFE_DELETE( m_pCantMove );
	SAFE_DELETE( m_pCantAction );
	if( m_pLightDefenseBlow ) {
		m_pLightDefenseBlow->OnEnd( 0, 0.f );
		SAFE_DELETE( m_pLightDefenseBlow );
	}
}

#ifdef _GAMESERVER
bool CDnElectricShockBlow::CanBegin( void )
{
	bool bResult = true;

	// Check light resistance.
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

	// <Target's electrocution evasion rate> = <light elemental resistance x elemental defense value from PlayerWeightTable>
	// Target's actual shock chance = <skill shock rate> - <target shock evasion rate>
	float fAvoidProb = 1.0f - (m_fAddSuccessProb - (m_hActor->GetElementDefense( CDnState::Light ) * fWeight));
	if( _rand(GetRoom()) % 10000 <= int(fAvoidProb * 10000.0f) )
	{
		// Changed so the caller invokes CanBegin and immediately deletes the state effect on failure.
		//SetState( STATE_BLOW::STATE_END );
		OutputDebug( "CDnElectricShockBlow::CanAdd - Shock Fail\n" );
		bResult = false;
	}

	return bResult;
}
#endif


void CDnElectricShockBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	m_bStiff = false;
	m_StartTime = 0;
	m_LastCheckTime = 0;
	m_LastStiffStartTime = 0;

	m_StartTime = LocalTime;
	m_LastCheckTime = LocalTime;

	// #27679 Effect is applied to named, boss, and 8-player nest bosses, but the additional effect of freeze is not actually applied.
	bool m_bIgnoreActionEffect = false;
	if( m_bNestMap )
	{
		if( m_hActor->IsMonsterActor() )
		{
			CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
			if( (CDnMonsterState::Boss == pMonsterActor->GetGrade() ||
				CDnMonsterState::BossHP4 == pMonsterActor->GetGrade() ||
				CDnMonsterState::NestBoss == pMonsterActor->GetGrade() ||
				CDnMonsterState::NestBoss8 == pMonsterActor->GetGrade()) )
			{
				m_bIgnoreActionEffect = true;
			}
		}
	}

	// #51048 Treated the same as nest named-grade monsters when state effect is active.
	// Since the state effect is applied independently, it is unrelated to the map.
	if( m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_235) )
	{
		m_bIgnoreActionEffect = true;
	}

	if( m_bIgnoreActionEffect )
	{
		m_fAddSuccessProb = 0.0f;
		m_bBossMonster = true;
	}

	// Process if light element adjustment stat exists.
	if( 0.0f != m_fLightResistDelta )
	{
		char acResist[ 256 ] = { 0 };
		sprintf_s( acResist, "%2.2f", m_fLightResistDelta );
		m_pLightDefenseBlow = new CDnBasicBlow(m_hActor, acResist);
		m_pLightDefenseBlow->SetBlow( STATE_BLOW::BLOW_038 );
		m_pLightDefenseBlow->SetParentSkillInfo(&m_ParentSkillInfo);
		m_pLightDefenseBlow->OnBegin( LocalTime, fDelta );
	}

	OutputDebug( "CDnElectricShockBlow::OnBegin, Value:%d \n", (int)m_fAddSuccessProb );
}


void CDnElectricShockBlow::OnCmdActionFromPacket( const char* pActionName )
{
	// For monsters, when the client receives an airborne electrocution action from the server, appropriate acceleration must be set so it does not stay floating in midair.
	// For players, when receiving an airborne shock action from the client, the server must set proper acceleration so they do not stay floating.
	if( pActionName )
	{
		if( strcmp( AIR_SHOCK_ACTION_NAME, pActionName ) == 0 )
		{
			m_hActor->SetVelocityY( 3.0f );
			m_hActor->SetDownRemainDelta( 3.0f );
			m_hActor->SetResistanceY( -15.0f );
			
			m_hActor->CancelUsingSkill();

#ifdef _GAMESERVER
			m_hActor->ResetSkillSuperAmmor();
#endif
		}
	}
}

void CDnElectricShockBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{	
	CDnBlow::Process( LocalTime, fDelta );

	// Skip if dead
	if( m_hActor->IsDie() )
		return;

	// If self, process electrocution action and disable movement/actions here,
	// For other players/monsters, process by receiving packets from the server.
	// On client, this routine does not run for other players/monsters either.
#ifndef _GAMESERVER
	if( false == m_bMyLocalPlayer )
		return;
#endif

	if( false == m_bBossMonster )
	{
		if( STIFF_CHECK_INTERVAL < LocalTime - m_LastCheckTime )
		{
			m_LastCheckTime = LocalTime;

#ifdef _GAMESERVER
			//When there is damage.. (dead state is checked above..)
#if defined(PRE_FIX_61382)
			DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
			if (m_fIntervalDamage > 0.0f &&
				hActor &&
				hActor->IsDie() == false)
			{
				CDnState::ElementEnum eBackUpElement = m_ParentSkillInfo.eSkillElement;
				m_ParentSkillInfo.eSkillElement = CDnState::ElementEnum::Light;
				m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), (int)m_fIntervalDamage);
				m_ParentSkillInfo.eSkillElement = eBackUpElement; 

				if (hActor->IsDie())
					hActor->OnDie(m_ParentSkillInfo.hSkillUser);
			}
#else
			if (m_fIntervalDamage > 0.0f && m_hActor->IsDie() == false)
			{
				CDnState::ElementEnum eBackUpElement = m_ParentSkillInfo.eSkillElement;
				m_ParentSkillInfo.eSkillElement = CDnState::ElementEnum::Light;
				m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), (int)m_fIntervalDamage);
				m_ParentSkillInfo.eSkillElement = eBackUpElement; 

				if (m_hActor->IsDie())
					m_hActor->OnDie(m_ParentSkillInfo.hSkillUser);
			}
#endif // PRE_FIX_61382
#endif	// #ifdef _GAMESERVER

			if( false == m_bStiff )
			{
				bool bAir = m_hActor->IsAir();
				if( bAir )
				{
					m_hActor->CmdAction( AIR_SHOCK_ACTION_NAME, 0, 3.0f, true, true );

					// Apply appropriate acceleration.
					m_hActor->SetVelocityY( 3.0f );
					m_hActor->SetDownRemainDelta( 3.0f );
					m_hActor->SetResistanceY( -15.0f );
					
					m_hActor->CancelUsingSkill();
#ifdef _GAMESERVER
					m_hActor->ResetSkillSuperAmmor();
#endif

#ifndef _GAMESERVER
					DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
					hCamera->SetFreeze( true );			// Automatically released on wake-up without needing explicit release
#endif
					m_bAirStiff = true;
				}
				else
				if( m_hActor->IsMovable() || m_hActor->IsAttack() )
				{
					m_hActor->CmdAction( SHOCK_ACTION_NAME );

					if( m_hActor->IsAttack() )
					{
						m_hActor->CancelUsingSkill();
#ifdef _GAMESERVER
						m_hActor->ResetSkillSuperAmmor();
#endif
					}
				}
				else
				if( m_hActor->IsDown() )
					m_hActor->CmdAction( DOWN_SHOCK_ACTION_NAME );

				if (m_pCantMove) m_pCantMove->OnBegin( LocalTime, fDelta );
				if (m_pCantAction) m_pCantAction->OnBegin( LocalTime, fDelta );

				m_LastStiffStartTime = LocalTime;

				if( false == bAir )
					m_bStiff = true;
			}
		}

		if( m_bStiff )
		{
			float fFPS = m_hActor->CDnActionBase::GetFPS();
			if( int((float)ELECTRIC_ACTION_FRAME / fFPS * 1000) < LocalTime - m_LastStiffStartTime )
			{
				if (m_pCantMove) m_pCantMove->OnEnd( LocalTime, fDelta );
				if (m_pCantAction) m_pCantAction->OnEnd( LocalTime, fDelta );

				if( (false == m_hActor->IsDie()) && !(m_hActor->GetState() & CDnActorState::Down) )
					m_hActor->CmdAction( "Stand" );

				m_bStiff = false;
			}
		}
		else
		if( m_bAirStiff )
		{
			float fFPS = m_hActor->CDnActionBase::GetFPS();
			if( int((float)ELECTRIC_ACTION_FRAME / fFPS * 1000) < LocalTime - m_LastStiffStartTime )
			{
				if (m_pCantMove) m_pCantMove->OnEnd( LocalTime, fDelta );
				if (m_pCantAction) m_pCantAction->OnEnd( LocalTime, fDelta );

				m_bAirStiff = false;
			}
		}
	}
}

void CDnElectricShockBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bStiff || m_bAirStiff )
	{
		if (m_pCantMove) m_pCantMove->OnEnd( LocalTime, fDelta );
		if (m_pCantAction) m_pCantAction->OnEnd( LocalTime, fDelta );

		if( m_bStiff )
		{
			if( false == m_hActor->IsDie() && !(m_hActor->GetState() & CDnActorState::Down))
				m_hActor->CmdAction( "Stand" );
		}
	}

	// Process if light element adjustment stat exists.
	if( 0.0f != m_fLightResistDelta && m_pLightDefenseBlow )
	{
		m_pLightDefenseBlow->OnEnd( LocalTime, fDelta );
		SAFE_DELETE( m_pLightDefenseBlow );
	}

	OutputDebug( "CDnElectricShockBlow::OnEnd (Id:%d)\n", GetBlowID() );
}


bool CDnElectricShockBlow::CalcDuplicateValue( const char* szValue )
{
	float fAddSuccessProb = 0.0f;
	float fLightResistDelta;

	// First argument is shock probability. Second argument is light element change ratio (0.0~1.0)
	string strValue( szValue );
	int iSemiColonIndex = (int)strValue.find_first_of( ';', 0 );
	bool bExistingLightResistArg = (string::npos != iSemiColonIndex);
	if( bExistingLightResistArg )
	{
		string strProb = strValue.substr( 0, iSemiColonIndex );
		string strLightResistDelta = strValue.substr( iSemiColonIndex+1, strValue.length() );

		// Probability
		fAddSuccessProb = (float)atof( strProb.c_str() );
		fLightResistDelta = (float)atof( strLightResistDelta.c_str() );

		m_fAddSuccessProb += fAddSuccessProb;
		m_fLightResistDelta += fLightResistDelta;
		
		// Process if light element adjustment stat exists.
		if( m_pLightDefenseBlow )
		{
			m_pLightDefenseBlow->OnEnd( 0, 0.f );
			SAFE_DELETE( m_pLightDefenseBlow );

			char acResist[ 256 ] = { 0 };
			sprintf_s( acResist, "%2.2f", m_fLightResistDelta );
			m_pLightDefenseBlow = new CDnBasicBlow( m_hActor, acResist );
			m_pLightDefenseBlow->OnBegin( 0, 0.0f );
		}
	}
	else
	{
		fAddSuccessProb = (float)atof( strValue.c_str() );

		m_fAddSuccessProb += fAddSuccessProb;
	}

	// #27679 Effect is applied to named, boss, and 8-player nest bosses, but the additional effect of freeze is not actually applied.
	if( m_bNestMap )
	{
		if( m_hActor->IsMonsterActor() )
		{
			CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
			if( (CDnMonsterState::Boss == pMonsterActor->GetGrade() ||
				CDnMonsterState::BossHP4 == pMonsterActor->GetGrade() ||
				CDnMonsterState::NestBoss == pMonsterActor->GetGrade() ||
				CDnMonsterState::NestBoss8 == pMonsterActor->GetGrade()) )
			{
				m_fAddSuccessProb = 0.0f;
				m_bBossMonster = true;
			}
		}
	}

	return true;
}

#if defined(_GAMESERVER)
float CDnElectricShockBlow::CalcIntervalDamage(CDnState::ElementEnum haElement)
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
	}
	else
		fCalcDamage = 1.0f;

	return fCalcDamage;
}

void CDnElectricShockBlow::OnSetParentSkillInfo()
{
	m_fIntervalDamage = CalcIntervalDamage(CDnState::Dark);
}

#endif	// #if defined(_GAMESERVER)

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnElectricShockBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	int nValueCount = 0;

	//Required value variable
	float fAddSuccessProb[2] = {0.0f, };
	float fLightResistDelta[2] = {0.0f, };


	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() > 1 ) {
		fAddSuccessProb[0] = (float)atof( vlTokens[0][0].c_str() );
		fLightResistDelta[0] = (float)atof( vlTokens[0][1].c_str() );

		nValueCount = 2;
	}
	else 
	{
		fAddSuccessProb[0] = (float)atof( szOrigValue );
		fLightResistDelta[0] = 0.0f;

		nValueCount = 1;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() > 1 ) {
		fAddSuccessProb[1] = (float)atof( vlTokens[1][0].c_str() );
		fLightResistDelta[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	else 
	{
		fAddSuccessProb[1] = (float)atof( szOrigValue );
		fLightResistDelta[1] = 0.0f;
	}
	//////////////////////////////////////////////////////////////////////////

	//Calculate the two values.
	float fResultAddSuccessProb = fAddSuccessProb[0] + fAddSuccessProb[1];
	float fResultLightResistDelta = fLightResistDelta[0] + fLightResistDelta[1];

	if (nValueCount == 2)
	{
		sprintf_s(szBuff, "%f;%f", fResultAddSuccessProb, fResultLightResistDelta);
	}
	else
	{
		sprintf_s(szBuff, "%f", fResultAddSuccessProb);
	}

	szNewValue = szBuff;
}

void CDnElectricShockBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	int nValueCount = 0;

	//Required value variable
	float fAddSuccessProb[2] = {0.0f, };
	float fLightResistDelta[2] = {0.0f, };


	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() > 1 ) {
		fAddSuccessProb[0] = (float)atof( vlTokens[0][0].c_str() );
		fLightResistDelta[0] = (float)atof( vlTokens[0][1].c_str() );

		nValueCount = 2;
	}
	else 
	{
		fAddSuccessProb[0] = (float)atof( szOrigValue );
		fLightResistDelta[0] = 0.0f;

		nValueCount = 1;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() > 1 ) {
		fAddSuccessProb[1] = (float)atof( vlTokens[1][0].c_str() );
		fLightResistDelta[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	else 
	{
		fAddSuccessProb[1] = (float)atof( szOrigValue );
		fLightResistDelta[1] = 0.0f;
	}
	//////////////////////////////////////////////////////////////////////////

	//Calculate the two values.
	float fResultAddSuccessProb = fAddSuccessProb[0] - fAddSuccessProb[1];
	float fResultLightResistDelta = fLightResistDelta[0] - fLightResistDelta[1];

	if (nValueCount == 2)
	{
		sprintf_s(szBuff, "%f;%f", fResultAddSuccessProb, fResultLightResistDelta);
	}
	else
	{
		sprintf_s(szBuff, "%f", fResultAddSuccessProb);
	}

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW


#if defined(PRE_FIX_51048)
void CDnElectricShockBlow::RemoveDebufAction(LOCAL_TIME LocalTime, float fDelta)
{
	m_fAddSuccessProb = 0.0f;
	m_bBossMonster = true;

	if( m_bStiff || m_bAirStiff )
	{
		if (m_pCantMove) m_pCantMove->OnEnd(LocalTime, fDelta);
		if (m_pCantAction) m_pCantAction->OnEnd(LocalTime, fDelta);

		SAFE_DELETE( m_pCantMove );
		SAFE_DELETE( m_pCantMove );

		if( m_bStiff )
		{
			if( false == m_hActor->IsDie() && !(m_hActor->GetState() & CDnActorState::Down))
				m_hActor->CmdAction( "Stand" );
		}
	}
}
#endif // PRE_FIX_51048