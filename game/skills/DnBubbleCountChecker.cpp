#include "stdafx.h"
#include "DnBubbleCountChecker.h"
#include "DnPlayerActor.h"
#include "DnBubbleSystem.h"

#ifndef _GAMESERVER
#include "DnLocalPlayerActor.h"
#endif

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnBubbleCountChecker::CDnBubbleCountChecker( DnActorHandle hActor, int iBubbleTypeID, int iNeedBubbleCount ) : IDnSkillUsableChecker( hActor ),
																											    m_iBubbleTypeID( iBubbleTypeID ),
																											    m_iNeedBubbleCount( iNeedBubbleCount )
{
	m_iType = BUBBLE_COUNT_CHECKER;
}

CDnBubbleCountChecker::~CDnBubbleCountChecker( void )
{

}


bool CDnBubbleCountChecker::CanUse( void )
{
	// On client, will only be used in LocalPlayerActor..
	BubbleSystem::CDnBubbleSystem* pBubbleSystem = NULL;
#ifdef _GAMESERVER
	if( false == m_hHasActor->IsPlayerActor() )
		return false;

	pBubbleSystem = static_cast<CDnPlayerActor*>(m_hHasActor.GetPointer())->GetBubbleSystem();
#else
	if( NULL == dynamic_cast<CDnLocalPlayerActor*>(m_hHasActor.GetPointer()) )
		return false;

	pBubbleSystem = static_cast<CDnLocalPlayerActor*>(m_hHasActor.GetPointer())->GetBubbleSystem();
#endif

	bool bResult = false;

	vector<BubbleSystem::CDnBubbleSystem::S_BUBBLE_INFO> vlAppliedBubbles;
	pBubbleSystem->GetAllAppliedBubbles( vlAppliedBubbles );

	int iNumAppliedBubble = (int)vlAppliedBubbles.size();
	for( int i = 0; i < iNumAppliedBubble; ++i )
	{
		const BubbleSystem::CDnBubbleSystem::S_BUBBLE_INFO& BubbleInfo = vlAppliedBubbles.at( i );
		if( BubbleInfo.iBubbleTypeID == m_iBubbleTypeID &&
			m_iNeedBubbleCount <= BubbleInfo.iCount )
		{
			bResult = true;
		}
	}

	return bResult;
}

IDnSkillUsableChecker* CDnBubbleCountChecker::Clone()
{
	CDnBubbleCountChecker* pNewChecker = new CDnBubbleCountChecker(m_hHasActor, m_iBubbleTypeID, m_iNeedBubbleCount);

	return pNewChecker;
}
