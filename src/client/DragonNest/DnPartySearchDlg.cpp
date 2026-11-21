#include "StdAfx.h"
#include "DnPartySearchDlg.h"
#include "DnPartyTask.h"
#include "DnTableDB.h"
#include "DnPartySearchItemDlg.h"
#include "DnInterfaceString.h"
#include "DnQuestTask.h"
#include "DnWorldDataManager.h"
#include "DnMissionTask.h"
#include "DnInterface.h"
#include "DnLocalPlayerActor.h"
#ifdef PRE_PARTY_DB
#include "DnPartyListDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#define _MAX_MISSION_ACHIEVE_TYPE 5

CDnPartySearchDlg::CDnPartySearchDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CEtUIDialog(dialogType, pParentDialog, nID, pCallback, true)
{
	m_pCurConditionStatic = NULL;
	int i = 0;
	for (; i < eSEARCH_MAX; ++i)
	{
		m_pSearchComboBox[i] = NULL;
		m_pDescriptions[i] = NULL;
	}

	m_pSearch = NULL;
	m_pClose = NULL;
	m_pListBoxEx = NULL;
	m_nCurSelectQuestMapStringId = -1;
	m_nCurSelectMisssionMapStringId = -1;
	m_nCurSelectDifficulty = -1;
#ifdef PRE_PARTY_DB
	m_pCurVillageDungeonCheckBox = NULL;
#else
	m_nCurPartyListPage = 0;
#endif
	m_nCurrentMapIdx = 0;
}

CDnPartySearchDlg::~CDnPartySearchDlg(void)
{
	m_pListBoxEx->RemoveAllItems();
	int i = 0;
	for (; i < eSEARCH_MAX; ++i)
	{
		if (m_pSearchComboBox[i])
			m_pSearchComboBox[i]->RemoveAllItems();
	}
}

void CDnPartySearchDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PartySearchDlg.ui" ).c_str(), bShow );

	SetDifficultyComboBox();
}

void CDnPartySearchDlg::InitialUpdate()
{
	int i = 0;
	std::string str;
	m_pCurConditionStatic = GetControl<CEtUIStatic>("ID_NOW_NAME");
	for (; i < eSEARCH_MAX; ++i)
	{
		str = FormatA("ID_COMBOBOX_%d", i);
		m_pSearchComboBox[i] = GetControl<CEtUIComboBox>(str.c_str());
	}

	for (i = eSEARCH_QUEST; i < eSEARCH_MAX; ++i)
	{
		str = FormatA("ID_TEXTBOX_%d", i);
		m_pDescriptions[i] = GetControl<CEtUITextBox>(str.c_str());
	}

	m_pSearch		= GetControl<CEtUIButton>("ID_BUTTON_OK");
	m_pClose		= GetControl<CEtUIButton>("ID_BUTTON_CLOSE");
	m_pListBoxEx	= GetControl<CEtUIListBoxEx>( "ID_TITLE_NAME_LISTBOX" );
#ifdef PRE_PARTY_DB
	m_pCurVillageDungeonCheckBox = GetControl<CEtUICheckBox>("ID_CHECKBOX0");
	m_pCurVillageDungeonCheckBox->SetChecked(true, false);
#endif
}

void CDnPartySearchDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		SetDifficultyComboBox();

		m_nCurSelectDifficulty = -1;
		m_nCurSelectQuestMapStringId = -1;
		m_nCurSelectMisssionMapStringId = -1;
		if (CDnPartyTask::IsActive())
		{
			if (CGlobalInfo::GetInstance().m_nCurrentMapIndex != m_nCurrentMapIdx)
			{
				m_nCurrentMapIdx = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
#ifdef PRE_PARTY_DB
				CDnPartyTask::GetInstance().MakePartyTargetStageList(m_nCurrentMapIdx, m_MapAreaNameList, true);
#else
				CDnPartyTask::GetInstance().MakePartyTargetStageList(CGlobalInfo::GetInstance().m_nCurrentMapIndex, m_MapAreaNameList, m_nMapLimitLevelList);
#endif
			}
		}

		SetQuestComboBox();
		SetMissionComboBox();
		RefreshListBox();
		RefreshSearchCondition();

		SetRenderPriority(this, bShow);
	}

	if( CDnActor::s_hLocalActor )
	{
		if( CDnActor::s_hLocalActor->IsMove() && CDnActor::s_hLocalActor->IsMovable() ) 
			CDnActor::s_hLocalActor->CmdStop( "Stand", 0, 3.f, 0.f, false, true );
	}
	CDnLocalPlayerActor::LockInput(bShow);

	CEtUIDialog::Show( bShow );
}

