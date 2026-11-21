#include "stdafx.h"
#include "DnWorldNpcProp.h"
#include "DNProtocol.h"


CDnWorldNpcProp::CDnWorldNpcProp( CMultiRoom *pRoom )
: CDnWorldActProp( pRoom )
{
}

CDnWorldNpcProp::~CDnWorldNpcProp()
{
}

bool CDnWorldNpcProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	bool bResult = false;

	if( GetData() ) {
		NpcStruct *pStruct = (NpcStruct *)GetData();
		int nNpcTableID = pStruct->nNpcID;
		bResult = InitializeNpcActorProp( nNpcTableID );
	}

	return bResult;
}

bool CDnWorldNpcProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	bool bResult = CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
	if( !bResult ) return false;

	if( m_hMonster ) *m_hMonster->GetMatEx() = *GetMatEx();
	return true;
}

void CDnWorldNpcProp::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_hMonster ) {
		m_hMonster->Process( LocalTime, fDelta );
	}
}
