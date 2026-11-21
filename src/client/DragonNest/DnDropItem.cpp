#include "StdAfx.h"
#include "DnDropItem.h"
#include "DnWorld.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"
#include "DnTableDB.h"
#include "PerfCheck.h"
#include "GameSendPacket.h"
#include "TaskManager.h"
#include "DnEtcObject.h"
#include "DnInterfaceString.h"
#include "DnInterface.h"
#include "DnGameTask.h"
#include "NavigationMesh.h"
#include "DnUIString.h"
#include "EtPostProcessFilter.h"
#include "DnNameLinkMng.h"
#ifdef PRE_FIX_MEMOPT_EXT
#include "DnCommonUtil.h"
#endif

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CEtOctree<DnDropItemHandle> *CDnDropItem::s_pOctree = NULL;
float CEtOctreeNode<DnDropItemHandle>::s_fMinRadius = 500.0f;
int CDnDropItem::s_nFontIndex = INT_MAX;
bool CDnDropItem::s_bActive = false;
DWORD CDnDropItem::s_dwUniqueCount = 0;
bool CDnDropItem::s_bShowToolTip = true;
float CDnDropItem::s_fToolTipRange = 1000.f;
bool CDnDropItem::s_bShowDropItems = true;
DnDropItemHandle CDnDropItem::s_hLastNearItem;

#define DROPITEM_LIFETIME 180.f

std::vector<DnDropItemHandle> CDnDropItem::s_hVecPreLoadItemList;

DECL_DN_SMART_PTR_STATIC( CDnDropItem, 200 )

CDnDropItem::CDnDropItem( DWORD dwUniqueID, bool bProcess )

	: CDnUnknownRenderObject( bProcess )
{
	m_dwUniqueID = dwUniqueID;
	m_nItemID = -1;
	m_nOverlapCount = 1;
	m_nRandomSeed = 0;
	m_cOption = 0;

	m_pCurrentNode = NULL;

	if( bProcess ) InsertOctreeNode();
	m_bShowToolTip = false;
	m_Rank = ITEMRANK_D;
	m_dwItemNameColor = DN_INTERFACE::STRING::ITEM::RANK_2_COLOR( m_Rank );

	m_fDistance = 0.f;
	m_fLifeTime = DROPITEM_LIFETIME;
	m_fDisappearTime = 0.f;
	m_bDisappear = false;
	m_pCurCell = NULL;
	m_dwOwnerUniqueID = -1;

	m_bFocus = false;
}

CDnDropItem::~CDnDropItem()
{
	SAFE_RELEASE_SPTR( m_hObject );
	SAFE_RELEASE_SPTR( m_hOutline );
	RemoveOctreeNode();

	ReleaseSignalImp();
}

bool CDnDropItem::InitializeClass()
{
	if( !CDnWorld::GetInstance().IsActive() ) return false;
	/*
	CDnWorldGrid *pGrid = (CDnWorldGrid*)CDnWorld::GetInstance().GetGrid();
	float fSize = max( pGrid->GetGridX() * pGrid->GetGridWidth() * 100.f, pGrid->GetGridY() * pGrid->GetGridHeight() * 100.f );
	if( CDnWorld::IsActive() ) {
		float fMaxMultiply = 0.f;
		for( DWORD i=0; i<CDnWorld::GetInstance().GetGrid()->GetActiveSectorCount(); i++ ) {
			float fTemp = CDnWorld::GetInstance().GetGrid()->GetActiveSector(i)->GetHeightMultiply();
			if( fTemp > fMaxMultiply ) fMaxMultiply = fTemp;
		}e
		fSize = max( fSize, fMaxMultiply * 65535.f );
	}
	*/
	s_dwUniqueCount = 0;

	float fCenter, fSize;
	CDnWorld::GetInstance().CalcWorldSize( fCenter, fSize );

	SAFE_DELETE( s_pOctree );
	s_pOctree = new CEtOctree<DnDropItemHandle>;
	s_pOctree->Initialize( EtVector3( 0.f, fCenter, 0.f ), fSize );

	// Preload gold
	int nCoinArray[] = { 1, 50, 100, 500, 2500, 10000, 30000, 100000 };
	int nCount = sizeof(nCoinArray) / sizeof(int);
	for( int i=0; i<nCount; i++ ) {
		CDnDropItem *pDropItem = new CDnDropItem( -1, false );
		pDropItem->Initialize( EtVector3( 0.f, 0.f, 0.f ), 0, 0, 0, nCoinArray[i], 0, true );
		s_hVecPreLoadItemList.push_back( pDropItem->GetMySmartPtr() );
	}	

	s_bActive = true;
	return true;
}

