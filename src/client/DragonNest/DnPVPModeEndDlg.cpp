#include "StdAfx.h"
#include "DnPVPModeEndDlg.h"
#include "DnInterface.h"
#include "SystemSendPacket.h"
#include "DnTableDB.h"

#ifdef PRE_MOD_PVPOBSERVER
#include "EtUITextUtil.h"
#include "DnMutatorRespawnMode.h"
#endif // PRE_MOD_PVPOBSERVER

//#include "DnInterfaceDlgID.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnPVPModeEndDlg::CDnPVPModeEndDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
, m_pStaticResult(NULL)
, m_pStaticSecResult(NULL)
, m_pStaticThirResult(NULL)
, m_pStaticQuartResult(NULL)
{
	m_nGameWinSound = -1;
	m_nGameLoseSound= -1;
	m_nGameDrawSound= -1;
	m_nRoundResultSound= -1;
	
	Reset();
}

CDnPVPModeEndDlg::~CDnPVPModeEndDlg(void)
{
	if( m_nGameWinSound != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nGameWinSound );
	if( m_nGameLoseSound != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nGameLoseSound );
	if( m_nGameDrawSound != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nGameDrawSound );
	if( m_nRoundResultSound != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nRoundResultSound );
	
}

void CDnPVPModeEndDlg::Reset()
{
	if( m_pStaticResult )
		m_pStaticResult->SetText(L"");
	if( m_pStaticSecResult )
		m_pStaticSecResult->SetText(L"");
	if( m_pStaticThirResult )
		m_pStaticThirResult->SetText(L"");
	if( m_pStaticQuartResult )
		m_pStaticQuartResult->SetText(L"");
}

void CDnPVPModeEndDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpResult.ui" ).c_str(), bShow );
}

void CDnPVPModeEndDlg::InitialUpdate()
{
	m_pStaticResult = GetControl<CEtUIStatic>("ID_PVP_RESULT0");
	m_pStaticSecResult = GetControl<CEtUIStatic>("ID_PVP_RESULT1");
	m_pStaticThirResult = GetControl<CEtUIStatic>("ID_PVP_RESULT2");
	m_pStaticQuartResult = GetControl<CEtUIStatic>("ID_PVP_RESULT3");

	const char *szFileName = CDnTableDB::GetInstance().GetFileName( PVPResult::GameWinSound );
	if( strlen( szFileName ) > 0 )
	{
		if( m_nGameWinSound == -1 )
			m_nGameWinSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );			
	}

	szFileName = CDnTableDB::GetInstance().GetFileName( PVPResult::GameLoseSound );
	if( strlen( szFileName ) > 0 )
	{
		if( m_nGameLoseSound == -1 )
			m_nGameLoseSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	}

	szFileName = CDnTableDB::GetInstance().GetFileName( PVPResult::GameDrawSound );
	if( strlen( szFileName ) > 0 )
	{
		if( m_nGameDrawSound == -1 )
			m_nGameDrawSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	}

	szFileName = CDnTableDB::GetInstance().GetFileName( PVPResult::RoundResultSound );
	if( strlen( szFileName ) > 0 )
	{
		if( m_nRoundResultSound == -1 )
			m_nRoundResultSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	}

	//m_pStaticTitle ->SetText( L"Temp:Exit Game");

}

void CDnPVPModeEndDlg::SetResultString1( const WCHAR * wszString )
{
	if( wszString && m_pStaticResult )
		m_pStaticResult->SetText( wszString );
}

void CDnPVPModeEndDlg::SetResultString2( const WCHAR * wszString )
{
	if( wszString && m_pStaticSecResult )
		m_pStaticSecResult->SetText( wszString );
}

#ifdef PRE_ADD_PVP_TOURNAMENT
void CDnPVPModeEndDlg::SetResultString2(const WCHAR* wszString, DWORD color)
{
	if (wszString && m_pStaticSecResult)
	{
		m_pStaticSecResult->SetTextColor(color);
		m_pStaticSecResult->SetText(wszString);
	}
}
#endif

void CDnPVPModeEndDlg::SetResultString3( const WCHAR * wszString )
{
	if( wszString && m_pStaticThirResult )
	{
#ifdef PRE_MOD_PVPOBSERVER
		m_pStaticThirResult->SetTextColor( textcolor::WHITE );
#endif // PRE_MOD_PVPOBSERVER
		m_pStaticThirResult->SetText( wszString );	
	}
}

void CDnPVPModeEndDlg::SetResultString4( const WCHAR * wszString )
{
	if( wszString && m_pStaticQuartResult )
		m_pStaticQuartResult->SetText( wszString );
}


#ifdef PRE_MOD_PVPOBSERVER
void CDnPVPModeEndDlg::SetResultStringObServer( UINT winTeam, const wchar_t * str, bool bZombieMode, bool bVictory  )
{
	if( m_pStaticResult && m_pStaticThirResult )
	{
		// Default Blue.
		DWORD color = textcolor::LIGHTSKYBLUE;
		int uiIndex = 0;		

		// Team B Red.
		if( winTeam == PvPCommon::Team::B )
			color = textcolor::RED;
			
		if( bZombieMode )
			uiIndex = ( winTeam == PvPCommon::Team::B ) ? 7866 : 7865; // "Ghoul Team", "Human Team"
		else
		{
			if( bVictory )
				uiIndex = ( winTeam == PvPCommon::Team::B ) ? 126138 : 126137; // "Chaos Dragon Team Wins!!", "Order Dragon Team Wins!!"
			else
				uiIndex = ( winTeam == PvPCommon::Team::B ) ? 7851 : 7850; // "Chaos Dragon Team", "Order Dragon Team"
		}
		
		m_pStaticResult->SetText( str ); // Match result - "Win", "Round" ..

		// Team name.
		m_pStaticThirResult->SetTextColor( color );
		m_pStaticThirResult->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, uiIndex ) );	
	}
}
#endif

void CDnPVPModeEndDlg::PlayResultSound(  byte cResult )
{
	switch( cResult )
	{
	case PVPResult::Win :
		if( m_nGameWinSound != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nGameWinSound, false );
		break;
	case PVPResult::Lose:
		if( m_nGameLoseSound != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nGameLoseSound, false );
		break;
	case PVPResult::Draw:
		if( m_nGameDrawSound != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nGameDrawSound, false );
		break;
	case PVPResult::RoundEND:
		if( m_nRoundResultSound != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nRoundResultSound, false );
		break;
	}

}
