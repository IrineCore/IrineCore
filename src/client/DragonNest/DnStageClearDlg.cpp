#include "StdAfx.h"
#include "DnStageClearDlg.h"
#include "DnDungeonClearMoveDlg.h"
#include "DnInterfaceDlgID.h"
#include "DnLocalPlayerActor.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "GameSendPacket.h"
#include "MAPartsBody.h"
#include "DnTableDB.h"
#include "GlobalValue.h"
#include "DnStageClearMoveDlg.h"
#include "DnCommonTask.h"
#include "DnStageClearCPRankDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnStageClearDlg::CDnStageClearDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
//	, m_pDungeonClearMoveDlg(NULL)
	, m_pFace(NULL)
	, m_pTextureRank(NULL)
	, m_pName(NULL)
	, m_pStaticRemainSelectBox(NULL)
	, m_pTime(NULL)
	, m_pTimeBase(NULL)
	, m_fElapsedTime(0.0f)
	, m_nCount(0)
	, m_bTimer(false)
	, m_nSeletBonusBoxIndex(0)
	, m_nCurPhase(0)
	, m_nBoxCount(0)
	, m_Phase(PHASE_NONE)
	, m_CurBoxIdxPresented(0)
	, m_fPhaseTime(0.f)
	, m_pIdentifySlot(NULL)
	, m_pIdentifySlotBase(NULL)
	, m_pIdentifyMsg(NULL)
	, m_pIdentifyBack(NULL)
	, m_pIdentifyItem(NULL)
	, m_bRequestIdentify(false)
	, m_pStaticSelectBoxNotice(NULL)
#if defined( PRE_ADD_CP_RANK )
	, m_pStageClearCPRankDlg( NULL )
#endif	// #if defined( PRE_ADD_CP_RANK )
{
	m_nSelectRemainCount = 0;

#if defined( PRE_ADD_CP_RANK )
	memset( m_pBestPlayer, NULL, sizeof(m_pBestPlayer) );
	memset( m_pNewRecord, NULL, sizeof(m_pNewRecord) );
#endif	// #if defined( PRE_ADD_CP_RANK )
}

CDnStageClearDlg::~CDnStageClearDlg(void)
{
#if defined( PRE_ADD_CP_RANK )
	SAFE_DELETE( m_pStageClearCPRankDlg );
#endif	// #if defined( PRE_ADD_CP_RANK )

	SAFE_DELETE( m_pIdentifyItem );
	SAFE_RELEASE_SPTR( m_hRankTexture );

	for ( int i = 0 ; i < SoundType::MAX ; i++ )
	{
		if( CEtSoundEngine::GetInstancePtr() )
			CEtSoundEngine::GetInstance().RemoveSound( m_nSoundIndexArray[i] );
	}
}

void CDnStageClearDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "StageClearDlg.ui" ).c_str(), bShow );
}

