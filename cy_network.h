#pragma once

#include <string.h>
#include<stdio.h>
#include<errno.h>


namespace cy{
namespace io{
#define TIMEOUT 20
class   NetWork
{
public:
	NetWork(void);
	virtual ~NetWork(void);

	static int host2ip(const char * name/*in*/,struct in_addr * ip/*out*/);
	static struct in_addr nn_resolve(const char * name);
	//����Ϊ������
	static int unblock_socket(int soc);
	//���������׽���
	static int block_socket(int soc);
	//
	static int tcp_init();
	//�ر��׽���
	static int socket_close(int soc);
	//�ܵõ������������ж����ֽ�Ҫ����ȡ��ֵ���� nread������
	static int data_left(int soc);
	//�󶨲������˿�
    static bool TryBindAndListenPort(int& port/*[in][out]����ʵ�ʼ����˿�*/,int& sock/*[out]�����׽���*/,bool localhost = false);
	/**
	*���Ӳ���API
	*/

	//������������
	static int open_sock_opt_hn(
	 const char * hostname,
	 unsigned int port,
	 int type,
	 int protocol,
	 int timeout);
	//ͨ��TCP���������˿�
	static int open_sock_tcp_hn(const char * hostname, unsigned int port);
    static int open_socket(struct sockaddr_in *paddr, 
	    int port, int type,
		int protocol, int timeout);
	static int open_sock_tcp_ip(const char * ip, unsigned int port);
	
	/**
	����������API
	**/
   static int recv_line( int soc,char * buf,size_t bufsiz);
   static int nrecv (int fd,void * data,int length,int  i_opt);
   static int os_recv(int soc, void * buf, int len, int opt );
   /**
	дAPI
   */
   static int nsend( int fd,void * data,int length, int i_opt);
   static int os_send(int soc, void * buf, int len, int opt );

};

}
}