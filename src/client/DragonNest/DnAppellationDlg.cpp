#include "StdAfx.h"

#include "DnActor.h"
#include "DnTableDB.h"
#include "DnAppellationDlg.h"
#include "DnAppellationItemDlg.h"
#include "DnAppellationTask.h"
#include "DnUiString.h"
#include "DnAppellationBookDlg.h"
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#include "DnInterface.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnAppellationDlg::CDnAppellationDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pListBoxEx(NULL)
, m_pCurrentName(NULL)
, m_pEffectDesc(NULL)
, m_pContantDesc(NULL)
, m_pTakeCondition(NULL)
, m_pStateCombo(NULL)
, m_pCoverAppellationName(NULL)
, m_pAppellationBookDlg(NULL)
, m_pAppellationTabDlg(NULL)
, m_nCoverAppellationIndex(-1)
, m_nApplyAppellationIndex(-1)
{
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5009), CDnAppellationTask::STRENGTH) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5010), CDnAppellationTask::AGILITY) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5011), CDnAppellationTask::INTELIGENCE) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5012), CDnAppellationTask::STAMINA) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5018), CDnAppellationTask::ATTACK_MIN_P) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5018), CDnAppellationTask::ATTACK_MAX_P) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5019), CDnAppellationTask::ATTACK_MIN_M) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5019), CDnAppellationTask::ATTACK_MAX_M) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5020), CDnAppellationTask::DEFENCE_P) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5062), CDnAppellationTask::DEFENCE_M) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5021), CDnAppellationTask::STIFF) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5024), CDnAppellationTask::STIFFRES) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5023), CDnAppellationTask::CRITICAL) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5026), CDnAppellationTask::CRITICALRES) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5022), CDnAppellationTask::STUN) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5025), CDnAppellationTask::STUNRES) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5029), CDnAppellationTask::FIRE_ATTACK) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5030), CDnAppellationTask::ICE_ATTACK) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5027), CDnAppellationTask::LIGHT_ATTACK) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5028), CDnAppellationTask::DARK_ATTACK) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5033), CDnAppellationTask::FIRE_DEFENSE) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5034), CDnAppellationTask::ICE_DEFENNSE) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5031), CDnAppellationTask::LIGHT_DEFENSE) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5032), CDnAppellationTask::DARK_DEFENSE) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5024), CDnAppellationTask::MOVE_SPEED) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 17),   CDnAppellationTask::MAX_HP) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 18),   CDnAppellationTask::MAX_SP) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2256), CDnAppellationTask::RECOVER_SP) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2279), CDnAppellationTask::SUPER_AMMOR) );
	m_vStateParam.push_back( make_pair(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2280), CDnAppellationTask::FINAL_DAMAGE) );


}

CDnAppellationDlg::~CDnAppellationDlg()
{
	m_pListBoxEx->RemoveAllItems();
	m_pStateCombo->RemoveAllItems();

	SAFE_DELETE( m_pAppellationBookDlg );
	SAFE_DELETE( m_pAppellationTabDlg );
}

void CDnAppellationDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "AppellationDlg.ui" ).c_str(), bShow );
}

void CDnAppellationDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>( "ID_TITLE_NAME_LISTBOX" );
	m_pCurrentName = GetControl<CEtUIStatic>( "ID_NOW_OPTION" );
	m_pEffectDesc = GetControl<CEtUITextBox>( "ID_TEXTBOX_EFFECT" );
	m_pContantDesc = GetControl<CEtUITextBox>( "ID_TEXTBOX_CONTANT" );
	m_pTakeCondition = GetControl<CEtUITextBox>( "ID_TEXTBOX_RULE" );
	m_pStateCombo = GetControl<CEtUIComboBox>( "ID_COMBOBOX_SORT" );

	int itr = -2;

	m_pStateCombo->AddItem(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93), NULL, ++itr);
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Strength
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Agility
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Intelligence
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Health
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Physical attack power (min, max)
	++itr;
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Magic attack (min, max)
	++itr;
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Physical Defense
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Magic defense
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Stagger power
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Stagger resistance
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Critical
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Critical resistance
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Stun
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Stun resistance
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Fire attack
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Physical attack
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Light attack
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Dark attack
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Fire defense
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Physical defense
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Light defense
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Dark defense
	++itr;																							//Move speed
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Max HP
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Max MP
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr );	//Mana recovery
	++itr;																							//Super armor
	m_pStateCombo->AddItem( m_vStateParam[itr].first.c_str(), &m_vStateParam[itr].second, ++itr ); //Final damage

	m_pCoverAppellationName = GetControl<CEtUIStatic>( "ID_NOW_NAME" );

	m_pAppellationBookDlg = new CDnAppellationBookDlg( UI_TYPE_CHILD, this );
	m_pAppellationBookDlg->Initialize( false );

	m_pAppellationTabDlg = new CDnAppellationTabDlg( UI_TYPE_CHILD, this );
	m_pAppellationTabDlg->Initialize( true );
}

