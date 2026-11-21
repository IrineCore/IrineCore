#pragma once

#include <string>

// Lightweight asynchronous Discord Desktop IPC Rich Presence client (Win32 x86).
class CDnDiscordPresence
{
public:
	static CDnDiscordPresence &GetInstance();

	void Initialize();
	void Update();
	void Shutdown();

private:
	struct PresenceSnapshot
	{
		bool active;
		bool hasTimestamp;
		__int64 startTimestamp;
		std::string details;
		std::string state;
		std::string largeImage;
		std::string largeText;
		std::string smallImage;
		std::string smallText;
		std::string partyId;
		int partySize;
		int partyMax;

		PresenceSnapshot()
			: active(false)
			, hasTimestamp(false)
			, startTimestamp(0)
			, partySize(0)
			, partyMax(0)
		{
		}

		bool operator==(const PresenceSnapshot &rhs) const;
		bool operator!=(const PresenceSnapshot &rhs) const { return !(*this == rhs); }
	};

	CDnDiscordPresence();
	~CDnDiscordPresence();
	CDnDiscordPresence(const CDnDiscordPresence &);
	CDnDiscordPresence &operator=(const CDnDiscordPresence &);

	PresenceSnapshot BuildSnapshot();
	void QueueSnapshot(const PresenceSnapshot &snapshot, bool forceSend = false);

	static unsigned int __stdcall WorkerThreadProc(void *pParam);
	void WorkerLoop();
	bool EnsureConnected();
	void DisconnectPipe();
	bool WriteFrame(DWORD dwOpcode, const std::string &payload);
	bool ReadAvailableFrame(DWORD *pOpcode, std::string *pPayload, DWORD dwTimeoutMs);
	void DrainIncomingFrames();
	bool SendSetActivity(const PresenceSnapshot &snapshot);
	bool SendClearActivity();

	CRITICAL_SECTION m_Lock;
	bool m_bLockInitialized;
	bool m_bInitialized;
	HANDLE m_hStopEvent;
	HANDLE m_hWakeEvent;
	HANDLE m_hWorkerThread;
	HANDLE m_hPipe;

	DWORD m_dwLastSampleTick;
	DWORD m_dwNonceCounter;
	DWORD m_dwPartyGeneration;
	bool m_bWasInParty;
	std::string m_strLastPartyLeader;
	__int64 m_nSessionStartUnix;

	PresenceSnapshot m_LastQueuedSnapshot;
	PresenceSnapshot m_PendingSnapshot;
	bool m_bHasPendingUpdate;
	bool m_bClearPending;
};
