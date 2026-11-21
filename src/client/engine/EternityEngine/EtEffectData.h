#pragma once

#include "EtEffectDataContainer.h"

struct SEffectFileHeader
{
	char szHeaderString[ 256 ];
	int nVersion;
};

struct SEffectToolInfo
{
	char szFileName[ 32 ];
	char szDescription[ 32 ];
	COLORREF TextColor;
	COLORREF BackColor;
	char cInitials;
};

#define EFFECT_FILE_STRING	"Eternity Engine Effect File"
#define EFFECT_FILE_VERSION	14
#define EFFECT_HEADER_RESERVED	( 1024 - sizeof( SEffectFileHeader ) )
#define EFFECT_TOOL_INFO	528
#define EFFECT_CUSTOM_PARAMETER_COUNT	4

class CEtEffectObject;
class CEtEffectElement;
class CEtElementData
{
public:
	CEtElementData();
	virtual ~CEtElementData();

protected:
	EffectResourceType m_Type;
	char m_szFileName[ 64 ];
	DWORD m_dwStartFrame;
	DWORD m_dwEndFrame;
	DWORD m_dwYOrder;
	DWORD m_nPositionInterpolationType;
	CEtEffectDataContainer m_DataContainer[ ERT_AMOUNT ];

	EtMatrix m_WorldMat;
	EtColor m_Color;

	bool m_bBillboard;

public:
	virtual void Load( CStream *pStream, int nVersion );
	EffectResourceType GetType() { return m_Type; }
	EtMatrix *GetWorldMat() { return &m_WorldMat; }
	EtColor *GetColor() { return &m_Color; }	
	const char *GetFileName() { return m_szFileName; }

	virtual bool Process( DWORD dwTick, EtMatrix WorldMat, float fScaleVal );
	virtual void CalcWorldMat( float fTime, float fWeight, EtMatrix WorldMat, float fScaleVal );
	virtual void CalcColor( float fWeight );

	bool IsBillboard() {return m_bBillboard;}
};

class CEtBillboardEffectElementData : public CEtElementData
{
public:
	CEtBillboardEffectElementData();
	virtual ~CEtBillboardEffectElementData();

protected:
	BOOL m_bIterate;
	BOOL m_bLoop;
	BOOL m_bStopFlag;

public:
	virtual void Load( CStream *pStream, int nVersion );

	BOOL IsLoop() { return m_bLoop; }
	BOOL IsIterate() { return m_bIterate; }
	BOOL IsStop() { return m_bStopFlag; }
};

struct SEffectCustomParam
{
	int nTableType;
	CEtEffectDataContainer *pTable;
	int nParamIndex;
	char *szNameBuffer;
};
class CEtMeshElementData : public CEtElementData
{
public:
	CEtMeshElementData();
	virtual ~CEtMeshElementData();

protected:
	float m_fLoopCountRatio;
	int m_nViewAxis;
	int m_nFixedAxis;
	int m_nBlendOP;
	int m_nSourceBlend;
	int m_nDestBlend;
	int m_nCullMode;
	int m_nTilingMode;
	int m_nZWriteMode;

	CEtEffectDataContainer m_TableScaleAxis[ 3 ];
	SEffectCustomParam m_EffectCustomParam[ EFFECT_CUSTOM_PARAMETER_COUNT ];

public:
	virtual void Load( CStream *pStream, int nVersion );
	int GetBlendOP() { return m_nBlendOP; }
	int GetSrcBlend() { return m_nSourceBlend; }
	int GetDestBlend() { return m_nDestBlend; }
	int GetCullMode() {return m_nCullMode;}
	int GetTilingMode() { return m_nTilingMode; }
	int GetZWriteMode() { return m_nZWriteMode; }
	virtual void CalcWorldMat( float fTime, float fWeight, EtMatrix WorldMat, float fScaleVal);
	void CalcBillboardMatrix( EtMatrix &BillboardMat, EtVector3 *pUpVec, EtVector3 *pDir, EtVector3 *pPosition, EtVector3 *pCameraPos );
};

class CEtPointLightElementData : public CEtElementData
{
public:
	CEtPointLightElementData();
	virtual ~CEtPointLightElementData();

protected:
	int m_nDefaultRange;
	float m_fRange;

public:
	float GetRange() { return m_fRange; }
	virtual void Load( CStream *pStream, int nVersion );

	void CalcWorldMat( float fTime, float fWeight, EtMatrix WorldMat, float fScaleVal );
};

class CEtEffectData;
typedef CSmartPtr< CEtEffectData > EtEffectDataHandle;
class CEtEffectData : public CEtResource
{
public:
	CEtEffectData(void);
	virtual ~CEtEffectData(void);

protected:
	DWORD m_dwTotalFrame;
	std::vector< EtResourceHandle > m_vecResource;	// Only for caching to preload resources used by effects. Not actually used.
	std::vector< CEtElementData * > m_vecElementData;
public:

public:
	virtual int LoadResource( CStream *pStream );
	void LoadEffectResource( CStream *pStream, int nVersion );
	void LoadEffectElement( CStream *pStream, int nVersion );

	int GetElementCount() { return ( int )m_vecElementData.size(); }
	EffectResourceType GetElementType( int nIndex );
	CEtElementData *GetElementData( int nIndex ) { return m_vecElementData[ nIndex ]; }
	DWORD GetEffectLength() { return m_dwTotalFrame; }
};
