#include "stdafx.h"
#include "DnShockInfectionBlow.h"
#include "EtActionSignal.h"
#include "DnProjectile.h"

#if _GAMESERVER
#include "DNGameServerManager.h"

#else
#include "DnLocalPlayerActor.h"
#include "DnInterface.h"
#include "DnMainDlg.h"

#endif

#define SHOCK_INFECTION_ACTION_NAME "Skill_ShockInfection"


#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnShockInfectionBlow::CDnShockInfectionBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
#ifdef _GAMESERVER
																							, m_dwCoolTime(0)
																							, m_LastActivateTime( 0 )
#else
																							, m_pLocalPlayerActor( NULL )
#endif // #ifdef _GAMESERVER
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_168;
	SetValue( szValue );

	m_fHitApplyPercent = 0.0f;
	m_fLimitRadius = 0.0f;
	m_fShockProb = 0.0f;
	m_nShockDurationTime = 0;

	SecureZeroMemory( &m_ProjectileSignalInfo, sizeof(m_ProjectileSignalInfo) );

	// Total of 4 arguments.
	// ShockChance;ShockDuration;Range;Damage%ForProjectileHitSignal
	string strValue( szValue );

	// After splitting the string by whitespace
	std::vector<string> vlTokens;
	TokenizeA( strValue, vlTokens, ";" );
	bool bValidArgument = (4 == (int)vlTokens.size());
	_ASSERT( bValidArgument && "Invalid Shock Infection state effect argument settings." );

	if( bValidArgument )
	{
		string& strShockProb = vlTokens.at( 0 );
		string& strShockDuration = vlTokens.at( 1 );
		string& strRadius = vlTokens.at( 2 );
		string& strProjectileHitSignalPercent = vlTokens.at( 3 );

		m_fShockProb = (float)atof( strShockProb.c_str() );
		m_nShockDurationTime = (int)atoi( strShockDuration.c_str() );
		m_fLimitRadius = (float)atof( strRadius.c_str() );
		m_fHitApplyPercent = (float)atof( strProjectileHitSignalPercent.c_str() );
		
		//// For testing for now.
		//m_fShockProb = 1.0f;
		//m_nShockDurationTime = 100000;
		//m_fLimitRadius = 1000.0f;
		//m_fHitApplyPercent = 1.0f;
		////////////////////////////////////////////////////////////////////////////
	}

	// Retrieve projectile signal info for electrocution chaining from the prearranged action.
	// Since this state effect is added as self, obtain it from m_hActor.
	CDnActionBase::ActionElementStruct* pActionElement = m_hActor->GetElement( SHOCK_INFECTION_ACTION_NAME );
	bool bValid = false;
	if( pActionElement )
	{
		if( false == pActionElement->pVecSignalList.empty() )
		{
			CEtActionSignal* pSignal = pActionElement->pVecSignalList.front();
			if( STE_Projectile == pSignal->GetSignalIndex() )
			{
#ifdef PRE_FIX_MEMOPT_SIGNALH
				CopyShallow_ProjectileStruct(m_ProjectileSignalInfo, static_cast<ProjectileStruct*>(pSignal->GetData()));
#else
				m_ProjectileSignalInfo = *static_cast<ProjectileStruct*>(pSignal->GetData());
#endif

#ifdef _GAMESERVER
				// Electrocution state effect
				m_AddtionalSE.nID = STATE_BLOW::BLOW_043;
				m_AddtionalSE.ApplyType = CDnSkill::ApplyTarget;
				m_AddtionalSE.nDurationTime = m_nShockDurationTime;
				m_AddtionalSE.szValue = "1.0";
#else
				// This class directly controls whether to attach the state effect visual.
				UseTableDefinedGraphicEffect( false );
#endif // #ifdef _GAMESERVER

				bValid = true;
			}
		}
	}

	// State effect ended immediately due to invalid resource settings.
	if( false == bValid )
	{
		SetState( STATE_BLOW::STATE_END );
	}

	//m_fRate = 1.0f;
	//m_fLimitRadius = 1000.0f;
	//m_nDurationTime = 5000;
	//m_fHitApplyPercent = 1.0f;

	////////////////////////////////////////////////////////////////////////////
	//m_OffSet = EtVector3(0.0f, 0.0f, 0.0f);
	//m_Direction = EtVector3(0.0f, 0.0f, 1.0f);
	//m_DestPos = EtVector3(0.0f, 0.0f, 0.0f);

	//m_nOrbitType = CDnProjectile::Homing;
	//m_nTargetType = CDnProjectile::Target;
	//m_VelocityType = CDnProjectile::Accell;
	//m_fProjectileSpeed = 1000.0f;
	//m_nProjectileValidTime = 5000;
	////////////////////////////////////////////////////////////////////////////

	//memset(&m_ProjectileSignalInfo, 0, sizeof(m_ProjectileSignalInfo));

	//m_ProjectileSignalInfo.nWeaponTableID = 2000;
	//m_ProjectileSignalInfo.nProjectileIndex = -1;
	//m_ProjectileSignalInfo.nOrbitType = CDnProjectile::Homing;
	//m_ProjectileSignalInfo.nTargetType = CDnProjectile::Target;
	//m_ProjectileSignalInfo.nDestroyOrbitType = CDnProjectile::Instantly;
	//m_ProjectileSignalInfo.bTraceHitTarget = FALSE;

	//m_ProjectileSignalInfo.vOffset = &m_OffSet;
	//m_ProjectileSignalInfo.vDirection = &m_Direction;
	//m_ProjectileSignalInfo.vDestPosition = &m_DestPos;
	
	//// Electrocution state effect
	//m_AddtionalSE.nID = STATE_BLOW::BLOW_043;
	//m_AddtionalSE.ApplyType = CDnSkill::ApplyTarget;
	//m_AddtionalSE.nDurationTime = m_nDurationTime;
	//m_AddtionalSE.szValue = "1.0;1.0";

	//m_bCreateProjectile = false;

#ifdef _GAMESERVER
	AddCallBackType( SB_ONTARGETHIT );
#endif
}

CDnShockInfectionBlow::~CDnShockInfectionBlow(void)
{

}

void CDnShockInfectionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	// Use parent skill's cooldown info as-is.
	if( m_ParentSkillInfo.hSkillUser )
	{
		DnSkillHandle hParentSkill = m_ParentSkillInfo.hSkillUser->FindSkill( m_ParentSkillInfo.iSkillID );
		m_dwCoolTime = DWORD(hParentSkill->GetDelayTime() * 1000.0f);
	}
#else
	m_pLocalPlayerActor = dynamic_cast<CDnLocalPlayerActor*>( m_hActor.GetPointer() );
	if( !m_hParentSkillForCoolTime && m_pLocalPlayerActor )
	{
		m_hParentSkillForCoolTime = CDnSkill::CreateSkill( m_hActor, m_ParentSkillInfo.iSkillID, 
#if defined(PRE_FIX_NEXTSKILLINFO)
															m_ParentSkillInfo.nSkillLevel);
#else
														   m_ParentSkillInfo.iSkillLevelID - m_ParentSkillInfo.iSkillLevelIDOffset + 1 );
#endif // PRE_FIX_NEXTSKILLINFO
	}
#endif

	__super::OnBegin(LocalTime, fDelta);
}

void CDnShockInfectionBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#ifdef _GAMESERVER
#else
	if( m_pLocalPlayerActor )
	{
		// Advance cooldown.
		m_hParentSkillForCoolTime->Process( LocalTime, fDelta ); 
	}
#endif // _GAMESERVER

	//if (m_bCreateProjectile)
	//{
	//	CreateProjectile();
	//	m_bCreateProjectile = false;
	//}
	//OutputDebug( "CDnFireBurnBlow::Process, Value:%2.2f (HP : %d ) \n", m_fValue, m_hActor->GetHP());
}

//void CDnShockInfectionBlow::DoShockInfection( DnActorHandle hTargetActor )
//{
//#ifdef _GAMESERVER
//	m_LastActivateTime = m_hActor->GetLocalTime();
//	CreateProjectileOnServer( hTargetActor );
////#else
////	CreateProjectileOnClient();
//#endif
//}

