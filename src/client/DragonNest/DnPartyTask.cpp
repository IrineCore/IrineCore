#include "StdAfx.h"
#include "DnPartyTask.h"
#include "DnDebugConsole.h"
#include "DNPacket.h"

#include "PartySendPacket.h"
#include "GameSendPacket.h"

#include "DnWorld.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "TaskManager.h"

#include "DnInterface.h"
#include "DnInterfaceDlgID.h"
#include "DnMessageManager.h"

#include "DNPacket.h"
#include "DnLocalPlayerActor.h"
#include "DnGameTask.h"
#include "DnBridgeTask.h"
#include "DnTableDB.h"
#include "DnInCodeResource.h"
#include "DnCommonTask.h"

#include "DnTradeTask.h"
#include "DnTradePrivateMarket.h"
#include "DnClientScriptAPI.h"
#include "DnMainMenuDlg.h"
#include "DnDropItem.h"
#include "DnIsolate.h"

#include "DnChatTabDlg.h"
#include "DnChatOption.h"
#include "DnMainDlg.h"
#include "DnMainMenuDlg.h"
#include "DnDungeonClearMoveDlg.h"
#include "DnInterfaceString.h"

#include "DnNameLinkMng.h"

#include "DnCashShopTask.h"

#include "GameOption.h"

#include "DnGaugeDlg.h"

#include "DnChatRoomDlg.h"
#include "DnAcceptRequestDlg.h"
#include "SystemSendPacket.h"
#include "DnChatRoomTask.h"
#include "ChatRoomSendPacket.h"

#ifdef PRE_ADD_SECONDARY_SKILL
#include "DnLifeSkillCookingTask.h"
#endif // PRE_ADD_SECONDARY_SKILL

#include "DnPetTask.h"

#if defined (PRE_FIX_NESTREBIRTH)
#include "DnItemTask.h"
#endif
#include "DnProgressSimpleMessageBox.h"
#include "DnNestInfoDlg.h"
#include "DnVillageTask.h"

#include "DnDungeonClearMoveDlg.h"
#include "DnDarklairClearImp.h"

#ifdef PRE_MOD_SYSTEM_STATE
#include "DnSystemStateManager.h"
#endif

#ifdef PRE_PARTY_DB
#include "DnCommunityDlg.h"
#include "DnPartyListDlg.h"
#include "DnMainMenuDlg.h"
#endif
#include "DnDLGameTask.h"


#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
#include "DnDirectDlg.h"
#endif 

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
#include "DnPotentialJewelDlg.h"
#endif

#include "FarmSendPacket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 


CDnPartyTask::CDnPartyTask() : CTaskListener(true)
{
	m_PartyRole = PartyRoleEnum::SINGLE;
	m_PrevPartyRole	= PartyRoleEnum::SINGLE;
	m_nEnteredGateIndex = -1;
	m_bRequestEnteredGate = false;
	m_nRequestEnteredGateIndex = -1;
	m_bSyncComplete = false;
	m_bWaitGate = false;
	m_emPartyRouletteLevel = ITEMLOOTRULE_NONE;
	m_emPartyRouletteItemRank = ITEMRANK_D;
#ifdef PRE_PARTY_DB
	m_bAllowWorldZoneMapList = (DEFAULT_PARTY_LIST_ALLOW_WORLDZONE != 0);
	m_bLockReqPartyList = false;
#endif
	m_bLockReqPartyMemberInfo = false;
	m_nTargetStageIdx = 0;
#ifdef PRE_PARTY_DB
	m_TargetStageDifficulty = Dungeon::Difficulty::Easy;
#else
	m_nTargetStageDifficulty = 0;
#endif
	m_bJobDice = false;

	m_nStageDifficultyForSort = 0;
	m_bResetSort = true;

	m_LocalData.nSessionID = CDnBridgeTask::GetInstance().GetSessionID();
	memset( &m_PvPInfo, 0, sizeof(TPvPGroup) );
	m_PartyState = ePartyState::NORMAL;
	m_bReversionItemSharing = false;
	memset(m_PartySoundIndex, 0, sizeof(m_PartySoundIndex));
	m_bWaitingPartyFindDlgOpen = false;

	m_bSingleToPartyByGMTrace = false;

	m_nKeepPartyBonusRate = 0;
	m_nBestFriendBonusRate = 0;

	m_bPartyMemberInviting = false;
	m_bAdvanceSearching = false;
#ifdef PRE_PARTY_DB
	m_nStageIdxListForSort.reserve(Party::Max::DetailSearchMapCount);
#else
	m_nStageIdxListForSort.reserve(PARTYSORTCONDITIONMAX);
#endif
	m_PartyType = _NORMAL_PARTY;

	memset(&m_sPVPLadderInfo,0,sizeof(m_sPVPLadderInfo));
	memset(&m_PvPGhoulInfo,0,sizeof(m_PvPGhoulInfo));

	m_PartyProcessState = PPS_NONE;
	m_bNoWarpBeforeItemSharing = false;

	m_PartyReconnectCheckCounter = 0;
	m_bPartyAcceptRequesting = false;
	m_nLocalActorPartyIndex = 0;
	m_nMasterSessionID = 0;
	m_nMaxPartyMemberCount = 0;
#if defined( PRE_PARTY_DB )
#else
	m_nMaxUserLevel = 0;
#endif // #if defined( PRE_PARTY_DB )
	m_nMinUserLevel = 0;
	m_PartyID = 0;
	m_nPartyIndexToJoin = 0;
	m_nStartingMemberCount = 0;
#ifdef PRE_PARTY_DB
	m_iPartyPassword = Party::Constants::INVALID_PASSWORD;
#else
	memset(m_wszPartyPasswordToJoin, 0, sizeof(m_wszPartyPasswordToJoin));
#endif

#ifdef PRE_WORLDCOMBINE_PARTY
	m_nCurWorldCombinePartyTableIndex = Party::Constants::INVALID_WORLDCOMBINE_TABLE_INDEX;
#endif

#if defined (PRE_PARTY_DB) && defined (_WORK)
	m_fPartyListRefreshTime = Party::Constants::PARTY_LIST_REFRESH_SEC_CLIENT;
#endif
}

CDnPartyTask::~CDnPartyTask()
{
	for ( int i = 0 ; i < SOUNDMAX ; i++ )
	{
		CEtSoundEngine::GetInstance().RemoveSound( m_PartySoundIndex[i] );
	}
}

bool CDnPartyTask::Initialize()
{
	InitParty();
	m_nPartyIndexToJoin = -1;
#ifdef PRE_PARTY_DB
#else
	m_wszPartyPasswordToJoin[0] = '\0';
#endif

	m_PartySoundIndex[CREATE]	= CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10028 ), false, false );
	m_PartySoundIndex[ENTER]	= CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10029 ), false, false );
	m_PartySoundIndex[KICK]		= CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10030 ), false, false );
	m_PartySoundIndex[EXILE]	= CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10031 ), false, false );
	m_PartySoundIndex[LOOTCHANGE] = CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10032 ), false, false );
	m_PartySoundIndex[REQUESTREADY] = CEtSoundEngine::GetInstance().LoadSound( CDnTableDB::GetInstance().GetFileName( 10038 ), false, false );

	return true;
}

void CDnPartyTask::FinalizeGameTask()
{
	if( m_PrevPartyRole == PartyRoleEnum::SINGLE )
	{
		m_PartyRole		= m_PrevPartyRole;
		m_PrevPartyRole	= m_PartyRole;
		m_VecPartyList.clear();	//Issue #15007 - Clear PartyList if previous PartyRole was SINGLE.
	}

	m_nLocalActorPartyIndex = 0;

	for( std::vector<PartyStruct>::iterator itor=m_VecPartyList.begin() ; itor!=m_VecPartyList.end() ; )
	{
		if( (*itor).bGMTrace )
		{
			itor = m_VecPartyList.erase( itor );
		}
		else
			++itor;
	}

	for( UINT i=0 ; i<m_VecPartyList.size() ; ++i )
	{
		if( m_VecPartyList[i].nSessionID == CDnBridgeTask::GetInstance().GetSessionID() )
		{
			m_nLocalActorPartyIndex = i;
		}
	}
}

void CDnPartyTask::InitParty()
{
	m_PartyRole = PartyRoleEnum::SINGLE;
	m_VecPartyList.clear();
	m_PartyID = -1;
	m_nMaxPartyMemberCount = 0;
	m_nStartingMemberCount = 0;
	m_nLocalActorPartyIndex = 0;
	m_szPartyName.clear();
#ifdef PRE_PARTY_DB
	m_iPartyPassword = Party::Constants::INVALID_PASSWORD;
#else
	m_szPartyPassword.clear();
	m_nMaxUserLevel = 1;
#endif
	m_nMinUserLevel = 1;
	m_nMasterSessionID = 0;
	m_PartyState = ePartyState::NORMAL;
	m_emPartyRouletteLevel = ITEMLOOTRULE_NONE;
	m_emPartyRouletteItemRank = ITEMRANK_NONE;
#ifdef PRE_PARTY_DB
	m_bAllowWorldZoneMapList = (DEFAULT_PARTY_LIST_ALLOW_WORLDZONE != 0);
#endif
	m_bSingleToPartyByGMTrace = false;

	m_bPartyMemberInviting = false;
	m_PartyType = _NORMAL_PARTY;
	m_bPartyAcceptRequesting = false;
	m_PartyProcessState = PPS_NONE;
}

void CDnPartyTask::SetLocalData( DnActorHandle hActor )
{
	m_LocalData.hActor = hActor;
	wsprintf( m_LocalData.wszCharacterName, hActor->GetName() );
	CDnPlayerActor *pActor = (CDnPlayerActor *)hActor.GetPointer();
	m_LocalData.cClassID = pActor->GetClassID();
	m_LocalData.cLevel = pActor->GetLevel();
	m_LocalData.nExp = pActor->GetExperience();
	m_LocalData.cPvPLevel = pActor->GetPvPLevel();
	m_LocalData.dwHairColor = pActor->GetPartsColor( MAPartsBody::HairColor );
	m_LocalData.dwEyeColor = pActor->GetPartsColor( MAPartsBody::EyeColor );
	m_LocalData.dwSkinColor = pActor->GetPartsColor( MAPartsBody::SkinColor );

	m_LocalData.nVecJobHistoryList.clear();

	// Rotha - Fix infinite vector push_back bug
	m_LocalData.VecCashParts.clear();
	m_LocalData.VecGlyph.clear();
	m_LocalData.VecParts.clear();
	m_LocalData.VecSkill.clear();
	// Local party struct <self info> update timing differs by type, so observe further before modifying. = WIP =

	((CDnLocalPlayerActor*)hActor.GetPointer())->GetJobHistory( m_LocalData.nVecJobHistoryList );

	if( m_nLocalActorPartyIndex > 0 && m_VecPartyList.size() > static_cast<size_t>(m_nLocalActorPartyIndex) )
	{
		m_VecPartyList[m_nLocalActorPartyIndex].nVecJobHistoryList.clear();
		m_VecPartyList[m_nLocalActorPartyIndex].nVecJobHistoryList = m_LocalData.nVecJobHistoryList;
	}
}

void CDnPartyTask::EnableWaitGateState(bool bEnable)
{
	m_bWaitGate = bEnable;
}

void CDnPartyTask::SetLocalActorEnterGateIdx(int idx, bool bForceMsgOff)
{
	if( idx != -1 && m_nEnteredGateIndex != idx ) {
		bool bShowCaption = false;
		for( DWORD i=0; i<m_VecPartyList.size(); i++ ) {
			if( m_nLocalActorPartyIndex == i ) continue;
			if( m_VecPartyList[i].bGMTrace ) continue;

			if( m_VecPartyList[i].nEnteredGateIndex != idx ) {
				bShowCaption = true;
				break;
			}
		}
		if( bShowCaption && bForceMsgOff == false) 
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1373), true );
	}
	m_nEnteredGateIndex = idx;
}

void CDnPartyTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !CDnActor::s_hLocalActor ) return;

	if( CTaskManager::GetInstance().GetTask( "GameTask" ) ) {
		if( !m_bSyncComplete ) return;
	}

	UpdateBreakIntoPartyList();

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	// So this makes the party call SendPartyAskJoinDecision().. 
	m_AcceptRequestAskInfo.Process(fDelta);
	m_AcceptRequestInviteInfo.Process(fDelta);
#else
	// All timing handling for each request is done here; the dialog only displays the processed values (such as time).
	// Must be placed above waitGate so the progress bar decreases even near the gate.
	LIST_INVITEINFO_ITER iter = m_listInviteInfo.begin();
	for( ; iter != m_listInviteInfo.end(); )
	{
		iter->m_fAcceptTime -= fDelta;

		if( iter->m_fAcceptTime <= 0.0f || GetInterface().IsOpenBlind() )
		{
			ReqDenyPartyInvite(*iter);
			iter = m_listInviteInfo.erase( iter );
			continue;
		}
		++iter;
	}

	std::list<SAskInfo>::iterator askIter = m_listAskInfo.begin();
	for (; askIter != m_listAskInfo.end();)
	{
		SAskInfo& info = *askIter;
		info.fAcceptTime -= fDelta;

		if (info.fAcceptTime <= 0.0f || GetInterface().IsOpenBlind())
		{
			AnswerPartyAsking(false, info);
			askIter = m_listAskInfo.erase(askIter);
			continue;
		}
		++askIter;
	}
#endif // PRE_MOD_INTEG_SYSTEM_STATE

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	ProcessEffectSkill( fDelta );
#endif

	if (IsWaitingGate())
		return;

	if (m_PartyState == ePartyState::STAGE_CLEAR_WARP_STANDBY)
	{
		CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstance().GetTask( "GameTask" ));
		if (pGameTask != NULL)
		{
			CDnWorld::MapTypeEnum mapType = CDnWorld::GetInstance().GetMapType();
			bool bShow = false;

			if (pGameTask->IsEnableDungeonClearLeaderWarp() &&
				m_bReversionItemSharing == false &&
				m_PartyRole != MEMBER && 
				mapType != CDnWorld::MapTypeVillage &&
				mapType != CDnWorld::MapTypeWorldMap)
				bShow = true;

			if ((GetInterface().GetDungeonMoveDlg() && GetInterface().GetDungeonMoveDlg()->IsShow() == true) ||
				GetInterface().IsOpenBlind() ||
				GetInterface().IsShowItemChoiceDialog())
				bShow = false;

			if (pGameTask->GetGameTaskType() == GameTaskType::DarkLair)
			{
				// #49247 Show the exit location selection dialog only for the leader.
				if( PartyRoleEnum::LEADER == m_PartyRole )
				{
					CDnDarklairClearImp *pClearImp = static_cast<CDnDarklairClearImp *>(pGameTask->GetStageClearImp());
					if (pClearImp)
					{
						if (pClearImp->GetDungeonClearValue().bClear == false)
						{
							bShow = false;
							CDnDungeonClearMoveDlg* pDungeonClearMoveDlg = GetInterface().GetDungeonMoveDlg();
							if (pDungeonClearMoveDlg && pDungeonClearMoveDlg->IsShow() == false)
								GetInterface().ShowDungeonMoveDlg(true, false);
						}
					}
				}
			}
			GetInterface().ShowStageClearMoveDialog(bShow);
		}
		else
		{
			GetInterface().ShowStageClearMoveDialog(false);
			m_PartyState = ePartyState::NORMAL;
		}
	}
	else
	{
		GetInterface().ShowStageClearMoveDialog(false);
	}

	EtVector3 vPos = *CDnActor::s_hLocalActor->GetStaticPosition();

	std::vector<CEtWorldEventControl *> VecControl;
	CEtWorldEventControl *pControl;
	CEtWorldEventArea *pArea;
	CDnWorld::GetInstance().FindEventControlFromUniqueID( ETE_EventArea, &VecControl );
	char szTemp[8];

	CDnCommonTask *pCommonTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
	for( DWORD i=0; i<VecControl.size(); i++ ) {
		pControl = VecControl[i];

		pControl->Process( vPos, true );

		PartyStruct* pPs = GetPartyDataFromSessionID(CDnActor::s_hLocalActor->GetUniqueID());
		if (pPs && pPs->bGMTrace)
			continue;

		std::vector<CEtWorldEventArea *> VecArea;
		pControl->FindCheckAreaFromSubStr( "Gate ", &VecArea );

		bool bFarm = false;
		bool bEnteredGate = false;
		for( DWORD j=0; j<VecArea.size(); j++ ) {
			pArea = VecArea[j];

			// When in a state where entry is not possible 
			int nGateIndex = -1;
			sscanf_s( pArea->GetName(), "%s %d", szTemp, 8, &nGateIndex );

			CDnWorld::GateStruct *pStruct = CDnWorld::GetInstance().GetGateStruct( nGateIndex );
			if( !pStruct ) continue;
			CDnWorld::PermitGateEnum PermitFlag = pStruct->PermitFlag;
			if( PermitFlag == CDnWorld::PermitClose ) continue; // If Close, do not output any message at all.
			if( PermitFlag != CDnWorld::PermitEnter ) {
				tstring szCaption = GetPermitGateErrorString( nGateIndex );

				CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption2, szCaption.c_str(), textcolor::RED );
				continue;
			}
			
			// When there are monsters nearby
			if( CDnWorld::GetInstance().CanEnterGate( (char)nGateIndex ) == false )
			{
				CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption2, 100025, textcolor::RED );
				continue;
			}

			if (IsPartyEnableStartGame( (char)nGateIndex ) != ENABLE)
			{
				CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption2, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3561), textcolor::WHITE, 1.f);	// UISTRING : Cannot start the stage because a party member has 0 FTG.
				continue;
			}
			if( ( pCommonTask && pCommonTask->IsRequestNpcTalk() ) || ( CDnCamera::GetActiveCamera() && CDnCamera::GetActiveCamera()->GetCameraType() != CDnCamera::PlayerCamera ) )
				continue;

			if (CDnCashShopTask::GetInstance().IsOpenCashShop())
				continue;

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
			CDnPotentialJewelDlg* pJewelDlg = GetInterface().GetItemPotentialDlg();
			if(pJewelDlg && pJewelDlg->IsShow())
				continue;
#endif // PRE_MOD_POTENTIAL_JEWEL_RENEWAL



			bEnteredGate = true;
			if (m_nEnteredGateIndex == INVALID_GATE_INDEX && m_bRequestEnteredGate == false && !CDnActor::s_hLocalActor->IsDie() && m_bNoWarpBeforeItemSharing == false) {
				CDnGameTask* pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask("GameTask");
				if (pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm || bFarm == true)
				{
#ifdef PRE_FIX_GAMEQUIT_TO_VILLAGE
					SendAbandonStage(false, false);
#else
					SendAbandonStage(false);
#endif
					m_bRequestEnteredGate = true;
					m_nRequestEnteredGateIndex = nGateIndex;
				}
				else
				{
					m_bRequestEnteredGate = true;
					m_nRequestEnteredGateIndex = nGateIndex;
					SendRefreshGateInfo(true, vPos);
				}
#ifdef PRE_ADD_COOKING_SYSTEM
				if (CDnLifeSkillCookingTask::IsActive() &&
					(GetLifeSkillCookingTask().IsNowCooking() || GetLifeSkillCookingTask().IsRequestCooking()))
				{
					GetInterface().OpenCookingDialog(false);
					GetLifeSkillCookingTask().CancelManufacture();
				}
#endif // PRE_ADD_COOKING_SYSTEM
			}
					if( m_nEnteredGateIndex != INVALID_GATE_INDEX && CDnActor::s_hLocalActor->IsDie() ) {
				bEnteredGate = false;
			}
		}
		if( bEnteredGate == false ) {
			CDnGameTask* pGameTask = (CDnGameTask *) CTaskManager::GetInstance().GetTask( "GameTask" );
			if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm ) 
				bFarm = true;
			if( bFarm )
			{
				m_bRequestEnteredGate = false;
				m_nRequestEnteredGateIndex = -1;
			}
			else
			{
				if( m_nEnteredGateIndex != INVALID_GATE_INDEX && m_bRequestEnteredGate == false ) {
					m_bRequestEnteredGate = true;
					m_nRequestEnteredGateIndex = m_nEnteredGateIndex;
					SendRefreshGateInfo( false, vPos );
				}
			}
		}
	}
}

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
void CDnPartyTask::ClearInviteInfoList(bool bTerminate)
#else
void CDnPartyTask::ClearInviteInfoList()
#endif
{
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	if (bTerminate)
	{
		m_AcceptRequestInviteInfo.TerminateList();
		m_AcceptRequestAskInfo.TerminateList();
	}

	m_AcceptRequestInviteInfo.ClearList();
	m_AcceptRequestAskInfo.ClearList();
#else
	LIST_INVITEINFO_ITER iter = m_listInviteInfo.begin();
	for( ; iter != m_listInviteInfo.end(); ++iter )
		ReqDenyPartyInvite(*iter);

	m_listInviteInfo.clear();

	std::list<SAskInfo>::const_iterator askIter = m_listAskInfo.begin();
	for(; askIter != m_listAskInfo.end(); ++askIter)
	{
		const SAskInfo& info = *askIter;
		AnswerPartyAsking(false, info);
	}

	m_listAskInfo.clear();
#endif
}

float CDnPartyTask::GetShortestAcceptTime()
{
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	float acceptTime = m_AcceptRequestInviteInfo.GetShortestAcceptTime();
	if (acceptTime == _INVALID_TIME_VALUE)
		acceptTime = m_AcceptRequestAskInfo.GetShortestAcceptTime();

	return acceptTime;
#else
	if( !m_listInviteInfo.empty() )
	{
		return m_listInviteInfo.begin()->m_fAcceptTime;
	}
	else
	{
		if (m_listAskInfo.empty() == false)
		{
			std::list<SAskInfo>::const_iterator askIter = m_listAskInfo.begin();
			const SAskInfo& askInfo = *askIter;
			return askInfo.fAcceptTime;
		}
		//else  //???
		return _INVALID_TIME_VALUE;
	}
#endif // PRE_MOD_INTEG_SYSTEM_STATE
}

bool CDnPartyTask::IsValidGameMapIdx(int mapIdx) const
{
	return (mapIdx >= 0);
}

void CDnPartyTask::OpenAcceptRequestDialog()
{
	// Calls ClearInveinfoList when a dialog activated by the interface is closed. 
	if (GetInterface().IsOpenBlind())
	{
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
		ClearInviteInfoList(true);
#else
		ClearInviteInfoList();
#endif
		GetInterface().ClearAcceptReqDlg();
		return;
	}

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	if (m_AcceptRequestInviteInfo.IsListEmpty() == false)
	{
		const CDnPartyAcceptRequestInviteInfo& info = m_AcceptRequestInviteInfo.GetCurrentInfo();
		std::wstring str, mapName;
		if (IsValidGameMapIdx(info.m_nGameMapIdx))
		{
			DN_INTERFACE::STRING::GetMapName(mapName, info.m_nGameMapIdx);
		}
		else
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
			if( !pSox ) return;
			int nID = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_MapNameID" )->GetInteger();
			mapName = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringTableID);
		}

		str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3434), info.m_szOtherSideName.c_str(), 
			info.m_szPartyName.c_str(), mapName.c_str(),
			info.m_nCurUserCount, info.m_nMaxUserCount, info.m_nAvrLevel);	// UISTRING : <html><div align=\"center\">%s's Party Invite<br>%s<br>Summon Point : %s<br>%d/%d members       Avg Level %d</div></html>
		GetInterface().OpenAcceptRequestDialog( str.c_str(), _REQUEST_ACCEPT_TOTALTIME, info.m_fAcceptTime, true, CDnAcceptRequestDlg::eType02, ACCEPT_REQUEST_DIALOG, this );	

		m_inviteInfo = info;
	}
#else
	if (m_listInviteInfo.empty() == false)
	{
		LIST_INVITEINFO_ITER iterInner = m_listInviteInfo.begin();
		std::wstring str, mapName;
		const SInviteInfo& info = *iterInner;
		if (IsValidGameMapIdx(info.m_nGameMapIdx))
		{
			DN_INTERFACE::STRING::GetMapName(mapName, info.m_nGameMapIdx);
		}
		else
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
			if( !pSox ) return;
			int nID = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_MapNameID" )->GetInteger();
			mapName = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringTableID);
		}

		str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3434), info.m_szInviterName.c_str(), 
			info.m_szPartyName.c_str(), mapName.c_str(),
			info.m_nCurUserCount, info.m_nMaxUserCount, info.m_nAvrLevel);	// UISTRING : <html><div align=\"center\">%s's Party Invite<br>%s<br>Summon Point : %s<br>%d/%d members       Avg Level %d</div></html>
		GetInterface().OpenAcceptRequestDialog( str.c_str(), _REQUEST_ACCEPT_TOTALTIME, info.m_fAcceptTime, true, CDnAcceptRequestDlg::eType02, ACCEPT_REQUEST_DIALOG, this );	

		m_inviteInfo = *iterInner;
	}
#endif
	else
	{
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
		if (m_AcceptRequestAskInfo.IsListEmpty() == false)
		{
			const CDnPartyAcceptRequestAskInfo& askInfo = m_AcceptRequestAskInfo.GetCurrentInfo();
			if (askInfo.m_szOtherSideName.empty() == false)
			{
				std::wstring askStr;
				askStr = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3436),
					askInfo.m_szOtherSideName.c_str(),
					DN_INTERFACE::STRING::GetJobString(askInfo.m_JobId),
					askInfo.m_Level
					); // UISTRING : //<html><div align=\\\"center\\\"><br> <br>%s requests to join the party<br>Class : %s / Level : %d</div></html>
				

				// Because PartyTask is passed as the Callback here, PartyTask's callback runs later when the accept button appears. 
				CDnInterface::GetInstance().OpenAcceptRequestDialog(askStr.c_str(), _REQUEST_ACCEPT_TOTALTIME, askInfo.m_fAcceptTime, true, CDnAcceptRequestDlg::eType02, ACCEPT_REQUEST_PARTYASK_DIALOG, this);
				m_AskInfoCache = askInfo;
			}
			// Enters here when accepted!
		}
#else
		if (m_listAskInfo.empty() == false)
		{
			std::list<SAskInfo>::const_iterator askIter = m_listAskInfo.begin();
			const SAskInfo& askInfo = *askIter;
			if (askInfo.szApplicantName.empty() == false)
			{
				std::wstring askStr;
				askStr = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3436),
					askInfo.szApplicantName.c_str(),
					DN_INTERFACE::STRING::GetJobString(askInfo.jobId),
					askInfo.level
					); // UISTRING : //<html><div align=\\\"center\\\"><br> <br>%s requests to join the party<br>Class : %s / Level : %d</div></html>

				CDnInterface::GetInstance().OpenAcceptRequestDialog(askStr.c_str(), _REQUEST_ACCEPT_TOTALTIME, askInfo.fAcceptTime, true, CDnAcceptRequestDlg::eType02, ACCEPT_REQUEST_PARTYASK_DIALOG, this);
				m_AskInfoCache = askInfo;
			}
		}
