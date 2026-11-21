#include "stdafx.h"
#include "DnAppellationBookDlg.h"
#include "DnAppellationTask.h"
#include "DnMissionAppellationTooltipDlg.h"
#include "DnAppellationBookContentsDlg.h"
#include "DnSimpleTooltipDlg.h"
#include "DnInterface.h"
#include "DnItemTask.h"
#include "DnMainDlg.h"
#ifdef PRE_FIX_APPELATION_TOOLTIP
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#endif


#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL

#include "DnUIString.h"
#include "DnAppellationRewardCoinTooltipDlg.h"


//////////////////////////////////////////////////////////////////////////
// CDnAppellationBookDlg (newly renewed)

CDnAppellationBookDlg::CDnAppellationBookDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_nCurrentPage(0)
, m_nMaxPage(0)
, m_pStaticCurrentPage( NULL )
, m_pAppellationTooltip(NULL)
, m_pListBoxEx(NULL)
, m_pRewardCoinToolTipDlg(NULL)
, m_bIsSelectListBox(false)
{
	memset( &m_stUIAppellationPage, 0, sizeof( stUIAppellationPage ) );
}

CDnAppellationBookDlg::~CDnAppellationBookDlg()
{
	m_pListBoxEx->RemoveAllItems();
	SAFE_DELETE( m_pRewardCoinToolTipDlg );
	SAFE_DELETE( m_pAppellationTooltip );
}

void CDnAppellationBookDlg::Initialize(bool bShow)
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "AppellationBookDlg.ui" ).c_str(), bShow );

	m_pAppellationTooltip = new CDnMissionAppellationTooltipDlg( UI_TYPE_CHILD, this );
	m_pAppellationTooltip->Initialize( false );

	m_pRewardCoinToolTipDlg = new CDnAppellationRewardCoinTooltipDlg( UI_TYPE_CHILD, this );
	m_pRewardCoinToolTipDlg->Initialize( false );
}

void CDnAppellationBookDlg::InitialUpdate()
{
	CEtUIDialog::InitialUpdate();

	m_stUIAppellationPage.m_StaticTitle			= GetControl<CEtUIStatic>( "ID_TEXT_TITLE1" );
	m_stUIAppellationPage.m_StaticDescription	= GetControl<CEtUIStatic>( "ID_TEXT_SUBTITLE1" );
	m_stUIAppellationPage.m_StaticPercent		= GetControl<CEtUIStatic>( "ID_TEXT_PERCENT1" );
	m_stUIAppellationPage.m_StaticReward		= GetControl<CEtUIStatic>( "ID_TEXT_REWARDNAME1" );
	
	m_pStaticCurrentPage = GetControl<CEtUIStatic>( "ID_TEXT_PAGE" );	
	m_pStaticCurrentPage->SetText( FormatW( L"%d", m_nCurrentPage + 1 ) );

	// List box
	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_LIST"); 

	// Clear mark
	m_stUIAppellationPage.m_pStaticClearMark = GetControl<CEtUIStatic>("ID_STATIC_CLEARMARK"); 
	m_stUIAppellationPage.m_pStaticClearMark->Show(false);

	// Item slot
	int nUIIndex = 1;
	for(int i = 0 ; i < 2 ; ++i , --nUIIndex)
	{
		// Set Slot
		m_stUIAppellationPage.m_pStaticItemSlot[i] = GetControl<CDnItemSlotButton>(FormatA("ID_BT_ITEM%d",nUIIndex).c_str());
		m_stUIAppellationPage.m_pStaticItemSlot[i]->SetSlotType(ST_MARKET_REGIST);
		
		// Set Coin
		m_stUIAppellationPage.m_pStaticItemCoin[i] = GetControl<CEtUIStatic>(FormatA("ID_STATIC_COIN%d",nUIIndex).c_str());
	}

	char szStr[64];
	for( int j=0; j<MAX_APPITEM_PER_PAGE; j++ )
	{
		sprintf_s( szStr, "ID_PAGE1_IN%d", j );		// Star shape
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticIn = GetControl<CEtUIStatic>( szStr );
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticIn->Show( false );
		sprintf_s( szStr, "ID_PAGE1_OUT%d", j );	// Star shape
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticOut = GetControl<CEtUIStatic>( szStr );
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticOut->Show( false );
		
		sprintf_s( szStr, "ID_STATIC_NEW%d", j );	// New mark
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewMark = GetControl<CEtUIStatic>( szStr );

		// Previous title
		sprintf_s( szStr, "ID_TEXT1_IN%d", j ); // Acquired title name
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[E_IN] = GetControl<CEtUIStatic>( szStr );
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[E_IN]->Show(false);
		sprintf_s( szStr, "ID_TEXT1_OUT%d",j ); // Unacquired title name
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[E_OUT] = GetControl<CEtUIStatic>( szStr );
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[E_OUT]->Show(false);
		
		// Title with New mark.
		sprintf_s( szStr, "ID_TEXT2_IN%d", j ); // Acquired title name
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[E_IN] = GetControl<CEtUIStatic>( szStr );
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[E_IN]->Show(false);
		sprintf_s( szStr, "ID_TEXT2_OUT%d",j ); // Unacquired title name
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[E_OUT] = GetControl<CEtUIStatic>( szStr );
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[E_OUT]->Show(false);
	}
}

void CDnAppellationBookDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		CDnAppellationTask::GetInstance().UpdateMyAppellationCollectionList();
		SetContents();
		SetPageUI( m_nCurrentPage );
		m_pStaticCurrentPage->SetText( FormatW( L"%d", m_nCurrentPage + 1 ) );
	}
	else
	{
		DWORD dwCount = CDnAppellationTask::GetInstance().GetAppellationCount();
		for( DWORD i=0; i<dwCount; i++ ) {
			CDnAppellationTask::AppellationStruct *pInfo = CDnAppellationTask::GetInstance().GetAppellationInfo(i);
			pInfo->bNew = false;
		}
	}

	CEtUIDialog::Show( bShow );
}

