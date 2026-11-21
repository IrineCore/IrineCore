#include "StdAfx.h"
#include "DnCharCreateSelectDlg.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnTableDB.h"
#include "EtUIRadioButton.h"
#include "EtUIStatic.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#ifdef PRE_MOD_SELECT_CHAR

// Modify whenever a new character is added!

CDnCharCreateSelectDlg::CDnCharCreateSelectDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_nFocusCharIndex( -1 )
{
	memset( m_pStaticJobName, NULL, sizeof(CEtUIStatic*) * CLASSKINDMAX );
	memset( m_pRadioButton, NULL, sizeof(CEtUIRadioButton*) * CLASSKINDMAX );

}

CDnCharCreateSelectDlg::~CDnCharCreateSelectDlg(void)
{
	m_mapTabJobIndex.clear();
}

void CDnCharCreateSelectDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharCreate_CharSelectDlg.ui" ).c_str(), bShow );
}

void CDnCharCreateSelectDlg::Show( bool bShow )
{
	if( bShow == m_bShow )
		return;

	CEtUIDialog::Show(bShow);
}

void CDnCharCreateSelectDlg::InitialUpdate()
{
	//
	/*float LastPos = 0.0f;
	CEtUIRadioButton*	m_pClassButton;
	for (std::vector<int>::iterator it = vecEnabledClass.begin(); it != vecEnabledClass.end(); ++it)
	{
		m_pClassButton = GetControl<CEtUIRadioButton>(FormatA("ID_RBT_%d", *it-1).c_str());
		SUICoord SUICoordtest;
		SUICoordtest = m_pClassButton->GetUICoord();

		m_pClassButton->SetPosition(SUICoordtest.fX, LastPos);
		LastPos += 0.07f;//+0.07 ?
		
		m_pClassButton->Show(true);

	
		OutputDebug("[%s] Valid Class :: %d  x=%.5f ; y=%.5f", __FUNCTION__, *it,SUICoordtest.fX,SUICoordtest.fY);
	//	m_pClassButton->SetPosition(10.f, LastPos + 50.f);
	}
	*/


	


	/*for (i = 0; i < 12; i++)
	{
		CEtUIRadioButton*	m_pClassButton;
		m_pClassButton = GetControl<CEtUIRadioButton>(FormatA("ID_RBT_%d", i).c_str());
		m_pClassButton->
		
	} */



	m_pRadioButton[0] = GetControl<CEtUIRadioButton>( "ID_RBT_WARRIOR" );
	m_pRadioButton[1] = GetControl<CEtUIRadioButton>( "ID_RBT_ARCHER" );
	m_pRadioButton[2] = GetControl<CEtUIRadioButton>( "ID_RBT_SOCERESS" );
	m_pRadioButton[3] = GetControl<CEtUIRadioButton>( "ID_RBT_CLERIC" );
	m_pRadioButton[4] = GetControl<CEtUIRadioButton>( "ID_RBT_ACADEMIC" );
	m_pRadioButton[5] = GetControl<CEtUIRadioButton>( "ID_RBT_KALI" );

	char szControlName[32];
	for( int i=0; i<CLASSKINDMAX; ++i )
	{
		sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_CLASS%d", i );
		m_pStaticJobName[i] = GetControl<CEtUIStatic>( szControlName );
		if( m_pStaticJobName[i] )
			m_pStaticJobName[i]->Show( false );
	}

	m_mapTabJobIndex.insert( make_pair( 5, 0 ) );
	m_mapTabJobIndex.insert( make_pair( 2, 1 ) );
	m_mapTabJobIndex.insert( make_pair( 0, 2 ) );
	m_mapTabJobIndex.insert( make_pair( 1, 3 ) );
	m_mapTabJobIndex.insert( make_pair( 3, 4 ) );
	m_mapTabJobIndex.insert( make_pair( 4, 5 ) );
}

void CDnCharCreateSelectDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	CDnLoginTask *pLoginTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( !pLoginTask ) return;

	if( EVENT_RADIOBUTTON_CHANGED == nCommand )
	{
		if( strstr( pControl->GetControlName(), "ID_RBT" ) )
		{
			int nSelectIndex = static_cast<CEtUIRadioButton*>( pControl )->GetTabID();
pLoginTask->SelectMoveCharacter( nSelectIndex );
		}
	}
}

void CDnCharCreateSelectDlg::SetCharCreateSelect( int nClassIndex )
{
	if( nClassIndex < 0 || nClassIndex >= CLASSKINDMAX )
		return;

	if( m_pRadioButton[nClassIndex] )
		m_pRadioButton[nClassIndex]->SetChecked( true );
}

void CDnCharCreateSelectDlg::ResetButton()
{
	for( int i=0; i<CLASSKINDMAX; ++i )
	{
		if (m_pRadioButton[i])
			m_pRadioButton[i]->SetChecked( false );
	}
}

bool CDnCharCreateSelectDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;
	if( uMsg == WM_MOUSEMOVE )
	{
		POINT MousePoint;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );

		float fMouseX, fMouseY;
		PointToFloat( MousePoint, fMouseX, fMouseY );

		int nFocusIndex = -1;
		for( int i=0; i<CLASSKINDMAX; ++i )
		{
			if( m_pRadioButton[i] && m_pRadioButton[i]->IsChecked() )
			{
				nFocusIndex = i;
			}
			if( m_pRadioButton[i] && m_pRadioButton[i]->IsInside( fMouseX, fMouseY ) )
			{
				nFocusIndex = i;
				break;
			}
		}

		if( m_nFocusCharIndex > -1 )
			nFocusIndex = m_nFocusCharIndex;

		for( int i=0; i<CLASSKINDMAX; ++i )
		{
			if( m_pStaticJobName[i] )
				m_pStaticJobName[i]->Show( false );
		}

		if( nFocusIndex > -1 )
		{
			std::map<int, int>::iterator iter = m_mapTabJobIndex.find( nFocusIndex );
			if( iter != m_mapTabJobIndex.end() )
			{
				int nSelectIndex = iter->second;

				if( nSelectIndex >= 0 && nSelectIndex < CLASSKINDMAX && m_pStaticJobName[nSelectIndex] )
					m_pStaticJobName[nSelectIndex]->Show( true );
			}
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}


#endif // PRE_MOD_SELECT_CHAR