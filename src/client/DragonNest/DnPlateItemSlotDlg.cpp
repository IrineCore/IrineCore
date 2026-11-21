#include "StdAfx.h"
#include "DnPlateItemSlotDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPlateItemSlotDlg::CDnPlateItemSlotDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	m_nMaxItemSlot = 0;
	for( int i = 0; i < 5; ++i )
	{
		m_pItem[i] = NULL;
		m_pItemSlotButton[i] = NULL;
		m_pItemSlotEffect[i] = NULL;
	}
}

CDnPlateItemSlotDlg::~CDnPlateItemSlotDlg(void)
{
	for( int i = 0; i < 5; ++i )
	{
		SAFE_DELETE(m_pItem[i]);
	}
}

void CDnPlateItemSlotDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
	}
	else
	{
		for( int i = 1; i <= m_nMaxItemSlot; ++i )
		{
			SAFE_DELETE( m_pItem[i-1] );
			m_pItemSlotButton[i-1]->ResetSlot();
			m_pItemSlotEffect[i-1]->Show(false);
			m_pItemSlotEffect[i-1]->SetBlink(false);
		}
	}

	CEtUIDialog::Show( bShow );
}

void CDnPlateItemSlotDlg::Init( int nMaxItemSlot )
{
	// Prevent Initialize from being called twice.
	ASSERT(!m_nMaxItemSlot&&"PlateItemSlotDlg::Init called twice");
	m_nMaxItemSlot = nMaxItemSlot;

	char szFilename[32];
	sprintf_s(szFilename, _countof(szFilename), "%s%d%s", "PlateItemSlotDlg0", nMaxItemSlot, ".ui");
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( szFilename ).c_str(), false );
}

void CDnPlateItemSlotDlg::InitialUpdate()
{
	char szName[32];
	for( int i = 1; i <= m_nMaxItemSlot; ++i )
	{
		sprintf_s(szName, _countof(szName), "%s%d", "ID_ITEM", i);
		m_pItemSlotButton[i-1] = GetControl<CDnItemSlotButton>(szName);
		m_pItemSlotButton[i-1]->SetSlotType( ST_JEWEL );

		sprintf_s(szName, _countof(szName), "%s%d", "ID_SLOT_EFFECT", i);
		m_pItemSlotEffect[i-1] = GetControl<CEtUIStatic>(szName);
	}
}

bool CDnPlateItemSlotDlg::SetCompoundInfo(int *pItemID, int *pItemCount)
{
	bool bOK = true;
	for( int i = 1; i <= m_nMaxItemSlot; ++i )
	{
		// Get required item ID and quantity,
		int nItemID = pItemID[i-1];
		int nItemCount = pItemCount[i-1];

		// Check item types and counts in inventory.
		int nCurItemCount = GetItemTask().GetCharInventory().GetItemCount( nItemID );
		if( nCurItemCount < nItemCount )
			bOK = false;

		// Even if the count is insufficient, set it and proceed.
		SetItemSlot( i-1, nItemID, nItemCount, nCurItemCount );
	}
	return bOK;
}

void CDnPlateItemSlotDlg::SetItemSlot( int nSlotIndex, int nItemID, int nNeedItemCount, int nCurItemCount )
{
	if( nItemID == 0 )
		return;

	SAFE_DELETE( m_pItem[nSlotIndex] );

	TItemInfo itemInfo;
	if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) return;
	m_pItem[nSlotIndex] = GetItemTask().CreateItem( itemInfo );
	m_pItemSlotButton[nSlotIndex]->SetItem(m_pItem[nSlotIndex], CDnSlotButton::ITEM_ORIGINAL_COUNT);
	m_pItemSlotButton[nSlotIndex]->SetJewelCount( nNeedItemCount, nCurItemCount );

	if( nCurItemCount < nNeedItemCount )
	{
		m_pItemSlotEffect[nSlotIndex]->Show( false );
		m_pItemSlotButton[nSlotIndex]->SetRegist( true );
	}
	else
	{
		m_pItemSlotButton[nSlotIndex]->SetRegist( false );
		m_pItemSlotEffect[nSlotIndex]->SetBlink( true );
		m_pItemSlotEffect[nSlotIndex]->Show( true );
	}
}