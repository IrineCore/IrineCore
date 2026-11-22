#include "stdafx.h"
#include "DnPlayerState.h"
#include "DnTableDB.h"
#include "MAPartsBody.h"
#include "DnActor.h"
#include "DnPlayerActor.h"
#include "DnGameTask.h"
#include "DnPvPGameTask.h"
#include "TaskManager.h"
#include "DnDLGameTask.h"
#include "DnItem.h"
#if defined(_GAMESERVER)
#include "DNGameDataManager.h"
#include "DNGameRoom.h"
#include "DNPvPGameRoom.h"
#include "DNUserSession.h"
#include "DNBestFriend.h"
#else //_GAMESERVER
#include "DnPetTask.h"
#include "DnItemTask.h"
#include "SyncTimer.h"
#endif // _GAMESERVER

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPlayerState::CDnPlayerState()
{
	m_pSetItemState = NULL;
	m_pAppellationState = NULL;
	m_pPetAbilityState = NULL;
#if defined(PRE_ADD_SKILLBUF_RENEW)
	m_BuffState.ResetState();
#endif

	m_nExperience = 0;
	m_nNextLevelExperience = 0;
	m_nCurrentLevelExperience = 0;

	m_fDeadDurabilityRatio = 0.f;
	m_nAppellationIndex = -1;
	m_nCoverAppellationIndex = -1;

	m_cAccountLevel = 0;
	m_cPvPLevel = 0;

#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
	memset(&m_sDamageLimitInfo,0,sizeof(m_sDamageLimitInfo));
#endif

}

CDnPlayerState::~CDnPlayerState()
{
	SAFE_DELETE( m_pSetItemState );
	SAFE_DELETE( m_pAppellationState );
	SAFE_DELETE( m_pPetAbilityState );
	for( int i=0; i<(int)m_vecpPetAccessaryState.size(); i++ )
	{
		if( m_vecpPetAccessaryState[i] )
			SAFE_DELETE( m_vecpPetAccessaryState[i] );
	}
	m_vecpPetAccessaryState.clear();
}

void CDnPlayerState::CalcBaseState( StateTypeEnum Type )
{
	if( m_nLevel == 0 ) return;

	m_BaseState.SetSuperAmmor( CPlayerLevelTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CPlayerLevelTable::SuperAmmor ) );
	m_fDeadDurabilityRatio = CPlayerLevelTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CPlayerLevelTable::DeadDurabilityRatio ) / 100.f;

	m_BaseState.CalcValueType();
}

void CDnPlayerState::CalcState( RefreshStateExtentEnum Extent, StateTypeEnum Type )
{
	if( m_nLevel == 0 ) return;

	std::vector<CDnState *> pVecAbsoluteList[2];
	std::vector<CDnState *> pVecRatioList[2];

	GetStateList( BaseList | WeaponList | DefenseList | AccessoryList | StateEffectList | SetItemList | AppellationList | GlyphList | TalismanList | PetAbility, ValueTypeAbsolute, pVecAbsoluteList[0] );
	GetStateList( BaseList | WeaponList | DefenseList | AccessoryList | StateEffectList | SetItemList | AppellationList | GlyphList | TalismanList | PetAbility, ValueTypeRatio, pVecRatioList[0] );
	GetStateList( PostStateEffectList, ValueTypeRatio, pVecRatioList[1] );
	GetStateList( PostStateEffectList, ValueTypeAbsolute, pVecAbsoluteList[1] );

	if( Type & ST_Strength ) CalcStrength( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_Agility ) CalcAgility( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_Intelligence ) CalcIntelligence( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_Stamina ) CalcStamina( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_AttackP ) CalcAttackP( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_AttackM ) CalcAttackM( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_DefenseP ) CalcDefenseP( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_DefenseM ) CalcDefenseM( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_HP ) CalcHP( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_SP ) CalcSP( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_MoveSpeed ) CalcMoveSpeed( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_DownDelay ) CalcDownDelay( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_Stiff ) CalcStiff( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_StiffResistance ) CalcStiffResistance( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_Critical ) CalcCritical( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_CriticalResistance ) CalcCriticalResistance( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_Stun ) CalcStun( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_StunResistance ) CalcStunResistance( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_RecoverySP ) CalcRecoverySP( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_SuperAmmor ) CalcSuperAmmor( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_FinalDamage ) CalcFinalDamage( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_Spirit ) CalcSpirit( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_SafeZoneMoveSpeed ) CalcSafeZoneMoveSpeed( Extent, pVecAbsoluteList, pVecRatioList );
	if( Type & ST_AddExp ) CalcAddExp( Extent, pVecAbsoluteList, pVecRatioList );

	for( int i=0; i<ElementEnum_Amount; i++ ) {
		if( Type & ST_ElementAttack ) CalcElementAttack( (ElementEnum)i, Extent, pVecAbsoluteList, pVecRatioList );
		if( Type & ST_ElementDefense ) CalcElementDefense( (ElementEnum)i, Extent, pVecAbsoluteList, pVecRatioList );
	}

	if( Extent & RefreshBase ) m_BaseState.CalcValueType();
	if( Extent & RefreshEquip ) m_StateStep[0].CalcValueType();
	if( Extent & RefreshSkill ) m_StateStep[1].CalcValueType();

	CalcValueType();
#if defined(PRE_ADD_MISSION_COUPON)
	if( m_nHP > m_nMaxHP ) SetHP(m_nMaxHP);
	if( m_nSP > m_nMaxSP ) SetSP(m_nMaxSP);
#else
	if( m_nHP > m_nMaxHP ) m_nHP = m_nMaxHP;
	if( m_nSP > m_nMaxSP ) m_nSP = m_nMaxSP;
#endif

	OnCalcPlayerState();
}

#if defined (PRE_ADD_BESTFRIEND)
bool CDnPlayerState::IsApplyPartsState(CDnActor *pActor, CDnParts* pParts, bool Durability)
{
	if( Durability && !pParts->IsInfinityDurability() && pParts->GetDurability() <= 0 ) 
		return false;

	int nParam1=0, nParam2 = 0;
#if defined (_GAMESERVER)
	int* nTypePram = pParts->GetTypeParam();
	nParam1 = nTypePram[0];
	nParam2 = nTypePram[1];
#elif defined (_CLIENT)
	nParam1 = pParts->GetTypeParam(0);
	nParam2 = pParts->GetTypeParam(1);
#endif

	if( !IsBestFriendItem(pActor, pParts->GetItemType(), nParam1, nParam2, pParts->GetSerialID()))
		return false;

	return true;
}

bool CDnPlayerState::IsApplyWeaponState(CDnActor *pActor, CDnWeapon* pWeapon, bool Durability)
{
	if( Durability && !pWeapon->IsInfinityDurability() && pWeapon->GetDurability() <= 0 ) 
		return false;

	int nParam1=0, nParam2 = 0;
#if defined (_GAMESERVER)
	int* nTypePram = pWeapon->GetTypeParam();
	nParam1 = nTypePram[0];
	nParam2 = nTypePram[1];
#elif defined (_CLIENT)
	nParam1 = pWeapon->GetTypeParam(0);
	nParam2 = pWeapon->GetTypeParam(1);
#endif

	if( !IsBestFriendItem(pActor, pWeapon->GetItemType(), nParam1, nParam2, pWeapon->GetSerialID()))
		return false;

	return true;
}

bool CDnPlayerState::IsBestFriendItem(CDnActor* pActor, eItemTypeEnum cType, int nParam1, int nParam2, INT64 nSerial)
{
#if defined (_GAMESERVER)
	if (pActor && pActor->IsPlayerActor())
	{
		CDNUserSession* pUserSession = ((CDnPlayerActor*)pActor)->GetUserSession();
		if (pUserSession)
		{
			TBestFriendInfo& Info = pUserSession->GetBestFriend()->GetInfo();

			if (cType == ITEMTYPE_PARTS && nParam1 == 10 && nParam2 == 10)
			{
				if (Info.biItemSerial != nSerial)
					return false;
			}
		}
	}
#elif defined (_CLIENT) 

	if( pActor &&
		cType == ITEMTYPE_PARTS && nParam1 == 10 && nParam2 == 10 &&
		pActor->GetBFserial() != nSerial )
	{		
		return false;	
	}


#endif

	return true;
}
#endif

