// LuaConsole.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <Windows.h>

typedef int (*pFunctionDLL)();
pFunctionDLL Lua_Init = NULL;
pFunctionDLL Lua_Process = NULL;
pFunctionDLL Lua_Release = NULL;

void Load_FunctionsDLL(HINSTANCE hIns);

int main()
{
	HINSTANCE hIns = LoadLibrary(L"DLL/LuaDLL.dll");
	if (!hIns)
	{
		hIns = LoadLibrary(L"LuaDLL.dll");
		//__debugbreak();
	}


	Load_FunctionsDLL(hIns);

	if ( !Lua_Init() )
	{
		//초기화 실패 예외 처리 추가.
		printf("Fail to Init\n\n\n");
	}

	Lua_Process();
	Lua_Release();


	getchar();

}


void Load_FunctionsDLL(HINSTANCE hIns)
{
	Lua_Init = (pFunctionDLL)GetProcAddress(hIns, "Lua_Init");
	if (!Lua_Init)
		__debugbreak();

	Lua_Process = (pFunctionDLL)GetProcAddress(hIns, "Lua_Process");
	if (!Lua_Process)
		__debugbreak();

	Lua_Release = (pFunctionDLL)GetProcAddress(hIns, "Lua_Release");
	if (!Lua_Release)
		__debugbreak();
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
