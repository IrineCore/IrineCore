#include "stdafx.h"
#include "DnTeleportBlow.h"

#include "DnMonsterActor.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnTeleportBlow::CDnTeleportBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																				m_dwMonsterIDToTeleport( 0 ),
																				m_fTeleportRangeMax( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_205;
	SetValue( szValue );

	m_fValue = 0.0f;

	std::string str = szValue;
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	TokenizeA(str, tokens, delimiters);

	m_dwMonsterIDToTeleport = atoi( tokens.at( 0 ).c_str() );
	m_fTeleportRangeMax = (float)atof( tokens.at( 1 ).c_str() );
}

CDnTeleportBlow::~CDnTeleportBlow(void)
{

}

void CDnTeleportBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);

#if defined(_GAMESERVER)
	if (!m_hActor)
		return;

	//
	EtVector3* pDestPosition = NULL;
	float fShortestDistanceSQ = FLT_MAX;

	/*
	DNVector(CEtWorldProp*) vlPropList;

	CDnWorld* pWorld = NULL;
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance(m_hActor->GetRoom()).GetTask( "GameTask" );
	pWorld = pGameTask ? &CDnWorld:GetInstance(pGameTask->GetRoom()) : NULL;


	if (pWorld)
		pWorld->ScanProp(*m_hActor->GetPosition(), pWorld->GetGridWidth(), vlPropList);

	int nScanPropCount = vlPropList.size();
	for (int i = 0; i < nScanPropCount; ++i)
	{
		CEtWorldProp* pWorldProp = vlPropList[i];
		if (pWorldProp && pWorldProp->GetClassID() == m_nObejctID &&
			pWorldProp->GetMasterActor() == m_hActor)
		{
			EtVector3 vDistance = (*m_hActor->GetPosition()) - (*pWorldProp->GetPosition());

			float fLengthSQ = EtVec3LengthSq( &vDistance );
			if( fLengthSQ < fShortestDistanceSQ )
			{
				fShortestDistanceSQ = fLengthSQ;
				pDestPosition = pWorldProp->GetPosition();
			}
		}
	}
	*/

	DNVector(DnActorHandle) vlMonsterActorList;
	m_hActor->ScanActor(m_hActor->GetRoom(), *m_hActor->GetPosition(), m_fTeleportRangeMax, vlMonsterActorList);
	int nMonsterActorCount = (int)vlMonsterActorList.size();
	for (int i = 0; i < nMonsterActorCount; ++i)
	{
		DnActorHandle hActor = vlMonsterActorList[i];
		if (hActor && hActor->IsMonsterActor())
		{
			CDnMonsterActor*	pMonster	= static_cast<CDnMonsterActor*>(hActor.GetPointer());
			if( pMonster &&
				(pMonster->GetMonsterClassID() == m_dwMonsterIDToTeleport) &&
				(pMonster->GetSummonerPlayerActor() == m_hActor) )
			{
				EtVector3 vDistance = (*m_hActor->GetPosition()) - (*pMonster->GetPosition());

				float fLengthSQ = EtVec3LengthSq( &vDistance );
				if( fLengthSQ < fShortestDistanceSQ )
				{
					fShortestDistanceSQ = fLengthSQ;
					pDestPosition = pMonster->GetPosition();
				}
			}
		}
	}

	//Once the final target position is found...
	if (pDestPosition)
	{
		m_hActor->SetPosition(*pDestPosition);
		m_hActor->CmdWarp();
	}
#endif // _GAMESERVER
}

void CDnTeleportBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	
}


void CDnTeleportBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnTeleportBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	DWORD dwMonsterID[2] = {0, };
	float fTeleportMaxRange[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		dwMonsterID[0] = atoi( vlTokens[0][0].c_str() );
		fTeleportMaxRange[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		dwMonsterID[1] = atoi( vlTokens[1][0].c_str() );
		fTeleportMaxRange[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	DWORD dwResultMonsterID = dwMonsterID[0];
	float fResultMaxRange = max(fTeleportMaxRange[0], fTeleportMaxRange[1]);

	sprintf_s(szBuff, "%d;%f", dwResultMonsterID, fResultMaxRange);

	szNewValue = szBuff;
}

void CDnTeleportBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	DWORD dwMonsterID[2] = {0, };
	float fTeleportMaxRange[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		dwMonsterID[0] = atoi( vlTokens[0][0].c_str() );
		fTeleportMaxRange[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		dwMonsterID[1] = atoi( vlTokens[1][0].c_str() );
		fTeleportMaxRange[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	DWORD dwResultMonsterID = dwMonsterID[0];
	float fResultMaxRange = min(fTeleportMaxRange[0], fTeleportMaxRange[1]);

	sprintf_s(szBuff, "%d;%f", dwResultMonsterID, fResultMaxRange);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
