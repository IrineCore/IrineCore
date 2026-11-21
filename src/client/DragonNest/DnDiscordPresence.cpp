#include "StdAfx.h"
#include "DnDiscordPresence.h"

#include <process.h>
#include <time.h>
#include <cstdio>

#include "TaskManager.h"
#include "DnLoginTask.h"
#include "DnGameTask.h"
#include "DnPartyTask.h"
#include "DnBridgeTask.h"
#include "DnActor.h"
#include "DnPlayerActor.h"
#include "DnWorld.h"
#include "DnTableDB.h"
#include "DnInterfaceString.h"
#include "GlobalValue.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

namespace
{
	const char *kDiscordAppId = "1568653839689711727";
	const char *kAssetBaseUrl = "https://raw.githubusercontent.com/monakaibrahim-cmyk/discord-assets/main/";
	const char *kLargeImageUrl = "https://raw.githubusercontent.com/monakaibrahim-cmyk/discord-assets/main/LargeImage.png";

	const DWORD kSampleIntervalMs = 250;
	const DWORD kReconnectIntervalMs = 4000;
	const DWORD kHandshakeTimeoutMs = 2000;

	struct JobAssetEntry
	{
		int nJobId;
		const char *szJobName;
	};

	const JobAssetEntry kCap60Jobs[] = {
		{ 1,  "Warrior" },
		{ 2,  "Archer" },
		{ 3,  "Sorceress" },
		{ 4,  "Cleric" },
		{ 5,  "Academic" },
		{ 6,  "Kali" },
		{ 11, "Swordmaster" },
		{ 12, "Mercenary" },
		{ 14, "Bowmaster" },
		{ 15, "Acrobat" },
		{ 17, "Elementallord" },
		{ 18, "Forceuser" },
		{ 20, "Paladin" },
		{ 22, "Priest" },
		{ 23, "Gladiator" },
		{ 24, "Moonlord" },
		{ 25, "Barbarian" },
		{ 26, "Destroyer" },
		{ 29, "Sniper" },
		{ 30, "Artillery" },
		{ 31, "Tempest" },
		{ 32, "Windwalker" },
		{ 35, "Saleana" },
		{ 36, "Elestra" },
		{ 37, "Smasher" },
		{ 38, "Majesty" },
		{ 41, "Guardian" },
		{ 42, "Crusader" },
		{ 43, "Saint" },
		{ 44, "Inquisitor" },
		{ 46, "Engineer" },
		{ 47, "Shootingstar" },
		{ 48, "Gearmaster" },
		{ 49, "Alchemist" },
		{ 50, "Adept" },
		{ 51, "Physician" },
		{ 54, "Screamer" },
		{ 55, "Darksummoner" },
		{ 56, "Souleater" },
		{ 57, "Dancer" },
		{ 58, "Bladedancer" },
		{ 59, "Spiritdancer" },
	};

	const JobAssetEntry *FindJobAsset(int nJobId)
	{
		const size_t count = sizeof(kCap60Jobs) / sizeof(kCap60Jobs[0]);
		for (size_t i = 0; i < count; ++i)
		{
			if (kCap60Jobs[i].nJobId == nJobId)
				return &kCap60Jobs[i];
		}
		return NULL;
	}

	std::string WideToUtf8(const std::wstring &wstr)
	{
		if (wstr.empty())
			return std::string();

		int needed = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
		if (needed <= 0)
			return std::string();

		std::string result(needed, '\0');
		::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], needed, NULL, NULL);
		return result;
	}

	std::string EscapeJson(const std::string &input)
	{
		std::string out;
		out.reserve(input.size() + 8);
		for (size_t i = 0; i < input.size(); ++i)
		{
			unsigned char ch = (unsigned char)input[i];
			switch (ch)
			{
			case '"':  out += "\\\""; break;
			case '\\': out += "\\\\"; break;
			case '\b': out += "\\b"; break;
			case '\f': out += "\\f"; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			case '\t': out += "\\t"; break;
			default:
				if (ch < 0x20)
				{
					char buf[8];
					_snprintf_s(buf, sizeof(buf), _TRUNCATE, "\\u%04x", ch);
					out += buf;
				}
				else
				{
					out += (char)ch;
				}
				break;
			}
		}
		return out;
	}

	// Discord rejects single-character activity strings (<2 bytes) or strings >128 bytes.
	std::string SanitizePresenceField(const std::string &str)
	{
		if (str.empty())
			return std::string();
		std::string out = str;
		if (out.size() > 127)
			out.resize(127);
		if (out.size() == 1)
			out += " ";
		return out;
	}

	std::wstring SafeGetMapName(int nMapIdx)
	{
		if (nMapIdx <= 0 || !CDnTableDB::IsActive() || !CEtUIXML::IsActive())
			return std::wstring();

		DNTableFileFormat *pSox = GetDNTable(CDnTableDB::TMAP);
		if (!pSox || !pSox->IsExistItem(nMapIdx))
			return std::wstring();

		DNTableCell *pCell = pSox->GetFieldFromLablePtr(nMapIdx, "_MapNameID");
		if (!pCell)
			return std::wstring();

		int nStrId = pCell->GetInteger();
		if (nStrId <= 0)
			return std::wstring();

		LPCWSTR pStr = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStrId);
		return pStr ? std::wstring(pStr) : std::wstring();
	}

	std::wstring SafeGetSelectedServerName()
	{
#ifdef PRE_MOD_SELECT_CHAR
		if (CGlobalInfo::IsActive() && CGlobalInfo::GetInstance().m_nSelectedServerIndex > 0)
			return CGlobalInfo::GetInstance().GetServerName(CGlobalInfo::GetInstance().m_nSelectedServerIndex);
#endif
		return std::wstring();
	}
}

bool CDnDiscordPresence::PresenceSnapshot::operator==(const PresenceSnapshot &rhs) const
{
	return active == rhs.active
		&& hasTimestamp == rhs.hasTimestamp
		&& startTimestamp == rhs.startTimestamp
		&& details == rhs.details
		&& state == rhs.state
		&& largeImage == rhs.largeImage
		&& largeText == rhs.largeText
		&& smallImage == rhs.smallImage
		&& smallText == rhs.smallText
		&& partyId == rhs.partyId
		&& partySize == rhs.partySize
		&& partyMax == rhs.partyMax;
}

CDnDiscordPresence &CDnDiscordPresence::GetInstance()
{
	static CDnDiscordPresence s_Instance;
	return s_Instance;
}

CDnDiscordPresence::CDnDiscordPresence()
	: m_bLockInitialized(false)
	, m_bInitialized(false)
	, m_hStopEvent(NULL)
	, m_hWakeEvent(NULL)
	, m_hWorkerThread(NULL)
	, m_hPipe(INVALID_HANDLE_VALUE)
	, m_dwLastSampleTick(0)
	, m_dwNonceCounter(0)
	, m_dwPartyGeneration(0)
	, m_bWasInParty(false)
	, m_nSessionStartUnix(0)
	, m_bHasPendingUpdate(false)
	, m_bClearPending(false)
{
	::InitializeCriticalSection(&m_Lock);
	m_bLockInitialized = true;
}

CDnDiscordPresence::~CDnDiscordPresence()
{
	Shutdown();
	if (m_bLockInitialized)
	{
		::DeleteCriticalSection(&m_Lock);
		m_bLockInitialized = false;
	}
}

void CDnDiscordPresence::Initialize()
{
	if (m_bInitialized)
		return;

	m_nSessionStartUnix = (__int64)::_time64(NULL);
	m_dwLastSampleTick = 0;

	m_hStopEvent = ::CreateEventW(NULL, TRUE, FALSE, NULL);
	m_hWakeEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
	if (!m_hStopEvent || !m_hWakeEvent)
	{
		Shutdown();
		return;
	}

	unsigned int threadId = 0;
	uintptr_t hThread = _beginthreadex(NULL, 0, &CDnDiscordPresence::WorkerThreadProc, this, 0, &threadId);
	if (hThread == 0)
	{
		Shutdown();
		return;
	}
	m_hWorkerThread = (HANDLE)hThread;
	m_bInitialized = true;

	// Publish initial loading/startup Rich Presence immediately.
	Update();
}

