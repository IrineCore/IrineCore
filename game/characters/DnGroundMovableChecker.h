#pragma once
#include "IDnSkillUsableChecker.h"

class CDnGroundMovableChecker : public IDnSkillUsableChecker, public TBoostMemoryPool< CDnGroundMovableChecker >
{
protected:
#ifndef _GAMESERVER
	static bool s_bUseSignalSkillCheck;
	static bool s_PrimUseSignalSkillCheck;
	static LOCAL_TIME s_EndTime;			// Valid duration for signal skill check.
#endif

public:
	CDnGroundMovableChecker( DnActorHandle hActor );
	virtual ~CDnGroundMovableChecker(void);

#ifndef _GAMESERVER
	static void SetUseSignalSkillCheck( bool bUseSignalSkillCheck, LOCAL_TIME SignalEndTime );
	static bool GetUseSignalSkillCheck( void ) { return s_bUseSignalSkillCheck; };
	static LOCAL_TIME GetUseSkillSignalEnd( void ) { return s_EndTime; };

	static void SetPrimUseSignalSkillCheck( bool bCheck ) { s_PrimUseSignalSkillCheck = bCheck; };
#endif

	// from IDnSkillUsableChecker
	bool CanUse( void );
	int GetNumArgument( void ) { return 0; };

	virtual IDnSkillUsableChecker* Clone();
};
