/* Copyright (C) Greg Snook, 2000. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Greg Snook, 2000"
 */
/****************************************************************************************\
	NavigationMesh.cpp

	NavigationMesh component implementation for the Navimesh sample program.
	Included as part of the Game Programming Gems sample code.

	Created 3/18/00 by Greg Snook
	greg@mightystudios.com
    ------------------------------------------------------------------------------------- 
	Notes/Revisions:

\****************************************************************************************/

#include "stdafx.h"
#define NAVIGATIONMESH_CPP
#include "navigationmesh.h"
#include "navigationpath.h"
#include <assert.h>
#include "Stream.h"

//:	SnapPointToCell
//----------------------------------------------------------------------------------------
//
// Force a point to be inside the cell
//
//-------------------------------------------------------------------------------------://
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

float CEtQuadtreeNode< NavigationCell * >::s_fMinRadius = 1000.0f;


void NavigationMesh::CreateNaviCells( int iCount )
{
	_ASSERT( m_apNavigationCells == NULL );
	m_apNavigationCells = new NavigationCell* [ iCount ];
	m_iWholeCountOfCell = iCount;
}

//:	SnapPointToMesh
//----------------------------------------------------------------------------------------
//
// Force a point to be inside the nearest cell on the mesh
//
//-------------------------------------------------------------------------------------://
EtVector3 NavigationMesh::SnapPointToMesh(NavigationCell** CellOut, const EtVector3& Point)
{
	EtVector3 PointOut = Point;
	*CellOut = FindClosestCell(PointOut);
	_ASSERT( *CellOut );
	return (FittingPoint(*CellOut, PointOut));
}

EtVector3 NavigationMesh::FittingPoint(NavigationCell* Cell, const EtVector3& Point)
{
	EtVector3 PointOut = Point;

	if (!Cell->IsPointInCellCollumn(PointOut))
	{
		Cell->ForcePointToCellCollumn(PointOut);
	}

	Cell->MapVectorHeightToCell(PointOut);
	return (PointOut);
}


//:	FindClosestCell
//----------------------------------------------------------------------------------------
//
// Find the closest cell on the mesh to the given point
//
//-------------------------------------------------------------------------------------://
NavigationCell* NavigationMesh::FindClosestCell(const EtVector3& Point)const
{
	float ClosestDistance = 3.4E+38f;
	float ClosestHeight = 3.4E+38f;
	bool FoundHomeCell = false;
	float ThisDistance;
	NavigationCell* ClosestCell=0;

	for( int i = 0; i < m_iWholeCountOfCell; i++ )
	{
		NavigationCell* pCell = m_apNavigationCells[i];
		if (pCell->IsPointInCellCollumn(Point))
		{
			EtVector3 NewPosition(Point);
			pCell->MapVectorHeightToCell(NewPosition);
			ThisDistance = fabsf(NewPosition.y - Point.y);
			if (FoundHomeCell)
			{
				if (ThisDistance < ClosestHeight)
				{
					ClosestCell = pCell;
					ClosestHeight = ThisDistance;
				}
			}
			else
			{
				ClosestCell = pCell;
				ClosestHeight = ThisDistance;
				FoundHomeCell = true;
			}
		}

		if (!FoundHomeCell)
		{
			EtVector2 Start(pCell->CenterPoint().x, pCell->CenterPoint().z);
			EtVector2 End(Point.x, Point.z);
			Line2D MotionPath(Start, End);
			NavigationCell* NextCell;
			NavigationCell::CELL_SIDE WallHit;
			EtVector2 PointOfIntersection;

			NavigationCell::PATH_RESULT Result = pCell->ClassifyPathToCell(MotionPath, &NextCell, WallHit, &PointOfIntersection);

			if (Result == NavigationCell::EXITING_CELL)
			{
				EtVector3 ClosestPoint3D(PointOfIntersection.x,0.0f,PointOfIntersection.y);
				pCell->MapVectorHeightToCell(ClosestPoint3D);
				ClosestPoint3D -= Point;
				ThisDistance = EtVec3Length( &ClosestPoint3D );
				if (ThisDistance<ClosestDistance)
				{
					ClosestDistance=ThisDistance;
					ClosestCell = pCell;
				}
			}
		}
	}
	return (ClosestCell);
}

