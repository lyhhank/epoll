/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		base.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  基类，完成析构，日志
**********************************************************************************/

#ifndef _BASE_HPP
#define _BASE_HPP 1

#include "struct.hpp"

namespace lmSOCKET
{
//调试日志　                
extern  int g_debug;
#define log(format,...)     if(g_debug)printf("%s,%d:" format "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#define logerr(format,...)  if(g_debug)printf("%s,%d:" format ",errno=%d\n", __FILE__, __LINE__, ##__VA_ARGS__,errno);

#define CHECKRESULT(m_ret)  \
    do{                     \
        if(m_ret)           \
        {                   \
            return m_ret;   \
        }                   \
    }while(0)           

#define CHECKRANG(para, min, max)           \
    do{                                     \
        if((para < min) || (para > max))    \
        {                                   \
            log("%s config err", #para);    \
            return FAIL;                    \
        }                                   \
    }while(0)           

	class CBASE
	{
		public:
			CBASE();
			virtual ~CBASE();
            
	};
}

#endif