void CDnStageClearDlg::InitialUpdate()
{
	m_pFace = GetControl<CDnGaugeFace>("ID_FACE");
	m_pFace->SetFaceType( FT_PLAYER );
	m_pTextureRank = GetControl<CEtUITextureControl>("ID_TEXTUREL_RANK");
	m_pName = GetControl<CEtUIStatic>("ID_NAME");
	m_pTime = GetControl<CEtUIStatic>("ID_TIME");
	m_pTimeBase = GetControl<CEtUIStatic>("ID_STATIC_TIMEBASE");
	m_pStaticRemainSelectBox = GetControl<CEtUIStatic>("ID_SELECT");
	m_pStaticRemainSelectBox->Show( true );
	m_pStaticSelectBoxNotice = GetControl<CEtUIStatic>("ID_STATIC1");
	m_pStaticSelectBoxNotice->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3214)); // UISTRING : Cannot change after selecting a chest.

	m_pIdentifySlot = GetControl<CDnItemSlotButton>("ID_ITEM_IDENTIFY");
	m_pIdentifySlotBase = GetControl<CEtUIStatic>("ID_IDENTIFY_BASE1");
	m_pIdentifyMsg = GetControl<CEtUIStatic>("ID_STATIC_MSG");
	m_pIdentifyBack = GetControl<CEtUIStatic>("ID_IDENTIFY_BASE0");
	m_pIdentifySlot->Show( false );
	m_pIdentifySlotBase->Show( false );
	m_pIdentifyMsg->Show( false );
	m_pIdentifyBack->Show( false );

	m_sBonusBoxButton[0].m_pBonusBoxButton = GetControl<CDnBonusBoxButton>("ID_BONUSBOX_00_00");
	m_sBonusBoxButton[1].m_pBonusBoxButton = GetControl<CDnBonusBoxButton>("ID_BONUSBOX_00_01");
	m_sBonusBoxButton[2].m_pBonusBoxButton = GetControl<CDnBonusBoxButton>("ID_BONUSBOX_00_02");
	m_sBonusBoxButton[3].m_pBonusBoxButton = GetControl<CDnBonusBoxButton>("ID_BONUSBOX_00_03");

	m_sBonusBoxButton[0].m_pItemSlotButton = GetControl<CDnItemSlotButton>("ID_ITEM_00_00");
	m_sBonusBoxButton[1].m_pItemSlotButton = GetControl<CDnItemSlotButton>("ID_ITEM_00_01");
	m_sBonusBoxButton[2].m_pItemSlotButton = GetControl<CDnItemSlotButton>("ID_ITEM_00_02");
	m_sBonusBoxButton[3].m_pItemSlotButton = GetControl<CDnItemSlotButton>("ID_ITEM_00_03");

	char szBase[32]={0};
	char szFace[32]={0};
	char szRank[32]={0};
	char szName[32]={0};
	char szBonusBox[32]={0};
	char szItem[32]={0};

	for( int i=0; i<3; i++ )
	{
		SStageClearList stageClearList;

		sprintf_s( szBase, 32, "ID_BASE_%02d", i );
		sprintf_s( szFace, 32, "ID_FACE_%02d", i );
		sprintf_s( szRank, 32, "ID_RANK_%02d", i );
		sprintf_s( szName, 32, "ID_NAME_%02d", i );

		stageClearList.m_pBase = GetControl<CEtUIStatic>(szBase);
		stageClearList.m_pFace = GetControl<CDnGaugeFace>(szFace);
		stageClearList.m_pFace->SetFaceType( FT_PLAYER );
		stageClearList.m_pTextureRank = GetControl<CEtUITextureControl>(szRank);
		stageClearList.m_pName = GetControl<CEtUIStatic>(szName);

		for( int j=0; j<4; j++ )
		{
			sprintf_s( szBonusBox, 32, "ID_BONUSBOX_%02d_%02d", i+1, j );
			sprintf_s( szItem, 32, "ID_ITEM_%02d_%02d", i+1, j );

			stageClearList.m_sBonusBoxButton[j].m_pBonusBoxButton = GetControl<CDnBonusBoxButton>(szBonusBox);
			stageClearList.m_sBonusBoxButton[j].m_pBonusBoxButton->SetUseHandle( false );
			stageClearList.m_sBonusBoxButton[j].m_pItemSlotButton = GetControl<CDnItemSlotButton>(szItem);
		}

		stageClearList.Show( false );
		m_vecStageClearList.push_back(stageClearList);
	}

#if defined( PRE_ADD_CP_RANK )
	char szLabel[256] = {0,};
	for( int itr = 0; itr < PLAYER_UI_NUMBER; ++itr )
	{
		sprintf_s( szLabel, "ID_STATIC_BEST%d", itr );
		m_pBestPlayer[itr] = GetControl<CEtUIStatic>( szLabel );

		sprintf_s( szLabel, "ID_STATIC_NEW%d", itr );
		m_pNewRecord[itr] = GetControl<CEtUIStatic>( szLabel );

		m_pBestPlayer_UICoord[itr] = m_pBestPlayer[itr]->GetUICoord();
		m_pNewRecord_UICoord[itr] = m_pNewRecord[itr]->GetUICoord();
	}

	m_pStageClearCPRankDlg = new CDnStageClearCPRankDlg();
	m_pStageClearCPRankDlg->Initialize( false );
#endif	// #if defined( PRE_ADD_CP_RANK )

	m_hRankTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( "DungeonRankIcon.dds" ).c_str(), RT_TEXTURE );

	m_nSoundIndexArray[SoundType::RECOMPENSE_WINDOW]	= CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10021 ), false, false );
	m_nSoundIndexArray[SoundType::BOX_MOUSEON]			= CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10022 ), false, false );
	m_nSoundIndexArray[SoundType::BOX_CLICK]			= CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10023 ), false, false );
	m_nSoundIndexArray[SoundType::BOX_OPEN]				= CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10024 ), false, false );
}

void CDnStageClearDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundIndexArray[SoundType::RECOMPENSE_WINDOW] );
		SetClearInfo();

		// Initialize text.
		m_pStaticSelectBoxNotice->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3214)); // UISTRING : Cannot change after selecting a chest.

		m_bTimer = true;
		m_nCount = 5;
		m_fElapsedTime = 0.f;
		m_pTimeBase->Show( true );

		DNTableFileFormat* pMapSox = GetDNTable(CDnTableDB::TMAP);
		if (pMapSox != NULL)
		{
			int nID = CGlobalInfo::GetInstance().m_nCurrentMapIndex;

#if defined( PRE_ADD_VIP_CLEAR_BOX ) && defined( PRE_ADD_VIP )
			const CDnVIPDataMgr* pMgr = GetInterface().GetLocalPlayerVIPDataMgr();
			if (pMgr != NULL && pMgr->IsVIPMode())
			{
				int nVipBonusItemCount = pMapSox->GetFieldFromLablePtr(nID, "_VipClear")->GetInteger();
				if (nVipBonusItemCount > 0)
					m_pStaticSelectBoxNotice->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3241)); // UISTRING : VIP members can select 2 chests. Selection cannot be changed.
			}

