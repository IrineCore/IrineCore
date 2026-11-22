#include "StdAfx.h"

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnCharTalismanListDlg.h"
#include "DnCharTalismanDlg.h"
#include "DnTooltipTalismanDlg.h"
#include "DnTalisman.h"
#include "DnItemTask.h"
#include "DnInterface.h"
#include "DnTableDB.h"
#include "DnLocalPlayerActor.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#include "ItemSendPacket.h"
#include "SyncTimer.h"
#include "DnCommonUtil.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const float CDnCharTalismanListDlg::TALISMAN_LIST_TOP_PADDING = 0.0065f;

CDnCharTalismanListDlg::CDnCharTalismanListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pParentTalismanDlg( NULL )
, m_pTooltipTalismanDlg( NULL )
, m_bDragMode( false )
, m_pSelectedCover( NULL )
, m_pSelectedDetachItem( NULL )
, m_nSelectedDetachSlot( -1 )
, m_fContentHeight( 0.0f )
{
}

CDnCharTalismanListDlg::~CDnCharTalismanListDlg()
{
	SAFE_DELETE( m_pTooltipTalismanDlg );
}

void CDnCharTalismanListDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharTalismanListDlg.ui" ).c_str(), bShow );
}

void CDnCharTalismanListDlg::InitialUpdate()
{
	// Override UI alignment (which defaults to AT_VERT_CENTER / AT_HORI_CENTER) so
	// SetPosition() positions this list at exact screen coordinates inside the parent listbox.
	SetAllignType( AT_HORI_LEFT_FIXED, AT_VERT_TOP );

	InitTalismanSlot();

	m_pTooltipTalismanDlg = new CDnTooltipTalismanDlg( UI_TYPE_TOP_MSG );
	m_pTooltipTalismanDlg->Initialize( false );

	if( !m_pParentTalismanDlg && m_pParentDialog )
		m_pParentTalismanDlg = dynamic_cast<CDnCharTalismanDlg*>( m_pParentDialog );
}

