#include "StdAfx.h"
#include "EtEffectData.h"
#include "EtEffectElement.h"
#include "EtEffectDataContainer.h"
#include "DebugSet.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtElementData::CEtElementData()
{
	m_Type = ERT_UNKNOWN;
	m_bBillboard = false;
	m_dwEndFrame = 0;
	m_dwStartFrame = 0;
	m_dwYOrder = 0;
	m_nPositionInterpolationType = 0;
	memset(m_szFileName, 0, sizeof(m_szFileName));
}

CEtElementData::~CEtElementData()
{
}

void CEtElementData::Load( CStream *pStream, int nVersion )
{
	if( nVersion < 12 ) {
		pStream->Seek( 32, SEEK_CUR );
		pStream->Read( m_szFileName, 32 );
	}
	else {
		pStream->Seek( 64, SEEK_CUR );
		pStream->Read( m_szFileName, 64 );
	}

	pStream->Read( &m_dwStartFrame, sizeof( DWORD ) );
	pStream->Read( &m_dwEndFrame, sizeof(DWORD) );
	pStream->Read( &m_dwYOrder, sizeof(DWORD) );
	pStream->Read( &m_nPositionInterpolationType, sizeof( int ) );

	int i;

	for( i = 0; i < ERT_AMOUNT; i++ )
	{
		m_DataContainer[ i ].Load( pStream );
	}
}

bool CEtElementData::Process( DWORD dwTick, EtMatrix WorldMat, float fScaleVal )
{
	float fTime, fWeight;
	DWORD dwLength;

	if( dwTick < m_dwStartFrame || dwTick >= m_dwEndFrame )
	{
		return false;
	}

	fTime = ( float )( dwTick - m_dwStartFrame );
	dwLength = m_dwEndFrame - m_dwStartFrame - 1;
	fWeight = 0.0f;
	if( dwLength > 0 )
	{
		fWeight = fTime / ( float )dwLength;
	}

	CalcWorldMat( fTime, fWeight, WorldMat, fScaleVal );
	CalcColor( fWeight );

	return true;
}

void CEtElementData::CalcWorldMat( float fTime, float fWeight, EtMatrix WorldMat, float fScaleVal )
{
	float fScale;
	EtVector3 *pPosition;
	EtQuat Quat(0,0,0,1);

	pPosition = m_DataContainer[ ERT_POSITION_TABLE ].GetInterpolationVector3( fTime, NULL, (m_nPositionInterpolationType == 1) );
	EtVector3 DefaultPos(0,0,0);
	if(!pPosition) {
		pPosition = &DefaultPos;
	}

	fScale = *m_DataContainer[ ERT_SCALE_TABLE ].GetInterpolationFloat( fWeight ) * fScaleVal;

	if( m_DataContainer[ ERT_ROTATE_TABLE ].GetDataCount() == 1 )
	{
		EtVector3 *pTemp;

		pTemp = ( EtVector3 * )( m_DataContainer[ ERT_ROTATE_TABLE ].GetValueFromIndex( 0 )->GetValue() );
		EtQuaternionRotationYawPitchRoll( &Quat, EtToRadian( pTemp->x ), EtToRadian( pTemp->y ), EtToRadian( pTemp->z ) );
	}
	else
	{
		bool bSuccess = false;
		void *pResult = m_DataContainer[ ERT_ROTATE_TABLE ].GetInterpolationQuat( fTime, &bSuccess );
		if( bSuccess && pResult ) {
			Quat = *( EtQuat *)pResult;
		}
	}

	EtMatrixTransformation( &m_WorldMat, NULL, NULL, &EtVector3( fScale, fScale, fScale ), NULL, &Quat, pPosition );
	EtMatrixMultiply(&m_WorldMat, &m_WorldMat, &WorldMat);
}

