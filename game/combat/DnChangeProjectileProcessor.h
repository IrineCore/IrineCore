#pragma once
#include "IDnSkillProcessor.h"
class CDnProjectile;


// Processor for skills using weapons like arrows
class CDnChangeProjectileProcessor : public IDnSkillProcessor, public TBoostMemoryPool< CDnChangeProjectileProcessor >
{
protected:
	int				m_iSourceWeaponID;
	int				m_iChangeWeaponID;
	CDnProjectile*	m_pProjectile;
	DnWeaponHandle  m_hProjectile;

public:
	CDnChangeProjectileProcessor( DnActorHandle hActor, int iSourceWeaponID, int iChangeWeaponID );
	virtual ~CDnChangeProjectileProcessor(void);

	int GetSourceWeaponID( void ) { return m_iSourceWeaponID; };
	int GetChangeWeaponID( void ) { return m_iChangeWeaponID; };

	CDnProjectile* GetProjectile( void ) { return m_pProjectile; };

	// from IDnSkillProcessor.h
	virtual void SetHasActor( DnActorHandle hActor );

	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	bool IsFinished( void );
	int GetNumArgument( void ) { return 2; }
};
