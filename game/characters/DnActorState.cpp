#include "stdafx.h"
#include "DnActorState.h"
#include "DnTableDB.h"
#include "DnActor.h"
#include "DnWeapon.h"
#include "DnItem.h"
#include "MAPartsBody.h"
#include "DnMonsterActor.h"
#include "DnPlayerActor.h"
#include "PerfCheck.h"
#ifndef _GAMESERVER
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnPvPGameTask.h"
#endif
#ifdef _GAMESERVER
#include "DNGameDataManager.h"
#include "DNUserSession.h"
#include "DNBestFriend.h"
#endif // #ifdef _GAMESERVER

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

int CDnActorState::s_nActorStateIndex[CDnActorState::ActorStateEnum_Amount] = { 
	CDnActorState::None, 
	CDnActorState::Stay, 
	CDnActorState::Move,
	CDnActorState::Attack,
	CDnActorState::Hit,
	CDnActorState::Air,
	CDnActorState::Down,
	CDnActorState::Stun,
	CDnActorState::Stiff,
	CDnActorState::IgnoreCantAction,
	CDnActorState::IgnoreBackMoveSpeed,
};

char *CDnActorState::s_szActorStateString[CDnActorState::ActorStateEnum_Amount] = {
	"None",
	"Stay",
	"Move",
	"Attack",
	"Hit",
	"Air",
	"Down",
	"Stun",
	"Stiff",
	"IgnoreCantAction",
	"IgnoreBackMoveSpeed"
};

int CDnActorState::s_nActorCustomStateIndex[CDnActorState::ActorCustomStateEnum_Amount] = { 
	CDnActorState::Custom_None, 
	CDnActorState::Custom_Ground,
	CDnActorState::Custom_Fly,
	CDnActorState::Custom_UnderGround,
	CDnActorState::Custom_Fly2,
};

char *CDnActorState::s_szActorCustomStateString[CDnActorState::ActorCustomStateEnum_Amount] = {
	"Custom_None",
	"Custom_Ground",
	"Custom_Fly",
	"Custom_UnderGround",
};

float CDnActorState::s_fMaxStiffTime = 3.f;
float CDnActorState::s_fCriticalDamageProb = 2.0f;

CDnActorState::CDnActorState()
{
	m_nActorTableID = 0;
	m_nLevel = 0;
	m_ActorType = (ActorTypeEnum)-1;
	m_State = ActorStateEnum::Stay;
	m_CustomState = ActorCustomStateEnum::Custom_None;
	m_StateEffect = ActorStateEffectEnum::StateEffect_None;
	m_Material = ActorMaterialEnum::Flesh;
	m_fWeight = 0.f;
	m_nHeight = 0;
	m_nUnitSize = 0;
	m_fRotateAngleSpeed = 720.f;
	m_nPressLevel = 0;

	/*
	m_nStrength = 0;
	m_nAgility = 0;
	m_nIntelligence = 0;
	m_nStamina = 0;

	m_nMaxHP = m_nHP = 0;
	m_nMaxSP = m_nSP = 0;
	m_nMoveSpeed = 0;
	m_nAttackP[0] = m_nAttackP[1] = 0;
	m_nAttackM[0] = m_nAttackM[1] = 0;
	m_nDefenseP = 0;
	m_nDefenseM = 0;
	m_fDownDelayProb = 0.f;

	m_nStiff = 0;
	m_nStiffResistance = 0;
	m_nCritical = 0; 
	m_nCriticalResistance = 0;
	m_nStun = 0;
	m_nStunResistance = 0;

	memset( m_fElementAttack, 0, sizeof(m_fElementAttack) );
	memset( m_fElementDefense, 0, sizeof(m_fElementDefense) );
	*/
	m_bHittable = true;
	m_bMovable = true;
	m_bRotatable = true;

	m_pActor = NULL;

	m_Press = ActorPressEnum::Press_Circle;
	m_nUnitSizeParam1 = 0;
	m_nUnitSizeParam2 = 0;

	m_StateChangeTime = 0;

#ifdef PRE_ADD_SKILLBUF_RENEW
	m_bCopiedFromSummoner = false;
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW


#ifdef PRE_ADD_BESTFRIEND
	m_BFserial = 0;
#endif

}

