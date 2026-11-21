#pragma once
#include "DnCustomDlg.h"
#include "DNTableFile.h"

class CDnSkillTreeDlg;
class CSecondarySkill;

struct stLifeSkillInfoData;

class CDnLifeSkillContent
{
public:
	CDnLifeSkillContent();
	virtual ~CDnLifeSkillContent();

public:
	void RefreshInfo();											// Update secondary skill info
	void AddLifeSkill( CSecondarySkill* pAddSecondarySkill );	// Add secondary skill
	void SetSkillTreeDlg( CDnSkillTreeDlg* pSkillTreeDlg ) { m_pSkillTreeDlg = pSkillTreeDlg; };

private:
	void SetCustomSkillFunction( DNTableFileFormat* pSoxSkill, stLifeSkillInfoData* pLifeSkillInfoData );

private:
	CDnSkillTreeDlg* m_pSkillTreeDlg;
};
