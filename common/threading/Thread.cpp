#include "stdafx.h"
#include "Thread.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

Thread::Thread(const char* szThreadNick)
: m_nThreadID( 0 ), m_hThreadHandle( INVALID_HANDLE_VALUE ), m_bRunning(true)
{
	strcpy_s( m_szThreadNick, _countof(m_szThreadNick), szThreadNick );
}

Thread::~Thread()
{
	m_bRunning = false;
	WaitForTerminate(100);

	if( m_hThreadHandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hThreadHandle );
		m_hThreadHandle = INVALID_HANDLE_VALUE;
	}
}

bool Thread::Start()
{
	if( m_hThreadHandle != INVALID_HANDLE_VALUE )
		return false;

	m_bRunning = true;

	m_hThreadHandle = (HANDLE)::_beginthreadex( 0, 0, _Runner, (void*)this, 0, &m_nThreadID );	
	if( m_hThreadHandle == 0 )
		return false;

	return true;
}

bool Thread::Terminate( DWORD nExitCode )
{
	if( m_hThreadHandle == INVALID_HANDLE_VALUE )
		return true;

#if defined(_WIN32)
	::_endthreadex(nExitCode);
#else
	(void)nExitCode;
	m_bRunning = false;
	WaitForTerminate(1000);
#endif

	CloseHandle( m_hThreadHandle );
	m_hThreadHandle	= INVALID_HANDLE_VALUE;
	return true;
}

bool Thread::WaitForTerminate( DWORD nTimeout )
{
	if( m_hThreadHandle == INVALID_HANDLE_VALUE )
		return false;

	return ( ::WaitForSingleObject( m_hThreadHandle, nTimeout ) == WAIT_OBJECT_0 );
}

DWORD_PTR Thread::SetThreadAffinityMask( DWORD dwMask )
{
	return ::SetThreadAffinityMask( m_hThreadHandle, dwMask );
}

unsigned int WINAPI Thread::_Runner( void * pParam )
{
	Thread* pInstance = static_cast<Thread*>( pParam );

	SetThreadName( pInstance->m_nThreadID, pInstance->m_szThreadNick);
	pInstance->Run();

	return 0;
}


void Thread::SetThreadName( DWORD dwThreadID, const char* szThreadName) 
{ 
#if defined(_MSC_VER)
	THREADNAME_INFO info; 
	info.dwType = 0x1000; 
	info.szName = szThreadName; 
	info.dwThreadID = dwThreadID; 
	info.dwFlags = 0; 

	__try 
	{ 
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), 
			(ULONG_PTR*)&info ); 
	} 
	__except (EXCEPTION_CONTINUE_EXECUTION) 
	{ 
	} 
#elif defined(__linux__)
	if (szThreadName && szThreadName[0])
	{
		char shortName[16] = { 0 };
		strncpy(shortName, szThreadName, sizeof(shortName) - 1);
		pthread_setname_np(pthread_self(), shortName);
	}
#endif
} 