void CDnDiscordPresence::Shutdown()
{
	if (m_hStopEvent)
	{
		::EnterCriticalSection(&m_Lock);
		m_bClearPending = true;
		m_bHasPendingUpdate = false;
		::LeaveCriticalSection(&m_Lock);

		::SetEvent(m_hStopEvent);
		if (m_hWakeEvent)
			::SetEvent(m_hWakeEvent);
	}

	if (m_hWorkerThread)
	{
		::WaitForSingleObject(m_hWorkerThread, 1500);
		::CloseHandle(m_hWorkerThread);
		m_hWorkerThread = NULL;
	}

	DisconnectPipe();

	if (m_hWakeEvent)
	{
		::CloseHandle(m_hWakeEvent);
		m_hWakeEvent = NULL;
	}
	if (m_hStopEvent)
	{
		::CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}

	m_bInitialized = false;
	m_bWasInParty = false;
	m_strLastPartyLeader.clear();
	m_LastQueuedSnapshot = PresenceSnapshot();
}

void CDnDiscordPresence::Update()
{
	if (!m_bInitialized)
		return;

	DWORD dwNow = ::GetTickCount();
	if (m_dwLastSampleTick != 0 && (dwNow - m_dwLastSampleTick) < kSampleIntervalMs)
		return;
	m_dwLastSampleTick = dwNow;

	PresenceSnapshot snapshot = BuildSnapshot();
	if (!snapshot.active)
		return;

	if (snapshot != m_LastQueuedSnapshot)
		QueueSnapshot(snapshot, false);
}

void CDnDiscordPresence::QueueSnapshot(const PresenceSnapshot &snapshot, bool forceSend)
{
	if (!m_bInitialized)
		return;

	m_LastQueuedSnapshot = snapshot;

	::EnterCriticalSection(&m_Lock);
	m_PendingSnapshot = snapshot;
	m_bHasPendingUpdate = true;
	if (forceSend)
		m_bClearPending = false;
	::LeaveCriticalSection(&m_Lock);

	if (m_hWakeEvent)
		::SetEvent(m_hWakeEvent);
}

