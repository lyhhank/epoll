/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		epoll_cs.cpp
  Author: 		limeng(wx:18180447276)
  Version:  	v1.0
  Date:			2020.3.1
  Description:  EPOLL SERVER, CLIENT建立
**********************************************************************************/


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <string.h>
#include <sys/time.h>
#include <signal.h>

#include "epoll.hpp"

#include <unistd.h>
#include <fcntl.h>

namespace lmSOCKET
{
    int CEPOLL::GetIpv6Index(int index, unsigned int &ifindex)
    {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];
        
        if(::getifaddrs(&ifaddr) != OK) 
        {
            logerr("getifaddrs err");
            return FAIL;
        }

        for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if(ifa->ifa_addr == NULL)
            {
                continue;
            }
        
            family = ifa->ifa_addr->sa_family;
            if(family == AF_INET || family == AF_INET6)
            {
                s = ::getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != OK)
                {
                    logerr("getnameinfo failed: %s", ::gai_strerror(s));
                    ::freeifaddrs(ifaddr);
                    return FAIL;
                }

                socket_t socket = GetSocketInfo(index);
                if(::memcmp(socket.addr, host, strlen(socket.addr)) == 0)
                {
                    ifindex = ::if_nametoindex(ifa->ifa_name);
                    if(ifindex == 0)
                    {
                        logerr("if_nametoindex err");
                        ::freeifaddrs(ifaddr);
                        return FAIL;
                    }
                    
                    ::freeifaddrs(ifaddr);
                    return OK;
                }
            }
        }

        ::freeifaddrs(ifaddr);
        return FAIL;
    }

    int CEPOLL::SetSocketAttr(int index, const int fd)
    {
        m_ret = SetAttr(index, fd);
        if(m_ret != OK)
        {
            log("fd = %d, SetAttr err", fd);
            return FAIL;
        }

        return OK;
    }

    int CEPOLL::UdpConnect(const int fd, struct sockaddr_storage *p_tcpudp_addr)
    {
        if(IsIpv4(m_index))
        {
            m_ret = ::connect(fd, (struct sockaddr*)(p_tcpudp_addr), sizeof(struct sockaddr_in));
        }
        else
        {
            m_ret = ::connect(fd, (struct sockaddr*)(p_tcpudp_addr), sizeof(struct sockaddr_in6));
        }

        return m_ret;
    }

    int CEPOLL::CreateServer(int index)
    {
        int fd = 0, domain = 0, type = 0;
        struct addrinfo hintss = {0}, *result = NULL, *result_check = NULL;

        socket_t socket = GetSocketInfo(index);
        const char *bind_addr = socket.addr;
        char bind_port_tmp[10] = {0};
        sprintf(bind_port_tmp, "%d", socket.port);
        const char *bind_port = bind_port_tmp;
        
        int flags = socket.flags;
        flags |= SOCK_NONBLOCK;
        
        switch (socket.tcpudp)
        {
            case TCP:
                type = SOCK_STREAM;
                break;
            case UDP:
                type = SOCK_DGRAM;
                break;
            default:
                log("tcpudp set err");
                return FAIL;
        }
        
        switch (socket.ipv4ipv6)
        {
            case IPV4:
                domain = AF_INET;
                break;
            case IPV6:
                domain = AF_INET6;
                break;
            /*
            case BOTH:
                domain = AF_UNSPEC;
                break;
                */
            default:
                log("ipv4ipv6 set err");
                return FAIL;
        }

        ::memset(&hintss, 0, sizeof(struct addrinfo));
        hintss.ai_socktype = type;
        hintss.ai_family = domain;
        hintss.ai_flags = AI_PASSIVE;

        m_ret = ::getaddrinfo(bind_addr, bind_port, &hintss, &result);
        if(m_ret != OK)
        {
            logerr("getaddrinfo err");
            return FAIL;
        }

        for (result_check = result; result_check != NULL; result_check = result_check->ai_next )
        {
            fd = ::socket(result_check->ai_family, result_check->ai_socktype | flags, result_check->ai_protocol);
            if (fd == FAIL)
            {
                if(errno != EINPROGRESS)
                {
                    continue;
                }
            }

            // 设置SOCKET属性
            if(SetSocketAttr(index, fd) != OK)
            {
                ::shutdown(fd, SHUT_WR);
                ::close(fd);
                ::freeaddrinfo(result);
                log("SetSocketAttr err");
                return FAIL;
            }

            // https://www.linuxidc.com/Linux/2013-06/85690.htm
            if(socket.ipv4ipv6 == IPV6) // 本地的IPV6需要填sin6_scope_id
            {
                struct sockaddr_in6 *sockaddr_ipv6 = (struct sockaddr_in6 *)result_check->ai_addr;
                GetIpv6Index(index, sockaddr_ipv6->sin6_scope_id);
            }
            
            m_ret = ::bind(fd, result_check->ai_addr, (socklen_t)result_check->ai_addrlen);
            if(m_ret != OK)
            {
                ::shutdown(fd, SHUT_WR);
                ::close(fd);
                continue;
            }
            else
            {
                if (socket.tcpudp == TCP) // udp 为无连接流, 不需要listen
                {
                    m_ret = ::listen(fd, MAX_BACKLOG);
                    if (m_ret == OK )   // 创建服务器成功
                    {
                        break;
                    }
                    else
                    {
                        ::shutdown(fd, SHUT_WR);
                        ::close(fd);
                        continue;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        ::freeaddrinfo(result);

        if (result_check == NULL)
        {
            log("Could not bind to any address");
            return FAIL;
        }

        return fd;
    }

    int CEPOLL::CreateClient(int index)
    {
        socket_t socket = GetSocketInfo(index);
        
        int fd = 0, type = 0, n = 0;
        struct addrinfo hints = {0}, *result = NULL, *result_check = NULL;

        const char *host    = socket.addr;

        char service_tmp[10] = {0};
        sprintf(service_tmp, "%d", socket.port);
        const char *service = service_tmp;

        int flags = socket.flags;
        flags |= SOCK_NONBLOCK;

        switch (socket.tcpudp)
        {
            case TCP:
                type = SOCK_STREAM;
                break;
            case UDP:
                type = SOCK_DGRAM;
                break;
            default:
                log("proto_osi4 is not TCP or UDP");
                return FAIL;
        }

        ::memset(&hints, 0, sizeof(hints));
        switch (socket.ipv4ipv6)
        {
            case IPV4:
                hints.ai_family = AF_INET;
                break;
            case IPV6:
                hints.ai_family = AF_INET6;
                break;
            /*
            case BOTH:
                hints.ai_family = AF_UNSPEC;
                break;
                */
            default:
                log("proto_osi3 is not IPV4 or IPV6");
                return FAIL;
        }

        hints.ai_socktype = type;
        m_ret = ::getaddrinfo(host, service, &hints, &result);
        if(m_ret != OK)
        {
            logerr("getaddrinfo err");
            return FAIL;
        }

        for (result_check = result; result_check != NULL; result_check = result_check->ai_next )
        {
            fd = ::socket(result_check->ai_family, result_check->ai_socktype | flags, result_check->ai_protocol);
            if (fd == FAIL)
            {
                if(errno != EINPROGRESS)
                {
                    continue;
                }
            }

            // 设置SOCKET属性
            if(SetSocketAttr(index, fd) != OK)
            {
                ::shutdown(fd, SHUT_WR);
                ::close(fd);
                ::freeaddrinfo(result);
                return FAIL;
            }

            // https://www.linuxidc.com/Linux/2013-06/85690.htm
            if(socket.ipv4ipv6 == IPV6) // 本地的IPV6需要填sin6_scope_id
            {
                struct sockaddr_in6 *sockaddr_ipv6 = (struct sockaddr_in6 *)(result_check->ai_addr);
                GetIpv6Index(index, sockaddr_ipv6->sin6_scope_id);
            }

            m_ret = ::connect(fd, result_check->ai_addr, result_check->ai_addrlen);
            if (m_ret == OK) // 连接成功
            {
                break;
            }
            else    // 检测连接状态
            {
                for(n = 0; n < CONNECT_TIMEOUT; n++)
                {
                    ::sleep(1);
                    m_ret = ::connect(fd, result_check->ai_addr, result_check->ai_addrlen);
                    if((m_ret == OK) || (errno == EISCONN))    // 成功
                    {
                        break;
                    }
                    else if((errno == EINPROGRESS) || (errno == EALREADY)) // 连接中
                    {
                        continue;
                    }
                    else
                    {
                        logerr("other connect err %d", m_ret); // 其它错误
                        continue;
                    }
                }
            }

            if(n != CONNECT_TIMEOUT) // 连接成功
            {
                break;
            }
            else // 失败
            {
                ::shutdown(fd, SHUT_WR);
                ::close(fd);
                continue;
            }
        }

        ::freeaddrinfo(result);
        
        if ((result_check == NULL )  || (n == CONNECT_TIMEOUT))
        {
            log("Could not connect to any address");
            return FAIL;
        }

        return fd;
    }


}


