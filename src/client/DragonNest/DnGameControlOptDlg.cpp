#include "StdAfx.h"
#include "DnGameControlOptDlg.h"
#include "DnGameControlComboDlg.h"
#include "GameOption.h"
#include "DnGameControlKeyDlg.h"
#include "DnGameControlQuickSlotDlg.h"
#include "DnGameControlUIDlg.h"
#include "DnGameControlMouseDlg.h"
#include "DnGameControlAttackDlg.h"
#include "DnGameControlLooktDlg.h"
#include "DnGameControlSideOptDlg.h"
#include "DnActor.h"
#include "DnLocalPlayerActor.h"
#include "DnInterface.h"
#include "InputWrapper.h"
#include <ShellAPI.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGameControlOptDlg::CDnGameControlOptDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnOptionTabDlg( dialogType, pParentDialog, nID, pCallback )
, m_pTabButtonBaseKey(NULL)
, m_pTabButtonQuickSlot(NULL)
, m_pTabButtonUI(NULL)
, m_pTabButtonMouse(NULL)
, m_pTabButtonPad(NULL)
, m_pTabButtonAttack(NULL)
, m_pTabButtonLookt(NULL)
, m_pTabButtonSideOpt(NULL)
, m_pGameControlKeyDlg(NULL)
, m_pGameControlQuickSlotDlg(NULL)
, m_pGameControlUIDlg(NULL)
, m_pGameControlMouseDlg(NULL)
, m_pGameControlComboDlg(NULL)
, m_pGameControlAttackDlg(NULL)
, m_pGameControlLooktDlg(NULL)
, m_pGameControlSideOptDlg(NULL)
, m_iComboIndex(-1)
{
}

CDnGameControlOptDlg::~CDnGameControlOptDlg()
{
}

void CDnGameControlOptDlg::Initialize( bool bShow )
{
	BaseClass::Initialize( CEtResourceMng::GetInstance().GetFullName( "GameControlOptDlg.ui" ).c_str(), bShow );
}

