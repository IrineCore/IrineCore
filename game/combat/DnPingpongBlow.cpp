#include "stdafx.h"
#include "DnPingpongBlow.h"
#include "DnProjectile.h"
#include "EtActionSignal.h"
#include "DnPlayerActor.h"
#include <float.h>
#if _GAMESERVER
#include "DNGameServerManager.h"
#endif

#if defined(_CLIENT)
#include "DnHideMonsterActor.h"
#include "DnPetActor.h"
#endif // _CLIENT

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnPingpongBlow::CDnPingpongBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
	m_iMaxCount( 0 ),
	m_fRange( 0.0f ),
	m_fHitApplyPercent( -1.0f ),
	CHAINATTACK_PROJECTILE_SPEED( 1000.0f )
	, m_iRootAttackerTeam( 0 )
#ifndef _GAMESERVER
	, m_iLeaveCountFromServer( 0 )
#endif
{
#ifndef _GAMESERVER
	ZeroMemory( &m_ProjectileSignalInfo, sizeof(m_ProjectileSignalInfo) );
#endif

	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_208;
	SetValue( szValue );
	m_fValue = 0.0f;

	std::string str = szValue;//"MaxHitCount;Range(cm);Ratio";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Category
	TokenizeA(str, tokens, delimiters);

	if (tokens.size() != 3)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szValue);
		m_iMaxCount = 5;
		m_fRange = 1000.0f;
		m_fHitApplyPercent = 0.15f;
	}
	else
	{
		m_iMaxCount = atoi(tokens[0].c_str());
		m_fRange = (float)atoi(tokens[1].c_str());
		m_fHitApplyPercent = (float)atof(tokens[2].c_str());
	}

#ifdef _GAMESERVER
	m_nSkillDamage = 0;
	if( m_hActor )
	{
		CDnDamageBase::SHitParam *pHitParam = m_hActor->GetHitParam();
		if (pHitParam)
		{
			if (pHitParam->nCalcDamage == 0 && m_ParentSkillInfo.hPrevAttacker)
				pHitParam = m_ParentSkillInfo.hPrevAttacker->GetHitParam();

			if (pHitParam)
				m_nSkillDamage = pHitParam->nCalcDamage;
		}
	}
#endif

}

CDnPingpongBlow::~CDnPingpongBlow(void)
{

}

void CDnPingpongBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{

}

void CDnPingpongBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	//// Add a time gap for debugging.
	//m_fElapsedTime += fDelta;
	//if( 1.0f > m_fElapsedTime )
	//	return;

	// Note: Attack allies using the received projectile info as-is. Executed independently on server and client.
	// If lag occurs, the hit position may differ slightly from the actual client.
	// When attacking, must launch while accounting for the original attacking actor's attack power.

	
#if _GAMESERVER
	Process_Server(LocalTime, fDelta);

#else
	Process_Client(LocalTime, fDelta);
#endif
}


void CDnPingpongBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
#if _GAMESERVER
	OutputDebug( "[%x] CDnPingpongBlow::OnEnd, LeaveCount: %d\n", this, m_ParentSkillInfo.iLeaveCount );
#else
	OutputDebug( "[%x] CDnPingpongBlow::OnEnd", this );
#endif
}

// Ignore duplicate ping-pong state effects.
void CDnPingpongBlow::OnDuplicate( const STATE_BLOW& StateBlowInfo )
{

}

#if _GAMESERVER
void CDnPingpongBlow::WriteAdditionalPacket( void )
{
	_ASSERT( m_ParentSkillInfo.hSkillUser );
	m_hRootAttacker = m_ParentSkillInfo.hSkillUser;
	m_iRootAttackerTeam = m_hRootAttacker->GetTeam();

	// Unique ID of the first attacking actor
	DWORD dwRootAttackerActorUniqueID = m_hRootAttacker->GetUniqueID();
	m_pPacketStream->Write( &dwRootAttackerActorUniqueID, sizeof(DWORD) );

	DWORD dwPrevAttackerActorUniqueID = m_ParentSkillInfo.hPrevAttacker ? m_ParentSkillInfo.hPrevAttacker->GetUniqueID() : UINT_MAX;
	m_pPacketStream->Write( &dwPrevAttackerActorUniqueID, sizeof(DWORD) );

	// Unique info needed to locate the projectile signal on the client.
	m_pPacketStream->Write( &m_ParentSkillInfo.iProjectileShootActionIndex, sizeof(int) );
	m_pPacketStream->Write( &m_ParentSkillInfo.iProjectileSignalArrayIndex, sizeof(int) );
	m_pPacketStream->Write( &m_ParentSkillInfo.iLeaveCount, sizeof(int) );
}

