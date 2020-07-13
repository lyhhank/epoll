/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		epoll_recv.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  EPOLL接收
**********************************************************************************/


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <string.h>
#include <sys/time.h>
#include <signal.h>

#include "epoll.hpp"
#include <set>

namespace lmSOCKET
{
    int CEPOLL::GetMsgType(void)
    {
        m_pRecvBuf->m_pbuf->msgType = (msgType_e)(*(m_pRecvBuf->m_pbuf->pbuf));
        if((m_pRecvBuf->m_pbuf->msgType == MSG_MSG) || (m_pRecvBuf->m_pbuf->msgType == MSG_FILE) ||
            (m_pRecvBuf->m_pbuf->msgType == MSG_FILE_END))
        {
            return OK;
        }
        else
        {
            return FAIL;
        }
    }

    int CEPOLL::MoveData(void)
    {
        if(m_pRecvBuf->m_pbuf->pbuf != m_pRecvBuf->m_pbuf->buf)
        {
            ::memcpy(m_pRecvBuf->m_pbuf->buf, m_pRecvBuf->m_pbuf->pbuf, m_pRecvBuf->m_pbuf->pbuf-m_pRecvBuf->m_pbuf->buf);
            m_pRecvBuf->m_pbuf->pbuf = m_pRecvBuf->m_pbuf->buf;
        }
        return CON;
    }

    int CEPOLL::DealData(int fd)
    {
        if(m_haveHead == 0)
        {
            RecvCs(fd, m_pRecvBuf->m_pbuf->pbuf, m_pRecvBuf->m_pbuf->len, MSG_NOTYPE);
            return OK;
        }
        else
        {
            while(1)
            {
                if(m_pRecvBuf->m_pbuf->len == 0)
                {
                    return OK;
                }
            
                if(GetMsgType() == FAIL)
                {
                    log("GetMsgType err, len=%d, type=%c", m_pRecvBuf->m_pbuf->len, m_pRecvBuf->m_pbuf->msgType);
                    return FAIL;
                }
                
                std::string  msghead;
                GetHead(m_pRecvBuf->m_pbuf->msgType, msghead);
                
                unsigned int headlen = msghead.size();
                if(::memcmp(m_pRecvBuf->m_pbuf->pbuf, msghead.c_str(), headlen) != 0)
                {
                    log("Msg Head err");
                    return FAIL;
                }
                
                headlen += MSG_LEN_BYTE;
                if(m_pRecvBuf->m_pbuf->len < headlen)
                {
                    return MoveData();
                }
                else if(m_pRecvBuf->m_pbuf->len == headlen)
                {
                    if(m_pRecvBuf->m_pbuf->msgType == MSG_FILE_END)
                    {
                        RecvCs(fd, NULL, 0, m_pRecvBuf->m_pbuf->msgType); // 只是通知文件传送完成
                        return OK;
                    }
                    else
                    {
                        return MoveData();
                    }
                }
                else // 接收数据长度 > 消息长度
                {
                    unsigned int datalen = *((unsigned short *)(m_pRecvBuf->m_pbuf->pbuf + headlen - MSG_LEN_BYTE));
                    if(m_pRecvBuf->m_pbuf->len - headlen < datalen)
                    {
                        return MoveData();
                    }
                    else if(m_pRecvBuf->m_pbuf->len - headlen == datalen)
                    {
                        RecvCs(fd, m_pRecvBuf->m_pbuf->pbuf + headlen, datalen, m_pRecvBuf->m_pbuf->msgType); // 没有粘包
                        return OK;
                    }
                    else // 粘包
                    {
                        RecvCs(fd, m_pRecvBuf->m_pbuf->pbuf + headlen, datalen, m_pRecvBuf->m_pbuf->msgType);
                        m_pRecvBuf->m_pbuf->pbuf += (headlen + datalen);
                        m_pRecvBuf->m_pbuf->len -= (headlen + datalen);
                        
                        continue;
                    }
                }
            }
        }
    }

