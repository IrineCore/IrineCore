#include "stdafx.h"
#include "DnItemCompounder.h"
#include "SundriesFunc.h"
#include <mmsystem.h>

#ifdef _UNIT_TEST
#include "DNTableFile.h"
#else
#include "DnTableDB.h"
#endif

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnItemCompounder::CDnItemCompounder(void) : m_iLastError( 0 ), m_bCheckPossibility( true )
{

}

CDnItemCompounder::~CDnItemCompounder(void)
{
	SAFE_DELETE_PMAP( TMapItemCompoundInfo, m_mapCompoundInfo );
	SAFE_DELETE_PVEC( m_vlpPlateInfo );
}


bool CDnItemCompounder::InitializeTable( void )
{
	// TODO: Also check data integrity, such as mismatched result item and item count pairs.

	bool bResult = true;

	// Load data from the combination table.

#ifdef _UNIT_TEST
	DNTableFileFormat* pCompoundTable = new DNTableFileFormat( "R:/GameRes/Resource/Ext/ItemCompoundTable.ext" );
#else				//#ifdef _UNIT_TEST
	DNTableFileFormat* pCompoundTable = GetDNTable( CDnTableDB::TITEMCOMPOUND );
#endif				//#ifdef _UNIT_TEST


	char acBuf[ 128 ];
	ZeroMemory( acBuf, sizeof(acBuf) );

	int iNumItem = pCompoundTable->GetItemCount();
	for( int iItem = 0; iItem < iNumItem; ++iItem )
	{
		S_ITEM_COMPOUND_INFO* pNewCompoundInfo = new S_ITEM_COMPOUND_INFO;

		int iItemID = pCompoundTable->GetItemID( iItem );

		// Basic info
		pNewCompoundInfo->iCompoundTableID = iItemID;
		pNewCompoundInfo->iNameStringID = pCompoundTable->GetFieldFromLablePtr( iItemID, "_NameID" )->GetInteger();
		pNewCompoundInfo->iDescStringID = pCompoundTable->GetFieldFromLablePtr( iItemID, "_DescriptionID" )->GetInteger();
		pNewCompoundInfo->fSuccessPossibility = pCompoundTable->GetFieldFromLablePtr( iItemID, "_Possibility" )->GetFloat();
		pNewCompoundInfo->iCost = pCompoundTable->GetFieldFromLablePtr( iItemID, "_Cost" )->GetInteger();
		pNewCompoundInfo->fTimeRequired = pCompoundTable->GetFieldFromLablePtr( iItemID, "_TimeRequired" )->GetFloat();
		pNewCompoundInfo->cSuccessItemOptionIndex = (char)pCompoundTable->GetFieldFromLablePtr( iItemID, "_CompoundPotentialID" )->GetInteger();

		pNewCompoundInfo->iCompoundPreliminaryID = pCompoundTable->GetFieldFromLablePtr(iItemID, "_CompoundPreliminaryID")->GetInteger();
		// If probability is 0%, it is not real data
		if( 0.0f == pNewCompoundInfo->fSuccessPossibility )
		{
//#ifndef _VILLAGESERVER
//			if( 0.0f != pNewCompoundInfo->fTimeRequired )
//				OutputDebug( "[CDnItemCompounder::InitializeTable()] %d failed to load compound info. Prob 0%.\n", iItemID );
//#endif
			SAFE_DELETE( pNewCompoundInfo );
			continue;
		}

		// Combination result type and result item.. Separated into success and failure cases.
		for( int i = 0;  i < NUM_MAX_RESULT_ITEM; ++i )
		{
			sprintf_s( acBuf, "_SuccessType%d", i+1 );
			pNewCompoundInfo->aiSuccessTypeOrCount[ i ] = pCompoundTable->GetFieldFromLablePtr( iItemID, acBuf )->GetInteger();

			sprintf_s( acBuf, "_SuccessItemID%d", i+1 );
			pNewCompoundInfo->aiSuccessItemID[ i ] = pCompoundTable->GetFieldFromLablePtr( iItemID, acBuf )->GetInteger();

#ifdef _CLIENT
	#ifdef PRE_ADD_PARTSITEM_TOOLTIP_INFO
			if (i == 0 && pNewCompoundInfo->aiSuccessItemID[i] > 0)
				m_SuccessIDsAsCompoundResultItem.insert(pNewCompoundInfo->aiSuccessItemID[i]);
	#endif
#endif

			sprintf_s( acBuf, "_FailType%d", i+1 );
			pNewCompoundInfo->aiFailTypeOrCount[ i ] = pCompoundTable->GetFieldFromLablePtr( iItemID, acBuf )->GetInteger();

			sprintf_s( acBuf, "_FailItemID%d", i+1 );
			pNewCompoundInfo->aiFailItemID[ i ] = pCompoundTable->GetFieldFromLablePtr( iItemID, acBuf )->GetInteger();
		}

		// Slot info
		for( int i = 0; i < NUM_MAX_NEED_ITEM; ++i )
		{
			sprintf_s( acBuf, "_Slot%dId", i+1 );
			pNewCompoundInfo->aiSlotItemID[ i ] = pCompoundTable->GetFieldFromLablePtr( iItemID, acBuf )->GetInteger();

			sprintf_s( acBuf, "_Slot%dNum", i+1 );
			pNewCompoundInfo->aiSlotItemCount[ i ] = pCompoundTable->GetFieldFromLablePtr( iItemID, acBuf )->GetInteger();
		}

		pNewCompoundInfo->bResultItemIsNeedItem = false;
		for( int i=0; i<NUM_MAX_RESULT_ITEM; i++ ) {
			for( int j=0; j<NUM_MAX_NEED_ITEM; j++ ) {
				if( pNewCompoundInfo->aiSuccessItemID[i] == pNewCompoundInfo->aiSlotItemID[j] && pNewCompoundInfo->aiSuccessItemID[i] != 0 ) {
					pNewCompoundInfo->bResultItemIsNeedItem = true;
					pNewCompoundInfo->abResultItemIsNeedItem[j] = true;
					break;
				}
			}
//			if( pNewCompoundInfo->bResultItemIsNeedItem ) break;
		}

		//m_vlpCompoundInfo.push_back( pNewCompoundInfo );
		m_mapCompoundInfo.insert( make_pair(pNewCompoundInfo->iCompoundTableID, pNewCompoundInfo) );
	}

#ifdef _UNIT_TEST
	delete pCompoundTable;
#endif

	// Load data from plate table.

#ifdef _UNIT_TEST
	DNTableFileFormat* pPlateTable = new DNTableFileFormat( "R:/GameRes/Resource/Ext/PlateTable.ext" );
#else
	DNTableFileFormat* pPlateTable = GetDNTable( CDnTableDB::TPLATE );
#endif


	ZeroMemory( acBuf, sizeof(acBuf) );
	iNumItem = pPlateTable->GetItemCount();
	for( int iItem = 0; iItem < iNumItem; ++iItem )
	{
		S_PLATE_INFO* pNewPlateInfo = new S_PLATE_INFO;

		int iItemID = pPlateTable->GetItemID( iItem );

		pNewPlateInfo->iPlateItemID = pPlateTable->GetFieldFromLablePtr( iItemID, "_ItemID" )->GetInteger();

		// Item ID 0 is invalid data
		// Note: Need to reference the item table to check whether the item exists and whether it is a plate-type item.
		if( 0 == pNewPlateInfo->iPlateItemID ) {
			SAFE_DELETE( pNewPlateInfo );
			continue;
		}

		for( int i = 0; i < NUM_MAX_PLATE_COMPOUND_VARI; ++i )
		{
			sprintf_s( acBuf, "_CompoundTableIndex%d", i+1 );
			pNewPlateInfo->aiCompoundTableID[ i ] = pPlateTable->GetFieldFromLablePtr( iItemID, acBuf )->GetInteger();
		}

		m_vlpPlateInfo.push_back( pNewPlateInfo );
		m_mapPlateInfoByItemID.insert( make_pair(pNewPlateInfo->iPlateItemID, pNewPlateInfo) );
	}

#ifdef _UNIT_TEST
	delete pPlateTable;
#endif

	return bResult;
}


