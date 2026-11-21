#include "StdAfx.h"
#include "DnPartyListDlg.h"
#include "DnInterface.h"
#include "DnInterfaceDlgID.h"
#include "TaskManager.h"
#include "DnVillageTask.h"
#include "DnPartyCreateDlg.h"
#include "DnPartyTask.h"
#include "DnPassWordInputDlg.h"
#include "DnChatTabDlg.h"
#include "PartySendPacket.h"
#include "DnInterfaceString.h"
#include "DnSimpleTooltipDlg.h"
#include "DnPartyEnterDlg.h"
#include "DnPartyEnterPasswordDlg.h"
#include "DnTableDB.h"
#include "DnPartySearchDlg.h"
#ifdef PRE_PARTY_DB
#include "DnLocalDataMgr.h"
#endif
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#define DEFAULT_MAP_SORT_COMBOBOX_VALUE			0
#define DEFAULT_DIFFICULT_SORT_COMBOBOX_VALUE	0

CDnPartyListDlg::CDnPartyListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
#ifdef PRE_PARTY_DB
	: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback)
#else
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
#endif
	, m_pPartyPage(NULL)
	, m_pSelectBar(NULL)
	, m_nCurrentPage(0)
	, m_nCurSelect(-1)
	, m_pButtonPagePrev(NULL)
	, m_pButtonPageNext(NULL)
	, m_pPartyCreateDlg(NULL)
	, m_pButtonCreate(NULL)
	, m_pButtonJoin(NULL)
	, m_pButtonRefresh(NULL)
	, m_fRefreshTime( 0.0f )
	, m_pMapSortComboBox(NULL)
	, m_pDifficultySortComboBox(NULL)
	, m_nCurrentMapIdx(-1)
	, m_bSearchCoolTimer(false)
	, m_fSearchCoolTime(0.f)
	, m_pPartySearchDlg(NULL)
	, m_bResetSortForAdvSearch(false)
	, m_pRaidEnterDlg(NULL)
	, m_pRaidEnterPasswordDlg(NULL)
	, m_pPartyEnterDlg(NULL)
	, m_pPartyEnterPasswordDlg(NULL)
	, m_pPassWordInputDlg(NULL)
	, m_pSearchBtn(NULL)
#ifdef PRE_PARTY_DB
	, m_pSearchHistoryComboBox(NULL)
	, m_pSearchEditBox(NULL)
	, m_pOnlySameVillageCheckBox(NULL)
	, m_fOnlySameVillageCoolTimeChecker(0.f)
	, m_bOnlySameVillageCoolTimer(false)
#endif
{
#ifdef PRE_PARTY_DB
	int i = 0;
	for (; i < eSortMax; ++i)
		m_pSortBtns[i] = NULL;

	m_CurrentOrderType = Party::OrderType::Default;
	m_CurrentStageOrderType = Party::StageOrderType::Default;
#endif
}

CDnPartyListDlg::~CDnPartyListDlg(void)
{
	SAFE_DELETE(m_pPartyEnterDlg);
	SAFE_DELETE(m_pPartyEnterPasswordDlg);
	SAFE_DELETE(m_pPartySearchDlg);
	SAFE_DELETE(m_pRaidEnterDlg);
	SAFE_DELETE(m_pRaidEnterPasswordDlg)
}

void CDnPartyListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PartyListDlg.ui" ).c_str(), bShow );
}

void CDnPartyListDlg::InitialUpdate()
{
#ifdef PRE_PARTY_DB
#else
	SPartyInfoList partyInfoList;
#endif

	char szNumber[32]={0};
	char szSecret[32]={0};
	char szTitle[32]={0};
	char szCount[32]={0};
	char szWorldLevel[32]={0};
	char szUserLevel[32]={0};
	char szBase[32]={0};
	char szStage[32] = {0};
	char szDifficulty[32] = {0};
	char szVoice[32]={0};
#ifdef PRE_PARTY_DB
	std::string villageIconCtrlName;
	std::string bonusExpPartyCtrlName;
#endif
#ifdef PRE_WORLDCOMBINE_PARTY
	std::string worldCombineBonusIcons;
#endif

	std::string strComebackUser;

	for( int i=0; i<PARTYLISTOFFSET; i++ )
	{
#ifdef PRE_PARTY_DB
		SPartyInfoList partyInfoList;
#endif

		sprintf_s( szSecret, 32, "ID_SECRET_%02d", i );
		sprintf_s( szTitle, 32, "ID_TITLE_%02d", i );
		sprintf_s( szCount, 32, "ID_COUNT_%02d", i );
		sprintf_s( szBase, 32, "ID_BASE_%02d", i );
		sprintf_s( szUserLevel, 32, "ID_USERLEVEL_%02d", i );
		sprintf_s( szStage, 32, "ID_MAP_%d", i);
		sprintf_s( szDifficulty, 32, "ID_DIFFICULT_%d", i);
		sprintf_s( szVoice, 32, "ID_VOICE_%d", i );
#ifdef PRE_PARTY_DB
		bonusExpPartyCtrlName = FormatA("ID_EXTRABONUS_%d", i);
#endif
#ifdef PRE_WORLDCOMBINE_PARTY
		worldCombineBonusIcons = FormatA("ID_STATIC_BONUSEXP%d", i);
#endif

// #78053 Returnee icon is shown.
		strComebackUser = FormatA("ID_STATIC_RETURNBONUSEXP%d", i);
		CEtUIStatic * pStatic = GetControl<CEtUIStatic>(strComebackUser.c_str());
#ifdef PRE_ADD_NEWCOMEBACK
		//..
#else
		if( pStatic )
			pStatic->Show( false );
#endif // PRE_ADD_NEWCOMEBACK

		partyInfoList.SetControl(	GetControl<CEtUIStatic>(szBase),
									GetControl<CEtUIStatic>(szSecret),
									GetControl<CEtUIStatic>(szTitle),
									GetControl<CEtUIStatic>(szCount),
									GetControl<CEtUIStatic>(szUserLevel)
									,GetControl<CEtUIStatic>(szStage)
									,GetControl<CEtUIStatic>(szDifficulty)
									,GetControl<CEtUIStatic>(szVoice)
#ifdef PRE_PARTY_DB
									,GetControl<CEtUIStatic>(bonusExpPartyCtrlName.c_str())
#endif
#ifdef PRE_WORLDCOMBINE_PARTY
									,GetControl<CEtUIStatic>(worldCombineBonusIcons.c_str())
#endif
#ifdef PRE_ADD_NEWCOMEBACK
									,pStatic	
#endif // PRE_ADD_NEWCOMEBACK

									);

#ifdef PRE_PARTY_DB
		villageIconCtrlName = FormatA("ID_STATIC_PRAIRIE%d", i);
		partyInfoList.InitializeVillageIconControl(1, GetControl<CEtUIStatic>(villageIconCtrlName.c_str()));
		villageIconCtrlName = FormatA("ID_STATIC_MANA%d", i);
		partyInfoList.InitializeVillageIconControl(5, GetControl<CEtUIStatic>(villageIconCtrlName.c_str()));
		villageIconCtrlName = FormatA("ID_STATIC_CATARACT%d", i);
		partyInfoList.InitializeVillageIconControl(8, GetControl<CEtUIStatic>(villageIconCtrlName.c_str()));
		villageIconCtrlName = FormatA("ID_STATIC_SAINT%d", i);
		partyInfoList.InitializeVillageIconControl(11, GetControl<CEtUIStatic>(villageIconCtrlName.c_str()));
		villageIconCtrlName = FormatA("ID_STATIC_LOTUS%d", i);
		partyInfoList.InitializeVillageIconControl(15, GetControl<CEtUIStatic>(villageIconCtrlName.c_str()));
#endif

		partyInfoList.InitInfoList();
		m_vecPartyInfoList.push_back(partyInfoList);
	}

#ifdef PRE_PARTY_DB
	int i = 0;
	std::string toggleBtnCtrlName[eSortMax];
	toggleBtnCtrlName[eLevelAvailable] = "ID_BT_LV";
	toggleBtnCtrlName[eUserCount] = "ID_BT_COUNT";
	toggleBtnCtrlName[eTargetStage] = "ID_BT_MAP";
	toggleBtnCtrlName[eDifficulty] = "ID_BT_DIFFICULT";

	for (; i < eSortMax; ++i)
	{
		m_pSortBtns[i] = GetControl<CDnRotateToggleButton>(toggleBtnCtrlName[i].c_str());
		m_pSortBtns[i]->RegisterRotate(eINCREASE, 2);
		m_pSortBtns[i]->RegisterRotate(eDECREASE, 3);
	}

	m_pTargetStageFilterButton = GetControl<CDnRotateToggleButton>("ID_BT_MODETOGGLE");
	m_pTargetStageFilterButton->RegisterRotate(eTARGETFILTER_ALL, 0, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 79)); // UISTRING : All
	m_pTargetStageFilterButton->RegisterRotate(eTARGETFILTER_DUNGEON, 0, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7110)); // UISTRING : Stage
	m_pTargetStageFilterButton->RegisterRotate(eTARGETFILTER_NEST, 0, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1372)); // UISTRING : Nest
	m_pTargetStageFilterButton->RegisterRotate(eTARGETFILTER_ETC, 0, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2292)); // UISTRING : Misc
	m_pTargetStageFilterButton->SetState(eTARGETFILTER_ALL, false, false, false);
