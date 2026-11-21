
#include "stdafx.h"
#include "SecondarySkillEventHandlerServer.h"
#include "DNUserSession.h"
#include "DNDBConnection.h"
#include "SecondarySkill.h"
#include "DNGameDataManager.h"
#include "SecondarySkillRecipe.h"
#include "ManufactureSkill.h"
#include "SecondarySkillRepositoryServer.h"
#if defined( _GAMESERVER )
#include "DNGameRoom.h"
#include "DnItemTask.h"
#include "DnDropItem.h"
#endif // #if defined( _GAMESERVER )
#include "NpcReputationProcessor.h"
#include "DNMissionSystem.h"

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
#if defined(_GAMESERVER)
#include "DnBlow.h"
#endif // _GAMESERVER
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined( PRE_ADD_SECONDARY_SKILL )

CSecondarySkillEventHandlerServer::CSecondarySkillEventHandlerServer( CDNUserSession* pSession )
:m_pSession(pSession)
{

}

bool CSecondarySkillEventHandlerServer::OnCreate( CSecondarySkill* pSkill )
{
	_ASSERT( m_pSession->CheckDBConnection() );
	if( m_pSession->CheckDBConnection() == false )
		return false;	

	// Send packet to client
	m_pSession->SendCreateSecondarySkill( ERROR_NONE, pSkill->GetSkillID() );

	// DB M/W request
	m_pSession->GetDBConnection()->QueryAddSecondarySkill( m_pSession, pSkill->GetSkillID(), pSkill->GetType() );
	return true;
}

bool CSecondarySkillEventHandlerServer::OnDelete( CSecondarySkill* pSkill )
{
	_ASSERT( m_pSession->CheckDBConnection() );
	if( m_pSession->CheckDBConnection() == false )
		return false;	

	// Send packet to client
	m_pSession->SendDelSecondarySkill( ERROR_NONE, pSkill->GetSkillID() );

	// DB M/W request
	m_pSession->GetDBConnection()->QueryDeleteSecondarySkill( m_pSession, pSkill->GetSkillID() );
	return true;
}

bool CSecondarySkillEventHandlerServer::OnAddRecipe( CSecondarySkillRecipe* pRecipe, bool bIsCash )
{
	_ASSERT( m_pSession->CheckDBConnection() );
	if( m_pSession->CheckDBConnection() == false )
		return false;

	// Send packet to client
	m_pSession->SendAddSecondarySkillRecipe( ERROR_NONE, pRecipe );

	// DB M/W request
	m_pSession->GetDBConnection()->QueryAddSecondarySkillRecipe( m_pSession, bIsCash, pRecipe->GetItemSerial(), pRecipe->GetSkillID() );
	return true;
}

bool CSecondarySkillEventHandlerServer::OnDeleteRecipe( CSecondarySkillRecipe* pRecipe )
{
	_ASSERT( m_pSession->CheckDBConnection() );
	if( m_pSession->CheckDBConnection() == false )
		return false;

	// Send packet to client
	m_pSession->SendDeleteSecondarySkillRecipe( ERROR_NONE, pRecipe->GetSkillID(), pRecipe->GetItemID() );

	// DB M/W request
	m_pSession->GetDBConnection()->QueryDeleteSecondarySkillRecipe( m_pSession, pRecipe->GetItemSerial() );
	return true;
}

bool CSecondarySkillEventHandlerServer::OnExtractRecipe( CSecondarySkillRecipe* pRecipe, bool bIsCash )
{
	_ASSERT( m_pSession->CheckDBConnection() );
	if( m_pSession->CheckDBConnection() == false )
		return false;

	// Send packet to client
	m_pSession->SendExtractSecondarySkillRecipe( ERROR_NONE, pRecipe->GetSkillID(), pRecipe->GetItemID() );
	m_pSession->SendDeleteSecondarySkillRecipe( ERROR_NONE, pRecipe->GetSkillID(), pRecipe->GetItemID() );

	// DB M/W request
	m_pSession->GetDBConnection()->QueryExtractSecondarySkillRecipe( m_pSession, pRecipe->GetItemSerial(), bIsCash ? DBDNWorldDef::ItemLocation::CashInventory : DBDNWorldDef::ItemLocation::Inventory );

	return true;
}

