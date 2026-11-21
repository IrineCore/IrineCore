#include "stdafx.h"
#include "EtTestCollision.h"
#include "EtComputeDist.h"
#include "EtCollisionFunc.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

bool TestSphereToSphere( SCollisionSphere &Sphere1, SCollisionSphere &Sphere2 )
{
	float fLengthSq;

	fLengthSq = EtVec3LengthSq( &( Sphere1.vCenter - Sphere2.vCenter ) );
	if( fLengthSq > ( Sphere1.fRadius + Sphere2.fRadius ) * ( Sphere1.fRadius + Sphere2.fRadius ) )
	{
		return false;
	}
	return true;
}

bool TestBoxToSphere( SCollisionBox &Box, SCollisionSphere &Sphere )
{
	EtVector3 Diff = Sphere.vCenter - Box.vCenter;

	float fAx = fabs( EtVec3Dot( &Diff, Box.vAxis ) );
	float fAy = fabs( EtVec3Dot( &Diff, Box.vAxis + 1 ) );
	float fAz = fabs( EtVec3Dot( &Diff, Box.vAxis + 2 ) );
	float fDx = fAx - Box.fExtent[ 0 ];
	float fDy = fAy - Box.fExtent[ 1 ];
	float fDz = fAz - Box.fExtent[ 2 ];
	float fSqrtRadius;

	if( fAx <= Box.fExtent[ 0 ] )
	{
		if( fAy <= Box.fExtent[ 1 ] )
		{
			if( fAz <= Box.fExtent[ 2 ] )
			{
				return true;
			}
			else
			{
				return fDz <= Sphere.fRadius;
			}
		}
		else
		{
			if( fAz <= Box.fExtent[ 2 ] )
			{
				return fDy <= Sphere.fRadius;
			}
			else
			{
				fSqrtRadius = Sphere.fRadius * Sphere.fRadius;
				return fDy * fDy + fDz * fDz <= fSqrtRadius;
			}
		}
	}
	else
	{
		if( fAy <= Box.fExtent[ 1 ] )
		{
			if( fAz <= Box.fExtent[ 2 ] )
			{
				return fDx <= Sphere.fRadius;
			}
			else
			{
				fSqrtRadius = Sphere.fRadius * Sphere.fRadius;
				return fDx*fDx + fDz*fDz <= fSqrtRadius;
			}
		}
		else
		{
			if( fAz <= Box.fExtent[ 2 ] )
			{
				fSqrtRadius = Sphere.fRadius * Sphere.fRadius;
				return fDx*fDx + fDy*fDy <= fSqrtRadius;
			}
			else
			{
				fSqrtRadius = Sphere.fRadius * Sphere.fRadius;
				return fDx*fDx + fDy*fDy + fDz*fDz <= fSqrtRadius;
			}
		}
	}
}

bool TestSphereToCapsule( SCollisionSphere &Sphere, SCollisionCapsule &Capsule, float &fSegParam )
{
	float fLengthSq;

	fLengthSq = DistPointToSegment( Sphere.vCenter, Capsule.Segment, fSegParam );

	return fLengthSq <= ( Capsule.fRadius + Sphere.fRadius ) * ( Capsule.fRadius + Sphere.fRadius );
}

