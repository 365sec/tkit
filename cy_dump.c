#include<stdio.h>
#include<tchar.h>
#include<Windows.h>
#include<Dbghelp.h>
#include<time.h>

#pragma comment(lib,"Dbghelp.lib")
// 创建Dump文件
// 
void CreateDumpFile(LPCTSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
	// 创建Dump文件
	//
	HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// Dump信息
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;
	// 写入Dump文件内容
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
	CloseHandle(hDumpFile);
}

// 处理Unhandled Exception的回调函数
//
LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{	
	// 这里弹出一个错误对话框并退出程序
	//
	//FatalAppExit(-1,  _T("*** Unhandled Exception! ***"));
	TCHAR    dpath[512];
	TCHAR    module[512];
	TCHAR   * pName;
	
	GetModuleFileName(NULL,module,sizeof(module));
	pName = _tcsrchr(module,'\\');
	if(pName && *pName)
	{
		_stprintf(dpath,_T("logs\\%I64d-%d-%s.dmp"),time(NULL),GetCurrentProcessId(),pName+1);
	}else
	_stprintf(dpath,_T("logs\\%I64d-%d.dmp"),time(NULL),GetCurrentProcessId());

	CreateDumpFile(dpath,  pException);
	return EXCEPTION_EXECUTE_HANDLER;
}

// 一个有函数调用的类
// 
void add_dumphandler()
{
	// 设置处理Unhandled Exception的回调函数
	// 
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler); 
}



