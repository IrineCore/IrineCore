#include "StdAfx.h"
#include "DnIndividualRoundModeHUD.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnIndividualRoundModeHUD::CDnIndividualRoundModeHUD( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnPVPBaseHUD( dialogType, pParentDialog, nID, pCallback )
#ifdef PRE_MOD_PVPOBSERVER	
, m_bFirstObserver( false )
#endif // PRE_MOD_PVPOBSERVER
{
	for(int i=0; i<PvPCommon::s_iMaxRoundNum ;i++)
	{
		m_pRoundMark[i] = NULL;
		m_pRoundEmptyMark[i] = NULL;
	}

	m_pStatic_TeamName_A = NULL;
	m_pStatic_TeamName_B = NULL;

	m_iRoundCount = 0;
	m_nStageNum = 0;
}

CDnIndividualRoundModeHUD::~CDnIndividualRoundModeHUD(void)
{
}

void CDnIndividualRoundModeHUD::SetStage( int iStageNum )
{
	m_nStageNum = iStageNum;

	if( iStageNum > PvPCommon::s_iMaxRoundNum )
	{		
		ErrorLog("CDnRoundModeHUD::SetStage :: stage num is wrong!");
		return;
	}

	for(int iNum = 0 ; iNum < PvPCommon::s_iMaxRoundNum  ; iNum++)
	{
		m_pRoundMark[iNum]->Show(false);
		m_pRoundEmptyMark[iNum] ->Show(false);
	}

	int nLeftControlNumber = 0;
	nLeftControlNumber = (PvPCommon::s_iMaxRoundNum - m_nStageNum) / 2;

	for(int iNum = nLeftControlNumber ; iNum < PvPCommon::s_iMaxRoundNum - nLeftControlNumber ; iNum++)
	{
		m_pRoundMark[iNum]->Show(false);
		m_pRoundEmptyMark[iNum] ->Show(true);
	}
}

#ifdef PRE_MOD_PVPOBSERVER
void CDnIndividualRoundModeHUD::SetWinStage( int iMyTeamVicNum , int iEnemyTeamVicNum, bool bZombie )
#else
void CDnIndividualRoundModeHUD::SetWinStage( int iMyTeamVicNum , int iEnemyTeamVicNum )
#endif // PRE_MOD_PVPOBSERVER
{
	m_iRoundCount = iMyTeamVicNum + iEnemyTeamVicNum;

	int nLeftControlNumber = 0;
	nLeftControlNumber = (PvPCommon::s_iMaxRoundNum - m_nStageNum) / 2;

	for(int iNum = nLeftControlNumber ; iNum < PvPCommon::s_iMaxRoundNum - nLeftControlNumber ; iNum++)
	{
		if( iNum < m_iRoundCount + nLeftControlNumber )
			m_pRoundMark[iNum] ->Show(true);
	}

#ifdef PRE_MOD_PVPOBSERVER
	if( IsShow() && m_bFirstObserver )
	{		
		int uiIndex_1 = 7856; // "Order Dragon"
		int uiIndex_2 = 7857; // "Chaos Dragon"

		if( bZombie )
		{
			uiIndex_1 = 120085; // "Human"
			uiIndex_2 = 120084; // "Ghoul"
		}
		if( SetTextObserverTeam( m_pStatic_TeamName_A, m_pStatic_TeamName_B, uiIndex_1, uiIndex_2 ) ) // "Order Dragon", "Chaos Dragon"	
			m_bFirstObserver = false;
	}
#endif // PRE_MOD_PVPOBSERVER

}


void CDnIndividualRoundModeHUD::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpRound_AllDlg.ui" ).c_str(), bShow );
}


void CDnIndividualRoundModeHUD::Process( float fElapsedTime )
{
	CDnPVPBaseHUD::Process(fElapsedTime);
}


void CDnIndividualRoundModeHUD::InitialUpdate()
{
	CDnPVPBaseHUD::InitialUpdate();

	char szUIName[100];
	SecureZeroMemory(szUIName,sizeof(szUIName));

	for(int iStageNum = 0 ; iStageNum < PvPCommon::s_iMaxRoundNum ; iStageNum++)
	{
		sprintf(szUIName,"ID_ALL_WIN%d",iStageNum);
		m_pRoundMark[iStageNum] = GetControl<CEtUIStatic>(szUIName);
		m_pRoundMark[iStageNum]->Show(false);
	
		sprintf(szUIName,"ID_ALL_STAGE%d",iStageNum);
		m_pRoundEmptyMark[iStageNum] = GetControl<CEtUIStatic>(szUIName);
		m_pRoundEmptyMark[iStageNum]->Show(false);
	}	
	m_pRoomName = GetControl<CEtUIStatic>("ID_ROOMNAME");

	m_pStatic_TeamName_A = GetControl<CEtUIStatic>("ID_PVP_RESPAWN_MYTEAM");
	m_pStatic_TeamName_B = GetControl<CEtUIStatic>("ID_PVP_RESPAWN_ENEMY");

#ifdef PRE_MOD_PVPOBSERVER
	m_bFirstObserver = true;
#endif // PRE_MOD_PVPOBSERVER

}

void CDnIndividualRoundModeHUD::ChangeTeamName(int nMyTeamUIString,int nEnemyTeamUIString)
{
	m_pStatic_TeamName_A->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nMyTeamUIString));
	m_pStatic_TeamName_B->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nEnemyTeamUIString));
}