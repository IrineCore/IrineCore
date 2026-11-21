#include "stdafx.h"
#include "EtTriggerElement.h"
#include "EtTriggerObject.h"
#include "EtTrigger.h"
#include "StringUtil.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


char *CEtTriggerElement::s_szFuncString[TriggerElementTypeEnum_Amount] = {
	"Condition",
	"Action",
	"",
};
CEtTriggerElement::CEtTriggerElement( CEtTriggerObject *pObject, lua_State *pState )
{
	m_Type = (TriggerElementTypeEnum)-1;
	m_ScriptType = ScriptFile;
	m_pTriggerObj = pObject;
	m_pLuaState = pState;
	m_bSelfOpenLua = ( pState ) ? false : true;
	m_bEnable = true;
	m_LastExecuteTime = -1;
	m_OperatorType = And;
	m_nOperatorIndex = 0;
	memset( m_nOperatorValue, 0, sizeof(m_nOperatorValue) );
}

CEtTriggerElement::~CEtTriggerElement()
{
	ReleaseParamList();

	if( m_pLuaState ) {
		if( m_bSelfOpenLua ) lua_close( m_pLuaState );
		m_pLuaState = NULL;
	}
	m_szScriptName.clear();
	m_szCustomScript.clear();

}

void CEtTriggerElement::ReleaseParamList()
{
	for( DWORD i=0; i<m_VecParamList.size(); i++ ) {
		switch( m_VecParamList[i].Type ) {
			case CEtTrigger::String:
				SAFE_DELETEA( m_VecParamList[i].szValue );
				break;
			case CEtTrigger::Position:
				SAFE_DELETE( m_VecParamList[i].vValue );
				break;
		}
	}
	SAFE_DELETE_VEC( m_VecParamList );
}

bool CEtTriggerElement::Load( CStream *pStream )
{
	pStream->Read( &m_Type, sizeof(int) );
	pStream->Read( &m_ScriptType, sizeof(int) );
	pStream->Read( &m_OperatorType, sizeof(int) );
	pStream->Read( &m_nOperatorIndex, sizeof(int) );
	pStream->Read( m_nOperatorValue, sizeof(m_nOperatorValue) );
	
	switch( m_ScriptType ) {
		case ScriptFile:
			{
				ReadStdString( m_szScriptName, pStream );
				int nCount;
				pStream->Read( &nCount, sizeof(int) );
				int nSize;
				char szBuffer[256] = {0,};
				for( int i=0; i<nCount; i++ ) {
					ParamStruct Struct;
					memset( szBuffer, 0, sizeof(szBuffer) );
					pStream->Read( &Struct.Type, sizeof(int) );
					pStream->Read( &nSize, sizeof(int) );
					pStream->Read( szBuffer, nSize );

					switch( Struct.Type ) {
						case CEtTrigger::Integer:
						case CEtTrigger::Prop:
						case CEtTrigger::Operator:
						case CEtTrigger::EventArea:
						case CEtTrigger::Value:
							Struct.nValue = *(int*)szBuffer;
							break;
						case CEtTrigger::Float:
							Struct.fValue = *(float*)szBuffer;
							break;
						case CEtTrigger::String:
							Struct.szValue = new char[nSize];
							strcpy_s( Struct.szValue, nSize, szBuffer );
							break;
						case CEtTrigger::Position:
							Struct.vValue = new EtVector3;
							memcpy( Struct.vValue, szBuffer, nSize );
							_ASSERT( nSize == sizeof(EtVector3));
							break;
					}
					m_VecParamList.push_back( Struct );
				}
			}
			break;
		case CustomScript:
			ReadStdString( m_szCustomScript, pStream );
			break;
	}
	
#ifdef PRE_FIX_62845
	return Initialize();
#else
	Initialize();
	return true;
#endif
	
}

void CEtTriggerElement::RegisterLuaApi()
{
	lua_tinker::class_add<EtVector3>( m_pLuaState, "EtVector3" );
	lua_tinker::class_mem<EtVector3>( m_pLuaState, "x", &EtVector3::x );
	lua_tinker::class_mem<EtVector3>( m_pLuaState, "y", &EtVector3::y );
	lua_tinker::class_mem<EtVector3>( m_pLuaState, "z", &EtVector3::z );

	m_pTriggerObj->GetTrigger()->RegisterDefineValue( m_pLuaState );
}

bool CEtTriggerElement::Initialize()
{
	char *pLuaBuffer = NULL;
	int nSize = 0;

	m_szFuncName = s_szFuncString[m_Type];
	switch( m_ScriptType ) {
		case ScriptFile:
			{
				CResMngStream Stream( m_szScriptName.c_str() );
				if( Stream.IsValid() == false ) return false;

				nSize = Stream.Size();
				pLuaBuffer = new char[nSize + 1];
				Stream.Read( pLuaBuffer, nSize );
				pLuaBuffer[nSize] = 0;

				if( !m_bSelfOpenLua ) {
					char szExt[32] = { 0, };
					char szFileName[128] = { 0, };

					_GetExt( szExt, _countof(szExt), m_szScriptName.c_str() );
					if( _stricmp( szExt, "lua" ) == NULL ) {
						_GetFileName( szFileName, _countof(szFileName), m_szScriptName.c_str() );
						szFileName[strlen(szFileName)] = '_';

						std::string szTemp = pLuaBuffer;

						char szFuncName[2][64] = { 0, };
						sprintf_s( szFuncName[0], "function %s", s_szFuncString[m_Type] );
						sprintf_s( szFuncName[1], "function %s%s", szFileName, s_szFuncString[m_Type] );

						int nValue = 0;
						while(1) {
							std::string::size_type nOffset = szTemp.find(szFuncName[0], nValue);
							if( nOffset < szTemp.size() ) {
								szTemp.replace( nOffset, strlen(szFuncName[0]), szFuncName[1] );
								nValue = (int)nOffset + (int)strlen(szFuncName[1]);
							}
							else break;
						}

						SAFE_DELETEA( pLuaBuffer );
						pLuaBuffer = new char[szTemp.size()+1];
						memcpy( pLuaBuffer, szTemp.c_str(), szTemp.size() );
						pLuaBuffer[szTemp.size()] = 0;
						nSize = (int)szTemp.size();

						m_szFuncName = szFileName;
						m_szFuncName += s_szFuncString[m_Type];
					}
				}
			}
			break;
		case CustomScript:
			{
				nSize = (int)m_szCustomScript.size() * sizeof(TCHAR);
				pLuaBuffer = (char*)m_szCustomScript.c_str();
			}
			break;
	}

	if( m_bSelfOpenLua ) {
		m_pLuaState = lua_open();
		luaL_openlibs(m_pLuaState);
	}
	int nResult = lua_tinker::dobuffer( m_pLuaState, pLuaBuffer, nSize );

	if( m_ScriptType == ScriptFile ) {
		SAFE_DELETEA( pLuaBuffer );
	}
	if( nResult != 0 ) {
		if( m_bSelfOpenLua ) lua_close( m_pLuaState );
		m_pLuaState = NULL;
		return false;
	}

	if( m_pLuaState ) RegisterLuaApi();

	return true;
}

bool CEtTriggerElement::CallLuaFunction()
{
	if( m_pLuaState == NULL ) return false;
	lua_pushcclosure(m_pLuaState, lua_tinker::on_error, 0);
	int errfunc = lua_gettop(m_pLuaState);

	lua_pushstring( m_pLuaState, m_szFuncName.c_str() );

	lua_gettable(m_pLuaState, LUA_GLOBALSINDEX);
	if(lua_isfunction(m_pLuaState,-1))
	{
		for( DWORD i=0; i<m_VecParamList.size(); i++ ) {
			switch( m_VecParamList[i].Type ) {
				case CEtTrigger::Integer:
				case CEtTrigger::Operator:
				case CEtTrigger::Prop:
				case CEtTrigger::EventArea:
				case CEtTrigger::Value:
					lua_tinker::push( m_pLuaState, m_VecParamList[i].nValue );
					break;
				case CEtTrigger::Float:
					lua_tinker::push( m_pLuaState, m_VecParamList[i].fValue );
					break;
				case CEtTrigger::String:
					lua_tinker::push( m_pLuaState, m_VecParamList[i].szValue );
					break;
				case CEtTrigger::Position:
					lua_tinker::push( m_pLuaState, *m_VecParamList[i].vValue );
					break;
			}
		}

		int nRet = lua_pcall(m_pLuaState, (int)m_VecParamList.size(), 1, errfunc);
		if( nRet != 0 ) {
			if( nRet == LUA_ERRRUN ) {
				OutputDebug( "Looks like there is a Lua syntax error!! - %s %s\n", m_szScriptName.c_str(), m_szFuncName.c_str() );

				return true;
			}
		}
	}
	else
	{
		lua_tinker::print_error(m_pLuaState, "lua_tinker::call() attempt to call global `%s' `%s' (not a function)", m_szFuncName.c_str(), s_szFuncString[m_Type] );
	}

//	lua_remove(m_pLuaState, -1);
	lua_remove(m_pLuaState, -2);

	return lua_tinker::pop<bool>(m_pLuaState);
}

CEtTriggerElement::ParamStruct *CEtTriggerElement::GetParameterStruct( DWORD dwIndex )
{
	if( dwIndex >= m_VecParamList.size() ) return NULL;
	return &m_VecParamList[dwIndex];
}