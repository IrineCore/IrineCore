#include "stdafx.h"
#include "EtCollisionFunc.h"
#include "EtCollisionPrimitive.h"
#include "EtComputeDist.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

bool TestLineToTriangle( EtVector3 &Origin, EtVector3 &Direction, EtVector3 &Point1, EtVector3 &Point2, EtVector3 &Point3, 
						float &fDist, float &fBary1, float &fBary2 )
{
	float fDeterminant;
	EtVector3 Edge1, Edge2;
	EtVector3 PVector, TVector, QVector;

	Edge1 = Point2 - Point1;
	Edge2 = Point3 - Point1;
	EtVec3Cross( &PVector, &Direction, &Edge2 );
	fDeterminant = EtVec3Dot( &Edge1, &PVector );

	fDist = FLT_MAX;
	if( fDeterminant > 0 )
	{
		TVector = Origin - Point1;
	}
	else
	{
		TVector = Point1 - Origin;
		fDeterminant = -fDeterminant;
	}
	if( fDeterminant < NEAR_ZERO )
	{
		return false;
	}

	fBary1 = EtVec3Dot( &TVector, &PVector );
	if( ( fBary1 < 0.0f ) || ( fBary1 > fDeterminant ) )
	{
		return false;
	}

	EtVec3Cross( &QVector, &TVector, &Edge1 );
	fBary2 = EtVec3Dot( &Direction, &QVector );
	if( ( fBary2 < 0.0f ) || ( fBary2 + fBary1 > fDeterminant ) )
	{
		return false;
	}

	fDist = EtVec3Dot( &Edge2, &QVector );
	fDist /= fDeterminant;
	fBary1 /= fDeterminant;
	fBary2 /= fDeterminant;

	return true;
}

bool TestLineToBox( EtVector3 &Origin, EtVector3 &Direction, SAABox &Box, float &fDist )
{
	bool bInside;

	fDist = -FLT_MAX;
	bInside = true;
	if( Box.Min.x - Origin.x > NEAR_ZERO )
	{
		if( Direction.x <= 0.0f )
		{
			return false;
		}
		else
		{
			bInside = false;
			fDist = max( fDist, ( Box.Min.x - Origin.x ) / Direction.x );
		}
	}
	else if( Origin.x - Box.Max.x > NEAR_ZERO )
	{
		if( Direction.x >= 0.0f )
		{
			return false;
		}
		else
		{
			bInside = false;
			fDist = max( fDist, ( Box.Max.x - Origin.x ) / Direction.x );
		}
	}

	if( Box.Min.y - Origin.y > NEAR_ZERO )
	{
		if( Direction.y <= 0.0f )
		{
			return false;
		}
		else
		{
			bInside = false;
			fDist = max( fDist, ( Box.Min.y - Origin.y ) / Direction.y );
		}
	}
	else if( Origin.y - Box.Max.y > NEAR_ZERO )
	{
		if( Direction.y >= 0.0f )
		{
			return false;
		}
		else
		{
			bInside = false;
			fDist = max( fDist, ( Box.Max.y - Origin.y ) / Direction.y );
		}
	}

	if( Box.Min.z - Origin.z > NEAR_ZERO )
	{
		if( Direction.z <= 0.0f )
		{
			return false;
		}
		else
		{
			bInside = false;
			fDist = max( fDist, ( Box.Min.z - Origin.z ) / Direction.z );
		}
	}
	else if( Origin.z - Box.Max.z > NEAR_ZERO )
	{
		if(Direction.z >= 0.0f)
		{
			return false;
		}
		else
		{
			bInside = false;
			fDist = max( fDist, ( Box.Max.z - Origin.z ) / Direction.z );
		}
	}

	if( bInside )
	{
		fDist = 0.0f;
		return true;
	}

	EtVector3 Hit;

	Hit = Origin + Direction * fDist;
	const float BOX_SIDE_THRESHOLD = 0.1f;
	if(	Hit.x > Box.Min.x - BOX_SIDE_THRESHOLD && Hit.x < Box.Max.x + BOX_SIDE_THRESHOLD &&
		Hit.y > Box.Min.y - BOX_SIDE_THRESHOLD && Hit.y < Box.Max.y + BOX_SIDE_THRESHOLD &&
		Hit.z > Box.Min.z - BOX_SIDE_THRESHOLD && Hit.z < Box.Max.z + BOX_SIDE_THRESHOLD)
		return true;

	return false;
}

