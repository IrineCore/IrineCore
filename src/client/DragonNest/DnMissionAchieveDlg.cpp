#include "StdAfx.h"
#include "DnMissionAchieveDlg.h"
#include "DnMissionTask.h"
#include "DnMissionListItem.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL

#include "DnMissionAchieveCompletListBoxDlg.h"
#include "DnMissionAppellationListBoxDlg.h"
#include "DnMissionAchieveEventListBoxDlg.h"
#include "DnInterface.h"

CDnMissionAchieveDlg::CDnMissionAchieveDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CEtUIDialog(dialogType, pParentDialog, nID, pCallback, true)
	, m_pTotalPoint(NULL)
	, m_pTotalPointProgress(NULL)
	, m_pTotalAchieveCount(NULL)
	, m_pDailyQuestCount(NULL)
	, m_pDailyQuestPoint(NULL)
	, m_pDailyQuestPointProgress(NULL)
	, m_pCompleteListBox(NULL)
	, m_pCompleteTitleBtn(NULL)
	, m_pAppellationBtn(NULL)
	, m_pEventBtn(NULL)
	, m_pAppellationListBox(NULL)
{
	memset(m_pOnOffAchieveCount, 0, sizeof(m_pOnOffAchieveCount));
	memset(m_pOnOffAchievePoint, 0, sizeof(m_pOnOffAchievePoint));
	memset(m_pOnOffAchievePointProgress, 0, sizeof(m_pOnOffAchievePointProgress));
}

CDnMissionAchieveDlg::~CDnMissionAchieveDlg(void)
{
	SAFE_DELETE(m_pAppellationListBox);
	SAFE_DELETE(m_pCompleteListBox);
}

void CDnMissionAchieveDlg::InitialUpdate()
{
	m_pTotalPoint = GetControl<CEtUIStatic>("ID_MYPOINT1");
	m_pTotalPointProgress = GetControl<CEtUIProgressBar>("ID_PROGRESSBAR0");;
	m_pTotalAchieveCount = GetControl<CEtUIStatic>("ID_M_SUCCESS1");

	char szTemp[32];
	const char *szOnOffStr[] = { "GENERAL", "DUNGEON", "BATTLE", "NEST" };
	for (int i = 0; i < 4; i++) {
		sprintf_s(szTemp, "ID_%s_NUM", szOnOffStr[i]);
		m_pOnOffAchieveCount[i] = GetControl<CEtUIStatic>(szTemp);
		sprintf_s(szTemp, "ID_%s_SCORE01", szOnOffStr[i]);
		m_pOnOffAchievePoint[i] = GetControl<CEtUIStatic>(szTemp);
		sprintf_s(szTemp, "ID_%s_GAGE", szOnOffStr[i]);
		m_pOnOffAchievePointProgress[i] = GetControl<CEtUIProgressBar>(szTemp);
	}

	m_pDailyQuestCount = GetControl<CEtUIStatic>("ID_DAYLIQUEST_NUM");
	m_pDailyQuestPoint = GetControl<CEtUIStatic>("ID_DAYLIQUEST_SCORE01");
	m_pDailyQuestPointProgress = GetControl<CEtUIProgressBar>("IDDAYLIQUEST_GAGE");


	// Radio buttons	
	m_pCompleteTitleBtn = GetControl<CEtUIRadioButton>("ID_RBT_COMPLETETITLE");
	m_pCompleteTitleBtn->SetChecked(true);
	m_pAppellationBtn = GetControl<CEtUIRadioButton>("ID_RBT_APPELLATION");
	{
		bool bExistEvent = false;
		m_pEventBtn = GetControl<CEtUIRadioButton>("ID_RBT_EVENT", &bExistEvent);
		if (!bExistEvent) m_pEventBtn = NULL;
	}

	// Recently completed mission list box
	m_pCompleteListBox = new CDnMissionAchieveCompletListBoxDlg(UI_TYPE_CHILD, this);
	m_pCompleteListBox->Initialize(false);

	// Title collection list box
	m_pAppellationListBox = new CDnMissionAppellationListBoxDlg(UI_TYPE_CHILD, this);
	m_pAppellationListBox->Initialize(false);
}

void CDnMissionAchieveDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("MissionTiltleDlg.ui").c_str(), bShow);
}

void CDnMissionAchieveDlg::ResetListBoxs(bool bShow)
{
	m_pCompleteTitleBtn->SetChecked(true);

	if (bShow)
	{
		m_pCompleteListBox->Show(bShow);
		m_pAppellationListBox->Show(!bShow);
		if (GetInterface().GetMissionAchieveEventListBoxDlg())
			GetInterface().GetMissionAchieveEventListBoxDlg()->Show(false);
	}
	else
	{
		m_pCompleteListBox->Show(bShow);
		m_pAppellationListBox->Show(bShow);
		if (GetInterface().GetMissionAchieveEventListBoxDlg())
			GetInterface().GetMissionAchieveEventListBoxDlg()->Show(false);
	}
}

void CDnMissionAchieveDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	ResetListBoxs(bShow);

	if (bShow)
	{
		RefreshInfo();
	}

	CEtUIDialog::Show(bShow);
}

void CDnMissionAchieveDlg::RefreshInfo()
{
	if (!CDnMissionTask::IsActive()) return;
	CDnMissionTask *pTask = &CDnMissionTask::GetInstance();

	int nTotalScore = 0;
	int nTotalCount = 0;
	int nTotalAchieveCount = 0;
	CDnMissionTask::MissionInfoStruct *pStruct;
	int nOnOffScore[NUM_ONOFFMISSION] = { 0, };
	int nOnOffTotalCount[NUM_ONOFFMISSION] = { 0, };
	int nOnOffAchieveCount[NUM_ONOFFMISSION] = { 0, };
	for (DWORD i = 0; i<pTask->GetMissionCount(); i++) {
		pStruct = pTask->GetMissionInfo(i);
		if (!pStruct) continue;
		if (pStruct->MainCategory < 0 || pStruct->MainCategory >= NUM_ONOFFMISSION) continue;

		nTotalCount++;
		nOnOffTotalCount[pStruct->MainCategory]++;
		if (pStruct->bAchieve) {
			nTotalAchieveCount++;
			nOnOffAchieveCount[pStruct->MainCategory]++;
			nOnOffScore[pStruct->MainCategory] += pStruct->nRewardPoint;
			nTotalScore += pStruct->nRewardPoint;
		}
	}
	nOnOffTotalCount[1] += nOnOffTotalCount[4];
	nOnOffAchieveCount[1] += nOnOffAchieveCount[4];
	nOnOffScore[1] += nOnOffScore[4];
	WCHAR wszTemp[64];
	float fProgress = 0.f;
	for (int i = 0; i < 4; i++) {
		wsprintf(wszTemp, L"%d / %d", nOnOffAchieveCount[i], nOnOffTotalCount[i]);
		if (m_pOnOffAchieveCount[i]) m_pOnOffAchieveCount[i]->SetText(wszTemp);
		wsprintf(wszTemp, L"%d", nOnOffScore[i]);
		if (m_pOnOffAchievePoint[i]) m_pOnOffAchievePoint[i]->SetText(wszTemp);

		if (nOnOffTotalCount[i] == 0) fProgress = 0.f;
		else fProgress = 100.f / nOnOffTotalCount[i] * nOnOffAchieveCount[i];
		if (m_pOnOffAchievePointProgress[i]) m_pOnOffAchievePointProgress[i]->SetProgress(fProgress);
	}

	// Later, get Daily Quest info here and add it
	nTotalScore += pTask->GetQuestScore();

	int nDailyQuestScore = pTask->GetQuestScore();
	int nDailyQuestTotalCount = pTask->GetDailyMissionCount(CDnMissionTask::Daily) + pTask->GetDailyMissionCount(CDnMissionTask::Weekly) + pTask->GetDailyMissionCount(CDnMissionTask::GuildWar);
	int nDailyQuestAchieveCount = 0;
	for (int j = 0; j<2; j++) {
		for (DWORD i = 0; i<pTask->GetDailyMissionCount((CDnMissionTask::DailyMissionTypeEnum)j); i++) {
			CDnMissionTask::MissionInfoStruct *pStruct = pTask->GetDailyMissionInfo((CDnMissionTask::DailyMissionTypeEnum)j, i);
			if (pStruct->bAchieve) nDailyQuestAchieveCount++;
		}
	}
	wsprintf(wszTemp, L"%d / %d", nDailyQuestAchieveCount, nDailyQuestTotalCount);
	m_pDailyQuestCount->SetText(wszTemp);
	wsprintf(wszTemp, L"%d", pTask->GetQuestScore());
	m_pDailyQuestPoint->SetText(wszTemp);

	if (nDailyQuestTotalCount == 0) fProgress = 0.f;
	else fProgress = 100.f / nDailyQuestTotalCount * nDailyQuestAchieveCount;
	m_pDailyQuestPointProgress->SetProgress(fProgress);

	// Total
	wsprintf(wszTemp, L"%d / %d", nTotalAchieveCount, nTotalCount);
	m_pTotalAchieveCount->SetText(wszTemp);
	wsprintf(wszTemp, L"%d", nTotalScore);
	m_pTotalPoint->SetText(wszTemp);
	if (nTotalCount == 0) fProgress = 0.f;
	else fProgress = 100.f / nTotalCount * nTotalAchieveCount;
	m_pTotalPointProgress->SetProgress(fProgress);
}

void CDnMissionAchieveDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_RADIOBUTTON_CHANGED)
	{
		if (strstr(pControl->GetControlName(), "ID_RBT_COMPLETETITLE") && bTriggeredByUser)
		{
			if (m_pCompleteListBox)
				m_pCompleteListBox->Show(true);

			if (m_pAppellationListBox)
				m_pAppellationListBox->Show(false);

			if (GetInterface().GetMissionAchieveEventListBoxDlg())
				GetInterface().GetMissionAchieveEventListBoxDlg()->Show(false);
		}
		else if (strstr(pControl->GetControlName(), "ID_RBT_APPELLATION") && bTriggeredByUser)
		{
			if (m_pCompleteListBox)
				m_pCompleteListBox->Show(false);

			if (m_pAppellationListBox)
				m_pAppellationListBox->Show(true);

			if (GetInterface().GetMissionAchieveEventListBoxDlg())
				GetInterface().GetMissionAchieveEventListBoxDlg()->Show(false);
		}
		else if (strstr(pControl->GetControlName(), "ID_RBT_EVENT") && bTriggeredByUser)
		{
			if (m_pCompleteListBox)
				m_pCompleteListBox->Show(false);

			if (m_pAppellationListBox)
				m_pAppellationListBox->Show(false);

			if (GetInterface().GetMissionAchieveEventListBoxDlg())
			{
				GetInterface().GetMissionAchieveEventListBoxDlg()->Show(true);
			}
		}
	}

	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

#else
CDnMissionAchieveDlg::CDnMissionAchieveDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CEtUIDialog(dialogType, pParentDialog, nID, pCallback, true)
	, m_pTotalPoint(NULL)
	, m_pTotalPointProgress(NULL)
	, m_pTotalAchieveCount(NULL)
	, m_pDailyQuestCount(NULL)
	, m_pDailyQuestPoint(NULL)
	, m_pDailyQuestPointProgress(NULL)
	, m_pListBoxEx(NULL)
{
	memset(m_pOnOffAchieveCount, 0, sizeof(m_pOnOffAchieveCount));
	memset(m_pOnOffAchievePoint, 0, sizeof(m_pOnOffAchievePoint));
	memset(m_pOnOffAchievePointProgress, 0, sizeof(m_pOnOffAchievePointProgress));
}

CDnMissionAchieveDlg::~CDnMissionAchieveDlg(void)
{
	m_pListBoxEx->RemoveAllItems();
}

void CDnMissionAchieveDlg::InitialUpdate()
{
	m_pTotalPoint = GetControl<CEtUIStatic>("ID_MYPOINT1");
	m_pTotalPointProgress = GetControl<CEtUIProgressBar>("ID_PROGRESSBAR0");;
	m_pTotalAchieveCount = GetControl<CEtUIStatic>("ID_M_SUCCESS1");

	char szTemp[32];
	const char *szOnOffStr[] = { "GENERAL", "DUNGEON", "BATTLE", "NEST" };
	for (int i = 0; i < 4; i++) {
		sprintf_s(szTemp, "ID_%s_NUM", szOnOffStr[i]);
		m_pOnOffAchieveCount[i] = GetControl<CEtUIStatic>(szTemp);
		sprintf_s(szTemp, "ID_%s_SCORE01", szOnOffStr[i]);
		m_pOnOffAchievePoint[i] = GetControl<CEtUIStatic>(szTemp);
		sprintf_s(szTemp, "ID_%s_GAGE", szOnOffStr[i]);
		m_pOnOffAchievePointProgress[i] = GetControl<CEtUIProgressBar>(szTemp);
	}

	m_pDailyQuestCount = GetControl<CEtUIStatic>("ID_DAYLIQUEST_NUM");
	m_pDailyQuestPoint = GetControl<CEtUIStatic>("ID_DAYLIQUEST_SCORE01");
	m_pDailyQuestPointProgress = GetControl<CEtUIProgressBar>("IDDAYLIQUEST_GAGE");

	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX0");
	m_pListBoxEx->SetRenderSelectBar(false);
	m_pListBoxEx->SetRenderScrollBar(false);
}

void CDnMissionAchieveDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("MissionTiltleDlg.ui").c_str(), bShow);
}

void CDnMissionAchieveDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	if (bShow)
	{
		RefreshInfo();
	}
	else
	{
		m_pListBoxEx->RemoveAllItems();
	}

	CEtUIDialog::Show(bShow);
}

