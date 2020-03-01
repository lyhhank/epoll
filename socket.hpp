/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		base.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  根据用户配置启动SOCKET
**********************************************************************************/

#ifndef _SOCKET_HPP
#define _SOCKET_HPP 1

#include "fork.hpp"
#include "pthread.hpp"

/*
	SOCKET类
*/
namespace lmSOCKET
{   
    class CSOCKET : virtual public CCONFIG
    {
    	public:
    		CSOCKET(const char *configFile, const int runType);
    		~CSOCKET();
            int Start();
            
        private:
            int m_ret;
            
            CFORK *m_pfork;
            CPTH *m_pthread;
    };
}

#endif

