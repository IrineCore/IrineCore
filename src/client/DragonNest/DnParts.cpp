#include "StdAfx.h"
#include "DnParts.h"
#include "DNTableFile.h"
#include "DnTableDB.h"
#include "DnActor.h"
#include "DnPlayerActor.h"
#include "DnPartsHair.h"
#include "DnPartsHelmet.h"
#include "DnPartsFace.h"
#include "DnPartsFairy.h"
#include "DnPartsWing.h"
#include "DnInterface.h"
#ifdef PRE_FIX_MEMOPT_EXT
#include "DnCommonUtil.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

DECL_DN_SMART_PTR_STATIC( CDnParts, 3000 )

CDnParts::CDnParts()
: CDnUnknownRenderObject( false )
{
	CDnActionBase::Initialize( this );
	m_nDurability = m_nMaxDurability = 0;
	m_dwUniqueID = -1;
	m_PartsType = (PartsTypeEnum)-1;
	memset( m_bPermitPlayer, 0, sizeof(m_bPermitPlayer) );
	m_nEmptyDurabilitySoundIndex = -1;
	m_nSetItemID = 0;
#if defined( PRE_ADD_OVERLAP_SETEFFECT )
	m_bOverLapSet = false;	
#endif
	m_bSimpleSetItemDescription = false;
	m_nSimpleSetItemDescriptionID = -1;
	m_bLinkBone = false;
	m_nAniIndex = -1;
	m_nBlendAniIndex = -1;
	m_fBlendAniFrame = 0.f;
	m_fBlendFrame = 0.f;
	m_fBlendStartFrame = 0.f;
	m_bShow = true;
	m_bExistAction = false;
	m_bIgnoreCombine = false;
	EtMatrixIdentity( &m_matWorld );
}

CDnParts::~CDnParts()
{
	FreeObject();
}

bool CDnParts::Initialize( int nPartsTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound, int nLookItemID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) return false;
	if( !pSox->IsExistItem( nPartsTableID ) ) return false;

	m_nClassID = nPartsTableID;
	m_PartsType = (PartsTypeEnum)pSox->GetFieldFromLablePtr( nPartsTableID, "_Parts" )->GetInteger();
	m_nDurability = m_nMaxDurability = pSox->GetFieldFromLablePtr( nPartsTableID, "_Durability" )->GetInteger();
	m_nDurabilityRepairCoin = pSox->GetFieldFromLablePtr( nPartsTableID, "_DurabilityRepairCoin" )->GetInteger();
	m_nSetItemID = pSox->GetFieldFromLablePtr( nPartsTableID, "_SetItemID" )->GetInteger();
#if defined( PRE_ADD_OVERLAP_SETEFFECT )	
	m_bOverLapSet = pSox->GetFieldFromLablePtr( nPartsTableID, "_SetOverlap" )->GetInteger() ? true : false;
#endif
	m_bSimpleSetItemDescription = ( pSox->GetFieldFromLablePtr( nPartsTableID, "_Grade_SetItem" )->GetInteger() == 1 ) ? true : false;
	m_nSimpleSetItemDescriptionID = pSox->GetFieldFromLablePtr( nPartsTableID, "_SetItemTextID" )->GetInteger();

	int partsLookItemId = nPartsTableID;
	if (nLookItemID != ITEMCLSID_NONE)
		partsLookItemId = nLookItemID;
	m_bIgnoreCombine = ( pSox->GetFieldFromLablePtr( partsLookItemId, "_IgnoreCombine" )->GetInteger() ) ? true : false;

	char szLabel[32];
	for( int i=0; i<4; i++ ) {
		sprintf_s( szLabel, "_SubParts%d", i + 1 );
		int nValue = pSox->GetFieldFromLablePtr( nPartsTableID, szLabel )->GetInteger();
		if( nValue > 0 ) {
			m_VecSubPartsList.push_back( (PartsTypeEnum)nValue );
		}
	}

	CDnItem::Initialize( nPartsTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound, nLookItemID );

	return true;
}