CDnActorState::~CDnActorState()
{
}

void CDnActorState::Initialize( int nClassID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TACTOR );
	if( !pSox ) return;

	if( nClassID <= 0 ) return;

	m_nActorTableID = nClassID;
	m_Press = (ActorPressEnum)pSox->GetFieldFromLablePtr( nClassID, "_Press" )->GetInteger();	
	m_nUnitSizeParam1 = pSox->GetFieldFromLablePtr( nClassID, "_SizeParam1" )->GetInteger();
	m_nUnitSizeParam2 = pSox->GetFieldFromLablePtr( nClassID, "_SizeParam2" )->GetInteger();
	m_nUnitSize = max(m_nUnitSizeParam1, m_nUnitSizeParam2);	// Store the larger one.

#ifndef _GAMESERVER
#ifdef RENDER_PRESS_hayannal2009
	if( m_Press == Press_Circle ) {
		float fGap = (float)m_nUnitSizeParam1;
		float fGap2 = fGap / 1.414f;
		float fYGap = 10.0f;
		EtVector3 vCenter(0.0f, fYGap, 0.0f);
		for( int i = 0; i < 10; ++i ) m_PressVertices1[i].dwColor = 0xFF00FFFF;
		m_PressVertices1[ 0 ].Position = vCenter;
		m_PressVertices1[ 1 ].Position = EtVector3( vCenter.x,			vCenter.y,	vCenter.z + fGap	);
		m_PressVertices1[ 2 ].Position = EtVector3( vCenter.x + fGap2,	vCenter.y,	vCenter.z + fGap2	);
		m_PressVertices1[ 3 ].Position = EtVector3( vCenter.x + fGap,	vCenter.y,	vCenter.z			);
		m_PressVertices1[ 4 ].Position = EtVector3( vCenter.x + fGap2,	vCenter.y,	vCenter.z - fGap2	);
		m_PressVertices1[ 5 ].Position = EtVector3( vCenter.x,			vCenter.y,	vCenter.z - fGap	);
		m_PressVertices1[ 6 ].Position = EtVector3( vCenter.x - fGap2,	vCenter.y,	vCenter.z - fGap2	);
		m_PressVertices1[ 7 ].Position = EtVector3( vCenter.x - fGap,	vCenter.y,	vCenter.z			);
		m_PressVertices1[ 8 ].Position = EtVector3( vCenter.x - fGap2,	vCenter.y,	vCenter.z + fGap2	);
		m_PressVertices1[ 9 ].Position = EtVector3( vCenter.x,			vCenter.y,	vCenter.z + fGap	);
	}
	else if( m_Press == Press_Capsule ) {
		float fGap = (float)min(m_nUnitSizeParam1, m_nUnitSizeParam2);
		float fGap2 = fGap / 1.414f;
		float fYGap = 10.0f;
		int nSizeX = m_nUnitSizeParam1;
		int nSizeZ = m_nUnitSizeParam2;
		EtVector3 vCenter1, vCenter2;
		if( nSizeX < nSizeZ ) {
			vCenter1.x = vCenter2.x = 0.0f;
			vCenter1.y = vCenter2.y = fYGap;
			vCenter1.z = (float)(nSizeZ - nSizeX);
			vCenter2.z = -vCenter1.z;
		}
		else {
			vCenter1.z = vCenter2.z = 0.0f;
			vCenter1.y = vCenter2.y = fYGap;
			vCenter1.x = (float)(nSizeX - nSizeZ);
			vCenter2.x = -vCenter1.x;
		}
		// 8 1 2
		// 7 0 3
		// 6 5 4
		for( int i = 0; i < 10; ++i ) m_PressVertices1[i].dwColor = 0xFF00FFFF;
		m_PressVertices1[ 0 ].Position = vCenter1;
		m_PressVertices1[ 1 ].Position = EtVector3( vCenter1.x,			vCenter1.y,	vCenter1.z + fGap	);
		m_PressVertices1[ 2 ].Position = EtVector3( vCenter1.x + fGap2,	vCenter1.y,	vCenter1.z + fGap2	);
		m_PressVertices1[ 3 ].Position = EtVector3( vCenter1.x + fGap,	vCenter1.y,	vCenter1.z			);
		m_PressVertices1[ 4 ].Position = EtVector3( vCenter1.x + fGap2,	vCenter1.y,	vCenter1.z - fGap2	);
		m_PressVertices1[ 5 ].Position = EtVector3( vCenter1.x,			vCenter1.y,	vCenter1.z - fGap	);
		m_PressVertices1[ 6 ].Position = EtVector3( vCenter1.x - fGap2,	vCenter1.y,	vCenter1.z - fGap2	);
		m_PressVertices1[ 7 ].Position = EtVector3( vCenter1.x - fGap,	vCenter1.y,	vCenter1.z			);
		m_PressVertices1[ 8 ].Position = EtVector3( vCenter1.x - fGap2,	vCenter1.y,	vCenter1.z + fGap2	);
		m_PressVertices1[ 9 ].Position = EtVector3( vCenter1.x,			vCenter1.y,	vCenter1.z + fGap	);

		for( int i = 0; i < 10; ++i ) m_PressVertices2[i].dwColor = 0xFF00FFFF;
		m_PressVertices2[ 0 ].Position = vCenter2;
		m_PressVertices2[ 1 ].Position = EtVector3( vCenter2.x,			vCenter2.y,	vCenter2.z + fGap	);
		m_PressVertices2[ 2 ].Position = EtVector3( vCenter2.x + fGap2,	vCenter2.y,	vCenter2.z + fGap2	);
		m_PressVertices2[ 3 ].Position = EtVector3( vCenter2.x + fGap,	vCenter2.y,	vCenter2.z			);
		m_PressVertices2[ 4 ].Position = EtVector3( vCenter2.x + fGap2,	vCenter2.y,	vCenter2.z - fGap2	);
		m_PressVertices2[ 5 ].Position = EtVector3( vCenter2.x,			vCenter2.y,	vCenter2.z - fGap	);
		m_PressVertices2[ 6 ].Position = EtVector3( vCenter2.x - fGap2,	vCenter2.y,	vCenter2.z - fGap2	);
		m_PressVertices2[ 7 ].Position = EtVector3( vCenter2.x - fGap,	vCenter2.y,	vCenter2.z			);
		m_PressVertices2[ 8 ].Position = EtVector3( vCenter2.x - fGap2,	vCenter2.y,	vCenter2.z + fGap2	);
		m_PressVertices2[ 9 ].Position = EtVector3( vCenter2.x,			vCenter2.y,	vCenter2.z + fGap	);

		// 1 2
		// 0 3
		for( int i = 0; i < 4; ++i ) m_PressVertices3[i].dwColor = 0xFF00FFFF;
		if( nSizeX < nSizeZ ) {
			m_PressVertices3[ 0 ].Position = m_PressVertices2[ 7 ].Position;
			m_PressVertices3[ 1 ].Position = m_PressVertices1[ 7 ].Position;
			m_PressVertices3[ 2 ].Position = m_PressVertices1[ 3 ].Position;
			m_PressVertices3[ 3 ].Position = m_PressVertices2[ 3 ].Position;
		}
		else {
			m_PressVertices3[ 0 ].Position = m_PressVertices2[ 5 ].Position;
			m_PressVertices3[ 1 ].Position = m_PressVertices2[ 1 ].Position;
			m_PressVertices3[ 2 ].Position = m_PressVertices1[ 1 ].Position;
			m_PressVertices3[ 3 ].Position = m_PressVertices1[ 5 ].Position;
		}
	}
