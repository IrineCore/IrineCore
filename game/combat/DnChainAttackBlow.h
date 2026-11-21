#pragma once
#include "DnBlow.h"

// #7189
// While creating OBT raid monster Cerberus and configuring various skills, a new effect became needed
// Requesting a new skill effect.
// A [status ailment]-like skill effect that deals damage to nearby allies.
//
// Skill effect name: Chain Attack
// Skill effect index: 60
// Effect description
// While affected by this effect, the target launches the same attack against allies within a certain distance.
// Effect may become stronger or weaker when attacking nearby targets.
// There must be a limit on the number of attacks they can launch at each other.
// Required parameters
// Attack range for attacking allies
// Damage increase % on attack
// Number of repetitions
// It seems we should discuss this together as we proceed.
// Please review the draft and share your feedback.
class CDnChainAttackBlow : public CDnBlow, public TBoostMemoryPool<CDnChainAttackBlow>
{
private:
	// Actor that attacked first
	DnActorHandle m_hRootAttacker;
	int m_iRootAttackerTeam;

	// Distance to check
	float m_fRange;
	// Max count
	int m_iMaxCount;
	// hitprob to apply to the hit signal (0.0 to 1.0)
	float m_fHitApplyPercent;

	//float m_fElapsedTime;

#ifdef _GAMESERVER
	CDnState m_RootAttackerState;
	ProjectileStruct m_ProjectileSignalInfo;
#else
	ProjectileStruct m_ProjectileSignalInfo;
	
	// Actor that last delivered to self.
	DnActorHandle m_hPrevAttacker;
#endif

public:
	CDnChainAttackBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChainAttackBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if !defined(SW_ADD_CHAINATTACK_STATEEFFECT_20091029_jhk8211 )

#if _GAMESERVER

	void SetProjectileSignal( const ProjectileStruct* pProjectileSignal )
	{
		if( NULL == pProjectileSignal )
			return;

#ifdef PRE_FIX_MEMOPT_SIGNALH
		CopyShallow_ProjectileStruct(m_ProjectileSignalInfo, pProjectileSignal);
#else
		m_ProjectileSignalInfo = *pProjectileSignal;
#endif
	}

	void SetRootAttackerState( CDnState* pRootAttackerState )
	{
		if( NULL == pRootAttackerState )
			return;

		m_RootAttackerState = *pRootAttackerState;
	}

	virtual void WriteAdditionalPacket( void );
#else
	void ReadyForChainAttack( DWORD dwRootAttackerActorID, DWORD dwPrevAttackerActorID, int iActionIndex, int iProjectileSignalArrayIndex );
	void OnReceiveAddPacket( CPacketCompressStream& PacketStream );
#endif

#endif

	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
