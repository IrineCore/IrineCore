#include "StdAfx.h"
#include "DnSkillTreeDlg.h"
#include "EtUIControl.h"
#include "DnSkillTreeContentDlg.h"
#include "DnLocalPlayerActor.h"
#include "DnSkillTask.h"
#include "DnTableDB.h"
#include "DnLifeSkillContent.h"
#include "DnLifeSkillItem.h"
#include "DnInterface.h"
#include "DnSkillMovieDlg.h"
#include "DnGuildWarTask.h"
#ifdef PRE_ADD_INSTANT_CASH_BUY
#include "DnInstantCashShopBuyDlg.h"
#include "DnCashShopTask.h"
#endif // PRE_ADD_INSTANT_CASH_BUY
#include "DnSkillGuideInitItemDlg.h"
#include "DnSkillTreeButtonDlg.h"
#include "DnSkillTreePreSetDlg.h"
#include "DnSkillTreeSetMessageDlg.h"
#include "DnJobInformationDlg.h"
#include "DnSkillClassInfoDlg.h"

#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#endif

//#undef PRE_ADD_SKILL_LEVELUP_RESERVATION

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
#include "DnTotalLevelSkillActivateListDlg.h"
#endif // PRE_ADD_TOTAL_LEVEL_SKILL


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const int SKILL_TREE_SCROLL_TRACK_PAGESIZE = 25;
const float SKILL_TREE_SCROLL_TRACK_RANGE = 40.0f;

CDnSkillTreeDlg::CDnSkillTreeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pSkillTreeContentDlg( NULL )
, m_pLifeSkillContent( NULL )
, m_pScrollBar( NULL )
, m_fOriContentDlgXPos( 0.0f )
, m_fOriContentDlgYPos( 0.0f )
, m_fContentWidth( 0.0f )
, m_fContentHeight( 0.0f )
, m_pContentListBox( NULL )
, m_iSelectedJobID( -1 )
, m_iSelectedJobBtnID( -1 )
, m_pSkillMovieDlg(NULL)
, m_iSkillTreeScrollPos( -1 )
, m_pStaticTitle( NULL )
, m_bGuildSkillMode( false )
, m_bSetGuildSkill( false )
, m_pLeaveSP(NULL)
, m_pWholeUsedSP(NULL)
#if defined( PRE_ADD_SKILL_RESET_COMMENT )
, m_pSkillGuideInitItemDlg( NULL )
, m_bResetComment( false )
#endif	// #if defined( PRE_ADD_SKILL_RESET_COMMENT )
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
, m_pTotalLevelSkillActivateDlg(NULL)
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
, m_pSkillTreeButtonDlg( NULL )
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
#if defined( PRE_ADD_PRESET_SKILLTREE )
, m_pSkillTreePreSetDlg( NULL )
, m_pSkillTreeSetMessageDlg( NULL )
, m_pPresetComboBox( NULL )
, m_bPresetSelect( false )
, m_bPresetDelEnable( false )
, m_bPresetModify( false )
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
{
	//m_pSkillTreeContentDlg = new CDnSkillTreeContentDlg( UI_TYPE_CHILD, this, -1, NULL );
	SecureZeroMemory( m_apJobRadioButton, sizeof(m_apJobRadioButton) );

	SecureZeroMemory( m_apDualSkillRadioButton , sizeof(m_apDualSkillRadioButton) ) ;
	m_iSelectedSkillPage = 0;
	m_fSkillPageDelay = -1.f;

	for(int i=DualSkill::Type::Primary; i< DualSkill::Type::MAX; i++ )
		m_bOpendSkillPage[i] = false;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	m_pTotalLevelInfo = NULL;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
}

CDnSkillTreeDlg::~CDnSkillTreeDlg(void)
{
	SAFE_DELETE( m_pLifeSkillContent );
	m_pContentListBox->RemoveAllItems();
	SAFE_DELETE( m_pSkillMovieDlg );

#if defined( PRE_ADD_SKILL_RESET_COMMENT )
	SAFE_DELETE( m_pSkillGuideInitItemDlg );
#endif	// #if defined( PRE_ADD_SKILL_RESET_COMMENT )

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	SAFE_DELETE(m_pTotalLevelSkillActivateDlg);
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
}

void CDnSkillTreeDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillTreeDlg.ui" ).c_str(), bShow );

	m_pSkillMovieDlg = new CDnSkillMovieDlg(UI_TYPE_CHILD, this);
	m_pSkillMovieDlg->Initialize(false);

	m_pLifeSkillContent = new CDnLifeSkillContent;
	if( m_pLifeSkillContent )
		m_pLifeSkillContent->SetSkillTreeDlg( this );

#if defined( PRE_ADD_SKILL_RESET_COMMENT )
	m_pSkillGuideInitItemDlg = new CDnSkillGuideInitItemDlg();
	m_pSkillGuideInitItemDlg->Initialize(false);
#endif	// #if defined( PRE_ADD_SKILL_RESET_COMMENT )
}

void CDnSkillTreeDlg::InitialUpdate()
{
	CEtUIDialog::InitialUpdate();

	//m_pSkillTreeContentDlg->InitialUpdate();

	// Get only the scrollbar of the created list box
	CEtUIListBox* pContentListBox = GetControl<CEtUIListBox>( "ID_LISTBOX0" );
	if( pContentListBox )
		pContentListBox->Show( false );
	m_pContentListBox = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIFE" );
	m_pScrollBar = m_pContentListBox->GetScrollBar();

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	m_pTotalLevelSkillContentListBox = GetControl<CEtUIListBoxEx>("ID_LISTBOX_SUBSKILL");
	if (m_pTotalLevelSkillContentListBox)
		m_pTotalLevelSkillContentListBox->Show(false);
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

	// Show only the scrollbar for now
	//pChildListBox->Show( false );
	//m_pScrollBar->Show( true );

	m_pScrollBar->SetPageSize( SKILL_TREE_SCROLL_TRACK_PAGESIZE );
	m_pScrollBar->SetTrackRange( 0, (int)SKILL_TREE_SCROLL_TRACK_RANGE );

	char acControlName[ 64 ] = { 0 };
	for( int i = 0; i < NUM_JOB_BTN; ++i )
	{
		sprintf_s( acControlName, 64, "ID_RBT_%d", i );
		m_apJobRadioButton[ i ] = GetControl<CEtUIRadioButton>( acControlName );
		m_apJobRadioButton[ i ]->Show( false );
		m_apJobRadioButton[ i ]->SetButtonID( i );
	}

	for( int i = 0; i < NUM_JOB_HIGHLIGHT; ++i )
	{
		sprintf_s( acControlName, 64, "ID_STATIC_BTLIGHT%d", i );
		m_pJobHighLightStatic[ i ] = GetControl<CEtUIStatic>( acControlName );
		m_pJobHighLightStatic[ i ]->Show( false );
	}

	for(int i=0; i< DualSkill::Type::MAX; i++)
	{
		sprintf_s( acControlName, 64, "ID_RBT_DUAL%d", i );
		m_apDualSkillRadioButton[ i ] = GetControl<CEtUIRadioButton>( acControlName );
		m_apDualSkillRadioButton[ i ]->Enable( false );
		m_apDualSkillRadioButton[ i ]->SetButtonID( i );
	}
	EnableSkillPage(DualSkill::Type::Primary);

	// Collection of static controls related to skill points.
	m_pLeaveSP = GetControl<CEtUIStatic>( "ID_SKILL_POINT1" );
	m_pWholeUsedSP = GetControl<CEtUIStatic>( "ID_SKILL_POINT0" );
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)

	m_pTotalLevelInfo = GetControl<CEtUIStatic>("ID_TEXT_TOTALLEVEL");
	UpdateTotalLevel();

	for( int i = 0; i < NUM_JOB_BTN; ++i )
	{
		bool bExist = false;
		CEtUIStatic* pStatic = NULL;
		sprintf_s( acControlName, 64, "ID_TEXT_JOB%d", i );
		pStatic = GetControl<CEtUIStatic>(acControlName, &bExist);
		if (bExist && pStatic)
			m_vlpUsedSPJobName.push_back( pStatic );

		sprintf_s( acControlName, 64, "ID_TEXT_JOB_ACCOUNT%d", i );
		pStatic = GetControl<CEtUIStatic>(acControlName, &bExist);
		if (bExist && pStatic)
			m_vlpUsedSPByJob.push_back( pStatic );

		sprintf_s( acControlName, 64, "ID_STATIC_LINE%d", i );
		pStatic = GetControl<CEtUIStatic>(acControlName, &bExist);
		if (bExist && pStatic)
			m_vlpJobLine.push_back( pStatic );

		sprintf_s( acControlName, 64, "ID_STATIC_TEXTLIGHT%d", i );
		pStatic = GetControl<CEtUIStatic>(acControlName, &bExist);
		if (bExist && pStatic)
			m_vlpHighLight.push_back( pStatic );
	}
