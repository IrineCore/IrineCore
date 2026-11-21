#include "StdAfx.h"
#include "DnMissionDlg.h"
#include "DnMissionAchieveDlg.h"
#include "DnMissionOnOffDlg.h"
#include "DnMissionDailyQuestDlg.h"
#include "DnMissionGuildCommonDlg.h"
#include "DnMissionTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMissionDlg::CDnMissionDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUITabDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pAchieveDlg(NULL)
, m_pDailyQuestDlg(NULL)
, m_pGuildMissionDlg(NULL)
{
	memset( m_pOnOfflDlg, 0, sizeof(m_pOnOfflDlg) );
}

CDnMissionDlg::~CDnMissionDlg()
{
}

void CDnMissionDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MissionMainTabDlg.ui" ).c_str(), bShow );
}

void CDnMissionDlg::InitialUpdate()
{
	CEtUIRadioButton *pTabButton, *pTabInfo;
	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_INFO");
	m_pAchieveDlg = new CDnMissionAchieveDlg( UI_TYPE_CHILD, this, TabEnum::Achieve );
	m_pAchieveDlg->Initialize( false );
	pTabInfo = pTabButton;
	AddTabDialog( pTabButton, m_pAchieveDlg );

#ifdef PRE_ADD_MISSION_NEST_TAB
	const char *szControlName[] = { "ID_TAB_GENERAL", "ID_TAB_DUNGEON", "ID_TAB_BATTLE", "ID_TAB_NEST", "ID_TAB_DUNGEON_S" };
#else
	const char *szControlName[] = { "ID_TAB_GENERAL", "ID_TAB_DUNGEON", "ID_TAB_BATTLE" };
#endif	

#ifdef PRE_ADD_MISSION_NEST_TAB
	int EnumTest = 0;
	for( int i=0; i< (int)_countof(szControlName); i++ ) {
#else
	for( int i=0; i<3; i++ ) {
#endif
		pTabButton = GetControl<CEtUIRadioButton>(szControlName[i]);
		int nTabID = ( i == 4 ) ? TabEnum::Special : ( TabEnum::Normal + i );
		m_pOnOfflDlg[i] = new CDnMissionOnOffDlg( UI_TYPE_CHILD, this, nTabID );
		m_pOnOfflDlg[i]->Initialize( false );
		m_pOnOfflDlg[i]->SetMainCategory( (CDnMissionTask::MainCategoryEnum)i );
		AddTabDialog( pTabButton, m_pOnOfflDlg[i] );
	}

	bool bExistGuildTab = false;
	pTabButton = GetControl<CEtUIRadioButton>("ID_RBT_GUILD", &bExistGuildTab);
	if( bExistGuildTab ) {
		m_pGuildMissionDlg = new CDnMissionGuildCommonDlg( UI_TYPE_CHILD, this, TabEnum::Guild );
		m_pGuildMissionDlg->Initialize( false );
		AddTabDialog( pTabButton, m_pGuildMissionDlg );
	}

	bool bExistScheduleTab = false;
	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_SCHEDULE", &bExistScheduleTab);
	if( !bExistScheduleTab ) {
		pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_DAYLIQUEST");
	}
	m_pDailyQuestDlg = new CDnMissionDailyQuestDlg( UI_TYPE_CHILD, this, TabEnum::DailyQuest );
	m_pDailyQuestDlg->Initialize( false );
	AddTabDialog( pTabButton, m_pDailyQuestDlg );

	SetCheckedTab( pTabInfo->GetTabID() );

	m_SmartMoveEx.SetControl( GetControl<CEtUIRadioButton>("ID_TAB_INFO") );
}


void CDnMissionDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) {
		if( IsCmdControl( "ID_BUTTON_CLOSE" ) ) {
			Show( false );
		}
	}
	if( nCommand == EVENT_RADIOBUTTON_CHANGED ) {
	}

	CEtUITabDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

}

void CDnMissionDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;
	
	if( bShow )
	{
		m_SmartMoveEx.MoveCursor();
	}

	CEtUITabDialog::Show( bShow );

	if( m_pCallback )
	{
		m_pCallback->OnUICallbackProc( GetDialogID(), bShow?EVENT_DIALOG_SHOW:EVENT_DIALOG_HIDE, NULL );
	}
}

void CDnMissionDlg::Process( float fElapsedTime )
{
	m_SmartMoveEx.Process();
	CEtUITabDialog::Process( fElapsedTime );
}

void CDnMissionDlg::RefreshInfo()
{
	if( IsShow() ) {
		Show( false );
		Show( true );
	}
}

#ifdef PRE_MOD_MISSION_HELPER
void CDnMissionDlg::SelectMissionItemFromNotifier( MissionInfoStruct *pStruct )
{
	CDnMissionTask::MissionInfoStruct *pMissionStruct = (CDnMissionTask::MissionInfoStruct *)pStruct;
	if( !pStruct ) return;

	DNNotifier::Type::eType Type = GetMissionTask().GetNotifierType( pMissionStruct );
	if( Type == DNNotifier::Type::NormalMission )
	{
		CDnMissionDlg::TabEnum TabIndex = Normal;
		switch( pMissionStruct->MainCategory )
		{
		case CDnMissionTask::MainCategoryEnum::Normal: TabIndex = CDnMissionDlg::TabEnum::Normal; break;
		case CDnMissionTask::MainCategoryEnum::Dungeon: TabIndex = CDnMissionDlg::TabEnum::Dungeon; break;
		case CDnMissionTask::MainCategoryEnum::Battle: TabIndex = CDnMissionDlg::TabEnum::Battle; break;
#ifdef PRE_ADD_MISSION_NEST_TAB
		case CDnMissionTask::MainCategoryEnum::Nest: TabIndex = CDnMissionDlg::TabEnum::Nest; break;
		case CDnMissionTask::MainCategoryEnum::Special: TabIndex = CDnMissionDlg::TabEnum::Special; break;
#endif 
		}
		SetCheckedTab( TabIndex );
		CDnMissionOnOffDlg *pDialog = (CDnMissionOnOffDlg *)m_groupTabDialog.GetDialog( TabIndex );
		if( pDialog )
		{
			pDialog->SetLastSelectCatogory( pMissionStruct->szSubCategory );
			pDialog->RefreshSubCategory();
			pDialog->SelectMissionItemFromNotifier( pMissionStruct->nNotifierID );
		}
	}
	else if( Type == DNNotifier::Type::DailyMission ||
			Type == DNNotifier::Type::WeeklyMission ||
			Type == DNNotifier::Type::GuildWarMission ||
			Type == DNNotifier::Type::WeekendEventMission ||
			Type == DNNotifier::Type::WeekendRepeatMission 
#ifdef PRE_ADD_MONTHLY_MISSION
			|| Type == DNNotifier::Type::MonthlyMission 
#endif 
			|| Type == DNNotifier::Type::LongTermMission
			)
	{
		SetCheckedTab( CDnMissionDlg::TabEnum::DailyQuest );
		m_pDailyQuestDlg->SetCategory( GetMissionTask().ToDailyMissionType( Type ) );
		m_pDailyQuestDlg->RefreshSubCategory();
		m_pDailyQuestDlg->SelectMissionItemFromNotifier( pMissionStruct->nNotifierID );
	}
	else if( Type == DNNotifier::Type::GuildCommonMission )
	{
		SetCheckedTab( CDnMissionDlg::TabEnum::Guild );
		m_pGuildMissionDlg->SetCategory( GetMissionTask().ToDailyMissionType( Type ) );
		m_pGuildMissionDlg->RefreshSubCategory();
		m_pGuildMissionDlg->SelectMissionItemFromNotifier( pMissionStruct->nNotifierID );
	}
}
#endif