void CDnCharTalismanListDlg::InitTalismanSlot()
{
	m_vecSlotInfo.clear();
	m_fContentHeight = 0.0f;

	char szControlName[64] = {0,};
	wchar_t wszEff[32] = {0,};

	// First hide all template slots (UI defines 48 slots; table may enable only a subset).
	for( int i = 0; i < TALISMAN_UI_SLOT_MAX; ++i )
	{
		bool bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_SLOT%d", i );
		CEtUIStatic *pStaticSlot = GetControl<CEtUIStatic>( szControlName, &bExist );
		if( bExist && pStaticSlot ) pStaticSlot->Show( false );

		bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_BT_ITEM%d", i );
		CDnItemSlotButton *pSlotButton = GetControl<CDnItemSlotButton>( szControlName, &bExist );
		if( bExist && pSlotButton ) pSlotButton->Show( false );

		bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_TEXT_OPTION%d", i );
		CEtUIStatic *pStaticEff = GetControl<CEtUIStatic>( szControlName, &bExist );
		if( bExist && pStaticEff ) pStaticEff->Show( false );

		bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_COVER%d", i );
		CEtUIStatic *pStaticCover = GetControl<CEtUIStatic>( szControlName, &bExist );
		if( bExist && pStaticCover ) pStaticCover->Show( false );

		bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_CASHCOVER%d", i );
		CEtUIStatic *pStaticCashCover = GetControl<CEtUIStatic>( szControlName, &bExist );
		if( bExist && pStaticCashCover ) pStaticCashCover->Show( false );

		if( i <= TALISMAN_CASH_MAX )
		{
			bExist = false;
			sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_TIME%d", i );
			CEtUIStatic *pStaticTime = GetControl<CEtUIStatic>( szControlName, &bExist );
			if( bExist && pStaticTime ) pStaticTime->Show( false );
		}
	}

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TTALISMANSLOT );
	if( !pSox ) return;

	int nCount = pSox->GetItemCount();
	if( nCount > TALISMAN_MAX )
		nCount = TALISMAN_MAX;

	for( int i = 0; i < nCount; ++i )
	{
		STalismanSlotInfo info;
		info.nTableRowID = pSox->GetItemID( i );
		info.bService = ( pSox->GetFieldFromLablePtr( info.nTableRowID, "_Service" )->GetInteger() != 0 );
		info.nLevel = pSox->GetFieldFromLablePtr( info.nTableRowID, "_Level" )->GetInteger();
		info.nAmount = pSox->GetFieldFromLablePtr( info.nTableRowID, "_Amount" )->GetInteger();
		info.nNeedItemID = pSox->GetFieldFromLablePtr( info.nTableRowID, "_item" )->GetInteger();
		info.nNeedItemCount = pSox->GetFieldFromLablePtr( info.nTableRowID, "_itemCount" )->GetInteger();
		info.fEfficiency = pSox->GetFieldFromLablePtr( info.nTableRowID, "_Efficiency" )->GetFloat();
		info.nDescriptionID = pSox->GetFieldFromLablePtr( info.nTableRowID, "_Description" )->GetInteger();
		info.nSlotType = pSox->GetFieldFromLablePtr( info.nTableRowID, "_Type" )->GetInteger();
		info.szTexture = pSox->GetFieldFromLablePtr( info.nTableRowID, "_Texture" )->GetString();
		info.bOpen = ( info.bService && info.nSlotType == TALISMAN_BASIC );

		bool bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_SLOT%d", i );
		info.pStaticSlot = GetControl<CEtUIStatic>( szControlName, &bExist );
		if( !bExist ) info.pStaticSlot = NULL;
		if( info.pStaticSlot )
		{
			info.pStaticSlot->Show( info.bService );
			if( info.bService )
			{
				SUICoord slotCoord;
				info.pStaticSlot->GetUICoord( slotCoord );
				float fSlotBottom = slotCoord.Bottom() + TALISMAN_LIST_TOP_PADDING;
				if( fSlotBottom > m_fContentHeight )
					m_fContentHeight = fSlotBottom;
			}
		}

		bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_BT_ITEM%d", i );
		info.pSlotButton = GetControl<CDnItemSlotButton>( szControlName, &bExist );
		if( !bExist ) info.pSlotButton = NULL;
		if( info.pSlotButton )
		{
			info.pSlotButton->SetSlotIndex( i );
			info.pSlotButton->SetSlotType( ( i <= TALISMAN_CASH_MAX ) ? ST_TALISMAN_CASH : ST_TALISMAN );
			info.pSlotButton->Show( info.bService );
		}

		bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_TEXT_OPTION%d", i );
		info.pStaticEfficiency = GetControl<CEtUIStatic>( szControlName, &bExist );
		if( !bExist ) info.pStaticEfficiency = NULL;
		if( info.pStaticEfficiency )
		{
			swprintf_s( wszEff, _countof(wszEff), L"%.f%%", info.fEfficiency * 100.0f );
			info.pStaticEfficiency->SetText( wszEff );
			info.pStaticEfficiency->Show( false );
			info.pStaticEfficiency->Enable( false );
		}

		bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_COVER%d", i );
		info.pStaticCover = GetControl<CEtUIStatic>( szControlName, &bExist );
		if( !bExist ) info.pStaticCover = NULL;
		if( info.pStaticCover )
		{
			info.pStaticCover->Show( info.bService && info.nSlotType == TALISMAN_EXPANSION );
			info.pStaticCover->Enable( info.pStaticCover->IsShow() );
			info.pStaticCover->SetID( i );
		}

		bExist = false;
		sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_CASHCOVER%d", i );
		info.pStaticCashCover = GetControl<CEtUIStatic>( szControlName, &bExist );
		if( !bExist ) info.pStaticCashCover = NULL;
		if( info.pStaticCashCover )
		{
			info.pStaticCashCover->Show( info.bService && info.nSlotType == TALISMAN_CASH_EXPANSION );
			info.pStaticCashCover->Enable( info.pStaticCashCover->IsShow() );
			info.pStaticCashCover->SetID( i );
		}

		if( i <= TALISMAN_CASH_MAX )
		{
			bExist = false;
			sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_TIME%d", i );
			info.pStaticTime = GetControl<CEtUIStatic>( szControlName, &bExist );
			if( !bExist ) info.pStaticTime = NULL;
			if( info.pStaticTime )
				info.pStaticTime->Show( false );
		}

		m_vecSlotInfo.push_back( info );
	}
}

void CDnCharTalismanListDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
		HideTooltip();

	CDnCustomDlg::Show( bShow );
}

