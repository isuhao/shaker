#include <shaker/cpp/logger.hpp>
#include <ppapi/cpp/var.h>

namespace pp
{
	Logger::Logger(PP_Instance instance)
		: Instance(instance)
		, tip(*this)
		, log(*this)
		, warning(*this)
		, error(*this)
	{
	}

	void Logger::console(PP_LogLevel logLevel, const std::string& msg)
	{
		LogToConsole(logLevel, msg);
	}

	void Logger::console(PP_LogLevel logLevel, const pp::Var& source, const std::string& msg)
	{
		LogToConsoleWithSource(logLevel, source, msg);
	}
}
