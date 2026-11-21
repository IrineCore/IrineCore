#include "Stdafx.h"
#include "DNGestureTask.h"
#include "DnTableDB.h"
#include "VillageSendPacket.h"
#include "DnInterface.h"
#include "DnMainDlg.h"
#include "DnLocalPlayerActor.h"
#include "GuildSendPacket.h"
#include "DnWorld.h"
#include "DnPlayerCamera.h"
#include "DnPetActor.h"



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

inline void SendUseGesture(USHORT nGestureID)
{
	CSUseGesture Packet;
	Packet.nGestureID = nGestureID;
	CClientSessionManager::GetInstance().SendPacket(CS_GESTURE, eGesture::CS_USEGESTURE, (char*)&Packet, sizeof(Packet));
}

CDnGestureTask::CDnGestureTask(void) : CTaskListener(true)
, m_bRequestQuickSlot( false )
, m_fPaperingRemainTime( 0.0f )
, m_nRequestGuildWarSkillID( 0 )
{
}

CDnGestureTask::~CDnGestureTask(void)
{
	Finalize();
}

bool CDnGestureTask::Initialize()
{
	// Unlike missions, load everything, show unpaid items in the window, and display a prompt to pay.
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGESTURE );

	int nNumItem = pSox->GetItemCount();
	for( int nItem = 0; nItem < nNumItem; ++nItem )
	{
		int nItemID = pSox->GetItemID( nItem );
		if( pSox->GetFieldFromLablePtr( nItemID, "_NameID" )->GetInteger() == 0 )
			continue;

		SGestureInfo* pNewGestureInfo = new SGestureInfo;
		pNewGestureInfo->nID = nItemID;
		pNewGestureInfo->szName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nItemID, "_NameID" )->GetInteger() );
		pNewGestureInfo->nIconID = pSox->GetFieldFromLablePtr( nItemID, "_IconID" )->GetInteger();
		pNewGestureInfo->szActionName = pSox->GetFieldFromLablePtr( nItemID, "_ActionID" )->GetString();
		pNewGestureInfo->nMsgID = pSox->GetFieldFromLablePtr( nItemID, "_MsgID" )->GetInteger();
		pNewGestureInfo->szDescription = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nItemID, "_DescID" )->GetInteger() );
		pNewGestureInfo->nUnlockLevel = pSox->GetFieldFromLablePtr( nItemID, "_UnlockLv" )->GetInteger();

		for( int i = 0; i < NUM_KEYWORD; ++i )
		{
			char szLabel[32];
			sprintf_s(szLabel, 32, "%s%d", "_KeywordID_", i);
			int nStringID = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetInteger();
			if( nStringID )
			{
				pNewGestureInfo->szKeyword[i] = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID );
				if( pNewGestureInfo->szKeyword[i] == L"_NotUsed" )	// Reserved keyword not yet in use
					pNewGestureInfo->szKeyword[i] = L"";
			}
		}

#if defined(PRE_ADD_GUILD_GESTURE)
		pNewGestureInfo->_Type = (eGestureType)pSox->GetFieldFromLablePtr( nItemID, "_GestureType" )->GetInteger();
#endif // PRE_ADD_GUILD_GESTURE

#if defined(PRE_ADD_GUILD_GESTURE)
		if (pNewGestureInfo->_Type == GestureType_GuildWarWin ||
			pNewGestureInfo->_Type == GestureType_GuildReward)
#else
		if( pSox->GetFieldFromLablePtr( nItemID, "_GestureType" )->GetInteger() == 1 )
#endif // PRE_ADD_GUILD_GESTURE
		{
			pNewGestureInfo->nUnlockLevel = -2;
			pNewGestureInfo->szReactionName1 = pSox->GetFieldFromLablePtr( nItemID, "_Reaction_1" )->GetString();
			pNewGestureInfo->szReactionName2 = pSox->GetFieldFromLablePtr( nItemID, "_Reaction_2" )->GetString();
		}

		// Default is false
		pNewGestureInfo->bUsable = false;
		m_pVecGestureInfo.push_back( pNewGestureInfo );
	}

	m_bRequestQuickSlot = true;
	m_fPaperingRemainTime = 0.0f;

	return true;
}

