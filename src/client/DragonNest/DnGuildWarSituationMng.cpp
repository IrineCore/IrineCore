#include "stdafx.h"
#include "DnGuildWarSituationMng.h"
#include "DnMiniPlayerGuildWarGaugeDlg.h"
#include "DnMiniPlayerGuildWarMainTabDlg.h"
#include "DnMiniPlayerGuildWarTabDlg.h"
#include "DnMiniPlayerGuildWarPopupDlg.h"
#include "TaskManager.h"
#include "DnPartyTask.h"
#include "DnMouseCursor.h"
#include "DnBridgeTask.h"
#include "DnInterface.h"
#include "PvPSendPacket.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnGuildWarSituationMng::CDnGuildWarSituationMng()
: m_pMainTabDlg( NULL )
, m_bPartyShow( false )
, m_bSituationShow( false )
, m_bNeedUpdate( false )
, m_bPartyRefresh( false )
, m_cPartyCount( 0 )
, m_uiMyState( 0 )
, m_cMyPartyPosition( 0 )
, m_pPopupDlg( NULL )
, m_bShowPopup( false )
, m_bPartyMove( false )
, m_cSelectUserCount( 0 )
{
	memset( m_pPartyTabDlgs, NULL, sizeof(CDnMiniPlayerGuildWarTabDlg *)*MAX_PARTY_COUNT );
	memset( m_pUserGaugeDlgs, NULL, sizeof(CDnMiniPlayerGuildWarGaugeDlg *)*MAX_USER_COUNT );
	memset( m_pPartyUserGaugeDlgs, NULL, sizeof(CDnMiniPlayerGuildWarGaugeDlg *)*MAX_PARTY_USER_COUNT );
	m_pSelectPartyTabDlg = NULL;
}

CDnGuildWarSituationMng::~CDnGuildWarSituationMng()
{
	SAFE_DELETE( m_pMainTabDlg );

	for( int itr = 0; itr < MAX_PARTY_COUNT; ++itr )
		SAFE_DELETE( m_pPartyTabDlgs[itr] );

	for( int itr = 0; itr < MAX_USER_COUNT; ++itr )
		SAFE_DELETE( m_pUserGaugeDlgs[itr] );

	for( int itr = 0; itr < MAX_PARTY_USER_COUNT; ++itr )
		SAFE_DELETE( m_pPartyUserGaugeDlgs[itr] );

	SAFE_DELETE( m_pPopupDlg );
	SAFE_DELETE( m_pSelectPartyTabDlg );
}

void CDnGuildWarSituationMng::Initialize()
{
	m_pMainTabDlg = new CDnMiniPlayerGuildWarMainTabDlg;
	m_pMainTabDlg->Initialize( false );
	m_pMainTabDlg->SetGuildWarSituation( this );

	for( int itr = 0; itr < MAX_PARTY_COUNT; ++itr )
	{
		m_pPartyTabDlgs[itr] = new CDnMiniPlayerGuildWarTabDlg(UI_TYPE_BOTTOM_MSG);
		m_pPartyTabDlgs[itr]->Initialize( false );
		m_pPartyTabDlgs[itr]->SetPartyName( itr + 1 );
	}

	for( int itr = 0; itr < MAX_USER_COUNT; ++itr )
	{
		m_pUserGaugeDlgs[itr] = new CDnMiniPlayerGuildWarGaugeDlg(UI_TYPE_BOTTOM_MSG);
		m_pUserGaugeDlgs[itr]->Initialize( false );
		m_pUserGaugeDlgs[itr]->SetGuildWarSituationMng( this );
	}

	for( int itr = 0; itr < MAX_PARTY_USER_COUNT; ++itr )
	{
		m_pPartyUserGaugeDlgs[itr] = new CDnMiniPlayerGuildWarGaugeDlg(UI_TYPE_BOTTOM_MSG);
		m_pPartyUserGaugeDlgs[itr]->Initialize( false );
		m_pPartyUserGaugeDlgs[itr]->SetGuildWarSituationMng( this );
	}

	m_pPopupDlg = new CDnMiniPlayerGuildWarPopupDlg;
	m_pPopupDlg->Initialize( false );

	m_pSelectPartyTabDlg = new CDnMiniPlayerGuildWarTabDlg(UI_TYPE_BOTTOM_MSG);
	m_pSelectPartyTabDlg->Initialize( false );

	InitializeSituaion();
}

