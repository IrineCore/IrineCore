#include "StdAfx.h"
#include "EtTrapezoidShadowMap.h"
#include "EtConvexVolume.h"
#include "EtCamera.h"
#include "EtEngine.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtTrapezoidShadowMap::CEtTrapezoidShadowMap()
{
	m_fShadowRange = 1000.0f;
}

CEtTrapezoidShadowMap::~CEtTrapezoidShadowMap()
{
}

int CEtTrapezoidShadowMap::FindContactVertex( D3DXVECTOR3 *pPoints, D3DXVECTOR2 vDir, bool bMax)
{
	float fMax = bMax ? -FLT_MAX : FLT_MAX;
	int nIndex = -1;
	for( int i = 0; i < 8; i++) {
		float fD = pPoints[i].x * vDir.x + pPoints[i].y * vDir.y;
		if( bMax ? fD > fMax : fD < fMax ) {
			fMax = fD;
			nIndex = i;
		}
	}
	return nIndex;
}

void CEtTrapezoidShadowMap::CalcShadowMat()
{
	if( !CEtCamera::GetActiveCamera() ) {
		return;
	}

	int i;
	EtMatrix *pInvViewMat;
	SLightInfo *pLightInfo;
	EtVector3 Eye, At, CamDir;

	pLightInfo = CEtLight::GetShadowCastDirLightInfo();
	if( pLightInfo == NULL ) return;

	pInvViewMat = CEtCamera::GetActiveCamera()->GetInvViewMat();
	if(pInvViewMat->_41 != pInvViewMat->_41 ) {
		return;
	}
	CamDir = *( EtVector3 * )&pInvViewMat->_31;
	CamDir.y = 0.0f;
	EtVec3Normalize( &CamDir, &CamDir );
	At = *( EtVector3 * )&pInvViewMat->_41 + CamDir * m_fDistanceLightAt;

	EtVector3 vLightDir = pLightInfo->Direction;

	float fDot2 = fabsf(EtVec3Dot( &vLightDir, ( EtVector3 * )&pInvViewMat->_31));
	if( fDot2  > 0.9999f) {
		vLightDir.x += 0.01f;
		EtVec3Normalize(&vLightDir, &vLightDir);
	}

	Eye = At - vLightDir * 10000.0f;

	if( fabsf(EtVec3Dot( &(Eye-At), &EtVector3(0,1,0))) > 0.999f) {
		EtMatrixLookAtLH( &m_LightViewMat, &Eye, &At, &EtVector3( 1.0f, 0.0f, 0.0f ) );
	}
	else {
		EtMatrixLookAtLH( &m_LightViewMat, &Eye, &At, &EtVector3( 0.0f, 1.0f, 0.0f ) );
	}


	EtMatrixOrthoLH( &m_LightProjMat, m_fShadowRange * 2.0f, m_fShadowRange * 2.0f, 10.0f, 100000.0f );

	static float fCamNear = 150.f;
	float fCamFar = m_fShadowRange;

	//////////////////////////////////////////////////////////////////////////
	EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
	EtMatrix matInvProj, matInvView, matProj;
	EtMatrixPerspectiveFovLH( &matProj, hCamera->GetFOV(), hCamera->GetAspectRatio(), fCamNear, fCamFar );
	EtMatrixInverse(&matInvProj, 0, &matProj);	
	matInvView = *hCamera->GetInvViewMat();
	*(D3DXVECTOR3*)&matInvView._41 -=  (*(D3DXVECTOR3*)&matInvView._31) * fCamNear;
	
	D3DXVECTOR3 vNear[4] = { D3DXVECTOR3(-1,1,0), D3DXVECTOR3(-1, -1, 0), D3DXVECTOR3( 1, 1, 0), D3DXVECTOR3( 1, -1, 0) };
	D3DXVECTOR3 vFar[4] = { D3DXVECTOR3(-1,1,1), D3DXVECTOR3(-1, -1, 1), D3DXVECTOR3( 1, 1, 1), D3DXVECTOR3( 1, -1, 1) };

	for( i = 0; i < 4; i++) {
		D3DXVec3TransformCoord(&vNear[i], &vNear[i], &matInvProj);
		D3DXVec3TransformCoord(&vFar[i], &vFar[i], &matInvProj);
		D3DXVec3TransformCoord(&vNear[i], &vNear[i], &matInvView);
		D3DXVec3TransformCoord(&vFar[i], &vFar[i], &matInvView);
		D3DXVec3TransformCoord(&vNear[i], &vNear[i], &m_LightViewMat);
		D3DXVec3TransformCoord(&vFar[i], &vFar[i], &m_LightViewMat);	
	}

	D3DXVECTOR3 vPoint[8];
	for( i = 0; i < 4; i++) {
		D3DXVec3TransformCoord(&vNear[i], &vNear[i], &m_LightProjMat);
		D3DXVec3TransformCoord(&vFar[i], &vFar[i], &m_LightProjMat);
		vPoint[i] = vNear[i];
		vPoint[i+4] = vFar[i];
	}
	
	D3DXVECTOR3 vNearCenter = (vNear[0]+vNear[1]+vNear[2]+vNear[3])*0.25f;
	D3DXVECTOR3 vFarCenter = (vFar[0]+vFar[1]+vFar[2]+vFar[3])*0.25f;

	D3DXVECTOR3 vCenterDir = vNearCenter - vFarCenter;
	vCenterDir.z = 0.f;
	D3DXVec3Normalize(&vCenterDir, &vCenterDir);

	D3DXVECTOR3 vCenterCross( -vCenterDir.y, vCenterDir.x, 0 );

	// Create a trapezoid enclosing the Frustum.
	int nIndex;
	D3DXVECTOR3 vLine[4];
	D3DXVECTOR3 vNearContact, vFarContact;

	nIndex = FindContactVertex( vPoint, D3DXVECTOR2(vCenterDir.x, vCenterDir.y), true);	
	vLine[0].x = -vCenterCross.y;	
	vLine[0].y = vCenterCross.x;	
	vLine[0].z = -(vLine[0].x * vPoint[nIndex].x + vLine[0].y * vPoint[nIndex].y);
	vNearContact = vPoint[nIndex];
	int nNearContactIndex = nIndex-4;

	nIndex = FindContactVertex( vPoint, D3DXVECTOR2(vCenterDir.x, vCenterDir.y), false);	
	vLine[1].x = -vCenterCross.y;
	vLine[1].y = vCenterCross.x;
	vLine[1].z = -(vLine[1].x * vPoint[nIndex].x + vLine[1].y * vPoint[nIndex].y);
	vFarContact = vPoint[nIndex];

	float fDot = fabsf(EtVec3Dot( (EtVector3*)&hCamera->GetInvViewMat()->_31, (EtVector3*)&pLightInfo->Direction ));

	float add_slope = 0.0f;
	float z, d, l, n;
	z = -0.6f;
	D3DXVECTOR3 pl = 0.5f*(vNearCenter+ vFarCenter);
	d = fabsf(D3DXVec3Dot(&(vNearContact - pl), &vCenterDir));
	l = fabsf(D3DXVec3Dot(&(vNearContact - vFarContact), &vCenterDir));
	n = (l*d*(1+z))/(l-2*d-l*z);
	vNearContact = pl + (n+d)*vCenterDir;

	float max_slope = -FLT_MAX;
	float min_slope =  FLT_MAX;

	for( i = 0; i < 4; i++) {
		if( i == nNearContactIndex ) {
			continue;
		}
		D3DXVECTOR3 v = vNearContact - vFar[i];
		v.z = 0.f;
		D3DXVec3Normalize(&v, &v);
		float fAngle = EtAcos( D3DXVec3Dot(&vCenterDir, &v) );	
		assert(fAngle == fAngle);
		D3DXVECTOR3 vCross;
		D3DXVec3Cross(&vCross, &vCenterDir, &v);
		if( vCross.z < 0.f ) fAngle = -fAngle;
		max_slope = __max( max_slope, fAngle );
		min_slope = __min( min_slope, fAngle);
	}

	float fMaxSlope = D3DX_PI*0.3f;
	if( nNearContactIndex >= 0 || fDot > 0.6f  ) {
		fMaxSlope = D3DX_PI*0.1f;
	}

	max_slope = __min(fMaxSlope, max_slope) + add_slope;
	min_slope = __min(fMaxSlope, fabsf(min_slope)) + add_slope;

	D3DXVECTOR3 vRightDir, vLeftDir;
	D3DXMATRIX matRot;
	D3DXMatrixRotationZ(&matRot, max_slope );
	D3DXVec3TransformNormal(&vRightDir, &vCenterDir, &matRot);
	D3DXMatrixRotationZ(&matRot, -min_slope );
	D3DXVec3TransformNormal(&vLeftDir, &vCenterDir, &matRot);

	D3DXVECTOR2 vRightCross(-vRightDir.y, vRightDir.x);
	nIndex = FindContactVertex( vPoint, vRightCross, false);	

	vLine[2].x = -vRightDir.y;
	vLine[2].y = vRightDir.x;
	vLine[2].z = -(vLine[2].x * vPoint[nIndex].x + vLine[2].y * vPoint[nIndex].y);

	D3DXVECTOR2 vLeftCross(-vLeftDir.y, vLeftDir.x);
	nIndex = FindContactVertex( vPoint, vLeftCross, true);

	vLine[3].x = -vLeftDir.y;
	vLine[3].y = vLeftDir.x;
	vLine[3].z = -(vLine[3].x * vPoint[nIndex].x + vLine[3].y * vPoint[nIndex].y);

	D3DXVECTOR3 t[5];
	int i0[5] = {1,1,0,0, 2};
	int i1[5] = {3,2,2,3, 3};

	for( i = 0; i < 5; i++) {
		t[i].y = (vLine[i0[i]].x*vLine[i1[i]].z - vLine[i0[i]].z*vLine[i1[i]].x) / (vLine[i0[i]].y*vLine[i1[i]].x-vLine[i0[i]].x*vLine[i1[i]].y);
		t[i].x = (vLine[i0[i]].y*vLine[i1[i]].z - vLine[i0[i]].z*vLine[i1[i]].y) / (vLine[i0[i]].x*vLine[i1[i]].y-vLine[i0[i]].y*vLine[i1[i]].x);
		t[i].z = 0.f;
	}
	///////////////////////////////////////////////////////////////	
	EtMatrix matTSM;
	EtMatrixIdentity(&matTSM);

	D3DXMATRIX T_1;
	D3DXVECTOR2 vCenterOrig = (t[2] + t[3] ) * 0.5f;
	D3DXMatrixTranslation(&T_1, -vCenterOrig.x, -vCenterOrig.y, 0.f);
	D3DXMatrixMultiply(&matTSM, &matTSM, &T_1);
	//////////////////////////////////////////////////////////////////////////
	D3DXMATRIX R;
	D3DXVECTOR3 u,v;
	u = (t[2] - t[3]) / D3DXVec3Length(&(t[2] - t[3]));
	assert(u.x == u.x);
	D3DXMatrixIdentity(&R);
	R._11 = u.x;
	R._12 = u.y;
	R._21 = u.y;
	R._22 = -u.x;
	D3DXMatrixMultiply(&matTSM, &matTSM, &R);

	D3DXMATRIX T_2;
	D3DXVec3TransformCoord(&u, &t[4], &matTSM);
	D3DXMatrixTranslation(&T_2, -u.x, -u.y, 0.f);
	D3DXMatrixMultiply(&matTSM, &matTSM, &T_2);

	D3DXVec3TransformCoord(&u, &((t[2]+t[3])*0.5f), &matTSM);
	assert(u.y != 0);
	D3DXMATRIX H(1.f, 0.f, 0.f, 0.f,
		-u.x/u.y, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f );
	D3DXMatrixMultiply(&matTSM, &matTSM, &H);

	D3DXVec3TransformCoord(&u, &t[2], &matTSM);
	assert(u.x != 0 && u.y != 0);
	D3DXMATRIX S_1(1/u.x, 0.f, 0.f, 0.f,
		0.f, 1/u.y, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f );
	D3DXMatrixMultiply(&matTSM, &matTSM, &S_1);
	assert(matTSM._11 == matTSM._11);
	
	D3DXMATRIX N;
	D3DXMatrixIdentity(&N);
	N._24 = 1.f;
	N._44 = 0.f;
	N._42 = -1.0f;
	D3DXMatrixMultiply(&matTSM, &matTSM, &N);

	D3DXVec3TransformCoord(&u, &t[0], &matTSM);
	float t3val = u.y;
	D3DXVec3TransformCoord(&u, &t[2], &matTSM);
	t3val += u.y;
	D3DXMATRIX T3(	1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, -(t3val)*0.5f, 0.f, 1.f );
	D3DXMatrixMultiply(&matTSM, &matTSM, &T3);
	
	D3DXVec3TransformCoord(&u, &t[0], &matTSM);	
	assert(u.y != 0);
	D3DXMATRIX S2(	1.f, 0.f, 0.f, 0.f,
		0.f, -1.f/u.y, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f );
	D3DXMatrixMultiply(&matTSM, &matTSM, &S2);
	EtMatrixMultiply( &m_LightProjMat, &m_LightProjMat, &matTSM);

	// Unit Cube Clipping
	int nCount;
	CEtConvexVolume Frustum;
	EtMatrix ProjMat;
	EtMatrixPerspectiveFovLH( &ProjMat, hCamera->GetFOV(), hCamera->GetAspectRatio(), hCamera->GetCameraNear(), m_fShadowRange*2 );
	EtMatrixMultiply( &ProjMat, hCamera->GetViewMat(), &ProjMat );
	Frustum.Initialize( ProjMat );
	{
		ScopeLock<CSyncLock> Lock( CEtObject::s_SmartPtrLock );
		nCount = CEtObject::GetItemCount();
		m_points.clear();	
		for( i = 0; i < nCount; i++ )
		{
			EtObjectHandle hHandle;
			hHandle = CEtObject::GetItem( i );
			if( ( !hHandle ) || ( !hHandle->GetSkin() ) || ( !hHandle->GetSkin()->GetMeshHandle() ) )
			{
				continue;
			}
			CEtObject *pObject = hHandle.GetPointer();
#ifdef PRE_CRASH_CHECK_BACKUP
			static EtSkinHandle hBackupSkin;
			static char szBAckupSkinFileName[ 256 ];
			hBackupSkin = pObject->GetSkin();
			strcpy( szBAckupSkinFileName, hBackupSkin->GetFileName() );
#endif	// #ifdef PRE_CRASH_CHECK_BACKUP
			if( pObject && ( pObject->GetFrustumMask() & g_nCurFrustumMask ) && ( pObject->IsShadowCast() ) && pObject->GetSkin()->GetMeshHandle()->GetSubMeshCount() > 0 )
			{
				EtVector3 Origin, Extent;
				hHandle->GetExtent( Origin, Extent );
				if( !hHandle->IsEnableCull() || Frustum.TesToBox( Origin, Extent ) )
				{
					SOBB obb;
					hHandle->GetBoundingBox(obb);
					obb.CalcVertices();

					if( fabsf(obb.Center.x) > 1000000.0f || fabsf(obb.Center.y) > 1000000.0f || fabsf(obb.Center.z) > 1000000.0f ) {
						continue;
					}

					for( int j = 0; j < 8; j++) {
						m_points.push_back( obb.Vertices[j] );
					}
				}
			}
		}
	}

	if( m_points.empty() ) {
		D3DXMatrixTranslation(&m_LightProjMat, 0.0f, 0.0f, 0.5f );
		m_LightProjMat._11 = 0.f;
		m_LightProjMat._22 = 0.f;
		m_LightProjMat._33 = 0.f;
		return;
	}

	EtMatrix matTrans;
	EtMatrixMultiply(&matTrans, &m_LightViewMat, &m_LightProjMat);

	SAABox aabb;
	aabb.Reset();
	nCount = (int)m_points.size();
	for( i = 0; i < nCount; i++) {
		EtVector3 point;
		EtVec3TransformCoord(&point, &m_points[i], &matTrans);
		aabb.AddPoint( point );
	}

	EtVector3 vExtent = aabb.GetExtent();
	float fLength = EtVec3Length( &vExtent );
	vExtent /= fLength;

	const float fMinLength = 1.0f;

	if( fLength < fMinLength ) {
		float fAddLength = (fMinLength - fLength)*0.5f;
		aabb.Min -= vExtent * fAddLength;
		aabb.Max += vExtent * fAddLength;
	}

	if( aabb.Max.x - aabb.Min.x > 2.f ) {
		aabb.Max.x = 1.f;
		aabb.Min.x = -1.f;
	}
	if( aabb.Max.y - aabb.Min.y > 2.f ) {
		aabb.Max.y = 1.f;
		aabb.Min.y = -1.f;
	}

	EtVector3 vExt = aabb.GetExtent();
	aabb.Min.x -= vExt.x*0.1f;
	aabb.Max.x += vExt.x*0.1f;
	aabb.Min.y -= vExt.y*0.1f;
	aabb.Max.y += vExt.y*0.1f;
	aabb.Min.z -= vExt.z*0.1f;
	aabb.Max.z += vExt.z*0.1f;

	EtMatrix matFit;
	D3DXMatrixOrthoOffCenterLH( &matFit, aabb.Min.x, aabb.Max.x, aabb.Min.y, aabb.Max.y, aabb.Min.z, aabb.Max.z);
	EtMatrixMultiply( &m_LightProjMat, &m_LightProjMat, &matFit);	

	// for depth matrix
	aabb.Reset();
	nCount = (int)m_points.size();
	for( i = 0; i < nCount; i++) {
		EtVector3 point;
		EtVec3TransformCoord(&point, &m_points[i], &m_LightViewMat );
		aabb.AddPoint( point );
	}
	vExt = aabb.GetExtent();
	aabb.Min.z -= vExt.z*0.5f;

	D3DXMatrixOrthoOffCenterLH( &m_LightProjDepthMat, aabb.Min.x, aabb.Max.x, aabb.Min.y, aabb.Max.y, 
		aabb.Min.z, aabb.Min.z + hCamera->GetCameraFar() * 2.0f  );
}