#endif
	}
}

void CDnPartyTask::OnDisconnectTcp( bool bValidDisconnect )
{

}

void CDnPartyTask::OnDisconnectUdp( bool bValidDisconnect )
{
}

void CDnPartyTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	if( CDnLoadingTask::IsActive() ) {
		if( CDnLoadingTask::GetInstance().InsertLoadPacket( this, nMainCmd, nSubCmd, (void*)pData, nSize ) ) return;
	}

	bool bProcessDispatch = false;
	switch( nMainCmd ) {
		case SC_PARTY: OnRecvPartyMessage( nSubCmd, pData, nSize ), bProcessDispatch = true; break;
		case SC_ROOM: OnRecvRoomMessage( nSubCmd, pData, nSize ), bProcessDispatch = true; break;
		case SC_SYSTEM: OnRecvSystemMessage( nSubCmd, pData, nSize ), bProcessDispatch = true; break;
		case SC_CHAR: OnRecvCharMessage( nSubCmd, pData, nSize ), bProcessDispatch = true; break;
		case SC_CUSTOMEVENTUI: OnRecvPlayerCustomEventUI( nSubCmd, pData, nSize ), bProcessDispatch = true; break;
	}
	if( bProcessDispatch ) {
		WriteLog( 1, ", Info, CDnPartyTask::OnDispatchMessage End : nMainCmd(%d), nSubCmd(%d), nSize(%d)\n", nMainCmd, nSubCmd, nSize );
	}
}

void CDnPartyTask::OnRecvPartyMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eParty::SC_CREATEPARTY: OnRecvPartyCreateParty( (SCCreateParty *)pData ) ; break;
		case eParty::SC_JOINPARTY: OnRecvPartyJoinParty( (SCJoinParty *)pData ); break;
		case eParty::SC_REFRESHPARTY: OnRecvPartyRefresh( (SCRefreshParty *)pData ); break;
		case eParty::SC_PARTYOUT: OnRecvPartyOutParty( (SCPartyOut *)pData ); break;
		case eParty::SC_REFRESHGATEINFO: OnRecvPartyRefreshGateInfo( (SCRefreshGateInfo *)pData ); break;
		case eParty::SC_CHANGEPARTYLEADER: OnRecvPartyChangeLeader( pData ); break;
		case eParty::SC_PARTYINVITE: OnRecvPartyInviteMsg(pData); break;
		case eParty::SC_PARTYINVITEFAIL: OnRecvPartyInviteFail(pData); break;
		case eParty::SC_PARTYINVITEDENIED: OnRecvPartyInviteDenied((SCPartyInviteDenied*)pData); break;
		case eParty::SC_GATEINFO: OnRecvPartyReadyGate( (SCGateInfo *)pData ); break;
		case eParty::SC_STARTSTAGE: OnRecvPartyStageStart( (SCStartStage *)pData ); break;
		case eParty::SC_STARTSTAGEDIRECT: OnRecvPartyStageStartDirect( (SCStartStageDirect *)pData ); break;
		case eParty::SC_CANCELSTAGE: OnRecvPartyStageCancel( (SCCancelStage *)pData ); break;
		case eParty::SC_STARTVILLAGE:OnRecvPartyVillageStart( pData ); break;

		case eParty::SC_PARTYMEMBERKICKED: OnRecvPartyMemberKick((SCPartyKickedMember*)pData); break;
		case eParty::SC_PARTYLEADERSWAP: OnRecvPartyLeaderSwap( (SCSwapPartyLeader*)pData ); break;
		case eParty::SC_PARTYMODIFY: OnRecvPartyModifyResult((SCPartyInfoModify*)pData); break;
		case eParty::SC_VERIFYJOINPARTY: OnRecvVerifyJoinParty((SCVerifyJoinParty*)pData); break;

		case eParty::SC_EQUIPDATA: OnRecvPartyEquipData( (SCEquipData *)pData ); break;
		case eParty::SC_EQUIPCASHDATA: OnRecvPartyCashEquipData( (SCCashEquipData *)pData ); break;
		case eParty::SC_SKILLDATA: OnRecvPartySkillData( (SCSkillData *)pData ); break;
		case eParty::SC_DEFAULTPARTSDATA: OnRecvPartyDefaultPartsData( (SCDefaultPartsData *)pData ); break;
		case eParty::SC_ETCDATA: OnRecvPartyEtcData( (SCEtcData *)pData ); break;
		case eParty::SC_WEAPONORDERDATA: OnRecvPartyWeaponOrderData( (SCWeaponOrderData*)pData ); break;
		case eParty::SC_GLYPHDATA: OnRecvPartyGlyphEquipData( (SCGlyphEquipData* )pData ); break;
		
		case eParty::SC_VEHICLEEQUIPDATA: OnRecvPartyVehicleEquipData( (SCVehicleEquipData* )pData ); break;
		case eParty::SC_PETEQUIPDATA: OnRecvPartyPetEquipData( (SCVehicleEquipData* )pData ); break;

		case eParty::SC_GUILDDATA: OnRecvPartyGuildData( (TPartyMemberGuild*)pData ); break;
		case eParty::SC_REQUEST_JOINGETREVERSIONITEM:	OnRecvPartyJoinGetReversionItem((SCPartyJoinGetReversionItemInfo*)pData); break;
		case eParty::SC_NOTIFY_ROLLGETREVERSIONITEM:	OnRecvPartyRollGetReversionItem((SCRollDiceForGetReverseItem*)pData); break;
		case eParty::SC_NOTIFY_RESULTGETREVERSIONITEM:	OnRecvPartyResultGetReversionItem((SCResultGetReverseItem*)pData); break;

		case eParty::SC_PARTYREQUEST: OnRecvPartyReadyRequest(); break;
		case eParty::SC_ABSENT: OnRecvPartyMemberAbsent((SCMemberAbsent*)pData); break;
		case eParty::SC_VERIFYTARGETDUNGEON: OnRecvVerifyDungeonRequest((SCVerifyDungeon*)pData); break;
		case eParty::SC_REFRESHREBIRTHCOIN: OnRecvPartyRefreshRebirthCoin((SCRefreshRebirthCoin*)pData); break;
		case eParty::SC_PARTYBONUSVALUE: OnRecvPartyKeepBonus( (SCPartyBonusValue *)pData ); break;
		case eParty::SC_PARTYINVITENOTICE: OnRecvPartyInviteNotice((SCPartyInviteNotice*)pData); break;
		case eParty::SC_ASKJOINPARTYRESULT: OnRecvPartyAskJoinResult((SCPartyAskJoinResult*)pData); break;
		case eParty::SC_ASKJOINPARTYTOLEADER: OnRecvPartyAskJoinToLeader((SCPartyAskJoinToLeader*)pData); break;
		case eParty::SC_ASKJOINPARTYDECISIONINFO: OnRecvPartyAskJoinDecisionInfo((SCPartyAskJoinDecisionInfo*)pData); break;
#if defined( PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP )
		case eParty::SC_PARTYINFO: OnRecvPartyAskInfo((SCPartyInfo*)pData); break;
#endif 
		case eParty::SC_PARTYUPDATEFIELDUI: OnRecvPartyUpdateFieldUI((SCPartyUpdateFieldUI*)pData); break;
		case eParty::SC_PARTYMEMBER_PART: OnRecvPartyMemberPart((SCPartyMemberPart *)pData); break;
		case eParty::SC_SWAPMEMBERINDEX: OnRecvPartySwapMemberIndex((SCPartySwapMemberIndex*)pData); break;
		case eParty::SC_CHECK_LASTDUNGEONINFO: OnRecvPartyLastDugeonInfo((SCPartyCheckLastDungeonInfo*)pData); break;
		case eParty::SC_CONFIRM_LASTDUNGEONINFO: OnRecvPartyConfirmLastDungeonInfo((SCPartyConfirmLastDungeonInfo*)pData); break;
		case eParty::SC_SELECTSTAGE: OnRecvSelectStage( (SCSelectStage *)pData ); break;
	}
}

void CDnPartyTask::OnRecvRoomMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) 
	{
		case eRoom::SC_SYNC_WAIT:				OnRecvRoomSyncWait( (SCSyncWait*)pData );							break;
		case eRoom::SC_SYNC_START:				OnRecvRoomSyncStart( (SCSyncStart*)pData );							break;
		case eRoom::SC_REFRESH_MEMBER:			OnRecvRefreshMember((SCRefreshMember*)pData);						break;
		case eRoom::SC_SYNC_MEMBERINFO:			OnRecvRoomMemberInfo( (SCROOM_SYNC_MEMBERINFO*)pData );				break;
		case eRoom::SC_SYNC_MEMBERTEAM:			OnRecvRoomMemberTeam( (SCROOM_SYNC_MEMBERTEAM*)pData );				break;
		case eRoom::SC_SYNC_MEMBER_BREAKINTO:	OnRecvRoomMemberBreakInto( (SCROOM_SYNC_MEMBER_BREAKINTO*)pData, nSize );	break;
		case eRoom::SC_SYNC_MEMBERHPSP:			OnRecvRoomMemberHPSP( (SCROOM_SYNC_MEMBERHPSP*)pData );				break;
		case eRoom::SC_SYNC_MEMBERBATTLEMODE:	OnRecvRoomMemberBattleMode( (SCROOM_SYNC_MEMBERBATTLEMODE*)pData );	break;
		case eRoom::SC_SYNC_DROPITEMLIST:		OnRecvRoomDropItemList( (SCROOM_SYNC_DROPITEMLIST*)pData );			break;
	}
}

void CDnPartyTask::OnRecvSystemMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eSystem::SC_VILLAGEINFO: OnRecvSystemVillageInfo( (SCVillageInfo*)pData ); break;
		case eSystem::SC_COUNTDOWNMSG: OnRecvSystemCountDown((SCCountDownMsg*)pData); break;
	}
}

void CDnPartyTask::OnRecvCharMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eChar::SC_ENTER:	OnRecvCharEntered( (SCEnter *)pData );
	}
}


void CDnPartyTask::OnRecvSystemVillageInfo( SCVillageInfo *pPacket )
{
	if( pPacket->nRet != ERROR_NONE )
	{
		EnableWaitGateState(false);
		//m_bWaitGate = false;
	}
}

void CDnPartyTask::OnRecvSystemCountDown(SCCountDownMsg * pPacket)
{
	if (pPacket->cIsStart > 0)
		api_trigger_BeginCountDownEx(pPacket->nCountTime, pPacket->nCode, pPacket->msgboxType);
	else
		api_trigger_EndCountDown();
}

void CDnPartyTask::OnRecvPartyCreateParty( SCCreateParty *pPacket )
{ 
	FUNC_LOG();

	if (pPacket == NULL)
	{
		_ASSERT(0);
		return;
	}

	switch( pPacket->nRetCode )
	{
	case ERROR_NONE:
		{
			m_PartyRole = PartyRoleEnum::LEADER;
			m_PartyID = pPacket->PartyID;
			m_VecPartyList.clear();
			m_bSingleToPartyByGMTrace = false;
			SetPartyType(pPacket->PartyType);

			if (pPacket->cIsAutoCreate == 1)
			{
				std::wstring str;
				str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3565), CDnActor::s_hLocalActor->GetName()); // UISTRING : %s's party has been automatically created.
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), str.c_str(), false);
			}

			CEtSoundEngine::GetInstance().PlaySound("2D", m_PartySoundIndex[CREATE]);
#ifdef PRE_ADD_AUTO_DICE
			GetInterface().GetMainBarDialog()->UpateAutoDiceOption(true);
#endif
#ifdef PRE_MOD_SYSTEM_STATE
			bool bStartState = GetInterface().GetSystemStateMgr().StartState(eSTATE_PARTY);
#endif

#ifdef PRE_ADD_CHAT_RENEWAL
			
			// Change chat mode.
			if( CGameOption::GetInstance().cPartyChatAcceptable == 1 )
			{
				if( GetInterface().GetChatDialog()->GetChatMode() != CHAT_PARTY )
				{
					GetInterface().GetChatDialog()->ClearText();
					GetInterface().GetChatDialog()->SetChatMode( CHAT_PARTY, true );
				}
			}
#endif

		}
		break;
	default:
		{
			InitParty();
			GetInterface().ServerMessageBox(pPacket->nRetCode);
		}
		break;
	}
}

void CDnPartyTask::OnPartyMemberNewAdded(ePartyRefreshSubject subject, const PartyStruct& memberInfo)
{
	if (IsPartyInfoEmpty() == false)
	{
		//Do not notify when returning back to village
		if (subject != PARTYREFRESH_RETURNVILLAGE)
		{
			CEtSoundEngine::GetInstance().PlaySound("2D", m_PartySoundIndex[ENTER]);
			if (memberInfo.bGMTrace == false)
			{
				std::wstring str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3552), memberInfo.wszCharacterName); // UISTRING : %s joined the party.
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), str.c_str(), false);
			}

			GetInterface().CloseCaptionDialog(CDnInterface::typeCaption5);
			m_bPartyMemberInviting = false;
			m_bPartyAcceptRequesting = false;
		}
	}

	if (memberInfo.wFatigue <= 0)
	{
		if (m_PartyRole == PartyRoleEnum::LEADER)
		{
			TCHAR szStr[256] = {0};
			swprintf_s( szStr, 256,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3568 ), memberInfo.wszCharacterName); // UISTRING : "%s" has run out of fatigue and cannot enter the stage.

			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), szStr, false);
		}
	}
}

static bool ComparePartyMemberIdx(const CDnPartyTask::PartyStruct& s1, const CDnPartyTask::PartyStruct& s2)
{
	return (s1.cMemberIndex < s2.cMemberIndex);
}

void CDnPartyTask::OnRecvPartyRefresh( SCRefreshParty *pPacket )
{
	FUNC_LOG();

	if( m_PartyRole != PartyRoleEnum::LEADER && m_PartyRole != PartyRoleEnum::MEMBER ) 
		_ASSERT(0);

	if (IsPartyInfoEmpty() == false)
	{
#ifdef PRE_PARTY_DB
		if (IsPartyInfoModified(pPacket->cPartyMemberMax, pPacket->cUserLvLimitMin, pPacket->wszName, pPacket->iPassword, pPacket->nTargetMapIdx, pPacket->Difficulty))
#else
		if (IsPartyInfoModified(pPacket->cPartyMemberMax, pPacket->cUserLvLimitMin, pPacket->cUserLvLimitMax, pPacket->wszName, pPacket->wszPass, pPacket->nTargetMapIdx, pPacket->Difficulty))
#endif
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3554), false);	// UISTRING : Party information has changed.

		bool bLootRuleChanged = false;
		if (IsPartyLootRuleModified((ePartyItemLootRule)pPacket->ItemLootRule))
		{
#ifdef PRE_PARTY_DB
			tstring prefix = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3475);	// UISTRING : Item loot mode changed :
#else
			tstring prefix = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3542);	// UISTRING : Change Distribution Rule
#endif
			tstring msg[ITEMLOOTRULE_MAX];
			msg[ITEMLOOTRULE_NONE]		= _T("[ERROR]");
			msg[ITEMLOOTRULE_LEADER]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3543);	// UISTRING : The party leader receives the item.
			msg[ITEMLOOTRULE_INORDER]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3544);	// UISTRING : Party members take turns receiving items.
			msg[ITEMLOOTRULE_RANDOM]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3545);	// UISTRING : Acquired items are distributed randomly.
			msg[ITEMLOOTRULE_OWNER]		= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3546);	// UISTRING : Keep acquired items for yourself.

			WCHAR str[128] = {0,};
			wsprintf(str, _T("%s%s"), prefix.c_str(), msg[pPacket->ItemLootRule].c_str());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), str);//, false);
			bLootRuleChanged = true;
		}

		//	Note : Check whether to notify on rank change as well 
		if (IsPartyLootRankModified((eItemRank)pPacket->ItemLootRank))
		{
#ifdef PRE_PARTY_DB
			tstring prefix = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3474);// UISTRING : Dice roll rule changed :
#else
			tstring prefix = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3402);// UISTRING : Random Item Distribution Rule Changed : 
#endif
			tstring msg[ITEMRANK_MAX + 2];

			msg[ITEMRANK_D]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3570);
			msg[ITEMRANK_C]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3571);
			msg[ITEMRANK_B]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3572);
			msg[ITEMRANK_A]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3573);
			msg[ITEMRANK_S]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3574);
#ifdef PRE_PARTY_DB
			msg[ITEMRANK_SS]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3457);
			msg[ITEMRANK_NONE]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3458); // UISTRING : Do not roll dice
#else
			msg[ITEMRANK_NONE]	= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3575);
#endif

			WCHAR str[128] = {0,};
			wsprintf(str, _T("%s%s"), prefix.c_str(), msg[pPacket->ItemLootRank].c_str());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), str);//, false);

			bLootRuleChanged = true;
		}

#ifdef PRE_PARTY_DB
		bool bAllowWorldZoneMapListChanged = false;
		bool bTempAllowWorldZoneMapList = ((pPacket->iBitFlag & Party::BitFlag::WorldmapAllow) == 0) ? false : true;
		if (IsPartyAllowWorldZoneMapListModified(bTempAllowWorldZoneMapList))
		{
			std::wstring str = FormatW(L"%s", bTempAllowWorldZoneMapList ? GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3471) : GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3472)); // UISTRING : Recruit party members in World Zone as well. / Do not recruit party members in World Zone.
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str());

			bAllowWorldZoneMapListChanged = true;
		}
#endif

		bool bJobDiceChanged = false;
#ifdef PRE_PARTY_DB
		bool bIsJobDiceFromPacket = (pPacket->iBitFlag & Party::BitFlag::JobDice);
#else
		bool bIsJobDiceFromPacket = (pPacket->cIsJobDice == 0) ? false : true;
#endif
		if (bIsJobDiceFromPacket != m_bJobDice)
		{
			int uistringNum = bIsJobDiceFromPacket ? 3408 : 3409;
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, uistringNum), true);
		}

		if (bLootRuleChanged || bJobDiceChanged)
			CEtSoundEngine::GetInstance().PlaySound("2D", m_PartySoundIndex[LOOTCHANGE]);

		bool bModifySpecifier[2];
		bModifySpecifier[0] = IsPartyTargetStageIdxModified(pPacket->nTargetMapIdx);
		bModifySpecifier[1] = IsPartyTargetStageDifficultyModified(pPacket->Difficulty);

		if (bModifySpecifier[0] || bModifySpecifier[1])
		{
			std::wstring prefix, main, map, difficulty;
			if (bModifySpecifier[0])
			{
				prefix = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3576); // UISTRING : Party target area changed : 
				DN_INTERFACE::STRING::GetMapName(map, pPacket->nTargetMapIdx);
				main = map.empty() ? GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 93) : map;	// UISTRING : None
			}

			if (bModifySpecifier[1])
			{
				prefix = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3400); // UISTRING : Party target difficulty changed : 
				DN_INTERFACE::STRING::GetStageDifficultyText(difficulty, pPacket->Difficulty);
				main = difficulty.empty() ? GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3599) : difficulty;	// UISTRING : Unlimited
			}

			if (bModifySpecifier[0] && bModifySpecifier[1])
			{
				prefix = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3401); // UISTRING : Party target area and difficulty changed : 
				main = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3589), map.empty() ? GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 93) : 
					map.c_str(), difficulty.empty() ? GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3599) : difficulty.c_str()); // UISTRING : None / Unlimited
			}

			main = FormatW(L"%s%s", prefix.c_str(), main.c_str());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), main.c_str());
		}
	}

	OutputDebug( "OnRecvPartyRefresh\n" );
	std::vector<PartyStruct> VecPrevList = m_VecPartyList;
	m_VecPartyList.clear();

	m_nLocalActorPartyIndex = -1;

	for( DWORD i=0; i<pPacket->cCurrentUserCount; i++ ) 
	{
		PartyStruct Struct;
		Struct.Assign( pPacket->Member[i] );

		// Keep existing data and restore GateIndex, Sync values, etc.
		//
		bool bNewAdded = true;
		for( DWORD j=0; j<VecPrevList.size(); j++ )
		{
			if( Struct.nSessionID == VecPrevList[j].nSessionID )
			{
				Struct.nEnteredGateIndex = VecPrevList[j].nEnteredGateIndex;
				Struct.bSync = VecPrevList[j].bSync;
				Struct.hActor = VecPrevList[j].hActor;
				Struct.vPos = VecPrevList[j].vPos;
				Struct.bGMTrace = VecPrevList[j].bGMTrace;
				bNewAdded = false;
				break;
			}
		}

		bool bMe = (pPacket->Member[i].nSessionID == CDnBridgeTask::GetInstance().GetSessionID());

		if (bNewAdded)
		{
			if (bMe == false)
			{
				OnPartyMemberNewAdded((ePartyRefreshSubject)pPacket->cPartyRefreshSubJect, Struct);
			}

			bool bBreakInto = false;
			{
				ScopeLock<CSyncLock> Lock( m_BreakIntoPartyListLock );
				for( std::list<PartyStruct>::iterator itor=m_BreakIntoPartyList.begin() ; itor!=m_BreakIntoPartyList.end() ; ++itor )
				{
					if( (*itor).nSessionID == pPacket->Member[i].nSessionID ) 
					{
						(*itor).Assign( pPacket->Member[i] );
						(*itor).bSync = true;

						if( CDnBridgeTask::IsActive() && CDnBridgeTask::GetInstance().GetGameTaskType() != GameTaskType::PvP )
							(*itor).bCompleteBreakInto = true;
						bBreakInto = true;
						break;
					}
				}
			}
			if( bBreakInto ) continue;

		}

		if( Struct.nSessionID == pPacket->nLeaderSessionID )
		{
			Struct.bPartyMaster = true;
		}
		else
		{
			Struct.bPartyMaster = false;
		}


		Struct.bCompleteBreakInto = true;
		m_VecPartyList.push_back( Struct );
	}

	//std::sort(m_VecPartyList.begin(), m_VecPartyList.end(), ComparePartyMemberIdx);

	for (int k = 0; k < (int)m_VecPartyList.size(); ++k)
	{
		const PartyStruct& data = m_VecPartyList[k];
		if (data.nSessionID == CDnBridgeTask::GetInstance().GetSessionID())
		{
			m_nLocalActorPartyIndex = k;
			break;
		}
	}

	if( pPacket->nLeaderSessionID == CDnBridgeTask::GetInstance().GetSessionID() )
	{
		m_PartyRole = PartyRoleEnum::LEADER;
	}
	else
	{
		m_PartyRole = PartyRoleEnum::MEMBER;
	}

#ifdef PRE_PARTY_DB
	int password = Party::Constants::INVALID_PASSWORD;
	if (pPacket->iBitFlag & Party::BitFlag::Password)
		password = pPacket->iPassword;
	SetPartyPassword(password);
#else
	SetPartyPassword( pPacket->wszPass );
#endif
	m_nMaxPartyMemberCount = pPacket->cPartyMemberMax;
	m_nMinUserLevel = pPacket->cUserLvLimitMin;
#if defined( PRE_PARTY_DB )
#else
	m_nMaxUserLevel = pPacket->cUserLvLimitMax;
#endif // #if defined( PRE_PARTY_DB )
	m_szPartyName = pPacket->wszName;
#ifdef PRE_PARTY_DB
	m_bAllowWorldZoneMapList = ((pPacket->iBitFlag & Party::BitFlag::WorldmapAllow) == 0) ? false : true;
#else
	m_szPartyPassword = pPacket->wszPass;
#endif
	m_emPartyRouletteLevel = (ePartyItemLootRule)pPacket->ItemLootRule;
	m_emPartyRouletteItemRank = (eItemRank)pPacket->ItemLootRank;

	m_nTargetStageIdx = pPacket->nTargetMapIdx;
#ifdef PRE_PARTY_DB
	m_TargetStageDifficulty = pPacket->Difficulty;
	m_bJobDice = (pPacket->iBitFlag & Party::BitFlag::JobDice);
	SetPartyType(pPacket->PartyType);
#else
	m_nTargetStageDifficulty = pPacket->Difficulty;
	m_bJobDice = (pPacket->cIsJobDice == 0) ? false : true;
	SetPartyType((ePartyType)pPacket->cPartyType);
#endif

	int nMasterIndex = ChangePartyMaster( pPacket->nLeaderSessionID );
	GetInterface().SetPartyInfoLeader( nMasterIndex );

#ifdef PRE_WORLDCOMBINE_PARTY
	m_nCurWorldCombinePartyTableIndex = pPacket->nWorldCombinePartyTableIndex;
#endif

	//BYTE cCurrentUserCount;
	//BYTE cPartyRefreshSubJect; // Refer to ePartyRefreshSubject

	switch( pPacket->cPartyRefreshSubJect )
	{
	case PARTYREFRESH_NONE:
		break;
	case PARTYREFRESH_CHANGE_LEADER:
		break;
	case PARTYREFRESH_MODIFY_INFO:
		break;
	default:
		break;
	}

	GetInterface().SwapPartyDialog();

	CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
	if( pMainMenuDlg ) pMainMenuDlg->RefreshZoneMapPartyActor();
}

#ifdef PRE_FIX_PARTYINDEX_DUMP
void CDnPartyTask::SetLocalActorPartyIndex()
{
	int i = 0;
	for (; i < (int)m_VecPartyList.size(); ++i)
	{
		const PartyStruct& data = m_VecPartyList[i];
		if (data.nSessionID == CDnBridgeTask::GetInstance().GetSessionID())
		{
			m_nLocalActorPartyIndex = i;
			return;
		}
	}

	m_nLocalActorPartyIndex = -1;
}
#endif

void CDnPartyTask::OnRecvPartySwapMemberIndex(SCPartySwapMemberIndex* pPacket)
{
	FUNC_LOG();

	switch( pPacket->nRetCode )
	{
	case ERROR_NONE:
		{
			if (pPacket->cCount <= 0)
			{
				_ASSERT(0);
				return;
			}
			int i = 0;
			for (; i < pPacket->cCount; ++i)
			{
				PartyStruct* pData = GetPartyDataFromSessionID(pPacket->Index[i].nSessionID);
				if (pData == NULL)
					continue;
				pData->cMemberIndex = pPacket->Index[i].cIndex;
			}

			//std::sort(m_VecPartyList.begin(), m_VecPartyList.end(), ComparePartyMemberIdx);

			for (i = 0; i < (int)m_VecPartyList.size(); ++i)
			{
				const PartyStruct& data = m_VecPartyList[i];
				if (data.nSessionID == CDnBridgeTask::GetInstance().GetSessionID())
				{
					m_nLocalActorPartyIndex = i;
					break;
				}
			}

			GetInterface().RefreshPartyInfoList();
			GetInterface().RefreshPartyGuage();
			GetInterface().RefreshPartyGateInfo();
		}
		break;
	default:
		_ASSERT(0);
		break;
	}
}

