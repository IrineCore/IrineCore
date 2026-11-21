#include "StdAfx.h"
#include "DnQuestSummaryInfoDlg.h"
#include "DnQuestTask.h"
#include "DnMissionTask.h"
#include "DnWorld.h"
#include "DnNPCActor.h"
#include "DnWorldData.h"
#include "DnWorldDataManager.h"
#include "DnMainMenuDlg.h"
#include "DnQuestTabDlg.h"
#include "DnQuestDlg.h"
#include "DnMissionDlg.h"
#include "DnWorldMapDlg.h"
#include "DnTableDB.h"
#include "DnUIString.h"
#include "DnInterface.h"
#include "DnLocalPlayerActor.h"
#include "EtUIListBoxEx.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnNotifierControls::CDnNotifierControls()
{
	m_pIcon = NULL;
	m_pTitle = NULL;
	m_Board = NULL;
	m_OverBoard = NULL;
	m_pTextBox = NULL;
	m_pButtonClose = NULL;
	m_pDialog = NULL;

	m_PosY = 0.0f;
	m_fPosY = 0.0f;
	m_fSmoothY = 0.0f;
	m_fDistance = 0.0f;
	m_nNotifierIndex = 0;

	m_bShow = false;
	m_bMouseOver = false;
	m_bClicked = false;
}

void CDnNotifierControls::Show( bool bShow )
{
	m_bShow = bShow;
	if( !bShow ) {
		if( m_Board ) m_Board->Show( false );
		if( m_OverBoard ) m_OverBoard->Show( false );
	}
	if( m_pTitle ) m_pTitle->Show( bShow );
	if( m_pTextBox ) m_pTextBox->Show( bShow );
	if( m_pIcon ) m_pIcon->Show( bShow );
	if( m_pButtonClose ) m_pButtonClose->Show( bShow );
}

void CDnNotifierControls::SetPosY( float fY )
{
	float fOffsetX = 0.0f;
	float fOffsetY = 0.0f;
	if( m_bClicked && m_bMouseOver ) {
		fOffsetX = 1.0f / (float)DEFAULT_UI_SCREEN_WIDTH;
		fOffsetY = 1.0f / (float)DEFAULT_UI_SCREEN_HEIGHT;
	}

	m_PosY = fY;
	float fDeltaY = ( fY - (float)m_nNotifierIndex ) * m_fDistance;

	if( m_Board ) {
		m_Board->SetPosition( m_BoardCoord.fX + fOffsetX, m_BoardCoord.fY + fOffsetY + fDeltaY );
	}
	if( m_OverBoard ) {
		m_OverBoard->SetPosition( m_BoardCoord.fX + fOffsetX, m_BoardCoord.fY + fOffsetY + fDeltaY );
	}
	if( m_pIcon ) {
		m_pIcon->SetPosition( m_IconCoord.fX + fOffsetX, m_IconCoord.fY + fOffsetY + fDeltaY );
	}
	if( m_pTitle ) {
		m_pTitle->SetPosition( m_TitleCoord.fX + fOffsetX, m_TitleCoord.fY + fOffsetY + fDeltaY );
	}
	if( m_pTextBox ) {
		m_pTextBox->SetPosition( m_TextBoxCoord.fX + fOffsetX, m_TextBoxCoord.fY + fOffsetY + fDeltaY );
	}
	if( m_pButtonClose ) {
		m_pButtonClose->SetPosition( m_CloseCoord.fX, m_CloseCoord.fY + fOffsetY + fDeltaY );
	}
}

void CDnNotifierControls::Process( float fElapsedTime )
{
	const float fConst = 1.5f;
	float fDiff = m_fPosY - m_fSmoothY;
	if( fDiff > fConst ) {
		m_fSmoothY += ( fDiff - fConst );
		fDiff = fConst;
	}
	else if( fDiff < -fConst ) {
		m_fSmoothY += ( fDiff + fConst );
		fDiff = -fConst;
	}
	float fLerp = fElapsedTime * 10.0f;
	if( fLerp > 1.0f ) fLerp = 1.0f;
	m_fSmoothY += fDiff * fLerp;
	SetPosY( m_fSmoothY );
}

void CDnNotifierControls::SetNotifier( int nNotifier )
{
	m_nNotifierIndex = nNotifier;
	if( m_pIcon ) m_pIcon->GetUICoord( m_IconCoord );
	if( m_Board ) m_Board->GetUICoord( m_BoardCoord );
	if( m_pTitle ) m_pTitle->GetUICoord( m_TitleCoord );
	if( m_pTextBox ) m_pTextBox->GetUICoord( m_TextBoxCoord );
	if( m_pButtonClose ) m_pButtonClose->GetUICoord( m_CloseCoord );

	m_SelectCoord = m_BoardCoord;
	m_SelectCoord.fHeight *= 0.8f;
}

void CDnNotifierControls::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !m_pDialog ) return;
	POINT MousePoint;
	float fMouseX, fMouseY;
	MousePoint.x = short( LOWORD( lParam ) );
	MousePoint.y = short( HIWORD( lParam ) );
	m_pDialog->PointToFloat( MousePoint, fMouseX, fMouseY );

	SUICoord selectCoord( m_SelectCoord );
	selectCoord.fY += ( m_PosY - (float)m_nNotifierIndex ) * m_fDistance;

	if( selectCoord.IsInside( fMouseX, fMouseY ) && m_bShow ) {
		m_bMouseOver = true;
	}
	else {
		m_bMouseOver = false;
	}
}

void CDnNotifierControls::Refresh( const wchar_t *szTitle, const wchar_t *szDest, std::vector< boost::tuple<std::wstring, int, int> > &vecGetList, std::wstring szProgress )
{
	if( !m_pTitle || !m_pTextBox || !m_pIcon ) return;

	m_pTitle->ClearText();
	m_pTitle->ClearTooltipText();
	m_pTextBox->ClearText();
	m_pTextBox->ClearTooltipText();

	WCHAR szTemp[256] = { 0, };
	swprintf_s( szTemp, _countof(szTemp), L"%s", szTitle );
	m_pTitle->SetTextWithEllipsis( szTemp, L"..." );

	if( vecGetList.size() == 1 ) {
		int nCnt = vecGetList[0].get<1>();
		int nTotalCnt = vecGetList[0].get<2>();
		if( nCnt < 0 && nTotalCnt < 0 ) {
			swprintf_s( szTemp, _countof(szTemp), L"%s", vecGetList[0].get<0>().c_str() );
		}
		else if( nTotalCnt < 0 ) {
			swprintf_s( szTemp, _countof(szTemp), L"- %s [%d]", vecGetList[0].get<0>().c_str(), vecGetList[0].get<1>() );
		}
		else {
			int WORD_MAXSIZE = 12;
			if( (int)vecGetList[0].get<0>().size() > WORD_MAXSIZE ) {
				std::wstring wszTemp = vecGetList[0].get<0>().substr( 0, WORD_MAXSIZE );
				swprintf_s( szTemp, _countof(szTemp), L"- %s.. [%d/%d]", wszTemp.c_str(), vecGetList[0].get<1>(), vecGetList[0].get<2>() );
			}
			else {
				swprintf_s( szTemp, _countof(szTemp), L"- %s [%d/%d]", vecGetList[0].get<0>().c_str(), vecGetList[0].get<1>(), vecGetList[0].get<2>() );
			}
		}
		m_pTextBox->AddText( szTemp );
	}
	else if( !vecGetList.empty() ) {
		int nItemCnt = 0;
		int nItemTotalCnt = 0;
		std::wstring szGetString;
		int nMaxLen = 0;
		for each( boost::tuple<std::wstring, int, int> e in vecGetList ) {
			if( e.get<1>() < 0 ) continue;
			int nTotalCount = e.get<2>();
			nItemCnt += e.get<1>();
			nItemTotalCnt += nTotalCount < 0 ? 1 : nTotalCount;
			if( nTotalCount < 0 ) {
				std::wstring szAdd = FormatW( L"  %s [%d]    \n", e.get<0>().c_str(), e.get<1>() );
				nMaxLen = EtMax( nMaxLen, (int)szAdd.length() );
				szGetString += szAdd;
			}
			else {
				std::wstring szAdd = FormatW( L"  %s [%d/%d]    \n", e.get<0>().c_str(), e.get<1>(), e.get<2>() );
				nMaxLen = EtMax( nMaxLen, (int)szAdd.length() );
				szGetString += szAdd;
			}
		}

		for( int i = 0; i < 5 * nMaxLen / 2; i++ )
			szGetString += L" ";
		szGetString += L"\n";
		if( nItemTotalCnt != 0 && nItemCnt <= nItemTotalCnt ) {
			m_pTextBox->AddText( FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 422 ), 100 * nItemCnt / nItemTotalCnt ).c_str(), textcolor::YELLOW );
			m_pTextBox->SetTooltipText( szGetString.c_str() );
		}
	}

	m_pTextBox->AppendText( szProgress.c_str(), textcolor::WHITE );

	std::wstring wszTooltip;
	wszTooltip += szTitle;
	if( !szProgress.empty() ) {
		wszTooltip += L"     ";
		wszTooltip += szProgress;
	}
	wszTooltip += L"\n\n";
	wszTooltip += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7877 );
	wszTooltip += L" ";
	wszTooltip += szDest;
	if( vecGetList.size() == 2 ) {
		wszTooltip += L"\n";
		wszTooltip += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7878 );
		wszTooltip += L" ";
		wszTooltip += vecGetList[0].get<0>();
		wszTooltip += L"\n";
		wszTooltip += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7879 );
		wszTooltip += L" ";
		wszTooltip += vecGetList[1].get<0>();
	}
	m_pTitle->SetTooltipText( wszTooltip.c_str() );
	m_pIcon->Show( true );
}

