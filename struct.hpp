/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		struct.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  基本结构定义和参数量定义
**********************************************************************************/

#ifndef STRUCT_HPP
#define STRUCT_HPP 1

#include <sys/types.h> // size_t
#include <string>

// 函数返回值定义
const int OK            = 0;
const int CON           = 1;
const int FAIL          = -1;

enum msgType_e
{
    MSG_NOTYPE = '1',         // 无头
    MSG_FILE = '2',           // 文件
    MSG_FILE_END = '3',       // 有头文件结束
    MSG_MSG = '4',            // 消息
    MSG_UDP_REQ = '5',        // udp 请求连接
    MSG_UDP_RSP = '6',        // udp 响应连接
    MSG_UDP_RSPERR = '7',     // 请求错误或连接数量超过门限
};

enum
{
    BEGIN   = 0,
    ON,
    OFF,
    IPV4,
    IPV6,
    TCP,
    UDP,
    FORK,
    PTH,
    SOCKET,
    COM,
    SERVER,
    CLIENT,
    NONE,
    END,
};
const char* const g_trans[] = {
    "BEGIN",
    "ON",
    "OFF",
    "IPV4",
    "IPV6",
    "TCP",
    "UDP",
    "FORK",
    "PTH",
    "SOCKET",
    "COM",
    "SERVER",
    "CLIENT",
    "NONE",
    "END",
};

enum bufStatus_e
{
    BUFF_FREE = 1,
    BUFF_BUSY = 2,
    BUFF_NO   = 3,
};

enum transType_e
{
    TRANS_FD     = 1, 
    TRANS_MSG    = 2,
    TRANS_FDNUM  = 3,
    TRANS_SENDOFFLINE = 4,
    TRANS_END,
};

// 配置文件行最大长度
const int MAX_CONFFILE_LEN = 256;

// 文件名+路径最长
const int MAX_FILENAME_LEN = 128;

// 事件最大值
const int MAXEVENTS = 128;

// 最大监听数量
const int MAX_BACKLOG = 128;

// 每个数据包的最大长度
const unsigned int  USR_BUF_LEN = 1400;
const unsigned int  BUF_LEN = USR_BUF_LEN + 50; // 包头

// 粘包超时
const unsigned int  RS_TIMEOUT       = 500; // MS

// 接收异常重试次数
const unsigned int  TRYTIMES         = 2;

// 设备名称长度
const unsigned int  DEV_NAME_LEN     = 128;

// 客户端最长超时时间,30S的基数
const unsigned int  CLIENT_TIMEOUT_BASE = 30;

// CSOCKET 连接超时时间
const int CONNECT_TIMEOUT = 10;

struct online_t
{
    std::string ip;
    int port;
    long long onlineTime; // 上线时间  
};

struct socketAttr_t
{
    int reused;     // 地址端口重
	int ndelay;     // 延迟发
};

struct pthAttr_t
{
	int detachstate;	//线程的分离状态PTHREAD_CREATE_DETACHED, PTHREAD_CREATE_JOINABLE （默认）   
	int schedpolicy;	//线程调度策略，SCHED_FIFO，SCHED_RR ， SCHED_OTHER（默认）
	int schedpriority;	//线程的调度参数优先级，越小越低，范围sched_get_priority_max-sched_get_priority_min
	int inheritsched;	//线程的继承性， PTHREAD_INHERIT_SCHED（默认）， PTHREAD_EXPLICIT_SCHED
	int scope;			//线程的作用域，PTHREAD_SCOPE_PROCESS（默认），PTHREAD_SCOPE_SYSTEM
	size_t guardsize;	//线程栈末尾的警戒缓冲区大小，默认为1页
	size_t stacksize;	//线程栈的大小，本机最小PTHREAD_STACK_MIN(16K)，默认为8M
};

struct socket_t
{
    char addr[DEV_NAME_LEN];
    int  port;
    int  tcpudp;
    int  ipv4ipv6;
    int  flags;
    int  timeout;
};

struct com_t
{
    char dev[DEV_NAME_LEN];
    int  data;
    int  stop;
    int  check;
    int  baudrate;
    int  timeout;
};

union devtype_u
{
    socket_t    socket;
    com_t       com;
};

struct dev_st
{
    devtype_u devInfo;
    int       devType;    // 设备类型
    int       fd;
    int       transIndex;// 转发INDEX
};

struct config_t
{
    dev_st         dev[MAXEVENTS];

    socketAttr_t   socketAttr;
    pthAttr_t      pthAttr;
    
    int            cs;         // 服务器，客户端
    int            runType;    // 运行模式
    int            cpuNum;     // 数量
    int            debug;
    int            sameIpLimit;
    int            clientTask;
};

#endif
