#include "StdAfx.h"
#include "DnDarklairEnterLevelDlg.h"
#include "DnTableDB.h"
#include "DnItemTask.h"
#include "TaskManager.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnDarklairEnterLevelDlg::CDnDarklairEnterLevelDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_fElapsedTime( 0.f )
, m_pNeedItem( NULL )
, m_pButtonEnter( NULL )
, m_pButtonCancel( NULL )
, m_pStaticTotalRound( NULL )
, m_pStaticLevel( NULL )
, m_pStaticItemCount( NULL )
, m_pStaticTimer( NULL )
, m_pStaticName( NULL )
, m_pButtonItemSlot( NULL )
, m_pStaticItemSlot( NULL )
, m_pTextureDungeonImage( NULL )
{
}

CDnDarklairEnterLevelDlg::~CDnDarklairEnterLevelDlg()
{
	SAFE_RELEASE_SPTR( m_hIntroArea );
}

void CDnDarklairEnterLevelDlg::InitialUpdate()
{
	m_pButtonEnter = GetControl<CEtUIButton>( "ID_BUTTON_ENTER" );
	m_pButtonCancel = GetControl<CEtUIButton>( "ID_BUTTON_CANCEL" );
	m_pStaticTotalRound = GetControl<CEtUIStatic>( "ID_SELECT_ALLROUND" );
	m_pStaticLevel = GetControl<CEtUIStatic>( "ID_SELECT_ROUND" );
	m_pStaticItemCount = GetControl<CEtUIStatic>( "ID_STATIC_COUNT" );
	m_pStaticTimer = GetControl<CEtUIStatic>( "ID_STATIC_TIMER" );
	m_pStaticName = GetControl<CEtUIStatic>( "ID_SELECT_NAME" );
	m_pButtonItemSlot = GetControl<CDnItemSlotButton>( "ID_ITEM" );
	m_pStaticItemSlot = GetControl<CEtUIStatic>( "ID_ITEMSLOT" );
	m_pTextureDungeonImage = GetControl<CEtUITextureControl>( "ID_TEXTUREL_AREA" );
}

void CDnDarklairEnterLevelDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DarkLairEnterLevelDlg.ui" ).c_str(), bShow );
}

void CDnDarklairEnterLevelDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( strcmp( pControl->GetControlName(), "ID_BUTTON_ENTER" ) == 0 || strcmp( pControl->GetControlName(), "ID_BUTTON_CANCEL" ) == 0 ) 
		{
			m_pButtonEnter->Enable( false );
			m_pButtonCancel->Enable( false );
		}
	}
}

void CDnDarklairEnterLevelDlg::Show( bool bShow )
{
	if( m_bShow == bShow ) return;

	if( bShow )
	{
		m_fElapsedTime = 60.f;
		SetTimer( (int)m_fElapsedTime );
		m_pButtonEnter->Enable( true );
		m_pButtonCancel->Enable( true );
	}

	CDnCustomDlg::Show( bShow );
}

void CDnDarklairEnterLevelDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );
	if( IsShow() )
	{
		if( m_fElapsedTime > 0.f ) 
		{
			int nPrevTime = (int)m_fElapsedTime;
			m_fElapsedTime -= fElapsedTime;
			if( m_fElapsedTime <= 0.f ) m_fElapsedTime = 0.f;

			if( nPrevTime != (int)m_fElapsedTime )
				SetTimer( (int)m_fElapsedTime );
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
}

bool CDnDarklairEnterLevelDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
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

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnDarklairEnterLevelDlg::UpdateFloorInfo( int nMapIndex, int nMinLevel, char cPermitFlag )
{
	CDnGameTask *pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstance().GetTask( "GameTask" ));
	if( !pGameTask ) return;
	int nReadyGateIndex = pGameTask->GetReadyGateIndex();
	if( !CDnWorld::IsActive() ) return;
	CDnWorld::DungeonGateStruct *pReadyGateStruct = (CDnWorld::DungeonGateStruct *)CDnWorld::GetInstance().GetGateStruct( nReadyGateIndex );
	if( !pReadyGateStruct ) return;

	int nRecommandLv = 0, nRecommandCount = 0, nRebirthLimit = -1;
	WCHAR *wszName = L"";

	CDnWorld::DungeonGateStruct* pGateStruct = NULL;
	for( DWORD i=0; i<pReadyGateStruct->pVecMapList.size(); i++ ) 
	{
		CDnWorld::DungeonGateStruct* pChildGate = (CDnWorld::DungeonGateStruct *)pReadyGateStruct->pVecMapList[i];
		if( pChildGate->nMapIndex == nMapIndex )
		{
			pGateStruct = pChildGate;
			break;
		}
	}

	if( pGateStruct == NULL )
		return;

	// Set dungeon name
	m_pStaticName->SetText( pGateStruct->szMapName.c_str() );

	SAFE_RELEASE_SPTR( m_hIntroArea );
	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TDLMAP );
	if( pSox && pSox->IsExistItem( pGateStruct->nMapIndex ) ) 
	{
		char szImageName[128] = {0};
		sprintf_s( szImageName, _countof(szImageName), "DungeonImage_%d.dds", pGateStruct->nMapIndex );

		m_hIntroArea = LoadResource( CEtResourceMng::GetInstance().GetFullName( szImageName ).c_str(), RT_TEXTURE );
		if( m_hIntroArea )
			m_pTextureDungeonImage->SetTexture( m_hIntroArea, 0, 0, m_hIntroArea->Width(), m_hIntroArea->Height() );

		wchar_t wszStr[128] = {0};
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		bool bChallengeType = pSox->GetFieldFromLablePtr( pGateStruct->nMapIndex, "_ChallengeType" )->GetInteger() == 0 ? false : true;
		if( true == bChallengeType )
		{
			int nStringID = pSox->GetFieldFromLablePtr( pGateStruct->nMapIndex, "_GameModeName" )->GetInteger();
			swprintf_s( wszStr, _countof(wszStr), L"%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID ) );
		}
		else
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		{
			int nStageCount = pSox->GetFieldFromLablePtr( pGateStruct->nMapIndex, "_StageCount" )->GetInteger();
			swprintf_s( wszStr, _countof(wszStr), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2383 ), nStageCount );
		}
		m_pStaticTotalRound->SetText( wszStr );
	}

	// Set required items
	SAFE_DELETE( m_pNeedItem );

	bool bShowSlot = false;
	if( pGateStruct->nNeedItemID > 0 )
	{
		TItemInfo Info;
		if( CDnItem::MakeItemInfo( pGateStruct->nNeedItemID, 0, Info ) == false ) return;
		m_pNeedItem = GetItemTask().CreateItem( Info );
		if( m_pNeedItem ) 
		{
			if( pGateStruct->nNeedItemCount > 1 ) 
			{
				WCHAR wszTemp[64];
				swprintf_s(wszTemp, _countof(wszTemp), L"X %d", pGateStruct->nNeedItemCount );
				m_pStaticItemCount->SetText(wszTemp);
			}
			else 
			{
				m_pStaticItemCount->SetText( L"" );
			}

			m_pButtonItemSlot->SetItem( m_pNeedItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
		}
		bShowSlot = true;
	}
	else
		m_pStaticItemCount->SetText( L"" );

	m_pStaticItemSlot->Show( bShowSlot );
	m_pButtonItemSlot->Show( bShowSlot );

#if !defined(PRE_FIX_50460)
	//The timer is set by calling Show(true). No need to set the timer again here.
	m_fElapsedTime = 60.f;
	SetTimer( (int)m_fElapsedTime );
#endif // PRE_FIX_50460

	CDnPartyTask::PartyRoleEnum partyState = GetPartyTask().GetPartyRole();
	bool bEnableButton = ( partyState == CDnPartyTask::MEMBER ) ? false : true;

	if( cPermitFlag != CDnWorld::PermitEnter )
		bEnableButton = false;

	m_pButtonEnter->Enable( bEnableButton );
	m_pButtonCancel->Enable( true );

	wchar_t wszStr[128] = {0};
	swprintf_s( wszStr, _countof(wszStr), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 833 ), nMinLevel );
	m_pStaticLevel->SetText( wszStr );
}

void CDnDarklairEnterLevelDlg::SetTimer( int nTime )
{
	wchar_t wszTimer[80] = {0};
	swprintf_s( wszTimer, _countof(wszTimer), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2312 ), nTime );
	m_pStaticTimer->SetText( wszTimer );
}
