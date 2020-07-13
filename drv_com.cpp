/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		drv_com.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  串口收发驱动
**********************************************************************************/

#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "drv_com.hpp"

namespace lmSOCKET
{
    static int com_speed_index[]  = { 50,75,110,134,150,300,600,1200,1800,2400,4800,9600,19200,
                               38400,57600,115200,230400,460800,500000,576000,921600,1000000,
                               1152000,1500000,2000000,2500000,3000000,3500000,4000000 };

    static int com_speed[] = { B50,B75,B110,B134,B150,B300,B600,B1200,B1800,B2400,B4800,B9600,B19200,
                               B38400,B57600,B115200,B230400,B460800,B500000,B576000,B921600,B1000000,
                               B1152000,B1500000,B2000000,B2500000,B3000000,B3500000,B4000000 };

    
    static const char com_data_index[]  =  { 5, 6, 7, 8 };
    static const char com_data[]        =  { CS5, CS6, CS7, CS8 };

    DRV_COM::DRV_COM()
    {
        m_ret = OK;
    }

    DRV_COM::~DRV_COM()
    {       
        
    }

    int DRV_COM::Create(const int index)
    {
        int fd = GetDevFd(index);
        if(fd > 0)
        {
            return fd;
        }
        else
        {
            com_t com = GetComInfo(index);
            int new_fd = OpenCom(com.dev, 1); // 1:noblock
            if(new_fd <= 0)
            {
                return FAIL;
            }
            else
            {
                m_ret = SetComAttr(new_fd, com.data, com.stop, com.check, com.baudrate);
                if(m_ret != OK)
                {
                    ::close(new_fd);
                    return FAIL;
                }

                SetDevFd(index, new_fd);
                return new_fd;
            }
        }
    }

    int DRV_COM::SetComDataBit(struct termios &options, const int databits)
    {
        // 设置数据位，屏蔽其他标志位
        options.c_cflag &= ~CSIZE;

        // 设置串口数据位
        for(unsigned int i = 0;  i < sizeof(com_data_index) / sizeof(char); i++)
        {
            if(databits == com_data_index[i])
            {
                options.c_cflag |= com_data[i];
                return OK;
            }
        }

        log("SetComDataBit err");
        return FAIL;
    }

    int DRV_COM::SetComStopBit(struct termios &options, const int stopbits)
    {
        // 设置停止位
        switch (stopbits)
        {
            case 1:
                options.c_cflag &= ~CSTOPB;
                break;
            case 2:
                options.c_cflag |= CSTOPB;
                break;
            default:
                return FAIL;
        }

        return OK;
    }

    int DRV_COM::SetComParityBit(struct termios &options, const int parity)
    {
        // 设置校验位
        switch (parity)
        {
            case 'n':
            case 'N':
                options.c_cflag &= ~PARENB;
                options.c_iflag &= ~INPCK;
                break; // 无奇偶校验位。
            case 'o':
            case 'O':
                options.c_cflag |= (PARODD | PARENB);
                options.c_iflag |= INPCK;
                break; // 设置为奇校验
            case 'e':
            case 'E':
                options.c_cflag |= PARENB;
                options.c_cflag &= ~PARODD;
                options.c_iflag |= INPCK;
                break; // 设置为偶校验
            case 's':
            case 'S':
                options.c_cflag &= ~PARENB;
                options.c_cflag &= ~CSTOPB;
                break; //设置为空格
            default:
                log("SetComParityBit err");
                return FAIL;
        }
    
        return OK;
    }

    int DRV_COM::SetComSpeed(struct termios &options, const int speed)
    {
        // 设置串口输入波特率和输出波特率
        for(unsigned int i = 0;  i < sizeof(com_speed_index) / sizeof(int);  i++)
        {
            if(speed == com_speed_index[i])
            {
                m_ret = ::cfsetispeed(&options, com_speed[i]);
                m_ret |= ::cfsetospeed(&options, com_speed[i]);
                return OK;
            }
        }

        logerr("SetComSpeed err");
        return FAIL;
    }

    int DRV_COM::SetComCtrl(struct termios &options)
    {
        // 关闭流控，不剥除第8位
        options.c_iflag = 0;
    
        // 设置数据流控制
        options.c_cflag = 0;
        options.c_cflag |= (CLOCAL | CREAD);
        
        // 原始输入
        options.c_lflag = 0;

        // 原始输出
        options.c_oflag = 0;

        // 设置EP模式读取方式
        options.c_cc[VTIME] = 0;
        options.c_cc[VMIN]  = 0;

        return OK;
    }
    
    int DRV_COM::SetComAttr(const int fd, const int databits, const int stopbits, const int parity, const int speed)
    {
        struct termios options;
        if(::tcgetattr(fd, &options) !=  OK)
        {
            logerr("tcgetattr err");
            return FAIL;
        }
        
        if(SetComCtrl(options) != OK)
        {
            log("SetComCtrl err");
            return FAIL;
        }
        if(SetComDataBit(options, databits) != OK)
        {
            log("SetComDataBit err");
            return FAIL;
        }
        if(SetComStopBit(options, stopbits) != OK)
        {
            log("SetComStopBit err");
            return FAIL;
        }
        if(SetComParityBit(options, parity) != OK)
        {
            log("SetComParityBit err");
            return FAIL;
        }
        if(SetComSpeed(options, speed) != OK)
        {
            log("SetComSpeed err");
            return FAIL;
        }

        // 刷新输入输出缓存
        if(::tcflush(fd, TCIOFLUSH != OK))
        {
            logerr("tcflush err");
            return FAIL;
        }
    
        // 激活配置 (将修改后的termios数据设置到串口中）
        if (::tcsetattr(fd, TCSANOW, &options) != OK)
        {
            logerr("tcsetattr err");
            return FAIL;
        }
        
        return OK;
    }
    
    //打开串口
    int DRV_COM::OpenCom(const char *dev, const int flag)
    {
        int fd = ::open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
        if (fd <= 0)
        {
            logerr("OpenCom fail dev=%s, fd = %d", dev,fd);
            return FAIL;
        }
        else
        {
            if(flag == 0)
            {
                m_ret = ::fcntl(fd,F_SETFL,0); //block mode
            }
            else
            {
                m_ret = ::fcntl(fd,F_SETFL,FNDELAY); //noblock mode
            }

            if(m_ret == FAIL)
            {
                logerr("fcntl err");
                ::close(fd);
                fd = 0;
            }

            return fd;
        }
    }

    int DRV_COM::Recv(const int fd, char *recvbuf, const int recvlen, const int flag)
    {
        return ::read(fd, recvbuf, recvlen); // 接收
    }

    int DRV_COM::Send(const int fd, char *sendbuf, const int sendlen, const int flag)
    {   
        return ::write(fd, sendbuf, sendlen); // 发送
    }
    
    std::string DRV_COM::GetMsgHead(void) const 
    {
        return "M:";
    }

    std::string DRV_COM::GetFileHead(void) const 
    {
        return "F:";
    }
}
