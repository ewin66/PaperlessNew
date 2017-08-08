#include "stdafx.h"
#include "CentermIFImp.h"
#include <windows.h>
#include <tlhelp32.h>
#include <atlconv.h>

typedef void* (__stdcall* CREATEHIGHCAMERA)(void* hwnd);
typedef void(__stdcall* RELEASEHIGHCAMERA)(void* handle);
typedef int(__stdcall* DETECTDEVICE)(void* handle);
typedef int(__stdcall* OPENDEVICE)(void* handle, int index);
typedef int(__stdcall* CLOSEDEVICE)(void* handle);
typedef int(__stdcall* SCANIMAGE)(void* handle, const wchar_t* path);
typedef int(__stdcall* GETSCANSIZECOUNT)(void* handle, int index);
typedef int(__stdcall* GETSCANSIZE)(void* handle, int index, int format, int* width, int* weight);
typedef int(__stdcall* SETSCANSIZE)(void* handle, int index, int format);
typedef int(__stdcall* UPDATEPREVIEW)(void* handle);
typedef int(__stdcall* SETAUTOCROP)(void* handle, int index, bool crop);
typedef int(__stdcall* SETCAPTUREDPI)(void* handle, int dpiX, int dpiY);

static DWORD GetCurrentModuleAddr(void)
{
	DWORD dwPID = GetCurrentProcessId();
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;
	DWORD dwCurrentModule = 0;
	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		return dwCurrentModule;
	}

	// Set the size of the structure before using it.
	me32.dwSize = sizeof(MODULEENTRY32);

	// Retrieve information about the first module,
	// and exit if unsuccessful
	if (!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);           // clean the snapshot object
		return dwCurrentModule;
	}

	// Now walk the module list of the process,
	// and display information about each module
	do
	{
		if (((int)GetCurrentModuleAddr > (int)me32.modBaseAddr)
			&& ((int)GetCurrentModuleAddr < (int)(me32.modBaseAddr + me32.modBaseSize)))
		{
			dwCurrentModule = (DWORD)me32.modBaseAddr;
			break;
		}
	} while (Module32Next(hModuleSnap, &me32));

	CloseHandle(hModuleSnap);
	return dwCurrentModule;
}

//在模块路径下,加载指定文件名的动态库;
HINSTANCE LoadSdk(const char* dllname)
{
	char curModulePath[MAX_PATH] = { 0 };
	char dllPath[MAX_PATH] = { 0 };
	HMODULE hModule = 0;
	DWORD curModule = GetCurrentModuleAddr();
	GetModuleFileNameA((HMODULE)curModule, curModulePath, _countof(curModulePath));
	char* p = strrchr(curModulePath, '\\');
	if (p)
	{
		*(p + 1) = '\0';
		memcpy(dllPath, curModulePath, strlen(curModulePath));
		memcpy(dllPath + strlen(curModulePath), dllname, strlen(dllname));
	}

	return LoadLibraryA(dllPath);
}

CCentermIFImp::CCentermIFImp(CStatic *pVideoWnd)
	:CHighCameraIF(pVideoWnd)
{
	m_hdll = LoadSdk("CTCamera.dll");
	m_hwnd = NULL;
	m_handle = NULL;

	m_pCreateHighCamera = GetProcAddress(m_hdll, "hhCreateHighCamera");
	m_pReleaseHighCamera = GetProcAddress(m_hdll, "hhReleaseHighCamera");
	m_pDetectDevice = GetProcAddress(m_hdll, "hhDetectDevice");
	m_pOpenDevice = GetProcAddress(m_hdll, "hhOpenDevice");
	m_pCloseDevice = GetProcAddress(m_hdll, "hhCloseDevice");
	m_pScanImage = GetProcAddress(m_hdll, "hhScanImage");
	m_pGetScanSizeCount = GetProcAddress(m_hdll, "hhGetScanSizeCount");
	m_pGetScanSize = GetProcAddress(m_hdll, "hhGetScanSize");
	m_pSetScanSize = GetProcAddress(m_hdll, "hhSetScanSize");
	m_pUpdatePreview = GetProcAddress(m_hdll, "hhUpdatePreview");
	m_pSetAutoCrop = GetProcAddress(m_hdll, "hhSetAutoCrop");
	m_pSetCaptureDPI = GetProcAddress(m_hdll, "hhSetCaptureDPI");

	if (pVideoWnd != NULL)
	{
		m_hwnd = pVideoWnd->GetSafeHwnd();
	}

	if (m_pCreateHighCamera != NULL)
	{
		m_handle = ((CREATEHIGHCAMERA)m_pCreateHighCamera)(m_hwnd);
	}

	
}

CCentermIFImp::~CCentermIFImp()
{
	if (m_pReleaseHighCamera != NULL)
	{
		((RELEASEHIGHCAMERA)m_pReleaseHighCamera)(m_handle);
	}

	if (m_hdll != NULL)
	{
		FreeLibrary(m_hdll);
	}
}

int CCentermIFImp::DetectDevice()
{
	if (m_pDetectDevice == NULL)
	{
		return 0;
	}

	return ((DETECTDEVICE)m_pDetectDevice)(m_handle);
}

int CCentermIFImp::OpenDevice(int index)
{
	if (m_pOpenDevice == NULL)
	{
		return -1;
	}

	return ((OPENDEVICE)m_pOpenDevice)(m_handle, index);
}

int CCentermIFImp::CloseDevice()
{
	if (m_pCloseDevice == NULL)
	{
		return -1;
	}

	return ((CLOSEDEVICE)m_pCloseDevice)(m_handle);
}

int CCentermIFImp::ScanImage(CString path)
{
	if (m_pScanImage == NULL)
	{
		return -1;
	}

	USES_CONVERSION;

	return ((SCANIMAGE)m_pScanImage)(m_handle, T2W(path.GetBuffer()));
}

int CCentermIFImp::GetScanSizeCount(int index)
{
	if (m_pGetScanSizeCount == NULL)
	{
		return 0;
	}

	return ((GETSCANSIZECOUNT)m_pGetScanSizeCount)(m_handle, index);
}

int CCentermIFImp::GetScanSize(int format, int index, int* width, int* height)
{
	if (m_pGetScanSize == NULL)
	{
		return -1;
	}

	return ((GETSCANSIZE)m_pGetScanSize)(m_handle, index, format, width, height);

}

int CCentermIFImp::SetScanSize(int format, int index)
{
	if (m_pSetScanSize == NULL)
	{
		return -1;
	}

	return ((SETSCANSIZE)m_pSetScanSize)(m_handle, index, format);
}

int CCentermIFImp::UpdatePreview()
{
	if (m_pUpdatePreview == NULL)
	{
		return -1;
	}

	return ((UPDATEPREVIEW)m_pUpdatePreview)(m_handle);
}

int CCentermIFImp::SetAutoCrop(bool crop, int index)
{
	if (m_pSetAutoCrop == NULL)
	{
		return -1;
	}

	return ((SETAUTOCROP)m_pSetAutoCrop)(m_handle, index, crop);
}

int CCentermIFImp::SetCaptureDPI(int dpiX, int dpiY, int index)
{
	if (m_pSetCaptureDPI == NULL)
	{
		return -1;
	}

	return ((SETCAPTUREDPI)m_pSetCaptureDPI)(m_handle, dpiX, dpiY);
}