bool CDnPartySearchDlg::DoAddListBox(const SPartyTargetMapInfo& info, int nMapStringId)
{
	if (IsMapInSearchArea(info.mapId, nMapStringId))
	{
		CDnPartySearchItemDlg* pItem = m_pListBoxEx->AddItem<CDnPartySearchItemDlg>(info.mapId);
		if (pItem)
		{
#if defined _RDEBUG && defined PRE_MOD_PARTY_SEARCH_MINLEV
			int minLevel = -1;
			std::map<int, int>::const_iterator limitLevelIter = m_nMapLimitLevelList.find(info.mapId);
			if (limitLevelIter !=m_nMapLimitLevelList.end())
				minLevel = (*limitLevelIter).second;
			std::wstring temp;
			temp = FormatW(L"%s(%d)", info.mapName, minLevel);
			pItem->SetInfo(temp, info.mapId, this);
#else
			pItem->SetInfo(info.mapName, info.mapId, this);
#endif
			return true;
		}
	}

	return false;
}

#ifdef PRE_MOD_PARTY_SEARCH_MINLEV
bool CDnPartySearchDlg::IsPlayerEnterMap(int mapId) const
{
	if (CDnActor::s_hLocalActor)
	{
		std::map<int, int>::const_iterator limitLevelIter = m_nMapLimitLevelList.find(mapId);
		if (limitLevelIter !=m_nMapLimitLevelList.end())
		{
			const int& minLevel = (*limitLevelIter).second;
			if (minLevel > CDnActor::s_hLocalActor->GetLevel())
				return false;
		}

		return true;
	}

	return false;
}
#endif

void CDnPartySearchDlg::RefreshListBox()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if (pSox == NULL || 
		m_pSearchComboBox[eSEARCH_QUEST] == NULL || m_pSearchComboBox[eSEARCH_DAILYMISSION] == NULL ||
		m_pDescriptions[eSEARCH_QUEST] == NULL || m_pDescriptions[eSEARCH_DAILYMISSION] == NULL)
		return;

	std::vector<int>& nMapSelected = CDnPartyTask::GetInstance().GetPartyStageIdxListForSort();
	std::string areaString = pSox->GetFieldFromLablePtr(CGlobalInfo::GetInstance().m_nCurrentMapIndex, "_MapArea")->GetString();
	if (areaString.empty() == false)
	{
		m_pListBoxEx->RemoveAllItems();
		nMapSelected.clear();
	}
	else
	{
		return;
	}

	m_pDescriptions[eSEARCH_QUEST]->ClearText();
	m_pDescriptions[eSEARCH_DAILYMISSION]->ClearText();

#ifdef PRE_PARTY_DB
	const std::vector<SPartyTargetMapInfo>& areaNameList = GetPartyTargetMapList();
	if (areaNameList.empty() == false)
	{
		std::vector<SPartyTargetMapInfo>::const_iterator iter = areaNameList.begin();
		for (; iter != areaNameList.end(); ++iter)
		{
#else
	if (m_MapAreaNameList.empty() == false)
	{
		std::vector<SPartyTargetMapInfo>::const_iterator iter = m_MapAreaNameList.begin();
		for (; iter != m_MapAreaNameList.end(); ++iter)
		{
#endif
			const SPartyTargetMapInfo& mapInfo = *iter;
			if (mapInfo.IsValid())
			{
#ifdef PRE_MOD_PARTY_SEARCH_MINLEV
				if (IsPlayerEnterMap(mapInfo.mapId) == false)
					continue;
#endif

				bool bNoFiltering = true;
				bool bAdded = false;

				int nMapStringId = 0;
				m_pSearchComboBox[eSEARCH_QUEST]->GetSelectedValue(nMapStringId);
				if (nMapStringId != 0)
				{
					bNoFiltering = false;
					bAdded = DoAddListBox(mapInfo, nMapStringId);
				}

				if (bAdded == false)
				{
					m_pSearchComboBox[eSEARCH_DAILYMISSION]->GetSelectedValue(nMapStringId);
					if (nMapStringId != 0)
					{
						bNoFiltering = false;
						bAdded = DoAddListBox(mapInfo, nMapStringId);
					}
				}

				if (bNoFiltering)
				{
					CDnPartySearchItemDlg* pItem = m_pListBoxEx->AddItem<CDnPartySearchItemDlg>(mapInfo.mapId);
					if (pItem)
					{
#if defined _RDEBUG && defined PRE_MOD_PARTY_SEARCH_MINLEV
						int minLevel = -1;
						std::map<int, int>::const_iterator limitLevelIter = m_nMapLimitLevelList.find(mapInfo.mapId);
						if (limitLevelIter !=m_nMapLimitLevelList.end())
							minLevel = (*limitLevelIter).second;
						std::wstring temp;
						temp = FormatW(L"%s(%d)", mapInfo.mapName.c_str(), minLevel);
						pItem->SetInfo(temp, mapInfo.mapId, this);
#else
						pItem->SetInfo(mapInfo.mapName, mapInfo.mapId, this);
#endif
					}
				}
			}
		}
	}

	if (m_pListBoxEx->GetSelectedIndex() < 0)
	{
		int i = eSEARCH_QUEST;
		for (; i < eSEARCH_MAX; ++i)
		{
			if (m_pDescriptions[i])
				m_pDescriptions[i]->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1381));	// UISTRING : No stage selected.
		}
	}
}

