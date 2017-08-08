#ifndef __IOCP_H__
#define __IOCP_H__

#include <windows.h>
#include "singlelist.h"
#ifdef BUILD_IOCP_DLL
#define IOCP_DLL_EXPORT /*__declspec(dllexport)*/ __stdcall
#else
#define IOCP_DLL_EXPORT /*__declspec(dllimport)*/ __stdcall
#endif

#define MAX_IOCP_BUF_LEN       (32*1024)

typedef void (__stdcall *PFN_IO_EVENT)(LPVOID, HANDLE, PCHAR, DWORD);

// 缓冲区
typedef struct _IO_BUF
{
	union {
		char sUnionStruct[16+MAX_IOCP_BUF_LEN];
		struct {
			S_LIST_ENTRY         item;
			LONG volatile        ref_c;
			WSABUF               wsa_buf;
			int                  addr_len;
			struct sockaddr      addr;
			unsigned int         buf_len;
			char                 buf[MAX_IOCP_BUF_LEN];
		};
	};
} IO_BUF, *PIO_BUF;

// 完成键 套接字状态
typedef struct _IO_OP_KEY
{
	union {
		char sUnionStruct[32];
		struct {
			S_LIST_ENTRY         item;
			union
			{
				SOCKET           s;
				HANDLE           h;
			};
			LONG volatile        ref_c;                 // 引用次数
			LONG volatile        is_not_closed;         // 是否已经关闭
			BYTE                 key_type;              // 种类
			BYTE                 is_link           :1;  // 链接完成
			BYTE                 is_io_check       :1;  // 数据超时检测
			BYTE                 is_l_w            :1;  // 需要放弃写
			BYTE                 is_close_call     :1;  // 启用关闭回调
			BYTE                 is_except_call    :1;  // 启用异常回调
			BYTE                 reserved1         :3;
			BYTE                 read_cout;             // 同时投递读数
			BYTE                 is_limit_read_len;     // 读取长度限制，0-不限制，2-按报文头限制，1-按固定长度限制
			WORD                 io_timeout;            // 超时时间
			WORD                 remote_port;           // 远程端口
			DWORD                remote_addr;           // 远程IP地址
			WORD                 last_error;
			WORD                 local_port;            // 本地端口
			DWORD                local_addr;            // 本地IP地址
			clock_t              io_clock;              // 数据时钟
			clock_t              read_clock;            // 读取数据时钟
			DWORD_PTR            read_limit;            // 读取限制的长度
			PFN_IO_EVENT         fnOpenEvt;             // 接受连接
			PFN_IO_EVENT         fnCloseEvt;            // 关闭,一经调用，此结构不允许再使用
			PFN_IO_EVENT         fnReadEvt;             // 读取
			PFN_IO_EVENT         fnWriteEvt;            // 写入（保留）
			PFN_IO_EVENT         fnExceptEvt;           // 异常
			LPVOID               lpParam;               // 事件参数
			LPVOID               lpReadParam;           // 读取事件参数
			LPVOID               lpUserParam;           // 用户参数，额外
			DWORD volatile       s_r_c;                 // 共成功读次数
			DWORD volatile       s_w_c;                 // 共成功写次数
			DWORD volatile       l_w_c;                 // 共丢弃写次数
		};
	};
} IO_OP_KEY, *PIO_OP_KEY;

// 完成数据
typedef struct _IO_OP_DATA
{
	union {
		char sUnionStruct[16];
		struct {
			S_LIST_ENTRY         item;
			OVERLAPPED           overlap;
			LONG                 op_type;
			PIO_OP_KEY           pop_key;
			PIO_BUF              pio_buf;
		};
	};
} IO_OP_DATA, *PIO_OP_DATA;

// 资源
typedef struct _IO_RESOURCE
{
	HANDLE              completion_port;

	LONG                max_key_c;
	LONG                max_buf_c_per_key;
	LONG                data_or_buf_c;

	S_LIST_HEADER       op_key_list;
	PIO_OP_KEY          pop_keys;

	S_LIST_HEADER       op_data_list;
	PIO_OP_DATA         pop_datas;

	S_LIST_HEADER       io_buf_list;
	PIO_BUF             pio_bufs;

	WORD                check_io_time;
	WORD                thread_c;
	LPHANDLE            pthreads;
	LONG volatile       real_thread_c;

	LONG volatile       key_error_c;
	LONG volatile       listener_c;
	LONG volatile       acceptor_c;
	LONG volatile       connector_c;
	LONG volatile       udp_c;

	LONG volatile       data_net_accept_c;
	LONG volatile       data_net_connect_c;
	LONG volatile       data_read_c;
	LONG volatile       data_write_c;

	HWND                hErrMsgWnd;
	DWORD               dwErrMsgID;

} IO_RESOURCE, *PIO_RESOURCE;