void CDnPlayerState::GetStateList( int ListType, CDnState::ValueType Type, std::vector<CDnState *> &VecList )
{
	VecList.clear();

	CDnActorState::GetStateList( ListType, Type, VecList );

	// Get armor list.
	if( ListType & DefenseList ) {
		CDnPlayerActor *pParts = static_cast<CDnPlayerActor *>(m_pActor);
		if( pParts ) {
			// Normal item
			for( DWORD i=0; i<CDnParts::PartsTypeEnum_Amount; i++ ) {
				DnPartsHandle hParts = pParts->GetParts( (CDnParts::PartsTypeEnum)i );
				if( !hParts ) continue;
				if( !hParts->IsInfinityDurability() && hParts->GetDurability() <= 0 ) continue;

				std::vector<int> nVecJobHistory;
				if( ((CDnPlayerActor*)m_pActor)->GetJobHistory( nVecJobHistory ) > 0 )
					if( hParts->IsPermitPlayer( nVecJobHistory ) == false ) continue;

				if( hParts->IsExistValueType( Type ) ) {
					VecList.push_back( hParts );
				}

				if( hParts->IsActiveEnchant() ) {
					if( hParts->GetEnchantState() && hParts->GetEnchantState()->IsExistValueType( Type ) )
						VecList.push_back( hParts->GetEnchantState() );
				}
				if( hParts->IsActivePotential() ) {
					if( hParts->GetPotentialState() && hParts->GetPotentialState()->IsExistValueType( Type ) )
						VecList.push_back( hParts->GetPotentialState() );
				}
			}
			// Cash item
			for( DWORD i=0; i<CDnParts::CashPartsTypeEnum_Amount; i++ ) {
				DnPartsHandle hParts = pParts->GetCashParts( (CDnParts::PartsTypeEnum)i );
				if( !hParts ) continue;
#if defined (PRE_ADD_BESTFRIEND)
				if (!IsApplyPartsState(pParts, hParts)) continue;
#endif
				// Cash items have no durability, so skip durability check.
				if( hParts->IsExistValueType( Type ) ) {
					VecList.push_back( hParts );
				}

				if( hParts->IsActiveEnchant() ) {
					if( hParts->GetEnchantState() && hParts->GetEnchantState()->IsExistValueType( Type ) )
						VecList.push_back( hParts->GetEnchantState() );

				}
				if( hParts->IsActivePotential() ) {
					if( hParts->GetPotentialState() && hParts->GetPotentialState()->IsExistValueType( Type ) )
						VecList.push_back( hParts->GetPotentialState() );
				}
			}
			// Cash weapon
			for( int i=0; i<2; i++ ) {
				DnWeaponHandle hWeapon = pParts->GetCashWeapon(i);
				if( !hWeapon ) continue;
#if defined (PRE_ADD_BESTFRIEND)
				if (!IsApplyWeaponState(pParts, hWeapon)) continue;
#endif
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
	}
	// SetItem State
	if( ( ListType & SetItemList ) && m_pSetItemState ) {
		VecList.push_back( m_pSetItemState );
	}
	// Appellation State
	if( ( ListType & AppellationList ) && m_pAppellationState ) {
		VecList.push_back( m_pAppellationState );
	}
	// Glyph State
	if( ListType & GlyphList ) { 
		CDnPlayerActor *pGlyph = static_cast<CDnPlayerActor *>(m_pActor);
		if( pGlyph ) {
			for( DWORD i=0; i<CDnGlyph::GlyphSlotEnum_Amount; i++ ) {
				DnGlyphHandle hGlyph = pGlyph->GetGlyph( (CDnGlyph::GlyphSlotEnum)i );
				if( !hGlyph ) continue;

#if !defined(_GAMESERVER)
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
				if( (i >= CDnGlyph::Cash1 && i <= CDnGlyph::Cash3) && false == GetItemTask().IsCashGlyphOpen( i - CDnGlyph::Cash1 ) )
#else
				if( i >= CDnGlyph::Cash1 && false == GetItemTask().IsCashGlyphOpen( i - CDnGlyph::Cash1 ) )
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
					continue;
#endif	// #if !defined(_GAMESERVER)

				if( hGlyph->IsExistValueType( Type ) ) 
				{
					VecList.push_back( hGlyph );
				}
			}
		}
	}

	// Talisman State
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	if( ListType & TalismanList ) {
		CDnPlayerActor *pPlayerActor = static_cast<CDnPlayerActor *>(m_pActor);
		if( pPlayerActor ) {
#if defined(_GAMESERVER)
			CDNUserSession *pSession = pPlayerActor->GetUserSession();
			__time64_t tCurTime = 0;
			_time64( &tCurTime );
#endif
			for( int i=0; i<TALISMAN_MAX; i++ ) {
#if !defined(_GAMESERVER)
				DnTalismanHandle hTalisman = pPlayerActor->GetTalisman( i );
				if( !hTalisman ) continue;

				if( !hTalisman->IsEternityItem() ) {
					const time_t *pExpireDate = hTalisman->GetExpireDate();
					if( pExpireDate && *pExpireDate <= CSyncTimer::GetInstance().GetCurTime() )
						continue;
				}
				if( i <= TALISMAN_CASH_MAX && CDnItemTask::IsActive() && m_pActor == CDnActor::s_hLocalActor.GetPointer() && !GetItemTask().IsCashTalismanOpen() )
					continue;
#else
				if( pSession && pSession->GetItem() ) {
					if( !pSession->GetItem()->IsTalismanSlotOpen( i ) )
						continue;
					const TItem *pTalismanData = pSession->GetItem()->GetTalisman( i );
					if( pTalismanData && !pTalismanData->bEternity && pTalismanData->tExpireDate <= tCurTime )
						continue;
				}
#endif	// #if !defined(_GAMESERVER)
				CDnState *pState = pPlayerActor->GetTalismanScaledState( i );
				if( pState && pState->IsExistValueType( Type ) ) {
					VecList.push_back( pState );
				}
			}
		}
	}
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

	if( ( ListType & PetAbility ) && m_pPetAbilityState )
	{
		VecList.push_back( m_pPetAbilityState );
	}
	if( ( ListType & PetAbility ) && m_vecpPetAccessaryState.size() > 0 )
	{
		for( int i=0; i<(int)m_vecpPetAccessaryState.size(); i++ )
			VecList.push_back( m_vecpPetAccessaryState[i] );
	}
}

void CDnPlayerState::CalcSetItemState()
{
	if( m_nLevel == 0 ) return;

	std::map<int, int> nMapSetList;
	std::map<int, int>::iterator it;
	CDnPlayerActor *pPlayerActor = static_cast<CDnPlayerActor *>(m_pActor);
	if( !pPlayerActor ) return;

	int nSetItemID = 0;

	// Normal item
	for( int i=CDnParts::Helmet; i<=CDnParts::Ring2; i++ ) {
		DnPartsHandle hParts = pPlayerActor->GetParts( (CDnParts::PartsTypeEnum)i );
		if( !hParts ) continue;

		nSetItemID = hParts->GetSetItemID();
		if( nSetItemID < 1 ) continue;

#if defined( PRE_ADD_OVERLAP_SETEFFECT )
		if( hParts->GetOverLapSet() )
		{
			CalcOverLapSetItemState(nSetItemID, nMapSetList);
			continue;
		}
#endif

		it = nMapSetList.find( nSetItemID );
		if( it != nMapSetList.end() ) {
			it->second++;
		}
		else {
			nMapSetList.insert( make_pair( nSetItemID, 1 ) );
		}
	}
	//Cash item
	for( int i=CDnParts::CashHelmet; i<=CDnParts::CashFairy; i++ ) {
		DnPartsHandle hParts = pPlayerActor->GetCashParts( (CDnParts::PartsTypeEnum)i );
		if( !hParts ) continue;
		nSetItemID = hParts->GetSetItemID();
		if( nSetItemID < 1 ) continue;
#if defined (PRE_ADD_BESTFRIEND)
		if (!IsApplyPartsState(pPlayerActor, hParts, false)) continue;
#endif
#if defined( PRE_ADD_OVERLAP_SETEFFECT )
		if( hParts->GetOverLapSet() )
		{
			CalcOverLapSetItemState(nSetItemID, nMapSetList);
			continue;
		}
#endif
		it = nMapSetList.find( nSetItemID );
		if( it != nMapSetList.end() ) {
			it->second++;
		}
		else {
			nMapSetList.insert( make_pair( nSetItemID, 1 ) );
		}
	}

	// Normal item
	for( int i=0; i<2; i++ ) {
		DnWeaponHandle hWeapon = m_pActor->GetWeapon(i);
		if( !hWeapon ) continue;
		nSetItemID = hWeapon->GetSetItemID();
		if( nSetItemID < 1 ) continue;

#if defined( PRE_ADD_OVERLAP_SETEFFECT )
		if( hWeapon->GetOverLapSet() )
		{
			CalcOverLapSetItemState(nSetItemID, nMapSetList);
			continue;
		}
#endif
		it = nMapSetList.find( nSetItemID );
		if( it != nMapSetList.end() ) {
			it->second++;
		}
		else {
			nMapSetList.insert( make_pair( nSetItemID, 1 ) );
		}
	}
	// Cash item
	for( int i=0; i<2; i++ ) {
		DnWeaponHandle hWeapon = pPlayerActor->GetCashWeapon(i);
		if( !hWeapon ) continue;
		nSetItemID = hWeapon->GetSetItemID();
		if( nSetItemID < 1 ) continue;
#if defined (PRE_ADD_BESTFRIEND)
		if (!IsApplyWeaponState(pPlayerActor, hWeapon, false)) continue;
#endif
#if defined( PRE_ADD_OVERLAP_SETEFFECT )
		if( hWeapon->GetOverLapSet() )
		{
			CalcOverLapSetItemState(nSetItemID, nMapSetList);
			continue;
		}
#endif
		it = nMapSetList.find( nSetItemID );
		if( it != nMapSetList.end() ) {
			it->second++;
		}
		else {
			nMapSetList.insert( make_pair( nSetItemID, 1 ) );
		}
	}

	SAFE_DELETE( m_pSetItemState );

	if( nMapSetList.empty() ) 
	{
#ifdef PRE_ADD_COSTUME_SKILL
		pPlayerActor->RefreshCostumeSkill( 0, 0 );
#endif
		return;
	}

	m_pSetItemState = new CDnState;
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSETITEM );
	char szLabel[32];
	int nTemp;
	for( it = nMapSetList.begin(); it != nMapSetList.end(); it++ ) {
		if( it->second < 2 ) continue;

		nSetItemID = it->first;
		std::vector<int> nVecStateList;
		for( int i=0; i<12; i++ ) {
			sprintf_s( szLabel, "_NeedSetNum%d", i + 1 );
			nTemp = pSox->GetFieldFromLablePtr( nSetItemID, szLabel )->GetInteger();
			if( nTemp < 2 ) continue;
			if( nTemp <= it->second ) nVecStateList.push_back(i);
		}
		for( DWORD i=0; i<nVecStateList.size(); i++ ) {
			sprintf_s( szLabel, "_State%d", nVecStateList[i] + 1 );
			int nStateType = pSox->GetFieldFromLablePtr( nSetItemID, szLabel )->GetInteger();
			sprintf_s( szLabel, "_State%dValue", nVecStateList[i] + 1 );
			char *szValue = pSox->GetFieldFromLablePtr( nSetItemID, szLabel )->GetString();
#ifdef _GAMESERVER
			CDnItem::CalcStateValue( m_pActor->GetRoom(), m_pSetItemState, nStateType, szValue, szValue, false, false );
#else
			CDnItem::CalcStateValue( m_pSetItemState, nStateType, szValue, szValue, false, false );
#endif 
		}
	}

#ifdef PRE_ADD_COSTUME_SKILL
	for( it = nMapSetList.begin(); it != nMapSetList.end(); it++ ) 
	{
		nSetItemID = it->first;

		int nCustumeSkillNeedCount = pSox->GetFieldFromLablePtr( nSetItemID, "_NeedCSSetNum" )->GetInteger();
		if( nCustumeSkillNeedCount > 0 )
		{
			if( it->second >= nCustumeSkillNeedCount )
			{
				int nSkillIndex = pSox->GetFieldFromLablePtr( nSetItemID, "_SkillID" )->GetInteger();
				int nSkillLevel = pSox->GetFieldFromLablePtr( nSetItemID, "_SkillLevel" )->GetInteger();
				pPlayerActor->RefreshCostumeSkill( nSkillIndex, nSkillLevel );
			}
			else
			{
				pPlayerActor->RefreshCostumeSkill( 0, 0 );
			}
		}
	}
#endif

}

#if defined( PRE_ADD_OVERLAP_SETEFFECT )
void CDnPlayerState::CalcOverLapSetItemState(int nOverLapSetItemID, std::map<int, int> &nMapSetList)
{	
	if( nOverLapSetItemID > 0 )
	{
		int nOverLapID = 0;
		std::map<int, int>::iterator it;
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TOVERLAPSETEFFECT );
		char szLabel[32];
		for(int i=0;i<10;i++)
		{				
			sprintf_s( szLabel, "_SetItemID%d", i + 1 );
			nOverLapID = pSox->GetFieldFromLablePtr( nOverLapSetItemID, szLabel )->GetInteger();
			if( nOverLapID > 0 )
			{
				it = nMapSetList.find( nOverLapID );
				if( it != nMapSetList.end() ) {
					it->second++;
				}
				else {
					nMapSetList.insert( make_pair( nOverLapID, 1 ) );
				}
			}
			else
				break;
		}		
	}
}
#endif

void CDnPlayerState::CalcAppellationState()
{
	SAFE_DELETE( m_pAppellationState );
	if( m_nAppellationIndex == -1 ) return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TAPPELLATION );
	int nItemID = pSox->GetItemID( m_nAppellationIndex );
	if( nItemID == -1 ) return;

	bool bExistState = false;
	char szLabel[32];
	int nState[10] = { -1, };
	char *szStateValue[10];

	for( int i=0; i<10; i++ ) {
		sprintf_s( szLabel, "_State%d", i + 1 );
		nState[i] = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetInteger();
		if( nState[i] >= 0 ) bExistState = true;
		sprintf_s( szLabel, "_StateValue%d", i + 1 );
		szStateValue[i] = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetString();
	}

	if( bExistState ) {
		m_pAppellationState = new CDnState;

		for( int i=0; i<10; i++ ) {
#ifdef _GAMESERVER
			CDnItem::CalcStateValue( m_pActor->GetRoom(), m_pAppellationState, nState[i], szStateValue[i], szStateValue[i], false );
#else
			CDnItem::CalcStateValue( m_pAppellationState, nState[i], szStateValue[i], szStateValue[i], false );
#endif 
		}
	}
}

void CDnPlayerState::SetAppellationIndex( int nCoverIndex, int nIndex )
{
	m_nCoverAppellationIndex = nCoverIndex;
	m_nAppellationIndex = nIndex;
}

void CDnPlayerState::ChangeAppellationState( int nCoverArrayIndex, int nArrayIndex )
{
	SetAppellationIndex( nCoverArrayIndex, nArrayIndex );
	RefreshState( RefreshEquip );
}

bool CDnPlayerState::IsSetAppellationIndex()
{
	if( m_nAppellationIndex == -1 && m_nCoverAppellationIndex == -1 )
		return false;

	return true;
}

void CDnPlayerState::CalcPetAbilityState()
{
	if( m_nLevel == 0 ) return;

	SAFE_DELETE( m_pPetAbilityState );

	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(m_pActor);
	if( !pPlayer ) return;

#ifdef _GAMESERVER
	if( !pPlayer->GetUserSession() || !pPlayer->GetUserSession()->GetItem() || !pPlayer->IsCanPetMode() ) return;
	if( pPlayer->GetUserSession()->GetItem()->GetPetBodyItemID() == 0 && pPlayer->GetUserSession()->GetItem()->GetPetExp() == 0 ) return;	// Return if no pet exists.
	TPetLevelDetail *pPetData = g_pDataManager->GetPetLevelDetail( pPlayer->GetUserSession()->GetItem()->GetPetBodyItemID(), pPlayer->GetUserSession()->GetItem()->GetPetExp() );
	if( !pPetData ) return;

	bool bExistState = false;
	for( int i=0; i<PetStateMax; i++ ) 
	{
		if (pPetData->nState[i] >= 0) 
		{
			bExistState = true;
			break;
		}
	}
#else // _GAMESERVER
	if( !pPlayer->GetSummonPet() || !pPlayer->IsSummonPet() ) return;
	int nPetLevelTableIndex = GetPetTask().GetPetLevelTableIndex( pPlayer->GetUniqueID() );
	if( nPetLevelTableIndex == -1 ) return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPETLEVEL );
	if( pSox == NULL ) return;

	bool bExistState = false;
	char szLabel[32];
	int nState[10] = { -1, };
	char* szStateMinValue[10];
	char* szStateMaxValue[10];

	for( int i=0; i<10; i++ ) 
	{
		sprintf_s( szLabel, "_State%d", i + 1 );
		nState[i] = pSox->GetFieldFromLablePtr( nPetLevelTableIndex, szLabel )->GetInteger();
		if( nState[i] >= 0 ) bExistState = true;
		sprintf_s( szLabel, "_State%d_Min", i + 1 );
		szStateMinValue[i] = pSox->GetFieldFromLablePtr( nPetLevelTableIndex, szLabel )->GetString();
		sprintf_s( szLabel, "_State%d_Max", i + 1 );
		szStateMaxValue[i] = pSox->GetFieldFromLablePtr( nPetLevelTableIndex, szLabel )->GetString();
	}
#endif // _GAMESERVER
	if( bExistState ) 
	{
		m_pPetAbilityState = new CDnState;

		for( int i=0; i<10; i++ )
		{
#ifdef _GAMESERVER
			if( pPlayer->GetUserSession()->GetItem()->GetPetEquip()->nType & Pet::Type::ePETTYPE_SATIETY )	// Apply different stats based on pet satiety
			{
				CDnItem::CalcStateValue( m_pActor->GetRoom(), m_pPetAbilityState, pPetData->nState[i], pPetData->szStateMin[i], pPetData->szStateMax[i], false , 
					true, pPlayer->GetUserSession()->GetItem()->GetSatietyApplyStateRatio() );
			}
			else
			{
				CDnItem::CalcStateValue( m_pActor->GetRoom(), m_pPetAbilityState, pPetData->nState[i], pPetData->szStateMin[i], pPetData->szStateMax[i], false );
			}
#else // _GAMESERVER
			if( pPlayer->GetPetInfo().nType & Pet::Type::ePETTYPE_SATIETY )	// Apply different stats based on pet satiety
			{
				CDnItem::CalcStateValue( m_pPetAbilityState, nState[i], szStateMinValue[i], szStateMaxValue[i], false, true, GetPetTask().GetSatietyApplyStateRatio() );
			}
			else
			{
				CDnItem::CalcStateValue( m_pPetAbilityState, nState[i], szStateMinValue[i], szStateMaxValue[i], false );
			}
#endif // _GAMESERVER
		}
	}
}

