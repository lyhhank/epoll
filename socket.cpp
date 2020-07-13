/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		socket.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  根据配置启动SOCKET
    FORK
    PTH
**********************************************************************************/


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "socket.hpp"

namespace lmSOCKET
{
    CSOCKET::CSOCKET(const char *configFile, const int runType) : CCONFIG(configFile, runType)
    {
        m_ret = OK;
        
        m_pfork   = NULL; 
        m_pthread = NULL;

        return;
    }

    CSOCKET::~CSOCKET()
    {       
        if(m_pfork)
        {
            delete m_pfork;
            m_pfork = NULL;
        }

        if(m_pthread)
        {
            delete m_pthread;
            m_pthread = NULL;
        }
    }

    int CSOCKET::Start()
    {
        CHECKRESULT(m_ret);
        
        int runType = GetRunType();
        if(runType == FORK)
        {
            m_pfork = new CFORK();
            if(m_pfork == NULL)
            {
                log("new CFORK err");
                return FAIL;
            }

            return m_pfork->Run();
        }
        else if(runType == PTH)
        {
            m_pthread = new CPTH();
            if(m_pthread == NULL)
            {
                log("new CPTH err");
                return FAIL;
            }

            return m_pthread->Run();
        }
        else
        {
            log("Run mode err");
            return FAIL;
        }
    }
    
}
