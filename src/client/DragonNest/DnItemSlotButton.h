#pragma once
#include "DnSlotButton.h"
#include "DnCustomControlCommon.h"

#ifdef PRE_ADD_EQUIPLOCK
class CDnItem;
#endif
class CDnItemSlotButton : public CDnSlotButton
{
	enum
	{
		typeItemOutlineD,
		typeSymbolOutline,
		typeItemDisable,
		typeItemNumber,
		typeItemNotWear,
		typeItemWearable,
		typeItemWearableArea,
		typeItemOutlineC,
		typeItemOutlineB,
		typeItemOutlineA,
		typeItemOutlineS,
		typeItemOutlineR,
		typeRepair01,
		typeRepair02,
		typeRepair03,
		typeMiscItemOutlineC,	// Misc item border.
		typeMiscItemOutlineB,
		typeMiscItemOutlineA,
		typeMiscItemOutlineS,
		typeMiscItemOutlineR,
		typeMiscItemOutlineN,
		typeDungeon,
		typeItemOutlineCash,
		typeItemCashMark,
		typeItemButton_Closed,
		typeItemButton_Seal,
		typeItemOutlineLG,
		typeItemOutlineLock,
		typeItemOutlineUnlocking,
	};

public:
	CDnItemSlotButton( CEtUIDialog *pParent );
	virtual ~CDnItemSlotButton(void);

protected:
	bool m_bOutlineRender;
	bool m_bMagnetic;
	bool m_bWearable;
	bool m_bBlankItem;
	bool m_bClosed;

	int m_nTypeOutline;

	int m_nNeedCount;
	int m_nCurCount;
	float m_fImageAlpha;
	void RenderJewelCount( SUIElement *pElement );

public:
	void SetOutlineRender( bool bRender )	{ m_bOutlineRender = bRender; }
	void SetMagnetic( bool bMagnetic )		{ m_bMagnetic = bMagnetic; }
	void SetWearable( bool bWearable )		{ m_bWearable = bWearable; }
	void SetClosed( bool bClosed )			{ m_bClosed = bClosed; }

	void SetJewelCount( int nNeedCount, int nCurCount, bool bCorrection = true );

public:
	virtual void	SetItem(MIInventoryItem *pItem, int count);
	void			SetBlankItem( bool bEnable = true );

public:
	virtual void	Initialize( SUIControlProperty *pProperty );
	virtual void	Render( float fElapsedTime );
#ifdef PRE_ADD_EQUIPLOCK
	void RenderLock(CDnItem* item);
#endif
	virtual void	RenderDrag( float fElapsedTime );

	void			DrawBasicItemSlot();
	void			SetImageAlpha( float fAlpha ) {m_fImageAlpha = fAlpha;}
	float			GetImageAlpha() {return m_fImageAlpha;}	
	virtual void	Enable( bool bEnable ) override;
};
