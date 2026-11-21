#include "StdAfx.h"
#include "DnGameVoiceChatOptDlg.h"
#include "GameOption.h"
#ifdef _USE_VOICECHAT
#include "VoiceChatClient.h"
#include "DNVoiceChatTask.h"
#include "DnInterface.h"
#include "DnGaugeDlg.h"
#include "DnItemTask.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef _USE_VOICECHAT
CDnGameVoiceChatOptDlg::CDnGameVoiceChatOptDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnOptionDlg( dialogType, pParentDialog, nID, pCallback )
, m_pVoiceChat(NULL)
, m_pButtonDown(NULL)
, m_pButtonUp(NULL)
, m_pSliderVoiceChat(NULL)
, m_pStaticVoiceChat(NULL)
, m_pButtonDown2(NULL)
, m_pButtonUp2(NULL)
, m_pSliderMic(NULL)
, m_pStaticMic(NULL)
, m_pButtonTest(NULL)
, m_pStaticTest(NULL)
, m_fTestTime(0.0f)
, m_pMicBoost(NULL)
, m_pStaticMicBoost(NULL)
, m_pCheckBoxAutoVolume(NULL)
, m_pButtonDown3(NULL)
, m_pButtonUp3(NULL)
, m_pSliderVolume(NULL)
, m_pStaticVolume(NULL)
, m_pComboCaptureDevice(NULL)
, m_pButtonPremium(NULL)
{
}

CDnGameVoiceChatOptDlg::~CDnGameVoiceChatOptDlg(void)
{
}

void CDnGameVoiceChatOptDlg::Initialize( bool bShow )
{
	CDnOptionDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "GameVoiceChatOptDlg.ui" ).c_str(), bShow );

	if( CDnVoiceChatTask::IsActive() )
	{
		GetVoiceChatTask().SetVoiceChatOptDlg( this );
	}
	else
	{
		CDebugSet::ToLogFile( "CDnGameVoiceChatOptDlg::Initialize, voice chat task must be created before voice chat option dialog is created." );
	}
}

void CDnGameVoiceChatOptDlg::InitialUpdate()
{
	m_pVoiceChat = GetControl<CEtUICheckBox>("ID_CHECKBOX_VOICECHAT");

	m_pButtonDown = GetControl<CEtUIButton>("ID_BUTTON_DOWN");
	m_pButtonUp = GetControl<CEtUIButton>("ID_BUTTON_UP");
	m_pSliderVoiceChat = GetControl<CEtUISlider>("ID_SLIDER_VOICECHAT");
	m_pStaticVoiceChat = GetControl<CEtUIStatic>("ID_STATIC_VOICECHAT");

	m_pButtonDown2 = GetControl<CEtUIButton>("ID_BUTTON_DOWN2");
	m_pButtonUp2 = GetControl<CEtUIButton>("ID_BUTTON_UP2");
	m_pSliderMic = GetControl<CEtUISlider>("ID_SLIDER_MIC");
	m_pStaticMic = GetControl<CEtUIStatic>("ID_STATIC_MIC");

	char szName[32];
	CEtUIStatic *pStaticTest;
	for( int i = 0; i < MAX_NUM_TEST; ++i ) {
		sprintf_s( szName, _countof(szName), "ID_STATIC_TEST%d", i+1 );
		pStaticTest = GetControl<CEtUIStatic>(szName);
		if( pStaticTest ) m_vecStaticTest.push_back(pStaticTest);
	}

	for( int i = 0; i < (int)m_vecStaticTest.size(); ++i ) {
		m_vecStaticTest[i]->Show( false );
	}

	m_pButtonTest = GetControl<CEtUIButton>("ID_BTN_MICTEST");
	m_pStaticTest = GetControl<CEtUIStatic>("ID_STATIC_MICTEST");
	m_pStaticTest->SetText( L"" );

	m_pMicBoost = GetControl<CEtUICheckBox>("ID_CHECK_MICBOOST");
	m_pStaticMicBoost = GetControl<CEtUIStatic>("ID_STATIC4");

	m_pCheckBoxAutoVolume = GetControl<CEtUICheckBox>("ID_CHECKBOX_AUTOVOLUME");
	m_pButtonDown3 = GetControl<CEtUIButton>("ID_BUTTON_DOWN3");
	m_pButtonUp3 = GetControl<CEtUIButton>("ID_BUTTON_UP3");
	m_pSliderVolume = GetControl<CEtUISlider>("ID_SLIDER_VOLUME");
	m_pStaticVolume = GetControl<CEtUIStatic>("ID_STATIC_VOLUME");

	m_pComboCaptureDevice = GetControl<CEtUIComboBox>("ID_COMBOBOX_MIC");

	m_pButtonPremium = GetControl<CEtUIButton>("ID_BUTTON_PREMIUMCHAT");
}

void CDnGameVoiceChatOptDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_UP") )
		{
			int nValue = m_pSliderVoiceChat->GetValue();
			if( (++nValue) > 100 ) nValue = 100;

			m_pSliderVoiceChat->SetValue( nValue );
			m_pStaticVoiceChat->SetIntToText( nValue );
			if( CVoiceChatClient::IsActive() ) CVoiceChatClient::GetInstance().SetVoiceChatVolume( nValue/100.0f );
			return;
		}
		else if( IsCmdControl("ID_BUTTON_DOWN") )
		{
			int nValue = m_pSliderVoiceChat->GetValue();
			if( (--nValue) < 0 ) nValue = 0;

			m_pSliderVoiceChat->SetValue( nValue );
			m_pStaticVoiceChat->SetIntToText( nValue );
			if( CVoiceChatClient::IsActive() ) CVoiceChatClient::GetInstance().SetVoiceChatVolume( nValue/100.0f );
			return;
		}
		else if( IsCmdControl("ID_BUTTON_UP2") )
		{
			int nValue = m_pSliderMic->GetValue();
			if( (++nValue) > 100 ) nValue = 100;

			m_pSliderMic->SetValue( nValue );
			m_pSliderMic->SetIntToText( nValue );
			if( CVoiceChatClient::IsActive() ) CVoiceChatClient::GetInstance().SetMicVolume( nValue/100.0f );
			return;
		}
		else if( IsCmdControl("ID_BUTTON_DOWN2") )
		{
			int nValue = m_pSliderMic->GetValue();
			if( (--nValue) < 0 ) nValue = 0;

			m_pSliderMic->SetValue( nValue );
			m_pSliderMic->SetIntToText( nValue );
			if( CVoiceChatClient::IsActive() ) CVoiceChatClient::GetInstance().SetMicVolume( nValue/100.0f );
			return;
		}
		else if( IsCmdControl("ID_BTN_MICTEST") )
		{
			m_fTestTime = 10.0f;
			m_pButtonTest->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3183 ) );
			m_pButtonTest->Enable( false );

			WCHAR wszText[256];
			swprintf_s( wszText, _countof(wszText), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3184 ), (int)m_fTestTime );
			m_pStaticTest->SetText( wszText );
		}
		else if( IsCmdControl("ID_BUTTON_UP3") )
		{
			int nValue = m_pSliderVolume->GetValue();
			if( (++nValue) > 100 ) nValue = 100;

			m_pSliderVolume->SetValue( nValue );
			m_pStaticVolume->SetIntToText( nValue );
			if( CDnVoiceChatTask::IsActive() ) GetVoiceChatTask().SetAutoAdjustVolume( m_pCheckBoxAutoVolume->IsChecked(), nValue/100.0f );
			return;
		}
		else if( IsCmdControl("ID_BUTTON_DOWN3") )
		{
			int nValue = m_pSliderVolume->GetValue();
			if( (--nValue) < 0 ) nValue = 0;

			m_pSliderVolume->SetValue( nValue );
			m_pStaticVolume->SetIntToText( nValue );
			if( CDnVoiceChatTask::IsActive() ) GetVoiceChatTask().SetAutoAdjustVolume( m_pCheckBoxAutoVolume->IsChecked(), nValue/100.0f );
			return;
		}
		else if( IsCmdControl("ID_BUTTON_PREMIUMCHAT") )
		{
			GetInterface().ShowVoiceChatPremiumOptDialog( true );
		}
	}
	else if( nCommand == EVENT_SLIDER_VALUE_CHANGED )
	{
		if( IsCmdControl( "ID_SLIDER_VOICECHAT" ) )
		{
			int nValue = m_pSliderVoiceChat->GetValue();
			m_pStaticVoiceChat->SetIntToText( nValue );
			if( CVoiceChatClient::IsActive() ) CVoiceChatClient::GetInstance().SetVoiceChatVolume( nValue/100.0f );
			return;
		}
		else if( IsCmdControl( "ID_SLIDER_MIC" ) )
		{
			int nValue = m_pSliderMic->GetValue();
			m_pStaticMic->SetIntToText( nValue );
			if( CVoiceChatClient::IsActive() ) CVoiceChatClient::GetInstance().SetMicVolume( nValue/100.0f );
			return;
		}
		else if( IsCmdControl( "ID_SLIDER_VOLUME" ) )
		{
			int nValue = m_pSliderVolume->GetValue();
			m_pStaticVolume->SetIntToText( nValue );
			if( CDnVoiceChatTask::IsActive() ) GetVoiceChatTask().SetAutoAdjustVolume( m_pCheckBoxAutoVolume->IsChecked(), nValue/100.0f );
			return;
		}
	}
	else if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if( IsCmdControl( "ID_CHECKBOX_VOICECHAT" ) && bTriggeredByUser )
		{
			if( m_pVoiceChat->IsChecked() && CDnVoiceChatTask::IsActive() )
				GetVoiceChatTask().InitVoiceChat();
			CheckEnableControl();
			if( m_pVoiceChat->IsChecked() )
			{
				ProcessCommand( EVENT_SLIDER_VALUE_CHANGED, false, m_pSliderVoiceChat, 0 );
				ProcessCommand( EVENT_SLIDER_VALUE_CHANGED, false, m_pSliderMic, 0 );
			}
			return;
		}
		else if( IsCmdControl( "ID_CHECK_MICBOOST" ) )
		{
			if( CVoiceChatClient::IsActive() )
				CVoiceChatClient::GetInstance().SetMicBoost( m_pMicBoost->IsChecked() );
		}
		else if( IsCmdControl( "ID_CHECKBOX_AUTOVOLUME" ) )
		{
			int nValue = m_pSliderVolume->GetValue();
			if( CDnVoiceChatTask::IsActive() ) GetVoiceChatTask().SetAutoAdjustVolume( m_pCheckBoxAutoVolume->IsChecked(), nValue/100.0f );
			bool bEnable = m_pCheckBoxAutoVolume->IsChecked();
			m_pButtonDown3->Enable( bEnable );
			m_pButtonUp3->Enable( bEnable );
			m_pSliderVolume->Enable( bEnable );
			m_pStaticVolume->Enable( bEnable );
		}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED && bTriggeredByUser )
	{
		if( IsCmdControl( "ID_COMBOBOX_MIC" ) )
		{
			int nDeviceIndex = -1;
			if( CVoiceChatClient::IsActive() && m_pComboCaptureDevice->GetSelectedValue( nDeviceIndex ) )
				CVoiceChatClient::GetInstance().SetDevice( nDeviceIndex, true );
		}
	}

	CDnOptionDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnGameVoiceChatOptDlg::ExportSetting()
{
	if( CGameOption::GetInstance().m_bVoiceChat != m_pVoiceChat->IsChecked() ) {
		if( CDnVoiceChatTask::IsActive() ) {
			GetVoiceChatTask().ReqVoiceAvailable( m_pVoiceChat->IsChecked() );
		}
	}

	CGameOption::GetInstance().m_bVoiceChat = m_pVoiceChat->IsChecked();
	CGameOption::GetInstance().m_fVoiceChatVolume = (m_pSliderVoiceChat->GetValue()/100.0f);
	CGameOption::GetInstance().m_fMicVolume = (m_pSliderMic->GetValue()/100.0f);
	CGameOption::GetInstance().m_bMicBoost = m_pMicBoost->IsChecked();
	CGameOption::GetInstance().m_bUseAutoAdjustVolume = m_pCheckBoxAutoVolume->IsChecked();
	CGameOption::GetInstance().m_fAutoAdjustVolume = (m_pSliderVolume->GetValue()/100.0f);
	CGameOption::GetInstance().Save();
}

