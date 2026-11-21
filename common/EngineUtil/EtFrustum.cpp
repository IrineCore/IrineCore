#include "stdafx.h"
#include "EtFrustum.h"

// Refer to improved frustum culling from Gems5 1.6 (code by Frank Puig Placeres) 

CEtFrustum::CEtFrustum()
{
	m_EyePosition = EtVector3(0,0,0);
	m_ForwardVector = EtVector3(0,0,0);
	m_RightVector = EtVector3(0,0,0);
	m_UpVector = EtVector3(0,0,0);

	m_rFactor = 0.0f; 
	m_uFactor = 0.0f;

	m_NearZ = 0.0f;
	m_FarZ = 0.0f;
}

CEtFrustum::~CEtFrustum()
{
}

void CEtFrustum::SetPerspective( const float FOV /*rad*/ , const float ViewAspect, const float nearZ, const float farZ )
{
	m_rFactor = FOV * 0.5f;		
	m_uFactor = m_rFactor * ViewAspect;
	m_NearZ   = nearZ;
	m_FarZ    = farZ;
}

//--------------------------------------------------------------------

void CEtFrustum::Build(  const EtVector3& Eye, const EtVector3& Forward, const EtVector3& Right, const EtVector3& Up )
{
	m_EyePosition   = Eye;
	m_ForwardVector = Forward;
	m_RightVector   = Right;
	m_UpVector      = Up;
}

//--------------------------------------------------------------------

bool CEtFrustum::IsPointIn( const EtVector3& Point )
{
	EtVector3 OP = Point - m_EyePosition;   

	float f = EtVec3Dot(&OP, &m_ForwardVector);         // OP dot ForwardVector 
	if (f < m_NearZ || m_FarZ < f) return false;

	float r = EtVec3Dot(&OP, &m_RightVector);              // OP dot RightVector
	float rLimit = m_rFactor * f;
	if (r < -rLimit || rLimit < r) return false;

	float u = EtVec3Dot(&OP, &m_UpVector);              // OP dot RightVector
	float uLimit = m_uFactor * f;
	if (u < -uLimit || uLimit < u) return false;

	// Up to here the point is known to be in the frustum
	return true;
}

//--------------------------------------------------------------------
// This method is highly unoptimized and is presented here just as
// an example because it is not covered in the article but the algorithm
// is so similiar to the followed in the six-planes approach that it 
// need no much explanation.
// please, refer to http://fpuig.cjb.net/ for an optimized method that 
// use p-n-points and extend transformation.
bool CEtFrustum::IsAABBIn  ( const EtVector3& v1, const EtVector3& v2)
{
	EtVector3 P;
	int nOutOfLeft=0, nOutOfRight=0, nOutOfFar=0, nOutOfNear=0, nOutOfTop=0, nOutOfBottom=0;
	bool bIsInRightTest, bIsInUpTest, bIsInFrontTest;

	EtVector3 Corners[2];
	Corners[0] = v1-m_EyePosition;
	Corners[1] = v2-m_EyePosition;

	for (int i=0; i<8; ++i) {
		bIsInRightTest = bIsInUpTest = bIsInFrontTest = false;
		P.x=Corners[i&1].x;
		P.y=Corners[(i>>2)&1].y;
		P.z=Corners[(i>>1)&1].z;

		float r=m_RightVector.x*P.x + m_RightVector.y*P.y + m_RightVector.z*P.z;
		float u=m_UpVector.x*P.x + m_UpVector.y*P.y + m_UpVector.z*P.z;
		float f=m_ForwardVector.x*P.x + m_ForwardVector.y*P.y + m_ForwardVector.z*P.z;

		if (r< -m_rFactor*f) ++nOutOfLeft;
		else if (r > m_rFactor*f) ++nOutOfRight;
		else bIsInRightTest=true;

		if (u < -m_uFactor*f) ++nOutOfBottom;
		else if (u > m_uFactor*f) ++nOutOfTop;
		else bIsInUpTest=true;

		if (f < m_NearZ) ++nOutOfNear;		
		else if (f > m_FarZ) ++nOutOfFar;
		else bIsInFrontTest=true;

		if (bIsInRightTest && bIsInFrontTest && bIsInUpTest) return true;
	}

	if (nOutOfLeft==8 || nOutOfRight==8 || nOutOfFar==8 || nOutOfNear==8 || nOutOfTop==8 || nOutOfBottom==8 ) return false;
	return true;
}

//--------------------------------------------------------------------
// This method is highly unoptimized and is presented here just as
// an example because it is not covered in the article but the algorithm
// is so similiar to the followed in the six-planes approach that it 
// need no much explanation.
// please, refer to http://fpuig.cjb.net/ for an optimized method that 
// use p-n-points and extend transformation.

