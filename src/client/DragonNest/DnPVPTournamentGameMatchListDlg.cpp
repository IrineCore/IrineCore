#include "stdafx.h"
#include "DnPVPTournamentGameMatchListDlg.h"
#include "DnPVPTournamentGameMatchListItemDlg.h"
#include "TaskManager.h"
#include "DnPvPGameTask.h"
#include "DnTableDB.h"
#include "DnInterface.h"
#include "DnPVPTournamentDataMgr.h"
#ifdef PRE_PVP_GAMBLEROOM
#include "DnInterfaceString.h"
#include "DnSimpleTooltipDlg.h"
#endif // PRE_PVP_GAMBLEROOM

#ifdef PRE_ADD_PVP_TOURNAMENT

CDnPVPTournamentGameMatchListDlg::CDnPVPTournamentGameMatchListDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
{
	m_pMatchListContainerListBox = NULL;
	m_pMatchTitle = NULL;
	m_pCloseButton = NULL;
	m_bAutoCursor = true;
	m_pGameMatchListItemDlg = NULL;
	m_pScrollBar = NULL;
	m_bInitUpdate = true;
	m_iScrollPos = -1;

	m_fOriContentDlgXPos = 0.f;
	m_fOriContentDlgYPos = 0.f;

#ifdef PRE_PVP_GAMBLEROOM
	m_pStaticGamblePrice = NULL;
#endif // PRE_PVP_GAMBLEROOM
}

CDnPVPTournamentGameMatchListDlg::~CDnPVPTournamentGameMatchListDlg(void)
{
}

void CDnPVPTournamentGameMatchListDlg::InitialUpdate()
{
	m_pMatchListContainerListBox = GetControl<CEtUIListBox>("ID_LISTBOX_TREE");
	m_pScrollBar = m_pMatchListContainerListBox->GetScrollBar();
	m_pMatchTitle = GetControl<CEtUIStatic>("ID_STATIC9");
	m_pCloseButton = GetControl<CEtUIButton>("ID_BT_CLOSE");

#ifdef PRE_PVP_GAMBLEROOM
	m_pStaticGamblePrice = GetControl<CEtUIStatic>( "ID_STATIC_PRIZE" );
	m_pStaticGamblePrice->Show( false );
#endif // PRE_PVP_GAMBLEROOM

	CDnCustomDlg::InitialUpdate();
}

void CDnPVPTournamentGameMatchListDlg::Initialize(bool bShow)
{
	CDnCustomDlg::Initialize(CEtResourceMng::GetInstance().GetFullName("PvpTournamentMatchListDlg.ui").c_str(), bShow);

	const CDnPVPTournamentDataMgr* pDataMgr = GetDataMgr();
	if (pDataMgr)
	{
		int modeMaxMemberCount = pDataMgr->GetMemberCountOfMode(pDataMgr->GetCurrentMode());
		std::wstring titleString = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120161), modeMaxMemberCount); // UISTRING : Tournament Round of %d
		m_pMatchTitle->SetText(titleString.c_str());
	}
}

void CDnPVPTournamentGameMatchListDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */)
{
	CDnCustomDlg::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);

	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_BT_CLOSE"))
		{
			Show(false);
			if (m_pGameMatchListItemDlg)
				m_pGameMatchListItemDlg->Show(false);
			return;
		}
	}
}

const CDnPVPTournamentDataMgr* CDnPVPTournamentGameMatchListDlg::GetDataMgr() const
{
	if (CDnBridgeTask::IsActive() == false)
		return NULL;

	return &(CDnBridgeTask::GetInstance().GetPVPTournamentDataMgr());
}

void CDnPVPTournamentGameMatchListDlg::SetGameMatchListItemDlg(CDnPVPTournamentGameMatchListItemDlg* pDlg)
{
	m_pGameMatchListItemDlg = pDlg;
}

void CDnPVPTournamentGameMatchListDlg::Update()
{
	const CDnPVPTournamentDataMgr* pDataMgr = GetDataMgr();
	if (pDataMgr == NULL)
	{
		_ASSERT(0);
		return;
	}

	CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstance().GetTask("GameTask"));
	if (pGameTask == NULL || pGameTask->GetGameTaskType() != GameTaskType::PvP)
		return;

	CDnPvPGameTask * pPvPGameTask = static_cast<CDnPvPGameTask*>(pGameTask);
	if (pPvPGameTask == NULL)
		return;

	if (pPvPGameTask->GetGameMode() != PvPCommon::GameMode::PvP_Tournament)
		return;

	if (m_pGameMatchListItemDlg == NULL)
		return;

	m_iScrollPos = 0;
	m_fOriContentDlgXPos = m_fOriContentDlgYPos = 0.f;

	m_pGameMatchListItemDlg->Update();
}

void CDnPVPTournamentGameMatchListDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	if (m_pGameMatchListItemDlg)
		m_pGameMatchListItemDlg->Show(bShow);

#ifdef PRE_PVP_GAMBLEROOM
	if( bShow )
	{
		const CDnPVPTournamentDataMgr* pDataMgr = GetDataMgr();
		if( pDataMgr )
		{
			const SBasicRoomInfo& roomInfo = pDataMgr->GetRoomInfo();
			std::wstring titleString;
			if( roomInfo.cGambleType == PvPGambleRoom::Petal || roomInfo.cGambleType == PvPGambleRoom::Gold )
			{
				titleString = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8413 ), roomInfo.cMaxPlayerNum ); // UISTRING : Tournament Round of %d (Gamble Mode)
				m_pStaticGamblePrice->Show( true );

				int nMoneyType = 0;
				if( roomInfo.cGambleType == PvPGambleRoom::Petal )
					nMoneyType = 4614;
				else if( roomInfo.cGambleType == PvPGambleRoom::Gold )
					nMoneyType = 507;

				int nWinnerPrizeMoney = (int)( roomInfo.cMaxPlayerNum * roomInfo.nGamblePrice * 0.3f );
				std::wstring str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8402 ), DN_INTERFACE::UTIL::GetAddCommaString( nWinnerPrizeMoney ).c_str(), 
											GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMoneyType ) );
				m_pStaticGamblePrice->SetText( str.c_str() );
			}
			else
			{
				titleString = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120161 ), roomInfo.cMaxPlayerNum ); // UISTRING : Tournament Round of %d
				m_pStaticGamblePrice->Show( false );
			}

			m_pMatchTitle->SetText(titleString.c_str());
		}
	}
#endif // PRE_PVP_GAMBLEROOM

	CDnCustomDlg::Show(bShow);
}

void CDnPVPTournamentGameMatchListDlg::_SetContentOffset()
{
	if (m_pGameMatchListItemDlg)
	{
		SUIDialogInfo matchListDlgInfo;
		m_pGameMatchListItemDlg->GetDlgInfo(matchListDlgInfo);

		if (0.f == m_fOriContentDlgXPos && 0.f == m_fOriContentDlgYPos)
		{
			m_fOriContentDlgXPos = matchListDlgInfo.DlgCoord.fX;
			m_fOriContentDlgYPos = matchListDlgInfo.DlgCoord.fY;
		}

		const SUICoord& matchListBoxCoord = m_pMatchListContainerListBox->GetUICoord();
		float fPageSize = matchListBoxCoord.fHeight / GetScreenHeightRatio() * GetEtDevice()->Height();
		m_pScrollBar->SetPageSize((int)fPageSize);

		// Since the dialog is also multiplied by the dialog scale value, it must be considered together to get the exact dialog size.
		float currentListUIHeight = m_pGameMatchListItemDlg->GetCurrentListUIHeight();
		if (currentListUIHeight < 0.f)
		{
			_ASSERT(0);
			return;
		}

		m_pScrollBar->SetTrackRange(0, int(currentListUIHeight * (float)GetEtDevice()->Height() / (float)DEFAULT_UI_SCREEN_HEIGHT * (float)DEFAULT_UI_SCREEN_HEIGHT) );
		if (m_iScrollPos != -1)
		{
			m_pScrollBar->SetTrackPos(m_iScrollPos);
			m_iScrollPos = -1;
		}

		int iPos = m_pScrollBar->GetTrackPos();
		float fNowYOffsetRatio = (float)iPos / fPageSize;
		float fHeight = matchListBoxCoord.fHeight / GetScreenHeightRatio();

		m_pGameMatchListItemDlg->SetPosition(m_fOriContentDlgXPos, m_fOriContentDlgYPos - fNowYOffsetRatio * fHeight);
	}
}

