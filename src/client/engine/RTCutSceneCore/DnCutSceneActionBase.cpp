#include "StdAfx.h"
#include "DnCutSceneActionBase.h"
#include "DnCutSceneActProp.h"
#include "EtActionSignal.h"
#include "LuaDelegate.h"
#include "IResourcePathFinder.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


const float FRAME_PER_SECOND = 60.0f;


CDnCutSceneActionBase::CDnCutSceneActionBase( void ) : m_iActionIndex( 0 ),
													   m_pEntity( NULL ),
													   m_pPathFinder( NULL ),
													   m_fLastProcessSignalFrame( 0.0f ),
													   m_LocalTime( 0 ),
													   m_fFrame( 0.0f )
{
	m_pCheckPreSignalFunc = CDnCutSceneActionBase::CheckPreSignal;
	m_pCheckPostSignalFunc = CDnCutSceneActionBase::CheckPostSignal;
}

CDnCutSceneActionBase::~CDnCutSceneActionBase(void)
{

}


void CDnCutSceneActionBase::Initialize( IDnCutSceneRenderObject* pEntity )
{
	m_pEntity = pEntity;
}


void CDnCutSceneActionBase::SetResPathFinder( IResourcePathFinder* pResPathFinder )
{
	m_pPathFinder = pResPathFinder;
}


// Preload resources used in the signal.
void CDnCutSceneActionBase::CheckPreSignal( ActionElementStruct* pElement, int nElementIndex, CEtActionSignal* pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	switch( pSignal->GetSignalIndex() )
	{
		case STE_Particle:
			{
				ParticleStruct* pStruct = static_cast<ParticleStruct*>(pSignal->GetData());
				pStruct->nParticleDataIndex = -1;
				
				if( strlen(pStruct->szFileName) == 0 )
					break;
				
				string strFullPath;
				if( pActionBase ) ((CDnCutSceneActionBase*)pActionBase)->GetResPathFinder()->GetFullPath( pStruct->szFileName, strFullPath );
				pStruct->nParticleDataIndex = EternityEngine::LoadParticleData( strFullPath.c_str() );
			}
			break;

		case STE_Sound:
			{
				SoundStruct* pStruct = static_cast<SoundStruct*>(pSignal->GetData());
				pStruct->nVecSoundIndex = new std::vector<int>;
				if( strlen(pStruct->szFileName) == 0 ) break;

				string strFullPath;
				if( pActionBase ) ((CDnCutSceneActionBase*)pActionBase)->GetResPathFinder()->GetFullPath( pStruct->szFileName, strFullPath );
				int nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( strFullPath.c_str(), true, false );
				if( nSoundIndex != -1 ) pStruct->nVecSoundIndex->push_back( nSoundIndex );
			}
			break;

		case STE_FX:
			{
				FXStruct* pStruct = static_cast<FXStruct*>(pSignal->GetData());
				pStruct->nFXDataIndex = -1;

				if( strlen(pStruct->szFileName) == 0 )
					break;

				string strFullPath;
				if( pActionBase ) ((CDnCutSceneActionBase*)pActionBase)->GetResPathFinder()->GetFullPath( pStruct->szFileName, strFullPath );
				pStruct->nFXDataIndex = EternityEngine::LoadEffectData( strFullPath.c_str() );
			}
			break;
	}
}

void CDnCutSceneActionBase::CheckPostSignal( ActionElementStruct* pElement, int nElementIndex, CEtActionSignal* pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	switch( pSignal->GetSignalIndex() )
	{
		case STE_Particle:
			{
				ParticleStruct* pParticleStruct = static_cast<ParticleStruct*>(pSignal->GetData());
				if( -1 == pParticleStruct->nParticleDataIndex )
					break;

				EternityEngine::DeleteParticleData( pParticleStruct->nParticleDataIndex );
			}
			break;

		case STE_Sound:
			{
				SoundStruct* pSoundStruct = static_cast<SoundStruct*>(pSignal->GetData());
				if( pSoundStruct->nVecSoundIndex == NULL ) break;

				for( DWORD i=0; i<pSoundStruct->nVecSoundIndex->size(); i++ ) {
					CEtSoundEngine::GetInstance().RemoveSound( (*pSoundStruct->nVecSoundIndex)[i] );
				}
				SAFE_DELETE( pSoundStruct->nVecSoundIndex );
			}
			break;

		case STE_FX:
			{
				FXStruct* pFXStruct = static_cast<FXStruct*>(pSignal->GetData());
				if( -1 == pFXStruct->nFXDataIndex )
					break;

				EternityEngine::DeleteEffectData( pFXStruct->nFXDataIndex );
			}
			break;

		case STE_ActionObject:
			{
				
			}
			break;
	}
}

