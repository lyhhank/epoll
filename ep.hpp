/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		ep.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  EPOLL 基类，EPOLL事件管理
**********************************************************************************/

#ifndef _EP_HPP
#define _EP_HPP 1

#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.hpp"

/*
	EP类
*/
namespace lmSOCKET
{
	class CEP: virtual public CCONFIG
	{
		public:
			CEP();
            ~CEP();
			int AddRecv(const int fd);
            int AddSend(const int fd);
            int Del(const int fd);
            int Wait(const int timeout = -1);
			int ModRecv(const int fd);
			int ModSend(const int fd);
            int GetEpFd(void) const;
			int GetEvFd(const int n) const;
			int GetEv(const int n) const;

		private:
            int m_ret;
            
            int m_epfd;
            struct epoll_event m_ev;
			struct epoll_event *m_pevents; 

	};
}

#endif


