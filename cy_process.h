#ifndef  CY_PROCESS_H
#define CY_PROCESS_H
#include<string>

namespace cy{
namespace  process{

class ProcessBuilder{
public :
//使用此方法创建一个进程
static HANDLE Start(std::string command,int bshow = false);
static HANDLE Start(std::string command,
					HANDLE in/*重定向标准输入*/,
					HANDLE out/*重定向标准输出*/,HANDLE error/*重定向标准错误流*/,int bshow = false);

//kill掉在某个目录下的所有进程
static  void KillProcessInDir(const char* dir,const char* ignore);
};

}//end of process

}//end of cy
#endif