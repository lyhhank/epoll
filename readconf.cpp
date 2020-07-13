/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		readconf.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  配置文件读取
    #注释
    样式：serveraddr=127.0.0.1
**********************************************************************************/


#include "readconf.hpp"

namespace lmSOCKET
{
	CREADCONF::CREADCONF()
	{
        m_ret = OK;
	}

	CREADCONF::~CREADCONF()
	{
        
	}

    template <class T1, class T2>
    T1 CREADCONF::TransType(T1 &, T2 &in)
	{
        T1 out;
        std::stringstream s;
        s.clear();
        s << in;
        s >> out;
        return out;
    }

    // 删除buf中的制表符以及空格，注释行跳过
    int CREADCONF::DelSpace(char *pbuf)
    {
        char buf[MAX_CONFFILE_LEN] = {0};
        if(pbuf == NULL || ::strlen(pbuf) == 0)
        {
            return FAIL;
        }
        
        int len = ::strlen(pbuf);
        int i = 0;
        int j = 0;

        while(i < len)
        {
            if(pbuf[i] == '#') // 注释行
            {
                return FAIL;
            }
    
            if((::iscntrl(pbuf[i]) == 0) && (::isspace(pbuf[i]) == 0))
            {
                buf[j++] = pbuf[i];
            }

            i++;
        }
     
        //将结果复制到buf
        ::memset(pbuf, 0, len);
        ::strncpy(pbuf, buf, j);
        pbuf[j] = '\0';
        
        return OK;
    }
     
    int CREADCONF::GetPos(char *buf)
    {
        if (buf == NULL || ::strlen(buf) == 0)
        {
            return FAIL;
        }
        
        int len = ::strlen(buf);
        int i = 0;
        
        while (i < len)
        {
            if (buf[i] == '=')
            {
                return i+1;
            }
            
            i++;
        }
        
        return FAIL;
    }
     
    // 从文件‘file’中的[title]中读取属性KeyName的值KeyVal
    int CREADCONF::ReadConf(const char *configFile, const char *title, const char *keyName, char *keyValue)
    {
        char bufLine[MAX_CONFFILE_LEN] = {0};
        int findTitle = FAIL;
        int findValue = FAIL;
     
        if(configFile == NULL || ::strlen(configFile) == 0) //文件名（路径）检查
        {
            log("configFile name null");
            return FAIL;
        }
        
        FILE *fp = ::fopen(configFile, "r"); //打开文件
        if(fp == NULL) //文件句柄检查
        {
            logerr("open configFile err");
            return FAIL;
        }

        while(::fgets(bufLine, MAX_CONFFILE_LEN, fp) != NULL)
        {
            if(findTitle != OK)
            {
                if(DelSpace(bufLine) != OK)
                {
                    continue;
                }   

                if(::strncasecmp(bufLine+1, title, ::strlen(title)) == 0) // 找到标签
                {
                    findTitle = OK;
                }
            }
            else 
            {
                if(DelSpace(bufLine) != OK)
                {
                    continue;
                }

                if(bufLine[0] == '[') // 到下一个标签了
                {
                    log("%s %s not find", title, keyName);
                    return FAIL;
                }   

                if(::strncasecmp(bufLine, keyName, ::strlen(keyName)) == 0) // 找到KEY
                {
                    int pos = GetPos(bufLine); // 开始取值
                    if(pos > 0)
                    {
                        char *pvalue = &bufLine[pos];

                        ::strncpy(keyValue, pvalue, ::strlen(pvalue));
                        keyValue[::strlen(pvalue)] = '\0';
                        findValue = OK;
                        break;
                    }     
                }
            } 
        }
        
        //关闭文件句柄
        if (fp != NULL)
        {
            ::fclose(fp);
        }

        if(findValue != OK)
        {
            log("find title=%s key=%s fail", title, keyName);
        }
        return findValue;
    }

