// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
#include "pch.h"

//메인이 존재하는 별도의 프로세스
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:                        //어떤 프로세스가 나한테 붙었당
    case DLL_THREAD_ATTACH:                         //어떤 쓰레드가 나한테 붙었을 때
    case DLL_THREAD_DETACH:                         //쓰래드가 해제 되었을 때
    case DLL_PROCESS_DETACH:                        //프로세스가 해제 되엇을 때
        break;
    }
    return TRUE;
}

