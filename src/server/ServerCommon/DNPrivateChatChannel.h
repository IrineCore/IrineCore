
#pragma once

#if defined( PRE_PRIVATECHAT_CHANNEL )
class CDNUserSession;

class CDNPrivateChaChannel 
{
public:
	CDNPrivateChaChannel(); 
	~CDNPrivateChaChannel();	
	void SetPrivateChannelInfo(const TPrivateChatChannelInfo &ChannelInfo );
	void AddPrivateChannelMember( const TPrivateChatChannelMember &Member ); // Add user to channel	
	void DelPrivateChannelMember( PrivateChatChannel::Common::eModType eType, INT64 biCharacterDBID ); // Remove user from channel
	void GetPrivateChannelMember( std::list<TPrivateChatChannelMember> &MemberList ); //Channel member list
	TPrivateChatChannelMember GetPrivateChannelMember( INT64 biCharacterDBID); //Channel member info
	bool CheckPrivateChannelMember(INT64 biCharacterDBID);	
	void SetPrivateChannelMemberAccountDBID(UINT nAccountDBID, INT64 biCharacterDBID);	
	TPrivateChatChannelMember GetPrivateChannelMember( WCHAR* wszName); //Channel member info
	void ModPrivateChannelInfo( PrivateChatChannel::Common::eModType eType, int nPassWord, INT64 biMasterCharacterID );		
	void ModPrivateChannelMemberName( INT64 biCharacterDBID, WCHAR* wszName );	
	INT64 GetNextMasterCharacterDBID();
	TPrivateChatChannelInfo GetChannelInfo() {return m_ChannelInfo;}
	int GetChannelPassword() {return m_ChannelInfo.nPassWord;}
	INT64 GetChannelID() {return m_ChannelInfo.nPrivateChatChannelID;}
	WCHAR* GetChannelName() {return m_ChannelInfo.wszName;}
	INT64 GetMasterCharacterDBID() {return m_ChannelInfo.biMasterCharacterDBID;}
	void GetMasterCharacterName(WCHAR* MasterName);
	bool bIsLoaded() {return m_bLoad;}
	void SetLoaded( bool bLoaded ) {m_bLoad=bLoaded;}

private:
	CSyncLock m_Sync;
	bool m_bLoad;
	TPrivateChatChannelInfo m_ChannelInfo;	
	
	std::map<INT64,TPrivateChatChannelMember> m_mJoinMembers;	// Character ID, member info
};
#endif // #if defined( PRE_PRIVATECHAT_CHANNEL )
