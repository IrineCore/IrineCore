#include "StdAfx.h"

#include "DnPGStorageTabDlg.h"
#include "DnPlayerStorageDlg.h"
#include "DnGuildStorageDlgNew.h"
#include "DnInventoryDlg.h"
#include "DnInven.h"
#include "MIInventoryItem.h"
#include "DnSlotButton.h"
#include "DnInterface.h"
#include "DnLocalPlayerActor.h"
#include "GuildSendPacket.h"
#ifdef PRE_ADD_SERVER_WAREHOUSE
#include "DnWorldServerStorageInventoryDlg.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnPGStorageTabDlg::CDnPGStorageTabDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUITabDialog( dialogType, pParentDialog, nID, pCallback ),
	m_iRemoteItemID( 0 ),
	m_bOpenGuildStorage( false )
{

}

CDnPGStorageTabDlg::~CDnPGStorageTabDlg(void)
{
}

void CDnPGStorageTabDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "StorageTabDlg.ui" ).c_str(), bShow );
}

void CDnPGStorageTabDlg::InitialUpdate()
{
	CEtUIRadioButton *pTabButton(NULL);

	pTabButton = GetControl<CEtUIRadioButton>("ID_MENU_NORMAL");
	pTabButton->Show( true );
#ifdef PRE_ADD_SERVER_WAREHOUSE
	CDnPlayerStorageDlg* pPlayerStorageDlg = new CDnPlayerStorageDlg( UI_TYPE_CHILD, this,	StorageUIDef::eTAB_PLAYER );
#else
	CDnPlayerStorageDlg* pPlayerStorageDlg = new CDnPlayerStorageDlg( UI_TYPE_CHILD, this,	PLAYER_STORAGE_TAB );
#endif
	pPlayerStorageDlg->Initialize( false );
	AddTabDialog( pTabButton, pPlayerStorageDlg );
	m_vecTabButton.push_back(pTabButton);

	pTabButton = GetControl<CEtUIRadioButton>("ID_RBT_TAB0");
	pTabButton->Show( true );
#ifdef PRE_ADD_SERVER_WAREHOUSE
	CDnGuildStorageDlgNew* pGuildStorage = new CDnGuildStorageDlgNew( UI_TYPE_CHILD, this, StorageUIDef::eTAB_GUILD );
#else
	CDnGuildStorageDlgNew* pGuildStorage = new CDnGuildStorageDlgNew( UI_TYPE_CHILD, this, GUILD_STORAGE_TAB );
#endif
	pGuildStorage->Initialize( false );
	AddTabDialog( pTabButton, pGuildStorage );
	m_vecTabButton.push_back(pTabButton);

#ifdef PRE_ADD_SERVER_WAREHOUSE
	pTabButton = GetControl<CEtUIRadioButton>("ID_RBT_TAB1");
	pTabButton->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1612)); // UISTRING : Normal (Server)
#ifdef PRE_REMOVE_SERVER_WAREHOUSE
	pTabButton->Show(false);
#else
	pTabButton->Show(true);
	CDnWorldServerStorageInventoryDlg* pAccountStorageNormal = new CDnWorldServerStorageInventoryDlg(UI_TYPE_CHILD, this, StorageUIDef::eTAB_WORLDSERVER_NORMAL);
	pAccountStorageNormal->Initialize(false);
	AddTabDialog(pTabButton, pAccountStorageNormal);
	m_vecTabButton.push_back(pTabButton);
#endif // PRE_REMOVE_SERVER_WAREHOUSE

	pTabButton = GetControl<CEtUIRadioButton>("ID_RBT_TAB2");
	pTabButton->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1613)); // UISTRING : Cash (Server)
#ifdef PRE_REMOVE_SERVER_WAREHOUSE
	pTabButton->Show(false);
#else
	pTabButton->Show(true);
	CDnWorldServerStorageCashInventoryDlg* pAccountStorageCash = new CDnWorldServerStorageCashInventoryDlg(UI_TYPE_CHILD, this, StorageUIDef::eTAB_WORLDSERVER_CASH);
	pAccountStorageCash->Initialize(false);
	AddTabDialog(pTabButton, pAccountStorageCash);
	m_vecTabButton.push_back(pTabButton);
#endif // PRE_REMOVE_SERVER_WAREHOUSE
#endif
}