#if defined(_GAMESERVER)
void CDnShockInfectionBlow::DoShockInfection( DnActorHandle hHitTargetActor )
{
	if( !hHitTargetActor )
		return;

	m_LastActivateTime = m_hActor->GetLocalTime();

	DNVector(DnActorHandle) vlActorsInRange;
	if (!hHitTargetActor->ScanActor( hHitTargetActor->GetRoom(), *hHitTargetActor->GetPosition(), m_fLimitRadius, vlActorsInRange ))
		return;
	
	// Select the nearest actor for now.. if other rules are added later, handle them then.
	float fShortestDistanceSQ = FLT_MAX;
	DnActorHandle hActorToInfection;
	DWORD dwNumActors = (DWORD)vlActorsInRange.size();
	for( DWORD dwActor = 0; dwActor < dwNumActors; ++dwActor )
	{
		DnActorHandle hActor = vlActorsInRange.at( dwActor );
		if( false == (hActor->IsShow() && hActor->IsProcess()) )
			continue;

		// Not the same actor
		if( hActor == hHitTargetActor )
			continue;

		// Exclude npc actors
		if( hActor->IsNpcActor() )
			continue;

		// Only when on a different team.. 
		if( hActor->GetTeam() != m_hActor->GetTeam())
		{
			EtVector3 vDistance = (*m_hActor->GetPosition()) - (*hActor->GetPosition());

			float fLengthSQ = EtVec3LengthSq( &vDistance );
			if( fLengthSQ < fShortestDistanceSQ )
			{
				fShortestDistanceSQ = fLengthSQ;
				hActorToInfection = hActor;
			}
		}
	}

	
	if( hActorToInfection )
	{
		ProjectileStruct* pProjectileSignalInfo = &m_ProjectileSignalInfo;

		// Misc settings for firing projectiles.
		DnSkillHandle hSkill = m_hActor->FindSkill( m_ParentSkillInfo.iSkillID );
		if( !hSkill )
			return;

		MatrixEx Cross = *m_hActor->GetMatEx();
		Cross.m_vPosition.y += m_hActor->GetHeight() / 2.0f;

		CDnProjectile *pProjectile = new CDnProjectile( GetRoom(), m_hActor );

		pProjectile->SetPierce( pProjectileSignalInfo->bPierce == TRUE ? true : false );
		pProjectile->SetMaxHitCount( pProjectileSignalInfo->nMaxHitCount );

		if( pProjectileSignalInfo->nWeaponTableID > 0 ) 
		{
			if( pProjectileSignalInfo->nProjectileIndex != -1 )
			{
				DnWeaponHandle hWeapon = CDnWeapon::GetSmartPtr( (CMultiRoom*)g_pGameServerManager->GetRootRoom(), pProjectileSignalInfo->nProjectileIndex );
				if( hWeapon ) *(CDnWeapon*)pProjectile = *hWeapon.GetPointer();
			}
		}

		int nLength = pProjectile->GetWeaponLength();
		if( pProjectileSignalInfo->bIncludeMainWeaponLength ) 
		{
			DnWeaponHandle hWeapon = m_hActor->GetWeapon(0);
			if ( hWeapon )
				nLength += hWeapon->GetWeaponLength();
		}

		// Give a range at least equal to the radius value so electrocution chains reliably.
		if( nLength < (int)m_fLimitRadius )
			nLength = (int)m_fLimitRadius;

		pProjectile->SetWeaponLength( nLength );

		pProjectile->SetWeaponType( (CDnWeapon::WeaponTypeEnum)( pProjectile->GetWeaponType() | CDnWeapon::Projectile ) );

		pProjectile->SetSpeed( pProjectileSignalInfo->fSpeed );

		pProjectile->SetTargetPosition( *hActorToInfection->GetPosition() );
		pProjectile->SetTargetActor( hActorToInfection );

		pProjectile->Initialize( Cross, static_cast<CDnProjectile::OrbitTypeEnum>(pProjectileSignalInfo->nOrbitType), 
										static_cast<CDnProjectile::DestroyOrbitTypeEnum>(pProjectileSignalInfo->nDestroyOrbitType), 
										static_cast<CDnProjectile::TargetTypeEnum>(pProjectileSignalInfo->nTargetType) );
		pProjectile->SetValidTime( pProjectileSignalInfo->nValidTime );
		pProjectile->SetVelocityType( static_cast<CDnProjectile::VelocityTypeEnum>(pProjectileSignalInfo->VelocityType) );

		pProjectile->SetParentSkill( hSkill );

		//m_ParentSkillInfo.hPrevAttacker = m_hActor;
		CDnSkill::SkillInfo ParentSkillInfo = m_ParentSkillInfo;
		
		char szBuff[128] = {0, };
		_snprintf_s(szBuff, _countof(szBuff), _TRUNCATE, "%d", 402 );
		ParentSkillInfo.szEffectOutputIDToClient = szBuff;

		// Since the projectile's shooter is the actor with this state effect and the shock target is an enemy, it must be sent this way for the effect to attach properly on the client.
		// The subject using this state effect has it as self.
		ParentSkillInfo.eTargetType = CDnSkill::TargetTypeEnum::Enemy;
		pProjectile->SetParentSkillInfo( ParentSkillInfo );
		pProjectile->FromSkill( true );

		pProjectile->SetHitApplyPercent( m_fHitApplyPercent );

		// Add electrocution state effect..
		pProjectile->AddStateEffect( m_AddtionalSE );

		boost::shared_ptr<CDnState> pActorStateSnapshotToPass = boost::shared_ptr<CDnState>(new CDnState);
		*pActorStateSnapshotToPass = *static_cast<CDnState*>( m_ParentSkillInfo.hSkillUser.GetPointer() );
		pActorStateSnapshotToPass->SetAttackMMax( m_hActor->GetAttackMMaxWithoutSkill() );
		pActorStateSnapshotToPass->SetAttackMMin( m_hActor->GetAttackMMinWithoutSkill() );
		pProjectile->SetShooterStateSnapshot( pActorStateSnapshotToPass );
		pProjectile->PostInitialize();

#if defined(PRE_FIX_65287)

		float fFinalDamageRate = 0.0f;
		if (m_hActor && m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_050))
		{
			DNVector(DnBlowHandle) vlhBlows;
			m_hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_050, vlhBlows );
			int iNumBlow = (int)vlhBlows.size();
			for( int i = 0; i < iNumBlow; ++i )
			{
				fFinalDamageRate += vlhBlows[i]->GetFloatValue();
			}
		}

		pProjectile->SetShooterFinalDamageRate(fFinalDamageRate);
