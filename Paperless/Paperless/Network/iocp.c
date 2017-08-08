#if defined(__BORLANDC__)
#pragma hdrstop
#pragma warn -8019
#endif

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <winsock2.h>
#include <windows.h> 
#include <mswsock.h>
#include <time.h>
#include <process.h>
#include "iocp.h"
#include "PrintMsgDef.h"

#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>

#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
#endif

#ifndef _T
#define _T(x) x
#endif

#pragma comment(lib, "Ws2_32.lib")

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define IOCP_STRNCPY(d, n, s) strcpy_s(d, n, s)
#define IOCP_MEMNCPY(d, dn, s, sn) memcpy_s(d, dn, s, sn)
#else
#define IOCP_STRNCPY(d, n, s) strcpy(d, s)
#define IOCP_MEMNCPY(d, dn, s, sn) memcpy(d, s, sn)
#endif
#define IOCP_WRITE_COMPLETE_CALL

// 常量定义

// 种类
#define IO_KEY_NET_LISTENER   1
#define IO_KEY_NET_ACCEPTOR   2
#define IO_KEY_NET_CONNECTOR  3
#define IO_KEY_NET_UDP        4

// 完成数据类型
#define IO_OP_NET_LISTEN     1
#define IO_OP_NET_ACCEPT     2
#define IO_OP_NET_CONNECT    3
#define IO_OP_READ_REQ       4
#define IO_OP_READ           5
#define IO_OP_WRITE          6


#define IOCP_LAST_ERROR 31

HANDLE g_hIoRes = NULL;

// 辅助函数
void LogTrace(LPCTSTR pszFormat, ...)
{
	va_list pArgs;

	char szMessageBuffer[16380]={0};
	va_start( pArgs, pszFormat );
	_vsntprintf( szMessageBuffer, 16380, pszFormat, pArgs );
	va_end( pArgs );

	OutputDebugString(szMessageBuffer);
}
static unsigned WINAPI WorkerThread(LPVOID);

static unsigned WINAPI CheckDataThread(LPVOID);
static void CheckData(PIO_RESOURCE pIoRes);

static BOOL CreateKeyRes(PIO_RESOURCE pIoRes);
static void DestroyKeyRes(PIO_RESOURCE pIoRes);

static PIO_OP_KEY ApplyFreeIoOpKey(
    PIO_RESOURCE pIoRes,
    BYTE byKeyType,
    PFN_IO_EVENT fnAcceptEvt,
    PFN_IO_EVENT fnCloseEvt,
    PFN_IO_EVENT fnReadEvt,
    PFN_IO_EVENT fnWriteEvt,
    PFN_IO_EVENT fnExceptEvt,
    PVOID lpParam
);
static void ReleaseIoOpKey(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey);
static void CloseIoOpKey(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey);
static void CloseIoHandle(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey);

static PIO_OP_DATA ApplyFreeIoOpData(PIO_RESOURCE pIoRes, DWORD dwDataType);
static void ReleaseIoOpData(PIO_RESOURCE pIoRes, PIO_OP_DATA pOpData);

static PIO_BUF ApplyFreeIoBuf(PIO_RESOURCE pIoRes);
static void ReleaseIoBuf(PIO_RESOURCE pIoRes, PIO_BUF pIoBuf);

static BOOL CreateSocket(PIO_OP_KEY pOpKey, int type);
static BOOL BindSocket(SOCKET s, const char *sBindAddr, WORD wBindPort);
static BOOL SetSocketLinger(SOCKET s, int /*1*/onoff, int /*0*/linger);
static BOOL SetSocketKeepAlive(SOCKET s);
static BOOL IsLostWrite(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey);

static void ChangeIoOpDataType(PIO_RESOURCE pIoRes, PIO_OP_DATA pOpData, DWORD dwType);

static BOOL CorrectIncrementRef(PIO_OP_KEY pOpKey);

static BOOL PostReadRequest(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey);

// Request -3:投递失败,-2:句柄已关闭,-1:资源不足
//    0:投递成功,1:立刻完成
// Completion 0-句柄关闭了,非0成功

static int AcceptRequest(
    PIO_RESOURCE pIoRes,
    PIO_OP_KEY listener,
    PIO_OP_DATA pOpData,
    PIO_BUF pIoBuf
);
static int AcceptCompletion(
    PIO_RESOURCE pIoRes,
    BOOL bCompletion,
    DWORD dwBytesTransferred,
    PIO_OP_KEY listener,
    PIO_OP_DATA acceptor_data
);

static int ReadRequest(
    PIO_RESOURCE pIoRes,
    PIO_OP_KEY pOpKey,
    PIO_OP_DATA pOpData,
    PIO_BUF pIoBuf
);
static int ReadCompletion(
    PIO_RESOURCE pIoRes,
    BOOL bCompletion,
    DWORD dwBytesTransferred,
    PIO_OP_KEY pOpKey,
    PIO_OP_DATA pOpData
);

static int WriteRequest(
    PIO_RESOURCE pIoRes,
    PIO_OP_KEY pOpKey,
    PIO_OP_DATA pOpData,
    PIO_BUF pIoBuf
);
static int WriteCompletion(
    PIO_RESOURCE pIoRes,
    BOOL bCompletion,
    DWORD dwBytesTransferred,
    PIO_OP_KEY pOpKey,
    PIO_OP_DATA pOpData
);


// 实现

HANDLE IOCP_DLL_EXPORT CreateIoResource(
    DWORD dwWorkerThreadCount,
    DWORD dwMaxIoCount,
    DWORD dwMaxBufCountPerIo,
    DWORD dwCheckIoTime
)
{
    WORD wVersion;
    WSADATA wsaData;
    DWORD i;
    PIO_RESOURCE pIoRes;
    SYSTEM_INFO sysInfo;
    // Initialize Windows Socket library
    wVersion = MAKEWORD(2, 2);
    if (WSAStartup(wVersion, &wsaData) != 0)
    {
        goto RET0;
    }
    // Check Windows Socket version
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        goto RET1;
    }
    // Alloc io resource
    pIoRes = (PIO_RESOURCE)HeapAlloc(
                 GetProcessHeap(),
                 HEAP_ZERO_MEMORY,
                 sizeof(IO_RESOURCE));
    if (NULL == pIoRes)
    {
        goto RET1;
    }
    // Create completion port
    pIoRes->completion_port =
        CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (NULL == pIoRes->completion_port)
    {
        goto RET2;
    }
    // Create key resource
    pIoRes->max_key_c = dwMaxIoCount;
    pIoRes->max_buf_c_per_key = dwMaxBufCountPerIo;
    pIoRes->data_or_buf_c = pIoRes->max_key_c
                            * pIoRes->max_buf_c_per_key;
    if (!CreateKeyRes(pIoRes))
    {
        goto RET3;
    }
    // Assign worker thread count value
    if (0 == dwWorkerThreadCount)
    {
        GetSystemInfo(&sysInfo);
        dwWorkerThreadCount = sysInfo.dwNumberOfProcessors * 3;
    }
    // Alloc worker thread handle space
    pIoRes->pthreads =
        (LPHANDLE)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(HANDLE) * (dwWorkerThreadCount + 1));
    if (NULL == pIoRes->pthreads)
    {
        goto RET4;
    }
    // Create worker thread
    for (i = 0; i < dwWorkerThreadCount; i++)
    {
        pIoRes->pthreads[pIoRes->thread_c] =
            (HANDLE)_beginthreadex(NULL, 0, WorkerThread, pIoRes, 0, NULL);
        if (pIoRes->pthreads[pIoRes->thread_c])
        {
            pIoRes->thread_c++;
        }
        else
        {
            break;
        }
    }
    if (0 == pIoRes->thread_c)
    {
        goto RET5;
    }
    if (dwCheckIoTime)
    {
        if (dwCheckIoTime > 0xFFFF)
        {
            pIoRes->check_io_time = 0xFFFF;
        }
        else
        {
            pIoRes->check_io_time = (WORD)dwCheckIoTime;
        }
        pIoRes->pthreads[pIoRes->thread_c] =
        	(HANDLE)_beginthreadex(NULL, 0, CheckDataThread, pIoRes, 0, NULL);
        if (pIoRes->pthreads[pIoRes->thread_c])
        {
            pIoRes->thread_c++;
        }
    }
    return pIoRes;
RET5:
    HeapFree(GetProcessHeap(), 0, pIoRes->pthreads);
RET4:
    DestroyKeyRes(pIoRes);
RET3:
    CloseHandle(pIoRes->completion_port);
RET2:
    HeapFree(GetProcessHeap(), 0, pIoRes);
RET1:
    WSACleanup();
RET0:
    return NULL;
}

unsigned WINAPI WorkerThread(HANDLE hIoRes)
{
    int iRet;
    BOOL bCompletion;
    DWORD dwBytesTransferred;
    LONG lOpType;
    PIO_OP_KEY pOpKey;
    PIO_OP_DATA pOpData;
    LPOVERLAPPED pOverlap;
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    InterlockedIncrement(&pIoRes->real_thread_c);
    while (TRUE)
    {
        pOpKey = NULL;
        dwBytesTransferred = 0;
        bCompletion = GetQueuedCompletionStatus(
                          pIoRes->completion_port,
                          &dwBytesTransferred,
                          (PULONG_PTR)&pOpKey,
                          (LPOVERLAPPED *)&pOverlap,
                          INFINITE);
        if (pOverlap == NULL)
        {
            if (dwBytesTransferred == 0xFFFFFFFF)
            {
                // thread exit
                break;
            }
            else if (dwBytesTransferred == 0xFFFFFFFE)
            {
                CheckData(pIoRes);
                continue;
            }
            else
            {
                // completion port error
                OTHER_ERR_MSG
                break;
            }
        }
        // Io completion
        pOpData = (PIO_OP_DATA)((char*)pOverlap-sizeof(S_LIST_ENTRY));
        lOpType = pOpData->op_type;
        switch (lOpType)
        {
        case IO_OP_NET_LISTEN:
            iRet = 0;
            while (dwBytesTransferred-- && 0 == iRet)
            {
                do
                {
                    // Repeat post accept
                    iRet = AcceptRequest(pIoRes, pOpKey, NULL, NULL);

                } while (1 == iRet);
            }
            ReleaseIoOpData(pIoRes, pOpData);
            break;
        case IO_OP_NET_ACCEPT:
            iRet = AcceptCompletion(
                       pIoRes, bCompletion,
                       dwBytesTransferred, pOpKey, pOpData);
            if (0 != iRet)
            {
                do
                {
                    // Repeat post accept
                    iRet = AcceptRequest(pIoRes, pOpKey, NULL, NULL);

                } while (1 == iRet);
            }
            break;
        case IO_OP_NET_CONNECT:
            break;
        case IO_OP_READ_REQ:
            // Post read
            ChangeIoOpDataType(pIoRes, pOpData, IO_OP_READ);
			ReadRequest(pIoRes, pOpKey, pOpData, pOpData->pio_buf);
			break;
		case IO_OP_READ:
			iRet = ReadCompletion(
				pIoRes, bCompletion,
				dwBytesTransferred, pOpKey, pOpData);
			if (iRet)
			{
				iRet = ReadRequest(pIoRes, pOpKey, pOpData, pOpData->pio_buf);
			}
            break;
        case IO_OP_WRITE:
            iRet = WriteCompletion(
                       pIoRes, bCompletion,
                       dwBytesTransferred, pOpKey, pOpData);
            break;
        }
    }
    InterlockedDecrement(&pIoRes->real_thread_c);
    return 0;
}

