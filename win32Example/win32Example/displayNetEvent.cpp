#include <windows.h>
#include <fwpmtypes.h>
#include <fwpmu.h>
#include <stdio.h>
#include "displayNetEvent.h"

#pragma comment (lib, "fwpuclnt.lib")

#define EXIT_ON_ERROR(err) if((err) != ERROR_SUCCESS) {goto CLEANUP;}


DWORD InitFilterConditions(
    __in_opt PCWSTR appPath,
    __in_opt const SOCKADDR** localAddr,
    __in_opt UINT8 ipProtocol,
    __in UINT32 numCondsIn,
    __out_ecount_part(numCondsIn, *numCondsOut) FWPM_FILTER_CONDITION0* conds,
    __out UINT32* numCondsOut,
    __deref_out FWP_BYTE_BLOB** appId
)
{
    *numCondsOut = 0;
    return ERROR_SUCCESS;
}


DWORD FindRecentEvents(
    __in HANDLE engine,
    __in_opt PCWSTR appPath,
    __in_opt const SOCKADDR* localAddr,
    __in_opt UINT8 ipProtocol,
    __in UINT32 seconds,
    __deref_out_ecount(*numEvents) FWPM_NET_EVENT0*** events,
    __out UINT32* numEvents
)
{
    DWORD result = ERROR_SUCCESS;
    FWPM_NET_EVENT_ENUM_TEMPLATE0 enumTempl;
    ULARGE_INTEGER ulTime;
    FWPM_FILTER_CONDITION0 conds[4];
    UINT32 numConds;
    FWP_BYTE_BLOB* appBlob = NULL;
    HANDLE enumHandle = NULL;

    memset(&enumTempl, 0, sizeof(enumTempl));

    // Use the current time as the end time of the window.
    GetSystemTimeAsFileTime(&(enumTempl.endTime));

    // Subtract the number of seconds specified by the caller to find the start
    // time.
    ulTime.LowPart = enumTempl.endTime.dwLowDateTime;
    ulTime.HighPart = enumTempl.endTime.dwHighDateTime;
    ulTime.QuadPart -= seconds * 10000000ui64;
    enumTempl.startTime.dwLowDateTime = ulTime.LowPart;
    enumTempl.startTime.dwHighDateTime = ulTime.HighPart;

    result = InitFilterConditions(
        appPath,
        &localAddr,
        ipProtocol,
        ARRAYSIZE(conds),
        conds,
        &numConds,
        &appBlob
    );
    EXIT_ON_ERROR(result);

    enumTempl.numFilterConditions = numConds;
    if (numConds > 0)
    {
        enumTempl.filterCondition = conds;
    }

    result = FwpmNetEventCreateEnumHandle0(
        engine,
        &enumTempl,
        &enumHandle
    );
    EXIT_ON_ERROR(result);

    result = FwpmNetEventEnum0(
        engine,
        enumHandle,
        INFINITE,
        events,
        numEvents
    );
    EXIT_ON_ERROR(result);

CLEANUP:
    FwpmNetEventDestroyEnumHandle0(engine, enumHandle);
    FwpmFreeMemory0((void**)&appBlob);
    return result;
}

DWORD displayNetEvent()
{

    HANDLE engineHandle = 0;
    FWPM_NET_EVENT0** events = NULL, * event;
    UINT32 numEvents = 0, i;
    FILETIME ft;
    SYSTEMTIME st;
    static const char* const types[] =
    {
       "FWPM_NET_EVENT_TYPE_IKEEXT_MM_FAILURE",
       "FWPM_NET_EVENT_TYPE_IKEEXT_QM_FAILURE",
       "FWPM_NET_EVENT_TYPE_IKEEXT_EM_FAILURE",
       "FWPM_NET_EVENT_TYPE_CLASSIFY_DROP",
       "FWPM_NET_EVENT_TYPE_IPSEC_KERNEL_DROP"
    };
    const char* type;

    FWPM_SESSION0 session;
    memset(&session, 0, sizeof(session));
    //セッション中に追加されたオブジェクトは、セッション終了後自動削除される
    session.flags = FWPM_SESSION_FLAG_DYNAMIC;

    DWORD result = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &engineHandle);
    if (ERROR_SUCCESS == result)
    {
        result = FindRecentEvents(
            engineHandle,
            0,
            0,
            0,
            100,
            &events,
            &numEvents
        );
    }

    if (numEvents == 0)
    {
        printf("No events matched.\n");
    }
    else
    {
        printf("Matching events:\n");

        for (i = 0; i < numEvents; ++i)
        {
            event = events[i];

            FileTimeToLocalFileTime(&(event->header.timeStamp), &ft);
            FileTimeToSystemTime(&ft, &st);

            type = (event->type < ARRAYSIZE(types)) ? types[event->type]
                : "<unknown>";

            printf(
                "   %04hu/%02hu/%02hu:%02hu:%02hu:%02hu.%03hu - %s\n",
                st.wYear,
                st.wMonth,
                st.wDay,
                st.wHour,
                st.wMinute,
                st.wSecond,
                st.wMilliseconds,
                type
            );
        }
    }
    return result;
}