typedef struct
{
    // 全局状态 hIoRes!=NULL 时有效
    LONG                io_error_c;
	LONG                listener_c;
    LONG                acceptor_c;
    LONG                connector_c;
    LONG                udp_c;
    LONG                data_net_accept_c;
    LONG                data_net_connect_c;
    LONG                data_read_c;
    LONG                data_write_c;
    LONG                remain_key_c;
    LONG                remain_data_c;
    LONG                remain_buf_c;
    LONG                reserved1[20];
    // 对象状态 hObject!=NULL 时有效
    DWORD               s_r_c;                 // 共成功读次数
    DWORD               s_w_c;                 // 共成功写次数
    DWORD               l_w_c;                 // 共丢弃写次数
    LONG                ref_c;
    LONG                reserved2[12];
} IO_USED_STATE, *PIO_USED_STATE;

#ifdef __cplusplus
extern "C"
{
#endif

// 返回NULL失败，否则作为以下函数的第一个参数
HANDLE IOCP_DLL_EXPORT CreateIoResource(
    DWORD dwWorkerThreadCount,  // 工作线程数量,0-默认内核数*3
    DWORD dwMaxIoCount,         // 最大IO句柄数量
    DWORD dwMaxBufCountPerIo,   // 每个IO句柄的最大缓冲区个数,至少为3
    DWORD dwCheckIoTime         // 每N秒检测数据，0不检测
);

// 销毁资源，此函数会等待所有工作线程退出
void IOCP_DLL_EXPORT DestroyIoResource(HANDLE hIoRes);

// 创建TCP服务端对象
// 成功返回后必须调用StartListenerAccept
HANDLE IOCP_DLL_EXPORT CreateTcpListener(
    HANDLE hIoRes,
    const char *sListenAddr,    // 监听地址，NULL-默认"0.0.0.0"
    WORD wListenPort,           // 监听端口
    int  iListenCount,          // 监听队列个数，建议最多填5
    PFN_IO_EVENT fnAcceptEvt,   // 接受连接回调
    PFN_IO_EVENT fnCloseEvt,    // 连接断开回调
    PFN_IO_EVENT fnReadEvt,     // 读取回调
    PFN_IO_EVENT fnWriteEvt,    // 定义宏 IOCP_WRITE_COMPLETE_CALL 时有效
    PFN_IO_EVENT fnExceptEvt,   // 监听异常回调
    LPVOID lpParam
);

// 返回FALSE时，此对象自动释放
// 此函数成功返回后才能开始接受连接
BOOL IOCP_DLL_EXPORT StartListenerAccept(
    HANDLE hIoRes,
    HANDLE hListener,
    LONG lAcceptCountTheTime // >0 && <=dwMaxBufCountPerIo
);

// 创建TCP客户端对象
// 成功返回后必须调用StartConnectorReceive
HANDLE IOCP_DLL_EXPORT CreateTcpConnector(
    HANDLE hIoRes,
    const char *sListenerAddr,  // 连接地址
    WORD wListenerPort,         // 连接端口
    DWORD dwWaitTime,           // 连接等待,非0-连接成功后返回
    PFN_IO_EVENT fnOpenEvt,     // 无效
    PFN_IO_EVENT fnCloseEvt,    // 关闭回调
    PFN_IO_EVENT fnReadEvt,     // 读取回调
    PFN_IO_EVENT fnWriteEvt,    // 定义宏 IOCP_WRITE_COMPLETE_CALL 时有效
    PFN_IO_EVENT fnExceptEvt,   // 无效
    LPVOID lpParam
);

// 返回FALSE时，此对象自动释放
// 此函数成功返回后才能开始读写数据
BOOL IOCP_DLL_EXPORT StartConnectorReceive(
    HANDLE hIoRes,
    HANDLE hConnector
);

// BUF 有引用计数，可以多次写数据
// 调用此函数后必需调用释放函数
// len 必须小于等于 MAX_IOCP_BUF_LEN
// UDP报文时:sRemoteAddr与wRemotePort有效
HANDLE IOCP_DLL_EXPORT GetFreeBuf(
    HANDLE hIoRes,
    PCHAR sRemoteAddr, WORD wRemotePort,
    PCHAR buf, DWORD len);

void IOCP_DLL_EXPORT ReleaseBuf(HANDLE hIoRes, HANDLE hIoBuf);

// 写数据函数
//     返回值 -3:投递失败,-2:句柄已关闭,-1:资源不足
//         0:投递成功,1:立刻完成,2:UDP投递失败
//     返回<-1后就不要再写了

// WriteData函数与获取空闲BUF配套使用
// 调用顺序 GetFreeBuf -> WriteData -> ReleaseBuf
int IOCP_DLL_EXPORT WriteData(
    HANDLE hIoRes, HANDLE hObject,
    HANDLE hIoBuf
);

int IOCP_DLL_EXPORT WriteDataEx(
    HANDLE hIoRes, HANDLE hObject,
    PCHAR sRemoteAddr, WORD wRemotePort,
    PCHAR buf, DWORD len
);
//设置全局资源
void IOCP_DLL_EXPORT SetGIoRes(HANDLE IoRes);
// 只能在连接者成功返回、Accept回调或接收数据回调中调用
void IOCP_DLL_EXPORT SetLimitReadLen(
    HANDLE hIoRes, HANDLE hObject, BOOL bLimit);

// 若对象在一定时间内没有访问，将被关闭
// 可用此函数，手动更新对象访问时间
void IOCP_DLL_EXPORT TouchObject(HANDLE hIoRes, HANDLE hObject);

// 对象有效时，随时可以调用
// 异常或关闭回调后对象无效了

void IOCP_DLL_EXPORT SetDataTimeOut(HANDLE hIoRes, HANDLE hObject, WORD wIoTimeout);

void IOCP_DLL_EXPORT CloseObject(HANDLE hIoRes, HANDLE hObject);

BOOL IOCP_DLL_EXPORT GetPeerAddr(
	HANDLE hIoRes, HANDLE hObject, PCHAR ip, PWORD port);

// 获取IO状态
void IOCP_DLL_EXPORT GetIoUsedState(
    HANDLE hIoRes, HANDLE hObject, PIO_USED_STATE);

// 创建UDP服务端对象
HANDLE IOCP_DLL_EXPORT CreateUdpObject(
    HANDLE hIoRes,
    const char *sListenAddr,    // 监听地址，NULL-默认"0.0.0.0"
    WORD wListenPort,           // 监听端口，0-随机
    PFN_IO_EVENT fnOpenEvt,     // 无效
    PFN_IO_EVENT fnCloseEvt,    // 关闭回调
    PFN_IO_EVENT fnReadEvt,     // 读取回调
    PFN_IO_EVENT fnWriteEvt,    // 定义宏 IOCP_WRITE_COMPLETE_CALL 时有效
    PFN_IO_EVENT fnExceptEvt,   // 无效
    LPVOID lpParam
);

// 开始UDP套接口接收数据
BOOL IOCP_DLL_EXPORT StartUdpReceive(
    HANDLE hIoRes,
    HANDLE hUdp,
    LONG lRecvCountTheTime      // 同时刻接收数量, >0 && <=dwMaxBufCountPerIo
);

BOOL IOCP_DLL_EXPORT GetUdpPeerAddr(
    HANDLE hIoRes, HANDLE hUdp,
    PCHAR pDataBuf,             // 读完成回调的数据参数
    PCHAR ip, PWORD port);

// hIoBuf: 为 GetFreeBuf 的返回值
// 获取缓冲区地址
PCHAR IOCP_DLL_EXPORT GetIoBufAddr(HANDLE hIoRes, HANDLE hIoBuf);
// 设置缓冲区长度
void IOCP_DLL_EXPORT SetIoBufLen(HANDLE hIoRes, HANDLE hIoBuf, DWORD len);

// 设置接收固定长度
// nLimit=1 时 dwLimitLen 有效，按长度限制
// nLimit=2 时 按头4个字节的网络序长度限制，长度包括4个字节
// nLimit=3 时 按头4个字节的内存值限制，不包括4个字节
// nLimit=4 时 dwLimitLen 有效，按长度限制，若非此长度丢弃
// nLimit=5 时 按头4个字节的网络序长度限制，长度包括4个字节，若一个包超出此长度，回调多次，直到一个包回调完毕
void IOCP_DLL_EXPORT SetLimitReadLen2(HANDLE hIoRes, HANDLE hObject, LONG nLimit, WORD wLimitLen);

// 设置回调函数及回调参数，目前只能设置读取事件
// lEvtType：1-OpenEvt,2-CloseEvt,3-ReadEvt,4-WriteEvt,5-ExceptEvt,6-AllEvt's Param,7-AllEvt
void IOCP_DLL_EXPORT SetIoEvent(HANDLE hIoRes, HANDLE hObject, LONG lEvtType, PFN_IO_EVENT pFunc, LPVOID lpParam);

// 发生错误时，消息发送给主窗口
void IOCP_DLL_EXPORT SetErrMsgWnd(HANDLE hIoRes, HWND hWnd, DWORD dwMsg);

// 获取本地地址
BOOL IOCP_DLL_EXPORT GetLocalAddr(
	HANDLE hIoRes, HANDLE hObject, PCHAR ip, PWORD port);

// 创建TCP客户端对象，增加本地端口
// 成功返回后必须调用StartConnectorReceive
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
);