#else	// PRE_ADD_VIP_CLEAR_BOX
			CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
			if( pTask && pTask->IsPCBangClearBox() )
			{
				int nVipBonusItemCount = pMapSox->GetFieldFromLablePtr(nID, "_VipClear")->GetInteger();
				if (nVipBonusItemCount > 0)
					m_pStaticSelectBoxNotice->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3316)); // UISTRING : Cannot change after selecting a chest (Premium PC Bang benefit: select 2 (excludes some content))
			}
#endif // #if defined( PRE_ADD_VIP_CLEAR_BOX ) && defined( PRE_ADD_VIP )
		}


#ifdef PRE_ADD_NEWCOMEBACK
		if( GetInterface().GetComebackLocalUser() && m_nSelectRemainCount > 1 )
			m_pStaticSelectBoxNotice->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4951)); // "You can select 2 with the Returnee title benefit (excluding some content). Cannot change once chest is selected."
#endif // PRE_ADD_NEWCOMEBACK

		wchar_t wszTemp[64] = {0};
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2367 ), m_nCount );
		m_pTime->SetText( wszTemp );

		SetAllQuestionBox();
	}
	else
	{
		m_nCurPhase = 0;
		m_nBoxCount = 0;

		for( int i=0; i<(int)m_vecStageClearList.size(); i++ )
		{
			for( int j=0; j<4; j++ )
			{
				m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->Show( false );
				m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetItemLight( false );
				m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->Enable( true );
				m_vecStageClearList[i].m_sBonusBoxButton[j].m_pItemSlotButton->Show( false );
			}

			m_vecStageClearList[i].Show( false );
			m_vecStageClearList[i].Clear();
		}

		for( int i=0; i<4; i++ )
		{
			m_sBonusBoxButton[i].m_pBonusBoxButton->Show( false );
			m_sBonusBoxButton[i].m_pBonusBoxButton->SetItemLight( false );
			m_sBonusBoxButton[i].m_pBonusBoxButton->Enable( true );
			m_sBonusBoxButton[i].m_pItemSlotButton->Show( false );
		}
		/*
		CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
		if( pTask ) pTask->StopStageClearBGM();
		*/
	}

#if defined( PRE_ADD_CP_RANK )
	CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	if( NULL != pGameTask && true == pGameTask->IsDungeonClearBestInfo() )
		m_pStageClearCPRankDlg->Show( bShow );
#endif	// #if defined( PRE_ADD_CP_RANK )

	CEtUIDialog::Show( bShow );
}

void CDnStageClearDlg::Process( float fElapsedTime )
{
	if( m_bTimer )
	{
		if( m_nCount > 0 )
		{
			m_fElapsedTime += fElapsedTime;

			if( m_fElapsedTime > 1.0f )
			{
				m_nCount--;

				wchar_t wszTemp[64] = {0};
				swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2367 ), m_nCount );
				m_pTime->SetText( wszTemp );

				m_fElapsedTime -= 1.0f;
			}
		}
		else
		{
			/*
			if( m_nSeletBonusBoxIndex == -1 )
			{
				m_nSeletBonusBoxIndex = 0;
				m_sBonusBoxButton[m_nSeletBonusBoxIndex].m_pBonusBoxButton->SetChecked( true );
			}
			
			SendSelectRewardItem( (char)m_nSeletBonusBoxIndex );
			*/
			m_bTimer = false;
			m_pTime->SetText( L" " );
			m_pStaticRemainSelectBox->SetText( L"" );
			m_pTimeBase->Show( false );
		}
	}
	else if( m_fPhaseTime > 0.0f )
	{
		m_fPhaseTime -= fElapsedTime;
	}
	else
	{
		if (m_Phase != PHASE_NONE)
		{
			if (m_Phase == PHASE_CLOSED_BOX)
			{
				SetClosedBonusBox(m_CurBoxIdxPresented);
				m_fPhaseTime = 0.1f;
			}
			else if (m_Phase == PHASE_OPEN_BOX)
			{
				SetOpenBonusBox(m_CurBoxIdxPresented);
				m_fPhaseTime = 0.1f;
			}

			m_CurBoxIdxPresented++;
			if (m_CurBoxIdxPresented >= m_nBoxCount)
			{
				m_Phase = PHASE_NONE;
				m_CurBoxIdxPresented = 0;
			}
		}
	}

	CEtUIDialog::Process( fElapsedTime );	
}

