#include "stdafx.h"
#include "DnPropActor.h"
#include "DnSkill.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#ifdef _GAMESERVER
CDnPropActor::CDnPropActor( CMultiRoom* pRoom, int nClassID ) : CDnActor( pRoom, nClassID )
#else
CDnPropActor::CDnPropActor( int nClassID, bool bProcess ) : CDnActor( nClassID, bProcess )
#endif
{
	
}

CDnPropActor::~CDnPropActor(void)
{
	//SAFE_RELEASE_SPTRVEC( m_vlhSkillList );
}


bool CDnPropActor::Initialize( void )
{
	CDnActorState::Initialize( m_nClassID );
	CDnActor::Initialize();

	return true;
}

//
//bool CDnPropActor::AddPropSkill( int iSkillID, int iSkillLevelID )
//{
//	bool bResult = false;
//
//	DnSkillHandle hSkill = CDnSkill::CreateSkill( GetMySmartPtr(), iSkillID, iSkillLevelID );
//	if( hSkill )
//	{
//		m_vlhSkillList.push_back( hSkill );
//		bResult = true;
//	}
//
//	return bResult;
//}
//
//
//DnSkillHandle CDnPropActor::UsePropSkill( int iSkillID, LOCAL_TIME LocalTime, float fDelta )
//{
//	DnSkillHandle hUseSkill;
//
//	int iNumSkills = (int)m_vlhSkillList.size();
//	for( int iSkill = 0; iSkill < iNumSkills; ++iSkill )
//	{
//		DnSkillHandle hSkill = m_vlhSkillList.at( iSkill );
//		if( hSkill->GetClassID() == iSkillID )
//		{
//			hSkill->OnBegin( LocalTime, fDelta );
//
//			// NOTE: Replace MASkillUser's m_hProcessSkill with the current prop's skill.
//			// Automatically processed on the CDnActor side.
//			MASkillUser::m_hProcessSkill = hSkill;
//			hUseSkill = hSkill;
//			break;
//		}
//	}
//
//	return hUseSkill;
//}


// If a skill is used later, receive and process the packet here.
//void CDnPropActor::OnDispatchMessage( DWORD dwActorProtocol, BYTE *pPacket )
//{
//	
//}