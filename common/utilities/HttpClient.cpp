

///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HttpClient.h"
#include "CommonMacros.h"
#include "BaseSet.h"
#include "DebugUtil.h"
#include "SafeStringMacros.h"

#include "Log.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

///////////////////////////////////////////////////////////////////
//	IMPLEMENTATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::CHttpClient
//*---------------------------------------------------------------
// DESC : Constructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CHttpClient::CHttpClient()
{
	Initialize();
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::~CHttpClient
//*---------------------------------------------------------------
// DESC : Destructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CHttpClient::~CHttpClient()
{
	Finalize();
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::Initialize
//*---------------------------------------------------------------
// DESC : Initialize object-related resources
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CHttpClient::Initialize()
{
	m_hInternet = NULL;
	m_hInternetConnection = NULL;
	m_hHttpRequest = NULL;
	m_eHttpConnectionMode = EV_HTTP_CONNECTION_NONE;
	m_wHttpServerPortNumber = DF_DEFAULT_HTTP_PORTNUMBER;
	::memset(m_szHttpServerAddress, 0, sizeof(m_szHttpServerAddress));
	::memset(m_szUserName, 0, sizeof(m_szUserName));
	::memset(m_szPassword, 0, sizeof(m_szPassword));
	m_RefCount = 0;	
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::Finalize
//*---------------------------------------------------------------
// DESC : Release object-related resources
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CHttpClient::Finalize()
{
	Close();
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::Open
//*---------------------------------------------------------------
// DESC : Connect HTTP session to server / For One-Req,One-Rsp mode, immediately fetch URL resource
// PARM : 1 . lpszHttpServerAddressUrl - One-Req,One-Rsp : full URL of HTTP resource / Keep-Alive : HTTP server address (domain name without http://, dot-decimal)
//			2 . eHttpConnectionMode - EV_HTTP_CONNECTION_ONEREQONERSP : One-Req,One-Rsp mode (default) / EV_HTTP_CONNECTION_KEEPALIVE : Keep-Alive mode
//			3 . wHttpServerPortNumber - HTTP server port number (Keep-Alive mode only, default 80)
//			4. lpszUserName - (Keep-Alive mode only) HTTP server user account name (default NULL)
//			5. lpszPassword - (Keep-Alive mode only) Password for HTTP server user account (default NULL)
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CHttpClient::Open(
	LPCTSTR lpszHttpServerAddressUrl,
	EF_HTTP_CONNECTION_TYPE eHttpConnectionMode,
	INTERNET_PORT wHttpServerPortNumber,
	LPCTSTR lpszUserName,
	LPCTSTR lpszPassword
)
{
//	DN_ASSERT(!IsOpen(),	"Invalid!");

	if (lpszHttpServerAddressUrl == NULL || !_tcscmp(lpszHttpServerAddressUrl, _T(""))) {
//		DN_TRACE(_T("lpszHttpServerAddressUrl == NULL || !_tcscmp(lpszHttpServerAddressUrl, _T(\"\")) - CHttpClient::Open()\n"));
		return FALSE;
	}

	if (IsOpen()) {
		Close();
	}

	switch(eHttpConnectionMode) {
	case EV_HTTP_CONNECTION_ONEREQONERSP:	// In the case of One-Req,One-Rsp
		{
			m_hInternet = ::InternetOpen(_T("Microsoft Internet Explorer"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL); 
			if (m_hInternet == NULL) {
//				DN_TRACE(_T("::InternetOpen() == NULL : %d - CHttpClient::Open()\n"), GetLastError());
				return FALSE;
			}		
			
			m_hInternetConnection = ::InternetOpenUrl(m_hInternet, lpszHttpServerAddressUrl, NULL, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_CACHE_WRITE, 0);
			if (m_hInternetConnection == NULL) {
				::InternetCloseHandle(m_hInternet);
				m_hInternet = NULL;
//				DN_TRACE(_T("::InternetOpenUrl() == NULL : %d - CHttpClient::Open()\n"), GetLastError());
				return FALSE;
			}
			
/*
			DWORD dwNumberOfBytesAvailable = 0;
			DWORD dwSize = sizeof(dwNumberOfBytesAvailable);
			DWORD dwStatus = 0;
			if (::HttpQueryInfo(m_hInternetConnection, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwNumberOfBytesAvailable, &dwSize, &dwStatus) ==FALSE 
				|| dwNumberOfBytesAvailable == 0)
			{
//				DN_TRACE(_T("dwNumberOfBytesAvailable : %u - CHttpClient::Open()\n"), dwNumberOfBytesAvailable);
				return FALSE;
			}
*/

			m_eHttpConnectionMode = EV_HTTP_CONNECTION_ONEREQONERSP;
		}
		return TRUE;
	case EV_HTTP_CONNECTION_KEEPALIVE:		// When Keep-Alive is used
		{
//			STRNCPY(m_szHttpServerAddress, lpszHttpServerAddressUrl, COUNT_OF(m_szHttpServerAddress));
			_tcsncpy_s(m_szHttpServerAddress, lpszHttpServerAddressUrl, _countof(m_szHttpServerAddress));
//			_tcsncpy(m_szHttpServerAddress, lpszHttpServerAddressUrl, COUNT_OF(m_szHttpServerAddress));
			m_wHttpServerPortNumber = wHttpServerPortNumber;

			if (lpszUserName == NULL) {
				m_szUserName[0] = _T('\0');
			}
			else {
//				STRNCPY(m_szUserName, lpszUserName, COUNT_OF(m_szUserName));
				_tcsncpy_s(m_szUserName, lpszUserName, _countof(m_szUserName));
//				_tcsncpy(m_szUserName, lpszUserName, COUNT_OF(m_szUserName));
			}

			if (lpszPassword == NULL) {
				m_szPassword[0] = _T('\0');
			}
			else {
//				STRNCPY(m_szPassword, lpszPassword, COUNT_OF(m_szPassword));
				_tcsncpy_s(m_szPassword, lpszPassword, _countof(m_szPassword));
//				_tcsncpy(m_szPassword, lpszPassword, COUNT_OF(m_szPassword));
			}
			m_hInternet = ::InternetOpen(_T("Microsoft Internet Explorer"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);

			if (m_hInternet == NULL) {
//				DN_TRACE(_T("::InternetOpen() == NULL : %d - CHttpClient::Open()\n"), GetLastError());
				return FALSE;
			}			

			m_hInternetConnection = ::InternetConnect(
				m_hInternet,
				m_szHttpServerAddress,
				m_wHttpServerPortNumber
				,m_szUserName, m_szPassword	,INTERNET_SERVICE_HTTP ,0 ,0);
			if (m_hInternetConnection == NULL) {
//				DN_TRACE(_T("::InternetConnect() == NULL : %d - CHttpClient::Open()\n"), GetLastError());
				return FALSE;
			}

			m_eHttpConnectionMode = EV_HTTP_CONNECTION_KEEPALIVE;
		}
		return TRUE;
	default:	// Error
		{
			m_eHttpConnectionMode = EV_HTTP_CONNECTION_NONE;
//			DN_TRACE(_T("Unknown Http Connection Type : %d - CHttpClient::Open()\n"), eHttpConnectionMode);
		}
		break;
	}

	return FALSE;
}

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::IsOpen
//*---------------------------------------------------------------
// DESC : Check whether HTTP session is connected
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CHttpClient::IsOpen()
{
	if ((m_hInternet == NULL) ||
		((m_eHttpConnectionMode != EV_HTTP_CONNECTION_ONEREQONERSP) && (m_eHttpConnectionMode != EV_HTTP_CONNECTION_KEEPALIVE))
	)
	{
		return FALSE;
	}

	return TRUE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::Close
//*---------------------------------------------------------------
// DESC : Close HTTP session connection and release resources
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CHttpClient::Close()
{
	if (m_hHttpRequest) {
		::InternetCloseHandle(m_hHttpRequest);
		m_hHttpRequest = NULL;
	}
	if (m_hInternetConnection) {
		::InternetCloseHandle(m_hInternetConnection);
		m_hInternetConnection = NULL;
	}
	if (m_hInternet) {
		::InternetCloseHandle(m_hInternet);
		m_hInternet = NULL;
	}
	m_eHttpConnectionMode = EV_HTTP_CONNECTION_NONE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::Close
//*---------------------------------------------------------------
// DESC : Re-establish HTTP session connection (Keep-Alive mode only)
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CHttpClient::ReOpen()
{
// 	DN_ASSERT(IsOpen(),	"Invalid!");
// 	DN_ASSERT(EV_HTTP_CONNECTION_KEEPALIVE == m_eHttpConnectionMode,	"Invalid!");
// 	DN_ASSERT(_tcscmp(m_szHttpServerAddress, _T("")),					"Invalid!");

	if (!IsOpen() || 
		EV_HTTP_CONNECTION_KEEPALIVE != m_eHttpConnectionMode ||
		!_tcscmp(m_szHttpServerAddress, _T(""))
		)
	{
		g_Log.Log(LogType::_ERROR, L"IsOpen() : %d / m_eHttpConnectionMode : %d / m_szHttpServerAddress : %s - CHttpClient::Open()\n", IsOpen(), m_eHttpConnectionMode, m_szHttpServerAddress);
// 		DN_TRACE(_T("IsOpen() : %d / m_eHttpConnectionMode : %d / m_szHttpServerAddress : %s - CHttpClient::Open()\n"), IsOpen(), m_eHttpConnectionMode, m_szHttpServerAddress);
// 		DN_RETURN(FALSE);
		return FALSE;
	}

	Close();

	return Open(m_szHttpServerAddress, EV_HTTP_CONNECTION_KEEPALIVE, m_wHttpServerPortNumber, m_szUserName, m_szPassword);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::SendRequest
//*---------------------------------------------------------------
// DESC : Request resource at relative path on server after connecting to HTTP server (Keep-Alive mode only)
// PARM : 1 . lpszUrl - URL representing relative resource path from HTTP server
//			2 . bCheckResultDataLength - whether to receive resource length on request success
//			3 . bAsyncRequest - 
// RETV : < 0 : failure / >= 0 : success; if bCheckResultDataLength == TRUE and >= 0, size of resource to receive
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CHttpClient::SendRequest(LPCTSTR lpszUrl, BOOL bCheckResultDataLength, BOOL bAsyncRequest)
{
// 	DN_ASSERT(IsOpen(),			"Invalid!");
// 	DN_ASSERT(NULL != lpszUrl,	"Invalid!");

	DWORD dwNumberOfBytesAvailable = 0;

	if (lpszUrl == NULL || !_tcscmp(lpszUrl, _T(""))) {
// 		DN_TRACE(_T("lpszUrl == NULL || !_tcscmp(lpszUrl, _T(\"\")) - CHttpClient::SendRequest()"));
		return -1;
	}

	if (IsOpen() == FALSE && ReOpen() == FALSE) {
// 		DN_TRACE(_T("IsOpen() == FALSE && ReOpen() == FALSE"));
		return -1;
	}

	if (m_hHttpRequest)	{
		::InternetCloseHandle(m_hHttpRequest);
		m_hHttpRequest = NULL;
	}
	m_hHttpRequest = ::HttpOpenRequest(m_hInternetConnection, _T("GET"), lpszUrl, _T("HTTP/1.1"), NULL, NULL,
		INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_KEEP_CONNECTION 
		| INTERNET_FLAG_DONT_CACHE 
		| INTERNET_FLAG_NO_AUTH
		| INTERNET_FLAG_PRAGMA_NOCACHE
		| INTERNET_FLAG_NO_COOKIES,
		0
		);	
	if (m_hHttpRequest == NULL) {
// 		DN_TRACE(_T("::HttpOpenRequest() == NULL : %d - CHttpClient::SendRequest()"), GetLastError());
		return -1;
	}

	BOOL bRetVal;
	if (!bAsyncRequest) {
		bRetVal = ::HttpSendRequest(m_hHttpRequest, NULL, 0, NULL, 0);
	}
	else {
		bRetVal = ::HttpSendRequestEx(m_hHttpRequest, NULL, NULL, HSR_ASYNC, 0);
	}
	if (bRetVal == FALSE) {
		::InternetCloseHandle(m_hHttpRequest);
		m_hHttpRequest = NULL;
// 		DN_TRACE(_T("::HttpSendRequest() == FALSE : %d, m_eHttpConnectionMode : %d - CHttpClient::RecvResponse()\n"), GetLastError(), m_eHttpConnectionMode);
		return -1;
	}

	if (bCheckResultDataLength) {
		DWORD dwSize = sizeof(dwNumberOfBytesAvailable);
		DWORD dwStatus = 0;
		if (::HttpQueryInfo(m_hHttpRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwNumberOfBytesAvailable, &dwSize, &dwStatus) == FALSE 
			|| dwNumberOfBytesAvailable == 0)
		{
			::InternetCloseHandle(m_hHttpRequest);
			m_hHttpRequest = NULL;
			return -1;
		}
	}

	return (INT)dwNumberOfBytesAvailable;
}

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::SendRequest
//*---------------------------------------------------------------
// DESC : Request resource at relative path on server after connecting to HTTP server (Keep-Alive mode only)
// PARM : 1 . lpszUrl - URL representing relative resource path from HTTP server
//			2 . szPostData - PostData
// RETV : < 0 : failure / >= 0 : success; if bCheckResultDataLength == TRUE and >= 0, size of resource to receive
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CHttpClient::SendRequsetPostA(LPCTSTR lpszUrl, LPCSTR szPostData)
{
	if (lpszUrl == NULL || !_tcscmp(lpszUrl, _T(""))) {
		return -1;
	}

	if (IsOpen() == FALSE && ReOpen() == FALSE) {
		return -1;
	}

	if (m_hHttpRequest)	{
		::InternetCloseHandle(m_hHttpRequest);
		m_hHttpRequest = NULL;
	}	
	m_hHttpRequest = ::HttpOpenRequest(m_hInternetConnection, _T("POST"), lpszUrl, _T("HTTP/1.1"), NULL, NULL,
		INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_KEEP_CONNECTION 
		| INTERNET_FLAG_DONT_CACHE 
		| INTERNET_FLAG_NO_AUTH
		| INTERNET_FLAG_PRAGMA_NOCACHE
		| INTERNET_FLAG_NO_COOKIES,
		0
		);	
	if (m_hHttpRequest == NULL) {
		return -1;
	}
	int nPostLength = 0, nHeaderLength = 0;
	char* strHeader = NULL;
	if( szPostData )
	{
		strHeader = "Content-Type: application/x-www-form-urlencoded";
		nPostLength = (int)strlen(szPostData);
		nHeaderLength = (int)strlen(strHeader);
	}

	BOOL bRetVal = ::HttpSendRequestA(m_hHttpRequest, strHeader, nHeaderLength, (LPVOID)szPostData, nPostLength);	
	if (bRetVal == FALSE) {
		::InternetCloseHandle(m_hHttpRequest);
		m_hHttpRequest = NULL;
// 		DN_TRACE(_T("::HttpSendRequest() == FALSE : %d, m_eHttpConnectionMode : %d - CHttpClient::RecvResponse()\n"), GetLastError(), m_eHttpConnectionMode);
		return -1;
	}
	INT nStatus =0;
	DWORD dwBuffSize = sizeof (INT) ;

	if ( ::HttpQueryInfo (
		m_hHttpRequest
		, HTTP_QUERY_STATUS_CODE		// Get the HTTP status code
		| HTTP_QUERY_FLAG_NUMBER
		, static_cast<void *> (&nStatus)
		, &dwBuffSize					// Buffer size (byte)
		, NULL							// Does not use a header index
		) == FALSE )
	{
		return -1;
	}
	return nStatus ;	
}

INT CHttpClient::SendRequestGet(LPCTSTR lpszUrl)
{
	DWORD dwNumberOfBytesAvailable = 0;

	if (lpszUrl == NULL || !_tcscmp(lpszUrl, _T(""))) {
		// 		DN_TRACE(_T("lpszUrl == NULL || !_tcscmp(lpszUrl, _T(\"\")) - CHttpClient::SendRequest()"));
		return -1;
	}

	if (IsOpen() == FALSE && ReOpen() == FALSE) {
		// 		DN_TRACE(_T("IsOpen() == FALSE && ReOpen() == FALSE"));
		return -1;
	}

	if (m_hHttpRequest)	{
		::InternetCloseHandle(m_hHttpRequest);
		m_hHttpRequest = NULL;
	}
	m_hHttpRequest = ::HttpOpenRequest(m_hInternetConnection, _T("GET"), lpszUrl, _T("HTTP/1.1"), NULL, NULL,
		INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_KEEP_CONNECTION 
		| INTERNET_FLAG_DONT_CACHE 
		//| INTERNET_FLAG_NO_AUTH
		| INTERNET_FLAG_PRAGMA_NOCACHE
		| INTERNET_FLAG_NO_COOKIES,
		0
		);	
	if (m_hHttpRequest == NULL) {
		// 		DN_TRACE(_T("::HttpOpenRequest() == NULL : %d - CHttpClient::SendRequest()"), GetLastError());
		return -1;
	}

	BOOL bRetVal = ::HttpSendRequest(m_hHttpRequest, NULL, 0, NULL, 0);	
	if (bRetVal == FALSE) {
		::InternetCloseHandle(m_hHttpRequest);
		m_hHttpRequest = NULL;
		// 		DN_TRACE(_T("::HttpSendRequest() == FALSE : %d, m_eHttpConnectionMode : %d - CHttpClient::RecvResponse()\n"), GetLastError(), m_eHttpConnectionMode);
		return -1;
	}	
	INT nStatus =0;
	DWORD dwBuffSize = sizeof (INT) ;

	if ( ::HttpQueryInfo (
		m_hHttpRequest
		, HTTP_QUERY_STATUS_CODE		// Get the HTTP status code
		| HTTP_QUERY_FLAG_NUMBER
		, static_cast<void *> (&nStatus)
		, &dwBuffSize					// Buffer size (byte)
		, NULL							// Does not use a header index
		) == FALSE )
	{
		return -1;
	}
	return nStatus;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::RecvResponse
//*---------------------------------------------------------------
// DESC : Receive data after successful resource request to HTTP server (One-Req,One-Rsp: after Open() / Keep-Alive: after SendRequest())
// PARM : 1 . lpbtBuffer - Pointer to buffer to receive data
//			2 . uBufferSize - buffer size to receive data
// RETV : < 0 : failure / 0 : success, receive complete / > 0 : success, data remaining to receive
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CHttpClient::RecvResponse(LPBYTE lpbtBuffer, INT nBufferSize)
{
// 	DN_ASSERT(IsOpen(),			"Invalid!");
// 	DN_ASSERT(NULL != lpbtBuffer,	"Invalid!");
// 	DN_ASSERT(0 < uBufferSize,	"Invalid!");

	if (lpbtBuffer == NULL || nBufferSize == 0) {
// 		DN_TRACE(_T("lpbtBuffer == NULL || uBufferSize == 0 - CHttpClient::RecvResponse()\n"));
		return -1;
	}

	if (!IsOpen()) {
// 		DN_TRACE(_T("!IsOpen() - CHttpClient::RecvResponse()\n"));
		return -1;
	}

	HINTERNET hInternetFile = NULL;

	switch(m_eHttpConnectionMode) {
	case EV_HTTP_CONNECTION_ONEREQONERSP:	// In the case of One-Req,One-Rsp
		{
			hInternetFile = m_hInternetConnection;
		}
		break;
	case EV_HTTP_CONNECTION_KEEPALIVE:		// When Keep-Alive is used
		{
			hInternetFile = m_hHttpRequest;
		}
		break;
	default:	// Error
		{
// 			DN_TRACE(_T("Unknown Http Connection Type : %d - CHttpClient::RecvResponse()\n"), m_eHttpConnectionMode);
		}
		return -1;
	}

	if (hInternetFile == NULL) {
// 		DN_TRACE(_T("hInternetFile == NULL - CHttpClient::RecvResponse()\n"));
		return -1;
	}

	DWORD dwNumberOfBytesAvailable, dwIoBytes, dwBufferOffset;
	dwNumberOfBytesAvailable = dwIoBytes = dwBufferOffset = 0;

	BOOL bRetVal = ::InternetQueryDataAvailable(hInternetFile, &dwNumberOfBytesAvailable, 0, 0);
	if (bRetVal == FALSE) {
		g_Log.Log(LogType::_ERROR, L"::InternetQueryDataAvailable() : %d - CHttpClient::RecvResponse()\n", GetLastError());
// 		DN_TRACE(_T("::InternetQueryDataAvailable() : %d - CHttpClient::RecvResponse()\n"), GetLastError());
		return -1;
	}

	int nCount = 0;
	while(dwNumberOfBytesAvailable > 0) {
		if ( nBufferSize - dwBufferOffset < dwNumberOfBytesAvailable ) 
		{
			if( nBufferSize == dwBufferOffset )
			{
				// Buffer completely insufficient.
				g_Log.Log(LogType::_ERROR, L"Buffer Empty!! nBufferSize:%d, dwBufferOffset:%u, dwNumberOfBytesAvailable:%u \n", nBufferSize, dwBufferOffset, dwNumberOfBytesAvailable);
				return 1;
			}
			// If buffer is insufficient, recalculate receive size to fit the available buffer
			dwNumberOfBytesAvailable = nBufferSize - dwBufferOffset;
			g_Log.Log(LogType::_ERROR, L"(nBufferSize:%d - dwBufferOffset:%u) < dwNumberOfBytesAvailable:%u - CHttpClient::RecvResponse()\n", nBufferSize, dwBufferOffset, dwNumberOfBytesAvailable);
//			DN_TRACE(_T("((INT)uBufferSize - (INT)dwBufferOffset) < (INT)dwNumberOfBytesAvailable - CHttpClient::RecvResponse()\n"));			
		}

		if ((INT)dwNumberOfBytesAvailable <= 0) {
			// Insufficient buffer - not an error, means allocate more buffer space
//			DN_TRACE(_T("(INT)dwNumberOfBytesAvailable <= 0 - CHttpClient::RecvResponse()\n"));
			return 1;
		}

		bRetVal = ::InternetReadFile(hInternetFile, lpbtBuffer + dwBufferOffset, dwNumberOfBytesAvailable, &dwIoBytes);
		if (bRetVal == FALSE) {
//			DN_TRACE(_T("::InternetReadFile() == FALSE : %d - CHttpClient::RecvResponse()\n"), GetLastError());
			return -1;
		}
		
		dwBufferOffset += dwIoBytes;
		
		bRetVal = ::InternetQueryDataAvailable(hInternetFile, &dwNumberOfBytesAvailable, 0, 0);
		if (bRetVal == FALSE) {
//			DN_TRACE(_T("::InternetQueryDataAvailable() == FALSE : %d - CHttpClient::RecvResponse()\n"), GetLastError());
			return -1;
		}
		++nCount;
		if( nCount > 20 )
		{
			// For tracking infinite loops..
			g_Log.Log(LogType::_ERROR, L"RecvResponse Loof nBufferSize:%d, dwBufferOffset:%u, dwNumberOfBytesAvailable:%u\n", nBufferSize, dwBufferOffset, dwNumberOfBytesAvailable);
			Close();
			return 1;
		}
	}

	if (dwNumberOfBytesAvailable == 0) {
		// All result data received successfully
		return 0;
	}
	
	// Unknown error - need to check if this result actually occurs
	return -1;
}

INT CHttpClient::RecvDownFile(LPCTSTR lpszFileName, LPBINDSTATUSCALLBACK pCallBack)
{
	FILE* downfile = NULL;
	downfile = _tfopen(lpszFileName, _T("w+b"));	

	if (!IsOpen()) {
		fclose(downfile);
		return -1;
	}

	HINTERNET hInternetFile = NULL;

	switch(m_eHttpConnectionMode) {
	case EV_HTTP_CONNECTION_ONEREQONERSP:	// In the case of One-Req,One-Rsp
		{
			hInternetFile = m_hInternetConnection;
		}
		break;
	case EV_HTTP_CONNECTION_KEEPALIVE:		// When Keep-Alive is used
		{
			hInternetFile = m_hHttpRequest;
		}
		break;
	default:	// Error
		{
			// 			DN_TRACE(_T("Unknown Http Connection Type : %d - CHttpClient::RecvResponse()\n"), m_eHttpConnectionMode);
		}
		return -1;
	}

	if (hInternetFile == NULL) {
		fclose(downfile);
		return -1;
	}	

	DWORD dwNumberOfBytesAvailable, dwTotalBytes, dwIoBytes, dwCurTotalBytes;
	dwNumberOfBytesAvailable = dwTotalBytes = dwIoBytes = dwCurTotalBytes = 0;	

	DWORD dwBufLen = sizeof(dwTotalBytes);

	BOOL bRetVal = ::HttpQueryInfo(hInternetFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&dwTotalBytes, &dwBufLen, 0);
	//	BOOL bRetVal = ::InternetQueryDataAvailable(hInternetFile, &dwNumberOfBytesAvailable, 0, 0);
	if (bRetVal == FALSE) {
		fclose(downfile);
		return -1;
	}
	char buf[1024*10]={0,};

	dwNumberOfBytesAvailable = dwTotalBytes;

	while(dwNumberOfBytesAvailable > 0)
	{		
		bRetVal = ::InternetReadFile(hInternetFile, buf, sizeof(buf), &dwIoBytes);
		if (bRetVal == FALSE) {
			fclose(downfile);
			return -1;
		}

		fwrite(buf, 1, dwIoBytes, downfile);
		dwCurTotalBytes += dwIoBytes;
		if( pCallBack )
			pCallBack->OnProgress( dwCurTotalBytes, dwTotalBytes, BINDSTATUS_DOWNLOADINGDATA, NULL);

		bRetVal = ::InternetQueryDataAvailable(hInternetFile, &dwNumberOfBytesAvailable, 0, 0);
		if (bRetVal == FALSE) {			
			fclose(downfile);
			return -1;
		}
	}

	if (dwNumberOfBytesAvailable == 0) {
		// All result data received successfully
		if( pCallBack )
			pCallBack->OnProgress( dwTotalBytes, dwTotalBytes, BINDSTATUS_ENDDOWNLOADDATA, NULL);		
		fclose(downfile);
		return 0;
	}
	return -1;
}

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CHttpClient::SetOption
//*---------------------------------------------------------------
// DESC : Set internet options needed for HTTP session (only supported when using wininet.lib, not winhttp.lib)
// PARM : 1 . dwMaxConnCount - Specify max number of HTTP requests
//			2 . dwTimout - HTTP request timeout
// RETV : < 0 : failure / 0 : success, receive complete / > 0 : success, data remaining to receive
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CHttpClient::SetOption(DWORD dwMaxConnCount, DWORD dwTimout)
{
// 	DN_ASSERT(0 < dwMaxConnCount,		"Invalid!");
// 	DN_ASSERT((1*1000) < dwTimout, "Should sending really complete within 1 second?");

	// Per HTTP spec, concurrent requests to an HTTP server are limited (1.0: 4, 1.1: 2)
	// If requests exceed this, the next one is blocked
	// P.S.> See article -> INFO: WinInet Limits Connections Per Server (Q183110)
	DWORD dwArgVal = dwMaxConnCount;
	if (FALSE == ::InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &dwArgVal, sizeof(dwArgVal)))
	{
//		DN_RETURN(::GetLastError());
		return(::GetLastError());
	}

	dwArgVal = dwMaxConnCount;
	if (FALSE == ::InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &dwArgVal, sizeof(dwArgVal)))
	{
//		DN_RETURN(::GetLastError());
		return(::GetLastError());
	}

	dwArgVal = dwTimout;
	if (FALSE == ::InternetSetOption(NULL, INTERNET_OPTION_SEND_TIMEOUT, &dwArgVal, sizeof(dwArgVal)))
	{
//		DN_RETURN(::GetLastError());
		return(::GetLastError());
	}

	dwArgVal = dwTimout;
	if (FALSE == ::InternetSetOption(NULL, INTERNET_OPTION_RECEIVE_TIMEOUT , &dwArgVal, sizeof(dwArgVal)))
	{
//		DN_RETURN(::GetLastError());
		return(::GetLastError());
	}

	// From Windows Server 2003 x64 (strictly Requires Internet Explorer 7.0.), IDN is used and MBCS gets corrupted
	// So if IDN is turned off, it must be sent using the system code page to avoid corruption.
	// Not supported on Windows XP SP2 and Windows Server 2003 SP1
/*
	dwArgVal = 0;
	if (FALSE == ::InternetSetOption(NULL, INTERNET_OPTION_IDN, &dwArgVal, sizeof(dwArgVal)))
	{
		dwArgVal = ::GetLastError();
		if (ERROR_CALL_NOT_IMPLEMENTED != dwArgVal)
		{
//			DN_RETURN(dwArgVal);
			return dwArgVal;
		}
	}
*/

	return(NOERROR);
}
