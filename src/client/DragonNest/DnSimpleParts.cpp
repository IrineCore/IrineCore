#include "StdAfx.h"
#include "DnSimpleParts.h"
#include "DNTableFile.h"
#include "DnTableDB.h"
#include "DnActor.h"
#include "DnPlayerActor.h"
#include "DnInterface.h"
#include "EtMergedSkin.h"
#include "EtOptionController.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

DECL_DN_SMART_PTR_STATIC( CDnSimpleParts, 100 )

CDnSimpleParts::CDnSimpleParts()
: CDnUnknownRenderObject( false )
{
	CDnActionBase::Initialize( this );

	m_bLinkBone = false;
	m_nAniIndex = -1;
	m_nBlendAniIndex = -1;
	m_fBlendAniFrame = 0.f;
	m_fBlendFrame = 0.f;
	m_fBlendStartFrame = 0.f;
	m_bShow = true;
	m_bExistAction = false;
	EtMatrixIdentity( &m_matWorld );

}

CDnSimpleParts::~CDnSimpleParts()
{
	FreeObject();
}


bool CDnSimpleParts::Initialize( int nPartsTableID , const char *szSkinName , const char *szAniName )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPARTS );
	if( !pSox ) return false;
	if( !pSox->IsExistItem( nPartsTableID ) ) return false;
	
	m_nClassID = nPartsTableID;

	CreateObject(szSkinName,szAniName);
	return true;
}


bool CDnSimpleParts::CreateObject( const char *szSkinName , const char *szAniName ) 
{
	m_hObject = EternityEngine::CreateAniObject(szSkinName , szAniName ); // *CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), szAniName*/ );
	return true;
}

DnPartsHandle CDnSimpleParts::CreateParts( int nPartsTableID , const char *szSkinName , const char *szAniName )
{
	if( nPartsTableID < 1 ) return CDnParts::Identity();

	DnSimplePartsHandle hParts;

	hParts = (new CDnSimpleParts)->GetMySmartPtr();
	if( !hParts ) return CDnSimpleParts::Identity();

	hParts->Initialize( nPartsTableID , szSkinName , szAniName);
	return hParts;
}

bool CDnSimpleParts::FreeObject()
{
	UnlinkParts();
	SAFE_RELEASE_SPTR( m_hObject );
	FreeAction();
	return true;
}

void CDnSimpleParts::UnlinkParts()
{
	if( m_hObject )
		m_hObject->SetParent( CEtObject::Identity(), 0 );
	m_bExistAction = false;
}


const char *CDnSimpleParts::GetSkinName( int nActorTableID )
{
	return "";
}

void CDnSimpleParts::ChangeAnimation( int nAniIndex, float fFrame, float fBlendFrame )
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

void CDnSimpleParts::Process( EtMatrix *pmatWorld, LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hObject ) return;
// 	if( m_nActionIndex == -1 ) return;
// 	if( m_nActionIndex >= (int)m_nVecAniIndexList.size() ) return;
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

void CDnSimpleParts::Process( LOCAL_TIME LocalTime, float fDelta )
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
}



int CDnSimpleParts::GetAniIndex( const char *szAniName )
{
	if( !m_hObject ) return -1;
	if( !m_hObject->GetAniHandle() ) return -1;
	for( int i=0; i<m_hObject->GetAniCount(); i++ ) {
		if( strcmp( m_hObject->GetAniName(i), szAniName ) == NULL ) return i;
	}
	return -1;
}

int CDnSimpleParts::GetBoneIndex( const char *szBoneName )
{
	if( !m_hObject ) return -1;
	EtAniHandle handle = m_hObject->GetAniHandle();
	if( !handle ) return -1;
	return handle->GetBoneIndex( szBoneName );
}

int CDnSimpleParts::GetDummyBoneIndex( const char *szBoneName )
{
	if( !m_hObject ) return -1;
	EtMeshHandle handle = m_hObject->GetMesh();
	if( !handle ) return -1;
	return handle->FindDummy( szBoneName );
}

void CDnSimpleParts::ShowRenderBase( bool bShow )
{
	m_bShow = bShow;
	if( !m_hObject ) return;
	m_hObject->ShowObject( bShow );
	if( !bShow )
		ReleaseSignalImp();
}

bool CDnSimpleParts::IsShow()
{
	return m_bShow;
}

void CDnSimpleParts::LinkPartsToBone( DnActorHandle hActor, const char *szBoneName )
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
			if( m_hObject ) m_hObject->SetParent( pRender->GetObjectHandle(), nBoneIndex );
		}

		m_bLinkBone = true;
	}
}


void CDnSimpleParts::LinkPartsToObject(const char *szSkinName , const char *szAniName ,EtObjectHandle hPlayerObject)
{
	// Attach using another object's animation.

	EtSkinHandle hSkin = EternityEngine::LoadSkin( szSkinName );
	if( !hSkin ) return;
	
	m_hObject = EternityEngine::CreateAniObject( hSkin, szAniName );
	
	if( !m_hObject ) return;
	m_hObject->GetSkinInstance()->SetRenderAlphaTwoPass( true );
	m_hObject->SetParent( hPlayerObject, -1);
	m_hObject->ShowObject( true );
}

void CDnSimpleParts::InitializeItem( int nTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound )
{
	CDnItem::Initialize( nTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound );
}