void CDnCharTalismanListDlg::HideTooltip()
{
	if( m_pTooltipTalismanDlg && m_pTooltipTalismanDlg->IsShow() )
		m_pTooltipTalismanDlg->Show( false );
}

bool CDnCharTalismanListDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( m_pParentTalismanDlg && !m_pParentTalismanDlg->IsShow() )
		return false;

	// Clip mouse events to parent listbox bounds
	if( m_pParentTalismanDlg && m_pParentTalismanDlg->GetListBox() )
	{
		if( uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP )
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fScreenX = 0.0f, fScreenY = 0.0f;
			m_pParentTalismanDlg->PointToFloat( MousePoint, fScreenX, fScreenY );

			SUICoord listCoord;
			m_pParentTalismanDlg->GetListBox()->GetUICoord( listCoord );
			if( !listCoord.IsInside( fScreenX, fScreenY ) )
			{
				HideTooltip();
				return false;
			}
		}
	}

	if( uMsg == WM_MOUSEMOVE )
	{
		CDnSlotButton *pDragButton = (CDnSlotButton*)drag::GetControl();
		if( pDragButton == NULL )
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fLocalX = 0.0f, fLocalY = 0.0f;
			PointToFloat( MousePoint, fLocalX, fLocalY );
			float fMouseX = fLocalX + GetXCoord();
			float fMouseY = fLocalY + GetYCoord();

			bool bHoveredLocked = false;
			for( size_t i = 0; i < m_vecSlotInfo.size(); ++i )
			{
				STalismanSlotInfo &slot = m_vecSlotInfo[i];
				if( !slot.bService || slot.bOpen )
					continue;

				CEtUIStatic *pCover = ( slot.nSlotType == TALISMAN_CASH_EXPANSION ) ? slot.pStaticCashCover : slot.pStaticCover;
				if( pCover && pCover->IsShow() )
				{
					SUICoord coverCoord;
					pCover->GetUICoord( coverCoord );
					if( coverCoord.IsInside( fLocalX, fLocalY ) )
					{
						if( m_pTooltipTalismanDlg )
						{
							m_pTooltipTalismanDlg->SetTalismanSlot( slot.nTableRowID, slot.nSlotType == TALISMAN_CASH_EXPANSION );
							m_pTooltipTalismanDlg->SetPosition( fMouseX, fMouseY );
							m_pTooltipTalismanDlg->AdjustPosition();
							m_pTooltipTalismanDlg->Show( true );
						}
						bHoveredLocked = true;
						break;
					}
				}
			}

			if( !bHoveredLocked )
				HideTooltip();
		}
		else
		{
			HideTooltip();
		}
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnCharTalismanListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( strstr( pControl->GetControlName(), "ID_STATIC_COVER" ) )
		{
			if( uMsg == WM_RBUTTONUP )
			{
				CEtUIStatic *pCover = dynamic_cast<CEtUIStatic*>( pControl );
				if( pCover )
				{
					int nSlotIdx = pCover->GetID();
					if( nSlotIdx >= 0 && nSlotIdx < (int)m_vecSlotInfo.size() && m_vecSlotInfo[nSlotIdx].bService && !m_vecSlotInfo[nSlotIdx].bOpen )
					{
						m_pSelectedCover = pCover;
						GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8360 ), MB_YESNO, TALISMAN_OPENSLOT_MSG_ID, this );
					}
				}
			}
			return;
		}

		if( strstr( pControl->GetControlName(), "ID_BT_ITEM" ) )
		{
			if( GetItemTask().IsRequestWait() ) return;
			if( GetInterface().IsOpenAcceptRequestDialog() ) return;

			CDnItemSlotButton *pPressedButton = static_cast<CDnItemSlotButton*>( pControl );
			CDnSlotButton *pDragButton = (CDnSlotButton*)drag::GetControl();

			if( pDragButton == NULL )
			{
				CDnItem *pItem = static_cast<CDnItem*>( pPressedButton->GetItem() );
				if( !pItem ) return;

				if( pPressedButton->GetSlotType() == ST_TALISMAN_CASH && !GetItemTask().IsCashTalismanOpen() )
				{
					GetInterface().MessageBox( 8617, MB_OK );
					return;
				}

				if( uMsg == WM_RBUTTONUP )
				{
					int nMoveCost = GetItemTask().GetTalismanMoveCost();
					if( nMoveCost > 0 )
					{
						int nGold = nMoveCost / 10000;
						int nSilver = ( nMoveCost % 10000 ) / 100;
						int nCopper = nMoveCost % 100;
						wchar_t wszMsg[256] = {0,};
						swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8489 ), nGold, nSilver, nCopper );
						m_pSelectedDetachItem = pItem;
						m_nSelectedDetachSlot = pPressedButton->GetSlotIndex();
						GetInterface().MessageBox( wszMsg, MB_YESNO, TALISMAN_DETACH_MSG_ID, this );
					}
					else
					{
						int nEmptySlot = GetItemTask().GetCharInventory().FindFirstEmptyIndex();
						if( nEmptySlot >= 0 )
							GetItemTask().RequestMoveItem( MoveType_TalismanToInven, pPressedButton->GetSlotIndex(), pItem->GetSerialID(), nEmptySlot, pItem->GetOverlapCount() );
					}
					return;
				}
				else if( uMsg == WM_LBUTTONDOWN )
				{
					pPressedButton->EnableSplitMode( 0 );
					drag::SetControl( pPressedButton );
					m_bDragMode = true;
					return;
				}
			}
			else
			{
				drag::ReleaseControl();
				m_bDragMode = false;

				CDnItem *pDragItem = static_cast<CDnItem*>( pDragButton->GetItem() );
				if( !pDragItem || pDragItem->GetItemType() != ITEMTYPE_TALISMAN )
					return;

				int nDestSlot = pPressedButton->GetSlotIndex();
				if( nDestSlot < 0 || nDestSlot >= (int)m_vecSlotInfo.size() )
					return;
				if( !GetItemTask().IsOpenTalismanSlot( nDestSlot ) )
					return;

				if( pDragButton->GetSlotType() == ST_INVENTORY )
				{
					GetItemTask().RequestMoveItem( MoveType_InvenToTalisman, pDragButton->GetSlotIndex(), pDragItem->GetSerialID(), nDestSlot, pDragItem->GetOverlapCount() );
				}
				else if( pDragButton->GetSlotType() == ST_TALISMAN || pDragButton->GetSlotType() == ST_TALISMAN_CASH )
				{
					if( pDragButton->GetSlotIndex() != nDestSlot )
					{
						GetItemTask().RequestMoveItem( MoveType_Talisman, pDragButton->GetSlotIndex(), pDragItem->GetSerialID(), nDestSlot, pDragItem->GetOverlapCount() );
					}
				}
			}
		}
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnCharTalismanListDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( nID == TALISMAN_OPENSLOT_MSG_ID )
		{
			if( IsCmdControl( "ID_YES" ) && m_pSelectedCover )
			{
				SendTalismanOpenSlot( m_pSelectedCover->GetID() );
			}
			m_pSelectedCover = NULL;
		}
		else if( nID == TALISMAN_DETACH_MSG_ID )
		{
			if( IsCmdControl( "ID_YES" ) && m_pSelectedDetachItem && m_nSelectedDetachSlot >= 0 )
			{
				int nEmptySlot = GetItemTask().GetCharInventory().FindFirstEmptyIndex();
				if( nEmptySlot >= 0 )
				{
					GetItemTask().RequestMoveItem( MoveType_TalismanToInven, m_nSelectedDetachSlot, m_pSelectedDetachItem->GetSerialID(), nEmptySlot, m_pSelectedDetachItem->GetOverlapCount() );
				}
				else
				{
					GetInterface().MessageBox( 7012, MB_OK );
				}
			}
			m_pSelectedDetachItem = NULL;
			m_nSelectedDetachSlot = -1;
		}
	}
}