#endif

	m_pPartyPage = GetControl<CEtUIStatic>("ID_PARTY_PAGE");
	m_pSelectBar = GetControl<CEtUIStatic>("ID_SELECT_BAR");
	m_pSelectBar->Show(false);

	m_pButtonPagePrev = GetControl<CEtUIButton>("ID_PARTY_PRIOR");
	m_pButtonPageNext = GetControl<CEtUIButton>("ID_PARTY_NEXT");
	m_pButtonPagePrev->Enable(false);
	m_pButtonPageNext->Enable(false);

	m_pButtonCreate = GetControl<CEtUIButton>("ID_BUTTON_CREATE");
	m_pButtonJoin = GetControl<CEtUIButton>("ID_BUTTON_JOIN");
	m_pButtonRefresh = GetControl<CEtUIButton>("ID_BUTTON_REFRESH");

	m_pPartyCreateDlg = new CDnPartyCreateDlg( UI_TYPE_CHILD_MODAL, this, PARTYCREATE_DIALOG, this );
	m_pPartyCreateDlg->Initialize( false );
	m_PartyDlgGroup.AddDialog( PARTYCREATE_DIALOG, m_pPartyCreateDlg );

	m_pPartyEnterDlg = new CDnPartyEnterDlg(UI_TYPE_TOP_MSG, this, PARTYJOIN_DIALOG, this);
	m_pPartyEnterDlg->Initialize( false, _NORMAL_PARTY );

	m_pPartyEnterPasswordDlg = new CDnPartyEnterPasswordDlg(UI_TYPE_TOP_MSG, this, PARTYJOINPASS_DIALOG, this);
	m_pPartyEnterPasswordDlg->Initialize( false, _NORMAL_PARTY );

	m_pRaidEnterDlg = new CDnPartyEnterDlg(UI_TYPE_TOP_MSG, this, PARTYJOIN_DIALOG, this);
	m_pRaidEnterDlg->Initialize(false, _RAID_PARTY_8);

	m_pRaidEnterPasswordDlg = new CDnPartyEnterPasswordDlg(UI_TYPE_TOP_MSG, this, PARTYJOINPASS_DIALOG, this);
	m_pRaidEnterPasswordDlg->Initialize(false, _RAID_PARTY_8);

	m_pMapSortComboBox = GetControl<CEtUIComboBox>("ID_MAP");
	m_pDifficultySortComboBox = GetControl<CEtUIComboBox>("ID_DIFFICULT");
	m_pSearchBtn = GetControl<CEtUIButton>("ID_BUTTON_SEARCH");

#ifdef PRE_PARTY_DB
	m_pSearchHistoryComboBox = GetControl<CEtUIComboBox>("ID_COMBOBOX_LIST");
	m_pSearchHistoryComboBox->SetEditMode(true);

	m_pSearchEditBox = GetControl<CEtUIIMEEditBox>("ID_IMEEDITBOX_LIST");

	m_pOnlySameVillageCheckBox = GetControl<CEtUICheckBox>("ID_CHECKBOX_TOWNLIST");
	m_pOnlySameVillageCheckBox->SetChecked(true, false);
#endif

	m_pPartySearchDlg = new CDnPartySearchDlg(UI_TYPE_CHILD_MODAL, this, PARTYSEARCH_DIALOG, this);
	m_pPartySearchDlg->Initialize(false);
}

const CVillageClientSession::PartyListStruct* CDnPartyListDlg::GetPartyListInfo( TPARTYID PartyIndex ) const
{
	const CDnVillageTask *pVillageTask = GetVillageTask();
	if (pVillageTask == NULL)
		return NULL;

	const CVillageClientSession::PARTYLISTINFO_MAP& partyList = pVillageTask->GetPartyList();
	if (partyList.size() <= 0)
		return NULL;

	CVillageClientSession::PARTYLISTINFO_MAP_CONST_ITER iter = partyList.begin();
	for (; iter != partyList.end(); ++iter)
	{
		const CVillageClientSession::PartyListStruct& partyStruct = *iter;
		if (partyStruct.PartyID == PartyIndex)
			return &partyStruct;
	}

	return NULL;
}

int CDnPartyListDlg::GetPartyListSize() const
{
	const CDnVillageTask *pVillageTask = GetVillageTask();
	if (pVillageTask == NULL)
		return 0;

	const CVillageClientSession::PARTYLISTINFO_MAP& partyList = pVillageTask->GetPartyList();
	return (int)partyList.size();
}

void CDnPartyListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if (CDnPartyTask::IsActive() && CDnPartyTask::GetInstance().IsLockedReqPartyMemberInfo())
		return;

	if (m_pPartyEnterDlg->IsShow() || m_pPartyEnterPasswordDlg->IsShow() ||
		m_pRaidEnterDlg->IsShow() || m_pRaidEnterPasswordDlg->IsShow())
		return;

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_PARTY_PRIOR" ) )
		{
			PriorPage();
			return;
		}

		if( IsCmdControl("ID_PARTY_NEXT" ) )
		{
			NextPage();
			return;
		}

		if( IsCmdControl("ID_BUTTON_CREATE" ) )
		{
			m_pPartyCreateDlg->SetMode(CDnPartyCreateDlg::PARTYREQ_CREATE);
			m_PartyDlgGroup.ShowDialog(PARTYCREATE_DIALOG, true);
			return;
		}

#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_PARTY_PLAY);
		}
#endif

		if( IsCmdControl("ID_BUTTON_JOIN" ) )
		{
			int nSelectIndex = GetCurSel();
			if (nSelectIndex >= 0 && nSelectIndex < PARTYLISTOFFSET)
			{
				if (nSelectIndex >= int(m_vecPartyInfoList.size()))
				{
					m_nCurSelect = -1;
#ifdef PRE_PARTY_DB
					m_pSelectBar->Show(false);
#endif
					return;
				}

#ifdef PRE_PARTY_DB
				TPARTYID partyIndex = m_vecPartyInfoList[nSelectIndex].GetNumber();
#else
				int partyIndex = m_vecPartyInfoList[nSelectIndex].GetNumber();
#endif
				const CVillageClientSession::PartyListStruct* pPartyInfo = GetPartyListInfo(partyIndex);
				if (pPartyInfo == NULL)
				{
					_ASSERT(0);
					return;
				}
				const CVillageClientSession::PartyListStruct& partyInfo = *pPartyInfo;
				int playerLevel = CDnActor::s_hLocalActor->GetLevel();
#if defined( PRE_PARTY_DB )
				if (partyInfo.nUserLvLimitMin > playerLevel )
#else
				if (partyInfo.nUserLvLimitMin > playerLevel ||
					partyInfo.nUserLvLimitMax < playerLevel )
#endif // #if defined( PRE_PARTY_DB )
				{
					GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3562), MB_OK);	// UISTRING : Cannot join because you do not meet the party level requirement
					return;
				}

				if( CDnPartyTask::IsActive() ) 
				{
					GetPartyTask().ReqPartyMemberInfo(partyIndex);
				}
			//m_PartyDlgGroup.ShowDialog(PARTYJOIN_DIALOG, true);
			}
