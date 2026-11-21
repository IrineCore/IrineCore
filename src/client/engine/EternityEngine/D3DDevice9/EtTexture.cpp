#include "StdAfx.h"
#include "EtTexture.h"
#include "EtType.h"
#include "EtOptionController.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

EtBaseTexture *CEtTexture::s_pEnvTexture = NULL;
bool CEtTexture::s_bPow2 = false;
EtTextureHandle CEtTexture::m_hWhiteTexture;
EtTextureHandle CEtTexture::m_hBlankTexture;

#ifdef PRE_MOD_IGNORE_MIPMAP
std::vector<std::string> CEtTexture::s_vecIgnoreMipmapPath;
std::vector<std::string> CEtTexture::s_vecMipmapPath;
#endif

CEtTexture::CEtTexture(void)
{
	m_pTexture = NULL;
	m_pSurface = NULL;
	m_TextureType = ETTEXTURE_NORMAL;
	m_ResourceType = RT_TEXTURE;
	m_nAlphaTexture = -1;
	m_nLockLevel = 0;
	m_nOriginalWidth = 0;
	m_nOriginalHeight = 0;
	m_nFileSize = 0;
}

CEtTexture::~CEtTexture(void)
{
	if( m_pTexture == s_pEnvTexture )
	{
		s_pEnvTexture = NULL;
	}
	Clear();
}

void CEtTexture::Clear()
{
	SAFE_RELEASE( m_pTexture );
	SAFE_RELEASE( m_pSurface );
	m_nFileSize = 0;
}

int CEtTexture::LoadResource( CStream *pStream )
{
	ASSERT( pStream && "Invalid Resource Stream( Texture )!!!" );
	if( pStream == NULL || !pStream->IsValid() )
	{
		m_nWidth = 0;
		m_nHeight = 0;
		return ETERR_INVALIDRESOURCESTREAM;
	}

	Clear();

	char *pBuffer;
	D3DSURFACE_DESC Desc;

	m_nFileSize = pStream->Size();
	pBuffer = new char[ m_nFileSize ];
	pStream->Read( pBuffer, m_nFileSize );

	D3DXIMAGE_INFO ImageInfo;
	D3DXGetImageInfoFromFileInMemory( pBuffer, m_nFileSize, &ImageInfo );
	m_TextureType = ( EtTextureType )ImageInfo.ResourceType;

	// Adjust texture size to High, Medium, or Low according to graphic options
	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );
	UINT nWidth = 0;
	UINT nHeight = 0;
	if( Option.nTextureQuality > 0 )
	{
		std::vector<std::string>::iterator iter = GetEtDevice()->GetApplyQualityPath().begin();
		for( ; iter != GetEtDevice()->GetApplyQualityPath().end(); iter++ )
		{
			if( strstr( pStream->GetName(), (*iter).c_str() ) )
			{
				nWidth = ImageInfo.Width;
				nHeight = ImageInfo.Height;

 				for( int i=0; i<Option.nTextureQuality; i++ )
				{
					int nHalfWidth = nWidth / 2;
					int nHalfHeight = nHeight / 2;
					if( nHalfWidth < 64 || nHalfHeight < 64 )
						break;
					nWidth = nHalfWidth;
					nHeight = nHalfHeight;
				}

				if( nWidth % 2 != 0 )
					nWidth = nWidth + 1;
				if( nHeight % 2 != 0 )
					nHeight = nHeight + 1;
			}
		}
	}

#ifdef PRE_MOD_IGNORE_MIPMAP
	bool bForceUseMipmap = false;
	std::vector<std::string>::iterator iter = s_vecMipmapPath.begin();
	if( m_TextureType == ETTEXTURE_NORMAL ) {
		for( ; iter != s_vecMipmapPath.end(); iter++ ) {
			if( strstr( pStream->GetName(), (*iter).c_str() ) )
				bForceUseMipmap = true;
		}
	}
	int nMipLevel = D3DX_DEFAULT;
	if( bForceUseMipmap == false ) {
		iter = s_vecIgnoreMipmapPath.begin();
		if( m_TextureType == ETTEXTURE_NORMAL ) {
			for( ; iter != s_vecIgnoreMipmapPath.end(); iter++ ) {
				if( strstr( pStream->GetName(), (*iter).c_str() ) )
					nMipLevel = 1;
			}
		}
	}
