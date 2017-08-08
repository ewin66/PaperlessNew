// GHook.cpp : 定义 DLL 应用程序的导出函数。
//
#include "stdafx.h"
#define DLL_IMPLEMENT
#include "GHook.h"

#define HOOK_NUM 4
//全局变量
HHOOK g_keyboardHook = NULL;
COMBKEY g_combKey[HOOK_NUM];
HWND g_hWnd[HOOK_NUM];
UINT g_msgType[HOOK_NUM];

LRESULT CALLBACK KeyBoardMsgProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	/*
	if(nCode < 0)
	{
		return CallNextHookEx(g_keyboardHook,nCode,wParam,lParam); 
	}
	else if(wParam == VK_ESCAPE)
	{
		return CallNextHookEx(g_keyboardHook,code,wParam,lParam);
	}
	else 
	{
		return 1;
	}*/

	KBDLLHOOKSTRUCT* pStruct = (KBDLLHOOKSTRUCT*)lParam;
	/*
	DWORD dwMsg = 1;
	dwMsg += pStruct->scanCode << 16;
	dwMsg += pStruct->flags << 24; 
	char str[20]={0};
	GetKeyNameText( dwMsg, str, 20);
	FILE *fp = fopen("C:/Users/HYQ/Desktop/test.txt", "a+");
	if(fp != NULL)
	{
		fprintf(fp, "key msg = %s, %d\n", str, pStruct->vkCode);
		fprintf(fp, "g_combKey = %d  %d, %d, %d, %d\n", g_combKey.count, g_combKey.keys[0], g_combKey.keys[1], g_combKey.keys[2], g_combKey.keys[3]);
		fclose(fp);
	}
	*/
	for (int i = 0; i < HOOK_NUM; i++)
	{
		if(g_combKey[i].count != 0)
		{
			switch(g_combKey[i].count)
			{
			case 2://2个键的组合键
				if(wParam == WM_KEYDOWN)
				{
					if(pStruct->vkCode == g_combKey[i].keys[1])
					{
						if(GetAsyncKeyState(g_combKey[i].keys[0])&0x8000) //另外一个键按下
						{
							if(g_hWnd)
								::PostMessage(g_hWnd[i], g_msgType[i], 0, 0);  //发送给对应的窗口
						}
					}
				}
				break;
			case 3://3个键的组合键
				if(wParam == WM_KEYDOWN)
				{
					if(pStruct->vkCode == g_combKey[i].keys[2])
					{
						if((GetAsyncKeyState(g_combKey[i].keys[0])&0x8000) && 
							(GetAsyncKeyState(g_combKey[i].keys[1])&0x8000)) //另外一个键按下
						{
							if(g_hWnd)
								::PostMessage(g_hWnd[i], g_msgType[i], 0, 0);  //发送给对应的窗口
						}
					}
				}
				break;
			case 4://4个键盘的组合键
				if(wParam == WM_KEYDOWN)
				{
					if(pStruct->vkCode == g_combKey[i].keys[3])
					{
						if((GetAsyncKeyState(g_combKey[i].keys[0])&0x8000) && 
							(GetAsyncKeyState(g_combKey[i].keys[1])&0x8000) && 
							(GetAsyncKeyState(g_combKey[i].keys[2])&0x8000)) //另外一个键按下
						{
							if(g_hWnd)
								::PostMessage(g_hWnd[i], g_msgType[i], 0, 0);  //发送给对应的窗口
						}
					}
				}
				break;
			}
		
		}
	}
	return CallNextHookEx(g_keyboardHook,nCode,wParam,lParam);
}
#ifdef __cplusplus
extern "C"
{
#endif
BOOL DLL_API SetGHook_KEYBOARD()
{
	if(NULL == g_keyboardHook)
		g_keyboardHook = (HHOOK)SetWindowsHookEx(WH_KEYBOARD_LL, KeyBoardMsgProc, GetModuleHandle(NULL), 0); 
	if(g_keyboardHook)
		return TRUE;
	else
		return FALSE;
}
BOOL DLL_API FreeGHook_KEYBOARD()
{
	if(NULL != g_keyboardHook)
		return UnhookWindowsHookEx(g_keyboardHook);
	return TRUE;
}
void DLL_API SetCombKey(COMBKEY combKey[])
{
	memset(g_combKey, 0, sizeof(COMBKEY) * 4);
	for (int i = 0; i < HOOK_NUM; i++)
	{
		g_combKey[i].count = combKey[i].count;
		for(int j=0; j<combKey[i].count; j++)
		{
			g_combKey[i].keys[j] = combKey[i].keys[j];
		}
	}
	/*
	FILE *fp = fopen("C:/Users/HYQ/Desktop/test.txt", "a+");
	if(fp != NULL)
	{
		fprintf(fp, "SetCombKey %d  %d, %d, %d, %d\n", g_combKey.count, g_combKey.keys[0], g_combKey.keys[1], g_combKey.keys[2], g_combKey.keys[3]);
		fclose(fp);
	}
	*/
}


void DLL_API SetXCombKey(int nX, COMBKEY combKey)
{
	if (nX >= HOOK_NUM || nX < 0)
	{
		return ;
	}
	g_combKey[nX].count = combKey.count;
	for(int j=0; j<combKey.count; j++)
	{
		g_combKey[nX].keys[j] = combKey.keys[j];
	}
}


void DLL_API SetHWndAndMsgType(HWND hWnd[], UINT msgType[])
{
	for (int i = 0; i < HOOK_NUM; i++)
	{
		g_hWnd[i] = hWnd[i];
		g_msgType[i] = msgType[i];
	}
}

#ifdef __cplusplus
}
#endif