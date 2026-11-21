#include "StdAfx.h"
#include "DnDungeonClearDlg.h"
#include "DnDungeonClearMoveDlg.h"
#include "DnInterfaceDlgID.h"
#include "TaskManager.h"
#include "DnPartyTask.h"
#include "DnActor.h"
#include "DnInterface.h"
#include "DnBridgeTask.h"
#include "GameSendPacket.h"
#include "MAPartsBody.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnDungeonClearDlg::CDnDungeonClearDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	//, m_pDungeonClearMoveDlg(NULL)
	, m_pPlayerFace(NULL)
	, m_pButtonBonusBox(NULL)
	, m_pButtonOK(NULL)
	, m_pStaticName(NULL)
	, m_pStaticType(NULL)
	, m_pStaticExp(NULL)
	, m_pStaticCp(NULL)
	, m_pStaticResultA(NULL)
	, m_pStaticResultB(NULL)
	, m_pStaticResultC(NULL)
	, m_pRankImage(NULL)
	, m_pStaticTimer(NULL)
	, m_fElapsedTime(0.0f)
	, m_nCount(0)
	, m_nCurrentTabID(TAB_FINAL)
	, m_pPlayerDungeonClearInfo(NULL)
	, m_pDungeonClearValue(NULL)
	, m_bTimer(false)
	, m_pTabFinal( NULL )
{
}

CDnDungeonClearDlg::~CDnDungeonClearDlg(void)
{
	for( int i=0; i<ITEM_TEXTURE_COUNT; i++ )
	{
		SAFE_RELEASE_SPTR( m_hItemIconList[i] );
	}

	SAFE_RELEASE_SPTR( m_hRankTexture );
	SAFE_RELEASE_SPTR( m_hFaceTexture );

	//SAFE_DELETE( m_pDungeonClearMoveDlg );
}

void CDnDungeonClearDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonClearDlg.ui" ).c_str(), bShow );
}