const char *CDnParts::GetSkinName( int nActorTableID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) return NULL;
	if( m_nClassID == -1 ) return NULL;

	int nClassID = m_nClassID;
	if (m_nLookItemID != ITEMCLSID_NONE)
		nClassID = m_nLookItemID;

	char szStr[128];
	sprintf_s( szStr, "_Player%d_SkinName", nActorTableID );
	bool bNewParts = false;

	if( !IsCashItem() ) {
		if( m_hActor && m_PartsType >= Body && m_PartsType <= Foot ) {
#ifdef PRE_FIX_MEMOPT_EXT
			DNTableCell* pSkinNameSoxField = pSox->GetFieldFromLablePtr( nClassID, szStr );
			const char* pSkinName = NULL;
			if (pSkinNameSoxField)
				pSkinName = CommonUtil::GetFileNameFromFileEXT(pSkinNameSoxField->GetInteger());

			if( !pSkinName || strlen(pSkinName) == 0 ) {
#else
			char *pValue = pSox->GetFieldFromLablePtr( nClassID, szStr )->GetString();
			if( !pValue || strlen(pValue) == 0 ) {
#endif
					CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor*>(m_hActor.GetPointer());
					if (pActor && !pActor->IsNudeParts(GetMySmartPtr())) {
						// Set appearance: read default parts and replace the skin name.
						int nTemp = pActor->GetDefaultPartsInfo(m_PartsType);
						if (nTemp > 0) nClassID = nTemp;
					}
				
			}
		}
	}

#ifdef PRE_FIX_MEMOPT_EXT
	const char* pSkinName = NULL;
	if (bNewParts)
	{
		pSkinName = CommonUtil::GetFileNameFromFileEXT(nClassID);
	}else{
		DNTableCell* pNewSkinNameSoxField = pSox->GetFieldFromLablePtr(nClassID, szStr);
		if (pNewSkinNameSoxField)
			pSkinName = CommonUtil::GetFileNameFromFileEXT(pNewSkinNameSoxField->GetInteger());
	}

	return pSkinName;
#else
	return pSox->GetFieldFromLablePtr( nClassID, szStr )->GetString();
#endif
}

const char *CDnParts::GetDecalName( int nActorTableID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) return NULL;
	if( m_nClassID == -1 ) return NULL;

	int nClassID = m_nClassID; 

	char szStr[128];
	sprintf_s( szStr, "_Player%d_DecalName", nActorTableID );

#ifdef PRE_FIX_MEMOPT_EXT
	DNTableCell* pDecalNameField = pSox->GetFieldFromLablePtr( nClassID, szStr );
	if (pDecalNameField)
		return CommonUtil::GetFileNameFromFileEXT(pDecalNameField->GetInteger());
	return NULL;
#else
	char *pValue = pSox->GetFieldFromLablePtr( nClassID, szStr )->GetString();

	return pValue;
#endif
}

void CDnParts::SetActor( DnActorHandle hActor )
{
	m_hActor = hActor;
}

bool CDnParts::IsLoadPartsAni()
{
	if( IsCashItem() ) {
		switch( GetPartsType() ) {
			case CDnParts::CashEarring:
			case CDnParts::CashRing:
			case CDnParts::CashRing2:
			case CDnParts::CashWing:
			case CDnParts::CashTail:
			case CDnParts::CashFairy:
				return true;
		}
	}

	return false;
}

bool CDnParts::CreateObject( DnActorHandle hActor, const char *szAniName )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) return NULL;
	if( !pSox->IsExistItem( m_nClassID ) ) return false;

#ifdef PRE_FIX_MEMOPT_EXT
	DNTableFileFormat*  pFileNameSox = GetDNTable(CDnTableDB::TFILE);
	if (!pFileNameSox) return NULL;