#ifdef PRE_MOD_SERVER_WAREHOUSE_NO_REMOTE
void CDnPGStorageTabDlg::MakeEnableStorageTab()
{
	bool bEnableTab[StorageUIDef::eTAB_MAX] = {0, };
	memset(bEnableTab, 0, sizeof(bEnableTab));
	if (m_iRemoteItemID >= 0)
		bEnableTab[StorageUIDef::eTAB_GUILD] = m_bOpenGuildStorage;

	if (m_iRemoteItemID <= 0)
	{
		bEnableTab[StorageUIDef::eTAB_WORLDSERVER_NORMAL] = true;
		bEnableTab[StorageUIDef::eTAB_WORLDSERVER_CASH] = true;
	}

	CEtUIRadioButton* pGuildStorageTabButton = GetControl<CEtUIRadioButton>("ID_RBT_TAB0");
	if (pGuildStorageTabButton != NULL)
		pGuildStorageTabButton->Enable(bEnableTab[StorageUIDef::eTAB_GUILD]);

	CEtUIRadioButton* pNormalWSStorageTabButton = GetControl<CEtUIRadioButton>("ID_RBT_TAB1");
	if (pNormalWSStorageTabButton != NULL)
		pNormalWSStorageTabButton->Enable(bEnableTab[StorageUIDef::eTAB_WORLDSERVER_NORMAL]);

	CEtUIRadioButton* pCashWSStorageTabButton = GetControl<CEtUIRadioButton>("ID_RBT_TAB2");
	if (pCashWSStorageTabButton != NULL)
		pCashWSStorageTabButton->Enable(bEnableTab[StorageUIDef::eTAB_WORLDSERVER_CASH]);
}
#else
	#ifdef PRE_ADD_SERVER_WAREHOUSE
void CDnPGStorageTabDlg::MakeEnableStorageTab(StorageUIDef::eStorageTabType type)
{
	switch(type)
	{
	case StorageUIDef::eTAB_GUILD:
		{
			CEtUIRadioButton* pGuildStorageTabButton = GetControl<CEtUIRadioButton>("ID_RBT_TAB0");
			if (pGuildStorageTabButton == NULL)
			{
				_ASSERT(0);
				return;
			}

			bool bEnableGuildStorage = false;
			if (m_iRemoteItemID >= 0)
			{
				if (m_bOpenGuildStorage)
					bEnableGuildStorage = true;
			}

			pGuildStorageTabButton->Enable(bEnableGuildStorage);
		}
		break;
	}
}
	#endif // PRE_ADD_SERVER_WAREHOUSE
#endif // PRE_MOD_SERVER_WAREHOUSE_NO_REMOTE

void CDnPGStorageTabDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	__super::Show( bShow );

	// After the storage unification task, when clicking "Use Storage" after talking to an NPC, the server 
	// Check if guild storage exists; if not, send personal storage open packet (SC_SHOWWAREHOUSE)
	// If the guild has storage, send the guild storage item list packet to open this dialog.
	// When opened via guild storage, personal storage is also visible, handling unified storage naturally.
	if( bShow )
	{
		// Always show personal storage first.
		if( false == m_vecTabButton.empty() )
		{
			SetCheckedTab( m_vecTabButton.front()->GetTabID() );
#ifdef PRE_ADD_SERVER_WAREHOUSE
			CDnInvenTabDlg* pInvenTabDlg = static_cast<CDnInvenTabDlg*>(GetInterface().GetMainMenuDialog(CDnMainMenuDlg::INVENTORY_DIALOG));
			if (pInvenTabDlg != NULL)
			{
				pInvenTabDlg->EnableInvenTab(CDnInvenTabDlg::eCOMMON);
				pInvenTabDlg->ShowTab(ST_INVENTORY);
			}
#endif
		}

#ifdef PRE_ADD_SERVER_WAREHOUSE
	#ifdef PRE_MOD_SERVER_WAREHOUSE_NO_REMOTE
		MakeEnableStorageTab();
	#else
		MakeEnableStorageTab(StorageUIDef::eTAB_GUILD);
	#endif
#else // PRE_ADD_ACCOUNT_STORAGE
		// Disable guild storage if opened in remote mode.
		CEtUIRadioButton* pGuildStorageTabButton = GetControl<CEtUIRadioButton>("ID_RBT_TAB0");

		if( 0 < m_iRemoteItemID )
		{
			pGuildStorageTabButton->Show( false );
		}
		else
		{
			if( m_bOpenGuildStorage )
				pGuildStorageTabButton->Show( true );
			else
				pGuildStorageTabButton->Show( false );
		}
#endif // PRE_ADD_ACCOUNT_STORAGE
	}
	else
	{
		// If guild storage is also open, close it together.
		if( m_bOpenGuildStorage )
			SendCloseGuildWare();

		if( drag::IsValid() )
		{
			CDnSlotButton *pDragButton = static_cast<CDnSlotButton*>(drag::GetControl());
			if( pDragButton->GetSlotType() == ST_STORAGE_PLAYER )
			{
				drag::Command(UI_DRAG_CMD_CANCEL);
				drag::ReleaseControl();
			}
		}

		GetInterface().CloseBlind();

		if( m_iRemoteItemID )
		{
			m_iRemoteItemID = 0;
			GetInterface().OpenBaseDialog();
			CDnLocalPlayerActor::LockInput( false );
		}
	}
}