#endif // PRE_FIX_65287


		// Send a packet to the client to fire a projectile at a nearby close target.
		// Target is the actor selected by the server.
		BYTE pBuffer[ 128 ] = { 0 };
		CPacketCompressStream Stream( pBuffer, 128 );

		DWORD dwTargetActorUniqueID = hHitTargetActor->GetUniqueID();
		DWORD dwActorToInfection = hActorToInfection->GetUniqueID();
		int nBlowID = GetBlowID();

		Stream.Write( &dwTargetActorUniqueID, sizeof(dwTargetActorUniqueID) );
		Stream.Write( &dwActorToInfection, sizeof(dwActorToInfection) );
		Stream.Write( &nBlowID, sizeof(nBlowID) );
		//Stream.Write( &m_bCreateProjectile, sizeof(m_bCreateProjectile));

		m_hActor->Send( eActor::SC_SHOCK_INFECTION, &Stream );
	}
}

#else

void CDnShockInfectionBlow::DoShockInfection( DnActorHandle hHitTargetActor, DnActorHandle hActorToInfection )
{
	if( hHitTargetActor && hActorToInfection )
	{
		// Called when a separate packet is sent from server upon successful cooldown parry.
		// Only show cooldown for the local client.
		if( m_pLocalPlayerActor )
		{
			// Only advance cooldown. For UI display.
			m_hParentSkillForCoolTime->OnBeginCoolTime();
			if( m_hParentSkillForCoolTime->HasCoolTime() )
			{
				CDnMainDlg *pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
				if( pMainDlg ) 
				{
					pMainDlg->AddPassiveSkill( m_hParentSkillForCoolTime );
				}
			}
		}

		// Misc settings for firing projectiles.
		CDnProjectile* pProjectile = CDnProjectile::CreateProjectile( m_hActor,
																	  *hHitTargetActor->GetMatEx(), &m_ProjectileSignalInfo, 
																	  EtVector3( 0.0f, 0.0f, 0.0f), hActorToInfection );
	}
}
#endif // _GAMESERVER


void CDnShockInfectionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);

}