void CDnMissionAchieveDlg::RefreshInfo()
{
	if (!CDnMissionTask::IsActive()) return;
	CDnMissionTask *pTask = &CDnMissionTask::GetInstance();

	int nTotalScore = 0;
	int nTotalCount = 0;
	int nTotalAchieveCount = 0;
	CDnMissionTask::MissionInfoStruct *pStruct;
	int nOnOffScore[NUM_ONOFFMISSION] = { 0, };
	int nOnOffTotalCount[NUM_ONOFFMISSION] = { 0, };
	int nOnOffAchieveCount[NUM_ONOFFMISSION] = { 0, };
	for (DWORD i = 0; i<pTask->GetMissionCount(); i++) {
		pStruct = pTask->GetMissionInfo(i);
		if (!pStruct) continue;
		if (pStruct->MainCategory < 0 || pStruct->MainCategory >= NUM_ONOFFMISSION) continue;

		nTotalCount++;
		nOnOffTotalCount[pStruct->MainCategory]++;
		if (pStruct->bAchieve) {
			nTotalAchieveCount++;
			nOnOffAchieveCount[pStruct->MainCategory]++;
			nOnOffScore[pStruct->MainCategory] += pStruct->nRewardPoint;
			nTotalScore += pStruct->nRewardPoint;
		}
	}
	nOnOffTotalCount[1] += nOnOffTotalCount[4];
	nOnOffAchieveCount[1] += nOnOffAchieveCount[4];
	nOnOffScore[1] += nOnOffScore[4];
	WCHAR wszTemp[64];
	float fProgress = 0.f;
	for (int i = 0; i < 4; i++) {
		wsprintf(wszTemp, L"%d / %d", nOnOffAchieveCount[i], nOnOffTotalCount[i]);
		if (m_pOnOffAchieveCount[i]) m_pOnOffAchieveCount[i]->SetText(wszTemp);
		wsprintf(wszTemp, L"%d", nOnOffScore[i]);
		if (m_pOnOffAchievePoint[i]) m_pOnOffAchievePoint[i]->SetText(wszTemp);

		if (nOnOffTotalCount[i] == 0) fProgress = 0.f;
		else fProgress = 100.f / nOnOffTotalCount[i] * nOnOffAchieveCount[i];
		if (m_pOnOffAchievePointProgress[i]) m_pOnOffAchievePointProgress[i]->SetProgress(fProgress);
	}

	// Later, get Daily Quest info here and add it
	nTotalScore += pTask->GetQuestScore();

	int nDailyQuestScore = pTask->GetQuestScore();
	int nDailyQuestTotalCount = pTask->GetDailyMissionCount(CDnMissionTask::Daily) + pTask->GetDailyMissionCount(CDnMissionTask::Weekly) + pTask->GetDailyMissionCount(CDnMissionTask::GuildWar);
	int nDailyQuestAchieveCount = 0;
	for (int j = 0; j<2; j++) {
		for (DWORD i = 0; i<pTask->GetDailyMissionCount((CDnMissionTask::DailyMissionTypeEnum)j); i++) {
			CDnMissionTask::MissionInfoStruct *pStruct = pTask->GetDailyMissionInfo((CDnMissionTask::DailyMissionTypeEnum)j, i);
			if (pStruct->bAchieve) nDailyQuestAchieveCount++;
		}
	}
	wsprintf(wszTemp, L"%d / %d", nDailyQuestAchieveCount, nDailyQuestTotalCount);
	m_pDailyQuestCount->SetText(wszTemp);
	wsprintf(wszTemp, L"%d", pTask->GetQuestScore());
	m_pDailyQuestPoint->SetText(wszTemp);

	if (nDailyQuestTotalCount == 0) fProgress = 0.f;
	else fProgress = 100.f / nDailyQuestTotalCount * nDailyQuestAchieveCount;
	m_pDailyQuestPointProgress->SetProgress(fProgress);

	// Total
	wsprintf(wszTemp, L"%d / %d", nTotalAchieveCount, nTotalCount);
	m_pTotalAchieveCount->SetText(wszTemp);
	wsprintf(wszTemp, L"%d", nTotalScore);
	m_pTotalPoint->SetText(wszTemp);
	if (nTotalCount == 0) fProgress = 0.f;
	else fProgress = 100.f / nTotalCount * nTotalAchieveCount;
	m_pTotalPointProgress->SetProgress(fProgress);

	// Update mission list
	m_pListBoxEx->RemoveAllItems();
	for (int i = 0; i < LASTMISSIONACHIEVEMAX; i++) {
		int nArrayIndex = pTask->GetLastUpdateAchieveIndex(i);
		if (nArrayIndex == -1) continue;

		pStruct = pTask->GetMissionFromArrayIndex(nArrayIndex);
		if (!pStruct) continue;
		CDnMissionListItem *pItem = m_pListBoxEx->AddItem<CDnMissionListItem>();
		pItem->SetInfo(pStruct);
#ifdef PRE_MOD_MISSION_HELPER
		pItem->ShowCheckBox(false);
#endif
	}
}

void CDnMissionAchieveDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

#endif
