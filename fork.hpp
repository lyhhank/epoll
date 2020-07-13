/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		fork.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  进程功能管理
**********************************************************************************/

#ifndef _FORK_HPP
#define _FORK_HPP 1

#include "transfd.hpp"
#include "pthread_attr.hpp"

#include "drv_socket.hpp"
#include "drv_com.hpp"

/*
	FORK类
*/
namespace lmSOCKET
{
    class CFORK : virtual public CTRANSFD, virtual public CPTHATTR
    {
        public:
            CFORK();
            ~CFORK();
            int Run(void);
            int Wait(void);
            
            static void *ForkRun(void *arg);
            static CEPOLL *ForkGetDevType(void *arg);
            static void *ForkRunAccept(void *arg);
            static void *ForkRunRecv(void *arg);
            static void *ForkRunSend(void *arg);
            static void *ForkRunUserTask(void *arg);

        private:
            int Start(int index);

        private:
            int m_ret;

            CEPOLL *m_pepoll;
    };
}

#endif

