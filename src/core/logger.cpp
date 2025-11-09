#include "core/logger.hpp"

#include <format>
#include <iostream>
#include <sstream>
#include "console/console.hpp"

namespace Logger {
	void Logger::addSink(ILogSink* sink) {
		_sinks.push_back(sink);
	}

	void Logger::log(const Severity sev, const std::string_view fmt, const std::format_args args) const {
		const std::string msg = std::vformat(fmt, args);
		if (sev <= _severity) {
			for (const auto& s : _sinks) {
				s->write(sev, msg);
			}
		}
	}

	void Logger::setSeverity(const Severity sev) {
		_severity = sev;
	}

	void CoutSink::write(const Severity sev, const std::string& msg) {
		switch (sev) {
		case Severity::Fatal:
			std::cout << "[CRITICAL] " << msg << std::endl;
			break;
		case Severity::Error:
			std::cout << "[ERROR] " << msg << std::endl;
			break;
		case Severity::Warning:
			std::cout << "[WARNING] " << msg << std::endl;
			break;
		case Severity::Info:
			std::cout << msg << std::endl;
			break;
		case Severity::Debug:
			std::cout << "[DEBUG] " << msg << std::endl;
			break;
		}
	}

	void ConsoleSink::write(Severity sev, const std::string& msg) {
		Console::Print(sev, msg);
	}
}


namespace Log {
	Logger::Logger* g_logger = nullptr;

	void Init() {
		g_logger = new Logger::Logger();
		g_logger->addSink(reinterpret_cast<Logger::ILogSink*>(new Logger::ConsoleSink()));
		g_logger->addSink(reinterpret_cast<Logger::ILogSink*>(new Logger::CoutSink()));
	}

	Logger::Logger* GetLogger() {
		return g_logger;
	}

	void SetSeverity(const Logger::Severity sev) {
		g_logger->setSeverity(sev);
	}
}