#endif
	switch( m_TextureType )
	{
		case ETTEXTURE_NORMAL:
#ifdef PRE_MOD_IGNORE_MIPMAP
			m_pTexture = GetEtDevice()->LoadTextureFromMemory( pBuffer, m_nFileSize, s_bPow2, nWidth, nHeight, nMipLevel );
#else
			m_pTexture = GetEtDevice()->LoadTextureFromMemory( pBuffer, m_nFileSize, s_bPow2, nWidth, nHeight );			
#endif
			if( m_pTexture ) {
				( ( EtTexture * )m_pTexture )->GetLevelDesc( 0, &Desc );
			}
			m_nWidth = Desc.Width;
			m_nHeight = Desc.Height;
			break;
		case ETTEXTURE_VOLUME:
			m_pTexture = GetEtDevice()->LoadVolumeTextureFromMemory( pBuffer, m_nFileSize, s_bPow2, nWidth, nHeight );
			break;
		case ETTEXTURE_CUBE:
			m_pTexture = GetEtDevice()->LoadCubeTextureFromMemory( pBuffer, m_nFileSize, s_bPow2, nWidth, nHeight );
			break;
	}

	if( !m_pTexture ) {
		delete []pBuffer;
		return ETERR_OUTOFMEMORY;
	}

	m_nOriginalWidth = ImageInfo.Width;
	m_nOriginalHeight = ImageInfo.Height;

	delete [] pBuffer;

	return ET_OK;
}

int CEtTexture::Width() 
{ 
	if( ( m_nWidth == -1 ) || ( m_nHeight == -1 ) )
	{
		return GetEtDevice()->Width();
	}
	else
	{
		return m_nWidth; 
	}
}
int CEtTexture::Height()
{ 
	if( ( m_nWidth == -1 ) || ( m_nHeight == -1 ) )
	{
		return GetEtDevice()->Height();
	}
	else
	{
		return m_nHeight; 
	}
}

EtSurface *CEtTexture::GetSurfaceLevel()
{
	if( m_pSurface == NULL )
	{
		ASSERT(m_pTexture&&"CEtTexture::GetSurfaceLevel, m_pTexture is NULL!");
		HRESULT hr = S_OK;
		if( m_pTexture ) {
			hr = ( ( EtTexture * )m_pTexture )->GetSurfaceLevel( 0, &m_pSurface );
			if( m_pSurface ) {
				ADD_D3D_RES( m_pSurface );
			}
		}
		if( !m_pSurface )
		{
			OutputDebug("%x Error Code\n", hr);
			ASSERT( 0 && "Cannot Find SurfaceLevel" );
		}
	}

	return m_pSurface;
}

void *CEtTexture::Lock( int &nStride, bool bCanWrite )
{
/*	if( m_TextureType != ETTEXTURE_NORMAL )
	{
		ASSERT( 0 );
		return NULL;
	}*/
	if( !m_pTexture )
	{
		return NULL;
	}

	D3DLOCKED_RECT Rect;
	Rect.pBits = NULL;
	Rect.Pitch = 0;
	m_nLockLevel = 0;
	HRESULT hr = ( ( EtTexture * )m_pTexture )->LockRect( m_nLockLevel, &Rect, NULL, ( bCanWrite ) ? 0 : D3DLOCK_READONLY );
	if( hr != D3D_OK ) {
		return NULL;
	}
	nStride = Rect.Pitch;

	return Rect.pBits;
}

void CEtTexture::Unlock()
{
	( ( EtTexture * )m_pTexture )->UnlockRect( m_nLockLevel );
}

