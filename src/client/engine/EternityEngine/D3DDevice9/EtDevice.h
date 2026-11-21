#pragma once
#include <ddraw.h>
#include "CriticalSection.h"
#include "SmartPtr.h"
#include "EtTexture.h"

struct FnDeviceOutOfMemory 
{
	virtual void Run() 
	{
		throw std::bad_alloc();
	}
};

struct VertexDecl
{
	EtVertexDecl *pDecl;
	D3DVERTEXELEMENT9 Elem[ 32 ];
	UINT	nElemCount;
};

#define ALPHA_TEST_VALUE (0x7F)

#define PRE_MOD_MEMORY_CHECK

class CEtTexture;
class CEtSystemFont;
class CEtDevice : public CSingleton< CEtDevice >
{	
public:
	CEtDevice();
	virtual ~CEtDevice();

protected:
	CSyncLock m_DeviceLock;
	LPDIRECT3D9						m_pD3D;
	LPDIRECT3DDEVICE9				m_pDevice;
	LPDIRECTDRAW7					m_pDD;
	D3DPRESENT_PARAMETERS			m_Param;
	HWND							m_hWnd;	
	std::vector< D3DDISPLAYMODE >	m_vecDisplayMode;
	D3DCAPS9						m_Caps;
	EtSurface						*m_pBackBufferSurface;
	EtSurface						*m_pDepthStencilSurface;
	ID3DXFont						*m_pFont;
	CEtSystemFont					*m_pSystemFont;
	EtSurface						*m_pCurSetRenderTarget;
	EtSurface						*m_pCurSetDepthStencil;
	std::vector< char * >			m_EffectMacroString;
	std::vector< D3DXMACRO >		m_EffectMacro;
	std::vector< VertexDecl >		m_vecVertexDecl;
	bool							m_bShaderDebug;
	int								m_nAdapter;
	D3DDEVTYPE						m_DevType;
	D3DFORMAT						m_AdapterFormat;	
	EtMultiSampleType				m_SupportedAntiAliasType;
	FnDeviceOutOfMemory				*m_pOutOfMemoryCallBack;
	CSyncLock						m_DeviceLostLock;
#ifdef PRE_MOD_MEMORY_CHECK
	DWORD							m_dwLocalVideoMemory;
#else
	bool							m_bUseDDrawMemCheck;
#endif
	DWORD							m_dwTotalAvailTextureMemory;
	int								m_nCurrentPolyCount;
	int								m_nPolyCountPerFrame;
	bool							m_bCallOutOfMemory;
	std::string						m_szLastErrorMsg;

	std::vector<std::string>		m_vecApplyQualityPath;

