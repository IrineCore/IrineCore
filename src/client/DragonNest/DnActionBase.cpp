#include "StdAfx.h"
#include "DnActionBase.h"
#include "DnRenderBase.h"
#include "EtActionSignal.h"
#include "DnWeapon.h"
#include "DnProjectile.h"
#include "DnActorClassDefine.h"
#include "DnTableDB.h"
#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
#include "EtActionCoreMng.h"
#include <mmsystem.h>
#endif

#ifndef _FINAL_BUILD
#include "DnInterface.h"
#endif //_FINAL_BUILD


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

map<int, DnActorHandle> CDnActionBase::s_mapAcademicSummonMonsterPreload;

CDnActionBase::CDnActionBase()
{
	m_pRender = NULL;

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	m_pCheckInitSignalFunc = CDnActionBase::CheckInitSignal;
#endif
	m_pCheckPreSignalFunc = CDnActionBase::CheckPreSignal;
	m_pCheckPostSignalFunc = CDnActionBase::CheckPostSignal;

	ResetActionBase();
}

CDnActionBase::~CDnActionBase()
{
	FreeAction();
}

bool CDnActionBase::Initialize( CDnRenderBase *pRender )
{
	m_pRender = pRender;
	return true;
}
void CDnActionBase::ResetActionBase()
{
	m_LocalTime = 0;
	m_ActionTime = 0;
	m_FirstActionTime = 0;
	m_nActionIndex = -1;
	m_fPrevFrame = 0.f;
	m_fFrame = 0.f;
	m_fQueueBlendFrame = m_fQueueStartFrame = 0.f;
	m_nCustomActionIndex = -1;

	m_nLoopCount = 0;
	m_fFps = s_fDefaultFps;
	m_bCustomProcessSignal = false;

	m_fActionQueueDelay = 0.f;
	m_szAction.clear();
	m_szActionQueue.clear();
}

void CDnActionBase::ProcessAction( LOCAL_TIME LocalTime, float fDelta )
{
	m_LocalTime = LocalTime;
	if( !m_szActionQueue.empty() )
	{
		bool bFlag = true;
		if( m_fActionQueueDelay != 0.f ) {
			m_fActionQueueDelay -= fDelta;
			if( m_fActionQueueDelay <= 0.f ) {
				m_fActionQueueDelay = 0.f;
			}
			else bFlag = false;
		}
		if( bFlag ) {
			if( m_fQueueBlendFrame == -1 ) 
			{
				ActionElementStruct *pStruct = GetElement( m_nActionIndex );
				if( pStruct ) {
					m_fQueueBlendFrame = (float)pStruct->dwBlendFrame;
				}
			}
			SetAction( m_szActionQueue.c_str(), m_fQueueStartFrame, m_fQueueBlendFrame );
			m_szActionQueue.clear();
		}
	}

	if( m_nActionIndex == -1 )
		return;

	ActionElementStruct *pStruct = GetElement( m_nActionIndex );

	if( 0.0f < m_fFps )
		m_fFrame = ( ( m_LocalTime - m_ActionTime ) / 1000.f ) * m_fFps;
	else
		return;

	if( !pStruct ) return;

#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
	pStruct->dwLastUsedTime = timeGetTime();
	if( pStruct->bCheckPostSignal )
	{
		for( DWORD k=0; k<pStruct->pVecSignalList.size(); k++ ) {
			if( CEtActionCoreMng::GetInstance().IsOptimizeSignal( pStruct->pVecSignalList[k]->GetSignalIndex() ) )
				m_pCheckPreSignalFunc( pStruct, m_nActionIndex, pStruct->pVecSignalList[k], k, NULL );
		}
		pStruct->bCheckPostSignal = false;
	}
#endif

	if( m_fFrame < 0.f ) m_fFrame = (float)pStruct->dwLength + 1.f;

	if( m_fFrame > (float)pStruct->dwLength )
	{
		ProcessSignal( pStruct, m_fFrame, m_fPrevFrame );

		if( m_pRender && m_nVecAniIndexList[m_nActionIndex] != -1 ) {
			EtVector3 vDist;
			m_pRender->CalcAniDistance( m_nVecAniIndexList[m_nActionIndex], (float)pStruct->dwLength, m_fPrevFrame, vDist );
			m_pRender->AddAniDistance( vDist );
		}

		if( m_nLoopCount > 0 || m_nLoopCount == -1 ) 
		{
			if( m_nLoopCount > 0 )
				m_nLoopCount--;

			OnLoopAction( m_fFrame, m_fPrevFrame );
			float fTemp = m_fFrame - (float)pStruct->dwLength;
			if( pStruct->szNextActionName == pStruct->szName ) {
				fTemp += (float)pStruct->dwNextActionFrame;
				if( fTemp > (float)pStruct->dwLength ) fTemp = (float)pStruct->dwLength;
			}
			SetAction( m_szAction.c_str(), fTemp, 0.f, true );
			OnNextAction( m_szAction.c_str() , pStruct->szNextActionName.c_str() );
		}
		else
		{

			OnFinishAction(m_szAction.c_str(), pStruct->szNextActionName.c_str(), LocalTime);

			if( pStruct->szNextActionName.empty() ) 
			{
				m_nActionIndex = -1;
				m_fFrame = -1.f;
				m_szAction.clear();
				return;
			}
			else 
			{
				SetAction( pStruct->szNextActionName.c_str(), ( pStruct->dwNextActionFrame == 0 ) ? 0.f : (float)pStruct->dwNextActionFrame - 0.001f, (float)pStruct->dwBlendFrame );
				if( m_fFrame > 0.f )
					m_fPrevFrame -= 1.f;
				OnNextAction( m_szAction.c_str() , pStruct->szNextActionName.c_str() );
			}
			
		}

		if( m_nActionIndex == -1 )
			return;

		pStruct = GetElement( m_nActionIndex );
		if( !pStruct ) return;
	}

	if( m_nCustomActionIndex != -1 ) 
	{
		ActionElementStruct *pCustomStruct = GetElement( m_nCustomActionIndex );
		if( pCustomStruct ) {
			float fFrame = ( ( m_LocalTime - m_CustomActionTime ) / 1000.f ) * m_fFps;
			if( fFrame > (float)pCustomStruct->dwLength ) 
			{
				ResetCustomAction();
				ProcessSignal( pCustomStruct, fFrame, m_fCustomPrevFrame );
			}
			else 
			{
				m_bCustomProcessSignal = true;
				float fTemp = m_fCustomPrevFrame;
				ProcessSignal( pCustomStruct, fFrame, m_fCustomPrevFrame );
				m_bCustomProcessSignal = false;

				if( m_fCustomPrevFrame == fTemp )
				{
					if( fFrame == m_fCustomPrevFrame )
						m_fCustomPrevFrame = fFrame + 0.001f;
					else
						m_fCustomPrevFrame = fFrame;
				}
			}

#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
			pCustomStruct->dwLastUsedTime = timeGetTime();
#endif
		}
	}
	// When CanMove is True and CustomAction is executed
	// Leave Input Signals unlocked for now since handling movement signals is convenient..
	// This way, if CanMove turns True mid-way, there is no need to insert the Input Signals below. 
	// Note: if not moving, Signals embedded in Move will not apply, so add them separately if needed
//	ProcessSignal( pStruct, m_fFrame, m_fPrevFrame );
	else
	{
		ProcessSignal( pStruct, m_fFrame, m_fPrevFrame );
	}
	if( m_fFrame == m_fPrevFrame )
		m_fPrevFrame = m_fFrame + 0.001f;
	else 
		m_fPrevFrame = m_fFrame;
}

