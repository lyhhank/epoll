/*********************************************************************************
  Copyright(C),2019-2020,lrwwxj@163.com
  FileName:		base.hpp
  Author: 		limeng
  Version:  	v1.0
  Date:			2020.3.1
  Description:  读配置文件
**********************************************************************************/

#ifndef _READCONF_HPP
#define _READCONF_HPP 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <set>
#include <sstream>

#include "base.hpp"

namespace lmSOCKET
{
 	class CREADCONF : virtual public CBASE
	{
		public:
			CREADCONF();
			~CREADCONF();

            int ReadPublicConf(const char *configFile, config_t *pconf);
            int ReadServerConf(const char *configFile, config_t *pconf);
            int ReadClientConf(const char *configFile, config_t *pconf);
        protected:
            template <class T1, class T2>
            T1 TransType(T1 &, T2 &in);
            int DelSpace(char *buf);
            int GetPos(char *buf);
            int ReadConf(const char *configFile, const char *title, const char *keyName, char *keyValue);
            int GetAllClientTitle(const char *configFile, std::set<std::string> &allClientTitle);
            int FindTransIndex(const std::set<std::string> &allClientTitle, const std::string &title);
            int MapType(const std::string str);
        private:
            int m_ret;
	};
}

#endif
