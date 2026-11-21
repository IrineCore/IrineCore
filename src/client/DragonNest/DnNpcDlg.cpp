#include "StdAfx.h"
#include "DnNpcDlg.h"
#include "EtUITextBox.h"
#include "EtUIStatic.h"
#include "DnInterface.h"
#include "DnTableDB.h"
#include "EtUITextureControl.h"
#include "DnNpcQuestRecompenseDlg.h"
#include "NpcSendPacket.h"
#include "DnLocalPlayerActor.h"
#include "DnInvenTabDlg.h"
#include "DnMainMenuDlg.h"
#include "DnCommonTask.h"
#include "TaskManager.h"
#include "DnItemTask.h"
#ifdef PRE_ADD_AIM_ASSIST_BUTTON
#include "GameOption.h"
#endif
#include "DnCashShopTask.h"
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#include "DnNpcAcceptPresentDlg.h"
#include "DnNpcActor.h"
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#include "DnMainDlg.h"
#include "DnQuestTask.h"
#include "DnStoreConfirmExDlg.h"
#include "DnNPCHotKeyInfoDlg.h"
#include "DnMainQuestDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnNpcDlg::CDnNpcDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pTextBoxMain(NULL)
, m_pTextBoxAnswer(NULL)
, m_pStaticName(NULL)
#ifdef PRE_DEL_NPCDLG_NEXTPAGE_FLAG
#else
, m_bNextPage(false)
#endif	// #ifdef PRE_DEL_NPCDLG_NEXTPAGE_FLAG
, m_pBGTextureCtl(NULL)
, m_pItemTextureCtl(NULL)
, m_pNPCQuestRecompenseDlg(NULL)
, m_pItem(NULL)
, m_pSlotBtn( NULL )
, m_bHideNextBlindOpened(false)
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
, m_pNpcAcceptPresentDlg( NULL )
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
, m_pCloseBtn(NULL)
{
#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	m_bSkipOpen = false;
#endif // PRE_ADD_MAILBOX_OPEN
}

CDnNpcDlg::~CDnNpcDlg(void)
{
	SAFE_RELEASE_SPTR( m_hNpcImage );
	SAFE_DELETE( m_pNPCQuestRecompenseDlg );
	SAFE_DELETE( m_pItem );
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	SAFE_DELETE( m_pNpcAcceptPresentDlg );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
}

void CDnNpcDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "NpcDlg.ui" ).c_str(), bShow );
}

void CDnNpcDlg::InitialUpdate()
{
	m_pStaticName = GetControl<CEtUIStatic>( "ID_STATIC_NAME" );
	m_pTextBoxMain = GetControl<CEtUIHtmlTextBox>( "ID_TEXTBOX_MAIN" );
	m_pTextBoxAnswer = GetControl<CDnNpcAnswerHtmlTextBox>( "ID_TEXTBOX_ANSWER" );
	m_pBGTextureCtl = GetControl<CEtUITextureControl>( "ID_BACK" );
	m_pItemTextureCtl = GetControl<CEtUITextureControl>( "ID_ITEM" );
	m_pCloseBtn = GetControl<CEtUIButton>( "ID_BUTTON_CLOSE" );

	m_pTextBoxAnswer->Show(false);

	m_pTextBoxMain->GetUICoord(m_MainTextBoxDummy);
	m_pTextBoxAnswer->GetUICoord(m_AnswerTextBoxDummy);

	m_pTextBoxMain->SetDefaultTextColor( textcolor::NPC_SPEECH );
	m_pTextBoxAnswer->SetDefaultTextColor( textcolor::NPC_SPEECH );

	m_pNPCQuestRecompenseDlg = new CDnNpcQuestRecompenseDlg( UI_TYPE_CHILD, this );
	m_pNPCQuestRecompenseDlg->Initialize( false );

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	m_pNpcAcceptPresentDlg = new CDnNpcAcceptPresentDlg( UI_TYPE_CHILD, this );
	m_pNpcAcceptPresentDlg->Initialize( false );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	m_bReadyMove = false;
#endif

}

