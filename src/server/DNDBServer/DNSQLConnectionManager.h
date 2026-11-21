#pragma once

#include "DNSQLWorld.h"
#include "DNSQLMembership.h"

class CDNSQLConnectionManager
{
private:
	CDNSQLMembership *m_pMembershipArray[THREADMAX];
	CDNSQLWorld *m_pWorldArray[THREADMAX][WORLDCOUNTMAX];

	bool CreateMembership();
	bool CreateWorld();

public:
	CDNSQLConnectionManager(void);
	~CDNSQLConnectionManager(void);

	void Clear();
	bool CreateDB();
	void DestroyDB();

	CDNSQLMembership* FindMembershipDB(int nThreadIndex);
	CDNSQLWorld* FindWorldDB(int nThreadIndex, char cWorldID);
	CDNSQLWorld* FindWorldDB(int nThreadIndex);
};

extern CDNSQLConnectionManager g_SQLConnectionManager;