BOOL CreateKeyRes(PIO_RESOURCE pIoRes)
{
    LONG i;
    // Alloc io completion key
    INIT_SLIST_HEAD(&pIoRes->op_key_list);
    pIoRes->pop_keys =
        (PIO_OP_KEY)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(IO_OP_KEY) * pIoRes->max_key_c);
    if (NULL == pIoRes->pop_keys)
    {
        goto RET0;
    }
    for (i = 0; i < pIoRes->max_key_c; i++)
    {
        PUSH_SLIST_ENTRY(&pIoRes->op_key_list, (PS_LIST_ENTRY)&pIoRes->pop_keys[i]);
    }
    // Alloc io overlap
    INIT_SLIST_HEAD(&pIoRes->op_data_list);
    pIoRes->pop_datas =
        (PIO_OP_DATA)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(IO_OP_DATA) * pIoRes->data_or_buf_c);
    if (NULL == pIoRes->pop_datas)
    {
        goto RET1;
    }
    for (i = 0; i < pIoRes->data_or_buf_c; i++)
    {
        PUSH_SLIST_ENTRY(&pIoRes->op_data_list, (PS_LIST_ENTRY)&pIoRes->pop_datas[i]);
    }
    // Alloc io buf
    INIT_SLIST_HEAD(&pIoRes->io_buf_list);
    pIoRes->pio_bufs =
        (PIO_BUF)HeapAlloc(
            GetProcessHeap(),
            0,
            sizeof(IO_BUF) * pIoRes->data_or_buf_c);
    if (NULL == pIoRes->pio_bufs)
    {
        goto RET2;
    }
    for (i = 0; i < pIoRes->data_or_buf_c; i++)
    {
        pIoRes->pio_bufs[i].ref_c = 0;
        PUSH_SLIST_ENTRY(&pIoRes->io_buf_list, (PS_LIST_ENTRY)&pIoRes->pio_bufs[i]);
    }
    return TRUE;
RET2:
    FLUSH_SLIST(&pIoRes->op_data_list);
    HeapFree(GetProcessHeap(), 0, pIoRes->pop_datas);
RET1:
    FLUSH_SLIST(&pIoRes->op_key_list);
    HeapFree(GetProcessHeap(), 0, pIoRes->pop_keys);
RET0:
    return FALSE;
}

void DestroyKeyRes(PIO_RESOURCE pIoRes)
{
    FLUSH_SLIST(&pIoRes->io_buf_list);
    RELEASE_SLIST_HEAD(&pIoRes->io_buf_list);
    HeapFree(GetProcessHeap(), 0, pIoRes->pio_bufs);

    FLUSH_SLIST(&pIoRes->op_data_list);
    RELEASE_SLIST_HEAD(&pIoRes->op_data_list);
    HeapFree(GetProcessHeap(), 0, pIoRes->pop_datas);

    FLUSH_SLIST(&pIoRes->op_key_list);
    RELEASE_SLIST_HEAD(&pIoRes->op_key_list);
    HeapFree(GetProcessHeap(), 0, pIoRes->pop_keys);
}

void ReleaseIoOpKey(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey)
{
    LONG ref = pOpKey->ref_c;
    while (ref > 0)
    {
        if (InterlockedCompareExchange(&pOpKey->ref_c, ref-1, ref) == ref)
        {
            break;
        }
        else
        {
            ref = pOpKey->ref_c;
        }
    }
    if (0 == ref-1)
    {
        // Push list
        PUSH_SLIST_ENTRY(&pIoRes->op_key_list,
                                  &pOpKey->item);
        // Set valid key count
        switch (pOpKey->key_type)
        {
        case IO_KEY_NET_LISTENER:
            InterlockedDecrement(&pIoRes->listener_c);
            break;
        case IO_KEY_NET_ACCEPTOR:
            InterlockedDecrement(&pIoRes->acceptor_c);
            break;
        case IO_KEY_NET_CONNECTOR:
            InterlockedDecrement(&pIoRes->connector_c);
            break;
        case IO_KEY_NET_UDP:
            InterlockedDecrement(&pIoRes->udp_c);
            break;
        }
    }
}

void CloseIoHandle(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey)
{
	int iRet = 0;
    if (2 == InterlockedCompareExchange(&pOpKey->is_not_closed, 1, 2))
    {
        // Set link disable
        pOpKey->is_link = 0;
        // Close
        switch (pOpKey->key_type)
        {
        case IO_KEY_NET_LISTENER:
            ACCEPT_ERR_MSG(pOpKey);
        case IO_KEY_NET_ACCEPTOR:
        case IO_KEY_NET_CONNECTOR:
        case IO_KEY_NET_UDP:
        	if (INVALID_SOCKET != pOpKey->s)
            {
                SOCKET s = pOpKey->s;
				/**
				 *2017-05-27 项目需要新增的功能
				 *关闭端口前最后一次发送数据
				 */
				/////////////////////////////////////////////////////////
		//		char *reStr = "OK";
		//		struct sockaddr_in addr;
		//		addr.sin_addr.s_addr = pOpKey->remote_addr;
		//		iRet = WriteDataEx(g_hIoRes,pOpKey,inet_ntoa(addr.sin_addr),pOpKey->remote_port,reStr,2);
				/////////////////////////////////////////////////////////
                pOpKey->s = INVALID_SOCKET;
				
                closesocket(s);
            }
            break;
        default:
        	if (INVALID_HANDLE_VALUE != pOpKey->h)
            {
                HANDLE h = pOpKey->h;
                pOpKey->h = INVALID_HANDLE_VALUE;
            	CloseHandle(h);
            }
            break;
        }
        // Dec ref
        ReleaseIoOpKey(pIoRes, pOpKey);
    }
}

void CloseIoOpKey(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey)
{
    CloseIoHandle(pIoRes, pOpKey);
    if (1 == InterlockedCompareExchange(&pOpKey->is_not_closed, 0, 1))
    {
        // Call back
        if (pOpKey->is_close_call)
        {
            pOpKey->fnCloseEvt(pOpKey->lpParam, pOpKey, (PCHAR)pOpKey->last_error, 0);
        }
        else if (pOpKey->is_except_call)
        {
            pOpKey->fnExceptEvt(pOpKey->lpParam, pOpKey, (PCHAR)pOpKey->last_error, 0);
        }
        // Dec ref
        ReleaseIoOpKey(pIoRes, pOpKey);
    }
}

PIO_OP_KEY ApplyFreeIoOpKey(
    PIO_RESOURCE pIoRes,
    BYTE byKeyType,
    PFN_IO_EVENT fnAcceptEvt,
    PFN_IO_EVENT fnCloseEvt,
    PFN_IO_EVENT fnReadEvt,
    PFN_IO_EVENT fnWriteEvt,
    PFN_IO_EVENT fnExceptEvt,
    LPVOID lpParam
)
{
    PIO_OP_KEY pOpKey;
    do {
        // Get IO_OP_KEY from list
        pOpKey = (PIO_OP_KEY)POP_SLIST_ENTRY(&pIoRes->op_key_list);
        if (NULL == pOpKey)
        {
            return NULL;
        }
        // Add ref
        if (0 != InterlockedCompareExchange(&pOpKey->ref_c, 1, 0))
        {
            // Error
            InterlockedIncrement(&pIoRes->key_error_c);
            pOpKey = NULL;
        }
    } while (pOpKey == NULL);
    // Set key param
    switch (byKeyType)
    {
    case IO_KEY_NET_LISTENER:
    case IO_KEY_NET_ACCEPTOR:
    case IO_KEY_NET_CONNECTOR:
    case IO_KEY_NET_UDP:
        // Socket
        pOpKey->s = INVALID_SOCKET;
        break;
    default:
        // File
        pOpKey->h = INVALID_HANDLE_VALUE;
        break;
    }
    pOpKey->is_not_closed = 0;
    pOpKey->key_type = byKeyType;
    pOpKey->is_link = 0;
    pOpKey->is_io_check = 0;
    switch (byKeyType)
    {
    case IO_KEY_NET_LISTENER:
    case IO_KEY_NET_ACCEPTOR:
    case IO_KEY_NET_CONNECTOR:
    case IO_KEY_NET_UDP:
        pOpKey->is_limit_read_len = 2;
	//	pOpKey->is_limit_read_len = 0;
        break;
    default:
        pOpKey->is_limit_read_len = 0;
        break;
    }
    pOpKey->is_l_w = 0;
    pOpKey->is_close_call = 0;
    pOpKey->is_except_call = 0;
    pOpKey->io_timeout = 0x7FFF;
    pOpKey->read_cout = 0;
    pOpKey->remote_addr = 0;
    pOpKey->remote_port = 0;
    pOpKey->local_addr = 0;
    pOpKey->local_port = 0;
    pOpKey->io_clock = 0;
    pOpKey->read_clock = 0;
    pOpKey->read_limit = 0;
    pOpKey->last_error = 0;
    pOpKey->fnOpenEvt = fnAcceptEvt;
    pOpKey->fnCloseEvt = fnCloseEvt;
    pOpKey->fnReadEvt = fnReadEvt;
    pOpKey->fnWriteEvt = fnWriteEvt;
    pOpKey->fnExceptEvt = fnExceptEvt;
    pOpKey->lpParam = lpParam;
    pOpKey->lpReadParam = lpParam;
	pOpKey->lpUserParam = 0;
    pOpKey->s_r_c = 0;
    pOpKey->s_w_c = 0;
    pOpKey->l_w_c = 0;
    // Set used key count
    switch (pOpKey->key_type)
    {
    case IO_KEY_NET_LISTENER:
        InterlockedIncrement(&pIoRes->listener_c);
        break;
    case IO_KEY_NET_ACCEPTOR:
        InterlockedIncrement(&pIoRes->acceptor_c);
        break;
    case IO_KEY_NET_CONNECTOR:
        InterlockedIncrement(&pIoRes->connector_c);
        break;
    case IO_KEY_NET_UDP:
        InterlockedIncrement(&pIoRes->udp_c);
        break;
    }
    return pOpKey;
}

