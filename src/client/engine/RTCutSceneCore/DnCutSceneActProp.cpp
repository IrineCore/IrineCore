#include "StdAfx.h"
#include "DnCutSceneActProp.h"
#include "DnCutSceneWorldSector.h"
#include "DNTableFile.h"
#include "DnCutSceneActor.h"
#include "EtActionSignal.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnCutSceneActProp::CDnCutSceneActProp(void) : m_pActionObject( NULL )
{
	EtMatrixIdentity( &m_matWorld );
	CDnCutSceneActionBase::Initialize( this );

	m_fFrame = 0.0f;
	m_bDestroy = false;
	m_pActionObject = NULL;
}

CDnCutSceneActProp::~CDnCutSceneActProp(void)
{
	SAFE_RELEASE_SPTR( m_Handle );
	SAFE_DELETE( m_pActionObject );
	FreeAction();
}


bool CDnCutSceneActProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, 
									 EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	bool bResult = CDnCutSceneWorldProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
	if( false == bResult )
		return bResult;

	//UpdateMatrix();

	CDnCutSceneWorldSector* pSector = dynamic_cast<CDnCutSceneWorldSector*>(m_pParentSector);
	if( pSector )
	{
		pSector->InsertProcessProp( this );
	}

	m_Handle->EnableShadowReceive( true );
	//m_Handle->SetStaticCollisionObject( true );

	

	return true;
}


bool CDnCutSceneActProp::InitializeTable( DNTableFileFormat*  pPropTable, int iTableID )
{
	if( CDnCutSceneWorldProp::InitializeTable( pPropTable, iTableID ) == false )
		return false;

	// In ActionObject signal, when loading a destructible action on the same skin, values are already set so 
	// In that case, do not load the action from the table. It can enter an infinite loop.
	if( m_strAniName.empty() )
		m_strAniName = pPropTable->GetFieldFromLablePtr( iTableID, "_AniName" )->GetString();

	if( m_strActName.empty() )
		m_strActName = pPropTable->GetFieldFromLablePtr( iTableID, "_ActName" )->GetString();

	m_strDefaultActionName = pPropTable->GetFieldFromLablePtr( iTableID, "_DefaultAction" )->GetString();

	// Just return if reaching here without an action name
	if( m_strActName.empty() )
		return false;

	//_ASSERT( !m_strActName.empty() && "Action prop has no action file name." );

	return true;
	
}


bool CDnCutSceneActProp::_CreateObject( void )
{
	m_Handle = EternityEngine::CreateAniObject( GetPropName(), (m_strAniName.empty() ? NULL : m_strAniName.c_str() ) );

	if( !m_strActName.empty() )
	{
		// For debugging
		//if( stricmp(m_strActName.c_str(), "apocalypsenest_monstersummoner01.act") == 0 )
		//{
		//	int i = 0;
		//	++i;
		//}

		if( LoadAction( CEtResourceMng::GetInstance().GetFullName(m_strActName.c_str()).c_str() ) == false ) return false;
	}

	if( m_Handle )
	{
		m_Handle->SetCalcPositionFlag( CALC_POSITION_X | CALC_POSITION_Y | CALC_POSITION_Z );
		m_Handle->GetSkinInstance()->SetRenderAlphaTwoPass( true );
	}

	if( IsExistAction( m_strDefaultActionName.c_str() ) )
		SetAction( m_strDefaultActionName.c_str(), 0.0f, 0.0f );

	// Call CheckPreSignal to preload items to be read in advance
	for( DWORD i = 0; i < GetElementCount(); i++ ) 
	{
		for( DWORD k = 0; k < GetElement(i)->pVecSignalList.size(); k++ ) 
		{
			CheckActionObjectSignal( GetElement(i), GetElement(i)->pVecSignalList[k] );
		}
	}
	
	return true;
}

