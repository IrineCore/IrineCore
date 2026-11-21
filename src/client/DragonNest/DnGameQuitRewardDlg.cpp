#include "Stdafx.h"

#ifdef PRE_ADD_GAMEQUIT_REWARD

#include "DnTableDB.h"

#include "TaskManager.h"
#include "DnPVPLobbyVillageTask.h"
#include "DnCommonTask.h"
#include "DnItemTask.h"

#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnSystemDlg.h"

#include "DnGameQuitRewardDlg.h"

CDnGameQuitRewardDlg::CDnGameQuitRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
,m_pStatic(NULL)
, m_nRewardType(0)
,m_bRecvReward(false)
{

}

CDnGameQuitRewardDlg::~CDnGameQuitRewardDlg(){
	
	ReleaseDlg();
	m_vecItemSlot.clear();

}

void CDnGameQuitRewardDlg::ReleaseDlg()
{
	int size = (int)m_vecItemSlot.size();
	for( int i=0; i<size; ++i )
		m_vecItemSlot[i]->ResetSlot();	

	for( int i=0; i<2; ++i )
	{
		int size = (int)m_vecItem[i].size();
		for( int k=0; k<size; ++k )
			delete m_vecItem[i][k];
		m_vecItem[i].clear();
	}
	
}

void CDnGameQuitRewardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("NewbieGiftBoxDlg.ui").c_str(), bShow );
}

void CDnGameQuitRewardDlg::InitialUpdate()
{
	m_vecItemSlot.reserve( 7 );

	char str[64] = {0,};
	for( int i=0; i<7; ++i )
	{
		sprintf_s( str, 64, "ID_BT_ITEM%d", i );
		CDnItemSlotButton * pControl = GetControl<CDnItemSlotButton>( str );
		if( pControl )
			m_vecItemSlot.push_back( pControl );
	}

	m_pStatic = GetControl<CEtUIStatic>("ID_TEXT_ASK");

	//GetControl<CEtUITextBox>("ID_TEXTBOX_NOTICE")->AddColorText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7960 ) );
	GetControl<CEtUIStatic>("ID_TEXTBOX_NOTICE")->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7960 ) );
}

void CDnGameQuitRewardDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	//if( bShow )
	//{
	//}

	CEtUIDialog::Show( bShow );
}

void CDnGameQuitRewardDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );


	// Button.
	if( nCommand == EVENT_BUTTON_CLICKED )
	{	
		// Close.
		if( IsCmdControl("ID_BT_CLOSE") )
		{
			Show( false );
		}

		// Exit game.
		if( IsCmdControl("ID_BT_CANCEL" ) )
		{
			Show( false );
			
			// Display reward on next login.
			ShowNextTimeReward();
		}

		// Receive gift.
		if( IsCmdControl("ID_BT_OK") )
		{
			GiftReceiving();
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnGameQuitRewardDlg::SetRewardType( int type )
{
	if( m_pStatic == NULL )
		return;
	m_pStatic->ClearText();

	if( type < GameQuitReward::RewardType::eType::None || type > GameQuitReward::RewardType::eType::ComeBackReward )
		return;

	m_nRewardType = type;
	
	// New user.
	int strIndex = 7954; // An item prepared for new adventurers!! The given gift is in inventory..

	// Returning user.
	if( type == GameQuitReward::RewardType::eType::ComeBackReward )
		strIndex = 7959; // Item prepared for returning heroes!! Granted gift is in inventory..

	m_pStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, strIndex ) );
	
	// Set reward item.
	SetRewardItem();
}


// Set reward items according to reward target.
void CDnGameQuitRewardDlg::SetRewardItem()
{		
	if( m_nRewardType < GameQuitReward::RewardType::eType::NewbieReward || m_nRewardType > GameQuitReward::RewardType::eType::ComeBackReward )
		return;

	int nLevel = 1;
	int nClass = 1;

	// When LocalPlayerActor exists.
	if( CDnActor::s_hLocalActor )
	{
		CDnActor * pActor = static_cast< CDnActor * >(CDnActor::s_hLocalActor);
		nLevel = pActor->GetLevel();
		nClass = pActor->GetClassID();
	}

	// When LocalPlayerActor does not exist.
	else
	{
		bool bPVP = false;
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
		}
	}

	// Target cannot claim item.
	if( m_nRewardType == GameQuitReward::RewardType::eType::None )
	{
		ReleaseDlg();
		return;
	}

	int idx = m_nRewardType - 1;

	// Reference Item only once.
	if( !m_vecItem[ idx ].empty() )
		return;

	int nEventType = 6; // 6 for new users.
	if( m_nRewardType == GameQuitReward::RewardType::eType::ComeBackReward )
		nEventType = 8; // Returning user is 8.
	
	// Find Item from Table.	
	CDnItemTask::GetInstance().FindEventItems( nEventType, nLevel, nClass, m_vecItem[ idx ] );

	// Set ItemSlot.
	CDnItem * pItem = NULL;
	int size = (int)m_vecItem[ idx ].size();
	int slotSize = (int)m_vecItemSlot.size();
	if( slotSize >= size )
	{
		for( int i=0; i<size; ++i )
		{
			pItem = m_vecItem[ idx ][i];
			if( pItem )
				m_vecItemSlot[ i ]->SetItem( pItem, pItem->GetOverlapCount() );
		}
	}

}


// Request to receive gift.
void CDnGameQuitRewardDlg::GiftReceiving()
{
	if( m_nRewardType == GameQuitReward::RewardType::eType::None )
		return;

	CTask * _pTask = CTaskManager::GetInstance().GetTask( "CommonTask" );
	if( _pTask )
		static_cast<CDnCommonTask *>(_pTask)->SendGiftReceiving( m_nRewardType );
}


void CDnGameQuitRewardDlg::RecvReward()
{
	if( m_nRewardType == GameQuitReward::RewardType::eType::NewbieReward )
		m_bRecvReward = true;
}

bool CDnGameQuitRewardDlg::ShowNewbieNextTimeReward()
{
	if( m_bRecvReward )
	{
		m_bRecvReward = false;
		GetInterface().ShowGameQuitNextTimeRewardDlg( true, GameQuitReward::RewardType::eType::NewbieReward );
		return true;
	}

	return false;
}


// Display reward on next login.
void CDnGameQuitRewardDlg::ShowNextTimeReward()
{
	if( m_nRewardType == GameQuitReward::RewardType::eType::None || m_nRewardType == GameQuitReward::RewardType::eType::ComeBackReward )
	{
		CEtUIDialog * pDlg = GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SYSTEM_DIALOG );
		if( pDlg )
		{
			CDnSystemDlg * pSystemDlg = (CDnSystemDlg *)pDlg;
			int strId = pSystemDlg->IsGameQuitCharSelectStrIdx();
			if( strId == 0 )
				pSystemDlg->OnExitButton( true );
			else
				pSystemDlg->OnCharButton();				
		}

		//GetInterface().MessageBox( MakeDurabilityWarningStr(100038).c_str(), MB_YESNO, MESSAGEBOX_CHAR, this );
		//GetInterface().GetMainMenuDialog()->GetSystemDlg()->OnExitButton( true );
	}

	else
	{
		GetInterface().ShowGameQuitNextTimeRewardDlg( true, 
			m_nRewardType == GameQuitReward::RewardType::eType::NewbieReward ? m_nRewardType : 0 );
	}

}

#endif // PRE_ADD_GAMEQUIT_REWARD