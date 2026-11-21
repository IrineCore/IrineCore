#pragma once

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnCustomDlg.h"
#include "DnCharTalismanInfoDlg.h"
#include "DnCharTalismanListDlg.h"

class CDnInspectTalismanSlotDlg;
class CDnInspectTalismanInfoDlg;

class CDnInspectTalismanDlg : public CDnCustomDlg
{
public:
	CDnInspectTalismanDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnInspectTalismanDlg();

protected:
	CDnInspectTalismanSlotDlg *m_pInspectTalismanSlotDlg;
	CDnInspectTalismanInfoDlg *m_pInspectTalismanInfoDlg;

	CEtUIButton *m_pBtnOption;
	CEtUIListBoxEx *m_pListBoxTalisman;
	CEtUIScrollBar *m_pScrollBar;
	CEtUIStatic *m_pTextOption;

	int m_nSlotCount;
	int m_nScrollMax;
	DWORD m_dwSessionID;

	void UpdateListPosition();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Render( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	CEtUIListBoxEx *GetListBox() const { return m_pListBoxTalisman; }
	CEtUIScrollBar *GetScrollBar() const { return m_pScrollBar; }

	void RefreshTalisman( DWORD dwSessionID );
};

class CDnInspectTalismanSlotDlg : public CDnCharTalismanListDlg
{
public:
	CDnInspectTalismanSlotDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnInspectTalismanSlotDlg();

protected:
	CDnInspectTalismanDlg *m_pParentInspectDlg;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void RefreshInspectTalisman( CDnPlayerActor *pActor );
};

class CDnInspectTalismanInfoDlg : public CDnCharTalismanInfoDlg
{
public:
	CDnInspectTalismanInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnInspectTalismanInfoDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

#endif // PRE_ADD_TALISMAN_SYSTEM
