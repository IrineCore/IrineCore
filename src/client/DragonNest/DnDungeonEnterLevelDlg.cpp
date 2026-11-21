#include "StdAfx.h"
#include "DnDungeonEnterLevelDlg.h"
#include "DnPartyTask.h"
#include "PartySendPacket.h"
#include "DnItemTask.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnNestInfoTask.h"
#include "DnTableDB.h"
#include "DnSimpleTooltipDlg.h"
#include "DnDungeonEnterNotifierDlg.h"
#include "DnNestNotifierDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnDungeonEnterLevelDlg::CDnDungeonEnterLevelDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
	, m_pStaticTimer(NULL)
	, m_pButtonEnter(NULL)
	, m_pButtonCancel(NULL)
	, m_pTextureDungeonArea(NULL)
	, m_pSelectName(NULL)
	, m_pSelectLevel(NULL)
	, m_pRecommLevel(NULL)
	, m_pRecommCount(NULL)
	, m_fElapsedTime(0.0f)
	, m_nCount(0)
	, m_nSelectedLevelIndex(0)
	, m_nMapIndex(0)
	, m_pNeedItem(NULL)
	, m_pItemSlotButton(NULL)
	, m_bEnoughNeedItem(false)
	, m_cDungeonEnterPermit(0)
	, m_pNeedItemCount(NULL)
	, m_pMaxUsableCoin(NULL)
	, m_pDungeonFatigueDegree( NULL )
	, m_pNestClearInfo( NULL )
	, m_pBeginnerDungeonFatigueDegree( NULL )
	, m_pStaticBeginnerMark( NULL )
	, m_nBeginnerMaxLevel( 0 )
	, m_nBaseFatigue( 0 )
	, m_nBeginnerDecreaseFatigue( 0 )
	, m_nMaxLevelDecreaseFatigue( 0 )
	, m_pNeedItemStatic( NULL )
{
#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
	m_bCloseProcessingByUser = false;
#endif
}

CDnDungeonEnterLevelDlg::~CDnDungeonEnterLevelDlg(void)
{
	SAFE_RELEASE_SPTR( m_hIntroArea );
	SAFE_DELETE( m_pNeedItem );
}

void CDnDungeonEnterLevelDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterLevelDlg.ui" ).c_str(), bShow );
}

void CDnDungeonEnterLevelDlg::InitialUpdate()
{
	m_pStaticTimer = GetControl<CEtUIStatic>("ID_STATIC_TIMER");
	m_pButtonEnter = GetControl<CEtUIButton>("ID_BUTTON_ENTER");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");
	m_pTextureDungeonArea = GetControl<CEtUITextureControl>("ID_TEXTUREL_AREA");

	CEtUIRadioButton *pLevelButton(NULL);
	pLevelButton = GetControl<CEtUIRadioButton>("ID_BUTTON_LEVEL_EASY");
	m_vecDungeonLevelButton.push_back( pLevelButton );
	pLevelButton = GetControl<CEtUIRadioButton>("ID_BUTTON_LEVEL_NORMAL");
	m_vecDungeonLevelButton.push_back( pLevelButton );
	pLevelButton = GetControl<CEtUIRadioButton>("ID_BUTTON_LEVEL_HARD");
	m_vecDungeonLevelButton.push_back( pLevelButton );
	pLevelButton = GetControl<CEtUIRadioButton>("ID_BUTTON_LEVEL_HELL");
	m_vecDungeonLevelButton.push_back( pLevelButton );
	pLevelButton = GetControl<CEtUIRadioButton>("ID_BUTTON_LEVEL_SPECIAL"); //ABYSS
	m_vecDungeonLevelButton.push_back( pLevelButton );

	bool bExistSpecialHell = false;
	CEtUIRadioButton *pSpecialHellBtn = GetControl<CEtUIRadioButton>("ID_BUTTON_LEVEL_SPECIALHELL", &bExistSpecialHell);
	if( bExistSpecialHell && pSpecialHellBtn )
	{
		pSpecialHellBtn->Show( false );
		m_vecDungeonLevelButton.push_back( pSpecialHellBtn );
	}
	else
	{
		m_vecDungeonLevelButton.push_back( pLevelButton );
	}

	m_pSelectName = GetControl<CEtUIStatic>("ID_SELECT_NAME");
	m_pSelectLevel = GetControl<CEtUIStatic>("ID_SELECT_LEVEL");
	m_pRecommLevel = GetControl<CEtUIStatic>("ID_RECOMM_LEVEL");
	m_pRecommCount = GetControl<CEtUIStatic>("ID_RECOMM_COUNT");

	m_pItemSlotButton = GetControl<CDnItemSlotButton>("ID_ITEM");
	m_pNeedItemStatic = GetControl<CEtUIStatic>("ID_ITEMSLOT");
	m_pNeedItemStatic->Show( false );
	m_pNeedItemCount = GetControl<CEtUIStatic>("ID_STATIC_COUNT");

	m_pMaxUsableCoin = GetControl<CEtUIStatic>("ID_REBIRTH_LIMIT");

	m_pDungeonFatigueDegree = GetControl<CEtUIStatic>( "ID_TEXT_TCOUNT" );
	m_pBeginnerDungeonFatigueDegree = GetControl<CEtUIStatic>( "ID_TEXT_TCOUNT_NOVICE" );
	m_pStaticBeginnerMark = GetControl<CEtUIStatic>( "ID_STATIC_NOVICEICON" );
	m_pStaticBeginnerMark->Show( false );
	m_pNestClearInfo = GetControl<CEtUIStatic>( "ID_TEXT_NOTICE" );
		

}

void CDnDungeonEnterLevelDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( m_nCount > 0 )
	{
		m_fElapsedTime += fElapsedTime;

		if( m_fElapsedTime > 1.0f )
		{
			m_nCount--;
			SetTime( m_nCount );

			m_fElapsedTime -= 1.0f;
		}
	}
	else
	{
		if( IsShow() )
		{
			Show( false );
			if( m_pButtonEnter->IsEnable() )
				CEtUIDialog::ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonEnter, 0 );
			else if( m_pButtonCancel->IsEnable() )
				CEtUIDialog::ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonCancel, 0 );
		}
	}
}

void CDnDungeonEnterLevelDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		CEtUIRadioButton *pRadioButton = (CEtUIRadioButton*)pControl;
		if( !pRadioButton ) return;

		if( strstr( pControl->GetControlName(), "ID_BUTTON_LEVEL" ) )
		{
			m_nSelectedLevelIndex = pRadioButton->GetTabID();
			m_pSelectLevel->SetText( m_vecDungeonLevelButton[m_nSelectedLevelIndex]->GetText() );
			wchar_t wszTemp[64] = {0};

			if( !m_vecRecommandLevel.empty() )
			{
				int nLevel = m_vecRecommandLevel[m_nSelectedLevelIndex];
				if( nLevel > 0 )
				{
					swprintf_s( wszTemp, _countof(wszTemp), L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2368 ), nLevel );
					m_pRecommLevel->SetText( wszTemp );
				}
			}

			if( !m_vecRecommandPartyCount.empty() )
			{
				int nPartyCount = m_vecRecommandPartyCount[m_nSelectedLevelIndex];
				if( nPartyCount > 0 )
				{
					swprintf_s( wszTemp, _countof(wszTemp), L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2369 ), nPartyCount );
					m_pRecommCount->SetText( wszTemp );
				}
			}

			::SendSelectDungeonInfo( m_nMapIndex, m_nSelectedLevelIndex );
			SetFatigueDegree( m_nMapIndex, m_nSelectedLevelIndex );
			static_cast<CDnDungeonEnterDlg*>( GetParentDialog() )->UpdateDungeonExpectReward( m_nMapIndex, m_nSelectedLevelIndex );
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	if( nCommand == EVENT_BUTTON_CLICKED ) {
		if( strcmp( pControl->GetControlName(), "ID_BUTTON_ENTER" ) == 0 || strcmp( pControl->GetControlName(), "ID_BUTTON_CANCEL" ) == 0 ) {
			m_pButtonEnter->Enable( false );
			m_pButtonCancel->Enable( false );
		}

#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
		m_bCloseProcessingByUser = true;
#endif
	}
}

void CDnDungeonEnterLevelDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_nCount = 60;
		SetControlState();
		SetTime( m_nCount );
#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
		m_bCloseProcessingByUser = false;
#endif
	}
	else
	{
		m_nMapIndex = 0;
		m_fElapsedTime = 0.0f;
		m_vecRecommandLevel.clear();
		m_vecRecommandPartyCount.clear();

		if (m_pSelectName)
			m_pSelectName->ClearText();
		if (m_pSelectLevel)
			m_pSelectLevel->ClearText();
		if (m_pDungeonFatigueDegree)
			m_pDungeonFatigueDegree->ClearText();
		if (m_pBeginnerDungeonFatigueDegree)
			m_pBeginnerDungeonFatigueDegree->ClearText();
		if (m_pStaticBeginnerMark)
			m_pStaticBeginnerMark->Show( false );
		if (m_pRecommLevel)
			m_pRecommLevel->ClearText();
		if (m_pRecommCount)
			m_pRecommCount->ClearText();
		if (m_pMaxUsableCoin)
			m_pMaxUsableCoin->ClearText();

#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
		GetInterface().ShowDungeonLimitDlg( false , 0 );
#endif
		if( GetInterface().GetDungeonEnterNotifierDlg() && GetInterface().GetDungeonEnterNotifierDlg()->IsShow() )
			GetInterface().GetDungeonEnterNotifierDlg()->Show( false );
		if( GetInterface().GetNestNotifierDlg() && GetInterface().GetNestNotifierDlg()->IsShow() )
			GetInterface().GetNestNotifierDlg()->Show( false );
	}

	CEtUIDialog::Show( bShow );
}

bool CDnDungeonEnterLevelDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_ESCAPE )
		{
			CEtUIDialog::ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonCancel, 0 );
			return true;
		}
	}
	else if( uMsg == WM_MOUSEMOVE )
	{
		float fMouseX, fMouseY;
		GetScreenMouseMovePoints( fMouseX, fMouseY );
		fMouseX -= GetXCoord();
		fMouseY -= GetYCoord();
		bool bMouseEnter(false);
		SUICoord uiCoord;

		if( m_nBeginnerDecreaseFatigue > 0 )
		{
			if( m_pBeginnerDungeonFatigueDegree )
				m_pBeginnerDungeonFatigueDegree->GetUICoord( uiCoord );
		}
		else
		{
			if( m_pDungeonFatigueDegree )
				m_pDungeonFatigueDegree->GetUICoord( uiCoord );
		}

		if( uiCoord.IsInside( fMouseX, fMouseY ) )
		{
			// Fatigue tooltip
			if( m_nBeginnerDecreaseFatigue > 0 || m_nMaxLevelDecreaseFatigue > 0 )
			{
				std::wstring str;
				str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5151 ), m_nBaseFatigue );
				str += L"\n";

				if( m_nBeginnerDecreaseFatigue > 0 )
				{
					str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5152 ), m_nBeginnerDecreaseFatigue );
					str += L"\n";
				}
				if( m_nMaxLevelDecreaseFatigue > 0 )
					str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5153 ), m_nMaxLevelDecreaseFatigue );

				if( str.empty() == false )
				{
					CDnSimpleTooltipDlg* pSimpleTooltipDlg = GetInterface().GetSimpleTooltipDialog();
					if( pSimpleTooltipDlg )
					{
						if( m_nBeginnerDecreaseFatigue > 0 )
							pSimpleTooltipDlg->ShowTooltipDlg( m_pBeginnerDungeonFatigueDegree, true, str, 0xffffffff, true );
						else
							pSimpleTooltipDlg->ShowTooltipDlg( m_pDungeonFatigueDegree, true, str, 0xffffffff, true );

						pSimpleTooltipDlg->SetTopPosition( true );
					}
				}
			}
		}

		if( m_pStaticBeginnerMark )
		{
			m_pStaticBeginnerMark->GetUICoord(uiCoord);
			if( m_pStaticBeginnerMark->IsShow() && uiCoord.IsInside( fMouseX, fMouseY ) )
			{
				// Beginner tooltip
				std::wstring str;
				str = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5149 );
				str += L"\n\n";
				str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5150 ), m_nBeginnerMaxLevel );

				if( str.empty() == false )
				{
					CDnSimpleTooltipDlg* pSimpleTooltipDlg = GetInterface().GetSimpleTooltipDialog();
					if( pSimpleTooltipDlg )
					{
						pSimpleTooltipDlg->ShowTooltipDlg( m_pStaticBeginnerMark, true, str, 0xffffffff, true );
						pSimpleTooltipDlg->SetTopPosition( true );
					}
				}
			}
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnDungeonEnterLevelDlg::SetTime( int nTime )
{
	wchar_t wszTimer[80] = {0};
	swprintf_s( wszTimer, _countof(wszTimer), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2312 ), nTime );
	m_pStaticTimer->SetText( wszTimer );
}

void CDnDungeonEnterLevelDlg::SetImage( int nMapIndex )
{
	m_nMapIndex = nMapIndex;
	SAFE_RELEASE_SPTR( m_hIntroArea );

	char szImageName[128] = {0};
	sprintf_s( szImageName, _countof(szImageName), "DungeonImage_%d.dds", nMapIndex );

	m_hIntroArea = LoadResource( CEtResourceMng::GetInstance().GetFullName( szImageName ).c_str(), RT_TEXTURE );

	// When rendering a texture control, create and set Trans/Scale matrices and render using ID3DXSprite,
	// Due to UI characteristics, floats with messy fractional parts may be set, causing a 1-pixel offset at times (0.5 issue).
	// Since it does not use a vertex buffer and cannot adjust the final value by 0.5,
	// Currently there seems to be no clean way to prevent 1-pixel misalignment at certain scales.
	// Until improved, ignore the rightmost 1 pixel of the texture (seems to have an issue due to alpha on the right edge).
	if( m_hIntroArea )
		m_pTextureDungeonArea->SetTexture( m_hIntroArea, 0, 0, m_hIntroArea->Width()-1, m_hIntroArea->Height() );
}