void CDnDungeonClearDlg::InitialUpdate()
{
	//m_pDungeonClearMoveDlg = new CDnDungeonClearMoveDlg( UI_TYPE_CHILD_MODAL, this, DUNGEON_CLEAR_MOVE_DIALOG );
	//m_pDungeonClearMoveDlg->Initialize( false );

	m_pStaticTimer = GetControl<CEtUIStatic>("ID_STATIC_TIMER");
	m_pTabFinal = GetControl<CEtUIRadioButton>("ID_TAB_FINAL");
	m_pButtonOK = GetControl<CEtUIButton>("ID_BUTTON_OK");
	m_pStaticName = GetControl<CEtUIStatic>("ID_STATIC_NAME");
	m_pStaticType = GetControl<CEtUIStatic>("ID_STATIC_TYPE");
	m_pStaticExp = GetControl<CEtUIStatic>("ID_STATIC_EXP");
	m_pStaticCp = GetControl<CEtUIStatic>("ID_STATIC_CP");
	m_pStaticResultA = GetControl<CEtUIStatic>("ID_LIST_TITLE_04");
	m_pStaticResultB = GetControl<CEtUIStatic>("ID_LIST_TITLE_05");
	m_pStaticResultC = GetControl<CEtUIStatic>("ID_LIST_TITLE_06");
	m_pRankImage = GetControl<CEtUITextureControl>("ID_TEXTUREL_RANK");
	m_pPlayerFace = GetControl<CDnGaugeFace>("ID_PLAYER_FACE");
	m_pPlayerFace->SetFaceType( FT_PLAYER );
	m_pButtonBonusBox = GetControl<CEtUIButton>("ID_BUTTON_BONUSBOX");

	char szControlName[32]={0};
	SPlayerRecordInfo playerRecordInfo;

	for( int i=0; i<PLAYER_RECORD_COUNT; i++ )
	{
		sprintf_s( szControlName, 32, "ID_RECORD_%02d_%02d", i, 0 );
		playerRecordInfo.m_pStaticTitle = GetControl<CEtUIStatic>(szControlName);
		sprintf_s( szControlName, 32, "ID_RECORD_%02d_%02d", i, 1 );
		playerRecordInfo.m_pStatic00 = GetControl<CEtUIStatic>(szControlName);
		sprintf_s( szControlName, 32, "ID_RECORD_%02d_%02d", i, 2 );
		playerRecordInfo.m_pStatic01 = GetControl<CEtUIStatic>(szControlName);
		sprintf_s( szControlName, 32, "ID_RECORD_%02d_%02d", i, 3 );
		playerRecordInfo.m_pStatic02 = GetControl<CEtUIStatic>(szControlName);

		playerRecordInfo.ClearText();
		m_vecPlayerRecordInfo.push_back(playerRecordInfo);
	}

	char szFace[32]={0};
	char szLevel[32]={0};
	char szName[32]={0};
	char szRank[32]={0};
	char szA[32]={0};
	char szB[32]={0};
	char szC[32]={0};
	char szResult[32]={0};
	char szItem[32]={0};
	char szCheck[32]={0};
	SMemberRecordInfo memberRecordInfo;

	for( int i=0, j=0; i<MEMBER_RECORD_COUNT; i++ )
	{
		sprintf_s( szFace, 32, "ID_FACE_%02d", i );
		sprintf_s( szLevel, 32, "ID_LEVEL_%02d", i );
		sprintf_s( szName, 32, "ID_NAME_%02d", i );
		sprintf_s( szRank, 32, "ID_RANK_%02d", i );
		sprintf_s( szA, 32, "ID_LIST_A_%02d", i );
		sprintf_s( szB, 32, "ID_LIST_B_%02d", i );
		sprintf_s( szC, 32, "ID_LIST_C_%02d", i );
		sprintf_s( szResult, 32, "ID_RESULT_%02d", i );
		sprintf_s( szItem, 32, "ID_ITEM_%02d", i );
		sprintf_s( szCheck, 32, "ID_CHECK_%02d", i );

		memberRecordInfo.m_pFaceImage = GetControl<CEtUITextureControl>(szFace);
		memberRecordInfo.m_pStaticLevel = GetControl<CEtUIStatic>(szLevel);
		memberRecordInfo.m_pStaticName = GetControl<CEtUIStatic>(szName);
		memberRecordInfo.m_pStaticRank = GetControl<CEtUIStatic>(szRank);
		memberRecordInfo.m_pStaticA = GetControl<CEtUIStatic>(szA);
		memberRecordInfo.m_pStaticB = GetControl<CEtUIStatic>(szB);
		memberRecordInfo.m_pStaticC = GetControl<CEtUIStatic>(szC);
		memberRecordInfo.m_pStaticResult = GetControl<CEtUIStatic>(szResult);
		memberRecordInfo.m_pItemImage = GetControl<CEtUITextureControl>(szItem);
		memberRecordInfo.m_pStaticCheck = GetControl<CEtUIStatic>(szCheck);

		memberRecordInfo.ClearAll();
		m_vecMemberRecordInfo.push_back(memberRecordInfo);
	}

	m_hRankTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( "DungeonRankIcon.dds" ).c_str(), RT_TEXTURE );
	m_hFaceTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( "GaugeFace.dds" ).c_str(), RT_TEXTURE );

	m_hItemIconList[0] = LoadResource( CEtResourceMng::GetInstance().GetFullName( "WeaponIcon01.dds" ).c_str(), RT_TEXTURE );
	m_hItemIconList[1] = LoadResource( CEtResourceMng::GetInstance().GetFullName( "ArmorIcon01.dds" ).c_str(), RT_TEXTURE );
	m_hItemIconList[2] = LoadResource( CEtResourceMng::GetInstance().GetFullName( "AccessoryIcon01.dds" ).c_str(), RT_TEXTURE );
	m_hItemIconList[3] = LoadResource( CEtResourceMng::GetInstance().GetFullName( "EtcIcon01.dds" ).c_str(), RT_TEXTURE );
}

void CDnDungeonClearDlg::Process( float fElapsedTime )
{
	if( m_bTimer )
	{
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
//			SendOKDungeonClear( CDnBridgeTask::GetInstance().GetSessionID() );

			m_pButtonOK->Enable(false);
			m_bTimer = false;
		}
	}

	CEtUIDialog::Process( fElapsedTime );	
}

void CDnDungeonClearDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_OK" ) )
		{
//			SendOKDungeonClear( CDnBridgeTask::GetInstance().GetSessionID() );

			m_pButtonOK->Enable(false);
			m_bTimer = false;
			return;
		}

		if( IsCmdControl("ID_BUTTON_BONUSBOX") )
		{
			// Note : Opened the bonus box.
			//		How...?
			//
			return;
		}
	}
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		int nTabID = static_cast<CEtUIRadioButton*>(pControl)->GetTabID();
		ChangedTabButton( nTabID );
		return;
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDungeonClearDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		SetPlayerInfo();
		SetMemberBaseRecordInfo();

		m_pTabFinal->SetChecked( true );
		ChangedTabButton( TAB_FINAL );

		m_bTimer = true;
		m_nCount = 60;

		m_pButtonOK->Enable(true);

		if( GetPartyTask().GetPartyRole() == CDnPartyTask::LEADER )
		{
			if( GetPartyTask().GetPartyCount() > 1 )
			{
				m_pButtonOK->Enable(false);
			}
		}
	}
	else
	{
		m_pPlayerDungeonClearInfo = NULL;
		m_pDungeonClearValue = NULL;
		m_bTimer = false;
	}

	CEtUIDialog::Show( bShow );
}

void CDnDungeonClearDlg::ChangedTabButton( int nTabID )
{
	m_nCurrentTabID = nTabID;
	SetTitle( nTabID );
	SetPlayerRecord( nTabID );
	SetMemberRecordInfo( nTabID );
}

void CDnDungeonClearDlg::SetPlayerInfo()
{
	//if( !CDnActor::s_hLocalActor ) return;
	//DnActorHandle hActor = CDnActor::s_hLocalActor;

	//CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	//if( !pGameTask )
	//{
	//	CDebugSet::ToLogFile( "CDnDungeonClearDlg::SetPlayerInfo, pPartyTask is NULL!" );
	//	return;
	//}

	//CDnPartyTask *pPartyTask = (CDnPartyTask*)CTaskManager::GetInstance().GetTask( "PartyTask" );
	//if( !pPartyTask )
	//{
	//	CDebugSet::ToLogFile( "CDnDungeonClearDlg::SetPlayerInfo, pPartyTask is NULL!" );
	//	return;
	//}

	//int nSessionID = CDnBridgeTask::GetInstance().GetSessionID();

	//m_pPlayerDungeonClearInfo = pGameTask->GetDungeonClearInfo( nSessionID );
	//if( !m_pPlayerDungeonClearInfo )
	//{
	//	CDebugSet::ToLogFile( "CDnDungeonClearDlg::SetPlayerInfo, m_pPlayerDungeonClearInfo is NULL!" );
	//	return;
	//}

	//m_pDungeonClearValue = &(pGameTask->GetDungeonClearValue());
	//if( !m_pDungeonClearValue )
	//{
	//	CDebugSet::ToLogFile( "CDnDungeonClearDlg::SetPlayerInfo, m_pDungeonClearValue is NULL!" );
	//	return;
	//}

	//m_pPlayerFace->SetFaceID( m_pPlayerDungeonClearInfo->nClassID-1 );
	//std::vector< EtObjectHandle > vecObjects;
	//MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(hActor.GetPointer());
	//vecObjects.push_back( pPartsBody->GetCombineObject() );	
	//vecObjects.push_back( pPartsBody->GetParts( CDnParts::Face )->GetObjectHandle() );
	//vecObjects.push_back( pPartsBody->GetParts( CDnParts::Hair )->GetObjectHandle() );
	//m_pPlayerFace->UpdatePortrait( vecObjects );
	//m_pRankImage->SetTexture( m_hRankTexture, m_pPlayerDungeonClearInfo->nRank*65, 0, 65, 86 );	

	//wchar_t szTemp[64]={0};
	//m_pStaticType->SetText(L"Attacker");
	//swprintf_s( szTemp, 64, L"%d", m_pPlayerDungeonClearInfo->nClearPoint );
	//m_pStaticCp->SetText( szTemp );
	////swprintf_s( szTemp, 64, L"Lv.%d %s", m_pPlayerDungeonClearInfo->nLevel, m_pPlayerDungeonClearInfo->strName.c_str() );
	//swprintf_s( szTemp, 64, L"Lv.%d %s", hActor->GetLevel(), m_pPlayerDungeonClearInfo->strName.c_str() );
	//m_pStaticName->SetText( szTemp );
}