#endif

	const char *szSkinName = GetSkinName( hActor->GetClassID() );

	if( szSkinName ) {
		bool bLoadPartsAni = IsLoadPartsAni();
		if( bLoadPartsAni ) {
			char szStr[128];
			sprintf_s( szStr, "_Player%d_AniName", hActor->GetClassID() );
#ifdef PRE_FIX_MEMOPT_EXT

	#ifdef PRE_FIX_DMIX_NOACTION
			int nLookItemId = GetLookItemID();
			int nItemIdForAction = (nLookItemId != ITEMCLSID_NONE) ? nLookItemId : m_nClassID;
			DNTableCell* pField = pSox->GetFieldFromLablePtr( nItemIdForAction, szStr );
	#else
			DNTableCell* pField = pSox->GetFieldFromLablePtr( m_nClassID, szStr );
	#endif
			const char *szPartsAniName = (pField) ? CommonUtil::GetFileNameFromFileEXT(pField->GetInteger(), pFileNameSox) : NULL;
#else
			const char *szPartsAniName = pSox->GetFieldFromLablePtr( m_nClassID, szStr )->GetString();
#endif

			m_hObject = EternityEngine::CreateAniObject( CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), szPartsAniName );
			/*
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
			if( m_hObject && m_hObject->GetAniHandle() ) m_hObject->GetAniHandle()->SetDeleteImmediate( true );
#endif
			*/
		}
		else {
			m_hObject = EternityEngine::CreateAniObject( CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), szAniName );
			if( m_hObject ) {
				m_hObject->SetParent( hActor->GetObjectHandle(), -1 );
				m_hObject->GetSkinInstance()->SetRenderAlphaTwoPass( true );
			}
		}
		/*
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
		if( m_hObject && m_hObject->GetMesh() ) m_hObject->GetMesh()->SetDeleteImmediate( true );
#endif
		*/
	}

	char szStr[128];
	sprintf_s( szStr, "_Player%d_ActName", hActor->GetClassID() );
#ifdef PRE_FIX_MEMOPT_EXT

	#ifdef PRE_FIX_DMIX_NOACTION
	int nLookItemId = GetLookItemID();
	int nItemIdForAction = (nLookItemId != ITEMCLSID_NONE) ? nLookItemId : m_nClassID;
	const char* szActName = CommonUtil::GetFileNameFromFileEXT(pSox, nItemIdForAction, szStr, pFileNameSox);
	#else
	const char* szActName = CommonUtil::GetFileNameFromFileEXT(pSox, m_nClassID, szStr, pFileNameSox);
	#endif
	if( szActName && strlen( szActName ) ) {
#else
	const char *szActName = pSox->GetFieldFromLablePtr( m_nClassID, szStr )->GetString();
	if( strlen( szActName ) ) {
#endif
		m_bExistAction = LoadAction( CEtResourceMng::GetInstance().GetFullName( szActName ).c_str() );
		if( IsExistAction( "Idle" ) ) SetAction( "Idle", 0.f, 0.f );
	}

#ifdef PRE_FIX_MEMOPT_EXT
	const char *szFileName = CommonUtil::GetFileNameFromFileEXT(pSox, m_nClassID, "_EmptyDurabilitySound", pFileNameSox);
#else
	const char *szFileName = pSox->GetFieldFromLablePtr( m_nClassID, "_EmptyDurabilitySound" )->GetString();
#endif

	if( szFileName && strlen( szFileName ) > 0 ) {
		m_nEmptyDurabilitySoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	}

	return true;
}

bool CDnParts::FreeObject()
{
	UnlinkParts();
	SAFE_RELEASE_SPTR( m_hObject );
	FreeAction();
	if( m_nEmptyDurabilitySoundIndex != -1 ) {
		CEtSoundEngine::GetInstance().RemoveSound( m_nEmptyDurabilitySoundIndex );
		m_nEmptyDurabilitySoundIndex = -1;
	}
	ReleaseSignalImp();
	return true;
}

