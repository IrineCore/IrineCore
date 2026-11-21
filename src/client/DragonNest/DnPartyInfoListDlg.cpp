#include "StdAfx.h"
#include "DnPartyInfoListDlg.h"
#include "TaskManager.h"
#include "DnPartyTask.h"
#include "DnInterfaceDlgID.h"
#include "DnActor.h"
#include "DnChatTabDlg.h"
#include "DnInterface.h"
#include "DnPartyCreateDlg.h"
#include "DnPartyKickDlg.h"
#include "DnPartyInviteDlg.h"
#include "SystemSendPacket.h"
#include "DnSimpleTooltipDlg.h"
#include "DNIsolate.h"
#include "DnInterfaceString.h"
#include "DnPartyNormalMemberDlg.h"
#include "DnPartyRaidMemberDlg.h"
#include "DnPartyRaidReGroupDlg.h"
#include "DnInterface.h"
#include "DnNestInfoDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPartyInfoListDlg::CDnPartyInfoListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
	, m_pStaticTitle(NULL)
	, m_pComboBoxRoulette(NULL)
	, m_pComboBoxRandRoulette(NULL)
	, m_pButtonInfo(NULL)
	, m_pButtonChange(NULL)
	, m_pButtonQuit(NULL)
	, m_pButtonInvite(NULL)
	, m_pButtonExile(NULL)
	, m_pPartyInfoDlg(NULL)
	, m_pPartyKickDlg(NULL)
	, m_pPartyInviteDlg(NULL)
	, m_PreReqChangeRuleCache(ITEMLOOTRULE_MAX)
	, m_pButtonRequestReady(NULL)
	, m_fRequestReadyCoolTimer(0.f)
	, m_pTargetMapStatic(NULL)
	, m_pDifficultyStatic(NULL)
	, m_pJobDiceCheckBox(NULL)
	, m_pJobDiceStatic(NULL)
	, m_SetJobDiceCoolTimer(0.f)
	, m_bIsReqJobDiceState(false)
	, m_pButtonEditIndex(NULL)
	, m_pReGroupDlg(NULL)
#ifdef PRE_PARTY_DB
	, m_pAllowWorldZoneMapListCheckBox(NULL)
	, m_pAllowWorldZoneMapListStatic(NULL)
#endif
{
#ifdef PRE_WORLDCOMBINE_PARTY
#else
	m_pPartyMemberDlgList.reserve(_PARTY_TYPE_MAX);
#endif
}

CDnPartyInfoListDlg::~CDnPartyInfoListDlg(void)
{
	SAFE_DELETE( m_pPartyInfoDlg );
	SAFE_DELETE( m_pPartyKickDlg );
	SAFE_DELETE( m_pPartyInviteDlg );

	std::vector<CDnPartyMemberDlg*>::iterator iter = m_pPartyMemberDlgList.begin();
	for (; iter != m_pPartyMemberDlgList.end(); ++iter)
	{
		CDnPartyMemberDlg* pDlg = *iter;
		SAFE_DELETE(pDlg);
	}

	m_pPartyMemberDlgList.clear();

	SAFE_DELETE(m_pReGroupDlg);
}

void CDnPartyInfoListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PartyInfoBaseDlg.ui" ).c_str(), bShow );
}