EtVector2 CDnNotifierControls::CalcArrowDestination( int nQuestIndex, int nJurnalStep )
{
	int nMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;

	if( nQuestIndex > 0 && nJurnalStep > 0 ) {
		Journal *pJournal = g_DataManager.GetJournalData( nQuestIndex );
		if( !pJournal ) return EtVector2( 0, 0 );

		JournalPage *pJournalPage = pJournal->FindJournalPage( nJurnalStep );
		if( !pJournalPage ) return EtVector2( 0, 0 );

		EtVector2 vTargetPos( 0, 0 );
		int nTargetMapIndex = pJournalPage->nDestnationMapIndex;
		int nCurrentMapIndex = nMapIndex;
		const float fDummyPos = -100000.0f;

		if( pJournalPage->nDestinationNpc == -1 ) {
			vTargetPos = pJournalPage->vDestinationPos;
		}
		else {
			vTargetPos = EtVector2( fDummyPos, fDummyPos );
			CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( nMapIndex );
			if( pWorldData ) {
				int nCount = pWorldData->GetNpcCount();
				for( int k = 0; k < nCount; k++ ) {
					CDnWorldData::NpcData *pNpcData = pWorldData->GetNpcData( k );
					if( pNpcData && pNpcData->nNpcID == pJournalPage->nDestinationNpc ) {
						vTargetPos = pNpcData->vPos;
						break;
					}
				}
			}

			if( vTargetPos.x <= fDummyPos && nMapIndex == pJournalPage->nDestnationMapIndex ) {
				CDnNPCActor *pNpcActor = CDnNPCActor::FindNpcActorFromID( pJournalPage->nDestinationNpc );
				if( pNpcActor ) {
					vTargetPos = EtVector2( pNpcActor->GetPosition()->x, pNpcActor->GetPosition()->z );
				}
			}
		}

		if( nCurrentMapIndex != nTargetMapIndex ) {
			std::deque<PathInfo> pathResult;
			GetQuestTask().GetPathFinder()->GetPathOneWay( nCurrentMapIndex, nTargetMapIndex, pathResult );
			if( !pathResult.empty() ) {
				int nGateIndex = pathResult[0].nGateIndex;
				vTargetPos.x = vTargetPos.y = fDummyPos;
				if( nGateIndex != -1 ) {
					CDnWorld::GateStruct *pStruct = CDnWorld::GetInstance().GetGateStruct( nGateIndex );
					if( pStruct ) {
						CEtWorldEventArea *pEventArea = pStruct->pGateArea;
						if( pEventArea ) {
							EtVector3 vGatePos = ( *pEventArea->GetMin() + *pEventArea->GetMax() ) * 0.5f;
							vTargetPos.x = vGatePos.x;
							vTargetPos.y = vGatePos.z;
						}
					}
				}
			}
		}

		return vTargetPos;
	}

	return EtVector2( 0, 0 );
}

int CDnNotifierControls::GetNotifyArrowState( int nQuestID, int nJurnalStep )
{
	int nArrowState = ArrowType::Hide;
	int nMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;

	Journal *pJournal = g_DataManager.GetJournalData( nQuestID );
	if( !pJournal ) return ArrowType::Hide;

	JournalPage *pJournalPage = pJournal->FindJournalPage( nJurnalStep );
	if( !pJournalPage ) return ArrowType::Hide;

	switch( pJournalPage->nDestnationMode )
	{
	case JournalPage::none:
		nArrowState = ArrowType::NormalArrow;
		break;
	case JournalPage::hide:
		nArrowState = ArrowType::Hide;
		break;
	case JournalPage::arrival:
		nArrowState = ArrowType::ArrivalMark;
		break;
	case JournalPage::stage_hide:
		if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeDungeon )
			nArrowState = ArrowType::Hide;
		else
			nArrowState = ArrowType::NormalArrow;
		break;
	case JournalPage::stage_arrival:
		if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeDungeon )
			nArrowState = ArrowType::ArrivalMark;
		else
			nArrowState = ArrowType::NormalArrow;
		break;
	case JournalPage::select_hide:
		if( std::find( pJournalPage->DestnationModeValue.begin(), pJournalPage->DestnationModeValue.end(), nMapIndex ) != pJournalPage->DestnationModeValue.end() )
			nArrowState = ArrowType::Hide;
		else
			nArrowState = ArrowType::NormalArrow;
		break;
	case JournalPage::select_arrival:
		if( std::find( pJournalPage->DestnationModeValue.begin(), pJournalPage->DestnationModeValue.end(), nMapIndex ) != pJournalPage->DestnationModeValue.end() )
			nArrowState = ArrowType::ArrivalMark;
		else
			nArrowState = ArrowType::Hide;
		break;
	case JournalPage::stage_hide_except:
		if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeDungeon &&
			std::find( pJournalPage->DestnationModeValue.begin(), pJournalPage->DestnationModeValue.end(), nMapIndex ) == pJournalPage->DestnationModeValue.end() )
			nArrowState = ArrowType::Hide;
		else
			nArrowState = ArrowType::NormalArrow;
		break;
	case JournalPage::custom:
		{
			if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeDungeon )
				nArrowState = ArrowType::Hide;
			else
				nArrowState = ArrowType::NormalArrow;

			for( size_t i = 0; i < pJournalPage->CustomDestnationMark.size(); i++ ) {
				if( pJournalPage->CustomDestnationMark[i].nMapIdx == nMapIndex ) {
					if( pJournalPage->CustomDestnationMark[i].nMarkIdx >= ArrowType::Hide && pJournalPage->CustomDestnationMark[i].nMarkIdx <= ArrowType::FindMark ) {
						nArrowState = pJournalPage->CustomDestnationMark[i].nMarkIdx;
					}
				}
			}

			nArrowState = CheckNotifyArrowStateByMap( nArrowState, pJournalPage->nDestnationMapIndex );
		}
		break;
	}

	return nArrowState;
}

