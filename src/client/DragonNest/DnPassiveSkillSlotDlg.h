#pragma once

#include "DnCustomDlg.h"
#include "DnCustomControlCommon.h"

class CDnSlotButton;
class CDnQuickSlotButton;
class MIInventoryItem;

class CDnPassiveSkillSlotDlg : public CDnCustomDlg
{
public:
	CDnPassiveSkillSlotDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPassiveSkillSlotDlg(void);

protected:
	std::vector< CDnQuickSlotButton* > m_vecSlotButton;
	std::vector< int > m_vecCreateOrder;

	int		m_nCreateOrder;	
	float	m_fGap;

	typedef CDnCustomDlg BaseClass;

public:	
	void AddPassiveSkill( DnSkillHandle hSkill );
	bool IsExistSkill(int Skill_ID); // Rotha - Check if the skill is currently running.
	void ResetSlotFromSkillClassID(int Skill_ID);

#if defined(PRE_FIX_61821)
	void ReplacePassiveSkill(DnSkillHandle hSkill);
#endif // PRE_FIX_61821

public:
	virtual void Initialize(bool bShow);
	virtual void InitCustomControl( CEtUIControl *pControl );	
	virtual void Process( float fElapsedTime );
};
