#include "StdAfx.h"
#include "DnHeadEffectRender.h"
#include "DnTableDB.h"
#include "DnActor.h"
#include "DnInCodeResource.h"

#define HEADEFFECT_ETCOFFSET	-200
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef  PRE_MOD_CHRAMBOX_ROTATE
#include "DnCamera.h"
#endif 


CDnHeadEffectRender::CDnHeadEffectRender()
{
	int aiPriority[HeadEffect_Amount] = {
		0,	// Speech bubble
		0,	// Radio message
		1,	// Enhancement effect
		1,	// Random item
		1,	// Crest Jewel
		1,	// Item synthesis
#ifdef PRE_ADD_SECONDARY_SKILL
		1,
		1,
		1,
		1,
		1,
		1,
#endif // PRE_ADD_SECONDARY_SKILL
	};
	memcpy( m_aiEffectPriority, aiPriority, sizeof(m_aiEffectPriority) );
#ifdef PRE_MOD_HEAD_EFFECT_HEIGHT
	m_fAdjustHeight = 0.0f;
#endif
}

CDnHeadEffectRender::~CDnHeadEffectRender()
{
	// If an actor is suddenly destroyed during playback, looping effects may remain without disappearing.
	if( m_hHeadEffectActor )
	{
		for( int i = 0; i < CDnInCodeResource::EtcIndex_Amount; ++i )
		{
			eHeadEffectType eType = ToHeadEffectType((CDnInCodeResource::eHeadEffectEtcIndex)i);
			m_hHeadEffectActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( HEADEFFECT_ETCOFFSET, eType );
		}
	}
}

void CDnHeadEffectRender::Initialize( DnActorHandle hActor )
{
	m_hHeadEffectActor = hActor;
}

void CDnHeadEffectRender::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hHeadEffectActor ) return;

	// When calling SetParentActor during Insert below, Process is called automatically so
	// EtcObject particles inherit alpha changes from the parent actor, but mesh alpha does not change automatically, so update it directly here.
	for( int i = 0; i < CDnInCodeResource::EtcIndex_Amount; ++i )
	{
		eHeadEffectType eType = ToHeadEffectType((CDnInCodeResource::eHeadEffectEtcIndex)i);
		if( m_hHeadEffectActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::IsExistSignalHandle( HEADEFFECT_ETCOFFSET, eType ) )
		{
			DnEtcHandle hHandle = m_hHeadEffectActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::GetSignalHandle( HEADEFFECT_ETCOFFSET, eType );
			if( hHandle ) {
				float fAlpha = m_hHeadEffectActor->GetAniObjectHandle()->GetObjectAlpha();
				if( hHandle->GetObjectHandle() )
					hHandle->GetObjectHandle()->SetObjectAlpha( fAlpha );	// Change alpha of mesh object.
			}
		}
	}
}

CDnInCodeResource::eHeadEffectEtcIndex CDnHeadEffectRender::ToEtcIndex( eHeadEffectType eType )
{
	switch( eType )
	{
	case HeadEffect_Enchant:	return CDnInCodeResource::EtcIndex_Enchant;
	case HeadEffect_RandomItem:	return CDnInCodeResource::EtcIndex_RandomItem;
	case HeadEffect_Plate:		return CDnInCodeResource::EtcIndex_Plate;
	case HeadEffect_Compound:	return CDnInCodeResource::EtcIndex_Compound;
#ifdef PRE_ADD_SECONDARY_SKILL
	case HeadEffect_Plant:			return CDnInCodeResource::EtcIndex_Plant;
	case HeadEffect_Water:			return CDnInCodeResource::EtcIndex_Water;
	case HeadEffect_Harvest:		return CDnInCodeResource::EtcIndex_Harvest;
	case HeadEffect_Cooking_Making:	return CDnInCodeResource::EtcIndex_Cooking_Making;
	case HeadEffect_Cooking_Finish:	return CDnInCodeResource::EtcIndex_Cooking_Finish;
	case HeadEffect_Fishing:		return CDnInCodeResource::EtcIndex_Fishing;
#endif // PRE_ADD_SECONDARY_SKILL
	default:
		_ASSERT(0&&"CDnHeadEffectRender::ToEtcIndex Invalid argument received.");
	}
	return CDnInCodeResource::EtcIndex_Enchant;
}