CDnDiscordPresence::PresenceSnapshot CDnDiscordPresence::BuildSnapshot()
{
	PresenceSnapshot snap;
	if (!CTaskManager::IsActive())
		return snap;

	CTaskManager &taskMgr = CTaskManager::GetInstance();

	// 1. Loading & startup intro movies: show Rich Presence with no extra info.
	if (taskMgr.GetTask("TitleTask") != NULL)
	{
		m_bWasInParty = false;
		m_strLastPartyLeader.clear();

		snap.active = true;
		snap.hasTimestamp = false;
		snap.largeImage = kLargeImageUrl;
		return snap;
	}

	snap.active = true;
	snap.hasTimestamp = true;
	snap.startTimestamp = m_nSessionStartUnix;
	snap.largeImage = kLargeImageUrl;
	snap.largeText = "Dragon Nest";

	// 2. Login / World Select / Character Select / Character Create screens.
	if (CDnLoginTask *pLoginTask = static_cast<CDnLoginTask *>(taskMgr.GetTask("LoginTask")))
	{
		m_bWasInParty = false;
		m_strLastPartyLeader.clear();

		std::string strServer = WideToUtf8(SafeGetSelectedServerName());
		switch (pLoginTask->GetState())
		{
		case CDnLoginTask::Logo:
		case CDnLoginTask::RequestIDPass:
			snap.details = "In Login";
			break;

		case CDnLoginTask::ServerList:
			snap.details = "World Selection";
			break;

		case CDnLoginTask::ChannelList:
			snap.details = "Channel Selection";
			if (!strServer.empty())
				snap.state = "World: " + strServer;
			break;

		case CDnLoginTask::CharSelect:
		case CDnLoginTask::CharDelete:
			snap.details = "Character Select";
			if (!strServer.empty())
				snap.state = "World: " + strServer;
			break;

		case CDnLoginTask::CharCreate_SelectClass:
			snap.details = "Create Character";
			snap.state = "Selecting Class";
			break;

		case CDnLoginTask::CharCreate_ChangeParts:
			snap.details = "Create Character";
			snap.state = "Customizing Character";
			break;

		case CDnLoginTask::StartGame:
			snap.details = "Entering Game";
			break;
		}
		return snap;
	}

	// 3. In-game states (Village, Colosseum Lobby, Dungeon / Nest / Farm / PvP).
	CDnGameTask *pGameTask = static_cast<CDnGameTask *>(taskMgr.GetTask("GameTask"));
	CTask *pVillageTask = taskMgr.GetTask("VillageTask");
	CTask *pPvPLobbyTask = taskMgr.GetTask("PVPLobbyTask");

	if (!pGameTask && !pVillageTask && !pPvPLobbyTask)
	{
		snap.details = "In Menu";
		return snap;
	}

	std::string strServer = WideToUtf8(SafeGetSelectedServerName());
	if (!strServer.empty())
		snap.largeText = "Dragon Nest - " + strServer;

	// Character identity, level, and job icon.
	if (CDnActor::s_hLocalActor)
	{
		CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
		if (pPlayer)
		{
			LPCWSTR wszName = pPlayer->GetName();
			std::string strName = wszName ? WideToUtf8(wszName) : std::string();
			char szBuf[192];
			if (!strName.empty())
				_snprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, "%s - Lv. %d", strName.c_str(), pPlayer->GetLevel());
			else
				_snprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, "Lv. %d", pPlayer->GetLevel());
			snap.details = szBuf;

			int nJobId = pPlayer->OnGetJobClassID();
			const JobAssetEntry *pJobAsset = FindJobAsset(nJobId);
			if (pJobAsset)
			{
				char szIconUrl[256];
				_snprintf_s(szIconUrl, sizeof(szIconUrl), _TRUNCATE, "%s%d.png", kAssetBaseUrl, nJobId);
				snap.smallImage = szIconUrl;

				LPCWSTR wszJobName = pPlayer->GetJobName();
				std::string strJob = wszJobName ? WideToUtf8(wszJobName) : std::string();
				snap.smallText = !strJob.empty() ? strJob : std::string(pJobAsset->szJobName);
			}
		}
	}
	else
	{
		snap.details = "Loading...";
	}

	// Location name (stage, village, PvP arena, farm, etc.).
	int nMapIdx = CGlobalInfo::IsActive() ? CGlobalInfo::GetInstance().m_nCurrentMapIndex : 0;
	std::string strMap = WideToUtf8(SafeGetMapName(nMapIdx));

	if (pGameTask)
	{
		GameTaskType::eType gt = pGameTask->GetGameTaskType();
		CDnWorld::MapTypeEnum mt = CDnWorld::IsActive() ? CDnWorld::GetInstance().GetMapType() : CDnWorld::MapTypeUnknown;

		if (strMap.empty())
			strMap = "In Stage";

		if (gt == GameTaskType::PvP)
		{
			strMap += " (PvP)";
		}
		else if (gt == GameTaskType::DarkLair)
		{
			strMap += " (Dark Lair)";
		}
		else if (gt == GameTaskType::Farm)
		{
			strMap += " (Farm)";
		}
		else if (mt == CDnWorld::MapTypeDungeon)
		{
			int nDiff = pGameTask->GetStageConstructionLevel();
			if (nDiff >= 0 && CEtUIXML::IsActive())
			{
				std::wstring wszDiff;
				DN_INTERFACE::STRING::GetStageDifficultyText(wszDiff, nDiff);
				std::string strDiff = WideToUtf8(wszDiff);
				if (!strDiff.empty())
					strMap += " (" + strDiff + ")";
			}
		}
	}
	else if (pPvPLobbyTask)
	{
		if (strMap.empty())
			strMap = "Colosseum Lobby";
	}
	else
	{
		if (strMap.empty())
			strMap = "In Village";
	}

	snap.state = strMap;

	// Party status (only sent when actually in a party).
	if (CDnPartyTask::IsActive())
	{
		CDnPartyTask &partyTask = CDnPartyTask::GetInstance();
		if (partyTask.GetPartyRole() != CDnPartyTask::SINGLE)
		{
			int nCur = (int)partyTask.GetPartyCountExcepGM();
			if (nCur < 1)
				nCur = (int)partyTask.GetPartyCount();
			int nMax = partyTask.GetMaxPartyCount();

			if (nCur >= 1 && nMax >= nCur)
			{
				std::string strLeader;
				if (partyTask.GetPartyData(0) && partyTask.GetPartyData(0)->wszCharacterName[0] != L'\0')
					strLeader = WideToUtf8(partyTask.GetPartyData(0)->wszCharacterName);

				if (!m_bWasInParty || strLeader != m_strLastPartyLeader)
				{
					m_dwPartyGeneration++;
					m_bWasInParty = true;
					m_strLastPartyLeader = strLeader;
				}

				char szPartyId[64];
				_snprintf_s(szPartyId, sizeof(szPartyId), _TRUNCATE, "dn60_p_%lu_%u",
					(unsigned long)::GetCurrentProcessId(), (unsigned int)m_dwPartyGeneration);
				snap.partyId = szPartyId;
				snap.partySize = nCur;
				snap.partyMax = nMax;
			}
		}
		else
		{
			m_bWasInParty = false;
			m_strLastPartyLeader.clear();
		}
	}

	return snap;
}

