#include "StdAfx.h"
#include "DnPotentialTransferInfoDlg.h"
#include "DnPotentialTransferInfoItemDlg.h"
#include "DnTableDB.h"
#include "DnWeapon.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef PRE_ADD_EXCHANGE_POTENTIAL

CDnPotentialTransferInfoDlg::CDnPotentialTransferInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pComboSort0(NULL)
, m_pComboSort1(NULL)
, m_pListBoxEx(NULL)
, m_pButtonPagePrev(NULL)
, m_pButtonPageNext(NULL)
, m_pStaticPage(NULL)
, m_nCurPage(0)
, m_nMaxPage(0)
{
}

CDnPotentialTransferInfoDlg::~CDnPotentialTransferInfoDlg(void)
{
	m_pListBoxEx->RemoveAllItems();
}

void CDnPotentialTransferInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "InvenPotentialOptionDlg.ui" ).c_str(), bShow );
}

void CDnPotentialTransferInfoDlg::InitialUpdate()
{
	m_pComboSort0 = GetControl<CEtUIComboBox>("ID_COMBOBOX_SORT0");
	m_pComboSort1 = GetControl<CEtUIComboBox>("ID_COMBOBOX_SORT1");
	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_LIST");
	m_pButtonPagePrev = GetControl<CEtUIButton>("ID_BT_PRIOR");
	m_pButtonPageNext = GetControl<CEtUIButton>("ID_BT_NEXT");
	m_pStaticPage = GetControl<CEtUIStatic>("ID_TEXT_PAGE");
	m_pButtonPagePrev->Enable(false);
	m_pButtonPageNext->Enable(false);

	m_pComboSort0->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1712 ), NULL, 1 );
	m_pComboSort0->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1713 ), NULL, 2 );
	m_pComboSort0->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2288 ), NULL, 3 );

	m_nCurPage = 0;
	InitControl();
	UpdateList();
}

void CDnPotentialTransferInfoDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		UpdateTransferCurrentPage();
	}
	else
	{
		m_pListBoxEx->RemoveAllItems();
	}

	CEtUIDialog::Show( bShow );
}

void CDnPotentialTransferInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_CLOSE") || IsCmdControl("ID_CANCEL") )
		{
			Show(false);
			return;
		}
		else if( IsCmdControl("ID_BT_PRIOR") )
		{
			PrevPage();
			UpdateTransferCurrentPage();
			return;
		}
		else if( IsCmdControl("ID_BT_NEXT") )
		{
			NextPage();
			UpdateTransferCurrentPage();
			return;
		}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED ) 
	{
		if( IsCmdControl("ID_COMBOBOX_SORT0") )
		{
			UpdateList();
		}
		else if( IsCmdControl("ID_COMBOBOX_SORT1") )
		{
			UpdateTransferList();
			UpdatePage();
			UpdateTransferCurrentPage();
		}
		// Clear focus from combo box so wheel messages do not go to focus::HandleMouse after changing combo box.
		focus::ReleaseControl();
		return;
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnPotentialTransferInfoDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	if( m_pComboSort0 && m_pComboSort1 &&
		!m_pComboSort0->IsOpenedDropDownBox() && !m_pComboSort1->IsOpenedDropDownBox() )
	{
		switch( uMsg )
		{
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
						if( m_nCurPage > 0 )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_PRIOR"), 0 );
					}
					else if( nScrollAmount < 0 )
					{
						if( m_nCurPage < m_nMaxPage )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_NEXT"), 0 );
					}
					return true;
				}
			}
			break;
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnPotentialTransferInfoDlg::InitControl()
{
	m_pStaticPage->SetText(L"");
	m_pButtonPagePrev->Enable(false);
	m_pButtonPageNext->Enable(false);
	m_pListBoxEx->RemoveAllItems();
}

void CDnPotentialTransferInfoDlg::UpdateList()
{
	UpdateComboBoxSub();
	UpdateTransferList();
	UpdatePage();
	UpdateTransferCurrentPage();
}

void CDnPotentialTransferInfoDlg::UpdateComboBoxSub()
{
	if( !m_pComboSort1 ) return;

	m_pComboSort1->RemoveAllItems();

	std::vector<int> vecType;
	SComboBoxItem *pItem = m_pComboSort0->GetSelectedItem();
	if( !pItem ) return;

	switch( pItem->nValue )
	{
	case 1:
		vecType.push_back(CDnWeapon::Sword);
		vecType.push_back(CDnWeapon::Axe);
		vecType.push_back(CDnWeapon::Hammer);
		vecType.push_back(CDnWeapon::Gauntlet);
		vecType.push_back(CDnWeapon::SmallBow);
		vecType.push_back(CDnWeapon::BigBow);
		vecType.push_back(CDnWeapon::CrossBow);
		vecType.push_back(CDnWeapon::Arrow);
		vecType.push_back(CDnWeapon::Staff);
		vecType.push_back(CDnWeapon::Orb);
		vecType.push_back(CDnWeapon::Puppet);
		vecType.push_back(CDnWeapon::Book);
		vecType.push_back(CDnWeapon::Mace);
		vecType.push_back(CDnWeapon::Flail);
		vecType.push_back(CDnWeapon::Wand);
		vecType.push_back(CDnWeapon::Shield);
#ifdef PRE_ADD_ACADEMIC
#ifndef PRE_REMOVE_ACADEMIC
		vecType.push_back(CDnWeapon::Cannon);
		vecType.push_back(CDnWeapon::BubbleGun);
		vecType.push_back(CDnWeapon::Glove);
#endif
#endif
#ifdef PRE_ADD_KALI
#ifndef PRE_REMOVE_KALI
		vecType.push_back(CDnWeapon::Fan);
		vecType.push_back(CDnWeapon::Chakram);
		vecType.push_back(CDnWeapon::Charm);
#endif
#endif
		for( int i = 0; i < (int)vecType.size(); ++i )
			m_pComboSort1->AddItem( DN_INTERFACE::STRING::ITEM::GetEquipString( (CDnWeapon::EquipTypeEnum)vecType[i]), NULL, vecType[i] );
		
		break;
	case 2:
		vecType.push_back(CDnParts::Helmet);
		vecType.push_back(CDnParts::Body);
		vecType.push_back(CDnParts::Leg);
		vecType.push_back(CDnParts::Hand);
		vecType.push_back(CDnParts::Foot);

		for( int i = 0; i < (int)vecType.size(); ++i )
			m_pComboSort1->AddItem( DN_INTERFACE::STRING::ITEM::GetPartsString( (CDnParts::PartsTypeEnum)vecType[i]), NULL, vecType[i] );
		break;
	case 3:
		vecType.push_back(CDnParts::Necklace);
		vecType.push_back(CDnParts::Earring);
		vecType.push_back(CDnParts::Ring);

		for( int i = 0; i < (int)vecType.size(); ++i )
			m_pComboSort1->AddItem( DN_INTERFACE::STRING::ITEM::GetPartsString( (CDnParts::PartsTypeEnum)vecType[i]), NULL, vecType[i] );
		break;
	}
}

