#include "StdAfx.h"
#include "DnCutSceneActorProcessor.h"
#include "DnActor.h"
#include "DnActionbase.h"
#include "DnInterface.h"
#include "DnWorldActProp.h"
#include "DnMonsterActor.h"

#include "DnPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 


CDnCutSceneActorProcessor::CDnCutSceneActorProcessor(void)
{
}

CDnCutSceneActorProcessor::~CDnCutSceneActorProcessor(void)
{
	// #60295 - Remove player actor created via Clone.
    S_CS_ACTOR_INFO * pInfo = NULL;
	int size = (int)m_vlpActors.size();	
	for( int i=0; i<size; ++i )
	{
		pInfo = m_vlpActors[ i ];
		if( pInfo->bPlayer && pInfo->bClone )
			static_cast< CDnActor * >( pInfo->pActor )->Release();
	}

}



void CDnCutSceneActorProcessor::AddObjectToPlay( S_CS_ACTOR_INFO* pNewObject )
{
	ICustomActorProcessor::AddObjectToPlay( pNewObject );

	m_vlPrevFrame.push_back( 0.0f );
	m_vlpPrevActionSeq.push_back( NULL );
}


void CDnCutSceneActorProcessor::OnChangeAction( S_CS_ACTOR_INFO* pActionChangeActor )
{
	//CDnActor* pDnActor = static_cast<CDnActor*>(pActionChangeActor->pActor);
	//pDnActor->SetActionQueue( pActionChangeActor->strNowActionName.c_str(), 0, 0.f );

	// Previous frame must be set to 0 to properly catch a 1-frame signal starting at frame 0.
	//m_vlPrevFrame.at( pActionChangeActor->iActorIndexInProcessorVector ) = 0.0f;
}


void CDnCutSceneActorProcessor::ShowSubtitle( const wchar_t* pSubtitle )
{
	GetInterface().SetCutSceneCaption( pSubtitle );
}


void CDnCutSceneActorProcessor::HideSubtitle( void )
{
	GetInterface().ClearCutSceneCaption();
}


void CDnCutSceneActorProcessor::SetPropAction( CEtWorldProp* pProp, const char* pActionName, LOCAL_TIME AbsoluteTime, float fFrame  )
{
	CDnWorldActProp* pActProp = dynamic_cast<CDnWorldActProp*>(pProp);

	if( pActProp ) {
		pActProp->SetAction( pActionName, 0.0f, 0.0f );
		pActProp->ProcessAction( AbsoluteTime, fFrame);		// Run ProcessAction once to prevent the first frame signal from being ignored.
	}
}


void CDnCutSceneActorProcessor::ShowProp( CEtWorldProp* pProp, bool bShow )
{
	CDnWorldProp* pDnWorldProp = dynamic_cast<CDnWorldProp*>( pProp );

	if( pDnWorldProp )
	{
		pDnWorldProp->Show( bShow );
		if( pDnWorldProp->GetObjectHandle() )
			pDnWorldProp->GetObjectHandle()->EnableCull( false );
	}
}



void CDnCutSceneActorProcessor::FadeIn( float fSpeed, DWORD dwColor )
{
	dwColor |= 0xff000000;
	//GetInterface().FadeDialog( 0xff000000, 0x00000000, /*CGlobalInfo::GetInstance().m_fFadeDelta*/ fSpeed );
	// On the client, this is total FadeIn duration, not Speed.

	GetInterface().FadeDialog( dwColor, 0x00000000, 1.0f/fSpeed, NULL, true );
}



void CDnCutSceneActorProcessor::FadeOut( float fSpeed, DWORD dwColor )
{
	dwColor |= 0xff000000;
	//GetInterface().FadeDialog( 0x00000000, 0xff000000, /*CGlobalInfo::GetInstance().m_fFadeDelta*/ fSpeed );
	// On the client, this is total FadeOut duration, not Speed.

	GetInterface().FadeDialog( 0x00000000, dwColor, 1.0f/fSpeed, NULL, true );
}


void CDnCutSceneActorProcessor::CalcAniDistance( S_CS_ACTOR_INFO* pActorInfo, float fCurFrame, float fPrevFrame, EtVector3& DistVec )
{
	if( NULL == pActorInfo )
		return;

	if( pActorInfo->strNowActionName.empty() )
		return;

	CDnActor* pDnActor = static_cast<CDnActor*>(pActorInfo->pActor);
	if (pDnActor == NULL)
		return;

	int iAniIndex = pDnActor->GetElementIndex( pActorInfo->strNowActionName.c_str() );
	if (iAniIndex == -1 || (int)pActorInfo->vlAniIndex.size() <= iAniIndex || !(pActorInfo->hObject))
		return;

	pActorInfo->hObject->CalcAniDistance( pActorInfo->vlAniIndex[iAniIndex], fCurFrame, fPrevFrame, DistVec );
}