void IOCP_DLL_EXPORT DestroyIoResource(HANDLE hIoRes)
{
    LONG i, j, count;
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    // Close Listener
    count = pIoRes->listener_c;
    for (i = 0, j = 0; i < pIoRes->max_key_c && j < count; i++)
    {
        if (IO_KEY_NET_LISTENER == pIoRes->pop_keys[i].key_type)
        {
            CloseIoOpKey(pIoRes, pIoRes->pop_keys + i);
            j++;
        }
    }
    // Close Not Listener
    count = pIoRes->connector_c + pIoRes->acceptor_c + pIoRes->udp_c;
    for (i = 0, j = 0; i < pIoRes->max_key_c && j < count; i++)
    {
        if(IO_KEY_NET_LISTENER != pIoRes->pop_keys[i].key_type)
        {
            CloseIoOpKey(pIoRes, pIoRes->pop_keys + i);
            j++;
        }
    }
    // Stop all io threads
    pIoRes->check_io_time = 0;
    for (i = 0; i < pIoRes->thread_c; i++)
    {
        PostQueuedCompletionStatus(
            pIoRes->completion_port, 0xFFFFFFFF, 0, NULL);
    }
/*
    for (i = 0; i < pIoRes->thread_c; i += MAXIMUM_WAIT_OBJECTS)
    {
        count = pIoRes->thread_c - i;
        if (count > MAXIMUM_WAIT_OBJECTS)
        {
            count = MAXIMUM_WAIT_OBJECTS;
        }
        if (WAIT_FAILED == WaitForMultipleObjects(
            count, pIoRes->pthreads + i, TRUE, INFINITE))
        {
        }
    }
*/
    // wait for all thread exit
    for (; pIoRes->real_thread_c; Sleep(10));
    // Free memory, Close handle
    for (i = 0; i < pIoRes->thread_c; i++)
    {
        if (pIoRes->pthreads[i])
        {
            CloseHandle(pIoRes->pthreads[i]);
            pIoRes->pthreads[i] = NULL;
        }
    }
    HeapFree(GetProcessHeap(), 0, pIoRes->pthreads);
    DestroyKeyRes(pIoRes);
    CloseHandle(pIoRes->completion_port);
    HeapFree(GetProcessHeap(), 0, pIoRes);
    WSACleanup();
}

HANDLE IOCP_DLL_EXPORT CreateTcpListener(
    HANDLE hIoRes,
    const char *sListenAddr,
    WORD wListenPort,
    int iListenCount,
    PFN_IO_EVENT fnAcceptEvt,
    PFN_IO_EVENT fnCloseEvt,
    PFN_IO_EVENT fnReadEvt,
    PFN_IO_EVENT fnWriteEvt,
    PFN_IO_EVENT fnExceptEvt,
    LPVOID lpParam
)
{
    PIO_OP_KEY listener;
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    // Get free IO_OP_KEY for listener
    listener = ApplyFreeIoOpKey(pIoRes,
                                IO_KEY_NET_LISTENER,
                                fnAcceptEvt,
                                fnCloseEvt,
                                fnReadEvt,
                                fnWriteEvt,
                                fnExceptEvt,
                                lpParam
                               );
    if (NULL == listener)
    {
        return NULL;
    }
    // Socket
    if (!CreateSocket(listener, SOCK_STREAM))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 1;
        // Release
        ReleaseIoOpKey(pIoRes, listener);
        return NULL;
    }
    if (!BindSocket(listener->s, sListenAddr, wListenPort))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 2;
        goto RET;
    }
    if (!SetSocketLinger(listener->s, 1, 0))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 3;
        goto RET;
    }
    // Listen
    if (0 != listen(listener->s, iListenCount))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 4;
        goto RET;
    }
    // Bind completion port
    if (NULL == CreateIoCompletionPort(
                listener->h,
                pIoRes->completion_port,
                (ULONG_PTR)listener,
                0))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 5;
        goto RET;
    }
    // Except call enable
    listener->is_except_call = 1;
    return listener;
RET:
    // Close
    CloseIoOpKey(pIoRes, listener);
    return NULL;
}

PIO_OP_DATA ApplyFreeIoOpData(PIO_RESOURCE pIoRes, DWORD dwDataType)
{
    PIO_OP_DATA pOpData;
    // Get IO_OP_DATA_ENTRY
    pOpData = (PIO_OP_DATA)POP_SLIST_ENTRY(&pIoRes->op_data_list);
    if (NULL == pOpData)
    {
        return NULL;
    }
    // Set IO_OP_DATA param
    memset(&pOpData->overlap, 0, sizeof(OVERLAPPED));
    pOpData->op_type = dwDataType;
    pOpData->pop_key = NULL;
    pOpData->pio_buf = NULL;
    // Set used data count
    switch (pOpData->op_type)
    {
    case IO_OP_NET_LISTEN:
        break;
    case IO_OP_NET_ACCEPT:
        InterlockedIncrement(&pIoRes->data_net_accept_c);
        break;
    case IO_OP_NET_CONNECT:
        InterlockedIncrement(&pIoRes->data_net_connect_c);
        break;
    case IO_OP_READ:
        InterlockedIncrement(&pIoRes->data_read_c);
        break;
    case IO_OP_WRITE:
        InterlockedIncrement(&pIoRes->data_write_c);
        break;
    }
    return pOpData;
}

void ReleaseIoOpData(PIO_RESOURCE pIoRes, PIO_OP_DATA pOpData)
{
    DWORD dwDataType = pOpData->op_type;
    pOpData->op_type = 0;
    PUSH_SLIST_ENTRY(&pIoRes->op_data_list,
                              &pOpData->item);
    switch (dwDataType)
    {
    case IO_OP_NET_ACCEPT:
        InterlockedDecrement(&pIoRes->data_net_accept_c);
        break;
    case IO_OP_NET_CONNECT:
        InterlockedDecrement(&pIoRes->data_net_connect_c);
        break;
    case IO_OP_READ:
        InterlockedDecrement(&pIoRes->data_read_c);
        break;
    case IO_OP_WRITE:
        InterlockedDecrement(&pIoRes->data_write_c);
        break;
    }
}

PIO_BUF ApplyFreeIoBuf(PIO_RESOURCE pIoRes)
{
    PIO_BUF pIoBuf;
    pIoBuf = (PIO_BUF)POP_SLIST_ENTRY(&pIoRes->io_buf_list);
    if (pIoBuf)
    {
        InterlockedIncrement(&pIoBuf->ref_c);
        pIoBuf->wsa_buf.len = MAX_IOCP_BUF_LEN;
        pIoBuf->wsa_buf.buf = pIoBuf->buf;
        pIoBuf->addr_len = sizeof(struct sockaddr);
        memset(&pIoBuf->addr, 0, sizeof(struct sockaddr));
        pIoBuf->buf_len = 0;
    }
    return pIoBuf;
}

void ReleaseIoBuf(PIO_RESOURCE pIoRes, PIO_BUF pIoBuf)
{
    if (0 == InterlockedDecrement(&pIoBuf->ref_c))
    {
        PUSH_SLIST_ENTRY(&pIoRes->io_buf_list, &pIoBuf->item);
    }
}

BOOL CorrectIncrementRef(PIO_OP_KEY pOpKey)
{
    LONG ref;
    while ((ref = pOpKey->ref_c) > 1 && pOpKey->is_not_closed == 2)
    {
        if (InterlockedCompareExchange(&pOpKey->ref_c, ref+1, ref) == ref)
        {
            return TRUE;
        }
    }
    return FALSE;
}

int AcceptRequest(
    PIO_RESOURCE pIoRes,
    PIO_OP_KEY listener,
    PIO_OP_DATA acceptor_data,
    PIO_BUF acceptor_buf
)
{
    int iRet;
    BOOL bRet;
    DWORD dwRet;
    PIO_OP_KEY acceptor;
    DWORD dwBytesTransferred;
    iRet = -1;
    // Get free IO_OP_KEY for acceptor
    acceptor = ApplyFreeIoOpKey(pIoRes,
                                IO_KEY_NET_ACCEPTOR,
                                listener->fnOpenEvt,
                                listener->fnCloseEvt,
                                listener->fnReadEvt,
                                listener->fnWriteEvt,
                                listener->fnExceptEvt,
                                listener->lpParam
                               );
    if (NULL == acceptor)
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 6;
        goto RET0;
    }
    // Socket
    if (!CreateSocket(acceptor, SOCK_STREAM))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 7;
        ReleaseIoOpKey(pIoRes, acceptor);
        goto RET0;
    }    
    // Apply free IO_BUF for acceptor
    acceptor_buf = ApplyFreeIoBuf(pIoRes);
    if (NULL == acceptor_buf)
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 8;
        goto RET1;
    }
    // Get free IO_OP_DATA for acceptor
    acceptor_data = ApplyFreeIoOpData(pIoRes, IO_OP_NET_ACCEPT);
    if (NULL == acceptor_data)
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 9;
        goto RET2;
    }
    // Post accept
    if (!CorrectIncrementRef(listener))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 10;
        goto RET3;
    }
    acceptor_data->pop_key = acceptor;
    acceptor_data->pio_buf = acceptor_buf;
    bRet = AcceptEx(
               listener->s,
               acceptor->s,
               acceptor_buf->buf,
               0,
               sizeof(struct sockaddr_in) + 16,
               sizeof(struct sockaddr_in) + 16,
               &dwBytesTransferred,
               &acceptor_data->overlap
           );
    if (bRet)
    {
        iRet = 1;
        // Accept completion
        AcceptCompletion(pIoRes, TRUE, dwBytesTransferred, listener, acceptor_data);
    }
    else
    {
        dwRet = WSAGetLastError();
        if (dwRet == ERROR_IO_PENDING)
        {
            // Accept pending
            iRet = 0;
        }
        else if (dwRet == WSAENETDOWN || dwRet == WSAENOTSOCK)
        {
            ACCEPT_ERR_MSG(listener); listener->last_error = 11;
            iRet = -3;
            goto RET5;
        }
        else
        {
            // Connect handle close
            ACCEPT_ERR_MSG(listener); listener->last_error = 12;
            iRet = 1;
            goto RET4;
        }
    }
    return iRet;
