#pragma once

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnCustomDlg.h"
#include "DnCharStatusDlg.h"

class CDnCharTalismanInfoDlg;
class CDnCharTalismanListDlg;

class CDnCharTalismanDlg : public CDnCustomDlg
{
public:
	CDnCharTalismanDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharTalismanDlg();

protected:
	CDnCharTalismanInfoDlg *m_pCharTalismanInfoDlg;
	CDnCharTalismanListDlg *m_pCharTalismanListDlg;

	CEtUIButton *m_pBtnOK;
	CEtUIButton *m_pBtnInfo;
	CEtUIButton *m_pBtnCompose;
	CEtUIListBoxEx *m_pListBoxTalisman;
	CEtUIScrollBar *m_pScrollBar;
	CEtUIStatic *m_pTextOption;
	CEtUIButton *m_pBtnBuyShortcut;
	CEtUIStatic *m_pStaticBuyShortcut;

	int m_nSlotCount;
	int m_nScrollMax;
	int m_nEquipSoundIndex;

	void UpdateListPosition();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void Render( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	CEtUIListBoxEx *GetListBox() const { return m_pListBoxTalisman; }
	CEtUIScrollBar *GetScrollBar() const { return m_pScrollBar; }

	void EquipTalismanFromInven( int nInvenIndex, MIInventoryItem *pItem, CDnSlotButton *pSlotButton );
	MIInventoryItem *GetEquipTalisman( const CDnItem *pItem ) const;
	void SetSlotMagneticMode( bool bMagnetic );
	void SetTalismanItem( int nSlotIndex, CDnItem *pItem );
	void ResetTalismanSlot( int nSlotIndex );
	void RefreshTalisman();
	void RefreshOpenSlot();
	void RefreshCashSlot();
	CDnCharStatusDlg::eRetWearable GetWearableTalismanEquipType( const MIInventoryItem *pItem, int &nSlotIndex, std::wstring *strErrorMsg = NULL );
};

#endif // PRE_ADD_TALISMAN_SYSTEM