int CDnCutSceneActorProcessor::GetActionIndex( int iActorIndex, const char* pActionName )
{
	//if( strcmp( "Event_LookTown", pActionName ) == 0 )
	//{
	//	int i = 0; 
	//	++i;
	//	return ;
	//}

	if( (int)m_vlpActors.size() < iActorIndex )
		return -1;

	if( NULL == pActionName )
		return -1;

	if( 0 == strlen( pActionName ) )
		return -1;

	S_CS_ACTOR_INFO* pActorInfo = m_vlpActors.at( iActorIndex );
	CDnActor* pDnActor = static_cast<CDnActor*>(pActorInfo->pActor);
	int iAniIndex = pDnActor->GetElementIndex( pActionName );
	
	return iAniIndex;
}


float CDnCutSceneActorProcessor::GetActorScale( int iActorIndex )
{
	float fResult = 1.0f;

	if( iActorIndex < (int)m_vlpActors.size() )
	{
		S_CS_ACTOR_INFO* pActorInfo = m_vlpActors.at( iActorIndex );

		// Currently only monster actors use the scale value.
		CDnMonsterActor* pDnMonsterActor = dynamic_cast<CDnMonsterActor*>((CDnActor*)pActorInfo->pActor);
		if( pDnMonsterActor )
			fResult = pDnMonsterActor->GetScale();
	}

	return fResult;
}


void CDnCutSceneActorProcessor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	int iNumActors = (int)m_vlpActors.size();
	for( int iActor = 0; iActor < iNumActors; ++iActor )
	{
		S_CS_ACTOR_INFO* pActorInfo = m_vlpActors.at( iActor );
		if( pActorInfo->bHided )
		{
			//OutputDebug( "[CutScene] %s Hided\n", pActorInfo->strActorName.c_str() );
			continue;
		}

		CDnActor* pDnActor = static_cast<CDnActor*>(pActorInfo->pActor);

		int iAniIndex = pDnActor->GetElementIndex( pActorInfo->strNowActionName.c_str() );
		if( iAniIndex == -1 )
		{
			//OutputDebug( "[CutScene] %s iAniIndex == -1\n", pActorInfo->strActorName.c_str() );

			// If there is no action, set only the position.
			pActorInfo->hObject->SetCalcPositionFlag( 0 );
			pActorInfo->hObject->Update( pActorInfo->matExFinal );
			continue;
		}

		// When the action changes, both current and previous frames must be set to 0, 0 to catch the first signal.. 
		float fSignalCurFrame = pActorInfo->fFrame;
		if( (NULL != pActorInfo->pNowActionSequence) && (m_vlpPrevActionSeq.at( iActor ) != pActorInfo->pNowActionSequence) )
		{
			//if( pActorInfo->strNowActionName == "PullOut_Weapon" )
			//{
			//	int i = 0;
			//	++i;
			//}

			m_vlPrevFrame.at( iActor ) = 0.0;
			fSignalCurFrame = 0.0f;
			pActorInfo->fFrame = 0.0f;
		}

		m_vlpPrevActionSeq.at( iActor ) = pActorInfo->pNowActionSequence;

		// Need to adjust Y value from Animation Distance in matExFinalPos?
		MatrixEx matExFinal = pActorInfo->matExFinal;
		EtVector3 vAniDistance;
		pActorInfo->hObject->CalcAniDistance( pActorInfo->vlAniIndex[iAniIndex], pActorInfo->fFrame, 0.0f, vAniDistance );
		matExFinal.MoveLocalYAxis( -vAniDistance.y );

		// The functions below internally use EtMatrixEx passed via GetMatEx, so update them as well
		MatrixEx* pMatExCross = pDnActor->GetMatEx();
		*pMatExCross = matExFinal;
		//*pCross = pActorInfo->matExFinal; 

		// For debugging
		//OutputDebug( "[CutScene] %s Position : %2.2f %2.2f %2.2f \n", pActorInfo->strActorName.c_str(), pDnActor->GetPosition()->x, 
		//																							    pDnActor->GetPosition()->y, 
		//																							    pDnActor->GetPosition()->z );

		//CEtActionBase::ActionElementStruct* pStruct = pDnActor->GetElement( pActorInfo->iNowAniIndex );

		// Do not use action queue in cutscenes.
		pDnActor->ClearActionQueue();
		CEtActionBase::ActionElementStruct* pStruct = pDnActor->GetElement( iAniIndex );
		pDnActor->SetActionIndex( iAniIndex );
		pDnActor->CDnActionBase::ProcessSignal( pStruct, fSignalCurFrame, m_vlPrevFrame.at(iActor) );
		for( int i=0; i < 2; i++ ) 
		{
			if( pDnActor->GetActiveWeapon( i ) ) {
				pDnActor->GetActiveWeapon( i )->Process( LocalTime, fDelta );
			}
		}
				//pActorInfo->hObject->SetAniFrame( pActorInfo->vlAniIndex[pActorInfo->iNowAniIndex], pActorInfo->fFrame );
		pActorInfo->hObject->SetAniFrame( pActorInfo->vlAniIndex[iAniIndex], pActorInfo->fFrame );
		
		// For Rose, being created as NpcActor caused a bug where AniDistance is double-calculated..
		// External settings can cause incorrect results, so forcibly disable X, Z Position animation calculation. by realgaia
		//pActorInfo->hObject->SetCalcPositionFlag( 0 );
		pActorInfo->hObject->Update( pActorInfo->matExFinal );

		// For signal processing.
		//pDnActor->CDnActionBase::ProcessAction( LocalTime, fDelta );
//		pDnActor->CDnActionBase::ProcessAction( LocalTime, fDelta );
		pDnActor->MAFaceAniBase::Process( LocalTime, fDelta );
		pDnActor->CDnActionSignalImp::Process( LocalTime, fDelta );
		CDnWeapon::ProcessClass( LocalTime, fDelta );

		m_vlPrevFrame[iActor] = pActorInfo->fFrame;
	}
}


