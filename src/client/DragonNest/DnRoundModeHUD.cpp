#include "StdAfx.h"
#include "DnRoundModeHUD.h"
#include "TaskManager.h"
#include "DnPvPGameTask.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnRoundModeHUD::CDnRoundModeHUD( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnPVPBaseHUD( dialogType, pParentDialog, nID, pCallback )
{
	for(int iNum = 0 ; iNum < PvPCommon::s_iMaxRoundNum ; iNum++)
	{
		m_pMyTeamWinMark[iNum]= NULL;
		m_pMyTeamEmptyMark[iNum] = NULL;
		m_pEnemyTeamWinMark[iNum]= NULL;
		m_pEnemyTeamEmptyMark[iNum] = NULL;
	}

	m_iMyTeamVicNum = 0;
	m_iEnemyTeamVicNum = 0;	
	m_pKillMark = NULL;

#ifdef PRE_ADD_PVP_DUAL_INFO
	m_fDualInfoDelayTime = -1.f;
#endif

#ifdef PRE_MOD_PVPOBSERVER
	m_pStaticFriendly = m_pStaticEnemy = NULL;
#endif // PRE_MOD_PVPOBSERVER
}

CDnRoundModeHUD::~CDnRoundModeHUD(void)
{

}

void CDnRoundModeHUD::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpRoundDlg.ui" ).c_str(), bShow );
}


void CDnRoundModeHUD::InitialUpdate()
{
	CDnPVPBaseHUD::InitialUpdate();

	m_pRoomName = GetControl<CEtUIStatic>("ID_ROOMNAME");

#ifdef PRE_MOD_PVPOBSERVER
	m_pStaticFriendly = GetControl< CEtUIStatic >( "ID_STATIC21" );
	m_pStaticEnemy = GetControl< CEtUIStatic >( "ID_STATIC22" );
	m_bFirstObserver = true;
#endif // PRE_MOD_PVPOBSERVER

	char szUIMYWin[100],szUIMYEmpty[100],szUIEnemyWin[100],szUIEnemyEmpty[100];

	SecureZeroMemory(szUIMYWin,sizeof(szUIMYWin));
	SecureZeroMemory(szUIMYEmpty,sizeof(szUIMYEmpty));
	SecureZeroMemory(szUIEnemyWin,sizeof(szUIEnemyWin));
	SecureZeroMemory(szUIEnemyEmpty,sizeof(szUIEnemyEmpty));

	for(int iStageNum = 0 ; iStageNum < PvPCommon::s_iMaxRoundNum ; iStageNum++)
	{
		sprintf(szUIMYWin,"ID_BLUE_WIN%d",iStageNum);
		sprintf(szUIMYEmpty,"ID_BLUE_STAGE%d",iStageNum);
		sprintf(szUIEnemyWin,"ID_RED_WIN%d",iStageNum);
		sprintf(szUIEnemyEmpty,"ID_RED_STAGE%d",iStageNum);

		m_pMyTeamWinMark[iStageNum] = GetControl<CEtUIStatic>(szUIMYWin); 
		m_pMyTeamEmptyMark[iStageNum] = GetControl<CEtUIStatic>(szUIMYEmpty);
		m_pEnemyTeamWinMark[iStageNum] = GetControl<CEtUIStatic>(szUIEnemyWin);
		m_pEnemyTeamEmptyMark[iStageNum] = GetControl<CEtUIStatic>(szUIEnemyEmpty);

		m_pMyTeamWinMark[iStageNum]->Show(false);
		m_pMyTeamEmptyMark[iStageNum]->Show(false);
		m_pEnemyTeamWinMark[iStageNum]->Show(false);
		m_pEnemyTeamEmptyMark[iStageNum]->Show(false);
	}	

#ifdef PRE_ADD_PVP_DUAL_INFO
	char *szTeam[2] = {"BLUE","RED"};

	m_sContinuousDualInfo.pStaticVSMark = GetControl<CEtUIStatic>("ID_STATIC_VS0");
	m_sImpactDualInfo.pStaticVSMark = GetControl<CEtUIStatic>("ID_STATIC_VS1");

	for(int i=0; i<PvPCommon::TeamIndex::Max; i++)
	{
		m_sContinuousDualInfo.pStaticUserName[i] = GetControl<CEtUIStatic>(FormatA("ID_TEXT_%sNAME0",szTeam[i]).c_str());
		m_sImpactDualInfo.pStaticUserName[i] = GetControl<CEtUIStatic>(FormatA("ID_TEXT_%sNAME1",szTeam[i]).c_str());

		m_sContinuousDualInfo.pUIJobIcon[i] = GetControl<CDnJobIconStatic>(FormatA("ID_STATIC_%sCLASS0",szTeam[i]).c_str());
		m_sImpactDualInfo.pUIJobIcon[i] = GetControl<CDnJobIconStatic>(FormatA("ID_STATIC_%sCLASS1",szTeam[i]).c_str());
	}
#endif

}

