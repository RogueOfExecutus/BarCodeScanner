#pragma once
#include <log4cplus/log4cplus.h>

class MyLogger
{
public:
	MyLogger(std::string name, std::string path, log4cplus::LogLevel level);
	virtual ~MyLogger();
	//MyLogger(const MyLogger&);
	void writeLog(log4cplus::LogLevel level, std::string logMsg, std::string name);
private:
	MyLogger();
};