// #54681 - Hide handling for player character.
#include "DnPlayerActor.h"
void CDnCutSceneActorProcessor::ShowEffect( void * pActor, bool bShow )
{
	CDnActor * pAc = static_cast< CDnActor * >( pActor );
	if( pAc->IsShow() == bShow ) return;
	CDnPlayerActor * pPlayerActor = dynamic_cast< CDnPlayerActor * >( pAc );
	if( pPlayerActor )
	{
		if( bShow == false )
		{
			// When hiding, effects created by the weapon must also be removed, so calling PlayerActor's Show to invoke DnWeapon::ShowWeapon is right,
			pPlayerActor->Show( bShow );
		}
		else
		{
			// When showing, weapons may be forcibly hidden during cutscenes, so preserving the weapon's Show state is right.
			pPlayerActor->Show( bShow );

			if( pPlayerActor->IsHideWeaponBySignalInCutScene( 0 ) )
			{
				pPlayerActor->CDnPlayerActor::ShowCashWeapon( 0, false );
				pPlayerActor->CDnActor::ShowWeapon( 0, false );
			}

			if( pPlayerActor->IsHideWeaponBySignalInCutScene( 1 ) )
			{
				pPlayerActor->CDnPlayerActor::ShowCashWeapon( 1, false );
				pPlayerActor->CDnActor::ShowWeapon( 1, false );
			}
			// While it is true that the code above is not ideal,
			// Since there are already many cutscenes made this way,
			// And since Show must be called, handle it with a flag like this.


			// #71465 - [Other] Halloween Pumpkin Spirit graphics are not displayed in event movie.			
			// < Problem >
			// When actor position is changed in cutscene using Action Property and Key Property
			// Because a situation arises where the changed position cannot be retrieved from the effect and the effect position is not properly updated,
			// This causes the effect to appear detached or invisible.
			// Difficult to work on because this part was not considered from the start.
			//
			// < Current workaround >
			// - Since parts effects are not shown in cutscenes currently, forcibly hide only spirit effects.
			//
			// < Future solution... >
			// - When changing actor position via Action Property and Key Property in a cutscene, changed position in particles
			// Must be able to fetch it.
			DnPartsHandle hParts = pPlayerActor->GetCashParts( CDnParts::PartsTypeEnum::CashFairy );
			if( hParts )
				hParts->ShowRenderBase( false );
		}
	}
	else
		pAc->Show( bShow );
	
//	CDnPlayerActor * pAC = static_cast< CDnPlayerActor * >( pActor );
//	pAC->Show( bShow );	
}
