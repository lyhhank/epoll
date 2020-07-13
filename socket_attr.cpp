/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		socket_attr.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  SOCKET ATTR 设置
      SO_REUSEADDR
      SO_REUSEPORT
      TCP_NODELAY
**********************************************************************************/


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include "socket_attr.hpp"

namespace lmSOCKET
{
    CSOCKETATTR::CSOCKETATTR()
    {
        m_ret = OK;
    }

    CSOCKETATTR::~CSOCKETATTR()
    {       
    
    }

    int CSOCKETATTR::SetAttr(int index, int fd)
    {
        CHECKRESULT(m_ret);
        
        if(fd <= 0)
        {
            log("SetAttr err,fd=%d", fd);
            return FAIL;
        }

        socketAttr_t attr = GetConfig()->socketAttr;

        if(attr.reused == ON)
        {
            int yes = 1;
            m_ret = ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
            if(m_ret != OK) 
            {
                logerr("set reused addr err return = %d", m_ret);
                return m_ret;
            }

            if(! IsUdp(index)) // UDP 不设置PORT.
            {
                m_ret = ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
                if(m_ret != OK) 
                {
                    logerr("set reused port err return = %d", m_ret);
                    return m_ret;
                }
            }
        }

        if(! IsUdp(index))
        {
            if(attr.ndelay == ON)
            {
                int yes = 1;
                m_ret = ::setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &yes, sizeof(yes));
                if(m_ret != OK) 
                {
                    logerr("set setsockopt err return = %d", m_ret);
                    return m_ret;
                }
            }
        }
        
        return OK;
    }
}