#else
	for( int i = 0; i < NUM_JOB_BTN; ++i )
	{
		if( i < NUM_JOB_BTN-1 )
		{
			sprintf_s( acControlName, 64, "ID_TEXT_JOB%d", i );
			m_vlpUsedSPJobName.push_back( GetControl<CEtUIStatic>(acControlName) );

			sprintf_s( acControlName, 64, "ID_TEXT_JOB_ACCOUNT%d", i );
			m_vlpUsedSPByJob.push_back( GetControl<CEtUIStatic>(acControlName) );

			sprintf_s( acControlName, 64, "ID_STATIC_LINE%d", i );
			m_vlpJobLine.push_back( GetControl<CEtUIStatic>(acControlName) );

			sprintf_s( acControlName, 64, "ID_STATIC_TEXTLIGHT%d", i );
			m_vlpHighLight.push_back( GetControl<CEtUIStatic>(acControlName) );
		}
	}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

	m_SmartMoveEx.SetControl( m_apJobRadioButton[1] );
	m_pStaticTitle = GetControl<CEtUIStatic>( "ID_TEXT0" );

#if defined( PRE_REMOVE_GUILD_WAR_UI )
	GetSkillTask().LoadGuildWarSkillList( PvPCommon::Team::A );
	SetGuildSkillMode( true, false );
#endif	// #if defined( PRE_REMOVE_GUILD_WAR_UI )

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	m_pTotalLevelSkillActivateDlg = new CDnTotalLevelSkillActivateListDlg(UI_TYPE_CHILD, this);
	m_pTotalLevelSkillActivateDlg->Initialize(false);
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	ResetSkillReservation();
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

#if defined( PRE_ADD_PRESET_SKILLTREE )
	m_pPresetComboBox = GetControl<CEtUIComboBox>( "ID_COMBOBOX_TREELIST" );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
}

void CDnSkillTreeDlg::SetContentPanelDlg( CDnSkillTreeContentDlg* pSkillTreeContentDlg )
{
	m_pSkillTreeContentDlg = pSkillTreeContentDlg;
	m_pSkillTreeContentDlg->SetSkillTreeDlg( this );
}

void CDnSkillTreeDlg::_SetContentOffset( void )
{
	if( m_pSkillTreeContentDlg )
	{
		SUIDialogInfo ContentDlgInfo;
		m_pSkillTreeContentDlg->GetDlgInfo( ContentDlgInfo );

		if( 0.0f == m_fOriContentDlgXPos && 0.0f == m_fOriContentDlgYPos )
		{
			m_fOriContentDlgXPos = ContentDlgInfo.DlgCoord.fX;
			m_fOriContentDlgYPos = ContentDlgInfo.DlgCoord.fY;
		}

		const SUICoord& ContentListBoxCoord = m_pContentListBox->GetUICoord();
		float fPageSize = ContentListBoxCoord.fHeight / GetScreenHeightRatio() * GetEtDevice()->Height();
		m_pScrollBar->SetPageSize( (int)fPageSize );

		// Since the dialog is also multiplied by the dialog scale value, it must be considered together to get the exact dialog size.
		m_pScrollBar->SetTrackRange( 0, int(ContentDlgInfo.DlgCoord.fHeight * (float)GetEtDevice()->Height() / (float)DEFAULT_UI_SCREEN_HEIGHT * (float)DEFAULT_UI_SCREEN_HEIGHT) );
		if( -1 != m_iSkillTreeScrollPos )
		{
			m_pScrollBar->SetTrackPos( m_iSkillTreeScrollPos );
			m_iSkillTreeScrollPos = -1;
		}

		int iPos = m_pScrollBar->GetTrackPos();
		float fNowYOffsetRatio = (float)iPos / fPageSize;
		float fHeight = ContentListBoxCoord.fHeight / GetScreenHeightRatio() /* * GetEtDevice()->Height()*/;
		
		m_pSkillTreeContentDlg->SetPosition( m_fOriContentDlgXPos, m_fOriContentDlgYPos - fNowYOffsetRatio*fHeight );
	}
}

