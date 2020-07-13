/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		config.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  配置管理
**********************************************************************************/

#ifndef _CONFIG_HPP
#define _CONFIG_HPP 1

#include <string>

#include "readconf.hpp"

namespace lmSOCKET
{
    class CCONFIG : virtual public CREADCONF
    {
		public:
            CCONFIG();
			CCONFIG(const char *configFile, const int runType);
			~CCONFIG();

            int CheckPara(void);
            
            config_t *GetConfig(void) const;
            int IsServer(void) const;
            int GetRunType(void) const;
            int GetCpuNum(void) const;
            int GetDevType(int index) const;
            socket_t GetSocketInfo(int index) const;
            com_t GetComInfo(int index) const;
            int GetDevFd(int index) const;
            void SetDevFd(int index, int fd);
            int GetTransIndex(int index) const;
            int IsUdp(int index) const ;
            int IsIpv4(int index) const;
            int IsDebugOn(void) const;
            int GetSameIpLimit(void) const;
            int GetTimeOut(const int index) const;

        protected:
            void LogParaPublic(void);
            void LogParaServer(void);
            void LogParaClient(void);
            int CheckParaPublic(void);
            int IsValidIPv4(const char *ipv4);
            int IsValidIPv6(const char *ipv6);
            int CheckParaServer(void);
            int CheckParaClient(void);

        private:
            int m_ret;
            
            static config_t *m_pconf;
    };
}

#endif