bool TestSegmentToSphere( SSegment &Segment, SSphere &Sphere )
{
	float fDistSq, fSegParam;

	fDistSq = DistPointToSegment( Sphere.Center, Segment, fSegParam );

	return fDistSq <= Sphere.fRadius * Sphere.fRadius;
}

bool TestSegmentToOBB( SSegment &Segment, SOBB &Box )
{
	float fAWdU[ 3 ], fADdU[ 3 ], fAWxDdU[ 3 ], fRhs;
	EtVector3 HalfDir = 0.5f * Segment.vDirection;
	EtVector3 Center = Segment.vOrigin + HalfDir;

	EtVector3 Diff = Center - Box.Center;

	fAWdU[ 0 ] = ( float )FastAbs(( float )( EtVec3Dot( &HalfDir, &Box.Axis[ 0 ] ) ) );
	fADdU[ 0 ] = ( float )FastAbs(( float )( EtVec3Dot( &Diff, &Box.Axis[ 0 ] ) ) );
	fRhs = Box.Extent[ 0 ] + fAWdU[ 0 ];
	if ( fADdU[ 0 ] > fRhs )
	{
		return false;
	}

	fAWdU[ 1 ] = ( float )FastAbs(( float )( EtVec3Dot( &HalfDir, &Box.Axis[ 1 ] ) ) );
	fADdU[ 1 ] = ( float )FastAbs(( float )( EtVec3Dot( &Diff, &Box.Axis[ 1 ] ) ) );
	fRhs = Box.Extent[ 1 ] + fAWdU[ 1 ];
	if ( fADdU[ 1 ] > fRhs )
	{
		return false;
	}

	fAWdU[ 2 ] = ( float )FastAbs(( float )( EtVec3Dot( &HalfDir, &Box.Axis[ 2 ] ) ) );
	fADdU[ 2 ] = ( float )FastAbs(( float )( EtVec3Dot( &Diff, &Box.Axis[ 2 ] ) ) );
	fRhs = Box.Extent[ 2 ] + fAWdU[ 2 ];
	if ( fADdU[ 2 ] > fRhs )
	{
		return false;
	}

	EtVector3 WxD;
	EtVec3Cross( &WxD, &HalfDir, &Diff );

	fAWxDdU[ 0 ] = ( float )FastAbs(( float )( EtVec3Dot( &WxD, &Box.Axis[ 0 ] ) ) );
	fRhs = Box.Extent[ 1 ]*fAWdU[ 2 ] + Box.Extent[ 2 ]*fAWdU[ 1 ];
	if ( fAWxDdU[ 0 ] > fRhs )
	{
		return false;
	}

	fAWxDdU[ 1 ] = ( float )FastAbs(( float )( EtVec3Dot( &WxD, &Box.Axis[ 1 ] ) ) );
	fRhs = Box.Extent[ 0 ]*fAWdU[ 2 ] + Box.Extent[ 2 ]*fAWdU[ 0 ];
	if ( fAWxDdU[ 1 ] > fRhs )
	{
		return false;
	}

	fAWxDdU[ 2 ] = ( float )FastAbs(( float )( EtVec3Dot( &WxD, &Box.Axis[ 2 ] ) ) );
	fRhs = Box.Extent[ 0 ]*fAWdU[ 1 ] + Box.Extent[ 1 ]*fAWdU[ 0 ];
	if ( fAWxDdU[ 2 ] > fRhs )
	{
		return false;
	}

	return true;
}

bool TestSegmentToCapsule( SSegment &Segment, SCapsule &Capsule )
{
	float fDistSq, fSegParam1, fSegParam2;

	fDistSq = DistSegmentToSegment( Segment, Capsule.Segment, fSegParam1, fSegParam2 );

	return fDistSq <= Capsule.fRadius * Capsule.fRadius;
}

bool TestLineToSphere( EtVector3 &Origin, EtVector3 &Direction, SSphere &Sphere )
{
	EtVector3 OriginToCenter;
	float fDot, fLength;

	OriginToCenter = Sphere.Center - Origin;
	fLength = EtVec3LengthSq( &OriginToCenter );
	EtVec3Normalize( &Direction, &Direction );
	fDot = EtVec3Dot( &OriginToCenter, &Direction );
	if( fDot < 0 )
	{
		return false;
	}
	if( Sphere.fRadius * Sphere.fRadius > fLength - fDot * fDot )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TestLineToPlane( EtVector3 &Origin, EtVector3 &Direction, EtVector4 &Plane, EtVector3 &IntersectPoint )
{
	float fDot;

	fDot = EtVec3Dot( ( EtVector3 * )&Plane, &Direction );
	if( FastAbs( fDot ) < NEAR_ZERO )
	{
		return false;
	}
	fDot = ( Plane.w - ( Plane.x * Origin.x + Plane.y * Origin.y + Plane.z * Origin.z ) ) / fDot;
	if( fDot < 0.0f )
	{
		return false;
	}
	IntersectPoint = Origin + Direction * fDot;

	return true;
}

bool TestEdgeToPlane( EtVector3 &Start, EtVector3 &End, EtVector4 &Plane, EtVector3 &IntersectPoint )
{
	EtVector3 Direction;
	float fDot;

	Direction = End - Start;
	fDot = EtVec3Dot( ( EtVector3 * )&Plane, &Direction );
	//	fDot = Plane.x * Direction.x + Plane.y * Direction.y + Plane.z * Direction.z;
	if( FastAbs( fDot ) < NEAR_ZERO )
	{
		return false;
	}
	fDot = ( Plane.w - ( Plane.x * Start.x + Plane.y * Start.y + Plane.z * Start.z ) ) / fDot;
	if( ( fDot < -NEAR_ZERO ) || ( fDot > 1.0f + NEAR_ZERO ) )
	{
		return false;
	}
	IntersectPoint = Start + Direction * fDot;

	return true;
}

bool IsBehindPoint( EtVector4 &Plane, EtVector3 &Point )
{	
	return Plane.x * Point.x + Plane.y * Point.y  + Plane.z * Point.z - Plane.w > NEAR_ZERO;
}

bool IsNearPoint( EtVector3 &Point1, EtVector3 &Point2, float fError )
{
	if( FastAbs( Point1.x - Point2.x ) > fError )
	{
		return false;
	}
	if( FastAbs( Point1.y - Point2.y ) > fError )
	{
		return false;
	}
	if( FastAbs( Point1.z - Point2.z ) > fError )
	{
		return false;
	}

	return true;
}

bool TestBoxToSphere( SAABox &Box, SSphere &Sphere )
{
	EtVector3 vBoxCenter, vBoxExtent, vDiff;
	float fAx, fAy, fAz, fDx, fDy, fDz, fSqrtRadius;

	vBoxExtent = ( Box.Max - Box.Min ) * 0.5f;
	vBoxCenter = ( Box.Max + Box.Min ) * 0.5f;
	vDiff = Sphere.Center - vBoxCenter;

	fAx = vDiff.x;
	fAy = vDiff.y;
	fAz = vDiff.z;
	fDx = fAx - vBoxExtent.x;
	fDy = fAy - vBoxExtent.y;
	fDz = fAz - vBoxExtent.z;

	if ( fAx <= vBoxExtent[ 0 ] )
	{
		if ( fAy <= vBoxExtent[ 1 ] )
		{
			if ( fAz <= vBoxExtent[ 2 ] )
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
			if ( fAz <= vBoxExtent[ 2 ] )
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
		if ( fAy <= vBoxExtent[ 1 ] )
		{
			if ( fAz <= vBoxExtent[ 2 ] )
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
			if ( fAz <= vBoxExtent[ 2 ] )
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

/*	EtVector3 Center, Extent;

	Center = ( Box.Max + Box.Min ) * 0.5f;
	Extent = ( Box.Max - Box.Min ) * 0.5f;

	if( fabs( Center.x - Sphere.Center.x ) > ( Extent.x + Sphere.fRadius ) )
	{
		return false;
	}
	if( fabs( Center.y - Sphere.Center.y ) > ( Extent.y + Sphere.fRadius ) )
	{
		return false;
	}
	if( fabs( Center.z - Sphere.Center.z ) > ( Extent.z + Sphere.fRadius ) )
	{
		return false;
	}

	float fRootRadius;

	fRootRadius = Sphere.fRadius * 0.707106f;
	if( fabs( Center.x - Sphere.Center.x ) < ( Extent.x + fRootRadius ) )
	{
		return true;
	}
	if( fabs( Center.y - Sphere.Center.y ) > ( Extent.y + fRootRadius ) )
	{
		return true;
	}
	if( fabs( Center.z - Sphere.Center.z ) > ( Extent.z + fRootRadius ) )
	{
		return true;
	}

	return false;*/
}

bool TestSphereToSphere( SSphere &Sphere1, SSphere &Sphere2 )
{
	float fLengthSq;

	fLengthSq = EtVec3LengthSq( &( Sphere1.Center - Sphere2.Center ) );
	if( fLengthSq > ( Sphere1.fRadius + Sphere2.fRadius ) * ( Sphere1.fRadius + Sphere2.fRadius ) )
	{
		return false;
	}
	return true;
}

bool TestSphereToCapsule( SSphere &Sphere, SCapsule &Capsule, float &fSegParam )
{
	float fLengthSq;

	fLengthSq = DistPointToSegment( Sphere.Center, Capsule.Segment, fSegParam );
	
	return fLengthSq <= ( Capsule.fRadius + Sphere.fRadius ) * ( Capsule.fRadius + Sphere.fRadius );
}

bool TestBoxToBox( SAABox &Box1, SAABox &Box2 )
{
	EtVector3 Center1, Center2;
	EtVector3 Extent1, Extent2;

	Center1 = ( Box1.Max + Box1.Min ) * 0.5f;
	Center2 = ( Box2.Max + Box2.Min ) * 0.5f;
	Extent1 = ( Box1.Max - Box1.Min ) * 0.5f;
	Extent2 = ( Box2.Max - Box2.Min ) * 0.5f;

	if( FastAbs( Center1.x - Center2.x ) > ( Extent1.x + Extent2.x ) )
	{
		return false;
	}
	if( FastAbs( Center1.y - Center2.y ) > ( Extent1.y + Extent2.y ) )
	{
		return false;
	}
	if( FastAbs( Center1.z - Center2.z ) > ( Extent1.z + Extent2.z ) )
	{
		return false;
	}

	return true;
}

bool TestCircleToCircle( SCircle &Circle1, SCircle &Circle2 )
{
	float fRadiusSum;

	fRadiusSum = Circle1.fRadius + Circle2.fRadius;
	if( EtVec2LengthSq( &( Circle1.Center - Circle2.Center ) ) < fRadiusSum * fRadiusSum )
	{
		return true;
	}
	return false;
}

bool TestCircleToBox2D( SCircle &Circle, SAABox2D &Box )
{
	int i;
	float fRadiusSq;
	DNVector(EtVector2) vecVertex;
	Box.GetVertices( vecVertex );
	fRadiusSq = Circle.fRadius * Circle.fRadius;
	for( i = 0; i < ( int )vecVertex.size(); i++ )
	{
		if( EtVec2LengthSq( &( vecVertex[ i ] - Circle.Center ) ) < fRadiusSq )
		{
			return true;
		}
	}
	if( Box.IsInside( Circle ) )
	{
		return true;
	}

	return false;
}

bool TestLineToOBB( EtVector3 &Origin, EtVector3 &Direction, SOBB &Box )
{
	float fWdU[ 3 ], fAWdU[ 3 ], fDdU[ 3 ], fADdU[ 3 ], fAWxDdU[ 3 ], fRhs;

	EtVector3 Diff = Origin - Box.Center;

	fWdU[ 0 ] =  EtVec3Dot( &Direction, &Box.Axis[ 0 ]);
	fAWdU[ 0 ] = ( float )FastAbs( ( float )(fWdU[ 0 ]) );
	fDdU[ 0 ] =  EtVec3Dot( &Diff, &Box.Axis[ 0 ] );
	fADdU[ 0 ] = ( float )FastAbs( ( float )(fDdU[ 0 ]) );
	if ( fADdU[ 0 ] > Box.Extent[ 0 ] && fDdU[ 0 ] * fWdU[ 0 ] >= 0.0f )
	{
		return false;
	}

	fWdU[ 1 ] =  EtVec3Dot( &Direction, &Box.Axis[ 1 ]);
	fAWdU[ 1 ] = ( float )FastAbs( ( float )(fWdU[ 1 ]) );
	fDdU[ 1 ] =  EtVec3Dot( &Diff, &Box.Axis[ 1 ]);
	fADdU[ 1 ] = ( float )FastAbs( ( float )(fDdU[ 1 ]) );
	if ( fADdU[ 1 ] > Box.Extent[ 1 ] && fDdU[ 1 ] * fWdU[ 1 ] >= 0.0f )
	{
		return false;
	}

	fWdU[ 2 ] =  EtVec3Dot( &Direction, &Box.Axis[ 2 ]);
	fAWdU[ 2 ] = ( float )FastAbs( ( float )(fWdU[ 2 ]) );
	fDdU[ 2 ] =  EtVec3Dot( &Diff, &Box.Axis[ 2 ]);
	fADdU[ 2 ] = ( float )FastAbs( ( float )(fDdU[ 2 ]) );
	if ( fADdU[ 2 ] > Box.Extent[ 2 ] && fDdU[ 2 ] * fWdU[ 2 ] >= 0.0f )
	{
		return false;
	}

	EtVector3 WxD;
	EtVec3Cross( &WxD, &Direction, &Diff );

	fAWxDdU[ 0 ] = ( float )FastAbs(( float )( EtVec3Dot( &WxD, &Box.Axis[ 0 ] ) ) );
	fRhs = Box.Extent[ 1 ] * fAWdU[ 2 ] + Box.Extent[ 2 ] * fAWdU[ 1 ];
	if ( fAWxDdU[ 0 ] > fRhs )
	{
		return false;
	}

	fAWxDdU[ 1 ] = ( float )FastAbs(( float )( EtVec3Dot( &WxD, &Box.Axis[ 1 ] ) ) );
	fRhs = Box.Extent[ 0 ] * fAWdU[ 2 ] + Box.Extent[ 2 ] * fAWdU[ 0 ];
	if ( fAWxDdU[ 1 ] > fRhs )
	{
		return false;
	}

	fAWxDdU[ 2 ] = ( float )FastAbs(( float )( EtVec3Dot( &WxD, &Box.Axis[ 2 ] ) ) );
	fRhs = Box.Extent[ 0 ] * fAWdU[ 1 ] + Box.Extent[ 1 ] * fAWdU[ 0 ];
	if ( fAWxDdU[ 2 ] > fRhs )
	{
		return false;
	}

	return true;
}

bool TestOBBToOBB( SOBB &Box1, SOBB &Box2 )
{
	EtVector3 Min, Max;
	float fExtentDiagonal[3];

	Box1.GetDiagonal( Min, Max, Box2.Axis[ 0 ] );
	fExtentDiagonal[ 0 ] = FastAbs( EtVec3Dot( &( ( Max - Min ) *0.5f ), Box2.Axis ) );

	Box1.GetDiagonal( Min, Max, Box2.Axis[ 1 ] );
	fExtentDiagonal[ 1 ] = FastAbs( EtVec3Dot( &( ( Max - Min ) * 0.5f ), Box2.Axis + 1 ) );

	Box1.GetDiagonal( Min, Max, Box2.Axis[ 2 ] );
	fExtentDiagonal[ 2 ] = FastAbs( EtVec3Dot( &( ( Max - Min ) * 0.5f ), Box2.Axis + 2 ) );

	EtVector3 CenterDist;

	CenterDist = Box1.Center - Box2.Center;
	if( Box2.Extent[ 0 ] + fExtentDiagonal[ 0 ] < FastAbs( EtVec3Dot( &CenterDist, Box2.Axis ) ) )
	{
		return false;
	}
	if( Box2.Extent[ 1 ] + fExtentDiagonal[ 1 ] < FastAbs( EtVec3Dot( &CenterDist, Box2.Axis + 1 ) ) )	
	{
		return false;
	}
	if( Box2.Extent[ 2 ] + fExtentDiagonal[ 2 ] < FastAbs( EtVec3Dot( &CenterDist, Box2.Axis + 2 ) ) )
	{
		return false;
	}

	return true;
}

bool TestOBBToSphere( SOBB &Box, SSphere &Sphere )
{
	EtVector3 Diff = Sphere.Center - Box.Center;

	float fAx = FastAbs( EtVec3Dot( &Diff, Box.Axis ) );
	float fAy = FastAbs( EtVec3Dot( &Diff, Box.Axis + 1 ) );
	float fAz = FastAbs( EtVec3Dot( &Diff, Box.Axis + 2 ) );
	float fDx = fAx - Box.Extent[ 0 ];
	float fDy = fAy - Box.Extent[ 1 ];
	float fDz = fAz - Box.Extent[ 2 ];
	float fSqrtRadius;

	if ( fAx <= Box.Extent[ 0 ] )
	{
		if ( fAy <= Box.Extent[ 1 ] )
		{
			if ( fAz <= Box.Extent[ 2 ] )
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
			if ( fAz <= Box.Extent[ 2 ] )
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
		if ( fAy <= Box.Extent[ 1 ] )
		{
			if ( fAz <= Box.Extent[ 2 ] )
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
			if ( fAz <= Box.Extent[ 2 ] )
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

/*bool TestOBBToCapsule( SOBB &Box, SCapsule &Capsule, float &fSegParam )
{
	float fLengthSq;
	float fBoxParam0, fBoxParam1, fBoxParam2;

	fLengthSq = DistSegToOBB( Capsule.Segment, Box, fSegParam, fBoxParam0, fBoxParam1, fBoxParam2 );

	return fLengthSq <= Capsule.fRadius * Capsule.fRadius;
}*/

bool TestCapsuleToCapsule( SCapsule &Capsule1, SCapsule &Capsule2, float &fSegParam1, float &fSegParam2 )
{
	float fLengthSq;

	fLengthSq = DistSegmentToSegment( Capsule1.Segment, Capsule2.Segment, fSegParam1, fSegParam2 );

	return fLengthSq <= ( Capsule1.fRadius + Capsule2.fRadius ) * ( Capsule1.fRadius + Capsule2.fRadius );
}

bool CalcFrustumPlane( EtVector4 &Out, float fX, float fY, float fZ, float fW )
{
	float fLength, fInvLength;

	fLength = sqrtf( fX * fX + fY * fY + fZ * fZ );
	if( fLength < NEAR_ZERO )
	{
		return false;
	}

	fInvLength = 1.0f / fLength;
	Out = EtVector4( -fX * fInvLength, -fY * fInvLength, -fZ * fInvLength, fW * fInvLength );

	return true;
}