void CDnPlayerState::CalcPetAccessaryAbilityState()
{
	if( m_nLevel == 0 ) return;

	for( int i=0; i<(int)m_vecpPetAccessaryState.size(); i++ )
	{
		if( m_vecpPetAccessaryState[i] )
			SAFE_DELETE( m_vecpPetAccessaryState[i] );
	}
	m_vecpPetAccessaryState.clear();

	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(m_pActor);
	if( !pPlayer ) return;

	vector<int> vecPartsItemID;
#ifdef _GAMESERVER
	if( !pPlayer->GetUserSession() || !pPlayer->GetUserSession()->GetItem() || !pPlayer->IsCanPetMode() ) return;

	const TVehicle* pPetEquip = pPlayer->GetUserSession()->GetItem()->GetPetEquip();
	if( !pPetEquip ) return;

	bool bExistState = false;
	for( int i=Pet::Slot::Accessory1; i<Pet::Slot::Max; ++i )
	{
		if( !pPetEquip->Vehicle[i].nItemID ) continue;
		vecPartsItemID.push_back( pPetEquip->Vehicle[i].nItemID );
	}
#else // _GAMESERVER
	if( !pPlayer->GetSummonPet() || !pPlayer->IsSummonPet() ) return;

	for( int i=Pet::Slot::Accessory1; i<Pet::Slot::Max; ++i )
	{
		if( !pPlayer->GetPetInfo().Vehicle[i].nItemID ) continue;
		vecPartsItemID.push_back( pPlayer->GetPetInfo().Vehicle[i].nItemID );
	}
#endif // _GAMESERVER

	if( vecPartsItemID.size() == 0 ) return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
	if( pSox == NULL ) return;

	char szLabel[32];

	for( int i=0; i<static_cast<int>( vecPartsItemID.size() ); i++ )
	{
		bool bExistState = false;
		int nState[10] = { -1, };
		char* szStateMinValue[10];
		char* szStateMaxValue[10];

		for( int j=0; j<10; j++ )
		{
			sprintf_s( szLabel, "_State%d", j + 1 );
			nState[j] = pSox->GetFieldFromLablePtr( vecPartsItemID[i], szLabel )->GetInteger();
			if( nState[j] >= 0 ) bExistState = true;
			sprintf_s( szLabel, "_State%d_Min", j + 1 );
			szStateMinValue[j] = pSox->GetFieldFromLablePtr( vecPartsItemID[i], szLabel )->GetString();
			sprintf_s( szLabel, "_State%d_Max", j + 1 );
			szStateMaxValue[j] = pSox->GetFieldFromLablePtr( vecPartsItemID[i], szLabel )->GetString();
		}

		if( bExistState )
		{
			CDnState* pPetEquipState = new CDnState;
			for( int k=0; k<10; k++ )
			{
#ifdef _GAMESERVER
				CDnItem::CalcStateValue( m_pActor->GetRoom(), pPetEquipState, nState[k], szStateMinValue[k], szStateMaxValue[k], false );
#else // _GAMESERVER
				CDnItem::CalcStateValue( pPetEquipState, nState[k], szStateMinValue[k], szStateMaxValue[k], false );
#endif // _GAMESERVER
			}
			m_vecpPetAccessaryState.push_back( pPetEquipState );
		}
	}
	vecPartsItemID.clear();

	// Pet set effect
	pSox = GetDNTable( CDnTableDB::TVEHICLEPARTS );
	if( !pSox ) return;
#ifdef _GAMESERVER
	int nSetItemID1 = pSox->GetFieldFromLablePtr( pPetEquip->Vehicle[Pet::Slot::Accessory1].nItemID, "_SetItemID" )->GetInteger();
	int nSetItemID2 = pSox->GetFieldFromLablePtr( pPetEquip->Vehicle[Pet::Slot::Accessory2].nItemID, "_SetItemID" )->GetInteger();
#else // _GAMESERVER
	int nSetItemID1 = pSox->GetFieldFromLablePtr( pPlayer->GetPetInfo().Vehicle[Pet::Slot::Accessory1].nItemID, "_SetItemID" )->GetInteger();
	int nSetItemID2 = pSox->GetFieldFromLablePtr( pPlayer->GetPetInfo().Vehicle[Pet::Slot::Accessory2].nItemID, "_SetItemID" )->GetInteger();
#endif // _GAMESERVER
	if( nSetItemID1 > 0 && nSetItemID2 > 0 && nSetItemID1 == nSetItemID2 )
	{
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSETITEM );
		char szLabel[32];
		int nTemp;
		std::vector<int> nVecStateList;
		for( int i=0; i<12; i++ ) 
		{
			sprintf_s( szLabel, "_NeedSetNum%d", i+1 );
			nTemp = pSox->GetFieldFromLablePtr( nSetItemID1, szLabel )->GetInteger();
			if( nTemp < 2 ) continue;
			nVecStateList.push_back(i);
		}

		CDnState* pPetEquipState = new CDnState;
		for( DWORD i=0; i<nVecStateList.size(); i++ ) 
		{
			sprintf_s( szLabel, "_State%d", nVecStateList[i] + 1 );
			int nStateType = pSox->GetFieldFromLablePtr( nSetItemID1, szLabel )->GetInteger();
			sprintf_s( szLabel, "_State%dValue", nVecStateList[i] + 1 );
			char *szValue = pSox->GetFieldFromLablePtr( nSetItemID1, szLabel )->GetString();
#ifdef _GAMESERVER
			CDnItem::CalcStateValue( m_pActor->GetRoom(), pPetEquipState, nStateType, szValue, szValue, false, false );
#else // _GAMESERVER
			CDnItem::CalcStateValue( pPetEquipState, nStateType, szValue, szValue, false, false );
#endif // _GAMESERVER
		}
		m_vecpPetAccessaryState.push_back( pPetEquipState );
	}
}

void CDnPlayerState::RefreshAdditionalState( RefreshStateExtentEnum &Extent, StateTypeEnum &Type )
{
	if( Extent != RefreshAll ) {
		// Check here because in RefreshSkill, if base stats change, Equip must also be calculated.
		if( ( Extent & RefreshSkill ) || ( Extent & RefreshEquip ) ) {
			if( (Type&ST_Strength) || (Type&ST_Agility) || (Type&ST_Intelligence) || (Type&ST_Stamina) ) {
				Extent = RefreshAll;
			}
		}
	}
	// Apply linked State internally.
	if( Type & ST_Strength ) Type = (StateTypeEnum)( Type | ST_AttackP | ST_Stiff | ST_StiffResistance | ST_Stun );
	if( Type & ST_Agility ) Type = (StateTypeEnum)( Type | ST_AttackP | ST_Critical | ST_CriticalResistance );
	if( Type & ST_Intelligence ) Type = (StateTypeEnum)( Type | ST_AttackM | ST_DefenseM | ST_SP | ST_RecoverySP );
	if( Type & ST_Stamina ) Type = (StateTypeEnum)( Type | ST_DefenseP | ST_HP | ST_StunResistance );


	// Set item configuration
	if( Type & RefreshEquip ) {
		CalcSetItemState();
		CalcAppellationState();
		CalcPetAbilityState();
		CalcPetAccessaryAbilityState();
	}
}

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT

float CDnPlayerState::GetUseStageStateLimit()
{
#if defined(_GAMESERVER)
	CDNGameRoom *pRoom = ((CDnPlayerActor*)m_pActor)->GetGameRoom();
	if( pRoom )
	{
		CDnGameTask *pTask = pRoom->GetGameTask();
		if( pTask && pTask->GetStageDamageLimit() )
		{
			return pTask->GetStageDamageLimit()->fStateLimit;
		}
	}
#else
	CDnGameTask *pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetStageDamageLimit() ) 
	{
		return pGameTask->GetStageDamageLimit()->fStateLimit;
	}
#endif

	return 0;
}

#endif

char CDnPlayerState::GetUseRegulation()
{

#if defined(_GAMESERVER)

	CDNGameRoom *pRoom = ((CDnPlayerActor*)m_pActor)->GetGameRoom();
	if( pRoom )
	{
		CDnGameTask *pTask = pRoom->GetGameTask();
		if( pTask )
		{
			if (pRoom->bIsPvPRoom())
			{
				if( pRoom->bIsZombieMode() ) 
					return 4; // Ghoul mode must not be affected by other modifiers.

				char cPvPRegulation = 1;

				if( pRoom->bIsLadderRoom() )
					cPvPRegulation = 3;
				if( pRoom->bIsGuildWarSystem() )
					cPvPRegulation = 5;

				if( pRoom->bIsLevelRegulation() == false )
					return 0;

				return cPvPRegulation;
			}
			else if( pRoom->bIsDLRoom() )
			{
#ifdef PRE_ADD_CHALLENGE_DARKLAIR
				CDnDLGameTask *pDLTask = (CDnDLGameTask *)pTask;
				if( pDLTask && pDLTask->IsChallengeDarkLair() )
				{
					return 7;
				}
#endif

				return 2;
			}
			else
			{
#if defined( PRE_NORMALSTAGE_REGULATION )
				const TMapInfo* pMapInfo = g_pDataManager->GetMapInfo( pRoom->GetGameTask()->GetMapTableID() );
				if(pMapInfo)
				{
					return pMapInfo->cRevisionNum;
				}
#endif
			}
		}
	}
	
	return 0;

#else // _CLIENT

	CDnGameTask *pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask ) 
	{
		switch( pGameTask->GetGameTaskType() ) 
		{
			case GameTaskType::PvP:
				{
					if( ((CDnPvPGameTask*)pGameTask)->GetGameMode() == PvPCommon::GameMode::PvP_Zombie_Survival )
						return 4; // Ghoul mode must not be affected by other modifiers.

					char cPvPRegulation = 1;

					if( ((CDnPvPGameTask*)pGameTask)->IsLadderMode() == true )
						cPvPRegulation = 3;
					if( ((CDnPvPGameTask*)pGameTask)->GetGameMode() == PvPCommon::GameMode::PvP_GuildWar )
						cPvPRegulation = 5;
					if( ((CDnPvPGameTask*)pGameTask)->IsUseRegulation() == false )
						return 0;

					return  cPvPRegulation;
				}
				break;
			case GameTaskType::DarkLair:
				{
#ifdef PRE_ADD_CHALLENGE_DARKLAIR
					CDnDLGameTask* pDarkLairGameTask = static_cast<CDnDLGameTask*>(pGameTask);
					if( pDarkLairGameTask && pDarkLairGameTask->IsChallengeDarkLair() )
						return 7;
#endif

					return 2;
				}
				break;
			case GameTaskType::Normal:
				{
#if defined( PRE_NORMALSTAGE_REGULATION )
					DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
					if ( pSox )
					{
						return pSox->GetFieldFromLablePtr( pGameTask->GetGameMapIndex(), "_RevisionNum" )->GetInteger();	
					}
#endif
				}
				break;
		}
	}

	return 0;

#endif

}

void CDnPlayerState::CalcStrength( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetStrength( CPlayerLevelTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CPlayerLevelTable::Strength ) );
	}
	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetStrength() );
		nTemp = nAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetStrengthRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetStrength( nTemp );

#ifdef PRE_ADD_BASE_STAT_REGULATION
		OnCalcEquipStep( ST_Strength, &m_StateStep[0] );
#endif

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetStrength() );
		nBuffValue = m_StateStep[0].GetStrength() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetStrengthRatio() );
#ifdef PRE_ADD_BASIC_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( pVecRatioBuffStateList , nBuffValue, fBuffRatioValue, GetStrengthRatio() 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Strength_Ratio_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Strength_Ratio_Max ) );
#endif
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetStrength( nBuffValue - m_StateStep[0].GetStrength() );

		m_StateStep[0].SetStrength( nBuffValue );
#endif


	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetStrength(), fRatioValue, GetStrengthRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetStrength() );
		nTemp += nAddValue;

		m_StateStep[1].SetStrength( nTemp );
	}
	m_nStrength = m_StateStep[0].GetStrength() + m_StateStep[1].GetStrength();
}

void CDnPlayerState::CalcAgility( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetAgility( CPlayerLevelTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CPlayerLevelTable::Agility ) );
	}
	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetAgility() );
		nTemp = nAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetAgilityRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetAgility( nTemp );

#ifdef PRE_ADD_BASE_STAT_REGULATION
		OnCalcEquipStep( ST_Agility, &m_StateStep[0] );
#endif

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetAgility() );
		nBuffValue = m_StateStep[0].GetAgility() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetAgilityRatio() );
#ifdef PRE_ADD_BASIC_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( pVecRatioBuffStateList , nBuffValue, fBuffRatioValue, GetAgilityRatio() 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Agility_Ratio_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Agility_Ratio_Max ) );
#endif
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetAgility( nBuffValue - m_StateStep[0].GetAgility() );

		m_StateStep[0].SetAgility( nBuffValue );
#endif
	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetAgility(), fRatioValue, GetAgilityRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetAgility() );
		nTemp += nAddValue;

		m_StateStep[1].SetAgility( nTemp );
	}
	m_nAgility = m_StateStep[0].GetAgility() + m_StateStep[1].GetAgility();
}

void CDnPlayerState::CalcIntelligence( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetIntelligence( CPlayerLevelTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CPlayerLevelTable::Intelligence ) );
	}
	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetIntelligence() );
		nTemp = nAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetIntelligenceRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetIntelligence( nTemp );

#ifdef PRE_ADD_BASE_STAT_REGULATION
		OnCalcEquipStep( ST_Intelligence, &m_StateStep[0] );
#endif

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetIntelligence() );
		nBuffValue = m_StateStep[0].GetIntelligence() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetIntelligenceRatio() );