bool CDnSkillTreeDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_MOUSEWHEEL:
			{
				if( IsMouseInDlg() )
				{
					UINT uLines;
					SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
					int iScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
					OnMouseWheel( iScrollAmount );
				}
			}
			break;
	}
	
	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnSkillTreeDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg/* = 0*/ )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	SetCmdControlName( pControl->GetControlName() );

	if( EVENT_BUTTON_CLICKED == nCommand && IsCmdControl( "ID_BT_CLASSINFO" ) )
	{
		int nJobID = ( m_iSelectedJobID > 0 ) ? m_iSelectedJobID : ( CDnActor::s_hLocalActor ? static_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() )->GetJobClassID() : 1 );
		if( GetInterface().GetJobInformationMainDlg() )
		{
			GetInterface().GetJobInformationMainDlg()->SetJobInfo( nJobID );
			GetInterface().GetJobInformationMainDlg()->Show( true );
		}
		if( GetInterface().GetSkillClassInfoDlg() )
		{
			DNTableFileFormat *pJobSox = GetDNTable( CDnTableDB::TJOB );
			if( pJobSox && pJobSox->IsExistItem( nJobID ) )
			{
				int nJobNameID = pJobSox->GetFieldFromLablePtr( nJobID, "_JobName" )->GetInteger();
				GetInterface().GetSkillClassInfoDlg()->SetClassInfoText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nJobNameID ) );
			}
			GetInterface().GetSkillClassInfoDlg()->Show( true );
		}
		return;
	}

	if( EVENT_RADIOBUTTON_CHANGED == nCommand )
	{
		if( strstr( pControl->GetControlName(), "ID_RBT_DUAL" ) )
		{
			int iButtonID = static_cast<CEtUIRadioButton*>( pControl )->GetButtonID();

			if(m_iSelectedSkillPage != iButtonID && bTriggeredByUser)
			{
				m_iSelectedSkillPage = iButtonID;
				GetSkillTask().SendChangeSkillPage(m_iSelectedSkillPage);
				SetSkillPageDelay(1.5f);
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
				ResetButtonCommand();
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
			}
		}
		if( strstr( pControl->GetControlName(), "ID_RBT" ) && !strstr( pControl->GetControlName(), "ID_RBT_DUAL" ) )
		{
			int iButtonID = static_cast<CEtUIRadioButton*>( pControl )->GetButtonID();
			
			if( m_iSelectedJobBtnID != iButtonID )
				m_pScrollBar->SetTrackPos( 0 );	// Reset scroll offset moved on the previous page.
		
			m_iSelectedJobBtnID = iButtonID;
			ShowGuildSkillPointInfo();
			_UpdateSelectionContentDlg();

			if (m_pSkillMovieDlg && m_pSkillMovieDlg->IsShow())
				ShowSkillMovieDlg(true, L"", std::string());

			if( drag::IsValid() )
			{
				CDnSlotButton *pDragButton = static_cast<CDnSlotButton*>(drag::GetControl());
				if( pDragButton->GetSlotType() == ST_SKILL )
					drag::ReleaseControl();
			}
		}
	}
	else if( nCommand == EVENT_BUTTON_CLICKED )
	{
#ifdef PRE_ADD_INSTANT_CASH_BUY
		if( IsCmdControl( "UI_BUTTON_BUYSHORTCUT" ) )
		{
			GetInterface().ShowInstantCashShopBuyDlg( true, INSTANT_BUY_SKILL, this );
			return;
		}
#endif // PRE_ADD_INSTANT_CASH_BUY

#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		else if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_SKILL);
		}
#endif
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED )
	{
#if defined( PRE_ADD_PRESET_SKILLTREE )
		if( IsCmdControl( "ID_COMBOBOX_TREELIST" ) )
		{
			SComboBoxItem* pItem = m_pPresetComboBox->GetSelectedItem();
			if( NULL == pItem )
				return;

			PresetSkillProcess( pItem->nValue );
		}
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
	}

	//m_pSkillTreeContentDlg->ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnSkillTreeDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
#ifdef PRE_ADD_INSTANT_CASH_BUY
	if( nID == INSTANT_CASHSHOP_BUY_DIALOG )
	{
		if( nCommand == 1 )
		{
			Show( false );
		}
	}
#endif // PRE_ADD_INSTANT_CASH_BUY
}

void CDnSkillTreeDlg::Render( float fElapsedTime )
{
	CEtUIDialog::Render( fElapsedTime );

	if( IsShow() )
	{
		if( m_iSelectedJobBtnID > 0 )
		{
			// Move the dialog position according to the scroll bar position.
			_SetContentOffset();

			// The width of the list box created for the scroll bar is the content size.
			// Use this to set up the viewport.
			
			/*
			EtViewPort OriViewport;
			EtViewPort ContentViewport;
			GetEtDevice()->GetViewport( &OriViewport );

			const SUICoord& ContentListBoxCoord = m_pContentListBox->GetUICoord();
			ContentViewport = OriViewport;
			ContentViewport.X = int((ContentListBoxCoord.fX+GetXCoord()) / GetScreenWidthRatio() * GetEtDevice()->Width());
			ContentViewport.Y = int((ContentListBoxCoord.fY+GetYCoord()) / GetScreenHeightRatio() * GetEtDevice()->Height());
			ContentViewport.Width = int(ContentListBoxCoord.fWidth / GetScreenWidthRatio() * GetEtDevice()->Width() );
			ContentViewport.Height = int(ContentListBoxCoord.fHeight / GetScreenHeightRatio() * GetEtDevice()->Height() );
			CEtSprite::GetInstance().Flush();
			GetEtDevice()->SetViewport( &ContentViewport );

			m_pSkillTreeContentDlg->Show( true );
			m_pSkillTreeContentDlg->Render( fElapsedTime );
			m_pSkillTreeContentDlg->Show( false );

			CEtSprite::GetInstance().Flush();
			GetEtDevice()->SetViewport( &OriViewport );
			*/

			RECT oldRect, newRect;
			GetEtDevice()->GetScissorRect( &oldRect );

			const SUICoord& ContentListBoxCoord = m_pContentListBox->GetUICoord();
			newRect.left = int((ContentListBoxCoord.fX+GetXCoord()) / GetScreenWidthRatio() * GetEtDevice()->Width());
			newRect.top = int((ContentListBoxCoord.fY+GetYCoord()) / GetScreenHeightRatio() * GetEtDevice()->Height());
			newRect.right = newRect.left + int(ContentListBoxCoord.fWidth / GetScreenWidthRatio() * GetEtDevice()->Width() );
			newRect.bottom = newRect.top + int(ContentListBoxCoord.fHeight / GetScreenHeightRatio() * GetEtDevice()->Height() );

			CEtSprite::GetInstance().Flush();
			//GetEtDevice()->SetViewport( &ContentViewport );
			GetEtDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE);
			GetEtDevice()->SetScissorRect( &newRect );

			m_pSkillTreeContentDlg->Show( true );
			m_pSkillTreeContentDlg->Render( fElapsedTime );
			m_pSkillTreeContentDlg->Show( false );

			CEtSprite::GetInstance().Flush();
			//GetEtDevice()->SetViewport( &OriViewport );
			GetEtDevice()->SetScissorRect( &oldRect );
			GetEtDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE);

#if defined(PRE_ADD_SKILL_RESET_COMMENT)
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
			if( true == m_bResetComment && m_iSelectedJobBtnID != TOTAL_LEVEL_SKILL )
			{
				m_pSkillGuideInitItemDlg->Show( true );
				m_pSkillGuideInitItemDlg->Render( fElapsedTime );
				m_pSkillGuideInitItemDlg->Show( false );
			}

#else
			if( true == m_bResetComment )
			{
				m_pSkillGuideInitItemDlg->Show( true );
				m_pSkillGuideInitItemDlg->Render( fElapsedTime );
				m_pSkillGuideInitItemDlg->Show( false );
			}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
#endif // PRE_ADD_SKILL_RESET_COMMENT

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
			m_pSkillTreeButtonDlg->Show( true );
			m_pSkillTreeButtonDlg->Render( fElapsedTime );
			m_pSkillTreeButtonDlg->Show( false );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

#if defined( PRE_ADD_PRESET_SKILLTREE )
			m_pSkillTreePreSetDlg->Show( true );
			m_pSkillTreePreSetDlg->Render( fElapsedTime );
			m_pSkillTreePreSetDlg->Show( false );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
		}
	}
}

void CDnSkillTreeDlg::Process( float fElapsedTime )
{
	m_SmartMoveEx.Process();
	CDnCustomDlg::Process( fElapsedTime );

	ProcessSkillPageDelay(fElapsedTime);
}