#endif
#endif

	m_fWeight = pSox->GetFieldFromLablePtr( nClassID, "_Weight" )->GetFloat();
	m_nHeight = pSox->GetFieldFromLablePtr( nClassID, "_Height" )->GetInteger();
	m_Material = (ActorMaterialEnum)pSox->GetFieldFromLablePtr( nClassID, "_Material" )->GetInteger();
	m_ActorType = (ActorTypeEnum)pSox->GetFieldFromLablePtr( nClassID, "_Class" )->GetInteger();
	m_fRotateAngleSpeed = pSox->GetFieldFromLablePtr( nClassID, "_RotationAngleSpeed" )->GetFloat();
	m_nPressLevel = pSox->GetFieldFromLablePtr( nClassID, "_PressLevel" )->GetInteger();


	RefreshState();
#if defined(PRE_ADD_MISSION_COUPON)
	SetHP(m_nMaxHP);
	SetSP(m_nMaxSP);
#else
	m_nHP = m_nMaxHP;
	m_nSP = m_nMaxSP;
#endif
}

#if defined (PRE_ADD_BESTFRIEND)
bool CDnActorState::IsApplyState(CDnActor *pActor, CDnWeapon* pWeapon)
{
	if( !pWeapon->IsInfinityDurability() && pWeapon->GetDurability() <= 0 ) 
		return false;

#if defined (_GAMESERVER)
	if (pActor && pActor->IsPlayerActor())
	{
		CDNUserSession* pSession = ((CDnPlayerActor*)pActor)->GetUserSession();
		if (pSession)
		{
			TBestFriendInfo& Info = pSession->GetBestFriend()->GetInfo();

			eItemTypeEnum cType = pWeapon->GetItemType();
			int* nItemTypeParam = pWeapon->GetTypeParam();

			if (cType == ITEMTYPE_PARTS && nItemTypeParam[0] == 10 && nItemTypeParam[1] == 10)
			{
				if (Info.biItemSerial != pWeapon->GetSerialID())
					return false;
			}
		}
	}
#endif
	
	return true;
}
#endif

void CDnActorState::GetStateList( int ListType, CDnState::ValueType Type, std::vector<CDnState *> &VecList )
{
	// Default set
	if( ( ListType & BaseList ) && m_BaseState.IsExistValueType( Type ) )
		VecList.push_back( &m_BaseState );

	// Get weapon list.
	if( ListType & WeaponList ) {
		for( int i=0; i<2; i++ ) {
			DnWeaponHandle hWeapon = m_pActor->CDnActor::GetWeapon(i);
			if( !hWeapon ) continue;
#if defined (PRE_ADD_BESTFRIEND)
			if (!IsApplyState(m_pActor, hWeapon)) continue;
#else
			if( !hWeapon->IsInfinityDurability() && hWeapon->GetDurability() <= 0 ) continue;
#endif
			if( m_pActor->IsPlayerActor() ) {
				std::vector<int> nVecJobHistory;
				if( ((CDnPlayerActor*)m_pActor)->GetJobHistory( nVecJobHistory ) > 0 )
					if( hWeapon->IsPermitPlayer( nVecJobHistory ) == false ) continue;
			}

			if( hWeapon->IsExistValueType( Type ) ) {
				VecList.push_back( hWeapon );
			}
			if( hWeapon->IsActiveEnchant() ) {
				if( hWeapon->GetEnchantState() && hWeapon->GetEnchantState()->IsExistValueType( Type ) ) 
					VecList.push_back( hWeapon->GetEnchantState() );
			}
			if( hWeapon->IsActivePotential() ) {
				if( hWeapon->GetPotentialState() && hWeapon->GetPotentialState()->IsExistValueType( Type ) ) 
					VecList.push_back( hWeapon->GetPotentialState() );
			}
		}
	}

	// Gets the abnormal state.
	if( ListType & StateEffectList ) {
		BLOW_STATE_LIST_ITER iter = m_listBlowState.begin();
		BLOW_STATE_LIST_ITER iter_end = m_listBlowState.end();
		for( ; iter != iter_end; ++iter )
		{
			if( !(*iter)->IsExistValueType( Type ) ) continue;
			VecList.push_back( (*iter) );
		}
	}

	// Get BeforePost state effect state.
	if( ListType & BeforePostStateEffectList )
	{
		BLOW_STATE_LIST_ITER iter = m_listBeforePostBlowState.begin();
		BLOW_STATE_LIST_ITER iter_end = m_listBeforePostBlowState.end();
		for( ; iter != iter_end; ++iter )
		{
			if( !(*iter)->IsExistValueType( Type ) ) continue;
			VecList.push_back( (*iter) );
		}
	}

	// Get Post abnormal status state.
	if( ListType & PostStateEffectList ) {
		BLOW_STATE_LIST_ITER iter = m_listPostBlowState.begin();
		BLOW_STATE_LIST_ITER iter_end = m_listPostBlowState.end();
		for( ; iter != iter_end; ++iter )
		{
			if( !(*iter)->IsExistValueType( Type ) ) continue;
			VecList.push_back( (*iter) );
		}
	}
}