void CDnGameControlOptDlg::InitialUpdate()
{
	m_pGameControlComboDlg = new CDnGameControlComboDlg(UI_TYPE_CHILD, this);
	m_pGameControlComboDlg->Initialize( false );
	m_pGameControlComboDlg->SetControlOptDlg(this);

	m_pTabButtonBaseKey = GetControl<CEtUIRadioButton>("ID_TAB_BASE");
	m_pGameControlKeyDlg = new CDnGameControlKeyDlg( UI_TYPE_CHILD, this );
	m_pGameControlKeyDlg->Initialize( false );
	m_pGameControlKeyDlg->GetComboDialog(m_pGameControlComboDlg);
	AddTabDialog( m_pTabButtonBaseKey, m_pGameControlKeyDlg );

	m_pTabButtonQuickSlot = GetControl<CEtUIRadioButton>("ID_TAB_KEY");
	m_pGameControlQuickSlotDlg = new CDnGameControlQuickSlotDlg( UI_TYPE_CHILD, this );
	m_pGameControlQuickSlotDlg->Initialize( false );
	m_pGameControlQuickSlotDlg->GetComboDialog(m_pGameControlComboDlg);
	AddTabDialog( m_pTabButtonQuickSlot, m_pGameControlQuickSlotDlg );

	m_pTabButtonUI = GetControl<CEtUIRadioButton>("ID_TAB_UI");
	m_pGameControlUIDlg = new CDnGameControlUIDlg( UI_TYPE_CHILD, this );
	m_pGameControlUIDlg->Initialize( false );
	m_pGameControlUIDlg->GetComboDialog(m_pGameControlComboDlg);
	AddTabDialog( m_pTabButtonUI, m_pGameControlUIDlg );

	m_pTabButtonMouse = GetControl<CEtUIRadioButton>("ID_TAB_MOUSE");
	m_pGameControlMouseDlg = new CDnGameControlMouseDlg( UI_TYPE_CHILD, this );
	m_pGameControlMouseDlg->Initialize( false );
	m_pGameControlMouseDlg->GetComboDialog(m_pGameControlComboDlg);
	AddTabDialog( m_pTabButtonMouse, m_pGameControlMouseDlg );

	m_pTabButtonAttack = GetControl<CEtUIRadioButton>("ID_TAB_ATTACK");
	m_pGameControlAttackDlg = new CDnGameControlAttackDlg( UI_TYPE_CHILD, this );
	m_pGameControlAttackDlg->Initialize( false );
	m_pGameControlAttackDlg->GetComboDialog(m_pGameControlComboDlg);
	AddTabDialog( m_pTabButtonAttack, m_pGameControlAttackDlg );

	m_pTabButtonLookt = GetControl<CEtUIRadioButton>("ID_TAB_LOOK");
	m_pGameControlLooktDlg = new CDnGameControlLooktDlg( UI_TYPE_CHILD, this );
	m_pGameControlLooktDlg->Initialize( false );
	m_pGameControlLooktDlg->GetComboDialog(m_pGameControlComboDlg);
	AddTabDialog( m_pTabButtonLookt, m_pGameControlLooktDlg );

	m_pTabButtonSideOpt = GetControl<CEtUIRadioButton>("ID_TAB_SIDEOPT");
	m_pGameControlSideOptDlg = new CDnGameControlSideOptDlg( UI_TYPE_CHILD, this );
	m_pGameControlSideOptDlg->Initialize( false );
	m_pGameControlSideOptDlg->GetComboDialog(m_pGameControlComboDlg);
	AddTabDialog( m_pTabButtonSideOpt, m_pGameControlSideOptDlg );

	m_pTabButtonAttack->Enable( false );
	m_pTabButtonLookt->Enable( false );

/*	m_pTabButtonPad = GetControl<CEtUIRadioButton>("ID_TAB_PAD");
	m_pTabButtonPad->Show( false );
	GetControl<CEtUIStatic>("ID_STATIC7")->Show( false );
*/
	SetCheckedTab( CONTROL( RadioButton, ID_TAB_BASE )->GetTabID() );
}

void CDnGameControlOptDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BUTTON_RESET" ) )
		{
			if( m_pTabButtonBaseKey->GetTabID() == GetCurrentTabID() && m_pGameControlKeyDlg )
				m_pGameControlKeyDlg->ResetDefault();
			else if( m_pTabButtonQuickSlot->GetTabID() == GetCurrentTabID() && m_pGameControlQuickSlotDlg )
				m_pGameControlQuickSlotDlg->ResetDefault();
			else if( m_pTabButtonUI->GetTabID() == GetCurrentTabID() && m_pGameControlUIDlg )
				m_pGameControlUIDlg->ResetDefault();
			else if( m_pTabButtonMouse->GetTabID() == GetCurrentTabID() && m_pGameControlMouseDlg )
				m_pGameControlMouseDlg->ResetDefault();
			else if( m_pTabButtonAttack->GetTabID() == GetCurrentTabID() && m_pGameControlAttackDlg )
				m_pGameControlAttackDlg->ResetDefault();
			else if( m_pTabButtonLookt->GetTabID() == GetCurrentTabID() && m_pGameControlLooktDlg )
				m_pGameControlLooktDlg->ResetDefault();
			else if( m_pTabButtonSideOpt->GetTabID() == GetCurrentTabID() && m_pGameControlSideOptDlg )
				m_pGameControlSideOptDlg->ResetDefault();
		}

		if( IsCmdControl( "ID_BUTTON_APPLY" ) || IsCmdControl( "ID_BUTTON_CANCEL" ) )
		{
			if( m_pTabButtonBaseKey->GetTabID() == GetCurrentTabID() && m_pGameControlKeyDlg && m_pGameControlKeyDlg->IsInputMode() )
				m_pGameControlKeyDlg->SetInputMode( false );
			else if( m_pTabButtonQuickSlot->GetTabID() == GetCurrentTabID() && m_pGameControlQuickSlotDlg && m_pGameControlQuickSlotDlg->IsInputMode() )
				m_pGameControlQuickSlotDlg->SetInputMode( false );
			else if( m_pTabButtonUI->GetTabID() == GetCurrentTabID() && m_pGameControlUIDlg && m_pGameControlUIDlg->IsInputMode() )
				m_pGameControlUIDlg->SetInputMode( false );
			else if( m_pTabButtonAttack->GetTabID() == GetCurrentTabID() && m_pGameControlAttackDlg && m_pGameControlAttackDlg->IsInputMode() )
				m_pGameControlAttackDlg->SetInputMode( false );
			else if( m_pTabButtonLookt->GetTabID() == GetCurrentTabID() && m_pGameControlLooktDlg && m_pGameControlLooktDlg->IsInputMode() )
				m_pGameControlLooktDlg->SetInputMode( false );
			else if( m_pTabButtonSideOpt->GetTabID() == GetCurrentTabID() && m_pGameControlSideOptDlg && m_pGameControlSideOptDlg->IsInputMode() )
				m_pGameControlSideOptDlg->SetInputMode( false );
		}
	}
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( IsCmdControl( "ID_TAB_PAD" ) )
		{
			ConfigPad();
		}
	}

	BaseClass::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
