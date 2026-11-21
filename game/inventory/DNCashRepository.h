
#pragma once

class CDNEffectItem;

class CDNEffectRepository:public TBoostMemoryPool<CDNEffectRepository>
{
public:
	CDNEffectRepository( CDNUserSession* pSession );
	~CDNEffectRepository();
	bool Add( INT64 biItemSerial, int iItemID, __time64_t tExpireDate = 0 );
	bool Remove( INT64 biItemSerial );

#if defined (PRE_ADD_VIP_FARM)
	void SendEffectItem();	
	bool bIsExpiredItem( int iItemType );
#endif // #if defined (PRE_ADD_VIP_FARM)		

	void DelGuildRewardItem();	// Delete guild reward effect item
#if defined( PRE_ADD_BESTFRIEND )
	void DelEffectItemType( char cType );	// Function to delete from memory only since already deleted from DB
#endif

private:
	CDNUserSession* m_pSession;
	std::map<INT64,CDNEffectItem*> m_mRepository;
};

