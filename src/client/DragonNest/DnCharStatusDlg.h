#pragma once
#include "DnCustomDlg.h"
#include "Singleton.h"
#include "DnParts.h"
#include "DnWeapon.h"
#include "DnSmartMoveCursor.h"

class CDnCharStatusBaseInfoDlg;
class CDnCharStatusDetailInfoDlg;
class MIInventoryItem;
class CDnAppellationDlg;
class CDnCharStatusPVPInfoDlg;
class CDnCharPlateDlg;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
class CDnCharTalismanDlg;
#endif
class CDnCharVehicleDlg;
class CDnCharPetDlg;
class CDnCharStatusLadderInfoDlg;
class CDnCharStatusGuildWarInfoDlg;

namespace{

	enum UIStatusUIstring
	{
		UI_GeneralPage = 1360,
		UI_DetailedPage = 1359,
		UI_PVPPage = 1364,
		UI_LadderPage = 126150,
		UI_GuildWarPage = 126151,
	};
}

class CDnCharStatusDlg : public CDnCustomDlg, public CEtUICallback
{
	enum
	{
		ITEMSLOT_MAX_SIZE = 13
	};

	enum
	{
		GENERAL_PAGE= 0,
		DETAILED_PAGE   = 1,
		PVP_PAGE = 2,
#ifndef PRE_ADD_PVP_HIDE_LADDERSYSTEM
		LADDER_PAGE,
#endif
#ifdef PRE_REMOVE_GUILD_WAR_UI
		GUILD_WAR_PAGE,
#endif // PRE_REMOVE_GUILD_WAR_UI
		MAX_PAGE,
	};

public:
	enum eRetWearable
	{
		eWEAR_ENABLE,
		eWEAR_UNABLE,
		eWEAR_NEED_UNSEAL,
	};

	enum EquipPageType {
		EquipPageNormal,
		EquipPageCash,
		EquipPagePlate,
		EquipPageVehicle, // Rotha - Vehicle 
		EquipPagePet,     // Rotha - Pre-adding pet
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		EquipPageTalisman,
#endif
	};

	enum{
		GLYPH_DETACH_DIALOG,
		GLYPH_ATTACH_DIALOG,
		GLYPH_LIFT_DIALOG,

		EQUIPLOCK_CONFIRM_DIALOG,
		EQUIPUNLOCK_CONFIRM_DIALOG,
	};


	enum eRadioOptType{
		eRadioOpt_None,		// Enable all radio buttons
		eRadioOpt_Glyph,	// Disable all radio buttons except Crest
	};

	CDnCharStatusDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharStatusDlg(void);

protected:
//	CDnItemSlotButton *m_pEquipSlotButton[12];
//	CDnItemSlotButton *m_pCashEquipSlotButton[15];
	std::vector< CDnItemSlotButton * > m_VecEquipSlotButton;
	std::vector< CDnItemSlotButton * > m_VecCashEquipSlotButton;
	std::vector< CEtUIButton * > m_VecSwapPartsButton;
	CEtUICheckBox *m_pCheckToggleHelmet;

	CEtUIStatic *m_pStaticName;
	CEtUIStatic *m_pMagnetic;
#ifdef PRE_ADD_EQUIPLOCK
	CEtUIStatic* m_pEquipLockMagnetic;
	bool m_bShowEquipLockMagnetic;
#endif
	CEtUIStatic *m_pNormalEquipBase;
	CEtUIStatic *m_pCashEquipBase;

	CEtUIRadioButton *m_pNormalButton;
	CEtUIRadioButton *m_pCashButton;
	CEtUIButton *m_pSwapWeapon[2];
	CEtUIRadioButton *m_pPlateButton;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	CEtUIRadioButton *m_pTalismanButton;
#endif


	CEtUIComboBox *m_pComboBoxInfo; // Button was reworked into a combo box.

	bool m_bDetailInfo;
	std::wstring m_strLastErrorMsg;

	CEtUIButton *m_pButtonAppellation;
	CDnCharStatusBaseInfoDlg *m_pBaseInfoDlg;
	CDnCharStatusDetailInfoDlg *m_pDetailInfoDlg;
	CDnCharStatusPVPInfoDlg *m_pPVPInfoDlg;
	CDnAppellationDlg *m_pAppellationDlg;
	CDnCharPlateDlg	*m_pPlateDlg;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	CDnCharTalismanDlg *m_pTalismanDlg;
#endif
	CEtUIRadioButton *m_pVehicleButton;
	CDnCharVehicleDlg	*m_pVehicleDlg;
	CEtUIRadioButton	*m_pPetButton;
	CDnCharPetDlg		*m_pPetDlg;
	CDnCharStatusLadderInfoDlg *m_pLadderInfoDlg;
	CDnCharStatusGuildWarInfoDlg* m_pGuildWarInfoDlg;

#if defined( PRE_ADD_COSTUME_SKILL )
	CDnQuickSlotButton *m_pCustumeSkillSlotButton;
#endif

	int m_iPage;
	bool m_bFocusCashEquip;
//	bool m_bLastFocusCashEquip;
	EquipPageType m_FocusEquipPage;

	CDnSmartMoveCursorEx m_SmartMoveEx;

	MIInventoryItem * m_pTempSlotItem;
	int	m_nTempSlotIndex;
	CDnSlotButton *m_pTempSlotButton;

	CDnItemSlotButton* m_pSourceItemSlot;

protected:
	void SetSlotMagneticMode( bool bMagnetic );
	void SelectPage();
	void HideInfoPage();
	void RefreshOnepieceBlock();

#ifdef PRE_ADD_CASHINVENTAB
	CDnItem * GetCashInvenItemBySN( INT64 SN );
#endif // PRE_ADD_CASHINVENTAB

public:

	void SwapPlateDlg( bool bPlate, bool bForce = false );
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	void SwapTalismanDlg( bool bTalisman, bool bForce = false );
#endif

	void SwapVehicleDlg(bool bTrue, bool bForce = false);
	CDnCharVehicleDlg *GetVehicleDlg()		{ return m_pVehicleDlg; }
	void SwapPetDlg( bool bTrue, bool bForce = false );
	CDnCharPetDlg *GetPetDlg() { return m_pPetDlg; }
	bool SetEquipItemFromInven( int nInvenIndex, MIInventoryItem *pItem, CDnSlotButton *pSlotButton );

	void ChangeEquipPage( EquipPageType Type, bool bFocus = true );
	void SwapEquipButtons( bool bCash, bool bForce = false );
	void ShowEquipButtons( bool bShow );
	MIInventoryItem *GetEquipItem( int nEquipIndex );
	void SetEquipItem( int nEquipIndex, MIInventoryItem *pItem );
	void SetSourceItem( MIInventoryItem* pItem );

	void OnRefreshPlayerStatus();
	void RefreshEquip();
	void ResetEquipSlot( int nEquipIndex );
	MIInventoryItem *GetEquipGlyph( int nTypeParam );
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	MIInventoryItem *GetEquipTalisman( const CDnItem *pItem ) const;
	eRetWearable GetWearableTalismanEquipType( const MIInventoryItem *pItem, int &nSlotIndex, std::wstring *strErrorMsg = NULL );
#endif

	MIInventoryItem *GetCashEquipItem( int nEquipIndex );
	void SetCashEquipItem( int nEquipIndex, MIInventoryItem *pItem );
	void RefreshCashEquip();
	void ResetCashEquipSlot( int nEquipIndex );

	void SetGlyphEquipItem( int nEquipIndex, MIInventoryItem *pItem );
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	void SetTalismanEquipItem( int nSlotIndex, MIInventoryItem *pItem );
	void ResetTalismanEquipSlot( int nSlotIndex );
	void RefreshTalismanEquip();
	void RefreshTalismanOpenSlot();
	void RefreshTalismanCashSlot();
#endif
	void RefreshGlyphEquip();
	void ResetGlyphEquipSlot( int nEquipIndex );
	void RefreshGlyphCover();
	void RefreshLadderInfo();

	void SetVehicleEquipItem( int nEquipIndex, MIInventoryItem *pItem );
	void RefreshVehicleEquip();
	void ResetVehicleEquipSlot( int nEquipIndex );
	void RefreshVehiclePreview();

	void SetPetEquipItem( CDnItem* pItem );
	void ResetPetEquipSlot( int nEquipIndex );
	void RefreshPetPreview();

	void SetWithDarwOpen( bool bOpen );
	bool GetWithDrawOpen();

	void SetRadioButton( eRadioOptType eType);

	void RefreshHideHelmetCheckBox();


#if defined( PRE_ADD_COSTUME_SKILL )
	void RefreshCustumeSkill( DnSkillHandle hSkill );
#endif

public:
	void SetSlotEvent();

	static int EQUIPINDEX_2_SLOTINDEX( int nEquipIndex );
	static int SLOTINDEX_2_EQUIPINDEX( int nSlotIndex );
	static int PARTSTYPE_2_EQUIPINDEX( CDnParts::PartsTypeEnum partsType );
	static int EQUIPTYPE_2_EQUIPINDEX( CDnWeapon::EquipTypeEnum equipType );

	static int CASH_EQUIPINDEX_2_SLOTINDEX( int nEquipIndex );
	static int CASH_SLOTINDEX_2_EQUIPINDEX( int nSlotIndex );
	static int CASH_PARTSTYPE_2_EQUIPINDEX( CDnParts::PartsTypeEnum partsType );
	static int CASH_EQUIPTYPE_2_EQUIPINDEX( CDnWeapon::EquipTypeEnum equipType );

#ifdef PRE_ADD_EQUIPLOCK
	int GetEquipIndex( const CDnItem *pItem ) const;
	int GetCashEquipIndex( const CDnItem *pItem ) const;
	void EnableRadioButton(bool bEnable);
#else
	int GetEquipIndex( CDnItem *pItem );
	int GetCashEquipIndex( CDnItem *pItem );
#endif

	eRetWearable GetWearableEquipType( MIInventoryItem *pItem, eEquipType &equipType );
	eRetWearable GetWearableCashEquipType( MIInventoryItem *pItem, eCashEquipType &equipType );
	eRetWearable GetWearableGlyphEquipType( MIInventoryItem *pItem, eGlyph &equipType );

	std::wstring &GetLastErrorMsg() { return m_strLastErrorMsg; }

	void RefreshGuildInfo();

	bool IsEquipped( CDnItem * pItem, bool bCash ); // Is it equipped?

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Render( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void Process( float fElapsedTime );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	CDnAppellationDlg *GetAppellationDlg()	{ return m_pAppellationDlg; }
};