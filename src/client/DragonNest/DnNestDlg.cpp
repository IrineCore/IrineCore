#include "StdAfx.h"
#include "DnNestDlg.h"
#include "DnNestInfoTask.h"
#include "DnTableDB.h"
#include "DnCommonTask.h"
#include "TaskManager.h"
#include "DnItemTask.h"
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#include "DnInterface.h"
#endif


CDnNestDlg::CDnNestDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	m_pTreeList = NULL;
}

CDnNestDlg::~CDnNestDlg()
{
}

void CDnNestDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonNestDlg.ui" ).c_str(), bShow );
}

void CDnNestDlg::InitialUpdate()
{
	m_pTreeList = GetControl<CEtUITreeCtl>( "ID_TREE_LIST" );
}

void CDnNestDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow ) {
		RefreshStageList();
	}
	else {
		m_pTreeList->DeleteAllItems();
	}

	CEtUIDialog::Show( bShow );
}

void CDnNestDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if(nCommand == EVENT_BUTTON_CLICKED)
	{
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_STAGE);
		}
#endif

	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnNestDlg::RefreshList()
{
	m_pTreeList->DeleteAllItems();

	CTreeItem *pNest = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1372 ), textcolor::FONT_GREEN );
	CTreeItem *pEvent = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100019124 ), textcolor::FONT_GREEN );
	CTreeItem *pChaos = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000032407 ), textcolor::FONT_GREEN );
	CTreeItem *pDarkLair = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000003489 ), textcolor::FONT_GREEN );

	WCHAR wszStr[256];
	WCHAR wszTemp[256];
	DWORD dwCount = CDnNestInfoTask::GetInstance().GetNestDungeonCount();
	for( DWORD i=0; i<dwCount; i++ ) {
		CDnNestInfoTask::NestDungeonInfoStruct *pStruct = CDnNestInfoTask::GetInstance().GetNestDungeonInfo(i);
		if( !pStruct ) continue;
		CTreeItem *pParent = NULL;
		CTreeItem::eClassIconType IconType = (CTreeItem::eClassIconType)8;
		DWORD dwColor;

		int nMaxTryCount = pStruct->nMaxTryCount;
		switch( pStruct->SubType ) {
			case CDnWorld::MapSubTypeNest:
				pParent = pNest;
				if (pStruct->bExpandable)
					nMaxTryCount += CDnNestInfoTask::GetInstance().GetExpandTryCount();
				break;

			case CDnWorld::MapSubTypeEvent: pParent = pEvent; break;
			case CDnWorld::MapSubTypeChaosField: pParent = pChaos; break;
			case CDnWorld::MapSubTypeDarkLair: pParent = pDarkLair; break;
		}
		if( pParent == NULL ) continue;

		int nTryCount = pStruct->nTryCount;
		if( nMaxTryCount <= pStruct->nTryCount ) {
			nTryCount = nMaxTryCount;
			IconType = (CTreeItem::eClassIconType)8;
			dwColor = textcolor::FONT_ALERT;
		}
		else {
			IconType = (CTreeItem::eClassIconType)9;
			dwColor = textcolor::WHITE;
		}

		swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 820 ), nMaxTryCount - nTryCount, nTryCount, nMaxTryCount );
		swprintf_s( wszStr, L"%s   %s", pStruct->szMapName.c_str(), wszTemp );
		CTreeItem *pItem = m_pTreeList->AddChildItem( pParent, CTreeItem::typeOpen, wszStr, dwColor );
		pItem->SetClassIconType( IconType );
	}

	if( !pEvent->HasChild() ) m_pTreeList->DeleteItem( pEvent );
	if( !pChaos->HasChild() ) m_pTreeList->DeleteItem( pChaos );
	if( !pDarkLair->HasChild() ) m_pTreeList->DeleteItem( pDarkLair );
}

bool CDnNestDlg::IsStageLevelEnable( int nEnterTableID )
{
	if( !CDnActor::s_hLocalActor )
		return false;

	if( 1 > nEnterTableID )
		return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
	if( NULL == pSox )
		return false;

	const int nPlayerLevel = CDnActor::s_hLocalActor->GetLevel();

	const int nMinLevel = pSox->GetFieldFromLablePtr( nEnterTableID, "_LvlMin" )->GetInteger();
	const int nMaxLevel = pSox->GetFieldFromLablePtr( nEnterTableID, "_LvlMax" )->GetInteger();

	if( nMinLevel > nPlayerLevel )
		return false;

	if( -1 == nMaxLevel )
		return true;
	
	if( nMaxLevel < nPlayerLevel )
		return false;

	return true;
}

void CDnNestDlg::RefreshStageList()
{
	m_pTreeList->DeleteAllItems();

	CTreeItem *pPCBangNest = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1384 ), textcolor::VIOLET );
	CTreeItem *pNest = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1372 ), textcolor::FONT_GREEN );
	CTreeItem *pEvent = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100019124 ), textcolor::FONT_GREEN );
	CTreeItem *pChaos = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000032407 ), textcolor::FONT_GREEN );
	CTreeItem *pDarkLair = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000003489 ), textcolor::FONT_GREEN );
#if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
	CTreeItem *pTrasureStage = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000046961 ), textcolor::FONT_GREEN );
#endif	// #if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
#ifdef PRE_ADD_DRAGON_FELLOWSHIP
	CTreeItem *pFellowshipStage = m_pTreeList->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10153 ), textcolor::FONT_GREEN );
#endif // PRE_ADD_DRAGON_FELLOWSHIP

	std::vector< sStageValue > vEnableNest, vLaterEnableNest, vDisableNest;
	std::vector< sStageValue > vEnableEvent, vLaterEnableEvent, vDisableEvent;
	std::vector< sStageValue > vEnableChaos, vLaterEnableChaos, vDisableChaos;
	std::vector< sStageValue > vEnableDarkLair, vLaterEnableDarkLair, vDisableDarkLair;
#ifdef PRE_ADD_SUBTYPE_TREASURESTAGE
	std::vector< sStageValue > vEnableTreasure, vLaterEnableTreasure, vDisableTreasure;
#endif // PRE_ADD_SUBTYPE_TREASURESTAGE
#ifdef PRE_ADD_DRAGON_FELLOWSHIP
	std::vector< sStageValue > vEnableFellowship, vLaterEnableFellowship, vDisableFellowship;
#endif // PRE_ADD_DRAGON_FELLOWSHIP

	DWORD dwCount = CDnNestInfoTask::GetInstance().GetNestDungeonCount();

	for( DWORD itr = 0; itr < dwCount; ++itr )
	{
		sProcessData sData;
		sStageValue sValue;

		if( false == InitializeData( itr, sData, sValue ) )
			continue;

		switch( sData.m_nStageType )
		{
			case CDnWorld::MapSubTypeNest:
				if( eStage_Level_Enable == sData.m_eStageLevelCondition && eStage_TryCount_Enable == sData.m_eStageTryCount )
					vEnableNest.push_back( sValue );
				else if( eStage_Level_Min_Disable == sData.m_eStageLevelCondition || eStage_TryCount_Disable == sData.m_eStageTryCount )
					vLaterEnableNest.push_back( sValue );
				else
					vDisableNest.push_back( sValue );
				break;

			case CDnWorld::MapSubTypeEvent:
				if( eStage_Level_Enable == sData.m_eStageLevelCondition && eStage_TryCount_Enable == sData.m_eStageTryCount )
					vEnableEvent.push_back( sValue );
				else if( eStage_Level_Min_Disable == sData.m_eStageLevelCondition || eStage_TryCount_Disable == sData.m_eStageTryCount )
					vLaterEnableEvent.push_back( sValue );
				else
					vDisableEvent.push_back( sValue );
				break;

			case CDnWorld::MapSubTypeChaosField:
				if( eStage_Level_Enable == sData.m_eStageLevelCondition && eStage_TryCount_Enable == sData.m_eStageTryCount )
					vEnableChaos.push_back( sValue );
				else if( eStage_Level_Min_Disable == sData.m_eStageLevelCondition || eStage_TryCount_Disable == sData.m_eStageTryCount )
					vLaterEnableChaos.push_back( sValue );
				else
					vDisableChaos.push_back( sValue );
				break;

			case CDnWorld::MapSubTypeDarkLair:
				if( eStage_Level_Enable == sData.m_eStageLevelCondition && eStage_TryCount_Enable == sData.m_eStageTryCount )
					vEnableDarkLair.push_back( sValue );
				else if( eStage_Level_Min_Disable == sData.m_eStageLevelCondition || eStage_TryCount_Disable == sData.m_eStageTryCount )
					vLaterEnableDarkLair.push_back( sValue );
				else
					vDisableDarkLair.push_back( sValue );
				break;

#if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
			case CDnWorld::MapSubTypeTreasureStage:
				if( eStage_Level_Enable == sData.m_eStageLevelCondition && eStage_TryCount_Enable == sData.m_eStageTryCount )
					vEnableTreasure.push_back( sValue );
				else if( eStage_Level_Min_Disable == sData.m_eStageLevelCondition || eStage_TryCount_Disable == sData.m_eStageTryCount )
					vLaterEnableTreasure.push_back( sValue );
				else
					vDisableTreasure.push_back( sValue );
				break;
#endif	// #if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
#ifdef PRE_ADD_DRAGON_FELLOWSHIP
			case CDnWorld::MapSubTypeFellowship:
				if( eStage_Level_Enable == sData.m_eStageLevelCondition && eStage_TryCount_Enable == sData.m_eStageTryCount )
					vEnableFellowship.push_back( sValue );
				else if( eStage_Level_Min_Disable == sData.m_eStageLevelCondition || eStage_TryCount_Disable == sData.m_eStageTryCount )
					vLaterEnableFellowship.push_back( sValue );
				else
					vDisableFellowship.push_back( sValue );
				break;
#endif // PRE_ADD_DRAGON_FELLOWSHIP
			default:
				_ASSERT( "Can not find the type of dungeon!!" );
				break;
		}
	}

	AddTreeItem( pNest, vEnableNest, vLaterEnableNest, vDisableNest );
	AddTreeItem( pEvent, vEnableEvent, vLaterEnableEvent, vDisableEvent );
	AddTreeItem( pChaos, vEnableChaos, vLaterEnableChaos, vDisableChaos );
	AddTreeItem( pDarkLair, vEnableDarkLair, vLaterEnableDarkLair, vDisableDarkLair );
#if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
	AddTreeItem( pTrasureStage, vEnableTreasure, vLaterEnableTreasure, vDisableTreasure );
#endif	// #if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
#ifdef PRE_ADD_DRAGON_FELLOWSHIP
	AddTreeItem( pFellowshipStage, vEnableFellowship, vLaterEnableFellowship, vDisableFellowship );
#endif // PRE_ADD_DRAGON_FELLOWSHIP

	SetPCBangNestInfo();
}

void CDnNestDlg::SetPCBangNestInfo()
{
	CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
	if( pTask == NULL )
		return;

	if( m_pTreeList == NULL )
		return;

	CTreeItem* pPCBangNest = m_pTreeList->GetBeginItem();
	if( pPCBangNest == NULL )
		return;

	if( !pTask->IsPcBang() )	// Do not display if not PCBang.
	{
		m_pTreeList->DeleteItem( pPCBangNest );
		return;
	}

	std::vector< sStageValue > vEnablePCBangNest, vLaterEnablePCBangNest, vDisablePCBangNest;

	DWORD dwCount = CDnNestInfoTask::GetInstance().GetNestDungeonCount();
	for( DWORD i=0; i<dwCount; i++ ) 
	{
		sProcessData sData;
		sStageValue sValue;

		CDnNestInfoTask::NestDungeonInfoStruct *pStruct = CDnNestInfoTask::GetInstance().GetNestDungeonInfo( i );
		if( !pStruct )	continue;
		if( !pStruct->bPCBangAdd ) continue;

		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
		if( NULL == pSox ) continue;

		sData.m_nMaxLevel = pSox->GetFieldFromLablePtr( pStruct->nEnterTableID, "_LvlMax" )->GetInteger();
		sData.m_nMinLevel = pSox->GetFieldFromLablePtr( pStruct->nEnterTableID, "_LvlMin" )->GetInteger();

		sData.m_nMaxTryCount = pStruct->nPCBangAddCount;
		if( pStruct->nPCBangClearCount > pStruct->nPCBangAddCount )
			sData.m_nTryCount = pStruct->nPCBangAddCount;
		else
			sData.m_nTryCount = pStruct->nPCBangClearCount;
		sData.m_nStageType = pStruct->SubType;
		sData.m_szMapName = pStruct->szMapName;

		sValue.m_nSortNumber = pSox->GetFieldFromLablePtr( pStruct->nEnterTableID, "_ListSort" )->GetInteger();

		ProcessData( sData, sValue );

		if( eStage_Level_Enable == sData.m_eStageLevelCondition && eStage_TryCount_Enable == sData.m_eStageTryCount )
			vEnablePCBangNest.push_back( sValue );
		else if( eStage_Level_Min_Disable == sData.m_eStageLevelCondition || eStage_TryCount_Disable == sData.m_eStageTryCount )
			vLaterEnablePCBangNest.push_back( sValue );
		else
			vDisablePCBangNest.push_back( sValue );
	}

	if( static_cast<int>( vEnablePCBangNest.size() ) == 0
		&& static_cast<int>( vLaterEnablePCBangNest.size() ) == 0 
		&& static_cast<int>( vDisablePCBangNest.size() ) == 0 )
	{
		m_pTreeList->DeleteItem( pPCBangNest );
	}
	else
	{
		AddTreeItem( pPCBangNest, vEnablePCBangNest, vLaterEnablePCBangNest, vDisablePCBangNest );
	}
}

bool CDnNestDlg::InitializeData( const DWORD nIndex, CDnNestDlg::sProcessData & sData, sStageValue & sValue )
{
	CDnNestInfoTask::NestDungeonInfoStruct *pStruct = CDnNestInfoTask::GetInstance().GetNestDungeonInfo( nIndex );
	if( !pStruct )
		return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
	if( NULL == pSox )
		return false;

	sData.m_nMaxLevel = pSox->GetFieldFromLablePtr( pStruct->nEnterTableID, "_LvlMax" )->GetInteger();
	sData.m_nMinLevel = pSox->GetFieldFromLablePtr( pStruct->nEnterTableID, "_LvlMin" )->GetInteger();

	sData.m_nMaxTryCount = pStruct->nMaxTryCount;

	if( CDnWorld::MapSubTypeNest == pStruct->SubType && pStruct->bExpandable )
		sData.m_nMaxTryCount += CDnNestInfoTask::GetInstance().GetExpandTryCount();

	sData.m_nTryCount = pStruct->nTryCount;
	sData.m_nStageType = pStruct->SubType;
	sData.m_szMapName = pStruct->szMapName;

	sValue.m_nSortNumber = pSox->GetFieldFromLablePtr( pStruct->nEnterTableID, "_ListSort" )->GetInteger();

#if defined( PRE_ADD_TSCLEARCOUNTEX )
	SetAddStageTryCount( sData );
#endif	// #if defined( PRE_ADD_TSCLEARCOUNTEX )

	ProcessData( sData, sValue );

	return true;
}

void CDnNestDlg::ProcessData( CDnNestDlg::sProcessData & sData, sStageValue & sValue )
{
	SetStageTryCount( sData );
	SetStageLevelCondition( sData );
	SetString( sData, sValue );
}

void CDnNestDlg::SetStageTryCount( CDnNestDlg::sProcessData & sData )
{
	sData.m_eStageTryCount = eStage_TryCount_Enable;

	if( sData.m_nMaxTryCount <= sData.m_nTryCount )
		sData.m_eStageTryCount = eStage_TryCount_Disable;
}

#if defined( PRE_ADD_TSCLEARCOUNTEX )
void CDnNestDlg::SetAddStageTryCount( CDnNestDlg::sProcessData & sData )
{
	if( CDnWorld::MapSubTypeTreasureStage == sData.m_nStageType )
	{
		std::vector<CDnItem *> vItems;
		CDnItemTask::GetInstance().FindItemFromItemType( ITEMTYPE_EXPAND_TS, ST_INVENTORY_CASH, vItems );
		if( vItems.empty() )
			return;

		int nTypeParam = vItems[0]->GetTypeParam( 0 );
		sData.m_nMaxTryCount += nTypeParam;
	}
}
#endif	// #if defined( PRE_ADD_TSCLEARCOUNTEX )

void CDnNestDlg::SetStageLevelCondition( CDnNestDlg::sProcessData & sData )
{
	sData.m_eStageLevelCondition = eStage_Level_Enable;

	if( !CDnActor::s_hLocalActor )
		return;

	const int nPlayerLevel = CDnActor::s_hLocalActor->GetLevel();

	if( sData.m_nMinLevel > nPlayerLevel )
		sData.m_eStageLevelCondition = eStage_Level_Min_Disable;

	if( sData.m_nMaxLevel != -1 && sData.m_nMaxLevel < nPlayerLevel )
		sData.m_eStageLevelCondition = eStage_Level_Max_Disable;
}

void CDnNestDlg::SetString( CDnNestDlg::sProcessData & sData, CDnNestDlg::sStageValue & sValue )
{
	WCHAR wszString[256];
	WCHAR wszTemp[256];

	if( eStage_Level_Min_Disable == sData.m_eStageLevelCondition )
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1006027 ), sData.m_nMinLevel );	// UISTRING : Enterable at level %d
	else if( eStage_Level_Max_Disable == sData.m_eStageLevelCondition )
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1006026 ) );	// UISTRING : Entry unavailable
	else
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 820 ), sData.m_nMaxTryCount - sData.m_nTryCount, sData.m_nTryCount, sData.m_nMaxTryCount );	// UISTRING : Can clear %d time(s). (%d/%d)

	swprintf_s( wszString, _countof(wszString), L"%s  %s", sData.m_szMapName.c_str(), wszTemp );
	sValue.m_wszString = std::wstring( wszString );

	if( eStage_Level_Enable != sData.m_eStageLevelCondition )
	{
		sValue.m_nIconIndex = 10;
		sValue.m_dwColor = textcolor::DARKGRAY;
	}
	else if( eStage_TryCount_Enable != sData.m_eStageTryCount )
	{
		sValue.m_nIconIndex = 8;
		sValue.m_dwColor = textcolor::FONT_ALERT;
	}
	else
	{
		sValue.m_nIconIndex = 9;
		sValue.m_dwColor = textcolor::WHITE;
	}
}

void CDnNestDlg::AddTreeItem( CTreeItem * pParent, std::vector< CDnNestDlg::sStageValue > & vEnableStage, std::vector< CDnNestDlg::sStageValue > & vLaterEnableStage, std::vector< CDnNestDlg::sStageValue > & vDisableStage )
{
	std::stable_sort( vEnableStage.begin(), vEnableStage.end(), CompareStage );
	std::stable_sort( vLaterEnableStage.begin(), vLaterEnableStage.end(), CompareStage );
	std::stable_sort( vDisableStage.begin(), vDisableStage.end(), CompareStage );

	for( DWORD itr = 0; itr < vEnableStage.size(); ++itr )
	{
		CTreeItem *pItem = m_pTreeList->AddChildItem( pParent, CTreeItem::typeOpen, vEnableStage[itr].m_wszString.c_str(), vEnableStage[itr].m_dwColor );	
		pItem->SetClassIconType( (CTreeItem::eClassIconType)vEnableStage[itr].m_nIconIndex );
	}

	for( DWORD itr = 0; itr < vLaterEnableStage.size(); ++itr )
	{
		CTreeItem *pItem = m_pTreeList->AddChildItem( pParent, CTreeItem::typeOpen, vLaterEnableStage[itr].m_wszString.c_str(), vLaterEnableStage[itr].m_dwColor );	
		pItem->SetClassIconType( (CTreeItem::eClassIconType)vLaterEnableStage[itr].m_nIconIndex );
	}

	for( DWORD itr = 0; itr < vDisableStage.size(); ++itr )
	{
		CTreeItem *pItem = m_pTreeList->AddChildItem( pParent, CTreeItem::typeOpen, vDisableStage[itr].m_wszString.c_str(), vDisableStage[itr].m_dwColor );	
		pItem->SetClassIconType( (CTreeItem::eClassIconType)vDisableStage[itr].m_nIconIndex );
	}

	if( false == pParent->HasChild() )
		m_pTreeList->DeleteItem( pParent );
}

bool CDnNestDlg::CompareStage( const CDnNestDlg::sStageValue & s1, const CDnNestDlg::sStageValue & s2 )
{
	if( s1.m_nSortNumber > s2.m_nSortNumber )
		return false;

	return true;
}