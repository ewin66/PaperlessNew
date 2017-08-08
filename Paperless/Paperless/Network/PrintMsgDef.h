#ifndef PRINT_MSG_DEF_H
#define PRINT_MSG_DEF_H

// ´íÎóÏûÏ¢·¢ËÍ
// 0x??000000-other;0x00??0000-accept;0x0000??00-read;0x000000??-write
// 0-none;1-PostMessage;2-printf;3-GtWriteTrace
#define PRINT_ERR_MSG 0x00000000

// other
#if ((PRINT_ERR_MSG & 0xFF000000) == 0x01000000) // PostMessage
#define OTHER_ERR_MSG if (pIoRes->hErrMsgWnd) \
    PostMessage(pIoRes->hErrMsgWnd, pIoRes->dwErrMsgID, (WPARAM)__LINE__, (LPARAM)GetLastError());
#elif ((PRINT_ERR_MSG & 0xFF000000) == 0x02000000) // printf
#include <stdio.h>
#define OTHER_ERR_MSG do { SYSTEMTIME st; GetLocalTime(&st); \
    printf("%02d:%02d:%02d.%03d: Err: line %d, code %d, tid = %u\n", \
    st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, \
    __LINE__, WSAGetLastError(), GetCurrentThreadId()); } while(0);
#else
#define OTHER_ERR_MSG
#endif

// accept
#if ((PRINT_ERR_MSG & 0xFF0000) == 0x010000) // PostMessage
#define ACCEPT_ERR_MSG(key) if (pIoRes->hErrMsgWnd) \
    PostMessage(pIoRes->hErrMsgWnd, pIoRes->dwErrMsgID, (WPARAM)__LINE__, (LPARAM)GetLastError());
#elif ((PRINT_ERR_MSG & 0xFF0000) == 0x020000) // printf
#include <stdio.h>
#define ACCEPT_ERR_MSG(key) do { SYSTEMTIME st; GetLocalTime(&st); \
    printf("%02d:%02d:%02d.%03d: Err: line %d, code %d, tid = %u\n", \
    st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, \
    __LINE__, WSAGetLastError(), GetCurrentThreadId()); } while(0);
#elif ((PRINT_ERR_MSG & 0xFF0000) == 0x030000) // GtWriteTrace
extern void GtWriteTrace(const int nLevel, LPCTSTR strFormat, ...);
#define ACCEPT_ERR_MSG(key) GtWriteTrace(10, \
    "IOCPError: Line=%d, ErrCode=%d, Listener ref=%d, h=%u, key=%u", \
    __LINE__, GetLastError(), key->ref_c, key->s, key);
#else
#define ACCEPT_ERR_MSG(key)
#endif

// read
#if ((PRINT_ERR_MSG & 0x00FF00) == 0x000100) // PostMessage
#define READ_ERR_MSG if (pIoRes->hErrMsgWnd) \
    PostMessage(pIoRes->hErrMsgWnd, pIoRes->dwErrMsgID, (WPARAM)__LINE__, (LPARAM)GetLastError());
#elif ((PRINT_ERR_MSG & 0x00FF00) == 0x000200) // printf
#include <stdio.h>
#define READ_ERR_MSG do { SYSTEMTIME st; GetLocalTime(&st); \
    printf("%02d:%02d:%02d.%03d: Err: line %d, code %d, tid = %u\n", \
    st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, \
    __LINE__, WSAGetLastError(), GetCurrentThreadId()); } while(0);
#else
#define READ_ERR_MSG
#endif

// write
#if ((PRINT_ERR_MSG & 0x0000FF) == 0x000001) // PostMessage
#define WRITE_ERR_MSG if (pIoRes->hErrMsgWnd) \
    PostMessage(pIoRes->hErrMsgWnd, pIoRes->dwErrMsgID, (WPARAM)__LINE__, (LPARAM)GetLastError());
#elif ((PRINT_ERR_MSG & 0x0000FF) == 0x000002) // printf
#include <stdio.h>
#define WRITE_ERR_MSG do { SYSTEMTIME st; GetLocalTime(&st); \
    printf("%02d:%02d:%02d.%03d: Err: line %d, code %d, tid = %u\n", \
    st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, \
    __LINE__, WSAGetLastError(), GetCurrentThreadId()); } while(0);
#else
#define WRITE_ERR_MSG
#endif

#endif
