#pragma once

#include "ICustomActorProcessor.h"

class CDnCutSceneActorProcessor : public ICustomActorProcessor
{
private:
	vector<float>				m_vlPrevFrame;
	vector<void*>				m_vlpPrevActionSeq;

public:
	CDnCutSceneActorProcessor(void);
	virtual ~CDnCutSceneActorProcessor(void);

	// from ICustomActorProcessor
	void AddObjectToPlay( S_CS_ACTOR_INFO* pNewObject );
	void OnChangeAction( S_CS_ACTOR_INFO* pActionChangeActor );
	void CalcAniDistance(  S_CS_ACTOR_INFO* pActorInfo, float fCurFrame, float fPrevFrame, EtVector3& DistVec );
	void SetPropAction( CEtWorldProp* pProp, const char* pActionName, LOCAL_TIME AbsoluteTime, float fFrame );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	int GetActionIndex( int iActorIndex, const char* pActionName );
	float GetActorScale( int iActorIndex );
	void ShowProp( CEtWorldProp* pProp, bool bShow );

	void FadeIn( float fSpeed, DWORD dwColor );
	void FadeOut( float fSpeed, DWORD dwColor );

	void ShowSubtitle( const wchar_t* pSubtitle );
	void HideSubtitle( void );

	virtual void ShowEffect( void * pActor, bool bShow ); // #54681 - Hide handling for player character.
};