void CDnAppellationDlg::ChangeDialogMode( eDialogMode eMode )
{
	if( eMode == eDialogMode::Appellation )
	{
		m_pAppellationBookDlg->Show( false );
	}
	else if( eMode == eDialogMode::CollectionBook )
	{
		m_pAppellationBookDlg->Show( true );
	}
}

void CDnAppellationDlg::SetCollectionMaxPage( int nMaxPage )
{
	if( m_pAppellationBookDlg )
		m_pAppellationBookDlg->SetMaxPage( nMaxPage );
}

void CDnAppellationDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED ) {
		if( IsCmdControl( "ID_BUTTON_USE" ) ) {
		}
		if( IsCmdControl( "ID_BUTTON_UNUSE" ) ) {
		}
		if( IsCmdControl( "ID_BUTTON_CLOSE" ) ) {
			Show( false );
		}
		if( IsCmdControl( "ID_BT_OK" ) )
			CDnAppellationTask::GetInstance().RequestSelectAppellation( m_nApplyAppellationIndex, m_nCoverAppellationIndex );
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_SET_APPELLATION);
		}
#endif
	}

	if( nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( IsCmdControl( "ID_TITLE_NAME_LISTBOX" ) )
		{
			m_pEffectDesc->ClearText();
			m_pContantDesc->ClearText();
			m_pTakeCondition->ClearText();

			int select = m_pListBoxEx->GetSelectedIndex();

			if( -1 != select )
			{
				CDnAppellationItemDlg *pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>(select);

				if( !pItem->GetInfo()->szEffectDescription.empty() )
					m_pEffectDesc->SetText( pItem->GetInfo()->szEffectDescription.c_str() );
				else
					m_pEffectDesc->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93) );

				if( !pItem->GetInfo()->szContantDescription.empty() )
					m_pContantDesc->SetText( pItem->GetInfo()->szContantDescription.c_str() );
				else
					m_pContantDesc->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93) );

				if( !pItem->GetInfo()->szTakeCondition.empty() )
					m_pTakeCondition->SetText( pItem->GetInfo()->szTakeCondition.c_str() );
				else
					m_pTakeCondition->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93) );
				if(!pItem->GetInfo()->szPeriodAppellation.empty())
				{	
					//Compare time: display in red if under one day, white if one day or more
					DWORD dwColor = pItem->IsOneDayExpire() ? textcolor::RED : textcolor::WHITE;
					m_pTakeCondition->AddText( pItem->GetInfo()->szPeriodAppellation.c_str(), dwColor);
				}
			}
		}
	}

	if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED )
	{
		if( IsCmdControl( "ID_COMBOBOX_SORT" ) )
		{
			if( NULL != m_pStateCombo->GetSelectedData() )
			{
				int nStateIndex = *((int *)m_pStateCombo->GetSelectedData());

				SortList( nStateIndex );

				m_pEffectDesc->ClearText();
				m_pContantDesc->ClearText();
				m_pTakeCondition->ClearText();

				int select = m_pListBoxEx->GetSelectedIndex();

				if( -1 != select )
				{
					CDnAppellationItemDlg *pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>(select);

					if( !pItem->GetInfo()->szEffectDescription.empty() )
						m_pEffectDesc->SetText( pItem->GetInfo()->szEffectDescription.c_str() );
					else
						m_pEffectDesc->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93) );

					if( !pItem->GetInfo()->szContantDescription.empty() )
						m_pContantDesc->SetText( pItem->GetInfo()->szContantDescription.c_str() );
					else
						m_pContantDesc->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93) );

					if( !pItem->GetInfo()->szTakeCondition.empty() )
						m_pTakeCondition->SetText( pItem->GetInfo()->szTakeCondition.c_str() );
					else
						m_pTakeCondition->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93) );
					if(!pItem->GetInfo()->szPeriodAppellation.empty())
					{	
						//Compare time: display in red if under one day, white if one day or more
						DWORD dwColor = pItem->IsOneDayExpire() ? textcolor::RED : textcolor::WHITE;
						m_pTakeCondition->AddText( pItem->GetInfo()->szPeriodAppellation.c_str(), dwColor);
					}
				}
			}
			else	//If it has no value, find items with descriptions only and sort them.
			{
				SortList( -1 );	
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnAppellationDlg::Show( bool bShow ) 
{ 
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		RefreshList();
		if( m_pAppellationBookDlg && m_pAppellationBookDlg->IsShow() )
		{
			CDnAppellationTask::GetInstance().UpdateMyAppellationCollectionList();
			m_pAppellationBookDlg->SetCurrentPageUI();
		}
	}
	else
	{
		m_pListBoxEx->RemoveAllItems();
		DWORD dwCount = CDnAppellationTask::GetInstance().GetAppellationCount();
		for( DWORD i=0; i<dwCount; i++ ) {
			CDnAppellationTask::AppellationStruct *pInfo = CDnAppellationTask::GetInstance().GetAppellationInfo(i);

			pInfo->bNew = false;
		}

		m_pAppellationTabDlg->ResetButton();
	}

	CEtUIDialog::Show( bShow );
}