void CDnDungeonClearDlg::SetTitle( int nTabID )
{
	for( int i=0; i<PLAYER_RECORD_COUNT; i++ )
		m_vecPlayerRecordInfo[i].ClearText();

	wchar_t szTemp1[64]={0};
	wchar_t szTemp2[64]={0};
	wchar_t szTemp3[64]={0};
	int nIndex(0);

	switch( nTabID )
	{
	case TAB_FINAL:		nIndex = 2319;	break;
	case TAB_PLAYER:	nIndex = 2335;	break;
	case TAB_PARTY:		nIndex = 2338;	break;
	case TAB_DEMERIT:	nIndex = 2341;	break;
	};

	swprintf_s( szTemp1, 64, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nIndex ) );
	swprintf_s( szTemp2, 64, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nIndex+1 ) );
	swprintf_s( szTemp3, 64, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nIndex+2 ) );

	m_vecPlayerRecordInfo[0].m_pStaticTitle->SetText(szTemp1);
	m_vecPlayerRecordInfo[1].m_pStaticTitle->SetText(szTemp2);
	m_vecPlayerRecordInfo[2].m_pStaticTitle->SetText(szTemp3);

	m_pStaticResultA->SetText( szTemp1 );
	m_pStaticResultB->SetText( szTemp2 );
	m_pStaticResultC->SetText( szTemp3 );
}

void CDnDungeonClearDlg::SetPlayerRecord( int nTabID )
{
	if( !m_pPlayerDungeonClearInfo )
	{
		CDebugSet::ToLogFile( "CDnDungeonClearDlg::SetPlayerRecord, m_pPlayerDungeonClearInfo is NULL!" );
		return;
	}

	// Note: Set own result matching the tab
	//
	CDnPartyTask *pPartyTask = (CDnPartyTask*)CTaskManager::GetInstance().GetTask( "PartyTask" );
	if( !pPartyTask )
	{
		CDebugSet::ToLogFile( "CDnDungeonClearDlg::SetPlayerRecord, pPartyTask is NULL!" );
		return;
	}

	wchar_t szTemp[64]={0};

	/*
	switch( nTabID )
	{
	case TAB_FINAL:
		{
			swprintf_s( szTemp, 64, L"+%dExp", m_pPlayerDungeonClearInfo->nRankExp );
			m_vecPlayerRecordInfo[0].m_pStatic00->SetText( szTemp );

			swprintf_s( szTemp, 64, L"+%dExp", m_pPlayerDungeonClearInfo->nRankExp );
			m_pStaticExp->SetText( szTemp );
		}
		break;
	case TAB_PLAYER:
		{
			m_vecPlayerRecordInfo[0].m_pStatic00->SetIntToText( m_pPlayerDungeonClearInfo->sBaseInfo.nMaxComboCount );
			swprintf_s( szTemp, 64, L"x %d", m_pDungeonClearValue->nMaxCombo );
			m_vecPlayerRecordInfo[0].m_pStatic01->SetText( szTemp );

			m_vecPlayerRecordInfo[1].m_pStatic00->SetIntToText( m_pPlayerDungeonClearInfo->sBaseInfo.nLastKillMonsterCount );
			swprintf_s( szTemp, 64, L"x %d", m_pDungeonClearValue->nKillMonster );
			m_vecPlayerRecordInfo[1].m_pStatic01->SetText( szTemp );

			m_vecPlayerRecordInfo[2].m_pStatic00->SetIntToText( m_pPlayerDungeonClearInfo->sBaseInfo.nLastKillBossCount );
			swprintf_s( szTemp, 64, L"x %d", m_pDungeonClearValue->nKillBoss );
			m_vecPlayerRecordInfo[2].m_pStatic01->SetText( szTemp );

			swprintf_s( szTemp, 64, L"+%dCp", m_pPlayerDungeonClearInfo->nPlayerResult );
			m_pStaticExp->SetText( szTemp );
		}
		break;
	case TAB_PARTY:
		{
			m_vecPlayerRecordInfo[1].m_pStatic00->SetIntToText( pPartyTask->GetPartyCount() );
			swprintf_s( szTemp, 64, L"x %d", m_pDungeonClearValue->nPartySurvivalCount );
			m_vecPlayerRecordInfo[1].m_pStatic01->SetText( szTemp );

			m_vecPlayerRecordInfo[2].m_pStatic00->SetIntToText( m_pPlayerDungeonClearInfo->sBaseInfo.nEventCount );
			swprintf_s( szTemp, 64, L"x %d", m_pDungeonClearValue->nEventCount );
			m_vecPlayerRecordInfo[2].m_pStatic01->SetText( szTemp );

			swprintf_s( szTemp, 64, L"+%dCp", m_pPlayerDungeonClearInfo->nPartyResult );
			m_pStaticExp->SetText( szTemp );
		}
		break;
	case TAB_DEMERIT:
		{
			m_vecPlayerRecordInfo[0].m_pStatic00->SetIntToText( m_pPlayerDungeonClearInfo->sBaseInfo.nHitCount );
			swprintf_s( szTemp, 64, L"x -%d", m_pDungeonClearValue->nHitCount );
			m_vecPlayerRecordInfo[0].m_pStatic01->SetText( szTemp );

			m_vecPlayerRecordInfo[1].m_pStatic00->SetIntToText( m_pPlayerDungeonClearInfo->sBaseInfo.nDeadCount );
			swprintf_s( szTemp, 64, L"x -%d", m_pDungeonClearValue->nDeadCount );
			m_vecPlayerRecordInfo[1].m_pStatic01->SetText( szTemp );

			m_vecPlayerRecordInfo[2].m_pStatic00->SetIntToText( m_pDungeonClearValue->nAliveMonsterCount );
			swprintf_s( szTemp, 64, L"x -%d", m_pDungeonClearValue->nSurvivalCount );
			m_vecPlayerRecordInfo[2].m_pStatic01->SetText( szTemp );

			swprintf_s( szTemp, 64, L"-%dCp", m_pPlayerDungeonClearInfo->nDeductionResult );
			m_pStaticExp->SetText( szTemp );
		}
		break;
	};
	*/
}

