
#pragma once

//Channel & Merit Bonus
typedef map<int, TMeritInfo*> TMapMeritInfo;
typedef map<int, TServerInfo*> TMapServerInfo;

class CDNDataManagerBase
{
public:
	CDNDataManagerBase();
	virtual ~CDNDataManagerBase();

	void SetChannelAttribute(int * pChannelAttOut, const WCHAR * pAttribute);
	void SetMeritBonusType(int * pMeritOut, const WCHAR * pMerit);

	bool LoadChannelInfo();
	
	//ServerInfo
	const TServerInfo * GetServerInfo(int nWorldID) const;
	const TServerInfo * GetServerInfoByWorldSetID(int nWorldSetID) const;
	const TVillageInfo* GetVillageInfo(char cWorldID, BYTE cVillageID) const;
	int GetWorldMaxUser(int nWorldSetID);
	int GetWorldSetID(int nWorldID);
	void GetWorldSetList(std::vector <int> &vList);

	//MeritBonus Info
	const TMeritInfo * GetMeritInfo(int nMeritID) const;
	void GetMeritList(std::vector<TMeritInfo> * vList);


	void GetChannelMapName(int nMapIndex, WCHAR* wszMapName);
	int GetChannelMapAtt(int nWorldID, int nMapIndex);
	const TChannelInfo* GetChannelInfo(int nWorldID, int nMapIndex) const;

protected:
	TMapMeritInfo m_pMeritInfoList;
	TMapServerInfo m_pServerInfoList;
	std::map<int, std::wstring> m_ChannelMapList;

	void ClearChannelData();
};