void CDnPartyInfoListDlg::InitialUpdate()
{
	CDnPartyMemberDlg* pNormalDlg = NULL;
	pNormalDlg = new CDnPartyNormalMemberDlg(UI_TYPE_CHILD, this, PARTY_NORMAL_MEMBER_DIALOG, this);
	pNormalDlg->Initialize(false);
	m_pPartyMemberDlgList.push_back(pNormalDlg);

	CDnPartyMemberDlg* pRaidDlg = NULL;
	pRaidDlg = new CDnPartyRaidMemberDlg(UI_TYPE_CHILD, this, PARTY_RAID_MEMBER_DIALOG, this);
	pRaidDlg->Initialize(false);
	m_pPartyMemberDlgList.push_back(pRaidDlg);

	m_pStaticTitle = GetControl<CEtUIStatic>("ID_TITLE");

	m_pComboBoxRoulette = GetControl<CEtUIComboBox>("ID_COMBOBOX_RULE");
	m_pComboBoxRoulette->RemoveAllItems();
	m_pComboBoxRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3538), NULL, ITEMLOOTRULE_INORDER );	//	Take in order
	m_pComboBoxRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3521), NULL, ITEMLOOTRULE_OWNER );	//	Assign to the acquiring character
	//m_pComboBoxRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3520), NULL, ITEMLOOTRULE_LEADER ); // Leader takes all - excluded due to #11257.
	m_pComboBoxRoulette->SetSelectedByIndex(0);
	m_pComboBoxRoulette->Enable( false );

	m_pComboBoxRandRoulette = GetControl<CEtUIComboBox>("ID_COMBOBOX_RULE1");
	m_pComboBoxRandRoulette->RemoveAllItems();
	m_pComboBoxRandRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3570), NULL, ITEMRANK_D );	//	Normal grade or higher
	m_pComboBoxRandRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3571), NULL, ITEMRANK_C );	//	Magic grade or higher
	m_pComboBoxRandRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3572), NULL, ITEMRANK_B );	//	Rare grade or higher
	m_pComboBoxRandRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3573), NULL, ITEMRANK_A );	//	Epic grade or higher
	m_pComboBoxRandRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3574), NULL, ITEMRANK_S );	//	Unique grade or higher
#ifdef PRE_PARTY_DB
	m_pComboBoxRandRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3457), NULL, ITEMRANK_SS );	//	Legend grade or higher
	m_pComboBoxRandRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3458), NULL, ITEMRANK_NONE );	//	UISTRING : Do not roll dice
#else
	m_pComboBoxRandRoulette->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3575), NULL, ITEMRANK_NONE );	//	Default distribution for all grades
#endif
	m_pComboBoxRandRoulette->SetSelectedByIndex(1);
	m_pComboBoxRandRoulette->Enable( false );

	m_pButtonQuit = GetControl<CEtUIButton>("ID_BUTTON_QUIT");
	m_pButtonInfo = GetControl<CEtUIButton>("ID_BUTTON_INFO");
	m_pButtonChange = GetControl<CEtUIButton>("ID_BUTTON_CHANGE");
	m_pButtonInvite = GetControl<CEtUIButton>("ID_BUTTON_INVITE");
	m_pButtonExile = GetControl<CEtUIButton>("ID_BUTTON_EXILE");

	m_pButtonRequestReady = GetControl<CEtUIButton>("ID_BUTTON_READY");

	m_pTargetMapStatic	= GetControl<CEtUIStatic>("ID_STATIC_TARGETMAP");
	m_pDifficultyStatic = GetControl<CEtUIStatic>("ID_STATIC_DIFFICULTY");
	m_pJobDiceCheckBox	= GetControl<CEtUICheckBox>("ID_CHECKBOX_RULE");
	m_pJobDiceStatic = GetControl<CEtUIStatic>("ID_STATIC6");
#ifdef PRE_PARTY_DB
	m_pAllowWorldZoneMapListCheckBox = GetControl<CEtUICheckBox>("ID_CHECKBOX_WORLDPARTYLIST");
	m_pAllowWorldZoneMapListStatic = GetControl<CEtUIStatic>("ID_STATIC7");
#endif

	m_pButtonEditIndex = GetControl<CEtUIButton>("ID_BUTTON_OPTION");

	m_pReGroupDlg = new CDnPartyRaidReGroupDlg(UI_TYPE_CHILD_MODAL, this, PARTY_RAID_REGROUP_DIALOG, this);
	m_pReGroupDlg->Initialize(false);

	m_pPartyInfoDlg = new CDnPartyCreateDlg( UI_TYPE_CHILD_MODAL, this, PARTYINFO_DIALOG, this );
	m_pPartyInfoDlg->Initialize( false );

	m_pPartyKickDlg = new CDnPartyKickDlg( UI_TYPE_CHILD_MODAL, this, PARTYKICK_DIALOG, this );
	m_pPartyKickDlg->Initialize( false );

	m_pPartyInviteDlg = new CDnPartyInviteDlg(UI_TYPE_CHILD_MODAL, this, PARTYINVITE_DIALOG, this);
	m_pPartyInviteDlg->Initialize(false);
}

void CDnPartyInfoListDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow == false)
	{
		ShowChildDialog( m_pPartyInfoDlg, false );
		ShowChildDialog( m_pPartyKickDlg, false );
		ShowChildDialog( m_pPartyInviteDlg, false );

		//Also close the Nest info Dialog.
		CDnNestInfoDlg* pNestInfoDlg = GetInterface().GetNestInfoDlg();
		if (pNestInfoDlg)
			pNestInfoDlg->Show(false);
	}

	ShowOffPartyMemberDlg();

	CDnPartyMemberDlg* pDlg = GetCurrentPartyMemberDlg();
	if (pDlg == NULL)
	{
		_ASSERT(0);
		return;
	}

	pDlg->Show(bShow);

	CEtUIDialog::Show( bShow );
}

void CDnPartyInfoListDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if (m_fRequestReadyCoolTimer > 0.f)
		m_fRequestReadyCoolTimer -= fElapsedTime;
	else
		m_fRequestReadyCoolTimer = 0.f;

	if (m_SetJobDiceCoolTimer > 0.f)
		m_SetJobDiceCoolTimer -= fElapsedTime;
	else
	{
		if (m_bIsReqJobDiceState)
			m_bIsReqJobDiceState = false;
		m_SetJobDiceCoolTimer = 0.f;
	}

	if( IsShow() && CDnPartyTask::IsActive() && CDnWorld::IsActive() )
	{
		bool bLeader = (GetPartyTask().GetPartyRole() == CDnPartyTask::LEADER);
		if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage )
		{
			m_pButtonExile->Enable(bLeader);
			//m_pButtonExile->Enable( false );
			m_pButtonInvite->Enable( GetPartyTask().IsPartyFull() == false );
			m_pButtonInfo->Enable( true );
			m_pButtonQuit->Enable( true );
			m_pButtonChange->Enable( true );
			m_pComboBoxRoulette->Enable(bLeader);
			if (m_pComboBoxRandRoulette)
			{
				int curValue = -1;
				if (m_pComboBoxRandRoulette->GetSelectedValue(curValue))
				{
					const eItemRank& curRank = (eItemRank)curValue;
					m_pComboBoxRoulette->Enable(bLeader && (curRank > ITEMRANK_D));
				}
			}
			m_pComboBoxRandRoulette->Enable( bLeader );
			m_pJobDiceCheckBox->Enable(m_bIsReqJobDiceState == false && bLeader);
#ifdef PRE_PARTY_DB
			m_pAllowWorldZoneMapListCheckBox->Enable(bLeader);
#endif

			m_pButtonRequestReady->Enable(bLeader && GetPartyTask().GetPartyCount() != 1 && m_fRequestReadyCoolTimer == 0.f);
		}
		else if (CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeWorldMap)
		{
			//m_pButtonExile->Enable( false );
			m_pButtonInvite->Enable(bLeader && GetPartyTask().IsPartyFull() == false && CDnPartyTask::GetInstance().IsPartyMemberInviting() == false);
			m_pButtonInfo->Enable( false );
			m_pButtonQuit->Enable( true );
			m_pButtonChange->Enable( true );
			m_pComboBoxRoulette->Enable( false );
			m_pComboBoxRandRoulette->Enable(false);
#ifdef PRE_PARTY_DB
			m_pAllowWorldZoneMapListCheckBox->Enable(bLeader);
#endif
			m_pButtonRequestReady->Enable(false);

			m_pJobDiceCheckBox->Enable(false);
		}
		else
		{
			m_pButtonExile->Enable( false );
			m_pButtonInvite->Enable( false );
			m_pButtonInfo->Enable( false );
			m_pButtonQuit->Enable( true );
			m_pButtonChange->Enable( true );
			m_pComboBoxRoulette->Enable( false );
			m_pComboBoxRandRoulette->Enable(false);
			m_pJobDiceCheckBox->Enable(false);
#ifdef PRE_PARTY_DB
			m_pAllowWorldZoneMapListCheckBox->Enable(false);
#endif
		}

		m_pJobDiceStatic->Enable(m_pJobDiceCheckBox->IsChecked());
#ifdef PRE_PARTY_DB
		m_pAllowWorldZoneMapListStatic->Enable(m_pAllowWorldZoneMapListCheckBox->IsChecked());
#endif

		if( GetPartyTask().GetPartyRole() != CDnPartyTask::LEADER )
		{
			m_pButtonExile->Enable( false );
			m_pButtonInvite->Enable( false );
			m_pButtonInfo->Enable(true);
			m_pButtonChange->Enable( false );
			m_pComboBoxRoulette->Enable( false );
			m_pComboBoxRandRoulette->Enable( false );
			m_pButtonRequestReady->Enable(false);
		}

		m_pButtonEditIndex->Enable(CDnPartyTask::GetInstance().IsPartyType(_RAID_PARTY_8) && 
			CDnPartyTask::GetInstance().IsLocalActorMaster() && 
			(CDnPartyTask::GetInstance().GetPartyCount() > 1));

		CDnPartyMemberDlg* pDlg = GetCurrentPartyMemberDlg();
		if (pDlg == NULL)
		{
			_ASSERT(0);
			return;
		}

		int nCurSelect = pDlg->GetCurSelectIndex();
		int nUserSessionId = pDlg->GetCurSelectUserSessionID();
		if (nCurSelect < 0 || GetPartyTask().GetMasterSessionID() == nUserSessionId)
		{
			m_pButtonChange->Enable( false );
			m_pButtonExile->Enable( false );
		}
		else
		{
			bool bLeader = (GetPartyTask().GetPartyRole() == CDnPartyTask::LEADER);
			if (bLeader && CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeVillage)
			{
				CDnPartyTask::PartyStruct* pMemberInfo = CDnPartyTask::GetInstance().GetPartyDataFromSessionID(nUserSessionId);
				if (pMemberInfo != NULL)
					m_pButtonExile->Enable(pMemberInfo->bAbsence);
			}
		}

#ifdef PRE_WORLDCOMBINE_PARTY
		if (GetPartyTask().IsPartyType(_WORLDCOMBINE_PARTY))
		{
			m_pButtonExile->Enable( false );
			m_pButtonInvite->Enable( false );
			m_pButtonInfo->Enable(false);
			m_pButtonChange->Enable( false );
			m_pButtonQuit->Enable( true );
			m_pComboBoxRoulette->Enable( false );
			m_pComboBoxRandRoulette->Enable( false );
			m_pButtonRequestReady->Enable(false);
			m_pAllowWorldZoneMapListCheckBox->Enable(false);
		}
#endif
	}
}

ePartyItemLootRule CDnPartyInfoListDlg::ConvertLootRule(const WCHAR* ruleName) const
{
	if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3519), ruleName))			return ITEMLOOTRULE_RANDOM;
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3520), ruleName))	return ITEMLOOTRULE_LEADER;
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3521), ruleName))	return ITEMLOOTRULE_OWNER;
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3538), ruleName))	return ITEMLOOTRULE_INORDER;

	return ITEMLOOTRULE_MAX;
}

const WCHAR* CDnPartyInfoListDlg::ConvertLootRule(ePartyItemLootRule rule) const
{
	if (ITEMLOOTRULE_RANDOM == rule)			return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3519);
	else if (ITEMLOOTRULE_LEADER == rule)		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3520);
	else if (ITEMLOOTRULE_OWNER == rule)			return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3521);
	else if (ITEMLOOTRULE_INORDER == rule)		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3538);

	return NULL;
}

eItemRank CDnPartyInfoListDlg::ConvertLootRank(const WCHAR* rankName) const
{
	if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3570), rankName))			return ITEMRANK_D;	// Normal grade or higher
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3571), rankName))	return ITEMRANK_C;	// Magic grade or higher
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3572), rankName))	return ITEMRANK_B;	// Rare grade or higher
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3573), rankName))	return ITEMRANK_A;	// Epic grade or higher
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3574), rankName))	return ITEMRANK_S;	// Unique grade or higher
#ifdef PRE_PARTY_DB
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3457), rankName))	return ITEMRANK_SS;	// Legend grade or higher
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3458), rankName))	return ITEMRANK_NONE;	// UISTRING : Do not roll dice
#else
	else if (!_tcscmp(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3575), rankName))	return ITEMRANK_NONE;	// Default distribution for all grades
