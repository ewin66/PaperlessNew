/***************************************

***************************************/
#ifndef __HIGHCAMERAIF_H__
#define __HIGHCAMERAIF_H__

class CHighCameraIF
{
public:
	CHighCameraIF(CStatic *pVideoWnd ){}
	virtual ~CHighCameraIF(){}

//要求OpenDevice、ScanImage、UpdatePreview等接口，成功返回0，其他为失败。需要特别说明的，请注明返回值
public:
	virtual int DetectDevice() = 0;//探测设备是否存在。返回厂商高拍仪的摄像头个数
	virtual	int OpenDevice( int idx ) = 0;//打开高拍仪的摄像头 要求：id=0为主摄像头 id=1为副摄像头
	virtual int CloseDevice() = 0;//关闭当前设备，释放相关资源，高拍仪退出时调用   --- 具体高拍仪切换镜头时是否需要调用？
	virtual int ScanImage( CString strFileName ) = 0;//拍照，按要求保存图像	-- 接口写好后请说明一下strFileName的格式
	virtual int GetScanSizeCount( int idx ) = 0;//获取指定摄像头分辨率模式数量，例如支持800*600,1024*768则返回2
	virtual int GetScanSize( int  nIndex,
                             int  idx,
                             int *pWidth,
                             int *pHeight ) = 0;//指定摄像头指定分辨率模式的具体数值
	virtual int SetScanSize( int nIndex, int idx ) = 0;//设置指定摄像头的分辨率
	virtual int UpdatePreview() = 0;//更新高拍仪参数设置--如曝光度等
	virtual int SetAutoCrop( bool bIsAuto, int idx ) = 0;//指定摄像头自动裁边
};

#endif