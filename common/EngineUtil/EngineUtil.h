#pragma once

EtVector2 EtVec3toVec2( const EtVector3 &vVec );
EtVector3 EtVec2toVec3( const EtVector2 &vVec );
bool CheckRect( float x, float y, const EtVector2 &VecMin, const EtVector2 &VecMax );
float RandomNumberInRange( float fMin, float fMax );

#ifndef EtClamp
#define EtClamp( value , minValue, maxValue )  ( ( (value) > (maxValue) ) ? (maxValue) : ( (value) < (minValue) ? (minValue) : (value) ) )
#endif
#ifndef EtAcos
#define EtAcos( value ) (  acosf( EtClamp( (value) , -1.0f, 1.0f ) ) )
#endif
