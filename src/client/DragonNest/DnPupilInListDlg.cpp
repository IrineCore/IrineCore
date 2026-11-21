#include "Stdafx.h"
#include "DnPupilInListDlg.h"
#include "DnMasterTask.h"
#include "DnInterfaceString.h"
#include "SyncTimer.h"
#include "DnWorldData.h"
#include "DnWorldDataManager.h"

CDnPupilInListDlg::CDnPupilInListDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
}

CDnPupilInListDlg::~CDnPupilInListDlg()
{
}

void CDnPupilInListDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PupilInListDlg.ui" ).c_str(), bShow );
}

void CDnPupilInListDlg::InitialUpdate()
{

}

void CDnPupilInListDlg::Show(bool bShow)
{
	CEtUIDialog::Show( bShow );
}

void CDnPupilInListDlg::Process(float fElapsedTime)
{
	CEtUIDialog::Process( fElapsedTime );
}

void CDnPupilInListDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_CLOSE" ) ) {
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPupilInListDlg::SetInfo( const TPupilInfoWithLocation & RefData )
{
	WCHAR szString[256];
	DWORD color;

	if( 0 == RefData.Location.cServerLocation )
	{
		color = textcolor::DARKGRAY;
		CONTROL( Static, ID_TEXT_NAME )->SetText( RefData.wszCharName );
		CONTROL( Static, ID_TEXT_NAME )->SetTextColor( color );

		CONTROL( Static, ID_TEXT_MAP )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4305 ) );	//Offline
		CONTROL( Static, ID_TEXT_MAP )->SetTextColor( color );

		INT64 timeDelta = CSyncTimer::GetInstance().GetCurTime() - RefData.tLastConnectDate;
		INT64 biHourDiff = timeDelta / 3600;	//Convert to time units
		INT64 biDayDiff = biHourDiff / 24;		//Convert to days
		biHourDiff = biHourDiff % 24;			//Convert back to time

		WCHAR wszTemp[64] = {0,};
		if( biDayDiff > 28 ) {
			swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3796 ), 28 );
		}
		else if( biDayDiff > 0 ) {
			swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3795 ), biDayDiff );
		}
		else if( biHourDiff ) {
			swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3794 ), biHourDiff );
		}
		else
		{
			swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3794 ), 1 );
		}
		CONTROL( Static, ID_TEXT_CONDITION )->SetText( wszTemp );
	}
	else
	{
		color = textcolor::WHITE;
		CONTROL( Static, ID_TEXT_NAME )->SetText( RefData.wszCharName );
		CONTROL( Static, ID_TEXT_CONDITION )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3798 ) );	//Online

		CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( RefData.Location.nMapIdx );

		if( pWorldData )
		{
			std::wstring wszWorldName;

			pWorldData->GetMapName(wszWorldName);

			if( RefData.Location.nChannelID > 0 && eLocationState::_LOCATION_VILLAGE == RefData.Location.cServerLocation  )
			{
				WCHAR wszTemp[256];
				swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4307 ), wszWorldName.c_str(), RefData.Location.nChannelID );
				wszWorldName = std::wstring(wszTemp);
			}

			CONTROL( Static, ID_TEXT_MAP )->SetText( wszWorldName );
		}
	}

	CONTROL( Static, ID_TEXT_CLASS )->SetText( DN_INTERFACE::STRING::GetJobString( (int)RefData.cJob ) );
	CONTROL( Static, ID_TEXT_CLASS )->SetTextColor( color );

	wsprintf( szString, L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 64 ), (int)RefData.cLevel );
	CONTROL( Static, ID_TEXT_LEVEL )->SetText( szString );
	CONTROL( Static, ID_TEXT_LEVEL )->SetTextColor( color );

	wsprintf( szString, L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7226 ), RefData.iFavorPoint );
	CONTROL( Static, ID_TEXT_REPUTE )->SetText( szString );
	CONTROL( Static, ID_TEXT_REPUTE )->SetTextColor( color );	
}