NavigationCell* NavigationMesh::FindCell( const EtVector3 &vPoint )
{
	NavigationCell* pResult = NULL;
	NavigationCell* pNowCell = NULL;
	NavigationCell* pFindNavCell[ 256 ] = { NULL, };

	int iMaxCount = m_NaviCellQuadTree.Pick( EtVector2( vPoint.x, vPoint.z ), pFindNavCell );	

	for( int i = 0; i < iMaxCount; i++ )
	{
		if( pFindNavCell[ i ]->IsPointInCellCollumn( vPoint ) )
		{
			if( !pNowCell )
			{
				pNowCell = pFindNavCell[ i ];
				continue;
			}

			if( fabs( pNowCell->Vertex( 0 ).y - vPoint.y ) > fabs( pFindNavCell[ i ]->Vertex( 0 ).y - vPoint.y ) )
			{
				pNowCell = pFindNavCell[ i ];
			}
		}
	}

	pResult = pNowCell;
	return pResult;
}

//:	BuildNavigationPath
//----------------------------------------------------------------------------------------
//
// Build a navigation path using the provided points and the A* method
//
//-------------------------------------------------------------------------------------://
bool NavigationMesh::BuildNavigationPath(NavigationPath& NavPath, NavigationCell* StartCell, const EtVector3& StartPos, NavigationCell* EndCell, const EtVector3& EndPos)
{
	bool FoundPath = false;

	// Increment our path finding session ID
	// This Identifies each pathfinding session
	// so we do not need to clear out old data
	// in the cells from previous sessions.
	++m_PathSession;

	if( !StartCell->IsUsing() ||
		 !EndCell->IsUsing() )
	{
		return false;
	}

	// load our data into the NavigationHeap object
	// to prepare it for use.
	m_NavHeap.Setup(m_PathSession, StartPos);

	// We are doing a reverse search, from EndCell to StartCell.
	// Push our EndCell onto the Heap at the first cell to be processed
	EndCell->QueryForPath(&m_NavHeap, 0, 0);

	// process the heap until empty, or a path is found
	while(m_NavHeap.NotEmpty() && !FoundPath)
	{
		NavigationNode ThisNode;

		// pop the top cell (the open cell with the lowest cost) off the Heap
		m_NavHeap.GetTop(ThisNode);

		// if this cell is our StartCell, we are done
		if(ThisNode.cell == StartCell)
		{
			FoundPath = true;
		}
		else
		{
			// Process the Cell, Adding it's neighbors to the Heap as needed
			ThisNode.cell->ProcessCell(&m_NavHeap);
		}
	}

	// if we found a path, build a waypoint list
	// out of the cells on the path
	if (FoundPath)
	{
		NavigationCell* TestCell = StartCell;
		EtVector3 NewWayPoint;

		// Setup the Path object, clearing out any old data
		NavPath.Setup(this, StartPos, StartCell, EndPos, EndCell);

		// Step through each cell linked by our A* algorythm 
		// from StartCell to EndCell
		while (TestCell && TestCell != EndCell)
		{
			// add the link point of the cell as a way point (the exit wall's center)
			int LinkWall = TestCell->ArrivalWall();

			NewWayPoint = TestCell->WallMidpoint(LinkWall);
			NewWayPoint = FittingPoint(TestCell, NewWayPoint); // just to be sure

			NavPath.AddWayPoint(NewWayPoint, TestCell);
			if( NavPath.WaypointList().size() >= NUM_MAX_WAYPOINT )
			{
				NavPath.WaypointList().clear();
				return false;
			}

			// and on to the next cell
			TestCell = TestCell->Link(LinkWall);
		}

		// cap the end of the path.
		NavPath.EndPath();
		return(true);
	}
	return(false);
}