#ifdef PRE_ADD_SKILLBUF_RENEW
void CDnActorState::GetBuffStateList( CDnState::ValueType Type, std::vector<CDnState *> &VecList )
{
	BLOW_STATE_LIST_ITER iter = m_listBuffBlowState.begin();
	BLOW_STATE_LIST_ITER iter_end = m_listBuffBlowState.end();
	for( ; iter != iter_end; ++iter )
	{
		if( !(*iter)->IsExistValueType( Type ) ) continue;
		VecList.push_back( (*iter) );
	}
}
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW


void CDnActorState::SetLevel( int nValue ) 
{ 
	m_nLevel = nValue; 
	RefreshState();

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	OnLevelChange();
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
}

void CDnActorState::RefreshState( RefreshStateExtentEnum Extent, StateTypeEnum Type )
{
	if( Extent == RefreshAll ) CalcBaseState( Type );

	RefreshAdditionalState( Extent, Type );
	CalcState( Extent, Type );

	OnRefreshState();
}

float CDnActorState::GetLevelWeightValue()
{
	int nIndex = CGlobalWeightTable::LevelWeight + m_nLevel - 1;
#ifdef _GAMESERVER
	CDNGameRoom *pRoom = m_pActor->GetGameRoom();
	if( pRoom && pRoom->bIsLevelRegulation() ) nIndex = CGlobalWeightTable::PvPLevelWeight;
#else
	CDnGameTask *pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask ) {
		switch( pGameTask->GetGameTaskType() ) {
			case GameTaskType::PvP:
				if( ((CDnPvPGameTask*)pGameTask)->IsUseRegulation() )
					nIndex = CGlobalWeightTable::PvPLevelWeight;
				break;
			case GameTaskType::DarkLair:
				nIndex = CGlobalWeightTable::PvPLevelWeight;
				break;
		}
	}
	
#endif
	if( nIndex < 0 || nIndex >= CGlobalWeightTable::Amount ) nIndex = CGlobalWeightTable::LevelWeight;
	return CGlobalWeightTable::GetInstance().GetValue( (CGlobalWeightTable::WeightTableIndex)nIndex );
}

bool CDnActorState::IsNeedPvPLevelWeight()
{
	bool bPvPLevelWeight = false;
#ifdef _GAMESERVER
	CDNGameRoom *pRoom = m_pActor->GetGameRoom();
	if( pRoom && pRoom->bIsLevelRegulation() )
		bPvPLevelWeight = true;
#else
	CDnGameTask *pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask )
	{
		switch( pGameTask->GetGameTaskType() ) 
		{
		case GameTaskType::PvP:
			if( ((CDnPvPGameTask*)pGameTask)->IsUseRegulation() )
				bPvPLevelWeight = true;
			break;
		case GameTaskType::DarkLair:
			bPvPLevelWeight = true;
			break;
		}
	}
#endif // #ifdef _GAMESERVER

	return bPvPLevelWeight;
}


float CDnActorState::GetDefenseConstant( void )
{
	// Changed per #33536 to fetch constant values from PlayerCommonLevelTable.
	// Changed from Globaltable to PlayerCommonLevelTable. Applies to both monsters and players.
	float fResult = 0.0f;
	bool bPvPLevelWeight = IsNeedPvPLevelWeight();

	if( bPvPLevelWeight )
	{
		fResult = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DefenseConstant );


	}
	else
	{
#ifdef _GAMESERVER
		TPlayerCommonLevelTableInfo* pPlayerCommonLevelTableInfo = g_pDataManager->GetPlayerCommonLevelTable(GetLevel());
		if( pPlayerCommonLevelTableInfo )
			fResult = pPlayerCommonLevelTableInfo->fDefense;
#else
		DNTableFileFormat*  pSoxPlayerCommonLevel = GetDNTable( CDnTableDB::TPLAYERCOMMONLEVEL );
		if( pSoxPlayerCommonLevel )
			fResult = pSoxPlayerCommonLevel->GetFieldFromLablePtr( GetLevel() , "_Cdefense" )->GetFloat();
#endif
	}

	return fResult;
}

float CDnActorState::GetCriticalConstant( void )
{
	// Changed per #33536 to fetch constant values from PlayerCommonLevelTable.
	// Changed from Globaltable to PlayerCommonLevelTable. Applies to both monsters and players.
	float fResult = 0.0f;
	bool bPvPLevelWeight = IsNeedPvPLevelWeight();

	if( bPvPLevelWeight )
	{
		fResult = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalConstant );


	}
	else
	{
#ifdef _GAMESERVER
		TPlayerCommonLevelTableInfo* pPlayerCommonLevelTableInfo = g_pDataManager->GetPlayerCommonLevelTable(GetLevel());
		if( pPlayerCommonLevelTableInfo )
			fResult = pPlayerCommonLevelTableInfo->fCritical;
#else
		DNTableFileFormat*  pSoxPlayerCommonLevel = GetDNTable( CDnTableDB::TPLAYERCOMMONLEVEL );
		if( pSoxPlayerCommonLevel )
			fResult = pSoxPlayerCommonLevel->GetFieldFromLablePtr( GetLevel() , "_Ccritical" )->GetFloat();
#endif

	}

	return fResult;
}

float CDnActorState::GetFinalDamageConstant( void )
{
	// Changed per #33536 to fetch constant values from PlayerCommonLevelTable.
	// Changed from Globaltable to PlayerCommonLevelTable. Applies to both monsters and players.
	float fResult = 0.0f;
	bool bPvPLevelWeight = IsNeedPvPLevelWeight();

	if( bPvPLevelWeight )
	{
		fResult = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FinalDamageConstant );


	}
	else
	{
#ifdef _GAMESERVER
		TPlayerCommonLevelTableInfo* pPlayerCommonLevelTableInfo = g_pDataManager->GetPlayerCommonLevelTable(GetLevel());
		if( pPlayerCommonLevelTableInfo )
			fResult = pPlayerCommonLevelTableInfo->fFinalDamage;
#else
		DNTableFileFormat*  pSoxPlayerCommonLevel = GetDNTable( CDnTableDB::TPLAYERCOMMONLEVEL );
		if( pSoxPlayerCommonLevel )
			fResult = pSoxPlayerCommonLevel->GetFieldFromLablePtr( GetLevel() , "_Cfinaldamage" )->GetFloat();
#endif
	}

	return fResult;
}


void CDnActorState::AddBlowState( CDnState *pState, AddBlowStateType eAddBlowStateType , bool bIsBuff )
{
	ASSERT(pState&&"CDnActorState::AddBlowState");
	if( !pState ) return;

	switch( eAddBlowStateType )
	{
		case Equip_Buff_Level:
			{
#ifdef PRE_ADD_SKILLBUF_RENEW
				if( bIsBuff )
					m_listBuffBlowState.push_back( pState );
				else
					m_listBlowState.push_back( pState );
#else
				m_listBlowState.push_back( pState );
#endif 
			}
			break;

		case Equip_Skill_Level:
			m_listBeforePostBlowState.push_back( pState );
			break;

		case Skill_Level:
			m_listPostBlowState.push_back( pState );
			break;
	}
}

