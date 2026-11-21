#include "StdAfx.h"
#include "EtBCLEngine.h"
#include <Shlobj.h>
#include <shlwapi.h>
#include <time.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


void (__stdcall *CEtBCLEngine::s_pCaptureInfoCallback)() = NULL;

static BCAP_PRESET MapCaptureOptionToPreset( CEtBCLEngine::CaptureOptionEnum Option )
{
	switch( Option ) {
		case CEtBCLEngine::Default:       return BCAP_PRESET_DEFAULT;
		case CEtBCLEngine::HalfSize:      return BCAP_PRESET_HALFSIZE;
		case CEtBCLEngine::Res_320x240:   return BCAP_PRESET_320x240;
		case CEtBCLEngine::Res_400x300:   return BCAP_PRESET_400x300;
		case CEtBCLEngine::Res_512x384:   return BCAP_PRESET_512x384;
		case CEtBCLEngine::Res_576x432:   return BCAP_PRESET_576x432;
		case CEtBCLEngine::Res_640x480:   return BCAP_PRESET_640x480;
		case CEtBCLEngine::Res_800x600:   return BCAP_PRESET_800x600;
		case CEtBCLEngine::Mpeg1:         return BCAP_PRESET_MPEG1;
		case CEtBCLEngine::MJpeg:         return BCAP_PRESET_MJPEG;
		case CEtBCLEngine::MJpeg_HighQ:   return BCAP_PRESET_MJPEG_HIGH_QUALITY;
		case CEtBCLEngine::Mpeg4:         return BCAP_PRESET_MPEG4;
		case CEtBCLEngine::Youtube:       return BCAP_PRESET_YOUTUBE;
		case CEtBCLEngine::Youtube_HighQ: return BCAP_PRESET_YOUTUBE_HIGH_QUALITY;
		case CEtBCLEngine::Youtube_HighD: return BCAP_PRESET_YOUTUBE_HIGH_DEFINITION;
		case CEtBCLEngine::Naver:         return BCAP_PRESET_NAVER_BLOG;
		case CEtBCLEngine::Daum:          return BCAP_PRESET_DAUM_TVPOT;
		case CEtBCLEngine::MnCast:
		default:                          return BCAP_PRESET_DEFAULT;
	}
}

CEtBCLEngine::CEtBCLEngine()
{
	m_pBCL = NULL;
	m_pEtDevice = NULL;
	m_hWnd = NULL;
	m_szDefaultPath = L".";
	m_pPrevPrePresentCallback = NULL;

	m_Config.Default();
}

CEtBCLEngine::~CEtBCLEngine()
{
	Finalize();
}


bool CEtBCLEngine::Initialize( HWND hWnd, CEtDevice *pDev, WCHAR *wszDefaultPath )
{
	Finalize();

	m_pBCL = new CBandiCaptureLibrary;

	if( FAILED( m_pBCL->Create( BANDICAP_RELEASE_DLL_FILE_NAME ) ) ) {
		SAFE_DELETE( m_pBCL );
		return false;
	}
	if( m_pBCL->GetVersion() < BCAP_VERSION ) {
		m_pBCL->Destroy();
		SAFE_DELETE( m_pBCL );
		return false;
	}

	m_pBCL->Verify( "EYEDENTITYGAMES_DRAGONNEST_20091102", "e740e5a4" );
	m_pEtDevice = pDev;
	m_hWnd = hWnd;

	if( wszDefaultPath ) {
		m_szDefaultPath = wszDefaultPath;
	}

	return true;
}

void CEtBCLEngine::Finalize()
{
	if( m_pBCL == NULL ) return;
	if( m_pBCL->IsCapturing() ) {
		Stop();
	}
	m_pBCL->Destroy();
	SAFE_DELETE( m_pBCL );
}

