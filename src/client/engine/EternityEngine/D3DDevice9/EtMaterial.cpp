#include "StdAfx.h"
#include "EtType.h"
#include "EtCallBack.h"
#include "EtMaterial.h"
#include "EtSaveMat.h"
#include "EtResourceMng.h"
#include "EtStateManager.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

LPD3DXEFFECTPOOL CEtMaterial::s_pEffectPool = NULL;
std::vector< SHADER_COMPILE_INFO > CEtMaterial::s_CompiledShaders;
int CEtMaterial::s_nShaderQuality = 0;

SGlobalParam g_GlobalParam[] =
{
	{ "g_ProjMat",				GetProjectionMat,		EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_ViewMat",				GetViewMat,				EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_ViewRotProjMat",		GetViewRotProjMat,		EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_PrevViewRotProjMat",	GetPrevViewRotProjMat,	EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_ViewPosition",			GetViewPosition,		EPT_VECTOR, VariableCountOne,	-1 },
	{ "g_FogFactor",			GetFogFactor,			EPT_VECTOR,	VariableCountOne,	-1 },
	{ "g_FogColor",				GetFogColor,			EPT_VECTOR,	VariableCountOne,	-1 },
	{ "g_FogSkyBoxTex",			GetFogSkyBoxTex,		EPT_TEX,	VariableCountOne,	-1 },
	{ "g_LightAmbient",			GetLightAmbient,		EPT_VECTOR,	VariableCountOne,	-1 },
	{ "g_DirLightCount",		GetDirLightCount,		EPT_INT,	VariableCountOne,	-1 },
	{ "g_DirLightDirection",	GetDirLightDirection,	EPT_VECTOR,	VariableCountDirLight,	-1 },
	{ "g_ShadowMapTex",			GetShadowMap,			EPT_TEX,	VariableCountOne,	-1 },
	{ "g_fShadowDensity",		GetShadowDensity,		EPT_VECTOR,	VariableCountOne,	-1 },
	{ "g_FetchDist",				GetShadowFetchDistance,		EPT_FLOAT,	VariableCountOne,	-1 },
	{ "g_fElapsedTime",			GetElapsedTime,			EPT_FLOAT,	VariableCountOne,	-1 },
	{ "g_EnvTex",				GetEnvTexture,			EPT_TEX,	VariableCountOne,	-1 },
	{ "g_BackBuffer",			GetBackBuffer,			EPT_TEX,	VariableCountOne,	-1 },
	{ "g_ScreenSizeScale",		GetScreenSizeScale,		EPT_VECTOR, VariableCountOne,	-1 },
	{ "g_AlphaRef",				GetDepthAlphaRef,		EPT_FLOAT,	VariableCountOne,	-1 },/*
	{ "g_DepthShadowBias",	GetDepthShadowBias,		EPT_FLOAT,	VariableCountOne,	-1 },
	{ "g_DepthShadowBias2",	GetDepthShadowBias2,		EPT_FLOAT,	VariableCountOne,	-1 },*/
};
#define GLOBAL_PARAM_COUNT		( sizeof( g_GlobalParam ) / sizeof( SGlobalParam ) )

SGlobalParam g_WorldMatParam[] =
{
	{ "g_WorldMat",					GetWorldMat,			EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_InvViewMat",				GetInvViewMat,			EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_WorldViewMat",				GetWorldViewMat,		EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_ViewProjMat",				GetViewProjMat,			EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_WorldViewProjMat",			GetWorldViewProjMat,	EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_PrevWorldViewProjMat",		GetPrevWorldViewProjMat,	EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_InvWorldViewPrevWVPMat",	GetInvWorldViewPrevWorldViewProjMat,	EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_ModelViewMat",				GetModelViewMat,		EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_InvViewModelViewMat",		GetInvViewModelViewMat,	EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_WorldLightViewProjMat",	GetWorldLightViewProjMat,	EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_InvViewLightViewProjMat",	GetInvViewLightViewProjMat,	EPT_MATRIX, VariableCountOne,	-1 },
	{ "g_WorldLightViewProjDepth",	GetWorldLightViewProjDepth,	EPT_VECTOR, VariableCountOne,	-1 },
	{ "g_InvViewLightViewProjDepth",	GetInvViewLightViewProjDepth,	EPT_VECTOR, VariableCountOne,	-1 },
	{ "g_DirLightDiffuse",		GetDirLightDiffuse,		EPT_VECTOR,	VariableCountDirLight,	-1 },		// The reason Directional Light color is placed in WorldMat Param is because lightmap influence was multiplied here
	{ "g_DirLightSpecular",		GetDirLightSpecular,	EPT_VECTOR,	VariableCountDirLight,	-1 },       // Even if GlobalParam setup is skipped because it's the same material, this must still be set.
	{ "g_PointLightCount",		GetPointLightCount,		EPT_INT,	VariableCountOne,	-1 },
	{ "g_PointLightPosition",	GetPointLightPosition,	EPT_VECTOR,	VariableCountPointLight,	-1 },
	{ "g_PointLightDiffuse",	GetPointLightDiffuse,	EPT_VECTOR,	VariableCountPointLight,	-1 },
	{ "g_PointLightSpecular",	GetPointLightSpecular,	EPT_VECTOR,	VariableCountPointLight,	-1 },
	{ "g_SpotLightCount",		GetSpotLightCount,		EPT_INT,	VariableCountOne,	-1 },
	{ "g_SpotLightDirection",	GetSpotLightDirection,	EPT_VECTOR,	VariableCountSpotLight,	-1 },
	{ "g_SpotLightPosition",	GetSpotLightPosition,	EPT_VECTOR,	VariableCountSpotLight,	-1 },
	{ "g_SpotLightDiffuse",		GetSpotLightDiffuse,	EPT_VECTOR,	VariableCountSpotLight,	-1 },
	{ "g_SpotLightSpecular",	GetSpotLightSpecular,	EPT_VECTOR,	VariableCountSpotLight,	-1 },
};

