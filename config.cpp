/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		config.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  程序配置类
**********************************************************************************/


#include <string.h> // memcpy()
#include <unistd.h> // close()
#include <arpa/inet.h>

#include "config.hpp"

namespace lmSOCKET
{
    config_t* CCONFIG::m_pconf = NULL;

    CCONFIG::CCONFIG()
    {

    }

    CCONFIG::CCONFIG(const char *configFile, const int runType)
    {
        m_ret = OK;

        if(m_pconf == NULL)
        {
            m_pconf = new config_t();
            if(m_pconf == NULL)
            {
                log("new m_pconf fail");
                m_ret = FAIL;
            }
            else
            {
                if(::access(configFile, F_OK) == FAIL)
                {
                    logerr("%s is not exist", configFile);
                    m_ret = FAIL;
                    return;
                }

                // 参数
                if(runType == SERVER)
                {
                    log("Run as Server");
                    m_pconf->cs = SERVER;
                    m_ret = ReadServerConf(configFile, m_pconf);
                }
                else
                {
                    log("Run as CLIENT");
                    m_pconf->cs = CLIENT;
                    m_ret = ReadClientConf(configFile, m_pconf);
                }
            }
        }
    }

    CCONFIG::~CCONFIG()
    {       
        if(m_pconf)
        {
            for(int i = 0; i < MAXEVENTS; i++)
            {
                if(m_pconf->dev[i].fd > 0)
                {
                    ::close(m_pconf->dev[i].fd);
                    m_pconf->dev[i].fd = 0;
                }
            }
            
            delete m_pconf;
            m_pconf = NULL;
        }
    }

    void CCONFIG::LogParaPublic(void)    
    {
        log("PUBLIC CONF:");
        log("cs = %s", g_trans[m_pconf->cs]);
        log("runType = %s", g_trans[m_pconf->runType]);
        log("cpuNum = %d", m_pconf->cpuNum);
        log("debug = %s", g_trans[m_pconf->debug]);
        log("stacksize = %zu", m_pconf->pthAttr.stacksize);
        log("socketreused = %s", g_trans[m_pconf->socketAttr.reused]);
    }

    void CCONFIG::LogParaServer(void)
    {
        LogParaPublic();
        log("SERVER CONF:");
        log("devType = %s", g_trans[m_pconf->dev[0].devType]);
        log("addr = %s", m_pconf->dev[0].devInfo.socket.addr);
        log("port = %d", m_pconf->dev[0].devInfo.socket.port);
        log("tcpudp = %s", g_trans[m_pconf->dev[0].devInfo.socket.tcpudp]);
        log("ipv4ipv6 = %s", g_trans[m_pconf->dev[0].devInfo.socket.ipv4ipv6]);
        log("heartTimeout = %d", m_pconf->dev[0].devInfo.socket.timeout);
        log("sameIpLimit = %d", m_pconf->sameIpLimit);
    }

    void CCONFIG::LogParaClient(void)
    {
        LogParaPublic();
        for(int i = 0; i < m_pconf->cpuNum; i++)
        {
            log("CLIENT[%d] CONF:", i);
            if(m_pconf->dev[i].devType == SOCKET)
            {
                log("devType = %s", g_trans[m_pconf->dev[i].devType]);
                log("transto = %d", m_pconf->dev[i].transIndex);
                log("addr = %s", m_pconf->dev[i].devInfo.socket.addr);
                log("port = %d", m_pconf->dev[i].devInfo.socket.port);
                log("tcpudp = %s", g_trans[m_pconf->dev[i].devInfo.socket.tcpudp]);
                log("ipv4ipv6 = %s", g_trans[m_pconf->dev[i].devInfo.socket.ipv4ipv6]);
                log("timeout = %d", m_pconf->dev[i].devInfo.socket.timeout);
                log("clientTask = %s", g_trans[m_pconf->clientTask]);
            }
            else
            {
                log("devType = %s", g_trans[m_pconf->dev[i].devType]);
                log("transto = %d", m_pconf->dev[i].transIndex);
                log("devpath = %s", m_pconf->dev[i].devInfo.com.dev);
                log("databit = %d", m_pconf->dev[i].devInfo.com.data);
                log("stopbit = %d", m_pconf->dev[i].devInfo.com.stop);
                log("checkbit = %c", (char)(m_pconf->dev[i].devInfo.com.check));
                log("baudrate = %d", m_pconf->dev[i].devInfo.com.baudrate);
                log("timeout = %d", m_pconf->dev[i].devInfo.com.timeout);
                log("clientTask = %s", g_trans[m_pconf->clientTask]);
            }
        }
    }

