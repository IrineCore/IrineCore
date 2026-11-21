#include "stdafx.h"
#include "DnEmblemFactory.h"
#include "SundriesFunc.h"

CDnEmblemFactory::CDnEmblemFactory(void)
{
}

CDnEmblemFactory::~CDnEmblemFactory(void)
{
	SAFE_DELETE_PVEC( m_vlpCompoundInfo );
	SAFE_DELETE_PVEC( m_vlpPlateInfo );
	SAFE_DELETE_PVEC( m_vlpJewelInfo );
}


bool CDnEmblemFactory::InitializeTable( void )
{
	bool bResult = true;

	// Fetch combination table data.
	//DNTableFileFormat* pCompoundTable = new DNTableFileFormat( "R:/GameRes/Resource/Ext/EmblemCompoundTable.ext" );
	DNTableFileFormat* pCompoundTable = GetDNTable( CDnTableDB::TEMBLEMCOMPOUND );

	char acBuf[ 256 ];
	int iNumItem = pCompoundTable->GetItemCount();
	m_vlpPlateInfo.reserve( iNumItem );
	for( int iItem = 1; iItem <= iNumItem; ++iItem )
	{
		S_EMBLEM_COMPOUND_INFO* pNewCompoundInfo = new S_EMBLEM_COMPOUND_INFO;

		for( DWORD dwHole = 1; dwHole <= NUM_MAX_PLATE_HOLE; ++dwHole )
		{
			ZeroMemory( acBuf, sizeof(acBuf) );
			sprintf_s( acBuf, "_Hole%d_ItemID", dwHole );
			int iJewelItemID = pCompoundTable->GetFieldFromLablePtr( iItem, acBuf )->GetInteger();
			if( iJewelItemID > 0 )		// Item index 0 is the default value in the table and does not exist as a table entry, so skip it.
				pNewCompoundInfo->setHoleItemID.insert( iJewelItemID );

			ZeroMemory( acBuf, sizeof(acBuf) );
			sprintf_s( acBuf, "_AllowShowHoleInfo_%d", dwHole );
			pNewCompoundInfo->abShowHoleInfo[dwHole-1] = pCompoundTable->GetFieldFromLablePtr( iItem, acBuf )->GetInteger() ? true : false;
		}

		pNewCompoundInfo->iResultItemID = pCompoundTable->GetFieldFromLablePtr( iItem, "_ResultItemIndex" )->GetInteger();
		pNewCompoundInfo->bAllowShowResult = pCompoundTable->GetFieldFromLablePtr( iItem, "_AllowShowResult" )->GetInteger() ? true : false;

		m_vlpCompoundInfo.push_back( pNewCompoundInfo );
	}

	// Fetch plate info
	DNTableFileFormat* pPlateTable = GetDNTable( CDnTableDB::TPLATE );
	//DNTableFileFormat* pPlateTable = new DNTableFileFormat( "R:/GameRes/Resource/Ext/PlateTable.ext" );

	iNumItem = pPlateTable->GetItemCount();
	m_vlpPlateInfo.reserve( iNumItem );
	for( int iItem = 1; iItem <= iNumItem; ++iItem )
	{
		S_PLATE_INFO* pNewPlateInfo = new S_PLATE_INFO;
		pNewPlateInfo->iItemID = pPlateTable->GetFieldFromLablePtr( iItem, "_ItemID" )->GetInteger();

		// Verify whether the same item ID exists on another plate.
#ifdef _DEBUG
		if( 0 != pNewPlateInfo->iItemID )
		{
			vector<S_PLATE_INFO*>::iterator iter = find_if( m_vlpPlateInfo.begin(), m_vlpPlateInfo.end(), 
															FindByItemID<S_PLATE_INFO>(pNewPlateInfo->iItemID) );
			if( iter != m_vlpPlateInfo.end() )
			{
				_ASSERT( "Duplicate item index found in PlateTable!" );

				// Returns without freeing memory. Since this case should never happen..
				return false;
			}
		}
#endif

		const char* pUIFileName = pPlateTable->GetFieldFromLablePtr( iItem, "_BackgroundTexture" )->GetString();
		//if( NULL == pUIFileName )
		//{
		//	//OutputDebug( "[Plate Table Error!!] ItemID: %d UIFile field is empty!\n", iItem );
		//	//_ASSERT( ! );
		//	//return false;
		//	//continue;
		//}
		//// Once data is properly written, remove the else branch
		//else
		pNewPlateInfo->strBGTextureFileName.assign( pUIFileName );
		pNewPlateInfo->iSlotTypeIndex = pPlateTable->GetFieldFromLablePtr( iItem, "_SlotTypeIndex" )->GetInteger();
		pNewPlateInfo->iNumSlot = pPlateTable->GetFieldFromLablePtr( iItem, "_SlotNumber" )->GetInteger();

		char acBuf[ 256 ];
		ZeroMemory( acBuf, sizeof(acBuf) );
		for( DWORD i = 0; i < NUM_MAX_JEWEL_COMPOUND; ++i )
		{
			// 0 means none
			sprintf_s( acBuf, "_CompoundTableIndex%d", i+1 );
			pNewPlateInfo->aiCompoundTableIndex[ i ] = pPlateTable->GetFieldFromLablePtr( iItem, acBuf )->GetInteger();

			if( pNewPlateInfo->aiCompoundTableIndex[ i ] > 0 )
			{
				int iNumCompoundJewelSlot = (int)m_vlpCompoundInfo.at( pNewPlateInfo->aiCompoundTableIndex[ i ] - 1 )->setHoleItemID.size();
				if( iNumCompoundJewelSlot != pNewPlateInfo->iNumSlot )
				{
					OutputDebug( "[EmblemFactory Error] Slot count mismatch between PlateTable and CompoundTable (PlateTableID: %d, recipe #%d, slots:%d, CompoundTableID: %d slots:%d)\n", 
								 iItem, i+1, pNewPlateInfo->iNumSlot,
								 pNewPlateInfo->aiCompoundTableIndex[ i ],
								 iNumCompoundJewelSlot );
					bResult = false;
				}
			}
		}

		pNewPlateInfo->bLostJewelWhenFailed = pPlateTable->GetFieldFromLablePtr( iItem, "_LostJewelWhenFailed" )->GetInteger() ? true : false;
		pNewPlateInfo->bLostPlateWhenFailed = pPlateTable->GetFieldFromLablePtr( iItem, "_LostPlateWhenFailed" )->GetInteger() ? true : false;
		pNewPlateInfo->iCompositionFailProb = pPlateTable->GetFieldFromLablePtr( iItem, "_CompositionFailProb" )->GetInteger();
		pNewPlateInfo->bAllowShowProb = pPlateTable->GetFieldFromLablePtr( iItem, "_AllowShowProb" )->GetInteger() ? true : false;
		pNewPlateInfo->bLostJewelWhenFailedProb = pPlateTable->GetFieldFromLablePtr( iItem, "_LostJewelWhenFailedProb" )->GetInteger() ? true : false;
		pNewPlateInfo->bLostPlateWhenFailedProb = pPlateTable->GetFieldFromLablePtr( iItem, "_LostPlateWhenFailedProb" )->GetInteger() ? true : false;
		pNewPlateInfo->iCost = pPlateTable->GetFieldFromLablePtr( iItem, "_Cost" )->GetInteger();

		m_vlpPlateInfo.push_back( pNewPlateInfo );
	}

	// Fetch Jewel table info
	//DNTableFileFormat* pJewelTable = new DNTableFileFormat( "R:/GameRes/Resource/Ext/JewelTable.ext" );
	DNTableFileFormat* pJewelTable = GetDNTable( CDnTableDB::TJEWEL );
	iNumItem = pJewelTable->GetItemCount();
	m_vlpJewelInfo.reserve( iNumItem );
	for( int iItem = 1; iItem <= iNumItem; ++iItem )
	{
		S_JEWEL_INFO* pNewJewelInfo = new S_JEWEL_INFO;

		pNewJewelInfo->iItemID = pJewelTable->GetFieldFromLablePtr( iItem, "_ItemID" )->GetInteger();
		pNewJewelInfo->iGrade = pJewelTable->GetFieldFromLablePtr( iItem, "_Grade" )->GetInteger();
		pNewJewelInfo->iColor = pJewelTable->GetFieldFromLablePtr( iItem, "_Color" )->GetInteger();
		pNewJewelInfo->iNeedUpgradeCount = pJewelTable->GetFieldFromLablePtr( iItem, "_NeedUpgradeCount" )->GetInteger();
		pNewJewelInfo->iNextGradeItemIndex = pJewelTable->GetFieldFromLablePtr( iItem, "_NextGradeItemIndex" )->GetInteger();
		pNewJewelInfo->iCost = pJewelTable->GetFieldFromLablePtr( iItem, "_Cost" )->GetInteger();

		m_vlpJewelInfo.push_back( pNewJewelInfo );
	}

	return bResult;
}


int CDnEmblemFactory::_ValidateUpgradeJewel( S_JEWEL_INFO* pJewelInfo, int iNumJewel )
{
	// TODO: Get item ID and check if grade is valid
	//pJewelInfo->iItemID
	int iResult = 0;

	if( pJewelInfo->iNeedUpgradeCount <= iNumJewel )
	{
		iResult = pJewelInfo->iNextGradeItemIndex;
	}

	return iResult;
}


int CDnEmblemFactory::_ValidateCompound( S_PLATE_INFO* pPlateInfo, const multiset<int>& setJewelItemID )
{
	int iResult = 0;

	// Get possible crafting combinations for this plate. (max 10
	for( int i = 0; i < NUM_MAX_PLATE_HOLE; ++i )
	{	
		int iCompoundIndex = pPlateInfo->aiCompoundTableIndex[ i ];
		if( iCompoundIndex <= 0 )
			continue;

		// Similarly, since table index starts from 1..
		S_EMBLEM_COMPOUND_INFO* pCompoundInfo = m_vlpCompoundInfo.at( iCompoundIndex-1 );
		if( pCompoundInfo->setHoleItemID == setJewelItemID )
		{
			iResult = iCompoundIndex;
			break;
		}
	}

	return iResult;
}


int CDnEmblemFactory::UpgradeJewel( int iJewelItemID, int iNumJewel, /*OUT*/ int* pCost, /*OUT*/ int* pNumUse )
{
	int iNewJewelItemID = 0;

	vector<S_JEWEL_INFO*>::iterator iter = find_if( m_vlpJewelInfo.begin(), m_vlpJewelInfo.end(),
													FindByItemID<S_JEWEL_INFO>(iJewelItemID) );
	if( m_vlpJewelInfo.end() == iter )
		return false;

	S_JEWEL_INFO* pJewelInfo = *iter;

	iNewJewelItemID = _ValidateUpgradeJewel( pJewelInfo, iNumJewel );
	if( iNewJewelItemID > 0 )
	{
		if( pCost )
			*pCost = pJewelInfo->iCost;

		if( pNumUse )
			*pNumUse = pJewelInfo->iNeedUpgradeCount;
	}

	return iNewJewelItemID;
}


