#pragma once

#include "Task.h"
#include "MessageListener.h"

#ifdef PRE_ADD_COOKING_SYSTEM

class CSecondarySkillRecipe;
class CManufactureSkill;
class CDnItem;

class CDnLifeSkillCookingTask : public CTask, public CTaskListener, public CEtUICallback, public CSingleton<CDnLifeSkillCookingTask>
{
public:
	CDnLifeSkillCookingTask();
	virtual ~CDnLifeSkillCookingTask();

public:
	bool Initialize();
	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

public:
	CManufactureSkill* GetCookingSkill() { return m_pCookingSkill; }

	void SendAddRecipe( CDnItem* pRecipeItem );
	void SendDeleteRecipe( int nSkillID, int nItemID );
	void SendExtractRecipe( int nSkillID, int nItemID );
	void SendManufacture( int nSkillID, int nItemID, bool bStart );
	void StartManufacture( CSecondarySkillRecipe* pCookRecipe, bool bManufactureLoop );

	void OnRecvRecipeList( SecondarySkill::SCRecipeList* pPacket );
	void OnRecvAddRecipe( SecondarySkill::SCAddRecipe* pPacket );
	void OnRecvUpdateRecipeExp( SecondarySkill::SCUpdateRecipeExp* pPacket );
	void OnRecvDeleteRecipe( SecondarySkill::SCDeleteRecipe* pPacket );
	void OnRecvExtractRecipe( SecondarySkill::SCExtractRecipe* pPacket );
	void OnRecvManufacture( SecondarySkill::SCManufacture* pPacket );
	void OnRecvCancelManufacture( SecondarySkill::SCCancelManufacture* pPacket );

	void RequestAddRecipe( CDnItem* pRecipeItem );
	bool IsNowCooking() { return m_bCooking; }
	bool IsRequestCooking() { return m_bRequestCooking; }
	void CancelManufacture( bool bReleaseLockInput = true );

private:
	bool Manufacture( CSecondarySkillRecipe* pCookRecipe );
	void CheckRecieveResult();

private:
	enum { MANUFACTURE_PROGRESS = 0, MSGBOX_REQUEST_ADD_RECIPE  };
	CManufactureSkill*		m_pCookingSkill;	// Cooking skill storage
	CSecondarySkillRecipe*	m_pCookRecipe;		// Cooking recipe currently being crafted
	bool					m_bStartManufacture;// Block multiple Send processing for cooking start
	bool					m_bRequestCooking;
	bool					m_bManufactureLoop;	// Continuous crafting
	bool					m_bRecieveResult;	// When result is received, decide whether to restart after 2 seconds in Process
	bool					m_bCooking;			// Whether currently cooking
	float					m_fTimer;
	CDnItem*				m_pRequestRecipe;	// Temp pointer for recipe add-request item
};

#define GetLifeSkillCookingTask()		CDnLifeSkillCookingTask::GetInstance()

#endif // PRE_ADD_COOKING_SYSTEM
