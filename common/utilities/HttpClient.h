/*//===============================================================

	<< HTTP CLIENT >>

	FILE : HTTPCLIENT.H, HTTPCLIENT.CPP
DESC : HTTP CLIENT class using WININET library
	INIT BUILT DATE : 2006. 02. 20
	LAST BUILT DATE : 2006. 02. 21

	P.S.>
- Supports One-Request,One-Response / Keep-Alive connection and send/recv modes
- One-Request,One-Response : Request and receive an HTTP resource from a specific URL in one shot (Open() -> RecvResponse() -> Close())
- Keep-Alive : Connect to a specific HTTP server once and request different resources repeatedly (Open() -> SendRequest() -> RecvResponse() -> Close())

Copyright (c) 2002~2007 B4nFter (b4nfter@gmail.com) and Co-Writers. All Rights Reserved.

*///===============================================================


#pragma once


///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

#include <TCHAR.H>
#include <WINDOWS.H>
#include <WININET.H>

#pragma comment(lib, "WININET.LIB")


///////////////////////////////////////////////////////////////////
//	DEFINE
///////////////////////////////////////////////////////////////////

#define	DF_DEFAULT_HTTP_PORTNUMBER	(80)

///////////////////////////////////////////////////////////////////
//	DECLARATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : CLASS
// NAME : CHttpClient
//*---------------------------------------------------------------
// DESC : Http Client class
// PRGM : B4nFter
//*---------------------------------------------------------------
class CHttpClient
{
public:
	enum EF_HTTP_CONNECTION_TYPE {
		EV_HTTP_CONNECTION_NONE			= 0,	// Initial value
		EV_HTTP_CONNECTION_ONEREQONERSP	= 1,	// HTTP connection mode: One-Req,One-Rsp
		EV_HTTP_CONNECTION_KEEPALIVE	= 2,	// HTTP connection mode: Keep-Alive
	};

public:
	CHttpClient();
	~CHttpClient();

	// Shared method for One-Req,One-Rsp / Keep-Alive
	BOOL Open(
		LPCTSTR lpszHttpServerAddressUrl,	// One-Req,One-Rsp : uses URL resource path only / Keep-Alive : server address (URL, DOT-DECIMAL)
		EF_HTTP_CONNECTION_TYPE eHttpConnectionMode = EV_HTTP_CONNECTION_ONEREQONERSP,
		INTERNET_PORT wHttpServerPortNumber = DF_DEFAULT_HTTP_PORTNUMBER,	// Used only when Keep-Alive is enabled
		LPCTSTR lpszUserName = NULL,		// Used only when Keep-Alive is enabled
		LPCTSTR lpszPassword = NULL			// Used only when Keep-Alive is enabled
		);	
	VOID Close();
	BOOL IsOpen();
	INT RecvResponse(LPBYTE lpbtBuffer, INT nBufferSize);
	INT RecvDownFile(LPCTSTR lpszFileName, LPBINDSTATUSCALLBACK pCallBack);
	DWORD SetOption(DWORD dwMaxConnCount, DWORD dwTimout);

	// Method exclusive to One-Req,One-Rsp

	// Keep-Alive-only method
	BOOL ReOpen();
	INT SendRequest(LPCTSTR lpszUrl, BOOL bCheckResultDataLength = FALSE, BOOL bAsyncRequest = FALSE);	
	INT SendRequsetPostA(LPCTSTR lpszUrl, LPCSTR szPostData); //Post for Ansi
	INT SendRequestGet(LPCTSTR lpszUrl);	

	LONG m_RefCount;
	LONG IncRefCount() { return(InterlockedIncrement(&m_RefCount)); }
	LONG DecRefCount() { return(InterlockedDecrement(&m_RefCount)); }

private:
	VOID Initialize();
	VOID Finalize();

private:
	// Shared member variable for One-Req,One-Rsp / Keep-Alive
	HINTERNET m_hInternet;				// Holds the handle from InternetOpen()
	HINTERNET m_hInternetConnection;	// One-Req,One-Rsp : holds InternetOpenUrl() handle / Keep-Alive : holds InternetConnect() handle
	HINTERNET m_hHttpRequest;			// Used only in Keep-Alive mode

	EF_HTTP_CONNECTION_TYPE m_eHttpConnectionMode;

	// Member variable exclusive to One-Req,One-Rsp

	// Keep-Alive-only member variable
	TCHAR m_szHttpServerAddress[MAX_PATH];
	INTERNET_PORT m_wHttpServerPortNumber;
	TCHAR m_szUserName[MAX_PATH];
	TCHAR m_szPassword[MAX_PATH];	
};