void CDnCutSceneActProp::CheckActionObjectSignal( ActionElementStruct* pElement, CEtActionSignal* pSignal )
{
	switch( pSignal->GetSignalIndex() )
	{
		case STE_ActionObject:
			{
				ActionObjectStruct *pStruct = (ActionObjectStruct *)pSignal->GetData();
				pStruct->nActionObjectIndex = -1;
				if( strlen( pStruct->szSkinName ) == 0 || strlen( pStruct->szActionName ) == 0 ) 
					break;

				SAFE_DELETE( m_pActionObject );
				m_pActionObject = OnCreateActionObject( pStruct->szSkinName, pStruct->szAniName, pStruct->szActionName );

				//CDnCutSceneActor* pNewActor = new CDnCutSceneActor;
				//pNewActor->SetResPathFinder( m_pPathFinder );
				//pNewActor->LoadSkin( pStruct->szSkinName, pStruct->szAniName );
				//pNewActor->LoadAction( pStruct->szActionName );

				//pNewActor->Show( false );
				//m_vlpActionObject.push_back( pNewActor );
				//pStruct->nActionObjectIndex = (int)m_vlpActionObject.size()-1;

				//DnEtcHandle hHandle = (new CDnEtcObject(false))->GetMySmartPtr();
				//if( hHandle->Initialize( pStruct->szSkinName, pStruct->szAniName, pStruct->szActionName ) == false ) {
				//	SAFE_RELEASE_SPTR( hHandle );
				//	break;
				//}
				//hHandle->Show( false );

				//pStruct->nActionObjectIndex = hHandle->GetMyIndex();
			}
			break;
	}
}

void CDnCutSceneActProp::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_Handle )
		return;

	if( -1 == m_iActionIndex )
		return;

	if( m_iActionIndex >= (int)m_vlAniIndexList.size() )
		return;

	float fPrevFrame = m_fFrame;

	if( m_vlAniIndexList.at(m_iActionIndex) != -1 )
	{
		float fAniLength = (float)m_Handle->GetLastFrame( m_vlAniIndexList.at(m_iActionIndex) );
		m_fFrame += fDelta*60.0f;
		if( m_fFrame >= fAniLength )
		{
			CEtActionBase::ActionElementStruct* pElement = GetElement( m_iActionIndex );
			if( pElement->szNextActionName.empty() )
			{
				m_fFrame = 0.0f;
				m_iActionIndex = -1;
				return;
			}
			else
				SetAction( pElement->szNextActionName.c_str(), (float)pElement->dwNextActionFrame, (float)pElement->dwBlendFrame );
		}

		m_Handle->SetAniFrame( m_vlAniIndexList.at( m_iActionIndex ), m_fFrame );
	}

	ProcessSignal( m_pVecActionElementList->at( m_iActionIndex ), LocalTime, m_fFrame, fPrevFrame );

	if( m_pActionObject )
	{
		//OutputDebug( "[Time] LocalTime: %d, fDelta: %2.2f\n", LocalTime, fDelta );
		m_pActionObject->Process( LocalTime, fDelta );
	}
}


int CDnCutSceneActProp::GetAniIndex( const char* pAniName )
{
	if( !m_Handle ) return -1;
	for( int i = 0; i < m_Handle->GetAniCount(); ++i )
	{
		if( strcmp( m_Handle->GetAniName(i), pAniName ) == 0 )
			return i;
	}

	return -1;
}