bool CSecondarySkillEventHandlerServer::OnChangeExp( CSecondarySkill* pSkill, int iAddExp )
{
	TSecondarySkillLevelTableData* pLevelData = g_pDataManager->GetSecondarySkillLevelTableData( pSkill->GetSkillID(), pSkill->GetExp() );
	if( pLevelData == NULL )
		return false;

	pSkill->SetGrade( pLevelData->Grade );
	pSkill->SetLevel( pLevelData->iLevel );
	if( pSkill->GetExp() > pLevelData->iExp )
	{
		if( pSkill->GetExp()-iAddExp >= pLevelData->iExp )
			iAddExp = 0;
		else
			iAddExp -= pSkill->GetExp()-pLevelData->iExp;
		pSkill->SetExp( pLevelData->iExp, NULL );
	}

	// Adjustment value
	if( iAddExp > 0 )
	{
		// Send packet to client
		m_pSession->SendUpdateSecondarySkillExp( ERROR_NONE, pSkill->GetSkillID(), pSkill->GetGrade(), pSkill->GetExp(), pSkill->GetLevel() );

		// DB M/W request
		if( m_pSession->CheckDBConnection() )
		{
			m_pSession->GetDBConnection()->QueryModSecondarySkillExp( m_pSession, pSkill->GetSkillID(), iAddExp, pSkill->GetExp() );
		}
	}

	return true;
}

bool CSecondarySkillEventHandlerServer::OnChangeRecipeExp( CSecondarySkillRecipe* pRecipe, int iAddExp )
{
	_ASSERT( m_pSession->CheckDBConnection() );
	if( m_pSession->CheckDBConnection() == false )
		return false;

	// Send packet to client
	m_pSession->SendUpdateSecondarySkillRecipeExp( ERROR_NONE, pRecipe->GetSkillID(), pRecipe->GetItemID(), pRecipe->GetExp() );
	
	// DB M/W request
	m_pSession->GetDBConnection()->QueryModSecondarySkillRecipeExp( m_pSession,	pRecipe->GetItemSerial(), iAddExp, pRecipe->GetExp() );

	return true;
}

bool CSecondarySkillEventHandlerServer::OnManufacture( CSecondarySkill* pSkill, CSecondarySkillRecipe* pRecipe, bool bIsStart )
{
	int iRet = ERROR_NONE;

	_ASSERT( dynamic_cast<CManufactureSkill*>(pSkill) );

	if( bIsStart == false )
	{
		TSecondarySkillRecipeTableData* pRecipeData = g_pDataManager->GetSecondarySkillRecipeTableData( pRecipe->GetItemID() );

		int iRandVal = 0;
#if defined( _GAMESERVER )
		_srand( m_pSession->GetGameRoom(), timeGetTime() );
		iRandVal = _rand( m_pSession->GetGameRoom() ) % 100;
#else
		_srand( timeGetTime() );
		iRandVal = _rand() % 100;
#endif // #if defined( _GAMESERVER )

		if( iRandVal >= pRecipeData->iSuccessProbability )
			iRet = ERROR_SECONDARYSKILL_MANUFACTURE_PROBABILITY_FAILED;

		// Remove material items
		for( int i=0 ; i<_countof(pRecipeData->iMaterialItemID) ; ++i )
		{
			if( pRecipeData->iMaterialItemID[i] > 0 )
			{
				_ASSERT( pRecipeData->iMaterialItemCount > 0 );
				bool bRet = m_pSession->GetItem()->DeleteInventoryByItemID( pRecipeData->iMaterialItemID[i], pRecipeData->iMaterialItemCount[i], DBDNWorldDef::UseItem::Use );
				_ASSERT( bRet );
				if( bRet == false )
					return false;
			}
		}

		// Create item
		if( iRet == ERROR_NONE )
		{
			int iCreateItemID		= pRecipeData->GetManufactureItemID( pRecipe->GetExp(), pRecipe->GetMaxExp() );
			int iCreateItemCount	= pRecipeData->iSuccessCount;

			TItemData* pItemData = g_pDataManager->GetItemData( iCreateItemID );
			if( pItemData && pItemData->nType == ITEMTYPE_INSTANT )
			{
#if defined( _GAMESERVER )
				CDNGameRoom* pRoom = m_pSession->GetGameRoom();
				DnDropItemHandle hDrop = pRoom->GetItemTask()->RequestDropItem( STATIC_INSTANCE_(CDnDropItem::s_dwUniqueCount)++, *(m_pSession->GetActorHandle()->GetPosition()), iCreateItemID, 0, 1, 0 );
				_ASSERT( hDrop );
				if( !hDrop )
					return false;
#else
				_ASSERT(0);
				return false;
#endif // #if defined( _GAMESERVER )
			}
			else
			{
				int iCreateItemRet	= m_pSession->GetItem()->CreateInvenItem1( iCreateItemID, iCreateItemCount, 0, 0, DBDNWorldDef::AddMaterializedItem::ItemCompound, 0 );
				_ASSERT( iCreateItemRet == ERROR_NONE );
				if( iCreateItemRet != ERROR_NONE )
					return false;
				TItemData *pItemData = g_pDataManager->GetItemData(iCreateItemID);
				if (pItemData)
					m_pSession->GetEventSystem()->OnEvent( EventSystem::OnItemGain, 2, EventSystem::ItemType, pItemData->nType, EventSystem::ItemCount, m_pSession->GetItem()->GetInventoryItemCountByType(pItemData->nType) );
			}
		}
		
		int iPlusExp = pRecipeData->iSuccessUpExp;
		int iAddExp = iPlusExp;
		if (pSkill->GetSubType() == SecondarySkill::SubType::CookingSkill)
			CNpcReputationProcessor::CheckAndCalcUnionBenefit( m_pSession, TStoreBenefitData::CookProficiencyUp, iAddExp );

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
#if defined(_GAMESERVER)
		DnActorHandle hActor = m_pSession->GetActorHandle();

		if (hActor && 
			hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_265) && 
			pSkill->GetSubType() == SecondarySkill::SubType::CookingSkill)
		{
			float fIncRate = 0.0f;
			DNVector(DnBlowHandle) vlhBlows;
			hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_265, vlhBlows );
			int iNumBlow = (int)vlhBlows.size();
			for( int i = 0; i < iNumBlow; ++i )
			{
				fIncRate += vlhBlows[i]->GetFloatValue();
			}

			iAddExp += (int)((float)iAddExp * fIncRate);
		}