void CDnGestureTask::Finalize()
{
	SAFE_DELETE_PVEC( m_pVecGestureInfo );
}

void CDnGestureTask::OnDispatchMessage(int nMainCmd, int nSubCmd, char *pData, int nSize)
{
	switch (nMainCmd)
	{
	case SC_GESTURE:	OnRecvGestureMessage(nSubCmd, pData, nSize);	break;
	case SC_GUILD:		OnRecvGuildMessage(nSubCmd, pData, nSize);	break;
	}
}

void CDnGestureTask::OnRecvGestureMessage(int nSubCmd, char *pData, int nSize)
{
	switch (nSubCmd)
	{
	case eGesture::SC_CASHGESTURELIST: OnRecvGestureList( (SCGestureList*)pData );	break;
	case eGesture::SC_CASHGESTUREADD: OnRecvGestureAdd( (SCGestureAdd*)pData );	break;
	case eGesture::SC_USEGESTURE: OnRecvUseGesture( (SCUseGesture*)pData ); break;
	case eGesture::SC_EFFECTITEMGESTURELIST: OnRecvGuildRewardGestureList((SCGestureList*)pData); break;
	}
}

void CDnGestureTask::OnRecvGuildMessage(int nSubCmd, char *pData, int nSize)
{
	switch (nSubCmd)
	{
	case eGuild::SC_GUILDWAR_PRE_WIN: OnRecvGuildWarPreWin( (SCGuildWarPreWin*)pData );	break;
	case eGuild::SC_GUILDWAR_WIN_SKILL: OnRecvGuildWarWinSkill( (SCGuildWarWinSkill*)pData ); break;
	case eGuild::SC_GUILDWAR_USER_WINSKILL: OnRecvGuildWarUserWinSkill( (SCGuildWarUserWinSkill*)pData ); break;
	}
}

void CDnGestureTask::OnRecvCmdAction( LPCWSTR wszName, int nGestureID )
{
	//SGestureInfo *pInfo = GetGestureInfoFromID( nGestureID );
	//if( !pInfo ) return;

	//WCHAR wszTemp[_MAX_PATH];
	//swprintf_s( wszTemp, _MAX_PATH, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pInfo->nMsgID ), wszName );
	//GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp, false );
}

DWORD CDnGestureTask::GetVisibleGestureCount()
{
	int nUnvisibleCashGestureCount = 0;
	for( DWORD i=0; i<m_pVecGestureInfo.size(); i++ )
	{
		if( m_pVecGestureInfo[i]->nUnlockLevel < 0 && m_pVecGestureInfo[i]->bUsable == false )
			++nUnvisibleCashGestureCount;
	}
	return (int)m_pVecGestureInfo.size() - nUnvisibleCashGestureCount;
}

CDnGestureTask::SGestureInfo *CDnGestureTask::GetVisibleGestureInfoFromIndex( DWORD nIndex )
{
	SGestureInfo *pInfo = NULL;
	for( DWORD i=0, j=0; i<m_pVecGestureInfo.size(); i++, j++ )
	{
		if( m_pVecGestureInfo[i]->nUnlockLevel < 0 && m_pVecGestureInfo[i]->bUsable == false )
		{
			--j;
			continue;
		}

		if( j == nIndex )
		{
			pInfo = m_pVecGestureInfo[i];
			break;
		}
	}
	return pInfo;
}

CDnGestureTask::SGestureInfo *CDnGestureTask::GetGestureInfoFromID( int nID )
{
	for( DWORD i=0; i<m_pVecGestureInfo.size(); i++ )
	{
		if( m_pVecGestureInfo[i]->nID == nID )
		{
			return m_pVecGestureInfo[i];
		}
	}
	return NULL;
}