void CDnSkillTreeDlg::Show( bool bShow )
{
	if( m_bShow == bShow ) return;

	bool bWhenClose = ((m_bShow == true ) && (bShow == false ));
	CEtUIDialog::Show( bShow );

	if( m_pSkillTreeContentDlg == NULL )
		return;

	// Reset newly acquired skill flag when closing.
	if( bWhenClose )
		GetSkillTask().ResetNewAcquireSkills();
	else
	{
		if( CDnActor::s_hLocalActor )
			UpdateView();
	}

	m_pScrollBar->SetTrackPos( 0 );
	_SetContentOffset();

	// Set content dialog to receive/block messages the same way as when skill tree dialog appears.
	if( m_iSelectedJobBtnID > 0 )
	{
		m_pSkillTreeContentDlg->SetAcceptInputMsgWhenHide( bShow );
		m_pSkillTreeContentDlg->SetElementDialogShowState( bShow );

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		m_pSkillTreeButtonDlg->SetAcceptInputMsgWhenHide( bShow );
		m_pSkillTreeButtonDlg->SetElementDialogShowState( bShow );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

#if defined( PRE_ADD_PRESET_SKILLTREE )
		m_pSkillTreePreSetDlg->SetAcceptInputMsgWhenHide( bShow );
		m_pSkillTreePreSetDlg->SetElementDialogShowState( bShow );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
	}
	else
	{
		m_pSkillTreeContentDlg->SetAcceptInputMsgWhenHide( false );
		m_pSkillTreeContentDlg->SetElementDialogShowState( false );

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		m_pSkillTreeButtonDlg->SetAcceptInputMsgWhenHide( false );
		m_pSkillTreeButtonDlg->SetElementDialogShowState( false );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

#if defined( PRE_ADD_PRESET_SKILLTREE )
		m_pSkillTreePreSetDlg->SetAcceptInputMsgWhenHide( false );
		m_pSkillTreePreSetDlg->SetElementDialogShowState( false );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
	}

	ShowChildDialog( m_pSkillMovieDlg, bShow );

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	ResetSkillReservation();
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

	if( !bShow )
	{
		if( drag::IsValid() )
		{
			CDnSlotButton *pDragButton = static_cast<CDnSlotButton*>(drag::GetControl());
			if( pDragButton->GetSlotType() == ST_SKILL )
				drag::ReleaseControl();
		}
		m_pSkillTreeContentDlg->Hide_SlotTooltipDlg();

		m_pSkillTreeContentDlg->CloseChildDialog();
		GetInterface().CloseBlind();
		//GetInterface().CloseNpcTalkReturnDlg();
		ReleaseMouseEnterControl();

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
		if( m_pTotalLevelSkillActivateDlg )
			m_pTotalLevelSkillActivateDlg->Show( bShow );
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
	}
	else
	{
		m_SmartMoveEx.MoveCursor();
#ifdef PRE_ADD_INSTANT_CASH_BUY
		int nShowType = GetCashShopTask().GetInstantCashBuyEnableType( INSTANT_BUY_SKILL );
		if( nShowType == INSTANT_BUY_HIDE )
		{
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Show( false );
			GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" )->Show( false );
		}
		else if( nShowType == INSTANT_BUY_SHOW )
		{
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Show( true );
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Enable( true );
			GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" )->Show( true );
		}
		else if( nShowType == INSTANT_BUY_DISABLE )
		{
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Show( true );
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Enable( false );
			GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" )->Show( true );
		}
#endif // PRE_ADD_INSTANT_CASH_BUY

#if defined( PRE_ADD_SKILL_RESET_COMMENT )
		m_bResetComment = IsResetCommentShow();
#endif	// #if defined( PRE_ADD_SKILL_RESET_COMMENT )
	}

	if( false == bShow )
		m_pSkillTreeContentDlg->SetUnlockByMoneyMode( bShow );

	if( m_pCallback )
		m_pCallback->OnUICallbackProc( GetDialogID(), bShow?EVENT_DIALOG_SHOW:EVENT_DIALOG_HIDE, NULL );


	for(int i=DualSkill::Type::Primary; i< DualSkill::Type::MAX; i++ )
	{
		if( m_bOpendSkillPage[i] == true && m_iSelectedSkillPage != i)
			m_apDualSkillRadioButton[i]->Enable( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeEnum::MapTypeVillage );
	}
	if( m_iSelectedJobBtnID == 4 )
		UpdateGuildWarSkillPointInfo();
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	UpdateTotalLevel();
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

}

void CDnSkillTreeDlg::OnMouseWheel( int nScrollAmount )
{
	if( m_pScrollBar->IsEnable() )
		m_pScrollBar->SetTrackPos( m_pScrollBar->GetTrackPos()-nScrollAmount*10 );
}

void CDnSkillTreeDlg::GetContentDialog( std::vector<CEtUIDialog*>& vlpContentDialogs )
{
	if( m_pSkillTreeContentDlg )
		vlpContentDialogs.push_back( m_pSkillTreeContentDlg );

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	if( m_pSkillTreeButtonDlg )
		vlpContentDialogs.push_back( m_pSkillTreeButtonDlg );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

#if defined( PRE_ADD_PRESET_SKILLTREE )
	if( m_pSkillTreePreSetDlg )
		vlpContentDialogs.push_back( m_pSkillTreePreSetDlg );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
}

void CDnSkillTreeDlg::GetContentDialogCoord( /*IN OUT*/ SUICoord& Coord )
{
	Coord = m_pContentListBox->GetUICoord();
}

void CDnSkillTreeDlg::UpdateView( void )
{
	// Set radio buttons per history. ////////////////////////////////////
	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	_ASSERT( pPlayer );

	vector<int> vlJobHistory;
	int iNumJobHistory = pPlayer->GetJobHistory( vlJobHistory );
	_ASSERT( 0 < iNumJobHistory && iNumJobHistory < NUM_JOB_BTN );

	if( -1 == m_iSelectedJobID || iNumJobHistory < m_iSelectedJobBtnID )
	{
		m_iSelectedJobID = pPlayer->GetJobClassID();
		m_iSelectedJobBtnID = iNumJobHistory;
	}

	// The first index is reserved for life skills.
	m_apJobRadioButton[ 0 ]->Show( true );

	for( int i = 1; i < NUM_JOB_BTN; ++i )
		m_apJobRadioButton[ i ]->Show( false );

	for( int i = 0; i < (int)m_vlpJobLine.size(); ++i )
		m_vlpJobLine.at( i )->Show( false );

	for( int i = 0; i < (int)m_vlpUsedSPJobName.size(); ++i )
		m_vlpUsedSPJobName.at( i )->Show( false );
	
	for( int i = 0; i < (int)m_vlpUsedSPByJob.size(); ++i )
		m_vlpUsedSPByJob.at( i )->Show( false );

	for( int i = 0; i < (int)m_vlpUsedSPByJob.size(); ++i )
		m_vlpHighLight.at( i )->Show( false );

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TJOB );
	for( int i = 0; i < iNumJobHistory; ++i )
	{
		m_apJobRadioButton[ i+1 ]->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( vlJobHistory.at(i), "_JobName" )->GetInteger()) );
		m_apJobRadioButton[ i+1 ]->Show( true );
	}
	if( m_bGuildSkillMode )
	{
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
		if( m_apJobRadioButton[ GUILD_BTN ]->IsChecked() )
			m_apJobRadioButton[ GUILD_BTN ]->SetChecked( true, true, false );
		else if( m_apJobRadioButton[ TOTAL_LEVEL_SKILL ]->IsChecked() )
			m_apJobRadioButton[ TOTAL_LEVEL_SKILL ]->SetChecked( true, true, false );
		else
			m_apJobRadioButton[ m_iSelectedJobBtnID ]->SetChecked( true, true, false );
#else
		if( m_apJobRadioButton[ NUM_JOB_BTN-1 ]->IsChecked() )
			m_apJobRadioButton[ NUM_JOB_BTN-1 ]->SetChecked( true, true, false );
		else
			m_apJobRadioButton[ m_iSelectedJobBtnID ]->SetChecked( true, true, false );
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
	}
	else
		m_apJobRadioButton[ m_iSelectedJobBtnID ]->SetChecked( true, true, false );

	ShowGuildSkillPointInfo();

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	if( m_bGuildSkillMode )
	{
		m_apJobRadioButton[ GUILD_BTN ]->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126238 ) );
		m_apJobRadioButton[ GUILD_BTN ]->Show( true );
	}
	else
		m_apJobRadioButton[ GUILD_BTN ]->Show( false );
