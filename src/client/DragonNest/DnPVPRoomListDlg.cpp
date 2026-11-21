#include "StdAfx.h"
#include "DnPVPRoomListDlg.h"
#include "DnInterface.h"
#include "EtUIDialog.h"
#include "SystemSendPacket.h"
#include "DnPVPLobbyVillageTask.h"
#include "TaskManager.h"
#include "PVPSendPacket.h"
#include "DnTableDB.h"
#include "DnChatTabDlg.h"
#include "DnFriendTask.h"
#include "DnGuildTask.h"
#include "GuildSendPacket.h"
#include "DnLocalPlayerActor.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#include "DnMainDlg.h"
#include "DnInterfaceString.h"

#ifdef PRE_ADD_PVP_HELP_MESSAGE
#include "DnSimpleTooltipDlg.h"
#endif

#ifdef PRE_PVP_GAMBLEROOM
#include "DnItemTask.h"
#include "DnMessageBox.h"
#endif // PRE_PVP_GAMBLEROOM
#if defined( PRE_ADD_DWC )
#include "DnDWCTask.h"
#include "DnPVPDWCTabDlg.h"
#include "DnDWCMarkInformer.h"
#endif


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnPVPRoomListDlg::CDnPVPRoomListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback , bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback , bAutoCursor )
,m_pPVPMakeRoomDlg(NULL)
,m_pReFreshButton (NULL)
,m_pQuickEnterButton(NULL)
,m_pPassWordInputDlg(NULL)
#ifdef PRE_MOD_PVPOBSERVER
,m_pPassWordInputEventDlg(NULL)
,m_bShowPasswordDlg(false)
#endif // PRE_MOD_PVPOBSERVER
,m_pSelector (NULL)
,m_pRoomNumberSort(NULL)
,m_pMaxPlayerSort(NULL)
,m_pRoomNumberSortAc(NULL)
,m_pRoomNumberSortDc(NULL)
,m_pMaxPlayerSortAc(NULL)
,m_pMaxPlayerSortDc(NULL)
,m_pMapCombo(NULL)
,m_pModeCombo(NULL)
,m_pPrePage(NULL)
,m_pNextPage(NULL)
,m_pPageIndex(NULL)
,m_pJoin(NULL)
,m_pObserverJoin(NULL)
,m_bObserverJoin(false)
,m_pUserSelect(NULL) 
,m_pUserPageIndex(NULL) 
,m_pButtonRefreshUser(NULL) 
,m_pButtonUserPrePage(NULL)
,m_pButtonUserNextPage(NULL)
,m_pRButtonLobby(NULL)
,m_pRButtonFriend(NULL)
,m_pRButtonGuild(NULL)
,m_pMapName(NULL)
,m_pModeName(NULL)
,m_pLadderInviteConfirmDlg(NULL)
,m_pPVPPopupDlg(NULL)
#ifdef PRE_ADD_PVP_HELP_MESSAGE
,m_pLobbyImagePopUpDlg(NULL)
#endif
{    
	Reset();

	m_isFirstRoomListPacket = false;
	vWaitUserSlot.reserve(PvPCommon::WaitUserList::Common::MaxPerPage);     // Maximum number of slots is 12.
	m_nUserType = eWaitUser::TypeLobby;
	m_nUserPageIndex = 0;
	m_nRefreshUserCount = 0;      // Calculate the number of users to SetRefresh.
	m_fRequestGuildTimeLimit = 0;      
	m_fRequestFriendTimeLimit = 0;

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	m_bVillageAccessMode = false;
#endif
}

CDnPVPRoomListDlg::~CDnPVPRoomListDlg(void)
{
	SAFE_DELETE( m_pPVPMakeRoomDlg );
#ifdef PRE_MOD_PVPOBSERVER
	SAFE_DELETE(m_pPassWordInputDlg );
	SAFE_DELETE(m_pPassWordInputEventDlg);
#else
	SAFE_DELETE(m_pPassWordInputDlg );
#endif // PRE_MOD_PVPOBSERVER	

	SAFE_DELETE(m_pPVPPopupDlg);
	SAFE_DELETE(m_pLadderInviteConfirmDlg);
#ifdef PRE_ADD_PVP_HELP_MESSAGE
	SAFE_DELETE(m_pLobbyImagePopUpDlg);
#endif
}

void CDnPVPRoomListDlg::Reset()
{
	m_nSelectedRoomIndex = -1;
	m_fRefreshTime = 0.0f;	
	m_RoomInfoList.clear();
	m_uiBackUpRoomIndex = 0;

	if( m_pSelector )
		m_pSelector->Show(false);

	m_uiCurPage = 0;
	m_emRoomNumAscend = SortTypeNone;
	m_emMaxPlayerAscend = SortTypeNone;
	m_cRoomSortType = PvPCommon::RoomSortType::Index;
	m_iDataIndex = 0;
	m_uiMaxPage = 0;
	m_nUserMaxPage = MaxPage;

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	m_eSelectedPVPGradeChannel = PvPCommon::RoomType::eRoomType::max;

#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
}

void CDnPVPRoomListDlg::Initialize( bool bShow )
{
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	if(m_bVillageAccessMode)
		CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpVillageBase.ui" ).c_str(), bShow );
	else
		CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpBase.ui" ).c_str(), bShow );
#else
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpBase.ui" ).c_str(), bShow );
#endif
}

void CDnPVPRoomListDlg::InitialUpdate()
{
	m_pPVPMakeRoomDlg = new CDnPVPMakeRoomDlg( UI_TYPE_MODAL, NULL, ROOMMAKE_DIALOG, this );
	m_pPVPMakeRoomDlg->Initialize( false );

#ifdef PRE_MOD_PVPOBSERVER
	m_pPassWordInputDlg = new CDnPassWordInputDlg( UI_TYPE_MODAL, this, PASSWORDINPUT_DIALOG, this );
	m_pPassWordInputDlg->Initialize( false );

	m_pPassWordInputEventDlg = new CDnPassWordInputEventDlg( UI_TYPE_MODAL, this, PASSWORDINPUT_DIALOG, this );
	m_pPassWordInputEventDlg->Initialize( false );
#else
	m_pPassWordInputDlg = new CDnPassWordInputDlg( UI_TYPE_CHILD_MODAL, this, PASSWORDINPUT_DIALOG, this );
	m_pPassWordInputDlg->Initialize( false );
#endif // PRE_MOD_PVPOBSERVER

	m_pPVPPopupDlg = new CDnPVPPopupDlg(UI_TYPE_CHILD,this);
	m_pPVPPopupDlg->Initialize(false);
	m_pLadderInviteConfirmDlg = new CDnPVPLadderInviteConfirmDlg(UI_TYPE_MODAL,this);
	m_pLadderInviteConfirmDlg->Initialize(false);

	char szUIName[256] = "";

	for(int iSlotNum = 0 ; iSlotNum < PVPRoomList::MAX_SLOTNUM ; iSlotNum++)
	{
		sprintf(szUIName,"ID_PVP_STATE%d",iSlotNum);
		m_sSlot[iSlotNum].pStaticState = GetControl<CEtUIStatic>( szUIName );

		sprintf(szUIName,"ID_PVP_NUMBER%d",iSlotNum);
		m_sSlot[iSlotNum].pStaticRoomNum = GetControl<CEtUIStatic>( szUIName );

		sprintf(szUIName,"ID_PVP_LIMIT%d",iSlotNum);
		m_sSlot[iSlotNum].pRoomPlayerNum = GetControl<CEtUIStatic>( szUIName );

		sprintf(szUIName,"ID_PVP_ROOMNAME%d",iSlotNum);
		m_sSlot[iSlotNum].pRoomName = GetControl<CEtUIStatic>( szUIName );

		sprintf(szUIName,"ID_SELECT%d",iSlotNum);
		m_sSlot[iSlotNum].pRoomSelect = GetControl<CEtUIStatic>( szUIName );

		sprintf(szUIName,"ID_PVP_SECRET%d",iSlotNum);
		m_sSlot[iSlotNum].pUsePassWord = GetControl<CEtUIStatic>( szUIName );

		sprintf(szUIName,"ID_PVP_MODE%d",iSlotNum);
		m_sSlot[iSlotNum].pGameMode= GetControl<CEtUIStatic>( szUIName );

		sprintf(szUIName,"ID_PVP_MAPNAME%d",iSlotNum);	
		m_sSlot[iSlotNum].pInMapName =  GetControl<CEtUIStatic>( szUIName );
	
		sprintf(szUIName,"ID_PVP_LEVEL%d",iSlotNum);
		m_sSlot[iSlotNum].pPlayerMinMaxLevel = GetControl<CEtUIStatic>(szUIName);

#ifdef PRE_PVP_GAMBLEROOM
		sprintf(szUIName,"ID_STATIC_CASH%d",iSlotNum);
		m_sSlot[iSlotNum].pGamblePetal = GetControl<CEtUIStatic>( szUIName );

		sprintf(szUIName,"ID_STATIC_GOLD%d",iSlotNum);
		m_sSlot[iSlotNum].pGambleGold = GetControl<CEtUIStatic>( szUIName );
#endif // PRE_PVP_GAMBLEROOM
	}
	
	m_pObserverJoin = GetControl<CEtUIButton>( "ID_PVP_OBSERVERJOIN" );

	m_pRoomNumberSort = GetControl<CEtUIButton>( "ID_BUTTON_NUMBER" );
	m_pMaxPlayerSort = GetControl<CEtUIButton>( "ID_BUTTON_MEMBERNUM" );
	m_pPrePage = GetControl<CEtUIButton>( "ID_PARTY_PRIOR" );
	m_pNextPage = GetControl<CEtUIButton>( "ID_PARTY_NEXT" );
	m_pJoin = GetControl<CEtUIButton>( "ID_PVP_JOIN" );

	m_pReFreshButton = GetControl<CEtUIButton>( "ID_PVP_REFRESH" );
	m_pReFreshButton->SetDisableTime(fRefreshTime);
	m_pQuickEnterButton = GetControl<CEtUIButton>( "ID_PVP_QUICKSTART" );	
	m_pQuickEnterButton->SetDisableTime(fRefreshTime);

	m_pMapCombo = GetControl<CEtUIComboBox >( "ID_COMBOBOX_MAP" );
	m_pModeCombo = GetControl<CEtUIComboBox>( "ID_COMBOBOX_MODE" );
	
	m_pRoomNumberSortAc = GetControl<CEtUIStatic>( "ID_BUTTON_NUMBER_UP" );
	m_pRoomNumberSortDc= GetControl<CEtUIStatic>( "ID_BUTTON_NUMBER_DOWN" );
	m_pMaxPlayerSortAc= GetControl<CEtUIStatic>( "ID_BUTTON_MEMBERNUM_UP" );
	m_pMaxPlayerSortDc= GetControl<CEtUIStatic>( "ID_BUTTON_MEMBERNUM_DOWN" );
#if defined( PRE_ADD_DWC )
	{
		bool bExistDwcCheck = false;
		CEtUICheckBox *pCheckDwc = GetControl<CEtUICheckBox>( "ID_CHECKBOX_DWC", &bExistDwcCheck );
		if( bExistDwcCheck && pCheckDwc )
			pCheckDwc->SetChecked( false );
	}
#endif
	m_pPageIndex = GetControl<CEtUIStatic>( "ID_PARTY_PAGE" );
	m_pMapName = GetControl<CEtUIStatic>( "ID_STATIC4" );
	m_pModeName = GetControl<CEtUIStatic>( "ID_STATIC5" );
	m_wszDefaultMapName = m_pMapName->GetText();
	m_wszDefaultModeName = m_pModeName->GetText();

	m_pSelector = GetControl<CEtUIStatic>( "ID_SELECT_BAR" );
	m_pSelector->Show(false);

#ifdef PRE_ADD_PVP_HELP_MESSAGE
	m_pLobbyImagePopUpDlg = new CDnPVPLobbyMapImageDlg(UI_TYPE_CHILD, this);
	m_pLobbyImagePopUpDlg->Initialize(false);
#endif

	MakeMapList();
	MakeModeList();

	// Sort in ascending order by number on start
	ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pRoomNumberSort, 0 );

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	if( m_bVillageAccessMode )
	{
		m_pQuickEnterButton->Show(false);
	}
	else
	{
		InitializeWaitUserList();
	}