#ifdef PRE_ADD_BASIC_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( pVecRatioBuffStateList , nBuffValue, fBuffRatioValue, GetIntelligenceRatio() 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Intelligence_Ratio_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Intelligence_Ratio_Max ) );
#endif
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetIntelligence( nBuffValue - m_StateStep[0].GetIntelligence() );

		m_StateStep[0].SetIntelligence( nBuffValue );
#endif

	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetIntelligence(), fRatioValue, GetIntelligenceRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetIntelligence() );
		nTemp += nAddValue;

		m_StateStep[1].SetIntelligence( nTemp );
	}

	m_nIntelligence = m_StateStep[0].GetIntelligence() + m_StateStep[1].GetIntelligence();
}

void CDnPlayerState::CalcStamina( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetStamina( CPlayerLevelTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CPlayerLevelTable::Stamina ) );
	}
	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetStamina() );
		nTemp = nAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetStaminaRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetStamina( nTemp );

#ifdef PRE_ADD_BASE_STAT_REGULATION
		OnCalcEquipStep( ST_Stamina, &m_StateStep[0] );
#endif

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetStamina() );
		nBuffValue = m_StateStep[0].GetStamina() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetStaminaRatio() );
#ifdef PRE_ADD_BASIC_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( pVecRatioBuffStateList , nBuffValue, fBuffRatioValue, GetStaminaRatio() 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Stamina_Ratio_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Stamina_Ratio_Max ) );
#endif
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetStamina( nBuffValue - m_StateStep[0].GetStamina() );

		m_StateStep[0].SetStamina( nBuffValue );
#endif
	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetStamina(), fRatioValue, GetStaminaRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetStamina() );
		nTemp += nAddValue;

		m_StateStep[1].SetStamina( nTemp );
	}

	m_nStamina = m_StateStep[0].GetStamina() + m_StateStep[1].GetStamina();
}

void CDnPlayerState::CalcAttackP( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp[2] = {0,0};

	if( Extent & RefreshBase ) 
	{
		int nBaseStrength = m_nStrength;
		int nBaseAgility = m_nAgility;
#ifdef PRE_ADD_CHANGE_ATK_FORMULA_BY_STATE
		nBaseStrength = m_StateStep[0].GetStrength() - m_BuffState.GetStrength();
		nBaseAgility = m_StateStep[0].GetAgility() - m_BuffState.GetAgility();
#endif
		int nAttackPoint = (int)( nBaseStrength * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::StrengthAttack ) );
		nAttackPoint += (int)( nBaseAgility * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::AgilityAttack ) );
		m_BaseState.SetAttackPMin( nAttackPoint );
		m_BaseState.SetAttackPMax( nAttackPoint );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetAttackPMin() );
		nTemp[0] = nAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp[0], fRatioValue, GetAttackPMinRatio() );
		nTemp[0] += (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetAttackPMax() );
		nTemp[1] = nAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp[1], fRatioValue, GetAttackPMaxRatio() );
		nTemp[1] += (int)fRatioValue;

		m_StateStep[0].SetAttackPMin( nTemp[0] );
		m_StateStep[0].SetAttackPMax( nTemp[1] );

		OnCalcEquipStep( ST_AttackP, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue[2];

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetAttackPMin() );
		nBuffValue[0] = m_StateStep[0].GetAttackPMin() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue[0], fBuffRatioValue, GetAttackPMinRatio() );
#ifdef PRE_ADD_BUFF_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( pVecRatioBuffStateList , nBuffValue[0], fBuffRatioValue, GetAttackPMinRatio() 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Magical_AttackRatio_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Magical_AttackRatio_Max ) );
#endif
		nBuffValue[0] += (int)fBuffRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetAttackPMax() );
		nBuffValue[1] = m_StateStep[0].GetAttackPMax() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue[1], fBuffRatioValue, GetAttackPMaxRatio() );
#ifdef PRE_ADD_BUFF_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( pVecRatioBuffStateList , nBuffValue[1], fBuffRatioValue, GetAttackPMaxRatio() 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Magical_AttackRatio_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Magical_AttackRatio_Max ) );
#endif
		nBuffValue[1] += (int)fBuffRatioValue;

#ifdef PRE_ADD_CHANGE_ATK_FORMULA_BY_STATE
		int nBuffSkillStrengh = m_BuffState.GetStrength() + m_StateStep[1].GetStrength();
		int nBuffSkillAgility = m_BuffState.GetAgility() + m_StateStep[1].GetAgility();
		int nAttackPoint = (int)(  nBuffSkillStrengh * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::StrengthAttack ) );
		nAttackPoint += (int)( nBuffSkillAgility * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::AgilityAttack ) );

		nBuffValue[0] += nAttackPoint;
		nBuffValue[1] += nAttackPoint;
#endif

		m_BuffState.SetAttackPMin( nBuffValue[0] - m_StateStep[0].GetAttackPMin() );
		m_BuffState.SetAttackPMax( nBuffValue[1] - m_StateStep[0].GetAttackPMax() );

		m_StateStep[0].SetAttackPMin( nBuffValue[0] );
		m_StateStep[0].SetAttackPMax( nBuffValue[1] );
#endif

		// #32220 Apply state effects that must precede skill state effects first. /////////////////////////
		std::vector<CDnState *> pVecAbsoluteListBeforePost;
		std::vector<CDnState *> pVecRatioListBeforePost;

		GetStateList( BeforePostStateEffectList, ValueTypeAbsolute, pVecAbsoluteListBeforePost );
		GetStateList( BeforePostStateEffectList, ValueTypeRatio, pVecRatioListBeforePost );

		if( false == pVecAbsoluteListBeforePost.empty() || false == pVecRatioListBeforePost.empty() )
		{
			CALC_STATE_VALUE_RATIO( pVecRatioListBeforePost, m_StateStep[0].GetAttackPMin(), fRatioValue, GetAttackPMinRatio() );
			nTemp[0] = (int)fRatioValue;
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteListBeforePost, nAddValue, GetAttackPMin() );
			nTemp[0] += nAddValue;

			CALC_STATE_VALUE_RATIO( pVecRatioListBeforePost, m_StateStep[0].GetAttackPMax(), fRatioValue, GetAttackPMaxRatio() );
			nTemp[1] = (int)fRatioValue;
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteListBeforePost, nAddValue, GetAttackPMax() );
			nTemp[1] += nAddValue;

			m_StateStep[0].SetAttackPMin( m_StateStep[0].GetAttackPMin() + nTemp[ 0 ] );
			m_StateStep[0].SetAttackPMax( m_StateStep[0].GetAttackPMax() + nTemp[ 1 ] );
		}

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
		if( GetUseStageStateLimit() > 0.f )
		{
			nTemp[0] = m_StateStep[0].GetAttackPMin();
			nTemp[1] = m_StateStep[0].GetAttackPMax();
			int nEquipLevelAttackPowerMin = (int)( ( 1.f * GetUseStageStateLimit() ) * ( m_StateStep[0].GetAttackPMin() - m_BuffState.GetAttackPMin() ) );
			int nEquipLevelAttackPowerMax = (int)( ( 1.f * GetUseStageStateLimit() ) * ( m_StateStep[0].GetAttackPMax() - m_BuffState.GetAttackPMax() ) );
			CALC_STATE_LIMIT( nTemp[0], 0, nEquipLevelAttackPowerMin );
			CALC_STATE_LIMIT( nTemp[1], 0, nEquipLevelAttackPowerMax );
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
			m_sDamageLimitInfo.bPhysicalAttack = ( nTemp[0] == nEquipLevelAttackPowerMin || nTemp[1] == nEquipLevelAttackPowerMax );
#endif
			m_StateStep[0].SetAttackPMin( nTemp[0] );
			m_StateStep[0].SetAttackPMax( nTemp[1] );
		}
#endif

	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect

		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetAttackPMin(), fRatioValue, GetAttackPMinRatio() );
		nTemp[0] = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetAttackPMin() );
		nTemp[0] += nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetAttackPMax(), fRatioValue, GetAttackPMaxRatio() );
		nTemp[1] = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetAttackPMax() );
		nTemp[1] += nAddValue;


		m_StateStep[1].SetAttackPMin( nTemp[0] );
		m_StateStep[1].SetAttackPMax( nTemp[1] );
	}

	m_nAttackP[0] = m_StateStep[0].GetAttackPMin() + m_StateStep[1].GetAttackPMin();
	m_nAttackP[1] = m_StateStep[0].GetAttackPMax() + m_StateStep[1].GetAttackPMax();

#ifndef PRE_ADD_BUFF_STATE_LIMIT
	// Related to #31367. Adjust minimum attack power if it falls below a certain ratio of the original value.
	float fAttackPowerClampLowestRatio = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::AttackPowerClampLowestRatio );
	int nAttackPowerLowest = int((float)m_StateStep[0].GetAttackPMin() * fAttackPowerClampLowestRatio);
	if( m_nAttackP[ 0 ] < nAttackPowerLowest )
	{
		m_nAttackP[ 0 ] = nAttackPowerLowest;
		m_nAttackP[ 1 ] = int((float)m_StateStep[0].GetAttackPMax() * fAttackPowerClampLowestRatio);
	}
#endif
}

void CDnPlayerState::CalcAttackM( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp[2] = {0,0};

	if( Extent & RefreshBase ) 
	{
		int nBaseIntelligence = m_nIntelligence;
#ifdef PRE_ADD_CHANGE_ATK_FORMULA_BY_STATE
		nBaseIntelligence = m_StateStep[0].GetIntelligence() - m_BuffState.GetIntelligence();
#endif

		int nAttackPoint = (int)( nBaseIntelligence * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::IntelligenceAttack ) );
		m_BaseState.SetAttackMMin( nAttackPoint );
		m_BaseState.SetAttackMMax( nAttackPoint );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetAttackMMin() );
		nTemp[0] = nAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp[0], fRatioValue, GetAttackMMinRatio() );
		nTemp[0] += (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetAttackMMax() );
		nTemp[1] = nAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp[1], fRatioValue, GetAttackMMaxRatio() );
		nTemp[1] += (int)fRatioValue;

		m_StateStep[0].SetAttackMMin( nTemp[0] );
		m_StateStep[0].SetAttackMMax( nTemp[1] );
		OnCalcEquipStep( ST_AttackM, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue[2];

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetAttackMMin() );
		nBuffValue[0] = m_StateStep[0].GetAttackMMin() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue[0], fBuffRatioValue, GetAttackMMinRatio() );
#ifdef PRE_ADD_BUFF_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( pVecRatioBuffStateList , nBuffValue[0], fBuffRatioValue, GetAttackMMinRatio() 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Magical_AttackRatio_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Magical_AttackRatio_Max ) );
#endif
		nBuffValue[0] += (int)fBuffRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetAttackMMax() );
		nBuffValue[1] = m_StateStep[0].GetAttackMMax() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue[1], fBuffRatioValue, GetAttackMMaxRatio() );
#ifdef PRE_ADD_BUFF_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( pVecRatioBuffStateList , nBuffValue[1], fBuffRatioValue, GetAttackMMaxRatio() 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Magical_AttackRatio_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Magical_AttackRatio_Max ) );
#endif
		nBuffValue[1] += (int)fBuffRatioValue;

#ifdef PRE_ADD_CHANGE_ATK_FORMULA_BY_STATE
		int nBuffSkillIntelligence = m_BuffState.GetIntelligence() + m_StateStep[1].GetIntelligence();
		int nAttackPoint = (int)(  nBuffSkillIntelligence * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::IntelligenceAttack ) );
		nBuffValue[0] += nAttackPoint;
		nBuffValue[1] += nAttackPoint;
#endif

		m_BuffState.SetAttackMMin( nBuffValue[0] - m_StateStep[0].GetAttackMMin() );
		m_BuffState.SetAttackMMax( nBuffValue[1] - m_StateStep[0].GetAttackMMax() );

		m_StateStep[0].SetAttackMMin( nBuffValue[0] );
		m_StateStep[0].SetAttackMMax( nBuffValue[1] );
#endif

		// #32220 Apply state effects that must precede skill state effects first. /////////////////////////
		std::vector<CDnState *> pVecAbsoluteListBeforePost;
		std::vector<CDnState *> pVecRatioListBeforePost;

		GetStateList( BeforePostStateEffectList, ValueTypeAbsolute, pVecAbsoluteListBeforePost );
		GetStateList( BeforePostStateEffectList, ValueTypeRatio, pVecRatioListBeforePost );

		if( false == pVecAbsoluteListBeforePost.empty() || false == pVecRatioListBeforePost.empty() )
		{
			CALC_STATE_VALUE_RATIO( pVecRatioListBeforePost, m_StateStep[0].GetAttackMMin(), fRatioValue, GetAttackMMinRatio() );
			nTemp[0] = (int)fRatioValue;
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteListBeforePost, nAddValue, GetAttackMMin() );
			nTemp[0] += nAddValue;

			CALC_STATE_VALUE_RATIO( pVecRatioListBeforePost, m_StateStep[0].GetAttackMMax(), fRatioValue, GetAttackMMaxRatio() );
			nTemp[1] = (int)fRatioValue;
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteListBeforePost, nAddValue, GetAttackMMax() );
			nTemp[1] += nAddValue;

			m_StateStep[0].SetAttackMMin( m_StateStep[0].GetAttackMMin() + nTemp[ 0 ] );
			m_StateStep[0].SetAttackMMax( m_StateStep[0].GetAttackMMax() + nTemp[ 1 ] );
		}

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
		if( GetUseStageStateLimit() > 0.f )
		{
			nTemp[0] = m_StateStep[0].GetAttackMMin();
			nTemp[1] = m_StateStep[0].GetAttackMMax();
			int nEquipLevelAttackPowerMin = (int)( ( 1.f * GetUseStageStateLimit() ) * ( m_StateStep[0].GetAttackMMin() - m_BuffState.GetAttackMMin() ) );
			int nEquipLevelAttackPowerMax = (int)( ( 1.f * GetUseStageStateLimit() ) * ( m_StateStep[0].GetAttackMMax() - m_BuffState.GetAttackMMax() ) );
			CALC_STATE_LIMIT( nTemp[0], 0, nEquipLevelAttackPowerMin );
			CALC_STATE_LIMIT( nTemp[1], 0, nEquipLevelAttackPowerMax );
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
			m_sDamageLimitInfo.bMagicalAttack = ( nTemp[0] == nEquipLevelAttackPowerMin || nTemp[1] == nEquipLevelAttackPowerMax );
#endif
			m_StateStep[0].SetAttackMMin( nTemp[0] );
			m_StateStep[0].SetAttackMMax( nTemp[1] );
		}
