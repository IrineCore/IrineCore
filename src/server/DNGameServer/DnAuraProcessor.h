#pragma once
#include "IDnSkillProcessor.h"

class CDnPartyTask;

// Processor responsible for overall aura skill handling. Used only on GameServer.
class CDnAuraProcessor : public IDnSkillProcessor, public TBoostMemoryPool< CDnAuraProcessor >
{
private:
	float				m_fRange;
	float				m_fRangeSQ;
	LOCAL_TIME			m_LastAuraCheckTime;
	set<DWORD>			m_setPartyMemberInAura;

	// IDs of state effects included in the applied aura
	// Since the same state effect can be used in other skills, state effect must...
	// Must distinguish by the ID assigned at creation.
	map<DWORD, vector<int> > m_mapAuraMemberSEID;

	bool				m_bAuraOff;

public:
	CDnAuraProcessor( DnActorHandle hActor, float fRange );
	virtual ~CDnAuraProcessor(void);

	// from IDnSkillProcessor
	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	// Skill toggled on and off just like a toggle skill
	bool IsFinished( void ) { return m_bAuraOff; };
	
	int GetNumArgument( void ) { return 1; };
};