void CDnActionBase::ProcessSignal( ActionElementStruct *pStruct, float fFrame, float fPrevFrame )
{
	CEtActionSignal *pSignal;
	float fFPS = m_fFps;
	if( m_fFps == 0.f ) return;
//	if( fFPS == 0.f ) fFPS = 60.f;
	OnBeginProcessSignal();
	for( DWORD i=0; i<pStruct->pVecSignalList.size(); i++ ) {
		pSignal = pStruct->pVecSignalList[i];

		if( pSignal->CheckSignal( fPrevFrame, fFrame ) == true )
		{
			LOCAL_TIME StartTime = m_LocalTime - (LOCAL_TIME)( 1000.f / fFPS * ( fFrame - pSignal->GetStartFrame() ) );
			LOCAL_TIME EndTime = m_LocalTime + (LOCAL_TIME)( 1000.f / fFPS * ( pSignal->GetEndFrame() - fFrame ) );

			OnSignal( (SignalTypeEnum)pSignal->GetSignalIndex(), pSignal->GetData(), m_LocalTime, StartTime, EndTime, pSignal->GetSignalListArrayIndex() );
		}

		if( !m_szActionQueue.empty() ) break;
	}
	OnEndProcessSignal();
}

void CDnActionBase::SetAction( const char *szActionName, float fFrame, float fBlendFrame, bool bLoop )
{
	int nIndex = GetElementIndex( szActionName );
	if( nIndex == -1 ) {
		return;
	}

	std::string szPrevAction = m_szAction;
	ActionElementStruct *pStruct = GetElement( (DWORD)nIndex );

#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
	if( pStruct ) pStruct->dwLastUsedTime = timeGetTime();
#endif

	bool bSameAction = false;
	if( m_nActionIndex == nIndex ) bSameAction = true;

	m_nActionIndex = nIndex;
	m_szAction = szActionName;

	m_fFrame = m_fPrevFrame = (float)fFrame;
	m_ActionTime = m_LocalTime - (LOCAL_TIME)( fFrame / m_fFps * 1000.f );
	if( m_ActionTime < 0 ) m_ActionTime = 0;

	if( m_pRender && m_nVecAniIndexList[nIndex] != -1 ) 
	{
		m_pRender->ChangeAnimation( m_nVecAniIndexList[nIndex], ( fFrame < 0.f ) ? 0.f : fFrame, fBlendFrame );
		if( bLoop ) m_pRender->SetPrevFrame( 0.f );
	}

	if( !bSameAction ) m_FirstActionTime = m_ActionTime;
	OnChangeAction( szPrevAction.c_str() );
}

void CDnActionBase::SetActionQueue( const char *szActionName, int nLoopCount, float fBlendFrame, float fStartFrame )
{
	m_szActionQueue = szActionName;
	m_fQueueBlendFrame = fBlendFrame;
	m_fQueueStartFrame = fStartFrame;

	m_nLoopCount = nLoopCount;

	m_fActionQueueDelay = 0.f;

	OnChangeActionQueue( m_szAction.c_str() );
}

void CDnActionBase::SetActionQueueDelay( const char *szActionName, int nLoopCount, float fBlendFrame, float fStartFrame, int nDelay )
{
	m_fActionQueueDelay = nDelay * 0.001f;

	m_szActionQueue = szActionName;
	m_fQueueBlendFrame = fBlendFrame;
	m_fQueueStartFrame = fStartFrame;

	m_nLoopCount = nLoopCount;
	OnChangeActionQueue( m_szAction.c_str() );
}


const char *CDnActionBase::GetCurrentAction()
{
	if( !m_szActionQueue.empty() ) return m_szActionQueue.c_str();
	return m_szAction.c_str();
}

void CDnActionBase::SetCustomAction( const char *szActionName, float fFrame )
{
	m_szCustomAction = szActionName;
	m_CustomActionTime = m_LocalTime - (LOCAL_TIME)( fFrame / m_fFps * 1000.f );
	m_nCustomActionIndex = GetElementIndex( szActionName );
	m_fCustomPrevFrame = fFrame - 1.f;

	OnChangeCustomAction();
}

