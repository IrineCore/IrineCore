#include "stdafx.h"
#include "DNSQLMembership.h"
#include "DNExtManager.h"
#include "Log.h"
#include "Util.h"
#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)
#include "DNLimitedCashItemRepository.h"
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)

CDNSQLMembership::CDNSQLMembership(void): CSQLConnection()
{
}

CDNSQLMembership::~CDNSQLMembership(void)
{
}

int CDNSQLMembership::QueryGetDatabaseVersion( TDatabaseVersion* pA )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetDatabaseVersion" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log( LogType::_ERROR, L"[QueryGetDatabaseVersion] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetDatabaseVersion}" );
 
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		SQLBindParameter( m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen );

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			int nNo = 1;			
			SQLBindCol( m_hstmt, nNo++, SQL_WCHAR, pA->wszName,	sizeof(pA->wszName), &cblen );
			SQLBindCol( m_hstmt, nNo++, SQL_INTEGER, &pA->iVersion, sizeof(int), &cblen );
			CheckColumnCount(nNo, "[Membership]P_GetDatabaseVersion");
			RetCode = SQLFetch(m_hstmt);
		}	
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

#if defined(_LOGINSERVER)
int CDNSQLMembership::QueryLogin(CDNUserConnection *pUserCon, WCHAR *pPassword, BYTE cStep, OUT Login::TQueryLoginOutput &LoginOutput)
{
	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, pUserCon, L"[QueryLogin] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	int nPublisher = DBDNWorldDef::PublisherCode::Studio;

	int nSize = (int)sizeof(TGameOptions);
	BYTE cNewAccount = 0;

	__time64_t BirthTime;
	time(&BirthTime);
	tm *tm_ptr = localtime(&BirthTime);

	BYTE szMID[MACHINEIDMAX] = {0,};
	int nMachineIDSize = sizeof(szMID);

	char cCreateCharMax = 0;
#if defined(PRE_MOD_SELECT_CHAR)
	cCreateCharMax = pUserCon->GetAvailableCharacterSlotCount();
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	cCreateCharMax = pUserCon->GetCharacterCountMax();
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	int nResult = ERROR_DB;

	SQLRETURN RetCode;
	SQLLEN cblen;

	CDNSqlLen SqlLen;
	CDNSqlLen SqlLenTime;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
#if defined(PRE_ADD_LOGINLOGOUT_LOG)

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	swprintf(m_wszQuery, L"{?=CALL dbo.P_Login(N'%s',N'%s',%d,%d,%d,%d,%d,'%d',N'%s',?,?,?,?,?,?,?,%d,?,%d,?,1,?,?,?,?,?,?)}", 
		pUserCon->GetAccountName(), pUserCon->GetwszIp(), pUserCon->GetSessionID(), nPublisher, DailyCreateCharacterMax, cCreateCharMax, pUserCon->m_cGender, (1900 + tm_ptr->tm_year) - pUserCon->GetAge(), pPassword, pUserCon->m_cRegion, cStep);
#else		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	swprintf(m_wszQuery, L"{?=CALL dbo.P_Login(N'%s',N'%s',%d,%d,%d,%d,%d,'%d',N'%s',?,?,?,?,?,?,?,%d,?,%d,?,0,?,?,?,?,?,?)}", 
		pUserCon->GetAccountName(), pUserCon->GetwszIp(), pUserCon->GetSessionID(), nPublisher, DailyCreateCharacterMax, cCreateCharMax, pUserCon->m_cGender, (1900 + tm_ptr->tm_year) - pUserCon->GetAge(), pPassword, pUserCon->m_cRegion, cStep);
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP

#else	

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	swprintf(m_wszQuery, L"{?=CALL dbo.P_Login(N'%s',N'%s',%d,%d,%d,%d,%d,'%d',N'%s',?,?,?,?,?,?,?,%d,?,%d,?,1,?,?,?,?,?,?)}", 
		pUserCon->GetAccountName(), pUserCon->GetwszIp(), pUserCon->GetSessionID(), nPublisher, DailyCreateCharacterMax, cCreateCharMax, pUserCon->m_cGender, (1900 + tm_ptr->tm_year) - pUserCon->GetAge(), pPassword, pUserCon->m_cRegion, 2);
#else		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	swprintf(m_wszQuery, L"{?=CALL dbo.P_Login(N'%s',N'%s',%d,%d,%d,%d,%d,'%d',N'%s',?,?,?,?,?,?,?,%d,?,%d,?,0,?,?,?,?,?,?)}", 
		pUserCon->GetAccountName(), pUserCon->GetwszIp(), pUserCon->GetSessionID(), nPublisher, DailyCreateCharacterMax, cCreateCharMax, pUserCon->m_cGender, (1900 + tm_ptr->tm_year) - pUserCon->GetAge(), pPassword, pUserCon->m_cRegion, 2);
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP

#endif	// #if defined(PRE_ADD_LOGINLOGOUT_LOG)

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		UINT nUserNo = 0;
		BYTE cCharCountMax = 0;

		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(BYTE), 0, &cNewAccount, sizeof(BYTE), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(UINT), 0, &LoginOutput.nAccountDBID, sizeof(UINT), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &pUserCon->m_cAccountLevel, sizeof(char), &cblen);
		cblen = SQL_LEN_DATA_AT_EXEC(nSize);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_BINARY, SQL_BINARY, sizeof(TGameOptions), 0, &pUserCon->m_GameOptions, sizeof(TGameOptions), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(BYTE), 0, &pUserCon->m_cDailyCreateCount, sizeof(BYTE), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(BYTE), 0, &cCharCountMax, sizeof(BYTE), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_BINARY, SQL_BINARY, sizeof(szMID), 0, szMID, sizeof(szMID), SqlLen.Get(nMachineIDSize));

		CDNSqlLen PartnerCodeLen;
		BYTE cPartnerID = 0;
		cPartnerID = static_cast<BYTE>(pUserCon->GetPartnerType());
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cPartnerID, sizeof(char), PartnerCodeLen.GetNull(cPartnerID<=0,sizeof(char)));
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(UINT), 0, &nUserNo, sizeof(UINT), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &LoginOutput.LogOutDate, sizeof(TIMESTAMP_STRUCT), SqlLenTime.Get(SQL_TIMESTAMP_LEN));
		bool bAuthMobile;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_BIT, SQL_BIT, sizeof(bool), 0, &bAuthMobile, sizeof(bool), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(BYTE), 0, &pUserCon->m_cCharacterSortCode, sizeof(BYTE), &cblen);

		bool bReConnectReward = false;
		TIMESTAMP_STRUCT RegistDate;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_BIT, SQL_BIT, sizeof(bool), 0, &bReConnectReward, sizeof(bool), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &RegistDate, sizeof(TIMESTAMP_STRUCT), &cblen);

		bool bComebakcUser = false;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_BIT, SQL_BIT, sizeof(bool), 0, &bComebakcUser, sizeof(bool), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
#if !defined(PRE_MOD_SELECT_CHAR)
			pUserCon->SetCharacterCountMax(cCharCountMax);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
			LoginOutput.IsNewAccount = cNewAccount == 0 ? false : true; 
#ifdef PRE_ADD_DOORS
			pUserCon->m_bMobileAuthentication = bAuthMobile;
#endif		//#ifdef PRE_ADD_DOORS
#if defined(PRE_ADD_GAMEQUIT_REWARD)
			pUserCon->m_bReConnectNewbieReward = !bReConnectReward; // In DB, 1(true) means received, 0(false) means not received; exceptionally, NULL (=existing user) is treated as received
			LoginOutput.RegistDate = RegistDate;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
#if defined( PRE_ADD_NEWCOMEBACK )
			pUserCon->m_bComebackUser = bComebakcUser;
#endif
		}
	}

	return nResult;
}
#endif	// #if defined(_LOGINSERVER)

int CDNSQLMembership::QueryGetCharacterCount( WCHAR *pAccountName, std::map<int,int>& mWorldCharCount )
{
	if (CheckConnect() < ERROR_NONE)
	{
		g_Log.Log(LogType::_ERROR, L"[QueryGetCharacterCount] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset( m_wszQuery, 0, sizeof(m_wszQuery) );
	swprintf( m_wszQuery, L"{?=CALL dbo.P_GetCharacterCount(N'%s',%d)}", pAccountName, DELETECHAR_WAITTIME_MINUTE );

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		SQLBindParameter( m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen );	

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			int iWorldID	= 0;
			int iCharCount	= 0;

			int nNo = 1;
			SQLBindCol( m_hstmt, nNo++, SQL_INTEGER, &iWorldID,	  sizeof(int), &cblen );
			SQLBindCol( m_hstmt, nNo++, SQL_INTEGER, &iCharCount, sizeof(int), &cblen );
			CheckColumnCount(nNo, "P_GetCharacterCount");
			while(1)
			{
				iWorldID = 0;
				iCharCount = 0;

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA)
				{					
					nResult = ERROR_NONE;
					break;
				}

				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO) )
					break;

				mWorldCharCount.insert( std::make_pair(iWorldID,iCharCount) );
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryLogout(UINT nAccountDBID, UINT nSessionID, const BYTE * pMachineID/* = NULL*/)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_Logout" );

	if (CheckConnect() < ERROR_NONE)
	{
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, nSessionID, L"[QueryLogout] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB, sqlparam = 0;

	CDNSqlLen SqlLen;
	BYTE szMID[MACHINEIDMAX] = {0,};
	if (pMachineID) memcpy_s(szMID, sizeof(szMID), pMachineID, MACHINEIDMAX);
	int nMachineIDSize = sizeof(szMID);

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_Logout(%d,%d,?)}", nAccountDBID, nSessionID);
	
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER,	SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int),	&cblen );
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_BINARY, SQL_BINARY, sizeof(szMID), 0, szMID, sizeof(szMID), SqlLen.Get(nMachineIDSize));

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") != ERROR_NONE)
			return ERROR_DB;
	}

	return nResult;
}

int CDNSQLMembership::QueryModLastConnectDate(UINT nAccountDBID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModLastConnectDate" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModLastConnectDate(%d)}", nAccountDBID);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryAddAccountKey(UINT nAccountDBID, UINT nUserNo)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddAccountKey" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddAccountKey(%d,%d)}", nAccountDBID, nUserNo);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryAddRestraint( DBDNWorldDef::RestraintTargetCode::eCode TargetCode, UINT uiAccountDBID, INT64 biCharacterDBID, int iReasonID, DBDNWorldDef::RestraintTypeCode::eCode TypeCode, WCHAR* pwszMemo, WCHAR* pwszRestraintMsg, TIMESTAMP_STRUCT StartDate, TIMESTAMP_STRUCT EndDate )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddRestraint" );

	if (CheckConnect() < ERROR_NONE)
	{
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, biCharacterDBID, 0, L"[QueryAddRestraint] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	CDNSqlLen SqlLen[2];

	int nResult = ERROR_DB, sqlparam = 0, nRestraintID = 0;
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddRestraint(%d,%d,%I64d,%d,%d,%d,N'%s',N'%s',?,?,?)}", TargetCode, uiAccountDBID, biCharacterDBID, iReasonID, TypeCode, 0, pwszMemo ? pwszMemo : L"", pwszRestraintMsg ? pwszRestraintMsg : L"" );
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER,	SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int),	&cblen );
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT,  SQL_C_TIMESTAMP, SQL_TIMESTAMP,	SQL_TIMESTAMP_LEN, 0, &StartDate, sizeof(TIMESTAMP_STRUCT), SqlLen[0].Get(SQL_TIMESTAMP_LEN) );
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT,  SQL_C_TIMESTAMP, SQL_TIMESTAMP,	SQL_TIMESTAMP_LEN, 0, &EndDate, sizeof(TIMESTAMP_STRUCT), SqlLen[1].Get(SQL_TIMESTAMP_LEN) );
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER,	SQL_INTEGER, sizeof(int), 0, &nRestraintID, sizeof(int),	&cblen );

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") != ERROR_NONE)
			return ERROR_DB;
	}

	return nResult;
}

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
int CDNSQLMembership::QueryAddBlockedIP(int nWorldID, UINT nAccountDBID, const char * pszIP)
{
	CQueryTimeLog QueryTimeLog("P_AddBlockedIP");

	if(CheckConnect() < 0) {
		g_Log.Log(LogType::_ERROR, nWorldID, nAccountDBID, 0, 0, L"[QueryAddBlockedIP] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;	// ODBC API call return value
	SQLLEN cbLen;
	SQLLEN cbNull;
	cbNull = SQL_NULL_DATA;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddBlockedIP(N'%S',N'%S',255,N'',0,?,?)}", pszIP, pszIP);

	int nResult = ERROR_DB;	
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if(CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE) 
	{
		SQL_TIMESTAMP_STRUCT JoinDate;
		int nNo = 1;

		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cbLen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, sizeof(SQL_TIMESTAMP_STRUCT), 0, &JoinDate, sizeof(SQL_TIMESTAMP_STRUCT), &cbNull);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, sizeof(SQL_TIMESTAMP_STRUCT), 0, &JoinDate, sizeof(SQL_TIMESTAMP_STRUCT), &cbNull);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryGetSimpleConfigValue(TASimpleConfig * pSimpleConfig)
{
	if (pSimpleConfig == NULL)
		return ERROR_DB;

	CQueryTimeLog QueryTimeLog( "[Membership]P_GetListSimpleConfigValue" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[QueryGetSimpleConfigValue] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListSimpleConfigValue}");

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		SQLBindParameter( m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen );	

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			SimpleConfig::ConfigData ConfigData;

			int nNo = 1;
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &ConfigData.cCode, sizeof(char), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &ConfigData.nValue, sizeof(int), &cblen);			
			CheckColumnCount(nNo, "[Membership]P_GetListSimpleConfigValue");
			while(1)
			{
				memset(&ConfigData, 0, sizeof(SimpleConfig::ConfigData));

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA){					
					nResult = ERROR_NONE;
					break;
				}

				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				pSimpleConfig->Configs[pSimpleConfig->cCount] = ConfigData;
				pSimpleConfig->cCount++;

				if (pSimpleConfig->cCount >= SimpleConfig::ConfigDataCountMax)
				{
					nResult = ERROR_NONE;
					break;
				}
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP

#ifdef PRE_ADD_SEETMAINTENANCEFLAG
int CDNSQLMembership::QuerySetMaintenanceFlag(int nFlag)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_DRS_ModMaintenanceFlag" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_DRS_ModMaintenanceFlag(%d)}", nFlag);

	return CommonResultQuery(m_wszQuery);
}
#endif		//#ifdef PRE_ADD_SEETMAINTENANCEFLAG