#if defined (PRE_PARTY_DB)
			else
			{
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3480), true); // UISTRING : Please select a party
			}
#endif
			return;
		}

		if( IsCmdControl("ID_BUTTON_REFRESH" ) )
		{
			if (m_pPartySearchDlg)
			{
#ifdef PRE_PARTY_DB
#else
				m_pPartySearchDlg->SetCurrentPartyListPage(m_nCurrentPage);
#endif
				m_pPartySearchDlg->Show(true);
			}

			return;
		}

		if (IsCmdControl("ID_BUTTON_SEARCH"))
		{
#ifdef PRE_PARTY_DB
			DoSearch();
#else
			int mapSortValue		= DEFAULT_MAP_SORT_COMBOBOX_VALUE;
			int difficultySortValue = DEFAULT_DIFFICULT_SORT_COMBOBOX_VALUE;
			m_pMapSortComboBox->GetSelectedValue(mapSortValue);
			m_pDifficultySortComboBox->GetSelectedValue(difficultySortValue);
			std::vector<int>& stageIdxList = CDnPartyTask::GetInstance().GetPartyStageIdxListForSort();
			stageIdxList.clear();
			CDnPartyTask::GetInstance().SetAdvancedSearching(false);

			CDnPartyTask::GetInstance().SetPartyStageIdxForSort(mapSortValue);
			CDnPartyTask::GetInstance().SetPartyStageDifficultyForSort(difficultySortValue);
			CDnPartyTask::GetInstance().ReqPartyListInfo(m_nCurrentPage, 
														 mapSortValue != DEFAULT_MAP_SORT_COMBOBOX_VALUE || 
														 difficultySortValue != DEFAULT_DIFFICULT_SORT_COMBOBOX_VALUE);

			m_fSearchCoolTime = 5.f;
			m_bSearchCoolTimer = true;
			m_pSearchBtn->Enable(false);
#endif
		}

#ifdef PRE_PARTY_DB
		if (IsCmdControl("ID_BT_LV"))
		{
			ResetAllSortBtn(eLevelAvailable);

			ePartyListSortType type = (ePartyListSortType)uMsg;
			m_CurrentOrderType = (type == eINCREASE) ? Party::OrderType::LevelAsc : Party::OrderType::LevelDesc;

			DoSearch();
		}
		else if (IsCmdControl("ID_BT_COUNT"))
		{
			ResetAllSortBtn(eUserCount);

			ePartyListSortType type = (ePartyListSortType)uMsg;
			m_CurrentOrderType = (type == eINCREASE) ? Party::OrderType::MemberCountAsc : Party::OrderType::MemberCountDesc;

			DoSearch();
		}
		else if (IsCmdControl("ID_BT_MAP"))
		{
			ResetAllSortBtn(eTargetStage);

			ePartyListSortType type = (ePartyListSortType)uMsg;
			m_CurrentOrderType = (type == eINCREASE) ? Party::OrderType::TargetMapAsc : Party::OrderType::TargetMapDesc;

			DoSearch();
		}
		else if (IsCmdControl("ID_BT_DIFFICULT"))
		{
			ResetAllSortBtn(eDifficulty);

			ePartyListSortType type = (ePartyListSortType)uMsg;
			m_CurrentOrderType = (type == eINCREASE) ? Party::OrderType::TargetMapDifficultyAsc : Party::OrderType::TargetMapDifficultyDesc;

			DoSearch();
		}
		else if (IsCmdControl("ID_BT_MODETOGGLE"))
		{
			ResetAllSortBtn(eSortMax);

			eTargetStageFilterType type = (eTargetStageFilterType)uMsg;
			if (type == eTARGETFILTER_ALL)
				m_CurrentStageOrderType = Party::StageOrderType::TargetMapAllDesc;
			else if (type == eTARGETFILTER_DUNGEON)
				m_CurrentStageOrderType = Party::StageOrderType::TargetMapStageDesc;
			else if (type == eTARGETFILTER_NEST)
				m_CurrentStageOrderType = Party::StageOrderType::TargetMapNestDesc;
			else if (type == eTARGETFILTER_ETC)
				m_CurrentStageOrderType = Party::StageOrderType::TargetMapEtcDesc;

			DoSearch();
		}
#endif
	}
#ifdef PRE_PARTY_DB
	else if (nCommand == EVENT_COMBOBOX_DROPDOWN_OPENED && bTriggeredByUser)
	{
		if (IsCmdControl("ID_COMBOBOX_LIST"))
		{
			MakeSearchHistoryWordsComboBox();
			return;
		}
	}
	else if (nCommand == EVENT_COMBOBOX_SELECTION_CHANGED && bTriggeredByUser)
	{
		if (IsCmdControl("ID_COMBOBOX_LIST"))
		{
			SComboBoxItem *pItem = m_pSearchHistoryComboBox->GetSelectedItem();
			if (pItem)
			{
				m_pSearchEditBox->SetText(pItem->strText);
				RequestFocus(m_pSearchEditBox);
			}
		}
	}
	else if (nCommand == EVENT_CHECKBOX_CHANGED && bTriggeredByUser)
	{
		if (IsCmdControl("ID_CHECKBOX_TOWNLIST"))
		{
			DoSearch();
			return;
		}
	}
#endif

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

#ifdef PRE_PARTY_DB
void CDnPartyListDlg::DoSearch()
{
	if (IsSearchBtnLocked())
		return;

	if (m_pSearchEditBox->GetTextLength() > 0 && m_pSearchEditBox->GetTextLength() < Party::Constants::MIN_SEARCH_WORD)
	{
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3470), true); // UISTRING : At least 2 characters are required to search.
		return;
	}

	int mapSortValue		= DEFAULT_MAP_SORT_COMBOBOX_VALUE;
	int difficultySortValue = DEFAULT_DIFFICULT_SORT_COMBOBOX_VALUE;
	m_pMapSortComboBox->GetSelectedValue(mapSortValue);
	m_pDifficultySortComboBox->GetSelectedValue(difficultySortValue);
	std::vector<int>& stageIdxList = CDnPartyTask::GetInstance().GetPartyStageIdxListForSort();
	stageIdxList.clear();
	CDnPartyTask::GetInstance().SetAdvancedSearching(false);

	CDnPartyTask::GetInstance().SetPartyStageIdxForSort(mapSortValue);
	CDnPartyTask::GetInstance().SetPartyStageDifficultyForSort(difficultySortValue);

	m_nCurrentPage = 0;
	CDnPartyTask::GetInstance().ReqPartyListInfo(m_nCurrentPage, 
		m_pSearchEditBox->GetText());

	m_fSearchCoolTime = MAX_PARTY_SEARCH_BTN_COOLTIME;//Party::Constants::PARTY_LIST_REFRESH_SEC_CLIENT;
	m_bSearchCoolTimer = true;
	m_pSearchBtn->Enable(false);
	EnableSortButtons(false);

	m_fOnlySameVillageCoolTimeChecker = MAX_PARTY_ONLY_SAME_VILLAGE_COOLTIME;
	m_bOnlySameVillageCoolTimer = true;
	m_pOnlySameVillageCheckBox->Enable(false);

	if (m_pSearchEditBox->GetTextLength() > 0)
		CDnLocalDataMgr::GetInstance().SetPartySearchHistoryWord(m_pSearchEditBox->GetText());
}

void CDnPartyListDlg::LockSearchBtn(bool bLock)
{
	m_pSearchBtn->Enable(bLock);
	EnableSortButtons(bLock);
}

bool CDnPartyListDlg::IsSearchBtnLocked() const
{
	return !m_pSearchBtn->IsEnable();
}

void CDnPartyListDlg::ResetAllSortBtn(ePartyListSortBtnType excepBtnType)
{
	int i = 0;
	for (; i < eSortMax; ++i)
	{
		if (excepBtnType != i)
			m_pSortBtns[i]->ResetState();
	}
}

