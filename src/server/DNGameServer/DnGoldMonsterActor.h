#pragma once
#include "DnNormalMonsterActor.h"

class CDnGoldMonsterActor : public CDnNormalMonsterActor
{
public:
	CDnGoldMonsterActor( CMultiRoom *pRoom, int nClassID );
	virtual ~CDnGoldMonsterActor();

	virtual void OnDie( DnActorHandle hHitter );
};

class IBoostPoolDnGoldMonsterActor : public CDnGoldMonsterActor, public TBoostMemoryPool< IBoostPoolDnGoldMonsterActor >
{
public:
	IBoostPoolDnGoldMonsterActor( CMultiRoom *pRoom, int nClassID ) : CDnGoldMonsterActor( pRoom, nClassID ) {}
	virtual ~IBoostPoolDnGoldMonsterActor() {}
};