#if defined(_GAMESERVER)
void CDnShockInfectionBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	if (!m_hActor)
		return;

	if (!hTargetActor || hTargetActor->IsDie())
		return;

	if (hTargetActor->IsImmuned(m_StateBlow.emBlowIndex))
		return;

	// Get the hit signal info of my currently active action.
	CDnDamageBase::SHitParam* pHitParameter = m_hActor->GetHitParam();

	// Applied interval criteria match the HitSignal interval.
	// Applied to one hit signal per cooldown.
	// If the first hit flag is on, this hit signal is the first hit, so
	// Start ticking down cooldown. When the next FirstHit flag arrives, cooldown interval...
	// Check and prevent shock transfer if the specified interval has not elapsed.
	bool bNowActivate = true;
	if( pHitParameter->bFirstHit )
	{
		LOCAL_TIME NowLocalTime = m_hActor->GetLocalTime();
		if( NowLocalTime - m_LastActivateTime < m_dwCoolTime )
		{
			bNowActivate = false;
		}
	}

	if( bNowActivate )
	{
		//Skip if not electrocuted..
		if (!hTargetActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_043))
			return;

		//Skip if not a light element attack
		CDnDamageBase::SHitParam* pHitParam = hTargetActor->GetHitParam();
		if( NULL == pHitParam || 
			CDnState::Light != pHitParam->HasElement )
			return;
		
		//Probability calculation..
		bool bExecuteable = rand() % 10000 <= (m_fShockProb * 10000.0f);
		if (!bExecuteable)
			return;

		DoShockInfection( hTargetActor );
	}

	//Send packet to change m_bCreateProjectile of this state effect.
	//m_bCreateProjectile = true;
}
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnShockInfectionBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required variables
	float fShockProb[2] = {0.0f, };
	int nShockDuration[2] = { 0, };
	float fLimitRadius[2] = { 0.0f, };
	float fHitApplyPercent[2] = { 0.0f, };

	// Total of 4 arguments.
	// ShockChance;ShockDuration;Range;Damage%ForProjectileHitSignal
	std::string strValue( szOrigValue );

	// After splitting the string by whitespace
	std::vector<string> vlTokens[2];
	TokenizeA( strValue, vlTokens[0], ";" );
	
	if( (int)vlTokens[0].size() == 4 )
	{
		fShockProb[0] = (float)atof( vlTokens[0][0].c_str() );
		nShockDuration[0] = (int)atoi( vlTokens[0][1].c_str() );
		fLimitRadius[0] = (float)atof( vlTokens[0][2].c_str() );
		fHitApplyPercent[0] = (float)atof( vlTokens[0][3].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	strValue = szAddValue;
	TokenizeA( strValue, vlTokens[1], ";" );

	if( (int)vlTokens[1].size() == 4 )
	{
		fShockProb[1] = (float)atof( vlTokens[1][0].c_str() );
		nShockDuration[1] = (int)atoi( vlTokens[1][1].c_str() );
		fLimitRadius[1] = (float)atof( vlTokens[1][2].c_str() );
		fHitApplyPercent[1] = (float)atof( vlTokens[1][3].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	float fResultShockProb = fShockProb[0] + fShockProb[1];
	int nResultShockDuration = max(nShockDuration[0], nShockDuration[1]);
	float fResultLimitRadius = max(fLimitRadius[0], fLimitRadius[1]);
	float fResultHitApplyPrecent = fHitApplyPercent[0] + fHitApplyPercent[1];

	sprintf_s(szBuff, "%f;%d;%f;%f", fResultShockProb, nResultShockDuration, fResultLimitRadius, fResultHitApplyPrecent);

	szNewValue = szBuff;
}

void CDnShockInfectionBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required variables
	float fShockProb[2] = {0.0f, };
	int nShockDuration[2] = { 0, };
	float fLimitRadius[2] = { 0.0f, };
	float fHitApplyPercent[2] = { 0.0f, };

	// Total of 4 arguments.
	// ShockChance;ShockDuration;Range;Damage%ForProjectileHitSignal
	std::string strValue( szOrigValue );

	// After splitting the string by whitespace
	std::vector<string> vlTokens[2];
	TokenizeA( strValue, vlTokens[0], ";" );

	if( (int)vlTokens[0].size() == 4 )
	{
		fShockProb[0] = (float)atof( vlTokens[0][0].c_str() );
		nShockDuration[0] = (int)atoi( vlTokens[0][1].c_str() );
		fLimitRadius[0] = (float)atof( vlTokens[0][2].c_str() );
		fHitApplyPercent[0] = (float)atof( vlTokens[0][3].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	strValue = szAddValue;
	TokenizeA( strValue, vlTokens[1], ";" );

	if( (int)vlTokens[1].size() == 4 )
	{
		fShockProb[1] = (float)atof( vlTokens[1][0].c_str() );
		nShockDuration[1] = (int)atoi( vlTokens[1][1].c_str() );
		fLimitRadius[1] = (float)atof( vlTokens[1][2].c_str() );
		fHitApplyPercent[1] = (float)atof( vlTokens[1][3].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	float fResultShockProb = fShockProb[0] - fShockProb[1];
	int nResultShockDuration = min(nShockDuration[0], nShockDuration[1]);
	float fResultLimitRadius = min(fLimitRadius[0], fLimitRadius[1]);
	float fResultHitApplyPrecent = fHitApplyPercent[0] - fHitApplyPercent[1];

	sprintf_s(szBuff, "%f;%d;%f;%f", fResultShockProb, nResultShockDuration, fResultLimitRadius, fResultHitApplyPrecent);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW