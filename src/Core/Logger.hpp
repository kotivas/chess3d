#pragma once
#include <format>
#include <string>
#include <vector>

namespace Logger {
	enum class Severity : uint8_t {
		Fatal = 0,
		Error,
		Warning,
		Info,
		Debug
	};

	class ILogSink {
	public:
		virtual void write(Severity, const std::string&) = 0;
		virtual ~ILogSink() = default;
	};

	class Logger final {
	public:
		Logger()
			: _severity(Severity::Debug) {} // log everything

		void addSink(ILogSink* sink);
		void log(Severity sev, std::string_view fmt, std::format_args args) const;
		void setSeverity(Severity sev);

	private:
		std::vector<ILogSink*> _sinks;
		Severity _severity;
	};

	class CoutSink final : ILogSink {
		void write(Severity, const std::string&) override;
	};

	class ConsoleSink final : ILogSink {
		void write(Severity, const std::string&) override;
	};
}

namespace Log {
	extern Logger::Logger* g_logger;

	void Init();
	Logger::Logger* GetLogger();

	template <typename... Args>
	void Debug(const std::string_view fmt, Args&&... args) {
		g_logger->log(Logger::Severity::Debug, fmt, std::make_format_args(std::forward<Args>(args)...));
	}

	template <typename... Args>
	void Info(const std::string_view fmt, Args&&... args) {
		g_logger->log(Logger::Severity::Info, fmt, std::make_format_args(std::forward<Args>(args)...));
	}

	template <typename... Args>
	void Warning(const std::string_view fmt, Args&&... args) {
		g_logger->log(Logger::Severity::Warning, fmt, std::make_format_args(std::forward<Args>(args)...));
	}

	template <typename... Args>
	void Error(const std::string_view fmt, Args&&... args) {
		g_logger->log(Logger::Severity::Error, fmt, std::make_format_args(std::forward<Args>(args)...));
	}

	template <typename... Args>
	void Fatal(const std::string_view fmt, Args&&... args) {
		g_logger->log(Logger::Severity::Fatal, fmt, std::make_format_args(std::forward<Args>(args)...));
	}

	template <typename... Args>
	void Log(const Logger::Severity sev, const std::string_view fmt, Args&&... args) {
		g_logger->log(sev, fmt, std::make_format_args(std::forward<Args>(args)...));
	}

	void SetSeverity(Logger::Severity sev);
}
