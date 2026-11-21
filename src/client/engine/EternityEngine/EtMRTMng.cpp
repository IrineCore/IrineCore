#include "StdAfx.h"
#include "EtMRTMng.h"
#include "EtOptionController.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtMRTMng::CEtMRTMng(void)
{
	m_bEnable = true;
	m_bInit = false;
	m_bClearNeeded = false;
	m_pBackupSurface = NULL;
}

CEtMRTMng::~CEtMRTMng(void)
{
	SAFE_RELEASE_SPTR( m_hDepthTarget );
	SAFE_RELEASE_SPTR( m_hVelocityTarget );
}

void CEtMRTMng::Initialize()
{
	SAFE_RELEASE_SPTR( m_hDepthTarget );
	m_hDepthTarget = CEtTexture::CreateRenderTargetTexture( -1, -1, FMT_R32F );
	
	if( GetEtOptionController()->IsEnableMotionBlur() )
	{
		EtFormat Format;

		Format = FMT_G16R16F;
		if( !GetEtDevice()->CheckDeviceFormat( Format, USAGE_RENDERTARGET ) )
		{
			Format = FMT_A16B16G16R16F;
			if( !GetEtDevice()->CheckDeviceFormat( Format, USAGE_RENDERTARGET ) )
			{
				Format = FMT_UNKNOWN;
			}
		}
		if( Format != FMT_UNKNOWN )
		{
			m_hVelocityTarget = CEtTexture::CreateRenderTargetTexture( -1, -1, Format );
		}
		else
		{
			SGraphicOption Option;
			GetEtOptionController()->GetGraphicOption( Option );
			Option.bEnableMotionBlur = false;
			GetEtOptionController()->SetGraphicOption( Option );
		}
	}
	m_bInit = true;
}

void CEtMRTMng::ClearDepthTarget()
{
	m_bClearNeeded = true;	
}

bool CEtMRTMng::BeginDepthTarget()
{
	if( !m_bEnable )
	{
		return true ;
	}
	if( !m_bInit )
	{
		Initialize();
	}

	m_pBackupSurface = GetEtDevice()->GetRenderTarget();
	GetEtDevice()->SetRenderTarget( m_hDepthTarget->GetSurfaceLevel() );	

	if( m_bClearNeeded ) {
		GetEtDevice()->ClearBuffer( 0x00000000, 1.0f, 0,  true, true, true );
		m_bClearNeeded = false;
	}
	return true;
}

void CEtMRTMng::EndDepthTarget()
{
	if( !m_bEnable )
	{
		return;
	}
	GetEtDevice()->SetRenderTarget( m_pBackupSurface );
}
