#pragma once

const USHORT QUERYSIZEMAX = 20480;


// DESC : Set a default value for the last argument 'StrLen_or_IndPtr' of SQLBindParameter(...) for input/output parameters of a specific procedure based on conditions
// Caution!> For input parameters needing default values (NULL, DEFAULT), each input parameter variable must be assigned individually (1 per parameter)
class CDNSqlLen
{
public:
	CDNSqlLen() : m_nSqlLen(0)
	{

	}
	SQLLEN* Get(SQLLEN pSize)
	{
		DN_ASSERT(0 == m_nSqlLen,	"Check!");
		return(&(m_nSqlLen = pSize));
	}
	SQLLEN* GetNull(bool pCond, SQLLEN pSize)
	{
		DN_ASSERT(0 == m_nSqlLen,	"Check!");
		return(&(m_nSqlLen = (pCond)?(SQL_NULL_DATA):(pSize)));
	}
	SQLLEN* GetDefault(bool pCond, SQLLEN pSize)
	{
		DN_ASSERT(0 == m_nSqlLen,	"Check!");
		return(&(m_nSqlLen = (pCond)?(SQL_DEFAULT_PARAM):(pSize)));
	}
private:
	SQLLEN m_nSqlLen;
};


class CSQLConnection
{
private:
	char m_szIp[IPLENMAX];
	int m_nPort;
	WCHAR m_wszDBName[DBNAMELENMAX];
	WCHAR m_wszID[IDLENMAX];
	LONG m_RefCount;


protected:
	SQLHENV m_henv;
	SQLHDBC m_hdbc;
	SQLHSTMT m_hstmt;

	WCHAR m_wszQuery[QUERYSIZEMAX];

	int Reconnect();

	int CommonRowcountQuery(WCHAR *pQuery);	// Call for general common use (return value is @@rowcount)
	int CommonResultQuery(WCHAR *pQuery);	// Call for general common use (receive standard return value)
	int CommonReturnValueQuery(WCHAR *pQuery);	// Call for general common use (receive standard return value); use this from Doyeol's renewal onward

public:
	BYTE m_cThreadID;
	int m_nWorldSetID;

public:
	CSQLConnection(void);
	virtual ~CSQLConnection(void);

	SQLHSTMT GetStmt() { return m_hstmt;};	

	int CheckConnect();

	int Connect(char *pIp, int nPort, WCHAR *pwszDBName, WCHAR *pwszID);
	void Disconnect();

	int CheckRetCode(SQLRETURN RetCode, WCHAR *pDesc);
#if defined(PRE_FIX_SQLCONNECTFAIL_LOG)
	int DisplaySQLConnectError();
#endif	// #if defined(PRE_FIX_SQLCONNECTFAIL_LOG)

	void DisplayInfo(SQLRETURN RetCode, WCHAR *pDesc);

	LONG IncRefCount() { return(InterlockedIncrement(&m_RefCount)); }
	LONG DecRefCount() { return(InterlockedDecrement(&m_RefCount)); }

	// Replaces single quotes with two single quotes in mail body or title to prevent SQL query errors
	void ConvertQuery(WCHAR *pSrc, int nSrcMaxSize, WCHAR *pResultStr, int nDesMaxSize);
	void CheckColumnCount(int nBindCount, const char* pszQuery);

};

#if defined(PRE_ADD_SQL_RENEW)

class CParamODBC
{
public:
	SQLSMALLINT			m_sType;
	SQLSMALLINT			m_sValueType;
	SQLSMALLINT			m_sParamType;
	SQLPOINTER			m_pValue;
	SQLUINTEGER			m_unColumnSize;
	SQLLEN				m_StrLenIndPtr;
	bool				m_bNull;

public:
	CParamODBC(SQLSMALLINT sType, SQLSMALLINT sValueType, SQLSMALLINT sParamType, SQLUINTEGER unColumnSize, SQLPOINTER pValue, bool bNull)
	{
		m_sType			= sType;
		m_sValueType	= sValueType;
		m_sParamType	= sParamType;
		m_unColumnSize	= unColumnSize;
		m_pValue		= pValue;
		m_bNull			= bNull;
		m_StrLenIndPtr  = 0;
	}
};

typedef std::vector<CParamODBC>				BIND_PARAM_ARRAY;
typedef std::vector<CParamODBC>::iterator	BIND_PARAM_ARRAY_IT;

class CSQLConnection;

class CSQLRecordSet
{
private :
	CSQLConnection* m_pConnection;	
	BIND_PARAM_ARRAY m_vBindParam;
	SQLHSTMT m_hstmt;
	bool	m_bFetch;
	int		m_nBindCount;

public :
	CSQLRecordSet(CSQLConnection* pConnection);
	~CSQLRecordSet();
	//AddParam --> Excute_Procedure --> Bind --> Fetch
	void	AddParamBit(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull=false);
	void	AddParamByte(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull=false);
	void	AddParamShort(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull=false);
	void	AddParamInt(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull=false);
	void	AddParamInt64(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull=false);
	void	AddParamTimeStamp(SQLSMALLINT type, SQLPOINTER pValue, bool bInputNull=false);
	void	AddParamString(SQLSMALLINT type, SQLPOINTER pValue, int nSize, bool bInputNull=false);
	void	AddParamWString(SQLSMALLINT type, SQLPOINTER pValue, int nSize, bool bInputNull=false);
	void	AddParamBinary(SQLSMALLINT type, SQLPOINTER pValue, int nSize, bool bInputNull=false);

	bool	Execute(const WCHAR *strQuery);
	bool	Execute_Procedure(const WCHAR *strProcName, bool bReturn);

	void	AddBindColBit(SQLPOINTER pValue);
	void	AddBindColByte(SQLPOINTER pValue);
	void	AddBindColShort(SQLPOINTER pValue);
	void	AddBindColInt(SQLPOINTER pValue);
	void	AddBindColInt64(SQLPOINTER pValue);
	void	AddBindColTimeStamp(SQLPOINTER pValue);
	void	AddBindColString(SQLPOINTER pValue, int nSize);
	void	AddBindColWString(SQLPOINTER pValue, int nSize);

	bool	Fetch();
};
#endif //#if defined(PRE_ADD_SQL_RENEW)


inline __time32_t _dbStamptoTime32_t(TIMESTAMP_STRUCT &dbtime)
{
	struct tm Tm;
	Tm.tm_year = dbtime.year - 1900;	//struct tm years since 1900
	Tm.tm_mon = dbtime.month - 1;		//starting at 0
	Tm.tm_mday = dbtime.day;
	Tm.tm_hour = dbtime.hour;
	Tm.tm_min = dbtime.minute;
	Tm.tm_sec = dbtime.second;
	Tm.tm_wday  = 0;
	Tm.tm_yday  = 0;
	Tm.tm_isdst = 0;

	return _mktime32(&Tm);
}

inline __time64_t _dbStamptoTime64_t(TIMESTAMP_STRUCT &dbtime)
{
	struct tm Tm;
	Tm.tm_year = dbtime.year - 1900;	//struct tm years since 1900
	Tm.tm_mon = dbtime.month - 1;		//starting at 0
	Tm.tm_mday = dbtime.day;
	Tm.tm_hour = dbtime.hour;
	Tm.tm_min = dbtime.minute;
	Tm.tm_sec = dbtime.second;
	Tm.tm_wday  = 0;
	Tm.tm_yday  = 0;
	Tm.tm_isdst = 0;

	return _mktime64(&Tm);
}
