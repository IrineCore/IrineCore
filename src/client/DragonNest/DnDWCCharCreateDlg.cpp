#include "StdAfx.h"

#if defined( PRE_ADD_DWC )

#include "DnDWCCharCreateDlg.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnInterface.h"
#include "TaskManager.h"
#include "DnLoginTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnDWCCharCreateButtonDlg
//-----------------------------------------------------------------------------
CDnDWCCharCreateButtonDlg::CDnDWCCharCreateButtonDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pParentBaseDlg( NULL )
{
}

CDnDWCCharCreateButtonDlg::~CDnDWCCharCreateButtonDlg()
{
}

void CDnDWCCharCreateButtonDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvP_CharCreateBT.ui" ).c_str(), bShow );
}

void CDnDWCCharCreateButtonDlg::InitialUpdate()
{
}

void CDnDWCCharCreateButtonDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_BACK" ) || IsCmdControl( "ID_BT_CANCEL" ) )
		{
			GetInterface().OpenDWCCharCreateDlg( false );
			return;
		}
		else if( IsCmdControl( "ID_BT_OK" ) || IsCmdControl( "ID_BT_CREATE" ) )
		{
			CDnLoginTask *pLoginTask = (CDnLoginTask*)CTaskManager::GetInstance().GetTask( "LoginTask" );
			if( pLoginTask && m_pParentBaseDlg && m_pParentBaseDlg->GetSelectDlg() )
			{
				const CDnDWCCharCreateSelectDlg::SDWCClassPreset *pPreset = m_pParentBaseDlg->GetSelectDlg()->GetSelectedPreset();
				if( pPreset )
				{
					m_pParentBaseDlg->Show( false );
					pLoginTask->StartDWCCharCreate( pPreset->nClassID, pPreset->nJobCode, pPreset->nJobCode2,
						pPreset->nLevel, pPreset->nExp, pPreset->nCreateMapIndex, pPreset->nGold, pPreset->nSkillPoint );
				}
			}
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnDWCCharCreateSelectDlg
//-----------------------------------------------------------------------------
CDnDWCCharCreateSelectDlg::CDnDWCCharCreateSelectDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pComboChar( NULL )
	, m_nSelectedIndex( 0 )
	, m_bSyncingSelection( false )
	, m_pParentBaseDlg( NULL )
{
}

CDnDWCCharCreateSelectDlg::~CDnDWCCharCreateSelectDlg()
{
}

void CDnDWCCharCreateSelectDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvP_CharCreateSelect.ui" ).c_str(), bShow );
}

void CDnDWCCharCreateSelectDlg::InitialUpdate()
{
	m_vecRadioClass.clear();
	char szName[32] = { 0 };
	for( int i = 0; i < MAX_DWC_CLASS_RADIO; ++i )
	{
		sprintf_s( szName, sizeof( szName ), "ID_RBT_%d", i );
		CEtUIRadioButton *pRadio = GetControl<CEtUIRadioButton>( szName );
		m_vecRadioClass.push_back( pRadio );
	}
	m_pComboChar = GetControl<CEtUIComboBox>( "ID_COMBOBOX_CHAR" );

	LoadPresets();
}

