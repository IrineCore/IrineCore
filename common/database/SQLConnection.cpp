#include "stdafx.h"
#include "SQLConnection.h"
#include "Log.h"

#if defined(_LOGINSERVER)
extern TLoginConfig g_Config;
#endif
#if defined(_DBSERVER)
extern TDBConfig g_Config;
#endif

namespace
{
	static void TrimWhitespaceW(WCHAR* pBuf)
	{
		if (!pBuf || !pBuf[0]) return;
		size_t len = wcslen(pBuf);
		size_t start = 0;
		while (start < len && (pBuf[start] == L' ' || pBuf[start] == L'\t'))
			++start;
		size_t end = len;
		while (end > start && (pBuf[end - 1] == L' ' || pBuf[end - 1] == L'\t' || pBuf[end - 1] == L'\r' || pBuf[end - 1] == L'\n'))
			--end;
		if (start > 0)
		{
			for (size_t i = 0; i < end - start; ++i)
				pBuf[i] = pBuf[start + i];
		}
		pBuf[end - start] = L'\0';
	}

	static void StripBracesW(WCHAR* pBuf)
	{
		TrimWhitespaceW(pBuf);
		size_t len = wcslen(pBuf);
		if (len >= 2 && pBuf[0] == L'{' && pBuf[len - 1] == L'}')
		{
			for (size_t i = 0; i < len - 2; ++i) pBuf[i] = pBuf[i + 1];
			pBuf[len - 2] = L'\0';
			TrimWhitespaceW(pBuf);
		}
	}

	static void EscapeOdbcValueW(const WCHAR* pSrc, WCHAR* pDst, size_t cchDst)
	{
		if (!pDst || cchDst == 0) return;
		pDst[0] = L'\0';
		if (!pSrc || !pSrc[0]) return;

		bool bNeedsBraces = false;
		for (const WCHAR* p = pSrc; *p; ++p)
		{
			if (*p == L';' || *p == L'{' || *p == L'}' || *p == L'=' || *p == L' ' || *p == L'\t')
			{
				bNeedsBraces = true;
				break;
			}
		}

		if (!bNeedsBraces)
		{
			wcsncpy_s(pDst, cchDst, pSrc, _TRUNCATE);
			return;
		}

		size_t pos = 0;
		if (pos + 2 >= cchDst) return;
		pDst[pos++] = L'{';
		for (const WCHAR* p = pSrc; *p && pos + 3 < cchDst; ++p)
		{
			if (*p == L'}')
			{
				if (pos + 4 >= cchDst) break;
				pDst[pos++] = L'}';
				pDst[pos++] = L'}';
			}
			else
			{
				pDst[pos++] = *p;
			}
		}
		pDst[pos++] = L'}';
		pDst[pos] = L'\0';
	}

	static bool IsTrueValueW(const WCHAR* pVal)
	{
		if (!pVal || !pVal[0]) return false;
		return (_wcsicmp(pVal, L"Yes") == 0 || _wcsicmp(pVal, L"True") == 0 || wcscmp(pVal, L"1") == 0);
	}

