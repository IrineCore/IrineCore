#include "StdAfx.h"
#include "DnJobInformationDlg.h"
#include "DnJobIconStatic.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnInterfaceString.h"
#include "DnActor.h"
#include "DnLocalPlayerActor.h"
#include "DnQuestTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnJobInformationMessageBox
//-----------------------------------------------------------------------------
CDnJobInformationMessageBox::CDnJobInformationMessageBox( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pStaticClass( NULL )
	, m_pStaticAsk( NULL )
{
}

CDnJobInformationMessageBox::~CDnJobInformationMessageBox()
{
}

void CDnJobInformationMessageBox::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ClassInfoMessageDlg.ui" ).c_str(), bShow );
}

void CDnJobInformationMessageBox::InitialUpdate()
{
	m_pStaticClass = GetControl<CDnJobIconStatic>( "ID_STATIC_CLASS" );
	m_pStaticAsk = GetControl<CEtUIStatic>( "ID_TEXT_ASK" );
}

void CDnJobInformationMessageBox::SetJobMessage( int nJobID, const WCHAR *wszMessage )
{
	if( m_pStaticClass && nJobID > 0 )
		m_pStaticClass->SetIconID( nJobID );
	if( m_pStaticAsk && wszMessage )
		m_pStaticAsk->SetText( wszMessage );
}

void CDnJobInformationMessageBox::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) || IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnJobInformationPopupDlg
//-----------------------------------------------------------------------------
CDnJobInformationPopupDlg::CDnJobInformationPopupDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pProgressExp( NULL )
	, m_pStaticLv( NULL )
	, m_pStaticNotice( NULL )
{
}

CDnJobInformationPopupDlg::~CDnJobInformationPopupDlg()
{
}

void CDnJobInformationPopupDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ClassGaugePopUpDlg.ui" ).c_str(), bShow );
}

void CDnJobInformationPopupDlg::InitialUpdate()
{
	m_pProgressExp = GetControl<CEtUIProgressBar>( "ID_PROGRESSBAR_EXP" );
	m_pStaticLv = GetControl<CEtUIStatic>( "ID_TEXT_LV" );
	m_pStaticNotice = GetControl<CEtUIStatic>( "ID_TEXT_NOTICE" );
}

void CDnJobInformationPopupDlg::SetPopupInfo( int nLevel, float fExpPercent, const WCHAR *wszNotice )
{
	if( m_pProgressExp )
		m_pProgressExp->SetProgress( fExpPercent );
	if( m_pStaticLv )
	{
		WCHAR wszBuf[32] = { 0 };
		swprintf_s( wszBuf, _countof( wszBuf ), L"Lv.%d", nLevel );
		m_pStaticLv->SetText( wszBuf );
	}
	if( m_pStaticNotice && wszNotice )
		m_pStaticNotice->SetText( wszNotice );
}

void CDnJobInformationPopupDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED && ( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_BT_CANCEL" ) ) )
	{
		Show( false );
		return;
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnJobInformationMainDlg
//-----------------------------------------------------------------------------
CDnJobInformationMainDlg::CDnJobInformationMainDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pBtnAddClass( NULL )
	, m_pBtnSkill( NULL )
	, m_pStaticClassIcon( NULL )
	, m_pTextureBg( NULL )
	, m_pTextureClass( NULL )
	, m_pStaticClassName( NULL )
	, m_pStaticInfo( NULL )
	, m_pStaticSkillInfo( NULL )
	, m_pMessageBox( NULL )
	, m_pPopupDlg( NULL )
	, m_nSelectedJobID( 0 )
	, m_nRemoteQuestID( 0 )
	, m_nPreviewSkillIdx( 0 )
{
	for( int i = 0; i < MAX_JOB_RADIO; ++i )
	{
		m_pRadioJobs[i] = NULL;
		m_nRadioJobIDs[i] = 0;
	}
}

CDnJobInformationMainDlg::~CDnJobInformationMainDlg()
{
	SAFE_RELEASE_SPTR( m_hClassTexture );
	SAFE_DELETE( m_pMessageBox );
	SAFE_DELETE( m_pPopupDlg );
}

void CDnJobInformationMainDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ClassInfoDlg.ui" ).c_str(), bShow );

	m_pMessageBox = new CDnJobInformationMessageBox( UI_TYPE_MODAL );
	if( m_pMessageBox )
		m_pMessageBox->Initialize( false );

	m_pPopupDlg = new CDnJobInformationPopupDlg( UI_TYPE_FOCUS );
	if( m_pPopupDlg )
		m_pPopupDlg->Initialize( false );
}

void CDnJobInformationMainDlg::InitialUpdate()
{
	m_pBtnAddClass = GetControl<CEtUIButton>( "ID_BT_ADDCLASS" );
	m_pBtnSkill = GetControl<CEtUIButton>( "ID_BT_SKILL" );
	m_pStaticClassIcon = GetControl<CDnJobIconStatic>( "ID_STATIC_CLASS" );
	m_pTextureBg = GetControl<CEtUITextureControl>( "ID_TEXTUREL_BG" );
	m_pTextureClass = GetControl<CEtUITextureControl>( "ID_TEXTUREL_CLASS" );
	m_pStaticClassName = GetControl<CEtUIStatic>( "ID_TEXT_CLASS" );
	m_pStaticInfo = GetControl<CEtUIStatic>( "ID_TEXT_INFO" );
	m_pStaticSkillInfo = GetControl<CEtUIStatic>( "ID_TEXT_SKILLINFO" );

	char szName[32] = { 0 };
	for( int i = 0; i < MAX_JOB_RADIO; ++i )
	{
		sprintf_s( szName, sizeof( szName ), "ID_RBT_%d", i );
		m_pRadioJobs[i] = GetControl<CEtUIRadioButton>( szName );
	}
}

void CDnJobInformationMainDlg::RefreshJobList()
{
	for( int i = 0; i < MAX_JOB_RADIO; ++i )
	{
		m_nRadioJobIDs[i] = 0;
		if( m_pRadioJobs[i] )
			m_pRadioJobs[i]->Show( false );
	}

	DNTableFileFormat *pJobSox = GetDNTable( CDnTableDB::TJOB );
	if( !pJobSox )
		return;

	int nCurrentJobID = 0;
	if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->IsPlayerActor() )
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
		nCurrentJobID = pPlayer->GetJobClassID();
	}

	int nFirstJobID = 0;
	int nFirstRadioIdx = -1;
	for( int i = 0; i < pJobSox->GetItemCount(); ++i )
	{
		int nJobID = pJobSox->GetItemID( i );
		DNTableCell *pParentCell = pJobSox->GetFieldFromLablePtr( nJobID, "_ParentJob" );
		if( nCurrentJobID > 0 && pParentCell && pParentCell->GetInteger() != nCurrentJobID )
			continue;

		DNTableCell *pBtnCell = pJobSox->GetFieldFromLablePtr( nJobID, "_Characterbutton" );
		if( !pBtnCell ) continue;
		const char *szBtnName = pBtnCell->GetString();
		if( !szBtnName || szBtnName[0] == '\0' ) continue;

		for( int r = 0; r < MAX_JOB_RADIO; ++r )
		{
			if( m_pRadioJobs[r] && strcmp( m_pRadioJobs[r]->GetControlName(), szBtnName ) == 0 )
			{
				m_nRadioJobIDs[r] = nJobID;
				m_pRadioJobs[r]->Show( true );
				if( nFirstJobID == 0 )
				{
					nFirstJobID = nJobID;
					nFirstRadioIdx = r;
				}
				break;
			}
		}
	}

	if( nFirstRadioIdx >= 0 && m_pRadioJobs[nFirstRadioIdx] )
		m_pRadioJobs[nFirstRadioIdx]->SetChecked( true, false );
	if( nFirstJobID > 0 )
		SetJobInfo( nFirstJobID );
	else if( nCurrentJobID > 0 )
		SetJobInfo( nCurrentJobID );
}

void CDnJobInformationMainDlg::UpdateSkillPreview( int nSkillIdx )
{
	if( !m_pStaticSkillInfo )
		return;
	if( m_vecPreviewSkillIDs.empty() || nSkillIdx < 0 || nSkillIdx >= (int)m_vecPreviewSkillIDs.size() )
	{
		m_pStaticSkillInfo->SetText( L"" );
		return;
	}

	int nSkillID = m_vecPreviewSkillIDs[nSkillIdx];
	DNTableFileFormat *pSkillSox = GetDNTable( CDnTableDB::TSKILL );
	if( pSkillSox && pSkillSox->IsExistItem( nSkillID ) )
	{
		DNTableCell *pNameCell = pSkillSox->GetFieldFromLablePtr( nSkillID, "_NameID" );
		int nNameID = pNameCell ? pNameCell->GetInteger() : 0;
		const WCHAR *wszSkillName = ( nNameID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nNameID ) : L"";
		m_pStaticSkillInfo->SetText( wszSkillName );
	}
}

void CDnJobInformationMainDlg::Show( bool bShow )
{
	if( bShow )
		RefreshJobList();
	CEtUIDialog::Show( bShow );
}