//:	ResolveMotionOnMesh
//----------------------------------------------------------------------------------------
//
// Resolve a movement vector on the mesh
//
//-------------------------------------------------------------------------------------://
bool NavigationMesh::ResolveMotionOnMesh(const EtVector3& StartPos, NavigationCell* StartCell, EtVector3& EndPos, NavigationCell** EndCell)
{
	// create a 2D motion path from our Start and End positions, tossing out their Y values to project them 
	// down to the XZ plane.
	Line2D MotionPath(EtVector2(StartPos.x,StartPos.z), EtVector2(EndPos.x,EndPos.z));

	// these three will hold the results of our tests against the cell walls
	NavigationCell::PATH_RESULT Result = NavigationCell::NO_RELATIONSHIP;
	NavigationCell::CELL_SIDE WallNumber;
	EtVector2 PointOfIntersection;
	NavigationCell* NextCell;

	// TestCell is the cell we are currently examining.
	NavigationCell* TestCell = StartCell;

	EtVector2 vTemp;
	bool bNotRelevant = false;
	bool bSolidWall = false;

	//
	// Keep testing until we find our ending cell or stop moving due to friction
	//
	while ((Result != NavigationCell::ENDING_CELL) && (MotionPath.Length() > 0.5f))
	{
		// use NavigationCell to determine how our path and cell interact
		Result = TestCell->ClassifyPathToCell(MotionPath, &NextCell, WallNumber, &PointOfIntersection);

		// if exiting the cell...
		if (Result == NavigationCell::EXITING_CELL)
		{
			// Set if we are moving to an adjacent cell or we have hit a solid (unlinked) edge
			if(NextCell)
			{
				// moving on. Set our motion origin to the point of intersection with this cell
				// and continue, using the new cell as our test cell.
				MotionPath.SetEndPointA(PointOfIntersection);
				TestCell = NextCell;

			}
			else
			{
				// we have hit a solid wall. Resolve the collision and correct our path.
				MotionPath.SetEndPointA(PointOfIntersection);
				TestCell->ProjectPathOnCellWall(WallNumber, MotionPath);
				bSolidWall = true;

				// add some friction to the new MotionPath since we are scraping against a wall.
				// we do this by reducing the magnatude of our motion by 10%
				EtVector2 Direction = MotionPath.EndPointB() - MotionPath.EndPointA();
				Direction *= 0.9f;
				MotionPath.SetEndPointB(MotionPath.EndPointA() + Direction);

			}
			bNotRelevant = false;
		}
		else if (Result == NavigationCell::NO_RELATIONSHIP)
		{
			// Although theoretically we should never encounter this case,
			// we do sometimes find ourselves standing directly on a vertex of the cell.
			// This can be viewed by some routines as being outside the cell.
			// To accomodate this rare case, we can force our starting point to be within
			// the current cell by nudging it back so we may continue.
			EtVector2 NewOrigin = MotionPath.EndPointA();

			if( bNotRelevant )
			{
				if( vTemp.x == NewOrigin.x && vTemp.y == NewOrigin.y )
				{
					break;
				}
			}
			vTemp = NewOrigin;
			bNotRelevant = true;

			TestCell->ForcePointToCellCollumn(NewOrigin);
			MotionPath.SetEndPointA(NewOrigin);
		}
	}

	// we now have our new host cell
	*EndCell = TestCell;

	// Update the new control point position, 
	// solving for Y using the Plane member of the NavigationCell
	EndPos.x = MotionPath.EndPointB().x;
	EndPos.z = MotionPath.EndPointB().y;
	TestCell->MapVectorHeightToCell(EndPos);

	return bSolidWall;
}


//:	LineOfSightTest
//----------------------------------------------------------------------------------------
//
// Test to see if two points on the mesh can view each other
//
//-------------------------------------------------------------------------------------://
bool NavigationMesh::LineOfSightTest(NavigationCell* StartCell, const EtVector3& StartPos, NavigationCell* EndCell, const EtVector3& EndPos)
{
	Line2D MotionPath(EtVector2(StartPos.x,StartPos.z), EtVector2(EndPos.x,EndPos.z));
	NavigationCell* NextCell = StartCell;
	NavigationCell::PATH_RESULT eResult;
	NavigationCell::CELL_SIDE eCellSide;

	int iCount = 0;
 	while((eResult = NextCell->ClassifyPathToCell(MotionPath, &NextCell, eCellSide, 0)) == NavigationCell::EXITING_CELL)
	{
		if( ++iCount >= 100 )
			return false;

		if( !NextCell ) return false;
		else if( NextCell == EndCell ) return true;
	}

	return (eResult == NavigationCell::ENDING_CELL);
}

//:	LinkCells
//----------------------------------------------------------------------------------------
//
// Link all the cells that are in our pool
//
//-------------------------------------------------------------------------------------://
void NavigationMesh::LinkCells()
{
	for( int i=0; i<m_iWholeCountOfCell; i++ )
	{
		NavigationCell* pCellA = m_apNavigationCells[i];
		pCellA->SetLink( NavigationCell::SIDE_AB, NULL );
		pCellA->SetLink( NavigationCell::SIDE_BC, NULL );
		pCellA->SetLink( NavigationCell::SIDE_CA, NULL );
		for( int k=0; k<m_iWholeCountOfCell; k++ )
		{
			NavigationCell* pCellB = m_apNavigationCells[k];
			if(i != k)
			{
				if (!pCellA->Link(NavigationCell::SIDE_AB) && pCellB->RequestLink(pCellA->Vertex(0), pCellA->Vertex(1), pCellA))
				{
					pCellA->SetLink(NavigationCell::SIDE_AB, pCellB);
				}
				else if (!pCellA->Link(NavigationCell::SIDE_BC) && pCellB->RequestLink(pCellA->Vertex(1), pCellA->Vertex(2), pCellA))
				{
					pCellA->SetLink(NavigationCell::SIDE_BC, pCellB);
				}
				else if (!pCellA->Link(NavigationCell::SIDE_CA) && pCellB->RequestLink(pCellA->Vertex(2), pCellA->Vertex(0), pCellA))
				{
					pCellA->SetLink(NavigationCell::SIDE_CA, pCellB);
				}
			}
		}
	}
}


