/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		base.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  SOCKETPARE通信
**********************************************************************************/

#ifndef _TRANSFD_HPP
#define _TRANSFD_HPP 1

#include "struct.hpp"

#include "config.hpp"

namespace lmSOCKET
{
    const int MSG_LEN_INDEX  = 0;
    const int MSG_TYPE_INDEX = 1;
    const int MSG_INDEX      = 2;
    const int MSG_VEC_NUM    = 3;
    const int MSG_INT_LEN    = sizeof(int);
    
	class CTRANSFD : virtual public CCONFIG
	{
		public:
			CTRANSFD();
			virtual ~CTRANSFD();
            
		protected:

            int SendFd(int index, const transType_e type, const int fd, char *buf, const int buflen);
            int RecvFd(int index, int &fd, char *buf, int &buflen);
            int GetSendFd(int index);
            int GetRecvFd(int index);

        private:
            int m_ret;
            
            static int *m_psocketFd;
            int m_cpuNum;
            
	};
}

#endif