#else
	if( m_bGuildSkillMode )
	{
		m_apJobRadioButton[ NUM_JOB_BTN-1 ]->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126238 ) );
		m_apJobRadioButton[ NUM_JOB_BTN-1 ]->Show( true );
	}
	else
		m_apJobRadioButton[ NUM_JOB_BTN-1 ]->Show( false );

#endif // PRE_ADD_TOTAL_LEVEL_SKILL
	//_UpdateSelectionContentDlg();

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	//Display unified level skill
#ifdef PRE_REMOVE_TOTAL_LEVEL_SKILL
	m_apJobRadioButton[ TOTAL_LEVEL_SKILL ]->Show( false );
#else // PRE_REMOVE_TOTAL_LEVEL_SKILL
	m_apJobRadioButton[ TOTAL_LEVEL_SKILL ]->Show( true );
#endif // PRE_REMOVE_TOTAL_LEVEL_SKILL
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined( PRE_ADD_PRESET_SKILLTREE )
	RefreshSkillTreePresetComboBox();
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
}

void CDnSkillTreeDlg::ResetCheckedButton()
{
	for( int i=0; i<NUM_JOB_BTN; i++ )
		m_apJobRadioButton[ i ]->SetChecked( false );

	m_iSelectedJobID = -1;
}

void CDnSkillTreeDlg::SetGuildSkillMode( bool bGuildSkillMode, bool bSetGuildSkill )
{
	m_bGuildSkillMode = bGuildSkillMode;
	m_bSetGuildSkill = bSetGuildSkill;
}

void CDnSkillTreeDlg::UpdateGuildWarSkillPointInfo()
{
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	int nResevationGuildSkillPoint = GetSkillTask().GetReservationGuildSkillCount();
	if( 0 != nResevationGuildSkillPoint )
	{
		WCHAR wszLeavePoint[256] = {0,};
		swprintf_s( wszLeavePoint, _countof(wszLeavePoint), L"%d/%d", nResevationGuildSkillPoint, GetGuildWarTask().GetGuildWarSkillLeavePoint() );

		m_pWholeUsedSP->SetIntToText( GetGuildWarTask().GetGuildWarSkillUsedPoint() );
		m_pLeaveSP->SetText( wszLeavePoint );
	}
	else
	{
		m_pWholeUsedSP->SetIntToText( GetGuildWarTask().GetGuildWarSkillUsedPoint() );
		m_pLeaveSP->SetIntToText( GetGuildWarTask().GetGuildWarSkillLeavePoint() );
	}
#else
	m_pWholeUsedSP->SetIntToText( GetGuildWarTask().GetGuildWarSkillUsedPoint() );
	m_pLeaveSP->SetIntToText( GetGuildWarTask().GetGuildWarSkillLeavePoint() );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

	if( GetGuildWarTask().GetGuildWarSkillLeavePoint() == 0 )
	{
		m_bSetGuildSkill = false;
		UpdateGuildWarSkillTreeContent();
	}
}

void CDnSkillTreeDlg::ShowGuildSkillPointInfo()
{
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	if( m_bGuildSkillMode && m_apJobRadioButton[ GUILD_BTN ]->IsChecked() )
#else
	if( m_bGuildSkillMode && m_apJobRadioButton[ NUM_JOB_BTN-1 ]->IsChecked() )
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
	{
		UpdateGuildWarSkillPointInfo();

		for( int i = 0; i < (int)m_vlpUsedSPJobName.size(); ++i )
			m_vlpUsedSPJobName.at( i )->Show( false );

		for( int i = 0; i < (int)m_vlpUsedSPByJob.size(); ++i )
		{
			m_vlpUsedSPByJob.at( i )->SetText( L"" );
			m_vlpUsedSPByJob.at( i )->Show( false );
		}

		m_pScrollBar->Show( false );
	}
	else
	{
		CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
		_ASSERT( pPlayer );
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TJOB );
		_ASSERT( pSox );

		vector<int> vlJobHistory;
		int iNumJobHistory = pPlayer->GetJobHistory( vlJobHistory );

		// Total points used.
		m_pWholeUsedSP->SetIntToText( GetSkillTask().GetWholeUsedSkillPoint() );

		// Remaining points.
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		int nReservationSkillNeedSP = GetSkillTask().GetReservationSkillNeedSP();

		if( 0 < nReservationSkillNeedSP )
		{
			int nSkillPoint = GetSkillTask().GetSkillPoint();

			nSkillPoint -= nReservationSkillNeedSP;

			WCHAR wszString[64] = {0,};
			swprintf_s( wszString, _countof(wszString), L"%d/%d", nReservationSkillNeedSP, nSkillPoint );

			m_pLeaveSP->SetText( wszString );
		}
		else
			m_pLeaveSP->SetIntToText( GetSkillTask().GetSkillPoint() );
#else	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		m_pLeaveSP->SetIntToText( GetSkillTask().GetSkillPoint() );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

		// Points spent per class.
		for( int i = 0; i < iNumJobHistory; ++i )
		{
			// Also set used skill points on static text.
			int iJobID = vlJobHistory.at(i);
			m_vlpUsedSPJobName.at( i )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( iJobID, "_JobName" )->GetInteger()) );

			_ASSERT( dynamic_cast<CDnLocalPlayerActor*>(pPlayer) );
			int iUsedSkillPointInThisJob = GetSkillTask().GetUsedSkillPointInThisJob( iJobID );
			wchar_t awcBuffer[ 256 ] = { 0 };

			// Calculate usable points per class by ratio.
			int iWholeAvailSPByJob = static_cast<CDnLocalPlayerActor*>(pPlayer)->GetSPUsingLimitByJobArrayIndex( i );

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
			if( true == GetSkillTask().IsJobSP_SkillReservation( iJobID ) )
				m_vlpUsedSPByJob.at( i )->SetTextColor( textcolor::ORANGE );
			else
				m_vlpUsedSPByJob.at( i )->SetTextColor( textcolor::WHITE );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

			swprintf_s( awcBuffer, L"%d / %d", iUsedSkillPointInThisJob, iWholeAvailSPByJob );
			m_vlpUsedSPByJob.at( i )->SetText( awcBuffer );

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
			if (i < GUILD_BTN)
#else
			if( i < NUM_JOB_BTN-1 )
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
				m_vlpJobLine.at( i )->Show( true );

			m_vlpUsedSPJobName.at( i )->Show( true );
			m_vlpUsedSPByJob.at( i )->Show( true );
		}

		m_pScrollBar->Show( true );
	}
}

