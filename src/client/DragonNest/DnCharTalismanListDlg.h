#pragma once

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnCustomDlg.h"
#include "DnCharStatusDlg.h"

class CDnTooltipTalismanDlg;
class CDnCharTalismanDlg;

class CDnCharTalismanListDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	struct STalismanSlotInfo
	{
		int nTableRowID;
		bool bService;
		int nLevel;
		int nAmount;
		int nNeedItemID;
		int nNeedItemCount;
		float fEfficiency;
		int nDescriptionID;
		bool bOpen;
		int nSlotType;
		std::string szTexture;

		CEtUIStatic *pStaticSlot;
		CDnItemSlotButton *pSlotButton;
		CEtUIStatic *pStaticEfficiency;
		CEtUIStatic *pStaticCover;
		CEtUIStatic *pStaticCashCover;
		CEtUIStatic *pStaticTime;

		STalismanSlotInfo()
		: nTableRowID( -1 ), bService( false ), nLevel( 0 ), nAmount( 0 )
		, nNeedItemID( 0 ), nNeedItemCount( 0 ), fEfficiency( 1.0f )
		, nDescriptionID( 0 ), bOpen( false ), nSlotType( TALISMAN_BASIC )
		, pStaticSlot( NULL ), pSlotButton( NULL ), pStaticEfficiency( NULL )
		, pStaticCover( NULL ), pStaticCashCover( NULL ), pStaticTime( NULL )
		{}
	};

	enum {
		TALISMAN_OPENSLOT_MSG_ID = 8360,
		TALISMAN_DETACH_MSG_ID   = 8361,
		TALISMAN_UI_SLOT_MAX     = 48,
	};

public:
	static const float TALISMAN_LIST_TOP_PADDING;

public:
	CDnCharTalismanListDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharTalismanListDlg();

protected:
	std::vector<STalismanSlotInfo> m_vecSlotInfo;
	CDnCharTalismanDlg *m_pParentTalismanDlg;
	CDnTooltipTalismanDlg *m_pTooltipTalismanDlg;
	bool m_bDragMode;
	CEtUIStatic *m_pSelectedCover;
	CDnItem *m_pSelectedDetachItem;
	int m_nSelectedDetachSlot;
	float m_fContentHeight;

	void InitTalismanSlot();

public:
	void SetParentTalismanDlg( CDnCharTalismanDlg *pParentDlg ) { m_pParentTalismanDlg = pParentDlg; }
	float GetContentHeight() const { return m_fContentHeight; }
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	void SetTalismanItem( int nSlotIndex, CDnItem *pItem );
	void ResetTalismanSlot( int nSlotIndex );
	void RefreshTalisman();
	void RefreshOpenSlot();
	void RefreshCashSlot();
	void SetSlotMagneticMode( bool bMagnetic );
	void SetEfficiencyTextShow( bool bShow );
	void HideTooltip();

	MIInventoryItem *GetEquipTalisman( const CDnItem *pItem ) const;
	CDnCharStatusDlg::eRetWearable GetWearableTalismanEquipType( const MIInventoryItem *pItem, int &nSlotIndex, std::wstring *strErrorMsg = NULL );
	void EquipTalismanFromInven( int nInvenIndex, MIInventoryItem *pItem, CDnSlotButton *pSlotButton );

	int GetSlotCount() const { return (int)m_vecSlotInfo.size(); }
	const STalismanSlotInfo *GetSlotInfo( int nSlotIndex ) const;
};

#endif // PRE_ADD_TALISMAN_SYSTEM
