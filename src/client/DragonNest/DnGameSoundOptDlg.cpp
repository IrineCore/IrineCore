#include "StdAfx.h"
#include "DnGameSoundOptDlg.h"
#include "GameOption.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGameSoundOptDlg::CDnGameSoundOptDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnOptionDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pComboBoxQuality(NULL)
{
}

CDnGameSoundOptDlg::~CDnGameSoundOptDlg(void)
{
}

void CDnGameSoundOptDlg::Initialize( bool bShow )
{
	BaseClass::Initialize( CEtResourceMng::GetInstance().GetFullName( "GameSoundOptDlg.ui" ).c_str(), bShow );
}

void CDnGameSoundOptDlg::InitialUpdate()
{
	m_volumeSlider[sound_ALL].m_pButtonDown = GetControl<CEtUIButton>("ID_BUTTON_ALL_DOWN");
	m_volumeSlider[sound_ALL].m_pButtonUp = GetControl<CEtUIButton>("ID_BUTTON_ALL_UP");
	m_volumeSlider[sound_ALL].m_pSlider = GetControl<CEtUISlider>("ID_SLIDER_ALL");
	m_volumeSlider[sound_ALL].m_pStatic = GetControl<CEtUIStatic>("ID_STATIC_ALL");
	m_volumeSlider[sound_ALL].m_pCheckBox = GetControl<CEtUICheckBox>("ID_TURNOFF_ALL");
	
	m_volumeSlider[sound_BGM].m_pButtonDown = GetControl<CEtUIButton>("ID_BUTTON_BM_DOWN");
	m_volumeSlider[sound_BGM].m_pButtonUp = GetControl<CEtUIButton>("ID_BUTTON_BM_UP");
	m_volumeSlider[sound_BGM].m_pSlider = GetControl<CEtUISlider>("ID_SLIDER_BM");
	m_volumeSlider[sound_BGM].m_pStatic = GetControl<CEtUIStatic>("ID_STATIC_BM");
	m_volumeSlider[sound_BGM].m_pCheckBox = GetControl<CEtUICheckBox>("ID_TURNOFF_BM");

	m_volumeSlider[sound_3D].m_pButtonDown = GetControl<CEtUIButton>("ID_BUTTON_EFFECT_DOWN");
	m_volumeSlider[sound_3D].m_pButtonUp = GetControl<CEtUIButton>("ID_BUTTON_EFFECT_UP");
	m_volumeSlider[sound_3D].m_pSlider = GetControl<CEtUISlider>("ID_SLIDER_EFFECT");
	m_volumeSlider[sound_3D].m_pStatic = GetControl<CEtUIStatic>("ID_STATIC_EFFECT");
	m_volumeSlider[sound_3D].m_pCheckBox = GetControl<CEtUICheckBox>("ID_TURNOFF_EFFECT");
	
	m_volumeSlider[sound_2D].m_pButtonDown = GetControl<CEtUIButton>("ID_BUTTON_UI_DOWN");
	m_volumeSlider[sound_2D].m_pButtonUp = GetControl<CEtUIButton>("ID_BUTTON_UI_UP");
	m_volumeSlider[sound_2D].m_pSlider = GetControl<CEtUISlider>("ID_SLIDER_UI");
	m_volumeSlider[sound_2D].m_pStatic = GetControl<CEtUIStatic>("ID_STATIC_UI");
	m_volumeSlider[sound_2D].m_pCheckBox = GetControl<CEtUICheckBox>("ID_TURNOFF_UI");
	
	m_volumeSlider[sound_VOICE].m_pButtonDown = GetControl<CEtUIButton>("ID_BUTTON_VOICE_DOWN");
	m_volumeSlider[sound_VOICE].m_pButtonUp = GetControl<CEtUIButton>("ID_BUTTON_VOICE_UP");
	m_volumeSlider[sound_VOICE].m_pSlider = GetControl<CEtUISlider>("ID_SLIDER_VOICE");
	m_volumeSlider[sound_VOICE].m_pStatic = GetControl<CEtUIStatic>("ID_STATIC_VOICE");
	m_volumeSlider[sound_VOICE].m_pCheckBox = GetControl<CEtUICheckBox>("ID_TURNOFF_VOICE");
	
	m_pComboBoxQuality = GetControl<CEtUIComboBox>("ID_COMBOBOX_QUALITY");
	m_pComboBoxQuality->Enable( false );
}

void CDnGameSoundOptDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_ALL_UP" ) )
		{
			m_volumeSlider[sound_ALL].OnClicked_ButtonUp();
			CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_BGM].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "3D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_3D].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "2D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_2D].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_VOICE].GetVolume() );
			return;
		}

		if( IsCmdControl("ID_BUTTON_ALL_DOWN" ) )
		{
			m_volumeSlider[sound_ALL].OnClicked_ButtonDown();
			CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_BGM].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "3D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_3D].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "2D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_2D].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_VOICE].GetVolume() );
			return;
		}

		if( IsCmdControl("ID_BUTTON_BM_UP" ) )
		{
			m_volumeSlider[sound_BGM].OnClicked_ButtonUp();
			CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_BGM].GetVolume() );
			return;
		}

		if( IsCmdControl("ID_BUTTON_BM_DOWN" ) )
		{
			m_volumeSlider[sound_BGM].OnClicked_ButtonDown();
			CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_BGM].GetVolume() );
			return;
		}

		if( IsCmdControl("ID_BUTTON_EFFECT_UP" ) )
		{
			m_volumeSlider[sound_3D].OnClicked_ButtonUp();
			CEtSoundEngine::GetInstance().SetMasterVolume( "3D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_3D].GetVolume() );
			return;
		}

		if( IsCmdControl("ID_BUTTON_EFFECT_DOWN" ) )
		{
			m_volumeSlider[sound_3D].OnClicked_ButtonDown();
			CEtSoundEngine::GetInstance().SetMasterVolume( "3D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_3D].GetVolume() );
			return;
		}

		if( IsCmdControl("ID_BUTTON_UI_UP" ) )
		{
			m_volumeSlider[sound_2D].OnClicked_ButtonUp();
			CEtSoundEngine::GetInstance().SetMasterVolume( "2D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_2D].GetVolume() );
			return;
		}

		if( IsCmdControl("ID_BUTTON_UI_DOWN" ) )
		{
			m_volumeSlider[sound_2D].OnClicked_ButtonDown();
			CEtSoundEngine::GetInstance().SetMasterVolume( "2D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_2D].GetVolume() );
			return;
		}

		if( IsCmdControl("ID_BUTTON_VOICE_UP" ) )
		{
			m_volumeSlider[sound_VOICE].OnClicked_ButtonUp();
			CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_VOICE].GetVolume() );
			return;
		}

		if( IsCmdControl("ID_BUTTON_VOICE_DOWN" ) )
		{
			m_volumeSlider[sound_VOICE].OnClicked_ButtonDown();
			CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_VOICE].GetVolume() );
			return;
		}
	}
	else if( (nCommand == EVENT_CHECKBOX_CHANGED) && bTriggeredByUser )
	{
		bool bCheck[sound_Amount] = {
			!m_volumeSlider[sound_ALL].m_pCheckBox->IsChecked(),
			!m_volumeSlider[sound_BGM].m_pCheckBox->IsChecked(),
			!m_volumeSlider[sound_3D].m_pCheckBox->IsChecked(),
			!m_volumeSlider[sound_2D].m_pCheckBox->IsChecked(),
			!m_volumeSlider[sound_VOICE].m_pCheckBox->IsChecked() };

		if( IsCmdControl( "ID_TURNOFF_ALL" ) )
		{			
			m_volumeSlider[sound_ALL].SetMute( bCheck[sound_ALL] );
			CEtSoundEngine::GetInstance().SetMute( "BGM", bCheck[sound_ALL] || bCheck[sound_BGM] );
			CEtSoundEngine::GetInstance().SetMute( "3D", bCheck[sound_ALL] || bCheck[sound_3D] );
			CEtSoundEngine::GetInstance().SetMute( "2D", bCheck[sound_ALL] || bCheck[sound_2D] );
			CEtSoundEngine::GetInstance().SetMute( "VOICE", bCheck[sound_ALL] || bCheck[sound_VOICE] );
			return;
		}

		if( IsCmdControl( "ID_TURNOFF_BM" ) )
		{			
			m_volumeSlider[sound_BGM].SetMute( bCheck[sound_BGM]  );
			CEtSoundEngine::GetInstance().SetMute( "BGM", bCheck[sound_ALL] || bCheck[sound_BGM] );
			return;
		}

		if( IsCmdControl( "ID_TURNOFF_EFFECT" ) )
		{			
			m_volumeSlider[sound_3D].SetMute( bCheck[sound_3D] );
			CEtSoundEngine::GetInstance().SetMute( "3D", bCheck[sound_ALL] || bCheck[sound_3D] );
			return;
		}

		if( IsCmdControl( "ID_TURNOFF_UI" ) )
		{			
			m_volumeSlider[sound_2D].SetMute( bCheck[sound_2D] );
			CEtSoundEngine::GetInstance().SetMute( "2D", bCheck[sound_ALL] || bCheck[sound_2D] );
			return;
		}

		if( IsCmdControl( "ID_TURNOFF_VOICE" ) )
		{			
			m_volumeSlider[sound_VOICE].SetMute( bCheck[sound_VOICE] );
			CEtSoundEngine::GetInstance().SetMute( "VOICE", bCheck[sound_ALL] || bCheck[sound_VOICE] );
			return;
		}
	}
	else if( nCommand == EVENT_SLIDER_VALUE_CHANGED )
	{
		if( IsCmdControl( "ID_SLIDER_ALL" ) )
		{
			m_volumeSlider[sound_ALL].OnSliderChanged();
			CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_BGM].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "3D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_3D].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "2D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_2D].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_VOICE].GetVolume() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "NULL", m_volumeSlider[sound_ALL].GetVolume() );
			return;
		}

		if( IsCmdControl( "ID_SLIDER_BM" ) )
		{
			m_volumeSlider[sound_BGM].OnSliderChanged();
			CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_BGM].GetVolume() );
			return;
		}

		if( IsCmdControl( "ID_SLIDER_EFFECT" ) )
		{
			m_volumeSlider[sound_3D].OnSliderChanged();
			CEtSoundEngine::GetInstance().SetMasterVolume( "3D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_3D].GetVolume() );
			return;
		}

		if( IsCmdControl( "ID_SLIDER_UI" ) )
		{
			m_volumeSlider[sound_2D].OnSliderChanged();
			CEtSoundEngine::GetInstance().SetMasterVolume( "2D", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_2D].GetVolume() );
			return;
		}

		if( IsCmdControl( "ID_SLIDER_VOICE" ) )
		{
			m_volumeSlider[sound_VOICE].OnSliderChanged();
			CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", m_volumeSlider[sound_ALL].GetVolume()*m_volumeSlider[sound_VOICE].GetVolume() );
			return;
		}
	}

	BaseClass::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