#define WORLDMAT_PARAM_COUNT	( sizeof( g_WorldMatParam ) / sizeof( SGlobalParam ) )

SGlobalParam g_PreComputeParam[] =
{
	{ "g_EmissiveColor",		ComputeEmissive,			EPT_VECTOR, VariableCountEmissive,	-1 },
	{ "g_SkinCoeffA",			GetCoefficientA,	EPT_VECTOR, VariableCountOne, -1 },
	{ "g_SkinCoeffB",			GetCoefficientB,	EPT_VECTOR, VariableCountOne, -1 },
	{ "g_SkinCoeffC",			GetCoefficientC,	EPT_VECTOR, VariableCountOne, -1 },
	{ "g_SkinCoeffD",			GetCoefficientD,	EPT_VECTOR, VariableCountOne, -1 },
	{ "g_ComputeAniTime",			GetAniTime, 	EPT_FLOAT, VariableCountOne, -1 },
};

#define PRECOMPUTE_PARAM_COUNT	( sizeof( g_PreComputeParam ) / sizeof( SGlobalParam ) )

CEtMaterial::CEtMaterial(void)
{
	m_pEffect = NULL;
	m_hWorldMatArray = NULL;
	m_bUseTangentSpace = false;
	m_bUseBackBuffer = false;
	m_nCurrentTechnique = -1;
}

CEtMaterial::~CEtMaterial(void)
{
	Clear();
}

void CEtMaterial::Clear()
{
	SAFE_RELEASE( m_pEffect );
	SAFE_DELETE_VEC( m_GlobalEffectParam );
	SAFE_DELETE_VEC( m_WorldMatEffectParam );
	SAFE_DELETE_VEC( m_PreComputeParam );
	SAFE_DELETE_VEC( m_CustomEffectParam );

	for( std::vector< void* >::iterator it = m_vecUserAllocMemList.begin(); it != m_vecUserAllocMemList.end(); ++it ) {
		::free( *it );
	}
	m_vecUserAllocMemList.clear();
	m_nCurrentTechnique = -1;
	m_hWorldMatArray = NULL;
	m_bUseTangentSpace = false;
	m_bUseBackBuffer = false;

	m_vecParamHandle.clear();
	m_vecParamName.clear();
	m_vecParamUsed.clear();
}

int CEtMaterial::LoadResource( CStream *pStream )
{
	ASSERT( pStream && "Invalid Resource Stream( Texture )!!!" );
	if( pStream == NULL )
	{
		return ETERR_INVALIDRESOURCESTREAM;
	}

	Clear();
	CreateEffectPool();

	if( !s_CompiledShaders.empty() && pStream->GetName() == NULL ) {
		int i, nSize;
		nSize = (int)s_CompiledShaders.size();
		const char *pFileName = ((CResMngStream*)pStream)->GetFullName();
		for( i = 0; i < nSize; i++) 
		{
			if( stricmp(s_CompiledShaders[i].szFileName, pFileName) == 0 && s_CompiledShaders[i].nQualityLevel == s_nShaderQuality ) {
				m_pEffect = GetEtDevice()->LoadEffectFromMemory( (char*)s_CompiledShaders[i].pCompiledBuffer, s_CompiledShaders[i].dwBufferSize, s_pEffectPool );
				m_szFullName = pFileName;
				m_szFileName = FindFileName( pFileName );
				break;
			}
		}
	}
	else {
		char *pBuffer;
		int nFileSize;

		nFileSize = pStream->Size();
		pBuffer = new char[ nFileSize ];
		pStream->Read( pBuffer, nFileSize );

		CEtShaderInclude Include;
		m_pEffect = GetEtDevice()->LoadEffectFromMemory( pBuffer, nFileSize, s_pEffectPool, &Include );

		SAFE_DELETEA( pBuffer );
//		m_pEffect = GetEtDevice()->LoadEffectFromFile( pStream->GetName(), s_pEffectPool );
	}
	if( m_pEffect == NULL ) {
		ASSERT( 0 && "LoadEffectFromFile Failed" );
		return ETERR_LOADEFFECTFAIL;	
	}
	SaveParamHandle();
	CreateAllVertexDecl();
	CreateGlobalParam();

	m_pEffect->SetStateManager( GetEtStateManager() );

	return ET_OK;
}

DWORD CEtMaterial::GetParameterUsedFlag( EtHandle hHandle )
{
	DWORD dwParamUsedFlag = 0;
	for( int j = 0; j < GetTechniqueCount(); j++) 
	{
		if( m_pEffect->IsParameterUsed( hHandle, m_pEffect->GetTechnique( j ) ) )
		{
			dwParamUsedFlag	 |= (1<<j);
		}
	}
	return dwParamUsedFlag;
}

