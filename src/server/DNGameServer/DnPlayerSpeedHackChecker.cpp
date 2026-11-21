
#include "stdafx.h"
#include "DnPlayerSpeedHackChecker.h"
#include "DnPlayerActor.h"
#include "GameSendPacket.h"
#include "DNLogConnection.h"
#include "DnGameTask.h"

bool CDnPlayerSpeedHackChecker::OnSyncDatumGap( const DWORD dwGap )
{
	const DWORD dwSyncTick = 5000;
	if( GetTickCount() - m_pActor->GetSyncDatumSendTick() < dwSyncTick )
		return true;

	// Collect samples
	DWORD dwServerGap	= m_pActor->GetSyncDatumGap();
	DWORD dwAddGap		= 0;
	if( dwServerGap < dwGap )
	{
		dwAddGap = (dwGap-dwServerGap);

		// Allowed threshold (10%)
		const float fAllowDoubt = 1.1f;
		if( dwGap > static_cast<DWORD>(dwServerGap*fAllowDoubt) )
			++m_uiDoubtCount;
	}

	// Save start-end gap info
	if( m_uiGapCount == 0 )
		m_dwFirstGap = dwAddGap;
	m_dwLastGap = dwAddGap;

	// Sample count
	m_dwGapSum += dwAddGap;
	++m_uiGapCount;

	// Sample collection interval
	const UINT uiSampleSec = 10000;
	if( GetTickCount()-m_dwCheckTick >= uiSampleSec )
	{
		// Min sample count
		const UINT uiMinSampleCount	= 10;
		if( uiMinSampleCount <= m_uiGapCount )
		{
			bool bUpdate = false;

			// Allowed threshold (10%)
			const UINT	uiAllowGap	= static_cast<UINT>(uiSampleSec*0.1);
			const DWORD dwAvgGap	= m_dwGapSum/m_uiGapCount;

			UINT uiDatumValue	= 0;
			UINT uiCheckValue	= 0;
			UINT uiCheckType	= CheckType::Max;

			if( dwAvgGap > uiAllowGap )
			{
				++m_uiHackCount;
				bUpdate = true;

				uiDatumValue	= uiAllowGap;
				uiCheckValue	= dwAvgGap;
				uiCheckType		= CheckType::Average;
			}
			else
			{
				// Allowed threshold (40%)
				const float fAllowCountRate = 0.40f;
				if( fAllowCountRate < m_uiDoubtCount/(float)m_uiGapCount )
				{
					++m_uiHackCount;
					bUpdate = true;

					uiDatumValue	= static_cast<UINT>(fAllowCountRate*100);
					uiCheckValue	= static_cast<int>((m_uiDoubtCount/(float)m_uiGapCount)*100);
					uiCheckType		= CheckType::DoubtPacketCount;
				}
			}

			// Check start-end gap
			if( bUpdate )
			{
				if( m_dwLastGap > m_dwFirstGap )
				{
					DWORD dwTemp = m_dwLastGap - m_dwFirstGap;
					if( uiAllowGap > dwTemp )
						bUpdate = false;
				}
				else
					bUpdate = false;

				if( bUpdate == false )
				{
					if( m_uiHackCount )
						--m_uiHackCount;
				}
			}

			if( bUpdate )
			{
				bool bAddPenalty = false;
				if( m_uiHackCount >= HackPenanty::Common::SpeedHackAllowCount && m_pActor->GetGameRoom() )
				{
					m_pActor->GetGameRoom()->AddHackPenalty( HackPenanty::Common::SpeedHack );
					bAddPenalty = true;
				}

				g_Log.Log(LogType::_HACK, m_pActor->GetUserSession(), L"HackChecker(SpeedHack) : CharName=%s AddPenalty=%d\n", m_pActor->GetUserSession()->GetCharacterName(), bAddPenalty );
				m_pActor->OnInvalidPlayerChecker( 1 );
			}
		}

		m_dwCheckTick	= GetTickCount() + dwSyncTick ;
		m_dwGapSum		= 0;
		m_uiGapCount	= 0;
		m_uiDoubtCount	= 0;
		m_dwFirstGap	= 0;
		m_dwLastGap		= 0;

		// Base time sync
		SendGameDatumTick( m_pActor->GetUserSession() );
	}

	return true;
}

