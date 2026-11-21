
#pragma once

#if defined( PRE_ADD_SECONDARY_SKILL )

class ISecondarySkillEventHandler;

class CSecondarySkill
{
public:

	CSecondarySkill( int iSkillID, SecondarySkill::ExecuteType::eType ExecuteType );
	virtual ~CSecondarySkill(){}

	int										GetSkillID(){ return m_iSkillID; }
	SecondarySkill::ExecuteType::eType		GetExecuteType(){ return m_ExecuteType; }
	SecondarySkill::Grade::eType			GetGrade(){ return m_Grade; }
	int										GetLevel(){ return m_iLevel; }
	int										GetExp(){ return m_iExp; }

	void									SetExp( const int iExp, ISecondarySkillEventHandler* pEventHandler );
	void									AddExp( const int iExp, ISecondarySkillEventHandler* pEventHandler );
	void									SetGrade( SecondarySkill::Grade::eType Grade ){ m_Grade = Grade; }
	void									SetLevel( int iLevel ){ m_iLevel = iLevel; }
	
	virtual SecondarySkill::Type::eType		GetType() = 0;
	virtual SecondarySkill::SubType::eType	GetSubType() = 0;
	
	
protected:

	const int							m_iSkillID;		// Skill ID
	SecondarySkill::ExecuteType::eType	m_ExecuteType;	// Execution type
	SecondarySkill::Grade::eType		m_Grade;		// Grade
	int									m_iLevel;		// Level
	int									m_iExp;			// EXP
};

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
