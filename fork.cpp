/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		fork.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  进程设置，根据配置启动多进程EPOLL
**********************************************************************************/


#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // wiatpid()
#include <unistd.h>     // getpid()

#include "fork.hpp"

namespace lmSOCKET
{ 
    CFORK::CFORK()
	{
        m_ret = OK;
    }
    
    CFORK::~CFORK()
    {
        
    }

    int CFORK::Wait()
    {
        while (1)
        {
            // Wait()函数的返回值是子进程的pid
            m_ret = ::wait(NULL);
            log("pid = %d\n", m_ret);
            if(m_ret == FAIL)
            {
                // 父进程wait()函数阻塞过程中，有可能被别的信号中断，需要做异常处理
                if(errno == EINTR)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
        }

        log("Wait exit");
        return FAIL;
    }

    int CFORK::Run()
    {
        CHECKRESULT(m_ret);

        if(SetAttr() != OK)
        {
            return FAIL;
        }

        int endindex = IsServer() ? GetCpuNum()+1 : GetCpuNum(); // server + Accept

        for (int i = 0; i < endindex; i++)
        {
            pid_t pid = ::fork();
            if(pid < 0)
            {
                logerr("fork err return %d", pid);
                return FAIL;
            }
            else if (pid == 0)    // 子进程
            {
                return Start(i);
            }
            else 
            {
                continue;
            }
        }

        return Wait();
    }

    int CFORK::Start(int index)
    {
        switch(GetDevType(index))
        {
            case SOCKET:
                m_pepoll = new DRV_SOCKET();
                break;
            case COM:
                m_pepoll = new DRV_COM();
                break;
            default:
                if(m_pepoll == NULL)
                {
                    log("new CEPOLL err");
                    return FAIL;
                }
        }
        
        m_pepoll->m_index = index;
        ForkRun(m_pepoll);
        
        delete m_pepoll;
        return FAIL;
    }
    
    void *CFORK::ForkRun(void *arg)
    {
       CEPOLL *ep = ForkGetDevType(arg);

       int index = ep->m_index;
       if((index == ep->GetCpuNum()) && (ep->IsServer()))
       {
           ForkRunAccept(arg);
       }
       else
       {
            int err = 0;
            int pthNum = 3;
            pthread_t tid[pthNum];

            pthNum = 2;
            err |= ::pthread_create(&tid[0], m_pattr, ForkRunRecv, arg);
            err |= ::pthread_create(&tid[1], m_pattr, ForkRunSend, arg);
            if(ep->GetConfig()->clientTask == ON)
            {
                pthNum = 3;
                err |= ::pthread_create(&tid[2], m_pattr, ForkRunUserTask, arg);
            }
            if(err)
            {
                logerr("pthread_create fail");
                return NULL;
            }
            
            int detachstate = ep->GetConfig()->pthAttr.detachstate;
            for(int i = 0; i < pthNum; i++)
            {
                void *ret;
                if(detachstate == PTHREAD_CREATE_JOINABLE)
                {
                    int err = ::pthread_join(tid[i], &ret);
                    if(err)
                    {
                        logerr("pthread_join fail");
                        return NULL;
                    }
                }
                else
                {
                    int err = ::pthread_detach(tid[i]);
                    if(err)
                    {
                        logerr("pthread_detach fail");
                        return NULL;
                    }
                }
            }
       }
       
       return NULL;
   }

   CEPOLL *CFORK::ForkGetDevType(void *arg)
   {
        CEPOLL *ep = (CEPOLL *)arg;
        int index = ep->m_index;
        if(ep->GetDevType(index) == SOCKET)
        {
            ep = (DRV_SOCKET *)arg;
        }
        else if(ep->GetDevType(index) == COM)
        {
            ep = (DRV_COM *)arg;
        }

        return ep;
   }

   void *CFORK::ForkRunAccept(void *arg)
   {
       CEPOLL *ep = ForkGetDevType(arg);

       ep->EpollAccept();
       return NULL;
   }

   void *CFORK::ForkRunRecv(void *arg)
   {
       CEPOLL *ep = ForkGetDevType(arg);

       ep->EpollRecv();
       return NULL;
   }

   void *CFORK::ForkRunSend(void *arg)
   {
       CEPOLL *ep = ForkGetDevType(arg);

       ep->EpollSend();
       return NULL;
   }

   void *CFORK::ForkRunUserTask(void *arg)
   {
       CEPOLL *ep = ForkGetDevType(arg);

       ep->EpollUserTask();
       return NULL;
   }
}

