#include "StdAfx.h"
#include "DnServerListDlg.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnInterface.h"
#include "DnInterfaceDlgID.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnServerListDlg::CDnServerListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pServerList(NULL)
	, m_pButtonOK(NULL)
	, m_pButtonBack(NULL)
	, m_pButtonExit(NULL)
{
}

CDnServerListDlg::~CDnServerListDlg(void)
{
}

void CDnServerListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ServerListDlg.ui" ).c_str(), bShow );
}

void CDnServerListDlg::InitialUpdate()
{
	m_pServerList = GetControl<CEtUIListBox>("ID_LISTBOX_SERVER");
	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonBack = GetControl<CEtUIButton>("ID_BACK");	
	m_pButtonExit = GetControl<CEtUIButton>("ID_EXIT");	

	// In the domestic live build, login is done via web, so the Exit button is needed.
	// For China, handle same as RDebug; add Japan once decided.
	m_pButtonExit->Show( false );
}

void CDnServerListDlg::ClearServerList()
{
	m_pServerList->RemoveAllItems();
#ifndef PRE_MOD_SELECT_CHAR
	m_mapServerList.clear();
#endif // PRE_MOD_SELECT_CHAR
}

void CDnServerListDlg::AddServerList( int nIndex, LPCWSTR szServerName, float fUserRatio, int nCharCount )
{
	int nStringID[] = {100042, 100041, 100040, 100039};
	wchar_t *wszColor[] = { L"@RGB(153,204,0)",  L"@RGB(102,204,255)", L"@RGB(255,204,0)", L"@RGB(204,51,0)"};

	int nCapacity = -1;
	if( fUserRatio < 0.05f ) nCapacity = 0;
	else if( fUserRatio < 0.2f ) nCapacity = 1;
	else if( fUserRatio < 0.9f ) nCapacity = 2;
	else nCapacity = 3;

	wchar_t szCharCount[16]={0,};
	if( nCharCount > 0 )
		swprintf_s( szCharCount, _countof(szCharCount), L"(%d)", nCharCount );

	wchar_t szServerFullName[255]={0,};
	int nSpaceCount = 17;
	while( nSpaceCount-- ) {
		wcscat_s(szServerFullName, L" ");
	}	
	wcscat_s(szServerFullName, szServerName);
	wcscat_s(szServerFullName, szCharCount);
	wcscat_s(szServerFullName, L"@MOVE(0.20f)[ ");
	wcscat_s(szServerFullName, wszColor[ nCapacity ]);
	wcscat_s(szServerFullName, GetEtUIXML().GetUIString( CEtUIXML::idCategory1,  nStringID[ nCapacity ] ) );
	wcscat_s(szServerFullName, L"@/RGB ]");

	m_pServerList->AddItem( szServerFullName, NULL, nIndex );
#ifndef PRE_MOD_SELECT_CHAR
	m_mapServerList.insert( pair<std::wstring,int>( std::wstring(szServerName), nIndex ) );
#endif // PRE_MOD_SELECT_CHAR
}

void CDnServerListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BACK" ) ) 
		{
			SendBackButtonLogin();	// Only this one needs to be sent separately
			EnableControl( false );
			//CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
			//if( pTask )
			//{
			//	SendBackButton();
			//	pTask->ChangeState( CDnLoginTask::RequestIDPass );
			//}
			return;
		}

		if( IsCmdControl("ID_OK" ) )
		{
			SelectServer();
			EnableControl( false );
			return;
		}

		if( IsCmdControl("ID_EXIT" ) )
		{
			GetInterface().MessageBox( MESSAGEBOX_21, MB_YESNO, MESSAGEBOX_21, CDnInterface::GetInstancePtr() );
			return;
		}
	}
	else if( nCommand == EVENT_LISTBOX_ITEM_DBLCLK )
	{
		if( IsCmdControl("ID_LISTBOX_SERVER" ) )
		{
			SelectServer();
			EnableControl( false );
			return;
		}
	}
}

void CDnServerListDlg::ShowBackButton( bool bShow )
{
	if( bShow )
	{
		m_pButtonBack->Show( true );
		m_pButtonExit->Show( false );
	}
	else
	{
		m_pButtonBack->Show( false );
		m_pButtonExit->Show( true );
	}
}

void CDnServerListDlg::Show( bool bShow ) 
{ 
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
#ifdef PRE_ADD_GAMEQUIT_REWARD
		CTask * pTask = CTaskManager::GetInstance().GetTask( "BridgeTask" );
		if( pTask )
		{
			CDnBridgeTask * pBridgeTask = (CDnBridgeTask *)pTask;
			pBridgeTask->EndNewbieRewardMessage();
		}
#endif // PRE_ADD_GAMEQUIT_REWARD

		m_pButtonOK->Enable( true );
		m_pButtonBack->Enable( true );
		m_pServerList->Enable( true );
		RequestFocus( m_pServerList );
	}
	else
	{
//		ClearServerList();
//		m_pServerList->RemoveAllItems();
	}

	CEtUIDialog::Show( bShow );
}
#ifndef PRE_MOD_SELECT_CHAR
std::wstring CDnServerListDlg::GetServerName( int nIndex )
{
	std::map<std::wstring,int>::iterator iter = m_mapServerList.begin();
	while( iter != m_mapServerList.end() )
	{
		if(iter->second == nIndex ) {
			return iter->first;
		}		
		iter++;
	}
	return std::wstring(L"");
}

int CDnServerListDlg::GetServerIndex( LPCWSTR szServerName )
{
	std::map<std::wstring,int>::iterator iter = m_mapServerList.find( szServerName );
	if( iter != m_mapServerList.end() )
	{
		return iter->second;
	}

	std::string strServerName;
	ToMultiString( std::wstring(szServerName), strServerName );
	CDebugSet::ToLogFile( "CDnServerListDlg::GetServerIndex, %s not found!", strServerName.c_str() );
	return -1;
}
#endif // PRE_MOD_SELECT_CHAR

void CDnServerListDlg::SetDefaultList()
{
	if( !m_pServerList ) return;
	m_pServerList->SelectItem(0);
}

void CDnServerListDlg::SelectServer()
{
	CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( !pTask ) return;

	int nIndex;
	if( !m_pServerList->GetSelectedValue( nIndex ) )
	{
		GetInterface().MessageBox( MESSAGEBOX_33, MB_OK );
		return;
	}

	pTask->SelectServer( nIndex );
}

void CDnServerListDlg::EnableControl( bool bEnable )
{
	m_pButtonOK->Enable( bEnable );
	m_pButtonBack->Enable( bEnable );
	m_pServerList->Enable( bEnable );
}