bool CDnActionBase::IsCustomAction() 
{ 
	return ( m_nCustomActionIndex == -1 ) ? false : true;
}

void CDnActionBase::ResetCustomAction()
{
	m_szCustomAction.clear();
	m_nCustomActionIndex = -1;
}

bool CDnActionBase::LoadAction( const char *szFullPathName )
{
	bool bResult = false;
	bResult = CEtActionBase::LoadAction( szFullPathName );
	if( bResult ) CacheAniIndex();
	return bResult;
}

void CDnActionBase::ReloadAction()
{
	FreeAction();
	LoadAction( m_szFileName.c_str() );
}

void CDnActionBase::FreeAction()
{
	CEtActionBase::FreeAction();
}

void CDnActionBase::CalcAniIndex()
{
	if( !m_pVecActionElementList ) return;
	if( m_nVecAniIndexList.empty() ) return;

	for( DWORD i=0; i<m_pVecActionElementList->size(); i++ ) {
		int nIndex = -1;
		if( m_pRender && !(*m_pVecActionElementList)[i]->szLinkAniName.empty() ) {
			nIndex = m_pRender->GetAniIndex( (*m_pVecActionElementList)[i]->szLinkAniName.c_str() );
		}
		if( i < 0 || i >= m_nVecAniIndexList.size() ) {
			assert(0);
			continue;
		}
		m_nVecAniIndexList[ i ] = nIndex;
	}
}

void CDnActionBase::CacheAniIndex()
{
	m_nVecAniIndexList.clear();
	m_nVecAniIndexList.resize( m_pVecActionElementList->size(), -1 );
	CalcAniIndex();
}

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE

void CDnActionBase::CheckInitSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	switch( pSignal->GetSignalIndex() ) {
		case STE_Input:
			if( pSignal->GetStartFrame() == 0 && pSignal->GetEndFrame() == pElement->dwLength ) {
				pSignal->SetStartFrame( -1 );
			}
			if( pSignal->GetStartFrame() == 0 ) {
				pSignal->SetStartFrame( -1 );
			}
			break;
		case STE_InputHasPassiveSkill:
			if( pSignal->GetStartFrame() == 0 && pSignal->GetEndFrame() == pElement->dwLength ) {
				pSignal->SetStartFrame( -1 );
			}
			if( pSignal->GetStartFrame() == 0 ) {
				pSignal->SetStartFrame( -1 );
			}
			break;
		case STE_Hit:
			{
				HitStruct *pHit = (HitStruct *)pSignal->GetData();
				if( !pHit->vOffset ) pSignal->InsertVec3Table( (EtVector3*)&pHit->vOffset, EtVector3( 0.f, 0.f, 0.f ) );
			}
			break;
		case STE_EnvironmentEffect:
			{
				EnvironmentEffectStruct *pEnvi = (EnvironmentEffectStruct *)pSignal->GetData();
				if( !pEnvi->vRotate ) pSignal->InsertVec3Table( (EtVector3*)&pEnvi->vRotate, EtVector3( 0.f, 0.f, 0.f ) );
			}
			break;
		case STE_Sound:
			{
				SoundStruct *pStruct = (SoundStruct *)pSignal->GetData();
				pStruct->bLoadingComplete = false;
				pStruct->nVecSoundIndex = NULL;
			}
			break;
		case STE_Particle:
			{
				ParticleStruct *pStruct = (ParticleStruct *)pSignal->GetData();
				pStruct->nParticleDataIndex = -1;
			}
			break;
		case STE_FX:
			{
				FXStruct *pStruct = (FXStruct *)pSignal->GetData();
				pStruct->nFXDataIndex = -1;
			}
			break;
		case STE_Decal:
			{
				DecalStruct *pStruct = (DecalStruct *)pSignal->GetData();
				pStruct->nTextureIndex = -1;
			}
			break;
#ifdef PRE_ADD_MARK_PROJECTILE
		case STE_MarkProjectile:
			{
				MarkProjectileStruct *pStruct = (MarkProjectileStruct *)pSignal->GetData();
				pStruct->nTextureIndex = -1;
				pStruct->nFXDataIndex = -1;
				pStruct->nParticleDataIndex = -1;
			}
			break;
#endif // PRE_ADD_MARK_PROJECTILE
		case STE_ActionObject:
			{
				ActionObjectStruct *pStruct = (ActionObjectStruct *)pSignal->GetData();
				pStruct->nActionObjectIndex = -1;
			}
			break;
		case STE_AttachTrail:
			{
				AttachTrailStruct *pStruct = (AttachTrailStruct *)pSignal->GetData();
				pStruct->nTextureIndex = -1;
			}
			break;
		case STE_AttachEffectTrail:
			{
				AttachEffectTrailStruct *pStruct = (AttachEffectTrailStruct *)pSignal->GetData();
				pStruct->nTextureIndex = -1;

				char szHashStr[256];
				sprintf_s( szHashStr, "%s %s %d", pStruct->szTextureName , pStruct->szLinkBoneName , (int)(pStruct->fTrailWidth * 10.f) );
				pStruct->nHashCode = GetHashCode( szHashStr );

			}
			break;
		case STE_AttachLine:
			{
				AttachLineStruct* pStruct = (AttachLineStruct *)pSignal->GetData();
				pStruct->nTextureIndex = -1;
			}
			break;
		case STE_Projectile:
			{
				ProjectileStruct *pStruct = (ProjectileStruct *)pSignal->GetData();
				if( !pStruct->vOffset ) pSignal->InsertVec3Table( (EtVector3*)&pStruct->vOffset, EtVector3( 0.f, 0.f, 0.f ) );
				if( !pStruct->vDestPosition ) pSignal->InsertVec3Table( (EtVector3*)&pStruct->vDestPosition, EtVector3( 0.f, 0.f, 0.f ) );

				pStruct->nProjectileIndex = -1;
			}
			break;
		case STE_CameraEffect_Shake:
			{
				CameraEffect_ShakeStruct *pStruct = (CameraEffect_ShakeStruct *)pSignal->GetData();
				pStruct->nCameraEffectRefIndex = -1;
			}
			break;
		case STE_CameraEffect_RadialBlur:
			{
				CameraEffect_RadialBlurStruct *pStruct = (CameraEffect_RadialBlurStruct *)pSignal->GetData();
				pStruct->nCameraEffectRefIndex = -1;
			}
			break;
		case STE_AttachSwordTrail:
			{
				AttachSwordTrailStruct *pStruct = (AttachSwordTrailStruct *)pSignal->GetData();
				pStruct->nTextureIndex = -1;
				pStruct->nNormalTextureIndex = -1;
			}
			break;
		case STE_ShowSwordTrail:
			{
				ShowSwordTrailStruct *pStruct = (ShowSwordTrailStruct *)pSignal->GetData();
				pStruct->nTextureIndex = -1;
				pStruct->nNormalTextureIndex = -1;
			}
			break;
		case STE_OutlineFilter:
			{
				OutlineFilterStruct *pStruct = (OutlineFilterStruct *)pSignal->GetData();
				if( !pStruct->vColor ) pSignal->InsertVec4Table( (EtVector4*)&pStruct->vColor, EtVector4( 1.f, 1.f, 1.f, 0.f ) );
			}
			break;
		case STE_EyeLightTrail:
			{
				EyeLightTrailStruct *pStruct = (EyeLightTrailStruct *)pSignal->GetData();
				char szHashStr[256];
				sprintf_s( szHashStr, "%s %s %d", pStruct->szSkinName, pStruct->szLinkBoneName, pStruct->nLightTrailCount );
				pStruct->nHashCode = GetHashCode( szHashStr );
			}
			break;
		case STE_Gravity:
			{
				GravityStruct *pStruct = (GravityStruct *)pSignal->GetData();
				if( !pStruct->vOffset ) pSignal->InsertVec3Table( (EtVector3*)&pStruct->vOffset, EtVector3( 0.f, 0.f, 0.f ) );
			}
			break;
	}
}
#endif

void CDnActionBase::CheckPreSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	switch( pSignal->GetSignalIndex() ) {
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
		case STE_Input:
			if( pSignal->GetStartFrame() == 0 && pSignal->GetEndFrame() == pElement->dwLength ) {
				pSignal->SetStartFrame( -1 );
			}
			if( pSignal->GetStartFrame() == 0 ) {
				pSignal->SetStartFrame( -1 );
			}
			break;
		case STE_InputHasPassiveSkill:
			if( pSignal->GetStartFrame() == 0 && pSignal->GetEndFrame() == pElement->dwLength ) {
				pSignal->SetStartFrame( -1 );
			}
			if( pSignal->GetStartFrame() == 0 ) {
				pSignal->SetStartFrame( -1 );
			}
			break;
		case STE_Hit:
			{
				HitStruct *pHit = (HitStruct *)pSignal->GetData();
				if( !pHit->vOffset ) pSignal->InsertVec3Table( (EtVector3*)&pHit->vOffset, EtVector3( 0.f, 0.f, 0.f ) );
			}
			break;
		case STE_EnvironmentEffect:
			{
				EnvironmentEffectStruct *pEnvi = (EnvironmentEffectStruct *)pSignal->GetData();
				if( !pEnvi->vRotate ) pSignal->InsertVec3Table( (EtVector3*)&pEnvi->vRotate, EtVector3( 0.f, 0.f, 0.f ) );
			}
			break;
#endif
		case STE_Sound:
			{
				SoundStruct *pStruct = (SoundStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->bLoadingComplete = false;
#endif
#ifdef PRE_MOD_LOAD_SOUND_SIGNAL
#else
				pStruct->nVecSoundIndex = new std::vector<int>;

				LoadSoundStruct( pStruct );
#endif
			}
			break;
		case STE_Particle:
			{
				ParticleStruct *pStruct = (ParticleStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nParticleDataIndex = -1;
#endif
				if( strlen( pStruct->szFileName ) == 0 ) break;
				pStruct->nParticleDataIndex = EternityEngine::LoadParticleData( CEtResourceMng::GetInstance().GetFullName( pStruct->szFileName ).c_str() );
			}
			break;
		case STE_FX:
			{
				FXStruct *pStruct = (FXStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nFXDataIndex = -1;
#endif
				if( strlen( pStruct->szFileName ) == 0 ) break;
				pStruct->nFXDataIndex = EternityEngine::LoadEffectData( CEtResourceMng::GetInstance().GetFullName( pStruct->szFileName ).c_str() );
			}
			break;
		case STE_Decal:
			{
				DecalStruct *pStruct = (DecalStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nTextureIndex = -1;
#endif
				if( strlen(pStruct->szFileName) == 0 ) break;
				EtTextureHandle hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( pStruct->szFileName ).c_str() );
				if( hTexture ) pStruct->nTextureIndex = hTexture->GetMyIndex();				
			}
			break;
#ifdef PRE_ADD_MARK_PROJECTILE
		case STE_MarkProjectile:
			{
				MarkProjectileStruct *pStruct = (MarkProjectileStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nTextureIndex = -1;
				pStruct->nFXDataIndex = -1;
				pStruct->nParticleDataIndex = -1;
#endif
				switch( pStruct->nMarkType )
				{
					case PROJECTILE_MARK_DECAL:
						{
							if( strlen(pStruct->szFileName) == 0 ) break;
							EtTextureHandle hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( pStruct->szFileName ).c_str() );
							if( hTexture ) pStruct->nTextureIndex = hTexture->GetMyIndex();	
						}
						break;
					case PROJECTILE_MARK_FX:
						{
							if( strlen( pStruct->szFileName ) == 0 ) break;
							pStruct->nFXDataIndex = EternityEngine::LoadEffectData( CEtResourceMng::GetInstance().GetFullName( pStruct->szFileName ).c_str() );
						}
						break;
					case PROJECTILE_MARK_PARTICLE:
						{
							if( strlen( pStruct->szFileName ) == 0 ) break;
							pStruct->nParticleDataIndex = EternityEngine::LoadParticleData( CEtResourceMng::GetInstance().GetFullName( pStruct->szFileName ).c_str() );
						}
						break;
				}
			}
			break;
#endif // PRE_ADD_MARK_PROJECTILE
		case STE_ActionObject:
			{
				ActionObjectStruct *pStruct = (ActionObjectStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nActionObjectIndex = -1;
#endif
				if( strlen( pStruct->szSkinName ) == 0 || strlen( pStruct->szActionName ) == 0 ) break;

				DnEtcHandle hHandle = (new CDnEtcObject(false))->GetMySmartPtr();
				if( hHandle->Initialize( pStruct->szSkinName, pStruct->szAniName, pStruct->szActionName ) == false ) {
					SAFE_RELEASE_SPTR( hHandle );
					break;
				}
				hHandle->Show( false );

				pStruct->nActionObjectIndex = hHandle->GetMyIndex();
			}
			break;
		case STE_AttachTrail:
			{
				AttachTrailStruct *pStruct = (AttachTrailStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nTextureIndex = -1;
#endif
				if( strlen( pStruct->szTrailTextureName ) == 0 ) break;
				EtTextureHandle hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( pStruct->szTrailTextureName ).c_str() );
				if( hTexture ) pStruct->nTextureIndex = hTexture->GetMyIndex();
			}
			break;
		case STE_AttachEffectTrail:
			{
				AttachEffectTrailStruct *pStruct = (AttachEffectTrailStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nTextureIndex = -1;
#endif
				if( strlen( pStruct->szTextureName ) == 0 ) break;
				EtTextureHandle hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( pStruct->szTextureName ).c_str() );
				if( hTexture ) pStruct->nTextureIndex = hTexture->GetMyIndex();

				char szHashStr[256];
				sprintf_s( szHashStr, "%s %s %d", pStruct->szTextureName , pStruct->szLinkBoneName , (int)(pStruct->fTrailWidth * 10.f) );
				pStruct->nHashCode = GetHashCode( szHashStr );

			}
			break;
		case STE_AttachLine:
			{
				AttachLineStruct* pStruct = (AttachLineStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nTextureIndex = -1;
#endif
				if( strlen( pStruct->szTextureFileName ) == 0 ) break;

				EtTextureHandle hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( pStruct->szTextureFileName ).c_str() );
				if( hTexture ) pStruct->nTextureIndex = hTexture->GetMyIndex();
			}
			break;
		case STE_Projectile:
			{
				ProjectileStruct *pStruct = (ProjectileStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nProjectileIndex = -1;
#endif
				if( pStruct->nWeaponTableID == 0 ) break;
			
				CDnProjectile *pProjectile = new CDnProjectile( CDnActor::Identity(), false, false );
				pProjectile->CDnWeapon::Initialize( pStruct->nWeaponTableID, -1 );
				pProjectile->CDnWeapon::CreateObject( NULL , false );
				// Projectiles inherit from Weapon, and Weapon does not load sound or effect files in towns or PVPVillage.
				// However, since resources preloaded in town are reused by reference without reloading, the above effects and sounds fail to play upon leaving the game server.
				// If the projectile is included in a character act, configure it to load sound in town even if it is a weapon type.
				pProjectile->ShowWeapon( false );
				pStruct->nProjectileIndex = pProjectile->GetMyIndex();

#ifdef PRE_ADD_PROJECTILE_RANDOM_WEAPON
				if( pStruct->RandomWeaponParam && strlen( pStruct->RandomWeaponParam ) > 0 )
				{
					std::vector<std::string> tokens;
					TokenizeA( pStruct->RandomWeaponParam, tokens, "/" );
					if( tokens.size() > 1 )
					{
						for( int i=1; i<static_cast<int>( tokens.size() ); i++ )
						{
							std::vector<std::string> tokensparam;
							TokenizeA( tokens[i].c_str(), tokensparam, ";" );
							if( tokensparam.size() > 0 )
							{
								CDnProjectile *pProjectile = new CDnProjectile( CDnActor::Identity(), false, false );
								pProjectile->CDnWeapon::Initialize( atoi( tokensparam[0].c_str() ), -1 );
								pProjectile->CDnWeapon::CreateObject( NULL , false );
								pProjectile->ShowWeapon( false );
							}
							tokensparam.clear();
						}
					}
					tokens.clear();
				}
#endif // PRE_ADD_PROJECTILE_RANDOM_WEAPON
			}
			break;
		case STE_AttachSwordTrail:
			{
				AttachSwordTrailStruct *pStruct = (AttachSwordTrailStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nTextureIndex = -1;
				pStruct->nNormalTextureIndex = -1;
#endif
				if( strlen( pStruct->szTextureFileName ) > 0 ) {
					EtTextureHandle hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( pStruct->szTextureFileName ).c_str() );
					if( hTexture ) pStruct->nTextureIndex = hTexture->GetMyIndex();
				}
				if( strlen( pStruct->szNormalTextureFileName ) > 0 ) {
					EtTextureHandle hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( pStruct->szNormalTextureFileName ).c_str() );
					if( hTexture ) pStruct->nNormalTextureIndex = hTexture->GetMyIndex();
				}
			}
			break;
		case STE_ShowSwordTrail:
			{
				ShowSwordTrailStruct *pStruct = (ShowSwordTrailStruct *)pSignal->GetData();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->nTextureIndex = -1;
				pStruct->nNormalTextureIndex = -1;
#endif
				if( strlen( pStruct->szTextureFileName ) > 0 ) {
					EtTextureHandle hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( pStruct->szTextureFileName ).c_str() );
					if( hTexture ) pStruct->nTextureIndex = hTexture->GetMyIndex();
				}
				if( strlen( pStruct->szNormalTextureFileName ) > 0 ) {
					EtTextureHandle hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( pStruct->szNormalTextureFileName ).c_str() );
					if( hTexture ) pStruct->nNormalTextureIndex = hTexture->GetMyIndex();
				}
			}
			break;
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
		case STE_OutlineFilter:
			{
				OutlineFilterStruct *pStruct = (OutlineFilterStruct *)pSignal->GetData();
				if( !pStruct->vColor ) pSignal->InsertVec4Table( (EtVector4*)&pStruct->vColor, EtVector4( 1.f, 1.f, 1.f, 0.f ) );
			}
			break;
		case STE_EyeLightTrail:
			{
				EyeLightTrailStruct *pStruct = (EyeLightTrailStruct *)pSignal->GetData();
				char szHashStr[256];
				sprintf_s( szHashStr, "%s %s %d", pStruct->szSkinName, pStruct->szLinkBoneName, pStruct->nLightTrailCount );
				pStruct->nHashCode = GetHashCode( szHashStr );
			}
			break;
		case STE_Gravity:
			{
				GravityStruct *pStruct = (GravityStruct *)pSignal->GetData();
				if( !pStruct->vOffset ) pSignal->InsertVec3Table( (EtVector3*)&pStruct->vOffset, EtVector3( 0.f, 0.f, 0.f ) );
			}
			break;
#endif

		case STE_SummonMonster:
			{
				SummonMonsterStruct* pStruct = (SummonMonsterStruct*)pSignal->GetData();
				if( pStruct->bPreLoad == TRUE &&
					0 == s_mapAcademicSummonMonsterPreload.count(pStruct->MonsterID) )
				{
					DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );
					if( pSox->IsExistItem( pStruct->MonsterID ) ) 
					{
						int nActorTableID = pSox->GetFieldFromLablePtr( pStruct->MonsterID, "_ActorTableID" )->GetInteger();
						DnActorHandle hActor = CreateActor( nActorTableID, false, false, false );
						if( hActor )
						{
							s_mapAcademicSummonMonsterPreload[ pStruct->MonsterID ] = hActor;
						}
					}
				}
			}
			break;
#ifdef PRE_ADD_CHANGE_MONSTER_SKIN
		case STE_ChangeSkin:
			{
				ChangeSkinStruct *pStruct = (ChangeSkinStruct *)pSignal->GetData();
				if( strlen( pStruct->szSkinFileName ) == 0 || strlen( pStruct->szAniFileName ) == 0 ) break;

				EtAniObjectHandle hHandle = EternityEngine::CreateAniObject( pStruct->szSkinFileName, pStruct->szAniFileName, false );	
				if( ! hHandle ) break;				

				pStruct->nObjectSkinIndex = hHandle->GetMyIndex();
			}
			break;
#endif 
	}
}