float CDnParts::GetRepairPrice()
{
	float fValue = 0.f;
	if( m_PartsType >= CDnParts::Helmet && m_PartsType <= CDnParts::Foot )
	{
		int nGapDur = m_nMaxDurability - m_nDurability;
		if( nGapDur > 0 )
		{
			float fEnchantRevision = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::RepairDurabilityRevision );
			float fRatio = ( 1.f / m_nMaxDurability ) * (float)nGapDur;

			fValue = m_nDurabilityRepairCoin * fRatio;
			if( m_nDurability == 0 ) fValue *= 1.3f;
			if( m_cEnchantLevel > 0 ) fValue *= pow( fEnchantRevision, m_cEnchantLevel );
		}
	}
	if( fValue != 0.f && fValue < 1.f ) fValue = 1.f;
	return fValue;
}

void CDnParts::OnEmptyDurability()
{
	if( m_hActor && m_hActor == CDnActor::s_hLocalActor ) {
		wchar_t wszTemp[256] = {0};
		swprintf_s( wszTemp, _countof(wszTemp), L"%s[%s]", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1735 ), m_szName.c_str() );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp );

		if( m_nEmptyDurabilitySoundIndex != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nEmptyDurabilitySoundIndex );
	}
}

DnPartsHandle CDnParts::CreateParts( int nPartsTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound, int nLookItemID)
{
	if( nPartsTableID < 1 ) return CDnParts::Identity();

	DnPartsHandle hParts;
	if( CDnItem::IsCashItem( nPartsTableID ) ) {
		switch( GetPartsType( nPartsTableID ) ) {
			case CashHelmet: hParts = (new CDnPartsHelmet)->GetMySmartPtr(); break;
			case CashFairy: hParts = (new CDnPartsFairy)->GetMySmartPtr(); break;
			case CashWing: hParts = (new CDnPartsWing)->GetMySmartPtr(); break;
			default: hParts = (new CDnParts)->GetMySmartPtr(); break;
		}
	}
	else {
		switch( GetPartsType( nPartsTableID ) ) {
			case Face: hParts = (new CDnPartsFace)->GetMySmartPtr(); break;
			case Hair: hParts = (new CDnPartsHair)->GetMySmartPtr(); break;
			case Helmet: hParts = (new CDnPartsHelmet)->GetMySmartPtr(); break;
			default: hParts = (new CDnParts)->GetMySmartPtr(); break;
		}
	}
	if( !hParts ) return CDnParts::Identity();

	hParts->Initialize( nPartsTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound, nLookItemID );
	return hParts;
}

CDnParts::PartsTypeEnum CDnParts::GetPartsType( int nPartsTableID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) return (PartsTypeEnum)-1;
	if( pSox->IsExistItem( nPartsTableID ) == false ) return (PartsTypeEnum)-1;

	return (PartsTypeEnum)pSox->GetFieldFromLablePtr( nPartsTableID, "_Parts" )->GetInteger();
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

void CDnParts::ConvertFloatToR10G10B10( DWORD *dwOutColor, float *pSourceColor )
{
	int nR = (int)(255 * pSourceColor[0]);
	int nG = (int)(255 * pSourceColor[1]);
	int nB = (int)(255 * pSourceColor[2]);
	ASSERT( nR >= 0 && nR < 1024);		// Assuming it will never exceed 1024 (4.0f).
	ASSERT( nG >= 0 && nG < 1024);
	ASSERT( nB >= 0 && nB < 1024);
	*dwOutColor = ((nR<<20)|(nG<<10)|nB);
}

void CDnParts::ConvertR10G10B10ToFloat( float *pOutColor, DWORD dwSourceColor )
{
	pOutColor[0] = ((dwSourceColor >> 20) & 1023) / 255.f;
	pOutColor[1] = ((dwSourceColor >> 10) & 1023) / 255.f;	
	pOutColor[2] = (dwSourceColor & 1023) / 255.f;
}

void CDnParts::ConvertD3DCOLORToR10G10B10( DWORD *pOutColor, D3DCOLOR dwSource, float fIntensity )
{
	int nR = (int)(fIntensity * LOBYTE(HIWORD(dwSource)));
	int nG = (int)(fIntensity * HIBYTE(LOWORD(dwSource)));
	int nB = (int)(fIntensity * LOBYTE(LOWORD(dwSource)));
	ASSERT( nR >= 0 && nR < 1024);		// Assuming it will never exceed 1024 (4.0f).
	ASSERT( nG >= 0 && nG < 1024);
	ASSERT( nB >= 0 && nB < 1024);
	*pOutColor = ((nR<<20)|(nG<<10)|nB);
}

