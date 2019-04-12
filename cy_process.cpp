#include<windows.h>

#include"cy_process.h"


namespace cy{
namespace  process{
//使用此方法创建一个进程
HANDLE ProcessBuilder::Start(std::string command,int bshow){
	STARTUPINFOA si = {sizeof(STARTUPINFO)};
	PROCESS_INFORMATION pi = {0};
	int nProgress = 1;
	DWORD dwRet = 0;
	ZeroMemory(&si,sizeof(si)); 
	si.cb   =   sizeof(si); 
	si.dwFlags   =   STARTF_USESHOWWINDOW; 
	if(!bshow){
		si.wShowWindow   =   SW_HIDE; 
	}else{
	    si.wShowWindow   =   SW_SHOW; 
	}
	si.lpReserved         =   NULL; 
	si.cbReserved2       =   0; 
	si.lpReserved2       =   NULL;	
	BOOL bRet = ::CreateProcessA(NULL,&command[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if(bRet){
		CloseHandle(pi.hThread);
		return (pi.hProcess);
	}
	return 0;
}


HANDLE ProcessBuilder::Start(std::string command,
					HANDLE in/*重定向标准输入*/,
					HANDLE out/*重定向标准输出*/,HANDLE error/*重定向标准错误流*/,int bshow )
{
	STARTUPINFOA si = {sizeof(STARTUPINFO)};
	PROCESS_INFORMATION pi = {0};
	int nProgress = 1;
	DWORD dwRet = 0;
	ZeroMemory(&si,sizeof(si)); 
	si.cb = sizeof(STARTUPINFOA);  
	GetStartupInfoA(&si); 
	si.dwFlags   = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; 
	si.hStdError = error;
	si.hStdInput = in;
	si.hStdOutput = out;
	si.wShowWindow = SW_HIDE; 
	BOOL bRet = ::CreateProcessA(NULL,&command[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if(bRet){
		CloseHandle(pi.hThread);
		return (pi.hProcess);
	}
	return 0;

}

void ProcessBuilder::KillProcessInDir(const char* dir,const char* ignore){



}

}//end of process

}//end of cy