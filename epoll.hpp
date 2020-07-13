/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		epoll.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  EPOLL接收，发送，ACCEPT, 用户程序调用等
**********************************************************************************/

#ifndef _EPOLL_HPP
#define _EPOLL_HPP 1

#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timerfd.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "shm.hpp"
#include "buffer.hpp"
#include "ep.hpp"
#include "transfd.hpp"
#include "usrapp.hpp"
#include "socket_attr.hpp"

#include <map>
#include <vector>
#include <algorithm>

namespace lmSOCKET
{
    // 表示消息长度的字节数,要和类型一致
    const unsigned short MSG_LEN_BYTE = 2;
    
    int comp(const std::pair<int, int> &lhs, const std::pair<int, int> &rhs);
    
	class CEPOLL : virtual public CSOCKETATTR, virtual public CTRANSFD, public CUSR
	{
		public:
			CEPOLL();
            ~CEPOLL();

            // 任务入口
            int EpollAccept(void);
            int EpollRecv(void);
            int EpollSend(void);  
            int EpollUserTask(void);

            // 子类实现
            virtual int Create(const int index) = 0;
            virtual int Send(const int fd, char *sendbuf, const int sendlen, const int flag) = 0;
            virtual int Recv(const int fd, char *sendbuf, const int sendlen, const int flag) = 0;
            virtual std::string GetMsgHead(void) const = 0;
            virtual std::string GetFileHead(void) const = 0;

            // 提供给USER的接口
            char *GetSendBuf(const int fd) const;
            int ClientGetFd(void) const;
            int StartSendMsg(const int fd, const int len);
            int StartSendFile(const int fd, const std::string &fileName, const size_t offset);
            
        protected:
            // ACCEPT
            void Accept(const int fd);
            int AcceptInit(void);
            int GetLoadIndex(void) const;
            int SetLoadIndex(void);
            int AcceptMsg(void);
            int LoadBlance(const int fd, std::string &ip, unsigned int &port, struct sockaddr_storage *p_tcpudp_addr, socklen_t &tcpudp_addr_len);
            int SameIpLimit(std::string &ip, unsigned int &port, struct sockaddr_storage *p_tcpudp_addr);
            int AcceptUdp(const int fd, struct sockaddr_storage *p_tcpudp_addr, socklen_t &tcpudp_addr_len);
            int GetIpPort(std::string &ip, unsigned int &port, struct sockaddr_storage *p_tcpudp_addr);
            int AcceptUdpErr(const int fd, struct sockaddr_storage *p_tcpudp_addr, socklen_t &tcpudp_addr_len);
            int RspUdpConnect(const int fd, struct sockaddr_storage *p_tcpudp_addr, socklen_t &tcpudp_addr_len);

            // CS
            int CreateServer(int index);
            int CreateClient(int index);
            int SetSocketAttr(int index, const int fd);
            int GetIpv6Index(int index, unsigned      &ifindex);
            int UdpConnect(const int fd, struct sockaddr_storage *p_tcpudp_addr);

            // SEND
            void FillHead(msgType_e type);
            int FillBody();
            int SendInit(void);
            int SendData(const int fd);
            int GetSlotIndex(void);
            int OnlineUpdate(const int fd, const std::string &ip, const unsigned int port);
            void RecvUpdate(const int fd);
            void OfflineUpdate(const int fd, const int index);
            int GetFdIndex(const int fd, const int new_index, const int delete_flag);
            
            // RECV
            int GetMsgType(void);
            int MoveData(void);
            int DealData(int fd);
            int RecvData(const int fd);
            int RecvConnect(void);
            int RecvInit(void);
            int RecvCs(const int fd, char *recvbuf, const int recvlen, const msgType_e type);
            int TimerInit(void);
            void TimeoutSec(const int fd);
                
            // CEPOLL
            void IgSignal(void) const;
            int MsgHeadInit(void);
            void GetHead(msgType_e type, std::string &head);
            int PthOffline(const int fd);
            int PthOnline(const int fd);
            int ForkOffline(const int fd);
            int ForkOnline(const int fd);
            int Offline(const int fd);
            int Online(const int fd, const std::string &ip, const unsigned int port);

            // TRANS
            int SendOnlineFd(const int fd, const std::string &ip, const unsigned int &port, const int index);
            int SendOfflineFd(const int index, const std::string &ip);
            int RecvTransMsg(void);
            int DealTransMsg(const int fd, char *buf, const int len);
            int RecvTransFd(const int fd, char *buf, const int len);
            int RecvTransMsgSend(char *buf, const int len);
            int SendTransFdNum(char *buf, int len);
            int RecvTransOffLineFd(const int fd, char *buf, const int len);
            int SendTransOfflineFd(const int fd); // TODO        需要 ???
		private:
            int m_ret;

            CBUFFER *m_pRecvBuf; // BUF
            CBUFFER *m_pSendBuf; // BUF
        
            CEP *m_pAcceptEp;    // EP对象
            CEP *m_pRecvEp;
            CEP *m_pSendEp;

            int m_haveHead;
            std::string m_msgHead;
            std::string m_fileHead;
            std::string m_fileEnd;

            std::vector<std::pair<int, int>> m_FdNumInLoad; // 每个进程的负载数量

            std::map<std::string, int> m_SameIpLimit; // ip, num;
            std::map<std::string, int>::iterator m_iterSameIpLimit;

            unsigned int  m_curSlot; // 时间轮
            std::map<int, int> m_fdIndex; // fd->index
            std::map<int, int>::iterator m_iterFdIndex;

            std::map<int, online_t> m_online[CLIENT_TIMEOUT_BASE]; // fd ip
            std::map<int, online_t>::iterator m_iterOnline;

            int m_timerFd; // 1S定时器
            
        public:
            int m_index; // 线程编号
	};
}

#endif