int CDNSQLMembership::QueryGetListRestraintForAccount(UINT nAccountDBID, OUT std::vector<TRestraintForAccountAndCharacter> &VecRestraint)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetListRestraintForAccount" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryLogin] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	VecRestraint.clear();

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListRestraintForAccount(%d)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		SQLBindParameter( m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen );	

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			TRestraintForAccountAndCharacter Restraint = { 0, };

			TIMESTAMP_STRUCT StartDate, EndDate;

			int nNo = 1;
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &Restraint.cLevelCode, sizeof(char), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Restraint.iReasonID, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, Restraint.wszRestraintReason, sizeof(WCHAR) * RESTRAINTREASONMAX, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &StartDate, sizeof(TIMESTAMP_STRUCT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &EndDate, sizeof(TIMESTAMP_STRUCT), &cblen);
			CheckColumnCount(nNo, "[Membership]P_GetListRestraintForAccount");
			while(1)
			{				
				memset(&Restraint, 0, sizeof(Restraint));
				memset(&StartDate, 0, sizeof(StartDate));
				memset(&EndDate, 0, sizeof(EndDate));

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA){					
					nResult = ERROR_NONE;
					break;
				}

				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				CTimeParamSet StartTime(&QueryTimeLog, Restraint._tStartDate, StartDate, 0, nAccountDBID);
				CTimeParamSet EndTime(&QueryTimeLog, Restraint._tEndDate, EndDate, 0, nAccountDBID);

				if (!StartTime.IsValid())
				{
					nResult = ERROR_DB;
					break;				
				}
				if (!EndTime.IsValid())
				{
					nResult = ERROR_DB;
					break;									
				}

				VecRestraint.push_back(Restraint);
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryGetListRestraintForCharacter(INT64 biCharacterDBID, OUT std::vector<TRestraintForAccountAndCharacter> &VecRestraint)
{
	CQueryTimeLog QueryTimeLog( "[Membership]QueryGetListRestraintFotCharacter" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryLogin] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	VecRestraint.clear();

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListRestraintForCharacter(%I64d)}", biCharacterDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		SQLBindParameter( m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen );	

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			TRestraintForAccountAndCharacter Restraint = { 0, };

			TIMESTAMP_STRUCT StartDate, EndDate;

			int nNo = 1;
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &Restraint.cLevelCode, sizeof(char), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Restraint.iReasonID, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, Restraint.wszRestraintReason, sizeof(WCHAR) * RESTRAINTREASONMAX, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &StartDate, sizeof(TIMESTAMP_STRUCT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &EndDate, sizeof(TIMESTAMP_STRUCT), &cblen);
			CheckColumnCount(nNo, "[Membership]P_GetListRestraintForCharacter");
			while(1)
			{				
				memset(&Restraint, 0, sizeof(Restraint));
				memset(&StartDate, 0, sizeof(StartDate));
				memset(&EndDate, 0, sizeof(EndDate));

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA){					
					nResult = ERROR_NONE;
					break;
				}

				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				CTimeParamSet StartTime(&QueryTimeLog, Restraint._tStartDate, StartDate, 0, 0, biCharacterDBID);
				CTimeParamSet EndTime(&QueryTimeLog, Restraint._tEndDate, EndDate, 0, 0, biCharacterDBID);

				if (!StartTime.IsValid())
				{
					nResult = ERROR_DB;
					break;									
				}
				if (!EndTime.IsValid())
				{
					nResult = ERROR_DB;
					break;									
				}

				VecRestraint.push_back(Restraint);
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryGetGameOption(UINT nAccountDBID, OUT TGameOptions &Options)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetGameOption" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryGetGameOption] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	int nSize = (int)sizeof(TGameOptions);

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetGameOption(%d,?)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		cblen = SQL_LEN_DATA_AT_EXEC(nSize);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_BINARY, SQL_BINARY, sizeof(TGameOptions), 0, &Options, sizeof(TGameOptions), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}
	return nResult;
}

int CDNSQLMembership::QueryModGameOption(UINT nAccountDBID, TGameOptions &Options)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModGameOption" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryModGameOption] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	int nSize = sizeof(TGameOptions);

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModGameOption(%d,?)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		cblen = SQL_LEN_DATA_AT_EXEC(nSize);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_BINARY, SQL_BINARY, nSize, 0, &Options, nSize, &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			SQLPOINTER pParam;
			while(RetCode == SQL_NEED_DATA)
			{
				RetCode = SQLParamData(m_hstmt, &pParam);
				if (RetCode == SQL_NEED_DATA)
				{
					if (pParam == &Options)
						SQLPutData(m_hstmt, pParam, nSize);
				}
			}
		}
	}

	return nResult;
}

int CDNSQLMembership::QueryAddCharacter(UINT nAccountDBID, WCHAR *pCharName, int nWorldID, int nDefaultMaxCharacterCountPerAccount, OUT INT64 &biCharacterDBID, OUT TIMESTAMP_STRUCT &CreateDate)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddCharacter" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryAddCharacter] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
#if defined(PRE_MOD_SELECT_CHAR)
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddCharacter(%d,N'%s',%d,%d,?,%d,%d,?)}", nAccountDBID, pCharName, nWorldID, DELETECHAR_WAITTIME_MINUTE, 1, nDefaultMaxCharacterCountPerAccount);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddCharacter(%d,N'%s',%d,%d,?)}", nAccountDBID, pCharName, nWorldID, DELETECHAR_WAITTIME_MINUTE );
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biCharacterDBID, sizeof(INT64), &cblen);	
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), 0, &CreateDate, sizeof(TIMESTAMP_STRUCT), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryRollbackAddCharacter(INT64 biCharacterDBID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_RollbackAddCharacter" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_RollbackAddCharacter(%I64d)}", biCharacterDBID);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryDelCharacter(INT64 biCharacterDBID, WCHAR *pPrivateIp, WCHAR *pPublicIp, bool bFirstVillage, TIMESTAMP_STRUCT& DeleteDate )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_DelCharacter" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryDelCharacter] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	CDNSqlLen SqlLen;

	int nResult = ERROR_DB, sqlparam = 0;
	swprintf(m_wszQuery, L"{?=CALL dbo.P_DelCharacter(%I64d,N'%s',N'%s',%d,?)}", biCharacterDBID, pPrivateIp, pPublicIp, bFirstVillage);
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER,	SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen );
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &DeleteDate, sizeof(TIMESTAMP_STRUCT), SqlLen.Get(SQL_TIMESTAMP_LEN));

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") != ERROR_NONE)
			return ERROR_DB;
	}
	return nResult;
}

int CDNSQLMembership::QueryReviveCharacter( INT64 biCharacterDBID, WCHAR* wszPrivateIP, WCHAR* wszPublicIP )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ReviveCharacter" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_ReviveCharacter(%I64d,N'%s',N'%s')}", biCharacterDBID, wszPrivateIP, wszPublicIP );

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryGetKeySettingOption( UINT uiAccountDBID, TKeySetting* pKeySetting )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetKeySettingOption" );

	if ( CheckConnect() < ERROR_NONE )
	{
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, 0, L"[QueryGetKeySettingOption] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	int iRet = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetKeySettingOption(%d,?)}", uiAccountDBID );

	SQLRETURN RetCode = SQLPrepare( m_hstmt, m_wszQuery, SQL_NTSL );
	if ( CheckRetCode( RetCode, L"SQLPrepare" ) == ERROR_NONE )
	{
		SQLLEN cblen;
		int iSize = sizeof(TKeySetting);

		int nNo = 1;
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &iRet, sizeof(int), &cblen );
		cblen = SQL_LEN_DATA_AT_EXEC(iSize);
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_BINARY, SQL_BINARY, iSize, 0, pKeySetting, iSize, &cblen );

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return iRet;
}

int CDNSQLMembership::QueryModKeySettingOption( TQModKeySettingOption* pQ, TAModKeySettingOption* pA )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModKeySettingOption" );

	if ( CheckConnect() < ERROR_NONE )
	{
		g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, 0, 0, "[QueryModKeySettingOption] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	int iRet = ERROR_DB;

	memset( &m_wszQuery, 0, sizeof(m_wszQuery) );
	swprintf( m_wszQuery, L"{?=CALL dbo.P_ModKeySettingOption(%d,?)}", pQ->nAccountDBID );

	SQLRETURN RetCode = SQLPrepare( m_hstmt, m_wszQuery, SQL_NTSL );
	if ( CheckRetCode( RetCode, L"SQLPrepare" ) == ERROR_NONE )
	{
		SQLLEN cblen;
		int iSize = sizeof(TKeySetting);

		int nNo = 1;
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &iRet, sizeof(int), &cblen );
		cblen	= SQL_LEN_DATA_AT_EXEC(iSize);
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_BINARY, SQL_BINARY, iSize, 0, &pQ->sKeySetting, iSize, &cblen );

		RetCode = SQLExecute(m_hstmt);
		if ( CheckRetCode( RetCode, L"SQLExecute" ) == ERROR_NONE )
		{
			SQLPOINTER pParam;
			while( RetCode == SQL_NEED_DATA )
			{
				RetCode = SQLParamData( m_hstmt, &pParam );
				if ( RetCode == SQL_NEED_DATA )
				{
					if ( pParam == &pQ->sKeySetting )
					{
						if ( pQ->bUseDefault )
						{
							SQLPutData( m_hstmt, pParam, SQL_NULL_DATA );
						}
						else
						{
							SQLPutData( m_hstmt, pParam, iSize );
						}
					}
				}
			}
		}
	}

	return iRet;
}

int CDNSQLMembership::QueryGetPadSettingOption( UINT uiAccountDBID, TPadSetting* pPadSetting )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetGamePadOption" );

	if ( CheckConnect() < ERROR_NONE )
	{
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, 0, L"[QueryGetPadSettingOption] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	int iRet = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetGamePadOption(%d,?)}", uiAccountDBID );

	SQLRETURN RetCode = SQLPrepare( m_hstmt, m_wszQuery, SQL_NTSL );
	if( CheckRetCode( RetCode, L"SQLPrepare" ) == ERROR_NONE )
	{
		SQLLEN cblen;
		int iSize = sizeof(TPadSetting);

		int nNo = 1;
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &iRet, sizeof(int), &cblen );
		cblen	= SQL_LEN_DATA_AT_EXEC(iSize);
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_BINARY, SQL_BINARY, iSize, 0, pPadSetting, iSize, &cblen );

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return iRet;
}

int CDNSQLMembership::QueryModPadSettingOption( TQModPadSettingOption* pQ, TAModPadSettingOption* pA )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModGamePadOption" );

	if( CheckConnect() < ERROR_NONE )
	{
		g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, 0, 0, "[QueryModPadSettingOption] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	int iRet = ERROR_DB;

	memset( &m_wszQuery, 0, sizeof(m_wszQuery) );
	swprintf( m_wszQuery, L"{?=CALL dbo.P_ModGamePadOption(%d,?)}", pQ->nAccountDBID );

	SQLRETURN RetCode = SQLPrepare( m_hstmt, m_wszQuery, SQL_NTSL );
	if( CheckRetCode( RetCode, L"SQLPrepare" ) == ERROR_NONE )
	{
		SQLLEN cblen;
		int iSize = sizeof(TPadSetting);

		int nNo = 1;
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &iRet, sizeof(int), &cblen );
		cblen	= SQL_LEN_DATA_AT_EXEC(iSize);
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_BINARY, SQL_BINARY, iSize, 0, &pQ->sPadSetting, iSize, &cblen );

		RetCode = SQLExecute(m_hstmt);
		if( CheckRetCode( RetCode, L"SQLExecute" ) == ERROR_NONE )
		{
			SQLPOINTER pParam;
			while( RetCode == SQL_NEED_DATA )
			{
				RetCode = SQLParamData( m_hstmt, &pParam );
				if( RetCode == SQL_NEED_DATA )
				{
					if( pParam == &pQ->sPadSetting )
					{
						if( pQ->bUseDefault )
						{
							SQLPutData( m_hstmt, pParam, SQL_NULL_DATA );
						}
						else
						{
							SQLPutData( m_hstmt, pParam, iSize );
						}
					}
				}
			}
		}
	}

	return iRet;
}