void CDnAppellationBookDlg::SetContents()
{
	m_pListBoxEx->RemoveAllItems();

	std::vector<CDnAppellationTask::stAppellationCollection>& pVecAppellationCollection = CDnAppellationTask::GetInstance().GetAppellationCollectionList();
	if(pVecAppellationCollection.empty())
		return;

	int nSize = static_cast<int>(pVecAppellationCollection.size());
	
	for(int i = 0 ; i < nSize ; ++i)
	{
		CDnAppellationBookContentsDlg* pItem = m_pListBoxEx->AddItem<CDnAppellationBookContentsDlg>();
		
		std::map<int,int>::iterator it = pVecAppellationCollection[i].m_mapAppellationIndex.begin();
		
		bool bIsExistAppellation = false;
		for( ; it != pVecAppellationCollection[i].m_mapAppellationIndex.end() ; ++it )
		{
			bIsExistAppellation = CDnAppellationTask::GetInstance().IsExistAppellation( (*it).first -1 );
			if(bIsExistAppellation)
				break;
		}

		pItem->SetData(pVecAppellationCollection[i].m_strCollectionTitle, bIsExistAppellation, i);
	}

	m_pListBoxEx->DeselectItem();
}

void CDnAppellationBookDlg::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );
	
	if(!IsShow())
		return;

#ifdef PRE_FIX_APPELATION_TOOLTIP
	CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
	CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
	if (pSystemDlg && pSystemDlg->IsShowDlg(CDnSystemDlg::HELP_DIALOG))
		return;
#endif

	float fMouseX, fMouseY;
	GetScreenMouseMovePoints( fMouseX, fMouseY );

	fMouseX -= GetXCoord();
	fMouseY -= GetYCoord();

	bool bShowTooltip = false;
	for( int i=0; i<2; i++ )
	{
		bool bMouseEnter( false );
		SUICoord uiCoord;
		m_stUIAppellationPage.m_StaticReward->GetUICoord( uiCoord );
		if( uiCoord.IsInside( fMouseX, fMouseY ) )
		{
			ShowAppellationTooltip( fMouseX, fMouseY, i + m_nCurrentPage );
			bShowTooltip = true;
			break;
		}
	}
	if( !bShowTooltip )
		m_pAppellationTooltip->Show( false );

	//---------------------------------------------------
	// Provide tooltips for titles as well
	std::vector<CDnAppellationTask::stAppellationCollection>& pAppellationList = CDnAppellationTask::GetInstance().GetAppellationCollectionList();
	if( pAppellationList.size()-1 != m_nMaxPage ) return;

	std::vector<CDnAppellationTask::stAppellationCollection>::iterator iter = pAppellationList.begin();
	iter += m_nCurrentPage;
	if( iter == pAppellationList.end() ) return;

	std::map<int, int>::iterator iterAppellation = (*iter).m_mapAppellationIndex.begin();
	if( (*iter).m_mapAppellationIndex.size() > MAX_APPITEM_PER_PAGE ) return;

	for( int j = 0 ; j < MAX_APPITEM_PER_PAGE ; ++j )
	{
		// 1. Check if the New button is on
		bool bIsNewMark = false;
		if( m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewMark->IsShow() )
			bIsNewMark = true;
		
		// 2. Check if owned
		int nStaticID = E_OUT; // None
		if( CDnAppellationTask::GetInstance().IsExistAppellation( (*iterAppellation).first -1 ) )
			nStaticID = E_IN; // Exists
		
		if(bIsNewMark)
		{
			if( m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[nStaticID]->IsInside(fMouseX, fMouseY) &&
				m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[nStaticID]->IsShow() )
			{
				std::wstring wszToolTip;
				SetAppellationToolTip(iterAppellation, wszToolTip, (*iter).m_bOpen);

				if(!wszToolTip.empty())
				{
					CDnSimpleTooltipDlg* pSimpleTooltTip = GetInterface().GetSimpleTooltipDialog();	
					if(pSimpleTooltTip)
						pSimpleTooltTip->ShowTooltipDlg(m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewMark, true, wszToolTip, 0xffffffff, true);
				}
			}
		}
		else
		{
			if( m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[nStaticID]->IsInside(fMouseX, fMouseY) &&
				m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[nStaticID]->IsShow() )
			{
				std::wstring wszToolTip;
				SetAppellationToolTip(iterAppellation, wszToolTip, (*iter).m_bOpen);

				if(!wszToolTip.empty())
				{
					CDnSimpleTooltipDlg* pSimpleTooltTip = GetInterface().GetSimpleTooltipDialog();	
					if(pSimpleTooltTip)
						pSimpleTooltTip->ShowTooltipDlg(m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewMark, true, wszToolTip, 0xffffffff, true);
				}
			}
		}

		int nAppellationSize = (int)(*iter).m_mapAppellationIndex.size();
		if( nAppellationSize-1 > j )
			++iterAppellation;
	}

	//---------------------------------------------------
	// Handle coin tooltip among reward tooltips
	bool bIsShow = false;
	for(int i = 0 ; i < 2 ; ++i)
	{
		if( !m_stUIAppellationPage.m_pStaticItemCoin[i]->IsShow() )
			continue;

		if( m_stUIAppellationPage.m_pStaticItemCoin[i]->GetUICoord().IsInside(fMouseX, fMouseY) )
		{
			bIsShow = true;
			break;
		}
	}

	if(bIsShow)
	{
		m_pRewardCoinToolTipDlg->SetRewardCoin(m_nRewardCoinValue);

		GetScreenMouseMovePoints( fMouseX, fMouseY );
		m_pRewardCoinToolTipDlg->SetPosition(fMouseX, fMouseY);

		SUICoord DlgCoord;
		m_pRewardCoinToolTipDlg->GetDlgCoord(DlgCoord);
		static float fXRGap(4.0f / DEFAULT_UI_SCREEN_WIDTH);
		if( (DlgCoord.Right()+fXRGap) > GetScreenWidthRatio() )
			DlgCoord.fX -= ( DlgCoord.Right()+fXRGap - GetScreenWidthRatio() );
		m_pRewardCoinToolTipDlg->SetDlgCoord(DlgCoord);
		m_pRewardCoinToolTipDlg->Show(true);
	}
	else
		m_pRewardCoinToolTipDlg->Show(false);
}

