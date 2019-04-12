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
	//设置为非阻塞
	static int unblock_socket(int soc);
	//设置阻塞套接字
	static int block_socket(int soc);
	//
	static int tcp_init();
	//关闭套接字
	static int socket_close(int soc);
	//能得到缓冲区里面有多少字节要被读取。值放在 nread里面了
	static int data_left(int soc);
	//绑定并监听端口
    static bool TryBindAndListenPort(int& port/*[in][out]返回实际监听端口*/,int& sock/*[out]监听套接字*/,bool localhost = false);
	/**
	*连接部分API
	*/

	//连接主机主机
	static int open_sock_opt_hn(
	 const char * hostname,
	 unsigned int port,
	 int type,
	 int protocol,
	 int timeout);
	//通过TCP连接主机端口
	static int open_sock_tcp_hn(const char * hostname, unsigned int port);
    static int open_socket(struct sockaddr_in *paddr, 
	    int port, int type,
		int protocol, int timeout);
	static int open_sock_tcp_ip(const char * ip, unsigned int port);
	
	/**
	读操作部分API
	**/
   static int recv_line( int soc,char * buf,size_t bufsiz);
   static int nrecv (int fd,void * data,int length,int  i_opt);
   static int os_recv(int soc, void * buf, int len, int opt );
   /**
	写API
   */
   static int nsend( int fd,void * data,int length, int i_opt);
   static int os_send(int soc, void * buf, int len, int opt );

};

}
}