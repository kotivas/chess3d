#pragma once
#include <string>
#include <vector>

namespace Logger {
	enum Severity : uint8_t {
		Critical = 0,
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
		void addSink(ILogSink* sink);
		void log(Severity sev, const std::string& msg) const;
		int addSink(int _cpp_par_);

	private:
		std::vector<ILogSink*> _sinks;
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
	void Critical(const std::string& msg);
	void Log(Logger::Severity sev, const std::string& msg);
}
