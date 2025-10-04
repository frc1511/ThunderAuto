#include <ThunderAuto/Logger.hpp>
#include <ThunderAuto/Error.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <mutex>

static const std::string kThunderAutoLoggerName = "ThunderAuto";

static std::shared_ptr<spdlog::logger> s_thunderAutoLogger;
static std::mutex s_loggerMutex;

static void InitLogger(const std::filesystem::path& logsDir) {
  std::vector<spdlog::sink_ptr> sinks;

  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

  if (!logsDir.empty()) {
    std::filesystem::path logFile = MakeLogFilePath(logsDir, "ThunderLibCoreTests");
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile.string(), true));
    CleanupLogsDirectory("logs", 10);
  }

  s_thunderAutoLogger = std::make_shared<spdlog::logger>(kThunderAutoLoggerName, sinks.begin(), sinks.end());
  ThunderLibLogger::make(sinks.begin(), sinks.end());  // ThunderLibLogger gets the same sinks
}

spdlog::logger* ThunderAutoLogger::get() {
  std::lock_guard<std::mutex> lock(s_loggerMutex);
  if (!s_thunderAutoLogger) {
    InitLogger("logs");
  }

  return s_thunderAutoLogger.get();
}
