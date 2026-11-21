#include "StdAfx.h"
#include "DnLifeSkillFishingTask.h"
#include "DnInterface.h"
#include "DnLifeSkillFishingDlg.h"
#include "DnPlayerActor.h"
#include "EtWorldSector.h"
#include "EtWorldEventArea.h"
#include "EtWorldEventControl.h"
#include "DnTableDB.h"
#include "SecondarySkillRepository.h"
#include "SecondarySkill.h"
#include "DnSkillTask.h"
#include "DnPlayerCamera.h"
#include "DnLocalPlayerActor.h"
#include "DnInventory.h"
#include "DnItemTask.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#include "DnLifeSkillCookingTask.h"
#include "DnCommonTask.h"
#include "TaskManager.h"
#ifdef PRE_ADD_CASHFISHINGITEM
#include "SyncTimer.h"
#endif // PRE_ADD_CASHFISHINGITEM

#include "DnBlow.h"


// Fishing order
// 1. Check fishing prerequisites and send a fishing request to the server
// 2. Server responds to start fishing
// 3. After fishing ends, request result from server at the end of the reeling animation
// 4. Receive result from server; repeat if auto-fishing, end if manual fishing

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnLifeSkillFishingTask::CDnLifeSkillFishingTask()
: CTaskListener( false )
, m_bRequestFishingReady( false )
, m_eFishingState( eFishingState::STATE_NONE )
, m_bRunningFishingTask( false)
, m_bActionNormalStand2( false )
, m_bRecieveFishingEnd( false )
#ifdef PRE_ADD_CASHFISHINGITEM
, m_bCashFishingRod( false )
, m_bCashFishingBait( false )
, m_nFishingRodItemID( 0 )
#endif // PRE_ADD_CASHFISHINGITEM
, m_nFishingRodIndex( -1 )
, m_nFishingBaitIndex( -1 )
//, m_fFishingRandomStandTime( FISHING_RANDOM_STAND_TIME )
, m_fRotate( 0.0f )
, m_vecPlayerStartPos( 0.0f, 0.0f, 0.0f )
, m_vecAreaDir( 0.0f, 0.0f )
#ifdef PRE_ADD_FISHING_RESULT_SOUND
, m_nFishingSuccessSound(-1)
, m_nFishingFailSound(-1)
#endif
{
	memset(&m_stFishingReward,0,sizeof(m_stFishingReward));
}

CDnLifeSkillFishingTask::~CDnLifeSkillFishingTask()
{
	SAFE_RELEASE_SPTR(m_hFishingSound);
}

bool CDnLifeSkillFishingTask::Initialize()
{
	return true;
}

void CDnLifeSkillFishingTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	if( nMainCmd == SC_FISHING )
	{
		switch( nSubCmd )
		{
			case eFishing::SC_READYFISHING:			OnRecvFishingReady( (SCFishingReady*)pData ); break;
			case eFishing::SC_FISHINGPATTERN:		OnRecvFishingStart( (SCFishingPattern*)pData ); break;
			case eFishing::SC_CASTBAITRESULT:		OnRecvCastBaitResult( (SCCastBait*)pData ); break;
			case eFishing::SC_STOPFISHINGRESULT:	OnRecvStopFishingResult( (SCStopFishing*)pData ); break;
			case eFishing::SC_FISHINGEND:			OnRecvFishingEnd(); break;
			case eFishing::SC_FISHINGREWARD:		OnRecvFishingResult( (SCFishingReward*)pData ); break;
			case eFishing::SC_FISHINGSYNC:			OnRecvFishingSync( (SCFishingSync*)pData ); break;
			case eFishing::SC_FISHINGPLAYER:		OnRecvFishingPlayer( (SCFishingPlayer*)pData ); break;
		}
	}
}

void CDnLifeSkillFishingTask::OnRecvFishingPlayer( SCFishingPlayer* pPacket )
{
	if( pPacket == NULL )
		return;

	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( !pPlayer ) return;

	pPlayer->CmdAction( "Fishing_Stand", 0, 3.0f, 0.0f, true, false );

#ifdef PRE_ADD_CASHFISHINGITEM
	pPlayer->CreateFishingRod( pPacket->nRodItemID );
#else // PRE_ADD_CASHFISHINGITEM
	pPlayer->CreateFishingRod( "Fishing Rod.skn", "Fishing Rod.ani", "Fishing Rod.act" );
#endif // PRE_ADD_CASHFISHINGITEM
	pPlayer->ShowFishingRod( true );
	pPlayer->SetFishingRodAction( "Fishing Rod_Stand" );
	pPlayer->SetHeadEffect( EffectType_Fishing, EffectState_Start );
}