void CDnPartySearchDlg::RefreshQuestDescription(int nMapId)
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if (CDnWorldDataManager::IsActive() == false || CDnQuestTask::IsActive() == false || m_pDescriptions[eSEARCH_QUEST] == NULL || pSox == NULL)
	{
		_ASSERT(0 && "[PARTY_SEARCH]RefreshQuestDescription : CDnWorldDataManager / CDnQuestTask / m_pDescriptions[eSEARCH_QUEST] is NOT VALID");
		return;
	}

	m_pDescriptions[eSEARCH_QUEST]->ClearText();

	DNTableCell* pField = pSox->GetFieldFromLablePtr(nMapId, "_MapNameID");
	if (pField == NULL)
		return;

	int nMapTableStringId = pField->GetInteger();
	const std::vector<int>* pMapGroupList = CDnWorldDataManager::GetInstance().GetMapGroupByName(nMapTableStringId);
	if (pMapGroupList == NULL)
		return;

	std::vector<TQuest*> vecProgQuest;
	CDnQuestTask::GetInstance().GetPlayQuest(vecProgQuest);

	std::wstring questDesc;

	std::vector<TQuest*>::const_iterator iter = vecProgQuest.begin();
	for (; iter != vecProgQuest.end(); ++iter)
	{
		TQuest* pQuest = *iter;
		if (pQuest)
		{
			Journal* pJournal = g_DataManager.GetJournalData(pQuest->nQuestID);
			if ( !pJournal )
				continue;

			JournalPage* pJournalPage = pJournal->FindJournalPage(pQuest->cQuestJournal);
			if ( !pJournalPage )
				continue;

			std::vector<int>::const_iterator destMapIter = std::find(pMapGroupList->begin(), pMapGroupList->end(), pJournalPage->nDestnationMapIndex);
			if (destMapIter != pMapGroupList->end())
				questDesc += FormatW(L"%s\n", pJournal->wszQuestTitle.c_str());
		}
	}

	if (questDesc.empty())
		questDesc = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1379); // UISTRING : No related quests.

	m_pDescriptions[eSEARCH_QUEST]->SetText(questDesc.c_str());
}

