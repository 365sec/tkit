#ifndef  CY_PROCESS_H
#define CY_PROCESS_H
#include<string>

namespace cy{
namespace  process{

class ProcessBuilder{
public :
//ʹ�ô˷�������һ������
static HANDLE Start(std::string command,int bshow = false);
static HANDLE Start(std::string command,
					HANDLE in/*�ض����׼����*/,
					HANDLE out/*�ض����׼���*/,HANDLE error/*�ض����׼������*/,int bshow = false);

//kill����ĳ��Ŀ¼�µ����н���
static  void KillProcessInDir(const char* dir,const char* ignore);
};

}//end of process

}//end of cy
#endif