void CDnJobInformationMainDlg::SetJobInfo( int nJobID )
{
	m_nSelectedJobID = nJobID;
	m_nRemoteQuestID = 0;
	m_vecPreviewSkillIDs.clear();
	m_nPreviewSkillIdx = 0;

	if( m_pStaticClassIcon && nJobID > 0 )
		m_pStaticClassIcon->SetIconID( nJobID );
	if( m_pStaticClassName && nJobID > 0 )
		m_pStaticClassName->SetText( DN_INTERFACE::STRING::GetJobString( nJobID ) );

	DNTableFileFormat *pJobSox = GetDNTable( CDnTableDB::TJOB );
	if( !pJobSox || !pJobSox->IsExistItem( nJobID ) )
		return;

	if( m_pStaticInfo )
	{
		DNTableCell *pDescCell = pJobSox->GetFieldFromLablePtr( nJobID, "_JobDescriptionID" );
		int nDescID = pDescCell ? pDescCell->GetInteger() : 0;
		m_pStaticInfo->SetText( ( nDescID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nDescID ) : L"" );
	}

	if( m_pTextureClass )
	{
		SAFE_RELEASE_SPTR( m_hClassTexture );
		DNTableCell *pImgCell = pJobSox->GetFieldFromLablePtr( nJobID, "_CharacterImage" );
		const char *szImg = pImgCell ? pImgCell->GetString() : NULL;
		if( szImg && szImg[0] != '\0' )
		{
			m_hClassTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( szImg ).c_str(), RT_TEXTURE );
			if( m_hClassTexture )
				m_pTextureClass->SetTexture( m_hClassTexture, 0, 0, m_hClassTexture->Width(), m_hClassTexture->Height() );
		}
	}

	DNTableCell *pSkillCell = pJobSox->GetFieldFromLablePtr( nJobID, "_SkillID" );
	if( pSkillCell )
	{
		const char *szSkillStr = pSkillCell->GetString();
		if( szSkillStr && szSkillStr[0] != '\0' )
		{
			std::vector<std::string> tokens;
			TokenizeA( std::string( szSkillStr ), tokens, ";" );
			for( size_t k = 0; k < tokens.size() && k < 4; ++k )
			{
				int nSkillID = atoi( tokens[k].c_str() );
				if( nSkillID > 0 )
					m_vecPreviewSkillIDs.push_back( nSkillID );
			}
		}
	}
	UpdateSkillPreview( 0 );

	if( m_pBtnAddClass )
	{
		bool bEnableAddClass = false;
#ifdef PRE_ADD_REMOTE_QUEST
		DNTableCell *pRemoteCell = pJobSox->GetFieldFromLablePtr( nJobID, "_QuestRemote" );
		int nRemoteID = pRemoteCell ? pRemoteCell->GetInteger() : 0;
		if( nRemoteID > 0 )
		{
			DNTableFileFormat *pRemoteSox = GetDNTable( CDnTableDB::TREMOTEQUEST );
			if( pRemoteSox && pRemoteSox->IsExistItem( nRemoteID ) )
			{
				DNTableCell *pQuestCell = pRemoteSox->GetFieldFromLablePtr( nRemoteID, "_QuestID" );
				m_nRemoteQuestID = pQuestCell ? pQuestCell->GetInteger() : 0;
				if( m_nRemoteQuestID > 0 && CDnQuestTask::IsActive() )
					bEnableAddClass = GetQuestTask().IsExistRemoteQuestAskList( m_nRemoteQuestID, CDnQuestTree::REMOTEQUEST_ASK );
			}
		}
#endif
		m_pBtnAddClass->Enable( bEnableAddClass );
	}
}

void CDnJobInformationMainDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_RADIOBUTTON_CHANGED && bTriggeredByUser )
	{
		for( int i = 0; i < MAX_JOB_RADIO; ++i )
		{
			if( m_pRadioJobs[i] == pControl && m_nRadioJobIDs[i] > 0 )
			{
				SetJobInfo( m_nRadioJobIDs[i] );
				return;
			}
		}
	}
	else if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_SKILL" ) )
		{
			if( !m_vecPreviewSkillIDs.empty() )
			{
				m_nPreviewSkillIdx = ( m_nPreviewSkillIdx + 1 ) % (int)m_vecPreviewSkillIDs.size();
				UpdateSkillPreview( m_nPreviewSkillIdx );
			}
			return;
		}
		else if( IsCmdControl( "ID_BT_ADDCLASS" ) )
		{
			if( m_pMessageBox && m_nSelectedJobID > 0 )
			{
				m_pMessageBox->SetJobMessage( m_nSelectedJobID, DN_INTERFACE::STRING::GetJobString( m_nSelectedJobID ) );
				m_pMessageBox->Show( true );
			}
			return;
		}
		else if( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_CLOSE_DIALOG" ) )
		{
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