bool TestBoxToBox( SCollisionBox &Box1, SCollisionBox &Box2 )
{
	EtVector3 Min, Max;
	float fExtentDiagonal[ 3 ];
	EtVector3 Vertices[ 8 ], *pBox1Axis, *pBox2Axis;
	float fEdgeLength[ 3 ];
	int nIndex;

	fEdgeLength[ 0 ] = Box1.fExtent[ 0 ] * 2.0f;
	fEdgeLength[ 1 ] = Box1.fExtent[ 1 ] * 2.0f;
	fEdgeLength[ 2 ] = Box1.fExtent[ 2 ] * 2.0f;
	pBox1Axis = Box1.vAxis;
	pBox2Axis = Box2.vAxis;
	Vertices[ 0 ] = Box1.vCenter - Box1.fExtent[ 0 ] * pBox1Axis[ 0 ] - Box1.fExtent[ 1 ] * pBox1Axis[ 1 ] - Box1.fExtent[ 2 ] * pBox1Axis[ 2 ];
	Vertices[ 1 ] = Vertices[ 0 ] + fEdgeLength[ 0 ] * pBox1Axis[ 0 ];
	Vertices[ 2 ] = Vertices[ 0 ] + fEdgeLength[ 1 ] * pBox1Axis[ 1 ];
	Vertices[ 3 ] = Vertices[ 0 ] + fEdgeLength[ 0 ] * pBox1Axis[ 0 ] + fEdgeLength[ 1 ] * pBox1Axis[ 1 ];
	Vertices[ 4 ] = Vertices[ 0 ] + fEdgeLength[ 2 ] * pBox1Axis[ 2 ];
	Vertices[ 5 ] = Vertices[ 0 ] + fEdgeLength[ 0 ] * pBox1Axis[ 0 ] + fEdgeLength[ 2 ] * pBox1Axis[ 2 ];
	Vertices[ 6 ] = Vertices[ 0 ] + fEdgeLength[ 1 ] * pBox1Axis[ 1 ] + fEdgeLength[ 2 ] * pBox1Axis[ 2 ];
	Vertices[ 7 ] = Vertices[ 0 ] + fEdgeLength[ 0 ] * pBox1Axis[ 0 ] + fEdgeLength[ 1 ] * pBox1Axis[ 1 ] + fEdgeLength[ 2 ] * pBox1Axis[ 2 ];

	nIndex = 0;
	if( EtVec3Dot( pBox2Axis, pBox1Axis ) > 0.f )
	{
		nIndex ^= 1;
	}
	if( EtVec3Dot( pBox2Axis, pBox1Axis + 1 ) > 0.f )
	{
		nIndex ^= 2;
	}
	if( EtVec3Dot( pBox2Axis, pBox1Axis + 2 ) > 0.f )
	{
		nIndex ^= 4;
	}
	fExtentDiagonal[ 0 ] = fabsf( EtVec3Dot( &( ( Vertices[ nIndex ] - Vertices[ 7 - nIndex ] ) * 0.5f ), pBox2Axis ) );
	nIndex = 0;
	if( EtVec3Dot( pBox2Axis + 1, pBox1Axis ) > 0.f )
	{
		nIndex ^= 1;
	}
	if( EtVec3Dot( pBox2Axis + 1, pBox1Axis + 1 ) > 0.f )
	{
		nIndex ^= 2;
	}
	if( EtVec3Dot( pBox2Axis + 1, pBox1Axis + 2 ) > 0.f )
	{
		nIndex ^= 4;
	}
	fExtentDiagonal[ 1 ] = fabsf( EtVec3Dot( &( ( Vertices[ nIndex ] - Vertices[ 7 - nIndex ] ) * 0.5f ), pBox2Axis +  1 ) );
	nIndex = 0;
	if( EtVec3Dot( pBox2Axis + 2, pBox1Axis ) > 0.f )
	{
		nIndex ^= 1;
	}
	if( EtVec3Dot( pBox2Axis + 2, pBox1Axis + 1 ) > 0.f )
	{
		nIndex ^= 2;
	}
	if( EtVec3Dot( pBox2Axis + 2, pBox1Axis + 2 ) > 0.f )
	{
		nIndex ^= 4;
	}
	fExtentDiagonal[ 2 ] = fabsf( EtVec3Dot( &( ( Vertices[ nIndex ] - Vertices[ 7 - nIndex ] ) * 0.5f ), pBox2Axis + 2 ) );

	EtVector3 CenterDist;

	CenterDist = Box1.vCenter - Box2.vCenter;
	if( Box2.fExtent[ 0 ] + fExtentDiagonal[ 0 ] < fabsf( EtVec3Dot( &CenterDist, pBox2Axis ) ) )
	{
		return false;
	}
	if( Box2.fExtent[ 1 ] + fExtentDiagonal[ 1 ] < fabsf( EtVec3Dot( &CenterDist, pBox2Axis + 1 ) ) )	
	{
		return false;
	}
	if( Box2.fExtent[ 2 ] + fExtentDiagonal[ 2 ] < fabsf( EtVec3Dot( &CenterDist, pBox2Axis + 2 ) ) )
	{
		return false;
	}

	return true;
}