CDnGestureTask::SGestureInfo *CDnGestureTask::GetGestureInfoFromActionName( const char *szActionName )
{
	for( DWORD i=0; i<m_pVecGestureInfo.size(); i++ )
	{
		if( _stricmp(m_pVecGestureInfo[i]->szActionName.c_str(), szActionName) == 0 )
		{
			return m_pVecGestureInfo[i];
		}
	}
	return NULL;
}

CDnGestureTask::SGestureInfo *CDnGestureTask::GetGestureInfoFromGestureName( LPCWSTR wszName )
{
	for( DWORD i=0; i<m_pVecGestureInfo.size(); i++ )
	{
		if( _wcsicmp(m_pVecGestureInfo[i]->szName.c_str(), wszName) == 0 )
		{
			return m_pVecGestureInfo[i];
		}
	}
	return NULL;
}

void CDnGestureTask::OnRecvGestureList( SCGestureList *pPacket )
{
	std::map<int, bool>	mapCashGesture;
	for( int i=0; i<pPacket->cGestureCount; i++)
	{
		mapCashGesture[pPacket->nGestureID[i]] = true;
	}

	// Update info only for cash item gestures
	for( DWORD i=0; i<m_pVecGestureInfo.size(); i++ )
	{
		if( m_pVecGestureInfo[i]->nUnlockLevel == -1 ) {
			m_pVecGestureInfo[i]->bUsable = false;
			if( mapCashGesture.find( m_pVecGestureInfo[i]->nID) != mapCashGesture.end() )
			{
				m_pVecGestureInfo[i]->bUsable = true;
			}
			continue;
		}
	}
}

void CDnGestureTask::OnRecvGestureAdd( SCGestureAdd *pPacket )
{
	WCHAR wszStr[256];
	SGestureInfo * pInfo = GetGestureInfoFromID( pPacket->nGestureID );
	if( pInfo == NULL )	return;

	// Print message to chat window - for added gestures, the full list packet follows.
	wsprintf( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1368 ), pInfo->szName.c_str() );
	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszStr, true );
}

void CDnGestureTask::OnRecvUseGesture( SCUseGesture *pPacket )
{
	CDnGestureTask::SGestureInfo *pInfo = GetGestureInfoFromID( pPacket->nGestureID );
	if( !pInfo ) return;
	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());

	// Check once more and perform action
	if( !pPlayer->IsBattleMode() && ( pPlayer->IsStay() || pPlayer->IsMove() ) ) 
	{
		pPlayer->CmdAction( pInfo->szActionName.c_str() );

		if( pPlayer->IsInPetSummonableMap() && pPlayer->IsSummonPet() )
		{
			CDnPetActor* pPetActor = pPlayer->GetMyPetActor();
			if( pPetActor )
				pPetActor->DoPetGesture( pInfo->nID );
		}
	}
}

void CDnGestureTask::RefreshGestureList( int nCurrentLevel, bool bOnLevelUp )
{
	// Unlock everything available based on level,
	for( DWORD i=0; i<m_pVecGestureInfo.size(); i++ )
	{
		// Gestures from cash items are not handled here.
		if( m_pVecGestureInfo[i]->nUnlockLevel < 0 ) {
			continue;
		}

		m_pVecGestureInfo[i]->bUsable = (m_pVecGestureInfo[i]->nUnlockLevel > nCurrentLevel) ? false : true;

		// Notify gesture addition for items matching the current level.
		if( bOnLevelUp && m_pVecGestureInfo[i]->nUnlockLevel == nCurrentLevel ) {
			WCHAR wszMsg[256] = {0,};
			swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1368 ), m_pVecGestureInfo[i]->szName.c_str() );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszMsg, false );
		}
	}
}