void CDnCharTalismanListDlg::SetTalismanItem( int nSlotIndex, CDnItem *pItem )
{
	if( nSlotIndex < 0 || nSlotIndex >= (int)m_vecSlotInfo.size() )
		return;
	if( !m_vecSlotInfo[nSlotIndex].pSlotButton )
		return;

	m_vecSlotInfo[nSlotIndex].pSlotButton->SetItem( pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
	m_vecSlotInfo[nSlotIndex].pSlotButton->OnRefreshTooltip();
}

void CDnCharTalismanListDlg::ResetTalismanSlot( int nSlotIndex )
{
	if( nSlotIndex < 0 || nSlotIndex >= (int)m_vecSlotInfo.size() )
		return;
	if( !m_vecSlotInfo[nSlotIndex].pSlotButton )
		return;

	m_vecSlotInfo[nSlotIndex].pSlotButton->ResetSlot();
	m_vecSlotInfo[nSlotIndex].pSlotButton->OnRefreshTooltip();
}

void CDnCharTalismanListDlg::RefreshTalisman()
{
	if( !CDnItemTask::IsActive() ) return;

	for( size_t i = 0; i < m_vecSlotInfo.size(); ++i )
	{
		CDnItem *pItem = GetItemTask().GetTalismanItem( (int)i );
		if( pItem )
			SetTalismanItem( (int)i, pItem );
		else
			ResetTalismanSlot( (int)i );
	}
	RefreshOpenSlot();
	RefreshCashSlot();
}

void CDnCharTalismanListDlg::RefreshOpenSlot()
{
	if( !CDnItemTask::IsActive() ) return;

	for( size_t i = 0; i < m_vecSlotInfo.size(); ++i )
	{
		STalismanSlotInfo &slot = m_vecSlotInfo[i];
		if( !slot.bService )
		{
			slot.bOpen = false;
			if( slot.pStaticCover ) slot.pStaticCover->Show( false );
			if( slot.pStaticCashCover ) slot.pStaticCashCover->Show( false );
			continue;
		}

		slot.bOpen = GetItemTask().IsOpenTalismanSlot( (int)i );

		if( slot.nSlotType == TALISMAN_EXPANSION )
		{
			if( slot.pStaticCover )
				slot.pStaticCover->Show( !slot.bOpen );
		}
		else if( slot.nSlotType == TALISMAN_CASH_EXPANSION )
		{
			if( slot.pStaticCashCover )
				slot.pStaticCashCover->Show( !slot.bOpen );
		}
		else
		{
			if( slot.pStaticCover )
				slot.pStaticCover->Show( false );
			if( slot.pStaticCashCover )
				slot.pStaticCashCover->Show( false );
		}
	}
}

void CDnCharTalismanListDlg::RefreshCashSlot()
{
	if( !CDnItemTask::IsActive() ) return;

	bool bCashOpen = GetItemTask().IsCashTalismanOpen();
	__time64_t tExpire = GetItemTask().GetCashTalismanExpireDate();
	__time64_t tCur = CSyncTimer::GetInstance().GetCurTime();

	std::wstring wszRemain;
	if( bCashOpen && tExpire > tCur )
	{
		__time64_t tDiff = tExpire - tCur;
		INT64 nDays = tDiff / 86400;
		INT64 nHours = ( tDiff % 86400 ) / 3600;
		wchar_t wszBuf[64] = {0,};
		if( nDays > 0 )
			swprintf_s( wszBuf, _countof(wszBuf), L"%I64d%s", nDays, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1213 ) );
		else
			swprintf_s( wszBuf, _countof(wszBuf), L"%I64d%s", max( (INT64)1, nHours ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1214 ) );
		wszRemain = wszBuf;
	}

	for( int i = TALISMAN_CASH1; i <= TALISMAN_CASH_MAX && i < (int)m_vecSlotInfo.size(); ++i )
	{
		if( !m_vecSlotInfo[i].bService )
			continue;

		m_vecSlotInfo[i].bOpen = bCashOpen;
		if( m_vecSlotInfo[i].pStaticCashCover )
			m_vecSlotInfo[i].pStaticCashCover->Show( !bCashOpen );
		if( m_vecSlotInfo[i].pStaticTime )
		{
			m_vecSlotInfo[i].pStaticTime->Show( bCashOpen && !wszRemain.empty() );
			if( !wszRemain.empty() )
				m_vecSlotInfo[i].pStaticTime->SetText( wszRemain );
		}
	}
}