void CDnDropItem::ProcessClass( LOCAL_TIME LocalTime, float fDelta )
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	for( DWORD i=0; i<s_pVecProcessList.size(); i++ ) {
		s_pVecProcessList[i]->SyncClassTime( LocalTime );
		s_pVecProcessList[i]->Process( LocalTime, fDelta );

		SSphere Sphere;
		s_pVecProcessList[i]->GetBoundingSphere( Sphere );
		s_pVecProcessList[i]->m_pCurrentNode = s_pOctree->Update( s_pVecProcessList[i]->GetMySmartPtr(), Sphere, s_pVecProcessList[i]->m_pCurrentNode );

		if( s_pVecProcessList[i]->IsDestroy() ) {
			s_pVecProcessList[i]->Release();
			i--;
		}
	}

	// Temporarily invert color for nearby items.
	if( s_bActive && CDnActor::s_hLocalActor ) {		

		DnDropItemHandle hNearItem;
		float fDistance = FLT_MAX;
		DNVector(DnDropItemHandle) hVecList;
		ScanItem( CDnActor::s_hLocalActor->GetMatEx()->m_vPosition, 60.f, hVecList );
		for( DWORD i=0; i<hVecList.size(); i++ ) {
			float fTemp = EtVec3LengthSq( &EtVector3( CDnActor::s_hLocalActor->GetMatEx()->m_vPosition - hVecList[i]->GetObjectCross()->m_vPosition ) );
			if( fTemp < fDistance ) {
				fDistance = fTemp;
				hNearItem = hVecList[i];
			}
		}
		if( hNearItem ) {
			hNearItem->SetFocus();
		}
		else {
			if( s_hLastNearItem )
				s_hLastNearItem->SetFocus( false );
		}
	}
}

void CDnDropItem::ReleaseClass()
{
	s_bActive = false;
	SAFE_RELEASE_SPTRVEC( s_hVecPreLoadItemList );
	SAFE_DELETE( s_pOctree );
	DeleteAllObject();
}

CDnDropItem* CDnDropItem::DropItem( EtVector3 &vPos, DWORD dwUniqueID, int nItemID, int nSeed, char cOption, int nOverlapCount, int nRotate, DWORD dwOwnerUniqueID )
{
	CDnDropItem *pDropItem = new CDnDropItem( dwUniqueID, true );
	pDropItem->InitializeSession( CTaskManager::GetInstance().GetTask( "GameTask" ) );
	pDropItem->Initialize( vPos, nItemID, nSeed, cOption, nOverlapCount, nRotate );
	pDropItem->SetOwnerUniqueID( dwOwnerUniqueID );
	OutputDebug( "DropItem : %d, %d, %d - %.2f, %.2f, %.2f (%d)\n", dwUniqueID, nItemID, nSeed, vPos.x, vPos.y, vPos.z, nRotate );

	return pDropItem;
}

CDnDropItem* CDnDropItem::DropItem( const TDropItemSync& DropItem )
{
	EtVector3 vec;
	vec.x = DropItem.fpos[0];
	vec.y = DropItem.fpos[1];
	vec.z = DropItem.fpos[2];

	CDnDropItem* pDropItem = CDnDropItem::DropItem( vec, DropItem.nSessionID, DropItem.nItemID, DropItem.nRandomSeed, -1, DropItem.nCount, DropItem.nRotate );
	if( !pDropItem )
		return NULL;

	pDropItem->m_fLifeTime	= DropItem.uiLifeTime/1000.f;
	pDropItem->m_fDistance	= DropItem.fDistance;

	return pDropItem;
}

int CDnDropItem::ScanItem( EtVector3 &vPos, float fRadius, DNVector(DnDropItemHandle) &VecList )
{
	if( !s_pOctree ) return 0;
	SSphere Sphere;
	Sphere.Center = vPos;
	Sphere.fRadius = fRadius;
	s_pOctree->Pick( Sphere, VecList );
	return (int)VecList.size();
}

DnDropItemHandle CDnDropItem::FindItemFromUniqueID( DWORD dwUniqueID )
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	for( DWORD i=0; i<s_pVecProcessList.size(); i++ ) {
		if( s_pVecProcessList[i]->GetUniqueID() == dwUniqueID ) return s_pVecProcessList[i]->GetMySmartPtr();
	}
	return CDnDropItem::Identity();
}

bool CDnDropItem::PreInitializeItem( int nItemID )
{
	for( DWORD i=0; i<s_hVecPreLoadItemList.size(); i++ ) {
		if( s_hVecPreLoadItemList[i]->GetItemID() == nItemID ) return true;
	}

	// Since this increases the render object list, it hurts performance.
	// Later it might not be bad to load EtSkin, EtAni, and EtActionBase separately and hold them as handles.
	CDnDropItem *pDropItem = new CDnDropItem( -1, false );
	if( pDropItem->Initialize( EtVector3( 0.f, 0.f, 0.f ), nItemID, 0, 0, 1, 0, true ) == false ) {
		SAFE_DELETE( pDropItem );
		return false; 
	}
	pDropItem->Enable( false );
	s_hVecPreLoadItemList.push_back( pDropItem->GetMySmartPtr() );

	return true;
}

void CDnDropItem::CalcCoinLump( int nCoin, int nMin, int nMax, std::vector<int> &nVecResult )
{
	int nResult = ( nMax - nMin ) + 1;
	if( nResult <= 0 ) nResult = 1;
	nResult = nMin + _rand()%nResult;

	if( nCoin < nResult ) nResult = nCoin;

	int nRemainder = nCoin;
	int nValue;
	nVecResult.clear();
	for( int i=0; i<nResult; i++ ) {
		if( i == nResult - 1 ) nValue = nRemainder;
		else {
			nValue = ( _rand() % nRemainder ) + 1;
			if( nValue > nRemainder ) nValue = nRemainder;
		}
		nRemainder -= nValue;
		nVecResult.push_back( nValue );
		if( nRemainder == 0 ) break;
	}
}

