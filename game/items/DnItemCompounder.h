#pragma once

//------------------------------------------------------------------------------
/**
Note Hangi:

Loads and holds data from the table, then when an item synthesis / heraldry plate creation request arrives
Class that returns an appropriate result.

Because it does not create anything new or modify data after reading it
Not thread-safe.
*/
//------------------------------------------------------------------------------

const DWORD NUM_MAX_NEED_ITEM = 10;
const DWORD NUM_MAX_PLATE_COMPOUND_VARI = 5;
const DWORD NUM_MAX_RESULT_ITEM = 3;


class CDnItemCompounder
{
public:
	enum ERROR_CODE
	{
		E_NONE = 0,					// Not an error
		E_NOT_ENOUGH_MONEY,			// Not enough money
		E_NOT_ENOUGH_ITEM,			// Insufficient material items
		E_NOT_MATCH_SLOT,				// Item does not fit the slot
		E_NOT_ENOUGH_ITEM_COUNT,		// Fits the slot, but insufficient quantity
		E_NOT_MATCH_PLATE_WITH_ITEM_COMPOUND,		// The given item combination index does not exist in the plate's item combination list.

		E_INVALID_CALL,				// Invalid call
	};

	enum RESULT_CODE
	{
		R_SUCCESS,					// Combination succeeded
		R_FAIL,						// Combination failed
		R_POSSIBILITY_FAIL,			// Probability check failed.
		R_ERROR,					// Request error
	};

	// Combination request
	struct S_ITEM_SETTING_INFO
	{
		int iCompoundTableID;
#ifndef _UNIT_TEST		// In the unit test project, structs seem to be misaligned by 4 bytes..
		INT64 iHasMoney;
#else
		int iHasMoney;
#endif
		int aiItemID[ NUM_MAX_NEED_ITEM ];			// Item ID placed in material item slot
		int aiItemCount[ NUM_MAX_NEED_ITEM ];		// Item count placed in material item slot.
		INT64 iDiscountedCost;						// Replace with this value when the fee is discounted using a remote crafting item, etc.
	
		S_ITEM_SETTING_INFO( void )
		{
			ZeroMemory( this, sizeof(S_ITEM_SETTING_INFO) );
		}
	};

	// Combination result 
	struct S_OUTPUT
	{
		vector<int> vlItemID;
		vector<int> vlItemCount;
		char cItemOptionIndex;
#ifndef _UNIT_TEST		// In the unit test project, structs seem to be misaligned by 4 bytes..
		INT64 iCost;
#else
		int iCost;
#endif
		RESULT_CODE eResultCode;				// Result code
		ERROR_CODE eErrorCode;					// Report error code when an error occurs
		float fTimeRequired;

		S_OUTPUT( void ) : iCost( 0 ), eResultCode( R_SUCCESS ), 
			eErrorCode( E_NONE ), fTimeRequired( 0.0f )
			, cItemOptionIndex(0)
		{

		}
	};

#ifdef _UNIT_TEST
	struct S_PLATE_INFO
#else
	struct S_PLATE_INFO : public TBoostMemoryPool< S_PLATE_INFO >
#endif
	{
		int iPlateItemID;
		//string strBGTextureFileName;
		int aiCompoundTableID[ NUM_MAX_PLATE_COMPOUND_VARI ];
		int aiCompoundTableNum[ NUM_MAX_PLATE_COMPOUND_VARI ];

		S_PLATE_INFO( void )
		{
			ZeroMemory( this, sizeof(S_PLATE_INFO) );
		}

	};


private:
#ifdef _UNIT_TEST
	struct S_ITEM_COMPOUND_INFO
#else
	struct S_ITEM_COMPOUND_INFO : public TBoostMemoryPool< S_ITEM_COMPOUND_INFO >
#endif
	{
		int iCompoundTableID;
		int iNameStringID;
		int iDescStringID;
		float fSuccessPossibility;
#ifndef _UNIT_TEST		// In the unit test project, structs seem to be misaligned by 4 bytes..
		INT64 iCost;
#else
		int iCost;
#endif
		char cSuccessItemOptionIndex;

		float fTimeRequired;									// Time required for combination

		int aiSuccessTypeOrCount[ NUM_MAX_RESULT_ITEM ];		// 0 for random; otherwise acts as drop table index
		int aiSuccessItemID[ NUM_MAX_RESULT_ITEM ];				// If SuccessType is greater than 0, result item count
		int aiFailTypeOrCount[ NUM_MAX_RESULT_ITEM ];			// Same as SuccessType
		int aiFailItemID[ NUM_MAX_RESULT_ITEM ];				// Same as SuccessItemID..

		int aiSlotItemID[ NUM_MAX_NEED_ITEM ];					// Required item ID for one material item slot
		int aiSlotItemCount[ NUM_MAX_NEED_ITEM ];				// Required item count for material item slot
		bool bResultItemIsNeedItem;
		bool abResultItemIsNeedItem[ NUM_MAX_NEED_ITEM ];

		int iCompoundPreliminaryID;	//Field restricting the option number of equipment material

		S_ITEM_COMPOUND_INFO( void )
		{
			ZeroMemory( this, sizeof(S_ITEM_COMPOUND_INFO) );
		}
	};

	//template< typename T >
	//struct FindByItemID : public unary_function<const T*, bool>
	//{
	//	int m_iItemIDToFind;