RET5:
    // Close listener
    CloseIoOpKey(pIoRes, listener);
RET4:
    // Dec listener ref
    ReleaseIoOpKey(pIoRes, listener);
RET3:
    ReleaseIoOpData(pIoRes, acceptor_data);
RET2:
    ReleaseIoBuf(pIoRes, acceptor_buf);
RET1:
    CloseIoOpKey(pIoRes, acceptor);
RET0:
    if (iRet != -3)
        InterlockedIncrement((LONG*)&listener->l_w_c);
    return iRet;
}

int AcceptCompletion(
    PIO_RESOURCE pIoRes,
    BOOL bCompletion,
    DWORD dwBytesTransferred,
    PIO_OP_KEY listener,
    PIO_OP_DATA acceptor_data
)
{
    int iRet, timeout;
    int addr_len;
    struct sockaddr_in addr;
    PIO_OP_KEY acceptor = acceptor_data->pop_key;
    PIO_BUF acceptor_buf = acceptor_data->pio_buf;
    if (!bCompletion)
    {
        if (listener->is_not_closed == 2)
        {
            ACCEPT_ERR_MSG(listener); listener->last_error = 13;
            iRet = -1;
            goto RET0;
        }
        else
        {
            ACCEPT_ERR_MSG(listener); listener->last_error = 14;
            iRet = 0;
            goto RET1;
        }
    }
    // Is enough of key res
    if (QUERY_SLIST(&pIoRes->op_key_list) == 0)
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 15;
        iRet = -2;
        goto RET0;
    }
    // Set accept socket
    if (0 != setsockopt(acceptor->s, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
            (char*)&(listener->s), sizeof(listener->s)))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 16;
        iRet = -3;
        goto RET0;
    }
    timeout = 5000;
    if (0 != setsockopt(acceptor->s, SOL_SOCKET, SO_RCVTIMEO,
            (char *)&timeout, sizeof(timeout)))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 17;
        iRet = -4;
        goto RET0;
    }
    // Bind completion port
    if (NULL == CreateIoCompletionPort(
                acceptor->h,
                pIoRes->completion_port,
                (ULONG_PTR)acceptor, 0))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 18;
        iRet = -5;
        goto RET0;
    }
    // Set listener clock
    listener->io_clock = clock();
    // Add success count
    InterlockedIncrement((LONG*)&listener->s_r_c);
    // Set acceptor clock
    acceptor->io_clock = clock();
    // Close call back enable
    acceptor->is_close_call = 1;
    // Set link enable
    acceptor->is_link = 1;
    // remote addr
    addr_len = sizeof(struct sockaddr_in);
    if (0 == getpeername(acceptor->s, (struct sockaddr *)&addr, &addr_len))
    {
        acceptor->remote_addr = addr.sin_addr.s_addr;
        acceptor->remote_port = addr.sin_port;
    }
    if (0 == getsockname(acceptor->s, (struct sockaddr *)&addr, &addr_len))
    {
        acceptor->local_addr = addr.sin_addr.s_addr;
        acceptor->local_port = addr.sin_port;
    }
    // Call back
    acceptor->fnOpenEvt(acceptor->lpParam, acceptor, (PCHAR)listener->ref_c, 0);
    // Change op data type
    ChangeIoOpDataType(pIoRes, acceptor_data, IO_OP_READ);
    // Post receive
        // Inc ref
    if (!CorrectIncrementRef(acceptor))
    {
        READ_ERR_MSG acceptor->last_error = 19;
        goto RET0;
    }
    ReadRequest(pIoRes, acceptor, acceptor_data, acceptor_buf);
    // Dec ref
    ReleaseIoOpKey(pIoRes, listener);
    return 1;
RET1:
    // Close listener
    CloseIoOpKey(pIoRes, listener);
RET0:
    if (iRet != 0)
        InterlockedIncrement((LONG*)&listener->l_w_c);
    // Dec ref
    ReleaseIoOpKey(pIoRes, listener);
    // Close acceptor
    CloseIoOpKey(pIoRes, acceptor);
    ReleaseIoBuf(pIoRes, acceptor_buf);
    ReleaseIoOpData(pIoRes, acceptor_data);
    return iRet;
}

BOOL CreateSocket(PIO_OP_KEY pOpKey, int type)
{
    LONG ref = pOpKey->ref_c;
    BOOL bReuseAddr = TRUE;
    // Socket
    pOpKey->s = WSASocket(AF_INET,type,0,NULL,0,WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == pOpKey->s)
    {
        return FALSE;
    }
    // Inc ref
    while (1 == ref)
    {
        if (InterlockedCompareExchange(&pOpKey->ref_c, ref+1, ref) == ref)
        {
            // Set not closed
            InterlockedExchange(&pOpKey->is_not_closed, 2);
            break;
        }
        else
        {
            ref = pOpKey->ref_c;
        }
    }
    if (1 != ref)
    {
        closesocket(pOpKey->s);
        pOpKey->s = INVALID_SOCKET;
        return FALSE;
    }
    return TRUE;
}

BOOL SetSocketLinger(SOCKET s, int /*1*/onoff, int /*0*/linger)
{
    // Set socket opt
    int iRet;
    struct linger l;
    l.l_onoff = onoff;
    l.l_linger = linger;
    iRet = setsockopt(s, SOL_SOCKET, SO_LINGER, (const char*)&l, sizeof(l));
    return SOCKET_ERROR != iRet;
}
BOOL SetSocketKeepAlive(SOCKET s)
{
	int optint = 1;
	int iRet = setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char*)&optint, sizeof(optint));
	return SOCKET_ERROR != iRet;
}
BOOL BindSocket(SOCKET s, const char *sBindAddr, WORD wBindPort)
{
    // Bind
    int iRet;
    struct sockaddr_in local;
    memset(&local, 0, sizeof(struct sockaddr_in));
    local.sin_family = AF_INET;
    local.sin_port = htons(wBindPort);
    if (sBindAddr)
    {
        local.sin_addr.S_un.S_addr = inet_addr(sBindAddr);
        if (INADDR_NONE == local.sin_addr.S_un.S_addr)
        {
            return FALSE;
        }
    }
    iRet = bind(s, (struct sockaddr *)&local, sizeof(struct sockaddr_in));
    return SOCKET_ERROR != iRet;
}

void IOCP_DLL_EXPORT CloseObject(HANDLE hIoRes, HANDLE hObject)
{
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
    if (pOpKey && pOpKey->is_not_closed > 1 && pOpKey->ref_c > 1)
    {
        CloseIoHandle((PIO_RESOURCE)hIoRes, pOpKey);
    }
}

int ReadRequest(
    PIO_RESOURCE pIoRes,
    PIO_OP_KEY pOpKey,
    PIO_OP_DATA pOpData,
    PIO_BUF pIoBuf
)
{
    int iRet;
    DWORD dwReaded = 0;
    DWORD dwFlags = 0;
    // Read
    switch (pOpKey->key_type)
    {
    case IO_KEY_NET_UDP:
        pIoBuf->wsa_buf.len = MAX_IOCP_BUF_LEN - pIoBuf->buf_len;
        pIoBuf->wsa_buf.buf = pIoBuf->buf + pIoBuf->buf_len;
        pIoBuf->addr_len = sizeof(struct sockaddr);
        memset(&pIoBuf->addr, 0, sizeof(struct sockaddr));
        iRet = WSARecvFrom(pOpKey->s,
                           &pIoBuf->wsa_buf,
                           1,
                           &dwReaded,
                           &dwFlags,
                           &pIoBuf->addr,
                           &pIoBuf->addr_len,
                           &pOpData->overlap,
                           NULL);
        if (iRet == 0)
        {
            return 1;
        }
        else if (WSAGetLastError() == WSA_IO_PENDING)
        {
            return 0;
        }
        break;
    default:
        iRet = ReadFile(pOpKey->h,
                        pIoBuf->buf + pIoBuf->buf_len,
                        MAX_IOCP_BUF_LEN - pIoBuf->buf_len,
                        &dwReaded,
                        &pOpData->overlap);
        if (iRet)
        {
            return 1;
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            return 0;
        }
        break;
    }
    READ_ERR_MSG pOpKey->last_error = 20;
    // Close IO_OP_KEY
    CloseIoOpKey(pIoRes, pOpKey);
    // Dec ref
    ReleaseIoOpKey(pIoRes, pOpKey);
    // Release
    ReleaseIoBuf(pIoRes, pIoBuf);
    ReleaseIoOpData(pIoRes, pOpData);
    return -3;
}