void CDnNpcDlg::InitCustomControl(CEtUIControl *pControl)
{
	if (pControl == NULL || !strstr(pControl->GetControlName(), "ID_SLOT_BUTTON"))
		return;

	m_pSlotBtn = static_cast<CDnItemSlotButton*>(pControl);
	m_pSlotBtn->SetSlotType(ST_QUEST);
	m_pSlotBtn->SetSlotIndex(0);
}

bool CDnNpcDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	CDnBlindDlg *pBlindDlg = GetInterface().GetBlindDialog();
	if( !pBlindDlg ) return false;
	if( pBlindDlg->GetBlindMode() != CDnBlindDlg::modeOpened )
		return false;

	CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
	if( pTask && false == pTask->IsRequestNpcTalk() )
	{
		if( uMsg == WM_KEYDOWN )
		{
			if( wParam == VK_ESCAPE )
			{
				if( m_pCallback ) 
					m_pCallback->OnUICallbackProc( GetDialogID(), EVENT_BUTTON_CLICKED, GetControl("ID_BUTTON_CLOSE") );

				pTask->SetNpcTalkRequestWait( false );
				GetInterface().CloseNpcDialog();
				return true;
			}
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnNpcDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
		if( IsCmdControl("ID_BUTTON_CLOSE" ) && pTask && false == pTask->IsRequestNpcTalk() )
		{
			if( m_pCallback ) 
				m_pCallback->OnUICallbackProc( GetDialogID(), EVENT_BUTTON_CLICKED, GetControl("ID_BUTTON_CLOSE") );

			pTask->SetNpcTalkRequestWait( false );
			GetInterface().CloseNpcDialog();
			return;
		}
	}
	else if( nCommand == EVENT_TEXTBOX_SELECTION )
	{
#ifdef PRE_DEL_NPCDLG_NEXTPAGE_FLAG
#else
		if( m_bNextPage )
		{
			m_pTextBoxMain->ScrollPageTurn();
			m_pTextBoxAnswer->ClearText();
			m_pTextBoxAnswer->Show(false);
			m_bNextPage = false;
			return;
		}
#endif	// #ifdef PRE_DEL_NPCDLG_NEXTPAGE_FLAG

		// Note: If the window is opened to check the final reward item
		//
		if( m_pNPCQuestRecompenseDlg->IsShow() )
		{
			if( m_pNPCQuestRecompenseDlg->IsRecompense() )
			{
				if( !m_pNPCQuestRecompenseDlg->IsCompleteCheck() )
				{
					int nCnt = m_pNPCQuestRecompenseDlg->GetAvailableCount();
					std::wstring strMsg;
					strMsg = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1199999 ), nCnt );

					GetInterface().MessageBox( strMsg.c_str(), MB_OK );
					return;
				}

				bool bItemArray[QUESTREWARD_INVENTORYITEMMAX] = {0,};
				bool bCashItemArray[MAILATTACHITEMMAX] = {0,};
				m_pNPCQuestRecompenseDlg->GetRecompenseItemArray( bItemArray, bCashItemArray );

				int nItemCount = 0, nCashItemCount = 0;
				for (int nIndex = 0 ; _countof(bItemArray) > nIndex ; ++nIndex) {
					if (!bItemArray[nIndex]) {
						continue;
					}
					++nItemCount;
				}
				for (int nIndex = 0 ; _countof(bCashItemArray) > nIndex ; ++nIndex) {
					if (!bCashItemArray[nIndex]) {
						continue;
					}
					++nCashItemCount;
				}

				if( ( (0 < nItemCount) || ( (0 < nCashItemCount) && (nItemCount != 0) ) ) )
				{
					CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
					if( pInvenDlg ) 
					{
						if( pInvenDlg->GetEmptySlotCount() < nItemCount )
						{
							GetInterface().MessageBox( 1925, MB_OK );
							return;
						}
					}
				}

				SendSelectQuestReward( m_pNPCQuestRecompenseDlg->GetRecompenseID(), bItemArray, bCashItemArray );
			}

			ShowChildDialog( m_pNPCQuestRecompenseDlg, false );
		}
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
		else
		if( m_pNpcAcceptPresentDlg->IsShow() )
		{
			CEtUITextBox* pTextBox = static_cast<CEtUITextBox*>( pControl );
			int iSelectedIndex = pTextBox->GetSelectedLineIndex( false );

			if( 0 == iSelectedIndex &&		// The first item is Gift.
				false == m_pNpcAcceptPresentDlg->IsPresentSelected() )
			{
				//std::wstring strMsg;
				//strMsg = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1199999 ), nCnt );

				//GetInterface().MessageBox( L"Please select a gift~ (UIString needed)", MB_OK );
				GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3208), MB_OK );
				return;
			}

			if (iSelectedIndex != 0)
				ShowChildDialog( m_pNpcAcceptPresentDlg, false );
		}
