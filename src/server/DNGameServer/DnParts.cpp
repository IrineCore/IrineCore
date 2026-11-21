#include "stdafx.h"
#include "DnParts.h"
#include "DNTableFile.h"
#include "DnTableDB.h"
#include "DnActor.h"
#ifdef PRE_FIX_MEMOPT_EXT
#include "DNGameDataManager.h"
#endif

DECL_DN_SMART_PTR_STATIC( CDnParts, MAX_SESSION_COUNT, 100 )

CDnParts::CDnParts( CMultiRoom *pRoom )
: CDnUnknownRenderObject( pRoom, false )
, CDnItem( pRoom )
{
	m_nDurability = m_nMaxDurability = 0;
	m_nClassID = 0;
	m_dwUniqueID = -1;
	m_PartsType = (PartsTypeEnum)-1;
	m_nSetItemID = 0;
#if defined( PRE_ADD_OVERLAP_SETEFFECT )
	m_bOverLapSet = false;
#endif
}

CDnParts::~CDnParts()
{
	FreeObject();
}

bool CDnParts::Initialize( int nPartsTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"PartsTable.ext failed\r\n");
		return false;
	}
	if( !pSox->IsExistItem( nPartsTableID ) ) return false;

	m_nClassID = nPartsTableID;
	m_PartsType = (PartsTypeEnum)pSox->GetFieldFromLablePtr( nPartsTableID, "_Parts" )->GetInteger();
	m_nDurability = m_nMaxDurability = pSox->GetFieldFromLablePtr( nPartsTableID, "_Durability" )->GetInteger();
	m_nSetItemID = pSox->GetFieldFromLablePtr( nPartsTableID, "_SetItemID" )->GetInteger();
#if defined( PRE_ADD_OVERLAP_SETEFFECT )	
	m_bOverLapSet = pSox->GetFieldFromLablePtr( nPartsTableID, "_SetOverlap" )->GetInteger() ? true : false;
#endif

	CDnItem::Initialize( nPartsTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound );
	return true;
}

const char *CDnParts::GetSkinName( int nActorTableID )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) return "";
	if( !pSox->IsExistItem( m_nClassID ) ) return "";

	char szStr[128];
	sprintf_s( szStr, "_Player%d_SkinName", nActorTableID );
#ifdef PRE_FIX_MEMOPT_EXT
	DNTableCell* pField = pSox->GetFieldFromLablePtr( m_nClassID, szStr );
	if (pField)
		return g_pDataManager->GetFileNameFromFileEXT(pField->GetInteger());
	return NULL;
#else
	return pSox->GetFieldFromLablePtr( m_nClassID, szStr )->GetString();
#endif
}

void CDnParts::SetActor( DnActorHandle hActor )
{
	m_hActor = hActor;
}

bool CDnParts::CreateObject( DnActorHandle hActor, const char *szAniName )
{
	const char *szSkinName = GetSkinName( hActor->GetClassID() );
	if( !szSkinName ) return false;

	m_hObject = EternityEngine::CreateAniObject( GetRoom(), CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), szAniName );

//	if( m_PartsType == Face ) return true;
	return true;
}

bool CDnParts::FreeObject()
{
	SAFE_RELEASE_SPTR( m_hObject );
	return true;
}

DnPartsHandle CDnParts::CreateParts( CMultiRoom *pRoom, int nPartsTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound )
{
	if( nPartsTableID < 1 ) return CDnParts::Identity();
	DnPartsHandle hParts = (new CDnParts(pRoom))->GetMySmartPtr();

	hParts->Initialize( nPartsTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound );
	return hParts;
}

/*
bool CDnParts::LoadParts( PartsTypeEnum Index, const char *szSkinName )
{
	MAPartsBody *pBody = dynamic_cast<MAPartsBody *>(m_hActor.GetPointer());
	if( pBody == NULL ) return;

	if( pBody->GetAni.empty() ) return false;

	if( strcmp( m_szPartsName[Index].c_str(), szSkinName ) == NULL ) return true;

	if( Index == FACE ) {
		m_hObject = EternityEngine::CreateAniObject( szSkinName, m_szAniFileName.c_str() );
		m_hObject->EnableShadowCast( true );
		m_hObject->EnableShadowReceive( false );

		m_hPartsObject[Index] = (new CDnParts)->GetMySmartPtr();
		m_hPartsObject[Index]->SetObject( m_hObject );

		for( int i=0; i<PARTS_INDEX_AMOUNT; i++ ) {
			if( m_bLoaded[i] && m_hPartsObject[Index] ) m_hPartsObject[Index]->GetObject()->SetParent( m_hObject, -1 );
			m_bLoaded[i] = false;
		}

		// For PartsBody, because m_hObject is created the moment parts are set
		// Call CacheAniIndex so Action can cache the Animation Index.
		CDnActor *pActor = dynamic_cast<CDnActor *>(this);
		if( pActor ) {
			pActor->CacheAniIndex();
			pActor->Initialize();
		}
	}
	else {
		SAFE_RELEASE_SPTR( m_hPartsObject[Index] );
		m_hPartsObject[Index] = (new CDnParts)->GetMySmartPtr();
		m_hPartsObject[Index]->CreateObject( szSkinName, m_szAniFileName.c_str() );

		if( m_hObject ) {
			m_hPartsObject[Index]->GetObject()->SetParent( m_hObject, -1 );
		}
		else m_bLoaded[Index] = true;
	}

	m_szPartsName[Index] = szSkinName;
	return true;
}


*/

CDnItem &CDnParts::operator = ( TItem &e )
{
	CDnItem::operator = (e);

	m_nDurability = e.wDur;

	return *this;
}
