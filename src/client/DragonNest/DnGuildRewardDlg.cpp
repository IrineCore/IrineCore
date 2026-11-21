#include "stdafx.h"

#include "DnGuildRewardDlg.h"
#include "DnGuildRewardItem.h"
#include "DnGuildTask.h"
#include "DnUIString.h"
#include "DnLocalPlayerActor.h"

CDnGuildRewardTooltipDlg::CDnGuildRewardTooltipDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback ) : CEtUIDialog( dialogType, pParentDialog, nID, pCallback ), 
m_pContents( NULL )
{
}

CDnGuildRewardTooltipDlg::~CDnGuildRewardTooltipDlg(void)
{
}

void CDnGuildRewardTooltipDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SimpleTooltipDlg.ui" ).c_str(), bShow );
}

void CDnGuildRewardTooltipDlg::InitialUpdate()
{
	m_pContents = GetControl<CEtUITextBox>( "ID_TEXTBOX" );
}

bool CDnGuildRewardTooltipDlg::AdjustPosition(float orgX, float orgY)
{
// 	float fRight = orgX + m_pContents->GetUICoord().fWidth;
// 	if (fRight > GetScreenWidthRatio())
// 		orgX -= m_pContents->GetUICoord().fWidth;

	SetPosition(orgX, orgY);

	return true;
}

bool CDnGuildRewardTooltipDlg::SetMarkTooltip(const GuildReward::GuildRewardInfo& info)
{
	if (m_pContents == NULL)
		return false;

	m_pContents->ClearText();

	std::wstring msg;
	//Guild reward name?
	MakeUIStringUseVariableParam( msg, info._NameID, (char*)info._NameIDParam.c_str() );
	m_pContents->AddColorText(msg.c_str(), textcolor::YELLOW);
	m_pContents->AddText(L"");

	//Applied duration
	msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 5162);
	m_pContents->AddColorText(msg.c_str(), textcolor::YELLOW);

	if (info._Period == 0)
		msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4705);
	else
		msg = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 5161), info._Period);
	m_pContents->AppendText(msg.c_str(), textcolor::WHITE);

	//Purchase limit
	msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3369);
	m_pContents->AddColorText(msg.c_str(), textcolor::YELLOW);

	if (info._GuildMasterLimit == 0)
		msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3370);
	else
		//msg = FormatW(L"Guild Master");
		msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3733);

	m_pContents->AppendText(msg.c_str(), textcolor::WHITE);
	m_pContents->AddText(L"");

	MakeUIStringUseVariableParam( msg, info._DescriptionID, (char*)info._DescriptionIDParam.c_str() );
	m_pContents->AddColorText(msg.c_str());

	return true;
}
// 
// bool CDnGuildRewardMarkTooltipDlg::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// {
// 	if (m_bShow == false)
// 		return false;
// 
// 	bool bRet = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
// 	switch( uMsg )
// 	{
// 	case WM_MOUSEMOVE:
// 		{
// 			float fMouseX = 0;
// 			float fMouseY = 0;
// 			POINT MousePoint;
// 			MousePoint.x = short( LOWORD( lParam ) );
// 			MousePoint.y = short( HIWORD( lParam ) );
// 			PointToFloat( MousePoint, fMouseX, fMouseY );
// 
// 			if (m_pContents && m_pContents->IsInside(fMouseX, fMouseY))
// 				Show(false);
// 		}
// 	}
// 
// 	return bRet;
// }

//////////////////////////////////////////////////////////////////////////
CDnGuildRewardDlg::CDnGuildRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
:CDnCustomDlg(dialogType, pParentDialog, nID, pCallback)
{
	m_nSelectedIndex = -1;
	m_nTotalPage = 0;
	m_nCurrentPage = 0;

	m_nElementCountPerPage = 3;

	m_pListBoxEx = NULL;
	m_pGuildLevel = NULL;
	m_pStaticPage = NULL;
	m_pButtonBuy = NULL;
	m_pComboFilter = NULL;
	m_pPurchaseOnly = NULL;

	m_nGuildLevel = 0;

	m_pConfirmDlg = NULL;

	m_pToolTipDlg = NULL;
}

CDnGuildRewardDlg::~CDnGuildRewardDlg(void)
{
	if (m_pListBoxEx)
		m_pListBoxEx->RemoveAllItems();

	SAFE_DELETE(m_pConfirmDlg);
	SAFE_DELETE(m_pToolTipDlg);
}

void CDnGuildRewardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildRewardDlg.ui" ).c_str(), bShow );

	m_pConfirmDlg = new CDnGuildRewardBuyConfirmDlg(UI_TYPE_MODAL);
	m_pConfirmDlg->Initialize(false);

	m_pToolTipDlg = new CDnGuildRewardTooltipDlg(UI_TYPE_TOP_MSG);
	m_pToolTipDlg->Initialize(false);
}
void CDnGuildRewardDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_REWARD");
	if (m_pListBoxEx)
	{
		m_pListBoxEx->SetRenderScrollBar( false );
		CEtUIScrollBar* pScrollBar = m_pListBoxEx->GetScrollBar();
		if (pScrollBar)
		{
			pScrollBar->SetPageSize(m_nElementCountPerPage);
		}
	}

	m_pGuildLevel = GetControl<CEtUIStatic>("ID_TEXT_GUILDLEVEL");

	m_pStaticPage = GetControl<CEtUIStatic>("ID_TEXT_PAGE");

	m_pComboFilter = GetControl<CEtUIComboBox>("ID_COMBOBOX_LIST");
	m_pPurchaseOnly = GetControl<CEtUICheckBox>("ID_CHECKBOX_SORT");

	m_pButtonBuy = GetControl<CEtUIButton>("ID_BUTTON_BUY");
	if (m_pButtonBuy)
		m_pButtonBuy->Enable(false);

	AddFilterTypeData();
}

void CDnGuildRewardDlg::AddFilterTypeData()
{
	if (m_pComboFilter == NULL)
		return;

	//m_pComboFilter->Clear
	for (int i = 0; i < GuildReward::GuildRewardType::MAX_COUNT; ++i)
	{
		int nUIStringID = GetSortStringNum(i);
		if (nUIStringID == -1)
			continue;

		m_pComboFilter->AddItem(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUIStringID), NULL, i);
	}
}

int CDnGuildRewardDlg::GetSortStringNum(int type)
{
	int nUIStringID = -1;

	switch(type)
	{
	case GuildReward::GuildRewardType::NONE:					nUIStringID =  427; break;	// Must show everything..
	case GuildReward::GuildRewardType::GUILDITEM:				nUIStringID = 1588; break; // Cosmetic item
	case GuildReward::GuildRewardType::MAKE_GUILDMARK:			nUIStringID = 3974; break; // Create guild mark
	case GuildReward::GuildRewardType::MAX_GUILDWARE:			nUIStringID = 3398; break; // Increase guild storage slots
	case GuildReward::GuildRewardType::MAX_MEMBER:				nUIStringID = 3393; break; // Increase guild member limit
	case GuildReward::GuildRewardType::EXTRA_EXP:				nUIStringID = 3399; break; // Additional EXP
	case GuildReward::GuildRewardType::EXTRA_PVPEXP:			nUIStringID = 3965; break; // Additional Colosseum EXP
	case GuildReward::GuildRewardType::EXTRA_FESTPOINT:			nUIStringID = 3966; break; // Additional guild festival points
	//case GuildReward::GuildRewardType::GUILDWARE_GOLD: nUIStringID = 10000; break; // Deposit gold to guild storage after hunt
	case GuildReward::GuildRewardType::EXTRA_UNIONPOINT:		nUIStringID = 1586; break; // Additional union points
	//case GuildReward::GuildRewardType::MOVE_SPEED: nUIStringID = 10000; break; // Safe zone move speed increase
	//case GuildReward::GuildRewardType::VEHICLE_SPEED: nUIStringID = 10000; break; // Mount move speed increase
	case GuildReward::GuildRewardType::REDUCE_DURABILITYRATIO:	nUIStringID = 1587; break; // Reduce durability consumption rate
	//case GuildReward::GuildRewardType::GUILD_GESTURE: nUIStringID = 1590; break; // Guild gesture
#ifdef PRE_ADD_GUILDREWARDITEM
	case GuildReward::GuildRewardType::GUILD_ADDSKILL1:			nUIStringID = 3874; break; // Increase guild member strength during combat
	case GuildReward::GuildRewardType::GUILD_ADDSKILL2:			nUIStringID = 3875; break; // Guild member Agility increase during combat
	case GuildReward::GuildRewardType::GUILD_ADDSKILL3:			nUIStringID = 3876; break; // Guild member Health increase during combat
	case GuildReward::GuildRewardType::GUILD_ADDSKILL4:			nUIStringID = 3877; break; // Guild member Wisdom increase during combat
#endif
	}

	return nUIStringID;
}

void CDnGuildRewardDlg::Show( bool bShow )
{
	__super::Show(bShow);

	if (bShow)
	{
		TGuild* pGuild = GetGuildTask().GetGuildInfo();
		if (pGuild)
		{
			SetGuildLevel(pGuild->wGuildLevel);
		}

		ShowRewardList();

		UpdateSelectedGuildRewardInfo();
	}

	if (bShow == false)
	{
		if (m_pToolTipDlg)
			m_pToolTipDlg->Show(bShow);

		if (m_pConfirmDlg)
			m_pConfirmDlg->Show(bShow);
	}
}

void CDnGuildRewardDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ||
		nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if(pControl == m_pPurchaseOnly )
		{
			ShowRewardList();
		}
		else
		if( IsCmdControl("ID_BUTTON_BUY" ) ) 
		{
			if (pControl && pControl->IsEnable())
			{
				CDnGuildRewardItem* pItem = GetSelectedRewardItem();
				if (pItem)
				{
					const GuildReward::GuildRewardInfo& rewardInfo = pItem->GetGuildRewardInfo();

					//Check guild reward items that must not be repurchaseable.
					bool isAvailableRepurchase = GetGuildTask().IsAvailableRepurchase(rewardInfo);
					if (false == isAvailableRepurchase)
					{
						//In case of guild storage error.
						if (rewardInfo._Type == GuildReward::MAX_GUILDWARE)
							GetInterface().ServerMessageBox( ERROR_ALREADY_MAXGUILDWARE );
						else
						{
							//Show ERROR_ITEM_BETTEREFFECT_ITEM(488) if not a guild item and timed item cannot be purchased.
							if (rewardInfo._Type != GuildReward::GUILDITEM &&
								rewardInfo._Period != 0)
								GetInterface().ServerMessageBox( ERROR_ITEM_BETTEREFFECT_ITEM );
							else
								GetInterface().ServerMessageBox( ERROR_ITEM_ALREADY_GUILDITEM );
						}
						return;
					}
					else
					if (m_pConfirmDlg)
					{
						m_pConfirmDlg->SetGuildRewardInfo(rewardInfo);
						m_pConfirmDlg->Show(true);
					}
				}
			}
			return;
		}
		else if (IsCmdControl("ID_BUTTON_CLOSE"))
		{
			if( m_pParentDialog ) 
				m_pParentDialog->Show( false );
			Show(false);

			return;
		}
		else if (IsCmdControl("ID_BUTTON_PREV"))
		{
			m_nCurrentPage--;
			if (m_nCurrentPage < 1)
				m_nCurrentPage = 1;

			UpdatePage(m_nCurrentPage);

			return;
		}
		else if (IsCmdControl("ID_BUTTON_NEXT"))
		{
			m_nCurrentPage++;
			if (m_nCurrentPage > m_nTotalPage)
				m_nCurrentPage = m_nTotalPage;

			UpdatePage(m_nCurrentPage);

			return;
		}
	}
	else if( nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( IsCmdControl("ID_LISTBOXEX_REWARD") ) 
		{
			UpdateSelectedGuildRewardInfo();

			return;
		}
	}
	else if (nCommand == EVENT_COMBOBOX_SELECTION_CHANGED && bTriggeredByUser)
	{
		if (IsCmdControl("ID_COMBOBOX_LIST"))
		{
			ShowRewardList();
		}
	}

	__super::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

bool CDnGuildRewardDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( m_pToolTipDlg && m_pToolTipDlg->IsShow() )
	{
		if( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE )
			m_pToolTipDlg->Show(false);
	}

	bool bResult = __super::MsgProc(hWnd, uMsg, wParam, lParam);
	if (bResult)
	{
		switch(uMsg)
		{
		case WM_MOUSEWHEEL:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			{
				POINT MousePoint;
				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );

				if (uMsg == WM_MOUSEWHEEL)
				{
					UpdateCurrentPage();
					RefreshPageInfo();

					//For WHEEL messages, does mouse position info come in Windows desktop absolute coordinates?
					ScreenToClient(hWnd, &MousePoint);

					return true;
				}

				
				float fMouseX, fMouseY;
				//PointToFloat( MousePoint, fMouseX, fMouseY );

				fMouseX = MousePoint.x / GetScreenWidth();
				fMouseY = MousePoint.y / GetScreenHeight();


				bool isInside = false;
				CDnGuildRewardItem *pMouseOverRewardItem = NULL;

				CEtUIScrollBar* pScrollBar = m_pListBoxEx->GetScrollBar();
				if (pScrollBar)
				{
					int nStartPos = pScrollBar->GetTrackPos();
					int nEndPos = nStartPos + pScrollBar->GetPageSize();
					int nTotalSize = m_pListBoxEx->GetSize();
					if (nEndPos > nTotalSize)
						nEndPos = nTotalSize;

					for( int i = nStartPos; i < nEndPos; ++i ) 
					{
						CDnGuildRewardItem *pRewardItem = m_pListBoxEx->GetItem<CDnGuildRewardItem>(i);
						if( !pRewardItem ) continue;
						SUICoord uiCoord;
						pRewardItem->GetDlgCoord( uiCoord );
						if( uiCoord.IsInside( fMouseX, fMouseY ) ) 
						{
							pMouseOverRewardItem = pRewardItem;
							break;
						}
					}
				}

				if (m_pComboFilter->IsOpenedDropDownBox())
					pMouseOverRewardItem = NULL;

				if (pMouseOverRewardItem == NULL)
					m_pToolTipDlg->Show(false);
				else
				{
					m_pToolTipDlg->SetMarkTooltip(pMouseOverRewardItem->GetGuildRewardInfo());
					m_pToolTipDlg->AdjustPosition(fMouseX, fMouseY + ( CDnMouseCursor::GetInstance().GetCursorHeight() * 3/4 ));
					m_pToolTipDlg->Show(true);
				}
			}
			break;
		}
	}

	return bResult;
}

