#pragma once

#ifdef LUA_API_EXPORTS
#define LUA_API extern "C" __declspec(dllexport)
#else
#define LUA_API extern "C" __declspec(dllimport)
#endif

LUA_API int Lua_Init();
LUA_API int Lua_Process();
LUA_API void Lua_Release();

LUA_API int Lua_VersionInfo();

/***************** end of file "HG_Lua.h" ************************/	