int ReadCompletion(
    PIO_RESOURCE pIoRes,
    BOOL bCompletion,
    DWORD dwBytesTransferred,
    PIO_OP_KEY pOpKey,
    PIO_OP_DATA pOpData
)
{
    BOOL bLoop;
    DWORD dwLimitLen, dwPos;
//     clock_t clk;
    PIO_BUF pIoBuf = pOpData->pio_buf;

    // Is success read
    if (0 == dwBytesTransferred)
    {
        READ_ERR_MSG pOpKey->last_error = 21;
        goto RET;
    }
    // Set clock
    pOpKey->io_clock = clock();
    // is forgo remain data
//     clk = pOpKey->io_clock - pOpKey->read_clock;
//     if (clk > 5 * CLOCKS_PER_SEC)
//     {
//         // Loop while enough buf
//         dwPos = pIoBuf->buf_len;
//         // Calc real len
//         pIoBuf->buf_len = dwBytesTransferred;
//     }
//     else
    {
        // Calc real len
        pIoBuf->buf_len += dwBytesTransferred;
        // Loop while enough buf
        dwPos = 0;
    }
    // Set read clock
    pOpKey->read_clock = pOpKey->io_clock;
    // Add succ count
    InterlockedIncrement((LONG*)&pOpKey->s_r_c);
    do
    {
        bLoop = FALSE;
        // Is limit read len
        switch (pOpKey->is_limit_read_len)
        {
        case 0:
            // Call back
			pOpKey->fnReadEvt(pOpKey->lpReadParam,
				pOpKey,
				pIoBuf->buf,
				pIoBuf->buf_len
				);
			pIoBuf->buf_len = 0;
            break;
        case 2:
            if (pIoBuf->buf_len >= sizeof(DWORD))
            {
                dwLimitLen = ntohl(*(DWORD*)(pIoBuf->buf + dwPos));
				if (0 == dwLimitLen) // heartbeat
				{
					pIoBuf->buf_len = 0;
					break;
				}
				else if (dwLimitLen > MAX_IOCP_BUF_LEN)
				{
					READ_ERR_MSG pOpKey->last_error = 22;
					goto RET; // Error
				}
				else if (dwLimitLen > pIoBuf->buf_len)
				{
					break;
				}
				else
				{
					// Call back
					pOpKey->fnReadEvt(pOpKey->lpReadParam,
						pOpKey,
						pIoBuf->buf + dwPos,
						dwLimitLen
						);
					// Remain buf len
					pIoBuf->buf_len -= dwLimitLen;
					// Remain buf addr pos
					dwPos += dwLimitLen;
					// Continue
					if (pIoBuf->buf_len > 0)
					{
						bLoop = TRUE;
					}
				}
            }
            break;
        case 1:
            if (pIoBuf->buf_len >= (DWORD)pOpKey->read_limit)
            {
                // Call back
                pOpKey->fnReadEvt(pOpKey->lpReadParam,
                    pOpKey,
                    pIoBuf->buf + dwPos,
                    pOpKey->read_limit
                    );
                // Remain buf len
                pIoBuf->buf_len -= pOpKey->read_limit;
                // Remain buf addr pos
                dwPos += pOpKey->read_limit;
                // Continue
                if (pIoBuf->buf_len > 0)
                {
                    bLoop = TRUE;
                }
            }
            break;
		case 3:
			if (pIoBuf->buf_len >= sizeof(DWORD))
			{
				IOCP_MEMNCPY(&dwLimitLen, sizeof(DWORD), pIoBuf->buf + dwPos, sizeof(DWORD));
				if (0 == dwLimitLen) // heartbeat
				{
					pIoBuf->buf_len = 0;
					break;
				}
				dwLimitLen += 4;
				if (dwLimitLen > MAX_IOCP_BUF_LEN)
				{
					READ_ERR_MSG pOpKey->last_error = 31;
					goto RET; // Error
				}
				else if (dwLimitLen > pIoBuf->buf_len)
				{
					break;
				}
				else
				{
					// Call back
					pOpKey->fnReadEvt(pOpKey->lpReadParam,
						pOpKey,
						pIoBuf->buf + dwPos,
						dwLimitLen
						);
					// Remain buf len
					pIoBuf->buf_len -= dwLimitLen;
					// Remain buf addr pos
					dwPos += dwLimitLen;
					// Continue
					if (pIoBuf->buf_len > 0)
					{
						bLoop = TRUE;
					}
				}
			}
			break;
		case 4:
			if ((pIoBuf->buf_len % (DWORD)pOpKey->read_limit) == 0)
			{
				// Call back
				pOpKey->fnReadEvt(pOpKey->lpReadParam,
					pOpKey,
					pIoBuf->buf + dwPos,
					pOpKey->read_limit
					);
			}
			pIoBuf->buf_len = 0;
			break;
		case 5:
			if ((DWORD)pOpKey->read_limit > 0)
			{
			}
			else if (pIoBuf->buf_len >= sizeof(DWORD))
			{
				pOpKey->read_limit = (DWORD_PTR)ntohl(*(DWORD*)(pIoBuf->buf + dwPos));
				if (0 == pOpKey->read_limit) // heartbeat
				{
					pIoBuf->buf_len = 0;
					break;
				}
			}
			else
			{
				break;
			}
			if ((DWORD)pOpKey->read_limit > pIoBuf->buf_len && 
				((DWORD)pOpKey->read_limit <= MAX_IOCP_BUF_LEN || pIoBuf->buf_len < MAX_IOCP_BUF_LEN))
			{
				break;
			}
			dwLimitLen = (DWORD)pOpKey->read_limit < pIoBuf->buf_len ? (DWORD)pOpKey->read_limit : pIoBuf->buf_len;
			// Call back
			pOpKey->fnReadEvt(pOpKey->lpReadParam,
				pOpKey,
				pIoBuf->buf + dwPos,
				dwLimitLen
				);
			// Remain buf len
			pIoBuf->buf_len -= dwLimitLen;
			pOpKey->read_limit -= dwLimitLen;
			// Remain buf addr pos
			dwPos += dwLimitLen;
			// Continue
			if (pIoBuf->buf_len > 0)
			{
				bLoop = TRUE;
			}
			break;
        default:
            pIoBuf->buf_len = 0;
            break;
        }
    } while (bLoop);
    // Remain buf
    if (pIoBuf->buf_len > 0 && dwPos > 0)
    {
        memcpy(pIoBuf->buf, pIoBuf->buf + dwPos, pIoBuf->buf_len);
    }
    return 1;
RET:
    // Close and Release IO_OP_KEY
    CloseIoOpKey(pIoRes, pOpKey);
    // Dec ref
    ReleaseIoOpKey(pIoRes, pOpKey);
    // Release
    ReleaseIoBuf(pIoRes, pIoBuf);
    ReleaseIoOpData(pIoRes, pOpData);
    return 0;
}
void IOCP_DLL_EXPORT SetGIoRes(HANDLE IoRes){
	g_hIoRes = IoRes;
}
void IOCP_DLL_EXPORT SetLimitReadLen(
	HANDLE hIoRes, HANDLE hObject, BOOL bLimit)
{
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
    pOpKey->is_limit_read_len = bLimit ? 2 : 0;
}

HANDLE IOCP_DLL_EXPORT CreateTcpConnector(
    HANDLE hIoRes,
    const char *sListenerAddr,
    WORD wListenerPort,
    DWORD dwWaitTime,
    PFN_IO_EVENT fnOpenEvt,
    PFN_IO_EVENT fnCloseEvt,
    PFN_IO_EVENT fnReadEvt,
    PFN_IO_EVENT fnWriteEvt,
    PFN_IO_EVENT fnExceptEvt,
    LPVOID lpParam
)
{
	return CreateTcpConnectorEx(hIoRes,0,sListenerAddr,wListenerPort,dwWaitTime,
    	fnOpenEvt,fnCloseEvt,fnReadEvt,fnWriteEvt,fnExceptEvt,lpParam);
}

HANDLE IOCP_DLL_EXPORT GetFreeBuf(
    HANDLE hIoRes,
    PCHAR sRemoteAddr, WORD wRemotePort,
    PCHAR buf, DWORD len)
{
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    PIO_BUF pIoBuf = ApplyFreeIoBuf(pIoRes);
    if (pIoBuf)
    {
        struct sockaddr_in *addr;
        addr = (struct sockaddr_in *)&pIoBuf->addr;
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = inet_addr(sRemoteAddr);
        addr->sin_port = htons(wRemotePort);
        memset(addr->sin_zero, 0, 8);

        memcpy(pIoBuf->buf, buf, len);
        pIoBuf->buf_len = len;
    }
    return pIoBuf;
}

void IOCP_DLL_EXPORT ReleaseBuf(HANDLE hIoRes, HANDLE hIoBuf)
{
    ReleaseIoBuf((PIO_RESOURCE)hIoRes, (PIO_BUF)hIoBuf);
}

int WriteRequest(
    PIO_RESOURCE pIoRes,
    PIO_OP_KEY pOpKey,
    PIO_OP_DATA pOpData,
    PIO_BUF pIoBuf
)
{
    int iRet;
    DWORD dwNumberOfBytesWritten;
    // Inc key ref
	
    if (!CorrectIncrementRef(pOpKey))
    {
        WRITE_ERR_MSG pOpKey->last_error = 23;
        return -2;
    }
	
    // Get free IO_OP_DATA
    pOpData = ApplyFreeIoOpData(pIoRes, IO_OP_WRITE);
    if (NULL == pOpData)
    {
        WRITE_ERR_MSG pOpKey->last_error = 24;
        iRet = -1;
        goto RET;
    }
    // Set IO_OP_DATA member
    pOpData->pop_key = pOpKey;
    pOpData->pio_buf = pIoBuf;
    // Inc buf ref
    InterlockedIncrement(&pIoBuf->ref_c);
    // Write
    switch (pOpKey->key_type)
    {
    case IO_KEY_NET_UDP:
        pIoBuf->wsa_buf.len = pIoBuf->buf_len;
        pIoBuf->addr_len = sizeof(struct sockaddr);
        iRet = WSASendTo(pOpKey->s,
                         &pIoBuf->wsa_buf,
                         1,
                         &dwNumberOfBytesWritten,
                         0,
                         &pIoBuf->addr,
                         pIoBuf->addr_len,
                         &pOpData->overlap,
                         NULL);
        if (iRet == 0)
        {
            return 1;
        }
        else if (WSAGetLastError() == WSA_IO_PENDING)
        {
            return 0;
        }
        else
        {
            WRITE_ERR_MSG pOpKey->last_error = 25;
            iRet = 2;
            goto UDP_RET;
        }
    default:
        iRet = WriteFile(
                   pOpKey->h,
                   pIoBuf->buf,
                   pIoBuf->buf_len,
                   &dwNumberOfBytesWritten,
                   &pOpData->overlap
               );
        if (iRet)
        {
		//	CloseIoOpKey(pIoRes,pOpKey);
            return 1;
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            return 0;
        }
        break;
    }
    WRITE_ERR_MSG pOpKey->last_error = 26;
    iRet = -3;
UDP_RET:
    ReleaseIoBuf(pIoRes, pIoBuf);
    ReleaseIoOpData(pIoRes, pOpData);
RET:
    ReleaseIoOpKey(pIoRes, pOpKey);
    InterlockedIncrement((LONG*)&pOpKey->l_w_c);
    return iRet;
}

BOOL IsLostWrite(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey)
{
    // Check is need losed
    if (pOpKey->is_l_w)
    {
        if (pOpKey->ref_c < pOpKey->read_cout+5)
        {
            pOpKey->is_l_w = 0;
        }
    }
    else if (pOpKey->ref_c >= pIoRes->max_buf_c_per_key)
    {
        pOpKey->is_l_w = 1;
    }
    return pOpKey->is_l_w;
}

int IOCP_DLL_EXPORT WriteData(HANDLE hIoRes, HANDLE hObject, HANDLE hIoBuf)
{
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)(hObject);
    PIO_BUF pIoBuf = (PIO_BUF)hIoBuf;
    if (NULL == pOpKey || pOpKey->is_not_closed < 2 || pOpKey->ref_c < 2)
    {
        return -2;
    }
    if (IsLostWrite(pIoRes, pOpKey))
    {
        InterlockedIncrement((LONG*)&pOpKey->l_w_c);
        return -1;
    }
    return WriteRequest(pIoRes, pOpKey, NULL, pIoBuf);
}

