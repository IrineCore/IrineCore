#include "stdafx.h"
#include "DnCalcCatmullrom.h"

void CDnCalcCatmullRom::Init( EtVector3& v0, EtVector3& v1, EtVector3& v2, EtVector3& v3, LOCAL_TIME nRemainTime, LOCAL_TIME nCurTime )
{
	m_v0 = v0;
	m_v1 = v1;
	m_v2 = v2;
	m_v3 = v3;
	m_vPrevPos = m_vCurPos = m_v0;
	m_nRemainTime = nRemainTime;
	m_nLastTime = nCurTime;
	m_nLocalTime = nCurTime;
	m_fLastMoveValue = 0.0f;
}

void CDnCalcCatmullRom::GetInfo( OUT EtVector3& v0, OUT EtVector3& v1, OUT EtVector3& v2, OUT EtVector3& v3, OUT LOCAL_TIME& nRemainTime, OUT LOCAL_TIME& nCurTime )
{
	v0 = m_v0;
	v1 = m_v1;
	v2 = m_v2;
	v3 = m_v3;
	nRemainTime = m_nRemainTime;
	nCurTime = m_nLastTime;
}

float CDnCalcCatmullRom::CalcWeight(float fMin, float fMax, float fValue )
{
	if ( fMin >= fValue )
		return 0.0f;
	if ( fMax <= fValue )
		return 1.0f;

	float f = fMax - fMin;
	float ff = f / 100.0f;

	float n = (fValue - fMin) / ff;

	return n / 100.0f;
}

void CDnCalcCatmullRom::Process( LOCAL_TIME nTime )
{
	// TODO: In the game server which updates at 10 frames per second when moving to a nearby location
	// Result of GetWeight() exceeds 1.0f in one go, so EtVec3Catmullrom() not even once
	// Not called and may fall into the else {} branch, leaving the projectile in place.
	// In that case, it seems we have no choice but to divide the time delta by 2 and run it twice
	// May load the game server, so implement later if an issue arises.
	m_nLocalTime = nTime;

	float fWeight = GetWeight();

	m_vPrevDir = m_vCurDir;
	m_vPrevPos = m_vCurPos;

	// Weight coefficient
	float s = 0.0f;
	if ( fWeight <= 0.34f )
	{
		s = CalcWeight(0.0f, 0.34f, fWeight);
		EtVec3CatmullRom(&m_vCurPos, &m_v0, &m_v0, &m_v1, &m_v2, s);
	}
	else if ( fWeight > 0.34f && fWeight <= 0.67f )
	{
		s = CalcWeight(0.34f, 0.67f, fWeight);
		EtVec3CatmullRom(&m_vCurPos, &m_v0, &m_v1, &m_v2, &m_v3, s);
	}
	else if ( fWeight > 0.67f && fWeight <= 1.0f )
	{
		s = CalcWeight(0.67f, 1.0f, fWeight);
		EtVec3CatmullRom(&m_vCurPos, &m_v1, &m_v2, &m_v3, &m_v3, s);
	}
	else
	{
		//EtVector3 v = m_vCurPos - m_vPrevPos;
		m_vCurPos += m_vCurDir * m_fLastMoveValue ;
		//m_vCurPos += v;
	}

	EtVector3 v = m_vCurPos - m_vPrevPos;
	m_fLastMoveValue = EtVec3Length(&v);
	EtVec3Normalize(&v, &v);
	m_vCurDir = v;

}

float CDnCalcCatmullRom::GetWeight() 
{ 
	LOCAL_TIME nElapsed = m_nLocalTime - m_nLastTime;
	float fWeight = float(nElapsed)/float(m_nRemainTime);
	return fWeight;
}

CDnHoming::CDnHoming()
{
	m_LocalTime = 0;
	m_CreateTime = 0;
	m_LifeTime = 0;
}

CDnHoming::~CDnHoming()
{
}

void CDnHoming::Init( LOCAL_TIME RemainTime, LOCAL_TIME CurTime, EtVector3 &StartVector, EtVector3 &EndVector, EtVector3 &ViewVec )
{
	m_CreateTime = CurTime;
	m_LifeTime = RemainTime;
	m_StartVec = StartVector;
	m_EndVec = EndVector;
	m_ViewVec = ViewVec;
}

EtVector3 CDnHoming::GetPos()
{
	EtVector3 res;
	LOCAL_TIME dwTime = ( m_LocalTime - m_CreateTime );
	if( dwTime > m_LifeTime ) dwTime = m_LifeTime;

	LOCAL_TIME dwMaxTime = m_LifeTime;
	EtVector3 Accel = ( 2.f / (float)( dwMaxTime * dwMaxTime ) ) * ( m_EndVec - m_StartVec - ( (float)dwMaxTime * m_ViewVec ) );
	res = m_StartVec + ( (float)dwTime * m_ViewVec ) + ( ( 0.5f * ( dwTime * dwTime ) ) * Accel );
	return res;
}


EtVector3 CDnHoming::GetVVec()
{
	EtVector3 res;
	LOCAL_TIME dwTime = ( m_LocalTime - m_CreateTime );
	if( dwTime > m_LifeTime ) dwTime = m_LifeTime;
	LOCAL_TIME dwMaxTime = m_LifeTime;
	res = m_ViewVec + dwTime * ( 2.f / (float)( dwMaxTime * dwMaxTime ) ) * ( m_EndVec - m_StartVec - ( (float)dwMaxTime * m_ViewVec ) );
	return res;
}

float CDnHoming::GetWeight()
{
	LOCAL_TIME dwTime = ( m_LocalTime - m_CreateTime );
	return ( 1.f / m_LifeTime ) * dwTime;
}