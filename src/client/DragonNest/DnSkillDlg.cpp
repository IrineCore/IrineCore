#include "StdAfx.h"
#include "DnSkillDlg.h"
#include "DnSkillSlotButton.h"
#include "DnMainDlg.h"
#include "DnSkillTask.h"
#include "DNTableFile.h"
#include "DnTableDB.h"
#include "DnActor.h"
#include "DnInterface.h"
#include "DnSkillLevelUpDlg.h"
#include "EtUIScrollBar.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnSkillDlg::CDnSkillDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallBack )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallBack )
	, m_pSkillPage(NULL)
	, m_pSkillPoint(NULL)
	, m_nMaxPage(1)
	, m_nCurrentPage(1)
	, m_pSelectBar(NULL)
	, m_nSelectedIndex(-1)
	, m_nDragSoundIndex(-1)
	, m_pButtonPageNext(NULL)
	, m_pButtonPagePrev(NULL)
	, m_pScrollBar(NULL)
{
}

CDnSkillDlg::~CDnSkillDlg(void)
{
	//SAFE_DELETE( m_pSkillLevelUpDlg );

	if( m_nDragSoundIndex == -1 ) 
	{
		CEtSoundEngine::GetInstance().RemoveSound( m_nDragSoundIndex );
	}
}


void CDnSkillDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillDlg.ui" ).c_str(), bShow );

	const char *szFileName = CDnTableDB::GetInstance().GetFileName( 10005 );
	if( strlen( szFileName ) > 0 )
	{
		m_nDragSoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	}
}

void CDnSkillDlg::InitialUpdate()
{
	m_pSkillPage = GetControl<CEtUIStatic>("ID_PAGE_NUM");
	m_pSkillPoint = GetControl<CEtUIStatic>("ID_SKILL_POINT");

	char szControlName[32]={0};

	for( int i=0; i<(int)m_vecSkillSlot.size(); i++ )
	{
		sprintf_s( szControlName, 32, "ID_STATIC%d", i+1 );
		m_vecSkillSlot[i].m_pSkillBase = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_NAME_%02d", i );
		m_vecSkillSlot[i].m_pSkillName = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_LEVEL_%02d", i );
		m_vecSkillSlot[i].m_pSkillLevel = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_POINT_%02d", i );
		m_vecSkillSlot[i].m_pSkillPoint = GetControl<CEtUIStatic>(szControlName);

		sprintf_s( szControlName, 32, "ID_SKILLUP_%02d", i );
		m_vecSkillSlot[i].m_pSkillUpButton = GetControl<CDnSkillUpButton>(szControlName);
		m_vecSkillSlot[i].m_pSkillUpButton->Enable(false);
	}

	m_pButtonPagePrev = GetControl<CEtUIButton>("ID_PAGE_PREV");
	m_pButtonPageNext = GetControl<CEtUIButton>("ID_PAGE_NEXT");
	m_pButtonPagePrev->Enable(false);
	m_pButtonPageNext->Enable(false);

	m_pSelectBar = GetControl<CEtUIStatic>("ID_SELECT_BAR");
	m_pSelectBar->Show(false);

	//m_pSkillLevelUpDlg = new CDnSkillLevelUpDlg( UI_TYPE_MODAL, NULL, SKILL_LEVELUP_DIALOG, this );
	//m_pSkillLevelUpDlg->Initialize( false );

	//// Get only the scrollbar of the created list box 
	//CEtUIListBox* pChildListBox = GetControl<CEtUIListBox>( "ID_LISTBOX0" );
	//m_pScrollBar = pChildListBox->GetScrollBar();

	//for( int i = 0; i < 40; ++i )
	//	pChildListBox->AddItem( _T("TEST"), NULL, 10 );
}

void CDnSkillDlg::InitCustomControl( CEtUIControl *pControl )
{
	if( typeid(*pControl) != typeid(CDnQuickSlotButton) )
		return;

	CDnQuickSlotButton *pSkillSlotButton(NULL);
	pSkillSlotButton = static_cast<CDnQuickSlotButton*>(pControl);

	pSkillSlotButton->SetSlotType(ST_SKILL);
	pSkillSlotButton->SetSlotIndex( (int)m_vecSkillSlot.size() );

	SKILL_SLOT skillSlot;
	skillSlot.m_pSlotButton = pSkillSlotButton;
	m_vecSkillSlot.push_back( skillSlot );
}

void CDnSkillDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_nCurrentPage = 1;
		UpdatePage();
	}
	else
	{
		m_pSelectBar->Show(false);
		m_nSelectedIndex = -1;
	}

	CEtUIDialog::Show( bShow );
}

void CDnSkillDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
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

		//if( strstr( pControl->GetControlName(), "ID_SKILLUP" ) )
		//{
		//	CDnSkillUpButton *pSkillUpButton = (CDnSkillUpButton*)pControl;
		//	m_nSelectedIndex = pSkillUpButton->GetButtonID();

		//	MIInventoryItem *pSkill = m_vecSkillSlot[m_nSelectedIndex].m_pSlotButton->GetItem();
		//	if( pSkill )
		//	{
		//		m_pSkillLevelUpDlg->SetSkill( pSkill );
		//		m_pSkillLevelUpDlg->Show( true );
		//	}
		//	return;
		//}

		if( strstr( pControl->GetControlName(), "ID_SKILL" ) )
		{
			CDnSlotButton *pDragButton;
			CDnQuickSlotButton *pPressedButton;

			pDragButton = ( CDnSlotButton * )drag::GetControl();
			pPressedButton = ( CDnQuickSlotButton * )pControl;

			if( pDragButton == NULL )
			{
				CDnSkill *pSkill = static_cast<CDnSkill*>(pPressedButton->GetItem());
				if( !pSkill ) return;
				if( pSkill->GetSkillType() == CDnSkill::Passive ) return;

				if( uMsg == WM_RBUTTONUP )
				{
					CDnActor::s_hLocalActor->UseSkill( pSkill->GetClassID() );
				}
				else
				{
					pPressedButton->EnableSplitMode(0);
					drag::SetControl(pPressedButton);

					if( m_nDragSoundIndex != -1 )
						CEtSoundEngine::GetInstance().PlaySound( "2D", m_nDragSoundIndex );
				}
			}
			else
			{
				pDragButton->DisableSplitMode(false);
				drag::ReleaseControl();
			}

			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnSkillDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() )
	{
		UpdateSkillPoint();
		UpdateSkillUpButton();
	}
}


void CDnSkillDlg::Render( float fElapsedTime )
{
	CDnCustomDlg::Render( fElapsedTime );

	// Note : Variable for scrolling while holding down the mouse click
	//
	//int nOldPosition=0, nCurPosition;
}


void CDnSkillDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	//SetCmdControlName( pControl->GetControlName() );

	//if( nCommand == EVENT_BUTTON_CLICKED )
	//{
	//	if( IsCmdControl( "ID_OK" ) )
	//	{
	//		if( m_nSelectedIndex >= 0 )
	//		{
	//			CDnSkill *pSkill = static_cast<CDnSkill*>(m_vecSkillSlot[m_nSelectedIndex].m_pSlotButton->GetItem());
	//			if( pSkill )
	//			{
	//				// Note: Currently UI only allows leveling up a skill one level at a time.
	//				// Related to receiving a failure packet in CDnSkillTask::OnRecvSkillLevelUp(), so fix that side too
	//				pSkill->CalcLevelUpUsePoint( 1 );
	//				GetSkillTask().SendSkillLevelUpReq( pSkill->GetClassID(), pSkill->GetUsePoint() );
	//			}
	//		}

	//		m_pSkillLevelUpDlg->Show( false );
	//		return;
	//	}

	//	if( IsCmdControl( "ID_CANCEL" ) ) 
	//	{
	//		m_nSelectedIndex = -1;
	//		m_pSkillLevelUpDlg->Show( false );
	//		return;
	//	}
	//}
}

bool CDnSkillDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fMouseX, fMouseY;
			PointToFloat( MousePoint, fMouseX, fMouseY );

			m_nSelectedIndex = FindInsideItem( fMouseX, fMouseY );
			if( m_nSelectedIndex != -1 )
			{
				UpdateSelectBar( m_nSelectedIndex );
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
					NextPage();
				}
				else if( nScrollAmount < 0 )
				{
					PrevPage();
				}
			}
		}
		break;
	case WM_KEYDOWN:
		{
			if( (wParam >= 0x30) && (wParam <= 0x39) )
			{
				if( GetKeyState( VK_SHIFT ) < 0 )
				{
					int nIndex(0);

					if( wParam == 0x30 )
					{
						nIndex = 9;
					}
					else
					{
						nIndex = int(wParam) - 49;
					}

					CDnMainDlg *pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
					if( pMainDlg && (m_nSelectedIndex>=0) )
					{
						CDnSkill *pSkill = static_cast<CDnSkill*>(m_vecSkillSlot[m_nSelectedIndex].m_pSlotButton->GetItem());
						if( !pSkill ) return true;
						if( pSkill->GetSkillType() == CDnSkill::Passive ) return true;

						pMainDlg->SetQuickSlot( nIndex, m_vecSkillSlot[m_nSelectedIndex].m_pSlotButton->GetItem() );
						return true;
					}
				}
			}
		}
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