bool CDnCutSceneActionBase::LoadAction( const char* szFullPathName )
{
	bool bResult = false;

	bResult = CEtActionBase::LoadAction( szFullPathName );

	if( bResult ) 
		CacheAniIndex();

	return bResult;
}


void CDnCutSceneActionBase::FreeAction( void )
{
	CEtActionBase::FreeAction();
}

void CDnCutSceneActionBase::CacheAniIndex( void )
{
	m_vlAniIndexList.clear();
	for( DWORD i = 0; i < m_pVecActionElementList->size(); ++i )
	{
		int iIndex = -1;

		ActionElementStruct* pAniElement = m_pVecActionElementList->at(i);

		if( !pAniElement->szLinkAniName.empty() )
		{
			iIndex = static_cast<CDnCutSceneActProp*>(m_pEntity)->GetAniIndex( pAniElement->szLinkAniName.c_str() );
		}

		m_vlAniIndexList.push_back( iIndex );
	}
}


void CDnCutSceneActionBase::ProcessSignal( ActionElementStruct* pActionElement, LOCAL_TIME LocalTime, float fFrame, float fPrevFrame )
{
	m_LocalTime = LocalTime;

	if( fFrame < m_fLastProcessSignalFrame )
		m_fLastProcessSignalFrame = fFrame-0.01f;
	if( m_fLastProcessSignalFrame < 0.0f )
		m_fLastProcessSignalFrame = 0.0f;

	CEtActionSignal* pSignal = NULL;
	int iNumSignal = (int)pActionElement->pVecSignalList.size();
	for( int iSignal = 0; iSignal < iNumSignal; ++iSignal )
	{
		pSignal = pActionElement->pVecSignalList.at( iSignal );
		if( /*!((0.0f == m_fLastProcessSignalFrame) && (0.0f == fFrame)) &&
			( */
			(pSignal->CheckSignal( m_fLastProcessSignalFrame, fFrame ) == true) ||
			( (m_setProcessedSignals.find(pSignal) == m_setProcessedSignals.end()) &&
			  ((pSignal->GetStartFrame() <= m_fLastProcessSignalFrame) && pSignal->GetEndFrame() <= fFrame)) //)
		  )
		{
			LOCAL_TIME StartTime = m_LocalTime - (LOCAL_TIME)( 1000.f / FRAME_PER_SECOND * (fFrame - pSignal->GetStartFrame()) );
			LOCAL_TIME EndTime = m_LocalTime + (LOCAL_TIME)( 1000.f / FRAME_PER_SECOND * (pSignal->GetEndFrame() - fFrame) );

			OnSignal( pSignal->GetSignalIndex(), pSignal->GetSignalListArrayIndex(), pSignal->GetData(), m_LocalTime, StartTime, EndTime );
			m_setProcessedSignals.insert( pSignal );
		}
	}

	m_fLastProcessSignalFrame = fFrame;
}


void CDnCutSceneActionBase::OnChangeAction( void )
{
	m_fLastProcessSignalFrame = 0.0f;
	m_setProcessedSignals.clear();
}


void CDnCutSceneActionBase::SetAction( const char* pActionName, float fFrame, float fBlendFrame )
{
	int iIndex = GetElementIndex( pActionName );
	if( -1 == iIndex ) 
	{
		OutputDebug( "Can't find Action : %s, %s\n", m_szFileName.c_str(), pActionName );
		return;
	}

	m_iActionIndex = iIndex;
	m_fFrame = fFrame;
	m_strActionName.assign( pActionName );

	if( -1 != m_vlAniIndexList[ iIndex ] )
	{
		if( m_pEntity )
			m_pEntity->ChangeAnimation( m_vlAniIndexList.at(iIndex), fFrame, fBlendFrame );
	}
}


void CDnCutSceneActionBase::SetAction( int iAniIndex )
{
	int iIndex = iAniIndex;//GetElementIndex( pActionName );
	const char* pActionName = GetElement(iIndex)->szName.c_str();
	if( -1 == iIndex ) 
	{
		OutputDebug( "Can't find Action : %s, %s\n", m_szFileName.c_str(), pActionName );
		return;
	}

	ActionElementStruct* pStruct = GetElement( (DWORD)iIndex );

	m_iActionIndex = iIndex;
	m_strActionName.assign( pActionName );

	if( m_pEntity )
		m_pEntity->ChangeAnimation( iAniIndex, 0.0f, 0.0f );

	//if( -1 != m_vlAniIndexList[ iIndex ] )
	//{
	//	if( m_pEntity )
	//		m_pEntity->ChangeAnimation( m_vlAniIndexList.at(iIndex)/*, fFrame, fBlendFrame*/ );
	//}
}


const char* CDnCutSceneActionBase::GetCurrentAction( void )
{
	return (m_strActionName.empty() ? NULL : m_strActionName.c_str());
}