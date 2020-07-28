#include "HG_Lua.h"
#include "StaticData.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

//������ �̺�Ʈ �ش� ��ġ ������ outputPath�� ������ ���´�. -> ���� �̸� ���� ���� �غ���.
//copy "$(ProjectDir)Text.txt" "$(OutputPath)"


#include "windows.h"
#include <tchar.h>

#define LUA_GLUE_  extern "C" 
char g_CharName[256] = "";

TCHAR LUA_Version[] = L"5.1.4";


///////////////////////////////////////////////////////////////////////////////
//
//���� ������
//
///////////////////////////////////////////////////////////////////////////////

HANDLE g_hScreen;


lua_State* g_pLuaState = NULL;

#define LUA_FILE_  "Start.lua"			
#define LUA_FILE1_ "/luaFiles/Start.lua"
#define LUA_FILE2_ "./luaFiles/Start.lua"


TCHAR* g_pBackBuffer = NULL;			//�����
TCHAR* g_pBackGround = NULL;			//���ȭ��

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
	puts("��� ������ v1.0 (Lua v5.1.4)");
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


	//����� �ʱ�ȭ
	g_pBackGround = new TCHAR[size];
	for (DWORD i = 0; i < size; i++)
	{
		g_pBackGround[i] = L'.';
	}


	printf("SetScreenSize\n");
	OutputDebugStringW(L"LUA_GLUE_ SetScreenSize");


	return 0;					//���� ������ 0 ����; �ֳĸ� �װ��� ����̴ϱ�
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
	DWORD x = (DWORD)lua_tonumber(ls, 1);	//1�� �Ķ����.��� 
	DWORD y = (DWORD)lua_tonumber(ls, 2);	//2�� �Ķ����.��� 

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

//ũ�� ������ �� �ְ�.. ����
LUA_GLUE_ int _lg_CreateBackBuffer(lua_State* ls)
{
	//����� ����.
	return 0;
}



//////////////////////////////////////////////
//
//��� ��ũ��Ʈ���� ��� �� �� �ִ� �Լ���.
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
// ���� ��¿� ��� �Լ�.
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

//��� ��ũ��Ʈ���� �Լ��� ��� �� �� �ֵ��� ���
int Lua_functionRegist()
{

	for (int i = 0; GlueFunctions[i].name; i++)
	{
		lua_register(g_pLuaState, GlueFunctions[i].name, GlueFunctions[i].func);
	}

	//���� ������ 0 ��ȯ.
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


	//luaL_dofile ������ ������ 0 ������ 1 ����.
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
// ��� �ʱ�ȭ �Լ� : Lua.5.1.4 ����. 2020.07.26 ���� -> ���� ���� : dll�� ���� ������Ʈ �׽�Ʈ�ϱ����� ���� ���� ����.
//	
///////////////////////////////////////////////////////////////////////////////

LUA_API int Lua_Init(HANDLE hScreen)
{
	int bResult = TRUE;
	g_hScreen = hScreen;					//�׸� ����� �Ǵ� Ÿ�� �ڵ� ����
	if (!g_hScreen)
	{
		//��ũ�� �ڵ� ȹ�� ����
		printf("Lua_init : Fail To Get Screen HANDLE");
		bResult = FALSE;
	}

	// ��� ȯ���� �ʱ�ȭ �մϴ�.
	//  
	g_pLuaState = luaL_newstate();			//���� 5.1.x ���.��
	luaL_openlibs(g_pLuaState);

	//printf("Lua Init\n\n");

	// �� ������ ��� ��Ʊ۷� �Լ��� ���ȯ�濡 ���.
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
	// ��� ���� ����. 
	//
	// ��� ���� ����. 
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

	// ����� ��� ȯ�� ����.
	lua_close(g_pLuaState);

}

int Lua_VersionInfo()
{
	return LUA_VERSION_RELEASE_NUM;
}



/***************** end of file "B3_Lua.cpp" ************************/
