#pragma once
#include "CriticalSection.h"

class CBackgroundLoader;
class CStream;

struct SLoaderInfo
{
	CBackgroundLoader *pLoader;
	CStream *pStream;
};

class CBackgroundLoaderCallback
{
public:
	CBackgroundLoaderCallback() {}
	virtual ~CBackgroundLoaderCallback();
	virtual void OnLoadComplete( CBackgroundLoader *pLoader );

protected:
	std::vector< CBackgroundLoader * > m_vecCallbackPushedLoader;

public:
	void CallbackPush( CBackgroundLoader *pLoader ) { m_vecCallbackPushedLoader.push_back( pLoader ); }
};

class CBackgroundLoader
{
public:
	CBackgroundLoader(void);
	virtual ~CBackgroundLoader(void);

protected:
	static bool s_bBeginBackgroundLoad;
	static bool s_bNowLoading;
	static HANDLE s_hThread;
	static std::vector< SLoaderInfo > s_vecLoadList;
	static CSyncLock m_LoadListLock;
	static HANDLE s_hLoaderEvent[ 2 ];

	bool m_bReady;
	std::vector< CBackgroundLoaderCallback * > m_vecCallback;

public:
	virtual int LoadResource( CStream *pStream ) { return 1;}

	void SetReady( bool bReady );
	bool IsReady() { return m_bReady; }

	bool AddCallback( CBackgroundLoaderCallback *pCallback );
	void EraseCallback( CBackgroundLoaderCallback *pCallback );
	void ProcessCallback();

	static bool IsBeginBackgroundLoad() { return s_bBeginBackgroundLoad; }
	static void BeginBackgroundLoad();
	static void EndBackgroundLoad();

	static int GetRemainLoaderCount();

	static void PushLoader( CBackgroundLoader *pLoader, CStream *pStream );
	static bool PopLoader( SLoaderInfo &LoaderInfo );

	static DWORD WINAPI BackgroundLoaderCallback( LPVOID pParam );

};
