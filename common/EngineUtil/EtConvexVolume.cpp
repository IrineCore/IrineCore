#include "stdafx.h"
#include "EtConvexVolume.h"
#include "EtCollisionFunc.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CEtConvexVolume::CEtConvexVolume(void)
{
}

CEtConvexVolume::~CEtConvexVolume(void)
{
}

void CEtConvexVolume::Initialize( EtMatrix &ViewProjMat )
{
	EtVector4 TempVec;

	m_Planes.clear();
	std::vector<EtVector4>().swap(m_Planes);
	if( CalcFrustumLeftPlane( TempVec, ViewProjMat ) )
	{
		m_Planes.push_back( TempVec );
	}
	if( CalcFrustumRightPlane( TempVec, ViewProjMat ) )
	{
		m_Planes.push_back( TempVec );
	}
	if( CalcFrustumTopPlane( TempVec, ViewProjMat ) )
	{
		m_Planes.push_back( TempVec );
	}
	if( CalcFrustumBottomPlane( TempVec, ViewProjMat ) )
	{
		m_Planes.push_back( TempVec );
	}
	if( CalcFrustumFarPlane( TempVec, ViewProjMat ) )
	{
		m_Planes.push_back( TempVec );
	}
	if( CalcFrustumNearPlane( TempVec, ViewProjMat ) )
	{
		m_Planes.push_back( TempVec );
	}
}

bool CEtConvexVolume::TesToBox( EtVector3 &Origin, EtVector3 &Extent )
{
	int i;

	for( i = 0; i < ( int )m_Planes.size(); i++ )
	{
		if( PlaneDot( m_Planes[ i ], Origin ) > CalcBoxPushOut( m_Planes[ i ], Extent ) )
		{
			return false;
		}
	}

	return true;
}