void CDnGuildRewardDlg::InitList()
{
	if (m_pListBoxEx)
		m_pListBoxEx->RemoveAllItems();
}

void CDnGuildRewardDlg::AddGuildRewardInfo(GuildReward::GuildRewardInfo &info, bool bUpdatePage)
{
	if (m_pListBoxEx == NULL)
		return;

	CDnGuildRewardItem* pItem = m_pListBoxEx->AddItem<CDnGuildRewardItem>();
	if (pItem == NULL)
		return;

	pItem->SetGuildRewardInfo(info);

	if (bUpdatePage)
		UpdatePageInfo();
}

void CDnGuildRewardDlg::SetGuildLevel(int nLevel)
{
	m_nGuildLevel = nLevel;

	wchar_t buffer[256] = {0, };
	swprintf_s( buffer, _countof(buffer), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3351 ), nLevel );

	m_pGuildLevel->SetText( buffer );
}

void CDnGuildRewardDlg::UpdateTotalPage()
{
	m_nTotalPage = 0;

	if (m_pListBoxEx)
	{
		int nElementCount = m_pListBoxEx->GetSize();
		
		//Calculate page count..
		m_nTotalPage = nElementCount / m_nElementCountPerPage;
		if (nElementCount % m_nElementCountPerPage != 0)
			m_nTotalPage++;
	}

	m_pListBoxEx->SetRenderScrollBar( m_nTotalPage > 1 );
}

void CDnGuildRewardDlg::UpdateCurrentPage()
{
	m_nCurrentPage = 0;

	if (m_pListBoxEx)
	{
		int nElementCount = m_pListBoxEx->GetSize();

		int nCurPos = 0;
		CEtUIScrollBar* pScrollBar = m_pListBoxEx->GetScrollBar();
		if (pScrollBar)
		{
			nCurPos = pScrollBar->GetTrackPos();

#ifdef PRE_FIX_SCROLL_PAGE_NUMBER
			int pageRevisionNumber = 1;
			if ((nCurPos % m_nElementCountPerPage) != 0)
				pageRevisionNumber = 2;

			if (nElementCount > 0)
				m_nCurrentPage = (nCurPos / m_nElementCountPerPage) + pageRevisionNumber;
#else
			if (nElementCount > 0)
				m_nCurrentPage = (nCurPos / m_nElementCountPerPage) + 1;
#endif
		}

		//m_pListBoxEx->UpdateRects();
	}
}

void CDnGuildRewardDlg::UpdatePageInfo()
{
	UpdateTotalPage();
	UpdateCurrentPage();

	RefreshPageInfo();

}

void CDnGuildRewardDlg::UpdatePage(int nCurPage)
{
	if (m_pListBoxEx)
	{
		int nCurPos = (nCurPage - 1) * m_nElementCountPerPage;
		CEtUIScrollBar* pScrollBar = m_pListBoxEx->GetScrollBar();
		if (pScrollBar)
		{
			pScrollBar->SetTrackPos(nCurPos);
		}
	}

	UpdateCurrentPage();

	RefreshPageInfo();
}

void CDnGuildRewardDlg::RefreshPageInfo()
{
	if (m_pStaticPage == NULL)
		return;

	wchar_t buffer[256] = {0, };
	swprintf_s( buffer, _countof(buffer), L"%02d/%02d", m_nCurrentPage, m_nTotalPage);
	m_pStaticPage->SetText(buffer);
}