bool TestBoxToCapsule( SCollisionBox &Box, SCollisionCapsule &Capsule, float &fSegParam, float &fBoxParam0, float &fBoxParam1, float &fBoxParam2 )
{
	float fLengthSq;

	fLengthSq = DistSegToBox( Capsule.Segment, Box, fSegParam, fBoxParam0, fBoxParam1, fBoxParam2 );

	return fLengthSq <= Capsule.fRadius * Capsule.fRadius;
}

bool TestCapsuleToCapsule( SCollisionCapsule &Capsule1, SCollisionCapsule &Capsule2, float &fSegParam1, float &fSegParam2 )
{
	float fLengthSq;

	fLengthSq = DistSegmentToSegment( Capsule1.Segment, Capsule2.Segment, fSegParam1, fSegParam2 );

	return fLengthSq <= ( Capsule1.fRadius + Capsule2.fRadius ) * ( Capsule1.fRadius + Capsule2.fRadius );
}

bool TestCapsuleToTriangle( SCollisionCapsule &Capsule, SCollisionTriangle &Triangle, float &fSegParam1, float &fTriParam1, float &fTriParam2 )
{
	float fLengthSq;

	fLengthSq = DistSegmentToTriangle( Capsule.Segment, Triangle, fSegParam1, fTriParam1, fTriParam2 );

	return fLengthSq <= Capsule.fRadius * Capsule.fRadius;
}

