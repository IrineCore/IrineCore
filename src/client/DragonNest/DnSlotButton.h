#pragma once
#include "etuibutton.h"
#include "DnCustomControlCommon.h"
#include "MIInventoryItem.h"
#include "SmartPtr.h"

class CDnSlotButton : public CEtUIButton , public AcquireOwnerShip, public CSmartPtrBase<CDnSlotButton>
{
public:
	enum eSlotBtnConst
	{
		INVALID_SPLIT_COUNT		= 0,
		NO_COUNTABLE_RENDER		= 0,
		ITEM_ORIGINAL_COUNT		= -1
	};

	CDnSlotButton( CEtUIDialog *pParent );
	virtual ~CDnSlotButton(void);

protected:
	SUICoord		m_IconUV;
	SUICoord		m_CursorCoord;

	ITEMSLOT_STATE	m_ButtonState;
	ITEM_SLOT_TYPE	m_SlotType;

	int m_nIconIndex;
	int m_nSlotIndex;

	int m_nItemID;

	MIInventoryItem *m_pItem;
	MIInventoryItem::InvenItemTypeEnum m_itemType;

	EtTextureHandle m_hSkillIcon;
	EtTextureHandle m_hSkillGrayIcon;
	EtTextureHandle m_hItemIcon;

	enum eIconTextureType
	{
		eItemIconTexture = 0,
		eSkillIconTexture = 1,
		eSkillGrayIconTexture = 2,
	};
	struct sOrigIconTextureInfo
	{
		int nType;
		int nTextureIndex;
		EtTextureHandle hOrigTexture;
		float fDelta;
	};
	static std::vector<sOrigIconTextureInfo> s_vecOrigIconTextureHandle;
	struct sIconTextureInfo
	{
		int nType;
		int nIconIndex;
		EtTextureHandle hIconTexture;
	};
	static std::vector<sIconTextureInfo> s_vecIconTextureHandle;

	bool		m_bRenderCount;
	int			m_nCountForRender;
	int			m_nSplitCount;
	int			m_nExpireCount;
	DWORD		m_dwIconColor;
	int			m_nTooltipIndexBack;
	DWORD		m_dwFontColor; // Font color. When SlotType is ST_SET_FONTCOLOR.

	// Becomes true when an item registration action occurs, such as trade window or disassembly.
	bool		m_bRegist;
	bool		m_bCountMoreThanOne;
	bool		m_bShowTooltip;

protected:
	void		ShowTooltip( float fX, float fY );
	void		RenderCount(  SUIElement *pElement );
	void		RenderExpire( SUIElement *pElement );
	bool		IsRenderCount();
	bool		IsRenderDragCount() const;
	void		RenderDragCount(SUIElement *pElement, SUICoord &Coord);

	static EtTextureHandle GetIconTexture( eIconTextureType eType, int nIconIndex );

public:
	void		EnableSplitMode(int splitCount);
	void		DisableSplitMode(bool bCancel);

 	bool		IsDragMode() const;
	bool		IsSplitting() const;
	int			GetRenderCount() const		{ return m_nCountForRender; }
	void		SetRenderCount( int nCountForRender ) { m_nCountForRender = nCountForRender; }

	void					SetSlotIndex( int nIndex ) { m_nSlotIndex = nIndex; }
	int						GetSlotIndex() const		{ return m_nSlotIndex; }

	void					SetIconColor( DWORD dwColor ) { m_dwIconColor = dwColor; }
	void					SetFontColor( DWORD dwColor ) { m_dwFontColor = dwColor; }

	virtual void			SetItem(MIInventoryItem *pItem, int count);
	MIInventoryItem*		GetItem() { return m_pItem; }
	const MIInventoryItem*	GetItem() const { return m_pItem; }

	int						GetIconIndex()				{ return m_nIconIndex; }
	int						GetItemID()	const			{ return m_nItemID; }
	void					SetItemID( int nItemID )	{ m_nItemID = nItemID; }

	ITEMSLOT_STATE			GetItemState()								{ return m_ButtonState; }
	void					SetItemState( ITEMSLOT_STATE ButtonState )	{ m_ButtonState = ButtonState; }

	void					SetSlotType( ITEM_SLOT_TYPE SlotType )	{ m_SlotType = SlotType; }
	ITEM_SLOT_TYPE			GetSlotType()							{ return m_SlotType; }

	void							SetItemType( MIInventoryItem::InvenItemTypeEnum emItemtype )	{ m_itemType = emItemtype; }
	MIInventoryItem::InvenItemTypeEnum	GetItemType() const					{ return m_itemType; }
	virtual void					ResetSlot();

	bool	IsEmptySlot() const { return (m_pItem==NULL); }

	void	OnRefreshTooltip();
	void	SetCursorCoord( const SUICoord &uiCoord ) { m_CursorCoord = uiCoord; }

	void	SetRenderCount( bool bRender ) { m_bRenderCount = bRender; }

	void	SetRegist( bool bRegist ) { m_bRegist = bRegist; }
	bool	IsRegist() { return m_bRegist; }

	void	SetExpire( int nExpireCount )	{ m_nExpireCount = nExpireCount; }
	int		GetExpire()	{ return m_nExpireCount; }

	void	OnCmdDrag(UIDragCmdParam param);

	static void ProcessMemoryOptimize( float fElapsedTime );
	static void ReleaseIconTexture();
	static EtTextureHandle GetItemIcon( int nIconIndex, SUICoord &UVCoord );
	static bool GetSkillIcon( int nIconIndex, SUICoord &UVCoord, EtTextureHandle &hSkillTexture, EtTextureHandle &hSkillGrayTexture );

	void	SetShowTooltip(bool bSet)	{ m_bShowTooltip = bSet; }
	int		GetSlotOriginalCount() const;

	void	RefreshIcon(const MIInventoryItem& item);

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Render( float fElapsedTime );
	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
	virtual void ReleaseOwnerShip();
};