    int CCONFIG::CheckParaPublic(void)
    {
        CHECKRANG(m_pconf->cs, SERVER, CLIENT);
        CHECKRANG(m_pconf->runType, FORK, PTH);
        CHECKRANG(m_pconf->cpuNum, 1, MAXEVENTS);
        CHECKRANG(m_pconf->debug, 0, 1);
        return OK;
    }

    int CCONFIG:: IsValidIPv4(const char *ipv4)
    {
        struct in_addr addr;
        if(ipv4 == NULL)
        {
            return FAIL;
        }
        if(::inet_pton(AF_INET, ipv4, (void *)&addr) == 1) // 1表示成功
        {
            return OK;
        }
        else
        {
            logerr("not valid ip");
            return FAIL;
        }
    }
    
    int CCONFIG:: IsValidIPv6(const char *ipv6)
    {
        struct in6_addr addr6;
        if(ipv6 == NULL)
        {
            return FAIL;
        }
        
        if(inet_pton(AF_INET6, ipv6, (void *)&addr6) == 1)
        {
            return OK;
        }
        else
        {
            logerr("not valid ip");
            return FAIL;
        }
    }

    int CCONFIG::CheckParaServer(void)
    {
        CHECKRANG(m_pconf->dev[0].devType, SOCKET, COM);
        if((IsValidIPv4(m_pconf->dev[0].devInfo.socket.addr) != OK) && (IsValidIPv6(m_pconf->dev[0].devInfo.socket.addr) != OK))
        {
            log("m_pconf->dev[0].devInfo.socket.addr config err");
            return FAIL;
        }
        CHECKRANG(m_pconf->dev[0].devInfo.socket.port, 1, 65535);
        CHECKRANG(m_pconf->dev[0].devInfo.socket.tcpudp, TCP, UDP);
        CHECKRANG(m_pconf->dev[0].devInfo.socket.ipv4ipv6, IPV4, IPV6);
        CHECKRANG(m_pconf->dev[0].devInfo.socket.timeout, 30, 65535);
        CHECKRANG(m_pconf->sameIpLimit, 1, 65535);
        return OK;
    }

