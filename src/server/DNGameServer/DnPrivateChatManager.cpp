#include "stdafx.h"
#include "DNPrivateChatChannel.h"
#include "DnPrivateChatManager.h" 
//
#if defined( PRE_PRIVATECHAT_CHANNEL )
CDNPrivateChatManager* g_pPrivateChatChannelManager = NULL;

CDNPrivateChatManager::CDNPrivateChatManager()
{	
	m_bFirst = false;	
	for(int i=0;i<PrivateChatChannel::Common::MAXWORLDCOUNT;i++)
	{
		m_mChannelInfo[i].clear();
		m_cWorldNumber[i] = 0;
	}	
}
CDNPrivateChatManager::~CDNPrivateChatManager()
{	
	for(int i=0;i<PrivateChatChannel::Common::MAXWORLDCOUNT;i++)
	{
		for(std::map<INT64,CDNPrivateChaChannel*>::iterator itor = m_mChannelInfo[i].begin();itor!= m_mChannelInfo[i].end();itor++)
		{
			SAFE_DELETE((*itor).second);
		}
		m_mChannelInfo[i].clear();
	}

	
}

bool CDNPrivateChatManager::AddPrivateChatChannel( char cWorldSetID, const TPrivateChatChannelInfo &ChannelInfo )
{
	ScopeLock <CSyncLock> Lock(m_Sync);
	char cWorldIndex = GetWorldIndex(cWorldSetID);
	std::map<INT64,CDNPrivateChaChannel*>::iterator itor = m_mChannelInfo[cWorldIndex].find(ChannelInfo.nPrivateChatChannelID);
	if(itor != m_mChannelInfo[cWorldIndex].end() )
	{
		g_Log.Log(LogType::_NORMAL, L"AddPrivateChatChannel Exist Channel\r\n");
		return false;		
	}

	CDNPrivateChaChannel* PrivateChatChannel = new CDNPrivateChaChannel;
	if(PrivateChatChannel)
	{
		PrivateChatChannel->SetPrivateChannelInfo( ChannelInfo );
		ScopeLock <CSyncLock> Lock(m_Sync);
		m_mChannelInfo[cWorldIndex].insert(std::make_pair(ChannelInfo.nPrivateChatChannelID,PrivateChatChannel));		
	}	
	else
		return false;
	
	return true;
}

CDNPrivateChaChannel* CDNPrivateChatManager::GetPrivateChannelInfo( char cWorldSetID, INT64 nChannelID )
{	
	ScopeLock <CSyncLock> Lock(m_Sync);
	char cWorldIndex = GetWorldIndex(cWorldSetID);	
	std::map<INT64,CDNPrivateChaChannel*>::iterator itor = m_mChannelInfo[cWorldIndex].find(nChannelID);
	if(itor != m_mChannelInfo[cWorldIndex].end())
		return (*itor).second;

	return NULL;
}

INT64 CDNPrivateChatManager::GetPrivateChatChannelId( char cWorldSetID, WCHAR* wszChannelName )
{	
	if( !wszChannelName )
		return 0;
	ScopeLock <CSyncLock> Lock(m_Sync);
	char cWorldIndex = GetWorldIndex(cWorldSetID);	
	for(std::map<INT64,CDNPrivateChaChannel*>::iterator itor = m_mChannelInfo[cWorldIndex].begin();itor != m_mChannelInfo[cWorldIndex].end();itor++)
	{
		if (wcscmp((*itor).second->GetChannelName(), wszChannelName) == 0)
		{
			return itor->first;
		}		
	}
	return -1;
}
char CDNPrivateChatManager::GetWorldIndex(char cWorldSetID)
{
	for(int i=0;i<PrivateChatChannel::Common::MAXWORLDCOUNT;i++)
	{
		if(m_cWorldNumber[i] == 0)
		{
			m_cWorldNumber[i] = cWorldSetID;
			return i;
		}
		else if(m_cWorldNumber[i] == cWorldSetID)
			return i;
	}
	return 0;
}
#endif