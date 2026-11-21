#include "StdAfx.h"
#include "DnDungeonExpectMissionDlg.h"
#include "DnTableDB.h"
#include "DnMissionTask.h"
#include "DnQuestTask.h"
#include "DnMissionTask.h"
#include "DnUIString.h"
#include "DnMissionAppellationTooltipDlg.h"
#include "DnMissionRewardCoinTooltipDlg.h"
#include "DnCashShopTask.h"
#include "DnItem.h"
#include "DnItemTask.h"
#include "TaskManager.h"
#include "DnWorld.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 


CDnDungeonExpectMissionDlg::CDnDungeonExpectMissionDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pMissionListBox( NULL )
, m_pDnDungeonExpectMissionRewardDlg( NULL )
{
}

CDnDungeonExpectMissionDlg::~CDnDungeonExpectMissionDlg(void)
{
	m_pMissionListBox->RemoveAllItems();
	SAFE_DELETE( m_pDnDungeonExpectMissionRewardDlg );
}

void CDnDungeonExpectMissionDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterTabListDlg.ui" ).c_str(), bShow );
}

void CDnDungeonExpectMissionDlg::InitialUpdate()
{
	m_pMissionListBox = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIST" );

	m_pDnDungeonExpectMissionRewardDlg = new CDnDungeonExpectMissionRewardDlg( UI_TYPE_CHILD, this );
	m_pDnDungeonExpectMissionRewardDlg->Initialize( false );
}

void CDnDungeonExpectMissionDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );
}

void CDnDungeonExpectMissionDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( strstr( pControl->GetControlName(), "ID_LISTBOXEX_LIST" ) )
		{
			SListBoxItem* pItem = m_pMissionListBox->GetSelectedItem();
			if( pItem )
			{
				CDnDungeonExpectMissionItemDlg* pDnDungeonExpectMissionItemDlg = static_cast<CDnDungeonExpectMissionItemDlg*>( pItem->pData );

				if( pDnDungeonExpectMissionItemDlg && pDnDungeonExpectMissionItemDlg->GetMissionInfo() != NULL )
				{
					if( m_pDnDungeonExpectMissionRewardDlg->SetMissionInfo( pDnDungeonExpectMissionItemDlg->GetMissionInfo() ) )	// Hide if neither title nor reward exists
					{
						SUICoord uiCoordReward, uiCoordItem;
						m_pDnDungeonExpectMissionRewardDlg->GetDlgCoord( uiCoordReward );
						pDnDungeonExpectMissionItemDlg->GetDlgCoord( uiCoordItem );
						uiCoordReward.fX = uiCoordItem.fX - uiCoordReward.fWidth;
						uiCoordReward.fY = uiCoordItem.fY;
						m_pDnDungeonExpectMissionRewardDlg->SetDlgCoord( uiCoordReward );

						ShowChildDialog( m_pDnDungeonExpectMissionRewardDlg, true );
					}
				}
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDungeonExpectMissionDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		ShowChildDialog( m_pDnDungeonExpectMissionRewardDlg, false );
		m_pMissionListBox->DeselectItem();
	}

	CEtUIDialog::Show( bShow );
}

bool CDnDungeonExpectMissionDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( m_pDnDungeonExpectMissionRewardDlg->IsShow() )
	{
		if( ( uMsg == WM_LBUTTONDOWN && !m_pDnDungeonExpectMissionRewardDlg->IsMouseInDlg() ) || uMsg == WM_MOUSEWHEEL )
		{
			m_pDnDungeonExpectMissionRewardDlg->Show( false );
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnDungeonExpectMissionDlg::SetAvailableMission( DNTableFileFormat*  pSox, CDnMissionTask::MissionInfoStruct* pMissionInfo, int nMapIndex, int nDifficult )
{
	if( pSox == NULL || pMissionInfo == NULL )
		return;

	DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
	if( !pMapSox )
		return;

	bool bFindMission = false;

	bFindMission = CheckAvailableMission( pSox, pMissionInfo, nMapIndex, nDifficult );

#ifdef PRE_ADD_RENEW_RANDOM_MAP
	if( bFindMission == false )
	{
		DNTableCell* pField = pMapSox->GetFieldFromLablePtr( nMapIndex , "_MapNameID");
		int nMapTableStringId = pField->GetInteger();
		const std::vector<int>* pMapGroupList = CDnWorldDataManager::GetInstance().GetMapGroupByName( nMapTableStringId );

		std::vector<int>::const_iterator iter = pMapGroupList->begin();
		for( iter = pMapGroupList->begin(); iter != pMapGroupList->end(); ++iter )
		{
			int nGroupMapIndex = (*iter);
			bFindMission = CheckAvailableMission( pSox, pMissionInfo, nGroupMapIndex, nDifficult );
			if( bFindMission == true ) break;
		}
	}
#endif

	if( bFindMission )
	{
		CDnDungeonExpectMissionItemDlg* pItem = m_pMissionListBox->AddItem<CDnDungeonExpectMissionItemDlg>();
		if( pItem )
			pItem->SetMissionInfo( pMissionInfo );
#ifdef PRE_MOD_MISSION_HELPER
		GetMissionTask().AddDungeonExpectMission( pMissionInfo );
#endif
	}
}

bool CDnDungeonExpectMissionDlg::CheckAvailableMission( DNTableFileFormat*  pSox, CDnMissionTask::MissionInfoStruct* pMissionInfo, int nMapIndex, int nDifficult )
{
	DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
	if( !pMapSox )
		return false;

	bool bFindMap = false;
	bool bFindMission = false;

	bool bDarkLairMap = false;
	std::vector<int> vecDarLairMapList;

	int nSubMapType = pMapSox->GetFieldFromLablePtr( nMapIndex , "_MapSubType" )->GetInteger();
	if( nSubMapType == CDnWorld::MapSubTypeDarkLair )
	{
		bDarkLairMap = true;
		CDnGameTask *pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstance().GetTask( "GameTask" ));
		if( !pGameTask ) return false;
		int nReadyGateIndex = pGameTask->GetReadyGateIndex();
		if( !CDnWorld::IsActive() ) return false;
		CDnWorld::DungeonGateStruct *pReadyGateStruct = (CDnWorld::DungeonGateStruct *)CDnWorld::GetInstance().GetGateStruct( nReadyGateIndex );
		if( !pReadyGateStruct ) return false;

		for( DWORD i=0; i<pReadyGateStruct->pVecMapList.size(); i++ ) 
		{
			CDnWorld::DungeonGateStruct* pGateStruct = (CDnWorld::DungeonGateStruct*)pReadyGateStruct->pVecMapList[i];
			if( !pGateStruct ) continue;
			vecDarLairMapList.push_back( pGateStruct->nMapIndex );
		}
	}

	// Check Map Index
	for( int i=0; i<5; i++ )
	{
		char szStr[64];
		sprintf_s( szStr, "_Achieve%dType", i+1 );

		if( pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger() == MapID )
		{
			sprintf_s( szStr, "_Achieve%dParam", i+1 );
			int nTargetMapIndex = pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger();
			if( nTargetMapIndex < nMapIndex )	// Target map index must be greater than or equal to entry map index.
				break;

			if( nTargetMapIndex - nMapIndex > 10 )	// If more than 10 portals, consider it not a connected map
				break;

			if( nTargetMapIndex == nMapIndex )
			{
				bFindMap = true;
				break;
			}

			// If target map index > entry map index, there is no way to check if linked, so reuse QuestTask's PathFinder function
			int nLinkedGate;
			for( int j=nMapIndex; j<nTargetMapIndex; j++ )
			{
				bool bResult = GetQuestTask().GetPathFinder()->IsLinkedMap( j, j + 1, nLinkedGate );

				if( !bResult )	// Check if connected when link is broken
				{
					bool bLinkedPath = false;
					int nLinkedIndex = 0;

					for( int k=j; k + 2 <= nTargetMapIndex; k++ )
					{
						bool bResult = GetQuestTask().GetPathFinder()->IsLinkedMap( j, k + 2, nLinkedGate );
						if( bResult )
						{
							nLinkedIndex = k + 2;
							bLinkedPath = true;
						}
					}

					if( !bLinkedPath )
						break;
					else
						j = nLinkedIndex - 1;
				}

				if( bDarkLairMap )
				{
					bool bDarkLairNextFloor = false;
					for( int i=0; i<static_cast<int>( vecDarLairMapList.size() ); i++ )
					{
						if( j + 1 == vecDarLairMapList[i] )
						{
							bDarkLairNextFloor = true;
							break;
						}
					}

					if( bDarkLairNextFloor )
						break;
				}

				if( j + 1 == nTargetMapIndex )	// If linked up to the target map, mark as found and exit.
				{
					bFindMap = true;
					break;
				}
			}
		}
	}

	// Check Difficult
	if( bFindMap )
	{
		bFindMission = true;
		for( int i=0; i<5; i++ )
		{
			char szStr[64];
			sprintf_s( szStr, "_Achieve%dType", i+1 );

			if( pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger() == MapLevel )
			{
				sprintf_s( szStr, "_Achieve%dOperator", i+1 );
				int nOperator = pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger();
				sprintf_s( szStr, "_Achieve%dParam", i+1 );
				switch( nOperator )
				{
				case 0:	if( pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger() != nDifficult ) bFindMission = false; break;
				case 1:	if( pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger() == nDifficult ) bFindMission = false; break;
				case 2:	if( pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger() >= nDifficult ) bFindMission = false; break;
				case 3:	if( pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger() <= nDifficult ) bFindMission = false; break;
				case 4:	if( pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger() > nDifficult ) bFindMission = false; break;
				case 5:	if( pSox->GetFieldFromLablePtr( pMissionInfo->nNotifierID, szStr )->GetInteger() < nDifficult ) bFindMission = false; break;
				}
				break;
			}
		}
	}

	return bFindMission;
}

int CDnDungeonExpectMissionDlg::LoadMissionInCurrentMap( int nMapIndex, int nDifficult )
{
	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TMISSION );
	if( pSox == NULL )
		return 0;

	m_pMissionListBox->RemoveAllItems();
#ifdef PRE_MOD_MISSION_HELPER
	GetMissionTask().ClearDungeonExpectMission();
#endif

	// Set normal dungeon mission
	DWORD dwCount = GetMissionTask().GetMissionCount();
	for( DWORD i=0; i<dwCount; i++ )
	{
		CDnMissionTask::MissionInfoStruct* pInfo = GetMissionTask().GetMissionInfo( i );

		if( !pInfo ) continue;
#ifdef PRE_ADD_MISSION_NEST_TAB
		if( pInfo->MainCategory == CDnMissionTask::Dungeon || pInfo->MainCategory ==  CDnMissionTask::Nest )
#else
		if( pInfo->MainCategory != CDnMissionTask::Dungeon ) continue;
#endif 
		if( pInfo->bAchieve ) continue;

		// Set Available Mission
		SetAvailableMission( pSox, pInfo, nMapIndex, nDifficult );
	}

	// Set weekly and daily missions
	pSox = GetDNTable( CDnTableDB::TDAILYMISSION );
	SetDailyMission( pSox, CDnMissionTask::Daily, nMapIndex, nDifficult );
	SetDailyMission( pSox, CDnMissionTask::Weekly, nMapIndex, nDifficult );
	SetDailyMission( pSox, CDnMissionTask::GuildWar, nMapIndex, nDifficult );
#ifdef PRE_ADD_MONTHLY_MISSION
	SetDailyMission( pSox, CDnMissionTask::MonthlyEvent, nMapIndex, nDifficult );
#endif 

	return m_pMissionListBox->GetSize();
}

void CDnDungeonExpectMissionDlg::SetDailyMission( DNTableFileFormat*  pSox, CDnMissionTask::DailyMissionTypeEnum Type, int nMapIndex, int nDifficult )
{
	if( pSox == NULL )
		return;

	DWORD dwCount = GetMissionTask().GetDailyMissionCount( Type );
	for( DWORD i=0; i<dwCount; i++ )
	{
		CDnMissionTask::MissionInfoStruct *pInfo = GetMissionTask().GetDailyMissionInfo( Type, i );
	
		if( !pInfo ) continue;
		if( pInfo->bAchieve ) continue;

		// Set Available Mission
		SetAvailableMission( pSox, pInfo, nMapIndex, nDifficult );
	}
}

//////////////////////////////////////////////////////////////////////////
// class CDnDungeonExpectMissionItemDlg
//////////////////////////////////////////////////////////////////////////

CDnDungeonExpectMissionItemDlg::CDnDungeonExpectMissionItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
, m_pStaticMissionTitle( NULL )
, m_pStaticMainIcon( NULL )
, m_pStaticSubIcon( NULL )
, m_pStaticMissinoIcon( NULL )
, m_pStaticSelect( NULL )
, m_pTextBoxDescription( NULL )
, m_pInfo( NULL )
{
}

CDnDungeonExpectMissionItemDlg::~CDnDungeonExpectMissionItemDlg( void )
{
}

void CDnDungeonExpectMissionItemDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterTabSubListDlg.ui" ).c_str(), bShow );
}

void CDnDungeonExpectMissionItemDlg::InitialUpdate()
{
	m_pStaticMissionTitle = GetControl<CEtUIStatic>( "ID_MAIN_TITLE" );
	m_pTextBoxDescription = GetControl<CEtUITextBox>( "ID_MIS_TEXTBOX" );

	m_pStaticMainIcon = GetControl<CEtUIStatic>( "ID_MAIN_ICON" );
	m_pStaticMainIcon->Show( false );
	m_pStaticSubIcon = GetControl<CEtUIStatic>( "ID_SUB0_ICON" );
	m_pStaticSubIcon->Show( false );
	m_pStaticMissinoIcon = GetControl<CEtUIStatic>( "ID_MIS_ICON" );
	m_pStaticMissinoIcon->Show( false );
	m_pStaticSelect = GetControl<CEtUIStatic>( "ID_STATIC_SELSCT" );
	m_pStaticSelect->Show( false );
}

void CDnDungeonExpectMissionItemDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDungeonExpectMissionItemDlg::SetMissionInfo( CDnMissionTask::MissionInfoStruct* pInfo )
{
	m_pInfo = pInfo;
	m_pStaticMissinoIcon->Show( true );
	m_pStaticMissionTitle->SetText( pInfo->szTitle.c_str() );
	m_pTextBoxDescription->AddText( pInfo->szSubTitle.c_str() );
}

//////////////////////////////////////////////////////////////////////////
// class CDnDungeonExpectMissionRewardDlg
//////////////////////////////////////////////////////////////////////////

CDnDungeonExpectMissionRewardDlg::CDnDungeonExpectMissionRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pStaticBoard1( NULL )
, m_pStaticBottom1( NULL )
, m_pStaticBoard2( NULL )
, m_pStaticBottom2( NULL )
, m_pRewardAppellation( NULL )
, m_pAppellationTooltip( NULL )
, m_pRewardCoinTooltipDlg( NULL )
, m_pInfo( NULL )
{
	for( int i=0; i<CDnMissionTask::RewardItem_Amount; i++ )
	{
		m_pItem[i] = NULL;
		m_pRewardItem[i] = NULL;
		m_pRewardMoney[i] = NULL;
	}
}

CDnDungeonExpectMissionRewardDlg::~CDnDungeonExpectMissionRewardDlg()
{
	SAFE_DELETE( m_pAppellationTooltip );
	SAFE_DELETE( m_pRewardCoinTooltipDlg );
	for( int i=0; i<CDnMissionTask::RewardItem_Amount; i++ )
		SAFE_DELETE( m_pItem[i] );
}

void CDnDungeonExpectMissionRewardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterTabAppelTipDlg.ui" ).c_str(), bShow );

	m_pAppellationTooltip = new CDnMissionAppellationTooltipDlg;
	m_pAppellationTooltip->Initialize( false );
	m_pRewardCoinTooltipDlg = new CDnMissionRewardCoinTooltipDlg;
	m_pRewardCoinTooltipDlg->Initialize( false );
}

void CDnDungeonExpectMissionRewardDlg::InitialUpdate()
{
	m_pStaticBoard1 = GetControl<CEtUIStatic>( "ID_STATIC_BOARD0" );
	m_pStaticBottom1 = GetControl<CEtUIStatic>( "ID_STATIC_BOTTOM0" );
	m_pStaticBoard2 = GetControl<CEtUIStatic>( "ID_STATIC_BOARD1" );
	m_pStaticBottom2 = GetControl<CEtUIStatic>( "ID_STATIC_BOTTOM1" );

	m_pRewardAppellation = GetControl<CEtUIStatic>( "ID_TEXT_APP" );

	char szLabel[32];
	for( int i=0; i<CDnMissionTask::RewardItem_Amount; i++ )
	{
		sprintf_s( szLabel, 32, "ID_ITEM_0%d", i+1 );
		m_pRewardItem[i] = GetControl<CDnItemSlotButton>( szLabel );
		m_pRewardItem[i]->SetSlotType( ST_MISSIONREWARD );

		sprintf_s( szLabel, 32, "ID_STATIC_COIN%d", i+1 );
		m_pRewardMoney[i] = GetControl<CEtUIStatic>( szLabel );
	}
}