int CDnNotifierControls::CheckNotifyArrowStateByMap( int nArrowState, int nDestinationMapIndex )
{
	int nMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
	DNTableFileFormat *pMapTable = GetDNTable( CDnTableDB::TMAP );

	if( pMapTable ) {
		if( CDnWorld::GetInstance().GetMapSubType() == CDnWorld::MapSubTypeNest ) {
			if( pMapTable->IsExistItem( nDestinationMapIndex ) &&
				pMapTable->GetFieldFromLablePtr( nDestinationMapIndex, "_MapSubType" )->GetInteger() == CDnWorld::MapSubTypeNest )
				return nArrowState;
			return ArrowType::Hide;
		}

		bool bCurrentMapDarkLair = false;
		bool bDestMapDarkLair = false;

		if( pMapTable->IsExistItem( nMapIndex ) &&
			pMapTable->GetFieldFromLablePtr( nMapIndex, "_MapSubType" )->GetInteger() == CDnWorld::MapSubTypeDarkLair )
			bCurrentMapDarkLair = true;

		if( pMapTable->IsExistItem( nDestinationMapIndex ) ) {
			int nSubType = pMapTable->GetFieldFromLablePtr( nDestinationMapIndex, "_MapSubType" )->GetInteger();
			if( nSubType == CDnWorld::MapSubTypeDarkLair || nSubType == CDnWorld::MapSubTypeEvent )
				bDestMapDarkLair = true;
		}

		if( bCurrentMapDarkLair != bDestMapDarkLair && nArrowState == ArrowType::NormalArrow )
			nArrowState = ArrowType::Hide;
	}

	return nArrowState;
}

// ============================================================================
// CDnQuestSummaryInfoListDlg
// ============================================================================

CDnQuestSummaryInfoListDlg::CDnQuestSummaryInfoListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	m_pIcon[0] = NULL;
	m_pIcon[1] = NULL;
	m_pTitle[0] = NULL;
	m_pTitle[1] = NULL;
	m_pArrowBase = NULL;
	for( int i = 0; i < Style_Max; ++i ) {
		m_pDistance[i] = NULL;
		m_pArrow[i] = NULL;
		m_pArrowArrival[i] = NULL;
		m_pArrowWrong[i] = NULL;
		m_pArrowBattle[i] = NULL;
		m_pArrowFind[i] = NULL;
		m_pArrowArrivalDistance[i] = NULL;
	}
	m_Board = NULL;
	m_OverBoard = NULL;
	m_pTextBox = NULL;
	m_pButtonClose = NULL;
	m_pParentSummaryDlg = NULL;
	m_nQuestIconIndex = 0;
	m_nNotifierIndex = 0;
	m_bTraceQuest = true;
	m_Quest = NULL;
	m_pJournal = NULL;
	m_bMouseOver = false;
	m_nArrowState = ArrowType::Hide;
	m_nQuestID = 0;
	m_bNowTraceQuest = false;
}

CDnQuestSummaryInfoListDlg::~CDnQuestSummaryInfoListDlg()
{
}

void CDnQuestSummaryInfoListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "QuestSumInfoListDlg.ui" ).c_str(), bShow );
}

void CDnQuestSummaryInfoListDlg::InitialUpdate()
{
	m_pIcon[0] = GetControl<CEtUIStatic>( "ID_MAIN_ICON" );
	m_pIcon[1] = GetControl<CEtUIStatic>( "ID_SUB0_ICON" );
	m_pTitle[0] = GetControl<CEtUIStatic>( "ID_MAIN_TITLE" );
	m_pTitle[1] = GetControl<CEtUIStatic>( "ID_SUB0_TITLE" );
	m_pTextBox = GetControl<CEtUITextBox>( "ID_MAIN_TEXTBOX" );

	bool bExistArrowBase = false;
	m_pArrowBase = GetControl<CEtUIStatic>( "ID_MAIN_ARROWBASE", &bExistArrowBase );
	if( !bExistArrowBase ) {
		m_pArrowBase = GetControl<CEtUIStatic>( "ID_MAIN_ARROW_BASE" );
	}

	m_pButtonClose = GetControl<CEtUIButton>( "ID_BT_MAIN_CLOSE" );
	m_OverBoard = GetControl<CEtUIStatic>( "ID_STATIC_OVER0" );
	m_Board = GetControl<CEtUIStatic>( "ID_STATIC_BOARD0" );

	const char *szPrefixes[Style_Max] = { "MAIN", "SUB", "TRACK" };
	for( int i = 0; i < Style_Max; ++i ) {
		m_pDistance[i] = GetControl<CEtUIStatic>( FormatA( "ID_%s_DISTANCE", szPrefixes[i] ).c_str() );
		m_pArrow[i] = GetControl<CEtUIStatic>( FormatA( "ID_%s_ARROW", szPrefixes[i] ).c_str() );
		m_pArrowArrival[i] = GetControl<CEtUIStatic>( FormatA( "ID_%s_ARRIVAL", szPrefixes[i] ).c_str() );
		m_pArrowWrong[i] = GetControl<CEtUIStatic>( FormatA( "ID_%s_WRONG", szPrefixes[i] ).c_str() );
		m_pArrowBattle[i] = GetControl<CEtUIStatic>( FormatA( "ID_%s_BATTLE", szPrefixes[i] ).c_str() );
		m_pArrowFind[i] = GetControl<CEtUIStatic>( FormatA( "ID_%s_FIND", szPrefixes[i] ).c_str() );
		m_pArrowArrivalDistance[i] = GetControl<CEtUIStatic>( FormatA( "ID_%s_HERE", szPrefixes[i] ).c_str() );
	}
}

void CDnQuestSummaryInfoListDlg::Show( bool bShow )
{
	if( m_bShow != bShow ) {
		Clear();
		CEtUIDialog::Show( bShow );
	}
}

void CDnQuestSummaryInfoListDlg::Clear()
{
	if( m_pIcon[0] ) m_pIcon[0]->Show( false );
	if( m_pIcon[1] ) m_pIcon[1]->Show( false );
	if( m_pTitle[0] ) {
		m_pTitle[0]->ClearText();
		m_pTitle[0]->ClearTooltipText();
	}
	if( m_pTitle[1] ) {
		m_pTitle[1]->ClearText();
		m_pTitle[1]->ClearTooltipText();
	}
	if( m_pTextBox ) {
		m_pTextBox->ClearText();
		m_pTextBox->ClearTooltipText();
	}
	if( m_OverBoard ) m_OverBoard->Show( false );
	if( m_Board ) m_Board->Show( false );

	for( int i = 0; i < Style_Max; ++i ) {
		if( m_pDistance[i] ) m_pDistance[i]->Show( false );
		if( m_pArrow[i] ) m_pArrow[i]->Show( false );
		if( m_pArrowArrival[i] ) m_pArrowArrival[i]->Show( false );
		if( m_pArrowWrong[i] ) m_pArrowWrong[i]->Show( false );
		if( m_pArrowBattle[i] ) m_pArrowBattle[i]->Show( false );
		if( m_pArrowFind[i] ) m_pArrowFind[i]->Show( false );
		if( m_pArrowArrivalDistance[i] ) m_pArrowArrivalDistance[i]->Show( false );
	}
}

void CDnQuestSummaryInfoListDlg::ChangeArrowStateByPathCheck()
{
	if( m_nArrowState == ArrowType::NormalArrow && m_pathResult.empty() ) {
		m_nArrowState = ArrowType::Hide;
	}
}

void CDnQuestSummaryInfoListDlg::UpdateDestinationMode()
{
	int nQuestIndex = 0;
	int nJournalStep = 0;
	if( m_Quest == NULL ) {
		CDnQuestTask::QuestNotifierInfo info = GetQuestTask().GetNotifierInfo( m_nNotifierIndex );
		nQuestIndex = info.nQuestIndex;
		nJournalStep = info.nJournalStep;
	}
	else {
		nQuestIndex = m_Quest->nQuestID;
		nJournalStep = m_Quest->cQuestJournal;
	}
	m_nArrowState = CDnNotifierControls::GetNotifyArrowState( nQuestIndex, nJournalStep );
}

