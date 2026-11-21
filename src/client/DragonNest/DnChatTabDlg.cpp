#include "StdAfx.h"
#include "DnChatTabDlg.h"
#include "DnChatDlg.h"
#include "DnChatOptDlg.h"
#include "VillageSendPacket.h"
#include "DnTableDB.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "PartySendPacket.h"
#include "EtUIXML.h"
#include "DebugCmdProc.h"
#include "DnChatHelpDlg.h"
#include "DnInterfaceString.h"
#include "DnLocalPlayerActor.h"
#include "MAChatBalloon.h"
#include "DnInvenTabDlg.h"
#include "DnPartyTask.h"
#include "DnTradeTask.h"
#include "DnMouseCursor.h"
#include "DnIsolate.h"
#include "DnHeadIcon.h"
#include "DNGestureTask.h"
#include "DnWorld.h"
#include "TaskManager.h"
#include "DnVillageTask.h"
#include "DnRestraintTask.h"
#include "DnPvPGameTask.h"
#include "GameOption.h"
#include "DnMissionTask.h"

#include "DnNameLinkMng.h"
#ifdef PRE_MOD_CHATBG
#include "DnChatBoardDlg.h"
#endif

#ifdef PRE_PRIVATECHAT_CHANNEL
#include "DNCountryUnicodeSet.h"
#include "DnChannelChatTask.h"
#include "DnPrivateChannelDlg.h"
#endif // PRE_PRIVATECHAT_CHANNEL

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
#include "DnLocalDataMgr.h"
#endif // PRE_ADD_PRIVATECHAT_CHANNEL



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#define CHATDLG_EXT_SIZE	0.07f

CDnChatTabDlg::CDnChatTabDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CEtUITabDialog(dialogType, pParentDialog, nID, pCallback)
	, m_pNormalTab(NULL)
#ifdef PRE_ADD_CHAT_RENEWAL
	, m_pWhisperTab(NULL) // Whisper.
#endif
	, m_pPartyTab(NULL)
	, m_pGuildTab(NULL)
	, m_pSystemTab(NULL)
	, m_pButtonDummy(NULL)
	, m_nSizeMode(0)
	, m_fBasisY(0.0f)
	, m_pChatNormalDlg(NULL)
	, m_pChatPartyDlg(NULL)
	, m_pChatGuildDlg(NULL)
	, m_pChatSystemDlg(NULL)
	, m_pIMEEditBox(NULL)
	, m_pStaticMode(NULL)
	, m_nChatMode(CHAT_NORMAL)
#ifdef PRE_ADD_CHAT_RENEWAL	
	, m_pChatWhisperDlg(NULL) // Whisper.
	, m_bChangeChatMode(false)
#endif
#ifdef PRE_PRIVATECHAT_CHANNEL
	, m_pPrivateChannelTab(NULL)
	, m_pPrivateChannelDlg(NULL)
#endif // PRE_PRIVATECHAT_CHANNEL
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	, m_pStaticPrivateChannelNoticeText(NULL)
	, m_pStaticPrivateChannelNoticeBalloon(NULL)
	, m_pStaticPrivateChannelTabHighlight(NULL)
	, m_bJoinPrivateChannelNotify(false)
	, m_fJoinPrivateChannelNotify(0.0f)
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
	, m_bShowOptDlg(false)
	, m_ChatDlgSize(SIZE_STEP_SMALL)
	, m_pButtonModeUp(NULL)
	, m_pButtonModeDown(NULL)
	, m_pButtonOption(NULL)
	, m_pButtonHelp(NULL)
	, m_pButtonReport(NULL)
	, m_bMouseIn(false)
	, m_bSetFocusEditBox(false)
	, m_pChatHelpDlg(NULL)
	, m_nRepeat(0)
	, m_IsPVPGameChat(false)
	, m_bPrivateModeChecker(false)
	, m_nPrivateAlarmSoundIndex(-1)
	, m_fPaperingRemainTime(0.0f)
	, m_bPVPLobbyChat(false)
	, m_nChatModeBeforePrivate(CHAT_NO)
	, m_bCustomizeChatMode(false)
	, m_nCustomizeModeType(-1)
	, m_wszCustomizeModeHeader(NULL)
	, m_dwCustomizeModeTextColor(-1)
	, m_bIgnoreShowFunc(false)
#if defined(PRE_ADD_ANTI_CHAT_SPAM)
	, m_bSpammer(false)
#endif
#ifdef PRE_MOD_CHATBG
	, m_pChatBoardDlg(NULL)
#endif
#ifdef PRE_ADD_WORLD_MSG_RED
	, m_pWorldChatRedBoard(NULL)
	, m_pWorldChatRed(NULL)
	, m_fWorldMsgRedCoolTime(0.0f)
	, m_fWorldMsgRed1Sec(0.0f)
	, m_nBlinkCount(0)
#endif // PRE_ADD_WORLD_MSG_RED
{
#if defined(PRE_FIX_57852)
	m_bPvPLobbyChatOtherAreaInside = false;
#endif // PRE_FIX_57852
}

CDnChatTabDlg::~CDnChatTabDlg(void)
{
	SAFE_DELETE(m_pChatHelpDlg);
#ifdef PRE_MOD_CHATBG
	SAFE_DELETE(m_pChatBoardDlg);
#endif

	if (m_nPrivateAlarmSoundIndex == -1)
	{
		CEtSoundEngine::GetInstance().RemoveSound(m_nPrivateAlarmSoundIndex);
	}
}

void CDnChatTabDlg::Process(float fElapsedTime)
{
	CEtUIDialog::Process(fElapsedTime);

	// All timing handling for each request is done here; the dialog only displays the processed values (such as time).
	std::list<float>::iterator iter = m_listRecentChatTime.begin();
	while (iter != m_listRecentChatTime.end())
	{
		*iter -= fElapsedTime;
		if (*iter <= 0.0f)
		{
			iter = m_listRecentChatTime.erase(iter);
			continue;
		}
		++iter;
	}

	if (m_fPaperingRemainTime > 0.0f)
		m_fPaperingRemainTime -= fElapsedTime;

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	UpdatePrivateChannelNotify(fElapsedTime);
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

#ifdef PRE_ADD_WORLD_MSG_RED
	if (!m_bPVPLobbyChat && m_pWorldChatRed)
	{
		if (m_fWorldMsgRedCoolTime > 0.0f)
		{
			m_fWorldMsgRedCoolTime -= fElapsedTime;
			if (m_fWorldMsgRedCoolTime < 0.0f)
				m_fWorldMsgRedCoolTime = 0.0f;

			m_fWorldMsgRed1Sec -= fElapsedTime;
			if (m_nBlinkCount > 0)
			{
				if (m_fWorldMsgRed1Sec <= 0.0f)
				{
					m_fWorldMsgRed1Sec = 1.0f;
					m_pWorldChatRed->Show(false);
					m_nBlinkCount--;
				}
				else if (m_fWorldMsgRed1Sec < 0.5f)
				{
					m_pWorldChatRed->Show(true);
				}
			}
			else
			{
				if (m_fWorldMsgRed1Sec < 0.5f)
					m_pWorldChatRed->Show(true);
			}
		}
		else
		{
			m_pWorldChatRed->Show(false);
			m_pWorldChatRedBoard->Show(false);
		}
	}
#endif // PRE_ADD_WORLD_MSG_RED
}

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
void CDnChatTabDlg::UpdatePrivateChannelNotify(float fElapsedTime)
{
	if (!IsShow())
		return;

	if (m_bJoinPrivateChannelNotify)
	{
		if (m_fJoinPrivateChannelNotify >= 5.f)
		{
			m_bJoinPrivateChannelNotify = false;
			m_pStaticPrivateChannelNoticeText->Show(false);
			m_pStaticPrivateChannelNoticeBalloon->Show(false);
			m_pStaticPrivateChannelTabHighlight->Show(false);
		}
		else
		{
			static int nTimeBack;
			int nBlinkTime = int(m_fJoinPrivateChannelNotify * 5.0f);

			if (((nBlinkTime % 3) == 0) && (nTimeBack != nBlinkTime))
			{
				nTimeBack = nBlinkTime;
				m_pStaticPrivateChannelNoticeText->Show(!m_pStaticPrivateChannelNoticeText->IsShow());
				m_pStaticPrivateChannelNoticeBalloon->Show(!m_pStaticPrivateChannelNoticeBalloon->IsShow());
				m_pStaticPrivateChannelTabHighlight->Show(!m_pStaticPrivateChannelTabHighlight->IsShow());
			}
		}

		m_fJoinPrivateChannelNotify += fElapsedTime;
	}
}

void CDnChatTabDlg::SetPrivateChannelNotify(bool bNotify)
{
	if (m_pStaticPrivateChannelNoticeText == NULL || m_pStaticPrivateChannelNoticeBalloon == NULL || m_pStaticPrivateChannelTabHighlight == NULL)
		return;

	if (bNotify)
	{
		m_bJoinPrivateChannelNotify = true;
		m_fJoinPrivateChannelNotify = 0.0f;
		m_pStaticPrivateChannelNoticeText->Show(true);
		m_pStaticPrivateChannelNoticeBalloon->Show(true);
		m_pStaticPrivateChannelTabHighlight->Show(true);

		if (!IsShow())
			Show(true);

		RequestFocus(m_pButtonOption);
		CDnMouseCursor::GetInstance().ShowCursor(false, true);
	}
	else
	{
		m_bJoinPrivateChannelNotify = false;
		m_fJoinPrivateChannelNotify = 0.0f;
		m_pStaticPrivateChannelNoticeText->Show(false);
		m_pStaticPrivateChannelNoticeBalloon->Show(false);
		m_pStaticPrivateChannelTabHighlight->Show(false);
	}
}

#endif // PRE_ADD_PRIVATECHAT_CHANNEL

void CDnChatTabDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("ChatTabDlg.ui").c_str(), bShow);
	SetFadeMode(CEtUIDialog::AllowRender);

	if (!CDnTableDB::GetInstancePtr())
		return;

	const char *szFileName = CDnTableDB::GetInstance().GetFileName(10015);
	if (strlen(szFileName) > 0)
	{
		m_nPrivateAlarmSoundIndex = CEtSoundEngine::GetInstance().LoadSound(CEtResourceMng::GetInstance().GetFullName(szFileName).c_str(), false, false);
	}
}

void CDnChatTabDlg::InitialUpdate()
{
	// Note : General tab and window
	m_pNormalTab = GetControl<CEtUIRadioButton>("ID_TAB_ALL");
	m_pChatNormalDlg = new CDnChatDlg(UI_TYPE_CHILD, this);
	m_pChatNormalDlg->Initialize(false);
	AddTabDialog(m_pNormalTab, m_pChatNormalDlg);
	m_vecChatDlg.push_back(m_pChatNormalDlg);

	// Note : Whisper tab and window.
#ifdef PRE_ADD_CHAT_RENEWAL
	m_pWhisperTab = GetControl<CEtUIRadioButton>("ID_TAB_SECRET");
	m_pChatWhisperDlg = new CDnChatDlg(UI_TYPE_CHILD, this);
	m_pChatWhisperDlg->Initialize(false);
	AddTabDialog(m_pWhisperTab, m_pChatWhisperDlg);
	m_vecChatDlg.push_back(m_pChatWhisperDlg);
#endif

	// Note: Party tab and window
	m_pPartyTab = GetControl<CEtUIRadioButton>("ID_TAB_PARTY");
	m_pChatPartyDlg = new CDnChatDlg(UI_TYPE_CHILD, this);
	m_pChatPartyDlg->Initialize(false);
	AddTabDialog(m_pPartyTab, m_pChatPartyDlg);
	m_vecChatDlg.push_back(m_pChatPartyDlg);

	// Note : Guild tab and window
	m_pGuildTab = GetControl<CEtUIRadioButton>("ID_TAB_GUILD");
	m_pChatGuildDlg = new CDnChatDlg(UI_TYPE_CHILD, this);
	m_pChatGuildDlg->Initialize(false);
	AddTabDialog(m_pGuildTab, m_pChatGuildDlg);
	m_vecChatDlg.push_back(m_pChatGuildDlg);


	// Note : System tab and window
	m_pSystemTab = GetControl<CEtUIRadioButton>("ID_TAB_SYSTEM");
	m_pChatSystemDlg = new CDnChatDlg(UI_TYPE_CHILD, this);
	m_pChatSystemDlg->Initialize(false);
	AddTabDialog(m_pSystemTab, m_pChatSystemDlg);
	m_vecChatDlg.push_back(m_pChatSystemDlg);

#ifdef PRE_PRIVATECHAT_CHANNEL
	// Note : Private channel tab and window
	m_pPrivateChannelTab = GetControl<CEtUIRadioButton>("ID_TAB_CHANNEL");
	m_pPrivateChannelDlg = new CDnChatDlg(UI_TYPE_CHILD, this);
	m_pPrivateChannelDlg->Initialize(false);
	AddTabDialog(m_pPrivateChannelTab, m_pPrivateChannelDlg);
	m_vecChatDlg.push_back(m_pPrivateChannelDlg);
#endif // PRE_PRIVATECHAT_CHANNEL
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	m_pStaticPrivateChannelNoticeText = GetControl<CEtUIStatic>("ID_TEXT_CHANNEL");
	m_pStaticPrivateChannelNoticeText->Initialize(false);
	m_pStaticPrivateChannelNoticeBalloon = GetControl<CEtUIStatic>("ID_STATIC_CHANNEL");
	m_pStaticPrivateChannelNoticeBalloon->Initialize(false);
	m_pStaticPrivateChannelTabHighlight = GetControl<CEtUIStatic>("ID_STATIC_CHANNELCOVER");
	m_pStaticPrivateChannelTabHighlight->Initialize(false);
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

	m_pChatNormalDlg->InitOption(CHAT_NORMAL);
	m_pChatPartyDlg->InitOption(CHAT_PARTY);
	m_pChatGuildDlg->InitOption(CHAT_GUILD);
	m_pChatSystemDlg->InitOption(CHAT_SYSTEM);
#ifdef PRE_ADD_CHAT_RENEWAL
	m_pChatWhisperDlg->InitOption(CHAT_PRIVATE);
#endif
#ifdef PRE_PRIVATECHAT_CHANNEL
	m_pPrivateChannelDlg->InitOption(CHAT_PRIVATE_CHANNEL);
#endif // PRE_PRIVATECHAT_CHANNEL

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	CDnLocalDataMgr::GetInstance().SetChatOption(m_pChatNormalDlg->GetChatOptDlg().GetChatTabType(), m_pChatNormalDlg->GetChatOptDlg().GetOption());
	CDnLocalDataMgr::GetInstance().SetChatOption(m_pChatPartyDlg->GetChatOptDlg().GetChatTabType(), m_pChatPartyDlg->GetChatOptDlg().GetOption());
	CDnLocalDataMgr::GetInstance().SetChatOption(m_pChatGuildDlg->GetChatOptDlg().GetChatTabType(), m_pChatGuildDlg->GetChatOptDlg().GetOption());
	CDnLocalDataMgr::GetInstance().SetChatOption(m_pChatSystemDlg->GetChatOptDlg().GetChatTabType(), m_pChatSystemDlg->GetChatOptDlg().GetOption());
	CDnLocalDataMgr::GetInstance().SetChatOption(m_pChatWhisperDlg->GetChatOptDlg().GetChatTabType(), m_pChatWhisperDlg->GetChatOptDlg().GetOption());
	CDnLocalDataMgr::GetInstance().SetChatOption(m_pPrivateChannelDlg->GetChatOptDlg().GetChatTabType(), m_pPrivateChannelDlg->GetChatOptDlg().GetOption());
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

	SetCheckedTab(m_pNormalTab->GetTabID());

	m_pButtonDummy = GetControl<CEtUIButton>("ID_BUTTON_DUMMY");

	m_mapCommandMode.insert(make_pair(std::wstring(L"/s "), CHAT_NORMAL));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/S "), CHAT_NORMAL));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/1 "), CHAT_NORMAL));

	m_mapCommandMode.insert(make_pair(std::wstring(L"/p "), CHAT_PARTY));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/P "), CHAT_PARTY));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/2 "), CHAT_PARTY));

	m_mapCommandMode.insert(make_pair(std::wstring(L"/g "), CHAT_GUILD));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/G "), CHAT_GUILD));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/3 "), CHAT_GUILD));

	m_mapCommandMode.insert(make_pair(std::wstring(L"/w "), CHAT_PRIVATE));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/W "), CHAT_PRIVATE));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/4 "), CHAT_PRIVATE));