void CDnDropItem::CalcDropItemList( int nItemDropTableID, std::vector<CDnItem::DropItemStruct> &VecList, bool bIncreaseUniqueID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMDROP );
	if( !pSox->IsExistItem( nItemDropTableID ) ) return;
	CDnItem::DropItemStruct Struct;

	// Calculate money
	int nMin = pSox->GetFieldFromLablePtr( nItemDropTableID, "_GoldMin" )->GetInteger();
	int nMax = pSox->GetFieldFromLablePtr( nItemDropTableID, "_GoldMax" )->GetInteger();
	int nProb = pSox->GetFieldFromLablePtr( nItemDropTableID, "_GoldProb" )->GetInteger();
	if( _rand()%1000000000 < nProb ) {
		int nLumpMin = pSox->GetFieldFromLablePtr( nItemDropTableID, "_GoldLumpMin" )->GetInteger();
		int nLumpMax = pSox->GetFieldFromLablePtr( nItemDropTableID, "_GoldLumpMax" )->GetInteger();
		int nAmount = ( nMax - nMin ) + 1;
		if( nAmount <= 0 ) nAmount = 1;
		int nCoin = nMin + _rand()%nAmount;
		std::vector<int> nVecLumpCoin;
		CalcCoinLump( nCoin, nLumpMin, nLumpMax, nVecLumpCoin );

		for( DWORD i=0; i<nVecLumpCoin.size(); i++ ) {
			Struct.nItemID = 0;
			Struct.nSeed = abs( _rand() );
			Struct.nCount = nVecLumpCoin[i];
			Struct.dwUniqueID = ( bIncreaseUniqueID ) ? CDnDropItem::s_dwUniqueCount++ : 0;

			VecList.push_back( Struct );
		}
	}

	int nDepth = ITEMDROP_DEPTH;
	CalcDropItems( nItemDropTableID, VecList, nDepth, bIncreaseUniqueID );
}

void CDnDropItem::CalcDropItemList( int nDifficulty, int nItemDropGroupTableID, std::vector<CDnItem::DropItemStruct> &VecList, bool bIncreaseUniqueID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMDROPGROUP );
	std::vector<int> nVecList;
	pSox->GetItemIDListFromField( "_DropGroupID", nItemDropGroupTableID, nVecList );
	if( nVecList.empty() ) return;

	if( nDifficulty == -1 ) {
		CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
		if( !pTask ) nDifficulty = 0;
		else nDifficulty = pTask->GetStageConstructionLevel();
	}

	for( DWORD i=0; i<nVecList.size(); i++ ) {
		int nItemID = nVecList[i];
		if( pSox->GetFieldFromLablePtr( nItemID, "_Difficulty" )->GetInteger() != nDifficulty ) continue;
		INT64 nProb = (INT64)( pSox->GetFieldFromLablePtr( nItemID, "_Expectation" )->GetFloat() * 1000000000 );
		while( nProb > 0 ) {
			if( _rand()%1000000000 < nProb ) {
				int nItemTableID = pSox->GetFieldFromLablePtr( nItemID, "_DropID" )->GetInteger();
				if( nItemTableID < 1 ) continue;
				CDnDropItem::CalcDropItemList( nItemTableID, VecList, bIncreaseUniqueID );
			}
			nProb -= 1000000000;
		}
	}
}

namespace DropItemNameSpace {
	struct DropTempStruct {
		bool bIsGroup;
		int nItemID;
		int nCount;
		int nOffset;
	};
};

void CDnDropItem::CalcDropItems( int nDropItemTableID, std::vector<CDnItem::DropItemStruct> &VecResult, int &nDepth, bool bIncreaseUniqueID )
{
	nDepth -= 1;
	if( nDepth < 0 ) return;

	using namespace DropItemNameSpace;
	// First get the index values containing table values
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMDROP );
	if( !pSox->IsExistItem( nDropItemTableID ) ) return;

	std::vector<DropTempStruct> VecList;

	char szLabel[64];
	int nOffset = 0;
	DropTempStruct Struct;
	for( DWORD i=0; i<20; i++ ) {
		sprintf_s( szLabel, "_IsGroup%d", i + 1 );
		bool bGroup = false;
		DNTableCell* field = pSox->GetFieldFromLablePtr( nDropItemTableID, szLabel );
		if (field)
			bGroup = ( field->GetInteger() == 1 ) ? true : false;

		sprintf_s( szLabel, "_Item%dIndex", i + 1 );
		int nIndex = -1;
		field = pSox->GetFieldFromLablePtr( nDropItemTableID, szLabel );
		if (field)
			nIndex = field->GetInteger();
		if( nIndex < 1 ) continue;

		sprintf_s( szLabel, "_Item%dProb", i + 1 );
		field = pSox->GetFieldFromLablePtr( nDropItemTableID, szLabel );
		int nProb = -1;
		if (field)
			nProb = field->GetInteger();
		if( nProb <= 0 ) continue;

		sprintf_s( szLabel, "_Item%dInfo", i + 1 );
		field = pSox->GetFieldFromLablePtr( nDropItemTableID, szLabel );
		int nInfo = -1;
		if (field)
			nInfo = field->GetInteger();
		//if( !bGroup && nInfo < 1 ) continue;
		if( !bGroup ){
			if( nInfo < 1 ) continue;
			DNTableFileFormat*  pItemSox = GetDNTable( CDnTableDB::TITEM );
			if( !pItemSox || !pItemSox->IsExistItem( nIndex ) ) continue;
		}

		nOffset += nProb;

		Struct.bIsGroup = bGroup;
		Struct.nItemID = nIndex;
		Struct.nCount = nInfo;
		Struct.nOffset = nOffset;
		VecList.push_back( Struct );
	}
	int nSeed = _rand()%1000000000;

	int nPrevOffset = 0;
	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( nSeed >= nPrevOffset && nSeed < VecList[i].nOffset ) {
			if( VecList[i].bIsGroup ) CalcDropItems( VecList[i].nItemID, VecResult, nDepth, bIncreaseUniqueID );
			else {
				CDnItem::DropItemStruct Result;
				Result.nItemID = VecList[i].nItemID;
				Result.nSeed = abs( _rand() );
				Result.nCount = VecList[i].nCount;
				Result.cOption = 0;

				Result.dwUniqueID = ( bIncreaseUniqueID ) ? CDnDropItem::s_dwUniqueCount++ : 0;

				VecResult.push_back( Result );
			}
			break;
		}
		nPrevOffset = VecList[i].nOffset;
	}
}