void CDnAppellationDlg::RefreshList()
{
	m_pListBoxEx->RemoveAllItems();
	m_pEffectDesc->ClearText();
	m_pContantDesc->ClearText();
	m_pTakeCondition->ClearText();
	m_nCoverAppellationIndex = -1;
	m_nApplyAppellationIndex = -1;
	bool bSetCoverAppellation = false;
	DWORD dwCount = CDnAppellationTask::GetInstance().GetAppellationCount();
	for( DWORD i=0; i<dwCount; i++ ) {
		CDnAppellationTask::AppellationStruct *pInfo = CDnAppellationTask::GetInstance().GetAppellationInfo(i);
		if( !pInfo ) continue;

		CDnAppellationItemDlg* pItem = NULL;
		if( pInfo->nArrayIndex == CDnAppellationTask::GetInstance().GetSelectAppellation() ) {
			m_nApplyAppellationIndex = pInfo->nArrayIndex;
			if( bSetCoverAppellation )
				pItem = m_pListBoxEx->InsertItem<CDnAppellationItemDlg>(1);
			else
				pItem = m_pListBoxEx->InsertItem<CDnAppellationItemDlg>(0);
			
			pItem->SetInfo( pInfo, this );

			pItem->SetChecked( true );
			m_pCurrentName->SetText( pInfo->szName );

			if( !pInfo->szEffectDescription.empty() )
				m_pEffectDesc->SetText( pInfo->szEffectDescription.c_str() );
			else
				m_pEffectDesc->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93) );
			
			if( !pInfo->szContantDescription.empty() )
				m_pContantDesc->SetText( pInfo->szContantDescription.c_str() );
			else
				m_pContantDesc->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93) );

			if( !pInfo->szTakeCondition.empty() )
				m_pTakeCondition->SetText( pInfo->szTakeCondition.c_str() );
			else
				m_pTakeCondition->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93) );
			if(!pItem->GetInfo()->szPeriodAppellation.empty())
			{	
				//Compare time: display in red if under one day, white if one day or more
				DWORD dwColor = pItem->IsOneDayExpire() ? textcolor::RED : textcolor::WHITE;
				m_pTakeCondition->AddText( pItem->GetInfo()->szPeriodAppellation.c_str(), dwColor);
			}
			if( pInfo->nArrayIndex == CDnAppellationTask::GetInstance().GetSelectCoverAppellation() )
			{
				m_nCoverAppellationIndex = pInfo->nArrayIndex;
				pItem->SetCoverChecked( true );
				m_pCoverAppellationName->SetText( pInfo->szName );
			}
		}
		else if( pInfo->nArrayIndex == CDnAppellationTask::GetInstance().GetSelectCoverAppellation() )
		{
			m_nCoverAppellationIndex = pInfo->nArrayIndex;
			pItem = m_pListBoxEx->InsertItem<CDnAppellationItemDlg>(0);
			pItem->SetInfo( pInfo, this );
			pItem->SetCoverChecked( true );
			m_pCoverAppellationName->SetText( pInfo->szName );
			bSetCoverAppellation = true;
		}
		else
		{
			pItem = m_pListBoxEx->AddItem<CDnAppellationItemDlg>();
			pItem->SetInfo( pInfo, this );
		}
	}

	if( CDnAppellationTask::GetInstance().GetSelectAppellation() == -1 ) {
		const wchar_t* szEmpty = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93 );
		m_pCurrentName->SetText( szEmpty );
		m_pEffectDesc->SetText( szEmpty );
		m_pContantDesc->SetText( szEmpty );
		m_pTakeCondition->SetText( szEmpty );
	}
	if( CDnAppellationTask::GetInstance().GetSelectCoverAppellation() == -1 ) {
		const wchar_t* szEmpty = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93 );
		m_pCoverAppellationName->SetText( szEmpty );
	}

	if( NULL != m_pStateCombo->GetSelectedData() )
	{
		int nStateIndex = *((int *)m_pStateCombo->GetSelectedData());

		SortList( nStateIndex );
	}
	else	//If it has no value, find items with descriptions only and sort them.
	{
		SortList( -1 );	
	}
}

