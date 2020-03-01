#include <iostream>
#include <sys/types.h>
#include <string.h>

#include <stdio.h>
#include <unistd.h>

#include "usrapp.hpp"

namespace lmSOCKET
{
    CUSR::CUSR()
    {

    }

    CUSR::~CUSR()
    {       
    
    }

    int CUSR::ClientTimeout(const int fd)
    {
        log("timeout");

        client_send_msg();

        //client_send_file("back.txt", 0);

        return OK;
    }

    int CUSR::ClientTask(void)
    {
        ::sleep(1);
        return 0;
    }
    
    int CUSR::ServerRecv(const int fd, const char *recvbuf, const int recvlen, const msgType_e type)
    {
        log("server Recv = %s", recvbuf);
        
        char Send[] = "i'm recved";
        
        char *sendbuf = GetSendBuf(fd);
        if(sendbuf == NULL)
        {
            log("server get buf null");
            return FAIL;
        }
        
        ::memcpy(sendbuf, Send, sizeof(Send));
        StartSendMsg(fd, sizeof(Send));

        return OK;
    }
    
     int CUSR::ClientRecv(const int fd, const char *recvbuf, const int recvlen, const msgType_e type)
    {
        std::string s;
        for(int i = 0; i < recvlen; i++)
        {
            s += recvbuf[i];
        }            
        std::cout << s << std::endl;
        
        return OK;
    }   
     
    int CUSR::client_send_msg(void)
    {
        char buf[] = "test";
        
        int fd = ClientGetFd();
        
        char *pbuf = GetSendBuf(fd);
        if(pbuf == NULL)
        {
            log("client get buf null");
            return FAIL;
        }

        ::memcpy(pbuf, buf, sizeof(buf));
        StartSendMsg(fd, sizeof(buf));

        return OK;
    }

    int CUSR::client_send_file(const std::string fileName, const size_t offset)
    {
        int fd = ClientGetFd();
        StartSendFile(fd, fileName, offset);

        return OK;
    }

}