void CDnPartyTask::OnRecvPartyLastDugeonInfo(SCPartyCheckLastDungeonInfo* pPacket)
{
	FUNC_LOG();

	if (pPacket->wszPartyName && pPacket->wszPartyName[0] != '\0')
	{
		std::wstring partyNameString = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3447), pPacket->wszPartyName); // UISTRING : Move to the previous party?
		m_PartyReconnectCheckCounter = 0;

		if (CDnActor::s_hLocalActor)
		{
			CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
			pLocalActor->LockInput(true);
		}
		GetInterface().CloseMessageBox();
		GetInterface().MessageBox(partyNameString.c_str(), MB_YESNO, MESSAGEBOX_PARTY_RECONN_LAST_DUNGEON, this);
	}
}

#define RECONNECT_PARTY_HEARTBEAT_SEC 5.f
void CDnPartyTask::OnRecvPartyConfirmLastDungeonInfo(SCPartyConfirmLastDungeonInfo* pPacket)
{
	FUNC_LOG();

	if (pPacket->iRet == ERROR_CANT_RESTORE_PARTY_STATE)
	{
		GetInterface().ProgressSimpleMsgBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3450), RECONNECT_PARTY_HEARTBEAT_SEC, true, UI_DISABLE_DIALOG, this); // UISTRING : Reconnecting to party...
	}
	else if (pPacket->iRet == ERROR_NONE)
	{
		GetInterface().CloseProgressSimpleMsgBox();
	}
	else
	{
		if (CDnActor::s_hLocalActor)
		{
			CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
			pLocalActor->LockInput(false);
		}

		GetInterface().CloseProgressSimpleMsgBox();
		GetInterface().ServerMessageBox(pPacket->iRet);
	}
}

void CDnPartyTask::OnRecvSelectStage( SCSelectStage * pPacket )
{
	FUNC_LOG();

	CDnBridgeTask::GetInstance().SetGateSelectIndex( pPacket->cSelectMapIndex );

	if( GetPartyTask().GetPartyRole() == CDnPartyTask::MEMBER )
		GetInterface().SetWorldZoneSelectIndex( pPacket->cSelectMapIndex );
}

void CDnPartyTask::OnRecvPartyJoinParty( SCJoinParty *pPacket )
{
	FUNC_LOG();

	switch( pPacket->nRetCode )
	{
	case ERROR_NONE:
		{
			m_PartyRole = PartyRoleEnum::MEMBER;
			m_PartyID = pPacket->PartyID;
			m_PartyState = ePartyState::NORMAL;
			m_bSingleToPartyByGMTrace = false;	// May need to set to false when returning to town and joining a party after being traced...

			std::wstring str = FormatW(_T("%s(%s)"), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3553), pPacket->wszPartyName);	// UISTRING : Joined the party.
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), str.c_str(), false);

			GetInterface().OnPartyJoin();
			CInputDevice::GetInstance().ResetAbsenceCheckTimer(CInputDevice::eABSENCE_PARTY);
			CEtSoundEngine::GetInstance().PlaySound("2D", m_PartySoundIndex[ENTER]);

			// Add exception handling.
			if( GetInterface().GetChatRoomDlg() && GetInterface().GetChatRoomDlg()->IsShow() )
			{
				SendLeaveChatRoom();
				GetChatRoomTask().ForceCloseChatRoom();
			}

			m_PartyProcessState &= ~PPS_ACCEPT_INVITE;

#ifdef PRE_ADD_AUTO_DICE
			GetInterface().GetMainBarDialog()->UpateAutoDiceOption(true);
#endif

#ifdef PRE_ADD_CHAT_RENEWAL
			// Change chat mode.
			if( CGameOption::GetInstance().cPartyChatAcceptable == 1 )
			{
				if( GetInterface().GetChatDialog()->GetChatMode() != CHAT_PARTY )
				{
					GetInterface().GetChatDialog()->ClearText();
					GetInterface().GetChatDialog()->SetChatMode( CHAT_PARTY, true );
				}
			}
#endif

		}
		break;
	default:
		{
			if (IsPartyProcessState(CDnPartyTask::PPS_ACCEPT_INVITE))
				m_PartyProcessState &= ~PPS_ACCEPT_INVITE;

			GetInterface().ServerMessageBox(pPacket->nRetCode);
		}
		break;
	}

	LockPartyReqPartyMemberInfo(false);
}

void CDnPartyTask::OnPartyMemberOut(UINT betrayerSessionID, char kickKind)
{
	std::wstring temp;

	PartyStruct *pPartyStruct = GetPartyDataFromSessionID( betrayerSessionID, true );
	bool bMe = (betrayerSessionID == CDnBridgeTask::GetInstance().GetSessionID());
	if( !pPartyStruct )
	{
		CDebugSet::ToLogFile( "CDnPartyTask::OnRecvPartyOutParty, Party member does not exist." );
		return;
	}

	DnDebugConsole::LogW( (betrayerSessionID >= 0x70000000) ? L"BOT" : L"PARTY",
		L"MemberOut %s SessionID=%u Name=%s KickKind=%d Remaining=%u",
		(betrayerSessionID >= 0x70000000) ? L"[BOT]" : L"[USER]",
		betrayerSessionID, pPartyStruct->wszCharacterName, (int)kickKind, (UINT)GetPartyCount() );

	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );

	switch( kickKind )
	{
	case 0: // Normally kicked
		{
			if (bMe)
			{
				temp = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3563 );	// UISTRING : Kicked from the party.
			}
			else
			{
				temp = FormatW(_T("%s%s"), pPartyStruct->wszCharacterName, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3527 ));	// UISTRING : has been kicked from the party.
				if( !pPartyStruct->bGMTrace )
					CEtSoundEngine::GetInstance().PlaySound("2D", m_PartySoundIndex[KICK]);
			}
		}
		break;
	case 1: // Permanently banned
		{
			if (bMe)
			{
				temp = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3564 );	// UISTRING : Permanently kicked from the party. You can no longer join this party.
			}
			else
			{
				temp = FormatW(_T("%s%s"), pPartyStruct->wszCharacterName, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3528 ));	// UISTRING : has been permanently banned from the party. Cannot rejoin the same party.
				if( !pPartyStruct->bGMTrace )
					CEtSoundEngine::GetInstance().PlaySound("2D", m_PartySoundIndex[KICK]);
			}
		}
		break;
	default:
		{
			if (bMe)
			{
				if (GetPartyCount() > 1)
					temp = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3551); // UISTRING : Left the party.
				else
					temp = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3555); // UISTRING : The party has been disbanded.
			}
			else
			{
				temp = FormatW(_T("%s%s"), pPartyStruct->wszCharacterName, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3529 ));	// UISTRING : has left the party.
				if( !pPartyStruct->bGMTrace && (!pTask || pTask->GetGameTaskType() != GameTaskType::Farm) )
					CEtSoundEngine::GetInstance().PlaySound("2D", m_PartySoundIndex[KICK]);
			}
		}
		break;
	}

	bool bAddChatMsg = false;
	if( !pTask || (pTask->GetGameTaskType() != GameTaskType::PvP && pTask->GetGameTaskType() != GameTaskType::Farm) ) bAddChatMsg = true;
	if( pPartyStruct->bGMTrace || betrayerSessionID >= 0x70000000 ) bAddChatMsg = false;

	if( bAddChatMsg )
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", temp.c_str(), false );

	// Reset master/apprentice icon when left alone in party.
	if( 2 == GetPartyCount() )
		GetInterface().ResetKeepMasterInfo();

	if (bMe)
	{
		InitParty();

		GetInterface().SwapPartyDialog();
		GetInterface().RefreshPartyInfoList();
		GetInterface().RefreshPartyGuage();
		GetInterface().HideVoiceButtonMode();

		// If chat mode was party, change it to normal.
		if( GetInterface().GetChatDialog()->GetChatMode() == CHAT_PARTY )
			GetInterface().GetChatDialog()->SetChatMode( CHAT_NORMAL );

		CEtSoundEngine::GetInstance().PlaySound("2D", m_PartySoundIndex[EXILE]);
	}

	
	if (pTask && bMe == false)
	{
		for (DWORD i=0; i<GetPartyCount(); i++) 
		{
			if( GetPartyData(i)->nSessionID == betrayerSessionID ) 
			{
				DnActorHandle hActor = GetPartyData(i)->hActor;
				if( hActor ) {
					if( GetPartyData(i)->bGMTrace ) {
						if( m_bSingleToPartyByGMTrace ) {
							if( m_PartyRole == LEADER && m_PrevPartyRole == SINGLE ) {
								m_bSingleToPartyByGMTrace = false;
								m_PartyRole = SINGLE;
							}
						}
					}
					else {
						DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
						if( hHandle ) {
							hHandle->SetPosition( *hActor->GetPosition() );
							hHandle->SetActionQueue( "Logout" );
						}
					}

					pTask->ProcessExitUser( hActor );
				}

				if( pTask && pTask->GetGameTaskType() == GameTaskType::PvP )
				{
					if(!m_VecPartyList.empty() && GetPartyData(i) && !GetPartyData(i)->bGMTrace && GetPartyData(i)->usTeam == PvPCommon::Team::Observer)
					{
						if( !CDnBridgeTask::GetInstance().IsPvPGameMode( PvPCommon::GameMode::PvP_GuildWar ) )
						{
							WCHAR wszMessage[256];
							SecureZeroMemory(wszMessage,sizeof(wszMessage));

							wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 121086 ),GetPartyData(i)->wszCharacterName);
							GetInterface().AddMessageText( wszMessage , textcolor::WHITE );
							GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", wszMessage);
						}
					}

				} // Observers do not create actors, so handle enter/exit here (must use PartyTask)

				m_VecPartyList.erase( m_VecPartyList.begin() + i );
				break;
			}
		}

		if( betrayerSessionID >= 0x70000000 && m_VecPartyList.size() == 1 && m_VecPartyList[0].nSessionID == CDnBridgeTask::GetInstance().GetSessionID() &&
			pTask && pTask->GetGameTaskType() != GameTaskType::PvP && m_PartyRole == PartyRoleEnum::LEADER )
		{
			m_PrevPartyRole = m_PartyRole;
			m_PartyRole = PartyRoleEnum::SINGLE;
		}

		for( DWORD i=0; i < (int)m_VecPartyList.size(); i++ )
		{
			const PartyStruct& data = m_VecPartyList[i];
			if (!data.hActor)
				continue;

			if (data.hActor == CDnActor::s_hLocalActor)
			{
				m_nLocalActorPartyIndex = i;
				break;
			}
		}

		pTask->RefreshConnectUser();

		GetInterface().RefreshPartyInfoList();
		GetInterface().RefreshPartyGuage();
#ifdef PRE_ADD_IMPROVE_MASTER_OBSERVER
		GetInterface().RefreshPvpPlayerGauge();
#endif
	}
}

void CDnPartyTask::OnRecvPartyOutParty( SCPartyOut *pPacket )
{
	FUNC_LOG();

	switch( pPacket->nRetCode )
	{
	case ERROR_NONE: // Normal exit, kicked
		{
			OnPartyMemberOut(pPacket->nSessionID, pPacket->cKickKind);

#ifdef PRE_MOD_SYSTEM_STATE
			GetInterface().GetSystemStateMgr().EndState(eSTATE_PARTY);
#endif
		}
		break;
	default:
		GetInterface().ServerMessageBox(pPacket->nRetCode);
		break;
	}
}

void CDnPartyTask::OnCancelStage()
{
	CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	int reqGateIdx = GetRequestEnteredGateIndex();
	if (reqGateIdx != -1)
	{
		const CDnWorld::GateStruct *pGate = CDnWorld::GetInstance().GetGateStruct(reqGateIdx);
		if (pGate)
		{
			//	Get StartPosition Center
			EtVector3 startPosCenter;
			char szStartName[32];
			sprintf_s( szStartName, "StartPosition %d", reqGateIdx );

			if (CDnWorld::IsActive() == false)
				return;

			for( DWORD i=0 ; i< CDnWorld::GetInstance().GetGrid()->GetActiveSectorCount() ; i++ ) 
			{
				CEtWorldSector* pSector	= CDnWorld::GetInstance().GetGrid()->GetActiveSector(i);
				CEtWorldEventControl* pControl	= pSector->GetControlFromUniqueID( ETE_EventArea );
				if (pControl == NULL)
					continue;

				CEtWorldEventArea* pArea = pControl->GetAreaFromName( szStartName );
				if (pArea) 
				{
					SOBB *pBox = pArea->GetOBB();
					if (pBox)
						startPosCenter = pBox->Center;
					break;
				}
			}

			SOBB* pOBB = pGate->pGateArea->GetOBB();
			if( pOBB != NULL)
			{
				EtVector3 vGatePos = pOBB->Center;
				EtVector3 gateOriented = startPosCenter - vGatePos;
				EtVec3Normalize(&gateOriented, &gateOriented);
				if (pLocalActor)
					pLocalActor->OnCancelStage(gateOriented);
				
			}
		}
	}
	else
	{
		_ASSERT(0);
	}
}

void CDnPartyTask::OnRecvPartyRefreshGateInfo( SCRefreshGateInfo *pPacket )
{
	FUNC_LOG();

	if (pPacket->nRet == ERROR_PARTY_CANT_ENTERGATE)
	{
		// Temporarily adjust code to fall back out.
		m_nEnteredGateIndex = m_nRequestEnteredGateIndex = pPacket->GateNo[0];
		m_bRequestEnteredGate = false;
		OnCancelStage();
		return;
	}

	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		m_LocalData.nEnteredGateIndex = pPacket->GateNo[0];
		SetLocalActorEnterGateIdx(pPacket->GateNo[0], false);
		// In single-player mode REfreshPartyInfo does not arrive, so force reset here (because there is nowhere else to apply mid-game changes like hair color, etc.).
		if( CDnActor::s_hLocalActor ) 
			SetLocalData( CDnActor::s_hLocalActor );
		//m_nEnteredGateIndex = pPacket->GateNo[0];
	}
	else {
		int nLeaderGateNo = -1;
		for( DWORD i=0; i<m_VecPartyList.size(); i++ ) {
			if (m_VecPartyList[i].cMemberIndex >= pPacket->cCount )
			{
				_ASSERT(0);
				continue;
			}

			int nPrevGate = m_VecPartyList[i].nEnteredGateIndex;
			m_VecPartyList[i].nEnteredGateIndex = pPacket->GateNo[m_VecPartyList[i].cMemberIndex];
			if( nPrevGate != m_VecPartyList[i].nEnteredGateIndex )
			{
				bool bIsBot = ( m_VecPartyList[i].nSessionID >= 0x70000000 );
				DnDebugConsole::LogW( bIsBot ? L"BOT" : L"GATE", L"%s GateReady Name=%s MemberIdx=%d Gate=%d",
					bIsBot ? L"[BOT]" : L"[USER]", m_VecPartyList[i].wszCharacterName, (int)m_VecPartyList[i].cMemberIndex, m_VecPartyList[i].nEnteredGateIndex );
			}
			if( m_VecPartyList[i].bPartyMaster ) nLeaderGateNo = pPacket->GateNo[m_VecPartyList[i].cMemberIndex];
		}

#ifdef PRE_FIX_PARTYINDEX_DUMP
		if (m_nLocalActorPartyIndex < 0 || m_nLocalActorPartyIndex >= (int)m_VecPartyList.size())
		{
			SetLocalActorPartyIndex();
			if (m_nLocalActorPartyIndex < 0 || m_nLocalActorPartyIndex >= (int)m_VecPartyList.size())
			{
				OnCancelStage();
				m_bRequestEnteredGate = false;
				return;
			}
		}

		const int& memberIndex = m_VecPartyList[m_nLocalActorPartyIndex].cMemberIndex;
		if (memberIndex < 0 || memberIndex >= pPacket->cCount)
		{
			OnCancelStage();
			m_bRequestEnteredGate = false;
			return;
		}
#else
		if (m_nLocalActorPartyIndex < 0 || m_nLocalActorPartyIndex >= (int)m_VecPartyList.size())
			_ASSERT(0);

		const int& memberIndex = m_VecPartyList[m_nLocalActorPartyIndex].cMemberIndex;
		if (memberIndex < 0 || memberIndex >= pPacket->cCount)
			_ASSERT(0);
#endif

		bool bForceMsgOff = (pPacket->nRet == ERROR_PARTY_DUNGEONWARPOUT_FAIL_ITEMSHARING);
		SetLocalActorEnterGateIdx(pPacket->GateNo[memberIndex], bForceMsgOff);

		if (pPacket->nRet == ERROR_PARTY_REFRESHGATEFAIL_REQRETREAT)
		{
			OnCancelStage();
			m_bRequestEnteredGate = false;
			return;
		}

		// Handle when entering a different gate from the party leader
		if( m_nEnteredGateIndex != -1 && nLeaderGateNo != -1 && m_nEnteredGateIndex != nLeaderGateNo ) {
			CDnWorld::GateStruct *pStruct = CDnWorld::GetInstance().GetGateStruct( nLeaderGateNo );
			if( pStruct && pStruct->pGateArea ) {
				EtVector3 vTargetPos = pStruct->pGateArea->GetOBB()->Center;
				GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3414), true );
				GetInterface().OpenNotifyArrowDialog( vTargetPos, 5000, true );
			}
		}

		if (pPacket->nRet == ERROR_PARTY_DUNGEONWARPOUT_FAIL_ITEMSHARING)
		{
			m_bNoWarpBeforeItemSharing = true;
			CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption2, 3578, textcolor::RED );
		}
	}

 	m_bRequestEnteredGate = false;
// 	m_nRequestEnteredGateIndex = -1;

	/*
	CDnLocalPlayerActor* pLocalPlayer = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	if( pLocalPlayer && pLocalPlayer->IsLockInput() ) {
		pLocalPlayer->LockInput( false );
	}
	*/
}

void CDnPartyTask::OnRecvPartyChangeLeader( char *pPacket )
{
	SCPartyChangeLeader *pMsg = (SCPartyChangeLeader*)pPacket;
	int nMasterIndex = ChangePartyMaster( pMsg->nNewLeaderSessionID );
	GetInterface().SetPartyInfoLeader( nMasterIndex );

	if (pMsg->nNewLeaderSessionID == CDnBridgeTask::GetInstance().GetSessionID())
	{
		CInputDevice::GetInstance().ResetAbsenceCheckTimer(CInputDevice::eABSENCE_PARTY);

		if (GetPartyCount() == 1)
		{
			PartyStruct* pStruct = GetPartyDataFromSessionID(pMsg->nNewLeaderSessionID);
			if (pStruct != NULL)
			{
				pStruct->bAbsence = false;
				GetInterface().RefreshPartyInfoList();
			}
		}
	}

	GetInterface().RefreshRebirthInterface();
}

void CDnPartyTask::OnRecvPartyInviteMsg(char * pPacket)
{
	SCPartyInvite * pMsg = (SCPartyInvite*)pPacket;

	if( m_PartyRole != PartyRoleEnum::SINGLE )
		return;

	// Whether currently trading... Now allowed to send during trade as well.
	// Whether in invite-decline state...
	// Whether fatigue is non-zero...

	WCHAR wszPartyName[PARTYNAMELENMAX]={0};
#ifdef PRE_PARTY_DB
	int iPassword = Party::Constants::INVALID_PASSWORD;
	if (pMsg->iPassword != Party::Constants::INVALID_PASSWORD)
		iPassword = pMsg->iPassword;
#else
	WCHAR wszPartyPass[PARTYPASSWORDMAX]={0};
#endif
	WCHAR wszInviterName[NAMELENMAX]={0};

	_wcscpy(wszPartyName, _countof(wszPartyName), pMsg->wszBuf, pMsg->nPartyNameLen);
#ifdef PRE_PARTY_DB
	_wcscpy(wszInviterName, _countof(wszInviterName), pMsg->wszBuf + pMsg->nPartyNameLen, pMsg->cPartyInviterNameLen);
#else
	_wcscpy(wszPartyPass, _countof(wszPartyPass), pMsg->wszBuf + pMsg->nPartyNameLen, pMsg->cPartyPassLen);
	_wcscpy(wszInviterName, _countof(wszInviterName), pMsg->wszBuf + pMsg->nPartyNameLen + pMsg->cPartyPassLen, pMsg->cPartyInviterNameLen);
#endif

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	CDnPartyAcceptRequestInviteInfo InviteInfo;
	InviteInfo.m_szOtherSideName = wszInviterName;
#else
	SInviteInfo InviteInfo;
	InviteInfo.m_szInviterName = wszInviterName;
#endif
	InviteInfo.m_PartyID = pMsg->PartyID;
	InviteInfo.m_szPartyName = wszPartyName;
#ifdef PRE_PARTY_DB
	InviteInfo.m_szPassword = FormatW(L"%d", iPassword);
#else
	InviteInfo.m_szPassword = wszPartyPass;
#endif
	InviteInfo.m_nMaxUserCount	= pMsg->cMaxUserCount;
	InviteInfo.m_nCurUserCount	= pMsg->cCurUserCount;
	InviteInfo.m_nAvrLevel		= pMsg->nAvrLevel;
	InviteInfo.m_nGameServerID	= pMsg->nGameServerID;
	InviteInfo.m_nGameMapIdx	= pMsg->nGameMapIdx;

	if (GetInterface().IsOpenBlind() || CDnInterface::GetInstance().IsPVP() || CDnInterface::GetInstance().IsPVPLobby() || CDnInterface::GetInstance().IsFarm() )
	{
		ReqDenyPartyInvite(InviteInfo, true);
		return;
	}

	if( GetInterface().GetChatRoomDlg() && GetInterface().GetChatRoomDlg()->IsShow() )
	{
		ReqDenyPartyInvite(InviteInfo);
		return;
	}

#ifdef PRE_ADD_SECONDARY_SKILL
	if( CDnLifeSkillCookingTask::IsActive() && ( GetLifeSkillCookingTask().IsNowCooking() || GetLifeSkillCookingTask().IsRequestCooking() ) )
	{
		ReqDenyPartyInvite(InviteInfo);
		return;
	}
	GetInterface().CloseCookingDialog();
#endif // PRE_ADD_SECONDARY_SKILL

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
	if (GetInterface().IsShowChangeJobDialog())
	{
		ReqDenyPartyInvite(InviteInfo);
		return;
	}
#endif

	if( GetInterface().IsShowMapMoveCashItemDlg() )
	{
		ReqDenyPartyInvite(InviteInfo);
		return;
	}

#ifdef PRE_MOD_INTEG_SYSTEM_STATE

	#ifdef PRE_MOD_SYSTEM_STATE
	bool bStartState = GetInterface().GetSystemStateMgr().StartState(eSTATE_REQ_PARTY);
	if (bStartState == false)
	{
		ReqDenyPartyInvite(InviteInfo);
		return;
	}
	#endif

	m_AcceptRequestInviteInfo.AddAcceptRequestInfo(InviteInfo);
#else
	m_listInviteInfo.push_back(InviteInfo);
#endif
}

void CDnPartyTask::OnRecvPartyInviteFail(char * pPacket)
{
	SCPartyInviteFail * pMsg = (SCPartyInviteFail*)pPacket;
	GetInterface().ServerMessageBox(pMsg->nRetCode);

	if (IsPartyProcessState(CDnPartyTask::PPS_ACCEPT_INVITE))
		m_PartyProcessState &= ~PPS_ACCEPT_INVITE;

	GetInterface().CloseCaptionDialog(CDnInterface::typeCaption5);
	m_bPartyMemberInviting = false;
}

void CDnPartyTask::OnRecvPartyInviteDenied(SCPartyInviteDenied * pPacket)
{
	//A bit messy, but for now...
	if (pPacket->nRetCode == ERROR_NONE)
	{
		std::wstring str;
		str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3558), pPacket->wszCharName);	//	UISTRING : Party invitation to %s was declined.
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), str.c_str(), false);
	}
	else
		GetInterface().ServerMessageBox(pPacket->nRetCode);

	GetInterface().CloseCaptionDialog(CDnInterface::typeCaption5);
	m_bPartyMemberInviting = false;
}

void CDnPartyTask::OnRecvPartyReadyGate( SCGateInfo *pPacket )
{
	EnableWaitGateState(true);
	//m_bWaitGate = true;
	FUNC_LOG();
}