// restore control value
void CDnGameSoundOptDlg::ImportSetting()
{
	m_volumeSlider[sound_ALL].SetVolume( CGameOption::GetInstance().GetVolumeTotal() );
	m_volumeSlider[sound_BGM].SetVolume( CGameOption::GetInstance().GetVolumeBGM() );
	m_volumeSlider[sound_3D].SetVolume( CGameOption::GetInstance().GetVolume3D() );
	m_volumeSlider[sound_2D].SetVolume( CGameOption::GetInstance().GetVolume2D() );
	m_volumeSlider[sound_VOICE].SetVolume( CGameOption::GetInstance().GetVolumeVoice() );

	m_volumeSlider[sound_ALL].SetMute( CGameOption::GetInstance().IsMuteTotal() );
	m_volumeSlider[sound_BGM].SetMute( CGameOption::GetInstance().IsMuteBGM() );
	m_volumeSlider[sound_3D].SetMute( CGameOption::GetInstance().IsMute3D() );
	m_volumeSlider[sound_2D].SetMute( CGameOption::GetInstance().IsMute2D() );
	m_volumeSlider[sound_VOICE].SetMute( CGameOption::GetInstance().IsMuteVoice() );

	m_volumeSlider[sound_ALL].m_pCheckBox->SetChecked( !CGameOption::GetInstance().IsMuteTotal() );
	m_volumeSlider[sound_BGM].m_pCheckBox->SetChecked( !CGameOption::GetInstance().IsMuteBGM() );
	m_volumeSlider[sound_3D].m_pCheckBox->SetChecked( !CGameOption::GetInstance().IsMute3D() );
	m_volumeSlider[sound_2D].m_pCheckBox->SetChecked( !CGameOption::GetInstance().IsMute2D() );
	m_volumeSlider[sound_VOICE].m_pCheckBox->SetChecked( !CGameOption::GetInstance().IsMuteVoice() );

	SetQuality();
}