void CDnActionBase::CheckPostSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	switch( pSignal->GetSignalIndex() ) {
		case STE_Sound:
			{
				SoundStruct *pStruct = (SoundStruct *)pSignal->GetData();
				if( pStruct->nVecSoundIndex == NULL ) break;
				for( DWORD i=0; i<pStruct->nVecSoundIndex->size(); i++ ) {
					CEtSoundEngine::GetInstance().RemoveSound( (*pStruct->nVecSoundIndex)[i] );
				}
				SAFE_DELETE(pStruct->nVecSoundIndex);
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
				pStruct->bLoadingComplete = false;
#endif
			}
			break;
		case STE_Particle:
			{
				ParticleStruct *pStruct = (ParticleStruct *)pSignal->GetData();
				if( pStruct->nParticleDataIndex == -1 ) break;
				EternityEngine::DeleteParticleData( pStruct->nParticleDataIndex );
				pStruct->nParticleDataIndex = -1;
			}
			break;
		case STE_FX:
			{
				FXStruct *pStruct = (FXStruct *)pSignal->GetData();
				if( pStruct->nFXDataIndex == -1 ) break;
				EternityEngine::DeleteEffectData( pStruct->nFXDataIndex );
				pStruct->nFXDataIndex = -1;
			}
			break;
		case STE_Decal:
			{
				DecalStruct *pStruct = (DecalStruct *)pSignal->GetData();
				if( pStruct->nTextureIndex == -1 ) break;
				if( pStruct->nTextureIndex != -1 ) {
					EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nTextureIndex );
					SAFE_RELEASE_SPTR( hTexture );
					pStruct->nTextureIndex = -1;
				}
			}
			break;
