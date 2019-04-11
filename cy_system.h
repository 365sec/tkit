#pragma once

#include<set>
#include <windows.h>
using namespace std;


namespace cy
{
	namespace system
	{
		class System
		{
		public:
			static BOOL DosPathToNtPath(char* pszDosPath, char* pszNtPath);
			/*
			* @brief   获取驱动器路径
			* @param   LPTSTR 
			* @param   LPTSTR
			* @return  成功返回true，其它返回false
			*/
			static BOOL GetProcessFullPath(DWORD dwPID, char pszFullPath[MAX_PATH], set<string> &s);
			/*
			* @brief   获取进程全路径
			* @param   DWORD
			* @param   TCHAR 
			* @param   set<string>
			* @return  成功返回true，其它返回false
			*/
			static BOOL get_system_process_path(set<string> &s);
			/*
			* @brief   调用GetProcessFullPath(),返回进程全路径
			* @param   set<string>
			* @return  成功返回true，其它返回false
			*/
			static BOOL get_system_service_path(set<string> &s);
			/*
			* @brief   返回服务路径
			* @param   set<string>
			* @return  成功返回true，其它返回false
			*/
			static BOOL IsWow64();
			/*
			* @brief   判断是否为wow64
			* @return  成功返回true，其它返回false
			*/
			static BOOL QueryKey(HKEY hKey, set<string> &s);
			/*
			* @brief   打开注册表，获取启动项路径
			* @param   HKEY 根键
			* @param   set<string>
			* @return  成功返回true，其它返回false
			*/
			static BOOL get_system_start_path(set<string> &s);
			/*
			* @brief   调用QueryKey()
			* @param   set<string>
			* @return  成功返回true，其它返回false
			*/
		}; 
	}
}