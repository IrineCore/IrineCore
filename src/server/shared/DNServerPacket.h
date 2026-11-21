#pragma once

#if !defined(DNPRJSAFE)
#include "DNPrjSafe.h"
#endif	// DNPRJSAFE
#include "DNProtocol.h"
#include "DNUserData.h"
#include "DNServerProtocol.h"
#include "DNServerPacketVillage.h"

/*--------------------------------------------------------------------------
Protocol and Packet used within Server

=> Split from DNServerProtocol.h because the file got too long 090218
=> Got too long, so separated by server now 100124
(Only items other than db, game, login, master, and village go here)
--------------------------------------------------------------------------*/

#pragma pack(push, 1)

//-------------------------------------------------
//	ServiceManager <-> NetLauncher or Service
//-------------------------------------------------
struct sServiceState
{
	int nSID;
	int nManagedLauncherID;
	int nServiceSessionID;
	int nState;			//eServiceState
	TCHAR szType[NAMELENMAX];
	ULONG nLastReportTick;
	char szVersion[NAMELENMAX];
	char szResource[32];
	BYTE cWorldIDs[WORLDCOUNTMAX];
	ULONG nCreateTime;
	bool bFinalBuild;
	bool bIsReConnect;	//Added to allow only 1 reconnect attempt

	void Init()
	{
		nServiceSessionID = 0;
		nLastReportTick = 0;
		nState = _SERVICE_STATE_OFFLINE;
		memset(szVersion, 0, sizeof(szVersion));
		memset(szResource, 0, sizeof(szResource));
		nCreateTime = 0;
		bFinalBuild = false;
		bIsReConnect = false;
	}
};

struct TNetResult				//NETSERVICE_RESULT
{
	char cAssingedType[MANAGEDMAX];
	WCHAR szBaseUrl[MAX_PATH];
	WCHAR szPatchUrl[MAX_PATH];
};

struct TRunProcess			//NETSERVICE_RUN_PROCESS
{
	int nSID;
};

struct TTerminateProcess		// NETSERVICE_TERMINATE_PROCESS
{
	int nSID;
	bool bRestart;
};

struct TPatch		//NETSERVICE_PATCH
{
	int nPatchResLevel;
	int nPatchID;
	WCHAR szKey[64];
};

struct TPatchUrl		//NETSERVICE_PATCH_BYURL
{
	int nPatchID;
	WCHAR szKey[64];
	bool bOnlyUseURL;
	WCHAR szDest[64];
	short nLen;
	char szUrl[1024];
};

struct TInfoCopy		//NETSERVICE_INFOCOPY
{
	bool bNeed;
};


struct TReturnPing		//NETSERVICE_RETURN_PING
{
	int nIdx;
	ULONG nTick;
};

struct TServiceConnected	//NETSERVICE_CONNECTED
{
	int nSessionID;
};

struct TServiceNotice			//NETSERVICE_NOTICE
{
	int nNoticeType;			//eSMNoticeType
	int nManagedID;
	int nChannelID;
	int nMapIdx;
	int nSlideShowSec;			//Slide notice duration in seconds (ticker board... -_-;?); if >= 0, it is a ticker board
	USHORT nLen;
	WCHAR wszMsg[CHATLENMAX];
};

struct TServiceChannelControl			//NETSERVICE_CHANNEL_CONTROL
{
	int nChannelID;
	BYTE cVisibility;
	int nChannelPopulation;
};

struct TServicePopulationControl			//NETSERVICE_POPULATION_CONTROL
{
	int nServerType;
	int nManagedID;
	BYTE cZeroPopulation;						//Population control
};

struct TServiceAffinityControl			//NETSERVICE_AFFINITY_CONTROL
{
	int nManagedID;
	BYTE cAffinityType;
};

struct TServiceUserRestraint			//NETSERVICE_USERRESTRAINT
{
	UINT nAccountDBID;
};

struct TServiceUserBan					//NETSERVICE_USERBAN
{
	eBanFileType eType;
	UINT nAccountDBID;
	WCHAR wszName[NAMELENMAX];
};

struct TServiceWordlMaxUser				//NETSERVICE_WORLDMAXUSER
{
	UINT nChangeMaxUser;
};

