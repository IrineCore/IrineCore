#include "stdafx.h"
#include "DnNULLBlow.h"
#include "DnStateBlow.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnNULLBlow::CDnNULLBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_100;
	SetValue( szValue );
	m_fValue = (float)atof(szValue);

#ifdef _CLIENT
	m_bUseCommonEffect = false;

	std::vector<string> vlTokens;
	std::string strValue = szValue;
	TokenizeA( strValue, vlTokens, ";" );

	if( vlTokens.size() == 2 )
	{
		m_bUseCommonEffect = atoi(vlTokens[0].c_str()) ? true : false;
		m_strCommonEffectName = vlTokens[1];
	}
#endif
	
}

CDnNULLBlow::~CDnNULLBlow(void)
{

}


void CDnNULLBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _CLIENT
	if( m_bUseCommonEffect == true )
	{
		DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
		if( hHandle ) 
		{
			hHandle->SetActionQueue( m_strCommonEffectName.c_str() );
			hHandle->SetParentActor( m_hActor );
			EtcObjectSignalStruct *pResult = m_hActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );
			pResult->bLinkObject = true;
			pResult->bDefendenceParent = true;
			sprintf_s( pResult->szBoneName, "Bip01" );
		}
	}
	else
	{
		m_pEffectOutputInfo = m_hActor->GetStateBlow()->GetEffectOutputInfo( (int)m_fValue );
		if( m_pEffectOutputInfo )
		{
			_AttachGraphicEffect( true );
		}
	}
#endif

}

void CDnNULLBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _CLIENT
	if( m_bUseCommonEffect == false )
		_DetachGraphicEffect();
#endif
}