bool TestSphereToTriangle( SCollisionSphere &Sphere, SCollisionTriangle &Triangle, float &fTriParam1, float &fTriParam2 )
{
	float fLengthSq;

	fLengthSq = DistPointToTriangle( Sphere.vCenter, Triangle, fTriParam1, fTriParam2 );

	return fLengthSq <= Sphere.fRadius * Sphere.fRadius;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TestSegmentToBox( SSegment &Segment, SCollisionBox &Box )
{
	float fAWdU[ 3 ], fADdU[ 3 ], fAWxDdU[ 3 ], fRhs;
	EtVector3 HalfDir = 0.5f * Segment.vDirection;
	EtVector3 Center = Segment.vOrigin + HalfDir;

	EtVector3 Diff = Center - Box.vCenter;

	fAWdU[ 0 ] = ( float )fabs(( float )( EtVec3Dot( &HalfDir, &Box.vAxis[ 0 ] ) ) );
	fADdU[ 0 ] = ( float )fabs(( float )( EtVec3Dot( &Diff, &Box.vAxis[ 0 ] ) ) );
	fRhs = Box.fExtent[ 0 ] + fAWdU[ 0 ];
	if( fADdU[ 0 ] > fRhs )
	{
		return false;
	}

	fAWdU[ 1 ] = ( float )fabs(( float )( EtVec3Dot( &HalfDir, &Box.vAxis[ 1 ] ) ) );
	fADdU[ 1 ] = ( float )fabs(( float )( EtVec3Dot( &Diff, &Box.vAxis[ 1 ] ) ) );
	fRhs = Box.fExtent[ 1 ] + fAWdU[ 1 ];
	if( fADdU[ 1 ] > fRhs )
	{
		return false;
	}

	fAWdU[ 2 ] = ( float )fabs(( float )( EtVec3Dot( &HalfDir, &Box.vAxis[ 2 ] ) ) );
	fADdU[ 2 ] = ( float )fabs(( float )( EtVec3Dot( &Diff, &Box.vAxis[ 2 ] ) ) );
	fRhs = Box.fExtent[ 2 ] + fAWdU[ 2 ];
	if( fADdU[ 2 ] > fRhs )
	{
		return false;
	}

	EtVector3 WxD;
	EtVec3Cross( &WxD, &HalfDir, &Diff );

	fAWxDdU[ 0 ] = ( float )fabs(( float )( EtVec3Dot( &WxD, &Box.vAxis[ 0 ] ) ) );
	fRhs = Box.fExtent[ 1 ]*fAWdU[ 2 ] + Box.fExtent[ 2 ]*fAWdU[ 1 ];
	if( fAWxDdU[ 0 ] > fRhs )
	{
		return false;
	}

	fAWxDdU[ 1 ] = ( float )fabs(( float )( EtVec3Dot( &WxD, &Box.vAxis[ 1 ] ) ) );
	fRhs = Box.fExtent[ 0 ]*fAWdU[ 2 ] + Box.fExtent[ 2 ]*fAWdU[ 0 ];
	if( fAWxDdU[ 1 ] > fRhs )
	{
		return false;
	}

	fAWxDdU[ 2 ] = ( float )fabs(( float )( EtVec3Dot( &WxD, &Box.vAxis[ 2 ] ) ) );
	fRhs = Box.fExtent[ 0 ]*fAWdU[ 1 ] + Box.fExtent[ 1 ]*fAWdU[ 0 ];
	if( fAWxDdU[ 2 ] > fRhs )
	{
		return false;
	}

	return true;
}

bool TestSegmentToSphere( SSegment &Segment, SCollisionSphere &Sphere )
{
	float fDistSq, fSegParam;

	fDistSq = DistPointToSegment( Sphere.vCenter, Segment, fSegParam );

	return fDistSq <= Sphere.fRadius * Sphere.fRadius;
}

bool TestSegmentToCapsule( SSegment &Segment, SCollisionCapsule &Capsule )
{
	float fDistSq, fSegParam1, fSegParam2;

	fDistSq = DistSegmentToSegment( Segment, Capsule.Segment, fSegParam1, fSegParam2 );

	return fDistSq <= Capsule.fRadius * Capsule.fRadius;
}

bool TestSegmentToTriangle( SSegment &Segment, SCollisionTriangle &Triangle )
{
	EtVector3 vDirection;
	float fSegmentLength, fDist, fBary1, fBary2;
	fSegmentLength = EtVec3Length( &Segment.vDirection );
	vDirection = Segment.vDirection / fSegmentLength;
	if( TestLineToTriangle( Segment.vOrigin, vDirection, Triangle.vOrigin, Triangle.vOrigin + Triangle.vEdge1, Triangle.vOrigin + Triangle.vEdge2, 
		fDist, fBary1, fBary2 ) )
	{
		if( ( fDist >= 0.0f ) && ( fDist < fSegmentLength ) )
		{
			return true;
		}
	}

	return false;
}

bool TestSegmentToPrimitive( SSegment &Segment, SCollisionPrimitive &Primitive )
{
	switch( Primitive.Type )
	{
	case CT_BOX:
		if( TestSegmentToBox( Segment, *( ( SCollisionBox * )&Primitive ) ) )
		{
			return true;
		}
		break;
	case CT_SPHERE:
		if( TestSegmentToSphere( Segment, *( ( SCollisionSphere * )&Primitive ) ) )
		{
			return true;
		}
		break;
	case CT_CAPSULE:
		if( TestSegmentToCapsule( Segment, *( ( SCollisionCapsule * )&Primitive ) ) )
		{
			return true;
		}
		break;
	case CT_TRIANGLE:
		if( TestSegmentToTriangle( Segment, *( ( SCollisionTriangle * )&Primitive ) ) )
		{
			return true;
		}
		break;
	}

	return false;

}

static bool Clip( float fDenom, float fNumer, float &fT0, float &fT1 )
{
	// Return value is 'true' if line segment intersects the current test
	// plane.  Otherwise 'false' is returned in which case the line segment
	// is entirely clipped.
	if( fDenom > 0.0f )
	{
		if( fNumer > fDenom * fT1 )
		{
			return false;
		}
		if( fNumer > fDenom * fT0 )
		{
			fT0 = fNumer / fDenom;
		}
		return true;
	}
	else if( fDenom < 0.0f )
	{
		if( fNumer > fDenom * fT0 )
		{
			return false;
		}
		if( fNumer > fDenom * fT1 )
		{
			fT1 = fNumer/fDenom;
		}
		return true;
	}
	else
	{
		return fNumer <= 0.0f;
	}
}

bool FindIntersection( EtVector3 &vOrigin, EtVector3 &vDirection, float *pExtent, float &fT0, float &fT1 )
{
	float fSaveT0 = fT0, fSaveT1 = fT1;

	bool bNotEntirelyClipped =
		Clip( +vDirection.x, -vOrigin.x - pExtent[ 0 ], fT0, fT1) &&
		Clip( -vDirection.x, +vOrigin.x - pExtent[ 0 ], fT0, fT1) &&
		Clip( +vDirection.y, -vOrigin.y - pExtent[ 1 ], fT0, fT1) &&
		Clip( -vDirection.y, +vOrigin.y - pExtent[ 1 ], fT0, fT1) &&
		Clip( +vDirection.z, -vOrigin.z - pExtent[ 2 ], fT0, fT1) &&
		Clip( -vDirection.z, +vOrigin.z - pExtent[ 2 ], fT0, fT1);

	return bNotEntirelyClipped;
	//	return bNotEntirelyClipped && ( fT0 != fSaveT0 || fT1 != fSaveT1 );
}

bool FindSegmentToBox( SSegment &Segment, SCollisionBox &Box, float &fContactTime )
{
	// convert segment to box coordinates
	EtVector3 vDiff = Segment.vOrigin - Box.vCenter;
	EtVector3 vTransOrigin, vTransDirection;
	float fT0, fT1;
	bool bResult;

	vTransOrigin.x = EtVec3Dot( &vDiff, Box.vAxis );
	vTransOrigin.y = EtVec3Dot( &vDiff, Box.vAxis + 1 );
	vTransOrigin.z = EtVec3Dot( &vDiff, Box.vAxis + 2 );

	vTransDirection.x = EtVec3Dot( &Segment.vDirection, Box.vAxis );
	vTransDirection.y = EtVec3Dot( &Segment.vDirection, Box.vAxis + 1 );
	vTransDirection.z = EtVec3Dot( &Segment.vDirection, Box.vAxis + 2 );

	fT0 = 0.0f;
	fT1 = 1.0f;
	bResult = FindIntersection( vTransOrigin, vTransDirection, Box.fExtent, fT0, fT1 );

	if( bResult )
	{
		if ( fT0 > 0.0f )
		{
			fContactTime = fT0;
		}
		else  // fT0 == 0
		{
			if ( fT1 < 1.0f )
			{
				fContactTime = fT1;
			}
			else  // fT1 == 1
			{
				fContactTime = fT0;
				return true;
			}
		}
	}

	return bResult;
}

bool FindSegmentToSphere( SSegment &Segment, SCollisionSphere &Sphere, float &fContactTime )
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	float fA, fB, fC;
	float fT0, fT1;
	EtVector3 vDiff = Segment.vOrigin - Sphere.vCenter;

	fA = EtVec3LengthSq( &Segment.vDirection );
	fB = EtVec3Dot( &vDiff, &Segment.vDirection );
	fC = EtVec3LengthSq( &vDiff ) - Sphere.fRadius * Sphere.fRadius;

	// no intersection if Q(t) has no float roots
	float fDiscr = fB * fB - fA * fC;
	if( fDiscr > 0.0f )
	{
		float fRoot = sqrtf( fDiscr );
		float fInvA = 1.0f / fA;

		fT0 = ( -fB - fRoot) * fInvA;
		fT1 = ( -fB + fRoot) * fInvA;

		// assert: t0 < t1 since A > 0

		if( fT0 > 1.0f || fT1 < 0.0f )
		{
			return false;
		}
		else if( fT0 >= 0.0 )
		{
			fContactTime = min( fT0, fT1 );
			return true;
		}
		else if( fT1 >= 0.0 )
		{
			fContactTime = fT1;
			return true;
		}
	}
	else if( fDiscr == 0.0f )
	{
		fT0 = -fB / fA;
		if( 0.0f <= fT0 && fT0 <= 1.0f )
		{
			fContactTime = fT0;
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

void GenerateOrthonormalBasis( EtVector3& rkU, EtVector3& rkV, EtVector3& rkW, bool bUnitLengthW )
{
	if( !bUnitLengthW )
	{
		EtVec3Normalize( &rkW, &rkW );
	}

	float fInvLength;

	if( fabs( rkW.x ) >= fabs( rkW.y ) )
	{
		// W.x or W.z is the largest magnitude component, swap them
		fInvLength = 1.0f / sqrtf( rkW.x * rkW.x + rkW.z * rkW.z );
		rkU.x = -rkW.z * fInvLength;
		rkU.y = 0.0f;
		rkU.z = +rkW.x * fInvLength;
	}
	else
	{
		// W.y or W.z is the largest magnitude component, swap them
		fInvLength = 1.0f / sqrtf( rkW.y * rkW.y + rkW.z * rkW.z );
		rkU.x = 0.0f;
		rkU.y = +rkW.z * fInvLength;
		rkU.z = -rkW.y * fInvLength;
	}

	EtVec3Cross( &rkV, &rkW, &rkU);
}

#define FLT_EPSILON     1.192092896e-07F
static int Find( EtVector3& vOrigin, EtVector3 &vDirection, SCollisionCapsule& Capsule, float afT[ 2 ] )
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	EtVector3 kU, kV, kW = Capsule.Segment.vDirection;
	float fWLength = EtVec3Length( &kW );
	kW /= fWLength;
	GenerateOrthonormalBasis(kU,kV,kW,true);
	EtVector3 kD;

	kD.x = EtVec3Dot( &kU, &vDirection);
	kD.y = EtVec3Dot( &kV, &vDirection);
	kD.z = EtVec3Dot( &kW, &vDirection);
	float fDLength = EtVec3Length( &kD );
	kD /= fDLength;

	float fInvDLength = 1.0f / fDLength;
	EtVector3 kDiff = vOrigin - Capsule.Segment.vOrigin;
	EtVector3 kP;
	kP.x = EtVec3Dot( &kU, &kDiff );
	kP.y = EtVec3Dot( &kV, &kDiff );
	kP.z = EtVec3Dot( &kW, &kDiff );
	float fRadiusSqr = Capsule.fRadius * Capsule.fRadius;

	float fInv, fA, fB, fC, fDiscr, fRoot, fT, fTmp;

	// Is the velocity parallel to the capsule direction? (or zero)
	if( fabs( kD.z ) >= 1.0f - FLT_EPSILON || fDLength < FLT_EPSILON )
	{
		float fAxisDir = EtVec3Dot( &vDirection, &Capsule.Segment.vDirection );

		fDiscr = fRadiusSqr - kP.x * kP.x - kP.y * kP.y;
		if( fAxisDir < 0 && fDiscr >= 0.0f )
		{
			// Velocity anti-parallel to the capsule direction
			fRoot = sqrtf( fDiscr );
			afT[0] = ( kP.z + fRoot ) * fInvDLength;
			afT[1] = -( fWLength - kP.z + fRoot ) * fInvDLength;
			return 2;
		}
		else if( fAxisDir > 0  && fDiscr >= 0.0f )
		{
			// Velocity parallel to the capsule direction
			fRoot = sqrtf( fDiscr );
			afT[0] = -( kP.z + fRoot ) * fInvDLength;
			afT[1] = ( fWLength - kP.z + fRoot ) * fInvDLength;
			return 2;
		}
		else
		{
			// sphere heading wrong direction, or no velocity at all
			return 0;
		}   
	}

	// test intersection with infinite cylinder
	fA = kD.x * kD.x + kD.y * kD.y;
	fB = kP.x * kD.x + kP.y * kD.y;
	fC = kP.x * kP.x + kP.y * kP.y - fRadiusSqr;
	fDiscr = fB*fB - fA*fC;
	if( fDiscr < 0.0f )
	{
		// line does not intersect infinite cylinder
		return 0;
	}

	int iQuantity = 0;

	if( fDiscr > 0.0f )
	{
		// line intersects infinite cylinder in two places
		fRoot = sqrtf( fDiscr );
		fInv = 1.0f / fA;
		fT = ( -fB - fRoot ) * fInv;
		fTmp = kP.z + fT * kD.z;
		if( 0.0f <= fTmp && fTmp <= fWLength )
			afT[ iQuantity++ ] = fT * fInvDLength;

		fT = ( -fB + fRoot) * fInv;
		fTmp = kP.z + fT * kD.z;
		if( 0.0f <= fTmp && fTmp <= fWLength )
			afT[ iQuantity++ ] = fT * fInvDLength;

		if( iQuantity == 2 )
		{
			// line intersects capsule wall in two places
			return 2;
		}
	}
	else
	{
		// line is tangent to infinite cylinder
		fT = -fB / fA;
		fTmp = kP.z + fT * kD.z;
		if( 0.0f <= fTmp && fTmp <= fWLength )
		{
			afT[ 0 ] = fT * fInvDLength;
			return 1;
		}
	}

	// test intersection with bottom hemisphere
	// fA = 1
	fB += kP.z * kD.z;
	fC += kP.z * kP.z;
	fDiscr = fB * fB - fC;
	if( fDiscr > 0.0f )
	{
		fRoot = sqrtf(fDiscr);
		fT = -fB - fRoot;
		fTmp = kP.z + fT * kD.z;
		if( fTmp <= 0.0f )
		{
			afT[ iQuantity++ ] = fT * fInvDLength;
			if( iQuantity == 2 )
			{
				return 2;
			}
		}

		fT = -fB + fRoot;
		fTmp = kP.z + fT * kD.z;
		if( fTmp <= 0.0f )
		{
			afT[ iQuantity++ ] = fT * fInvDLength;
			if( iQuantity == 2 )
			{
				return 2;
			}
		}
	}
	else if( fDiscr == 0.0f )
	{
		fT = -fB;
		fTmp = kP.z + fT * kD.z;
		if( fTmp <= 0.0f )
		{
			afT[ iQuantity++ ] = fT * fInvDLength;
			if( iQuantity == 2 )
			{
				return 2;
			}
		}
	}

	// test intersection with top hemisphere
	// fA = 1
	fB -= kD.z * fWLength;
	fC += fWLength * ( fWLength - 2.0f * kP.z );

	fDiscr = fB * fB - fC;
	if( fDiscr > 0.0f )
	{
		fRoot = sqrtf( fDiscr );
		fT = -fB - fRoot;
		fTmp = kP.z + fT * kD.z;
		if( fTmp >= fWLength )
		{
			afT[ iQuantity++ ] = fT * fInvDLength;
			if( iQuantity == 2 )
			{
				return 2;
			}
		}

		fT = -fB + fRoot;
		fTmp = kP.z + fT * kD.z;
		if( fTmp >= fWLength )
		{
			afT[ iQuantity++ ] = fT * fInvDLength;
			if( iQuantity == 2 )
			{
				return 2;
			}
		}
	}
	else if( fDiscr == 0.0f )
	{
		fT = -fB;
		fTmp = kP.z + fT * kD.z;
		if( fTmp >= fWLength )
		{
			afT[ iQuantity++ ] = fT * fInvDLength;
			if( iQuantity == 2 )
			{
				return 2;
			}
		}
	}

	return iQuantity;
}

bool FindSegmentToCapsule( SSegment& Segment, SCollisionCapsule &Capsule, float &fContactTime )
{
	float fTime[ 2 ];
	int i, nQuantity, nClipQuantity;

	nQuantity = Find( Segment.vOrigin, Segment.vDirection, Capsule, fTime );
	nClipQuantity = 0;
	for ( i = 0; i < nQuantity; i++ )
	{
		if( 0.0f <= fTime[ i ] && fTime[ i ] <= 1.0f )
		{
			nClipQuantity++;
		}
	}
	if( fTime[ 0 ] >= 0.0f )
	{
		fContactTime = min( fTime[ 0 ], fTime[ 1 ] );
	}
	else if( fTime[ 1 ] >= 0.0f )
	{
		fContactTime = fTime[ 1 ];
	}

	return nClipQuantity > 0;
}

bool FindSegmentToTriangle( SSegment &Segment, SCollisionTriangle &Triangle, float &fContactTime )
{
	float fTriParam1, fTriParam2;
	if( DistSegmentToTriangle( Segment, Triangle, fContactTime, fTriParam1, fTriParam2 ) <= 0.0f )
	{
		return true;
	}
	return false;
}

bool TestPlaneToCapsule( EtVector4 &vPlane, SCollisionCapsule &Capsule )
{
	float fValue1 = EtVec3Dot( ( EtVector3 * )&vPlane, &Capsule.Segment.vOrigin ) + vPlane.w;
	float fValue2 = fValue1 + EtVec3Dot( ( EtVector3 * )&vPlane, &Capsule.Segment.vDirection );
	if( fValue1 * fValue2 <= 0.0f )
	{
		return true;
	}
	else
	{
		return fabs( fValue1 ) <= Capsule.fRadius || fabs( fValue2 ) <= Capsule.fRadius;
	}
}
