#include "HG_Lua.h"
#include "StaticData.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

//빌드후 이벤트 해당 위치 파일을 outputPath로 복사해 놓는다. -> 파일 이름 변경 까지 해보자.
//copy "$(ProjectDir)Text.txt" "$(OutputPath)"


#include "windows.h"
#include <tchar.h>

#define LUA_GLUE_  extern "C" 
char g_CharName[256] = "";

TCHAR LUA_Version[] = L"5.1.4";


///////////////////////////////////////////////////////////////////////////////
//
//전역 변수들
//
///////////////////////////////////////////////////////////////////////////////

HANDLE g_hScreen;


lua_State* g_pLuaState = NULL;

#define LUA_FILE_  "Start.lua"			
#define LUA_FILE1_ "/luaFiles/Start.lua"
#define LUA_FILE2_ "./luaFiles/Start.lua"


TCHAR* g_pBackBuffer = NULL;			//백버퍼
TCHAR* g_pBackGround = NULL;			//배경화면

struct SCREEN_SIZE
{
	INT width;
	INT height;
};

SCREEN_SIZE g_ScreenSize = { 120, 40 };


COORD pos = {};


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


LUA_GLUE_ int _lg_SetScreenSize(lua_State* ls)
{
	DWORD size = g_ScreenSize.width * g_ScreenSize.height;
	g_pBackBuffer = new TCHAR[size];


	//백버퍼 초기화
	g_pBackGround = new TCHAR[size];
	for (DWORD i = 0; i < size; i++)
	{
		g_pBackGround[i] = L'.';
	}


	printf("SetScreenSize\n");
	OutputDebugStringW(L"LUA_GLUE_ SetScreenSize");


	return 0;					//문제 없을시 0 리턴; 왜냐면 그것이 약속이니까
}


LUA_GLUE_ int _lg_ClearSCN(lua_State* ls)
{
	DWORD size = g_ScreenSize.width * g_ScreenSize.height;
	memcpy(g_pBackBuffer, g_pBackGround, sizeof(TCHAR) * size);

	printf("Clear Screen\n");

	return 0;
}


LUA_GLUE_ int _lg_Blt(lua_State* ls)
{
	printf("Blt\n\n");
	DWORD size = g_ScreenSize.width * g_ScreenSize.height;

	DWORD length = 0;
	WriteConsoleOutputCharacter(g_hScreen, g_pBackBuffer, size, pos, &length);

	return 0;
}

LUA_GLUE_ int _lg_DrawBasicTalkBox(lua_State* ls)
{
	DWORD x = (DWORD)lua_tonumber(ls, 1);	//1번 파라미터.얻기 
	DWORD y = (DWORD)lua_tonumber(ls, 2);	//2번 파라미터.얻기 

	DWORD scnWidth = g_ScreenSize.width;
	DWORD scnHeight = g_ScreenSize.height;

	STATIC_DATA* pData = &basic_TalkBox;

	x = max(x, 0);
	y = max(y, 0);

	x = min(x, scnWidth - 1);
	y = min(y, scnHeight - 1);

	COORD dest = { x, y };

	for (int i = 0; i < pData->height; i++)
	{
		for (int j = 0; j < pData->width; j++)
		{
			g_pBackBuffer[x + j + scnWidth * (i + y)] = pData->pData[j + pData->width * i];
		}
	}

	return 0;
};

//크기 설정할 수 있게.. 수정
LUA_GLUE_ int _lg_CreateBackBuffer(lua_State* ls)
{
	//백버퍼 생성.
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
	{"SetSCNSize",		_lg_SetScreenSize },
	{"ClearScreen",     _lg_ClearSCN},
	{"Blt",				_lg_Blt },
	{"DrawBasicTalkBox",_lg_DrawBasicTalkBox},

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
	bool bResult = TRUE;

	//TCHAR buff[256];
	//ZeroMemory(buff, sizeof(TCHAR) * 256);
	//::GetCurrentDirectory(256 ,buff);
	//_tprintf(buff);
	//printf("\n");

	//char buff[256];
	//ZeroMemory(buff, sizeof(char) * 256);
	//::GetCurrentDirectoryA(256, buff);
	//printf(buff);


	//luaL_dofile 오류가 없으면 0 있으면 1 리턴.
	if (luaL_dofile(g_pLuaState, LUA_FILE2_) != 0)
	{
		Lua_Error(g_pLuaState, "%s", lua_tostring(g_pLuaState, -1));
		bResult = FALSE;
	}

	return bResult;
}



////////////////////////////////////////////////////////////////////////////////
//
//							DLL
//
///////////////////////////////////////////////////////////////////////////////
//
// 루아 초기화 함수 : Lua.5.1.4 기준. 2020.07.26 제작 -> 추후 변경 : dll만 별도 업데이트 테스트하기위해 기존 버젼 유지.
//	
///////////////////////////////////////////////////////////////////////////////

LUA_API int Lua_Init(HANDLE hScreen)
{
	int bResult = TRUE;
	g_hScreen = hScreen;					//그릴 대상이 되는 타겟 핸들 셋팅
	if (!g_hScreen)
	{
		//스크린 핸들 획득 실패
		printf("Lua_init : Fail To Get Screen HANDLE");
		bResult = FALSE;
	}

	// 루아 환경을 초기화 합니다.
	//  
	g_pLuaState = luaL_newstate();			//신형 5.1.x 방법.★
	luaL_openlibs(g_pLuaState);

	//printf("Lua Init\n\n");

	// 이 루프는 모든 루아글루 함수를 루아환경에 등록.
	if (Lua_functionRegist() != 0)
	{
		bResult = FALSE;
	}

	
	if (Lua_StartFile())
	{
		bResult = FALSE;

	}
	


	return bResult;
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