unsigned int __stdcall CDnDiscordPresence::WorkerThreadProc(void *pParam)
{
	CDnDiscordPresence *pSelf = static_cast<CDnDiscordPresence *>(pParam);
	if (pSelf)
		pSelf->WorkerLoop();
	return 0;
}

void CDnDiscordPresence::WorkerLoop()
{
	HANDLE waitHandles[2] = { m_hStopEvent, m_hWakeEvent };
	PresenceSnapshot lastSent;
	bool bHasSentSinceConnect = false;
	DWORD dwLastConnectAttempt = 0;

	while (::WaitForSingleObject(m_hStopEvent, 0) == WAIT_TIMEOUT)
	{
		bool bStopping = false;
		bool bShouldClear = false;
		bool bHasUpdate = false;
		PresenceSnapshot nextSnap;

		::EnterCriticalSection(&m_Lock);
		if (m_bClearPending)
		{
			bShouldClear = true;
			m_bClearPending = false;
		}
		if (m_bHasPendingUpdate)
		{
			bHasUpdate = true;
			nextSnap = m_PendingSnapshot;
			m_bHasPendingUpdate = false;
		}
		::LeaveCriticalSection(&m_Lock);

		if (::WaitForSingleObject(m_hStopEvent, 0) == WAIT_OBJECT_0)
			bStopping = true;

		if (bShouldClear || bStopping)
		{
			if (m_hPipe != INVALID_HANDLE_VALUE)
			{
				SendClearActivity();
				WriteFrame(2, std::string("{\"v\":1,\"client_id\":\"") + kDiscordAppId + "\"}");
				DisconnectPipe();
			}
			break;
		}

		if (m_hPipe == INVALID_HANDLE_VALUE)
		{
			DWORD dwNow = ::GetTickCount();
			if (dwLastConnectAttempt == 0 || (dwNow - dwLastConnectAttempt) >= kReconnectIntervalMs)
			{
				dwLastConnectAttempt = dwNow;
				if (EnsureConnected())
				{
					bHasSentSinceConnect = false;
					::EnterCriticalSection(&m_Lock);
					if (!bHasUpdate)
					{
						nextSnap = m_LastQueuedSnapshot;
						bHasUpdate = nextSnap.active;
					}
					::LeaveCriticalSection(&m_Lock);
				}
			}
		}

		if (m_hPipe != INVALID_HANDLE_VALUE)
		{
			DrainIncomingFrames();

			if (bHasUpdate || (!bHasSentSinceConnect && lastSent.active))
			{
				const PresenceSnapshot &toSend = bHasUpdate ? nextSnap : lastSent;
				if (toSend.active)
				{
					if (SendSetActivity(toSend))
					{
						lastSent = toSend;
						bHasSentSinceConnect = true;
					}
					else
					{
						DisconnectPipe();
						bHasSentSinceConnect = false;
					}
				}
			}
			else if (bHasUpdate)
			{
				lastSent = nextSnap;
			}
		}
		else if (bHasUpdate)
		{
			lastSent = nextSnap;
		}

		DWORD dwWaitMs = (m_hPipe == INVALID_HANDLE_VALUE) ? 1000 : 500;
		DWORD dwRes = ::WaitForMultipleObjects(2, waitHandles, FALSE, dwWaitMs);
		if (dwRes == WAIT_OBJECT_0)
			continue; // Handle stop cleanly at top of loop.
	}
}

bool CDnDiscordPresence::EnsureConnected()
{
	if (m_hPipe != INVALID_HANDLE_VALUE)
		return true;

	for (int i = 0; i < 10; ++i)
	{
		if (::WaitForSingleObject(m_hStopEvent, 0) == WAIT_OBJECT_0)
			return false;

		char szPipeName[64];
		_snprintf_s(szPipeName, sizeof(szPipeName), _TRUNCATE, "\\\\.\\pipe\\discord-ipc-%d", i);

		HANDLE hPipe = ::CreateFileA(szPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hPipe == INVALID_HANDLE_VALUE && ::GetLastError() == ERROR_PIPE_BUSY)
		{
			if (::WaitNamedPipeA(szPipeName, 100))
				hPipe = ::CreateFileA(szPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		}
		if (hPipe == INVALID_HANDLE_VALUE)
			continue;

		m_hPipe = hPipe;

		std::string handshake = std::string("{\"v\":1,\"client_id\":\"") + kDiscordAppId + "\"}";
		if (!WriteFrame(0, handshake))
		{
			DisconnectPipe();
			continue;
		}

		DWORD dwOp = 0;
		std::string reply;
		if (!ReadAvailableFrame(&dwOp, &reply, kHandshakeTimeoutMs))
		{
			DisconnectPipe();
			continue;
		}

		if (dwOp == 1 && reply.find("\"READY\"") != std::string::npos)
			return true;

		DisconnectPipe();
	}

	return false;
}

void CDnDiscordPresence::DisconnectPipe()
{
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
	}
}

bool CDnDiscordPresence::WriteFrame(DWORD dwOpcode, const std::string &payload)
{
	if (m_hPipe == INVALID_HANDLE_VALUE)
		return false;

	DWORD header[2] = { dwOpcode, (DWORD)payload.size() };
	std::string packet;
	packet.resize(sizeof(header) + payload.size());
	memcpy(&packet[0], header, sizeof(header));
	if (!payload.empty())
		memcpy(&packet[sizeof(header)], payload.data(), payload.size());

	DWORD dwTotalWritten = 0;
	const DWORD dwTotal = (DWORD)packet.size();
	while (dwTotalWritten < dwTotal)
	{
		DWORD dwWritten = 0;
		if (!::WriteFile(m_hPipe, packet.data() + dwTotalWritten, dwTotal - dwTotalWritten, &dwWritten, NULL) || dwWritten == 0)
			return false;
		dwTotalWritten += dwWritten;
	}
	return true;
}

