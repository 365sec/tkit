#include "stdafx.h"
#include <tchar.h>
#ifndef PSAPI_VERSION
#define PSAPI_VERSION  1
#endif
#include<set>
#include <windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include "cy_system.h"
#pragma comment (lib,"Psapi.lib")
#include <atlconv.h>
#include "cy_stringconvert.h"
using namespace cy::system;
using namespace cy::convert;
using namespace std;
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

BOOL System::DosPathToNtPath(char* pszDosPath, char* pszNtPath)
{
	char            szDriveStr[500];
	char            szDrive[3];
	char            szDevName[100];
	int                cchDevName;
	int                i;

	//检查参数
	if (!pszDosPath || !pszNtPath)
		return FALSE;

	//获取本地磁盘字符串
	if (GetLogicalDriveStringsA(sizeof(szDriveStr), szDriveStr))
	{
		for (i = 0; szDriveStr[i]; i += 4)
		{
			if (!lstrcmpiA(&(szDriveStr[i]), ("A:\\")) || !lstrcmpiA(&(szDriveStr[i]), ("B:\\")))
				continue;

			szDrive[0] = szDriveStr[i];
			szDrive[1] = szDriveStr[i + 1];
			szDrive[2] = '\0';
			if (!QueryDosDeviceA(szDrive, szDevName, 100))//查询 Dos 设备名
				return FALSE;

			cchDevName = lstrlenA(szDevName);
			if (_tcsnicmp(pszDosPath, szDevName, cchDevName) == 0)//命中
			{
				lstrcpyA(pszNtPath, szDrive);//复制驱动器
				lstrcatA(pszNtPath, pszDosPath + cchDevName);//复制路径

				return TRUE;
			}
		}
	}

	lstrcpyA(pszNtPath, pszDosPath);

	return FALSE;
};
BOOL System::GetProcessFullPath(DWORD dwPID, char pszFullPath[MAX_PATH], set<string> &s)
{
	char        szImagePath[MAX_PATH];
	HANDLE        hProcess;
	if (!pszFullPath)
		return FALSE;

	pszFullPath[0] = '\0';
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, dwPID);
	if (!hProcess)
		return FALSE;

	if (!GetProcessImageFileNameA(hProcess, szImagePath, MAX_PATH))
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	if (!DosPathToNtPath(szImagePath, pszFullPath))
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	CloseHandle(hProcess);

	//_tprintf(_T("%s \r\n"), pszFullPath);
	if (pszFullPath[0] != 0)
	{
		//		printf("%d --- %s\n", i++, shortpath);
		s.insert(pszFullPath);
	}
	return TRUE;
}

BOOL System::get_system_process_path(set<string> &s)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return NULL;
	}
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);

	BOOL fOk;
	for (fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe))
	{
		char szProcessName[MAX_PATH] = { 0 };
		GetProcessFullPath(pe.th32ProcessID, szProcessName, s);
	}
	return FALSE;
}

BOOL System::get_system_service_path(set<string> &s)
{
	int menu = 1;
	if (menu == 1)
	{
		SC_HANDLE SCMan = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); //打开系统服务控制器
		if (SCMan == NULL)
		{
			printf("OpenSCManager Eorror\n");
			return 0;
		}
		LPENUM_SERVICE_STATUS service_status; //保存系统服务的结构
		DWORD cbBytesNeeded = NULL;
		DWORD ServicesReturned = NULL;
		DWORD ResumeHandle = NULL;

		service_status = (LPENUM_SERVICE_STATUS)LocalAlloc(LPTR, 1024 * 64);//分配内存，注意大小


																			//获取系统服务的简单信息
		bool ESS = EnumServicesStatus(SCMan, //系统服务句柄
			SERVICE_WIN32, //服务的类型
			SERVICE_STATE_ALL,  //服务的状态
			(LPENUM_SERVICE_STATUS)service_status,  //输出参数，系统服务的结构
			1024 * 64,  // 结构的大小
			&cbBytesNeeded, //输出参数，接收返回所需的服务
			&ServicesReturned, //输出参数，接收返回服务的数量
			&ResumeHandle); //输入输出参数，第一次调用必须为0，返回为0代表成功
		if (ESS == NULL)
		{
			printf("EnumServicesStatus Eorror\n");
			return FALSE;
		}
		for (int i = 0; i<int(ServicesReturned); i++)
		{
			//printf("服务名: %s ", service_status[i].lpDisplayName);
			LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL; //服务详细信息结构
			SC_HANDLE service_curren = NULL; //当前的服务句柄
			service_curren = OpenService(SCMan, service_status[i].lpServiceName, SERVICE_QUERY_CONFIG); //打开当前服务
			lpServiceConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 8 * 1024);      //分配内存， 最大为8kb 

			if (NULL == QueryServiceConfig(service_curren, lpServiceConfig, 8 * 1024, &ResumeHandle))
			{
				printf("QueryServiceConfig Error\n");
				return 0;
			}
			int nArgs = 0;
			LPWSTR *szArglist = NULL;
			szArglist = CommandLineToArgvW(cy::convert::StringConvert::AnsiToWide(lpServiceConfig->lpBinaryPathName).c_str(), &nArgs);

			if (NULL != szArglist) {

				s.insert(cy::convert::StringConvert::WideToAnsi(szArglist[0]).c_str());
				//	printf("%d %s\n",ssss++, cy::convert::StringConvert::WideToAnsi(szArglist[0]).c_str());
				LocalFree(szArglist);
			}
			CloseServiceHandle(service_curren);//关闭当前服务的句柄
		}
		CloseServiceHandle(SCMan);//关闭服务管理器的句柄
	}
	return 0;
}/*
BOOL System::IsWow64()  //64?
{
	typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process");
	BOOL bIsWow64 = FALSE;
	if (NULL != fnIsWow64Process)
	{
		fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
	}


	return bIsWow64;

}*/