void CDnAppellationDlg::RefreshSelect()
{
	RefreshList();
	if( m_pStateCombo )
	{
		int nStateIndex = -1;
		m_pStateCombo->GetSelectedValue( nStateIndex );
		SortList( nStateIndex );
	}
}

bool SortByInfoName( CDnAppellationTask::AppellationStruct* pInfoA, CDnAppellationTask::AppellationStruct* pInfoB )
{
	if( pInfoA == NULL || pInfoB == NULL )
		return false;

	return (0 > wcscmp( pInfoA->szName.c_str(), pInfoB->szName.c_str() ) );
}

void CDnAppellationDlg::SortList( const int nStateIndex )
{
	if( -1 == nStateIndex )
	{
		std::vector<CDnAppellationTask::AppellationStruct*> vecAppellationInfoList;

		for( int itr = 0; itr < m_pListBoxEx->GetSize(); ++itr )
		{
			CDnAppellationItemDlg *pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>( itr );
			if( pItem == NULL )	continue;
			if( CDnAppellationTask::GetInstance().GetSelectAppellation() == pItem->GetInfo()->nArrayIndex )	continue;
			if( CDnAppellationTask::GetInstance().GetSelectCoverAppellation() == pItem->GetInfo()->nArrayIndex ) continue;

			vecAppellationInfoList.push_back( pItem->GetInfo() );
		}

		std::sort( vecAppellationInfoList.begin(), vecAppellationInfoList.end(), SortByInfoName );

		std::vector<CDnAppellationTask::AppellationStruct*>::reverse_iterator riter = vecAppellationInfoList.rbegin();
		for( int itr = 0; itr < m_pListBoxEx->GetSize(); ++itr )
		{
			CDnAppellationItemDlg *pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>( itr );
			if( pItem == NULL )	continue;
			if( CDnAppellationTask::GetInstance().GetSelectAppellation() == pItem->GetInfo()->nArrayIndex )	continue;
			if( CDnAppellationTask::GetInstance().GetSelectCoverAppellation() == pItem->GetInfo()->nArrayIndex ) continue;

			m_pListBoxEx->RemoveItem( itr );

			int nIndex = 0;
			if( CDnAppellationTask::GetInstance().GetSelectAppellation() != -1 )
				nIndex++;
			if( CDnAppellationTask::GetInstance().GetSelectAppellation() != CDnAppellationTask::GetInstance().GetSelectCoverAppellation() 
				&& CDnAppellationTask::GetInstance().GetSelectCoverAppellation() != -1 )
				nIndex++;

			pItem = m_pListBoxEx->InsertItem<CDnAppellationItemDlg>( nIndex );
			pItem->SetInfo( (*riter), this );
			pItem->SetChecked( false );
		
			riter++;
			if( riter == vecAppellationInfoList.rend() )
				break;
		}

		SortPcBangAppellation();
		return;
	}

	int pos;
	int value;

	for( int itr = 0; itr < m_pListBoxEx->GetSize(); ++itr )
	{
		CDnAppellationItemDlg *pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>( itr );
		if( pItem == NULL ) continue;
		if( CDnAppellationTask::GetInstance().GetSelectAppellation() == pItem->GetInfo()->nArrayIndex ) continue;
		if( CDnAppellationTask::GetInstance().GetSelectCoverAppellation() == pItem->GetInfo()->nArrayIndex ) continue;

		value = FindList( pItem, nStateIndex );

		// If an item is currently selected, check starting from the 2nd list.
		if( -1 != value ) 
		{
			CDnAppellationTask::GetInstance().GetSelectAppellation() == -1 ?
				pos = FindPosition( nStateIndex, 0, FindValue(pItem, value) ) : pos = FindPosition( nStateIndex, 1, FindValue(pItem, value) );
			if( CDnAppellationTask::GetInstance().GetSelectCoverAppellation() != -1 
				&& CDnAppellationTask::GetInstance().GetSelectAppellation() != CDnAppellationTask::GetInstance().GetSelectCoverAppellation() )
			{
				if( CDnAppellationTask::GetInstance().GetSelectAppellation() == -1 )
					pos = FindPosition( nStateIndex, 1, FindValue(pItem, value) );
				else
					pos = FindPosition( nStateIndex, 2, FindValue(pItem, value) );
			}

			//When self is selected and was not at the top
			if( itr != pos - 1 )
			{
				CDnAppellationTask::AppellationStruct *pInfo = pItem->GetInfo();
				m_pListBoxEx->RemoveItem( itr );

				pItem = m_pListBoxEx->InsertItem<CDnAppellationItemDlg>( pos );
				pItem->SetInfo( pInfo, this );
				pItem->SetChecked( false );
			}
		}
	}

	SortPcBangAppellation();

	// Re-apply Check mark since apply method changed to pressing the Apply button
	for( int itr = 0; itr < m_pListBoxEx->GetSize(); ++itr )
	{
		CDnAppellationItemDlg *pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>( itr );

		pItem->SetChecked( false );
		pItem->SetCoverChecked( false );

		if( CDnAppellationTask::GetInstance().GetSelectAppellation() == pItem->GetInfo()->nArrayIndex )
		{
			SetSelectAppellationName( CDnAppellationDlg::eAppellationType::Appellation_Apply, pItem->GetInfo()->nArrayIndex );
			pItem->SetChecked( true );
		}

		if( CDnAppellationTask::GetInstance().GetSelectCoverAppellation() == pItem->GetInfo()->nArrayIndex )
		{
			SetSelectAppellationName( CDnAppellationDlg::eAppellationType::Appellation_Cover, pItem->GetInfo()->nArrayIndex );
			pItem->SetCoverChecked( true );
		}
	}

	if( CDnAppellationTask::GetInstance().GetSelectAppellation() == -1 )
		SetSelectAppellationName( CDnAppellationDlg::eAppellationType::Appellation_Apply, -1 );
	if( CDnAppellationTask::GetInstance().GetSelectCoverAppellation() == -1 )
		SetSelectAppellationName( CDnAppellationDlg::eAppellationType::Appellation_Cover, -1 );
}