    // 只返回OK, FAIL, CON处理中可能返回
    int CEPOLL::RecvData(const int fd)
    {       
        int n = 0;
        while(1)
        {
            n = Recv(fd, m_pRecvBuf->m_pbuf->pbuf + m_pRecvBuf->m_pbuf->len, BUF_LEN - m_pRecvBuf->m_pbuf->len, MSG_DONTWAIT);
            if(n > 0)
            {
                m_pRecvBuf->m_pbuf->len += n;
                if(m_pRecvBuf->m_pbuf->len >= BUF_LEN) // 超出
                {
                    return OK;
                }
            }
            else if(n == 0) // 出错
            {
                Offline(fd);
                return FAIL;
            }
            else // 中断
            {
                if(errno == EINTR)
                {
                    continue;
                }
                else if((errno == EAGAIN) || (errno == EWOULDBLOCK)) // 接收完成，至少缓冲区已经无包
                {
                    return CON;
                }
                else 
                {
                    Offline(fd);
                    return FAIL; 
                }
            }
        }
    }

    int CEPOLL::RecvConnect(void)
    {
        int devfd = GetDevFd(m_index);
        if(devfd > 0)
        {
            return OK;
        }
        else
        {
            int fd = Create(m_index);
            if(fd <= 0)
            {
                log("RecvConnect Create err");
                return FAIL;
            }
            
            devfd = GetDevFd(m_index);

            std::string ip;
            unsigned int port;
            if(GetDevType(m_index) == SOCKET)
            {
                socket_t socket = GetSocketInfo(m_index);
                ip = socket.addr;
                port = socket.port;
            }
            else
            {
                com_t com = GetComInfo(m_index);
                ip = com.dev;
                port = 0;
            }

            return Online(devfd, ip, port);
        }
    }

    int CEPOLL::RecvInit(void)
    {
        if(IsServer())
        {
            if(m_timerFd <= 0)
            {
                if(TimerInit() != OK)
                {
                    log("TimerInit err");
                    return FAIL;
                }
                else
                {
                    if(m_pRecvEp->AddRecv(m_timerFd) != OK) // 接收FD加入EPOLL
                    {
                        log("AddRecv err");
                        return FAIL;
                    }
                    
                    // FD 由ACCEPT加入
                }
            }
        }
        else // 客户端
        {
            if(RecvConnect() != OK)
            {
                return FAIL;
            }
        }

        int RecvFd = GetRecvFd(m_index);
        if(RecvFd <= 0)
        {
            log("GetRecvFd fail, index = %d", m_index);
            return FAIL;
        }
        else
        {
            if(m_pRecvEp->AddRecv(RecvFd) != OK) // 接收FD加入EPOLL
            {
                log("AddRecv err");
                return FAIL;
            }
        }

        return OK;
    }

    int CEPOLL::RecvCs(const int fd, char *buf, const int len, const msgType_e type)
    {
        RecvUpdate(fd);
        
        if(IsServer())
        {
            return ServerRecv(fd, buf, len, type); // USER 处理
        }
        else
        {
            int transIndex = GetTransIndex(m_index);
            if(transIndex != FAIL) // 需要转发
            {
                return SendFd(transIndex, TRANS_MSG, 0, buf, len);
            }
            else
            {
                return ClientRecv(fd, buf, len, type); // USER 处理
            }
        }
    }

