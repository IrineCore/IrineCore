#pragma once

#define EMITTER_VER_10				10
#define EMITTER_VER_11				11
#define EMITTER_VER					12
#define EMITTER_STRUCT_RESERVED_10	( 1024 - sizeof( SEmitter_10 ) )
#define EMITTER_STRUCT_RESERVED_11	( 1024 - sizeof( SEmitter_11 ) )
#define EMITTER_STRUCT_RESERVED		( 1024 - sizeof( SEmitter ) )
#define EMITTER_TABLE_COUNT			21
#define BILLBOARD_EFFECT_FRAME		60.0f

struct SEmitter_10
{
	int nGenerateCount;
	int nLifeDuration;
	int nBeginningTime;
	float fSpeedVelocityMin;
	float fSpeedVelocityMax;
	EtVector4 vInitiateLaunch;
	float fGravitySpeed;
	EtVector3 vGravityVector;
	float fZRotateStartValue;
	float fZRotateEndValue;
	float fOpposeTime;
	float fOpposeScaleValue;
	float fTextureRepeat;
	unsigned int nRandomSeed;
	BOOL bUseRandomSpin;
	int nBlendOP;
	int nSourceBlendMode;
	int nDestBlendMode;
	BOOL bDisregardTracePos;
	BOOL bUseBumpEffectTexture;
	char szTextureName[ _MAX_PATH ];
	BOOL bUseYBillBoard;
};

struct SEmitter_11
{
	int nGenerateCount;
	int nLifeDuration;
	int nBeginningTime;
	float fSpeedVelocityMin;
	float fSpeedVelocityMax;
	EtVector4 vInitiateLaunch;
	float fGravitySpeed;
	EtVector3 vGravityVector;
	float fZRotateStartValue;
	float fZRotateEndValue;
	float fOpposeTime;
	float fOpposeScaleValue;
	float fTextureRepeat;
	unsigned int nRandomSeed;
	BOOL bUseRandomSpin;
	int nBlendOP;
	int nSourceBlendMode;
	int nDestBlendMode;
	BOOL bDisregardTracePos;
	BOOL bUseBumpEffectTexture;
	char szTextureName[ _MAX_PATH ];
	BOOL bUseYBillBoard;

	bool bUseTimeRandomSeed;
	EtVector3 vStartPosInnerRadius;
	EtVector3 vStartPosOuterRadius;
	int nSizeAdjustRate;
	int nUseRotateByDir;
};

struct SEmitter
{
	int nGenerateCount;
	int nLifeDuration;
	int nBeginningTime;
	float fSpeedVelocityMin;
	float fSpeedVelocityMax;
	EtVector4 vInitiateLaunch;
	float fFallSpeed;
	EtVector3 vFallVector;
	float fZRotateStartValue;
	float fZRotateEndValue;
	float fOpposeTime;
	float fOpposeScaleValue;
	float fDummy;
	unsigned int nRandomSeed;
	BOOL bUseRandomSpin;
	int nBlendOP;
	int nSourceBlendMode;
	int nDestBlendMode;
	BOOL bDisregardTracePos;
	BOOL bUseBumpEffectTexture;
	char szTextureName[ _MAX_PATH ];
	BOOL bUseYBillBoard;

	bool bUseTimeRandomSeed;
	EtVector3 vStartPosInnerRadius;
	EtVector3 vStartPosOuterRadius;
	int nSizeAdjustRate;
	int nUseRotateByDir;

	int nOptionCountMid;
	int nOptionCountLow;
};

class CEtEffectDataContainer;
class CEtBillboardEffectEmitter
{
public:
	CEtBillboardEffectEmitter();
	~CEtBillboardEffectEmitter();

	enum eDataType
	{
		COLOR_DATA_TYPE,
		ALPHA_DATA_TYPE,
		SCALE_DATA_TYPE,
		DATA_TYPE_AMOUNT,
	};

public:
	void InitializeEmitter();
	void FinalizeEmitter();
	void Clear();

	void LoadEmitter( CStream *pStream, int nVersion );
	void SaveEmitter( CStream *pStream );
	void LoadTexture( const char *pFileName );
	void CreateEmitterBuffer();
	void SetEffectParam( EtMaterialHandle hMaterial );

	int GetGenerateCount() { return m_EmitterInfo.nGenerateCount; }
	int GetLifeDuration() { return m_EmitterInfo.nLifeDuration; }
	void SetEmitterInfo( SEmitter *pEmitter ) { m_EmitterInfo = *pEmitter; }
	void GetEmitterInfo( SEmitter *pEmitter ) { *pEmitter = m_EmitterInfo; }
	SEmitter *GetEmitterInfo() { return &m_EmitterInfo; }
	void SetMaxEffectLifeTime( int nTime ) { m_nMaxEffectLifeTime = nTime; }
	void EnableDraw( bool bDraw ) { m_bDraw = bDraw; }		
	bool IsDraw() { return m_bDraw; }

	CEtEffectDataContainer *GetTable( eDataType Type ) { return m_pDataContainer[ Type ]; }

	const char *GetTextureName() { return m_hTexture->GetFileName(); }
	EtTextureHandle GetTextureHandle() { return m_hTexture; }

protected:
	SEmitter m_EmitterInfo;
	std::vector<float> m_vecBeginningTime;
	std::vector<SCustomParam> m_vecCustomParam;

	int m_nMaxEffectLifeTime;
	bool m_bDraw;
	CEtMeshStream m_EmitterMeshStream;
	EtTextureHandle m_hTexture;

	CEtEffectDataContainer *m_pDataContainer[DATA_TYPE_AMOUNT];
	EtColor m_ColorData[EMITTER_TABLE_COUNT];
	float m_ScaleData[EMITTER_TABLE_COUNT];

	void LoadVersion_10( CStream *pStream );
	void LoadVersion_11( CStream *pStream );
	void CreateDataContainer();

	friend class CEtBillboardEffectDataInstance;
};