void CDnQuestSummaryInfoListDlg::Refresh( int nQuestIconIndex, int nNotifierIndex, const wchar_t *szTitle, const wchar_t *szDest, std::vector< boost::tuple<std::wstring, int, int> > &vecGetList, std::deque<PathInfo> &path )
{
	Clear();
	m_nQuestIconIndex = ( nQuestIconIndex >= 0 && nQuestIconIndex < 2 ) ? nQuestIconIndex : 1;
	m_nNotifierIndex = nNotifierIndex;

	WCHAR szTemp[256] = { 0, };
	swprintf_s( szTemp, _countof(szTemp), L"%s", szTitle );
	if( m_pTitle[m_nQuestIconIndex] ) {
		m_pTitle[m_nQuestIconIndex]->SetTextWithEllipsis( szTemp, L"..." );
	}
	if( m_pTextBox ) {
		m_pTextBox->AddText( szDest, textcolor::WHITE );
	}

	if( vecGetList.size() == 1 ) {
		int nCnt = vecGetList[0].get<1>();
		int nTotalCnt = vecGetList[0].get<2>();
		if( nCnt < 0 && nTotalCnt < 0 ) {
			swprintf_s( szTemp, _countof(szTemp), L"%s", vecGetList[0].get<0>().c_str() );
		}
		else if( nTotalCnt < 0 ) {
			swprintf_s( szTemp, _countof(szTemp), L"- %s [%d]", vecGetList[0].get<0>().c_str(), vecGetList[0].get<1>() );
		}
		else {
			int WORD_MAXSIZE = 12;
			if( (int)vecGetList[0].get<0>().size() > WORD_MAXSIZE ) {
				std::wstring wszTemp = vecGetList[0].get<0>().substr( 0, WORD_MAXSIZE );
				swprintf_s( szTemp, _countof(szTemp), L"- %s.. [%d/%d]", wszTemp.c_str(), vecGetList[0].get<1>(), vecGetList[0].get<2>() );
			}
			else {
				swprintf_s( szTemp, _countof(szTemp), L"- %s [%d/%d]", vecGetList[0].get<0>().c_str(), vecGetList[0].get<1>(), vecGetList[0].get<2>() );
			}
		}
		if( m_pTextBox ) m_pTextBox->AddText( szTemp );
	}
	else if( !vecGetList.empty() ) {
		int nItemCnt = 0;
		int nItemTotalCnt = 0;
		std::wstring szGetString;
		int nMaxLen = 0;
		for each( boost::tuple<std::wstring, int, int> e in vecGetList ) {
			if( e.get<1>() < 0 ) continue;
			int nTotalCount = e.get<2>();
			nItemCnt += e.get<1>();
			nItemTotalCnt += nTotalCount < 0 ? 1 : nTotalCount;
			if( nTotalCount < 0 ) {
				std::wstring szAdd = FormatW( L"  %s [%d]    \n", e.get<0>().c_str(), e.get<1>() );
				nMaxLen = EtMax( nMaxLen, (int)szAdd.length() );
				szGetString += szAdd;
			}
			else {
				std::wstring szAdd = FormatW( L"  %s [%d/%d]    \n", e.get<0>().c_str(), e.get<1>(), e.get<2>() );
				nMaxLen = EtMax( nMaxLen, (int)szAdd.length() );
				szGetString += szAdd;
			}
		}

		for( int i = 0; i < 5 * nMaxLen / 2; i++ )
			szGetString += L" ";
		szGetString += L"\n";
		if( nItemTotalCnt != 0 && nItemCnt <= nItemTotalCnt && m_pTextBox ) {
			m_pTextBox->AddText( FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 422 ), 100 * nItemCnt / nItemTotalCnt ).c_str(), textcolor::YELLOW );
			m_pTextBox->SetTooltipText( szGetString.c_str() );
		}
	}

	if( m_pIcon[m_nQuestIconIndex] ) m_pIcon[m_nQuestIconIndex]->Show( true );
	if( m_pArrowBase ) m_pArrowBase->Show( true );
	m_pathResult = path;
	UpdateDestinationMode();
}

bool CDnQuestSummaryInfoListDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() || !m_pParentSummaryDlg || !m_pParentSummaryDlg->GetHWnd() ) {
		return false;
	}

	if( uMsg == WM_MOUSEMOVE ) {
		POINT MousePoint;
		float fMouseX, fMouseY;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		fMouseX = MousePoint.x / GetScreenWidth();
		fMouseY = MousePoint.y / GetScreenHeight();

		SUICoord dlgCoord( m_DlgInfo.DlgCoord );
		if( dlgCoord.IsInside( fMouseX, fMouseY ) && IsShowElementDialog() ) {
			m_bMouseOver = true;
			return true;
		}
		m_bMouseOver = false;
	}
	return true;
}

void CDnQuestSummaryInfoListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) {
		if( IsCmdControl( "ID_BT_MAIN_CLOSE" ) ) {
			if( m_pJournal ) {
				if( m_nQuestIconIndex == 0 ) {
					if( GetQuestTask().RemoveQuestNotifier( DNNotifier::Type::MainQuest, m_pJournal->nQuestIndex ) )
						GetQuestTask().OnUpdateNotifier( true );
				}
				else {
					if( GetQuestTask().RemoveQuestNotifier( DNNotifier::Type::SubQuest, m_pJournal->nQuestIndex ) )
						GetQuestTask().OnUpdateNotifier( true );
				}
			}

			CDnQuestSummaryInfoDlg *pDlg = (CDnQuestSummaryInfoDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::NOTIFIER_DIALOG );
			if( pDlg && pDlg->IsPriorityMode() ) {
				pDlg->DeletePriorityQuest( m_nQuestID );
			}

			if( GetQuestTask().GetQuestDlg() ) {
				GetQuestTask().GetQuestDlg()->RefreshQuestNotifierCheck();
			}
		}
	}
}