void CDnPartyListDlg::MakeSearchHistoryWordsComboBox()
{
	m_pSearchHistoryComboBox->RemoveAllItems();

	const std::deque<std::wstring>& words = CDnLocalDataMgr::GetInstance().GetPartySearchHistoryWords();
	std::deque<std::wstring>::const_iterator iter = words.begin();
	for (; iter != words.end(); ++iter)
	{
		const std::wstring& curWord = (*iter);
		m_pSearchHistoryComboBox->AddItem(curWord.c_str(), NULL, 0, true);
	}

	m_pSearchHistoryComboBox->ClearSelectedItem();
}

void CDnPartyListDlg::MakeHighLightSearchWord(CEtUIStatic* pStatic)
{
	if (pStatic == NULL)
		return;

	std::vector<std::wstring> searchingTokensFromEditBox;
	if (m_pSearchEditBox->GetTextLength() > 0)
	{
		std::wstring staticStringToLower(pStatic->GetText());
		ToLowerW(staticStringToLower);

		std::wstring editBoxStringToLower(m_pSearchEditBox->GetText());
		ToLowerW(editBoxStringToLower);

		searchingTokensFromEditBox.push_back(m_pSearchEditBox->GetText());
#ifdef PRE_FIX_61545
		const WCHAR temp = CommonUtil::GetPartySearchWordSeperator();
		TokenizeW(editBoxStringToLower, searchingTokensFromEditBox, std::wstring(&temp));
#else
		TokenizeW(editBoxStringToLower, searchingTokensFromEditBox);
#endif

		std::vector<std::wstring>::iterator iter = searchingTokensFromEditBox.begin();
		for (; iter != searchingTokensFromEditBox.end();)
		{
			const std::wstring& curSearching = (*iter);
			if (staticStringToLower.find(curSearching) != std::wstring::npos)
			{
				++iter;
			}
			else
			{
				iter = searchingTokensFromEditBox.erase(iter);
			}
		}
	}

	if (searchingTokensFromEditBox.empty() == false)
	{
		std::vector<std::wstring>::const_iterator iter = searchingTokensFromEditBox.begin();
		for (; iter != searchingTokensFromEditBox.end(); ++iter)
		{
			const std::wstring& curToken = (*iter);
			pStatic->SetPartColorText(curToken, D3DCOLOR_ARGB(200, 255, 255, 255), D3DCOLOR_ARGB(200, 255, 255, 0));
		}
	}
	else
	{
		pStatic->ClearPartColorText();
	}
}

void CDnPartyListDlg::MakeSearch(const std::wstring& searchName)
{
	m_pSearchEditBox->SetText(searchName.c_str());

	DoSearch();
}

void CDnPartyListDlg::EnableSortButtons(bool bEnable)
{
	int i = 0;
	for (; i < eSortMax; ++i)
	{
		if (m_pSortBtns[i])
			m_pSortBtns[i]->Enable(bEnable);
	}

	m_pTargetStageFilterButton->Enable(bEnable);
}
#endif // PRE_PARTY_DB

bool CDnPartyListDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() || (CDnPartyTask::IsActive() && CDnPartyTask::GetInstance().IsLockedReqPartyMemberInfo()) )
	{
		return false;
	}

	if (m_pPartyEnterDlg->IsShow() || m_pPartyEnterPasswordDlg->IsShow() ||
		m_pRaidEnterDlg->IsShow() || m_pRaidEnterPasswordDlg->IsShow())
	{
		return false;
	}

#ifdef PRE_PARTY_DB
	if (uMsg == WM_KEYDOWN || uMsg == WM_CHAR)
	{
		if (wParam == VK_RETURN)
		{
			if ((uMsg == WM_CHAR) && m_pSearchEditBox->IsFocus() && m_pSearchBtn->IsEnable())
			{
				DoSearch();
				LockSearchBtn(true);
			}
		}
		//#56897 On "Shift+9" input, uMsg is WM_CHAR and wParam has 0x28 corresponding to '('.
		//Malfunctions when EditBox has focus. Handle VK_### only on WM_KEYDOWN
		else if (m_pSearchEditBox->IsFocus())
		{
			if (uMsg == WM_KEYDOWN && wParam == VK_DOWN)
			{
				m_pSearchHistoryComboBox->ToggleDropDownList();
			}
			else if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
			{
				focus::ReleaseControl();
				return true;
			}
			else
			{
				WCHAR ch = (WCHAR)wParam;
				if (CommonUtil::IsCtrlChar(ch) == false)
				{
					std::wstring curText = m_pSearchEditBox->GetText();
					CommonUtil::eErrorPartySearchWord type = CommonUtil::IsValidPartySearchWord(curText.c_str());
					if (type == CommonUtil::ePARTYWORD_OVER_LENGTH || type == CommonUtil::ePARTYWORD_OVER_SPACING)
						return true;
				}
			}
		}
	}
#endif

	POINT MousePoint;
	float fMouseX, fMouseY;

	MousePoint.x = short( LOWORD( lParam ) );
	MousePoint.y = short( HIWORD( lParam ) );
	PointToFloat( MousePoint, fMouseX, fMouseY );

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
			int nFindSelect = FindInsideItem( fMouseX, fMouseY );
			if (nFindSelect != -1)
			{
				SPartyInfoList& infoList = m_vecPartyInfoList[nFindSelect];
#ifdef PRE_PARTY_DB
				std::vector<CEtUIStatic*> pOnStatics;
				pOnStatics.push_back(infoList.m_pTargetStage);
				pOnStatics.push_back(infoList.m_pUserLevel);

				std::vector<CEtUIStatic*>::iterator iter = pOnStatics.begin();
				for (; iter != pOnStatics.end(); ++iter)
				{
					CEtUIStatic* pOnStatic = (*iter);
					if (pOnStatic)
					{
						std::wstring str = pOnStatic->GetRenderText();
						if (pOnStatic->IsInside(fMouseX, fMouseY) && str.find(L"...") != std::wstring::npos)
						{
							CDnSimpleTooltipDlg* pDlg = GetInterface().GetSimpleTooltipDialog();
							std::wstring presentString = pOnStatic->GetText();
							pDlg->ShowTooltipDlg(pOnStatic, true, presentString, textcolor::WHITE, true);
						}
					}
				}
#else
				CEtUIStatic* pOnStatic = infoList.m_pTargetStage;
				if (pOnStatic)
				{
					std::wstring str = pOnStatic->GetRenderText();
					if (pOnStatic->IsInside(fMouseX, fMouseY) && str.find(L"...") != std::wstring::npos)
					{
						CDnSimpleTooltipDlg* pDlg = GetInterface().GetSimpleTooltipDialog();
						std::wstring presentString = pOnStatic->GetText();
						pDlg->ShowTooltipDlg(pOnStatic, true, presentString, textcolor::WHITE, true);
					}
				}
#endif
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			// Because it's a ChildModal, MsgProc needs to run first or similar. Handle like this for now.
			if( m_pPartyCreateDlg && m_pPartyCreateDlg->IsShow() )
				break;

			if (m_pPartySearchDlg && m_pPartySearchDlg->IsShow())
				break;

			if (m_pMapSortComboBox->IsOpenedDropDownBox() || m_pDifficultySortComboBox->IsOpenedDropDownBox())
				break;

#ifdef PRE_PARTY_DB
			if (m_pSearchHistoryComboBox->IsOpenedDropDownBox())
				break;

			if (m_pSearchEditBox->IsFocus() && m_pSearchEditBox->IsInside(fMouseX, fMouseY) == false)
				focus::ReleaseControl();
#endif

			int nFindSelect = FindInsideItem( fMouseX, fMouseY );
			if (nFindSelect != -1)
			{
#ifdef PRE_PARTY_DB
				if (nFindSelect >= 0 && (int)m_vecPartyInfoList.size() > nFindSelect &&
					m_vecPartyInfoList[nFindSelect].IsEmpty() == false)
#else
				if( GetPartyListSize() > nFindSelect )
#endif
				{
					m_nCurSelect = nFindSelect;
					UpdateSelectBar();
				}
				else
				{
					m_pSelectBar->Show(false);
					m_nCurSelect = -1;
				}
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		{
			// Because it's a ChildModal, MsgProc needs to run first or similar. Handle like this for now.
			if( m_pPartyCreateDlg && m_pPartyCreateDlg->IsShow() )
				break;

			if (m_pPartySearchDlg && m_pPartySearchDlg->IsShow())
				break;

			if( !IsMouseInDlg())
				break;
#ifdef PRE_PARTY_DB
			if ( (m_nCurSelect > PARTYLISTOFFSET) ||
				(m_nCurSelect >= (int)m_vecPartyInfoList.size()) ||
				(m_nCurSelect < 0))
				break;
#endif
			ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonJoin, 0 );
		}
		return true;
	}
	
	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnPartyListDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( nID == PARTYCREATE_DIALOG )
		{
			if( IsCmdControl("ID_BUTTON_CREATE" ) )
			{
				if (m_pPartyCreateDlg->DoCreateParty(CDnPartyCreateDlg::PARTYREQ_CREATE) == false)
					return;
			}

			m_PartyDlgGroup.ShowDialog( PARTYCREATE_DIALOG, false );
			return;
		}
		else if( nID == PARTYJOINPASS_DIALOG )
		{
			CDnPartyEnterPasswordDlg* pDlg = m_pPartyEnterPasswordDlg->IsShow() ? m_pPartyEnterPasswordDlg : m_pRaidEnterPasswordDlg;
			if (pDlg == NULL)
				return;

			if( IsCmdControl("ID_OK") )
			{
				int nSelectIndex = GetCurSel();
				if( nSelectIndex >= 0 && nSelectIndex < PARTYLISTOFFSET )
				{
					JoinParty( m_vecPartyInfoList[nSelectIndex].GetNumber(), pDlg->GetPassword() );
				}
				else
				{
					_ASSERT( 0&&"CDnPartyListDlg::OnUICallbackProc, Invalid nSelectIndex!" );
				}
			}

			pDlg->Show(false);
			return;
		}
	}
}