void CDnParts::ConvertInt3ToR10G10B10( DWORD *pOutColor, int *pSource )
{
	int nR = pSource[0];
	int nG = pSource[1];
	int nB = pSource[2];
	ASSERT( nR >= 0 && nR < 1024);
	ASSERT( nG >= 0 && nG < 1024);
	ASSERT( nB >= 0 && nB < 1024);
	*pOutColor = ((nR<<20)|(nG<<10)|nB);
}

#ifdef PRE_ADD_TRANSPARENCY_COSTUME
bool CDnParts::IsTransparentSkin( int nPartsTableID, int nActorTableID )
{
	if( !CDnTableDB::IsActive() ) return false;
	if( nActorTableID == 0 ) return false;
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) return false;
	if( !pSox->IsExistItem( nPartsTableID ) ) return false;

	char szStr[128];
	sprintf_s( szStr, "_Player%d_SkinName", nActorTableID );

	//
	int nSkinIndex = pSox->GetFieldFromLablePtr(nPartsTableID, "_SkinID")->GetInteger();
	if (nSkinIndex > 0)
		return false;

#ifdef PRE_FIX_MEMOPT_EXT
	std::string wszSkin;
	CommonUtil::GetFileNameFromFileEXT(wszSkin, pSox, nPartsTableID, szStr);
#else
	std::string wszSkin = pSox->GetFieldFromLablePtr( nPartsTableID, szStr )->GetString();
#endif
	if( wszSkin.empty() ) return true;
	return false;
}
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME

void CDnParts::UpdateSkinColorParam( DWORD dwSkinColor )
{
	if( !m_hObject ) {
		return;
	}
	int nCustomColor = m_hObject->AddCustomParam( "g_SkinColor" );
	if( nCustomColor != -1 )  {
		EtColor Color = dwSkinColor;
		m_hObject->SetCustomParam( nCustomColor, &Color );
	}
}

void CDnParts::UpdateEyeColorParam( DWORD dwEyeColor )
{
	if( !m_hObject ) {
		return;
	}
	int nCustomColor = m_hObject->AddCustomParam( "g_EyeColor" );
	if( nCustomColor != -1 )  {
		EtColor Color;
		ConvertR10G10B10ToFloat((FLOAT*)&Color, dwEyeColor);
		m_hObject->SetCustomParam( nCustomColor, &Color );
	}
}

void CDnParts::LinkParts( DnActorHandle hActor, const char *szBoneName )
{
	m_hActor = hActor;
	m_bLinkBone = false;
	m_szLinkDummyBoneName.clear();

	MAActorRenderBase *pRender = dynamic_cast<MAActorRenderBase *>(m_hActor.GetPointer());
	if( pRender ) {
		int nBoneIndex = -1;
		if( szBoneName ) nBoneIndex = pRender->GetBoneIndex( szBoneName );
		if( szBoneName && nBoneIndex == -1 ) {
			if( pRender->GetDummyBoneIndex( szBoneName ) != -1 ) {
				if( m_hObject ) 
					m_hObject->SetParent( m_hActor->GetObjectHandle(), szBoneName );
				else m_szLinkDummyBoneName = szBoneName;
			}
			else return;
		}
		else {
			if( m_hObject )
				m_hObject->SetParent( pRender->GetObjectHandle(), nBoneIndex );
		}

		m_bLinkBone = true;
	}
}

void CDnParts::UnlinkParts()
{
	if( m_hObject )
		m_hObject->SetParent( CEtObject::Identity(), 0 );
	m_bExistAction = false;
}