void CDnPartyTask::OnRecvPartySkillData( SCSkillData *pPacket )
{
#ifdef PRE_MOD_SYNCPACKET
	SetSkillInfo( pPacket );
#else		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberSkill MemberSkill[MAX_SEND_SYNCPLAYER];
	int nOffset = sizeof(SCSkillData)-sizeof(pPacket->MemberSkill);
	memset( &MemberSkill, 0, sizeof(MemberSkill) );
	int nSize = 0;

	for( int i=0; i<pPacket->cCount; i++ ) {
		char cCount = *((char*)pPacket + nOffset);
		nSize = ( cCount * sizeof(TSkill) ) + sizeof(char);
		memcpy( &MemberSkill[i], (char*)pPacket + nOffset, nSize );
		nOffset += nSize;
	}

	SetSkillInfo( MemberSkill, pPacket );
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDnPartyTask::OnRecvPartyEtcData( SCEtcData *pPacket )
{
	SetEtcInfo( pPacket );
}

void CDnPartyTask::OnRecvPartyEquipData(SCEquipData * pPacket)
{
	LogWnd::Log(1, _T("OnTRecvPartyStageStart "));

#ifdef PRE_MOD_SYNCPACKET
	SetEquipInfo( pPacket );
#else		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberEquip MemberEquip[MAX_SEND_SYNCPLAYER];
	int nOffset = sizeof(SCEquipData)-sizeof(pPacket->MemberEquip);
	memset( &MemberEquip, 0, sizeof(MemberEquip) );
	int nSize = 0;

	for( int i=0; i<pPacket->cCount; i++ ) {
		char cCount = *((char*)pPacket + nOffset);
		nSize = ( cCount * sizeof(TItemInfo) ) + sizeof(char);
		memcpy( &MemberEquip[i], (char*)pPacket + nOffset, nSize );
		nOffset += nSize;
	}

	SetEquipInfo( MemberEquip, pPacket );
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDnPartyTask::OnRecvPartyCashEquipData( SCCashEquipData *pPacket )
{
#ifdef PRE_MOD_SYNCPACKET
	SetCashEquipInfo( pPacket );
#else		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberCashEquip MemberEquip[MAX_SEND_SYNCPLAYER];
	int nOffset = sizeof(SCCashEquipData)-sizeof(pPacket->MemberEquip);
	memset( &MemberEquip, 0, sizeof(MemberEquip) );
	int nSize = 0;

	for( int i=0; i<pPacket->cCount; i++ ) {
		char cCount = *((char*)pPacket + nOffset);
		nSize = ( cCount * sizeof(TItemInfo) ) + sizeof(char);
		memcpy( &MemberEquip[i], (char*)pPacket + nOffset, nSize );
		nOffset += nSize;
	}

	SetCashEquipInfo( MemberEquip, pPacket );
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDnPartyTask::OnRecvPartyDefaultPartsData( SCDefaultPartsData *pPacket )
{
	SetDefaultPartsInfo( pPacket );
}

void CDnPartyTask::OnRecvPartyWeaponOrderData( SCWeaponOrderData *pPacket )
{
	SetWeaponOrderInfo( pPacket );
}

void CDnPartyTask::OnRecvPartyGlyphEquipData( SCGlyphEquipData *pPacket )
{
#ifdef PRE_MOD_SYNCPACKET
	SetGlyphEquipInfo( pPacket );
#else		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberGlyphEquip MemberEquip[MAX_SEND_SYNCPLAYER];
	int nOffset = sizeof(SCGlyphEquipData)-sizeof(pPacket->MemberEquip);
	memset( &MemberEquip, 0, sizeof(MemberEquip) );
	int nSize = 0;

	for( int i=0; i<pPacket->cCount; i++ ) {
		char cCount = *((char*)pPacket + nOffset);
		nSize = ( cCount * sizeof(TItemInfo) ) + sizeof(char);
		memcpy( &MemberEquip[i], (char*)pPacket + nOffset, nSize );
		nOffset += nSize;
	}

	SetGlyphEquipInfo( MemberEquip, pPacket );
#endif		//#ifdef PRE_MOD_SYNCPACKET
}


void CDnPartyTask::OnRecvPartyVehicleEquipData( SCVehicleEquipData *pPacket )
{
#ifdef PRE_MOD_SYNCPACKET
	SetVehicleEquipInfo( pPacket );
#else		//#ifdef PRE_MOD_SYNCPACKET
	TVehicleCompact MemberEquip[MAX_SEND_SYNCPLAYER];
	int nOffset = sizeof(SCVehicleEquipData)-sizeof(pPacket->MemberVehicle);
	memset( &MemberEquip, 0, sizeof(MemberEquip) );
	int nSize = 0;

	for( int i=0; i<pPacket->cCount; i++ ) {
		nSize = sizeof(TVehicleCompact);
		memcpy( &MemberEquip[i], (char*)pPacket + nOffset, nSize );
		nOffset += nSize;
	}

	SetVehicleEquipInfo( MemberEquip, pPacket );
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDnPartyTask::OnRecvPartyPetEquipData( SCVehicleEquipData* pPacket )
{
#ifdef PRE_MOD_SYNCPACKET
	SetPetEquipInfo( pPacket );
#else		//#ifdef PRE_MOD_SYNCPACKET
	TVehicleCompact MemberEquip[MAX_SEND_SYNCPLAYER];
	int nOffset = sizeof(SCVehicleEquipData)-sizeof(pPacket->MemberVehicle);
	memset( &MemberEquip, 0, sizeof(MemberEquip) );
	int nSize = 0;

	for( int i=0; i<pPacket->cCount; i++ ) 
	{
		nSize = sizeof(TVehicleCompact);
		memcpy( &MemberEquip[i], (char*)pPacket + nOffset, nSize );
		nOffset += nSize;
	}

	SetPetEquipInfo( MemberEquip, pPacket );
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDnPartyTask::OnRecvPartyGuildData( TPartyMemberGuild *pPacket )
{
	for( int i = 0; i < pPacket->nCount; ++i ) {
		if( !pPacket->List[i].IsSet() ) continue;
		PartyStruct *pPartyStruct = GetPartyDataFromSessionID( pPacket->List[i].nSessionID );
		if( !pPartyStruct ) continue;
		pPartyStruct->GuildSelfView = pPacket->List[i];
	}
}

void CDnPartyTask::OnRecvPartyJoinGetReversionItem(SCPartyJoinGetReversionItemInfo* pData)
{
	if (pData == NULL)
	{
		_ASSERT(0);
		return;
	}

	DnDebugConsole::LogW( L"DICE", L"JoinReversionItem DropUniqueID=%u", pData->dropItemUniqueId );
	
	GetInterface().OpenItemChoiceDialog(pData->itemInfo, 30.f, pData->dropItemUniqueId);
	m_bReversionItemSharing = true;
}

bool CDnPartyTask::HasPartyMemberJobDicePriority(int diceValue) const
{
	return (diceValue > MAXPARTYDICEGAMENUMBER);
}

void CDnPartyTask::OnRecvPartyRollGetReversionItem(SCRollDiceForGetReverseItem* pData)
{
	if (pData == NULL)
	{
		_ASSERT(0);
		return;
	}

	std::wstring rollValueStr;
	std::wstring jobDiceStr;
	PartyStruct* pPartyData = GetPartyDataFromSessionID(pData->nSessionID);
	if (pPartyData)
	{
		int diceValue = pData->nDiceValue;
		if (IsJobDice() && HasPartyMemberJobDicePriority(diceValue))
			diceValue -= (MAXPARTYDICEGAMENUMBER + 1);
		DnDebugConsole::LogW( L"DICE", L"Roll %s SessionID=%u Name=%s Rolled=%d Value=%d",
			(pData->nSessionID >= 0x70000000) ? L"[BOT]" : L"[USER]",
			pData->nSessionID, pPartyData->wszCharacterName, pData->bRoll ? 1 : 0, diceValue );
		rollValueStr = (pData->bRoll) ? 
		FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3582), pPartyData->wszCharacterName, diceValue, MAXPARTYDICEGAMENUMBER) : 
		FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3577), pPartyData->wszCharacterName); // UISTRING : %s rolled %d. (Max %d), %s passed on rolling.

		jobDiceStr = (pData->bRoll && IsJobDice() && HasPartyMemberJobDicePriority(pData->nDiceValue)) ? 
			FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3405), pPartyData->wszCharacterName) : L""; // UISTRING : %s has dice roll priority over other classes.
	}

	GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", rollValueStr.c_str(), false);
	if (jobDiceStr.empty() == false)
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", jobDiceStr.c_str(), false);
};

void CDnPartyTask::OnRecvPartyResultGetReversionItem(SCResultGetReverseItem* pData)
{
	if (pData == NULL)
	{
		_ASSERT(0);
		return;
	}

	std::wstring str;
	PartyStruct* pPartyData = GetPartyDataFromSessionID(pData->nSessionID);
	if (pPartyData)
	{
		if (pData->dropItemUniqueId != UINT_MAX)
		{
			DnDropItemHandle hDropItem = CDnDropItem::FindItemFromUniqueID(pData->dropItemUniqueId);
			if (hDropItem)
				hDropItem->SetOwnerUniqueID(pData->nSessionID);
		}
		std::wstring temp, itemStr;
		temp = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3547), pPartyData->wszCharacterName);	// UISTRING : %s acquired an item.
		GetInterface().GetNameLinkMng()->MakeNameLinkString_Item(itemStr, pData->itemInfo);

		str = FormatW(L"%s %s", temp.c_str(), itemStr.c_str());
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);
	}

	m_bReversionItemSharing = false;
	m_bNoWarpBeforeItemSharing = false;
}

void CDnPartyTask::OnRecvPartyReadyRequest()
{
	int nMsgIndex = 0;
	if (GetPartyRole() != CDnPartyTask::LEADER && IsLocalActorEnterGateReady() == false)
		nMsgIndex = 3591;	// UISTRING : The party leader requested ready. Move to the portal when ready.
	else if( GetPartyRole() == CDnPartyTask::LEADER )
		nMsgIndex = 3404;	// UISTRING : Requested party members to ready up.

	if (nMsgIndex)
	{
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nMsgIndex), true);
		CEtSoundEngine::GetInstance().PlaySound("2D", m_PartySoundIndex[REQUESTREADY]);
	}
}

void CDnPartyTask::OnRecvPartyMemberAbsent(SCMemberAbsent * pPacket)
{
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm )
		return;

	CDnPartyTask::PartyStruct* pStruct = GetPartyDataFromSessionID(pPacket->nSessionID);
	if (pStruct && pStruct->hActor != NULL)
	{
		if( pStruct->bGMTrace )
			return;

		CDnPlayerActor *pPlayerActor = static_cast<CDnPlayerActor*>((CDnActor*)pStruct->hActor);
		if (pPlayerActor)
		{
			bool bAbsent = (pPacket->cAbsentFlag == 0);
			pPlayerActor->SetAbsence(bAbsent);
			pStruct->bAbsence = (bAbsent);
			GetInterface().RefreshPartyInfoList();

			if (bAbsent)
			{
				std::wstring absentStr;
				if (CDnActor::s_hLocalActor->GetUniqueID() == pPacket->nSessionID)
					absentStr = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3415);	// UISTRING : Status will change to Away.
				else
					absentStr = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3594), pPlayerActor->GetName());	// UISTRING : %s is away.
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", absentStr.c_str(), false);
			}
		}
		else
		{
			OutputDebug("[ABSENCE] OnRecvPartyMemberAbsent : PartyActor(%d) does not exist.\n", pPacket->nSessionID);
		}
	}
}

void CDnPartyTask::OnRecvVerifyDungeonRequest(SCVerifyDungeon * pPacket)
{
	if (pPacket == NULL)
	{
		_ASSERT(0);
		return;
	}

	GetInterface().SetPartyCreateDifficulties(pPacket->cVerifyFlag);		//0: cannot select difficulty, 1: can select difficulty
}

void CDnPartyTask::OnRecvPartyStageStart( SCStartStage *pPacket )
{
}

void CDnPartyTask::OnRecvPartyStageStartDirect( SCStartStageDirect *pPacket )
{
}

void CDnPartyTask::OnRecvPartyStageCancel( SCCancelStage *pPacket )
{
	EnableWaitGateState(false);
	FUNC_LOG();
}

void CDnPartyTask::OnRecvPartyVillageStart( char *pPacket )
{
	EnableWaitGateState(false);
	FUNC_LOG();
}

void CDnPartyTask::OnRecvRoomSyncWait( SCSyncWait *pPacket )
{
	DebugLog("CDnPartyTask::OnRecvRoomSyncWait");

	bool bIsObserver = false;

	for( DWORD i=0; i<GetPartyCount(); i++ ) {
		if( GetPartyData(i)->nSessionID != pPacket->nSessionID) continue;
		GetPartyData(i)->bSync = true;
		if( IsPvpGame() )
			bIsObserver = (GetPartyData(i)->usTeam == PvPCommon::Team::Observer);
		break;
	}
	GetInterface().RefreshPartyGateInfo();
	// If local, fade in.
	if( bIsObserver || (CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetUniqueID() == pPacket->nSessionID) ) {
		bool bFadeIn = true;
		CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
		if( pTask ) {
			if( pTask->IsExistInitTimeProcess() ) bFadeIn = false;
		}

		if( bFadeIn ) {
			CEtSoundEngine::GetInstance().FadeVolume( NULL, 1.f, 0.f, false );
			GetInterface().FadeDialog( 0xff000000, 0x00000000, CGlobalInfo::GetInstance().m_fFadeDelta );
		}
	}
}
void CDnPartyTask::OnRecvRoomSyncStart( SCSyncStart *pPacket )
{
#ifndef _FINAL_BUILD
	LogWnd::Log(1, _T("RequestSyncWaitMsg "));
	DebugLog("OnGameSyncStart::RecvSyncStartMsg");
	GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"Debug", L"RecvSyncStartMsg");
#endif
	m_bSyncComplete = true;
	CDnLocalPlayerActor::LockInput( false );

	char buff[512];
	sprintf_s(buff, "try connect to agent-rudp ip:%s port:%d \n", CClientSessionManager::GetInstance().GetUdpIP(), CClientSessionManager::GetInstance().GetUdpPort());
	OutputDebug(buff);
	DnDebugConsole::LogA( "INSTANCE", "RoomSyncStart: SessionID=%u UDP=%s:%d PartyCount=%u",
		pPacket ? pPacket->nSessionID : 0, CClientSessionManager::GetInstance().GetUdpIP(), CClientSessionManager::GetInstance().GetUdpPort(), (UINT)m_VecPartyList.size() );
	FUNC_LOG();

	CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstance().GetTask( "GameTask" ));
	DN_ASSERT( pGameTask != NULL, "CDnPartyTask::OnRecvRoomSyncStart() pGameTask != NULL" );

	// If it is a drop-in Task, sync existing users.
	if( pGameTask->bIsBreakIntoTask() )
	{
		for( UINT i=0 ; i<m_VecPartyList.size() ; ++i )
		{
			if( m_VecPartyList[i].nSessionID != pPacket->nSessionID )
			{
				m_VecPartyList[i].bSync = true;
			}
		}
	}

	GetInterface().RefreshPartyGateInfo();
}

void CDnPartyTask::OnRecvRefreshMember(SCRefreshMember * pPacket)
{
	//Arrives before syncstart.
	std::vector<int> nVecList;
	for( int i=0; i<pPacket->cCount; i++ ) {
		nVecList.push_back( pPacket->nMemberSessionID[i] );
	}
	for( DWORD i=0; i<m_VecPartyList.size(); i++ ) {
		PartyStruct *pStruct = &m_VecPartyList[i];
		if( std::find( nVecList.begin(), nVecList.end(), pStruct->nSessionID ) == nVecList.end() ) {
			OnPartyMemberOut( pStruct->nSessionID, -1 );
			i--;
		}
	}
}

void CDnPartyTask::OnRecvRoomMemberInfo( SCROOM_SYNC_MEMBERINFO* pPacket )
{
	CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask( "GameTask" ));
	DnActorHandle hExistingLocal = ( !pPacket->bIsBreakInto && pGameTask && pGameTask->GetGameTaskType() != GameTaskType::PvP ) ? CDnActor::s_hLocalActor : CDnActor::Identity();

	m_PartyRole = PartyRoleEnum::MEMBER;
	if( pPacket->bIsStart == true )
		m_VecPartyList.clear();
	m_PartyState = ePartyState::NORMAL;

	for( int i=0; i<pPacket->nCount; ++i ) 
	{
		CDnPartyTask::PartyStruct Struct;
		Struct.Assign( pPacket->Member[i] );
		if( Struct.cMemberIndex >= PARTYMAX )
			Struct.cMemberIndex = (BYTE)m_VecPartyList.size();

		Struct.bCompleteBreakInto = true;
		m_VecPartyList.push_back( Struct );
	}

	for( UINT i=0 ; i<m_VecPartyList.size() ; ++i )
	{
		if( m_VecPartyList[i].nSessionID == CDnBridgeTask::GetInstance().GetSessionID() )
		{
			m_nLocalActorPartyIndex = i;
			if( hExistingLocal )
				m_VecPartyList[i].hActor = hExistingLocal;
		}
	}

	if( !pPacket->bIsBreakInto && pGameTask && pGameTask->GetGameTaskType() != GameTaskType::PvP && m_nLocalActorPartyIndex == 0 )
	{
		m_PrevPartyRole = PartyRoleEnum::SINGLE;
		m_PartyRole = PartyRoleEnum::LEADER;
		if( !m_VecPartyList.empty() )
			m_VecPartyList[0].bPartyMaster = true;
	}

	DnDebugConsole::LogW( L"INSTANCE", L"RoomMemberInfo: Start=%d BreakInto=%d PacketCount=%d TotalParty=%u LocalIdx=%d",
		pPacket->bIsStart ? 1 : 0, pPacket->bIsBreakInto ? 1 : 0, (int)pPacket->nCount, (UINT)m_VecPartyList.size(), m_nLocalActorPartyIndex );
	for( UINT i = 0; i < m_VecPartyList.size(); ++i )
	{
		bool bBot = ( m_VecPartyList[i].nSessionID >= 0x70000000 );
		DnDebugConsole::LogW( bBot ? L"BOT" : L"PARTY", L"  [%u] %s SessionID=%u Name=%s Class=%d Lv=%d MemberIdx=%d Team=%u",
			i, bBot ? L"[BOT]" : L"[USER]", m_VecPartyList[i].nSessionID, m_VecPartyList[i].wszCharacterName,
			(int)m_VecPartyList[i].cClassID, (int)m_VecPartyList[i].cLevel, (int)m_VecPartyList[i].cMemberIndex, (UINT)m_VecPartyList[i].usTeam );
	}

	if( pPacket->bIsBreakInto )
	{
		// Mark that this task was created by drop-in.
		if( pGameTask )
		{
			pGameTask->SetBreakIntoTask();
		}
		else
		{
			_ASSERT(0);
		}
	}
}

void CDnPartyTask::OnRecvRoomMemberTeam( SCROOM_SYNC_MEMBERTEAM* pPacket )
{
#ifdef PRE_MOD_SYNCPACKET
	std::vector<PartyStruct>::iterator ii;
	for( int i=0 ; i<pPacket->cCount ; ++i )
	{
		for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
		{
			if ((*ii).nSessionID == pPacket->tTeam[i].nSessionID)
			{
				(*ii).usTeam = pPacket->tTeam[i].usTeam;
				(*ii).OnRecv(SC_ROOM, eRoom::SC_SYNC_MEMBERTEAM);
				break;
			}
		}
	}
#else		//#ifdef PRE_MOD_SYNCPACKET
	for( int i=0 ; i<pPacket->cCount ; ++i )
	{
		int nPartyIndex = i+pPacket->unStartIndex;
		if( 0 <= nPartyIndex && nPartyIndex < (int)m_VecPartyList.size() )
			m_VecPartyList[nPartyIndex].usTeam = pPacket->usTeam[i];
	}
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDnPartyTask::OnRecvRoomMemberHPSP( SCROOM_SYNC_MEMBERHPSP* pPacket )
{
#ifdef PRE_MOD_SYNCPACKET
	std::vector<PartyStruct>::iterator ii;
	for( int i = 0; i < pPacket->cCount; ++i )
	{
		for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
		{
			if ((*ii).nSessionID == pPacket->tHPSP[i].nSessionID)				
			{
				(*ii).bSyncHPSP = true;
				(*ii).iSyncHP = pPacket->tHPSP[i].iHP;
				(*ii).iSyncSP = pPacket->tHPSP[i].iSP;
				(*ii).OnRecv(SC_ROOM, eRoom::SC_SYNC_MEMBERHPSP);
				break;
			}
		}
	}
#else		//#ifdef PRE_MOD_SYNCPACKET
	for( int i=pPacket->unStartIndex ; i<pPacket->unStartIndex+pPacket->cCount ; ++i )
	{
		m_VecPartyList[i].bSyncHPSP	= true;
		m_VecPartyList[i].iSyncHP	= *(pPacket->iHPSPArr+(i-pPacket->unStartIndex)*2);
		m_VecPartyList[i].iSyncSP	= *(pPacket->iHPSPArr+((i-pPacket->unStartIndex)*2+1));
	}
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDnPartyTask::OnRecvRoomMemberBattleMode( SCROOM_SYNC_MEMBERBATTLEMODE* pPacket )
{
#ifdef PRE_MOD_SYNCPACKET
	std::vector<PartyStruct>::iterator ii;
	for( int i = 0; i < pPacket->cCount; ++i )
	{
		for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
		{
			if ((*ii).nSessionID == pPacket->tBattleMode[i].nSessionID)
			{
				(*ii).cSyncBattleMode = pPacket->tBattleMode[i].cBattleMode;
				(*ii).OnRecv(SC_ROOM, eRoom::SC_SYNC_MEMBERBATTLEMODE);
				break;
			}
		}
	}
#else		//#ifdef PRE_MOD_SYNCPACKET
	for( int i=pPacket->unStartIndex ; i<pPacket->unStartIndex+pPacket->cCount ; ++i )
	{
		m_VecPartyList[i].cSyncBattleMode = *(pPacket->cBattleModeArr+(i-pPacket->unStartIndex));
	}
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDnPartyTask::OnRecvRoomDropItemList( SCROOM_SYNC_DROPITEMLIST* pPacket )
{
	for( UINT i=0 ; i<pPacket->cCount ; ++i )
		CDnDropItem::DropItem( pPacket->sDropItemList[i] );
}

void CDnPartyTask::OnRecvRoomMemberBreakInto( SCROOM_SYNC_MEMBER_BREAKINTO* pPacket, int nSize )
{
	if( CDnLoadingTask::IsActive() ) {
		CDnLoadingTask::GetInstance().InsertLoadObject( 
			OnLoadRecvBreakIntoCallback, 
			OnLoadRecvBreakIntoUniqueID,
			OnCheckLoadingPacket, 
			NULL,
			this, (void*)pPacket, nSize, m_LocalTime, CDnLoadingTask::CreateActor );
	}
}

bool __stdcall CDnPartyTask::OnLoadRecvBreakIntoCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime )
{
	SCROOM_SYNC_MEMBER_BREAKINTO *pPacket = (SCROOM_SYNC_MEMBER_BREAKINTO *)pParam;
	CDnPartyTask *pTask = (CDnPartyTask *)pThis;

	CDnPartyTask::PartyStruct Struct;
	Struct.Assign( pPacket->Member );

	PartyStruct* pStruct = &Struct;
	if( pStruct )
	{
		pStruct->bGMTrace	= (pPacket->cGMTrace == 1) ? true : false;
		pStruct->usTeam		= pPacket->usTeam;
		memcpy( pStruct->nDefaultPartsIndex, pPacket->iDefaultPartsIndex, sizeof(pPacket->iDefaultPartsIndex) );

		// Title
		pStruct->nSelectAppellation	= pPacket->iSelectAppellation;
		pStruct->nSelectCoverAppellation	= pPacket->nCoverAppellation;
#if defined(PRE_ADD_VIP)
		pStruct->bVIP = pPacket->bVIP;
#endif	// #if defined(PRE_ADD_VIP)
		pStruct->cAccountLevel = pPacket->cAccountLevel;
		memcpy(pStruct->cViewCashEquipBitmap, pPacket->cViewCashEquipBitmap, sizeof(pStruct->cViewCashEquipBitmap));

		pStruct->VehicleInfo = pPacket->VehicleInfo;
		pStruct->PetInfo = pPacket->PetInfo;
		char* pBuffer = reinterpret_cast<char*>(pPacket->EquipArray);

		// Equipment
		for( int i=0 ; i<pPacket->nEquipCount ; ++i )
		{
			TItemInfo* pItemInfo = reinterpret_cast<TItemInfo*>(pBuffer);

			if( pItemInfo->cSlotIndex < EQUIP_WEAPON1)
				pStruct->VecParts.push_back( *pItemInfo );
			else 
				pStruct->Weapon[ pItemInfo->cSlotIndex-EQUIP_WEAPON1 ] = *pItemInfo;

			pBuffer += sizeof(TItemInfo);
		}

		// Cash equipment
		for( int i=0 ; i<pPacket->nCashEquipCount ; ++i )
		{
			TItemInfo* pCashItemInfo = reinterpret_cast<TItemInfo*>(pBuffer);

			if( pCashItemInfo->cSlotIndex < CASHEQUIP_WEAPON1 || pCashItemInfo->cSlotIndex == CASHEQUIP_EFFECT )
				pStruct->VecCashParts.push_back( *pCashItemInfo );
			else
				pStruct->CashWeapon[ pCashItemInfo->cSlotIndex-CASHEQUIP_WEAPON1 ] = *pCashItemInfo;

			pBuffer += sizeof(TItemInfo);
		}

		// Crest
		for( int i=0 ; i<pPacket->nGlyphCount ; ++i )
		{
			TItemInfo* pItemInfo = reinterpret_cast<TItemInfo*>(pBuffer);
			pStruct->VecGlyph.push_back( *pItemInfo );
			pBuffer += sizeof(TItemInfo);
		}

		// Skill
		for( int i=0 ; i<pPacket->nSkillCount ; ++i )
		{
			TSkill* pSkill = reinterpret_cast<TSkill*>(pBuffer);
			pStruct->VecSkill.push_back( *pSkill );
			pBuffer += sizeof(TSkill);
		}

		// Guild
		pStruct->GuildSelfView = pPacket->GuildSelfView;

		CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
		if( pGameTask ) 
			pGameTask->OnInitializeBreakIntoActor( pStruct, 0 );
	}

	pTask->AddBreakIntoPartyList( Struct );
	OutputDebug( "OnLoadRecvBreakIntoCallback\n" );
	return true;
}

int __stdcall CDnPartyTask::OnLoadRecvBreakIntoUniqueID( void *pParam, int nSize )
{
	return ((SCROOM_SYNC_MEMBER_BREAKINTO *)pParam)->Member.nSessionID;
}

bool __stdcall CDnPartyTask::OnCheckLoadingPacket( void *pThis, UINT nUniqueID, int nMainCmd, int nSubCmd, void *pParam, int nSize )
{
	// Actor protocol on drop-in is handled in GameTask, since this side cannot do it..
	if( nMainCmd == SC_ACTOR ) {
		if( pThis != dynamic_cast<CDnPartyTask*>(CTaskManager::GetInstance().GetTask( "GameTask" )) ) {
			return CGameClientSession::OnCheckLoadingPacket( pThis, nUniqueID, nMainCmd, nSubCmd, pParam, nSize );
		}
	}
	if( pThis != dynamic_cast<CDnPartyTask*>(CTaskManager::GetInstance().GetTask( "PartyTask" )) ) return false;
	switch( nMainCmd ) {
		case SC_ROOM:
			switch( nSubCmd ) {
				case eRoom::SC_SYNC_MEMBER_BREAKINTO:
					if( ((SCROOM_SYNC_MEMBER_BREAKINTO*)pParam)->Member.nSessionID == nUniqueID ) {
						return true;
					}
					break;
			}
			break;
		case SC_PARTY:
			switch( nSubCmd ) {
				case eParty::SC_PARTYOUT:
					if( ((SCPartyOut*)pParam)->nSessionID == nUniqueID ) {
						return true;
					}
					break;
				case eParty::SC_REFRESHPARTY:
					return true;
			}
			break;
	}
	return false;
}

void CDnPartyTask::SetDefaultPartsInfo( SCDefaultPartsData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;

		memcpy( pStruct->nDefaultPartsIndex, pPacket->DefaultParts[0].iDefaultPartsIndex, sizeof(pStruct->nDefaultPartsIndex) );
	}
	else {
#ifdef PRE_MOD_SYNCPACKET
		std::vector<PartyStruct>::iterator ii;
		PartyStruct * pStruct;
		for( int i = 0; i < pPacket->cCount; i++ )
		{
			for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
			{
				if ((*ii).nSessionID == pPacket->DefaultParts[i].nSessionID)
				{
					pStruct = &(*ii);
					memcpy( pStruct->nDefaultPartsIndex, pPacket->DefaultParts[i].iDefaultPartsIndex, sizeof(pStruct->nDefaultPartsIndex) );
					pStruct->OnRecv(SC_PARTY, eParty::SC_DEFAULTPARTSDATA);
					break;
				}
			}
		}
#else		//#ifdef PRE_MOD_SYNCPACKET
		for( int i=pPacket->unStartIndex; i<pPacket->unStartIndex+pPacket->cCount; i++ ) {
			PartyStruct *pStruct = &m_VecPartyList[i];

			memcpy( pStruct->nDefaultPartsIndex, pPacket->DefaultParts[i-pPacket->unStartIndex].iDefaultPartsIndex, sizeof(pStruct->nDefaultPartsIndex) );
		}
#endif		//#ifdef PRE_MOD_SYNCPACKET
	}
}

void CDnPartyTask::SetWeaponOrderInfo( SCWeaponOrderData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;
		memcpy( pStruct->cViewCashEquipBitmap, pPacket->WeaponOrder[0].cViewCashEquipBitmap, sizeof(pStruct->cViewCashEquipBitmap) );
	}
	else {
#ifdef PRE_MOD_SYNCPACKET
		std::vector<PartyStruct>::iterator ii;
		PartyStruct * pStruct;
		for( int i = 0; i < pPacket->cCount; i++ )
		{
			for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
			{
				if ((*ii).nSessionID == pPacket->WeaponOrder[i].nSessionID)
				{
					pStruct = &(*ii);
					memcpy( pStruct->cViewCashEquipBitmap, pPacket->WeaponOrder[i].cViewCashEquipBitmap, sizeof(pStruct->cViewCashEquipBitmap) );
					pStruct->OnRecv(SC_PARTY, eParty::SC_WEAPONORDERDATA);
					break;
				}
			}
		}
#else		//#ifdef PRE_MOD_SYNCPACKET
		PartyStruct *pStruct;
		for( int i=pPacket->unStartIndex; i<pPacket->unStartIndex+pPacket->cCount; i++ ) {
			pStruct = &m_VecPartyList[i];
			memcpy( pStruct->cViewCashEquipBitmap, pPacket->WeaponOrder[i-pPacket->unStartIndex].cViewCashEquipBitmap, sizeof(pStruct->cViewCashEquipBitmap) );
		}
#endif		//#ifdef PRE_MOD_SYNCPACKET
	}
}

