#pragma once
#include "DnCustomDlg.h"
#include "DnCharPlateInfoDlg.h"


class CDnInspectPlateInfoDlg;
class CDnTooltipPlateDlg;

class CDnInspectPlateDlg : public CDnCustomDlg
{
public:
	CDnInspectPlateDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnInspectPlateDlg( void );

private:
	CDnInspectPlateInfoDlg*				m_pInspectPlateInfoDlg;
	std::vector< CDnItemSlotButton * >	m_VecGlyphSlotButton;
	std::vector< CEtUIStatic * >		m_VecGlyphCover;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );

	void RefreshGlyphEquip();

private:
	void SetGlyphEquipItem( int nEquipIndex, MIInventoryItem *pItem );
	int GLYPH_EQUIPINDEX_2_SLOTINDEX( int nEquipIndex );
};

// Inspect Plate Information Dialog
class CDnInspectPlateInfoDlg : public CDnCharPlateInfoDlg
{
public:
	CDnInspectPlateInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnInspectPlateInfoDlg( void );

public:
	virtual void Initialize( bool bShow );
	virtual void SetText( int nSessionID );
	virtual void Show( bool bShow );

protected:
	void SetEnchatInfo( CDnPlayerActor * pActor );
	void SetSkillInfo( CDnPlayerActor * pActor );
	void SetSpecialSkillInfo( CDnPlayerActor * pActor );

	void AddDescription( int nTableID );
};