void CDnAppellationDlg::SortPcBangAppellation()
{
	int nStartIndex = 0;

	if( CDnAppellationTask::GetInstance().GetSelectAppellation() != -1 )
		nStartIndex++;
	if( CDnAppellationTask::GetInstance().GetSelectCoverAppellation() != -1 &&
		CDnAppellationTask::GetInstance().GetSelectAppellation() != CDnAppellationTask::GetInstance().GetSelectCoverAppellation() )
		nStartIndex++;

	std::vector<int> vecPCBangAppellationIndex;
	for( int itr = 0; itr < m_pListBoxEx->GetSize(); ++itr )
	{
		CDnAppellationItemDlg *pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>( itr );
		if( pItem == NULL ) continue;
		if( pItem->GetInfo()->nArrayIndex == CDnAppellationTask::GetInstance().GetSelectAppellation() 
			|| pItem->GetInfo()->nArrayIndex == CDnAppellationTask::GetInstance().GetSelectCoverAppellation()  )
			continue;

		if( pItem->GetInfo()->cColorType == AppellationType::Type::PCBang )
		{
			if( itr != nStartIndex )
			{
				if( nStartIndex >= m_pListBoxEx->GetSize() )
					break;

				CDnAppellationItemDlg *pSwapItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>( nStartIndex );
				if( pSwapItem && pSwapItem->GetInfo()->cColorType != AppellationType::Type::PCBang )
				{
					CDnAppellationTask::AppellationStruct* pInfo = pItem->GetInfo();
					m_pListBoxEx->RemoveItem( itr );

					pItem = m_pListBoxEx->InsertItem<CDnAppellationItemDlg>( nStartIndex );
					pItem->SetInfo( pInfo, this );
				}
			}
			vecPCBangAppellationIndex.push_back( nStartIndex );
			nStartIndex++;
		}
	}

	// Sort by level
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TAPPELLATION );
	if( pSox == NULL )
		return;

	for( int i=static_cast<int>( vecPCBangAppellationIndex.size() ) - 2; i>=0; --i )
	{
		for( int j=0; j<=i; ++j )
		{
			CDnAppellationItemDlg *pFirstItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>( vecPCBangAppellationIndex[j] );
			CDnAppellationItemDlg *pSecondItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>( vecPCBangAppellationIndex[j+1] );

			if( pFirstItem == NULL || pSecondItem == NULL ) continue;

			int nItemID = pSox->GetItemID( pFirstItem->GetInfo()->nArrayIndex );
			int nFirstLimitLevel = pSox->GetFieldFromLablePtr( nItemID, "_LevelLimit" )->GetInteger();

			nItemID = pSox->GetItemID( pSecondItem->GetInfo()->nArrayIndex );
			int nSecondLimitLevel = pSox->GetFieldFromLablePtr( nItemID, "_LevelLimit" )->GetInteger();

			if( nFirstLimitLevel < nSecondLimitLevel )
			{
				m_pListBoxEx->SwapItem( vecPCBangAppellationIndex[j], vecPCBangAppellationIndex[j+1] );
			}
		}
	}
}

