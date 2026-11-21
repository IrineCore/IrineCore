#include "StdAfx.h"
#include "DnSkillStoreDlg.h"
#include "DnSkillTask.h"
#include "DnSkillBuyDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSkillStoreDlg::CDnSkillStoreDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallBack )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallBack )
// 	, m_nMaxPage(1)
// 	, m_nCurrentPage(1)
// 	, m_nSkillSlotIndex(-1)
// 	, m_pSkillPage(NULL)
// 	, m_pSelectBar(NULL)
// 	, m_pButtonPagePrev(NULL)
// 	, m_pButtonPageNext(NULL)
// 	, m_pButtonBuy(NULL)
// 	, m_pSkillBuyDlg(NULL)
{
}

CDnSkillStoreDlg::~CDnSkillStoreDlg(void)
{
	//SAFE_DELETE( m_pSkillBuyDlg );
}

void CDnSkillStoreDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillStoreDlg.ui" ).c_str(), bShow );
}

/* 
void CDnSkillStoreDlg::InitialUpdate()
{
	m_pSkillPage = GetControl<CEtUIStatic>("ID_PAGE_NUM");

	char szControlName[32]={0};

	for( int i=0; i<(int)m_vecSkillSlot.size(); i++ )
	{
		sprintf_s( szControlName, 32, "ID_BASE%d", i );
		m_vecSkillSlot[i].m_pSkillBase = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_NAME_%02d", i );
		m_vecSkillSlot[i].m_pSkillName = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_LEVEL_%02d", i );
		m_vecSkillSlot[i].m_pSkillLevel = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_MONEY_%02d", i );
		m_vecSkillSlot[i].m_pSkillMoney = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_SP_%02d", i );
		m_vecSkillSlot[i].m_pSkillSP = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_ITEM_CNT_%02d", i );
		m_vecSkillSlot[i].m_pNeedItemCount = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_NEEDITEM_%02d", i );
		m_vecSkillSlot[i].m_pItemButton = GetControl<CDnItemSlotButton>(szControlName);
	}

	m_pButtonPagePrev = GetControl<CEtUIButton>("ID_PAGE_PREV");
	m_pButtonPageNext = GetControl<CEtUIButton>("ID_PAGE_NEXT");
	m_pButtonPagePrev->Enable(false);
	m_pButtonPageNext->Enable(false);

	m_pButtonBuy = GetControl<CEtUIButton>("ID_BUTTON_BUY");

	m_pSelectBar = GetControl<CEtUIStatic>("ID_SELECT_BAR");
	m_pSelectBar->Show(false);

	m_pSkillBuyDlg = new CDnSkillBuyDlg( UI_TYPE_MODAL, NULL, SKILL_BUY_DIALOG, this );
	m_pSkillBuyDlg->Initialize( false );
}

void CDnSkillStoreDlg::InitCustomControl( CEtUIControl *pControl )
{
	if( !strstr( pControl->GetControlName(), "ID_SKILL_" ) )
		return;

	CDnSkillSlotButton *pSkillSlotButton(NULL);
	pSkillSlotButton = static_cast<CDnSkillSlotButton*>(pControl);

	pSkillSlotButton->SetSlotType(ST_SKILL_STORE);
	pSkillSlotButton->SetSlotIndex( (int)m_vecSkillSlot.size() );

	SSkillStoreSlot skillSlot;
	skillSlot.m_pSkillButton = pSkillSlotButton;
	m_vecSkillSlot.push_back( skillSlot );
}

void CDnSkillStoreDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_PAGE_PREV" ) )
		{
			PrevPage();
			return;
		}

		if( IsCmdControl("ID_PAGE_NEXT" ) )
		{
			NextPage();
			return;
		}

		if( IsCmdControl("ID_BUTTON_BUY") )
		{
			// Note : Buy item
			//		Check amount
			//		Check required item
			//		Check level
			//
			if( m_nSkillSlotIndex >= 0 )
			{
				MIInventoryItem *pSkill = m_vecSkillSlot[m_nSkillSlotIndex].m_pSkillButton->GetItem();
				if( pSkill )
				{
					m_pSkillBuyDlg->SetSkillInfo( pSkill, m_vecSkillSlot[m_nSkillSlotIndex].m_pItemButton->GetItem(), m_vecSkillSlot[m_nSkillSlotIndex].m_pSkillMoney->GetText() );
					m_pSkillBuyDlg->Show( true );
				}
			}
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnSkillStoreDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fMouseX, fMouseY;
			PointToFloat( MousePoint, fMouseX, fMouseY );

			int nSlotIndex = FindInsideItem( fMouseX, fMouseY );
			if( nSlotIndex != -1 )
			{
				UpdateSelectBar( nSlotIndex );
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			if( m_pButtonBuy->IsEnable() && ( m_nSkillSlotIndex >= 0 ) )
			{
				MIInventoryItem *pSkill = m_vecSkillSlot[m_nSkillSlotIndex].m_pSkillButton->GetItem();
				if( pSkill )
				{
					m_pSkillBuyDlg->SetSkillInfo( pSkill, m_vecSkillSlot[m_nSkillSlotIndex].m_pItemButton->GetItem(), m_vecSkillSlot[m_nSkillSlotIndex].m_pSkillMoney->GetText() );
					m_pSkillBuyDlg->Show( true );
				}
			}
		}
		break;
	case WM_MOUSEWHEEL:
		{
			if( IsMouseInDlg() )
			{
				UINT uLines;
				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
				int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
				if( nScrollAmount > 0 )
				{
					PrevPage();
				}
				else if( nScrollAmount < 0 )
				{
					NextPage();
				}
			}
		}
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnSkillStoreDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_nCurrentPage = 1;
		UpdateMaxPageNum();
		UpdateSkillPage();
		UpdateSkillISlot();
	}
	else
	{
		InitSkillInfo();
	}

	CDnCustomDlg::Show( bShow );
}

int CDnSkillStoreDlg::FindInsideItem( float fX, float fY )
{
	for( int i = 0; i < ITEM_SIZE; i++ )
	{
		if( m_vecSkillSlot[i].m_pSkillButton->IsEmptySlot() )
			continue;

		if( m_vecSkillSlot[i].IsInside( fX, fY ) )
		{
			return m_vecSkillSlot[i].m_pSkillButton->GetSlotIndex();
		}
	}

	return -1;
}

void CDnSkillStoreDlg::SetItem( int nSlotIndex, MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnSkillStoreDlg::SetItem");
	if( !pItem ) return;

	SSkillItemInfo skillItemInfo;
	skillItemInfo.nSlotIndex = nSlotIndex;
	skillItemInfo.pSkill = static_cast<CDnSkill*>(pItem);

	m_vecSkillItem.push_back(skillItemInfo);
}

void CDnSkillStoreDlg::UpdateSkillISlot()
{
	InitSkillSlotList();

	if( m_nCurrentPage == 0 )		return;
	if( m_vecSkillItem.empty() )	return;

	int nStartIndex = (m_nCurrentPage-1)*ITEM_SIZE;
	int nEndIndex = m_nCurrentPage*ITEM_SIZE;

	for( int i=nStartIndex, j=0; i<nEndIndex; i++, j++ )
	{
		if( i >= (int)m_vecSkillItem.size() )
			break;

		m_vecSkillSlot[j].SetSlot( m_vecSkillItem[i].nSlotIndex, m_vecSkillItem[i].pSkill );
	}
}

void CDnSkillStoreDlg::UpdateMaxPageNum()
{
	int nSkillListSize = (int)m_vecSkillItem.size();
	if( nSkillListSize == 0 )
	{
		m_nMaxPage = 1;
	}
	else
	{
		m_nMaxPage = (nSkillListSize / ITEM_SIZE);
		if( (nSkillListSize%ITEM_SIZE) )
		{
			m_nMaxPage++;
		}
	}

	UpdatePageButton();
}

void CDnSkillStoreDlg::UpdateSkillPage()
{
	wchar_t wszTemp[80]={0};
	swprintf_s( wszTemp, 80, L"%d/%d", m_nCurrentPage, m_nMaxPage );
	m_pSkillPage->SetText(wszTemp);
}

void CDnSkillStoreDlg::NextPage()
{
	if( m_vecSkillItem.empty() )
		return;

	m_nCurrentPage++;
	if( m_nCurrentPage > m_nMaxPage )
	{
		m_nCurrentPage = m_nMaxPage;
	}

	UpdateSkillISlot();
	UpdateSkillPage();
	UpdatePageButton();
}

void CDnSkillStoreDlg::PrevPage()
{
	if( m_vecSkillItem.empty() )
		return;

	m_nCurrentPage--;
	if( m_nCurrentPage < 1 )
	{
		m_nCurrentPage = 1;
	}

	UpdateSkillISlot();
	UpdateSkillPage();
	UpdatePageButton();
}

void CDnSkillStoreDlg::InitSkillSlotList()
{
	for( int i=0; i<(int)m_vecSkillSlot.size(); i++ )
	{
		m_vecSkillSlot[i].InitSlot();
	}
}

void CDnSkillStoreDlg::ResetAllItem()
{
	InitSkillSlotList();
	m_vecSkillItem.clear();
}

void CDnSkillStoreDlg::UpdatePageButton()
{
	if( m_nCurrentPage == m_nMaxPage )
	{
		m_pButtonPageNext->Enable(false);
	}
	else
	{
		m_pButtonPageNext->Enable(true);
	}

	if( m_nCurrentPage == 1 )
	{
		m_pButtonPagePrev->Enable(false);
	}
	else
	{
		m_pButtonPagePrev->Enable(true);
	}
}

void CDnSkillStoreDlg::UpdateSelectBar( int nSlotIndex )
{
	ASSERT( nSlotIndex>=0 );
	ASSERT( nSlotIndex<ITEM_SIZE );

	SUICoord &buttonCoord = m_vecSkillSlot[nSlotIndex].m_pSkillButton->GetUICoord();
	SUICoord &barCoord = m_pSelectBar->GetUICoord();

	barCoord.fX = buttonCoord.fX-0.009f;
	barCoord.fY = buttonCoord.fY-0.01f;

	m_pSelectBar->SetUICoord( barCoord );
	m_pSelectBar->Show(true);

	m_pButtonBuy->Enable( m_vecSkillSlot[nSlotIndex].m_bEnable );
	m_nSkillSlotIndex = nSlotIndex;
}

void CDnSkillStoreDlg::InitSkillInfo()
{
	m_pButtonPagePrev->Enable(false);
	m_pButtonPageNext->Enable(false);
	m_pButtonBuy->Enable(false);
	m_pSelectBar->Show(false);
	m_nSkillSlotIndex = -1;
}

void CDnSkillStoreDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) )
		{
			if( m_nSkillSlotIndex >= 0 )
			{
				int nSlotIndex = m_vecSkillSlot[m_nSkillSlotIndex].m_nSlotIndex;
				if( nSlotIndex >= 0 )
				{
					InitSkillInfo();
					///GetTradeTask().GetTradeSkill().RequestSkillShopBuy( nSlotIndex );
				}
			}

			m_pSkillBuyDlg->Show( false );
			return;
		}

		if( IsCmdControl( "ID_CANCEL" ) ) 
		{
			m_pSkillBuyDlg->Show( false );
			return;
		}
	}
}

void CDnSkillStoreDlg::RefreshDialog()
{
	InitSkillInfo();

	m_nCurrentPage = 1;
	UpdateMaxPageNum();
	UpdateSkillPage();
	UpdateSkillISlot();
}

bool CDnSkillStoreDlg::IsEmptySlotDialog()
{
	if( m_vecSkillItem.empty() )
		return true;

	return false;
}
*/