    int CREADCONF::GetAllClientTitle(const char *configFile, std::set<std::string> &allClientTitle)
    {
        std::set<std::string>::iterator iterAllClientTitle;
        
        char bufLine[MAX_CONFFILE_LEN] = {0};
    
        if(configFile == NULL || ::strlen(configFile) == 0) //文件名（路径）检查
        {
            log("configFile name null");
            return FAIL;
        }
        
        FILE *fp = ::fopen(configFile, "r"); //打开文件
        if(fp == NULL) //文件句柄检查
        {
            logerr("open configFile err");
            return FAIL;
        }

        while(::fgets(bufLine, MAX_CONFFILE_LEN, fp) != NULL)
        {
            if(DelSpace(bufLine) != OK)
            {
                continue;
            }   

            if((bufLine[0] == '[') && (bufLine[::strlen(bufLine) - 1] == ']'))
            {
                if(::strncasecmp(bufLine+1, g_trans[SOCKET], ::strlen(g_trans[SOCKET])) == 0) // 找到socket...
                {
                    bufLine[::strlen(bufLine) - 1] = '\0';
                    std::string title = &bufLine[1];
                    for(iterAllClientTitle = allClientTitle.begin(); iterAllClientTitle != allClientTitle.end(); iterAllClientTitle++)
                    {
                        std::string strTitle;
                        strTitle = *iterAllClientTitle;
                        if(::strncasecmp(strTitle.c_str(), title.c_str(), ::strlen(title.c_str())) == 0)
                        {
                            log("config %s have repeat", strTitle.c_str());
                            return FAIL;
                        }
                    }
                    allClientTitle.insert(title);
                }
                else if(::strncasecmp(bufLine+1, g_trans[COM], ::strlen(g_trans[COM])) == 0) // 找到com...
                {
                    bufLine[::strlen(bufLine) - 1] = '\0';
                    std::string title = &bufLine[1];
                    for(iterAllClientTitle = allClientTitle.begin(); iterAllClientTitle != allClientTitle.end(); iterAllClientTitle++)
                    {
                        std::string strTitle;
                        strTitle = *iterAllClientTitle;
                        if(::strncasecmp(strTitle.c_str(), title.c_str(), ::strlen(title.c_str())) == 0)
                        {
                            log("config %s have repeat", strTitle.c_str());
                            return FAIL;
                        }
                    }
                    allClientTitle.insert(title);
                }
            }
        }

        if(allClientTitle.size() > 0)
        {
            return OK;
        }

        return FAIL;
    }

    int CREADCONF::ReadPublicConf(const char *configFile, config_t *pconf)
    {
        std::string strValue;
        int ret;
        
        int m_ret = OK;
        char KeyValue[MAX_CONFFILE_LEN];

        m_ret |= ReadConf(configFile, "public", "debug", KeyValue);
        strValue = KeyValue;
        ret = MapType(strValue);
        if(ret != FAIL)
        {
            pconf->debug = ret;
        }
        else
        {
            log("MapType err");
            return FAIL;
        }

        m_ret |= ReadConf(configFile, "public", "socketreused", KeyValue);
        strValue = KeyValue;
        ret = MapType(strValue);
        if(ret != FAIL)
        {
            pconf->socketAttr.reused = ret;
        }
        else
        {
            log("MapType err");
            return FAIL;
        }

        if(pconf->cs == SERVER)
        {
            m_ret |= ReadConf(configFile, "public", "serverstacksize", KeyValue);
            pconf->pthAttr.stacksize = TransType(pconf->pthAttr.stacksize, KeyValue) * 1024;
            
            m_ret |= ReadConf(configFile, "public", "serverforkpth", KeyValue);
            strValue = KeyValue;
            ret = MapType(strValue);
            if(ret != FAIL)
            {
                pconf->runType = ret;
            }
            else
            {
                log("MapType err");
                return FAIL;
            }
        }
        else if(pconf->cs == CLIENT)
        {
            m_ret |= ReadConf(configFile, "public", "clientstacksize", KeyValue);
            pconf->pthAttr.stacksize = TransType(pconf->pthAttr.stacksize, KeyValue) * 1024;
            
            m_ret |= ReadConf(configFile, "public", "clientforkpth", KeyValue);
            strValue = KeyValue;
            ret = MapType(strValue);
            if(ret != FAIL)
            {
                pconf->runType = ret;
            }
            else
            {
                log("MapType err");
                return FAIL;
            }
        }
        else
        {
            log("config run mode err");
            return FAIL;
        }
        
        return m_ret;
    }
    
