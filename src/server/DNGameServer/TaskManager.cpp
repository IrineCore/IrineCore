#include "stdafx.h"
#include <process.h>
#include "TaskManager.h"
#include "Task.h"
#include "FrameSync.h"

CTaskManager::CTaskManager( CMultiRoom *pRoom )
: CMultiSingleton<CTaskManager, MAX_SESSION_COUNT>( pRoom )
{
	m_bLockDebugTask = false;
}

CTaskManager::~CTaskManager()
{
	for( DWORD i=0; i<m_pVecTaskList.size(); i++ ) {
		RemoveTask( m_pVecTaskList[i]->m_dwHandle );
		--i;
	}
}


DWORD CTaskManager::AddTask( CTask *pTask, const char *szName, int nSecureFrame, bool bUseThread )
{
	static DWORD dwCounter = 0;

	m_pVecTaskList.push_back( pTask );
	pTask->m_dwHandle = dwCounter;
	++dwCounter;

	pTask->m_bUseThread = bUseThread;

	pTask->ResetTimer();
	pTask->m_pFrameSync->Begin( ( m_bLockDebugTask == true ) ? 60 : nSecureFrame );
	pTask->SetTaskName( szName );

	if( bUseThread == true ) {
		pTask->m_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)pTask, 0, &pTask->m_dwThreadID );
	}

//	if( strcmp( szName, "GameTask" ) == NULL ) TaskEnum::GameTask = (int)m_pVecTaskList.size() - 1;
	return pTask->m_dwHandle;
}

void CTaskManager::RemoveAllTask()
{
	for( DWORD i=0; i<m_pVecTaskList.size(); i++ ) {
		if( m_pVecTaskList[i]->m_bUseThread == true ) {

			m_pVecTaskList[i]->m_bUseThread = false;
			CloseHandle( m_pVecTaskList[i]->m_hThreadHandle );

		}
	}
	SAFE_DELETE_PVEC( m_pVecTaskList );
}

void CTaskManager::RemoveTask( DWORD dwTaskHandle )
{
	for( DWORD i=0; i<m_pVecTaskList.size(); i++ ) {
		if( m_pVecTaskList[i]->m_dwHandle == dwTaskHandle ) {
			if( m_pVecTaskList[i]->m_bUseThread == true ) {
				m_pVecTaskList[i]->m_bUseThread = false;
				CloseHandle( m_pVecTaskList[i]->m_hThreadHandle );

			}
			m_pVecTaskList.erase( m_pVecTaskList.begin() + i );
			return;
		}
	}
}

void CTaskManager::RemoveTask( const char *szName )
{
	CTask *pTask = GetTask( szName );
	if( !pTask ) return;
	RemoveTask( pTask->m_dwHandle );
}

CTask *CTaskManager::GetTask( DWORD dwTaskHandle )
{
	for( DWORD i=0; i<m_pVecTaskList.size(); i++ ) {
		if( m_pVecTaskList[i]->m_dwHandle == dwTaskHandle ) return m_pVecTaskList[i];
	}
	return NULL;
}

CTask *CTaskManager::GetTask( const char *szName )
{
	for( DWORD i=0; i<m_pVecTaskList.size(); i++ ) {
		if( !strcmp( m_pVecTaskList[i]->GetTaskName(), szName ) ) return m_pVecTaskList[i];
	}
	return NULL;
}

bool CTaskManager::Excute()
{
	for( DWORD i=0; i<m_pVecTaskList.size(); i++ ) {
		if( m_pVecTaskList[i]->m_bUseThread == false ) {
			CTask *pTask = m_pVecTaskList[i];
			if( !pTask->IsEnableTaskProcess() ) continue;
			if( pTask->m_pFrameSync->CheckSync() ) {
				LOCAL_TIME LocalTime = pTask->m_pFrameSync->GetMSTime();
				float fDelta = ( LocalTime - pTask->GetPrevLocalTime() ) / 1000.f;

				pTask->SetLocalTime( LocalTime );
				pTask->SetDeltaTime( fDelta );

				pTask->Process( LocalTime, fDelta );

				if( pTask->m_bResetTimer ) {
					pTask->m_bResetTimer = false;
					continue;
				}
				pTask->SetPrevLocalTime( LocalTime );
			}
			pTask->m_pFrameSync->UpdateTime( ( m_bLockDebugTask == true ) ? 60 : 0 );

			if( pTask->m_bDestroyTask == true ) {
				RemoveTask( pTask->m_dwHandle );
				i--;
				if( pTask->m_bDeleteThis ) SAFE_DELETE( pTask );
			}
		}
	}
	if( m_pVecTaskList.empty() ) return false;
	return true;
}

UINT __stdcall CTaskManager::BeginThread( void *pParam )
{
	CTask *pTask = (CTask*)pParam;

	while(1) {
		if( pTask->m_bUseThread == false ) {
			OutputDebug( "Thread Exit : %x\n", &pTask );
			return 0;
		}

		if( pTask->m_bDestroyTask == true ) {
			CTaskManager::GetInstance( pTask->GetRoom() ).RemoveTask( pTask->m_dwHandle );
			if( pTask->m_bDeleteThis ) SAFE_DELETE( pTask );
			OutputDebug( "Thread Exit : %x\n", &pTask );
			return 0;
		}

		if( !pTask->IsEnableTaskProcess() ) continue;
		if( pTask->m_pFrameSync->CheckSync() ) {
			LOCAL_TIME LocalTime = pTask->m_pFrameSync->GetMSTime();
			float fDelta = ( LocalTime - pTask->GetPrevLocalTime() ) / 1000.f;

			pTask->SetLocalTime( LocalTime );
			pTask->SetDeltaTime( fDelta );

			pTask->Process( LocalTime, fDelta );

			pTask->SetPrevLocalTime( LocalTime );
		}
		pTask->m_pFrameSync->UpdateTime();

		Sleep(1);
	}

	return 0;
}
