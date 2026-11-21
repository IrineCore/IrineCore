#pragma once

#include "DnCustomDlg.h"

class CDnNPCHotKeyInfoDlg : public CDnCustomDlg
{
public:
	enum {
		MAX_NPC_HOTKEY = 4,
	};

public:
	CDnNPCHotKeyInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnNPCHotKeyInfoDlg();

protected:
	CEtUIStatic *m_pBack[MAX_NPC_HOTKEY];
	CEtUIButton *m_pButton[MAX_NPC_HOTKEY];
	CEtUIStatic *m_pNpcName[MAX_NPC_HOTKEY];
	CEtUIStatic *m_pStatic[MAX_NPC_HOTKEY];
	CEtUIStatic *m_pText[MAX_NPC_HOTKEY];
	UINT m_nNpcUIDs[MAX_NPC_HOTKEY];

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void ClearNpcHotKeys();
	void SetNpcHotKey( int nIndex, UINT nNpcUID, const WCHAR *wszNpcName, const WCHAR *wszKeyText );
};