#endif

	}

	if( Extent & RefreshSkill ) {

		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetAttackMMin(), fRatioValue, GetAttackMMinRatio() );
		nTemp[0] = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetAttackMMin() );
		nTemp[0] += nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetAttackMMax(), fRatioValue, GetAttackMMaxRatio() );
		nTemp[1] = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetAttackMMax() );
		nTemp[1] += nAddValue;

		m_StateStep[1].SetAttackMMin( nTemp[0] );
		m_StateStep[1].SetAttackMMax( nTemp[1] );
	}

	m_nAttackM[0] = m_StateStep[0].GetAttackMMin() + m_StateStep[1].GetAttackMMin();
	m_nAttackM[1] = m_StateStep[0].GetAttackMMax() + m_StateStep[1].GetAttackMMax();

#ifndef PRE_ADD_BUFF_STATE_LIMIT
	// Related to #31367. Adjust minimum attack power if it falls below a certain ratio of the original value.
	float fAttackPowerClampLowestRatio = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::AttackPowerClampLowestRatio );
	int nAttackPowerLowest = int((float)m_StateStep[0].GetAttackMMin() * fAttackPowerClampLowestRatio);
	if( m_nAttackM[ 0 ] < nAttackPowerLowest )
	{
		m_nAttackM[ 0 ] = nAttackPowerLowest;
		m_nAttackM[ 1 ] = int((float)m_StateStep[0].GetAttackMMax() * fAttackPowerClampLowestRatio);
	}
#endif
}

void CDnPlayerState::CalcDefenseP( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetDefenseP( (int)( m_nStamina * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::PhysicalDefense ) ) );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetDefenseP() );
		if( nAddValue <= 0 ) nAddValue = 1;
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetDefensePRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetDefenseP( nTemp );
		OnCalcEquipStep( ST_DefenseP, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetDefenseP() );
		nBuffValue = m_StateStep[0].GetDefenseP() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetDefensePRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetDefenseP( nBuffValue - m_StateStep[0].GetDefenseP() );

		m_StateStep[0].SetDefenseP( nBuffValue );
#endif

	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetDefenseP(), fRatioValue, GetDefensePRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetDefenseP() );
		nTemp += nAddValue;

		m_StateStep[1].SetDefenseP( nTemp );
	}

	m_nDefenseP = m_StateStep[0].GetDefenseP() + m_StateStep[1].GetDefenseP();
}

void CDnPlayerState::CalcDefenseM( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetDefenseM( (int)( m_nIntelligence * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::MagicDefense ) ) );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetDefenseM() );
		if( nAddValue <= 0 ) nAddValue = 1;
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetDefenseMRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetDefenseM( nTemp );
		OnCalcEquipStep( ST_DefenseM, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetDefenseM() );
		nBuffValue = m_StateStep[0].GetDefenseM() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetDefenseMRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetDefenseM( nBuffValue - m_StateStep[0].GetDefenseM() );

		m_StateStep[0].SetDefenseM( nBuffValue );
#endif

	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetDefenseM(), fRatioValue, GetDefenseMRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetDefenseM() );
		nTemp += nAddValue;

		m_StateStep[1].SetDefenseM( nTemp );
	}

	m_nDefenseM = m_StateStep[0].GetDefenseM() + m_StateStep[1].GetDefenseM();
}

void CDnPlayerState::CalcHP( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList, bool bRevisionCurrentHP )
{
	float fRatioValue;
	INT64 nAddValue;
	INT64 nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetMaxHP( (INT64)( m_nStamina * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::HP ) ) );
	}

	if( Extent & RefreshEquip ) {
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetMaxHP() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetMaxHPRatio() );
		nTemp += (INT64)fRatioValue;

		m_StateStep[0].SetMaxHP( nTemp );
		OnCalcEquipStep( ST_HP, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		INT64 nBuffAddValue;
		INT64 nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetMaxHP() );
		nBuffValue = m_StateStep[0].GetMaxHP() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetMaxHPRatio() );
		nBuffValue += (INT64)fBuffRatioValue;

		m_BuffState.SetMaxHP( nBuffValue - m_StateStep[0].GetMaxHP() );

		m_StateStep[0].SetMaxHP( nBuffValue );
#endif

	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetMaxHP(), fRatioValue, GetMaxHPRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetMaxHP() );
		nTemp += nAddValue;

		m_StateStep[1].SetMaxHP( nTemp );
	}

	m_nMaxHP = m_StateStep[0].GetMaxHP() + m_StateStep[1].GetMaxHP();

#if defined(PRE_ADD_MISSION_COUPON)
	if( bRevisionCurrentHP && m_nHP > m_nMaxHP ) SetHP(m_nMaxHP);
#else
	if( bRevisionCurrentHP && m_nHP > m_nMaxHP ) m_nHP = m_nMaxHP;
#endif
}

void CDnPlayerState::CalcSP(RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList, bool bRevisionCurrentSP)
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if (Extent & RefreshBase) {
		m_BaseState.SetMaxSP((int)(m_nIntelligence * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::SP)));
	}

	if (Extent & RefreshEquip) {

		CALC_STATE_VALUE_ABSOLUTE(pVecAbsoluteList[0], nAddValue, GetMaxSP());
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO(pVecRatioList[0], nTemp, fRatioValue, GetMaxSPRatio());
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetMaxSP(nTemp);
		OnCalcEquipStep(ST_SP, &m_StateStep[0]);

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetMaxSP() );
		nBuffValue = m_StateStep[0].GetMaxSP() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetMaxSPRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetMaxSP( nBuffValue - m_StateStep[0].GetMaxSP() );

		m_StateStep[0].SetMaxSP( nBuffValue );
#endif

	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetMaxSP(), fRatioValue, GetMaxSPRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetMaxSP() );
		nTemp += nAddValue;

		m_StateStep[1].SetMaxSP( nTemp );
	}
	m_nMaxSP = m_StateStep[0].GetMaxSP() + m_StateStep[1].GetMaxSP();

#if defined(PRE_ADD_MISSION_COUPON)
	if( bRevisionCurrentSP && m_nSP > m_nMaxSP ) SetSP(m_nMaxSP);
#else
	if( bRevisionCurrentSP && m_nSP > m_nMaxSP ) m_nSP = m_nMaxSP;
#endif
}

void CDnPlayerState::CalcMoveSpeed( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		float fValue1 = CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::MoveSpeed );
		m_BaseState.SetMoveSpeed( (int)fValue1 );
	}

	if( Extent & RefreshEquip ) {
		float fValue1 = CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::MoveSpeed );
		float fValue2 = CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::MoveSpeedRevision );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetMoveSpeed() );
		nTemp = (int)( fValue1 * ( 2.f / ( 1.f + exp( -1 * ( ( nAddValue - fValue1 ) / fValue2 ) ) ) ) );

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetMoveSpeedRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetMoveSpeed( nTemp );
		OnCalcEquipStep( ST_MoveSpeed, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetMoveSpeed() );
		nBuffValue = m_StateStep[0].GetMoveSpeed() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetMoveSpeedRatio() );

#ifdef PRE_ADD_SLOW_DEBUFF_LIMIT
#ifdef _GAMESERVER
		CDNGameRoom* pRoom = ((CDnPlayerActor*)m_pActor)->GetGameRoom();
		if( pRoom && pRoom->bIsPvPRoom() )
#else // _GAMESERVER
		CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
		if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP )
#endif // _GAMESERVER
		{
			MODIFY_STATE_VALUE_LIMIT_RATIO_INTEGER( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetMoveSpeedRatio() 
													, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MoveSpeedRatio_Limit_Min )
													, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MoveSpeedRatio_Limit_Max ) );
		}
#endif // PRE_ADD_SLOW_DEBUFF_LIMIT

		nBuffValue += (int)fBuffRatioValue;
		m_BuffState.SetMoveSpeed( nBuffValue - m_StateStep[0].GetMoveSpeed() );
		m_StateStep[0].SetMoveSpeed( nBuffValue );
#endif
	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetMoveSpeed(), fRatioValue, GetMoveSpeedRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetMoveSpeed() );
		nTemp += nAddValue;

		m_StateStep[1].SetMoveSpeed( nTemp );
	}

	m_nMoveSpeed = m_StateStep[0].GetMoveSpeed() + m_StateStep[1].GetMoveSpeed();
}

void CDnPlayerState::CalcDownDelay( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fAddValue;
	float fTemp;

	if( Extent & RefreshBase ) {
	}

	if( Extent & RefreshEquip ) {
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], fAddValue, GetDownDelayProb() );
		fTemp = fAddValue;
		CALC_STATE_LIMIT( fTemp, 0.f, 1.f );
		m_StateStep[0].SetDownDelayProb( fTemp );
	}
	if( Extent & RefreshSkill ) {
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], fAddValue, GetDownDelayProb() );
		fTemp = fAddValue;
		CALC_STATE_LIMIT( fTemp, 0.f, 1.f );

		m_StateStep[1].SetDownDelayProb( fTemp );
	}

	m_fDownDelayProb = m_StateStep[0].GetDownDelayProb() + m_StateStep[1].GetDownDelayProb();
	m_fDownDelayProb *= CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::DownDelay );
}

void CDnPlayerState::CalcStiff( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	int nAddValue;
	float fRatioValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetStiff( (int)( m_nStrength * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::Stiff ) ) );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetStiff() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetStiffRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetStiff( nTemp );
		OnCalcEquipStep( ST_Stiff, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetStiff() );
		nBuffValue = m_StateStep[0].GetStiff() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetStiffRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetStiff( nBuffValue - m_StateStep[0].GetStiff() );

		m_StateStep[0].SetStiff( nBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetStiff(), fRatioValue, GetStiffRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetStiff() );
		nTemp += nAddValue;

		m_StateStep[1].SetStiff( nTemp );

	}
	m_nStiff = m_StateStep[0].GetStiff() + m_StateStep[1].GetStiff();
}

void CDnPlayerState::CalcStiffResistance( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	int nAddValue;
	float fRatioValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetStiffResistance( (int)( m_nStrength * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::StiffResistance ) ) );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetStiffResistance() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetStiffResistanceRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetStiffResistance( nTemp );
		OnCalcEquipStep( ST_StiffResistance, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetStiffResistance() );
		nBuffValue = m_StateStep[0].GetStiffResistance() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetStiffResistanceRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetStiffResistance( nBuffValue - m_StateStep[0].GetStiffResistance() );

		m_StateStep[0].SetStiffResistance( nBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetStiffResistance(), fRatioValue, GetStiffResistanceRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetStiffResistance() );
		nTemp += nAddValue;

		m_StateStep[1].SetStiffResistance( nTemp );

	}
	m_nStiffResistance = m_StateStep[0].GetStiffResistance() + m_StateStep[1].GetStiffResistance();
}

void CDnPlayerState::CalcCritical( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	int nAddValue;
	float fRatioValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetCritical( (int)( m_nAgility * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::Critical ) ) );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetCritical() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetCriticalRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetCritical( nTemp );
		OnCalcEquipStep( ST_Critical, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetCritical() );
		nBuffValue = m_StateStep[0].GetCritical() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetCriticalRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetCritical( nBuffValue - m_StateStep[0].GetCritical() );

		m_StateStep[0].SetCritical( nBuffValue );
#endif

		// Same as #32220: apply state effects that must precede skill state effects first. /////////////////////////
		std::vector<CDnState *> pVecAbsoluteListBeforePost;
		std::vector<CDnState *> pVecRatioListBeforePost;

		GetStateList( BeforePostStateEffectList, ValueTypeAbsolute, pVecAbsoluteListBeforePost );
		GetStateList( BeforePostStateEffectList, ValueTypeRatio, pVecRatioListBeforePost );

		if( false == pVecAbsoluteListBeforePost.empty() || false == pVecRatioListBeforePost.empty() )
		{
			CALC_STATE_VALUE_RATIO( pVecRatioListBeforePost, m_StateStep[0].GetAttackPMin(), fRatioValue, GetCriticalRatio() );
			nTemp = (int)fRatioValue;

			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteListBeforePost, nAddValue, GetCritical() );
			nTemp += nAddValue;

			m_StateStep[0].SetCritical( m_StateStep[0].GetCritical() + nTemp );
		}
	}
	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetCritical(), fRatioValue, GetCriticalRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetCritical() );
		nTemp += nAddValue;

		m_StateStep[1].SetCritical( nTemp );

	}
	m_nCritical = m_StateStep[0].GetCritical() + m_StateStep[1].GetCritical();
}

void CDnPlayerState::CalcCriticalResistance( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	int nAddValue;
	float fRatioValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetCriticalResistance( (int)( m_nAgility * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::CriticalResistance ) ) );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetCriticalResistance() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetCriticalResistanceRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetCriticalResistance( nTemp );
		OnCalcEquipStep( ST_CriticalResistance, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetCriticalResistance() );
		nBuffValue = m_StateStep[0].GetCriticalResistance() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetCriticalResistanceRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetCriticalResistance( nBuffValue - m_StateStep[0].GetCriticalResistance() );

		m_StateStep[0].SetCriticalResistance( nBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetCriticalResistance(), fRatioValue, GetCriticalResistanceRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetCriticalResistance() );
		nTemp += nAddValue;

		m_StateStep[1].SetCriticalResistance( nTemp );

	}
	m_nCriticalResistance = m_StateStep[0].GetCriticalResistance() + m_StateStep[1].GetCriticalResistance();
}

void CDnPlayerState::CalcStun( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	int nAddValue;
	float fRatioValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetStun( (int)( m_nStrength * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::Stun ) ) );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetStun() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetStunRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetStun( nTemp );
		OnCalcEquipStep( ST_Stun, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetStun() );
		nBuffValue = m_StateStep[0].GetStun() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetStunRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetStun( nBuffValue - m_StateStep[0].GetStun() );

		m_StateStep[0].SetStun( nBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetStun(), fRatioValue, GetStunRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetStun() );
		nTemp += nAddValue;

		m_StateStep[1].SetStun( nTemp );

	}
	m_nStun = m_StateStep[0].GetStun() + m_StateStep[1].GetStun();
}

void CDnPlayerState::CalcStunResistance( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	int nAddValue;
	float fRatioValue;
	int nTemp;

	if( Extent & RefreshBase ) {
		m_BaseState.SetStunResistance( (int)( m_nStamina * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::StunResistance ) ) );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetStunResistance() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetStunResistanceRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetStunResistance( nTemp );
		OnCalcEquipStep( ST_StunResistance, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetStunResistance() );
		nBuffValue = m_StateStep[0].GetStunResistance() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetStunResistanceRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetStunResistance( nBuffValue - m_StateStep[0].GetStunResistance() );

		m_StateStep[0].SetStunResistance( nBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetStunResistance(), fRatioValue, GetStunResistanceRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetStunResistance() );
		nTemp += nAddValue;

		m_StateStep[1].SetStunResistance( nTemp );

	}
	m_nStunResistance = m_StateStep[0].GetStunResistance() + m_StateStep[1].GetStunResistance();
}

void CDnPlayerState::CalcElementAttack( ElementEnum Type, RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fAddValue;
	float fTemp;

	if( Extent & RefreshBase ) {
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], fAddValue, GetElementAttack( Type ) );
		fTemp = fAddValue * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::ElementAttack );

		m_StateStep[0].SetElementAttack( Type , fTemp );
#ifdef PRE_FIX_ELEMENT_REGULATION
		OnCalcEquipStep_ElementAttack( Type, &m_StateStep[0] );
#else
		OnCalcEquipStep( ST_ElementAttack, &m_StateStep[0] );
#endif

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffAddValue;
		float fBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , fBuffAddValue , GetElementAttack(Type) );
		fBuffValue = m_StateStep[0].GetElementAttack(Type) + fBuffAddValue;

#ifdef PRE_ADD_BUFF_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_ABSOLUTE( pVecAbsoluteBuffStateList , fBuffValue , GetElementAttack(Type) 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Attack_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Attack_Max ) );
#endif

		m_BuffState.SetElementAttack( Type , fBuffValue - m_StateStep[0].GetElementAttack( Type ) );

		m_StateStep[0].SetElementAttack( Type , fBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], fAddValue, GetElementAttack( Type ) );
		fTemp = fAddValue;
		m_StateStep[1].SetElementAttack( Type, fTemp );

	}

	m_fElementAttack[Type] = m_StateStep[0].GetElementAttack( Type ) + m_StateStep[1].GetElementAttack( Type );
}

void CDnPlayerState::CalcElementDefense( ElementEnum Type, RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fAddValue;
	float fTemp;

	if( Extent & RefreshBase ) {
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], fAddValue, GetElementDefense( Type ) );
		fTemp = fAddValue * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::ElementDefense );

		m_StateStep[0].SetElementDefense( Type , fTemp );

#ifdef PRE_FIX_ELEMENT_REGULATION
		OnCalcEquipStep_ElementDefense( Type, &m_StateStep[0] );
#else
		OnCalcEquipStep( ST_ElementDefense, &m_StateStep[0] );
#endif
		

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffAddValue;
		float fBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , fBuffAddValue , GetElementDefense(Type) );
		fBuffValue = m_StateStep[0].GetElementDefense(Type) + fBuffAddValue;

#ifdef PRE_ADD_BUFF_STATE_LIMIT
		MODIFY_STATE_VALUE_LIMIT_ABSOLUTE( pVecAbsoluteBuffStateList , fBuffValue , GetElementDefense(Type) 
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Defense_Min )
			, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Defense_Max ) );
#endif

		m_BuffState.SetElementDefense( Type , fBuffValue - m_StateStep[0].GetElementDefense( Type ) );

#ifdef PRE_ADD_ELEMENT_DEFENSE_LIMIT
		if( fBuffValue > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Defense_Limit ) )
		{
			fBuffValue = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Defense_Limit );
		}
#endif

		m_StateStep[0].SetElementDefense( Type , fBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], fAddValue, GetElementDefense( Type ) );
		fTemp = fAddValue;
		m_StateStep[1].SetElementDefense( Type, fTemp );
	}

	m_fElementDefense[Type] = m_StateStep[0].GetElementDefense( Type ) + m_StateStep[1].GetElementDefense( Type );

#ifdef PRE_ADD_ELEMENT_DEFENSE_LIMIT
	if( m_fElementDefense[Type] > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Defense_Limit ) )
	{
		m_fElementDefense[Type] = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Defense_Limit );
	}
#endif

}

void CDnPlayerState::CalcRecoverySP( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) 
	{
		m_BaseState.SetRecoverySP( (int)( GetLevelWeightValue() * CPlayerWeightTable::GetInstance().GetValue( m_nActorTableID, CPlayerWeightTable::RecoverySP ) ) );
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetRecoverySP() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetRecoverySPRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetRecoverySP( nTemp );
		OnCalcEquipStep( ST_RecoverySP, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetRecoverySP() );
		nBuffValue = m_StateStep[0].GetRecoverySP() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetRecoverySPRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetRecoverySP( nBuffValue - m_StateStep[0].GetRecoverySP() );

		m_StateStep[0].SetRecoverySP( nBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetRecoverySP(), fRatioValue, GetRecoverySPRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetRecoverySP() );
		nTemp += nAddValue;

		m_StateStep[1].SetRecoverySP( nTemp );
	}

	m_nRecoverySP = m_StateStep[0].GetRecoverySP() + m_StateStep[1].GetRecoverySP();
}

void CDnPlayerState::CalcSuperAmmor( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetSuperAmmor() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetSuperAmmorRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetSuperAmmor( nTemp );
		OnCalcEquipStep( ST_SuperAmmor, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetSuperAmmor() );
		nBuffValue = m_StateStep[0].GetSuperAmmor() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetSuperAmmorRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetSuperAmmor( nBuffValue - m_StateStep[0].GetSuperAmmor() );

		m_StateStep[0].SetSuperAmmor( nBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetSuperAmmor(), fRatioValue, GetSuperAmmorRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetSuperAmmor() );
		nTemp += nAddValue;

		m_StateStep[1].SetSuperAmmor( nTemp );
	}

	m_nSuperAmmor = m_StateStep[0].GetSuperAmmor() + m_StateStep[1].GetSuperAmmor();
}

void CDnPlayerState::CalcFinalDamage( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp = 0;

	if( Extent & RefreshBase ) {
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetFinalDamage() );
		nTemp = nAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetFinalDamageRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetFinalDamage( nTemp );
		OnCalcEquipStep( ST_FinalDamage, &m_StateStep[0] );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetFinalDamage() );
		nBuffValue = m_StateStep[0].GetFinalDamage() + nBuffAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetFinalDamageRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetFinalDamage( nBuffValue - m_StateStep[0].GetFinalDamage() );
		m_StateStep[0].SetFinalDamage( nBuffValue );
#endif

	}

	if( Extent & RefreshSkill ) 
	{
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetFinalDamageRatio(), fRatioValue, GetFinalDamageRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetFinalDamage() );
		nTemp = nAddValue;

		m_StateStep[1].SetFinalDamage( nTemp );
	}

	m_nFinalDamage = m_StateStep[0].GetFinalDamage() + m_StateStep[1].GetFinalDamage();
}

void CDnPlayerState::CalcSpirit( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetSpirit() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], nTemp, fRatioValue, GetSpiritRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetSpirit( nTemp );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetSpirit() );
		nBuffValue = m_StateStep[0].GetSpirit() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetSpiritRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetSpirit( nBuffValue - m_StateStep[0].GetSpirit() );

		m_StateStep[0].SetSpirit( nBuffValue );
#endif

	}
	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetSpirit(), fRatioValue, GetSpiritRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetSpirit() );
		nTemp += nAddValue;

		m_StateStep[1].SetSpirit( nTemp );
	}

	m_nSpirit = m_StateStep[0].GetSpirit() + m_StateStep[1].GetSpirit();
}

void CDnPlayerState::CalcSafeZoneMoveSpeed( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fRatioValue;
	int nAddValue;
	int nTemp;

	if( Extent & RefreshBase ) {
	}

	if( Extent & RefreshEquip ) {

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], nAddValue, GetSafeZoneMoveSpeed() );
		nTemp = nAddValue;

		CALC_STATE_VALUE_RATIO( pVecRatioList[0], (nTemp + m_nMoveSpeed), fRatioValue, GetSafeZoneMoveSpeedRatio() );
		nTemp += (int)fRatioValue;

		m_StateStep[0].SetSafeZoneMoveSpeed( nTemp );

#ifdef PRE_ADD_SKILLBUF_RENEW
		float fBuffRatioValue;
		int nBuffAddValue;
		int nBuffValue;

		std::vector<CDnState*> pVecAbsoluteBuffStateList;
		std::vector<CDnState*> pVecRatioBuffStateList;
		GetBuffStateList( CDnState::ValueType::ValueTypeAbsolute, pVecAbsoluteBuffStateList );
		GetBuffStateList( CDnState::ValueType::ValueTypeRatio, pVecRatioBuffStateList );

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteBuffStateList , nBuffAddValue , GetSafeZoneMoveSpeed() );
		nBuffValue = m_StateStep[0].GetSafeZoneMoveSpeed() + nBuffAddValue;
		CALC_STATE_VALUE_RATIO( pVecRatioBuffStateList, nBuffValue, fBuffRatioValue, GetSafeZoneMoveSpeedRatio() );
		nBuffValue += (int)fBuffRatioValue;

		m_BuffState.SetSafeZoneMoveSpeed( nBuffValue - m_StateStep[0].GetSafeZoneMoveSpeed() );

		m_StateStep[0].SetSafeZoneMoveSpeed( nBuffValue );
#endif

	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_RATIO( pVecRatioList[1], m_StateStep[0].GetSafeZoneMoveSpeed(), fRatioValue, GetSafeZoneMoveSpeedRatio() );
		nTemp = (int)fRatioValue;

		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], nAddValue, GetSafeZoneMoveSpeed() );
		nTemp += nAddValue;

		m_StateStep[1].SetSafeZoneMoveSpeed( nTemp );
	}

	m_nSafeZoneMoveSpeed = m_StateStep[0].GetSafeZoneMoveSpeed() + m_StateStep[1].GetSafeZoneMoveSpeed();
}

void CDnPlayerState::CalcAddExp( RefreshStateExtentEnum Extent, std::vector<CDnState *> *pVecAbsoluteList, std::vector<CDnState *> *pVecRatioList )
{
	float fAddValue;
	float fTemp;

	if( Extent & RefreshBase ) {
	}

	if( Extent & RefreshEquip ) {
		//Absolute
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[0], fAddValue, GetAddExp() );
		fTemp = fAddValue;

		m_StateStep[0].SetAddExp( fTemp );
	}

	if( Extent & RefreshSkill ) {
		// PostStateEffect
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList[1], fAddValue, GetAddExp() );
		fTemp = fAddValue;

		m_StateStep[1].SetAddExp( fTemp );
	}

	m_fAddExp = m_StateStep[0].GetAddExp() + m_StateStep[1].GetAddExp();
}

void CDnPlayerState::OnCalcEquipStep_ElementAttack( ElementEnum eElementType , CDnState *pState )
{
	char cRegulation = GetUseRegulation();
	if( cRegulation == 0 ) return;
	float fDeclarationRatio = 0.f;
	float fRevision = 0.f;

#if defined(PRE_FIX_ELEMENT_REGULATION_RENEW)
	float fCurrentElement =  pState->GetElementAttack( eElementType );
	float fBasisValue = (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::ElementAttackRevision );

	fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::ElementAttackRevision );
	fDeclarationRatio = pow( fCurrentElement / fBasisValue , fRevision );
	pState->SetElementAttack( eElementType , CEqualLevelTable::GetInstance().GetValueFloat( cRegulation, GetJobClassID(), CEqualLevelTable::ElementAttackRevision )  * fDeclarationRatio );
#else
	float fCurrentElement =  pState->GetElementAttack( eElementType );
	float fExpotentLimit = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Expotent_Limit );
	float fBasisValue = (float)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_EqualBasis );

	if( fCurrentElement <=  fExpotentLimit)
	{
		pState->SetElementAttack( eElementType , fCurrentElement );
	}
	else
	{
		fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::ElementAttackRevision );
		fDeclarationRatio = pow( fCurrentElement / fBasisValue , fRevision );
		pState->SetElementAttack( eElementType , (float)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_EqualLevel ) * fDeclarationRatio );
	}
#endif

}

void CDnPlayerState::OnCalcEquipStep_ElementDefense( ElementEnum eElementType , CDnState *pState )
{
	char cRegulation = GetUseRegulation();
	if( cRegulation == 0 ) return;
	float fDeclarationRatio = 0.f;
	float fRevision = 0.f;

#ifdef PRE_FIX_ELEMENT_REGULATION_RENEW // Configure to operate normally just like other adjustments
	float fCurrentElement =  pState->GetElementDefense( eElementType );
	float fBasisValue = (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::ElementDefenseRevision );

	fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::ElementDefenseRevision );
	fDeclarationRatio = pow( fCurrentElement / fBasisValue , fRevision );
	pState->SetElementDefense( eElementType , CEqualLevelTable::GetInstance().GetValueFloat( cRegulation, GetJobClassID(), CEqualLevelTable::ElementDefenseRevision )  * fDeclarationRatio );
#endif

}

void CDnPlayerState::OnCalcEquipStep( StateTypeEnum Type, CDnState *pState )
{
	char cRegulation = GetUseRegulation();
	if( cRegulation == 0 ) return;
	float fDeclarationRatio = 0.f;
	float fRevision = 0.f;

	switch( Type ) {
#if defined(PRE_ADD_REVEAL_REGULATION_VALUE) || defined(_GAMESERVER)
		case ST_AttackP:
			{

#ifdef PRE_FIX_MODIFY_AVERAGE_DAMAGE_REGULATION
				float fMinRevisionSquare = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::PhysicalAttackMinRevision );
				float fMaxRevisionSquare = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::PhysicalAttackMaxRevision );
				fRevision = (fMinRevisionSquare + fMaxRevisionSquare) / 2; // Average of Square values to compare

				float fMinRevisionValue = (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::PhysicalAttackMinRevision );
				float fMaxRevisionValue = (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::PhysicalAttackMaxRevision );
				float fTotalRevisionValue = fMinRevisionValue + fMaxRevisionValue; // Sum of Value values to compare

				float fTotalAttackValue = (float)pState->GetAttackPMin() + (float)pState->GetAttackPMax();
				fDeclarationRatio = pow( ( fTotalAttackValue / fTotalRevisionValue ) , fRevision );

				float fBasisValueMin = (float)CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::PhysicalAttackMinRevision );
				float fBaseisValueMax = (float)CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::PhysicalAttackMaxRevision ); 

				float fResultTotalValue = (fBasisValueMin + fBaseisValueMax) * fDeclarationRatio;
				float fResultMinValue = fResultTotalValue * ( (float)pState->GetAttackPMin() / (float)(pState->GetAttackPMin() + pState->GetAttackPMax()) );
				float fResultMaxValue = fResultTotalValue * ( (float)pState->GetAttackPMax() / (float)(pState->GetAttackPMin() + pState->GetAttackPMax()) );

				pState->SetAttackPMin( (int)fResultMinValue );
				pState->SetAttackPMax( (int)fResultMaxValue );
#else
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::PhysicalAttackMinRevision );
				fDeclarationRatio = pow( pState->GetAttackPMin() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::PhysicalAttackMinRevision ), fRevision );
				pState->SetAttackPMin( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::PhysicalAttackMinRevision ) * fDeclarationRatio ) );

				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::PhysicalAttackMaxRevision );
				fDeclarationRatio = pow( pState->GetAttackPMax() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::PhysicalAttackMaxRevision ), fRevision );
				pState->SetAttackPMax( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::PhysicalAttackMaxRevision ) * fDeclarationRatio ) );
#endif

			}
			break;
		case ST_AttackM:
			{
#ifdef PRE_FIX_MODIFY_AVERAGE_DAMAGE_REGULATION
				float fMinRevisionSquare = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::MagicAttackMinRevision );
				float fMaxRevisionSquare = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::MagicAttackMaxRevision );
				fRevision = (fMinRevisionSquare + fMaxRevisionSquare) / 2; // Average of Square values to compare

				float fMinRevisionValue = (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::MagicAttackMinRevision );
				float fMaxRevisionValue = (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::MagicAttackMaxRevision );
				fDeclarationRatio = fMinRevisionValue + fMaxRevisionValue; // Sum of Value values to compare

				float fTotalAttackValue = (float)pState->GetAttackMMin() + (float)pState->GetAttackMMax();
				float fResultTotalRatio = pow( (fTotalAttackValue / fDeclarationRatio ) , fRevision );

				float fBasisValueMin = (float)CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::MagicAttackMinRevision );
				float fBaseisValueMax = (float)CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::MagicAttackMaxRevision ); 
				
				float fResultTotalValue = (fBasisValueMin + fBaseisValueMax) * fResultTotalRatio;
				float fResultMinValue = fResultTotalValue * ( (float)pState->GetAttackMMin() / (float)(pState->GetAttackMMin() + pState->GetAttackMMax()) );
				float fResultMaxValue = fResultTotalValue * ( (float)pState->GetAttackMMax() / (float)(pState->GetAttackMMin() + pState->GetAttackMMax()) );

				pState->SetAttackMMin( (int)fResultMinValue );
				pState->SetAttackMMax( (int)fResultMaxValue );
#else
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::MagicAttackMinRevision );
				fDeclarationRatio = pow( pState->GetAttackMMin() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::MagicAttackMinRevision ), fRevision );
				pState->SetAttackMMin( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::MagicAttackMinRevision ) * fDeclarationRatio ) );

				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::MagicAttackMaxRevision );
				fDeclarationRatio = pow( pState->GetAttackMMax() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::MagicAttackMaxRevision ), fRevision );
				pState->SetAttackMMax( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::MagicAttackMaxRevision ) * fDeclarationRatio ) );
#endif

			}
			break;
		case ST_DefenseP:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::PhysicalDefenseRevision );
				fDeclarationRatio = pow( pState->GetDefenseP() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::PhysicalDefenseRevision ), fRevision );
				pState->SetDefenseP( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::PhysicalDefenseRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_DefenseM:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::MagicDefenseRevision );
				fDeclarationRatio = pow( pState->GetDefenseM() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::MagicDefenseRevision ), fRevision );
				pState->SetDefenseM( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::MagicDefenseRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_Stiff:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::StiffRevision );
				fDeclarationRatio = pow( pState->GetStiff() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::StiffRevision ), fRevision );
				pState->SetStiff( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::StiffRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_StiffResistance:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::StiffResistanceRevision );
				fDeclarationRatio = pow( pState->GetStiffResistance() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::StiffResistanceRevision ), fRevision );
				pState->SetStiffResistance( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::StiffResistanceRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_Critical:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::CriticalRevision );
				fDeclarationRatio = pow( pState->GetCritical() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::CriticalRevision ), fRevision );
				pState->SetCritical( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::CriticalRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_CriticalResistance:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::CriticalResistanceRevision );
				fDeclarationRatio = pow( pState->GetCriticalResistance() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::CriticalResistanceRevision ), fRevision );
				pState->SetCriticalResistance( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::CriticalResistanceRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_Stun:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::StunRevision );
				fDeclarationRatio = pow( pState->GetStun() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::StunRevision ), fRevision );
				pState->SetStun( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::StunRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_StunResistance:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::StunResistanceRevision );
				fDeclarationRatio = pow( pState->GetStunResistance() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::StunResistanceRevision ), fRevision );
				pState->SetStunResistance( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::StunResistanceRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_SuperAmmor:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::SuperAmmorRevision );
				fDeclarationRatio = pow( pState->GetSuperAmmor() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::SuperAmmorRevision ), fRevision );
				pState->SetSuperAmmor( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::SuperAmmorRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_RecoverySP:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::RecoverySPRevision );
				fDeclarationRatio = pow( pState->GetRecoverySP() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::RecoverySPRevision ), fRevision );
				pState->SetRecoverySP( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::RecoverySPRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_ElementAttack:
			{
				// Element is loop-based so multiple passes skew modifiers, e.g. changing Light refreshes remaining elements 
				// Must be set only once per element via OnCalcEqupStep_Element.
#ifndef PRE_FIX_ELEMENT_REGULATION
				float fDeclaration = CEqualLevelTable::GetInstance().GetValueFloat( cRegulation, GetJobClassID(), CEqualLevelTable::ElementAttackRevision );
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::ElementAttackRevision );

				for( int i=0; i<ElementEnum_Amount; i++ )
				{
					pState->SetElementAttack( (ElementEnum)i, fDeclaration * pow( pState->GetElementAttack( (ElementEnum)i ), fRevision ) );
				}
#endif
			}
			break;
		case ST_ElementDefense:
			{
				// Element is loop-based so multiple passes re-adjust modified values, e.g. changing Light refreshes remaining elements 
				// Must be set only once per element via OnCalcEqupStep_Element.
				// Design said not to touch elemental defense for now, so leave it as is. However, the formula itself is wrong, so table values must not be misconfigured.
#ifndef PRE_FIX_ELEMENT_REGULATION
				float fDeclaration = CEqualLevelTable::GetInstance().GetValueFloat( cRegulation, GetJobClassID(), CEqualLevelTable::ElementDefenseRevision );
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::ElementDefenseRevision );

				for( int i=0; i<ElementEnum_Amount; i++ ) {
					pState->SetElementDefense( (ElementEnum)i, fDeclaration * pow( pState->GetElementDefense( (ElementEnum)i ), fRevision ) );
				}
#endif
			}
			break;
		case ST_FinalDamage:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::FinalDamageRevision );
				fDeclarationRatio = pow( pState->GetFinalDamage() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::FinalDamageRevision ), fRevision );
				pState->SetFinalDamage( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::FinalDamageRevision ) * fDeclarationRatio ) );
			}
			break;
#endif // #if defined(PRE_ADD_REVEAL_REGULATION_VALUE) || defined(_GAMESERVER)

		case ST_HP:
			{			
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::MaxHPRevision );
				fDeclarationRatio = pow( pState->GetMaxHP() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::MaxHPRevision ), fRevision );
				pState->SetMaxHP( (INT64)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::MaxHPRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_SP:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::MaxSPRevision );
				fDeclarationRatio = pow( pState->GetMaxSP() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::MaxSPRevision ), fRevision );
				pState->SetMaxSP( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::MaxSPRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_MoveSpeed:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::MoveSpeedRevision );
				fDeclarationRatio = pow( pState->GetMoveSpeed() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::MoveSpeedRevision ), fRevision );
				pState->SetMoveSpeed( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::MoveSpeedRevision ) * fDeclarationRatio ) );
			}
			break;
#ifdef PRE_ADD_BASE_STAT_REGULATION
		case ST_Strength:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::StrengthRevision );
				fDeclarationRatio = pow( pState->GetStrength() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::StrengthRevision ), fRevision );
				pState->SetStrength( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::StrengthRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_Agility:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::AgilityRevision );
				fDeclarationRatio = pow( pState->GetAgility() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::AgilityRevision ), fRevision );
				pState->SetAgility( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::AgilityRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_Intelligence:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::IntelligenceRevision );
				fDeclarationRatio = pow( pState->GetIntelligence() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::IntelligenceRevision ), fRevision );
				pState->SetIntelligence( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::IntelligenceRevision ) * fDeclarationRatio ) );
			}
			break;
		case ST_Stamina:
			{
				fRevision = CEqualExponentTable::GetInstance().GetValue( cRegulation, CEqualExponentTable::StaminaRevision );
				fDeclarationRatio = pow( pState->GetStamina() / (float)CEqualBasisTable::GetInstance().GetValue( GetJobClassID(), m_nLevel, CEqualBasisTable::StaminaRevision ), fRevision );
				pState->SetStamina( (int)( CEqualLevelTable::GetInstance().GetValue( cRegulation, GetJobClassID(), CEqualLevelTable::StaminaRevision ) * fDeclarationRatio ) );
			}
			break;
#endif
	}
}

int CDnPlayerState::GetJobClassID()
{
	if( m_nVecJobHistoryList.empty() ) return 0;
	return m_nVecJobHistoryList[ m_nVecJobHistoryList.size() - 1 ];
}

int CDnPlayerState::GetJobHistory( std::vector<int> &nVecResult )
{
	nVecResult = m_nVecJobHistoryList;
	return (int)m_nVecJobHistoryList.size();
}

void CDnPlayerState::SetJobHistory( const std::vector<int> &nVecList )
{
	m_nVecJobHistoryList = nVecList;
}

void CDnPlayerState::SetJobHistory( int nValue )
{
	//m_nVecJobHistoryList.clear();
	vector<int>::iterator iter = find( m_nVecJobHistoryList.begin(), m_nVecJobHistoryList.end(), nValue );
	_ASSERT( iter == m_nVecJobHistoryList.end() && "Received job change notice even though job already exists. Cheat key may have been misused." );
	if( iter == m_nVecJobHistoryList.end() )
		m_nVecJobHistoryList.push_back( nValue );
}

// #26902 Roll back the last job change. The server never does this currently. Used on the client for temporary job changes.
void CDnPlayerState::PopJobHistory( int nValue )
{
	if( nValue == m_nVecJobHistoryList.back() )
		m_nVecJobHistoryList.pop_back();
}

bool CDnPlayerState::IsPassJob( int nJobClassID )
{
	for( DWORD i=0; i<m_nVecJobHistoryList.size(); i++ ) {
		if( m_nVecJobHistoryList[i] == nJobClassID ) return true;
	}
	return false;
}

void CDnPlayerState::OnChangeJob( int nJobID )
{
	RefreshState();
}

void CDnPlayerState::SetExperience( int nValue )
{
	m_nExperience = nValue;
	CalcExperience();
}

void CDnPlayerState::AddExperience( int nValue, int nLogCode, INT64 biFKey )
{
#if defined(_GAMESERVER)
	if( GetAccountLevel() < AccountLevel_Master ) {
		int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
		if( m_nLevel >= nLevelLimit ) return;
	}
#endif

	m_nExperience += nValue;
	OnAddExperience( nValue, nLogCode, biFKey );
	CalcExperience();
}