bool CDnLifeSkillFishingTask::IsNowFishing()
{
	if( GetInterface().GetLifeSkillFishingDlg() == NULL )
		return false;

	if( m_eFishingState != STATE_NONE || ( GetInterface().GetLifeSkillFishingDlg()->IsShow() && GetInterface().GetLifeSkillFishingDlg()->IsAutoFishing() ) )
		return true;

	return false;
}

bool CDnLifeSkillFishingTask::CheckFishingTool()
{
#ifdef PRE_ADD_CASHFISHINGITEM
	m_bCashFishingRod = false;
	m_bCashFishingBait = false;
	m_nFishingRodItemID = 0;
#endif // PRE_ADD_CASHFISHINGITEM
	m_nFishingRodIndex = -1;
	m_nFishingBaitIndex = -1;

	CDnItem* pItem = NULL;
	CDnInventory::INVENTORY_MAP_ITER iter = GetItemTask().GetCharInventory().GetInventoryItemList().begin();
	for( ; iter != GetItemTask().GetCharInventory().GetInventoryItemList().end(); iter++ )
	{
		pItem = iter->second;
		if( !pItem )
			continue;

		if( pItem->GetItemType() == ITEMTYPE_FISHINGROD )
		{
			m_nFishingRodIndex = pItem->GetSlotIndex();
#ifdef PRE_ADD_CASHFISHINGITEM
			m_nFishingRodItemID = pItem->GetClassID();
#endif // PRE_ADD_CASHFISHINGITEM
		}

		if( pItem->GetItemType() == ITEMTYPE_FISHINGBAIT )
		{
			if( m_nFishingBaitIndex > -1 )	// Find the stack of fishing bait with the smallest count first
			{
				CDnItem* pFishingBaitItem = GetItemTask().GetCharInventory().GetItem( m_nFishingBaitIndex );
				int nPrevFishingBaitCount = pFishingBaitItem->GetOverlapCount();
				int nNextFishingBaitCount = pItem->GetOverlapCount();

				if( nNextFishingBaitCount <= nPrevFishingBaitCount )
					m_nFishingBaitIndex = pItem->GetSlotIndex();
			}
			else
				m_nFishingBaitIndex = pItem->GetSlotIndex();
		}
	}

#ifdef PRE_ADD_CASHFISHINGITEM
	bool bExsitFishingBait = false;
	if( m_nFishingBaitIndex > -1 )
		bExsitFishingBait = true;

	const time_t tNowTime = CSyncTimer::GetInstance().GetCurTime();
	iter = GetItemTask().GetCashInventory().GetInventoryItemList().begin();
	for( ; iter != GetItemTask().GetCashInventory().GetInventoryItemList().end(); iter++ )
	{
		pItem = iter->second;
		if( !pItem )
			continue;

		if( pItem->GetItemType() == ITEMTYPE_FISHINGROD && 
			( pItem->IsEternityItem() || tNowTime < *( pItem->GetExpireDate() ) ) )
		{
			m_bCashFishingRod = true;
			m_nFishingRodIndex = pItem->GetSlotIndex();
			m_nFishingRodItemID = pItem->GetClassID();
		}

		if( !bExsitFishingBait && pItem->GetItemType() == ITEMTYPE_FISHINGBAIT )
		{
			m_bCashFishingBait = true;
			if( m_nFishingBaitIndex > -1 )	// Find the stack of fishing bait with the smallest count first
			{
				CDnItem* pFishingBaitItem = GetItemTask().GetCashInventory().GetItem( m_nFishingBaitIndex );
				int nPrevFishingBaitCount = pFishingBaitItem->GetOverlapCount();
				int nNextFishingBaitCount = pItem->GetOverlapCount();

				if( nNextFishingBaitCount <= nPrevFishingBaitCount )
					m_nFishingBaitIndex = pItem->GetSlotIndex();
			}
			else
				m_nFishingBaitIndex = pItem->GetSlotIndex();
		}
	}
#endif // PRE_ADD_CASHFISHINGITEM

	bool bRetValue = false;
	if( m_nFishingRodIndex != -1 && m_nFishingBaitIndex != -1 )
		bRetValue = true;

	return bRetValue;
}

int CDnLifeSkillFishingTask::CheckFishingArea()
{
	CEtWorldGrid *pGrid = CDnWorld::GetInstance().GetGrid();

	if( !pGrid )
		return false;

	DNTableFileFormat* pSoxFishingArea = GetDNTable( CDnTableDB::TFISHINGAREA );
	if( pSoxFishingArea == NULL )
	{
		DN_ASSERT( 0, "Invalid pSoxFishingArea" );
		return false;
	}

	DNTableFileFormat* pSoxFishingPoint = GetDNTable( CDnTableDB::TFISHINGPOINT );
	if( pSoxFishingPoint == NULL )
	{
		DN_ASSERT( 0, "Invalid pSoxFishingPoint" );
		return false;
	}

	DWORD dwCount = pGrid->GetActiveSectorCount();
	CEtWorldEventControl* pControl = NULL;
	CEtWorldSector* pSector = NULL;
	EtVector2 vStartPos( 0.0f, 0.0f );

	int nFishingAreaID = -1;
	bool bWrongArea = true;

	for( DWORD i=0; i<dwCount; i++ )
	{
		pSector = pGrid->GetActiveSector( i );
		if( !pSector )
			continue;

		pControl = pSector->GetControlFromUniqueID( ETE_FarmFishingArea );
		if( !pControl )
			continue;

		pControl->Process( *CDnActor::s_hLocalActor->GetPosition() );

		if( pControl->GetCheckAreaCount() > 0 )
		{
			bWrongArea = false;
			bool bAvailableFishingLevel = false;
			SecondarySkill::Grade::eType eRequiredGrade =  SecondarySkill::Grade::Beginning;
			int nRequiredSkillLevel = -1;

			for( DWORD j=0; j<pControl->GetCheckAreaCount(); j++ )
			{
				CEtWorldEventArea *pArea = pControl->GetCheckArea( j );
				if( pArea == NULL )
					continue;

				int nFishingPointID = 0;
				int nEventAreaID = -1;
				for( int i=0; i<=pSoxFishingArea->GetItemCount(); i++ )
				{
					// Get the fishing point ID for the location with the matching Area ID from the Fishing Area table.
					if( pArea->GetCreateUniqueID() == pSoxFishingArea->GetFieldFromLablePtr( i, "_EventAreaID" )->GetInteger() )
					{
						nEventAreaID = pArea->GetCreateUniqueID();
						nFishingPointID = pSoxFishingArea->GetFieldFromLablePtr( i, "_FishingPointID" )->GetInteger();
						break;
					}
				}

				if( nFishingPointID == 0 || nEventAreaID == -1 )
					break;

				// Check whether the fishing point ID matches the level and grade in the fishing point table.
				eRequiredGrade = (SecondarySkill::Grade::eType)pSoxFishingPoint->GetFieldFromLablePtr( nFishingPointID, "_SecondarySkillClass" )->GetInteger();
				nRequiredSkillLevel = pSoxFishingPoint->GetFieldFromLablePtr( nFishingPointID, "_SecondarySkillLevel" )->GetInteger();

				CSecondarySkillRepository& pLifeSkillRepository = GetSkillTask().GetLifeSkillRepository();
				CSecondarySkill* pSecondarySkill = pLifeSkillRepository.Get( SecondarySkill::SubType::FishingSkill );
				if( pSecondarySkill )
				{
					if( eRequiredGrade <= pSecondarySkill->GetGrade() && nRequiredSkillLevel <= pSecondarySkill->GetLevel() )
					{
						bAvailableFishingLevel = true;
						nFishingAreaID = nEventAreaID;
						m_fRotate = pArea->GetRotate();
						m_vecAreaDir = EtVector2( sin( EtToRadian( m_fRotate ) ), cos( EtToRadian( m_fRotate ) ) );
					}
				}
			}

			if( nRequiredSkillLevel == -1 )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7494 ) );
				break;
			}

			if( !bAvailableFishingLevel )
			{
				WCHAR wszGrade[64] = {0,};
				int nGradeStringIndex = 0;
				switch( eRequiredGrade )
				{
					case SecondarySkill::Grade::Beginning:
						nGradeStringIndex = 7303;
						break;
					case SecondarySkill::Grade::Intermediate:
						nGradeStringIndex = 7304;
						break;
					case SecondarySkill::Grade::Advanced:
						nGradeStringIndex = 7305;
						break;
				}
				swprintf_s( wszGrade, _countof( wszGrade ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nGradeStringIndex ) );

				WCHAR wszTemp[1024] = {0, };
				swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7495 ), wszGrade, nRequiredSkillLevel );

				GetInterface().MessageBox( wszTemp );
			}
		}
	}

	if( bWrongArea )
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7494 ) );

	return nFishingAreaID;
}

void CDnLifeSkillFishingTask::SendFishingReady()
{
	CDnCommonTask* pCommonTask = dynamic_cast<CDnCommonTask*>( CTaskManager::GetInstance().GetTask( "CommonTask" ) );

	if( GetItemTask().IsRequestWait() || 
		GetLifeSkillCookingTask().IsRequestCooking() ||
		GetLifeSkillCookingTask().IsNowCooking() ||
		( pCommonTask && pCommonTask->IsRequestNpcTalk() ) )	// Do not start fishing if another use request is in progress
		return;

	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );

	if( pPlayer )
	{
		int nCurrentIndex = pPlayer->GetCurrentActionIndex();
		int nJumpIndex = pPlayer->GetElementIndex( "Normal_Jump" );
		int nJumpLandingIndex = pPlayer->GetElementIndex( "Normal_Jump_Landing" );

 		if( nCurrentIndex == nJumpIndex || nCurrentIndex == nJumpLandingIndex )	// Cannot start fishing while jumping
			return;

		if( pPlayer->IsBattleMode() )
			pPlayer->CmdToggleBattle( false );

		if( pPlayer->IsCanVehicleMode() && ( pPlayer->IsVehicleMode() || pPlayer->GetVehicleInfo().Vehicle[0].nItemID != 0 ) )
		{
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7497 ) );
		}
		else if( !CheckFishingTool() )
		{
			if( m_nFishingRodIndex == -1 )
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7498 ) );
			else if( m_nFishingBaitIndex == -1 )
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7499 ) );
		}
		else if( !GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG, ITEM_SLOT_TYPE::ST_INVENTORY ) )
		{
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7500 ) );
		}
		else
		{
			int nFishingAreaID = CheckFishingArea();
			if( nFishingAreaID != -1 )
			{
				pPlayer->ProcessFlushPacketQueue();

				CSFishingReady stFishingReady;
				stFishingReady.nFishingAreaIdx = nFishingAreaID;
#ifdef PRE_ADD_CASHFISHINGITEM
				if( m_bCashFishingRod )
				{
					CDnItem* pItem = GetItemTask().GetCashInventory().GetItem( m_nFishingRodIndex );
					if( pItem )
						stFishingReady.ToolInfo.biCashRodSerial = pItem->GetSerialID();

					stFishingReady.ToolInfo.nFishingRodInvenIdx = -1;
				}
				else
				{
					stFishingReady.ToolInfo.biCashRodSerial = -1;
					stFishingReady.ToolInfo.nFishingRodInvenIdx = m_nFishingRodIndex;
				}

				if( m_bCashFishingBait )
				{
					CDnItem* pItem = GetItemTask().GetCashInventory().GetItem( m_nFishingBaitIndex );
					if( pItem )
						stFishingReady.ToolInfo.biCashBaitSerial = pItem->GetSerialID();

					stFishingReady.ToolInfo.nFishingBaitInvenIdx = -1;
				}
				else
				{
					stFishingReady.ToolInfo.biCashBaitSerial = -1;
					stFishingReady.ToolInfo.nFishingBaitInvenIdx = m_nFishingBaitIndex;
				}
#else	//#ifdef PRE_ADD_CASHFISHINGITEM
				stFishingReady.nFishingRodInvenIdx = m_nFishingRodIndex;
				stFishingReady.nFishingBaitInvenIdx = m_nFishingBaitIndex;
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

				CClientSessionManager::GetInstance().SendPacket( CS_FISHING, eFishing::eCSFishing::CS_READYFISHING, (char*)&stFishingReady, int(sizeof(CSFishingReady)) );
				m_bRecieveFishingEnd = false;
				m_bRequestFishingReady = true;
			}
		}
	}
}

void CDnLifeSkillFishingTask::OnRecvFishingReady( SCFishingReady* pPacket )
{
	if( pPacket->nRetCode == ERROR_NONE )
		SendFishingStart();
	else
		GetInterface().ServerMessageBox( pPacket->nRetCode );
}

void CDnLifeSkillFishingTask::SendFishingStart()
{
	if( !GetInterface().GetLifeSkillFishingDlg() )
		return;

	CSCastBait stCastBait;

	if( GetInterface().GetLifeSkillFishingDlg()->IsAutoFishing() )
		stCastBait.eCastType = Fishing::Cast::eCast::CASTAUTO;
	else
		stCastBait.eCastType = Fishing::Cast::eCast::CASTMANUAL;

#ifdef PRE_ADD_CASHFISHINGITEM
	if( m_bCashFishingRod )
	{
		CDnItem* pItem = GetItemTask().GetCashInventory().GetItem( m_nFishingRodIndex );
		if( pItem )
			stCastBait.ToolInfo.biCashRodSerial = pItem->GetSerialID();

		stCastBait.ToolInfo.nFishingRodInvenIdx = -1;
	}
	else
	{
		stCastBait.ToolInfo.biCashRodSerial = -1;
		stCastBait.ToolInfo.nFishingRodInvenIdx = m_nFishingRodIndex;
	}

	if( m_bCashFishingBait )
	{
		CDnItem* pItem = GetItemTask().GetCashInventory().GetItem( m_nFishingBaitIndex );
		if( pItem )
			stCastBait.ToolInfo.biCashBaitSerial = pItem->GetSerialID();

		stCastBait.ToolInfo.nFishingBaitInvenIdx = -1;
	}
	else
	{
		stCastBait.ToolInfo.biCashBaitSerial = -1;
		stCastBait.ToolInfo.nFishingBaitInvenIdx = m_nFishingBaitIndex;
	}
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
	stCastBait.nFishingRodInvenIdx = m_nFishingRodIndex;
	stCastBait.nFishingBaitInvenIdx = m_nFishingBaitIndex;
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

	CClientSessionManager::GetInstance().SendPacket( CS_FISHING, eFishing::eCSFishing::CS_CASTBAIT, (char*)&stCastBait, int(sizeof(CSCastBait)) );
}

void CDnLifeSkillFishingTask::OnRecvCastBaitResult( SCCastBait* pPacket )
{
	if( pPacket->nRetCode != ERROR_NONE )
		GetInterface().ServerMessageBox( pPacket->nRetCode );
}

void CDnLifeSkillFishingTask::OnRecvFishingStart( SCFishingPattern* pPacket )
{
	m_bRunningFishingTask = true;

	// Set values using the received fishing pattern ID.
	DNTableFileFormat* pSoxFishing = GetDNTable( CDnTableDB::TFISHING );
	if( pSoxFishing == NULL )
	{
		DN_ASSERT( 0, "Invalid pSoxFishing" );
		return;
	}

	int nFishingPattern = pPacket->nPatternID;
	stFishingInfo finshinginfo;
	finshinginfo.m_fStrengthTime = static_cast<float> ( pSoxFishing->GetFieldFromLablePtr( nFishingPattern, "_MaxTime" )->GetInteger() );

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	//Decrease auto-fishing time by n seconds..
	DnActorHandle hActor = CDnActor::s_hLocalActor;
	float incTimeValue = 0.0f;
	if (hActor && hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_264))
	{
		DNVector(DnBlowHandle) vlBlows;
		hActor->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_264, vlBlows);
		{
			int nCount = (int)vlBlows.size();
			for (int i = 0; i < nCount; ++i)
			{
				DnBlowHandle hBlow = vlBlows[i];
				if (hBlow && hBlow->IsEnd() == false)
				{
					incTimeValue += hBlow->GetFloatValue();
				}
			}
		}
	}

	finshinginfo.m_fStrengthTime -= incTimeValue;

#if defined(PRE_ADD_CASHFISHINGITEM)
	pPacket->nFishingAutoMaxTime -= incTimeValue;
#endif // PRE_ADD_CASHFISHINGITEM
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

	finshinginfo.m_fPowerSpeedTime = static_cast<float> ( pSoxFishing->GetFieldFromLablePtr( nFishingPattern, "_TargetSpeed" )->GetInteger() );
	finshinginfo.m_fPowerTime = FISHING_POWER_TIME / finshinginfo.m_fPowerSpeedTime;
	finshinginfo.m_fSpeedUpStartTime = finshinginfo.m_fPowerTime * ( static_cast<float> ( pSoxFishing->GetFieldFromLablePtr( nFishingPattern, "_TargetMin" )->GetInteger() ) / 100.0f );
	finshinginfo.m_fSpeedUpEndTime = finshinginfo.m_fPowerTime * ( static_cast<float> ( pSoxFishing->GetFieldFromLablePtr( nFishingPattern, "_TargetMax" )->GetInteger() ) / 100.0f );
	finshinginfo.m_fSpeedUpFactor = static_cast<float> ( pSoxFishing->GetFieldFromLablePtr( nFishingPattern, "_TargetUpTime" )->GetInteger() ) / 100.0f;

