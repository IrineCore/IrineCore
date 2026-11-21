
#include "stdafx.h"
#include "DNDataManagerBase.h"
#include <boost/lexical_cast.hpp>

#include "XMLParser.h"
#include "Log.h"

CDNDataManagerBase::CDNDataManagerBase()
{
}

CDNDataManagerBase::~CDNDataManagerBase()
{
	ClearChannelData();
}

void CDNDataManagerBase::SetChannelAttribute(int * pChannelAttOut, const WCHAR * pAttribute)
{
	if(pAttribute == NULL)
	{
#if defined(_SERVICEMANAGER_EX)
		ErrorMessage(L"Channel attribute is null.");
#else
		_ASSERT_EXPR(0, L"[SetChannelAttribute] pAttribute == NULL" );
#endif
		return;
	}

	const WCHAR* ptr = pAttribute;

	while( *ptr )
	{
		WCHAR token[256];
		memset(token, 0, sizeof(token));

		// Add | so Attribute can be ORed.
		for( ; *ptr && wcschr(L"|\n\r\t ", *ptr) ; ++ptr );
		for( int j=0 ; *ptr && !wcschr(L"|\n\r\t ", *ptr) ; ++j, ++ptr )
			token[j] = *ptr;

		if( wcsicmp( token, L"normal") == 0 )
			*pChannelAttOut |= GlobalEnum::CHANNEL_ATT_NORMAL;
		else if( wcsicmp( token, L"pvp") == 0 )
			*pChannelAttOut |= GlobalEnum::CHANNEL_ATT_PVP;
		else if( wcsicmp( token, L"pvplobby") == 0 )
			*pChannelAttOut |= GlobalEnum::CHANNEL_ATT_PVPLOBBY;
		else if( wcsicmp( token, L"merit") == 0)
			*pChannelAttOut |= GlobalEnum::CHANNEL_ATT_MERITBONUS;
		else if( wcsicmp( token, L"gm") == 0 )
			*pChannelAttOut |= GlobalEnum::CHANNEL_ATT_GM;
		else if( wcsicmp( token, L"darklair") == 0 )
			*pChannelAttOut |= GlobalEnum::CHANNEL_ATT_DARKLAIR;
		else if( wcsicmp( token, L"FarmTown") == 0 )
			*pChannelAttOut |= GlobalEnum::CHANNEL_ATT_FARMTOWN;
		else if( wcsicmp( token, L"dwc") == 0 )
			*pChannelAttOut |= GlobalEnum::CHANNEL_ATT_DWC;		
		else
		{
#if defined(_SERVICEMANAGER_EX)
			ErrorMessage(L"Invalid channel attribute. \'%s\'", token);
#else
			_ASSERT_EXPR(0, L"[SetChannelAttribute] token invalid" );
#endif
		}
	}
}

void CDNDataManagerBase::SetMeritBonusType(int * pMeritOut, const WCHAR * pMerit)
{
	if(!pMerit)
	{
#if defined(_SERVICEMANAGER_EX)
		ErrorMessage(L"Channel merit bonus is null.");
#else
		_ASSERT_EXPR(0, L"[SetMeritBonusType] pMerit == NULL" );
#endif
	}

	if(wcsicmp(pMerit, L"CompleteExp") == 0)
		*pMeritOut = GlobalEnum::MERIT_BONUS_COMPLETEEXP;
	else
	{
#if defined(_SERVICEMANAGER_EX)
		ErrorMessage(L"Invalid merit bonus type. \'%s\'", pMerit);
#else
		_ASSERT_EXPR(0, L"[SetMeritBonusType] pMerit != CompleteExp" );
#endif
	}
}