CDnHeadEffectRender::eHeadEffectType CDnHeadEffectRender::ToHeadEffectType( CDnInCodeResource::eHeadEffectEtcIndex eType )
{
	switch( eType )
	{
	case CDnInCodeResource::EtcIndex_Enchant:		return HeadEffect_Enchant;
	case CDnInCodeResource::EtcIndex_RandomItem:	return HeadEffect_RandomItem;
	case CDnInCodeResource::EtcIndex_Plate:			return HeadEffect_Plate;
	case CDnInCodeResource::EtcIndex_Compound:		return HeadEffect_Compound;
#ifdef PRE_ADD_SECONDARY_SKILL
	case CDnInCodeResource::EtcIndex_Plant:				return HeadEffect_Plant;
	case CDnInCodeResource::EtcIndex_Water:				return HeadEffect_Water;
	case CDnInCodeResource::EtcIndex_Harvest:			return HeadEffect_Harvest;
	case CDnInCodeResource::EtcIndex_Cooking_Making:	return HeadEffect_Cooking_Making;
	case CDnInCodeResource::EtcIndex_Cooking_Finish:	return HeadEffect_Cooking_Finish;
	case CDnInCodeResource::EtcIndex_Fishing:			return HeadEffect_Fishing;
#endif // PRE_ADD_SECONDARY_SKILL
	}
	return HeadEffect_Enchant;
}

CDnHeadEffectRender::eHeadEffectType CDnHeadEffectRender::IsEffectPlaying()
{
	if( m_ChatBalloon.IsRenderChatBalloon() )
		return HeadEffect_ChatBalloon;

	if( m_HeadIcon.IsRenderHeadIcon() )
		return HeadEffect_RadioMsg;

	for( int i = 0; i < CDnInCodeResource::EtcIndex_Amount; ++i ) {
		eHeadEffectType eType = ToHeadEffectType((CDnInCodeResource::eHeadEffectEtcIndex)i);
		if( m_hHeadEffectActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::IsExistSignalHandle( HEADEFFECT_ETCOFFSET, eType ) )
		{
			DnEtcHandle hHandle = m_hHeadEffectActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::GetSignalHandle( HEADEFFECT_ETCOFFSET, eType );
			if( hHandle ) {
				std::string szCurrentActionName = hHandle->GetCurrentAction();
				if( !szCurrentActionName.empty() )
					return eType;
			}
			else {
				// Entering here happens after non-looping effects like enhancement success/failure finish playing
				// DnEtcHandle was destroyed by the destroy signal, but only the inserted signal remains.
				// There is no reason for only the signal to exist without an Etc handle, so delete it here.
				// For looping Making, IsPlaying is checked above and follows the Stop routine, so it does not enter here.
				m_hHeadEffectActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( HEADEFFECT_ETCOFFSET, eType );
			}
		}
	}

	return HeadEffect_None;
}

void CDnHeadEffectRender::StopHeadEffect( eHeadEffectType eType )
{
	// Stop the currently playing effect.
	switch( eType )
	{
	case HeadEffect_ChatBalloon:	m_ChatBalloon.StopRenderChatBalloon();	break;
	case HeadEffect_RadioMsg:		m_HeadIcon.StopRenderHeadIcon();		break;
	case HeadEffect_Enchant:
	case HeadEffect_RandomItem:
	case HeadEffect_Plate:
	case HeadEffect_Compound:
#ifdef PRE_ADD_SECONDARY_SKILL
	case HeadEffect_Plant:
	case HeadEffect_Water:
	case HeadEffect_Harvest:
	case HeadEffect_Cooking_Making:
	case HeadEffect_Cooking_Finish:
	case HeadEffect_Fishing:
#endif // PRE_ADD_SECONDARY_SKILL
		m_hHeadEffectActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( HEADEFFECT_ETCOFFSET, eType );
		break;
	}
}

bool CDnHeadEffectRender::CheckPlayable( eHeadEffectType eType )
{
	eHeadEffectType eCurType = IsEffectPlaying();
	if( eCurType != HeadEffect_None ) {
		if( m_aiEffectPriority[eCurType] > m_aiEffectPriority[eType] )
			return false;
		StopHeadEffect( eCurType );
	}
	return true;
}

void CDnHeadEffectRender::SetChatBalloonText( LPCWSTR wszMessage, DWORD dwTalkTime, int nChatType, bool nCount )
{
	if( !CheckPlayable(HeadEffect_ChatBalloon) )
		return;

	m_ChatBalloon.SetChatBalloonText( wszMessage, dwTalkTime, nChatType );
	
	if( nCount )
		m_ChatBalloon.IncreaseBalloon();
}

