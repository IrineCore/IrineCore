#pragma once

struct SAABox;

#define NEAR_ZERO	10e-5

bool TestLineToTriangle( EtVector3 &Origin, EtVector3 &Direction, EtVector3 &Point1, EtVector3 &Point2, EtVector3 &Point3, 
						float &fDist, float &fBary1, float &fBary2 );
bool TestLineToSphere( EtVector3 &Origin, EtVector3 &Direction, SSphere &Sphere );
bool TestLineToPlane( EtVector3 &Origin, EtVector3 &Direction, EtVector4 &Plane, EtVector3 &IntersectPoint );
bool TestEdgeToPlane( EtVector3 &Start, EtVector3 &End, EtVector4 &Plane, EtVector3 &IntersectPoint );
bool TestLineToBox( EtVector3 &Origin, EtVector3 &Direction, SAABox &Box, float &fDist );
bool TestSegmentToSphere( SSegment &Segment, SSphere &Sphere );
bool TestSegmentToOBB( SSegment &Segment, SOBB &Box );
bool TestSegmentToCapsule( SSegment &Segment, SCapsule &Capsule );
bool IsBehindPoint( EtVector4 &Plane, EtVector3 &Point );
bool IsNearPoint( EtVector3 &Point1, EtVector3 &Point2, float fError = 0.001f );
bool TestBoxToSphere( SAABox &Box, SSphere &Sphere );
bool TestSphereToSphere( SSphere &Sphere1, SSphere &Sphere2 );
bool TestSphereToCapsule( SSphere &Sphere, SCapsule &Capsule, float &fSegParam );
bool TestBoxToBox( SAABox &Box1, SAABox &Box2 );
bool TestCircleToCircle( SCircle &Circle1, SCircle &Circle2 );
bool TestCircleToBox2D( SCircle &Circle, SAABox2D &Box );
bool TestLineToOBB( EtVector3 &Origin, EtVector3 &Direction, SOBB &Box );
bool TestOBBToOBB( SOBB &Box1, SOBB &Box2 );
bool TestOBBToSphere( SOBB &Box, SSphere &Sphere );
//bool TestOBBToCapsule( SOBB &Box, SCapsule &Capsule, float &fSegParam );
bool TestCapsuleToCapsule( SCapsule &Capsule1, SCapsule &Capsule2, float &fSegParam1, float &fSegParam2 );


bool CalcFrustumPlane( EtVector4 &Out, float fX, float fY, float fZ, float fW );
inline bool CalcFrustumNearPlane( EtVector4 &Out, EtMatrix &ViewProjMat )
{
	return CalcFrustumPlane( Out, ViewProjMat._13, ViewProjMat._23, ViewProjMat._33, ViewProjMat._43 );
}
inline bool CalcFrustumFarPlane( EtVector4 &Out, EtMatrix &ViewProjMat )
{
	return CalcFrustumPlane( Out, ViewProjMat._14 - ViewProjMat._13, ViewProjMat._24 - ViewProjMat._23,
		ViewProjMat._34 - ViewProjMat._33, ViewProjMat._44 - ViewProjMat._43 );
}
inline bool CalcFrustumLeftPlane( EtVector4 &Out, EtMatrix &ViewProjMat )
{
	return CalcFrustumPlane( Out, ViewProjMat._14 + ViewProjMat._11, ViewProjMat._24 + ViewProjMat._21,
		ViewProjMat._34 + ViewProjMat._31, ViewProjMat._44 + ViewProjMat._41 );
}
inline bool CalcFrustumRightPlane( EtVector4 &Out, EtMatrix &ViewProjMat )
{
	return CalcFrustumPlane( Out, ViewProjMat._14 - ViewProjMat._11, ViewProjMat._24 - ViewProjMat._21,
		ViewProjMat._34 - ViewProjMat._31, ViewProjMat._44 - ViewProjMat._41 );
}
inline bool CalcFrustumTopPlane( EtVector4 &Out, EtMatrix &ViewProjMat )
{
	return CalcFrustumPlane( Out, ViewProjMat._14 - ViewProjMat._12, ViewProjMat._24 - ViewProjMat._22,
		ViewProjMat._34 - ViewProjMat._32, ViewProjMat._44 - ViewProjMat._42 );
}
inline bool CalcFrustumBottomPlane( EtVector4 &Out, EtMatrix &ViewProjMat )
{
	return CalcFrustumPlane( Out, ViewProjMat._14 + ViewProjMat._12, ViewProjMat._24 + ViewProjMat._22,
		ViewProjMat._34 + ViewProjMat._32, ViewProjMat._44 + ViewProjMat._42 );
}


inline float PlaneDot( EtVector4 &Plane, EtVector3 &Vector )
{
	return Plane.x * Vector.x + Plane.y * Vector.y + Plane.z * Vector.z - Plane.w;
}

inline float FastAbs( float fVal ) 
{
	DWORD dwResult = (*(DWORD*)&fVal) & 0x7fffffff;
	return *(float*)&dwResult;
}

inline float CalcBoxPushOut( EtVector4 &Plane, EtVector3 &Vector )
{
	return FastAbs( Plane.x * Vector.x ) + FastAbs( Plane.y * Vector.y ) + FastAbs( Plane.z * Vector.z );
}