bool CDNDataManagerBase::LoadChannelInfo()
{
	CXMLParser parser;

	if(parser.Open("./systems/ChannelInfo.xml") == false && parser.Open("./System/ChannelInfo.xml") == false)
	{
		g_Log.Log( LogType::_FILELOG, L"ChannelInfo.xml failed\r\n" );
		return false;
	}

	if(parser.FirstChildElement("ChannelInfo", true) == true)
	{

		if(parser.FirstChildElement("MeritBonusTable", true) == true)
		{
			if(parser.FirstChildElement("Bonus") == true)
			{
				do {
					TMeritInfo * pInfo = new TMeritInfo;

					pInfo->nID = _wtoi(parser.GetAttribute("ID"));
					pInfo->nMinLevel = _wtoi(parser.GetAttribute("MinLevel"));
					pInfo->nMaxLevel = _wtoi(parser.GetAttribute("MaxLevel"));
					SetMeritBonusType(&pInfo->nMeritType, parser.GetAttribute("BonusType"));
					pInfo->nExtendValue = _wtoi(parser.GetAttribute("ExtendValue"));

					_ASSERT(m_pMeritInfoList.find(pInfo->nID) == m_pMeritInfoList.end());
					m_pMeritInfoList[pInfo->nID] = pInfo;
				} while(parser.NextSiblingElement("Bonus"));
			}
			parser.GoParent();
		}

		std::map <int, int> MapWorldSet;
		MapWorldSet.clear();

		if(parser.FirstChildElement("ServerMergerInfo", true) == true)
		{
			if(parser.FirstChildElement("Merger") == true)
			{
				do {
					int nID = _wtoi(parser.GetAttribute("SetID"));

					std::wstring wstrServerID = parser.GetAttribute("ServerID");
					if(!wstrServerID.empty()){
						std::vector<std::wstring> tokens;
						TokenizeW(wstrServerID, tokens, L",");

						for (int i = 0; i <(int)tokens.size(); i++){
							int nServerID = _wtoi(tokens[i].c_str());
							MapWorldSet[nServerID] = nID;
						}
					}
				} while(parser.NextSiblingElement("Merger"));
			}
			parser.GoParent();
		}

		WCHAR buf[IPLENMAX] = { 0, };
		m_pServerInfoList.clear();
		std::map <int, int> ChannelIdxList;

		std::map <int, std::vector<int>> vChannelIDVerify;

		if(parser.FirstChildElement("ServerInfo", true) == true)
		{
			if(parser.FirstChildElement("Server", true) == true)
			{
				do {
					ChannelIdxList.clear();

					TServerInfo * serverinfo = new TServerInfo;
					//2010.10.7 haling Converted to constructor for STL memory protection
					//memset(serverinfo, 0, sizeof(TServerInfo));

					serverinfo->cWorldID = _wtoi(parser.GetAttribute("ID"));
					memset(&buf, 0, sizeof(buf));
					_wcscpy(serverinfo->wszWorldName, _countof(serverinfo->wszWorldName), parser.GetAttribute("Name"), (int)wcslen(parser.GetAttribute("Name")));
					serverinfo->nWorldMaxUser = _wtoi(parser.GetAttribute("WorldMaxUser"));
					serverinfo->nDefaultMaxUser = _wtoi(parser.GetAttribute("DefaultMaxUser"));
					if(parser.GetAttribute("OnTop"))
						serverinfo->bOnTop = !_wcsicmp(parser.GetAttribute("OnTop"), L"true");
					else
						serverinfo->bOnTop = false;

					bool bCheckPvPLobby = false;
					if(parser.FirstChildElement("VillageServerInfo", true) == true)
					{
						do {
							//TVillageInfo VillageInfo = { 0, };
							//2010.10.7 haling Converted to constructor for STL protection

							TVillageInfo VillageInfo;

							VillageInfo.cVillageID = _wtoi(parser.GetAttribute("ID"));
							memset(&buf, 0, sizeof(buf));
							_wcscpy(buf, _countof(buf), parser.GetAttribute("IP"), (int)wcslen(parser.GetAttribute("IP")));
							WideCharToMultiByte(CP_ACP, 0, buf, -1, VillageInfo.szIP, IPLENMAX, NULL, NULL);
							VillageInfo.nPort = _wtoi(parser.GetAttribute("Port"));


							if(parser.FirstChildElement("VillageInfo") == true)
							{
								do {
									TChannelInfo ChannelInfo = { 0, };

									ChannelInfo.nChannelID = _wtoi(parser.GetAttribute("CID"));
									ChannelInfo.nChannelIdx = ++ChannelIdxList[_wtoi(parser.GetAttribute("ID"))];
									ChannelInfo.nMapIdx =  _wtoi(parser.GetAttribute("ID"));
									_wcscpy(ChannelInfo.wszMapName, _countof(ChannelInfo.wszMapName), parser.GetAttribute("Name"), (int)wcslen(parser.GetAttribute("Name")));

									const WCHAR * pChannelMaxUser = parser.GetAttribute("MaxUser");
									if(pChannelMaxUser)
										ChannelInfo.nChannelMaxUser = _wtoi(pChannelMaxUser);
									else
										ChannelInfo.nChannelMaxUser = serverinfo->nDefaultMaxUser;
									_ASSERT(ChannelInfo.nChannelMaxUser);

									ChannelInfo.nAttribute = 0;
									SetChannelAttribute(&ChannelInfo.nAttribute, parser.GetAttribute("Attribute"));									
									if(ChannelInfo.nAttribute&GlobalEnum::CHANNEL_ATT_PVPLOBBY)
									{
										if(false == bCheckPvPLobby)
											bCheckPvPLobby = true;
										else
										{
											g_Log.Log( LogType::_FILELOG, L"ServerInfo Duplicated Normal|PvPLobby [WorldID:%d]\r\n", serverinfo->cWorldID);
											return false;
										}
									}
									if(ChannelInfo.nAttribute&GlobalEnum::CHANNEL_ATT_MERITBONUS)
									{
										int nMeritBonusID = _wtoi(parser.GetAttribute("MeritBonusID"));
										if (m_pMeritInfoList.find(nMeritBonusID) == m_pMeritInfoList.end())
										{
											g_Log.Log( LogType::_FILELOG, L"ServerInfo Invalid MeritBonusID [WorldID:%d][MeritBonus:%d]\r\n", serverinfo->cWorldID, nMeritBonusID);
											return false;
										}
										ChannelInfo.nMeritBonusID = _wtoi(parser.GetAttribute("MeritBonusID"));
									}

									ChannelInfo.bVisibility = !wcscmp(parser.GetAttribute("Visibility"), L"True") ? true : false;

									const WCHAR* pLimitLevel = parser.GetAttribute("LimitLevel");
									ChannelInfo.nLimitLevel = pLimitLevel ? boost::lexical_cast<int>(pLimitLevel) : 0;

									ChannelInfo.bShow = false;

									const WCHAR * pDependentMapID = parser.GetAttribute("DependentMapID");
									ChannelInfo.nDependentMapID = pDependentMapID ? boost::lexical_cast<int>(pDependentMapID) : 0;
									std::map <int, std::vector<int>>::iterator iverify = vChannelIDVerify.find(serverinfo->cWorldID);
									if(iverify == vChannelIDVerify.end())
									{
										std::vector<int> vVerify;
										vVerify.push_back(ChannelInfo.nChannelID);
										vChannelIDVerify[serverinfo->cWorldID] = vVerify;
									}
									else
									{
										//check
										for (int g = 0; g <(int)(*iverify).second.size(); g++)
										{
											for(std::vector<int>::iterator veribegin = (*iverify).second.begin(); veribegin != (*iverify).second.end(); veribegin++)
											{
												if((*veribegin) == ChannelInfo.nChannelID)
												{
#if defined(_SERVICEMANAGER_EX)
													ErrorMessage(L"VillageInfo channel ID duplicated. \'%d\'", ChannelInfo.nChannelID);
#else
													WCHAR wszErrMsg[256];
													memset(wszErrMsg, 0, sizeof(wszErrMsg));

													swprintf(wszErrMsg, L"WorldID[%d] ChannelID[%d] Duplicated!", (*iverify).first, ChannelInfo.nChannelID);
													_ASSERT_EXPR(0, wszErrMsg);
#endif
													
													return false;
												}
											}
										}
									}
#if defined(PRE_ADD_CHANNELNAME)
									const WCHAR* pLanguageName = parser.GetAttribute("LanguageName");
									if( pLanguageName )
										_wcscpy(ChannelInfo.wszLanguageName, _countof(ChannelInfo.wszLanguageName), pLanguageName, (int)wcslen(pLanguageName));
#endif //#if defined(PRE_ADD_CHANNELNAME)

									std::map<int, std::wstring>::const_iterator ii = m_ChannelMapList.find(ChannelInfo.nMapIdx);
									if(ii == m_ChannelMapList.end())
										m_ChannelMapList[ChannelInfo.nMapIdx] = ChannelInfo.wszMapName;

									VillageInfo.vOwnedChannelList.push_back(ChannelInfo);
								} while(parser.NextSiblingElement("VillageInfo"));
							}

							serverinfo->vOwnedVillageList.push_back(VillageInfo);
						} while(parser.NextSiblingElement("VillageServerInfo"));
						parser.GoParent();
					} 
					m_pServerInfoList[serverinfo->cWorldID] = serverinfo;
					if(MapWorldSet.empty()){	// Merge setting is not configured. Just set equal to worldid
						m_pServerInfoList[serverinfo->cWorldID]->nWorldSetID = m_pServerInfoList[serverinfo->cWorldID]->cWorldID;
					}
					else{
						//Add handling when expanding a merged server group
						bool bMergedWorld = false;
						std::map<int, int>::iterator iter = MapWorldSet.find(serverinfo->cWorldID);
						if(iter != MapWorldSet.end())
						{
							m_pServerInfoList[serverinfo->cWorldID]->nWorldSetID = iter->second;
							bMergedWorld = true;
						}

						if(bMergedWorld)
							m_pServerInfoList[serverinfo->cWorldID]->bMergedWorld = true;
						else
							m_pServerInfoList[serverinfo->cWorldID]->nWorldSetID = m_pServerInfoList[serverinfo->cWorldID]->cWorldID;
					}
				} while(parser.NextSiblingElement("Server"));
				parser.GoParent();
			}
			return true;
		}
	}

	g_Log.Log( LogType::_FILELOG, L"ServerInfo Parsing failed\r\n");
	return false;
}

