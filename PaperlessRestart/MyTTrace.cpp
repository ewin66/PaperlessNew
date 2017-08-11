#include "stdafx.h"

#include "MyTTrace.h"


// close the current trace file
void TXYTraceHelper::CloseTraceFile()
{
	if (m_hFile)
#ifdef USE_STDC_API
		::fclose(m_hFile);
#else
		::CloseHandle(m_hFile);
#endif
	m_hFile = NULL;
}


// open a new trace file
#ifdef USE_STDC_API
FILE*
#else
HANDLE
#endif
TXYTraceHelper::OpenTraceFile()
{ 
	// construct the new trace file path
	TCHAR strFilePath[MAX_PATH];
	SYSTEMTIME sysTime;
	::GetLocalTime(&sysTime);

	if ( NULL ==  m_pTraceFilePrefix )
	{
		GetFileAttributes(".//log//");               //获得文件或目录属性
		if (GetLastError()   ==   2)
		{
			 CreateDirectoryA(".//log//",NULL);
		}
		SetTraceFilePrefix(".//log//Trace");
	}
	_stprintf
	(
		strFilePath,
		_T("%s_%04d%02d%02d_%02d%02d%02d_%X.log"),
		m_pTraceFilePrefix?m_pTraceFilePrefix:_T("Trace"),
		sysTime.wYear,
		sysTime.wMonth,
		sysTime.wDay,
		sysTime.wHour,
		sysTime.wMinute,
		sysTime.wSecond,
		::GetCurrentProcessId()
		);
	// create the new trace file
#ifdef USE_STDC_API
	m_hFile = ::_tfopen(strFilePath, _T("w"));
#else
	m_hFile = ::CreateFile
			  (
				  strFilePath,
				  GENERIC_WRITE,
				  FILE_SHARE_READ,
				  NULL,
				  CREATE_ALWAYS,
				  FILE_ATTRIBUTE_NORMAL,
				  NULL
			  );
#endif
	// if successful, save the start time variable
	if (m_hFile) m_timeStart = sysTime;
	// return the file handle
	return m_hFile;
}


// set the current trace level
void TXYTraceHelper::SetTraceLevel(const int nLevel)
{
	m_nLevel = nLevel>0?nLevel:0;
}


// set the trace file name prefix
void TXYTraceHelper::SetTraceFilePrefix(LPCTSTR strFilePrefix)
{
	// close existing trace file first
	CloseTraceFile();
	// copy the file name prefix
	int nSize = strFilePrefix?_tcslen(strFilePrefix):0;
	delete []m_pTraceFilePrefix;
	m_pTraceFilePrefix = new TCHAR[nSize+1];
	_tcscpy(m_pTraceFilePrefix, nSize>0?strFilePrefix:_T(""));
}


// constructor and destructor
TXYTraceHelper::TXYTraceHelper()
{
	m_hFile = NULL;
	m_nLevel = 0;
	m_nThreadId = 0;
	m_pTraceFilePrefix = NULL;
}


TXYTraceHelper::~TXYTraceHelper()
{
	CloseTraceFile();
	delete []m_pTraceFilePrefix;
}


TXYTraceHelper* TXYTraceHelper::GetInstance()
{
	static TXYTraceHelper obj;
	return &obj;
}


// set lock to gain exclusive access to trace
// functions
void TXYTraceHelper::Lock()
{
	long nThreadId = ::GetCurrentThreadId();
	while (m_nThreadId!=nThreadId)
	{
		// keep trying until successfully completed the operation
		::InterlockedCompareExchange((  long *)&m_nThreadId, (long)nThreadId, 0);
		//vc 6
		//::InterlockedCompareExchange((  void **)&m_nThreadId, (void *)nThreadId, 0);
		if (m_nThreadId==nThreadId) break;
		::Sleep(25);
	}
}


// release lock so that other threads can access
// trace functions
void TXYTraceHelper::Unlock()
{
	// only the thread that set the lock can release it
	::InterlockedCompareExchange((  long *)&m_nThreadId, 0, (long )::GetCurrentThreadId());
	//vc 6
	//::InterlockedCompareExchange((  void **)&m_nThreadId, 0, (void *)::GetCurrentThreadId());
}


void TXYTraceHelper::WriteTrace(const int nLevel, LPCTSTR strFormat, va_list args)
{
	// if the specified trace level is greater than
	// the current trace level, return immediately
	//cbm if(pTrace->m_nLevel==0||nLevel>this->m_nLevel) return;
	// set lock
	// return; /// debug for memo inc
	if (nLevel > this->m_nLevel)
	{
		return;
	}
	this->Lock();
	try
	{
		// get local time
		SYSTEMTIME sysTime;
		::GetLocalTime(&sysTime);
		// get trace file handle
#ifdef USE_STDC_API
		FILE*
#else
		HANDLE
#endif
			hFile = this->m_hFile;
		// open the trace file if not already open
		if (hFile==NULL)
		{
			hFile = this->OpenTraceFile();
		}
		// if it is already a new day, close the old
		// trace file and open a new one
		else if (
			sysTime.wYear!=this->m_timeStart.wYear||
			sysTime.wMonth!=this->m_timeStart.wMonth||
			sysTime.wDay!=this->m_timeStart.wDay)
		{
			this->CloseTraceFile();
			hFile = this->OpenTraceFile();
		}
		// write the trace message
		if (hFile)
		{
#ifdef USE_STDC_API
			// print time stamp and thread id to buffer
			_ftprintf(
						   hFile,
						   _T("\n%02d:%02d:%02d_%03d_%04X: "),
						   sysTime.wHour,
						   sysTime.wMinute,
						   sysTime.wSecond,
						   sysTime.wMilliseconds,
						   this->m_nThreadId
					 );
			// print the trace message to buffer
			_vftprintf(hFile, strFormat, args);
			fflush(hFile);
#else
			// declare buffer (default max buffer size = 32k)
			const int nMaxSize = 32*1024;
			TCHAR pBuffer[nMaxSize+51];
			// print time stamp and thread id to buffer
			int nPos = _stprintf
					   (
						   pBuffer,
						   _T("%02d:%02d:%02d_%03d_%X: "),
						   sysTime.wHour,
						   sysTime.wMinute,
						   sysTime.wSecond,
						   sysTime.wMilliseconds,
						   this->m_nThreadId
					   );
			// print the trace message to buffer
			nPos += _vsntprintf(pBuffer+nPos,nMaxSize,strFormat,args);
			// print the end of the line to buffer
			_stprintf(pBuffer+nPos,_T("\r\n"));
			// write the buffer to the trace file
			DWORD dwBytes;
			::WriteFile(hFile,pBuffer,_tcslen(pBuffer),&dwBytes,NULL);
#endif
		}
	}
	catch (...)
	{
		// add code to handle exception (if needed)
	}
	// release lock
	this->Unlock();
}


void GtSetTraceFilePrefix(LPCTSTR strFilePrefix)
{
	TXYTraceHelper *pTrace = TXYTraceHelper::GetInstance();

    // set lock
    pTrace->Lock();
    // set trace file name prefix
    pTrace->SetTraceFilePrefix(strFilePrefix);
    // release lock
    pTrace->Unlock();
}


void GtSetTraceLevel(const int nLevel)
{
	TXYTraceHelper *pTrace = TXYTraceHelper::GetInstance();

    // set lock
    pTrace->Lock();
    // set trace level
    pTrace->SetTraceLevel(nLevel);
    // release lock
    pTrace->Unlock();
}


void GtWriteTrace(const int nLevel, LPCTSTR strFormat, ...)
{
	TXYTraceHelper *pTrace = TXYTraceHelper::GetInstance();
    va_list args;
    va_start(args, strFormat);
    pTrace->WriteTrace(nLevel, strFormat, args);
    va_end(args);
}