void CDnPlayerSpeedHackChecker::InsertPos( const EtVector3 &vPos, DWORD dwTime, int nMoveSpeed )
{
	PosHistory Struct;
	Struct.vPos = vPos;
	Struct.dwTime = dwTime;
	Struct.nMoveSpeed = nMoveSpeed;

	m_VecPosList.push_back( Struct );

	for( DWORD i=0; i<m_VecPosList.size() - 1; i++ ) {
		if( dwTime - m_VecPosList[i].dwTime > 5000 ) {
			m_VecPosList.erase( m_VecPosList.begin() + i );
			i--;
		}
	}
}

bool CDnPlayerSpeedHackChecker::IsValidDist()
{
	if( m_VecPosList.size() < 2 ) return true;
	float fDist = 0.f;
	EtVector3 vTemp;
	DWORD dwTick = 0;
	int nMoveSpeed = 0;

	for( DWORD i=0; i<m_VecPosList.size()-1; i++ ) {
		vTemp = m_VecPosList[i].vPos - m_VecPosList[i+1].vPos;
		vTemp.y = 0.f;
		fDist += EtVec3Length( &vTemp );
		dwTick += m_VecPosList[i+1].dwTime - m_VecPosList[i].dwTime;
		nMoveSpeed += m_VecPosList[i].nMoveSpeed;
	}

	if( dwTick == 0 )
		return true;

	nMoveSpeed += m_VecPosList[m_VecPosList.size()-1].nMoveSpeed;
	nMoveSpeed /= (int)m_VecPosList.size();

	if( fDist / (float)dwTick > ( nMoveSpeed / 1000.f ) * 1.2f ) return false;
	return true;
}

void CDnPlayerSpeedHackChecker::Process( LOCAL_TIME LocalTime, float fDelta )
{

}

bool CDnPlayerSpeedHackChecker::OnSyncMoveSpeed( const int nMoveSpeed )
{
	if( !m_pActor )
		return false;

	if( m_pActor->IsDeveloperAccountLevel() == true )
		return false;

	if( m_pActor->GetGameRoom() && m_pActor->GetGameRoom()->GetGameServer() )
	{
		if( !m_pActor->GetGameRoom()->GetGameServer()->IsAcceptRoom() ) { // Skip check if current game server Frame is abnormal.
			return false;
		}
	}

	if( timeGetTime() - m_dwLastMoveSpeedHackTick < 1000 )
		return false;

	m_dwLastMoveSpeedHackTick = timeGetTime();

	if( m_pActor->CDnActor::GetMoveSpeed() != nMoveSpeed )
	{
		m_uiMoveSpeedHackCount++;
	}

	if( m_uiMoveSpeedHackCount >= 15 )
	{
		int nSkillSpeed = 0;
		int nSkillIndex = 0;

		if( m_pActor->GetStateStep(1) )
			nSkillSpeed = m_pActor->GetStateStep(1)->GetMoveSpeed();
		if( m_pActor->GetProcessSkill() )
			nSkillIndex = m_pActor->GetProcessSkill()->GetClassID();

		g_Log.Log(LogType::_HACK, m_pActor->GetUserSession(), L"HackChecker(MoveSpeedHack) ServerSpeed(%d) ServerSkillSpeed(%d) ClientSpeed(%d) SkillIndex(%d)" , m_pActor->CDnActor::GetMoveSpeed(), nSkillSpeed, nMoveSpeed, nSkillIndex  );
		m_pActor->OnInvalidPlayerChecker( 1 );
		m_uiMoveSpeedHackCount = 0;
	}

	if( timeGetTime() - m_dwLastMoveSpeedCheckTick >= 20000 ) 
	{
		m_uiMoveSpeedHackCount = 0;
		m_dwLastMoveSpeedCheckTick = timeGetTime();
	}

	return true;
}