// Information needed for UI display.
void CDnItemCompounder::GetCompoundInfo( int iCompoundTableID, /*OUT*/S_COMPOUND_INFO_EXTERN* pInfo )
{
	if( pInfo == NULL || iCompoundTableID < 0 )
		return;

	map<int, S_ITEM_COMPOUND_INFO*>::iterator iterCompoundInfo = m_mapCompoundInfo.find( iCompoundTableID );
	if( m_mapCompoundInfo.end() == iterCompoundInfo )
		return;

	const S_ITEM_COMPOUND_INFO* pCompoundInfo = iterCompoundInfo->second;//m_vlpCompoundInfo.at( iCompoundTableID-1 );
	pInfo->iCompoundTableID = pCompoundInfo->iCompoundTableID;
	memcpy_s( pInfo->aiSuccessTypeOrCount, sizeof(pInfo->aiSuccessTypeOrCount), pCompoundInfo->aiSuccessTypeOrCount, sizeof(pCompoundInfo->aiSuccessTypeOrCount) );
	memcpy_s( pInfo->aiSuccessItemID, sizeof(pInfo->aiSuccessItemID), pCompoundInfo->aiSuccessItemID, sizeof(pCompoundInfo->aiSuccessItemID) );
	//pInfo->iNameStringID = pCompoundInfo->iNameStringID;
	//pInfo->iDescStringID = pCompoundInfo->iDescStringID;
	memcpy_s( pInfo->aiItemID, sizeof(pInfo->aiItemID), pCompoundInfo->aiSlotItemID, sizeof(pCompoundInfo->aiSlotItemID) );
	memcpy_s( pInfo->aiItemCount, sizeof(pInfo->aiItemCount), pCompoundInfo->aiSlotItemCount, sizeof(pCompoundInfo->aiSlotItemCount) );
	pInfo->iCost= pCompoundInfo->iCost;
	pInfo->fSuccessPossibility = pCompoundInfo->fSuccessPossibility;
	pInfo->fTimeRequired = pCompoundInfo->fTimeRequired;
	pInfo->bResultItemIsNeedItem = pCompoundInfo->bResultItemIsNeedItem;
	memcpy_s( pInfo->abResultItemIsNeedItem, sizeof(pInfo->abResultItemIsNeedItem), pCompoundInfo->abResultItemIsNeedItem, sizeof(pCompoundInfo->abResultItemIsNeedItem) );

	pInfo->iCompoundPreliminaryID = pCompoundInfo->iCompoundPreliminaryID;
}


int CDnItemCompounder::GetEmblemCompoundInfoCountByPlate( int iPlateItemID )
{
	if( iPlateItemID < 0 || iPlateItemID-1 >= (int)m_vlpPlateInfo.size() )
		return -1;

	return (int)m_vlpPlateInfo.size();
}


void CDnItemCompounder::GetEmblemCompoundInfo( int iPlateItemID, int iCompoundTableID, /*OUT*/S_COMPOUND_INFO_EXTERN* pInfo )
{
	if( pInfo == NULL || iPlateItemID < 0 || iPlateItemID-1 >= (int)m_vlpPlateInfo.size() ||
		iCompoundTableID < 0 )
		return;

	// Search in the map.
	map<int, S_ITEM_COMPOUND_INFO*>::iterator iterCompoundInfo = m_mapCompoundInfo.find( iCompoundTableID );
	if( m_mapCompoundInfo.end() == iterCompoundInfo )
		return;

	// Get plate information.
	bool bValidEmblemCompound = false;
	int iSelectedCompoundTableID = -1;
	map<int, S_PLATE_INFO*>::iterator iter = m_mapPlateInfoByItemID.find( iPlateItemID );
	if( m_mapPlateInfoByItemID.end() != iter )
	{
		S_PLATE_INFO* pPlateInfo = iter->second;

		for( int i = 0; i < NUM_MAX_PLATE_COMPOUND_VARI; ++i )
		{
			if( iCompoundTableID == pPlateInfo->aiCompoundTableID[ i ] )
			{
				iSelectedCompoundTableID = iCompoundTableID;
				bValidEmblemCompound = true;
				break;
			}
		}
	}

	if( bValidEmblemCompound )
		GetCompoundInfo( iSelectedCompoundTableID, pInfo );
}


void CDnItemCompounder::GetEmblemCompoundInfoByIndex( int iPlateItemID, int iIndex, /*OUT*/S_COMPOUND_INFO_EXTERN* pInfo )
{
	if( pInfo == NULL || iPlateItemID < 0 || iPlateItemID-1 >= (int)m_vlpPlateInfo.size() ||
		iIndex < 0 || iIndex >= NUM_MAX_PLATE_COMPOUND_VARI )
		return;

	// Get plate information.
	bool bValidEmblemCompound = false;
	int iSelectedCompoundTableID = -1;
	map<int, S_PLATE_INFO*>::iterator iter = m_mapPlateInfoByItemID.find( iPlateItemID );
	if( m_mapPlateInfoByItemID.end() != iter )
	{
		S_PLATE_INFO* pPlateInfo = iter->second;
		int iCompoundTableID = pPlateInfo->aiCompoundTableID[ iIndex ];

		GetCompoundInfo( iCompoundTableID, pInfo );
	}
}


const CDnItemCompounder::S_PLATE_INFO* CDnItemCompounder::GetPlateInfoByItemID( int iPlateTableID )
{
	map<int, S_PLATE_INFO*>::iterator iter = m_mapPlateInfoByItemID.find( iPlateTableID );
	if( m_mapPlateInfoByItemID.end() != iter )
	{
		return iter->second;	
	}

	return NULL;
}


