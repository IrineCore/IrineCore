#include "StdAfx.h"
#include "DnSkillStoreTabDlg.h"
#include "DnSkillStoreDlg.h"
#include "DnTradeTask.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSkillStoreTabDlg::CDnSkillStoreTabDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUITabDialog( dialogType, pParentDialog, nID, pCallback, true )
	, m_pActiveSkillTabButton(NULL)
	, m_pPassiveSkillTabButton(NULL)
	, m_pActiveSkillStoreDlg(NULL)
	, m_pPassiveSkillStoreDlg(NULL)
{
}

CDnSkillStoreTabDlg::~CDnSkillStoreTabDlg(void)
{
}

void CDnSkillStoreTabDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillStoreTabDlg.ui" ).c_str(), bShow );

	if( CDnTradeTask::IsActive() )
	{
		GetTradeTask().GetTradeSkill().SetSkillStoreDialog( this );
	}
	else
	{
		CDebugSet::ToLogFile( "CDnMailDlg::Initialize, Trade task must be created before skill store dialog is created." );
	}
}

void CDnSkillStoreTabDlg::InitialUpdate()
{
	m_pActiveSkillTabButton = GetControl<CEtUIRadioButton>("ID_TAB_ACTIVE");
	m_pActiveSkillStoreDlg = new CDnSkillStoreDlg( UI_TYPE_CHILD, this );
	m_pActiveSkillStoreDlg->Initialize( false );
	AddTabDialog( m_pActiveSkillTabButton, m_pActiveSkillStoreDlg );

	m_pPassiveSkillTabButton = GetControl<CEtUIRadioButton>("ID_TAB_PASSIVE");
	m_pPassiveSkillStoreDlg = new CDnSkillStoreDlg( UI_TYPE_CHILD, this );
	m_pPassiveSkillStoreDlg->Initialize( false );
	AddTabDialog( m_pPassiveSkillTabButton, m_pPassiveSkillStoreDlg );

	SetCheckedTab( m_pActiveSkillTabButton->GetTabID() );
}

void CDnSkillStoreTabDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUITabDialog::Show( bShow );
//	CDnMouseCursor::GetInstance().ShowCursor( bShow );

	if( bShow )
	{
		UpdateSkillTabButton();
	}
	else
	{
		if( CDnTradeTask::IsActive() )
		{
			//GetTradeTask().GetTradeSkill().DeleteSkillStore();
		}

		GetInterface().CloseBlind();
	}
}

void CDnSkillStoreTabDlg::SetItem( int nSlotIndex, MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnSkillStoreTabDlg::SetItem, pItem is NULL!");

	CDnSkillStoreDlg *pSkillDlg(NULL);
	CDnSkill *pSkill = static_cast<CDnSkill*>(pItem);

	switch( pSkill->GetSkillType() )
	{
	case CDnSkill::Active:
		pSkillDlg = static_cast<CDnSkillStoreDlg*>(m_groupTabDialog.GetDialog(m_pActiveSkillTabButton->GetTabID()));
		break;
	case CDnSkill::Passive:
		pSkillDlg = static_cast<CDnSkillStoreDlg*>(m_groupTabDialog.GetDialog(m_pPassiveSkillTabButton->GetTabID()));
		break;
	default:
		ASSERT(0&&"CDnSkillStoreTabDlg::SetItem");
		break;
	}

	if( pSkillDlg )
	{
		pSkillDlg->SetItem( nSlotIndex, pItem );
	}
}

void CDnSkillStoreTabDlg::ResetAllItem()
{
	m_pActiveSkillStoreDlg->ResetAllItem();
	m_pPassiveSkillStoreDlg->ResetAllItem();
}

void CDnSkillStoreTabDlg::OnRefreshSlot()
{
	if( IsShow() )
	{
		m_pActiveSkillStoreDlg->UpdateSkillISlot();
		m_pPassiveSkillStoreDlg->UpdateSkillISlot();
	}
}

void CDnSkillStoreTabDlg::OnRefreshDialog()
{
	if( IsShow() )
	{
		m_pActiveSkillStoreDlg->RefreshDialog();
		m_pPassiveSkillStoreDlg->RefreshDialog();

		UpdateSkillTabButton();
	}
}

void CDnSkillStoreTabDlg::UpdateSkillTabButton()
{
	if( !m_pActiveSkillStoreDlg->IsEmptySlotDialog() )
		m_pActiveSkillTabButton->Show( true );
	else
		m_pActiveSkillTabButton->Show( false );

	if( !m_pPassiveSkillStoreDlg->IsEmptySlotDialog() )
		m_pPassiveSkillTabButton->Show( true );
	else
		m_pPassiveSkillTabButton->Show( false );

	if( !m_pActiveSkillTabButton->IsShow() || !m_pPassiveSkillTabButton->IsShow() )
	{
		CEtUIRadioButton *pButton(NULL);

		if( m_pActiveSkillTabButton->IsShow() )
		{
			pButton = m_pActiveSkillTabButton;
		}
		else if( m_pPassiveSkillTabButton->IsShow() )
		{
			pButton = m_pPassiveSkillTabButton;
		}

		if( pButton )
		{
			SetCheckedTab( pButton->GetTabID() );
		}
	}
}