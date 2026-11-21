#include "StdAfx.h"

#if defined( PRE_ADD_DWC )

#include "DnDWCTask.h"
#include "DWCSendPacket.h"
#include "DnInterface.h"
#include "DnDwcDlg.h"
#include "DnPvPRankDlg.h"
#include "DnActor.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnDWCTask::CDnDWCTask()
	: CTaskListener( false )
	, m_bIsDWCChar( false )
	, m_bHasTeam( false )
	, m_bIsTeamLeader( false )
	, m_bRequestWait( false )
	, m_nRankTotalCount( 0 )
	, m_nRankCurPage( 0 )
	, m_cDWCGrade( 0 )
	, m_nDWCRank( 0 )
	, m_nPendingInviteTeamID( 0 )
	, m_nPendingInviteFromSessionID( 0 )
{
	memset( &m_TeamInfo, 0, sizeof( m_TeamInfo ) );
	memset( &m_ChannelInfo, 0, sizeof( m_ChannelInfo ) );
}

CDnDWCTask::~CDnDWCTask()
{
	Finalize();
}

bool CDnDWCTask::Initialize()
{
	ClearTeamInfo();
	return true;
}

void CDnDWCTask::Finalize()
{
	ClearTeamInfo();
}

void CDnDWCTask::ClearTeamInfo()
{
	m_bHasTeam = false;
	m_bIsTeamLeader = false;
	memset( &m_TeamInfo, 0, sizeof( m_TeamInfo ) );
	m_vecTeamMembers.clear();
}

void CDnDWCTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	UNREFERENCED_PARAMETER( LocalTime );
	UNREFERENCED_PARAMETER( fDelta );
}

void CDnDWCTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	if( nMainCmd == SC_DWC )
		OnRecvDWCMessage( nSubCmd, pData, nSize );
}

void CDnDWCTask::OnRecvDWCMessage( int nSubCmd, char *pData, int nSize )
{
	UNREFERENCED_PARAMETER( nSize );

	switch( nSubCmd )
	{
	case eDWC::SC_DWC_CREATETEAM:			OnRecvCreateTeam( (SCDWCCreateTeam*)pData ); break;
	case eDWC::SC_DWC_LEAVETEAM:			OnRecvLeaveTeam( (SCDWCLeaveTeam*)pData ); break;
	case eDWC::SC_DWC_DISMISS_TEAM:			OnRecvDismissTeam( (SCDWCDismissTeam*)pData ); break;
	case eDWC::SC_DWC_INVITE_MEMBER_REQ:	OnRecvInviteMemberReq( (SCDWCInviteMemberReq*)pData ); break;
	case eDWC::SC_DWC_INVITE_MEMBER_ACK:	OnRecvInviteMemberAck( (SCDWCInviteMemberAck*)pData ); break;
	case eDWC::SC_DWC_GET_TEAMINFO:			OnRecvGetTeamInfo( (SCDWCGetTeamInfo*)pData ); break;
	case eDWC::SC_DWC_GET_TEAMMEMBER:		OnRecvGetTeamMember( (SCDWCGetTeamMember*)pData ); break;
	case eDWC::SC_DWC_GET_RANKPAGE:			OnRecvGetRankPage( (SCDWCGetRankPage*)pData ); break;
	case eDWC::SC_DWC_FIND_RANK:			OnRecvFindRank( (SCDWCFindRank*)pData ); break;
	case eDWC::SC_DWC_UPDATE_MEMBERSTATE:	OnRecvUpdateMemberState( (SCDWCUpdateMemberState*)pData ); break;
	case eDWC::SC_DWC_CHANNELINFO:			OnRecvChannelInfo( (SCDWCChannelInfo*)pData ); break;
	case eDWC::SC_DWC_TEAMNAME_INFO:		OnRecvTeamNameInfo( (SCDWCTeamNameInfo*)pData ); break;
	case eDWC::SC_DWC_SCORE_INFO:			OnRecvScoreInfo( (SCDWCScoreInfo*)pData ); break;
	case eDWC::SC_DWC_GRADE_INFO:			OnRecvGradeInfo( (SCDWCGradeInfo*)pData ); break;
	}
}

void CDnDWCTask::RequestCreateTeam( const WCHAR *wszTeamName )
{
	if( m_bRequestWait || !wszTeamName || wcslen( wszTeamName ) == 0 )
		return;
	m_bRequestWait = true;
	SendDWCCreateTeam( wszTeamName );
}

void CDnDWCTask::RequestLeaveTeam()
{
	if( m_bRequestWait )
		return;
	m_bRequestWait = true;
	SendDWCLeaveTeam();
}

void CDnDWCTask::RequestInviteMember( const WCHAR *wszCharacterName )
{
	if( !wszCharacterName || wcslen( wszCharacterName ) == 0 )
		return;
	SendDWCInviteMember( wszCharacterName );
}