    int CEPOLL::EpollRecv(void)
    {
EpollRecvInit:
        if(RecvInit() != OK)
        {
            log("RecvInit err");
            goto EpollRecvInit;
        }
        log("EpollRecv index = %d", m_index);
        
        while(1)
        {
            int nfds = FAIL;
            if(IsServer())
            {
                nfds = m_pRecvEp->Wait();
            }
            else
            {
                if(GetDevType(m_index) == SOCKET)
                {
                    nfds = m_pRecvEp->Wait(GetSocketInfo(m_index).timeout);
                }
                else if(GetDevType(m_index) == COM)
                {
                    nfds = m_pRecvEp->Wait(GetComInfo(m_index).timeout);
                }
            }

            if(nfds == 0) // 超时
            {
                if(GetDevFd(m_index) <= 0)
                {
                    if(RecvConnect() != OK) // SERVER 不会超时
                    {
                        ;
                    }
                }
                else
                {
                    ClientTimeout(GetDevFd(m_index));
                }
                continue;
            }

            for(int i = 0; i < nfds; i++)
            {
                int evfd = m_pRecvEp->GetEvFd(i);
                int ev = m_pRecvEp->GetEv(i);
                
                if(evfd == m_timerFd)
                {
                    TimeoutSec(evfd);
                }
                else if(evfd == GetRecvFd(m_index)) // 取进程描述符
                {
                    RecvTransMsg();
                }
                else if(ev & (EPOLLIN | EPOLLPRI)) // 接收数据
                {
                    m_pRecvBuf->m_pbuf = m_pRecvBuf->GetBuf(evfd);
                    if(m_pRecvBuf->m_pbuf == NULL)
                    {
                        log("EpollRecv GetBuf null");
                        continue;
                    }

                    m_ret = RecvData(evfd);
                    if(m_ret == OK)
                    {
                        m_ret = DealData(evfd);
                        if(m_ret == OK)
                        {
                            m_pRecvBuf->FreeBuf(evfd);
                        }
                        else if(m_ret == CON)
                        {
                            m_pRecvBuf->SetBufTime();
                        }
                        else
                        {
                            m_pRecvBuf->FreeBuf(evfd);
                        }

                        m_pRecvEp->ModRecv(evfd);
                    }
                    else if(m_ret == CON)
                    {
                        m_ret = DealData(evfd);
                        if(m_ret == OK)
                        {
                            m_pRecvBuf->FreeBuf(evfd);
                        }
                        else if(m_ret == CON)
                        {
                            m_pRecvBuf->SetBufTime();
                        }
                        else
                        {
                            m_pRecvBuf->FreeBuf(evfd);
                        }
                    }
                    else
                    {
                        m_pRecvBuf->FreeBuf(evfd);
                    }
                }
            }
        }

        log("EpollRecv exit");
        return FAIL;
    }

    int CEPOLL::TimerInit(void)
    {
        struct itimerspec time_intv; //用来存储时间
        m_timerFd = ::timerfd_create(CLOCK_REALTIME, 0);   //创建定时器
        if(m_timerFd == FAIL)
        {
            logerr("TimerInit fail");
            return FAIL;
        }

        int time_int;
        int time_float;
        float time = GetTimeOut(m_index) / CLIENT_TIMEOUT_BASE;
        time_int = (int)time;
        time_float = (time - time_int) * 1000000;

        time_intv.it_value.tv_sec = time_int; //设定超时
        time_intv.it_value.tv_nsec = time_float;
        time_intv.it_interval.tv_sec = time_intv.it_value.tv_sec;
        time_intv.it_interval.tv_nsec = time_intv.it_value.tv_nsec;

        m_ret = ::timerfd_settime(m_timerFd, 0, &time_intv, NULL);  //启动定时器
        
        return m_ret;
    }

    void CEPOLL::TimeoutSec(const int fd)
    {
        uint64_t value;
        m_ret = ::read(fd, &value, sizeof(uint64_t));
        if(m_ret == FAIL)
        {
            logerr("TimeoutSec read fail");
            return;
        }

        m_curSlot++;
        if(m_curSlot >= CLIENT_TIMEOUT_BASE)
        {
            m_curSlot = 0;
        }

        if(m_online[m_curSlot].size() > 0) // 到这里就该清除了
        {
            for(m_iterOnline = m_online[m_curSlot].begin(); m_iterOnline != m_online[m_curSlot].end(); m_iterOnline++)
            {
                log("TimeoutSec offline");
                Offline(m_iterOnline->first);
            }
            m_online[m_curSlot].clear();            
        }
    }

}