int CDNSQLMembership::QueryCheckPCRoomIP(char *pIp, bool &bPCbang, char &cPCbangGrade)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_CheckPCRoomIP" );

	if (CheckConnect() < ERROR_NONE)
	{
		g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[QueryCheckPCRoomIP] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	swprintf(m_wszQuery, L"{?=CALL dbo.P_CheckPCRoomIP('%S',?,?)}", pIp);
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER,	SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int),	&cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_BIT,	SQL_BIT, sizeof(bool), 0, &bPCbang, sizeof(bool), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_TINYINT,	SQL_TINYINT, sizeof(char), 0, &cPCbangGrade, sizeof(char), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") != ERROR_NONE)
			return ERROR_DB;
	}

	return nResult;
}

// Auth
int CDNSQLMembership::QueryBeginAuth(UINT uiAccountDBID, char& cCertifyingStep, int& nServerID, char& cWorldSetID, UINT& nSessionID, INT64& biCharacterDBID, LPCWSTR wszAccountName, LPCWSTR wszCharacterName, char cAccountLevel, BYTE bIsAdult, char nAge, BYTE nDailyCreateCount, int nPrmInt1)
{
	CQueryTimeLog QueryTimeLog( "[Membership]Auth_BeginAuth" );

	if (CheckConnect() < 0) {
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, 0, L"[QueryBeginAuth] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	SQLRETURN RetCode;
	SQLLEN cblen;
	int nResult = 1;	// SQL error occurred

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.Auth_BeginAuth(%d,?,%d,?,?,%d,?,%I64d,?,N'%s',N'%s',%d,%d,%d,%d,%d,11)}",
		uiAccountDBID, nServerID, nSessionID, biCharacterDBID, wszAccountName, wszCharacterName, cAccountLevel, bIsAdult, nAge, nDailyCreateCount, nPrmInt1);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(BYTE), 0, &cCertifyingStep, sizeof(BYTE), &cblen);
		SQLBindParameter(m_hstmt, 3, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nServerID, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 4, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(BYTE), 0, &cWorldSetID, sizeof(BYTE), &cblen);
		SQLBindParameter(m_hstmt, 5, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nSessionID, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 6, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biCharacterDBID, sizeof(INT64), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){			
			return nResult;
		}
	}

	return nResult;
}

int CDNSQLMembership::QueryStoreAuth(UINT uiAccountDBID, INT64 biCertifyingKey, int nCurServerID, char cWorldSetID, UINT nSessionID, INT64 biCharacterDBID, LPCWSTR wszAccountName, LPCWSTR wszCharacterName, BYTE bIsAdult, char nAge, BYTE nDailyCreateCount, int nPrmInt1, char cLastServerType)
{
	CQueryTimeLog QueryTimeLog( "[Membership]Auth_StoreAuth" );

	if (CheckConnect() < 0) {
		g_Log.Log(LogType::_ERROR, cWorldSetID, uiAccountDBID, biCharacterDBID, nSessionID, L"[QueryStoreAuth] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	SQLRETURN RetCode;
	SQLLEN cblen;
	int nResult = 1;	// SQL error occurred

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.Auth_StoreAuth(%d,%I64d,%d,%d,%d,%I64d,N'%s',N'%s',%d,%d,%d,%d,%d)}", 
		uiAccountDBID, biCertifyingKey, nCurServerID, cWorldSetID, nSessionID, biCharacterDBID, wszAccountName, wszCharacterName, bIsAdult, nAge, nDailyCreateCount, nPrmInt1, cLastServerType);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);	

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){			
			return nResult;
		}
	}

	return ERROR_NONE;
}

int CDNSQLMembership::QueryCheckAuth(UINT uiAccountDBID, INT64 biCertifyingKey, int nCurServerID, OUT char& cWorldSetID, OUT UINT& nSessionID, OUT INT64& biCharacterDBID, OUT LPWSTR* wszAccountName, OUT LPWSTR* wszCharacterName, OUT char& cAccountLevel, OUT BYTE& bIsAdult, OUT char& nAge, OUT BYTE& nDailyCreateCount, OUT int& nPrmInt1, OUT char& cLastServerType)
{
	CQueryTimeLog QueryTimeLog( "[Membership]Auth_CheckAuth" );

	if (CheckConnect() < 0) {
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, 0, L"[QueryCheckAuth] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	SQLRETURN RetCode;
	SQLLEN cblen;
	int nResult = 1;	// SQL error occurred

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.Auth_CheckAuth(%d,%I64d,%d,%d,?,?,?,?,?,?,?,?,?,?,?)}", uiAccountDBID, biCertifyingKey, nCurServerID, CHECKAUTHLIMITSEC);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);	
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(BYTE), 0, &cWorldSetID, sizeof(BYTE), &cblen);
		SQLBindParameter(m_hstmt, 3, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nSessionID, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 4, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biCharacterDBID, sizeof(INT64), &cblen);
		SQLBindParameter(m_hstmt, 5, SQL_PARAM_OUTPUT, SQL_WCHAR, SQL_WVARCHAR, sizeof(WCHAR) * IDLENMAX, 0, wszAccountName, sizeof(WCHAR) * IDLENMAX, &cblen);
		SQLBindParameter(m_hstmt, 6, SQL_PARAM_OUTPUT, SQL_WCHAR, SQL_WVARCHAR, sizeof(WCHAR) * NAMELENMAX, 0, wszCharacterName, sizeof(WCHAR) * NAMELENMAX, &cblen);
		SQLBindParameter(m_hstmt, 7, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cAccountLevel, sizeof(char), &cblen);
		SQLBindParameter(m_hstmt, 8, SQL_PARAM_OUTPUT, SQL_BIT, SQL_BIT, sizeof(BYTE), 0, &bIsAdult, sizeof(BYTE), &cblen);
		SQLBindParameter(m_hstmt, 9, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &nAge, sizeof(char), &cblen);
		SQLBindParameter(m_hstmt, 10, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &nDailyCreateCount, sizeof(char), &cblen);
		SQLBindParameter(m_hstmt, 11, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nPrmInt1, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 12, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cLastServerType, sizeof(char), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){			
			return nResult;
		}
	}

	return ERROR_NONE;
}

int CDNSQLMembership::QueryResetAuth(UINT uiAccountDBID, UINT nSessionID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]Auth_ResetAuth" );

	int nResult = 1;	// SQL error occurred
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.Auth_ResetAuth(%d,%d)}", uiAccountDBID, nSessionID);

	nResult = CommonResultQuery(m_wszQuery);
	if (nResult != ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, nSessionID, L"[ADBID:%d] [Q:%s]\r\n", uiAccountDBID, m_wszQuery);
	}

	return nResult;
}

int CDNSQLMembership::QueryResetAuthServer(int nServerID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]Auth_ResetAuthServer" );

	int nResult = 1;	// SQL error occurred
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.Auth_ResetAuthServer(%d)}", nServerID);

	nResult = CommonResultQuery(m_wszQuery);
	if (nResult != ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[Q:%s] W:%d\r\n", m_wszQuery, nServerID);
	}

	return nResult;
}

int CDNSQLMembership::QueryResetAuthByAccountWorld(UINT uiAccountDBID, char cWorldSetID, UINT nSessionID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]Auth_ResetAuthByAccountWorld" );

	int nResult = 1;	// SQL error occurred
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.Auth_ResetAuthByAccountWorld(%d,%d,%d)}", uiAccountDBID, cWorldSetID, nSessionID);

	nResult = CommonResultQuery(m_wszQuery);
	if (nResult != ERROR_NONE){
		g_Log.Log(LogType::_ERROR, cWorldSetID, uiAccountDBID, 0, nSessionID, L"[ADBID:%d] [Q:%s]\r\n", uiAccountDBID, m_wszQuery);
	}

	return nResult;
}

int CDNSQLMembership::QuerySetWorldIDAuth(char cWorldSetID, UINT nAccountDBID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]Auth_SetWorldID" );

	int nResult = 1;	// SQL error occurred
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.Auth_SetWorldID(%d,%d)}", nAccountDBID, cWorldSetID);

	nResult = CommonResultQuery(m_wszQuery);
	if (nResult != ERROR_NONE){
		g_Log.Log(LogType::_ERROR, cWorldSetID, nAccountDBID, 0, 0, L"[ADBID:%d] [Q:%s]\r\n", nAccountDBID, m_wszQuery);
	}

	return nResult;
}

// 2nd auth related query
int CDNSQLMembership::QueryValidataSecondAuthPassphrase( UINT uiAccountDBID, const WCHAR* pwszPW, BYTE& cFailCount )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ValidateSecondAuthPassphrase" );

	if( CheckConnect() < 0 ) 
	{
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, 0, L"[QueryValidataSecondAuthPassphrase] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	int	nResult	= ERROR_DB;	// SQL error occurred

	memset( &m_wszQuery, 0, sizeof(m_wszQuery) );

	swprintf(m_wszQuery, L"{?=CALL dbo.P_ValidateSecondAuthPassphrase(%d,N'%s',%d,?)}", uiAccountDBID, pwszPW, SecondAuth::Common::LimitCount );

	SQLRETURN RetCode = SQLPrepare( m_hstmt, m_wszQuery, SQL_NTSL );
	if( CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE )
	{
		int nNo = 1;
		SQLLEN cblen;
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int),  0, &nResult,	  sizeof(int),  &cblen );
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cFailCount, sizeof(char), &cblen );

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode( RetCode, L"SQLExecute" );
	}

	return nResult;
}

int CDNSQLMembership::QueryModSecondAuthPassphrase( UINT uiAccountDBID, const WCHAR* pwszOldPW, const WCHAR* pwszNewPW, BYTE& cFailCount )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModSecondAuthPassphrase" );

	if( CheckConnect() < 0 ) 
	{
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, 0, L"[QueryModSecondAuthPassphrase] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	int	nResult	= ERROR_DB;	// SQL error occurred
	memset( &m_wszQuery, 0, sizeof(m_wszQuery) );
	swprintf( m_wszQuery, L"{?=CALL dbo.P_ModSecondAuthPassphrase(%d,N'%s',N'%s',%d,?)}", uiAccountDBID, pwszOldPW, pwszNewPW, SecondAuth::Common::LimitCount );

	SQLRETURN RetCode = SQLPrepare( m_hstmt, m_wszQuery, SQL_NTSL );
	if( CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE )
	{
		int nNo = 1;
		SQLLEN cblen;
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int),  0, &nResult,	  sizeof(int),  &cblen );
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cFailCount, sizeof(char), &cblen );

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode( RetCode, L"SQLExecute" );
	}

	return nResult;
}

int CDNSQLMembership::QueryModSecondAuthLockFlag( UINT uiAccountDBID, bool bLock )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModSecondAuthLockFlag" );

	if( CheckConnect() < 0 ) 
	{
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, 0, L"[QueryModSecondAuthLockFlag] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	int	nResult	= ERROR_DB;	// SQL error occurred

	memset( &m_wszQuery, 0, sizeof(m_wszQuery) );

	swprintf( m_wszQuery, L"{?=CALL dbo.P_ModSecondAuthLockFlag(%d,%d)}", uiAccountDBID, bLock ? 1 : 0 );

	SQLRETURN RetCode = SQLPrepare( m_hstmt, m_wszQuery, SQL_NTSL );
	if( CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE )
	{
		int nNo = 1;
		SQLLEN cblen;
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int),  0, &nResult,	  sizeof(int),  &cblen );

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode( RetCode, L"SQLExecute" );
	}

	return nResult;
}

int CDNSQLMembership::QueryGetSecondAuthStatus( UINT uiAccountDBID, bool& bSetPW, bool& bLock, __time64_t& tResetDate )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetSecondAuthStatus" );

	if( CheckConnect() < 0 ) 
	{
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, 0, L"[QueryGetSecondAuthStatus] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	int	nResult	= ERROR_DB;	// SQL error occurred
	BYTE cSetPW, cLock;
	TIMESTAMP_STRUCT ResetDate = {0,};

	memset( &m_wszQuery, 0, sizeof(m_wszQuery) );

	swprintf( m_wszQuery, L"{?=CALL dbo.P_GetSecondAuthStatus(%d,?,?)}", uiAccountDBID );

	SQLRETURN RetCode = SQLPrepare( m_hstmt, m_wszQuery, SQL_NTSL );
	if( CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE )
	{
		int nNo = 1;
		SQLLEN cblen;
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int),  0, &nResult, sizeof(int),  &cblen );
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cSetPW,  sizeof(char), &cblen );
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_BIT,	   SQL_BIT,		sizeof(BYTE), 0, &cLock,   sizeof(BYTE), &cblen );
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), 0, &ResetDate, sizeof(TIMESTAMP_STRUCT), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode( RetCode, L"SQLExecute" );
	}

	if( nResult == ERROR_NONE )
	{
		bSetPW		= (cSetPW == 3);
		bLock		= (cLock == 1);
		CTimeParamSet ResetTime(&QueryTimeLog, tResetDate, ResetDate, 0, uiAccountDBID);
		if (!ResetTime.IsValid()) return ERROR_DB;
	}

	return nResult;
}

int CDNSQLMembership::QueryInitSecondAuth( UINT uiAccountDBID )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_InitSecondAuth" );

	if( CheckConnect() < 0 ) 
	{
		g_Log.Log(LogType::_ERROR, 0, uiAccountDBID, 0, 0, L"[QueryInitSecondAuth] Check Connect Fail\r\n");
		return ERROR_DB;
	}	

	int	nResult	= ERROR_DB;	// SQL error occurred

	memset( &m_wszQuery, 0, sizeof(m_wszQuery) );
	swprintf( m_wszQuery, L"{?=CALL dbo.P_InitSecondAuth(%d)}", uiAccountDBID );

	SQLRETURN RetCode = SQLPrepare( m_hstmt, m_wszQuery, SQL_NTSL );
	if( CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE )
	{
		int nNo = 1;
		SQLLEN cblen;
		SQLBindParameter( m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int),  0, &nResult, sizeof(int),  &cblen );

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode( RetCode, L"SQLExecute" );
	}

	return nResult;
}

int CDNSQLMembership::QueryCheckIntroducedAccount(UINT nAccountDBID, OUT bool &bCheckResult)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_CheckIntroducedAccount" );

	if (CheckConnect() < ERROR_NONE)
	{
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryCheckIntroducedAccount:%d] CheckConnect Fail\r\n", nAccountDBID);
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_CheckIntroducedAccount(%d,?)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER,	SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int),	&cblen );
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_BIT,	SQL_BIT, sizeof(bool), 0, &bCheckResult, sizeof(bool), &cblen );		

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") != ERROR_NONE)
			return ERROR_DB;
	}

	return nResult;
}

int CDNSQLMembership::QueryGetProduct(int nProductID, TCashCommodityData *Product)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetProduct" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryGetProduct] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetProduct(%d)}", nProductID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter( m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen );	

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){			
			//TCashCommodityData Product = { 0, };
			//2010.10.7 haling Converted to constructor for STL protection
			TCashCommodityData Product;
			int nCategory = 0, nPeriod = 0, nPriority = 0;
			WCHAR wszProductName[NAMELENMAX] = { 0, };

			int nNo = 1;
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nCategory, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nSubCategory, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, wszProductName, sizeof(WCHAR) * NAMELENMAX, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nItemID[0], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nItemID[1], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nItemID[2], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nItemID[3], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nItemID[4], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nLinkSN[0], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nLinkSN[1], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nLinkSN[2], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nLinkSN[3], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nLinkSN[4], sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nPeriod, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nPrice, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nPriceFix, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nCount, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nPriority, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &Product.bOnSale, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &Product.bLimit, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &Product.bReserveGive, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nReserve, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Product.nValidity, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &Product.bReserveAble, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &Product.bPresentAble, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &Product.bCartAble, sizeof(bool), &cblen);
			CheckColumnCount(nNo, "[Membership]P_GetProduct");
			RetCode = SQLFetch(m_hstmt);
			if(RetCode == SQL_NO_DATA )
			{
				SQLCloseCursor(m_hstmt);
				return nResult;
			}
			if (CheckRetCode(RetCode, L"SQLFetch") == ERROR_NONE){
				Product.cCategory = nCategory;
				Product.wPeriod = nPeriod;
				Product.nPriority = nPriority;
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}


#ifdef PRE_ADD_LIMITED_CASHITEM
int CDNSQLMembership::QueryAddPurchaseOrderByCash(INT64 biCharacterDBID, bool bPCBang, int nItemSN, int nPrice, int nLimitCount, int nSelectItemID, BYTE cSelectItemOption, int nLimitedItemMax, int nTotalPrice, char *pIp, 
												  bool bGift, WCHAR *pReceiveCharacterName, WCHAR *pMemo, OUT INT64 &biPurchaseOrderID, OUT char* pPurchaseOrderDetailID, const char *pszRefundAbles, bool bCheatFlag )
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
int CDNSQLMembership::QueryAddPurchaseOrderByCash(INT64 biCharacterDBID, bool bPCBang, int nItemSN, int nPrice, int nLimitCount, int nSelectItemID, BYTE cSelectItemOption, int nTotalPrice, char *pIp, 
												  bool bGift, WCHAR *pReceiveCharacterName, WCHAR *pMemo, OUT INT64 &biPurchaseOrderID, OUT char* pPurchaseOrderDetailID, const char *pszRefundAbles, bool bCheatFlag )
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddPurchaseOrderByCash" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryAddPurchaseOrderByCash] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	WCHAR wszReceiveName[NAMELENMAX * 2 + 1] = {0,}, wszMemo[GIFTMESSAGEMAX * 2 + 1] = {0,};
	if (pReceiveCharacterName)
		ConvertQuery(pReceiveCharacterName, NAMELENMAX, wszReceiveName, _countof(wszReceiveName));
	if (pMemo)
		ConvertQuery(pMemo, GIFTMESSAGEMAX, wszMemo, _countof(wszMemo));

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));	
	if( pszRefundAbles && pPurchaseOrderDetailID )
	{
#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%d','%d','%d','%d',%d,'%S',%d,N'%s',N'%s',?,?,'%S',%d,'%d','%d')}",
			biCharacterDBID, bPCBang, nItemSN, nPrice, nLimitCount, nSelectItemID, nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, pszRefundAbles, bCheatFlag, cSelectItemOption, nLimitedItemMax);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%d','%d','%d','%d',%d,'%S',%d,N'%s',N'%s',?,?,'%S',%d,'%d')}",
			biCharacterDBID, bPCBang, nItemSN, nPrice, nLimitCount, nSelectItemID, nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, pszRefundAbles, bCheatFlag, cSelectItemOption);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	}
	else if( !pszRefundAbles && !pPurchaseOrderDetailID )
	{
#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%d','%d','%d','%d',%d,'%S',%d,N'%s',N'%s',?,NULL,NULL,%d,'%d','%d')}", 
			biCharacterDBID, bPCBang, nItemSN, nPrice, nLimitCount, nSelectItemID, nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, bCheatFlag, cSelectItemOption, nLimitedItemMax);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%d','%d','%d','%d',%d,'%S',%d,N'%s',N'%s',?,NULL,NULL,%d,'%d')}", 
			biCharacterDBID, bPCBang, nItemSN, nPrice, nLimitCount, nSelectItemID, nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, bCheatFlag, cSelectItemOption);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	}
	else if( pszRefundAbles && !pPurchaseOrderDetailID )
	{
#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%d','%d','%d','%d',%d,'%S',%d,N'%s',N'%s',?,NULL,'%S',%d,'%d','%d')}", 
			biCharacterDBID, bPCBang, nItemSN, nPrice, nLimitCount, nSelectItemID, nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, pszRefundAbles, bCheatFlag, cSelectItemOption, nLimitedItemMax);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%d','%d','%d','%d',%d,'%S',%d,N'%s',N'%s',?,NULL,'%S',%d,'%d')}", 
			biCharacterDBID, bPCBang, nItemSN, nPrice, nLimitCount, nSelectItemID, nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, pszRefundAbles, bCheatFlag, cSelectItemOption);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	}
	else if( !pszRefundAbles && pPurchaseOrderDetailID )
	{
#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%d','%d','%d','%d',%d,'%S',%d,N'%s',N'%s',?,?,NULL,%d,'%d','%d')}", 
			biCharacterDBID, bPCBang, nItemSN, nPrice, nLimitCount, nSelectItemID, nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, bCheatFlag, cSelectItemOption, nLimitedItemMax);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%d','%d','%d','%d',%d,'%S',%d,N'%s',N'%s',?,?,NULL,%d,'%d')}", 
			biCharacterDBID, bPCBang, nItemSN, nPrice, nLimitCount, nSelectItemID, nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, bCheatFlag, cSelectItemOption);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	}

	CDNSqlLen SqlLen1;
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biPurchaseOrderID, sizeof(INT64), &cblen);			
		if( pPurchaseOrderDetailID )
			SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_CHAR, SQL_VARCHAR, 64, 0, pPurchaseOrderDetailID, 64, SqlLen1.GetNull(!pPurchaseOrderDetailID, 64)); //This one is a single item.. so 64
		
		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)
	if (nResult == ERROR_NONE && nLimitedItemMax > 0)
		QueryGetLimitedItem();
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)

	return nResult;
}

int CDNSQLMembership::QueryAddPurchaseOrderByCash(INT64 biCharacterDBID, bool bPCBang, std::vector<string> &VecItemString, int nTotalPrice, char *pIp, bool bGift, WCHAR *pReceiveCharacterName, WCHAR *pMemo, 
												  OUT INT64 &biPurchaseOrderID, OUT char* pPurchaseOrderDetailIDs/*=NULL*/, bool bCheatFlag/*=false*/)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddPurchaseOrderByCash" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryAddPurchaseOrderByCash] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	WCHAR wszReceiveName[NAMELENMAX * 2 + 1] = {0,}, wszMemo[GIFTMESSAGEMAX * 2 + 1] = {0,};
	if (pReceiveCharacterName)
		ConvertQuery(pReceiveCharacterName, NAMELENMAX, wszReceiveName, _countof(wszReceiveName));
	if (pMemo)
		ConvertQuery(pMemo, GIFTMESSAGEMAX, wszMemo, _countof(wszMemo));

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));	

	if( !(VecItemString[Append_Refundable].empty()) && pPurchaseOrderDetailIDs)
	{
#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%S','%S','%S','%S',%d,'%S',%d,N'%s',N'%s',?,?,'%S',%d,'%S','%S')}", 
			biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(),
			nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, VecItemString[Append_Refundable].c_str(), bCheatFlag, VecItemString[Append_Option].c_str(), VecItemString[Append_QuantityLimited].c_str());
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%S','%S','%S','%S',%d,'%S',%d,N'%s',N'%s',?,?,'%S',%d,'%S')}", 
			biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(),
			nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, VecItemString[Append_Refundable].c_str(), bCheatFlag, VecItemString[Append_Option].c_str());
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	}
	else if( VecItemString[Append_Refundable].empty() && !pPurchaseOrderDetailIDs )
	{
#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%S','%S','%S','%S',%d,'%S',%d,N'%s',N'%s',?,NULL,NULL,%d,'%S','%S')}", 
			biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(),
			nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, bCheatFlag, VecItemString[Append_Option].c_str(), VecItemString[Append_QuantityLimited].c_str());
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%S','%S','%S','%S',%d,'%S',%d,N'%s',N'%s',?,NULL,NULL,%d,'%S')}", 
			biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(),
			nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, bCheatFlag, VecItemString[Append_Option].c_str());
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	}
	else if( !(VecItemString[Append_Refundable].empty()) && !pPurchaseOrderDetailIDs )
	{
#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%S','%S','%S','%S',%d,'%S',%d,N'%s',N'%s',?,NULL,'%S',%d,'%S','%S')}", 
			biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(),
			nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, VecItemString[Append_Refundable].c_str(), bCheatFlag, VecItemString[Append_Option].c_str(), VecItemString[Append_QuantityLimited].c_str());
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%S','%S','%S','%S',%d,'%S',%d,N'%s',N'%s',?,NULL,'%S',%d,'%S')}", 
			biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(),
			nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, VecItemString[Append_Refundable].c_str(), bCheatFlag, VecItemString[Append_Option].c_str());
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	}
	else if( VecItemString[Append_Refundable].empty() && pPurchaseOrderDetailIDs )
	{
#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%S','%S','%S','%S',%d,'%S',%d,N'%s',N'%s',?,?,NULL,%d,'%S','%S')}", 
			biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(),
			nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, bCheatFlag, VecItemString[Append_Option].c_str(), VecItemString[Append_QuantityLimited].c_str());
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPurchaseOrderByCash(%I64d,%d,'%S','%S','%S','%S',%d,'%S',%d,N'%s',N'%s',?,?,NULL,%d,'%S')}", 
			biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(),
			nTotalPrice, pIp, bGift, wszReceiveName, wszMemo, bCheatFlag, VecItemString[Append_Option].c_str());
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	}

	CDNSqlLen SqlLen1;
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biPurchaseOrderID, sizeof(INT64), &cblen);	
		if( pPurchaseOrderDetailIDs )
			SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_CHAR, SQL_VARCHAR, 1024, 0, pPurchaseOrderDetailIDs, 1024, SqlLen1.GetNull(!pPurchaseOrderDetailIDs, 1024)); 		
		
		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)
	if (nResult == ERROR_NONE && atoi(VecItemString[Append_QuantityLimited].c_str()) > 0)
		QueryGetLimitedItem();
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)

	return nResult;
}

int CDNSQLMembership::QuerySetPurchaseOrderResult(INT64 biPurchaseOrderID, char cOrderStatusCode, char *pOrderKey, int nOrderResult, int nPetalAmount, int nPaidCashAmount, char *pContextKey)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_SetPurchaseOrderResult" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QuerySetPurchaseOrderResult] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	CDNSqlLen SqlLen, SqlPaidLen;
	char szContextKey[PUBLISHERORDERKEYMAX] = { 0, };
	if (pContextKey)
		_strcpy(szContextKey, _countof(szContextKey), pContextKey, (int)strlen(pContextKey));

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_SetPurchaseOrderResult(%I64d,%d,'%S',%d,%d,?,?)}", biPurchaseOrderID, cOrderStatusCode, pOrderKey, nOrderResult, nPetalAmount);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_CHAR, SQL_VARCHAR, PUBLISHERORDERKEYMAX, 0, szContextKey, PUBLISHERORDERKEYMAX, SqlLen.GetNull(!pContextKey, PUBLISHERORDERKEYMAX));
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nPaidCashAmount, sizeof(int), SqlPaidLen.GetNull(nPaidCashAmount <= 0, sizeof(int)));

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryRollbackPurchaseOrderByCash(INT64 biPurchaseOrderID, OUT char *pOrderKey, OUT int &nProductID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_RollbackPurchaseOrderByCash" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryRollbackPurchaseOrderByCash] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	nProductID = 0;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_RollbackPurchaseOrderByCash(%I64d,?)}", biPurchaseOrderID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_CHAR, SQL_VARCHAR, sizeof(char) * 32, 0, pOrderKey, sizeof(char) * 32, &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QuerySetRollbackPurchaseOrderResult(INT64 biOrderID, char cStatusCode, int nRollbackResult)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_SetRollbackPurchaseOrderResult" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_SetRollbackPurchaseOrderResult(%I64d,%d,%d)}", biOrderID, cStatusCode, nRollbackResult);

	return CommonReturnValueQuery(m_wszQuery);
}

