#include "StdAfx.h"
#include "DnCharStatusLadderInfoDlg.h"
#include "DnPlayerActor.h"
#include "DnInterfaceString.h"
#include "DnTableDB.h"
#include "DnPartyTask.h"
#include "DnPVPLadderSystemDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCharStatusLadderInfoDlg::CDnCharStatusLadderInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	m_pLadderPoint = NULL;
	m_pExpectingLadderPoint = NULL;
	m_vecLadderScoreInfo.clear();
}

CDnCharStatusLadderInfoDlg::~CDnCharStatusLadderInfoDlg(void)
{
}

void CDnCharStatusLadderInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharStatusPvpLadderInfoDlg.ui" ).c_str(), bShow );
}

void CDnCharStatusLadderInfoDlg::Show( bool bShow )
{
	if(bShow)
		RefreshLadderInfo();

	CEtUIDialog::Show( bShow );
}


void CDnCharStatusLadderInfoDlg::InitialUpdate()
{
	m_pLadderPoint =          GetControl<CEtUIStatic>("ID_TEXT_GUTSPOINT");
	m_pExpectingLadderPoint = GetControl<CEtUIStatic>("ID_TEXT_NOWGUTSPOINT");

	InitializeLadderScoreInfo();
#ifdef PRE_FIX_TEAM_LADDER_3vs3 // Replace 2:2 text with 3:3.. Hardcoded, but unavoidable.
	CEtUIStatic *pStaticCover = GetControl<CEtUIStatic>("ID_STATIC5");
	if(pStaticCover) pStaticCover->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126156 ));
	pStaticCover = GetControl<CEtUIStatic>("ID_STATIC6");
	if(pStaticCover) pStaticCover->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126160 ));
	pStaticCover = GetControl<CEtUIStatic>("ID_STATIC7");
	if(pStaticCover) pStaticCover->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126164 ));
#endif
}

void CDnCharStatusLadderInfoDlg::InitializeLadderScoreInfo()
{
	char szUIName[256] = "";

	for(int i =0; i<LadderSystem::MatchType::MaxCount; i++)
	{
		sLadderScoreInfo_UI TempInfo;
		memset(&TempInfo,0,sizeof(TempInfo));

		sprintf(szUIName,"ID_TEXT_LADDERPOINT%d",i);
		TempInfo.pLadderGrade = GetControl<CEtUIStatic>(szUIName);
		sprintf(szUIName,"ID_TEXT_LADDERCOUNT%d",i);
		TempInfo.pLadderPlayCount = GetControl<CEtUIStatic>(szUIName);
		sprintf(szUIName,"ID_TEXT_LADDERWIN%d",i);
		TempInfo.pLadderStatus = GetControl<CEtUIStatic>(szUIName);

		m_vecLadderScoreInfo.push_back(TempInfo);

		for(int k=0;k<4;k++)
		{
			sprintf(szUIName,"ID_STATIC%d",(i*4)+k);
			CEtUIStatic *pStaticCover = GetControl<CEtUIStatic>(szUIName);
			pStaticCover->Show(false);
		}
	}
}

void CDnCharStatusLadderInfoDlg::RefreshLadderInfo()
{
	if(CDnPartyTask::IsActive())
		SetPvpLadderInfo(CDnPartyTask::GetInstance().GetPVPLadderInfo());
}

