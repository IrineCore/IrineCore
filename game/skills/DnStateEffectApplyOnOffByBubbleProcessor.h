#pragma once
#include "IDnSkillProcessor.h"
#include "DnSkill.h"


// Used by Alchemist's "Cocktail" skill.
// Remove unused state effect data configured on the skill based on the current bubble count
// Restore to original state when skill use finishes.
// Does not consume bubbles. If bubbles need to be consumed, a new manifestation type must be created.
class CDnStateEffectApplyOnOffByBubbleProcessor : public IDnSkillProcessor, 
												  public TBoostMemoryPool< CDnStateEffectApplyOnOffByBubbleProcessor >
{
private:
	struct S_STATE_EFFECT_ON_OFF
	{
		int iBubbleID;
		int iBubbleCount;
		vector<int> vlApplyList;

		S_STATE_EFFECT_ON_OFF( void ) : iBubbleID( 0 ), iBubbleCount( 0 ) {};
	};

	DNVector(S_STATE_EFFECT_ON_OFF) m_vlApplyOnOff;
	DNVector(CDnSkill::StateEffectStruct) m_vlSEBackup;

public:
	CDnStateEffectApplyOnOffByBubbleProcessor( DnActorHandle hActor, const char* pArg );
	virtual ~CDnStateEffectApplyOnOffByBubbleProcessor(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	void SelectAvailableSE( DNVector(CDnSkill::StateEffectStruct)& vlSEList );
	void RestoreSEList( DNVector(CDnSkill::StateEffectStruct)& vlSEList );

	int GetNumArgument( void ) { return 1; };
};