#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)
int CDNSQLMembership::QueryGetLimitedItem()
{
	CQueryTimeLog QueryTimeLog( "[Membership]QueryGetLimitedItem" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryGetLimitedItem] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListLimitedProduct}");

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			int nNo = 1;
			std::vector<LimitedCashItem::TLimitedQuantityCashItem> vList;
			LimitedCashItem::TLimitedQuantityCashItem LimitedItem;
			memset(&LimitedItem, 0, sizeof(LimitedCashItem::TLimitedQuantityCashItem));
			
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &LimitedItem.nProductID, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &LimitedItem.nSaleCount, sizeof(int), &cblen);	

			CheckColumnCount(nNo, "[Membership]QueryGetLimitedItem");
			while(1)
			{
				memset(&LimitedItem, 0, sizeof(LimitedCashItem::TLimitedQuantityCashItem));

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA){
					nResult = ERROR_NONE;
					break;
				}
				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				vList.push_back(LimitedItem);
			}

			if (g_pLimitedCashItemRepository)
				g_pLimitedCashItemRepository->UpdateLimitedItemList(vList);
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)

#if defined(PRE_ADD_CASH_REFUND)
int CDNSQLMembership::QueryGetListRefundableProducts(INT64 biCharacterDBID, TAPaymentItemList* pPaymentItemList, TAPaymentPackageItemList* pPaymentPackageItemList, int nRefundLimitPeriod )
{	
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetListRefundableProducts" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryGetListRefundableProducts] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	int nVersion = 2;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListRefundableProducts(%I64d, %d, %d)}", biCharacterDBID, nRefundLimitPeriod, nVersion);
	
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			int nNo = 1;
			INT64 biPurchaseOrderDetailID = 0;
			char szOrderNum[64] = {0, };			
			int nProductID = 0;
			int nPrice = 0;			
			char szSelectedItemIDs[2048] = {0, }, szSelectedItemOptions[2048] = {0,};
			TIMESTAMP_STRUCT OrderDate = {0,};
			
			bool bflag = false;  //
			SQLBindCol(m_hstmt, nNo++, SQL_C_SBIGINT, &biPurchaseOrderDetailID, sizeof(INT64), &cblen);			
			SQLBindCol(m_hstmt, nNo++, SQL_CHAR, szOrderNum, sizeof(CHAR) * 64, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nProductID, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nPrice, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_CHAR, szSelectedItemIDs, sizeof(CHAR)*2048, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_CHAR, szSelectedItemOptions, sizeof(CHAR)*2048, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &bflag, sizeof(bool), &cblen);	
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &OrderDate, sizeof(TIMESTAMP_STRUCT), &cblen);	

			CheckColumnCount(nNo, "[Membership]P_GetListRefundableProducts");
			while(1)
			{
				memset(&szOrderNum, 0, sizeof(szOrderNum));
				memset(&OrderDate, 0, sizeof(OrderDate));
				memset(&szSelectedItemIDs, 0, sizeof(szSelectedItemIDs));
				memset(&szSelectedItemOptions, 0, sizeof(szSelectedItemOptions));
				biPurchaseOrderDetailID = 0;
				nProductID = 0;
				nPrice = 0;
				bflag = false;

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA){
					nResult = ERROR_NONE;
					break;
				}
				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				// Check first if it is a package				
				string strSelectedItemIDs = szSelectedItemIDs;
				string strSelectedItemOptions = szSelectedItemOptions;

				std::vector<std::string> tokens, Optiontokens;
				TokenizeA(strSelectedItemIDs.c_str(), tokens, ",");
				TokenizeA(strSelectedItemOptions.c_str(), Optiontokens, ",");
				if( tokens.size() == 1 ) // If there is one ID, it is a single item.
				{
					// Single item..
					int nTempCount = pPaymentItemList->nPaymentCount; // Because the code length is too long..
					pPaymentItemList->PaymentItemList[nTempCount].uiOrderNo = strtoul(szOrderNum, NULL, 10);
					pPaymentItemList->PaymentItemList[nTempCount].nPrice = nPrice;
					pPaymentItemList->PaymentItemList[nTempCount].PaymentItemInfo.biDBID = biPurchaseOrderDetailID;
					CTimeParamSet OrderTime(&QueryTimeLog, pPaymentItemList->PaymentItemList[nTempCount].PaymentItemInfo.tPaymentDate, OrderDate, 0, 0, biCharacterDBID);
					if (!OrderTime.IsValid())
					{
						nResult = ERROR_DB;
						break;									
					}
					pPaymentItemList->PaymentItemList[nTempCount].PaymentItemInfo.ItemInfo.nItemSN = nProductID;
					pPaymentItemList->PaymentItemList[nTempCount].PaymentItemInfo.ItemInfo.nItemID = atoi(szSelectedItemIDs);
					pPaymentItemList->PaymentItemList[nTempCount].PaymentItemInfo.ItemInfo.cItemOption = atoi(szSelectedItemOptions);
					++pPaymentItemList->nPaymentCount;
				}
				else
				{
					// Package...					
					int nTempCount = pPaymentPackageItemList->nPaymentPackegeCount;  // Because the code length is too long..
					pPaymentPackageItemList->PaymentPackageItemList[nTempCount].uiOrderNo = strtoul(szOrderNum, NULL, 10);
					pPaymentPackageItemList->PaymentPackageItemList[nTempCount].nPrice = nPrice;
					pPaymentPackageItemList->PaymentPackageItemList[nTempCount].PaymentPackageItemInfo.biDBID = biPurchaseOrderDetailID;
					pPaymentPackageItemList->PaymentPackageItemList[nTempCount].PaymentPackageItemInfo.nPackageSN = nProductID;
					CTimeParamSet OrderTime(&QueryTimeLog, pPaymentPackageItemList->PaymentPackageItemList[nTempCount].PaymentPackageItemInfo.tPaymentDate, OrderDate, 0, 0, biCharacterDBID);
					if (!OrderTime.IsValid())
					{
						nResult = ERROR_DB;
						break;									
					}

					std::vector<std::string>::iterator ii;
					int nCount = 0;
					for(ii = tokens.begin(); ii != tokens.end(); ii++)
					{
						// nItemSN probably needs to be received from village and set.. not in DB..
						pPaymentPackageItemList->PaymentPackageItemList[nTempCount].PaymentPackageItemInfo.ItemInfoList[nCount].nItemID = atoi((*ii).c_str());
						pPaymentPackageItemList->PaymentPackageItemList[nTempCount].PaymentPackageItemInfo.ItemInfoList[nCount].cItemOption = -1;
						++nCount;		
					}

					nCount = 0;
					for(ii = Optiontokens.begin(); ii != Optiontokens.end(); ii++)
					{
						pPaymentPackageItemList->PaymentPackageItemList[nTempCount].PaymentPackageItemInfo.ItemInfoList[nCount].cItemOption = atoi((*ii).c_str());
						++nCount;		
					}

					++pPaymentPackageItemList->nPaymentPackegeCount;
				}

				// Cap at 150 for now..
				if(pPaymentItemList->nPaymentCount >= CASHINVENTORYMAX || pPaymentPackageItemList->nPaymentPackegeCount >= CASHINVENTORYMAX ){
					nResult = ERROR_NONE;
					break;
				}
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryModPurchaseOrderDetail(INT64 biPurchaseOrderDetailID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModPurchaseOrderDetail" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModPurchaseOrderDetail(%I64d)}", biPurchaseOrderDetailID);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryCashRefund(INT64 biPurchaseOrderDetailID, int nRefundLimitPeriod )
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_Refund" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_Refund(%I64d, %d)}", biPurchaseOrderDetailID, nRefundLimitPeriod);

	return CommonReturnValueQuery(m_wszQuery);
}
#endif // #if defined(PRE_ADD_CASH_REFUND)

int CDNSQLMembership::QueryCheatGiftClear(INT64 biCharacterDBID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_CheatGiftClear" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_CheatGiftClear(%I64d)}", biCharacterDBID);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryIncreaseVIPBasicPoint(INT64 biCharacterDBID, int nBasicPoint, INT64 biPurchaseOrderID, short wVIPPeriod, bool bAutoPay, OUT int &nVIPTotalPoint, OUT __time64_t &tVIPEndDate)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_IncreaseVIPBasicPoint" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryIncreaseVIPBasicPoint] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_IncreaseVIPBasicPoint(%I64d,%d,%I64d,%d,%d,?,?,?)}", biCharacterDBID, nBasicPoint, biPurchaseOrderID, wVIPPeriod, bAutoPay);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		TIMESTAMP_STRUCT StartDate = {0,}, EndDate = {0,};

		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nVIPTotalPoint, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), 0, &StartDate, sizeof(TIMESTAMP_STRUCT), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), 0, &EndDate, sizeof(TIMESTAMP_STRUCT), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
			CTimeParamSet EndTime(&QueryTimeLog, tVIPEndDate, EndDate, 0, 0, biCharacterDBID);
			if (!EndTime.IsValid()) return ERROR_DB;
		}
	}

	return nResult;
}

int CDNSQLMembership::QueryGetVIPPoint(INT64 biCharacterDBID, OUT int &nVIPTotalPoint, OUT __time64_t &tVIPEndDate, OUT bool &bAutoPay)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetVIPPoint" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryGetVIPPoint] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetVIPPoint(%I64d,?,?,?,?)}", biCharacterDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		TIMESTAMP_STRUCT StartDate = {0,}, EndDate = {0,};

		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nVIPTotalPoint, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), 0, &StartDate, sizeof(TIMESTAMP_STRUCT), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), 0, &EndDate, sizeof(TIMESTAMP_STRUCT), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_BIT, SQL_BIT, sizeof(bool), 0, &bAutoPay, sizeof(bool), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
			CTimeParamSet EndTime(&QueryTimeLog, tVIPEndDate, EndDate, 0, 0, biCharacterDBID);
			if (!EndTime.IsValid()) return ERROR_DB;
		}
	}

	return nResult;
}

int CDNSQLMembership::QueryModVIPAutoPayFlag(INT64 biCharacterDBID, bool bAutoPay)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModVIPAutoPayFlag" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModVIPAutoPayFlag(%I64d,%d)}", biCharacterDBID, bAutoPay);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryAddBanOfSale(int nItemSN)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddBanOfSale" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddBanOfSale(%d)}", nItemSN);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryGetListBanOfSale(std::vector<int> &VecProhibitList)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetListBanOfSale" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryGetListBanOfSale] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListBanOfSale}");

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
			int nItemSN = 0;
			int nNo = 1;
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nItemSN, sizeof(int), &cblen);
			CheckColumnCount(nNo, "[Membership]P_GetListBanOfSale");
			while(1)
			{
				nItemSN = 0;

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA){
					nResult = ERROR_NONE;
					break;
				}
				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				VecProhibitList.push_back(nItemSN);
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryGetPetalBalance(int nAccountDBID, OUT int &nPetalBalance)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetPetalBalance" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryGetPetalBalance] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetPetalBalance(%d,?)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nPetalBalance, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryPurchaseItemByPetal(INT64 biCharacterDBID, bool bPCBang, std::vector<string> &VecItemString, int nTotalPrice, char *pIp, OUT INT64 &biPurchaseOrderID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_PurchaseItemByPetal" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryPurchaseItemByPetal] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
#ifdef PRE_ADD_LIMITED_CASHITEM
	swprintf(m_wszQuery, L"{?=CALL dbo.P_PurchaseItemByPetal(%I64d,%d,'%S','%S','%S','%S',%d,'%S',?,'%S','%S')}",
		biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(), nTotalPrice, pIp, VecItemString[Append_Option].c_str(), VecItemString[Append_QuantityLimited].c_str());
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
	swprintf(m_wszQuery, L"{?=CALL dbo.P_PurchaseItemByPetal(%I64d,%d,'%S','%S','%S','%S',%d,'%S',?,'%S')}",
		biCharacterDBID, bPCBang, VecItemString[Append_ItemSN].c_str(), VecItemString[Append_Price].c_str(), VecItemString[Append_Limit].c_str(), VecItemString[Append_ItemID].c_str(), nTotalPrice, pIp, VecItemString[Append_Option].c_str());
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biPurchaseOrderID, sizeof(INT64), &cblen);	

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)
	if (nResult == ERROR_NONE && atoi(VecItemString[Append_QuantityLimited].c_str()) > 0)
		QueryGetLimitedItem();
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)

	return nResult;
}

