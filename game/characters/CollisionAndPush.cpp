#include "stdafx.h"
#include "CollisionAndPush.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

void ProcessCollisionAndPush( EtObjectHandle hObject, MatrixEx &matEx )
{
	// Handle pushing Basil later..
	// by mapping
/*	std::vector< SCollisionResponse > vecResult;

	if( hObject->FindCollision( Cross, vecResult, true, false ) )
	{
		int i;
		EtVector3 vMove;
		CCollisionCallback *pCallback;

		for( i = 0; i < ( int )vecResult.size(); i++ )
		{
			pCallback = vecResult[ i ].pCollisionPrimitive->GetCallback();
			if( pCallback )
			{
				vMove = ( 1.0f - vecResult[ i ].fContactTime ) * vecResult[ i ].vMove;
				if( EtVec3LengthSq( &vMove ) > 0.0f )
				{
					pCallback->ProcessCollision( vMove );
				}
			}
		}
	}*/
}