void CDnPotentialTransferInfoDlg::UpdateTransferList()
{
	m_vecTransferItemID.clear();
	m_nMaxPage = 0;
	m_nCurPage = 0;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPOTENTIAL_TRANS );
	if( !pSox ) return;

	int nItemType = -1;
	SComboBoxItem *pItem = m_pComboSort0->GetSelectedItem();
	if( !pItem ) return;
	switch( pItem->nValue )
	{
	case 1: nItemType = 0; break;
	case 2:
	case 3:	nItemType = 1; break;
	}
	if( nItemType == -1 ) return;

	int nItemSubType = -1;
	pItem = m_pComboSort1->GetSelectedItem();
	if( !pItem ) return;
	nItemSubType = pItem->nValue;

	for (int i = 0; i < (int)pSox->GetItemCount(); i++)
	{
		int nItemID = pSox->GetItemID( i );
		int nMainType = pSox->GetFieldFromLablePtr( nItemID, "_MainType" )->GetInteger();
		int nSubType = pSox->GetFieldFromLablePtr( nItemID, "_SubType" )->GetInteger();

		if( nMainType == nItemType && nSubType == nItemSubType )
			m_vecTransferItemID.push_back(nItemID);
	}

	m_nMaxPage = ((int)m_vecTransferItemID.size()-1) / NUM_MAX_TRANSFER_ITEM;
}

void CDnPotentialTransferInfoDlg::PrevPage()
{
	--m_nCurPage;
	if( m_nCurPage < 0 )
		m_nCurPage = 0;

	UpdatePage();
}

void CDnPotentialTransferInfoDlg::NextPage()
{
	++m_nCurPage;
	if( m_nCurPage > m_nMaxPage )
		m_nCurPage = m_nMaxPage;

	UpdatePage();
}

void CDnPotentialTransferInfoDlg::UpdatePage()
{
	wchar_t wszPage[10]={0};
	swprintf_s( wszPage, 10, L"%d/%d", m_nCurPage+1, m_nMaxPage+1 );
	m_pStaticPage->SetText( wszPage );

	if( m_nCurPage == m_nMaxPage )
		m_pButtonPageNext->Enable(false);
	else
		m_pButtonPageNext->Enable(true);

	if( m_nCurPage == 0 )
		m_pButtonPagePrev->Enable(false);
	else
		m_pButtonPagePrev->Enable(true);
}

void CDnPotentialTransferInfoDlg::UpdateTransferCurrentPage()
{
	m_pListBoxEx->RemoveAllItems();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPOTENTIAL_TRANS );
	if( !pSox ) return;

	WCHAR wszText1[64] = {0,};
	WCHAR wszText2[64] = {0,};
	WCHAR wszText3[64] = {0,};

	int nOffset = m_nCurPage * NUM_MAX_TRANSFER_ITEM;
	for( int i = nOffset; i < (int)m_vecTransferItemID.size(); ++i )
	{
		if( i >= nOffset + NUM_MAX_TRANSFER_ITEM )
			break;

		int nLevel1 = pSox->GetFieldFromLablePtr( m_vecTransferItemID[i], "_FromLevel" )->GetInteger();
		int nRank1 = pSox->GetFieldFromLablePtr( m_vecTransferItemID[i], "_FromGrade" )->GetInteger();
		int nLevel2 = pSox->GetFieldFromLablePtr( m_vecTransferItemID[i], "_ToLevel" )->GetInteger();
		int nRank2 = pSox->GetFieldFromLablePtr( m_vecTransferItemID[i], "_ToGrade" )->GetInteger();
		int nCount = pSox->GetFieldFromLablePtr( m_vecTransferItemID[i], "_Count" )->GetInteger();
		DWORD dwColor = DN_INTERFACE::STRING::ITEM::RANK_2_COLOR( (eItemRank)nRank1 );

		swprintf_s( wszText1, _countof(wszText1), L"%d %s", nLevel1, DN_INTERFACE::STRING::ITEM::GetRankString( (eItemRank)nRank1 ) );
		swprintf_s( wszText2, _countof(wszText2), L"%d %s", nLevel2, DN_INTERFACE::STRING::ITEM::GetRankString( (eItemRank)nRank2 ) );
		swprintf_s( wszText3, _countof(wszText3), L"%d", nCount );

		CDnPotentialTransferInfoItemDlg *pItemDlg = m_pListBoxEx->AddItem<CDnPotentialTransferInfoItemDlg>();
		pItemDlg->SetInfo( wszText1, wszText2, wszText3, dwColor );
	}
}

#endif