int NavigationMesh::GetIndex( NavigationCell* pCell )
{
	if( pCell == NULL )
		return -1;

	for( int i = 0; i < m_iWholeCountOfCell; i++ )
	{
		if( m_apNavigationCells[ i ] == pCell )
			return i;
	}

	return -1;
}

void NavigationMesh::AddCellCount( int nCount )
{
	if( nCount <= 0 )
	{
		return;
	}

	m_iWholeCountOfCell += nCount;
	m_apNavigationCells = ( NavigationCell** )realloc( m_apNavigationCells, sizeof( NavigationCell* ) * m_iWholeCountOfCell );
}

void NavigationMesh::CopyFrom( NavigationMesh *pMesh )
{
	if( pMesh == NULL )
	{
		return;
	}

	int i;

	pMesh->CreateNaviCells( m_iWholeCountOfCell );
	for( i = 0; i < m_iWholeCountOfCell; i++ )
	{
		EtVector3 vPoint[ 3 ];
		Cell( i )->GetVertex( vPoint );
		pMesh->AddCell( vPoint[ 0 ], vPoint[ 1 ], vPoint[ 2 ], i );
		pMesh->Cell( i )->SetType( Cell( i )->GetType() );
		pMesh->Cell( i )->SetNavType( Cell( i )->GetNavType() );
	}
	pMesh->m_vlLinkableNodes.resize( m_vlLinkableNodes.size() );
	std::copy( m_vlLinkableNodes.begin(), m_vlLinkableNodes.end(), pMesh->m_vlLinkableNodes.begin() );

	pMesh->LinkCells();
}


NavigationCell *NavigationMesh::SplitCell( NavigationCell *pCell, int nEdgeIndex, EtVector3 &vSplitPos )
{
	NavigationCell *pNewCell = new NavigationCell;
	EtVector3 vVertex[ 3 ], vNewCellVertex[ 3 ];

	AddCellCount( 1 );
	pCell->GetVertex( vVertex );
	pCell->GetVertex( vNewCellVertex );

	vNewCellVertex[ nEdgeIndex ] = vSplitPos;
	pNewCell->Initialize( vNewCellVertex[ 0 ], vNewCellVertex[ 1 ], vNewCellVertex[ 2 ] );
	m_apNavigationCells[ m_iWholeCountOfCell - 1 ] = pNewCell;

	vVertex[ ( nEdgeIndex + 1 ) % 3 ] = vSplitPos;
	pCell->Initialize( vVertex[ 0 ], vVertex[ 1 ], vVertex[ 2 ] );

	LinkCells();

	return pNewCell;
}


void NavigationMesh::RemoveAcuteCell()
{
	std::vector< int > vecIndexToDelete;
	for( int i = 0; i < m_iWholeCountOfCell; i++ )
	{
		int iSideCount = 0;
		int iSideIndex = -1;
		for( int k = 0; k < 3; k++ )
		{
			if( m_apNavigationCells[ i ]->Link( k ) )
			{
				iSideCount++;
				iSideIndex = k;
			}
		}
		if( iSideCount != 1 )
		{
			continue;
		}
		iSideIndex = ( iSideIndex + 2 ) % 3;
		EtVector3 vSegmentA, vSegmentB;
		float fLength1, fLength2;
		vSegmentA = m_apNavigationCells[ i ]->Vertex( ( iSideIndex + 1 ) % 3 ) - m_apNavigationCells[ i ]->Vertex( iSideIndex );
		vSegmentB = m_apNavigationCells[ i ]->Vertex( ( iSideIndex + 2 ) % 3 ) - m_apNavigationCells[ i ]->Vertex( iSideIndex );

		vSegmentA.y = 0.0f; vSegmentB.y = 0.0f;

		fLength1 = EtVec3Length( &vSegmentA );
		fLength2 = EtVec3Length( &vSegmentB );

		EtVec3Normalize( &vSegmentA, &vSegmentA );
		EtVec3Normalize( &vSegmentB, &vSegmentB );

		float fDot = fabs( EtVec3Dot( &vSegmentA, &vSegmentB ) );
		if( fDot < 0.258819f )
		{
			if( ( fLength1 < 30.0f ) || ( fLength2 < 30.0f ) )
			{
				vecIndexToDelete.push_back( i );
			}
		}
		else if( fDot > 0.866025f )
		{
			vecIndexToDelete.push_back( i );
		}
	}
	int nLastIndex = ( int )vecIndexToDelete.size() - 1;
	for( int i = nLastIndex; i >= 0; i-- )
	{
		DeleteCell( vecIndexToDelete[ i ] );
	}
	if( !vecIndexToDelete.empty() )
	{
		LinkCells();
	}
}


