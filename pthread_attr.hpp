/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		base.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  线程属性管理
**********************************************************************************/

#ifndef _PTHREAD_ATTR_HPP
#define _PTHREAD_ATTR_HPP 1

#include <pthread.h>

#include "config.hpp"

/*
	SOCKET类
*/
namespace lmSOCKET
{   
    class CPTHATTR : virtual public CCONFIG
    {
    	public:
    		CPTHATTR();
    		~CPTHATTR();

            int GetAttr(pthAttr_t *attr);
            int SetAttr(void);

        protected:
            static pthread_attr_t *m_pattr;
            
        private:
            int m_ret;
            static int m_ref; // 计数
    };
}

#endif