#else
	InitializeWaitUserList();
#endif
	m_nUserType = eWaitUser::TypeLobby;

	ClearRoomInfo();
}

void CDnPVPRoomListDlg::InitializeWaitUserList()
{
	char szTemp[256];
	for(int i=0;i<PvPCommon::WaitUserList::Common::MaxPerPage; i++)
	{
		sWaitUserSlot sWAitUserSlotTemp;
		vWaitUserSlot.push_back(sWAitUserSlotTemp);

		_snprintf_s(szTemp, sizeof(szTemp), "ID_JOB%d",i);
		vWaitUserSlot[i].pStaticJobIcon = GetControl<CDnJobIconStatic>( szTemp );
		_snprintf_s(szTemp, sizeof(szTemp), "ID_TEXT_LEVEL%d",i);
		vWaitUserSlot[i].pStaticLevel = GetControl<CEtUIStatic>( szTemp );
		_snprintf_s(szTemp, sizeof(szTemp), "ID_TEXTURE_RANK%d",i);
		vWaitUserSlot[i].pStaticRankIcon = GetControl<CEtUITextureControl>( szTemp );
		_snprintf_s(szTemp, sizeof(szTemp), "ID_TEXT_NANE%d",i);
		vWaitUserSlot[i].pStaticName = GetControl<CEtUIStatic>( szTemp );
		_snprintf_s(szTemp, sizeof(szTemp), "ID_ROBBY_SELECT%d",i);
		vWaitUserSlot[i].pStaticDummy = GetControl<CEtUIStatic>( szTemp );
	}

	m_pUserSelect = GetControl<CEtUIStatic>( "ID_SELECT" );
	m_pUserSelect->Show(false);
	m_pUserPageIndex = GetControl<CEtUIStatic>( "ID_PAGE" );
	m_pUserPageIndex->SetText(L"1/1");

	m_pButtonUserPrePage = GetControl<CEtUIButton>( "ID_PRIOR" );
	m_pButtonUserNextPage = GetControl<CEtUIButton>( "ID_NEXT" );
	m_pButtonUserPrePage->Enable(false); // Initialize to false and check when SetWaitUserList is called.
	m_pButtonUserNextPage->Enable(false);

	m_pButtonRefreshUser = GetControl<CEtUIButton>("ID_REFRESH");
	m_pButtonRefreshUser->SetDisableTime(2.f); // Prevent button mashing.

	m_pRButtonLobby = GetControl<CEtUIRadioButton>( "ID_RBT_ROBBY" );
	m_pRButtonFriend = GetControl<CEtUIRadioButton>( "ID_RBT_FRIEND" );
	m_pRButtonGuild = GetControl<CEtUIRadioButton>( "ID_RBT_GUILD" );
	m_pRButtonLobby->SetChecked(true);

}

void CDnPVPRoomListDlg::SetRefreshUserCount(int nCount)
{
	if( nCount>=0 && nCount <= PvPCommon::WaitUserList::Common::MaxPerPage )
	{
		m_nRefreshUserCount = nCount; 
	}
}

void CDnPVPRoomListDlg::ClearWaitUserList(bool bShow)
{
	if(vWaitUserSlot.empty())
		return;

	for(int i=0;i<(int)vWaitUserSlot.size();i++)
	{
		vWaitUserSlot[i].pStaticJobIcon->SetIconID( -1 );
		vWaitUserSlot[i].pStaticLevel->Show(bShow);
		vWaitUserSlot[i].pStaticLevel->SetText(L"");
		vWaitUserSlot[i].pStaticRankIcon->Show(bShow);
		vWaitUserSlot[i].pStaticName->Show(bShow);
		vWaitUserSlot[i].pStaticName->SetText(L"");
	}

	m_pUserSelect->Show(false);
}

void CDnPVPRoomListDlg::SetFriendList(int nPageNumber)
{
	int nFriendCount = 0;    // Number of friends per page
	int nAllFriendCount = 0; // Total number of friends

	if(!CTaskManager::GetInstance().GetTask( "FriendTask" ))
		return;

	const CDnFriendTask::FRIEND_MAP& mFriendList = GetFriendTask().GetFriendList();    // Receive friend list.

	if(mFriendList.empty())
		return;

	CDnFriendTask::FRIEND_MAP::const_iterator iter = mFriendList.begin();
	PvPCommon::WaitUserList::Repository sFriendUserList;
		
	for( ; iter != mFriendList.end(); ++iter ) // <Group map list> of friend list
	{
		const CDnFriendTask::FRIENDINFO_MAP *pFriendInfo = iter->second;
		CDnFriendTask::FRIENDINFO_MAP::const_iterator _iter;
		_iter = pFriendInfo->begin();

		for (; _iter != pFriendInfo->end(); ++_iter)
		{
			const CDnFriendTask::SFriendInfo *info = &(_iter->second);

			if(nFriendCount< PvPCommon::WaitUserList::Common::MaxPerPage)
			{

				if((nPageNumber *  PvPCommon::WaitUserList::Common::MaxPerPage) <= nAllFriendCount)
				{
					if(info->bHaveDetailInfo)
					{
						if(info->Location.cServerLocation == CDnFriendTask::_LOCATION_NONE ) // Offline
							continue;

						sFriendUserList.WaitUserList[nFriendCount].cJob = info->nJob;
						sFriendUserList.WaitUserList[nFriendCount].cLevel = info->nLevel;
						sFriendUserList.WaitUserList[nFriendCount].cPvPLevel = 0;
						wsprintf(sFriendUserList.WaitUserList[nFriendCount].wszCharName,info->wszFriendName.c_str()); // Display only when detail info exists.
						nFriendCount++;
					}
					else if(!info->bHaveDetailInfo)
					{
						GetFriendTask().RequestFriendDetailInfo( info->biFriendCharDBID ); // Request if the value is missing.
						continue;
					}
					nAllFriendCount++;
				}
				else
				{
					nAllFriendCount++;
				}
			}
			else
			{
				break;
			}
		}

		if(nFriendCount >= PvPCommon::WaitUserList::Common::MaxPerPage)
			break;
	}
	
	if(m_nUserType == eWaitUser::TypeFriend)
	{
		m_nUserMaxPage = nAllFriendCount/PvPCommon::WaitUserList::Common::MaxPerPage;
		m_nRefreshUserCount = nFriendCount;
		SetWaitUserList(sFriendUserList,m_nUserMaxPage);
	}
}