int CDNSQLMembership::QueryRollbackPurchaseItemByPetal(INT64 biOrderID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_RollbackPurchaseItemByPetal" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_RollbackPurchaseItemByPetal(%I64d)}", biOrderID);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryAddPetalIncome(INT64 biCharacterDBID, INT64 biPurchaseOrderID, int nPetal, OUT int &nTotalPetal, bool bTrade)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddPetalIncome" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryAddPetalIncome] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	if( bTrade )
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPetalIncome(%I64d,%I64d,%d,?, 7)}", biCharacterDBID, biPurchaseOrderID, nPetal); //Version 7 if this is market sales proceeds
	else
		swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPetalIncome(%I64d,%I64d,%d,?)}", biCharacterDBID, biPurchaseOrderID, nPetal);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nTotalPetal, sizeof(int), &cblen);	

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

#if defined( PRE_PVP_GAMBLEROOM )
int CDNSQLMembership::QueryAddPetalIncome(INT64 biCharacterDBID, INT64 biPurchaseOrderID, int nPetal, OUT int &nTotalPetal, BYTE cPetalChangeCode, INT64 nGambleDBID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddPetalIncome" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryAddPetalIncome] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	CDNSqlLen SqlLen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPetalIncome(%I64d,?,%d,?,%d,%I64d)}", biCharacterDBID, nPetal, cPetalChangeCode, nGambleDBID);
	
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biPurchaseOrderID, sizeof(INT64), SqlLen.GetNull(biPurchaseOrderID <= 0, sizeof(INT64)));
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nTotalPetal, sizeof(int), &cblen);	

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryUsePetal(INT64 biCharacterDBID, int nPetal, BYTE cPetalChangeCode, INT64 nGambleDBID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_UsePetal" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryAddPetalIncome] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_UsePetal(%I64d,%d,%d,%I64d)}", biCharacterDBID, nPetal, cPetalChangeCode, nGambleDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);		

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}
#endif

#if defined(PRE_ADD_CASH_REFUND)
int CDNSQLMembership::QueryAddPetalIncomeCashRefund(INT64 biCharacterDBID, INT64 biPurchaseOrderDetailID, int nPetal, OUT int &nTotalPetal)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddPetalIncomeForReward" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryAddPetalIncomeCashRefund] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddPetalIncomeForReward(%I64d,%I64d,%d,?)}", biCharacterDBID, biPurchaseOrderDetailID, nPetal);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nTotalPetal, sizeof(int), &cblen);	

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}
#endif // #if defined(PRE_ADD_CASH_REFUND)

int CDNSQLMembership::QueryGetCashBalance(int nAccountDBID, WCHAR *pAccountName, int &nCashBalance)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetCashBalance" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryGetCashBalance] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	CDNSqlLen SqlLen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetCashBalance(%d,?,?)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_INPUT, SQL_WCHAR, SQL_WVARCHAR, sizeof(WCHAR) * IDLENMAX, 0, pAccountName, sizeof(WCHAR) * IDLENMAX, SqlLen.GetNull(!pAccountName, sizeof(WCHAR) * IDLENMAX));
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nCashBalance, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryDeductCash(int nAccountDBID, INT64 biPurchaseOrderID, int nDelCash, int &nCashOutgoID, int &nCashBalance)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_DeductCash" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryDeductCash] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	CDNSqlLen SqlLen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_DeductCash(%d,%I64d,%d,?,?)}", nAccountDBID, biPurchaseOrderID, nDelCash);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nCashOutgoID, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nCashBalance, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}


int CDNSQLMembership::QueryMakeGiftByCoupon(INT64 biCharacterDBID, bool bPCBang, const char *pszItemSNs, const char *pszSelectItemIDs, const char *pszSelectItemOptions, WCHAR *pCoupon, const char *pOrderKey, char *pIp, OUT INT64 &biPurchaseOrderID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_MakeGiftByCoupon" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryMakeGiftByCoupon] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_MakeGiftByCoupon(%I64d,%d,'%S','%S',N'%s','%S','%S',?,%d,'%S')}", biCharacterDBID, bPCBang, pszItemSNs, pszSelectItemIDs, pCoupon, pOrderKey, pIp, false, pszSelectItemOptions);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biPurchaseOrderID, sizeof(INT64), &cblen);	

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryMakeGiftByQuest(INT64 biCharacterDBID, bool bPCBang, int nItemSN, const char *pszSelectItemIDs, const char *pszSelectItemOptions, int nQuestID, WCHAR *pMemo, int nLifeSpan, char *pIp, bool bNewFlag, OUT INT64 &biPurchaseOrderID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_MakeGiftByQuest" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryMakeGiftByQuest] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_MakeGiftByQuest(%I64d,%d,%d,'%S',%d,N'%s',%d,'%S',?,%d,'%S')}", biCharacterDBID, bPCBang, nItemSN, pszSelectItemIDs, nQuestID, pMemo, nLifeSpan, pIp, bNewFlag, pszSelectItemOptions);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biPurchaseOrderID, sizeof(INT64), &cblen);	

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryMakeGiftByMission(INT64 biCharacterDBID, bool bPCBang, int nItemSN, const char *pszSelectItemIDs, const char *pszSelectItemOptions, int nMissionID, WCHAR *pMemo, int nLifeSpan, char *pIp, bool bNewFlag, OUT INT64 &biPurchaseOrderID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_MakeGiftByMission" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryMakeGiftByMission] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_MakeGiftByMission(%I64d,%d,%d,'%S',%d,N'%s',%d,'%S',?,%d,'%S')}", biCharacterDBID, bPCBang, nItemSN, pszSelectItemIDs, nMissionID, pMemo, nLifeSpan, pIp, bNewFlag, pszSelectItemOptions);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biPurchaseOrderID, sizeof(INT64), &cblen);	

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

#ifdef PRE_ADD_LIMITED_CASHITEM
int CDNSQLMembership::QueryMakeGift(INT64 biCharacterDBID, bool bPCBang, int nItemSN, const char *pszSelectItemIDs, const char *pszSelectItemOptions, const char * pszLimitedQuantityMax, WCHAR *pMemo, int nLifeSpan, int nPayMethodCode, int iOrderKey, char *pIp, bool bNewFlag, OUT INT64 &biPurchaseOrderID)
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
int CDNSQLMembership::QueryMakeGift(INT64 biCharacterDBID, bool bPCBang, int nItemSN, const char *pszSelectItemIDs, const char *pszSelectItemOptions, WCHAR *pMemo, int nLifeSpan, int nPayMethodCode, int iOrderKey, char *pIp, bool bNewFlag, OUT INT64 &biPurchaseOrderID)
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_MakeGift" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryMakeGift] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
#ifdef PRE_ADD_LIMITED_CASHITEM
	swprintf(m_wszQuery, L"{?=CALL dbo.P_MakeGift(%I64d,%d,%d,'%S',N'%s',%d,%d,%d,'%S',?,%d,'%S','%S')}", biCharacterDBID, bPCBang, nItemSN, pszSelectItemIDs, pMemo, nLifeSpan, nPayMethodCode, iOrderKey, pIp, bNewFlag, pszSelectItemOptions, pszLimitedQuantityMax);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
	swprintf(m_wszQuery, L"{?=CALL dbo.P_MakeGift(%I64d,%d,%d,'%S',N'%s',%d,%d,%d,'%S',?,%d,'%S')}", biCharacterDBID, bPCBang, nItemSN, pszSelectItemIDs, pMemo, nLifeSpan, nPayMethodCode, iOrderKey, pIp, bNewFlag, pszSelectItemOptions);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biPurchaseOrderID, sizeof(INT64), &cblen);	

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryAddGuildWarMakeGiftQueue(INT64 biCharacterDBID, int nItemSN, const char *pszSelectItemIDs, WCHAR *pMemo, int nLifeSpan, int nPayMethodCode, int nMatchTypeCode , char *pIp, bool bNewFlag)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddGuildWarMakeGiftQueue" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryAddGuildWarMakeGiftQueue] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddGuildWarMakeGiftQueue(%I64d,'0',%d,'%S',N'%s',%d,%d,%d,'%S','0',%d)}", biCharacterDBID, nItemSN, pszSelectItemIDs, pMemo, nLifeSpan, nPayMethodCode, nMatchTypeCode, pIp, bNewFlag);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);		

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}
	return nResult;
}

// For Guild War finals reward.. reserve gift..
int CDNSQLMembership::QueryAddGiftSendSchedule(const char *pszCharacterDBIDs, int nItemSN, const char *pszSelectItemIDs, WCHAR *pMemo, int nLifeSpan, int nPayMethodCode, char *pIp, bool bNewFlag)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_AddGiftSendSchedule" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryAddGiftSendSchedule] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddGiftSendSchedule('1','%S','',%d,'%S',%d,'%S',N'%s',%d,%d)}", pszCharacterDBIDs, nLifeSpan, pIp, nItemSN, pszSelectItemIDs, pMemo, nPayMethodCode, bNewFlag);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);		

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}
	return nResult;
}

int CDNSQLMembership::QueryModGiveFailFlag(INT64 biPurchaseOrderID, int nProductSN, int nItemID, bool bItemGiveFail)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModGiveFailFlag" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModGiveFailFlag(%I64d,%d,%d,%d)}", biPurchaseOrderID, nProductSN, nItemID, bItemGiveFail);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryModGiveFailFlag2(INT64 biPurchaseOrderDetailID, bool bItemGiveFail)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModGiveFailFlag2" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModGiveFailFlag2(%I64d,%d)}", biPurchaseOrderDetailID, bItemGiveFail);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryGetListGiveFailItem(INT64 biCharacterDBID, TAGetListGiveFailItem *pA)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetListGiveFailItem" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryGetListGiveFailItem] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	int nVersion = 14;

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListGiveFailItem(%I64d, %d)}", biCharacterDBID, nVersion);

	pA->nCount = 0;
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
			DBPacket::TCashFailItem FailItem = { 0, };
			int nNo = 1;
			SQLBindCol(m_hstmt, nNo++, SQL_C_SBIGINT, &FailItem.biPurchaseOrderID, sizeof(INT64), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_C_SBIGINT, &FailItem.biSenderCharacterDBID, sizeof(INT64), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &FailItem.bGift, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &FailItem.nItemSN, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &FailItem.nItemID, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &FailItem.cItemOption, sizeof(BYTE), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &FailItem.nPaidCashAmount, sizeof(int), &cblen);
			CheckColumnCount(nNo, "[Membership]P_GetListGiveFailItem");
			while(1)
			{
				memset(&FailItem, 0, sizeof(FailItem));

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA){
					nResult = ERROR_NONE;
					break;
				}
				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				pA->CashFailItem[pA->nCount] = FailItem;
				pA->nCount++;

				if (pA->nCount >= CASHINVENTORYDBMAX){
					nResult = ERROR_NONE;
					break;
				}
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryGetCountNotReceivedGift(INT64 biCharacterDBID, OUT int &nGiftCount)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetCountNotReceivedGift" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryGetCountNotReceivedGift] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
#if defined(PRE_ADD_GIFT_RETURN)
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetCountNotReceivedGift(%I64d,?,6)}", biCharacterDBID);
#else
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetCountNotReceivedGift(%I64d,?)}", biCharacterDBID);
#endif

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nGiftCount, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryGetListGiftBox(INT64 biCharacterDBID, OUT char &cCount, OUT TGiftInfo *GiftBoxList)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetListGiftBox" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryGetListGiftBox] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	int nVersion = 9;

	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListGiftBox(%I64d,%d)}", biCharacterDBID, nVersion);

	cCount = 0;
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
			TGiftInfo Gift = { 0, };
			INT64 biSenderCharacterDBID = 0;
			TIMESTAMP_STRUCT OrderDate = {0,}, GiftExpireDate = {0,};
			WCHAR wszItemID[512] = {0, }, wszItemOptions[512] = {0,};
			bool bGiftReturn;
			int nNo = 1;
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &Gift.cPayMethodCode, sizeof(char), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_C_SBIGINT, &Gift.nGiftDBID, sizeof(INT64), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_C_SBIGINT, &biSenderCharacterDBID, sizeof(INT64), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, Gift.wszSenderName, sizeof(WCHAR) * MAILNAMELENMAX, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Gift.nItemSN, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, wszItemID, sizeof(WCHAR) * 512, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, wszItemOptions, sizeof(WCHAR) * 512, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &OrderDate, sizeof(TIMESTAMP_STRUCT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, Gift.wszMessage, sizeof(WCHAR) * GIFTMESSAGEMAX, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &GiftExpireDate, sizeof(TIMESTAMP_STRUCT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &bGiftReturn, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &Gift.bNewFlag, sizeof(bool), &cblen);
			CheckColumnCount(nNo, "[Membership]P_GetListGiftBox");
			while(1)
			{
				memset(&Gift, 0, sizeof(Gift));
				memset(&OrderDate, 0, sizeof(OrderDate));
				memset(&GiftExpireDate, 0, sizeof(GiftExpireDate));				
				memset(&wszItemID, 0, sizeof(wszItemID));
				memset(&wszItemOptions, 0, sizeof(wszItemOptions));
				biSenderCharacterDBID = 0;
				bGiftReturn = false;

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA){
					nResult = ERROR_NONE;
					break;
				}
				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				CTimeParamSet OrderTime(&QueryTimeLog, Gift.tOrderDate, OrderDate, 0, 0, biCharacterDBID);
				if (!OrderTime.IsValid())
				{
					nResult = ERROR_DB;
					break;									
				}
				CTimeParamSet ExpireTime(&QueryTimeLog, Gift.tGiftExpireDate, GiftExpireDate, 0, 0, biCharacterDBID);
				if (!ExpireTime.IsValid())
				{
					nResult = ERROR_DB;
					break;									
				}

				std::vector<std::wstring> IDTokens, OptionTokens;
				TokenizeW(wszItemID, IDTokens, L",");
				int nItemID = 0;
				if (!IDTokens.empty())
					nItemID = _wtoi(IDTokens[0].c_str());

				TokenizeW(wszItemOptions, OptionTokens, L",");
				int nItemOptions = 0;
				if (!OptionTokens.empty())
					nItemOptions = _wtoi(OptionTokens[0].c_str());
				
				Gift.nItemID = nItemID;
				Gift.cItemOption = nItemOptions;
#if defined(PRE_ADD_GIFT_RETURN)
				Gift.bGiftReturn = bGiftReturn;
#endif
				GiftBoxList[cCount] = Gift;
				cCount++;

				if (cCount >= GIFTBOXLISTMAX){
					nResult = ERROR_NONE;
					break;
				}
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryGetListItemOfGift(INT64 biPurchaseOrderID, OUT int &nItemSN, OUT int &nItemPrice, OUT std::vector<DBPacket::TItemIDOption> &VecItemList)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetListItemOfGift" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryGetListItemOfGift] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	int nVersion = 2;
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListItemOfGift(%I64d,%d)}", biPurchaseOrderID, nVersion);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
			int nNo = 1;
			int nItemID = 0;
			char cOption = -1;
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nItemSN, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nItemPrice, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nItemID, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &cOption, sizeof(BYTE), &cblen);
			CheckColumnCount(nNo, "[Membership]P_GetListItemOfGift");
			while(1)
			{
				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA)
				{					
					nResult = ERROR_NONE;
					break;
				}

				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO) )
					break;

				if (nItemID > 0){
					DBPacket::TItemIDOption IDOption = {0,};
					IDOption.nItemID = nItemID;
					IDOption.cOption = cOption;
					VecItemList.push_back(IDOption);
				}
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryModGiftReceiveFlag(INT64 biPurchaseOrderID, OUT INT64 &biReplySenderCharacterDBID, OUT WCHAR *pReplyReceiverCharName, OUT INT64 &biGiftSenderCharacterDBID, OUT int &nPaidCashAmount)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModGiftReceiveFlag" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryModGiftReceiveFlag] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModGiftReceiveFlag(%I64d,?,?,?,?)}", biPurchaseOrderID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biReplySenderCharacterDBID, sizeof(INT64), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_WCHAR, SQL_WVARCHAR, sizeof(WCHAR) * NAMELENMAX, 0, pReplyReceiverCharName, sizeof(WCHAR) * NAMELENMAX, &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &biGiftSenderCharacterDBID, sizeof(INT64), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nPaidCashAmount, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