#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetEquipInfo( SCEquipData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;
		for( int j = 0; j < pPacket->MemberEquip[0].cCount; j++ )
		{
			if ( pPacket->MemberEquip[0].EquipArray[j].cSlotIndex < EQUIP_WEAPON1)
				pStruct->VecParts.push_back( pPacket->MemberEquip[0].EquipArray[j] );
			else 
				pStruct->Weapon[ pPacket->MemberEquip[0].EquipArray[j].cSlotIndex - EQUIP_WEAPON1 ] = pPacket->MemberEquip[0].EquipArray[j];
		}
	}
	else {
		std::vector<PartyStruct>::iterator ii;
		for( int i = 0; i < pPacket->cCount; i++ )
		{
			for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
			{
				PartyStruct& info = (*ii);
				if (info.nSessionID == pPacket->MemberEquip[i].nSessionID)
				{
					for( int j = 0; j < pPacket->MemberEquip[i].cCount; j++ )
					{
						if ( pPacket->MemberEquip[i].EquipArray[j].cSlotIndex < EQUIP_WEAPON1)
							info.VecParts.push_back( pPacket->MemberEquip[i].EquipArray[j] );
						else 
							info.Weapon[ pPacket->MemberEquip[i].EquipArray[j].cSlotIndex - EQUIP_WEAPON1 ] = pPacket->MemberEquip[i].EquipArray[j];
					}
					info.OnRecv(SC_PARTY, eParty::SC_EQUIPDATA);
					break;
				}
			}
		}
	}

	m_nStartingMemberCount = static_cast<int>(m_VecPartyList.size());
}
#else		//#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetEquipInfo( TPartyMemberEquip *pEquipInfo, SCEquipData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;

		for( int j=0; j<pEquipInfo[0].cCount; j++ ) {
			if ( pEquipInfo[0].EquipArray[j].cSlotIndex < EQUIP_WEAPON1)
				pStruct->VecParts.push_back( pEquipInfo[0].EquipArray[j] );
			else 
				pStruct->Weapon[ pEquipInfo[0].EquipArray[j].cSlotIndex - EQUIP_WEAPON1 ] = pEquipInfo[0].EquipArray[j];
		}
	}
	else {
		PartyStruct *pStruct;
		for( int i=pPacket->unStartIndex ; i<pPacket->unStartIndex+pPacket->cCount ; i++ ) {
			pStruct = &m_VecPartyList[i];

			for( int j=0; j<pEquipInfo[i-pPacket->unStartIndex].cCount; j++ ) {
				if ( pEquipInfo[i-pPacket->unStartIndex].EquipArray[j].cSlotIndex < EQUIP_WEAPON1)
					pStruct->VecParts.push_back( pEquipInfo[i-pPacket->unStartIndex].EquipArray[j] );
				else 
					pStruct->Weapon[ pEquipInfo[i-pPacket->unStartIndex].EquipArray[j].cSlotIndex - EQUIP_WEAPON1 ] = pEquipInfo[i-pPacket->unStartIndex].EquipArray[j];
			}
		}
	}

	m_nStartingMemberCount = static_cast<int>(m_VecPartyList.size());
}
#endif		//#ifdef PRE_MOD_SYNCPACKET

#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetCashEquipInfo( SCCashEquipData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;
		for( int j=0; j<pPacket->MemberEquip[0].cCount; j++ )
		{
			if ( pPacket->MemberEquip[0].EquipArray[j].cSlotIndex < CASHEQUIP_WEAPON1 || pPacket->MemberEquip[0].EquipArray[j].cSlotIndex == CASHEQUIP_EFFECT )
				pStruct->VecCashParts.push_back( pPacket->MemberEquip[0].EquipArray[j] );
			else 
				pStruct->CashWeapon[ pPacket->MemberEquip[0].EquipArray[j].cSlotIndex - CASHEQUIP_WEAPON1 ] = pPacket->MemberEquip[0].EquipArray[j];
		}
	}
	else {
		std::vector<PartyStruct>::iterator ii;
		PartyStruct *pStruct;
		for( int i = 0; i < pPacket->cCount; i++ )
		{
			for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
			{
				if ((*ii).nSessionID == pPacket->MemberEquip[i].nSessionID)
				{
					pStruct = &(*ii);
					for( int j=0; j<pPacket->MemberEquip[i].cCount; j++ )
					{
						if ( pPacket->MemberEquip[i].EquipArray[j].cSlotIndex < CASHEQUIP_WEAPON1 || pPacket->MemberEquip[i].EquipArray[j].cSlotIndex == CASHEQUIP_EFFECT )
							pStruct->VecCashParts.push_back( pPacket->MemberEquip[i].EquipArray[j] );
						else 
							pStruct->CashWeapon[ pPacket->MemberEquip[i].EquipArray[j].cSlotIndex - CASHEQUIP_WEAPON1 ] = pPacket->MemberEquip[i].EquipArray[j];
					}
					break;
				}
			}
		}
	}
}
#else		//#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetCashEquipInfo( TPartyMemberCashEquip *pEquipInfo, SCCashEquipData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;

		for( int j=0; j<pEquipInfo[0].cCount; j++ ) {
			if ( pEquipInfo[0].EquipArray[j].cSlotIndex < CASHEQUIP_WEAPON1 || pEquipInfo[0].EquipArray[j].cSlotIndex == CASHEQUIP_EFFECT )
				pStruct->VecCashParts.push_back( pEquipInfo[0].EquipArray[j] );
			else 
				pStruct->CashWeapon[ pEquipInfo[0].EquipArray[j].cSlotIndex - CASHEQUIP_WEAPON1 ] = pEquipInfo[0].EquipArray[j];
		}
	}
	else {
		PartyStruct *pStruct;
		for( int i=pPacket->unStartIndex ; i<pPacket->unStartIndex+pPacket->cCount ; i++ ) {
			pStruct = &m_VecPartyList[i];

			for( int j=0; j<pEquipInfo[i-pPacket->unStartIndex].cCount; j++ ) {
				if ( pEquipInfo[i-pPacket->unStartIndex].EquipArray[j].cSlotIndex < CASHEQUIP_WEAPON1 || pEquipInfo[i-pPacket->unStartIndex].EquipArray[j].cSlotIndex == CASHEQUIP_EFFECT )
					pStruct->VecCashParts.push_back( pEquipInfo[i-pPacket->unStartIndex].EquipArray[j] );
				else 
					pStruct->CashWeapon[ pEquipInfo[i-pPacket->unStartIndex].EquipArray[j].cSlotIndex - CASHEQUIP_WEAPON1 ] = pEquipInfo[i-pPacket->unStartIndex].EquipArray[j];
			}
		}
	}
}
#endif		//#ifdef PRE_MOD_SYNCPACKET

#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetGlyphEquipInfo( SCGlyphEquipData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;

		for( int j=0; j<pPacket->MemberEquip[0].cCount; j++ ) {
			pStruct->VecGlyph.push_back( pPacket->MemberEquip[0].EquipArray[j] );
		}
	}
	else {
		std::vector<PartyStruct>::iterator ii;
		PartyStruct *pStruct;
		for( int i = 0; i < pPacket->cCount; i++ )
		{
			for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
			{
				if ((*ii).nSessionID == pPacket->MemberEquip[i].nSessionID)
				{
					pStruct = &(*ii);
					for( int j = 0; j < pPacket->MemberEquip[i].cCount; j++ )
					{
						pStruct->VecGlyph.push_back( pPacket->MemberEquip[i].EquipArray[j] );
					}
					pStruct->OnRecv(SC_PARTY, eParty::SC_GLYPHDATA);
					break;
				}
			}
		}
	}
}
#else		//#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetGlyphEquipInfo( TPartyMemberGlyphEquip *pEquipInfo, SCGlyphEquipData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;

		for( int j=0; j<pEquipInfo[0].cCount; j++ ) {
			pStruct->VecGlyph.push_back( pEquipInfo[0].EquipArray[j] );
		}
	}
	else {
		PartyStruct *pStruct;
		for( int i=pPacket->unStartIndex ; i<pPacket->unStartIndex+pPacket->cCount ; i++ ) {
			pStruct = &m_VecPartyList[i];

			for( int j=0; j<pEquipInfo[i-pPacket->unStartIndex].cCount; j++ ) {
				pStruct->VecGlyph.push_back( pEquipInfo[i-pPacket->unStartIndex].EquipArray[j] );
			}
		}
	}
}
#endif		//#ifdef PRE_MOD_SYNCPACKET


#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetVehicleEquipInfo( SCVehicleEquipData* pPacket ) // PartyStruct holds the mount info.
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;
		pStruct->VehicleInfo = pPacket->MemberVehicle[0];
	}
	else {
		std::vector<PartyStruct>::iterator ii;
		PartyStruct *pStruct;
		for( int i = 0; i < pPacket->cCount; i++ )
		{
			for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
			{
				if ((*ii).nSessionID == pPacket->MemberVehicle[i].nSessionID)
				{
					pStruct = &(*ii);
					pStruct->VehicleInfo = pPacket->MemberVehicle[i];
					pStruct->OnRecv(SC_PARTY, eParty::SC_VEHICLEEQUIPDATA);
					break;
				}
			}
		}
	}
}
#else		//#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetVehicleEquipInfo( TVehicleCompact *pEquipInfo, SCVehicleEquipData* pPacket ) // PartyStruct holds the mount info.
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;
		pStruct->VehicleInfo = pEquipInfo[0];
	}
	else {
		PartyStruct *pStruct;
		for( int i=pPacket->unStartIndex ; i<pPacket->unStartIndex+pPacket->cCount ; i++ ) {
			pStruct = &m_VecPartyList[i];
			pStruct->VehicleInfo = pEquipInfo[i-pPacket->unStartIndex];
		}
	}
}
#endif		//#ifdef PRE_MOD_SYNCPACKET

#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetSkillInfo( SCSkillData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;

		for( int j=0; j<pPacket->MemberSkill[0].cCount; j++ ) {
			pStruct->VecSkill.push_back( pPacket->MemberSkill[0].SkillArray[j] );
		}
	}
	else {
		std::vector<PartyStruct>::iterator ii;
		PartyStruct *pStruct;
		for( int i = 0; i < pPacket->cCount; i++ )
		{
			for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
			{
				if ((*ii).nSessionID == pPacket->MemberSkill[i].nSessionID)
				{
					pStruct = &(*ii);
					for( int j = 0; j < pPacket->MemberSkill[i].cCount; j++ )
					{
						pStruct->VecSkill.push_back( pPacket->MemberSkill[i].SkillArray[j] );
					}
					pStruct->OnRecv(SC_PARTY, eParty::SC_SKILLDATA);
					break;
				}
			}
		}
	}
}
#else		//#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetSkillInfo( TPartyMemberSkill *pSkillInfo, SCSkillData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;

		for( int j=0; j<pSkillInfo[0].cCount; j++ ) {
			pStruct->VecSkill.push_back( pSkillInfo[0].SkillArray[j] );
		}
	}
	else {
		PartyStruct *pStruct;
		for( int i=pPacket->unStartIndex ; i<pPacket->unStartIndex+pPacket->cCount ; i++ ) {
			pStruct = &m_VecPartyList[i];

			for( int j=0; j<pSkillInfo[i-pPacket->unStartIndex].cCount; j++ ) {
				pStruct->VecSkill.push_back( pSkillInfo[i-pPacket->unStartIndex].SkillArray[j] );
			}
		}
	}
}
#endif		//#ifdef PRE_MOD_SYNCPACKET

void CDnPartyTask::SetEtcInfo( SCEtcData* pPacket )
{
#ifdef PRE_MOD_SYNCPACKET
	if( m_PartyRole == PartyRoleEnum::SINGLE )
	{
		PartyStruct* pStruct = &m_LocalData;
		pStruct->nSelectAppellation = pPacket->EtcData[0].nSelectAppellation;
		pStruct->nSelectCoverAppellation = pPacket->EtcData[0].nCoverAppellation;
		pStruct->bGMTrace			= (pPacket->EtcData[0].cGMTrace == 1) ? true : false;
#if defined(PRE_ADD_VIP)
		pStruct->bVIP				= pPacket->EtcData[0].bVIP;
#endif	// #if defined(PRE_ADD_VIP)
		pStruct->cAccountLevel 		= pPacket->EtcData[0].cAccountLevel ;
	}
	else
	{
		std::vector<PartyStruct>::iterator ii;
		PartyStruct* pStruct;
		for( int i = 0; i < pPacket->cCount; i++ )
		{
			for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
			{
				if ((*ii).nSessionID == pPacket->EtcData[i].nSessionID)
				{
					pStruct = &(*ii);
					pStruct->nSelectAppellation = pPacket->EtcData[i].nSelectAppellation;
					pStruct->nSelectCoverAppellation = pPacket->EtcData[i].nCoverAppellation;
					pStruct->bGMTrace			= (pPacket->EtcData[i].cGMTrace == 1) ? true : false;
#if defined(PRE_ADD_VIP)
					pStruct->bVIP				= pPacket->EtcData[i].bVIP;
#endif	// #if defined(PRE_ADD_VIP)
					pStruct->cAccountLevel 		= pPacket->EtcData[i].cAccountLevel ;
					pStruct->OnRecv(SC_PARTY, eParty::SC_ETCDATA);
					break;
				}
			}
		}
	}
#else		//#ifdef PRE_MOD_SYNCPACKET
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct* pStruct = &m_LocalData;
		pStruct->nSelectAppellation = pPacket->EtcData[0].nSelectAppellation;
		pStruct->nSelectCoverAppellation = pPacket->EtcData[0].nCoverAppellation;
		pStruct->bGMTrace			= (pPacket->EtcData[0].cGMTrace == 1) ? true : false;
#if defined(PRE_ADD_VIP)
		pStruct->bVIP				= pPacket->EtcData[0].bVIP;
#endif	// #if defined(PRE_ADD_VIP)
		pStruct->cAccountLevel 		= pPacket->EtcData[0].cAccountLevel ;
	}
	else
	{
		PartyStruct* pStruct;
		for( int i=pPacket->unStartIndex ; i<pPacket->unStartIndex+pPacket->cCount ; i++ ) {
			pStruct = &m_VecPartyList[ i ];
			pStruct->nSelectAppellation = pPacket->EtcData[i-pPacket->unStartIndex].nSelectAppellation;
			pStruct->nSelectCoverAppellation = pPacket->EtcData[i-pPacket->unStartIndex].nCoverAppellation;
			pStruct->bGMTrace			= (pPacket->EtcData[i-pPacket->unStartIndex].cGMTrace == 1) ? true : false;
#if defined(PRE_ADD_VIP)
			pStruct->bVIP				= pPacket->EtcData[i-pPacket->unStartIndex].bVIP;
#endif	// #if defined(PRE_ADD_VIP)
			pStruct->cAccountLevel 		= pPacket->EtcData[i-pPacket->unStartIndex].cAccountLevel ;
		}
	}
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetPetEquipInfo( SCVehicleEquipData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;
		pStruct->PetInfo = pPacket->MemberVehicle[0];
	}
	else {
		std::vector<PartyStruct>::iterator ii;
		PartyStruct *pStruct;
		for( int i = 0; i < pPacket->cCount; i++ )
		{
			for (ii = m_VecPartyList.begin(); ii != m_VecPartyList.end(); ii++)
			{
				if ((*ii).nSessionID == pPacket->MemberVehicle[i].nSessionID)
				{
					pStruct = &(*ii);
					pStruct->PetInfo = pPacket->MemberVehicle[i];
					pStruct->OnRecv(SC_PARTY, eParty::SC_PETEQUIPDATA);
					break;
				}
			}
		}
	}
}
#else		//#ifdef PRE_MOD_SYNCPACKET
void CDnPartyTask::SetPetEquipInfo( TVehicleCompact *pEquipInfo, SCVehicleEquipData* pPacket )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		PartyStruct *pStruct = &m_LocalData;
		pStruct->PetInfo = pEquipInfo[0];
	}
	else {
		PartyStruct *pStruct;
		for( int i=pPacket->unStartIndex ; i<pPacket->unStartIndex+pPacket->cCount ; i++ ) {
			pStruct = &m_VecPartyList[i];
			pStruct->PetInfo = pEquipInfo[i-pPacket->unStartIndex];
		}
	}
}
#endif		//#ifdef PRE_MOD_SYNCPACKET

DWORD CDnPartyTask::GetPartyCount() 
{ 
	if( m_PartyRole == PartyRoleEnum::SINGLE ) 
		return 1;
	return (DWORD)m_VecPartyList.size(); 
}

DWORD CDnPartyTask::GetPartyCountExcepGM() const
{
	DWORD ret = 0;
	if( m_PartyRole == PartyRoleEnum::SINGLE ) 
		return 1;

	std::vector<PartyStruct>::const_iterator iter = m_VecPartyList.begin();
	for (; iter != m_VecPartyList.end(); ++iter)
	{
		const PartyStruct& ps = (*iter);
		if (ps.bGMTrace == false)
			ret++;
	}

	return ret;
}


DWORD CDnPartyTask::GetObserverCount()
{
	int nObserverCount= 0;

	for(int i=0; i<(int)m_VecPartyList.size();i++)
	{
		if(m_VecPartyList[i].usTeam == PvPCommon::Team::Observer)
			nObserverCount++;
	}
	
	return nObserverCount;
}

CDnPartyTask::PartyStruct *CDnPartyTask::GetPartyData( DWORD dwIndex )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) 
		return &m_LocalData;

	if( dwIndex >= (DWORD)m_VecPartyList.size() ) return NULL;
	return &m_VecPartyList[dwIndex];
}

CDnPartyTask::PartyStruct *CDnPartyTask::GetPartyDataFromSessionID( UINT nSessionID, bool bFindBreakIntoList )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) 
		return &m_LocalData;

	for( int i=0; i<(int)m_VecPartyList.size(); i++ )
	{
		if( m_VecPartyList[i].nSessionID == nSessionID )
		{
			return &m_VecPartyList[i];
		}
	}
	if( bFindBreakIntoList ) 
	{
		ScopeLock<CSyncLock> Lock( m_BreakIntoPartyListLock );
		for( std::list<PartyStruct>::iterator itor=m_BreakIntoPartyList.begin() ; itor!=m_BreakIntoPartyList.end() ; ++itor )
		{
			if( (*itor).nSessionID == nSessionID )
				return &(*itor);
		}
	}

	return NULL;
}

CDnPartyTask::PartyStruct *CDnPartyTask::GetPartyDataFromMemberName( const WCHAR* pName )
{
	if( m_PartyRole == PartyRoleEnum::SINGLE ) 
		return &m_LocalData;

	for( int i=0; i<(int)m_VecPartyList.size(); i++ )
	{
		if (!_tcscmp(m_VecPartyList[i].wszCharacterName, pName))
			return &m_VecPartyList[i];
	}

	return NULL;
}

#ifdef PRE_PARTY_DB
void CDnPartyTask::ReqCreateParty(SPartyCreateParam& param)
{
#ifdef _WORK
	if ((param.bAddDummyParty == false) && (m_PartyRole != PartyRoleEnum::SINGLE))
		return;
#else
	if( m_PartyRole != PartyRoleEnum::SINGLE )
		return;	
#endif

	param.cUseVoice = 0;
	if (CGameOption::GetInstance().m_bVoiceChat && CGameOption::GetInstance().m_bMicTest)
		param.cUseVoice = 1;
	SendCreateParty(param);
}
#else
void CDnPartyTask::ReqCreateParty(BYTE cPartyType, BYTE cPartyMemberMax, int nUserLvLimitMin, int nUserLvLimitMax, const WCHAR * pPartyName, const WCHAR * pPass, int nMapIdx, int nDifficulty, BYTE cIsPartyDice)
{
	if( m_PartyRole != PartyRoleEnum::SINGLE )
		return;	

	BYTE cUseVoice = 0;
	if( CGameOption::GetInstance().m_bVoiceChat && CGameOption::GetInstance().m_bMicTest ) cUseVoice = 1;
	SendCreateParty( cPartyType, cPartyMemberMax, nUserLvLimitMin, nUserLvLimitMax, ITEMLOOTRULE_INORDER, ITEMRANK_B, pPartyName, pPass, nMapIdx, nDifficulty, cIsPartyDice, cUseVoice );
}
#endif

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
void CDnPartyTask::ReqDenyPartyInvite(const CDnPartyAcceptRequestInviteInfo& info, bool bIsOpenBlind)
{
#if defined( PRE_PARTY_DB )
	if (IsValidGameMapIdx(info.m_nGameMapIdx) && info.m_nGameServerID >= 0 )
#else
	if (IsValidGameMapIdx(info.m_nGameMapIdx))
#endif // #if defined( PRE_PARTY_DB )
		SendOverServerInviteResult(info.m_nGameServerID, info.m_szOtherSideName.c_str(), false, bIsOpenBlind);
	else
		SendInviteDenied(info.m_PartyID, info.m_szOtherSideName.c_str(), bIsOpenBlind);
}

void CDnPartyTask::ReqJoinPartyByInvite(const CDnPartyAcceptRequestInviteInfo& info)
{
	LockPartyReqPartyMemberInfo(true);

#if defined( PRE_PARTY_DB )
	if (IsValidGameMapIdx(info.m_nGameMapIdx) && info.m_nGameServerID >= 0 )
#else
	if (IsValidGameMapIdx(info.m_nGameMapIdx))
#endif // #if defined( PRE_PARTY_DB )
		SendOverServerInviteResult(info.m_nGameServerID, info.m_szOtherSideName.c_str(), true);
	else
#ifdef PRE_PARTY_DB
		SendJoinParty(info.m_PartyID, _wtoi(info.m_szPassword.c_str()));
#else
		SendJoinParty(info.m_PartyID, info.m_szPassword.c_str());
#endif

	if (GetInterface().IsOpenAcceptDialog())
		GetInterface().CloseAcceptDialog();

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	ClearInviteInfoList(false);
#else
	ClearInviteInfoList();
#endif
}
#else // PRE_MOD_INTEG_SYSTEM_STATE
void CDnPartyTask::ReqDenyPartyInvite(const SInviteInfo& info, bool bIsOpenBlind)
{
	if (IsValidGameMapIdx(info.m_nGameMapIdx))
		SendOverServerInviteResult(info.m_nGameServerID, info.m_szInviterName.c_str(), false, bIsOpenBlind);
	else
		SendInviteDenied(info.m_PartyID, info.m_szInviterName.c_str(), bIsOpenBlind);
}

void CDnPartyTask::ReqJoinPartyByInvite(const SInviteInfo& info)
{
	LockPartyReqPartyMemberInfo(true);

	if (IsValidGameMapIdx(info.m_nGameMapIdx))
		SendOverServerInviteResult(info.m_nGameServerID, info.m_szInviterName.c_str(), true);
	else
		SendJoinParty(info.m_PartyID, info.m_szPassword.c_str());

	if (GetInterface().IsOpenAcceptDialog())
		GetInterface().CloseAcceptDialog();

	if (m_listInviteInfo.empty() == false)
		m_listInviteInfo.pop_front();

	ClearInviteInfoList();
}
#endif

#ifdef PRE_PARTY_DB
void CDnPartyTask::ReqJoinParty( TPARTYID PartyIndex, int iPassword )
#else
void CDnPartyTask::ReqJoinParty( TPARTYID PartyIndex, const TCHAR *szPartyPassword )
#endif
{
	//if( !GetMessageManager().AddTaskMessage( SC_PARTY, eParty::SC_JOINPARTY, 110003 ) )
	//	return;

	LockPartyReqPartyMemberInfo(true);

	// SendJoinParty() CS_JOINPARTY here: password has already been entered in the dialog by this point. 
	
#ifdef PRE_PARTY_DB
	SendJoinParty( PartyIndex, iPassword );
#else
	SendJoinParty( PartyIndex, szPartyPassword );
#endif

	if( GetInterface().IsOpenAcceptDialog())
		GetInterface().CloseAcceptDialog();

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	ClearInviteInfoList(false);
#else
	ClearInviteInfoList();
#endif
}

#include "SystemSendPacket.h"
#ifdef PRE_FIX_GAMEQUIT_TO_VILLAGE
void CDnPartyTask::ReqOutParty(bool bIntendedDisconnect)
{
	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if (pTask)
	{
		SendAbandonStage(true, bIntendedDisconnect);
	}
	else
		SendPartyOut();
}
#else
void CDnPartyTask::ReqOutParty()
{
	//if( !GetMessageManager().AddTaskMessage( SC_PARTY, eParty::SC_PARTYOUT, 110004 ) )
	//	return;

	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if (pTask)
	{
		SendAbandonStage(true);
	}
	else
		SendPartyOut();
}
#endif

