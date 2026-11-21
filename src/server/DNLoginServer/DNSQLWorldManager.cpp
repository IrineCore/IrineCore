#include "stdafx.h"
#include "DNSQLWorldManager.h"
#include "Log.h"
#if defined( PRE_ADD_NEWCOMEBACK )
#include "DNExtManager.h"
#endif		//#ifdef PRE_ADD_COMEBACK

CDNSQLWorldManager* g_pSQLWorldManager = NULL;

extern TLoginConfig g_Config;

CDNSQLWorldManager::CDNSQLWorldManager(UINT uiWorkerThreadCount)
{
	Clear();
#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
	m_nLastHearbeatTick = timeGetTime();
#endif
}

CDNSQLWorldManager::~CDNSQLWorldManager(void)
{
	Clear();
}

void CDNSQLWorldManager::Clear()
{
	for (int iIndex = 0 ; g_Config.nWorldDBCount > iIndex ; ++iIndex) {
		TVecWorld::iterator iter = m_pVecWorld[iIndex].begin();
		for (; m_pVecWorld[iIndex].end() != iter ; ++iter) {
			CDNSQLWorld* pSQLCon = (*iter);
			DN_ASSERT(NULL != pSQLCon,	"Check!");
			delete pSQLCon;
		}
		m_pVecWorld[iIndex].clear();

		while(!m_pQueueWorld[iIndex].empty()) {
			m_pQueueWorld[iIndex].pop();
		}
	}

	m_VecWorldConnectError.clear();
	m_VecWorldVersionError.clear();

#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
	m_pVecWorldHeartbeat.clear();
#endif

}

void CDNSQLWorldManager::ClearWorldDB(int nWorldID)
{
	
	int iIndex = nWorldID - 1;
	TVecWorld::iterator iter = m_pVecWorld[iIndex].begin();
	for (; m_pVecWorld[iIndex].end() != iter ; ++iter) 
	{
		CDNSQLWorld* pSQLCon = (*iter);	
		DN_ASSERT(NULL != pSQLCon,	"Check!");
		delete pSQLCon;
	}
	m_pVecWorld[iIndex].clear();

	while(!m_pQueueWorld[iIndex].empty()) 
		m_pQueueWorld[iIndex].pop();
}

bool CDNSQLWorldManager::CreateDB()
{
	CDNSQLWorld *pWorld = NULL;

	bool bConnect, bVersion;

	for (int j = 0; j < g_Config.nWorldDBCount; j++)
	{
		bConnect = true;
		bVersion = true;

		for (int i = 0; i < g_Config.nSQLMax; i++)
		{
			if (g_Config.WorldDB[j].nWorldSetID <= 0) continue;
			if (!bConnect || !bVersion) break;

			pWorld = new CDNSQLWorld;

			pWorld->m_cThreadID = i;
			pWorld->m_nWorldSetID = g_Config.WorldDB[j].nWorldSetID;

			if (pWorld->Connect(g_Config.WorldDB[j].szIP, g_Config.WorldDB[j].nPort, g_Config.WorldDB[j].wszDBName, g_Config.WorldDB[j].wszDBID) < 0)			
			{
				g_Log.Log(LogType::_FILELOG, L"Not Connect WorldDB Ip:%S Port:%d DBName:%s, DBID:%s \r\n", g_Config.WorldDB[j].szIP, g_Config.WorldDB[j].nPort, g_Config.WorldDB[j].wszDBName, g_Config.WorldDB[j].wszDBID);
				SAFE_DELETE(pWorld);
				bConnect = false;
				break;
			}

			m_pVecWorld[g_Config.WorldDB[j].nWorldSetID - 1].push_back(pWorld);
			m_pQueueWorld[g_Config.WorldDB[j].nWorldSetID - 1].push(pWorld);
			
			if( i==0 )
			{
				int iRet = pWorld->QueryGetDatabaseVersion( &m_DatabaseVersion[j] );
				if( iRet == ERROR_NONE )
				{
					g_Log.Log(LogType::_FILELOG, L"QueryGetDatabaseVersion Success!! WorldDB Ip:%S Port:%d DBName:%s, DBID:%s \r\n", g_Config.WorldDB[j].szIP, g_Config.WorldDB[j].nPort, g_Config.WorldDB[j].wszDBName, g_Config.WorldDB[j].wszDBID);
				}
				else
				{
					g_Log.Log(LogType::_FILELOG, L"QueryGetDatabaseVersion Failed!! WorldDB Ip:%S Port:%d DBName:%s, DBID:%s \r\n", g_Config.WorldDB[j].szIP, g_Config.WorldDB[j].nPort, g_Config.WorldDB[j].wszDBName, g_Config.WorldDB[j].wszDBID);
					bVersion = false;
					continue;
				}
			}

			Sleep(1);
		}
	
		if (!bConnect)
			m_VecWorldConnectError.push_back(j);

		if (!bVersion)
			m_VecWorldVersionError.push_back(j);		
	}

	return true;
}

int CDNSQLWorldManager::CreateEachDB(int nWorldDB, int & nIndex)
{
	CDNSQLWorld *pWorld = NULL;

	int j = -1;
	for (int i = 0; i < g_Config.nWorldDBCount; i++)
	{
		if (g_Config.WorldDB[i].nWorldSetID == nWorldDB)
		{
			j = i;		// Find the target index
			break;
		}
	}

	if (j == -1)
		return 3;	// No index

	if (m_pVecWorld[g_Config.WorldDB[j].nWorldSetID - 1].size() == g_Config.nSQLMax)
		return 4;	// DB is already constructed
	else
	{
		// When only partially configured
		// Delete list and rebuild DB.
		ClearWorldDB(nWorldDB);
	}

	nIndex = j;

	for (int i = 0; i < g_Config.nSQLMax; i++)
	{
		if (g_Config.WorldDB[j].nWorldSetID <= 0) continue;

		pWorld = new CDNSQLWorld;

		pWorld->m_cThreadID = i;
		pWorld->m_nWorldSetID = g_Config.WorldDB[j].nWorldSetID;

		if (pWorld->Connect(g_Config.WorldDB[j].szIP, g_Config.WorldDB[j].nPort, g_Config.WorldDB[j].wszDBName, g_Config.WorldDB[j].wszDBID) < 0)			
		{
			g_Log.Log(LogType::_FILELOG, L"Not Connect WorldDB Ip:%S Port:%d DBName:%s, DBID:%s \r\n", g_Config.WorldDB[j].szIP, g_Config.WorldDB[j].nPort, g_Config.WorldDB[j].wszDBName, g_Config.WorldDB[j].wszDBID);
			SAFE_DELETE(pWorld);
			return 1;
		}

		m_pVecWorld[g_Config.WorldDB[j].nWorldSetID - 1].push_back(pWorld);
		m_pQueueWorld[g_Config.WorldDB[j].nWorldSetID - 1].push(pWorld);

		if( i==0 )
		{
			int iRet = pWorld->QueryGetDatabaseVersion( &m_DatabaseVersion[j] );
			if( iRet == ERROR_NONE )
			{
				g_Log.Log(LogType::_FILELOG, L"QueryGetDatabaseVersion Success!! WorldDB Ip:%S Port:%d DBName:%s, DBID:%s \r\n", g_Config.WorldDB[j].szIP, g_Config.WorldDB[j].nPort, g_Config.WorldDB[j].wszDBName, g_Config.WorldDB[j].wszDBID);
			}
			else
			{
				g_Log.Log(LogType::_FILELOG, L"QueryGetDatabaseVersion Failed!! WorldDB Ip:%S Port:%d DBName:%s, DBID:%s \r\n", g_Config.WorldDB[j].szIP, g_Config.WorldDB[j].nPort, g_Config.WorldDB[j].wszDBName, g_Config.WorldDB[j].wszDBID);
				return 2;
			}
		}

		Sleep(1);
	}

	return 0;
}

