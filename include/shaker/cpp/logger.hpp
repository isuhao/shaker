#pragma once

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/var.h>
#include <sstream>

namespace pp
{
	class Logger : public Instance
	{
	protected:
		explicit Logger(PP_Instance instance);

	public:
		void console(PP_LogLevel logLevel, const std::string& msg);
		void console(PP_LogLevel logLevel, const pp::Var& source, const std::string& msg);

		template <PP_LogLevel logLevel>
		class Severity;

		template <PP_LogLevel logLevel>
		class Source
		{
			template <PP_LogLevel logLevel>
			friend class Severity;
			Logger& parent_;
			pp::Var source_;

			Source(Logger& parent, const pp::Var& source) : parent_(parent), source_(source)
			{}

			Source(const Source&);
			Source& operator=(const Source&);

			void utf8(std::ostream& o) {}
			template <typename T, typename... Args>
			void utf8(std::ostream& o, T&& arg, Args&&... rest)
			{
				o << arg;
				utf8(o, std::forward<Args>(rest)...);
			}

			template <typename... Args>
			void utf8(std::ostream& o, PP_Var arg, Args&&... rest)
			{
				o << VarInterface().VarToUtf8(arg);
				utf8(o, std::forward<Args>(rest)...);
			}

		public:

			template <typename... Args>
			void operator()(Args&&... args)
			{
				std::ostringstream o;
				o << std::boolalpha;
				utf8(o, std::forward<Args>(args)...);
				parent_.console(logLevel, source_, o.str());
			}

			void direct(const pp::Var& msg)
			{
				parent_.LogToConsoleWithSource(logLevel, source_, msg);
			}
		};

		template <PP_LogLevel logLevel>
		class Severity
		{
			friend class Logger;
			Logger& parent_;

			explicit Severity(Logger& parent) : parent_(parent)
			{}

			Severity(const Severity&);
			Severity& operator=(const Severity&);

			void utf8(std::ostream& o) {}
			template <typename T, typename... Args>
			void utf8(std::ostream& o, T&& arg, Args&&... rest)
			{
				o << arg;
				utf8(o, std::forward<Args>(rest)...);
			}

			template <typename... Args>
			void utf8(std::ostream& o, PP_Var arg, Args&&... rest)
			{
				o << VarInterface().VarToUtf8(arg);
				utf8(o, std::forward<Args>(rest)...);
			}

		public:

			template <typename... Args>
			void operator()(Args&&... args)
			{
				std::ostringstream o;
				o << std::boolalpha;
				utf8(o, std::forward<Args>(args)...);
				parent_.console(logLevel, o.str());
			}

			void direct(const pp::Var& msg)
			{
				parent_.LogToConsole(logLevel, msg);
			}

			Source<logLevel> operator[](const pp::Var& source)
			{
				return{ parent_, source };
			}
		};
		Severity<PP_LOGLEVEL_TIP> tip;
		Severity<PP_LOGLEVEL_LOG> log;
		Severity<PP_LOGLEVEL_WARNING> warning;
		Severity<PP_LOGLEVEL_ERROR> error;
	};
}