void CDnAppellationBookDlg::SetAppellationToolTip(std::map<int, int>::iterator it, std::wstring& wszTooltip, bool bIsOpenAppellation)
{
	if( ((*it).second == 1) ||						// Title owned (O)
		((*it).second == 0) && bIsOpenAppellation)	// Title not owned (X) && Open title.
	{
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMISSION );
		if(!pSox)
			return;

		int  nMissionID		= -1;
		bool bIsSearchData  = false;
		CDnMissionTask::MissionInfoStruct* pMissionData = new CDnMissionTask::MissionInfoStruct;
		for(int i = 0 ; i < pSox->GetItemCount() ; ++i)
		{
			nMissionID = pSox->GetItemID(i);

			if( (*it).first == pSox->GetFieldFromLablePtr(nMissionID, "_RewardAppellation")->GetInteger() )
			{
				pMissionData->nArrayIndex = i;
				pMissionData->nNotifierID = nMissionID;

				pMissionData->MainCategory = (CDnMissionTask::MainCategoryEnum)pSox->GetFieldFromLablePtr( nMissionID, "_MainCategory" )->GetInteger();
				pMissionData->szSubCategory = pSox->GetFieldFromLablePtr( nMissionID, "_SubCategoryID" )->GetString();
				int		nNameID = pSox->GetFieldFromLablePtr( nMissionID, "_TitleNameID" )->GetInteger();
				char*	szParam = pSox->GetFieldFromLablePtr( nMissionID, "_TitleNameIDParam" )->GetString();
				MakeUIStringUseVariableParam( pMissionData->szTitle, nNameID, szParam );

				nNameID = pSox->GetFieldFromLablePtr( nMissionID, "_SubTitleNameID" )->GetInteger();
				szParam = pSox->GetFieldFromLablePtr( nMissionID, "_SubTitleNameIDParam" )->GetString();
				MakeUIStringUseVariableParam( pMissionData->szSubTitle, nNameID, szParam );

				pMissionData->nType = pSox->GetFieldFromLablePtr( nMissionID, "_Type" )->GetInteger();

				bIsSearchData = true;
				break;
			}
		}

		// If not found in mission table, search once more in daily missions.
		if(!bIsSearchData)
		{
			pSox = GetDNTable(CDnTableDB::TDAILYMISSION);
			if(!pSox)
			{
				SAFE_DELETE( pMissionData );
				return;
			}

			for(int i = 0 ; i < pSox->GetItemCount() ; ++i)
			{
				nMissionID = pSox->GetItemID(i);

				if( (*it).first == pSox->GetFieldFromLablePtr(nMissionID, "_RewardAppellation")->GetInteger() )
				{
					pMissionData->nArrayIndex = i;
					pMissionData->nNotifierID = nMissionID;

					pMissionData->MainCategory = (CDnMissionTask::MainCategoryEnum)pSox->GetFieldFromLablePtr( nMissionID, "_MainCategory" )->GetInteger();
					pMissionData->szSubCategory = pSox->GetFieldFromLablePtr( nMissionID, "_SubCategoryID" )->GetString();

					int		nNameID = pSox->GetFieldFromLablePtr( nMissionID, "_TitleNameID" )->GetInteger();
					char*	szParam = pSox->GetFieldFromLablePtr( nMissionID, "_TitleNameIDParam" )->GetString();
					MakeUIStringUseVariableParam( pMissionData->szTitle, nNameID, szParam );

					nNameID = pSox->GetFieldFromLablePtr( nMissionID, "_SubTitleNameID" )->GetInteger();
					szParam = pSox->GetFieldFromLablePtr( nMissionID, "_SubTitleNameIDParam" )->GetString();
					MakeUIStringUseVariableParam( pMissionData->szSubTitle, nNameID, szParam );

					pMissionData->nType = pSox->GetFieldFromLablePtr( nMissionID, "_Type" )->GetInteger();
					bIsSearchData = true;

					break;
				}
			}
		}

		if(pMissionData->nNotifierID < 0 || !bIsSearchData)
		{
			SAFE_DELETE( pMissionData );
			return;
		}

		std::vector< boost::tuple<std::wstring, int, int> > vecGetList;
		std::wstring wszCategory;
		//if( pMissionData->nArrayIndex != -1 )
		if( pMissionData->nType == eMissionType::MissionType_Normal ) 
		{
			switch( pMissionData->MainCategory )
			{
			case CDnMissionTask::Normal:  wszCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7109 ); break;
			case CDnMissionTask::Dungeon: wszCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7110 ); break;
			case CDnMissionTask::Battle:  wszCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9745 ); break;
			default:					  wszCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7109 ); break;
			}
		}
		else if( pMissionData->nType == eDailyMissionType::DAILYMISSION_GUILDCOMMON )
		{
			wszCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000029520 );
		}
		else
		{
			wszCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9739 );
		}
		vecGetList.push_back( boost::make_tuple(wszCategory, -1, -1) );

		std::wstring wszSubCategory;
		//if( pMissionData->nArrayIndex != -1 )
		if( !pMissionData->szSubCategory.empty() )
		{
			for( int j = 0;; j++ )
			{
				std::string strValue = _GetSubStrByCountSafe( j, (char*)pMissionData->szSubCategory.c_str(), '/' );
				if( strValue.size() == 0 ) break;
				if( j > 0 ) wszSubCategory += L"/";
				int nCategoryID = atoi( strValue.c_str() );
				wszSubCategory += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nCategoryID );
			}
		}
		else
		{
			switch( pMissionData->nType )
			{
			case eDailyMissionType::DAILYMISSION_DAY: wszSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9740 ); break;
			case eDailyMissionType::DAILYMISSION_WEEK: wszSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9741 ); break;
			case eDailyMissionType::DAILYMISSION_GUILDWAR: wszSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126187 ); break;
			case eDailyMissionType::DAILYMISSION_GUILDCOMMON: wszSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000029520 ); break;
			case eDailyMissionType::DAILYMISSION_WEEKENDEVENT: wszSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10077 ); break;
			case eDailyMissionType::DAILYMISSION_WEEKENDREPEAT: wszSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10079 ); break;