const TServerInfo * CDNDataManagerBase::GetServerInfo(int nWorldID) const
{
	if(m_pServerInfoList.empty()) return NULL;

	TMapServerInfo::const_iterator ii = m_pServerInfoList.find(nWorldID);
	if(ii != m_pServerInfoList.end())
		return(*ii).second;
	return NULL;
}

const TServerInfo * CDNDataManagerBase::GetServerInfoByWorldSetID(int nWorldSetID) const
{
	if(m_pServerInfoList.empty()) return 0;

	for(TMapServerInfo::const_iterator iter = m_pServerInfoList.begin(); iter != m_pServerInfoList.end(); ++iter){
		if(iter->second->nWorldSetID == nWorldSetID)
			return iter->second;
	}
	return NULL;
}

const TVillageInfo* CDNDataManagerBase::GetVillageInfo(char cWorldID, BYTE cVillageID) const
{
	const TServerInfo *pServerInfo = GetServerInfo(cWorldID);
	if(!pServerInfo) return NULL;
	if(pServerInfo->vOwnedVillageList.empty()) return NULL;

	for (int i = 0; i <(int)pServerInfo->vOwnedVillageList.size(); i++){
		if(pServerInfo->vOwnedVillageList[i].cVillageID != cVillageID) continue;
		return &pServerInfo->vOwnedVillageList[i];
	}
	return NULL;
}

