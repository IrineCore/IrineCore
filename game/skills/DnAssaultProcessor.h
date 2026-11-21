#pragma once
#include "IDnSkillProcessor.h"
#include "SignalHeader.h"

// Skill processor index 15 (1-based table 16): Assault movement & hit processor
class CDnAssaultProcessor : public IDnSkillProcessor, public TBoostMemoryPool< CDnAssaultProcessor >
{
protected:
	int m_nHitInterval;
	int m_nHitLimitCount;
	int m_nExtraParam;
	int m_nTime;
	float m_fElapsedTime;
	float m_fMoveSpeed;

	HitStruct m_HitStruct;
	EtVector3 m_vOffset;

	std::string m_strStartActionName;
	std::string m_strLoopActionName;
	std::string m_strHitActionName;
	std::string m_strEndActionName;

	bool IsActionMatch( const std::string& strTarget, const char* szAction ) const;

public:
	CDnAssaultProcessor( void );
	CDnAssaultProcessor( DnActorHandle hActor,
						 const char* pStartActionName,
						 const char* pLoopActionName,
						 const char* pEndActionName,
						 int iTime,
						 float fMoveSpeed,
						 const char* pHitActionName,
						 const char* pHitParam,
						 const char* pHitAreaParam );
	virtual ~CDnAssaultProcessor( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	virtual bool IsFinished( void );
	virtual int GetNumArgument( void ) { return 8; }
	virtual void CopyFrom( IDnSkillProcessor* pProcessor );
};