void CDnDWCTask::RequestInviteMemberAck( bool bAccept )
{
	if( m_nPendingInviteTeamID == 0 )
		return;
	SendDWCInviteMemberAck( m_nPendingInviteTeamID, m_nPendingInviteFromSessionID, bAccept );
	m_nPendingInviteTeamID = 0;
	m_nPendingInviteFromSessionID = 0;
}

void CDnDWCTask::RequestGetTeamInfo( bool bNeedMemberList )
{
	SendDWCGetTeamInfo( bNeedMemberList );
}

void CDnDWCTask::RequestGetRankPage( int nPageNum )
{
	SendDWCGetRankPage( nPageNum );
}

void CDnDWCTask::RequestFindRank( BYTE cFindType, const WCHAR *wszFindName )
{
	SendDWCFindRank( cFindType, wszFindName );
}

void CDnDWCTask::OnRecvCreateTeam( SCDWCCreateTeam *pPacket )
{
	m_bRequestWait = false;
	if( !pPacket )
		return;

	if( pPacket->nRetCode == ERROR_NONE )
	{
		m_bHasTeam = true;
		m_bIsTeamLeader = true;
		m_TeamInfo.nTeamID = pPacket->nTeamID;
		_wcscpy( m_TeamInfo.wszTeamName, _countof( m_TeamInfo.wszTeamName ), pPacket->wszTeamName, (int)wcslen( pPacket->wszTeamName ) );

		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120237 ), true );
		if( GetInterface().GetDwcCreateTeamDlg() )
			GetInterface().GetDwcCreateTeamDlg()->Show( false );
		if( GetInterface().GetDwcDlg() )
			GetInterface().GetDwcDlg()->RefreshTeamInfo();

		RequestGetTeamInfo( true );
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}

void CDnDWCTask::OnRecvLeaveTeam( SCDWCLeaveTeam *pPacket )
{
	m_bRequestWait = false;
	if( !pPacket )
		return;

	if( pPacket->nRetCode == ERROR_NONE )
	{
		std::wstring strMsg = FormatW( L"%s %s", pPacket->wszLeaveCharName, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3529 ) );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", strMsg.c_str(), true );

		bool bIsLocal = false;
		if( CDnActor::s_hLocalActor && wcscmp( CDnActor::s_hLocalActor->GetName(), pPacket->wszLeaveCharName ) == 0 )
			bIsLocal = true;

		if( bIsLocal )
		{
			ClearTeamInfo();
			if( GetInterface().GetDwcDlg() )
				GetInterface().GetDwcDlg()->Show( false );
		}
		else
		{
			for( std::vector<TDWCTeamMemberInfo>::iterator it = m_vecTeamMembers.begin(); it != m_vecTeamMembers.end(); )
			{
				if( it->biCharacterDBID == pPacket->biLeaveCharDBID )
					it = m_vecTeamMembers.erase( it );
				else
					++it;
			}
			if( GetInterface().GetDwcDlg() )
				GetInterface().GetDwcDlg()->RefreshTeamInfo();
		}
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}

void CDnDWCTask::OnRecvDismissTeam( SCDWCDismissTeam *pPacket )
{
	m_bRequestWait = false;
	if( !pPacket )
		return;

	if( pPacket->nRetCode == ERROR_NONE )
	{
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120235 ), true );
		ClearTeamInfo();
		if( GetInterface().GetDwcDlg() && GetInterface().GetDwcDlg()->IsShow() )
			GetInterface().GetDwcDlg()->Show( false );
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}

void CDnDWCTask::OnRecvInviteMemberReq( SCDWCInviteMemberReq *pPacket )
{
	if( !pPacket )
		return;

	m_nPendingInviteTeamID = pPacket->nTeamID;
	m_nPendingInviteFromSessionID = pPacket->nFromSessionID;
	m_strPendingInviteTeamName = pPacket->wszTeamName;
	m_strPendingInviteFromCharName = pPacket->wszFromCharName;

	CDnDwcInviteReqDlg *pInviteDlg = GetInterface().GetDwcInviteReqDlg();
	if( pInviteDlg )
	{
		pInviteDlg->SetInviteInfo( pPacket->nTeamID, pPacket->nFromSessionID, pPacket->wszTeamName, pPacket->wszFromCharName, 15.0f );
		pInviteDlg->Show( true );
	}
}