void CDnDungeonEnterLevelDlg::SetControlState()
{
	bool bEnable(false);

	CDnPartyTask::PartyRoleEnum partyState = GetPartyTask().GetPartyRole();

	if( partyState == CDnPartyTask::LEADER || partyState == CDnPartyTask::SINGLE )
	{
		bEnable = true;
	}
	else
	{
		bEnable = false;

		for( int i=0; i<(int)m_vecDungeonLevelButton.size(); i++ )
		{
			m_vecDungeonLevelButton[i]->Enable( false );
		}
	}

	m_pButtonEnter->ClearTooltipText();
	m_pButtonEnter->Enable(bEnable);
	m_pButtonCancel->Enable(bEnable);

	// First check if it's the player's own item. If not owned by self, display a self-related tooltip message.
	WCHAR wszMsg[1024];
	/*
	if( !m_bEnoughNeedItem ) {
		m_pButtonEnter->Enable(false);
		if( m_pNeedItem ) {
			swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2375 ), m_pNeedItem->GetName() );
			m_pButtonEnter->SetTooltipText( wszMsg );
		}
	}
	else
	{
		// When self has the required item but a party member does not,
		if( m_nDungeonEnterPermit != ERROR_NONE ) {
			m_pButtonEnter->Enable(false);
			if( m_pNeedItem ) {
				swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2376 ), m_pNeedItem->GetName() );
				m_pButtonEnter->SetTooltipText( wszMsg );
			}
		}
	}
	*/

	//m_pButtonEnter->Enable( ( m_cDungeonEnterPermit == CDnWorld::PermitEnter ) ? true : false ); // Changed because party members can also press the enter button.
	if( m_cDungeonEnterPermit != CDnWorld::PermitEnter )
		m_pButtonEnter->Enable( false );

	if( m_cDungeonEnterPermit & CDnWorld::PermitNotEnoughItem ) {
		if( m_pNeedItem ) {
			std::wstring wszStr;
			if( !m_bEnoughNeedItem )
				swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2375 ), m_pNeedItem->GetName() );
			else 
				swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2376 ), m_pNeedItem->GetName() );

			// Intended to show a list of items that are short, but ran out of time due to sync issues, so leaving the code here for later
			// Working on it. (Once done, the m_bEnoughNeedItem code above won't be needed.) - siva
			wszStr += wszMsg;

			for( DWORD i=0; i<m_hVecPermitActorList.size() ;i++ ) {
				DnActorHandle hActor = m_hVecPermitActorList[i];
				if( hActor ) {
					wszStr += L" - ";
					wszStr += hActor->GetName();
					wszStr += L"\n";
				}
			}
			m_pButtonEnter->SetTooltipText( wszStr.c_str() );
		}
	}
}

void CDnDungeonEnterLevelDlg::SetDungeonName( const wchar_t *wszDungeonName )
{
	m_pSelectName->SetText( wszDungeonName );
}