void CEtTexture::CreateTexture( int nWidth, int nHeight, EtFormat Format, EtUsage Usage, EtPool Pool, int nLevel )
{
	CheckResourceMemory();

	m_TextureType = ETTEXTURE_NORMAL;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_Format = Format;
	m_Usage = Usage;
	m_Pool = Pool;
	m_pTexture = GetEtDevice()->CreateTexture( nWidth, nHeight, Format, Usage, Pool, nLevel );
}

EtTextureHandle CEtTexture::CreateNormalTexture( int nWidth, int nHeight, EtFormat Format, EtUsage Usage, EtPool Pool, int nLevel )
{
	CEtTexture *pTexture;

	pTexture = new CEtTexture();
	pTexture->CreateTexture( nWidth, nHeight, Format, Usage, Pool, nLevel );
	pTexture->AddRef();

	return pTexture->GetMySmartPtr();
}

void CEtTexture::CreateRenderTarget( int nWidth, int nHeight, EtFormat Format, EtUsage Usage, EtPool Pool )
{
	CheckResourceMemory();

	m_TextureType = ETTEXTURE_RENDER_TARGET;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_Format = Format;
	m_Usage = Usage;
	m_Pool = Pool;
	m_pTexture = GetEtDevice()->CreateRenderTargetTexture( nWidth, nHeight, Format, Usage, Pool );
	SetDeleteImmediate( true );		// RenderTarget must be deleted immediately on Release instead of queuing.
}

EtTextureHandle CEtTexture::CreateRenderTargetTexture( int nWidth, int nHeight, EtFormat Format, EtUsage Usage, EtPool Pool )
{
	CEtTexture *pTexture;

	pTexture = new CEtTexture();
	pTexture->CreateRenderTarget( nWidth, nHeight, Format, Usage, Pool );
	pTexture->AddRef();

	return pTexture->GetMySmartPtr();
}

#ifdef PRE_CRASH_CHECK_BACKUP
float g_fMemoryUsePercent;
#endif
bool CEtTexture::ChangeFormat( EtFormat Format, EtUsage Usage, EtPool Pool )
{
	EtTexture *pOldTexture;

	SAFE_RELEASE( m_pSurface );
	pOldTexture = ( EtTexture * )m_pTexture;
	CreateTexture( m_nWidth, m_nHeight, Format, Usage, Pool );
	if( m_pTexture == NULL )
	{
		m_pTexture = pOldTexture;
		return false;
	}

	EtSurface *pOldSurface, *pNewSurface;
	pOldTexture->GetSurfaceLevel( 0, &pOldSurface );
	( ( EtTexture * )m_pTexture )->GetSurfaceLevel( 0, &pNewSurface );
	HRESULT hResult = D3DERR_DEVICELOST;
	if( ( pOldSurface ) && ( pNewSurface ) )
	{
#ifdef PRE_CRASH_CHECK_BACKUP
		g_fMemoryUsePercent = GetEtDevice()->GetMemoryUsePercent();
#endif
		hResult = D3DXLoadSurfaceFromSurface( pNewSurface, NULL, NULL, pOldSurface, NULL, NULL, D3DX_DEFAULT, 0 );
	}
	ADD_D3D_RES( pNewSurface );
	ADD_D3D_RES( pOldSurface );
	SAFE_RELEASE( pOldSurface );
	SAFE_RELEASE( pNewSurface );
	SAFE_RELEASE( pOldTexture );

	ASSERT( hResult == D3D_OK );
	return hResult == D3D_OK;
}

