#include"stdafx.h"
#include "DnGoldNoReservationDlg.h"
#include "DnSkillTreeDlg.h"
#include "DnSkillTask.h"
#include "DnItemTask.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

CDnReservationGoldNoDlg::CDnReservationGoldNoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallBack )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallBack )
, m_pSPText( NULL )
, m_pGoldText( NULL )
, m_pSilverText( NULL )
, m_pBronzeText( NULL )
, m_pSkillTreeDlg( NULL )
, m_nSP( 0 )
{

}

CDnReservationGoldNoDlg::~CDnReservationGoldNoDlg()
{

}

void CDnReservationGoldNoDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillSubLevelUpDlg.ui" ).c_str(), bShow );
}

void CDnReservationGoldNoDlg::InitialUpdate()
{
	m_pSPText = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_OK") );
}

void CDnReservationGoldNoDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( true == bShow )
	{
		m_SmartMove.MoveCursor();
		SetInfo();
	}
	else
	{
		m_SmartMove.ReturnCursor();
		m_pSPText->ClearText();
	}

	CEtUIDialog::Show( bShow );
}

void CDnReservationGoldNoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) )
		{
			if( m_nSP > GetSkillTask().GetSkillPoint() )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1147) );		// UISTRING : Insufficient SP.
				GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1147) );	// UISTRING : Insufficient SP.
				return;
			}
			else
			{
				if( m_pSkillTreeDlg )
					m_pSkillTreeDlg->ApplyButtonCommand();
			}
			Show( false );
		}
		else if( IsCmdControl( "ID_CANCEL" ) )
		{
			Show( false );
		}
	}
}

void CDnReservationGoldNoDlg::SetSkillTreeDlg( CDnSkillTreeDlg * pSkillTreeDlg )
{
	m_pSkillTreeDlg = pSkillTreeDlg;
}

void CDnReservationGoldNoDlg::SetInfo()
{
	m_nSP = GetSkillTask().GetReservationSkillNeedSP();

	m_pSPText->SetIntToText( m_nSP );
}

#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )