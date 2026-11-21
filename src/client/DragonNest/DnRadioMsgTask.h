#pragma once

#include "Task.h"
#include "MessageListener.h"


class CDnRadioMsgTask : public CTask, public CTaskListener, public CSingleton<CDnRadioMsgTask>
{
	enum
	{
		RADIOMSG_DELAY = 3000,	// Can only send once every 3 seconds
#if defined(PRE_ADD_ACADEMIC)
	//#ifdef PRE_ADD_KALI
	//	NUM_JOB = 6,
	//#else
	//	NUM_JOB = 5,
	//#endif

	NUM_JOB = 7,
#else
		NUM_JOB = 4,
#endif	// #if defined(PRE_ADD_ACADEMIC)

		NUM_SOUNDMSG = 6 ,       // Number of macros containing sound.
	};

public:
	CDnRadioMsgTask();
	virtual ~CDnRadioMsgTask();

	struct SRadioMsgInfo {
		int nID;
		tstring szName;
		int nIconID;
		int nMacroNum;	// 0 for RadioMsg, 1~4 for macro.
		tstring szMsg[NUM_JOB];
		std::vector<int> vecMsgSndIndex[NUM_JOB];
		EtTextureHandle hHeadIconTex;
	};

	struct STransformMsgInfo // For transformed actors, replace only messages and sounds.
	{
		std::vector<int> vecMsgSndIndex;
		std::vector<int> vecMsgIndex;
	};

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	void OnDisconnectTcp(bool bValidDisconnect) {}
	void OnDisconnectUdp(bool bValidDisconnect) {}
	void OnDispatchMessage(int nMainCmd, int nSubCmd, char *pData, int nSize);

	void Initialize();
	void Finalize();

	DWORD GetRadioMsgCount() { return (DWORD)m_pVecRadioMsgInfo.size(); }
	SRadioMsgInfo *GetRadioMsgInfoFromIndex( DWORD nIndex );
	SRadioMsgInfo *GetRadioMsgInfoFromID( int nID );

	void UseRadioMsg( USHORT nRadioID );

	void LoadTransformSound();
	void ClearTransformSound();
	void EnableTransformSound(int nTransformID,bool bEnable); // Sound when transforming.

protected:
	std::vector<SRadioMsgInfo *> m_pVecRadioMsgInfo;
	void OnRecvRadioMsgMessage(int nSubCmd, char * pData, int nSize);
	//recv
	void OnRecvUseRadio(SCUseRadio * pPacket);

	LOCAL_TIME m_RadioMsgSendTime;
	EtSoundChannelHandle m_hCurVoiceChannel;
	int m_nCurSessionID;

	std::map< int , STransformMsgInfo > m_mapTransformRadioMsgInfo;
	bool m_bEnableTransformRadioMsg;
	int m_nTrasnformMsgIndex;
};

#define GetRadioMsgTask()	CDnRadioMsgTask::GetInstance()