void CDnGuildWarSituationMng::InitializeSituaion()
{
	m_pMainTabDlg->PartyShowCheck( true );
	SituationShow( false );
}

void CDnGuildWarSituationMng::AddUser( DnActorHandle hActor, void * pData )
{
	CDnPartyTask::PartyStruct * pInfo = (CDnPartyTask::PartyStruct *)pData;

	if( !pInfo )
		return;

	int nMemberIndex = pInfo->cMemberIndex - (CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::B ? MAX_USER_COUNT : 0);

	if( nMemberIndex < 0 || nMemberIndex >= MAX_USER_COUNT )
	{
		OutputDebug("Situation board AddUser member index is invalid.");
		return;
	}

	if( m_pUserGaugeDlgs[ nMemberIndex ]->IsUser() )
		return;

	m_pUserGaugeDlgs[ nMemberIndex ]->SetUser( pInfo );

	// Not yet added as a party member in CDnPartyTask at this point, so Refresh in the next process.
	m_bPartyRefresh = true;
}

void CDnGuildWarSituationMng::RemoveUser(DnActorHandle hActor)
{
	for( int itr = 0; itr < MAX_USER_COUNT; ++itr )
	{
		if( m_pUserGaugeDlgs[itr]->IsUser() && m_pUserGaugeDlgs[itr]->GetActor() && m_pUserGaugeDlgs[itr]->GetActor()->GetUniqueID() == hActor->GetUniqueID() )
		{
			m_pUserGaugeDlgs[itr]->ClearUser();
			break;
		}
	}

	PartyRefresh();
}

void CDnGuildWarSituationMng::MoveUser( SCPvPMemberIndex * pPacket )
{
	if( !CDnActor::s_hLocalActor )	return;

	if( CDnActor::s_hLocalActor->GetTeam() != pPacket->nTeam )
		return;

	if( PvPCommon::Team::B == pPacket->nTeam )
	{
		pPacket->Index[0].cIndex -= MAX_USER_COUNT;
		pPacket->Index[1].cIndex -= MAX_USER_COUNT;

		if( pPacket->Index[0].cIndex < 0 || pPacket->Index[1].cIndex < 0 )
		{
			OutputDebug("CDnGuildWarSituation MoveUser Index Error");
			return;
		}
	}

	m_pUserGaugeDlgs[ pPacket->Index[0].cIndex ]->ClearUser();
	m_pUserGaugeDlgs[ pPacket->Index[1].cIndex ]->ClearUser();

	CDnPartyTask::PartyStruct * pFirstInfo = NULL;
	CDnPartyTask::PartyStruct * pSecondInfo = NULL;

	DWORD dwPartyCount = GetPartyTask().GetPartyCount();

	for( DWORD itr = 0; itr < dwPartyCount; ++itr )
	{
		CDnPartyTask::PartyStruct * pInfo = GetPartyTask().GetPartyData(itr);

		if( !pInfo )
		{
			_ASSERT("Hey, no party member info found");
			return;
		}

		if( pPacket->Index[0].nSessionID == pInfo->nSessionID )
			pFirstInfo = GetPartyTask().GetPartyData(itr);
		else if( pPacket->Index[1].nSessionID == pInfo->nSessionID )
			pSecondInfo = GetPartyTask().GetPartyData(itr);
	}

	if( pFirstInfo )
		m_pUserGaugeDlgs[ pPacket->Index[0].cIndex ]->SetUser( pFirstInfo );
	if( pSecondInfo )
		m_pUserGaugeDlgs[ pPacket->Index[1].cIndex ]->SetUser( pSecondInfo );

	if( CDnActor::s_hLocalActor->GetUniqueID() == pPacket->Index[0].nSessionID )
		m_cMyPartyPosition = pPacket->Index[0].cIndex / MAX_PARTY_COUNT;
	else if( CDnActor::s_hLocalActor->GetUniqueID() == pPacket->Index[1].nSessionID )
		m_cMyPartyPosition = pPacket->Index[1].cIndex / MAX_PARTY_COUNT;

	PartyRefresh();
}