void CDnHeadEffectRender::SetHeadIcon( EtTextureHandle hTexture, DWORD dwStartTime, DWORD dwRenderTime, bool bReleaseTexture )
{
	if( !CheckPlayable(HeadEffect_RadioMsg) )
		return;

	m_HeadIcon.SetHeadIcon( hTexture, dwStartTime, dwRenderTime, bReleaseTexture );
}

void CDnHeadEffectRender::SetHeadEffect( eBroadcastingEffect eType, eBroadcastingEffect eState )
{
	// Match type received from server to client type
	eHeadEffectType eEffectType = HeadEffect_None;
	switch(eType)
	{
	case EffectType_Enchant:	eEffectType = HeadEffect_Enchant;		break;
	case EffectType_Compound:	eEffectType = HeadEffect_Compound;		break;
	case EffectType_Emblem:		eEffectType = HeadEffect_Plate;			break;
	case EffectType_Random:		eEffectType = HeadEffect_RandomItem;	break;
#ifdef PRE_ADD_SECONDARY_SKILL
	case EffectType_Plant:				eEffectType = HeadEffect_Plant;			break;
	case EffectType_Water:				eEffectType = HeadEffect_Water;			break;
	case EffectType_Harvest:			eEffectType = HeadEffect_Harvest;		break;
	case EffectType_Cooking_Making:		eEffectType = HeadEffect_Cooking_Making;	break;
	case EffectType_Cooking_Finish:		eEffectType = HeadEffect_Cooking_Finish;	break;
	case EffectType_Fishing:			eEffectType = HeadEffect_Fishing;		break;
#endif // PRE_ADD_SECONDARY_SKILL
	}

	if( eEffectType == HeadEffect_None )
	{
		_ASSERT(0&&"Broadcast effect type value is invalid.");
		return;
	}

	if( !CheckPlayable(eEffectType) )
		return;

	if( eState == EffectState_Cancel )
		return;

	DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreateHeadEffect( ToEtcIndex(eEffectType) );
	if( hHandle ) {
		hHandle->SetPosition( *m_hHeadEffectActor->GetPosition() );
		hHandle->SetParentActor( m_hHeadEffectActor );

		// Manage by putting head effect define offset in action index and effect type in signal index.
		EtcObjectSignalStruct *pResult = m_hHeadEffectActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( HEADEFFECT_ETCOFFSET, eEffectType, hHandle );
		pResult->bLinkObject = true;
#ifdef PRE_MOD_HEAD_EFFECT_HEIGHT
		pResult->vOffset = EtVector3( 0.f, m_fAdjustHeight, 0.f );
#else
		pResult->vOffset = EtVector3( 0.f, 0.f, 0.f );
#endif
#ifdef PRE_MOD_CHRAMBOX_ROTATE // Set Degree fixed to 180 so the chest always opens facing the direction the character is looking. 
		if( eEffectType == HeadEffect_RandomItem ) {
			pResult->vRotate = EtVector3( 0.f, 180, 0.f );
		}
		else
			pResult->vRotate = EtVector3( 0.f, 0.f, 0.f );
#else
		pResult->vRotate = EtVector3( 0.f, 0.f, 0.f );
#endif 
		pResult->bDefendenceParent = false;
		pResult->szBoneName[0] = 0;

		// Execute the appropriate action for the state.
		CEtActionBase::ActionElementStruct *pStruct = NULL;
		switch( eState )
		{
		case EffectState_Start:
			hHandle->SetActionQueue( "Making" );
			break;
		case EffectState_Success:
			// They said the sound comes out after the result due to the Ready animation, which feels bad. Unavoidably skip the Ready animation and show it immediately.
			//hHandle->SetActionQueue( "Ready_Success" );
			hHandle->SetActionQueue( "Success" );
			break;
		case EffectState_Fail:
			//hHandle->SetActionQueue( "Ready_Fail" );
			hHandle->SetActionQueue( "Fail" );
			break;
		case EffectState_SoftFail:
			//hHandle->SetActionQueue( "Ready_DownGrade" );
			hHandle->SetActionQueue( "DownGrade" );
			break;
		case EffectState_Open:
			hHandle->SetActionQueue( "Ready" );
			break;
		case EffectState_VeryGood:
			hHandle->SetActionQueue( "Open_VeryGood" );
			break;
		case EffectState_Good:
			hHandle->SetActionQueue( "Open_Good" );
			break;
		case EffectState_Normal:
			hHandle->SetActionQueue( "Open_Normal" );
			break;
		case EffectState_Bad:
			hHandle->SetActionQueue( "Open_Bad" );
			break;
		}
	}
}