#include "stdafx.h"
#include "DnWorld.h"
#include "MAWalkMovementNav.h"
#include "DnActor.h"
#include "navigationmesh.h"
#include "navigationcell.h"
#include "PerfCheck.h"
#if defined( _GAMESERVER )
#include "DNGameRoom.h"
#endif // #if defined( _GAMESERVER )

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

MAWalkMovementNav::MAWalkMovementNav(void)
{
	m_pCurCell = NULL;
}

MAWalkMovementNav::~MAWalkMovementNav(void)
{
}

void MAWalkMovementNav::ValidateCurCell()
{
	NavigationMesh *pNavMesh = INSTANCE(CDnWorld).GetNavMesh( m_pMatExWorld->m_vPosition );
	if( !pNavMesh ) return;
	m_pCurCell = pNavMesh->FindCell( m_pMatExWorld->m_vPosition );
}

void MAWalkMovementNav::ProcessNormal( LOCAL_TIME LocalTime, float fDelta )
{
	ProcessCommon( LocalTime, fDelta );
	EtVector3 vPrevPos = *m_pActor->GetPrevPosition();

	ProcessWallCollision( m_pActor, m_pActor->GetObjectHandle(), vPrevPos );
	PostProcess( LocalTime );
}

void MAWalkMovementNav::Process( LOCAL_TIME LocalTime, float fDelta )
{
	NavigationMesh *pNavMesh = INSTANCE(CDnWorld).GetNavMesh( m_pMatExWorld->m_vPosition );

	if( pNavMesh == NULL )
	{
		return ProcessNormal( LocalTime, fDelta );
	}

	if( m_pCurCell == NULL )
	{
		m_pCurCell = pNavMesh->FindCell( m_pMatExWorld->m_vPosition );
		if( m_pCurCell == NULL )
		{
			return ProcessNormal( LocalTime, fDelta );
		}
	}

	PROFILE_TIME_TEST( ProcessCommon( LocalTime, fDelta ) );
	EtVector3 vPrevPos = *m_pActor->GetPrevPosition();

	PROFILE_TIME_TEST( ProcessWallCollision( m_pActor, m_pActor->GetObjectHandle(), vPrevPos ) );

	int nSide = -1;
	NavigationCell *pLastCell = NULL;
	bool bSuccess;
	PROFILE_TIME_TEST_BLOCK_START( "m_pCurCell->FindLastCollision" );
	bSuccess = m_pCurCell->FindLastCollision( vPrevPos, m_pMatExWorld->m_vPosition, &pLastCell, nSide );
	PROFILE_TIME_TEST_BLOCK_END();
#if defined( _GAMESERVER )
	if( bSuccess == false )
	{
		if( m_pActor && m_pActor->GetGameRoom() )
		{
			g_Log.Log( LogType::_NORMAL, L"[GameServer] FindLastCollision InfiniteLoop!!!!!! MapIndex=%d", m_pActor->GetGameRoom()->m_iMapIdx );
		}
	}
#endif // #if defined( _GAMESERVER )

	PROFILE_TIME_TEST_BLOCK_START( "Movement1" );
	if( nSide != -1 )
	{
		int nWallAttribute = pLastCell->GetWallAttribute( nSide ) & 0x0f;
		if( ( nWallAttribute == NAV_WALLATTR_BLUE ) || ( nWallAttribute == NAV_WALLATTR_YELLOW ) )
		{
			OnClash( m_pMatExWorld->m_vPosition, eCFCT_None );
		}
		if( nWallAttribute == NAV_WALLATTR_GREEN )
		{
			OnBumpWall();
		}
		if( pLastCell->Link( nSide ) == NULL )
		{
			EtVector2 vWallDir2D;
			EtVector2 vMoveDir2D( m_pMatExWorld->m_vPosition.x - vPrevPos.x, m_pMatExWorld->m_vPosition.z - vPrevPos.z );
			float fMoveLength = EtVec2Length( &vMoveDir2D );
			vMoveDir2D /= fMoveLength;
			pLastCell->GetWallDir( nSide, vWallDir2D );
			fMoveLength *= EtVec2Dot( &vWallDir2D, &vMoveDir2D );
			m_pMatExWorld->m_vPosition.x = vPrevPos.x + fMoveLength * vWallDir2D.x;
			m_pMatExWorld->m_vPosition.z = vPrevPos.z + fMoveLength * vWallDir2D.y;

			int nNewSide = -1;
			m_pCurCell->FindLastCollision( vPrevPos, m_pMatExWorld->m_vPosition, &pLastCell, nNewSide );
			if( nNewSide != -1 )
			{
				if( pLastCell->Link( nNewSide ) )
				{
					NavigationCell *pNewCell;
					pNewCell = pLastCell->Link( nNewSide );
					if( pNewCell->IsPointInCellCollumn( m_pMatExWorld->m_vPosition ) )
					{
						m_pCurCell = pNewCell;
					}
					else
					{
						m_pMatExWorld->m_vPosition.x = vPrevPos.x;
						m_pMatExWorld->m_vPosition.z = vPrevPos.z;
					}
				}
				else if( !m_pCurCell->IsPointInCellCollumn( m_pMatExWorld->m_vPosition ) )
				{
					m_pMatExWorld->m_vPosition.x = vPrevPos.x;
					m_pMatExWorld->m_vPosition.z = vPrevPos.z;
				}
			}
			// Navigation mode active
			OnBeginNaviMode();
		}
		else
		{
			m_pCurCell = pLastCell->Link( nSide );
		}

		if( !m_pCurCell->IsPointInCellCollumn( m_pMatExWorld->m_vPosition ) )
		{
			m_pCurCell = pNavMesh->FindCell( m_pMatExWorld->m_vPosition );
		}
	}
	PROFILE_TIME_TEST_BLOCK_END();

	PROFILE_TIME_TEST_BLOCK_START( "Movement2" );
	float fHeight = INSTANCE(CDnWorld).GetHeight( m_pMatExWorld->m_vPosition );
	if( m_pCurCell && m_pCurCell->GetType() == NavigationCell::CT_PROP )
		m_pActor->SetAddHeight( m_pCurCell->GetPlane()->SolveForY( m_pMatExWorld->m_vPosition.x, m_pMatExWorld->m_vPosition.z ) - fHeight );
	else
		m_pActor->SetAddHeight( 0.f );

	if( !m_vVelocity.y && false == m_bAppliedYDistance )
		m_pMatExWorld->m_vPosition.y = fHeight + m_pActor->GetAddHeight();
	PROFILE_TIME_TEST_BLOCK_END();

	PROFILE_TIME_TEST( PostProcess( LocalTime ) );
}

void MAWalkMovementNav::ProcessWallCollision( CDnActor *pActor, EtObjectHandle hObject, EtVector3 &vPrevPos )
{
	int i;
	float fMoveDist;
	EtVector3 vMoveDir;
	DNVector(SCollisionResponse) &vecResult = m_vecResult;
	vecResult.clear();

	vMoveDir = m_pMatExWorld->m_vPosition - vPrevPos;
	fMoveDist = EtVec3Length( &vMoveDir );
	vMoveDir /= fMoveDist;
	if( fMoveDist <= 0.0f )
	{
		return;
	}

	int nLoopCount = 0;
	float fTemoMoveDist;
	EtVector3 vTempPrevPos = vPrevPos;
	while( ( fMoveDist > 0.0f ) && ( nLoopCount < 20 ) )
	{
		if( fMoveDist > 20.0f )
		{
			fTemoMoveDist = 20.0f;
		}
		else
		{
			fTemoMoveDist = fMoveDist;
		}
		m_pMatExWorld->m_vPosition = vTempPrevPos + fTemoMoveDist * vMoveDir;
		fMoveDist -= fTemoMoveDist;
		nLoopCount++;
		if( hObject && hObject->FindDynamicCollision( *m_pMatExWorld, vecResult, false ) )
		{
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Filter out ceilings and floors from collision mesh
			for( i = ( int )vecResult.size() - 1; i >= 0; i-- )
			{
				if( EtVec3Dot( &vecResult[ i ].vNormal, &EtVector3( 0.0f, -1.0f, 0.0f ) ) > 0.965925f )	// Check 15 degrees for ceiling
				{
					vecResult.erase( vecResult.begin() + i );
					continue;
				}
				if( EtVec3Dot( &vecResult[ i ].vNormal, &EtVector3( 0.0f, 1.0f, 0.0f ) ) > 0.342020f )
				{
					vecResult.erase( vecResult.begin() + i );
					continue;
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Wall collision
			if( !vecResult.empty() )
			{
				EtVector3 vSlideDir, vWallNormal, vTempMoveDir;
				float fWeight;

				vTempMoveDir = vMoveDir;
				for( i = 0; i < ( int )vecResult.size(); i++ )
				{
					if( EtVec3Dot( &vecResult[ i ].vNormal, &vTempMoveDir ) < 0.0f )	// Ignore if wall normal matches movement direction
					{
						vWallNormal = vecResult[ i ].vNormal;
						vWallNormal.y = 0.0f;
						EtVec3Normalize( &vWallNormal, &vWallNormal );
						CalcSlide( vSlideDir, fWeight, vWallNormal, vTempMoveDir );
						vTempMoveDir = vSlideDir;
						fTemoMoveDist *= fWeight;

						m_pMatExWorld->m_vPosition.x = vTempPrevPos.x + vTempMoveDir.x * fTemoMoveDist;
						m_pMatExWorld->m_vPosition.z = vTempPrevPos.z + vTempMoveDir.z * fTemoMoveDist;
					}
				}
			}
			OnClash( m_pMatExWorld->m_vPosition, eCFCT_None ); // note by kalliste : Set to None since floor collision detection is not needed at this point. Set to NoFloor later if needed.
		}
		vTempPrevPos = m_pMatExWorld->m_vPosition;
	}
}
