
#pragma once

#include "SecondarySkill.h"

#if defined( PRE_ADD_SECONDARY_SKILL )

//========================================================================
// Common skill
//========================================================================

class CCommonSkill:public CSecondarySkill
{
public:

	CCommonSkill( const int iSkillID, SecondarySkill::ExecuteType::eType ExecuteType )
		:CSecondarySkill( iSkillID, ExecuteType )
	{
	}
	virtual ~CCommonSkill(){}

	virtual SecondarySkill::Type::eType		GetType(){ return SecondarySkill::Type::CommonSkill; }
};

//========================================================================
// Common skill - Fishing
//========================================================================

class CFishingSkill:public CCommonSkill,public TBoostMemoryPool<CFishingSkill>
{
public:

	CFishingSkill( const int iSkillID, SecondarySkill::ExecuteType::eType ExecuteType )
		:CCommonSkill( iSkillID, ExecuteType )
	{
	}
	virtual ~CFishingSkill(){}

	virtual SecondarySkill::SubType::eType	GetSubType(){ return SecondarySkill::SubType::FishingSkill; }
};

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