void CDnCutSceneActProp::OnSignal( int iSignalType, int iSignalArrayIndex, void* pSignalData, LOCAL_TIME LocalTime, LOCAL_TIME StartTime, LOCAL_TIME EndTime )
{
	switch( iSignalType ) 
	{
		case STE_AlphaBlending:
			{
				if( !m_Handle ) break;
				AlphaBlendingStruct *pStruct = (AlphaBlendingStruct *)pSignalData;
				float fValue = 1.f / ( EndTime - StartTime - 16.6666f ) * ( LocalTime - StartTime );
				fValue = EtClamp( fValue, 0.0f, 1.0f );
				float fCurAlpha = pStruct->fStartAlpha + ( ( pStruct->fEndAlpha - pStruct->fStartAlpha ) * fValue );
				fCurAlpha = EtClamp( fCurAlpha, 0.0f, 1.0f );
				m_Handle->SetObjectAlpha( fCurAlpha );
				if( fCurAlpha < 0.5f ) {
					if( m_Handle->IsShadowCast() )
						m_Handle->EnableShadowCast( false );
				}
			}
			break;
		case STE_Particle:
			{
				//ParticleStruct* pParticle = static_cast<ParticleStruct*>(pSignalData);

				//if( -1 == pParticle->nParticleDataIndex )
				//	break;

				//MatrixEx Cross = m_matExWorld;//*GetEtMatrixEx();
				//Cross.MoveLocalZAxis( pParticle->vPosition->z );
				//Cross.MoveLocalXAxis( pParticle->vPosition->x );
				//Cross.MoveLocalYAxis( pParticle->vPosition->y );

				//EtBillboardEffectHandle hParticle = EternityEngine::CreateParticle( pParticle->nParticleDataIndex, Cross );
				//hParticle->EnableIterate( (pParticle->bIterator == TRUE) ? true : false );
				//hParticle->EnableLoop( (pParticle->bLoop == TRUE) ? true : false );
				//hParticle->SetCullDist( 10000000.0f );
				//hParticle->Show( true );

				//CDnCutSceneActor::S_PARTICLE_INFO ParticleInfo;
				//ParticleInfo.hParticle = hParticle;
				//ParticleInfo.pParticleStruct = pParticle;
				//m_vlParticles.push_back( ParticleInfo );

				//if( !IsShow() ) break;
				//ParticleStruct *pStruct = (ParticleStruct *)pPtr;
				//if( pStruct->nParticleDataIndex == -1 ) break;
				//EtBillboardEffectHandle hParticle;
				//if( pStruct->bLinkObject ) hParticle = TSmartPtrSignalImp<EtBillboardEffectHandle, LinkParticleSignalStruct>::GetSignalHandle( m_nActionIndex, nSignalIndex );
				//else hParticle = TSmartPtrSignalImp<EtBillboardEffectHandle, TSmartPtrSignalStruct::Default>::GetSignalHandle( m_nActionIndex, nSignalIndex );
				//if( hParticle ) {
				//	if( m_fVisibleRange > 0.f ) {
				//		hParticle->SetCullDist( m_fVisibleRange );
				//	}
				//}
			}
			break;
		//case STE_EnableCollision:
		//	{
		//		if( !IsShow() ) break;

		//		EnableCollisionStruct *pStruct = (EnableCollisionStruct *)pPtr;
		//		if( !m_Handle ) break;

		//		m_Handle->EnableCollision( pStruct->bEnable ? true : false );
		//	};
		//	break;
		case STE_Destroy:
			{
				//m_bDestroy = true;
				if( m_Handle->IsShow() )
					Show( false );
			}
			break;
		//case STE_ObjectVisible:
		//	{
		//		ObjectVisibleStruct *pStruct = (ObjectVisibleStruct *)pPtr;
		//		Show( ( pStruct->bShow == TRUE ) );
		//		//				ShowRenderBase( ( pStruct->bShow == TRUE ) );
		//	}
		//	break;

		case STE_ActionObject:
			{
				if( m_Handle->IsShow() )
				{
					ActionObjectStruct* pActionObject = (ActionObjectStruct*)pSignalData;
					
					if( m_pActionObject )
					{
						m_pActionObject->SetAction( pActionObject->szAction, 0.0f, 0.0f );
						m_pActionObject->Show( true );
					}
				}
			}
			break;
	}
}


CDnCutSceneActProp* CDnCutSceneActProp::OnCreateActionObject( const char* pSkinName, const char* pAniName, const char* pActionName )
{
	CDnCutSceneActProp* pProp = new CDnCutSceneActProp;
	
	pProp->SetResPathFinder( m_pPathFinder );

	pProp->m_strAniName = pAniName;
	pProp->m_strActName = pActionName;

	if( false == pProp->Initialize( GetSector(), pSkinName, m_vPosition, m_vRotation, m_vScale ) )
	{
		SAFE_DELETE( pProp );
		return NULL;
	}

	pProp->SetClassID( GetClassID() );

	//GetSector()->InsertProp( pProp );
	static_cast<CDnCutSceneWorldSector*>(GetSector())->DeleteProcessProp( pProp );
	pProp->Show( false );
	pProp->EnableCastShadow( false );

	return pProp;
}

void CDnCutSceneActProp::OnResetCutScene( void )
{
	Show( true );

	if( m_pActionObject )
	{
		m_pActionObject->Show( false );
		m_pActionObject->GetObjectHandle()->SetObjectAlpha( 1.0f );
	}
}