#endif

	return ITEMRANK_C;
}

void CDnPartyInfoListDlg::OnPartyJoin()
{
	m_PreReqChangeRuleCache = ITEMLOOTRULE_RANDOM;
}

#define JOB_DICE_COOLTIME 1

void CDnPartyInfoListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_QUIT" ) )
		{
			if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeWorldMap )
			{
#ifdef PRE_FIX_GAMEQUIT_TO_VILLAGE
				::SendAbandonStage(true, false);
#else
				::SendAbandonStage(true);
#endif
			}
			else
			{
				GetPartyTask().ReqOutParty();
			}

			return;
		}

		if( IsCmdControl("ID_BUTTON_INFO" ) )
		{
			// Set and display.
			m_pPartyInfoDlg->SetMode(CDnPartyCreateDlg::PARTYREQ_MODIFY);
			ShowChildDialog( m_pPartyInfoDlg, true );
			return;
		}

		if( IsCmdControl("ID_BUTTON_CHANGE" ) )
		{
			CDnPartyMemberDlg* pDlg = GetCurrentPartyMemberDlg();
			if (pDlg == NULL)
			{
				_ASSERT(0);
				return;
			}

			GetPartyTask().ReqPartyLeaderSwap(pDlg->GetCurSelectIndex());
			return;
		}

		if( IsCmdControl("ID_BUTTON_INVITE" ) )
		{
			//GetPartyTask().ReqInviteParty();
			ShowChildDialog(m_pPartyInviteDlg, true);
			return;
		}

		if( IsCmdControl("ID_BUTTON_EXILE" ) )
		{
			// Note: Kick party member
			//
			ShowChildDialog( m_pPartyKickDlg, true );
			return;
		}

		if (IsCmdControl("ID_BUTTON_READY"))
		{
			m_fRequestReadyCoolTimer = 10.f;
			GetPartyTask().ReqPartyRequestReady();
			return;
		}

		if (IsCmdControl("ID_BUTTON_OPTION"))
		{
			m_pReGroupDlg->Show(true);
			return;
		}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED && bTriggeredByUser ) 
	{
		if( IsCmdControl("ID_COMBOBOX_RULE" ) )
		{
			SComboBoxItem* item = m_pComboBoxRoulette->GetSelectedItem();
			if (item)
			{
				//ePartyItemLootRule rule = ConvertLootRule(item->strText);
				ePartyItemLootRule rule = (ePartyItemLootRule)item->nValue;
				if (IsInvalidItemLootRule(rule) == false)
				{
					GetPartyTask().ReqPartyItemLootRuleModify(rule);
					m_PreReqChangeRuleCache = GetPartyTask().GetPartyLootRule();
				}
				else
					_ASSERT(0);
			}
			return;
		}

		if( IsCmdControl("ID_COMBOBOX_RULE1") )
		{
			SComboBoxItem* item = m_pComboBoxRandRoulette->GetSelectedItem();
			if (item)
			{
				// todo by kalliste : Using text comparison due to lack of understanding of combobox value. Change to check by value later.
				eItemRank rank = ConvertLootRank(item->strText);
				GetPartyTask().ReqPartyItemLootRankModify(rank);
				//m_PreReqChangeRankCache = GetPartyTask().GetPartyLootItemRank();
			}
			return;
		}
	}
	else if (nCommand == EVENT_CHECKBOX_CHANGED && bTriggeredByUser)
	{
		if (IsCmdControl("ID_CHECKBOX_RULE"))
		{
			if (m_SetJobDiceCoolTimer == 0.f)
			{
				GetPartyTask().ReqPartyJobDiceModified(m_pJobDiceCheckBox->IsChecked());
				m_SetJobDiceCoolTimer = JOB_DICE_COOLTIME;
				m_bIsReqJobDiceState = true;
				m_pJobDiceCheckBox->Enable(false);
			}
			else
				m_pJobDiceCheckBox->SetChecked(!m_pJobDiceCheckBox->IsChecked());
		}
#ifdef PRE_PARTY_DB
		else if (IsCmdControl("ID_CHECKBOX_WORLDPARTYLIST"))
		{
			GetPartyTask().ReqPartyAllowWorldZoneMapList(m_pAllowWorldZoneMapListCheckBox->IsChecked());
			return;
		}
#endif
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPartyInfoListDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( nID == PARTYINFO_DIALOG )
		{
			if( IsCmdControl("ID_BUTTON_CREATE") )
			{
			}

			ShowChildDialog( m_pPartyInfoDlg, false );
			return;
		}
		else if( nID == PARTYKICK_DIALOG )
		{
			CDnPartyMemberDlg* pDlg = GetCurrentPartyMemberDlg();
			if (pDlg == NULL)
			{
				_ASSERT(0);
				return;
			}

			if( IsCmdControl("ID_PERMAN_KICK" ) )
				GetPartyTask().ReqPartyMemberKick( pDlg->GetCurSelectIndex(), 1 );
			else if( IsCmdControl("ID_NORMAL_KICK" ) )
				GetPartyTask().ReqPartyMemberKick( pDlg->GetCurSelectIndex(), 0 );

			ShowChildDialog( m_pPartyKickDlg, false );
			pDlg->ResetCurSelectIndex();
			return;
		}
		else if (nID == PARTYINVITE_DIALOG)
		{
			ShowChildDialog(m_pPartyInviteDlg, false);
			return;
		}
	}
}