void CEtMaterial::SaveParamHandle()
{
	if( !m_vecParamHandle.empty() )
	{
		int i, j, nParamCount;

		memset( &m_vecParamHandle[ 0 ], 0, sizeof( EtHandle ) * m_vecParamHandle.size() );
		nParamCount = GetParameterCount();
		for( i = 0; i < nParamCount; i++ )
		{
			bool bFind;
			EtHandle hHandle;
			D3DXPARAMETER_DESC Desc;

			bFind = false;
			hHandle = m_pEffect->GetParameter( NULL, i );
			m_pEffect->GetParameterDesc( hHandle, &Desc );
			for( j = 0; j < ( int )m_vecParamName.size(); j++ )
			{
				if( stricmp( m_vecParamName[ j ].c_str(), Desc.Name ) == 0 )
				{
					m_vecParamHandle[ j ] = hHandle;
					m_vecParamUsed[ j ] = GetParameterUsedFlag( hHandle );
					bFind = true;
					break;
				}
			}
			if( !bFind )
			{
				m_vecParamHandle.push_back( hHandle );
				m_vecParamName.push_back( Desc.Name );				
				m_vecParamUsed.push_back( GetParameterUsedFlag(hHandle) );
			}
		}
	}
	else
	{
		int i, nCount;

		nCount = GetParameterCount();
		m_vecParamHandle.resize( nCount );
		for( i = 0; i < nCount; i++ )
		{
			D3DXPARAMETER_DESC Desc;
			m_vecParamHandle[ i ] = m_pEffect->GetParameter( NULL, i );		
			m_pEffect->GetParameterDesc( m_vecParamHandle[ i ], &Desc );
			m_vecParamName.push_back( Desc.Name );
			m_vecParamUsed.push_back( GetParameterUsedFlag( m_vecParamHandle[ i ] ) );
		}
	}
}

void CEtMaterial::Reload()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	Clear();
	Load( GetFullName() );
	Release();
}

void CEtMaterial::OnLostDevice()
{
	if( m_pEffect )
	{
		m_pEffect->OnLostDevice();
	}
}

void CEtMaterial::OnResetDevice()
{
	if( m_pEffect )
	{
		m_pEffect->OnResetDevice();
	}
}

int CEtMaterial::GetTechniqueCount()
{
	D3DXEFFECT_DESC EffectDesc;

	m_pEffect->GetDesc( &EffectDesc );

	return EffectDesc.Techniques;
}

int CEtMaterial::GetPassCount( EtHandle hTechnique )
{
	D3DXTECHNIQUE_DESC TechniqueDesc;

	m_pEffect->GetTechniqueDesc( hTechnique, &TechniqueDesc );

	return TechniqueDesc.Passes;
}

int CEtMaterial::GetParameterCount()
{
	D3DXEFFECT_DESC EffectDesc;

	m_pEffect->GetDesc( &EffectDesc );

	return EffectDesc.Parameters;	
}

EtParameterHandle CEtMaterial::GetParameterByName( const char *pName ) 
{ 
	EtHandle hFindHandle;
	int i;
	if( !m_pEffect ) {
		return -1;
	}

	hFindHandle = m_pEffect->GetParameterByName( NULL, pName );
	for( i = 0; i < ( int )m_vecParamHandle.size(); i++ )
	{
		if( m_vecParamHandle[ i ] == hFindHandle )
		{
			return i;
		}
	}
	return -1; 
}

const char *CEtMaterial::GetParameterName( EtParameterHandle hParam )
{
	D3DXPARAMETER_DESC ParamDesc;

	m_pEffect->GetParameterDesc( m_vecParamHandle[ hParam ], &ParamDesc );

	return ParamDesc.Name;
}

D3DXPARAMETER_TYPE CEtMaterial::GetParameterType( EtParameterHandle hParam )
{
	D3DXPARAMETER_DESC ParamDesc;

	m_pEffect->GetParameterDesc( m_vecParamHandle[ hParam ], &ParamDesc );

	return ParamDesc.Type;
}

EtTextureType CEtMaterial::GetTextureType( EtParameterHandle hHandle )
{
	if( hHandle < 0 || hHandle >= (int) m_vecParamHandle.size() ) {
		return ETTEXTURE_NORMAL;
	}
	D3DXPARAMETER_DESC Desc;
	EtTextureType TextureType;

	m_pEffect->GetParameterDesc( m_vecParamHandle[ hHandle ], &Desc );
	switch( Desc.Type )
	{
		case D3DXPT_TEXTURE3D:
			TextureType = ETTEXTURE_VOLUME;
			break;
		case D3DXPT_TEXTURECUBE:
			TextureType = ETTEXTURE_CUBE;
			break;
		default:
			TextureType = ETTEXTURE_NORMAL;
			break;
	}

	return TextureType;
}

const char *CEtMaterial::GetSemantic( EtParameterHandle hHandle )
{
	D3DXPARAMETER_DESC Desc;

	m_pEffect->GetParameterDesc( m_vecParamHandle[ hHandle ], &Desc );

	return Desc.Semantic;
}

