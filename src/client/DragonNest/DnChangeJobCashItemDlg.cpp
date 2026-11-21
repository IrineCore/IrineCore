#include "StdAfx.h"
#include "DnChangEJobCashItemDlg.h"
#include "DnItemTask.h"
#include "DnWorld.h"
#include "DnItem.h"
#include "DnTableDB.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef PRE_ADD_CHANGEJOB_CASHITEM

CDnChangeJobCashItemDlg::CDnChangeJobCashItemDlg( UI_DIALOG_TYPE dialogType /*= UI_TYPE_FOCUS*/, CEtUIDialog *pParentDialog /*= NULL*/, int nID /*= -1*/, CEtUICallback *pCallback /*= NULL*/ )
												: CEtUIDialog( dialogType, pParentDialog, nID, pCallback ),
												  m_pJobListBox( NULL ),
												  m_pJobExplainTextBox( NULL ),
												  m_pChangeJobOKButton( false ),
												  m_iSelectedJobID( 0 ),
												  m_pChangeJobCashItem( NULL )
{

}

CDnChangeJobCashItemDlg::~CDnChangeJobCashItemDlg( void )
{

}

void CDnChangeJobCashItemDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "JobReselectDlg.ui" ).c_str(), bShow );
}

void CDnChangeJobCashItemDlg::InitialUpdate( void )
{
	m_pJobListBox = GetControl<CEtUIListBox>( "ID_LISTBOX_CLASS" );
	m_pJobExplainTextBox = GetControl<CEtUITextBox>( "ID_TEXTBOX_CLASSINFO" );
	m_pChangeJobOKButton = GetControl<CEtUIButton>( "ID_BT_OK" );

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_BT_CLOSE") );
}

void CDnChangeJobCashItemDlg::OnResponseUseItemFromServer( void )
{
	
	
}

void CDnChangeJobCashItemDlg::_CalcChangableJobs( void )
{
	m_vlChangableJobIDs.clear();
	m_pJobListBox->RemoveAllItems();

	// List jobs that can be changed based on the current character's job.
	vector<int> vlJobHistory;
	CDnLocalPlayerActor* pLocalPlayerActor = static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	pLocalPlayerActor->GetJobHistory( vlJobHistory );

	if( vlJobHistory.empty() )
		return;

	int iJobDeep = int(vlJobHistory.size() - 1);
	int iMyClass = (int)vlJobHistory.front();

	// A character that has not changed class cannot do anything with this item.
	if( 0 < iJobDeep )
	{
		DNTableFileFormat*  pJobTable = GetDNTable( CDnTableDB::TJOB );
		pJobTable->GetItemIDListFromField( "_JobNumber", iJobDeep, m_vlChangableJobIDs );

		// Exclude ones with a different parent class.
		vector<int>::iterator iter = m_vlChangableJobIDs.begin();
		for( iter; iter != m_vlChangableJobIDs.end(); )
		{
			int iJobID = *iter;
			int iParentJobID = pJobTable->GetFieldFromLablePtr( iJobID, "_Class" )->GetInteger();
#ifdef PRE_ADD_JOBCHANGE_RESTRICT_JOB
			bool bAvailableData = true;
			CDNTableFile::Cell* pServiceCell = pJobTable->GetFieldFromLablePtr(iJobID, "_Service");
			CDNTableFile::Cell* pJobChangeAvailCell = pJobTable->GetFieldFromLablePtr(iJobID, "_JobChangeAble");

			if (pServiceCell == NULL || pJobChangeAvailCell == NULL)
			{
				bAvailableData = false;
			}
			else
			{
				if (pServiceCell->GetInteger() <= 0 || pJobChangeAvailCell->GetInteger() <= 0)
					bAvailableData = false;
			}
#else
			bool bAvailableData = (pJobTable->GetFieldFromLablePtr( iJobID, "_Service" )->GetInteger() == 1);
#endif
			
			// Only need to compare character class. Exclude classes that match the current class or have a different parent.
			if( (iParentJobID != iMyClass) ||
				(iJobID == vlJobHistory.back()) || 
				(false == bAvailableData) )
			{
				iter = m_vlChangableJobIDs.erase( iter );
			}
			else
			{
				++iter;
			}
		}

		// Add class name to list box.
		for( int i = 0; i < (int)m_vlChangableJobIDs.size(); ++i )
		{
			int iJobID = m_vlChangableJobIDs.at( i );
			int iJobNameStringID = pJobTable->GetFieldFromLablePtr( iJobID, "_JobName" )->GetInteger();

			const wchar_t* pJobName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iJobNameStringID );
			m_pJobListBox->AddItem( pJobName, NULL, iJobID );
		}
	}

}

void CDnChangeJobCashItemDlg::SetItem( CDnItem* pChangeJobCashItem )
{
	m_pChangeJobCashItem = pChangeJobCashItem;
}

void CDnChangeJobCashItemDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_iSelectedJobID = 0;

		// After finding all jobs that can be changed into based on the current character's job, 
		// Add to the job list box.
		_CalcChangableJobs();

		m_pChangeJobOKButton->Enable( false );
		m_pJobExplainTextBox->ClearText();

		m_SmartMove.MoveCursor();
	}
	else
	{
		m_SmartMove.ReturnCursor();
	}

	CEtUIDialog::Show( bShow );
}

void CDnChangeJobCashItemDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand )
	{
		if( nCommand == EVENT_BUTTON_CLICKED )
		{
			if( IsCmdControl("ID_BT_OK") )
			{
				// Usable only in village.
				if( CDnWorld::MapTypeVillage == CDnWorld::GetInstance().GetMapType() )
				{
					// Your class will be changed to ~~, and skills will be reset. Do you really want to use this?
					DNTableFileFormat*  pJobTable = GetDNTable( CDnTableDB::TJOB ); 
					int iJobStringID = pJobTable->GetFieldFromLablePtr( m_iSelectedJobID, "_JobName" )->GetInteger();

					WCHAR wcBuffer[ 256 ] = { 0 };
					swprintf_s( wcBuffer, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4847), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, iJobStringID ) );

					GetInterface().MessageBox( wcBuffer, MB_YESNO, CHANGEJOB_CONFIRM_MB, this );
				}
			}
			else
			if( IsCmdControl( "ID_BT_CLOSE" ) )
			{
				Show( false );
			}
		}
		else
		if( nCommand == EVENT_LISTBOX_SELECTION )
		{
			if( IsCmdControl("ID_LISTBOX_CLASS") )
			{
				// Get job index using the selected index.
				m_pJobListBox->GetSelectedValue( m_iSelectedJobID );

				// Set class description in the text box.
				DNTableFileFormat*  pJobTable = GetDNTable( CDnTableDB::TJOB ); 
				int iJobExplainStringID = pJobTable->GetFieldFromLablePtr( m_iSelectedJobID, "_JobDescriptionID" )->GetInteger();

				m_pJobExplainTextBox->ClearText();
				m_pJobExplainTextBox->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, iJobExplainStringID) );

				m_pChangeJobOKButton->Enable( true );
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnChangeJobCashItemDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID )
	{
		case CHANGEJOB_CONFIRM_MB:
			{
				if( nCommand == EVENT_BUTTON_CLICKED )
				{
					if( IsCmdControl( "ID_YES" ) )
					{
						// Send class-change item use packet to server.
						GetItemTask().RequestUseChangeJobCashItem( m_pChangeJobCashItem->GetSerialID(), m_iSelectedJobID );

						// Close dialog
						Show( false );
					}
				}
			}
			break;
	}
}

#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM