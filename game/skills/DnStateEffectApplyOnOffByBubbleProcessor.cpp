#include "stdafx.h"
#include "DnStateEffectApplyOnOffByBubbleProcessor.h"
#include "DnBubbleSystem.h"
#include "DnPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnStateEffectApplyOnOffByBubbleProcessor::CDnStateEffectApplyOnOffByBubbleProcessor( DnActorHandle hActor, const char* pArg ) : IDnSkillProcessor( hActor )
{
	m_iType = STATE_EFFECT_APPLY_ONOFF_BY_BUBBLE;

	if( pArg )
	{
		// BubbleID;BubbleCount;[StateEffectIndex1][StateEffectIndex2];BubbleID;BubbleCount;[StateEffectIndex1][StateEffectIndex2]... in groups of 3.
		vector<string> vlTokens;
		TokenizeA( pArg, vlTokens, ";" );
		_ASSERT( 0 == (vlTokens.size()%3) );
		for( int i = 0; i < (int)vlTokens.size(); i += 3 )
		{
			S_STATE_EFFECT_ON_OFF Info;
			Info.iBubbleID = atoi( vlTokens.at( i ).c_str() );
			Info.iBubbleCount = atoi( vlTokens.at( i+1 ).c_str() );

			vector<string> vlSETokens;
			TokenizeA( vlTokens.at(i+2), vlSETokens, string("["), string("]"), false );
			for( int k = 0; k < (int)vlSETokens.size(); ++k )
			{
				Info.vlApplyList.push_back( atoi(vlSETokens.at(k).c_str()) );
			}

			m_vlApplyOnOff.push_back( Info );
		}
	}
}

CDnStateEffectApplyOnOffByBubbleProcessor::~CDnStateEffectApplyOnOffByBubbleProcessor(void)
{

}


void CDnStateEffectApplyOnOffByBubbleProcessor::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill )
{

}


void CDnStateEffectApplyOnOffByBubbleProcessor::Process( LOCAL_TIME LocalTime, float fDelta )
{


}


void CDnStateEffectApplyOnOffByBubbleProcessor::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{

}

void CDnStateEffectApplyOnOffByBubbleProcessor::SelectAvailableSE( DNVector(CDnSkill::StateEffectStruct)& vlSEList )
{
	if( false == m_hHasActor->IsPlayerActor() )
		return;

	// First search based on the current stacked bubble state.
	int iGreatestCount = 0;
	BubbleSystem::CDnBubbleSystem* pBubbleSystem = static_cast<CDnPlayerActor*>(m_hHasActor.GetPointer())->GetBubbleSystem();
	const S_STATE_EFFECT_ON_OFF* pSelectedInfo = NULL;
	for( int i = 0; i < (int)m_vlApplyOnOff.size(); ++i )
	{
	 	S_STATE_EFFECT_ON_OFF& Info = m_vlApplyOnOff.at( i );
		int iNowBubbleCount = pBubbleSystem->GetBubbleCountByTypeID( Info.iBubbleID );
		if( Info.iBubbleCount <= iNowBubbleCount )
		{
			if( iGreatestCount < Info.iBubbleCount )
			{
				iGreatestCount = Info.iBubbleCount;
				pSelectedInfo = &Info;
			}
		}
	}

	if( pSelectedInfo )
	{
		m_vlSEBackup = vlSEList;
		vlSEList.clear();

		int iApplyAllPairOffset = 0;
		for( int i = 0; i < (int)pSelectedInfo->vlApplyList.size(); ++i )
		{
			int iApplySEIndex = pSelectedInfo->vlApplyList.at(i)-1 + iApplyAllPairOffset;
			if( iApplySEIndex < (int)m_vlSEBackup.size() )
			{
				const CDnSkill::StateEffectStruct& SE = m_vlSEBackup.at( iApplySEIndex );
				vlSEList.push_back( SE );
				if( SE.bApplyAllPair )
				{
					int iPairIndex = iApplySEIndex+1;
					vlSEList.push_back( m_vlSEBackup.at(iPairIndex) );
					++iApplyAllPairOffset;
				}
			}
		}
	}
}

void CDnStateEffectApplyOnOffByBubbleProcessor::RestoreSEList( DNVector(CDnSkill::StateEffectStruct)& vlSEList )
{
	// Restore only when a state effect was selected and the state effect list was modified.
	if( false == m_vlSEBackup.empty() )
	{
		vlSEList = m_vlSEBackup;
	}
}