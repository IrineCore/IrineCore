#include "StdAfx.h"
#include "DnWorldHitMoveDamageBrokenProp.h"
#include "DnWorld.h"
#include "DnTableDB.h"
#include "DnWorldSector.h"
#include "NavigationMesh.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnWorldHitMoveDamageBrokenProp::CDnWorldHitMoveDamageBrokenProp(void) : m_bHitMove( false ), 
																		 m_fMoveDistance( 0.0f ),
																		 m_fWholeMoveTime( 0.0f ),
																		 m_fMovedTime( 0.0f ),
																		 m_fVelocity( 0.0f ),
																		 m_fResist( 0.0f ),
																		 m_pCurCell( NULL ),
																		 m_iMovingAxis( MOVE_AXIS_NONE ),
																		 m_fAxisMovingSpeed( 0.0f ),
																		 m_fAxisMaxMoveDistance( 0.0f ),
																		 m_bArrived( false ),
																		 m_fHitDurability( 0.0f ),
																		 m_iActivateElementIndex( -1 ),
																		 m_bActivate( false )

{
}

CDnWorldHitMoveDamageBrokenProp::~CDnWorldHitMoveDamageBrokenProp(void)
{
	ReleasePostCustomParam();
}


// Prop header struct differs from broken prop, so redo it here
bool CDnWorldHitMoveDamageBrokenProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPROP );

	LoadHitEffectData( nTableID );
	m_bIsStaticCollision = false;

	if( GetData() ) 
	{
		HitMoveDamageBrokenStruct* pStruct = (HitMoveDamageBrokenStruct *)GetData();
		bool					   bResult = false;

//#61146 Structure allowing Trap-type prop attack power to be set differently by difficulty.
#ifdef PER_ADD_PROP_TRAP_DIFFICULT_SET
		bResult = InitializeMonsterActorProp( pStruct->nMonsterTableID, pStruct->MonsterWeight );
#else
		bResult = InitializeMonsterActorProp( pStruct->nMonsterTableID );
#endif	

		_ASSERT( bResult );
		if( bResult )
		{
			m_nDurability = pStruct->nDurability;

			if( -1 != m_nDurability )
			{
				if( 0 == m_nDurability ) m_nDurability = 1;
				m_nItemDropGroupTableID = pStruct->nItemDropGroupTableID;
				if( m_nItemDropGroupTableID > 0 )
				{
					CDnDropItem::CalcDropItemList( -1, m_nItemDropGroupTableID, m_VecDropItemList );

					for( DWORD i=0; i<m_VecDropItemList.size(); i++ )
					{
						if( CDnDropItem::PreInitializeItem( m_VecDropItemList[i].nItemID ) == false ) 
						{
							m_VecDropItemList.erase( m_VecDropItemList.begin() + i );
							i--;
						}
					}
				}

				m_iMovingAxis = pStruct->MovingAxis;
				m_fAxisMovingSpeed = pStruct->fDefaultAxisMovingSpeed;
				m_fAxisMaxMoveDistance = pStruct->fMaxAxisMoveDistance;
			}
		}
		else
			return false;
	}

	return true;
}


bool CDnWorldHitMoveDamageBrokenProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	bool bResult = CDnWorldBrokenDamageProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );

	EtVector3 vDir = m_matExWorld.m_vZAxis;
	EtVec3Normalize( &vDir, &vDir );
	m_vInitialPos = m_matExWorld.m_vPosition;
	m_vDestPosition = m_matExWorld.m_vPosition + (vDir * m_fAxisMaxMoveDistance);

	if( m_Handle )
		m_Handle->SetTargetCollisionGroup( COLLISION_GROUP_STATIC( 1 ) | COLLISION_GROUP_DYNAMIC( 2 ) );

	return bResult;
}


bool CDnWorldHitMoveDamageBrokenProp::CreateObject( void )
{
	bool bResult = CDnWorldBrokenDamageProp::CreateObject();

	if( bResult )
	{
		m_iActivateElementIndex = GetElementIndex( "Activate" );
	}

	return bResult;
}


void CDnWorldHitMoveDamageBrokenProp::_GetPosProperly( EtVector3 &vPrevPos )
{
	NavigationMesh *pNavMesh = INSTANCE(CDnWorld).GetNavMesh( m_matExWorld.m_vPosition );
	if( !pNavMesh )
	{
		//EtVector3 vPrevPos = m_matExWorld.m_vPosition;
		if( CDnWorld::GetInstance().GetAttribute( m_matExWorld.m_vPosition ) != 0 )
		{
			m_matExWorld.m_vPosition = vPrevPos;
		}
		m_matExWorld.m_vPosition.y = CDnWorld::GetInstance().GetHeight( m_matExWorld.m_vPosition );
	}
	else 
	{
		if( m_pCurCell == NULL ) 
		{
			m_pCurCell = pNavMesh->FindClosestCell( m_matExWorld.m_vPosition );
			if( ( m_pCurCell == NULL ) || ( !m_pCurCell->IsPointInCellCollumn( m_matExWorld.m_vPosition ) ) ) 
			{
				m_pCurCell = NULL;

				//EtVector3 vPrevPos = m_matExWorld.m_vPosition;
				if( CDnWorld::GetInstance().GetAttribute( m_matExWorld.m_vPosition ) != 0 )
				{
					m_matExWorld.m_vPosition = vPrevPos;
				}
				m_matExWorld.m_vPosition.y = CDnWorld::GetInstance().GetHeight( m_matExWorld.m_vPosition );
			}
		}
		if( m_pCurCell )
		{
			//EtVector3 vPrevPos = m_matExWorld.m_vPosition;
			//m_matExWorld.MoveLocalZAxis( fValue );
			int nSide = -1;
			NavigationCell *pLastCell = NULL;
			m_pCurCell->FindLastCollision( vPrevPos, m_matExWorld.m_vPosition, &pLastCell, nSide );
			if( nSide != -1 )
			{
				if( pLastCell->Link( nSide ) == NULL )
				{
					EtVector2 vMoveDir2D( m_matExWorld.m_vPosition.x - vPrevPos.x, m_matExWorld.m_vPosition.z - vPrevPos.z );
					float fMoveLength = EtVec2Length( &vMoveDir2D );
					vMoveDir2D /= fMoveLength;
					EtVector2 vWallDir2D = pLastCell->Side( nSide )->EndPointB() - pLastCell->Side( nSide )->EndPointA();
					EtVec2Normalize( &vWallDir2D, &vWallDir2D );
					fMoveLength *= EtVec2Dot( &vWallDir2D, &vMoveDir2D );
					m_matExWorld.m_vPosition.x = vPrevPos.x + fMoveLength * vWallDir2D.x;
					m_matExWorld.m_vPosition.z = vPrevPos.z + fMoveLength * vWallDir2D.y;

					int nNewSide = -1;
					m_pCurCell->FindLastCollision( vPrevPos, m_matExWorld.m_vPosition, &pLastCell, nNewSide );
					if( nNewSide != -1 )
					{
						if( pLastCell->Link( nNewSide ) )
						{
							NavigationCell *pNewCell;
							pNewCell = pLastCell->Link( nNewSide );
							if( pNewCell->IsPointInCellCollumn( m_matExWorld.m_vPosition ) )
							{
								m_pCurCell = pNewCell;
							}
							else
							{
								m_matExWorld.m_vPosition.x = vPrevPos.x;
								m_matExWorld.m_vPosition.z = vPrevPos.z;
							}
						}
						else if( !m_pCurCell->IsPointInCellCollumn( m_matExWorld.m_vPosition ) )
						{
							m_matExWorld.m_vPosition.x = vPrevPos.x;
							m_matExWorld.m_vPosition.z = vPrevPos.z;
						}
					}
				}
				else
				{
					m_pCurCell = pLastCell->Link( nSide );
				}
			}

			if( m_pCurCell->GetType() == NavigationCell::CT_PROP )
				m_matExWorld.m_vPosition.y = m_pCurCell->GetPlane()->SolveForY( m_matExWorld.m_vPosition.x, m_matExWorld.m_vPosition.z );
			else
				m_matExWorld.m_vPosition.y = CDnWorld::GetInstance().GetHeight( m_matExWorld.m_vPosition );
		}
	}
}


void CDnWorldHitMoveDamageBrokenProp::_UpdatePropPos()
{
	// Reversely adjust position value as well.
	EtVector3 vOffset = *m_pParentSector->GetOffset();
	vOffset.x -= ( m_pParentSector->GetTileWidthCount() * m_pParentSector->GetTileSize() ) / 2.f;
	vOffset.z -= ( m_pParentSector->GetTileHeightCount() * m_pParentSector->GetTileSize() ) / 2.f;
	vOffset.y = 0.f;
	m_vPosition = m_matExWorld.m_vPosition - vOffset;

	// Consider cases where a scale value also exists.
	// Since applying scale to the m_vCross vector affects movement, use it only when updating the object at the end.
	// When handling hit signal, fetch scale value directly from prop object. EtMatrixEx only has translation and rotation.
	EtMatrix matScale;
	m_matWorld = m_matExWorld;
	EtMatrixScaling( &matScale, m_vScale.x, m_vScale.y, m_vScale.z );
	EtMatrixMultiply( &m_matWorld, &matScale, &m_matWorld );

	m_Handle->Update( &m_matWorld );

	// Since props normally do not update the octree when moved, update this one separately.
	SSphere Sphere;
	GetBoundingSphere( Sphere );
	m_pCurOctreeNode = static_cast<CDnWorldSector*>(m_pParentSector)->GetOctree()->Update( GetMySmartPtr(), Sphere, m_pCurOctreeNode );
}


// Client moves while being pushed back, and server sets position in one step.
void CDnWorldHitMoveDamageBrokenProp::Process( EtMatrix *pmatWorld, LOCAL_TIME LocalTime, float fDelta )
{
	CDnWorldActProp::Process( pmatWorld, LocalTime, fDelta );
	if( m_hMonster )
		*m_hMonster->GetMatEx() = *GetMatEx();

	m_bActivate = (GetElementIndex( GetCurrentAction() ) == m_iActivateElementIndex );

	// Leave it like this for now, and find another way if server and client prop positions diverge.
	// Utilize navigation mesh!
	if( m_bHitMove )
	{
		m_fMovedTime += fDelta;

		EtVector3 vPrevPos = m_matExWorld.m_vPosition;
		EtVector3 vTargetPos;
		EtVector3* pNowPos = this->GetPosition();

		if( m_fMovedTime < m_fWholeMoveTime )
		{
			vTargetPos = m_vHitPosition + m_vVelocity*m_fMovedTime + 0.5f*m_vResist*m_fMovedTime*m_fMovedTime;
		}
		else
		{
			// Just set as destination
			vTargetPos = m_vHitPosition + m_vVelocity*m_fWholeMoveTime + 0.5f*m_vResist*m_fWholeMoveTime*m_fWholeMoveTime;
			m_bHitMove = false;
			m_fMovedTime = 0.0f;
		}

		MatrixEx m_matTarget = m_matExWorld;
		m_matTarget.m_vPosition = vTargetPos;

		DNVector(SCollisionResponse) vecResult;
		m_Handle->FindCollision( m_matTarget, vecResult );

		bool bCollision = false;
		if( vecResult.size() > 1 )
		{
			for( int i=0; i<(int)vecResult.size(); i++ )
			{
				if( vecResult[i].pCollisionPrimitive->Type != CT_CAPSULE )
				{
					bCollision = true;
				}
				else
				{
					SCollisionCapsule* pCollisionCapsule = dynamic_cast<SCollisionCapsule*>( vecResult[i].pCollisionPrimitive );
					if( pCollisionCapsule && pCollisionCapsule->fRadius != 18.0f )	// Since hit actors cannot be distinguished, distinguish them using radius 18.0f
					{
						bCollision = true;
						m_bHitMove = false;
						m_fMovedTime = 0.0f;
					}
				}
			}
		}

		if( !bCollision )
			m_matExWorld.m_vPosition = vTargetPos;

		_GetPosProperly( vPrevPos );
		_UpdatePropPos();
	}
	else
	if( m_bActivate &&
		false == m_bArrived &&
		MOVE_AXIS_NONE != m_iMovingAxis &&
		0.0f != m_fAxisMovingSpeed )
	{
		EtVector3 vPrevPos = m_matExWorld.m_vPosition;

		// Move if default movement state along the specified axis is defined.
		m_matExWorld.MoveLocalZAxis( m_fAxisMovingSpeed * fDelta );

		// If past the destination position, treat as arrived.
		EtVector3 vMovableLength = m_vInitialPos - m_vDestPosition;
		float fMovableLengthSQ = EtVec3LengthSq( &vMovableLength );
		EtVector3 vMovedLength = m_vInitialPos - m_matExWorld.m_vPosition;
		float fCurrentMovedLength = EtVec3LengthSq( &vMovedLength );
		if( fMovableLengthSQ < fCurrentMovedLength )
		{
			m_matExWorld.m_vPosition = m_vDestPosition;
			m_bArrived = true;
		}

		_GetPosProperly( vPrevPos );
		_UpdatePropPos();
	}
}