#if defined ( PRE_ADD_NAMEDITEM_SYSTEM )
void CDnStageClearDlg::ShowBonusBox()
{
	m_Phase = PHASE_CLOSED_BOX;
	m_pStaticRemainSelectBox->SetText( L"" );
	m_CurBoxIdxPresented = 0;

	SetRewardBoxOrRewardItemInfo(true);
	SetAllPartyBonusBox();
}
#endif

void CDnStageClearDlg::OpenBonusBox()
{
#if defined ( PRE_ADD_NAMEDITEM_SYSTEM )
	SetRewardBoxOrRewardItemInfo(false);
#else
	m_Phase = PHASE_CLOSED_BOX;
	m_pStaticRemainSelectBox->SetText( L"" );
	m_CurBoxIdxPresented = 0;

	SetRewardItemInfo();
	SetAllPartyBonusBox();
#endif
}

void CDnStageClearDlg::OpenBonusItem()
{
	m_Phase = PHASE_OPEN_BOX;
	m_CurBoxIdxPresented = 0;
	SetAllPartyOpenBonusBox();
}

void CDnStageClearDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( strstr( pControl->GetControlName(), "ID_BONUSBOX_00_" ) && bTriggeredByUser )
		{
			CDnBonusBoxButton *pButton = (CDnBonusBoxButton*)pControl;
			if( m_Phase == PHASE_NONE && m_nSelectRemainCount > 0 )
			{
				int nSelectBoxIndex = pButton->GetTabID();
				SendSelectRewardItem( (char)nSelectBoxIndex );

				m_nSelectRemainCount--;
				if( m_nSelectRemainCount == 0 ) {
					for( int i=0; i<4; i++ ) {
						if( m_sBonusBoxButton[i].m_pBonusBoxButton )
							m_sBonusBoxButton[i].m_pBonusBoxButton->Enable( false );
					}
					m_pStaticRemainSelectBox->SetText( L"" );
				}
				else {
					pButton->Enable( false );
					WCHAR wszStr[64];
					swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2377 ), m_nSelectRemainCount );
					m_pStaticRemainSelectBox->SetText( wszStr );
				}

				pButton->SetChecked( false );
				CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundIndexArray[SoundType::BOX_CLICK] );
			}
			return;
		}
	}
	if( nCommand == EVENT_BUTTON_RCLICKED )
	{
		if( strstr( pControl->GetControlName(), "ID_BONUSBOX_00_" ) && bTriggeredByUser ) {
			CDnBonusBoxButton *pButton = (CDnBonusBoxButton*)pControl;
			if( m_pIdentifyItem && !m_bRequestIdentify && pButton->GetBonusBoxButtonType() == CDnBonusBoxButton::typeQuestion && m_nSelectRemainCount > 0 )
			{
				SendIdentifyRewardItem( (char)pButton->GetTabID() );
				m_bRequestIdentify = true;
			}
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnStageClearDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnStageClearDlg::OnBlindClose()
{
	SetCallback( NULL );
	Show( false );
}

void CDnStageClearDlg::OnBlindClosed()
{
	GetInterface().OpenBaseDialog();

	CDnLocalPlayerActor::LockInput(false);

	/*
	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pTask ) pTask->StopStageClearBGM();
	*/
}

void CDnStageClearDlg::SetClearInfo()
{
	CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	if( !pGameTask )
	{
		CDebugSet::ToLogFile( "CDnStageClearDlg::SetClearInfo, pGameTask is NULL!" );
		return;
	}

	m_bRequestIdentify = false;
	CDnDungeonClearImp *pClearImp = (CDnDungeonClearImp *)pGameTask->GetStageClearImp();
	if( !pClearImp ) return;

	std::vector<CDnDungeonClearImp::SDungeonClearInfo> &dungeonClearInfoList = pClearImp->GetDungeonClearInfoList();

#if defined( PRE_ADD_CP_RANK )
	ResetCPRank();

	bool bIsDungeonClearBestInfo = pGameTask->IsDungeonClearBestInfo();
	if( true == bIsDungeonClearBestInfo )
		SetCPRank( dungeonClearInfoList );
#endif	// #if defined( PRE_ADD_CP_RANK )

	for( int i=0, k=0; i<(int)dungeonClearInfoList.size(); i++ )
	{
		if( dungeonClearInfoList[i].sBaseInfo.nSessionID == CDnBridgeTask::GetInstance().GetSessionID() )
		{
		
			
			//
			m_pFace->SetFaceID( dungeonClearInfoList[i].nClassID-1 );

			EtTextureHandle hTexture = GetInterface().CloneMyPortrait();
			m_pFace->SetTexture( hTexture );

			m_pTextureRank->SetTexture( m_hRankTexture, dungeonClearInfoList[i].sBaseInfo.cRank*216, 0, 216, 256 );
			m_pName->SetText( dungeonClearInfoList[i].strName );

			m_nSelectRemainCount = dungeonClearInfoList[i].sBaseInfo.cSelectBoxCount;
			WCHAR wszStr[64] = {0,};
			swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2377 ), m_nSelectRemainCount );
			m_pStaticRemainSelectBox->SetText( wszStr );

			m_nBoxCount = dungeonClearInfoList[i].sBaseInfo.cShowBoxCount;

#if defined( PRE_ADD_CP_RANK )
			if( true == bIsDungeonClearBestInfo )
				SetCPRankUI( 0, dungeonClearInfoList[i].bNewRecord, dungeonClearInfoList[i].bBestPlayer );
#endif	// #if defined( PRE_ADD_CP_RANK )
		}
		else
		{
			m_vecStageClearList[k].m_pFace->SetFaceID( dungeonClearInfoList[i].nClassID-1 );

			for( DWORD m=0; m<GetPartyTask().GetPartyCount(); m++ )
			{
				CDnPartyTask::PartyStruct *pInfo = GetPartyTask().GetPartyData(m);
				if( !pInfo ) continue;
				if( pInfo->nSessionID == dungeonClearInfoList[i].sBaseInfo.nSessionID ) 
				{
					MAPartsBody *pPartsBody = dynamic_cast< MAPartsBody * >( pInfo->hActor.GetPointer() );
					if( ( pPartsBody ) && ( pPartsBody->GetObjectHandle() ) )
					{
						int nAniIndex = pInfo->hActor->GetCachedAniIndex( "Normal_Stand" );
						if( nAniIndex == -1 ) nAniIndex = pInfo->hActor->GetCachedAniIndex( "Stand" );	// If in ghost state, Normal_Stand may not exist.
						if( nAniIndex != -1 )
						{
							m_vecStageClearList[k].m_pFace->UpdatePortrait( pPartsBody->GetObjectHandle(), false, false, nAniIndex, 0.0f );
						}
						break;
					}
				}
			}

			m_vecStageClearList[k].m_pTextureRank->SetTexture( m_hRankTexture, dungeonClearInfoList[i].sBaseInfo.cRank*216, 0, 216, 256 );
			m_vecStageClearList[k].m_pName->SetText( dungeonClearInfoList[i].strName );
			m_vecStageClearList[k].m_nSessionID = dungeonClearInfoList[i].sBaseInfo.nSessionID;
			m_vecStageClearList[k].m_nBoxCount = dungeonClearInfoList[i].sBaseInfo.cShowBoxCount;
			m_vecStageClearList[k].Show( true );

#if defined( PRE_ADD_CP_RANK )
			if( true == bIsDungeonClearBestInfo )
				SetCPRankUI( k + 1, dungeonClearInfoList[i].bNewRecord, dungeonClearInfoList[i].bBestPlayer );
#endif	// #if defined( PRE_ADD_CP_RANK )

			k++;
		}
	}
}