void CDnPVPRoomListDlg::SetGuildList(int nPageNumber)
{
	if(!CTaskManager::GetInstance().GetTask( "GuildTask" ))
		return;

	int GuildList = GetGuildTask().GetGuildMemberCount();  // Receive guild list.
	int nUserCount = 0;

	PvPCommon::WaitUserList::Repository sUserList;
	
	if(!GetGuildTask().IsHaveGuildInfo())
	{
		GetGuildTask().RequestGetGuildInfo(true);
		m_fRequestGuildTimeLimit = eWaitUser::RequestTimeLimit_Guild;
		return;
	}

	for(int i=0;i<GuildList;i++) // Loop for the number of guilds
	{
		if(i>=(nPageNumber * PvPCommon::WaitUserList::Common::MaxPerPage))
		{
			if(nUserCount<PvPCommon::WaitUserList::Common::MaxPerPage)
			{
				TGuildMember *pMember = GetGuildTask().GetGuildMemberFromArrayIndex(i);
				if(pMember->Location.cServerLocation == CDnFriendTask::_LOCATION_NONE) // Currently offline.
				{
					continue;
				}
				sUserList.WaitUserList[nUserCount].cJob = pMember->nJob;
				sUserList.WaitUserList[nUserCount].cLevel = pMember->cLevel;
				wsprintf(sUserList.WaitUserList[nUserCount].wszCharName,pMember->wszCharacterName);
				nUserCount++;
			}
			else
			{
				continue;
			}
		}
	}

	if(m_nUserType == eWaitUser::TypeGuild)
	{
		m_nUserMaxPage = GuildList/PvPCommon::WaitUserList::Common::MaxPerPage;
		m_nRefreshUserCount = nUserCount;
		SetWaitUserList(sUserList,nPageNumber);
	}
}


void CDnPVPRoomListDlg::SetWaitUserList(PvPCommon::WaitUserList::Repository sUserList , UINT uiMaxPage)
{
	for(int i=0;i<(int)vWaitUserSlot.size();i++)
	{
		if(i>=m_nRefreshUserCount)
			break;

		int iIconW,iIconH;
		int iU,iV;
		iIconW = GetInterface().GeticonWidth();
		iIconH = GetInterface().GeticonHeight();

		if( sUserList.WaitUserList[i].cJob > 0 )
		{
			vWaitUserSlot[i].pStaticJobIcon->SetIconID( sUserList.WaitUserList[i].cJob , true );
		}
	
		vWaitUserSlot[i].pStaticLevel->Show(true);
		vWaitUserSlot[i].pStaticLevel->SetIntToText(sUserList.WaitUserList[i].cLevel);

		if(m_nUserType == eWaitUser::TypeLobby) // Display "PVP Rank Info" only for waiting players.
		{
			if( GetInterface().ConvertPVPGradeToUV( sUserList.WaitUserList[i].cPvPLevel , iU, iV ))
			{
				vWaitUserSlot[i].pStaticRankIcon->SetTexture(GetInterface().GetPVPIconTex(), iU, iV, iIconW, iIconH);
				vWaitUserSlot[i].pStaticRankIcon->Show(true);
			}
		}
		else
		{
			vWaitUserSlot[i].pStaticRankIcon->Show(false); // Do not display friend list or guild list info.
		}

		vWaitUserSlot[i].pStaticName->SetText(sUserList.WaitUserList[i].wszCharName);
		vWaitUserSlot[i].pStaticName->Show(true);
	}

	m_nUserMaxPage = uiMaxPage;

	if(m_nUserPageIndex>m_nUserMaxPage)
	{
		m_nUserPageIndex = m_nUserMaxPage;
	}

	WCHAR wsTemp[256];
	wsprintf(wsTemp,L"%d/%d",m_nUserPageIndex+1,m_nUserMaxPage+1); // Add +1 to each because the minimum value is 0/0.

	if(m_nUserPageIndex == m_nUserMaxPage) // If both are equal, disable the next page button.
	{
		if(m_nUserPageIndex == 0)
		{
			m_pButtonUserPrePage->Enable(false); // If current page is at minimum, disable previous page
		}
		else
		{
			m_pButtonUserPrePage->Enable(true); // Enable if at max value
		}
		m_pButtonUserNextPage->Enable(false);
	}
	else
	{
		if(m_nUserPageIndex > 0)
		{
			m_pButtonUserPrePage->Enable(true); // Current page must be greater than 0 to enable previous page.
		}
		else if(m_nUserPageIndex == 0)
		{
			m_pButtonUserPrePage->Enable(false); // Always disabled when at minimum
		}

		m_pButtonUserNextPage->Enable(true);
	}

	m_pUserPageIndex->SetText(wsTemp);
}

void CDnPVPRoomListDlg::SendRefreshWaitUserList(UINT Page)
{

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	if( m_bVillageAccessMode )
		return;
#endif

	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );

	if( pPVPLobbyTask &&  pPVPLobbyTask->GetEnterd())
	{
		if(m_nUserType == eWaitUser::TypeLobby)
		{
			SendPvPWaitUserList(Page,PvPCommon::WaitUserList::SortType::eSortType::Null,true);
		}
		else if(m_nUserType == eWaitUser::TypeFriend)
		{
			ClearWaitUserList();
			SetFriendList(Page);
			
			if(m_fRequestFriendTimeLimit <= 0)
			{
				if(CTaskManager::GetInstance().GetTask( "FriendTask" ))
					GetFriendTask().RequestFriendLocationList();
				
				m_fRequestFriendTimeLimit = eWaitUser::RequestTimeLimit_Friend; // 1-minute cooldown - pressing refresh does not always send a packet
			}

		}
		else if(m_nUserType == eWaitUser::TypeGuild)
		{
			ClearWaitUserList();
			SetGuildList(Page);

			if(m_fRequestGuildTimeLimit <= 0)
			{
				if(CTaskManager::GetInstance().GetTask( "GuildTask" ))
					SendGetGuildInfo(true);

				m_fRequestGuildTimeLimit = eWaitUser::RequestTimeLimit_Guild;  // 1-minute cooldown - pressing refresh does not always send a packet
			}
		}
	}
}

void CDnPVPRoomListDlg::ClearRoomInfo()
{
	for(int i=0 ; i<PVPRoomList::MAX_SLOTNUM ; i++ )
	{
		m_sSlot[i].pStaticState->SetText(L" " );
		m_sSlot[i].pStaticRoomNum->SetText( L" " );
		m_sSlot[i].pRoomPlayerNum->SetText( L" " ); 
		m_sSlot[i].pRoomName->SetText( L" " );
		m_sSlot[i].pUsePassWord->Show(false);
		m_sSlot[i].pGameMode->SetText( L" " );
		m_sSlot[i].pInMapName->SetText( L" " );
		m_sSlot[i].pPlayerMinMaxLevel->SetText(L"");
#ifdef PRE_PVP_GAMBLEROOM
		m_sSlot[i].pGamblePetal->Show( false );
		m_sSlot[i].pGambleGold->Show( false );
#endif // PRE_PVP_GAMBLEROOM
	}	
}

void CDnPVPRoomListDlg::SetRoomInfo( sRoomInfo* RoomInfo )
{
	if( RoomInfo == NULL )
		return;

    SRoomInfo Info;

	Info.iRoomIndex = RoomInfo->nPVPIndex;

	_wcscpy( Info.szRoomName, _countof(Info.szRoomName), RoomInfo->RoomName, (int)wcslen(RoomInfo->RoomName));
	Info.byCur_PlayerNum = RoomInfo->cPlayerNum;
	Info.byMax_PlayerNum = RoomInfo->cMaxPlayerNum;
	Info.cRoomState = RoomInfo->cRoomState;
	Info.isInGameJoin =   RoomInfo->bInGameJoin;
	Info.cGameMode = RoomInfo->cGameMode;
	Info.uiObjective = RoomInfo->uiObjective;
	Info.uiMapIndex =  RoomInfo->nMapIndex;
	Info.bDropItem = RoomInfo->bDropItem;
	Info.byMin_PlayerLevel = RoomInfo->cMinLevel;
	Info.byMax_PlayerLevel = RoomInfo->cMaxLevel;
	Info.nEventID = RoomInfo->nEventID;

	for( int itr = 0; itr < PvPCommon::TeamIndex::Max; ++itr )
		Info.nGuildDBID[itr] = RoomInfo->nGuildDBID[itr];
#ifdef PRE_MOD_PVPOBSERVER
	Info.bEventRoom = RoomInfo->bExtendObserver;
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_WORLDCOMBINE_PVP
	Info.nWorldPvPRoomType = RoomInfo->nWorldPvPRoomType;
#endif // PRE_WORLDCOMBINE_PVP
#ifdef PRE_PVP_GAMBLEROOM
	Info.cGambleType = RoomInfo->cGambleType;
	Info.nGamblePrice = RoomInfo->nGamblePrice;
#endif // PRE_PVP_GAMBLEROOM
	m_RoomInfoList.push_back(Info);

	UpdateRoomInfo();
}

void CDnPVPRoomListDlg::SetRoomListColor(DWORD dwColor,int nRoomIndex)
{
	if(nRoomIndex >= PVPRoomList::MAX_SLOTNUM || nRoomIndex < 0) 
		return; // This is not allowed.

	m_sSlot[nRoomIndex].pRoomName->SetTextColor(dwColor);
	m_sSlot[nRoomIndex].pRoomPlayerNum->SetTextColor(dwColor);
	m_sSlot[nRoomIndex].pStaticRoomNum->SetTextColor(dwColor);
	m_sSlot[nRoomIndex].pStaticState->SetTextColor(dwColor);
	m_sSlot[nRoomIndex].pPlayerMinMaxLevel->SetTextColor(dwColor);
	m_sSlot[nRoomIndex].pGameMode->SetTextColor(dwColor);
	m_sSlot[nRoomIndex].pInMapName->SetTextColor(dwColor);
}

