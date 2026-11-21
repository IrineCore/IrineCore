#include "StdAfx.h"
#include "MAEnvironmentEffect.h"
#include "DnTableDB.h"
#include "EtSoundChannel.h"
#include "PerfCheck.h"
#include "DnActor.h"
#include "DnWorld.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

MAEnvironmentEffect::MAEnvironmentEffect()
{
	memset( m_nEnviEffect, -1, sizeof(m_nEnviEffect) );
	memset( m_cEnviEffectType, 0, sizeof(m_cEnviEffectType) );
	m_fEnviSoundVolume = 1.f;
	m_fEnviSoundRange = 400.f;
	m_fEnviSoundRolloff = 0.4f;
	m_nEnviActorTableID = 0;
}

MAEnvironmentEffect::~MAEnvironmentEffect()
{
	ReleaseEnvironmentEffect();
}

bool MAEnvironmentEffect::Initialize( int nActorTableID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TENVIEFFECT );
	if( !pSox ) return false;
	if( nActorTableID == -1 ) return false;
	if( pSox->IsExistItem( nActorTableID ) == false ) return false;
	m_nEnviActorTableID = nActorTableID;

	m_fEnviSoundVolume = pSox->GetFieldFromLablePtr( nActorTableID, "_Volume" )->GetFloat();
	m_fEnviSoundRange = pSox->GetFieldFromLablePtr( nActorTableID, "_Range" )->GetFloat() * 100.f;
	m_fEnviSoundRolloff = pSox->GetFieldFromLablePtr( nActorTableID, "_RollOff" )->GetFloat();

	InitializeUsingEnvironmentEffect();
	return true;
}

void MAEnvironmentEffect::ReleaseEnvironmentEffect()
{
	for( int i=0; i<ConditionEnum_Amount; i++ ) {
		// Because TileType has None 
		for( int j=1; j<TileTypeEnum_Amount; j++ ) {
			if( m_nEnviEffect[i][j-1] != -1 ) {
				switch( m_cEnviEffectType[i][j-1] ) {
					case 0: EternityEngine::DeleteParticleData( m_nEnviEffect[i][j-1] ); break;
					case 1: EternityEngine::DeleteEffectData( m_nEnviEffect[i][j-1] ); break;
				}
				m_nEnviEffect[i][j-1] = -1;
				m_cEnviEffectType[i][j-1] = 0;
			}

			for( DWORD k=0; k<m_nVecEnviSound[i][j-1].size(); k++ ) {
				CEtSoundEngine::GetInstance().RemoveSound( m_nVecEnviSound[i][j-1][k] );
			}
			SAFE_DELETE_VEC( m_nVecEnviSound[i][j-1] );
		}
	}
}

bool MAEnvironmentEffect::InitializeUsingEnvironmentEffect()
{
	ReleaseEnvironmentEffect();
	if( m_nEnviActorTableID <= 0 ) return false;
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	CDnActor *pActor = dynamic_cast<CDnActor*>(this);
	if( pActor && pActor->IsNpcActor() ) return false;
#endif

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TENVIEFFECT );
	char *szConditionStr[ConditionEnum_Amount] = { "Step", "Landing", "Down", "Tumble" };
	char szTemp[64];
	char szExt[32];
	CFileNameString szFileName;
	int nIndex;

	for( int i=0; i<ConditionEnum_Amount; i++ ) {
		// Because TileType has None 
		for( int j=1; j<TileTypeEnum_Amount; j++ ) {
			if( CDnWorld::IsActive() && !CDnWorld::GetInstance().IsUsingTileType( (TileTypeEnum)j ) ) continue;
			// Particle
			sprintf_s( szTemp, "_%sParticle_TileType%d", szConditionStr[i], j );
			CDNTableFile::Cell* pField = pSox->GetFieldFromLablePtr( m_nEnviActorTableID, szTemp );
			if( pField )
				szFileName = pField->GetString();
			if( !szFileName.empty() ) {
				_GetExt( szExt, _countof(szExt), szFileName.c_str() );
				if( _stricmp( szExt, "ptc" ) == NULL ) {
					m_cEnviEffectType[i][j-1] = 0;
					m_nEnviEffect[i][j-1] = EternityEngine::LoadParticleData( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str() );
				}
				else if( _stricmp( szExt, "eff" ) == NULL ) {
					m_cEnviEffectType[i][j-1] = 1;
					m_nEnviEffect[i][j-1] = EternityEngine::LoadEffectData( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str() );
				}
			}

			// Sound
			sprintf_s( szTemp, "_%sSound_TileType%d", szConditionStr[i], j );
			pField = pSox->GetFieldFromLablePtr( m_nEnviActorTableID, szTemp );
			if( pField )
				szFileName = pField->GetString();
			if( !szFileName.empty() ) {
				CFileNameString szSoundName;
				bool bFindFile = true;
				for( int k=0; ; k++ ) {
					szSoundName = CEtResourceMng::GetInstance().GetFullNameRandom( szFileName, k, &bFindFile );
					if( bFindFile == false ) break;
					nIndex = CEtSoundEngine::GetInstance().LoadSound( szSoundName.c_str(), true, false );
					if( nIndex == -1 ) break;
					m_nVecEnviSound[i][j-1].push_back( nIndex );
				}
			}
		}
	}

	return true;
}

bool MAEnvironmentEffect::CreateEnviEffectParticle( ConditionEnum Condition, TileTypeEnum TileType, MatrixEx *pCross )
{
	if( TileType == TileTypeEnum::None ) return false;
	if( m_nEnviEffect[Condition][TileType-1] == -1 ) return false;
	CDnActor *pActor = dynamic_cast<CDnActor *>(this);
	if( pActor && !pActor->IsShow() ) return false;

	switch( m_cEnviEffectType[Condition][TileType-1] ) {
		case 0: return ( EternityEngine::CreateBillboardEffect( m_nEnviEffect[Condition][TileType-1], *pCross ) ) ? true : false;
		case 1: return ( EternityEngine::CreateEffectObject( m_nEnviEffect[Condition][TileType-1], *pCross ) ) ? true : false;
	}
	return false;
}

EtSoundChannelHandle MAEnvironmentEffect::CreateEnviEffectSound( ConditionEnum Condition, TileTypeEnum TileType, EtVector3 &vPos )
{
	if( TileType == TileTypeEnum::None ) return CEtSoundChannel::Identity();
	if( m_nVecEnviSound[Condition][TileType-1].empty() ) return CEtSoundChannel::Identity();
	CDnActor *pActor = dynamic_cast<CDnActor *>(this);
	if( pActor && !pActor->IsShow() ) return CEtSoundChannel::Identity();

	int nRandomIndex = m_nVecEnviSound[Condition][TileType-1][ _rand()%m_nVecEnviSound[Condition][TileType-1].size() ];

	EtSoundChannelHandle hChannel = CEtSoundEngine::GetInstance().PlaySound( "3D", nRandomIndex, false, true );
	if( hChannel ) {
		hChannel->SetVolume( m_fEnviSoundVolume );
		hChannel->SetRollOff( 3, 0.f, 1.f, m_fEnviSoundRange * m_fEnviSoundRolloff, 1.f, m_fEnviSoundRange, 0.f );
		hChannel->SetPosition( vPos );
		hChannel->Resume();

	}
	return hChannel;
}