#if defined ( PRE_ADD_NAMEDITEM_SYSTEM )
void CDnStageClearDlg::SetRewardBoxOrRewardItemInfo(bool bBoxInfo)
#else
void CDnStageClearDlg::SetRewardItemInfo()
#endif
{
	CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	if( !pGameTask )
	{
		CDebugSet::ToLogFile( "CDnStageClearDlg::SetClearInfo, pGameTask is NULL!" );
		return;
	}

	CDnDungeonClearImp *pClearImp = (CDnDungeonClearImp *)pGameTask->GetStageClearImp();
	if( !pClearImp ) return;

	std::vector<CDnDungeonClearImp::SDungeonClearInfo> &dungeonClearInfoList = pClearImp->GetDungeonClearInfoList();

	for( int i=0; i<(int)dungeonClearInfoList.size(); i++ )
	{
		if( dungeonClearInfoList[i].sBaseInfo.nSessionID == CDnBridgeTask::GetInstance().GetSessionID() )
		{
			for( int j=0; j<m_nBoxCount; j++ )
			{
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
				if (bBoxInfo)
					m_sBonusBoxButton[j].SetBoxType(dungeonClearInfoList[i].cRewardItemType[j]);
				else
					m_sBonusBoxButton[j].SetBoxItem(dungeonClearInfoList[i].RewardItem[j]);
#else
				m_sBonusBoxButton[j].SetBoxInfo( dungeonClearInfoList[i].cRewardItemType[j], dungeonClearInfoList[i].RewardItem[j] );
#endif
			}
		}
		else
		{
			for( DWORD k=0; k<m_vecStageClearList.size(); k++ ) {
				if( m_vecStageClearList[k].m_nSessionID == dungeonClearInfoList[i].sBaseInfo.nSessionID ) {
					for( int j=0; j<m_vecStageClearList[k].m_nBoxCount; j++ )
					{
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
						if (bBoxInfo)
							m_vecStageClearList[k].m_sBonusBoxButton[j].SetBoxType( dungeonClearInfoList[i].cRewardItemType[j] );
						else
							m_vecStageClearList[k].m_sBonusBoxButton[j].SetBoxItem( dungeonClearInfoList[i].RewardItem[j] );
#else
						m_vecStageClearList[k].m_sBonusBoxButton[j].SetBoxInfo( dungeonClearInfoList[i].cRewardItemType[j], dungeonClearInfoList[i].RewardItem[j] );
#endif
					}
					break;
				}
			}
		}
	}
}