void CEtBCLEngine::SetOption( CaptureOptionEnum Option )
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return;

	BCapConfigPreset( &m_Config, MapCaptureOptionToPreset( Option ) );
	m_Config.IncludeCursor = TRUE;
	m_pBCL->CheckConfig( &m_Config );
	m_pBCL->SetConfig( &m_Config );
}

void CEtBCLEngine::RefreshConfig()
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return;

	m_pBCL->CheckConfig( &m_Config );
	m_pBCL->SetConfig( &m_Config );
}

DWORD CEtBCLEngine::GetSuggestedVideoCodec()
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return FOURCC_H264;
	return m_pBCL->GetSuggestedVideoCodec();
}

bool CEtBCLEngine::IsSupportedVideoCodec( DWORD dwCodec )
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return false;
	return m_pBCL->IsSupportedVideoCodec( dwCodec ) == TRUE;
}

bool CEtBCLEngine::CaptureImage( LPCWSTR wszFilePath, int nFileType, int nQuality, bool bIncludeCursor )
{
	if( !m_pBCL || !m_pBCL->IsCreated() || !m_pEtDevice || !wszFilePath ) return false;
	HRESULT hr = m_pBCL->CaptureImage( wszFilePath, nFileType, nQuality, BCAP_MODE_D3D9,
		bIncludeCursor ? TRUE : FALSE, (LONG_PTR)m_pEtDevice->GetDevicePtr() );
	return SUCCEEDED( hr );
}

void CEtBCLEngine::Start( TCHAR *szFileName )
{
	if( !m_pBCL || !m_pBCL->IsCreated() || !m_pEtDevice ) return;

	TCHAR szResultName[MAX_PATH * 2] = { 0, };
	const WCHAR *wszExt = ( m_Config.FileType == BCAP_CONFIG::F_MP4 ) ? L"mp4" : L"avi";

	if( szFileName == NULL ) {
		__time64_t ltime;
		TCHAR szBuf[256] = { 0, };
		struct tm tm_ptr;
		time( &ltime );
		_localtime64_s( &tm_ptr, &ltime );

		wcsftime( szBuf, _countof(szBuf), L"%Y-%m-%d %H-%M-%S %a", &tm_ptr );

		swprintf_s( szResultName, _countof(szResultName), L"DN %s.%s", szBuf, wszExt );
		m_szSaveFileName = szResultName;
		swprintf_s( szResultName, _countof(szResultName), L"%s\\DN %s.%s", m_szDefaultPath.c_str(), szBuf, wszExt );
	}
	else {
		m_szSaveFileName = szFileName;
		if( szFileName[0] != L'\0' && ( szFileName[1] == L':' || ( szFileName[0] == L'\\' && szFileName[1] == L'\\' ) ) ) {
			wcsncpy_s( szResultName, _countof(szResultName), szFileName, _TRUNCATE );
		}
		else {
			swprintf_s( szResultName, _countof(szResultName), L"%s\\%s", m_szDefaultPath.c_str(), szFileName );
		}
	}

	m_pBCL->CheckConfig( &m_Config );
	m_pBCL->SetConfig( &m_Config );

	HRESULT hr = m_pBCL->Start( szResultName, m_hWnd, BCAP_MODE_D3D9_SCALE, (LONG_PTR)m_pEtDevice->GetDevicePtr() );
	if( hr == BCERR_AUDIO_CAPTURE ) {
		// Retry video-only capture if system audio loopback capture is unavailable.
		m_Config.AudioCodec = WAVETAG_NULL;
		m_pBCL->CheckConfig( &m_Config );
		m_pBCL->SetConfig( &m_Config );
		hr = m_pBCL->Start( szResultName, m_hWnd, BCAP_MODE_D3D9_SCALE, (LONG_PTR)m_pEtDevice->GetDevicePtr() );
	}

	if( FAILED( hr ) ) {
		CaptureErrorEnum ErrorCode = ErrorUnknown;
		switch( hr ) {
			case BCERR_VIDEO_CAPTURE:        ErrorCode = ErrorVideo; break;
			case BCERR_AUDIO_CAPTURE:        ErrorCode = ErrorAudio; break;
			case BCERR_FILE_CREATE:          ErrorCode = ErrorCreateFile; break;
			case BCERR_FILE_WRITE:           ErrorCode = ErrorWriteFile; break;
			case BCERR_NOT_ENOUGH_DISKSPACE: ErrorCode = ErrorNotEnoughDiskSpace; break;
		}
		m_pBCL->Stop();
		if( m_hWnd ) {
			SendMessage( m_hWnd, UM_CAPTURE_ERROR, ErrorCode, 0 );
		}
		return;
	}

	m_pPrevPrePresentCallback = ( void ( __stdcall * )() )m_pEtDevice->SetPrePresentCallback( PrePresentCallback );
}

