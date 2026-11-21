#include "stdafx.h"
#include "DnChatTask.h"

CDnChatTask::CDnChatTask(CDNGameRoom * pRoom)
: CTask( pRoom )
, CMultiSingleton<CDnChatTask, MAX_SESSION_COUNT>( pRoom )
{

}

CDnChatTask::~CDnChatTask()
{

}


bool CDnChatTask::Initialize()
{

	return true;
}

void CDnChatTask::Process( LOCAL_TIME LocalTime, float fDelta )
{

}

int CDnChatTask::OnDispatchMessage( CDNUserSession * pSession, int nMainCmd, int nSubCmd, char *pData, int nLen )
{
	switch( nMainCmd ) 
	{
		case CS_CHATROOM: return pSession->OnRecvChatRoomMessage(nSubCmd, pData, nLen );
	}

	return ERROR_NONE;
}