int CDnEmblemFactory::Compound( int iPlateItemID, const multiset<int>& setJewelItemID, int* pCost )
{
	int iEmblemItemID = 0;

	// Since table index starts from 1..
	vector<S_PLATE_INFO*>::iterator iter = find_if( m_vlpPlateInfo.begin(), m_vlpPlateInfo.end(), 
													FindByItemID<S_PLATE_INFO>(iPlateItemID) );
	if( m_vlpPlateInfo.end() == iter )
		return false;

	S_PLATE_INFO* pPlateInfo = *iter;

	int iResult = _ValidateCompound( pPlateInfo, setJewelItemID );
	if( iResult > 0 )
	{
		// Table index starts from 1.. since list starts at 0,
		iEmblemItemID = m_vlpCompoundInfo.at( iResult-1 )->iResultItemID;

		_ASSERT( pCost && "Cost output parameter must be provided" );
		if( pCost )
			*pCost = pPlateInfo->iCost;
	}

	return iEmblemItemID;
}

// Data lookup functions
const CDnEmblemFactory::S_EMBLEM_COMPOUND_INFO* CDnEmblemFactory::GetEmblemCompoundInfo( int iIndex ) const
{
	const S_EMBLEM_COMPOUND_INFO* pResult = NULL;
	
	if( iIndex < (int)m_vlpCompoundInfo.size() )
		pResult = m_vlpCompoundInfo.at( iIndex );

	return pResult;
}


const CDnEmblemFactory::S_PLATE_INFO* CDnEmblemFactory::GetPlateInfoByIndex( int iIndex ) const
{
	const S_PLATE_INFO* pResult = NULL;

	if( iIndex < (int)m_vlpPlateInfo.size() )
		pResult = m_vlpPlateInfo.at( iIndex );

	return pResult;
}


const CDnEmblemFactory::S_PLATE_INFO* CDnEmblemFactory::GetPlateInfoByItemID( int iItemID ) const
{
	const S_PLATE_INFO* pResult = NULL;

	vector<S_PLATE_INFO*>::const_iterator iter = find_if( m_vlpPlateInfo.begin(), m_vlpPlateInfo.end(), 
													FindByItemID<S_PLATE_INFO>(iItemID) );
	if( m_vlpPlateInfo.end() != iter )
		pResult = *iter;

	return pResult;
}

const CDnEmblemFactory::S_JEWEL_INFO* CDnEmblemFactory::GetJewelInfoByIndex( int iIndex ) const
{
	const S_JEWEL_INFO* pResult = NULL;

	if( iIndex < (int)m_vlpJewelInfo.size() )
		pResult = m_vlpJewelInfo.at(iIndex);

	return pResult;
}


const CDnEmblemFactory::S_JEWEL_INFO* CDnEmblemFactory::GetJewelInfoByItemID( int iItemID ) const
{
	const S_JEWEL_INFO* pResult = NULL;

	vector<S_JEWEL_INFO*>::const_iterator iter = find_if( m_vlpJewelInfo.begin(), m_vlpJewelInfo.end(), 
													FindByItemID<S_JEWEL_INFO>(iItemID) );

	if( m_vlpJewelInfo.end() != iter )
		pResult = *iter;

	return pResult;
}