bool CDnDropItem::Initialize( EtVector3 &vPos, int nItemID, int nRandomSeed, char cOption, int nCount, int nRotate, bool bPreInitialize )
{
	m_nOverlapCount = nCount;
	m_nItemID = nItemID;
	m_nRandomSeed = nRandomSeed;
	m_cOption = cOption;
	m_matExWorld.m_vPosition = vPos;

	if( nItemID == 0 ) {
		std::string szCoinName;
		std::string szCoinAniName;
		if( nCount < 50 ) szCoinName = "Coin_Copper_Little", szCoinAniName = "Coin";
		else if( nCount >= 50 && nCount < 100 ) szCoinName = "Coin_Copper_Much", szCoinAniName = "CoinSack";
		else if( nCount >= 100 && nCount < 500 ) szCoinName = "Coin_Silver_Little", szCoinAniName = "Coin";
		else if( nCount >= 500 && nCount < 2500 ) szCoinName = "Coin_Silver_Much", szCoinAniName = "Coin";
		else if( nCount >= 2500 && nCount < 10000 ) szCoinName = "Coin_Silver_VeryMuch", szCoinAniName = "CoinSack";
		else if( nCount >= 10000 && nCount < 30000 ) szCoinName = "Coin_Gold_Little", szCoinAniName = "Coin";
		else if( nCount >= 30000 && nCount < 100000 ) szCoinName = "Coin_Gold_Much", szCoinAniName = "CoinSack";
		else if( nCount >= 100000 ) szCoinName = "Coin_Gold_VeryMuch", szCoinAniName = "CoinBox";

		char szSknName[32];
		char szAniName[32];
		char szActName[32];
		sprintf_s( szSknName, "%s.skn", szCoinName.c_str() );
		sprintf_s( szAniName, "%s.ani", szCoinAniName.c_str() );
		sprintf_s( szActName, "%s.act", szCoinName.c_str() );

		if( CDnActionRenderBase::Initialize( szSknName, szAniName, szActName ) == false ) return false;

		int nGold = nCount / 10000;
		int nSilver = ( nCount - ( nGold * 10000 ) ) / 100;
		int nCopper = nCount % 100;
		WCHAR wszCoinTemp[32] = { 0, };

		if( nGold > 0 ) {
			m_szItemName += _itow( nGold, wszCoinTemp, 10 );
			m_szItemName += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 507 );
			m_szItemName += L" ";
		}
		if( nSilver > 0 ) {
			m_szItemName += _itow( nSilver, wszCoinTemp, 10 );
			m_szItemName += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 508 );
			m_szItemName += L" ";
		}
		if( nCopper > 0 ) {
			m_szItemName += _itow( nCopper, wszCoinTemp, 10 );
			m_szItemName += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 509 );
			m_szItemName += L"  ";
		}

		m_Rank = ITEMRANK_D;
		m_dwItemNameColor = D3DCOLOR_ARGB( 255, 236, 220, 0 );

	}
	else {
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
		if( !pSox->IsExistItem( nItemID ) ) return false;

		char *szSknName;
		char *szAniName;
		char *szActName;

#ifdef PRE_FIX_MEMOPT_EXT
		DNTableFileFormat*  pFileNameSox = GetDNTable( CDnTableDB::TFILE );
		if (!pFileNameSox) return false;

		szSknName = CommonUtil::GetFileNameFromFileEXT(pSox, nItemID, "_DropSkinName", pFileNameSox);
		szAniName = CommonUtil::GetFileNameFromFileEXT(pSox, nItemID, "_DropAniName", pFileNameSox);
		szActName = CommonUtil::GetFileNameFromFileEXT(pSox, nItemID, "_DropActName", pFileNameSox);
#else
		szSknName = pSox->GetFieldFromLablePtr( nItemID, "_DropSkinName" )->GetString();
		szAniName = pSox->GetFieldFromLablePtr( nItemID, "_DropAniName" )->GetString();
		szActName = pSox->GetFieldFromLablePtr( nItemID, "_DropActName" )->GetString();
#endif
		
		if( CDnActionRenderBase::Initialize( szSknName, szAniName, szActName ) == false ) return false;

		int nNameID = pSox->GetFieldFromLablePtr( nItemID, "_NameID" )->GetInteger();
		char *szParam = pSox->GetFieldFromLablePtr( nItemID, "_NameIDParam" )->GetString();
		MakeUIStringUseVariableParam( m_szItemName, nNameID, szParam );
		m_Rank = (eItemRank)pSox->GetFieldFromLablePtr( nItemID, "_Rank" )->GetInteger();
		m_dwItemNameColor = DN_INTERFACE::STRING::ITEM::RANK_2_COLOR( m_Rank );
	}

	if( m_hObject ) {
		if( !bPreInitialize ) {
			m_matExWorld.RotateYaw( (float)nRotate );

			m_hObject->SetCalcPositionFlag( CALC_POSITION_Y );
			if( m_hObject->GetSkinInstance() )
				m_hObject->GetSkinInstance()->SetRenderAlphaTwoPass( true );

			char szTemp[16];
			int nDropActionCount = 0;
			for( int i=0; ; i++ ) {
				sprintf_s( szTemp, "Drop_%c", 'a' + i );
				if( IsExistAction( szTemp ) == false ) break;
				nDropActionCount++;
			}
			if( nDropActionCount > 0 ) {
				sprintf_s( szTemp, "Drop_%c", 'a' + _rand()%nDropActionCount );
				SetActionQueue( szTemp, 0, 0.f );
			}

			m_hObject->Update( m_matExWorld );
			m_hObject->EnableShadowCast( true );
			m_hObject->EnableShadowReceive( false );

#if defined(PRE_FIX_46730)
			if (m_nRandomSeed == 0)
				m_fDistance = 0;
			else
				m_fDistance = 50.f + ( 10 * ( m_nRandomSeed % 11 ) );
#else
			m_fDistance = 50.f + ( 10 * ( m_nRandomSeed % 11 ) );
#endif // PRE_FIX_46730

			int nIndex = GetElementIndex( "Disappear" );
			if( nIndex != -1 ) {
				m_fDisappearTime = GetElement( nIndex )->dwLength / 60.f;
			}
			else m_bDisappear = true;

			if( !s_bShowDropItems )
				m_hObject->ShowObject( s_bShowDropItems );
		}
		else {
			m_hObject->ShowObject( false );
			m_hObject->EnableObject( false );
		}
	}
	return true;
}

void CDnDropItem::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnActionRenderBase::Process( LocalTime, fDelta );

	CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask( "GameTask" ));

	if( m_fDistance > 0.f ) 
	{
		float fValue = 100.f * fDelta;
		m_fDistance -= fValue;
		if( m_fDistance <= 0.f ) {
			fValue += m_fDistance;
			m_fDistance = 0.f;
		}
		switch( pGameTask->GetGameTaskType() ) {
			case GameTaskType::Normal:
			case GameTaskType::DarkLair:
				break;
			case GameTaskType::PvP:
				fValue = 0.f;
				break;
		}

		NavigationMesh *pNavMesh = INSTANCE(CDnWorld).GetNavMesh( m_matExWorld.m_vPosition );
		if( pNavMesh == NULL ) {
			ProcessAttributeMovement( fValue );
		}
		else {
			if( m_pCurCell == NULL ) {
				m_pCurCell = pNavMesh->FindClosestCell( m_matExWorld.m_vPosition );
				if( ( m_pCurCell == NULL ) || ( !m_pCurCell->IsPointInCellCollumn( m_matExWorld.m_vPosition ) ) )
				{
					m_pCurCell = NULL;
					ProcessAttributeMovement( fValue );
				}
			}
			if( m_pCurCell ) {
				EtVector3 vPrevPos = m_matExWorld.m_vPosition;
				m_matExWorld.MoveLocalZAxis( fValue );
				int nSide = -1;
				NavigationCell *pLastCell = NULL;
				m_pCurCell->FindLastCollision( vPrevPos, m_matExWorld.m_vPosition, &pLastCell, nSide );
				if( nSide != -1 )
				{
					if( pLastCell->Link( nSide ) == NULL )
					{
						EtVector2 vMoveDir2D( m_matExWorld.m_vPosition.x - vPrevPos.x, m_matExWorld.m_vPosition.z - vPrevPos.z );
						float fMoveLength = EtVec2Length( &vMoveDir2D );
						vMoveDir2D /= fMoveLength;
						EtVector2 vWallDir2D = pLastCell->Side( nSide )->EndPointB() - pLastCell->Side( nSide )->EndPointA();
						EtVec2Normalize( &vWallDir2D, &vWallDir2D );
						fMoveLength *= EtVec2Dot( &vWallDir2D, &vMoveDir2D );
						m_matExWorld.m_vPosition.x = vPrevPos.x + fMoveLength * vWallDir2D.x;
						m_matExWorld.m_vPosition.z = vPrevPos.z + fMoveLength * vWallDir2D.y;

						int nNewSide = -1;
						m_pCurCell->FindLastCollision( vPrevPos, m_matExWorld.m_vPosition, &pLastCell, nNewSide );
						if( nNewSide != -1 )
						{
							if( pLastCell->Link( nNewSide ) )
							{
								NavigationCell *pNewCell;
								pNewCell = pLastCell->Link( nNewSide );
								if( pNewCell->IsPointInCellCollumn( m_matExWorld.m_vPosition ) )
								{
									m_pCurCell = pNewCell;
								}
								else
								{
									m_matExWorld.m_vPosition.x = vPrevPos.x;
									m_matExWorld.m_vPosition.z = vPrevPos.z;
								}
							}
							else if( !m_pCurCell->IsPointInCellCollumn( m_matExWorld.m_vPosition ) )
							{
								m_matExWorld.m_vPosition.x = vPrevPos.x;
								m_matExWorld.m_vPosition.z = vPrevPos.z;
							}
						}
					}
					else
					{
						m_pCurCell = pLastCell->Link( nSide );
					}
				}

				if( m_pCurCell->GetType() == NavigationCell::CT_PROP )
					m_matExWorld.m_vPosition.y = m_pCurCell->GetPlane()->SolveForY( m_matExWorld.m_vPosition.x, m_matExWorld.m_vPosition.z );
				else
					m_matExWorld.m_vPosition.y = CDnWorld::GetInstance().GetHeight( m_matExWorld.m_vPosition );
			}
		}
	}
	
	CDnActionSignalImp::Process( LocalTime, fDelta );

	ProcessToolTip();
	ProcessLifetime( LocalTime, fDelta );
}

