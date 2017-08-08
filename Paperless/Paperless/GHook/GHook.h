#ifndef _GHOOK_H_
#define _GHOOK_H_

#include <windows.h>

#pragma once
#ifdef DLL_IMPLEMENT
#define DLL_API /*__declspec(dllexport)*/ __stdcall
#else
#define DLL_API /*__declspec(dllimport)*/ __stdcall
#endif
#include <stdio.h>
//组合键结构体
typedef struct S_COMBKEY
{
	int count;//组合键个数
	BYTE keys[4];
}COMBKEY;

#ifdef __cplusplus
extern "C"
{
#endif

BOOL DLL_API SetGHook_KEYBOARD();
//目前就设置了一个键盘钩子，如果有多个钩子的话，就根据钩子id来释放钩子
BOOL DLL_API FreeGHook_KEYBOARD();
void DLL_API SetCombKey(COMBKEY combKey[]);
void DLL_API SetXCombKey(int nX, COMBKEY combKey);
void DLL_API SetHWndAndMsgType(HWND hWnd[],UINT msgType[]);

#ifdef __cplusplus
}
#endif

#endif