void CDNSQLWorldManager::DoUpdate(DWORD nCurTick)
{
#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
	if (nCurTick - m_nLastHearbeatTick >= DNProcessCheck::SQLHeartbeatInterval)
	{
		if (m_nLastHearbeatTick == 0)
		{
			m_nLastHearbeatTick = timeGetTime();
			return;
		}

		m_nLastHearbeatTick = timeGetTime();

		CDNSQLWorld* pWorld = NULL;

		//Cleanly pull out unused entries
		for (int i = 0; i < g_Config.nWorldDBCount; i++)
		{
			ScopeLock<CSyncLock> Lock(m_Sync);

			while (!m_pQueueWorld[g_Config.WorldDB[i].nWorldSetID - 1].empty())
			{
				pWorld = m_pQueueWorld[g_Config.WorldDB[i].nWorldSetID - 1].front();
				LONG nRefCount = pWorld->IncRefCount();
				if (1 != nRefCount) {
					DN_ASSERT(0,	"Check!");
				}

				m_pQueueWorld[g_Config.WorldDB[i].nWorldSetID - 1].pop();
				m_pVecWorldHeartbeat.push_back(pWorld);
			}

			for (TVecWorld::iterator ii = m_pVecWorldHeartbeat.begin(); ii != m_pVecWorldHeartbeat.end(); )
			{
				pWorld = (*ii);
				if (pWorld->QueryHeartbeat() != ERROR_NONE)
					_DANGER_POINT();

				LONG nRefCount = pWorld->DecRefCount();				
				_ASSERT_EXPR(0 == nRefCount, L"Unexpected WorldDB state (contact developer)");
				m_pQueueWorld[g_Config.WorldDB[i].nWorldSetID - 1].push(pWorld);

				ii = m_pVecWorldHeartbeat.erase(ii);
			}
			m_pVecWorldHeartbeat.clear();
		}
	}
#endif		//#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
}

CDNSQLWorld* CDNSQLWorldManager::FindWorldDB(int nWorldSetID)
{
	CDNSQLWorld* pWorld = NULL;
	{
		ScopeLock<CSyncLock> Lock(m_Sync);

		if (!CHECK_RANGE(nWorldSetID, 1, WORLDCOUNTMAX)) {
			return NULL;
		}

		DWORD dwThreadID = ::GetCurrentThreadId();
		DN_ASSERT(0 != dwThreadID,	"Check!");

		TMapWorld::iterator iter = m_pMapWorld[nWorldSetID - 1].find(dwThreadID);
		if (m_pMapWorld[nWorldSetID - 1].end() != iter) {
			pWorld = (iter->second);
			LONG nRefCount = pWorld->IncRefCount();
			if (1 >= nRefCount) {
				DN_ASSERT(0,	"Check!");
			}
			return pWorld;
		}

		if (m_pQueueWorld[nWorldSetID - 1].empty()) {
			return NULL;
		}

		pWorld = m_pQueueWorld[nWorldSetID - 1].front();
		LONG nRefCount = pWorld->IncRefCount();
		if (1 != nRefCount) {
			DN_ASSERT(0,	"Check!");
		}
		m_pQueueWorld[nWorldSetID - 1].pop();
		m_pMapWorld[nWorldSetID - 1].insert(TMapWorld::value_type(dwThreadID, pWorld));	// In sync state, so no need to check result?
	}
	DN_ASSERT(NULL != pWorld,	"Invalid!");

	return pWorld;
}

CDNSQLWorld* CDNSQLWorldManager::FindWorldDB()
{
	return(FindWorldDB(1));
}

void CDNSQLWorldManager::FreeWorldDB(int nWorldSetID, CDNSQLWorld* pWorldDB)
{
	if (!pWorldDB) {
		DN_RETURN_NONE;
	}

	{
		ScopeLock<CSyncLock> aLock(m_Sync);

		LONG nRefCount = pWorldDB->DecRefCount();
		if (0 == nRefCount) {
			m_pMapWorld[nWorldSetID - 1].erase(::GetCurrentThreadId());
			m_pQueueWorld[nWorldSetID - 1].push(pWorldDB);
		}
		DN_ASSERT(0 <= nRefCount,	"Invalid!");
	}
}

// Create character
int CDNSQLWorldManager::QueryAddCharacter(int nWorldSetID, INT64 biCharacterDBID, UINT nAccountDBID, WCHAR *pAccountName, char cAccountLevel, int nWorldID, WCHAR *pCharName, char cClass, char cCharIndex, DWORD dwHairColor, DWORD dwEyeColor, DWORD dwSkinColor, 
										  int nMapID, TPosition &Pos, float fRotate, int nRebirthCoin, int nPCBangRebirthCoin, int *nEquipArray, int *nSkillArray, int *nUnlockSkillArray, TQuickSlot *QuickSlotArray, 
										  TCreateCharacterItem *CreateItemArray, BYTE cCreateItemMax, bool bJoinBeginnerGuild, UINT &nGuildDBID, WCHAR* pwszIP )
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);

	return pWorldDB->QueryAddCharacter(biCharacterDBID, nAccountDBID, pAccountName, cAccountLevel, nWorldID, pCharName, cClass, cCharIndex, dwHairColor, dwEyeColor, dwSkinColor, nMapID, Pos, fRotate, nRebirthCoin, nPCBangRebirthCoin, 
		nEquipArray, nSkillArray, nUnlockSkillArray, QuickSlotArray, CreateItemArray, cCreateItemMax, bJoinBeginnerGuild, nGuildDBID, pwszIP );
}