void CDnDWCCharCreateSelectDlg::LoadPresets()
{
	m_vecPresets.clear();
	if( m_pComboChar )
		m_pComboChar->RemoveAllItems();

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TDWCCREATE );
	if( !pSox )
		return;

	int nCount = pSox->GetItemCount();
	for( int i = 0; i < nCount; ++i )
	{
		int nRowID = pSox->GetItemID( i );
		SDWCClassPreset preset;
		preset.nRowID = nRowID;

		DNTableCell *pCell = pSox->GetFieldFromLablePtr( nRowID, "_ClassID" );
		if( pCell ) preset.nClassID = pCell->GetInteger();

		pCell = pSox->GetFieldFromLablePtr( nRowID, "_Jobcode1" );
		if( pCell ) preset.nJobCode = pCell->GetInteger();

		pCell = pSox->GetFieldFromLablePtr( nRowID, "_Jobcode2" );
		if( pCell ) preset.nJobCode2 = pCell->GetInteger();

		pCell = pSox->GetFieldFromLablePtr( nRowID, "_JobName" );
		if( pCell ) preset.nJobNameUIStringID = pCell->GetInteger();

		pCell = pSox->GetFieldFromLablePtr( nRowID, "_Level" );
		if( pCell ) preset.nLevel = pCell->GetInteger();

		pCell = pSox->GetFieldFromLablePtr( nRowID, "_Experience" );
		if( pCell ) preset.nExp = pCell->GetInteger();

		pCell = pSox->GetFieldFromLablePtr( nRowID, "_CreateMapIndex" );
		if( pCell ) preset.nCreateMapIndex = pCell->GetInteger();

		pCell = pSox->GetFieldFromLablePtr( nRowID, "_Gold" );
		if( pCell ) preset.nGold = pCell->GetInteger();

		pCell = pSox->GetFieldFromLablePtr( nRowID, "_SkillPoint" );
		if( pCell ) preset.nSkillPoint = pCell->GetInteger();

		pCell = pSox->GetFieldFromLablePtr( nRowID, "_ClassImage" );
		if( pCell && pCell->GetString() ) preset.strClassImage = pCell->GetString();

		m_vecPresets.push_back( preset );

		if( m_pComboChar )
		{
			const WCHAR *wszJobName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, preset.nJobNameUIStringID );
			m_pComboChar->AddItem( wszJobName ? wszJobName : L"", NULL, nRowID );
		}
	}

	for( size_t i = 0; i < m_vecRadioClass.size(); ++i )
	{
		if( m_vecRadioClass[i] )
			m_vecRadioClass[i]->Show( i < m_vecPresets.size() );
	}
}

void CDnDWCCharCreateSelectDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow && m_vecPresets.empty() )
		LoadPresets();

	if( bShow && !m_vecPresets.empty() )
	{
		m_bSyncingSelection = true;
		m_nSelectedIndex = 0;
		if( !m_vecRadioClass.empty() && m_vecRadioClass[0] )
			m_vecRadioClass[0]->SetChecked( true, true, false );
		if( m_pComboChar )
			m_pComboChar->SetSelectedByIndex( 0 );
		m_bSyncingSelection = false;
		if( m_pParentBaseDlg )
			m_pParentBaseDlg->OnSelectPreset( GetSelectedPreset() );
	}

	CEtUIDialog::Show( bShow );
}

void CDnDWCCharCreateSelectDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( m_bSyncingSelection )
	{
		CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
		return;
	}

	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		int nIdx = -1;
		if( sscanf_s( pControl->GetControlName(), "ID_RBT_%d", &nIdx ) == 1 && nIdx >= 0 && nIdx < (int)m_vecPresets.size() )
		{
			m_bSyncingSelection = true;
			m_nSelectedIndex = nIdx;
			if( m_pComboChar && m_pComboChar->GetSelectedIndex() != nIdx )
				m_pComboChar->SetSelectedByIndex( nIdx );
			m_bSyncingSelection = false;
			if( m_pParentBaseDlg )
				m_pParentBaseDlg->OnSelectPreset( GetSelectedPreset() );
			return;
		}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED && IsCmdControl( "ID_COMBOBOX_CHAR" ) )
	{
		if( m_pComboChar )
		{
			int nIdx = m_pComboChar->GetSelectedIndex();
			if( nIdx >= 0 && nIdx < (int)m_vecPresets.size() )
			{
				m_bSyncingSelection = true;
				m_nSelectedIndex = nIdx;
				if( nIdx < (int)m_vecRadioClass.size() && m_vecRadioClass[nIdx] && !m_vecRadioClass[nIdx]->IsChecked() )
					m_vecRadioClass[nIdx]->SetChecked( true, true, false );
				m_bSyncingSelection = false;
				if( m_pParentBaseDlg )
					m_pParentBaseDlg->OnSelectPreset( GetSelectedPreset() );
			}
		}
		return;
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

const CDnDWCCharCreateSelectDlg::SDWCClassPreset *CDnDWCCharCreateSelectDlg::GetSelectedPreset() const
{
	if( m_nSelectedIndex < 0 || m_nSelectedIndex >= (int)m_vecPresets.size() )
		return NULL;
	return &m_vecPresets[m_nSelectedIndex];
}

int CDnDWCCharCreateSelectDlg::GetSelectedClassID() const
{
	const SDWCClassPreset *p = GetSelectedPreset();
	return p ? p->nClassID : 0;
}

int CDnDWCCharCreateSelectDlg::GetSelectedJobCode() const
{
	const SDWCClassPreset *p = GetSelectedPreset();
	return p ? p->nJobCode : 0;
}

//-----------------------------------------------------------------------------
// CDnDWCCharCreateDlg
//-----------------------------------------------------------------------------
CDnDWCCharCreateDlg::CDnDWCCharCreateDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pButtonDlg( NULL )
	, m_pSelectDlg( NULL )
	, m_pTextureClass( NULL )
	, m_pStaticName( NULL )
{
}

CDnDWCCharCreateDlg::~CDnDWCCharCreateDlg()
{
	SAFE_RELEASE_SPTR( m_hClassTexture );
	SAFE_DELETE( m_pButtonDlg );
	SAFE_DELETE( m_pSelectDlg );
}

void CDnDWCCharCreateDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvP_CharCreateBase.ui" ).c_str(), bShow );

	m_pButtonDlg = new CDnDWCCharCreateButtonDlg( UI_TYPE_CHILD, this );
	if( m_pButtonDlg )
	{
		m_pButtonDlg->SetParentBaseDlg( this );
		m_pButtonDlg->Initialize( false );
	}

	m_pSelectDlg = new CDnDWCCharCreateSelectDlg( UI_TYPE_CHILD, this );
	if( m_pSelectDlg )
	{
		m_pSelectDlg->SetParentBaseDlg( this );
		m_pSelectDlg->Initialize( false );
	}
}

void CDnDWCCharCreateDlg::InitialUpdate()
{
	m_pTextureClass = GetControl<CEtUITextureControl>( "ID_TEXTUREL0" );
	m_pStaticName = GetControl<CEtUIStatic>( "ID_STATIC_NAME" );
}

void CDnDWCCharCreateDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( m_pSelectDlg )
		ShowChildDialog( m_pSelectDlg, bShow );
	if( m_pButtonDlg )
		ShowChildDialog( m_pButtonDlg, bShow );
}

bool CDnDWCCharCreateDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN && wParam == VK_ESCAPE )
	{
		GetInterface().OpenDWCCharCreateDlg( false );
		return true;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnDWCCharCreateDlg::OnSelectPreset( const CDnDWCCharCreateSelectDlg::SDWCClassPreset *pPreset )
{
	if( !pPreset )
		return;

	if( m_pStaticName )
	{
		const WCHAR *wszJobName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pPreset->nJobNameUIStringID );
		m_pStaticName->SetText( wszJobName ? wszJobName : L"" );
	}

	if( m_pTextureClass && !pPreset->strClassImage.empty() )
	{
		SAFE_RELEASE_SPTR( m_hClassTexture );
		m_hClassTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( pPreset->strClassImage.c_str() ).c_str(), RT_TEXTURE );
		if( m_hClassTexture )
			m_pTextureClass->SetTexture( m_hClassTexture, 0, 0, m_hClassTexture->Width(), m_hClassTexture->Height() );
	}
}

#endif // PRE_ADD_DWC
