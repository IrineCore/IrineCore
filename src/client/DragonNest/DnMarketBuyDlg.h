#pragma once
#include "EtUIDialog.h"
class CDnItem;
class CDnMarketBuyListDlg;
class CDnMarketPriceTooltipDlg;

class CDnMarketBuyDlg : public CEtUIDialog, public CEtUICallback
{
public:
	enum eJob {
		JOB_NONE,
		JOB_WARRIER = 1,
		JOB_ARCHER,
		JOB_SOCERESS,
		JOB_CLERIC,

		JOB_SWORDMASTER = 11,
		JOB_MERCENARY = 12,
		JOB_BOWMASTER = 14,
		JOB_ACROBAT = 15,
		JOB_ELEMENTALLORD = 17,
		JOB_FORCEUSER = 18,
		JOB_PALADIN = 20,
		JOB_PRIEST = 22,

		JOB_MAX = 13
	};

	enum {
		ALL = -1,
	};

	enum eItemTypeEnum {
		ITEMTYPE_FARM = -3,
		ITEMTYPE_ETC = -2,
		ITEMTYPE_WEAPON = 0,		// Weapon/Sub-weapon
		ITEMTYPE_PARTS = 1,			// Armor
		ITEMTYPE_NORMAL = 2,		// Etc
		ITEMTYPE_SKILL = 3,			// 
		ITEMTYPE_JEWEL = 4,			// Jewel
		ITEMTYPE_PLATE = 5,			// Crest
		ITEMTYPE_INSTANT = 6,		//
		ITEMTYPE_QUEST = 7,			// Quest
		ITEMTYPE_RANDOM = 8,		// Random item
		ITEMTYPE_HAIRDYE = 9,
		ITEMTYPE_SKILLBOOK = 10,	// Skill book
		ITEMTYPE_GLYPH = 38,			// Crest
		ITEMTYPE_POTENTIAL_JEWEL = 43,	// Potential
		ITEMTYPE_PETALTOKEN = 49,	// Petal
		ITEMTYPE_RECIPE = 50,		// Recipe
		ITEMTYPE_SEED = 52,			// Seed
		ITEMTYPE_CROP = 64,			// Crops
		ITEMTYPE_COOKING = 65,		// Cooking
		ITEMTYPE_FISH = 66,			// Seafood
	};

	enum eWeaponType
	{
		WEAPON_SWORD,			// Greatsword
		WEAPON_GAUNTLET,		// Gauntlet
		WEAPON_AXE,				// Axe
		WEAPON_HAMMER,			// Hammer
		WEAPON_SMALLBOW,		// Shortbow
		WEAPON_BIGBOW,			// Longbow
		WEAPON_CROSSBOW,		// Crossbow
		WEAPON_STAFF,			// Staff
		WEAPON_BOOK,			// Spellbook
		WEAPON_ORB,				// Crystal orb
		WEAPON_PUPPET,			// Voodoo doll
		WEAPON_MACE,			// Mace
		WEAPON_FLAIL,			// Flail
		WEAPON_WAND,			// Wand
		WEAPON_SHIELD,			// Shield
		WEAPON_ARROW,			// Arrow
	};

	enum eEquipType
	{
		EQUIP_FACE,				// Face
		EQUIP_HAIR,				// Head
		EQUIP_HELMET,			// Helmet
		EQUIP_BODY,				// Upper body
		EQUIP_LEG,				// Bottoms
		EQUIP_HAND,				// Hand
		EQUIP_FOOT,				// Foot
		EQUIP_NECKLACE,
		EQUIP_EARRING,
		EQUIP_RING1,
		EQUIP_RING2,
		EQUIP_CREST,
		EQUIP_WEAPON1,			// Weapon
		EQUIP_WEAPON2,
	};

	enum eCount {		
		MAX_ITEM_COUNT_FOR_PAGE = 5,
		PAGE_COUNT_FOR_REQUEST = 10,
		PAGE_COUNT_FOR_SCREEN = 10,	// Expected to remain unchanged
	};

	enum eSortType {
		MARKETSORT_ITEM_ASC = 0,	// Item ascending order
		MARKETSORT_ITEM_DESC,		// Item descending order
		MARKETSORT_LEVEL_ASC,		// Level ascending
		MARKETSORT_LEVEL_DESC,		// Level descending
		MARKETSORT_ONEPRICE_ASC,	// Unit price ascending
		MARKETSORT_ONEPRICE_DESC,	// Unit price descending
		MARKETSORT_ITEMGRADE_ASC,	// Enhancement ascending
		MARKETSORT_ITEMGRADE_DESC,	// Enhancement descending
	};

