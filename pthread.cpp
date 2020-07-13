/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		pthread.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  线程设置，根据配置启动多线程EPOLL.
**********************************************************************************/


#include <signal.h>
#include <limits.h>

#include "pthread.hpp"

namespace lmSOCKET
{
    CPTH::CPTH()
	{
        m_ret = OK;
    }

	CPTH::~CPTH()
	{

	}

    int CPTH::Run()
    {
        CHECKRESULT(m_ret);
        
        if(SetAttr() != OK)
        {
            return FAIL;
        }

        int endindex = IsServer() ? GetCpuNum()+1 : GetCpuNum(); // server + Accept

        for(int index = 0; index < endindex; index++)
		{
            switch(GetDevType(index))
            {
                case SOCKET:
                    m_pepoll[index] = new DRV_SOCKET();
                    break;
                case COM:
                    m_pepoll[index] = new DRV_COM();
                    break;
                default:
                    m_pepoll[index] = NULL;
                    log("new CEPOLL err");
                    return FAIL;
            }

            m_pepoll[index]->m_index = index;
			m_ret = ::pthread_create(&m_tid[index], m_pattr, PthRun, m_pepoll[index]);
            if(m_ret != OK)
            {
                logerr("pthread_create return %d", m_ret);
                return FAIL;
            }
		}

        int detachstate = GetConfig()->pthAttr.detachstate;
        for(int index = 0; index < endindex; index++)
        {
            if(detachstate == PTHREAD_CREATE_JOINABLE)
            {
                m_ret = Join(m_tid[index]);
                CHECKRESULT(m_ret);
            }
            else
            {
                m_ret = Detach(m_tid[index]);
                CHECKRESULT(m_ret);
            }
        }

        for(int index = 0; index < GetCpuNum(); index++)
        {
            delete m_pepoll[index];
        }
        
        return OK;
    }

	int CPTH::Kill(pthread_t pth, int signal)
	{
		if(pth == 0)
		{
			m_ret = ::pthread_kill(Self(), signal);
            if(m_ret != OK)
            {
                logerr("pthread_kill return %d", m_ret);
            }
		}
		else
		{
			m_ret = ::pthread_kill(pth, signal);
            if(m_ret != OK)
            {
                logerr("pthread_kill return %d", m_ret);
            }
		}
        
        return m_ret;
	}

	int CPTH::Detach(pthread_t pth)
	{
		if(pth)
		{
			m_ret = ::pthread_detach(pth); 
            if(m_ret != OK)
            {
                logerr("pthread_detach return %d", m_ret);
            }
		}
		else
		{
			m_ret = ::pthread_detach(Self()); 
            if(m_ret != OK)
            {
                logerr("pthread_detach return %d", m_ret);
            }
		}

        return m_ret;
	}
	
	int CPTH::Cancel(pthread_t pth)
	{
		if(pth)
		{
			m_ret = ::pthread_cancel(pth); 
            if(m_ret != OK)
            {
                logerr("pthread_cancel return %d", m_ret);
            }
		}
		else
		{
			m_ret = ::pthread_cancel(Self()); 
            if(m_ret != OK)
            {
                logerr("pthread_cancel return %d", m_ret);
            }
		}

        return m_ret;
	}
	
	int CPTH::Join(pthread_t pth, void *ret)
	{
		if(pth)
		{
			m_ret = ::pthread_join(pth, &ret); 
            if(m_ret != OK)
            {
                logerr("pthread_join return %d", m_ret);
            }
		}
		else
		{
			m_ret = ::pthread_join(Self(), &ret); 
            if(m_ret != OK)
            {
                logerr("pthread_join return %d", m_ret);
            }
		}

        return m_ret;
	}
	
	pthread_t CPTH::Self(void)
	{
		m_ret = ::pthread_self();
        if(m_ret != OK)
        {
            logerr("pthread_self return %d", m_ret);
        }
        
		return m_ret;
	}

    void *CPTH::PthRun(void *arg)
	{
        CEPOLL *ep = PthGetDevType(arg);

        int err = 0;
        int pthNum = 3;
        pthread_t tid[pthNum];
        
        int index = ep->m_index;
        if((index == ep->GetCpuNum()) && (ep->IsServer()))
        {
            pthNum = 1;
            // Accept 线程
            int err = ::pthread_create(&tid[0], m_pattr, PthRunAccept, arg);
            if(err)
            {
                logerr("PthRun pthread_create err");
                return NULL;
            }
        }
        else
        {
            pthNum = 2;
            err |= ::pthread_create(&tid[0], m_pattr, PthRunRecv, arg);
            err |= ::pthread_create(&tid[1], m_pattr, PthRunSend, arg);
            if(ep->GetConfig()->clientTask == ON)
            {
                pthNum = 3;
                err |= ::pthread_create(&tid[2], m_pattr, PthRunUserTask, arg);
            }
            if(err)
            {
                logerr("PthRun pthread_create err");
                return NULL;
            }
        }

        int detachstate = ep->GetConfig()->pthAttr.detachstate;
        for(int i = 0; i < pthNum; i++)
        {
            void *ret;
            if(detachstate == PTHREAD_CREATE_JOINABLE)
            {
                err = ::pthread_join(tid[i], &ret);
                if(err)
                {
                    logerr("PthRun pthread_join err");
                    return NULL;
                }
            }
            else
            {
                err = ::pthread_detach(tid[i]);
                if(err)
                {
                    logerr("PthRun pthread_detach err");
                    return NULL;
                }
            }
        }

        return NULL;
    }

   CEPOLL *CPTH::PthGetDevType(void *arg)
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

    void *CPTH::PthRunAccept(void *arg)
    {
        CEPOLL *ep = PthGetDevType(arg);

        ep->EpollAccept();
        return NULL;
    }

    void *CPTH::PthRunRecv(void *arg)
    {
        CEPOLL *ep = PthGetDevType(arg);

        ep->EpollRecv();
        return NULL;
    }

    void *CPTH::PthRunSend(void *arg)
    {
        CEPOLL *ep = PthGetDevType(arg);
        
        ep->EpollSend();
        return NULL;
    }

    void *CPTH::PthRunUserTask(void *arg)
    {
        CEPOLL *ep = PthGetDevType(arg);

        ep->EpollUserTask();
        return NULL;
    }
}

