
#pragma once

#include <iprtrmib.h>
#include <IPTypes.h>
#include <string>
#include <vector>

#define CXIP_A(IP)	((IP&0xFF000000)>>24)
#define CXIP_B(IP)	((IP&0x00FF0000)>>16)
#define CXIP_C(IP)	((IP&0x0000FF00)>>8)
#define CXIP_D(IP)	(IP&0x000000FF)

typedef BOOL (WINAPI* fnGetIpAddrTable)(PMIB_IPADDRTABLE pIpAddrTable, PULONG pdwSize, BOOL bOrder);
typedef DWORD (WINAPI* fnGetAdaptersInfo)(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen);

namespace NetAdapter
{
	enum eErrorType
	{
		ERROR_NONE = 0,
		ERROR_LIBRARY = 1,
		ERROR_LPTABLE = 2,
		ERROR_ADAPTERINFO = 3,
		ERROR_EMPTYINFO = 4,
		ERROR_PUBLIC_SUCCESS = 5,
		ERROR_FIRST_SUCCESS = 6,
	};
};

class CNetAdapters
{
public:
	CNetAdapters();
	~CNetAdapters();

	void GetFirstMacAddress(std::string &strMacAddr);
	void GetFirstPublicMacAddress(std::string &strMacAddr);
	void GetMacAddressList(std::vector<std::string> &vMacAddrList);
	void GetPublicMacAddressList(std::vector<std::string> &vMacAddrList);

	int GetErrorNum() { return m_nError;};

private:
	CNetAdapters& operator=(const CNetAdapters& Adaters)
	{
		_ASSERT(0);
		return(*this);
	}

	HINSTANCE m_hIpHlp;
	fnGetIpAddrTable m_fnGetIpAddrTable;
	fnGetAdaptersInfo m_fnGetAdaptersInfo;

	struct sAdapterInfo
	{
		int iIndex;
		std::string strAdapterName;
		std::string strDescription;
		std::string strMacAddress;
		DWORD dwPublicIP;
		bool bPublic;
		char szIP[IPLENMAX];
	};
	std::vector <sAdapterInfo> m_vAdapterList;
	int m_nError;

	int _Init();
	void _MakeNetAdapterInfo();
	bool _FindPublicIndex(std::vector <sAdapterInfo> &vList);
};