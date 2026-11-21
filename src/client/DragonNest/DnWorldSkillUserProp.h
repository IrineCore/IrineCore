#pragma once
#include "DnWorldActProp.h"
#include "MAActorProp.h"


// Prop that uses skills... Does not use MASkillUser used on the actor side.
class CDnWorldSkillUserProp : public CDnWorldActProp,
							  public MAActorProp
{
private:
	enum
	{
		ONCE,
		CONTINUOUS,
	};

	//DnSkillHandle m_hSkill;
	//int m_iUseType;
	//float m_fCheckRadius;



protected:
	bool InitializeTable( int nTableID );

public:
	CDnWorldSkillUserProp(void);
	virtual ~CDnWorldSkillUserProp(void);
	
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnSignal( SignalTypeEnum Type, void* pPtr, LOCAL_TIME LocalTime, 
						   LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
};