#ifdef PRE_ADD_CASHFISHINGITEM
	if( GetInterface().GetLifeSkillFishingDlg()->IsAutoFishing() )
		finshinginfo.m_fStrengthTime = (float)pPacket->nFishingAutoMaxTime;
#endif	// PRE_ADD_CASHFISHINGITEM

	GetInterface().GetLifeSkillFishingDlg()->SetFishingInfo( finshinginfo );
	GetInterface().GetLifeSkillFishingDlg()->RecieveResult( false );

	// Open guide window when starting basic fishing at beginner level
	if( !GetInterface().GetLifeSkillFishingDlg()->IsAutoFishing() )
	{
		CSecondarySkillRepository& pLifeSkillRepository = GetSkillTask().GetLifeSkillRepository();
		CSecondarySkill* pSecondarySkill = pLifeSkillRepository.Get( SecondarySkill::SubType::FishingSkill );
		if( pSecondarySkill )
		{
			if( pSecondarySkill->GetGrade() == SecondarySkill::Grade::Beginning )
				GetInterface().GetLifeSkillFishingDlg()->SetShowGuide( true );
		}
	}

	if( !GetInterface().GetLifeSkillFishingDlg()->IsShow() )
	{
		GetInterface().CloseAllMainMenuDialog();
		GetInterface().GetLifeSkillFishingDlg()->Show( true );
	}

	m_eFishingState = STATE_FISHING;

	CDnLocalPlayerActor* pLocalPlayer = dynamic_cast<CDnLocalPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
	if( pLocalPlayer )
		pLocalPlayer->LockInput( true );

	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
	if( pPlayer )
	{
		float fDot = D3DXVec2Dot( &m_vecAreaDir, &EtVec3toVec2( pPlayer->GetMatEx()->m_vZAxis ) );
		if( fDot >= 1.0f )
			fDot = 1.0f;

		float fAngle = EtToDegree( EtAcos(fDot) );
		OutputDebug( "Player Pos : (%f, %f, %f)\n", pPlayer->GetPosition()->x, pPlayer->GetPosition()->y, pPlayer->GetPosition()->z );

		if( fAngle > 60.0f )	// Change direction only if the angle difference exceeds the threshold
		{
			OutputDebug( "fAngle : %f\n", fAngle );

			EtVector3 vCrossVec;
			D3DXVec3Cross( &vCrossVec, &EtVec2toVec3( m_vecAreaDir ), &pPlayer->GetMatEx()->m_vZAxis );

			if( vCrossVec.y > 0.0f )
				pPlayer->GetMatEx()->RotateYaw( fAngle );
			else 
				pPlayer->GetMatEx()->RotateYaw( -fAngle );
		}

		m_vecPlayerStartPos = *( pPlayer->GetPosition() );
		pPlayer->CmdAction( "Fishing_casting", 0, 3.0f, 0.0f, true, false );
#ifdef PRE_ADD_CASHFISHINGITEM
		pPlayer->CreateFishingRod( GetFishingRodItemID() );
#else // PRE_ADD_CASHFISHINGITEM
		pPlayer->CreateFishingRod( "Fishing Rod.skn", "Fishing Rod.ani", "Fishing Rod.act" );
#endif // PRE_ADD_CASHFISHINGITEM
		pPlayer->ShowFishingRod( true );
		pPlayer->SetFishingRodAction( "Fishing Rod_Casting" );
		SendAction( eActor::eCSActor::CS_FISHINGROD_CAST );

		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7491 ), false );
	}
}

void CDnLifeSkillFishingTask::OnRecvFishingSync( SCFishingSync* pPacket )
{
	OutputDebug( "Fishing Sync : %d, %d\n", pPacket->nReduceTick, pPacket->nFishingGauge );
	GetInterface().GetLifeSkillFishingDlg()->FishingTimeSync( pPacket->nReduceTick, pPacket->nFishingGauge );
}

void CDnLifeSkillFishingTask::ResetFishing( bool bRevisePos )
{
	if( !bRevisePos )
	{
		CDnLocalPlayerActor* pLocalPlayer = dynamic_cast<CDnLocalPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
		if( pLocalPlayer )
			pLocalPlayer->LockInput( false );
	}

	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
	if( pPlayer )
	{
		if( bRevisePos )
			pPlayer->SetPosition( m_vecPlayerStartPos );
		pPlayer->CmdStop( "Stand" );
	}

	m_bRequestFishingReady = false;
	m_eFishingState = STATE_NONE;
	m_bRunningFishingTask = false;
	m_fRotate = 0.0f;
	m_vecPlayerStartPos = EtVector3( 0.0f, 0.0f, 0.0f );
	m_vecAreaDir = EtVector2( 0.0f, 0.0f );

	// Remove fishing rod
	pPlayer->ShowFishingRod( false );
	SendAction( eActor::eCSActor::CS_FISHINGROD_HIDE );
}

