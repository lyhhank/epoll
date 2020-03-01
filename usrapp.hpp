/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		base.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  用户程序基本功能及例子
**********************************************************************************/

#ifndef _USRAPP_HPP
#define _USRAPP_HPP 1

#include "base.hpp"

/*
	USRAPP类
*/
namespace lmSOCKET
{   
    class CUSR : virtual public CBASE
    {
    	public:
    		CUSR();
    		~CUSR();
            
            // 得到对象FD
            virtual int ClientGetFd(void) const = 0;

            // 获取BUF
            virtual char *GetSendBuf(const int fd) const = 0;
            
            // 开始发送
            virtual int StartSendMsg(const int fd, const int sendlen) = 0;
            virtual int StartSendFile(const int fd, const std::string fileName, const size_t offset) = 0;

            // 框架调客户接口
            int ServerRecv(const int fd, const char *recvbuf, const int recvlen, const msgType_e type);
            int ClientRecv(const int fd, const char *recvbuf, const int recvlen, const msgType_e type);

            // 发送（用户可自定义，此例是调用上面“开始发送”的函数示例）
            int client_send_msg(void);
            int client_send_file(const std::string fileName, const size_t offset);

            // EP超时调
            int ClientTimeout(const int fd); 
            // 客户任务
            int ClientTask(void);
    };
}

#endif

