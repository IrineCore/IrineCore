#pragma once
#include "EtUIControl.h"
#include "DnCustomControlCommon.h"
#include "EtRTTRenderer.h"
#include "CameraData.h"

class CDnGaugeFace : public CEtUIControl, public CEtRTTRenderer, public CLostDeviceProcess
{
public:
	CDnGaugeFace( CEtUIDialog *pParent );
	virtual ~CDnGaugeFace(void);

protected:
	int m_nFaceID;
	GAUGE_FACE_TYPE m_FaceType;
	EtTextureHandle m_hFaceTexture;
	EtTextureHandle m_hMaskTexture;
	EtTextureHandle m_hBossMaskTexture;
	EtTextureHandle m_hTempTextureForDeviceLost;
	bool m_bUseTempTextureForDeviceLost;
	
	SUICoord m_FaceUV;
	SUICoord m_FaceCoord;
	DWORD m_dwFaceColor;
	float m_fFaceTime;
	bool m_bDamage;
	bool m_bDeleteObject;
	bool m_bBoss;

	SCameraFileHeader m_CamHeader;
	EtObjectHandle m_hFaceObject;
	int m_nAniIndex;
	float m_fFrame;

	CEtColorAdjustTexFilter *m_pColorAdjFilter;

	// Since GaugeFace is rendered simply unlike other previews, was going to change only ambient,
	// For Dark Lair, directional lights have too many colors, so configure everything like RenderAvatar.
	EtColor m_TempAmbient;
	int m_nDirLightCount;
	EtColor m_TempLightDiffuse[2];
	EtVector3 m_TempLightDir[2];

	// Some particle-only bosses have dds textures instead of skn or cam files.
	EtTextureHandle m_hBossPortraitTexture;

	bool IsLocalPlayerGaugeFace();
 
public:
	void SetFaceType( GAUGE_FACE_TYPE FaceType );
	void SetFaceID( int nID );
	int GetFaceID() { return m_nFaceID; }
	void CalcFaceCoord();
	void OnDamage();
	void UpdatePortrait( EtObjectHandle hHandle, bool bDelete, bool bBoss = false, int nAniIndex = 0, float fFrame = 0.0f );
	void BakePortrait();
	bool IsUpdatePortrait();	
	EtTextureHandle GetPortraitTexture() {return m_hFaceTexture; }
	void SetTexture( EtTextureHandle hTexture);
	void BackupPortrait();

	virtual void OnLostDevice();
	virtual void OnResetDevice();

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Render( float fElapsedTime );
	virtual void Process( float fElapsedTime );

	virtual void BeginRTT();
	virtual void EndRTT();
	virtual void RenderRTT( float fElapsedTime );
};
