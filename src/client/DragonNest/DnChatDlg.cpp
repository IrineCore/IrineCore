#include "StdAfx.h"
#include "DnChatDlg.h"
#include "DnChatOptDlg.h"
#include "DebugCmdProc.h"
#include "DnChatTabDlg.h"
#include "DnPartyTask.h"
#include "DnChatTabDlg.h"
#include "TaskManager.h"
#include "DnNameLinkMng.h"
#include "DnInterface.h"
#include "DnCustomControlCommon.h"
#include "DnTooltipDlg.h"
#include "DnContextMenuDlg.h"
#include "DnPVPLobbyVillageTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChatDlg::CDnChatDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pChatTextBox(NULL)
	, m_nChatType(CHAT_NO)
	, m_pButtonDummy(NULL)
	//, m_pStaticBase(NULL)
	, m_bIsShowScrollBar(false)
	, m_pChatOptDlg( NULL )
{
}

CDnChatDlg::~CDnChatDlg(void)
{
	SAFE_DELETE( m_pChatOptDlg );
}

void CDnChatDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChatDialog.ui" ).c_str(), bShow );
}

void CDnChatDlg::InitialUpdate()
{
	m_pChatOptDlg = new CDnChatOptDlg( UI_TYPE_CHILD, this );
	m_pChatOptDlg->Initialize( false );

	m_pChatTextBox = GetControl<CEtUITextBox>("ID_TEXTBOX_CHAT");

	m_pButtonDummy = GetControl<CEtUIButton>("ID_BUTTON_DUMMY");
	if( m_pButtonDummy )
		m_pButtonDummy->Show(false);
}

bool CDnChatDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hWnd;

	if( !m_bShow )
	{
		return false;
	}

	if( uMsg == WM_MOUSEWHEEL )
	{
		// Allow wheel messages to work only when the chat tab is visible.
		// Other messages (clicks, etc.) must be processed even if the chat tab is hidden because ChatDlg is activated by pressing BUTTON_DUMMY.
		CDnChatTabDlg *pChatDialog = (CDnChatTabDlg*)GetParentDialog();
		if( !pChatDialog->IsShow() )
		{
			return false;
		}

		POINT MousePoint;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );

		float fMouseX, fMouseY;
		PointToFloat( MousePoint, fMouseX, fMouseY );

		if( IsMouseInDlg() )
		{
			if( m_pChatTextBox->HandleMouse( uMsg, fMouseX, fMouseY, wParam, lParam ) )
			{
				return true;
			}
		}
	}


	bool bCloseContextMenu = false;
	if ( uMsg == WM_LBUTTONDOWN )
	{
		if (IsMouseInDlg())
			SetRenderPriority(this, true);
		bCloseContextMenu = true;
	}

	if (uMsg == WM_KEYDOWN)
		bCloseContextMenu = true;

	if (bCloseContextMenu)
	{
		CDnContextMenuDlg* pDlg = GetInterface().GetContextMenuDlg(CDnInterface::CONTEXTM_CHAT);
		if (pDlg && pDlg->IsShow())
			pDlg->Show(false);
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnChatDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_DUMMY" ) )
		{
			CDnChatTabDlg *pChatDialog = (CDnChatTabDlg*)GetParentDialog();
			pChatDialog->ShowEx(true);
			return;
		}
	}
	else if( nCommand == EVENT_TEXTBOX_SELECTION )
	{
		if( IsCmdControl("ID_TEXTBOX_CHAT" ) )
		{
			CDnChatTabDlg *pChatDialog = (CDnChatTabDlg*)GetParentDialog();
			if( !pChatDialog->IsShow() )
				return;

			CWord selectedWord = m_pChatTextBox->GetSelectedWordData();
			if (selectedWord.m_strWordWithTag.empty() == false)
				EtInterface::GetNameLinkMgr().TranslateText(std::wstring(), selectedWord.m_strWordWithTag.c_str(), this);
		}
	}
}

void CDnChatDlg::InitOption( int nChatType )
{
	m_nChatType = nChatType;
	m_pChatOptDlg->InitOption( nChatType );
}


void CDnChatDlg::AddChat( LPCWSTR wszName, LPCWSTR szMessage, int nType, DWORD dwFormat, bool isAppend, bool hasColor, DWORD colorValue, DWORD bgColorValue, bool bIgnoreCarrageReturn, bool bOnResetDevice )
{

	if( !bOnResetDevice )
	{
		SAddChatData Data;
		if( wszName != NULL ) Data.wszName = wszName;
		if( szMessage != NULL ) Data.szMessage = szMessage;
		Data.nChatType = nType;
		Data.dwFormat = dwFormat;
		Data.isAppend = isAppend;
		Data.hasColor = hasColor;
		Data.colorValue = colorValue;
		Data.bgColorValue = bgColorValue;
		Data.bIgnoreCarrageReturn = bIgnoreCarrageReturn;
		m_listAddChatData.push_back(Data);
		if( (int)m_listAddChatData.size() > 150 )
			m_listAddChatData.pop_front();
	}

	DWORD dwTextColor(textcolor::WHITE);

	switch( nType )
	{
	case CHAT_NORMAL:	dwTextColor = chatcolor::NORMAL;	break;
	case CHAT_PARTY:	dwTextColor = chatcolor::PARTY;		break;
	case CHAT_GUILD:	dwTextColor = chatcolor::GUILD;		break;
	case CHAT_PRIVATE:	dwTextColor = chatcolor::PRIVATE;	break;
	case CHAT_CHANNEL:	dwTextColor = chatcolor::CHANNEL;	bgColorValue = chatcolor::CHANNELBACK;	break;
	case CHAT_WORLD:	dwTextColor = chatcolor::WORLD;		bgColorValue = chatcolor::WORLDBACK;	break;
	case CHAT_SYSTEM:	dwTextColor = textcolor::GOLD;		break;
	case CHAT_RAIDNOTICE: dwTextColor = chatcolor::RAIDNOTICE; break;
	case CHAT_GUILDWAR : dwTextColor = chatcolor::RAIDNOTICE; break;
#ifdef PRE_PRIVATECHAT_CHANNEL
	case CHAT_PRIVATE_CHANNEL:	dwTextColor = chatcolor::PRIVATECHANNEL;	break;
#endif // PRE_PRIVATECHAT_CHANNEL
#ifdef PRE_ADD_WORLD_MSG_RED
	case CHAT_WORLD_RED:	dwTextColor = chatcolor::WORLD_RED;		bgColorValue = chatcolor::WORLD_REDBACK;	break;
#endif // PRE_ADD_WORLD_MSG_RED
	case CHAT_NOTICE:
		{
			dwTextColor = textcolor::HOTPINK;
			m_pChatTextBox->SetText( szMessage, dwTextColor );
		}
		return;
	}

	if(GetInterface().GetChatTabDialog()->IsCustomizeChatMode() && GetInterface().GetChatTabDialog()->GetCustomizeChatMode() == nType)
	{
		dwTextColor = GetInterface().GetChatTabDialog()->GetCustomizeChatColor();
	}

	if( wszName != NULL && wszName[0] != '\0')
	{
		SLineData sLineData;
		sLineData.m_strData = wszName;
		m_pChatTextBox->SetLineData( sLineData );
	}

	std::vector<std::wstring> sentences;
	if (bIgnoreCarrageReturn)
	{
		sentences.push_back(szMessage);
	}
	else
	{
		TokenizeW(szMessage, sentences, L"\n");
	}

	std::vector<std::wstring>::const_iterator iter = sentences.begin();
	for (; iter != sentences.end(); ++iter)
	{
		const std::wstring& message = *iter;

		if (isAppend)
		{
			if (hasColor) {
				dwTextColor = colorValue;
			}
			m_pChatTextBox->AppendText(message.c_str(), dwTextColor, dwFormat, true, bgColorValue);
		}
		else
		{
			bool bAutoScroll = false;

			if( m_pChatTextBox->IsLastPage() || m_pChatTextBox->IsEmpty() )
				bAutoScroll = true;

			// Apply auto-scroll for own messages even if scrolled up.
			if( !bAutoScroll && wszName ) {
				bool bMyChat = false;
				if( CDnActor::s_hLocalActor ) {
					if( __wcsicmp_l( wszName, CDnActor::s_hLocalActor->GetName() ) == 0 )
						bMyChat = true;
				}
				else {
					if( CTaskManager::GetInstance().GetTask( "PVPLobbyTask" ) ) {
						// In PVP lobby, character processing is unnecessary so s_hLocalActor may not exist.
						// Fetch from party task local data and compare here.
						if( CDnPartyTask::IsActive() && __wcsicmp_l( wszName, GetPartyTask().GetLocalData()->wszCharacterName ) == 0 )
							bMyChat = true;
					}
				}
				if( bMyChat )
					bAutoScroll = true;
			}
			m_pChatTextBox->SetAutoScroll( bAutoScroll );

			// Use ColorText for system messages added via AddText.
			// Doing it this way currently makes it impossible to use the system message's dwFormat.
			// To be exact, ColorText should only be used when color tags like #r, #g are present,
			// Since system messages have always used UITEXT_NONE anyway, leave it as-is for now.
// 			if( nType == CHAT_SYSTEM )	// commented by kalliste
// 				m_pChatTextBox->AddColorText( message.c_str(), dwTextColor, bgColorValue );
// 			else
			m_pChatTextBox->AddText( message.c_str(), dwTextColor,  dwFormat, bgColorValue );
		}
	}
}