#if defined( PRE_ADD_CP_RANK )
void CDnStageClearDlg::ResetCPRank()
{
	for( int itr = 0; itr < PLAYER_UI_NUMBER; ++itr )
	{
		m_pBestPlayer[itr]->Show( false );
		m_pNewRecord[itr]->Show( false );
	}
}

void CDnStageClearDlg::SetCPRank( std::vector<CDnDungeonClearImp::SDungeonClearInfo> & vecDungeonClearInfo )
{
	UINT nBestCP = 0;

	int nBestIndex = -1, nMyIndex = -1;
	int nCount = (int)vecDungeonClearInfo.size();
	for( int itr = 0; itr < nCount; ++itr )
	{
		TDungeonClearInfo & sInfo = vecDungeonClearInfo[itr].sBaseInfo;
		const UINT nCP = sInfo.nDefaultCP + sInfo.nBonusCP;

		if( nCP > nBestCP || 0 == nBestCP )
		{
			nBestIndex = itr;
			nBestCP = nCP;
		}

		if( CDnBridgeTask::GetInstance().GetSessionID() == sInfo.nSessionID )
			nMyIndex = itr;
	}

	if( -1 == nBestCP || -1 == nMyIndex )
		return;

	vecDungeonClearInfo[nBestIndex].bBestPlayer = true;
	GetNewRecordPlayer( vecDungeonClearInfo[nBestIndex] );

	if( nBestIndex != nMyIndex )
		GetNewRecordPlayer( vecDungeonClearInfo[nMyIndex] );
}

void CDnStageClearDlg::GetNewRecordPlayer( CDnDungeonClearImp::SDungeonClearInfo & sDungeonClearInfo )
{
	CDnGameTask * pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	if( NULL == pGameTask )
		return;

	CDnGameTask::SDungeonClearBestInfo & sDungeonClearBestInfo = pGameTask->GetDungeonClearBestInfo();
	const int nCP = sDungeonClearInfo.sBaseInfo.nDefaultCP + sDungeonClearInfo.sBaseInfo.nBonusCP;
	const char cRank = sDungeonClearInfo.sBaseInfo.cRank;
	const bool bIsAbyssLevel = IsAbyssMinLevel( sDungeonClearInfo.nLevel );

	if( CDnBridgeTask::GetInstance().GetSessionID() == sDungeonClearInfo.sBaseInfo.nSessionID 
		&& sDungeonClearBestInfo.sMyClearBest.nClearPoint < nCP )
	{
		sDungeonClearInfo.bNewRecord = true;

		sDungeonClearBestInfo.sMyClearBest.nClearPoint = nCP;
		sDungeonClearBestInfo.sMyClearBest.cRank = cRank;
		sDungeonClearBestInfo.sMyClearBest.bNew = true;
	}

	if( true == bIsAbyssLevel && sDungeonClearBestInfo.sLegendClearBest.nClearPoint < nCP )
	{
		sDungeonClearInfo.bNewRecord = true;

		sDungeonClearBestInfo.sLegendClearBest.nClearPoint = nCP;
		sDungeonClearBestInfo.sLegendClearBest.cRank = cRank;
		sDungeonClearBestInfo.sLegendClearBest.bNew = true;
		sDungeonClearBestInfo.sLegendClearBest.wszName = sDungeonClearInfo.strName;
	}

	if( true == bIsAbyssLevel && sDungeonClearBestInfo.sMonthlyClearBest.nClearPoint < nCP )
	{
		sDungeonClearInfo.bNewRecord = true;

		sDungeonClearBestInfo.sMonthlyClearBest.nClearPoint = nCP;
		sDungeonClearBestInfo.sMonthlyClearBest.cRank = cRank;
		sDungeonClearBestInfo.sMonthlyClearBest.bNew = true;
		sDungeonClearBestInfo.sMonthlyClearBest.wszName = sDungeonClearInfo.strName;
	}
}