bool CDnDungeonExpectMissionRewardDlg::SetMissionInfo( CDnMissionTask::MissionInfoStruct* pInfo )
{
	bool bSetMissionInfo = false;
	m_pInfo = pInfo;

	WCHAR wszAppellation[256];
	if( pInfo->nRewardAppellationID > 0 )
	{
		m_pRewardAppellation->Show( true );
		DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TAPPELLATION );
		if( pSox && pSox->IsExistItem( pInfo->nRewardAppellationID ) )
		{
			int nAppellationIndex = pSox->GetFieldFromLablePtr( pInfo->nRewardAppellationID, "_NameID" )->GetInteger();
			swprintf_s( wszAppellation, _countof(wszAppellation), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3260 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nAppellationIndex ) );
			bSetMissionInfo = true;
		}
	}
	else
		swprintf_s( wszAppellation, _countof(wszAppellation), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3260 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93 ) );

	m_pRewardAppellation->SetText( wszAppellation );

	int nRewardItemCount = 0;
	// Verify data.
	// When a cash item reward is set in mission rewards, normal item rewards or money should not be included.
	bool bRewardCashItem = false;
	for( int i=0; i<CDnMissionTask::RewardItem_Amount; i++ )
	{
		if( pInfo->nRewardCashItemSN[i] > 0 ) 
		{
			bRewardCashItem = true;
			break;
		}
	}

#ifndef _FINAL_BUILD
	bool bRewardItem = false;
	for( int i=0; i<CDnMissionTask::RewardItem_Amount; i++ )
	{
		if( pInfo->nRewardItemID[i] > 0 )
		{
			bRewardItem = true;
			break;
		}
	}
	if( (bRewardCashItem && bRewardItem) || (bRewardCashItem && pInfo->nRewardCoin) )
	{
		_ASSERT(0&&"Mission reward has cash item, normal item, and gold all set together. Check the data.");
	}
#endif // _FINAL_BUILD

	if( bRewardCashItem )	// Cash item reward
	{
		for( int i=0; i<CDnMissionTask::RewardItem_Amount; i++ )
		{
			SAFE_DELETE( m_pItem[i] );
			m_pRewardItem[i]->Show( false );
			m_pRewardMoney[i]->Show( false );
			if( pInfo->nRewardCashItemSN[i] > 0 ) 
			{
				const SCashShopItemInfo* pCashInfo = GetCashShopTask().GetItemInfo( pInfo->nRewardCashItemSN[i] );
				if( pCashInfo ) 
				{
					TItemInfo Info;
					if( CDnItem::MakeItemInfo( pCashInfo->presentItemId, pCashInfo->count, Info ) ) 
					{
						m_pItem[i] = CDnItemTask::GetInstance().CreateItem( Info );

						if (m_pItem[i]) 
						{
							m_pItem[i]->SetCashItemSN( pInfo->nRewardCashItemSN[i] );
							m_pRewardItem[i]->SetItem(m_pItem[i], CDnSlotButton::ITEM_ORIGINAL_COUNT);
							m_pRewardItem[i]->Show( true );
							m_pRewardItem[i]->SetRegist( pInfo->bAchieve );
							nRewardItemCount++;
							bSetMissionInfo = true;
						}
					}
				}
			}
		}
	}
	else	// Normal item + gold reward
	{
		int nLastRewardItemIndex = -1;
		for( int i=0; i<CDnMissionTask::RewardItem_Amount; i++ )
		{
			SAFE_DELETE( m_pItem[i] );
			m_pRewardItem[i]->Show( false );
			m_pRewardMoney[i]->Show( false );
			if( pInfo->nRewardItemID[i] > 0 ) 
			{
				TItemInfo Info;
				if( CDnItem::MakeItemInfo( pInfo->nRewardItemID[i], 1, Info ) ) 
				{
					m_pItem[i] = CDnItemTask::GetInstance().CreateItem( Info );

					if (m_pItem[i])
					{
						m_pRewardItem[i]->SetItem(m_pItem[i], CDnSlotButton::ITEM_ORIGINAL_COUNT);
						m_pRewardItem[i]->Show( true );
						m_pRewardItem[i]->SetRegist( pInfo->bAchieve );
						nLastRewardItemIndex = i;
						nRewardItemCount++;
						bSetMissionInfo = true;
					}
				}
			}
		}

		if( pInfo->nRewardCoin )
		{
#ifndef _FINAL_BUILD
			if( nLastRewardItemIndex + 1 >= CDnMissionTask::RewardItem_Amount ) 
				_ASSERT(0&&"Mission already has all 3 reward items, yet also gives gold as a reward. Check the data.");
#endif
			if( 0 <= nLastRewardItemIndex + 1 && nLastRewardItemIndex + 1 < CDnMissionTask::RewardItem_Amount ) 
			{
				m_pRewardMoney[nLastRewardItemIndex + 1]->Show( true );
				m_pRewardMoney[nLastRewardItemIndex + 1]->Enable( !pInfo->bAchieve );
				nRewardItemCount++;
				bSetMissionInfo = true;
			}
		}
	}

	m_pRewardAppellation->Enable( true ); // This apparently needs to be in Enable state to be clearly visible.

	if( nRewardItemCount == 0 )
	{
		m_pStaticBoard1->Show( true );
		m_pStaticBottom1->Show( true );
		m_pStaticBoard2->Show( false );
		m_pStaticBottom2->Show( false );
	}
	else
	{
		m_pStaticBoard1->Show( false );
		m_pStaticBottom1->Show( false );
		m_pStaticBoard2->Show( true );
		m_pStaticBottom2->Show( true );
	}

	return bSetMissionInfo;
}

void CDnDungeonExpectMissionRewardDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		m_pRewardAppellation->SetText( L"" );

		for( int i=0; i<CDnMissionTask::RewardItem_Amount; i++ )
		{
			SAFE_DELETE( m_pItem[i] );
			m_pRewardItem[i]->ResetSlot();
			m_pRewardItem[i]->Show( false );
			m_pRewardMoney[i]->Show( false );
		}

		if( m_pAppellationTooltip && m_pAppellationTooltip->IsShow() )
			m_pAppellationTooltip->Show( false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnDungeonExpectMissionRewardDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDungeonExpectMissionRewardDlg::Process( float fElapsedTime )
{
	if( !IsShow() )
		return;

	CDnCustomDlg::Process( fElapsedTime );

	float fMouseX, fMouseY;
	// ListBoxEx ItemDlg does not run MsgProc on its own, so it cannot be retrieved with this function.
	//GetMouseMovePoints( fMouseX, fMouseY );
	GetScreenMouseMovePoints( fMouseX, fMouseY );
	fMouseX -= GetXCoord();
	fMouseY -= GetYCoord();
	bool bMouseEnter(false);
	SUICoord uiCoord;
	m_pRewardAppellation->GetUICoord(uiCoord);
	if( uiCoord.IsInside( fMouseX, fMouseY ) )
		bMouseEnter = true;

	if( m_pRewardAppellation->IsShow() && bMouseEnter && m_pInfo && m_pInfo->nRewardAppellationID > 0 ) 
	{
		m_pAppellationTooltip->SetAppellationInfo( m_pInfo->nRewardAppellationID );
		GetScreenMouseMovePoints( fMouseX, fMouseY );
		m_pAppellationTooltip->SetPosition( fMouseX, fMouseY );
		SUICoord DlgCoord;
		m_pAppellationTooltip->GetDlgCoord( DlgCoord );
		DlgCoord.fX -= DlgCoord.fWidth / 2;
		static float fXRGap(4.0f/DEFAULT_UI_SCREEN_WIDTH);
		if( (DlgCoord.Right()+fXRGap) > GetScreenWidthRatio() )
			DlgCoord.fX -= (DlgCoord.Right()+fXRGap - GetScreenWidthRatio());
		m_pAppellationTooltip->SetDlgCoord( DlgCoord );
		m_pAppellationTooltip->Show( true );
	}
	else m_pAppellationTooltip->Show( false );

	// Check reward Coin control position
	bMouseEnter = false;
	for( int i = 0; i < CDnMissionTask::RewardItem_Amount; ++i )
	{
		if( !m_pRewardMoney[i]->IsShow() ) continue;
		m_pRewardMoney[i]->GetUICoord(uiCoord);
		if( uiCoord.IsInside( fMouseX, fMouseY ) )
			bMouseEnter = true;
	}

	if( bMouseEnter )
	{
		m_pRewardCoinTooltipDlg->SetRewardCoin( m_pInfo->nRewardCoin );
		GetScreenMouseMovePoints( fMouseX, fMouseY );
		m_pRewardCoinTooltipDlg->SetPosition( fMouseX, fMouseY );
		SUICoord DlgCoord;
		m_pRewardCoinTooltipDlg->GetDlgCoord( DlgCoord );
		static float fXRGap(4.0f/DEFAULT_UI_SCREEN_WIDTH);
		if( (DlgCoord.Right()+fXRGap) > GetScreenWidthRatio() )
			DlgCoord.fX -= (DlgCoord.Right()+fXRGap - GetScreenWidthRatio());
		m_pRewardCoinTooltipDlg->SetDlgCoord( DlgCoord );
		m_pRewardCoinTooltipDlg->Show( true );
	}
	else m_pRewardCoinTooltipDlg->Show( false );
}