	void (__stdcall *m_pPrePresentCallback)();
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	bool (__stdcall *m_pFlushWaitDeleteCallback)();
#endif

#ifdef PRE_MOD_MEMORY_CHECK
public:
	DWORD							m_dwUsingLocalVideoMemory;	// Debug variable
	DWORD							m_dwMaxLocalVideoMemory;
#endif
	
public:
	void Clear();
	void CreateDDraw();
	void EnumDisplayMode();
	void FindSuitableDisplayMode( int &nWidth, int &nHeight );
	int FindSuitableRefreshRate( int nWidth, int nHeight );
	int Initialize( HWND hWnd, int nWidth, int nHeight, bool bWindow = false, bool bEnableMultiThread = false, bool bVSync = true, bool bEnableShaderDebug = false );
	void Reinitialize( int nWidth, int nHeight );
	// Does not actually switch to windowed mode; calling this before Reinitialize makes Reinitialize switch to windowed mode.
	void SetWindowMode( bool bWindow ) { m_Param.Windowed = bWindow; }
	void SetVSync( bool bVSync );
	void SetDefaultState();
	void ClearBuffer( D3DCOLOR COLOR, float fZ = 1.0f, DWORD dwStencil = 0, bool bClearColor = true, bool bClearZ = true, bool bClearStencil = true );
	EtDeviceCheck DeviceValidCheck();
	void BeginScene() { m_pDevice->BeginScene(); }
	void EndScene() { m_pDevice->EndScene(); }
	void ShowFrame( RECT *pTargetRect = NULL );
	HWND GetHWnd() { return m_hWnd; }
	void *GetDevicePtr() { return m_pDevice; }
	D3DPRESENT_PARAMETERS *GetPresentParameter() { return &m_Param; }
	int Width() { return m_Param.BackBufferWidth; }
	int Height() { return m_Param.BackBufferHeight; }
	void SetSize( DWORD dwWidth, DWORD dwHeight ) { m_Param.BackBufferWidth = dwWidth; m_Param.BackBufferHeight = dwHeight; }
	void GetViewport( EtViewPort *pViewPort ) { m_pDevice->GetViewport( pViewPort ); }
	void SetViewport( EtViewPort *pViewPort ) { m_pDevice->SetViewport( pViewPort ); }
	EtSurface *GetRenderTarget() { return m_pCurSetRenderTarget; }
	void SetRenderTarget( EtSurface *pSurface );
	EtSurface *GetDepthStencilSurface() { return m_pCurSetDepthStencil; }
	void SetDepthStencilSurface( EtSurface *pSurface );
	void RestoreRenderTarget() { SetRenderTarget( m_pBackBufferSurface ); }
	void RestoreDepthStencil() { SetDepthStencilSurface( m_pDepthStencilSurface ); }
	void DumpBackBuffer( EtSurface *pSurface, LPDIRECT3DSURFACE9 pSourceSurface = NULL);
	void SaveBackbuffer( const char *pFileName, D3DXIMAGE_FILEFORMAT Format = D3DXIFF_DDS );
	void SaveBackbuffer( WCHAR *pFileName, D3DXIMAGE_FILEFORMAT Format = D3DXIFF_DDS );
	int FindVertexDeclaration( D3DVERTEXELEMENT9 *pVertexElements );
	int CreateVertexDeclaration( D3DVERTEXELEMENT9 *pVertexElements );
	EtVertexDecl *GetVertexDeclaration( int nIndex );
	void GetVertexElement( int nIndex, D3DVERTEXELEMENT9* pElement, UINT* pNumElements );
	void SetVertexDeclaration( int nIndex );
	EtVertexBuffer *CreateVertexBuffer( int nLength, DWORD dwFVF, EtUsage Usage = USAGE_DEFAULT, EtPool Pool = POOL_MANAGED );
	EtIndexBuffer *CreateIndexBuffer( int nLength, EtUsage Usage = USAGE_DEFAULT, EtPool Pool = POOL_MANAGED );
	void SetStreamSource( int nStreamNum, EtVertexBuffer *pBuffer, int nStride, int nStartIndex = 0 );
	void SetStreamSourceFreq( UINT StreamNumber, UINT FrequencyParameter ) {m_pDevice->SetStreamSourceFreq(StreamNumber, FrequencyParameter); }
	void SetIndexBuffer( EtIndexBuffer *pBuffer );
	EtBaseTexture *CreateTexture( int nWidth, int nHeight, EtFormat Format, EtUsage Usage = USAGE_DEFAULT, EtPool Pool = POOL_DEFAULT, int nLevel = 1 );
	EtSurface *CreateRenderTarget( int nWidth, int nHeight, EtFormat Format, EtPool Pool, EtMultiSampleType MultiSampleType );
	EtBaseTexture *CreateRenderTargetTexture( int nWidth, int nHeight, EtFormat Format, EtUsage Usage = USAGE_DEFAULT, EtPool Pool = POOL_DEFAULT );
	EtSurface *CreateDepthStencil( int nWidth, int nHeight, EtFormat Format, EtMultiSampleType MultiSampleType = MULTISAMPLE_NONE );
	EtBaseTexture *LoadTextureFromMemory( char *pSource, int nSrcSize, bool bPow2 = false, UINT nWidth = 0, UINT nHeight = 0, int nMipLevel = D3DX_DEFAULT );
	EtBaseTexture *LoadVolumeTextureFromMemory( char *pSource, int nSrcSize, bool bPow2 = false, UINT nWidth = 0, UINT nHeight = 0 );
	EtBaseTexture *LoadCubeTextureFromMemory( char *pSource, int nSrcSize, bool bPow2 = false, UINT nWidth = 0, UINT nHeight = 0 );
	EtBaseTexture *LoadTextureFromFile( const char *pFileName, bool bPow2 = false );
	EtBaseTexture *LoadVolumeTextureFromFile( const char *pFileName, bool bPow2 = false );
	EtBaseTexture *LoadCubeTextureFromFile( const char *pFileName, bool bPow2 = false );
	void SetTexture( int nSampler, EtBaseTexture *pTexture );
	void ClearEffectMacro();
	void AddEffectMacro( char *szName, char *szDefinition );
	EtEffect *LoadEffectFromMemory( char *pSource, int nSrcSize, void *pSharedEffectPool = NULL, ID3DXInclude *pInclude = NULL );
	EtEffect *LoadEffectFromFile( const char *pFileName, void *pSharedEffectPool = NULL );
	
