#include "StdAfx.h"
#include "DnPVPInfoDlg.h"
#include "DnInterfaceString.h"
#include "DnInterface.h"
#include "DnTableDB.h"
#include "DnPartyTask.h"
#include "DnPlayerActor.h"
#include "DnPVPLobbyVillageTask.h"
#include "TaskManager.h"
#include "DnCharInventory.h"
#include "DnStorageInventory.h"
#include "DnItemTask.h"

CDnPVPInfoDlg::CDnPVPInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
,
 pUIJobIcon(NULL),   // Job name 
 pUIGuildName(NULL), // Guild name
 pUILevel(NULL),     // Level
 pUIName(NULL),      // ID
 pUIRankIcon(NULL),  // Rank icon
 pUIRankName(NULL),  // Rank name
 pUIXP(NULL),        // Current Colosseum XP
 pUINeedXP(NULL)    // XP required for next level
 ,pUIScore(NULL)    // Win/Loss
,pUIMedal(NULL)     // Number of owned medals
{
}

CDnPVPInfoDlg::~CDnPVPInfoDlg(void)
{
}

void CDnPVPInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpInfoDlg.ui" ).c_str(), bShow );
}

void CDnPVPInfoDlg::InitialUpdate()
{
	pUIJobIcon =  GetControl<CDnJobIconStatic>("ID_STATIC_JOB");
	pUIGuildName = GetControl<CEtUIStatic>("ID_TEXT_GUILDNAME");
	pUILevel =  GetControl<CEtUIStatic>("ID_TEXT_LEVEL");
	pUIName =  GetControl<CEtUIStatic>("ID_TEXT_NAME");
	pUIRankIcon =  GetControl<CEtUITextureControl>("ID_STATIC_RANK");
	pUIRankName =  GetControl<CEtUIStatic>("ID_TEXT_RANKNAME");
	pUIXP =  GetControl<CEtUIStatic>("ID_TEXT_PVPXP");
	pUINeedXP =  GetControl<CEtUIStatic>("ID_TEXT_NEXTXP");
	pUIScore = GetControl<CEtUIStatic>("ID_TEXT_SCORE");
	pUIMedal = GetControl<CEtUIStatic>("ID_TEXT_MEDAL");

	SetPVPInfo();
}

void CDnPVPInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPVPInfoDlg::Process( float fElapsedTime )
{
	if(!IsShow())
		return;

	CEtUIDialog::Process(fElapsedTime);
}

void CDnPVPInfoDlg::SetPVPInfo()
{
	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
	if(!pPVPLobbyTask)
		return;
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPVPRANK );
	
	int iUIString = 0;

	if ( pSox && pSox->IsExistItem( pPVPLobbyTask->GetUserInfo().sPvPInfo.cLevel ) )
	{
		iUIString = pSox->GetFieldFromLablePtr(  pPVPLobbyTask->GetUserInfo().sPvPInfo.cLevel , "PvPRankUIString" )->GetInteger();
	}
	if(pPVPLobbyTask->GetUserInfo().GuildSelfView.IsSet()) // Is player in a guild?
	{
		pUIGuildName->SetText(pPVPLobbyTask->GetUserInfo().GuildSelfView.wszGuildName);
	}

	vector<int> jobHistory;
	for( int i=0; i<JOBMAX; i++ ) {
		if( pPVPLobbyTask->GetUserInfo().cJobArray[i] != 0 )
			jobHistory.push_back( pPVPLobbyTask->GetUserInfo().cJobArray[i] );

	}

	if( !jobHistory.empty() )
	{
		pUIJobIcon->SetIconID(jobHistory[jobHistory.size() -1],true);
	}
	
	pUILevel->SetIntToText(pPVPLobbyTask->GetUserInfo().cLevel);
	pUIName->SetText( pPVPLobbyTask->GetUserInfo().wszCharacterName );
	
	//Icon
	int iIconW,iIconH;
	int iU,iV;
	iIconW = GetInterface().GeticonWidth();
	iIconH = GetInterface().GeticonHeight();
	if( GetInterface().ConvertPVPGradeToUV( pPVPLobbyTask->GetUserInfo().sPvPInfo.cLevel , iU, iV ))
	{
		pUIRankIcon->SetTexture(GetInterface().GetPVPIconTex(), iU, iV, iIconW, iIconH);
		pUIRankIcon->Show(true);
	}

	pUIRankName->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( pPVPLobbyTask->GetUserInfo().sPvPInfo.cLevel, "PvPRankUIString" )->GetInteger()));
	pUIXP->SetIntToText( pPVPLobbyTask->GetUserInfo().sPvPInfo.uiXP );

	if ( pSox )
	{
		int nNextXP = pSox->GetFieldFromLablePtr( pPVPLobbyTask->GetUserInfo().sPvPInfo.cLevel , "PvPRankEXP" )->GetInteger();
		pUINeedXP->SetIntToText(nNextXP);
	}

	WCHAR wszStr[128];
	wsprintf( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PVPWinUIString ),pPVPLobbyTask->GetUserInfo().sPvPInfo.uiWin , pPVPLobbyTask->GetUserInfo().sPvPInfo.uiLose , pPVPLobbyTask->GetUserInfo().sPvPInfo.uiDraw );

	pUIScore->SetText(wszStr);
	pUIMedal->SetIntToText(GetItemTask().GetCharInventory().GetItemCount( PVPMedalTableID ) + GetItemTask().GetStorageInventory().GetItemCount(PVPMedalTableID));
}