#ifdef PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert(make_pair(std::wstring(L"/c "), CHAT_PRIVATE_CHANNEL));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/C "), CHAT_PRIVATE_CHANNEL));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/5 "), CHAT_PRIVATE_CHANNEL));
#else // PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert(make_pair(std::wstring(L"/c "), CHAT_CHANNEL));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/C "), CHAT_CHANNEL));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/5 "), CHAT_CHANNEL));

	m_mapCommandMode.insert(make_pair(std::wstring(L"/t "), CHAT_WORLD));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/T "), CHAT_WORLD));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/6 "), CHAT_WORLD));
#endif // PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert(make_pair(std::wstring(L"/r "), CHAT_PRIVATE_REPLY));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/R "), CHAT_PRIVATE_REPLY));

#if defined(_RDEBUG)
	m_mapCommandMode.insert(make_pair(std::wstring(L"/\u3134 "), CHAT_NORMAL));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/\u3154 "), CHAT_PARTY));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/\u314E "), CHAT_GUILD));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/\u3148 "), CHAT_PRIVATE));

#ifdef PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert(make_pair(std::wstring(L"/\u314A "), CHAT_PRIVATE_CHANNEL));
#else // PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert(make_pair(std::wstring(L"/\u314A "), CHAT_CHANNEL));
	m_mapCommandMode.insert(make_pair(std::wstring(L"/\u3145 "), CHAT_WORLD));
#endif // PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert(make_pair(std::wstring(L"/\u3131 "), CHAT_PRIVATE_REPLY));
#endif

	m_mapCommandMessage.insert(make_pair(std::wstring(L"/?"), CMD_MSG_HELP));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/invite"), CMD_MSG_INVITE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/makeparty"), CMD_MSG_MAKEPARTY));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/leaveparty"), CMD_MSG_LEAVEPARTY));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/ban"), CMD_MSG_BAN));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/trade"), CMD_MSG_TRADE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/escape"), CMD_MSG_ESCAPE));
#ifdef PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/chcreate"), CMD_MSG_CHANNEL_CREATE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/chjoin"), CMD_MSG_CHANNEL_JOIN));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/chpassword"), CMD_MSG_CHANNEL_PASSWORD));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/chinvite"), CMD_MSG_CHANNEL_INVITE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/chleave"), CMD_MSG_CHANNEL_LEAVE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/chban"), CMD_MSG_CHANNEL_BAN));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/chmaster"), CMD_MSG_CHANNEL_MASTER));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/chhelp"), CMD_MSG_CHANNEL_HELP));
#endif // PRE_PRIVATECHAT_CHANNEL

#if defined(_RDEBUG)
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uD30C\uD2F0\uCD08\uB300"), CMD_MSG_INVITE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uD30C\uD2F0\uC0DD\uC131"), CMD_MSG_MAKEPARTY));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uD30C\uD2F0\uD0C8\uD1F4"), CMD_MSG_LEAVEPARTY));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uD30C\uD2F0\uCD94\uBC29"), CMD_MSG_BAN));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uAC70\uB798"), CMD_MSG_TRADE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uD0C8\uCD9C"), CMD_MSG_ESCAPE));
#ifdef PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uCC44\uB110\uAC1C\uC124"), CMD_MSG_CHANNEL_CREATE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uCC44\uB110\uCC38\uC5EC"), CMD_MSG_CHANNEL_JOIN));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uCC44\uB110\uBE44\uBC00\uBC88\uD638"), CMD_MSG_CHANNEL_PASSWORD));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uCC44\uB110\uCD08\uB300"), CMD_MSG_CHANNEL_INVITE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uB098\uAC00\uAE30"), CMD_MSG_CHANNEL_LEAVE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uCC44\uB110\uCD94\uBC29"), CMD_MSG_CHANNEL_BAN));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uBC29\uC7A5"), CMD_MSG_CHANNEL_MASTER));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\uCC44\uB110\uB3C4\uC6C0\uB9D0"), CMD_MSG_CHANNEL_HELP));
#endif // PRE_PRIVATECHAT_CHANNEL
#endif

	m_mapCommandMessage.insert(make_pair(std::wstring(L"/y"), CMD_MSG_RAIDNOTICE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/Y"), CMD_MSG_RAIDNOTICE));
	m_mapCommandMessage.insert(make_pair(std::wstring(L"/\u315B"), CMD_MSG_RAIDNOTICE));

	m_pStaticMode = GetControl<CEtUIStatic>("ID_STATIC_MODE");
	m_pIMEEditBox = GetControl<CEtUIIMEEditBox>("ID_EDITBOX_CHAT");
	m_pButtonModeUp = GetControl<CEtUIButton>("ID_BUTTON_MODE_UP");
	m_pButtonModeDown = GetControl<CEtUIButton>("ID_BUTTON_MODE_DOWN");
	m_pButtonOption = GetControl<CEtUIButton>("ID_BUTTON_OPTION");
	m_pButtonHelp = GetControl<CEtUIButton>("ID_BUTTON_HELP");
	m_pButtonReport = GetControl<CEtUIButton>("ID_BUTTON_REPORT");
#ifdef PRE_MOD_CHATBG
	m_pChatBoardDlg = new CDnChatBoardDlg(UI_TYPE_BOTTOM);
	m_pChatBoardDlg->Initialize(false);
#endif
	m_pButtonReport->Show(false);

	m_pIMEEditBox->ForceRenderCandidateReadingWindowBottomPosition(true);

	SetChatModeText();

	m_pChatHelpDlg = new CDnChatHelpDlg(UI_TYPE_CHILD, this, CHAT_HELP_DIALOG, this);
	m_pChatHelpDlg->Initialize(false);

	m_pChatHelpDlg->AddChatHelpText(L"/s", L"/1", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 501), CHAT_NORMAL, chatcolor::NORMAL);
	m_pChatHelpDlg->AddChatHelpText(L"/p", L"/2", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 632), CHAT_PARTY, chatcolor::PARTY);
	m_pChatHelpDlg->AddChatHelpText(L"/g", L"/3", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 5007), CHAT_GUILD, chatcolor::GUILD);

	m_pChatHelpDlg->AddChatHelpText(L"/w", L"/4", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 648), CHAT_PRIVATE, chatcolor::PRIVATE);
#ifdef PRE_PRIVATECHAT_CHANNEL
	m_pChatHelpDlg->AddChatHelpText(L"/c", L"/5", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1202), CHAT_PRIVATE_CHANNEL, chatcolor::PRIVATECHANNEL);
#endif // PRE_PRIVATECHAT_CHANNEL
	m_bPrivateModeChecker = false;

#ifdef PRE_ADD_WORLD_MSG_RED
	m_pWorldChatRedBoard = GetControl<CEtUIStatic>("ID_STATIC_WORLDCHATBOARD");
	m_pWorldChatRed = GetControl<CEtUITextBox>("ID_TEXTBOX_WORLDCHAT");
	SUIControlProperty* pProperty = m_pWorldChatRed->GetProperty();
	pProperty->TextBoxProperty.bVerticalScrollBar = false;
#endif // PRE_ADD_WORLD_MSG_RED
}

bool CDnChatTabDlg::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_hWnd = hWnd;

	if (IsShow())
	{
		if (uMsg == WM_KEYDOWN)
		{
			if (GetKeyState(VK_CONTROL) < 0)
			{
#ifdef PRE_PRIVATECHAT_CHANNEL
				WCHAR checkChar[] = { '1', '2', '3', '4', '5' };
#ifdef PRE_ADD_CHAT_RENEWAL				
				int chatMode[] = { CHAT_NORMAL, CHAT_PARTY, CHAT_GUILD, CHAT_PRIVATE_CHANNEL, CHAT_PRIVATE };
#else // PRE_ADD_CHAT_RENEWAL			
				int chatMode[] = { CHAT_NORMAL, CHAT_PARTY, CHAT_GUILD, CHAT_PRIVATE, CHAT_PRIVATE_CHANNEL };
#endif // PRE_ADD_CHAT_RENEWAL
#else // PRE_PRIVATECHAT_CHANNEL
				WCHAR checkChar[] = { '1', '2', '3', '4', '5', '6' };
#ifdef PRE_ADD_CHAT_RENEWAL				
				int chatMode[] = { CHAT_NORMAL, CHAT_PARTY, CHAT_GUILD, CHAT_CHANNEL, CHAT_PRIVATE, CHAT_WORLD };
#else // PRE_ADD_CHAT_RENEWAL				
				int chatMode[] = { CHAT_NORMAL, CHAT_PARTY, CHAT_GUILD, CHAT_PRIVATE, CHAT_CHANNEL, CHAT_WORLD };
#endif // PRE_ADD_CHAT_RENEWAL
#endif // PRE_PRIVATECHAT_CHANNEL

				_ASSERT(_countof(checkChar) == _countof(chatMode));

				int i = 0;
				for (; i < _countof(checkChar); ++i)
				{
					if (wParam == checkChar[i])
					{
						SetChatMode(chatMode[i]);
						return true;
					}
				}
			}

			switch (wParam)
			{
			case VK_TAB:
			{
				if (CDnMouseCursor::IsActive() && !CDnMouseCursor::GetInstance().IsShowCursor())
					break;

				if (GetChatMode() == CHAT_PRIVATE)
				{
					ChangePrivateName();
				}

				RequestFocus(m_pIMEEditBox);
				return true;
			}
			break;

			case VK_ESCAPE:
			case VK_RETURN:
			{
				if (IsShow() && m_pIMEEditBox->GetTextLength() <= 0)
				{
					ShowEx(false);
					return true;
				}
			}
			break;
			}
		}

		if (!m_bPVPLobbyChat)
		{
			// Adjust chat tab vertical size
			POINT MousePoint;
			MousePoint.x = short(LOWORD(lParam));
			MousePoint.y = short(HIWORD(lParam));

			float fMouseX, fMouseY;
			PointToFloat(MousePoint, fMouseX, fMouseY);

			if (uMsg == WM_MOUSEMOVE)
			{
				if (m_nSizeMode == SIZE_BASE)
				{
					if (m_pButtonDummy && m_pButtonDummy->IsInside(fMouseX, fMouseY))
					{
						// Additionally check if mouse/key was already pressed from elsewhere upon entering.
						if (!CDnMouseCursor::GetInstance().IsPushMouseButton(0))
						{
							m_nSizeMode = SIZE_OVER;
							CDnMouseCursor::GetInstance().SetCursorState(CDnMouseCursor::SIZE);
						}
					}
				}
				else if (m_nSizeMode == SIZE_OVER)
				{
					if (m_pButtonDummy && !m_pButtonDummy->IsInside(fMouseX, fMouseY))
					{
						m_nSizeMode = SIZE_BASE;
						CDnMouseCursor::GetInstance().SetCursorState(CDnMouseCursor::NORMAL);
					}
				}
				else if (m_nSizeMode == SIZE_DRAG)
				{
					// Adjust chat dialog according to y change.
					float fDelta = m_fBasisY - fMouseY;
					ResizeChatBox(fDelta);

					// Exception handling: check if mouse was pressed inside and released outside the game window.
					if (m_pButtonDummy && !m_pButtonDummy->IsInside(fMouseX, fMouseY))
					{
						if (!CDnMouseCursor::GetInstance().IsPushMouseButton(0))
						{
							m_nSizeMode = SIZE_BASE;
							CDnMouseCursor::GetInstance().SetCursorState(CDnMouseCursor::NORMAL);
						}
					}
				}
			}
			else if (uMsg == WM_LBUTTONDOWN)
			{
				if (IsMouseInDlg())
					SetRenderPriority(this, true);

				if (m_nSizeMode == SIZE_OVER)
				{
					m_nSizeMode = SIZE_DRAG;
					m_fBasisY = fMouseY;
				}

#ifdef PRE_ADD_WORLD_MSG_RED
				bool isInside4 = false;
				if (m_fWorldMsgRedCoolTime > 0.0f && !m_bPVPLobbyChat && m_pWorldChatRed)
				{
					float fPTMouseX, fPTMouseY;
					PointToFloat(MousePoint, fPTMouseX, fPTMouseY);
					if (m_pWorldChatRed->IsInside(fPTMouseX, fPTMouseY))
						m_pWorldChatRedBoard->Show(true);
				}
#endif // PRE_ADD_WORLD_MSG_RED
			}
			else if (uMsg == WM_LBUTTONUP)
			{
				if (m_nSizeMode == SIZE_DRAG)
				{
					if (m_pButtonDummy && m_pButtonDummy->IsInside(fMouseX, fMouseY))
						m_nSizeMode = SIZE_OVER;
					else
					{
						m_nSizeMode = SIZE_BASE;
						CDnMouseCursor::GetInstance().SetCursorState(CDnMouseCursor::NORMAL);
					}
				}
			}
		}
	}

	if (!IsShow() && !m_bPassMessageToChild)
	{
		return false;
	}