	void DrawPrimitive( PrimitiveType nPrimitiveType, int nStartVertex, int nPrimitiveCount );
	void DrawIndexedPrimitive( PrimitiveType nPrimitiveType, int nStartVertex, int nVertexCount, int nStartIndex, int nPrimitiveCount );
	void DrawPrimitiveUP( PrimitiveType nPrimitiveType, int nPrimitiveCount, void *pVertexData, int nStride );
	void DrawIndexedPrimitiveUP( PrimitiveType nPrimitiveType, int nMinVertexIndex, int nNumVertices, int nPrimitiveCount, void *pIndexData, EtFormat IndexDataFormat, void *pVertexData, int nStride );

	void *CreateFont( const char *pFontName );
	void *CreateSystemFont( const char *pFontName );
	void *CreateSprite();
	void SetCullMode( CullMode Mode );
	void SetWireframe( bool bEnable );
	bool EnableAlphaBlend( bool bEnable );
	bool IsEnableAlphaBlend();
	bool EnableAlphaTest( bool bEnable );
	bool IsEnableAlphaTest();
	bool EnableZ( bool bEnable );
	bool EnableZTest( bool bZTest );
	bool IsEnableZTest();
	bool EnableZWrite( bool bEnable );
	EtCmpFunc SetZFunc( EtCmpFunc ZFunc );
	bool IsEnableZWrite();
	float SetDepthBias( float fBiass );
	void GetRenderState( D3DRENDERSTATETYPE State, DWORD *pdwValue );
	void SetRenderState( D3DRENDERSTATETYPE State, DWORD dwValue );
	void GetSamplerState( DWORD Sampler, D3DSAMPLERSTATETYPE State, DWORD *pdwValue );
	void SetSamplerState( DWORD Sampler, D3DSAMPLERSTATETYPE State, DWORD Value );
	void GetTextureStageState( DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
	void SetTextureStageState( DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	DWORD SetColorWriteEnable( DWORD dwEnable );
	EtBlendOP SetBlendOP( EtBlendOP BlendOP );
	EtBlendMode SetSrcBlend( EtBlendMode BlendMode );
	EtBlendMode SetDestBlend( EtBlendMode BlendMode );
	void SetWorldTransform( EtMatrix *pWorldMat );
	void SetViewTransform( EtMatrix *pViewMat );
	void SetProjTransform( EtMatrix *pProjMat );
	void GetFVF( DWORD *dwFVF );
	void SetFVF( DWORD dwFVF );
	void SetVertexShader( LPDIRECT3DVERTEXSHADER9 pShader );
	void SetPixelShader( LPDIRECT3DPIXELSHADER9 pShader );
	/*void EnableLight( bool bEnable );*/
	void SetCursorProperties( EtSurface *pSurface, UINT nXHotSpot = 1, UINT nYHotSpot = 1 );
	void ShowCursor( bool bShow );
	void SetCursorPos( int nX, int nY );
	void SetGammaRamp( float fGamma, int nBright );
	float GetMemoryUsePercent();
	DWORD SetAlphaRef( DWORD dwAlphaRef );
	DWORD GetAlphaRef();
	// Device Caps functions
	int GetSimultaneousRTs() { return m_Caps.NumSimultaneousRTs; }
	int GetVSVersion() { return m_Caps.VertexShaderVersion; }
	int GetPSVersion() { return m_Caps.PixelShaderVersion; }
	DWORD GetTextureFilterCaps() { return m_Caps.TextureFilterCaps; }
	DWORD GetMaxAnisotropy() { return m_Caps.MaxAnisotropy;}
	DWORD GetMaxTextureWidth() { return m_Caps.MaxTextureWidth; }
	DWORD GetMaxTextureHeight() { return m_Caps.MaxTextureHeight; }
	bool IsAutoGenMipmap() { return (m_Caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) != 0;}
	bool CheckDeviceFormat( EtFormat CheckFormat, EtUsage Usage );
	int GetDisplayModeCount() { return ( int )m_vecDisplayMode.size(); }
	D3DDISPLAYMODE *GetDisplayMode( int nIndex ) { return &m_vecDisplayMode[ nIndex ]; }
	int SetDialogBoxMode(bool b) { if( m_pDevice ) return m_pDevice->SetDialogBoxMode(b?TRUE:FALSE); return 0; }
	void SetClipCursor( bool bClip );
	void GetAdapterIdentifier( D3DADAPTER_IDENTIFIER9 *did ) { m_pD3D->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, did); }
	EtMultiSampleType	GetSupportedAAType() { return m_SupportedAntiAliasType; }
	void OnOutOfMemory();
	bool IsCallOutOfMemory() { return m_bCallOutOfMemory; }
	char *GetLastErrorMsg() { return (char*)m_szLastErrorMsg.c_str(); }
	void SetOutOfMemoryCallBack( FnDeviceOutOfMemory *pFunc ) { m_pOutOfMemoryCallBack = pFunc; }
	void EvictManagedResources() { m_pDevice->EvictManagedResources(); }
#ifdef PRE_MOD_MEMORY_CHECK
	void CheckTotalLocalVideoMemory();
#else
	void UseDDrawMemoyCheck( bool bUseDDraw ) { m_bUseDDrawMemCheck = bUseDDraw; }
#endif
	void *SetPrePresentCallback( void ( __stdcall *pCallback )() ) 
	{ 
		void ( __stdcall *pBackupPrePresentCallback )();
		pBackupPrePresentCallback = m_pPrePresentCallback;
		m_pPrePresentCallback = pCallback; 
		return pBackupPrePresentCallback;
	}
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	void SetWaitDeleteCallback( bool ( __stdcall *pCallback )() )
	{
		m_pFlushWaitDeleteCallback = pCallback;
	}
	bool CheckFlushWaitDelete();
#endif
	int GetPolyCountPerFrame() { return m_nPolyCountPerFrame; }

	void GetScissorRect( RECT *pRect ) {m_pDevice->GetScissorRect( pRect ); }
	void SetScissorRect( RECT *pRect ) {m_pDevice->SetScissorRect( pRect ); }

	HRESULT CheckDeviceType( UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed ) {
		return m_pD3D->CheckDeviceType( Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed );
	}

	HRESULT CreateRenderToSurface( UINT Width, UINT Height, EtFormat Format, BOOL DepthStencil, EtFormat DepthStencilFormat, LPD3DXRENDERTOSURFACE*  ppRenderToSurface ) {
		return D3DXCreateRenderToSurface( m_pDevice, Width, Height, (D3DFORMAT)Format, DepthStencil, (D3DFORMAT)DepthStencilFormat, ppRenderToSurface );
	}

	void SetApplyQualityPath( std::string strApplyPath ) { m_vecApplyQualityPath.push_back( strApplyPath ); }
	std::vector<std::string>& GetApplyQualityPath() { return m_vecApplyQualityPath; }

	// Whether graphics card only supports basic shaders.
	bool IsOnlyLowShaderAvailable( void );
};

extern CEtDevice g_EtDevice;
inline CEtDevice *GetEtDevice() { return &g_EtDevice; }
