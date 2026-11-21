#include "StdAfx.h"
#include "DnDungeonClearImp.h"
#include "DnInterface.h"
#include "DnLocalPlayerActor.h"
#include "DnTableDB.h"
#include "DnPartyTask.h"
#include "TaskManager.h"
#include "DnGameTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
CDnDungeonClearImp::CDnDungeonClearImp( CDnGameTask *pTask )
: CDnStageClearImp( pTask )
{
}

CDnDungeonClearImp::~CDnDungeonClearImp()
{
}


void CDnDungeonClearImp::OnCalcClearProcess( void *pData )
{
	SCDungeonClear *pPacket = (SCDungeonClear *)pData;

	// Note: Set party members' dungeon info and open the dungeon clear window.
	m_vecDungeonClearInfo.clear();
	memset( &m_DungeonClearValue, 0, sizeof(m_DungeonClearValue) );

	m_DungeonClearValue.nDungeonLevel = pPacket->nDungeonClearTableID;
	m_DungeonClearValue.nClearTime = pPacket->nClearTime;
	m_DungeonClearValue.bIgnoreRewardItem = pPacket->bIgnoreRewardItem;
	if( CDnActor::s_hLocalActor ) {
		m_DungeonClearValue.nCurrentExp = pPacket->nCurExp;
		m_DungeonClearValue.nCurrentLevel = pPacket->cCurLevel;
	}

	SDungeonClearInfo dungeonClearInfo;

	for( int i=0; i<pPacket->cCount; i++ )
	{
		dungeonClearInfo.sBaseInfo = pPacket->Info[i];
		m_vecDungeonClearInfo.push_back(dungeonClearInfo);
	}

	CalcDungeonClearInfo();
	GetInterface().OpenStageClearReportDialog( m_pGameTask );

	CDnLocalPlayerActor::StopAllPartyPlayer();

}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDnDungeonClearImp::OnCalcClearRewardBoxProcess(void* pData)
{
	SCDungeonClearBoxType *pPacket = static_cast<SCDungeonClearBoxType*>(pData);
	for (int i=0; i < pPacket->cCount; i++)
	{
		for( DWORD j=0; j<m_vecDungeonClearInfo.size(); j++ )
		{
			if( m_vecDungeonClearInfo[j].sBaseInfo.nSessionID == pPacket->Info[i].nSessionID )
			{
				memcpy( m_vecDungeonClearInfo[j].cRewardItemType, pPacket->Info[i].cRewardItemType, sizeof(pPacket->Info[i].cRewardItemType) );
				break;
			}
		}
	}
}
#endif

void CDnDungeonClearImp::OnCalcClearRewardItemProcess( void *pData )
{
	SCDungeonClearRewardItem *pPacket = (SCDungeonClearRewardItem *)pData;
	for( int i=0; i<pPacket->cCount; i++ )
	{
		for( DWORD j=0; j<m_vecDungeonClearInfo.size(); j++ ) {
			if( m_vecDungeonClearInfo[j].sBaseInfo.nSessionID == pPacket->Info[i].nSessionID ) {
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
#else
				memcpy( m_vecDungeonClearInfo[j].cRewardItemType, pPacket->Info[i].cRewardItemType, sizeof(pPacket->Info[i].cRewardItemType) );
#endif
				for( int k=0; k<4; k++ ) {
					CDnItem::MakeItemInfo( pPacket->Info[i].nRewardItemID[k], 1, m_vecDungeonClearInfo[j].RewardItem[k] );
					m_vecDungeonClearInfo[j].RewardItem[k].nRandomSeed = pPacket->Info[i].nRewardItemRandomSeed[k];
					m_vecDungeonClearInfo[j].RewardItem[k].cOption = pPacket->Info[i].cRewardItemOption[k];
					m_vecDungeonClearInfo[j].RewardItem[k].wCount = pPacket->Info[i].wRewardItemCount[k];
					m_vecDungeonClearInfo[j].RewardItem[k].bSoulbound = pPacket->Info[i].cRewardItemSoulBound[k] ? true : false;

#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					m_vecDungeonClearInfo[j].RewardItem[k].cLevel = pPacket->Info[i].cRewardItemLevel[k];
					m_vecDungeonClearInfo[j].RewardItem[k].cPotential = pPacket->Info[i].cRewardItemPotential[k];
#endif
				}
//				memcpy( m_vecDungeonClearInfo[j].nRewardItemID, pPacket->Info[i].nRewardItemID, sizeof(pPacket->Info[i].nRewardItemID) );
				break;
			}
		}
	}
}

void CDnDungeonClearImp::CalcDungeonClearInfo()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDUNGEONCLEAR );
	if( !pSox ) return;

#if defined( PRE_ADD_CP_RENEWAL )
	int nStandardCP = pSox->GetFieldFromLablePtr( m_DungeonClearValue.nDungeonLevel, "_StageBaseKillScore" )->GetInteger();
#endif	// #if defined( PRE_ADD_CP_RENEWAL )

	m_DungeonClearValue.nMaxCombo = pSox->GetFieldFromLablePtr( m_DungeonClearValue.nDungeonLevel, "_MaxComboScore" )->GetInteger();
	m_DungeonClearValue.nKillMonster = pSox->GetFieldFromLablePtr( m_DungeonClearValue.nDungeonLevel, "_KillBossNumScore" )->GetInteger();
	m_DungeonClearValue.nTimeLimit = pSox->GetFieldFromLablePtr( m_DungeonClearValue.nDungeonLevel, "_TimeLimit" )->GetInteger();
	m_DungeonClearValue.nTimeRatio = pSox->GetFieldFromLablePtr( m_DungeonClearValue.nDungeonLevel, "_TimeLimitScore" )->GetInteger();

	CDnPartyTask *pPartyTask = (CDnPartyTask*)CTaskManager::GetInstance().GetTask( "PartyTask" );
	if( !pPartyTask ) return;

	m_DungeonClearValue.nPartyCount = pPartyTask->GetPartyCount();

	if( m_DungeonClearValue.nPartyCount > 1 )
	{
		char szTemp[32] = {0};
		sprintf_s( szTemp, _countof(szTemp), "_RewordCP%d", m_DungeonClearValue.nPartyCount );
		m_DungeonClearValue.nPartyBonus = pSox->GetFieldFromLablePtr( m_DungeonClearValue.nDungeonLevel, szTemp )->GetInteger();
	}
	else
	{
		m_DungeonClearValue.nPartyBonus = 0;
	}

	wchar_t wszTemp[64] = {0};
	CDnPartyTask::PartyStruct *pPartyStruct(NULL);

	for( int i=0; i<(int)m_vecDungeonClearInfo.size(); i++ )
	{
		// Note : Set basic info
		//
		{
			pPartyStruct = pPartyTask->GetPartyDataFromSessionID( m_vecDungeonClearInfo[i].sBaseInfo.nSessionID );
			if( !pPartyStruct ) continue;

			m_vecDungeonClearInfo[i].nClassID = pPartyStruct->cClassID;
			m_vecDungeonClearInfo[i].nLevel = pPartyStruct->cLevel;
			m_vecDungeonClearInfo[i].strName = pPartyStruct->wszCharacterName;
		}

		int nClearTime = m_DungeonClearValue.nClearTime / 1000;
		int nH = nClearTime / (60 * 60);
		int nM = (nClearTime - (nH * 60 * 60)) / 60;
		int nS = (nClearTime - ((nH * 60 * 60) + (nM * 60)));

		swprintf_s( wszTemp, _countof(wszTemp), L"%02d : %02d : %02d", nH, nM, nS );
		m_vecDungeonClearInfo[i].strTime = wszTemp;

#if defined( PRE_ADD_CP_RENEWAL )
		int nScore = m_vecDungeonClearInfo[i].sBaseInfo.nMaxComboCount * m_DungeonClearValue.nMaxCombo;
		int nMaxScore = nStandardCP * MACP_Renewal::eBonusCP_Revision_MaxCombo;
		if( nMaxScore < nScore )
			nScore = nMaxScore;
		m_vecDungeonClearInfo[i].nComboBonus = nScore;

		nScore = m_vecDungeonClearInfo[i].sBaseInfo.nKillBossCount * m_DungeonClearValue.nKillMonster;
		m_vecDungeonClearInfo[i].nKillBonus = nScore;

		CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask( "GameTask" ));
		if( NULL == pGameTask )
			return;

		if( 0 >= m_DungeonClearValue.nTimeRatio )
			m_DungeonClearValue.nTimeRatio = MACP_Renewal::eBonusCP_Revision_ClearTime;

		m_DungeonClearValue.nTimeLimit = (nStandardCP * MACP_Renewal::eBonusCP_Revision_ClearTime / m_DungeonClearValue.nTimeRatio) + m_DungeonClearValue.nMaxCombo;
		int nStageConstructionLevel = pGameTask->GetStageConstructionLevel();
		int nRevisionClearTime = MACP_Renewal::GetClearTime_Revision( (Dungeon::Difficulty::eCode)(nStageConstructionLevel) );
		nScore = (m_DungeonClearValue.nTimeLimit - nClearTime + nRevisionClearTime) * m_DungeonClearValue.nTimeRatio;
		nMaxScore = nStandardCP * MACP_Renewal::eBonusCP_Revision_ClearTime;
		if( nScore < 0 )
			nScore = 0;
		else if( nMaxScore < nScore )
			nScore = nMaxScore;
		m_vecDungeonClearInfo[i].nTimeBonus = nScore;
#else	// #if defined( PRE_ADD_CP_RENEWAL )
		m_vecDungeonClearInfo[i].nComboBonus = m_vecDungeonClearInfo[i].sBaseInfo.nMaxComboCount * m_DungeonClearValue.nMaxCombo;
		m_vecDungeonClearInfo[i].nKillBonus = m_vecDungeonClearInfo[i].sBaseInfo.nKillBossCount * m_DungeonClearValue.nKillMonster;

		int nTimeBonus = (m_DungeonClearValue.nTimeLimit - nClearTime) * m_DungeonClearValue.nTimeRatio;
		if( nTimeBonus < 0 ) nTimeBonus = 0;
		m_vecDungeonClearInfo[i].nTimeBonus = nTimeBonus;
#endif	// #if defined( PRE_ADD_CP_RENEWAL )

		m_vecDungeonClearInfo[i].nPartyBonus = m_DungeonClearValue.nPartyBonus;
	}
}

CDnDungeonClearImp::SDungeonClearInfo* CDnDungeonClearImp::GetDungeonClearInfo( int nSessionID )
{
	for( int i=0; i<(int)m_vecDungeonClearInfo.size(); i++ )
	{
		if( m_vecDungeonClearInfo[i].sBaseInfo.nSessionID == nSessionID )
		{
			return &m_vecDungeonClearInfo[i];
		}
	}

	return NULL;
}