void CDnGuildRewardDlg::UpdateSelectedGuildRewardInfo()
{
	CDnGuildRewardItem *pItem = GetSelectedRewardItem();
	
	if (pItem)
	{
		const GuildReward::GuildRewardInfo& info = pItem->GetGuildRewardInfo();
		bool bEnableReward = GetGuildTask().IsAvailableReward(info);
		
		m_pButtonBuy->Enable(bEnableReward);
	}
	else
		m_pButtonBuy->Enable(false);
}

CDnGuildRewardItem* CDnGuildRewardDlg::GetSelectedRewardItem()
{
	CDnGuildRewardItem *pItem = NULL;
	
	int select = m_pListBoxEx ? m_pListBoxEx->GetSelectedIndex() : -1;
	if( -1 != select )
		pItem = m_pListBoxEx->GetItem<CDnGuildRewardItem>(select);
	
	return pItem;
}

static bool CompareGuildReward( const GuildReward::GuildRewardInfo &info1, const GuildReward::GuildRewardInfo &info2 )
{
	return info1._NeedGuildLevel < info2._NeedGuildLevel;
}

void CDnGuildRewardDlg::ShowRewardList()
{
	CDnPlayerActor* pPlayer = NULL;
	if( CDnActor::s_hLocalActor )
		pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	
	CGuildRewardTable::GUILDREWARD_LIST guildRewardList = CGuildRewardTable::GetInstance().GetGuildRewardList();
	CGuildRewardTable::GUILDREWARD_LIST::iterator iter = guildRewardList.begin();
	CGuildRewardTable::GUILDREWARD_LIST::iterator endIter = guildRewardList.end();

	std::vector<GuildReward::GuildRewardInfo> filterList;

	//Filter Type
	int nFilterType = GetFilterType();
	bool bPurchaseOnly = GetPurchaseOnly();

	bool isPurchased = false;
	for (; iter != endIter; ++iter)
	{
		GuildReward::GuildRewardInfo& info = iter->second;

		if (info._IsApplicable == false)	//Check whether active..
			continue;

		if (( 0 != info._NeedJobClass ) && (pPlayer && pPlayer->IsPassJob(info._NeedJobClass) == false)) //Job restriction...
			continue;
			
		if (GetGuildTask().IsPrerequisite(info) == false)	//Only add items of a better grade(?) than the purchased item
			continue;

		//If filter type is set and type differs, skip.
		if (nFilterType != 0 && info._Type != nFilterType)
			continue;
		
		if (bPurchaseOnly == true && GetGuildTask().IsPurchased(info) == false)
			continue;
		
		filterList.push_back(info);
	}

	//Compare the current list count with the newly filtered list count
	//If different, update Page info.
	int nNewListCount = (int)filterList.size();
	int nOldListCount = m_pListBoxEx->GetSize();
	int nOldPos = 0;
	
	//If the previous list count equals the new list count, preserve the existing page? info...
	if (nNewListCount == nOldListCount)
		nOldPos = m_pListBoxEx->GetScrollBar()->GetTrackPos();

	//Reset existing listbox...
	InitList();

	std::sort(filterList.begin(), filterList.end(), CompareGuildReward);
	std::vector<GuildReward::GuildRewardInfo>::iterator listIter = filterList.begin();
	std::vector<GuildReward::GuildRewardInfo>::iterator listEndIter = filterList.end();
	for (; listIter != listEndIter; ++listIter)
	{
		//No need to update page info on full list refresh..
		AddGuildRewardInfo(*listIter, false);
	}

	//After adding all to the list, restore saved page info if any.
	m_pListBoxEx->GetScrollBar()->SetTrackPos(nOldPos);

	UpdatePageInfo();
}

void CDnGuildRewardDlg::CloseBuyGuildRewardConfirmDlg()
{
	if (m_pConfirmDlg)
		m_pConfirmDlg->Show(false);
}

int CDnGuildRewardDlg::GetFilterType()
{
	int nFilterType = 0;

	if (m_pComboFilter == NULL)
		return nFilterType;
	
	SComboBoxItem* pItem = m_pComboFilter->GetSelectedItem();
	if (pItem)
	{
		m_pComboFilter->GetSelectedValue(nFilterType);
	}

	return nFilterType;
}

bool CDnGuildRewardDlg::GetPurchaseOnly()
{
	return m_pPurchaseOnly ? m_pPurchaseOnly->IsChecked() : false;
}