#if defined(PRE_ADD_GIFT_RETURN)
int CDNSQLMembership::QueryModGiftRejectFlag(INT64 biPurchaseOrderID, TAGiftReturn* pGiftReturn)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModGiftRejectFlag" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, L"[QueryModGiftRejectFlag] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModGiftRejectFlag(%I64d,1,?,?,?)}", biPurchaseOrderID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);		
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &pGiftReturn->nReceiverAccountDBID, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(INT64), 0, &pGiftReturn->biReceiverCharacterDBID, sizeof(INT64), &cblen);
		SQLBindParameter(m_hstmt, 4, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &pGiftReturn->nReceiverTotalMailCount, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}
#endif //#if defined(PRE_ADD_GIFT_RETURN)


int CDNSQLMembership::QueryModCharacterName(TQChangeCharacterName* pQ, TAChangeCharacterName* pA)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModCharacterName" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, pQ->nCharacterDBID, 0, L"[QueryModCharacterName] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset( m_wszQuery, 0, sizeof(m_wszQuery) );
	swprintf( m_wszQuery, L"{?=CALL dbo.P_ModCharacterName(%I64d,N'%s',%d,%d,%d,%d,?)}", 
				pQ->nCharacterDBID, 
				pQ->wszCharacterName, 
				pQ->btChangeCode, 
				10080,				// For 7 days
				pQ->nReservedPeriodDay, 
				pQ->nNextPeriodDay);
	
	TIMESTAMP_STRUCT NextDate = {0,};
	
	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);		
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP,	SQL_TIMESTAMP_LEN, 0, &NextDate, sizeof(TIMESTAMP_STRUCT), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode( RetCode, L"SQLExecute" );
		CTimeParamSet NextTime(&QueryTimeLog, pA->tNextChangeDate, NextDate, pQ->cWorldSetID, pQ->nAccountDBID, pQ->nCharacterDBID);
		if (!NextTime.IsValid()) return ERROR_DB;
	}

	return nResult;
}

int CDNSQLMembership::QueryIncreaseCharacterMaxCount(UINT nAccountDBID, char cAddCount, char cLimitCount, char &cCharMaxCount)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_IncreaseCharacterMaxCount" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryIncreaseCharacterMaxCount] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_IncreaseCharacterMaxCount(%d,%d,%d,?)}", nAccountDBID, cAddCount, cLimitCount);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cCharMaxCount, sizeof(char), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryGetCharacterMaxCount(UINT nAccountDBID, char &cCharMaxCount)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetCharacterMaxCount" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryGetCharacterMaxCount] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetCharacterMaxCount(%d,?)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cCharMaxCount, sizeof(char), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryModCharacterSlotCount(INT64 biCharacterDBID, char cAddCount, char &cCharMaxCount)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModCharacterSlotCount" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, biCharacterDBID, 0, L"[QueryModCharacterSlotCount] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModCharacterSlotCount(%I64d,%d,?)}", biCharacterDBID, cAddCount);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cCharMaxCount, sizeof(char), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryGetCharacterSlotCount(UINT nAccountDBID, int nWorldID, INT64 biCharacterDBID, char &cCharMaxCount)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetCharacterSlotCount" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, nWorldID, nAccountDBID, biCharacterDBID, 0, L"[QueryGetCharacterSlotCount] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	CDNSqlLen SqlLen[2];

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetCharacterSlotCount(%I64d,?,?,?)}", biCharacterDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(char), 0, &cCharMaxCount, sizeof(char), &cblen);
		SQLBindParameter(m_hstmt, 3, SQL_PARAM_INPUT, SQL_INTEGER, SQL_INTEGER, sizeof(UINT), 0, &nAccountDBID, sizeof(UINT), SqlLen[0].GetNull(nAccountDBID <= 0, sizeof(UINT)));
		SQLBindParameter(m_hstmt, 4, SQL_PARAM_INPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nWorldID, sizeof(int), SqlLen[1].GetNull(nWorldID <= 0, sizeof(int)));

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

#if defined(PRE_ADD_PETALTRADE)
int CDNSQLMembership::QueryPurchaseTradeItemByPetal(INT64 biCharacterDBID, int nPetalPrice)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_PurchaseTradeItemByPetal" );

	memset( m_wszQuery, 0, sizeof(m_wszQuery) );
	swprintf( m_wszQuery, L"{?=CALL dbo.P_PurchaseTradeItemByPetal(%I64d,%d)}", biCharacterDBID, nPetalPrice);

	return CommonReturnValueQuery(m_wszQuery);
}
#endif

#if defined(PRE_SPECIALBOX)
int CDNSQLMembership::QueryAddEventReward(TQAddEventReward *pQ, TAAddEventReward *pA)
{
	CQueryTimeLog QueryTimeLog("[Membership]P_AddEventReward");

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, pQ->biCharacterDBID, 0, L"[QueryAddEventReward] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	CDNSqlLen SqlLen[2];

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	std::wstring wstrAccountDBID, wstrAccountName, wstrCharacterDBID, wstrCharacterName, wstrItems;
	g_pExtManager->GetXMLAccountDBID(pQ->nAccountDBID, wstrAccountDBID);
	g_pExtManager->GetXMLAccountName(pQ->wszAccountName, wstrAccountName);
	g_pExtManager->GetXMLCharacterDBID(pQ->biCharacterDBID, wstrCharacterDBID);
	g_pExtManager->GetXMLCharacterName(pQ->wszCharacterName, wstrCharacterName);
	g_pExtManager->GetXMLItems(pQ->cCount, pQ->EventRewardItems, wstrItems);
	int nExpirationPeriod = g_pExtManager->GetGlobalWeightIntValue(SpecialBoxKeepPeriod);

	std::wstring wstrEventName = FormatW(L"%d", pQ->nEventNameUIString);
	std::wstring wstrSenderName = FormatW(L"%d", pQ->nSenderNameUIString);
	std::wstring wstrContent = FormatW(L"%d", pQ->nContentUIString);

	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddEventReward(%d,%d,N'%s',N'%s',N'%s',N'%s',%d,%d,%d,%d,%d,N'%s',N'%s',N'%s',%d,N'%s',%d,?)}",
		pQ->cReceiveTypeCode, pQ->cTargetTypeCode, wstrAccountDBID.c_str(), wstrAccountName.c_str(), wstrCharacterDBID.c_str(), wstrCharacterName.c_str(), pQ->cWorldSetID, pQ->cTargetClassCode,
		pQ->cTargetMinLevel, pQ->cTargetMaxLevel, nExpirationPeriod, wstrEventName.c_str(), wstrSenderName.c_str(), wstrContent.c_str(), pQ->nRewardCoin, wstrItems.c_str(), pQ->cEventRewardTypeCode);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		int nNo = 1;
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, nNo++, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &pA->nEventRewardID, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

int CDNSQLMembership::QueryGetCountEventReward(int nAccountDBID, INT64 biCharacterDBID, int &nEventTotalCount)
{
	CQueryTimeLog QueryTimeLog("[Membership]P_GetCountEventReward");

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, biCharacterDBID, 0, L"[QueryGetCountEventReward] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	nEventTotalCount = 0;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetCountEventReward(%d,%I64d)}", nAccountDBID, biCharacterDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
			int nNo = 1;
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nEventTotalCount, sizeof(int), &cblen);

			CheckColumnCount(nNo, "[Membership]P_GetCountEventReward");
			while(1)
			{
				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA)
				{					
					nResult = ERROR_NONE;
					break;
				}

				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO) )
					break;
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryGetListEventReward(TQGetListEventReward *pQ, TAGetListEventReward *pA)
{
	CQueryTimeLog QueryTimeLog("[Membership]P_GetListEventReward");
	
	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, pQ->biCharacterDBID, 0, L"[QueryGetListEventReward] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListEventReward(%d,%I64d,%d,%d,%d)}", pQ->nAccountDBID, pQ->biCharacterDBID, pQ->cWorldSetID, pQ->cClassCode, pQ->nLevel);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
			int nNo = 1;
			TSpecialBoxInfo BoxInfo = {0,};
			TIMESTAMP_STRUCT ReserveDate, ExpirationDate;
			WCHAR wszEventName[MAILTITLELENMAX];

			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &BoxInfo.nEventRewardID, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &BoxInfo.cReceiveTypeCode, sizeof(BYTE), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &BoxInfo.cTargetTypeCode, sizeof(BYTE), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &BoxInfo.cWorldID, sizeof(BYTE), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &BoxInfo.cTargetClassCode, sizeof(BYTE), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &BoxInfo.cTargetMinLevel, sizeof(BYTE), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &BoxInfo.cTargetMaxLevel, sizeof(BYTE), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &ReserveDate, sizeof(TIMESTAMP_STRUCT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &ExpirationDate, sizeof(TIMESTAMP_STRUCT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, wszEventName, sizeof(WCHAR) * MAILTITLELENMAX, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, BoxInfo.wszSenderName, sizeof(WCHAR) * NAMELENMAX, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_WCHAR, BoxInfo.wszContent, sizeof(WCHAR) * MAILTEXTLENMAX, &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_C_SBIGINT, &BoxInfo.biRewardCoin, sizeof(INT64), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &BoxInfo.bSystemSendFlag, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &BoxInfo.cEventRewardTypeCode, sizeof(char), &cblen);

			CheckColumnCount(nNo, "[Membership]P_GetListEventReward");
			while(1)
			{
				memset(&BoxInfo, 0, sizeof(BoxInfo));
				memset(&ReserveDate, 0, sizeof(ReserveDate));
				memset(&ExpirationDate, 0, sizeof(ExpirationDate));
				memset(&wszEventName, 0, sizeof(wszEventName));

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA)
				{					
					nResult = ERROR_NONE;
					break;
				}

				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO) )
					break;

				CTimeParamSet ReserveTime(&QueryTimeLog, BoxInfo.tReserveSendDate, ReserveDate, pQ->cWorldSetID, pQ->nAccountDBID, pQ->biCharacterDBID);
				CTimeParamSet ExpirationTime(&QueryTimeLog, BoxInfo.tExpirationDate, ExpirationDate, pQ->cWorldSetID, pQ->nAccountDBID, pQ->biCharacterDBID);

				if (!ReserveTime.IsValid()){
					nResult = ERROR_DB;
					break;
				}
				if (!ExpirationTime.IsValid()){
					nResult = ERROR_DB;
					break;
				}

				pA->SpecialBoxInfo[pA->cCount] = BoxInfo;
				pA->cCount++;

				if (pA->cCount >= SpecialBox::Common::ListMax){
					nResult = ERROR_NONE;
					break;
				}
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryGetListEventRewardItem(TQGetListEventRewardItem *pQ, TAGetListEventRewardItem *pA)
{
	CQueryTimeLog QueryTimeLog("[Membership]P_GetListEventRewardItem");
	
	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, 0, 0, L"[QueryGetListEventRewardItem] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListEventRewardItem(%d)}", pQ->nEventRewardID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE){
			int nNo = 1;
			TSpecialBoxItemInfo BoxItemInfo = {0,};
			TIMESTAMP_STRUCT ExpireDate;
			INT64 biItemID = 0;
			int nIndex = 0;

			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nIndex, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &BoxItemInfo.bCashItem, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_C_SBIGINT, &biItemID, sizeof(INT64), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_SMALLINT, &BoxItemInfo.RewardItem.wCount, sizeof(short), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_SMALLINT, &BoxItemInfo.RewardItem.wDur, sizeof(USHORT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &BoxItemInfo.RewardItem.nRandomSeed, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT,	&BoxItemInfo.RewardItem.cLevel, sizeof(char), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT,	&BoxItemInfo.RewardItem.cPotential, sizeof(char), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &BoxItemInfo.RewardItem.bSoulbound, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT,	&BoxItemInfo.RewardItem.cSealCount, sizeof(char), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT,	&BoxItemInfo.RewardItem.cOption, sizeof(char), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &BoxItemInfo.RewardItem.nLifespan, sizeof(int), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_BIT, &BoxItemInfo.RewardItem.bEternity, sizeof(bool), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TIMESTAMP, &ExpireDate, sizeof(TIMESTAMP_STRUCT), &cblen);

			CheckColumnCount(nNo, "[Membership]P_GetListEventRewardItem");
			while(1)
			{
				memset(&BoxItemInfo, 0, sizeof(BoxItemInfo));
				memset(&ExpireDate, 0, sizeof(ExpireDate));
				biItemID = 0;

				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA)
				{					
					nResult = ERROR_NONE;
					break;
				}

				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO) )
					break;

				if(!BoxItemInfo.RewardItem.bEternity) 
				{
					CTimeParamSet ExpireTime(&QueryTimeLog, BoxItemInfo.RewardItem.tExpireDate, ExpireDate, pQ->cWorldSetID, pQ->nAccountDBID, 0);
					if (!ExpireTime.IsValid())
					{
						nResult = ERROR_DB;
						break;									
					}
				}
				else
					BoxItemInfo.RewardItem.tExpireDate = 0;

				BoxItemInfo.RewardItem.nItemID = (int)biItemID;

				pA->SpecialBoxItem[pA->cCount] = BoxItemInfo;
				pA->cCount++;

				if (pA->cCount >= SpecialBox::Common::RewardSelectMax){
					nResult = ERROR_NONE;
					break;
				}
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
}

