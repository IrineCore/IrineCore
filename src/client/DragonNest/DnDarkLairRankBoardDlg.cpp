#include "StdAfx.h"
#include "DnDarkLairRankBoardDlg.h"
#include "DnActorState.h"
#include "DnDLGameTask.h"
#include "DnDarklairClearImp.h"
#include "TaskManager.h"
#include "DnTableDB.h"
#include "DnPlayerActor.h"
#include "DnLocalPlayerActor.h"
#include "DnItemTask.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//////////////////////////////////////////////////////////////////////////
// CDnDarkLairRankBoardDlg
//////////////////////////////////////////////////////////////////////////

CDnDarkLairRankBoardDlg::CDnDarkLairRankBoardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_bRankInfoReqeust( false ), m_pDarkLairRankListBox( NULL ), m_pStaticPlayerInfo( NULL ), m_pStaticHistoryTitle( NULL )
{
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	// Without making a separate table, use direct input.
	m_nRankRewardItem[ 0 ] = 1610612796;
	m_nRankRewardItem[ 1 ] = 1610612797;
	m_nRankRewardItem[ 2 ] = 1610612798;
	m_nRankRewardItem[ 3 ] = 1610612799;
	m_nRankRewardItem[ 4 ] = 1610612800;
	m_nRankRewardItem[ 5 ] = 1610612801;
	m_nRankRewardItem[ 6 ] = 1610612801;
	m_nRankRewardItem[ 7 ] = 1610612801;
	m_nRankRewardItem[ 8 ] = 1610612801;
	m_nRankRewardItem[ 9 ] = 1610612801;

	m_bRankReward = false;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
}

CDnDarkLairRankBoardDlg::~CDnDarkLairRankBoardDlg()
{
	m_pDarkLairRankListBox->RemoveAllItems();
}

void CDnDarkLairRankBoardDlg::InitialUpdate()
{
	CEtUIDialog::InitialUpdate();

	m_pDarkLairRankListBox = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_RANK" );

	m_pStaticPlayerInfo = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );

	m_StaticBestInfo.m_pStaticTitle = GetControl<CEtUIStatic>( "ID_TEXT_PARTYNAME" );
	m_StaticBestInfo.m_pStaticRound = GetControl<CEtUIStatic>( "ID_TEXT_ROUNDBEST" );
	m_StaticBestInfo.m_pStaticRank = GetControl<CEtUIStatic>( "ID_TEXT_RANKBEST_LIST" );
	m_StaticBestInfo.m_pStaticTime = GetControl<CEtUIStatic>( "ID_TEXT_RANKBEST_TIME" );
	
	char szStr[64] = {0,};
	for( int i=0; i<MAX_PARTY_MEMEBER; i++ )
	{
		sprintf_s( szStr, "ID_BEST_CLASS%d", i );
		m_StaticBestInfo.m_pJobIcon[i] = GetControl<CDnJobIconStatic>(szStr);
	}

	for( int i=0; i<MAX_PARTY_MEMEBER-1; i++ )
	{
		sprintf_s( szStr, "ID_TEXT_PARTYCLASS%d", i );
		m_StaticBestInfo.m_pStaticMember[i] = GetControl<CEtUIStatic>( szStr );
	}

	m_pStaticHistoryTitle = GetControl<CEtUIStatic>("ID_TEXT_DARKRANK_TITLE");
	for( int i=0; i<3; i++ ) 
	{
		sprintf_s( szStr, "ID_TEXT_RANK_NAME%d", i );
		m_StaticTop3Info[i].m_pStaticTitle = GetControl<CEtUIStatic>( szStr );
		sprintf_s( szStr, "ID_TEXT_RANK_ROUND%d", i );
		m_StaticTop3Info[i].m_pStaticRound = GetControl<CEtUIStatic>( szStr );
		sprintf_s( szStr, "ID_TEXT_RANK%d", i );
		m_StaticTop3Info[i].m_pStaticRank = GetControl<CEtUIStatic>( szStr );
		sprintf_s( szStr, "ID_TEXT_RANK_TIME%d", i );
		m_StaticTop3Info[i].m_pStaticTime = GetControl<CEtUIStatic>( szStr );

		for( int j=0; j<MAX_PARTY_MEMEBER; j++ )
		{
			sprintf_s( szStr, "ID_TEXT_RANK%d_CLASS%d", i, j );
			m_StaticTop3Info[i].m_pStaticMember[j] = GetControl<CEtUIStatic>( szStr );

			sprintf_s( szStr, "ID_RANK%d_CLASS%d", i, j );
			m_StaticTop3Info[i].m_pJobIcon[j] = GetControl<CDnJobIconStatic>(szStr);
		}

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		sprintf_s( szStr, "ID_BT_ITEM%d", i );
		m_StaticTop3Info[i].m_pItemSlotButton = GetControl<CDnItemSlotButton>( szStr );
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	}
}

void CDnDarkLairRankBoardDlg::ResetStatic()
{
	m_pDarkLairRankListBox->RemoveAllItems();

	//  Hide best record static text
	m_StaticBestInfo.m_pStaticTitle->Show( false );
	m_StaticBestInfo.m_pStaticRound->Show( false );
	m_StaticBestInfo.m_pStaticRank->Show( false );
	m_StaticBestInfo.m_pStaticTime->Show( false );

	for( int i=0; i<MAX_PARTY_MEMEBER-1; i++ )
		m_StaticBestInfo.m_pStaticMember[i]->Show( false );

	for( int i=0; i<MAX_PARTY_MEMEBER; i++ )
		m_StaticBestInfo.m_pJobIcon[i]->SetIconID( -1 );

	//  Hide Top3 records
	for( int i=0; i<3; i++ )
		m_StaticTop3Info[i].Reset();
}

void CDnDarkLairRankBoardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DarkLairRankBoardDlg.ui" ).c_str(), bShow );
}

void CDnDarkLairRankBoardDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
	DnActorHandle hNpc = CDnActor::FindActorFromUniqueID( nUID );

	if( hNpc )
	{
		if( bShow )
			hNpc->SetActionQueue( "Open" );
		else
		{
			hNpc->SetActionQueue( "Close" );
			GetInterface().CloseBlind();
		}
	}

	if( !bShow )
	{
		m_pDarkLairRankListBox->RemoveAllItems();
		GetInterface().CloseBlind();
		//GetInterface().CloseNpcTalkReturnDlg();
	}

	CEtUIDialog::Show( bShow );
}


bool CDnDarkLairRankBoardDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	// Since there is no close button, handle it in MsgProc like this.
	// Still better to handle it with an invisible button, though next time we should avoid doing it this way.
	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_ESCAPE )
		{
			Show( false );
			return true;
		}
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnDarkLairRankBoardDlg::SetDarkLairRankInfo( TDLRankHistoryPartyInfo *pBestInfo, TDLRankHistoryPartyInfo *pHistoryInfo )
{
	if( pBestInfo == NULL || pHistoryInfo == NULL )
		return;

	ResetStatic();

	WCHAR wszStr[128] = {0,};
	DnActorHandle hActor = CDnActor::s_hLocalActor;
	if( !hActor ) return;

	// Set basic info
	CDnPlayerActor *pActor = static_cast<CDnPlayerActor*>( hActor.GetPointer() );
	swprintf_s( wszStr, L"%s ( %s Lv.%d )", pActor->GetName(), pActor->GetJobName(), pActor->GetLevel() );
	m_pStaticPlayerInfo->SetText( wszStr );

	swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 810 ), pBestInfo->cPartyUserCount );
	m_pStaticHistoryTitle->SetText( wszStr );

	// Set best record
	if( pBestInfo->nPlayRound > 0 && pBestInfo->nRank > 0 && pBestInfo->nPlaySec > 0 )
	{
		m_StaticBestInfo.m_pStaticTitle->SetText( pBestInfo->wszPartyName );
		m_StaticBestInfo.m_pStaticTitle->Show( true );

		swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 808 ), pBestInfo->nPlayRound );
		m_StaticBestInfo.m_pStaticRound->SetText( wszStr );
		m_StaticBestInfo.m_pStaticRound->Show( true );

		switch( pBestInfo->nRank )
		{
			case 1: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 813 ) ); break;
			case 2: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 814 ) ); break;
			case 3: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 815 ) ); break;
			default: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 809 ), pBestInfo->nRank ); break;
		}
		m_StaticBestInfo.m_pStaticRank->SetText( wszStr );
		m_StaticBestInfo.m_pStaticRank->Show( true );

		int nH = pBestInfo->nPlaySec / (60 * 60);
		int nM = (pBestInfo->nPlaySec - (nH * 60 * 60)) / 60;
		int nS = (pBestInfo->nPlaySec - ((nH * 60 * 60) + (nM * 60)));
		swprintf_s( wszStr, L"%02d : %02d : %02d", nH, nM, nS );
		m_StaticBestInfo.m_pStaticTime->SetText( wszStr );
		m_StaticBestInfo.m_pStaticTime->Show( true );
		
		int nPartyMemeberIndex = 0;
		WCHAR wszStrMemberInfo[128] = {0,};
		for( int i=0; i<pBestInfo->cPartyUserCount; i++ ) 
		{
			if( wcscmp( pBestInfo->Info[i].wszCharacterName, pActor->GetName() ) != 0 )		// Display info of party members excluding myself
			{
				if( pBestInfo->Info[i].nJobIndex <= 0 ) continue;
				if( m_StaticBestInfo.m_pJobIcon[nPartyMemeberIndex+1] )
					m_StaticBestInfo.m_pJobIcon[nPartyMemeberIndex+1]->SetIconID( pBestInfo->Info[i].nJobIndex, true );

				swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 827 ), pBestInfo->Info[i].nLevel );
				swprintf_s( wszStrMemberInfo, L"%s%s", pBestInfo->Info[i].wszCharacterName, wszStr );
				if( nPartyMemeberIndex < MAX_PARTY_MEMEBER -1 )
				{
					m_StaticBestInfo.m_pStaticMember[nPartyMemeberIndex]->SetText( wszStrMemberInfo );
					m_StaticBestInfo.m_pStaticMember[nPartyMemeberIndex]->Show( true );
				}
				nPartyMemeberIndex++;
			}
			else	// Display only class icon for my info and show it at the very front
			{
				if( pBestInfo->Info[i].nJobIndex <= 0 ) continue;
				if( m_StaticBestInfo.m_pJobIcon[0] )
					m_StaticBestInfo.m_pJobIcon[0]->SetIconID( pBestInfo->Info[i].nJobIndex, true );
			}
		}
	}

	// Set Top3 records
	WCHAR wszStrMemberInfo[128] = {0,};
	for( int i=0; i<DarkLair::Rank::SelectRankBoardTop; i++ ) 
	{
		if( pHistoryInfo[i].nPlayRound == 0 ) continue;
		if( pHistoryInfo[i].nRank == 0 ) continue;
		if( pHistoryInfo[i].nPlaySec == 0 ) continue;

		if( i < 3 )	// Top3 is shown in main dialog, the rest in a list box
		{
			m_StaticTop3Info[i].m_pStaticTitle->SetText( pHistoryInfo[i].wszPartyName );
			m_StaticTop3Info[i].m_pStaticTitle->Show( true );

			swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 811 ), pHistoryInfo[i].nPlayRound );
			m_StaticTop3Info[i].m_pStaticRound->SetText( wszStr );
			m_StaticTop3Info[i].m_pStaticRound->Show( true );

			m_StaticTop3Info[i].m_pStaticRank->Show( true );

			int nH = pHistoryInfo[i].nPlaySec / (60 * 60);
			int nM = (pHistoryInfo[i].nPlaySec - (nH * 60 * 60)) / 60;
			int nS = (pHistoryInfo[i].nPlaySec - ((nH * 60 * 60) + (nM * 60)));
			swprintf_s( wszStr, L"%02d : %02d : %02d", nH, nM, nS );
			m_StaticTop3Info[i].m_pStaticTime->SetText( wszStr );
			m_StaticTop3Info[i].m_pStaticTime->Show( true );

			for( int j=0; j<pHistoryInfo[i].cPartyUserCount; j++ )
			{
				if( pHistoryInfo[i].Info[j].nJobIndex <= 0 ) continue;
				if( m_StaticTop3Info[i].m_pJobIcon[j] )
					m_StaticTop3Info[i].m_pJobIcon[j]->SetIconID( pHistoryInfo[i].Info[j].nJobIndex, true );

				swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 827 ), pHistoryInfo[i].Info[j].nLevel );
				swprintf_s( wszStrMemberInfo, L"%s%s", pHistoryInfo[i].Info[j].wszCharacterName, wszStr );
				m_StaticTop3Info[i].m_pStaticMember[j]->SetText( wszStrMemberInfo );
				m_StaticTop3Info[i].m_pStaticMember[j]->Show( true );
			}

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
			if( true == m_bRankReward )
			{
				TItemInfo itemInfo;
				if( CDnItem::MakeItemInfo( m_nRankRewardItem[i], 1, itemInfo ) == false )
					continue;
				CDnItem * pItem = GetItemTask().CreateItem( itemInfo );
				m_StaticTop3Info[i].m_pItemSlotButton->SetItem( pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
				m_StaticTop3Info[i].m_pItemSlotButton->Show( true );
			}
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		}
		else
		{
			CDnDarkLairRankBoardListItemDlg* pRankListItemDlg = m_pDarkLairRankListBox->AddItem<CDnDarkLairRankBoardListItemDlg>();

#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
			if( true == m_bRankReward )
				pRankListItemDlg->SetDarLairRankRewardItem( m_nRankRewardItem[i] );
			else
				pRankListItemDlg->SetDarLairRankRewardItem( 0 );
#endif	// #if defined(PRE_ADD_CHALLENGE_DARKLAIR)

			pRankListItemDlg->SetDarkLairRankInfo( &pHistoryInfo[i] );
		}
	}

	m_bRankInfoReqeust = false;
}

