#include "StdAfx.h"
#include "DnTitleTask.h"
#include "DnInterfaceCamera.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnInterface.h"
#include "DnMainFrame.h"
#include "EtPrimitiveDraw.h"
#include "DnLoadingTask.h"
#include "DnServiceSetup.h"
#include "GameOption.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

char *CDnTitleTask::s_szMovieName[] = {
	"Publisher.avi",
	"Eyedentity.avi",
	"OpeningTitle.avi",
};

CDnTitleTask::CDnTitleTask()
: CInputReceiver( true )
, m_eTitlePhase( Phase_Loading )
, m_bStartedLoading( false )
, m_nLogoStep( -1 )
, m_bPlayMovie( false )
, m_bFinishPreoadLogin( false )
, m_pLoginTask( NULL )
, m_nHoundLoading( 0 )
, m_nForceAddHoundLoadingValue( 0 )
, m_dwForceAddTime( 0 )
, m_fForceAddTimeDelta( 0.0f )
, m_dwPreloadFinishTick( 0 )
{
}

CDnTitleTask::~CDnTitleTask()
{
	SAFE_RELEASE_SPTR( m_hCamera );
	GetInterface().Finalize( CDnInterface::Title );
}

bool CDnTitleTask::Initialize()
{
	m_hCamera = (new CDnInterfaceCamera)->GetMySmartPtr();
	CDnCamera::SetActiveCamera( m_hCamera );

	m_pBVLEngine.reset(new CEtBVLEngine);
	if (m_pBVLEngine->Initialize( CDnMainFrame::GetInstance().GetHWnd(), &CEtDevice::GetInstance() ) == false)
		m_pBVLEngine.reset();

	if( CDnLoadingTask::IsActive() ) {
		CDnLoadingTask::GetInstance().EnableBackgroundLoading( true );
	}

	if ( g_pServiceSetup )
		g_pServiceSetup->OnEvent(1, "WiseLogEvent");

	return true;
}

void CDnTitleTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnMouseCursor::GetInstance().ShowCursor( CGameOption::GetInstance().m_bWindow, true );
	int nCount = sizeof(s_szMovieName) / sizeof(char *);

#ifndef _FINAL_BUILD
	static DWORD dwTest1;
	static DWORD dwTest2;
	if( GetAsyncKeyState( VK_LSHIFT ) )
	{
		CGlobalValue::GetInstance().m_bMessageBoxWithMsgID = true;
	}
#endif

	if( m_eTitlePhase == Phase_Loading ) {
		if( !m_bStartedLoading ) {
			m_bStartedLoading = true;
			m_dwPreloadFinishTick = 0;
#ifndef _FINAL_BUILD
			dwTest1 = GetTickCount();
#endif
			if( m_pBVLEngine ) {
				m_pBVLEngine->Close();
				if( m_pBVLEngine->Open( CEtResourceMng::GetInstance().GetFullName( "TitleLoading.avi" ).c_str() ) ) {
					m_pBVLEngine->Play( true );
					m_bPlayMovie = true;
				}
			}
			if( CDnLoadingTask::IsActive() ) {
				CDnLoadingTask::GetInstance().InsertLoadObject( OnLoadLoginTasktCallback, NULL, NULL, NULL, (void*)this, NULL, 0, 0 );
			}
		}

		ProcessHoundLoading( LocalTime, fDelta );
		if( m_pBVLEngine ) m_pBVLEngine->Process( LocalTime, fDelta, m_nHoundLoading );

		if( m_bFinishPreoadLogin ) {
			if( m_dwPreloadFinishTick == 0 ) {
				CheckPointHoundLoading( 100 );
				m_dwPreloadFinishTick = GetTickCount() | 1;
				return;
			}
			if( GetTickCount() - m_dwPreloadFinishTick < 250 ) {
				return;
			}

			if( m_pBVLEngine ) {
				m_pBVLEngine->Stop();
				m_pBVLEngine->Close();
			}
			m_bPlayMovie = false;
			m_nLogoStep = -1;
			m_eTitlePhase = Phase_IntroMovies;
		}
		return;
	}

	if( m_bPlayMovie && m_nLogoStep >= 0 && m_nLogoStep < nCount && _stricmp( s_szMovieName[m_nLogoStep], "OpeningTitle.avi" ) == 0 ) {
		if( GetFocus() == CDnMainFrame::GetInstance().GetHWnd() && ( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 ) ) {
			if( m_pBVLEngine ) {
				m_pBVLEngine->Stop();
			}
			m_bPlayMovie = false;
		}
	}

	if( !m_bPlayMovie ) {
		if( m_nLogoStep < nCount - 1 ) {
			m_nLogoStep++;
			if( m_pBVLEngine ) {
				m_pBVLEngine->Close();
				if( m_pBVLEngine->Open( CEtResourceMng::GetInstance().GetFullName( s_szMovieName[m_nLogoStep] ).c_str() ) ) {
					m_pBVLEngine->Play();
					m_bPlayMovie = true;
				}
			}
		}
		else {
#ifndef _FINAL_BUILD
			dwTest2 = GetTickCount();
			dwTest1 = dwTest2 - dwTest1;
			char szTemp[128] = {0,};
			sprintf_s( szTemp, _countof(szTemp), "\n=============================\nTitle Loading Time = %d\n", dwTest1 );
			OutputDebug( szTemp );
#endif

			if( m_pLoginTask && m_pLoginTask->Initialize( CDnLoginTask::Logo ) == false ) {
				SAFE_DELETE( m_pLoginTask );
				OutputDebug( "Error - Can't Initialize Login Task\n" );
			}
			else if( m_pLoginTask ) {
				CTaskManager::GetInstance().AddTask( m_pLoginTask, "LoginTask", -1, false );
			}

//#ifndef _FINAL_BUILD
			if( CGlobalInfo::GetInstance().m_bUITest ) {
				// PreloadLogin (garbled mojibake)
				GetInterface()._TestUIDump( this );
				GetInterface().MessageBox( L"UI test complete! Please check uitest.txt.", MB_OK, MESSAGEBOX_10, CDnBridgeTask::GetInstancePtr(), true, false, false, true );
			}
//#endif	// #ifndef _FINAL_BUILD

			DestroyTask( true );
			return;
		}
	}
	else {
		m_bPlayMovie = ( m_pBVLEngine ) ? m_pBVLEngine->IsPlaying() : false;
	}

	if( m_pBVLEngine ) m_pBVLEngine->Process( LocalTime, fDelta, -1 );
}