int CDNSQLWorldManager::QueryAddDWCCharacter(int nWorldSetID, INT64 biCharacterDBID, UINT nAccountDBID, WCHAR *pAccountName, int nWorldID, WCHAR *pCharName,
											 BYTE cClass, BYTE cJobCode1, BYTE cJobCode2, BYTE cLevel, int nExp, BYTE cCharIndex,
											 DWORD dwHairColor, DWORD dwEyeColor, DWORD dwSkinColor,
											 int nMapID, TPosition &Pos, float fRotate, BYTE cRebirthCoin, BYTE cPCBangRebirthCoin,
											 int nDeleteWaitingTime, int nCoin, int nSkillPoint,
											 int *nEquipArray, int *nSkillArray, int *nUnlockSkillArray, TQuickSlot *QuickSlotArray,
											 TCreateCharacterItem *CreateItemArray, BYTE cCreateItemMax, WCHAR* pwszIP)
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);

	return pWorldDB->QueryAddDWCCharacter(biCharacterDBID, nAccountDBID, pAccountName, nWorldID, pCharName,
		cClass, cJobCode1, cJobCode2, cLevel, nExp, cCharIndex,
		dwHairColor, dwEyeColor, dwSkinColor,
		nMapID, Pos, fRotate, cRebirthCoin, cPCBangRebirthCoin,
		nDeleteWaitingTime, nCoin, nSkillPoint,
		nEquipArray, nSkillArray, nUnlockSkillArray, QuickSlotArray,
		CreateItemArray, cCreateItemMax, pwszIP);
}

int CDNSQLWorldManager::QueryRollbackAddCharacter(int nWorldSetID, INT64 biCharacterDBID)
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);

	return pWorldDB->QueryRollbackAddCharacter(biCharacterDBID);
}

int CDNSQLWorldManager::QueryFirstUseCharacter(int nWorldSetID, INT64 biCharacterDBID, int *nSkillArray, int *nUnlockSkillArray, TQuickSlot *QuickSlotArray)
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);

	return pWorldDB->QueryFirstUseCharacter(biCharacterDBID, nSkillArray, nUnlockSkillArray, QuickSlotArray);
}

// Delete character
int CDNSQLWorldManager::QueryDelCharacter(int nWorldSetID, INT64 biCharacterDBID, OUT bool &bVillageFirstVisit, OUT TIMESTAMP_STRUCT& DeleteDate )
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);

	return pWorldDB->QueryDelCharacter(biCharacterDBID, bVillageFirstVisit, DeleteDate );
}

int CDNSQLWorldManager::QueryReviveCharacter( int nWorldSetID, INT64 biCharacterDBID )
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);

	return pWorldDB->QueryReviveCharacter( biCharacterDBID );
}

// Query character attributes
#if defined(PRE_MOD_SELECT_CHAR)
int CDNSQLWorldManager::QueryGetListCharacter(int nWorldSetID, int nWorldID, UINT nAccountDBID, int nCharacterMaxCount, OUT std::map<INT64, TDBListCharData> &MapCharacterList)
#else	// #if defined(PRE_MOD_SELECT_CHAR)
int CDNSQLWorldManager::QueryGetListCharacter(int nWorldSetID, int nWorldID, UINT nAccountDBID, int nCharacterMaxCount, OUT TDBListCharData *CharList)
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
			printf("[Warning] FindWorldDB(nWorldSetID: %d)  ERROR_GENERIC_DBCON_NOT_FOUND 7 \n", nWorldSetID); //[Warning] 
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);
#if defined( PRE_ADD_NEWCOMEBACK )
	int nCombackEffectItemID = g_pExtManager->GetGlobalWeightIntValue(ComebackEffectItemID_Login);
#endif