BOOL System::QueryKey(HKEY hKey, set<string> &s)
{
	{
		//TCHAR    achKey[MAX_KEY_LENGTH];   // 存储子键名称
		//DWORD    cbName;                   // 名字字符串的长度
		char    achClass[MAX_PATH] = TEXT("");  // 指定一个字串，用于装载这个注册表项的类名 
		unsigned long    cchClassName = MAX_PATH;  // 指定一个变量，用于装载lpClass缓冲区的长度。一旦返回，它会设为实际装载到缓冲区的字节数量 
		unsigned long    cSubKeys = 0;               // 子键的数目 
		unsigned long    cbMaxSubKey;              // 设置最大子键长度 
		unsigned long    cchMaxClass;              // 指定一个变量，用于装载这个项之子项的最长一个类名的长度 
		unsigned long    cValues;              // 用于装载这个项的设置值数量的一个变量
		unsigned long    cchMaxValue;          // value的最长名字
		unsigned long    cbMaxValueData;       // value的最长数据
		unsigned long    cbSecurityDescriptor; // 安全描述符的大小 
		FILETIME ftLastWriteTime;      // 上次写入的时间 

		unsigned long i, retCode;
		unsigned long dwType = REG_SZ;
		unsigned long dwSize;

		char *wStr = new char[MAX_VALUE_NAME];
		char  achValue[MAX_VALUE_NAME];
		char  data[MAX_VALUE_NAME];
		unsigned long cchValue = MAX_VALUE_NAME;

		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(
			hKey,                    // 主键句柄 
			achClass,                // 指定一个字串，用于装载这个注册表项的类名
			&cchClassName,           // 指定一个变量，用于装载lpClass缓冲区的长度。一旦返回，它会设为实际装载到缓冲区的字节数量
			NULL,                    // reserved 
			&cSubKeys,               // 用于装载（保存）这个项的子项数量的一个变量
			&cbMaxSubKey,            // 指定一个变量，用于装载这个项最长一个子项的长度。注意这个长度不包括空中止字符
			&cchMaxClass,            // 指定一个变量，用于装载这个项之子项的最长一个类名的长度
			&cValues,                // 用于装载这个项的设置值数量的一个变量
			&cchMaxValue,            // 指定一个变量，用于装载这个项之子项的最长一个值名的长度
			&cbMaxValueData,         // 指定一个变量，用于装载容下这个项最长一个值数据所需的缓冲区长度
			&cbSecurityDescriptor,   // 装载值安全描述符长度的一个变量 
			&ftLastWriteTime);       // 指定一个结构，用于容纳该项的上一次修改时间

		if (cValues)
		{
			//printf( "\nNumber of values: %d\n", cValues);

			for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
			{
				cchValue = MAX_VALUE_NAME;
				dwSize = MAX_VALUE_NAME;
				dwType = REG_SZ;
				achValue[0] = '\0';
				data[0] = '\0';
				retCode = RegEnumValue(hKey, i,
					wStr,
					&cchValue,
					NULL,
					NULL,
					NULL,
					NULL);
				RegQueryValueEx(hKey, wStr,
					NULL,
					&dwType,
					(LPBYTE)data,
					&dwSize);

				if (retCode == ERROR_SUCCESS)
				{
					int nArgs = 0;
					LPWSTR *szArglist = NULL;
					szArglist = CommandLineToArgvW(cy::convert::StringConvert::AnsiToWide(data).c_str(), &nArgs);

					if (NULL != szArglist) {

						s.insert(cy::convert::StringConvert::WideToAnsi(szArglist[0]).c_str());
					  //  printf("%s\n",cy::convert::StringConvert::WideToAnsi(szArglist[0]).c_str());
						LocalFree(szArglist);
					}
					//s.insert(data);
				}
			}
		}
		delete[]wStr;
		RegCloseKey(hKey);
	}
	return 0;
}
BOOL  System::get_system_start_path(set<string> &s)
{

	HKEY hTestKey;
	HKEY aTestKey;
	HKEY sTestKey = NULL;

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
		0,
		KEY_ALL_ACCESS,
		&hTestKey) == ERROR_SUCCESS
		)

	{
		QueryKey(hTestKey, s);
	}
	RegCloseKey(hTestKey);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
		0,
		KEY_ALL_ACCESS,
		&aTestKey) == ERROR_SUCCESS
		)
	{
		QueryKey(aTestKey, s);
	}
	RegCloseKey(aTestKey);
	//printf("%d", IsWow64());
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
			0,
			KEY_ALL_ACCESS | KEY_WOW64_64KEY,
			&sTestKey) == ERROR_SUCCESS
			)
		{
		//	printf("%s\n", "dfaf");
			QueryKey(sTestKey, s);
		}
	return 0;
}