#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
void CDnDarkLairRankBoardDlg::SetDarkLairRankMapIndex( int nMapIndex )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDLMAP );
	if( NULL == pSox )
		return;

	if( DarkLair::TopFloorAbstractMapIndex < nMapIndex )
		nMapIndex -= DarkLair::TopFloorAbstractMapIndex;

	m_bRankReward = pSox->GetFieldFromLablePtr( nMapIndex, "_ChallengeType" )->GetInteger() == 0 ? false : true;
}
#endif	// #if defined(PRE_ADD_CHALLENGE_DARKLAIR)

//////////////////////////////////////////////////////////////////////////
// CDnDarkLairRankBoardListItemDlg
//////////////////////////////////////////////////////////////////////////


CDnDarkLairRankBoardListItemDlg::CDnDarkLairRankBoardListItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
, m_nRankRewardItem( 0 )
#endif	// #if defined(PRE_ADD_CHALLENGE_DARKLAIR)
{
}

CDnDarkLairRankBoardListItemDlg::~CDnDarkLairRankBoardListItemDlg()
{
}

void CDnDarkLairRankBoardListItemDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "DarkLairRankBoardListDlg.ui" ).c_str(), bShow );
	SetElementDialog( true );
}

void CDnDarkLairRankBoardListItemDlg::InitialUpdate()
{
	CDnCustomDlg::InitialUpdate();

	m_StaticRankInfo.m_pStaticTitle = GetControl<CEtUIStatic>( "ID_TEXT_RANK_NAME" );
	m_StaticRankInfo.m_pStaticRound = GetControl<CEtUIStatic>( "ID_TEXT_RANK_ROUND" );
	m_StaticRankInfo.m_pStaticRank = GetControl<CEtUIStatic>( "ID_TEXT_RANK" );
	m_StaticRankInfo.m_pStaticTime = GetControl<CEtUIStatic>( "ID_TEXT_RANK_TIME" );

	char szStr[64] = {0,};
	for( int i=0; i<MAX_PARTY_MEMEBER; i++ )
	{
		sprintf_s( szStr, "ID_TEXT_RANK_CLASS%d", i );
		m_StaticRankInfo.m_pStaticMember[i] = GetControl<CEtUIStatic>( szStr );

		sprintf_s( szStr, "ID_CLASS%d", i );
		m_StaticRankInfo.m_pJobIcon[i] = GetControl<CDnJobIconStatic>(szStr);
	}

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	m_StaticRankInfo.m_pItemSlotButton = GetControl<CDnItemSlotButton>( "ID_BT_ITEM0" );
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
}

