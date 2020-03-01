/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		base.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  进程属性管理
**********************************************************************************/

#ifndef _SOCKET_ATTR_HPP
#define _SOCKET_ATTR_HPP 1

#include "config.hpp"

/*
	SOCKET类
*/
namespace lmSOCKET
{   
    class CSOCKETATTR : virtual public CCONFIG
    {
    	public:
    		CSOCKETATTR();
    		~CSOCKETATTR();

            int SetAttr(int index, int fd);

        private:
            int m_ret;
    };
}

#endif

