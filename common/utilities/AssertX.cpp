#include "stdafx.h"
#include "AssertX.h"
#include "SundriesFunc.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef  __cplusplus
extern "C" {
#endif
	_CRTIMP void __cdecl _wassert(__in_z const wchar_t * _Message, __in_z const wchar_t *_File, __in unsigned _Line);
#ifdef  __cplusplus
}
#endif

void NullAssert(const wchar_t * _Message, const wchar_t *_File, unsigned _Line)
{
	char szMsg[255] = { 0, }, szFile[255]={0,};
	WideCharToMultiByte( CP_ACP, 0, _Message, -1, szMsg, 64, NULL, NULL );
	WideCharToMultiByte( CP_ACP, 0, _File, -1, szFile, 64, NULL, NULL );

	OutputDebug("[Assert] %s at %s:%d\n", szMsg, szFile, _Line);
}

#if defined(NDEBUG)
void (*s_passert)( const wchar_t*, const wchar_t*, unsigned ) = NullAssert;
#else
void (*s_passert)( const wchar_t*, const wchar_t*, unsigned ) = _wassert;
#endif

void SetAssertFuncPtr( void (*Func)( const wchar_t*, const wchar_t*, unsigned ) )
{
	s_passert = Func;
}

void EnableAssert( bool bEnable )
{
	if( bEnable ) {
		SetAssertFuncPtr( _wassert );
	}
	else {
		SetAssertFuncPtr( NullAssert );
	}
}
