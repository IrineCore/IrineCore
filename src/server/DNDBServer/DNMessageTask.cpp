#include "stdafx.h"
#include "DNMessageTask.h"

CDNSPErrorCheckManager* g_pSPErrorCheckManager=NULL;

CDNMessageTask::CDNMessageTask(CDNConnection* pConnection)
	: m_pConnection(pConnection)
{
}

CDNMessageTask::~CDNMessageTask(void)
{
}