// 附加参数
void IOCP_DLL_EXPORT SetObjectParam(HANDLE hIoRes, HANDLE hObject, LPVOID lpParam);
LPVOID IOCP_DLL_EXPORT GetObjectParam(HANDLE hIoRes, HANDLE hObject);

// 同步读取网络包，调用函数Start...Receive后就不能再调用此函数
// 返回：0：超时，-1：错误，>0：缓冲区大小
// 读取长度由 SetLimitReadLen2 决定
// nMsecOfTimeout: 超时时间，毫秒
int IOCP_DLL_EXPORT IocpSyncRead(HANDLE hIoRes, HANDLE hObject, HANDLE hIoBuf, LONG nMsecOfTimeout);

#ifdef OLD_IOCP
typedef enum _IO_OPERATION
{
	IO_CONNECT_COMPLETED = 0,			//Client主动连接完成
	IO_ACCEPT_COMPLETED,			//Server监听连接完成
	IO_INITIAL,						//IO初始化，以停用
	IO_RECV,						//数据读取中
	IO_RECV_COMPLETED,				//数据接收完成
	IO_SEND,						//数据发送中
	IO_SEND_COMPLETED,				//数据发送完毕
	IO_NET_EXCEPTION,				//网络异常,表示远程连接断开
	IO_ZERO_READ,					//0初始化读取，停用
	IO_ZERO_READ_COMPLETED,			//0初始化读取完毕，停用
	IO_TRANSMIT_FILE_COMPLETED,		//文件发送完毕
	IO_POST_PACKAGE,				//传递IO控制信号 

}IO_OPERATION;
typedef void (CALLBACK *fNetSDKCallBack)(int iClientID,/*客户ID标识*/
										 IO_OPERATION ioFlag,/*IO标志，参见IO_OPERATION*/
										 int iSuccess,/*0=ERR_OK,其他错误号*/
										 char *pBuffer,/*缓冲区*/
										 int iLen,/*缓冲区数据长度*/
										 DWORD dwUser/*用户自定义数据*/);
BOOL IOCP_DLL_EXPORT NET_Startup();
BOOL IOCP_DLL_EXPORT NET_Listen(DWORD dwPort);
BOOL IOCP_DLL_EXPORT NET_RegisterNetCallBack(fNetSDKCallBack lpFunc,DWORD dwUser);
BOOL IOCP_DLL_EXPORT NET_SendPacket(int iClientID,char *pPacket,int iLen);
BOOL IOCP_DLL_EXPORT NET_Connect(char *pClientIP,DWORD dwPort,int *iClientID);
BOOL IOCP_DLL_EXPORT NET_Disconnect(int iClientID);
BOOL IOCP_DLL_EXPORT NET_Close();
char* IOCP_DLL_EXPORT NET_GetRemoteIP(int iClientID);
#endif

#ifdef __cplusplus
}
#endif

#endif // __IOCP_H__

