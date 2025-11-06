#include "core/logger.hpp"

#include <iostream>
#include "console/console.hpp"

namespace Logger {
	void Logger::addSink(ILogSink* sink) {
		_sinks.push_back(sink);
	}

	void Logger::log(const Severity sev, const std::string& msg) const {
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
			std::cout << "[INFO] " << msg << std::endl;
			break;
		case Severity::Debug:
			std::cout << "[DEBUG] " << msg << std::endl;
			break;
		case Severity::None:
			std::cout << msg << std::endl;
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

	void Fatal(const std::string& msg) {
		g_logger->log(Logger::Severity::Fatal, msg);
	}

	void Log(const Logger::Severity sev, const std::string& msg) {
		g_logger->log(sev, msg);
	}

	void SetSeverity(const Logger::Severity sev) {
		g_logger->setSeverity(sev);
	}
}