void CDnItemCompounder::_ResetOutputInfo( S_OUTPUT* pOutput )
{
	if( NULL == pOutput )
		return;

	pOutput->vlItemID.clear();
	pOutput->vlItemCount.clear();
	pOutput->cItemOptionIndex = 0;

	pOutput->iCost = 0;
	pOutput->eResultCode = R_SUCCESS;
	pOutput->eErrorCode = E_NONE;
	pOutput->fTimeRequired = 0.0f;
}

void CDnItemCompounder::ValidateCompound( S_ITEM_SETTING_INFO &Input, S_OUTPUT* pResult )
{
	_ASSERT( 0 < Input.iDiscountedCost );

	_ResetOutputInfo( pResult );

	// Search in the map.
	map<int, S_ITEM_COMPOUND_INFO*>::iterator iterCompoundInfo = m_mapCompoundInfo.find( Input.iCompoundTableID );
	if( m_mapCompoundInfo.end() == iterCompoundInfo )
		return;

	if( NULL == pResult )
	{
		OutputDebug( "[EmblemCompound Error] Invalid input value.\n" );
		return;
	}

	// Fetch combination info
	S_ITEM_COMPOUND_INFO* pCompoundInfo = iterCompoundInfo->second;//m_vlpCompoundInfo.at( Input.iCompoundTableID-1 );

	// Is there enough money?
	INT64 iCost = pCompoundInfo->iCost;
	if( 0 < Input.iDiscountedCost )
		iCost = Input.iDiscountedCost;

	if( Input.iHasMoney < iCost )
	{
		pResult->eErrorCode = E_NOT_ENOUGH_MONEY;
		pResult->eResultCode = R_ERROR;
		return;
	}

	// Check if it fits the slot, or if it fits the slot but the count is insufficient.
	for( int i  = 0; i < NUM_MAX_NEED_ITEM; ++i )
	{
		if( 0 == pCompoundInfo->aiSlotItemID[ i ] )
			break;

		if( 0 == Input.aiItemID[ i ] )
		{
			pResult->eErrorCode = E_NOT_ENOUGH_ITEM;
			pResult->eResultCode = R_ERROR;
		}
		else
		if( pCompoundInfo->aiSlotItemID[ i ] != Input.aiItemID[ i ] )
		{
			pResult->eErrorCode = E_NOT_MATCH_SLOT;
			pResult->eResultCode = R_ERROR;
			return;
		}
		else
		if( pCompoundInfo->aiSlotItemCount[ i ] != Input.aiItemCount[ i ] )
		{
			pResult->eErrorCode = E_NOT_ENOUGH_ITEM_COUNT;
			pResult->eResultCode = R_ERROR;
			return;
		}
	}
}


void CDnItemCompounder::ValidateEmblemCompound( int iPlateItemID, S_ITEM_SETTING_INFO &Input, S_OUTPUT* pResult )
{
	ValidateCompound( Input, pResult );

	if( pResult && R_ERROR != pResult->eResultCode )
	{
		if( iPlateItemID < 0 )
		{	
			pResult->eErrorCode = E_INVALID_CALL;
			pResult->eResultCode = R_ERROR;
			return;
		}

		bool bValidEmblemCompound = false;

		// Get plate information.
		map<int, S_PLATE_INFO*>::iterator iter = m_mapPlateInfoByItemID.find( iPlateItemID );
		if( m_mapPlateInfoByItemID.end() != iter )
		{
			S_PLATE_INFO* pPlateInfo = iter->second;

			for( int i = 0; i < NUM_MAX_PLATE_COMPOUND_VARI; ++i )
			{
				if( Input.iCompoundTableID == pPlateInfo->aiCompoundTableID[ i ] )
				{
					bValidEmblemCompound = true;
					break;
				}
			}
		}

		if( false == bValidEmblemCompound )
		{
			pResult->eErrorCode = E_NOT_MATCH_PLATE_WITH_ITEM_COMPOUND;
			pResult->eResultCode = R_ERROR;
			return;
		}
	}
}

