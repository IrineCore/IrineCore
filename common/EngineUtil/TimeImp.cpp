#include "stdafx.h"
#include "TimeImp.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

float TimeImp::GetTimeVelocity( float fVelocity, LOCAL_TIME LocalTime, LOCAL_TIME PrevTime )
{
	float fValue = ( LocalTime - PrevTime ) / TimeImp::m_fCriterionTime;
	return powf( fVelocity, fValue );
}

float TimeImp::GetTimeVelocity( float fVelocity, float fDelta )
{
	float fValue = ( fDelta * 1000.f ) / TimeImp::m_fCriterionTime;
	return powf( fVelocity, fValue );
}