void CDnDropItem::ProcessAttributeMovement( float fDistance )
{
	EtVector3 vPrevPos = m_matExWorld.m_vPosition;

	m_matExWorld.MoveLocalZAxis( fDistance );
	if( CDnWorld::GetInstance().GetAttribute( m_matExWorld.m_vPosition ) != 0 ) {
		m_matExWorld.m_vPosition = vPrevPos;
	}
	m_matExWorld.m_vPosition.y = CDnWorld::GetInstance().GetHeight( m_matExWorld.m_vPosition );
}

void CDnDropItem::ProcessToolTip()
{
	bool bShow = false;

	if( s_bShowToolTip ) {
		if( CDnCamera::GetActiveCamera() ) {
			EtVector3 vPos;
			if( CDnCamera::GetActiveCamera()->GetCameraType() == CDnCamera::PlayerCamera && CDnActor::s_hLocalActor ) {
				vPos = *CDnActor::s_hLocalActor->GetPosition();
			}
			else {
				vPos = CDnCamera::GetActiveCamera()->GetMatEx()->m_vPosition;
			}

			bShow = ( EtVec3Length( &EtVector3( vPos - *GetPosition() ) ) < s_fToolTipRange ) ? true : false;
			if( strstr( GetCurrentAction(), "Drop" ) == NULL ) bShow = false;
		}
	}

	ShowToolTip( bShow );
}

void CDnDropItem::ProcessLifetime( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_Rank >= ITEMRANK_B ) return;
	if( m_fLifeTime <= 0.f ) return;
	CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask( "GameTask" ));
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm )
		return;

	m_fLifeTime -= fDelta;
	if( !m_bDisappear && m_fLifeTime <= m_fDisappearTime ) {
		m_bDisappear = true;
		SetActionQueue( "Disappear", 0, 20.f );
	}
	if( m_fLifeTime <= 0.f ) {
		SetDestroy();
	}
}


void CDnDropItem::SyncClassTime( LOCAL_TIME LocalTime )
{
	CDnActionBase::m_LocalTime = LocalTime;
}

void CDnDropItem::GetBoundingSphere( SSphere &Sphere, bool bActorSize/* = false*/ )
{
	/*
	if( m_hObject ) {
		m_hObject->GetBoundingSphere( Sphere );
	}
	else {
		Sphere.Center = m_matExWorld.m_vPosition;
		Sphere.fRadius = 10.f;
	}
	*/
	Sphere.Center = m_matExWorld.m_vPosition;
	Sphere.fRadius = 30.f;
}

void CDnDropItem::GetBoundingBox( SAABox &Box )
{
	if( m_hObject ) {
		m_hObject->GetBoundingBox( Box );
	}
	else {
		Box.Min = m_matExWorld.m_vPosition + EtVector3( -50.f, -50.f, -50.f );
		Box.Max = m_matExWorld.m_vPosition + EtVector3( 50.f, 50.f, 50.f );
	}
}

void CDnDropItem::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	CDnActionSignalImp::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
	switch( Type ) {
		case STE_Destroy:
			SetDestroy();
			break;
		case STE_AlphaBlending:
			{
				if( !m_hObject ) break;
				AlphaBlendingStruct *pStruct = (AlphaBlendingStruct *)pPtr;
				float fValue = 1.f / ( SignalEndTime - SignalStartTime - 16.6666f ) * ( LocalTime - SignalStartTime );
				fValue = EtClamp( fValue, 0.0f, 1.0f );
				float fCurAlpha = pStruct->fStartAlpha + ( ( pStruct->fEndAlpha - pStruct->fStartAlpha ) * fValue );
				fCurAlpha = EtClamp( fCurAlpha, 0.0f, 1.0f );
				m_hObject->SetObjectAlpha( fCurAlpha );
				if( fCurAlpha < 0.5f && m_hObject->IsShadowCast() )
					m_hObject->EnableShadowCast( false );
			}
			break;
		case STE_ObjectVisible:
			{
				ObjectVisibleStruct *pStruct = (ObjectVisibleStruct *)pPtr;
				m_hObject->ShowObject( ( pStruct->bShow == TRUE ) ? true : false );
			}
			break;

	}
}

void CDnDropItem::InsertOctreeNode()
{
	if( s_pOctree ) {
		SSphere Sphere;
		GetBoundingSphere( Sphere );
		m_pCurrentNode = s_pOctree->Insert( GetMySmartPtr(), Sphere );
	}
}

void CDnDropItem::RemoveOctreeNode()
{
	if( s_pOctree && m_pCurrentNode ) {
		s_pOctree->Remove( GetMySmartPtr(), m_pCurrentNode );
	}
}