void CDnQuestSummaryInfoListDlg::DrawArrow()
{
	if( !CDnQuestTask::IsActive() || !m_bShow || m_nNotifierIndex >= DNNotifier::RegisterCount::TotalQuest )
		return;

	CDnQuestTask::QuestNotifierInfo info = GetQuestTask().GetNotifierInfo( m_nNotifierIndex );

	EtVector3 vSourcePos;
	if( CDnActor::s_hLocalActor ) {
		vSourcePos = *CDnActor::s_hLocalActor->GetPosition();
	}
	else {
		vSourcePos = *CDnWorld::GetInstance().GetUpdatePosition();
	}

	EtVector2 vTargetPos = info.vTargetPos;
	if( m_Quest != NULL ) {
		vTargetPos = CDnNotifierControls::CalcArrowDestination( m_Quest->nQuestID, m_Quest->cQuestJournal );
	}

	const float fDummyPos = -99999.0f;
	bool bInvalidTarget = false;
	if( vTargetPos.x <= fDummyPos ) {
		bInvalidTarget = true;
	}

	EtVector3 vTarget( vTargetPos.x, 0.0f, vTargetPos.y );
	EtVector3 vSource = vSourcePos;
	EtVector3 vDir = vTarget - vSource;
	EtVector3 vTemp = vDir;
	EtVector3 vCross( 0, 0, 0 );
	EtVector3 vZDir( 0, 0, 0 );
	vTemp.y = 0.0f;
	EtVec3Normalize( &vTemp, &vTemp );

	EtVector2 v1( vTarget.x, vTarget.z );
	EtVector2 v2( vSource.x, vSource.z );
	EtVector2 v = v1 - v2;

	float fDist = EtVec2Length( &v );
	int nDist = abs( int( fDist / 100.0f ) );
	WCHAR wszBuff[256] = { 0, };
	if( fDist > 0.0f ) {
		if( bInvalidTarget ) {
			nDist = 0;
		}
		swprintf_s( wszBuff, _countof(wszBuff), L"%dM", nDist );
	}

	int nStyle = m_nQuestIconIndex;
	if( m_bNowTraceQuest && m_bTraceQuest ) {
		nStyle = Style_Track;
	}
	if( nStyle < 0 || nStyle >= Style_Max ) {
		nStyle = Style_Sub;
	}

	if( m_pDistance[nStyle] ) {
		m_pDistance[nStyle]->SetText( wszBuff );
	}

	CEtUIStatic *pArrow = NULL;
	if( m_nArrowState == ArrowType::Hide ) pArrow = m_pArrowWrong[nStyle];
	if( m_nArrowState == ArrowType::NormalArrow ) {
		if( fDist < 100.0f ) pArrow = m_pArrowArrivalDistance[nStyle];
		else pArrow = m_pArrow[nStyle];
	}
	if( m_nArrowState == ArrowType::ArrivalMark ) pArrow = m_pArrowArrival[nStyle];
	if( m_nArrowState == ArrowType::BattleMark ) pArrow = m_pArrowBattle[nStyle];
	if( m_nArrowState == ArrowType::FindMark ) pArrow = m_pArrowFind[nStyle];

	if( !pArrow )
		return;

	DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
	if( hCamera ) {
		vZDir = hCamera->GetMatEx()->m_vZAxis;
		vZDir.y = 0.0f;
	}

	EtVec3Normalize( &vZDir, &vZDir );
	float fDegree = EtAcos( EtVec3Dot( &vTemp, &vZDir ) );
	EtVec3Cross( &vCross, &vTemp, &vZDir );
	if( vCross.y > 0.0f ) {
		fDegree = -fDegree;
	}
	if( bInvalidTarget ) {
		fDegree = 0.0f;
	}

	SUICoord uiCoord;
	pArrow->GetUICoord( uiCoord );
	SUIElement *pElement = pArrow->GetElement( 0 );
	if( !pElement )
		return;

	LOCAL_TIME time = CGlobalInfo::GetInstance().m_LocalTime;

	switch( m_nArrowState )
	{
	case ArrowType::NormalArrow:
		{
			static int nTerm = 1200;
			int nCurrMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
			if( m_pathResult.size() == 1 && m_pathResult[0].nMapIndex == nCurrMapIndex ) {
				int nAlpha = EtClamp( (int)( sinf( ET_PI * 2 * ( GetTickCount() % nTerm ) / (float)nTerm ) * 100 + ( 255 - 100 ) ), 0, 255 );
				if( pArrow->GetTemplate().m_hTemplateTexture )
					DrawSprite( pArrow->GetTemplate().m_hTemplateTexture, pElement->TemplateUVCoord, D3DCOLOR_ARGB( nAlpha, 255, 255, 255 ), uiCoord, EtToDegree( fDegree ) );
				else
					DrawSprite( pElement->UVCoord, D3DCOLOR_ARGB( nAlpha, 255, 255, 255 ), uiCoord, EtToDegree( fDegree ) );
			}
			else {
				if( pArrow->GetTemplate().m_hTemplateTexture )
					DrawSprite( pArrow->GetTemplate().m_hTemplateTexture, pElement->TemplateUVCoord, 0xffffffff, uiCoord, EtToDegree( fDegree ) );
				else
					DrawSprite( pElement->UVCoord, 0xffffffff, uiCoord, EtToDegree( fDegree ) );
			}
			if( m_pDistance[nStyle] ) m_pDistance[nStyle]->Show( true );
		}
		break;
	case ArrowType::Hide:
		{
			if( pArrow->GetTemplate().m_hTemplateTexture )
				DrawSprite( pArrow->GetTemplate().m_hTemplateTexture, pElement->TemplateUVCoord, D3DCOLOR_XRGB( 255, 255, 255 ), uiCoord, 0.0f );
			else
				DrawSprite( pElement->UVCoord, D3DCOLOR_XRGB( 255, 255, 255 ), uiCoord, 0.0f );
			if( m_pDistance[nStyle] ) m_pDistance[nStyle]->Show( false );
		}
		break;
	case ArrowType::ArrivalMark:
	case ArrowType::BattleMark:
	case ArrowType::FindMark:
		{
			int nAlpha = int( time % 1000 );
			nAlpha = ( nAlpha > 499 ) ? ( 1000 - nAlpha ) : nAlpha;
			nAlpha = nAlpha * 255 / 1000;
			if( pArrow->GetTemplate().m_hTemplateTexture )
				DrawSprite( pArrow->GetTemplate().m_hTemplateTexture, pElement->TemplateUVCoord, D3DCOLOR_ARGB( nAlpha, 255, 255, 255 ), uiCoord, 0.0f );
			else
				DrawSprite( pElement->UVCoord, D3DCOLOR_ARGB( nAlpha, 255, 255, 255 ), uiCoord, 0.0f );
			if( m_pDistance[nStyle] ) m_pDistance[nStyle]->Show( false );
		}
		break;
	}
}

// ============================================================================
// CDnQuestSummaryInfoDlg
// ============================================================================

CDnQuestSummaryInfoDlg::CDnQuestSummaryInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	m_nClickIndex = -1;
	m_nOverIndex = -1;
	m_lastOverIndex = -1;
	m_fSelectAlpha = 0.0f;
	m_fOverIndexSmooth = 0.0f;
	m_fControlDistance = 0.0f;
	m_lastHeight = 0.0f;
	m_bPriorityMode = false;
	m_nDeletePriorityQuestID = -1;
	m_pListBoxEx = NULL;
}

CDnQuestSummaryInfoDlg::~CDnQuestSummaryInfoDlg(void)
{
}

void CDnQuestSummaryInfoDlg::Initialize( bool bShow )
{
	BaseClass::Initialize( CEtResourceMng::GetInstance().GetFullName( "QuestSumInfoDlg.ui" ).c_str(), bShow );
}

void CDnQuestSummaryInfoDlg::InitialUpdate()
{
	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
		char szLabel[32] = { 0, };
		sprintf_s( szLabel, _countof(szLabel), "MIS%d", i );

		m_NotifierControls[i].m_OverBoard = GetControl<CEtUIStatic>( FormatA( "ID_STATIC_OVER%d", i ).c_str() );
		m_NotifierControls[i].m_Board = GetControl<CEtUIStatic>( FormatA( "ID_STATIC_BOARD%d", i ).c_str() );
		if( m_NotifierControls[i].m_OverBoard ) m_NotifierControls[i].m_OverBoard->Show( false );
		if( m_NotifierControls[i].m_Board ) m_NotifierControls[i].m_Board->Show( false );

		m_NotifierControls[i].m_pIcon = GetControl<CEtUIStatic>( FormatA( "ID_%s_ICON", szLabel ).c_str() );
		m_NotifierControls[i].m_pTitle = GetControl<CEtUIStatic>( FormatA( "ID_%s_TITLE", szLabel ).c_str() );
		m_NotifierControls[i].m_pTextBox = GetControl<CEtUITextBox>( FormatA( "ID_%s_TEXTBOX", szLabel ).c_str() );
		m_NotifierControls[i].m_pButtonClose = GetControl<CEtUIButton>( FormatA( "ID_BT_%s_CLOSE", szLabel ).c_str() );

		m_NotifierControls[i].SetDialog( this );
		m_NotifierControls[i].SetNotifier( i );
		m_NotifierControls[i].Show( false );
		m_NotifierControls[i].MoveY( (float)i );
	}

	m_pListBoxEx = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_QUEST" );
	if( m_pListBoxEx ) {
		CDnQuestSummaryInfoListDlg *pDummy = m_pListBoxEx->AddItem<CDnQuestSummaryInfoListDlg>();
		if( pDummy ) {
			SUICoord dlgCoord;
			pDummy->GetDlgCoord( dlgCoord );
			m_fControlDistance = dlgCoord.fHeight;
		}
		m_pListBoxEx->RemoveAllItems();
	}

	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
		m_NotifierControls[i].SetDistance( m_fControlDistance );
	}
}

bool CDnQuestSummaryInfoDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() ) {
		return BaseClass::MsgProc( hWnd, uMsg, wParam, lParam );
	}

	int nOverIndex = -1;
	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
		if( uMsg == WM_MOUSEMOVE ) {
			m_NotifierControls[i].MsgProc( hWnd, uMsg, wParam, lParam );
		}
		if( m_NotifierControls[i].IsMouseOver() ) {
			nOverIndex = i;
		}
	}

	if( m_pListBoxEx ) {
		for( int i = 0; i < m_pListBoxEx->GetSize(); ++i ) {
			CDnQuestSummaryInfoListDlg *pItem = m_pListBoxEx->GetItem<CDnQuestSummaryInfoListDlg>( i );
			if( pItem ) {
				pItem->MsgProc( hWnd, uMsg, wParam, lParam );
				if( pItem->IsMouseOver() ) {
					nOverIndex = i + DNNotifier::RegisterCount::TotalQuest;
					break;
				}
			}
		}
	}

	if( nOverIndex == -1 ) {
		for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
			if( m_NotifierControls[i].m_OverBoard ) m_NotifierControls[i].m_OverBoard->Show( false );
			if( m_NotifierControls[i].m_Board ) m_NotifierControls[i].m_Board->Show( false );
		}
		if( m_pListBoxEx ) {
			for( int i = 0; i < m_pListBoxEx->GetSize(); ++i ) {
				CDnQuestSummaryInfoListDlg *pItem = m_pListBoxEx->GetItem<CDnQuestSummaryInfoListDlg>( i );
				if( pItem ) {
					if( pItem->GetOverBoard() ) pItem->GetOverBoard()->Show( false );
					if( pItem->GetBoard() ) pItem->GetBoard()->Show( false );
				}
			}
			m_pListBoxEx->DeselectItem();
		}
	}
	else {
		for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
			if( m_NotifierControls[i].IsShow() ) {
				if( m_NotifierControls[i].m_OverBoard ) m_NotifierControls[i].m_OverBoard->Show( i == nOverIndex );
				if( m_NotifierControls[i].m_Board ) m_NotifierControls[i].m_Board->Show( true );
			}
			else {
				if( m_NotifierControls[i].m_Board ) m_NotifierControls[i].m_Board->Show( false );
				if( m_NotifierControls[i].m_OverBoard ) m_NotifierControls[i].m_OverBoard->Show( false );
			}
		}
		if( m_pListBoxEx ) {
			for( int i = 0; i < m_pListBoxEx->GetSize(); ++i ) {
				CDnQuestSummaryInfoListDlg *pItem = m_pListBoxEx->GetItem<CDnQuestSummaryInfoListDlg>( i );
				if( pItem ) {
					if( pItem->IsShow() ) {
						if( pItem->GetOverBoard() ) pItem->GetOverBoard()->Show( i == ( nOverIndex - DNNotifier::RegisterCount::TotalQuest ) );
						if( pItem->GetBoard() ) pItem->GetBoard()->Show( true );
					}
					else {
						if( pItem->GetOverBoard() ) pItem->GetOverBoard()->Show( false );
						if( pItem->GetBoard() ) pItem->GetBoard()->Show( false );
					}
				}
			}
		}

		if( nOverIndex < DNNotifier::RegisterCount::TotalMission ) {
			if( uMsg == WM_LBUTTONDOWN ) {
				m_nClickIndex = nOverIndex;
				if( m_NotifierControls[nOverIndex].m_pButtonClose ) {
					POINT MousePoint;
					MousePoint.x = short( LOWORD( lParam ) );
					MousePoint.y = short( HIWORD( lParam ) );
					float fMouseX, fMouseY;
					m_NotifierControls[nOverIndex].m_pDialog->PointToFloat( MousePoint, fMouseX, fMouseY );
					if( m_NotifierControls[nOverIndex].m_pButtonClose->IsInside( fMouseX, fMouseY ) ) {
						m_NotifierControls[nOverIndex].Show( false );
						CDnMissionTask::MissionInfoStruct *pMissionStruct = GetMissionTask().GetMissionInfoForNotifier( nOverIndex );
						if( pMissionStruct ) {
							GetMissionTask().RemoveNotifier( GetMissionTask().GetNotifierType( pMissionStruct ), pMissionStruct->nNotifierID );
						}
						CDnMissionDlg *pDlg = (CDnMissionDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::MISSION_DIALOG );
						if( pDlg && pDlg->IsShow() ) {
							pDlg->Show( false );
							pDlg->Show( true );
						}
						m_nClickIndex = -1;
					}
					if( GetQuestTask().GetQuestDlg() ) {
						GetQuestTask().GetQuestDlg()->RefreshQuestNotifierCheck();
					}
				}
			}
			else if( uMsg == WM_LBUTTONUP ) {
				if( m_nClickIndex == nOverIndex ) {
					CDnMissionTask::MissionInfoStruct *pMissionStruct = GetMissionTask().GetMissionInfoForNotifier( nOverIndex );
					if( pMissionStruct && GetInterface().GetMainMenuDialog() ) {
						GetInterface().GetMainMenuDialog()->ShowMissionDialog( (MissionInfoStruct *)pMissionStruct );
					}
				}
				m_nClickIndex = -1;
				return true;
			}
		}
		else if( nOverIndex >= DNNotifier::RegisterCount::TotalQuest && uMsg == WM_LBUTTONUP ) {
			if( m_pListBoxEx ) {
				int nSelIdx = nOverIndex - DNNotifier::RegisterCount::TotalQuest;
				CDnQuestSummaryInfoListDlg *pItem = m_pListBoxEx->GetItem<CDnQuestSummaryInfoListDlg>( nSelIdx );
				if( pItem ) {
					POINT MousePoint;
					MousePoint.x = short( LOWORD( lParam ) );
					MousePoint.y = short( HIWORD( lParam ) );
					float fMouseX, fMouseY;
					pItem->PointToFloat( MousePoint, fMouseX, fMouseY );
					if( pItem->GetCloseButton() && !pItem->GetCloseButton()->IsInside( fMouseX, fMouseY ) && pItem->GetJournal() ) {
						if( GetInterface().GetMainMenuDialog() ) {
							Journal *pJournal = pItem->GetJournal();
							GetInterface().GetMainMenuDialog()->ShowQuestDialog( true, pJournal->nChapterIndex, pJournal->nQuestIndex, pJournal->nChapterIndex > 0 );
						}
						CDnWorldMapDlg *pWorldMapDlg = (CDnWorldMapDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::WORLDMAP_DIALOG );
						if( pWorldMapDlg ) {
							pWorldMapDlg->Show( false );
						}
						return true;
					}
				}
			}
		}
	}

	return BaseClass::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnQuestSummaryInfoDlg::Process( float fElapsedTime )
{
	if( m_pListBoxEx && m_nDeletePriorityQuestID > -1 ) {
		for( int i = 0; i < m_pListBoxEx->GetSize(); ++i ) {
			CDnQuestSummaryInfoListDlg *pItem = m_pListBoxEx->GetItem<CDnQuestSummaryInfoListDlg>( i );
			if( pItem && pItem->GetQuestID() == m_nDeletePriorityQuestID ) {
				m_pListBoxEx->RemoveItem( i );
				m_nDeletePriorityQuestID = -1;
				break;
			}
		}
	}

	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
		m_NotifierControls[i].SetClicked( m_nClickIndex == i );
		m_NotifierControls[i].Process( fElapsedTime );
	}

	BaseClass::Process( fElapsedTime );
}

void CDnQuestSummaryInfoDlg::Render( float fElapsedTime )
{
	if( !IsShow() ) return;

	CDnWorldMapDlg *pWorldMapDlg = (CDnWorldMapDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::WORLDMAP_DIALOG );
	if( pWorldMapDlg && pWorldMapDlg->IsShow() ) {
		if( !GetInterface().IsOpenBlind() ) return;
	}

	BaseClass::Render( fElapsedTime );

	if( m_pListBoxEx ) {
		for( int i = 0; i < m_pListBoxEx->GetSize(); ++i ) {
			CDnQuestSummaryInfoListDlg *pItem = m_pListBoxEx->GetItem<CDnQuestSummaryInfoListDlg>( i );
			if( pItem && pItem->IsShowElementDialog() ) {
				pItem->DrawArrow();
			}
		}
	}
}