void CDnLifeSkillFishingTask::SendFishingCancel( bool bRevisePos )
{
	ResetFishing( bRevisePos );
	// Notify server of fishing cancellation
	CClientSessionManager::GetInstance().SendPacket( CS_FISHING, eFishing::eCSFishing::CS_STOPFISHING, NULL, 0 );
}

void CDnLifeSkillFishingTask::OnRecvStopFishingResult( SCStopFishing* pPacket )
{
	if( pPacket->nRetCode != ERROR_NONE )
	{
		ResetFishing( false );
		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}

void CDnLifeSkillFishingTask::OnRecvFishingEnd()
{
	OutputDebug( "Recieve Fishing End" );
	// Disable the interface cancel button.
	GetInterface().GetLifeSkillFishingDlg()->RecieveResult( true );
	m_bRecieveFishingEnd = true;
}

void CDnLifeSkillFishingTask::OnRecvFishingResult( SCFishingReward* pPacket )
{
	OutputDebug( "Recieve Fishing Result : %d ( %d )\n", pPacket->nRetCode, pPacket->nRewardItemID );

	bool bResultIsMine = false;
	if( CDnLocalPlayerActor::s_hLocalActor && pPacket->nSessionID == CDnLocalPlayerActor::s_hLocalActor->GetUniqueID() )
		bResultIsMine = true;

	if( pPacket->nRetCode == ERROR_NONE || pPacket->nRetCode == ERROR_FISHING_FAIL )
	{
		m_stFishingReward.nRetCode = pPacket->nRetCode;
		m_stFishingReward.nRewardItemID = pPacket->nRewardItemID;

		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
		if( !hActor ) return;

		CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
		if( !pPlayer ) return;

		pPlayer->SetFishingReward( m_stFishingReward.nRetCode, m_stFishingReward.nRewardItemID );

		if( bResultIsMine && m_bRecieveFishingEnd )
		{
			if( m_stFishingReward.nRetCode == ERROR_NONE )	// Fishing succeeded
			{
				GetInterface().OpenMovieAlarmDlg( NULL, 2.0f, m_stFishingReward.nRewardItemID );
				// GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7492 ), false );
#ifdef PRE_ADD_FISHING_RESULT_SOUND
				const char* szFileName = CDnTableDB::GetInstance().GetFileName(20927);
				m_nFishingSuccessSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName(szFileName).c_str(), false, false );
				if( m_nFishingSuccessSound != -1 )
				{
					m_hFishingSound = CEtSoundEngine::GetInstance().PlaySound("2D", m_nFishingSuccessSound, false, true);
					if(m_hFishingSound)
					{
						m_hFishingSound->SetVolume( CEtSoundEngine::GetInstance().GetMasterVolume("2D"));
						m_hFishingSound->Resume();
					}
				}
#endif
				CDnItem *pItem = CDnItemTask::GetInstance().GetCharInventory().FindItem( m_stFishingReward.nRewardItemID );
				if( pItem )
				{
					WCHAR wszMsg[256];
					swprintf_s( wszMsg , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7551) , pItem->GetName() );
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszMsg , false );
				}
			}
			else if( m_stFishingReward.nRetCode == ERROR_FISHING_FAIL )	// Fishing failed
			{
				GetInterface().OpenMovieAlarmDlg( NULL, 2.0f, 0, 0, 0, 7490 );
				GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7490 ), false );
#ifdef PRE_ADD_FISHING_RESULT_SOUND
				const char* szFileName = CDnTableDB::GetInstance().GetFileName(20928);
				m_nFishingFailSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName(szFileName).c_str(), false, false );
				if( m_nFishingFailSound != -1 )
				{
					m_hFishingSound = CEtSoundEngine::GetInstance().PlaySound("2D", m_nFishingFailSound, false, true);
					if(m_hFishingSound)
					{
						m_hFishingSound->SetVolume( CEtSoundEngine::GetInstance().GetMasterVolume("2D"));
						m_hFishingSound->Resume();
					}
				}
#endif
			}

			m_eFishingState = STATE_SHOW_RESULT;
		}
	}
	else
	{
		if( bResultIsMine )
		{
			ResetFishing( false );
			GetInterface().GetLifeSkillFishingDlg()->Show( false );
			GetInterface().ServerMessageBox( pPacket->nRetCode );
		}
	}
}

void CDnLifeSkillFishingTask::DoFishingEnd()
{
	if( m_bRecieveFishingEnd )
	{
		m_eFishingState = STATE_FISHING_END;
		CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
		if( pPlayer )
		{
			pPlayer->CmdAction( "Fishing_Lift", 0, 3.0f, 0.0f, true, false );
			pPlayer->SetFishingRodAction( "Fishing Rod_Lift." );
			SendAction( eActor::eCSActor::CS_FISHINGROD_LIFT );
		}
	}
}

void CDnLifeSkillFishingTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bDestroyTask ) return;
	if( !m_bRunningFishingTask ) return;

	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
	if( pPlayer == NULL )
		return;

	switch( m_eFishingState )
	{
		case STATE_FISHING:	// Randomly insert fishing action in the middle
			{
/*				m_fFishingRandomStandTime -= fDelta;
				if( m_fFishingRandomStandTime <= 0.0f )
				{
					if( _rand() % 3 == 0 )
						pPlayer->CmdAction( "Fishing_Stand2", 0, 3.0f, 0.0f, true, false );

					m_fFishingRandomStandTime = FISHING_RANDOM_STAND_TIME;
				}*/
			}
			break;
		case STATE_FISHING_END:	// Output fishing result when fishing animation completes.
			{
				int nCurrentIndex = pPlayer->GetCurrentActionIndex();
				int nElementIndex = pPlayer->GetElementIndex( "Fishing_Normal Stand2" );

				if( nCurrentIndex == nElementIndex )
					CClientSessionManager::GetInstance().SendPacket( CS_FISHING, eFishing::eCSFishing::CS_FISHINGREWARD, NULL, 0 );
			}
			break;
		case STATE_SHOW_RESULT:
			{
				int nCurrentIndex = pPlayer->GetCurrentActionIndex();
				int nElementIndex = pPlayer->GetElementIndex( "Fishing_Normal Stand" );

				if( nCurrentIndex == nElementIndex )
				{
//					m_eFishingState = STATE_NONE;
					bool bFishingEnd = false;

					if( GetInterface().GetLifeSkillFishingDlg()->IsAutoFishing() )
					{
						pPlayer->SetPosition( m_vecPlayerStartPos );

						if( !CheckFishingTool() )
						{
							if( m_nFishingRodIndex == -1 )
								GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7498 ) );
							else if( m_nFishingBaitIndex == -1 )
								GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7499 ) );

							bFishingEnd = true;
						}
						else if( !GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG, ITEM_SLOT_TYPE::ST_INVENTORY ) )
						{
							GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7500 ) );
							bFishingEnd = true;
						}

						if( !bFishingEnd )
							SendFishingStart();
					}
					else
						bFishingEnd = true;

					if( bFishingEnd )
					{
						GetInterface().GetLifeSkillFishingDlg()->Show( false );
						SendFishingCancel( false );

						CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
						if( pInvenDlg && pInvenDlg->IsShow() )
							pInvenDlg->EnableSortButton();
					}
				}
			}
			break;
	}
}

void CDnLifeSkillFishingTask::SendPullingRodElapsedTime( bool bPullingRod )
{
	CSPullingRod stPullingRod;

	if( bPullingRod )
		stPullingRod.ePullingControl = Fishing::Control::eControl::CONTROL_PULLING;
	else
		stPullingRod.ePullingControl = Fishing::Control::eControl::CONTROL_NONE;
	
	CClientSessionManager::GetInstance().SendPacket( CS_FISHING, eFishing::eCSFishing::CS_PULLINGROD, (char*)&stPullingRod, int(sizeof(CSPullingRod)) );
}

void CDnLifeSkillFishingTask::SendAction( eActor::eCSActor eAction )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor* plocalActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));

	if( plocalActor )
	{
		BYTE pBuffer[32] = {0};
		CPacketCompressStream Stream( pBuffer, 32 );
#ifdef PRE_ADD_CASHFISHINGITEM
		Stream.Write( &m_nFishingRodItemID, sizeof(int) );
#endif // PRE_ADD_CASHFISHINGITEM
		plocalActor->Send( eAction, &Stream );
	}
}
