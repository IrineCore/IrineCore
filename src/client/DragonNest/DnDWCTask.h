#pragma once

#if defined( PRE_ADD_DWC )

#include "Task.h"
#include "MessageListener.h"
#include "DNPacket.h"

class CDnDWCTask : public CTask, public CTaskListener, public CSingleton<CDnDWCTask>, public CEtUICallback
{
public:
	CDnDWCTask();
	virtual ~CDnDWCTask();

	bool Initialize();
	void Finalize();

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

protected:
	void OnRecvDWCMessage( int nSubCmd, char *pData, int nSize );

	void OnRecvCreateTeam( SCDWCCreateTeam *pPacket );
	void OnRecvLeaveTeam( SCDWCLeaveTeam *pPacket );
	void OnRecvDismissTeam( SCDWCDismissTeam *pPacket );
	void OnRecvInviteMemberReq( SCDWCInviteMemberReq *pPacket );
	void OnRecvInviteMemberAck( SCDWCInviteMemberAck *pPacket );
	void OnRecvGetTeamInfo( SCDWCGetTeamInfo *pPacket );
	void OnRecvGetTeamMember( SCDWCGetTeamMember *pPacket );
	void OnRecvGetRankPage( SCDWCGetRankPage *pPacket );
	void OnRecvFindRank( SCDWCFindRank *pPacket );
	void OnRecvUpdateMemberState( SCDWCUpdateMemberState *pPacket );
	void OnRecvChannelInfo( SCDWCChannelInfo *pPacket );
	void OnRecvTeamNameInfo( SCDWCTeamNameInfo *pPacket );
	void OnRecvScoreInfo( SCDWCScoreInfo *pPacket );
	void OnRecvGradeInfo( SCDWCGradeInfo *pPacket );

protected:
	bool m_bIsDWCChar;
	bool m_bHasTeam;
	bool m_bIsTeamLeader;
	bool m_bRequestWait;

	TDWCTeamInfo m_TeamInfo;
	std::vector<TDWCTeamMemberInfo> m_vecTeamMembers;
	std::vector<TDWCRankInfo> m_vecRankList;
	TDWCChannelInfo m_ChannelInfo;
	int m_nRankTotalCount;
	int m_nRankCurPage;
	BYTE m_cDWCGrade;
	int m_nDWCRank;

	UINT m_nPendingInviteTeamID;
	UINT m_nPendingInviteFromSessionID;
	std::wstring m_strPendingInviteTeamName;
	std::wstring m_strPendingInviteFromCharName;

public:
	void RequestCreateTeam( const WCHAR *wszTeamName );
	void RequestLeaveTeam();
	void RequestInviteMember( const WCHAR *wszCharacterName );
	void RequestInviteMemberAck( bool bAccept );
	void RequestGetTeamInfo( bool bNeedMemberList = true );
	void RequestGetRankPage( int nPageNum );
	void RequestFindRank( BYTE cFindType, const WCHAR *wszFindName );

	bool IsDWCChar() const { return m_bIsDWCChar; }
	void SetDWCChar( bool bDWC ) { m_bIsDWCChar = bDWC; }
	bool HasDWCTeam() const { return m_bHasTeam; }
	bool IsTeamLeader() const { return m_bIsTeamLeader; }

	const TDWCTeamInfo &GetTeamInfo() const { return m_TeamInfo; }
	const std::vector<TDWCTeamMemberInfo> &GetTeamMemberList() const { return m_vecTeamMembers; }
	const std::vector<TDWCRankInfo> &GetRankList() const { return m_vecRankList; }
	const TDWCChannelInfo &GetChannelInfo() const { return m_ChannelInfo; }
	int GetRankTotalCount() const { return m_nRankTotalCount; }
	int GetRankCurPage() const { return m_nRankCurPage; }
	BYTE GetDWCGrade() const { return m_cDWCGrade; }
	int GetDWCRank() const { return m_nDWCRank; }

	void ClearTeamInfo();
};

#define GetDWCTask() CDnDWCTask::GetInstance()

#endif // PRE_ADD_DWC