void CDnQuestSummaryInfoDlg::ClearAll()
{
	if( m_pListBoxEx ) {
		m_pListBoxEx->RemoveAllItems();
	}
	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; i++ ) {
		m_NotifierControls[i].Show( false );
	}
}

bool CompareQuestLevel( int nFirstQuestIndex, int nSecondQuestIndex )
{
	Journal *pFirst = g_DataManager.GetJournalData( nFirstQuestIndex );
	Journal *pSecond = g_DataManager.GetJournalData( nSecondQuestIndex );

	if( pFirst && pSecond ) {
		if( pFirst->nQuestLevel < pSecond->nQuestLevel )
			return true;
	}
	return false;
}

bool CDnQuestSummaryInfoDlg::IsCanProgressQuest( int nQuestIndex )
{
	int nMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
	TQuest *QuestInfo = NULL;
	Journal *pJournal = NULL;
	JournalPage *pJournalPage = NULL;
	std::deque<PathInfo> pathResult;
	int nArrowState = ArrowType::Hide;

	std::vector<TQuest*> vecProgSubQuest;
	GetQuestTask().GetPlaySubQuest( vecProgSubQuest );

	for( DWORD i = 0; i < vecProgSubQuest.size(); i++ ) {
		if( vecProgSubQuest[i] && vecProgSubQuest[i]->nQuestID == nQuestIndex ) {
			QuestInfo = vecProgSubQuest[i];
			break;
		}
	}

	if( QuestInfo ) {
		pJournal = g_DataManager.GetJournalData( QuestInfo->nQuestID );
		if( pJournal ) {
			pJournalPage = pJournal->FindJournalPage( QuestInfo->cQuestJournal );
			if( pJournalPage ) {
				nArrowState = CDnNotifierControls::GetNotifyArrowState( QuestInfo->nQuestID, QuestInfo->cQuestJournal );
				GetQuestTask().GetPathFinder()->GetPathOneWay( nMapIndex, pJournalPage->nDestnationMapIndex, pathResult );
				if( pJournalPage->nDestnationMapIndex != nMapIndex && nArrowState == ArrowType::NormalArrow && pathResult.empty() ) {
					nArrowState = ArrowType::Hide;
				}
				return ( nArrowState != ArrowType::Hide );
			}
		}
	}

	return false;
}

void CDnQuestSummaryInfoDlg::RefreshPriortyNotifier()
{
	if( !m_pListBoxEx ) return;

	DNTableFileFormat *pMapTable = GetDNTable( CDnTableDB::TMAP );
	if( !pMapTable ) return;

	int nTraceQuestID = GetQuestTask().GetTraceQuestID();

	std::vector< boost::tuple<std::wstring, int, int> > vecGetList;
	std::wstring szProgress;

	std::vector<int> vec_PriorityQuestList;
	std::vector<int> vec_AddedQuestList;
	std::vector<TQuest*> vecProgSubQuest;
	GetQuestTask().GetPlaySubQuest( vecProgSubQuest );

	for( int i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++ ) {
		Journal *pJournal = GetQuestTask().GetNotifierJournal( i );
		JournalPage *pJournalPage = GetQuestTask().GetNotifierJournalPage( i );

		if( pJournal && pJournalPage && IsCanProgressQuest( pJournal->nQuestIndex ) ) {
			if( pMapTable->GetFieldFromLablePtr( pJournalPage->nDestnationMapIndex, "_MapType" )->GetInteger() == CDnWorld::MapTypeDungeon )
				vec_PriorityQuestList.push_back( pJournal->nQuestIndex );
			else
				vec_AddedQuestList.push_back( pJournal->nQuestIndex );
		}
	}

	int nRemainSize = (int)( vec_PriorityQuestList.size() + vec_AddedQuestList.size() );
	if( nRemainSize < DNNotifier::RegisterCount::SubQuest ) {
		for( DWORD j = 0; j < vecProgSubQuest.size(); j++ ) {
			if( !vecProgSubQuest[j] ) continue;
			bool bPassIndex = true;
			for( DWORD k = 0; k < vec_PriorityQuestList.size(); k++ ) {
				if( vecProgSubQuest[j]->nQuestID == vec_PriorityQuestList[k] ) {
					bPassIndex = false;
					break;
				}
			}
			for( DWORD k = 0; k < vec_AddedQuestList.size(); k++ ) {
				if( vecProgSubQuest[j]->nQuestID == vec_AddedQuestList[k] ) {
					bPassIndex = false;
					break;
				}
			}

			if( bPassIndex ) {
				Journal *pJournal = g_DataManager.GetJournalData( vecProgSubQuest[j]->nQuestID );
				JournalPage *pJournalPage = pJournal ? pJournal->FindJournalPage( vecProgSubQuest[j]->cQuestJournal ) : NULL;
				if( pJournal && pJournalPage && IsCanProgressQuest( pJournal->nQuestIndex ) ) {
					if( pMapTable->GetFieldFromLablePtr( pJournalPage->nDestnationMapIndex, "_MapType" )->GetInteger() == CDnWorld::MapTypeDungeon )
						vec_PriorityQuestList.push_back( pJournal->nQuestIndex );
					else
						vec_AddedQuestList.push_back( pJournal->nQuestIndex );
				}
			}
		}
	}

	if( vec_PriorityQuestList.size() > 1 ) {
		std::sort( vec_PriorityQuestList.begin(), vec_PriorityQuestList.end(), CompareQuestLevel );
	}

	if( !vec_AddedQuestList.empty() ) {
		if( vec_AddedQuestList.size() > 1 ) {
			std::sort( vec_AddedQuestList.begin(), vec_AddedQuestList.end(), CompareQuestLevel );
		}
		for( DWORD i = 0; i < vec_AddedQuestList.size(); i++ ) {
			vec_PriorityQuestList.push_back( vec_AddedQuestList[i] );
			if( vec_PriorityQuestList.size() >= DNNotifier::RegisterCount::SubQuest )
				break;
		}
	}

	for( DWORD i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++ ) {
		int nArrayIndex = i - DNNotifier::RegisterCount::MainQuest;
		if( (int)vec_PriorityQuestList.size() <= nArrayIndex )
			break;

		TQuest *QuestInfo = NULL;
		for( DWORD j = 0; j < vecProgSubQuest.size(); j++ ) {
			if( vecProgSubQuest[j] && vecProgSubQuest[j]->nQuestID == vec_PriorityQuestList[nArrayIndex] ) {
				QuestInfo = vecProgSubQuest[j];
				break;
			}
		}

		Journal *pJournal = QuestInfo ? g_DataManager.GetJournalData( QuestInfo->nQuestID ) : NULL;
		JournalPage *pJournalPage = pJournal ? pJournal->FindJournalPage( QuestInfo->cQuestJournal ) : NULL;

		if( pJournal && pJournalPage ) {
			vecGetList.clear();
			szProgress.clear();
			std::deque<PathInfo> pathResult;
			GetQuestTask().GetPathFinder()->GetPathOneWay( CGlobalInfo::GetInstance().m_nCurrentMapIndex, pJournalPage->nDestnationMapIndex, pathResult );
			GetQuestTask().GetCurJournalCountingInfoString( pJournal->nQuestIndex, pJournalPage->nJournalPageIndex, vecGetList, szProgress );

			CDnQuestSummaryInfoListDlg *pItem = m_pListBoxEx->AddItem<CDnQuestSummaryInfoListDlg>();
			if( pItem ) {
				pItem->SetQuestID( pJournal->nQuestIndex );
				pItem->SetParentSummaryDlg( this );
				pItem->SetQuestInfo( QuestInfo );
				pItem->SetJournal( pJournal );
				pItem->Refresh( 1, (int)i, pJournal->wszQuestTitle.c_str(), pJournalPage->szDestination.c_str(), vecGetList, pathResult );
				pItem->UpdateDestinationMode();
				if( pJournalPage->nDestnationMapIndex != CGlobalInfo::GetInstance().m_nCurrentMapIndex ) {
					pItem->ChangeArrowStateByPathCheck();
				}
				pItem->SetTraceQuest( pJournal->nQuestIndex == nTraceQuestID );
			}
		}
	}
}