	static bool FileExistsW(const WCHAR* pPath)
	{
		DWORD dwAttr = GetFileAttributesW(pPath);
		return (dwAttr != INVALID_FILE_ATTRIBUTES && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY));
	}

	static bool ResolveOdbcIniPathW(WCHAR* pOutPath, size_t cchOut)
	{
		const WCHAR* relCandidates[] = {
			L"./config/odbc.ini",
			L"./Config/odbc.ini",
			L"./odbc.ini"
		};
		for (size_t i = 0; i < _countof(relCandidates); ++i)
		{
			WCHAR fullPath[MAX_PATH] = { 0, };
			if (GetFullPathNameW(relCandidates[i], _countof(fullPath), fullPath, NULL) > 0 && FileExistsW(fullPath))
			{
				wcsncpy_s(pOutPath, cchOut, fullPath, _TRUNCATE);
				return true;
			}
		}

		WCHAR exeDir[MAX_PATH] = { 0, };
		DWORD dwLen = GetModuleFileNameW(NULL, exeDir, _countof(exeDir));
		if (dwLen > 0 && dwLen < _countof(exeDir))
		{
			WCHAR* pLastSlash = wcsrchr(exeDir, L'\\');
			if (!pLastSlash) pLastSlash = wcsrchr(exeDir, L'/');
			if (pLastSlash) *(pLastSlash + 1) = L'\0';

			const WCHAR* subCandidates[] = {
				L"config\\odbc.ini",
				L"Config\\odbc.ini",
				L"odbc.ini"
			};
			for (size_t i = 0; i < _countof(subCandidates); ++i)
			{
				WCHAR fullPath[MAX_PATH] = { 0, };
				swprintf_s(fullPath, _countof(fullPath), L"%s%s", exeDir, subCandidates[i]);
				if (FileExistsW(fullPath))
				{
					wcsncpy_s(pOutPath, cchOut, fullPath, _TRUNCATE);
					return true;
				}
			}
		}
		return false;
	}

	static void ReadOdbcSettingW(const WCHAR* pIniPath, const WCHAR* pDbSection, const WCHAR* pKey, const WCHAR* pDefault, WCHAR* pOut, DWORD cchOut)
	{
		wcsncpy_s(pOut, cchOut, pDefault ? pDefault : L"", _TRUNCATE);
		if (!pIniPath || !pIniPath[0]) return;

		WCHAR buf[512] = { 0, };
		if (GetPrivateProfileStringW(L"MSSQL", pKey, L"", buf, _countof(buf), pIniPath) > 0 && buf[0])
		{
			TrimWhitespaceW(buf);
			wcsncpy_s(pOut, cchOut, buf, _TRUNCATE);
		}
		if (GetPrivateProfileStringW(L"ODBC", pKey, L"", buf, _countof(buf), pIniPath) > 0 && buf[0])
		{
			TrimWhitespaceW(buf);
			wcsncpy_s(pOut, cchOut, buf, _TRUNCATE);
		}
		if (pDbSection && pDbSection[0])
		{
			if (GetPrivateProfileStringW(pDbSection, pKey, L"", buf, _countof(buf), pIniPath) > 0 && buf[0])
			{
				TrimWhitespaceW(buf);
				wcsncpy_s(pOut, cchOut, buf, _TRUNCATE);
			}
		}
	}

	static int ReadOdbcIntW(const WCHAR* pIniPath, const WCHAR* pDbSection, const WCHAR* pKey, int nDefault)
	{
		if (!pIniPath || !pIniPath[0]) return nDefault;
		WCHAR buf[64] = { 0, };
		ReadOdbcSettingW(pIniPath, pDbSection, pKey, L"", buf, _countof(buf));
		if (!buf[0]) return nDefault;
		return _wtoi(buf);
	}

	static bool IsDriverMissingDiagW(SQLHDBC hdbc)
	{
		if (!hdbc) return false;
		SQLWCHAR sqlState[10] = { 0, };
		SQLWCHAR errMsg[512] = { 0, };
		SQLINTEGER nativeErr = 0;
		SQLSMALLINT msgLen = 0;
		for (SQLSMALLINT rec = 1; rec <= 5; ++rec)
		{
			SQLRETURN rc = SQLGetDiagRecW(SQL_HANDLE_DBC, hdbc, rec, sqlState, &nativeErr, errMsg, _countof(errMsg), &msgLen);
			if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
				break;
			if (wcscmp(sqlState, L"IM002") == 0 || wcscmp(sqlState, L"IM003") == 0)
				return true;
		}
		return false;
	}
}

CSQLConnection::CSQLConnection(void): m_henv(0), m_hdbc(0), m_hstmt(0), m_cThreadID(0), m_nWorldSetID(0)
{
	memset(&m_szIp, 0, sizeof(m_szIp));
	m_nPort = 0;
	memset(&m_wszDBName, 0, sizeof(m_wszDBName));
	memset(&m_wszID, 0, sizeof(m_wszID));

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	m_RefCount = 0;

}

CSQLConnection::~CSQLConnection(void)
{
	Disconnect();
}

int CSQLConnection::Connect(char *pIp, int nPort, WCHAR *pwszDBName, WCHAR *pwszID)
{
	// Copy caller arguments first so Reconnect() passing m_szIp / m_wszDBName / m_wszID is safe across Disconnect().
	char szCallerIp[IPLENMAX] = { 0, };
	int nCallerPort = nPort;
	WCHAR wszCallerDB[DBNAMELENMAX] = { 0, };
	WCHAR wszCallerID[IDLENMAX] = { 0, };

	if (pIp) strncpy_s(szCallerIp, _countof(szCallerIp), pIp, _TRUNCATE);
	if (pwszDBName) wcsncpy_s(wszCallerDB, _countof(wszCallerDB), pwszDBName, _TRUNCATE);
	if (pwszID) wcsncpy_s(wszCallerID, _countof(wszCallerID), pwszID, _TRUNCATE);

	Disconnect();

	WCHAR wszIniPath[MAX_PATH] = { 0, };
	bool bHasIni = ResolveOdbcIniPathW(wszIniPath, _countof(wszIniPath));
	if (!bHasIni)
	{
		g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"[ODBC] odbc.ini not found; using DSN-less Auto driver defaults\n");
	}

	const WCHAR* pSection = wszCallerDB[0] ? wszCallerDB : NULL;

	WCHAR wszDriver[128] = { 0, };
	WCHAR wszServerOverride[256] = { 0, };
	WCHAR wszDatabase[DBNAMELENMAX] = { 0, };
	WCHAR wszUid[IDLENMAX] = { 0, };
	WCHAR wszPwd[256] = { 0, };
	WCHAR wszTrusted[32] = { 0, };
	WCHAR wszEncrypt[32] = { 0, };
	WCHAR wszTrustCert[32] = { 0, };
	WCHAR wszMars[32] = { 0, };
	WCHAR wszMultiSubnet[32] = { 0, };
	WCHAR wszAppName[128] = { 0, };
	WCHAR wszWsid[128] = { 0, };

	ReadOdbcSettingW(wszIniPath, pSection, L"Driver", L"Auto", wszDriver, _countof(wszDriver));
	StripBracesW(wszDriver);
	ReadOdbcSettingW(wszIniPath, pSection, L"Server", L"", wszServerOverride, _countof(wszServerOverride));
	int nPortOverride = ReadOdbcIntW(wszIniPath, pSection, L"Port", 0);
	ReadOdbcSettingW(wszIniPath, pSection, L"Database", wszCallerDB, wszDatabase, _countof(wszDatabase));
	ReadOdbcSettingW(wszIniPath, pSection, L"Trusted_Connection", L"No", wszTrusted, _countof(wszTrusted));
	ReadOdbcSettingW(wszIniPath, pSection, L"Uid", wszCallerID, wszUid, _countof(wszUid));
	if (!wszUid[0] && !IsTrueValueW(wszTrusted))
		ReadOdbcSettingW(wszIniPath, pSection, L"User", wszCallerID, wszUid, _countof(wszUid));
	ReadOdbcSettingW(wszIniPath, pSection, L"Pwd", L"", wszPwd, _countof(wszPwd));
	if (!wszPwd[0])
		ReadOdbcSettingW(wszIniPath, pSection, L"Password", L"", wszPwd, _countof(wszPwd));
	ReadOdbcSettingW(wszIniPath, pSection, L"Encrypt", L"Optional", wszEncrypt, _countof(wszEncrypt));
	ReadOdbcSettingW(wszIniPath, pSection, L"TrustServerCertificate", L"Yes", wszTrustCert, _countof(wszTrustCert));
	ReadOdbcSettingW(wszIniPath, pSection, L"MARS_Connection", L"No", wszMars, _countof(wszMars));
	ReadOdbcSettingW(wszIniPath, pSection, L"MultiSubnetFailover", L"No", wszMultiSubnet, _countof(wszMultiSubnet));
	ReadOdbcSettingW(wszIniPath, pSection, L"ApplicationName", L"DragonNestCap60", wszAppName, _countof(wszAppName));
	ReadOdbcSettingW(wszIniPath, pSection, L"WorkstationID", L"DragonNestServer", wszWsid, _countof(wszWsid));

	int nLoginTimeout = ReadOdbcIntW(wszIniPath, pSection, L"LoginTimeout", 5);
	if (nLoginTimeout <= 0) nLoginTimeout = 5;
	int nConnTimeout = ReadOdbcIntW(wszIniPath, pSection, L"ConnectionTimeout", 5);
	if (nConnTimeout <= 0) nConnTimeout = 5;

	WCHAR wszServerTarget[280] = { 0, };
	if (wszServerOverride[0])
	{
		wcsncpy_s(wszServerTarget, _countof(wszServerTarget), wszServerOverride, _TRUNCATE);
	}
	else
	{
		MultiByteToWideChar(CP_ACP, 0, szCallerIp, -1, wszServerTarget, _countof(wszServerTarget));
	}
	int nEffectivePort = (nPortOverride > 0) ? nPortOverride : nCallerPort;
	if (nEffectivePort > 0 && wszServerTarget[0] && !wcschr(wszServerTarget, L',') && !wcschr(wszServerTarget, L'\\'))
	{
		WCHAR wszPortBuf[32] = { 0, };
		swprintf_s(wszPortBuf, _countof(wszPortBuf), L",%d", nEffectivePort);
		wcsncat_s(wszServerTarget, _countof(wszServerTarget), wszPortBuf, _TRUNCATE);
	}

	bool bTrusted = IsTrueValueW(wszTrusted);
	WCHAR wszEscUid[256] = { 0, };
	WCHAR wszEscPwd[512] = { 0, };
	EscapeOdbcValueW(wszUid, wszEscUid, _countof(wszEscUid));
	EscapeOdbcValueW(wszPwd, wszEscPwd, _countof(wszEscPwd));

	if (!bTrusted && !wszPwd[0])
	{
		g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"[ODBC] Warning: empty password in odbc.ini for DB[%s] Uid[%s]\n", wszDatabase, wszUid);
	}

	const WCHAR* autoDrivers[] = {
		L"ODBC Driver 18 for SQL Server",
		L"ODBC Driver 17 for SQL Server",
		L"ODBC Driver 13 for SQL Server",
		L"SQL Server Native Client 11.0",
		L"SQL Server Native Client 10.0",
		L"SQL Server"
	};
	const WCHAR* customDrivers[1] = { wszDriver };
	bool bAutoDriver = (!wszDriver[0] || _wcsicmp(wszDriver, L"Auto") == 0);
	const WCHAR** pCandidates = bAutoDriver ? autoDrivers : customDrivers;
	size_t nCandidateCount = bAutoDriver ? _countof(autoDrivers) : 1;

	SQLRETURN RetCode = SQL_ERROR;
	const WCHAR* pSelectedDriver = L"";
	WCHAR wszConnStrIn[1024] = { 0, };
	SQLWCHAR wszConnStrOut[1024] = { 0, };
	SQLSMALLINT cbConnStrOutMax = 0;

	RetCode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv);
	if (CheckRetCode(RetCode, L"SQLAllocHandle") != 0)
	{
		g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"DBConnect Error SQLAllocHandle(ENV) RetCode[%d]\n", RetCode);
		Disconnect();
		return ERROR_DB;
	}

	RetCode = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	if (CheckRetCode(RetCode, L"SQLSetEnvAttr") != 0)
	{
		g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"DBConnect Error SQLSetEnvAttr RetCode[%d]\n", RetCode);
		Disconnect();
		return ERROR_DB;
	}

	bool bConnected = false;
	for (size_t idx = 0; idx < nCandidateCount; ++idx)
	{
		pSelectedDriver = pCandidates[idx];
		RetCode = SQLAllocHandle(SQL_HANDLE_DBC, m_henv, &m_hdbc);
		if (CheckRetCode(RetCode, L"SQLAllocHandle") != 0)
		{
			g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"DBConnect Error SQLAllocHandle(DBC) RetCode[%d]\n", RetCode);
			Disconnect();
			return ERROR_DB;
		}

		RetCode = SQLSetConnectAttrW(m_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)(INT_PTR)nLoginTimeout, 0);
		if (CheckRetCode(RetCode, L"SQLSetConnectAttr(LOGIN_TIMEOUT)") != 0)
			g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"DBConnect Warning SQLSetConnectAttr(LOGIN_TIMEOUT) RetCode[%d]\n", RetCode);

		SQLSetConnectAttrW(m_hdbc, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER)(INT_PTR)nConnTimeout, 0);

		bool bIsLegacyDriver = (_wcsicmp(pSelectedDriver, L"SQL Server") == 0);
		memset(wszConnStrIn, 0, sizeof(wszConnStrIn));
		swprintf_s(wszConnStrIn, _countof(wszConnStrIn), L"Driver={%s};Server=%s;Database=%s;",
			pSelectedDriver, wszServerTarget, wszDatabase);

		if (bIsLegacyDriver)
		{
			wcsncat_s(wszConnStrIn, _countof(wszConnStrIn), L"Network=dbmssocn;", _TRUNCATE);
		}
		else
		{
			WCHAR wszTls[160] = { 0, };
			swprintf_s(wszTls, _countof(wszTls), L"Encrypt=%s;TrustServerCertificate=%s;", wszEncrypt, wszTrustCert);
			wcsncat_s(wszConnStrIn, _countof(wszConnStrIn), wszTls, _TRUNCATE);
			if (IsTrueValueW(wszMars))
				wcsncat_s(wszConnStrIn, _countof(wszConnStrIn), L"MARS_Connection=Yes;", _TRUNCATE);
			if (IsTrueValueW(wszMultiSubnet))
				wcsncat_s(wszConnStrIn, _countof(wszConnStrIn), L"MultiSubnetFailover=Yes;", _TRUNCATE);
		}

		if (bTrusted)
		{
			wcsncat_s(wszConnStrIn, _countof(wszConnStrIn), L"Trusted_Connection=Yes;", _TRUNCATE);
		}
		else
		{
			WCHAR wszAuth[800] = { 0, };
			swprintf_s(wszAuth, _countof(wszAuth), L"Uid=%s;Pwd=%s;", wszEscUid, wszEscPwd);
			wcsncat_s(wszConnStrIn, _countof(wszConnStrIn), wszAuth, _TRUNCATE);
		}

		if (wszAppName[0])
		{
			WCHAR wszApp[160] = { 0, };
			swprintf_s(wszApp, _countof(wszApp), L"APP=%s;", wszAppName);
			wcsncat_s(wszConnStrIn, _countof(wszConnStrIn), wszApp, _TRUNCATE);
		}
		if (wszWsid[0])
		{
			WCHAR wszHost[160] = { 0, };
			swprintf_s(wszHost, _countof(wszHost), L"WSID=%s;", wszWsid);
			wcsncat_s(wszConnStrIn, _countof(wszConnStrIn), wszHost, _TRUNCATE);
		}

		memset(wszConnStrOut, 0, sizeof(wszConnStrOut));
		cbConnStrOutMax = 0;
		RetCode = SQLDriverConnectW(m_hdbc, NULL, (SQLWCHAR*)wszConnStrIn, SQL_NTS, wszConnStrOut, _countof(wszConnStrOut), &cbConnStrOutMax, SQL_DRIVER_NOPROMPT);
		if (RetCode == SQL_SUCCESS || RetCode == SQL_SUCCESS_WITH_INFO)
		{
			bConnected = true;
			break;
		}

		if ((idx + 1 < nCandidateCount) && IsDriverMissingDiagW(m_hdbc))
		{
			SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
			m_hdbc = 0;
			continue;
		}
		break;
	}

	// Clear plaintext password buffer promptly from stack
	SecureZeroMemory(wszPwd, sizeof(wszPwd));
	SecureZeroMemory(wszEscPwd, sizeof(wszEscPwd));

	if (bConnected)
	{
		RetCode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);
		if (CheckRetCode(RetCode, L"SQLAllocHandle") == 0)
		{
			WCHAR wszQuery[128] = { 0, };
			swprintf_s(wszQuery, _countof(wszQuery), L"USE [%s]", wszDatabase);
			RetCode = SQLExecDirectW(m_hstmt, (SQLWCHAR*)wszQuery, SQL_NTS);

			if (RetCode == SQL_SUCCESS || RetCode == SQL_SUCCESS_WITH_INFO)
			{
				strncpy_s(m_szIp, _countof(m_szIp), szCallerIp, _TRUNCATE);
				m_nPort = nCallerPort;
				wcsncpy_s(m_wszDBName, _countof(m_wszDBName), wszCallerDB, _TRUNCATE);
				wcsncpy_s(m_wszID, _countof(m_wszID), wszCallerID, _TRUNCATE);
				g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L">> [T:%d, WorldID:%d] %s:%s (Driver: %s)\r\n",
					m_cThreadID, m_nWorldSetID, wszServerTarget, wszDatabase, pSelectedDriver);

				return ERROR_NONE;
			}
			else
			{
				CheckRetCode(RetCode, L"SQLExecDirect");
				g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"DBConnect Error SQLExecDirect RetCode[%d]\n", RetCode);
			}
		}
		else
			g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"DBConnect Error SQLAllocHandle(STMT) RetCode[%d]\n", RetCode);
	}
	else
	{
		SQLWCHAR sqlState[10] = { 0, };
		SQLWCHAR errMsg[512] = { 0, };
		SQLINTEGER nativeErr = 0;
		SQLSMALLINT msgLen = 0;
		for (SQLSMALLINT rec = 1; rec <= 3; ++rec)
		{
			if (SQLGetDiagRecW(SQL_HANDLE_DBC, m_hdbc, rec, sqlState, &nativeErr, errMsg, _countof(errMsg), &msgLen) != SQL_SUCCESS)
				break;
			g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"DBConnect Diag[%d] (%s) %ld: %s\n", rec, sqlState, nativeErr, errMsg);
		}
