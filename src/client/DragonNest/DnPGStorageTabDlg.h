#pragma once

#include "EtUITabDialog.h"
#ifdef PRE_ADD_SERVER_WAREHOUSE
#include "DnUIDefine.h"
#endif

class MIInventoryItem;
class CDnInventoryDlg;
class CDnPlayerStorageDlg;
class CDnGuildStorageDlgNew;

class CDnPGStorageTabDlg : public CEtUITabDialog
{
public:
#ifdef PRE_ADD_SERVER_WAREHOUSE
#else
	enum
	{
		// TAB ID saved in UI file starts from 1;; 
		PLAYER_STORAGE_TAB = 1,
		GUILD_STORAGE_TAB,
	};
#endif

	CDnPGStorageTabDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPGStorageTabDlg(void);

protected:
	//int m_iSelectedTabIndex;
	std::vector<CEtUIRadioButton*> m_vecTabButton;

	int m_iRemoteItemID;
	
	// Received packet to open guild storage since player has a guild.
	bool m_bOpenGuildStorage;

	CEtUIDialog* GetSelectedStorageDlg( void );

public:
	// Personal and guild storage are unified, so select externally first before processing.
	//void SelectTab( int iSelectedTabIndex );
	
	void SetItem( MIInventoryItem *pItem );
	void ResetSlot( MIInventoryItem* pItem );

	void SetUseItemCnt( DWORD dwItemCnt );

	bool IsEmptySlot();
	int GetEmptySlot();

	CEtUIDialog* GetStorageDlg( int iTabID );

	void SetRemoteItemID( int iItemID );
	void SetOpenGuildStorage( bool bOpenGuildStorage ) { m_bOpenGuildStorage = bOpenGuildStorage; };

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
#ifdef PRE_ADD_SERVER_WAREHOUSE
	virtual void ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */);
#endif

private:
#ifdef PRE_ADD_SERVER_WAREHOUSE
	#ifdef PRE_MOD_SERVER_WAREHOUSE_NO_REMOTE
	void MakeEnableStorageTab();
	#else
	void MakeEnableStorageTab(StorageUIDef::eStorageTabType type);
	#endif
#endif
};