void CDnPVPRoomListDlg::UpdateRoomInfo()
{
	ClearRoomInfo();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPMAP );
	DNTableFileFormat* pSoxGameMode = GetDNTable( CDnTableDB::TPVPGAMEMODE );

	for (DWORD i = 0;i < m_RoomInfoList.size();i++)
	{
		WCHAR PlayerNum[256];
		WCHAR RoomIndex[256];
		WCHAR wszRoomState[256];
		WCHAR wszPlayerMinMaxLevel[256];
	
		m_sSlot[i].pRoomName->SetText(m_RoomInfoList[i].szRoomName);

		wsprintf( PlayerNum,L"%d/%d",m_RoomInfoList[i].byCur_PlayerNum,m_RoomInfoList[i].byMax_PlayerNum );
		m_sSlot[i].pRoomPlayerNum->SetText(PlayerNum);

		wsprintf( RoomIndex,L"%d",m_RoomInfoList[i].iRoomIndex );
		m_sSlot[i].pStaticRoomNum->SetText(RoomIndex);

		wsprintf( wszRoomState,L"%d",m_RoomInfoList[i].cRoomState );
		m_sSlot[i].pStaticState->SetText(wszRoomState);

	
		wsprintf( wszPlayerMinMaxLevel, L"%d~%d",(int)m_RoomInfoList[i].byMin_PlayerLevel,(int)m_RoomInfoList[i].byMax_PlayerLevel);
		m_sSlot[i].pPlayerMinMaxLevel->SetText(wszPlayerMinMaxLevel);

		if(IsPasssWordRoom(m_RoomInfoList[i].cRoomState))
			m_sSlot[i].pUsePassWord->Show(true);

		if ( pSox ) 
		{
			if( pSox->GetFieldFromLablePtr( m_RoomInfoList[i].uiMapIndex , "MapTableID" )->GetInteger() != 0 )
				m_sSlot[i].pInMapName->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( m_RoomInfoList[i].uiMapIndex, "MapNameUIstring" )->GetInteger()));

		}
	
		if ( pSoxGameMode && m_RoomInfoList[i].cGameMode > 0 ) 
		{
			if(  pSoxGameMode->GetFieldFromLablePtr( m_RoomInfoList[i].cGameMode, "GameModeUIString" )->GetInteger() != -1 )
			{
				m_sSlot[i].pGameMode->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSoxGameMode->GetFieldFromLablePtr( m_RoomInfoList[i].cGameMode, "GameModeUIString" )->GetInteger()));
			}
		}

#ifdef PRE_MOD_PVPOBSERVER
		if( m_RoomInfoList[i].bEventRoom )
			SetRoomListColor( textcolor::YELLOW, i );
		else
		{
			if(m_RoomInfoList[i].nEventID > 0)
				SetRoomListColor(difficultycolor::MASTER,i);
			else
				SetRoomListColor(textcolor::WHITE,i);
		}
#else
		if(m_RoomInfoList[i].nEventID > 0)
			SetRoomListColor(difficultycolor::MASTER,i);
		else
			SetRoomListColor(textcolor::WHITE,i);
#endif // PRE_MOD_PVPOBSERVER

#ifdef PRE_WORLDCOMBINE_PVP
		if( m_RoomInfoList[i].nWorldPvPRoomType == WorldPvPMissionRoom::Common::MissionRoom )
		{
			SetRoomListColor( textcolor::GREENYELLOW, i );
		}
		else if( m_RoomInfoList[i].nWorldPvPRoomType == WorldPvPMissionRoom::Common::GMRoom )
		{
			SetRoomListColor( textcolor::YELLOW, i );
		}
#endif // PRE_WORLDCOMBINE_PVP

#ifdef PRE_PVP_GAMBLEROOM
		if( m_RoomInfoList[i].cGambleType == PvPGambleRoom::Petal )
		{
			m_sSlot[i].pStaticRoomNum->SetText( L" " );
			m_sSlot[i].pGamblePetal->Show( true );
			m_sSlot[i].pGambleGold->Show( false );
			SetRoomListColor( textcolor::YELLOW, i );
		}
		else if( m_RoomInfoList[i].cGambleType == PvPGambleRoom::Gold )
		{
			m_sSlot[i].pStaticRoomNum->SetText( L" " );
			m_sSlot[i].pGamblePetal->Show( false );
			m_sSlot[i].pGambleGold->Show( true );
			SetRoomListColor( textcolor::YELLOW, i );
		}
		else
		{
			m_sSlot[i].pGamblePetal->Show( false );
			m_sSlot[i].pGambleGold->Show( false );
		}
#endif // PRE_PVP_GAMBLEROOM

		UINT uiUIstring = PVPRoomList::READY_ROOM;		

		if( IsSyncingRoom(m_RoomInfoList[i].cRoomState ) || IsStartingRoom(m_RoomInfoList[i].cRoomState))
			uiUIstring = PVPRoomList::RESERVATION_ROOM;

		if( IsPlayingRoom(m_RoomInfoList[i].cRoomState )) 
			uiUIstring = PVPRoomList::PLAYING_ROOM;

		m_sSlot[i].pStaticState->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, uiUIstring ));
	}

	if( 0 == m_uiCurPage )
		m_pPrePage->Enable( false);
	else
		m_pPrePage->Enable( true );

	if( MaxPage == m_uiCurPage)
		m_pNextPage->Enable( false);
	else
		m_pNextPage->Enable( true);

	WCHAR wszPageNum[256];
	SecureZeroMemory(wszPageNum,sizeof(wszPageNum) );

	wsprintf( wszPageNum , L"%d/%d",m_uiCurPage+1,m_uiMaxPage+1);
	m_pPageIndex->SetText(wszPageNum);

	if( m_uiCurPage >= m_uiMaxPage )
		m_pNextPage->Enable( false );

	if( m_RoomInfoList.size() == 0 )//When there is no room 
		m_pQuickEnterButton->Enable( false );
	else
		m_pQuickEnterButton->Enable( true );

	bool bEnableEnter = false;
	if(m_pSelector->IsShow())
	{
		bool bEnableObserver = false;
		UINT uiDBID = 0;

		CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
		if(pPVPLobbyTask)
			uiDBID= pPVPLobbyTask->GetUserInfo().GuildSelfView.GuildUID.nDBID;
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
		else if( m_bVillageAccessMode && CDnActor::s_hLocalActor )
		{
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			uiDBID = pPlayer->GetGuildSelfView().GuildUID.nDBID;
		}
#endif
		else
			return;


		if( !m_RoomInfoList.empty() && m_nSelectedRoomIndex < (int)m_RoomInfoList.size() )
		{
			if( 60 != m_RoomInfoList[m_nSelectedRoomIndex].cGameMode )	// 60 is boss battle.
				bEnableEnter = true;
			else
			{
				if(  AccountLevel_New <= CGlobalInfo::GetInstance().m_cLocalAccountLevel )
				{
					bEnableEnter = true;
					bEnableObserver = true;
				}
				else if( m_RoomInfoList[m_nSelectedRoomIndex].nGuildDBID[0] != 0 )
				{
					bool bMyRoom = false;
					for( int itr = 0; itr < PvPCommon::TeamIndex::Max; ++itr )
					{
						if( uiDBID == m_RoomInfoList[m_nSelectedRoomIndex].nGuildDBID[itr] )
						{
							bMyRoom = true;
							break;
						}
					}

					if( !bMyRoom )
						bEnableEnter = false;
					else
						bEnableEnter = true;
				}
				else
					bEnableEnter = true;

				m_pJoin->Enable( bEnableEnter );

				m_pObserverJoin->Enable(bEnableObserver);
				return;
			}
		}
		else
			m_pSelector->Show( false );
	}

	m_pJoin->Enable( bEnableEnter );
	m_pObserverJoin->Enable(bEnableEnter);
}

void CDnPVPRoomListDlg::SendExitLobby()
{
	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
	if( pPVPLobbyTask &&  pPVPLobbyTask->GetEnterd() ) // Remove firstroom
	{
		SendMovePvPLobbyToPvPVillage();
	}

	CDnMouseCursor::GetInstance().SetStaticMouse(false);
}

void CDnPVPRoomListDlg::OpenExitDlg()
{
	std::wstring msgBoxStr, mapName;

	if( CDnBridgeTask::IsActive())
	{
		DN_INTERFACE::STRING::GetMapName(mapName, CDnBridgeTask::GetInstance().GetPreviousMapIndexForPvP());
		msgBoxStr = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPRoomList::EXIT_DIRECT_FROM_VILLAGE), mapName.c_str()); // UISTRING : Move to the previous town? [Town: %s]
	}
	else
	{
		msgBoxStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPRoomList::EXIT_DIRECT_FROM_VILLAGE); // UISTRING : Move to the previous town? [Town: %s]
	}

	GetInterface().MessageBox( msgBoxStr.c_str(), MB_YESNO , EXIT_DIALOG ,this );
}

void CDnPVPRoomListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	SetCmdControlName( pControl->GetControlName() );

#if defined( PRE_ADD_DWC )
	if( nCommand == EVENT_CHECKBOX_CHANGED && IsCmdControl( "ID_CHECKBOX_DWC" ) )
	{
		CEtUICheckBox *pCheckDwc = static_cast<CEtUICheckBox*>( pControl );
		bool bChecked = pCheckDwc ? pCheckDwc->IsChecked() : false;
		if( GetInterface().GetPVPDWCTabDlg() )
			GetInterface().GetPVPDWCTabDlg()->Show( bChecked );
		if( bChecked && CDnDWCTask::IsActive() )
		{
			if( GetDWCTask().HasDWCTeam() )
			{
				const TDWCTeamInfo &team = GetDWCTask().GetTeamInfo();
				if( GetInterface().GetDWCMarkInformer() )
					GetInterface().GetDWCMarkInformer()->UpdateMarkInfo( team.wszTeamName, GetDWCTask().GetDWCRank(), GetDWCTask().GetDWCGrade() );
				GetInterface().OpenDwcDlg( true );
			}
			else
			{
				GetInterface().OpenDwcCreateTeamDlg( true );
			}
		}
		return;
	}
#endif

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_PVP_CREATE" ) )
		{
			m_pPVPMakeRoomDlg->Shuffle();

			m_pPVPMakeRoomDlg->InitCheckBox();
			m_pPVPMakeRoomDlg->Show( true );
			return;
		}

		if( IsCmdControl("ID_PVP_JOIN" ) )
		{
#ifdef PRE_FIX_PVP_LOBBY_RENEW
			if (CheckJoinRoom(m_RoomInfoList[m_nSelectedRoomIndex]) == false)
				return;
#endif
#ifdef PRE_PVP_GAMBLEROOM
			if( m_RoomInfoList[m_nSelectedRoomIndex].cGambleType == PvPGambleRoom::Petal
				||  m_RoomInfoList[m_nSelectedRoomIndex].cGambleType == PvPGambleRoom::Gold )
			{
				CheckJoinGambleRoom();
			}
			else
			{
				SendJoinRoom( m_nSelectedRoomIndex );
			}
#else // PRE_PVP_GAMBLEROOM
			SendJoinRoom( m_nSelectedRoomIndex );
#endif // PRE_PVP_GAMBLEROOM
			return;
		}

		if( IsCmdControl("ID_PVP_CANCEL" ) )
		{
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
			if( m_bVillageAccessMode )
			{
				GetInterface().ShowPVPVillageAccessDlg(false);
				return;
			}
#endif
			OpenExitDlg();							
			return;
		}

		if(IsCmdControl("ID_PVP_REFRESH"))
		{
			SendRefresh( m_uiCurPage );
			m_pSelector->Show(false);
			return;
		}

		if(IsCmdControl("ID_PVP_QUICKSTART"))
		{
			CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
			if( ( ( pPVPLobbyTask && pPVPLobbyTask->GetEnterd() ) || m_bVillageAccessMode ) && m_isFirstRoomListPacket )
				SendPvPRandomJoinRoom();
#else
			if( pPVPLobbyTask &&  pPVPLobbyTask->GetEnterd() && m_isFirstRoomListPacket )
				SendPvPRandomJoinRoom();
#endif
			return;
		}
	
		if(IsCmdControl("ID_PVP_OBSERVERJOIN"))
		{
			SendJoinRoom(m_nSelectedRoomIndex,true);
			m_bObserverJoin = true;
			return;
		}

		if( IsCmdControl("ID_BUTTON_NUMBER" ) ) //Sort by room number
		{
			m_cRoomSortType = PvPCommon::RoomSortType::Index;

			if( SortTypeDescend == m_emRoomNumAscend || SortTypeNone == m_emRoomNumAscend )
			{
				m_emRoomNumAscend  = SortTypeAccend ;
				m_pRoomNumberSortAc->Show(true);
				m_pRoomNumberSortDc->Show(false);

			}else if(  SortTypeAccend == m_emRoomNumAscend  )
			{
				m_emRoomNumAscend = SortTypeDescend;
				m_pRoomNumberSortAc->Show(false);
				m_pRoomNumberSortDc->Show(true);
			}

			m_emMaxPlayerAscend = SortTypeNone;
			m_pMaxPlayerSortAc->Show(false);
			m_pMaxPlayerSortDc->Show(false);
			m_pMapName->SetText(m_wszDefaultMapName);
			m_pModeName->SetText(m_wszDefaultModeName);

			if(bTriggeredByUser)
				SendRefresh( m_uiCurPage );
			
			return;
		}

		if( IsCmdControl("ID_BUTTON_MEMBERNUM" ) )//Sort by max users
		{
			m_cRoomSortType = PvPCommon::RoomSortType::NumOfPlayer;

			if( SortTypeDescend == m_emMaxPlayerAscend || SortTypeNone == m_emMaxPlayerAscend )
			{
				m_emMaxPlayerAscend  = SortTypeAccend ;
				m_pMaxPlayerSortAc->Show(true);
				m_pMaxPlayerSortDc->Show(false);

			}else if(  SortTypeAccend == m_emMaxPlayerAscend  )
			{
				m_emMaxPlayerAscend = SortTypeDescend;
				m_pMaxPlayerSortAc->Show(false);
				m_pMaxPlayerSortDc->Show(true);
			}

			m_emRoomNumAscend = SortTypeNone;
			m_pRoomNumberSortAc->Show(false);
			m_pRoomNumberSortDc->Show(false);
			m_pMapName->SetText(m_wszDefaultMapName);
			m_pModeName->SetText(m_wszDefaultModeName);

			SendRefresh( m_uiCurPage );
			return;
		}

		if( IsCmdControl("ID_PARTY_PRIOR" ) )
		{
			if( m_uiCurPage > 0 )
				--m_uiCurPage;
			SendRefresh( m_uiCurPage );
			m_pSelector->Show(false);
			return;
		}

		if( IsCmdControl("ID_PARTY_NEXT" ) )
		{
			if( m_uiCurPage < MaxPage )
				++m_uiCurPage;
			SendRefresh( m_uiCurPage );
			m_pSelector->Show(false);
			return;
		}

		if(IsCmdControl("ID_REFRESH" ))
		{
			SendRefreshWaitUserList(m_nUserPageIndex); // CurPage value needs to be passed. // UserType also needs to be passed
			return;
		}
		if(IsCmdControl("ID_NEXT" ))
		{
			if( m_nUserPageIndex < MaxPage )
			{
				m_nUserPageIndex++;
			}

			if(m_nUserPageIndex > m_nUserMaxPage)
				m_nUserPageIndex = m_nUserMaxPage;

			SendRefreshWaitUserList(m_nUserPageIndex);
			return;
		}
		if(IsCmdControl("ID_PRIOR" ))
		{
			if( m_nUserPageIndex > 0 )
			{
				m_nUserPageIndex--;
			}
			SendRefreshWaitUserList(m_nUserPageIndex);
			return;
		}

#ifdef PRE_ADD_PVP_RANKING
		if( IsCmdControl("ID_RANK" ) )
		{
			GetInterface().OpenPvPRankDlg();
			return;
		}
#endif // PRE_ADD_PVP_RANKING
	}
	else if(nCommand == EVENT_RADIOBUTTON_CHANGED)
	{

		if(IsCmdControl("ID_RBT_ROBBY"))
		{
			m_nUserType = eWaitUser::TypeLobby;
			m_nUserPageIndex = 0;
			SendRefreshWaitUserList(m_nUserPageIndex);
			return;

		}
		if(IsCmdControl("ID_RBT_FRIEND"))
		{
			m_nUserType = eWaitUser::TypeFriend;
			m_nUserPageIndex = 0;

			ClearWaitUserList(); // After clearing the list
			SendRefreshWaitUserList(m_nUserPageIndex); 
			return;

		}
		if(IsCmdControl("ID_RBT_GUILD"))
		{
			m_nUserType = eWaitUser::TypeGuild;
			m_nUserPageIndex = 0;
			ClearWaitUserList(); // After clearing the list
			SendRefreshWaitUserList(m_nUserPageIndex);
			return;
		}

	}else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED )
	{
		if( IsCmdControl("ID_COMBOBOX_MAP" ) )
		{

			m_emRoomNumAscend = SortTypeNone;
			m_pRoomNumberSortAc->Show(false);
			m_pRoomNumberSortDc->Show(false);

			m_emMaxPlayerAscend = SortTypeNone;
			m_pMaxPlayerSortAc->Show(false);
			m_pMaxPlayerSortDc->Show(false);

			m_cRoomSortType = PvPCommon::RoomSortType::MapIndex;
			m_pMapCombo->GetSelectedValue(m_iDataIndex);				
			if( m_pMapCombo->GetSelectedItem() )
				m_pMapName->SetText(m_pMapCombo->GetSelectedItem()->strText);
			m_pModeName->SetText(m_wszDefaultModeName);

			if(bTriggeredByUser)
				SendRefresh( m_uiCurPage );

			m_pMapCombo->ClearSelectedItem();
			return;
		}

		if( IsCmdControl("ID_COMBOBOX_MODE" ) )
		{
			m_emRoomNumAscend = SortTypeNone;
			m_pRoomNumberSortAc->Show(false);
			m_pRoomNumberSortDc->Show(false);

			m_emMaxPlayerAscend = SortTypeNone;
			m_pMaxPlayerSortAc->Show(false);
			m_pMaxPlayerSortDc->Show(false);

			m_cRoomSortType = PvPCommon::RoomSortType::GameMode;
			m_pModeCombo->GetSelectedValue(m_iDataIndex);
			if( m_pModeCombo->GetSelectedItem() )
				m_pModeName->SetText(m_pModeCombo->GetSelectedItem()->strText);
			m_pMapName->SetText(m_wszDefaultMapName);
			
			if(bTriggeredByUser)
				SendRefresh( m_uiCurPage );
			
			m_pModeCombo->ClearSelectedItem();
			return;
		}
	}
}

