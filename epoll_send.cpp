/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		epoll_send.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  EPOLL发送
**********************************************************************************/


#include <fstream>
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
    char *CEPOLL::GetSendBuf(const int fd) const 
    {
        m_pSendBuf->m_pbuf = m_pSendBuf->GetBuf(fd);
        if(m_pSendBuf->m_pbuf == NULL)
        {
            log("GetSendBuf null");
            return NULL;
        }

        unsigned int msgoff = m_msgHead.size();
        if(msgoff)
        {
            msgoff += MSG_LEN_BYTE;
        }
        return m_pSendBuf->m_pbuf->buf + msgoff;
    }

    void CEPOLL::FillHead(msgType_e type)
    {
        if(m_haveHead)
        {
            std::string head;
            GetHead(type, head);

            unsigned int headlen = head.size();
            ::memcpy(m_pSendBuf->m_pbuf->pbuf, head.c_str(), headlen);

            *((unsigned short *)(m_pSendBuf->m_pbuf->pbuf + headlen)) = m_pSendBuf->m_pbuf->len;
            m_pSendBuf->m_pbuf->len += head.size() + MSG_LEN_BYTE;
        }
    }

    int CEPOLL::FillBody(void)
    {
        m_pSendBuf->m_pbuf->len = 0;
    
        std::ifstream in(m_pSendBuf->m_pbuf->fileName, std::ios::in | std::ios::binary);
        if(! in.is_open())
        {
            log("fopen %s, fail", m_pSendBuf->m_pbuf->fileName);
            return FAIL;
        }
        else
        {
            in.seekg(m_pSendBuf->m_pbuf->off, std::ios::beg);
            if(in.good())
            {
                unsigned int headsize = 0;
                if(m_haveHead)
                {
                    std::string head;
                    GetHead(m_pSendBuf->m_pbuf->msgType, head);
                    headsize = head.size() + MSG_LEN_BYTE;
                }
                
                ssize_t cnt = in.read(m_pSendBuf->m_pbuf->pbuf + headsize, USR_BUF_LEN).gcount();
                if(cnt <= 0)
                {
                    in.close();
                    return FAIL;
                }
                else
                {
                    m_pSendBuf->m_pbuf->len = cnt;
                    m_pSendBuf->m_pbuf->off += cnt;
                    in.close();
                    return OK;
                }
            }
            else
            {
                in.close();
                return FAIL;
            }
        }

    }

	int CEPOLL::SendData(const int fd)
	{
        int n = 0;
        unsigned int sendLen = 0;
        while(1)
        {
            n = Send(fd, m_pSendBuf->m_pbuf->pbuf + sendLen, m_pSendBuf->m_pbuf->len - sendLen, MSG_DONTWAIT);
            if(n > 0)
            {
                sendLen += n;
                if(sendLen >= m_pSendBuf->m_pbuf->len) // 发送完成
                {
                    return OK;
                }
            }
            else if(n == 0)
            {
                Offline(fd);
                return FAIL;
            }
            else 
            {
                if(errno == EINTR)
                {
                    continue;
                }
                else if((errno == EAGAIN) || (errno == EWOULDBLOCK)) 
                {
                    continue; // 直到发送完成
                }
                else
                {
                    Offline(fd);
                    return FAIL;
                }
            }
        }
	}

    int CEPOLL::SendInit()
	{
        if(IsServer())
        {
            ; // 服务器不主动发数据
        }
        else
        {
            ; // RecvInit 创建FD;
        }
        
        if(MsgHeadInit() != OK)
        {
            return FAIL;
        }

        return OK;
    }
    
    int CEPOLL::EpollSend(void)
	{
EpollSendInit:
        if(SendInit() != OK)
        {
            log("SendInit err");
            goto EpollSendInit;
        }
        log("EpollSend index = %d",m_index);
        
        while(1)
        {
            int nfds = FAIL;
            nfds = m_pSendEp->Wait();

            for(int i = 0; i < nfds; i++)
            {
                int evfd = m_pSendEp->GetEvFd(i);
                int ev = m_pSendEp->GetEv(i);
                
                if(ev & EPOLLOUT) // 发送文件数据
                {
                    m_pSendBuf->m_pbuf = m_pSendBuf->GetBuf(evfd);
                    if(m_pSendBuf->m_pbuf == NULL)
                    {
                        log("EpollSend GetBuf null");
                        continue;
                    }

                    if(m_pSendBuf->m_pbuf->msgType != MSG_FILE)
                    {
                        m_pSendBuf->FreeBuf(evfd);
                        continue;
                    }
                    
                    if(FillBody() == FAIL)
                    {
                        m_pSendBuf->m_pbuf->msgType = MSG_FILE_END;
                    }
                    FillHead(m_pSendBuf->m_pbuf->msgType);

                    if(SendData(evfd) == OK)
                    {
                        if(m_pSendBuf->m_pbuf->msgType != MSG_FILE_END)
                        {
                            m_pSendEp->ModSend(evfd);
                        }
                        else
                        {
                            m_pSendBuf->FreeBuf(evfd);
                        }
                    }
                    else
                    {
                        m_pSendBuf->FreeBuf(evfd);
                    }
                }
            }
        }

        log("EpollSend exit");
        return FAIL;
    }

    int CEPOLL::StartSendMsg(const int fd, const int len)
    {
        if(m_pSendBuf->m_pbuf == NULL)
        {
            log("StartSendMsg buf is NULL");
            return FAIL;
        }

        if(len <= 0)
        {
            m_pSendBuf->FreeBuf(fd);
            log("StartSendMsg len is 0");
            return FAIL;
        }
        
        m_pSendBuf->m_pbuf->msgType = MSG_MSG;
        m_pSendBuf->m_pbuf->len = len;

        FillHead(m_pSendBuf->m_pbuf->msgType);
        m_ret = SendData(fd);
        m_pSendBuf->FreeBuf(fd);

        return m_ret;
    }
    
    int CEPOLL::StartSendFile(const int fd, const std::string &fileName, const size_t offset)
    {
        m_pSendBuf->m_pbuf = m_pSendBuf->GetBuf(fd);
        if(m_pSendBuf->m_pbuf == NULL)
        {
            log("StartSendFile buf is NULL");
            return FAIL;
        }

        if(fileName.size() == 0)
        {
            log("StartSendFile fileName is NULL");
            m_pSendBuf->FreeBuf(fd);
            return FAIL;
        }

        m_pSendBuf->m_pbuf->msgType = MSG_FILE;
        m_pSendBuf->m_pbuf->len = 0;
        m_pSendBuf->m_pbuf->off = offset;
        
        ::memcpy(m_pSendBuf->m_pbuf->fileName, fileName.c_str(), fileName.size());

        return m_pSendEp->ModSend(fd);
    }

    int CEPOLL::GetSlotIndex(void)
    {
        int index;
        if(m_curSlot == 0)
        {
            index = CLIENT_TIMEOUT_BASE - 1;
        }
        else
        {
            index = m_curSlot - 1;
        }

        return index;
    }

    // delete_flag = OK, 一定会删除； new_index == index, 返回FAIL, 不删除， 否则删除插入新的
    int CEPOLL::GetFdIndex(const int fd, const int new_index, const int delete_flag)
    {
        int index = FAIL;
        m_iterFdIndex = m_fdIndex.find(fd);
        if(m_iterFdIndex != m_fdIndex.end())
        {
            index = m_iterFdIndex->second;
            if(delete_flag == OK)
            {
                m_fdIndex.erase(m_iterFdIndex);
                return index;
            }
            else
            {
                if(index == new_index)
                {
                    return FAIL;
                }
                else
                {
                    m_fdIndex.erase(m_iterFdIndex);
                    m_fdIndex.insert(std::pair<int, int>(fd, new_index));
                    return index;
                }
            }
        }
        else
        {
            log("GetFdIndex err");
            return FAIL;
        }
    }

    int CEPOLL::OnlineUpdate(const int fd, const std::string &ip, const unsigned int port)
    {
        int index = GetSlotIndex();

        online_t Online;
        Online.ip = ip;
        Online.port = port;
        Online.onlineTime = m_pSendBuf->GetTimesMs();

        m_fdIndex.insert(std::pair<int, int>(fd, index));
        m_online[index].insert(std::pair<int, online_t>(fd, Online));

        return OK;
    }

    void CEPOLL::RecvUpdate(const int fd)
    {
        int new_index = GetSlotIndex();
        int index = GetFdIndex(fd, new_index, FAIL);
        if(index == FAIL)
        {
            return;
        }

        m_iterOnline = m_online[index].find(fd);
        if(m_iterOnline != m_online[index].end())
        {
            m_online[new_index].insert(std::pair<int, online_t>(fd, m_iterOnline->second));
            m_online[index].erase(m_iterOnline);
        }
        else
        {
            log("RecvUpdate err fd = %d, index = %d", fd, index);
        }
    }

    void CEPOLL::OfflineUpdate(const int fd, const int index)
    {
        if(index == FAIL)
        {
            log("OfflineUpdate err fd = %d, index = %d", fd, index);
            return;
        }

        m_iterOnline = m_online[index].find(fd); // 找旧的
        if(m_iterOnline != m_online[index].end())
        {
            if(IsServer()) // 客户端发不到ACCEPT进程
            {
                online_t Online = (online_t)(m_iterOnline->second);
                SendOfflineFd(index, Online.ip);
            }

            m_online[index].erase(m_iterOnline); // 删除旧的
        }
        else
        {
            log("OfflineUpdate err fd = %d, index = %d", fd, index);
        }
    }

}