bool CEtTexture::ChangeFormatSize( EtFormat Format, EtUsage Usage, EtPool Pool , int nWidth, int nHeight )
{
	EtTexture *pOldTexture;

	SAFE_RELEASE( m_pSurface );
	pOldTexture = ( EtTexture * )m_pTexture;
	if( nWidth > 0 ) m_nWidth = nWidth;
	if( nHeight > 0 ) m_nHeight = nHeight;
	CreateTexture( m_nWidth, m_nHeight, Format, Usage, Pool );
	if( m_pTexture == NULL )
	{
		m_pTexture = pOldTexture;
		return false;
	}

	EtSurface *pOldSurface, *pNewSurface;
	pOldTexture->GetSurfaceLevel( 0, &pOldSurface );
	ADD_D3D_RES( pOldSurface );
	( ( EtTexture * )m_pTexture )->GetSurfaceLevel( 0, &pNewSurface );
	ADD_D3D_RES( pNewSurface );
	HRESULT hResult = D3DXLoadSurfaceFromSurface( pNewSurface, NULL, NULL, pOldSurface, NULL, NULL, D3DX_DEFAULT, 0 );
	SAFE_RELEASE( pOldSurface );
	SAFE_RELEASE( pNewSurface );
	SAFE_RELEASE( pOldTexture );

	ASSERT( hResult == D3D_OK );
	return hResult == D3D_OK;
}

void CEtTexture::Copy( EtTextureHandle hSour, DWORD dwFilter, RECT *pDestRect, RECT *pSrcRect )
{
	if( ( !hSour ) || ( m_pTexture == NULL ) )
	{
		return;
	}

	EtSurface *pSourSurface, *pTargetSurface;

	pTargetSurface = GetSurfaceLevel();
	pSourSurface = hSour->GetSurfaceLevel();
	D3DXLoadSurfaceFromSurface( pTargetSurface, NULL, pDestRect, pSourSurface, NULL, pSrcRect, dwFilter, 0 );
}

EtTextureHandle CEtTexture::CopyEx()
{
	//CEtTexture * pTexture = new CEtTexture();
	//pTexture->CreateTexture( m_nWidth, m_nHeight, m_Format, m_Usage, m_Pool, m_nLockLevel );
	//pTexture->AddRef();

	//EtSurface *pSourSurface, *pTargetSurface;
	//
	//pSourSurface = GetSurfaceLevel();
	//pTargetSurface = pTexture->GetSurfaceLevel();
	//
	//D3DXLoadSurfaceFromSurface( pTargetSurface, NULL, NULL, pSourSurface, NULL, NULL, D3DX_DEFAULT, 0 );

	////pTargetSurface->Release();
	////pSourSurface->Release();

	//return pTexture->GetMySmartPtr();

	EtTextureHandle newTexture = CreateNormalTexture( m_nWidth, m_nHeight );
	
	EtSurface *pSourSurface, *pTargetSurface;

	pTargetSurface = newTexture->GetSurfaceLevel();
	pSourSurface = GetSurfaceLevel();
	D3DXLoadSurfaceFromSurface( pTargetSurface, NULL, NULL, pSourSurface, NULL, NULL, D3DX_DEFAULT, 0 );

	return newTexture;
}

void CEtTexture::RemoveRenderTarget()
{
	EtTexture *pOldTexture;

	pOldTexture = ( EtTexture * )m_pTexture;

	LPDIRECT3DSURFACE9 pSysMemSurface;
	EtBaseTexture *pSysMemTexture = GetEtDevice()->CreateTexture( m_nWidth, m_nHeight, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_SYSTEMMEM );
	((LPDIRECT3DTEXTURE9)pSysMemTexture)->GetSurfaceLevel(0, &pSysMemSurface);
	HRESULT hr = ((IDirect3DDevice9*)GetEtDevice()->GetDevicePtr())->GetRenderTargetData( m_pSurface, pSysMemSurface);
	if( hr == D3DERR_DEVICELOST ) {
		SAFE_RELEASE( pSysMemTexture );
		pSysMemSurface->Release();	
		return;
	}
	SAFE_RELEASE( m_pSurface );
	SAFE_RELEASE( pOldTexture );
	
	CreateTexture( m_nWidth, m_nHeight, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_MANAGED);
	int nDestStride, nSrcStride;
	BYTE *DestPtr = (BYTE*)Lock( nDestStride );
	D3DLOCKED_RECT SrcRect;
	((LPDIRECT3DTEXTURE9)pSysMemTexture)->LockRect( 0, &SrcRect, 0, D3DLOCK_READONLY);
	BYTE *SrcPtr = (BYTE*)SrcRect.pBits;
	nSrcStride = SrcRect.Pitch;
	for( int i = 0; i < m_nHeight; i++) {
		memcpy( DestPtr + i * nDestStride, SrcPtr + i * nSrcStride, nSrcStride);
	}

	((LPDIRECT3DTEXTURE9)pSysMemTexture)->UnlockRect( 0 );
	Unlock();

	
	SAFE_RELEASE( pSysMemTexture );
	pSysMemSurface->Release();

	m_TextureType = ETTEXTURE_NORMAL;
}