void CDnActorState::DelBlowState( CDnState *pState )
{
	ASSERT(pState&&"CDnActorState::AddBlowState");
	if( !pState ) return;

	BLOW_STATE_LIST_ITER iter;
	BLOW_STATE_LIST_ITER iter_end;

#ifdef PRE_ADD_SKILLBUF_RENEW
	iter = m_listBuffBlowState.begin();
	iter_end = m_listBuffBlowState.end();
	for( ; iter != iter_end; )
	{
		if( (*iter) == pState )
		{
			iter = m_listBuffBlowState.erase(iter);
			return;
		}
		else
		{
			++iter;
		}
	}
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW

	iter = m_listBlowState.begin();
	iter_end = m_listBlowState.end();
	for( ; iter != iter_end; )
	{
		if( (*iter) == pState )
		{
			iter = m_listBlowState.erase(iter);
			return;
		}
		else
		{
			++iter;
		}
	}

	iter = m_listPostBlowState.begin();
	iter_end = m_listPostBlowState.end();
	for( ; iter != iter_end; )
	{
		if( (*iter) == pState )
		{
			iter = m_listPostBlowState.erase(iter);
			return;
		}
		else
		{
			++iter;
		}
	}

	iter = m_listBeforePostBlowState.begin();
	iter_end = m_listBeforePostBlowState.end();
	for( ; iter != iter_end; )
	{
		if( (*iter) == pState )
		{
			iter = m_listBeforePostBlowState.erase( iter );
			return;
		}
		else
		{
			++iter;
		}
	}
}

CDnActorState::ActorStateEnum CDnActorState::String2ActorStateEnum( const char *szStr )
{
	for( int i=0; i<CDnActorState::ActorStateEnum_Amount; i++ ) {
		if ( CDnActorState::s_szActorStateString[i] != NULL )
			if( _stricmp( szStr, CDnActorState::s_szActorStateString[i] ) == NULL ) return (CDnActorState::ActorStateEnum)CDnActorState::s_nActorStateIndex[i];
	}
	return (CDnActorState::ActorStateEnum)CDnActorState::s_nActorStateIndex[0];
}

/*
#ifdef _SHADOW_TEST
bool CDnActorState::IsDie() {
	CDnActor *pActor = dynamic_cast<CDnActor *>(this);
	if( pActor->IsShadowActor() ) return true;

	if( m_nHP <= 0 ) return true;

	return false;
}

bool CDnActorState::IsDie() const {
	const CDnActor *pActor = dynamic_cast<const CDnActor *>(this);
	if( pActor->IsShadowActor() ) return true;

	if( m_nHP <= 0 ) return true;

	return false;
}
#endif //_SHADOW_TEST
*/

bool CDnActorState::IsDie()
{
#ifdef _SHADOW_TEST
	const CDnActor *pActor = dynamic_cast<const CDnActor *>(this);
	if( pActor->IsShadowActor() ) return true;
#endif //_SHADOW_TEST
	if( m_nHP <= 0 ) return true;
	
	return false;
}