void CDnPartySearchDlg::RefreshMissionDescription(int nMapId)
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pMissionSox = GetDNTable(CDnTableDB::TDAILYMISSION);
	if (CDnWorldDataManager::IsActive() == false || CDnMissionTask::IsActive() == false || 
		m_pDescriptions[eSEARCH_DAILYMISSION] == NULL || pSox == NULL || pMissionSox == NULL)
	{
		_ASSERT(0 && "[PARTY_SEARCH]RefreshQuestDescription : CDnWorldDataManager / CDnMissionTask / m_pDescriptions[eSEARCH_DAILYMISSION] is NOT VALID");
		return;
	}

	m_pDescriptions[eSEARCH_DAILYMISSION]->ClearText();

	DNTableCell* pField = pSox->GetFieldFromLablePtr(nMapId, "_MapNameID");
	if (pField == NULL)
		return;

	int nMapTableStringId = pField->GetInteger();
	const std::vector<int>* pMapGroupList = CDnWorldDataManager::GetInstance().GetMapGroupByName(nMapTableStringId);
	if (pMapGroupList == NULL)
		return;

	std::wstring missionDesc;
	int i = 0;
	CDnMissionTask::MissionInfoStruct *pStruct;
	for (; i < (int)CDnMissionTask::GetInstance().GetDailyMissionCount(CDnMissionTask::Daily); ++i)
	{
		pStruct = CDnMissionTask::GetInstance().GetDailyMissionInfo(CDnMissionTask::Daily, i);
		if (pStruct == NULL)
			continue;

		if (pStruct->bAchieve)
			continue;

		bool bAdd = false;
		int j = 0;
		for (; j < _MAX_MISSION_ACHIEVE_TYPE; ++j)
		{
			std::string fieldElem = FormatA("_Achieve%dType", j);
			int type = pMissionSox->GetFieldFromLablePtr(pStruct->nNotifierID, fieldElem.c_str())->GetInteger();
			if (type != MapID)
				continue;

			fieldElem = FormatA("_Achieve%dParam", j);
			int mapId = pMissionSox->GetFieldFromLablePtr(pStruct->nNotifierID, fieldElem.c_str())->GetInteger();

			if (IsMapInSearchArea(mapId, nMapTableStringId))
			{
				missionDesc += FormatW(L"%s\n", pStruct->szTitle.c_str());
				bAdd = true;
				break;
			}
		}
	}

	if (missionDesc.empty())
		missionDesc = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1380); // UISTRING : No related daily missions.

	m_pDescriptions[eSEARCH_DAILYMISSION]->SetText(missionDesc.c_str());
}

void CDnPartySearchDlg::RefreshSearchCondition()
{
	m_pCurConditionStatic->ClearText();
	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TMAP);
	if (pSox == NULL)
		return;

	std::wstring condition;
	const std::vector<int>& nMapSelected = CDnPartyTask::GetInstance().GetPartyStageIdxListForSort();
	std::vector<int>::const_iterator iter = nMapSelected.begin();
	for (; iter != nMapSelected.end(); ++iter)
	{
		const int& mapSelectedId = *iter;
		DNTableCell* pField = pSox->GetFieldFromLablePtr(mapSelectedId, "_MapNameID");
		if (pField == NULL)
			break;

		int nMapTableStringId = pField->GetInteger();

		condition += FormatW(L"%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nMapTableStringId));
		if (mapSelectedId != nMapSelected.back())
			condition += L", ";
	}

	condition += L"   ";
	int value = 0;
	m_pSearchComboBox[eSEARCH_DIFFICULTY]->GetSelectedValue(value);
	if (m_pSearchComboBox[eSEARCH_DIFFICULTY] && value > 0)
	{
		std::wstring difficultyText;
		DN_INTERFACE::STRING::GetStageDifficultyText(difficultyText, value);
		condition += difficultyText;
	}

	m_pCurConditionStatic->SetText(condition.c_str());
}

void CDnPartySearchDlg::HandlePartySearchError(ePartySearchError code)
{
	std::wstring whole, errString;
	switch(code)
	{
	case eERRPS_FAIL_NO_STAGE:	errString = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1381); break;	// UISTRING : No stage selected. 
	default:					errString = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3437); break;	// UISTRING : Party search error. Please close the search window and try again.
	}

	CDnInterface::GetInstance().MessageBox(errString.c_str(), MB_OK);
}

void CDnPartySearchDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if (IsCmdControl("ID_BUTTON_CLOSE"))
		{
			Show(false);
			return;
		}
		else if (IsCmdControl("ID_BUTTON_OK"))
		{
			CDnPartyTask::GetInstance().SetAdvancedSearching(true);
			CDnPartyTask::GetInstance().SetPartyStageDifficultyForSort(m_nCurSelectDifficulty);

#ifdef PRE_PARTY_DB
			const std::vector<int>& stageIdxList = CDnPartyTask::GetInstance().GetPartyStageIdxListForSort();
			if (stageIdxList.empty() == false)
			{
				std::wstring mapName;
				const int& mapIdx = stageIdxList.back();
				DN_INTERFACE::STRING::GetMapName(mapName, mapIdx);

				if (mapName.empty() == false)
				{
					CDnPartyListDlg* pPartyListDlg = static_cast<CDnPartyListDlg*>(m_pParentDialog);
					if (pPartyListDlg == NULL)
						return;

					pPartyListDlg->MakeSearch(mapName.c_str());
				}
			}
#else
			CDnPartyTask::GetInstance().ReqPartyListInfo(m_nCurPartyListPage, 
				m_nCurSelectDifficulty > 0 || CDnPartyTask::GetInstance().GetPartyStageIdxListForSort().empty() == false); // DEFAULT_MAP_SORT_COMBOBOX_VALUE
#endif

			Show(false);

			return;
		}
	}
	else if (nCommand == EVENT_COMBOBOX_SELECTION_CHANGED && bTriggeredByUser)
	{
		if (IsCmdControl("ID_COMBOBOX_0"))	// eSEARCH_DIFFICULTY
		{
			CEtUIComboBox* pCurComboBox = static_cast<CEtUIComboBox*>(pControl);
			if (pCurComboBox == NULL)
				return;

			int selValue = 0;
			pCurComboBox->GetSelectedValue(selValue);
			if (selValue == m_nCurSelectDifficulty)
				return;

			RefreshSearchCondition();
			pCurComboBox->GetSelectedValue(m_nCurSelectDifficulty);
			return;
		}
		else if (IsCmdControl("ID_COMBOBOX_1"))	// eSEARCH_QUEST
		{
			CEtUIComboBox* pCurComboBox = static_cast<CEtUIComboBox*>(pControl);
			if (pCurComboBox == NULL)
				return;

			int selValue = 0;
			pCurComboBox->GetSelectedValue(selValue);
			if (selValue == m_nCurSelectQuestMapStringId)
				return;

			RefreshListBox();
			RefreshSearchCondition();
			pCurComboBox->GetSelectedValue(m_nCurSelectQuestMapStringId);
			return;
		}
		else if (IsCmdControl("ID_COMBOBOX_2"))	// eSEARCH_DAILYMISSION
		{
			CEtUIComboBox* pCurComboBox = static_cast<CEtUIComboBox*>(pControl);
			if (pCurComboBox == NULL)
				return;

			int selValue = 0;
			pCurComboBox->GetSelectedValue(selValue);
			if (selValue == m_nCurSelectMisssionMapStringId)
				return;

			RefreshListBox();
			RefreshSearchCondition();
			pCurComboBox->GetSelectedValue(m_nCurSelectMisssionMapStringId);
			return;
		}
	}
	else if (nCommand == EVENT_LISTBOX_SELECTION && bTriggeredByUser)
	{
		if (IsCmdControl("ID_TITLE_NAME_LISTBOX"))
		{
			CEtUIListBoxEx* pListBox = static_cast<CEtUIListBoxEx*>(pControl);
			if (pListBox == NULL)
				return;

			int select = pListBox->GetSelectedIndex();
			if (select != -1)
			{
				CDnPartySearchItemDlg* pItem = pListBox->GetItem<CDnPartySearchItemDlg>(select);
				if (pItem)
				{
#ifdef PRE_PARTY_DB
					OnSetMapSelect(pItem->GetMapIdForMSG(), true);
					RefreshSearchCondition();
#endif
					RefreshQuestDescription(pItem->GetDialogID());
					RefreshMissionDescription(pItem->GetDialogID());
				}
			}

			return;
		}
	}
#ifdef PRE_PARTY_DB
	else if (nCommand == EVENT_CHECKBOX_CHANGED && bTriggeredByUser)
	{
		if (IsCmdControl("ID_CHECKBOX0"))
		{
			if (m_pCurVillageDungeonCheckBox->IsChecked() == false)
			{
				CDnPartyTask::GetInstance().MakePartyTargetStageList(CGlobalInfo::GetInstance().m_nCurrentMapIndex, m_MapAreaNameList);
				CDnPartyTask::GetInstance().MakePartyTargetStageLevelLimitList(m_nMapLimitLevelList, m_MapAreaNameList);
			}

			SetQuestComboBox();
			SetMissionComboBox();
			RefreshListBox();
			RefreshSearchCondition();

			return;
		}
	}