void CDnDarkLairRankBoardListItemDlg::ResetStatic()
{
	for( int i=0; i<MAX_PARTY_MEMEBER; i++ )
	{
		m_StaticRankInfo.m_pStaticMember[i]->Show( false );
		m_StaticRankInfo.m_pJobIcon[i]->SetIconID( -1 );
	}

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	if( NULL != m_StaticRankInfo.m_pItemSlotButton )
	{
		MIInventoryItem * pItem = m_StaticRankInfo.m_pItemSlotButton->GetItem();
		SAFE_DELETE( pItem );
		m_StaticRankInfo.m_pItemSlotButton->Show( false );
	}
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
}

void CDnDarkLairRankBoardListItemDlg::SetDarkLairRankInfo( TDLRankHistoryPartyInfo *pHistoryInfo )
{
	if( pHistoryInfo == NULL )
		return;

	ResetStatic();

	WCHAR wszStr[128] = {0,};
	DnActorHandle hActor = CDnActor::s_hLocalActor;
	if( !hActor ) return;

	m_StaticRankInfo.m_pStaticTitle->SetText( pHistoryInfo->wszPartyName );

	swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 811 ), pHistoryInfo->nPlayRound );
	m_StaticRankInfo.m_pStaticRound->SetText( wszStr );

	switch( pHistoryInfo->nRank )
	{
		case 1: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 813 ) ); break;
		case 2: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 814 ) ); break;
		case 3: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 815 ) ); break;
		default: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 809 ), pHistoryInfo->nRank ); break;
	}
	m_StaticRankInfo.m_pStaticRank->SetText( wszStr );

	int nH = pHistoryInfo->nPlaySec / (60 * 60);
	int nM = (pHistoryInfo->nPlaySec - (nH * 60 * 60)) / 60;
	int nS = (pHistoryInfo->nPlaySec - ((nH * 60 * 60) + (nM * 60)));
	swprintf_s( wszStr, L"%02d : %02d : %02d", nH, nM, nS );
	m_StaticRankInfo.m_pStaticTime->SetText( wszStr );

	WCHAR wszStrMemberInfo[128] = {0,};
	for( int i=0; i<pHistoryInfo->cPartyUserCount; i++ ) 
	{
		if( pHistoryInfo->Info[i].nJobIndex <= 0 ) continue;
		if( m_StaticRankInfo.m_pJobIcon[i] )
			m_StaticRankInfo.m_pJobIcon[i]->SetIconID( pHistoryInfo->Info[i].nJobIndex, true );

		swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 827 ), pHistoryInfo->Info[i].nLevel );
		swprintf_s( wszStrMemberInfo, L"%s%s", pHistoryInfo->Info[i].wszCharacterName, wszStr );
		m_StaticRankInfo.m_pStaticMember[i]->SetText( wszStrMemberInfo );
		m_StaticRankInfo.m_pStaticMember[i]->Show( true );
	}

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	if( 0 != m_nRankRewardItem )
	{
		TItemInfo itemInfo;
		if( CDnItem::MakeItemInfo( m_nRankRewardItem, 1, itemInfo ) == false )
			return;
		CDnItem * pItem = GetItemTask().CreateItem( itemInfo );
		m_StaticRankInfo.m_pItemSlotButton->SetItem( pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
		m_StaticRankInfo.m_pItemSlotButton->Show( true );
	}
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
}
