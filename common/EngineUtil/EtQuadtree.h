#pragma once

#include "EtPrimitive.h"
#include "EtCollisionFunc.h"

template < class T >
class CEtQuadtreeNode:public TBoostMemoryPool< CEtQuadtreeNode<T> >
{
public:
	CEtQuadtreeNode()
	{
		Init();
	}
	CEtQuadtreeNode( SCircle &Circle )
	{
		Init();
		m_NodeCircle = Circle;
	}
	~CEtQuadtreeNode()
	{
		int i;

		for( i = 0; i < 4; i++ )
		{
			SAFE_DELETE( m_pChildNode[ i ] );
		}
	}

protected:
	SCircle m_NodeCircle;
	CEtQuadtreeNode *m_pParent;
	CEtQuadtreeNode *m_pChildNode[ 4 ];

	std::vector< T > m_vecObjectList;

	static float s_fMinRadius;

public:
	void Init()
	{
		m_pParent = NULL;
		memset( m_pChildNode, 0, sizeof( CEtQuadtreeNode * ) * 4 );
	}
	CEtQuadtreeNode *GetParent()
	{
		return m_pParent;
	}
	int GetObjectCount()
	{
		return ( int )m_vecObjectList.size();
	}
	T GetObject( int nIndex )
	{
		return m_vecObjectList[ nIndex ];
	}
	void SetRange( const EtVector2 &CenterPos, float fRadius )
	{
		m_NodeCircle.Center = CenterPos;
		m_NodeCircle.fRadius = fRadius;
	}

	void GetAllObject( std::vector< T > &vecPickObject )
	{
		if( !m_vecObjectList.empty() )
		{
			for( DWORD i=0; i<m_vecObjectList.size(); i++ )
				vecPickObject.push_back( m_vecObjectList[i] );
//			std::copy( m_vecObjectList.begin(), m_vecObjectList.end(), vecPickObject.end() );
		}

		int i;
		for( i = 0; i < 4; i++ )
		{
			if( m_pChildNode[ i ] )
			{
				m_pChildNode[ i ]->GetAllObject( vecPickObject );
			}
		}
	}
	CEtQuadtreeNode *Insert( T Object, SCircle &Circle )
	{
		if( m_NodeCircle.fRadius <= s_fMinRadius )
		{
			m_vecObjectList.push_back( Object );
			return this;
		}
		if( Circle.fRadius > m_NodeCircle.fRadius )
		{
			if( m_pParent )
			{
				return m_pParent->Insert( Object, Circle );
			}
			else
			{
				ASSERT( 0 && "Object Is Too Big" );
				return NULL;
			}
			return this;
		}

		bool bInside[ 4 ];
		int i, nInsideCount, nChildIndex = 0;
		float fChildPivot;
		SCircle ChildCircle[ 4 ];
		static EtVector2 CenterTable[ 8 ] = 
		{
			EtVector2( 1.0f, 1.0f ),
			EtVector2( 1.0f, -1.0f ),
			EtVector2( -1.0f, 1.0f ),
			EtVector2( -1.0f, -1.0f ),
		};

		fChildPivot = ( m_NodeCircle.fRadius / 1.414213f ) * 0.5f;
		nInsideCount = 0;
		for( i = 0; i < 4; i++ )
		{
			ChildCircle[ i ].Center = m_NodeCircle.Center + CenterTable[ i ] * fChildPivot;
			ChildCircle[ i ].fRadius = m_NodeCircle.fRadius * 0.5f;
			bInside[ i ] = false;
			if( ChildCircle[ i ].IsInside( Circle ) )
			{
				nChildIndex = i;
				nInsideCount++;
				bInside[ i ] = true;
			}
		}

		if( nInsideCount == 0 )
		{
			m_vecObjectList.push_back( Object );
			return this;
		}
		else if( nInsideCount >= 2 )
		{
			float fMinDist, fDist;

			fMinDist = FLT_MAX;
			for( i = 0; i < 4; i++ )
			{
				if( bInside[ i ] )
				{
					EtVector2 vDiff = Circle.Center - ChildCircle[ i ].Center;
					fDist = EtVec2LengthSq( &vDiff );
					if( fDist < fMinDist )
					{
						fMinDist = fDist;
						nChildIndex = i;
					}
				}
			}
		}
		if( m_pChildNode[ nChildIndex ] == NULL )
		{
			m_pChildNode[ nChildIndex ] = new CEtQuadtreeNode( ChildCircle[ nChildIndex ] );
		}
		return m_pChildNode[ nChildIndex ]->Insert( Object, Circle );
	}
	bool Remove( T Object )
	{
		typename std::vector< T >::iterator Iter;
		Iter = std::find( m_vecObjectList.begin(), m_vecObjectList.end(), Object );
		if( Iter != m_vecObjectList.end() )
		{
			m_vecObjectList.erase( Iter );
			return true;
		}

		int i;

		for( i = 0; i < 4; i++ )
		{
			if( m_pChildNode[ i ] == NULL )
			{
				continue;
			}
			if( m_pChildNode[ i ]->Remove( Object ) )
			{
				return true;
			}
		}

		return false;
	}
	CEtQuadtreeNode *Update( T Object, SCircle &Circle )
	{
		if( m_NodeCircle.IsInside( Circle ) )
		{
			return this;
		}
		else
		{
			return NULL;
		}
	}