void CDnGameSoundOptDlg::SetQuality()
{
	m_pComboBoxQuality->RemoveAllItems();

	m_pComboBoxQuality->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3028 ), NULL, CGameOption::Sound_High );
	m_pComboBoxQuality->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3029 ), NULL, CGameOption::Sound_Middle );
	m_pComboBoxQuality->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3030 ), NULL, CGameOption::Sound_Low );

	m_pComboBoxQuality->SetSelectedByIndex( CGameOption::GetInstance().m_nSoundQuality );
}

void CDnGameSoundOptDlg::ExportSetting()
{
	CGameOption::GetInstance().SetVolumeTotal( m_volumeSlider[sound_ALL].GetVolume() );
	CGameOption::GetInstance().SetVolumeBGM( m_volumeSlider[sound_BGM].GetVolume() );
	CGameOption::GetInstance().SetVolume3D( m_volumeSlider[sound_3D].GetVolume() );
	CGameOption::GetInstance().SetVolume2D( m_volumeSlider[sound_2D].GetVolume() );
	CGameOption::GetInstance().SetVolumeVoice( m_volumeSlider[sound_VOICE].GetVolume() );

	CGameOption::GetInstance().SetMuteTotal( !m_volumeSlider[sound_ALL].m_pCheckBox->IsChecked() );
	CGameOption::GetInstance().SetMuteBGM( !m_volumeSlider[sound_BGM].m_pCheckBox->IsChecked() );
	CGameOption::GetInstance().SetMute3D( !m_volumeSlider[sound_3D].m_pCheckBox->IsChecked() );
	CGameOption::GetInstance().SetMute2D( !m_volumeSlider[sound_2D].m_pCheckBox->IsChecked() );
	CGameOption::GetInstance().SetMuteVoice( !m_volumeSlider[sound_VOICE].m_pCheckBox->IsChecked() );

	CGameOption::GetInstance().Save();
}

bool CDnGameSoundOptDlg::IsChanged()
{
	if( CGameOption::GetInstance().GetVolumeTotal() != m_volumeSlider[sound_ALL].GetVolume() ||
		CGameOption::GetInstance().GetVolumeBGM() != m_volumeSlider[sound_BGM].GetVolume() ||
		CGameOption::GetInstance().GetVolume3D() != m_volumeSlider[sound_3D].GetVolume() ||
		CGameOption::GetInstance().GetVolume2D() != m_volumeSlider[sound_2D].GetVolume() ||
		CGameOption::GetInstance().GetVolumeVoice() != m_volumeSlider[sound_VOICE].GetVolume() )
		return true;

	// Checked means sound ON, so check the inverse.
	if( CGameOption::GetInstance().IsMuteTotal() == m_volumeSlider[sound_ALL].m_pCheckBox->IsChecked() ||
		CGameOption::GetInstance().IsMuteBGM() == m_volumeSlider[sound_BGM].m_pCheckBox->IsChecked() ||
		CGameOption::GetInstance().IsMute3D() == m_volumeSlider[sound_3D].m_pCheckBox->IsChecked() ||
		CGameOption::GetInstance().IsMute2D() == m_volumeSlider[sound_2D].m_pCheckBox->IsChecked() ||
		CGameOption::GetInstance().IsMuteVoice() == m_volumeSlider[sound_VOICE].m_pCheckBox->IsChecked() )
		return true;

	return false;
}

void CDnGameSoundOptDlg::CancelOption()
{
	CGameOption::GetInstance().ApplySound();;
}
//	
