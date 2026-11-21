#include "StdAfx.h"
#include "DnOptionTabDlg.h"
#include "DnInterface.h"
#include "DnMessageBox.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnOptionTabDlg::CDnOptionTabDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor) 
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
{
	m_bShowChangeMsg = true;
	m_dwPrevTabID = 0;
}

CDnOptionTabDlg::~CDnOptionTabDlg()
{

}

void CDnOptionTabDlg::AddTabDialog( CEtUIControl *pControl, CDnOptionDlg *pDialog )
{
	AddTabDialog( pControl, pDialog, pDialog );
}

void CDnOptionTabDlg::AddTabDialog( CEtUIControl *pControl, CDnOptionTabDlg *pDialog )
{
	AddTabDialog( pControl, pDialog, pDialog );
}

void CDnOptionTabDlg::AddTabDialog( CEtUIControl *pControl, CEtUIDialog *pDialog, CDnOptionBase *pOptionBase )
{
	ASSERT(pControl&&pDialog&&"CEtUIDialog::AddTabDialog");

	if (pControl->GetType() != UI_CONTROL_RADIOBUTTON)
	{
		_ASSERT(0);
		return;
	}

	CEtUIRadioButton* pRadioBtn = static_cast<CEtUIRadioButton*>(pControl);

	int tabID = pRadioBtn->GetTabID();

#if defined (_DEBUG) || defined (_RDEBUG)
	std::vector< boost::tuple<DWORD, CEtUIControl*, CDnOptionBase*> >::const_iterator iter = m_vecTabControl.begin();
	for(;iter != m_vecTabControl.end(); ++iter)
	{
		const  boost::tuple<DWORD, CEtUIControl*, CDnOptionBase*> &id_ctl_pair = *iter;
		if (id_ctl_pair.get<0>() == tabID)
		{
			_ASSERT(0 && "TabDialog ID already exists!");
			break;
		}
	}
#endif

	m_vecTabControl.push_back( boost::make_tuple(tabID , pControl, pOptionBase) );
	m_groupTabDialog.AddDialog( static_cast<CEtUIRadioButton*>(pControl)->GetTabID(), pDialog );
}

void CDnOptionTabDlg::SetCheckedTab( DWORD dwControlID )
{
	int nVecSize = (int)m_vecTabControl.size();

	for( int i=0; i<nVecSize; ++i )
	{
		boost::tuple<DWORD, CEtUIControl*, CDnOptionBase*> &id_ctl_pair = m_vecTabControl[i];

		if( id_ctl_pair.get<0>() == dwControlID )
		{
			static_cast<CEtUIRadioButton*>(id_ctl_pair.get<1>())->SetChecked(true);
			return;
		}
	}
}

DWORD CDnOptionTabDlg::GetCurrentTabID()
{
	int nVecSize = (int)m_vecTabControl.size();

	for( int i=0; i<nVecSize; ++i )
	{
		boost::tuple<DWORD, CEtUIControl*, CDnOptionBase*> &id_ctl_pair = m_vecTabControl[i];
		if( static_cast<CEtUIRadioButton*>(id_ctl_pair.get<1>())->IsChecked() )
		{
			return id_ctl_pair.get<0>();
		}
	}

	return UINT_MAX;
}

CDnOptionBase* CDnOptionTabDlg::GetOptionBase( DWORD dwTabID )
{
	int nVecSize = (int)m_vecTabControl.size();
	for( int i=0; i<nVecSize; ++i )
	{
		boost::tuple<DWORD, CEtUIControl*, CDnOptionBase*> &id_ctl_pair = m_vecTabControl[i];
		if( id_ctl_pair.get<0>() == dwTabID )
		{
			return id_ctl_pair.get<2>();
		}
	}
	return NULL;
}

//const CDnOptionTabDlg* CEtUITabDialog::GetCurrentTabControl() const
//{
//	int nVecSize = (int)m_vecTabControl.size();
//	for( int i=0; i<nVecSize; ++i ) {
//		const boost::tuple<DWORD, CEtUIControl*, CDnOptionBase*> &id_ctl_pair = m_vecTabControl[ i ];
//		const CEtUIRadioButton* pBtn = static_cast<const CEtUIRadioButton*>(id_ctl_pair.get<1>());
//		if ( pBtn && pBtn->IsChecked() )
//			return pBtn;
//	}
//	return NULL;
//}


void CDnOptionTabDlg::Show( bool bShow )
{
	if( bShow ) {
		m_dwPrevTabID = GetFrontTabID();
		SetCheckedTab( GetFrontTabID() );
	}
	else {
		for( int i = 0; i < m_groupTabDialog.GetDialogCount(); i++) {
			CEtUIDialog *pDialog = m_groupTabDialog.GetDialogFromIndex( i );
			if( pDialog ) {
				pDialog->Show( false );
			}			 
		}
		CancelOption();
		ImportSetting();
	}

	CEtUIDialog::Show( bShow );

	DWORD dwDlgID(UINT_MAX);

	if( bShow )
	{
		dwDlgID = GetCurrentTabID();
	}
	else
	{
		dwDlgID = m_groupTabDialog.GetShowDialogID();
	}

	if( dwDlgID != UINT_MAX )
	{
		m_groupTabDialog.ShowDialog( dwDlgID, bShow );
	}
}

int CDnOptionTabDlg::GetFrontTabID()
{
	if( m_vecTabControl.empty() ) {
		return -1;
	}
	return m_vecTabControl.front().get<0>();
}

void CDnOptionTabDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_APPLY" ) )
		{
			ExportSetting();
		}
		if( IsCmdControl("ID_BUTTON_CANCEL" ) )
		{
			CancelOption();
			ImportSetting();
		}
	}
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED && bTriggeredByUser )
	{
		// Actually, simply clicking the currently active tab button still triggers a radio button change event.
		// So check if they differ.
		if( m_dwPrevTabID != GetCurrentTabID() ) {
			if( m_bShowChangeMsg ) {
				CDnOptionBase *pOptionBase = GetOptionBase( m_dwPrevTabID );
				// Get the current dialog,
				if( pOptionBase && pOptionBase->IsChanged() ) {		// Since it is only shown once,																					
					m_bShowChangeMsg = false;													// Restore the selected tab button to its original state,
					SetCheckedTab( m_dwPrevTabID );											// Message box 3170. Show once.
					GetInterface().MessageBox( 3170 );
					return;
				}
			}
			else
			{
				m_bShowChangeMsg = true;
			}
		}
		m_groupTabDialog.ShowDialog( static_cast<CEtUIRadioButton*>(pControl)->GetTabID(), true );	
		m_dwPrevTabID = GetCurrentTabID();
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnOptionTabDlg::ImportSetting()
{
	CDnOptionBase *pOptionBase = GetOptionBase( GetCurrentTabID() );
	if( pOptionBase ) pOptionBase->ImportSetting();
}

void CDnOptionTabDlg::ExportSetting()
{
	CDnOptionBase *pOptionBase = GetOptionBase( GetCurrentTabID() );
	if( pOptionBase ) pOptionBase->ExportSetting();
}

bool CDnOptionTabDlg::IsChanged()
{
	CDnOptionBase *pOptionBase = GetOptionBase( GetCurrentTabID() );
	if( pOptionBase )
		return pOptionBase->IsChanged();
	return false;
}