bool CDnPlayerSpeedHackChecker::OnSyncPosition( const EtVector3& vPos )
{
	if( m_pActor->GetGameRoom() && m_pActor->GetGameRoom()->GetGameServer() )
	{
		// Skip check if current game server Frame is abnormal.
		if( !m_pActor->GetGameRoom()->GetGameServer()->IsAcceptRoom() ) {
			return false;
		}
	}
	// Check position hack every 1 second
	if( timeGetTime()-m_dwLastPosHackTick < 1000 )
		return false;
	m_dwLastPosHackTick = timeGetTime();

	float fAllowGap = max( m_pActor->CDnActor::GetMoveSpeed(), ((MAWalkMovement*)m_pActor->GetMovement())->GetAverageAniDist() );

	bool bIgnorePress = false;
	if( m_bLastCheckPress ) {
		fAllowGap += (int)m_fPressDist;
		bIgnorePress = true;
		m_bLastCheckPress = false;
	}

	float fDistSq	= (fAllowGap) * (fAllowGap);
	float fMoveDist = EtVec3LengthSq( &(vPos - *m_pActor->GetPosition()) ); // Move delta 
	
	if( fMoveDist >= fDistSq && !bIgnorePress )
	{
		m_dwLastInvalidTick = m_dwLastPosHackTick;
		m_nValidCount = 0;
		++m_uiPosHackCount;
		++m_nInvalidCount;

		if( m_uiPosHackCount >= 3 )
		{
			bool bHackPenalty = false;
			if( m_uiPosHackCount >= HackPenanty::Common::PosHackAllowCount && m_pActor->GetGameRoom() )
			{
				bHackPenalty = true;
			}
		}
		if( m_nInvalidCount >= 3 ) {
			m_pActor->OnInvalidPlayerChecker( 1 );
			g_Log.Log(LogType::_HACK, m_pActor->GetUserSession(), L"HackChecker(PositionHack) : CharName=%s, InvalidCount=%d, ValidCount=%d, Job=%d, Action=%S, State=%d, MapIndex=%d, Position=%.2f,%.2f,%.2f -> %.2f,%.2f,%.2f(MoveSpeed : %d/AvgDist : %.2f)\n", 
				m_pActor->GetUserSession()->GetCharacterName(), m_nInvalidCount, m_nValidCount, m_pActor->GetJobClassID(), m_pActor->GetCurrentAction(), m_pActor->GetState(), ( m_pActor->GetGameRoom() && m_pActor->GetGameRoom()->GetGameTask() ) ? m_pActor->GetGameRoom()->GetGameTask()->GetMapTableID() : -1, m_pActor->GetPosition()->x, m_pActor->GetPosition()->y, m_pActor->GetPosition()->z, vPos.x, vPos.y, vPos.z, m_pActor->CDnActor::GetMoveSpeed(), ((MAWalkMovement*)m_pActor->GetMovement())->GetAverageAniDist() );
		}
	}
	else m_nValidCount++;

	if( m_nValidCount > 10 ) {
		m_nInvalidCount = 0;
	}
	if( m_nInvalidCount >= 3 && m_nValidCount < 10 ) {
		if( timeGetTime() - m_dwLastInvalidTick >= 10000 ) {
			m_nInvalidCount = 0;
			m_nValidCount = 0;
		}
	}

	return true;
}

bool CDnPlayerSpeedHackChecker::IsInvalidPlayer()
{
	return ( m_nInvalidCount >= 3 ) ? true : false;
}


void CDnPlayerSpeedHackChecker::ResetInvalid()
{
	m_nInvalidCount = 0;
	m_nValidCount = 0;
	m_VecPosList.clear();
}

#if defined(PRE_ADD_POSITIONHACK_POS_LOG)
void CDnPlayerSpeedHackChecker::CheckInvalidActorAndPositionLog()
{
	if(m_nInvalidCount <= 0) return;
	if(!m_pActor) return;
	if(!m_pActor->GetUserSession()) return;
	g_Log.Log(LogType::_HACK, m_pActor->GetUserSession(), L"HackChecker(InvalidActorClickProp) : CharName=%s, InvalidCount=%d, ValidCount=%d, Job=%d, Action=%S, State=%d, MapIndex=%d, Position=%.2f,%.2f,%.2f\n", m_pActor->GetUserSession()->GetCharacterName(), m_nInvalidCount, m_nValidCount, m_pActor->GetJobClassID(), m_pActor->GetCurrentAction(), m_pActor->GetState(), ( m_pActor->GetGameRoom() && m_pActor->GetGameRoom()->GetGameTask() ) ? m_pActor->GetGameRoom()->GetGameTask()->GetMapTableID() : -1, m_pActor->GetPosition()->x, m_pActor->GetPosition()->y, m_pActor->GetPosition()->z );
}
#endif	// #if defined(PRE_ADD_POSITIONHACK_POS_LOG)