void CDnDungeonClearDlg::SetMemberBaseRecordInfo()
{
	//for( int i=0; i<MEMBER_RECORD_COUNT; i++ )
	//{
	//	m_vecMemberRecordInfo[i].ClearAll();
	//}

	//// Note: Set basic info of party member
	////
	//CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	//if( !pGameTask )
	//{
	//	CDebugSet::ToLogFile( "CDnDungeonClearDlg::SetMemberBaseRecordInfo, pGameTask is NULL!" );
	//	return;
	//}

	//std::vector<CDnGameTask::SDungeonClearInfo> &dungeonClearInfoList = pGameTask->GetDungeonClearInfoList();

	//static wchar_t *wszRank[RANL_MAX] = { L"SSS", L"SS", L"S", L"A", L"B", L"C", L"D" };
	//wchar_t szTemp[64]={0};
	//DnActorHandle hActor;

	//for( int i=0; i<(int)m_vecMemberRecordInfo.size(); i++ )
	//{
	//	if( i >= (int)dungeonClearInfoList.size() )
	//		break;

	//	hActor = CDnActor::FindActorFromUniqueID( dungeonClearInfoList[i].sBaseInfo.nSessionID );
	//	// Note : Not sure how to handle this part. Handling when a party member leaves or dies
	//	//
	//	if( !hActor || hActor->IsDie() )
	//		continue;

	//	m_vecMemberRecordInfo[i].m_pFaceImage->SetTexture( m_hFaceTexture, (dungeonClearInfoList[i].nClassID-1)*FACE_ICON_XSIZE, 0, FACE_ICON_XSIZE, FACE_ICON_YSIZE );
	//	m_vecMemberRecordInfo[i].m_pFaceImage->Show(true);
	//	m_vecMemberRecordInfo[i].m_pStaticLevel->SetIntToText( hActor->GetLevel() );
	//	m_vecMemberRecordInfo[i].m_pStaticName->SetText( dungeonClearInfoList[i].strName );

	//	swprintf_s( szTemp, 64, L"%s", wszRank[dungeonClearInfoList[i].nRank] );
	//	m_vecMemberRecordInfo[i].m_pStaticRank->SetText( szTemp );

	//	// Note : Set treasure chest image
	//	// 130th icon in ItemIcon2.dds
	//	//
	//	m_vecMemberRecordInfo[i].m_pItemImage->SetTexture( m_hItemIconList[3], 9*ITEMSLOT_ICON_XSIZE, 0, ITEMSLOT_ICON_XSIZE, ITEMSLOT_ICON_YSIZE );
	//	m_vecMemberRecordInfo[i].m_pItemImage->Show(true);
	//	m_vecMemberRecordInfo[i].m_pStaticCheck->Show( dungeonClearInfoList[i].bCheck );
	//}
}

