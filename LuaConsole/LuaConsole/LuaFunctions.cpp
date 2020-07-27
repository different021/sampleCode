#include <stdio.h>
#include "LuaFunctions.h"

pInitFunc Lua_Init = NULL;
pFunctionDLL Lua_Process = NULL;
pFunctionDLL Lua_Release = NULL;


void Load_FunctionsDLL(HINSTANCE hIns)
{
	Lua_Init = (pInitFunc)GetProcAddress(hIns, "Lua_Init");
	if (!Lua_Init)
	{
		printf("Load Func Lua_Init");
		__debugbreak();
	}
		

	Lua_Process = (pFunctionDLL)GetProcAddress(hIns, "Lua_Process");
	if (!Lua_Process)
	{
		printf("Load Func Lua_Proccess");
		__debugbreak();
	}
		

	Lua_Release = (pFunctionDLL)GetProcAddress(hIns, "Lua_Release");
	if (!Lua_Release)
	{
		printf("Load Func Lua_Release");
		__debugbreak();
	}
		
}