#endif
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnNpcDlg::Render( float fElapsedTime )
{
	if( !IsShow() )
		return;

	if( m_pTextBoxMain->IsLastPage() )
	{
		// Note: Resize text box
		m_pTextBoxMain->FitTextBoxToLine();

		SUICoord uiCoordMain, uiCoordAnswer;
		m_pTextBoxMain->GetUICoord(uiCoordMain);
		m_pTextBoxAnswer->GetUICoord(uiCoordAnswer);

		float fY = uiCoordMain.Bottom() + 0.0065f;
		float fHeight = uiCoordAnswer.fY - fY;
		uiCoordAnswer.fY = fY;
		uiCoordAnswer.fHeight += fHeight;
		m_pTextBoxAnswer->SetUICoord(uiCoordAnswer);
	}
#ifdef PRE_DEL_NPCDLG_NEXTPAGE_FLAG
#else
	else if( m_pTextBoxAnswer->IsEmpty() )
	{
		//m_pTextBoxAnswer->SetText( CDnTableDB::GetInstance().GetUIString(68) );
		m_pTextBoxAnswer->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 68) );
		m_pTextBoxAnswer->Show(true);
		m_bNextPage = true;
	}
#endif	// #ifdef PRE_DEL_NPCDLG_NEXTPAGE_FLAG

	if( !m_pTextBoxAnswer->IsEmpty() )
	{
		float fMouseX, fMouseY;
		GetMouseMovePoints(fMouseX, fMouseY);
		m_pTextBoxAnswer->HandleMouse( WM_MOUSEMOVE, fMouseX, fMouseY, 0, 0 );
	}

	CEtUIDialog::Render( fElapsedTime );
}

void CDnNpcDlg::Show( bool bShow )
{ 
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
#ifdef NPC_ANSWER_KEYBOARD_SELECT
		RequestFocus( m_pTextBoxAnswer );
#endif
	}
	else
	{
		SAFE_RELEASE_SPTR( m_hNpcImage );
		ShowChildDialog( m_pNPCQuestRecompenseDlg, false );
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
		ShowChildDialog( m_pNpcAcceptPresentDlg, false );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
		if( GetInterface().GetNPCHotKeyInfoDlg() && GetInterface().GetNPCHotKeyInfoDlg()->IsShow() )
			GetInterface().GetNPCHotKeyInfoDlg()->Show( false );
		if( GetInterface().GetMainQuestDlg() && GetInterface().GetMainQuestDlg()->IsShow() )
			GetInterface().GetMainQuestDlg()->Show( false );
	}

	CEtUIDialog::Show( bShow );
	//CDnMouseCursor::GetInstance().ShowCursor(bShow);
}

void CDnNpcDlg::SetNpcTalk( LPCWSTR wszNpcTalk, const std::string &strImageName, int nItemIndex )
{
	ASSERT(wszNpcTalk);
	ASSERT(m_pTextBoxMain);

	m_pTextBoxMain->SetUICoord(m_MainTextBoxDummy);
	m_pTextBoxAnswer->SetUICoord(m_AnswerTextBoxDummy);

	m_pStaticName->ClearText();
	m_pTextBoxMain->ClearText();

	std::wstring strNpcTalk(wszNpcTalk);
	std::wstring::size_type index_s = strNpcTalk.find_first_of( L"$" );
	std::wstring::size_type index_e = strNpcTalk.find_last_of( L"$" );

	if( index_s == std::wstring::npos )
	{
		m_pTextBoxMain->ReadHtmlString( strNpcTalk.c_str() );
	}
	else
	{
		std::wstring strNpcName = strNpcTalk.substr( index_s+1, index_e-index_s-1 );
		std::wstring strNpcText =strNpcTalk.erase( index_s, index_e-index_s+1 );

		m_pStaticName->SetText( strNpcName );
		m_pTextBoxMain->ReadHtmlString( strNpcText.c_str() );
	}

	if( !strImageName.empty() )
	{
		SAFE_RELEASE_SPTR( m_hNpcImage );
		m_hNpcImage = LoadResource( CEtResourceMng::GetInstance().GetFullName( strImageName.c_str() ).c_str(), RT_TEXTURE );
		if ( m_hNpcImage )
		{
			m_pBGTextureCtl->SetTexture(m_hNpcImage, 0, 0, 256, 128);
			m_pBGTextureCtl->SetManualControlColor( true );
			m_pBGTextureCtl->SetTextureColor( 0xAAFFFFFF );
			m_pBGTextureCtl->Show( true );

			if( nItemIndex > 0 ) {
				TItemInfo itemInfo;
				CDnItem::MakeItemInfo( nItemIndex, 1, itemInfo );
				SAFE_DELETE( m_pItem );
				m_pItem = GetItemTask().CreateItem( itemInfo );
				if( m_pSlotBtn) {
					m_pSlotBtn->SetItem(m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);												
					m_pSlotBtn->Show( true );
				}
			}
			else {
				m_pItemTextureCtl->Show( false );
				if(m_pSlotBtn)m_pSlotBtn->Show( false );
			}
		}
		else
		{
			m_pBGTextureCtl->Show( false );
			m_pItemTextureCtl->Show( false );
			if(m_pSlotBtn)m_pSlotBtn->Show( false );
		}

	}
	else
	{
		SAFE_RELEASE_SPTR( m_hNpcImage );
		m_pBGTextureCtl->Show( false );
		m_pItemTextureCtl->Show( false );
		if(m_pSlotBtn)m_pSlotBtn->Show( false );
	}

	//GetInterface().OpenNotifyArrowDialog(EtVector3(0,0,0), 1000*7, false);
	if( GetInterface().GetMainQuestDlg() && GetInterface().GetMainQuestDlg()->IsShow() )
		GetInterface().GetMainQuestDlg()->SetNpcTalkText( m_pStaticName ? m_pStaticName->GetText() : L"", wszNpcTalk );
	if( GetInterface().GetNPCHotKeyInfoDlg() && m_pStaticName )
	{
		GetInterface().GetNPCHotKeyInfoDlg()->ClearNpcHotKeys();
		GetInterface().GetNPCHotKeyInfoDlg()->SetNpcHotKey( 0, CDnLocalPlayerActor::GetTakeNpcUID(), m_pStaticName->GetText(), L"F" );
	}
}