void CDnPVPTournamentGameMatchListDlg::Render(float fElapsedTime)
{
	CDnCustomDlg::Render(fElapsedTime);

	if (IsShow())
	{
		// Move the dialog position according to the scroll bar position.
		_SetContentOffset();

		// The width of the list box created for the scroll bar is the content size.
		// Use this to set up the viewport.
		RECT oldRect, newRect;
		GetEtDevice()->GetScissorRect(&oldRect);

		const SUICoord& ContentListBoxCoord = m_pMatchListContainerListBox->GetUICoord();
		newRect.left = int((ContentListBoxCoord.fX+GetXCoord()) / GetScreenWidthRatio() * GetEtDevice()->Width());
		newRect.top = int((ContentListBoxCoord.fY+GetYCoord()) / GetScreenHeightRatio() * GetEtDevice()->Height());
		newRect.right = newRect.left + int(ContentListBoxCoord.fWidth / GetScreenWidthRatio() * GetEtDevice()->Width());
		newRect.bottom = newRect.top + int(ContentListBoxCoord.fHeight / GetScreenHeightRatio() * GetEtDevice()->Height());

		CEtSprite::GetInstance().Flush();
		GetEtDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		GetEtDevice()->SetScissorRect(&newRect);

		m_pGameMatchListItemDlg->Show(true);
		m_pGameMatchListItemDlg->Render(fElapsedTime);
		m_pGameMatchListItemDlg->Show(false);

		CEtSprite::GetInstance().Flush();
		GetEtDevice()->SetScissorRect(&oldRect);
		GetEtDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}
}

#ifdef PRE_PVP_GAMBLEROOM

bool CDnPVPTournamentGameMatchListDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fMouseX, fMouseY;
			PointToFloat( MousePoint, fMouseX, fMouseY );

			CDnSimpleTooltipDlg* pSimpleTooltTip = GetInterface().GetSimpleTooltipDialog();
			if( pSimpleTooltTip == NULL )
				break;

			if( m_pStaticGamblePrice->IsShow() && m_pStaticGamblePrice->GetUICoord().IsInside( fMouseX, fMouseY ) )
			{
				std::wstring str;
				const CDnPVPTournamentDataMgr* pDataMgr = GetDataMgr();
				if( pDataMgr )
				{
					const SBasicRoomInfo& roomInfo = pDataMgr->GetRoomInfo();

					int nMoneyType = 0;
					if( roomInfo.cGambleType == PvPGambleRoom::Petal )
						nMoneyType = 4614;
					else if( roomInfo.cGambleType == PvPGambleRoom::Gold )
						nMoneyType = 507;

					int nTotalGamblePrice = roomInfo.cMaxPlayerNum * roomInfo.nGamblePrice;
					int nWinnerPrizeMoney = (int)( nTotalGamblePrice * 0.3f );
					int nSecondPrizeMoney = (int)( nTotalGamblePrice * 0.2f );

					str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8403 ), 
						DN_INTERFACE::UTIL::GetAddCommaString( nTotalGamblePrice ).c_str(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMoneyType ),
						DN_INTERFACE::UTIL::GetAddCommaString( nWinnerPrizeMoney ).c_str(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMoneyType ),
						DN_INTERFACE::UTIL::GetAddCommaString( nSecondPrizeMoney ).c_str(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMoneyType ) );
				}

				if( str.length() > 0 )
					pSimpleTooltTip->ShowTooltipDlg( m_pStaticGamblePrice, true, str, 0xffffffff, true );

				break;
			}
		}
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

#endif // PRE_PVP_GAMBLEROOM

#endif // PRE_ADD_PVP_TOURNAMENT