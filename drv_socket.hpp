/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		drv_socket.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  SOCKET驱动
**********************************************************************************/

#ifndef _DRV_SOCKET_HPP
#define _DRV_SOCKET_HPP 1

#include "epoll.hpp"
#include "usrapp.hpp"

/*
	设备初始化类
*/
namespace lmSOCKET
{   
    class DRV_SOCKET : public CEPOLL
    {
    	public:
    		DRV_SOCKET();
    		~DRV_SOCKET();

        protected:
            int Create(const int index);
            int Recv(const int fd, char *recvbuf, const int recvlen, const int flag);
            int Send(const int fd, char *sendbuf, const int sendlen, const int flag);

            std::string GetMsgHead(void) const;
            std::string GetFileHead(void) const;

        private:
            int UdpConnectReq(const int fd);
            
        private:
            int m_ret;
            
    };
}

#endif