void CDnPingpongBlow::Process_Server(LOCAL_TIME LocalTime, float fDelta)
{
	// Fires a projectile when an ally enters within a set distance; removes state effect from self.
	DNVector(DnActorHandle) vlActorsInRange;

	// If this is the first outgoing chain attack, record the total count.
	if( -1 == m_ParentSkillInfo.iLeaveCount )
		m_ParentSkillInfo.iLeaveCount = m_iMaxCount;

	// When count reaches limit, stop propagating and end here.
	if( m_ParentSkillInfo.iLeaveCount <= 0 )
	{
		SetState( STATE_BLOW::STATE_END );
		return;
	}

	CDnActor::ScanActor( m_hActor->GetRoom(), *m_hActor->GetPosition(), m_fRange, vlActorsInRange );

	float fShortestDistanceSQ = FLT_MAX;
	DnActorHandle hActorToAttack;
	DWORD dwNumActors = (DWORD)vlActorsInRange.size();
	for( DWORD dwActor = 0; dwActor < dwNumActors; ++dwActor )
	{
		DnActorHandle hActor = vlActorsInRange.at( dwActor );
		if (!hActor)
			continue;

		if( false == (hActor->IsShow() && hActor->IsProcess()) )
			continue;

		if (hActor->IsDie() || hActor->IsNpcActor() )
			continue;

		if( hActor->IsPlayerActor() )
		{
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
			if( pPlayer->IsSpectatorMode() )
				continue;
		}
		// Do not pass the state effect back to the actor that just passed it to me.
		if( m_ParentSkillInfo.hPrevAttacker != hActor )
		{
			// #30643 Everyone except me is an enemy
			if( m_iRootAttackerTeam != hActor->GetTeam() &&
				hActor != m_hActor )
			{
				EtVector3 vDistance = (*m_hActor->GetPosition()) - (*hActor->GetPosition());

				float fLengthSQ = EtVec3LengthSq( &vDistance );
				if( fLengthSQ < fShortestDistanceSQ )
				{
					fShortestDistanceSQ = fLengthSQ;
					hActorToAttack = hActor;
				}
			}
		}
	}

	//When no new actor to transfer to is found.
	if (m_ParentSkillInfo.iLeaveCount > 0 && !hActorToAttack)
	{
		if (m_ParentSkillInfo.hSkillUser)
		{
			//Additional damage
			float fAddDamageRate = m_ParentSkillInfo.iLeaveCount * m_fHitApplyPercent;
			int nAddDamage = (int)(m_nSkillDamage * fAddDamageRate);
			if (nAddDamage > 0)
			{
				char buffer[65];
				_itoa_s(nAddDamage, buffer, 65, 10 );

				m_hActor->CmdAddStateEffect(&m_ParentSkillInfo, STATE_BLOW::BLOW_177, 0, buffer, false, false);
			}
		}
	}
	else

	// 01/23 Added a NULL check for now because m_pProjectileSignalInfo can be NULL and cause a crash dump.
	// 2010.3.19 Fixed suspected game server crash/dump cause.
	if( hActorToAttack && m_hRootAttacker )
	{
		ProjectileStruct* pProjectileSignalInfo = &m_ProjectileSignalInfo;

		// Misc settings for firing projectiles.
		DnSkillHandle hSkill = m_hRootAttacker->FindSkill( m_ParentSkillInfo.iSkillID );

		// Set to designate the target via homing.
		pProjectileSignalInfo->nOrbitType = CDnProjectile::Homing;
		pProjectileSignalInfo->nTargetType = CDnProjectile::Target;
		pProjectileSignalInfo->VelocityType = CDnProjectile::Accell;
		pProjectileSignalInfo->fSpeed = CHAINATTACK_PROJECTILE_SPEED;
		pProjectileSignalInfo->nValidTime = 5000;
		//CDnProjectile* pProjectile = CDnProjectile::CreateProjectile( m_hActor->GetRoom(), hActorToAttack,
		//															  *m_hActor->GetMatEx(), pProjectileSignalInfo.get() );

		MatrixEx Cross = *m_hActor->GetMatEx();
		Cross.m_vPosition.y += m_hActor->GetHeight() / 2.0f;

		CDnProjectile *pProjectile = new CDnProjectile( GetRoom(), m_hRootAttacker );

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
		pProjectile->SetWeaponLength( nLength );

		pProjectile->SetWeaponType( (CDnWeapon::WeaponTypeEnum)( pProjectile->GetWeaponType() | CDnWeapon::Projectile ) );

		pProjectile->SetSpeed( pProjectileSignalInfo->fSpeed );

		pProjectile->SetTargetPosition( *hActorToAttack->GetPosition() );
		pProjectile->SetTargetActor( hActorToAttack );

		pProjectile->Initialize( Cross, static_cast<CDnProjectile::OrbitTypeEnum>(pProjectileSignalInfo->nOrbitType), 
			static_cast<CDnProjectile::DestroyOrbitTypeEnum>(pProjectileSignalInfo->nDestroyOrbitType), 
			static_cast<CDnProjectile::TargetTypeEnum>(pProjectileSignalInfo->nTargetType) );
		pProjectile->SetValidTime( pProjectileSignalInfo->nValidTime );
		pProjectile->SetVelocityType( static_cast<CDnProjectile::VelocityTypeEnum>(pProjectileSignalInfo->VelocityType) );

		pProjectile->SetParentSkill( hSkill );

		m_ParentSkillInfo.hPrevAttacker = m_hActor;
		m_ParentSkillInfo.iLeaveCount -= 1; // Decrement configured count by one.
		pProjectile->SetParentSkillInfo( m_ParentSkillInfo );
		pProjectile->FromSkill( true );

		//pProjectile->SetHitApplyPercent( m_fHitApplyPercent );

		int iNumSE = hSkill->GetStateEffectCount();
		for( int i = 0; i < iNumSE; ++i )
		{
			const CDnSkill::StateEffectStruct* pStateEffect = hSkill->GetStateEffectFromIndex( i );
			if( pStateEffect->ApplyType == CDnSkill::ApplySelf ) 
				continue;

			CDnSkill::StateEffectStruct SE = *pStateEffect;

			// For chain attack state effect, subtract remaining time appropriately and attach the state effect to the projectile before sending.
			if( STATE_BLOW::BLOW_208 == pStateEffect->nID )
				SE.nDurationTime = int(GetDurationTime() * 1000);

			pProjectile->AddStateEffect( SE );

		}
		
		// Set the projectile with the original shooter's attack power info.
		boost::shared_ptr<CDnState> pActorStateSnapshotToPass = boost::shared_ptr<CDnState>(new CDnState);
		*pActorStateSnapshotToPass = m_RootAttackerState;
		pProjectile->SetShooterStateSnapshot( pActorStateSnapshotToPass );
		pProjectile->PostInitialize();

#if defined(PRE_FIX_65287)

		float fFinalDamageRate = 0.0f;
		if (m_hRootAttacker && m_hRootAttacker->IsAppliedThisStateBlow(STATE_BLOW::BLOW_050))
		{
			DNVector(DnBlowHandle) vlhBlows;
			m_hRootAttacker->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_050, vlhBlows );
			int iNumBlow = (int)vlhBlows.size();
			for( int i = 0; i < iNumBlow; ++i )
			{
				fFinalDamageRate += vlhBlows[i]->GetFloatValue();
			}
		}

		pProjectile->SetShooterFinalDamageRate(fFinalDamageRate);
#endif // PRE_FIX_65287
	}

	// Launch projectile at nearby ally and then end state effect.
	SetState( STATE_BLOW::STATE_END );
}
#else
void CDnPingpongBlow::Process_Client(LOCAL_TIME LocalTime, float fDelta)
{
	// When count reaches limit, stop propagating and end here.
	if( 0 == m_iLeaveCountFromServer )
	{
		SetState( STATE_BLOW::STATE_END );
		return;
	}

	// Fires a projectile when an ally enters within a set distance; removes state effect from self.
	DNVector(DnActorHandle) vlActorsInRange;

	CDnActor::ScanActor( *m_hActor->GetPosition(), m_fRange, vlActorsInRange );

	float fShortestDistanceSQ = FLT_MAX;
	DnActorHandle hActorToAttack;
	DWORD dwNumActors = (DWORD)vlActorsInRange.size();
	for( DWORD dwActor = 0; dwActor < dwNumActors; ++dwActor )
	{
		DnActorHandle hActor = vlActorsInRange.at( dwActor );

		if (!hActor)
			continue;

		if (hActor->IsDie() || hActor->IsNpcActor() )
			continue;

		if( hActor->IsPlayerActor() )
		{
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
			if( pPlayer->IsSpectatorMode() )
				continue;
		}
		// #32115 Skip actors created as invisible monsters for a specific purpose.
		if( hActor->IsMonsterActor() )
		{
			if( NULL != dynamic_cast<CDnHideMonsterActor*>(hActor.GetPointer()) )
				continue;
		}

		//Pets are not created on the server..
		if ( NULL != dynamic_cast<CDnPetActor*>(hActor.GetPointer()))
			continue;

		// Do not pass the state effect back to the actor that just passed it to me.
		if( m_hPrevAttacker != hActor )
		{
			//Transfer if on a different team from the skill caster.
			if( m_ParentSkillInfo.hSkillUser && hActor->GetTeam() != m_ParentSkillInfo.hSkillUser->GetTeam() &&
				hActor != m_hActor )
			{
				EtVector3 vDistance = (*m_hActor->GetPosition()) - (*hActor->GetPosition());

				float fLengthSQ = EtVec3LengthSq( &vDistance );
				if( fLengthSQ < fShortestDistanceSQ )
				{
					fShortestDistanceSQ = fLengthSQ;
					hActorToAttack = hActor;
				}
			}
		}
	}

	/*
	//When no new actor to transfer to is found.
	if (m_ParentSkillInfo.iLeaveCount > 0 && !hActorToAttack)
	{
		
	}
	else
	*/
	if( hActorToAttack && m_hRootAttacker )
	{
		// Misc settings for firing projectiles.
		DnSkillHandle hSkill = m_hRootAttacker->FindSkill( m_ParentSkillInfo.iSkillID );

		// Set to designate the target via homing.
		m_ProjectileSignalInfo.nOrbitType = CDnProjectile::Homing;
		m_ProjectileSignalInfo.nTargetType = CDnProjectile::Target;
		m_ProjectileSignalInfo.VelocityType = CDnProjectile::Accell;
		m_ProjectileSignalInfo.fSpeed = CHAINATTACK_PROJECTILE_SPEED;
		m_ProjectileSignalInfo.nValidTime = 5000;
		CDnProjectile* pProjectile = CDnProjectile::CreateProjectile( m_hRootAttacker, *m_hActor->GetMatEx(), &m_ProjectileSignalInfo, 
										EtVector3( 0.0f, 0.0f, 0.0f), hActorToAttack, NULL, false );
		if( pProjectile )
		{
			pProjectile->SetTargetActor( hActorToAttack );
			pProjectile->SetChainShooter( m_hActor );
		}
	}
	// Launch projectile at nearby ally and then end state effect.
	SetState( STATE_BLOW::STATE_END );
}

void CDnPingpongBlow::ReadyForChainAttack( DWORD dwRootAttackerActorID, DWORD dwPrevAttackerActorID, int iActionIndex, int iProjectileSignalArrayIndex )
{
	// Find actor, action, and projectile index, then set projectile signal info.
	m_hRootAttacker = CDnActor::FindActorFromUniqueID( dwRootAttackerActorID );
	m_hPrevAttacker = CDnActor::FindActorFromUniqueID( dwPrevAttackerActorID );
	if( m_hRootAttacker )
	{
		bool bFound = false;

		/*
		CEtActionBase::ActionElementStruct* pActionElement = m_hRootAttacker->GetElement( iActionIndex );
		DWORD dwNumSignalList = (DWORD)pActionElement->pVecSignalList.size();
		for( DWORD dwSignal = 0; dwSignal < dwNumSignalList; ++dwSignal )
		{
			CEtActionSignal* pSignal = pActionElement->pVecSignalList.at( dwSignal );
			if( pSignal->GetSignalListArrayIndex() == iProjectileSignalArrayIndex )
			{
				m_ProjectileSignalInfo = *(static_cast<ProjectileStruct*>(pSignal->GetData()));
				bFound = true;
				break;
			}
		}
		*/
		CEtActionSignal *pSignal = m_hRootAttacker->GetSignal( iActionIndex, iProjectileSignalArrayIndex );
		if( pSignal && pSignal->GetSignalIndex() == STE_Projectile ) {
#ifdef PRE_FIX_MEMOPT_SIGNALH
			CopyShallow_ProjectileStruct(m_ProjectileSignalInfo, static_cast<ProjectileStruct*>(pSignal->GetData()));
#else
			m_ProjectileSignalInfo = *(static_cast<ProjectileStruct*>(pSignal->GetData()));
#endif
			bFound = true;
		}

		_ASSERT( bFound && "CDnPingpongBlow::SetProjectileSignal() -> Projectile signal not found. Server and client action files may differ!" );
		if( false == bFound )
		{
			SetState( STATE_BLOW::STATE_END );
			OutputDebug( "CDnPingpongBlow::SetProjectileSignal Failed!! - Can't found Projectile Signal\n" );
		}
	}
	else
	{
		// Remove state effect because the initial attacker actor could not be found.
		SetState( STATE_BLOW::STATE_END );
		OutputDebug( "CDnPingpongBlow::SetProjectileSignal Failed!! - Can't found Root Attacker Actor Handle\n" );
	}
}

void CDnPingpongBlow::OnReceiveAddPacket( CPacketCompressStream& PacketStream )
{
	// If it is a chain attack state effect, process the additionally received packet.
	DWORD dwRootAttackerActorUniqueID = UINT_MAX;
	DWORD dwPrevAttackerActorUniqueID = UINT_MAX;
	int iShootActionIndex = -1;
	int iProjectileSignalArrayIndex = -1;

	PacketStream.Read( &dwRootAttackerActorUniqueID, sizeof(DWORD) );
	PacketStream.Read( &dwPrevAttackerActorUniqueID, sizeof(DWORD) );
	PacketStream.Read( &iShootActionIndex, sizeof(int) );
	PacketStream.Read( &iProjectileSignalArrayIndex, sizeof(int) );
	PacketStream.Read( &m_iLeaveCountFromServer, sizeof(int) );

	ReadyForChainAttack( dwRootAttackerActorUniqueID, dwPrevAttackerActorUniqueID, 
						 iShootActionIndex, iProjectileSignalArrayIndex );
}

#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnPingpongBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int iMaxCount[2] = {0, };
	float fRange[2] = {0.0f, };
	float fHitApplyPercent[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 3 ) 
	{
		iMaxCount[0] = atoi(vlTokens[0][0].c_str());
		fRange[0] = (float)atof( vlTokens[0][1].c_str() );
		fHitApplyPercent[0] = (float)atof( vlTokens[0][2].c_str() );		
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 3 ) 
	{
		iMaxCount[1] = atoi(vlTokens[1][0].c_str());
		fRange[1] = (float)atof( vlTokens[1][1].c_str() );
		fHitApplyPercent[1] = (float)atof( vlTokens[1][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int iResultMaxCount = iMaxCount[0] + iMaxCount[1];
	float fResultRange = max(fRange[0], fRange[1]);
	float fResultHitApplyPercent = fHitApplyPercent[0] + fHitApplyPercent[1];

	sprintf_s(szBuff, "%d;%f;%f", iResultMaxCount, fResultRange, fResultHitApplyPercent);

	szNewValue = szBuff;
}

void CDnPingpongBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int iMaxCount[2] = {0, };
	float fRange[2] = {0.0f, };
	float fHitApplyPercent[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 3 ) 
	{
		iMaxCount[0] = atoi(vlTokens[0][0].c_str());
		fRange[0] = (float)atof( vlTokens[0][1].c_str() );
		fHitApplyPercent[0] = (float)atof( vlTokens[0][2].c_str() );		
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 3 ) 
	{
		iMaxCount[1] = atoi(vlTokens[1][0].c_str());
		fRange[1] = (float)atof( vlTokens[1][1].c_str() );
		fHitApplyPercent[1] = (float)atof( vlTokens[1][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int iResultMaxCount = iMaxCount[0] - iMaxCount[1];
	float fResultRange = min(fRange[0], fRange[1]);
	float fResultHitApplyPercent = fHitApplyPercent[0] - fHitApplyPercent[1];

	sprintf_s(szBuff, "%d;%f;%f", iResultMaxCount, fResultRange, fResultHitApplyPercent);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
