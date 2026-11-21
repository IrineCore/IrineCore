#include "StdAfx.h"
#include "DnDungeonEnterDlg.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnInterface.h"
#include "DnLocalPlayerActor.h"
#include "DnDungeonEnterLevelDlg.h"
#include "DnPartyTask.h"
#include "DnDungeonExpectDlg.h"
#include "DnDungeonNestEnterLevelDlg.h"

#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
#include "PartySendPacket.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnDungeonEnterDlg::CDnDungeonEnterDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_nGateIndex( 0 )
	, m_pStaticDungeonName( NULL )
	, m_pTextureDungeonImage( NULL )
	, m_pTextureDungeonMap( NULL )
	, m_pDungeonEnterLevelDlg( NULL )
	, m_pStaticBottomBack( NULL )
	, m_pDungeonExpectDlg( NULL )
	, m_eDungeonEnterType( eDungeonEnterType::DUNGEONENTER_TYPE_NORMAL )
	, m_pDungeonNestEnterLevelDlg( NULL )
{
}

CDnDungeonEnterDlg::~CDnDungeonEnterDlg(void)
{
	SAFE_RELEASE_SPTR( m_hIntroImage );
	SAFE_RELEASE_SPTR( m_hIntroMap );

	SAFE_DELETE( m_pDungeonEnterLevelDlg );
	SAFE_DELETE( m_pDungeonExpectDlg );
	SAFE_DELETE( m_pDungeonNestEnterLevelDlg );
}

void CDnDungeonEnterDlg::Initialize( bool bShow )
{
	if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NORMAL )
		CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterDlg.ui" ).c_str(), bShow );
	else if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NEST )
		CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "HardMoveEnterDlg.ui" ).c_str(), bShow );
}

void CDnDungeonEnterDlg::InitialUpdate()
{
	GetControl<CEtUIStatic>("ID_STATIC_NOTICE0")->Show(false);
	GetControl<CEtUIStatic>("ID_STATIC_NOTICE1")->Show(false);
	
		
	m_pStaticDungeonName = GetControl<CEtUIStatic>("ID_STATIC_DUNGEONNAME");
	m_pTextureDungeonImage = GetControl<CEtUITextureControl>("ID_TEXTURE_DUNGEONIMAGE");
	m_pTextureDungeonImage->GetUICoord( m_uiTextureImageBack );
	m_pTextureDungeonMap = GetControl<CEtUITextureControl>("ID_TEXTUREL_MAP");

	m_pStaticBottomBack = GetControl<CEtUIStatic>("ID_BOTTOM_BLACK");
	m_pStaticBottomBack->GetUICoord( m_uiBottomBlackBack );

	char szButtonGate[32]={0};
	SDungeonGateInfo dungeonGateInfo;

	for( int i=0; i<DUNGEON_GATE_COUNT; i++ )
	{
		sprintf_s( szButtonGate, 32, "ID_BUTTON_DUN_%02d", i+1 );
		dungeonGateInfo.m_pButtonGate = GetControl<CDnStageEnterButton>(szButtonGate);
		
		dungeonGateInfo.Show( false );
		dungeonGateInfo.Clear();
		m_vecDungeonGateInfo.push_back(dungeonGateInfo);
	}

	InitializeEnterLevelDlg();
}

void CDnDungeonEnterDlg::InitializeEnterLevelDlg()
{
	m_pDungeonEnterLevelDlg = new CDnDungeonEnterLevelDlg( UI_TYPE_CHILD, this );
	m_pDungeonEnterLevelDlg->Initialize( false );
	m_pDungeonEnterLevelDlg->SetDialogID( GetDialogID() );

	m_pDungeonExpectDlg = new CDnDungeonExpectDlg( UI_TYPE_CHILD, this );
	m_pDungeonExpectDlg->Initialize( false );

	m_pDungeonNestEnterLevelDlg = new CDnDungeonNestEnterLevelDlg( UI_TYPE_CHILD, this );
	m_pDungeonNestEnterLevelDlg->Initialize( false );
	m_pDungeonNestEnterLevelDlg->SetDialogID( GetDialogID() );
}

void CDnDungeonEnterDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		CEtUIRadioButton *pRadioButton = (CEtUIRadioButton*)pControl;
		if( !pRadioButton ) return;

		if( strstr( pControl->GetControlName(), "ID_BUTTON_DUN" ) )
		{
			m_nGateIndex = pRadioButton->GetTabID();
			if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NORMAL )
			{
				if( m_nGateIndex >= 0 )
				{
					m_pDungeonEnterLevelDlg->UpdateDungeonInfo( &m_vecDungeonGateInfo[m_nGateIndex] );
					CDnDungeonEnterDlg::SDungeonGateInfo *pInfo = &m_vecDungeonGateInfo[ m_nGateIndex ];
					if( pInfo )
					{
						if( m_pDungeonExpectDlg )
							m_pDungeonExpectDlg->UpdateDungeonExpectReward( pInfo->m_nMapIndex, m_pDungeonEnterLevelDlg->GetDungeonLevel() );
					}
				}
			}
			else if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NEST )
			{
				if( m_nGateIndex >= 0 )
				{
					m_pDungeonNestEnterLevelDlg->UpdateDungeonInfo( &m_vecDungeonGateInfo[m_nGateIndex] );
					CDnDungeonEnterDlg::SDungeonGateInfo *pInfo = &m_vecDungeonGateInfo[ m_nGateIndex ];
					if( pInfo )
					{
						if( m_pDungeonExpectDlg )
							m_pDungeonExpectDlg->UpdateDungeonExpectReward( pInfo->m_nMapIndex, m_pDungeonNestEnterLevelDlg->GetDungeonLevel() );
					}
				}
			}
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDungeonEnterDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		UpdateControlPos();
		UpdateButtonState();
	}
	else
	{
		m_vecEnableGateIndex.clear();

		for( int i=0; i<(int)m_vecDungeonGateInfo.size(); i++ )
		{
			m_vecDungeonGateInfo[i].Enable( false );
		}
	}

	CEtUIDialog::Show( bShow );
}

void CDnDungeonEnterDlg::SetDungeonImage( const std::string &DungeonImageName )
{
	SAFE_RELEASE_SPTR( m_hIntroImage );
	SAFE_RELEASE_SPTR( m_hIntroMap );

	if( DungeonImageName.empty() )
	{
		m_hIntroImage = LoadResource( CEtResourceMng::GetInstance().GetFullName( "DungeonImage01.dds" ).c_str(), RT_TEXTURE );
	}
	else
	{
		m_hIntroImage = LoadResource( CEtResourceMng::GetInstance().GetFullName( DungeonImageName.c_str() ).c_str(), RT_TEXTURE );
	}

	if( m_hIntroImage && m_pTextureDungeonImage )
		m_pTextureDungeonImage->SetTexture( m_hIntroImage, 0, 0, m_hIntroImage->Width(), m_hIntroImage->Height() );

	if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NORMAL )
		m_hIntroMap = LoadResource( CEtResourceMng::GetInstance().GetFullName( "DungeonRoadmap01.dds" ).c_str(), RT_TEXTURE );
	else if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NEST )
		m_hIntroMap = LoadResource( CEtResourceMng::GetInstance().GetFullName( "DungeonRoadmap02.dds" ).c_str(), RT_TEXTURE );

	if( m_hIntroMap && m_pTextureDungeonMap )
		m_pTextureDungeonMap->SetTexture( m_hIntroMap, 0, 0, m_hIntroMap->Width(), m_hIntroMap->Height() );
}

void CDnDungeonEnterDlg::SetGateName( std::wstring &strGateName )
{
	m_pStaticDungeonName->ClearText();
	m_pStaticDungeonName->SetText(strGateName);
}

void CDnDungeonEnterDlg::SetGateInfo( CDnWorld::DungeonGateStruct *pGateInfo )
{
	ASSERT(pGateInfo&&"CDnDungeonEnterDlg::SetGateInfo, pGateInfo is NULL!");

	for( int i=0; i<(int)m_vecDungeonGateInfo.size(); i++ )
	{
		m_vecDungeonGateInfo[i].Show( false );
		m_vecDungeonGateInfo[i].Clear();
	}

	SetDungeonImage( pGateInfo->szGateEnterImage );

	CDnWorld::DungeonGateStruct *pGateStruct(NULL);
	wchar_t wszTemp[128]={0};

	for( int i=0; i<(int)pGateInfo->pVecMapList.size(); i++ )
	{
		pGateStruct = (CDnWorld::DungeonGateStruct*)pGateInfo->pVecMapList[i];
		if( !pGateStruct ) continue;

		m_vecDungeonGateInfo[i].SetInfo( pGateStruct->szMapName.c_str(), pGateStruct->szDesc.c_str(), pGateStruct->nMinPartyCount, pGateStruct->nMaxPartyCount, pGateStruct->nMapIndex, pGateStruct->Environment, pGateStruct->nMinLevel, pGateStruct->nMaxLevel, pGateStruct->PermitFlag );
		m_vecDungeonGateInfo[i].SetUserLevel( pGateStruct->nMinLevel, pGateStruct->nMaxLevel );
		for( DWORD j=0; j<pGateStruct->cVecNotEnoughItemActorIndex.size(); j++ ) {
			CDnPartyTask::PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pGateStruct->cVecNotEnoughItemActorIndex[j] );
			if( !pPartyStruct ) continue;
			if( !pPartyStruct->hActor ) continue;
			m_vecDungeonGateInfo[i].m_hVecNeedItemActorList.push_back( pPartyStruct->hActor );
		}
		
		for( int j=0; j<5; j++ )
		{
			m_vecDungeonGateInfo[i].m_cCanDifficult[j] = pGateStruct->cCanDifficult[j];
			m_vecDungeonGateInfo[i].m_vecRecommandLevel.push_back( pGateStruct->nRecommendLevel[j] );
			m_vecDungeonGateInfo[i].m_vecRecommandPartyCount.push_back( pGateStruct->nRecommendPartyCount[j] );
		}
		m_vecDungeonGateInfo[i].m_nMaxUsableCoinCount = pGateStruct->nMaxUsableCoin;
		m_vecDungeonGateInfo[i].m_nNeedItemID = pGateStruct->nNeedItemID;
		m_vecDungeonGateInfo[i].m_nNeedItemCount = pGateStruct->nNeedItemCount;
		m_vecDungeonGateInfo[i].m_cDungeonEnterPermit = pGateStruct->PermitFlag;
		m_vecDungeonGateInfo[i].m_nAbyssMinLevel = pGateStruct->nAbyssMinLevel;
		m_vecDungeonGateInfo[i].m_nAbyssMaxLevel = pGateStruct->nAbyssMaxLevel;
		m_vecDungeonGateInfo[i].m_nAbyssNeedQuestID = pGateStruct->nAbyssNeedQuestID;
		m_vecDungeonGateInfo[i].m_nMinPartyCount = pGateStruct->nMinPartyCount;
		m_vecDungeonGateInfo[i].m_nMaxPartyCount = pGateStruct->nMaxPartyCount;
		m_vecDungeonGateInfo[i].Show( true );
		
	}

	CDnGameTask *pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstance().GetTask( "GameTask" ));
	if( !pGameTask ) return;

	pGameTask->GetEnableGateIndex( pGateInfo, m_vecEnableGateIndex );

}

