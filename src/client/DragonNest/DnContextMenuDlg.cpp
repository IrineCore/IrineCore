#include "StdAfx.h"

#include "DnContextMenuDlg.h"
#include "DnInterfaceString.h"
#include "TaskManager.h"
#include "DnFriendTask.h"
#include "DnChatTabDlg.h"
#include "DnPVPLobbyVillageTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnContextMenuDlg::CDnContextMenuDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CEtUIDialog(dialogType, pParentDialog, nID, pCallback, true)
	, m_pTitle(NULL)
	, m_pBackGround(NULL)
{
	m_bWaitClose = false;
}

void CDnContextMenuDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PCInfoDlg01.ui" ).c_str(), bShow );

	const float topSpaceHeight	= (3.f / GetScreenHeight()) * GetScreenHeightRatio();
	const float gapBetweenCtrls = (2.f / GetScreenHeight()) * GetScreenHeightRatio();

	SUICoord coord = m_pBackGround->GetBaseUICoord();
	float newHeight = coord.fHeight;
	newHeight = topSpaceHeight + (m_pTitle ? m_pTitle->GetUICoord().fHeight : 0.f) + gapBetweenCtrls;
	if (newHeight < coord.fHeight)
		newHeight = coord.fHeight;
	m_pBackGround->SetSize(coord.fWidth, newHeight);

	PreBakeMenu();
}

void CDnContextMenuDlg::InitialUpdate()
{
	m_pBackGround	= GetControl<CEtUIStatic>("ID_STATIC_BOARD");
	//m_pTitle		= GetControl<CEtUIStatic>("ID_STATIC_TITLE");	// control deleted

	int i = 0;
	for (; i < MAX_CONTEXTMENU_BTNS; ++i)
	{
		SButtonInfo info;
		std::string ctrlName = FormatA("ID_BUTTON_%d", i);
		info.pBtn = GetControl<CEtUIButton>(ctrlName.c_str());
		if (info.pBtn == NULL)
			break;
		info.pBtn->Show(false);
		m_pButtonList.push_back(info);
	}

// control deleted
/*	
	for (i = 0; i < MAX_CONTEXTMENU_STATICS; ++i)
	{
		SStaticInfo info;
		std::string ctrlName = FormatA("ID_STATIC_%d", i);
		info.pStatic = GetControl<CEtUIStatic>(ctrlName.c_str());
		if (info.pStatic == NULL)
			break;
		info.pStatic->Show(false);

		ctrlName = FormatA("ID_STATIC_BG%d", i);
		info.pBGStatic = GetControl<CEtUIStatic>(ctrlName.c_str());
		if (info.pBGStatic == NULL)
			break;
		info.pBGStatic->Show(false);

		m_pStaticList.push_back(info);
	}
*/
}

CEtUIButton* CDnContextMenuDlg::GetUsableButton()
{
	std::vector<SButtonInfo>::iterator iter = m_pButtonList.begin();
	for (; iter != m_pButtonList.end(); ++iter)
	{
		SButtonInfo& info = *iter;
		if (info.pBtn)
		{
			if (info.pBtn->IsShow() == false)
				return info.pBtn;
		}
	}

	return NULL;
}

CDnContextMenuDlg::SStaticInfo* CDnContextMenuDlg::GetUsableStaticInfo()
{
	std::vector<SStaticInfo>::iterator iter = m_pStaticList.begin();
	for (; iter != m_pStaticList.end(); ++iter)
	{
		SStaticInfo& info = *iter;
		if (info.pStatic)
		{
			if (info.pStatic->IsShow() == false)
				return &info;
		}
	}

	return NULL;
}

void CDnContextMenuDlg::Show(bool bShow)
{
	CEtUIDialog::Show(bShow);
}

void CDnContextMenuDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */)
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		Show(false);
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnContextMenuDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			POINT MousePoint;
			float fMouseX, fMouseY;

			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );
			PointToFloat( MousePoint, fMouseX, fMouseY );

			if (m_pBackGround && m_pBackGround->IsInside(fMouseX, fMouseY) == false)
				Show(false);
		}
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnContextMenuDlg::RefreshDlg()
{
	const float gap = (15.f / GetScreenHeight()) * GetScreenHeightRatio();
	const float buttonGap = (3.5f / GetScreenHeight()) * GetScreenHeightRatio();
	const float staticGap = (5.f / GetScreenHeight()) * GetScreenHeightRatio();

	const std::wstring& titleText = m_pTitle ? m_pTitle->GetText() : L"";
	float fHeight = titleText.empty() ? 0.f : m_pBackGround->GetBaseUICoord().fHeight;
	std::vector<SStaticInfo>::const_iterator staticIter =	m_pStaticList.begin();
	for (; staticIter != m_pStaticList.end(); ++staticIter)
	{
		const SStaticInfo& pInfo = *staticIter;
		if (pInfo.pBGStatic->IsShow())
			fHeight += pInfo.pBGStatic->GetBaseUICoord().fHeight + staticGap;
	}

	std::vector<SButtonInfo>::const_iterator btnIter =	m_pButtonList.begin();
	for (; btnIter != m_pButtonList.end(); ++btnIter)
	{
		const SButtonInfo& pInfo = *btnIter;
		if (pInfo.pBtn->IsShow())
			fHeight += pInfo.pBtn->GetBaseUICoord().fHeight + buttonGap;
	}

	fHeight += (titleText.empty() ? gap : 0.f);

	m_pBackGround->SetSize(m_pBackGround->GetBaseUICoord().fWidth, fHeight);
}

void CDnContextMenuDlg::AddButton(const std::wstring& text)
{
	CEtUIButton* pBtn = GetUsableButton();
	if (pBtn)
	{
		pBtn->SetText(text.c_str());
		pBtn->Show(true);

		RefreshDlg();
	}
}

void CDnContextMenuDlg::AddStatic(const std::wstring& text)
{
	SStaticInfo* pStaticInfo = GetUsableStaticInfo();
	if (pStaticInfo && pStaticInfo->pStatic && pStaticInfo->pBGStatic)
	{
		pStaticInfo->pStatic->SetText(text.c_str());
		pStaticInfo->pStatic->Show(true);
		pStaticInfo->pBGStatic->Show(true);

		RefreshDlg();
	}
}

void CDnContextMenuDlg::SetTitle(LPCWSTR wszTitle)
{
	if (m_pTitle == NULL)
		return;

	m_pTitle->SetText(wszTitle);

	RefreshDlg();
}

void CDnContextMenuDlg::SetButton(int index, const std::wstring& text)
{
	if (index < 0 || index >= (int)m_pButtonList.size())
		return;

	m_pButtonList[index].pBtn->SetText(text);
}

void CDnContextMenuDlg::SetStatic(int index, const std::wstring& text)
{
	if (index < 0 || index >= (int)m_pStaticList.size())
		return;

	m_pStaticList[index].pStatic->SetText(text);
}

void CDnContextMenuDlg::EnableButton(const std::wstring& btnText, bool bEnable)
{
	std::vector<SButtonInfo>::iterator iter = m_pButtonList.begin();
	for (; iter != m_pButtonList.end(); ++iter)
	{
		SButtonInfo& info = *iter;
		if (btnText.compare(info.pBtn->GetText()) == 0)
		{
			info.pBtn->Enable(bEnable);
			break;
		}
	}
}

bool CDnContextMenuDlg::IsEnableButton(const std::wstring& btnText) const
{
	std::vector<SButtonInfo>::const_iterator iter = m_pButtonList.begin();
	for (; iter != m_pButtonList.end(); ++iter)
	{
		const SButtonInfo& info = *iter;
		if (btnText.compare(info.pBtn->GetText()) == 0)
			return info.pBtn->IsEnable();
	}

	return false;
}

void CDnContextMenuDlg::EnableStatic(const std::wstring& staticText, bool bEnable)
{
	std::vector<SStaticInfo>::iterator iter = m_pStaticList.begin();
	for (; iter != m_pStaticList.end(); ++iter)
	{
		SStaticInfo& info = *iter;
		if (staticText.compare(info.pStatic->GetText()) == 0)
		{
			info.pStatic->Enable(bEnable);
			break;
		}
	}
}

void CDnContextMenuDlg::Process(float fElapsedTime)
{
	CEtUIDialog::Process(fElapsedTime);
}

// todo by kalliste : Refactor with directdlg::CheckFriendCondition()
bool CDnContextMenuDlg::CheckFriendCondition(const WCHAR* name)
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

	if (CDnActor::s_hLocalActor && !__wcsicmp_l(name, CDnActor::s_hLocalActor->GetName()))
	{
		GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4311), MB_OK);	// UISTRING : Cannot add yourself as a friend.
		return false;
	}

	return true;
}

SUICoord CDnContextMenuDlg::GetBackgroundUISize()
{
	return m_pBackGround->GetUICoord();
}

//////////////////////////////////////////////////////////////////////////

#include "DnTradeTask.h"
#include "DnGuildTask.h"
#include "GameOptionSendPacket.h"

