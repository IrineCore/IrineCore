#include "stdafx.h"
#include "GameSession.h"
#include "Task.h"
#include "DNGameRoom.h"
//#include "DNSession.h"

CGameSession::CGameSession()
{
}

CGameSession::~CGameSession()
{
}

DWORD CGameSession::GetProcessID()
{
	CTask *pTask = dynamic_cast<CTask *>(this);
	return pTask->GetRoom()->GetProcessID();
	//return pTask->GetSession()->GetProcessID();
}

int CGameSession::GetSessionID()
{
	CTask *pTask = dynamic_cast<CTask *>(this);
	return pTask->GetRoom()->GetSessionID();
	//return pTask->GetSession()->GetSessionID();
}
