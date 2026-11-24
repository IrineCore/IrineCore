#include "stdafx.h"
#include "MAStaticMovement.h"
#include "MAActorRenderBase.h"
#include "DnActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

MAStaticMovement::MAStaticMovement()
: m_vIdentity( 0.f, 0.f, 0.f )
{
}

MAStaticMovement::~MAStaticMovement()
{
}


void MAStaticMovement::Process( LOCAL_TIME LocalTime, float fDelta )
{
	EtVector3 *vDist = m_pActor->GetAniDistance();
	m_pMatExWorld->m_vPosition += m_pMatExWorld->m_vXAxis * vDist->x;
	m_pMatExWorld->m_vPosition += m_pMatExWorld->m_vYAxis * vDist->y;
	m_pMatExWorld->m_vPosition += m_pMatExWorld->m_vZAxis * vDist->z;
}

void MAStaticMovement::ForceLook( const EtVector2 &vVec )
{
	if( EtVec2LengthSq( &vVec ) > 0.f ) 
	{
		m_pMatExWorld->m_vZAxis = EtVector3( vVec.x, 0.f, vVec.y );
		EtVec3Normalize( &m_pMatExWorld->m_vZAxis, &m_pMatExWorld->m_vZAxis );
		m_pMatExWorld->MakeUpCartesianByZAxis();
	}
}