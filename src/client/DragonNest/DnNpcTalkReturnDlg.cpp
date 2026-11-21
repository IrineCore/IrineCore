#include "StdAfx.h"
#include "DnNpcTalkReturnDlg.h"
#include "DnLocalPlayerActor.h"
#include "DnNPCActor.h"
#include "NpcSendPacket.h"
#include "TaskManager.h"
#include "DnCommonTask.h"
#include "DnItemTask.h"
#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
#include "DnNpcDlg.h"
#include "DnNpcQuestRecompenseDlg.h"
#include "DnNpcAcceptPresentDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnNpcTalkReturnDlg::CDnNpcTalkReturnDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
#if defined(PRE_ADD_QUEST_BACK_DIALOG_BUTTON)
, m_bIsClickedBackButton(false)
#endif
{
#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	m_bSkipOpen = false;
#endif // PRE_ADD_MAILBOX_OPEN
}

CDnNpcTalkReturnDlg::~CDnNpcTalkReturnDlg(void)
{
}

void CDnNpcTalkReturnDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "NpcChatReturn.ui" ).c_str(), bShow );
}

void CDnNpcTalkReturnDlg::InitialUpdate()
{
	m_pBackButton = GetControl<CEtUIButton>("ID_BUTTON_RETURN");
}

void CDnNpcTalkReturnDlg::Show( bool bShow )
{
	if( m_bShow == bShow ) return;

#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	if (bShow && m_bSkipOpen == false)
#else
	if( bShow )
#endif // PRE_ADD_MAILBOX_OPEN
	{
		DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
		DnActorHandle hNpc = CDnActor::FindActorFromUniqueID(nUID);
		if( !hNpc ) return;
	}

#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	if (bShow == false)
		m_bSkipOpen = false;
#endif // PRE_ADD_MAILBOX_OPEN

	CEtUIDialog::Show( bShow );
}

void CDnNpcTalkReturnDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) {
		if( IsCmdControl( "ID_BUTTON_RETURN" ) ) {
			if( CDnItemTask::IsActive() && CDnItemTask::GetInstance().IsRequestWait() ) return;
			
#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
			if (m_bSkipOpen == true)
			{
				GetInterface().CloseAllMainMenuDialog();
				Show(false);
				return;
			}
#endif // PRE_ADD_MAILBOX_OPEN

			DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
			DnActorHandle hNpc = CDnActor::FindActorFromUniqueID(nUID);
			DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
			if( !hCamera ) return;

#ifndef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
			if( hCamera->GetCameraType() == CDnCamera::NpcTalkCamera ) return;
#endif

			if( hNpc ) {
				CDnNPCActor *pNPC = dynamic_cast<CDnNPCActor*>(hNpc.GetPointer());
				CDnNPCActor::NpcData& data = pNPC->GetNpcData();

				// When doing custom NPCTALK, add the corresponding string to NpcTalkStringGroup!
				CDnCommonTask* pCommonTask = dynamic_cast<CDnCommonTask*>( CTaskManager::GetInstance().GetTask( "CommonTask" ) );
				if(pCommonTask)
				{
					if( !pCommonTask->IsRequestNpcTalk() )
					{
						SendNpcTalk(pNPC->GetUniqueID(), NpcTalkStringGroup[NPCTALK_START], data.wszTalkFileName);
						pCommonTask->SetNpcTalkRequestWait();
					}
				}

#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
				m_bIsClickedBackButton = true;
				
				if( hCamera->GetCameraType() == CDnCamera::NpcTalkCamera )
				{
					CDnNpcDlg* pNpcDlg = GetInterface().GetNpcDialog();

					if(pNpcDlg)
					{
						pNpcDlg->GetNpcQuestRecompenseDlg()->ResetRecompenseDlg();
						pNpcDlg->GetNpcPresentDlg()->ResetPresentDlg();
					}
				}
#endif

				((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->LockInput(true);
				GetInterface().CloseAllMainMenuDialog();
								
				/*
				// Among windows opened via NPC dialogue, guild-related windows do not belong to MainMenu. Close them additionally.
				GetInterface().ShowGuildMgrBox( CDnInterface::GuildMgrBox_Amount, false );
#ifdef PRE_ADD_GACHA_JAPAN
				GetInterface().CloseGachaDialog();
#endif // PRE_ADD_GACHA_JAPAN
				*/
				
				CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
				if( pTask ) pTask->BeginNpcTalk( hNpc );
			}

			Show( false );
		}
	}

#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
	m_bIsClickedBackButton = false;
#endif
}
