
#include "stdafx.h"
#include "PvPRoundModeScoreSystem.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
void CPvPRoundModeScoreSystem::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	// PvPRoundMode score is calculated on the server and sent.
}