bool CDnPartyInfoListDlg::IsInvalidItemLootRule(ePartyItemLootRule rule) const
{
	return (rule == ITEMLOOTRULE_MAX || rule == ITEMLOOTRULE_NONE);
}

void CDnPartyInfoListDlg::OnChangeItemLootRule()
{
	ePartyItemLootRule changeRule = GetPartyTask().GetPartyLootRule();
	if (IsInvalidItemLootRule(changeRule) == false)
		m_pComboBoxRoulette->SetSelectedByValue(changeRule);
}

bool CDnPartyInfoListDlg::IsInvalidItemLootRank(eItemRank rank) const
{
#ifdef PRE_PARTY_DB
	return (rank != ITEMRANK_NONE && rank > ITEMRANK_MAX);
#else
	return (rank == ITEMRANK_SS || rank == ITEMRANK_MAX );
#endif
}

void CDnPartyInfoListDlg::OnChangeItemLootRank()
{
	eItemRank changeRank = GetPartyTask().GetPartyLootItemRank();
	if (IsInvalidItemLootRank(changeRank))
	{
		_ASSERT(0);
		return;
	}

	m_pComboBoxRandRoulette->SetSelectedByValue(changeRank);
}

void CDnPartyInfoListDlg::RefreshList()
{
	CDnPartyMemberDlg* pDlg = GetCurrentPartyMemberDlg();
	if (pDlg == NULL)
	{
		_ASSERT(0);
		return;
	}

	CDnPartyTask *pTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
	if( !pTask ) return;

	if( pTask->GetPartyRole() == CDnPartyTask::SINGLE )
		return;

	//Reset NestInfo when party info is received again.
	pTask->InitNestInfoList();
	m_pStaticTitle->SetText( pTask->GetPartyName() );
	pDlg->RefreshList();

	OnChangeItemLootRule();
	OnChangeItemLootRank();

	std::wstring result;
	int mapIdx = pTask->GetPartyTargetStageIdx();
	if (mapIdx > 0)
	{
		pTask->GetPartyTargetStageString(result);
		m_pTargetMapStatic->SetText(result);
	}
	else
	{
		m_pTargetMapStatic->ClearText();
	}

	int difficulty = pTask->GetPartyTargetStageDifficulty();
#ifdef PRE_PARTY_DB
	if (difficulty != Dungeon::Difficulty::Max)
#else
	if (difficulty > 0)
#endif
	{
		result.clear();
		pTask->GetPartyTargetStageDifficultyString(result);
		m_pDifficultyStatic->SetTextColor(DN_INTERFACE::STRING::GetStageDifficultyColor(pTask->GetPartyTargetStageDifficulty()));
		m_pDifficultyStatic->SetText(result);
	}
	else
	{
		m_pDifficultyStatic->ClearText();
	}

	//if (pTask->GetPartyRole() != CDnPartyTask::LEADER)
	m_pJobDiceCheckBox->SetChecked(pTask->IsJobDice());
#ifdef PRE_PARTY_DB
	m_pAllowWorldZoneMapListCheckBox->SetChecked(pTask->IsPartyAllowWorldZoneMapList());
#endif

	//m_nCurSelect = -1;
}