#ifdef PRE_ADD_MARK_PROJECTILE
		case STE_MarkProjectile:
			{
				MarkProjectileStruct *pStruct = (MarkProjectileStruct *)pSignal->GetData();

				switch( pStruct->nMarkType )
				{
					case PROJECTILE_MARK_DECAL:
						{
							if( pStruct->nTextureIndex == -1 ) break;
							if( pStruct->nTextureIndex != -1 ) 
							{
								EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nTextureIndex );
								SAFE_RELEASE_SPTR( hTexture );
								pStruct->nTextureIndex = -1;
							}
						}
						break;
					case PROJECTILE_MARK_FX:
						{
							if( pStruct->nFXDataIndex == -1 ) break;
							EternityEngine::DeleteEffectData( pStruct->nFXDataIndex );
							pStruct->nFXDataIndex = -1;
						}
						break;
					case PROJECTILE_MARK_PARTICLE:
						{
							if( pStruct->nParticleDataIndex == -1 ) break;
							EternityEngine::DeleteParticleData( pStruct->nParticleDataIndex );
							pStruct->nParticleDataIndex = -1;
						}
						break;
				}
			}
			break;
#endif // PRE_ADD_MARK_PROJECTILE
		case STE_ActionObject:
			{
				ActionObjectStruct *pStruct = (ActionObjectStruct *)pSignal->GetData();
				if( pStruct->nActionObjectIndex == -1 ) break;

				DnEtcHandle hHandle = CDnEtcObject::GetSmartPtr( pStruct->nActionObjectIndex );
				SAFE_RELEASE_SPTR( hHandle );
				pStruct->nActionObjectIndex = -1;
			}
			break;
		case STE_AttachTrail:
			{
				AttachTrailStruct *pStruct = (AttachTrailStruct *)pSignal->GetData();
				if( pStruct->nTextureIndex == -1 ) break;
				EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nTextureIndex );
				SAFE_RELEASE_SPTR( hTexture );
				pStruct->nTextureIndex = -1;
			}
			break;
		case STE_AttachEffectTrail:
			{
				AttachEffectTrailStruct* pStruct = (AttachEffectTrailStruct *)pSignal->GetData();
				if( pStruct->nTextureIndex == -1 ) break;

				EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nTextureIndex );
				SAFE_RELEASE_SPTR( hTexture );
				pStruct->nTextureIndex = -1;
			}
			break;
		case STE_AttachLine:
			{
				AttachLineStruct* pStruct = (AttachLineStruct *)pSignal->GetData();
				if( pStruct->nTextureIndex == -1 ) break;

				EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nTextureIndex );
				SAFE_RELEASE_SPTR( hTexture );
				pStruct->nTextureIndex = -1;
			}
			break;

		case STE_Projectile:
			{
				ProjectileStruct *pStruct = (ProjectileStruct *)pSignal->GetData();
				if( pStruct->nWeaponTableID == 0 ) break;
				if( pStruct->nProjectileIndex == -1 ) break;

				DnWeaponHandle hWeapon = CDnWeapon::GetSmartPtr( pStruct->nProjectileIndex );
				SAFE_RELEASE_SPTR( hWeapon );
				pStruct->nProjectileIndex = -1;
			}
			break;
		case STE_AttachSwordTrail:
			{
				AttachSwordTrailStruct *pStruct = (AttachSwordTrailStruct *)pSignal->GetData();
				if( pStruct->nTextureIndex != -1 ) {
					EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nTextureIndex );
					SAFE_RELEASE_SPTR( hTexture );
					pStruct->nTextureIndex = -1;
				}
				if( pStruct->nNormalTextureIndex != -1 ) {
					EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nNormalTextureIndex );
					SAFE_RELEASE_SPTR( hTexture );
					pStruct->nNormalTextureIndex = -1;
				}
			}
			break;			
		case STE_ShowSwordTrail:
			{
				ShowSwordTrailStruct *pStruct = (ShowSwordTrailStruct *)pSignal->GetData();
				if( pStruct->nTextureIndex != -1 ) { 
					EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nTextureIndex );
					SAFE_RELEASE_SPTR( hTexture );
					pStruct->nTextureIndex = -1;
				}
				if( pStruct->nNormalTextureIndex != -1 ) {
					EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nNormalTextureIndex );
					SAFE_RELEASE_SPTR( hTexture );
					pStruct->nNormalTextureIndex = -1;
				}
			}
			break;

		case STE_SummonMonster:
			{
				SummonMonsterStruct* pStruct = (SummonMonsterStruct*)pSignal->GetData();
				if( pStruct->bPreLoad == TRUE &&
					0 < s_mapAcademicSummonMonsterPreload.count(pStruct->MonsterID) )
				{
					DnActorHandle hActor = s_mapAcademicSummonMonsterPreload[ pStruct->MonsterID ];
					SAFE_RELEASE_SPTR( hActor );
					s_mapAcademicSummonMonsterPreload.erase( pStruct->MonsterID );
				}
			}
			break;
