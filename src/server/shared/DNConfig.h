#pragma once

#include "DNCommonDef.h"
#include "DNServerDef.h"

struct TConnectionInfo
{
	char szIP[IPLENMAX];
	int nPort;
};

struct TWorldInfo
{
	int nWorldSetID;
	WCHAR wszWorldName[WORLDNAMELENMAX];
	TConnectionInfo VillageInfos[VILLAGECOUNTMAX];	// Number of village servers
};




const BYTE DSNLENMAX = 30;
const BYTE DBNAMELENMAX = 30;

struct TDBName: TConnectionInfo
{
	int nWorldSetID;
	WCHAR wszDBName[DBNAMELENMAX];	// DB table name
	WCHAR wszDBID[IDLENMAX];		// DB login account
};

struct TLoginConfig
{
	int nManagedID;								//Launcher ManagedID

	int nVersion;								// Version
	char szVersion[SERVERVERSIONMAX];
	char szResVersion[SERVERVERSIONMAX];
	USHORT nMajorVersion;
	USHORT nMinorVersion;

	int nIocpMax;								// iocp init max
	int nSQLMax;								// SQL max
	int nWorkerThreadMax;						// worker thread max

	TDBName MembershipDB;
	TDBName WorldDB[WORLDCOUNTMAX];
	int nWorldDBCount;

	int nMasterAcceptPort;						// Master TCP port
	int nClientAcceptPort;						// Client TCP port

	TConnectionInfo LogInfo;

	std::string szResourcePath;					// resource path

};

struct TVillageConfig
{
	char szVersion[SERVERVERSIONMAX];
	char szResVersion[SERVERVERSIONMAX];
	int nCreateCount;
	int nCreateIndex;
	int nManagedID;

	int nWorldSetID;								// World ID
	int nVillageID;								// Village server identifier ID (ID to distinguish when multiple instances are running)

	int nIocpMax;								// iocp init max

	int nClientAcceptPort;						// Client TCP port

	TConnectionInfo CashInfo;
	TConnectionInfo MasterInfo;					// Master info

	int nDBCount;								// Total DB count
	TConnectionInfo DBInfos[DBSERVERMAX];		// DB info

	TConnectionInfo	LogInfo;

	// SQL used to get ItemSerial (must never use anything else)
	TDBName MembershipDB;

	char szPrivateDolbyIp[IPLENMAX];					// Dolby server IP
	char szPublicDolbyIp[IPLENMAX];					// Dolby server IP
	int nControlPort;							// Dolby server port (control)
	int nAudioPort;								// Dolby server port (audio)

	std::string szResourcePath;					// resource path

#if defined( PRE_WORLDCOMBINE_PARTY )
	int nCombinePartyWorld;
#endif
};

struct TGameConfig
{
	char szVersion[SERVERVERSIONMAX];	//exe version
	char szResVersion[SERVERVERSIONMAX];	//ResVersion
	BYTE cAffinityType;							// Server support type (Normal:0, PvP+Normal:1, PvP:2)	
	int nCreateCount;
	int nCreateIndex;
	int nManagedID;

	bool bPreLoad;								// Preload~~
	bool bAllLoaded;							// To determine initial load timing
	//int nGameID; // Game server ID

	int nGameServerOpenCount;					//Number of open game servers
	int nGameAcceptPortBegin;					//Game server UDP accept port
	int nClientAcceptPort;						// Client TCP port

	int nIocpMax;								// MaxSocket

	TConnectionInfo CashInfo;
	int nMasterCount;							// Total Master count
	TConnectionInfo MasterInfo[WORLDCOUNTMAX];	// Master info

	int nDBCount;								// Total DB count
	TConnectionInfo DBInfos[DBSERVERMAX];		// DB info

	TConnectionInfo	LogInfo;

	// SQL used to get ItemSerial (must never use anything else)
	TDBName MembershipDB;

	char szPrivateDolbyIp[IPLENMAX];					// Dolby server IP
	char szPublicDolbyIp[IPLENMAX];					// Dolby server IP
	int nControlPort;							// Dolby server port (control)
	int nAudioPort;								// Dolby server port (audio)

	int nProbePort;								//Probe info to properly obtain UDP address
	char szProbeIP[16];

	std::string szResourcePath;					// resource path


#if defined( PRE_FIX_WORLDCOMBINEPARTY )
	bool bWorldCombineGameServer;
#endif
};

struct TMasterConfig
{
	char szVersion[SERVERVERSIONMAX];
	char szResVersion[SERVERVERSIONMAX];
	int nManagedID;

	int nWorldSetID;								// World ID (only one master server exists per world)

	int nVillageAcceptPort;
	int nGameAcceptPort;

	TConnectionInfo LoginInfo[LOGINCOUNTMAX];

	std::string szResourcePath;					// resource path

	TConnectionInfo	LogInfo;
#ifdef PRE_ADD_DOORS
	TConnectionInfo	DoorsInfo;
	TConnectionInfo DoorsIdenty[2];
#endif		//#ifdef PRE_ADD_DOORS


#if defined( PRE_WORLDCOMBINE_PARTY )
	int nCombinePartyWorld;
#endif
#ifdef PRE_MOD_OPERATINGFARM
	int nFarmServerID;
#endif		//#ifdef PRE_MOD_OPERATINGFARM
};

struct TDBConfig
{
	char szVersion[SERVERVERSIONMAX];
	char szResVersion[SERVERVERSIONMAX];
	int nManagedID;

	int nAcceptPort;
	int nThreadMax;

	TDBName MembershipDB;
	TDBName WorldDB[WORLDCOUNTMAX];
	int nWorldDBCount;
#if defined( PRE_WORLDCOMBINE_PARTY )
	int nCombineWorldDBID;
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	TConnectionInfo LogInfo;

	std::string szResourcePath;		// resource path
};

struct TLogConfig
{
	char szVersion[SERVERVERSIONMAX];
	char szResVersion[SERVERVERSIONMAX];
	int nManagedID;

	int	nAcceptPort;
	int nThreadMax;

	TDBName MasterLogDB;			// MasterLogDB (DB required for Nexon web service)
	TDBName ServerLogDB;
	TDBName LogDB[WORLDCOUNTMAX];	// LogDB
	int	nLogDBCount;
	std::string szResourcePath;		// resource path
};

struct TCashConfig
{
	int nManagedID;								//Launcher ManagedID

	int nVersion;								// Version
	char szVersion[SERVERVERSIONMAX];
	char szResVersion[SERVERVERSIONMAX];
	USHORT nMajorVersion;
	USHORT nMinorVersion;

	int nWorldSetID;								// World ID
	int nThreadMax;

	TDBName MembershipDB;
	TDBName WorldDB[WORLDCOUNTMAX];
	int nWorldDBCount;

	int nAcceptPort;

	TConnectionInfo LogInfo;

	std::string szResourcePath;		// resource path

};
