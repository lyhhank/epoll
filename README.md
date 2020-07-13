# epoll
epoll server client

程序功能：

1，SOCKET服务器，客户端功能集中化
   	大多数SOCKET程序服务器，客户端分离设计，这样需要管理2套代码，不利于管理和代码重用。

2，支持IPV4, IPV6
	可配置的IPV4, IPV6启动方式。

3, 支持TCP, UDP
	支持TCP、UDP的客户端，服务器模式，统一架构管理。

4, 支持包头，无包头设置
	支持设置用户数据包头，以解决粘包，分包。

5，支持消息，文件方式
	包括SOCKET, COM都支持文件，消息方式。

6，客户端，服务端口统一使用EPOLL架构，方便管理
	EPOLL是目前效率最高的通信方式，客户端，服务器统一使用此方式。

7，支持进程，线程方式启动程序
	不用考虑设计线程、进程对程序的影响，可以选择最合适、最有效率的方式启动。

8，服务器支持限制同IP客户端上线数量
	支持限制同IP客户端数量，防止恶意连接。

9，支持其它驱动接入，目前有SOCKET和串口，只需要一个驱动程序
	支持其它方式，只需要增加相应的驱动，目前已经支持SOCKET和COM。

10，客户端支持数据转发
	非SOCKET可以和SOCKET转发，比如SOCKET，COM

11，支持最大进程、线程数量为128
	进程，线程数量，可根据实际工作场景设置，即进入进程、线程池方式，以减少进程线程申请释放的资源使用。

12，服务器进程、线程间负载均衡
	服务器中的进程，线程，客户端接入后，总是均衡的分布在不同的进程，线程上。

13，支持断线重连
	客户端断线后，可自动重连。

14，支持超时下线客户端
	服务器在客户端口超时未收到消息后，下线客户端。

15，内存管理
	集中内存管理，以减少申请释放的系统资源占用和风险，同时提高效率。

16，进程线程属性
	如地址端口的重用、线程大小等，以适应不同的内存使用。

18，接口简单
	客户端超时通知，接收通知，发送只需要调用一下发送函数。在设置包头的情况下，架构保证每次传用的是一个完整的包。

19，支持配置文件
	所有的配置都在配置文件就可以完成，比如进程，线程，TCP，UDP等。
	
20，关于分布式
	程序简单做一个协议，就可以实现，比如设置一个管理主机，应答分布式机器的地址。

使用：
	SERVER: ./epcs server ./config.txt
	CLIENT: ./epcs client ./config.txt
		
