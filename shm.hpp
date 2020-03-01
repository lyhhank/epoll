#ifndef _SHM_HPP
#define _SHM_HPP 1

#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <netinet/in.h>

#include "config.hpp"

/*
	共享内存：适用于多进程通信，不会读清
*/
namespace lmSOCKET
{
	class SHM: virtual public CCONFIG
	{
		public:
			SHM();
			~SHM();
			
			/*
			功能：写共享内存数据函数
			返回值：成功返回0
			*/
			int shmwrite(int index, struct sockaddr_storage *p_tcpudp_addr);

			/*
			功能：读共享内存数据函数
			返回值：成功返回0
			*/
			struct sockaddr_storage *shmread(int index);

		private:
			void init(void);
            
			int m_ret;
			int m_shm_create;
			int m_shm_shmid;//共享内存ID
			char *m_shm_p;//共享内存数据指针
            unsigned int m_shm_size;//共享内存size
			int m_shm_fd[2];//读写共享内存句柄
			char m_shm_fn[100];//保存有名共享内存文件名
	};
}

#endif