#ifdef PRE_ADD_MONTHLY_MISSION
			case eDailyMissionType::DAILYMISSION_MONTH: wszSubCategory = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10167 ); break;
#endif 
			}
		}
		vecGetList.push_back( boost::make_tuple(wszSubCategory, -1, -1) );

		// Tooltip settings
		if(!pMissionData->szTitle.empty())
		{
			// Title
			wszTooltip += pMissionData->szTitle.c_str();
			wszTooltip += L"\n\n";

			// Mission condition
			if(!pMissionData->szSubTitle.empty())
			{
				wszTooltip += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7877 ); // Mission condition
				wszTooltip += L" ";
				wszTooltip += pMissionData->szSubTitle.c_str();
			}

			if( vecGetList.size() == 2 ) 
			{
				// Category
				if(!vecGetList[0].get<0>().empty())
				{
					wszTooltip += L"\n";
					wszTooltip += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7878 ); // Category
					wszTooltip += L" ";
					wszTooltip += vecGetList[0].get<0>();
				}

				// Position
				if(!vecGetList[1].get<0>().empty())
				{					
					wszTooltip += L"\n";
					wszTooltip += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7879 ); // Position
					wszTooltip += L" ";
					wszTooltip += vecGetList[1].get<0>();
				}
			}
		}

		SAFE_DELETE( pMissionData );
	}
	else
	{
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TAPPELLATION );
		if(!pSox)
			return;

		for( int i = 0 ; i < pSox->GetItemCount() ; ++i )
		{
			int nItemID = pSox->GetItemID(i);
			if( nItemID == (*it).first )
			{
				int nStringID = pSox->GetFieldFromLablePtr(nItemID , "_DescriptionID")->GetInteger();
				wszTooltip += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID);
			}
		}
	}
}

void CDnAppellationBookDlg::ShowAppellationTooltip( float fMouseX, float fMouseY, int i, int j )
{
	std::vector<CDnAppellationTask::stAppellationCollection>& pVecAppellationCollection = CDnAppellationTask::GetInstance().GetAppellationCollectionList();
	std::vector<CDnAppellationTask::stAppellationCollection>::iterator iter = pVecAppellationCollection.begin();

	if( i >= static_cast<int>( pVecAppellationCollection.size() ) )
		return;

	int nAppellationIndex = 0;
	if( j == -1 )	// Reward title tooltip
	{
		iter += i;
		nAppellationIndex = iter->m_nRewardAppellationIndex;
	}
/*	else // Collection title tooltip
	{
		iter += i;
		std::map<int, int>::iterator iterAppellation = (*iter).m_mapAppellationIndex.begin();

		if( j < static_cast<int>( (*iter).m_mapAppellationIndex.size() ) )
		{
			for( int k=0; k<j; k++ )
				iterAppellation++;

			if( (*iter).m_bOpen || (*iterAppellation).second == 1 )
				nAppellationIndex = (*iterAppellation).first;
		}
	}
*/
	if( nAppellationIndex > 0 )
	{
		m_pAppellationTooltip->Show( false );
		m_pAppellationTooltip->SetAppellationInfo( nAppellationIndex );
		GetScreenMouseMovePoints( fMouseX, fMouseY );
		m_pAppellationTooltip->SetPosition( fMouseX, fMouseY );
		SUICoord DlgCoord;
		m_pAppellationTooltip->GetDlgCoord(DlgCoord);
		DlgCoord.fX -= DlgCoord.fWidth / 2;
		DlgCoord.fY += CDnMouseCursor::GetInstance().GetCursorHeight() / 2;
		static float fXRGap(4.0f/DEFAULT_UI_SCREEN_WIDTH);
		if( (DlgCoord.Right()+fXRGap) > GetScreenWidthRatio() )
			DlgCoord.fX -= (DlgCoord.Right()+fXRGap - GetScreenWidthRatio());
		m_pAppellationTooltip->SetDlgCoord(DlgCoord);
		m_pAppellationTooltip->Show( true );
	}
	else
		m_pAppellationTooltip->Show( false );
}

void CDnAppellationBookDlg::ClearPageUI()
{
	m_stUIAppellationPage.m_StaticTitle->SetText( L"" );
	m_stUIAppellationPage.m_StaticDescription->SetText( L"" );
	m_stUIAppellationPage.m_StaticPercent->SetText( L"" );
	m_stUIAppellationPage.m_StaticReward->SetText( L"" );
	m_stUIAppellationPage.m_pStaticClearMark->Show(false);
	
	for(int i = 0 ; i < 2 ; ++i)
	{
		m_stUIAppellationPage.m_pStaticItemSlot[i]->ResetSlot();
		m_stUIAppellationPage.m_pStaticItemCoin[i]->Show(false);
	}

	for( int j=0; j<MAX_APPITEM_PER_PAGE; j++ )
	{
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticIn->Show( false );
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticOut->Show( false );
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewMark->Show( false );

		for(int i = 0 ; i < 2; ++i)
		{
			m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[i]->SetText( L"" );
			m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[i]->Show(false);

			m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[i]->SetText( L"" );
			m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[i]->Show(false);
		}
	}
	m_nRewardCoinValue = 0;
}

void CDnAppellationBookDlg::SetPageUI( int nPage )
{
	if(m_nMaxPage == 0 || m_nMaxPage < nPage)
		return;

	ClearPageUI();

	std::vector<CDnAppellationTask::stAppellationCollection>& pVecAppellationCollection = CDnAppellationTask::GetInstance().GetAppellationCollectionList();
	if( pVecAppellationCollection.size()-1 != m_nMaxPage )
		return;

	std::vector<CDnAppellationTask::stAppellationCollection>::iterator iter = pVecAppellationCollection.begin();
	iter += nPage;

	if( iter == pVecAppellationCollection.end() )
		return;

	DWORD dwTextColor = 0xff000000;
	switch( (*iter).m_nDifficulty )
	{
	case 0:
		dwTextColor = D3DCOLOR_ARGB( 255, 127, 127, 127 );
		break;
	case 1:
		dwTextColor = D3DCOLOR_ARGB( 255, 130, 130, 0 );
		break;
	case 2:
		dwTextColor = D3DCOLOR_ARGB( 255, 40, 135, 165 );
		break;
	case 3:
		dwTextColor = D3DCOLOR_ARGB( 255, 205, 100, 0 );
		break;
	case 4:
		dwTextColor = D3DCOLOR_ARGB( 255, 150, 0, 0 );
		break;
	}
	
	m_stUIAppellationPage.m_StaticTitle->SetTextColor( dwTextColor );
	m_stUIAppellationPage.m_StaticTitle->SetText( (*iter).m_strCollectionTitle );
	m_stUIAppellationPage.m_StaticDescription->SetText( (*iter).m_strCollectionDescription );
	
	WCHAR wszPercentText[64] = {0,};
	swprintf_s( wszPercentText, _countof( wszPercentText ), L"%.0f%%", (*iter).m_fPercent );
	m_stUIAppellationPage.m_StaticPercent->SetText( wszPercentText );
	m_stUIAppellationPage.m_StaticReward->SetText( CDnAppellationTask::GetInstance().GetAppellationName( (*iter).m_nRewardAppellationIndex - 1 ) );

	std::map<int, int>::iterator iterAppellation = (*iter).m_mapAppellationIndex.begin();
	if( (*iter).m_mapAppellationIndex.size() > MAX_APPITEM_PER_PAGE )
		return;
	
	// Completion rate mark
	if( (*iter).m_fPercent >= 100.0f )
		m_stUIAppellationPage.m_pStaticClearMark->Show(true);

	// Item Slot & Coin
	DNTableFileFormat* pRewardSox = GetDNTable( CDnTableDB::TCOLLECTIONBOOK );
	if(!pRewardSox) return;
	int nMailTableID = pRewardSox->GetFieldFromLablePtr( (*iter).m_nTitleID, "_Mail2" )->GetInteger();
	if( nMailTableID > 0 )
	{
		pRewardSox = GetDNTable( CDnTableDB::TMAIL );
		if(!pRewardSox) return;

		// Coin Slot
		bool bIsHaveCoinReward = false; // Does the reward include coins?
		int nRewardCoinID = pRewardSox->GetFieldFromLablePtr(nMailTableID, "_MailPresentMoney")->GetInteger();
		if(nRewardCoinID > 0)
		{
			bIsHaveCoinReward = true;
			m_nRewardCoinValue = nRewardCoinID;
			m_stUIAppellationPage.m_pStaticItemCoin[0]->Show(true);
			m_stUIAppellationPage.m_pStaticItemCoin[0]->Enable(true);
		}

		// Item Slot
		struct sItemSlotData
		{
			int nItemID;
			int nItemCount;
			
			sItemSlotData() : nItemID(0) , nItemCount(0) {}
		};
		std::vector<sItemSlotData> vecItemData;

		for(int i = 0 ; i < 2 ; ++i)
		{
			sItemSlotData tempData;
			
			tempData.nItemID	= pRewardSox->GetFieldFromLablePtr( nMailTableID, FormatA("_MailPresentItem%d",i+1).c_str() )->GetInteger();
			tempData.nItemCount = pRewardSox->GetFieldFromLablePtr( nMailTableID, FormatA("_Count%d",i+1).c_str() )->GetInteger();
			
			if(tempData.nItemID)
				vecItemData.push_back(tempData);
		}

		if( !vecItemData.empty() )
		{
			TItemInfo itemInfo;

			for(int i = 0 ; i < (int)vecItemData.size() ; ++i)
			{
				// If coin is a reward, set only item 1 in slot and ignore item 2.
				if(bIsHaveCoinReward && i == 1)
					break;

				if( CDnItem::MakeItemInfo( vecItemData[i].nItemID, vecItemData[i].nItemCount, itemInfo ) )
				{
					CDnItem* pItem = GetItemTask().CreateItem( itemInfo );
					if(pItem)
					{
						if(bIsHaveCoinReward) // If there is a Coin reward, fix item to slot 2.
							m_stUIAppellationPage.m_pStaticItemSlot[1]->SetItem(pItem, vecItemData[i].nItemCount);
						else
							m_stUIAppellationPage.m_pStaticItemSlot[i]->SetItem(pItem, vecItemData[i].nItemCount);
					}
				}
			}
		}
	}

	for( int j=0; iterAppellation != (*iter).m_mapAppellationIndex.end(); j++, iterAppellation++ )
	{
		// 1. Owned titles
		int nIsExist = E_OUT; // None
		if( (*iterAppellation).second == 1 )
		{
			nIsExist = E_IN; // Exists
			m_stUIAppellationPage.m_AppellationItem[j].m_StaticIn->Show( true );
			m_stUIAppellationPage.m_AppellationItem[j].m_StaticOut->Show( false );
		}
		else
		{
			m_stUIAppellationPage.m_AppellationItem[j].m_StaticOut->Show( true );
			m_stUIAppellationPage.m_AppellationItem[j].m_StaticIn->Show( false );
		}

		// 2. New button
		bool bIsNewAppellation = false;
		CDnAppellationTask::AppellationStruct* pMyAppellation = CDnAppellationTask::GetInstance().GetAppellationInfoByArrayIndex((*iterAppellation).first - 1);
		if(pMyAppellation)
		{
			bIsNewAppellation = pMyAppellation->bNew;
		}
		m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewMark->Show(bIsNewAppellation);
		
		// 3. Hidden mission / Open mission
		if( (*iter).m_bOpen )
		{
			if(bIsNewAppellation)
			{
				m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[nIsExist]->SetText( CDnAppellationTask::GetInstance().GetAppellationName( (*iterAppellation).first - 1 ) );
				m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[nIsExist]->Show(true);
			}
			else
			{
				m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[nIsExist]->SetText( CDnAppellationTask::GetInstance().GetAppellationName( (*iterAppellation).first - 1 ) );
				m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[nIsExist]->Show(true);
			}
		}
		else
		{
			if(nIsExist == E_IN)
			{
				if(bIsNewAppellation)
				{
					m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[nIsExist]->SetText( CDnAppellationTask::GetInstance().GetAppellationName( (*iterAppellation).first - 1 ) );
					m_stUIAppellationPage.m_AppellationItem[j].m_StaticNewInfo[nIsExist]->Show(true);
				}
				else
				{
					m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[nIsExist]->SetText( CDnAppellationTask::GetInstance().GetAppellationName( (*iterAppellation).first - 1 ) );
					m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[nIsExist]->Show(true);
				}
			}
			else
			{
				m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[nIsExist]->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000007374 ) ); // ENUM : ?????????
				m_stUIAppellationPage.m_AppellationItem[j].m_StaticInfo[nIsExist]->Show(true);
			}
		} // end of if
	} // end of for
}

void CDnAppellationBookDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		//m_bIsSelectListBox = false;

		if( IsCmdControl( "ID_BT_PRIOR" ) )
		{
			if( m_nCurrentPage > 0 )
			{
				m_nCurrentPage--;
				SetPageUI( m_nCurrentPage );
				m_pStaticCurrentPage->SetText( FormatW( L"%d", m_nCurrentPage + 1 ) );
			}
		}
		else if( IsCmdControl( "ID_BT_NEXT" ) )
		{
			if( m_nCurrentPage < m_nMaxPage )
			{
				m_nCurrentPage++;
				SetPageUI( m_nCurrentPage );
				m_pStaticCurrentPage->SetText( FormatW( L"%d", m_nCurrentPage + 1 ) );
			}
		}
	}
	else if( nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( strstr( pControl->GetControlName(), "ID_LISTBOXEX_LIST" ) )
		{
			SListBoxItem* pItem = m_pListBoxEx->GetSelectedItem();
			if( pItem )
			{
				CDnAppellationBookContentsDlg* pContentsDlg = static_cast<CDnAppellationBookContentsDlg*>( pItem->pData );
				if(pContentsDlg)
				{
					float fX, fY = .0f;
					GetScreenMouseMovePoints(fX, fY);
					if( pContentsDlg->GetDlgCoord().IsInside(fX, fY) )
					{
						m_nCurrentPage = pContentsDlg->m_nIndexID;
						pContentsDlg->SelectListBox(true);
						SelectContents(pContentsDlg->m_nIndexID);
					}
				}
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnAppellationBookDlg::SelectContents(int nIndex)
{
	if(nIndex > m_nMaxPage || nIndex < 0)
		return;

	SetPageUI(nIndex);
	m_pStaticCurrentPage->SetText( FormatW( L"%d", nIndex + 1 ) );
}

bool CDnAppellationBookDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	bool bRet = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );

	switch( uMsg )
	{
		case WM_MOUSEWHEEL:
		{
			if (IsMouseInDlg() && !m_pListBoxEx->IsFocus())
			{
				UINT uLines;
				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
				int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;

				//Wheel down (-): next page, wheel up (+): previous page
				if ( nScrollAmount != 0 )
				{
					int tempCurrentPage = m_nCurrentPage;

					if( nScrollAmount > 0 )
					{
						if( tempCurrentPage > 0 )
							tempCurrentPage--;
					}
					else
					{
						if( m_nMaxPage > tempCurrentPage )
							tempCurrentPage++;
					}

					if( tempCurrentPage != m_nCurrentPage )
					{
						m_nCurrentPage = tempCurrentPage;
						SetPageUI( m_nCurrentPage );
						m_pStaticCurrentPage->SetText( FormatW( L"%d", m_nCurrentPage + 1 ) );
					}
				}
			}
		}
		break;

		case WM_MOUSEMOVE:
		{
			if( IsMouseInDlg() && m_pListBoxEx->IsFocus() )
			{
				int nCount = m_pListBoxEx->GetSize();
				for(int i = 0 ; i < nCount ; ++i)
				{
					CDnAppellationBookContentsDlg* pContentsDlg = m_pListBoxEx->GetItem<CDnAppellationBookContentsDlg>(i);
					if(pContentsDlg)
					{
						float fX, fY = .0f;
						GetScreenMouseMovePoints(fX, fY);	
						bool bIsMouseOver = pContentsDlg->GetDlgCoord().IsInside(fX, fY);
						if(pContentsDlg->m_nIndexID == m_nCurrentPage) // Skip if it is the selected table of contents entry.
							continue;

						pContentsDlg->SelectListBox(bIsMouseOver);
					}
				}
			}
		}
		break;
	}

	return bRet;
}

#else

//////////////////////////////////////////////////////////////////////////
// CDnAppellationBookDlg

CDnAppellationBookDlg::CDnAppellationBookDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_nCurrentPage( 0 )
, m_nMaxPage( 0 )
, m_pStaticCurrentPage1( NULL )
, m_pStaticCurrentPage2( NULL )
, m_pAppellationTooltip( NULL )
{
	memset( m_stUIAppellationPage, 0, sizeof( stUIAppellationPage ) * 2 );
}

CDnAppellationBookDlg::~CDnAppellationBookDlg()
{
	SAFE_DELETE( m_pAppellationTooltip );
}

void CDnAppellationBookDlg::Initialize(bool bShow)
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "AppellationBookDlg.ui" ).c_str(), bShow );

	m_pAppellationTooltip = new CDnMissionAppellationTooltipDlg( UI_TYPE_CHILD, this );
	m_pAppellationTooltip->Initialize( false );
}