void CDnSkillTreeDlg::UpdateGuildWarSkillTreeContent()
{
	if( m_iSelectedJobBtnID == 4 && m_pSkillTreeContentDlg )
		m_pSkillTreeContentDlg->UpdateGuildWarSkillTree( m_bSetGuildSkill ); 
}

void CDnSkillTreeDlg::_UpdateSelectionContentDlg()
{
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	m_pTotalLevelSkillActivateDlg->Show(false);

	m_pTotalLevelSkillContentListBox->Show(false);
	m_pTotalLevelSkillContentListBox->Enable(false);

	m_pContentListBox = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIFE" );

	m_pContentListBox->Show(true);
	m_pContentListBox->Enable(true);

	//Retrieve and keep the original scrollbar..
	if (m_pContentListBox)
		m_pScrollBar = m_pContentListBox->GetScrollBar();

#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined( PRE_ADD_PRESET_SKILLTREE )
	m_pPresetComboBox->Enable( false );
	m_pSkillTreePreSetDlg->Enable( false );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

	bool bAcceptMsg = false;
	if( 0 < m_iSelectedJobBtnID )
	{
		if( m_iSelectedJobBtnID < 4 )
		{
			CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
			_ASSERT( pPlayer );

			vector<int> vlJobHistory;
			pPlayer->GetJobHistory( vlJobHistory );

			// Account for extra index offset because the front slot is a sub-skill.
			_ASSERT( 1 <= m_iSelectedJobBtnID && m_iSelectedJobBtnID < (int)vlJobHistory.size()+1 );
			m_iSelectedJobID = vlJobHistory.at(m_iSelectedJobBtnID-1);

			m_pSkillTreeContentDlg->UpdateSkillTree();
			m_pScrollBar->Show( true );
			m_pScrollBar->Enable( true );

			m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 214 ) );

#if defined( PRE_ADD_PRESET_SKILLTREE )
			m_pPresetComboBox->Enable( true );
			ButtonStateSkillTreePreset();
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
		}
		else
		{
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
			switch(m_iSelectedJobBtnID)
			{
			case GUILD_BTN:
				{
					m_pSkillTreeContentDlg->UpdateGuildWarSkillTree( m_bSetGuildSkill );
					m_pScrollBar->Show( false );
					m_pScrollBar->Enable( false );

					m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000032422 ) );
				}
				break;
			case TOTAL_LEVEL_SKILL:
				{
					
					if (m_pTotalLevelSkillContentListBox)
						m_pScrollBar = m_pTotalLevelSkillContentListBox->GetScrollBar();

					m_pSkillTreeContentDlg->UpdateTotalLevelSkillTree(true);
					m_pScrollBar->Show( true );
					m_pScrollBar->Enable( true );

					//Disable existing one..
					m_pContentListBox->Show(false);
					m_pContentListBox->Enable(false);

					m_pContentListBox = m_pTotalLevelSkillContentListBox;
					m_pTotalLevelSkillContentListBox->Show(true);
					m_pTotalLevelSkillContentListBox->Enable(true);

					m_pTotalLevelSkillActivateDlg->Show(true);

					m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1821 ) );
				}
				break;
			}
#else
			m_pSkillTreeContentDlg->UpdateGuildWarSkillTree( m_bSetGuildSkill );
			m_pScrollBar->Show( false );
			m_pScrollBar->Enable( false );

			m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000032422 ) );
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
		}
		m_iSkillTreeScrollPos = m_pScrollBar->GetTrackPos();		// Save the current scroll position before it gets lost by the statement below.
		m_pContentListBox->RemoveAllItems();

		bAcceptMsg = true;
	}
	else
	{
		m_pScrollBar = m_pContentListBox->GetScrollBar();

		m_pScrollBar->Show( false );
		m_pScrollBar->Enable( false );

		if( m_pLifeSkillContent )
			m_pLifeSkillContent->RefreshInfo();

		bAcceptMsg = false;
	}

	if( m_bShow )
	{
		m_pSkillTreeContentDlg->SetAcceptInputMsgWhenHide( bAcceptMsg );
		m_pSkillTreeContentDlg->SetElementDialogShowState( bAcceptMsg );

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		m_pSkillTreeButtonDlg->SetAcceptInputMsgWhenHide( bAcceptMsg );
		m_pSkillTreeButtonDlg->SetElementDialogShowState( bAcceptMsg );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

#if defined( PRE_ADD_PRESET_SKILLTREE )
		m_pSkillTreePreSetDlg->SetAcceptInputMsgWhenHide( bAcceptMsg );
		m_pSkillTreePreSetDlg->SetElementDialogShowState( bAcceptMsg );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
	}
}

void CDnSkillTreeDlg::SetUnlockByMoneyMode( void )
{
	if( m_pSkillTreeContentDlg )
	{
		UpdateView();
		m_pSkillTreeContentDlg->SetUnlockByMoneyMode( true );
	}
}

bool CDnSkillTreeDlg::IsUnlockByMoneyMode()
{
	if( m_pSkillTreeContentDlg )
	{
		return m_pSkillTreeContentDlg->IsUnlockByMoneyMode();
	}
	return false;
}

void CDnSkillTreeDlg::ShowSkillMovieDlg(bool bShow, const std::wstring& skillName, const std::string& fileName)
{
	if (m_pSkillMovieDlg)
		m_pSkillMovieDlg->Show(bShow);

	if (bShow)
		m_pSkillMovieDlg->Play(skillName, fileName);
}

void CDnSkillTreeDlg::LifeSkillAdd( CSecondarySkill* pAddSecondarySkill )
{
	if( pAddSecondarySkill == NULL )
		return;
	
	if( m_pLifeSkillContent )
		m_pLifeSkillContent->AddLifeSkill( pAddSecondarySkill );
}

void CDnSkillTreeDlg::LifeSkillDelete( int nSkillID )
{
	if( m_pContentListBox == NULL )
		return;

	for( int i=0; i<m_pContentListBox->GetSize(); i++)
	{
		CDnLifeSkillItem *pItem = m_pContentListBox->GetItem<CDnLifeSkillItem>( i );
		if( pItem->GetLifeSkillInfoData().m_nSkillID == nSkillID )
		{
			m_pContentListBox->RemoveItem( i );
			break;
		}
	}
}

void CDnSkillTreeDlg::LifeSkillUpdateExp( int nSkillID, SecondarySkill::Grade::eType eGrade, int nLevel, int nExp )
{
	if( m_pContentListBox == NULL )
		return;

	for( int i=0; i<m_pContentListBox->GetSize(); i++)
	{
		CDnLifeSkillItem *pItem = m_pContentListBox->GetItem<CDnLifeSkillItem>( i );
		if( pItem->GetLifeSkillInfoData().m_nSkillID == nSkillID )
		{
			pItem->GetLifeSkillInfoData().m_eGrade = eGrade;
			pItem->GetLifeSkillInfoData().m_nLevel = nLevel;
			pItem->GetLifeSkillInfoData().m_nExp = nExp;
			pItem->SetUIInfo();
			break;
		}
	}
}

