#include "bhapi.h"

BHAPI::BHAPI()
{
}

#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinst,DWORD a_Reason,LPVOID a_Reserved)
{
    //

    switch(a_Reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            break;
        }
        case DLL_THREAD_ATTACH:
        {
            break;
        }
        case DLL_THREAD_DETACH:
        {
            break;
        }
    }

    /* Return TRUE on success, FALSE on failure */
    return TRUE;
}
