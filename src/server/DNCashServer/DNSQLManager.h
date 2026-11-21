#pragma once

#include "DNSQLWorld.h"
#include "DNSQLMembership.h"

class CDNSQLManager
{
private:
	CDNSQLMembership *m_pMembershipDBArray[THREADMAX];
	CDNSQLWorld *m_pWorldDBArray[THREADMAX][WORLDCOUNTMAX];

	bool CreateMembershipDB();
	bool CreateWorldDB();

public:
	CDNSQLManager(void);
	~CDNSQLManager(void);

	void Clear();
	bool CreateDB();
	void DestroyDB();

	CDNSQLMembership* FindMembershipDB(int nThreadIndex);
	CDNSQLWorld* FindWorldDB(int nThreadIndex, char cWorldID);
	CDNSQLWorld* FindWorldDB(int nThreadIndex);
};

extern CDNSQLManager *g_pSQLManager;
