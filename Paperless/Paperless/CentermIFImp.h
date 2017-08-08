#include "Centerm/HighCameraIF.h"

#pragma once

class CCentermIFImp : public CHighCameraIF
{
public:
	CCentermIFImp(CStatic* videoWnd);
	virtual ~CCentermIFImp();

public:
	int DetectDevice();
	int OpenDevice(int index);
	int CloseDevice();
	int ScanImage(CString path);
	int GetScanSizeCount(int index);
	int GetScanSize(int format, int index, int* width, int* height);
	int SetScanSize(int format, int index);
	int UpdatePreview();
	int SetAutoCrop(bool crop, int index);
	int SetCaptureDPI(int dpiX, int dpiY,int index);//Ô¤Áô½Ó¿Ú;

private:
	HMODULE m_hdll;
	HWND m_hwnd;
	void* m_handle;
	void* m_pCreateHighCamera;
	void* m_pReleaseHighCamera;
	void* m_pDetectDevice;
	void* m_pOpenDevice;
	void* m_pCloseDevice;
	void* m_pScanImage;
	void* m_pGetScanSizeCount;
	void* m_pGetScanSize;
	void* m_pSetScanSize;
	void* m_pUpdatePreview;
	void* m_pSetAutoCrop;
	void* m_pSetCaptureDPI;
};