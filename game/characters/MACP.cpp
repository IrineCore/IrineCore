#include "stdafx.h"
#include "MACP.h"
#include "DnTableDB.h"
#include "DnSkill.h"
#include "DnActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

MACP::MACP()
{
	m_pActor = NULL;
	ResetCP();
}

MACP::~MACP()
{
}

bool MACP::Initialize( CDnActor *pActor )
{
	m_pActor = pActor;
	return true;
}

void MACP::ResetCP()
{
	memset( &m_Data, 0, sizeof(m_Data) );
	
	m_nCPTableID = 0;
}

bool MACP::SetCPTableID( const int nMapIndex, const int nClassID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TCP );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"CPTable.ext failed\r\n");
		return false;
	}

	int iItemID = 0;
	for (; iItemID < pSox->GetItemCount(); ++iItemID)
	{
		int curTableID = pSox->GetItemID(iItemID);
		int curMapIdx = pSox->GetFieldFromLablePtr( curTableID, "_MapIndex" )->GetInteger();

		if( nMapIndex != curMapIdx )
			continue;

		int curClassID = pSox->GetFieldFromLablePtr( curTableID, "_ClassID" )->GetInteger();
		if( nClassID != curClassID )
			continue;

		m_nCPTableID = curTableID;
		return true;
	}

	return false;
}

bool MACP::InitializeCPScore( const int nMapIndex, const int nClassID, const int nDungeonClearID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TCP );
	if( !pSox )
	{
		g_Log.Log( LogType::_FILELOG, L"CPTable.ext failed\r\n");
		return false;
	}

	if( false == SetCPTableID( nMapIndex, nClassID ) )
		return false;

	if( false == pSox->IsExistItem( m_nCPTableID ) )
		return false;

	m_Data.pScore[CPTypeEnum::KillMonsterScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_KillScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::CriticalHitScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_CriticalHitScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::StunHitScore]	= pSox->GetFieldFromLablePtr( m_nCPTableID, "_StunHitScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::SuperAmmorBreakScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_SuperAmmorBreakScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::BrokenShieldScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_BrokenShieldScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::GenocideScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_GenocideScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::AirComboScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_AirComboScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::PartyComboScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_ChainComboScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::RebirthPlayerScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_RescueScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::ComboScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_ComboScore" )->GetInteger();

	m_Data.pScore[CPTypeEnum::PropBreakScore] = pSox->GetFieldFromLablePtr(m_nCPTableID, "_ProbBreak")->GetInteger();
	m_Data.pScore[CPTypeEnum::GetItemScore] = pSox->GetFieldFromLablePtr(m_nCPTableID, "_GetItem")->GetInteger();

	m_Data.pScore[CPTypeEnum::AttackedHitScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_HitedScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::AttackedCriticalHitScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_CriticalHitedScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::AttackedStunHitScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_StunHitedScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::DeadScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_DeathCountScore" )->GetInteger();
	m_Data.pScore[CPTypeEnum::AssistMonsterScore] = pSox->GetFieldFromLablePtr( m_nCPTableID, "_AssistScore" )->GetInteger();

	return true;
}

void MACP::UpdateMaxCombo( int nValue )
{
	if( nValue > m_Data.pScore[CPTypeEnum::MaxComboCount] )
	{
		m_Data.pScore[CPTypeEnum::MaxComboCount] = nValue;
		OnEventCP( MaxComboCount, nValue );
	}
}

void MACP::UpdateKillBoss()
{
	m_Data.pScore[CPTypeEnum::KillBossCount]++;
	OnEventCP( KillBossCount, m_Data.pScore[CPTypeEnum::KillBossCount] );
}

bool MACP::UpdateKillMonster( int *pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::KillMonsterScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( KillMonsterScore, nValue );
	return true;
}

bool MACP::UpdateCriticalHit( int *pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::CriticalHitScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( CriticalHitScore, nValue );
	return true;
}

bool MACP::UpdateStunHit( int *pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::StunHitScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( StunHitScore, nValue );
	return true;
}

bool MACP::UpdateSuperAmmorBreak( int *pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::SuperAmmorBreakScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( SuperAmmorBreakScore, nValue );
	return true;
}

bool MACP::UpdateBrokenShield( int *pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::BrokenShieldScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( BrokenShieldScore, nValue );
	return true;
}

bool MACP::UpdateGenocide( int *pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::GenocideScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( GenocideScore, nValue );
	return true;
}

bool MACP::UpdateAirCombo( int *pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::AirComboScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( AirComboScore, nValue );
	return true;
}

bool MACP::UpdatePartyCombo( int nCombo, int *pResult )
{
	int nValue = nCombo * m_Data.pScore[CPTypeEnum::PartyComboScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( PartyComboScore, nValue );
	return true;
}

bool MACP::UpdateRebirthPlayer( int *pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::RebirthPlayerScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( RebirthPlayerScore, nValue );
	return true;
}

bool MACP::UpdateUseSkill( DnSkillHandle hSkill, int *pResult )
{
	if( !hSkill ) return false;

	int nValue = hSkill->GetCPScore();
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( UseSkillScore, nValue );
	return true;
}

bool MACP::UpdateCombo( int nCombo, int *pResult )
{
	int nValue = nCombo * m_Data.pScore[CPTypeEnum::ComboScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( ComboScore, nValue );
	return true;
}

bool MACP::UpdateAttackedHit( int *pResult )
{
	m_Data.pScore[CPTypeEnum::AttackedCount]++;
	int nValue = m_Data.pScore[CPTypeEnum::AttackedHitScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( AttackedHitScore, nValue );
	return true;
}

bool MACP::UpdateAttackedCriticalHit( int *pResult )
{
	m_Data.pScore[CPTypeEnum::AttackedCount]++;
	int nValue = m_Data.pScore[CPTypeEnum::AttackedCriticalHitScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( AttackedCriticalHitScore, nValue );
	return true;
}

bool MACP::UpdateAttackedStunHit( int *pResult )
{
	m_Data.pScore[CPTypeEnum::AttackedCount]++;
	int nValue = m_Data.pScore[CPTypeEnum::AttackedStunHitScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( AttackedStunHitScore, nValue );
	return true;
}

bool MACP::UpdateDead( int *pResult )
{
	m_Data.pScore[CPTypeEnum::DieCount]++;
	int nValue = m_Data.pScore[CPTypeEnum::DeadScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( DieCount, nValue );
	return true;
}

bool MACP::UpdatePartyMemberDead( int *pResult )
{
	m_Data.nPartyMemberDieCount++;
	return true;
}

bool MACP::UpdateStartPartyCount( int nPartyCount )
{
	m_Data.nStartPartyCount = nPartyCount;
	return true;
}

bool MACP::UpdatePropBreak( int* pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::PropBreakScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( PropBreakScore, nValue );
	return true;
}

bool MACP::UpdateGetItem( int* pResult )
{
	int nValue = m_Data.pScore[CPTypeEnum::GetItemScore];
	if( nValue == 0 ) return false;
	m_Data.nAccumulationCP += nValue;
	if( pResult ) *pResult = nValue;
	OnEventCP( GetItemScore, nValue );
	return true;
}

void MACP::OnEventCP( CPTypeEnum Type, int nResult )
{
	m_Data.nAccumulationCP = max( m_Data.nAccumulationCP, 0 );
}

bool MACP::UpdateAssistScore( int myAggro, int wholeAggro )
{
	int nValue = m_Data.pScore[CPTypeEnum::AssistMonsterScore];
	if( nValue == 0 ) return false;
	int result = (int)((float(myAggro) / float(wholeAggro)) * (float)nValue);
	m_Data.nAccumulationCP += result;
	OutputDebug("[CP] ASSIST : %d/%d nValue:%d, calc:%d, acc:%d\n", myAggro, wholeAggro, nValue, result, m_Data.nAccumulationCP);
	OnEventCP( AssistMonsterScore, result );
	return true;
}

bool MACP::UpdateMaxLevelGainExperience( int nExp )
{
	int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
	if( m_pActor->GetLevel() < nLevelLimit ) return false;

	m_Data.nMaxLevelGainExperience += nExp;
	return true;
}