void CEtTexture::BuildMipmap()
{
	EtTexture *pOldTexture;

	SAFE_RELEASE( m_pSurface );
	pOldTexture = ( EtTexture * )m_pTexture;
	CreateTexture( m_nWidth, m_nHeight, m_Format, USAGE_DEFAULT, POOL_MANAGED, 0);	

	EtSurface *pOldSurface, *pNewSurface;
	pOldTexture->GetSurfaceLevel( 0, &pOldSurface );
	ADD_D3D_RES( pOldSurface );

	int i, nLevelCount;
	nLevelCount = m_pTexture->GetLevelCount();

	for( i = 0; i < nLevelCount; i++) {
		( ( EtTexture * )m_pTexture )->GetSurfaceLevel( i, &pNewSurface );	
		ADD_D3D_RES( pNewSurface );
		D3DXLoadSurfaceFromSurface( pNewSurface, NULL, NULL, pOldSurface, NULL, NULL, D3DX_DEFAULT, 0 );
		SAFE_RELEASE( pNewSurface );
	}
	
	SAFE_RELEASE( pOldSurface );	
	SAFE_RELEASE( pOldTexture );
}

void CEtTexture::OnLostDevice()
{
	if( m_pSurface )
	{
		SAFE_RELEASE( m_pSurface );
	}
	if( m_TextureType == ETTEXTURE_RENDER_TARGET )
	{
 		SAFE_RELEASE( m_pTexture );
	}
}

void CEtTexture::OnResetDevice()
{
	if( m_TextureType == ETTEXTURE_RENDER_TARGET )
	{
		CreateRenderTarget( m_nWidth, m_nHeight, m_Format, m_Usage, m_Pool );
	}
}

