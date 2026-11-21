#include "StdAfx.h"
#include "DnDungeonExpectDlg.h"
#include "DnDungeonExpectRewardDlg.h"
#include "DnDungeonExpectMissionDlg.h"
#include "DnDungeonExpectQuestDlg.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 


CDnDungeonExpectDlg::CDnDungeonExpectDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUITabDialog( dialogType, pParentDialog, nID, pCallback )
, m_pExpectRewardTab( NULL )
, m_pExpectQuestTab( NULL )
, m_pExpectMissionTab( NULL )
, m_pDnDungeonExpectRewardDlg( NULL )
, m_pDnDungeonExpectMissionDlg( NULL )
, m_pDnDungeonExpectQuestDlg( NULL )
, m_nMapIndex( 0 )
, m_nDifficult( 0 )
{
}

CDnDungeonExpectDlg::~CDnDungeonExpectDlg()
{
}

void CDnDungeonExpectDlg::Initialize( bool bShow )
{ 
	CEtUITabDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterTabDlg.ui" ).c_str(), bShow );
}

void CDnDungeonExpectDlg::InitialUpdate()
{
	CEtUITabDialog::InitialUpdate();

	m_pExpectRewardTab = GetControl<CEtUIRadioButton>( "ID_RBT_REWARD" );
	m_pDnDungeonExpectRewardDlg = new CDnDungeonExpectRewardDlg( UI_TYPE_CHILD, this );
	m_pDnDungeonExpectRewardDlg->Initialize( false );
	AddTabDialog( m_pExpectRewardTab, m_pDnDungeonExpectRewardDlg );

	m_pExpectQuestTab = GetControl<CEtUIRadioButton>( "ID_RBT_QUEST" );
	m_pDnDungeonExpectQuestDlg = new CDnDungeonExpectQuestDlg( UI_TYPE_CHILD, this );
	m_pDnDungeonExpectQuestDlg->Initialize( false );
	AddTabDialog( m_pExpectQuestTab, m_pDnDungeonExpectQuestDlg );

	m_pExpectMissionTab = GetControl<CEtUIRadioButton>( "ID_RBT_MISSION" );
	m_pDnDungeonExpectMissionDlg = new CDnDungeonExpectMissionDlg( UI_TYPE_CHILD, this );
	m_pDnDungeonExpectMissionDlg->Initialize( false );
	AddTabDialog( m_pExpectMissionTab, m_pDnDungeonExpectMissionDlg );

	SetCheckedTab( m_pExpectRewardTab->GetTabID() );

	WCHAR wszText[128] = {0,};
	CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_RBT_QUEST" );
	if( pButton )
	{
		swprintf_s( wszText, _countof( wszText ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3256 ), 0 );
		pButton->SetText( wszText );
	}

	pButton = GetControl<CEtUIButton>( "ID_RBT_MISSION" );
	if( pButton )
	{
		swprintf_s( wszText, _countof( wszText ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3257 ), 0 );
		pButton->SetText( wszText );
	}
}

void CDnDungeonExpectDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	CEtUITabDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDungeonExpectDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		m_nMapIndex = 0;
		m_nDifficult = 0;
	}

	CEtUITabDialog::Show( bShow );
}

bool CDnDungeonExpectDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	return CEtUITabDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnDungeonExpectDlg::UpdateDungeonExpectReward( int nMapIndex, int nDifficult )
{
	m_nMapIndex = nMapIndex;
	m_nDifficult = nDifficult;

	SetCheckedTab( m_pExpectRewardTab->GetTabID() );

	WCHAR wszText[128] = {0,};
	if( m_pDnDungeonExpectQuestDlg )
	{
		int nPlayableQuestNumInCurrentMap = m_pDnDungeonExpectQuestDlg->LoadQuestInCurrentMap( nMapIndex, nDifficult );
		CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_RBT_QUEST" );
		if( pButton )
		{
			swprintf_s( wszText, _countof( wszText ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3256 ), nPlayableQuestNumInCurrentMap );
			pButton->SetText( wszText );
		}
	}

	if( m_pDnDungeonExpectMissionDlg )
	{
		int nPlayableMissionNumInCurrentMap = m_pDnDungeonExpectMissionDlg->LoadMissionInCurrentMap( nMapIndex, nDifficult );
		CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_RBT_MISSION" );
		if( pButton )
		{
			swprintf_s( wszText, _countof( wszText ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3257 ), nPlayableMissionNumInCurrentMap );
			pButton->SetText( wszText );
		}
	}

	m_pDnDungeonExpectRewardDlg->UpdateDungeonExpectReward( nMapIndex, nDifficult );
}
