
#include "stdafx.h"
#include "SecondarySkillFactory.h"
#include "ManufactureSkill.h"
#include "ProductionSkill.h"
#include "CommonSkill.h"
#include "SecondarySkillRecipe.h"
#if defined( _SERVER )
#include "DNGameDataManager.h"
#else // _SERVER
#include "DnTableDB.h"
#include "DNTableFile.h"
#endif // _SERVER

#if defined( PRE_ADD_SECONDARY_SKILL )

CSecondarySkill* CSecondarySkillFactory::CreateSkill( const int iSecondarySkillID )
{
	SecondarySkill::Type::eType			Type;
	SecondarySkill::SubType::eType		SubType;
	SecondarySkill::ExecuteType::eType	ExecuteType;

#if defined( _SERVER )
	TSecondarySkillTableData* pSecondarySkillTable = g_pDataManager->GetSecondarySkillTableData( iSecondarySkillID );
	if( pSecondarySkillTable == NULL )
	{
		DN_ASSERT( 0, "Invalid SecondarySkillType" );
		return NULL;
	}

	Type				= pSecondarySkillTable->Type;
	SubType				= pSecondarySkillTable->SubType;
	ExecuteType			= pSecondarySkillTable->ExecuteType;
#else 
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSecondarySkill );
	if( pSox == NULL )
	{
		DN_ASSERT( 0, "Invalid SecondarySkillType" );
		return NULL;
	}
	Type = (SecondarySkill::Type::eType)pSox->GetFieldFromLablePtr( iSecondarySkillID, "_SecondarySkillKind" )->GetInteger();
	SubType = (SecondarySkill::SubType::eType)pSox->GetFieldFromLablePtr( iSecondarySkillID, "_SecondarySkillType" )->GetInteger();
	ExecuteType = (SecondarySkill::ExecuteType::eType)pSox->GetFieldFromLablePtr( iSecondarySkillID, "_SecondarySkillCharacter" )->GetInteger();
#endif // #if defined( _SERVER )

	CSecondarySkill* pCreateSKill = NULL;

	switch( SubType )
	{
		case SecondarySkill::SubType::CookingSkill:
		{
			_ASSERT( Type == SecondarySkill::Type::ManufactureSkill );
			pCreateSKill = new CCookingSkill( iSecondarySkillID, ExecuteType );
			break;
		}
		case SecondarySkill::SubType::CultivationSkill:
		{
			_ASSERT( Type == SecondarySkill::Type::ProductionSkill );
			pCreateSKill = new CCultivationSkill( iSecondarySkillID, ExecuteType );
			break;
		}
		case SecondarySkill::SubType::FishingSkill:
		{
			_ASSERT( Type == SecondarySkill::Type::CommonSkill );
			pCreateSKill = new CFishingSkill( iSecondarySkillID, ExecuteType );
			break;
		}
	}

	if( pCreateSKill == NULL )
	{
		_ASSERT( 0 );
	}

	return pCreateSKill;
}

CSecondarySkillRecipe* CSecondarySkillFactory::CreateRecipe( int iSkillID, int iItemID, INT64 biItemSerial, USHORT nDurability )
{
	CSecondarySkillRecipe* pRecipe = new CSecondarySkillRecipe( iSkillID, iItemID, biItemSerial, nDurability );
	if( pRecipe == NULL )
	{
		_ASSERT(0);
	}

	return pRecipe;
}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