CDnPartyContextMenuDlg::CDnPartyContextMenuDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CDnContextMenuDlg(dialogType, pParentDialog, nID, pCallback)
, m_SessionID( 0 )
, m_bRequestFriend( false )
, m_bReceivePermission( false )
{
}

void CDnPartyContextMenuDlg::SetInfo(const SContextMenuInfo* pInfo)
{
	if (pInfo == NULL)
	{
		_ASSERT(0);
		return;
	}

	const SContextMenuPartyInfo* pPartyInfo = static_cast<const SContextMenuPartyInfo*>(pInfo); // todo by kalliste: Dislike the casting pass-through approach. Find an alternative.
	m_SessionID = pPartyInfo->dwSessionID;
	m_Name		= pPartyInfo->name;

	if (m_Name.empty() == false)
		SetStatic(eNAME, m_Name.c_str());

// 	if (pPartyInfo->level > 0)
// 	{
// 		std::wstring levelStr;
// 		levelStr = FormatW(L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 64 ), pPartyInfo->level);
// 		SetTitle(levelStr.c_str());
// 	}

	if (pPartyInfo->job > 0)
		SetStatic(eJOB, DN_INTERFACE::STRING::GetJobString(pPartyInfo->job));

	m_bReceivePermission = false;
	GetInterface().RegisterMsgListenDlg(eGameOption::SC_GAMEOPTION_USERCOMMOPTION, this);
	SendCharCommOption(m_SessionID);
}

bool CDnPartyContextMenuDlg::IsFriend(const WCHAR *name)
{
	CDnFriendTask* pFriendTask = static_cast<CDnFriendTask*>(CTaskManager::GetInstance().GetTask( "FriendTask" ));
	bool bFriend = ( pFriendTask != NULL ) && pFriendTask->IsFriend(name);
	return bFriend;
}

// todo by kalliste : Refactor with directdlg::CheckValidActor()
bool CDnPartyContextMenuDlg::CheckValidActor()
{
	bool bValid = true;
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( m_SessionID );
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

void CDnPartyContextMenuDlg::Show(bool bShow)
{
	if (bShow == false && m_bWaitClose == false)
		Clear();

	CDnContextMenuDlg::Show(bShow);
}

void CDnPartyContextMenuDlg::Process(float fElapsedTime)
{
	CDnContextMenuDlg::Process(fElapsedTime);

	if (m_bRequestFriend)
	{
		if (CheckFriendCondition(m_Name.c_str()))
			GetFriendTask().RequestFriendAdd(0, m_Name.c_str());
		Clear();
	}

	if (IsShow())
	{
		bool bMe = false;
		if (CDnActor::s_hLocalActor)
			bMe = (m_Name.compare(CDnActor::s_hLocalActor->GetName()) == 0);

		bool bCheckValidActor = false;
		if (bMe == false)
			bCheckValidActor = CheckValidActor();

		bool bProcessed = false;
		if (bMe || bCheckValidActor == false || GetInterface().IsOpenAcceptDialog())
		{
			EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1406), false);	// UISTRING : Inspect
			EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1404), false);	// UISTRING : Request Trade
			EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1402), false);	// UISTRING : Add Friend
			EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3728), false);	// UISTRING : Guild Invite
			bProcessed = true;
		}

		if (m_bReceivePermission && bProcessed == false)
		{
			bool bEnableInspect, bEnableGuild, bEnableFriend, bEnableTrade;
			bEnableInspect = bEnableTrade = bEnableFriend = bEnableGuild = true;
#ifdef PRE_ADD_QUICK_PVP
			bool bEnableDuel = true;
#endif

			if (m_cGuildInviteAcceptable == _CA_OPTIONTYPE_DENYALL)
				bEnableGuild = false;
			if (m_cTradeRequestAcceptable == _CA_OPTIONTYPE_DENYALL)
				bEnableTrade = false;
#ifdef PRE_MOD_ALLOW_INSPECT
#else
			if (m_cObserveStuffAcceptable == _CA_OPTIONTYPE_DENYALL)
				bEnableInspect = false;
#endif	// #ifdef PRE_MOD_ALLOW_INSPECT
#ifdef PRE_ADD_QUICK_PVP
			if (m_cObserveStuffAcceptable == _CA_OPTIONTYPE_DENYALL)
				bEnableDuel = false;
#endif

			bool bFriend = IsFriend(m_Name.c_str());
			bool bGuild = false;	// TODO
			if (!bFriend && !bGuild)
			{
				if (m_cTradeRequestAcceptable == _CA_OPTIONTYPE_FRIEND_GUILD_ACCEPT)
					bEnableTrade = false;
#ifdef PRE_MOD_ALLOW_INSPECT
#else
				if (m_cObserveStuffAcceptable == _CA_OPTIONTYPE_FRIEND_GUILD_ACCEPT)
					bEnableInspect = false;
#endif	// #ifdef PRE_MOD_ALLOW_INSPECT
			}

			// If self is requesting a trade with someone, or someone is requesting a trade with self,
			if( GetTradeTask().GetTradePrivateMarket().IsRequesting() || GetTradeTask().GetTradePrivateMarket().IsAccepting() )
				bEnableTrade = false;

			EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1406), (bEnableInspect && bCheckValidActor));	// UISTRING : Inspect
			EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1404), bEnableTrade);	// UISTRING : Request Trade
			EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1402), bEnableFriend);	// UISTRING : Add Friend
			EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3728), bEnableGuild);	// UISTRING : Guild Invite
#ifdef PRE_ADD_QUICK_PVP
			EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1405), bEnableDuel);	// UISTRING : Request Duel
#endif
		}

		// Even if the guild button is enabled, recheck if invite is possible based on guild info.
		// Do not search for targets nearby every frame due to high computation cost. Just check with own authority.
		if (bProcessed == false && IsEnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3728)))
		{
			bool bEnableGuild = false;
			if (GetGuildTask().GetGuildInfo()->IsSet() &&
				GetGuildTask().GetAuth( static_cast<eGuildRoleType>(GetGuildTask().GetMyRole()), GUILDAUTH_TYPE_INVITE ) &&
				CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage)
				bEnableGuild = true;

			if (bEnableGuild == false)
				EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3728), false);	// UISTRING : Guild Invite
		}
	}
}

void CDnPartyContextMenuDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if (m_SessionID == 0 || m_Name.empty())
		{
			_ASSERT(0);
			return;
		}

		if (IsCmdControl("ID_BUTTON_0"))
		{
			GetInterface().OpenInspectPlayerInfoDialog(m_SessionID);
		}
		else if (IsCmdControl("ID_BUTTON_1"))
		{
			GetTradeTask().GetTradePrivateMarket().RequestPrivateMarket(m_SessionID);
		}
		else if (IsCmdControl("ID_BUTTON_2"))
		{
			if (CheckFriendCondition(m_Name.c_str()))
			{
				wchar_t wszQuery[255];
				swprintf_s(wszQuery, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1418), m_Name.c_str());	// UISTRING : Add %s to your friend list?

				GetInterface().MessageBox(wszQuery, MB_YESNO, MESSAGEBOX_FRIEND_QUERY, this);
				m_bWaitClose = true;
			}
		}
		else if (IsCmdControl("ID_BUTTON_3"))
		{
			GetGuildTask().RequestInviteGuildMember(m_Name.c_str());
		}
		else if (IsCmdControl("ID_BUTTON_4"))
		{
			CEtUILineEditBox::CopyStringToClipboard(m_Name.c_str());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1422), false);
		}
		else if (IsCmdControl("ID_BUTTON_5"))
		{
			GetPartyTask().RequestNestInfo(m_SessionID);
		}
	}

	CDnContextMenuDlg::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnPartyContextMenuDlg::OnUICallbackProc(int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{
	case MESSAGEBOX_FRIEND_QUERY:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if (IsCmdControl("ID_YES"))
					m_bRequestFriend = true;
			}
		}
		break;
	}
}

void CDnPartyContextMenuDlg::OnHandleMsg(int protocol, char* pData)
{
	if (protocol == eGameOption::SC_GAMEOPTION_USERCOMMOPTION)
	{
		SCGameOptionCommOption* pOption = (SCGameOptionCommOption*)pData;
		if (m_SessionID != pOption->nSessionID)
			return;

		memcpy(m_cCommunityOption, pOption->cCommunityOption, sizeof(m_cCommunityOption));
		m_bReceivePermission = true;
	}
}

void CDnPartyContextMenuDlg::PreBakeMenu()
{
	SetTitle(L"");	// Level
	AddStatic(L"");	// eNAME
	AddStatic(L"");	// eJOB
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1406));	// UISTRING : Inspect
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1404));	// UISTRING : Request Trade
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1402));	// UISTRING : Add Friend
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3728));	// UISTRING : Guild Invite
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1415));	// UISTRING : Save Name
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3336));	// UISTRING : Nest Experience
}

void CDnPartyContextMenuDlg::Clear()
{
	m_SessionID = 0;
	m_Name.clear();
	m_bRequestFriend = false;
	m_bReceivePermission = false;
	m_bWaitClose = false;
}

//////////////////////////////////////////////////////////////////////////
#include "DnPartyTask.h"

CDnChatContextMenuDlg::CDnChatContextMenuDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
{
	m_bRequestFriend = false;
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	m_bRequstPartyWithChat = false;
#endif 
}

void CDnChatContextMenuDlg::PreBakeMenu()
{
	SetTitle(L"");
	AddStatic(L"");
	AddStatic(L"");
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 648));	// UISTRING : Whisper
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1402));	// UISTRING : Add Friend
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1401));	// UISTRING : Party Invite
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3511));	// UISTRING : Party Application
	AddButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1415));	// UISTRING : Save Name
}

void CDnChatContextMenuDlg::SetInfo(const SContextMenuInfo* pInfo)
{
	if (pInfo == NULL)
	{
		_ASSERT(0);
		return;
	}

	const SContextMenuChatInfo* pChatInfo = static_cast<const SContextMenuChatInfo*>(pInfo); // todo by kalliste: Dislike the casting pass-through approach. Find an alternative.
	m_Name		= pChatInfo->name;
}

bool CDnChatContextMenuDlg::SetCurrentPartyMember()
{
	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );

	if (m_Name.empty() || 
		(CDnPartyTask::IsActive() && CDnPartyTask::GetInstance().IsPartyMember(m_Name.c_str()))
		|| GetInterface().GetCurrentDungeonEnterDialog() != NULL
		|| pPVPLobbyTask
		)
	{
		EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3511), false); // UISTRING : Party Application
		EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1401), false); // UISTRING : Party Invite

		return true;
	}
	else
	{
		EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3511), true); // UISTRING : Party Application
		EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1401), true); // UISTRING : Party Invite
	}

	return false;
}

void CDnChatContextMenuDlg::Show(bool bShow)
{
	if (bShow == false && m_bWaitClose == false)
		Clear();

	SetCurrentPartyMember();

	CDnContextMenuDlg::Show(bShow);
}

void CDnChatContextMenuDlg::Process(float fElapsedTime)
{
	CDnContextMenuDlg::Process(fElapsedTime);

	if (m_bRequestFriend)
	{
		if (CheckFriendCondition(m_Name.c_str()))
			CDnFriendTask::GetInstance().RequestFriendAdd(0, m_Name.c_str());
		Clear();
	}

	if (GetInterface().GetCurrentDungeonEnterDialog() != NULL && IsEnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3511)))
	{
		EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3511), false); // UISTRING : Party Application
		EnableButton(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1401), false); // UISTRING : Party Invite
	}

	if (m_bShow == false)
		return;
}

void CDnChatContextMenuDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if (m_Name.empty())
			return;

		if (IsCmdControl("ID_BUTTON_0"))	// Whisper
		{
			CDnChatTabDlg *pChatDlg = GetInterface().GetChatDialog();
			if (pChatDlg)
				pChatDlg->SetPrivateName(m_Name);
		}
		else if (IsCmdControl("ID_BUTTON_1"))	// Add friend
		{
			if (CheckFriendCondition(m_Name.c_str()))
			{
				wchar_t wszQuery[255];
				swprintf_s(wszQuery, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1418), m_Name.c_str());	// UISTRING : Add %s to your friend list?

				GetInterface().MessageBox(wszQuery, MB_YESNO, MESSAGEBOX_FRIEND_QUERY, this);
				m_bWaitClose = true;
			}
		}
		else if (IsCmdControl("ID_BUTTON_2"))	// Party invite
		{
			if (CDnPartyTask::IsActive())
				CDnPartyTask::GetInstance().ReqInviteParty(m_Name.c_str());
		}
		else if (IsCmdControl("ID_BUTTON_3"))
		{
			if (CDnPartyTask::IsActive())
			{
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
				GetPartyTask().ReqPartyInfoWithPlayerName( m_Name.c_str() );
				SetReqPartyWithChat( true );
#else
				CDnPartyTask::GetInstance().ReqAskParty(m_Name.c_str());
#endif
			}
		}
		else if (IsCmdControl("ID_BUTTON_4"))
		{
			CEtUILineEditBox::CopyStringToClipboard( m_Name.c_str() );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1422 ), false );	// UISTRING : Saved the character's name. You can now paste it.
		}
	}

	CDnContextMenuDlg::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnChatContextMenuDlg::Clear()
{
	m_bWaitClose = false;
	m_bRequestFriend = false;
	m_Name.clear();
}

void CDnChatContextMenuDlg::OnUICallbackProc(int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{
	case MESSAGEBOX_FRIEND_QUERY:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if (IsCmdControl("ID_YES"))
					m_bRequestFriend = true;
			}
		}
		break;
	}
}