void CEtTexture::CalcFittingArea( BYTE cAlphaThreshold, int *pX, int *pY, int *pWidth, int *pHeight )
{
	D3DSURFACE_DESC Desc;
	((EtTexture*)m_pTexture)->GetLevelDesc(0, &Desc);

	BYTE *pAlphaBuffer = new BYTE[ Desc.Width * Desc.Height ];
	memset(pAlphaBuffer, 0, Desc.Width * Desc.Height);

	if( Desc.Format == D3DFMT_DXT5 ) {
		D3DLOCKED_RECT Rect;
		( ( EtTexture * )m_pTexture )->LockRect( 0, &Rect, NULL, D3DLOCK_READONLY );

		BYTE		*pPtr = (BYTE*)Rect.pBits;

		for (int y = 0; y < (int)Desc.Height; y += 4) {
			for (int x = 0; x < (int)Desc.Width; x += 4) {
				if (y >= (int)Desc.Height || x >= (int)Desc.Width)
					break;

				BYTE		alphas[8];

				alphas[0] = pPtr[0];
				alphas[1] = pPtr[1];
				BYTE *alphamask = pPtr + 2;
				pPtr += 16;
				
				// 8-alpha or 6-alpha block?    
				if (alphas[0] > alphas[1]) {
					// 8-alpha block:  derive the other six alphas.
					// Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
					alphas[2] = (6 * alphas[0] + 1 * alphas[1] + 3) / 7;	// bit code 010
					alphas[3] = (5 * alphas[0] + 2 * alphas[1] + 3) / 7;	// bit code 011
					alphas[4] = (4 * alphas[0] + 3 * alphas[1] + 3) / 7;	// bit code 100
					alphas[5] = (3 * alphas[0] + 4 * alphas[1] + 3) / 7;	// bit code 101
					alphas[6] = (2 * alphas[0] + 5 * alphas[1] + 3) / 7;	// bit code 110
					alphas[7] = (1 * alphas[0] + 6 * alphas[1] + 3) / 7;	// bit code 111
				}
				else {
					// 6-alpha block.
					// Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
					alphas[2] = (4 * alphas[0] + 1 * alphas[1] + 2) / 5;	// Bit code 010
					alphas[3] = (3 * alphas[0] + 2 * alphas[1] + 2) / 5;	// Bit code 011
					alphas[4] = (2 * alphas[0] + 3 * alphas[1] + 2) / 5;	// Bit code 100
					alphas[5] = (1 * alphas[0] + 4 * alphas[1] + 2) / 5;	// Bit code 101
					alphas[6] = 0x00;										// Bit code 110
					alphas[7] = 0xFF;										// Bit code 111
				}

				// Note: Have to separate the next two loops,
				//	it operates on a 6-byte system.
				// First three bytes
				//bits = *((ILint*)alphamask);
				UINT		bits;
				bits = (alphamask[0]) | (alphamask[1] << 8) | (alphamask[2] << 16);
				for (int j = 0; j < 2; j++) {
					for (int i = 0; i < 4; i++) {
						// only put pixels out < width or height
						if (((x + i) < (int)Desc.Width) && ((y + j) < (int)Desc.Height)) {
							int Offset = (y + j) * Desc.Width + (x + i);
							pAlphaBuffer[ Offset ] = alphas[bits & 0x07];	
						}
						bits >>= 3;
					}
				}
				// Last three bytes
				//bits = *((ILint*)&alphamask[3]);
				bits = (alphamask[3]) | (alphamask[4] << 8) | (alphamask[5] << 16);
				for (int j = 2; j < 4; j++) {
					for (int i = 0; i < 4; i++) {
						// only put pixels out < width or height
						if (((x + i) < (int)Desc.Width) && ((y + j) < (int)Desc.Height)) {
							int Offset = (y + j) * Desc.Width + (x + i);
							pAlphaBuffer[ Offset ] = alphas[bits & 0x07];
						}
						bits >>= 3;
					}
				}
			}
		}
		( ( EtTexture * )m_pTexture )->UnlockRect( 0 );
	}
	else if ( Desc.Format == D3DFMT_A8R8G8B8 ) {
		D3DLOCKED_RECT Rect;
		( ( EtTexture * )m_pTexture )->LockRect( 0, &Rect, NULL, D3DLOCK_READONLY );
		BYTE		*pPtr = (BYTE*)Rect.pBits;
		for (int y = 0; y < (int)Desc.Height; y++)
		{
			for (int x = 0; x < (int)Desc.Width; x ++)
			{
				pAlphaBuffer[ x + y * Desc.Width ] = pPtr[ x * 4 + 3 + y * Rect.Pitch ];
			}
		}
		( ( EtTexture * )m_pTexture )->UnlockRect( 0 );
	}
	else {
		assert( false );
		delete [] pAlphaBuffer;
		return;
	}

	int nBlankCount;

	for( nBlankCount = 0; nBlankCount < (int)Desc.Width; nBlankCount++) {
		int i;
		for( i = 0; i < (int)Desc.Height; i++) {
			if( pAlphaBuffer[ nBlankCount + i * Desc.Width] > cAlphaThreshold ) break;
		}
		if( i != Desc.Height ) break;
	}
	*pX = nBlankCount;


	for( nBlankCount = 0; nBlankCount < (int)Desc.Width; nBlankCount++) {
		int i;
		for( i = 0; i < (int)Desc.Height; i++) {
			if( pAlphaBuffer[ (Desc.Width-nBlankCount-1) + i * Desc.Width] > cAlphaThreshold ) break;
		}
		if( i != Desc.Height ) break;
	}
	*pWidth = (Desc.Width-nBlankCount) - *pX;

	for( nBlankCount = 0; nBlankCount < (int)Desc.Height; nBlankCount++) {
		int i;
		for( i = 0; i < (int)Desc.Width; i++) {
			if( pAlphaBuffer[ i + nBlankCount * Desc.Width] > cAlphaThreshold ) break;
		}
		if( i != Desc.Width ) break;
	}
	*pY = nBlankCount;

	for( nBlankCount = 0; nBlankCount < (int)Desc.Height; nBlankCount++) {
		int i;
		for( i = 0; i < (int)Desc.Width; i++) {
			if( pAlphaBuffer[ i + (Desc.Height-nBlankCount-1) * Desc.Width] > cAlphaThreshold ) break;
		}
		if( i != Desc.Height ) break;
	}
	*pHeight = (Desc.Height-nBlankCount) - *pY;

	delete [] pAlphaBuffer;
}

