#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>

#if defined(__BORLANDC__)
#pragma warn -8027
#endif

#define USE_STDC_API

#define EXCEPTION_PRINT() GtWriteTrace( \
	EM_TraceError, "发生异常 %s %d", __FILE__, __LINE__)

#define EXCEPTION_PRINT_INT(x) GtWriteTrace( \
	EM_TraceError, "发生异常 %s %d %d", __FILE__, __LINE__, x)

#define TRY_DEF try {
#define CATCH_DEF } catch(...) { EXCEPTION_PRINT(); }

#define ENTER_CS(x) { ::EnterCriticalSection(x); TRY_DEF
#define TRY_ENTER_CS(x) if (::TryEnterCriticalSection(x)) { TRY_DEF
#define LEAVE_CS(x) ::LeaveCriticalSection(x); CATCH_DEF }

#ifndef __func__
# ifdef __FUNCTION__
#   define __func__  __FUNCTION__
# else
#   ifdef __FUNC__
#     define __func__ __FUNC__
#   else
#     define __func__  __FILE__
#   endif
# endif
#endif

enum Em_TraceLevel
{
    EM_TraceNone = 0, // no trace
    EM_TraceError = 10, // only trace error
    EM_TraceInfo = 20, // some extra info
    EM_TraceDebug = 30, // debugging info
    EM_TraceDetail = 40 // detailed debugging info
};

// private helper class
class TXYTraceHelper
{
    // friend functions of this class
    friend void GtSetTraceFilePrefix(LPCTSTR strFilePrefix);
    friend void GtSetTraceLevel(const int nLevel);
    // internal data members
#ifdef USE_STDC_API
    FILE *m_hFile;
#else
    HANDLE m_hFile;
#endif
    int m_nLevel;
	long m_nThreadId;
	TCHAR* m_pTraceFilePrefix;
    SYSTEMTIME m_timeStart;

	// close the current trace file
    void CloseTraceFile();
#ifdef USE_STDC_API
FILE*
#else
HANDLE
#endif
	OpenTraceFile();
    // set the current trace level
    void SetTraceLevel(const int nLevel);
    // set the trace file name prefix
    void SetTraceFilePrefix(LPCTSTR strFilePrefix);
    // constructor and destructor
    TXYTraceHelper();

public:
    ~TXYTraceHelper();
    static TXYTraceHelper* GetInstance();
    // set lock to gain exclusive access to trace
    // functions
    void Lock();
    // release lock so that other threads can access
    // trace functions
    void Unlock();
    void WriteTrace(const int nLevel, LPCTSTR strFormat, va_list args);
};

extern void GtSetTraceFilePrefix(LPCTSTR strFilePrefix);

extern void GtSetTraceLevel(const int nLevel);

extern "C" void GtWriteTrace(const int nLevel, LPCTSTR strFormat, ...);

class TFuncEnterLeave
{
    char *m_func;
    int m_iLevel;
public:
    __fastcall TFuncEnterLeave(char *func, int iLevel)
        : m_func(func) , m_iLevel(iLevel)
    {
        GtWriteTrace(m_iLevel, "%s: Enter", m_func);
    }

    __fastcall ~TFuncEnterLeave()
    {
        GtWriteTrace(m_iLevel, "%s: Leave", m_func);
    }
};

#ifndef DEBUG_ENTER_LEAVE_FUNC
    #define DEBUG_ENTER_LEAVE_FUNC TFuncEnterLeave _funcEnterLeave(__func__, EM_TraceDebug);
#endif

#ifndef INFO_ENTER_LEAVE_FUNC
    #define INFO_ENTER_LEAVE_FUNC TFuncEnterLeave _funcEnterLeave(__func__, EM_TraceInfo);
#endif

#ifndef ERROR_ENTER_LEAVE_FUNC
    #define ERROR_ENTER_LEAVE_FUNC TFuncEnterLeave _funcEnterLeave(__func__, EM_TraceError);
#endif

#ifndef DETAIL_ENTER_LEAVE_FUNC
    #define DETAIL_ENTER_LEAVE_FUNC TFuncEnterLeave _funcEnterLeave(__func__, EM_TraceDetail);
#endif

inline void GtLogError(LPCTSTR sFormat, ...)
{
	TXYTraceHelper *pTrace = TXYTraceHelper::GetInstance();
    va_list args;
    va_start(args, sFormat);
    pTrace->WriteTrace(EM_TraceError, sFormat, args);
    va_end(args);
}

inline void GtLogInfo(LPCTSTR sFormat, ...)
{
	TXYTraceHelper *pTrace = TXYTraceHelper::GetInstance();
    va_list args;
    va_start(args, sFormat);
    pTrace->WriteTrace(EM_TraceInfo, sFormat, args);
    va_end(args);
}

inline void GtLogDebug(LPCTSTR sFormat, ...)
{
	TXYTraceHelper *pTrace = TXYTraceHelper::GetInstance();
    va_list args;
    va_start(args, sFormat);
    pTrace->WriteTrace(EM_TraceDebug, sFormat, args);
    va_end(args);
}

inline void GtLogDetail(LPCTSTR sFormat, ...)
{
	TXYTraceHelper *pTrace = TXYTraceHelper::GetInstance();
    va_list args;
    va_start(args, sFormat);
    pTrace->WriteTrace(EM_TraceDetail, sFormat, args);
    va_end(args);
}

inline CString GetFilePath()
{
	CString m_FilePath;
	GetModuleFileName(NULL,m_FilePath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
	m_FilePath.ReleaseBuffer();
	int m_iPosindex;
	m_iPosindex = m_FilePath.ReverseFind('\\');
	m_FilePath = m_FilePath.Left(m_iPosindex);
	return m_FilePath;
}