    int CREADCONF::ReadServerConf(const char *configFile, config_t *pconf)
    {
        std::string strValue;
        int ret;
        
        int m_ret = OK;
        char KeyValue[MAX_CONFFILE_LEN];
            
        pconf->dev[0].devType = SOCKET;
        pconf->dev[0].transIndex = -1;

        m_ret = ReadPublicConf(configFile, pconf);
        if(m_ret != OK)
        {
            log("ReadPublicConf err");
            return m_ret;
        }

        m_ret |= ReadConf(configFile, "server", "serveraddr", KeyValue);
        ::memcpy(pconf->dev[0].devInfo.socket.addr, KeyValue, ::strlen(KeyValue));
        
        m_ret |= ReadConf(configFile, "server", "serverport", KeyValue);
        pconf->dev[0].devInfo.socket.port = TransType(pconf->dev[0].devInfo.socket.port, KeyValue);
        
        m_ret |= ReadConf(configFile, "server", "tcpudp", KeyValue);
        strValue = KeyValue;
        ret = MapType(strValue);
        if(ret != FAIL)
        {
            pconf->dev[0].devInfo.socket.tcpudp = ret;
        }
        else
        {
            log("MapType err");
            return FAIL;
        }
        
        m_ret |= ReadConf(configFile, "server", "ipv4ipv6", KeyValue);
        strValue = KeyValue;
        ret = MapType(strValue);
        if(ret != FAIL)
        {
            pconf->dev[0].devInfo.socket.ipv4ipv6 = ret;
        }
        else
        {
            log("MapType err");
            return FAIL;
        }
        
        pconf->dev[0].devInfo.socket.flags = 0;

        m_ret |= ReadConf(configFile, "server", "timeout", KeyValue);
        pconf->dev[0].devInfo.socket.timeout = TransType(pconf->dev[0].devInfo.socket.timeout, KeyValue) * CLIENT_TIMEOUT_BASE;

        m_ret |= ReadConf(configFile, "server", "cpunum", KeyValue);
        pconf->cpuNum = TransType(pconf->cpuNum, KeyValue);
        
        m_ret |= ReadConf(configFile, "server", "sameiplimit", KeyValue);
        pconf->sameIpLimit = TransType(pconf->sameIpLimit, KeyValue);

        if(m_ret != OK)
        {
            log("ReadServerConf fail");
            return FAIL;
        }

        return m_ret;
    }

