/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		epoll.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  EPOLL 管理程序
    上线
    下线
**********************************************************************************/


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <string.h>
#include <sys/time.h>
#include <signal.h>

#include "epoll.hpp"

namespace lmSOCKET
{
	CEPOLL::CEPOLL()
	{
        m_ret = OK;
        m_index = 0;

        m_pAcceptEp = new CEP();
        if(m_pAcceptEp == NULL)
        {
            m_ret = FAIL;
        }
        
        m_pRecvEp = new CEP();
        if(m_pRecvEp == NULL)
        {
            m_ret = FAIL;
        }
        
        m_pSendEp = new CEP();
        if(m_pSendEp == NULL)
        {
            m_ret = FAIL;
        }

        m_pRecvBuf = new CBUFFER();
        if(m_pRecvBuf == NULL)
        {
            m_ret = FAIL;
        }

        m_pSendBuf = new CBUFFER();
        if(m_pSendBuf == NULL)
        {
            m_ret = FAIL;
        }

        for(int i = 0; i < GetCpuNum(); i++)
        {
            m_FdNumInLoad.push_back(std::pair<int, int>(i, 0));
        }
        
        m_SameIpLimit.clear();

        m_curSlot = 0;
        for(unsigned int i = 0; i < CLIENT_TIMEOUT_BASE; i++)
        {
            m_online[i].clear();
        }

        m_haveHead = 0;

        m_timerFd = 0;
	}

	CEPOLL::~CEPOLL()
	{
        if(m_pAcceptEp)
        {
            delete m_pAcceptEp;
            m_pAcceptEp = NULL;
        }
        if(m_pRecvEp)
        {
            delete m_pRecvEp;
            m_pRecvEp = NULL;
        }
        if(m_pSendEp)
        {
            delete m_pSendEp;
            m_pSendEp = NULL;
        }

        if(m_pSendBuf)
        {
            delete m_pSendBuf;
            m_pSendBuf = NULL;
        }
        
        if(m_pRecvBuf)
        {
            delete m_pRecvBuf;
            m_pRecvBuf = NULL;
        }

        m_FdNumInLoad.clear();
        m_SameIpLimit.clear();

        m_curSlot = 0;
        for(unsigned int i = 0; i < CLIENT_TIMEOUT_BASE; i++)
        {
            m_online[i].clear();
        }
	}

    void CEPOLL::IgSignal(void) const
    {
        ::signal(SIGPIPE, SIG_IGN);
        ::signal(SIGHUP, SIG_IGN);
    }

    int CEPOLL::EpollUserTask(void) 
    {
        IgSignal();
        
        log("EpollUserTask index = %d",m_index);
        while(1)
        {
            ClientTask();
        }

        log("EpollUserTask exit");
        return FAIL;
    }

    int CEPOLL::ClientGetFd(void) const 
    {
        return GetDevFd(m_index);
    }

    int CEPOLL::MsgHeadInit(void)
    {
        m_haveHead = 0;
        if(GetMsgHead().size() != 0)
        {
            m_haveHead++;
            m_msgHead.push_back(MSG_MSG);
            m_msgHead += GetMsgHead();
        }

        if(GetFileHead().size() != 0)
        {
            m_haveHead++;
            m_fileHead.push_back(MSG_FILE);
            m_fileHead += GetMsgHead();

            m_fileEnd.push_back(MSG_FILE_END);
            m_fileEnd += GetMsgHead();
        }

        if(m_haveHead != 0 && m_haveHead != 2)
        {
            log("need all head or all not head %d!", m_haveHead);
            
            return FAIL;
        }

        return OK;
    }

    void CEPOLL::GetHead(msgType_e type, std::string &head)
    {
        if(type == MSG_MSG)
        {
            head = m_msgHead;
        }
        else if(type == MSG_FILE)
        {
            head = m_fileHead;
        }
        else
        {
            head = m_fileEnd;
        }
    }

    int CEPOLL::PthOffline(const int fd)
    {
        m_ret = m_pRecvEp->Del(fd);
        if(m_ret != OK)
        {
            return FAIL;
        }

        m_ret = m_pSendEp->Del(fd);
        if(m_ret != OK)
        {
            return FAIL;
        }

        ::shutdown(fd, SHUT_WR);
        ::close(fd);
        
        if(! IsServer())
        {
            SetDevFd(m_index, -1);
        }

        return OK;
    }

    int CEPOLL::ForkOffline(const int fd)
    {
        m_ret = m_pRecvEp->Del(fd); // 只关一次，一个EPOLL对象
        if(m_ret != OK)
        {
            return FAIL;
        }

        m_ret = m_pSendEp->Del(fd);
        if(m_ret != OK)
        {
            return FAIL;
        }

        ::shutdown(fd, SHUT_WR);
        ::close(fd);

        if(! IsServer())
        {
            SetDevFd(m_index, -1);
        }
        
        return OK;
    }

    int CEPOLL::Offline(const int fd)
    {
        log("index = %d, Offline fd = %d", m_index, fd);

        m_iterFdIndex = m_fdIndex.find(fd);
        if(m_iterFdIndex != m_fdIndex.end())
        {
            int index = GetFdIndex(fd, FAIL, OK);
            if(index == FAIL)
            {
                log("Offline GetFdIndex err");
                // 不返回
            }
            else
            {
                OfflineUpdate(fd, index);
            }
        }
        else
        {
            log("Offline m_iterOnline find err");
            m_ret = FAIL;
        }

        if(GetRunType() == FORK)
        {
            m_ret = ForkOffline(fd);
        }
        else
        {
            m_ret = PthOffline(fd);
        }

        return m_ret;
    }

    int CEPOLL::PthOnline(const int fd)
    {
        m_ret = m_pRecvEp->AddRecv(fd);
        if(m_ret != OK)
        {
            ::shutdown(fd, SHUT_WR);
            ::close(fd);
            return FAIL;
        }
        m_ret = m_pSendEp->AddSend(fd);
        if(m_ret != OK)
        {
            m_pRecvEp->Del(fd);
            ::shutdown(fd, SHUT_WR);
            ::close(fd);
            return FAIL;
        }
        
        return OK;
    }

    int CEPOLL::ForkOnline(const int fd)
    {
        m_ret = m_pRecvEp->AddRecv(fd);
        if(m_ret != OK)
        {
            ::shutdown(fd, SHUT_WR);
            ::close(fd);
            return FAIL;
        }
        
        m_ret = m_pSendEp->AddSend(fd);
        if(m_ret != OK)
        {
            m_pRecvEp->Del(fd);
            ::shutdown(fd, SHUT_WR);
            ::close(fd);
            return FAIL;
        }

        return OK;
    }

    int CEPOLL::Online(const int fd, const std::string &ip, const unsigned int port)
    {
        log("index = %d, Online fd = %d, ip = %s, port = %d", m_index, fd, ip.c_str(), port);
        
        if(GetRunType() == FORK)
        {
            m_ret = ForkOnline(fd);
        }
        else
        {
            m_ret = PthOnline(fd);
        }

        if(m_ret == OK)
        {
            OnlineUpdate(fd, ip, port);
        }

        return m_ret;
    }

}