void CDnGestureTask::UseGesture( int nGestureID, bool bUseByChat )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());

	if( !pPlayer->IsSwapSingleSkin() && !pPlayer->IsDie() && !pPlayer->IsAir() && ( pPlayer->IsStay() || pPlayer->IsMove() ) )
	{
		SGestureInfo *pInfo = GetGestureInfoFromID( nGestureID );
		if( !pInfo ) return;
		if( !pInfo->bUsable ) return;

		if( CheckPapering() )
		{
			// Output as system message unlike chat spam; do not display if received as a chat message.
			if( bUseByChat == false )
			{
				wchar_t wszMsg[256]={0};
				int nTime = (int)m_fPaperingRemainTime;
				swprintf_s( wszMsg, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 674 ), nTime );
				GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszMsg, false );
			}
			return;
		}

#if defined(PRE_ADD_GUILD_GESTURE)
		if (pInfo->_Type == GestureType_GuildWarWin)
		{
			// Guild victory gesture skill uses global cooldown instead of spam prevention.
			// Still need to go through spam prevention; otherwise players send packets as fast as they press skill keys.
			if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage )
			{
				m_nRequestGuildWarSkillID = nGestureID;
				SendGuildWarWinSkill( nGestureID );
			}
		}
#else
		if( pInfo->nUnlockLevel == -2 )
		{
			// Guild victory gesture skill uses global cooldown instead of spam prevention.
			// Still need to go through spam prevention; otherwise players send packets as fast as they press skill keys.
			if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage )
			{
				m_nRequestGuildWarSkillID = nGestureID;
				SendGuildWarWinSkill( nGestureID );
			}
		}
#endif //PRE_ADD_GUILD_GESTURE
		else
		{
			// Remaining arguments use defaults. Server packet transmission is also handled internally.
			SendUseGesture( nGestureID );
		}

		// Gestures performed by self must also appear in the chat window.
		//WCHAR wszTemp[_MAX_PATH];
		//swprintf_s( wszTemp, _MAX_PATH, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pInfo->nMsgID ), pPlayer->GetName() );
		//GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp, false );
	}
}

void CDnGestureTask::UseGestureByChat( LPCWSTR wszMessage )
{
	// Skip if it is a combat dialogue
	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	if( pPlayer->IsBattleMode() ) return;

	// Gestures triggered by chat
	// Check current action so it does not trigger while moving or sitting (Normal is when weapon is sheathed).
	if( strcmp( pPlayer->GetCurrentAction(), "Normal_Stand" ) != 0 )
		return;

	// / Check social name
	std::wstring strText(wszMessage);
	if( strText[0] == L'/' )
	{
		std::wstring strCommand;
		std::wstring::size_type endIdx;
		endIdx = strText.find_first_of(L" ");
		if( endIdx == std::wstring::npos )
			strCommand = strText.substr(1, endIdx);
		else
			strCommand = strText.substr(1, endIdx-1);

		if( strCommand.size() )
		{
			SGestureInfo *pInfo = GetGestureInfoFromGestureName( strCommand.c_str() );
			if( pInfo ) UseGesture( pInfo->nID, true );
		}
	}
	else
	{
		// Check keyword
		bool bFind = false;
		int nID = 0;
		for( DWORD i=0; i<m_pVecGestureInfo.size(); i++ )
		{
			for( int j = 0; j < NUM_KEYWORD; ++j )
			{
				if( m_pVecGestureInfo[i]->szKeyword[j].size() )
				{
					if( _wcsnicmp(m_pVecGestureInfo[i]->szKeyword[j].c_str(), strText.c_str(), m_pVecGestureInfo[i]->szKeyword[j].size()) == 0 )
					{
						nID = m_pVecGestureInfo[i]->nID;
						bFind = true;
						break;
					}
				}
			}
			if( bFind ) break;
		}

		if( bFind && nID )
		{
			SGestureInfo *pInfo = GetGestureInfoFromID( nID );
			if( pInfo ) UseGesture( pInfo->nID, true );
		}
	}
}

void CDnGestureTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	std::list<float>::iterator iter = m_listRecentChatTime.begin();
	while( iter != m_listRecentChatTime.end() )
	{
		*iter -= fDelta;
		if( *iter <= 0.0f )
		{
			iter = m_listRecentChatTime.erase( iter );
			continue;
		}
		++iter;
	}

	if( m_fPaperingRemainTime > 0.0f )
		m_fPaperingRemainTime -= fDelta;
}

bool CDnGestureTask::CheckPapering()
{
	// If currently restricted due to chat spamming,
	if( m_fPaperingRemainTime > 0.0f )
		return true;

	// If the limit count is reached when adding,
	if( (int)m_listRecentChatTime.size() + 1 >= GESTURE_PAPERING_CHECKCOUNT )
		m_fPaperingRemainTime = (float)GESTURE_PAPERING_RESTRICTIONTIME;

	m_listRecentChatTime.push_back((float)GESTURE_PAPERING_CHECKTIME);
	return false;
}

void CDnGestureTask::OnRecvGuildWarPreWin( SCGuildWarPreWin *pPacket )
{
	for( DWORD i=0; i<m_pVecGestureInfo.size(); i++ ) {
#if defined(PRE_ADD_GUILD_GESTURE)
		if( m_pVecGestureInfo[i]->_Type == GestureType_GuildWarWin ) {
#else
		if( m_pVecGestureInfo[i]->nUnlockLevel == -2 ) {
#endif // PRE_ADD_GUILD_GESTURE
			m_pVecGestureInfo[i]->bUsable = pPacket->bPreWin;
		}
	}
}

void CDnGestureTask::OnRecvGuildWarWinSkill( SCGuildWarWinSkill *pPacket )
{
	if( pPacket->nRetCode == ERROR_NONE ) {
		if( m_nRequestGuildWarSkillID > 0 ) {
			SGestureInfo *pInfo = GetGestureInfoFromID( m_nRequestGuildWarSkillID );
			if( pInfo ) {
				if( CDnActor::s_hLocalActor ) {
					CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
					if( !pPlayer->IsBattleMode() && ( pPlayer->IsStay() || pPlayer->IsMove() ) ) {
						pPlayer->CmdAction( pInfo->szActionName.c_str() );
					}
				}
			}
		}
	}
	else {
		// Display error value only in the two cases below
		WCHAR wszTemp[256]={0,};
		if( pPacket->nRetCode == ERROR_GUILDWAR_NOT_PRE_WIN ) {
			swprintf_s( wszTemp, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126266 ) );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp, false );
		}
		else if( pPacket->nRetCode == ERROR_GUILDWAR_SKILL_COOLTIME ) {
			swprintf_s( wszTemp, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126267 ), pPacket->dwCoolTime / 1000 );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp, false );
		}
	}
	m_nRequestGuildWarSkillID = 0;
}