void CDnDungeonEnterDlg::UpdateButtonState()
{
	CDnPartyTask::PartyRoleEnum partyState = GetPartyTask().GetPartyRole();
	if( partyState == CDnPartyTask::MEMBER )
	{
		return;
	}

	for( int i=0; i<(int)m_vecEnableGateIndex.size(); i++ )
	{
		int nIndex = m_vecEnableGateIndex[i];
		m_vecDungeonGateInfo[nIndex].Enable( true );
	}

	int nPartyTargetMapIndex = 0;
	if( GetPartyTask().GetPartyRole() == CDnPartyTask::LEADER )
		nPartyTargetMapIndex = GetPartyTask().GetPartyTargetStageIdx();

	int nMinLevel(-1);
	int nMaxIndex(0);

	for( int i=0; i<(int)m_vecDungeonGateInfo.size(); i++ )
	{
		if( !m_vecDungeonGateInfo[i].IsShow() )
			continue;

		if( !m_vecDungeonGateInfo[i].IsEnable() )
			continue;

		// If the map index matches the target, select it immediately.
		if( m_vecDungeonGateInfo[i].m_nMapIndex == nPartyTargetMapIndex )
		{
			nMaxIndex = i;
			break;
		}

		if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NORMAL )
		{
			if( m_vecDungeonGateInfo[i].m_nMinLevel > nMinLevel )
			{
				nMinLevel = m_vecDungeonGateInfo[i].m_nMinLevel;
				nMaxIndex = i;
			}
		}
		else if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NEST )
		{
			if( nPartyTargetMapIndex == 0 && m_vecDungeonGateInfo[i].m_cCanDifficult[4] > 0 )	// Make Nest Normal the default
			{
				nMaxIndex = i;
				break;
			}
		}
	}

	m_vecDungeonGateInfo[nMaxIndex].m_pButtonGate->SetChecked( true );
}

void CDnDungeonEnterDlg::UpdateControlPos()
{
	SUICoord textureCoord, bottomCoord;

	textureCoord.fWidth = m_uiTextureImageBack.fWidth * GetScreenWidthRatio();
	textureCoord.fHeight = m_uiTextureImageBack.fHeight * GetScreenWidthRatio();

	textureCoord.fX = m_uiTextureImageBack.fX - ((textureCoord.fWidth - m_uiTextureImageBack.fWidth)/2.0f);
	textureCoord.fY = m_uiTextureImageBack.fY - ((textureCoord.fHeight - m_uiTextureImageBack.fHeight)/2.0f);

	m_pTextureDungeonImage->SetUICoord( textureCoord );

	bottomCoord.fWidth = m_uiBottomBlackBack.fWidth * GetScreenWidthRatio();
	bottomCoord.fHeight = m_uiBottomBlackBack.fHeight * GetScreenWidthRatio();

	bottomCoord.fX = m_uiBottomBlackBack.fX - ((bottomCoord.fWidth - m_uiBottomBlackBack.fWidth)/2.0f);
	bottomCoord.fY = m_uiBottomBlackBack.fY;

	m_pStaticBottomBack->SetUICoord( bottomCoord );
}

void CDnDungeonEnterDlg::OnBlindOpen() 
{
	CDnLocalPlayerActor::LockInput(true);
//	CDnMouseCursor::GetInstance().ShowCursor( true );
}

void CDnDungeonEnterDlg::OnBlindOpened()
{
	if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NORMAL )
	{
		Show(true);
		m_pDungeonEnterLevelDlg->SetCallback( GetCallBack() );
		SetCallback( NULL );
		ShowChildDialog( m_pDungeonEnterLevelDlg, true );
		ShowChildDialog( m_pDungeonExpectDlg, true );
		GetInterface().ShowChatDialog();
	}
	else if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NEST )
	{
		Show(true);
		m_pDungeonNestEnterLevelDlg->SetCallback( GetCallBack() );
		SetCallback( NULL );
		ShowChildDialog( m_pDungeonNestEnterLevelDlg, true );
		ShowChildDialog( m_pDungeonExpectDlg, true );
		GetInterface().ShowChatDialog();
	}
}

void CDnDungeonEnterDlg::OnBlindClose()
{
	if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NORMAL )
	{
	#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
		if (m_pDungeonEnterLevelDlg == NULL)
			return;
	#endif
		m_pDungeonEnterLevelDlg->SetCallback( NULL );
		ShowChildDialog( m_pDungeonEnterLevelDlg, false );
		ShowChildDialog( m_pDungeonExpectDlg, false );
		Show( false );

	#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
		if (m_pDungeonEnterLevelDlg->IsCloseProcessingByUser() == false)
			SendCancelStage();
	#endif
	}
	else if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NEST )
	{
	#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
		if (m_pDungeonNestEnterLevelDlg == NULL)
			return;
	#endif
		m_pDungeonNestEnterLevelDlg->SetCallback( NULL );
		ShowChildDialog( m_pDungeonNestEnterLevelDlg, false );
		ShowChildDialog( m_pDungeonExpectDlg, false );
		Show( false );

	#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
		if (m_pDungeonNestEnterLevelDlg->IsCloseProcessingByUser() == false)
			SendCancelStage();
	#endif
	}
}

void CDnDungeonEnterDlg::OnBlindClosed()
{
	GetInterface().OpenBaseDialog();

	if( CDnPartyTask::GetInstance().IsSyncComplete() )
		CDnLocalPlayerActor::LockInput(false);
//	CDnMouseCursor::GetInstance().ShowCursor( false );
}

int CDnDungeonEnterDlg::GetDungeonLevel()
{
	if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NORMAL )
	{
		if( m_pDungeonEnterLevelDlg )
			return m_pDungeonEnterLevelDlg->GetDungeonLevel();
	}
	else if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NEST )
	{
		if( m_pDungeonNestEnterLevelDlg )
			return m_pDungeonNestEnterLevelDlg->GetDungeonLevel();
	}

	return -1;
}

void CDnDungeonEnterDlg::SetSelectDungeonInfo( int nMapIndex, int nDifficult )
{
	if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NORMAL )
	{
		if( !m_pDungeonEnterLevelDlg ) return;
		m_pDungeonEnterLevelDlg->SetSelectDungeonInfo( nMapIndex, nDifficult );

		// Do we really have to search through everything like this.
		for( int i = 0; i < (int)m_vecDungeonGateInfo.size(); ++i ) 
		{
			if( m_vecDungeonGateInfo[i].m_nMapIndex == nMapIndex ) 
			{
				m_pDungeonEnterLevelDlg->UpdateNeedItem( m_vecDungeonGateInfo[i].m_nNeedItemID, m_vecDungeonGateInfo[i].m_nNeedItemCount, m_vecDungeonGateInfo[i].m_cDungeonEnterPermit, NULL );
				break;
			}
		}
	}
	else if( m_eDungeonEnterType == DUNGEONENTER_TYPE_NEST )
	{
		if( !m_pDungeonNestEnterLevelDlg ) return;
		m_pDungeonNestEnterLevelDlg->SetSelectDungeonInfo( nMapIndex, nDifficult );

		// Do we really have to search through everything like this.
		for( int i = 0; i < (int)m_vecDungeonGateInfo.size(); ++i ) 
		{
			if( m_vecDungeonGateInfo[i].m_nMapIndex == nMapIndex ) 
			{
				m_pDungeonNestEnterLevelDlg->UpdateNeedItem( m_vecDungeonGateInfo[i].m_nNeedItemID, m_vecDungeonGateInfo[i].m_nNeedItemCount, m_vecDungeonGateInfo[i].m_cDungeonEnterPermit, NULL );
				break;
			}
		}
	}

	UpdateDungeonExpectReward( nMapIndex, nDifficult );
}

void CDnDungeonEnterDlg::UpdateDungeonExpectReward( int nMapIndex, int nDifficult )
{
	if( m_pDungeonExpectDlg && m_pDungeonExpectDlg->IsShow() ) 
		m_pDungeonExpectDlg->UpdateDungeonExpectReward( nMapIndex, nDifficult );
}