#ifdef PRE_PARTY_DB
const WCHAR* CDnPartyListDlg::GetCurrentSearchWord() const
{
	return m_pSearchEditBox->GetText();
}

Party::OrderType::eCode CDnPartyListDlg::GetCurrentOrderType() const
{
	return m_CurrentOrderType;
}

Party::StageOrderType::eCode CDnPartyListDlg::GetCurrentStageOrderType() const
{
	return m_CurrentStageOrderType;
}
#endif

void CDnPartyListDlg::ResetSort(bool bUIOnly)
{
	m_pMapSortComboBox->SetSelectedByValue(DEFAULT_MAP_SORT_COMBOBOX_VALUE);
	m_pDifficultySortComboBox->SetSelectedByValue(DEFAULT_DIFFICULT_SORT_COMBOBOX_VALUE);

	if (bUIOnly == false)
	{
		if( CDnPartyTask::IsActive() ) {
			GetPartyTask().SetPartyStageIdxForSort(DEFAULT_MAP_SORT_COMBOBOX_VALUE);
			GetPartyTask().SetPartyStageDifficultyForSort(DEFAULT_DIFFICULT_SORT_COMBOBOX_VALUE);
		}
	}
}

void CDnPartyListDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	m_nCurSelect = -1;

	if (bShow)
	{
		m_nCurrentPage = 0;

		if (m_pPartySearchDlg && m_pPartySearchDlg->IsShow())
			m_pPartySearchDlg->Show(false);

#ifdef PRE_PARTY_DB
		m_pSearchEditBox->ClearText();
#endif

		//	todo by kalliste - Refactoring with CDnPartyCreateDlg::Show()
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );

		if (pSox != NULL && CGlobalInfo::GetInstance().m_nCurrentMapIndex != m_nCurrentMapIdx)
		{
			std::string areaString = pSox->GetFieldFromLablePtr(CGlobalInfo::GetInstance().m_nCurrentMapIndex, "_MapArea")->GetString();

			if (areaString.empty() == false)
			{
				m_pMapSortComboBox->RemoveAllItems();
				m_pMapSortComboBox->AddItem(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3599), NULL, DEFAULT_MAP_SORT_COMBOBOX_VALUE);	// UISTRING : Unlimited
				m_pMapSortComboBox->Enable(true);

				std::vector<std::string> tokens;
				TokenizeA(areaString, tokens, ";");

				std::vector<SPartyTargetMapInfo> mapAreaNameListForSort;
				std::vector<std::string>::const_iterator iter = tokens.begin();
				for (; iter != tokens.end(); ++iter)
				{
					const std::string& dgIDString = (*iter);
					int dgid = atoi(dgIDString.c_str());

					const std::vector<CDnWorldDataManager::SMapAreaInfoUnit>* pMapAreaList = CDnWorldDataManager::GetInstance().GetMapAreaList(dgid);
					if (pMapAreaList != NULL)
					{
						std::vector<CDnWorldDataManager::SMapAreaInfoUnit>::const_iterator areaIter = pMapAreaList->begin();
						for (; areaIter != pMapAreaList->end(); ++areaIter)
						{
							const CDnWorldDataManager::SMapAreaInfoUnit& mapAreaInfoUnit = *areaIter;
							SPartyTargetMapInfo info;
							info.mapName	= GetEtUIXML().GetUIString( CEtUIXML::idCategory1, mapAreaInfoUnit.mapNameId );
							info.mapId		= mapAreaInfoUnit.mapId;
							mapAreaNameListForSort.push_back(info);
						}
					}
				}

				std::sort(mapAreaNameListForSort.begin(), mapAreaNameListForSort.end());

				std::vector<SPartyTargetMapInfo>::const_iterator nameIter = mapAreaNameListForSort.begin();
				for (; nameIter != mapAreaNameListForSort.end(); ++nameIter)
				{
					const SPartyTargetMapInfo& mapInfo = (*nameIter);
					m_pMapSortComboBox->AddItem(mapInfo.mapName.c_str(), NULL, mapInfo.mapId, true);
				}

				m_pDifficultySortComboBox->RemoveAllItems();
				m_pDifficultySortComboBox->AddItem(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3599), NULL, DEFAULT_DIFFICULT_SORT_COMBOBOX_VALUE);	// UISTRING : Unlimited
				m_pDifficultySortComboBox->Enable(true);

				int i = 1;
				std::wstring difficultyText;
				for(; i <= Dungeon::Difficulty::Max; ++i)
				{
					DN_INTERFACE::STRING::GetStageDifficultyText(difficultyText, i);
					m_pDifficultySortComboBox->AddItem(difficultyText.c_str(), NULL, i);
				}

				m_nCurrentMapIdx = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
			}
		}

		if (CDnPartyTask::IsActive() && GetPartyTask().GetPartyListResetSort())
		{
			ResetSort();
			GetPartyTask().SetPartyListResetSort(false);
		}
	}
	else
	{
		ShowChildDialog(m_pPartyCreateDlg, false);
		ShowChildDialog(m_pPartyEnterDlg, false);
		ShowChildDialog(m_pPartyEnterPasswordDlg, false);
		ShowChildDialog(m_pRaidEnterDlg, false);
		ShowChildDialog(m_pRaidEnterPasswordDlg, false);

		m_pSelectBar->Show(false);
	}

	if (CTaskManager::IsActive())
	{
		CDnVillageTask *pTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
		if( pTask )
		{
			pTask->SetPartyListCurrentPage( m_nCurrentPage );
			pTask->EnableRefreshPartyList( bShow );
			if( CDnPartyTask::IsActive() ) {
				GetPartyTask().LockPartyReqPartyMemberInfo(false);
				GetPartyTask().LockPartyReqPartyList(false);
			}
		}
		else
		{
			ClearLists();
			SetPartyList();
		}

#ifdef PRE_PARTY_DB
		m_pSearchEditBox->Enable(pTask != NULL);
		m_pSearchHistoryComboBox->Enable(pTask != NULL);
#endif
	}

	CEtUIDialog::Show( bShow );
}

void CDnPartyListDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() )
	{
		bool bEnable( false );

		if( GetInterface().GetInterfaceType() == CDnInterface::Village )
		{
			bEnable = true;

			if( m_fRefreshTime > 0.0f )
			{
				m_fRefreshTime -= fElapsedTime;
				m_pButtonRefresh->Enable( false );
			}
			else
			{
				m_fRefreshTime = 0.0f;
				m_pButtonRefresh->Enable( true );
			}
		}
		else
		{
			bEnable = false;

			m_pButtonRefresh->Enable( false );
		}

		if (CDnPartyTask::IsActive() && GetPartyTask().IsLockedReqPartyMemberInfo())
		{
			if (m_vecPartyInfoList.size() > 0)
			{
				if (m_vecPartyInfoList[0].IsEnable())
				{
					PARTYINFO_LIST_VEC::iterator iter = m_vecPartyInfoList.begin();
					for (; iter != m_vecPartyInfoList.end(); ++iter)
					{
						SPartyInfoList& infoList = *iter;
						infoList.Enable(false);
					}
				}
			}
		}
		else
		{
			if (m_vecPartyInfoList.size() > 0)
			{
				if (m_vecPartyInfoList[0].IsEnable() == false)
				{
					PARTYINFO_LIST_VEC::iterator iter = m_vecPartyInfoList.begin();
					for (; iter != m_vecPartyInfoList.end(); ++iter)
					{
						SPartyInfoList& infoList = *iter;
						infoList.Enable(true);
					}
				}
			}
		}

		m_pButtonCreate->Enable( bEnable );
		m_pButtonJoin->Enable( bEnable );
		m_pMapSortComboBox->Enable(bEnable);
		m_pDifficultySortComboBox->Enable(bEnable);
#ifdef PRE_PARTY_DB
		bool bEnableSearchBtn = (m_bSearchCoolTimer == false && bEnable);
		m_pSearchBtn->Enable(bEnableSearchBtn);
		EnableSortButtons(bEnableSearchBtn);
		m_pOnlySameVillageCheckBox->Enable(m_bOnlySameVillageCoolTimer == false && bEnable);
#else
		m_pSearchBtn->Enable(m_bSearchCoolTimer == false && bEnable);
		if( GetCurSel() < 0 )
		{
			m_pButtonJoin->Enable( false );
		}
#endif

#ifdef PRE_PARTY_DB
		ProcessCoolTimer(fElapsedTime, m_fSearchCoolTime, m_bSearchCoolTimer);
		ProcessCoolTimer(fElapsedTime, m_fOnlySameVillageCoolTimeChecker, m_bOnlySameVillageCoolTimer);
#else
		if (m_fSearchCoolTime > 0.f)
		{
			m_fSearchCoolTime -= fElapsedTime;
		}
		else
		{
			m_fSearchCoolTime = 0.f;
			if (m_bSearchCoolTimer)
			{
				m_pSearchBtn->Enable(true);
				m_bSearchCoolTimer = false;
			}
		}
#endif
	}
}

#ifdef PRE_PARTY_DB
void CDnPartyListDlg::ProcessCoolTimer(float fElapsedTime, float& coolTimer, bool& bCoolTimeChecker)
{
	if (coolTimer > 0.f)
	{
		coolTimer -= fElapsedTime;
	}
	else
	{
		coolTimer = 0.f;
		if (bCoolTimeChecker)
			bCoolTimeChecker = false;
	}
}

bool CDnPartyListDlg::IsOnlySameVillage() const
{
	return m_pOnlySameVillageCheckBox->IsChecked();
}
#endif

void CDnPartyListDlg::RefreshList()
{
	CDnVillageTask *pTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
	if( !pTask ) return;

	SetPartyList();

	m_nCurSelect = -1;
#ifdef PRE_PARTY_DB
	m_pSelectBar->Show(false);
#endif
}

int CDnPartyListDlg::FindInsideItem( float fX, float fY )
{
	for( int i = 0; i < PARTYLISTOFFSET; i++ )
	{
		if( m_vecPartyInfoList[i].IsInsideItem( fX, fY ) )
		{
			return i;
		}
	}

	return -1;
}

void CDnPartyListDlg::UpdateSelectBar()
{
	SUICoord uiCoordNumber, uiCoordSelectBar;
	if ((m_nCurSelect > PARTYLISTOFFSET) ||
		(m_nCurSelect >= (int)m_vecPartyInfoList.size()))
		return;

	m_vecPartyInfoList[m_nCurSelect].m_pSecret->GetUICoord(uiCoordNumber);
	m_pSelectBar->GetUICoord(uiCoordSelectBar);
#ifdef PRE_PARTY_DB
	uiCoordSelectBar.fY = uiCoordNumber.fY;
#else
	uiCoordSelectBar.fY = uiCoordNumber.fY - 0.01f;
#endif
	m_pSelectBar->SetUICoord(uiCoordSelectBar);
	m_pSelectBar->Show(true);
}

int CDnPartyListDlg::GetStartIndex()
{
	return m_nCurrentPage * PARTYLISTOFFSET;
}

int CDnPartyListDlg::GetMaxPage()
{
#ifdef PRE_PARTY_DB
	CDnVillageTask *pVillageTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
	if( !pVillageTask ) return 1;

	return pVillageTask->GetPartyMaxPage();
#else
	if (GetPartyListSize() <= 0)
		return 1;

	CDnVillageTask *pVillageTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
	if( !pVillageTask ) return 1;

	int nPartyListCount = pVillageTask->GetPartyListCount();

	int nMaxPage = nPartyListCount / PARTYLISTOFFSET;

	if( nPartyListCount % PARTYLISTOFFSET )
	{
		nMaxPage++;
	}

	if( nMaxPage == 0 ) 
		nMaxPage = 1;

	return nMaxPage;
#endif
}

void CDnPartyListDlg::SetPartyList()
{
	int nMaxPage = GetMaxPage();
	if( m_nCurrentPage >= nMaxPage )
		m_nCurrentPage = nMaxPage - 1;

#ifdef PRE_PARTY_DB
	if (UpdatePartyInfoList(m_nCurrentPage) == false)
		return;
#else
	UpdatePartyInfoList();
#endif
	UpdatePage();
	UpdatePageButton();

	CDnVillageTask *pVillageTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
	if( !pVillageTask ) return;

	pVillageTask->SetPartyListCurrentPage( m_nCurrentPage );

	if (m_nCurrentPage > nMaxPage)
	{
		int a = 0;
	}
}

#ifdef PRE_PARTY_DB
void CDnPartyListDlg::ResetPartyList()
{
	ClearLists();
	m_nCurrentPage = 0;
	SetPartyList();
}
#endif

int CDnPartyListDlg::GetCurSel()
{
	return m_nCurSelect;
}

void CDnPartyListDlg::PriorPage()
{
	m_nCurrentPage--;
	if( m_nCurrentPage < 0 )
	{
		m_nCurrentPage = 0;
	}

#ifdef PRE_PARTY_DB
	OnTurnOverPage();
#else
	CDnVillageTask *pTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
	if( pTask ) pTask->RequestPartyListInfo( m_nCurrentPage );

	m_nCurSelect = -1;

	UpdatePage();
#endif
}

void CDnPartyListDlg::NextPage()
{
	m_nCurrentPage++;
	if( m_nCurrentPage >= GetMaxPage() )
	{
		m_nCurrentPage = GetMaxPage() - 1;
	}

#ifdef PRE_PARTY_DB
	OnTurnOverPage();
#else
	CDnVillageTask *pTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
	if( pTask ) pTask->RequestPartyListInfo( m_nCurrentPage );

	m_nCurSelect = -1;

	UpdatePage();
#endif
}

#ifdef PRE_PARTY_DB
void CDnPartyListDlg::OnTurnOverPage()
{
	bool bRequestPartyInfoToServer = false;
	CDnVillageTask *pTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
	if (pTask)
	{
		if (pTask->HasPartyListInfoPage(m_nCurrentPage))
		{
			if (UpdatePartyInfoList(m_nCurrentPage) == false)
				return;

			pTask->SetPartyListCurrentPage(m_nCurrentPage);
		}
		else
		{
			const LPCWSTR pSearchWord = m_pSearchEditBox->GetText();
			pTask->RequestPartyListInfo(m_nCurrentPage, (pSearchWord == NULL) ? L"" : pSearchWord);
			bRequestPartyInfoToServer = true;
		}
	}

	m_nCurSelect = -1;

	UpdatePage();
	m_pSelectBar->Show(false);

	if (bRequestPartyInfoToServer == false)
		UpdatePageButton();
}
#endif