#ifdef PRE_ADD_CHANGE_MONSTER_SKIN
		case STE_ChangeSkin:
			{
				ChangeSkinStruct *pStruct = (ChangeSkinStruct *)pSignal->GetData();
				if( pStruct->nObjectSkinIndex == -1 ) break;

				EtAniObjectHandle hHandle = static_cast<EtAniObjectHandle>( CEtObject::GetSmartPtr( pStruct->nObjectSkinIndex ) );
				SAFE_RELEASE_SPTR( hHandle );

				pStruct->nObjectSkinIndex = -1;
			}
			break;
#endif 

	}
}

void CDnActionBase::SetFPS( float fValue )
{
	if( 0.0f < fValue )
	{
		m_ActionTime = m_LocalTime - (LOCAL_TIME)( m_fFrame / fValue * 1000.f );
		if( IsCustomAction() ) {
			float fFrame = ( ( m_LocalTime - m_CustomActionTime ) / 1000.f ) * m_fFps;
			m_CustomActionTime = m_LocalTime - (LOCAL_TIME)( fFrame / fValue * 1000.f );
		}
	}
	else
	{
		// When called with fValue == 0.0f.
		m_ActionTime = m_LocalTime;
		if( IsCustomAction() ) {
			m_CustomActionTime = m_LocalTime;
		}
	}

	float fPrevFps = m_fFps;
	m_fFps = fValue;
	if( fPrevFps == 0.f && fValue != 0.f && m_fFrame == 0.f ) {
		m_fPrevFrame = -1.f;
	}

	if( m_pRender ) 
		m_pRender->SetFPS( fValue );
}

float CDnActionBase::GetFPS()
{
	return m_fFps;
}


bool CDnActionBase::IsSignalRange( SignalTypeEnum Type, int nSignalIndex )
{
	if( m_nActionIndex == -1 ) return false;
	ActionElementStruct *pStruct = GetElement( m_nActionIndex );
	if( !pStruct ) return false;

	std::map<int, std::vector<CEtActionSignal *>>::iterator it = pStruct->MapSearch.find( Type );
	if( it == pStruct->MapSearch.end() ) return false;

	CEtActionSignal *pSignal;
	for( DWORD i=0; i<it->second.size(); i++ ) {
		pSignal = it->second[i];

		if( m_fFrame >= pSignal->GetStartFrame() && m_fFrame <= pSignal->GetEndFrame() ) 
		{
			if( nSignalIndex != -1 && nSignalIndex != pSignal->GetSignalListArrayIndex() ) 
				continue;

			return true;
		}
	}
	return false;
}

