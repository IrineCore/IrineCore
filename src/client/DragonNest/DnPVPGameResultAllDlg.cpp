#include "StdAfx.h"
#include "DnPVPGameResultAllDlg.h"
#include "DnPlayerActor.h"
#include "DnBridgeTask.h"
#include "DnTooltipDlg.h"
#include "Dninterface.h"
#include "TaskManager.h"
#include "DnPvPGameTask.h"
#include "DnMainMenuDlg.h"
#include "SystemSendPacket.h"
#include "DnGuildTask.h"
#include "DnRevengeTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

bool Compare_IndividualTotalScore( CDnPVPGameResultAllDlg::SUserInfo a, CDnPVPGameResultAllDlg::SUserInfo b )
{
	if( a.uiTotalScore < b.uiTotalScore ) 	return false;
	else if( a.uiTotalScore > b.uiTotalScore )	return true;
	else if( a.uiTotalScore == b.uiTotalScore && a.uiKObyCount < b.uiKObyCount)	return true;
	else return false;
	
	return false;
}

#if defined( PRE_ADD_REVENGE )
bool Compare_Individual_RevengeUser( CDnPVPGameResultAllDlg::SUserInfo a, CDnPVPGameResultAllDlg::SUserInfo b )
{
	return a.eRevengeUser > b.eRevengeUser;
}
#endif	// #if defined( PRE_ADD_REVENGE )

CDnPVPGameResultAllDlg::CDnPVPGameResultAllDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	m_pBoardName = NULL;

	for(int iPlayerNum = 0 ; iPlayerNum < PvP_MaxUserSlot  ; iPlayerNum++)
	{
		m_pTeamLevel[iPlayerNum] = NULL;
		m_pJobIcon[iPlayerNum] = NULL;
		m_pSlotRank[iPlayerNum] = NULL;
		m_pGuildMark[iPlayerNum] = NULL;
		m_pGuildName[iPlayerNum] = NULL;
		m_pTeamName[iPlayerNum] = NULL;
		m_pKOCount[iPlayerNum] = NULL;
		m_pKObyCount[iPlayerNum] = NULL;
		m_pTotalPoint[iPlayerNum] = NULL;
		m_pXP[iPlayerNum] = NULL;
		m_pPCRoom[iPlayerNum] = NULL;
		m_pMedalIcon[iPlayerNum] = NULL;
		m_pMedal[iPlayerNum] = NULL;
		m_pStaticCover[iPlayerNum] = NULL;
		m_pStaticAssist[iPlayerNum] = NULL;
	}

	m_IsFinalResult = false;
	m_bAssistMode = false;
	m_pButtonClose = NULL;
	m_fTotal_ElapsedTime = 0.0f;

	m_nBestUserKillCount = 0;
	m_nBestUserDeathCount = 0;
	m_nBestUserSessionID = 0;
}

CDnPVPGameResultAllDlg::~CDnPVPGameResultAllDlg(void)
{
	SAFE_RELEASE_SPTR( m_hPVPMedalIconImage );
}

void CDnPVPGameResultAllDlg::Initialize( bool bShow )
{
	if(m_bAssistMode) // Include / exclude Assist
		CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpGhoulScore.ui" ).c_str(), bShow );
	else
		CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpScore_AllDlg.ui" ).c_str(), bShow );
}

