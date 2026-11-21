
#include "stdafx.h"
#include "SecondarySkillRepository.h"
#include "SecondarySkill.h"
#include "ManufactureSkill.h"
#include "SecondarySkillFactory.h"
#include "SecondarySkillEventHandler.h"
#include "SecondarySkillRecipe.h"

#if defined( PRE_ADD_SECONDARY_SKILL )

CSecondarySkillRepository::CSecondarySkillRepository() : m_pEventHandler( NULL )
{
}

CSecondarySkillRepository::~CSecondarySkillRepository()
{
	for( std::map<int,CSecondarySkill*>::iterator itor=m_mSecondarySkill.begin() ; itor!=m_mSecondarySkill.end() ; ++itor )
		delete (*itor).second;

	SAFE_DELETE( m_pEventHandler );
}

int	CSecondarySkillRepository::GetCount( SecondarySkill::Type::eType Type )
{
	int iCount = 0;

	for( std::map<int,CSecondarySkill*>::iterator itor=m_mSecondarySkill.begin() ; itor!=m_mSecondarySkill.end() ; ++itor )
	{
		if( (*itor).second->GetType() == Type )
			++iCount;
	}

	return iCount;
}

CSecondarySkill* CSecondarySkillRepository::Get( const int iSecondarySkillID )
{
	std::map<int,CSecondarySkill*>::iterator itor = m_mSecondarySkill.find( iSecondarySkillID );
	if( itor != m_mSecondarySkill.end() )
		return (*itor).second;

	return NULL;
}

CSecondarySkill* CSecondarySkillRepository::Get( SecondarySkill::SubType::eType Type )
{
	for( std::map<int,CSecondarySkill*>::iterator itor=m_mSecondarySkill.begin() ; itor!=m_mSecondarySkill.end() ; ++itor )
	{
		if( (*itor).second->GetSubType() == Type )
			return (*itor).second;
	}

	return NULL;
}

CManufactureSkill* CSecondarySkillRepository::GetManufactureSkill( const int iSecondarySkillID )
{
	CSecondarySkill* pSkill = Get( iSecondarySkillID );
	if( pSkill == NULL )
		return NULL;

	if( pSkill->GetType() != SecondarySkill::Type::ManufactureSkill )
		return NULL;

	return static_cast<CManufactureSkill*>(pSkill);
}

// Create and add secondary skill
bool CSecondarySkillRepository::Create( const int iSecondarySkillID )
{
	// Create skill
	CSecondarySkill* pCreateSkill = CSecondarySkillFactory::CreateSkill( iSecondarySkillID );
	if( pCreateSkill == NULL )
		return false;

	if( CanCreate( pCreateSkill ) != ERROR_NONE )
	{
		delete pCreateSkill;
		return false;
	}

	m_mSecondarySkill.insert( std::make_pair(iSecondarySkillID,pCreateSkill) );
#if defined( _SERVER )
	bool bRet = m_pEventHandler->OnCreate( pCreateSkill );
	_ASSERT( bRet == true );
#endif // _SERVER

	return true;
}

bool CSecondarySkillRepository::Delete( const int iSecondarySkillID )
{
	CSecondarySkill* pSkill = Get( iSecondarySkillID );
	if( pSkill == NULL )
		return false;

	// Check crafting skill condition
	if( pSkill->GetType() == SecondarySkill::Type::ManufactureSkill )
	{
		CManufactureSkill* pManufactureSkill = static_cast<CManufactureSkill*>(pSkill);

		// Check owned recipes
		if( pManufactureSkill->GetRecipeCount() > 0 )
			return false;
	}

	m_mSecondarySkill.erase( pSkill->GetSkillID() );
#if defined( _SERVER )
	bool bRet = m_pEventHandler->OnDelete( pSkill );
	_ASSERT( bRet == true );
#endif // _SERVER
	SAFE_DELETE( pSkill );

	return true;
}

// Add existing sub-skill
bool CSecondarySkillRepository::Add( const int iSecondarySkillID, const int iExp )
{
	// Create skill
	CSecondarySkill* pAddSkill = CSecondarySkillFactory::CreateSkill( iSecondarySkillID );
	if( pAddSkill == NULL )
		return false;

	// Duplicate check
	if( Get( iSecondarySkillID ) )
	{
		delete pAddSkill;
		return false;
	}

	// Set Exp
	pAddSkill->SetExp( iExp, m_pEventHandler );

	m_mSecondarySkill.insert( std::make_pair(iSecondarySkillID,pAddSkill) );
	return true;
}

// Increase secondary skill EXP
bool CSecondarySkillRepository::AddExp( const int iSecondarySkillID, const int iExp )
{
	CSecondarySkill* pSkill = Get( iSecondarySkillID );
	if( pSkill == NULL )
		return false;

	pSkill->AddExp( iExp, m_pEventHandler );
	return true;
}

bool CSecondarySkillRepository::AddExp( SecondarySkill::SubType::eType Type, const int iExp )
{
	CSecondarySkill* pSkill = Get( Type );
	if( pSkill == NULL )
		return false;
	pSkill->AddExp( iExp, m_pEventHandler );
	return true;
}

// Increase recipe EXP
bool CSecondarySkillRepository::AddRecipeExp( const int iSecondarySkillID, const int iItemID, const int iExp )
{
	CSecondarySkill* pSkill = Get( iSecondarySkillID );
	if( pSkill == NULL )
		return false;

	if( pSkill->GetType() != SecondarySkill::Type::ManufactureSkill )
		return false;

	CManufactureSkill* pManufactureSkill = static_cast<CManufactureSkill*>(pSkill);

	CSecondarySkillRecipe* pRecipe = pManufactureSkill->GetRecipe( iItemID );
	if( pRecipe == NULL )
		return false;

	pRecipe->AddExp( iExp, m_pEventHandler );
	return true;
}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