void CDnParts::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_hActor && m_bLinkBone ) {
		if( !m_hObject && m_bExistAction && !m_szLinkDummyBoneName.empty() ) {
			bool bExistDummyBone;
			m_matWorld = m_hActor->GetBoneMatrix( m_szLinkDummyBoneName.c_str(), &bExistDummyBone );
		}
	}
	CDnActionBase::ProcessAction( LocalTime, fDelta );
	Process( &m_matWorld, LocalTime, fDelta );
	CDnActionSignalImp::Process( LocalTime, fDelta );

	// Update periodically since the actor's alpha state can change in real time, such as with stealth state effects.
	if( m_hActor )
	{
		float fAlpha = m_hActor->GetAlpha();
		ApplyAlphaToSignalImps( fAlpha );
	}
}

void CDnParts::ChangeAnimation( int nAniIndex, float fFrame, float fBlendFrame )
{
	m_nBlendAniIndex = m_nAniIndex;
	m_fBlendStartFrame = fFrame;
	m_fBlendFrame = fBlendFrame;
	if( m_fBlendFrame == 0.f ) m_nBlendAniIndex = -1;

	if( m_nBlendAniIndex != -1 ) {
		m_fBlendAniFrame = m_fFrame;

		float fAniLength = (float)m_hObject->GetLastFrame( m_nBlendAniIndex );
		if( m_fBlendAniFrame >= fAniLength ) {
			m_fBlendAniFrame = fAniLength;
		}
	}

	m_nAniIndex = nAniIndex;
}

void CDnParts::Process( EtMatrix *pmatWorld, LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hObject ) return;
	if( m_nActionIndex == -1 ) return;
	if( m_nActionIndex >= (int)m_nVecAniIndexList.size() ) return;
	if( m_nAniIndex == -1 ) return;

	m_hObject->SetAniFrame( m_nAniIndex, m_fFrame );

	// Blend Ani
	if( m_nBlendAniIndex != -1 ) {
		if( m_fFrame - m_fBlendStartFrame < m_fBlendFrame ) {
			float fWeight = 1.f - ( 1.f / m_fBlendFrame * ( m_fFrame - m_fBlendStartFrame ) );
			m_hObject->BlendAniFrame( m_nBlendAniIndex, m_fBlendAniFrame, fWeight, 0 );
		}
		else {
			m_nBlendAniIndex = -1;
		}
	}

	m_hObject->SkipPhysics( IsSignalRange( STE_PhysicsSkip ) );
}

int CDnParts::GetAniIndex( const char *szAniName )
{
	if( !m_hObject ) return -1;
	if( !m_hObject->GetAniHandle() ) return -1;
	for( int i=0; i<m_hObject->GetAniCount(); i++ ) {
		if( strcmp( m_hObject->GetAniName(i), szAniName ) == NULL ) return i;
	}
	return -1;
}

int CDnParts::GetBoneIndex( const char *szBoneName )
{
	if( !m_hObject ) return -1;
	EtAniHandle handle = m_hObject->GetAniHandle();
	if( !handle ) return -1;
	return handle->GetBoneIndex( szBoneName );
}

int CDnParts::GetDummyBoneIndex( const char *szBoneName )
{
	if( !m_hObject ) return -1;
	EtMeshHandle handle = m_hObject->GetMesh();
	if( !handle ) return -1;
	return handle->FindDummy( szBoneName );
}

void CDnParts::ShowRenderBase( bool bShow )
{
	m_bShow = bShow;
	if( !bShow )
		ReleaseSignalImp();
	if( !m_hObject ) return;
	m_hObject->ShowObject( bShow );
}

bool CDnParts::IsShow()
{
	return m_bShow;
}

CDnParts::PartsTypeEnum CDnParts::GetSubPartsIndex( int nIndex ) 
{ 
	if( nIndex < 0 || nIndex > PartsTypeEnum_Amount )
		return (PartsTypeEnum)-1;
	return m_VecSubPartsList[nIndex]; 
}

#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
bool CDnParts::IsAccessoryItem(PartsTypeEnum type, bool bIsCash)
{
	if (bIsCash)
		return (type > DefaultPartsTypeEnum_Amount);
	return (type > DefaultPartsType_Max);
}
#endif
