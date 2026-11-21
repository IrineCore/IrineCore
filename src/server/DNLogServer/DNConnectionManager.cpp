#include "stdafx.h"
#include "DNConnectionManager.h"
#include "DNConnection.h"
#include "Util.h"

CDNConnectionManager* g_pConnectionManager = NULL;

CDNConnectionManager::CDNConnectionManager(void): CConnectionManager()
{

}

CDNConnectionManager::~CDNConnectionManager(void)
{
	
}

CConnection* CDNConnectionManager::AddConnection(const char *pIp, const USHORT nPort)
{
	CDNConnection *pCon = new CDNConnection;
	if (!pCon) return NULL;

	// Just issue master connection ID..
	pCon->SetSessionID(g_IDGenerator.GetMasterConnectionID());
	pCon->SetIp(pIp);
	pCon->SetPort(nPort);

	PushConnection(pCon);

	return pCon;
}
