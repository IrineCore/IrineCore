
#pragma once

#include "Task.h"
#include "MessageListener.h"
#include "DNProtocol.h"
#include "DNPacket.h"

class CDnVoiceChatTask : public CTask, public CTaskListener, public CSingleton<CDnVoiceChatTask>
{
public:
	CDnVoiceChatTask();
	~CDnVoiceChatTask();

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	bool Initialize();
	void Finalize();

	void OnDisconnectTcp(bool bValidDisconnect) {}
	void OnDisconnectUdp(bool bValidDisconnect) {}

	void OnDispatchMessage(int nMainCmd, int nSubCmd, char *pData, int nSize);

	bool InitVoiceChat();
	bool IsInitialized() { return m_bInit; }

	void ReqVoiceAvailable(bool bFlag);
	void ReqUserMute(UINT nSessionID, bool bFlag);
	void ReqUserComplaint(UINT nSessionID);

	void MuteMyMic(bool bMute);
	void SetRotation(EtVector2 vLook);

	void SetAutoAdjustVolume(bool bEnable, float fVolume = 0.0f) { m_bUseAutoAdjustVolume = bEnable; m_fAutoAdjustVolume = fVolume; }

#ifdef _USE_VOICECHAT
	bool CheckVoiceFont();
#endif

	// Set up voice chat dialog
	void SetVoiceChatOptDlg( CEtUIDialog *pDialog ) { m_pVoiceChatOptDlg = pDialog; }

protected:
	void OnRecvVoiceChatMessage(int nSubCmd, char *pData, int nSize);
	void OnRecvSystemVoiceChatInfo(SCVoiceChatInfo *pPacket);
	void OnRecvTalkingInfo(SCTalkingInfo * pPacket);
	void OnRecvVoiceMemberInfo(SCVoiceMemberInfo * pPacket);

private:
	volatile bool m_bInit;
	char m_szVoiceChatIp[IPLENMAX];		//dolby server ip
	USHORT m_wControlPort;
	USHORT m_wAudioPort;
	UINT m_nUniqueVoiceID;

	bool m_bUseAutoAdjustVolume;
	float m_fAutoAdjustVolume;
	bool m_bAutoAdjustVolumeMode;

	CEtUIDialog *m_pVoiceChatOptDlg;
};

#define GetVoiceChatTask()	CDnVoiceChatTask::GetInstance()