void CDnDungeonEnterLevelDlg::UpdateDungeonInfo( CDnDungeonEnterDlg::SDungeonGateInfo* pInfo )
{
	if( pInfo == NULL )
		return;

	SetDungeonName( pInfo->m_pButtonGate->GetDungeonName() );
	SetDungeonDesc( pInfo->m_pButtonGate->GetDungeonDesc() );
	SetImage( pInfo->m_nMapIndex );

	UpdateRecommandLevel( pInfo->m_vecRecommandLevel );
	UpdateRecommandPartyCount( pInfo->m_vecRecommandPartyCount );
	UpdateNeedItem( pInfo->m_nNeedItemID, pInfo->m_nNeedItemCount, pInfo->m_cDungeonEnterPermit, &pInfo->m_hVecNeedItemActorList );
	UpdateLevelButton( pInfo->m_cCanDifficult, pInfo->m_nAbyssMinLevel, pInfo->m_nAbyssMaxLevel, pInfo->m_nAbyssNeedQuestID );
	UpdateMaxUsableCoin( pInfo->m_nMaxUsableCoinCount );

	if( m_pNestClearInfo )
	{
		CDnNestInfoTask::NestDungeonInfoStruct* pNestDungeonInfo = CDnNestInfoTask::GetInstance().FindNestDungeonInfoFromMapIndex( pInfo->m_nMapIndex );

		if( pNestDungeonInfo )	// Display remaining clear count if it is a Nest
		{
			WCHAR wszStr[256];
			WCHAR wszTemp[256];
			swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 820 ), pNestDungeonInfo->nMaxTryCount - pNestDungeonInfo->nTryCount, pNestDungeonInfo->nTryCount, pNestDungeonInfo->nMaxTryCount );
			swprintf_s( wszStr, L"%s   %s", pNestDungeonInfo->szMapName.c_str(), wszTemp );
			m_pNestClearInfo->SetText( wszStr );

			if( GetInterface().GetNestNotifierDlg() )
			{
				GetInterface().GetNestNotifierDlg()->RefreshNestList();
				GetInterface().GetNestNotifierDlg()->SetNoticeText( wszStr );
			}
		}
		else
			m_pNestClearInfo->SetText( L"" );
	}

	if( m_vecDungeonLevelButton.size() > 5 && m_vecDungeonLevelButton[5] && m_vecDungeonLevelButton[5] != m_vecDungeonLevelButton[4] )
	{
		bool bHasHellMap = false;
		DNTableFileFormat *pMapSox = GetDNTable( CDnTableDB::TMAP );
		DNTableFileFormat *pEnterSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
		if( pMapSox && pEnterSox && pMapSox->IsExistItem( pInfo->m_nMapIndex ) )
		{
			DNTableCell *pCondCell = pMapSox->GetFieldFromLablePtr( pInfo->m_nMapIndex, "_EnterConditionTableID" );
			int nCondID = pCondCell ? pCondCell->GetInteger() : 0;
			if( nCondID > 0 && pEnterSox->IsExistItem( nCondID ) )
			{
				DNTableCell *pHellCell = pEnterSox->GetFieldFromLablePtr( nCondID, "_HellMapID" );
				if( pHellCell && pHellCell->GetInteger() > 0 )
					bHasHellMap = true;
			}
		}
		m_vecDungeonLevelButton[5]->Show( bHasHellMap );
		m_vecDungeonLevelButton[5]->Enable( bHasHellMap );
	}

	if( GetInterface().GetDungeonEnterNotifierDlg() && pInfo->m_pButtonGate )
	{
		GetInterface().GetDungeonEnterNotifierDlg()->SetDungeonNotice(
			pInfo->m_pButtonGate->GetDungeonName(),
			pInfo->m_pButtonGate->GetDungeonDesc() );
	}

#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
	GetInterface().ShowDungeonLimitDlg( true , pInfo->m_nMapIndex );
#endif

}


void CDnDungeonEnterLevelDlg::UpdateLevelButton( char *cState, int nAbyssMinLvl, int nAbyssMaxLvl, int nAbyssNeedQuestID )
{
	m_vecDungeonLevelButton[0]->Enable( ( cState[0] == 1 ) ? true : false );
	m_vecDungeonLevelButton[1]->Enable( ( cState[1] == 1 ) ? true : false );
	m_vecDungeonLevelButton[2]->Enable( ( cState[2] == 1 ) ? true : false );
	m_vecDungeonLevelButton[3]->Enable( ( cState[3] == 1 ) ? true : false );
	m_vecDungeonLevelButton[4]->Enable( ( cState[4] == 1 ) ? true : false );

	m_vecDungeonLevelButton[0]->SetChecked( true );
	if( CDnActor::s_hLocalActor ) {


		if (CDnActor::s_hLocalActor->GetLevel() < 15)
			m_vecDungeonLevelButton[4]->Enable( false );
		//
		if( CDnActor::s_hLocalActor->GetLevel() > 0 && CDnActor::s_hLocalActor->GetLevel() <= 5 ) {
			m_vecDungeonLevelButton[0]->SetChecked( true );
		}
		else if( CDnPartyTask::IsActive() ) {

			// Divided into two conditions.
			TDUNGEONDIFFICULTY difficultyLevel = Dungeon::Difficulty::Easy;

			if (GetPartyTask().GetPartyRole() == CDnPartyTask::LEADER && 
				CommonUtil::IsValidDungeonDifficulty(GetPartyTask().GetPartyTargetStageDifficulty()))
			{
				difficultyLevel = GetPartyTask().GetPartyTargetStageDifficulty();
			}
			else
			{
				// Otherwise, set based on player count (single-player also falls here).
				int nPartyMemberCount = GetPartyTask().GetPartyCount();
				switch( nPartyMemberCount )
				{
				case 1:	difficultyLevel = Dungeon::Difficulty::Normal;	break;	// Normal when solo (1 player)
				case 2:	difficultyLevel = Dungeon::Difficulty::Hard;	break;	// Hard difficulty for 2 players
				case 3:
				case 4:	difficultyLevel = Dungeon::Difficulty::Master;	break;	// Master difficulty for 3 or 4 players
				default:difficultyLevel = Dungeon::Difficulty::Normal;	break;	// Exceptional cases default to normal.
				}
			}

			if (difficultyLevel >= 0 && difficultyLevel < (int)m_vecDungeonLevelButton.size())
			{
				// Try selecting according to the configured difficulty; if locked, select the closest difficulty.
				for( int i = difficultyLevel; i >= 0; --i )
				{
					if( m_vecDungeonLevelButton[i]->IsEnable() ) {
						m_vecDungeonLevelButton[i]->SetChecked( true );
						break;
					}
				}
				// Re-check because there are cases where only Abyss is turned on
				if( !m_vecDungeonLevelButton[difficultyLevel]->IsEnable() ) {
					for( int i = 0; i < 5; i++ ) {
						if( m_vecDungeonLevelButton[i]->IsEnable() ) {
							m_vecDungeonLevelButton[i]->SetChecked( true );
							break;
						}
					}
				}

#ifdef PRE_WORLDCOMBINE_PARTY
				if (CDnPartyTask::GetInstance().IsPartyType(_WORLDCOMBINE_PARTY))
				{
					for (int i = 0; i < 5; i++)
					{
						m_vecDungeonLevelButton[i]->Enable(i == difficultyLevel);
					}
				}
#endif
			}
		}
	}

	m_vecDungeonLevelButton[4]->ClearTooltipText();
	if( cState[4] != 1 ) {
		WCHAR wszStr[512] = { 0, };
		if( cState[4] & 0x10 ) swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 920 ), nAbyssMinLvl );
		else if( cState[4] & 0x20 ) {
			swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 921 ) );
		}
		m_vecDungeonLevelButton[4]->SetTooltipText( wszStr );
	}
}