void CEtElementData::CalcColor( float fWeight )
{
	float fAlpha, *pAlpha;
	EtVector3 *pColor;

	pAlpha = m_DataContainer[ ERT_ALPHA_TABLE ].GetInterpolationFloat( fWeight );
	if( pAlpha ) fAlpha = *pAlpha;
	else fAlpha = 1.0f;
	if( fAlpha > 1.0f ) fAlpha = 1.0f;
	pColor = m_DataContainer[ ERT_COLOR_TABLE ].GetInterpolationVector3( fWeight );
	if( pColor ) m_Color = EtColor( pColor->x, pColor->y, pColor->z, fAlpha );
	else m_Color = EtColor( 1.0f, 1.0f, 1.0f, fAlpha );
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
CEtBillboardEffectElementData::CEtBillboardEffectElementData()
{
	m_Type = ERT_PARTICLE;
	m_bIterate = FALSE;
	m_bLoop = FALSE;
	m_bStopFlag = FALSE;
}

CEtBillboardEffectElementData::~CEtBillboardEffectElementData()
{
}

void CEtBillboardEffectElementData::Load( CStream *pStream, int nVersion )
{
	CEtElementData::Load( pStream, nVersion );

	pStream->Read( &m_bIterate, sizeof(BOOL) );
	pStream->Read( &m_bLoop, sizeof(BOOL) );
	pStream->Read( &m_bStopFlag, sizeof(BOOL) );
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
CEtMeshElementData::CEtMeshElementData()
{
	m_Type = ERT_MESH;
	m_fLoopCountRatio = 1.f;
	m_nViewAxis = 0;
	m_nFixedAxis = 0;
	m_nBlendOP = BLENDOP_ADD;	
	m_nSourceBlend = BLEND_SRCALPHA;
	m_nDestBlend = BLEND_INVSRCALPHA;
	m_nCullMode = 0;
	m_nTilingMode = 0;
	m_nZWriteMode = 0;
	memset(m_EffectCustomParam, 0, sizeof(m_EffectCustomParam));
}

CEtMeshElementData::~CEtMeshElementData()
{
}

void CEtMeshElementData::Load( CStream *pStream, int nVersion )
{
	CEtElementData::Load( pStream, nVersion );

	if( nVersion < 11 ) {
		pStream->Read( &m_fLoopCountRatio, sizeof(float) );
	}

	pStream->Read( &m_nViewAxis, sizeof(int) );
	pStream->Read( &m_nFixedAxis, sizeof(int) );
	m_bBillboard = ( m_nViewAxis != 0 || m_nFixedAxis != 0 );
	pStream->Read( &m_nBlendOP, sizeof(int) );
	pStream->Read( &m_nSourceBlend, sizeof(int) );
	pStream->Read( &m_nDestBlend, sizeof(int) );


	m_nCullMode = 0;
	m_nTilingMode = 0;
	if( nVersion >= 13 ) {
		pStream->Read( &m_nCullMode, sizeof(int) );
	}
	if( nVersion >= 14 ) {
		pStream->Read( &m_nTilingMode, sizeof(int) );
	}

	if( nVersion != 11 ) {
		pStream->Read( &m_nZWriteMode, sizeof(int) );
		for( int i = 0; i < 3; i++ ) {
			m_TableScaleAxis[ i ].Load( pStream );
		}
	}

	if( nVersion < 11 ) {
		for( int i = 0; i< EFFECT_CUSTOM_PARAMETER_COUNT; i++ ) {
			pStream->Read( &m_EffectCustomParam[ i ].nParamIndex, sizeof( int ) );
			if( m_EffectCustomParam[ i ].nParamIndex != -1 ) {
				pStream->Read( &m_EffectCustomParam[ i ].nTableType, sizeof( int ) );
				m_EffectCustomParam[ i ].pTable->Load( pStream );
			}
		}
	}
}

void CEtMeshElementData::CalcWorldMat( float fTime, float fWeight, EtMatrix WorldMat, float fScaleVal )
{
	int i;
	float fScale = 1.f;
	EtVector3 *pPosition = NULL, ScaleVec;
	EtQuat Quat(0,0,0,1);

	pPosition = m_DataContainer[ ERT_POSITION_TABLE ].GetInterpolationVector3( fTime, NULL, (m_nPositionInterpolationType == 1) );
	EtVector3 DefaultPos(0,0,0);
	if(!pPosition) {
		pPosition = &DefaultPos;
	}

	float *pScale = m_DataContainer[ ERT_SCALE_TABLE ].GetInterpolationFloat( fWeight );
	if( pScale ) {
		fScale = (*pScale) * fScaleVal;
	}
	ScaleVec = EtVector3( 1.0f, 1.0f, 1.0f );
	for( i = 0; i < 3; i++ ) {
		if( m_TableScaleAxis[ i ].GetDataCount() ) {
			ScaleVec[ i ] = *m_TableScaleAxis[ i ].GetInterpolationFloat( fWeight );
		}
	}
	ScaleVec *= fScale;
	
	if( m_DataContainer[ ERT_ROTATE_TABLE ].GetDataCount() == 1 ) {
		EtVector3 *pTemp;
		pTemp = ( EtVector3 * )( m_DataContainer[ ERT_ROTATE_TABLE ].GetValueFromIndex( 0 )->GetValue() );
		EtQuaternionRotationYawPitchRoll( &Quat, EtToRadian( pTemp->x ), EtToRadian( pTemp->y ), EtToRadian( pTemp->z ) );
	}
	else {
		bool bSuccess = false;
		void *pResult = m_DataContainer[ ERT_ROTATE_TABLE ].GetInterpolationVector3( fTime, &bSuccess );
		if( bSuccess && pResult ) {
			Quat = *( EtQuat *)pResult;
		}
	}
	EtMatrixTransformation( &m_WorldMat, NULL, NULL, &ScaleVec, NULL, &Quat, pPosition );
		
	if( m_nViewAxis == 0 && m_nFixedAxis == 4 ) {
		EtMatrixTransformation( &m_WorldMat, NULL, NULL, NULL, NULL, &Quat, pPosition );
		EtMatrixMultiply(&m_WorldMat, &m_WorldMat, &WorldMat);
		EtMatrix BillboardMat;
		EtMatrix InvViewMat, ScaleMat;
		
		EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
		InvViewMat = *hCamera->GetInvViewMat();
		InvViewMat._41 = InvViewMat._42 = InvViewMat._43 = 0.f;
		EtMatrixScaling( &ScaleMat, ScaleVec.x, ScaleVec.y, ScaleVec.z );
		EtMatrixMultiply(&InvViewMat, &ScaleMat, &InvViewMat);

		*(EtVector3*)&m_WorldMat._11 = *(EtVector3*)&InvViewMat._11;
		*(EtVector3*)&m_WorldMat._21 = *(EtVector3*)&InvViewMat._21;
		*(EtVector3*)&m_WorldMat._31 = *(EtVector3*)&InvViewMat._31;
	}
	else if( m_nViewAxis != 0 && m_nFixedAxis != 0 ) {
		
		EtCameraHandle hCamera;
		EtMatrix InvViewMat, ScaleMat, RotateMat, BillboardMat;
		EtVector3 vTempPos, vFixedVec;
		switch( m_nFixedAxis ) {
		case 1:	
			vFixedVec = EtVector3( 1.f, 0.f, 0.f );	
			break;
		case 2:	
			vFixedVec = EtVector3( 0.f, 1.f, 0.f );	
			break;
		case 3:	
			vFixedVec = EtVector3( 0.f, 0.f, 1.f );	
			break;
		case 4: 
			vFixedVec = EtVector3( 0.f, 1.f, 0.f );	
			break;
		}
		hCamera = CEtCamera::GetActiveCamera();
		InvViewMat = *hCamera->GetInvViewMat();
		vTempPos = *pPosition;
		vTempPos.x += WorldMat._41;
		vTempPos.y += WorldMat._42;
		vTempPos.z += WorldMat._43;
		CalcBillboardMatrix( BillboardMat, &vFixedVec, (EtVector3*)&m_WorldMat._21, &vTempPos, ( EtVector3 * )&InvViewMat._41 );
		switch( m_nViewAxis ) {
		case 1: 
			EtMatrixRotationY( &RotateMat, EtToRadian( -90.0f ) );
			break;
		case 2: 
			EtMatrixRotationY( &RotateMat, EtToRadian( 90.0f ) );
			break;
		case 3:	
			EtMatrixRotationX( &RotateMat, EtToRadian( 90.0f ) );
			break;
		case 4:	
			EtMatrixRotationX( &RotateMat, EtToRadian( -90.0f ) );
			break;
		case 5:
			EtMatrixIdentity( &RotateMat );
			break;
		case 6: 
			EtMatrixRotationX( &RotateMat, EtToRadian( 180.0f ) );
			break;
		}
		EtMatrixMultiply( &BillboardMat, &RotateMat, &BillboardMat );
		EtMatrixScaling( &ScaleMat, ScaleVec.x, ScaleVec.y, ScaleVec.z );
		EtMatrixMultiply( &m_WorldMat, &ScaleMat, &BillboardMat );
	}
	else {
		EtMatrixMultiply(&m_WorldMat, &m_WorldMat, &WorldMat);
	}
}

void CEtMeshElementData::CalcBillboardMatrix( EtMatrix &BillboardMat, EtVector3 *pUpVec, EtVector3 *pDir, EtVector3 *pPosition, EtVector3 *pCameraPos )
{
	float fLengthSq;
	EtVector3 XVec, UpVec, ZVec;
	fLengthSq = EtVec3LengthSq( pUpVec );
	if( fLengthSq <= 0.0f ) {
		UpVec = *pDir;
	}
	else {
		UpVec = *pUpVec;
		EtVec3Normalize( &UpVec, &UpVec );
	}
	ZVec = *pCameraPos - *pPosition;
	EtVec3Normalize( &ZVec, &ZVec );
	EtVec3Cross( &XVec, &UpVec, &ZVec );
	EtVec3Normalize( &XVec, &XVec );
	if( fLengthSq <= 0.0f ) {
		EtVec3Cross( &UpVec, &ZVec, &XVec );
		EtVec3Normalize( &UpVec, &UpVec );
	}
	else {
		EtVec3Cross( &ZVec, &XVec, &UpVec );
		EtVec3Normalize( &ZVec, &ZVec );
	}
	EtMatrixIdentity( &BillboardMat );
	memcpy( &BillboardMat._11, &XVec, sizeof( EtVector3 ) );
	memcpy( &BillboardMat._21, &UpVec, sizeof( EtVector3 ) );
	memcpy( &BillboardMat._31, &ZVec, sizeof( EtVector3 ) );
	memcpy( &BillboardMat._41, pPosition, sizeof( EtVector3 ) );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEtPointLightElementData::CEtPointLightElementData()
{
	m_Type = ERT_POINT_LIGHT;
	m_fRange = 0.0f;
	m_nDefaultRange = 0;
}

CEtPointLightElementData::~CEtPointLightElementData()
{
}

void CEtPointLightElementData::Load( CStream *pStream, int nVersion )
{
	CEtElementData::Load( pStream, nVersion );
	if( nVersion < 12 ) {
		pStream->Read( &m_nDefaultRange, sizeof( int ) );
	}
}

void CEtPointLightElementData::CalcWorldMat( float fTime, float fWeight, EtMatrix WorldMat , float fScaleVal )
{
	float fScale;
	EtVector3 *pPosition;
	EtQuat Quat;

	pPosition = m_DataContainer[ ERT_POSITION_TABLE ].GetInterpolationVector3( fTime, NULL, (m_nPositionInterpolationType == 1) );
	EtVector3 DefaultPos(0,0,0);
	if(!pPosition) {
		pPosition = &DefaultPos;
	}
	EtMatrixTranslation( &m_WorldMat, pPosition->x, pPosition->y, pPosition->z );

	fScale = *m_DataContainer[ ERT_SCALE_TABLE ].GetInterpolationFloat( fWeight ) * fScaleVal;
	m_fRange = fScale * m_nDefaultRange;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEtEffectData::CEtEffectData(void)
{
	m_dwTotalFrame = 0;
}

CEtEffectData::~CEtEffectData(void)
{
	int i;
	for( i = 0; i < ( int )m_vecResource.size(); i++ )
	{
		if( m_vecResource[ i ] )
		{
			if( m_vecResource[ i ]->Release() <= 0 )
			{
				delete m_vecResource[ i ];
			}
		}
	}
	m_vecResource.clear();
	SAFE_DELETE_PVEC( m_vecElementData );
}

int CEtEffectData::LoadResource( CStream *pStream )
{
	SEffectFileHeader Header;

	pStream->Read( &Header, sizeof( SEffectFileHeader ) );
	assert( strstr(Header.szHeaderString, "Eternity Engine Effect File") );

	if( strstr(Header.szHeaderString, EFFECT_FILE_STRING) == NULL ) return ETERR_FILENOTFOUND;

	pStream->Seek( EFFECT_HEADER_RESERVED, SEEK_CUR );
	if( Header.nVersion < 12) {
		pStream->Seek( EFFECT_TOOL_INFO, SEEK_CUR );
	}

	LoadEffectResource( pStream, Header.nVersion );
	LoadEffectElement( pStream, Header.nVersion );

	return ET_OK;
}

void CEtEffectData::LoadEffectResource( CStream *pStream, int nVersion )
{
	EffectResourceType Type;
	DWORD i, dwResourceCount;
	//SEffectToolInfo ToolHeader;
	EtResourceHandle hResource;

	pStream->Read( &dwResourceCount, sizeof( DWORD ) );
	for( i = 0; i < dwResourceCount; i++ )
	{
		pStream->Read( &Type, sizeof( EffectResourceType ) );
		char szFileName[64]={0,};
		if( nVersion < 12 ) {
			SEffectToolInfo ToolHeader;
			pStream->Read( &ToolHeader, sizeof( SEffectToolInfo ) );
			strcpy(szFileName, ToolHeader.szFileName);
		}
		else {
			pStream->Read( szFileName, 64 );
		}
		switch( Type )
		{
		case ERT_PARTICLE:
			hResource = ::LoadResource( szFileName, RT_PARTICLE );
			if( hResource )
				m_vecResource.push_back( hResource );
			break;
		case ERT_MESH:		
			hResource = ::LoadResource( szFileName, RT_SKIN );
			if( hResource )
				m_vecResource.push_back( hResource );
			break;
		case ERT_POINT_LIGHT:	
			/*hResource.Identity();
			m_vecResource.push_back( hResource );*/
			break;
		default:	
			ASSERT( 0 );
			break;
		}
	}
}

void CEtEffectData::LoadEffectElement( CStream *pStream, int nVersion )
{
	DWORD i, dwCount;
	CEtElementData *pElementData = NULL;
	EffectResourceType Type;

	if( nVersion < 12 ) {
		pStream->Seek( 32, SEEK_CUR );
	}
	pStream->Read( &m_dwTotalFrame, sizeof( DWORD ) );

	pStream->Read( &dwCount, sizeof( DWORD ) );
	for( i = 0; i < dwCount; i++ )
	{
		pStream->Read( &Type, sizeof( EffectResourceType ) );
		switch( Type )
		{
		case ERT_PARTICLE:
			pElementData = new CEtBillboardEffectElementData();
			break;
		case ERT_MESH:		
			pElementData = new CEtMeshElementData();
			break;
		case ERT_POINT_LIGHT:	
			pElementData = new CEtPointLightElementData();
			break;
		default:
			ASSERT( 0 );
			break;
		}

		if (pElementData)
		{
			pElementData->Load( pStream, nVersion );
			m_vecElementData.push_back( pElementData );
		}
	}
}

EffectResourceType CEtEffectData::GetElementType( int nIndex )
{ 
	if (nIndex < 0 || nIndex >= (int)m_vecElementData.size())
		return ERT_UNKNOWN;

	return m_vecElementData[ nIndex ]->GetType();
}