    int CCONFIG::CheckParaClient(void)
    {
        for(int i = 0; i < m_pconf->cpuNum; i++)
        {
            if(m_pconf->dev[i].devType == SOCKET)
            {
                CHECKRANG(m_pconf->dev[i].devType, SOCKET, COM);
                if((IsValidIPv4(m_pconf->dev[i].devInfo.socket.addr) != OK) && (IsValidIPv6(m_pconf->dev[i].devInfo.socket.addr) != OK))
                {
                    log("m_pconf->dev[%d].devInfo.socket.addr config err", i);
                    return FAIL;
                }
                CHECKRANG(m_pconf->dev[i].devInfo.socket.port, 1, 65535);
                CHECKRANG(m_pconf->dev[i].devInfo.socket.tcpudp, TCP, UDP);
                CHECKRANG(m_pconf->dev[i].devInfo.socket.ipv4ipv6, IPV4, IPV6);
                CHECKRANG(m_pconf->dev[i].devInfo.socket.timeout, 30, 65535);
                CHECKRANG(m_pconf->clientTask, ON, OFF);
            }
            else
            {
                CHECKRANG(m_pconf->dev[i].devType, SOCKET, COM);
                if(::access(m_pconf->dev[i].devInfo.com.dev, F_OK) == FAIL)
                {
                    log("%s is not exist", m_pconf->dev[i].devInfo.com.dev);
                    return FAIL;
                }

                CHECKRANG(m_pconf->dev[i].devInfo.com.data, 5, 8);
                CHECKRANG(m_pconf->dev[i].devInfo.com.stop, 1, 2);
                if((m_pconf->dev[i].devInfo.com.check != (int)('N')) && (m_pconf->dev[i].devInfo.com.check != (int)('E')) 
                    && (m_pconf->dev[i].devInfo.com.check != (int)('O')) && (m_pconf->dev[i].devInfo.com.check != (int)('S')))
                {
                    log("m_pconf->dev[%d].devInfo.com.check config err", i);
                    return FAIL;
                }
                CHECKRANG(m_pconf->dev[i].devInfo.com.baudrate, 50, 4000000);
                CHECKRANG(m_pconf->dev[i].devInfo.com.timeout, 1, 65535);
                CHECKRANG(m_pconf->clientTask, ON, OFF);
            }
        }
        return OK;
    }

    int CCONFIG::CheckPara()
    {
        CHECKRESULT(m_ret);
        
        m_ret = CheckParaPublic();
        CHECKRESULT(m_ret);

        if(m_pconf->cs == SERVER)
        {
            m_ret = CheckParaServer();
            CHECKRESULT(m_ret);
            LogParaServer();
        }
        else
        {
            m_ret = CheckParaClient();
            CHECKRESULT(m_ret);
            LogParaClient();
        }

        g_debug = m_pconf->debug;

        return OK;
    }

    config_t* CCONFIG::GetConfig(void) const
    {
        return m_pconf;
    }

    int CCONFIG::IsServer(void) const
    {
        return (GetConfig()->cs == SERVER);
    }

    int CCONFIG::GetRunType(void) const
    {
        return GetConfig()->runType;
    }
    
    int CCONFIG::GetCpuNum(void) const
    {
        return GetConfig()->cpuNum;
    }

    int CCONFIG::GetDevType(int index) const
    {
        if(IsServer())
        {
            index = 0;
        }
        return GetConfig()->dev[index].devType;
    }

    socket_t CCONFIG::GetSocketInfo(int index) const
    {
        if(IsServer())
        {
            index = 0;
        }
        return GetConfig()->dev[index].devInfo.socket;
    }

    com_t CCONFIG::GetComInfo(int index) const
    {
        if(IsServer())
        {
            index = 0;
        }
        return GetConfig()->dev[index].devInfo.com;
    }

    int CCONFIG::GetDevFd(int index) const
    {
        if(IsServer())
        {
            index = 0;
        }
        return GetConfig()->dev[index].fd;
    }
    
    void CCONFIG::SetDevFd(int index, int fd)
    {
        if(IsServer())
        {
            index = 0;
        }
        GetConfig()->dev[index].fd = fd;
    }

    int CCONFIG::GetTransIndex(int index) const
    {
        if(IsServer())
        {
            return FAIL; // 无效
        }
        return GetConfig()->dev[index].transIndex;
    }

    int CCONFIG::IsUdp(int index) const
    {
        return (GetSocketInfo(index).tcpudp == UDP);
    }

    int CCONFIG::IsIpv4(int index) const
    {
        return (GetSocketInfo(index).ipv4ipv6 == IPV4);
    }

    int CCONFIG::IsDebugOn(void) const
    {
        return m_pconf->debug;
    }

    int CCONFIG::GetSameIpLimit(void) const
    {
        return m_pconf->sameIpLimit;
    }

    int CCONFIG::GetTimeOut(const int index) const
    {
        if(GetDevType(index) == SOCKET)
        {
            return GetComInfo(index).timeout;
        }
        else
        {
            return GetSocketInfo(index).timeout;
        }
    }

}

