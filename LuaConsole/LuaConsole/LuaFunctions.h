#pragma once
#include <Windows.h>

typedef int (*pInitFunc)(HANDLE hOutput);
typedef int (*pFunctionDLL)();

extern pInitFunc Lua_Init;
extern pFunctionDLL Lua_Process;
extern pFunctionDLL Lua_Release;



void Load_FunctionsDLL(HINSTANCE hOutput);