	void Pick( SCircle &Circle, std::vector< T > &vecPickObject, bool bInside )
	{
		if( !TestCircleToCircle( Circle, m_NodeCircle ) )
		{
			return;
		}
		if( Circle.IsInside( m_NodeCircle ) )
		{
			GetAllObject( vecPickObject );
		}
		else
		{
			int i;
			for( i = 0; i < ( int )m_vecObjectList.size(); i++ )
			{
				SCircle ObjectCircle;
				m_vecObjectList[ i ]->GetBoundingCircle( ObjectCircle );
				if( bInside )
				{
					if( Circle.IsInside( ObjectCircle ) )
					{
						vecPickObject.push_back( m_vecObjectList[ i ] );
					}
				}
				else
				{
					if( TestCircleToCircle( Circle, ObjectCircle ) )
					{
						vecPickObject.push_back( m_vecObjectList[ i ] );
					}
				}
			}
			for( i = 0; i < 4; i++ )
			{
				if( m_pChildNode[ i ] )
				{
					m_pChildNode[ i ]->Pick( Circle, vecPickObject, bInside );
				}
			}
		}
	}

	void Pick( const EtVector2 &vPoint, std::vector< T > &vecPickObject )
	{
		if( !m_NodeCircle.IsInside( vPoint ) )
		{
			return;
		}

		int i;
		for( i = 0; i < ( int )m_vecObjectList.size(); i++ )
		{
			SCircle ObjectCircle;
			m_vecObjectList[ i ]->GetBoundingCircle( ObjectCircle );
			if( ObjectCircle.IsInside( vPoint ) )
			{
				vecPickObject.push_back( m_vecObjectList[ i ] );
			}
		}
		for( i = 0; i < 4; i++ )
		{
			if( m_pChildNode[ i ] )
			{
				m_pChildNode[ i ]->Pick( vPoint, vecPickObject );
			}
		}
	}

	void Pick( const EtVector2 &vPoint, T *pPickObject, int &nCurIndex, int nMaxSize )
	{
		if( !m_NodeCircle.IsInside( vPoint ) )
		{
			return;
		}

		int i;
		for( i = 0; i < ( int )m_vecObjectList.size(); i++ )
		{
			SCircle ObjectCircle;
			m_vecObjectList[ i ]->GetBoundingCircle( ObjectCircle );
			if( ObjectCircle.IsInside( vPoint ) )
			{
				if( nCurIndex < nMaxSize ) {
					pPickObject[ nCurIndex ] = m_vecObjectList[ i ];
					nCurIndex++;
				}
			}
		}
		for( i = 0; i < 4; i++ )
		{
			if( m_pChildNode[ i ] )
			{
				m_pChildNode[ i ]->Pick( vPoint, pPickObject, nCurIndex, nMaxSize );
			}
		}
	}
};

template < class T >
class CEtQuadtree
{
public:
	CEtQuadtree(void) 
	{
		m_pRootNode = NULL;
		m_bInit = false;
	}
	virtual ~CEtQuadtree(void) 
	{
		SAFE_DELETE( m_pRootNode );
	}

protected:
	bool m_bInit;
	CEtQuadtreeNode< T > *m_pRootNode;

public:
	void Initialize( const EtVector2 &CenterPos, float fWorldSize )
	{
		m_bInit = true;
		float fRootRadius = fWorldSize * 1.414214f * 0.5f;
		SAFE_DELETE( m_pRootNode );
		m_pRootNode = new CEtQuadtreeNode< T >();
		m_pRootNode->SetRange( CenterPos, fRootRadius );
	}
	CEtQuadtreeNode< T > *Insert( T Object, SCircle &Circle )
	{
		if( !m_bInit )
		{
			return NULL;
		}
		return m_pRootNode->Insert( Object, Circle );
	}
	bool Remove( T Object, CEtQuadtreeNode< T > *pCurNode )
	{
		if( !m_bInit )
		{
			return false;
		}
		if( pCurNode )
		{
			return pCurNode->Remove( Object );
		}
		else
		{
			return m_pRootNode->Remove( Object );
		}
	}
	CEtQuadtreeNode< T > *Update( T Object, SCircle &Circle, CEtQuadtreeNode< T > *pCurNode )
	{
		if( !m_bInit )
		{
			return NULL;
		}

		CEtQuadtreeNode< T > *pUpdateNode;

		pUpdateNode = pCurNode->Update( Object, Circle );
		if( pUpdateNode == NULL )
		{
			CEtQuadtreeNode< T > *pParentNode;

			Remove( Object, pCurNode );
			pParentNode = pCurNode->GetParent();
			if( pParentNode )
			{
				pUpdateNode = pParentNode->Insert( Object, Circle );
			}
			if( pUpdateNode == NULL )
			{
				pUpdateNode = m_pRootNode->Insert( Object, Circle );
			}
			ASSERT( pUpdateNode );
		}

		return pUpdateNode;
	}

	void Pick( SCircle &Circle, std::vector< T > &vecPickObject, bool bInside = false )
	{
		if( !m_bInit )
		{
			return;
		}
		m_pRootNode->Pick( Circle, vecPickObject, bInside );
	}

	void Pick( const EtVector2 &vPoint, std::vector< T > &vecPickObject )
	{
		if( !m_bInit )
		{
			return;
		}
		m_pRootNode->Pick( vPoint, vecPickObject );
	}

	int Pick( const EtVector2 &vPoint, T* pPickObject, int nMaxSize )
	{
		if( !m_bInit )
		{
			return 0;
		}
		int nCurrentIndex = 0;
		m_pRootNode->Pick( vPoint, pPickObject, nCurrentIndex, nMaxSize );
		return nCurrentIndex;
	}

	template< int _Size>
	int Pick( const EtVector2 &vPoint, T (&pPickObject)[_Size] )
	{
		return Pick( vPoint, pPickObject, _Size);
	}
};