// Almost same as broken prop. Difference is it moves when hit and explodes to deal damage when durability reaches 0.
// Dealing damage is handled on the server.
void CDnWorldHitMoveDamageBrokenProp::OnDamage( CDnDamageBase *pHitter, CDnActor::SHitParam &HitParam )
{
	CDnWorldBrokenProp::OnDamage( pHitter, HitParam );

	if( false == m_bBroken )
	{
		// First set the position received from the server,
//		m_matExWorld.SetPosition( HitParam.vPosition );
		m_matWorld = m_matExWorld;
		m_Handle->Update( &m_matWorld );

		// Reversely adjust position value as well.
		EtVector3 vOffset = *m_pParentSector->GetOffset();
		vOffset.x -= ( m_pParentSector->GetTileWidthCount() * m_pParentSector->GetTileSize() ) / 2.f;
		vOffset.z -= ( m_pParentSector->GetTileHeightCount() * m_pParentSector->GetTileSize() ) / 2.f;
		vOffset.y = 0.f;
		m_vPosition = m_matExWorld.m_vPosition - vOffset;

		// Calculate flight time to destination.
		m_fVelocity = EtVec3Length( &HitParam.vVelocity );
		m_fResist = EtVec3Length( &HitParam.vResistance );

		if( 0.0f == m_fVelocity && 0.0f == m_fResist )
			m_fWholeMoveTime = 0.0f;
		else
			m_fWholeMoveTime = m_fVelocity / m_fResist;

		if( m_fWholeMoveTime > 0.2f )
			m_fWholeMoveTime = 0.2f;

		// If movement axis is fixed, handle with fixed direction.
		if( MOVE_AXIS_NONE != m_iMovingAxis &&
			0.0f != m_fAxisMovingSpeed )
		{
			switch( m_iMovingAxis )
			{
				case MOVE_PROPLOCAL_AXIS_X:
					m_vDir = -m_matExWorld.m_vXAxis;
					break;

				case MOVE_PROPLOCAL_AXIS_Y:
					m_vDir = -m_matExWorld.m_vYAxis;
					break;

				case MOVE_PROPLOCAL_AXIS_Z:
					m_vDir = -m_matExWorld.m_vZAxis;
					break;
			}

			EtVec3Normalize( &m_vDir, &m_vDir );

			// Turn on flag to resume axis movement when hit even if it reached the end.
			m_bArrived = false;
		}
		else
		{
			m_vDir.y = 0.0f;
			EtVec3Normalize( &m_vDir, &HitParam.vViewVec );
			m_vDir = -m_vDir;
		}

		// Apply a certain force in the hit direction. 
		// Process pushback in proportion to durability reduced.
		m_vVelocity = m_vDir * 200.0f * m_fHitDurability;			// Initial velocity
		m_vResist = -m_vDir * 80.0f;			// Resistance, friction
		m_vHitPosition = m_matExWorld.m_vPosition;

		m_bHitMove = true;
	}
	else
	{
		// For an axis-moving object that is broken, set it to the start position.
		if( false == m_bArrived &&
			MOVE_AXIS_NONE != m_iMovingAxis &&
			0.0f != m_fAxisMovingSpeed )
		{
			m_matExWorld.m_vPosition = m_vInitialPos;
			m_bArrived = true;
			_UpdatePropPos();
		}
	}
}

void CDnWorldHitMoveDamageBrokenProp::OnDamageReadAdditionalPacket( CDnDamageBase::SHitParam &HitParam, CPacketCompressStream *pStream )
{
	pStream->Read( &m_matExWorld.m_vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
//	pStream->Read( &HitParam.vViewVec, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
	pStream->Read( &HitParam.vVelocity, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	pStream->Read( &HitParam.vResistance, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	pStream->Read( &m_fHitDurability, sizeof(float) );
}