    int CREADCONF::ReadClientConf(const char *configFile, config_t *pconf)
    {
        std::string strTitle;
        std::string strValue;
        int ret;
        
        int m_ret = OK;
        char KeyValue[MAX_CONFFILE_LEN];

        m_ret = ReadPublicConf(configFile, pconf);
        if(m_ret != OK)
        {
            log("ReadPublicConf err");
            return m_ret;
        }
        
        std::set<std::string> allClientTitle;  
        std::set<std::string>::iterator iterAllClientTitle;
        allClientTitle.clear();
        
        m_ret = GetAllClientTitle(configFile, allClientTitle);
        if(m_ret != OK)
        {
            log("GetAllClientTitle fail");
            return FAIL;
        }

        int i = 0;
        for(iterAllClientTitle = allClientTitle.begin(); iterAllClientTitle != allClientTitle.end(); iterAllClientTitle++)
        {
            strTitle = *iterAllClientTitle;
            log("title = %s", strTitle.c_str());

            if(::strncasecmp(strTitle.c_str(), g_trans[SOCKET], ::strlen(g_trans[SOCKET])) == 0)
            {
                pconf->dev[i].devType = SOCKET;
                
                m_ret |= ReadConf(configFile, strTitle.c_str(), "serveraddr", KeyValue);
                ::memcpy(pconf->dev[i].devInfo.socket.addr, KeyValue, ::strlen(KeyValue));

                m_ret |= ReadConf(configFile, strTitle.c_str(), "serverport", KeyValue);
                pconf->dev[i].devInfo.socket.port = TransType(pconf->dev[i].devInfo.socket.port, KeyValue);
                
                m_ret |= ReadConf(configFile, strTitle.c_str(), "tcpudp", KeyValue);
                strValue = KeyValue;
                ret = MapType(strValue);
                if(ret != FAIL)
                {
                    pconf->dev[i].devInfo.socket.tcpudp = ret;
                }
                else
                {
                    log("MapType err");
                    return FAIL;
                }

                m_ret |= ReadConf(configFile, strTitle.c_str(), "ipv4ipv6", KeyValue);
                strValue = KeyValue;
                ret = MapType(strValue);
                if(ret != FAIL)
                {
                    pconf->dev[i].devInfo.socket.ipv4ipv6 = ret;
                }
                else
                {
                    log("MapType err");
                    return FAIL;
                }

                pconf->dev[i].devInfo.socket.flags = 0;
                
                m_ret |= ReadConf(configFile, strTitle.c_str(), "timeout", KeyValue);
                pconf->dev[i].devInfo.socket.timeout = TransType(pconf->dev[i].devInfo.socket.timeout, KeyValue);

                m_ret |= ReadConf(configFile, strTitle.c_str(), "clienttask", KeyValue);
                strValue = KeyValue;
                ret = MapType(strValue);
                if(ret != FAIL)
                {
                    pconf->clientTask = ret;
                }
                else
                {
                    log("MapType err");
                    return FAIL;
                }

                m_ret |= ReadConf(configFile, strTitle.c_str(), "transto", KeyValue);
                strValue = KeyValue;
                int index = -1;
                if(::strncasecmp(strValue.c_str(), g_trans[NONE], ::strlen(g_trans[NONE])) != 0) // 要转发
                {
                    if((::strncasecmp(strTitle.c_str(), g_trans[SOCKET], ::strlen(g_trans[SOCKET])) == 0) ||
                        (::strncasecmp(strTitle.c_str(), g_trans[COM], ::strlen(g_trans[COM])) == 0))
                    {
                        index = FindTransIndex(allClientTitle, strTitle);
                        if(index < 0)
                        {
                            log("FindTransIndex err");
                            return FAIL;
                        }
                    }
                }
                pconf->dev[i].transIndex = index;
                
                if(m_ret != OK)
                {
                    log("ReadClientSocketConf socket err");
                    return m_ret;
                }
                else 
                {
                    i++;
                    continue;
                }
                
            }
            else
            {
                if(::strncasecmp(strTitle.c_str(), g_trans[COM], ::strlen(g_trans[COM])) == 0)
                {
                    pconf->dev[i].devType = COM;

                    m_ret |= ReadConf(configFile, strTitle.c_str(), "devpath", KeyValue);
                    ::memcpy(pconf->dev[i].devInfo.com.dev, KeyValue, ::strlen(KeyValue));

                    m_ret |= ReadConf(configFile, strTitle.c_str(), "databit", KeyValue);
                    pconf->dev[i].devInfo.com.data = TransType(pconf->dev[i].devInfo.com.data, KeyValue);
                    
                    m_ret |= ReadConf(configFile, strTitle.c_str(), "stopbit", KeyValue);
                    pconf->dev[i].devInfo.com.stop = TransType(pconf->dev[i].devInfo.com.stop, KeyValue);
                    
                    m_ret |= ReadConf(configFile, strTitle.c_str(), "checkbit", KeyValue);
                    pconf->dev[i].devInfo.com.check = KeyValue[0];
                    
                    m_ret |= ReadConf(configFile, strTitle.c_str(), "baudrate", KeyValue);
                    pconf->dev[i].devInfo.com.baudrate = TransType(pconf->dev[i].devInfo.com.baudrate, KeyValue);

                    m_ret |= ReadConf(configFile, strTitle.c_str(), "timeout", KeyValue);
                    pconf->dev[i].devInfo.com.timeout = TransType(pconf->dev[i].devInfo.com.timeout, KeyValue);

                    m_ret |= ReadConf(configFile, strTitle.c_str(), "transto", KeyValue);
                    strValue = KeyValue;
                    
                    int index = -1;
                    if(::strncasecmp(strValue.c_str(), g_trans[NONE], ::strlen(g_trans[NONE])) != 0) // 要转发
                    {
                        if((::strncasecmp(strTitle.c_str(), g_trans[SOCKET], ::strlen(g_trans[SOCKET])) == 0) ||
                            (::strncasecmp(strTitle.c_str(), g_trans[COM], ::strlen(g_trans[COM])) == 0))
                        {
                            index = FindTransIndex(allClientTitle, strTitle);
                            if(index < 0)
                            {
                                log("FindTransIndex err");
                                return FAIL;
                            }
                        }
                    }
                    pconf->dev[i].transIndex = index;

                    if(m_ret != OK)
                    {
                        log("ReadClientSocketConf com err");
                        return m_ret;
                    }
                    else
                    {
                        i++;
                        continue;
                    }
                }
            }
        }

        pconf->cpuNum = i;

        return OK;
    }

    int CREADCONF::FindTransIndex(const std::set<std::string> &allClientTitle, const std::string &title)
    {
        int i = 0;

        std::string strTitle;
        std::set<std::string>::iterator iterFindTransIndex;
        for(iterFindTransIndex = allClientTitle.begin(); iterFindTransIndex != allClientTitle.end(); iterFindTransIndex++)
        {
            strTitle = *iterFindTransIndex;
            if(::strncasecmp(strTitle.c_str(), title.c_str(), ::strlen(title.c_str())) == 0)
            {
                return i;
            }
            
            i++;
        }

        return FAIL;
    }

    int CREADCONF::MapType(const std::string strTitle)
    {
        for(int i = BEGIN+1; i < END; i++)
        {
            if(::strncasecmp(strTitle.c_str(), g_trans[i], ::strlen(g_trans[i])) == 0)
            {
                return i;
            }
        }

        return FAIL;
    }

}