bool CDnDiscordPresence::ReadAvailableFrame(DWORD *pOpcode, std::string *pPayload, DWORD dwTimeoutMs)
{
	if (m_hPipe == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwStart = ::GetTickCount();
	while (true)
	{
		DWORD dwAvail = 0;
		if (!::PeekNamedPipe(m_hPipe, NULL, 0, NULL, &dwAvail, NULL))
			return false;

		if (dwAvail >= 8)
		{
			DWORD header[2] = { 0, 0 };
			DWORD dwRead = 0;
			if (!::ReadFile(m_hPipe, header, sizeof(header), &dwRead, NULL) || dwRead != sizeof(header))
				return false;

			DWORD dwLen = header[1];
			if (dwLen > 65536)
				return false;

			std::string body(dwLen, '\0');
			DWORD dwGot = 0;
			while (dwGot < dwLen)
			{
				DWORD dwChunk = 0;
				if (!::ReadFile(m_hPipe, &body[dwGot], dwLen - dwGot, &dwChunk, NULL) || dwChunk == 0)
					return false;
				dwGot += dwChunk;
			}

			if (pOpcode) *pOpcode = header[0];
			if (pPayload) pPayload->swap(body);
			return true;
		}

		if (dwTimeoutMs == 0 || (::GetTickCount() - dwStart) >= dwTimeoutMs)
			return false;

		if (::WaitForSingleObject(m_hStopEvent, 25) == WAIT_OBJECT_0)
			return false;
	}
}

void CDnDiscordPresence::DrainIncomingFrames()
{
	DWORD dwOp = 0;
	std::string dummy;
	while (ReadAvailableFrame(&dwOp, &dummy, 0))
	{
		if (dwOp == 2) // Close frame from Discord client.
		{
			DisconnectPipe();
			break;
		}
	}
}

bool CDnDiscordPresence::SendSetActivity(const PresenceSnapshot &snapshot)
{
	std::string details = SanitizePresenceField(snapshot.details);
	std::string state = SanitizePresenceField(snapshot.state);
	std::string largeText = SanitizePresenceField(snapshot.largeText);
	std::string smallText = SanitizePresenceField(snapshot.smallText);

	char szNonce[32];
	_snprintf_s(szNonce, sizeof(szNonce), _TRUNCATE, "dn60_%lu", (unsigned long)++m_dwNonceCounter);

	char szPid[32];
	_snprintf_s(szPid, sizeof(szPid), _TRUNCATE, "%lu", (unsigned long)::GetCurrentProcessId());

	std::string json = "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":";
	json += szPid;
	json += ",\"activity\":{";

	bool bNeedComma = false;
	if (!details.empty())
	{
		json += "\"details\":\"" + EscapeJson(details) + "\"";
		bNeedComma = true;
	}
	if (!state.empty())
	{
		if (bNeedComma) json += ",";
		json += "\"state\":\"" + EscapeJson(state) + "\"";
		bNeedComma = true;
	}
	if (snapshot.hasTimestamp && snapshot.startTimestamp > 0)
	{
		char szTime[48];
		_snprintf_s(szTime, sizeof(szTime), _TRUNCATE, "%lld", snapshot.startTimestamp);
		if (bNeedComma) json += ",";
		json += "\"timestamps\":{\"start\":";
		json += szTime;
		json += "}";
		bNeedComma = true;
	}

	if (!snapshot.largeImage.empty() || !snapshot.smallImage.empty())
	{
		if (bNeedComma) json += ",";
		json += "\"assets\":{";
		bool bAssetComma = false;
		if (!snapshot.largeImage.empty())
		{
			json += "\"large_image\":\"" + EscapeJson(snapshot.largeImage) + "\"";
			bAssetComma = true;
			if (!largeText.empty())
				json += ",\"large_text\":\"" + EscapeJson(largeText) + "\"";
		}
		if (!snapshot.smallImage.empty())
		{
			if (bAssetComma) json += ",";
			json += "\"small_image\":\"" + EscapeJson(snapshot.smallImage) + "\"";
			bAssetComma = true;
			if (!smallText.empty())
				json += ",\"small_text\":\"" + EscapeJson(smallText) + "\"";
		}
		json += "}";
		bNeedComma = true;
	}

	if (!snapshot.partyId.empty() && snapshot.partySize >= 1 && snapshot.partyMax >= snapshot.partySize)
	{
		char szParty[128];
		_snprintf_s(szParty, sizeof(szParty), _TRUNCATE,
			"\"party\":{\"id\":\"%s\",\"size\":[%d,%d]}",
			EscapeJson(snapshot.partyId).c_str(), snapshot.partySize, snapshot.partyMax);
		if (bNeedComma) json += ",";
		json += szParty;
		bNeedComma = true;
	}

	json += "}},\"nonce\":\"";
	json += szNonce;
	json += "\"}";

	if (!WriteFrame(1, json))
		return false;

	DrainIncomingFrames();
	return true;
}

bool CDnDiscordPresence::SendClearActivity()
{
	char szNonce[32];
	_snprintf_s(szNonce, sizeof(szNonce), _TRUNCATE, "dn60_clr_%lu", (unsigned long)++m_dwNonceCounter);

	char szPid[32];
	_snprintf_s(szPid, sizeof(szPid), _TRUNCATE, "%lu", (unsigned long)::GetCurrentProcessId());

	std::string json = "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":";
	json += szPid;
	json += ",\"activity\":null},\"nonce\":\"";
	json += szNonce;
	json += "\"}";

	return WriteFrame(1, json);
}