void CDnPVPRoomListDlg::SendJoinRoom( int iRoomindex, bool bObserver )
{
	if( m_RoomInfoList.size() <= 0 || (int)m_RoomInfoList.size() <= iRoomindex || -1 == iRoomindex )
		return;

	UINT uiPvPIndex = m_RoomInfoList[iRoomindex].iRoomIndex ;

	if( uiPvPIndex <= 0 )
	{
		ErrorLog(" SendJoinRoom:: index invalid! ");
		return;	
	}

	if( m_RoomInfoList[iRoomindex].cRoomState&PvPCommon::RoomState::Password?true:false )//Password-protected room
	{
#ifdef PRE_MOD_PVPOBSERVER
		// Enter immediately if event room and spectator entry.
		if( m_RoomInfoList[iRoomindex].bEventRoom && bObserver )
		{
			SendPvPJoinRoom( uiPvPIndex, 0, L"", bObserver );
			m_bObserverJoin = bObserver; // PRE_MOD_PVPOBSERVER
			return;
		}

		if( m_RoomInfoList[iRoomindex].bEventRoom )
		{
			m_bShowPasswordDlg = true;
			ShowChildDialog( m_pPassWordInputEventDlg, true );		
		}
		else
		{
			m_bShowPasswordDlg = false;
			ShowChildDialog( m_pPassWordInputDlg, true );
		}
#else
		ShowChildDialog( m_pPassWordInputDlg, true );
#endif // PRE_MOD_PVPOBSERVER
		m_uiBackUpRoomIndex = uiPvPIndex;
		return; 
	}

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if( m_RoomInfoList[iRoomindex].nEventID > 0 )
	{
		if(pSox && pSox->IsExistItem(m_RoomInfoList[iRoomindex].cGameMode))
		{
			int nNeedInvenCount = 0;
			nNeedInvenCount = pSox->GetFieldFromLablePtr( m_RoomInfoList[iRoomindex].cGameMode , "_NeedInven" )->GetInteger();

			if(nNeedInvenCount != 0)
			{
				CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
				if( pInvenDlg && pInvenDlg->GetEmptySlotCount() < nNeedInvenCount )
				{
					WCHAR wszMsg[256]={0,};
					wsprintf( wszMsg, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120138 ), nNeedInvenCount );
					GetInterface().ShowCaptionDialog( CDnInterface::typeCaption4,  wszMsg , textcolor::ORANGERED ,5.0f );
					return;
				}
			}
		}
	}
	if( false == IsGuildWarRoomEnter( iRoomindex, bObserver ) )
		return;
	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	if( ((pPVPLobbyTask &&  pPVPLobbyTask->GetEnterd()) || m_bVillageAccessMode) && m_isFirstRoomListPacket )
	{
		SendPvPJoinRoom(uiPvPIndex,0,L"",bObserver);
		m_bObserverJoin = bObserver; // PRE_MOD_PVPOBSERVER
	}
#else
	if( pPVPLobbyTask &&  pPVPLobbyTask->GetEnterd() && m_isFirstRoomListPacket )
	{
		SendPvPJoinRoom(uiPvPIndex,0,L"",bObserver);
		m_bObserverJoin = bObserver; // PRE_MOD_PVPOBSERVER
	}
#endif
}

void CDnPVPRoomListDlg::AddLobbyList( sRoomInfo * RoomInfo , UINT uiMaxPage )
{
	if( RoomInfo )
		SetRoomInfo(RoomInfo);

	m_uiMaxPage = uiMaxPage; 
}

void CDnPVPRoomListDlg::RefreshRoomList()
{
	ClearRoomInfo();
}

void CDnPVPRoomListDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( nID == PASSWORDINPUT_DIALOG )
		{
			if( IsCmdControl("ID_OK" ) )
			{
				LPCWSTR PassWord = NULL;
#ifdef PRE_MOD_PVPOBSERVER
				if( m_bShowPasswordDlg )
					PassWord = m_pPassWordInputEventDlg->GetPassword();
				else
					PassWord = m_pPassWordInputDlg->GetPassword();

				GetInterface().SetPVPRoomPassword( PassWord );
#else
				PassWord = m_pPassWordInputDlg->GetPassword();
#endif // PRE_MOD_PVPOBSERVER

				if( PassWord == NULL)
					return;
				
				if(m_bObserverJoin)
				{
					SendPvPJoinRoom( m_uiBackUpRoomIndex, byte(wcslen( PassWord)) , PassWord ,m_bObserverJoin);
					m_bObserverJoin= false;
				}
				else
				{
					SendPvPJoinRoom( m_uiBackUpRoomIndex, byte(wcslen( PassWord)) , PassWord );
					m_bObserverJoin = false; // PRE_MOD_PVPOBSERVER
				}
				
#ifdef PRE_MOD_PVPOBSERVER
				if( m_bShowPasswordDlg )
				{
					m_bShowPasswordDlg = false;
					ShowChildDialog( m_pPassWordInputEventDlg, false );
				}
				else
				{					
					ShowChildDialog( m_pPassWordInputDlg, false );
				}
#else
				ShowChildDialog( m_pPassWordInputDlg, false );
#endif // PRE_MOD_PVPOBSERVER
			}

			if( IsCmdControl("ID_CANCEL" ) )
			{
#ifdef PRE_MOD_PVPOBSERVER
				if( m_bShowPasswordDlg )
				{
					m_bShowPasswordDlg = false;
					ShowChildDialog( m_pPassWordInputEventDlg, false );
				}
				else
					ShowChildDialog( m_pPassWordInputDlg, false );
#else
				ShowChildDialog( m_pPassWordInputDlg, false );
#endif // PRE_MOD_PVPOBSERVER				
			}			
		}

		if( nID == EXIT_DIALOG )
		{
			if( IsCmdControl("ID_YES" ) )
			{
				SendExitLobby();
			}
		}
#ifdef PRE_PVP_GAMBLEROOM
		if( nID == CONFIRM_JOIN_GAMBLEROOM )
		{
			if( IsCmdControl("ID_YES" ) )
			{
				SendJoinRoom( m_nSelectedRoomIndex, false );
			}
		}
#endif // PRE_PVP_GAMBLEROOM

#ifdef PRE_FIX_PVP_LOBBY_RENEW
		if (nID == CONFIRM_NO_BREAKINTO)
		{
			if (IsCmdControl("ID_YES"))
			{
	#ifdef PRE_PVP_GAMBLEROOM
				if( m_RoomInfoList[m_nSelectedRoomIndex].cGambleType == PvPGambleRoom::Petal
					||  m_RoomInfoList[m_nSelectedRoomIndex].cGambleType == PvPGambleRoom::Gold )
				{
					CheckJoinGambleRoom();
				}
				else
				{
					SendJoinRoom( m_nSelectedRoomIndex );
				}
	#else // PRE_PVP_GAMBLEROOM
				SendJoinRoom( m_nSelectedRoomIndex );
	#endif // PRE_PVP_GAMBLEROOM
			}
		}
#endif
	}

}

void CDnPVPRoomListDlg::SendRefresh( UINT Page)
{
	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	if( (pPVPLobbyTask && pPVPLobbyTask->GetEnterd()) || m_bVillageAccessMode )
#else
	if( pPVPLobbyTask && pPVPLobbyTask->GetEnterd() )
#endif
	{
		switch( m_cRoomSortType )
		{
		case PvPCommon::RoomSortType::GameMode :
		case PvPCommon::RoomSortType::MapIndex :
			SendPvPRoomList( Page , m_cRoomSortType , true ,m_iDataIndex);
			break;
		case PvPCommon::RoomSortType::NumOfPlayer :
			SendPvPRoomList( Page , m_cRoomSortType , SortTypeAccend == m_emMaxPlayerAscend?true:false);
			break;
		case PvPCommon::RoomSortType::Index:
			SendPvPRoomList( Page , m_cRoomSortType , SortTypeAccend == m_emRoomNumAscend?true:false);			
			break;
		default:
			ErrorLog("SortType is wrong");
			break;
		}
	}

}

void CDnPVPRoomListDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	m_fRefreshTime += fElapsedTime;

	if( m_fRefreshTime > fSendRefreshTime/1000  ) //Auto-refresh interval: 5 seconds
	{
		m_fRefreshTime =0.0f;
		SendRefresh( m_uiCurPage );
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
		if( !m_bVillageAccessMode )
			SendRefreshWaitUserList(m_nUserPageIndex);
#else
		SendRefreshWaitUserList(m_nUserPageIndex);
#endif
	}

	if(m_fRequestFriendTimeLimit  > -1) m_fRequestFriendTimeLimit  -= fElapsedTime;
	if(m_fRequestGuildTimeLimit  > -1) m_fRequestGuildTimeLimit  -= fElapsedTime;
}

bool CDnPVPRoomListDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	if( !IsShow() )
	{
		return false;
	}	

	switch( uMsg )
	{
		case WM_LBUTTONDBLCLK:
			{
				POINT MousePoint;
				float fMouseX, fMouseY;

				if ( m_pMapCombo && m_pModeCombo )
				{
					if( m_pMapCombo->IsOpenedDropDownBox() || m_pModeCombo->IsOpenedDropDownBox() || m_pPVPMakeRoomDlg->IsShow())
						break;
				}

				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );
				PointToFloat( MousePoint, fMouseX, fMouseY );

				int nFindSelect = FindInsideSlot( fMouseX, fMouseY );
				if( nFindSelect > -1 && (UINT)nFindSelect < m_RoomInfoList.size() )
				{
					m_nSelectedRoomIndex = nFindSelect;
#ifdef PRE_FIX_PVP_LOBBY_RENEW
					if (CheckJoinRoom(m_RoomInfoList[m_nSelectedRoomIndex]) == false)
						break;
#endif
#ifdef PRE_PVP_GAMBLEROOM
					if( m_RoomInfoList[m_nSelectedRoomIndex].cGambleType == PvPGambleRoom::Petal
						||  m_RoomInfoList[m_nSelectedRoomIndex].cGambleType == PvPGambleRoom::Gold )
					{
						CheckJoinGambleRoom();
					}
					else
					{
						SendJoinRoom( m_nSelectedRoomIndex );
					}
#else // PRE_PVP_GAMBLEROOM
					SendJoinRoom( m_nSelectedRoomIndex );
#endif // PRE_PVP_GAMBLEROOM
				}
			}
			break;

		case WM_LBUTTONDOWN:
			{
				POINT MousePoint;
				float fMouseX, fMouseY;

				if ( m_pMapCombo && m_pModeCombo )
				{
					if( m_pMapCombo->IsOpenedDropDownBox() || m_pModeCombo->IsOpenedDropDownBox() || m_pPVPMakeRoomDlg->IsShow())
						break;
				}

				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );
				PointToFloat( MousePoint, fMouseX, fMouseY );

				int nFindSelect = FindInsideSlot( fMouseX, fMouseY );
				if( nFindSelect > -1 && (UINT)nFindSelect < m_RoomInfoList.size() )
				{
					m_nSelectedRoomIndex = nFindSelect;
					UpdateSelectBar();
				}


				for( int i = 0 ; i < m_nRefreshUserCount ; ++i ) 
				{
					if( !vWaitUserSlot[i].pStaticDummy ) continue;
					SUICoord uiCoord;
					vWaitUserSlot[i].pStaticDummy->GetUICoord( uiCoord );

					if( uiCoord.IsInside( fMouseX, fMouseY ) ) 
					{
						m_pUserSelect->SetUICoord(vWaitUserSlot[i].pStaticDummy->GetUICoord());
						m_pUserSelect->Show(true);
						break;
					}
				}

				if( IsMouseInDlg() ) {
					if( m_pPVPPopupDlg->IsShow() ) {
						SUICoord uiCoord;
						m_pPVPPopupDlg->GetDlgCoord( uiCoord );
						fMouseX = MousePoint.x / GetScreenWidth();
						fMouseY = MousePoint.y / GetScreenHeight();
						if( !uiCoord.IsInside( fMouseX, fMouseY ) ) {
							ShowChildDialog( m_pPVPPopupDlg, false );
							return true;
						}
					}
				}
			}
			break;

		case WM_RBUTTONDOWN:
			{
				POINT MousePoint;
				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );

				float fMouseX, fMouseY;
				PointToFloat( MousePoint, fMouseX, fMouseY );

				bool bIsInsideItem = false;

				if(!m_pPVPPopupDlg->IsShow()) {
					for( int i = 0 ; i < m_nRefreshUserCount ; ++i ) {

						if( !vWaitUserSlot[i].pStaticDummy ) continue;

						SUICoord uiCoord;
						vWaitUserSlot[i].pStaticDummy->GetUICoord( uiCoord );

						if( uiCoord.IsInside( fMouseX, fMouseY ) ) {
							bIsInsideItem = true;
							m_pPVPPopupDlg->SetTargetName(vWaitUserSlot[i].pStaticName->GetText());
							m_pUserSelect->SetUICoord(vWaitUserSlot[i].pStaticDummy->GetUICoord());
							m_pUserSelect->Show(true);
							break;
						}
					}
				}
			
				if( bIsInsideItem ) {
					fMouseX = MousePoint.x / GetScreenWidth();
					fMouseY = MousePoint.y / GetScreenHeight();
					
					SUICoord sDlgCoord;
					static float fyBGap(16.f /DEFAULT_UI_SCREEN_WIDTH);   // Looks better positioned slightly to the right..


					ShowChildDialog( m_pPVPPopupDlg, false );
					m_pPVPPopupDlg->SetPosition( fMouseX+fyBGap, fMouseY );

					m_pPVPPopupDlg->GetDlgCoord( sDlgCoord );
					if( (sDlgCoord.Right()) > GetScreenWidthRatio() )
						sDlgCoord.fX -= (sDlgCoord.Right() - GetScreenWidthRatio()); //  Push back if overflowing off screen.
					m_pPVPPopupDlg->SetDlgCoord( sDlgCoord );
					ShowChildDialog( m_pPVPPopupDlg, true );
				}

			}
			break;

		case WM_MOUSEWHEEL:
			{
				POINT MousePoint;
				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );

				float fMouseX, fMouseY;
				PointToFloat( MousePoint, fMouseX, fMouseY );

				if( IsMouseInDlg() )
				{
					UINT uLines;
					SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
					int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
					if( nScrollAmount > 0 )
					{
						if( m_uiCurPage > 0 )
							--m_uiCurPage;
						SendRefresh( m_uiCurPage );						
						break;
					}
					else if( nScrollAmount < 0 )
					{
						if( m_uiCurPage < MaxPage && m_uiCurPage < m_uiMaxPage )
							++m_uiCurPage;
						SendRefresh( m_uiCurPage );						
						break;
					}
				}
			}
			break;

#ifdef PRE_ADD_PVP_HELP_MESSAGE
		case WM_MOUSEMOVE:
			{
				POINT MousePoint;
				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );

				float fMouseX, fMouseY;
				PointToFloat( MousePoint, fMouseX, fMouseY );
				
				m_pLobbyImagePopUpDlg->Show(false);

				CDnSimpleTooltipDlg* pSimpleTooltTip = GetInterface().GetSimpleTooltipDialog();
				if( pSimpleTooltTip == NULL )
					break;

				for(int iSlotNum = 0 ; iSlotNum < (int)m_RoomInfoList.size(); iSlotNum++)
				{
					if( !m_sSlot )
						continue;

					if( !m_sSlot[iSlotNum].pInMapName->IsEnable() || !m_sSlot[iSlotNum].pInMapName->IsShow() )
						continue;

					if( !m_sSlot[iSlotNum].pGameMode->IsEnable() || !m_sSlot[iSlotNum].pGameMode->IsShow() )
						continue;

					std::wstring str;
					if( m_sSlot[iSlotNum].pGameMode->GetUICoord().IsInside(fMouseX, fMouseY) )
					{
						if(pSimpleTooltTip)
						{
							DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
							if (!pSox)
								return false;

							int nUiNumber = 0;
							for( int i=0; i<pSox->GetItemCount() ; i++ ) 
							{
								nUiNumber = pSox->GetFieldFromLablePtr( m_RoomInfoList[iSlotNum].cGameMode , "_TooltipUIString" )->GetInteger();
								if(nUiNumber > 0)
									break;
							}

							if (nUiNumber)
								str = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUiNumber);
							
							if (!str.empty())
							{
								m_sSlot[iSlotNum].pGameMode->ClearTooltipText();
								pSimpleTooltTip->ShowTooltipDlg(m_sSlot[iSlotNum].pGameMode, true, str, 0xffffffff, true);
							}
						}
						break;
					}

					if( m_sSlot[iSlotNum].pInMapName->GetUICoord().IsInside(fMouseX, fMouseY) )
					{
						bool bResult = m_pLobbyImagePopUpDlg->LoadPVPMapImage( m_RoomInfoList[iSlotNum].uiMapIndex );
						m_pLobbyImagePopUpDlg->Show(bResult);
						break;
					}
#ifdef PRE_PVP_GAMBLEROOM
					if( m_sSlot[iSlotNum].pGamblePetal->IsShow() && m_sSlot[iSlotNum].pGamblePetal->GetUICoord().IsInside( fMouseX, fMouseY ) )
					{
						str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8397 ), DN_INTERFACE::UTIL::GetAddCommaString( m_RoomInfoList[iSlotNum].nGamblePrice ).c_str(), 
										GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4614 ) );
						pSimpleTooltTip->ShowTooltipDlg( m_sSlot[iSlotNum].pGameMode, true, str, 0xffffffff, true );
						break;
					}
					else if( m_sSlot[iSlotNum].pGambleGold->IsShow() && m_sSlot[iSlotNum].pGambleGold->GetUICoord().IsInside( fMouseX, fMouseY ) )
					{
						str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8397 ), DN_INTERFACE::UTIL::GetAddCommaString( m_RoomInfoList[iSlotNum].nGamblePrice ).c_str(), 
							GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 507 ) );
						pSimpleTooltTip->ShowTooltipDlg( m_sSlot[iSlotNum].pGameMode, true, str, 0xffffffff, true );
						break;
					}
#endif // PRE_PVP_GAMBLEROOM
				}
			}
			break;
#endif
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

int CDnPVPRoomListDlg::FindInsideSlot( float fX, float fY )
{
	for( int i = 0; i < PVPRoomList::MAX_SLOTNUM; i++ )
	{
		if( m_sSlot[i].pRoomSelect->IsInside( fX, fY ) )
		{
			return i;
		}
	}

	return -1;
}

void CDnPVPRoomListDlg::UpdateSelectBar()
{
	SUICoord uiCoordBase, uiCoordSelect;
	m_sSlot[m_nSelectedRoomIndex].pRoomSelect->GetUICoord(uiCoordBase);
	m_pSelector->GetUICoord(uiCoordSelect);
	uiCoordSelect.fX = uiCoordBase.fX ;
	uiCoordSelect.fY = uiCoordBase.fY ;
	m_pSelector->SetUICoord(uiCoordSelect);
	m_pSelector->Show(true);

}