struct TServiceSelectjoin	//NETSERVICE_SELECTJOIN
{
	int nSID;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct TScheduleServiceClose	//NETSERVICE_SCHEDULE_SERVERCLOSE
{
	__time64_t _tOderedTime;
	__time64_t _tCloseTime;
};

struct TUpdateFarm				//NETSERVICE_UPDATEFARM
{
	int nWorldID;
	bool bForceStart;
};

struct TExtReload				//NETSERVICE_EXTRELOAD
{
	int nType;
};

struct TUPdateGuildWare			//NETSERVICE_UPDATEGUILDWARE
{
	int nGuildID;
};

struct TCreateDB				//NETSERVICE_CREATEWORLDDB
{
	int nWorldID;
};

struct TReloadGuildwarSchedule	//NETSERVICE_RELOADGUILDWARSCHEDULE
{
	int nWorldID;
};

struct TChangeLimitItemMax		//NETSERVICE_LIMITEITEM_CHANGEQUANTITY
{
	int nSN;
	int nLimiteMax;
};

struct TServiceStartManage			//NETSERVICE_STARTOF_MANAGING
{
	bool bOther;
};

struct TServiceManaging					//NETSERVICE_MANAGING
{
	int nSID;
	BYTE cCreateCount;
	BYTE cCreateIndex;
	USHORT nCmdSize;
	char szCmd[EXCUTELENMAX + EXCUTECMDMAX];
};

struct TPatcherSrevicePatchState		//PATCHERSERVICE_PATCHSTATE
{
	BYTE cState;				// 0 fail, otherwise success
};

struct TServerManagedList	//SERVICE_LAUNCHER_PROCESS_LIST
{
	BYTE cCount;
	int nListArr[255];
};

struct TServerProcessState		//SERVICE_LAUNCHER_PROCESS_STATE
{
	BYTE cPatch;
	char szState[256];
};

struct TServerEsmState
{
	char szState[256];
};

struct TServiceLauncherVersion		//SERVICE_LAUNCHER_VERSION
{
	char szVersion[SERVERVERSIONMAX];
};

struct TPatchState		//SERVICE_LAUNCHER_PATCH_STATE
{
	bool bFlag;
	int nPatchID;
	BYTE cLenState;
	BYTE cLenKey;
	char szState[128];
};

struct TPatchProgress //SERVICE_LAUNCER_PATCH_DATA_PROGRESS
{
	ULONG progress;
	ULONG progressMax;
	BYTE keyLen;
	WCHAR key[32];
};

struct TUnzipProgress
{
	ULONG progress;
	ULONG progressMax;
	short len;
	WCHAR filename[MAX_PATH + 1];
};

struct TSendPing		// SERVICE_LAUNCHER_PING
{
	char szPublicIP[IPLENMAX];
};

struct TRunBatch
{
	int nBatchID;
};
struct TRunBatchState
{
	int nRet;
	int nBatchID;
};

struct TStopBatch
{
	int nBatchID;
};

struct TStopBatchState
{
	int nBatchID;
};

struct TLiveExtCopy		//SERVICE_LAUNCHER_LIVEEXTCOPY
{
	bool bCopyRet;
};

struct TServerManagedID		//SERVICE_SERVER_REGIST_MANAGEDID
{
	int nMID;
	int nManagedType;
	int nServiceID;
	bool bFinalBuild;
	char szVersion[NAMELENMAX];
	char szResVersion[NAMELENMAX];
	BYTE cWorldCount;
	BYTE cWorldID[WORLDCOUNTMAX];
};

struct TServiceReportLogin				//SERVICE_SERVER_USUAL_LOGINREPORT
{
	UINT nUserCount;
};

struct TServiceReportVillage			//SERVICE_SERVER_USUAL_VILLAGEREPORT
{
	UINT nUserCount;
	USHORT nCount;
	TVillageUserReport Info[VILLAGECHANNELMAX];
};

struct TFarmStatus
{
	int nManagedID;
	UINT nFarmDBID;
	int nFarmCurUserCount;
	bool bActivate;
};

struct TServiceReportMaster				//SERVICE_SERVER_USUAL_MASTERREPORT
{
	int nWorldID;
	UINT nWaitserCount;
	BYTE cFarmCount;
	TFarmStatus FarmStatus[Farm::Max::FARMCOUNT/2];
};

struct TGameUserReport
{
	int nMapIdx;
	BYTE cUserCount;
};

struct TServiceReportGame				//SERVICE_SERVER_USUAL_GAMEREPORT
{
	BYTE cIsZeroPopulation;
	UINT nUserCount;
	UINT nRoomCount;
	UINT nTotalRoomCount;
};

struct TServiceDelayedReport		//SERVICE_SERVER_DELAYEDREPORT
{
	int nManagedID;
	ULONG nDelayedTick;
};

struct TServiceUnrecover			//SERVICE_SERVER_UNRECOVERY
{
	bool bBlock;
	bool bPermannently;
};

struct TServiceChannelInfo			//SERVICE_SERVER_CHANNELINFO
{
	int nManagedID;
	int nWorldID;
	BYTE cCount;
	sChannelInfo Info[VILLAGECHANNELMAX];
};

struct TServiceWorldInfo			//SERVICE_SERVER_WORLDINFO
{
	int nWorldID;
	int nWorldMaxuser;
};

struct TServiceMeritInfo			//SERVICE_SERVER_MERITINFO
{
	BYTE cCount;
	TMeritInfo Info[MERITINFOMAX];
};

struct TServerException			//SERVICE_SERVER_EXCEPTION_REPORT
{
	int nMID;
	int nType;
};

struct TServerExtReloadResult		//SERVICE_SERVER_EXT_RELOADRESULT
{
	bool bResult;
};

struct TServerLoginWorldDResult		// SERVICE_SERVER_LOGIN_WORDDBRESULT
{
	int nResult;				// 0 : Success / 1:Connect / 2:Version / 3:Not Find / 4:Already Created
	int nWorldID;
	char szIp[IPLENMAX];
	int nPort;
};

struct TQueryDelayedReport
{
	int nMID;
	DWORD dwThreadID;
	DWORD dwElasped;
	char szQueryName[QUERYNAMESIZE];
};

struct TGameDelayedReport
{
	WCHAR  wszLog[GAMEDELAYSIZE];
};

struct TQueryErrorReport
{
	USHORT	unLogType;
	USHORT	unWorldSetID;
	UINT	uiAccountDBID;
	INT64	biCharDBID;
	UINT	uiSessionID;
	USHORT	unServerType;
	USHORT	unLen;
	WCHAR	wszBuf[800];
};

struct TVillageDelayedReport
{
	WCHAR  wszLog[GAMEDELAYSIZE];
};

struct TNetLauncherInfo
{
	int nNID;
	WCHAR wszIP[IPLENMAX];
	WCHAR wszPublicIP[IPLENMAX];
};

struct TApplyWorldPvPRoom
{
	int nRetCode;
};

struct TCreateGambleRoom
{
	int nRoomIndex;	
	bool bRegulation;
	int nGambleType;
	int nPrice;
};

struct TStopGambleRoom
{
	int nRetCode;
};

struct TDelGambleRoom
{
	int nRetCode;
};

struct TUpdateDWCStatus
{
	int nRetCode;
};

//-------------------------------------------------
//	Log Packet
//-------------------------------------------------

const int LOGPARAMSIZEMAX = 1024;
struct TLog
{
	UINT nAccountDBID;
	INT64 biCharacterDBID;
	UINT nSessionID;
	char cWorldID;
	int nLogType;
	short wTotalParamCount;
	short wParamSize;
	char szParam[LOGPARAMSIZEMAX];
};

struct TLogFile
{
	USHORT	unLogType;
	USHORT	unWorldSetID;
	int		nServerID;
	UINT	uiAccountDBID;
	INT64	biCharDBID;
	UINT	uiSessionID;
	USHORT	unServerType;
	USHORT	unLen;
	WCHAR	wszBuf[1024];
};

//-------------------------------------------------
//	Auth Packet (Nexon)
//-------------------------------------------------
struct TAuthHeader
{
	BYTE cHeader;		// Fixed to 0xAA
	USHORT wLength;		// Up to 65536
};


#pragma pack(pop)