void CDnDungeonEnterLevelDlg::UpdateRecommandLevel( std::vector<int> &vecRecommandLevel )
{
	m_vecRecommandLevel.clear();

	for( int i=0; i<(int)vecRecommandLevel.size(); i++ )
	{
		m_vecRecommandLevel.push_back( vecRecommandLevel[i] );
	}
}

void CDnDungeonEnterLevelDlg::UpdateRecommandPartyCount( std::vector<int> &vecRecommandPartyCount )
{
	m_vecRecommandPartyCount.clear();

	for( int i=0; i<(int)vecRecommandPartyCount.size(); i++ )
	{
		m_vecRecommandPartyCount.push_back( vecRecommandPartyCount[i] );
	}
}

void CDnDungeonEnterLevelDlg::UpdateMaxUsableCoin( int nMaxUsableCoinCount )
{
	if( nMaxUsableCoinCount == -1 ) {
		m_pMaxUsableCoin->ClearText();
	}
	else {
		WCHAR wszTemp[64];
		swprintf_s( wszTemp, _countof(wszTemp), L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2378 ), nMaxUsableCoinCount );
		m_pMaxUsableCoin->SetText( wszTemp );
	}
}

void CDnDungeonEnterLevelDlg::UpdateNeedItem( int nItemID, int nCount, char cPermitFlag, std::vector<DnActorHandle> *pVecList )
{
	SAFE_DELETE( m_pNeedItem );

	if( nItemID > 0 )
	{
		TItemInfo Info;
		if( CDnItem::MakeItemInfo( nItemID, 0, Info ) == false ) return;
		m_pNeedItem = GetItemTask().CreateItem( Info );

		m_pItemSlotButton->SetItem(m_pNeedItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
		m_pNeedItemStatic->Show( true );

		// Handle Enter button after checking if item exists
		if( GetItemTask().GetCharInventory().GetItemCount( nItemID ) >= nCount ) {
			m_pItemSlotButton->SetRegist( false );
			m_bEnoughNeedItem = true;
		}
#if defined(PRE_ADD_68838)
		//If the target item is used after clearing the dungeon..
		else if(m_pNeedItem->GetItemType() == ITEMTYPE_STAGE_COMPLETE_USEITEM)
		{
			int nUserTicketCount = 0;
			// Handle Enter button after checking if item exists
			if(CDnItem::IsCashItem(nItemID))
				nUserTicketCount = GetItemTask().GetCashInventory().GetItemCount( nItemID );
			else
				nUserTicketCount = GetItemTask().GetCharInventory().GetItemCount( nItemID );
			
			if( nUserTicketCount >= nCount ) 
			{
				m_pItemSlotButton->SetRegist( false );
				m_bEnoughNeedItem = true;
			}
			else
			{
				m_pItemSlotButton->SetRegist( true );
				m_bEnoughNeedItem = false;
			}
		}
#endif  // #if defined(PRE_ADD_68838)
		else {
			m_pItemSlotButton->SetRegist( true );
			m_bEnoughNeedItem = false;
		}

		// For now, there's no request to turn the text red when quantity is insufficient.
		if( nCount > 1 ) {
			WCHAR wszTemp[64];
			swprintf_s(wszTemp, _countof(wszTemp), L"X %d", nCount );
			m_pNeedItemCount->SetText(wszTemp);
		}
		else {
			m_pNeedItemCount->SetText(L"");
		}
	}
	else
	{
		m_pItemSlotButton->ResetSlot();
		m_pNeedItemStatic->Show( false );
//		m_bEnoughNeedItem = true;
		m_pNeedItemCount->SetText(L"");
	}

	// Save data sent by server.
	m_cDungeonEnterPermit = cPermitFlag;
	if( pVecList ) m_hVecPermitActorList = *pVecList;

	// Called somewhat oddly here because NeedItem was added later
	// Called this way to minimize changes to the existing structure.
	SetControlState();
}

void CDnDungeonEnterLevelDlg::SetSelectDungeonInfo( int nMapIndex, int nDifficult )
{
	CDnGameTask *pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstance().GetTask( "GameTask" ));
	if( !pGameTask ) return;
	int nReadyGateIndex = pGameTask->GetReadyGateIndex();
	if( !CDnWorld::IsActive() ) return;
	CDnWorld::DungeonGateStruct *pGateStruct = (CDnWorld::DungeonGateStruct *)CDnWorld::GetInstance().GetGateStruct( nReadyGateIndex );
	if( !pGateStruct ) return;

	int nRecommandLv = 0, nRecommandCount = 0, nRebirthLimit = -1;
	WCHAR *wszName = L"";

	for( DWORD i=0; i<pGateStruct->pVecMapList.size(); i++ ) {
		CDnWorld::DungeonGateStruct *pChildGate = (CDnWorld::DungeonGateStruct *)pGateStruct->pVecMapList[i];
		if( pChildGate->nMapIndex == nMapIndex ) {
			wszName = (WCHAR *)pChildGate->szMapName.c_str();
			nRecommandLv = pChildGate->nRecommendLevel[nDifficult];
			nRecommandCount = pChildGate->nRecommendPartyCount[nDifficult];
			nRebirthLimit = pChildGate->nMaxUsableCoin;
			break;
		}
	}
	SetImage( nMapIndex );

	m_pSelectName->SetText( wszName );
	m_pSelectLevel->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2305+nDifficult ) );

	wchar_t wszTemp[128] = {0};
	swprintf_s( wszTemp, _countof(wszTemp), L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2368 ), nRecommandLv );
	m_pRecommLevel->SetText( wszTemp );

	swprintf_s( wszTemp, _countof(wszTemp), L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2369 ), nRecommandCount );
	m_pRecommCount->SetText( wszTemp );

	if( nRebirthLimit == -1 ) {
		m_pMaxUsableCoin->ClearText();
	}
	else {
		swprintf_s( wszTemp, _countof(wszTemp), L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2378 ), nRebirthLimit );
		m_pMaxUsableCoin->SetText( wszTemp );
	}

	SetFatigueDegree( nMapIndex, nDifficult );
}