void CDnPartyTask::ReqInviteParty( const WCHAR *szPlayerName )
{
	if( m_PartyRole == PartyRoleEnum::MEMBER )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3560), MB_OK ); // UISTRING : Only the party leader can use the invite function
		return;
	}

	if (GetIsolateTask().IsBlackList(szPlayerName))
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4210), MB_OK ); // UISTRING : Cannot invite a blocked character to a party.
		return;
	}

	if (IsPartyInfoEmpty() == false && 
		(int)GetPartyCount() >= GetMaxPartyCount())
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3559), MB_OK ); // UISTRING : Cannot invite because the party is full
		return;
	}

	if (m_bPartyMemberInviting && CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeWorldMap)
	{
		std::wstring str;
		CDnInterface::GetInstance().GetServerMessage(str, ERROR_PARTY_INVITEFAIL_ALREADYINVITED);
		GetInterface().MessageBox(str.c_str(), MB_OK ); // UISTRING : Party invite is in progress. Duplicate invites are not allowed.
		return;
	}

	if (IsPartyMember(szPlayerName))
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3435), MB_OK ); // UISTRING : Cannot invite party member
		return;
	}

	SendInvitePartyMember( szPlayerName );
}

void CDnPartyTask::ReqAskParty(const WCHAR* szPlayerName)
{
	SendPartyAskJoin(szPlayerName);
}


#ifdef PRE_MOD_INTEG_SYSTEM_STATE
void CDnPartyTask::AnswerPartyAsking(bool bAccept, const std::wstring& applicantName)
{
	SendPartyAskJoinDecision(bAccept, applicantName.c_str());
//#endif 
}
#else
void CDnPartyTask::AnswerPartyAsking(bool bAccept, const SAskInfo& info)
{
	SendPartyAskJoinDecision(bAccept, info.szApplicantName.c_str());
}
#endif

#ifdef PRE_PARTY_DB
void CDnPartyTask::ReqPartyMemberInfo( TPARTYID nPartyIdx )
#else
void CDnPartyTask::ReqPartyMemberInfo( UINT nPartyIdx )
#endif
{
	// The function call above is required to activate the party join window. 
	if( m_PartyRole != PartyRoleEnum::SINGLE )
		return;

	LockPartyReqPartyMemberInfo(true);

	SendReqPartyMemberInfo( nPartyIdx );
}

void CDnPartyTask::OnInitializeVillageStage()
{
	m_bResetSort = true;
}

void CDnPartyTask::ResetGateIndex()
{
	//m_bWaitGate = false;
	EnableWaitGateState(false);
	SetLocalActorEnterGateIdx(INVALID_GATE_INDEX, false);
	//m_nEnteredGateIndex = -1;
	m_bRequestEnteredGate = false;
	m_nRequestEnteredGateIndex = -1;
	for( DWORD i=0; i<GetPartyCount(); i++ ) 
	{
		GetPartyData(i)->nEnteredGateIndex = -1;
	}
}

void CDnPartyTask::ResetSync()
{
	m_bSyncComplete = false;
	for( DWORD i=0; i<GetPartyCount(); i++ ) {
		GetPartyData(i)->bSync = false;
	}
}

void CDnPartyTask::RequestSyncWaitMsg()
{
#ifndef _FINAL_BUILD
	char szVirtualAddress[32] = { 0, };
	//OutputDebug( "SendSyncWaitMsg\n" );
	DebugLog("CDnPartyTask::RequestSyncWaitMsg");
	LogWnd::Log(1, _T("RequestSyncWaitMsg "));

	GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"Debug", L"SendSyncWaitMsg");
#endif
	SendGameSyncWait( CDnBridgeTask::GetInstance().GetSessionID(), _RELIABLE );
	CDnLocalPlayerActor::LockInput( true );

	FUNC_LOG();
}

void CDnPartyTask::UpdatePartyMemberInfo()
{
	for( DWORD i=0; i<GetPartyCount(); i++ ) {
		PartyStruct *pStruct = GetPartyData(i);
		if( !pStruct || !pStruct->hActor ) continue;
		DnActorHandle hActor = pStruct->hActor;
		CDnPlayerActor *pActor = (CDnPlayerActor *)hActor.GetPointer();
		pStruct->cLevel = (char)hActor->GetLevel();
		pStruct->nExp = pActor->GetExperience();
		pStruct->vPos = *hActor->GetPosition();
		pActor->GetJobHistory( pStruct->nVecJobHistoryList );
	}

	CDnInterface::GetInstance().RefreshPartyInfoList();
	CDnInterface::GetInstance().RefreshPartyGuage();

	CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
	if( pMainMenuDlg ) pMainMenuDlg->RefreshZoneMapPartyActor();

}

void CDnPartyTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{

	switch( nID ) 
	{
	case ACCEPT_DIALOG:
		if( nCommand == EVENT_BUTTON_CLICKED )
		{
			if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
			{
#ifdef PRE_PARTY_DB 
				ReqJoinParty( m_inviteInfo.m_PartyID, _wtoi(m_inviteInfo.m_szPassword.c_str()) );
#else
				ReqJoinParty( m_inviteInfo.m_PartyID, m_inviteInfo.m_szPassword.c_str() );
#endif
			}
			else if( (strcmp( pControl->GetControlName(), "ID_CANCEL" ) == 0) )
			{
				ReqDenyPartyInvite(m_inviteInfo);
			}

			GetInterface().CloseAcceptDialog();
		}
		break;
	case ACCEPT_REQUEST_DIALOG:
		if( nCommand == EVENT_BUTTON_CLICKED )
		{
			if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
			{
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
				if (m_AcceptRequestInviteInfo.IsListEmpty() == false)
#else
				if (m_listInviteInfo.empty() == false)
#endif
				{
					ReqJoinPartyByInvite(m_inviteInfo);
					m_PartyProcessState |= PPS_ACCEPT_INVITE;
				}
			}
			else if( (strcmp( pControl->GetControlName(), "ID_REJECT" ) == 0) )
			{
				ReqDenyPartyInvite(m_inviteInfo);

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
				m_AcceptRequestInviteInfo.PopAcceptRequestInfo();
#else
				// Delete only one
				m_listInviteInfo.pop_front();
#endif
			}

#ifdef PRE_MOD_SYSTEM_STATE
			GetInterface().GetSystemStateMgr().EndState(eSTATE_REQ_PARTY);
#endif
			GetInterface().CloseAcceptRequestDialog();
		}
		break;

	case ACCEPT_REQUEST_PARTYASK_DIALOG:
		{
			if (nCommand == EVENT_BUTTON_CLICKED)
			{
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
				if (strcmp( pControl->GetControlName(), "ID_OK" ) == 0)
				{
					if (m_AcceptRequestAskInfo.IsListEmpty() == false)
					{
						m_bPartyAcceptRequesting = true;
						AnswerPartyAsking(true, m_AskInfoCache.m_szOtherSideName);

					}
				}
				else if (strcmp( pControl->GetControlName(), "ID_REJECT" ) == 0)   // When rejected 
					AnswerPartyAsking(false, m_AskInfoCache.m_szOtherSideName);

				m_AcceptRequestAskInfo.PopAcceptRequestInfo();
#else // PRE_MOD_INTEG_SYSTEM_STATE
				if (strcmp( pControl->GetControlName(), "ID_OK" ) == 0)
				{
					if (m_listAskInfo.empty() == false)
					{
						m_bPartyAcceptRequesting = true;
						AnswerPartyAsking(true, m_AskInfoCache);
					}
				}
				else if (strcmp( pControl->GetControlName(), "ID_REJECT" ) == 0)
					AnswerPartyAsking(false, m_AskInfoCache);

				if (m_listAskInfo.empty() == false)
					m_listAskInfo.pop_front();
#endif // PRE_MOD_INTEG_SYSTEM_STATE

				GetInterface().CloseAcceptRequestDialog();
			}
		}
		break;
	case MESSAGEBOX_PARTY_RECONN_LAST_DUNGEON:
		{
			if (nCommand == EVENT_BUTTON_CLICKED)
			{
				bool bIsIn = (strcmp(pControl->GetControlName(), "ID_YES") == 0);
				SendPartyCheckLastDungeonInfo(bIsIn);

				if (bIsIn == false)
				{
					if (CDnActor::s_hLocalActor)
					{
						CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
						pLocalActor->LockInput(false);
					}

					GetInterface().CloseProgressSimpleMsgBox();
				}
			}
		}
		break;

#define _MAX_PARTY_RECONNECT_COUNTER 5
	case UI_DISABLE_DIALOG:
		{
			if (nCommand == EVENT_BUTTON_CLICKED)
			{
				m_PartyReconnectCheckCounter++;
				if (m_PartyReconnectCheckCounter > _MAX_PARTY_RECONNECT_COUNTER)
				{
					if (CDnActor::s_hLocalActor)
					{
						CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
						pLocalActor->LockInput(false);
					}

					GetInterface().CloseProgressSimpleMsgBox();
					GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3451), MB_OK); // UISTRING : Failed to connect to the party
				}

				SendPartyCheckLastDungeonInfo(true);
			}
		}
		break;
	}
}

void CDnPartyTask::OnRecvPartyMemberInfo(ePartyType type)
{
	CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
	if( pMainMenuDlg )
		pMainMenuDlg->OnRecvPartyMemberInfo(type);
}

void CDnPartyTask::OnRecvPartyRefreshRebirthCoin(SCRefreshRebirthCoin * pPacket)
{
	if( m_PartyRole == CDnPartyTask::SINGLE)
		return;

	for( DWORD i=0; i<GetPartyCount(); i++ ) 
	{
		PartyStruct& memberInfo = m_VecPartyList[i];
		if (memberInfo.nSessionID == pPacket->nSessionID)
		{
			memberInfo.wStdRebirthCoin = pPacket->cRebirthCoin + pPacket->cPCBangRebirthCoin;
			memberInfo.wCashRebirthCoin = pPacket->nCashRebirthCoin;
		}
	}

#ifdef PRE_FIX_NESTREBIRTH
	if (CDnItemTask::IsActive())
		CDnItemTask::GetInstance().SetUpdateRebirthCoinOrItem(true);
#endif

	GetInterface().RefreshPartyInfoList();
}

void CDnPartyTask::OnRecvPartyMemberKick( SCPartyKickedMember *pPacket )
{
	if (pPacket->nRetCode != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRetCode);
		return;
	}

	for( DWORD i=0; i<GetPartyCount(); i++ ) 
	{
		if( GetPartyData(i)->nSessionID == pPacket->nSessionID )
		{
			m_VecPartyList.erase( m_VecPartyList.begin() + i );
			break;
		}
	}

	for( DWORD i=0; i<(int)m_VecPartyList.size(); i++ ) 
	{
		const PartyStruct& data = m_VecPartyList[i];
		if( data.nSessionID == CDnBridgeTask::GetInstance().GetSessionID() )
		{
			m_nLocalActorPartyIndex = i;
			break;
		}
	}

	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pTask ) pTask->RefreshConnectUser();

	GetInterface().RefreshPartyInfoList();
}

void CDnPartyTask::OnRecvPartyLeaderSwap( SCSwapPartyLeader *pPacket )
{
	if (pPacket->nRetCode == ERROR_NONE)
	{
		int nMasterIndex = ChangePartyMaster( pPacket->nSessionID );
		GetInterface().SetPartyInfoLeader( nMasterIndex );
	}
}

void CDnPartyTask::OnRecvPartyModifyResult(SCPartyInfoModify * pPacket)
{
	GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnPartyTask::OnRecvCharEntered( SCEnter *pPacket )
{	
	// Ultimately, here the character has not fully entered the village yet (character state is STATE_CHECKVILLAGE).
	// Not handled properly. // Should process after loading in village.	
	//if( m_nPartyIndexToJoin != -1 )	
	//{	
	//	ReqJoinParty( m_nPartyIndexToJoin, NULL );	
	//	m_nPartyIndexToJoin = -1;	
	//}
}

void CDnPartyTask::JoinReservedParty()
{	
	if( m_nPartyIndexToJoin != -1 )
	{
#ifdef PRE_PARTY_DB
		ReqJoinParty( m_nPartyIndexToJoin, GetPartyPassword() );
		m_nPartyIndexToJoin = -1;
#else
		ReqJoinParty( m_nPartyIndexToJoin, m_wszPartyPasswordToJoin );
		m_nPartyIndexToJoin = -1;
		m_wszPartyPasswordToJoin[0] = '\0';
#endif
	}
}

void CDnPartyTask::OnRecvVerifyJoinParty(SCVerifyJoinParty * pPacket)
{
	if (pPacket->nRet != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
		LockPartyReqPartyMemberInfo(false);
	}
	else
	{
		//Normal processing
		//pPacket->nChannelID; // Use this to move channel, then
		//pPacket->nPartyIdx; // Use this to join.

		// Store in variable to join after channel move; password was already saved during Reserve.
		m_nPartyIndexToJoin = pPacket->nPartyIdx;

		// Call channel move. Doesn't it check congestion?
		// When channel population info arrives later, check it like below.
		//float fCongestion = (float)pChannelInfo->nCurrentUserCount / pChannelInfo->nMaxUserCount;

		GetTradeTask().GetTradePrivateMarket().ClearTradeUserInfoList();
		SendReqMoveSelectChannel( pPacket->nChannelID );
	}
}

void CDnPartyTask::ReqPartyMemberKick( int nMemberIndex, int nKickType )
{
	if( GetPartyRole() != LEADER )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3530 ), MB_OK );
		return;
	}

	if( nMemberIndex < 0 ) return;
	int i = 0;
	UINT nSessionID = 0;
	for (; i<(int)m_VecPartyList.size(); i++)
	{
		const PartyStruct& memberInfo = m_VecPartyList[i];
		if (memberInfo.cMemberIndex == nMemberIndex)
			nSessionID = memberInfo.nSessionID;
	}

	if (nSessionID == 0)
	{
		_ASSERT(0);
		return;
	}

	if( GetMasterSessionID() == nSessionID )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3531 ), MB_OK );
		return;
	}

	SendReqPartyMemberKick( nSessionID, nKickType );
}

void CDnPartyTask::ReqPartyMemberKick( const WCHAR *szPlayerName, int nKickType )
{
	if( GetPartyRole() == SINGLE )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3536 ), MB_OK );
		return;
	}

// 	DnActorHandle hActor = CDnActor::FindActorFromName( (TCHAR *)szPlayerName );
// 	if( !hActor ) return;
// 	DWORD nSessionID = hActor->GetUniqueID();
	PartyStruct* pBadManInfo = GetPartyDataFromMemberName(szPlayerName);
	if (pBadManInfo == NULL)
	{
		// need message?
		return;
	}
	UINT nSessionID = pBadManInfo->nSessionID;

	if( !IsPartyMember( nSessionID ) )
		return;

	if( GetPartyRole() == MEMBER )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3530 ), MB_OK );
		return;
	}

	if( GetMasterSessionID() == nSessionID )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3531 ), MB_OK );
		return;
	}

	SendReqPartyMemberKick( nSessionID, nKickType );
}

void CDnPartyTask::ReqPartyLeaderSwapWithSessionID( UINT nSessionID )
{
	if( GetPartyRole() != LEADER )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3530 ), MB_OK );
		return;
	}

	SendReqPartyLeaderSwap( nSessionID );
}

void CDnPartyTask::ReqPartyLeaderSwap( int nNewLeaderIndex )
{
	if( nNewLeaderIndex < 0) return;
	int i = 0;
	for (; i<(int)m_VecPartyList.size(); i++)
	{
		const PartyStruct& memberInfo = m_VecPartyList[i];
		if (memberInfo.cMemberIndex == nNewLeaderIndex)
		{
			if (memberInfo.nSessionID == CDnBridgeTask::GetInstance().GetSessionID())
				return;
			ReqPartyLeaderSwapWithSessionID(memberInfo.nSessionID);
			return;
		}
	}
}

bool CDnPartyTask::DelegatePartyLeader(bool bExcludeMe)
{
	int i = 0;
	for (; i<(int)m_VecPartyList.size(); i++)
	{
		const PartyStruct& memberInfo = m_VecPartyList[i];
		if (memberInfo.nSessionID == CDnBridgeTask::GetInstance().GetSessionID() && bExcludeMe)
			continue;

		if (memberInfo.bAbsence == false && memberInfo.bPartyMaster == false && memberInfo.bGMTrace == false)
		{
			ReqPartyLeaderSwapWithSessionID(memberInfo.nSessionID);
			break;
		}
	}

	if (i >= (int)m_VecPartyList.size())
		return false;
	
	return true;
}

int CDnPartyTask::ChangePartyMaster( UINT nSessionID )
{
	int i = 0;
	if (m_nMasterSessionID != nSessionID)
	{
		std::wstring changeMasterStr;
		for (i=0; i<(int)m_VecPartyList.size(); i++)
		{
			const PartyStruct& memberInfo = m_VecPartyList[i];
			if (memberInfo.nSessionID == nSessionID)
			{
				if (nSessionID == CDnBridgeTask::GetInstance().GetSessionID())
					changeMasterStr = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3556);	// UISTRING : You became the party leader.
				else
					changeMasterStr = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3557), memberInfo.wszCharacterName); // UISTRING : %s became the party leader.

				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, _T(""), changeMasterStr.c_str(), false);
				break;
			}
		}

		if (i >= (int)m_VecPartyList.size())
			_ASSERT(0);
	}

	m_nMasterSessionID = nSessionID;

	if (nSessionID == CDnBridgeTask::GetInstance().GetSessionID())
	{
		m_PartyRole = PartyRoleEnum::LEADER;

#ifdef PRE_FIX_PARTY_STAGECLEAR_CHANGEMASTER
#else
		if (m_PartyState == ePartyState::STAGE_CLEAR_WARP_STANDBY)
		{
			CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
			if (pTask != NULL)
				pTask->SetEnableDungeonClearLeaderWarp(true);
		}
#endif

		CDnGameTask* pTask = static_cast<CDnGameTask*>(CTaskManager::GetInstance().GetTask( "GameTask" ));
		if (pTask && pTask->GetGameTaskType() == GameTaskType::DarkLair)
		{
			CDnDLGameTask* pDarkLairGameTask = static_cast<CDnDLGameTask*>(pTask);
			if (pDarkLairGameTask)
				pDarkLairGameTask->EnableRequestChallengeDlg(true);
		}
	}
	else
	{
		m_PartyRole = PartyRoleEnum::MEMBER;
	}

	if (m_PartyState == ePartyState::NONE)
		m_PartyState = ePartyState::NORMAL;

	int nLeaderIndex(-1);

	for (i=0; i<(int)m_VecPartyList.size(); i++)
	{
		if (m_VecPartyList[i].nSessionID == nSessionID)
		{
			m_VecPartyList[i].bPartyMaster = true;
			nLeaderIndex = m_VecPartyList[i].cMemberIndex;
		}
		else
		{
			m_VecPartyList[i].bPartyMaster = false;
		}
	}

	GetInterface().RefreshPartyGuage();

	return nLeaderIndex;
}

void CDnPartyTask::CalcDungeonOffsetNeedItem( CDnWorld::DungeonGateStruct *pStruct, int &nResultItemID, int &nResultItemCount )
{
	for( DWORD i=0; i<pStruct->pVecMapList.size(); i++ ) {
		if( ((CDnWorld::DungeonGateStruct *)pStruct->pVecMapList[i])->nNeedItemID > 0 ) {
			nResultItemID = ((CDnWorld::DungeonGateStruct *)pStruct->pVecMapList[i])->nNeedItemID;
			nResultItemCount = ((CDnWorld::DungeonGateStruct *)pStruct->pVecMapList[i])->nNeedItemCount;
			break;
		}
	}
}

tstring CDnPartyTask::GetPermitGateErrorString( int nGateIndex )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAPGATE );
	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( !pTask ) return L"";

	tstring szCaption;
	CDnWorld::GateStruct *pStruct = CDnWorld::GetInstance().GetGateStruct( nGateIndex );

	if( pStruct->nMapIndex >= DUNGEONGATE_OFFSET ) {
		/*
		CDnWorld::PermitGateEnum PermitFlag = pStruct->PermitFlag;

		int nCount = GetPartyCount();
		int nMin, nMax;

		if( !IsPartyCountToEnterGate( nCount, (CDnWorld::DungeonGateStruct*)pStruct, &nMin, &nMax ) )
		{
			WCHAR szTemp[256];
			if( nMin != -1 ) {
				swprintf_s( szTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100053 ), nMin );
			}
			if( nMax != -1 ) {
				swprintf_s( szTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100054 ), nMax );
			}
			szCaption += szTemp;
			szCaption += L"\n";
		}

		if( PermitFlag & CDnWorld::PermitNotEnoughItem ) {
			szCaption += L"\n- ";
			szCaption += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2333 );
			szCaption += L" : ";

			int nItemNameID = 0, nItemCount = 0;
			CalcDungeonOffsetNeedItem( (CDnWorld::DungeonGateStruct*)pStruct, nItemNameID, nItemCount );

			if( nItemNameID ) {
				WCHAR wszTemp[32];
				swprintf_s( wszTemp, L" (%d)", nItemCount );
				szCaption += CDnItem::GetItemFullName( nItemNameID );
				szCaption += wszTemp;
				szCaption += L" ";

				szCaption += L"\n";
				for( DWORD k=0; k<pStruct->cVecNotEnoughItemActorIndex.size(); k++ ) {
					PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pStruct->cVecNotEnoughItemActorIndex[k] );
					if( !pPartyStruct ) continue;
					szCaption += pPartyStruct->wszCharacterName;
					if( k < pStruct->cVecNotEnoughItemActorIndex.size() - 1 ) szCaption += L", ";
				}
			}
		}
		if( PermitFlag & CDnWorld::PermitLessPlayerLevel ) {
			bool bCheck = false;
			for( int i=0; i<nCount; i++ ) {
				if( !IsPartyLevelToEnterGate( GetPartyData(i)->cLevel, (CDnWorld::DungeonGateStruct*)pStruct, &nMin, &nMax ) ) {
					bCheck = true;
					break;
				}
			}

			if( bCheck ) {
				szCaption += L"\n- ";
				if( nMin != -1 ) {
					WCHAR szTemp[256];
					swprintf_s( szTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100053 ), nMin );
					szCaption += szTemp;
				}
				if( nMax != -1 ) {
					if( nMin != -1 ) szCaption += L"\n-";
					WCHAR szTemp[256];
					swprintf_s( szTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100054 ), nMax );
					szCaption += szTemp;
				}

				szCaption += L"\n";
				for( DWORD k=0; k<pStruct->cVecLessLevelActorIndex.size(); k++ ) {
					PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pStruct->cVecLessLevelActorIndex[k] );
					if( !pPartyStruct ) continue;
					szCaption += pPartyStruct->wszCharacterName;
					if( k < pStruct->cVecLessLevelActorIndex.size() - 1 ) szCaption += L", ";
				}
			}
		}
		if( PermitFlag & CDnWorld::PermitExceedTryCount ) {
			szCaption += L"\n- ";
			szCaption += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 821 );
			szCaption += L"\n";


			for( DWORD k=0; k<pStruct->cVecExceedTryActorIndex.size(); k++ ) {
				PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pStruct->cVecExceedTryActorIndex[k] );
				if( !pPartyStruct ) continue;
				szCaption += pPartyStruct->wszCharacterName;
				if( k < pStruct->cVecExceedTryActorIndex.size() - 1 ) szCaption += L", ";
			}
		}
		*/
		int nCount = GetPartyCount();
		int nMin, nMax;

		if( !IsPartyCountToEnterGate( GetPartyCountExcepGM(), (CDnWorld::DungeonGateStruct*)pStruct, &nMin, &nMax ) )
		{
			WCHAR szTemp[256];
			if( nMin != -1 && (int)GetPartyCountExcepGM() < nMin ) {
				swprintf_s( szTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114060 ), nMin );
			}

			if( nMax != -1 && (int)GetPartyCountExcepGM() > nMax ) {
				swprintf_s( szTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114059 ), nMax );
			}
			return szTemp;
		}
		CDnPartyTask::PartyStruct* pParty(NULL);
		for( int i = 0; i<nCount; i++ )
		{
			pParty = GetPartyData( i );
			if( !pParty ) continue;

			if( !IsPartyLevelToEnterGate( pParty->cLevel, (CDnWorld::DungeonGateStruct*)pStruct, &nMin, &nMax ) )
			{
				tstring wszStr;
				if( nMin != -1 ) {
					WCHAR szTemp[256];
					swprintf_s( szTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100053 ), nMin );
					wszStr += szTemp;
					wszStr += L"\n";
				}
				if( nMax != -1 ) {
					WCHAR szTemp[256];
					swprintf_s( szTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100054 ), nMax );
					wszStr += szTemp;
				}
				return wszStr;
			}
		}

		CDnWorld::PermitGateEnum PermitFlag = pStruct->PermitFlag;
		if( pStruct->MapType == CDnWorld::MapTypeDungeon ) 
		{
			if( PermitFlag & CDnWorld::PermitNotRideVehicle ) {
				szCaption += L"\n";
				szCaption += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000085 ); // Cannot enter this area without riding a mount.
				szCaption += L"\n";
				for( DWORD k=0; k<pStruct->cVecNotRideVehicleActorIndex.size(); k++ ) {
					PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pStruct->cVecNotRideVehicleActorIndex[k] );
					if( !pPartyStruct ) continue;
					szCaption += pPartyStruct->wszCharacterName;
					if( k < pStruct->cVecNotRideVehicleActorIndex.size() - 1 ) szCaption += L", ";
				}
			}
		}
	}
	else {
		std::vector<int> nVecItemList;
		pSox->GetItemIDListFromField( "_MapIndex", pTask->GetGameMapIndex(), nVecItemList );
		CDnWorld::PermitGateEnum PermitFlag = pStruct->PermitFlag;
		if( !nVecItemList.empty() ) {
			int nItemID = -1;
			for( DWORD i=0; i<nVecItemList.size(); i++ ) {
				int nTemp = pSox->GetFieldFromLablePtr( nVecItemList[i], "_GateIndex" )->GetInteger();
				if( nTemp == nGateIndex ) {
					nItemID = nVecItemList[i];
					break;
				}
			}

			if( nItemID != -1 ) {
				szCaption += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100026 );

				DNTableFileFormat* pSymbolSox = GetDNTable( CDnTableDB::TITEM );
				if( PermitFlag & CDnWorld::PermitLessPlayerLevel ) {
					szCaption += L"\n- ";
					szCaption += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100029 );
					szCaption += L" : ";

					WCHAR wszTemp[32];
					wsprintf( wszTemp, L"Lv. %3d", pSox->GetFieldFromLablePtr( nItemID, "_PermitPlayerLevel" )->GetInteger() );
					szCaption += wszTemp;

					szCaption += L"\n";
					for( DWORD k=0; k<pStruct->cVecLessLevelActorIndex.size(); k++ ) {
						PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pStruct->cVecLessLevelActorIndex[k] );
						if( !pPartyStruct ) continue;
						szCaption += pPartyStruct->wszCharacterName;
						if( k < pStruct->cVecLessLevelActorIndex.size() - 1 ) szCaption += L", ";
					}
				}
			}
		}
		if( pStruct->MapType == CDnWorld::MapTypeDungeon ) {
			if( PermitFlag & CDnWorld::PermitNotEnoughItem ) {
				szCaption += L"\n- ";
				szCaption += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100028 );
				szCaption += L" : ";

				int nItemNameID = ((CDnWorld::DungeonGateStruct *)pStruct)->nNeedItemID;
				int nItemCount = ((CDnWorld::DungeonGateStruct *)pStruct)->nNeedItemCount;

				WCHAR wszTemp[32];
				swprintf_s( wszTemp, L" (%d)", nItemCount );
				szCaption += CDnItem::GetItemFullName( nItemNameID );
				szCaption += wszTemp;
				szCaption += L" ";

				szCaption += L"\n";
				for( DWORD k=0; k<pStruct->cVecNotEnoughItemActorIndex.size(); k++ ) {
					PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pStruct->cVecNotEnoughItemActorIndex[k] );
					if( !pPartyStruct ) continue;
					szCaption += pPartyStruct->wszCharacterName;
					if( k < pStruct->cVecNotEnoughItemActorIndex.size() - 1 ) szCaption += L", ";
				}
			}
			if( PermitFlag & CDnWorld::PermitLessPlayerLevel ) {
				szCaption += L"\n- ";
				szCaption += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100029 );
				szCaption += L" : ";

				
				WCHAR wszTemp[32];
				wsprintf( wszTemp, L"Lv. %3d", ((CDnWorld::DungeonGateStruct*)pStruct)->nMinLevel );
				szCaption += wszTemp;

				szCaption += L"\n";
				for( DWORD k=0; k<pStruct->cVecLessLevelActorIndex.size(); k++ ) {
					PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pStruct->cVecLessLevelActorIndex[k] );
					if( !pPartyStruct ) continue;
					szCaption += pPartyStruct->wszCharacterName;
					if( k < pStruct->cVecLessLevelActorIndex.size() - 1 ) szCaption += L", ";
				}
			}
			if( PermitFlag & CDnWorld::PermitExceedTryCount ) {
				szCaption += L"\n- ";
				szCaption += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 821 );
				szCaption += L"\n";


				for( DWORD k=0; k<pStruct->cVecExceedTryActorIndex.size(); k++ ) {
					PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pStruct->cVecExceedTryActorIndex[k] );
					if( !pPartyStruct ) continue;
					szCaption += pPartyStruct->wszCharacterName;
					if( k < pStruct->cVecExceedTryActorIndex.size() - 1 ) szCaption += L", ";
				}
			}
			if( PermitFlag & CDnWorld::PermitNotRideVehicle ) {
				szCaption += L"\n- ";
				szCaption += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000085 ); // Cannot enter this area without riding a mount.
				szCaption += L"\n";
				for( DWORD k=0; k<pStruct->cVecNotRideVehicleActorIndex.size(); k++ ) {
					PartyStruct *pPartyStruct = CDnPartyTask::GetInstance().GetPartyData( pStruct->cVecNotRideVehicleActorIndex[k] );
					if( !pPartyStruct ) continue;
					szCaption += pPartyStruct->wszCharacterName;
					if( k < pStruct->cVecNotRideVehicleActorIndex.size() - 1 ) szCaption += L", ";
				}
			}
		}
	}
	return szCaption;
}

