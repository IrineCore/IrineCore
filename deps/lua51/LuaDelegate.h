#pragma once

#include <windows.h>
#include <string>
using namespace std;

extern "C" 
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};

#define LUA_GLUE extern "C" int 
extern "C" 
{
typedef int (*LuaFunctionType)(struct lua_State* pLuaState);
};


// For convenient singleton usage
#define LUA_DELEGATE	LuaDelegate_Singleton()


//--------------------------------------------------------------------------//
// Class Name: CLuaDelegate
//
// Author : Han-gi Jeong
//
// Desc: Lua interface delegate		  
//--------------------------------------------------------------------------//
class CLuaDelegate
{
private:
	lua_State*		m_pLuaState;
	void (*m_pErrorHandler)( const char* pError );

	// This is a singleton.
	CLuaDelegate(void);

	// If the text file cannot be found, try looking for the binary file (lub).
	void			_ConvertProperFileExt( IN OUT string& strFileName );

public:
	~CLuaDelegate(void);

	// Allow only in debug mode.

//#if defined (_DEBUG) | (DEBUG)
#if defined (_DEBUG)
	void StartDebugConsole( HINSTANCE hInst );
#endif

	bool RunScript( const char* pFileName );
	bool RunString( const char* pCommand );
	const char* GetErrorString( void );
	bool AddFunction( const char* pFunctionName, LuaFunctionType pFunc );
	const char* GetStringArgument( int iNum, const char* pDefault = NULL );
	double GetNumberArgument( int iNum, double dDefault = 0.0 );
	void PushString( const char* pString );
	void PushNumber( double dValue );

	// Get global variable
	double GetNumber( const char* pVarName );
	const char* GetString( const char* pVarName );

	// Set global variable
	void SetNumber( const char* pVarName, double dNumber );
	void SetString( const char* pVarName, const char* pString );

	// Get table.
	int GetTable( const char* pTableName );

	// Insert a key-value pair into the table.
	void AddToTable( int iTableIndex, const char* pStrKey, const char* pStrValue );
	void AddToTable( int iTableIndex, const char* pStrKey, lua_Number NumericValue );

	// Assumes the table is at the top of the stack.
	void GetField( const char* pStrKey, /*IN OUT*/ string& strValue  );
	lua_Number GetField( const char* pStrKey );

	void SetErrorHandler( void(*pErrorHandler)(const char* pError) );

	lua_State* GetLuaState( void );

	friend CLuaDelegate& LuaDelegate_Singleton( void );
};
