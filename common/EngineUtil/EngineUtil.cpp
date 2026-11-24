#include "stdafx.h"
#include "EngineUtil.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

EtVector2 EtVec3toVec2( const EtVector3 &vVec )
{
	return EtVector2( vVec.x, vVec.z );
}

EtVector3 EtVec2toVec3( const EtVector2 &vVec )
{
	return EtVector3( vVec.x, 0.f, vVec.y );
}


bool CheckRect( float x, float y, const EtVector2 &VecMin, const EtVector2 &VecMax ) 
{
	if( x >= VecMin.x && x <= VecMax.x && y >= VecMin.y && y <= VecMax.y ) return true;
	return false;
}

float RandomNumberInRange( float fMin, float fMax )
{
	float fRange, fRandom;

	fRandom = float( ::rand() ) / float( RAND_MAX );
	fRange = fMax - fMin;
	fRandom *= fRange;
	fRandom += fMin;

	return fRandom;
}
