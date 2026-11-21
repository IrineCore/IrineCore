#pragma once
#include "DnCustomDlg.h"
#include "DnSmartMoveCursor.h"

class CDnItem;
class CDnItemSlotButton;

class CDnItemCompoundMixDlg : public CDnCustomDlg
{
public:
	CDnItemCompoundMixDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemCompoundMixDlg(void);

	enum
	{
		NUM_NEED_ITEM = 5,
	};

protected:
	CEtUIStatic *m_pGold;
	CEtUIStatic *m_pSilver;
	CEtUIStatic *m_pBronze;
	DWORD m_dwColorGold;
	DWORD m_dwColorSilver;
	DWORD m_dwColorBronze;

	CEtUIButton *m_pButtonOK;
	CEtUIButton *m_pButtonCancel;

	CEtUIStatic *m_pProb;

	// Material item slot
	CDnItem *m_pItem[NUM_NEED_ITEM];
	CDnItemSlotButton *m_pItemSlotButton[NUM_NEED_ITEM];

	// Set slot
	void SetItemSlot( int nSlotIndex, int nItemID, int nNeedItemCount, int nCurItemCount );

	// Smart move
	CDnSmartMoveCursor m_SmartMove;

protected:

public:
	// Set probability
	void SetProb( int nProb );

	// Set price
	void SetPrice( int nPrice );

	// Set material list
	void SetCompoundInfo( int *pItemID, int *pItemCount );

	// Set OK button
	void SetOK( bool bOK );

	void EnableButton( bool bEnable );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
};