void CDnQuestSummaryInfoDlg::RefreshMainNotifier()
{
	if( !m_pListBoxEx ) return;

	std::vector< boost::tuple<std::wstring, int, int> > vecGetList;
	std::wstring szProgress;
	int nTraceQuestID = GetQuestTask().GetTraceQuestID();

	if( GetQuestTask().IsMainQuestNotifierExist() ) {
		Journal *pJournal = GetQuestTask().GetNotifierJournal( 0 );
		JournalPage *pJournalPage = GetQuestTask().GetNotifierJournalPage( 0 );
		if( pJournal && pJournalPage ) {
			vecGetList.clear();
			szProgress.clear();
			GetQuestTask().GetCurJournalCountingInfoString( pJournal->nQuestIndex, pJournalPage->nJournalPageIndex, vecGetList, szProgress );

			CDnQuestSummaryInfoListDlg *pItem = m_pListBoxEx->AddItem<CDnQuestSummaryInfoListDlg>();
			if( pItem ) {
				pItem->SetParentSummaryDlg( this );
				pItem->SetQuestID( pJournal->nQuestIndex );
				pItem->SetJournal( pJournal );
				CDnQuestTask::QuestNotifierInfo info = GetQuestTask().GetNotifierInfo( 0 );
				pItem->Refresh( 0, 0, pJournal->wszQuestTitle.c_str(), pJournalPage->szDestination.c_str(), vecGetList, info.pathResult );
				pItem->UpdateDestinationMode();
				if( pJournalPage->nDestnationMapIndex != CGlobalInfo::GetInstance().m_nCurrentMapIndex ) {
					pItem->ChangeArrowStateByPathCheck();
				}
				pItem->SetTraceQuest( pJournal->nQuestIndex == nTraceQuestID );
			}
		}
		else {
			const TQuest *pQuest = GetQuestTask().GetQuestIndex( GetQuestTask().GetNotifierInfo( 0 ).nQuestIndex );
			if( !pQuest || pQuest->cQuestJournal != 0 ) {
				GetQuestTask().ClearNotifierData( 0 );
			}
		}
	}
}

void CDnQuestSummaryInfoDlg::RefreshSubNotifier()
{
	if( !m_pListBoxEx ) return;

	std::vector< boost::tuple<std::wstring, int, int> > vecGetList;
	std::wstring szProgress;
	int nTraceQuestID = GetQuestTask().GetTraceQuestID();

	for( int i = DNNotifier::RegisterCount::MainQuest; i < DNNotifier::RegisterCount::TotalQuest; i++ ) {
		Journal *pJournal = GetQuestTask().GetNotifierJournal( i );
		JournalPage *pJournalPage = GetQuestTask().GetNotifierJournalPage( i );
		if( pJournal && pJournalPage ) {
			vecGetList.clear();
			szProgress.clear();
			GetQuestTask().GetCurJournalCountingInfoString( pJournal->nQuestIndex, pJournalPage->nJournalPageIndex, vecGetList, szProgress );

			CDnQuestSummaryInfoListDlg *pItem = m_pListBoxEx->AddItem<CDnQuestSummaryInfoListDlg>();
			if( pItem ) {
				pItem->SetQuestID( pJournal->nQuestIndex );
				pItem->SetParentSummaryDlg( this );
				pItem->SetJournal( pJournal );
				CDnQuestTask::QuestNotifierInfo info = GetQuestTask().GetNotifierInfo( i );
				pItem->Refresh( 1, i, pJournal->wszQuestTitle.c_str(), pJournalPage->szDestination.c_str(), vecGetList, info.pathResult );
				pItem->UpdateDestinationMode();
				if( pJournalPage->nDestnationMapIndex != CGlobalInfo::GetInstance().m_nCurrentMapIndex ) {
					pItem->ChangeArrowStateByPathCheck();
				}
				pItem->SetTraceQuest( pJournal->nQuestIndex == nTraceQuestID );
			}
		}
		else {
			const TQuest *pQuest = GetQuestTask().GetQuestIndex( GetQuestTask().GetNotifierInfo( i ).nQuestIndex );
			if( !pQuest || pQuest->cQuestJournal != 0 ) {
				GetQuestTask().ClearNotifierData( i );
			}
		}
	}

	if( GetQuestTask().GetQuestDlg() ) {
		GetQuestTask().GetQuestDlg()->RefreshQuestNotifierCheck();
	}
}

void CDnQuestSummaryInfoDlg::RefreshMissionNotifier()
{
	std::vector< boost::tuple<std::wstring, int, int> > vecGetList;
	std::wstring szProgress;

	for( int i = 0; i < DNNotifier::RegisterCount::TotalMission; ++i ) {
		CDnMissionTask::MissionInfoStruct *pStruct = GetMissionTask().GetMissionInfoForNotifier( i );
		if( !pStruct ) continue;

		m_NotifierControls[i].Show( true );
		szProgress.clear();
		if( pStruct->nArrayIndex == -1 && ( pStruct->nType == 1 || pStruct->nType == 2 ) ) {
			szProgress = FormatW( L"(%d/%d)", pStruct->nCounter, pStruct->nRewardCounter );
		}

		vecGetList.clear();
		std::wstring szCategory;
		if( pStruct->nArrayIndex == -1 ) {
			if( pStruct->nType == 5 )
				szCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000029520 );
			else
				szCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10186 );
		}
		else {
			switch( pStruct->MainCategory )
			{
			case 0: szCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 501 ); break;
			case 1: szCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10191 ); break;
			case 2: szCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2419 ); break;
			case 3: szCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1372 ); break;
			case 4: szCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10192 ); break;
			}
		}
		vecGetList.push_back( boost::make_tuple( szCategory, -1, -1 ) );

		std::wstring szSubCategory;
		if( pStruct->nArrayIndex == -1 ) {
			switch( pStruct->nType )
			{
			case 1: szSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9740 ); break;
			case 2: szSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9741 ); break;
			case 3: szSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126187 ); break;
			case 5: szSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000029520 ); break;
			case 6: szSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10077 ); break;
			case 7: szSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10079 ); break;
			case 8: szSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10167 ); break;
			case 9: szSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8569 ); break;
			}
		}
		else {
			for( int k = 0; ; k++ ) {
				std::string strValue = _GetSubStrByCountSafe( k, (char*)pStruct->szSubCategory.c_str(), '/' );
				if( strValue.size() == 0 ) break;
				if( k > 0 ) szSubCategory += L" > ";
				szSubCategory += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, atoi( strValue.c_str() ) );
			}
		}
		vecGetList.push_back( boost::make_tuple( szSubCategory, -1, -1 ) );

		m_NotifierControls[i].Refresh( pStruct->szTitle.c_str(), pStruct->szSubTitle.c_str(), vecGetList, szProgress );
	}
}

void CDnQuestSummaryInfoDlg::OnChangeResolution()
{
	CEtUIDialog::OnChangeResolution();
	RefreshNotifier();
}

void CDnQuestSummaryInfoDlg::RefreshNotifier()
{
	if( !CDnQuestTask::IsActive() )
		return;

	ClearAll();
	RefreshMainNotifier();

	if( m_bPriorityMode ) {
		RefreshPriortyNotifier();
	}
	else {
		RefreshSubNotifier();
	}

	RefreshMissionNotifier();

	if( m_pListBoxEx ) {
		int nQuestCount = m_pListBoxEx->GetSize();
		m_pListBoxEx->SetRenderScrollBar( nQuestCount > 4 );
		float fPosY = (float)( nQuestCount < 5 ? nQuestCount : 4 ) - 4.0f;

		if( m_NotifierControls[0].IsShow() ) {
			m_NotifierControls[0].MoveY( fPosY );
			fPosY += 1.0f;
		}
		if( m_NotifierControls[1].IsShow() ) {
			if( !m_NotifierControls[0].IsShow() ) {
				fPosY += 0.5f;
			}
			m_NotifierControls[1].MoveY( fPosY );
		}
	}
}
