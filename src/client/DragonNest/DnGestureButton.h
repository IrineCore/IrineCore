#pragma once
#include "DnSlotButton.h"

// Originally GestureButton, later renamed to LifeSkillButton for extensibility.
// Can represent both MIInventoryItem::Gesture and MIInventoryItem::SecondarySkill types and is used commonly (collectively called LifeSkill).
// Note that custom control name still uses the old gesture button name.
class CDnLifeSkillButton : public CDnSlotButton
{
public:
	CDnLifeSkillButton( CEtUIDialog *pParent );
	virtual ~CDnLifeSkillButton(void);

protected:

	EtTextureHandle m_hShortCutIcon;
	SUICoord m_ShortCutUV;
	SUICoord m_ShortCutCoord;

	EtTextureHandle m_hGestureIcon;
	EtTextureHandle m_hGestureIconList[GESTURE_TEXTURE_COUNT];
	EtTextureHandle m_hSecondarySkillIconList[SECONDARYSKILL_TEXTURE_COUNT];

	int		m_nItemCondition;	// Whether usable
	bool	m_bCash;
#ifdef PRE_MOD_SELECT_CHAR
	bool	m_bForceRender;		// Used in character creation screen
#endif // PRE_MOD_SELECT_CHAR

public:
	void SetGestureInfo( int nGestureID );
	void SetSecondarySkillInfo( int nSecondarySkillID );
	bool IsUsable();
#ifdef PRE_MOD_SELECT_CHAR
	void SetGestureIcon( int nIconID );
#endif // PRE_MOD_SELECT_CHAR

	virtual void SetHotKey( int nKey );

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Render( float fElapsedTime );
	virtual void RenderDrag( float fElapsedTime );
	virtual void Process( float fElapsedTime );
};