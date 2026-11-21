#pragma once
#include "IDnSkillProcessor.h"
class CDnProjectile;


// Processor for skills using weapons like arrows
class CDnChangeActionStrProcessor : public IDnSkillProcessor, public TBoostMemoryPool< CDnChangeActionStrProcessor >
{
protected:
//	std::string m_szActionStr;
//	std::string m_szAdditionalStr;
	map<string, string> m_szMapMatchStr;
	set<string> m_setChangeActionNames;
protected:

public:
	CDnChangeActionStrProcessor( void ) { m_iType = CHANGE_ACTIONSTR; }; // Constructor created for temp skill backup. Initialize essential variables.
	CDnChangeActionStrProcessor( DnActorHandle hActor, const char *szAdditionalStr, DNVector(std::string) &szActionStr );
	virtual ~CDnChangeActionStrProcessor(void);

	// from IDnSkillProcessor.h
	virtual void SetHasActor( DnActorHandle hActor );

	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	bool IsFinished( void );
	int GetNumArgument( void ) { return (int)m_szMapMatchStr.size()+1; }

	const char *GetChangeActionName( const string& strNowActionName, bool* pBChanged = NULL );
	bool IsChangedActionName( const char* pActionName );

	void CopyFrom( IDnSkillProcessor* pProcessor );
	map<string, string>& GetMachStringList() { return m_szMapMatchStr; }
	set<string>& GetChangeActionNameList() { return m_setChangeActionNames; }
};