void CDnGameVoiceChatOptDlg::ImportSetting()
{
	m_pMicBoost->SetChecked( CGameOption::GetInstance().m_bMicBoost );

	int nValue = int(CGameOption::GetInstance().m_fVoiceChatVolume * 100.0f + 0.01f);
	m_pSliderVoiceChat->SetValue( nValue );
	m_pStaticVoiceChat->SetIntToText( nValue );

	nValue = int(CGameOption::GetInstance().m_fMicVolume * 100.0f + 0.01f);
	m_pSliderMic->SetValue( nValue );
	m_pStaticMic->SetIntToText( nValue );

	m_pCheckBoxAutoVolume->SetChecked( CGameOption::GetInstance().m_bUseAutoAdjustVolume );
	nValue = int(CGameOption::GetInstance().m_fAutoAdjustVolume * 100.0f + 0.01f);
	m_pSliderVolume->SetValue( nValue );
	m_pStaticVolume->SetIntToText( nValue );

	m_pVoiceChat->SetChecked( CGameOption::GetInstance().m_bVoiceChat );

	CheckEnableControl();
}

void CDnGameVoiceChatOptDlg::CancelOption()
{
	CGameOption::GetInstance().ApplyVoiceChat();
}

void CDnGameVoiceChatOptDlg::CheckEnableControl()
{
	bool bVoiceChat = ( m_pVoiceChat->IsChecked() && CDnVoiceChatTask::IsActive() && GetVoiceChatTask().IsInitialized() );
	m_pButtonDown->Enable( bVoiceChat );
	m_pButtonUp->Enable( bVoiceChat );
	m_pSliderVoiceChat->Enable( bVoiceChat );
	m_pStaticVoiceChat->Enable( bVoiceChat );

	m_pCheckBoxAutoVolume->Enable( bVoiceChat );
	bool bAutoVolume = ( bVoiceChat && m_pCheckBoxAutoVolume->IsChecked() );
	m_pButtonDown3->Enable( bAutoVolume );
	m_pButtonUp3->Enable( bAutoVolume );
	m_pSliderVolume->Enable( bAutoVolume );
	m_pStaticVolume->Enable( bAutoVolume );

	bool bMic = ( m_pVoiceChat->IsChecked() && CDnVoiceChatTask::IsActive() && GetVoiceChatTask().IsInitialized() &&
		CVoiceChatClient::IsActive() && CVoiceChatClient::GetInstance().IsExistMic() );
	bool bMicControl = ( m_pVoiceChat->IsChecked() && CDnVoiceChatTask::IsActive() && GetVoiceChatTask().IsInitialized() &&
		CVoiceChatClient::IsActive() && CVoiceChatClient::GetInstance().IsExistMicControl() );

	m_pButtonDown2->Enable( bMicControl );
	m_pButtonUp2->Enable( bMicControl );
	m_pSliderMic->Enable( bMicControl );
	m_pStaticMic->Enable( bMicControl );

	if( !bMic ) {
		for( int i = 0; i < (int)m_vecStaticTest.size(); ++i )
			m_vecStaticTest[i]->Show( false );
	}

	if( !m_pVoiceChat->IsChecked() )
		m_pStaticTest->SetText( L"" );
	else if( !bVoiceChat )
		m_pStaticTest->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3417 ) );	// Cannot initialize voice chat.
	else if( !bMic )
		m_pStaticTest->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3188 ) );
	else if( CGameOption::GetInstance().m_bMicTest )
		m_pStaticTest->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3186 ) );
	else
		m_pStaticTest->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3182 ) );

	m_pButtonTest->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3181 ) );
	m_pButtonTest->Enable( bVoiceChat && bMic );

	// Reset if testing was in progress
	m_fTestTime = 0.0f;

	bool bBoost = false;
	if( bVoiceChat && bMic && CVoiceChatClient::IsActive() && CVoiceChatClient::GetInstance().IsEnableMicBoost() )
		bBoost = true;
	m_pMicBoost->Show( bBoost );
	m_pStaticMicBoost->Show( bBoost );

	m_pComboCaptureDevice->RemoveAllItems();
	if( bVoiceChat && bMic )
	{
		std::wstring wszDeviceName, wszName;
		CVoiceChatClient::GetInstance().GetCaptureDeviceName( wszDeviceName );
		int nNum = CVoiceChatClient::GetInstance().GetNumDevice();
		for( int i = 0; i < nNum; ++i )
		{
			ICEClient_device *pDevice = CVoiceChatClient::GetInstance().GetDevice( i );
			if( pDevice->devtype == ICECLIENT_DEVICE_CAPTURE )
			{
				ToWideString( pDevice->name, wszName );
				m_pComboCaptureDevice->AddItem( wszName.c_str(), NULL, i, true );
				if( wszDeviceName == wszName )
					m_pComboCaptureDevice->SetSelectedByValue( i );
			}
		}
		m_pComboCaptureDevice->Enable( true );
	}
	else
	{
		m_pComboCaptureDevice->Enable( false );
	}

	bool bPremium = false;
	std::vector<CDnItem*> pVecResult;
	if( bMic && CDnItemTask::IsActive() && GetItemTask().FindItemFromItemType( ITEMTYPE_VOICEFONT, ST_INVENTORY_CASH, pVecResult ) )
		bPremium = true;
	m_pButtonPremium->Enable( bPremium );
}