#ifdef PRE_FIX_57852
	bool bExeMsgProc = false;
	bool bNeedFocusEditBox = false;
	if (IsShow())
	{
		bExeMsgProc = true;
	}
	else
	{
#if defined(PRE_ADD_68286)
		bool isAlarmIconClick = false;
#endif // PRE_ADD_68286

		if (uMsg == WM_LBUTTONDOWN)
		{
			POINT MousePoint;
			MousePoint.x = short(LOWORD(lParam));
			MousePoint.y = short(HIWORD(lParam));

			float fMouseX, fMouseY;
			//PointToFloat( MousePoint, fMouseX, fMouseY );

			fMouseX = MousePoint.x / GetScreenWidth();
			fMouseY = MousePoint.y / GetScreenHeight();

			//////////////////////////////////////////////////////////////////////////
			bool isInside2 = false;
			bool isInside1 = false;
			bool isInside3 = false;

			SUICoord normalDlgCoord;
			SUICoord optDlgCoord;

			CDnChatOptDlg &chatOptDlg = m_pChatNormalDlg->GetChatOptDlg();

			m_pChatNormalDlg->GetDlgCoord(normalDlgCoord);
			normalDlgCoord.fX = m_pChatNormalDlg->GetXCoord();
			normalDlgCoord.fY = m_pChatNormalDlg->GetYCoord();
			isInside1 = normalDlgCoord.IsInside(fMouseX, fMouseY);

			chatOptDlg.GetDlgCoord(optDlgCoord);
			optDlgCoord.fX = chatOptDlg.GetXCoord();
			optDlgCoord.fY = chatOptDlg.GetYCoord();
			if (chatOptDlg.IsShow() && optDlgCoord.IsInside(fMouseX, fMouseY))
				isInside1 = true;
			//////////////////////////////////////////////////////////////////////////

#if defined(PRE_MOD_CHATBG)
			SUICoord chatBoardCoord;
			if (m_pChatBoardDlg)
			{
				m_pChatBoardDlg->GetDlgCoord(chatBoardCoord);
				chatBoardCoord.fX = m_pChatBoardDlg->GetXCoord();
				chatBoardCoord.fY = m_pChatBoardDlg->GetYCoord();
				isInside2 = chatBoardCoord.IsInside(fMouseX, fMouseY);
			}

#endif // PRE_MOD_CHATBG
			if (m_pIMEEditBox)
				isInside3 = m_pIMEEditBox->IsInside(fMouseX, fMouseY);

#if defined(PRE_ADD_68286)
			if (GetInterface().IsAlarmIconClick(MousePoint) == true)
			{
				isInside1 = isInside2 = isInside3 = false;
				isAlarmIconClick = true;
			}
#endif // PRE_ADD_68286

#ifdef PRE_ADD_WORLD_MSG_RED
			bool isInside4 = false;
			if (m_fWorldMsgRedCoolTime > 0.0f && !m_bPVPLobbyChat && m_pWorldChatRed)
			{
				float fPTMouseX, fPTMouseY;
				PointToFloat(MousePoint, fPTMouseX, fPTMouseY);
				isInside4 = m_pWorldChatRed->IsInside(fPTMouseX, fPTMouseY);
			}

			if (isInside1 || isInside2 || isInside3 || isInside4)
#else // PRE_ADD_WORLD_MSG_RED
			if (isInside1 || isInside2 || isInside3)
#endif // PRE_ADD_WORLD_MSG_RED
			{
				ShowEx(true);
				bNeedFocusEditBox = true;
				bExeMsgProc = true;
			}
		}

		//For handling mouse events at the bottom-right of the PvP chat window
		if (uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONUP || uMsg == WM_LBUTTONDOWN)
		{
			//Right area in PvP lobby chat
			if (m_bPVPLobbyChat && m_bPvPLobbyChatOtherAreaInside)
			{
				bNeedFocusEditBox = false;
				bExeMsgProc = true;
			}
		}

#if defined(PRE_ADD_68286)
		if (isAlarmIconClick == true)
		{
			bNeedFocusEditBox = false;
			bExeMsgProc = false;
		}
#endif // PRE_ADD_68286
	}

	if (bExeMsgProc)
	{
		bool bMsgRet = CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
		if (bNeedFocusEditBox)
			RequestFocus(m_pIMEEditBox);

		return bMsgRet;
	}
	else
		return false;
#else
	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
#endif
}

void CDnChatTabDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	CDnChatDlg *pChatDlg = static_cast<CDnChatDlg*>(m_groupTabDialog.GetShowDialog());

	if (nCommand == EVENT_RADIOBUTTON_CHANGED)
	{
		if (pChatDlg)
		{
			pChatDlg->ShowOptDlg(false);
		}
	}

	CEtUITabDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);

	pChatDlg = static_cast<CDnChatDlg*>(m_groupTabDialog.GetShowDialog());
	if (!pChatDlg)
		return;

	if (nCommand == EVENT_EDITBOX_KEYUP)
	{
		SetNextChatHistory();
		return;
	}

	if (nCommand == EVENT_EDITBOX_KEYDOWN)
	{
		SetPrevChatHistory();
		return;
	}

	if (nCommand == EVENT_EDITBOX_SHIFT_KEYUP)
	{
		SetChatNextMode();
		return;
	}

	if (nCommand == EVENT_EDITBOX_SHIFT_KEYDOWN)
	{
		SetChatPrevMode();
		return;
	}

	if (nCommand == EVENT_EDITBOX_LCTRL_KEYUP)
	{
		// Switch tab
		int nNextTab = GetCurrentTabID() + 1;
		if (nNextTab > 4) nNextTab = 1;
		SetCheckedTab(nNextTab);
		return;
	}

	if (nCommand == EVENT_EDITBOX_LCTRL_KEYDOWN)
	{
		// Switch tab
		int nNextTab = GetCurrentTabID() - 1;
		if (nNextTab < 1) nNextTab = 4;
		SetCheckedTab(nNextTab);
		return;
	}

	if (nCommand == EVENT_EDITBOX_STRING)
	{
		if (!m_bSetFocusEditBox)
		{
			if (AddEditBoxString())
			{
				ShowEx(false);
			}
		}
		else
		{
			m_bSetFocusEditBox = false;
		}

		return;
	}

	if (nCommand == EVENT_EDITBOX_ESCAPE)
	{
		ShowEx(false);
		return;
	}

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_BUTTON_MODE_UP"))
		{
			SetChatNextMode();
			return;
		}

		if (IsCmdControl("ID_BUTTON_MODE_DOWN"))
		{
			SetChatPrevMode();
			return;
		}

		if (IsCmdControl("ID_BUTTON_OPTION"))
		{
			bool bShow = pChatDlg->IsShowOptDlg();
			pChatDlg->ShowOptDlg(!bShow);
			m_bShowOptDlg = !bShow;
			return;
		}

		if (IsCmdControl("ID_BUTTON_HELP"))
		{
			bool bShow = m_pChatHelpDlg->IsShow();
			ShowChildDialog(m_pChatHelpDlg, !bShow);
			return;
		}

		if (IsCmdControl("ID_BUTTON_REPORT"))
		{
			return;
		}
	}

	if (nCommand == EVENT_RADIOBUTTON_CHANGED)
	{
		if (pChatDlg->GetChatType() != CHAT_SYSTEM)
		{
#ifdef PRE_ADD_CHAT_RENEWAL
			m_bChangeChatMode = true;
#endif
			SetChatMode(pChatDlg->GetChatType());
		}

		if (m_bShowOptDlg)
		{
			pChatDlg->ShowOptDlg(true);
		}

#ifdef PRE_ADD_CHAT_RENEWAL
		// #65714 Scrollbar appearing in chat window when joining a party after selecting auto-switch to party chat.
		if (IsShow())
#endif 
			pChatDlg->ShowScrollBar(true);

		RequestFocus(m_pIMEEditBox);
		return;
	}
#ifdef PRE_ADD_WORLD_MSG_RED
	if (!m_bPVPLobbyChat && nCommand == EVENT_TEXTBOX_SELECTION)
	{
		if (IsCmdControl("ID_TEXTBOX_WORLDCHAT"))
		{
			CWord selectedWord = m_pWorldChatRed->GetSelectedWordData();
			if (selectedWord.m_strWordWithTag.empty() == false)
				EtInterface::GetNameLinkMgr().TranslateText(std::wstring(), selectedWord.m_strWordWithTag.c_str(), this);
		}
	}
#endif // PRE_ADD_WORLD_MSG_RED
}

#ifdef PRE_ADD_WORLD_MSG_RED
bool CDnChatTabDlg::OnParseTextItemInfo(const std::wstring& argString)
{
	CDnNameLinkMng* pMgr = GetInterface().GetNameLinkMng();
	if (pMgr)
	{
		CDnItem* pLinkItem = pMgr->MakeItem(argString);
		if (pLinkItem)
		{
			CDnTooltipDlg* pToolTipDlg = GetInterface().GetNameLinkToolTipDlg();
			if (pToolTipDlg)
			{
				ITEM_SLOT_TYPE slotType = ST_ITEM_NONE;
				if (pLinkItem->GetType() == MIInventoryItem::Item)
					slotType = ST_INVENTORY;
				else if (pLinkItem->GetType() == MIInventoryItem::Skill)
					slotType = ST_SKILL;
				pToolTipDlg->ShowTooltip(pLinkItem, pLinkItem->GetType(), slotType, GetScreenWidthRatio() * 0.5f, GetScreenHeightRatio() * 0.83f, true);
			}
		}
	}

	return true;
}
#endif // PRE_ADD_WORLD_MSG_RED

void CDnChatTabDlg::SetChatGameMode(bool isPVP)
{
	m_IsPVPGameChat = isPVP;

	if (m_IsPVPGameChat)
	{
		if (m_pPartyTab)
		{
#ifdef PRE_FIX_COLOCHAT_ICON
#else
			m_pPartyTab->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 121018));
#endif
			SetChatMode(CHAT_PARTY);
			//SetChatModeText();
			SetCheckedTab(m_pPartyTab->GetTabID());
			ShowEx(true);
			ShowEx(false);
		}
	}
#ifdef PRE_FIX_COLOCHAT_ICON
#else
	else
	{
		if (m_pPartyTab)
			m_pPartyTab->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 632));

	}
#endif
}

void CDnChatTabDlg::Render(float fElapsedTime)
{
	if (IsShow())
		CheckCommandMode();

	CEtUIDialog::Render(fElapsedTime);
}

void CDnChatTabDlg::ResizeChatBox(float fHeightDelta)
{
	// Set the size arbitrarily for now. Needs to be changed later.
	if (m_DlgInfo.DlgCoord.fHeight + fHeightDelta < 0.24f || m_DlgInfo.DlgCoord.fHeight + fHeightDelta > 0.6f)
		return;

	UpdateDlgCoord(0, 0, 0, fHeightDelta);
	SetPosChatDlg(fHeightDelta);
	SetPosBottomCtl(fHeightDelta);
#ifdef PRE_MOD_CHATBG
	if (m_pChatBoardDlg)
		m_pChatBoardDlg->UpdateDlgCoord(0, 0, 0, fHeightDelta);
#endif
}

void CDnChatTabDlg::ResizeIMECCtl(LPCWSTR wszHeader)
{
	DWORD dwTextColor(chatcolor::NORMAL);

	switch (GetChatMode())
	{
	case CHAT_NORMAL:	dwTextColor = chatcolor::NORMAL;	break;
	case CHAT_PRIVATE:	dwTextColor = chatcolor::PRIVATE;	break;
	case CHAT_PARTY:	dwTextColor = chatcolor::PARTY;		break;
	case CHAT_GUILD:	dwTextColor = chatcolor::GUILD;		break;
	case CHAT_CHANNEL:	dwTextColor = chatcolor::CHANNEL;	break;
	case CHAT_WORLD:	dwTextColor = chatcolor::WORLD;		break;
#ifdef PRE_PRIVATECHAT_CHANNEL
	case CHAT_PRIVATE_CHANNEL:	dwTextColor = chatcolor::PRIVATECHANNEL;	break;
#endif // PRE_PRIVATECHAT_CHANNEL
	default:
		ASSERT(0 && "CDnChatTabDlg::ResizeIMECCtl");
		break;
	}

	wchar_t wszMsg[256] = { 0 };

	if (m_bCustomizeChatMode && m_nCustomizeModeType == GetChatMode())
	{
		swprintf_s(wszMsg, 256, L"[%s]:", m_wszCustomizeModeHeader);

		m_pIMEEditBox->SetTextColor(m_dwCustomizeModeTextColor);
		m_pIMEEditBox->SetCompTextColor(m_dwCustomizeModeTextColor);
		m_pStaticMode->SetTextColor(m_dwCustomizeModeTextColor);

		m_pStaticMode->SetText(wszMsg);
	}
	else
	{
		m_pIMEEditBox->SetTextColor(dwTextColor);
		m_pIMEEditBox->SetCompTextColor(dwTextColor);
		m_pStaticMode->SetTextColor(dwTextColor);

		m_pStaticMode->SetText(wszHeader);
	}

	SUICoord sTextCoord;
	CalcTextRect(wszHeader, m_pStaticMode->GetElement(0), sTextCoord);
	sTextCoord.fWidth += 0.001f;

	SUICoord sCtlCoord;
	m_pStaticMode->GetUICoord(sCtlCoord);
	float fTemp = sTextCoord.fWidth - sCtlCoord.fWidth;
	sCtlCoord.fWidth = sTextCoord.fWidth;
	m_pStaticMode->SetUICoord(sCtlCoord);

	m_pIMEEditBox->GetUICoord(sCtlCoord);
	sCtlCoord.fX += fTemp;
	sCtlCoord.fWidth -= fTemp;
	m_pIMEEditBox->SetUICoord(sCtlCoord);
}

void CDnChatTabDlg::OnChangeResolution()
{
	CEtUIDialog::OnChangeResolution();
	SetChatModeText();
}

void CDnChatTabDlg::SetPosChatDlg(float fResizeRatio)
{
	int nVecSize = (int)m_vecChatDlg.size();
	for (int i = 0; i<nVecSize; i++)
	{
		if (m_vecChatDlg[i])
		{
			m_vecChatDlg[i]->UpdateDlgCoord(0, 0, 0, fResizeRatio);
		}
	}

	m_pChatHelpDlg->UpdateDlgCoord(0, -fResizeRatio, 0, 0);
}

void CDnChatTabDlg::SetPosBottomCtl(float fResizeRatio)
{
	m_pButtonModeUp->UpdateUICoord(0, fResizeRatio, 0, 0);
	m_pButtonModeDown->UpdateUICoord(0, fResizeRatio, 0, 0);
	m_pStaticMode->UpdateUICoord(0, fResizeRatio, 0, 0);
	m_pIMEEditBox->UpdateUICoord(0, fResizeRatio, 0, 0);
	m_pButtonReport->UpdateUICoord(0, fResizeRatio, 0, 0);
}

void CDnChatTabDlg::CheckCommandMode()
{
	std::wstring strCmd;
	COMMANDMODE_MAP_ITER iter;
	m_bPrivateModeChecker = false;

	strCmd = m_pIMEEditBox->GetText();

	std::wstring::size_type idx = strCmd.find_first_of(L" ");
	if (idx != std::wstring::npos)
	{
		std::wstring checkStr = strCmd.substr(0, idx + 1);

		iter = m_mapCommandMode.find(checkStr);
		if (iter != m_mapCommandMode.end())
		{
			CDnChatDlg *pChatDlg = static_cast<CDnChatDlg*>(m_groupTabDialog.GetShowDialog());
			if (pChatDlg)
			{
				int chatType = iter->second;
				if (chatType & CHAT_PRIVATE_REPLY)
				{
					std::wstring strLastName;
					if (GetLastPrivateName(strLastName))
					{
						SetPrivateName(strLastName);
					}
					return;
				}

				if (chatType & CHAT_PARTY)
				{
					CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask("GameTask");
					if (pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm)
						return;
				}

				CDnChatOptDlg &chatOptDlg = pChatDlg->GetChatOptDlg();
				int nOption = chatOptDlg.GetOption();

				// Force change mode.
				// In party tab with Normal unchecked, typing /s switches to Normal tab. Is this needed..
				if (!(nOption & chatType))
				{
					switch (chatType)
					{
					case CHAT_NORMAL:	SetCheckedTab(m_pNormalTab->GetTabID());	break;
					case CHAT_PARTY:	SetCheckedTab(m_pPartyTab->GetTabID());	break;
					case CHAT_GUILD:	SetCheckedTab(m_pGuildTab->GetTabID());	break;
#ifdef PRE_ADD_CHAT_RENEWAL
					case CHAT_PRIVATE:	SetCheckedTab(m_pWhisperTab->GetTabID());	break;
#else
					case CHAT_PRIVATE:	SetCheckedTab(m_pNormalTab->GetTabID());	break;
#endif
#ifdef PRE_PRIVATECHAT_CHANNEL
					case CHAT_PRIVATE_CHANNEL:	SetCheckedTab(m_pPrivateChannelTab->GetTabID()); break;
#endif // PRE_PRIVATECHAT_CHANNEL
					case CHAT_CHANNEL:	SetCheckedTab(m_pNormalTab->GetTabID());	break;
					case CHAT_WORLD:	SetCheckedTab(m_pNormalTab->GetTabID());	break;
					}
				}

				if (chatType == CHAT_PRIVATE)
				{
					m_bPrivateModeChecker = true;
				}
				else
				{
					SetChatMode(chatType);
					m_pIMEEditBox->ClearText();
					RequestFocus(m_pIMEEditBox);
				}
			}
		}

		if (m_bPrivateModeChecker)
		{
			// Note : Do not change mode until opponent name is entered.
			//
			std::wstring::size_type begIdx, endIdx;

			begIdx = strCmd.find_first_of(L" ");
			begIdx = (begIdx != std::wstring::npos) ? ++begIdx : std::wstring::npos;
			endIdx = strCmd.find_first_of(L" ", begIdx);

			if (endIdx != std::wstring::npos)
			{
				std::wstring strPrivateUserName = strCmd.substr(begIdx, endIdx - begIdx);
				EtInterface::GetNameLinkMgr().TranslateText(m_strPrivateUserName, strPrivateUserName.c_str());
				if ((int)m_strPrivateUserName.size() > CGlobalInfo::GetInstance().m_nClientCharNameLenMax)
					m_strPrivateUserName.resize(CGlobalInfo::GetInstance().m_nClientCharNameLenMax);

				SetChatMode(CHAT_PRIVATE);
				m_pIMEEditBox->ClearText();
				RequestFocus(m_pIMEEditBox);

				m_bPrivateModeChecker = false;
			}
		}
	}
}

bool CDnChatTabDlg::CheckCommandMode(LPCWSTR wszMsg)
{
	std::wstring strCmd(wszMsg);
	if (strCmd.size() < 2) return false;
	if ((strCmd.size() > 2) && (strCmd[2] != L' ')) return false;

	strCmd = strCmd.substr(0, 2);
	strCmd += L" ";
	COMMANDMODE_MAP_ITER iter = m_mapCommandMode.find(strCmd);

	if (iter != m_mapCommandMode.end())
	{
		if (iter->second&CHAT_PRIVATE_REPLY)
		{
			std::wstring strLastName;
			if (GetLastPrivateName(strLastName))
			{
				SetPrivateName(strLastName);
			}
			return true;
		}

		if (iter->second & CHAT_PARTY)
		{
			CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask("GameTask");
			if (pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm)
				return false;
		}

		CDnChatDlg *pChatDlg = static_cast<CDnChatDlg*>(m_groupTabDialog.GetShowDialog());
		if (pChatDlg)
		{
			CDnChatOptDlg &chatOptDlg = pChatDlg->GetChatOptDlg();
			int nOption = chatOptDlg.GetOption();

			if (!(nOption&iter->second))
			{
				switch (iter->second)
				{
				case CHAT_NORMAL:	SetCheckedTab(m_pNormalTab->GetTabID());	break;
#ifdef PRE_ADD_CHAT_RENEWAL
				case CHAT_PRIVATE:	SetCheckedTab(m_pWhisperTab->GetTabID());	break;
#else
				case CHAT_PRIVATE:	SetCheckedTab(m_pNormalTab->GetTabID());	break;
#endif
#ifdef PRE_PRIVATECHAT_CHANNEL
				case CHAT_PRIVATE_CHANNEL:	SetCheckedTab(m_pPrivateChannelTab->GetTabID()); break;
#endif // PRE_PRIVATECHAT_CHANNEL
				case CHAT_PARTY:	SetCheckedTab(m_pPartyTab->GetTabID());	break;
				case CHAT_GUILD:	SetCheckedTab(m_pGuildTab->GetTabID());	break;
				case CHAT_CHANNEL:	SetCheckedTab(m_pNormalTab->GetTabID());	break;
				case CHAT_WORLD:	SetCheckedTab(m_pNormalTab->GetTabID());	break;
				}
			}

			if (iter->second == CHAT_PRIVATE)
			{
				strCmd = wszMsg;
				std::wstring::size_type begIdx = strCmd.find_first_not_of(L" ", 2);
				if (begIdx != std::wstring::npos)
				{
					std::wstring strPrivateUserName = strCmd.substr(begIdx);
					EtInterface::GetNameLinkMgr().TranslateText(m_strPrivateUserName, strPrivateUserName.c_str());
					if ((int)m_strPrivateUserName.size() > CGlobalInfo::GetInstance().m_nClientCharNameLenMax)
					{
						m_strPrivateUserName.resize(CGlobalInfo::GetInstance().m_nClientCharNameLenMax);
					}
				}
			}

			SetChatMode(iter->second);
			m_pIMEEditBox->ClearText();
			RequestFocus(m_pIMEEditBox);
		}

		return true;
	}

	return false;
}

void CDnChatTabDlg::SetChatPrevMode()
{
	// Unlike before, since it is not affected by options,
	int nCurChatMode = GetChatMode();
	while (true)
	{
		nCurChatMode >>= 1;

		if (nCurChatMode == 0)
			nCurChatMode = CHAT_WORLD;

		if (SetChatMode(nCurChatMode))
		{
			RequestFocus(m_pIMEEditBox);
			break;
		}
	}
}

void CDnChatTabDlg::SetChatNextMode()
{
	// Unlike before, since it is not affected by options,
	int nCurChatMode = GetChatMode();
	while (true)
	{
		nCurChatMode <<= 1;

		if (nCurChatMode >= CHAT_MODE_MAX)
			nCurChatMode = CHAT_NORMAL;

		if (SetChatMode(nCurChatMode))
		{
			RequestFocus(m_pIMEEditBox);
			break;
		}
	}
}

void CDnChatTabDlg::SetChatModeText()
{
	if (!m_pStaticMode)
		return;

	wchar_t wszMsg[256] = { 0 };

	switch (m_nChatMode)
	{
	case CHAT_NORMAL:
		swprintf_s(wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 501));
		break;
	case CHAT_PARTY:
	{
		if (m_IsPVPGameChat)
			swprintf_s(wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 121018));
		else
			swprintf_s(wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 632));

	}

	break;
	case CHAT_GUILD:
		swprintf_s(wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 5007));
		break;
	case CHAT_PRIVATE:
	{
		swprintf_s(wszMsg, 256, L"[%s][%s]:", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 648), m_strPrivateUserName.c_str());

		if (!m_strPrivateUserName.empty())
		{
			AddPrivateName(m_strPrivateUserName);
		}
	}
	break;
	case CHAT_CHANNEL:
		swprintf_s(wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 605));
		break;
	case CHAT_WORLD:
		swprintf_s(wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 606));
		break;
#ifdef PRE_PRIVATECHAT_CHANNEL
	case CHAT_PRIVATE_CHANNEL:
		swprintf_s(wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1202));
		break;
#endif // PRE_PRIVATECHAT_CHANNEL
	}

	if (IsCustomizeChatMode() && GetChatMode() == m_nCustomizeModeType)
	{
		swprintf_s(wszMsg, 256, L"[%s]:", m_wszCustomizeModeHeader);
		ResizeIMECCtl(wszMsg);
	}
	else
	{
		ResizeIMECCtl(wszMsg);
	}
}

void CDnChatTabDlg::ResetChatMode()
{
	m_nChatMode = CHAT_NORMAL;

	SetChatModeText();

	SetCheckedTab(m_pNormalTab->GetTabID());
	ShowEx(true);
	ShowEx(false);
}

bool CDnChatTabDlg::SetChatMode(int nChatMode, bool bParty)
{
	if (nChatMode == CHAT_PARTY)
	{
		CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask("GameTask");
		if (pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm)
			return false;

		// If not in a party (must switch because it's used as team chat in PVP)
		if (GetPartyTask().GetPartyRole() == CDnPartyTask::SINGLE && !m_IsPVPGameChat)
			return false;
	}
	else if (nChatMode == CHAT_GUILD)
	{
		// If not joined a guild,
		if (CDnActor::s_hLocalActor) {
			CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

			if (!pPlayer->IsJoinGuild()
				)
			{
				return false;
			}
		}
	}
	else if (nChatMode == CHAT_PRIVATE)
	{
#ifdef PRE_ADD_CHAT_RENEWAL
		// If there is no whisper target
		if (GetPrivateName() == L"")
		{
			std::wstring strLastName;
			if (GetLastPrivateName(strLastName))
				m_strPrivateUserName = strLastName;
			else
				return false;
		}
#else
		// If there is no whisper target
		if (GetPrivateName() == L"")
			return false;
#endif // PRE_ADD_CHAT_RENEWAL
	}
	else if (nChatMode == CHAT_CHANNEL)
	{
		// Disable feature for CBT first.
		return false;
	}
	else if (nChatMode == CHAT_WORLD)
	{
		return false;
	}
#ifdef PRE_PRIVATECHAT_CHANNEL
	else if (nChatMode == CHAT_PRIVATE_CHANNEL)
	{
		if (GetChannelChatTask().IsActive() && !GetChannelChatTask().IsJoinChannel())
		{
			return false;
		}
	}
#endif // PRE_PRIVATECHAT_CHANNEL

#ifdef PRE_ADD_CHAT_RENEWAL	
	// Auto-switch and party state.
	if (CGameOption::IsActive() && CGameOption::GetInstance().cPartyChatAcceptable == 1 && CDnPartyTask::IsActive() &&
		(CDnPartyTask::PartyRoleEnum::LEADER == GetPartyTask().GetPartyRole() ||
			CDnPartyTask::PartyRoleEnum::MEMBER == GetPartyTask().GetPartyRole())
		)
	{
		// Party create or join - when called after joining party, switch tab to normal and set chat mode to party.
		if (bParty)
		{
			nChatMode = CHAT_PARTY;
			SetCheckedTab(m_pNormalTab->GetTabID());
			CDnChatDlg * pChatDlg = static_cast<CDnChatDlg*>(m_groupTabDialog.GetShowDialog());
			if (pChatDlg && pChatDlg->IsShowScrollBar())
				pChatDlg->ShowScrollBar(false);
		}

		// Switch by tab click - if normal tab, set chat mode to party state.
		else if (m_bChangeChatMode && nChatMode == CHAT_NORMAL)
		{
			m_bChangeChatMode = false;
			nChatMode = CHAT_PARTY;
		}
	}
#endif

	m_nChatMode = nChatMode;
	SetChatModeText();
	return true;
}

bool CDnChatTabDlg::AddEditBoxString()
{
	std::wstring strChat = m_pIMEEditBox->GetText();
	strChat.reserve(CHATLENMAX);

	// If pressing Enter with empty input should deactivate the chat box,
	// Uncomment the two lines below and change return true at the bottom to return false.
	//if( strChat.empty() )
	//	return true;

	std::wstring chatTemp;
	chatTemp = boost::algorithm::trim_copy(strChat);
	if (chatTemp.empty())
		return true;

	if (CheckCommandMode(strChat.c_str()))
		return false;

	// Must store the command before processing so it can be re-entered using history.
	AddChatHistotry(strChat.c_str());

	// Return true so input box loses focus when handled as a valid command.
	if (CommandMessage(strChat.c_str()))
		return true;

	// Check chat restriction here.
	if (GetRestraintTask().CheckRestraint(_RESTRAINTTYPE_CHAT, true))
		return true;

	// Gestures also have chat commands.
	// Unlike other commands, gestures do not cause focus loss since the user is just chatting.
	GetGestureTask().UseGestureByChat(strChat.c_str());

	// Do not Send messages starting with / due to GM commands.
	// Commands like set_user_level and makeitem are not handled in DebugCmdProc, so we have no choice but to send them directly to the server.
	// So check this only in Final_build.
#ifdef _FINAL_BUILD
	CDnLocalPlayerActor* pLocalPlayer = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	if (pLocalPlayer)
	{
		if (strChat[0] == L'/' && CDnActor::s_hLocalActor && pLocalPlayer->IsDeveloperAccountLevel() == false)
		{
			_AddChatMessage(m_nChatMode, CDnActor::s_hLocalActor->GetName(), strChat.c_str());

			// If it was a valid command it would be handled above; otherwise return false so chat does not lose focus.
			return false;
		}
	}
#endif

#ifdef PRE_ADD_ANTI_CHAT_SPAM
	// #47642 Restrict only in normal (all) mode chat.
	if (CHAT_NORMAL == m_nChatMode)
	{
		// US anti-spam penalty.	
		bool bSpammer = GetRestraintTask().CheckUSAChatSpam(strChat);
		if (bSpammer)
		{
			_AddChatMessage(m_nChatMode, CDnActor::s_hLocalActor->GetName(), strChat.c_str());
			if (!m_bSpammer)
			{
				//Send to the server for now.
				SendChatSpammer(true);
				m_bSpammer = true;
			}
			//m_pIMEEditBox->ClearText();
			//return false;
		}
		else if (m_bSpammer)
		{
			SendChatSpammer(false);
			m_bSpammer = false;
		}
	}
#endif // #ifdef PRE_ADD_ANTI_CHAT_SPAM

	SendChat(m_nChatMode, strChat, m_strPrivateUserName);

	m_pIMEEditBox->ClearText();

	return true;
}

#ifdef PRE_CHAT_FIX_DICE_PAPERING
bool CDnChatTabDlg::HandleChatPapering(const std::wstring& chatMsg)
{
	// Do not check spam in party or guild chat mode.
	if (m_nChatMode != CHAT_PARTY && m_nChatMode != CHAT_GUILD)
	{
		// If the message starts with '/', do not apply the spam prevention feature.
		bool bCheckPapering = true;
		if (chatMsg[0] == L'/')
		{
			std::wstring::size_type firstBlank = chatMsg.find_first_of(L" ");
			std::wstring subStr = chatMsg.substr(1, firstBlank - 1);
			bCheckPapering = (subStr.compare(L"dice") == 0
#if defined(_RDEBUG)
				|| subStr.compare(L"Dice") == 0
#endif
				);
		}
		if (bCheckPapering && CheckPapering())
		{
			wchar_t wszMsg[256] = { 0 };
			int nTime = (int)m_fPaperingRemainTime;
			swprintf_s(wszMsg, 256, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 672), nTime);

			// Handled this way since option check doesn't apply and color is completely separate.
			CDnChatDlg *pDlg = NULL;
			if (m_pNormalTab->GetTabID() == GetCurrentTabID())			pDlg = m_pChatNormalDlg;
#ifdef PRE_ADD_CHAT_RENEWAL
			else if (m_pWhisperTab->GetTabID() == GetCurrentTabID())   pDlg = m_pChatWhisperDlg;
#endif
#ifdef PRE_PRIVATECHAT_CHANNEL
			else if (m_pPrivateChannelTab->GetTabID() == GetCurrentTabID())	pDlg = m_pPrivateChannelDlg;
#endif // PRE_PRIVATECHAT_CHANNEL
			else if (m_pPartyTab->GetTabID() == GetCurrentTabID())		pDlg = m_pChatPartyDlg;
			else if (m_pGuildTab->GetTabID() == GetCurrentTabID())		pDlg = m_pChatGuildDlg;
			else if (m_pSystemTab->GetTabID() == GetCurrentTabID())	pDlg = m_pChatSystemDlg;
			if (pDlg)
			{
				pDlg->AddChat(NULL, L"", 0, UITEXT_NONE, false, false, -1, 0, true);
				pDlg->AddChat(NULL, wszMsg, 0, UITEXT_NONE, true, true, textcolor::RED);
			}
			m_pIMEEditBox->ClearText();
			return true;
		}
	}

	return false;
}
#endif // PRE_CHAT_FIX_DICE_PAPERING

void CDnChatTabDlg::SendChat(int chatMode, std::wstring& chatMsg, const std::wstring& userName)
{
	// 5-repeat check is reportedly no longer used.
	// Will delete completely later when it is truly no longer needed.
	//if( CheckRepeat() )
	//{
	//	wchar_t wszMsg[256]={0};
	//	swprintf_s( wszMsg, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 670) );
	//	AddChatDlg( wszMsg, CHAT_SYSTEM );
	//	m_pIMEEditBox->ClearText();
	//	return;
	//}

#ifdef PRE_CHAT_FIX_DICE_PAPERING
	if (HandleChatPapering(chatMsg))
		return;
#else
	// Do not check spam in party or guild chat mode.
#ifdef PRE_PRIVATECHAT_CHANNEL
	if (m_nChatMode != CHAT_PARTY && m_nChatMode != CHAT_GUILD && m_nChatMode != CHAT_PRIVATE_CHANNEL)
#else // PRE_PRIVATECHAT_CHANNEL
	if (m_nChatMode != CHAT_PARTY && m_nChatMode != CHAT_GUILD)
#endif // PRE_PRIVATECHAT_CHANNEL
	{
		// If the message starts with '/', do not apply the spam prevention feature.
		bool bCheckPapering = true;
		if (chatMsg[0] == L'/') bCheckPapering = false;
		if (bCheckPapering && CheckPapering())
		{
			wchar_t wszMsg[256] = { 0 };
			int nTime = (int)m_fPaperingRemainTime;
			swprintf_s(wszMsg, 256, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 672), nTime);

			// Handled this way since option check doesn't apply and color is completely separate.
			CDnChatDlg *pDlg = NULL;
			if (m_pNormalTab->GetTabID() == GetCurrentTabID())			pDlg = m_pChatNormalDlg;
#ifdef PRE_ADD_CHAT_RENEWAL
			else if (m_pWhisperTab->GetTabID() == GetCurrentTabID())   pDlg = m_pChatWhisperDlg;
#endif
#ifdef PRE_PRIVATECHAT_CHANNEL
			else if (m_pPrivateChannelTab->GetTabID() == GetCurrentTabID())	pDlg = m_pPrivateChannelDlg;
#endif // PRE_PRIVATECHAT_CHANNEL
			else if (m_pPartyTab->GetTabID() == GetCurrentTabID())		pDlg = m_pChatPartyDlg;
			else if (m_pGuildTab->GetTabID() == GetCurrentTabID())		pDlg = m_pChatGuildDlg;
			else if (m_pSystemTab->GetTabID() == GetCurrentTabID())	pDlg = m_pChatSystemDlg;
			if (pDlg)
			{
				pDlg->AddChat(NULL, L"", 0, UITEXT_NONE, false, false, -1, 0, true);
				pDlg->AddChat(NULL, wszMsg, 0, UITEXT_NONE, true, true, textcolor::RED);
			}
			m_pIMEEditBox->ClearText();
			return;
		}
	}
#endif	// PRE_CHAT_FIX_DICE_PAPERING

	switch (chatMode)
	{
	case CHAT_NORMAL:
	{
		bool bAdd = true;
		if (CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->IsPlayerActor())
		{
			CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if (pPlayer && pPlayer->IsObserver())
			{
				bAdd = false; // Observer mode does not add normal chat messages.
			}
		}
		if (bAdd)
		{
			SendChatMsg(CHATTYPE_NORMAL, chatMsg.c_str());

#if defined( PRE_ADD_CHAT_MISSION )
			GetMissionTask().RequestChatMission(chatMsg);
#endif	// #if defined( PRE_ADD_CHAT_MISSION )
		}
	}
	break;
	case CHAT_PRIVATE:
	{
		if (userName.empty())
			break;

		_AddChatMessage(CHAT_PRIVATE, userName.c_str(), chatMsg.c_str());
		SendChatPrivateMsg(userName.c_str(), chatMsg.c_str());
	}
	break;
	case CHAT_PARTY:
	{
		if (m_IsPVPGameChat)
			SendChatMsg(CHATTYPE_TEAMCHAT, chatMsg.c_str());
		else
			SendChatMsg(CHATTYPE_PARTY, chatMsg.c_str());
	}
	break;
	case CHAT_GUILD:
	{
		SendChatMsg(CHATTYPE_GUILD, chatMsg.c_str());
	}
	break;
	case CHAT_CHANNEL:
	{
		SendChatMsg(CHATTYPE_CHANNEL, chatMsg.c_str());
	}
	break;
	case CHAT_WORLD:
	{
		SendChatMsg(CHATTYPE_WORLD, chatMsg.c_str());
	}
	break;
#ifdef PRE_PRIVATECHAT_CHANNEL
	case CHAT_PRIVATE_CHANNEL:
	{
		SendChatMsg(CHATTYPE_PRIVATECHANNEL, chatMsg.c_str());
	}
	break;
#endif // PRE_PRIVATECHAT_CHANNEL
	}
}

bool CDnChatTabDlg::CommandMessage(LPCWSTR wszMsg)
{
	std::wstring strText(wszMsg);
	std::wstring strCommand;
	std::wstring strArg1;
	if (strText[0] == L'/')
	{
		bool bProcessed = false;

		// Pre-count the number of space-delimited arguments.
		// Since all commands currently process up to 2 tokens including the command itself,
		// Treat only 1 or 2 as valid.
		int nNumArg = 0;
		std::wstring::size_type endIdx, endIdx2;
		endIdx = strText.find_first_of(L" ");
		if (endIdx != std::wstring::npos)
		{
			strCommand = strText.substr(0, endIdx);
			endIdx2 = strText.find_first_of(L" ", endIdx + 1);
			if (endIdx2 != std::wstring::npos
				&& strCommand.compare(L"/y") != 0
				&& strCommand.compare(L"/Y") != 0
				&& strCommand.compare(L"/\u315B") != 0
				)
				nNumArg = 2;
			else
			{
				nNumArg = 1;
				strArg1 = strText.substr(endIdx + 1, endIdx2);
			}
		}
		else
		{
			strCommand = strText;
		}

		if (nNumArg == 0 || nNumArg == 1)
		{
			COMMANDMESSAGE_MAP_ITER iter;

			iter = m_mapCommandMessage.find(strCommand);
			if (iter != m_mapCommandMessage.end())
			{
				// If in blind state, ignore all commands except help.
				if (GetInterface().IsOpenBlind()
					|| GetInterface().IsShowCostumeMixDlg()
					)
				{
					if (iter->second != CMD_MSG_HELP)
					{
						AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 100301), CHAT_SYSTEM);
						m_pIMEEditBox->ClearText();
						return true;
					}
				}

				// When using a village-only feature outside of village
				// This feature is currently unavailable. Output message.
				if (CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeVillage)
				{
					if (iter->second == CMD_MSG_MAKEPARTY)
					{
						AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 100301), CHAT_SYSTEM);
						m_pIMEEditBox->ClearText();
						return true;
					}
				}

				if (CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeDungeon)
				{
					if (iter->second == CMD_MSG_INVITE)
					{
						AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 100301), CHAT_SYSTEM);
						m_pIMEEditBox->ClearText();
						return true;
					}
				}

				// Do not execute commands below if in Colosseum, PvP lobby, or PvP game.
				CDnVillageTask *pVillage = (CDnVillageTask *)CTaskManager::GetInstance().GetTask("VillageTask");
				if ((pVillage && pVillage->GetVillageType() == CDnVillageTask::PvPVillage) ||
					(pVillage && pVillage->GetVillageType() == CDnVillageTask::FarmVillage) ||
					m_bPVPLobbyChat ||
					m_IsPVPGameChat)
				{
					if (iter->second == CMD_MSG_INVITE || iter->second == CMD_MSG_MAKEPARTY || iter->second == CMD_MSG_LEAVEPARTY || iter->second == CMD_MSG_BAN ||
						iter->second == CMD_MSG_TRADE)
					{
						AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 100301), CHAT_SYSTEM);
						m_pIMEEditBox->ClearText();
						return true;
					}
				}

#ifdef PRE_PRIVATECHAT_CHANNEL
				if (iter->second != CMD_MSG_CHANNEL_CREATE && iter->second != CMD_MSG_CHANNEL_JOIN && iter->second != CMD_MSG_CHANNEL_PASSWORD && iter->second != CMD_MSG_CHANNEL_INVITE &&
					iter->second != CMD_MSG_CHANNEL_LEAVE && iter->second != CMD_MSG_CHANNEL_BAN && iter->second != CMD_MSG_CHANNEL_MASTER && iter->second != CMD_MSG_CHANNEL_HELP)
				{
					// If local actor cannot be retrieved, this is not a normal state.
					if (!CDnActor::s_hLocalActor)
						return true;

					// Disallow using commands while in ghost state.
					if (CDnActor::s_hLocalActor->IsDie())
					{
						wchar_t wszMsg[256] = { 0 };
						swprintf_s(wszMsg, 256, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 671));
						AddChatDlg(wszMsg, CHAT_SYSTEM);
						return true;
					}
				}
#else // PRE_PRIVATECHAT_CHANNEL
				// If local actor cannot be retrieved, this is not a normal state.
				if (!CDnActor::s_hLocalActor)
					return true;

				// Disallow using commands while in ghost state.
				if (CDnActor::s_hLocalActor->IsDie())
				{
					wchar_t wszMsg[256] = { 0 };
					swprintf_s(wszMsg, 256, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 671));
					AddChatDlg(wszMsg, CHAT_SYSTEM);
					return true;
				}
#endif // PRE_PRIVATECHAT_CHANNEL

				switch (iter->second)
				{
				case CMD_MSG_HELP:
				{
					// Output help
					if (nNumArg == 0)
					{
						const UINT nStartIndex = 112001;
						const UINT nEndIndex = 112999;
						WCHAR wszMessage[128];
						for (UINT i = nStartIndex; i < nEndIndex; ++i)
						{
							swprintf_s(wszMessage, 128, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, i));
							if (wszMessage[0] == '/')
							{
								AddChatDlg(wszMessage, CHAT_SYSTEM);
							}
						}
						bProcessed = true;
					}
				}
				break;
				case CMD_MSG_INVITE:
				{
					// Self invite - after party creation and becoming leader, target is already in party...
					// Offline user - cannot find user to invite.
					// User in another channel - invitation failed.
					if (nNumArg == 1)
					{
						GetPartyTask().ReqInviteParty(strArg1.c_str());
						bProcessed = true;
					}
				}
				break;
				case CMD_MSG_MAKEPARTY:
				{
					// Create party with default settings.
					if (nNumArg > 0 && CommonUtil::IsValidPartyNameLen((int)strArg1.length()) == false)
					{
						AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3534), CHAT_SYSTEM);
						m_pIMEEditBox->ClearText();
						bProcessed = true;
						break;
					}

#ifdef PRE_PARTY_DB
					SPartyCreateParam param;
					if (nNumArg > 0)
						param.wszPartyName = strArg1;
					else
						param.wszPartyName = FormatW(L"%s%s", CDnActor::s_hLocalActor->GetName(), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3535));
					GetPartyTask().ReqCreateParty(param);
#else
					std::wstring partyName;
					if (nNumArg > 0)
						partyName = strArg1;
					else
						partyName = FormatW(L"%s%s", CDnActor::s_hLocalActor->GetName(), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3535));

					GetPartyTask().ReqCreateParty(0, 4, 1, 100, partyName.c_str(), NULL, 0, 0, 0);