CEtUIDialog* CDnPGStorageTabDlg::GetStorageDlg( int iTabID )
{
	CEtUIDialog* pResult = NULL;

	CEtUIDialog* pDialog = m_groupTabDialog.GetDialog( iTabID );
	if( pDialog )
	{
		switch( iTabID )
		{
#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_PLAYER:
#else
			case PLAYER_STORAGE_TAB:
#endif
				pResult = static_cast<CDnPlayerStorageDlg*>(pDialog);
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_GUILD:
#else
			case GUILD_STORAGE_TAB:
#endif
				pResult = static_cast<CDnGuildStorageDlgNew*>(pDialog);
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_WORLDSERVER_NORMAL:
				{
					pResult = static_cast<CDnWorldServerStorageInventoryDlg*>(pDialog);
				}
				break;
			case StorageUIDef::eTAB_WORLDSERVER_CASH:
				{
					pResult = static_cast<CDnWorldServerStorageCashInventoryDlg*>(pDialog);
				}
				break;
#endif

			default:
				pResult = NULL;
				break;
		}
	}

	return pResult;
}

CEtUIDialog* CDnPGStorageTabDlg::GetSelectedStorageDlg( void )
{
	return GetStorageDlg( GetCurrentTabID() );
}

void CDnPGStorageTabDlg::SetItem( MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnStoreDlg::SetItem");
	if( !pItem ) return;

	CEtUIDialog* pStorageDlg = GetSelectedStorageDlg();
	if( pStorageDlg )
	{
		switch( GetCurrentTabID() )
		{
#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_PLAYER:
#else
			case PLAYER_STORAGE_TAB:
#endif
				static_cast<CDnPlayerStorageDlg*>(pStorageDlg)->SetItem( pItem );
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_GUILD:
#else
			case GUILD_STORAGE_TAB:
#endif
				static_cast<CDnGuildStorageDlgNew*>(pStorageDlg)->SetItem( pItem );
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_WORLDSERVER_NORMAL:
				static_cast<CDnWorldServerStorageInventoryDlg*>(pStorageDlg)->SetItem(pItem);
				break;

			case StorageUIDef::eTAB_WORLDSERVER_CASH:
				static_cast<CDnWorldServerStorageCashInventoryDlg*>(pStorageDlg)->SetItem(pItem);
				break;
#endif
		}
	}
}

void CDnPGStorageTabDlg::ResetSlot( MIInventoryItem* pItem )
{
	CEtUIDialog* pStorageDlg = GetSelectedStorageDlg();
	if( pStorageDlg )
	{
		switch( GetCurrentTabID() )
		{
#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_PLAYER:
#else
			case PLAYER_STORAGE_TAB:
#endif
				static_cast<CDnPlayerStorageDlg*>(pStorageDlg)->ResetSlot( pItem );
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_GUILD:
#else
			case GUILD_STORAGE_TAB:
#endif
				static_cast<CDnGuildStorageDlgNew*>(pStorageDlg)->ResetSlot( pItem );
				break;
		}
	}
}

void CDnPGStorageTabDlg::SetUseItemCnt( DWORD dwItemCnt )
{
	CEtUIDialog* pStorageDlg = GetSelectedStorageDlg();
	if( pStorageDlg )
	{
		switch( GetCurrentTabID() )
		{
#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_PLAYER:
#else
			case PLAYER_STORAGE_TAB:
#endif
				static_cast<CDnPlayerStorageDlg*>(pStorageDlg)->SetUseItemCnt( dwItemCnt );
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_GUILD:
#else
			case GUILD_STORAGE_TAB:
#endif
				static_cast<CDnGuildStorageDlgNew*>(pStorageDlg)->SetUseItemCnt( dwItemCnt );
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_WORLDSERVER_NORMAL:
				static_cast<CDnWorldServerStorageInventoryDlg*>(pStorageDlg)->SetUseItemCnt(dwItemCnt);
				break;

			case StorageUIDef::eTAB_WORLDSERVER_CASH:
				static_cast<CDnWorldServerStorageCashInventoryDlg*>(pStorageDlg)->SetUseItemCnt(dwItemCnt);
				break;
#endif
		}
	}
}