int CDNDataManagerBase::GetWorldMaxUser(int nWorldSetID)
{
	if(m_pServerInfoList.empty()) return 0;

	for(TMapServerInfo::iterator iter = m_pServerInfoList.begin(); iter != m_pServerInfoList.end(); ++iter){
		if(iter->second->nWorldSetID == nWorldSetID)
			return iter->second->nWorldMaxUser;
	}

	return 0;
}

int CDNDataManagerBase::GetWorldSetID(int nWorldID)
{
	const TServerInfo *pInfo = GetServerInfo(nWorldID);
	if(!pInfo) return -1;

	return pInfo->nWorldSetID;
}

void CDNDataManagerBase::GetWorldSetList(std::vector <int> &vList)
{
	TMapServerInfo::iterator ii;
	for(ii = m_pServerInfoList.begin(); ii != m_pServerInfoList.end(); ii++)
	{
		if((*ii).second->bMergedWorld)
		{
			std::vector <int>::iterator ih = std::find(vList.begin(), vList.end(), (*ii).second->nWorldSetID);
			if(ih == vList.end())
				vList.push_back((*ii).second->nWorldSetID);
		}
	}
}

const TMeritInfo * CDNDataManagerBase::GetMeritInfo(int nMeritID) const
{
	TMapMeritInfo::const_iterator ii = m_pMeritInfoList.find(nMeritID);
	if(ii != m_pMeritInfoList.end())
		return(*ii).second;
	return NULL;
}