void CDnActorState::CopyStateFromThis( DnActorHandle hActor, bool bDontCopySkillStateEffect/* = false*/ )
{
	if( !hActor )
		return;

	// Important. CDnActorState holds the m_pActor pointer internally, so m_pActor must not be overwritten.
	CDnActor* pMyActor = m_pActor;
	CDnActorState* pActorStateToCopy = static_cast<CDnActorState *>(hActor.GetPointer());

	*(static_cast<CDnActorState *>(this)) = *(pActorStateToCopy);
	m_pActor = pMyActor;

	// Delete the blowstate that was added to the summoner. (#17931)
	m_listBlowState.clear();
#ifdef PRE_ADD_SKILLBUF_RENEW
	m_listBuffBlowState.clear();
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW
	m_listPostBlowState.clear();
	m_listBeforePostBlowState.clear();

	m_BaseState = *(pActorStateToCopy->GetBaseState());

	// #37841 When copying summoner stats to a summon, stats increased by the summoner's state effects can sometimes be excluded.
	if( false == bDontCopySkillStateEffect )
	{
		// Due to #31011, leave StateStep[ 1 ] empty for monsters summoned by a skill's state effect, and 
		// Consolidate current stats into StateStep[ 0 ].
		// Relics also receive attack power enhancement from buff state effects.
#ifdef PRE_ADD_SKILLBUF_RENEW
		// Due to the skill revamp, buff state effects and skill state effects are separated, so stats are updated via RefreshAll.
		// Therefore, since the contents of the StateStep array are overwritten with the monster's values
		// Consolidate into BaseState. BaseState is blocked from updating.
		// BaseState value has been summed into Step[0].
		m_BaseState = *(pActorStateToCopy->GetStateStep( 0 ));
		m_BaseState += *(pActorStateToCopy->GetStateStep( 1 ));

		// Need BaseState values included when updating only PostStateEffect via RefreshSkill.
		m_StateStep[ 0 ] = *(pActorStateToCopy->GetStateStep( 0 ));
#else
		m_StateStep[ 0 ] = *(pActorStateToCopy->GetStateStep( 0 ));
		m_StateStep[ 0 ] += *(pActorStateToCopy->GetStateStep( 1 ));
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW
		m_StateStep[ 1 ].ResetState();
	}
	else
	{
		// Set all to 0
		ResetState();

		// Add while excluding stats granted by state effects.
		// BaseState value has been summed into Step[0]. So no need to add baseState separately
		*(static_cast<CDnState *>(this)) += *const_cast<CDnState*>((pActorStateToCopy->GetStateStep( 0 )));

		// Also synchronize the values of each held step.
#ifdef PRE_ADD_SKILLBUF_RENEW
		m_BaseState = *(pActorStateToCopy->GetStateStep( 0 ));
		
		// Need BaseState values included when updating only PostStateEffect via RefreshSkill.
		m_StateStep[ 0 ] = *(pActorStateToCopy->GetStateStep( 0 ));
#else
		m_StateStep[ 0 ] = *(pActorStateToCopy->GetStateStep( 0 ));
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW
		m_StateStep[ 1 ].ResetState();
	}

#ifdef PRE_ADD_SKILLBUF_RENEW
	m_bCopiedFromSummoner = true;
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW
}


void CDnActorState::SetStateChangeTime()
{ 
	if (m_pActor) 
		m_StateChangeTime = m_pActor->GetLocalTime(); 
}

#if defined(PRE_ADD_MISSION_COUPON) && (_GAMESERVER)
void CDnActorState::SetHP( INT64 nValue )
{
	//Divide HP % into 10 brackets -> 0~9%: bracket 0, 10~19%: bracket 1 ..
	bool bCheck = (m_nHP != 0 && m_nMaxHP != 0);
	int nPrevPercent = GetHPPercent()/10 < 10 ? GetHPPercent()/10 : 9;	//100% and above maps to tier 9
	
	m_nHP = nValue; 
	m_nHP = (m_nHP < 0) ? 0 : m_nHP;
	
	if( bCheck && nPrevPercent != (GetHPPercent()/10 < 10 ? GetHPPercent()/10 : 9) )
	{
		if ( GetMaxHP() != 0 && m_pActor && m_pActor->IsPlayerActor())
		{
			CDNUserSession* pSession = ((CDnPlayerActor*)m_pActor)->GetUserSession();
			if (pSession)
				pSession->GetEventSystem()->OnEvent( EventSystem::OnHPChanged );
		}
	}
}

void CDnActorState::SetSP( int nValue )
{
	//Divide mp % into 10 brackets. -> 0~9% bracket 0, 10~19 : bracket 1 ..
	bool bCheck = (m_nSP != 0 && m_nMaxSP != 0);
	int nPrevPercent = GetSPPercent()/10 < 10 ? GetSPPercent()/10 : 9;	//100% and above maps to tier 9
	m_nSP = nValue;
	
	if( bCheck && nPrevPercent != (GetSPPercent()/10 < 10 ? GetSPPercent()/10 : 9) )
	{
		if ( GetMaxSP() != 0 && m_pActor && m_pActor->IsPlayerActor())
		{
			CDNUserSession* pSession = ((CDnPlayerActor*)m_pActor)->GetUserSession();
			if (pSession)
				pSession->GetEventSystem()->OnEvent( EventSystem::OnMPChanged );
		}
	}
}
#endif