void CDnPlayerState::CalcExperience()
{
	if( m_ActorType > ActorTypeEnum::Reserved6 ) return;

	int nExperience;
	int nTempLevel = 1;
	int nOffset = m_nLevel - 1;

	if( nOffset < 0 ) nOffset = 0;

	for( int i=nOffset; i<PLAYER_MAX_LEVEL; i++ ) {
		nExperience = CPlayerLevelTable::GetInstance().GetValue( GetJobClassID(), i + 1, CPlayerLevelTable::Experience );
		if( m_nExperience >= nExperience ) {
			m_nCurrentLevelExperience = nExperience;
			nTempLevel = i + 1;
		}
		else {
			m_nNextLevelExperience = nExperience;
			break;
		}
	}
#if defined(_GAMESERVER)
	if( GetAccountLevel() < AccountLevel_Master ) {
		int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
		if( nTempLevel >= nLevelLimit ) {
			nTempLevel = nLevelLimit;
			m_nExperience = CPlayerLevelTable::GetInstance().GetValue( GetJobClassID(), nLevelLimit, CPlayerLevelTable::Experience );
			m_nCurrentLevelExperience = m_nExperience;
			m_nNextLevelExperience = m_nExperience;
		}
	}
#endif

	if( m_nLevel < nTempLevel ) {
		int nLevelUpAmount = nTempLevel - m_nLevel;
		SetLevel( nTempLevel );
		OnLevelUp( nTempLevel, nLevelUpAmount );
	}

}

int CDnPlayerState::GetJobToBaseClassID( int nJobID )
{
	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TJOB);
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"JobTable.ext failed\r\n");
		return nJobID;
	}

	if( !pSox->IsExistItem( nJobID ) ) return nJobID;

	return pSox->GetFieldFromLablePtr( nJobID, "_BaseClass" )->GetInteger() + 1;
}

CDnState CDnPlayerState::MakeEquipState()
{
	CDnState SumAbsolute;
	CDnState SumRatio;
	CDnState PureBaseState;


	std::vector<CDnState *> pVecAbsoluteList;
	std::vector<CDnState *> pVecRatioList;

	// Do not fetch base state value here.
	GetStateList( WeaponList | DefenseList | AccessoryList | SetItemList | AppellationList | GlyphList | TalismanList | PetAbility, ValueTypeAbsolute, pVecAbsoluteList );
	GetStateList( WeaponList | DefenseList | AccessoryList | SetItemList | AppellationList | GlyphList | TalismanList | PetAbility, ValueTypeRatio, pVecRatioList );

	// Among base stats, the cases below are indirectly affected by other stats, so
	// Recalculate to retain only values increased by equipment.

	PureBaseState = m_BaseState; // Same by default

								 // Remove buff-increased values from equipment-increased values = becomes base value increased only by stats affected up to equipment.
	int nBaseStrength = m_StateStep[0].GetStrength() - m_BuffState.GetStrength();
	int nBaseAgility = m_StateStep[0].GetAgility() - m_BuffState.GetAgility();
	int nBaseIntelligence = m_StateStep[0].GetIntelligence() - m_BuffState.GetIntelligence();
	int nBaseStamina = m_StateStep[0].GetStamina() - m_BuffState.GetStamina();

	int nAttackPoint = (int)(nBaseStrength * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::StrengthAttack));
	nAttackPoint += (int)(nBaseAgility * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::AgilityAttack));
	PureBaseState.SetAttackPMin(nAttackPoint);
	PureBaseState.SetAttackPMax(nAttackPoint);

	nAttackPoint = (int)(nBaseIntelligence * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::IntelligenceAttack));
	PureBaseState.SetAttackMMin(nAttackPoint);
	PureBaseState.SetAttackMMax(nAttackPoint);

	PureBaseState.SetMaxHP((INT64)(nBaseStamina * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::HP)));
	PureBaseState.SetDefenseP((int)(nBaseStamina * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::PhysicalDefense)));
	PureBaseState.SetStunResistance((int)(nBaseStamina * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::StunResistance)));

	PureBaseState.SetDefenseM((int)(nBaseIntelligence * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::MagicDefense)));
	PureBaseState.SetMaxSP((int)(nBaseIntelligence * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::SP)));

	PureBaseState.SetStun((int)(nBaseStrength * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::Stun)));
	PureBaseState.SetStiff((int)(nBaseStrength * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::Stiff)));
	PureBaseState.SetStiffResistance((int)(nBaseStrength * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::StiffResistance)));

	PureBaseState.SetCritical((int)(nBaseAgility * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::Critical)));
	PureBaseState.SetCriticalResistance((int)(nBaseAgility * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::CriticalResistance)));

	for( DWORD i=0; i<pVecAbsoluteList.size(); i++ )
	{
		CDnState *pState = pVecAbsoluteList[i];
		if( pState )
			SumAbsolute.MergeState( *pState, ValueTypeAbsolute );
	}

	// Insert base stats increased purely by equipment
	SumAbsolute.MergeState( PureBaseState , ValueTypeAbsolute );

	for( DWORD i=0; i<pVecRatioList.size(); i++ )
	{
		CDnState *pState = pVecRatioList[i];
		if( pState )
			SumRatio.MergeState( *pState, ValueTypeRatio );
	}

	// PureBaseState base stats are not used in multiplicative calculations, so there is no need to add them.

	SumAbsolute.CalculateRatioValue( SumRatio );

	return SumAbsolute;
}

CDnState CDnPlayerState::MakeComparePotentialState(CDnItem* pCurrItemState, CDnItem* pNewItemState)
{
	ValueType eType = ValueType::ValueTypeNone;
	CDnState  SumAbsolute;
	CDnState  SumRatio;
	CDnState* pTempPotentialState = NULL;
	CDnState* pTempEnchantState = NULL;
	std::vector<CDnState*> pVecAbsoluteList;
	std::vector<CDnState*> pVecRatioList;

	GetStateList( WeaponList | DefenseList | AccessoryList | StateEffectList | SetItemList | AppellationList | GlyphList | TalismanList | PetAbility, ValueTypeAbsolute, pVecAbsoluteList );
	GetStateList( WeaponList | DefenseList | AccessoryList | StateEffectList | SetItemList | AppellationList | GlyphList | TalismanList | PetAbility, ValueTypeRatio, pVecRatioList );

#pragma region --------------------------  Pass 1  --------------------------
	for( DWORD i=0; i<pVecAbsoluteList.size(); i++ )
	{
		CDnState *pState = pVecAbsoluteList[i];
		if( pState )
			SumAbsolute.MergeState( *pState, ValueTypeAbsolute );
	}
	SumAbsolute.MergeState( m_BaseState , ValueTypeAbsolute );

	for( DWORD i=0; i<pVecRatioList.size(); i++ )
	{
		CDnState *pState = pVecRatioList[i];
		if( pState )
			SumRatio.MergeState( *pState, ValueTypeRatio );
	}

	if(pCurrItemState) 
	{
		SumAbsolute -= (CDnState)*pCurrItemState;
		pTempPotentialState = (CDnState*)pCurrItemState->GetPotentialState();
		pTempEnchantState	= (CDnState*)pCurrItemState->GetEnchantState();
		
		if(pTempPotentialState) 
		{
			eType = pTempPotentialState->GetValueType();
			if(eType == ValueTypeAbsolute)
				SumAbsolute -= *pTempPotentialState;
			else if(eType == ValueTypeRatio)
				SumRatio -= *pTempPotentialState;
		}

		if(pTempEnchantState)
		{
			eType = pTempEnchantState->GetValueType();
			if(eType == ValueTypeAbsolute)
				SumAbsolute -= *pTempEnchantState;
			else if(eType == ValueTypeRatio)
				SumRatio -= *pTempEnchantState;
		}
	}

	if(pNewItemState) 
	{
		SumAbsolute += (CDnState)*pNewItemState;
		pTempPotentialState = (CDnState*)pNewItemState->GetPotentialState();
		pTempEnchantState	= (CDnState*)pNewItemState->GetEnchantState();
		
		if(pTempPotentialState) 
		{
			eType = pTempPotentialState->GetValueType();
			if(eType == ValueTypeAbsolute)
				SumAbsolute += *pTempPotentialState;
			else if(eType == ValueTypeRatio)
				SumRatio += *pTempPotentialState;
		}

		if(pTempEnchantState)
		{
			eType = pTempEnchantState->GetValueType();
			if(eType == ValueTypeAbsolute)
				SumAbsolute += *pTempEnchantState;
			else if(eType == ValueTypeRatio)
				SumRatio += *pTempEnchantState;
		}
	}

	SumAbsolute.CalculateRatioValue( SumRatio );
#pragma  endregion


#pragma region --------------------------  Pass 2 Base Recalc  --------------------------
	CDnState ResultBaseState = m_BaseState;

	int nBaseStrength = SumAbsolute.GetStrength();
	int nBaseAgility = SumAbsolute.GetAgility();
	int nBaseIntelligence = SumAbsolute.GetIntelligence();
	int nBaseStamina = SumAbsolute.GetStamina();

	int nAttackPoint = (int)(nBaseStrength * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::StrengthAttack));
	nAttackPoint += (int)(nBaseAgility * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::AgilityAttack));
	ResultBaseState.SetAttackPMin(nAttackPoint);
	ResultBaseState.SetAttackPMax(nAttackPoint);

	nAttackPoint = (int)(nBaseIntelligence * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::IntelligenceAttack));
	ResultBaseState.SetAttackMMin(nAttackPoint);
	ResultBaseState.SetAttackMMax(nAttackPoint);

	ResultBaseState.SetMaxHP((INT64)(nBaseStamina * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::HP)));
	ResultBaseState.SetDefenseP((int)(nBaseStamina * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::PhysicalDefense)));
	ResultBaseState.SetStunResistance((int)(nBaseStamina * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::StunResistance)));

	ResultBaseState.SetDefenseM((int)(nBaseIntelligence * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::MagicDefense)));
	ResultBaseState.SetMaxSP((int)(nBaseIntelligence * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::SP)));

	ResultBaseState.SetStun((int)(nBaseStrength * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::Stun)));
	ResultBaseState.SetStiff((int)(nBaseStrength * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::Stiff)));
	ResultBaseState.SetStiffResistance((int)(nBaseStrength * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::StiffResistance)));

	ResultBaseState.SetCritical((int)(nBaseAgility * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::Critical)));
	ResultBaseState.SetCriticalResistance((int)(nBaseAgility * CPlayerWeightTable::GetInstance().GetValue(m_nActorTableID, CPlayerWeightTable::CriticalResistance)));
#pragma  endregion

	CDnState NewSumAbsolute;
	CDnState NewSumRatio;
	for( DWORD i=0; i<pVecAbsoluteList.size(); i++ )
	{
		CDnState *pState = pVecAbsoluteList[i];
		if( pState )
			NewSumAbsolute.MergeState( *pState, ValueTypeAbsolute );
	}
	NewSumAbsolute.MergeState( ResultBaseState , ValueTypeAbsolute );

	for( DWORD i=0; i<pVecRatioList.size(); i++ )
	{
		CDnState *pState = pVecRatioList[i];
		if( pState )
			NewSumRatio.MergeState( *pState, ValueTypeRatio );
	}

	if(pCurrItemState) 
	{
		NewSumAbsolute -= (CDnState)*pCurrItemState;
		pTempPotentialState = (CDnState*)pCurrItemState->GetPotentialState();
		pTempEnchantState	= (CDnState*)pCurrItemState->GetEnchantState();
		
		if(pTempPotentialState) 
		{
			eType = pTempPotentialState->GetValueType();
			if(eType == ValueTypeAbsolute)
				NewSumAbsolute -= *pTempPotentialState;
			else if(eType == ValueTypeRatio)
				NewSumRatio -= *pTempPotentialState;
		}

		if(pTempEnchantState)
		{
			eType = pTempEnchantState->GetValueType();
			if(eType == ValueTypeAbsolute)
				NewSumAbsolute -= *pTempEnchantState;
			else if(eType == ValueTypeRatio)
				NewSumRatio -= *pTempEnchantState;
		}
	}

	if(pNewItemState) 
	{
		NewSumAbsolute += (CDnState)*pNewItemState;
		pTempPotentialState = (CDnState*)pNewItemState->GetPotentialState();
		pTempEnchantState	= (CDnState*)pNewItemState->GetEnchantState();
		
		if(pTempPotentialState)
		{
			eType = pTempPotentialState->GetValueType();
			if(eType == ValueTypeAbsolute)
				NewSumAbsolute += *pTempPotentialState;
			else if(eType == ValueTypeRatio)
				NewSumRatio += *pTempPotentialState;
		}
		
		if(pTempEnchantState)
		{
			eType = pTempEnchantState->GetValueType();
			if(eType == ValueTypeAbsolute)
				NewSumAbsolute += *pTempEnchantState;
			else if(eType == ValueTypeRatio)
				NewSumRatio += *pTempEnchantState;
		}
	}
	NewSumAbsolute.CalculateRatioValue( NewSumRatio ); // Final modified stat

	return NewSumAbsolute;
}

bool CDnPlayerState::IsDeveloperAccountLevel()
{
	bool bIsDeveloper = true;
	
	if(m_cAccountLevel < eAccountLevel::AccountLevel_New)
		bIsDeveloper = false;


	return bIsDeveloper;
}

bool CDnPlayerState::IsDeveloperAccountLevel(char cAccountLevel)
{
	bool bIsDeveloper = true;

	if(cAccountLevel < eAccountLevel::AccountLevel_New)
		bIsDeveloper = false;


	return bIsDeveloper;
}