void CDNDataManagerBase::GetMeritList(std::vector<TMeritInfo> * vList)
{
	TMeritInfo Info;
	TMapMeritInfo::iterator ii;
	for(ii = m_pMeritInfoList.begin(); ii != m_pMeritInfoList.end(); ii++)
	{
		memset(&Info, 0, sizeof(Info));
		memcpy(&Info, (*ii).second, sizeof(TMeritInfo));
		vList->push_back(Info);
	}
}

void CDNDataManagerBase::ClearChannelData()
{
	for(TMapServerInfo::iterator iterS = m_pServerInfoList.begin(); iterS != m_pServerInfoList.end(); ++iterS)
		SAFE_DELETE(iterS->second);
	m_pServerInfoList.clear();

	for(TMapMeritInfo::iterator im = m_pMeritInfoList.begin(); im != m_pMeritInfoList.end(); im++)
		SAFE_DELETE((*im).second);
	m_pMeritInfoList.clear();
}

void CDNDataManagerBase::GetChannelMapName(int nMapIndex, WCHAR* wszMapName)
{
	std::map<int, std::wstring>::const_iterator ii = m_ChannelMapList.find(nMapIndex);
	if(ii != m_ChannelMapList.end())
		wcscpy(wszMapName, (*ii).second.c_str());
}

int CDNDataManagerBase::GetChannelMapAtt( int nWorldID, int nMapIndex )
{
	const TServerInfo *pServerInfo = GetServerInfo(nWorldID);
	if(!pServerInfo) 
		return 0;
	if(pServerInfo->vOwnedVillageList.empty()) return 0;

	for (int i = 0; i <(int)pServerInfo->vOwnedVillageList.size(); i++)
	{
		for(int j = 0; j <(int)pServerInfo->vOwnedVillageList[i].vOwnedChannelList.size(); j++)
		{
			if(pServerInfo->vOwnedVillageList[i].vOwnedChannelList[j].nMapIdx == nMapIndex)
			{
				return pServerInfo->vOwnedVillageList[i].vOwnedChannelList[j].nAttribute;
			}		
		}		
	}		
	return 0;
}

const TChannelInfo* CDNDataManagerBase::GetChannelInfo(int nWorldID, int nMapIndex) const
{
	const TServerInfo *pServerInfo = GetServerInfo(nWorldID);
	if(!pServerInfo) 
		return NULL;

	for (int i = 0; i <(int)pServerInfo->vOwnedVillageList.size(); i++)
	{
		for(int j = 0; j <(int)pServerInfo->vOwnedVillageList[i].vOwnedChannelList.size(); j++)
		{
			if(pServerInfo->vOwnedVillageList[i].vOwnedChannelList[j].nMapIdx == nMapIndex)
			{
				return &(pServerInfo->vOwnedVillageList[i].vOwnedChannelList[j]);
			}		
		}		
	}		
	return NULL;
}