void CDnDungeonClearDlg::SetMemberRecordInfo( int nTabID )
{
	for( int i=0; i<MEMBER_RECORD_COUNT; i++ )
	{
		m_vecMemberRecordInfo[i].ClearChangePart();
	}

	// Note: Set party member info matching each tab
	//
	CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	if( !pGameTask )
	{
		CDebugSet::ToLogFile( "CDnDungeonClearDlg::SetMemberRecordInfo, pGameTask is NULL!" );
		return;
	}

	CDnDungeonClearImp *pClearImp = (CDnDungeonClearImp *)pGameTask->GetStageClearImp();
	if( !pClearImp ) return;
	std::vector<CDnDungeonClearImp::SDungeonClearInfo> &dungeonClearInfoList = pClearImp->GetDungeonClearInfoList();

	/*
	wchar_t szTemp[64]={0};

	switch( nTabID )
	{
	case TAB_FINAL:
		{
			for( int i=0; i<(int)m_vecMemberRecordInfo.size(); i++ )
			{
				if( i >= (int)dungeonClearInfoList.size() )
					break;

				swprintf_s( szTemp, 64, L"%dExp", dungeonClearInfoList[i].nRankExp );
				m_vecMemberRecordInfo[i].m_pStaticA->SetText( szTemp );

				swprintf_s( szTemp, 64, L"%dExp", dungeonClearInfoList[i].nResultExp );
				m_vecMemberRecordInfo[i].m_pStaticResult->SetText( szTemp );
			}
		}
		break;
	case TAB_PLAYER:
		{
			for( int i=0; i<(int)m_vecMemberRecordInfo.size(); i++ )
			{
				if( i >= (int)dungeonClearInfoList.size() )
					break;

				m_vecMemberRecordInfo[i].m_pStaticA->SetIntToText( dungeonClearInfoList[i].sBaseInfo.nMaxComboCount );
				m_vecMemberRecordInfo[i].m_pStaticB->SetIntToText( dungeonClearInfoList[i].sBaseInfo.nLastKillMonsterCount );
				m_vecMemberRecordInfo[i].m_pStaticC->SetIntToText( dungeonClearInfoList[i].sBaseInfo.nLastKillBossCount );
				swprintf_s( szTemp, 64, L"%dCp", dungeonClearInfoList[i].nPlayerResult );
				m_vecMemberRecordInfo[i].m_pStaticResult->SetText( szTemp );
			}
		}
		break;
	case TAB_PARTY:
		{
			for( int i=0; i<(int)m_vecMemberRecordInfo.size(); i++ )
			{
				if( i >= (int)dungeonClearInfoList.size() )
					break;

				m_vecMemberRecordInfo[i].m_pStaticB->SetIntToText( m_pDungeonClearValue->nPartySurvivalCount );
				m_vecMemberRecordInfo[i].m_pStaticC->SetIntToText( dungeonClearInfoList[i].sBaseInfo.nEventCount );
				swprintf_s( szTemp, 64, L"%dCp", dungeonClearInfoList[i].nPartyResult );
				m_vecMemberRecordInfo[i].m_pStaticResult->SetText( szTemp );
			}
		}
		break;
	case TAB_DEMERIT:
		{
			for( int i=0; i<(int)m_vecMemberRecordInfo.size(); i++ )
			{
				if( i >= (int)dungeonClearInfoList.size() )
					break;

				m_vecMemberRecordInfo[i].m_pStaticA->SetIntToText( dungeonClearInfoList[i].sBaseInfo.nHitCount );
				m_vecMemberRecordInfo[i].m_pStaticB->SetIntToText( dungeonClearInfoList[i].sBaseInfo.nDeadCount );
				m_vecMemberRecordInfo[i].m_pStaticC->SetIntToText( m_pDungeonClearValue->nAliveMonsterCount );
				m_vecMemberRecordInfo[i].m_pStaticResult->SetIntToText( dungeonClearInfoList[i].nDeductionResult );
			}
		}
		break;
	};
	*/
}

