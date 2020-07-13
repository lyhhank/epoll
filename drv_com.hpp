/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		drv_com.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  串口驱动
**********************************************************************************/

#ifndef _DRV_COM_HPP
#define _DRV_COM_HPP 1

#include "epoll.hpp"
#include "usrapp.hpp"

/*
	设备初始化类
*/
namespace lmSOCKET
{   
    class DRV_COM : public CEPOLL
    {
    	public:
    		DRV_COM();
    		~DRV_COM();

        protected:

            int Create(const int index);
            int Recv(const int fd, char *recvbuf, const int recvlen, const int flag);
            int Send(const int fd, char *sendbuf, const int sendlen, const int flag);

            std::string GetMsgHead(void) const;
            std::string GetFileHead(void) const;
            
       private:
           int OpenCom(const char *dev, const int flag);
           int SetComAttr(const int fd, const int databits, const int stopbits, const int parity, const int speed);
           int SetComDataBit(struct termios &options, const int databits);
           int SetComStopBit(struct termios &options, const int stopbits);
           int SetComParityBit(struct termios &options, const int parity);
           int SetComSpeed(struct termios &options, const int speed);
           int SetComCtrl(struct termios &options);

       private:
           int m_ret;
    };
}

#endif