	enum {
		MESSAGEBOX_LEVEL_ERROR = 3549,
		MESSAGEBOX_BUY_ITEM = 4050,
		MESSAGEBOX_PETAL_BUY_ITEM = 4161,
	};

public:
	CDnMarketBuyDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnMarketBuyDlg(void);

protected:
	struct NODE  {
		int m_nStringID;
		int m_Job;
		int m_MainType;
		int m_DetailType;
		int m_CashDetailType;
		int m_ExcahngeItemID;
		int m_GroupID;
		int m_ExchangeType;
		NODE( int nStringID, int Job = JOB_NONE, int MainType = ALL, int DetailType = ALL, int CashDetailType = ALL, int ExchangeItemID = ALL, int GroupID = ALL, int ExchangeType = ALL ) 
			: m_nStringID( nStringID )
			, m_Job( Job )
			, m_MainType( MainType )
			, m_DetailType( DetailType )
			, m_CashDetailType( CashDetailType )
			, m_ExcahngeItemID( ExchangeItemID )
			, m_GroupID( GroupID )
			, m_ExchangeType( ExchangeType )
		{}
	};

	struct SearchInfo {
		int nPageIndex;
		int nReqPageIndex;
		char cMinLevel;
		char cMaxLevel;
		std::vector<char> cItemGradeArray;
		char cJob;
		short nMainType;
		char cDetailType;
		std::wstring pwszSearch1;
		std::wstring pwszSearch2;
		std::wstring pwszSearch3;
		char cSortType;
		int nExchangeItemID;
		bool bLowJobGroup;
		std::wstring pwszSearch;
	} m_LastSearchInfo;

	std::vector< NODE *>				m_pNodes;
	std::vector< NODE >				m_Nodes;
	int											m_nTotalItemCount;
	std::deque< std::pair<int, char> >						m_RequestedKeys;
	std::pair<int, char>											m_LastBuyKey;
	std::map< std::pair<int, char >, std::vector< TMarketInfo >	>	m_CachedBuyList;
	std::set< std::pair<int, char > > m_IsNewKey;
	int											m_nBuyItemDBID;

	CDnMarketPriceTooltipDlg * m_pMarketPriceTooltipDlg;

	typedef CEtUIDialog BaseClass;
#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
	CEtUIListBox* m_pSearchItemListBox;
	CEtUIStatic* m_pSearchItemListBoxBoard;
#endif // PRE_ADD_ITEMNAME_AUTOCOMPLETE

private:
	void SetDefaultState();	
	void RequestMarketBuyList();
	void RefreshPageControls();
	int GetTotalPage();
	int GetStartPage();
	int GetReqStartPage();
	void UpdateListItems();
	void RefreshCount();
	std::pair<int, char> GetLastReqKey() { return std::make_pair( m_LastSearchInfo.nReqPageIndex, m_LastSearchInfo.cSortType ); }
	std::pair<int, char> GetLastKey() { return std::make_pair( m_LastSearchInfo.nPageIndex, m_LastSearchInfo.cSortType ); }
	void MakeEmptyKey( std::pair<int, char> &Key ) { Key.first = -1; }
	bool IsEmptyKey( std::pair<int, char> &Key ) { return (Key.first == -1);}  
	void ProcessBuy();
#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
	void OnSetSearchItemList( const std::wstring& strText );
	void OnSearchItem();
#endif // PRE_ADD_ITEMNAME_AUTOCOMPLETE

public:
	void InitialUpdate() override;
	void Initialize( bool bShow ) override;
	void Show( bool bShow ) override;
	void Process( float fElapsedTime ) override;
	void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 ) override;
	
	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) override;
	void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg ) override;

public:
	void OnRecvUpdateList( std::vector< TMarketInfo > &vecBuyList, int nTotalCount );	
	void OnRecvRemoveItem( int nMarketDBID );
	void RefreshRequestNeed() { SetDefaultState(); }
	void SetSearchItem( CDnItem *pItem );
	void OnRecvMarketPrice( int nMarketDBID, std::vector< TMarketPrice > & vecPrice );
};