int CEtMaterial::SetTechnique( int nIndex )
{
	EtHandle hTechnique;
	
	hTechnique = m_pEffect->GetTechnique( nIndex );
	if( hTechnique == NULL )
	{
		ASSERT( 0 && "Invalid Technique Handle" );
		return ETERR_INVALIDTECHNIQUEHANDLE;
	}
	m_pEffect->SetTechnique( hTechnique );
	m_nCurrentTechnique = nIndex;

	return ET_OK;
}
int CEtMaterial::CreateVertexDecl( EtHandle hTechnique, EtHandle hPass, bool bMergeType )
{
	int i, nOffset;
	UINT nSemanticCount;
	D3DXPASS_DESC PassDesc;
	D3DXSEMANTIC Semantic[ 20 ];	// Only about 20 items.
	D3DVERTEXELEMENT9 VertexElement[ 20 ];

	m_pEffect->GetPassDesc( hPass, &PassDesc );
	D3DXGetShaderInputSemantics( PassDesc.pVertexShaderFunction, Semantic, &nSemanticCount );

	nOffset = 0;
	for( i = 0; i < ( int )nSemanticCount; i++ )
	{
		if( bMergeType )
		{
			VertexElement[ i ].Stream = 0;
		}
		else
		{
			VertexElement[ i ].Stream = i;
		}
		if( bMergeType )
		{
			VertexElement[ i ].Offset = nOffset;
		}
		else
		{
			VertexElement[ i ].Offset = 0;
		}
		VertexElement[ i ].Method = D3DDECLMETHOD_DEFAULT;
		VertexElement[ i ].Usage = Semantic[ i ].Usage;
		VertexElement[ i ].UsageIndex = Semantic[ i ].UsageIndex;

		switch( Semantic[ i ].Usage )		// Vertex buffer compression is not accounted for. by mapping
		{
			case D3DDECLUSAGE_POSITION:
			case D3DDECLUSAGE_NORMAL:
				VertexElement[ i ].Type = D3DDECLTYPE_FLOAT3;
				nOffset += sizeof( float ) * 3;
				break;
			case D3DDECLUSAGE_TANGENT:
			case D3DDECLUSAGE_BINORMAL:
				m_bUseTangentSpace = true;
				VertexElement[ i ].Type = D3DDECLTYPE_FLOAT3;
				nOffset += sizeof( float ) * 3;
				break;
			case D3DDECLUSAGE_BLENDWEIGHT:
				VertexElement[ i ].Type = D3DDECLTYPE_FLOAT4;
				nOffset += sizeof( float ) * 4;
				break;
			case D3DDECLUSAGE_BLENDINDICES:
				VertexElement[ i ].Type = D3DDECLTYPE_SHORT4;
				nOffset += sizeof( short ) * 4;
				break;
			case D3DDECLUSAGE_TEXCOORD:
				VertexElement[ i ].Type = D3DDECLTYPE_FLOAT2;
				nOffset += sizeof( float ) * 2;
				break;
			case D3DDECLUSAGE_COLOR:
				VertexElement[ i ].Type = D3DDECLTYPE_D3DCOLOR;
				nOffset += sizeof( DWORD );
				break;
			case D3DDECLUSAGE_DEPTH:
				VertexElement[ i ].Type = D3DDECLTYPE_FLOAT1;
				nOffset += sizeof( float );
				break;
			case D3DDECLUSAGE_PSIZE:
				VertexElement[ i ].Type = D3DDECLTYPE_FLOAT1;
				nOffset += sizeof( float );
				break;
			default:
				ASSERT( 0 && "Invalid Semantic!!!" );
				VertexElement[ i ].Type = D3DDECLTYPE_UNUSED;
				break;
		}
	}

	VertexElement[ nSemanticCount ].Stream = 0xff;
	VertexElement[ nSemanticCount ].Offset = 0;
	VertexElement[ nSemanticCount ].Type   = D3DDECLTYPE_UNUSED;
	VertexElement[ nSemanticCount ].Method = 0;
	VertexElement[ nSemanticCount ].Usage  = 0;
	VertexElement[ nSemanticCount ].UsageIndex = 0;

	return GetEtDevice()->CreateVertexDeclaration( VertexElement );
}

void CEtMaterial::CreateAllVertexDecl()
{
	UINT i, j, nTechniqueCount, nPassCount;
	D3DXHANDLE hTechnique, hPass;

	nTechniqueCount = GetTechniqueCount();
	m_vecVertexDecl.resize( nTechniqueCount );
	m_vecMergedVertexDecl.resize( nTechniqueCount );
	for( i = 0; i < nTechniqueCount; i++ )
	{
		hTechnique = m_pEffect->GetTechnique( i );
		nPassCount = GetPassCount( hTechnique );
		m_vecVertexDecl[ i ].resize( nPassCount );
		m_vecMergedVertexDecl[ i ].resize( nPassCount );
		for( j = 0; j < nPassCount; j++ )
		{
			hPass = m_pEffect->GetPass( hTechnique, j );
			m_vecVertexDecl[ i ][ j ] = CreateVertexDecl( hTechnique, hPass );
			m_vecMergedVertexDecl[ i ][ j ] = -1;
		}
	}
}