void CDnDungeonEnterLevelDlg::SetFatigueDegree( int nMapIndex, int nDifficult )
{
	int nPartyMemberCount = GetPartyTask().GetPartyCount();
	float fFatiguePercent = CFatigueAdjustmentTable::GetInstance().GetValue( nPartyMemberCount, nDifficult );

	m_nBeginnerMaxLevel = 0;
	m_nBaseFatigue = 0;
	m_nBeginnerDecreaseFatigue = 0;
	m_nMaxLevelDecreaseFatigue = 0;

	DNTableFileFormat*  pSoxMap = GetDNTable( CDnTableDB::TMAP );
	if( pSoxMap == NULL ) return;
	int nMapSid = pSoxMap->GetFieldFromLablePtr( nMapIndex, "_MapSid" )->GetInteger();
	if( nMapSid > 0 )
	{
		for( int i=0; i<pSoxMap->GetItemCount(); i++ )
		{
			int nItemID = pSoxMap->GetItemID( i );
			if( nMapSid == pSoxMap->GetFieldFromLablePtr( nItemID, "_MapSid" )->GetInteger() )
			{
				// Base fatigue
				int nBaseFatigue = 0;
				switch( nDifficult )
				{
					case 0:
						nBaseFatigue = (int)( fFatiguePercent * (float)( pSoxMap->GetFieldFromLablePtr( nItemID, "_FatigueEasy" )->GetInteger() ) );
						break;
					case 1:
						nBaseFatigue = (int)( fFatiguePercent * (float)( pSoxMap->GetFieldFromLablePtr( nItemID, "_DecFatigue" )->GetInteger() ) );
						break;
					case 2:
						nBaseFatigue = (int)( fFatiguePercent * (float)( pSoxMap->GetFieldFromLablePtr( nItemID, "_FatigueHard" )->GetInteger() ) );
						break;
					case 3:
						nBaseFatigue = (int)( fFatiguePercent * (float)( pSoxMap->GetFieldFromLablePtr( nItemID, "_FatigueMaster" )->GetInteger() ) );
						break;
					case 4:
						nBaseFatigue = (int)(fFatiguePercent * (float)(pSoxMap->GetFieldFromLablePtr(nItemID, "_FatigueAbyss")->GetInteger()));
						break;
					case 5:
						nBaseFatigue = (int)(fFatiguePercent * (float)(pSoxMap->GetFieldFromLablePtr(nItemID, "_FatigueChaos")->GetInteger()));
						break;
					case 6:
						nBaseFatigue = (int)(fFatiguePercent * (float)(pSoxMap->GetFieldFromLablePtr(nItemID, "_FatigueHell")->GetInteger()));
						break;
				}
				m_nBaseFatigue += nBaseFatigue;
				
				// Beginner FTG reduction amount
				DNTableFileFormat*  pSoxPlayerCommonLevel = GetDNTable( CDnTableDB::TPLAYERCOMMONLEVEL );
				if( pSoxPlayerCommonLevel )
				{
					for( int j=0; j<pSoxPlayerCommonLevel->GetItemCount(); j++ )
					{
						int nItemID = pSoxPlayerCommonLevel->GetItemID( j );
						if( pSoxPlayerCommonLevel->GetFieldFromLablePtr( nItemID, "_Fatigueratio" )->GetInteger() > 0 )
							m_nBeginnerMaxLevel = nItemID;
						else
							break;
					}

					int nMyLevel = CDnActor::s_hLocalActor->GetLevel();
					if( nMyLevel <= m_nBeginnerMaxLevel )	// Beginner?
					{
						m_nBeginnerDecreaseFatigue += nBaseFatigue * pSoxPlayerCommonLevel->GetFieldFromLablePtr( nMyLevel, "_Fatigueratio" )->GetInteger() / 10000; 
					}
				}

				// Fatigue reduction amount for max-level characters
				CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask( "GameTask" ));
				int nMaxLevelCharNum = 0;
				if( pGameTask )
					nMaxLevelCharNum = pGameTask->GetMaxCharCount();

				if( nMaxLevelCharNum > 0 )
				{
					DNTableFileFormat*  pSoxLevelPromo = GetDNTable( CDnTableDB::TLEVELPROMO );
					if( pSoxLevelPromo )
					{
						for( int j=0; j<pSoxLevelPromo->GetItemCount(); j++ )
						{
							int nItemID = pSoxLevelPromo->GetItemID( j );

							int nConditionValue = atoi( pSoxLevelPromo->GetFieldFromLablePtr( nItemID, "_UserConditionValue1" )->GetString() );
							int nRewardType = pSoxLevelPromo->GetFieldFromLablePtr( nItemID, "_RewardType1")->GetInteger();

							if( nConditionValue == nMaxLevelCharNum	&& nRewardType == ePromotionType::PROMOTIONTYPE_DECREASEFATIGUE )
							{
								float fRatio = (float)atof( pSoxLevelPromo->GetFieldFromLablePtr( nItemID, "_RewardValue1" )->GetString() );
								m_nMaxLevelDecreaseFatigue += (int)( nBaseFatigue * fRatio );
								break;
							}
						}
					}
				}
			}
		}
	}

	int nTotalFatigueDegree = m_nBaseFatigue - m_nBeginnerDecreaseFatigue - m_nMaxLevelDecreaseFatigue;
	if( nTotalFatigueDegree < 0 )
		nTotalFatigueDegree = 0;

	wchar_t wszTemp[128] = {0};
	swprintf_s( wszTemp, _countof(wszTemp), L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3280 ), nTotalFatigueDegree );

	if( m_nBeginnerDecreaseFatigue > 0 )
	{
		if (m_pBeginnerDungeonFatigueDegree)
		{
			m_pBeginnerDungeonFatigueDegree->Show( true );
			m_pBeginnerDungeonFatigueDegree->SetText( wszTemp );
		}

		if (m_pDungeonFatigueDegree)
		{
			m_pDungeonFatigueDegree->ClearText();
			m_pDungeonFatigueDegree->Show( false );
		}

		if (m_pStaticBeginnerMark)
			m_pStaticBeginnerMark->Show( true );
	}
	else
	{
		if (m_pBeginnerDungeonFatigueDegree)
		{
			m_pBeginnerDungeonFatigueDegree->ClearText();
			m_pBeginnerDungeonFatigueDegree->Show( false );
		}
		if (m_pDungeonFatigueDegree)
		{
			m_pDungeonFatigueDegree->Show( true );
			m_pDungeonFatigueDegree->SetText( wszTemp );
		}

		if (m_pStaticBeginnerMark)
			m_pStaticBeginnerMark->Show( false );
	}
}

#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
bool CDnDungeonEnterLevelDlg::IsCloseProcessingByUser() const
{
	return m_bCloseProcessingByUser;
}
#endif


