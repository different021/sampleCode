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

#define LUA_FILE_  "Demo.lua"			


///////////////////////////////////////////////////////////////////////////////
//
// ���� ��¿� ��� �Լ�.
//
///////////////////////////////////////////////////////////////////////////////

void Lua_Error(lua_State* L, const char* fmt, ...)
{
	/*
	char text[4096];

	va_list args;
	va_start(args, fmt);
	vsprintf(text, fmt, args);
	va_end(args);

	lua_pushstring(L, "_ALERT");
	lua_gettable(L, LUA_GLOBALSINDEX);
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
	*/
}


///////////////////////////////////////////////////////////////////////////////
//
// ��� �ʱ�ȭ �Լ� : Lua.5.1.4 ����. 2020.07.26 ���� -> ���� ���� : dll�� ���� ������Ʈ �׽�Ʈ�ϱ����� ���� ���� ����.
//
///////////////////////////////////////////////////////////////////////////////

int Lua_Init()
{
	// ��� ȯ���� �ʱ�ȭ �մϴ�.
	//  
	g_pLuaState = luaL_newstate();			//���� 5.1.x ���.��
	luaL_openlibs(g_pLuaState);

	printf("Lua Init\n\n");


	// �� ������ ��� ��Ʊ۷� �Լ��� ���ȯ�濡 ���.
	//
	/*for(int i=0; GlueFunctions[i].name; i++)
	{
		lua_register(g_pLuaState, GlueFunctions[i].name, GlueFunctions[i].func);
	}*/

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
	if (luaL_dofile(g_pLuaState, LUA_FILE_) != 0)
	{
		Lua_Error(g_pLuaState, "%s", lua_tostring(g_pLuaState, -1));	//�����޼��� üũ.			 
	}


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
