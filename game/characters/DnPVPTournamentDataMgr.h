#pragma once

#include "DnTableDB.h"
#include "DnUIDefine.h"
#include "DnBaseRoomDlg.h"
#include <set>

#ifdef PRE_ADD_PVP_TOURNAMENT

using namespace PvPTournamentUIDef;

class CDnItem;
class CDnParts;
class CDnPVPTournamentDataMgr
{
public:
	CDnPVPTournamentDataMgr();

	void SetRoomInfo(PvPCommon::RoomInfo& info);
	void SetRoomState(UINT roomState);
	const SBasicRoomInfo& GetRoomInfo() const { return m_RoomInfo; }

	bool IsReadyRoom() const;
	bool IsPasssWordRoom() const;
	bool IsSyncingRoom() const;
	bool IsStartingRoom() const;
	bool IsPlayingRoom() const;
	bool IsInGamePlayingRoom() const;
	bool IsInBattlePlayer(const UINT& sessionID) const;

	int GetCurrentMemberCount() const;

	static int GetMemberCountOfMode(eMatchModeByCount modeType);
	static eMatchModeByCount GetModeByMemberCount(int memberCount);

	eMatchModeByCount GetCurrentMode() const;

	const SMatchUserInfo* GetSlotInfoByCommonIndex(int commonIndex) const;
	const SMatchUserInfo* GetSlotInfoBySessionID(UINT uiSessionID) const;
	const SMatchUserInfo* GetSlotInfoByUIIndex(int matchUIIndex) const;

	void GetNeedUpdateMatchInfoIndexes(std::vector<int>& indexList) const;
	const std::vector<SMatchUserInfo>& GetMatchUserList() const { return m_MatchUserList; }

	int GetEventRoomIndex() const;

	bool SetInfo(const SMatchUserInfo& info);
	bool RemoveInfo(const UINT sessionID);
	bool MakeMatchListIndexerByMode(eMatchModeByCount mode);
	bool SwapSlotByCommonIndex(const int& sourceIndex, const int& destIndex);

	void HoldSwapPosition(bool bHold);
	bool IsHoldSwapPosition() const { return m_bHoldSwapPosition; }

	void SetEditPositionState(bool bEnable) { m_bEditPositionState = bEnable; }
	bool IsStateEditPosition() const { return m_bEditPositionState; }
	bool IsEnableStartGame() const;

	void SetReadyState(bool bSetReady) { m_bReadyState = bSetReady; }
	bool IsStateReady() const { return m_bReadyState; }

	void SetGameMatchUserList(const SCPvPTournamentMatchList& packet);
	void SetGameMatchCurrentUser(const std::vector<UINT>& sessionIds);
	const std::vector<SGameMatchUserInfo>& GetGameMatchUserList() const;

	bool AmIMaster() const;

	void Reset();
	void ResetRoomInfo();

	void AddUserToFinalReportList(const SFinalReportUserInfo& info);
	void RemoveUserFromFinalReportList(UINT sessionID);
	std::vector<SFinalReportUserInfo>& GetFinalReportUserList() { return m_FinalReportUserList; }
	SFinalReportUserInfo* GetFinalReportUser(UINT sessionID);
	void SetFinalReportUser_TournamentStep(UINT sessionID, int tournamentStep);
	void SortFinalReportUserList();
	void SetFinalReportUser_WinnersRank(UINT* winnersSessionId, int count);
	bool IsAllPlayerReady() const;

	void ClearGameMatchCurrentUser();

private:
	int GetMatchListUIStartIndexByCount(eMatchModeByCount mode) const;
	int GetUIIndexByCommonIndex(int commonIndex) const;

	// todo by kalliste : Merge MatchUserList (room info) / GameMatchUserList (bracket) / FinalReportUserList (results)
	// m_MatchUserList holds info of users competing from the room
	// m_GameMatchUserList is data used for the in-game tournament bracket
	// The two data sets are separated due to different uses, though they could later be grouped and managed under a user key. 
	// (matchuserlist only inserts/deletes in Room, so no sync mismatch concern)

	SBasicRoomInfo m_RoomInfo;
	std::vector<int> m_MatchListIndexer;	// vector index : commonIndex

	std::vector<SMatchUserInfo> m_MatchUserList;
	std::vector<SGameMatchUserInfo> m_GameMatchUserList;
	std::vector<SFinalReportUserInfo> m_FinalReportUserList;

	bool m_bHoldSwapPosition;
	bool m_bEditPositionState;
	bool m_bReadyState;
};

#endif // PRE_ADD_PVP_TOURNAMENT