bool CDnChatDlg::IsShowOptDlg()
{ 
	return m_pChatOptDlg->IsShow();
}

void CDnChatDlg::ShowOptDlg( bool bShow )			
{ 
	ShowChildDialog( m_pChatOptDlg, bShow );
}

void CDnChatDlg::UpdateDlgCoord( float fX, float fY, float fWidth, float fHeight )
{
	CEtUIDialog::UpdateDlgCoord( fX, fY, fWidth, fHeight );

	m_pChatOptDlg->UpdateDlgCoord( 0, -fHeight, 0, 0 );

	m_pChatTextBox->UpdateUICoord( 0, 0, 0, fHeight );

	if( m_pButtonDummy )
		m_pButtonDummy->UpdateUICoord( 0, 0, 0, fHeight );
}

void CDnChatDlg::ShowScrollBar( bool bShow )
{
	SUIControlProperty *pProperty = m_pChatTextBox->GetProperty();
	pProperty->TextBoxProperty.bVerticalScrollBar = bShow;

	if( m_pButtonDummy )
		m_pButtonDummy->Show(!bShow);

	m_bIsShowScrollBar = bShow;
}

bool CDnChatDlg::IsTextBoxEmpty()
{
	return m_pChatTextBox->IsEmpty();
}

void CDnChatDlg::OnChangeResolution()
{
	m_pChatTextBox->ClearText();
	CEtUIDialog::OnChangeResolution();
	for( std::list<SAddChatData>::iterator iter = m_listAddChatData.begin(); iter != m_listAddChatData.end(); ++iter )
	{
		SAddChatData &Data = *iter;
		LPCWSTR wszName = NULL;
		if( (int)Data.wszName.size() ) wszName = Data.wszName.c_str();	// Name can be handled as NULL.
		AddChat( wszName, Data.szMessage.c_str(), Data.nChatType, Data.dwFormat, Data.isAppend, Data.hasColor, Data.colorValue, Data.bgColorValue, Data.bIgnoreCarrageReturn, true );
	}
}

bool CDnChatDlg::OnParseTextItemInfo(const std::wstring& argString)
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

bool CDnChatDlg::OnParseTextUserName(const std::wstring& name)
{
	float mousePointX, mousePointY;
	GetMouseMovePoints(mousePointX, mousePointY);
	mousePointX += GetXCoord();
	mousePointY += GetYCoord();
	ShowChatContextMenuDlg(true, mousePointX, mousePointY, name);

	return true;
}

void CDnChatDlg::ShowChatContextMenuDlg(bool bShow, float fX, float fY, const std::wstring& name)
{
	CDnContextMenuDlg* pDlg = GetInterface().GetContextMenuDlg(CDnInterface::CONTEXTM_CHAT);
	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );

	if (pDlg)
	{
		if (CDnActor::s_hLocalActor)
		{
			if (name.compare(CDnActor::s_hLocalActor->GetName()) == 0)
				return;
		}
		else if( pPVPLobbyTask )
		{
			if( __wcsicmp_l( name.c_str(), pPVPLobbyTask->GetUserInfo().wszCharacterName ) == 0 ) 
				return;
		}

		CDnChatContextMenuDlg::SContextMenuChatInfo info;
		info.name = name;

		SUICoord Coord = pDlg->GetBackgroundUISize();

		if( fY + Coord.fHeight > 1.f )
			pDlg->SetPosition( fX, 1.f - Coord.fHeight );
		else
			pDlg->SetPosition(fX, fY);

		pDlg->GetDlgCoord( Coord );

		pDlg->SetInfo(&info);
		pDlg->Show(bShow);

		if (bShow)
			SetRenderPriority(pDlg, true);
	}
}