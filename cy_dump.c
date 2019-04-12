#include<stdio.h>
#include<tchar.h>
#include<Windows.h>
#include<Dbghelp.h>
#include<time.h>

#pragma comment(lib,"Dbghelp.lib")
// ����Dump�ļ�
// 
void CreateDumpFile(LPCTSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
	// ����Dump�ļ�
	//
	HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// Dump��Ϣ
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;
	// д��Dump�ļ�����
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
	CloseHandle(hDumpFile);
}

// ����Unhandled Exception�Ļص�����
//
LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{	
	// ���ﵯ��һ������Ի����˳�����
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

// һ���к������õ���
// 
void add_dumphandler()
{
	// ���ô���Unhandled Exception�Ļص�����
	// 
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler); 
}