void CDnAppellationBookDlg::InitialUpdate()
{
	CEtUIDialog::InitialUpdate();

	char szStr[64];
	for( int i=0; i<2; i++ )
	{
		sprintf_s( szStr, "ID_TEXT_TITLE%d", i );
		m_stUIAppellationPage[i].m_StaticTitle = GetControl<CEtUIStatic>( szStr );
		sprintf_s( szStr, "ID_TEXT_SUBTITLE%d", i );
		m_stUIAppellationPage[i].m_StaticDescription = GetControl<CEtUIStatic>( szStr );
		sprintf_s( szStr, "ID_TEXT_PERCENT%d", i );
		m_stUIAppellationPage[i].m_StaticPercent = GetControl<CEtUIStatic>( szStr );
		sprintf_s( szStr, "ID_TEXT_REWARDNAME%d", i );
		m_stUIAppellationPage[i].m_StaticReward = GetControl<CEtUIStatic>( szStr );

		for( int j=0; j<MAX_APPITEM_PER_PAGE; j++ )
		{
			sprintf_s( szStr, "ID_PAGE%d_IN%d", i, j );
			m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticIn = GetControl<CEtUIStatic>( szStr );
			m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticIn->Show( false );
			sprintf_s( szStr, "ID_PAGE%d_OUT%d", i, j );
			m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticOut = GetControl<CEtUIStatic>( szStr );
			m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticOut->Show( false );
			sprintf_s( szStr, "ID_TEXT%d_IN%d", i, j );
			m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticInfo = GetControl<CEtUIStatic>( szStr );
		}
	}

	m_pStaticCurrentPage1 = GetControl<CEtUIStatic>( "ID_TEXT_PAGE0" );
	m_pStaticCurrentPage1->SetText( FormatW( L"%d", m_nCurrentPage + 1 ) );
	m_pStaticCurrentPage2 = GetControl<CEtUIStatic>( "ID_TEXT_PAGE1" );
	m_pStaticCurrentPage2->SetText( FormatW( L"%d", m_nCurrentPage + 2 ) );	
}

void CDnAppellationBookDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		CDnAppellationTask::GetInstance().UpdateMyAppellationCollectionList();
		SetPageUI( m_nCurrentPage );
	}

	CEtUIDialog::Show( bShow );
}

void CDnAppellationBookDlg::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );

	if(!IsShow())
		return;

	float fMouseX, fMouseY;
	GetScreenMouseMovePoints( fMouseX, fMouseY );

	fMouseX -= GetXCoord();
	fMouseY -= GetYCoord();

	bool bShowTooltip = false;
	for( int i=0; i<2; i++ )
	{
		bool bMouseEnter( false );
		SUICoord uiCoord;
		m_stUIAppellationPage[i].m_StaticReward->GetUICoord( uiCoord );
		if( uiCoord.IsInside( fMouseX, fMouseY ) )
		{
			ShowAppellationTooltip( fMouseX, fMouseY, i + m_nCurrentPage );
			bShowTooltip = true;
			break;
		}
/*		
		for( int j=0; j<MAX_APPITEM_PER_PAGE; j++ )
		{
			m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticInfo->GetUICoord( uiCoord );
			if( uiCoord.IsInside( fMouseX, fMouseY ) )
			{
				ShowAppellationTooltip( fMouseX, fMouseY, i + m_nCurrentPage, j );
				bShowTooltip = true;
				break;
			}
		}

		if( bShowTooltip )
			break;
*/
	}

	if( !bShowTooltip )
		m_pAppellationTooltip->Show( false );
}

void CDnAppellationBookDlg::ShowAppellationTooltip( float fMouseX, float fMouseY, int i, int j )
{
	std::vector<CDnAppellationTask::stAppellationCollection>& pVecAppellationCollection = CDnAppellationTask::GetInstance().GetAppellationCollectionList();
	std::vector<CDnAppellationTask::stAppellationCollection>::iterator iter = pVecAppellationCollection.begin();

	if( i >= static_cast<int>( pVecAppellationCollection.size() ) )
		return;

	int nAppellationIndex = 0;
	if( j == -1 )	// Reward title tooltip
	{
		iter += i;
		nAppellationIndex = iter->m_nRewardAppellationIndex;
	}
/*	else // Collection title tooltip
	{
		iter += i;
		std::map<int, int>::iterator iterAppellation = (*iter).m_mapAppellationIndex.begin();

		if( j < static_cast<int>( (*iter).m_mapAppellationIndex.size() ) )
		{
			for( int k=0; k<j; k++ )
				iterAppellation++;

			if( (*iter).m_bOpen || (*iterAppellation).second == 1 )
				nAppellationIndex = (*iterAppellation).first;
		}
	}
*/
	if( nAppellationIndex > 0 )
	{
		m_pAppellationTooltip->Show( false );
		m_pAppellationTooltip->SetAppellationInfo( nAppellationIndex );
		GetScreenMouseMovePoints( fMouseX, fMouseY );
		m_pAppellationTooltip->SetPosition( fMouseX, fMouseY );
		SUICoord DlgCoord;
		m_pAppellationTooltip->GetDlgCoord(DlgCoord);
		DlgCoord.fX -= DlgCoord.fWidth / 2;
		DlgCoord.fY += CDnMouseCursor::GetInstance().GetCursorHeight() / 2;
		static float fXRGap(4.0f/DEFAULT_UI_SCREEN_WIDTH);
		if( (DlgCoord.Right()+fXRGap) > GetScreenWidthRatio() )
			DlgCoord.fX -= (DlgCoord.Right()+fXRGap - GetScreenWidthRatio());
		m_pAppellationTooltip->SetDlgCoord(DlgCoord);
		m_pAppellationTooltip->Show( true );
	}
	else
		m_pAppellationTooltip->Show( false );
}

