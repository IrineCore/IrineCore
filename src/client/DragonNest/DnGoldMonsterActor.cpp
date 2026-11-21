#include "StdAfx.h"
#include "DnGoldMonsterActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGoldMonsterActor::CDnGoldMonsterActor( int nClassID, bool bProcess )
	: CDnNormalMonsterActor( nClassID, bProcess )
{
}

CDnGoldMonsterActor::~CDnGoldMonsterActor()
{
}
