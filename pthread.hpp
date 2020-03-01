/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		base.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  线程功能管理
**********************************************************************************/

#ifndef _PTHREAD_HPP
#define _PTHREAD_HPP 1

#include "transfd.hpp"
#include "pthread_attr.hpp"

#include "drv_socket.hpp"
#include "drv_com.hpp"

namespace lmSOCKET
{
	class CPTH : virtual public CTRANSFD, virtual public CPTHATTR
    {
		public:
			CPTH();
			~CPTH();

			/*
			功能：线程启动函数
			返回值：成功返回0
			*/
            int Run(void);

        protected:
			/*
			功能：信号发送函数
			返回值：成功返回0
			*/
			int Kill(pthread_t pth, int signal);

			/*
			功能：线程分离，默认是joinable
			返回值：成功返回0
			*/
			int Detach(pthread_t pth = 0);

			/*
			功能：线程退出，可以是别人
			返回值：成功返回0
			*/
			int Cancel(pthread_t pth = 0);

			/*
			功能：线程加入
			返回值：成功返回0
			*/
			int Join(pthread_t pth, void *ret = NULL);

			/*
			功能：获取线程自己ID
			返回值：成功返回0
			*/
			pthread_t Self(void);

            static void *PthRun(void *arg);
            static CEPOLL *PthGetDevType(void *arg);
            static void *PthRunAccept(void *arg);
            static void *PthRunRecv(void *arg);
            static void *PthRunSend(void *arg);
            static void *PthRunUserTask(void *arg);
			
		private:
            int m_ret;

            CEPOLL *m_pepoll[MAXEVENTS];
            pthread_t m_tid[MAXEVENTS];
	};
}

#endif


