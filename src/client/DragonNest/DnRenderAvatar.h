#pragma once

#include "EtRTTRenderer.h"
class CDnRenderAvatarNew : public CEtRTTRenderer	// Maybe rename it later.. Just appended New, though it seems too late to change;
{
public:
	CDnRenderAvatarNew();
	virtual ~CDnRenderAvatarNew();

	void Initialize( int nWidth, int nHeight, SCameraInfo &CameraInfo, int nViewPortWidth, int nViewPortHeight, EtFormat Format = FMT_A8B8G8R8, bool bUseColorAdjFilter = true, bool bUseBorderAlpha = true );
	void CameraLookAt( EtVector3 &Eye, EtVector3 &At, EtVector3 &Up );

	bool SetActor( DnActorHandle hActor, bool bShowWeapon = true, bool bForceCashWeapon = false );
	bool SetActor( int nClassID, bool bShowWeapon = true );	// For nearby targets not created via actor handle, create them directly with default appearance.
	bool ResetActor( bool bInitCamera = true );
	int GetActorID();
	DnActorHandle GetActor() { return m_hActor; }

	// Enchant level. For weapons, pass as enhancement level argument for the enhance effect; ignored for armor.
	bool AttachItem( int nItemID, int nEnchantLevel = 0, int nSubArg1 = 0, int nLookItemID = ITEMCLSID_NONE );
	// Ended up creating a Detach function because it was strictly needed for things like clearing the cart in the Cash Shop preview.
	// If this had been considered from the start, it would probably have been better to hold all default parts too..
	// Anyway, if there is a chance to refactor next time, make all part attach/detach happen inside RenderAvatar.
	bool DetachItem( int nItemID );
	// Copy equipment info of the actor passed as argument. Currently not implemented for player actors, only for mounts.
	bool RefreshEquip( DnActorHandle hActor );

	SUICoord &GetUVCoord() { return m_RTTUVCoord; }
	void CalcUVCoord( float fUIWidth, float fUIHeight );
	void SetCameraYPos( float fYPos = 56.0f ) { m_fDefaultCameraYPos = fYPos; }

	virtual void Process( float fElapsedTime );
	virtual void BeginRTT();
	virtual void EndRTT();
	virtual void RenderRTT( float fElapsedTime );


	// Keep as-is for compatibility with existing code.
	void AddRotateYaw( float fAngle ) { m_nRotateYaw += (int)( fAngle * ROTATE_SCALE ); }
	bool IsFrontView() { return (m_nRotateYaw % (360*ROTATE_SCALE)) == 0; }
	void SetFrontView();
	void SetRearView();
	void ZoomIn();
	void ZoomOut();
	void Panning( float fX, float fY );

	// EtcObject
	void SetEtc( const char *szSkinName, const char *szAniName, const char *szActName );
	DnEtcHandle GetEtc() { return m_hEtc; }
	void ResetEtc( bool bInitCamera );

protected:

	enum {
		ROTATE_SCALE = 10,
	};

	// Deriving subclasses makes a has-a relationship messy in source, so distinguish by type internally instead.
	enum ActorEnum {
		None,
		PlayerActor,
		VehicleActor,
		PetActor,
		EtcObject,
		//MonsterActor, // Includes all single-body actors. Not yet supported.
	};

	// Unlike the character face shown on the gauge, Inspect or Cash Shop preview must create a new object to use for rendering.
	// Initially considered receiving MAPartsBody(or SingleBody) and action signal separately,
	// Ultimately, it's hard to forward action signals in each part separately, and weapon linking has to be done separately.
	// For monster actors, signals are on the Actor itself rather than on parts; due to this and several other issues, decided to create and use a full actor.
	DnActorHandle m_hActor;
	DnEtcHandle m_hEtc;
	ActorEnum m_eActorType;

	// Keep separate info on equipped parts for exception handling when equipping a one-piece outfit.
	typedef struct _SPartsInfo {
		int nType;
		int nItemID;
	} SPartsInfo;
	std::vector<SPartsInfo> m_vecPartsInfo;

	SUICoord m_RTTUVCoord;

	struct sTempTargetInfo
	{
		int nWidth;
		int nHeight;
		EtFormat Format;
		EtTextureHandle hTexture;
	};
	static std::vector<sTempTargetInfo> s_vecTempTarget;
	static int s_nTempTargetRefCount;

	static CEtColorAdjustTexFilter *s_pColorAdjFilter;
	// When no light object has been created at all, create it directly.
	static EtLightHandle s_hTempLight[2];
	// EnviTexture as well.
	static EtTextureHandle s_hTempEnviTexture;
	// Other environment values needed for color filtering
	static EtColor s_TempAmbient;
	static EtColor s_TempLightDiffuse[2];
	static EtVector3 s_TempLightDir[2];

	static bool s_bInitializeStaticObject;
	static void InitializeStaticObject();
	static void FinalizeStaticObject();

	bool m_bUseColorAdjFilter;
	bool m_bUseBorderAlpha;

	void PostProcess();	// Color filter post-processing. Apply filter to render target just like in-game.
	void InitCameraControl();
	void CheckType();
	void CheckActorType();
	void CheckEtcType();
	bool IsLoading();

	// Function to get a shared temporary render target
	EtTextureHandle GetTempRenderTarget( int nWidth, int nHeight, EtFormat Format );

	// For camera handling, use the camera owned by RTTRenderer for the rest,
	// Keep only zoom-in and other values separately.
	float m_fDefaultCameraYPos;
	float m_fDefaultCameraZPos;
	int m_nRotateYaw;
	float m_fSmoothRotateYaw;

	// Since zoom in/out is not perspective projection, we cannot move the camera or adjust FOV. We have to resize the viewport to simulate zooming in.
	// Old RenderAvatar changed character scale, but skipping that since it may cause issues with effects and other things.
	int m_nDefaultViewPortWidth;
	int m_nDefaultViewPortHeight;
	int m_nZoomLevel;
	float m_fSmoothZoomLevel;

	float m_fPanX;
	float m_fPanY;
	float m_fSmoothPanX;
	float m_fSmoothPanY;

	// Temporarily copy ambient and up to 2 directional lights before rendering. Temp variable to hold Orig values.
	EtColor m_TempAmbient;
	int m_nDirLightCount;
	EtColor m_TempLightDiffuse[2];
	EtVector3 m_TempLightDir[2];
	bool m_bUseTempLight;
};
