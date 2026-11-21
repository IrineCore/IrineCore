#include "stdafx.h"
#include "DNAdjustTask.h"
#include "DNConnection.h"

CDNAdjustTask::CDNAdjustTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{
}

CDNAdjustTask::~CDNAdjustTask(void)
{
}

void CDNAdjustTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	switch (nSubCmd)
	{
	case QUERY_MSGADJUST:
		{
			TQMsgAdjust * pPacket = (TQMsgAdjust*)pData;
			TAMsgAdjust packet;
			memset(&packet, 0, sizeof(packet));

			packet.nAccountDBID = pPacket->nAccountDBID;
			packet.biCharacterDBID = pPacket->biCharacterDBID;
			packet.nChannelID = pPacket->nChannelID;
			packet.nVerifyMapIndex = pPacket->nVerifyMapIndex;
			packet.cGateNo = pPacket->cGateNo;

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&packet, sizeof(packet));
		}
		break;
#ifdef PRE_FIX_63822
	default:
		{
			TQMsgAdjustSeq * pPacket = (TQMsgAdjustSeq*)pData;
			TAMsgAdjustSeq packet;
			memset(&packet, 0, sizeof(TAMsgAdjustSeq));

			packet.nAccountDBID = pPacket->nAccountDBID;
			packet.nRetCode = ERROR_NONE;

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&packet, sizeof(packet));
		}
		break;
#endif		//#ifdef PRE_FIX_63822
	}
}