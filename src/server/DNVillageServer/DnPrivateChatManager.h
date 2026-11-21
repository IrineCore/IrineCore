
#pragma once

#if defined( PRE_PRIVATECHAT_CHANNEL )

class CDNUserSession;
class CDNPrivateChaChannel;

class CDNPrivateChatManager 
{
public:
	CDNPrivateChatManager(); 
	~CDNPrivateChatManager();	
	
	bool AddPrivateChatChannel(char cWorldSetID, const TPrivateChatChannelInfo &ChannelInfo );		
	CDNPrivateChaChannel* GetPrivateChannelInfo( char cWorldSetID, INT64 nChannelID ); // Get channel info (including member users)	

	INT64 GetPrivateChatChannelId( char cWorldSetID, WCHAR* wszChannelName );
	const WCHAR* GetPrivateChatChannelName( char cWorldSetID, INT64 nChannelId );

	bool GetFirst() {return m_bFirst;}
	bool SetFirst( bool bFirst ) {return m_bFirst = bFirst;}

private:	
	bool m_bFirst;
	std::map<INT64,CDNPrivateChaChannel*> m_mChannelInfo;	// Channel ID, channel info	
};


extern CDNPrivateChatManager* g_pPrivateChatChannelManager;

#endif // #if defined( pre_privatechat_channel )
