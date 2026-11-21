#pragma once

#include "DnCustomDlg.h"


class CDnPetOptionDlg;
class CDnCharPetPreviewDlg;
class CDnCharPetInfoDlg;
class CDnItem;

class CDnCharPetDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnCharPetDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharPetDlg();

	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );
	virtual void Render( float fElapsedTime );

public:
	void SetPetEquipItem( CDnItem* pItem );
	void RefreshPetEquip();
	void ResetPetEquipSlot( int nEquipIndex );
	void SetAvatarAction( const char* szActionName );
	void RefreshPetEquipPreview();
	void PetSummonedOff();
	void SetPetInfoDetail();
	void SetEnableMode( bool bEnable );
	CDnPetOptionDlg* GetPetOptionDlg() { return m_pPetOptionDlg; }
	void SetSatietyInfo();
	void DisablePetSkill( int nSkillNum );
	void EnablePetSkill( int nSkillNum );

private:
	void RemovePetInfoDetail();

public:
	enum PetSlotIndex
	{
		PetSlot = 0,
		PetPartsSlot1 = 1,
		PetPartsSlot2 = 2
	};

	enum
	{
		MAX_PET_SKILL = 2,
	};

protected:
	CDnItemSlotButton*	m_pPetSlot;
	CDnItemSlotButton*	m_pPetPartsSlot1;
	CDnItemSlotButton*	m_pPetPartsSlot2;
	CEtUIButton*		m_pCancelButton;
	CEtUIStatic*		m_pStaticPetName;
	CEtUIStatic*		m_pStaticPetLevel;
	CEtUIStatic*		m_pStaticPetExpireDate;
	CEtUIProgressBar*	m_pProgressBarPetExp;
	CEtUIStatic*		m_pStaticPetExp;
	CEtUIProgressBar*	m_pProgressBarPetSatiety;
	CEtUIStatic*		m_pStaticPetSatiety;
	DnSkillHandle		m_hSkill1;
	DnSkillHandle		m_hSkill2;

	CDnPetOptionDlg*		m_pPetOptionDlg;
	CDnCharPetPreviewDlg*	m_pPreviewDlg;
	CDnCharPetInfoDlg*		m_pPetInfoDlg;
	int						m_nPetLevelTableID;
	CDnSkillSlotButton*		m_pSkillSlotButton[MAX_PET_SKILL];
	CEtUIStatic*			m_pBackgroundSkillSlot[MAX_PET_SKILL];
};
