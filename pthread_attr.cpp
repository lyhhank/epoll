/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		pthread_attr.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  线程基本ATTR设置
**********************************************************************************/

#include "pthread_attr.hpp"

namespace lmSOCKET
{
    pthread_attr_t* CPTHATTR::m_pattr = NULL;

    CPTHATTR::CPTHATTR()
    {
        m_ret = OK;
        
        if(m_pattr == NULL)
        {
            m_pattr = new pthread_attr_t();
            if(m_pattr == NULL)
            {
                log("new pthread_attr_t err!");
                m_ret = FAIL;
                return;
            }
        }
    }

    CPTHATTR::~CPTHATTR()
    {   
        if(m_pattr)
        {
            m_ret = ::pthread_attr_destroy(m_pattr);
            if(m_ret != OK)
            {
                logerr("pthread_attr_destroy err");
            }
            delete m_pattr;
            m_pattr = NULL;
        }
    }

    int CPTHATTR::GetAttr(pthAttr_t *attr)
    {
        CHECKRESULT(m_ret);
    
        sched_param sch = {attr->schedpriority};
        
        m_ret = ::pthread_attr_init(m_pattr);
        if(m_ret)
        {
            logerr("pthread_attr_init err return %d", m_ret);
            return FAIL;
        }
        
        m_ret |= ::pthread_attr_getdetachstate(m_pattr, &attr->detachstate);
        m_ret |= ::pthread_attr_getschedpolicy(m_pattr, &attr->schedpolicy);
        m_ret |= ::pthread_attr_getschedparam(m_pattr, &sch);
        m_ret |= ::pthread_attr_getinheritsched(m_pattr, &attr->inheritsched);
        m_ret |= ::pthread_attr_getscope(m_pattr, &attr->scope);
        m_ret |= ::pthread_attr_getguardsize(m_pattr, &attr->guardsize);
        m_ret |= ::pthread_attr_getstacksize(m_pattr, &attr->stacksize);
        if(m_ret)
        {
            logerr("pthAttr get err return %d", m_ret);
            return FAIL;
        }

        return OK;
    }

    int CPTHATTR::SetAttr(void)
    {
        CHECKRESULT(m_ret);
        
        pthAttr_t attr = GetConfig()->pthAttr;
    
        m_ret = ::pthread_attr_init(m_pattr);
        if(m_ret)
        {
            logerr("pthread_attr_init err return %d", m_ret);
            return FAIL;
        }

        m_ret = ::pthread_attr_setdetachstate(m_pattr, attr.detachstate);
        if(m_ret)
        {
            logerr("pthread_attr_setdetachstate err return %d", m_ret);
            return FAIL;
        }

        m_ret = ::pthread_attr_setschedpolicy(m_pattr, attr.schedpolicy);
        if(m_ret)
        {
            logerr("pthread_attr_setschedpolicy err return %d", m_ret);
            return FAIL;
        }

        const sched_param sch = {attr.schedpriority};
        m_ret = ::pthread_attr_setschedparam(m_pattr, &sch);
        if(m_ret)
        {
            logerr("schedpriority err return %d", m_ret);
            return FAIL;
        }

        m_ret = ::pthread_attr_setinheritsched(m_pattr, attr.inheritsched);
        if(m_ret)
        {
            logerr("pthread_attr_setinheritsched err return %d", m_ret);
            return FAIL;
        }   

        m_ret = ::pthread_attr_setscope(m_pattr, attr.scope);
        if(m_ret)
        {
            logerr("pthread_attr_setscope err return %d", m_ret);
            return FAIL;
        }   

        m_ret = ::pthread_attr_setguardsize(m_pattr, attr.guardsize);
        if(m_ret)
        {
            logerr("pthread_attr_setguardsize err return %d", m_ret);
            return FAIL;
        }

        m_ret = ::pthread_attr_setstacksize(m_pattr, attr.stacksize);
        if(m_ret)
        {
            logerr("pthread_attr_setstacksize err return %d", m_ret);
            return FAIL;
        }

        return OK;
    }
}
