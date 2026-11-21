
#pragma once

#include "DnDamageBase.h"
#include "DnActor.h"

class CDnPlayerActor;

class CDNAggroSystem
{
public:
	
	struct AggroStruct 
	{
		DnActorHandle hActor;
		int	iAggro;
		int	iModifierAggro;
		float fDecreaseDelta;	

		AggroStruct( DnActorHandle hAggroActor, int iDefaultAggro ) 
		{
			hActor = hAggroActor;
			iAggro = iModifierAggro = iDefaultAggro;
			fDecreaseDelta = static_cast<float>(eDecreaseDeltaTick/1000.f);
		}
	};

	CDNAggroSystem( DnActorHandle hActor );
	virtual ~CDNAggroSystem();

	void ResetAggro();
	void ResetAggro( DnActorHandle hActor );
	void AddAggro( DnActorHandle hActor, int iAggroValue, bool bForce=false );
	void SetIgnoreFlag( bool bFlag ){ m_bIgnore = bFlag; }
	AggroStruct* GetAggroStruct( DnActorHandle hActor );
	UINT GetAggroSize(){ return static_cast<UINT>(m_AggroList.size()); }
	void Convert( DNVector(DnActorHandle)& vOutput );
	int	GetAggroSum() const;
	std::list<AggroStruct>& GetAggroList() { return m_AggroList; }
	
	virtual void OnProcessAggro( const LOCAL_TIME LocalTime, const float fDelta ) = 0;
	virtual DnActorHandle OnGetAggroTarget( bool& bIsProvocationTarget, DnActorHandle hExceptActor=CDnActor::Identity(), DNVector(DnActorHandle)* vTarget = NULL) = 0;
	virtual void OnDamageAggro( DnActorHandle hActor, CDnDamageBase::SHitParam& HitParam, int iDamage ) = 0;
	virtual void OnStateBlowAggro( DnBlowHandle hBlow ) = 0;
	virtual bool bOnCheckPlayerBeginStateBlow( CDnPlayerActor* pPlayer ) = 0;
	virtual void OnAggroRegulation( DnActorHandle hActor, int& iAggroValue ){}

protected:

	enum
	{
		eDecreaseDeltaTick = 5000,
	};
	
	std::list<AggroStruct> m_AggroList;
	DnActorHandle m_hActor;
	bool m_bIgnore;
};