void CDnRoundModeHUD::Process( float fElapsedTime )
{
	CDnPVPBaseHUD::Process( fElapsedTime );
	ProcessDualCount( fElapsedTime );
}


#ifdef PRE_ADD_PVP_DUAL_INFO
void CDnRoundModeHUD::ProcessDualCount( float fElapsedTime )
{
	if( m_fDualInfoDelayTime > 0.f )
	{
		m_fDualInfoDelayTime -=fElapsedTime;

		if( m_fDualInfoDelayTime > 3.f )
		{
			m_sImpactDualInfo.Show( true );
			CDnLocalPlayerActor::LockInput(true);
		}
		else
		{
			m_sImpactDualInfo.Show( false );
			CDnLocalPlayerActor::LockInput(false);
		}
	}
	else if( m_fDualInfoDelayTime <= 0.f && m_fDualInfoDelayTime != -1.f )
	{
		m_sContinuousDualInfo.Show(true);
		m_sImpactDualInfo.Show(false);
		m_fDualInfoDelayTime = -1.f;
	}

	if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer && m_sContinuousDualInfo.IsShow() == false )
		ShowContinouosInfoForBreakInto();

#ifdef PRE_MOD_PVPOBSERVER
	if( IsShow() && m_bFirstObserver )
	{		
		if( SetTextObserverTeam( m_pStaticFriendly, m_pStaticEnemy, 7856, 7857 ) ) // "Order Dragon", "Chaos Dragon"	
			m_bFirstObserver = false;
	}
#endif // PRE_MOD_PVPOBSERVER

}
#endif

void CDnRoundModeHUD::ResetStage()
{
	for(int iNum = 0 ; iNum < PvPCommon::s_iMaxRoundNum ; iNum++)
	{
		m_pMyTeamEmptyMark[iNum]->Show( false );
		m_pMyTeamWinMark[iNum] ->Show( false );

		m_pEnemyTeamEmptyMark[iNum]->Show( false );
		m_pEnemyTeamWinMark[iNum]->Show( false );
	}	

}

void CDnRoundModeHUD::SetStage( int iStageNum )
{
	if( iStageNum > PvPCommon::s_iMaxRoundNum )
	{		
		ErrorLog("CDnRoundModeHUD::SetStage :: stage num is wrong!");
		return;
	}

	for(int iNum = 0 ; iNum < PvPCommon::s_iMaxRoundNum ; iNum++)
	{
		m_pMyTeamEmptyMark[iNum]->Show(iNum < iStageNum ? true : false );
		m_pMyTeamWinMark[iNum] ->Show(false);

		m_pEnemyTeamEmptyMark[iNum]->Show(iNum < iStageNum ? true : false);
		m_pEnemyTeamWinMark[iNum]->Show(false);
	}
}

void CDnRoundModeHUD::SetWinStage( int iMyTeamVicNum , int iEnemyTeamVicNum  )
{
#ifdef PRE_MOD_PVPOBSERVER
	if( IsShow() && m_bFirstObserver )
	{		
		if( SetTextObserverTeam( m_pStaticFriendly, m_pStaticEnemy, 7856, 7857 ) ) // "Order Dragon", "Chaos Dragon"	
			m_bFirstObserver = false;
	}
#endif // PRE_MOD_PVPOBSERVER

	m_iMyTeamVicNum = iMyTeamVicNum;
	
	m_iEnemyTeamVicNum = iEnemyTeamVicNum;

	for(int iNum = 0 ; iNum < PvPCommon::s_iMaxRoundNum ; iNum++)
	{
		if( iNum < m_iMyTeamVicNum )
		{
			m_pMyTeamWinMark[iNum] ->Show(true);
		}		
	}

	for(int iNum = 0 ; iNum < PvPCommon::s_iMaxRoundNum ; iNum++)
	{
		if( iNum < m_iEnemyTeamVicNum  )
		{
			m_pEnemyTeamWinMark[iNum]->Show(true);
		}		
	}
}

