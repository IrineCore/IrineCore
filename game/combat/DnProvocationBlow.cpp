#include "stdafx.h"
#include "DnProvocationBlow.h"
#if defined( _GAMESERVER )
#include "DNAggroSystem.h"
#endif // #if defined( _GAMESERVER )

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

// Must end state effect immediately if distance exceeds threshold; otherwise keeps chasing.
const float PROVOKE_BOUNDARY = 1000.0f * 1000.0f;

CDnProvocationBlow::CDnProvocationBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_132;

	SetValue( szValue );
	m_fValue = (float)atof(szValue);

	//// 3 arguments: value; distance; angle relative to character front.
	//string strValue( szValue );

	//// After splitting the string by whitespace
	//std::vector<string> vlTokens;
	//TokenizeA( strValue, vlTokens, ";" );

	//bool bValidArgument = (3 == (int)vlTokens.size());
	//_ASSERT( bValidArgument && "Invalid argument setting for Provoke state effect #132." );
	//if( bValidArgument )
	//{
	//	m_fValue = (float)atoi(vlTokens.at( 0 ).c_str());
	//	m_fRange = (float)atof(vlTokens.at( 1 ).c_str());
	//	m_fRangeAngleByRadian = EtToRadian((float)atof(vlTokens.at( 2 ).c_str()));
	//	m_fRangeSQ = m_fRange * m_fRange;
	//}
}

CDnProvocationBlow::~CDnProvocationBlow(void)
{

}

void CDnProvocationBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnProvocationBlow::OnBegin\n" );
	
//#if defined( _GAMESERVER )
//	if( IsPermanent() )
//	{
//		MatrixEx Cross = *m_hActor->GetMatEx();
//		EtVector3 vPlayerDir = Cross.m_vZAxis;
//		EtVec3Normalize( &vPlayerDir, &vPlayerDir );
//
//		CMultiRoom *pRoom = m_hActor->GetRoom();
//		for( DWORD i=0; i<STATIC_INSTANCE_(CDnActor::s_pVecProcessList).size(); i++ ) 
//		{
//			DnActorHandle hActor = STATIC_INSTANCE_(CDnActor::s_pVecProcessList)[i]->GetMySmartPtr();
//			if( hActor && hActor->IsMonsterActor() )
//			{
//				// Apply only to targets within a certain distance.
//				EtVector3 vDistance = *hActor->GetPosition() - *m_hActor->GetPosition();
//				float fDistanceSQ = EtVec3LengthSq( &vDistance );
//				if( fDistanceSQ <= m_fRangeSQ )
//				{
//					// Apply only to targets within a certain angle in front of the character.
//					EtVector3 vDirToMonster;
//					EtVec3Normalize( &vDirToMonster, &vDistance );
//					float fDot = EtVec3Dot( &vPlayerDir, &vDistance );
//					if( fDot <= m_fRangeAngleByRadian )
//					{
//						if( hActor->GetAggroSystem() )
//						{
//#if defined( PRE_FIX_22526 )
//							hActor->GetAggroSystem()->AddAggro( m_hActor, static_cast<int>(GetFloatValue()) );
//#else
//							CDNAggroSystem::AggroStruct* pStruct = hActor->GetAggroSystem()->GetAggroStruct( m_hActor );
//							if( pStruct )
//							{
//								pStruct->iAggro += static_cast<int>(GetFloatValue());
//							}
//#endif // #if defined( PRE_FIX_22526 )
//						}
//					}
//				}
//			}
//		}
//	}
//#endif // #if defined( _GAMESERVER )
}


void CDnProvocationBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#ifdef _GAMESERVER
	// Remove the state effect immediately if the distance between the player who used the skill and the monster with this active state effect exceeds 10 meters.
	bool bValid = true;
	if( m_ParentSkillInfo.hSkillUser )
	{
		// Remove immediately upon death as well.
		if( m_ParentSkillInfo.hSkillUser->IsDie() )
			bValid = false;

		//float fDistanceSQ = EtVec3LengthSq( &EtVector3(*m_hActor->GetPosition() - *m_ParentSkillInfo.hSkillUser->GetPosition()) );

		// Removed distance-check release per design team request. #26329
		// Will need to expose externally or adjust if control is needed later.
		//if( PROVOKE_BOUNDARY < fDistanceSQ )
		//{
		//	bValid = false;
		//}
	}
	else
		bValid = false;		// Remove immediately even when there is no player object.

	if( false == bValid )
	{
		// Schedule state effect deletion.
		m_hActor->AddReserveRemoveBlow( GetMySmartPtr() );
	}
#endif
}


void CDnProvocationBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{

	OutputDebug( "CDnProvocationBlow::OnEnd\n" );
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnProvocationBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	fValue[0] = (float)atof( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	fValue[1] = (float)atof( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] + fValue[1];

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}

void CDnProvocationBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	fValue[0] = (float)atof( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	fValue[1] = (float)atof( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Calculate value
	float fResultValue = fValue[0] - fValue[1];

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW