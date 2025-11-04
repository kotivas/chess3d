#include "core/logger.hpp"

#include <iostream>

#include "console/console.hpp"

namespace Logger {
	void Logger::addSink(ILogSink* sink) {
		_sinks.push_back(sink);
	}

	void Logger::log(Severity sev, const std::string& msg) const {
		for (const auto& s : _sinks) {
			s->write(sev, msg);
		}
	}

	void CoutSink::write(Severity sev, const std::string& msg) {
		switch (sev) {
		case Severity::Critical:
			std::cout << "[CRITICAL] " << msg << std::endl;
			break;
		case Severity::Error:
			std::cout << "[ERROR] " << msg << std::endl;
			break;
		case Severity::Warning:
			std::cout << "[WARNING] " << msg << std::endl;
			break;
		case Severity::Info:
			std::cout << "[INFO] " << msg << std::endl;
			break;
		case Severity::Debug:
			std::cout << "[DEBUG] " << msg << std::endl;
			break;
		}
	}

	void ConsoleSink::write(Severity sev, const std::string& msg) {
		Console::Print(static_cast<Console::SeverityLevel>(sev), msg);
	}
}


namespace Log {
	Logger::Logger* g_logger = nullptr;

	void Init() {
		g_logger = new Logger::Logger();
		g_logger->addSink(reinterpret_cast<Logger::ILogSink*>(new Logger::ConsoleSink()));
		// g_logger->addSink(reinterpret_cast<Logger::ILogSink*>(new Logger::CoutSink()));
	}

	Logger::Logger* GetLogger() {
		return g_logger;
	}

	void Debug(const std::string& msg) {
		g_logger->log(Logger::Severity::Debug, msg);
	}

	void Info(const std::string& msg) {
		g_logger->log(Logger::Severity::Info, msg);
	}

	void Warning(const std::string& msg) {
		g_logger->log(Logger::Severity::Warning, msg);
	}

	void Error(const std::string& msg) {
		g_logger->log(Logger::Severity::Error, msg);
	}

	void Critical(const std::string& msg) {
		g_logger->log(Logger::Severity::Critical, msg);
	}

	void Log(Logger::Severity sev, const std::string& msg) {
		g_logger->log(sev, msg);
	}
}
