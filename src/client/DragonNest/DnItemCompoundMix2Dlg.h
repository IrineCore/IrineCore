#pragma once
#include "DnCustomDlg.h"
#include "DnSmartMoveCursor.h"

class CDnItem;
class CDnItemSlotButton;

class CDnItemCompoundMix2Dlg : public CDnCustomDlg
{
public:
	CDnItemCompoundMix2Dlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemCompoundMix2Dlg(void);

	enum
	{
		NUM_NEED_ITEM = 5,
	};

protected:
	CEtUIButton *m_pButtonOK;
	CEtUIButton *m_pButtonCancel;

	CEtUIStatic *m_pProb;
	CEtUIStatic *m_pDescription;
	CEtUIStatic *m_pName;
	CEtUIStatic *m_pStaticSelect;

	// Material item slot
	CDnItem *m_pItem[NUM_NEED_ITEM];
	CDnItemSlotButton *m_pItemSlotButton[NUM_NEED_ITEM];

	// Set slot
	void SetItemSlot( int nSlotIndex, CDnItem *pItem, char nOptionIndex = -1 );

	// Smart move
	CDnSmartMoveCursor m_SmartMove;

	int m_nSelectSlotIndex;

protected:
	void RefreshSelectItemSlot();

public:
	// Set probability
	void SetProb( int nProb );

	// Set material list
	void SetCompoundInfo( std::vector<CDnItem *> &pVecList, char nOptionIndex = -1 );
	void SetDescription( const WCHAR *wszStr );

	// Set OK button
	void SetOK( bool bOK );

	void EnableButton( bool bEnable );
	INT64 GetSelectItemSerialID();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};