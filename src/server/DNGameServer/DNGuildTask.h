#pragma once

#include "Task.h"
#include "GameListener.h"
#include "MultiSingleton.h"
#include "DNGameRoom.h"

class CDnGuildTask : public CTask, public CMultiSingleton<CDnGuildTask, MAX_SESSION_COUNT>, public CGameListener, public TBoostMemoryPool< CDnGuildTask >
{
public:
	CDnGuildTask(CDNGameRoom * pRoom);
	virtual ~CDnGuildTask();

protected:
	int OnRecvGuildMessage(CDNUserSession * pSession, int nSubCmd, char *pData, int nLen);

public:
	bool Initialize();

	virtual void Process(LOCAL_TIME LocalTime, float fDelta);

	virtual int OnDispatchMessage(CDNUserSession * pSession, int nMainCmd, int nSubCmd, char *pData, int nLen);
};