#endif
					bProcessed = true;
				}
				break;
				case CMD_MSG_LEAVEPARTY:
				{
					// Nothing happens if done without a party.
					if (nNumArg == 0)
					{
						GetPartyTask().ReqOutParty();
						bProcessed = true;
					}
				}
				break;
				case CMD_MSG_BAN:
				{
					// For kicking, need to check not only whether it is a village, but also whether inside a dungeon.
					// This feature is currently unavailable. Output message.
					if (CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeDungeon)
					{
						AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 100301), CHAT_SYSTEM);
						return true;
					}

					if (nNumArg == 1)
					{
						GetPartyTask().ReqPartyMemberKick(strArg1.c_str(), 0);
						bProcessed = true;
					}
				}
				break;
				case CMD_MSG_TRADE:
				{
					if (nNumArg == 1)
					{
						wchar_t wszMsg[256] = { 0 };

						// If self is requesting a trade with someone, or someone is requesting a trade with self,
						if (GetTradeTask().GetTradePrivateMarket().IsRequesting() || GetTradeTask().GetTradePrivateMarket().IsAccepting())
						{
							swprintf_s(wszMsg, 256, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1724));
							AddChatDlg(wszMsg, CHAT_SYSTEM);
							break;
						}

						// If the target cannot be found nearby,
						DnActorHandle hActor = CDnActor::FindActorFromName((TCHAR *)strArg1.c_str());
						if (!hActor)
						{
							swprintf_s(wszMsg, 256, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 673));
							AddChatDlg(wszMsg, CHAT_SYSTEM);
							break;
						}
						DWORD nSessionID = hActor->GetUniqueID();

						// If someone attempted to trade with self
						if (CDnActor::s_hLocalActor->GetUniqueID() == nSessionID)
							break;

						GetTradeTask().GetTradePrivateMarket().RequestPrivateMarket(nSessionID);
						bProcessed = true;
					}
				}
				break;
				case CMD_MSG_ESCAPE:
				{
#ifdef PRE_FIX_ESCAPE
					GetInterface().ShowEscapeDlg(true);
#else
					if (!CDnActor::s_hLocalActor) break;
					CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
					if (!pPlayer) break;
					pPlayer->CmdEscape();
#endif

					bProcessed = true;
					return true;
				}
				break;
				case CMD_MSG_RAIDNOTICE:
				{
					if (!CDnActor::s_hLocalActor) break;
					CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
					if (pPlayer && IsGuildWarMaster(pPlayer->GetName()))
					{
						std::wstring::size_type startidx, endidx;
						startidx = strText.find_first_of(L" ");
						endidx = strText.length();

						std::wstring wstrTemp;
						wstrTemp = strText.substr(startidx + 1, endidx);

						SendChatMsg(CHATTYPE_RAIDNOTICE, wstrTemp.c_str());
						return true;
					}
					bool bCanNotUse = false;
					if (CDnPartyTask::IsActive() == false)
						bCanNotUse = true;

					if (CDnPartyTask::GetInstance().IsPartyType(_RAID_PARTY_8) == false ||
						CDnPartyTask::GetInstance().IsLocalActorMaster() == false)
						bCanNotUse = true;

					if (bCanNotUse)
					{
						AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3840), CHAT_SYSTEM);	// UISTRING : Only the raid leader can use the shout function.
						return true;
					}

					SendChatMsg(CHATTYPE_RAIDNOTICE, strText.c_str());
					return true;
				}
				break;
#ifdef PRE_PRIVATECHAT_CHANNEL
				case CMD_MSG_CHANNEL_CREATE:
				{
					m_pIMEEditBox->ClearText();
					if (nNumArg == 1)
					{
						if (GetChannelChatTask().IsActive() && !GetChannelChatTask().IsJoinChannel())
						{
							if (strArg1.size() < CHANNEL_NAME_MIN || strArg1.size() > CHANNEL_NAME_MAX)
							{
								GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7812), false);	// UISTRING : Please enter a channel title between 2 and 10 characters.
								return true;
							}

							DWORD dwCheckType = ALLOW_STRING_DEFAULT;
							if (g_CountryUnicodeSet.Check(strArg1.c_str(), dwCheckType) == false)
							{
								GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7811), false);	// UISTRING : Invalid channel name format.
								return true;
							}
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
							GetChannelChatTask().SendChannelAdd(strArg1.c_str(), -1);
#else // PRE_ADD_PRIVATECHAT_CHANNEL
							GetChannelChatTask().SendChannelAdd(strArg1.c_str());
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
						}
						else
						{
							AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7813), CHAT_SYSTEM);	// UISTRING : Cannot join more than 1 channel.
						}
					}
					return true;
				}
				break;
				case CMD_MSG_CHANNEL_JOIN:
				{
					m_pIMEEditBox->ClearText();
					if (nNumArg == 1)
					{
						if (GetChannelChatTask().IsActive() && !GetChannelChatTask().IsJoinChannel())
						{
							if (strArg1.size() < CHANNEL_NAME_MIN || strArg1.size() > CHANNEL_NAME_MAX)
							{
								GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7812), false);	// UISTRING : Please enter a channel title between 2 and 10 characters.
								return true;
							}

							DWORD dwCheckType = ALLOW_STRING_DEFAULT;
							if (g_CountryUnicodeSet.Check(strArg1.c_str(), dwCheckType) == false)
							{
								GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7811), false);	// UISTRING : Invalid channel name format.
								return true;
							}

							GetChannelChatTask().SendChannelJoin(strArg1.c_str(), -1);
						}
						else
						{
							AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7813), CHAT_SYSTEM);	// UISTRING : Cannot join more than 1 channel.
						}
					}
					return true;
				}
				break;
				case CMD_MSG_CHANNEL_PASSWORD:
				{
					m_pIMEEditBox->ClearText();
					if (GetChannelChatTask().IsActive() && GetChannelChatTask().IsJoinChannel())
					{
						if (GetChannelChatTask().IsChannelMaster())
						{
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
							CDnPrivateChannelPasswordChangeDlg* pDnPrivateChannelPasswordChangeDlg = GetInterface().GetPrivateChannelPasswordChangeDlg();
							if (pDnPrivateChannelPasswordChangeDlg)
							{
								pDnPrivateChannelPasswordChangeDlg->Show(true);
							}
#else // PRE_ADD_PRIVATECHAT_CHANNEL
							CDnPrivateChannelPasswordDlg* pDnPrivateChannelPasswordDlg = GetInterface().GetPrivateChannelPasswordDlg();
							if (pDnPrivateChannelPasswordDlg)
							{
								pDnPrivateChannelPasswordDlg->SetChannelPasswordType(CDnPrivateChannelPasswordDlg::TYPE_PASSWORD_CHANGE);
								pDnPrivateChannelPasswordDlg->Show(true);
							}
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
						}
						else
						{
							AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7817), CHAT_SYSTEM);	// UISTRING : You do not have permission.
						}
					}
					else
					{
						AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7829), CHAT_SYSTEM);	// UISTRING : Not participating in any channel.
					}
					return true;
				}
				break;
				case CMD_MSG_CHANNEL_INVITE:
				{
					m_pIMEEditBox->ClearText();
					if (nNumArg == 1)
					{
						if (GetChannelChatTask().IsActive() && GetChannelChatTask().IsJoinChannel())
						{
							if (GetChannelChatTask().IsChannelMaster())
							{
								GetChannelChatTask().SendChannelInvite(strArg1.c_str());
							}
							else
							{
								AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7817), CHAT_SYSTEM);	// UISTRING : You do not have permission.
							}
						}
						else
						{
							AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7829), CHAT_SYSTEM);	// UISTRING : Not participating in any channel.
						}
					}
					return true;
				}
				break;
				case CMD_MSG_CHANNEL_LEAVE:
				{
					m_pIMEEditBox->ClearText();
					if (GetChannelChatTask().IsActive() && GetChannelChatTask().IsJoinChannel())
					{
						GetChannelChatTask().SendChannelOut();
					}
					else
					{
						AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7829), CHAT_SYSTEM);	// UISTRING : Not participating in any channel.
					}
					return true;
				}
				break;
				case CMD_MSG_CHANNEL_BAN:
				{
					m_pIMEEditBox->ClearText();
					if (nNumArg == 1)
					{
						if (GetChannelChatTask().IsActive() && GetChannelChatTask().IsJoinChannel())
						{
							if (GetChannelChatTask().IsChannelMaster())
							{
								GetChannelChatTask().SendChannelKick(strArg1.c_str());
							}
							else
							{
								AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7817), CHAT_SYSTEM);	// UISTRING : You do not have permission.
							}
						}
						else
						{
							AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7829), CHAT_SYSTEM);	// UISTRING : Not participating in any channel.
						}
					}
					return true;
				}
				break;
				case CMD_MSG_CHANNEL_MASTER:
				{
					m_pIMEEditBox->ClearText();
					if (nNumArg == 1)
					{
						if (GetChannelChatTask().IsActive() && GetChannelChatTask().IsJoinChannel())
						{
							if (GetChannelChatTask().IsChannelMaster())
							{
								PrivateChatChannel::TMemberInfo* pChannelMemberInfo = GetChannelChatTask().GetChannelMemberInfoByName(strArg1.c_str());
								if (pChannelMemberInfo)
								{
									GetChannelChatTask().SendChannelMod(PrivateChatChannel::Common::ChangeMaster, 0, pChannelMemberInfo->biCharacterDBID);
								}
								else
								{
									AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7825), CHAT_SYSTEM);	// UISTRING : No user available to delegate room master to.
								}
							}
							else
							{
								AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7817), CHAT_SYSTEM);	// UISTRING : You do not have permission.
							}
						}
						else
						{
							AddChatDlg(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7829), CHAT_SYSTEM);	// UISTRING : Not participating in any channel.
						}
					}
					return true;
				}
				break;
				case CMD_MSG_CHANNEL_HELP:
				{
					m_pIMEEditBox->ClearText();
					std::vector<std::wstring> vecHelps;
					TokenizeW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7836), vecHelps, L"\\n");

					for (int i = 0; i<static_cast<int>(vecHelps.size()); i++)
					{
						AddChatDlg(vecHelps[i].c_str(), CHAT_NORMAL);
					}
					return true;
				}
#endif // PRE_PRIVATECHAT_CHANNEL
				}
			}
		}

		if (DebugCmdProc(strText.c_str())) bProcessed = true;

		m_pIMEEditBox->ClearText();
		RequestFocus(m_pIMEEditBox);

		// Return true so input box loses focus when handled as a valid command.
		if (bProcessed)
			return true;
	}

	return false;
}

void CDnChatTabDlg::_AddChatMessage(int nChatType, LPCWSTR wszFromCharName, LPCWSTR wszMessage)
{
	std::wstring wszTempMsg = wszMessage;
	if (CDnActor::s_hLocalActor && ((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->IsDeveloperAccountLevel() == false)
		DN_INTERFACE::UTIL::CheckChat(wszTempMsg, '*');
	wszMessage = wszTempMsg.c_str();

	wchar_t wszMsg[1024] = { 0 };

	int nVecSize = (int)m_vecChatDlg.size();
	for (int i = 0; i<nVecSize; i++)
	{
		if (m_vecChatDlg[i])
		{
			CDnChatOptDlg &chatOptDlg = m_vecChatDlg[i]->GetChatOptDlg();

			// Among the types below, excluding whispers, the only ones executed are commands starting with / registered in the local chat window.
			// So there is no code checking whether wszFromCharName is NULL.
			switch (nChatType)
			{
			case CHAT_NORMAL:
			{
				if (chatOptDlg.IsNomal())
				{
					swprintf_s(wszMsg, 1024, L"[%s] : %s", wszFromCharName, wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_NORMAL);
				}
			}
			break;
			case CHAT_PARTY:
			{
				if (chatOptDlg.IsParty())
				{
					swprintf_s(wszMsg, 1024, L"[%s] : %s", wszFromCharName, wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_PARTY);
				}
			}
			break;
			case CHAT_GUILD:
			{
				if (chatOptDlg.IsGuild())
				{
					swprintf_s(wszMsg, 1024, L"[%s] : %s", wszFromCharName, wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_GUILD);
				}
			}
			break;
			case CHAT_PRIVATE:
			{
				CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::INVENTORY_DIALOG);
				if (pInvenDlg)
				{
					// Register in my speech bubble only if wszFromCharName matches the trading partner's name.
					if (pInvenDlg->IsYourName(wszFromCharName))
						pInvenDlg->AddPrivateMarketChatMe(wszMessage);
				}

				if (chatOptDlg.IsPrivate())
				{
					swprintf_s(wszMsg, 1024, L"[%s]<<< : %s", wszFromCharName, wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_PRIVATE);
				}
			}
			break;
			case CHAT_CHANNEL:
			{
				if (chatOptDlg.IsChannel())
				{
					swprintf_s(wszMsg, 1024, L"[%s] : %s", wszFromCharName, wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_CHANNEL);
				}
			}
			break;
			case CHAT_WORLD:
			{
				if (chatOptDlg.IsWorld())
				{
					swprintf_s(wszMsg, 1024, L"[%s] : %s", wszFromCharName, wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_WORLD);
				}
			}
			break;
#ifdef PRE_PRIVATECHAT_CHANNEL
			case CHAT_PRIVATE_CHANNEL:
			{
				if (chatOptDlg.IsPrivateChannel())
				{
					swprintf_s(wszMsg, 1024, L"[%s] : %s", wszFromCharName, wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_PRIVATE_CHANNEL);
				}
			}
			break;
#endif // PRE_PRIVATECHAT_CHANNEL
			}
		}
	}

}

#ifdef PRE_ADD_DOORS
void CDnChatTabDlg::AddChatMessage(int nChatType, LPCWSTR wszFromCharName, LPCWSTR wszMessage, bool isAppend, bool hasColor, DWORD colorValue, DWORD bgColorValue, bool bIsDoorsMobile)
#else
void CDnChatTabDlg::AddChatMessage(int nChatType, LPCWSTR wszFromCharName, LPCWSTR wszMessage, bool isAppend, bool hasColor, DWORD colorValue, DWORD bgColorValue)
#endif
{
	if (!m_bPVPLobbyChat) {
		if (nChatType == CHAT_NORMAL || nChatType == CHAT_PARTY) {
			if (wcslen(wszFromCharName) > 0) {
				DnActorHandle hActor = CDnActor::FindActorFromName(const_cast<TCHAR*>(wszFromCharName));
				bool bBlackList = (CDnIsolateTask::IsActive() && GetIsolateTask().IsBlackList(wszFromCharName));

				if (hActor && false == bBlackList) {

#ifdef PRE_ADD_CASH_AMULET
					if (hActor->GetChatBalloon().IsEnableCustomChatBallon())
					{
						hActor->GetChatBalloon().StopChatBalloonCustom();
						hActor->GetChatBalloon().ClearCustomChatBalloonText();
					}

					if (CGameOption::GetInstance().m_bShowCashChatBalloon && hActor->IsPlayerActor())
					{
						CDnPlayerActor* pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
						DnPartsHandle hCashAmulet = pPlayer->GetCashParts(CDnParts::CashNecklace);

						if (hCashAmulet)
						{
							int nItemID = hCashAmulet->GetClassID();
							DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TCASHCHATBALLOONTABLE);

							if (pSox && pSox->IsExistItem(nItemID))
							{
								const DWORD NORMAL_CHAT_COLOR = D3DCOLOR_ARGB(255, 90, 82, 76);
								const DWORD PARTY_CHAT_COLOR = D3DCOLOR_ARGB(255, 0, 0, 128);
								// chatroomcolor::NORMAL

								if (nChatType == CHAT_PARTY)
									hActor->GetChatBalloon().SetChatBalloonCustomText(pSox->GetFieldFromLablePtr(nItemID, "_UIResourceName2")->GetString(), wszMessage, PARTY_CHAT_COLOR, GetTickCount(), 1);
								else
									hActor->GetChatBalloon().SetChatBalloonCustomText(pSox->GetFieldFromLablePtr(nItemID, "_UIResourceName1")->GetString(), wszMessage, NORMAL_CHAT_COLOR, GetTickCount(), 1);
							}
							else
							{
								hActor->SetChatBalloonText(wszMessage, GetTickCount(), nChatType);
							}
						}
						else
							hActor->SetChatBalloonText(wszMessage, GetTickCount(), nChatType);

					}
					else
						hActor->SetChatBalloonText(wszMessage, GetTickCount(), nChatType);

#else
					hActor->SetChatBalloonText(wszMessage, GetTickCount(), nChatType);
#endif
				}
			}
		}
	}

	// Play sound if inactive when a whisper arrives.
	if (nChatType == CHAT_PRIVATE)
	{
		if (!IsShow())
		{
			if (m_nPrivateAlarmSoundIndex != -1)
				CEtSoundEngine::GetInstance().PlaySound("2D", m_nPrivateAlarmSoundIndex);
		}
	}

	wchar_t wszMsg[1024] = { 0 };

	std::wstring fromCharNameString;
	if (wcslen(wszFromCharName) != 0)
	{
		CDnNameLinkMng* pMgr = GetInterface().GetNameLinkMng();
		if (pMgr)
			pMgr->MakeNameLinkString_UserName(fromCharNameString, wszFromCharName);
	}

	int nGuildMid = 5007; // Mid: Guild

#ifndef _FINAL_BUILD
	bool bCheckDebugMessage = false;
	if (nChatType == CHATTYPE_NORMAL && __wcsicmp_l(wszFromCharName, L"Debug") == 0) bCheckDebugMessage = true;
#endif
	int nVecSize = (int)m_vecChatDlg.size();
	for (int i = 0; i<nVecSize; i++)
	{
		if (m_vecChatDlg[i])
		{
			CDnChatOptDlg &chatOptDlg = m_vecChatDlg[i]->GetChatOptDlg();

			switch (nChatType)
			{
			case CHAT_NORMAL:
			{
				if (CDnIsolateTask::IsActive() && GetIsolateTask().IsBlackList(wszFromCharName))
					return;

				if (chatOptDlg.IsNomal())
				{
					if (fromCharNameString.empty())
						swprintf_s(wszMsg, 1024, L"%s", wszMessage);
					else
						swprintf_s(wszMsg, 1024, L"[%s] : %s", fromCharNameString.c_str(), wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_NORMAL, 0, isAppend, hasColor, colorValue, bgColorValue);
				}
#ifndef _FINAL_BUILD
				if (bCheckDebugMessage) break;
#endif
			}
			break;
			case CHAT_PARTY:
			{
				if (chatOptDlg.IsParty())
				{
					if (isAppend)
					{
						swprintf_s(wszMsg, 1024, L"%s", wszMessage);
					}
					else
					{
						if (m_IsPVPGameChat)
						{
							if (IsGuildWarMaster(wszFromCharName))
								nChatType = CHAT_GUILDWAR;
							swprintf_s(wszMsg, 1024, L"[%s][%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 121018), fromCharNameString.c_str(), wszMessage);
						}
						else
						{
							if (CDnPartyTask::IsActive() == false)
								return;

							bool bRaidMaster = false;
							if (CDnPartyTask::GetInstance().IsPartyType(_RAID_PARTY_8))
							{
								CDnPartyTask::PartyStruct* pData = CDnPartyTask::GetInstance().GetPartyDataFromMemberName(wszFromCharName);
								if (pData == NULL)
									return;

								if (pData->bPartyMaster)
								{
									swprintf_s(wszMsg, 1024, L"[%s]%s : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3838), fromCharNameString.c_str(), wszMessage); // UISTRING : Raid Leader
									bRaidMaster = true;
								}
								else
								{
									swprintf_s(wszMsg, 1024, L"[%s]%s : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3839), fromCharNameString.c_str(), wszMessage); // UISTRING : Raid Member
								}
							}
							else
							{
								if (fromCharNameString.empty())
									swprintf_s(wszMsg, 1024, L"[%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 632), wszMessage);
								else
									swprintf_s(wszMsg, 1024, L"[%s][%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 632), fromCharNameString.c_str(), wszMessage);
							}
						}
					}

					if (!m_bCustomizeChatMode)
					{
						m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, nChatType, 0, isAppend, hasColor, colorValue, bgColorValue);
					}
					else
					{
						if (m_nCustomizeModeType == GetChatMode() && nChatType == m_nCustomizeModeType)
						{
							swprintf_s(wszMsg, 1024, L"[%s][%s] : %s", m_wszCustomizeModeHeader, fromCharNameString.c_str(), wszMessage);
							m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, nChatType, 0, isAppend, hasColor, colorValue, bgColorValue);
						}
						else
						{
							m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, nChatType, 0, isAppend, hasColor, colorValue, bgColorValue);
						}
					}

					nChatType = CHAT_PARTY;
				}
			}
			break;
			case CHAT_RAIDNOTICE:
			{
				std::wstring chatContents = wszMessage;
				if (wszMessage[0] == L'/')
				{
					int nNumArg = 0;
					std::wstring::size_type endIdx;
					endIdx = chatContents.find_first_of(L" ");
					if (endIdx != std::wstring::npos)
						chatContents = chatContents.substr(endIdx, std::wstring::npos);
				}
				std::wstring chatBoxStr;
				std::wstring captionStr;
				if (CDnBridgeTask::GetInstance().IsPvPGameMode(PvPCommon::GameMode::PvP_GuildWar))
				{
					if (!IsGuildWarMaster(wszFromCharName))
						return;

					int nPvPUserState = GetGuildWarUserState(wszFromCharName);

					if (PvPCommon::UserState::GuildWarCaptain & nPvPUserState)
					{
						chatBoxStr = FormatW(L"[%s]%s:%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2373), fromCharNameString.c_str(), chatContents.c_str()); // UISTRING : Master
						captionStr = FormatW(L"[%s]:%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2373), chatContents.c_str());
					}
					else if (PvPCommon::UserState::GuildWarSedcondCaptain & nPvPUserState)
					{
						chatBoxStr = FormatW(L"[%s]%s:%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 126290), fromCharNameString.c_str(), chatContents.c_str()); // UISTRING : Vice Master
						captionStr = FormatW(L"[%s]:%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 126290), chatContents.c_str());
					}
				}
				else
				{
					chatBoxStr = FormatW(L"[%s]%s:%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3838), fromCharNameString.c_str(), chatContents.c_str()); // UISTRING : Raid Leader
					captionStr = FormatW(L"[%s]:%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3838), chatContents.c_str());
				}
				m_vecChatDlg[i]->AddChat(wszFromCharName, chatBoxStr.c_str(), CHAT_RAIDNOTICE, 0, isAppend, hasColor, colorValue, bgColorValue);
				GetInterface().ShowCaptionDialog(CDnInterface::typeCaption4, captionStr.c_str(), chatcolor::RAIDNOTICE);
			}
			break;
			case CHAT_GUILD:
			{
				if (chatOptDlg.IsGuild())
				{
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
					if (bIsDoorsMobile)
						swprintf_s(wszMsg, 1024, L"[%s][%s][m] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nGuildMid), fromCharNameString.c_str(), wszMessage);
					else
						swprintf_s(wszMsg, 1024, L"[%s][%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nGuildMid), fromCharNameString.c_str(), wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_GUILD, 0, isAppend, hasColor, colorValue, bgColorValue);
#else
					swprintf_s(wszMsg, 1024, L"[%s][%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nGuildMid), fromCharNameString.c_str(), wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_GUILD, 0, isAppend, hasColor, colorValue, bgColorValue);
#endif
				}
			}
			break;
			case CHAT_PRIVATE:
			{
				CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::INVENTORY_DIALOG);
				if (pInvenDlg)
				{
					// Register in my speech bubble only if wszFromCharName matches the trading partner's name.
					if (pInvenDlg->IsYourName(wszFromCharName))
						pInvenDlg->AddPrivateMarketChatYou(wszMessage);
				}

				// Save this regardless of the IsPrivate option.
				// This is required to allow replying with Shift+r.
				SetLastPrivateName(wszFromCharName);

				if (chatOptDlg.IsPrivate())
				{
#ifdef PRE_ADD_DOORS
					if (bIsDoorsMobile)
						swprintf_s(wszMsg, 1024, L"[%s][m]>>> : %s", fromCharNameString.c_str(), wszMessage);
					else
						swprintf_s(wszMsg, 1024, L"[%s]>>> : %s", fromCharNameString.c_str(), wszMessage);
#else
					swprintf_s(wszMsg, 1024, L"[%s]>>> : %s", fromCharNameString.c_str(), wszMessage);
#endif
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_PRIVATE, 0, isAppend, hasColor, colorValue, bgColorValue);
					AddPrivateName(wszFromCharName);
				}
			}
			break;
			case CHAT_CHANNEL:
			{
				if (chatOptDlg.IsChannel())
				{
					swprintf_s(wszMsg, 1024, L"[%s][%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 605), fromCharNameString.c_str(), wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_CHANNEL, 0, isAppend, hasColor, colorValue, bgColorValue);
				}
			}
			break;
			case CHAT_WORLD:
			{
				if (chatOptDlg.IsWorld())
				{
					swprintf_s(wszMsg, 1024, L"[%s][%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 606), fromCharNameString.c_str(), wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_WORLD, 0, isAppend, hasColor, colorValue, bgColorValue);
				}
			}
			break;
#ifdef PRE_PRIVATECHAT_CHANNEL
			case CHAT_PRIVATE_CHANNEL:
			{
				if (chatOptDlg.IsPrivateChannel())
				{
					if (fromCharNameString.length() > 0)
						swprintf_s(wszMsg, 1024, L"[%s][%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1202), fromCharNameString.c_str(), wszMessage);
					else
						swprintf_s(wszMsg, 1024, L"[%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1202), wszMessage);

					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_PRIVATE_CHANNEL, 0, isAppend, hasColor, colorValue, bgColorValue);
				}
			}
			break;
#endif // PRE_PRIVATECHAT_CHANNEL
#ifdef PRE_ADD_WORLD_MSG_RED
			case CHAT_WORLD_RED:
			{
				if (chatOptDlg.IsWorld())
				{
					swprintf_s(wszMsg, 1024, L"[%s][%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 606), fromCharNameString.c_str(), wszMessage);
					m_vecChatDlg[i]->AddChat(wszFromCharName, wszMsg, CHAT_WORLD_RED, 0, isAppend, hasColor, colorValue, bgColorValue);
				}
			}
			break;
#endif // PRE_ADD_WORLD_MSG_RED
			}
		}
	}

#ifdef PRE_ADD_WORLD_MSG_RED
	if (!m_bPVPLobbyChat && nChatType == CHAT_WORLD_RED)
	{
		if (m_pWorldChatRed)
		{
			m_pWorldChatRed->ClearText();
			swprintf_s(wszMsg, 1024, L"[%s][%s]", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 606), fromCharNameString.c_str());
			m_pWorldChatRed->AddText(wszMsg, textcolor::WHITE);
			swprintf_s(wszMsg, 1024, L"%s", wszMessage);
			m_pWorldChatRed->AddText(wszMsg, chatcolor::WORLD_RED);
			int nLineCount = 4 - m_pWorldChatRed->GetLineSize();
			for (int i = 0; i<nLineCount; i++)
				m_pWorldChatRed->AddText(L"\n");
			m_fWorldMsgRedCoolTime = CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::WorldMessageRedCoolTime) / 1000.0f;
			m_nBlinkCount = 1;//static_cast<int>( CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::WorldMessageRedCoolTime ) / 1000.0f ) % 3;
			m_fWorldMsgRed1Sec = 0.3f;
		}
	}
#endif // PRE_ADD_WORLD_MSG_RED
}

void CDnChatTabDlg::AppendChatEditBox(LPCWSTR wszString, bool bFocus)
{
	if (m_pIMEEditBox->GetFullTextLength() + lstrlenW(wszString) >= CHATLENMAX)
		return;

	if (IsShow() == false)
	{
		Show(true);
		if (bFocus == false)
			focus::ReleaseControl();
	}
	m_pIMEEditBox->AddText(wszString, bFocus);
}

void CDnChatTabDlg::AddSystemMessage(LPCWSTR wszFromCharName, LPCWSTR wszMessage, bool bShowCaption)
{
	wchar_t wszMsg[1024] = { 0 };

	if (wszFromCharName == NULL || wszFromCharName[0] == '\0')
	{
		swprintf_s(wszMsg, 1024, L"%s", wszMessage);
	}
	else
	{
		// Directly inserting into only one like this should probably be changed.
		// Wait, are there even system messages that come in with a name included?
		std::wstring strFromCharName;
		CDnNameLinkMng* pMgr = GetInterface().GetNameLinkMng();
		if (pMgr)
			pMgr->MakeNameLinkString_UserName(strFromCharName, wszFromCharName);

		swprintf_s(wszMsg, 1024, L"[%s] : %s", strFromCharName.c_str(), wszMessage);
	}
	AddChatDlg(wszMsg, CHAT_SYSTEM);

	if (bShowCaption)
		GetInterface().ShowCaptionDialog(CDnInterface::typeCaption3, wszMessage, textcolor::YELLOW, 5.0f);
}

void CDnChatTabDlg::AddNoticeMessage(LPCWSTR wszFromCharName, LPCWSTR wszMessage)
{
	wchar_t wszMsg[1024] = { 0 };

	if (wszFromCharName == NULL || wszFromCharName[0] == '\0')
	{
		// Changed so server no longer sends the 'Notice' word, so handle it directly like this.
		swprintf_s(wszMsg, 1024, L"[%s] : %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 96), wszMessage);
	}
	else
	{
		std::wstring strFromCharName;
		CDnNameLinkMng* pMgr = GetInterface().GetNameLinkMng();
		if (pMgr)
			pMgr->MakeNameLinkString_UserName(strFromCharName, wszFromCharName);

		swprintf_s(wszMsg, 1024, L"[%s] : %s", strFromCharName.c_str(), wszMessage);
	}

	// Existing approach was broken, so cover it for now by inserting directly as below.
	m_pChatNormalDlg->AddChat(wszFromCharName, wszMsg, CHAT_NOTICE);
#ifdef PRE_ADD_CHAT_RENEWAL
	m_pChatWhisperDlg->AddChat(wszFromCharName, wszMsg, CHAT_NOTICE); // Whisper.
#endif
	m_pChatPartyDlg->AddChat(wszFromCharName, wszMsg, CHAT_NOTICE);
	m_pChatGuildDlg->AddChat(wszFromCharName, wszMsg, CHAT_NOTICE);
	m_pChatSystemDlg->AddChat(wszFromCharName, wszMsg, CHAT_NOTICE);

	GetInterface().ShowCaptionDialog(CDnInterface::typeCaption1, wszMessage, textcolor::HOTPINK);
}

bool CDnChatTabDlg::CheckRepeat()
{
	std::wstring wszString = m_pIMEEditBox->GetText();

	return CheckRepeat(wszString);
}

bool CDnChatTabDlg::CheckRepeat(const std::wstring& chatMsg)
{
	const UINT nMaxCount = 5;

	// Exception handling
	if (chatMsg == L"")
		return false;

	if (m_wszPrevString != chatMsg)
	{
		m_wszPrevString = chatMsg;
		m_nRepeat = 1;
	}
	else
	{
		if (m_nRepeat >= nMaxCount)
			return true;

		++m_nRepeat;
	}
	return false;
}

bool CDnChatTabDlg::CheckPapering()
{
	// If currently restricted due to chat spamming,
	if (m_fPaperingRemainTime > 0.0f)
		return true;

	// If the limit count is reached when adding,
	if ((int)m_listRecentChatTime.size() + 1 >= CHAT_PAPERING_CHECKCOUNT)
		m_fPaperingRemainTime = (float)CHAT_PAPERING_RESTRICTIONTIME;

	m_listRecentChatTime.push_back((float)CHAT_PAPERING_CHECKTIME);
	return false;
}

void CDnChatTabDlg::AddChatQuestMsg(LPCWSTR wszMsg)
{
	AddChatDlg(wszMsg, CHAT_SYSTEM);
	GetInterface().ShowCaptionDialog(CDnInterface::typeCaption2, wszMsg, textcolor::YELLOW);
}

void CDnChatTabDlg::AddChatQuestReceipt(LPCWSTR wszQuestName)
{
	wchar_t wszMsg[256] = { 0 };
#if defined(PRE_ADD_ENGLISH_STRING)
	swprintf_s(wszMsg, 256, L"\"%s\" %s", wszQuestName, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 635));
#else
	swprintf_s(wszMsg, 256, L"%s, %s %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 647), wszQuestName, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 635));
#endif
	AddChatDlg(wszMsg, CHAT_SYSTEM);

	GetInterface().ShowCaptionDialog(CDnInterface::typeCaption2, wszMsg, textcolor::YELLOW);
}

void CDnChatTabDlg::AddChatQuestCompletion(LPCWSTR wszQuestName)
{
	wchar_t wszMsg[256] = { 0 };
#if defined(PRE_ADD_ENGLISH_STRING)
	swprintf_s(wszMsg, 256, L"\"%s\" %s", wszQuestName, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 636));
#else
	swprintf_s(wszMsg, 256, L"%s, %s %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 647), wszQuestName, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 636));
#endif
	AddChatDlg(wszMsg, CHAT_SYSTEM);

	GetInterface().ShowCaptionDialog(CDnInterface::typeCaption2, wszMsg, textcolor::YELLOW);
}

void CDnChatTabDlg::AddChatDlg(LPCWSTR wszMsg, int nChatType)
{
	int nVecSize = (int)m_vecChatDlg.size();
	for (int i = 0; i<nVecSize; i++)
	{
		if (m_vecChatDlg[i])
		{
			CDnChatOptDlg &chatOptDlg = m_vecChatDlg[i]->GetChatOptDlg();

			if (chatOptDlg.GetOption() & nChatType)
			{
				m_vecChatDlg[i]->AddChat(NULL, wszMsg, nChatType);
			}
		}
	}
}

CDnChatDlg *CDnChatTabDlg::GetCurrentChatDlg()
{
	CDnChatDlg *pChatDlg = static_cast<CDnChatDlg*>(m_groupTabDialog.GetShowDialog());
	return pChatDlg;
}

void CDnChatTabDlg::ShowChatDlg()
{
	m_groupTabDialog.ShowDialog(GetCurrentTabID(), true);

	// This ShowChatDlg() function is called when showing the chat window again after CloseAllDialog, and
	// Without the scrollbar handling below, the scrollbar becomes visible even though the chat tab is not shown (active).
	// So force the scrollbar to be hidden.

#ifdef PRE_MOD_CHATBG
	if (m_pChatBoardDlg)
		m_pChatBoardDlg->Show(IsBackGroundMode() ? true : false);
#endif

	if (!IsShow())
	{
		CDnChatDlg *pChatDlg = static_cast<CDnChatDlg*>(m_groupTabDialog.GetShowDialog());
		if (!pChatDlg) return;
		pChatDlg->ShowScrollBar(false);
	}
}

void CDnChatTabDlg::Show(bool bShow)
{
	if (m_bIgnoreShowFunc)
	{
		m_bIgnoreShowFunc = false;
		return;
	}

	if (!m_bEndInitialize)
	{
		ShowEx(bShow);
		return;
	}

	CEtUIDialog::Show(bShow);
	ShowControl(bShow);

	m_groupTabDialog.ShowDialog(GetCurrentTabID(), bShow);

	if (bShow)
	{
		RequestFocus(m_pIMEEditBox);
	}
	else
	{
		m_pIMEEditBox->ClearText();
		focus::ReleaseControl();
		m_bShowOptDlg = false;
	}

	CDnMouseCursor::GetInstance().ShowCursor(bShow);
}

void CDnChatTabDlg::ShowEx(bool bShow)
{
	if (bShow && GetInterface().IsShowCashShopDlg())
		return;

	SetRenderPriority(this, bShow);

	if (m_bShow == bShow)
	{
		if (bShow)
		{
			RequestFocus(m_pIMEEditBox);
			m_pIMEEditBox->CheckIMEToggleState();

			CDnMouseCursor::GetInstance().ShowCursor(true, true);
		}
		return;
	}

	CDnChatDlg *pChatDlg = static_cast<CDnChatDlg*>(m_groupTabDialog.GetShowDialog());
	if (!pChatDlg) return;

	CEtUIDialog::Show(bShow);
	ShowControl(bShow);

#ifdef PRE_ADD_WORLD_MSG_RED
	if (!m_bPVPLobbyChat && m_pWorldChatRedBoard)
	{
		if (m_fWorldMsgRedCoolTime > 0)
			m_pWorldChatRedBoard->Show(bShow);
	}
#endif // PRE_ADD_WORLD_MSG_RED

	pChatDlg->ShowScrollBar(bShow);

	if (bShow)
	{
		RequestFocus(m_pIMEEditBox);
		m_pIMEEditBox->CheckIMEToggleState();
		CDnMouseCursor::GetInstance().ShowCursor(true);
	}
	else
	{
		ShowChildDialog(m_pChatHelpDlg, false);
		pChatDlg->ShowOptDlg(false);

		m_pIMEEditBox->ClearText();
		if (m_pIMEEditBox->IsFocus())
			focus::ReleaseControl();

		m_bShowOptDlg = false;

		if (!drag::IsValid())
		{
			CDnMouseCursor::GetInstance().ShowCursor(false);
		}
		else
		{
			CDnMouseCursor::GetInstance().ReleaseShowRefCnt();
		}

		// Exception handling: Esc while hovering or dragging.
		if (m_nSizeMode == SIZE_OVER || m_nSizeMode == SIZE_DRAG)
		{
			m_nSizeMode = SIZE_BASE;
			CDnMouseCursor::GetInstance().SetCursorState(CDnMouseCursor::NORMAL);
		}
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
		SetPrivateChannelNotify(false);
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
	}

#ifdef PRE_MOD_CHATBG
	if (m_pChatBoardDlg)
	{
		m_pChatBoardDlg->Show(IsBackGroundMode() ? true : bShow);
	}
#endif
}

#ifdef PRE_MOD_CHATBG
bool CDnChatTabDlg::IsBackGroundMode() const
{
	const CDnChatDlg *pChatDlg = static_cast<const CDnChatDlg*>(m_groupTabDialog.GetShowDialog());
	if (pChatDlg)
	{
		const CDnChatOptDlg &chatOptDlg = pChatDlg->GetChatOptDlg();
		return chatOptDlg.IsCheckBackground();
	}

	return false;
}
#endif

void CDnChatTabDlg::ShowControl(bool bShow)
{
	m_pNormalTab->Show(bShow);
#ifdef PRE_ADD_CHAT_RENEWAL
	m_pWhisperTab->Show(bShow);
#endif
	m_pPartyTab->Show(bShow);
	m_pGuildTab->Show(bShow);
	m_pSystemTab->Show(bShow);
#ifdef PRE_PRIVATECHAT_CHANNEL
	m_pPrivateChannelTab->Show(bShow);
#endif // PRE_PRIVATECHAT_CHANNEL

	m_pButtonModeUp->Show(bShow);
	m_pButtonModeDown->Show(bShow);
	m_pButtonOption->Show(bShow);
	m_pButtonHelp->Show(bShow);
	m_pButtonReport->Show(false);

	m_pStaticMode->Show(bShow);
	m_pIMEEditBox->Show(bShow);
}

void CDnChatTabDlg::ChangePrivateName()
{
	if (!m_listPrivateName.empty())
	{
		m_strPrivateUserName = m_listPrivateName.front();
		m_listPrivateName.pop_front();
		m_listPrivateName.push_back(m_strPrivateUserName);

		wchar_t wszMsg[256] = { 0 };
		swprintf_s(wszMsg, 256, L"[%s][%s]:", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 648), m_strPrivateUserName.c_str());
		ResizeIMECCtl(wszMsg);
	}
}

void CDnChatTabDlg::AddPrivateName(const std::wstring strPrivateName)
{
	PRIVATENAME_LIST_ITER iter = m_listPrivateName.begin();
	for (; iter != m_listPrivateName.end(); )
	{
		if ((*iter) == strPrivateName)
		{
			iter = m_listPrivateName.erase(iter);
		}
		else ++iter;
	}

	m_listPrivateName.push_back(strPrivateName);
}

void CDnChatTabDlg::DelPrivateName(const std::wstring &strPrivateName)
{
	return;

	PRIVATENAME_LIST_ITER iter = m_listPrivateName.begin();
	for (; iter != m_listPrivateName.end(); )
	{
		if ((*iter) == strPrivateName)
		{
			iter = m_listPrivateName.erase(iter);
		}
		else ++iter;
	}

	if (!m_listPrivateName.empty())
	{
		ChangePrivateName();
	}
	else
	{
		SetChatMode(CHAT_NORMAL);
		m_strPrivateUserName.clear();
	}
}

void CDnChatTabDlg::SetPrivateName(const std::wstring &strPrivateName)
{
	m_strPrivateUserName = strPrivateName;

	SetChatMode(CHAT_PRIVATE);
	m_pIMEEditBox->ClearText();
	RequestFocus(m_pIMEEditBox);
}

bool CDnChatTabDlg::GetLastPrivateName(std::wstring &strLastPrivateName)
{
	if (!m_strLastPrivateUserName.empty())
	{
		strLastPrivateName = m_strLastPrivateUserName;
		return true;
	}

	return false;
}

void CDnChatTabDlg::SetLastPrivateName(LPCWSTR wszLastPrivateName)
{
	ASSERT(wszLastPrivateName&&"CDnChatTabDlg::SetLastPrivateName");
	m_strLastPrivateUserName = wszLastPrivateName;
}

void CDnChatTabDlg::OnUICallbackProc(int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg)
{
	switch (nID)
	{
	case CHAT_HELP_DIALOG:
		if (nCommand == EVENT_TEXTBOX_SELECTION)
		{
			CEtUITextBox *pTextBox = (CEtUITextBox *)pControl;
			if (!pTextBox) return;

			SLineData sLineData;
			if (pTextBox->GetSelectedLineData(sLineData))
			{
				SetChatMode(sLineData.m_nData);
			}

			ShowChildDialog(m_pChatHelpDlg, false);
			RequestFocus(m_pIMEEditBox);
		}
		break;
	}
}

void CDnChatTabDlg::AddChatHistotry(LPCWSTR wszChat)
{
	if (m_listChatHistory.size() > 20)
	{
		m_listChatHistory.pop_back();
		m_listChatHistory.push_front(wszChat);
	}
	else
	{
		m_listChatHistory.push_front(wszChat);
	}
}

void CDnChatTabDlg::SetPrevChatHistory()
{
	if (!m_listChatHistory.empty())
	{
		std::wstring strChat = m_listChatHistory.back();
		m_listChatHistory.pop_back();
		m_listChatHistory.push_front(strChat);

		m_pIMEEditBox->SetText(strChat.c_str());
	}
}

void CDnChatTabDlg::SetNextChatHistory()
{
	if (!m_listChatHistory.empty())
	{
		std::wstring strChat = m_listChatHistory.front();
		m_listChatHistory.pop_front();
		m_listChatHistory.push_back(strChat);

		m_pIMEEditBox->SetText(strChat.c_str());
	}

	return;
}

void CDnChatTabDlg::CustomizeChatMode(bool bTrue, int nModeType, LPCWSTR wszHeader, DWORD dwTextColor)
{
	m_bCustomizeChatMode = bTrue;
	m_nCustomizeModeType = nModeType;
	m_wszCustomizeModeHeader = wszHeader;
	m_dwCustomizeModeTextColor = dwTextColor;

	SetChatModeText();

};

bool CDnChatTabDlg::IsGuildWarMaster(LPCWSTR wszFromCharName)
{
	if (CDnBridgeTask::GetInstance().IsPvPGameMode(PvPCommon::GameMode::PvP_GuildWar))
	{
		int nPvPUserState = GetGuildWarUserState(wszFromCharName);

		if (PvPCommon::UserState::GuildWarCaptain & nPvPUserState
			|| PvPCommon::UserState::GuildWarSedcondCaptain & nPvPUserState)
			return true;
	}

	return false;
}

UINT CDnChatTabDlg::GetGuildWarUserState(LPCWSTR wszFromCharName)
{
	CDnPartyTask::PartyStruct * pData = GetPartyTask().GetPartyDataFromMemberName(wszFromCharName);

	if (pData)
	{
		if (!CDnLocalPlayerActor::s_hLocalActor) return 0;
		CDnLocalPlayerActor *pLocalActor = dynamic_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
		if (!pLocalActor) return 0;
		if (pLocalActor->GetTeam() != pData->usTeam)
			return 0;

		return pData->uiPvPUserState;
	}

	return 0;
}

void CDnChatTabDlg::ClearText()
{
	m_pIMEEditBox->ClearText();
}