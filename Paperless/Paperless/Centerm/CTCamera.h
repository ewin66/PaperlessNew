	/**
	* @brief 创建sdk操作句柄;
	* @param[in] hwnd 预览窗口句柄;
	* @return 0/-1 成功/失败;
	*/
	HANDLE WINAPI hhCreateHighCamera(HWND hwnd);

	/*
	* @brief 释放SDK操作句柄,回收资源;
	* @return 0
	*/
	void WINAPI hhReleaseHighCamera(HANDLE handle);

	/*
	* @brief 枚举摄像头设备;
	* @return 设备数;
	*/
	int WINAPI hhDetectDevice(HANDLE handle);

	/*
	* @brief 打开指定摄像头;
	* @param[in] index 摄像头编号: 0/1 主摄像头/副摄像头;
	* @return 0/1 成功/失败;
	*/
	int WINAPI hhOpenDevice(HANDLE handle, int index);

	/*
	* @brief 关闭当前摄像头;
	* @param[in] handle sdk操作句柄;
	* @return 0/-1 成功/失败;
	*/
	int WINAPI hhCloseDevice(HANDLE handle);

	/*
	* @brief 拍照;
	* @param[in] handle sdk操作句柄;
	* @param[in] index 摄像头编号: 0/1 主摄像头/副摄像头;
	* @return -1 失败 ; 其他： 分辨率个数;
	*/
	int WINAPI hhScanImage(HANDLE handle, const wchar_t* path);

	/*
	* @brief 关闭指定摄像头;
	* @param[in] handle sdk操作句柄;
	* @param[in] index 摄像头编号: 0/1 主摄像头/副摄像头;
	* @return 0/-1 成功/失败;
	*/
	int WINAPI hhGetScanSizeCount(HANDLE handle, int index);

	/*
	* @brief 获取指定设备的分辨率;
	* @param[in] handle sdk操作句柄;
	* @param[in] index 摄像头编号: 0/1 主摄像头/副摄像头;
	* @param[in] format 分辨率索引号;
	* @param[out] width/height  输出获取的分辨率值;
	* @return 0/-1 成功/失败;
	*/
	int WINAPI hhGetScanSize(HANDLE handle, int index, int format, int* width, int* height);

	/*
	* @brief 设置指定设备的分辨率;
	* @param[in] handle sdk操作句柄;
	* @param[in] index 摄像头编号: 0/1 主摄像头/副摄像头;
	* @param[in] format 分辨率索引号;
	* @return 0/-1 成功/失败;
	*/
	int WINAPI hhSetScanSize(HANDLE handle, int index, int format);

	/*
	* @brief 弹出属性设置界面;
	* @param[in] handle sdk操作句柄;
	* @return 0/-1 成功/失败;
	*/
	int WINAPI hhUpdatePreview(HANDLE handle);

	/*
	* @brief 设置自动裁边;
	* @param[in] handle sdk操作句柄;
	* @param[in] index 摄像头编号: 0/1 主摄像头/副摄像头;
	* @param[in] crop true/false 开启/关闭;
	* @return 0/-1 成功/失败;
	*/
	int WINAPI hhSetAutoCrop(HANDLE handle, int index, bool crop);

	/*
	* @brief 设置扫描照片的dpi;
	* @param[in] handle sdk操作句柄;
	* @param[in] dpiX / dpiY  横向dpi/纵向dpi;
	* @return 0/-1 成功/失败;
	*/
	int WINAPI hhSetCaptureDPI(HANDLE handle, int dpiX, int dpiY);

	/*
	* @brief 设置是否扫描框选部分;
	* @param[in] handle sdk操作句柄;
	* @param[in] captureCrop true/false 开启/关闭;
	*/
	int WINAPI SetCaptureCrop(HANDLE handle, bool captureCrop);