void CDnCharTalismanListDlg::SetSlotMagneticMode( bool bMagnetic )
{
	for( size_t i = 0; i < m_vecSlotInfo.size(); ++i )
	{
		if( m_vecSlotInfo[i].pSlotButton )
			m_vecSlotInfo[i].pSlotButton->SetMagnetic( bMagnetic && m_vecSlotInfo[i].bOpen );
	}
}

void CDnCharTalismanListDlg::SetEfficiencyTextShow( bool bShow )
{
	for( size_t i = 0; i < m_vecSlotInfo.size(); ++i )
	{
		if( m_vecSlotInfo[i].pStaticEfficiency )
			m_vecSlotInfo[i].pStaticEfficiency->Show( bShow && m_vecSlotInfo[i].bService );
	}
}

MIInventoryItem *CDnCharTalismanListDlg::GetEquipTalisman( const CDnItem *pItem ) const
{
	if( !pItem || !CDnItemTask::IsActive() ) return NULL;

	int nEquipIndex = -1;
	if( GetItemTask().IsEquipTalismanItem( pItem->GetClassID(), nEquipIndex ) )
		return GetItemTask().GetTalismanItem( nEquipIndex );

	int nTypeParam = pItem->GetTypeParam( 1 );
	if( nTypeParam > 0 )
	{
		for( int i = 0; i < TALISMAN_MAX; ++i )
		{
			CDnItem *pEquipped = GetItemTask().GetTalismanItem( i );
			if( pEquipped && pEquipped->GetTypeParam( 1 ) == nTypeParam )
				return pEquipped;
		}
	}
	return NULL;
}