void CDnNpcDlg::SetAnswer( std::vector<TALK_ANSWER>& answers, bool bIsTalk)
{
	// Note : Call AddText as many times as the number of Answers.
	//
	ASSERT(m_pTextBoxAnswer);
	m_pTextBoxAnswer->ClearText();
	m_pTextBoxAnswer->Show(false);

	m_pCloseBtn->Enable( true );
	m_pCloseBtn->Show(true);

	std::wstring wszAnswer;
	for( size_t i=0; i<answers.size() ; i++ )
	{
		wszAnswer += answers[i].szAnswer;
		if ( i < answers.size()-1 )
			wszAnswer += L"<BR>";

		//m_pTextBoxAnswer->AddText( answers[i].szAnswer.c_str(), textcolor::WHITE, UITEXT_SYMBOL );
	}

	RemoveStringW(wszAnswer, std::wstring(L"<html>"));
	RemoveStringW(wszAnswer, std::wstring(L"</html>"));
	RemoveStringW(wszAnswer, std::wstring(L"<HTML>"));
	RemoveStringW(wszAnswer, std::wstring(L"</HTML>"));

	wszAnswer.insert(0, L"<HTML>");
	wszAnswer += L"</HTML>";
	// Ignore if the last line is a newline.
	AllReplaceW(wszAnswer, std::wstring(L"<BR></HTML>"), std::wstring(L"</HTML>") );
	m_pTextBoxAnswer->ReadHtmlString(wszAnswer.c_str());

	m_answers = answers;
	/*
	if( !m_answers.empty() )
	{
	m_pTextBoxAnswer->Enable(false);
	m_pTextBoxAnswer->Show(true);
	m_pTextBoxAnswer->SetBlendRate( 0.f );
	}
	*/
	if( !m_answers.empty() ) {
		m_pTextBoxAnswer->Show(true);
	}

#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	// Unlike other windows, smart move in Show function does not solve it.
	// Because handling must differ depending on whether choices exist or not.
	// It's messy, but let's handle the exception here.
	if( CGameOption::GetInstance().m_bSmartMoveMainMenu )
	{
		if( m_answers.empty() )
		{
			SUICoord uiCoord;
			m_pCloseBtn->GetUICoord( uiCoord );
			m_fMovePosX = GetXCoord() + uiCoord.fX + uiCoord.fWidth / 2.0f;
			m_fMovePosY = GetYCoord() + uiCoord.fY + uiCoord.fHeight / 2.0f;
			m_bReadyMove = true;
		}
		else
		{
			std::vector<EtVector2> vecControlPos;
			m_pTextBoxAnswer->GetLinePos( vecControlPos );
			if( !vecControlPos.empty() )
			{
				m_fMovePosX = vecControlPos[0].x;
				m_fMovePosY = vecControlPos[0].y;

				// After actually smart-moving to this position
				// Pressing the auto-aim key again moves the mouse.
				// That is, the position computed right after setting Answer initially differs slightly from the actual rendered position,
				// This is because UpdateVisibleCount is called before rendering starts, and an error occurs when the area is recalculated at that time.
				// Fixing this properly requires refactoring internals, so leave it like this for now.
				//m_fMovePosY -= m_pTextBoxAnswer->GetProperty()->TextBoxProperty.fLineSpace;

				m_bReadyMove = true;
			}
		}
	}
#endif
}

void CDnNpcDlg::Process( float fElapsedTime )
{
#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	ProcessSmartMoveCursor();
#endif

	CEtUIDialog::Process( fElapsedTime );

#ifdef NPC_ANSWER_KEYBOARD_SELECT
	if( IsShow() )
	{
		if( GetInterface().IsFocusEditBox() == false )
		{
			if( focus::GetControl() == NULL )
			{
				RequestFocus( m_pTextBoxAnswer );
			}
			else if( focus::GetControl() != NULL )
			{
				if( focus::GetControl() != m_pTextBoxAnswer && focus::GetControl()->IsPressed() == false && focus::GetControl()->IsRightPressed() == false )
					RequestFocus( m_pTextBoxAnswer );
			}
		}
	}
#endif
}