bool __stdcall CDnTitleTask::OnLoadLoginTasktCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime )
{
	CDnTitleTask *pTask = (CDnTitleTask *)pThis;
	pTask->PreLoadLoginTask();
	return true;
}

void CDnTitleTask::PreLoadLoginTask()
{
	EtInterface::g_bEtUIMsgProc = false;
	if( CDnMainFrame::GetInstance().ThreadInitialize() == false ) { //
		_exit(0);
		return;
	}
	m_pLoginTask = new CDnLoginTask;
	m_pLoginTask->SetTaskName( "LoginTask" );	// Force-insert the task name in advance so DnInterface remembers it.
	m_pLoginTask->PreInitialize();
	CheckPointHoundLoading( 100 );
	m_bFinishPreoadLogin = true;

#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
	CDnMainFrame::GetInstance().ThreadInitializePreloadPlayer();
#endif

	EtInterface::g_bEtUIMsgProc = true;
}


void CDnTitleTask::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( nReceiverState & IR_KEY_DOWN ) {
		if( IsPushKey( DIK_ESCAPE ) ) {
			int nCount = sizeof(s_szMovieName) / sizeof(char *);
			if( m_eTitlePhase == Phase_IntroMovies && m_bPlayMovie && m_nLogoStep >= 0 && m_nLogoStep < nCount && _stricmp( s_szMovieName[m_nLogoStep], "OpeningTitle.avi" ) == 0 ) {
				if( m_pBVLEngine ) {
					m_pBVLEngine->Stop();
				}
				m_bPlayMovie = false;
			}
		}
	}
}

bool CDnTitleTask::RenderScreen( LOCAL_TIME LocalTime, float fDelta )
{
	return !m_bFinishPreoadLogin;
}

void CDnTitleTask::CheckPointHoundLoading( int nValue )
{
	//
	//
	if( m_nHoundLoading < nValue )
		m_nHoundLoading = nValue;

	m_nForceAddHoundLoadingValue = 0;
	m_dwForceAddTime = 0;
}

void CDnTitleTask::ForceAddHoundLoading( int nValue, DWORD dwTime )
{
	//
	m_nForceAddHoundLoadingValue = nValue;
	m_dwForceAddTime = dwTime / nValue;
	m_fForceAddTimeDelta = m_dwForceAddTime / 1000.0f;
}

void CDnTitleTask::ProcessHoundLoading( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_nForceAddHoundLoadingValue > 0 )
	{
		m_fForceAddTimeDelta -= fDelta;
		if( m_fForceAddTimeDelta < 0.0f )
		{
			m_nHoundLoading++;
			if( m_nHoundLoading > 99 )
				m_nHoundLoading = 99;
			m_nForceAddHoundLoadingValue--;

			if( m_nForceAddHoundLoadingValue > 0 )
				m_fForceAddTimeDelta += m_dwForceAddTime / 1000.0f;
		}
	}
}