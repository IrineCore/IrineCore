#pragma once
#include "DnBlow.h"

// Skill effect ID: 100
// Displays graphic effect, attaches common effect to Bip01, or plays action signals on target.
class CDnEffectBlow : public CDnBlow, public TBoostMemoryPool< CDnEffectBlow >
{
protected:
	int m_nEffectType;
	std::string m_strEffectName;
#ifndef _GAMESERVER
	MatrixEx m_matEx;
	CEtActionBase::ActionElementStruct* m_pActionElement;
	std::vector<CEtActionSignal*> m_vlpSignalList;
	float m_fFrame;
#endif

public:
	CDnEffectBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnEffectBlow( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifndef _GAMESERVER
	virtual void SignalProcess( LOCAL_TIME LocalTime, float fDelta );
#endif
};
