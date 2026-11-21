
#pragma once

#include "SecondarySkillRepository.h"

#if defined( PRE_ADD_SECONDARY_SKILL )

class CDNUserSession;

class CSecondarySkillRepositoryServer:public CSecondarySkillRepository,public TBoostMemoryPool<CSecondarySkillRepositoryServer>
{
public:

	CSecondarySkillRepositoryServer( CDNUserSession* pSession );

	int					AddRecipe( SecondarySkill::CSAddRecipe* pPacket );			// Add recipe using info received from client
	bool				AddRecipe( TSecondarySkillRecipe* pRecipeData );			// Add recipe using info received from DB
	bool				DeleteRecipe( SecondarySkill::CSDeleteRecipe* pPacket );	// Remove recipe using info received from client
	bool				ExtractRecipe( SecondarySkill::CSExtractRecipe* pPacket );	// Extract recipe using info received from client
	int					DoManufacture( SecondarySkill::CSManufacture* pPacket );	// Craft using info received from client
	bool				CancelManufacture( bool bSend=true);						// Cancel crafting
	CSecondarySkill*	GetManufacturingSkill();									// Get pointer to the skill currently being created
	
	void				SetManufacturingFlag( bool bFlag ){ m_bManufacturing = bFlag; }
	bool				bIsManufacturing(){ return m_bManufacturing; }

	virtual void		SendList();
	
	virtual int			CanCreate( CSecondarySkill* pCreateSkill );
	virtual int			CanAddRecipe( CSecondarySkill* pSkill, CSecondarySkillRecipe* pRecipe );
	virtual int			CanDeleteRecipe( CSecondarySkill* pSkill, int iItemID, CSecondarySkillRecipe*& pRecipe );
	virtual int			CanExtractRecipe( CSecondarySkill* pSkill, int iItemID, CSecondarySkillRecipe*& pRecipe );
	virtual int			CanManufacture( CSecondarySkill* pSkill, bool bIsStart, int iItemID, CSecondarySkillRecipe*& pRecipe );
	virtual int			CanCancelManufacture( CManufactureSkill*& pManufacturingSkill );

protected:

	void _SendRecipeList();

	CDNUserSession* m_pSession;
	bool			m_bManufacturing;
};

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