bool NavigationMesh::SaveToStream( CStream* pStream )
{
	if(!pStream->IsValid()) 
		return false;

	SNavFileHeader Header;
	Header.nSignature = 0xefefefef;
	Header.nVersion = 12;
	Header.nTriangleCount = m_iWholeCountOfCell;
	pStream->Write( &Header, sizeof( SNavFileHeader ) );

	for( int i = 0; i < m_iWholeCountOfCell; i++ )
	{
		m_apNavigationCells[ i ]->SaveToStream( pStream );
	}

	for( int i = 0; i < m_iWholeCountOfCell; i++ )
	{
		for( int k = 0; k < 3; k++ )
		{
			NavigationCell* pNowCell = m_apNavigationCells[ i ]->GetLinked( k );
			int iIndex = GetIndex( pNowCell );
			pStream->Write( &iIndex, sizeof(int) );
		}
	}

	return true;
}

bool NavigationMesh::LoadFromStream( CStream* pStream, EtMatrix *pWorldMat )
{
	if(!pStream->IsValid()) 
		return false;

	_ASSERT( (int)m_iWholeCountOfCell == 0 );

	SNavFileHeader Header;
	pStream->Read( &Header, sizeof( SNavFileHeader ) );
	if( Header.nSignature != 0xefefefef )
	{
		pStream->Seek( 0, SEEK_SET );
		pStream->Read( &m_iWholeCountOfCell, sizeof(int) );
		Header.nVersion = 0;
	}
	else
	{
		m_iWholeCountOfCell = Header.nTriangleCount;
	}

	CreateNaviCells( m_iWholeCountOfCell );

	for( int i = 0; i < m_iWholeCountOfCell; i++ )
	{
		NavigationCell* NewCell = new NavigationCell;

		m_apNavigationCells[ i ] =  NewCell;
		m_apNavigationCells[ i ]->LoadFromStream( pStream, Header.nVersion, pWorldMat );
	}

	m_BoundingBox.Reset();
	for( int i = 0; i < m_iWholeCountOfCell; i++ )
	{
		NavigationCell* pNowCell = Cell( i );
		for( int j = 0; j < 3; j++ )
		{
			int iIndex;
			pStream->Read( &iIndex, sizeof(int) );
			if( iIndex != -1 )
			{
				NavigationCell* pCell = Cell( iIndex );
				pNowCell->SetLink( (NavigationCell::CELL_SIDE)j, pCell );
			}

			m_BoundingBox.AddPoint( ( EtVector3 )pNowCell->Vertex( j ) );
		}
	}

	int iLinkableNodeCount;
	if( pStream->Read( &iLinkableNodeCount, sizeof( int ) ) > 0 )
	{
		if( iLinkableNodeCount )
		{
			m_vlLinkableNodes.resize( iLinkableNodeCount * 2 );
			pStream->Read( &m_vlLinkableNodes[ 0 ], sizeof( int ) * 2 * iLinkableNodeCount );
		}
	}

	float fWorldSize = max( m_BoundingBox.Max.x - m_BoundingBox.Min.x, m_BoundingBox.Max.z - m_BoundingBox.Min.z );
	m_NaviCellQuadTree.Initialize( EtVector2( ( m_BoundingBox.Max.x + m_BoundingBox.Min.x ) * 0.5f, ( m_BoundingBox.Max.z + m_BoundingBox.Min.z ) * 0.5f ), fWorldSize );
	for( int i = 0; i < m_iWholeCountOfCell; i++ )
	{
		NavigationCell *pCurCell = Cell( i );
		SCircle BoundingCircle;
		pCurCell->GetBoundingCircle( BoundingCircle );
		m_NaviCellQuadTree.Insert( pCurCell, BoundingCircle );
	}

	return true;
}

//****************************************************************************************
// end of file      ( NavigationMesh.cpp )
