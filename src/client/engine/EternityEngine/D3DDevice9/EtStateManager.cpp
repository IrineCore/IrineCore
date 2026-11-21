#include "StdAfx.h"
#include "EtDevice.h"
#include "EtStateManager.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtStateBlockManager::CEtStateBlockManager() : m_pD3DDevice( NULL ) 
{
	m_bEnable = true;
	Reset();
}

CEtStateBlockManager::~CEtStateBlockManager() 
{
}

void CEtStateBlockManager::Initialize( CEtDevice *pDevice ) 
{
	m_pD3DDevice = (IDirect3DDevice9*)pDevice->GetDevicePtr();
}

int CEtStateBlockManager::CreateStateBlock( SStateBlock *StateBlock )
{
	int i, nSize;	
	nSize = (int)m_stateBlocks.size();
	for( i = 0; i < nSize; i++) {
		if( m_stateBlocks[ i ] == *StateBlock ) {
			return i;
		}
	}
	m_stateBlocks.push_back( *StateBlock );
	return (int)m_stateBlocks.size()-1;

}

//#define _DEBUG_VALID_STATE
void CEtStateBlockManager::SetDefaultState()
{
#if defined( _DEBUG_VALID_STATE )
	DWORD dwVal;
	for( int i = 1; i < RS_MAX; i++) {		
		if( i <= 6 ) continue;
		if( i >= 10 && i <= 13  ) continue;
		if( i == 17 || i == 18 || i == 21 || i == 30 || i == 31 || i == 32 || i == 33 || ( i >= 39 && i <= 47)  || (i>=49&&i<=51)  || ( i >= 61 && i <= 127) ) continue;
		if( i == 138 || i == 149 || i == 150 || i == 153 || i == 164 || i == 169 || i == 177 || i == 197 || i == 198  ) continue;
		m_pD3DDevice->GetRenderState( (D3DRENDERSTATETYPE)i, &dwVal);		
		SetRenderStateDefered( (D3DRENDERSTATETYPE)i, dwVal);		
	}
	for( int j = 1; j < STAGE_MAX; j++) {
		for( int i = 0; i < SS_MAX; i++) {			
			m_pD3DDevice->GetSamplerState( j, (D3DSAMPLERSTATETYPE)i, &dwVal);			
			SetSamplerStateDefered( j, (D3DSAMPLERSTATETYPE)i, dwVal);			
		}
		for( int i = 1; i < TS_MAX; i++) {			
			if( (i >= 12 && i <= 21) || i == 25 || i == 29 || i == 30 || i == 31 ) continue;
			m_pD3DDevice->GetTextureStageState( j, (D3DTEXTURESTAGESTATETYPE)i, &dwVal);
			SetTextureStageStateDefered( j, (D3DTEXTURESTAGESTATETYPE)i, dwVal);
		}
	}
#endif

	SetRenderStateDefered( D3DRS_ZENABLE, D3DZB_TRUE );	
	SetRenderStateDefered( D3DRS_ZWRITEENABLE, TRUE );	
	SetRenderStateDefered( D3DRS_CULLMODE, D3DCULL_CCW );
	SetRenderStateDefered( D3DRS_ALPHABLENDENABLE, FALSE );
	SetRenderStateDefered( D3DRS_ALPHAREF, ALPHA_TEST_VALUE );
	SetRenderStateDefered( D3DRS_ALPHATESTENABLE, TRUE );
	SetRenderStateDefered( D3DRS_SCISSORTESTENABLE, FALSE);
	
	SetRenderStateDefered( D3DRS_LIGHTING, FALSE );
	SetRenderStateDefered( D3DRS_COLORWRITEENABLE, 0xf );
	SetRenderStateDefered( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	SetRenderStateDefered( D3DRS_DEPTHBIAS, 0 );
	
	SetRenderStateDefered( D3DRS_ALPHAFUNC, D3DCMP_GREATER );

	SetRenderStateDefered( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	SetRenderStateDefered( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	SetRenderStateDefered( D3DRS_BLENDOP, D3DBLENDOP_ADD );

	// State For Sprite
	GetEtDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	GetEtDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	GetEtDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	GetEtDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	GetEtDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	GetEtDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE );

	GetEtDevice()->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY, 1 );

	for( int i = 0; i < STAGE_MAX; i++) {
		SetSamplerStateDefered( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		SetSamplerStateDefered( i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		SetSamplerStateDefered( i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

		SetSamplerStateDefered( i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
		SetSamplerStateDefered( i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	}
}

void CEtStateBlockManager::Reset() 
{
	for( int i = 0; i < RS_MAX; i++) {
		m_RenderState[ 0 ][ i ] = RESET_MAGIC_NUMBER;
		m_RenderState[ 1 ][ i ] = RESET_MAGIC_NUMBER;
		m_RenderState[ 2 ][ i ] = RESET_MAGIC_NUMBER;
		m_RenderState[ 3 ][ i ] = RESET_MAGIC_NUMBER;
	}

	for( int j = 0; j < STAGE_MAX; j++)
	{
		for( int i = 0; i < SS_MAX; i++)  {
			m_SamplerState[ 0 ][ j ][ i ] = RESET_MAGIC_NUMBER;
			m_SamplerState[ 1 ][ j ][ i ] = RESET_MAGIC_NUMBER;
			m_SamplerState[ 2 ][ j ][ i ] = RESET_MAGIC_NUMBER;
			m_SamplerState[ 3 ][ j ][ i ] = RESET_MAGIC_NUMBER;
		}
		for( int i = 0; i < TS_MAX; i++)  {
			m_TextureStageState[ 0 ][ j ][ i ] = RESET_MAGIC_NUMBER;
			m_TextureStageState[ 1 ][ j ][ i ] = RESET_MAGIC_NUMBER;
			m_TextureStageState[ 2 ][ j ][ i ] = RESET_MAGIC_NUMBER;
			m_TextureStageState[ 3 ][ j ][ i ] = RESET_MAGIC_NUMBER;
		}
	}

	for( int i = 0; i < RS_MAX; i++) {
		m_DirtyRenderFlag[ i ] = false;
	}
	for( int j = 0; j < STAGE_MAX; j++) {
		for( int i = 0; i < SS_MAX; i++) {
			m_DirtySamplerFlag[ j ][ i ] = false;
		}
		for( int i = 0; i < TS_MAX; i++) {
			m_DirtyTextureStageFlag[ j ][ i ] = false;
		}
	}

	m_nDirtyRenderCount = 0;
	for( int i = 0; i < STAGE_MAX; i++) {
		m_DirtySamplerCount[ i ] = 0;
		m_DirtyTextureStageCount[ i ] = 0;
		m_currTexture[ i ] = NULL;
	}

	m_currPixelShader = NULL;
	m_currVertexShader = NULL;		

#if defined( _CHECK_STATE_PERFORMANCE )	
	m_nStateSkip = 0;
	m_nStateTotal = 0;

	m_nShaderSkip = 0;
	m_nShaderTotal = 0;

	m_nTextureSkip = 0;
	m_nTextureTotal = 0;	
#endif			
}

void CEtStateBlockManager::GetRenderStateDefered( D3DRENDERSTATETYPE State, DWORD *pdwValue )
{
	if( m_bEnable ) {
		ASSERT( m_RenderState[ 0 ][ State ] != RESET_MAGIC_NUMBER );	
		*pdwValue = m_RenderState[ 0 ][ State ];
	}
	else {
		m_pD3DDevice->GetRenderState( State, pdwValue );
	}
}

void CEtStateBlockManager::SetRenderStateDefered( D3DRENDERSTATETYPE State, DWORD dwValue )
{
	if( m_bEnable ) {
		
		ASSERT( m_nDirtyRenderCount < DIRTY_MAX  );
		if( m_nDirtyRenderCount >= DIRTY_MAX  ) {
			return;
		}

		if( m_DirtyRenderFlag[ State ] == false ) {
			m_DirtyRenderFlag[ State ] = true;
			m_DirtyRenderState[ m_nDirtyRenderCount++ ] = State;
		}
		m_RenderState[ 0 ][ State ] = dwValue;
	}
	else {
		m_pD3DDevice->SetRenderState( State, dwValue );
	}
}

void CEtStateBlockManager::GetSamplerStateDefered( DWORD Sampler, D3DSAMPLERSTATETYPE State, DWORD *pdwValue )
{
	if( m_bEnable ) {
		//ASSERT( Sampler <= 5 );
		//ASSERT( m_SamplerState[ 0 ][ Sampler ][ State ] != RESET_MAGIC_NUMBER );	
		*pdwValue = m_SamplerState[ 0 ][ Sampler ][ State ];
	}
	else {
		m_pD3DDevice->GetSamplerState( Sampler, State, pdwValue );
	}
}

void CEtStateBlockManager::SetSamplerStateDefered( DWORD Sampler, D3DSAMPLERSTATETYPE State, DWORD Value )
{
	if( m_bEnable )
	{
		if( Sampler >= STAGE_MAX || m_DirtySamplerCount[ Sampler ] >= DIRTY_MAX )
		{
			ASSERT( Sampler < STAGE_MAX );
			ASSERT( m_DirtySamplerCount[ Sampler ] < DIRTY_MAX );
			return;
		}

		if( m_DirtySamplerFlag[ Sampler ][ State ] == false  ) {
			m_DirtySamplerFlag[ Sampler ][ State ] = true;
			m_DirtySamplerState[ Sampler ][ m_DirtySamplerCount[ Sampler ]++ ] = State;
		}
		m_SamplerState[ 0 ][ Sampler ][ State ] = Value;	
	}
	else {
		m_pD3DDevice->SetSamplerState( Sampler, State, Value );
	} 
}

void CEtStateBlockManager::GetTextureStageStateDefered( DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pdwValue)
{
	if( m_bEnable ) {
		//ASSERT( Stage < STAGE_MAX );
		//ASSERT( m_TextureStageState[ 0 ][ Stage ][ Type ] != RESET_MAGIC_NUMBER );
		*pdwValue = m_TextureStageState[ 0 ][ Stage ][ Type ];
	}
	else {
		m_pD3DDevice->GetTextureStageState( Stage, Type, pdwValue );
	}
}

void CEtStateBlockManager::SetTextureStageStateDefered( DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	if( m_bEnable )
	{
		if( Stage >= STAGE_MAX || m_DirtyTextureStageCount[ Stage ] >= DIRTY_MAX )
		{
			ASSERT( Stage < STAGE_MAX );
			ASSERT( m_DirtyTextureStageCount[ Stage ] < DIRTY_MAX );
			return;
		}

		if( m_DirtyTextureStageFlag[ Stage ][ Type ] == false  ) {
			m_DirtyTextureStageFlag[ Stage ][ Type ] = true;
			m_DirtyTextureStageState[ Stage ][ m_DirtyTextureStageCount[ Stage ]++ ] = Type;
		}
		m_TextureStageState[ 0 ][ Stage ][ Type ] = Value;
	}
	else {
		m_pD3DDevice->SetTextureStageState( Stage, Type, Value );
	}
}

void CEtStateBlockManager::FlushDeferedStates()
{
#if defined( _CHECK_STATE_PERFORMANCE )
	static bool bPressPrev = false;
	bool bPress = GetAsyncKeyState(VK_HOME)<0;
	if( !bPressPrev && bPress ) {
		SetEnable( !IsEnable() );
		OutputDebug("RenderState Optimize %s\n", IsEnable() ? "Enable" : "Disable");
	}
	bPressPrev = bPress;
#endif

	if( !m_bEnable ) {
		return;
	}

#if defined( _DEBUG_VALID_STATE )
	for( int i = 1; i < RS_MAX; i++) {		
		if( m_RenderState[ 1 ][ i ] != RESET_MAGIC_NUMBER ) 
		{
			DWORD dwVal=0;
			m_pD3DDevice->GetRenderState( (D3DRENDERSTATETYPE)i, &dwVal);
			ASSERT( m_RenderState[ 1 ][ i ] == dwVal );
		}		
	}
	for( int j = 0; j < STAGE_MAX; j++) {
		for( int i = 1; i < SS_MAX; i++) {
			if( RESET_MAGIC_NUMBER != m_SamplerState[ 1 ][ j ][ i ] ) {
				DWORD dwVal=0;
				m_pD3DDevice->GetSamplerState( j, (D3DSAMPLERSTATETYPE)i, &dwVal);
				ASSERT( m_SamplerState[ 1 ][ j ][ i ] == dwVal );
			}
		}
		for( int i = 1; i < TS_MAX; i++) {
			if( m_TextureStageState[ 1 ][ j ][ i ] != RESET_MAGIC_NUMBER ) {
				DWORD dwVal=0;
				m_pD3DDevice->GetTextureStageState( j, (D3DTEXTURESTAGESTATETYPE)i, &dwVal);
				ASSERT(  m_TextureStageState[ 1 ][ j ][ i ] == dwVal );
			}
		}
	}
#endif


#if defined( _CHECK_STATE_PERFORMANCE )
	m_nStateTotal += m_nDirtyRenderCount;
#endif
	for( int i = 0; i < m_nDirtyRenderCount; i++) {
		D3DRENDERSTATETYPE State = (D3DRENDERSTATETYPE)m_DirtyRenderState[ i ];
		m_DirtyRenderFlag[ State ] = false;

		if( m_RenderState[ 0 ][ State ] != m_RenderState[ 1 ][ State ] ) {
			m_pD3DDevice->SetRenderState( State, m_RenderState[ 0 ][ State ] );
			m_RenderState[ 1 ][ State ] = m_RenderState[ 0 ][ State ];
		}
#if defined( _CHECK_STATE_PERFORMANCE )
		else {
			m_nStateSkip++;
		}
#endif
	}
	m_nDirtyRenderCount = 0;

	for( int j = 0; j < STAGE_MAX; j++) {
#if defined( _CHECK_STATE_PERFORMANCE )
		m_nStateTotal += m_DirtySamplerCount[ j ];
		m_nStateTotal += m_DirtyTextureStageCount[ j ];
#endif
		for( int i = 0; i < m_DirtySamplerCount[ j ]; i++) {			
			D3DSAMPLERSTATETYPE State = (D3DSAMPLERSTATETYPE)m_DirtySamplerState[ j ][ i ];
			m_DirtySamplerFlag[ j ][ State ] = false;

			if( m_SamplerState[ 0 ][ j ][ State ] != m_SamplerState[ 1 ][ j ][ State ] ) {
				m_pD3DDevice->SetSamplerState( j, State, m_SamplerState[ 0 ][ j ][ State ] );
				m_SamplerState[ 1 ][ j ][ State ] = m_SamplerState[ 0 ][ j ][ State ];
			}
#if defined( _CHECK_STATE_PERFORMANCE )
			else {
				m_nStateSkip++;
			}
#endif
		}
		m_DirtySamplerCount[ j ] = 0;
		for( int i = 0; i < m_DirtyTextureStageCount[ j ]; i++) {
			D3DTEXTURESTAGESTATETYPE State = (D3DTEXTURESTAGESTATETYPE)m_DirtyTextureStageState[ j ][ i ];
			m_DirtyTextureStageFlag[ j ][ State ] = false;

			if( m_TextureStageState[ 0 ][ j ][ State ] != m_TextureStageState[ 1 ][ j ][ State ] ) {
				m_pD3DDevice->SetTextureStageState( j, State, m_TextureStageState[ 0 ][ j ][ State ] );
				m_TextureStageState[ 1 ][ j ][ State ] = m_TextureStageState[ 0 ][ j ][ State ];
			}
#if defined( _CHECK_STATE_PERFORMANCE )
			else {
				m_nStateSkip++;
			}
#endif
		}
		m_DirtyTextureStageCount[ j ] = 0;
	}
}

void CEtStateBlockManager::Capture()
{
	for( int i = 0; i < RS_MAX; i++) {
		m_RenderState[ 2 ][ i ] = m_RenderState[ 0 ][ i ];
	}
	for( int j = 0; j < STAGE_MAX; j++) {
		for( int i = 0; i < SS_MAX; i++)  {
			m_SamplerState[ 2 ][ j ][ i ] = m_SamplerState[ 0 ][ j ][ i ];
		}
		for( int i = 0; i < TS_MAX; i++)  {
			m_TextureStageState[ 2 ][ j ][ i ] = m_TextureStageState[ 0 ][ j ][ i ];
		}
	}
}

void CEtStateBlockManager::Diff()
{
	for( int i = 0; i < RS_MAX; i++) {
		if( m_RenderState[ 3 ][ i ] != RESET_MAGIC_NUMBER && m_RenderState[ 3 ][ i ] != m_RenderState[ 0 ][ i ] )
		{
			int df = 3;
		}
		m_RenderState[ 3 ][ i ] = m_RenderState[ 0 ][ i ];
	}
	for( int j = 0; j < STAGE_MAX; j++) {
		for( int i = 0; i < SS_MAX; i++)  {

			if( m_SamplerState[ 3 ][ j ][ i ] != RESET_MAGIC_NUMBER && m_SamplerState[ 3 ][ j ][ i ] != m_SamplerState[ 0 ][ j ][ i ] )
			{
				int df = 3;
			}
			m_SamplerState[ 3 ][ j ][ i ] = m_SamplerState[ 0 ][ j ][ i ];

		}
		for( int i = 0; i < TS_MAX; i++)  {

			if( m_TextureStageState[ 3 ][ j ][ i ] != RESET_MAGIC_NUMBER && m_TextureStageState[ 3 ][ j ][ i ] != m_TextureStageState[ 0 ][ j ][ i ] )
			{
				int df = 3;
			}
			m_TextureStageState[ 3 ][ j ][ i ] = m_TextureStageState[ 0 ][ j ][ i ];
		}
	}
}

void CEtStateBlockManager::Restore()
{
	for( int i = 0; i < RS_MAX; i++) {
		if( m_RenderState[ 2 ][ i ] != m_RenderState[ 0 ][ i ] ) {
			ASSERT( m_RenderState[ 2 ][ i ] != RESET_MAGIC_NUMBER );
			SetRenderStateDefered( (D3DRENDERSTATETYPE) i, m_RenderState[ 2 ][ i ]);
		}
	}
	for( int j = 0; j < STAGE_MAX; j++) {
		for( int i = 0; i < SS_MAX; i++)  {
			if( m_SamplerState[ 2 ][ j ][ i ] != m_SamplerState[ 0 ][ j ][ i ] ) {
				ASSERT( m_SamplerState[ 2 ][ j ][ i ] != RESET_MAGIC_NUMBER );
				SetSamplerStateDefered( j, (D3DSAMPLERSTATETYPE)i, m_SamplerState[ 2 ][ j ][ i ]);
			}
		}
		for( int i = 0; i < TS_MAX; i++)  {
			if( m_TextureStageState[ 2 ][ j ][ i ] != m_TextureStageState[ 0 ][ j ][ i ] ) {
				ASSERT( m_TextureStageState[ 2 ][ j ][ i ] != RESET_MAGIC_NUMBER );
				SetTextureStageStateDefered( j, (D3DTEXTURESTAGESTATETYPE)i, m_TextureStageState[ 2 ][ j ][ i ]);
			}
		}
	}
}

void CEtStateBlockManager::DebugDumpState()
{
#if defined(_DEBUG) || defined(_RDEBUG)
	FILE *fp = fopen( "d:\\RenderState.txt", "wt");
	for( int i = 0; i < RS_MAX; i++) {
		fprintf(fp, "RS %d = %d\n", i, m_RenderState[ 0 ][ i ] );
	}
	for( int j = 0; j < STAGE_MAX; j++) {
		for( int i = 0; i < SS_MAX; i++) {
			fprintf(fp, "SS %d:%d = %d\n", j, i, m_SamplerState[ 0 ][ j ][ i ] );
		}
		for( int i = 0; i < TS_MAX; i++) {
			fprintf(fp, "TS %d:%d = %d\n", j, i, m_TextureStageState[ 0 ][ j ][ i ] );
		}
	}
	fclose(fp);
#endif
}

static CEtStateBlockManager s_EtStateManager;
CEtStateBlockManager* GetEtStateManager()
{
	return &s_EtStateManager;
}
