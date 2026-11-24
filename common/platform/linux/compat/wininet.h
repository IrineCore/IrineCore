#pragma once
#include "windows.h"
#ifndef INTERNET_OPEN_TYPE_PRECONFIG
#define INTERNET_OPEN_TYPE_PRECONFIG 0
#define INTERNET_OPEN_TYPE_DIRECT    1
#define INTERNET_DEFAULT_HTTP_PORT   80
#define INTERNET_DEFAULT_HTTPS_PORT  443
#define INTERNET_SERVICE_HTTP        3
#define INTERNET_FLAG_RELOAD         0x80000000u
#define INTERNET_FLAG_NO_CACHE_WRITE 0x04000000u
#define INTERNET_FLAG_DONT_CACHE     INTERNET_FLAG_NO_CACHE_WRITE
#define INTERNET_FLAG_SECURE         0x00800000u
#define INTERNET_FLAG_KEEP_CONNECTION 0x00400000u
#define INTERNET_FLAG_NO_COOKIES     0x00080000u
#define INTERNET_FLAG_NO_AUTH        0x00040000u
#define INTERNET_FLAG_PRAGMA_NOCACHE 0x00000100u
#define HSR_ASYNC                    0x00000004u
#define HSR_INITIATE                 0x00000008u
#define HSR_SYNC                     0x00000004u
#define HTTP_QUERY_STATUS_CODE       19
#define HTTP_QUERY_CONTENT_LENGTH    5
#define HTTP_QUERY_FLAG_NUMBER       0x20000000u
#define HTTP_ADDREQ_FLAG_ADD         0x20000000u
#define HTTP_ADDREQ_FLAG_REPLACE     0x80000000u
#define INTERNET_OPTION_SEND_TIMEOUT             5u
#define INTERNET_OPTION_RECEIVE_TIMEOUT          6u
#define INTERNET_OPTION_MAX_CONNS_PER_SERVER     73u
#define INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER 74u
#endif
using INTERNET_PORT = WORD;
struct INTERNET_BUFFERSA { DWORD dwStructSize; INTERNET_BUFFERSA* Next; LPCSTR lpcszHeader; DWORD dwHeadersLength; DWORD dwHeadersTotal; LPVOID lpvBuffer; DWORD dwBufferLength; DWORD dwBufferTotal; DWORD dwOffsetLow; DWORD dwOffsetHigh; };
struct INTERNET_BUFFERSW { DWORD dwStructSize; INTERNET_BUFFERSW* Next; LPCWSTR lpcszHeader; DWORD dwHeadersLength; DWORD dwHeadersTotal; LPVOID lpvBuffer; DWORD dwBufferLength; DWORD dwBufferTotal; DWORD dwOffsetLow; DWORD dwOffsetHigh; };
using LPINTERNET_BUFFERSA = INTERNET_BUFFERSA*;
using LPINTERNET_BUFFERSW = INTERNET_BUFFERSW*;
inline BOOL HttpSendRequestExA(HINTERNET, LPINTERNET_BUFFERSA, LPINTERNET_BUFFERSA, DWORD, DWORD_PTR) noexcept { return FALSE; }
inline BOOL HttpSendRequestExW(HINTERNET, LPINTERNET_BUFFERSW, LPINTERNET_BUFFERSW, DWORD, DWORD_PTR) noexcept { return FALSE; }
inline BOOL HttpEndRequestA(HINTERNET, LPINTERNET_BUFFERSA, DWORD, DWORD_PTR) noexcept { return FALSE; }
inline BOOL HttpEndRequestW(HINTERNET, LPINTERNET_BUFFERSW, DWORD, DWORD_PTR) noexcept { return FALSE; }
inline BOOL InternetWriteFile(HINTERNET, LPCVOID, DWORD, LPDWORD) noexcept { return FALSE; }
inline BOOL InternetSetOptionA(HINTERNET, DWORD, LPVOID, DWORD) noexcept { return TRUE; }
inline BOOL InternetSetOptionW(HINTERNET, DWORD, LPVOID, DWORD) noexcept { return TRUE; }
inline BOOL InternetQueryOptionA(HINTERNET, DWORD, LPVOID, LPDWORD) noexcept { return FALSE; }
inline BOOL InternetQueryOptionW(HINTERNET, DWORD, LPVOID, LPDWORD) noexcept { return FALSE; }
inline HINTERNET InternetOpenA(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD) noexcept { return nullptr; }
inline HINTERNET InternetOpenW(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD) noexcept { return nullptr; }
inline HINTERNET InternetConnectA(HINTERNET, LPCSTR, INTERNET_PORT, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR) noexcept { return nullptr; }
inline HINTERNET InternetConnectW(HINTERNET, LPCWSTR, INTERNET_PORT, LPCWSTR, LPCWSTR, DWORD, DWORD, DWORD_PTR) noexcept { return nullptr; }
inline HINTERNET HttpOpenRequestA(HINTERNET, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR*, DWORD, DWORD_PTR) noexcept { return nullptr; }
inline HINTERNET HttpOpenRequestW(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR*, DWORD, DWORD_PTR) noexcept { return nullptr; }
inline BOOL HttpSendRequestA(HINTERNET, LPCSTR, DWORD, LPVOID, DWORD) noexcept { return FALSE; }
inline BOOL HttpSendRequestW(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD) noexcept { return FALSE; }
inline BOOL HttpAddRequestHeadersA(HINTERNET, LPCSTR, DWORD, DWORD) noexcept { return FALSE; }
inline BOOL HttpAddRequestHeadersW(HINTERNET, LPCWSTR, DWORD, DWORD) noexcept { return FALSE; }
inline BOOL HttpQueryInfoA(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD) noexcept { return FALSE; }
inline BOOL HttpQueryInfoW(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD) noexcept { return FALSE; }
inline BOOL InternetReadFile(HINTERNET, LPVOID, DWORD, LPDWORD) noexcept { return FALSE; }
inline BOOL InternetQueryDataAvailable(HINTERNET, LPDWORD lpdwNumberOfBytesAvailable, DWORD, DWORD_PTR) noexcept { if (lpdwNumberOfBytesAvailable) *lpdwNumberOfBytesAvailable = 0; return FALSE; }
inline BOOL InternetCloseHandle(HINTERNET) noexcept { return TRUE; }
inline HINTERNET InternetOpenUrlA(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR) noexcept { return nullptr; }
inline HINTERNET InternetOpenUrlW(HINTERNET, LPCWSTR, LPCWSTR, DWORD, DWORD, DWORD_PTR) noexcept { return nullptr; }
#if defined(UNICODE) || defined(_UNICODE)
using INTERNET_BUFFERS = INTERNET_BUFFERSW;
using LPINTERNET_BUFFERS = LPINTERNET_BUFFERSW;
#define InternetOpen InternetOpenW
#define InternetConnect InternetConnectW
#define HttpOpenRequest HttpOpenRequestW
#define HttpSendRequest HttpSendRequestW
#define HttpSendRequestEx HttpSendRequestExW
#define HttpEndRequest HttpEndRequestW
#define HttpAddRequestHeaders HttpAddRequestHeadersW
#define HttpQueryInfo HttpQueryInfoW
#define InternetOpenUrl InternetOpenUrlW
#define InternetSetOption InternetSetOptionW
#define InternetQueryOption InternetQueryOptionW
#else
using INTERNET_BUFFERS = INTERNET_BUFFERSA;
using LPINTERNET_BUFFERS = LPINTERNET_BUFFERSA;
#define InternetOpen InternetOpenA
#define InternetConnect InternetConnectA
#define HttpOpenRequest HttpOpenRequestA
#define HttpSendRequest HttpSendRequestA
#define HttpSendRequestEx HttpSendRequestExA
#define HttpEndRequest HttpEndRequestA
#define HttpAddRequestHeaders HttpAddRequestHeadersA
#define HttpQueryInfo HttpQueryInfoA
#define InternetOpenUrl InternetOpenUrlA
#define InternetSetOption InternetSetOptionA
#define InternetQueryOption InternetQueryOptionA
#endif
