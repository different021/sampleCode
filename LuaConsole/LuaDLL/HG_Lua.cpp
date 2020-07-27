#include "HG_Lua.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

//빌드후 이벤트 해당 위치 파일을 outputPath로 복사해 놓는다. -> 파일 이름 변경 까지 해보자.
//copy "$(ProjectDir)Text.txt" "$(OutputPath)"


#include "windows.h"


#define LUA_GLUE_  extern "C" 
char g_CharName[256] = "";

TCHAR LUA_Version[] = L"5.1.4";


///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

lua_State* g_pLuaState = NULL;

//시작파일 - > 바꿀수 있도록 수정 할 예정.
#define LUA_FILE_  "Start.lua"			




//extern "C" int _lg_Version(lua_State* ls)
LUA_GLUE_ int _lg_Version(lua_State* ls)
{
	puts("--------------------------------------");
	puts("루아 데모엔진 v1.0 (Lua v5.1.4)");
	puts(LUA_VERSION);
	puts(LUA_COPYRIGHT);
	puts(LUA_AUTHORS);
	puts("--------------------------------------");
	return 0;
}

LUA_GLUE_ int _lg_Test(lua_State* ls)
{
	
	printf("TestFunction\n\n");
	
	return 0;
}


//////////////////////////////////////////////
//
//루아 스크립트에서 사용 할 수 있는 함수들.
//
/////////////////////////////////////////////
static luaL_reg GlueFunctions[] =
{
	{"Version",			_lg_Version },
	{"Test",			_lg_Test },

	{NULL,				NULL}
};



///////////////////////////////////////////////////////////////////////////////
//
// 에러 출력용 루아 함수.
//
///////////////////////////////////////////////////////////////////////////////

void Lua_Error(lua_State* L, const char* fmt, ...)
{

	char text[4096];

	va_list args;
	va_start(args, fmt);
	vsprintf(text, fmt, args);
	va_end(args);

	lua_pushstring(L, "_ALERT");
	//lua_gettable(L, LUA_GLOBALSINDEX);
//		if(lua_isfunction(L, -1))
	//{
	//lua_pushstring(L, text);
	//lua_call(L, 1, 0);
	//}
	//else
	//{
	
	MessageBoxA(NULL, text, "Lua::Error", MB_OK);
	//		printf("%s\n", text);
	lua_pop(L, 1);
	//	}
	
}

//루아 스크립트에서 함수를 사용 할 수 있도록 등록
int Lua_functionRegist()
{

	for (int i = 0; GlueFunctions[i].name; i++)
	{
		lua_register(g_pLuaState, GlueFunctions[i].name, GlueFunctions[i].func);
	}
	
	//문제 없으면 0 반환.
	return 0;
}

bool Lua_StartFile()
{
	//luaL_dofile 오류가 없으면 0 있으면 1 리턴.
	if (luaL_dofile(g_pLuaState, LUA_FILE_) != 0)
	{
		Lua_Error(g_pLuaState, "%s", lua_tostring(g_pLuaState, -1));
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
// 루아 초기화 함수 : Lua.5.1.4 기준. 2020.07.26 제작 -> 추후 변경 : dll만 별도 업데이트 테스트하기위해 기존 버젼 유지.
//
///////////////////////////////////////////////////////////////////////////////

int Lua_Init()
{
	int bResult = TRUE;

	// 루아 환경을 초기화 합니다.
	//  
	g_pLuaState = luaL_newstate();			//신형 5.1.x 방법.★
	luaL_openlibs(g_pLuaState);

	//printf("Lua Init\n\n");

	// 이 루프는 모든 루아글루 함수를 루아환경에 등록.
	if (!Lua_functionRegist())
	{
		bResult = FALSE;
	}


	if (!Lua_StartFile())
	{
		bResult = FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int Lua_Process()
{
	// 루아 파일 실행. 
	//
	// 루아 파일 실행. 
	//



	printf("Lua Process\n\n");

	return TRUE;

}



///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void Lua_Release()
{
	printf("Lua Release\n\n");

	// 종료시 루아 환경 삭제.
	lua_close(g_pLuaState);

}

int Lua_VersionInfo()
{
	return LUA_VERSION_RELEASE_NUM;
}



/***************** end of file "B3_Lua.cpp" ************************/