	//	FindByItemID( int iItemIDToFind ) : m_iItemIDToFind( iItemIDToFind ) {};
	//	bool operator() ( const T* pRhs )
	//	{
	//		return (pRhs->iItemID == m_iItemIDToFind);
	//	}
	//};
public:
	// Combination info needed externally
	struct S_COMPOUND_INFO_EXTERN
	{
		int iCompoundTableID;
		int aiSuccessTypeOrCount[ NUM_MAX_RESULT_ITEM ];		// 0 for random; otherwise acts as drop table index
		int aiSuccessItemID[ NUM_MAX_RESULT_ITEM ];				// If SuccessType is greater than 0, result item count
		int aiItemID[ NUM_MAX_NEED_ITEM ];			// Item ID placed in material item slot
		int aiItemCount[ NUM_MAX_NEED_ITEM ];		// Item count placed in material item slot.
#ifndef _UNIT_TEST		// In the unit test project, structs seem to be misaligned by 4 bytes..
		INT64 iCost;								// Cost
#else
		int iCost;
#endif
		float fSuccessPossibility;					// Success rate
		float fTimeRequired;						// Elapsed time
		bool bResultItemIsNeedItem;
		bool abResultItemIsNeedItem[ NUM_MAX_NEED_ITEM ];

		int iCompoundPreliminaryID;	//Field restricting the option number of equipment material

		S_COMPOUND_INFO_EXTERN( void )
		{
			ZeroMemory( this, sizeof(S_COMPOUND_INFO_EXTERN) );
		};
	};

private:
	// Read and hold all information needed for combination upfront..
	//vector<S_ITEM_COMPOUND_INFO*> m_vlpCompoundInfo;
	map<int, S_ITEM_COMPOUND_INFO*> m_mapCompoundInfo;
	vector<S_PLATE_INFO*> m_vlpPlateInfo;
	map<int, S_PLATE_INFO*> m_mapPlateInfoByItemID;
	typedef std::map<int, S_ITEM_COMPOUND_INFO*> TMapItemCompoundInfo;
#ifdef _CLIENT
	#ifdef PRE_ADD_PARTSITEM_TOOLTIP_INFO
	std::set<int> m_SuccessIDsAsCompoundResultItem;	// Cache for determining whether an item can be crafted in parts item tooltips
	#endif
#endif

	int m_iLastError;
	bool m_bCheckPossibility;

protected:
	void _ResetOutputInfo( S_OUTPUT* pOutput );
#if defined( _GAMESERVER )
	void _Compound( CMultiRoom* pRoom, S_ITEM_SETTING_INFO &Input, S_OUTPUT* pResult );
#else
	void _Compound( S_ITEM_SETTING_INFO &Input, S_OUTPUT* pResult );
#endif // #if defined( _GAMESERVER )
	//int _ValidateCompound( S_PLATE_INFO* pPlateInfo, const multiset<int>& setJewelItemID );

public:
	CDnItemCompounder(void);
	~CDnItemCompounder(void);

	// Initialize per table
	bool InitializeTable( void );

	// Return new item ID on upgrade success
	//int UpgradeJewel( int iJewelItemID, int iNumJewel, /*OUT*/ int* pCost, /*OUT*/ int* pNumUse );

	// Information needed for UI display.
	void GetCompoundInfo( int iCompoundTableID, /*OUT*/S_COMPOUND_INFO_EXTERN* pInfo );
	int GetEmblemCompoundInfoCountByPlate( int iPlateTableID );
	void GetEmblemCompoundInfo( int iPlateTableID, int iCompoundTableID, /*OUT*/S_COMPOUND_INFO_EXTERN* pInfo );
	void GetEmblemCompoundInfoByIndex( int iPlateTableID, int iIndex, /*OUT*/S_COMPOUND_INFO_EXTERN* pInfo );
	const CDnItemCompounder::S_PLATE_INFO* GetPlateInfoByItemID( int iPlateTableID );

	// Function to validate before combination
	void ValidateEmblemCompound( int iPlateTableID, S_ITEM_SETTING_INFO &Input, S_OUTPUT* pResult );
	void ValidateCompound( S_ITEM_SETTING_INFO &Input, S_OUTPUT* pResult );

	// Combination functions.. Returns 0 on failure
#if defined( _GAMESERVER )
	void EmblemCompound( CMultiRoom* pRoom, int iPlateItemID, S_ITEM_SETTING_INFO& Input, /*OUT*/S_OUTPUT* pResult );
	void Compound( CMultiRoom* pRoom, /*IN*/ S_ITEM_SETTING_INFO& Input, /*OUT*/S_OUTPUT* pResult );
#else
	void EmblemCompound( int iPlateItemID, S_ITEM_SETTING_INFO& Input, /*OUT*/S_OUTPUT* pResult );
	void Compound( /*IN*/ S_ITEM_SETTING_INFO& Input, /*OUT*/S_OUTPUT* pResult );
#endif // #if defined( _GAMESERVER )

	int GetLastError( void ) { return m_iLastError; };

	void CheckPossibility( bool bCheckPossibility ) { m_bCheckPossibility = bCheckPossibility; };

#ifdef _CLIENT
	#ifdef PRE_ADD_PARTSITEM_TOOLTIP_INFO
	bool IsEnableCompoundItem(int itemId) const;
	#endif
#endif
};
