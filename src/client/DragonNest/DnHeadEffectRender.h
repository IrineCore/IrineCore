#pragma once

#include "DnActionSignalImp.h"
#include "DnInCodeResource.h"
#include "MAChatBalloon.h"
#include "DnHeadIcon.h"

class CDnHeadEffectRender
{
public:
	// If an overhead effect Enum is added, update the type conversion below as well.
	// Moving enum declaration to public because it needs to be controlled from npc reaction objects. - Hanki.
	enum eHeadEffectType
	{
		HeadEffect_None = -1,
		HeadEffect_ChatBalloon,
		HeadEffect_RadioMsg,
		HeadEffect_Enchant,
		HeadEffect_RandomItem,
		HeadEffect_Plate,
		HeadEffect_Compound,

#ifdef PRE_ADD_SECONDARY_SKILL
		HeadEffect_Plant,
		HeadEffect_Water,
		HeadEffect_Harvest,
		HeadEffect_Cooking_Making,
		HeadEffect_Cooking_Finish,
		HeadEffect_Fishing,
#endif // PRE_ADD_SECONDARY_SKILL

#ifdef PRE_ADD_SECONDARY_SKILL
		HeadEffect_Amount = 12,
#else // PRE_ADD_SECONDARY_SKILL
		HeadEffect_Amount = 6,
#endif // PRE_ADD_SECONDARY_SKILL
	};

	CDnHeadEffectRender();
	virtual ~CDnHeadEffectRender();

	void Initialize( DnActorHandle hActor );
	void Process( LOCAL_TIME LocalTime, float fDelta );

	void SetChatBalloonText( LPCWSTR wszMessage, DWORD dwTalkTime, int nChatType, bool nCount = false );
	void SetHeadIcon( EtTextureHandle hTexture, DWORD dwStartTime, DWORD dwRenderTime = 0, bool bReleaseTexture = false );
	void SetHeadEffect( eBroadcastingEffect eType, eBroadcastingEffect eState );

	void StopHeadEffect( eHeadEffectType eType );

	// Balloon class became large, so changed to fetch and reuse it.
	MAChatBalloon &GetChatBalloon() { return m_ChatBalloon; }

#ifdef PRE_MOD_HEAD_EFFECT_HEIGHT
	void SetHeadEffectAdjustHeight( float fAdjustHeight ) { m_fAdjustHeight = fAdjustHeight; }
#endif

protected:
	CDnInCodeResource::eHeadEffectEtcIndex ToEtcIndex( eHeadEffectType eType );
	eHeadEffectType ToHeadEffectType( CDnInCodeResource::eHeadEffectEtcIndex eType );

	// Must have higher priority than what is currently playing to stop it and play.
	int m_aiEffectPriority[HeadEffect_Amount];

	eHeadEffectType IsEffectPlaying();
	bool CheckPlayable( eHeadEffectType eType );

	// Since there is a limit to what can be grouped in common, use a has-a relationship instead.
	MAChatBalloon m_ChatBalloon;
	CDnHeadIcon m_HeadIcon;
	DnActorHandle m_hHeadEffectActor;

#ifdef PRE_MOD_HEAD_EFFECT_HEIGHT
	float m_fAdjustHeight;
#endif
};