bool CDnPartyTask::IsPartyLevelToEnterGate( int nLevel, CDnWorld::DungeonGateStruct *pStruct, int *nRequireMinLevel, int *nRequireMaxLevel )
{
	CDnWorld::DungeonGateStruct *pGateStruct(NULL);
	if( nRequireMinLevel ) *nRequireMinLevel = -1;
	if( nRequireMaxLevel ) *nRequireMaxLevel = -1;

	if( pStruct->pVecMapList.empty() ) {
		if( pStruct->IsEnableLevel( nLevel ) )
			return true;

		if( nRequireMinLevel ) *nRequireMinLevel = pStruct->nMinLevel;
		if( nRequireMaxLevel ) *nRequireMaxLevel = pStruct->nMaxLevel;
	}
	else {
//		bool bEnableLevel = true;
		for( int i=0; i<(int)pStruct->pVecMapList.size(); i++ )
		{
			pGateStruct = (CDnWorld::DungeonGateStruct*)pStruct->pVecMapList[i];
			if( !pGateStruct ) continue;

			if( pGateStruct->IsEnableLevel( nLevel ) ) return true;
			/*
			{
				// If there is no value, it exists only to return, so check this to return True during dungeon entry verification.
				if( nRequireMinLevel == NULL && nRequireMaxLevel == NULL ) return true; 
				continue;
			}
			bEnableLevel = false;
			*/

			if( nRequireMinLevel ) {
				if( *nRequireMinLevel == -1 || *nRequireMinLevel > pGateStruct->nMinLevel )
					*nRequireMinLevel = pGateStruct->nMinLevel;
			}
			if( nRequireMaxLevel ) {
				if( *nRequireMaxLevel == -1 || *nRequireMaxLevel > pGateStruct->nMaxLevel )
					*nRequireMaxLevel = pGateStruct->nMaxLevel;
			}
		}
//		if( bEnableLevel ) return true;
	}
	if( nRequireMinLevel && *nRequireMinLevel != -1 ) {
		if( nLevel >= *nRequireMinLevel ) *nRequireMinLevel = -1;
	}
	if( nRequireMaxLevel && *nRequireMaxLevel != -1 ) {
		if( nLevel <= *nRequireMaxLevel ) *nRequireMaxLevel = -1;
	}

	return false;
}

bool CDnPartyTask::IsPartyCountToEnterGate( int nCount, CDnWorld::DungeonGateStruct *pStruct, int *nRequireMinCount, int *nRequireMaxCount )
{
	CDnWorld::DungeonGateStruct *pGateStruct(NULL);
	if( nRequireMinCount ) *nRequireMinCount = -1;
	if( nRequireMaxCount ) *nRequireMaxCount = -1;

	if( pStruct->pVecMapList.empty() ) { // Dungeon entered directly
		if( pStruct->IsEnablePartyCount( nCount ) )
			return true;

		if( nRequireMinCount ) *nRequireMinCount = pStruct->nMinPartyCount;
		if( nRequireMaxCount ) *nRequireMaxCount = pStruct->nMaxPartyCount;
	}
	else { // Selectable dungeon
		for( int i=0; i<(int)pStruct->pVecMapList.size(); i++ )
		{
			pGateStruct = (CDnWorld::DungeonGateStruct*)pStruct->pVecMapList[i];
			if( !pGateStruct ) continue;

			if( pGateStruct->IsEnablePartyCount( nCount ) )
				return true;

			if( nRequireMinCount ) {
				if( *nRequireMinCount == -1 || *nRequireMinCount > pGateStruct->nMinPartyCount )
					*nRequireMinCount = pGateStruct->nMinPartyCount;
			}
			if( nRequireMaxCount ) {
				if( *nRequireMaxCount == -1 || *nRequireMaxCount > pGateStruct->nMaxPartyCount )
					*nRequireMaxCount = pGateStruct->nMaxPartyCount;
			}
		}
	}

	return false;
}

bool CDnPartyTask::IsPartyMemberAlive()
{
	CDnPlayerActor *pPlayerActor(NULL);

	if( m_PartyRole == PartyRoleEnum::SINGLE ) {
		if( CDnActor::s_hLocalActor ) {
			pPlayerActor = static_cast<CDnPlayerActor*>((CDnActor*)CDnActor::s_hLocalActor.GetPointer());
			return !pPlayerActor->IsGhost();
		}
		return false;
	}

	for( int i=0; i<(int)m_VecPartyList.size(); i++ )
	{
		pPlayerActor = static_cast<CDnPlayerActor*>((CDnActor*)m_VecPartyList[i].hActor);
		if( !pPlayerActor ) continue;
		if( GetPartyDataFromSessionID( pPlayerActor->GetUniqueID() ) && GetPartyDataFromSessionID( pPlayerActor->GetUniqueID() )->bGMTrace ) continue;

		if( !pPlayerActor->IsGhost() )
		{
			return true;
		}
	}

	return false;
}

#ifdef PRE_PARTY_DB
void CDnPartyTask::ReqPartyInfoModify(BYTE cPartyMemberMax, TPARTYITEMLOOTRULE cLootRule, TITEMRANK cLootRank, int nUserLvLimitMin, const WCHAR * pPartyName, int iPassword, int mapIdx, TDUNGEONDIFFICULTY difficulty, bool bAllowWorldZoneMapList)
{
	SendPartyInfoModify(cPartyMemberMax, cLootRule, cLootRank, nUserLvLimitMin, pPartyName, iPassword, mapIdx, difficulty, IsJobDice(), bAllowWorldZoneMapList);
}
#else
void CDnPartyTask::ReqPartyInfoModify(BYTE cPartyMemberMax, BYTE cLootRule, BYTE cLootRank, int nUserLvLimitMin, int nUserLvLimitMax, const WCHAR * pPartyName, const WCHAR * pPass, int mapIdx, int difficulty)
{
	// Check if party leader
	//
	SendPartyInfoModify(cPartyMemberMax, cLootRule, cLootRank, nUserLvLimitMin, nUserLvLimitMax, pPartyName, pPass, mapIdx, difficulty, IsJobDice());
}
#endif

void CDnPartyTask::ReqPartyRequestReady()
{
	SendReadyRequest();
}

void CDnPartyTask::ReqPartySwapMemberIndex(const std::vector<SRaidReGroupInfo>& members)
{
	std::vector<TSwapMemberIndex> memberIdxList;
	int index = 0;
	std::vector<SRaidReGroupInfo>::const_iterator iter = members.begin();
	for (; iter != members.end(); ++iter)
	{
		const SRaidReGroupInfo& info = (*iter);

		std::vector<SRaidReGroupCtrlUnit>::const_iterator unitIter = info.partyCtrlList.begin();
		for (; unitIter != info.partyCtrlList.end(); ++unitIter)
		{
			const SRaidReGroupCtrlUnit& unit = *unitIter;

			TSwapMemberIndex memberIdx;
			memberIdx.cIndex = index;
			//memberIdx.nSessionID = (unit.sessionId < 0) ? 0 : unit.sessionId;
			memberIdx.nSessionID = unit.sessionId;

			memberIdxList.push_back(memberIdx);
			index++;
		}
	}

	if (memberIdxList.size() > 0)
		SendPartySwapMemberIdx(memberIdxList);
}

void CDnPartyTask::ReqPartyItemLootRuleModify(ePartyItemLootRule lootRule)
{
#ifdef PRE_PARTY_DB
	SendPartyInfoModify((BYTE)GetMaxPartyCount(), lootRule, GetPartyLootItemRank(), GetMinUserLevel(), GetPartyName(), GetPartyPassword(), GetPartyTargetStageIdx(), GetPartyTargetStageDifficulty(), IsJobDice(), IsPartyAllowWorldZoneMapList());
#else
	SendPartyInfoModify((BYTE)GetMaxPartyCount(), (BYTE)lootRule, GetPartyLootItemRank(), GetMinUserLevel(), GetMaxUserLevel(), GetPartyName(), GetPartyPassword(), GetPartyTargetStageIdx(), GetPartyTargetStageDifficulty(), IsJobDice());
#endif
}

void CDnPartyTask::ReqPartyItemLootRankModify(eItemRank lootRank)
{
#if defined( PRE_PARTY_DB )
	SendPartyInfoModify((BYTE)GetMaxPartyCount(), GetPartyLootRule(), lootRank, GetMinUserLevel(), GetPartyName(), GetPartyPassword(), GetPartyTargetStageIdx(), GetPartyTargetStageDifficulty(), IsJobDice(), IsPartyAllowWorldZoneMapList());
#else
	SendPartyInfoModify((BYTE)GetMaxPartyCount(), GetPartyLootRule(), lootRank, GetMinUserLevel(), GetMaxUserLevel(), GetPartyName(), GetPartyPassword(), GetPartyTargetStageIdx(), GetPartyTargetStageDifficulty(), IsJobDice());
#endif // #if defined( PRE_PARTY_DB )
}

#ifdef PRE_PARTY_DB
void CDnPartyTask::ReqPartyAllowWorldZoneMapList(bool bAllow)
{
	SendPartyInfoModify((BYTE)GetMaxPartyCount(), GetPartyLootRule(), GetPartyLootItemRank(), GetMinUserLevel(), GetPartyName(), GetPartyPassword(), GetPartyTargetStageIdx(), GetPartyTargetStageDifficulty(), IsJobDice(), bAllow);
}
#endif

void CDnPartyTask::ReqPartyJoinGetReversionItem(bool bJoin)
{
	SendPartyJoinGetReversionItem(bJoin);
}

void CDnPartyTask::ReqPartyDungeonDifficultyInfo(int mapIdx)
{
	SendVerifyDungeon(mapIdx);
}

void CDnPartyTask::ReqPartyAbsence(bool bAbsence)
{
	SendUserAbsent(bAbsence);
}

bool CDnPartyTask::IsUserLevelLimit() const
{
	int nMinLevel = GetMinUserLevel();
#if defined( PRE_PARTY_DB )
	if( nMinLevel == 1  )
	{
		return true;
	}
#else
	int nMaxLevel = GetMaxUserLevel();

	if( nMinLevel == 1 && nMaxLevel == 100 )
	{
		return true;
	}
#endif // #if defined( PRE_PARTY_DB )

	return false;
}

bool CDnPartyTask::IsLocalActorMaster() const
{
	return (GetMasterSessionID() == CDnBridgeTask::GetInstance().GetSessionID());
}

bool CDnPartyTask::IsPartyMember( UINT sessionID )
{
	for( int i=0; i<(int)m_VecPartyList.size(); i++ )
	{
		if( m_VecPartyList[i].nSessionID == sessionID )
			return true;
	}

	return false;
}

bool CDnPartyTask::IsPartyMember( const wchar_t* name)
{
	for( int i=0; i<(int)m_VecPartyList.size(); i++ )
	{
		if( !_tcscmp(m_VecPartyList[i].wszCharacterName, name) )
			return true;
	}

	return false;
}


bool CDnPartyTask::IsPvpGame()
{
	CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
	return( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP );
}

bool CDnPartyTask::IsPartyMemberEnableStartGame(UINT sessionID) const
{
	for( int i=0; i<(int)m_VecPartyList.size(); i++ )
	{
		if( m_VecPartyList[i].nSessionID == sessionID )
			return (m_VecPartyList[i].wFatigue > 0);
	}

	return false;
}

CDnPartyTask::eErrorPartyStartGame CDnPartyTask::IsPartyEnableStartGame( char cGateIndex ) const
{
	return ENABLE;
}

bool CDnPartyTask::IsPartyFull() const
{
	int count = int(m_VecPartyList.size());
	if (count > PARTYCOUNTMAX)
	{
		_ASSERT(0&&_T("PARTY MEMBER COUNT ERROR!!"));
		return true;
	}
	else if (count == PARTYCOUNTMAX)
	{
		return true;
	}

	return false;
}

bool CDnPartyTask::IsPartyInfoEmpty() const
{
	if (GetMaxPartyCount() == 0 &&
		m_szPartyName == _T(""))
		return true;

	return false;
}

bool CDnPartyTask::IsPartyInfoModified(int newMaxPartyCount, 
									   int newMinLevel, 
#if defined( PRE_PARTY_DB )
#else
									   int newMaxLevel, 
#endif // #if defined( PRE_PARTY_DB )
									   const std::wstring& newPartyName, 
#ifdef PRE_PARTY_DB
									   int iPassword
#else
									   const std::wstring& newPassword
#endif
									   , int newTargetStageIdx
#ifdef PRE_PARTY_DB
									   , TDUNGEONDIFFICULTY newTargetStageDifficulty
#else
									   , int newTargetStageDifficulty
#endif
									   ) const
{
	if ((GetMaxPartyCount() != newMaxPartyCount) ||
		(GetMinUserLevel() != newMinLevel) ||
#if defined( PRE_PARTY_DB )
#else
		(GetMaxUserLevel() != newMaxLevel) ||
#endif // #if defined( PRE_PARTY_DB )
		(newPartyName != GetPartyName()) ||
#ifdef PRE_PARTY_DB
		(iPassword != GetPartyPassword())
#else
		(newPassword != GetPartyPassword())
#endif
		|| (newTargetStageIdx != GetPartyTargetStageIdx())
		|| (newTargetStageDifficulty != GetPartyTargetStageDifficulty())
		)
		return true;

	return false;
}

bool CDnPartyTask::IsPartyLootRuleModified(ePartyItemLootRule newRule) const
{
	if (newRule <= ITEMLOOTRULE_NONE ||
		newRule >= ITEMLOOTRULE_MAX)
	{
		//_ASSERT(0);
		return false;
	}
	return (GetPartyLootRule() != newRule);
}

bool CDnPartyTask::IsPartyLootRankModified(eItemRank newRank) const
{
#ifdef PRE_PARTY_DB
	if (newRank == ITEMRANK_MAX)
		return false;
#else
	if (newRank == ITEMRANK_SS || newRank == ITEMRANK_MAX )
		return false;
#endif

	return (GetPartyLootItemRank() != newRank);
}

#ifdef PRE_PARTY_DB
bool CDnPartyTask::IsPartyAllowWorldZoneMapListModified(bool bAllow) const
{
	return (bAllow != IsPartyAllowWorldZoneMapList());
}
#endif

bool CDnPartyTask::IsPartyTargetStageIdxModified(int mapIdx) const
{
	return (mapIdx != GetPartyTargetStageIdx());
}

bool CDnPartyTask::IsPartyTargetStageDifficultyModified(int newDifficulty) const
{
	return (newDifficulty != GetPartyTargetStageDifficulty());
}

void CDnPartyTask::LockPartyReqPartyMemberInfo(bool bLock)
{
	m_bLockReqPartyMemberInfo = bLock;
}

#ifdef PRE_PARTY_DB
void CDnPartyTask::LockPartyReqPartyList(bool bLock)
{
	m_bLockReqPartyList = bLock;
}
#endif

void CDnPartyTask::LockBreakIntoPartyList( bool bLock )
{
	if( bLock ) m_BreakIntoPartyListLock.Lock();
	else m_BreakIntoPartyListLock.UnLock();
}

void CDnPartyTask::AddBreakIntoPartyList( PartyStruct &Struct ) 
{
	ScopeLock<CSyncLock> Lock( m_BreakIntoPartyListLock );
	m_BreakIntoPartyList.push_back( Struct );
}

void CDnPartyTask::UpdateBreakIntoPartyList()
{
	ScopeLock<CSyncLock> Lock( m_BreakIntoPartyListLock );

	for( std::list<PartyStruct>::iterator itor=m_BreakIntoPartyList.begin() ; itor!=m_BreakIntoPartyList.end() ; ++itor )
	{
		if( (*itor).hActor )
		{
			if( InterlockedDecrement( &(*itor).hActor->m_lBreakIntoLoadingFlag ) > 0 )
			{
				(*itor).hActor->m_lBreakIntoLoadingFlag = 0;
				CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
				if( pTask ) 
					pTask->RefreshConnectUser();
				return;
			}
			else
			{
				AddPartyList( (*itor) );

				CDnGameTask *pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
				if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP )
				{
					if( (*itor).hActor )
					{
						(*itor).hActor->Show( !(*itor).bGMTrace );
						(*itor).hActor->SetProcess( !(*itor).bGMTrace );
					}
				}

			}
		}
		else if((*itor).usTeam == PvPCommon::Team::Observer)
		{
			AddPartyList( (*itor) );
		}
	}
	m_BreakIntoPartyList.clear();
}

void CDnPartyTask::DeleteBreakIntoPartyList( UINT iSessionID )
{
	ScopeLock<CSyncLock> Lock( m_BreakIntoPartyListLock );

	for( std::list<PartyStruct>::iterator itor=m_BreakIntoPartyList.begin() ; itor!=m_BreakIntoPartyList.end() ; ++itor )
	{
		if( (*itor).nSessionID == iSessionID )
		{
			m_BreakIntoPartyList.erase( itor );
			break;
		}
	}
}


void CDnPartyTask::AddPartyList( PartyStruct &Struct ) 
{ 
	// Exception handling when someone intrudes in PartyRoleEnum::SINGLE state
	if( m_PartyRole == PartyRoleEnum::SINGLE )
	{
		// 1. Remember that it was changed due to intrusion
		m_PrevPartyRole = PartyRoleEnum::SINGLE;
		// 2. Reset party info
		m_VecPartyList.clear();
		// 3. Set party leader
		m_PartyRole = PartyRoleEnum::LEADER;
		// 4. Copy local info
		m_VecPartyList.push_back( m_LocalData );

		m_PartyState = ePartyState::NORMAL;

		if( Struct.bGMTrace )
			m_bSingleToPartyByGMTrace = true;
	}

	m_VecPartyList.push_back(Struct);

	OnPartyMemberNewAdded(PARTYREFRESH_MODIFY_INFO, Struct);	// todo by kalliste : add the type which having new member to ePartyRefreshSubject
	GetInterface().RefreshPartyInfoList();
	GetInterface().RefreshPartyGuage();	
#ifdef PRE_ADD_IMPROVE_MASTER_OBSERVER
	GetInterface().RefreshPvpPlayerGauge();
#endif

	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );

	if( pTask && pTask->GetGameTaskType() == GameTaskType::PvP )
	{
		if(!m_VecPartyList.empty() && !Struct.bGMTrace && Struct.usTeam && (Struct.usTeam == PvPCommon::Team::Observer))
		{
			if( !CDnBridgeTask::GetInstance().IsPvPGameMode( PvPCommon::GameMode::PvP_GuildWar ) )
			{
				WCHAR wszMessage[256];
				SecureZeroMemory(wszMessage,sizeof(wszMessage));

				wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 121087 ),Struct.wszCharacterName);
				GetInterface().AddMessageText( wszMessage , textcolor::WHITE );
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", wszMessage);
			}
		}
	} 
}

void CDnPartyTask::SetPartyState(ePartyState state)
{
	m_PartyState = state;
}

const wchar_t* CDnPartyTask::GetPartyLootRuleString(ePartyItemLootRule rule) const
{
	if (rule <= ITEMLOOTRULE_NONE || rule >= ITEMLOOTRULE_MAX)
		return NULL;

	// todo : Refactoring
	switch(rule)
	{
	case ITEMLOOTRULE_LEADER:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3543);	// UISTRING : The party leader receives the item.

	case ITEMLOOTRULE_INORDER:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3544);	// UISTRING : Party members take turns receiving items.

	case ITEMLOOTRULE_RANDOM:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3545);	// UISTRING : Acquired items are distributed randomly.

	case ITEMLOOTRULE_OWNER:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3546);	// UISTRING : Keep acquired items for yourself.
	}

	return NULL;
}

const wchar_t* CDnPartyTask::GetPartyLootItemRankString(eItemRank rank) const
{
	switch(rank)
	{
	case ITEMRANK_D:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3570);

	case ITEMRANK_C:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3571);

	case ITEMRANK_B:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3572);

	case ITEMRANK_A:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3573);

	case ITEMRANK_S:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3574);

#ifdef PRE_PARTY_DB
	case ITEMRANK_SS:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3457);

	case ITEMRANK_NONE:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3458); // UISTRING : Do not roll dice
#else
	case ITEMRANK_NONE:
		return GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3575);
#endif
	}

	return NULL;
}

bool CDnPartyTask::SetPartyStageIdxForSort(int idx)
{
	if (idx < 0)
		idx = 0;

#ifdef PRE_PARTY_DB
	if (m_nStageIdxListForSort.size() >= Party::Max::DetailSearchMapCount)
		m_nStageIdxListForSort.clear();

	m_nStageIdxListForSort.push_back(idx);
#else
	if (m_nStageIdxListForSort.size() >= PARTYSORTCONDITIONMAX)
		return false;

	m_nStageIdxListForSort.push_back(idx);
#endif

	return true;
}

void CDnPartyTask::SetPartyStageDifficultyForSort(int difficulty)
{
	if (difficulty < 0 || difficulty > Dungeon::Difficulty::Max)
		difficulty = 0;

	m_nStageDifficultyForSort = difficulty;
}

void CDnPartyTask::SetPartyTargetStageIdx(int idx)
{
	if (idx < 0)
		idx = 0;
	m_nTargetStageIdx = idx;
}

void CDnPartyTask::SetPartyTargetStageDifficulty(int difficulty)
{
	if (CommonUtil::IsValidDungeonDifficulty((TDUNGEONDIFFICULTY)difficulty))
		m_TargetStageDifficulty = Dungeon::Difficulty::Easy;
	else
		m_TargetStageDifficulty = (TDUNGEONDIFFICULTY)difficulty;
}

void CDnPartyTask::GetPartyTargetStageString(std::wstring& result)
{
	result.clear();
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if (pSox != NULL)
	{
		int nStringTableID = pSox->GetFieldFromLablePtr(m_nTargetStageIdx, "_MapNameID")->GetInteger();
		result = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringTableID);
	}
}

void CDnPartyTask::GetPartyTargetStageDifficultyString(std::wstring& result)
{
#ifdef PRE_PARTY_DB
	DN_INTERFACE::STRING::GetStageDifficultyText(result, m_TargetStageDifficulty);
#else
	DN_INTERFACE::STRING::GetStageDifficultyText(result, m_nTargetStageDifficulty);
#endif
}

void CDnPartyTask::ReqPartyJobDiceModified(bool bJobDice)
{
#if defined( PRE_PARTY_DB )
	SendPartyInfoModify((BYTE)GetMaxPartyCount(), GetPartyLootRule(), GetPartyLootItemRank(), GetMinUserLevel(), GetPartyName(), GetPartyPassword(), GetPartyTargetStageIdx(), GetPartyTargetStageDifficulty(), bJobDice ? 1 : 0, IsPartyAllowWorldZoneMapList());
#else
	SendPartyInfoModify((BYTE)GetMaxPartyCount(), GetPartyLootRule(), GetPartyLootItemRank(), GetMinUserLevel(), GetMaxUserLevel(), GetPartyName(), GetPartyPassword(), GetPartyTargetStageIdx(), GetPartyTargetStageDifficulty(), bJobDice ? 1 : 0);
#endif // #if defined( PRE_PARTY_DB )
}

#ifdef PRE_PARTY_DB