int CEtMaterial::GetVertexDeclIndex( int nTechniqueIndex, int nPassIndex, bool bMergeType )
{
	if( bMergeType )
	{
		if( nTechniqueIndex >= ( int )m_vecMergedVertexDecl.size() ) {
			return -1;
		}
		if( nPassIndex >= ( int )m_vecMergedVertexDecl[ nTechniqueIndex ].size() ) {
			return -1;
		}

		if( m_vecMergedVertexDecl[ nTechniqueIndex ][ nPassIndex ] == -1 )
		{
			D3DXHANDLE hTechnique, hPass;

			hTechnique = m_pEffect->GetTechnique( nTechniqueIndex );
			hPass = m_pEffect->GetPass( hTechnique, nPassIndex );
			m_vecMergedVertexDecl[ nTechniqueIndex ][ nPassIndex ] = CreateVertexDecl( hTechnique, hPass, true );
		}
		return m_vecMergedVertexDecl[ nTechniqueIndex ][ nPassIndex ];
	}
	else
	{
		if( nTechniqueIndex >= ( int )m_vecVertexDecl.size() ) {
			return -1;
		}
		if( nPassIndex >= ( int )m_vecVertexDecl[ nTechniqueIndex ].size() ) {
			return -1;
		}
		return m_vecVertexDecl[ nTechniqueIndex ][ nPassIndex ];
	}
}

void CEtMaterial::CreateEffectPool()
{
	if( !s_pEffectPool )
	{
		D3DXCreateEffectPool( &s_pEffectPool );
		ADD_D3D_RES( s_pEffectPool );
	}
}

void CEtMaterial::LoadCompiledShaders()
{
	if( s_CompiledShaders.empty() ) 
	{
		CResMngStream Stream("DnShaders.dat");
		if( !Stream.IsValid() ) return;
		int nCount;
		DWORD dwHeader;
		Stream.Read(&dwHeader, sizeof(DWORD));
		Stream.Read(&nCount, sizeof(DWORD));
		for( int i = 0; i < nCount; i++) 
		{
			SHADER_COMPILE_INFO scInfo;
			Stream.Read(scInfo.szFileName, 64);
			Stream.Read(&scInfo.nQualityLevel, 4);
			Stream.Read(&scInfo.dwBufferSize, 4);
			scInfo.pCompiledBuffer = new BYTE[ scInfo.dwBufferSize ];
			Stream.Read(scInfo.pCompiledBuffer, scInfo.dwBufferSize);
			s_CompiledShaders.push_back( scInfo );
		}
	}
}

void CEtMaterial::DeleteCompiledShaders()
{
	int i, nSize;
	nSize = (int)s_CompiledShaders.size();
	for( i = 0; i < nSize; i++) {
		delete [] s_CompiledShaders[i].pCompiledBuffer;
	}
	s_CompiledShaders.clear();
}

void CEtMaterial::CreateGlobalParam()
{
	int i, nParamCount;

	nParamCount = ( int )m_vecParamHandle.size();
	for( i = 0; i < nParamCount; i++ )
	{
		D3DXPARAMETER_DESC ParamDesc;

		if( m_vecParamHandle[ i ] == NULL )
		{
			continue;
		}
		m_pEffect->GetParameterDesc( m_vecParamHandle[ i ], &ParamDesc );
		if( stricmp( ParamDesc.Name, "g_BackBuffer") == 0 ) 
		{
			m_bUseBackBuffer = true;
		}
		if( stricmp( ParamDesc.Name, "g_WorldViewMatArray" ) == 0 ) 
		{
			m_hWorldMatArray = m_vecParamHandle[ i ];
		}		
		else
		{
			if( !AddGlobalParam( i, ParamDesc.Name ) )
			{
				if( !AddWorldMatParam( i, ParamDesc.Name ) )
				{
					if( !AddPreComputeParam( i, ParamDesc.Name ) )  
					{
						AddCustomParam( i, ParamDesc );
					}					
				}
			}
		}
	}
}

bool CEtMaterial::AddGlobalParam( EtParameterHandle hParam, const char *pParamName )
{
	int i;
	SCustomParam GlobalParam;

	for( i = 0; i < GLOBAL_PARAM_COUNT; i++ )
	{
		if( stricmp( pParamName, g_GlobalParam[ i ].szParamName ) == 0 )
		{
			GlobalParam.hParamHandle = hParam;
			GlobalParam.pCallBack = g_GlobalParam[ i ].pCallBack;
			GlobalParam.Type = EPT_VARIABLE;
			GlobalParam.VariableType = g_GlobalParam[ i ].VariableType;
			GlobalParam.pCountCallBack = g_GlobalParam[ i ].pCountCallBack;
			m_GlobalEffectParam.push_back( GlobalParam );
			return true;
		}
	}

	return false;
}

bool CEtMaterial::AddWorldMatParam( EtParameterHandle hParam, const char *pParamName )
{
	int i;
	SCustomParam WorldMatParam;

	for( i = 0; i < WORLDMAT_PARAM_COUNT; i++ )
	{
		if( stricmp( pParamName, g_WorldMatParam[ i ].szParamName ) == 0 )
		{
			WorldMatParam.hParamHandle = hParam;
			WorldMatParam.pCallBack = g_WorldMatParam[ i ].pCallBack;
			WorldMatParam.Type = EPT_VARIABLE;
			WorldMatParam.VariableType = g_WorldMatParam[ i ].VariableType;
			WorldMatParam.pCountCallBack = g_WorldMatParam[ i ].pCountCallBack;
			m_WorldMatEffectParam.push_back( WorldMatParam );
			return true;
		}
	}

	return false;
}