void CDnSkillTreeDlg::SetSkillPageDelay(float fDelay)
{
	m_fSkillPageDelay = fDelay;

	for(int i=DualSkill::Type::Primary; i< DualSkill::Type::MAX; i++ )
	{
		if( m_bOpendSkillPage[i] == true && m_iSelectedSkillPage != i)
			m_apDualSkillRadioButton[i]->Enable( false );
	}
}

void CDnSkillTreeDlg::ProcessSkillPageDelay(float fElapsedTime)
{
	if(m_fSkillPageDelay > 0 )
		m_fSkillPageDelay -= fElapsedTime;
	else if(m_fSkillPageDelay != -1.f)
	{
		m_fSkillPageDelay = -1.f;
		for(int i=DualSkill::Type::Primary; i< DualSkill::Type::MAX; i++ )
		{
			if( m_bOpendSkillPage[i] == true )
				m_apDualSkillRadioButton[i]->Enable( true );
		}
	}
}

void CDnSkillTreeDlg::SelectSkillPage(int nSkillPage)
{
	if( nSkillPage >= DualSkill::Type::MAX || nSkillPage < DualSkill::Type::Primary )
		return;

	if(m_apDualSkillRadioButton[nSkillPage])
	{
		m_apDualSkillRadioButton[nSkillPage]->SetChecked( true, true, false );
		m_apDualSkillRadioButton[nSkillPage]->Enable(true);
	}

	m_iSelectedSkillPage = nSkillPage;
}

void CDnSkillTreeDlg::EnableSkillPage(int nSkillPage , bool bEnable)
{
	if( nSkillPage < DualSkill::Type::Primary || nSkillPage >= DualSkill::Type::MAX )
		return;

	m_apDualSkillRadioButton[nSkillPage]->Enable(bEnable);
	m_bOpendSkillPage[nSkillPage] = true;
}

bool CDnSkillTreeDlg::IsEnableSkillPage(int  nSkillPage)
{
	if( nSkillPage >= DualSkill::Type::MAX || nSkillPage < DualSkill::Type::Primary )
		return false;

	return m_bOpendSkillPage[nSkillPage];
}

void CDnSkillTreeDlg::_Show_Highlight_Job( const int nNeedJobClassID )
{
	if( !CDnActor::s_hLocalActor )
		return;

	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	_ASSERT( pPlayer );

	vector<int> vlJobHistory;
	if (pPlayer)
		pPlayer->GetJobHistory( vlJobHistory );

	int nHighlightIndex = -1;
	for( DWORD itr = 0; itr < vlJobHistory.size(); ++itr )
	{
		if( nNeedJobClassID == vlJobHistory.at(itr) )
		{
			nHighlightIndex = itr;
			break;
		}
	}

	if( -1 == nHighlightIndex )
		return;

	m_pJobHighLightStatic[nHighlightIndex]->Show( true );
}

void CDnSkillTreeDlg::_Hide_Highlight_Job()
{
	for( DWORD itr = 0; itr < NUM_JOB_HIGHLIGHT; ++itr )
		m_pJobHighLightStatic[itr]->Show( false );
}

void CDnSkillTreeDlg::_Show_Highlight_SP( std::vector<int> & nNeedSPValues )
{
	if( !CDnActor::s_hLocalActor )
		return;

	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	_ASSERT( pPlayer );

	vector<int> vlJobHistory;
	int iNumJobHistory = pPlayer ? pPlayer->GetJobHistory( vlJobHistory ) : 0;

	vector<int> vlUsedSkillPointThisJob;
	vlUsedSkillPointThisJob.reserve( m_vlpHighLight.size() );
	for( int i = 0; i < iNumJobHistory; ++i )
	{
		int iJobID = vlJobHistory.at(i);
		int iUsedSkillPointInThisJob = GetSkillTask().GetUsedSkillPointInThisJob( iJobID );
		vlUsedSkillPointThisJob.push_back( iUsedSkillPointInThisJob );
	}

	for( DWORD itr = 0; itr < m_vlpHighLight.size(); ++itr )
	{
		if( nNeedSPValues.size() <= itr )
			break;

		if( 0 < nNeedSPValues[itr] && nNeedSPValues[itr] > vlUsedSkillPointThisJob[itr] )
			m_vlpHighLight[itr]->Show( true );
	}
}

void CDnSkillTreeDlg::_Hide_Highlight_SP()
{
	for( DWORD itr = 0; itr < m_vlpHighLight.size(); ++itr )
		m_vlpHighLight[itr]->Show( false );
}

#if defined( PRE_ADD_SKILL_RESET_COMMENT )
bool CDnSkillTreeDlg::IsResetCommentShow()
{
	if( !CDnActor::s_hLocalActor )
		return false;

#if defined( PRE_MOD_68531 )
	if( true == CDnActor::s_hLocalActor->ExistSkillLevelUpValue() )
	{
		m_pSkillGuideInitItemDlg->SetItemSkillComment();
		return true;
	}
#endif	// #if defined( PRE_MOD_68531 )

	const int nPlayer_Level = CDnActor::s_hLocalActor->GetLevel();
	const int nResetComment_Level = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Skill_ResetComment_Level );

	if( -1 > nResetComment_Level )
		return false;

	if( nPlayer_Level <= nResetComment_Level )
	{
		m_pSkillGuideInitItemDlg->SetResetGuideComment();
		return true;
	}

	return false;
}
#endif	// #if defined( PRE_ADD_SKILL_RESET_COMMENT )

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
void CDnSkillTreeDlg::SetSkillButtonDlg( CDnSkillTreeButtonDlg * pSkillTreeButtonDlg )
{
	m_pSkillTreeButtonDlg = pSkillTreeButtonDlg;

	if( m_pSkillTreeButtonDlg )
		m_pSkillTreeButtonDlg->SetSkillTreeDlg( this );
}

void CDnSkillTreeDlg::ButtonStateSkillReservation()
{
	bool bEnable = GetSkillTask().IsClear_SkillLevelUp();

	if( m_pSkillTreeButtonDlg )
		m_pSkillTreeButtonDlg->SetButtonEnable( !bEnable );

#if defined( PRE_ADD_PRESET_SKILLTREE )
	ButtonStateSkillTreePreset();
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
}

void CDnSkillTreeDlg::ResetSkillReservation()
{
	GetSkillTask().Clear_SkillLevelUp();
	ButtonStateSkillReservation();
}

void CDnSkillTreeDlg::ResetButtonCommand()
{
	ResetSkillReservation();
	if( m_bGuildSkillMode && m_apJobRadioButton[ GUILD_BTN ]->IsChecked() )
		UpdateGuildWarSkillTreeContent();
	else if (m_apJobRadioButton[ TOTAL_LEVEL_SKILL ]->IsChecked())
		UpdateTotalLevelSkillTreeContent();
	else
		m_pSkillTreeContentDlg->UpdateSkillTree();

	ShowGuildSkillPointInfo();
}

void CDnSkillTreeDlg::ApplyButtonCommand()
{
	GetSkillTask().SendReservationSkillList();
	ResetSkillReservation();
}
#endif	//	#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

#if defined( PRE_ADD_PRESET_SKILLTREE )
void CDnSkillTreeDlg::SetSkillTreePreSetDlg( CDnSkillTreePreSetDlg * pSkillTreePreSetDlg )
{
	m_pSkillTreePreSetDlg = pSkillTreePreSetDlg;

	if( NULL != m_pSkillTreePreSetDlg )
		m_pSkillTreePreSetDlg->SetSkillTreeDlg( this );
}

