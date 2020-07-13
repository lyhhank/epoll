/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		drv_socket.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  socket 收发驱动
**********************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "drv_socket.hpp"

namespace lmSOCKET
{
    DRV_SOCKET::DRV_SOCKET()
    {
        m_ret = OK;
    }

    DRV_SOCKET::~DRV_SOCKET()
    {       
        
    }

    int DRV_SOCKET::UdpConnectReq(const int fd)
    {
        // 请求端口
        char reqbuf[2];
        reqbuf[0] = MSG_UDP_REQ;
        m_ret = Send(fd, reqbuf, 1, MSG_DONTWAIT);
        if(m_ret == FAIL)
        {
            logerr("UdpConnectReq Send fail");
            ::close(fd);
            return FAIL;
        }

        unsigned int tryTimes = 0;
TRYRECV:
        ::sleep(1); // 如果1秒后还没有成功返回，就失败了, 客户端可以作延时
        m_ret = Recv(fd, reqbuf, 1, MSG_DONTWAIT);
        if(m_ret == FAIL)
        {
            logerr("UdpConnectReq Recv fail");
            if(tryTimes++ >= TRYTIMES)
            {
                ::close(fd);
                return FAIL;
            }
            else
            {
                goto TRYRECV;
            }
        }
        
        if(reqbuf[0] != MSG_UDP_RSP)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }

    int DRV_SOCKET::Create(const int index)
    {
        int fd = -1;
        
        if(IsServer())
        {
            fd = CreateServer(index);
            if(fd > 0)
            {
                SetDevFd(index, fd);
            }
        }
        else
        {            
            fd = CreateClient(index); // 获取端口FD
            if(fd > 0)
            {
                SetDevFd(index, fd);

                if(IsUdp(index))
                {
                    if(UdpConnectReq(fd) != OK)
                    {
                        SetDevFd(index, -1);

                        ::close(fd);
                        return FAIL;
                    }
                }
            }
        }

        return fd;
    }

    int DRV_SOCKET::Recv(const int fd, char *recvbuf, const int recvlen, const int flag)
    {
        return ::recv(fd, recvbuf, recvlen, flag);
    }

    int DRV_SOCKET::Send(const int fd, char *sendbuf, const int sendlen, const int flag)
    {   
        return ::send(fd, sendbuf, sendlen, flag); // 发送
    }
    
    std::string DRV_SOCKET::GetMsgHead(void) const 
    {
        return "M:";
    }
    
    std::string DRV_SOCKET::GetFileHead(void) const 
    {
        return "F:";
    } 
}
