#include "stdafx.h"
#include "DnActorStatIntervalManipulator.h"
#include "DnDamageBase.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnActorStatIntervalManipulator::CDnActorStatIntervalManipulator( DnActorHandle hActor, DnBlowHandle hParentBlow ) : m_hActor( hActor ),
																						   m_hParentBlow( hParentBlow ),
																						   m_LocalStartTime( 0 ),
																						   m_dwInterval( 0 ),
																						   m_LocalElapsedTime( 0 ),
																						   m_LastIntervalTime( 0 )
																	 
{
}

CDnActorStatIntervalManipulator::~CDnActorStatIntervalManipulator(void)
{
}


void CDnActorStatIntervalManipulator::OnBegin( LOCAL_TIME LocalTime, DWORD dwInterval )
{
	m_LocalElapsedTime = 0;

	m_LocalStartTime = LocalTime;
	m_LastIntervalTime = LocalTime;
	m_dwInterval = dwInterval;
}

//void CDnActorStatIntervalManipulator::_ProcessInterval( int& iProcessManipulateCount, int iDestProcessCount )
//{
//	if( !m_hActor || m_hActor->IsDie() )
//		return;
//
//#ifdef _GAMESERVER
//	// The state effect owning this object must implement a periodic processing function.
//	bool bResult = m_hParentBlow->OnCustomIntervalProcess();
//	_ASSERT( bResult );
//
//	// If killed by a Blow, set the Blow caster Actor as Hitter.
//	if( m_hActor->IsDie() )
//	{
//		DnActorHandle hHitter;
//		if( m_hParentBlow )
//		{
//			const CDnSkill::SkillInfo* pSkillInfo = m_hParentBlow->GetParentSkillInfo();
//			if( pSkillInfo )
//				hHitter = pSkillInfo->hSkillUser;
//		}
//
//		m_hActor->Die( hHitter );
//	}
//#endif
//}

void CDnActorStatIntervalManipulator::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( 0 == LocalTime )
		return;

	if( !m_hActor || m_hActor->IsDie() )
		return;

	// Since LocalTime resets on zone transition..
	if( LocalTime < m_LastIntervalTime )
		m_LastIntervalTime = LocalTime;

	if( m_dwInterval < LocalTime - m_LastIntervalTime )
	{
		// The state effect owning this object must implement a periodic processing function.
		bool bResult = m_hParentBlow->OnCustomIntervalProcess();
		_ASSERT( bResult );

		m_LastIntervalTime = LocalTime;

#ifdef _GAMESERVER
		// If killed by Blow, set the Blow caster Actor as Hitter.
		// Important!!
		// Die processing must always happen last.
		// When Die is processed, the state effect disappears, so this object held by that state effect also disappears.
		// Therefore, no manipulation must occur.

#if defined(PRE_FIX_61382)
		DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
		if( hActor && hActor->IsDie() )
		{
			DnActorHandle hHitter;
			if( m_hParentBlow )
			{
				const CDnSkill::SkillInfo* pSkillInfo = m_hParentBlow->GetParentSkillInfo();
				if( pSkillInfo )
					hHitter = pSkillInfo->hSkillUser;
			}

			hActor->Die( hHitter );

			// Do not modify any variable values or perform processing below this point!
			return;
		}
#else
		if( m_hActor->IsDie() )
		{
			DnActorHandle hHitter;
			if( m_hParentBlow )
			{
				const CDnSkill::SkillInfo* pSkillInfo = m_hParentBlow->GetParentSkillInfo();
				if( pSkillInfo )
					hHitter = pSkillInfo->hSkillUser;
			}

			m_hActor->Die( hHitter );

			// Do not modify any variable values or perform processing below this point!
			return;
		}
#endif // PRE_FIX_61382
#endif
	}
}


void CDnActorStatIntervalManipulator::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	//if( false == m_hActor || m_hActor->IsDie() )
	//	return;

	//if( m_hParentBlow->GetDurationTime() > 0.0f )
	//{
	//	// If interrupted mid-way, ServerEndTime was already updated when CmdRemoveStateEffect packet was sent..
	//	LOCAL_TIME WholeElapsed = m_ServerEndTime - m_ServerStartTime;
	//	int iChangedProcessCount = int(WholeElapsed / m_dwInterval) + 1;

	//	if( m_iProcessedManipulateCount < iChangedProcessCount )
	//		_ProcessInterval( m_iProcessedManipulateCount, iChangedProcessCount );
	//}
	//else
	//{	
	//	_ProcessInterval( m_iProcessedManipulateCount, m_iExpectedManipulateCount );
	//}
}