void CDnCharStatusLadderInfoDlg::SetPvpLadderInfo(LadderSystem::SC_SCOREINFO* pInfo)
{

#ifdef PRE_ADD_LIMIT_LADDERPOINT
	m_pLadderPoint->SetText( FormatW( L"%d / %d" ,  pInfo->Data.iPvPLadderPoint , (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PvPLadder_LimitPoint ) ) );
#else
	m_pLadderPoint->SetIntToText(pInfo->Data.iPvPLadderPoint);
#endif

	TPvPLadderMatchTypeScore LadderScoreInfo[LadderSystem::MatchType::MaxCount];
	memset(&LadderScoreInfo,0,sizeof(LadderScoreInfo));
	for(int i=0;i<LadderSystem::MatchType::MaxCount;i++)
	{
		LadderScoreInfo[i].iPvPLadderGradePoint = LadderSystem::Stats::InitGradePoint; // Ladder initial value. Set to initial value if no info exists.

		m_vecLadderScoreInfo[i].pLadderGrade->Show(false);
		m_vecLadderScoreInfo[i].pLadderPlayCount->Show(false);
		m_vecLadderScoreInfo[i].pLadderStatus->Show(false);
	}

	for(int i=0; i< pInfo->Data.cLadderTypeCount; i++)
	{
		if(pInfo->Data.cLadderTypeCount >= LadderSystem::MatchType::_1vs1 && pInfo->Data.cLadderTypeCount <= LadderSystem::MatchType::MaxCount )
		{
			if(pInfo->Data.LadderScore[i].cPvPLadderCode >= LadderSystem::MatchType::_1vs1 && pInfo->Data.LadderScore[i].cPvPLadderCode <= LadderSystem::MatchType::MaxCount)
				LadderScoreInfo[pInfo->Data.LadderScore[i].cPvPLadderCode-1] = pInfo->Data.LadderScore[i];
		}
	}

	int nEchangePoint = 0;
	bool bCanReceiveLadderPoint = false;

	for(int i=0; i<LadderSystem::MatchType::MaxCount; i++)
	{
#ifdef PRE_FIX_TEAM_LADDER_3vs3 // When 2:2 is not open and 3:3 goes live, swap out 2:2 and hide all 3/3 slots.
		if( i+1 == LadderSystem::MatchType::_3vs3)
		{
			m_vecLadderScoreInfo[i-1].pLadderGrade->SetIntToText(LadderScoreInfo[i].iPvPLadderGradePoint ? LadderScoreInfo[i].iPvPLadderGradePoint : LadderSystem::Stats::InitGradePoint);

			WCHAR wszStr[128];
			wsprintf( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126039 ),LadderScoreInfo[i].iWin , LadderScoreInfo[i].iLose , LadderScoreInfo[i].iDraw );
			m_vecLadderScoreInfo[i-1].pLadderStatus->SetText(wszStr);
			m_vecLadderScoreInfo[i-1].pLadderPlayCount->SetIntToText(LadderScoreInfo[i].nWeeklyCount); // Weekly participation count.

			m_vecLadderScoreInfo[i-1].pLadderGrade->Show(true);
			m_vecLadderScoreInfo[i-1].pLadderPlayCount->Show(true);
			m_vecLadderScoreInfo[i-1].pLadderStatus->Show(true);

		}
		else
		{
			m_vecLadderScoreInfo[i].pLadderGrade->SetIntToText(LadderScoreInfo[i].iPvPLadderGradePoint ? LadderScoreInfo[i].iPvPLadderGradePoint : LadderSystem::Stats::InitGradePoint);

			WCHAR wszStr[128];
			wsprintf( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126039 ),LadderScoreInfo[i].iWin , LadderScoreInfo[i].iLose , LadderScoreInfo[i].iDraw );
			m_vecLadderScoreInfo[i].pLadderStatus->SetText(wszStr);
			m_vecLadderScoreInfo[i].pLadderPlayCount->SetIntToText(LadderScoreInfo[i].nWeeklyCount); // Weekly participation count.
			m_vecLadderScoreInfo[i].pLadderGrade->Show(true);
			m_vecLadderScoreInfo[i].pLadderPlayCount->Show(true);
			m_vecLadderScoreInfo[i].pLadderStatus->Show(true);
		}
#else
		m_vecLadderScoreInfo[i].pLadderGrade->SetIntToText(LadderScoreInfo[i].iPvPLadderGradePoint ? LadderScoreInfo[i].iPvPLadderGradePoint : LadderSystem::Stats::InitGradePoint);
		
		WCHAR wszStr[128];
		wsprintf( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126039 ),LadderScoreInfo[i].iWin , LadderScoreInfo[i].iLose , LadderScoreInfo[i].iDraw );
		m_vecLadderScoreInfo[i].pLadderStatus->SetText(wszStr);
		m_vecLadderScoreInfo[i].pLadderPlayCount->SetIntToText(LadderScoreInfo[i].nWeeklyCount); // Weekly participation count.
		m_vecLadderScoreInfo[i].pLadderGrade->Show(true);
		m_vecLadderScoreInfo[i].pLadderPlayCount->Show(true);
		m_vecLadderScoreInfo[i].pLadderStatus->Show(true);
#endif


		if(nEchangePoint < LadderScoreInfo[i].GetExchangeLadderPoint() && LadderScoreInfo[i].nWeeklyCount >= LadderSystem::Exchange::MinGamePlay)
		{
			nEchangePoint = LadderScoreInfo[i].GetExchangeLadderPoint();
			bCanReceiveLadderPoint = true;
		}

		if(LadderScoreInfo[i].nWeeklyCount < LadderSystem::Exchange::MinGamePlay)
			m_vecLadderScoreInfo[i].pLadderPlayCount->SetTextColor(textcolor::RED);
		else
			m_vecLadderScoreInfo[i].pLadderPlayCount->SetTextColor(textcolor::GREENYELLOW);

		for(int k=0;k<4;k++)
		{
			char szUIName[256] = "";
			sprintf(szUIName,"ID_STATIC%d",(i*4)+k);
			CEtUIStatic *pStaticCover = GetControl<CEtUIStatic>(szUIName);
			pStaticCover->Show(true);
#ifdef PRE_FIX_TEAM_LADDER_3vs3
			if( i+1 == LadderSystem::MatchType::_3vs3)
				pStaticCover->Show(false);
#endif
		}
	}

	if(bCanReceiveLadderPoint)
		m_pExpectingLadderPoint->SetIntToText(nEchangePoint); // Estimated points
	else
		m_pExpectingLadderPoint->SetIntToText(0); // Estimated points
}