int CDnSkillDlg::FindInsideItem( float fX, float fY )
{
	for( int i = 0; i < ITEMSLOT_MAX_SIZE; i++ )
	{
		if( m_vecSkillSlot[i].m_pSlotButton->IsEmptySlot() )
			continue;

		if( m_vecSkillSlot[i].IsInside( fX, fY ) )
		{
			return m_vecSkillSlot[i].m_pSlotButton->GetSlotIndex();
		}
	}

	return -1;
}

void CDnSkillDlg::UpdateSelectBar( int nSlotIndex )
{
	ASSERT( nSlotIndex>=0 );
	ASSERT( nSlotIndex<ITEMSLOT_MAX_SIZE );

	SUICoord &buttonCoord = m_vecSkillSlot[nSlotIndex].m_pSkillName->GetUICoord();
	SUICoord &barCoord = m_pSelectBar->GetUICoord();

	barCoord.fX = buttonCoord.fX-0.014f;
	barCoord.fY = buttonCoord.fY-0.012f;

	m_pSelectBar->SetUICoord( barCoord );
	m_pSelectBar->Show(true);
}

void CDnSkillDlg::SetItem( MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnSkillDlg::SetItem");
	if( !pItem ) return;

	CDnSkill *pSkillItem = static_cast<CDnSkill*>(pItem);
	m_vecSkillItem.push_back(pSkillItem->GetMySmartPtr());
}

void CDnSkillDlg::UpdateSkillISlot()
{
	InitSkillSlotList();

	if( m_vecSkillItem.empty() )
		return;
	
	// Sort for display
	struct SkillSort : public binary_function<const DnSkillHandle&, const DnSkillHandle&, bool>
	{
		bool operator () ( const DnSkillHandle& hSkill1, const DnSkillHandle& hSkill2 )
		{
			return hSkill1->operator < ( hSkill2 );
		}
	};
	sort( m_vecSkillItem.begin(), m_vecSkillItem.end(), SkillSort() );

	int nStartIndex = (m_nCurrentPage-1)*ITEMSLOT_MAX_SIZE;
	int nEndIndex = m_nCurrentPage*ITEMSLOT_MAX_SIZE;

	SKILL_SLOT *pSkillSlot(NULL);
	CDnSkill *pSkillItem(NULL);
	wchar_t wszTemp[80]={0};

	for( int i=nStartIndex, j=0; i<nEndIndex; i++, j++ )
	{
		if( i >= (int)m_vecSkillItem.size() )
			break;

		m_vecSkillSlot[j].SetSkillSlot( m_vecSkillItem[i], m_vecSkillItem[i]->GetName(), m_vecSkillItem[i]->GetLevel(), m_vecSkillItem[i]->GetNextLevelSkillPoint() );
	}
}

void CDnSkillDlg::UpdateMaxPageNum()
{
	int nSkillListSize = (int)m_vecSkillItem.size();
	if( nSkillListSize == 0 )
	{
		m_nMaxPage = 1;
	}
	else
	{
		m_nMaxPage = (nSkillListSize / ITEMSLOT_MAX_SIZE);
		if( (nSkillListSize%ITEMSLOT_MAX_SIZE) )
		{
			m_nMaxPage++;
		}
	}

	UpdateSkillPage();
}

void CDnSkillDlg::UpdateSkillPage()
{
	wchar_t wszTemp[80]={0};
	swprintf_s( wszTemp, 80, L"%d/%d", m_nCurrentPage, m_nMaxPage );
	m_pSkillPage->SetText(wszTemp);
}

void CDnSkillDlg::NextPage()
{
	if( m_vecSkillItem.empty() )
		return;

	m_nCurrentPage++;
	if( m_nCurrentPage > m_nMaxPage )
	{
		m_nCurrentPage = m_nMaxPage;
	}

	UpdatePage();
}

void CDnSkillDlg::PrevPage()
{
	if( m_vecSkillItem.empty() )
		return;

	m_nCurrentPage--;
	if( m_nCurrentPage < 1 )
	{
		m_nCurrentPage = 1;
	}

	UpdatePage();
}

void CDnSkillDlg::InitSkillSlotList()
{
	for( int i=0; i<(int)m_vecSkillSlot.size(); i++ )
	{
		m_vecSkillSlot[i].InitSkillSlot();
	}

	m_pSelectBar->Show(false);
	//m_nSelectedIndex = -1;
}

