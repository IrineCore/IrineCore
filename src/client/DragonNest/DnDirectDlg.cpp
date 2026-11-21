#include "StdAfx.h"
#include "DnDirectDlg.h"
#include "DnInterfaceString.h"
#include "DnPartyTask.h"
#include "DnTradeTask.h"
#include "DnVillageTask.h"
#include "DnFriendTask.h"
#include "DnGuildTask.h"
#include "DnInterface.h"
#include "TaskManager.h"
//blondymarry start
#include "GlobalValue.h"
//blondymarry end
#include "DnInterfaceDlgID.h"
#include "GameOptionSendPacket.h"
#include "DnPlayerActor.h"
#include "DnLocalPlayerActor.h"
#include "DnWorld.h"
#include "DnChatRoomTask.h"
#include "DnMasterTask.h"
#include "SyncTimer.h"
#include "DnGameTask.h"
#include "DnMainMenuDlg.h"
#include "DnTableDB.h"
#ifdef PRE_ADD_QUICK_PVP
#include "PvPSendPacket.h"
#ifdef PRE_FIX_QUICKPVP_EXCLUSIVEREQ
#include "DnCommonTask.h"
#include "DnLifeSkillCookingTask.h"
#endif // PRE_FIX_QUICKPVP_EXCLUSIVEREQ
#endif // PRE_ADD_QUICK_PVP

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
#include "DnPartyListDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnDirectDlg::CDnDirectDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
	, m_pButtonParty(NULL)
	, m_pButtonFriend(NULL)
	, m_pButtonGuild(NULL)
	, m_pButtonTrade(NULL)
	, m_pButtonDuel(NULL)
	, m_pButtonInfo(NULL)
	, m_pButtonFollow(NULL)
	, m_pButtonChat(NULL)
	, m_pButtonPupil(NULL)
	, m_pButtonMaster(NULL)
	, m_pStaticUserLevel(NULL)
	, m_pStaticId(NULL)
	, m_pStaticJob(NULL)
	, m_dwSessionID(0)
	, m_bRequestFriend( false )
	, m_bReceivePermission( false )
	, m_nLevel(0)
	, m_PartyID(0)
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	, m_pPartyEnterDlg( NULL )
	, m_pRaidEnterDlg( NULL )
	, m_pPartyEnterPasswordDlg( NULL )
	, m_pRaidEnterPasswordDlg( NULL )
	, m_bDirectReqParty( false )
	, m_bSecret( false )
	, m_Raid( false )
#endif
{
	memset(m_cCommunityOption, 0, sizeof(m_cCommunityOption) );
}

CDnDirectDlg::~CDnDirectDlg(void)
{
	
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	SAFE_DELETE(m_pPartyEnterDlg);
	SAFE_DELETE(m_pPartyEnterPasswordDlg);
	SAFE_DELETE(m_pRaidEnterDlg);
	SAFE_DELETE(m_pRaidEnterPasswordDlg)
#endif
}

void CDnDirectDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DirectDlg.ui" ).c_str(), bShow );
}

void CDnDirectDlg::InitialUpdate()
{
	m_pButtonParty = GetControl<CEtUIButton>("ID_BUTTON_INVITE");
	m_pButtonFriend = GetControl<CEtUIButton>("ID_BUTTON_FRIEND");
	m_pButtonGuild = GetControl<CEtUIButton>("ID_BUTTON_GUILD");
	m_pButtonTrade = GetControl<CEtUIButton>("ID_BUTTON_TRADE");
	m_pButtonDuel = GetControl<CEtUIButton>("ID_BUTTON_DUEL");
	m_pButtonInfo = GetControl<CEtUIButton>("ID_BUTTON_INFO");
	m_pButtonFollow = GetControl<CEtUIButton>("ID_BUTTON_FOLLOW");
	m_pButtonChat = GetControl<CEtUIButton>("ID_BUTTON_CHAT");
	m_pButtonPupil = GetControl<CEtUIButton>("ID_BUTTON_JOINPUPIL");
	m_pButtonMaster = GetControl<CEtUIButton>("ID_BUTTON_JOINMASTER");

	m_pButtonParty->Enable(false);
	m_pButtonGuild->Enable(false);

	m_pStaticUserLevel = GetControl<CEtUIStatic>("ID_STATIC_USERLEVEL");
	//m_pStaticWorldLevel = GetControl<CEtUIStatic>("ID_STATIC_WORLDLEVEL");
	m_pStaticId = GetControl<CEtUIStatic>("ID_STATIC_ID");
	m_pStaticJob = GetControl<CEtUIStatic>("ID_STATIC_JOB");

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	m_pPartyEnterDlg = new CDnPartyEnterDlg(UI_TYPE_FOCUS, NULL, PARTYJOIN_DIALOG, this, true);
	m_pPartyEnterDlg->Initialize( false, _NORMAL_PARTY );

	m_pPartyEnterPasswordDlg = new CDnPartyEnterPasswordDlg(UI_TYPE_FOCUS, NULL, PARTYJOINPASS_DIALOG, this, true);
	m_pPartyEnterPasswordDlg->Initialize( false, _NORMAL_PARTY );

	m_pRaidEnterDlg = new CDnPartyEnterDlg(UI_TYPE_FOCUS, NULL, PARTYJOIN_DIALOG, this, true);
	m_pRaidEnterDlg->Initialize(false, _RAID_PARTY_8);

	m_pRaidEnterPasswordDlg = new CDnPartyEnterPasswordDlg(UI_TYPE_FOCUS, NULL, PARTYJOINPASS_DIALOG, this, true);
	m_pRaidEnterPasswordDlg->Initialize(false, _RAID_PARTY_8);
#endif
}

void CDnDirectDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_bReceivePermission = false;

		m_pButtonParty->Enable(false);
		m_pButtonFriend->Enable(false);
		m_pButtonGuild->Enable(false);
		m_pButtonDuel->Enable(false);
		m_pButtonInfo->Enable(false);
		m_pButtonFollow->Enable(false);

		// If the window is opened right after connecting to the server, the player may be in a guild without having received its info yet, so handle it this way.
		if( CDnActor::s_hLocalActor ) {
			CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if( pPlayer->IsJoinGuild() ) {
				TGuild *pGuild = GetGuildTask().GetGuildInfo();
				if( !pGuild->IsSet() )
					GetGuildTask().RequestGetGuildInfo( true );
			}
		}
		// This is too heavy to do in Process, so check it on Show. Assume m_dwSessionID is valid.
		m_pButtonChat->Enable( GetChatRoomTask().IsEnableChatRoomEnter( m_dwSessionID ) );

		if( !CDnActor::s_hLocalActor ) return;
		CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));

		if( localActor->GetLevel() > static_cast<int>(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MasterSystem_PupilMaxLevel)) && GetMasterTask().GetSimpleInfo().iMasterCount == 0 )
		{
			m_pButtonPupil->Show(false);

			const TMasterSystemSimpleInfo & SimpleInfo = GetMasterTask().GetSimpleInfo();

			INT64 timeDelta = CSyncTimer::GetInstance().GetCurTime() - SimpleInfo.BlockDate;

			int levelDelta = localActor->GetLevel() - m_nLevel;

			if( m_nLevel > static_cast<int>(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MasterSystem_PupilMaxLevel)) || 
				GetMasterTask().IsMasterPlayer( m_pStaticId->GetText() ) != 0 || 
				timeDelta < 0 ||

				levelDelta < static_cast<int>(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MasterSystem_MasterAndPupilAllowLevelGap)) ||
				SimpleInfo.iPupilCount == MasterSystem::Max::PupilCount )
				m_pButtonMaster->Enable(false);
			else
				m_pButtonMaster->Enable(true);

			m_pButtonMaster->Show(true);
		}
		else
		{
			m_pButtonMaster->Show(false);

			const TMasterSystemSimpleInfo & SimpleInfo = GetMasterTask().GetSimpleInfo();

			INT64 timeDelta = CSyncTimer::GetInstance().GetCurTime() - SimpleInfo.BlockDate;

			int levelDelta = m_nLevel - localActor->GetLevel();

			if( m_nLevel < static_cast<int>(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MasterSystem_MasterMinLevel)) || 
				GetMasterTask().IsMasterPlayer( m_pStaticId->GetText() ) != 0 || 
				timeDelta < 0 ||

				levelDelta < static_cast<int>(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MasterSystem_MasterAndPupilAllowLevelGap)) ||
				SimpleInfo.iMasterCount == MasterSystem::Max::MasterCount )
				m_pButtonPupil->Enable(false);
			else
				m_pButtonPupil->Enable(true);

			m_pButtonPupil->Show(true);
		}

		if( CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeVillage )
		{
			m_pButtonPupil->Enable(false);
			m_pButtonMaster->Enable(false);
			m_pButtonDuel->Enable(false);
		}
	}
	else
	{
		m_pButtonParty->Enable(false);
		m_pButtonFriend->Enable(false);
		m_pButtonGuild->Enable(false);
		m_pButtonDuel->Enable(false);
		m_pButtonInfo->Enable(false);
		m_pButtonFollow->Enable(false);
		m_dwSessionID = 0;

	}
	CEtUIDialog::Show( bShow );
//	CDnMouseCursor::GetInstance().ShowCursor( bShow );
}

bool CDnDirectDlg::CheckValidActor()
{
	bool bValid = true;
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( m_dwSessionID );
	if( !hActor || hActor->IsDestroy() ) {
		bValid = false;
	}
	else {
		const float fDistanceLimit = 500.0f;
		if( CDnActor::GetLocalActor() && EtVec3Length( &(*hActor->GetPosition() - *CDnActor::GetLocalActor()->GetPosition() )) > fDistanceLimit  ) {
			bValid = false;
		}
	}
	return bValid;
}

bool CDnDirectDlg::CheckFriendCondition(const WCHAR* name)
{
	CDnFriendTask* pFriendTask = static_cast<CDnFriendTask*>(CTaskManager::GetInstance().GetTask( "FriendTask" ));
	if( !pFriendTask )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1420 ), MB_OK );	// UISTRING : Friend feature is unavailable.
		return false;
	}

	if (pFriendTask->IsFriend(name))
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1421 ), MB_OK );	// UISTRING : Target is already registered.
		return false;
	}
	return true;
}

bool CDnDirectDlg::IsFriend( const WCHAR *name )
{
	CDnFriendTask* pFriendTask = static_cast<CDnFriendTask*>(CTaskManager::GetInstance().GetTask( "FriendTask" ));
	bool bFriend = ( pFriendTask != NULL ) && pFriendTask->IsFriend(name);
	return bFriend;
}

void CDnDirectDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( m_bRequestFriend ) {
		if( CheckFriendCondition(m_pStaticId->GetText()) ) {
			GetFriendTask().RequestFriendAdd( 0, m_pStaticId->GetText() );
		}
		m_bRequestFriend = false;
	}

	if( IsShow() )
	{
		if( !CheckValidActor() ) {
			Show( false );
			return;
		}
		if( !CDnMouseCursor::GetInstance().IsShowCursor() ) {
			CDnMouseCursor::GetInstance().ShowCursor( true, true );
		}

		if( GetInterface().IsOpenAcceptDialog() )
		{
			m_pButtonTrade->Enable( false );
			m_pButtonParty->Enable( false );
			m_pButtonFriend->Enable( false );
			m_pButtonGuild->Enable( false );
			m_pButtonDuel->Enable( false );
		}
		else
		{
			CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstance().GetTask( "PartyTask" ));
			if( !pPartyTask ) return;

			bool bEnableParty(true);

			if( CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeVillage )
			{
				bEnableParty = false;
			}
			if( pPartyTask->IsPartyMember( m_dwSessionID ) )
			{
				bEnableParty = false;
			}
			if( pPartyTask->GetPartyRole() == CDnPartyTask::LEADER )
			{
				if( (DWORD)pPartyTask->GetMaxPartyCount() <= pPartyTask->GetPartyCount() )
				{
					bEnableParty = false;
				}
			}
			if( pPartyTask->GetPartyRole() == CDnPartyTask::MEMBER )
			{
				bEnableParty = false;
			}
			CDnVillageTask *pVillageTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
			if( pVillageTask && pVillageTask->GetVillageType() == CDnVillageTask::PvPVillage ||
				 pVillageTask && pVillageTask->GetVillageType() == CDnVillageTask::FarmVillage ) 
			{
				bEnableParty = false;
			}

			bool bEnableTrade = true;
			// If self is requesting a trade with someone, or someone is requesting a trade with self,
			if( GetTradeTask().GetTradePrivateMarket().IsRequesting() || GetTradeTask().GetTradePrivateMarket().IsAccepting() )
				bEnableTrade = false;
#ifdef PRE_WORLDCOMBINE_PARTY
			if (pPartyTask->GetPartyType() == _WORLDCOMBINE_PARTY)
				bEnableTrade = false;
#endif

			//Temp Enable
			/*m_pButtonParty->Enable( bEnableParty );
			m_pButtonTrade->Enable( bEnableTrade );
			m_pButtonDuel->Enable( true );
			m_pButtonInfo->Enable( true );
			m_pButtonFriend->Enable( true );*/
			// Temporary enable
			/**/

			m_pButtonFriend->Enable( true );		// Adding a friend is one-sided, so it is always possible
			m_pButtonFollow->Enable( true );		// Follow is also always possible.

			if( m_bReceivePermission ) {

				/*m_pButtonFriend->Enable( m_CommOption[CGameOption::CommOpt_Friend] != CGameOption::CommVal_NoAllow );*/
				m_pButtonGuild->Enable( m_cGuildInviteAcceptable != _CA_OPTIONTYPE_DENYALL );

				bool bFriend = IsFriend( m_pStaticId->GetText() );
				if( m_cPartyInviteAcceptable == _CA_OPTIONTYPE_DENYALL ) {
					bEnableParty = false;
				}
				if( m_cTradeRequestAcceptable == _CA_OPTIONTYPE_DENYALL ) {
					bEnableTrade = false;
				}
				bool bEnableDuel = true, bEnableInfo = true;
				if( m_cDuelRequestAcceptable == _CA_OPTIONTYPE_DENYALL ) {
					bEnableDuel = false;
				}
#ifdef PRE_MOD_ALLOW_INSPECT
#else
				if( m_cObserveStuffAcceptable == _CA_OPTIONTYPE_DENYALL ) {
					bEnableInfo = false;
				}
#endif	// #ifdef PRE_MOD_ALLOW_INSPECT

				bool bGuild = false;	// TODO
				if( !bFriend && !bGuild ) {
					if( m_cPartyInviteAcceptable == _CA_OPTIONTYPE_FRIEND_GUILD_ACCEPT ) {
						bEnableParty = false;
					}
					if( m_cTradeRequestAcceptable == _CA_OPTIONTYPE_FRIEND_GUILD_ACCEPT ) {
						bEnableTrade = false;
					}
					if( m_cDuelRequestAcceptable == _CA_OPTIONTYPE_FRIEND_GUILD_ACCEPT ) {
						bEnableDuel = false;
					}
#ifdef PRE_MOD_ALLOW_INSPECT
#else
					if( m_cObserveStuffAcceptable == _CA_OPTIONTYPE_FRIEND_GUILD_ACCEPT ) {
						bEnableInfo = false;
					}
#endif	// #ifdef PRE_MOD_ALLOW_INSPECT
				}				

#ifdef PRE_FIX_QUICKPVP_EXCLUSIVEREQ
				CDnCommonTask *pCommonTask = static_cast<CDnCommonTask *>(CTaskManager::GetInstance().GetTask( "CommonTask" ));
				if (pCommonTask)
				{
					if (pCommonTask->IsRequestingQuickPvP() || GetInterface().IsOpenAcceptRequestDialog())
					{
						bEnableDuel = false;
						bEnableParty = false;
						bEnableTrade = false;
					}
				}

				if (bEnableDuel)
				{
					if (CDnLifeSkillCookingTask::IsActive() && GetLifeSkillCookingTask().IsNowCooking())
						bEnableDuel = false;
				}
#endif

				m_pButtonParty->Enable( bEnableParty );
				m_pButtonTrade->Enable( bEnableTrade );
				m_pButtonDuel->Enable( bEnableDuel );

				m_pButtonInfo->Enable( bEnableInfo );				
			}

			// Even if the guild button is enabled, recheck if invite is possible based on guild info.
			// Do not search for targets nearby every frame due to high computation cost. Just check with own authority.
			if( m_pButtonGuild->IsEnable() ) {
				if( GetGuildTask().GetGuildInfo()->IsSet() &&
					GetGuildTask().GetAuth( static_cast<eGuildRoleType>(GetGuildTask().GetMyRole()), GUILDAUTH_TYPE_INVITE ) &&
					CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage ) {
						m_pButtonGuild->Enable(true);
				}
				else {
					m_pButtonGuild->Enable(false);
				}
			}

			if (m_pButtonDuel->IsEnable() == true && CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeVillage)
				m_pButtonDuel->Enable(false);

#ifdef PRE_ADD_QUICK_PVP
#else
			m_pButtonDuel->Enable( false );		// Disabled in CBT2
#endif
		}
	}
}

void CDnDirectDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	if (m_pPartyEnterDlg->IsShow() || m_pPartyEnterPasswordDlg->IsShow() ||
		m_pRaidEnterDlg->IsShow() || m_pRaidEnterPasswordDlg->IsShow())
	{
		return;
	}
#endif

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_INFO")) 
		{
			GetInterface().OpenInspectPlayerInfoDialog( m_dwSessionID );	
		}
		else if( IsCmdControl("ID_BUTTON_INVITE") )
		{
			if (IsAskingParty())
			{
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
				GetPartyTask().ReqPartyInfoWithPlayerID( m_PartyID );
				SetDirectPlayerAskParty(true);
#else
				CDnPartyTask::GetInstance().ReqAskParty(m_pStaticId->GetText());
#endif
			}
			else
				CDnPartyTask::GetInstance().ReqInviteParty(m_pStaticId->GetText());
		}
		else if( IsCmdControl("ID_BUTTON_CLOSE" ) )
		{
		}
		else if( IsCmdControl("ID_BUTTON_TRADE" ) )
		{
			GetInterface().GetMainMenuDialog()->CloseInvenDialog();
			GetTradeTask().GetTradePrivateMarket().RequestPrivateMarket( m_dwSessionID );
		}
		else if( IsCmdControl("ID_BUTTON_FRIEND") )
		{
			wchar_t wszQuery[255];
			swprintf_s(wszQuery, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1418 ), m_pStaticId->GetText() );	// UISTRING : Add %s to your friend list?

			GetInterface().MessageBox( wszQuery, MB_YESNO, MESSAGEBOX_FRIEND_QUERY, this);
		}
		else if( IsCmdControl("ID_BUTTON_GUILD") )
		{
			GetGuildTask().RequestInviteGuildMember( m_pStaticId->GetText() );
		}
		else if( IsCmdControl("ID_BUTTON_COPYNAME") )
		{
			CEtUILineEditBox::CopyStringToClipboard( const_cast<wchar_t*>(m_pStaticId->GetText()) );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1422 ), false );
		}
		else if( IsCmdControl("ID_BUTTON_FOLLOW") )
		{
			if( CDnActor::s_hLocalActor  ) 
			{
				CDnLocalPlayerActor* pLocal = (CDnLocalPlayerActor*)(CDnActor::s_hLocalActor.GetPointer());
				_ASSERT( pLocal );
				DnActorHandle hActor = CDnActor::FindActorFromUniqueID( m_dwSessionID );
				if( hActor ) {
					pLocal->FollowActor( hActor );
				}
			}
		}
		else if( IsCmdControl("ID_BUTTON_CHAT") )
		{
			if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage )
			{
				if( GetPartyTask().GetPartyRole() != CDnPartyTask::SINGLE )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8127 ) );
					return;
				}
			}
			else
			{
				CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
				bool bFarmGameTask = false;
				if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm )
					bFarmGameTask = true;

				if( !bFarmGameTask )
					return;
			}
			GetChatRoomTask().RequestEnterChatRoom( m_dwSessionID );	// Room number, password
		}
		else if( IsCmdControl("ID_BUTTON_JOINPUPIL") )
		{
			wchar_t wszString[255];
			swprintf_s(wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7239 ), m_pStaticId->GetText() );	// UISTRING : Do you accept Master %S?

			GetInterface().MessageBox( wszString, MB_YESNO, MESSAGEBOX_PUPIL_APPLICATION, this);
		}			
		else if( IsCmdControl("ID_BUTTON_JOINMASTER") )
		{
			wchar_t wszString[255];
			swprintf_s(wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7238 ), m_pStaticId->GetText() );	// UISTRING : Accept %S as your apprentice?

			GetInterface().MessageBox( wszString, MB_YESNO, MESSAGEBOX_MASTER_APPLICATION, this);
		}
#ifdef PRE_ADD_QUICK_PVP
		else if (IsCmdControl("ID_BUTTON_DUEL"))
		{
			std::wstring pvpStr;
			if (CheckValidActor())
			{
				SendQuickPvPInvite(m_dwSessionID);

#ifdef PRE_FIX_QUICKPVP_EXCLUSIVEREQ
				CDnCommonTask *pCommonTask = static_cast<CDnCommonTask *>(CTaskManager::GetInstance().GetTask( "CommonTask" ));
				if (pCommonTask)
					pCommonTask->SetRequestingQuickPvP(true);
#endif

				pvpStr = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120149 ), m_pStaticId->GetText()); // UISTRING : Requested a duel with %s.
			}
			else
			{
				pvpStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120150 ); // UISTRING : Unable to verify user. Please try again.
			}
			
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", pvpStr.c_str(), false);
		}
#endif

		Show(false);
		return;
	}
}

bool CDnDirectDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	if( !IsShow() )
	{
		return false;
	}

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	if (m_pPartyEnterDlg->IsShow() || m_pPartyEnterPasswordDlg->IsShow() ||
		m_pRaidEnterDlg->IsShow() || m_pRaidEnterPasswordDlg->IsShow())
	{
		return false;
	}
#endif

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
		{
			if( !IsMouseInDlg() )
			{
				Show( false );
				return true;
			}
		}
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnDirectDlg::SetInfo( DWORD dwSessionID, int nUserLevel, const wchar_t *wszJobName, const wchar_t *wszName )
{
	m_dwSessionID = dwSessionID;
	m_nLevel = nUserLevel;

	wchar_t wszTemp[256]={0};

	swprintf_s( wszTemp, 256, L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 64 ), nUserLevel );
	m_pStaticUserLevel->SetText( wszTemp );

	m_pStaticId->SetText( wszName );
	m_pStaticJob->SetText( wszJobName );

	m_bReceivePermission = false;
	SendCharCommOption( dwSessionID );
}

#ifdef PRE_PARTY_DB
void CDnDirectDlg::SetPermission( UINT nSessionID, char *pCommunityOption, TPARTYID PartyID)
#else
void CDnDirectDlg::SetPermission( UINT nSessionID, char *pCommunityOption, UINT PartyID)
#endif
{
	if( m_dwSessionID != nSessionID ) {
		return;
	}

	memcpy(m_cCommunityOption, pCommunityOption, sizeof( m_cCommunityOption) );
	m_bReceivePermission = true;
	m_PartyID = PartyID;
	int nPartyInviteBtnString = (IsAskingParty()) ? 3511 : 1401; // UISTRING : Party Application / Party Invite

	m_pButtonParty->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nPartyInviteBtnString));
}

void CDnDirectDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if (IsCmdControl("ID_YES"))
		{
			switch( nID ) 
			{
			case MESSAGEBOX_FRIEND_QUERY:
				m_bRequestFriend = true;
				break;
			case MESSAGEBOX_MASTER_APPLICATION :
				GetMasterTask().RequestInvitePupil( m_pStaticId->GetText() );
				break;
			case MESSAGEBOX_PUPIL_APPLICATION :
				GetMasterTask().RequestJoinDirect( m_pStaticId->GetText() );
				break;
			}
		}
	}
	
}

bool CDnDirectDlg::IsAskingParty() const
{
	return (CDnPartyTask::GetInstance().IsLocalActorMaster() == false && m_PartyID > 0);
}

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
void CDnDirectDlg::OnRecvPartyInfo( const SPartyListInfo& partyInfo )
{
	std::wstring PartyEnterDlgTitleString;
	std::wstring TargetStageString;
	DN_INTERFACE::STRING::GetMapName(TargetStageString, partyInfo.nTargetMapIdx);
	if ( ! TargetStageString.empty() )
	{
		std::wstring difficultyStr;
#ifdef PRE_PARTY_DB
		if (partyInfo.TargetDifficulty < Dungeon::Difficulty::Max)
#else
		if (partyInfo.TargetDifficulty > 0)
#endif
		{
			DN_INTERFACE::STRING::GetStageDifficultyText(difficultyStr,  partyInfo.TargetDifficulty);
		}
		PartyEnterDlgTitleString = difficultyStr.empty() ? TargetStageString.c_str() : FormatW(L"%s(%s)", TargetStageString.c_str(), difficultyStr.c_str() );
	}
	else
		PartyEnterDlgTitleString = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 93); // UISTRING : None
	
	m_bSecret = ( partyInfo.cIsPrivateRoom > 0 ) ? true : false;
	m_PartyType = partyInfo.PartyType;

#ifdef PRE_WORLDCOMBINE_PARTY
	if ( partyInfo.PartyType == _RAID_PARTY_8)
	{
		SetPartyEnterDlgByRaidOrNormal(true, partyInfo, PartyEnterDlgTitleString);
	}
	else if (partyInfo.PartyType == _WORLDCOMBINE_PARTY)
	{
		if (CDnPartyTask::IsActive() == false)
			return;
		
		SWorldCombinePartyData data;
		CDnPartyTask::GetInstance().GetWorldCombinePartyTableData(partyInfo.nWorldCombinePartyTableIndex, data);
		if (data.IsEmpty() == false)
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP 
		SetPartyEnterDlgByRaidOrNormal( (data.maxUser > NORMPARTYCOUNTMAX), partyInfo, PartyEnterDlgTitleString, partyInfo.PartyType );
#else
		SetPartyEnterDlgByRaidOrNormal((data.maxUser > NORMPARTYCOUNTMAX), partyInfo, PartyEnterDlgTitleString);
#endif
	}
	else
	{
		SetPartyEnterDlgByRaidOrNormal(false, partyInfo, PartyEnterDlgTitleString);
	}
#else
	if (partyInfo.PartyType == _RAID_PARTY_8)
	{
		if ( m_bSecret )
		{
			m_pRaidEnterPasswordDlg->SetPartyInfo(partyInfo.PartyID, partyInfo.wszBuf, PartyEnterDlgTitleString);
		}
		else
		{
			m_pRaidEnterDlg->SetPartyInfo(partyInfo.PartyID, partyInfo.wszBuf, PartyEnterDlgTitleString);
		}
	}
	else
	{
		if ( m_bSecret )
		{
			m_pPartyEnterPasswordDlg->SetPartyInfo(partyInfo.PartyID, partyInfo.wszBuf, PartyEnterDlgTitleString);
		}
		else
		{
			m_pPartyEnterDlg->SetPartyInfo(partyInfo.PartyID, partyInfo.wszBuf, PartyEnterDlgTitleString);
		}
	}
#endif
}

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
void CDnDirectDlg::EnablePartyDlg( bool bShow )
{
#ifdef PRE_WORLDCOMBINE_PARTY
	if ( m_PartyType == _RAID_PARTY_8)
	{
		m_bSecret ?	m_pRaidEnterPasswordDlg->Show(bShow) : m_pRaidEnterDlg->Show(bShow);
	}
	else if ( m_PartyType == _WORLDCOMBINE_PARTY)
	{
		if (m_Raid)
		{
			m_bSecret ? m_pRaidEnterPasswordDlg->Show(bShow) : m_pRaidEnterDlg->Show(bShow);
		}
		else
		{
			m_bSecret ? m_pPartyEnterPasswordDlg->Show(bShow) : m_pPartyEnterDlg->Show(bShow);
		}	
	}
	else
	{
		m_bSecret ? m_pPartyEnterPasswordDlg->Show(bShow) : m_pPartyEnterDlg->Show(bShow);
	}
#else
	if ( m_PartyType == _RAID_PARTY_8)
	{
		m_bSecret ?	m_pRaidEnterPasswordDlg->Show(bShow) : m_pRaidEnterDlg->Show(bShow);
	}
	else
	{
		m_bSecret ? m_pPartyEnterPasswordDlg->Show(bShow) : m_pPartyEnterDlg->Show(bShow);
	}
#endif
}
#endif 

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
void CDnDirectDlg::SetPartyEnterDlgByRaidOrNormal(bool bRaid, const SPartyListInfo& partyInfoList, const std::wstring& title, ePartyType type)
{
	bool bSecret = ( partyInfoList.cIsPrivateRoom > 0 ) ? true : false;
	std::wstring PartyName = partyInfoList.wszBuf;

	m_Raid = bRaid;
	

	if (bRaid)
	{
		if ( bSecret )
		{
			m_pRaidEnterPasswordDlg->SetPartyInfo(partyInfoList.PartyID, PartyName.c_str(), title);
			m_pRaidEnterPasswordDlg->SetPartyType(type);
		}
		else
		{
			m_pRaidEnterDlg->SetPartyInfo(partyInfoList.PartyID, PartyName.c_str(), title);
			m_pRaidEnterDlg->SetPartyType(type);
		}
	}
	else
	{
		if ( bSecret )
		{
			m_pPartyEnterPasswordDlg->SetPartyInfo(partyInfoList.PartyID, PartyName.c_str(), title);
			m_pPartyEnterPasswordDlg->SetPartyType(type);
		}
		else
		{
			m_pPartyEnterDlg->SetPartyInfo(partyInfoList.PartyID,PartyName.c_str(), title);
			m_pPartyEnterDlg->SetPartyType(type);
		}
	}
}
#endif

#ifdef PRE_WORLDCOMBINE_PARTY
void CDnDirectDlg::SetPartyEnterDlgByRaidOrNormal(bool bRaid, const SPartyListInfo& partyInfoList, const std::wstring& title)
{
	bool bSecret = ( partyInfoList.cIsPrivateRoom > 0 ) ? true : false;
	std::wstring PartyName = partyInfoList.wszBuf;

	m_Raid = bRaid;

	if (bRaid)
	{
		if ( bSecret )
		{
			m_pRaidEnterPasswordDlg->SetPartyInfo(partyInfoList.PartyID, PartyName.c_str(), title);
		}
		else
		{
			m_pRaidEnterDlg->SetPartyInfo(partyInfoList.PartyID, PartyName.c_str(), title);
		}
	}
	else
	{
		if ( bSecret )
		{
			m_pPartyEnterPasswordDlg->SetPartyInfo(partyInfoList.PartyID, PartyName.c_str(), title);
		}
		else
		{
			m_pPartyEnterDlg->SetPartyInfo(partyInfoList.PartyID,PartyName.c_str(), title);
		}
	}
}
#endif

void CDnDirectDlg::OnPartyJoinFailed()
{
	if (m_pPartyEnterPasswordDlg && m_pPartyEnterPasswordDlg->IsShow())
		m_pPartyEnterPasswordDlg->OnPartyJoinFailed();
	if (m_pRaidEnterPasswordDlg && m_pRaidEnterPasswordDlg->IsShow())
		m_pRaidEnterPasswordDlg->OnPartyJoinFailed();
}

#endif // PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