void CDnDropItem::RenderCustom( float fElapsedTime )
{
	if( !m_bShowToolTip ) return;
	m_bShowToolTip = false;
	DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
	if( !hCamera ) return;
	if( !m_hObject ) return;

	EtMatrix *pmat = hCamera->GetCameraHandle()->GetViewProjMat();
	SAABox Box;
	GetBoundingBox( Box );
	EtVector3 vTemp = m_matExWorld.m_vPosition;
	vTemp.y = m_hObject->GetBoneTransMat(0)->_42;
	vTemp.y += Box.Min.y + ( ( Box.Max.y - Box.Min.y ) / 2.f );

	m_fCustomRenderDepth = EtVec3Dot(&vTemp, &hCamera->GetMatEx()->m_vZAxis);

	EtVec3TransformCoord( &vTemp, &vTemp, pmat );
	if( vTemp.z > 1.0f ) return;

	EtVector2 vPos;
	vPos.x = ( vTemp.x + 1.f ) / 2.f;
	vPos.y = 1.f - ( ( vTemp.y + 1.f ) / 2.f );

	SUICoord Coord, CoordRect;
	CEtFontMng::GetInstance().CalcTextRect( s_nFontIndex, 12, m_szItemName.c_str(), DT_VCENTER | DT_CENTER, Coord, -1 );	
	Coord.fX = vPos.x - ( Coord.fWidth / 2.f );
	Coord.fY = vPos.y;

	CoordRect = Coord;
	CoordRect.fWidth += 0.01f;
	CoordRect.fHeight += 0.01f;
	CoordRect.fX -= 0.005f;
	CoordRect.fY -= 0.005f;
	
	//CEtSprite::GetInstance().Flush();
	CEtSprite::GetInstance().DrawRect( CoordRect, ( m_bFocus ) ? 0x884682F0 : 0x88000000 );

	SFontDrawEffectInfo Info;
	Info.dwFontColor = m_dwItemNameColor;
	CEtFontMng::GetInstance().DrawTextW( s_nFontIndex, 12, m_szItemName.c_str(), DT_VCENTER | DT_CENTER, Coord, -1, Info );

	if( m_dwOwnerUniqueID != -1 ) {
		DnActorHandle hOwner = CDnActor::FindActorFromUniqueID( m_dwOwnerUniqueID );
		if( hOwner ) {
			CEtFontMng::GetInstance().CalcTextRect( s_nFontIndex, 12, hOwner->GetName(), DT_VCENTER | DT_CENTER, Coord, -1 );
			Coord.fX = vPos.x - ( Coord.fWidth / 2.f );
			Coord.fY = vPos.y;
			Coord.fY -= ( Coord.fHeight * 1.2f );

			CoordRect = Coord;
			CoordRect.fWidth += 0.01f;
			CoordRect.fHeight += 0.01f;
			CoordRect.fX -= 0.005f;
			CoordRect.fY -= 0.005f;

			Info.dwFontColor = D3DCOLOR_ARGB( 255, 20, 250, 20 );
			CEtSprite::GetInstance().DrawRect( CoordRect, ( m_bFocus ) ? 0x884682F0 : 0x88000000 );
			CEtFontMng::GetInstance().DrawTextW( s_nFontIndex, 12, hOwner->GetName(), DT_VCENTER | DT_CENTER, Coord, -1, Info );
		}
	}

	m_bFocus = false;
}

void CDnDropItem::Send( DWORD dwProtocol, DnActorHandle hActor, CMemoryStream *pStream, ClientSessionTypeEnum SessionType )
{
}

void CDnDropItem::Send( DWORD dwProtocol, CMemoryStream *pStream, ClientSessionTypeEnum SessionType, bool bImmediate )
{
}

void CDnDropItem::AddCountString(std::wstring& resultString)
{
	if (m_nOverlapCount > 1)
	{
		std::wstring countStr = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 607), m_nOverlapCount); // UISTRING : x %d
		resultString = FormatW(L"%s %s", resultString.c_str(), countStr.c_str());
	}
}

void CDnDropItem::OnDispatchMessage( DWORD dwProtocol, BYTE *pPacket )
{
	switch( dwProtocol ) {
		case DPT_PICKUP:
			{
				CPacketCompressStream Stream(pPacket, 32);
				char cLootType;
				DWORD dwUniqueID = 0;

				if( GetItemID() != 0 ) {
					bool bNameLink = false;
					Stream.Read(&bNameLink, sizeof(bool));

					TItem pickupItemInfo;
					memset(&pickupItemInfo, 0, sizeof(TItem));
					pickupItemInfo.nItemID = m_nItemID;

					if (bNameLink)
					{
						Stream.Read(&pickupItemInfo.cLevel, sizeof(char));
						Stream.Read(&pickupItemInfo.nRandomSeed, sizeof(int));
						Stream.Read(&pickupItemInfo.wDur, sizeof(USHORT));
						Stream.Read(&pickupItemInfo.cPotential, sizeof(char));
						Stream.Read(&pickupItemInfo.cOption, sizeof(char));
						Stream.Read(&pickupItemInfo.cSealCount, sizeof(char));
					}

					Stream.Read( &cLootType, sizeof(char) );

					eChatType chatType = CHATTYPE_NORMAL;
					if (CDnPartyTask::IsActive() && CDnPartyTask::GetInstance().GetPartyCount() > 1 && CDnPartyTask::GetInstance().IsSingleToPartyByGMTrace() == false)
						chatType = CHATTYPE_PARTY;

					switch( cLootType ) {
						case ITEMLOOTRULE_NONE:
							{
								if (CDnPartyTask::GetInstance().GetPartyCount() == 1)
								{
									std::wstring msgStr, tempStr, itemStr;
									Stream.Read( &dwUniqueID, sizeof(DWORD) );
									tempStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3548 );	// UISTRING : Obtained the item.
									GetInterface().GetNameLinkMng()->MakeNameLinkString_Item(itemStr, pickupItemInfo);
									msgStr = FormatW(L"%s %s", tempStr.c_str(), itemStr.c_str());
									AddCountString(msgStr);
									GetInterface().AddChatMessage( chatType, L"", msgStr.c_str(), false );
								}
							}
							break;

						case ITEMLOOTRULE_RANDOM:
						case ITEMLOOTRULE_OWNER:
						case ITEMLOOTRULE_INORDER:
							{
								Stream.Read( &dwUniqueID, sizeof(DWORD) );
								DnActorHandle hActor = CDnActor::FindActorFromUniqueID( dwUniqueID );
								if( !hActor ) break;

								std::wstring msgStr, tempStr, itemStr;
								if (dwUniqueID != CDnActor::s_hLocalActor->GetUniqueID())
									tempStr = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3547 ), hActor->GetName());	// UISTRING : %s acquired an item.
								else
									tempStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3548 );	// UISTRING : Obtained the item.
								GetInterface().GetNameLinkMng()->MakeNameLinkString_Item(itemStr, pickupItemInfo);
								msgStr = FormatW(L"%s %s", tempStr.c_str(), itemStr.c_str());
								AddCountString(msgStr);
								GetInterface().AddChatMessage( chatType, L"", msgStr.c_str(), false );
							}
							break;
						case ITEMLOOTRULE_LEADER:
							{
								std::wstring msgStr, itemStr;
								GetInterface().GetNameLinkMng()->MakeNameLinkString_Item(itemStr, pickupItemInfo);
								msgStr = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100052 ), itemStr.c_str() );	// UISTRING : The party leader acquired the item. [%s]
								AddCountString(msgStr);

								GetInterface().AddChatMessage( chatType, L"", msgStr.c_str(), false );
							}
							break;
						default:
							break;
					}
				}

				if( IsExistAction( "PickUp" ) ) {
					RemoveOctreeNode();
					SetActionQueue( "PickUp" );
				}
				else SetDestroy();
			}
			break;
		case DPT_NOPICKUP:
			SetActionQueue( "NoPickup" );
			break;
		case DPT_PICKINSTANT:
			{		

				DWORD dwUniqueID;
				int nItemID, nSeed;

				CMemoryStream Stream( pPacket, 16 );

				Stream.Read( &dwUniqueID, sizeof(DWORD) );
				Stream.Read( &nItemID, sizeof(int) );
				Stream.Read( &nSeed, sizeof(int) );

				CDnItem *pItem = CDnItem::CreateItem( nItemID, nSeed );
				if( pItem ) {
					DnActorHandle hActor = CDnActor::FindActorFromUniqueID( dwUniqueID );
					if( hActor ) {
						pItem->PlayInstantUseSound();
						pItem->ActivateSkillEffect( hActor, true );
					}
					SAFE_DELETE( pItem );
				}
				if( IsExistAction( "PickUp" ) ) {
					RemoveOctreeNode();
					SetActionQueue( "PickUp" );
				}
				else {
					SetDestroy();
				}
			}
			break;
	}
}


void CDnDropItem::ToggleToolTip()
{
	s_bShowToolTip = !s_bShowToolTip;
}

bool CDnDropItem::IsShowToolTip()
{
	return s_bShowToolTip;
}

void CDnDropItem::ShowDropItems( bool bShow )
{
	if( s_bShowDropItems == bShow ) return;

	s_bShowDropItems = bShow;
	ScopeLock<CSyncLock> Lock(s_Lock);
	for( DWORD i=0; i<s_pVecProcessList.size(); i++ ) {
		s_pVecProcessList[i]->ShowRenderBase( bShow );
	}
}

bool CDnDropItem::IsShowDropItems()
{
	return s_bShowDropItems;
}


void CDnDropItem::SetFocus( bool bFocus )
{
	if( s_hLastNearItem && s_hLastNearItem != GetMySmartPtr() ) {
		s_hLastNearItem->SetFocus( false );
	}
	m_bFocus = bFocus;

	if( bFocus ) {
		s_hLastNearItem = GetMySmartPtr();
		CEtOutlineFilter *pOutlineFilter = CDnWorld::GetInstance().GetEnvironment()->GetOutlineFilter();
		if( pOutlineFilter ) {
			if( !m_hOutline ) {
				m_hOutline = CEtOutlineObject::Create( GetObjectHandle() );
				m_hOutline->SetColor( EtColor(1.0f, 0.2f, 0.1f, 1.0f) );
				m_hOutline->SetWidth( 1.2f );
	//			m_hOutline->SetTechnique( 1 );
			}
			m_hOutline->Show( true );
		}
	}
	else {
		SAFE_RELEASE_SPTR( m_hOutline );
	}
}

CDnRenderBase *CDnDropItem::GetRenderBase()
{
	CDnRenderBase *pRender = dynamic_cast<CDnRenderBase *>(this);
	return pRender;
}