void CEtBCLEngine::Pause( bool bPause )
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return;
	m_pBCL->Pause( bPause ? TRUE : FALSE );
}

void CEtBCLEngine::Stop()
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return;

	if( m_pEtDevice ) {
		m_pEtDevice->SetPrePresentCallback( m_pPrevPrePresentCallback );
	}
	m_pPrevPrePresentCallback = NULL;
	m_pBCL->Stop();
}

bool CEtBCLEngine::IsCapturing()
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return false;

	return ( m_pBCL->IsCapturing() == TRUE ) ? true : false;
}

int CEtBCLEngine::GetCaptureTime()
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return 0;

	return m_pBCL->GetCaptureTime();
}

INT64 CEtBCLEngine::GetCaptureFileSize()
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return 0;

	return m_pBCL->GetCaptureFileSize();
}

void CEtBCLEngine::SetPriority( int nPriority )
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return;

	m_pBCL->SetPriority( nPriority );
}

int CEtBCLEngine::GetPriority()
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return 0;

	return m_pBCL->GetPriority();
}

void CEtBCLEngine::SetMinMaxFPS( int nMin, int nMax )
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return;

	m_pBCL->SetMinMaxFPS( nMin, nMax );
}

void CEtBCLEngine::GetMinMaxFPS( int &nMin, int &nMax )
{
	if( !m_pBCL || !m_pBCL->IsCreated() ) return;

	m_pBCL->GetMinMaxFPS( &nMin, &nMax );
}

void __stdcall CEtBCLEngine::PrePresentCallback()
{
	if( CEtBCLEngine::GetInstance().m_pPrevPrePresentCallback )
	{
		CEtBCLEngine::GetInstance().m_pPrevPrePresentCallback();
	}
	if( CEtBCLEngine::IsActive() && CEtBCLEngine::GetInstance().IsCapturing() ) {
		CEtBCLEngine::GetInstance().Work();

		if( CEtBCLEngine::s_pCaptureInfoCallback ) CEtBCLEngine::s_pCaptureInfoCallback();
	}
}

void CEtBCLEngine::Work()
{
	if( !m_pBCL || !m_pBCL->IsCreated() || !m_pEtDevice ) return;

	HRESULT hResult = m_pBCL->Work( (LONG_PTR)m_pEtDevice->GetDevicePtr() );
	if( FAILED( hResult ) ) {
		CaptureErrorEnum ErrorCode = ErrorUnknown;
		switch( hResult ) {
			case BCERR_VIDEO_CAPTURE: ErrorCode = ErrorVideo; break;
			case BCERR_AUDIO_CAPTURE: ErrorCode = ErrorAudio; break;
			case BCERR_FILE_CREATE: ErrorCode = ErrorCreateFile; break;
			case BCERR_FILE_WRITE: ErrorCode = ErrorWriteFile; break;
			case BCERR_NOT_ENOUGH_DISKSPACE: ErrorCode = ErrorNotEnoughDiskSpace; break;
		}
		if( m_hWnd ) {
			SendMessage( m_hWnd, UM_CAPTURE_ERROR, ErrorCode, 0 );
		}
	}
}