void CDnGuildWarSituationMng::SwapUser()
{

}

void CDnGuildWarSituationMng::SetUserState( UINT uiSessionID, UINT uiUserState )
{
	CDnPartyTask::PartyStruct * pInfo = GetPartyTask().GetPartyDataFromSessionID( uiSessionID, true );

	if( !pInfo )
	{
#ifdef _WORK
		WCHAR wszMessage[512];
		swprintf_s( wszMessage, _countof(wszMessage), L"SetUserState : Invalid user state info!! Please take a screenshot and send it to verygoodd21." );
		GetInterface().MessageBox( wszMessage );
#endif
		return;
	}
	pInfo->uiPvPUserState = uiUserState;

	if( uiSessionID == CDnBridgeTask::GetInstance().GetSessionID() )
	{
		m_uiMyState = uiUserState;
		GetInterface().SetGuildWarSkillUI( m_uiMyState, false );
	}
	else if( PvPCommon::UserState::GuildWarCaptain & m_uiMyState && PvPCommon::UserState::GuildWarCaptain & uiUserState )
	{
		m_uiMyState = PvPCommon::UserState::None;
		GetInterface().SetGuildWarSkillUI( m_uiMyState, false );
	}

	for( int itr = 0; itr < MAX_USER_COUNT; ++itr )
	{
		if( m_pUserGaugeDlgs[itr]->IsUser() )
		{
			DnActorHandle hActor = m_pUserGaugeDlgs[itr]->GetActor();

			if( !hActor )	continue;

			if( uiSessionID == hActor->GetUniqueID() )
				m_pUserGaugeDlgs[itr]->SetUserState( uiUserState );
			else if( PvPCommon::UserState::GuildWarCaptain & uiUserState && PvPCommon::UserState::GuildWarCaptain & m_pUserGaugeDlgs[itr]->GetUserState() )
			{
				CDnPartyTask::PartyStruct * pInfo = GetPartyTask().GetPartyDataFromSessionID( hActor->GetUniqueID(), true );

				if( !pInfo )
				{
#ifdef _WORK
					WCHAR wszMessage[512];
					swprintf_s( wszMessage, _countof(wszMessage), L"SetUserState1 : Invalid user state info!! Please take a screenshot and send it to verygoodd21." );
					GetInterface().MessageBox( wszMessage );
#endif
					return;
				}
				pInfo->uiPvPUserState = PvPCommon::UserState::None;
				m_pUserGaugeDlgs[itr]->SetUserState( PvPCommon::UserState::None );
			}
		}
	}

	PartyRefresh();

	if( PvPCommon::UserState::GuildWarCaptain & m_uiMyState )
		m_pMainTabDlg->SetMaster( true );
	else
		m_pMainTabDlg->SetMaster( false );
}

void CDnGuildWarSituationMng::SetSlotMove( bool bMove )
{
	m_bPartyMove = bMove;

	if( !m_bPartyMove )
		m_cSelectUserCount = 0;

	for( int itr = 0; itr < MAX_USER_COUNT; ++itr )
		m_pUserGaugeDlgs[itr]->SetSelectMode( bMove );
}

void CDnGuildWarSituationMng::PartyShow( bool bShow )
{
	m_bPartyShow = bShow;

	for( int itr = 0; itr < MAX_PARTY_USER_COUNT; ++itr )
	{
		m_pPartyUserGaugeDlgs[itr]->Show( bShow );

		if( bShow )
		{
			if( !m_pPartyUserGaugeDlgs[itr]->IsUser() || !m_pPartyUserGaugeDlgs[itr]->GetActor() )
				m_pPartyUserGaugeDlgs[itr]->Show( false );

			m_pPartyUserGaugeDlgs[itr]->SetPartyShow();
		}
	}

	m_pSelectPartyTabDlg->Show( bShow );
}