bool CEtMaterial::AddPreComputeParam( EtParameterHandle hParam, const char *pParamName )
{
	int i;
	SCustomParam PreComputeParam;

	for( i = 0; i < PRECOMPUTE_PARAM_COUNT; i++ )
	{
		if( stricmp( pParamName, g_PreComputeParam[ i ].szParamName ) == 0 )
		{
			PreComputeParam.hParamHandle = hParam;
			PreComputeParam.pCallBack = g_PreComputeParam[ i ].pCallBack;
			PreComputeParam.Type = EPT_VARIABLE;
			PreComputeParam.VariableType = g_PreComputeParam[ i ].VariableType;
			PreComputeParam.pCountCallBack = g_PreComputeParam[ i ].pCountCallBack;
			m_PreComputeParam.push_back( PreComputeParam );
			return true;
		}
	}

	return false;
}

void CEtMaterial::AddCustomParam( EtParameterHandle hParam, D3DXPARAMETER_DESC &Desc )
{
	SCustomParam CustomParam;

	CustomParam.hParamHandle = hParam;
	switch( Desc.Type )
	{
		case D3DXPT_INT:
			CustomParam.Type = EPT_INT;
			break;
		case D3DXPT_FLOAT:
			switch( Desc.Bytes ) {
				case 4: CustomParam.Type = EPT_FLOAT; break;
				case 16: CustomParam.Type = EPT_VECTOR; break;
				case 64: CustomParam.Type = EPT_MATRIX; break;
				default: return;
			}
			/*
			if( Desc.Rows > 1 )
			{
				CustomParam.Type = EPT_VECTOR;
			}
			else if( Desc.Columns > 1 )
			{
				CustomParam.Type = EPT_MATRIX;
			}
			else
			{
				CustomParam.Type = EPT_FLOAT;
			}
			*/
			break;
		case D3DXPT_TEXTURE:
		case D3DXPT_TEXTURE1D:
		case D3DXPT_TEXTURE2D:
		case D3DXPT_TEXTURE3D:
		case D3DXPT_TEXTURECUBE:
			CustomParam.Type = EPT_TEX;
			break;
		default:
			return;
	}
	m_CustomEffectParam.push_back( CustomParam );
}

void CEtMaterial::SetCustomParamList( std::vector< SCustomParam > &vecCustomParam )
{
	SetPreComputeParams( vecCustomParam );

	int i, j, nSize, nPreSize;

	nSize = ( int )vecCustomParam.size();

	nPreSize = ( int )m_PreComputeParam.size();

	for( i = 0; i < nSize; i++ )
	{
		for( j = 0; j < nPreSize; j++) {
			if( m_PreComputeParam[j].hParamHandle == vecCustomParam[ i ].hParamHandle ) {
				break;
			}
		}
		if( j == nPreSize ) {
			SetCustomParam( &vecCustomParam[ i ] );
		}
	}
}