#if defined(PRE_MOD_SELECT_CHAR)
	int nRet = pWorldDB->QueryGetListCharacter(nAccountDBID, nWorldID, nCharacterMaxCount, MapCharacterList);
	if (nRet == ERROR_NONE){
		if (!MapCharacterList.empty()){
			for (std::map<INT64, TDBListCharData>::iterator iter = MapCharacterList.begin(); iter != MapCharacterList.end(); iter++){
				if (iter->second.biCharacterDBID <= 0) continue;

				pWorldDB->QueryGetListEquipment(iter->second.biCharacterDBID, iter->second.nEquipArray, iter->second.nCashEquipArray);
#if defined( PRE_ADD_NEWCOMEBACK )
				int nEffectItemRet = pWorldDB->QueryCheckCombackEffectItem( iter->second.biCharacterDBID, nCombackEffectItemID );
				if( nEffectItemRet == ERROR_NONE )
					iter->second.bCombackEffectItem = true;
				else
					iter->second.bCombackEffectItem = false;
#endif
			}
		}
	}
	else
		nRet = ERROR_LOGIN_CHARACTERLIST_ERROR;

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	int nRet = pWorldDB->QueryGetListCharacter(nAccountDBID, nWorldID, nCharacterMaxCount, CharList);
	if (nRet == ERROR_NONE){
		for (int i = 0; i < nCharacterMaxCount; i++){
			if (CharList[i].biCharacterDBID <= 0) continue;

			pWorldDB->QueryGetListEquipment(CharList[i].biCharacterDBID, CharList[i].nEquipArray, CharList[i].nCashEquipArray);
#if defined( PRE_ADD_NEWCOMEBACK )
			int nEffectItemRet = pWorldDB->QueryCheckCombackEffectItem( CharList[i].biCharacterDBID, nCombackEffectItemID );
			if( nEffectItemRet == ERROR_NONE )
				CharList[i].bCombackEffectItem = true;			 
			else
				CharList[i].bCombackEffectItem = false;
#endif
		}
	}
	else
		nRet = ERROR_LOGIN_CHARACTERLIST_ERROR;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	return nRet;
}

int CDNSQLWorldManager::QueryModLastVillageMapID( int nWorldSetID, UINT uiAccountDBID, INT64 biCharacterDBID, int iLastVillageMapID )
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);

	TPosition Pos = {0, };
	return pWorldDB->QueryModCharacterStatus( biCharacterDBID, 16, 0, 0, 0, 0, iLastVillageMapID, 0, 0, Pos, 0, 0, 0, false, false, 0);
}

int CDNSQLWorldManager::QueryGetCharacterPartialy8(int nWorldSetID, INT64 biCharacterDBID, WCHAR *pCharName, int &nWorldID)
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);

	return pWorldDB->QueryGetCharacterPartialy8(biCharacterDBID, pCharName, nWorldID);
}

int CDNSQLWorldManager::QueryAddSkill(int nWorldSetID, INT64 biCharacterDBID, int nSkillID, char cSkillLevel, int nCoolTime, char cSkillChangeCode, INT64 biCoin, int nChannelID, int nMapID, char cSkillPage)
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);	

	return pWorldDB->QueryAddSkill(biCharacterDBID, nSkillID, cSkillLevel, nCoolTime, cSkillChangeCode, biCoin, nChannelID, nMapID, cSkillPage);
}

#ifdef PRE_ADD_BEGINNERGUILD
int CDNSQLWorldManager::QueryAddWillSendMail(int nWorldSetID, INT64 biCharacterDBID, int nMailID, const WCHAR * pMemo)
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);	

	return pWorldDB->QueryAddWillSendMail(biCharacterDBID, nMailID, pMemo);
}
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

#ifdef PRE_ADD_DOORS
int CDNSQLWorldManager::QueryCancelDoorsAuthentication(int nWorldSetID, UINT nAccountDBID)
{
	CDNSQLWorld *pWorldDB = FindWorldDB(nWorldSetID);
	if (!pWorldDB) {
		return ERROR_GENERIC_DBCON_NOT_FOUND;
	}

	CDNSQLWorldAuto Auto(nWorldSetID, pWorldDB, this);	

	return pWorldDB->QueryCancelDoorsAuthentication(nAccountDBID);
}
#endif		//#ifdef PRE_ADD_DOORS
