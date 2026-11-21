
#pragma once

#include "DNAggroSystem.h"
#include "DNAggroRange.h"

class CDNMonsterAggroSystem : public CDNAggroSystem, public TBoostMemoryPool<CDNMonsterAggroSystem>
{
public:

	CDNMonsterAggroSystem( DnActorHandle hActor );
	virtual ~CDNMonsterAggroSystem();

	virtual void			OnProcessAggro( const LOCAL_TIME LocalTime, const float fDelta );
	virtual DnActorHandle	OnGetAggroTarget( bool& bIsProvocationTarget, DnActorHandle hExceptActor=CDnActor::Identity(), DNVector(DnActorHandle)* vTarget = NULL);
	virtual void			OnDamageAggro( DnActorHandle hActor, CDnDamageBase::SHitParam& HitParam, int iDamage );
	virtual void			OnStateBlowAggro( DnBlowHandle hBlow );
	virtual bool			bOnCheckPlayerBeginStateBlow( CDnPlayerActor* pPlayer );
	virtual void			OnAggroRegulation( DnActorHandle hActor, int& iAggroValue );

	float					GetThreatRange(){ return m_AggroRange.GetThreatRange(); }
	float					GetCognizanceThreatRange(){ return m_AggroRange.GetCognizanceThreatRange(); }
	float					GetCognizanceThreatRangeSq(){ return m_AggroRange.GetCognizanceThreatRangeSq(); }
	float					GetCognizanceGentleRangeSq(){ return m_AggroRange.GetCognizanceGentleRangeSq(); }

	enum eCommon
	{
		AIProcessFrame = 5,
	};
	bool					bIsInit(){ return m_bInit; }

private:

	void	_Create();
	int		_CalcAdditionalAggro( const int iAggro, const float fValue );

	CDnMonsterActor*	m_pMonsterActor;
	int					m_iThreatAggro;
	CDNAggroRange		m_AggroRange;
	float				m_fDecreaseAggroPer;
	float				m_fRangeAttackAdditionalAggroPer;
	float				m_fStunStateAdditionalAggroPer;
	float				m_fDownStateAdditionalAggroPer;
	float				m_fCantMoveStateAdditionalAggroPer;
	float				m_fSleepStateAdditionalAggroPer;
	float				m_fUnderHPStateAdditionalAggroPer;

	UINT				m_uiFrameCount;
	bool				m_bInit;

	UINT				m_uiLastTopAggro;		// Previous max aggro value
};