void CDnGameControlOptDlg::ProcessCombo(const int index)
{
	CGameOption::GetInstance().ApplyInputDevice();

	if( CDnGameControlComboDlg::KEYBOARD_MOUSE == index )
	{
		m_pTabButtonAttack->Enable( false );
		m_pTabButtonLookt->Enable( false );
	}
	else
	{
		m_pTabButtonAttack->Enable( true );
		m_pTabButtonLookt->Enable( true );
	}

	DWORD tabIndex = GetCurrentTabID();
	GetOptionBase(tabIndex)->ProcessCombo(index);

	return;
}

void CDnGameControlOptDlg::ConfigPad()
{
	// Considered using EnumWindow, but that seems unnecessary, so do a simple check.
	// Gamepad program caption and class name verified with Spy++.
	HWND hWndPad = FindWindow( L"{5B1989CD-1D2B-48c8-8DB4-0AA8E7A0041C}", L"Default" );
	if( hWndPad )
	{
		SetForegroundWindow( hWndPad );
		return;
	}

	long lRet;
	HKEY hKey;
	DWORD dwData;
	DWORD dwType = REG_SZ;
	WCHAR wszPath[MAX_PATH] = {0,};
	WCHAR wszName[MAX_PATH] = {0,};
	lRet = ::RegOpenKey(HKEY_LOCAL_MACHINE, _T("Software\\NHN Japan\\HGGAMEPAD"), &hKey);
	if (lRet == ERROR_SUCCESS)
	{
		dwData = _countof(wszPath);
		::RegQueryValueExW(hKey, L"Folder", NULL, &dwType, (unsigned char*)wszPath, &dwData);
		dwData = _countof(wszName);
		::RegQueryValueExW(hKey, L"Editor", NULL, &dwType, (unsigned char*)wszName, &dwData);
		::RegCloseKey(hKey);
	}
	else
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3191 ), MB_OK );
		return;
	}

	wcscat_s( wszPath, _countof(wszPath), L"\\" );
	wcscat_s( wszPath, _countof(wszPath), wszName );
	if (GetFileAttributesW(wszPath) == INVALID_FILE_ATTRIBUTES)
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3192 ), MB_OK );
		return;
	}

	HINSTANCE hInst = ShellExecuteW( NULL, L"open", wszPath, L"j_dnest", NULL, SW_SHOWNORMAL );
	if( hInst <= (HINSTANCE)SE_ERR_NOASSOC )	// According to MSDN, less than 32 is an error.
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3193 ), MB_OK );
		return;
	}
}

void CDnGameControlOptDlg::Process( float fElapsedTime )
{
	CDnOptionTabDlg::Process( fElapsedTime );

	m_pGameControlComboDlg->LoadDialogTexture();
}