#include "stdafx.h"
#include <string>
#include <string.h>
using std::string;

#pragma once

/**    
    功能：    将汉字转换成拼音全拼，例如将“马兆瑞”转换成“mazhaorui”。（“妈ma521”可转换成“mama521”）
    @param[in]    Chinese：要转换的汉字字符
    @param[out]    PinYin：转换后的拼音字符
    @par修改记录：
**/
void GetPinYin(unsigned char* Chinese, string& PinYin);

void GetOtherCodePinYin(int nCode, string& strValue);

/* 功能:将传入的中文转大写拼音，且各个中文拼音之间有一个空格
 * 入参：name：传入的中文，传入字符串必须都为中文，且不含有空格，最大字节数128字节
 * 出参：PinYin：转后的拼音
 */
void MyGetPinYin(const char *name, char *PinYin);

// GBK转UTF-8
void ConvertGBKToUtf8(CString &strGBK);

// UTF-8转GBK
void ConvertUtf8ToGBK(CString &strUtf8);

// GBK转UTF-8  2
string MyGBKToUtf8(const string& strGBK);

// UTF-8转GBK  2
string MyUtf8ToGBK(const string& strUTF8);

/* 获取当前程序执行路径
*/
CString GetAppPath();

/* 通过GetLastError()的返回值获取文字信息
 * 返回信息：sLastMsg
*/
char* MyGetLastError(char *sLastMsg, int strLen);

/* 用'|'分割字符串，获取src字符串第 index 个子串(index从0开始)，存入des字符串
 * 0-成功，其他-失败
*/
int splitString(char *des, const char *src, int index);

/* 读取注册表信息
*/
BOOL MyReadRegedit(char *subKeyDir, char *subKey, char *subValue, int subValueLen);

/* 写注册表信息
*/
BOOL MyWriteRegedit(char *subKeyDir, char *subKey, char *subValue);

/* 获取sFilePath的文件信息存入pStat中
*/
BOOL GetFileAttributes(struct _stati64 *pStat, CString sFilePath);