void CDnSkillDlg::ResetAllItem()
{
	InitSkillSlotList();
	m_vecSkillItem.clear();
}

void CDnSkillDlg::UpdatePageButton()
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

void CDnSkillDlg::UpdateSkillPoint()
{
	if( CDnSkillTask::IsActive() ) {
		m_pSkillPoint->SetIntToText( GetSkillTask().GetSkillPoint() );
	}
}

void CDnSkillDlg::UpdateSkillUpButton()
{
	SKILL_SLOT *pSkillSlot(NULL);
	CDnSkill *pSkill(NULL);

	if( !CDnActor::s_hLocalActor )
		return;

	for( int i=0; i<(int)m_vecSkillSlot.size(); i++ )
	{
		pSkillSlot = &m_vecSkillSlot[i];
		pSkillSlot->m_pSkillUpButton->Enable( false );
		pSkillSlot->m_pSkillUpButton->SetBlink( false );
		pSkillSlot->m_pSkillPoint->SetTextColor( EtInterface::textcolor::WHITE );
	}

	int nSkillPoint = GetSkillTask().GetSkillPoint();
	if( nSkillPoint <= 0 ) return;

	DNTableFileFormat*  pSkillLevelTable = GetTableDB().GetTable( CDnTableDB::TSKILLLEVEL );
	if( !pSkillLevelTable )
		return;

#ifndef PRE_FIX_NEXTSKILLINFO
	const CDnSkill::SkillInfo *pSkillInfo(NULL);
#endif

	int nLevelLimit(0);

	for( int i=0; i<(int)m_vecSkillSlot.size(); i++ )
	{
		pSkillSlot = &m_vecSkillSlot[i];
		if( !pSkillSlot->m_pSlotButton->IsEmptySlot() )
		{
			pSkill = static_cast<CDnSkill*>(pSkillSlot->m_pSlotButton->GetItem());

			// Cannot level up equipment item skills.
			if( pSkill->GetLevel() < pSkill->GetMaxLevel() && !pSkill->IsEquipItemSkill() )
			{
				// Cannot level up skill while toggle is on or aura is active.
				if( pSkill->IsToggleOn() || pSkill->IsAuraOn() )
					continue;

#if defined(PRE_FIX_NEXTSKILLINFO)
				int nSkillLevelTableID = -1;
				SKILL_LEVEL_INFO* pTableInfo = GetSkillTask().GetSkillLevelTableIDList(pSkill->GetClassID(), pSkill->GetSelectedLevelDataType());
				SKILL_LEVEL_TABLE_IDS::iterator findIter = pTableInfo->_SkillLevelTableIDs.find(pSkill->GetLevel());
				if (findIter != pTableInfo->_SkillLevelTableIDs.end())
					nSkillLevelTableID = findIter->second;

				nLevelLimit = pSkillLevelTable->GetFieldFromLablePtr( nSkillLevelTableID, "_LevelLimit" )->GetInteger();
#else
				pSkillInfo = pSkill->GetInfo();
				nLevelLimit = pSkillLevelTable->GetFieldFromLablePtr( pSkillInfo->iSkillLevelIDOffset+pSkill->GetLevel(), "_LevelLimit" )->GetInteger();
#endif // PRE_FIX_NEXTSKILLINFO

				if( ( CDnActor::s_hLocalActor->GetLevel() >= nLevelLimit ) && ( nSkillPoint >= pSkill->GetNextLevelSkillPoint() ) )
				{
					pSkillSlot->m_pSkillUpButton->Enable( true );
					pSkillSlot->m_pSkillUpButton->SetBlink( true );
				}

				if( nSkillPoint < pSkill->GetNextLevelSkillPoint() )
				{
					pSkillSlot->m_pSkillPoint->SetTextColor( EtInterface::textcolor::FONT_ALERT );
				}
			}
		}
	}
}

void CDnSkillDlg::OnSkillLevelUp( bool bSuccessed )
{
	//if( bSuccessed )
	//{
	//	if( m_nSelectedIndex >= 0 )
	//	{
	//		m_vecSkillSlot[m_nSelectedIndex].m_pSlotButton->SetCompleteCoolTime( 0.5f );
	//	}

	//	m_nSelectedIndex = -1;

	//	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1134 ) );
	//}
	//else
	//{
	//}
}

void CDnSkillDlg::UpdatePage()
{
	UpdateSkillPoint();
	UpdateSkillISlot();
	
	UpdateMaxPageNum();
	UpdatePageButton();
}