void CEtMaterial::SetCustomParam( SCustomParam *pParam, void *pAddParam, void *pAddParam2 )
{
	if( ( pParam == NULL ) || ( pParam->hParamHandle < 0 ) || ( pParam->hParamHandle >= (int)m_vecParamHandle.size() ) )
	{
		//ASSERT( 0 && "Invalid Custom Param!!!" );
		return;
	}
	EtHandle hParamHandle;

	hParamHandle = m_vecParamHandle[ pParam->hParamHandle ];
	if( hParamHandle == NULL )
	{
		return;
	}

	// SetTechnique() function must be called before SetCustomParam().
	BOOL bUsed = ( m_vecParamUsed[ pParam->hParamHandle ] & (1<<m_nCurrentTechnique) ) != 0 ;
	if( !bUsed ) {
		return;
	}

#if defined(_DEBUG) || defined(_RDEBUG)
	const std::string &paramName = m_vecParamName[ pParam->hParamHandle ];
#endif


	switch( pParam->Type )
	{
		case EPT_INT:
			if( pParam->nVariableCount == 1 )
			{
				m_pEffect->SetInt( hParamHandle, pParam->nInt );
			}
			else
			{
				if( pParam->nVariableCount > 0 )
					m_pEffect->SetIntArray( hParamHandle, ( int * )pParam->pPointer, pParam->nVariableCount );
			}
			break;
		case EPT_FLOAT:
			if( pParam->nVariableCount == 1 )
			{
				m_pEffect->SetFloat( hParamHandle, pParam->fFloat );
			}
			else
			{
				if( pParam->nVariableCount ) {
					m_pEffect->SetFloatArray( hParamHandle, ( float * )pParam->pPointer, pParam->nVariableCount );
				}
			}
			break;
		case EPT_VECTOR:
			if( pParam->nVariableCount == 1 )
			{
				m_pEffect->SetVector( hParamHandle, ( EtVector4 * )pParam->fFloat4 );
			}
			else
			{
				if( pParam->nVariableCount > 0 ) {
					m_pEffect->SetVectorArray( hParamHandle, ( EtVector4 * )pParam->pPointer, pParam->nVariableCount );
				}
			}
			break;
		case EPT_MATRIX:
			assert( false && "Use EPT_MATRIX_PTR !!");
			break;
		case EPT_INT_PTR:
			if( pParam->pPointer )
			{
				m_pEffect->SetInt( hParamHandle, *( int * )pParam->pPointer );
			}
			break;
		case EPT_FLOAT_PTR:
			if( pParam->pPointer )
			{
				m_pEffect->SetFloat( hParamHandle, *( float * )pParam->pPointer );
			}
			break;
		case EPT_VECTOR_PTR:
			if( pParam->pPointer )
			{
				if( pParam->nVariableCount > 1 )
				{
					m_pEffect->SetVectorArray( hParamHandle, ( EtVector4 * )pParam->pPointer, pParam->nVariableCount );
				}
				else
				{
					m_pEffect->SetVector( hParamHandle, ( EtVector4 * )pParam->pPointer );
				}
			}
			break;
		case EPT_MATRIX_PTR:
			if( pParam->pPointer )
			{
				if( pParam->nVariableCount > 1 )
				{
					m_pEffect->SetMatrixArray( hParamHandle, ( EtMatrix * )pParam->pPointer, pParam->nVariableCount );
				}
				else
				{
					m_pEffect->SetMatrix( hParamHandle, ( EtMatrix * )pParam->pPointer );
				}
			}
			break;

		case EPT_TEX:
		{
			EtTextureHandle hTexture;

			if( pParam->nTextureIndex == -1 )
			{
				m_pEffect->SetTexture( hParamHandle, NULL );
				break;
			}
			hTexture = CEtResource::GetResource( pParam->nTextureIndex );
			if( ( hTexture ) && ( hTexture->GetResourceType() == RT_TEXTURE ) )
			{
				m_pEffect->SetTexture( hParamHandle, hTexture->GetTexturePtr() );
			}
			break;
		}

		case EPT_VARIABLE:
			int nVariableCount = pParam->pCountCallBack( pAddParam, pAddParam2 );
			switch( pParam->VariableType )
			{
				case EPT_INT:
					if( nVariableCount & 0x80000000 )
					{
						m_pEffect->SetIntArray( hParamHandle, ( int * )pParam->pCallBack( pAddParam, pAddParam2 ), nVariableCount & 0x7fffffff );
					}
					else
					{
						m_pEffect->SetInt( hParamHandle, *( int * )pParam->pCallBack( pAddParam, pAddParam2 ) );
					}
					break;
				case EPT_FLOAT:
					if( nVariableCount & 0x80000000 )
					{
						m_pEffect->SetFloatArray( hParamHandle, ( float * )pParam->pCallBack( pAddParam, pAddParam2 ), nVariableCount & 0x7fffffff );
					}
					else
					{
						m_pEffect->SetFloat( hParamHandle, *( float * )pParam->pCallBack( pAddParam, pAddParam2 ) );
					}
					break;
				case EPT_VECTOR:
					if( nVariableCount & 0x80000000 )
					{
						m_pEffect->SetVectorArray( hParamHandle, ( EtVector4 *)pParam->pCallBack( pAddParam, pAddParam2 ), nVariableCount & 0x7fffffff );
					}
					else
					{
						m_pEffect->SetVector( hParamHandle, ( EtVector4 *)pParam->pCallBack( pAddParam, pAddParam2 ) );
					}
					break;
				case EPT_MATRIX:
					if( nVariableCount & 0x80000000 )
					{
						m_pEffect->SetMatrixArray( hParamHandle, ( EtMatrix * )pParam->pCallBack( pAddParam, pAddParam2 ), nVariableCount & 0x7fffffff );
					}
					else
					{
						m_pEffect->SetMatrix( hParamHandle, ( EtMatrix * )pParam->pCallBack( pAddParam, pAddParam2 ) );
					}
					break;
				case EPT_TEX:
					m_pEffect->SetTexture( hParamHandle, ( EtBaseTexture * )pParam->pCallBack( pAddParam, pAddParam2 ) );
					break;
			}
			break;
	}
}

void CEtMaterial::SetGlobalParams()
{
	int i, nSize;

	nSize = ( int )m_GlobalEffectParam.size();
	
	for( i = 0; i < nSize; i++ )
	{
		SetCustomParam( &m_GlobalEffectParam[ i ] );
	}
}

void CEtMaterial::SetWorldMatParams( EtMatrix *pWorldMat, EtMatrix *pPrevWorldMat, int nSaveMatIndex )
{
	EtMatrix *pSaveMat, WorldMat, PrevWorldMat;

	pSaveMat = GetEtSaveMat()->GetMatrix( nSaveMatIndex );
	EtMatrixMultiply( &WorldMat, pSaveMat, pWorldMat );
	if( pPrevWorldMat )
	{
		EtMatrixMultiply( &PrevWorldMat, pSaveMat, pPrevWorldMat );
	}
	SetWorldMatParams( &WorldMat, &PrevWorldMat );
}

void CEtMaterial::SetWorldMatParams( EtMatrix *pWorldMat, EtMatrix *pPrevWorldMat )
{
	int i, nSize;

	nSize = ( int )m_WorldMatEffectParam.size();

	for( i = 0; i < nSize; i++ )
	{
		SetCustomParam( &m_WorldMatEffectParam[ i ], pWorldMat, pPrevWorldMat );
	}
}

void CEtMaterial::SetPreComputeParams( std::vector< SCustomParam > &vecCustomParam )
{
	int i, nSize;
	nSize = ( int )m_PreComputeParam.size();

	for( i = 0; i < nSize; i++ )
	{
		SetCustomParam( &m_PreComputeParam[ i ] ,this , &vecCustomParam);
	}
}

