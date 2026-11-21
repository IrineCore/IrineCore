#pragma once
#include "DnDamageBase.h"
#include "DnActor.h"
#include "DnPvPGameTask.h"

class CDnMutatorTeamGame:public CDnMutatorGame
{
public:
	CDnMutatorTeamGame( CDnPvPGameTask * pGameTask );
	virtual ~CDnMutatorTeamGame();

protected:
	int		m_nTeam; //My team
	float   m_fCountDownTime;
	
	bool	m_bIsTeamHairColorMode;	// Set hair according to team.
	bool    m_bIsHairColorMode;
	bool    m_bIsIndividualMode;

public:
	virtual void ProcessEnterUser( DnActorHandle hActor , bool isIngameJoin = false, void * pData = NULL );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void RestartGame();

	void SetHairColorMode(bool bTrue){m_bIsHairColorMode = bTrue;}; // Individual mode does not need head color distinction by ally/enemy team.
	void SetIndividualMode(bool bTrue){m_bIsIndividualMode = bTrue;}
	bool IsIndividualMode(){return m_bIsIndividualMode;} // Is it individual mode or not? 

};