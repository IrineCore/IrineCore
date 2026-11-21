#pragma once

//////////////////////////////////////////////////////////////////////////
// Per-country definitions (resource library and PatchConfigList info)

//	_USE_COMMAND_LINE // Use command line only
//	_USE_SINGLE_CLIENT // Support single client only
//	_USE_PARTITION_SELECT // Support partition selection
//	_USE_SPLIT_COMPRESS_FILE // Support split-compressed (RAR) file download


	#define _USE_SINGLE_CLIENT
	#include "ResourceUSA/resource.h"
	#include "ResourceUSA/ResourceInit.h"
	#define DLL_RESOURCE_FILE	"ResourceUSA/ResourceUSA.rc"
	// ResourceUSA.lib is linked via ResourceUSA.vcxproj ProjectReference.

	// URL definition
	#define	PATCHCONFIG_LIST_URL	_T( "http://127.0.0.1/Game/DragonNest/patch/" )
	#define	PATCHCONFIG_LIST		_T( "PatchConfigList.xml" )


//////////////////////////////////////////////////////////////////////////
// Common definitions

#define DNLAUNCHER_NAME			"DnLauncher.exe"
#define DNLAUNCHER_NAME_TMP		"dnlauncher.tmp"
#define DNCLIENT_NAME			"DragonNest.exe"
#define DNAUTOUPDATE_NAME		"DnAutoUpdate.exe"
#define CLIENT_VERSION_NAME		_T( "Version.cfg" )
#define SERVER_VERSION_NAME		_T( "PatchInfoServer.cfg" )
#define PATCHINFONAME			_T( "PatchConfig.cfg" )
#define FIRSTPATCHLIST_NAME		_T( "FirstPatchList.txt" )

#define FIRSTPATCH_CONTROL_NAME _T( "FirstPatchControlData.xml" )

#define RETRY_MAX_COUNT		3
#define __MAX_PATH			8192
#define LAUNCHER_VERSION	2
#define _USE_LOG
#define _USE_AUTOUPDATE
//#define ACTOZ_TEST_MESSAGE
//#define _FIRST_PATCH
//#define _USE_RTPATCH