#ifdef PRE_ADD_PVP_DUAL_INFO

void CDnRoundModeHUD::OrderShowDualInfo(  bool bBreakInto )
{
	if(bBreakInto)
		m_sContinuousDualInfo.Show(true);
	else
		m_fDualInfoDelayTime = 6.5f;
}

void CDnRoundModeHUD::ClearDualInfo()
{
	m_sImpactDualInfo.Show(false);
	m_sContinuousDualInfo.Show(false);
}

#ifdef PRE_ADD_PVP_TOURNAMENT
void CDnRoundModeHUD::ForceShowOff_ContinuousDualInfo(bool bShowOff)
{
	m_sContinuousDualInfo.SetForceShowOff(bShowOff);
}
#endif

void CDnRoundModeHUD::SetDualInfo( const WCHAR *blueName , int blueJobIndex , const WCHAR *redName , int redJobIndex )
{
	m_sContinuousDualInfo.pStaticUserName[PvPCommon::TeamIndex::A]->SetText(blueName);
	m_sContinuousDualInfo.pStaticUserName[PvPCommon::TeamIndex::B]->SetText(redName);
	
	m_sContinuousDualInfo.pUIJobIcon[PvPCommon::TeamIndex::A]->SetIconID(blueJobIndex);
	m_sContinuousDualInfo.pUIJobIcon[PvPCommon::TeamIndex::B]->SetIconID(redJobIndex);

	m_sImpactDualInfo.pStaticUserName[PvPCommon::TeamIndex::A]->SetText(blueName);
	m_sImpactDualInfo.pStaticUserName[PvPCommon::TeamIndex::B]->SetText(redName);

	m_sImpactDualInfo.pUIJobIcon[PvPCommon::TeamIndex::A]->SetIconID(blueJobIndex);
	m_sImpactDualInfo.pUIJobIcon[PvPCommon::TeamIndex::B]->SetIconID(redJobIndex);
}

void CDnRoundModeHUD::ShowContinouosInfoForBreakInto()
{
	CDnPvPGameTask* pGameTask = (CDnPvPGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );

	if( pGameTask && ( pGameTask->GetLadderType() == LadderSystem::MatchType::_1vs1 ) )
	{
		DnActorHandle hBlueActor;
		DnActorHandle hRedActor;

		for( int itr = 0; itr < (int)CDnActor::s_pVecProcessList.size(); ++itr )
		{
			if( CDnActor::s_pVecProcessList[itr] && CDnActor::s_pVecProcessList[itr]->IsPlayerActor() )
			{
				if( CDnActor::s_pVecProcessList[itr]->GetTeam() == PvPCommon::Team::A )
					hBlueActor = CDnActor::s_pVecProcessList[itr]->GetActorHandle();

				if( CDnActor::s_pVecProcessList[itr]->GetTeam() == PvPCommon::Team::B )
					hRedActor = CDnActor::s_pVecProcessList[itr]->GetActorHandle();
			}
		}

		if( hBlueActor && hRedActor )
		{
			if(CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::B ) // If Team B 
			{
				SetDualInfo(
					hRedActor->GetName(),
					hRedActor->OnGetJobClassID(),
					hBlueActor->GetName(),
					hBlueActor->OnGetJobClassID() );
			}
			else // Applies to Observer Team A, etc.
			{
				SetDualInfo(
					hBlueActor->GetName(),
					hBlueActor->OnGetJobClassID(),
					hRedActor->GetName(),
					hRedActor->OnGetJobClassID() );
			}

			m_sContinuousDualInfo.Show(true);
		}
	}
}

#endif
