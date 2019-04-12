#include <winsock2.h>
#include<windows.h>
#include <ws2tcpip.h>
#include "cy_network.h"
namespace cy{
namespace io{

NetWork::NetWork(void){
}

NetWork::~NetWork(void){
}

int NetWork::host2ip(const char * name/*in*/,struct in_addr * ip/*out*/){
		struct hostent * ent;

		ent = gethostbyname(name);
		if(!ent){
			return -1;
		}else if(ip){
			memcpy(ip, ent->h_addr, ent->h_length);
		}
		return 0; /* success */
}

struct in_addr NetWork::nn_resolve(const char * name){
		struct in_addr ret;
		if(host2ip(name, &ret)  < 0)
		{
			ret.s_addr = INADDR_NONE;
		}
		return ret;
   }

int NetWork::unblock_socket(int soc){
	  int l = 1;
	  int flags = ioctlsocket(soc, FIONBIO, (unsigned long *)&l);
	  if (flags < 0)
		{
		  perror("ioctlsocket(FIONBIO)");
		  return -1;
		}
	  return 0;
}

int NetWork::block_socket(int soc){
	  int l = 0;
	  int flags = ioctlsocket(soc, FIONBIO, (unsigned long *)&l);
	  if (flags < 0)
		{
		  perror("ioctlsocket(~FIONBIO)");
		  return -1;
		}
	  return 0;
}

//�ܵõ������������ж����ֽ�Ҫ����ȡ��ֵ���� nread������
 int NetWork::data_left(int soc){
	 int len = 0;
	 int ret = ioctlsocket(soc, FIONREAD, (unsigned long *)&len);
	 if (ret >= 0) {
	  if(len <= 0)
	   return 0;
	  else 
	   return len;
	 }else {
	  return -1;
	 }
} /* data_left */


int NetWork::open_sock_tcp_hn(const char * hostname, unsigned int port){
	   return  open_sock_opt_hn(hostname, port, SOCK_STREAM, IPPROTO_TCP, 20/*���ӳ�ʱ20��*/);
}

int NetWork::open_sock_tcp_ip(const char * ip, unsigned int port){
	  struct sockaddr_in addr;
	
	  memset((void*)&addr,0, sizeof(addr));
	  addr.sin_family=AF_INET;
	  addr.sin_port=htons((unsigned short)port);
	  addr.sin_addr.s_addr = inet_addr(ip);
	  return open_socket(&addr, port, SOCK_STREAM, IPPROTO_TCP, 20);
}

int NetWork::open_sock_opt_hn(
	 const char * hostname,
	 unsigned int port,
	 int type,
	 int protocol,
	 int timeout)
	{
	  struct sockaddr_in addr;
	  
	  memset((void*)&addr,0, sizeof(addr));
	  addr.sin_family=AF_INET;
	  addr.sin_port=htons((unsigned short)port);
	  addr.sin_addr = nn_resolve(hostname);
	  if (addr.sin_addr.s_addr == INADDR_NONE || addr.sin_addr.s_addr == 0)
		{
		  fprintf(stderr, "open_sock_opt_hn: invalid socket address\n");
		  return  -1;
		}
	   
	   return open_socket(&addr, port, type, protocol, timeout);
}


int NetWork::open_socket(struct sockaddr_in *paddr, 
	    int port, int type,
		int protocol, int timeout)
   {
	 fd_set		fd_w;
	 struct timeval	to;
	 int			soc, x;
	 int			opt, opt_sz;
	 DWORD         err = 0;

	// __port_closed = 0;
	 //����һ���׽���
	 if ((soc = socket(AF_INET, type, protocol)) < 0){
		//nessus_perror("socket");
		return -1;
	  }

	if (timeout == -2){
      timeout = TIMEOUT;
    }

   if (timeout > 0){
	 //��������ӳ�ʱ���Ƚ�������Ϊ������
    if (unblock_socket(soc) < 0){
			closesocket(soc);
			return -1;
     }
  }
  //set_socket_source_addr(soc, 0);
  //����Ŀ��
  if (connect(soc, (struct sockaddr*) paddr, sizeof(*paddr)) < 0){
      err = WSAGetLastError();
again:
      switch (err){
		//case EINPROGRESS:
		case WSAEWOULDBLOCK:
		case EAGAIN:
			FD_ZERO(&fd_w);
			FD_SET(soc, &fd_w);
			to.tv_sec = timeout;
			to.tv_usec = 0;
			x = select(soc + 1, NULL, &fd_w, NULL, &to);
			err = WSAGetLastError();
			if (x == 0){
				 socket_close(soc);
				 errno = WSAETIMEDOUT;
				 return -1;
			 }
			else if (x < 0){
				if ( err == WSAEINTR ){
 					err = EAGAIN;
					goto again;
				}
				socket_close(soc);
				return -1;
         }
		 opt = 0; opt_sz = sizeof(opt);
		 if (getsockopt(soc, SOL_SOCKET, SO_ERROR, (char*)&opt, &opt_sz) < 0)
	     {
	       //nessus_perror("getsockopt");
	       socket_close(soc);
	       return -1;
	     }
	    if (opt == 0)
	      break;  
	default:
	 // __port_closed = 1;
	  socket_close(soc);
	  return  -1;
	}
    }
  //���½��׽�������Ϊ����ģʽ
   block_socket(soc);
  return soc;
}

int NetWork::socket_close(int soc){
  return closesocket(soc);
}

int   NetWork::nrecv (int fd,void * data,int length,int  i_opt){
  int e;
  //����Ϊ����
  block_socket(fd);
  /*
  ���ĸ�������
	MSG_PEEK �鿴��ǰ���ݡ����ݽ������Ƶ��������У������������������ɾ����
	MSG_OOB �����������
  */
  do {
	/*
	���޴�������recv()���ض�����ֽ�����
	�����������ֹ������0������Ļ�������SOCKET_ERROR����
	Ӧ�ó����ͨ��WSAGetLastError()��ȡ��Ӧ������롣
	*/
	e = recv(fd, (char*)data, length, i_opt);
  } while ( e < 0 && WSAGetLastError() == WSAEINTR );
  //��Ϊ������
  unblock_socket(fd);
  return e;
}

int NetWork::os_recv(int soc, void * buf, int len, int opt ){
	 char * buf0 = (char*)buf;
	 int e, n;
	 for ( n = 0 ; n < len ; ) {
		  errno = 0;
		  e = recv(soc, buf0 + n , len -  n, opt);
		  if ( e < 0 && WSAGetLastError() == WSAEINTR ){
			  continue; 
		  }else if ( e <= 0 ){
			  printf("[error]os_recv-------------�������ݳ���\n");
			  printf("os_recv e= %d,len=%d,errono =%d\n",e, len -  n,WSAGetLastError() );
			  return -1;
		  }else{
			  n += e;
		  }
	 }
	 return n;
}

int NetWork::recv_line( int soc,char * buf,size_t bufsiz){
   int n, ret = 0;
   fd_set rd;
   struct timeval tv;
   do
   {
      int e;
 again:
      errno = 0;
      FD_ZERO(&rd);
      FD_SET(soc, &rd);
      tv.tv_sec = 5;
      tv.tv_usec = 0;
      e = select(soc+1, &rd, NULL, NULL, &tv); 
	  if( e < 0 && WSAGetLastError() == WSAEINTR){
		  goto again;
	  }
      if( e > 0 ){
		   n = recv(soc, buf + ret, 1, 0);
		   switch(n)
		   {
			case -1 :
				 if ( WSAGetLastError() == WSAEINTR ) continue;
				 if(ret == 0){
					return -1;
				 }else{
					 return ret;
				 }
				 break;  
		   case 0 :
			     return ret;
       		     break;
		   default:
			     ret ++;	
		   }
      } else{ 
		  break;
	  }
      tv.tv_sec = 1;
      tv.tv_usec = 0;
    } while(buf[ret -1 ] != '\0' && buf[ret -1 ] != '\n' && ret < bufsiz);
    
    if(ret > 0){
		if(buf[ret - 1] != '\0'){
		if ( ret < bufsiz )
	      	buf[ret] = '\0';
		else
			buf[bufsiz - 1] = '\0';
		}
    }
    return ret;
}

int NetWork::tcp_init()
{
    WSADATA wsa;

    if(WSAStartup(MAKEWORD(1, 1), &wsa) != 0)
        return -1;
    //wsa_ok = 1;
    return 0;
}

int NetWork::nsend( int fd,void * data,int length, int i_opt){
  int		n = 0;
  DWORD     err = 0;
  int e;
  struct timeval tv = {0,5};
  fd_set wr;
  do
   {
      FD_ZERO(&wr); 
	  FD_SET(fd, &wr);
	  /*
		select()���÷��ش��ھ���״̬�����Ѿ�������fd_set�ṹ�е�������������
		�����ʱ�򷵻�0������Ļ�������SOCKET_ERROR����
		Ӧ�ó����ͨ��WSAGetLastError��ȡ��Ӧ������롣
	  */
      e  = select(fd + 1, NULL, &wr, NULL, &tv);
	  if ( e > 0 ){
	       n = os_send(fd, data, length, i_opt);
		   /*
		   	n < 0 �Ƿ���Ҫ�ط������⣿
			����ֱ�Ӷ���
		   */
		   break;
       }else if(e == 0){
		   /*
				����ϵͳ��������������ʱ�ȴ�
		   */
	       continue;
	   }else{
		   err = WSAGetLastError();
		   fprintf(stderr, "nsend():send %s,WSAGetLastError =%d\n",  strerror(errno),err);
	   	   break;
	   }
     }while (true);
   return n;
 }
 
int NetWork::os_send(int soc, void * buf, int len, int opt ){
 char * buf0 = (char*)buf;
 int e, n;
 for ( n = 0 ; n < len ; ){
	  errno = 0;
	  /*
	          ���޴�������send()�������������ݵ�����,
	  ��ע��������ֿ���С�� len�����涨�Ĵ�С��
	          ����Ļ�������SOCKET_ERROR����
	  Ӧ�ó����ͨ��WSAGetLastError()��ȡ��Ӧ������롣
	  */
	  e = send(soc, buf0 + n , len -  n, opt);
	  if ( e < 0 && WSAGetLastError() == WSAEINTR ){
		  continue; 
	  }else if ( e <= 0 ) {
		  printf("[os_send]e = %d,errno = %d,WSAGetLastError() = %d \n",e,errno,WSAGetLastError());
		  return -1;
	  }else{
		  n += e;
	  }
 }
 return n;
}

bool NetWork::TryBindAndListenPort(int& port,int& sock,bool localhost){
		bool bret =FALSE;
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		if(localhost){
			//�ܾ���������
			inet_pton(AF_INET,"127.0.0.1",&sin.sin_addr);
		}else{
			sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
		sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		//���׽��ֵ����ػ���
		//����20�ΰ󶨶˿�
		int tries = 40;
		while(--tries){
			if(::bind(sock,(sockaddr*)&sin,sizeof(sin)) == SOCKET_ERROR){
				port++; //�����������ö˿�
				sin.sin_port = htons(port);
			}else{
				//�˿ڰ󶨳ɹ����˳�ѭ��
				bret = TRUE;
				break;
			}
		}
		if(bret){
			::listen(sock,3);
		    return  bret;
		}
		return  bret;
}
}
}