#else
	else if (nCommand == EVENT_CHECKBOX_CHANGED && bTriggeredByUser)
	{
		if (IsCmdControl("ID_CHECKBOX0"))
		{
			CEtUICheckBox* pCheckBox = static_cast<CEtUICheckBox*>(pControl);
			if (pCheckBox)
			{
				bool bOk = OnSetMapSelect(uMsg, pCheckBox->IsChecked());
				if (bOk == false && pCheckBox->IsChecked())
					pCheckBox->SetChecked(false);
				else
					RefreshSearchCondition();
			}

			return;
		}
	}
#endif

	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnPartySearchDlg::SetDifficultyComboBox()
{
	if (m_pSearchComboBox[eSEARCH_DIFFICULTY] == NULL)
	{
		_ASSERT(0&&"DIFFICULT COMBOBOX ERROR");
		return;
	}

	m_pSearchComboBox[eSEARCH_DIFFICULTY]->RemoveAllItems();
	m_pSearchComboBox[eSEARCH_DIFFICULTY]->AddItem(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3599), NULL, 0);	// UISTRING : Unlimited

	int i = 1;
	std::wstring difficultyText;
	for(; i <= Dungeon::Difficulty::Max; ++i)
	{
		DN_INTERFACE::STRING::GetStageDifficultyText(difficultyText, i);
		m_pSearchComboBox[eSEARCH_DIFFICULTY]->AddItem(difficultyText.c_str(), NULL, i);
	}
}

void CDnPartySearchDlg::SetQuestComboBox()
{
	if (CDnQuestTask::IsActive() == false || m_pSearchComboBox[eSEARCH_QUEST] == NULL)
	{
		_ASSERT(0&&"QUEST COMBOBOX ERROR OR QUEST TASK IS NOT ACTIVE");
		return;
	}

	m_pSearchComboBox[eSEARCH_QUEST]->RemoveAllItems();
	m_pSearchComboBox[eSEARCH_QUEST]->AddItem(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3599), NULL, 0);	// UISTRING : Unlimited

	std::vector<TQuest*> vecProgQuest;
	CDnQuestTask::GetInstance().GetPlayQuest(vecProgQuest);

	std::vector<TQuest*>::const_iterator iter = vecProgQuest.begin();
	for (; iter != vecProgQuest.end(); ++iter)
	{
		TQuest* pQuest = *iter;
		if (pQuest)
			DoAddQuestComboBox(pQuest);
	}
}

bool CDnPartySearchDlg::IsMapStringInMapAreaNameList(const std::wstring& mapName) const
{
#ifdef PRE_PARTY_DB
	const std::vector<SPartyTargetMapInfo>& areaNameList = GetPartyTargetMapList();
	std::vector<SPartyTargetMapInfo>::const_iterator iter = areaNameList.begin();
	for (; iter != areaNameList.end(); ++iter)
#else
	std::vector<SPartyTargetMapInfo>::const_iterator iter = m_MapAreaNameList.begin();
	for (; iter != m_MapAreaNameList.end(); ++iter)
#endif
	{
		const SPartyTargetMapInfo& info = *iter;
#ifdef PRE_MOD_PARTY_SEARCH_MINLEV
		if (info.mapName.compare(mapName) == 0 && IsPlayerEnterMap(info.mapId))
#else
		if (info.mapName.compare(mapName) == 0)
#endif
			return true;
	}

	return false;
}

void CDnPartySearchDlg::SetMissionComboBox()
{
	if (CDnMissionTask::IsActive() == false || m_pSearchComboBox[eSEARCH_DAILYMISSION] == NULL)
	{
		_ASSERT(0&&"MISSION COMBOBOX ERROR OR MISSION TASK IS NOT ACTIVE");
		return;
	}

	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TDAILYMISSION);
	if (pSox == NULL)
		return;

	m_pSearchComboBox[eSEARCH_DAILYMISSION]->RemoveAllItems();
	m_pSearchComboBox[eSEARCH_DAILYMISSION]->AddItem(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3599), NULL, 0);	// UISTRING : Unlimited

	int i = 0;
	CDnMissionTask::MissionInfoStruct *pStruct;
	for (; i < (int)CDnMissionTask::GetInstance().GetDailyMissionCount(CDnMissionTask::Daily); ++i)
	{
		pStruct = CDnMissionTask::GetInstance().GetDailyMissionInfo(CDnMissionTask::Daily, i);
		if (pStruct == NULL)
			continue;

		if (pStruct->bAchieve)
			continue;

		bool bAdd = false;
		int j = 0;
		for (; j < _MAX_MISSION_ACHIEVE_TYPE; ++j)
		{
			std::string fieldElem = FormatA("_Achieve%dType", j);
			int type = pSox->GetFieldFromLablePtr(pStruct->nNotifierID, fieldElem.c_str())->GetInteger();
			if (type != MapID)
				continue;

			fieldElem = FormatA("_Achieve%dParam", j);
			int mapId = pSox->GetFieldFromLablePtr(pStruct->nNotifierID, fieldElem.c_str())->GetInteger();

			DoAddMissionComboBox(pStruct->szTitle, mapId);
			bAdd = true;
			break;
		}
	}
}