void CDnDWCTask::OnRecvInviteMemberAck( SCDWCInviteMemberAck *pPacket )
{
	if( !pPacket )
		return;

	if( pPacket->nRetCode == ERROR_NONE )
	{
		RequestGetTeamInfo( true );
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}

void CDnDWCTask::OnRecvGetTeamInfo( SCDWCGetTeamInfo *pPacket )
{
	if( !pPacket )
		return;

	if( pPacket->nRetCode == ERROR_NONE && pPacket->TeamInfo.nTeamID > 0 )
	{
		m_bHasTeam = true;
		m_TeamInfo = pPacket->TeamInfo;
	}
	else
	{
		ClearTeamInfo();
	}

	if( GetInterface().GetDwcDlg() )
		GetInterface().GetDwcDlg()->RefreshTeamInfo();
}

void CDnDWCTask::OnRecvGetTeamMember( SCDWCGetTeamMember *pPacket )
{
	if( !pPacket )
		return;

	m_vecTeamMembers.clear();
	m_bIsTeamLeader = false;
	if( pPacket->nRetCode == ERROR_NONE )
	{
		int nCount = pPacket->nCount;
		if( nCount > DWC::MAX_TEAM_MEMBER )
			nCount = DWC::MAX_TEAM_MEMBER;
		for( int i = 0; i < nCount; ++i )
		{
			m_vecTeamMembers.push_back( pPacket->MemberList[i] );
			if( pPacket->MemberList[i].bTeamLeader && CDnActor::s_hLocalActor &&
				wcscmp( CDnActor::s_hLocalActor->GetName(), pPacket->MemberList[i].wszCharacterName ) == 0 )
			{
				m_bIsTeamLeader = true;
			}
		}
	}

	if( GetInterface().GetDwcDlg() )
		GetInterface().GetDwcDlg()->RefreshTeamInfo();
}

void CDnDWCTask::OnRecvGetRankPage( SCDWCGetRankPage *pPacket )
{
	if( !pPacket )
		return;

	m_vecRankList.clear();
	if( pPacket->nRetCode == ERROR_NONE )
	{
		m_nRankTotalCount = pPacket->nTotalCount;
		m_nRankCurPage = pPacket->nPageNum;
		int nCount = pPacket->nCount;
		if( nCount > DWC::MAX_RANK_PER_PAGE )
			nCount = DWC::MAX_RANK_PER_PAGE;
		for( int i = 0; i < nCount; ++i )
			m_vecRankList.push_back( pPacket->RankList[i] );
	}
#ifdef PRE_ADD_PVP_RANKING
	if( GetInterface().GetPvPRankDlg() && GetInterface().GetPvPRankDlg()->IsShow() )
		GetInterface().GetPvPRankDlg()->RefreshDWCRankList();
#endif
}

void CDnDWCTask::OnRecvFindRank( SCDWCFindRank *pPacket )
{
	if( !pPacket )
		return;

	if( pPacket->nRetCode == ERROR_NONE )
	{
		m_vecRankList.clear();
		m_vecRankList.push_back( pPacket->RankInfo );
#ifdef PRE_ADD_PVP_RANKING
		if( GetInterface().GetPvPRankDlg() && GetInterface().GetPvPRankDlg()->IsShow() )
			GetInterface().GetPvPRankDlg()->RefreshDWCRankList();
#endif
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}

void CDnDWCTask::OnRecvUpdateMemberState( SCDWCUpdateMemberState *pPacket )
{
	if( !pPacket || pPacket->nTeamID != m_TeamInfo.nTeamID )
		return;

	for( size_t i = 0; i < m_vecTeamMembers.size(); ++i )
	{
		if( m_vecTeamMembers[i].biCharacterDBID == pPacket->biCharacterDBID )
		{
			m_vecTeamMembers[i].nMapIndex = pPacket->nMapIndex;
			m_vecTeamMembers[i].nChannelID = pPacket->nChannelID;
			m_vecTeamMembers[i].cConnectState = pPacket->cConnectState;
			break;
		}
	}

	if( GetInterface().GetDwcDlg() )
		GetInterface().GetDwcDlg()->RefreshTeamInfo();
}

void CDnDWCTask::OnRecvChannelInfo( SCDWCChannelInfo *pPacket )
{
	if( !pPacket )
		return;
	m_ChannelInfo = pPacket->ChannelInfo;
}

void CDnDWCTask::OnRecvTeamNameInfo( SCDWCTeamNameInfo *pPacket )
{
	UNREFERENCED_PARAMETER( pPacket );
}

void CDnDWCTask::OnRecvScoreInfo( SCDWCScoreInfo *pPacket )
{
	if( !pPacket )
		return;
	m_TeamInfo.nDWCPoint = pPacket->nDWCPoint;
	m_TeamInfo.nTotalWin = pPacket->nTotalWin;
	m_TeamInfo.nTotalLose = pPacket->nTotalLose;
	m_TeamInfo.nTotalDraw = pPacket->nTotalDraw;

	if( GetInterface().GetDwcDlg() )
		GetInterface().GetDwcDlg()->RefreshTeamInfo();
}

void CDnDWCTask::OnRecvGradeInfo( SCDWCGradeInfo *pPacket )
{
	if( !pPacket )
		return;
	m_nDWCRank = pPacket->nRank;
	m_cDWCGrade = pPacket->cGrade;
	m_TeamInfo.nRank = pPacket->nRank;
}

void CDnDWCTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	UNREFERENCED_PARAMETER( nID );
	UNREFERENCED_PARAMETER( nCommand );
	UNREFERENCED_PARAMETER( pControl );
	UNREFERENCED_PARAMETER( uiMsg );
}

#endif // PRE_ADD_DWC