void CDnStageClearDlg::SetCPRankUI( const int nIndex, bool bNewRecord, bool bBestPlayer )
{
	if( true == bNewRecord && false == bBestPlayer )
		m_pNewRecord[nIndex]->SetUICoord( m_pBestPlayer_UICoord[nIndex] );
	else
		m_pNewRecord[nIndex]->SetUICoord( m_pNewRecord_UICoord[nIndex] );

	m_pBestPlayer[nIndex]->Show( bBestPlayer );
	m_pNewRecord[nIndex]->Show( bNewRecord );
}

bool CDnStageClearDlg::IsAbyssMinLevel( const int nLevel )
{
	CDnGameTask * pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	if( NULL == pGameTask )
		return false;

	if( Dungeon::Difficulty::Abyss != pGameTask->GetStageConstructionLevel() )
		return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );

	int nMapIndex = pGameTask->GetGameMapIndex();

	int nDungeonEnterTableID = pSox->GetFieldFromLablePtr(nMapIndex, "_EnterConditionTableID")->GetInteger();
	int nAbyssMinLevel = pDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendNightmareLevel" )->GetInteger();

	if( nAbyssMinLevel < nLevel )
		return false;

	return true;
}
#endif	// #if defined( PRE_ADD_CP_RANK )

void CDnStageClearDlg::SetAllQuestionBox()
{
	for( int i=0; i<m_nBoxCount; i++ )
	{
		m_sBonusBoxButton[i].m_pBonusBoxButton->SetQuestionBox();
		m_sBonusBoxButton[i].m_pBonusBoxButton->SetChecked( false );
		m_sBonusBoxButton[i].m_pBonusBoxButton->SetUseHandle( true );
		m_sBonusBoxButton[i].m_pBonusBoxButton->Show( true );
	}

	for( int i=0; i<(int)m_vecStageClearList.size(); i++ )
	{
		for( int j=0; j<m_vecStageClearList[i].m_nBoxCount; j++ )
		{
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetQuestionBox();
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetChecked( false );
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->Show( true );
		}
	}
	CheckIdentifyItem();
}

void CDnStageClearDlg::SetClosedBonusBox(int idx)
{
	if (idx > m_nBoxCount)
	{
		_ASSERT(0);
		return;
	}

	m_sBonusBoxButton[idx].m_pBonusBoxButton->SetBonusBox( m_sBonusBoxButton[idx].m_nBoxType );
 	m_sBonusBoxButton[idx].m_pBonusBoxButton->SetUseHandle( false );

	// test
	//m_vecStageClearList[2].m_sBonusBoxButton[idx].m_pBonusBoxButton->SetBonusBox( m_sBonusBoxButton[idx].m_nBoxType );
}

void CDnStageClearDlg::SetAllPartyBonusBox()
{
	for( int i=0; i<m_nBoxCount; i++ )
	{
		m_sBonusBoxButton[i].m_pBonusBoxButton->Enable( true );
		m_sBonusBoxButton[i].m_pBonusBoxButton->SetUseHandle( false );
	}

	for( int i=0; i<(int)m_vecStageClearList.size(); i++ )
	{
		for( int j=0; j<m_vecStageClearList[i].m_nBoxCount; j++ )
		{
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->Enable( true );
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetBonusBox( m_vecStageClearList[i].m_sBonusBoxButton[j].m_nBoxType );
		}
	}
}

void CDnStageClearDlg::SetAllBonusBox()
{
	for( int i=0; i<m_nBoxCount; i++ )
	{
		m_sBonusBoxButton[i].m_pBonusBoxButton->SetBonusBox( m_sBonusBoxButton[i].m_nBoxType );
		m_sBonusBoxButton[i].m_pBonusBoxButton->SetUseHandle( false );
		m_sBonusBoxButton[i].m_pBonusBoxButton->Enable( true );
	}

	for( int i=0; i<(int)m_vecStageClearList.size(); i++ )
	{
		for( int j=0; j<m_vecStageClearList[i].m_nBoxCount; j++ )
		{
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->Enable( true );
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetBonusBox( m_vecStageClearList[i].m_sBonusBoxButton[j].m_nBoxType );
		}
	}
}

void CDnStageClearDlg::SetOpenBonusBox(int idx)
{
	if (idx > m_nBoxCount)
	{
		_ASSERT(0);
		return;
	}

	m_sBonusBoxButton[idx].m_pBonusBoxButton->SetOpenBonusBox( m_sBonusBoxButton[idx].m_nBoxType );
	m_sBonusBoxButton[idx].m_pBonusBoxButton->Show( true );
	m_sBonusBoxButton[idx].m_pItemSlotButton->Show( true );

	if( m_sBonusBoxButton[idx].m_pBonusBoxButton->IsChecked() )
	{
		m_sBonusBoxButton[idx].m_pBonusBoxButton->SetItemLight( true );
	}
}

void CDnStageClearDlg::SetAllPartyOpenBonusBox()
{
	for( int i=0; i<(int)m_vecStageClearList.size(); i++ )
	{
		for( int j=0; j<m_vecStageClearList[i].m_nBoxCount; j++ )
		{
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetOpenBonusBox( m_vecStageClearList[i].m_sBonusBoxButton[j].m_nBoxType );
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->Enable( true );
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->Show( true );
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pItemSlotButton->Show( true );

			if( m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->IsChecked() )
			{
				m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetItemLight( true );
			}
		}
	}
}

void CDnStageClearDlg::SetAllOpenBonusBox()
{
	CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundIndexArray[SoundType::BOX_OPEN] );

	for( int i=0; i<m_nBoxCount; i++ )
	{
		m_sBonusBoxButton[i].m_pBonusBoxButton->SetOpenBonusBox( m_sBonusBoxButton[i].m_nBoxType );
		//m_sBonusBoxButton[i].m_pBonusBoxButton->SetBonusBox( m_sBonusBoxButton[i].m_nBoxType );
		m_sBonusBoxButton[i].m_pBonusBoxButton->Show( true );
		m_sBonusBoxButton[i].m_pItemSlotButton->Show( true );

		if( m_sBonusBoxButton[i].m_pBonusBoxButton->IsChecked() )
		{
			m_sBonusBoxButton[i].m_pBonusBoxButton->SetItemLight( true );
		}
	}

	for( int i=0; i<(int)m_vecStageClearList.size(); i++ )
	{
		for( int j=0; j<m_vecStageClearList[i].m_nBoxCount; j++ )
		{
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetOpenBonusBox( m_vecStageClearList[i].m_sBonusBoxButton[j].m_nBoxType );
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->Show( true );
			m_vecStageClearList[i].m_sBonusBoxButton[j].m_pItemSlotButton->Show( true );

			if( m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->IsChecked() )
			{
				m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetItemLight( true );
			}
		}
	}
}

void CDnStageClearDlg::SelectBonusBox( int nSessionID, char cSelectIndex )
{
	if( nSessionID == CDnBridgeTask::GetInstance().GetSessionID() ) {
		for( int j=0; j<4; j++ ) {
			m_sBonusBoxButton[j].m_pBonusBoxButton->SetChecked( ( cSelectIndex & (char)( 0x01 << j ) ) ? true : false );
		}
	}
	for( int i=0; i<(int)m_vecStageClearList.size(); i++ )
	{
		if( m_vecStageClearList[i].m_nSessionID == nSessionID )
		{
			for( int j=0; j<4; j++ ) {
				m_vecStageClearList[i].m_sBonusBoxButton[j].m_pBonusBoxButton->SetChecked( ( cSelectIndex & (char)( 0x01 << j ) ) ? true : false );
			}
		}
	}
}

void CDnStageClearDlg::CheckIdentifyItem()
{
	if( !CDnItemTask::IsActive() ) return;

	int nCount = CDnItemTask::GetInstance().FindItemCountFromItemType( ITEMTYPE_REWARDITEM_IDENTIFY );

	if( !nCount ) {
		m_pIdentifySlot->Show( false );
		m_pIdentifySlotBase->Show( false );
		m_pIdentifyMsg->Show( false );
		m_pIdentifyBack->Show( false );
	}
	else {
		m_pIdentifySlot->Show( true );
		m_pIdentifySlotBase->Show( true );
		m_pIdentifyMsg->Show( true );
		m_pIdentifyBack->Show( true );

		TItemInfo itemInfo;
		if( CDnItem::MakeItemInfo( REWARDIDENTITY_ITEMID, 1, itemInfo ) == false ) return;

		SAFE_DELETE( m_pIdentifyItem );
		m_pIdentifyItem = GetItemTask().CreateItem( itemInfo );
		m_pIdentifySlot->SetItem( m_pIdentifyItem, nCount );
	}
}

void CDnStageClearDlg::OpenBonusBox( char cSlotIndex, char cBoxType )
{
	m_bRequestIdentify = false;
	if( m_pIdentifyItem ) {
		m_pIdentifyItem->PlayInstantUseSound();
	}
	CheckIdentifyItem();
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	m_sBonusBoxButton[cSlotIndex].SetBoxType(cBoxType);
#else
	TItem ItemInfo;
	memset( &ItemInfo, 0, sizeof(TItem) );
	m_sBonusBoxButton[cSlotIndex].SetBoxInfo( cBoxType, ItemInfo );
#endif
	m_sBonusBoxButton[cSlotIndex].m_pBonusBoxButton->SetBonusBox( m_sBonusBoxButton[cSlotIndex].m_nBoxType );
}