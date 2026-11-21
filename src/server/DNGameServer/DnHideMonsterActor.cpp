#include "stdafx.h"
#include "DnHideMonsterActor.h"

CDnHideMonsterActor::CDnHideMonsterActor( CMultiRoom *pRoom, int nClassID )
: CDnSimpleRushMonsterActor( pRoom, nClassID )
{
}

CDnHideMonsterActor::~CDnHideMonsterActor()
{
}

bool CDnHideMonsterActor::IsHittable( DnActorHandle hHitter, LOCAL_TIME LocalTime, HitStruct *pHitSignal, int iHitUniqueID )
{
	return false;
}