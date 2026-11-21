#include "StdAfx.h"
#include "DnCharCreatePlayerInfo.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnTableDB.h"
#include "DnJobIconStatic.h"
#include "DnInterface.h"
#include "DnJobInformationDlg.h"

static int s_nLastPreviewClassID = 0;



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#ifdef PRE_MOD_SELECT_CHAR


CDnCharCreatePlayerInfo::CDnCharCreatePlayerInfo( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pMovieCtrl( NULL )
{
}

CDnCharCreatePlayerInfo::~CDnCharCreatePlayerInfo(void)
{

}

void CDnCharCreatePlayerInfo::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize(CEtResourceMng::GetInstance().GetFullName("CharCreate_PlayerInfo.ui").c_str(), bShow);
}

void CDnCharCreatePlayerInfo::InitialUpdate()
{
	m_pMovieCtrl = GetControl<CDnMovieControl>( "ID_MOVIE_SKILL" );

	bool bExistClassInfo = false;
	CEtUIButton *pBtnClassInfo = GetControl<CEtUIButton>( "ID_BT_CLASSINFO", &bExistClassInfo );
	if( bExistClassInfo && pBtnClassInfo )
		pBtnClassInfo->Show( true );
}

void CDnCharCreatePlayerInfo::Show( bool bShow )
{
	if( bShow == m_bShow )
		return;

	if( m_pMovieCtrl )
	{
		m_pMovieCtrl->Show( bShow );

		if( bShow == false )
			m_pMovieCtrl->Stop();
	}

	CDnCustomDlg::Show(bShow);
}

void CDnCharCreatePlayerInfo::PlayMovie( const std::string& fileName )
{
	if( m_pMovieCtrl == NULL )
		return;

	if( fileName.empty() == false )
		m_pMovieCtrl->Play( fileName.c_str(), true );
	else
		m_pMovieCtrl->Stop();
}

void CDnCharCreatePlayerInfo::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );
	MoveToTail( m_pMovieCtrl );
}

void CDnCharCreatePlayerInfo::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	CDnLoginTask *pLoginTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( !pLoginTask ) return;

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( IsCmdControl( "ID_BT_OK" ) ) 
		{
			pLoginTask->StartCharacterChangeParts();
		}
		else if( IsCmdControl( "ID_BT_CLASSINFO" ) )
		{
			CDnJobInformationMainDlg *pJobInfoDlg = GetInterface().GetJobInformationMainDlg();
			if( pJobInfoDlg )
			{
				pJobInfoDlg->SetJobInfo( s_nLastPreviewClassID + 1 );
				pJobInfoDlg->Show( true );
			}
		}
//		else if( IsCmdControl( "ID_BT_CANCEL" ) ) 
//		{
//			pLoginTask->DeSelectCharacter();
//		}
	}
	
}

void CDnCharCreatePlayerInfo::SetJobID( int nJobClassID, int nJobExplanationID )
{
	s_nLastPreviewClassID = nJobClassID;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TJOB );	
	if( pSox == NULL )
		return;


	if( pSox->IsExistItem( nJobClassID + 1 ) ) 
	{
		std::wstring m_wszJobName;
			m_wszJobName = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr(nJobClassID + 1, "_JobName")->GetInteger());
		CEtUIStatic* pStaticClassName = GetControl<CEtUIStatic>( "ID_TEXT_CLASSNAME" );
		pStaticClassName->SetText( m_wszJobName.c_str() );
		pStaticClassName->Show( true );
	}

	CDnJobIconStatic* pStaticJobIcon = GetControl<CDnJobIconStatic>( "ID_STATIC_CLASSICON" );
	pStaticJobIcon->SetIconID(nJobClassID + 1);
	pStaticJobIcon->Show( true );

	CEtUIStatic* pStaticClassExplanation = GetControl<CEtUIStatic>( "ID_TEXT_CLASSSUMMARY" );
	pStaticClassExplanation->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nJobExplanationID ) );
	pStaticClassExplanation->Show( true );
	//

	CEtUIStatic* pStaticClassAbility[CLASSKINDMAX];
	pStaticClassAbility[0] = GetControl<CEtUIStatic>( "ID_STATIC_WARRIOR" );
	pStaticClassAbility[1] = GetControl<CEtUIStatic>( "ID_STATIC_ARCHER" );
	pStaticClassAbility[2] = GetControl<CEtUIStatic>( "ID_STATIC_SOSERESS" );
	pStaticClassAbility[3] = GetControl<CEtUIStatic>( "ID_STATIC_CLERIC" );
	pStaticClassAbility[4] = GetControl<CEtUIStatic>( "ID_STATIC_ACADEMIC" );
	pStaticClassAbility[5] = GetControl<CEtUIStatic>( "ID_STATIC_KALI" );

	for( int i = 0; i < CLASSKINDMAX; ++i )
		pStaticClassAbility[i]->Show( false );

	if( nJobClassID >= 0 && nJobClassID < CLASSKINDMAX )
		pStaticClassAbility[nJobClassID]->Show( true );
}

#endif // PRE_MOD_SELECT_CHAR