bool CDnPGStorageTabDlg::IsEmptySlot()
{
	bool bResult = false;

	CEtUIDialog* pStorageDlg = GetSelectedStorageDlg();
	if( pStorageDlg )
	{
		switch( GetCurrentTabID() )
		{
#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_PLAYER:
#else
			case PLAYER_STORAGE_TAB:
#endif
				bResult = static_cast<CDnPlayerStorageDlg*>(pStorageDlg)->IsEmptySlot();
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_GUILD:
#else
			case GUILD_STORAGE_TAB:
#endif
				bResult = static_cast<CDnGuildStorageDlgNew*>(pStorageDlg)->IsEmptySlot();
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_WORLDSERVER_NORMAL:
				bResult = static_cast<CDnWorldServerStorageInventoryDlg*>(pStorageDlg)->IsEmptySlot();
				break;

			case StorageUIDef::eTAB_WORLDSERVER_CASH:
				bResult = static_cast<CDnWorldServerStorageCashInventoryDlg*>(pStorageDlg)->IsEmptySlot();
				break;
#endif
		}
	}

	return bResult;
}

int CDnPGStorageTabDlg::GetEmptySlot()
{
	int iSlotIndex = -1;

	CEtUIDialog* pStorageDlg = GetSelectedStorageDlg();
	if( pStorageDlg )
	{
		switch( GetCurrentTabID() )
		{
#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_PLAYER:
#else
			case PLAYER_STORAGE_TAB:
#endif
				iSlotIndex = static_cast<CDnPlayerStorageDlg*>(pStorageDlg)->GetEmptySlot();
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_GUILD:
#else
			case GUILD_STORAGE_TAB:
#endif
				iSlotIndex = static_cast<CDnGuildStorageDlgNew*>(pStorageDlg)->GetEmptySlot();
				break;

#ifdef PRE_ADD_SERVER_WAREHOUSE
			case StorageUIDef::eTAB_WORLDSERVER_NORMAL:
				iSlotIndex = static_cast<CDnWorldServerStorageInventoryDlg*>(pStorageDlg)->GetEmptySlot();
				break;

			case StorageUIDef::eTAB_WORLDSERVER_CASH:
				iSlotIndex = static_cast<CDnWorldServerStorageCashInventoryDlg*>(pStorageDlg)->GetEmptySlot();
				break;
#endif
		}
	}

	return iSlotIndex;
}

void CDnPGStorageTabDlg::SetRemoteItemID( int iItemID )
{
	m_iRemoteItemID = iItemID;

#ifdef PRE_ADD_SERVER_WAREHOUSE
	CDnPlayerStorageDlg* pPlayerStorageDlg = static_cast<CDnPlayerStorageDlg*>(GetStorageDlg(StorageUIDef::eTAB_PLAYER));
#else
	CDnPlayerStorageDlg* pPlayerStorageDlg = static_cast<CDnPlayerStorageDlg*>(GetStorageDlg( PLAYER_STORAGE_TAB ));
#endif
	if( pPlayerStorageDlg )
	{
		pPlayerStorageDlg->SetRemoteItemID( iItemID );
	}
}

#ifdef PRE_ADD_SERVER_WAREHOUSE
void CDnPGStorageTabDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_RADIOBUTTON_CHANGED)
	{
		if (bTriggeredByUser)
		{
			CDnInvenTabDlg* pInvenTabDlg = static_cast<CDnInvenTabDlg*>(GetInterface().GetMainMenuDialog(CDnMainMenuDlg::INVENTORY_DIALOG));
			if (pInvenTabDlg == NULL)
			{
				_ASSERT(0);
				return;
			}

			CEtUIRadioButton *pTabControl = static_cast<CEtUIRadioButton*>(pControl);
			switch (pTabControl->GetTabID())
			{
			case StorageUIDef::eTAB_PLAYER:
			case StorageUIDef::eTAB_GUILD:
			case StorageUIDef::eTAB_WORLDSERVER_NORMAL:
				{
					pInvenTabDlg->EnableInvenTab(CDnInvenTabDlg::eCOMMON);
					pInvenTabDlg->ShowTab(ST_INVENTORY);
				}
				break;

			case StorageUIDef::eTAB_WORLDSERVER_CASH:
				{
					pInvenTabDlg->EnableInvenTab(CDnInvenTabDlg::eCASH);
					pInvenTabDlg->ShowTab(ST_INVENTORY_CASH);
				}
				break;
			}
		}
	}

	CEtUITabDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}
#endif