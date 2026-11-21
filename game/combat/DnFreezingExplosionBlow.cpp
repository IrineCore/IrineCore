#include "stdafx.h"
#include "DnFreezingExplosionBlow.h"
#include "DnMonsterActor.h"
#include "EtActionSignal.h"

#ifdef _GAMESERVER
#include "DNGameRoom.h"
#include "DNGameDataManager.h"
#include "DNUserSession.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#define FREEZING_EXPLOSION_ACTION_NAME "Hit_Freezing"
#ifdef PRE_FIX_70011
#define FREEZING_EXPLOSION_HIT_ACTION_NAME "Hit_Pushed"
#endif // PRE_FIX_70011

CDnFreezingExplosionBlow::CDnFreezingExplosionBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																								  m_pActionElement( NULL ),
																								  m_fFrame( 0.0f )
#ifdef _GAMESERVER
																								 ,m_iOriAttackMMin( 0 ),
																								  m_iOriAttackMMax( 0 ),
																								  m_bFreezingHitAction( false )
#endif
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_117;
	AddCallBackType(SB_ONTARGETHIT);

	SetValue( szValue );
	m_fValue = (float)atof(szValue);

	m_pActionElement = m_hActor->GetElement( FREEZING_EXPLOSION_ACTION_NAME );
	_ASSERT( m_pActionElement && "Burn explosion action info not found." );

	m_ActionDurationTime = 0;

	if (m_pActionElement)
	{
		m_ActionDurationTime = m_pActionElement->dwLength;
		m_vlpSignalList = m_pActionElement->pVecSignalList;
	}
}

CDnFreezingExplosionBlow::~CDnFreezingExplosionBlow(void)
{

}

bool CDnFreezingExplosionBlow::CanBegin( void )
{
	// Cannot start unless the freeze state effect is active.
	bool bCanBegin = m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_041 ) ||
					 m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_144 );	// Also remove frostbite state effect. (#27838);
	
	if( false == bCanBegin )
		OutputDebug( "CDnFreezingExplosionBlow::CanBegin - Freezing Explosion state effect activation failed because Freezing state effect is not active.\n" );

	// Do not trigger when there is no explosion action.
	if( NULL == m_pActionElement )
	{
		bCanBegin = false;
		OutputDebug( "CDnFreezingExplosionBlow::CanBegin - Burn Explosion state effect activation failed because there is no Freezing Explosion action.\n" );
	}

	return bCanBegin;
}

void CDnFreezingExplosionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	// Remove freeze state effect
#ifdef _GAMESERVER

	// #28747 Remove only one of freeze or frostbite state effects.
	if( m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_041 ) )
		m_hActor->CmdRemoveStateEffect( STATE_BLOW::BLOW_041 );

	if( m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_144 ) )
		m_hActor->CmdRemoveStateEffect( STATE_BLOW::BLOW_144 );	// Also remove frostbite state effect. (#27838)

	_ApplyOtherSE( m_hActor );		// Add slow state effect to the subject afflicted by the state effect.

#endif

#ifdef PRE_FIX_70011
	if( m_hActor->GetHitParam()->bSuccessNormalDamage == true && m_hActor->GetHitParam()->szActionName.empty() == true )
		m_hActor->CmdAction( FREEZING_EXPLOSION_HIT_ACTION_NAME );
#endif // PRE_FIX_70011

	// Replaces the affected actor's magic attack power with the magic attack power of the skill caster who applied this state effect for its duration.
#ifdef _GAMESERVER
	_ASSERT( m_ParentSkillInfo.hSkillUser );
	if( !m_ParentSkillInfo.hSkillUser )
	{
		g_Log.Log( LogType::_ERROR, L"CDnFreezingExplosionBlow::OnBegin - Freezing Explosion state effect activation failed because actor is invalid..\n" );
		SetState( STATE_BLOW::STATE_END );
		return;
	}

	m_iOriAttackMMin = m_hActor->GetAttackMMin();
	m_iOriAttackMMax = m_hActor->GetAttackMMax();

	m_hActor->SetAttackMMin( m_ParentSkillInfo.hSkillUser->GetAttackMMin() );
	m_hActor->SetAttackMMax( m_ParentSkillInfo.hSkillUser->GetAttackMMax() );

	// If monster, set up for damage display on the client.
	if( m_hActor && CDnActorState::Reserved6 < m_hActor->GetActorType() )
		static_cast<CDnMonsterActor*>(m_hActor.GetPointer())->SlaveOf( m_ParentSkillInfo.hSkillUser->GetUniqueID() );
#endif

	// Duration is handled as the duration of the explosion action.
	float fFPS = 60.0f;
	//m_StateBlow.fDurationTime = (float)m_pActionElement->dwLength / fFPS;
	m_StateBlow.fDurationTime = m_ActionDurationTime != 0 ? (float)m_ActionDurationTime / fFPS : 0.0f;

	// Add only in normal state since it can terminate immediately
	if (STATE_BLOW::STATE_BEGIN == GetBlowState())
		m_hActor->AddStateBlowSignal(this->GetMySmartPtr());

	OutputDebug( "CDnFreezingExplosionBlow::OnBegin\n" );
}


void CDnFreezingExplosionBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

}


void CDnFreezingExplosionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	m_hActor->SetAttackMMin( m_iOriAttackMMin );
	m_hActor->SetAttackMMax( m_iOriAttackMMax );

	// If monster, restore the settings made for damage display on the client.
	//if( CDnActorState::Reserved6 < m_hActor->GetActorType() )
	if( m_hActor && m_hActor->IsMonsterActor() )
	{
		DWORD dwInvalidMasterActorID = UINT_MAX;
		static_cast<CDnMonsterActor*>(m_hActor.GetPointer())->SlaveOf( dwInvalidMasterActorID );
	}
#endif

	m_hActor->RemoveStateBlowSignal(this->GetMySmartPtr());

	OutputDebug( "CDnFreezingExplosionBlow::OnEnd\n" );
}

void CDnFreezingExplosionBlow::OnTargetHit( DnActorHandle hTargetActor )
{
#ifdef _GAMESERVER
	if( m_bFreezingHitAction )
		_ApplyOtherSE( hTargetActor );
#endif
}

#ifdef _GAMESERVER
void CDnFreezingExplosionBlow::_ApplyOtherSE( DnActorHandle hActor )
{
	// Hardcoded in code as an experimental part for now. (#7842)
	//- Frame increase = 0.5
	//- Move speed reduction = -0.5
	//- State abnormal effect (Frostbite) -> refers to the visual effect.

	// If it is a Nest boss, additional state effects are not added.
	CDNGameRoom* pGameRoom = static_cast<CDNGameRoom*>(m_hActor->GetRoom());
	bool bNestMap = false;
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
				bNestMap = CDnBlow::CheckEffectIgnoreMapType(pMapData->MapType, pMapData->MapSubType);
#else	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
				bNestMap = (GlobalEnum::MAP_DUNGEON == pMapData->MapType) && (GlobalEnum::MAPSUB_NEST == pMapData->MapSubType);
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
		}
	}

	bool bBossMonster = false;
	if( hActor->IsMonsterActor() )
	{
		CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
		if( (CDnMonsterState::Boss == pMonsterActor->GetGrade() ||
			CDnMonsterState::BossHP4 == pMonsterActor->GetGrade() ||
			CDnMonsterState::NestBoss == pMonsterActor->GetGrade() ||
			CDnMonsterState::NestBoss8 == pMonsterActor->GetGrade()) )
		{
			bBossMonster = true;
		}
	}

	//#51048 Prevent action speed reduction effects when state effect 235 is active.
	if( m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_235) )
	{
		bNestMap = true;
		bBossMonster = true;
	}

	if( false == bNestMap ||
		false == bBossMonster )
	{
		// Do not add if a frame or move-speed reduction state effect from the same skill already exists.
		bool bCanAdd = true;
		if( hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_025) &&
			hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_076) )
		{
			DNVector(DnBlowHandle) vlBlows;
			hActor->GetAllAppliedStateBlowBySkillID( m_ParentSkillInfo.iSkillID, vlBlows );
			bCanAdd = vlBlows.empty();
			//for( int i = 0; i < (int)vlBlows.size(); ++i )
			//{
			//	DnBlowHandle hBlow = vlBlows.at( i );
			//}
		}

		if( bCanAdd )
		{
			CDnSkill::SkillInfo SkillInfo = m_ParentSkillInfo;
			char szBuff[128] = {0, };
			_snprintf_s(szBuff, _countof(szBuff), _TRUNCATE, "%d", 22 );
			SkillInfo.szEffectOutputIDToClient = szBuff;
			//SkillInfo.hSkillUser = m_hActor;/*GetMySmartPtr();*/
			hActor->CmdAddStateEffect( &SkillInfo, STATE_BLOW::BLOW_025, 7000, "-0.5" );
			hActor->CmdAddStateEffect( &SkillInfo, STATE_BLOW::BLOW_076, 7000, "-0.5" );
		}
	}
}
#endif
//#ifdef _GAMESERVER
//
//#endif