#ifdef PRE_ADD_AIM_ASSIST_BUTTON
void CDnNpcDlg::ProcessSmartMoveCursor()
{
	if( !IsAllowRender() || !m_bReadyMove ) return;

	int nX, nY;
	nX = (int)(GetScreenWidth() * m_fMovePosX);
	nY = (int)(GetScreenHeight() * m_fMovePosY);
	CDnMouseCursor::GetInstance().SetCursorPos( nX, nY );

	if( GetInterface().GetAimKeyState() )
	{
		RECT rcMouse;
		POINT pt;
		pt.x = nX;
		pt.y = nY;
		ClientToScreen( GetHWnd(), &pt );
		rcMouse.left = rcMouse.right = pt.x;
		rcMouse.top = rcMouse.bottom = pt.y;
		//CDnMouseCursor::GetInstance().ClipCursor( &rcMouse );
		ClipCursor( &rcMouse );
	}

	m_bReadyMove = false;
}

void CDnNpcDlg::GetSmartMovePos( std::vector<EtVector2> &vecSmartMovePos )
{
	if( !IsAllowRender() ) return;

	float fX, fY;

	SUICoord uiCoord;
	m_pCloseBtn->GetUICoord( uiCoord );
	fX = GetXCoord() + uiCoord.fX + uiCoord.fWidth / 2.0f;
	fY = GetYCoord() + uiCoord.fY + uiCoord.fHeight / 2.0f;
	vecSmartMovePos.push_back( EtVector2(fX, fY) );

	if( !m_answers.empty() )
	{
		// To do this properly, it should also auto-scroll the scrollbar down...
		// For now, try getting only the visible lines.
		std::vector<EtVector2> vecControlPos;
		m_pTextBoxAnswer->GetLinePos( vecControlPos );
		for( int i = 0; i < (int)vecControlPos.size(); ++i )
			vecSmartMovePos.push_back( EtVector2(vecControlPos[i].x, vecControlPos[i].y) );
	}
}
#endif

bool CDnNpcDlg::GetAnswerIndex(OUT std::wstring& szIndex, OUT std::wstring& szTarget, bool bClearTextBoxSelect) const
{
	ASSERT(m_pTextBoxAnswer);
	// Because a line may wrap, we must not calculate it simply by line index like this.
	//int nIdx =  m_pTextBoxAnswer->GetSelectedLineIndex();
	SLineData LineData;
	m_pTextBoxAnswer->GetSelectedLineData(LineData, bClearTextBoxSelect);
	int nIdx = LineData.m_nData;
	if ( nIdx < 0  ) 
		return false;
	if ( (int)m_answers.size() <= nIdx )
	{
		return false;
	}

	szIndex = m_answers[nIdx].szLinkIndex;
	szTarget = m_answers[nIdx].szLinkTarget;

	return true;
}

int CDnNpcDlg::GetAnswerLineIndex()
{
	SLineData LineData;
	m_pTextBoxAnswer->GetSelectedLineData(LineData);

	int nIdx = LineData.m_nData;
	if ( nIdx < 0  || nIdx > (int)m_answers.size()) 
		return -1;

	return nIdx;
}

void CDnNpcDlg::Open()
{
	if( m_bHideNextBlindOpened == false )
		Show(true);
	m_bHideNextBlindOpened = false;
}

void CDnNpcDlg::Close()
{
	Show(false);
	SetCallback(NULL);
}

void CDnNpcDlg::SetRecompense( const TQuestRecompense &questRecompense, const bool bRecompense )
{
	if( !m_pNPCQuestRecompenseDlg ) 
		return;

	m_pNPCQuestRecompenseDlg->SetRecompense( questRecompense, bRecompense );

	SetLevelCapRecompense( questRecompense );
	ShowChildDialog( m_pNPCQuestRecompenseDlg, true );
}

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
void CDnNpcDlg::SetAcceptPresent( int nNpcID )
{
	m_pNpcAcceptPresentDlg->SetPresent( nNpcID );

	ShowChildDialog( m_pNpcAcceptPresentDlg, true );
}

bool CDnNpcDlg::OnAnswer(int nNpcID)
{
	std::wstring curIndex, curTarget;
	GetAnswerIndex(curIndex, curTarget, false);
	if (curIndex.empty() == false && curIndex.empty() == false)
	{
		std::transform(curIndex.begin(), curIndex.end(), curIndex.begin(), ::tolower);
		if (curIndex.compare(L"gift") == 0)
		{
			OpenSelectedPresentCountDlg(nNpcID);
			return true;
		}
	}

	return false;
}

void CDnNpcDlg::OpenSelectedPresentCountDlg(int nNpcID)
{
	if (m_pNpcAcceptPresentDlg != NULL)
	{
		CDnSlotButton* pFromSlotBtn = m_pNpcAcceptPresentDlg->GetSelectedPresentSlotButton();
		if (pFromSlotBtn)
			CDnInterface::GetInstance().OpenStoreConfirmExDlg(pFromSlotBtn, CDnInterface::NPC_REPUTATION_GIFT, this);
	}
}

void CDnNpcDlg::SendSelectedNpcPresent( int nNpcID )
{
	_ASSERT( m_pNpcAcceptPresentDlg->IsPresentSelected() );
	int iPresentID = m_pNpcAcceptPresentDlg->GetSelectedPresentID();

	// Display effect on NPC.
	CDnNPCActor* pNpcActor = CDnNPCActor::FindNpcActorFromID( nNpcID );
	_ASSERT( pNpcActor );
	DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
	if( hHandle )
	{
		hHandle->SetActionQueue( "Expression_Goodfeeling_01" );
		hHandle->SetPosition( *pNpcActor->GetPosition() );
		pNpcActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );
	}

	if( GetInterface().GetMainBarDialog() ) 
		GetInterface().GetMainBarDialog()->BlinkMenuButton( CDnMainMenuDlg::NPC_REPUTATION_TAB_DIALOG );

	CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));	
	if (pQuestTask) 
	{
		CReputationSystemRepository* pReputationRepos = pQuestTask->GetReputationRepository();
		if (pReputationRepos)
			pReputationRepos->SetLastUpdateNpcID(nNpcID);

	}

	int presentCount = 0;
	CDnStoreConfirmExDlg* pDlg = CDnInterface::GetInstance().GetStoreConfirmExDlg();
	if (pDlg)
		presentCount = pDlg->GetItemCount();

	if (presentCount <= 0)
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3208), MB_OK ); // UISTRING : Please select a gift.
		return;
	}

	CDnInterface::GetInstance().CloseStoreConfirmExDlg();
	ShowChildDialog( m_pNpcAcceptPresentDlg, false );	

	SendGiveNpcPresent( nNpcID, iPresentID, presentCount );
}
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM


void CDnNpcDlg::OnBlindOpen() 
{
#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	if (m_bSkipOpen == true)
	{
		//Remove head icon display and send packet to server.
		if (CDnActor::s_hLocalActor)
		{
			CDnLocalPlayerActor* pLocalActor = ((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer());

			if (pLocalActor)
			{
				//Auto-run? Follow?? Reset.
				if (pLocalActor->IsFollowing())
				{
					pLocalActor->ResetAutoRun();
					pLocalActor->CmdStop("Stand");
				}
			}

		}
	}
#endif // PRE_ADD_MAILBOX_OPEN

	CDnLocalPlayerActor::LockInput(true);
	//	CDnMouseCursor::GetInstance().ShowCursor( true );
	CDnPlayerActor::HideAnotherPlayers(true, 1.0f);
}

void CDnNpcDlg::OnBlindOpened()
{
#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	if (m_bSkipOpen == true)
		return;
#endif // PRE_ADD_MAILBOX_OPEN

	Open();
	GetInterface().ShowChatDialog();
}

void CDnNpcDlg::OnBlindClose()
{
#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	if (m_bSkipOpen == true)
		return;
#endif // PRE_ADD_MAILBOX_OPEN

	CDnCommonTask *pCommonTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
	if( pCommonTask ) pCommonTask->EndNpcTalk();
}

void CDnNpcDlg::OnBlindClosed()
{
#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	if (m_bSkipOpen)
	{
		//Remove head icon display and send packet to server.
		if (CDnActor::s_hLocalActor)
		{
			((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->CloseMailBoxByShortCutKey();
		}
		
		//Reset when the window closes.
		SetSkipOpen(false);
	}
#endif // PRE_ADD_MAILBOX_OPEN

	bool bIsShowMainQuestDlg = false;

	if (CDnCashShopTask::IsActive() == false || GetCashShopTask().IsOpenCashShop() == false )
	{
		// When opening the Cash Shop directly from the gachapon dialog, inputlock remains active 
		// Default dialog must be closed, so handle it separately here. (#17411)
		if(bIsShowMainQuestDlg == false)
		{
			CDnLocalPlayerActor::LockInput(false);
			GetInterface().OpenBaseDialog();
		}
	}
	else if( CDnWorld::MapTypeVillage != CDnWorld::GetInstance().GetMapType() )
	{
		if(bIsShowMainQuestDlg == false)
		{
			CDnLocalPlayerActor::LockInput(false);
			GetInterface().OpenBaseDialog();
		}
	}

	if(bIsShowMainQuestDlg == false)
		CDnLocalPlayerActor::SetTakeNpcUID(0xffffffff);
//	CDnLocalPlayerActor::LockInput(false);
//	CDnMouseCursor::GetInstance().ShowCursor( false );

	CDnPlayerActor::HideAnotherPlayers(false, 1.0f);

	m_bHideNextBlindOpened = false;
}

void CDnNpcDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) ) 
		{
			CDnCommonTask *pTask = static_cast<CDnCommonTask *>(CTaskManager::GetInstance().GetTask( "CommonTask" ));
			if (pTask)
			{
				CDnStoreConfirmExDlg* pDlg = CDnInterface::GetInstance().GetStoreConfirmExDlg();
				if (pDlg)
				{
					pDlg->EnalbleButtons(false);

					std::wstring szIndex, szTarget;
					GetAnswerIndex(szIndex, szTarget);

					pTask->SendNpcTalkToNextStep(false, szIndex, szTarget);
				}
			}
			return;
		}
		else if (IsCmdControl("ID_CANCEL"))
		{
			CDnInterface::GetInstance().CloseStoreConfirmExDlg();
			CDnInterface::GetInstance().CloseNpcTalkReturnDlg();
			return;
		}
	}
}

void CDnNpcDlg::SetLevelCapRecompense( const TQuestRecompense & questRecompese )
{
	if( !CDnActor::s_hLocalActor )
		return;

	CDnLocalPlayerActor * pLocalActor = dynamic_cast<CDnLocalPlayerActor *>( CDnActor::s_hLocalActor.GetPointer() );
	if( !pLocalActor )
		return;

	if( pLocalActor->GetLevel() < (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::PlayerLevelLimit) )
		return;

	QuestInfo* pQuestInfo = g_DataManager.FindQuestInfo( questRecompese.nQuestIndex );
	if( NULL == pQuestInfo )
		return;

	if( false == pQuestInfo->bLevelCapReward )
		return;

	std::vector<int> vlJobHistory;
	pLocalActor->GetJobHistory( vlJobHistory );
	int iFirstJob = (int)vlJobHistory.front();

	TQuestLevelCapRecompense questLevelCapRecompense;
	if( false == g_DataManager.GetQuestLevelCapRecompense( pQuestInfo->cQuestType, iFirstJob, questLevelCapRecompense ) )
		return;

	m_pNPCQuestRecompenseDlg->SetLevelCapRecompense( questRecompese, questLevelCapRecompense );
}
