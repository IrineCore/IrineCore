#pragma once

#include "DnActor.h"

// If nTeamSetting is not -1, set to nTeamSetting value; if -1, set automatically.
// Currently 0 is user and 1 is mob.
DnActorHandle CreateActor( CMultiRoom *pRoom, int nActorTableID, int nTeamSetting = -1 );