void CDnAppellationDlg::SetSelectAppellationName( eAppellationType AppellationType, int nAppellationIndex )
{
	if( nAppellationIndex == -1 )
	{
		if( AppellationType == eAppellationType::Appellation_Cover )
		{
			m_pCoverAppellationName->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93 ) );
			m_nCoverAppellationIndex = -1;
		}
		else if( AppellationType == eAppellationType::Appellation_Apply )
		{
			m_pCurrentName->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93 ) );
			m_nApplyAppellationIndex = -1;
		}
	}
	else
	{
		if( AppellationType == eAppellationType::Appellation_Cover )
		{
			ClearAppellationCheck( AppellationType, nAppellationIndex );
			m_pCoverAppellationName->SetText( CDnAppellationTask::GetInstance().GetAppellationName( nAppellationIndex, (CDnPlayerActor *)CDnActor::s_hLocalActor.GetPointer() ) );
			m_nCoverAppellationIndex = nAppellationIndex;
		}
		else if( AppellationType == eAppellationType::Appellation_Apply )
		{
			ClearAppellationCheck( AppellationType, nAppellationIndex );
			m_pCurrentName->SetText( CDnAppellationTask::GetInstance().GetAppellationName( nAppellationIndex, (CDnPlayerActor *)CDnActor::s_hLocalActor.GetPointer() ) );
			m_nApplyAppellationIndex = nAppellationIndex;
		}
	}
}

void CDnAppellationDlg::ClearAppellationCheck( eAppellationType AppellationType, int nCheckAppellationIndex )
{
	for( int i=0; i<m_pListBoxEx->GetSize(); i++ )
	{
		CDnAppellationItemDlg* pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>( i );
		if( pItem->GetInfo()->nArrayIndex != nCheckAppellationIndex )
		{
			if( AppellationType == eAppellationType::Appellation_Apply )
				pItem->SetChecked( false );
			else if( AppellationType == eAppellationType::Appellation_Cover )
				pItem->SetCoverChecked( false );
		}
	}
}

/*
Check if a state value exists at the position in the given list
If present, check whether it is larger or smaller.
If greater than or equal, or not found, return the current position
If it exists and is smaller, run the recursive function at the next position.
*/
int CDnAppellationDlg::FindPosition( const int nStateIndex, int nPosition, const float value )
{
	CDnAppellationItemDlg *pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>(nPosition);

	int itr = FindList(pItem, nStateIndex);

	if( -1 != itr )
	{
		float temp = FindValue(pItem, itr);

		if( value < temp )
			nPosition = FindPosition( nStateIndex, nPosition+1, value );
	}

	return nPosition;
}