#elif defined( _VILLAGESERVER )
		float fTotalLevel = m_pSession->GetTotalLevelSkillEffect(TotalLevelSkill::Common::CookExpIncrease);
		iAddExp += (int)(iAddExp * fTotalLevel);		
#endif // _GAMESERVER
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

		// Apply secondary skill EXP
		pSkill->AddExp( iAddExp, this );
		
		// Send packet to client
		m_pSession->SendManufactureSecondarySkill( iRet, bIsStart, pRecipe->GetSkillID(), pRecipe->GetItemID() );

		// Apply recipe EXP
		pRecipe->AddExp( pRecipeData->iRecipeUpExp, this );

		// Initialize Tick
		static_cast<CManufactureSkill*>(pSkill)->ClearManufactureTick();
		// Set Flag
		static_cast<CSecondarySkillRepositoryServer*>(m_pSession->GetSecondarySkillRepository())->SetManufacturingFlag( false );
		// HeadEffect
		m_pSession->BroadcastingEffect( EffectType_Cooking_Finish, EffectState_Success );
		return true;
	}
	else
	{
		// Save Tick
		static_cast<CManufactureSkill*>(pSkill)->SetManufactureTick( timeGetTime() );
		// Clear Flag
		static_cast<CSecondarySkillRepositoryServer*>(m_pSession->GetSecondarySkillRepository())->SetManufacturingFlag( true );
		// HeadEffect
		m_pSession->BroadcastingEffect( EffectType_Cooking_Making, EffectState_Start );
	}

	// Send packet to client
	m_pSession->SendManufactureSecondarySkill( iRet, bIsStart, pRecipe->GetSkillID(), pRecipe->GetItemID() );

	return true;
}

bool CSecondarySkillEventHandlerServer::OnCancelManufacture( CManufactureSkill* pManufactureSkill, bool bSend )
{
	// Clear Flag
	static_cast<CSecondarySkillRepositoryServer*>(m_pSession->GetSecondarySkillRepository())->SetManufacturingFlag( false );
	// Initialize Tick
	pManufactureSkill->ClearManufactureTick();
	// HeadEffect
	m_pSession->BroadcastingEffect( EffectType_Cooking_Making, EffectState_Cancel );
	
	// Send packet to client
	if( bSend )
		m_pSession->SendCancleManufactureSecondarySkill( ERROR_NONE, pManufactureSkill->GetSkillID() );
	return true;
}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