int IOCP_DLL_EXPORT WriteDataEx(
    HANDLE hIoRes,
    HANDLE hObject,
    PCHAR sRemoteAddr,
    WORD wRemotePort,
    PCHAR buf,
    DWORD len
)
{
//	int result = MessageBox(NULL,TEXT("这是对话框"),TEXT("你好"),MB_ICONINFORMATION|MB_YESNO);
    int iRet;
    PIO_BUF pIoBuf;
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)(hObject);
	
    if (NULL == pOpKey || pOpKey->is_not_closed < 2 || pOpKey->ref_c < 2)
    {
        return -2;
    }
	
    if (IsLostWrite(pIoRes, pOpKey))
    {
        InterlockedIncrement((LONG*)&pOpKey->l_w_c);
        return -1;
    }
    pIoBuf = (PIO_BUF)GetFreeBuf(pIoRes, sRemoteAddr, wRemotePort, buf, len);
    if (NULL == pIoBuf)
    {
        InterlockedIncrement((LONG*)&pOpKey->l_w_c);
        return -1;
    }
    iRet = WriteRequest(pIoRes, pOpKey, NULL, pIoBuf);
    ReleaseBuf(pIoRes, pIoBuf);
	//LogTrace("StreamMedia--  WriteRequest iRet=%d",iRet);
    return iRet;
}

int WriteCompletion(
    PIO_RESOURCE pIoRes,
    BOOL bCompletion,
    DWORD dwBytesTransferred,
    PIO_OP_KEY pOpKey,
    PIO_OP_DATA pOpData
)
{
    int iRet;
    PIO_BUF pIoBuf = pOpData->pio_buf;
    // Is transfer completion
    if (dwBytesTransferred < pIoBuf->buf_len)
    {
        WRITE_ERR_MSG pOpKey->last_error = 27;
        // Add lose count
        InterlockedIncrement((LONG*)&pOpKey->l_w_c);
        if (pOpKey->key_type != IO_KEY_NET_UDP)
        {
            iRet = 0;
           // Close and Release IO_OP_KEY
           CloseIoOpKey(pIoRes, pOpKey);
        }
        else
        {
		//    CloseIoOpKey(pIoRes, pOpKey);
            iRet = 2;
        }
    }
    else
    {
        iRet = 1;
        // Set clock
        pOpKey->io_clock = clock();
        // Add succ count
        InterlockedIncrement((LONG*)&pOpKey->s_w_c);
        // Call back
#ifdef IOCP_WRITE_COMPLETE_CALL
        pOpKey->fnWriteEvt(pOpKey->lpParam,
                           pOpKey,
                           pIoBuf->buf,
                           pIoBuf->buf_len
                          );
#endif
    }
    // Dec ref
    ReleaseIoOpKey(pIoRes, pOpKey);
    // Release
    ReleaseIoBuf(pIoRes, pIoBuf);
    ReleaseIoOpData(pIoRes, pOpData);
    return iRet;
}

void IOCP_DLL_EXPORT TouchObject(HANDLE hIoRes, HANDLE hObject)
{
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)(hObject);
    pOpKey->io_clock = clock();
}

BOOL IOCP_DLL_EXPORT StartListenerAccept(
    HANDLE hIoRes,
    HANDLE hListener,
    LONG lAcceptCountTheTime
)
{
    PIO_OP_DATA pOpData;
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    PIO_OP_KEY listener = (PIO_OP_KEY)hListener;
    // Check listen count
    if (lAcceptCountTheTime > pIoRes->max_buf_c_per_key)
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 28;
        goto RET0;
    }
    lAcceptCountTheTime -= listener->ref_c - 2;
    if (lAcceptCountTheTime + 2 <= listener->ref_c)
    {
        return TRUE;
    }
    // Inc ref
    if (!CorrectIncrementRef(listener))
    {
        ACCEPT_ERR_MSG(listener); listener->last_error = 29;
        goto RET0;
    }
    // Apply IO_OP_DATA
    pOpData = ApplyFreeIoOpData(pIoRes, IO_OP_NET_LISTEN);
    if (NULL == pOpData)
    {
        goto RET1;
    }
    listener->read_cout += (BYTE)lAcceptCountTheTime;
    // Post receive request
    if (!PostQueuedCompletionStatus(pIoRes->completion_port,
                                    lAcceptCountTheTime,
                                    (ULONG_PTR)listener,
                                    &pOpData->overlap
                                   ))
    {
        goto RET2;
    }
    // Dec ref
    ReleaseIoOpKey(pIoRes, listener);
    return TRUE;
RET2:
    ReleaseIoOpData(pIoRes, pOpData);
RET1:
    // Dec ref
    ReleaseIoOpKey(pIoRes, listener);
RET0:
    listener->is_except_call = 0;
    CloseIoOpKey(pIoRes, listener);
    return FALSE;
}

BOOL PostReadRequest(PIO_RESOURCE pIoRes, PIO_OP_KEY pOpKey)
{
    PIO_OP_DATA pOpData;
    PIO_BUF pIoBuf;
    // Inc ref
    if (!CorrectIncrementRef(pOpKey))
    {
        READ_ERR_MSG pOpKey->last_error = 30;
        goto RET0;
    }
    // Apply IO_BUF
    pIoBuf = ApplyFreeIoBuf(pIoRes);
    if (NULL == pIoBuf)
    {
        goto RET1;
    }
    // Apply IO_OP_DATA
    pOpData = ApplyFreeIoOpData(pIoRes, IO_OP_READ_REQ);
    if (NULL == pOpData)
    {
        goto RET2;
    }
    pOpData->pop_key = pOpKey;
    pOpData->pio_buf = pIoBuf;
    // Post receive request
    if (!PostQueuedCompletionStatus(pIoRes->completion_port,
                                    0,
                                    (ULONG_PTR)pOpKey,
                                    &pOpData->overlap
                                   ))
    {
        goto RET3;
    }
    return TRUE;
RET3:
    // Release IO_OP_DATA
    ReleaseIoOpData(pIoRes, pOpData);
RET2:
    // Release IO_BUF
    ReleaseIoBuf(pIoRes, pIoBuf);
RET1:
    // Dec ref
    ReleaseIoOpKey(pIoRes, pOpKey);
RET0:
    pOpKey->is_close_call = 0;
    CloseIoOpKey(pIoRes, pOpKey);
    return FALSE;
}

BOOL IOCP_DLL_EXPORT StartConnectorReceive(
    HANDLE hIoRes,
    HANDLE hConnector
)
{
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hConnector;
    if (0 == pOpKey->read_cout)
    {
        pOpKey->read_cout = 1;
        return PostReadRequest((PIO_RESOURCE)hIoRes, pOpKey);
    }
    return TRUE;
}

BOOL IOCP_DLL_EXPORT GetPeerAddr(
    HANDLE hIoRes, HANDLE hObject, PCHAR ip, PWORD port)
{
    BOOL bRet = FALSE;
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
    if (pOpKey)
    {
        char *sAddr;
#if 1
		struct sockaddr_in addr;
		int addr_len = sizeof(struct sockaddr_in);
		getpeername(pOpKey->s, (struct sockaddr *)&addr, &addr_len);
		sAddr = inet_ntoa(addr.sin_addr);
#else
		struct in_addr addr;
		addr.s_addr = pOpKey->remote_addr;
		sAddr = inet_ntoa(addr);
#endif
        if (sAddr)
            IOCP_STRNCPY(ip, 16, sAddr);
        else
            ip[0] = '\0';
        *port = ntohs(pOpKey->remote_port);
        bRet = TRUE;
    }
    return bRet;
}

void ChangeIoOpDataType(PIO_RESOURCE pIoRes, PIO_OP_DATA pOpData, DWORD dwType)
{
    switch (pOpData->op_type)
    {
    case IO_OP_NET_LISTEN:
        break;
    case IO_OP_NET_ACCEPT:
        InterlockedDecrement(&pIoRes->data_net_accept_c);
        break;
    case IO_OP_NET_CONNECT:
        InterlockedDecrement(&pIoRes->data_net_connect_c);
        break;
    case IO_OP_READ:
        InterlockedDecrement(&pIoRes->data_read_c);
        break;
    case IO_OP_WRITE:
        InterlockedDecrement(&pIoRes->data_write_c);
        break;
    }
    pOpData->op_type = dwType;
    switch (pOpData->op_type)
    {
    case IO_OP_NET_LISTEN:
        break;
    case IO_OP_NET_ACCEPT:
        InterlockedIncrement(&pIoRes->data_net_accept_c);
        break;
    case IO_OP_NET_CONNECT:
        InterlockedIncrement(&pIoRes->data_net_connect_c);
        break;
    case IO_OP_READ:
        InterlockedIncrement(&pIoRes->data_read_c);
        break;
    case IO_OP_WRITE:
        InterlockedIncrement(&pIoRes->data_write_c);
        break;
    }
}

void IOCP_DLL_EXPORT GetIoUsedState(
    HANDLE hIoRes, HANDLE hObject, PIO_USED_STATE pState)
{
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
	PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
    if (pIoRes)
    {
        pState->io_error_c = pIoRes->key_error_c;
        pState->listener_c = pIoRes->listener_c;
        pState->acceptor_c = pIoRes->acceptor_c;
        pState->connector_c = pIoRes->connector_c;
        pState->udp_c = pIoRes->udp_c;
        pState->data_net_accept_c = pIoRes->data_net_accept_c;
        pState->data_net_connect_c = pIoRes->data_net_connect_c;
        pState->data_read_c = pIoRes->data_read_c;
        pState->data_write_c = pIoRes->data_write_c;
        pState->remain_key_c = QUERY_SLIST(&pIoRes->op_key_list);
        pState->remain_data_c = QUERY_SLIST(&pIoRes->op_data_list);
        pState->remain_buf_c = QUERY_SLIST(&pIoRes->io_buf_list);
    }
    if (pOpKey)
    {
        pState->s_r_c = pOpKey->s_r_c;
        pState->s_w_c = pOpKey->s_w_c;
        pState->l_w_c = pOpKey->l_w_c;
        pState->ref_c = pOpKey->ref_c;
    }
}

unsigned WINAPI CheckDataThread(LPVOID hIoRes)
{
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    HANDLE hCompPort = pIoRes->completion_port;
    DWORD dwSleep = pIoRes->check_io_time;
    dwSleep *= 1000;
    while (TRUE)
    {
        Sleep(dwSleep);
        if (!PostQueuedCompletionStatus(hCompPort, 0xFFFFFFFE, 0, NULL))
            break;
    }
    return 0;
}

void CheckData(PIO_RESOURCE pIoRes)
{
    LONG i;
    clock_t clk;
    PIO_OP_KEY pOpKey;
    for (i = 0; i < pIoRes->max_key_c && pIoRes->check_io_time; i++)
    {
        pOpKey = &pIoRes->pop_keys[i];
        if (pOpKey->ref_c < 2 || pOpKey->is_not_closed < 2)
        {
            continue;
        }
        if (0 == pOpKey->is_link || 0 == pOpKey->is_io_check)
        {
            continue;
        }
        clk = (clock() - pOpKey->io_clock) / CLOCKS_PER_SEC;
        if (clk > (clock_t)pOpKey->io_timeout)
        {
            CloseIoOpKey(pIoRes, pOpKey);
        }
    }
}

void IOCP_DLL_EXPORT SetDataTimeOut(HANDLE hIoRes, HANDLE hObject, WORD wIoTimeout)
{
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
    if (pOpKey->is_io_check)
    {
        pOpKey->is_io_check = (0 == wIoTimeout) ? 0 : 1;
        pOpKey->io_timeout = wIoTimeout;
    }
    else
    {
        pOpKey->io_timeout = wIoTimeout;
        pOpKey->is_io_check = (0 == wIoTimeout) ? 0 : 1;
    }
}