void CDnGuildWarSituationMng::SituationShow( bool bShow )
{
	if( m_bSituationShow != bShow )
	{
		if( bShow )
			CDnMouseCursor::GetInstance().ShowCursor( true, true );
	}

	m_bSituationShow = bShow;

	int nShowCount = m_cPartyCount * MAX_PARTY_USER_COUNT;

	for( int itr = 0; itr < MAX_PARTY_COUNT; ++itr )
	{
		if( bShow )
		{
			if( itr < m_cPartyCount )
				m_pPartyTabDlgs[itr]->Show( true );
			else
				m_pPartyTabDlgs[itr]->Show( false );
		}
		else
			m_pPartyTabDlgs[itr]->Show( false );
	}

	for( int itr = 0; itr < MAX_USER_COUNT; ++itr )
	{
		if( bShow )
		{
			if( itr < nShowCount )
				m_pUserGaugeDlgs[itr]->Show( true );
			else
				m_pUserGaugeDlgs[itr]->Show( false );
		}
		else
			m_pUserGaugeDlgs[itr]->Show( false );
	}

	m_pMainTabDlg->Show( bShow );

	if( bShow )
	{
		m_pPopupDlg->Show(m_bShowPopup);
	}
	else
	{	
		m_bShowPopup = false;
		m_pPopupDlg->Show(m_bShowPopup);
	}
}

void CDnGuildWarSituationMng::PartyPosition()
{
	float fX = 0.0f, fY = 0.18f;
	SUICoord Crood = m_pPartyUserGaugeDlgs[0]->GetDlgCoord();
	SUICoord tabCoord = m_pSelectPartyTabDlg->GetDlgCoord();
	float fHeight = Crood.fHeight;

	m_pSelectPartyTabDlg->SetPosition( fX, fY );
	fY += tabCoord.fHeight;

	for( int itr = 0; itr < MAX_PARTY_USER_COUNT; ++itr )
	{
		m_pPartyUserGaugeDlgs[itr]->SetPosition( fX, fY );
		fY += fHeight + 0.02f;
	}
}

void CDnGuildWarSituationMng::SituationPosition()
{
	SUICoord GaugeCoord = m_pUserGaugeDlgs[0]->GetDlgCoord(),
			 PartyCoord = m_pPartyTabDlgs[0]->GetDlgCoord(),
			 MainCoord = m_pMainTabDlg->GetDlgCoord();

	float fWidthRatio = m_pUserGaugeDlgs[0]->GetScreenWidthRatio(), 
	  	  fHeightRatio = m_pUserGaugeDlgs[0]->GetScreenHeightRatio();

	float fWidth = GaugeCoord.fWidth, 
		  fHeight = GaugeCoord.fHeight;

	float fX = 0.5f * fWidthRatio, fY = 0.0f;
	
	float fWidthGap = 0.002f, 
		  fHeightGap= 0.015f;

	int nMax = m_cPartyCount * MAX_USER_COUNT / MAX_PARTY_COUNT;

	if( 1 == m_cPartyCount )		fX -= fWidth/2;
	else if( 2 == m_cPartyCount )	fX -= fWidth + fWidthGap;
	else if( 3 == m_cPartyCount )	fX -= fWidth + fWidth/2 + fWidthGap;
	else if( 4 == m_cPartyCount )	fX -= (fWidth * 2.0f) + fWidthGap;

	for( int itr = 0; itr < nMax; ++itr )
	{
		if( 0 == itr % MAX_PARTY_USER_COUNT )
		{
			if( 0 != itr )
				fX += fWidth + fWidthGap;

			fY = MainCoord.fHeight * 0.6f;
			m_pPartyTabDlgs[itr / MAX_PARTY_USER_COUNT]->SetPosition( fX, fY );
			fY += PartyCoord.fHeight;
		}

		m_pUserGaugeDlgs[itr]->SetPosition( fX, fY );
		fY += fHeight + fHeightGap;
	}
}

void CDnGuildWarSituationMng::OnChangeResolution()
{
	PartyPosition();
	SituationPosition();
}

void CDnGuildWarSituationMng::Process( float fDelta )
{
	if( m_bNeedUpdate )
	{
		OnChangeResolution();
		m_bNeedUpdate = false;
	}

	if( m_bPartyRefresh )
	{
		PartyRefresh();
		m_bPartyRefresh = false;
	}

	for( int itr = 0; itr < MAX_USER_COUNT; ++itr )
	{
		if( m_pUserGaugeDlgs[itr]->IsShow() )
			m_pUserGaugeDlgs[itr]->ProcessGauge();
	}

	if( m_bPartyShow )
	{
		for( int itr = 0; itr < MAX_PARTY_USER_COUNT; ++itr )
		{
			if( m_pPartyUserGaugeDlgs[itr]->IsShow() )
				m_pPartyUserGaugeDlgs[itr]->ProcessGauge();
		}
	}
}

void CDnGuildWarSituationMng::Refresh()
{
	DWORD dwPartyCount = GetPartyTask().GetPartyCount();
	m_cPartyCount = MAX_PARTY_COUNT;
	m_pMainTabDlg->SetPartyCount( MAX_PARTY_COUNT - 1 );

	int nTeam = CDnActor::s_hLocalActor->GetTeam();
	int nUser = 0, nUserIndex = 0;
	m_cMyPartyPosition = 0;

	for( DWORD itr = 0; itr < dwPartyCount; ++itr )
	{
		CDnPartyTask::PartyStruct *pInfo = GetPartyTask().GetPartyData(itr);
		if( !pInfo || pInfo->usTeam != nTeam || !pInfo->hActor || pInfo->hActor->GetTeam() == PvPCommon::Team::Observer) continue;

		if( PvPCommon::Team::A == pInfo->usTeam )
			nUserIndex = pInfo->cMemberIndex;
		else if( PvPCommon::Team::B == pInfo->usTeam )
			nUserIndex = pInfo->cMemberIndex - MAX_USER_COUNT;

		if( nUserIndex < 0 || nUserIndex >= MAX_USER_COUNT )
		{
#ifdef _WORK
			WCHAR wszMessage[512];
			swprintf_s( wszMessage, _countof(wszMessage), L"Refresh : Guild war index is messed up!! Please take a screenshot and send it to verygoodd21.%d, %d, %d, %d", nUserIndex, pInfo->cMemberIndex, nTeam, pInfo->usTeam );
			GetInterface().MessageBox( wszMessage );
#endif
			continue;
		}

		m_pUserGaugeDlgs[ nUserIndex ]->SetUser( pInfo );

		if( pInfo->hActor->GetUniqueID() == CDnActor::s_hLocalActor->GetUniqueID() )	// In case of self
		{
			m_uiMyState = pInfo->uiPvPUserState;
			if( PvPCommon::UserState::GuildWarCaptain & m_uiMyState )
				m_pMainTabDlg->SetMaster( true );
			else
				m_pMainTabDlg->SetMaster( false );
			m_cMyPartyPosition = nUserIndex / 4;
		}
	}

	PartyRefresh();

	OnChangeResolution();
}

void CDnGuildWarSituationMng::PartyRefresh()
{
	int nUser = m_cMyPartyPosition * MAX_PARTY_USER_COUNT;
	DnActorHandle hActor;

#ifdef PRE_MOD_PVPOBSERVER
	if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer )
		m_pSelectPartyTabDlg->Show( false );
	else
#endif // PRE_MOD_PVPOBSERVER
		m_pSelectPartyTabDlg->SetPartyName( m_cMyPartyPosition + 1 );

	if( nUser < 0 || nUser >= MAX_USER_COUNT || MAX_PARTY_COUNT <= m_cMyPartyPosition )
	{
#ifdef _WORK
		WCHAR wszMessage[512];
		swprintf_s( wszMessage, _countof(wszMessage), L"PartyRefresh : Guild war index is messed up!! Please take a screenshot and send it to verygoodd21.%d", nUser );
		GetInterface().MessageBox( wszMessage );
#endif
		return;
	}

	int nPartyIndex = 0;
	for( int itr = 0; itr < MAX_PARTY_USER_COUNT; ++itr, ++nUser )
	{
		m_pPartyUserGaugeDlgs[itr]->ClearUser();
		m_pPartyUserGaugeDlgs[itr]->Show( false );

		hActor = m_pUserGaugeDlgs[nUser]->GetActor();
		if( !m_pUserGaugeDlgs[nUser]->IsUser() || !hActor ) continue;

		CDnPartyTask::PartyStruct *pInfo = GetPartyTask().GetPartyDataFromSessionID( hActor->GetUniqueID(), true );
		if( !pInfo )	continue;

		m_pPartyUserGaugeDlgs[nPartyIndex]->SetUser( pInfo );
		m_pPartyUserGaugeDlgs[nPartyIndex]->Show( true );
		m_pPartyUserGaugeDlgs[nPartyIndex]->SetPartyShow();

		++nPartyIndex;
	}
}

void CDnGuildWarSituationMng::ShowPopupDlg(  float fx, float fy, UINT uiUserState, UINT uiSessionID, bool bShow )
{
	m_bShowPopup = bShow;

	if( m_bShowPopup )
	{
		static float fyBGap(16.f /DEFAULT_UI_SCREEN_WIDTH);
		m_pPopupDlg->SetPosition( fx+fyBGap, fy );

		// Whether there are 3 or more sub-masters
		int nSecondaryMaster = 0;
		for( int itr = 0; itr < MAX_USER_COUNT; ++itr )
		{
			if( m_pUserGaugeDlgs[itr]->IsUser() 
				&& m_pUserGaugeDlgs[itr]->GetUserState() & PvPCommon::UserState::GuildWarSedcondCaptain )
			{
				++nSecondaryMaster;
			}
		}

		m_pPopupDlg->SetControl( m_uiMyState, uiUserState, uiSessionID, nSecondaryMaster < 3 );
	}
}

void CDnGuildWarSituationMng::SelectUser( CDnMiniPlayerGuildWarGaugeDlg * pDlg )
{
	if( pDlg->IsSelect() )
		++m_cSelectUserCount;
	else
		--m_cSelectUserCount;

	CDnMiniPlayerGuildWarGaugeDlg * pFirstDlg = NULL;
	CDnMiniPlayerGuildWarGaugeDlg * pSecondDlg = NULL;
	int nFirstIndex, nSecondIndex;

	if( 2 == m_cSelectUserCount )
	{
		for( int itr = 0; itr < MAX_USER_COUNT; ++itr )
		{
			if( m_pUserGaugeDlgs[itr]->IsSelect() )
			{
				if( !pFirstDlg )
				{
					pFirstDlg = m_pUserGaugeDlgs[itr];
					nFirstIndex = itr;
				}
				else
				{
					pSecondDlg = m_pUserGaugeDlgs[itr];
					nSecondIndex = itr;
					break;
				}
			}
		}

		m_cSelectUserCount = 0;
	}

	if( pSecondDlg )
	{
		pFirstDlg->Select( false );
		pSecondDlg->Select( false );

		BYTE pIndex[2];
		UINT pSessionID[2];

		pIndex[0] = nFirstIndex + (CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::B ? MAX_USER_COUNT : 0);
		pIndex[1] = nSecondIndex + (CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::B ? MAX_USER_COUNT : 0);

		pSessionID[0] = pSecondDlg->IsUser() ? pSecondDlg->GetActor()->GetUniqueID() : 0;
		pSessionID[1] = pFirstDlg->IsUser() ? pFirstDlg->GetActor()->GetUniqueID() : 0;
		
		if( 0 == pSessionID[0] && 0 == pSessionID[1] )
			return;

		SendSwapMemberIndex( 2, pIndex, pSessionID );
	}
}

bool CDnGuildWarSituationMng::IsPartyMember( DnActorHandle hActor )
{
	if( !hActor )
		return false;

	for( int itr = 0; itr < MAX_PARTY_USER_COUNT; ++itr )
	{
		if( m_pPartyUserGaugeDlgs[itr]->IsUser() && m_pPartyUserGaugeDlgs[itr]->GetActor() )
		{
			if( hActor->GetUniqueID() == m_pPartyUserGaugeDlgs[itr]->GetActor()->GetUniqueID() )
				return true;
		}
	}

	return false;
}