void CDnFreezingExplosionBlow::SignalProcess( LOCAL_TIME LocalTime, float fDelta )
{
	// Execute only signals without executing the actual action.
	float fFPS = 60.0f;
	float fPrevFrame = m_fFrame;
	m_fFrame += (fDelta * fFPS);

	//if(m_pActionElement->dwLength < (DWORD)m_fFrame )
	if (m_ActionDurationTime < (DWORD)m_fFrame)
	{
		m_fFrame = 1.0f;
		fPrevFrame = -0.1f;

		// Once all actions finish, stop processing signals and end the state effect. 
		// Duration is set to action length, so state effect will end soon.
		return;
	}

	if( 0.0f == fPrevFrame )
		fPrevFrame = -(m_fFrame * 0.1f);

	CEtActionSignal *pSignal = NULL;

#if defined(PRE_FIX_55378)
	m_hActor->SetChargerAction(FREEZING_EXPLOSION_ACTION_NAME);
#endif // PRE_FIX_55378

#if defined(PRE_FIX_62309)
#if defined(_GAMESERVER)
	//#62309
	//Attributes may not be set on Hit via OnSignal while running the signal processor.
	//Set forced attributes before calling the signal processor, and use the set attribute values when processing the Hit signal
	//Force setting..
	m_hActor->PushForceHitElement(m_ParentSkillInfo.eSkillElement);
#endif // _GAMESERVER
#endif // PRE_FIX_62309

	//for( DWORD i=0; i < m_pActionElement->pVecSignalList.size(); i++ )
	for( DWORD i=0; i < m_vlpSignalList.size(); i++ )
	{
		//pSignal = m_pActionElement->pVecSignalList[i];
		pSignal = m_vlpSignalList[i];
		if( pSignal->CheckSignal( fPrevFrame, m_fFrame ) == true ) 
		{
#ifdef _GAMESERVER
			if( STE_Hit == pSignal->GetSignalIndex() )
				m_bFreezingHitAction = true;
#endif

			LOCAL_TIME StartTime = LocalTime - (LOCAL_TIME)( 1000.f / fFPS * ( m_fFrame - pSignal->GetStartFrame() ) );
			LOCAL_TIME EndTime = LocalTime + (LOCAL_TIME)( 1000.f / fFPS * ( pSignal->GetEndFrame() - m_fFrame ) );

			m_hActor->OnSignal( (SignalTypeEnum)pSignal->GetSignalIndex(), pSignal->GetData(), LocalTime, 
				StartTime, EndTime, pSignal->GetSignalListArrayIndex() );

#ifdef _GAMESERVER
			if( m_bFreezingHitAction )
				m_bFreezingHitAction = false;
#endif
		}
	}

#if defined(PRE_FIX_62309)
#if defined(_GAMESERVER)
	m_hActor->PopForceHitElement();
#endif // _GAMESERVER
#endif // PRE_FIX_62309

#if defined(PRE_FIX_55378)
	m_hActor->SetChargerAction(NULL);
#endif // PRE_FIX_55378
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnFreezingExplosionBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnFreezingExplosionBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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