BOOL SetUdpSocketConnReset(SOCKET s, BOOL bNewBehavior)
{
    DWORD dwBytesReturned = 0;
    int iRet = WSAIoctl(s, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
    return iRet != SOCKET_ERROR;
}

// 创建UDP服务端对象
HANDLE IOCP_DLL_EXPORT CreateUdpObject(
    HANDLE hIoRes,
    const char *sListenAddr,        // 监听地址，NULL-默认"0.0.0.0"
    WORD wListenPort,               // 监听端口
    PFN_IO_EVENT fnOpenEvt,         // 无效
    PFN_IO_EVENT fnCloseEvt,        // 关闭回调
    PFN_IO_EVENT fnReadEvt,         // 读取回调
    PFN_IO_EVENT fnWriteEvt,        // 定义宏 IOCP_WRITE_COMPLETE_CALL 时有效
    PFN_IO_EVENT fnExceptEvt,       // 无效
    LPVOID lpParam
)
{
    PIO_OP_KEY pOpKey;
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    // Get free IO_OP_KEY for udp
    pOpKey = ApplyFreeIoOpKey(pIoRes,
                              IO_KEY_NET_UDP,
                              fnOpenEvt,
                              fnCloseEvt,
                              fnReadEvt,
                              fnWriteEvt,
                              fnExceptEvt,
                              lpParam
                              );
    if (NULL == pOpKey)
    {
        return NULL;
    }
    // Socket
    if (!CreateSocket(pOpKey, SOCK_DGRAM))
    {
        // Release
        ReleaseIoOpKey(pIoRes, pOpKey);
        return NULL;
    }
    if (!BindSocket(pOpKey->s, sListenAddr, wListenPort))
    {
        goto RET;
    }
    SetUdpSocketConnReset(pOpKey->s, FALSE);
    // Bind completion port
    if (NULL == CreateIoCompletionPort(
                pOpKey->h,
                pIoRes->completion_port,
                (ULONG_PTR)pOpKey,
                0))
    {
        goto RET;
    }
    // Close call enable
    pOpKey->is_close_call = 1;
    return pOpKey;
RET:
    // Close
    CloseIoOpKey(pIoRes, pOpKey);
    return NULL;
}

BOOL IOCP_DLL_EXPORT StartUdpReceive(
    HANDLE hIoRes,
    HANDLE hUdp,
    LONG lRecvCountTheTime      // 同时刻接收数量, >0 && <=dwMaxBufCountPerIo
)
{
    BYTE i;
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hUdp;
    if (lRecvCountTheTime > pIoRes->max_buf_c_per_key)
    {
        goto RET;
    }
    lRecvCountTheTime -= pOpKey->ref_c - 2;
    if (lRecvCountTheTime + 2 <= pOpKey->ref_c)
    {
        return TRUE;
    }
    pOpKey->read_cout += (BYTE)lRecvCountTheTime;
    for (i = 0; i < lRecvCountTheTime; i++)
    {
        if (!PostReadRequest((PIO_RESOURCE)hIoRes, pOpKey))
        {
            return FALSE;
        }
    }
    return TRUE;
RET:
    pOpKey->is_except_call = 0;
    CloseIoOpKey(pIoRes, pOpKey);
    return FALSE;
}

BOOL IOCP_DLL_EXPORT GetUdpPeerAddr(
    HANDLE hIoRes, HANDLE hUdp,
    PCHAR pDataBuf, // 读完成回调的数据参数
    PCHAR ip, PWORD port)
{
    struct sockaddr_in *addr = (struct sockaddr_in *)(
        pDataBuf - sizeof(unsigned int) - sizeof(struct sockaddr));
    IOCP_STRNCPY(ip, 16, inet_ntoa(addr->sin_addr));
    *port = ntohs(addr->sin_port);
    return TRUE;
}

PCHAR IOCP_DLL_EXPORT GetIoBufAddr(HANDLE hIoRes, HANDLE hIoBuf)
{
    PIO_BUF pIoBuf = (PIO_BUF)hIoBuf;
    return pIoBuf->buf;
}

void IOCP_DLL_EXPORT SetIoBufLen(HANDLE hIoRes, HANDLE hIoBuf, DWORD len)
{
    PIO_BUF pIoBuf = (PIO_BUF)hIoBuf;
    pIoBuf->buf_len = len;
}

void IOCP_DLL_EXPORT SetLimitReadLen2(HANDLE hIoRes, HANDLE hObject, LONG nLimit, WORD wLimitLen)
{
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
    pOpKey->is_limit_read_len = (BYTE)nLimit;
    pOpKey->read_limit = wLimitLen;
    if (pOpKey->read_limit > MAX_IOCP_BUF_LEN)
    {
        CloseObject(hIoRes, hObject);
    }
}

void IOCP_DLL_EXPORT SetIoEvent(HANDLE hIoRes, HANDLE hObject, LONG lEvtType, PFN_IO_EVENT pFunc, LPVOID lpParam)
{
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
    switch (lEvtType)
    {
	case 1:
		pOpKey->fnOpenEvt = pFunc;
		pOpKey->lpParam = lpParam;
		break;
	case 2:
		pOpKey->fnCloseEvt = pFunc;
		pOpKey->lpParam = lpParam;
		break;
    case 3:
        pOpKey->fnReadEvt = pFunc;
        pOpKey->lpReadParam = lpParam;
        break;
	case 4:
		pOpKey->fnWriteEvt = pFunc;
		pOpKey->lpParam = lpParam;
		break;
	case 5:
		pOpKey->fnExceptEvt = pFunc;
		pOpKey->lpParam = lpParam;
		break;
    case 6:
        pOpKey->lpParam = lpParam;
        pOpKey->lpReadParam = lpParam;
        break;
	case 7:
		pOpKey->fnOpenEvt = pFunc;
		pOpKey->fnCloseEvt = pFunc;
		pOpKey->fnReadEvt = pFunc;
		pOpKey->fnWriteEvt = pFunc;
		pOpKey->fnExceptEvt = pFunc;
		pOpKey->lpParam = lpParam;
		break;
    }
}

void IOCP_DLL_EXPORT SetErrMsgWnd(HANDLE hIoRes, HWND hWnd, DWORD dwMsg)
{
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    pIoRes->hErrMsgWnd = hWnd;
    pIoRes->dwErrMsgID = dwMsg;
}

BOOL IOCP_DLL_EXPORT GetLocalAddr(
	HANDLE hIoRes, HANDLE hObject, PCHAR ip, PWORD port)
{
    BOOL bRet = FALSE;
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
    if (pOpKey)
    {
        char *sAddr;
        struct in_addr addr;
        addr.s_addr = pOpKey->local_addr;
        sAddr = inet_ntoa(addr);
        if (sAddr)
            IOCP_STRNCPY(ip, 16, sAddr);
        else
            ip[0] = '\0';
        *port = ntohs(pOpKey->local_port);
        bRet = TRUE;
    }
    return bRet;
}

/*
	return CreateTcpConnectorEx(hIoRes,0,sListenerAddr,wListenerPort,dwWaitTime,
    	fnOpenEvt,fnCloseEvt,fnReadEvt,fnWriteEvt,fnExceptEvt,lpParam);
*/
HANDLE IOCP_DLL_EXPORT CreateTcpConnectorEx(
    HANDLE hIoRes,
    WORD wBindPort,             // 本地端口
    const char *sListenerAddr,  // 连接地址
    WORD wListenerPort,         // 连接端口
    DWORD dwWaitTime,           // 连接等待,非0-连接成功后返回
    PFN_IO_EVENT fnOpenEvt,     // 无效
    PFN_IO_EVENT fnCloseEvt,    // 关闭回调
    PFN_IO_EVENT fnReadEvt,     // 读取回调
    PFN_IO_EVENT fnWriteEvt,    // 定义宏 IOCP_WRITE_COMPLETE_CALL 时有效
    PFN_IO_EVENT fnExceptEvt,   // 无效
    LPVOID lpParam
)
{
    int iRet;
    PIO_OP_KEY connector;
    unsigned long ul = 1;
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    // sockaddr
    struct sockaddr_in addr;
    int addr_len = sizeof(struct sockaddr_in);
    memset(&addr, 0, addr_len);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(wListenerPort);
    addr.sin_addr.S_un.S_addr = inet_addr(sListenerAddr);
    if (INADDR_NONE == addr.sin_addr.S_un.S_addr)
    {
        return NULL;
    }
    // Get free IO_OP_KEY for connector
    connector = ApplyFreeIoOpKey(pIoRes,
                                 IO_KEY_NET_CONNECTOR,
                                 fnOpenEvt,
                                 fnCloseEvt,
                                 fnReadEvt,
                                 fnWriteEvt,
                                 fnExceptEvt,
                                 lpParam
                                );
    if (NULL == connector)
    {
        return NULL;
    }
    // Socket
    if (!CreateSocket(connector, SOCK_STREAM))
    {
        // Release
        ReleaseIoOpKey(pIoRes, connector);
        return NULL;
    }
    if (!BindSocket(connector->s, NULL, wBindPort))
    {
        goto RET;
    }
    if (!SetSocketLinger(connector->s, 0, 0))
    {
        goto RET;
    }

    ul = 1;
    ioctlsocket(connector->s, FIONBIO, (unsigned long *)&ul);

    // Connect
    iRet = connect(connector->s, (struct sockaddr *)&addr, addr_len);
    if (SOCKET_ERROR == iRet)
    {
        if (WSAEWOULDBLOCK != WSAGetLastError())
        {
            goto RET;
        }
        if (dwWaitTime)
        {
            fd_set fd;
            struct timeval t;
            FD_ZERO(&fd);
            FD_SET(connector->s, &fd);
            memset(&t, 0, sizeof(struct timeval));
            t.tv_sec = dwWaitTime / 1000;
            t.tv_usec = dwWaitTime % 1000;
            iRet = select(1, NULL, &fd, NULL, &t);
            if (0 == iRet || SOCKET_ERROR == iRet)
            {
                goto RET;
            }
            else
            {
                connector->io_clock = clock();
            }
        }
    }
    // remote and local addr
    connector->remote_addr = addr.sin_addr.s_addr;
    connector->remote_port = addr.sin_port;
    if (0 == getsockname(connector->s, (struct sockaddr *)&addr, &addr_len))
    {
        connector->local_addr = addr.sin_addr.s_addr;
        connector->local_port = addr.sin_port;
    }
    // Bind completion port
    if (NULL == CreateIoCompletionPort(
                connector->h,
                pIoRes->completion_port,
                (ULONG_PTR)connector,
                0))
    {
        goto RET;
    }
    // Close call enable
    connector->is_close_call = 1;
    // Set link enable
    connector->is_link = 1;
    return connector;
RET:
    // Close
    CloseIoOpKey(pIoRes, connector);
    return NULL;
}

void IOCP_DLL_EXPORT SetObjectParam(HANDLE hIoRes, HANDLE hObject, LPVOID lpParam)
{
	PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
	if (pOpKey)
	{
		pOpKey->lpUserParam = lpParam;
	}
}

LPVOID IOCP_DLL_EXPORT GetObjectParam(HANDLE hIoRes, HANDLE hObject)
{
	PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
	if (pOpKey)
	{
		return pOpKey->lpUserParam;
	}
	return 0;
}

int IOCP_DLL_EXPORT IocpSyncRead(HANDLE hIoRes, HANDLE hObject, HANDLE hIoBuf, LONG nMsecOfTimeout)
{
    fd_set fds;
    struct timeval tv;
    DWORD uPos, uNeedRecvLen;
    int iRet, iRecvRet, iSelectRet, iLoop;
	clock_t tPrev;
    PIO_RESOURCE pIoRes = (PIO_RESOURCE)hIoRes;
    PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
    PIO_BUF pIoBuf = (PIO_BUF)hIoBuf;
	switch (pOpKey->is_limit_read_len)
	{
	default:
	case 0:
		uNeedRecvLen = MAX_IOCP_BUF_LEN;
		break;
	case 1:
	case 4:
		uNeedRecvLen = (DWORD)pOpKey->read_limit;
		break;
	case 2:
	case 3:
	case 5:
		uNeedRecvLen = 4;
		break;
	}
	uPos = 0;
	iRet = 0;
	iLoop = 1;
    while (iLoop)
    {
        FD_ZERO(&fds);
        FD_SET((unsigned)pOpKey->s, &fds);
        tv.tv_sec = nMsecOfTimeout / 1000;
        tv.tv_usec = (nMsecOfTimeout % 1000) * 1000;
        tPrev = clock();
		nMsecOfTimeout -= (clock() - tPrev) * 1000 / CLOCKS_PER_SEC;
		if (nMsecOfTimeout <= 0)
		{
			iRet = 0;
			break;
		}
        iSelectRet = select(0, &fds, NULL, NULL, &tv);
        if (iSelectRet <= 0)
        {
			iRet = iSelectRet;
            break;
        }
		if (IO_KEY_NET_UDP == pOpKey->key_type)
		{
			pIoBuf->addr_len = sizeof(struct sockaddr);
			memset(&pIoBuf->addr, 0, sizeof(struct sockaddr));
			iRecvRet = recvfrom(pOpKey->s, pIoBuf->buf + uPos, uNeedRecvLen - uPos, 0, &pIoBuf->addr, &pIoBuf->addr_len);
		}
		else
		{
			iRecvRet = recv(pOpKey->s, pIoBuf->buf + uPos, uNeedRecvLen - uPos, 0);
		}
        if (iRecvRet <= 0)
        {
			iRet = -1;
            break;
        }
        uPos += iRecvRet;
		pIoBuf->buf_len = iRet = uPos;
		switch (pOpKey->is_limit_read_len)
		{
		default:
		case 0:
		case 4:
			iLoop = 0;
			break;
		case 1:
			if (uPos >= (unsigned)pOpKey->read_limit)
			{
				iLoop = 0;
			}
			break;
		case 2:
		case 5:
			if (uPos >= 4)
			{
				uNeedRecvLen = ntohl(*(UINT32*)pIoBuf->buf);
				if (0 == uNeedRecvLen || uNeedRecvLen > MAX_IOCP_BUF_LEN || uPos >= uNeedRecvLen)
				{
					iLoop = 0;
				}
			}
			break;
		case 3:
			if (uPos >= 4)
			{
				IOCP_MEMNCPY(&uNeedRecvLen, 4, pIoBuf->buf, 4);
				if (0 == uNeedRecvLen)
				{
					iLoop = 0;
				}
				uNeedRecvLen += 4;
				if (uNeedRecvLen > MAX_IOCP_BUF_LEN || uPos >= uNeedRecvLen)
				{
					iLoop = 0;
				}
			}
			break;
		}
    }
    return iRet;
}

#ifdef OLD_IOCP

static HANDLE s_hIoRes = NULL;
static HANDLE s_hListener = NULL;
static fNetSDKCallBack s_gCB = NULL;
static DWORD s_dwUser = 0;

static void __stdcall AcceptEvt(LPVOID p, HANDLE h, PCHAR buf, DWORD len)
{
	SetLimitReadLen2(s_hIoRes, h, 1, 684);
	s_gCB((int)h, IO_ACCEPT_COMPLETED, 0, buf, (int)len, s_dwUser);
}

static void __stdcall CloseEvt(LPVOID p, HANDLE h, PCHAR buf, DWORD len)
{
	s_gCB((int)h, IO_NET_EXCEPTION, (int)buf, buf, (int)len, s_dwUser);
}

static void __stdcall ReadEvt(LPVOID p, HANDLE h, PCHAR buf, DWORD len)
{
	s_gCB((int)h, IO_RECV_COMPLETED, 0, buf+4, (int)len-4, s_dwUser);
}

static void __stdcall WriteEvt(LPVOID p, HANDLE h, PCHAR buf, DWORD len)
{
	s_gCB((int)h, IO_SEND_COMPLETED, 0, buf, (int)len, s_dwUser);
}

static void __stdcall ExceptEvt(LPVOID p, HANDLE h, PCHAR buf, DWORD len)
{
	s_gCB((int)h, 0x7F, (int)buf, buf, (int)len, s_dwUser);
}

BOOL __stdcall 	NET_Startup()
{
	if (!s_hIoRes)
        s_hIoRes = CreateIoResource(0, 50, 50, 0);
	return (BOOL)s_hIoRes;
}

BOOL __stdcall	NET_Listen(DWORD dwPort)
{
	if (s_hListener)
		return FALSE;
	if (!s_gCB)
		return FALSE;
	s_hListener = CreateTcpListener(s_hIoRes, NULL, (WORD)dwPort, 5, AcceptEvt, CloseEvt, ReadEvt, WriteEvt, ExceptEvt, NULL);
	if (s_hListener)
	{
		if (!StartListenerAccept(s_hIoRes, s_hListener, 10))
			s_hListener = NULL;
	}
	return (BOOL)s_hListener;
}

BOOL __stdcall	NET_RegisterNetCallBack(fNetSDKCallBack lpFunc,DWORD dwUser)
{
	s_dwUser = dwUser;
    s_gCB = lpFunc;
	return TRUE;
}

BOOL __stdcall  NET_SendPacket(int iClientID,char *pPacket,int iLen)
{
	int iRet;
	HANDLE hIoBuf;
	char *pBuf;
	hIoBuf = GetFreeBuf(s_hIoRes, NULL, 0, "", 0);
	if (!hIoBuf)
		return FALSE;
	pBuf = GetIoBufAddr(s_hIoRes, hIoBuf);
	IOCP_MEMNCPY(pBuf, 4, &iLen, sizeof(int));
	IOCP_MEMNCPY(pBuf+4, MAX_IOCP_BUF_LEN-4, pPacket, iLen);
	SetIoBufLen(s_hIoRes, hIoBuf, iLen+4);
    iRet = WriteData(s_hIoRes, (HANDLE)iClientID, hIoBuf);
	ReleaseBuf(s_hIoRes, hIoBuf);
	return iRet >= 0;
}

BOOL __stdcall 	NET_Connect(char *pClientIP,DWORD dwPort,int *iClientID)
{
	HANDLE hClient;
	if (!s_gCB)
		return FALSE;
	hClient = CreateTcpConnector(s_hIoRes, pClientIP, (WORD)dwPort, 2000, AcceptEvt, CloseEvt, ReadEvt, WriteEvt, ExceptEvt, NULL);
	if (hClient)
	{
		SetLimitReadLen2(s_hIoRes, hClient, 1, 684);
		if (!StartConnectorReceive(s_hIoRes, hClient))
			hClient = NULL;
		*iClientID = (int)hClient;
	}
	return (BOOL)hClient;
}

BOOL __stdcall 	NET_Disconnect(int iClientID)
{
	CloseObject(s_hIoRes, (HANDLE)iClientID);
	return TRUE;
}

BOOL __stdcall	NET_Close()
{
	if (s_hIoRes)
	{
		DestroyIoResource(s_hIoRes);
		s_hIoRes = NULL;
	}
	return TRUE;
}

char* __stdcall NET_GetRemoteIP(int iClientID)
{
	static char sIP[16];
	WORD wPort;
	if (!GetPeerAddr(s_hIoRes, (HANDLE)iClientID, sIP, &wPort))
		sIP[0] = 0;
	return sIP;
}

#endif

