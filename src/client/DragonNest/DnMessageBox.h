#pragma once
#include "EtUIDialog.h"
#include "DnInterface.h"
#include "DnSmartMoveCursor.h"

class CDnMessageBox : public CEtUIDialog
{
public:
	CDnMessageBox( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnMessageBox(void);

protected:
	CDnInterface::InterfaceTypeEnum m_emInterfaceType;

	CEtUIButton *m_pButtonOK;
	CEtUIButton *m_pButtonCancel;
	CEtUIButton *m_pButtonRetry;
	CEtUIButton *m_pButtonYes;
	CEtUIButton *m_pButtonNo;
	CEtUIButton *m_pButtonAuthPw;
	CEtUIButton *m_pButtonWaterBottle;
	CEtUIButton *m_pButtonWater;

	CEtUIStatic *m_pStaticMessage;
	CEtUIStatic *m_pStaticTitle;

	SUICoord m_ButtonCoord[3];
	std::vector<CEtUIButton*> m_vecButtonControl;

	std::wstring m_strTitle;
	DWORD m_dwTitleColor;

	// Smart move
	CDnSmartMoveCursor m_SmartMove;

	UINT	m_BtnType;
	bool m_bTopMost;
	bool m_bSmartMoveNoButton;	// Want SmartMove to go to No.
	bool m_bIgnoreEnterHotkey;

	// By default, calling SetMessageBox overwrites existing contents.
	// However, if you want the message box to stay until dismissed by user input, set this to true.
	bool m_bNoOverwrite;

protected:
	void ShowControl( bool bShow );

public:
	void SetTitle( LPCWSTR wszTitle, DWORD dwColor = EtInterface::textcolor::WHITE );
	bool IsTopMost() { return m_bTopMost; }

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void SetMessageBox( CDnInterface::InterfaceTypeEnum emType, LPCWSTR pwszMessage, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = false, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
};


//////////////////////////////////////////////////////////////////////////
// MessageBox using TextBox

class CDnMessageBoxTextBox : public CDnMessageBox , public CEtUINameLinkInterface
{
public:
	CDnMessageBoxTextBox( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnMessageBoxTextBox();

protected:
	CEtUITextBox*	m_pTextMessage;

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void SetMessageBox( CDnInterface::InterfaceTypeEnum emType, LPCWSTR pwszMessage, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = false, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
	virtual void OnOpenHyperLink( const std::wstring& argString );

public:
	void AddMessage( LPCWSTR pwszMessage ) { if( m_pTextMessage ) m_pTextMessage->AddText( pwszMessage ); }

//---- PRE_ADD_JELLYCOUNT
	void AddMessage( LPCWSTR pwszMessage, const D3DCOLOR TextColor, DWORD dwFormat = UITEXT_NONE, const D3DCOLOR BgColor = 0 ){ 
		if( m_pTextMessage ) 
			m_pTextMessage->AddText( pwszMessage, TextColor, dwFormat, BgColor ); 
	}

	void ClearText(){
		m_pTextMessage->ClearText();
	}
// -----//

};