void CDnPartyInfoListDlg::SetPartyInfoLeader( int nLeaderIndex )
{
	CDnPartyMemberDlg* pDlg = GetCurrentPartyMemberDlg();
	if (pDlg == NULL)
	{
		_ASSERT(0);
		return;
	}
	
	pDlg->SetPartyInfoLeader(nLeaderIndex);
}

void CDnPartyInfoListDlg::ShowOffPartyMemberDlg()
{
	std::vector<CDnPartyMemberDlg*>::iterator iter = m_pPartyMemberDlgList.begin();
	for (; iter != m_pPartyMemberDlgList.end(); ++iter)
	{
		CDnPartyMemberDlg* pDlg = *iter;
		if (pDlg)
			pDlg->Show(false);
	}
}


CDnPartyMemberDlg* CDnPartyInfoListDlg::GetPartyMemberDlg(ePartyType type) const
{
#ifdef PRE_WORLDCOMBINE_PARTY
	if (type == _WORLDCOMBINE_PARTY)
	{
		if (CDnPartyTask::IsActive())
		{
			SWorldCombinePartyData data;
			CDnPartyTask::GetInstance().GetCurrentWorldCombinePartyData(data);
			if (data.IsEmpty() == false)
			{
				if (data.maxUser <= NORMPARTYCOUNTMAX)
					return m_pPartyMemberDlgList[_NORMAL_PARTY];
				return m_pPartyMemberDlgList[_RAID_PARTY_8];
			}
		}

		return m_pPartyMemberDlgList[_RAID_PARTY_8];
	}
	else
	{
		if (type < 0 || type >= _PARTY_TYPE_MAX)
			return NULL;

		return m_pPartyMemberDlgList[type];
	}
#else
	if (type < 0 || type >= _PARTY_TYPE_MAX)
		return NULL;
	return m_pPartyMemberDlgList[type];
#endif
}

CDnPartyMemberDlg* CDnPartyInfoListDlg::GetCurrentPartyMemberDlg() const
{
	if( !CDnPartyTask::IsActive() ) return NULL;
	return GetPartyMemberDlg(CDnPartyTask::GetInstance().GetPartyType());
}

void CDnPartyInfoListDlg::EnableLeaderChangeBtn(bool bEnable)
{
	if (m_pButtonChange)
		m_pButtonChange->Enable(bEnable);
}

void CDnPartyInfoListDlg::EnableExileBtn(bool bEnable)
{
	if (m_pButtonExile)
		m_pButtonExile->Enable(bEnable);
}

#ifdef PRE_ADD_NEWCOMEBACK
void CDnPartyInfoListDlg::SetComebackAppellation( UINT sessionID, bool bComeback )
{
	if( CDnPartyTask::IsActive())
	{
		int nPartyType = CDnPartyTask::GetInstance().GetPartyType();
		if( nPartyType == _WORLDCOMBINE_PARTY)
		{
			SWorldCombinePartyData data;
			CDnPartyTask::GetInstance().GetCurrentWorldCombinePartyData(data);
			if (data.IsEmpty() == false)
			{
				if (data.maxUser <= NORMPARTYCOUNTMAX)
				{
					nPartyType = _NORMAL_PARTY;
				}
				else
				{
					nPartyType = _RAID_PARTY_8;
				}
			}
		}

		if( nPartyType >= 0 && nPartyType < (int)m_pPartyMemberDlgList.size() )
		{
			m_pPartyMemberDlgList[nPartyType]->SetComebackAppellation( sessionID, bComeback );
		}
	}
}
#endif // PRE_ADD_NEWCOMEBACK