void CEtMaterial::SetWorldMatArray( EtMatrix *pWorldMat, int nSaveMatIndex, int nBoneCount, const int *pBoneIndex )
{
	if( !GetEtSaveMat()->IsValidIndex( nSaveMatIndex ) )
	{
		return;
	}
	
	if( !m_hWorldMatArray ) 
	{
		return;
	}

	EtMatrix WorldViewMat = *(EtMatrix*)GetWorldViewMat( pWorldMat, NULL );

	GetEtSaveMat()->Transform( nSaveMatIndex, WorldViewMat );

	EtMatrix WorldViewMatArray[ 50 ];	// Let's reserve quite a bit.. thinking around 50 max
	EtMatrix *pWorldMatArray;
	pWorldMatArray = GetEtSaveMat()->GetTransMatrix( nSaveMatIndex );
	ASSERT( nBoneCount <= 50);
	for( int i = 0; i < nBoneCount; i++ )
	{
		WorldViewMatArray[ i ] = (pBoneIndex[ i ] == -1) ? WorldViewMat : pWorldMatArray[ pBoneIndex[ i ] ];			
	}
	m_pEffect->SetMatrixArray( m_hWorldMatArray, WorldViewMatArray, nBoneCount );	
}

void CEtMaterial::ReloadMaterial()
{
	CEtResource::FlushWaitDelete();		// Delete all Materials with RefCount 0.

	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i;
	int nCount = GetItemCount();
	EtMaterialHandle hMaterial;

	for( i = 0; i < nCount; i++ )
	{
		hMaterial = GetItem( i );
		if( ( hMaterial ) && ( hMaterial->GetResourceType() == RT_SHADER ) )
		{
			hMaterial->Reload();
		}
	}
}

void CEtMaterial::BeginEffect( int &nPasses )
{
	if( GetEtStateManager()->IsEnable() ) {
		GetEtStateManager()->FlushDeferedStates();
		m_pEffect->Begin( ( UINT * )&nPasses, D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE);
	}
	else {
		m_pEffect->Begin( ( UINT * )&nPasses, 0 );
	}
}

void CEtMaterial::BeginEffect()
{
	if( GetEtStateManager()->IsEnable() ) {
		GetEtStateManager()->FlushDeferedStates();
		m_pEffect->Begin( NULL, D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE );
	}
	else {
		m_pEffect->Begin( NULL, 0 );
	}
}

void CEtMaterial::EndEffect()
{
	m_pEffect->End();
	if( GetEtStateManager()->IsEnable() ) {
		for( int i = 0; i < CEtStateBlockManager::STAGE_MAX; i++) {
			GetEtStateManager()->SetSamplerStateDefered( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
			GetEtStateManager()->SetSamplerStateDefered( i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
			GetEtStateManager()->SetSamplerStateDefered( i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

			GetEtStateManager()->SetSamplerStateDefered( i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
			GetEtStateManager()->SetSamplerStateDefered( i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
		}
	}
}
int FindCustomParam( std::vector< SCustomParam > &ParamList, EtParameterHandle hHandle ) 
{
	int i;

	for( i = 0; i < ( int )ParamList.size(); i++ )
	{
		if( ParamList[ i ].hParamHandle == hHandle )
		{
			return i;
		}
	}

	return -1;
}

int AddCustomParam( std::vector< SCustomParam >	&ParamList, EffectParamType Type, EtMaterialHandle hMaterial, 
				   const char *pParamName, void *pValue, int nParamCount )
{
	int nParamIndex;
	SCustomParam CustomParam;

	CustomParam.Type = Type;
	CustomParam.hParamHandle = hMaterial->GetParameterByName( pParamName );
	ASSERT( CustomParam.hParamHandle != -1 );
	CustomParam.nVariableCount = nParamCount;
	nParamIndex = FindCustomParam( ParamList, CustomParam.hParamHandle ); 
	if( nParamCount > 1 )
	{
		CustomParam.pPointer = pValue;
	}
	else
	{
		switch( Type )
		{
			case EPT_INT:
				memcpy( &CustomParam.nInt, pValue, sizeof( int ) );
				break;
			case EPT_FLOAT:
				memcpy( &CustomParam.fFloat, pValue, sizeof( float ) );
				break;
			case EPT_VECTOR:
				memcpy( &CustomParam.fFloat4, pValue, sizeof( float ) * 4 );
				break;
			case EPT_TEX:
				memcpy( &CustomParam.nTextureIndex, pValue, sizeof( int ) );
				break;
			case EPT_INT_PTR:
			case EPT_FLOAT_PTR:
			case EPT_VECTOR_PTR:
			case EPT_MATRIX_PTR:
				CustomParam.pPointer = pValue;
				break;
		}
	}
	if( nParamIndex == -1 )
	{
		ParamList.push_back( CustomParam );
		nParamIndex = ( int )ParamList.size() - 1;
	}
	else
	{
		ParamList[ nParamIndex ] = CustomParam;
	}

	return nParamIndex;
}

CEtShaderInclude::CEtShaderInclude()
{
}

HRESULT CEtShaderInclude::Open( D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes )
{
	CResMngStream Stream( pFileName );
	if( !Stream.IsValid() ) return E_FAIL;

	char *pBuffer = new char[Stream.Size()];
	Stream.Read( pBuffer, Stream.Size() );
	*ppData = pBuffer;
	*pBytes = Stream.Size();

	return S_OK;
}

HRESULT CEtShaderInclude::Close( LPCVOID pData )
{
	char *pBuffer = (char*)pData;
	SAFE_DELETEA( pBuffer );
	return S_OK;
}
