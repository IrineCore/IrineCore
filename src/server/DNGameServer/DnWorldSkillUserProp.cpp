#include "stdafx.h"
#include "DnWorldSkillUserProp.h"
#include "DnSkill.h"


CDnWorldSkillUserProp::CDnWorldSkillUserProp(CMultiRoom* pRoom) : CDnWorldActProp( pRoom ),
																  m_iUseType( ONCE ),
																  m_fCheckRadius( 0.0f ),
																  m_iActivateElementIndex( -1 ),
																  m_iIdleElementIndex(0),
																  m_iNowActionElementIndex(0)
{

}

CDnWorldSkillUserProp::~CDnWorldSkillUserProp(void)
{
}

bool CDnWorldSkillUserProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	if( CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale ) == false ) return false;
	if( m_hMonster ) *m_hMonster->GetMatEx() = *GetMatEx();
	return true;
}

bool CDnWorldSkillUserProp::InitializeTable(int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	if( GetData() )
	{
		SkillUserStruct* pStruct = (SkillUserStruct*)GetData();

		// Temporarily just create a goblin actor. (Monster table ID 101)
		int iMonsterTableID = 101;
		int iSkillID = pStruct->SkillTableID;
		int iSkillLevel = pStruct->SkillLevel;
		m_fCheckRadius = pStruct->fRadius;
		m_iUseType = pStruct->UseType;
	}

	return true;
}


bool CDnWorldSkillUserProp::CreateObject()
{
	bool bResult = CDnWorldActProp::CreateObject();

	m_iIdleElementIndex = GetElementIndex( "Idle" );
	m_iActivateElementIndex = GetElementIndex( "Activate" );

	return bResult;
}

void CDnWorldSkillUserProp::Process( LOCAL_TIME LocalTime, float fDelta )
{
	*(m_hMonster->GetMatEx()) = m_Cross;
	m_iNowActionElementIndex = GetElementIndex( GetCurrentAction() );

	// TODO: Since this is a test for now, let it activate immediately.
	if( m_iActivateElementIndex != m_iNowActionElementIndex )
		CmdAction( "Activate" );

	// TODO: Inevitably have to run ScanActor every frame. Need to scan differently based on friend/foe distinction.
	// For now, act like an aura via range check.
	DNVector(DnActorHandle) vlNeighboringActors;
	CDnActor::ScanActor( GetRoom(), *GetPosition(), m_fCheckRadius, vlNeighboringActors );
	if( false == vlNeighboringActors.empty() )
	{
		// Update affected actors
		set<DWORD> setPrevAffectedActorsID = m_setAffectedActorsID;

		m_setAffectedActorsID.clear();
		for( int i = 0; i < (int)vlNeighboringActors.size(); ++i )
			m_setAffectedActorsID.insert( vlNeighboringActors.at(i)->GetUniqueID() );

		switch( m_iUseType )
		{
			case ONCE:
				{

				}
				break;

			case CONTINUOUS:
				{
					// Select newly added and removed elements using set difference.
					vector<DWORD> vlNewMember;
					set_difference( m_setAffectedActorsID.begin(), m_setAffectedActorsID.end(), setPrevAffectedActorsID.begin(), setPrevAffectedActorsID.end(), 
									back_inserter(vlNewMember) );

					vector<DWORD> vlOutMember;
					set_difference( setPrevAffectedActorsID.begin(), setPrevAffectedActorsID.end(), m_setAffectedActorsID.begin(), m_setAffectedActorsID.end(),
									back_inserter(vlOutMember) );

					int iNumNewMember = (int)vlNewMember.size();
					for( int iMember = 0; iMember < iNumNewMember; ++iMember )
					{
						// Add state effects held by the skill...
						DnActorHandle hNewMember = CDnActor::FindActorFromUniqueID( GetRoom(), vlNewMember.at(iMember) );
						if( hNewMember )
						{
							DWORD dwNumStateEffect = m_hSkill->GetStateEffectCount();
							for( DWORD dwStateEffect = 0; dwStateEffect < dwNumStateEffect; ++dwStateEffect )
							{
								const CDnSkill::StateEffectStruct* pSE = m_hSkill->GetStateEffectFromIndex( dwStateEffect );
								hNewMember->CmdAddStateEffect( m_hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pSE->nID, -1/*SE.nDurationTime*/, pSE->szValue.c_str() );
							}
						}
					}

					int iNumOutMember = (int)vlOutMember.size();
					for( int iMember = 0; iMember < iNumOutMember; ++iMember )
					{
						// Remove state effects held by the skill.
						DnActorHandle hOutMember = CDnActor::FindActorFromUniqueID( GetRoom(), vlOutMember.at(iMember) );
						if( hOutMember )
						{
							DWORD dwNumStateEffect = m_hSkill->GetStateEffectCount();
							for( DWORD dwStateEffect = 0; dwStateEffect < dwNumStateEffect; ++dwStateEffect )
							{
								const CDnSkill::StateEffectStruct* pSE = m_hSkill->GetStateEffectFromIndex( dwStateEffect );
								hOutMember->CmdRemoveStateEffect( (STATE_BLOW::emBLOW_INDEX)pSE->nID );
								//hOutMember->DelStateBlow( (STATE_BLOW::emBLOW_INDEX)SE.nID );
							}
						}
					}
				}
				break;
		}

	}
}

void CDnWorldSkillUserProp::OnSignal( SignalTypeEnum Type, void* pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	//switch( Type )
	//{
	//	case STE_UsePropSkill:
	//		{
	//			CmdAction( "Activate" );
	//		}
	//		break;
	//}
}