#if defined( _GAMESERVER )
void CDnItemCompounder::_Compound( CMultiRoom* pRoom, S_ITEM_SETTING_INFO& Input, S_OUTPUT* pResult )
#else
void CDnItemCompounder::_Compound( S_ITEM_SETTING_INFO& Input, S_OUTPUT* pResult )
#endif // #if defined( _GAMESERVER )
{
	static long s_CompoundInterlocked = 0;
	CMtRandom Random;
	Random.srand( timeGetTime()+InterlockedIncrement(&s_CompoundInterlocked) );

	// Create if all inputs arrived properly.
	if( pResult && R_ERROR != pResult->eResultCode )
	{
		// Search in the map.
		map<int, S_ITEM_COMPOUND_INFO*>::iterator iterCompoundInfo = m_mapCompoundInfo.find( Input.iCompoundTableID );
		if( m_mapCompoundInfo.end() == iterCompoundInfo )
		{

#ifndef _VILLAGESERVER
			_ASSERT( !"Impossible. Verified everything above, how can the compound table id be invalid here? -_-" );
#else
			g_Log.Log( LogType::_ERROR, L"[Item Compound] CompoundTable: %d, No received item compound info. \n", Input.iCompoundTableID );
#endif
			pResult->eResultCode = R_ERROR;
			pResult->eErrorCode = E_INVALID_CALL;
			return;
		}

		S_ITEM_COMPOUND_INFO* pCompoundInfo = iterCompoundInfo->second;//m_vlpCompoundInfo.at( Input.iCompoundTableID-1 );

		//float r = (float)_rand() / ((float)(RAND_MAX) + 1.0f );
		//if( r <= pCompoundInfo->fSuccessPossibility || !m_bCheckPossibility )
		int iRandom = Random.rand() % 10000;
		if( iRandom <= int(pCompoundInfo->fSuccessPossibility * 10000.0f) || !m_bCheckPossibility )
		{
			// Combination succeeded.
			for( int i = 0; i < NUM_MAX_RESULT_ITEM; ++i )
			{
				if( 0 == pCompoundInfo->aiSuccessItemID[ i ] )
					break;

				pResult->vlItemCount.push_back( pCompoundInfo->aiSuccessTypeOrCount[ i ] );
				pResult->vlItemID.push_back( pCompoundInfo->aiSuccessItemID[ i ] );
			}

			pResult->cItemOptionIndex = pCompoundInfo->cSuccessItemOptionIndex;
			pResult->iCost = pCompoundInfo->iCost;
			pResult->fTimeRequired = pCompoundInfo->fTimeRequired;
			pResult->eResultCode = R_SUCCESS;
		}
		else
		{
			// Combination failed. 
			for( int i = 0; i < NUM_MAX_RESULT_ITEM; ++i )
			{
				if( 0 == pCompoundInfo->aiFailItemID[ i ] )
					break;

				pResult->vlItemCount.push_back( pCompoundInfo->aiFailTypeOrCount[ i ] ); 
				pResult->vlItemID.push_back( pCompoundInfo->aiFailItemID[ i ] );
			}

			pResult->iCost = pCompoundInfo->iCost;
			pResult->fTimeRequired = pCompoundInfo->fTimeRequired;
			pResult->eResultCode = R_POSSIBILITY_FAIL;
		}

		if( 0 < Input.iDiscountedCost )
			pResult->iCost = Input.iDiscountedCost;
	}
}


// Combine heraldry jewel
#if defined( _GAMESERVER )
void CDnItemCompounder::EmblemCompound( CMultiRoom* pRoom, int iPlateTableID, S_ITEM_SETTING_INFO& Input, /*OUT*/S_OUTPUT* pResult )
#else
void CDnItemCompounder::EmblemCompound( int iPlateTableID, S_ITEM_SETTING_INFO& Input, /*OUT*/S_OUTPUT* pResult )
#endif // #if defined( _GAMESERVER )
{
	ValidateEmblemCompound( iPlateTableID, Input, pResult );

#if defined( _GAMESERVER )
	_Compound(pRoom,Input, pResult);
#else
	_Compound(Input, pResult);
#endif // #if defined( _GAMESERVER )
}

#if defined( _GAMESERVER )
void CDnItemCompounder::Compound( CMultiRoom* pRoom, S_ITEM_SETTING_INFO& Input, /*OUT*/S_OUTPUT* pResult )
#else
void CDnItemCompounder::Compound( S_ITEM_SETTING_INFO& Input, /*OUT*/S_OUTPUT* pResult )
#endif // #if defined( _GAMESERVER )
{
	// Fetch combination info
	ValidateCompound( Input, pResult );
#if defined( _GAMESERVER )
	_Compound( pRoom, Input, pResult );
#else
	_Compound( Input, pResult );
#endif // #if defined( _GAMESERVER )
}

#ifdef _CLIENT
	#ifdef PRE_ADD_PARTSITEM_TOOLTIP_INFO
bool CDnItemCompounder::IsEnableCompoundItem(int itemId) const
{
	std::set<int>::const_iterator iter = m_SuccessIDsAsCompoundResultItem.find(itemId);
	return (iter != m_SuccessIDsAsCompoundResultItem.end());
}
	#endif
#endif