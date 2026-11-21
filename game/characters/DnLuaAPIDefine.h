#pragma once

extern const char *g_szLuaAPIList[];

int api_Identity();
void DefineLuaAPI( lua_State *pState );

#define LUA_TINKER_DEF(pLS, pFUNC)	\
	lua_tinker::def(pLS, #pFUNC, &pFUNC); char a##pFUNC = 0; a##pFUNC;