void CDnSkillTreeDlg::SetSkillTreeSetMessageDlg( CDnSkillTreeSetMessageDlg * pSkillTreeSetMessageDlg )
{
	m_pSkillTreeSetMessageDlg = pSkillTreeSetMessageDlg;

	if( NULL != m_pSkillTreeSetMessageDlg )
		m_pSkillTreeSetMessageDlg->SetSkillTreeDlg( this );
}

void CDnSkillTreeDlg::RefreshSkillTreePresetComboBox()
{
	m_pPresetComboBox->RemoveAllItems();
	m_pPresetComboBox->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1125), NULL, CDnSkillTask::E_DEFAULT_PRESET_ID );	// UISTRING : Basic Skill Tree

	if( !CDnActor::s_hLocalActor )
		return;
	CDnPlayerActor * pPlayer = dynamic_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	if( NULL == pPlayer )
		return;

	const int nLocalPlayerLevel = pPlayer->GetLevel();

	std::vector< CDnSkillTask::SPresetSkillTree > & vecPresetSkillTree = GetSkillTask().GetPresetSkillTree();
	for( DWORD itr = 0; itr < vecPresetSkillTree.size(); ++itr )
	{
		const CDnSkillTask::SPresetSkillTree & sData = vecPresetSkillTree[itr];

		if( nLocalPlayerLevel >= sData.m_nLevelLimit && false == sData.m_Data.empty() )
			m_pPresetComboBox->AddItem( sData.m_wszTreeName.c_str(), NULL, sData.m_cIndex );
	}
}

void CDnSkillTreeDlg::PresetSkillProcess( const BYTE cIndex )
{
	m_bPresetSelect = false;
	m_bPresetDelEnable = false;
	m_bPresetModify = false;
	ResetButtonCommand();

	if( CDnSkillTask::E_DEFAULT_PRESET_ID == cIndex )
		return;

	if( 0 != GetSkillTask().GetWholeUsedSkillPoint() )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 748) );	// UISTRING : Cannot load because skill points have already been spent.
		m_pPresetComboBox->SetSelectedByIndex( 0 );
		return;
	}

	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	if( NULL == pPlayerActor )	return;

	vector<int> vlJobHistory;
	pPlayerActor->GetJobHistory( vlJobHistory );

	std::vector< CDnSkillTask::SPresetSkillTree > & vecPresetSkillTree = GetSkillTask().GetPresetSkillTree();
	for( DWORD itr = 0; itr < vecPresetSkillTree.size(); ++itr )
	{
		CDnSkillTask::SPresetSkillTree & sData = vecPresetSkillTree[itr];

		if( sData.m_cIndex == cIndex )
		{
			m_bPresetDelEnable = sData.m_bAdvice == true ? false : true;

			for( std::vector< std::pair<int, BYTE> >::iterator Itor = sData.m_Data.begin(); Itor != sData.m_Data.end(); ++Itor )
			{
				if( false == GetSkillTask().IsMySkillTreeSkill( vlJobHistory, Itor->first ) )
					continue;

				GetSkillTask().PresetSkillLevelUp( Itor->first, Itor->second );
			}
			break;
		}
	}

	m_bPresetSelect = true;
	m_pSkillTreeContentDlg->UpdateSkillTree();
	ShowGuildSkillPointInfo();
	ButtonStateSkillReservation();
}

void CDnSkillTreeDlg::ButtonStateSkillTreePreset()
{
	if( NULL == m_pSkillTreePreSetDlg )
		return;

	if( true == m_bPresetSelect )
	{
		if( true == m_bPresetModify )
			m_pSkillTreePreSetDlg->RefreshButtonShow( true, true, false, false );
		else if( true == m_bPresetDelEnable )
			m_pSkillTreePreSetDlg->RefreshButtonShow( false, false, true, true );
		else
			m_pSkillTreePreSetDlg->RefreshButtonShow( false, false, true, false );
	}
	else
		m_pSkillTreePreSetDlg->RefreshButtonShow( true, true, false, false );
}

void CDnSkillTreeDlg::ShowPresetSkillTreeSaveMessage()
{
	if( false == GetSkillTask().IsPresetSkillTreeSave() || NULL == m_pSkillTreeSetMessageDlg )
		return;

	m_pSkillTreeSetMessageDlg->SetModeShow( CDnSkillTreeSetMessageDlg::E_ADD_SKILLTREE_PRESET );
}

void CDnSkillTreeDlg::ShowPresetSkillTreeDelMessage()
{
	if( NULL != m_pSkillTreeSetMessageDlg )
		m_pSkillTreeSetMessageDlg->SetModeShow( CDnSkillTreeSetMessageDlg::E_DEL_SKILLTREE_PRESET );
}

void CDnSkillTreeDlg::PresetSkillTreeSave( std::wstring wszName )
{
	GetSkillTask().SendPresetSkillTreeSave( wszName );
}

void CDnSkillTreeDlg::PresetSkillTreeDel()
{
	SComboBoxItem* pItem = m_pPresetComboBox->GetSelectedItem();
	if( NULL == pItem )
		return;

	GetSkillTask().SendPresetSkillTreeDel( pItem->nValue );
}

#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
void CDnSkillTreeDlg::UpdateTotalLevel()
{
	if (m_pTotalLevelInfo)
	{
		int nTotalLevel = GetSkillTask().GetTotalLevel();

		WCHAR wszTotalLevelInfo[128] = {0, };
		_snwprintf_s(wszTotalLevelInfo, _countof(wszTotalLevelInfo), _TRUNCATE, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1832 ), nTotalLevel);
		if (m_pTotalLevelInfo)
			m_pTotalLevelInfo->SetText(wszTotalLevelInfo);
	}
}

void CDnSkillTreeDlg::UpdateTotalLevelSkillTreeContent()
{
	if( m_iSelectedJobBtnID == TOTAL_LEVEL_SKILL && m_pSkillTreeContentDlg )
	{
		m_pSkillTreeContentDlg->UpdateTotalLevelSkillTree( true );

		UpdateTotalLevel();
	}
}

void CDnSkillTreeDlg::AddTotalLevelSkill(int nSlotIndex, DnSkillHandle hSkill)
{
	if (m_pTotalLevelSkillActivateDlg)
		m_pTotalLevelSkillActivateDlg->SetSlot(nSlotIndex, hSkill);
}

void CDnSkillTreeDlg::RemoveTotalLevelSkill(int nSlotIndex)
{
	if (m_pTotalLevelSkillActivateDlg)
		m_pTotalLevelSkillActivateDlg->ResetSlot(nSlotIndex);
}

void CDnSkillTreeDlg::ActivateTotalLevelSkillSlot(int nSlotIndex, bool bActivate)
{
	if (m_pTotalLevelSkillActivateDlg)
		m_pTotalLevelSkillActivateDlg->ActivateSkillSlot(nSlotIndex, bActivate);
}

void CDnSkillTreeDlg::ActivateTotalLevelSkillCashSlot(int nSlotIndex, bool bActivate, __time64_t tExpireDate)
{
	if (m_pTotalLevelSkillActivateDlg)
		m_pTotalLevelSkillActivateDlg->ActivateSkillCashSlot(nSlotIndex, bActivate, tExpireDate);
}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL