#include "stdafx.h"
#include "MyLogger.h"

using namespace log4cplus;
using namespace std;

MyLogger::MyLogger()
{
}

MyLogger::MyLogger(string name, string path, LogLevel level)
{

	SharedAppenderPtr fileAppender(new RollingFileAppender(
		LOG4CPLUS_STRING_TO_TSTRING(path),
		5 * 1024 * 1024,
		5
	)
	);

	fileAppender->setName(LOG4CPLUS_TEXT("file"));
	tstring pattern = LOG4CPLUS_TEXT("%D{%m/%d/%y %H:%M:%S,%Q} [%t] %-5p %c - %m [%l]%n");
	fileAppender->setLayout(unique_ptr<Layout>(new PatternLayout(pattern)));

	Logger logger = Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(name));
	logger.setLogLevel(level);

	//第4步：为Logger实例添加FileAppender
	logger.addAppender(fileAppender);
}

MyLogger::~MyLogger()
{
}

void MyLogger::writeLog(LogLevel level, string logMsg, string name)
{
	Logger logger = Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(name));
	switch (level)
	{
	case FATAL_LOG_LEVEL:
		LOG4CPLUS_FATAL(logger, LOG4CPLUS_STRING_TO_TSTRING(logMsg));
		break;
	case ERROR_LOG_LEVEL:
		LOG4CPLUS_ERROR(logger, LOG4CPLUS_STRING_TO_TSTRING(logMsg));
		break;
	case WARN_LOG_LEVEL:
		LOG4CPLUS_WARN(logger, LOG4CPLUS_STRING_TO_TSTRING(logMsg));
		break;
	case INFO_LOG_LEVEL:
		LOG4CPLUS_INFO(logger, LOG4CPLUS_STRING_TO_TSTRING(logMsg));
		break;
	case DEBUG_LOG_LEVEL:
		LOG4CPLUS_DEBUG(logger, LOG4CPLUS_STRING_TO_TSTRING(logMsg));
		break;
	case TRACE_LOG_LEVEL:
		LOG4CPLUS_TRACE(logger, LOG4CPLUS_STRING_TO_TSTRING(logMsg));
		break;
	default:
		break;
	}
}