bool CEtFrustum::IsOBBIn   ( const EtVector3& Center, const EtVector3& HalfDimensions, const EtQuat& RotationQuat)
{

	EtVector3 P;
	int nOutOfLeft=0, nOutOfRight=0, nOutOfFar=0, nOutOfNear=0, nOutOfTop=0, nOutOfBottom=0;
	bool bIsInRightTest, bIsInUpTest, bIsInFrontTest;

	EtVector3 Corners[8];
	Corners[0] = EtVector3( Center.x+HalfDimensions.x,  Center.y+HalfDimensions.y,  Center.z+HalfDimensions.z);
	Corners[1] = EtVector3( Center.x+HalfDimensions.x,  Center.y+HalfDimensions.y,  Center.z-HalfDimensions.z);
	Corners[2] = EtVector3( Center.x+HalfDimensions.x,  Center.y-HalfDimensions.y,  Center.z+HalfDimensions.z);
	Corners[3] = EtVector3( Center.x+HalfDimensions.x,  Center.y-HalfDimensions.y,  Center.z-HalfDimensions.z);
	Corners[4] = EtVector3( Center.x-HalfDimensions.x,  Center.y+HalfDimensions.y,  Center.z+HalfDimensions.z);
	Corners[5] = EtVector3( Center.x-HalfDimensions.x,  Center.y+HalfDimensions.y,  Center.z-HalfDimensions.z);
	Corners[6] = EtVector3( Center.x-HalfDimensions.x,  Center.y-HalfDimensions.y,  Center.z+HalfDimensions.z);
	Corners[7] = EtVector3( Center.x-HalfDimensions.x,  Center.y-HalfDimensions.y,  Center.z-HalfDimensions.z);

	int i;
	for (i=0; i<8; ++i) Corners[i] -= m_EyePosition;

	for (i=0; i<8; ++i) {
		bIsInRightTest = bIsInUpTest = bIsInFrontTest = false;
		P=Corners[i];

		float r=m_RightVector.x*P.x + m_RightVector.y*P.y + m_RightVector.z*P.z;
		float u=m_UpVector.x*P.x + m_UpVector.y*P.y + m_UpVector.z*P.z;
		float f=m_ForwardVector.x*P.x + m_ForwardVector.y*P.y + m_ForwardVector.z*P.z;

		if (r< -m_rFactor*f) ++nOutOfLeft;
		else if (r > m_rFactor*f) ++nOutOfRight;
		else bIsInRightTest=true;

		if (u < -m_uFactor*f) ++nOutOfBottom;
		else if (u > m_uFactor*f) ++nOutOfTop;
		else bIsInUpTest=true;

		if (f < m_NearZ) ++nOutOfNear;		
		else if (f > m_FarZ) ++nOutOfFar;
		else bIsInFrontTest=true;

		if (bIsInRightTest && bIsInFrontTest && bIsInUpTest) return true;
	}

	if (nOutOfLeft==8 || nOutOfRight==8 || nOutOfFar==8 || nOutOfNear==8 || nOutOfTop==8 || nOutOfBottom==8 ) return false;
	return true;
}

//--------------------------------------------------------------------

bool CEtFrustum::IsHullIn( const EtVector3* aVertices, const char nVertices )
{
	int BehindLeft=0, BehindRight=0, BehindFar=0, BehindNear=0, BehindTop=0, BehindBottom=0;
	bool inForward, inRight, inUp;
	for (char i(0); i<nVertices; ++i)
	{
		inForward=inRight=inUp=false;

		EtVector3 OP = aVertices[i] - m_EyePosition;   

		float f = EtVec3Dot(&OP, &m_ForwardVector);
		float r = EtVec3Dot(&OP, &m_RightVector);              // OP dot RightVector
		float u = EtVec3Dot(&OP, &m_UpVector);              // OP dot RightVector

		float rLimit = m_rFactor * f;
		float uLimit = m_uFactor * f;

		if (f < m_NearZ ) ++BehindNear;
		else if (f > m_FarZ) ++BehindFar;
		else inForward=true;

		if (r < -rLimit) BehindLeft++;
		else if (r > rLimit) BehindRight++;
		else inRight=true;

		if (u < -uLimit) ++BehindBottom;		
		else if (u > uLimit) ++BehindTop;
		else inUp=true;

		if (inForward && inRight && inUp) return true;
	}

	if (BehindLeft==8 || BehindRight==8 || BehindFar==8 || BehindNear==8 || BehindTop==8 || BehindBottom==8) return false;
	return true;
}

//--------------------------------------------------------------------

bool  CEtFrustum::IsSphereIn( const EtVector3& Center, const float Radius )
{
	EtVector3 OP = Center - m_EyePosition;   
	float f = EtVec3Dot(&OP, &m_ForwardVector);         // OP dot ForwardVector 
	if (f < m_NearZ-Radius || m_FarZ+Radius < f) return false;

	// Unoptimized but more understandable
	float r = EtVec3Dot(&OP, &m_RightVector); 
	float rLimit = m_rFactor * f;
	float rTop = rLimit + Radius;
	if (r < -rTop || rTop< r) return false;

	// Optimized ( a substraction is removed )
	float u = EtVec3Dot(&OP, &m_UpVector);
	float uLimit = m_uFactor * f;
	float uTop = uLimit + Radius;
	if (u < -uTop || uTop < u) return false;

	return true;
}