void CDnAppellationBookDlg::ClearPageUI()
{
	for( int i=0; i<2; i++ )
	{
		m_stUIAppellationPage[i].m_StaticTitle->SetText( L"" );
		m_stUIAppellationPage[i].m_StaticDescription->SetText( L"" );
		m_stUIAppellationPage[i].m_StaticPercent->SetText( L"" );
		m_stUIAppellationPage[i].m_StaticReward->SetText( L"" );

		for( int j=0; j<MAX_APPITEM_PER_PAGE; j++ )
		{
			m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticIn->Show( false );
			m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticOut->Show( false );
			m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticInfo->SetText( L"" );
		}
	}
}

void CDnAppellationBookDlg::SetPageUI( int nPage )
{
	if( m_nMaxPage == 0 || nPage >= m_nMaxPage )
		return;

	ClearPageUI();

	std::vector<CDnAppellationTask::stAppellationCollection>& pVecAppellationCollection = CDnAppellationTask::GetInstance().GetAppellationCollectionList();

	if( pVecAppellationCollection.size() != m_nMaxPage )
		return;

	std::vector<CDnAppellationTask::stAppellationCollection>::iterator iter = pVecAppellationCollection.begin();
	iter += nPage;

	for( int i=0; i<2; i++, iter++ )
	{
		if( iter == pVecAppellationCollection.end() )
			break;

		DWORD dwTextColor = 0xff000000;
		switch( (*iter).m_nDifficulty )
		{
			case 0:
				dwTextColor = D3DCOLOR_ARGB( 255, 127, 127, 127 );
				break;
			case 1:
				dwTextColor = D3DCOLOR_ARGB( 255, 130, 130, 0 );
				break;
			case 2:
				dwTextColor = D3DCOLOR_ARGB( 255, 40, 135, 165 );
				break;
			case 3:
				dwTextColor = D3DCOLOR_ARGB( 255, 205, 100, 0 );
				break;
			case 4:
				dwTextColor = D3DCOLOR_ARGB( 255, 150, 0, 0 );
				break;
		}

		m_stUIAppellationPage[i].m_StaticTitle->SetTextColor( dwTextColor );
		m_stUIAppellationPage[i].m_StaticTitle->SetText( (*iter).m_strCollectionTitle );
		m_stUIAppellationPage[i].m_StaticDescription->SetText( (*iter).m_strCollectionDescription );
		WCHAR wszPercentText[64] = {0,};
		swprintf_s( wszPercentText, _countof( wszPercentText ), L"%.0f%%", (*iter).m_fPercent );
		m_stUIAppellationPage[i].m_StaticPercent->SetText( wszPercentText );
		m_stUIAppellationPage[i].m_StaticReward->SetText( CDnAppellationTask::GetInstance().GetAppellationName( (*iter).m_nRewardAppellationIndex - 1 ) );

		std::map<int, int>::iterator iterAppellation = (*iter).m_mapAppellationIndex.begin();
		
		if( (*iter).m_mapAppellationIndex.size() > MAX_APPITEM_PER_PAGE )
			break;

		for( int j=0; iterAppellation != (*iter).m_mapAppellationIndex.end(); j++, iterAppellation++ )
		{
			if( (*iterAppellation).second == 1 )
			{
				m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticIn->Show( true );
				m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticOut->Show( false );
			}
			else
			{
				m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticOut->Show( true );
				m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticIn->Show( false );
			}

			if( (*iter).m_bOpen )
				m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticInfo->SetText( CDnAppellationTask::GetInstance().GetAppellationName( (*iterAppellation).first - 1 ) );
			else
			{
				if( (*iterAppellation).second == 1 )
					m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticInfo->SetText( CDnAppellationTask::GetInstance().GetAppellationName( (*iterAppellation).first - 1 ) );
				else
					m_stUIAppellationPage[i].m_AppellationItem[j].m_StaticInfo->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000007374 ) );
			}
		}
	}
}

void CDnAppellationBookDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_PRIOR" ) )
		{
			if( m_nCurrentPage >= 2 )
			{
				m_nCurrentPage -= 2;
				// Set UI
				SetPageUI( m_nCurrentPage );
				m_pStaticCurrentPage1->SetText( FormatW( L"%d", m_nCurrentPage + 1 ) );
				m_pStaticCurrentPage2->SetText( FormatW( L"%d", m_nCurrentPage + 2 ) );
			}
		}
		else if( IsCmdControl( "ID_BT_NEXT" ) )
		{
			if( m_nMaxPage > m_nCurrentPage + 2 )
			{
				m_nCurrentPage += 2;
				// Set UI
				SetPageUI( m_nCurrentPage );
				m_pStaticCurrentPage1->SetText( FormatW( L"%d", m_nCurrentPage + 1 ) );
				m_pStaticCurrentPage2->SetText( FormatW( L"%d", m_nCurrentPage + 2 ) );
			}
		}
	}
}

bool CDnAppellationBookDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	bool bRet = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
	switch( uMsg )
	{
		case WM_MOUSEWHEEL:
		{
			if ( IsMouseInDlg() )
			{
				UINT uLines;
				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
				int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;

				//Wheel down (-): next page, wheel up (+): previous page
				if ( nScrollAmount != 0 )
				{
					int tempCurrentPage = m_nCurrentPage;

					if( nScrollAmount > 0 )
					{
						if( tempCurrentPage >= 2 )
							tempCurrentPage -= 2;
					}
					else
					{
						if( m_nMaxPage > tempCurrentPage + 2 )
							tempCurrentPage += 2;
					}

					if( tempCurrentPage != m_nCurrentPage )
					{
						m_nCurrentPage = tempCurrentPage;
						SetPageUI( m_nCurrentPage );
						m_pStaticCurrentPage1->SetText( FormatW( L"%d", m_nCurrentPage + 1 ) );
						m_pStaticCurrentPage2->SetText( FormatW( L"%d", m_nCurrentPage + 2 ) );
					}
				}
			}
		}
		break;
	}

	return bRet;
}

#endif //PRE_MOD_APPELLATIONBOOK_RENEWAL
