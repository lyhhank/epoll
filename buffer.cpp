/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		buffer.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  BUF 管理
**********************************************************************************/


#include <sys/time.h>
#include "buffer.hpp"

namespace lmSOCKET
{
	CBUFFER::CBUFFER()
	{
        m_ret = OK;

        int num = 0;
        if(IsServer())
        {
            num = MAXEVENTS + 2; // 增加BUF防止粘包
        }
        else
        {
            num = 1 + 1; // 转发，心跳
        }

        m_pdata = new data[num];
        if(m_pdata == NULL)
        {
            m_ret = FAIL;
        }

        for(int i = 0; i < num; i++)
        {
            m_bufFree.insert(i);
        }
        m_bufUsed.clear();
	}

	CBUFFER::~CBUFFER()
	{
        if(m_pdata)
        {
            delete [] m_pdata;
            m_pdata = NULL;
        }

        m_bufFree.clear();
        m_bufUsed.clear();
	}

    void CBUFFER::SetBufTime(void)
    {
        m_pbuf->time = GetTimesMs(); // 占用
    }

    void CBUFFER::InitBuf(struct data *buf, const int fd)
    {
        buf->fd = fd;  // 占用
        buf->pbuf = buf->buf;

        buf->len = 0;
        buf->msgType = MSG_NOTYPE;
        
        buf->off = 0;
        buf->time = GetTimesMs();
    }
    
    struct data* CBUFFER::GetBuf(const int fd)
    {
        m_iterUsed = m_bufUsed.find(fd);
        m_iterFree = m_bufFree.begin();

        if(m_iterUsed != m_bufUsed.end()) // 已有
        {
            return &m_pdata[m_iterUsed->second]; // BUSY!!!
        }
        else // 新申请 
        {
            if(m_iterFree != m_bufFree.end()) // 可申请 
            {
                m_bufUsed.insert(std::pair<int, int>(fd, *m_iterFree)); // 增加
                m_bufFree.erase(*m_iterFree); // 删除

                InitBuf(&m_pdata[*m_iterFree], fd);

                return &m_pdata[*m_iterFree];
            }
            else // 无空的BUFF;
            {
                for(m_iterUsed = m_bufUsed.begin(); m_iterUsed != m_bufUsed.end(); m_iterUsed++)
                {
                    // 释放超时BUFF
                    if(BufTimeOut(&m_pdata[m_iterUsed->second]) == OK)
                    {
                        if(FreeBuf(m_iterUsed->first) == OK)
                        {
                            InitBuf(&m_pdata[m_iterUsed->second], fd);

                            return &m_pdata[fd];
                        }
                    }
                }
                
                log("get buf fail, no free buf");
                return NULL;
            }
        }
    }

    int CBUFFER::BufTimeOut(struct data *pbuf)
    {
        long long timems = GetTimesMs();
        if((pbuf->time == 0) || (timems - m_pbuf->time > RS_TIMEOUT))
        {
            return OK;
        }
        else
        {
            return FAIL;
        }
    }

    int CBUFFER::FreeBuf(const int fd)
    {
        int index = -1;
    
        m_iterUsed = m_bufUsed.find(fd);
        if(m_iterUsed != m_bufUsed.end()) // 找到FD
        {
            index = m_iterUsed->second; // 取INDEX
            
            m_bufUsed.erase(fd); 
            m_bufFree.insert(index);

            this->m_pbuf = NULL;
            return OK;
        }
        else
        {
            log("FreeBuf err");
            return FAIL;
        }
    }

    long long CBUFFER::GetTimesMs(void)
    {
        struct timeval tv;
        m_ret = ::gettimeofday(&tv, NULL);
        if(m_ret == OK)
        {
            return (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
        }
        else
        {
            logerr("GetTimesMs err");
            return 0;
        }   
    }

}