/*
Check whether a state exists on the current list item, and if so,
Report the vector position.
*/
int CDnAppellationDlg::FindList( const CDnAppellationItemDlg * pItem, const int nStateIndex )
{
	//first : state parameter value
	//second : state index value of title data

	int Retval = -1;
	int nValueIndex;

	for( int itr = 0; itr < (int)(pItem->GetInfo()->vParamIndex.size()); ++itr ) 
	{
		nValueIndex = pItem->GetInfo()->vParamIndex[itr].first;

		if( nValueIndex == 100 )		nValueIndex = 30;
		else if( nValueIndex >= 50 )	nValueIndex -= 50;

		if( nStateIndex == nValueIndex ) {
			Retval = itr;
			break;
		}
	}

	return Retval;
}

float CDnAppellationDlg::FindValue( const CDnAppellationItemDlg * pItem, const int nIndex )
{
	float Retval;
	int nStateIndex;

	char szLabel[32];

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TAPPELLATION );
	int nItemID = pSox->GetItemID( pItem->GetInfo()->nArrayIndex );

	sprintf_s( szLabel, "_StateValue%d", pItem->GetInfo()->vParamIndex[nIndex].second + 1 );
	char *szStateValue = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetString();

	nStateIndex = pItem->GetInfo()->vParamIndex[nIndex].first;

	if( nStateIndex == 100 )		nStateIndex = 30;

	if( nStateIndex < 50 && !(16 <= nStateIndex && nStateIndex <= 23) )
	{
		Retval = (float)atoi(szStateValue);
	}
	else
	{
		if( nStateIndex >= 50 ) nStateIndex -= 50;

		nStateIndex -= 50;
		Retval = (float)atof(szStateValue) * 100.0f;
	}

	return Retval;
}

void CDnAppellationDlg::UnselectAllCheckBox( CDnAppellationItemDlg *pCheckItem )
{
	for( int i=0; i<m_pListBoxEx->GetSize(); i++ ) {
		CDnAppellationItemDlg *pItem = m_pListBoxEx->GetItem<CDnAppellationItemDlg>(i);
		if( pCheckItem == pItem ) continue;
		pItem->SetChecked( false );
	}
}


//////////////////////////////////////////////////////////////////////////
// CDnAppellationTabDlg

CDnAppellationTabDlg::CDnAppellationTabDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pAppellationButton( NULL )
, m_pCollectionButton( NULL )
{
}

CDnAppellationTabDlg::~CDnAppellationTabDlg()
{
}

void CDnAppellationTabDlg::Initialize(bool bShow)
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "AppellationTabDlg.ui" ).c_str(), bShow );
}

void CDnAppellationTabDlg::InitialUpdate()
{
	m_pAppellationButton = GetControl<CEtUIRadioButton>("ID_RBT_APP");
	m_pCollectionButton = GetControl<CEtUIRadioButton>("ID_RBT_BOOK");
}

void CDnAppellationTabDlg::InitCustomControl( CEtUIControl *pControl )
{
}

void CDnAppellationTabDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		CDnAppellationDlg* pAppellationDlg = dynamic_cast<CDnAppellationDlg*>( GetParentDialog() );

		if( pAppellationDlg )
		{
			if( IsCmdControl( "ID_RBT_APP" ) )
				pAppellationDlg->ChangeDialogMode( CDnAppellationDlg::eDialogMode::Appellation );
			else if( IsCmdControl( "ID_RBT_BOOK" ) )
				pAppellationDlg->ChangeDialogMode( CDnAppellationDlg::eDialogMode::CollectionBook );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnAppellationTabDlg::ResetButton()
{
	m_pAppellationButton->SetChecked( true );
	m_pCollectionButton->SetChecked( false );

	CDnAppellationDlg* pAppellationDlg = dynamic_cast<CDnAppellationDlg*>( GetParentDialog() );

	if( pAppellationDlg )
		pAppellationDlg->ChangeDialogMode( CDnAppellationDlg::eDialogMode::Appellation );
}