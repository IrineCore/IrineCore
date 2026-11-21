#pragma once

#include "EtType.h"
#include "EtObject.h"

class CEtCamera;
typedef CSmartPtr< CEtCamera >	EtCameraHandle;

struct SRenderSkin
{
	std::string szSkinName;
	EtSkinHandle hRenderSkin;
	EtMatrix SkinRenderMat;
};

class CEtCamera : public CSmartPtrBase< CEtCamera >
{
public:
	CEtCamera( bool bProcess = true );
	virtual ~CEtCamera();

protected:
	SCameraInfo m_CameraInfo;
	EtViewPort m_Viewport;
	float m_fAspectRatio;

	EtMatrix m_ViewMat;
	EtMatrix m_PrevViewMat;
	EtMatrix m_InvViewMat;
	EtMatrix m_ProjMat;
	EtMatrix m_ProjMatForCull;

	EtTextureHandle m_hRenderTarget;
	EtDepthHandle m_hDepthTarget;
	std::vector< EtObjectHandle > m_vecRenderTargetObject;

	std::vector< SRenderSkin > m_vecRenderSkin;

	static EtCameraHandle s_hActiveCamera;

	static std::vector<CEtCamera*> m_pVecProcessList;

public:
	void Initialize( SCameraInfo *pCamera );
	void Reset();
	void LookAt( EtVector3 &Eye, EtVector3 &At, EtVector3 &Up );
	void Update( EtMatrix *pCameraMat ) 
	{ 
		EtMatrixInverse( &m_ViewMat, NULL, pCameraMat ); 
		m_InvViewMat = *pCameraMat;
	}
	EtVector3 *GetPosition() { return ( EtVector3 * )&m_InvViewMat._41; }
	EtVector3 *GetDirection() { return ( EtVector3 * )&m_InvViewMat._31; }
	EtVector3 *GetUpVector() { return ( EtVector3 * )&m_InvViewMat._21; }
	EtVector3 *GetRightVector() { return ( EtVector3 * )&m_InvViewMat._11; }
	EtMatrix *GetViewMat() { return &m_ViewMat; }
	EtMatrix *GetPrevViewMat() { return &m_PrevViewMat; }
	EtMatrix *GetInvViewMat() { return &m_InvViewMat; }
	EtMatrix *GetProjMat() { return &m_ProjMat; }
	EtMatrix *GetProjMatForCull() { return &m_ProjMatForCull; }
	void SetProjMat( EtMatrix &ProjMat ) { m_ProjMat = ProjMat; }
	EtMatrix *GetViewProjMat();
	EtMatrix *GetViewProjMatForCull();
	void CalcInvViewMat() { EtMatrixInverse( &m_InvViewMat, NULL, &m_ViewMat ); }
	EtViewPort *GetViewport() { return &m_Viewport; }
	CameraTarget GetCameraTargetType() { return m_CameraInfo.Target; }

	void EndCamera() { m_PrevViewMat = m_ViewMat;	}

	void SetCameraFar( float fFar ) { m_CameraInfo.fFar = fFar;}
	float GetCameraFar() { return m_CameraInfo.fFar; }
	float GetCameraNear() { return m_CameraInfo.fNear; }
	void SetAspectRatio( float fAspectRatio );
	float GetAspectRatio() { return m_fAspectRatio; }
	void SetFOV( float fFOV );
	float GetFOV() { return m_CameraInfo.fFOV; }
	void SetOrthogonalView( int nViewWidth, int nViewHeight, float fOffsetX = 0.0f, float fOffsetY = 0.0f );

	float GetFogNear() { return m_CameraInfo.fFogNear; }
	void SetFogFar( float fFar ) { m_CameraInfo.fFogFar = fFar;}
	float GetFogFar() { return m_CameraInfo.fFogFar; }
	EtColor *GetFogColor() { return &m_CameraInfo.FogColor; }
	SCameraInfo GetCameraInfo() { return m_CameraInfo; }

	float GetWaterFarRatio() { return m_CameraInfo.fWaterFarRatio; }
	void SetWaterFarRatio( float fValue ) { m_CameraInfo.fWaterFarRatio = fValue; }

	void CalcPositionAndDir( int nX, int nY, EtVector3 &Position, EtVector3 &Dir );

	int AddRenderObject( EtObjectHandle hObject );
	void DeleteRenderObject( EtObjectHandle hObject );
	void RenderObjectList();
	EtTextureHandle GetRenderTargetTexture() { return m_hRenderTarget; }

	EtSkinHandle AddRenderSkin( const char *pSkinName, EtMatrix *pWorldMat );
	void DeleteRenderSkin( const char *pSkinName );
	void RenderSkinList();
	void RenderTerrain();

	void Render();

	EtCameraHandle Activate();

	static EtCameraHandle SetActiveCamera( int nItemIndex );
	static EtCameraHandle GetActiveCamera() { return s_hActiveCamera; }

	static int GetProcessItemCount();
	static EtCameraHandle GetProcessItem( int nIndex );
};