int CDNSQLMembership::QueryAddEventRewardReceiver(int nAccountDBID, INT64 biCharacterDBID, int nEventRewardID)
{
	CQueryTimeLog QueryTimeLog("[Membership]P_AddEventRewardReceiver");
	
	memset(m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_AddEventRewardReceiver(%d,%d,%I64d)}", nEventRewardID, nAccountDBID, biCharacterDBID);

	return CommonReturnValueQuery(m_wszQuery);
}
#endif	// #if defined(PRE_SPECIALBOX)

#ifdef PRE_ADD_DOORS
int CDNSQLMembership::QueryGetAuthenticationFlag(UINT nAccountDBID, bool &bFlag)
{
	CQueryTimeLog QueryTimeLog( "[Membership] P_DRS_GetMobileAuthenticationFlag" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryGetAuthenticationFlag] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	char szAuthKey[Doors::Common::AuthenticationKey_LenMax];
	memset(szAuthKey, 0, sizeof(szAuthKey));

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_DRS_GetMobileAuthenticationFlag(%d,?)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_C_BIT, SQL_BIT, sizeof(bool), 0, &bFlag, sizeof(bool), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}
	return nResult;
}

int CDNSQLMembership::QueryGetDoorsAuthentication(UINT nAccountDBID, char * pszAuthKey)
{
	if (pszAuthKey == NULL)
		return ERROR_DB;

	CQueryTimeLog QueryTimeLog( "[Membership]P_DRS_IssueAccountAuthenticationKey" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryGetDoorsAuthentication] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;
	char szAuthKey[Doors::Common::AuthenticationKey_LenMax];
	memset(szAuthKey, 0, sizeof(szAuthKey));

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{?=CALL dbo.P_DRS_IssueAccountAuthenticationKey(%d,?)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_CHAR, SQL_VARCHAR, sizeof(szAuthKey), 0, szAuthKey, sizeof(szAuthKey), &cblen);
		
		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	_strcpy(pszAuthKey, Doors::Common::AuthenticationKey_LenMax, szAuthKey, Doors::Common::AuthenticationKey_LenMax);
	return nResult;
}

int CDNSQLMembership::QueryCancelDoorsAuthentication(UINT nAccountDBID)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_DRS_DelAccountAuthentication" );

	memset( m_wszQuery, 0, sizeof(m_wszQuery) );
	swprintf( m_wszQuery, L"{?=CALL dbo.P_DRS_DelAccountAuthentication(%d)}", nAccountDBID);

	return CommonReturnValueQuery(m_wszQuery);
}
#endif		//#ifdef PRE_ADD_DOORS

int CDNSQLMembership::QueryModCharacterSortCode(UINT nAccountDBID, BYTE cCharacterSortCode)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModCharacterSortCode" );

	memset(m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf( m_wszQuery, L"{?=CALL dbo.P_ModCharacterSortCode(%d,%d)}", nAccountDBID, cCharacterSortCode);

	return CommonReturnValueQuery(m_wszQuery);
}

int CDNSQLMembership::QueryGetCharacterSortCode(UINT nAccountDBID, BYTE &cCharacterSortCode)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetCharacterSortCode" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[QueryGetCharacterSortCode] Check Connect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetCharacterSortCode(%d,?)}", nAccountDBID);

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE){
		SQLBindParameter(m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen);
		SQLBindParameter(m_hstmt, 2, SQL_PARAM_OUTPUT, SQL_TINYINT, SQL_TINYINT, sizeof(BYTE), 0, &cCharacterSortCode, sizeof(BYTE), &cblen);

		RetCode = SQLExecute(m_hstmt);
		CheckRetCode(RetCode, L"SQLExecute");
	}

	return nResult;
}

#if defined(PRE_ADD_GAMEQUIT_REWARD)
int CDNSQLMembership::QueryModNewbieRewardFlag(UINT nAccountDBID, bool bRewardFlag)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModNewbieRewardFlag" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModNewbieRewardFlag(%d,%d)}", nAccountDBID, bRewardFlag);

	return CommonReturnValueQuery(m_wszQuery);
}
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)

#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
int CDNSQLMembership::QueryHeartbeat()
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_HeartBeat" );

	memset( m_wszQuery, 0, sizeof(m_wszQuery) );
	swprintf( m_wszQuery, L"{?=CALL dbo.P_HeartBeat}");

	return CommonReturnValueQuery(m_wszQuery);
}
#endif		//#if defined(PRE_ADD_HEARTBEAT_TO_SQL)

#if defined(PRE_ADD_WORLD_EVENT)
int CDNSQLMembership::QueryEventList(TQEventList * pQ, TAEventList * pA)
{
	CQueryTimeLog QueryTimeLog( "P_GetListEventStatus" );

	if(CheckConnect() < 0)
	{
		g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, 0, 0, L"[QueryEventList] Check Connect Fail\r\n");
		pA->nRetCode = ERROR_DB;
		return ERROR_DB;
	}	

	SQLRETURN RetCode;
	SQLLEN cblen;
	int nResult = ERROR_DB;

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));

	swprintf(m_wszQuery, L"{CALL dbo.P_GetListDNEvent}");

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);	
	if(CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		RetCode = SQLExecute(m_hstmt);
		if(CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			TEventInfo Info;
			memset(&Info, 0, sizeof(Info));

			TIMESTAMP_STRUCT beginTime = {0, }, endTime = {0, };
			memset(&beginTime, 0, sizeof(TIMESTAMP_STRUCT));
			memset(&endTime, 0, sizeof(TIMESTAMP_STRUCT));
			int nNo = 1;			
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.EventID, sizeof(UINT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.WroldID, sizeof(UINT), &cblen);			
			SQLBindCol(m_hstmt, nNo++, SQL_TYPE_TIMESTAMP, &beginTime, sizeof(TIMESTAMP_STRUCT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_TYPE_TIMESTAMP, &endTime, sizeof(TIMESTAMP_STRUCT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.EventType[0], sizeof(UINT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.EventType[1], sizeof(UINT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.EventType[2], sizeof(UINT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.Att[0], sizeof(UINT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.Att[1], sizeof(UINT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.Att[2], sizeof(UINT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.Att[3], sizeof(UINT), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &Info.Att[4], sizeof(UINT), &cblen);			
			CheckColumnCount(nNo, "P_GetListDNEvent");
			while(1)
			{
				memset(&Info, 0, sizeof(Info));
				memset(&beginTime, 0, sizeof(beginTime));
				memset(&endTime, 0, sizeof(endTime));

				RetCode = SQLFetch(m_hstmt);
				if(RetCode == SQL_NO_DATA || pA->cCount >= EVENTLISTMAX)
				{
					nResult = ERROR_NONE;
					break;					
				}
				if((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				CTimeParamSet BeginTime(&QueryTimeLog, Info._tBeginTime, beginTime, pQ->cWorldSetID, pQ->nAccountDBID);
				if (!BeginTime.IsValid())
				{
					nResult = ERROR_DB;
					break;
				}			

				CTimeParamSet EndTime(&QueryTimeLog, Info._tEndTime, endTime, pQ->cWorldSetID, pQ->nAccountDBID);
				if (!EndTime.IsValid())
				{
					nResult = ERROR_DB;
					break;
				}

				if( pQ->cWorldSetID == 0 || Info.WroldID == 0 || pQ->cWorldSetID == Info.WroldID )
				{
					memcpy(&pA->EventList[pA->cCount], &Info, sizeof(Info));
					pA->cCount++;
				}				

				if(pA->cCount >= EVENTLISTMAX){
					nResult = ERROR_NONE;
					break;					
				}
			}
			SQLCloseCursor(m_hstmt);
			return nResult;
		}
	}
	return ERROR_NONE;
}
#endif //#if defined(PRE_ADD_WORLD_EVENT)

#if defined( PRE_ADD_NEWCOMEBACK )
int CDNSQLMembership::QueryModComebackFlag(UINT nAccountDBID, bool bRewardComeback)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_ModReturnRewardFalg" );

	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_ModReturnRewardFalg(%d,%d)}", nAccountDBID, bRewardComeback);

	return CommonReturnValueQuery(m_wszQuery);
}
#endif

#if defined( PRE_WORLDCOMBINE_PVP )
int CDNSQLMembership::QueryUpdateWorldPvPRoom(TQUpdateWorldPvPRoom* pQ, TAUpdateWorldPvPRoom *pA)
{
	CQueryTimeLog QueryTimeLog( "[Membership]P_GetListSimpleConfigValue" );

	if (CheckConnect() < ERROR_NONE){
		g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[QueryUpdateWorldPvPRoom] CheckConnect Fail\r\n");
		return ERROR_DB;
	}

	SQLRETURN RetCode;
	SQLLEN cblen;

	int nResult = ERROR_DB;
	memset(&m_wszQuery, 0, sizeof(m_wszQuery));
	swprintf(m_wszQuery, L"{?=CALL dbo.P_GetListSimpleConfigValue(2)}");

	RetCode = SQLPrepare(m_hstmt, m_wszQuery, SQL_NTSL);
	if (CheckRetCode(RetCode, L"SQLPrepare") == ERROR_NONE)
	{
		SQLBindParameter( m_hstmt, 1, SQL_PARAM_OUTPUT, SQL_INTEGER, SQL_INTEGER, sizeof(int), 0, &nResult, sizeof(int), &cblen );	

		RetCode = SQLExecute(m_hstmt);
		if (CheckRetCode(RetCode, L"SQLExecute") == ERROR_NONE)
		{
			char cCode = 0;
			int nValue = 0;

			int nNo = 1;
			SQLBindCol(m_hstmt, nNo++, SQL_TINYINT, &cCode, sizeof(char), &cblen);
			SQLBindCol(m_hstmt, nNo++, SQL_INTEGER, &nValue, sizeof(int), &cblen);
			CheckColumnCount(nNo, "[Membership]P_GetListSimpleConfigValue");
			while(1)
			{
				RetCode = SQLFetch(m_hstmt);
				if (RetCode == SQL_NO_DATA){
					nResult = ERROR_NONE;
					break;
				}

				if ((CheckRetCode(RetCode, L"SQLFetch") < ERROR_NONE) &&(RetCode != SQL_SUCCESS_WITH_INFO)) break;

				if( cCode == 2 )
				{
					pA->bFlag = nValue > 0 ? true : false;;
					nResult = ERROR_NONE;
					break;
				}
			}
		}
	}

	SQLCloseCursor(m_hstmt);
	return nResult;
};
#endif