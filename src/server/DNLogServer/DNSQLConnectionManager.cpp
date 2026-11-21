#include "stdafx.h"
#include "DNSQLConnectionManager.h"
#include "Log.h"

extern TLogConfig g_Config;

CDNSQLConnectionManager g_SQLConnectionManager;
CDNSQLConnectionManager::CDNSQLConnectionManager(void)
{
	Clear();
}

CDNSQLConnectionManager::~CDNSQLConnectionManager(void)
{
	DestroyDB();
}

void CDNSQLConnectionManager::Clear()
{
	memset(&m_ServerLogDBArray, 0, sizeof(m_ServerLogDBArray));
	memset(&m_LogDBArray, 0, sizeof(m_LogDBArray));
}

bool CDNSQLConnectionManager::CreateServerLogDB()
{
	CDNSQLConnection *pDB = NULL;
	for (int i = 0; i < g_Config.nThreadMax; i++)
	{
		pDB = new CDNSQLConnection;

		m_ServerLogDBArray[i] = pDB;
		m_ServerLogDBArray[i]->m_cThreadID = i;

		if (pDB->Connect(g_Config.ServerLogDB.szIP, g_Config.ServerLogDB.nPort, g_Config.ServerLogDB.wszDBName, g_Config.ServerLogDB.wszDBID) < 0)
		{
			g_Log.Log(LogType::_FILELOG, L"Not Connect ServerLogDB Ip:%S Port:%d DBName:%s DBID:%s \r\n", g_Config.ServerLogDB.szIP, g_Config.ServerLogDB.nPort, g_Config.ServerLogDB.wszDBName, g_Config.ServerLogDB.wszDBID);
			//SAFE_DELETE(pDB);
			return false;
		}

		Sleep(100);
	}

	return true;	
}

bool CDNSQLConnectionManager::CreateLogDB()
{
	int nWorldID = 0;
	CDNSQLConnection *pDB = NULL;
	for (int j = 0; j < g_Config.nLogDBCount; j++){
		for (int i = 0; i < g_Config.nThreadMax; i++){
			nWorldID = g_Config.LogDB[j].nWorldSetID;

			pDB = new CDNSQLConnection;

			m_LogDBArray[i][nWorldID - 1] = pDB;
			m_LogDBArray[i][nWorldID - 1]->m_cThreadID = i;
			m_LogDBArray[i][nWorldID - 1]->m_nWorldSetID = nWorldID;

			if (pDB->Connect(g_Config.LogDB[j].szIP, g_Config.LogDB[j].nPort, g_Config.LogDB[j].wszDBName, g_Config.LogDB[j].wszDBID) < 0)			
			{
				g_Log.Log(LogType::_FILELOG, L"Not Connect LogDB Ip:%S Port:%d DBName:%s DBID:%s \r\n", g_Config.LogDB[j].szIP, g_Config.LogDB[j].nPort, g_Config.LogDB[j].wszDBName, g_Config.LogDB[j].wszDBID);
				//SAFE_DELETE(pDB);
				return false;
			}

			Sleep(100);
		}
	}

	return true;
}

bool CDNSQLConnectionManager::CreateDB()
{
	
	if (!CreateServerLogDB()) return false;
	if (!CreateLogDB()) return false;

	return true;
}

void CDNSQLConnectionManager::DestroyDB()
{
	int i, j;

	for( i=0 ; i<THREADMAX ; ++i )
	{
		SAFE_DELETE( m_ServerLogDBArray[i] );
	}

	for (i = 0; i < WORLDCOUNTMAX; i++){
		for (j = 0; j < g_Config.nThreadMax; j++){
			SAFE_DELETE(m_LogDBArray[j][i]);
		}
	}

	Clear();
}

CDNSQLConnection* CDNSQLConnectionManager::FindServerLogDB(int nThreadIndex)
{
	if ((nThreadIndex < 0) || (nThreadIndex >= g_Config.nThreadMax)) return NULL;
	return m_ServerLogDBArray[nThreadIndex];
}

CDNSQLConnection* CDNSQLConnectionManager::FindLogDB(int nThreadIndex, char cWorldID)
{
	if ((cWorldID <= 0) || (cWorldID > WORLDCOUNTMAX)) return NULL;
	if ((nThreadIndex < 0) || (nThreadIndex >= g_Config.nThreadMax)) return NULL;

	return m_LogDBArray[nThreadIndex][cWorldID - 1];
}

CDNSQLConnection* CDNSQLConnectionManager::FindLogDB(int nThreadIndex)
{
	if ((nThreadIndex < 0) || (nThreadIndex >= g_Config.nThreadMax)) return NULL;
	return m_LogDBArray[nThreadIndex][0];
}
