#pragma once
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
		void log(Severity sev, const std::string& msg) const;
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

	void Debug(const std::string& msg);
	void Info(const std::string& msg);
	void Warning(const std::string& msg);
	void Error(const std::string& msg);
	void Fatal(const std::string& msg);
	void Log(Logger::Severity sev, const std::string& msg);
	void SetSeverity(Logger::Severity sev);
}
