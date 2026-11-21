#pragma once

#include "EtMathPortable.h"
#include "EtMathWrapperD3D.h"

#define ET_PI					( (FLOAT )3.141592654f )
#define EtToRadian( degree )	( ( degree ) * ( ET_PI / 180.0f ) )
#define EtToDegree( radian )	( ( radian ) * ( 180.0f / ET_PI ) )

enum EffectParamType 
{
	EPT_INT			= 0,
	EPT_FLOAT		= 1,
	EPT_VECTOR		= 2,
	EPT_TEX			= 3,
	EPT_MATRIX		= 4,
	EPT_VARIABLE	= 5,
	EPT_INT_PTR		= 6,
	EPT_FLOAT_PTR	= 7,
	EPT_VECTOR_PTR	= 8,
	EPT_MATRIX_PTR	= 9,
	EPT_UNKNOWN		= 0xffffffff
};
