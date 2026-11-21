#include "StdAfx.h"
#include "DnWorldNpcProp.h"
#include "DNProtocol.h"
#include "DnWorld.h"
#include "DnWorldSector.h"
#include "DnWorldGrid.h"
#include "DnNpcActor.h"
#include "DnMinimap.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnWorldNpcProp::CDnWorldNpcProp()
{
}

CDnWorldNpcProp::~CDnWorldNpcProp()
{
}

bool CDnWorldNpcProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	bool bResult = false;

	if( GetData() ) {
		NpcStruct *pStruct = (NpcStruct *)GetData();
		int nNpcTableID = pStruct->nNpcID;
		bResult = InitializeNpcActorProp( nNpcTableID );
	}

	return bResult;
}

bool CDnWorldNpcProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	bool bResult = CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
	if( !bResult ) return false;

	if( m_hMonster ) {
		*m_hMonster->GetMatEx() = *GetMatEx();
		if( m_hMonster->GetObjectHandle() ) m_hMonster->GetObjectHandle()->Update( *m_hMonster->GetMatEx() );
		m_hMonster->SetActionSignalScale( *GetActionSignalScale() );

		// In village, CanTalk must be kept off until EnterPropNpc packet is received.. 
		if( CDnWorld::IsActive() && ((CDnWorld*)m_pParentSector->GetParentGrid()->GetWorld())->GetMapType() == CDnWorld::MapTypeVillage ) {
			CDnNPCActor *pNpc = (CDnNPCActor*)m_hMonster.GetPointer();
			pNpc->EnableOperator( false );
		}

		SAABox Box;
		GetBoundingBox( Box );
		float fHeight = Box.Max.y - Box.Min.y;
		m_hMonster->GetBoundingBox( Box );
		fHeight -= Box.Max.y - Box.Min.y;
		m_hMonster->SetIncreaseHeight( fHeight );
	}
	return true;
}

void CDnWorldNpcProp::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnWorldActProp::Process( LocalTime, fDelta );

	// Once an Npc is spawned, it forces its position in MAMovementbase Process during its Process loop.
	// Originally it should be created as PropNpc with StaticMovement, but since it was based on NpcActor
	// Since doing it that way is awkward, force-set it here for now and later move it to NPCActor base
	// Will perform dismount work.
	if( m_hMonster ) {
		*m_hMonster->GetMatEx() = *GetMatEx();
	}
}

void CDnWorldNpcProp::Show( bool bShow )
{
	CDnWorldActProp::Show( bShow );
	if( m_hMonster ) {
		m_hMonster->Show( bShow );
		CDnNPCActor *pNpc = (CDnNPCActor*)m_hMonster.GetPointer();
		GetMiniMap().ShowNpcQuestInfo( pNpc->GetNpcData().nNpcID, bShow );
	}
}