void CDnGestureTask::OnRecvGuildWarUserWinSkill( SCGuildWarUserWinSkill *pPacket )
{
	if( !CDnActor::s_hLocalActor ) return;
	if( CDnPlayerActor::s_bHideAnotherPlayer ) return;
	if( GetInterface().IsOpenBlind() ) return;

	if( !pPacket->GuildUID.IsSet() ) return;
	SGestureInfo *pInfo = GetGestureInfoFromID( pPacket->nSkillID );
	if( !pInfo ) return;
	if( pInfo->nUnlockLevel != -2 ) return;

	DnActorHandle hActor;
	CDnPlayerActor* pPlayer;
	ScopeLock<CSyncLock> Lock(CDnActor::s_LockSearchMap);
	std::map<DWORD, DnActorHandle>::iterator it = CDnActor::s_dwMapActorSearch.begin();
	for( ; it != CDnActor::s_dwMapActorSearch.end(); ++it ) {
		if( !it->second->IsProcess() ) continue;
		if( !it->second->IsPlayerActor() ) continue;
		hActor = it->second;
		pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		if( pPlayer->IsJoinGuild() && pPlayer->GetGuildSelfView().GuildUID == pPacket->GuildUID ) continue;

		// Still, they ask to include even those using skills or jumping for now.
		//if( pPlayer->IsStay() || pPlayer->IsMove() )
		{
			// Units in combat mode should be switched to non-combat mode,
			if( pPlayer->IsBattleMode() ) {
				pPlayer->CDnPlayerActor::CmdStop( "Stand" );
				pPlayer->CDnPlayerActor::CmdToggleBattle( false );
			}

			// If currently using a skill, release it
			if( pPlayer->IsProcessSkill() )
				pPlayer->CancelUsingSkill();

			// Unlock camera Yaw-Lock when it is self
			if( pPlayer->GetUniqueID() == CDnActor::s_hLocalActor->GetUniqueID() ) {
				DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
				if( hCamera && hCamera->GetCameraType() == CDnCamera::PlayerCamera )
					((CDnPlayerCamera*)hCamera.GetPointer())->LockFreeYaw( true );
			}

			// Make it look at target and
			if( pPlayer->IsVehicleMode() ) {
				CDnVehicleActor *pVehicle = pPlayer->GetMyVehicleActor();
				EtVector2 vDir;
				EtVector3 vCrossVec;
				vDir.x = pPacket->vPos.x - pVehicle->GetPosition()->x;
				vDir.y = pPacket->vPos.z - pVehicle->GetPosition()->z;
				D3DXVec2Normalize( &vDir, &vDir );
				float fDot = D3DXVec2Dot( &vDir, &EtVec3toVec2( pVehicle->GetMatEx()->m_vZAxis ) );
				if( fDot >= 1.0f ) fDot = 1.0f;
				float fAngle = EtToDegree( EtAcos(fDot) );
				D3DXVec3Cross( &vCrossVec, &EtVec2toVec3( vDir ), &pVehicle->GetMatEx()->m_vZAxis );
				if( vCrossVec.y > 0.0f ) pVehicle->GetMatEx()->RotateYaw( fAngle );
				else pVehicle->GetMatEx()->RotateYaw( -fAngle );

				// Action
				if( !pInfo->szReactionName2.empty() )
					pVehicle->SetActionQueue( pInfo->szReactionName2.c_str() );
			}
			else {
				EtVector2 vDir;
				EtVector3 vCrossVec;
				vDir.x = pPacket->vPos.x - pPlayer->GetPosition()->x;
				vDir.y = pPacket->vPos.z - pPlayer->GetPosition()->z;
				D3DXVec2Normalize( &vDir, &vDir );
				float fDot = D3DXVec2Dot( &vDir, &EtVec3toVec2( pPlayer->GetMatEx()->m_vZAxis ) );
				if( fDot >= 1.0f ) fDot = 1.0f;
				float fAngle = EtToDegree( EtAcos(fDot) );
				D3DXVec3Cross( &vCrossVec, &EtVec2toVec3( vDir ), &pPlayer->GetMatEx()->m_vZAxis );
				if( vCrossVec.y > 0.0f ) pPlayer->GetMatEx()->RotateYaw( fAngle );
				else pPlayer->GetMatEx()->RotateYaw( -fAngle );

				// Action
				if( !pInfo->szReactionName1.empty() )
					pPlayer->SetActionQueue( pInfo->szReactionName1.c_str() );
			}
		}
	}
}

void CDnGestureTask::OnRecvGuildRewardGestureList( SCGestureList *pPacket )
{
	std::map<int, bool>	mapCashGesture;
	for( int i=0; i<pPacket->cGestureCount; i++)
	{
		mapCashGesture[pPacket->nGestureID[i]] = true;
	}

	for( DWORD i=0; i<m_pVecGestureInfo.size(); i++ )
	{
		if( m_pVecGestureInfo[i]->_Type == eGestureType::GestureType_GuildReward ) {
			m_pVecGestureInfo[i]->bUsable = false;
			if( mapCashGesture.find( m_pVecGestureInfo[i]->nID) != mapCashGesture.end() )
			{
				m_pVecGestureInfo[i]->bUsable = true;
			}
			continue;
		}
	}
}