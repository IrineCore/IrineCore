#include "StdAfx.h"
#include "DnGameProfileOptDlg.h"
#include "DnInterface.h"
#include "DnInterfaceString.h"
#include "DnGameOptionTask.h"
#include "GameOptionSendPacket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGameProfileOptDlg::CDnGameProfileOptDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pCheckMale(NULL)
, m_pCheckFemale(NULL)
, m_pEditBoxPR(NULL)
, m_pCheckOpen(NULL)
{
}

CDnGameProfileOptDlg::~CDnGameProfileOptDlg(void)
{
}

void CDnGameProfileOptDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ProfileOptDlg.ui" ).c_str(), bShow );

	if( CDnGameOptionTask::IsActive() )
	{
		GetGameOptionTask().SetProfileDialog( this );
	}
	else
	{
		CDebugSet::ToLogFile( "CDnGameProfileOptDlg::Initialize, game option task must be created before profile dialog is created." );
	}
}

void CDnGameProfileOptDlg::InitialUpdate()
{
#ifdef PRE_FIX_USA_PROFILE
#else
	m_pCheckMale = GetControl<CEtUICheckBox>("ID_CHECKBOX_MALE");
	m_pCheckFemale = GetControl<CEtUICheckBox>("ID_CHECKBOX_FEMALE");
#endif
	m_pEditBoxPR = GetControl<CEtUILineIMEEditBox>("ID_LINEIMEEDITBOX_PR");
	m_pCheckOpen = GetControl<CEtUICheckBox>("ID_CHECKBOX_OPEN");

	m_pEditBoxPR->SetMaxEditLength( PROFILEGREETINGLEN+1 );
}

void CDnGameProfileOptDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_OK") )
		{
			// In line edit box, newline characters seem excluded from SetMaxEditLength check, so up to 24 characters can be written even with newlines in between.
			// The issue is that replacing newlines with spaces in this string and taking 24 characters truncates a few trailing characters.
			// So to retrieve all written text, allocate a generous string buffer, read it, and then process it.
			wchar_t wszTemp[PROFILEGREETINGLEN+5] = {0, };
			m_pEditBoxPR->GetTextCopy( wszTemp, PROFILEGREETINGLEN+5 );

			// Strangely a newline arrives at the end. Force it to null terminator.
			int nLen = (int)wcslen( wszTemp );			
			wszTemp[nLen-1] = '\0';

			// Measure length again and replace embedded newline characters with spaces, then
			nLen = (int)wcslen( wszTemp );
			for( int i = 0; i < nLen; ++i )
			{
				if( wszTemp[i] == '\n' )
					wszTemp[i] = ' ';
			}

			// If length is within max, send as is; if over max, trim spaces as much as possible and pull forward.
			std::wstring wszPR;
			if( nLen > PROFILEGREETINGLEN )
			{
				wchar_t wszTemp2[PROFILEGREETINGLEN+1] = {0, };
				int nIndex = 0;
				for( int j = 0; j < nLen; ++j )
				{
					if( wszTemp[j] != ' ' )
					{
						wszTemp2[nIndex] = wszTemp[j];
						++nIndex;
						if( nIndex == PROFILEGREETINGLEN )
							break;
					}
				}
				wszPR = wszTemp2;
			}
			else
			{
				wszPR = wszTemp;
			}

			if( m_pCheckOpen->IsChecked() )
			{
				bool bEnable = true;
#ifdef PRE_FIX_USA_PROFILE
#else
				if( m_pCheckMale->IsChecked() == false && m_pCheckFemale->IsChecked() == false )
					bEnable = false;
#endif
				
				if( wszPR.empty() )
					bEnable = false;
				// Check if there is unsaved information.
				if( !bEnable )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8141 ) );
					return;
				}
			}

			if( DN_INTERFACE::UTIL::CheckChat( wszPR ) )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3772 ), MB_OK, 0, this );
				return;
			}

			// Set PR info and Send
			BYTE cGender = 0;
#ifdef PRE_FIX_USA_PROFILE
#else
			if( m_pCheckMale->IsChecked() ) cGender = 1;
			else if( m_pCheckFemale->IsChecked() ) cGender = 2;
#endif
			SendSetProfile( cGender, wszPR.c_str(), m_pCheckOpen->IsChecked() );

			// No need to receive a response. Assume there will be no problem saving.
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, NULL, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100060 ) );
			return;
		}
		else if( IsCmdControl("ID_CANCEL") )
		{
			Show(false);
			return;
		}
	}
	else if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
#ifdef PRE_FIX_USA_PROFILE
#else
		if( IsCmdControl("ID_CHECKBOX_MALE") )
		{
			m_pCheckFemale->SetChecked( !m_pCheckMale->IsChecked(), false );
		}
		else if( IsCmdControl("ID_CHECKBOX_FEMALE") )
		{
			m_pCheckMale->SetChecked( !m_pCheckFemale->IsChecked(), false );
		}
#endif
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnGameProfileOptDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) )
		{
			RequestFocus( m_pEditBoxPR );
		}
	}
}

void CDnGameProfileOptDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		// Initialize controls
#ifdef PRE_FIX_USA_PROFILE
#else
		m_pCheckMale->SetChecked( false, false );
		m_pCheckFemale->SetChecked( false, false );
#endif
		m_pEditBoxPR->ClearText();
		m_pCheckOpen->SetChecked( false, false );

		// Decided to request every time profile settings window opens, regardless of entering village.
		// Because once set, it will probably rarely be opened again.
		SendGetProfile();
	}

	CEtUIDialog::Show( bShow );
}

void CDnGameProfileOptDlg::SetProfile( TProfile *pProfile )
{
#ifdef PRE_FIX_USA_PROFILE
#else
	m_pCheckMale->SetChecked( pProfile->cGender == 1, false );
	m_pCheckFemale->SetChecked( pProfile->cGender == 2, false );
#endif
	m_pEditBoxPR->SetTextBuffer( pProfile->wszGreeting );
	m_pCheckOpen->SetChecked( pProfile->bOpenPublic );
}