void CDnPartyListDlg::UpdatePage()
{
	wchar_t wszPage[10]={0};
	swprintf_s( wszPage, 10, L"%d/%d", m_nCurrentPage + 1, GetMaxPage() );
	m_pPartyPage->SetText( wszPage );
}

#ifdef PRE_PARTY_DB
bool CDnPartyListDlg::UpdatePartyInfoList(int nPage)
{
	const CDnVillageTask *pVillageTask = GetVillageTask();
	if (pVillageTask == NULL)
		return false;

	const CVillageClientSession::PARTYLISTINFO_MAP& partyList = pVillageTask->GetPartyList();
	if (partyList.size() <= 0)
	{
		InitPartyInfoList();
		return false;
	}

	int listStartOffset = pVillageTask->GetPartyListCountOffset(nPage);
	if (listStartOffset < 0 || listStartOffset >= (int)partyList.size())
		return false;

	InitPartyInfoList();

	int i = 0;
	for (; i < PARTYLISTOFFSET; ++i)
	{
		int listCurrentOffset = listStartOffset + i;
		if (listCurrentOffset >= (int)partyList.size())
			break;

		const CVillageClientSession::PartyListStruct& partyListInfo = partyList[listCurrentOffset];

		m_vecPartyInfoList[i].SetNumber( partyListInfo.PartyID );
		m_vecPartyInfoList[i].SetSecret( partyListInfo.bSecret);
		m_vecPartyInfoList[i].SetTitle( partyListInfo.szPartyName.c_str() );
		MakeHighLightSearchWord(m_vecPartyInfoList[i].m_pTitle);

		m_vecPartyInfoList[i].SetCount( partyListInfo.nCurCount, partyListInfo.nMaxCount);
		m_vecPartyInfoList[i].SetUserLevel(partyListInfo.nUserLvLimitMin);
		m_vecPartyInfoList[i].ShowVillageIcon(partyListInfo.nCurrentMapIdx);
		m_vecPartyInfoList[i].SetVoiceChat( partyListInfo.bUseVoice );

#ifdef PRE_ADD_NEWCOMEBACK
		m_vecPartyInfoList[i].SetComebackUser( partyListInfo.bComeback );
#endif // PRE_ADD_NEWCOMEBACK

		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
		if (pSox)
		{
			int mapIdx = partyListInfo.nMapIdx;
			if (mapIdx == 0)
			{
				m_vecPartyInfoList[i].SetTargetStage(L"");
				m_vecPartyInfoList[i].SetTargetDifficulty(Dungeon::Difficulty::Max);
			}
			else
			{
				TDUNGEONDIFFICULTY difficultyIdx = partyListInfo.Difficulty;
				int nStringTableID = pSox->GetFieldFromLablePtr( mapIdx, "_MapNameID" )->GetInteger();
				if ( Dungeon::Difficulty::Max + 1 < difficultyIdx)
					difficultyIdx = Dungeon::Difficulty::Easy;

				m_vecPartyInfoList[i].SetTargetStage(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringTableID ));
				m_vecPartyInfoList[i].SetTargetDifficulty(difficultyIdx);
			}

			MakeHighLightSearchWord(m_vecPartyInfoList[i].m_pTargetStage);
			MakeHighLightSearchWord(m_vecPartyInfoList[i].m_pTargetDifficulty);

#ifdef PRE_WORLDCOMBINE_PARTY
			SetWorldCombineParty(m_vecPartyInfoList[i], partyListInfo);
#endif
		}
	}

	if( m_nCurSelect >= 0 )
	{
		if( m_vecPartyInfoList[m_nCurSelect].GetNumber() > 0 )
		{
			UpdateSelectBar();
		}
	}

	return true;
}

#ifdef PRE_WORLDCOMBINE_PARTY
void CDnPartyListDlg::SetWorldCombineParty(SPartyInfoList& partyInfo, const CVillageClientSession::PartyListStruct& partyListInfo)
{
	if (partyListInfo.PartyType == _WORLDCOMBINE_PARTY)
	{
		if (CDnPartyTask::IsActive() == false)
			return;

		SWorldCombinePartyData data;
		CDnPartyTask::GetInstance().GetWorldCombinePartyTableData(partyListInfo.nWorldCombinePartyTableIndex, data);
		if (data.IsEmpty())
		{
			DNTableFileFormat* pWorldCombineTable = GetDNTable( CDnTableDB::TWORLDCOMBINEPARTY );
			if (pWorldCombineTable == NULL)
				return;

			if (pWorldCombineTable->IsExistItem(partyListInfo.nWorldCombinePartyTableIndex) == false)
				return;

			SWorldCombinePartyData newOne;
			int i = 0;
			for (; i < Party::Max::WorldCombinePartyBonus; ++i)
			{
				std::string buffColumn, buffDescriptionColumn;
				buffColumn = FormatA("_PartySkill%dID", i);

				CDNTableFile::Cell* pCell = pWorldCombineTable->GetFieldFromLablePtr(partyListInfo.nWorldCombinePartyTableIndex, buffColumn.c_str());
				if (pCell)
				{
					int nSkillID = pCell->GetInteger();
					if (nSkillID > 0)
					{
						newOne.bHasPartyBuff = true;
						buffDescriptionColumn = FormatA("_PartySkill%dDescription", i);
						CDNTableFile::Cell* pCell = pWorldCombineTable->GetFieldFromLablePtr(partyListInfo.nWorldCombinePartyTableIndex, buffDescriptionColumn.c_str());
						if (pCell)
						{
							int descriptionUiStringNum = pCell->GetInteger();
							if (newOne.partyBuffTooltipString.empty() == false)
								newOne.partyBuffTooltipString += L"\n";
							newOne.partyBuffTooltipString += GetEtUIXML().GetUIString(CEtUIXML::idCategory1, descriptionUiStringNum);
						}
					}
				}
			}
			newOne.maxUser = partyListInfo.nMaxCount;

			CDnPartyTask::GetInstance().AddWorldCombinePartyTableDataCache(partyListInfo.nWorldCombinePartyTableIndex, newOne);
			partyInfo.SetWorldCombineParty(true, newOne);
		}
		else
		{
			partyInfo.SetWorldCombineParty(true, data);
		}

		partyInfo.m_nWorldCombinePartyID = partyListInfo.nWorldCombinePartyTableIndex;
	}
	else
	{
		partyInfo.SetWorldCombineParty(false, SWorldCombinePartyData());
	}
}
#endif

#else // PRE_PARTY_DB
									void CDnPartyListDlg::UpdatePartyInfoList()
									{
										InitPartyInfoList();

										const CDnVillageTask *pVillageTask = GetVillageTask();
										if (pVillageTask == NULL)
											return;

										const CVillageClientSession::PARTYLISTINFO_MAP& partyList = pVillageTask->GetPartyList();
										if (partyList.size() <= 0)
											return;

										CVillageClientSession::PARTYLISTINFO_MAP_CONST_ITER iter = partyList.begin();
										for( int i=0; iter != partyList.end(); ++iter, i++ )
										{
											if( i >= PARTYLISTOFFSET )
												break;

											const CVillageClientSession::PartyListStruct& partyListInfo = *iter;
											m_vecPartyInfoList[i].SetNumber( partyListInfo.PartyID );
											m_vecPartyInfoList[i].SetSecret( partyListInfo.bSecret);
											m_vecPartyInfoList[i].SetTitle( partyListInfo.szPartyName.c_str() );

											m_vecPartyInfoList[i].SetCount( partyListInfo.nCurCount, partyListInfo.nMaxCount);
											m_vecPartyInfoList[i].SetUserLevel( partyListInfo.nUserLvLimitMin, partyListInfo.nUserLvLimitMax );
											m_vecPartyInfoList[i].SetVoiceChat( partyListInfo.bUseVoice );

#ifdef PRE_ADD_NEWCOMEBACK
											m_vecPartyInfoList[i].SetComebackUser( partyListInfo.bComeback );
#endif // PRE_ADD_NEWCOMEBACK

											DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
											if (pSox)
											{
												int mapIdx = partyListInfo.nMapIdx;
												if (mapIdx == 0)
												{
													m_vecPartyInfoList[i].SetTargetStage(L"");
													m_vecPartyInfoList[i].SetTargetDifficulty(0);
												}
												else
												{
													int difficultyIdx = partyListInfo.Difficulty;
													int nStringTableID = pSox->GetFieldFromLablePtr( mapIdx, "_MapNameID" )->GetInteger();
													if (difficultyIdx < 0 || Dungeon::Difficulty::Max + 1 < difficultyIdx)
														difficultyIdx = 0;

													m_vecPartyInfoList[i].SetTargetStage(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringTableID ));
													m_vecPartyInfoList[i].SetTargetDifficulty(difficultyIdx);
												}
											}
										}

										if( m_nCurSelect >= 0 )
										{
											if( m_vecPartyInfoList[m_nCurSelect].GetNumber() > 0 )
											{
												UpdateSelectBar();
											}
										}
									}