CDnCharStatusDlg::eRetWearable CDnCharTalismanListDlg::GetWearableTalismanEquipType( const MIInventoryItem *pItem, int &nSlotIndex, std::wstring *strErrorMsg )
{
	nSlotIndex = -1;
	const CDnItem *pDnItem = dynamic_cast<const CDnItem*>( pItem );
	if( !pDnItem || pDnItem->GetItemType() != ITEMTYPE_TALISMAN )
		return CDnCharStatusDlg::eWEAR_UNABLE;

	if( !CDnActor::s_hLocalActor || !CDnItemTask::IsActive() )
		return CDnCharStatusDlg::eWEAR_UNABLE;

	if( CDnActor::s_hLocalActor->GetLevel() < pDnItem->GetLevelLimit() )
	{
		if( strErrorMsg )
			*strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5070 );
		return CDnCharStatusDlg::eWEAR_NEED_UNSEAL;
	}

	if( GetItemTask().IsEquipTalismanType( pDnItem ) )
	{
		if( strErrorMsg )
			*strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8358 );
		return CDnCharStatusDlg::eWEAR_UNABLE;
	}

	// Find first open empty slot
	for( size_t i = 0; i < m_vecSlotInfo.size(); ++i )
	{
		if( !GetItemTask().IsOpenTalismanSlot( (int)i ) )
			continue;
		if( CDnActor::s_hLocalActor->GetLevel() < m_vecSlotInfo[i].nLevel )
			continue;
		if( GetItemTask().GetTalismanItem( (int)i ) == NULL )
		{
			nSlotIndex = (int)i;
			return CDnCharStatusDlg::eWEAR_ENABLE;
		}
	}

	return CDnCharStatusDlg::eWEAR_UNABLE;
}

void CDnCharTalismanListDlg::EquipTalismanFromInven( int nInvenIndex, MIInventoryItem *pItem, CDnSlotButton *pSlotButton )
{
	CDnItem *pDnItem = dynamic_cast<CDnItem*>( pItem );
	if( !pDnItem || pDnItem->GetItemType() != ITEMTYPE_TALISMAN )
		return;

	int nSlotIndex = -1;
	std::wstring wszErr;
	if( GetWearableTalismanEquipType( pDnItem, nSlotIndex, &wszErr ) != CDnCharStatusDlg::eWEAR_ENABLE || nSlotIndex < 0 )
	{
		if( !wszErr.empty() )
			GetInterface().MessageBox( wszErr.c_str(), MB_OK );
		return;
	}

	GetItemTask().RequestMoveItem( MoveType_InvenToTalisman, nInvenIndex, pDnItem->GetSerialID(), nSlotIndex, pDnItem->GetOverlapCount() );
}

const CDnCharTalismanListDlg::STalismanSlotInfo *CDnCharTalismanListDlg::GetSlotInfo( int nSlotIndex ) const
{
	if( nSlotIndex < 0 || nSlotIndex >= (int)m_vecSlotInfo.size() )
		return NULL;
	return &m_vecSlotInfo[nSlotIndex];
}

#endif // PRE_ADD_TALISMAN_SYSTEM
