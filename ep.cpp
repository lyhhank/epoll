/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		ep.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  EPOLL 基本函数封装
**********************************************************************************/


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include "ep.hpp"

namespace lmSOCKET
{
	CEP::CEP()
	{
        m_ret = OK;
        
        m_pevents = new epoll_event[MAXEVENTS]();
        if(m_pevents == NULL)
        {
            log("new m_pevents err");
            m_ret = FAIL;
        }
        
		m_epfd = ::epoll_create1(0);
		if(m_epfd == FAIL)
		{
			logerr("epoll_create1 err");
            m_ret = FAIL;
		}
	}

	CEP::~CEP()
	{
        if(m_pevents)
        {
            delete [] m_pevents;
            m_pevents = NULL;
        }
        
        if(m_epfd > 0)
        {
            ::close(m_epfd);
            m_epfd = 0;
        }
	}

	int CEP::AddRecv(const int fd)
	{
        m_ev.data.fd = fd;
		m_ev.events = EPOLLIN | EPOLLET | EPOLLPRI;
		m_ret = ::epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &m_ev);
		if(m_ret != OK)
		{
			logerr("add err");
		}

		return m_ret;
	}
    
	int CEP::AddSend(const int fd)
	{
        m_ev.data.fd = fd;
		m_ev.events = EPOLLOUT |EPOLLET;

		m_ret = ::epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &m_ev);
		if(m_ret != OK)
		{
			logerr("add err");
		}

		return m_ret;
	}

	int CEP::Del(const int fd)
	{
		m_ret = ::epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
		if(m_ret != OK)
		{
			logerr("Del err");
		}

        /*
		m_ret = ::close(fd);
        if(m_ret != OK)
        {
            logerr("close err");
        }
        */
    
		return m_ret;
	}

	int CEP::Wait(const int timeout)
	{
		m_ret = ::epoll_wait(m_epfd, m_pevents, MAXEVENTS, timeout);
        if(m_ret == FAIL)
        {
            logerr("Wait err");
        }

		return m_ret;
	}

	int CEP::ModRecv(const int fd)
	{
        m_ev.data.fd = fd;
		m_ev.events = EPOLLIN | EPOLLET | EPOLLPRI;

		m_ret = ::epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &m_ev);
		if(m_ret != OK)
		{
			logerr("ModRecv err");
		}

		return m_ret;
	}

	int CEP::ModSend(const int fd)
	{
		m_ev.data.fd = fd;
		m_ev.events = EPOLLOUT |EPOLLET;

		m_ret = ::epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &m_ev);
		if(m_ret != OK)
		{
			logerr("ModSend err");
		}

		return m_ret;
	}


    int CEP::GetEpFd(void) const
	{
        return m_epfd;
    }

	int CEP::GetEvFd(const int n) const
	{
		if(n < MAXEVENTS)
		{
			return m_pevents[n].data.fd;
		}
		else
		{
            log("GetEvFd err,n=%d",n);
			return FAIL;
		}
	}

	int CEP::GetEv(const int n) const
	{
		if(n < MAXEVENTS)
		{
			return m_pevents[n].events;
		}
		else
		{
            log("GetEv err,n=%d",n);
			return FAIL;
		}
	}
}