void CDnGameVoiceChatOptDlg::Process( float fElapsedTime )
{
	CDnOptionDlg::Process( fElapsedTime );
	if( !IsShow() ) return;

	bool bMic = ( m_pVoiceChat->IsChecked() && CDnVoiceChatTask::IsActive() && GetVoiceChatTask().IsInitialized() &&
		CVoiceChatClient::IsActive() && CVoiceChatClient::GetInstance().IsExistMic() );
	if( !bMic ) return;

	BYTE bySpeak = CVoiceChatClient::GetInstance().GetSpeaking();
	float fRate = (float)bySpeak / UCHAR_MAX;

	int nNumShow = (int)(m_vecStaticTest.size() * fRate);

	for( int i = 0; i < nNumShow; ++i ) {
		m_vecStaticTest[i]->Show( true );
	}

	for( int i = nNumShow; i < (int)m_vecStaticTest.size(); ++i ) {
		m_vecStaticTest[i]->Show( false );
	}

	// Test check
	if( m_fTestTime > 0.0f )
	{
		m_fTestTime -= fElapsedTime;

		WCHAR wszText[256];
		swprintf_s( wszText, _countof(wszText), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3184 ), (int)m_fTestTime );
		m_pStaticTest->SetText( wszText );

		// End test
		if( nNumShow )
		{
			m_fTestTime = 0.0f;
			CGameOption::GetInstance().m_bMicTest = true;
			CGameOption::GetInstance().Save();
			CheckEnableControl();
		}
		else if( m_fTestTime <= 0.0f )
		{
			CGameOption::GetInstance().m_bMicTest = false;
			CGameOption::GetInstance().Save();
			GetInterface().MessageBox( 3187 );
			CheckEnableControl();
		}
	}
}

bool CDnGameVoiceChatOptDlg::IsChanged()
{
	if( CGameOption::GetInstance().m_bVoiceChat != m_pVoiceChat->IsChecked() ||
		(int)(CGameOption::GetInstance().m_fVoiceChatVolume*100.0f + 0.01f) != m_pSliderVoiceChat->GetValue() ||
		(int)(CGameOption::GetInstance().m_fMicVolume*100.0f + 0.01f) != m_pSliderMic->GetValue() ||
		CGameOption::GetInstance().m_bMicBoost != m_pMicBoost->IsChecked() ||
		(int)(CGameOption::GetInstance().m_fAutoAdjustVolume*100.0f + 0.01f) != m_pSliderVolume->GetValue() ||
		CGameOption::GetInstance().m_bUseAutoAdjustVolume != m_pCheckBoxAutoVolume->IsChecked() )
		return true;
	return false;
}

#endif