void CDnDungeonClearDlg::SetCallbackEx( CEtUICallback *pCallBack )
{
	//m_pDungeonClearMoveDlg->SetCallback( pCallBack );
	GetInterface().SetProgressMBoxCallback(pCallBack);
}

void CDnDungeonClearDlg::OpenDungeonMoveDialog()
{
	CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	if( !pGameTask )
	{
		CDebugSet::ToLogFile( "CDnDungeonClearDlg::SetMemberRecordInfo, pGameTask is NULL!" );
		return;
	}

	//ShowChildDialog( m_pDungeonClearMoveDlg, true );
	//GetInterface().ProgressMBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 803 ), 10, 0, false, DUNGEON_CLEAR_MOVE_DIALOG, pGameTask);
	m_bTimer = false;
}

void CDnDungeonClearDlg::SetTime( int nTime )
{
	swprintf_s( m_wszTimer, 80, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2312 ), nTime );
	m_pStaticTimer->SetText( m_wszTimer );
}

void CDnDungeonClearDlg::SetCheckRecordInfo( int nSessionID )
{
	//CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	//if( !pGameTask ) return;

	//CDnGameTask::SDungeonClearInfo *pDungeonClearInfo = pGameTask->GetDungeonClearInfo( nSessionID );
	//if( !pDungeonClearInfo ) return;

	//EtTextureHandle hItemIcon;
	//int nItemIconIndex = pDungeonClearInfo->nItemIconIndex;

	//if( nItemIconIndex != -1 )
	//{
	//	//if( nItemIconIndex >= ITEM_TEXTURE_ICON_COUNT )
	//	//{
	//	//	hItemIcon = m_hItemTexture2;
	//	//	nItemIconIndex -= ITEM_TEXTURE_ICON_COUNT;
	//	//}
	//	//else
	//	//{
	//	//	hItemIcon = m_hItemTexture1;
	//	//}
	//}

	//for( int i=0; i<(int)m_vecMemberRecordInfo.size(); i++ )
	//{
	//	if( pDungeonClearInfo->strName == m_vecMemberRecordInfo[i].m_pStaticName->GetText() )
	//	{
	//		if( nItemIconIndex != -1 )
	//		{
	//			//m_vecMemberRecordInfo[i].m_pItemImage->SetTexture( hItemIcon, (nItemIconIndex%ITEM_TEXTURE_X_COUNT)*ITEMSLOT_ICON_XSIZE, 
	//			//																(nItemIconIndex/ITEM_TEXTURE_X_COUNT)*ITEMSLOT_ICON_YSIZE,
	//			//																ITEMSLOT_ICON_XSIZE, ITEMSLOT_ICON_YSIZE );
	//			m_vecMemberRecordInfo[i].m_pItemImage->Show(true);
	//		}
	//		else
	//		{
	//			m_vecMemberRecordInfo[i].m_pItemImage->Show(false);
	//		}

	//		m_vecMemberRecordInfo[i].m_pStaticCheck->Show( true );
	//		break;
	//	}
	//}

	//if( GetPartyTask().GetPartyType() == CDnPartyTask::LEADER )
	//{
	//	std::vector<CDnGameTask::SDungeonClearInfo> &dungeonClearInfoList = pGameTask->GetDungeonClearInfoList();

	//	int nCheckCount(0);

	//	for( int i=0; i<(int)dungeonClearInfoList.size(); i++ )
	//	{
	//		if( dungeonClearInfoList[i].bCheck )
	//		{
	//			nCheckCount++;
	//		}
	//	}

	//	if( nCheckCount == ((int)dungeonClearInfoList.size()-1) )
	//	{
	//		m_pButtonOK->Enable(true);
	//	}
	//}
}

void CDnDungeonClearDlg::OnBlindOpen() 
{
	CDnLocalPlayerActor::LockInput(true);
//	CDnMouseCursor::GetInstance().ShowCursor( true );
}

void CDnDungeonClearDlg::OnBlindOpened()
{
	Show(true);
	GetInterface().ShowChatDialog();
}

void CDnDungeonClearDlg::OnBlindClose()
{
	SetCallbackEx( NULL );
	Show( false );
}

void CDnDungeonClearDlg::OnBlindClosed()
{
	GetInterface().OpenBaseDialog();

	CDnLocalPlayerActor::LockInput(false);
//	CDnMouseCursor::GetInstance().ShowCursor( false );
}