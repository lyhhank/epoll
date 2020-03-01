#include <iostream>
#include <sys/types.h>
#include <string.h>

#include "config.hpp"
#include "socket.hpp"
#include "readconf.hpp"

void StartCs(const char *configFile, const int runType)
{
    lmSOCKET::CSOCKET *psocket = new lmSOCKET::CSOCKET(configFile, runType);

    // 配置
    if(psocket->CheckPara() != OK)
    {   
        return;
    }   

    // 启动SOCKET
    psocket->Start();
    delete psocket;
}


int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        std::cout << "pls Run as " << argv[0] << " server/client config.txt" << std::endl;
        return 1;
    }

    char *cs = argv[1];
    char *configFile = argv[2];
    int   runType = 0;
    
    if(::strncasecmp(cs, g_trans[SERVER], ::strlen(g_trans[SOCKET])) == 0)
    {
        runType = SERVER;
    }
    else if(::strncasecmp(cs, g_trans[CLIENT], ::strlen(g_trans[CLIENT])) == 0)
    {
        runType = CLIENT;
    }
    else
    {
        std::cout << "arg[1] is err" << std::endl;
    }

    StartCs(configFile, runType);

    return 0;
}