void CDnPartyTask::ReqPartyListInfo(int currentPage, const std::wstring& searchWord)
{
	std::wstring searchWordToLower(searchWord);
	ToLowerW(searchWordToLower);

	CDnPartyListDlg* pPartyListDlg = NULL;
	CDnCommunityDlg *pCommunityDlg = static_cast<CDnCommunityDlg*>(CDnInterface::GetInstance().GetMainMenuDialog(CDnMainMenuDlg::COMMUNITY_DIALOG));
	if (pCommunityDlg)
		pPartyListDlg = pCommunityDlg->GetPartyListDialog();
	
	if (pPartyListDlg == NULL)
		return;

#ifdef PRE_FIX_61545
	SendReqPartyListInfo(currentPage, searchWordToLower.c_str(), (TDUNGEONDIFFICULTY)GetPartyStageDifficultyForSort(), pPartyListDlg->GetCurrentOrderType(), 
		pPartyListDlg->GetCurrentStageOrderType(), pPartyListDlg->IsOnlySameVillage(), CommonUtil::GetPartySearchWordSeperator());
#else
	SendReqPartyListInfo(currentPage, searchWordToLower.c_str(), (TDUNGEONDIFFICULTY)GetPartyStageDifficultyForSort(), pPartyListDlg->GetCurrentOrderType(), pPartyListDlg->GetCurrentStageOrderType(), pPartyListDlg->IsOnlySameVillage());
#endif

	LockPartyReqPartyList(true);
}

void CDnPartyTask::ResetPartyList()
{
	ReqPartyListInfo(0, L"");
}
#else
void CDnPartyTask::ReqPartyListInfo(int currentPage, bool bSort)
{
	int nMapIdx[PARTYSORTCONDITIONMAX];
	memset(nMapIdx, 0, sizeof(nMapIdx));

	if (m_nStageIdxListForSort.empty() == false)
	{
		UINT i = 0;
		for (; i < PARTYSORTCONDITIONMAX; ++i)
		{
			if (i >= m_nStageIdxListForSort.size())
				break;
			nMapIdx[i] = m_nStageIdxListForSort[i];
		}
	}

	SendReqPartyListInfo(currentPage, nMapIdx, GetPartyStageDifficultyForSort(), bSort);
}
#endif

void CDnPartyTask::OnRecvPartyKeepBonus( SCPartyBonusValue *pPacket )
{
#ifdef PRE_ADD_BEGINNERGUILD
	RefreshKeepPartyInfo( pPacket->cUpkeepBonus, pPacket->cFriendBonus, ( pPacket->cUpKeepBonusLimit > 0 ) ? true : false, pPacket->cBeginnerGuildBonus );
#else
	RefreshKeepPartyInfo( pPacket->cUpkeepBonus, pPacket->cFriendBonus, ( pPacket->cUpKeepBonusLimit > 0 ) ? true : false );
#endif
}

#ifdef PRE_ADD_BEGINNERGUILD
void CDnPartyTask::RefreshKeepPartyInfo( int nKeepPartyBonusRate, int nBestFriendBonusRate, bool bLimitKeepPartyBonus, int nBeginnerGuildBonusRate )
#else
void CDnPartyTask::RefreshKeepPartyInfo( int nKeepPartyBonusRate, int nBestFriendBonusRate, bool bLimitKeepPartyBonus )
#endif
{
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && (pGameTask->GetGameTaskType() == GameTaskType::Farm || pGameTask->GetGameTaskType() == GameTaskType::PvP) )
		return;

	int nUIStringIndex = -1;
	if( m_nKeepPartyBonusRate == 0 && nKeepPartyBonusRate > 0 ) nUIStringIndex = 100068;
	else if( m_nKeepPartyBonusRate > 0 && nKeepPartyBonusRate == 0 ) nUIStringIndex = 100065;
	else if( m_nKeepPartyBonusRate > 0 && nKeepPartyBonusRate > 0 ) {
		if( bLimitKeepPartyBonus ) nUIStringIndex = 100069;
		else if( m_nKeepPartyBonusRate < nKeepPartyBonusRate ) nUIStringIndex = 100064;
	}
	if( nUIStringIndex != -1 )
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUIStringIndex), false );

	nUIStringIndex = -1;
	if( m_nBestFriendBonusRate > 0 && nBestFriendBonusRate == 0 ) nUIStringIndex = 100067;
	else if( m_nBestFriendBonusRate == 0 && nBestFriendBonusRate > 0 ) nUIStringIndex = 100066;
	if( nUIStringIndex != -1 )
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUIStringIndex), false );

	m_nKeepPartyBonusRate = nKeepPartyBonusRate;
	m_nBestFriendBonusRate = nBestFriendBonusRate;

	if( CDnInterface::IsActive() && GetInterface().GetPlayerGauge() )
#ifdef PRE_ADD_BEGINNERGUILD
		GetInterface().GetPlayerGauge()->UpdateKeepPartyInfo( nKeepPartyBonusRate, nBestFriendBonusRate, nBeginnerGuildBonusRate );
#else
		GetInterface().GetPlayerGauge()->UpdateKeepPartyInfo( nKeepPartyBonusRate, nBestFriendBonusRate );
#endif
}

void CDnPartyTask::OnRecvPartyInviteNotice(SCPartyInviteNotice* pPacket)
{
	if (pPacket->nRetCode != ERROR_NONE)
	{
		if (m_PartyRole == PartyRoleEnum::LEADER)
		{
			GetInterface().ServerMessageBox(pPacket->nRetCode);
		}
		else
		{
			std::wstring str;
			CDnInterface::GetInstance().GetServerMessage(str, pPacket->nRetCode);
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);
		}

		GetInterface().CloseCaptionDialog(CDnInterface::typeCaption5);
		m_bPartyMemberInviting = false;
		return;
	}

	GetInterface().ShowCaptionDialog(CDnInterface::typeCaption5, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3419), textcolor::YELLOW); // UISTRING : Summoning party members
	GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3419), false);
	m_bPartyMemberInviting = true;
}

void CDnPartyTask::OnRecvPartyAskJoinResult(SCPartyAskJoinResult * pPacket)
{
	//Result value for a normal party request. Mostly for failures or errors.
	if (pPacket == NULL || pPacket->nRet == ERROR_NONE)
	{
		_ASSERT(0);
		return;
	}	
	GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnPartyTask::OnRecvPartyAskJoinToLeader(SCPartyAskJoinToLeader * pPacket)
{
	//Someone wants to join the party.
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	CDnPartyAcceptRequestAskInfo info;
	info.m_szOtherSideName = pPacket->wszAskChracterName;
	info.m_JobId = pPacket->cJob;
	info.m_Level = pPacket->cLevel;

	// Info must be entered into AcceptRequestInfo here so that CDnAcceptRequestDlg inside CDnInterface sets info via SetInfo() and calls Show( true ) on Dlg 
	// true means CDnAcceptRequestDlg is shown and the accept/decline dialog is open. 
	// On the next frame, Process runs and calls GetPartyTask().OpenAcceptRequestDialog() to send an accept/decline message box to the party leader. 
	// Then when the party leader accepts, calling the Ok button of CDnAcceptRequestDlg in Interface triggers EVENT_BUTTON_CLICKED, so 

	m_AcceptRequestAskInfo.AddAcceptRequestInfo(info);


#else
	SAskInfo info;
	info.szApplicantName = pPacket->wszAskChracterName;
	info.jobId = pPacket->cJob;
	info.level = pPacket->cLevel;

	m_listAskInfo.push_back(info);
#endif

}

void CDnPartyTask::OnRecvPartyAskJoinDecisionInfo(SCPartyAskJoinDecisionInfo * pPacket)
{
	//When this message arrives, join using the packet info.
#ifdef PRE_PARTY_DB
	ReqJoinParty(pPacket->PartyID, pPacket->iPassword);
#else
	ReqJoinParty(pPacket->PartyID, pPacket->wszPartyPassword);
#endif
}

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
void CDnPartyTask::OnRecvPartyAskInfo(SCPartyInfo *pPacket)
{
	if( ! pPacket && ! GetInterface().GetDirectDlg() )
		return;

	GetPartyTask().ReqPartyMemberInfo( pPacket->PartyInfo.PartyID );
	GetInterface().GetDirectDlg()->OnRecvPartyInfo( pPacket->PartyInfo );	
}
#endif

void CDnPartyTask::OnRecvPartyUpdateFieldUI(SCPartyUpdateFieldUI * pPacket)
{
	//For updating UI on whether an actor in sight on the village field is the party leader
	//Not in a party in village, but intended for future use if party status etc. are added.

	// Added validity check because session ID might arrive invalid.
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID(pPacket->nSessionID);
	if( hActor && hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		pActor->SetPartyLeader((pPacket->cLeader <= 0) ? false : true);
	}
}

#ifdef PRE_PARTY_DB
const std::vector<SPartyTargetMapInfo>& CDnPartyTask::MakeWholePartyTargetStageList()
{
	if (m_PartyWholeTargetMapCache.empty())
	{
		m_PartyWholeTargetMapLevelLimitCache.clear();

		const std::vector<int>& villageMapIndexList = CDnWorldDataManager::GetInstance().GetVillageMapIndexList();
		std::vector<int>::const_iterator iter = villageMapIndexList.begin();
		for (; iter != villageMapIndexList.end(); ++iter)
		{
			const int& curMapId = (*iter);
			MakePartyTargetStageList(curMapId, m_PartyWholeTargetMapCache, false);
			MakePartyTargetStageLevelLimitList(m_PartyWholeTargetMapLevelLimitCache, m_PartyWholeTargetMapCache, false);
		}
	}

	return m_PartyWholeTargetMapCache;
}

const std::map<int, int>& CDnPartyTask::MakeWholePartyTargetStageLevelLimitList()
{
	if (m_PartyWholeTargetMapLevelLimitCache.empty())
		MakeWholePartyTargetStageList();

	return m_PartyWholeTargetMapLevelLimitCache;
}
#endif

#ifdef PRE_PARTY_DB
void CDnPartyTask::MakePartyTargetStageList(int currentMapIndex, std::vector<SPartyTargetMapInfo>& stageNameList, bool bClearList)
#else
void CDnPartyTask::MakePartyTargetStageList(int currentMapIndex, std::vector<SPartyTargetMapInfo>& stageNameList, std::map<int, int>& stageLevelLimitList)
#endif
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if (pSox != NULL)
	{
#ifdef PRE_PARTY_DB
		if (bClearList)
			stageNameList.clear();
#else
		stageNameList.clear();
#endif
		DNTableFileFormat* pDungeonEnterSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
		if( !pDungeonEnterSox ) return;

		std::string areaString = pSox->GetFieldFromLablePtr(currentMapIndex, "_MapArea")->GetString();

		if (areaString.empty() == false)
		{
			std::vector<std::string> tokens;
			TokenizeA(areaString, tokens, ";");

			std::vector<std::string>::const_iterator iter = tokens.begin();
			for (; iter != tokens.end(); ++iter)
			{
				const std::string& dgIDString = (*iter);
				int dgid = atoi(dgIDString.c_str());

				const std::vector<CDnWorldDataManager::SMapAreaInfoUnit>* pMapAreaList = CDnWorldDataManager::GetInstance().GetMapAreaList(dgid);
				if (pMapAreaList != NULL)
				{
					std::vector<CDnWorldDataManager::SMapAreaInfoUnit>::const_iterator areaIter = pMapAreaList->begin();
					for (; areaIter != pMapAreaList->end(); ++areaIter)
					{
						const CDnWorldDataManager::SMapAreaInfoUnit& mapAreaInfoUnit = *areaIter;
						SPartyTargetMapInfo info;
						info.mapName	= GetEtUIXML().GetUIString( CEtUIXML::idCategory1, mapAreaInfoUnit.mapNameId );
						info.mapId		= mapAreaInfoUnit.mapId;

#ifdef PRE_PARTY_DB
						std::vector<SPartyTargetMapInfo>::const_iterator iter = std::find(stageNameList.begin(), stageNameList.end(), info);
						if (iter != stageNameList.end())
							continue;
#endif
						
						int nDungeonEnterTableID = pSox->GetFieldFromLablePtr(mapAreaInfoUnit.mapId, "_EnterConditionTableID")->GetInteger();
						if (nDungeonEnterTableID > 0 && pDungeonEnterSox->IsExistItem(nDungeonEnterTableID))
						{
							info.minPartyNum = pDungeonEnterSox->GetFieldFromLablePtr(nDungeonEnterTableID, "_PartyOneNumMin")->GetInteger();
							info.maxPartyNum = pDungeonEnterSox->GetFieldFromLablePtr(nDungeonEnterTableID, "_PartyOneNumMax")->GetInteger();
						}
						else
						{
							info.minPartyNum = info.maxPartyNum = -1;
						}

#ifdef PRE_PARTY_DB
						info.filterType = eTARGETFILTER_ALL;
						CDnWorld::MapTypeEnum mapType = (CDnWorld::MapTypeEnum)pSox->GetFieldFromLablePtr(mapAreaInfoUnit.mapId, "_MapType")->GetInteger();
						CDnWorld::MapSubTypeEnum MapSubType = (CDnWorld::MapSubTypeEnum)pSox->GetFieldFromLablePtr(mapAreaInfoUnit.mapId, "_MapSubType")->GetInteger();
						if (mapType == CDnWorld::MapTypeDungeon)
						{
							if (MapSubType == CDnWorld::MapSubTypeNest || MapSubType == CDnWorld::MapSubTypeNestNormal)
								info.filterType = eTARGETFILTER_NEST;
#ifdef PRE_ADD_PARTYTARGETMAPTOGGLE
							else if (MapSubType == CDnWorld::MapSubTypeDarkLair || MapSubType == CDnWorld::MapSubTypeChaosField || MapSubType == CDnWorld::MapSubTypeEvent)
								info.filterType = eTARGETFILTER_ETC;
#endif
#ifdef PRE_ADD_DRAGON_FELLOWSHIP
							else if (MapSubType == CDnWorld::MapSubTypeFellowship)
								info.filterType = eTARGETFILTER_ETC;
#endif
							else if (MapSubType == CDnWorld::MapSubTypeNone
								  || MapSubType == CDnWorld::MapSubTypeTreasureStage )
								info.filterType = eTARGETFILTER_DUNGEON;
						}
#endif

						stageNameList.push_back(info);
					}
				}
			}
			std::sort(stageNameList.begin(), stageNameList.end());

#ifdef PRE_PARTY_DB
#else
			stageLevelLimitList.clear();
			std::vector<SPartyTargetMapInfo>::const_iterator nameIter = stageNameList.begin();
			for (; nameIter != stageNameList.end(); ++nameIter)
			{
				const SPartyTargetMapInfo& mapAreaInfoUnit = (*nameIter);
				int nDungeonEnterTableID = pSox->GetFieldFromLablePtr(mapAreaInfoUnit.mapId, "_EnterConditionTableID")->GetInteger();
				if (nDungeonEnterTableID > 0)
				{
					if (pDungeonEnterSox->IsExistItem(nDungeonEnterTableID))
					{
						int limitLevel = pDungeonEnterSox->GetFieldFromLablePtr(nDungeonEnterTableID, "_LvlMin")->GetInteger();
						if (limitLevel > 0 && limitLevel < 100)
							stageLevelLimitList.insert(std::make_pair(mapAreaInfoUnit.mapId, limitLevel));
						else
							_ASSERT(0);
					}
				}
			}
#endif
		}
	}
}

#ifdef PRE_PARTY_DB
void CDnPartyTask::MakePartyTargetStageLevelLimitList(std::map<int, int>& stageLevelLimitList, std::vector<SPartyTargetMapInfo>& stageNameList, bool bClearList)
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if (pSox == NULL)
		return;

	DNTableFileFormat* pDungeonEnterSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
	if (pDungeonEnterSox == NULL)
		return;

	if (bClearList)
		stageLevelLimitList.clear();
	std::vector<SPartyTargetMapInfo>::const_iterator nameIter = stageNameList.begin();
	for (; nameIter != stageNameList.end(); ++nameIter)
	{
		const SPartyTargetMapInfo& mapAreaInfoUnit = (*nameIter);
		int nDungeonEnterTableID = pSox->GetFieldFromLablePtr(mapAreaInfoUnit.mapId, "_EnterConditionTableID")->GetInteger();
		if (nDungeonEnterTableID > 0)
		{
			if (pDungeonEnterSox->IsExistItem(nDungeonEnterTableID))
			{
				int limitLevel = pDungeonEnterSox->GetFieldFromLablePtr(nDungeonEnterTableID, "_LvlMin")->GetInteger();
				if (limitLevel > 0 && limitLevel < 100)
					stageLevelLimitList.insert(std::make_pair(mapAreaInfoUnit.mapId, limitLevel));
				else
					_ASSERT(0);
			}
		}
	}
}


bool CDnPartyTask::IsSecretParty() const
{
	return (m_iPartyPassword != Party::Constants::INVALID_PASSWORD);
}
#endif

void CDnPartyTask::SetAdvancedSearching(bool bSet)
{
	m_bAdvanceSearching = bSet;
	if (m_bAdvanceSearching)
		CDnInterface::GetInstance().ResetPartyListSort();
}

void CDnPartyTask::OnRecvPartyMemberPart( SCPartyMemberPart *pPacket )
{
	if( !CDnInterface::IsActive() ) return;
	CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyDataFromSessionID( pPacket->nSessionID );
	if( !pStruct ) return;

	pStruct->cLevel = pPacket->cLevel;
	pStruct->nExp = pPacket->nExp;
	memcpy( pStruct->cJobArray, pPacket->cJobArray, sizeof(pStruct->cJobArray) );
	pStruct->nHairID = pPacket->nHairID;
	pStruct->nFaceID = pPacket->nFaceID;
	pStruct->dwHairColor = pPacket->dwHairColor;
	pStruct->dwSkinColor = pPacket->dwSkinColor;
	pStruct->dwEyeColor = pPacket->dwEyeColor;
	pStruct->wFatigue = pPacket->wFatigue;
	pStruct->nHelmetID = pPacket->nHelmetID;
	pStruct->nEarringID = pPacket->nEarringID;

	CDnInterface::GetInstance().RefreshPartyInfoList();
	CDnInterface::GetInstance().RefreshPartyGuage();

	CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
	if( pMainMenuDlg ) pMainMenuDlg->RefreshZoneMapPartyActor();
}

bool CDnPartyTask::IsPartyType(ePartyType type) const
{
	return (m_PartyType == type);
}

bool CDnPartyTask::IsGMTracing()
{
	if (!CDnActor::s_hLocalActor)
		return false;

	DWORD mySessionId = CDnActor::s_hLocalActor->GetUniqueID();
	const PartyStruct* pPs = GetPartyDataFromSessionID(mySessionId);
	return (pPs && pPs->bGMTrace);
}

void CDnPartyTask::SetPVPLadderInfo(LadderSystem::SC_SCOREINFO *pData)
{
	memset( &m_sPVPLadderInfo, 0, sizeof(m_sPVPLadderInfo) );

	m_sPVPLadderInfo.Data.iPvPLadderPoint	= pData->Data.iPvPLadderPoint;
	m_sPVPLadderInfo.Data.cMaxKillJobCode	= pData->Data.cMaxKillJobCode;
	m_sPVPLadderInfo.Data.cMaxDeathJobCode	= pData->Data.cMaxDeathJobCode;
	m_sPVPLadderInfo.Data.cLadderTypeCount	= pData->Data.cLadderTypeCount;
	for( int i=0 ;i<m_sPVPLadderInfo.Data.cLadderTypeCount ; ++i )
	{
		m_sPVPLadderInfo.Data.LadderScore[i] = pData->Data.LadderScore[i];
	}
}

//Set with new Nest info..
void CDnPartyTask::AddNestInfo(int nSessionID, SCGetPlayerCustomEventUI* pInfo)
{
	SCGetPlayerCustomEventUI newInfo;
	memset(&newInfo, 0, sizeof(newInfo));

	if (pInfo != NULL)
		newInfo = *pInfo;

	NESTINFO_LIST::iterator findIter = m_NestInfoList.find(nSessionID);
	if (findIter != m_NestInfoList.end())
	{
		findIter->second = newInfo;
	}
	else
		m_NestInfoList.insert(std::make_pair(nSessionID, newInfo));
}

//Remove nest info.
void CDnPartyTask::RemoveNestInfo(int nSessionID)
{
	NESTINFO_LIST::iterator findIter = m_NestInfoList.find(nSessionID);
	if (findIter != m_NestInfoList.end())
	{
		m_NestInfoList.erase(findIter);
	}
}

SCGetPlayerCustomEventUI* CDnPartyTask::GetNestInfo(int nSessionID)
{
	NESTINFO_LIST::iterator findIter = m_NestInfoList.find(nSessionID);
	if (findIter != m_NestInfoList.end())
		return &findIter->second;

	return NULL;
}

void CDnPartyTask::OnRecvPlayerCustomEventUI(  int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case ePlayerCustomEventUI::SC_GETPLAYERUI:	OnRecvPlayerCustomEventUI( (SCGetPlayerCustomEventUI *)pData );	break;
	}
}

void CDnPartyTask::OnRecvPlayerCustomEventUI(SCGetPlayerCustomEventUI* pPacket)
{
	if (pPacket != NULL)
		AddNestInfo(pPacket->nSessionID, pPacket);

	CDnNestInfoDlg* pNestInfoDlg = GetInterface().GetNestInfoDlg();
	if (pNestInfoDlg)
	{
		pNestInfoDlg->RefreshInfo(pPacket);
		pNestInfoDlg->Show(true);
	}
}

void CDnPartyTask::RequestNestInfo(int nSessionID)
{
	//If VillageTask exists, send a request packet to get the latest info.
// 	const CDnVillageTask *pVillageTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask("VillageTask");
// 	if (pVillageTask)
//	{
		SendRequestNestInfo(nSessionID);
//	}
// 	else
// 	{
// 		// If VillageTask is missing, display using previously saved info
// 		SCGetPlayerCustomEventUI* pNestInfo = GetNestInfo(nSessionID);
// 		if (pNestInfo != NULL)
// 			OnRecvPlayerCustomEventUI(pNestInfo);
// 	}
}

#ifdef PRE_PARTY_DB
int	CDnPartyTask::GetMaxPasswordLength() const
{
	return 4; // todo by kalliste
}

#ifdef _WORK
void CDnPartyTask::SetPartyListRefreshTime(float fTime)
{
	m_fPartyListRefreshTime = fTime;

	CDnVillageTask *pVillageTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask("VillageTask");
	if (pVillageTask)
		pVillageTask->SetPartyRefreshDelta(fTime);
}
#endif // _WORK
#endif

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )

void CDnPartyTask::ProcessEffectSkill( float fElapsedTime )
{
	if ( !CTaskManager::GetInstance().GetTask("VillageTask"))
		return;

	for( DWORD i=0; i<m_VecPartyList.size(); i++ )
	{
		std::vector<VillagePartyEffectSkillInfo>::iterator it = m_VecPartyList[i].vecEffestSkillData.begin();
		for( it; it != m_VecPartyList[i].vecEffestSkillData.end(); )
		{
			if( (*it).bEternity == false )
			{
				(*it).fRemainTime -= fElapsedTime;
				
				if( (*it).fRemainTime < 0.f )
				{
					it = m_VecPartyList[i].vecEffestSkillData.erase( it );
					continue;
				}
			}

			++it;
		}
	}
}

void CDnPartyTask::AddEffectSkillInfo(  UINT nSessionID , VillagePartyEffectSkillInfo* pInfo )
{
	if ( !CTaskManager::GetInstance().GetTask("VillageTask"))
		return;

	for( DWORD i=0; i<m_VecPartyList.size(); i++ )
	{
		if( m_VecPartyList[i].nSessionID == nSessionID )
		{
			m_VecPartyList[i].vecEffestSkillData.push_back(*pInfo);
			break;
		}
	}
}

void CDnPartyTask::RemoveEffectSkillInfo( UINT nSessionID , int nEffectSkillIndex )
{
	if ( !CTaskManager::GetInstance().GetTask("VillageTask"))
		return;

	for( DWORD i=0; i<m_VecPartyList.size(); i++ )
	{
		if( m_VecPartyList[i].nSessionID == nSessionID )
		{
			std::vector<VillagePartyEffectSkillInfo>::iterator itor = m_VecPartyList[i].vecEffestSkillData.begin();

			for( itor = m_VecPartyList[i].vecEffestSkillData.begin() ; itor != m_VecPartyList[i].vecEffestSkillData.end(); )
			{
				if( itor->nSkillID == nEffectSkillIndex  )
				{
					itor = m_VecPartyList[i].vecEffestSkillData.erase( itor );
				}
				else
				{
					++itor;
				}
			}
			break;
		}
	}
}

#endif

#ifdef PRE_WORLDCOMBINE_PARTY
void CDnPartyTask::AddWorldCombinePartyTableDataCache(int index, SWorldCombinePartyData& data)
{
	std::map<int, SWorldCombinePartyData>::iterator iter = m_WorldCombineTableDataCache.find(index);
	if (iter != m_WorldCombineTableDataCache.end())
	{
		SWorldCombinePartyData& existingData = (*iter).second;
		existingData = data;
	}
	else
	{
		m_WorldCombineTableDataCache.insert(std::make_pair(index, data));
	}
}

void CDnPartyTask::GetWorldCombinePartyTableData(int index, SWorldCombinePartyData& data) const
{
	std::map<int, SWorldCombinePartyData>::const_iterator iter = m_WorldCombineTableDataCache.find(index);
	if (iter != m_WorldCombineTableDataCache.end())
		data = (*iter).second;
}

bool CDnPartyTask::IsPartyTypeNeedRaidPartyTypeUI() const
{
	if (m_PartyType == _RAID_PARTY_8)
	{
		return true;
	}
	else if (m_PartyType == _WORLDCOMBINE_PARTY)
	{
		if (m_nCurWorldCombinePartyTableIndex == Party::Constants::INVALID_WORLDCOMBINE_TABLE_INDEX)
			return true;

		SWorldCombinePartyData data;
		GetCurrentWorldCombinePartyData(data);
		if (data.IsEmpty())
			return true;

		return (data.maxUser > NORMPARTYCOUNTMAX);
	}
	
	return false;
}

void CDnPartyTask::GetCurrentWorldCombinePartyData(SWorldCombinePartyData& data) const
{
	std::map<int, SWorldCombinePartyData>::const_iterator iter = m_WorldCombineTableDataCache.find(m_nCurWorldCombinePartyTableIndex);
	if (iter != m_WorldCombineTableDataCache.end())
		data = (*iter).second;
}

#endif

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
void CDnPartyTask::ReqPartyInfoWithPlayerID( const TPARTYID PlayerID )
{	
	SendReqPartyInfo( PlayerID);  	
}
void CDnPartyTask::ReqPartyInfoWithPlayerName( const WCHAR* pPlayerName )
{
	SendReqPartyInfo( pPlayerName );
}
#endif
