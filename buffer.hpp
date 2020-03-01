/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		base.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  内存管理
**********************************************************************************/

#ifndef _BUFFER_HPP
#define _BUFFER_HPP 1

#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ep.hpp"
#include "transfd.hpp"
#include "usrapp.hpp"

#include <set>
#include <map>

struct data
{
    int fd;

    char buf[RECV_BUF_LEN];             // 以接收BUF大小申请，发送为1半
    char *pbuf;                         // 处理数据指针
    
    unsigned int len;                   // 收发的长度
    unsigned int dealLen;               // 处理数据长度

    msgType_e msgType;              // 消息类型
    
    char fileName[MAX_FILENAME_LEN];      // 文件名长度
    ssize_t off;                        // 传送文件的off

    long long time;                     // 占BUF时间
    unsigned int tryTimes;              // 错误重试
};

/*
	BUFFER类
*/
namespace lmSOCKET
{
	class CBUFFER : virtual public CCONFIG
	{
		public:
			CBUFFER();
            ~CBUFFER();

            long long GetTimesMs(void);
            void SetBufTime(void);
            int BufTimeOut(struct data *pbuf);
            
            void InitBuf(struct data *buf, const int fd);
            struct data *GetBuf(const int fd, bufStatus_e &status);
            int FreeBuf(const int fd);

            struct data *m_pdata; // BUF
            struct data *m_pbuf;  // BUF 指针

		private:
            int m_ret;
            
            std::set<int> m_bufFree;
            std::set<int>::iterator m_iterFree;
            
            std::map<int, int> m_bufUsed; // fd, index;
            std::map<int, int>::iterator m_iterUsed; 
            
	};
}

#endif


