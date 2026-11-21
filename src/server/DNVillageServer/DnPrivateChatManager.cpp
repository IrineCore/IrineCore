#include "stdafx.h"
//#include "DNUserSession.h"
//#include "DNMasterConnection.h"
#include "DNPrivateChatChannel.h"
#include "DnPrivateChatManager.h"
//
#if defined( PRE_PRIVATECHAT_CHANNEL )
CDNPrivateChatManager* g_pPrivateChatChannelManager = NULL;

CDNPrivateChatManager::CDNPrivateChatManager()
{	
	m_bFirst = false;
	m_mChannelInfo.clear();	
}
CDNPrivateChatManager::~CDNPrivateChatManager()
{	
	for(std::map<INT64,CDNPrivateChaChannel*>::iterator itor = m_mChannelInfo.begin();itor!= m_mChannelInfo.end();itor++)
	{
		SAFE_DELETE((*itor).second);
	}
	m_mChannelInfo.clear();		
}

bool CDNPrivateChatManager::AddPrivateChatChannel(char cWorldSetID, const TPrivateChatChannelInfo &ChannelInfo )
{
	std::map<INT64,CDNPrivateChaChannel*>::iterator itor = m_mChannelInfo.find(ChannelInfo.nPrivateChatChannelID);
	if(itor != m_mChannelInfo.end() )
	{
		g_Log.Log(LogType::_NORMAL, L"AddPrivateChatChannel Exist Channel\r\n");
		return false;		
	}

	CDNPrivateChaChannel* PrivateChatChannel = new CDNPrivateChaChannel;
	if(PrivateChatChannel)
	{
		PrivateChatChannel->SetPrivateChannelInfo( ChannelInfo );
		m_mChannelInfo.insert(std::make_pair(ChannelInfo.nPrivateChatChannelID,PrivateChatChannel));		
	}	
	else
		return false;

	return true;
}

CDNPrivateChaChannel* CDNPrivateChatManager::GetPrivateChannelInfo(char cWorldSetID, INT64 nChannelID )
{
	std::map<INT64,CDNPrivateChaChannel*>::iterator itor = m_mChannelInfo.find(nChannelID);
	if(itor != m_mChannelInfo.end())
		return (*itor).second;

	return NULL;
}

INT64 CDNPrivateChatManager::GetPrivateChatChannelId(char cWorldSetID, WCHAR* wszChannelName )
{
	if(!wszChannelName)
		return 0;
	for(std::map<INT64,CDNPrivateChaChannel*>::iterator itor = m_mChannelInfo.begin();itor != m_mChannelInfo.end();itor++)
	{
		if (wcscmp((*itor).second->GetChannelName(), wszChannelName) == 0)
		{
			return itor->first;
		}		
	}
	return -1;
}
#endif