void CEtTexture::FindAlphaBlock()
{
	D3DSURFACE_DESC Desc;
	((EtTexture*)m_pTexture)->GetLevelDesc(0, &Desc);
	if( Desc.Format == D3DFMT_DXT1 ) {
		D3DLOCKED_RECT Rect;
		HRESULT hr = ( ( EtTexture * )m_pTexture )->LockRect( 0, &Rect, NULL, D3DLOCK_READONLY );
		if( !FAILED(hr) )  {
			for( int y = 0; y < (int)Desc.Height/4; y++) {
				for( int x = 0; x < (int)Desc.Width*2; x+=8) {
					unsigned short *pData = (unsigned short*)(((BYTE*)Rect.pBits) + y * Rect.Pitch + x);
					if ( pData[0] < pData[1] ) {
						for( int j =2;j<=3;j++) {
							unsigned short dwVal = pData[j];
							for( int i=0;i<8;i++) {
								if((dwVal&3)==3) {
									( ( EtTexture * )m_pTexture )->UnlockRect( 0 );
									m_nAlphaTexture = 1;
									return;
								}
								dwVal >>= 2;
							}
						}
					}
				}
			}
		}
		( ( EtTexture * )m_pTexture )->UnlockRect( 0 );
		m_nAlphaTexture = 0;
	}	
	else {
		m_nAlphaTexture = 1;
	}
}

bool CEtTexture::IsAlphaTexture()
{
	if( m_TextureType == ETTEXTURE_VOLUME ) {
		return true;
	}

	if( m_nAlphaTexture == -1 ) {
		FindAlphaBlock();
	}
	return (m_nAlphaTexture != 0) ? true : false;
}

unsigned int CEtTexture::CalcTextureSize( unsigned int nNonPow2Size )
{
	/*ASSERT( size != 0 );
	unsigned int used = (size-1) | 0xf;
	used |= used >> 1;
	used |= used >> 2;
	used |= used >> 4;
	used |= used >> 8;
	used += 1;*/
	unsigned int rsize = 16;
	while(nNonPow2Size > rsize)rsize<<=1;
	/*ASSERT( used == rsize );*/
	return rsize;
}

void CEtTexture::CreateDebugRes()
{
	int nStride=0;
	m_hWhiteTexture = CreateNormalTexture( 1, 1, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_MANAGED );
	DWORD *pPtr = (DWORD *)m_hWhiteTexture->Lock( nStride, true);			
	*pPtr = 0xFFFFFFFF;
	m_hWhiteTexture->Unlock();

	m_hBlankTexture = CreateNormalTexture( 1, 1, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_MANAGED );
	pPtr = (DWORD *)m_hBlankTexture->Lock( nStride, true);			
	*pPtr = 0x0;
	m_hBlankTexture->Unlock();
}

void CEtTexture::ReleaseDebugRes()
{
	SAFE_RELEASE_SPTR( m_hWhiteTexture );
	SAFE_RELEASE_SPTR( m_hBlankTexture );
}