void CDnPVPGameResultAllDlg::InitialUpdate()
{
	m_pBoardName = GetControl<CEtUIStatic>("ID_STATIC1");
	char szFileName[100];
	SecureZeroMemory(szFileName,sizeof(szFileName));
	
	for(int iPlayerNum = 0 ; iPlayerNum < PvP_MaxUserSlot ; iPlayerNum++)
	{
		sprintf(szFileName,"ID_SCORE_LEVEL%d",iPlayerNum);
		m_pTeamLevel[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		sprintf(szFileName,"ID_SCORE_JOB%d",iPlayerNum);
		m_pJobIcon[iPlayerNum] = GetControl<CDnJobIconStatic>( szFileName );
		sprintf(szFileName,"ID_TEXTURE_RANK%d",iPlayerNum);
		m_pSlotRank[iPlayerNum] = GetControl<CEtUITextureControl>( szFileName );
		sprintf(szFileName,"ID_TEXTURE_MARK%d",iPlayerNum);
		m_pGuildMark[iPlayerNum] = GetControl<CEtUITextureControl>( szFileName );
		sprintf(szFileName,"ID_SCORE_GUILDNAME%d",iPlayerNum);
		m_pGuildName[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		sprintf(szFileName,"ID_SCORE_NAME%d",iPlayerNum);
		m_pTeamName[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		sprintf(szFileName,"ID_SCORE_KILLCOUND%d",iPlayerNum);
		m_pKOCount[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		sprintf(szFileName,"ID_SCORE_DEATH%d",iPlayerNum);
		m_pKObyCount[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		sprintf(szFileName,"ID_SCORE_SCORE%d",iPlayerNum);
		m_pTotalPoint[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		sprintf(szFileName,"ID_SCORE_PVPXP%d",iPlayerNum);
		m_pXP[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		sprintf(szFileName,"ID_SCORE_PC%d",iPlayerNum);
		m_pPCRoom[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		sprintf(szFileName,"ID_SCORE_BLUE_MEDALICON%d",iPlayerNum);
		m_pMedalIcon[iPlayerNum] = GetControl<CEtUITextureControl>( szFileName );
		sprintf(szFileName,"ID_SCORE_MEDAL%d",iPlayerNum);
		m_pMedal[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		sprintf(szFileName,"ID_STATIC_ME%d",iPlayerNum);
		m_pStaticCover[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		if(m_bAssistMode)
		{
			sprintf(szFileName,"ID_SCORE_ASSIST%d",iPlayerNum);
			m_pStaticAssist[iPlayerNum] = GetControl<CEtUIStatic>( szFileName );
		}
	}

	m_pButtonClose = GetControl<CEtUIButton>("ID_BUTTON_CLOSE");
	m_pButtonClose->Show( false );

	SAFE_RELEASE_SPTR( m_hPVPMedalIconImage );
	m_hPVPMedalIconImage = LoadResource( CEtResourceMng::GetInstance().GetFullName( "Pvp_Medal.dds" ).c_str(), RT_TEXTURE );
}


void CDnPVPGameResultAllDlg::RestartRound()
{
	for( UINT i = 0; i< m_vUserInfo.size();i++ )
	{
		if ( m_vUserInfo[i].iState != MINE_S )
			m_vUserInfo[i].iState = Normal_S;
	}
}

void CDnPVPGameResultAllDlg::AddUser( DnActorHandle hUser )
{
	if( hUser->GetTeam() == PvPCommon::Team::Observer )
		return;

	if( hUser )
	{
		CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(hUser.GetPointer());
		if( pPlayerActor )
		{
			SUserInfo stUserInfo;
			stUserInfo.nSessionID = pPlayerActor->GetUniqueID();
			stUserInfo.nLevel = pPlayerActor->GetLevel();
			stUserInfo.cJobClassID = pPlayerActor->GetJobClassID();
			stUserInfo.cPVPlevel = pPlayerActor->GetPvPLevel();
			stUserInfo.wszUserName = pPlayerActor->GetName();
			if( pPlayerActor->IsJoinGuild() )
				stUserInfo.GuildSelfView.Set( pPlayerActor->GetGuildSelfView() );

#if defined( PRE_ADD_REVENGE )
			CDnRevengeTask * pRevengeTask = (CDnRevengeTask *)CTaskManager::GetInstance().GetTask( "RevengeTask" );
			if( NULL != pRevengeTask && CDnBridgeTask::GetInstance().GetSessionID() != stUserInfo.nSessionID )
				 pRevengeTask->GetRevengeUserID( pPlayerActor->GetUniqueID(), stUserInfo.eRevengeUser );
#endif	// #if defined( PRE_ADD_REVENGE )

			m_vUserInfo.push_back(stUserInfo);
		}
	}
}

void CDnPVPGameResultAllDlg::SetUserState( DnActorHandle hUser, int iState )
{
	if( hUser )
	{
		for( UINT i = 0; i< m_vUserInfo.size();i++ )
		{
			if( hUser->GetUniqueID() == m_vUserInfo[i].nSessionID && m_vUserInfo[i].iState != MINE_S )
			{
				m_vUserInfo[i].iState = iState;
				return;
			}
		}
	}
}

void CDnPVPGameResultAllDlg::RemoveUser( DnActorHandle hUser )
{
	if(m_IsFinalResult)
		return;

	if( hUser )
	{
		for( UINT i = 0; i< m_vUserInfo.size();i++ )
		{
			if( hUser->GetUniqueID() == m_vUserInfo[i].nSessionID )
			{
				m_vUserInfo.erase(m_vUserInfo.begin() + i );
				return;
			}
		}
	}
}

void CDnPVPGameResultAllDlg::FinalResultOpen( )
{
	WCHAR wszTemp[256];
	SecureZeroMemory(wszTemp,sizeof(wszTemp));

	// When FinalResultOpen happens, force-close first,
	// Ignore the bShow argument in subsequent Show calls triggered by user.
	// And control it directly in Process.
	Show(false);
	m_IsFinalResult = true;

	// Switch cursor mode
	CDnMouseCursor::GetInstance().ShowCursor( true, true );
}

void CDnPVPGameResultAllDlg::Show( bool bShow )
{ 
	if( bShow == m_bShow || m_IsFinalResult )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnPVPGameResultAllDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CLOSE" ) )
		{
			SendMovePvPGameToPvPLobby();
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnPVPGameResultAllDlg::SetUserScore( int nSessionID, int nKOCount, int nKObyCount, UINT uiKOP , UINT uiAssistP , UINT uiTotalP )
{
	for( UINT i = 0; i< m_vUserInfo.size();i++ )
	{
		if( nSessionID == m_vUserInfo[i].nSessionID )
		{
			m_vUserInfo[i].uiKOCount = nKOCount;
			m_vUserInfo[i].uiKObyCount = nKObyCount;
			m_vUserInfo[i].uiKOP = uiKOP;
			m_vUserInfo[i].uiTotalScore = uiTotalP;
			m_vUserInfo[i].uiAssist = uiAssistP;
			return;
		}
	}
}

void CDnPVPGameResultAllDlg::InitializeSlot()
{
	for(int iPlayerNum = 0 ; iPlayerNum < PvP_MaxUserSlot; iPlayerNum++)
	{
		m_pTeamLevel[iPlayerNum]->ClearText();
		m_pJobIcon[iPlayerNum]->SetIconID( -1 );
		m_pSlotRank[iPlayerNum]->Show( false );
		m_pGuildMark[iPlayerNum]->Show( false );
		m_pGuildName[iPlayerNum]->ClearText();
		m_pTeamName[iPlayerNum]->ClearText();
		m_pKOCount[iPlayerNum]->ClearText();
		m_pKObyCount[iPlayerNum]->ClearText();
		m_pTotalPoint[iPlayerNum]->ClearText();
		m_pXP[iPlayerNum]->ClearText();
		m_pPCRoom[iPlayerNum]->ClearText();
		m_pMedalIcon[iPlayerNum]->Show( false );
		m_pMedal[iPlayerNum]->ClearText();
		m_pStaticCover[iPlayerNum]->Show( false );
		if(m_bAssistMode && m_pStaticAssist[iPlayerNum]) 
			m_pStaticAssist[iPlayerNum]->ClearText();
	}
}

void CDnPVPGameResultAllDlg::UpdateUser()
{
	WCHAR wszTemp[256];
	SecureZeroMemory(wszTemp,sizeof(wszTemp));

	if( m_vUserInfo.size() > PvP_MaxUserSlot)
	{
		ErrorLog("CDnPVPGameResultAllDlg::UpdateUser() User Num is Wrong");
		return;
	}

	std::sort( m_vUserInfo.begin(), m_vUserInfo.end(), Compare_IndividualTotalScore );

#if defined( PRE_ADD_REVENGE )
	std::sort( m_vUserInfo.begin(), m_vUserInfo.end(), Compare_Individual_RevengeUser );
#endif	// #if defined( PRE_ADD_REVENGE )

	InitializeSlot();

	for( UINT i = 0; i< m_vUserInfo.size(); i++ )
	{
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
		wsprintf(wszTemp, L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), m_vUserInfo[i].nLevel);
#else
		wsprintf(wszTemp, L"LV %d", m_vUserInfo[i].nLevel);
#endif 
		m_pTeamLevel[i]->SetText(wszTemp);
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		if( m_vUserInfo[i].cJobClassID > 0 )
			m_pJobIcon[i]->SetIconID( m_vUserInfo[i].cJobClassID, true );

		//Icon
		int iIconW,iIconH;
		int iU,iV;
		iIconW = GetInterface().GeticonWidth();
		iIconH = GetInterface().GeticonHeight();
		if( GetInterface().ConvertPVPGradeToUV( m_vUserInfo[i].cPVPlevel, iU, iV ))
		{
			m_pSlotRank[i]->SetTexture(GetInterface().GetPVPIconTex(), iU, iV, iIconW, iIconH);
			m_pSlotRank[i]->Show(true);
		}

			if( m_vUserInfo[i].GuildSelfView.IsSet() ) 
		{
			// Guild name
			m_pGuildName[i]->SetText( m_vUserInfo[i].GuildSelfView.wszGuildName );

			// Guild mark
			const TGuildView &GuildView = m_vUserInfo[i].GuildSelfView;
			if( GetGuildTask().IsShowGuildMark( GuildView ) )
			{
				EtTextureHandle hGuildMark = GetGuildTask().GetGuildMarkTexture( GuildView );
				m_pGuildMark[i]->SetTexture( hGuildMark );
				m_pGuildMark[i]->Show( true );
			}
		}

		// Name
		if ( m_vUserInfo[i].iState == Normal_S )
			m_pTeamName[i]->SetTextColor(EtInterface::textcolor::WHITE);
		else if ( m_vUserInfo[i].iState == MINE_S )
			m_pTeamName[i]->SetTextColor(EtInterface::textcolor::GOLD);
		else if ( KILLED_S == m_vUserInfo[i].iState )
			m_pTeamName[i]->SetTextColor(EtInterface::textcolor::RED);

#if defined( PRE_ADD_REVENGE )
		if( MINE_S != m_vUserInfo[i].iState )
		{
			CDnRevengeTask * pRevengeTask = (CDnRevengeTask *)CTaskManager::GetInstance().GetTask( "RevengeTask" );
			if( NULL != pRevengeTask )
				pRevengeTask->GetRevengeUserID( m_vUserInfo[i].nSessionID, m_vUserInfo[i].eRevengeUser );

			if( Revenge::RevengeTarget::eRevengeTarget_Target == m_vUserInfo[i].eRevengeUser )
				m_pTeamName[i]->SetTextColor( EtInterface::textcolor::PVP_REVENGE_TARGET );
			else if( Revenge::RevengeTarget::eRevengeTarget_Me == m_vUserInfo[i].eRevengeUser )
				m_pTeamName[i]->SetTextColor( EtInterface::textcolor::PVP_REVENGE_ME );
			else
				m_pTeamName[i]->SetTextColor( EtInterface::textcolor::WHITE );
		}
#endif	//	#if defined( PRE_ADD_REVENGE )
		m_pTeamName[i]->SetText( m_vUserInfo[i].wszUserName.c_str() );
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		// From kills downward..
		m_pKOCount[i]->SetIntToText(m_vUserInfo[i].uiKOCount);
		m_pKObyCount[i]->SetIntToText(m_vUserInfo[i].uiKObyCount);
		
		if(m_bAssistMode && m_pStaticAssist[i]) 
			m_pStaticAssist[i]->SetIntToText(m_vUserInfo[i].uiAssist);

		wsprintf(wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120315 ),m_vUserInfo[i].uiTotalScore );
		m_pTotalPoint[i]->SetText(wszTemp);
		SecureZeroMemory(wszTemp,sizeof(wszTemp));

		// Show only when XP arrives and is set during FinalResultOpen.
		if( m_IsFinalResult || m_vUserInfo[i].uiXP )
		{
			m_pXP[i]->SetIntToText(m_vUserInfo[i].uiXP);
			m_pPCRoom[i]->ClearText();
			wsprintf(wszTemp,L"X %d",m_vUserInfo[i].uiMedal);
			m_pMedal[i]->SetText(wszTemp);

			// Currently there is only one type of medal.
			m_pMedalIcon[i]->SetTexture( m_hPVPMedalIconImage, 0, 0, PVP_MEDAL_ICON_XSIZE, PVP_MEDAL_ICON_YSIZE );
			m_pMedalIcon[i]->Show(true);
		}

		if( m_vUserInfo[i].iState == MINE_S )
			m_pStaticCover[i]->Show( true );
	}

}

void CDnPVPGameResultAllDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() )
		UpdateUser();

	if( m_IsFinalResult ) {
		m_fTotal_ElapsedTime += fElapsedTime;

		if( m_fTotal_ElapsedTime > static_cast<float>(PvPCommon::Common::PvPFinishScoreOpenDelay) )
		{
			if( !IsShow() ) {
				m_pButtonClose->Show( true );
				CEtUIDialog::Show( true );	// Force Show
			}

			WCHAR wszCloseMessage[256];
			SecureZeroMemory(wszCloseMessage ,sizeof(wszCloseMessage));
			wsprintf(wszCloseMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1,121013), PvPCommon::Common::PvPFinishAutoClose-static_cast<int>(m_fTotal_ElapsedTime) );
			if( m_pButtonClose )
				m_pButtonClose->SetText(wszCloseMessage );
		}

		if( m_fTotal_ElapsedTime > static_cast<float>(PvPCommon::Common::PvPFinishAutoClose) )
		{
			SendMovePvPGameToPvPLobby();
			m_IsFinalResult = false;
		}
	}
}

void CDnPVPGameResultAllDlg::SetPVPXP( UINT nSessionID, UINT uiAddXPScore,UINT uiTotalXPScore ,UINT uiMedalScore)
{
	// Could handle in CDnMutatorGame::EndGame, but since each mode needs separate handling, handle when receiving EXP.
	// The XP packet arrives after the result-window-open packet, and besides, medal Exp is split and calculated from experience anyway.
	int nGainMedalCount = 0;
	int nMedalExp = 0;
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP ) 
	{
		CDnPvPGameTask *pPVPGameTask = static_cast<CDnPvPGameTask*>(pGameTask);
		nMedalExp = pPVPGameTask->GetMedalExp();
		if( pPVPGameTask->GetGameMode() == PvPCommon::GameMode::PvP_Zombie_Survival )
			uiAddXPScore = 0;
	}

	for( UINT i = 0; i< m_vUserInfo.size();i++ )
	{
		if( nSessionID == m_vUserInfo[i].nSessionID )
		{
			m_vUserInfo[i].uiXP = uiAddXPScore;
			m_vUserInfo[i].uiMedal = uiMedalScore;
			m_vUserInfo[i].uiTotalXP = uiTotalXPScore; 

			if( m_vUserInfo[i].iState == MINE_S )
			{
				if( nGainMedalCount && GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG ) ) {
					WCHAR wszTemp[128] = { 0, };
					swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 121069 ), nGainMedalCount );
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp, false );
				}
			}
			return;
		}
	}
	
}


std::wstring CDnPVPGameResultAllDlg::GetBestUserName()
{

	int UserIndex = 0;

	if(!m_vUserInfo.empty())
	{
		for(int i=0;i<(int)m_vUserInfo.size();i++)
		{
			if(m_vUserInfo[i].uiKOCount >= m_nBestUserKillCount)
			{

				if(m_vUserInfo[i].uiKOCount == m_nBestUserKillCount) // If kill count equals the previous best user's,
				{
					if(m_nBestUserSessionID != m_vUserInfo[i].nSessionID) // Session ID is different < it's another person >
					{
						if((int)m_vUserInfo[i].uiKObyCount >= GetBestUserDeathScore()) // If kill count is equal or higher, not the best user
							continue;
					}
				}

				m_nBestUserKillCount = m_vUserInfo[i].uiKOCount;
				m_nBestUserDeathCount = m_vUserInfo[i].uiKObyCount;
				m_nBestUserSessionID = m_vUserInfo[i].nSessionID;
				UserIndex = i;
			}
		}

		return m_vUserInfo[UserIndex].wszUserName;
	}

	
	m_nBestUserKillCount = 0;
	return L"";
}

int CDnPVPGameResultAllDlg::GetBestUserScore()
{
	int nCurrentBestUser = 0;

	if(!m_vUserInfo.empty())
	{
		for(int i=0;i<(int)m_vUserInfo.size();i++)
		{
			if((int)m_vUserInfo[i].uiKOCount >= nCurrentBestUser)
			{
				nCurrentBestUser = m_vUserInfo[i].uiKOCount;
			}
		}
	}
	m_nBestUserKillCount = nCurrentBestUser;

	return m_nBestUserKillCount;
}


int CDnPVPGameResultAllDlg::GetBestUserDeathScore()
{
	int nCurrentBestUserDeathCount = 1000;

	if(!m_vUserInfo.empty())
	{
		for(int i=0;i<(int)m_vUserInfo.size();i++)
		{
			if((int)m_vUserInfo[i].uiKOCount == GetBestUserScore())
			{
				if((int)m_vUserInfo[i].uiKObyCount < nCurrentBestUserDeathCount)
				{
					nCurrentBestUserDeathCount = (int)m_vUserInfo[i].uiKObyCount;
					m_nBestUserSessionID = (int)m_vUserInfo[i].nSessionID;
				}
			}
		}
	}

	m_nBestUserDeathCount = nCurrentBestUserDeathCount;
	return m_nBestUserDeathCount;
}


#ifdef PRE_ADD_PVP_COMBOEXERCISE

// UI settings in combo practice mode.
void CDnPVPGameResultAllDlg::SetComboExerciseMode()
{
	if( m_pBoardName )
		m_pBoardName->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7932 ) );
		
	CEtUIStatic * pStatic = NULL;
	char * strID[16] = { "ID_KILLCOUND", "ID_DEATH", "ID_SCORE", "ID_PVPXP", "ID_PC", "ID_MEDAL" };
	for( int i=0; i<6; ++i )
	{
		pStatic = GetControl<CEtUIStatic>( strID[i] );
		if( pStatic )
			pStatic->Show( false );
	}

	for( int i=0; i<PvP_MaxUserSlot; ++i )
	{
		m_pKOCount[i]->Show( false );
		m_pKObyCount[i]->Show( false );
		m_pTotalPoint[i]->Show( false );
		m_pXP[i]->Show( false );
		m_pPCRoom[i]->Show( false );
		m_pMedalIcon[i]->Show( false );
		m_pMedal[i]->Show( false );
	}

}
#endif // PRE_ADD_PVP_COMBOEXERCISE
