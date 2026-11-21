#include "stdafx.h"
#include "EtEngine.h"
#include "EtObject.h"
#include "EtTerrainArea.h"
#include "PerfCheck.h"

#ifdef _DEBUG 
#define new DEBUG_NEW 
#endif 

CEtEngine g_EtEngine;
float g_fElapsedTime = 0.0f;
float g_fTotalElapsedTime = 0.0f;

CEtEngine::CEtEngine()
{
}

CEtEngine::~CEtEngine()
{

}

void CEtEngine::Initialize( CMultiRoom *pRoom )
{
	char szCurDir[ _MAX_PATH ];

	GetCurrentDirectoryA( _MAX_PATH, szCurDir );
	CEtResourceMng::GetInstance().AddResourcePath( szCurDir );
}

void CEtEngine::Finalize( CMultiRoom *pRoom )
{
	CEtObject::DeleteAllObject( pRoom );
	CEtTerrainArea::DeleteAllObject( pRoom );
}
