#include "HG_Lua.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

//������ �̺�Ʈ �ش� ��ġ ������ outputPath�� ������ ���´�. -> ���� �̸� ���� ���� �غ���.
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

//�������� - > �ٲܼ� �ֵ��� ���� �� ����.
#define LUA_FILE_  "Start.lua"			




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

LUA_GLUE_ int _lg_Test(lua_State* ls)
{
	
	printf("TestFunction\n\n");
	
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
	{"Test",			_lg_Test },

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
	//luaL_dofile ������ ������ 0 ������ 1 ����.
	if (luaL_dofile(g_pLuaState, LUA_FILE_) != 0)
	{
		Lua_Error(g_pLuaState, "%s", lua_tostring(g_pLuaState, -1));
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
// ��� �ʱ�ȭ �Լ� : Lua.5.1.4 ����. 2020.07.26 ���� -> ���� ���� : dll�� ���� ������Ʈ �׽�Ʈ�ϱ����� ���� ���� ����.
//
///////////////////////////////////////////////////////////////////////////////

int Lua_Init()
{
	int bResult = TRUE;

	// ��� ȯ���� �ʱ�ȭ �մϴ�.
	//  
	g_pLuaState = luaL_newstate();			//���� 5.1.x ���.��
	luaL_openlibs(g_pLuaState);

	//printf("Lua Init\n\n");

	// �� ������ ��� ��Ʊ۷� �Լ��� ���ȯ�濡 ���.
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
