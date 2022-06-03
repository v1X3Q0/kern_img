#include <stdio.h>
#include <Windows.h>

#include <psapi.h>
#include <tchar.h>

#include <localUtil.h>
#include <localUtil_windows.h>

#include "kern_static.h"

int kern_static::ksym_dlsym(const char* newString, size_t* out_address)
{
    int result = -1;
    size_t outAddr = 0;

    outAddr = redlsym((UINT8*)binBegin, newString, FALSE);
    SAFE_BAIL(outAddr == 0);
    pe_vatoraw((uint8_t*)binBegin, outAddr, (void**)&outAddr);
    outAddr = outAddr + binBegin;

    result = 0;
    if (out_address != 0)
    {
        *out_address = outAddr;
    }
fail:
    return result;
}

int kern_static::parseAndGetGlobals()
{
    int result = -1;
    result = 0;
    return result;
}
