#pragma once

#include "DnActor.h"
class CDnSkill;


// Fetch and use data from the monster table.
// Does not actually do anything...
// No need to run Process since the prop handles signals.
class CDnPropActor : public CDnActor, public TBoostMemoryPool< CDnPropActor >
{
public:
#ifdef _GAMESERVER
	CDnPropActor( CMultiRoom* pRoom, int nClassID );
#else
	CDnPropActor( int nClassID, bool bProcess = true );
#endif
	virtual ~CDnPropActor(void);

	//vector<DnSkillHandle> m_vlhSkillList;

protected:
	bool Initialize( void );
	//void OnDispatchMessage( DWORD dwActorProtocol, BYTE *pPacket );

	//// Create a separate skill used by props; it consumes no MP and has no class restrictions
	//// Done once triggered by a specific signal.
	//// Set MASkillUser::m_hProcessSkill appropriately so state effects are added properly on hit (server side).
	//bool AddPropSkill( int iSkillID, int iSkillLevelID );
	//DnSkillHandle UsePropSkill( int iSkillID, LOCAL_TIME LocalTime, float fDelta );
};