bool CDnPartySearchDlg::IsMapInSearchArea(int mapIdx, int mapNameId) const
{
	if (CDnWorldDataManager::IsActive() == false)
		return false;

	const std::vector<int>* pList = CDnWorldDataManager::GetInstance().GetMapGroupByName(mapNameId);
	if (pList)
	{
		std::vector<int>::const_iterator iter = std::find(pList->begin(), pList->end(), mapIdx);
		if (iter != pList->end())
			return true;
	}

	return false;
}

void CDnPartySearchDlg::DoAddQuestComboBox(TQuest* pQuest)
{
	if (pQuest == NULL)
		return;

	Journal* pJournal = g_DataManager.GetJournalData(pQuest->nQuestID);
	if ( !pJournal )
		return;

	JournalPage* pJournalPage = pJournal->FindJournalPage(pQuest->cQuestJournal);
	if ( !pJournalPage )
		return;

	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TMAP);
	if (pSox == NULL)
		return;
	
	DNTableCell* pField = pSox->GetFieldFromLablePtr(pJournalPage->nDestnationMapIndex, "_MapNameID");
	if (pField == NULL)
		return;

	int nMapTableStringId = pField->GetInteger();
	std::wstring curMapName = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nMapTableStringId);
	if (IsMapStringInMapAreaNameList(curMapName) == false)
		return;

	if (IsMapInSearchArea(pJournalPage->nDestnationMapIndex, nMapTableStringId) == false)
		return;

	m_pSearchComboBox[eSEARCH_QUEST]->AddItem(pJournal->wszQuestTitle.c_str(), NULL, nMapTableStringId);
}

void CDnPartySearchDlg::DoAddMissionComboBox(const std::wstring& missionName, int mapId)
{
	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TMAP);
	if (pSox == NULL)
		return;

	DNTableCell* pField = pSox->GetFieldFromLablePtr(mapId, "_MapNameID");
	if (pField == NULL)
		return;

	int nMapTableStringId = pField->GetInteger();
	std::wstring curMapName = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nMapTableStringId);
	if (IsMapStringInMapAreaNameList(curMapName) == false)
		return;

	if (IsMapInSearchArea(mapId, nMapTableStringId) == false)
		return;

	m_pSearchComboBox[eSEARCH_DAILYMISSION]->AddItem(missionName.c_str(), NULL, nMapTableStringId);
}

bool CDnPartySearchDlg::OnSetMapSelect(int nMapId, bool bOn)
{
	std::vector<int>& nMapSelected = CDnPartyTask::GetInstance().GetPartyStageIdxListForSort();

	if (bOn)
	{
		return CDnPartyTask::GetInstance().SetPartyStageIdxForSort(nMapId);
	}
	else
	{
		if (nMapSelected.size() <= 0)
			return false;

		std::vector<int>::iterator iter = std::find(nMapSelected.begin(), nMapSelected.end(), nMapId);
		if (iter != nMapSelected.end())
			nMapSelected.erase(iter);
	}

	return true;
}

#ifdef PRE_PARTY_DB
const std::map<int, int>& CDnPartySearchDlg::GetPartyTargetMapLevelLimitList() const
{
	if (m_pCurVillageDungeonCheckBox->IsChecked())
		return m_nMapLimitLevelList;
	else
		return CDnPartyTask::GetInstance().MakeWholePartyTargetStageLevelLimitList();
}

const std::vector<SPartyTargetMapInfo>& CDnPartySearchDlg::GetPartyTargetMapList() const
{
	if (m_pCurVillageDungeonCheckBox->IsChecked())
		return m_MapAreaNameList;
	else
		return CDnPartyTask::GetInstance().MakeWholePartyTargetStageList();
}
#endif