#include "Stdafx.h"

#ifdef PRE_ADD_GAMEQUIT_REWARD

#include "TaskManager.h"
#include "DnCommonTask.h"
#include "DnItemTask.h"
#include "DnPVPLobbyVillageTask.h"

#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnSystemDlg.h"

#include "SyncTimer.h"

#include "DnGameQuitNextTimeRewardDlg.h"

CDnGameQuitNextTimeRewardDlg::CDnGameQuitNextTimeRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
,m_pStatic(NULL)
,m_nRewardType(0)
,m_bLogin(false)
{

}

CDnGameQuitNextTimeRewardDlg::~CDnGameQuitNextTimeRewardDlg(){
	
	ReleaseDlg();
	m_vecItemSlot.clear();
	
}


void CDnGameQuitNextTimeRewardDlg::ReleaseDlg()
{
	int size = (int)m_vecItemSlot.size();
	for( int i=0; i<size; ++i )
		m_vecItemSlot[i]->ResetSlot();
		
	size = (int)m_vecItem.size();
	for( int k=0; k<size; ++k )
		delete m_vecItem[k];
	m_vecItem.clear();
	

}

void CDnGameQuitNextTimeRewardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("NewbieGiftBoxListDlg.ui").c_str(), bShow );
}

void CDnGameQuitNextTimeRewardDlg::InitialUpdate()
{
	m_vecItemSlot.reserve( 21 );

	char str[64] = {0,};
	for( int i=0; i<21; ++i )
	{
		sprintf_s( str, 64, "ID_BT_ITEM%d", i );
		CDnItemSlotButton * pControl = GetControl<CDnItemSlotButton>( str );
		if( pControl )
			m_vecItemSlot.push_back( pControl );
	}

	m_pStatic = GetControl<CEtUIStatic>("ID_TEXT_ASK");	
	
}

void CDnGameQuitNextTimeRewardDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
	}
	else
	{
		m_bLogin = false;
	}

	CEtUIDialog::Show( bShow );
}

void CDnGameQuitNextTimeRewardDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );


	// Button.
	if( nCommand == EVENT_BUTTON_CLICKED )
	{	
		// Close.
		if( IsCmdControl("ID_BT_CLOSE") )
			Show( false );

		// Exit game.
		if( IsCmdControl("ID_BT_CANCEL" ) )
		{
			// Login.
			if( m_bLogin )
			{
				
			}
			
			// Game.
			else
			{
				CEtUIDialog * pDlg = GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SYSTEM_DIALOG );
				if( pDlg )
				{
					CDnSystemDlg * pSystemDlg = (CDnSystemDlg *)pDlg;
					int strId = pSystemDlg->IsGameQuitCharSelectStrIdx();
					if( strId == 0 )
						GetInterface().GetMainMenuDialog()->GetSystemDlg()->OnExitButton( true );
					else
						GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, strId ), MB_YESNO, CDnSystemDlg::MESSAGEBOX_CHAR, pSystemDlg );						
				}
			}

			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnGameQuitNextTimeRewardDlg::SetNextTimeRewardType( int type, bool bCharSelect, int nLevel, int nClass )
{
	m_bLogin = bCharSelect;

	if( m_pStatic == NULL )
		return;	
	m_pStatic->ClearText();

	if( type < GameQuitReward::RewardType::eType::None || type > GameQuitReward::RewardType::eType::ComeBackReward )
		return;

	m_nRewardType = type;

	//// New user.
	//if( type == GameQuitReward::RewardType::eType::NewbieReward )
	//	m_pStatic->AddColorText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7954 ) );	

	//// Returning user.
	//else if( type == GameQuitReward::RewardType::eType::ComeBackReward )
	//	m_pStatic->AddColorText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7959 ) );	

	// Set reward item.
	SetNextTimeRewardItem( bCharSelect, nLevel, nClass );



	// Set Text.
	CDnBridgeTask *pTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
	if( pTask )
	{
		__time64_t tEnd = CSyncTimer::GetInstance().GetCurTime(); // Current time.
		__time64_t tBegin = pTask->GetAccountRegTime();        // Account creation time.

		__time64_t elapsedTime = tEnd - tBegin; // Elapsed time since account creation.

		std::wstring str;	
	
		int newbieRewardTime = (int)CGlobalWeightIntTable::GetInstance().GetValue( CGlobalWeightIntTable::NewbieRewardTime ) * 3600; // Reward time.
		
		// Time requirement met to claim item.
		if( elapsedTime >= newbieRewardTime )
		{
			str.assign( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7972 ) );
		}

		// Insufficient time to claim item.
		else
		{
			// Time when reward can be claimed.
			tBegin += newbieRewardTime; // elapsedTime;

			tm * _tm = _localtime64( &tBegin );

			// "You can receive the following items\\nwhen logging in after %d/%d/%d %d:%d"
			wchar_t buf[128]={0,};
			swprintf_s( buf, 128, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7952 ), 
				        _tm->tm_year+1900, _tm->tm_mon+1, _tm->tm_mday, _tm->tm_hour, _tm->tm_min );
			str.assign( buf );			 
		}
		
		//m_pStatic->AddColorText( str.c_str() );
		m_pStatic->SetText( str.c_str() );
	}

}


// Set reward items according to reward target.
void CDnGameQuitNextTimeRewardDlg::SetNextTimeRewardItem( bool bCharSelect, int nLevel, int nClass )
{	
	if( m_nRewardType < GameQuitReward::RewardType::eType::NewbieReward || m_nRewardType > GameQuitReward::RewardType::eType::ComeBackReward )
		return;

	ReleaseDlg();
	
	// When called from character selection screen.
	if( bCharSelect )
	{
		// Find Item from Table.	
		CDnItemTask::GetInstance().FindEventItems( 7, nLevel, nClass, m_vecItem );	
	}

	// When called in-game - ensure Item is referenced only once.
	else if( CDnActor::s_hLocalActor )
	{
		CDnActor * pActor = static_cast< CDnActor * >(CDnActor::s_hLocalActor);
		int nLevel = pActor->GetLevel();
		int nClass = pActor->GetClassID();

		// Find Item from Table.	
		CDnItemTask::GetInstance().FindEventItems( 7, nLevel, nClass, m_vecItem );
	}

	// When LocalPlayerActor does not exist.
	else
	{		
		CTask * pTask = CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
		if( pTask )
		{
#ifdef PRE_ADD_PVPRANK_INFORM
			CDnPVPLobbyVillageTask* pPVPLobbyVillageTask = static_cast<CDnPVPLobbyVillageTask*>(pTask);
			if (pPVPLobbyVillageTask)
			{
				const SCEnter& userInfo = pPVPLobbyVillageTask->GetUserInfo();
				nLevel = userInfo.cLevel;
				nClass = userInfo.cClassID;
			}
#else
			SCEnter & userInfo = ((CDnPVPLobbyVillageTask *)pTask)->GetUserInfo();
			nLevel = userInfo.cLevel;
			nClass = userInfo.cClassID;
#endif
			// Find Item from Table.	
			CDnItemTask::GetInstance().FindEventItems( 7, nLevel, nClass, m_vecItem );
		}
	}

	// Set ItemSlot.
	CDnItem * pItem = NULL;
	int size = (int)m_vecItem.size();
	int slotSize = (int)m_vecItemSlot.size();
	if( slotSize >= size )
	{
		for( int i=0; i<size; ++i )
		{
			pItem = m_vecItem[i];
			if( pItem )
				m_vecItemSlot[ i ]->SetItem( pItem, pItem->GetOverlapCount() );
		}
	}

}

#endif // PRE_ADD_GAMEQUIT_REWARD