void CDnPVPRoomListDlg::MakeMapList()
{
	int nItemIndex= 0;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPMAP );
	if ( !pSox ) 
	{
		ErrorLog("CDnPVPRoomListDlg::MakeMapList():: PVP Map table Not found!! ");
		return;
	}

	for( int i=0; i<pSox->GetItemCount() ; i++ ) 
	{
		nItemIndex = pSox->GetItemID(i);
		if( 0 < nItemIndex )
		{
			if( pSox->GetFieldFromLablePtr( nItemIndex, "ReleaseShow" )->GetInteger() == 0 ) continue;
			if( pSox->GetFieldFromLablePtr( nItemIndex, "MapTableID" )->GetInteger() == 0 ) continue;

			bool bIsLadderMode = false;
			bIsLadderMode = pSox->GetFieldFromLablePtr( nItemIndex, "_LadderType" )->GetInteger() ? true : false;

			if(bIsLadderMode)
				continue;

			m_pMapCombo->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nItemIndex, "MapNameUIstring" )->GetInteger() ) ,
								NULL,
								pSox->GetFieldFromLablePtr( nItemIndex, "MapTableID" )->GetInteger(),
								true );
		}
	}

}

void CDnPVPRoomListDlg::MakeModeList()
{
	int nItemIndex= 0;
	int nModeType = 0;
	bool bIsLadderMode = false;


	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );
	if ( !pSox ) 
	{
		ErrorLog("CDnPVPRoomListDlg::MakeModeList():: GameMode table Not found!! ");
		return;
	}

	for( int i=0; i<pSox->GetItemCount() ; i++ ) 
	{
		nItemIndex = pSox->GetItemID(i);		
		nModeType = pSox->GetFieldFromLablePtr( nItemIndex, "GamemodeID" )->GetInteger();
	
		if( nModeType != -1 )
		{			
			bool IsNewMode = true;
			
			//IsNewMode = m_pModeCombo->ContainsItem(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nItemIndex, "GameModeUIString" )->GetInteger()));
			IsNewMode = ( m_pModeCombo->FindItemByValue( nModeType ) != -1 );

			switch(nModeType)
			{
			case PvPCommon::GameMode::PvP_Respawn:
			case PvPCommon::GameMode::PvP_Round:
			case PvPCommon::GameMode::PvP_Captain:
			case PvPCommon::GameMode::PvP_IndividualRespawn:
			case PvPCommon::GameMode::PvP_Zombie_Survival:
			case PvPCommon::GameMode::PvP_Occupation:
			case PvPCommon::GameMode::PvP_AllKill:
			case PvPCommon::GameMode::PvP_GuildWar:
#ifdef PRE_ADD_PVP_COMBOEXERCISE
			case PvPCommon::GameMode::PvP_ComboExercise:				
#endif // PRE_ADD_PVP_COMBOEXERCISE
#ifdef PRE_ADD_PVP_TOURNAMENT
			case PvPCommon::GameMode::PvP_Tournament:
#endif
				break;
#if defined( PRE_ADD_RACING_MODE )
			case PvPCommon::GameMode::PvP_Racing:
				break;
#endif	// #if defined( PRE_ADD_RACING_MODE )
			default:
				continue;
			}
			bIsLadderMode = pSox->GetFieldFromLablePtr( nItemIndex , "LadderType" )->GetInteger() ? true : false;
			if(bIsLadderMode)
				continue;

			if( !IsNewMode && pSox->GetFieldFromLablePtr( nItemIndex, "ReleaseShow")->GetInteger() == 1 )
			{
#ifdef PRE_ADD_PVP_TOURNAMENT
				std::wstring gameModeUIString = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr(nItemIndex, "GameModeUIString")->GetInteger());
				if (nModeType == PvPCommon::GameMode::PvP_Tournament)
					gameModeUIString = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120175); // UISTRING : Tournament
				m_pModeCombo->AddItem(gameModeUIString.c_str(), NULL, nModeType, true);
#else
				m_pModeCombo->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nItemIndex, "GameModeUIString" )->GetInteger() ),NULL, nModeType, true );
#endif
			}
		}
	}

}

void CDnPVPRoomListDlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	if(m_bVillageAccessMode)
	{
		if(GetInterface().GetMainBarDialog())
			GetInterface().GetMainBarDialog()->ToggleMenuButton( CDnMainMenuDlg::PVP_VILLAGE_ACCESS , bShow );

		if( bShow )
			SendRefresh(m_uiCurPage);
		return;
	}
#endif

	if( bShow )
	{
		CDnChatTabDlg *pChatDialog = (CDnChatTabDlg*)GetInterface().GetPVPLobbyChatTabDlg();
		if(pChatDialog)
			pChatDialog->ShowEx(true);

		CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
		if(pPVPLobbyTask)
		{
			if(!pPVPLobbyTask->GetUserInfo().GuildSelfView.IsSet()) // Is player in a guild?
				m_pRButtonGuild->Enable(false);
			else
				m_pRButtonGuild->Enable(true);
		}
		CDnBridgeTask::GetInstance().SetGuildWarSystem( false );
	}
	else
	{
		m_pLadderInviteConfirmDlg->Show(false);
		m_bObserverJoin = false;
	}

	if( m_pPVPPopupDlg && m_pPVPPopupDlg->IsShow() )
		ShowChildDialog( m_pPVPPopupDlg, false );
}

void CDnPVPRoomListDlg::ShowLadderInviteConfirmDlg(LadderSystem::SC_INVITE_CONFIRM_REQ *pData)
{
	if(m_pLadderInviteConfirmDlg)
	{
		m_pLadderInviteConfirmDlg->Show(true);
		m_pLadderInviteConfirmDlg->SetInviteInfo(pData);
		if(m_pPVPMakeRoomDlg)
			m_pPVPMakeRoomDlg->Show(false);
	}
}

bool CDnPVPRoomListDlg::IsGuildWarRoomEnter( const int iRoomIndex, const bool bObserver )
{
	if( 0 != m_RoomInfoList[iRoomIndex].nGuildDBID[0] )
	{
		if( bObserver )
		{	
			if( CGlobalInfo::GetInstance().m_cLocalAccountLevel < eAccountLevel::AccountLevel_New )
			{
				GetInterface().MessageBox( PVPRoomList::GUILDWAR_SYSTEM_ROOM_OBSERVER_NOT_ENTER );
				return false;
			}
		}
		else
		{
			bool bMyRoom = false;
			UINT uiDBID = 0;

			CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
			if( pPVPLobbyTask )
				uiDBID = pPVPLobbyTask->GetUserInfo().GuildSelfView.GuildUID.nDBID;
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
			else if(CDnActor::s_hLocalActor)
			{
				CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
				uiDBID = pPlayer->GetGuildSelfView().GuildUID.nDBID;
			}
#endif

			for( int itr = 0; itr < PvPCommon::TeamIndex::Max; ++itr )
			{
				if( uiDBID == m_RoomInfoList[iRoomIndex].nGuildDBID[itr] )
				{
					bMyRoom = true;
					break;
				}
			}

			if( false == bMyRoom )
			{
				GetInterface().MessageBox( PVPRoomList::GUILDWAR_SYSTEM_ROOM_NOT_MEMBER_ENTER );
				return false;
			}
		}
	}

	return true;
}

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS

void CDnPVPRoomListDlg::EnableVillageAccessMode(bool bTrue)
{
	m_bVillageAccessMode = bTrue;
}

#endif

#ifdef PRE_PVP_GAMBLEROOM
void CDnPVPRoomListDlg::CheckJoinGambleRoom()
{
	int nMoneyStringIndex = 0;
	if( m_RoomInfoList[m_nSelectedRoomIndex].cGambleType == PvPGambleRoom::Petal )
		nMoneyStringIndex = 4614;
	else if( m_RoomInfoList[m_nSelectedRoomIndex].cGambleType == PvPGambleRoom::Gold )
		nMoneyStringIndex = 507;

	CDnMessageBoxTextBox* pDnMessageBoxTextBox = GetInterface().GetMessageBoxTextBox();
	if( pDnMessageBoxTextBox )
	{				
		pDnMessageBoxTextBox->ClearText();

		std::wstring strMessage;
		strMessage = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8406 ), 
							DN_INTERFACE::UTIL::GetAddCommaString( m_RoomInfoList[m_nSelectedRoomIndex].nGamblePrice ).c_str(), 
							GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMoneyStringIndex ) );

		pDnMessageBoxTextBox->AddMessage( strMessage.c_str(), textcolor::YELLOW, UITEXT_CENTER );
		pDnMessageBoxTextBox->AddMessage( L"\n" );
		pDnMessageBoxTextBox->AddMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8399 ), textcolor::WHITE, UITEXT_CENTER );
		GetInterface().MessageBoxTextBox( L" ", MB_YESNO, CONFIRM_JOIN_GAMBLEROOM, this );
	}
}
#endif // PRE_PVP_GAMBLEROOM

#ifdef PRE_FIX_PVP_LOBBY_RENEW
bool CDnPVPRoomListDlg::CheckJoinRoom(const SRoomInfo& info)
{
	bool bIsPlaying = ((info.cRoomState & (PvPCommon::RoomState::Starting|PvPCommon::RoomState::Playing|PvPCommon::RoomState::NoMoreBreakInto)) != 0) ? true : false;
	if (info.isInGameJoin == false && bIsPlaying)
	{
		GetInterface().MessageBox(8433, MB_YESNO, CONFIRM_NO_BREAKINTO, this); // UISTRING : Join-in-progress is not allowed in this room. Do you want to enter?
		return false;
	}

	return true;
}
#endif