#if defined(PRE_FIX_SQLCONNECTFAIL_LOG)
		DisplaySQLConnectError();
#else
		g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"DBConnect Error SQLDriverConnect RetCode[%d] Driver[%s] Target[%s] DB[%s]\n", RetCode, pSelectedDriver, wszServerTarget, wszDatabase);
#endif
	}

	Disconnect();
	return ERROR_DB;
}

void CSQLConnection::Disconnect()
{
	if (m_hstmt){
		SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
		m_hstmt = 0;
	}
	if (m_hdbc){
		SQLDisconnect (m_hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
		m_hdbc = 0;
	}

	if (m_henv){
		SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
		m_henv = 0;
	}	
}

int CSQLConnection::CheckRetCode(SQLRETURN RetCode, WCHAR *pDesc)
{
	if (SQL_SUCCEEDED(RetCode) || RetCode == SQL_NEED_DATA || RetCode == SQL_NO_DATA){
		return ERROR_NONE;
	}

	DisplayInfo (RetCode, pDesc);

	return ERROR_DB; 
}

#if defined(PRE_FIX_SQLCONNECTFAIL_LOG)
int CSQLConnection::DisplaySQLConnectError()
{
	SQLINTEGER Error;
	SQLSMALLINT MsgLen;
	WCHAR InfoMsg[10] = { 0, }, ErrorMsg[1025] = { 0, }, SqlState[10] = { 0, };
	_wcscpy(InfoMsg, _countof(InfoMsg), L"SQLError", (int)wcslen(L"SQLError"));

	SQLRETURN sqlReturn = SQLGetDiagRec(SQL_HANDLE_DBC, m_hdbc, 1, SqlState, &Error, ErrorMsg, 1024, &MsgLen);
	if (sqlReturn == SQL_SUCCESS)
		g_Log.Log(LogType::_DBSYSTEM_ERROR, m_nWorldSetID, 0, 0, 0, L"SQLError #SQLDriveConnect# (%s) %ld, %s\r\n", SqlState, Error, ErrorMsg);
	else
		g_Log.Log(LogType::_DBSYSTEM_ERROR, m_nWorldSetID, 0, 0, 0, L"SQLError #SQLDriveConnect# (%s) unknown (%d)\r\n", SqlState, sqlReturn);

	return ERROR_DB;
}
#endif	// #if defined(PRE_FIX_SQLCONNECTFAIL_LOG)

void CSQLConnection::DisplayInfo(SQLRETURN RetCode, WCHAR *pDesc)
{
	SQLINTEGER Error;
	SQLSMALLINT MsgLen;
	WCHAR InfoMsg[10] = { 0, }, ErrorMsg[1025] = { 0, }, SqlState[10] = { 0, };

	if (SQL_SUCCEEDED(RetCode))
		_wcscpy(InfoMsg, _countof(InfoMsg), L"SQLInfo", (int)wcslen(L"SQLInfo"));
	else
		_wcscpy(InfoMsg, _countof(InfoMsg), L"SQLError", (int)wcslen(L"SQLError"));

	SQLRETURN sqlReturn = SQLGetDiagRec(SQL_HANDLE_STMT, m_hstmt, 1, SqlState, &Error, ErrorMsg, 1024, &MsgLen);
	if (sqlReturn == SQL_SUCCESS)
	{
		if (Error == 2601) return;

		g_Log.Log(LogType::_DBSYSTEM_ERROR, L"%s: #%s# (%s) %ld, %s\r\n", InfoMsg, pDesc, SqlState, Error, ErrorMsg);
		g_Log.Log(LogType::_DBSYSTEM_ERROR, L"Query:%s\r\n", m_wszQuery );

		if (Error == WSAECONNRESET || 
			wcscmp(SqlState, L"08S01") == 0 ||	// Disconnected state
			wcscmp(SqlState, L"24000") == 0)	// Invalid cursor state
		{
			Disconnect();	// Disconnect and reconnect
		}
	}
	else
	{
		g_Log.Log(LogType::_DBSYSTEM_ERROR, L"%s: #%s# (%s) unknown (%d)\r\n", InfoMsg, pDesc, SqlState, sqlReturn);
	}
}

int CSQLConnection::CheckConnect()
{
	if (m_hstmt == 0) return Reconnect();	// Reconnect if disconnected
	return ERROR_NONE;
}

int CSQLConnection::Reconnect()
{
	int nResult = Connect(m_szIp, m_nPort, m_wszDBName, m_wszID);
	if (nResult < 0)
		g_Log.Log(LogType::_FILELOG, m_nWorldSetID, 0, 0, 0, L"SQL Reconnect Error~! (T:%d W:%d (%S:%d) DBName:%s)\r\n", m_cThreadID, m_nWorldSetID, m_szIp, m_nPort, m_wszDBName);

	return nResult;
}

int CSQLConnection::CommonRowcountQuery(WCHAR *pQuery)
{
	if (CheckConnect() < 0){
		g_Log.Log(LogType::_ERROR, L"[CommonRowcountQuery] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	SQLRETURN RetCode;
	SQLLEN cblen;
	int nResult = 0;

	RetCode = SQLPrepare (m_hstmt, pQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		RetCode = SQLBindParameter (m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);	// Refers to the @@rowcount value

		RetCode = SQLExecute (m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{			
			if (nResult == ERROR_NONE_ROWCOUNT) return ERROR_NONE;	// Success
			// return ERROR_NONE;
			return nResult;			
		}
	}	
	return ERROR_DB;
}

int CSQLConnection::CommonResultQuery(WCHAR *pQuery)
{
	if (CheckConnect() < 0){
		g_Log.Log(LogType::_ERROR, L"[CommonResultQuery] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	SQLRETURN RetCode;
	SQLLEN cblen;
	int nResult = 0;

	RetCode = SQLPrepare (m_hstmt, pQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		RetCode = SQLBindParameter (m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);	// Refers to the return value

		RetCode = SQLExecute (m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){			
			// if (nResult == 0) return ERROR_DB;
			return nResult;
		}
	}	
	return ERROR_DB;
}

int CSQLConnection::CommonReturnValueQuery(WCHAR *pQuery)
{
	if (CheckConnect() < 0){
		g_Log.Log(LogType::_ERROR, L"[CommonReturnValueQuery] CheckConnect Fail\r\n");
		return ERROR_DB;
	}	

	SQLRETURN RetCode;
	SQLLEN cblen;
	int nResult = ERROR_DB;

	RetCode = SQLPrepare (m_hstmt, pQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		RetCode = SQLBindParameter (m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);	// Refers to the return value

		RetCode = SQLExecute (m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

// Replaces single quotes with two single quotes in mail body or title to prevent SQL query errors
void CSQLConnection::ConvertQuery(WCHAR *pSrc, int nSrcMaxSize, WCHAR *pResultStr, int nDesMaxSize)
{
	if( nSrcMaxSize * 2 + 1 >  nDesMaxSize )
	{
		g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"ConvertQuery MaxSize Error\r\n");
		return;
	}
	int nLen = (int)wcslen(pSrc);
	if (nSrcMaxSize < nLen) nLen = nSrcMaxSize;

	int j = 0;
	for (int i = 0; i < nLen; i++){
		if (pSrc[i] == '\'')
		{
			pResultStr[j] = pSrc[i];
			j++;			
			pResultStr[j] = pSrc[i];
		}
		else
		{
			pResultStr[j] = pSrc[i];
		}
		j++;		
	}

	if( j >= nDesMaxSize)
	{			
		j -= 1;		
	}

	pResultStr[j] = L'\0';
}

void CSQLConnection::CheckColumnCount(int nBindCount, const char* pszQuery)
{
	SQLSMALLINT nColCount = 0;
	SQLRETURN RetCode;
	RetCode = SQLNumResultCols(m_hstmt, &nColCount);
	if (CheckRetCode(RetCode, L"SQLNumResultCols") == ERROR_NONE)
	{
		if( nBindCount-1 != nColCount )
			g_Log.Log(LogType::_ERROR, L"[%S] BindCount:%d, ResultCount:%d\r\n", pszQuery, nBindCount, nColCount);
	}
}

#if defined(PRE_ADD_SQL_RENEW)
CSQLRecordSet::CSQLRecordSet(CSQLConnection* pConnection)
: m_pConnection(pConnection), m_bFetch(false), m_nBindCount(1)
{
	if ( m_pConnection && m_pConnection->CheckConnect() < 0){
		//g_Log.Log(LogType::_ERROR, L"[CommonReturnValueQuery] CheckConnect Fail\r\n");		
	}
	if( m_pConnection )
		m_hstmt = m_pConnection->GetStmt();
}

CSQLRecordSet::~CSQLRecordSet()
{
	m_vBindParam.clear();
	if( m_bFetch )
		SQLCloseCursor(m_hstmt);
}

void CSQLRecordSet::AddParamBit(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull)
{
	m_vBindParam.push_back(CParamODBC(type, SQL_C_BIT, SQL_BIT, sizeof(bool), pValue, bInputNull));
}

void CSQLRecordSet::AddParamByte(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull)
{
	m_vBindParam.push_back(CParamODBC(type, SQL_C_TINYINT, SQL_TINYINT, sizeof(BYTE), pValue, bInputNull));
}

void CSQLRecordSet::AddParamShort(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull)
{
	m_vBindParam.push_back(CParamODBC(type, SQL_C_SSHORT, SQL_SMALLINT, sizeof(short), pValue, bInputNull));
}

void CSQLRecordSet::AddParamInt(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull)
{
	m_vBindParam.push_back(CParamODBC(type, SQL_C_SLONG, SQL_INTEGER, sizeof(int), pValue, bInputNull));
}

void CSQLRecordSet::AddParamInt64(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull)
{
	m_vBindParam.push_back(CParamODBC(type, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), pValue, bInputNull)); 
}

void CSQLRecordSet::AddParamTimeStamp(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull)
{
	m_vBindParam.push_back(CParamODBC(type, SQL_C_TIMESTAMP, SQL_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), pValue, bInputNull)); 
}

void CSQLRecordSet::AddParamString(SQLSMALLINT type, SQLPOINTER pValue, int nSize, bool bInputNull)
{
	m_vBindParam.push_back(CParamODBC(type, SQL_C_CHAR, SQL_VARCHAR, nSize, pValue, bInputNull));
}

void CSQLRecordSet::AddParamWString(SQLSMALLINT type, SQLPOINTER pValue, int nSize, bool bInputNull)
{
	m_vBindParam.push_back(CParamODBC(type, SQL_C_WCHAR, SQL_WVARCHAR, nSize, pValue, bInputNull));
}

void CSQLRecordSet::AddParamBinary(SQLSMALLINT type, SQLPOINTER pValue, int nSize, bool bInputNull)
{
	m_vBindParam.push_back(CParamODBC(type, SQL_C_BINARY, SQL_BINARY, nSize, pValue, bInputNull));
}

void CSQLRecordSet::AddBindColBit(SQLPOINTER pValue)
{
	SQLLEN cblen;
	SQLBindCol(m_pConnection->GetStmt(), m_nBindCount++, SQL_BIT, pValue, sizeof(bool), &cblen);
}

void CSQLRecordSet::AddBindColByte(SQLPOINTER pValue)
{
	SQLLEN cblen;
	SQLBindCol(m_pConnection->GetStmt(), m_nBindCount++, SQL_TINYINT, pValue, sizeof(BYTE), &cblen);
}

void CSQLRecordSet::AddBindColShort(SQLPOINTER pValue)
{
	SQLLEN cblen;
	SQLBindCol(m_pConnection->GetStmt(), m_nBindCount++, SQL_SMALLINT, pValue, sizeof(short), &cblen);
}

void CSQLRecordSet::AddBindColInt(SQLPOINTER pValue)
{
	SQLLEN cblen;
	SQLBindCol(m_pConnection->GetStmt(), m_nBindCount++, SQL_INTEGER, pValue, sizeof(INT), &cblen);
}

void CSQLRecordSet::AddBindColInt64(SQLPOINTER pValue)
{
	SQLLEN cblen;
	SQLBindCol(m_pConnection->GetStmt(), m_nBindCount++, SQL_C_SBIGINT, pValue, sizeof(INT64), &cblen);
}

void CSQLRecordSet::AddBindColTimeStamp(SQLPOINTER pValue)
{
	SQLLEN cblen;
	SQLBindCol(m_pConnection->GetStmt(), m_nBindCount++, SQL_TIMESTAMP, pValue, sizeof(TIMESTAMP_STRUCT), &cblen);
}

void CSQLRecordSet::AddBindColString(SQLPOINTER pValue, int nSize)
{
	SQLLEN cblen;
	SQLBindCol(m_pConnection->GetStmt(), m_nBindCount++, SQL_CHAR, pValue, nSize, &cblen);
}

void CSQLRecordSet::AddBindColWString(SQLPOINTER pValue, int nSize)
{
	SQLLEN cblen;
	SQLBindCol(m_pConnection->GetStmt(), m_nBindCount++, SQL_WCHAR, pValue, nSize, &cblen);
}

bool CSQLRecordSet::Execute(const WCHAR* strQuery)
{
	if( !m_pConnection || m_pConnection->GetStmt() == 0)
		return false;

	SQLRETURN RetCode;	

	RetCode = SQLExecDirect(m_hstmt, (SQLTCHAR*)strQuery, SQL_NTS);
	if( m_pConnection->CheckRetCode( RetCode, L"SQLExecDirect" ) != ERROR_NONE )	
		return false;

	return true;
}

bool CSQLRecordSet::Execute_Procedure(const WCHAR* strProcName, bool bReturn)
{
	if( !m_pConnection || m_pConnection->GetStmt() == 0)
		return false;

	WCHAR			strQuery[4096];
	SQLRETURN		RetCode;
	int				nCount = 1;	

	if (!m_vBindParam.empty())
	{
		BIND_PARAM_ARRAY_IT	it;

		for (it = m_vBindParam.begin(); it != m_vBindParam.end(); it++)
		{
			if((*it).m_bNull)			
				(*it).m_StrLenIndPtr = SQL_NULL_DATA;
			else if( (*it).m_sValueType == SQL_C_CHAR)
				(*it).m_StrLenIndPtr = SQL_NTS;
			else if( (*it).m_sValueType == SQL_C_WCHAR)
				(*it).m_StrLenIndPtr = SQL_NTSL;

			RetCode = SQLBindParameter(m_hstmt, nCount++, (*it).m_sType, (*it).m_sValueType, 
				(*it).m_sParamType, (*it).m_unColumnSize, 0, (*it).m_pValue, 0, &(*it).m_StrLenIndPtr);

			if (RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO)
				return false;

			if (it == m_vBindParam.begin())
			{
				if (bReturn)
					swprintf(strQuery,4095, L"{ ? = CALL %s(", strProcName);
				else
					swprintf(strQuery,4095, L"{ CALL %s(?, ", strProcName);
			}
			else
				wcscat(strQuery, _T("?, "));
		}

		if (strQuery[wcslen(strQuery) - 2] == L',')
			wcscpy(&strQuery[wcslen(strQuery) - 2], L") }");
		else
			wcscat(strQuery, _T(") }"));
	}
	else
		swprintf(strQuery, 4095, L"{ CALL %s() }", strProcName);

	RetCode = SQLPrepare( m_hstmt, strQuery, SQL_NTSL );
	if( m_pConnection->CheckRetCode( RetCode, L"SQLPrepare" ) == ERROR_NONE )
	{
		RetCode = SQLExecute( m_hstmt );
		if( m_pConnection->CheckRetCode( RetCode, L"SQLExecute" ) != ERROR_NONE )
			return false;
	}
	return true;
}

bool CSQLRecordSet::Fetch()
{
	SQLRETURN		RetCode;
	RetCode = SQLFetch(m_pConnection->GetStmt());
	if( m_pConnection->CheckRetCode( RetCode, L"SQLFetch" ) != ERROR_NONE || RetCode == SQL_NO_DATA )
		return false;
	m_bFetch = true;
	return true;
}
#endif //#if defined(PRE_ADD_SQL_RENEW)