int CDnActionBase::GetCachedAniIndex( const char *szActionName )
{
	int nActionIndex = GetElementIndex( szActionName );
	if( ( nActionIndex < 0 ) || ( nActionIndex >= ( int )m_nVecAniIndexList.size() ) )
	{
		return -1;
	}
	return m_nVecAniIndexList[ nActionIndex ];
}


void CDnActionBase::LoadSoundStruct( SoundStruct *pStruct )
{
	if( strlen( pStruct->szFileName ) == 0 ) {
		pStruct->bLoadingComplete = true;
		return;
	}
	if( pStruct->nVecSoundIndex == NULL ) return;
	if( pStruct->bLoadingComplete ) return;
	if( pStruct->bUseRandomSound ) {
		bool bFind;
		char szTempName[256] = { 0, };
		_GetOriginalRandomBaseName( szTempName, pStruct->szFileName );
		for( int i=0; ; i++ ) {
			CFileNameString szSoundName = CEtResourceMng::GetInstance().GetFullNameRandom( CFileNameString(szTempName), i, &bFind );
			if( bFind == false ) break;
			int nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( szSoundName.c_str(), true, false );
			if( nSoundIndex != -1 ) pStruct->nVecSoundIndex->push_back( nSoundIndex );
		}
		// If filename is _c etc. without _a or _b, setting random would fail to read the file since _a is missing, so
		// Let's check it once here.
		CFileNameString szTemp = CEtResourceMng::GetInstance().GetFullName( pStruct->szFileName, &bFind );
		if( bFind ) {
			int nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( szTemp.c_str(), true, false );
			if( nSoundIndex != -1 ) {
				if( std::find( pStruct->nVecSoundIndex->begin(), pStruct->nVecSoundIndex->end(), nSoundIndex ) == pStruct->nVecSoundIndex->end() ) {
					pStruct->nVecSoundIndex->push_back( nSoundIndex );
				}
				else {
					CEtSoundEngine::GetInstance().RemoveSound( nSoundIndex );
				}
			}
		}
#ifndef _FINAL_BUILD
		/*
		if( pStruct->nVecSoundIndex->empty() ) {
			WCHAR wszStr[256];
swprintf_s( wszStr, L"Sound file not found. : ActionFile : %S, Action :%S, File : %S(random files)\n", m_szFileName.c_str(), pElement->szName.c_str(), pStruct->szFileName );
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"Debug", wszStr, false);
		}
		*/
#endif
	}
	else {
		bool bFind;
		CFileNameString szTemp = CEtResourceMng::GetInstance().GetFullName( pStruct->szFileName, &bFind );
		if( bFind == false ) {
			pStruct->bLoadingComplete = true;
			return;
		}
		int nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( szTemp.c_str(), true, false );
		if( nSoundIndex != -1 ) pStruct->nVecSoundIndex->push_back( nSoundIndex );
#ifndef _FINAL_BUILD
		/*
		else {
			WCHAR wszStr[256];
swprintf_s( wszStr, L"Sound file not found. : ActionFile : %S, Action :%S, File : %S\n", m_szFileName.c_str(), pElement->szName.c_str(), pStruct->szFileName );
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"Debug", wszStr);
		}
		*/
#endif
	}
	pStruct->bLoadingComplete = true;
}

#ifdef PRE_ADD_ENABLE_EFFECT_SIGNAL
bool CDnActionBase::SetEnableSignal( SignalTypeEnum SignalType, void* pSignalData )
{
	switch( SignalType )
	{	
	case STE_Particle:
		{	
			ParticleStruct* pData = static_cast<ParticleStruct*>( pSignalData );
			return pData->bEnable == TRUE ? true : false;
		}
		break;
	case STE_FX:
		{
			FXStruct* pData = static_cast<FXStruct*>( pSignalData );
			return pData->bEnable == TRUE ? true : false;
		}
		break;
	case STE_ActionObject:
		{
			ActionObjectStruct* pData = static_cast<ActionObjectStruct*>( pSignalData );	
			return pData->bEnable == TRUE ? true : false;
		}
		break;
	}

	return true;
}
#endif

#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
void CDnActionBase::SetMemoryOptimize( int nStep )
{
	if( nStep < 1 || nStep > 6 )
		return;

	std::vector<int> vecOptimizeSignalIndex;
	switch( nStep )
	{
	case 1:
		break;
	case 2:
		vecOptimizeSignalIndex.push_back( STE_Sound );
		break;
	case 3:
		vecOptimizeSignalIndex.push_back( STE_Sound );
		vecOptimizeSignalIndex.push_back( STE_ActionObject );
		break;
	case 4:
		vecOptimizeSignalIndex.push_back( STE_Sound );
		vecOptimizeSignalIndex.push_back( STE_ActionObject );
		vecOptimizeSignalIndex.push_back( STE_Particle );
		break;
	case 5:
		vecOptimizeSignalIndex.push_back( STE_Sound );
		vecOptimizeSignalIndex.push_back( STE_ActionObject );
		vecOptimizeSignalIndex.push_back( STE_Particle );
		vecOptimizeSignalIndex.push_back( STE_FX );
		break;
	case 6:
		vecOptimizeSignalIndex.push_back( STE_Sound );
		vecOptimizeSignalIndex.push_back( STE_ActionObject );
		vecOptimizeSignalIndex.push_back( STE_Particle );
		vecOptimizeSignalIndex.push_back( STE_FX );
		vecOptimizeSignalIndex.push_back( STE_Decal );
		break;
	}

	CEtActionCoreMng::GetInstance().SetOptimizeSignalIndexArray( vecOptimizeSignalIndex );
}
#endif