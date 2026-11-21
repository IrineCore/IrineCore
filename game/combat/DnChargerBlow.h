#pragma once
#include "DnBlow.h"

class CDnProjectile;

// Effect index: 62
// Debuff skill
// Apply a specific debuff to the target so the user's desired attack originates from the target's body.
// Specific effect
// During [Duration], target repeatedly executes the abnormal-state Act file specified in [AbnormalEffectIndex].
// The specified abnormal-state Act file contains Hit signals or projectile-related signals.
// The specified abnormal-state Act file refers to the abnormal-state effect index configured in SkillLevelTable.
// Executing an abnormal state Act file on the affected target does not affect the target's actual action.
// The reason for using a state-effect Act file is to control direction, speed, etc. of projectiles.
//  When executing a state-effect Act file, the target-setting subject is the skill caster, not the target afflicted with the effect.
// e.g. When a user casts it on a monster, it executes the specified status effect Act file on the monster's body,
// Attack target in the Act file distinguishes enemy/ally relative to the caster.
// For Hit signals or projectile effects executed in a state-effect Act file, enter another effect type besides the charger effect in the SkillTable where the charger effect is applied so that effect takes effect.
// In effect types, the subject of Self is the caster.
// e.g. If SkillA's SkillTable skill effects have [Charger] in effect 1 and [Shock] in effect 2
// Monsters hit by projectiles fired from the abnormal-state act attached to a monster under the Charger effect receive the [Shock] state.

// Basic implementation method is to extract only the action signals (hit or projectile) originally owned by the player
// Process via the actor's onsignal, but replace the position with that of the actor affected(?) by this state effect.
class CDnChargerBlow : public CDnBlow, public TBoostMemoryPool< CDnChargerBlow >
{
private:
//#ifdef _GAMESERVER
	float m_fFrame;
	string m_strChargerActionName;
	CEtActionBase::ActionElementStruct* m_pActionElement;

	// On game server, m_pActionElement sometimes becomes NULL after OnSignal runs, so pre-copy needed data.
	vector<CEtActionSignal *> m_vlpSignalList;
	int m_iAdditionalWeaponLength;
	boost::shared_ptr<CDnState> m_pShooterStateSnapshot;

#if defined(PRE_FIX_65287)
	float m_fShooterFinalDamageRate;
#endif // PRE_FIX_65287
//#endif

public:
	CDnChargerBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChargerBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	void OnProjectileSignal( CDnProjectile* pProjectile );

	// Moved the OnSignal call functionality from Process to here..
	virtual void SignalProcess( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