#endif // PRE_PARTY_DB

void CDnPartyListDlg::InitPartyInfoList()
{
	for( int i=0; i<PARTYLISTOFFSET; i++ )
	{
		m_vecPartyInfoList[i].InitInfoList();
	}

	m_pSelectBar->Show(false);
#ifdef PRE_PARTY_DB
	LockSearchBtn(false);
#endif
}

void CDnPartyListDlg::ClearLists()
{
	InitPartyInfoList();
}

void CDnPartyListDlg::UpdatePageButton()
{
	if( m_nCurrentPage == (GetMaxPage()-1) )
	{
		m_pButtonPageNext->Enable(false);
	}
	else
	{
		m_pButtonPageNext->Enable(true);
	}

	if( m_nCurrentPage == 0 )
	{
		m_pButtonPagePrev->Enable(false);
	}
	else
	{
		m_pButtonPagePrev->Enable(true);
	}
}

void CDnPartyListDlg::RefreshPartyMemberList()
{
	//m_pPartyMemberDlg->SetPartyMemberList();
}

#ifdef PRE_PARTY_DB
void CDnPartyListDlg::JoinParty( TPARTYID nPartyIndex, int iPassword )
{
	CDnPartyTask::GetInstance().ReqJoinParty( nPartyIndex, iPassword );
}
#else
void CDnPartyListDlg::JoinParty( int nPartyIndex, const WCHAR * pPartyPassword )
{
	CDnPartyTask::GetInstance().ReqJoinParty( nPartyIndex, pPartyPassword );
}
#endif

void CDnPartyListDlg::SetDifficultyToPartyCreateDlg(const BYTE* pDifficulties)
{
	//m_pPartyCreateDlg->SetDifficulty(pDifficulties);
}
 
void CDnPartyListDlg::OnRecvPartyMemberInfo(ePartyType type)
{
	int nSelectIndex = GetCurSel();
	if (nSelectIndex < 0 || nSelectIndex >= (int)m_vecPartyInfoList.size())
		return;

	SPartyInfoList& partyInfoList = m_vecPartyInfoList[nSelectIndex];

	std::wstring str;
	const std::wstring& targetString = partyInfoList.m_pTargetStage->GetText();
	if (targetString.empty() == false)
	{
		std::wstring difficultyStr = partyInfoList.m_pTargetDifficulty->GetText();
		if (difficultyStr.empty())
			str = targetString.c_str();
		else
			str = FormatW(L"%s(%s)", targetString.c_str(), partyInfoList.m_pTargetDifficulty->GetText());
	}
	else
		str = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 93); // UISTRING : None

#ifdef PRE_WORLDCOMBINE_PARTY
	if (type == _RAID_PARTY_8)
	{
		SetPartyEnterDlgByRaidOrNormal(true, partyInfoList, str);
	}
	else if (type == _WORLDCOMBINE_PARTY)
	{
		if (CDnPartyTask::IsActive() == false)
			return;

		SWorldCombinePartyData data;
		CDnPartyTask::GetInstance().GetWorldCombinePartyTableData(partyInfoList.m_nWorldCombinePartyID, data);
		if (data.IsEmpty() == false)
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
			SetPartyEnterDlgByRaidOrNormal((data.maxUser > NORMPARTYCOUNTMAX), partyInfoList, str, type);
#else
			SetPartyEnterDlgByRaidOrNormal((data.maxUser > NORMPARTYCOUNTMAX), partyInfoList, str);
#endif
	}
	else
	{
		SetPartyEnterDlgByRaidOrNormal(false, partyInfoList, str);
	}
#else
	if (type == _RAID_PARTY_8)
	{
		if (partyInfoList.IsSecret())
		{
			m_pRaidEnterPasswordDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), str);
			m_pRaidEnterPasswordDlg->Show(true);
		}
		else
		{
			m_pRaidEnterDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), str);
			m_pRaidEnterDlg->Show(true);
		}
	}
	else
	{
		if (partyInfoList.IsSecret())
		{
			m_pPartyEnterPasswordDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), str);
			m_pPartyEnterPasswordDlg->Show(true);
		}
		else
		{
			m_pPartyEnterDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), str);
			m_pPartyEnterDlg->Show(true);
		}
	}
#endif
}

#ifdef PRE_WORLDCOMBINE_PARTY
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
void CDnPartyListDlg::SetPartyEnterDlgByRaidOrNormal(bool bRaid, const SPartyInfoList& partyInfoList, const std::wstring& title, ePartyType type)
{
	if (bRaid)
	{
		if (partyInfoList.IsSecret())
		{
			m_pRaidEnterPasswordDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), title);
			m_pRaidEnterPasswordDlg->SetPartyType(type);
			m_pRaidEnterPasswordDlg->Show(true);
		}
		else
		{
			m_pRaidEnterDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), title);
			m_pRaidEnterDlg->SetPartyType(type);
			m_pRaidEnterDlg->Show(true);
		}
	}
	else
	{
		if (partyInfoList.IsSecret())
		{
			m_pPartyEnterPasswordDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), title);
			m_pPartyEnterPasswordDlg->SetPartyType(type);
			m_pPartyEnterPasswordDlg->Show(true);
		}
		else
		{
			m_pPartyEnterDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), title);
			m_pPartyEnterDlg->SetPartyType(type);
			m_pPartyEnterDlg->Show(true);
		}
	}
}
#endif

void CDnPartyListDlg::SetPartyEnterDlgByRaidOrNormal(bool bRaid, const SPartyInfoList& partyInfoList, const std::wstring& title)
{
	if (bRaid)
	{
		if (partyInfoList.IsSecret())
		{
			m_pRaidEnterPasswordDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), title);
			m_pRaidEnterPasswordDlg->Show(true);
		}
		else
		{
			m_pRaidEnterDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), title);
			m_pRaidEnterDlg->Show(true);
		}
	}
	else
	{
		if (partyInfoList.IsSecret())
		{
			m_pPartyEnterPasswordDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), title);
			m_pPartyEnterPasswordDlg->Show(true);
		}
		else
		{
			m_pPartyEnterDlg->SetPartyInfo(partyInfoList.GetNumber(), partyInfoList.m_pTitle->GetText(), title);
			m_pPartyEnterDlg->Show(true);
		}
	}
}
#endif	// PRE_WORLDCOMBINE_PARTY

void CDnPartyListDlg::OnPartyJoinFailed()
{
	if (m_pPartyEnterPasswordDlg && m_pPartyEnterPasswordDlg->IsShow())
		m_pPartyEnterPasswordDlg->OnPartyJoinFailed();
	if (m_pRaidEnterPasswordDlg && m_pRaidEnterPasswordDlg->IsShow())
		m_pRaidEnterPasswordDlg->OnPartyJoinFailed();
}

const CDnVillageTask* CDnPartyListDlg::GetVillageTask() const
{
	const CDnVillageTask *pVillageTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask("VillageTask");
	if (pVillageTask != NULL)
		return pVillageTask;

	return NULL;
}
